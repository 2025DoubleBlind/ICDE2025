#ifndef TIGHTSKETCH_H
#define TIGHTSKETCH_H

#include "Abstract.h"
#include <vector>
#include <cstring>

template<typename DATA_TYPE, typename COUNT_TYPE>
class TightSketch : public Abstract<DATA_TYPE, COUNT_TYPE> {
private:
    struct SBucket {
        COUNT_TYPE count;
        COUNT_TYPE hotcount;
        DATA_TYPE key;
        uint8_t status;
        
        SBucket() : count(0), hotcount(0), key(0), status(0) {}
    };

    int depth;
    int width;
    COUNT_TYPE sum;
    SBucket** counts;
    
public:
    TightSketch(int _depth, int _width) : depth(_depth), width(_width), sum(0) {
        counts = new SBucket*[depth * width];
        for (int i = 0; i < depth * width; i++) {
            counts[i] = new SBucket();
        }
    }

    ~TightSketch() {
        for (int i = 0; i < depth * width; i++) {
            delete counts[i];
        }
        delete[] counts;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window) override {
        sum += 1;
        unsigned long bucket;
        SBucket* sbucket;
        int flag = 0;
        COUNT_TYPE min = 99999999;
        int loc = -1;
        
        for (int i = 0; i < depth; i++) {
            bucket = this->hash(item, i) % width;
            int index = i * width + bucket;
            sbucket = counts[index];
            
            if (sbucket->count == 0 && sbucket->status == 0) {
                sbucket->key = item;
                sbucket->count = 1;
                sbucket->status = 1;
                return;
            }
            else if (sbucket->key == item) {
                if (sbucket->status == 0) {
                    sbucket->count += 1;
                    sbucket->status = 1;
                }
                return;
            }
            
            if (sbucket->count < min) {
                min = sbucket->count;
                loc = index;
            }
        }
        
        if (loc >= 0) {
            sbucket = counts[loc];
            
            if (sbucket->status == 1) return;
            
            if (sbucket->count < 10) {
                int k = rand() % (int)(sbucket->count + 1) + 1;
                if (k > (int)(sbucket->count)) {
                    sbucket->count -= 1;
                }
                if (sbucket->count <= 0) {
                    sbucket->key = item;
                    sbucket->count = 1;
                    sbucket->status = 1;
                }
            }
            else {
                int j = rand() % (int)(sbucket->hotcount * sbucket->count + 1) + 1;
                if (j > (int)(sbucket->hotcount * sbucket->count)) {
                    sbucket->count -= 1;
                    if (sbucket->count <= 0) {
                        sbucket->key = item;
                        sbucket->count = 1;
                        sbucket->status = 1;
                    }
                }
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item) override {
        COUNT_TYPE max = 0, min = INT32_MAX;
        for (int i = 0; i < depth; i++) {
            unsigned long bucket = this->hash(item, i) % width;
            int index = i * width + bucket;
            if (counts[index]->key == item) {
                max += counts[index]->count;
            }
            if (counts[index]->count < min) min = counts[index]->count;
        }
        if (max != 0) return max;
        return min;
    }

    void NewWindow(const COUNT_TYPE window) override {
        for (int i = 0; i < depth * width; i++) {
            counts[i]->status = 0;
        }
        QueryHotness();
    }

    void QueryHotness() {
        for (int i = 0; i < depth * width; i++) {
            if (counts[i]->status == 0) {
                counts[i]->hotcount = std::max(counts[i]->hotcount - 1, (COUNT_TYPE)0);
            }
            else if (counts[i]->status == 1) {
                counts[i]->hotcount += 1;
            }
        }
    }

    std::string getName() override {
        return "TS";
    }

    void reset() override {
        sum = 0;
        for (int i = 0; i < depth * width; i++) {
            counts[i]->count = 0;
            counts[i]->hotcount = 0;
            counts[i]->key = 0;
            counts[i]->status = 0;
        }
    }
};

#endif //TIGHTSKETCH_H