#include<armv5.h>

/* memory access permission check */
bool ARMV5::permission(uint8_t ap, uint8_t mode, bool write)
{
    if ((mode & 0x001f) == MODE_USR) {
        switch (ap & 0x03) {
            case 0:

                if ((((cp15.c1_sys >> 8) & 0x03) == 2) && !write) {
                    return true;
                }

                else {
                    return false;
                }

                break;
            case 1:
                return false;
                break;
            case 2:
                return (write) ? false : true;
                break;
            case 3:
                return true;
                break;
        }
    }

    else {
        if (ap & 0x03) {
            return true;
        }

        else {
            if ((((cp15.c1_sys >> 8) & 0x03) == 0) || write) {
                return false;
            }

            else {
                return true;
            }
        }
    }

    return true;
}

/* translate the virtual address to physical address */
int ARMV5::vir2phy(uint32_t vir, uint32_t* phy, uint8_t mode, bool write, bool* cache)
{
    static bool first_time = true;

    if (first_time) {
        printm(core_id, d_mmu, "v5 mmu first time, pc = 0x%X", rf.pc);
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
            *cache = (table_2 >> 3) & 0x03;

            switch (table_2 & 0x03) {
                case 1:     // large page

                    switch (cp15.c3_domain[domain]) {
                        case 1:     // client

                            if (permission(ap, mode, write)) {
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

                    switch (cp15.c3_domain[domain]) {
                        case 1:     // client

                            if (permission(ap, mode, write)) {
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
                default:    // page translation fault
                    *phy = 0;
                    fault = FAULT_PAGETRANS | (domain << 4);
                    break;
            }

            break;
        case 2:     // section
            ap = (table_1 >> 10) & 0x03;
            *cache = (table_1 >> 3) & 0x01;

            switch (cp15.c3_domain[domain]) {
                case 1:     // client

                    if (permission(ap, mode, write)) {
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
        case 3:     // fine page
            printb(core_id, d_armv5, "@0x%.8x fine page mode not implemented yet", rf.pc - 4);
            break;
        default:    // section translation fault
            *phy = 0;
            fault = FAULT_SECTTRANS | (domain << 4);
            break;
    }

    return fault;
}

