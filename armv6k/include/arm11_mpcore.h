#ifndef _ARM11_MPCORE_H_
#define _ARM11_MPCORE_H_

#include <memory.h>
#include <stdint.h>
#include <systemc.h>
#include <boost/shared_ptr.hpp>
#include <armv6k.h>
#include <gic2_dist.h>
#include <gic2_cpu_if.h>

class ARM11_MPCORE: public sc_module
{
    public:
        sc_in_clk clk;
        sc_in_clk clk_slow;
        sc_in<bool> gic_irq_n[64];

        sc_signal<bool> irq_n[4];
        sc_signal<bool> virq_n[4];

        SC_HAS_PROCESS(ARM11_MPCORE);
        ARM11_MPCORE(sc_module_name name, uint32_t addr, uint32_t second_addr, bool using_gdb, unsigned int gdb_port, unsigned int n_cores);
        ~ARM11_MPCORE();

        boost::shared_ptr<mp_scu> scu;
        boost::shared_ptr<gic2_dist> gic_dist;
        boost::shared_ptr<gic2_cpu_if> gic_cpu_if;

        auto_ptr<ARMV6K> core0;
        auto_ptr<ARMV6K> core1;
        auto_ptr<ARMV6K> core2;
        auto_ptr<ARMV6K> core3;

    private:
        unsigned int n_cores;
};

#endif
