#ifndef _ARMV6_H_
#define _ARMV6_H_

#include <armv5.h>

class ARMV6: public ARMV5
{
        /* basic */
    public:
        SC_HAS_PROCESS(ARMV6);
        ARMV6(sc_module_name name, uint32_t addr, bool using_gdb, unsigned int gdb_port);
        ~ARMV6(void);

        /* mmu */
    protected:
        uint32_t table_1_addr;
        uint32_t table_1;
        uint32_t table_2_addr;
        uint32_t table_2;

        int gen_fault(int fault, uint8_t domain, bool write);
        bool permission(bool apx, uint8_t ap, uint8_t mode, bool write);
        virtual int vir2phy(uint32_t vir, uint32_t* phy, uint8_t mode, bool write, bool* cache);
        virtual int vir2phy_v6(uint32_t vir, uint32_t* phy, uint8_t mode, bool write, bool* cache);
};

#endif

