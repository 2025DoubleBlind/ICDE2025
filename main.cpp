#include <getopt.h>
#include "benchmark.h"

int main(int argc, char *argv[]) {
    char* dataset = "./data/caida15.dat";
    int M = 500;
    int W = 1600;
    int c;
    while((c=getopt(argc, argv, "d:m:w:")) != -1) {
        switch(c) {
            case 'd':
                strcpy(dataset,optarg);
                break;
            case 'm':
                M = atoi(optarg);
                break;
            case 'w':
                W = atoi(optarg);
                break;
        }
    }

    BenchMark<string ,int32_t> benchMark(dataset, W, M);
    benchMark.SketchError();
    return 0;
}
