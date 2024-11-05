#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <sys/stat.h>
#include <iostream>

#include <chrono>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "OO.h"
#include "hash.h"
#include "PF.h"
#include "CM.h"


using namespace std;
template<typename DATA_TYPE,typename COUNT_TYPE>
class BenchMark{
public:

    typedef vector<Abstract<DATA_TYPE, COUNT_TYPE>*> AbsVector;
    typedef unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    vector<DATA_TYPE> packets;

    BenchMark(const char* _PATH, const COUNT_TYPE _W, const int _M):
            PATH(_PATH){
        W = _W;

        M = _M;

        FILE* file = fopen(PATH, "rb");

        COUNT_TYPE number = 0;
        DATA_TYPE item;
        HashMap record;

        TOTAL = 0;
        T = 0;

        int max = 0;

        char tmp[105];
        while(fread(&tmp, data_size, 1, file) > 0){
            tmp[data_size] = '\0';
            item = std::string(tmp, data_size);

            packets.push_back(item);
            if(number % W == 0)
                T += 1;
            number += 1;

            if(record[item] != T){
                record[item] = T;
                mp[item] += 1;
                TOTAL += 1;
                if(mp[item] > max) max = mp[item];
            }
        }
        packet_num = number;
        fclose(file);

        HIT = T * 0.2;

        int cnt = 0, cnt100 = 0, cnt50 = 0;
        int lt255cnt = 0;
        for(auto it = mp.begin();it != mp.end();++it){
            if(it->second > HIT){
                cnt++;
            }
            if(it->second < 255) lt255cnt ++;
            if(it->second > 100) cnt100++;
            if(it->second > 50) cnt50++;
        }

        printf("Memory = %dKB\nWindow length = %d\nWindows cnt = %d\n\n", M, W, T);
//        printf("packet_num %d packet_type %d max_persistence %d more than HIT cnt= %d more than 255 cnt %d\n", packet_num, record.size(), max, cnt, record.size() - lt255cnt);
//        printf("more than 100 cnt %d more than 50 cnt %d\n", cnt100,cnt50);
    }

    void SketchError(){
        std::cout << "PE exam begin" << std::endl;
        AbsVector PEs = {
                new OO<DATA_TYPE, COUNT_TYPE>(3, M * 1024 / 3.0 / (BITSIZE + sizeof(COUNT_TYPE))),
                new PF<DATA_TYPE, COUNT_TYPE>(3, M),
                new CM<DATA_TYPE, COUNT_TYPE>(4, M * 1024 / 32),
        };

        BenchInsert(PEs);

        for(auto PE : PEs){
            PECheckError(PE);
            delete PE;
        }
        std::cout << "PE exam end\n" << std::endl;
    }


private:
    int M;
    int data_size = 13;
    unordered_map<string, double> insert_through;
    int test_cycle = 1;

    double TOTAL;
    COUNT_TYPE T;
    COUNT_TYPE W;
    uint32_t packet_num;
    COUNT_TYPE HIT;

    HashMap mp;
    const char* PATH;

    typedef std::chrono::high_resolution_clock::time_point TP;

    inline TP now(){
        return std::chrono::high_resolution_clock::now();
    }

    inline double durations(TP finish, TP start){
        return std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
    }

    void BenchInsert(AbsVector sketches){
        for(auto sketch : sketches){
            TP start, end;
            double new_window_time = 0.0;
            start = now();
            for(int j = 0; j < test_cycle; j++){
                sketch->reset();
                COUNT_TYPE windowId = 0;
                for (int i = 0; i < packet_num; i++){
                    if (i % W == 0) {
                        windowId += 1;
                        new_window_time += newWindow(sketch, windowId);
                    }
                    sketch->Insert(packets[i], windowId);
                }
                new_window_time += newWindow(sketch, windowId);
            }
            end = now();
            insert_through[sketch->getName()] = (double) test_cycle * packet_num / (durations(end, start) - new_window_time);
        }
    }

    double newWindow(Abstract<DATA_TYPE, COUNT_TYPE>* sketch, COUNT_TYPE windowId){
        TP window_start, window_end;
        window_start = now();
        sketch->NewWindow(windowId);
        window_end = now();
        return durations(window_end, window_start);
    }

    void InsertThp(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        TP start, end;
        double new_window_time = 0.0;
        int windowId = 0;
        start = now();
        for(int j = 0; j < test_cycle; j++){
            sketch->reset();
            for (int i = 0; i < packet_num; i++){
                if (i % W == 0) {
                    windowId += 1;
                    new_window_time += newWindow(sketch, windowId);
                }
                sketch->Insert(packets[i], windowId);
            }
            new_window_time += newWindow(sketch, windowId);
        }
        end = now();
        std::cout << sketch->getName() << " Thp: " << (double) test_cycle * packet_num / (durations(end, start) - new_window_time) << std::endl;
    }

    void PECheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        double aae = 0, are = 0;

        for(auto it = mp.begin();it != mp.end();++it) {
            COUNT_TYPE per = sketch->Query(it->first);
            COUNT_TYPE real = it->second;
            aae += abs(real - per);
            are += abs(real - per) / (real + 0.0);
        }

        ofstream outFile(R"(./output.csv)",std::ios::app);
        outFile << sketch->getName() << "," << M << "," << W << "," << aae / mp.size() << ","
                << are / mp.size() << "," << insert_through[sketch->getName()] << "\n";
        outFile.close();

        std::cout << sketch->getName() + ":" << std::endl;
        std::cout << "\tAAE: " << aae / mp.size() << std::endl
                  << "\tARE: " << are / mp.size() << std::endl
                  << "\tthroughput(insert): " << insert_through[sketch->getName()] << std::endl;
    }
};

#endif //BENCHMARK_H
