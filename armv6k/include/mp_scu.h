#ifndef _MP_SCU_H_
#define _MP_SCU_H_

#include <stdint.h>
#include <ahb_master_if.h>

class scu_registers
{
    public:
        scu_registers() {
            control = 3;
        }
        uint32_t control;
};

class mp_scu: public ahb_master_if, public sc_module
{
    public:
        bool event_reg;
        unsigned int n_cores;

        SC_HAS_PROCESS(mp_scu);
        mp_scu(sc_module_name name, unsigned int n_cores);
        ~mp_scu();

        bool read(unsigned int core_id, uint32_t* data, uint32_t addr, unsigned int length);
        bool write(unsigned int core_id, uint32_t data, uint32_t addr, unsigned int length);

    protected:
        scu_registers scu_regs;

        bool bus_read(uint32_t* data, uint32_t addr, unsigned int length);
        bool bus_write(uint32_t data, uint32_t addr, unsigned int length);

        bool scu_read(unsigned int core_id, uint32_t* data, uint32_t offset);
        bool scu_write(unsigned int core_id, uint32_t data, uint32_t offset);
};

#endif

