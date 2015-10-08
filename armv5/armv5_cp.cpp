#include <armv5.h>
#include <bit_opt.h>

/* initialize the registers of CP15 coprocessor */
void ARMV5::cpInit(void)
{
    /* reference the ARMV5 manual to understand the setting of each register */

    /* register c0 */
    //cp15.c0_idcode = 0x41069265;
    //cp15.c0_cachetype = 0x1d112152; // seperate i/d-cache, 8kb 16-way d-cache and 16 kb 16-way i-cache, both with 8 words cache line

    cp15.c0_tcmstatus = 0x00010001;
    //cp15.c0_idcode = 0x41 << 24 | 0x1<<20 | 0x7<<16 | 0xb36<<4 | 0x3 ;
    cp15.c0_cpuid = 0;

    /* register c1 */
    cp15.c1_sys = 0x00090078;

    /* register c2 */
    cp15.c2_ttbr = 0;
    cp15.c2_ttbr1 = 0;
    cp15.c2_ttbcr = 0;

    /* register c3 */
    for (int i = 0; i < 16; i++) {
        cp15.c3_domain[i] = 0;
    }

    /* register c5 */
    cp15.c5_dfsr = 0;
    cp15.c5_ifsr = 0;

    /* register c6 */
    cp15.c6_far = 0;

    /* register c7 */

    /* register c8 */

    /* register c9 */

    /* register c10 */

    /* register c13 */
    cp15.c13_fcse = 0;
    cp15.c13_context = 0;
    cp15.c13_context_id1 = 0;
    cp15.c13_context_id2 = 0;
    cp15.c13_context_id3 = 0;

}

