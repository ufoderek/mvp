#ifndef _CPU_MONITOR_H_
#define _CPU_MONITOR_H_

#include <vector>
#include <cstdio>
#include <stdint.h>

using namespace std;

class cpu_monitor
{
    public:

        cpu_monitor(const char* title, const char* x_title, const char* y_title, unsigned int size);
        ~cpu_monitor();

        void add(uint64_t x, uint32_t y);

        void plot();

    private:
        FILE* gp;
        FILE* fout;

        char title[128];
        char x_title[128];
        char y_title[128];

        vector<uint32_t> x;
        vector<uint64_t> y;
};

#endif
