#ifndef _GIC2_CPU_IF_H_
#define _GIC2_CPU_IF_H_

#include <stdint.h>
#include <systemc.h>
#include <boost/shared_ptr.hpp>
#include <ahb_slave_if.h>
#include <gic2_dist.h>

class cpu_if_info
{
    public:
        cpu_if_info()
        {
            en = false;
            busy = false;
            fw_ir = 0;
        }
        bool en;
        bool busy;
        uint32_t fw_ir;
};

class gic2_cpu_if: public ahb_slave_if, public sc_module
{
    public:
        sc_in_clk clk;

        sc_out<bool>*irq_n;          //array [cpu_count]

        SC_HAS_PROCESS(gic2_cpu_if);
        gic2_cpu_if(sc_module_name name, uint32_t mapping_size, boost::shared_ptr<gic2_dist>dist, uint32_t cpu_count, uint32_t ir_count);
        ~gic2_cpu_if();

        void forward_pending_irq();

        bool read(uint32_t cpu, uint32_t* data, uint32_t offset);
        bool write(uint32_t cpu, uint32_t data, uint32_t offset);
        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
        uint32_t cpu_count;
        uint32_t ir_count;

        cpu_if_info* if_infos;        //array [cpu_count]

        boost::shared_ptr<gic2_dist>dist;
};

#endif

