#include <armv5.h>
#include <bit_opt.h>

bool ARMV5::bus_read(uint32_t* data, uint32_t addr, unsigned int length)
{
    bool success;

    /*
    printd(core_id, d_armv5_bus, "read req");
    req = true;
    wait(grant.posedge_event());
    printd(core_id, d_armv5_bus, "read req granted");
    */
    success = bus_b_access(false, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(data), length);
    /*
    req = false;
    printd(core_id, d_armv5_bus, "read done");
    done = true;
    wait(clk_period_ns, SC_NS);
    done = false;
    */

    if (success) {
        return true;
    }

    printm(core_id, d_armv5_bus, "bus_read failed");
    return false;
}

bool ARMV5::bus_write(uint32_t data, uint32_t addr, unsigned int length)
{
    bool success;

    /*
    printd(core_id, d_armv5_bus, "write req");
    req = true;
    wait(grant.posedge_event());
    printd(core_id, d_armv5_bus, "write req granted");
    */
    success = bus_b_access(true, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(&data), length);
    /*
    req = false;
    printd(core_id, d_armv5_bus, "write done");
    done = true;
    wait(clk.period());
    done = false;
    */

    if (success) {
        return true;
    }

    printm(core_id, d_armv5_bus, "bus_write failed");
    return false;
}

unsigned int ARMV5::get_word_size(word_type wtype)
{
    switch (wtype) {
        case type_byte:
        case type_bsync:
            return 1;
        case type_hword:
            return 2;
        case type_wload:
        case type_wstore:
        case type_wsync:
        case type_mword:
        case type_dword:
            return 4;
        default:
            printb(core_id, d_armv5_bus, "error size of word_type");
    }
}

bool ARMV5::check_alignment(uint32_t addr, word_type wtype)
{
    bool unaligned;

    switch (wtype) {
        case type_byte:
        case type_bsync:
            return true;

        case type_hword:
            unaligned = addr & M(0);

            if (unaligned) {
                if (cp15.version() == 5) {
                    printb(core_id, d_armv5_bus, "check alignment failed, type_hword v5");
                }

                else if (cp15.version() == 6) {
                    if (cp15.c1_a()) {
                        return false;
                    }

                    else if (cp15.c1_u()) {
                        printb(core_id, d_armv5_bus, "check alignment failed, type_hword v6");
                        return true;
                    }

                    else {
                        printb(core_id, d_armv5_bus, "check alignment failed, type_hword v6-2");
                    }
                }

                else {
                    printb(core_id, d_armv5_bus, "check alignment failed, type_hword version");
                }
            }

            return true;

        case type_wload:
            unaligned = addr & MM(1, 0);

            if (unaligned) {
                if (cp15.version() == 5) {
                    printb(core_id, d_armv5_bus, "check_alignment failed, type_wload v5");
                }

                else if (cp15.version() == 6) {
                    if (cp15.c1_a()) {
                        return false;
                    }

                    else if (cp15.c1_u()) {
                        printb(core_id, d_armv5_bus, "check_alignment failed, type_wload v6");
                        return true;
                    }

                    else {
                        printb(core_id, d_armv5_bus, "check_alignment failed, type_wload v6-2");
                    }
                }

                else {
                    printb(core_id, d_armv5_bus, "check alignment failed, type_wload version");
                }
            }

            return true;

        case type_wstore:
            unaligned = addr & MM(1, 0);

            if (unaligned) {
                if (cp15.version() == 5) {
                    printb(core_id, d_armv5_bus, "check_alignment failed, type_wstore v5");
                }

                else if (cp15.version() == 6) {
                    if (cp15.c1_a()) {
                        return false;
                    }

                    else if (cp15.c1_u()) {
                        printb(core_id, d_armv5_bus, "check_alignment failed, type_wstore v6");
                        return true;
                    }

                    else {
                        printb(core_id, d_armv5_bus, "check_alignment failed, type_wstore v6-2");
                    }
                }

                else {
                    printb(core_id, d_armv5_bus, "check alignment failed, type_wstore version");
                }
            }

            return true;

        case type_wsync:
            unaligned = addr & MM(1, 0);

            if (unaligned) {
                if (cp15.c1_u() || cp15.c1_a()) {
                    printb(core_id, d_armv5_bus, "check alginment failed: type_wsync");
                    return false;
                }

                else {
                    printb(core_id, d_armv5_bus, "check alginment failed: type_wsync 2");
                }
            }

            return true;

        case type_mword:
            unaligned = addr & MM(1, 0);

            if (unaligned) {
                if (cp15.c1_u()) {
                    printb(core_id, d_armv5_bus, "check alignment failed: type_mword, you should clear bit[1:0] yourself");
                    return false;
                }

                else {
                    // see ARM Ref Manual about ldm and stm
                    printb(core_id, d_armv5_bus, "check alignment failed: type_mword, you should clear bit[1:0] yourself 2");
                }
            }

            return true;

        case type_dword:

            if (cp15.version() == 5) {
                unaligned = addr & MM(2, 0);

                if (unaligned) {
                    printb(core_id, d_armv5_bus, "check_alignment failed, type_dword v5");
                }

                else {
                    return true;
                }
            }

            else if ((cp15.version() == 6) || (cp15.version() == 0xF)) {
                unaligned = addr & MM(1, 0);

                if (unaligned) {
                    printb(core_id, d_armv5_bus, "check_alignment failed, type_dword v6");
                }

                else {
                    return true;
                }
            }

            else {
                printb(core_id, d_armv5_bus, "check alignment failed, type_dword version");
            }

        default:
            printb(core_id, d_armv5_bus, "check alignment failed, no such type");
    }
}

