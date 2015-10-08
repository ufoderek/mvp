#include<armv5.h>

/* read register by GDB */
void ARMV5::regRead(uint32_t* data, uint16_t index)
{
    if (index <= FPS) {
        rfRead(data, index, CPSR_MODE(rf.cpsr));

        if (index == PC) {
            *data -= 4;
        }
    }

    else {
        switch (index) {
            case PID:
                *data = cp15.c13_context;
                break;
            case SYS:
                *data = cp15.c1_sys;
                break;
            case TTBR:
                *data = cp15.c2_ttbr;
                break;
                //          case DFSR:  *data = cp15.c5_dfsr; break;
                //          case IFSR:  *data = cp15.c5_ifsr; break;
                //          case FAR:   *data = cp15.c6_far; break;
            case DOM:
                *data = 0;

                for (int i = 0; i < 16; i++) {
                    *data |= (cp15.c3_domain[i] << (i * 2));
                }

                break;
                //case INST:    *data = inst_arm; break;
                //case CYCLE:   *data = cycle_total; break;
            case INST:
                *data = 0;
                printm(core_id, d_armv5, "gdb INST not implemented yet");
                break;
            case CYCLE:
                *data = 0;
                printm(core_id, d_armv5, "gdb CYCLE not implemented yet");
                break;
            default:
                *data = 0;
                printm(core_id, d_armv5, "no such register");
                break;
        }
    }
}

/* write register by external modules */
void ARMV5::regWrite(uint32_t data, uint16_t index)
{
    rfWrite(data, index, CPSR_MODE(rf.cpsr));
}

/* read memory by external modules */
void ARMV5::memRead(uint32_t* data, uint32_t vir, int size)
{
    /* NOTE: don't use data_read() function directly, or it probably brings some bizarre side effects */
    uint32_t phy = 0;
    bool cache = true;

    if (mmu_enable) {
        vir2phy(vir, &phy, CPSR_MODE(rf.cpsr), false, &cache);
    }

    else {
        phy = vir;
    }

    printm(core_id, d_armv5_gdb, "memRead, vir = 0x%X, phy = 0x%X", vir, phy);
    bus_read(data, phy, size);
    printd(core_id, d_armv5_gdb, "memRead return: 0x%X", *data);
}

/* write memory by external modules */
void ARMV5::memWrite(uint32_t data, uint32_t vir, int size)
{
    /* NOTE: don't use data_write() function directly, or it probably brings some bizarre side effects */
    uint32_t phy = 0;
    bool cache = true;

    if (mmu_enable) {
        vir2phy(vir, &phy, CPSR_MODE(rf.cpsr), true, &cache);
    }

    else {
        phy = vir;
    }

    bus_write(data, phy, size);
}

/* insert break point */
void ARMV5::bkptInsert(uint32_t addr)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        if (break_point[i].exist == false) {
            break_point[i].exist = true;
            break_point[i].addr = addr;
            break;
        }
    }
}

/* remove break point */
void ARMV5::bkptRemove(uint32_t addr)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        if ((break_point[i].exist == true) && (break_point[i].addr == addr)) {
            break_point[i].exist = false;
        }
    }
}

/* remove all break points */
void ARMV5::bkptRemove(void)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        break_point[i].exist = false;
    }
}

/* insert both rWatch and wWatch point */
void ARMV5::awatchInsert(uint32_t addr)
{
    rwatchInsert(addr);
    wwatchInsert(addr);
}

/* insert rWatch point */
void ARMV5::rwatchInsert(uint32_t addr)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        if (rwatch_point[i].exist == false) {
            rwatch_point[i].exist = true;
            rwatch_point[i].addr = addr;
            break;
        }
    }
}

/* insert wWatch point */
void ARMV5::wwatchInsert(uint32_t addr)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        if (wwatch_point[i].exist == false) {
            wwatch_point[i].exist = true;
            wwatch_point[i].addr = addr;
            break;
        }
    }
}

/* remove both rWatch and wWatch point */
void ARMV5::awatchRemove(uint32_t addr)
{
    rwatchRemove(addr);
    wwatchRemove(addr);
}

/* remove wWatch point */
void ARMV5::rwatchRemove(uint32_t addr)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        if ((rwatch_point[i].exist == true) && (rwatch_point[i].addr == addr)) {
            rwatch_point[i].exist = false;
        }
    }
}

/* remove wWatch point */
void ARMV5::wwatchRemove(uint32_t addr)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        if ((wwatch_point[i].exist == true) && (wwatch_point[i].addr == addr)) {
            wwatch_point[i].exist = false;
        }
    }
}

/* remove all watch points */
void ARMV5::watchRemove(void)
{
    for (int i = 0; i < SIZE_BKPT; i++) {
        rwatch_point[i].exist = false;
        wwatch_point[i].exist = false;
    }
}


/* check the break point reaching */
bool ARMV5::bkptCheck(uint32_t addr)
{
    bool result = false;

    for (int i = 0; i < SIZE_BKPT; i++) {
        if ((break_point[i].exist) && (break_point[i].addr == addr)) {
            result = true;
            break;
        }
    }

    return result;
}

/* check the read watch point reaching */
bool ARMV5::rwatchCheck(uint32_t addr)
{
    bool result = false;

    for (int i = 0; i < SIZE_BKPT; i++) {
        if ((rwatch_point[i].exist) && (rwatch_point[i].addr == addr)) {
            result = true;
            break;
        }
    }

    return result;
}

/* check the write watch point reaching */
bool ARMV5::wwatchCheck(uint32_t addr)
{
    bool result = false;

    for (int i = 0; i < SIZE_BKPT; i++) {
        if ((wwatch_point[i].exist) && (wwatch_point[i].addr == addr)) {
            result = true;
            break;
        }
    }

    return result;
}

