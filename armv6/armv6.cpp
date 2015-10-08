#include <armv6.h>

ARMV6::ARMV6(sc_module_name name, uint32_t addr, bool using_gdb, unsigned int gdb_port): ARMV5(name, addr, using_gdb, gdb_port)
{
    this->core_id = 0;
    cp15.c0_idcode = 0x41 << 24 | 0x0 << 20 | 0x7 << 16 | 0xb36 << 4 | 0x2 ;
    cp15.c1_sys &= M(23);
    cp15.c0_cachetype = 0x1dd20d2;
}

ARMV6::~ARMV6()
{
}

