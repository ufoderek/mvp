#include <armv6.h>
#include <armv5_defs.h>
#include <bit_opt.h>

int ARMV6::vir2phy(uint32_t vir, uint32_t* phy, uint8_t mode, bool write, bool* cache)
{
    //if (cp15.c1_xp() && (cp15.c2_ttbcr!=0)) {
    if (cp15.c1_xp()) {
        return vir2phy_v6(vir, phy, mode, write, cache);
    }

    else {
        return ARMV5::vir2phy(vir, phy, mode, write, cache);
    }
}

int ARMV6::gen_fault(int fault, uint8_t domain, bool write)
{
    fault |= ((domain & MM(3, 0)) << 4);

    if (write) {
        fault &= M(11);
    }

    return fault;
}

/* memory access permission check */
bool ARMV6::permission(bool apx, uint8_t ap, uint8_t mode, bool write)
{
    mode &= 0x001f;

    if (!apx) {
        switch (ap) {
            case 0:
                return false;
            case 1:

                if (mode == MODE_USR) {
                    return false;
                }

                return true;
            case 2:

                if (write && (mode == MODE_USR)) {
                    return false;
                }

                return true;
            case 3:
                return true;
            default:
                printb(core_id, d_armv6_mmu, "permission error: ap");
        }
    }

    else {
        switch (ap) {
            case 0:
                printb(core_id, d_armv6_mmu, "access permission fault");
                break;
            case 1:

                if ((mode == MODE_USR) || write) {
                    return false;
                }

                return true;
            case 2:
            case 3:

                if (write) {
                    return false;
                }

                return true;
            default:
                printb(core_id, d_armv6_mmu, "permission error: ap");
        }
    }

    printb(core_id, d_armv6_mmu, "permission error");
    return false;
}

int ARMV6::vir2phy_v6(uint32_t vir, uint32_t* phy, uint8_t mode, bool write, bool* cache)
{
    static bool first_time = true;

    if (first_time) {
        printm(core_id, d_mmu, "v6 mmu first time, pc = 0x%X", rf.pc);
        first_time = false;
    }

    int fault = FAULT_NONE;
    uint32_t table_1_addr;
    uint32_t table_2_base_addr;
    uint32_t table_2_offset;
    uint32_t table_2_addr;
    uint32_t table_1;
    uint32_t table_2;                       // second level page descriptor
    uint32_t domain;
    bool apx = false;
    uint8_t ap = 0;

    // cp15.c2_ttbr: base address of first-level table

    table_1_addr = (cp15.c2_ttbr & 0xFFFFC000) + ((vir >> 18) & (0xFFFFFFFF << 2));
    bus_read(&table_1, table_1_addr, 4);
    domain = (table_1 >> 5) & 0x000f;

    switch (table_1 & 0x03) {
        case 1:     // coarse page
            table_2_base_addr = table_1 & (0xFFFFFFFF << 10);
            table_2_offset = (vir >> 10) & 0x000003FC;
            table_2_addr = table_2_base_addr + table_2_offset;
            bus_read(&table_2, table_2_addr, 4);

            ap = (table_2 >> 4) & 0x03;
            apx = (table_2 >> 9) & 0x01;
            *cache = (table_2 >> 3) & 0x03;

            switch (table_2 & 0x03) {
                case 1:     // large page

                    switch (cp15.c3_domain[domain]) {
                        case 1:     // client

                            if (permission(apx, ap, mode, write)) {
                                *phy = (table_2 & 0xffff0000) + (vir & 0x0000ffff);
                                fault = FAULT_NONE;
                            }

                            else {
                                *phy = 0;
                                fault = FAULT_PAGEPERMI | (domain << 4);
                            }

                            break;
                        case 3:     // manager
                            *phy = (table_2 & 0xffff0000) + (vir & 0x0000ffff);
                            fault = FAULT_NONE;
                            break;
                        default:    // no access, page domain fault
                            *phy = 0;
                            fault = FAULT_PAGEDOMAIN | (domain << 4);
                            break;
                    }

                    break;
                case 2:     // small page
                case 3:

                    switch (cp15.c3_domain[domain]) {
                        case 1:     // client

                            if (permission(apx, ap, mode, write)) {
                                *phy = (table_2 & 0xfffff000) + (vir & 0x00000fff);
                                fault = FAULT_NONE;
                            }

                            else {
                                *phy = 0;
                                fault = FAULT_PAGEPERMI | (domain << 4);
                            }

                            break;
                        case 3:     // manager
                            *phy = (table_2 & 0xfffff000) + (vir & 0x00000fff);
                            fault = FAULT_NONE;
                            break;
                        default:    // no access, page domain fault
                            *phy = 0;
                            fault = FAULT_PAGEDOMAIN | (domain << 4);
                            break;
                    }

                    break;
                case 0:    // page translation fault
                    *phy = 0;
                    fault = FAULT_PAGETRANS | (domain << 4);
                    break;
            }

            break;
        case 2:     // section

            if (table_1 & M(18)) {
                printb(core_id, d_armv6_mmu, "super section");
            }

            ap = (table_1 >> 10) & 0x03;
            apx = (table_1 >> 15) & 0x01;
            *cache = (table_1 >> 3) & 0x01;

            switch (cp15.c3_domain[domain]) {
                case 1:     // client

                    if (permission(apx, ap, mode, write)) {
                        *phy = (table_1 & 0xfff00000) + (vir & 0x000fffff);
                        fault = FAULT_NONE;
                    }

                    else {
                        *phy = 0;
                        fault = FAULT_SECTPERMI | (domain << 4);
                    }

                    break;
                case 3:     // manager
                    *phy = (table_1 & 0xfff00000) + (vir & 0x000fffff);
                    fault = FAULT_NONE;
                    break;
                default:    // no access, section domain fault
                    *phy = 0;
                    fault = FAULT_SECTDOMAIN | (domain << 4);
                    break;
            }

            break;
        case 3:
            printb(core_id, d_armv6_mmu, "error page mode");
            break;
        default:    // section translation fault
            *phy = 0;
            fault = FAULT_SECTTRANS | (domain << 4);
            break;
    }

    return fault;
}