/* read data from bus */
bool ARMV5::data_read(uint32_t* data, uint32_t vir, uint8_t mode, word_type wtype)
{
    uint32_t phy = 0;
    int fault = FAULT_NONE;
    bool cachable = true;

    /* check rwatch point */
    if (unlikely(using_gdb)) {
        if (unlikely(rwatchCheck(vir))) {
            gdb->reason = GDB_SIGNAL_TRAP;
            gdb->step = false;
            gdb->stop = true;
        }
    }

    /*
    static bool first = true;
    if( (rf.pc > 0xc0008a20) && first){
        setl(d_armv5_bus);
        first = false;
    }
    */

    bool check_1 = wtype == type_wsync;
    bool check_2 = wtype == type_mword;
    bool check_3 = wtype == type_dword;

    bool check_4 = cp15.c1_a();
    bool check_5 = check_1 || check_2 || check_3;

    if (check_4 || check_5) {
        if (!check_alignment(addr, wtype)) {
            fault = FAULT_ALIGN;
        }
    }

    /* address translation */
    if (mmu_enable) {
        int temp_fault = vir2phy(vir, &phy, mode, false, &cachable);

        if (fault == FAULT_NONE) {
            fault = temp_fault;
        }
    }

    else {
        phy = vir;
    }

    /* memory access */
    if (fault == FAULT_NONE) {
        printd(core_id, d_armv5_bus, "bus read 0x%X", phy);

        if (bus_read(data, phy, get_word_size(wtype))) {
            return true;
        }

        else {
            fault = FAULT_EXTERNAL;
        }
    }

    /* fault handler */
    cp15.c5_dfsr = fault;
    cp15.c6_far = vir;
    return false;
}

/* write data to bus */
bool ARMV5::data_write(uint32_t data, uint32_t vir, uint8_t mode, word_type wtype)
{
    uint32_t phy = 0;
    int fault = FAULT_NONE;
    bool cachable = true;

    /* check wwatch point */
    if (unlikely(using_gdb)) {
        if (unlikely(wwatchCheck(vir))) {
            gdb->reason = GDB_SIGNAL_TRAP;
            gdb->step = false;
            gdb->stop = true;
        }
    }

    bool check_1 = wtype == type_wsync;
    bool check_2 = wtype == type_mword;
    bool check_3 = wtype == type_dword;

    bool check_4 = cp15.c1_a();
    bool check_5 = check_1 || check_2 || check_3;

    if (check_4 || check_5) {
        if (!check_alignment(addr, wtype)) {
            fault = FAULT_ALIGN;
        }
    }

    /* address translation */
    if (mmu_enable) {
        int temp_fault = vir2phy(vir, &phy, mode, true, &cachable);

        if (fault == FAULT_NONE) {
            fault = temp_fault;
        }
    }

    else {
        phy = vir;
    }

    /* memory access */
    if (fault == FAULT_NONE) {
        printd(core_id, d_armv5_bus, "bus write 0x%X", phy);

        if (bus_write(data, phy, get_word_size(wtype))) {
            return true;
        }

        else {
            fault = FAULT_EXTERNAL;
            printb(core_id, d_armv5_bus, "data_write failed, vir = 0x%X, phy = 0x%X, data = 0x%X", vir, phy, data);
        }
    }

    /* fault handler */
    cp15.c5_dfsr = fault;
    cp15.c6_far = vir;
    return false;
}
