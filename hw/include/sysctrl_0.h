#include <stdint.h>
#include <systemc.h>
#include <ahb_slave_if.h>

#ifndef _SYSCTRL_0_H_
#define _SYSCTRL_0_H_

/* the SystemC module of ARM SP810 status & system control module */
class SYSCTRL_0: public ahb_slave_if, public sc_module
{
    public:
        sc_in_clk clk;              //clock signal
        sc_in<bool>rst_n;           //negative edge trigger reset signal

        SC_HAS_PROCESS(SYSCTRL_0);
        SYSCTRL_0(sc_module_name name, uint32_t mapping_size, uint32_t sys_id, uint32_t proc_id);

        bool read_0(uint32_t*, uint32_t, int);
        bool write_0(uint32_t, uint32_t, int);

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
        uint32_t SYS_ID;
        uint32_t SYS_PROC_ID_0;
        uint32_t SYS_PROC_ID_1;
        uint32_t led;
        uint32_t lock;
        uint32_t flag;
        uint32_t counter_100, counter_24M;
        uint32_t sys_pldctl;

        void counter_100Hz();
        void counter_24MHz();
};


#endif
