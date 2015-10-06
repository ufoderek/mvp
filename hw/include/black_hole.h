#ifndef _BLACK_HOLE_H_
#define _BLACK_HOLE_H_

#include <stdint.h>
#include <systemc.h>
#include <ahb_slave_if.h>
#include <cstdio>

using namespace std;

class black_hole: public ahb_slave_if, public sc_module
{
    public:
        SC_HAS_PROCESS(black_hole);
        black_hole(sc_module_name name, uint32_t mapping_size);
        ~black_hole();

        bool read(uint32_t*, uint32_t, int);
        bool write(uint32_t, uint32_t, int);

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

        void sucker();

    private:
        uint32_t buf_size;
        FILE* fout;
        sc_event suck;
};

#endif
