#ifndef BENCH_PF_H
#define BENCH_PF_H
#define SLOT_NUM 1

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class PF : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    struct Bucket {
        uint16_t counters[SLOT_NUM];
    };

    PF(uint32_t _hash_num, uint32_t memory):
            hash_num(_hash_num){
        length = memory * 1024 / 3.0 / (BITSIZE + sizeof(uint16_t)) / SLOT_NUM;
        buckets = new Bucket * [hash_num];
        bitsets = new BitSet* [hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            buckets[i] = new Bucket [length];
            bitsets[i] = new BitSet(length);
            memset(buckets[i], 0, length * sizeof(Bucket));
        }
    }

    ~PF(){
        for(uint32_t i = 0;i < hash_num;++i){
            delete [] buckets[i];
            delete bitsets[i];
        }
        delete [] buckets;
        delete [] bitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        int min = INT32_MAX;
        uint64_t hash = this->hash64(item);
        uint32_t pos[hash_num];

        for(uint32_t i = 0;i < hash_num;++i){
            pos[i] = (hash & (0x1fffff)) % length;
            hash >>= 21;
            for(int j = 0; j < SLOT_NUM; j++){
                if(buckets[i][pos[i]].counters[j] < min){
                    min = buckets[i][pos[i]].counters[j];
                }
            }
        }
        if(min != INT32_MAX){
            for(int i = 0; i < hash_num; i++){
                for(int j = 0; j < SLOT_NUM; j++){
                    if(buckets[i][pos[i]].counters[j] == min){
                        buckets[i][pos[i]].counters[j] += (!bitsets[i]->SetNGet(pos[i]));
                    }
                }
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        int ret = INT32_MAX;
        uint64_t hash = this->hash64(item);
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = (hash & (0x1fffff)) % length;
            hash >>= 21;
            for(int j = 0; j < SLOT_NUM; j++) {
                ret = MIN(ret, buckets[i][pos].counters[j]);
            }
        }
        return ret;
    }

    std::string getName(){
        return "PF";
    }

    void NewWindow(const COUNT_TYPE window){
        for(uint32_t i = 0;i < hash_num;++i){
            bitsets[i]->Clear();
        }
    }

    void reset(){
        for(uint32_t i = 0;i < hash_num;++i){
            bitsets[i]->Clear();
            memset(buckets[i], 0, length * sizeof(Bucket));
        }
    }

    uint32_t getLength(){
        return length;
    }

private:
    const uint32_t hash_num;
    unsigned int length;

    BitSet** bitsets;
    //COUNT_TYPE** counters;
    Bucket ** buckets;
};

#endif //BENCH_PF_H