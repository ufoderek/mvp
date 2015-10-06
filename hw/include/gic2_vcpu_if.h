#ifndef _GIC2_VCPU_IF_H_
#define _GIC2_VCPU_IF_H_

#include <stdint.h>
#include <systemc.h>
#include <boost/shared_ptr.hpp>
#include <ahb_slave_if.h>
#include <gic2_vctrl.h>
#include <gic2_cpu_if.h>

class gic2_vcpu_if: public ahb_slave_if, public sc_module
{
    public:
        sc_in_clk clk;

        sc_out<bool> virq_n;

        SC_HAS_PROCESS(gic2_vcpu_if);
        gic2_vcpu_if(sc_module_name name, uint32_t mapping_size, boost::shared_ptr<gic2_vctrl>gic_vctrl, boost::shared_ptr<gic2_cpu_if>gic_cpu_if);
        ~gic2_vcpu_if();

        void forward_pending_virq();

        bool read(uint32_t* data, uint32_t offset);
        bool write(uint32_t data, uint32_t offset);
        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
        bool busy;
        bool group0_en;
        bool group1_en;
        int fw_list;

        boost::shared_ptr<gic2_vctrl>gic_vctrl;
        boost::shared_ptr<gic2_cpu_if>gic_cpu_if;
};

#endif

