#include <armv6k.h>
#include <debug_utils.h>

ARMV6K::ARMV6K(sc_module_name name, unsigned int core_id, boost::shared_ptr<mp_scu> scu, boost::shared_ptr<gic2_dist> gic_dist, boost::shared_ptr<gic2_cpu_if> gic_cpu_if, uint32_t addr, bool using_gdb, unsigned int gdb_port): ARMV6(name, addr, using_gdb, gdb_port)
{
    printm(d_armv6k, "armv6k, core: %u", core_id);
    this->core_id = core_id;

    this->scu = scu;
    this->gic_dist = gic_dist;
    this->gic_cpu_if = gic_cpu_if;

    cp15.c0_cpuid = this->core_id & 0xF;
    cp15.c0_idcode = 0x41 << 24 | 0x0 << 20 | 0xf << 16 | 0xB02 << 4 | 0x2 ;
    cp15.c0_cachetype = 0x1dd20d2;
    cp15.c1_sys = 0x00054078;
    cp15.c1_aux = 0xF;
    cp15.c0_mmfr0 = 1 << 24 | 1 << 20 | 1 << 8 | 3;
    cp15.c0_mmfr1 = 1 << 28 | 2 << 16 | 3 << 8 | 2;
    cp15.c0_mmfr2 = 0x01222000;;
    cp15.c0_mmfr3 = 0;
}

ARMV6K::~ARMV6K()
{
}

bool ARMV6K::bus_read(uint32_t* data, uint32_t addr, unsigned int length)
{
    bool success;

    if ( (addr & MM(31, 13)) == 0x10100000) {
        uint32_t offset = addr & MM(12, 0);

        if ( (offset <= 0x01FF) && (offset >= 0x0100) ) {
            success  = gic_cpu_if->read(core_id, data, addr & 0xFF);
        }

        else if ( (offset <= 0x1FFF) && (offset >= 0x1000)) {
            success  = gic_dist->read(core_id, data, addr & 0xFFF);
        }

        else {
            success = scu->read(core_id, data, addr, length);
        }
    }

    else {
        success = scu->read(core_id, data, addr, length);
    }

    if (!success) {
        printm(core_id, d_armv6k, "bus read error, phy_addr = 0x%X, pc = 0x%X", addr, rf.pc);
    }

    return success;
}

bool ARMV6K::bus_write(uint32_t data, uint32_t addr, unsigned int length)
{
    bool success;

    if ( (addr & MM(31, 13) ) == 0x10100000) {
        uint32_t offset = addr & MM(12, 0);

        if ( (offset <= 0x01FF) && (offset >= 0x0100)) {
            success  = gic_cpu_if->write(core_id, data, addr & 0xFF);
        }

        else if ( (offset <= 0x1FFF) && (offset >= 0x1000)) {
            success  = gic_dist->write(core_id, data, addr & 0xFFF);
        }

        else {
            success = scu->write(this->core_id, data, addr, length);
        }
    }

    else {
        success = scu->write(this->core_id, data, addr, length);
    }

    if (!success) {
        printm(core_id, d_armv6k, "bus write error: 0x%X, phy_addr = 0x%X, pc = 0x%X", data, addr, rf.pc);
    }

    return success;
}

