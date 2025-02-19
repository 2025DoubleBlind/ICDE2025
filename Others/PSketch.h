#ifndef PSKETCH_H
#define PSKETCH_H

#include "Abstract.h"
#include <random>

template<typename DATA_TYPE, typename COUNT_TYPE>
class PSketch : public Abstract<DATA_TYPE, COUNT_TYPE> {
private:
    struct Bucket {
        DATA_TYPE key;         // item key
        COUNT_TYPE p;          // persistence counter
        COUNT_TYPE h;          // hotness counter
        bool flag;             // On/Off flag
        
        Bucket() : key(0), p(0), h(0), flag(true) {}  // flag初始化为On
    };

    int depth;          // r in pseudocode
    int width;          // w in pseudocode
    int window_size;    // W in pseudocode
    int cur_count;      // c in pseudocode
    Bucket** buckets;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;

public:
    PSketch(int _depth, int _width, int _window_size) 
        : depth(_depth), width(_width), window_size(_window_size), cur_count(0),
          gen(rd()), dis(0.0, 1.0) {
        
        buckets = new Bucket*[depth];
        for(int i = 0; i < depth; i++) {
            buckets[i] = new Bucket[width];
        }
    }

    ~PSketch() {
        for(int i = 0; i < depth; i++) {
            delete[] buckets[i];
        }
        delete[] buckets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window) override {
        cur_count++;
        
        // Stage I: Check new window
        if(cur_count % window_size == 0) {
            for(int j = 0; j < depth; j++) {
                for(int q = 0; q < width; q++) {
                    if(buckets[j][q].flag) {
                        buckets[j][q].h = std::max(buckets[j][q].h - 1, (COUNT_TYPE)0);
                    } else {
                        buckets[j][q].flag = true;
                    }
                }
            }
        }

        // Find minimum persistence + hotness
        COUNT_TYPE min_val = std::numeric_limits<COUNT_TYPE>::max();
        int R = 0, M = 0;

        // Stage I: Finding an available bucket
        for(int i = 0; i < depth; i++) {
            int index = this->hash(item, i) % width;
            
            // Empty bucket
            if(buckets[i][index].p == 0) {
                buckets[i][index].key = item;
                buckets[i][index].p = 1;
                buckets[i][index].h = 1;
                buckets[i][index].flag = false;
                return;
            }
            
            // Found same item
            if(buckets[i][index].key == item && buckets[i][index].flag) {
                buckets[i][index].p += 1;
                buckets[i][index].h += 1;
                buckets[i][index].flag = false;
                return;
            }

            // Track minimum for replacement
            COUNT_TYPE sum = buckets[i][index].p + buckets[i][index].h;
            if(sum < min_val) {
                min_val = sum;
                R = i;
                M = index;
            }
        }

        // Stage II: Probability-based replacement
        if(buckets[R][M].flag == false) {
            return;  // Discard new item
        }

        // Probability-based replacement
        double prob = 1.0 / (buckets[R][M].p + buckets[R][M].h + 1);
        if(dis(gen) < prob) {
            buckets[R][M].key = item;
            buckets[R][M].p += 1;
            buckets[R][M].h += 1;
            buckets[R][M].flag = false;
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item) override {
        COUNT_TYPE result = 0, min = INT32_MAX;
        for(int i = 0; i < depth; i++) {
            int index = this->hash(item, i) % width;
            if(buckets[i][index].key == item) {
                result += buckets[i][index].p;
            }
            if(buckets[i][index].p < min) min = buckets[i][index].p;
        }
        if(result != 0) return result;
        return min;
    }

    void NewWindow(const COUNT_TYPE window) override {
        for(int i = 0; i < depth; i++) {
            for(int j = 0; j < width; j++) {
                if(buckets[i][j].flag) {
                    buckets[i][j].h = std::max(buckets[i][j].h - 1, (COUNT_TYPE)0);
                } else {
                    buckets[i][j].flag = true;
                }
            }
        }
    }

    std::string getName() override {
        return "PS";
    }

    void reset() override {
        for(int i = 0; i < depth; i++) {
            for(int j = 0; j < width; j++) {
                buckets[i][j].key = 0;
                buckets[i][j].p = 0;
                buckets[i][j].h = 0;
                buckets[i][j].flag = true;
            }
        }
        cur_count = 0;
    }
};

#endif //PSKETCH_H 