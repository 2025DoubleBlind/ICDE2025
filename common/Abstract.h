#ifndef ABSTRACT_H
#define ABSTRACT_H

#include <unordered_map>

#include "hash.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class Abstract{
public:
    Abstract(){}
    virtual ~Abstract(){};

    virtual void Insert(const DATA_TYPE item, const COUNT_TYPE window) = 0;
    virtual COUNT_TYPE Query(const DATA_TYPE item) = 0;
    virtual void NewWindow(const COUNT_TYPE window) = 0;
    virtual std::string getName() = 0;
    virtual void reset() = 0;

//    inline uint32_t hash(DATA_TYPE data, uint32_t seed = 0){
//        if(std::is_same<DATA_TYPE, std::string>::value){
//            return Hash::BOBHash32((uint8_t*)(data.c_str()), 13, seed);
//        }else{
//            return Hash::BOBHash32((uint8_t*)&data, sizeof(DATA_TYPE), seed);
//        }
//    }
    inline uint32_t hash(DATA_TYPE data) {
        if (std::is_same<DATA_TYPE, std::string>::value) {
            return Hash::BOBHash32((uint8_t*)data.c_str(), data.size(), 0);
        } else{
            return Hash::BOBHash32((uint8_t*)&data, sizeof(DATA_TYPE), 0);
        }
    }

    inline uint64_t hash64(DATA_TYPE data) {
        if (std::is_same<DATA_TYPE, std::string>::value) {
            return Hash::BOBHash64((uint8_t*)data.c_str(), data.size(), 0);
        } else{
            return Hash::BOBHash64((uint8_t*)&data, sizeof(DATA_TYPE), 0);
        }
    }

    inline uint32_t hash(DATA_TYPE data, uint32_t seed){
        if (std::is_same<DATA_TYPE, std::string>::value) {
            return Hash::BOBHash32((uint8_t*)data.c_str(), data.size(), seed);
        } else{
            return Hash::BOBHash32((uint8_t*)&data, sizeof(DATA_TYPE), seed);
        }
    }
};

#endif //ABSTRACT_H
