#ifndef _GIC2_VCTRL_H_
#define _GIC2_VCTRL_H_

#include <stdint.h>
#include <systemc.h>
#include <ahb_slave_if.h>
#include <bit_opt_v2.h>

enum list_reg_state
{
    LR_Invalid = B(00),
    LR_Pending = B(01),
    LR_Active = B(10),
    LR_PendingActive = B(11)
};

class list_reg
{
    public:
        bool valid;
        bool hw;
        bool group1;
        list_reg_state state;
        uint32_t priority;
        uint32_t pid;
        uint32_t vid;
};

class gic2_vctrl: public ahb_slave_if, public sc_module
{
    public:
        SC_HAS_PROCESS(gic2_vctrl);

        gic2_vctrl(sc_module_name name, uint32_t mapping_size);
        ~gic2_vctrl();

        bool read(uint32_t* data, uint32_t offset);
        bool write(uint32_t data, uint32_t offset);

        bool en;
        list_reg list[64];

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
};

#endif
