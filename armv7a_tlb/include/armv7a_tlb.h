#ifndef _ARMV7A_TLB_H_
#define _ARMV7A_TLB_H_

#include <armv7a.h>

class armv7a_tlb: public armv7a
{
    public:

        SC_HAS_PROCESS(armv7a_tlb);
        armv7a_tlb(sc_module_name name, const uint32_t& pc, uint32_t core_id,
                   const bool using_gdb, const unsigned int gdb_port);
        ~armv7a_tlb();

        //virtual armv7a_cp15_tlb cp15;
};

#endif