/* read the registers of cp15 */
bool ARMV5::cpRead(uint32_t* data, uint16_t cn, uint16_t cm, uint8_t op)
{
    /********************************************************************************
     * op
     * bit 0 ~ 3 --> op2
     * bit 4 ~ 6 --> op1
     ********************************************************************************/

    bool result = true;
    uint8_t op1 = op >> 3;
    uint8_t op2 = op & 0x0007;

    switch (cn) {
        case 0: // CRn 0

            if (op1 != 0) {
                printb(core_id, d_armv5_cp, "c0 error op1 = %d", op1);
            }

            switch (cm) {
                case 0: // 0 - 0

                    switch (op2) {
                        case 0:
                            printd(core_id, d_armv5_cp, "read c0_idcode, pc = 0x%X", rf.pc);
                            *data = cp15.c0_idcode;
                            break;
                        case 1:
                            printd(core_id, d_armv5_cp, "read c0_cachetype, pc = 0x%X", rf.pc);
                            *data = cp15.c0_cachetype;
                            break;
                        case 2:
                            printd(core_id, d_armv5_cp, "read c0_tcmstatus, pc = 0x%X", rf.pc);
                            *data = cp15.c0_tcmstatus;
                            break;
                            /*
                                                   case 4:
                            printm(core_id,d_armv5_cp, "read c0_mpu_type, pc = 0x%X", rf.pc);
                            *data = cp15.c0_mpu_type;
                            break;
                            */
                        case 5:
                            printd(core_id, d_armv5_cp, "read c0_cpuid, pc = 0x%X", rf.pc);
                            *data = cp15.c0_cpuid;
                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 CRn=0 CRm=0 op2=%d read failed", op2);
                            break;
                    }

                    break;
                case 1: // 0 - 1

                    switch (op2) {
                        case 4:
                            printd(core_id, d_armv5_cp, "read c0_mmfr0, pc = 0x%X", rf.pc);
                            *data = cp15.c0_mmfr0;
                            break;
                        case 5:
                            printd(core_id, d_armv5_cp, "read c0_mmfr1, pc = 0x%X", rf.pc);
                            *data = cp15.c0_mmfr1;
                            break;
                        case 6:
                            printd(core_id, d_armv5_cp, "read c0_mmfr2, pc = 0x%X", rf.pc);
                            *data = cp15.c0_mmfr2;
                            break;
                        case 7:
                            printd(core_id, d_armv5_cp, "read c0_mmfr3, pc = 0x%X", rf.pc);
                            *data = cp15.c0_mmfr3;
                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 CRn=0 CRm=1 op2=%d read failed", op2);
                            break;
                    }

                    break;
                default: // 0 - ?
                    printb(core_id, d_armv5_cp, "cp15 CRn=0 CRm=%d read failed", cm);
                    break;
            }

            break;

        case 1: // CRn 1

            if ((op1 != 0) || (cm != 0)) {
                printb(core_id, d_armv5_cp, "cpRead CRn 1 error");
            }

            switch (op2) {
                case 0:
                    //printm(core_id,d_armv5_cp, "read c1_sys, pc = 0x%X", rf.pc);
                    *data = cp15.c1_sys;
                    break;
                case 1:
                    printm(core_id, d_armv5_cp, "read c1_aux, pc = 0x%X", rf.pc);
                    *data = cp15.c1_aux;
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 CRn=1 read failed");
                    break;
            }

            break;

        case 2: // CRn 2

            if ((op1 != 0) || (cm != 0)) {
                printb(core_id, d_armv5_cp, "cpRead CRn 2 error");
            }

            switch (op2) {
                case 0:
                    printd(core_id, d_armv5_cp, "read c2_ttbr: 0x%X", cp15.c2_ttbr);
                    *data = cp15.c2_ttbr;
                    break;
                case 1:
                    printb(core_id, d_armv5_cp, "read c2_ttbr1 failed");
                    break;
                case 2:
                    printb(core_id, d_armv5_cp, "read c2_ttbcr failed");
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 register 2 read error");
                    break;
            }

            break;

            /* register c3 */
        case 3:

            if ((op1 != 0) || (cm != 0)) {
                printb(core_id, d_armv5_cp, "cpRead CRn 3 error");
            }

            switch (op & 0x0007) {
                case 0:
                    *data = 0;

                    for (int i = 0; i < 16; i++) {
                        *data |= (cp15.c3_domain[i] << (i << 1));
                    }

                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 register 3 read error");
                    break;
            }

            break;

            /* register c5 */
        case 5:

            if ((op1 != 0) || (cm != 0)) {
                printb(core_id, d_armv5_cp, "cpRead CRn 5 error");
            }

            switch (op & 0x0007) {
                case 0:
                    *data = cp15.c5_dfsr;
                    break;
                case 1:
                    *data = cp15.c5_ifsr;
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 register 5 read error");
                    break;
            }

            break;

            /* register c6 */
        case 6:

            if ((op1 != 0) || (cm != 0)) {
                printb(core_id, d_armv5_cp, "cpRead CRn 6 error");
            }

            switch (op & 0x0007) {
                case 0:
                    *data = cp15.c6_far;
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 register 6 read error");
                    break;
            }

            break;

            /* register c7 */
        case 7:

            switch (cm) {
                case 10:    // test & clean D-cache
                    printd(core_id, d_armv5_cp, "test & clean d-cache");
                    break;
                case 14:    // test, clean, & invalidate  D-cache
                    printd(core_id, d_armv5_cp, "test, clean & invalidate d-cache");
                    result = ((op & 0x0007) == 3);

                    if (result == true) {
                        CPSR_SET_Z(rf.cpsr);
                    }

                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 register 7 read error");
                    break;
            }

            break;

        case 13:

            if ((op1 != 0) || (cm != 0)) {
                printb(core_id, d_armv5_cp, "cpRead CRn 13 error");
            }

            switch (op & 0x0007) {
                case 0:
                    *data = cp15.c13_fcse;
                    break;
                case 1:
                    *data = cp15.c13_context;
                    break;
                case 2:
                    *data = cp15.c13_context_id1;
                    break;
                case 3:
                    *data = cp15.c13_context_id2;
                    break;
                case 4:
                    *data = cp15.c13_context_id3;
                    break;

                default:
                    printb(core_id, d_armv5_cp, "cp15 register 13 doesn't implemented opcode 0x%X (read)", op & 0x0007);
                    break;
            }

            break;

        default:
            printb(core_id, d_armv5_cp, "cp15c%d not implemented yet, read failed", cn);
            break;
    }

    return result;
}

