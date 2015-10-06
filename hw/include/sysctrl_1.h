#include <stdint.h>
#include <systemc.h>
#include <ahb_slave_if.h>

#ifndef _SYSCTRL_1_H_
#define _SYSCTRL_1_H_

/* the SystemC module of ARM SP810 status & system control module */
class SYSCTRL_1: public ahb_slave_if, public sc_module
{
    public:
        sc_in_clk clk;              //clock signal
        sc_in<bool>rst_n;           //negative edge trigger reset signal

        SC_HAS_PROCESS(SYSCTRL_1);
        SYSCTRL_1(sc_module_name name, uint32_t mapping_size);

        bool read_1(uint32_t*, uint32_t, int);
        bool write_1(uint32_t, uint32_t, int);

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
        uint32_t led;
        uint32_t lock;
        uint32_t counter_100, counter_24M;

        void run(void);
};


#endif