/* write the registers of CP15 */
bool ARMV5::cpWrite(uint32_t data, uint16_t cn, uint16_t cm, uint8_t op)
{
    /********************************************************************************
     * op
     * bit 0 ~ 3 --> op2
     * bit 4 ~ 6 --> op1
     ********************************************************************************/

    static bool first_time = true;

    bool result = true;

    bool c1_mmu_old = cp15.c1_mmu();
    bool c1_a_old = cp15.c1_a();
    bool c1_xp_old = cp15.c1_xp();
    bool c1_u_old = cp15.c1_u();

    switch (cn) {
            /* register c0 */
        case 0:

            switch (op & 0x0007) {
                default:
                    printb(core_id, d_armv5_cp, "cp15 c0 write error, pc = 0x%X, inst = 0x%X, data = 0x%X, cm = %d", rf.pc, inst, data, op & 0x0007);
            }

            break;

            /* register c1 */
        case 1:

            switch (op & 0x0007) {
                case 0:
                    cp15.c1_sys = data;
                    mmu_enable = data & 0x0001;
                    icache_enable = (data >> 12) & 0x0001;
                    dcache_enable = (data >> 2) & 0x0001;
                    vectorHi = (data >> 13) & 0x0001;       // the base address of exception vectors

                    if ((cp15.c1_mmu() != c1_mmu_old) || first_time) {
                        printd(core_id, d_armv5_cp, "mmu %s, pc = 0x%X", cp15.c1_mmu() ? "on" : "off", rf.pc);
                    }

                    if (cp15.c1_force_ap()) {
                        printb(core_id, d_armv5_cp, "v6k c1_sys force_ap on");
                    }

                    if (cp15.c1_tex_remap()) {
                        printb(core_id, d_armv5_cp, "v6k c1_sys tex_remap on");
                    }

                    if (cp15.c1_nmfi()) {
                        printb(core_id, d_armv5_cp, "v6k c1_sys nmfi on");
                    }

                    if ((cp15.c1_xp() != c1_xp_old) || first_time) {
                        if (cp15.version() == 6) {
                            if (!cp15.c1_xp()) {
                                printd(core_id, d_armv5_cp, "v6 only, c1_xp off, Subpage AP enabled, old mode, pc = %X", rf.pc);
                            }

                            else {
                                printd(core_id, d_armv5_cp, "v6 only, c1_xp on, Subpage AP disabled, new mode, pc = %X", rf.pc);
                            }
                        }
                    }

                    if ((cp15.c1_a() != c1_a_old) || first_time) {
                        if (!cp15.c1_a()) {
                            if (cp15.version() == 5) {
                                printd(core_id, d_armv5_cp, "v5: c1_a off, alignment fault checking disable");
                            }

                            else {
                                printd(core_id, d_armv5_cp, "v6: c1_a off, alignment not strict, ignore");
                            }
                        }

                        else {
                            if (cp15.version() == 5) {
                                printd(core_id, d_armv5_cp, "v5: c1_a on, alignment fault checking enabled");
                                //cp15.c1_sys &= ~(M(1));
                            }

                            else {
                                printd(core_id, d_armv5_cp, "v6: c1_a on, strict alignment, fault = data abort");
                            }
                        }
                    }

                    if (cp15.c1_b()) {
                        printb(core_id, d_armv5_cp, "cp15.c1_b on, big-endian");
                    }

                    if ((cp15.c1_u() != c1_u_old) || first_time) {
                        if (cp15.version() == 6) {
                            if (!cp15.c1_u()) {
                                printd(core_id, d_armv5_cp, "v6 only, c1_u off, unaligned loads: rotate, legacy");
                            }

                            else {
                                printd(core_id, d_armv5_cp, "v6 only, c1_u on, unalgined loads/stores permitted, mixed-endian support");
                            }
                        }
                    }

                    if (cp15.c1_ee()) {
                        printb(core_id, d_armv5_cp, "cp15.c1_ee on");
                    }

                    first_time = false;
                    break;
                case 1:
                    printm(core_id, d_armv5_cp, "write to c1_aux = %X", data);
                    cp15.c1_aux = data;
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 c1 write failed, op: %d", op & 0x0007);
                    break;
            }

            break;

            /* register c2 */
        case 2:

            switch (op & 0x0007) {
                case 0:
                    printd(core_id, d_armv5_cp, "write c2_ttbr0: 0x%X", data);
                    cp15.c2_ttbr = data;            // update the page table base address in clint machine
                    //page_table = bus_lookup(data);    // update the page table base address in host machine
                    break;
                case 1:
                    printd(core_id, d_armv5_cp, "write to c2_ttbr1: 0x%X", data);
                    cp15.c2_ttbr1 = data;
                    break;
                case 2:
                    printd(core_id, d_armv5_cp, "write to c2_ttbcr: 0x%X", data);
                    cp15.c2_ttbcr = data;
                    data &= MM(2, 0);

                    if (data != 0) {
                        printb(core_id, d_armv5_cp, "ttbcr != 0");
                    }

                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 CRn=2 write failed");
                    break;
            }

            break;

            /* register c3 */
        case 3:

            switch (op & 0x0007) {
                case 0:

                    for (int i = 0; i < 16; i++) {
                        cp15.c3_domain[i] = (data >> (i << 1)) & 0x0003;
                    }

                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 CRn=3 write failed");
                    break;
            }

            break;

            /* register c5 */
        case 5:

            switch (op & 0x0007) {
                case 0:
                    cp15.c5_dfsr = data;
                    break;
                case 1:
                    cp15.c5_ifsr = data;
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 c5 write failed");
                    break;
            }

            break;

            /* register c6 */
        case 6:

            switch (op & 0x0007) {
                case 0:
                    cp15.c6_far = data;
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 c6 write failed");
                    break;
            }

            break;

            /* register c7 */
        case 7:

            switch (cm) {
                case 0:         // wait for interrupt
                    printd(core_id, d_armv5_cp, "wait for interrupt");
                    break;
                case 5:         // invalidate I-cache

                    switch (op & 0x0007) {
                        case 0:
                        case 1:
                        case 2:
                            printd(core_id, d_armv5_cp, "invalidate i-cache or i-cache line");

                            for (int i = 0; i < CACHE_WAY; i++)
                                for (int j = 0; j < CACHE_SET; j++) {
                                    icache[i][j].valid = false;
                                }

                            break;
                        case 4:
                            printd(core_id, d_armv5_cp, "flush prefetch buffer");
                            break;
                        case 6:
                            printd(core_id, d_armv5_cp, "flush entire branch target cache");
                            break;
                        case 7:
                            printd(core_id, d_armv5_cp, "flush branch target cache entry");
                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 c7-5 write error");

                    }

                    break;
                case 6:         // invalidate D-cache

                    switch (op & 0x0007) {
                        case 0:
                        case 1:
                        case 2:
                            printd(core_id, d_armv5_cp, "invalidate d-cache or d-cache line");

                            for (int i = 0; i < CACHE_WAY; i++)
                                for (int j = 0; j < CACHE_SET; j++) {
                                    dcache[i][j].valid = false;
                                }

                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 c7-6 write error");
                    }

                    break;
                case 7:         // invalidate I-cache & D-cache

                    switch (op & 0x0007) {
                        case 0:
                            printd(core_id, d_armv5_cp, "invalidate i/d-cache");

                            for (int i = 0; i < CACHE_WAY; i++)
                                for (int j = 0; j < CACHE_SET; j++) {
                                    icache[i][j].valid = false;
                                    dcache[i][j].valid = false;
                                }

                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 c7-7 write error");
                    }

                    break;
                case 10:

                    switch (op & 0x0007) {
                        case 0:
                        case 1:
                        case 2:
                            printd(core_id, d_armv5_cp, "c7-10, clean d-cache or d-cache line, fake operation here");
                            break;
                        case 4:
                            printd(core_id, d_armv5_cp, "c7-10, old: drain write buffer, new: data synchronization barrier");
                            break;
                        case 5:
                            printd(core_id, d_armv5_cp, "c7-10, data memory barrier");
                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 c7-10 write error");
                            break;
                    }

                    break;
                case 14:

                    switch (op & 0x0007) {
                        case 0:
                        case 1:
                        case 2:
                            printd(core_id, d_armv5_cp, "invalidate d-cache or d-cache line");

                            for (int i = 0; i < CACHE_WAY; i++)
                                for (int j = 0; j < CACHE_SET; j++) {
                                    dcache[i][j].valid = false;
                                }

                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 c7-14 write error");
                            break;
                    }

                    break;
                case 15:

                    switch (op & 0x0007) {
                        case 0:
                        case 1:
                        case 2:
                            printd(core_id, d_armv5_cp, "invalidate unified d-cache or d-cache line");

                            for (int i = 0; i < CACHE_WAY; i++)
                                for (int j = 0; j < CACHE_SET; j++) {
                                    dcache[i][j].valid = false;
                                }

                            break;
                        default:
                            printb(core_id, d_armv5_cp, "cp15 c7-15 write error");
                            break;
                    }

                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 c7 write failed, cm: %d, op2: %d", cm, op & 0x0007);
                    break;
            }

            break;

            /* register c8 */
        case 8:

            switch (cm) {
                case 5: // invalidate I-TLB
                    printd(core_id, d_armv5_cp, "invalidate i-tlb");
                    break;
                case 6: // invalidate D-TLB
                    printd(core_id, d_armv5_cp, "invalidate d-tlb");
                    break;
                case 7: // invalidate I-TLB & D-TLB
                    printd(core_id, d_armv5_cp, "invalidate i/d-tlb");
                    break;
                default:
                    printb(core_id, d_armv5_cp, "cp15 c8 write failed");
                    break;
            }

            break;

            /* register c9 */

            /* register c10 */

            /* register c13 */
        case 13:

            switch (op & 0x0007) {
                case 0:
                    cp15.c13_fcse = data;
                    break;
                case 1:
                    cp15.c13_context = data;
                    break;
                case 2:
                    cp15.c13_context_id1 = data;
                    break;
                case 3:
                    cp15.c13_context_id2 = data;
                    break;
                case 4:
                    cp15.c13_context_id3 = data;
                    break;

                default:
                    printb(core_id, d_armv5_cp, "cp15 CRn=13 write failed");
                    break;
            }

            break;

            /* no the assigned control register */
        default:
            printb(core_id, d_armv5_cp, "cp15c%d not implemented yet, write failed", cn);
            break;
    }

    return result;
}
