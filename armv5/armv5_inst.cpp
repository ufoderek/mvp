#include <armv5.h>
#include <cstdio>
#include <cstring>
#include <bit_opt.h>

void ARMV5::print_inst(const char* inst_name)
{
#ifndef RELEASE
    strcpy(current_inst_name, inst_name);
    printd(core_id, d_inst, "%s\t%8X", inst_name, inst);
#endif
}

void ARMV5::arm_adc(void)
{
    print_inst("adc");
    inst_assert(0, 27, 26);
    inst_assert(B8(0101), 24, 21);
    uint64_t result = (uint64_t)operand_1 + (uint64_t)operand_2 + (uint64_t)CPSR_C(rf.cpsr);
    rfWrite((uint32_t)result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            ((int32_t)result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (result >> 32) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

            /* overflow detection */
            if (((operand_1 >> 31) ^ (operand_2 >> 31)) != 0) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else {
                CPSR_SET_V(rf.cpsr);
            }
        }
    }
}

void ARMV5::arm_add(void)
{
    print_inst("add");
    inst_assert(0, 27, 26);
    inst_assert(B8(0100), 24, 21);
    uint64_t result = (uint64_t)operand_1 + (uint64_t)operand_2;
    rfWrite((uint32_t)result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            ((int32_t)result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (result >> 32) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

            /* overflow detection */
            if (((operand_1 >> 31) ^ (operand_2 >> 31)) != 0) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else {
                CPSR_SET_V(rf.cpsr);
            }
        }
    }

    printd(core_id, d_inst, "%X %X %X %X", operand_1, operand_2, result, rd);
}

void ARMV5::arm_and(void)
{
    print_inst("and");
    inst_assert(0, 27, 26);
    inst_assert(0, 24, 21);
    uint32_t result = operand_1 & operand_2;
    rfWrite(result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
        }
    }
}

void ARMV5::arm_bic(void)
{
    print_inst("bic");
    inst_assert(0, 27, 26);
    inst_assert(B8(1110), 24, 21);
    uint32_t result = operand_1 & ~operand_2;
    rfWrite(result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
        }
    }
}

void ARMV5::arm_bkpt(void)
{
    print_inst("bkpt");
    printb(core_id, d_inst, "bkpt not implemented yet");
}

void ARMV5::arm_blx(void)
{
    print_inst("blx");
    inst_assert(B8(1111101), 31, 25);
    printb(core_id, d_inst, "blx not implemented yet");
}

void ARMV5::arm_blx2(void)
{
    print_inst("blx2");
    inst_assert(B8(00010010), 27, 20);
    inst_assert(B16(1111, 11111111), 19, 8);
    inst_assert(B8(0011), 7, 4);
    uint32_t addr;

    rm = inst & 0x000f;
    rfRead(&addr, rm, CPSR_MODE(rf.cpsr));
    rfWrite(rf.pc, LR, CPSR_MODE(rf.cpsr));
    rf.pc = addr & 0xfffffffc;
    (addr & 0x0003) ? CPSR_SET_T(rf.cpsr) : CPSR_CLEAR_T(rf.cpsr);
}


void ARMV5::arm_bx(void)
{
    print_inst("bx");
    inst_assert(B8(00010010), 27, 20);
    inst_assert(B16(1111, 11111111), 19, 8);
    inst_assert(B8(0001), 7, 4);
    uint32_t addr;

    rm = inst & 0x000f;
    rfRead(&addr, rm, CPSR_MODE(rf.cpsr));
    rf.pc = addr & 0xfffffffc;
    (addr & 0x0003) ? CPSR_SET_T(rf.cpsr) : CPSR_CLEAR_T(rf.cpsr);
    /*
    uint8_t rm = SMM(inst, 3, 0);
    uint32_t rm_val;

    rfRead(&rm_val, rm, CPSR_MODE(rf.cpsr));
    rf.pc = rm_val & 0xFFFFFFFE;

    if( (rm_val & M(0)) ){
        printb(core_id, d_inst, "arm_bx with T flag");
    }
    */
}

void ARMV5::arm_bxj(void)
{
    print_inst("bxj");
    inst_assert(B8(00010010), 27, 20);
    inst_assert(B16(1111, 11111111), 19, 8);
    inst_assert(B8(0010), 7, 4);
    printb(core_id, d_inst, "bxj not implemented yet");
}

void ARMV5::arm_cdp(void)
{
    print_inst("cdp");
    inst_assert(B8(1110), 27, 24);
    inst_assert_unset(4);

    if (((inst >> 8) & 0x000f) != 15) {
        arm_except_undef();
    }

    else {
        cout << "ERROR: CDP instruction hasn't been implemented yet!!" << endl;
        exit(EXIT_SUCCESS);
    }
}

void ARMV5::arm_clrex()
{
    print_inst("clrex");
    inst_assert(B8(11110101), 31, 24);
    inst_assert(B8(01111111), 23, 16);
    inst_assert(B8(11110000), 15, 8);
    inst_assert(B8(00011111), 7, 0);
}

void ARMV5::arm_clz(void)
{
    print_inst("clz");
    inst_assert(B8(00010110), 27, 20);
    inst_assert(B8(1111), 19, 16);
    inst_assert(B8(1111), 11, 8);
    inst_assert(B8(0001), 7, 4);

    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rm, CPSR_MODE(rf.cpsr));

    if (operand_1 == 0) {
        rfWrite(32, rd, CPSR_MODE(rf.cpsr));
    }

    else {
        int i;

        for (i = 31; i >= 0; i--) {
            if ((operand_1 >> i) & 0x0001) {
                break;
            }
        }

        rfWrite(31 - i, rd, CPSR_MODE(rf.cpsr));
    }
}

void ARMV5::arm_cmn(void)
{
    print_inst("cmn");
    inst_assert(B8(00), 27, 26);
    inst_assert(B8(10111), 24, 20);
    inst_assert(0, 15, 12);

    /* the rd should be 0 here */
    /* anyway, we suppose it never occur and ignore this condition */
    if (update == true) {
        uint64_t result = (uint64_t)operand_1 + (uint64_t)operand_2;
        ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        ((int32_t)result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
        (result >> 32) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

        /* overflow detection */
        if (((operand_1 >> 31) ^ (operand_2 >> 31)) != 0) {
            CPSR_CLEAR_V(rf.cpsr);
        }

        else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
            CPSR_CLEAR_V(rf.cpsr);
        }

        else {
            CPSR_SET_V(rf.cpsr);
        }
    }
}

void ARMV5::arm_cmp(void)
{
    print_inst("cmp");
    inst_assert(B8(00), 27, 26);
    inst_assert(B8(10101), 24, 20);
    inst_assert(0, 15, 12);
    rn = (inst & MM(19, 16)) >> 16;

    if (update == true) {
        uint32_t operand_tmp = -operand_2;
        uint64_t result = (uint64_t)operand_1 + (uint64_t)operand_tmp;
        ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (((int32_t)result) == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
        (operand_1 >= operand_2) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

        uint32_t sign_bit_a  = (operand_1 >> 31) & 0x01;
        uint32_t sign_bit_b  = (operand_tmp >> 31) & 0x01;
        uint32_t sign_bit_c  = (result >> 31) & 0x01;

        if ( (sign_bit_a == sign_bit_b) && (sign_bit_a != sign_bit_c)) {
            CPSR_SET_V(rf.cpsr);
        }

        else {
            CPSR_CLEAR_V(rf.cpsr);
        }

        /*
        // overflow detection
        if (((operand_1 >> 31) ^(operand_tmp >> 31)) != 0) {
            CPSR_CLEAR_V(rf.cpsr);
        }
        else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
            CPSR_CLEAR_V(rf.cpsr);
        }
        else {
            CPSR_SET_V(rf.cpsr);
        }
        */
    }

    /*
    OverflowFrom
    Returns 1 if the addition or subtraction specified as its parameter caused a 32-bit signed overflow.

    Addition generates an overflow if both operands have the same sign (bit[31]), and the sign of the result is different to
    the sign of both operands.

    Subtraction causes an overflow if the operands have different signs, and the first
    operand and the result have different signs.

    This delivers further information about an addition or subtraction which occurred earlier in the pseudo-code.
    The addition or subtraction is not repeated.
    */
}

void ARMV5::arm_cps(void)
{
    print_inst("cps");
    inst_assert(B16(1111, 00010000), 31, 20);
    inst_assert_unset(16);
    inst_assert(0, 15, 9);
    inst_assert_unset(5);

    //printb(core_id, d_armv6, "check arm_cps again");
    if (CPSR_MODE(rf.cpsr) == MODE_USR) {
        printb(core_id, d_armv6, "arm_cps in user mode");
    }

    /* change the AIF bits in CPSR */
    if (inst & M(19)) {
        if (inst & M(8)) {
            if (inst & M(18)) {
                CPSR_SET_A(rf.cpsr);
            }

            else {
                CPSR_CLEAR_A(rf.cpsr);
            }
        }

        if (inst & M(7)) {
            if (inst & M(18)) {
                printd(core_id, d_armv5, "enable interrupt");
                CPSR_SET_I(rf.cpsr);
            }

            else {
                printd(core_id, d_armv5, "disable interrupt");
                CPSR_CLEAR_I(rf.cpsr);
            }
        }

        if (inst & M(6)) {
            if (inst & M(18)) {
                CPSR_SET_F(rf.cpsr);
            }

            else {
                CPSR_CLEAR_F(rf.cpsr);
            }
        }
    }

    /* change CPU mode */
    if (inst & M(17)) {
        rf.cpsr &= 0xffffffe0;
        rf.cpsr |= (inst & 0x1F);
    }
}

void ARMV5::arm_cpy(void)
{
    print_inst("cpy");
    inst_assert(B8(00011010), 27, 20);
    inst_assert(0, 19, 16);
    inst_assert(0, 11, 4);

    uint32_t tmp;
    rm = inst & MM(3, 0);
    rd = (inst & MM(15, 12)) >> 12;
    rfRead(&tmp, rm, CPSR_MODE(rf.cpsr));
    rfWrite(tmp, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_eor(void)
{
    print_inst("eor");
    inst_assert(0, 27, 26);
    inst_assert(B8(0001), 24, 21);
    uint32_t result = operand_1 ^ operand_2;
    rfWrite(result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
        }
    }
}

void ARMV5::arm_ldm(void)
{
    print_inst("ldm");
    inst_assert(B8(100), 27, 25);
    uint32_t addr;
    uint32_t data;
    uint8_t mode;
    int count = 0;
    volatile bool p = (inst >> 24) & 0x0001;
    volatile bool u = (inst >> 23) & 0x0001;
    volatile bool w = (inst >> 21) & 0x0001;
    bool error = false;

    if ((update == true) && !((inst >> 15) & 0x0001)) {
        mode = MODE_USR;
    }

    else {
        mode = CPSR_MODE(rf.cpsr);
    }

    if (u) { // increment mode
        addr = (p) ? (operand_1 + 4) : operand_1;

        for (int i = 0; i <= 15; i++) {
            if ((inst >> i) & 0x0001) {
                if (data_read(&data, addr, CPSR_MODE(rf.cpsr), type_mword) == false) {
                    arm_except_dabort();
                    return;
                    //error = true;
                    //break;
                }

                else {
                    addr += 4;
                    count++;
                    rfWrite(data, i, mode);
                }
            }
        }
    }

    else {  // decrement mode
        addr = (p) ? (operand_1 - 4) : operand_1;

        for (int i = 15; i >= 0; i--) {
            if ((inst >> i) & 0x0001) {
                if (data_read(&data, addr, CPSR_MODE(rf.cpsr), type_mword) == false) {
                    arm_except_dabort();
                    return;
                    //error = true;
                    //break;
                }

                else {
                    addr -= 4;
                    count++;
                    rfWrite(data, i, mode);
                }
            }
        }
    }

    if (w && !error) {
        operand_1 = (u) ? operand_1 + (count << 2) : operand_1 - (count << 2);
        rfWrite(operand_1, rn, CPSR_MODE(rf.cpsr));
    }

    /*
    if(update && !error)
    {
        if((inst >> 15) & 0x0001)
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        else
            rf.cpsr = psr_tmp; // switch back to original mode
    }*/

    if (update && !error && ((inst >> 15) & 0x0001)) {
        rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
    }

    for (int i = 0; i <= 15; i++) {
        uint32_t tmp;
        rfRead(&tmp, i, mode);
        printd(core_id, d_armv5_decode, "R%d = 0x%X", i, tmp);
    }
}

/*
void ARMV5::arm_ldm1(uint32_t start_addr, uint32_t end_addr)
{
    print_inst("ldm");
    inst_assert(B8(100), 27, 25);
    inst_assert_unset(22);
    inst_assert_set(20);

    uint16_t reg_list = SMM(inst, 15, 0);
    uint32_t addr = 0;
    uint32_t data = 0;

    addr = start_addr;
    if (cp15.c1_u()) {
        printb(core_id, d_armv5, "ldm1 addr error");
    }
    else {
        addr &= MM(31, 2);
    }

    for (int i = 0; i <= 14; i++) {
        if (reg_list & M(i)) {
            data = 0;
            if (data_read(&data, addr, CPSR_MODE(rf.cpsr), 4) == false) {
                arm_except_dabort();
            }
            rfWrite(data, i, CPSR_MODE(rf.cpsr));
            addr += 4;
        }
    }

    if (reg_list & M(15)) {
        data = 0;
        if (data_read(&data, addr, CPSR_MODE(rf.cpsr), 4) == false) {
            arm_except_dabort();
        }
        rfWrite(data & 0xFFFFFFFE, 15 , CPSR_MODE(rf.cpsr));

        if (data & M(0)) {
            CPSR_SET_T(rf.cpsr);
        }
        else {
            CPSR_CLEAR_T(rf.cpsr);
        }
        addr += 4;
    }

    if (end_addr != (addr - 4)) {
        printb(core_id, d_inst, "arm_ldm1 end_addr != (addr - 4)");
    }
}

void ARMV5::arm_ldm2(uint32_t start_addr, uint32_t end_addr)
{
    print_inst("ldm");
    inst_assert(B8(100), 27, 25);
    inst_assert(B8(101), 22, 20);
    inst_assert_unset(15);

    uint16_t reg_list = SMM(inst, 14, 0);
    uint32_t addr = 0;
    uint32_t data = 0;

    bool check1 = CPSR_MODE(rf.cpsr) == MODE_USR;
    bool check2 = CPSR_MODE(rf.cpsr) == MODE_SYS;

    if (check1 || check2) {
        printb(core_id, d_armv5, "arm_ldm2 mode error");
    }

    addr = start_addr;
    if (cp15.c1_u()) {
        printb(core_id, d_armv5, "ldm2 addr error");
    }
    else {
        addr &= MM(31, 2);
    }

    for (int i = 0; i <= 14; i++) {
        if (reg_list & M(i)) {
            data = 0;
            if (data_read(&data, addr, CPSR_MODE(rf.cpsr), 4) == false) {
                arm_except_dabort();
            }
            rfWrite(data, i, MODE_USR);
            addr += 4;
        }
    }

    if (end_addr != (addr - 4)) {
        printb(core_id, d_inst, "arm_ldm2 end_addr != (addr - 4)");
    }
}

void ARMV5::arm_ldm3(uint32_t start_addr, uint32_t end_addr)
{
    print_inst("ldm");
    inst_assert(B8(100), 27, 25);
    inst_assert_set(22);
    inst_assert_set(20);
    inst_assert_set(15);

    uint16_t reg_list = SMM(inst, 14, 0);
    uint32_t addr = 0;
    uint32_t data = 0;

    bool check1 = CPSR_MODE(rf.cpsr) == MODE_USR;
    bool check2 = CPSR_MODE(rf.cpsr) == MODE_SYS;

    if (check1 || check2) {
        printb(core_id, d_armv5, "arm_ldm3 mode error");
    }

    addr = start_addr;
    if (cp15.c1_u()) {
        printb(core_id, d_armv5, "ldm3 addr error");
    }
    else {
        addr &= MM(31, 2);
    }

    for (int i = 0; i <= 14; i++) {
        if (reg_list & M(i)) {
            data = 0;
            if (data_read(&data, addr, CPSR_MODE(rf.cpsr), 4) == false) {
                arm_except_dabort();
            }
            rfWrite(data, i, CPSR_MODE(rf.cpsr));
            addr += 4;
        }
    }

    uint32_t tmp = 0;
    rfRead(&tmp, SPSR, CPSR_MODE(rf.cpsr));
    rf.cpsr = tmp;

    data = 0;
    if (data_read(&data, addr, CPSR_MODE(rf.cpsr), 4) == false) {
        arm_except_dabort();
    }
    rfWrite(data, 15 , CPSR_MODE(rf.cpsr));

    addr += 4;

    if (end_addr != (addr - 4)) {
        printb(core_id, d_inst, "arm_ldm3 end_addr != (addr - 4)");
    }

}
*/

void ARMV5::arm_ldr()
{
    print_inst("ldr");
    inst_assert(B8(01), 27, 26);
    inst_assert_unset(22);
    inst_assert_set(20);

    uint32_t addr;
    uint32_t data;
    uint32_t tmp = 0;
    bool p = inst & M(24);
    bool u = inst & M(23);
    bool w = inst & M(21);
    uint8_t mode = (!p && w) ? MODE_USR : CPSR_MODE(rf.cpsr);   // LDRT or not

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t rn_val = 0;
    uint32_t rd_val = 0;

    if (p) {
        addr = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
        tmp = addr;
    }

    else {
        addr = operand_1;
        tmp = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
    }

    // LDRT
    if (!p && w) {
        if (rd == 15) {
            printb(core_id, d_armv5, "ldrt rd = 15");
        }
    }

    if (!cp15.c1_u()) {
        // aligned access
        // here we assume that memory supports aligned access only, so mask out addr[1:0]
        if (data_read(&data, addr & CMM(1, 0), mode, type_wload) == false) {
            arm_except_dabort();
            return;
        }

        switch (addr & 0x03) {
            case 0:
                break;
            case 1:
                ROTATER(data, 8);
                break;
            case 2:
                ROTATER(data, 16);
                break;
            case 3:
                ROTATER(data, 24);
                break;
        }
    }

    else {
        // unaligned access
        // our memory supports unaligned access so just do nothing and send the unaligned address
        if (data_read(&data, addr, mode, type_wload) == false) {
            arm_except_dabort();
            return;
        }
    }

    if (rd == 15) {
        rfWrite(data & CMM(1, 0), rd, CPSR_MODE(rf.cpsr));

        if (data & M(0)) {
            CPSR_SET_T(rf.cpsr);
        }

        else {
            CPSR_CLEAR_T(rf.cpsr);
        }
    }

    else {
        rfWrite(data, rd, CPSR_MODE(rf.cpsr));
    }

    if (!p || w) {
        rfWrite(tmp, rn, CPSR_MODE(rf.cpsr));
    }

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rd_val, rd, CPSR_MODE(rf.cpsr));
    printd(core_id, d_inst, "Rn = 0x%X, Rd = 0x%X, addr = 0x%X, pc = 0x%X, data = 0x%X", rn_val, rd_val, addr, rf.pc, data);
}

void ARMV5::arm_ldrb()
{
    print_inst("ldrb");
    inst_assert(B8(01), 27, 26);
    inst_assert_set(22);
    inst_assert_set(20);

    uint32_t addr;
    uint32_t data;
    uint32_t tmp = 0;
    bool p = inst & M(24);
    bool u = inst & M(23);
    bool w = inst & M(21);
    uint8_t mode = (!p && w) ? MODE_USR : CPSR_MODE(rf.cpsr);   // LDRBT or not

    if (p) {
        addr = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
        tmp = addr;
    }

    else {
        addr = operand_1;
        tmp = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
    }

    // lDRB
    // LDRBT
    if (data_read(&data, addr, mode, type_byte) == false) {
        arm_except_dabort();
        return;
    }

    rfWrite(data, rd, CPSR_MODE(rf.cpsr));

    if (!p || w) {
        rfWrite(tmp, rn, CPSR_MODE(rf.cpsr));
    }

    printd(core_id, d_inst, "addr = 0x%X, data = 0x%X", addr, data);
}

void ARMV5::arm_ldrd(uint32_t addr, uint32_t rn_val_old)
{
    print_inst("ldrd");
    inst_assert(0, 27, 25);
    inst_assert_unset(20);
    inst_assert(B8(1101), 7, 4);

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t rd_val = 0;

    bool check_1 = (rd % 2) == 0;
    bool check_2 = rd != 14;
    bool check_3 = cp15.c1_u();
    bool check_4 = (addr & M(2)) == 0;

    if (check_1 && check_2 && (check_3 || check_4)) {
        // load first word
        if (data_read(&rd_val, addr, CPSR_MODE(rf.cpsr), type_dword) == false) {
            // restore rn
            rfWrite(rn_val_old, rn, CPSR_MODE(rf.cpsr));
            arm_except_dabort();
            return;
        }

        rfWrite(rd_val, rd, CPSR_MODE(rf.cpsr));

        // load second word
        if (data_read(&rd_val, addr + 4, CPSR_MODE(rf.cpsr), type_dword) == false) {
            // restore rn
            rfWrite(rn_val_old, rn, CPSR_MODE(rf.cpsr));
            arm_except_dabort();
            return;
        }

        rfWrite(rd_val, rd + 1, CPSR_MODE(rf.cpsr));
    }

    else {
        printb(core_id, d_armv5, "arm_ldrd error, inst: 0x%X", inst);
    }
}

void ARMV5::arm_ldrex(void)
{
    print_inst("ldrex");
    inst_assert(B8(00011001), 27, 20);
    inst_assert(B8(11111001), 11, 4);

    //printb(core_id, d_armv5, "check ldrex");

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);

    uint32_t rn_val;
    uint32_t rd_val;

    if ((rn == 15) || (rn == 15)) {
        printb(core_id, d_armv5, "ldrex rd or rn = 15");
    }

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));

    if (data_read(&rd_val, rn_val, CPSR_MODE(rf.cpsr), type_wsync) == false) {
        arm_except_dabort();
        return;
    }

    /* mark_exclusive in mmu */
    rfWrite(rd_val, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_ldrexb()
{
    //printb(core_id, d_inst, "ldrexb, pc = 0x%X", rf.pc);
    print_inst("ldrexb");
    inst_assert(B8(00011101), 27, 20);
    inst_assert(B8(11111001), 11, 4);
    inst_assert(B8(1111), 3, 0);

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t rn_val, data;

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));

    if (data_read(&data, rn_val, CPSR_MODE(rf.cpsr), type_bsync) == false) {
        arm_except_dabort();
    }

    else {
        rfWrite(data, rd, CPSR_MODE(rf.cpsr));
    }
}

void ARMV5::arm_ldrh(uint32_t addr, uint32_t rn_val_old)
{
    print_inst("ldrh");
    inst_assert(0, 27, 25);
    inst_assert_set(20);
    inst_assert(B8(1011), 7, 4);

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t data = 0;

    if (cp15.c1_u() == false) {
        if ((addr & M(0)) == 0) {
            if (data_read(&data, addr, CPSR_MODE(rf.cpsr), type_hword) == false) {
                // restore rn
                rfWrite(rn_val_old, rn, CPSR_MODE(rf.cpsr));
                arm_except_dabort();
                return;
            }
        }

        else {
            printb(core_id, d_armv5, "arm_ldrh addr error");
        }
    }

    else {
        if (data_read(&data, addr, CPSR_MODE(rf.cpsr), type_hword) == false) {
            // restore rn
            rfWrite(rn_val_old, rn, CPSR_MODE(rf.cpsr));
            arm_except_dabort();
            return;
        }
    }

    data &= MM(15, 0);
    rfWrite(data, rd, CPSR_MODE(rf.cpsr));

    printd(core_id, d_inst, "data = 0x%X, addr = 0x%X", data, addr);
}

void ARMV5::arm_ldrsb(uint32_t addr, uint32_t rn_val_old)
{
    print_inst("ldrsb");
    inst_assert(0, 27, 25);
    inst_assert_set(20);
    inst_assert(B8(1101), 7, 4);

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t data = 0;

    if (data_read(&data, addr, CPSR_MODE(rf.cpsr), type_byte) == false) {
        // restore rn
        rfWrite(rn_val_old, rn, CPSR_MODE(rf.cpsr));
        arm_except_dabort();
        return;
    }

    if ((data & 0x80) == 0x80) {
        data |= 0xFFFFFF00;
    }

    else {
        data &= 0xFF;
    }

    rfWrite(data, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_ldrsh(uint32_t addr, uint32_t rn_val_old)
{
    print_inst("ldrsh");
    inst_assert(0, 27, 25);
    inst_assert_set(20);
    inst_assert(B8(1111), 7, 4);

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t data = 0;

    if (cp15.c1_u() == false) {
        if ((addr & M(0)) == 0) {
            if (data_read(&data, addr, CPSR_MODE(rf.cpsr), type_hword) == false) {
                // restore rn
                rfWrite(rn_val_old, rn, CPSR_MODE(rf.cpsr));
                arm_except_dabort();
                return;
            }
        }

        else {
            printb(core_id, d_armv5, "ldrsh error");
        }
    }

    else {
        if (data_read(&data, addr, CPSR_MODE(rf.cpsr), type_hword) == false) {
            // restore rn
            rfWrite(rn_val_old, rn, CPSR_MODE(rf.cpsr));
            arm_except_dabort();
            return;
        }
    }

    if ((data & 0x8000) == 0x8000) { //minus
        data |= 0xFFFF0000;
    }

    else {
        data &= 0xFFFF;
    }

    rfWrite(data, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_mcr(void)
{
    print_inst("mcr");
    inst_assert(B8(1110), 27, 24);
    inst_assert_unset(20);
    inst_assert_set(4);

    uint32_t data;

    if (((inst >> 8) & 0x000f) != 15) {
        arm_except_undef();
    }

    else {
        rfRead(&data, rd, CPSR_MODE(rf.cpsr));

        if (cpWrite(data, rn, rm, ((operand_1 << 3) | (operand_2 & 0x0007))) == false) {
            arm_except_undef();
        }
    }
}

void ARMV5::arm_mcrr()
{
    print_inst("mcrr");
    inst_assert(B8(11000100), 27, 20);

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint8_t cp_num = SMM(inst, 11, 8);
    uint8_t op = SMM(inst, 7, 4);
    uint8_t CRm = SMM(inst, 3, 0);

    /* ARM1136 Technical Reference Manual 3-24 */
    if (op == 0) {
        switch (CRm) {
            case 6:
                printd(core_id, d_inst, "arm_mcrr: invalidate data cache range");
                break;
            case 14:
                printd(core_id, d_inst, "arm_mcrr: clean and invalidate data cache range");
                break;
            default:
                printb(core_id, d_inst, "arm_mcrr: undefined mcrr access, op = %d, CRm = %d", op, CRm);
                break;
        }
    }

    else {
        printb(core_id, d_inst, "arm_mcrr: undefined mcrr access, op = %d, CRm = %d", op, CRm);
    }

}

void ARMV5::arm_mla(void)
{
    print_inst("mla");
    inst_assert(B8(0000001), 27, 21);
    inst_assert(B8(1001), 7, 4);
    uint32_t result;
    rd = (inst >> 16) & 0x000f;
    rn = (inst >> 12) & 0x000f;
    rs = (inst >> 8) & 0x000f;
    rm = inst & 0x000f;
    update = (inst >> 20) & 0x0001;
    rfRead(&operand_1, rm, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rs, CPSR_MODE(rf.cpsr));
    rfRead(&result, rn, CPSR_MODE(rf.cpsr));
    result += operand_1 * operand_2;
    rfWrite(result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
    }
}

void ARMV5::arm_mov(void)
{
    print_inst("mov");
    inst_assert(0, 27, 26);
    inst_assert(B8(1101), 24, 21);
    inst_assert(0, 19, 16);

    /* the rn should be 0 here */
    /* anyway, we suppose it never occur and ignore this condition */
    rfWrite(operand_2, rd, CPSR_MODE(rf.cpsr));


    if (rd == 15) {
        printd(d_inst, "arm_mov restore PC from LR:0x%X", operand_2);
    }

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)operand_2 < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            (operand_2 == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
        }
    }

    printd(core_id, d_inst, "0x%X", operand_2);
}

void ARMV5::arm_mrc(void)
{
    print_inst("mrc");
    inst_assert(B8(1110), 27, 24);
    inst_assert_set(20);
    inst_assert_set(4);

    uint32_t data = 0;

    if (((inst >> 8) & 0x000f) != 15) {
        arm_except_undef();
    }

    else {
        if (cpRead(&data, rn, rm, ((operand_1 << 3) | (operand_2 & 0x0007))) == false) {
            arm_except_undef();
        }

        else {
            if ((rd == 15) && rn != 7) {
                (data & 0x80000000) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
                (data & 0x40000000) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
                (data & 0x20000000) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
                (data & 0x10000000) ? CPSR_SET_V(rf.cpsr) : CPSR_CLEAR_V(rf.cpsr);
            }

            else if (rd != 15) {
                rfWrite(data, rd, CPSR_MODE(rf.cpsr));
            }
        }
    }
}

void ARMV5::arm_mrs(void)
{
    print_inst("mrs");
    inst_assert(B8(00010), 27, 23);
    inst_assert(0, 21, 20);
    inst_assert(B8(1111), 19, 16);
    inst_assert(0, 11, 0);

    uint32_t value;

    if ((inst >> 22) & 0x0001) { // R = 1
        rfRead(&value, SPSR, CPSR_MODE(rf.cpsr));
    }

    else {                      // R = 0
        value = rf.cpsr;
    }

    rfWrite(value, rd, CPSR_MODE(rf.cpsr));
    printd(core_id, d_inst, "%X %X", value, rd);
}

void ARMV5::arm_msr(void)
{
    print_inst("msr");
    inst_assert(0, 27, 26);
    inst_assert(B8(10), 24, 23);
    inst_assert(B8(1111), 15, 12);

    //printb(core_id, d_inst, "msr");

    /* this instruction cannont be executed in USR mode */
    if (CPSR_MODE(rf.cpsr) != MODE_USR) {
        uint32_t index;
        uint32_t value;

        /* CPSR or SPSR be accessed? */
        if ((inst >> 22) & 0x0001) { // R = 1
            index = SPSR;
            rfRead(&value, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {                      // R = 0
            index = CPSR;
            value = rf.cpsr;
        }

        switch (rn) {
            case 0x0:
                value = (value & 0xffffffff) | (operand_2 & 0x00000000);
                break;
            case 0x1:
                value = (value & 0xffffff00) | (operand_2 & 0x000000ff);
                break;
            case 0x2:
                value = (value & 0xffff00ff) | (operand_2 & 0x0000ff00);
                break;
            case 0x3:
                value = (value & 0xffff0000) | (operand_2 & 0x0000ffff);
                break;
            case 0x4:
                value = (value & 0xff00ffff) | (operand_2 & 0x00ff0000);
                break;
            case 0x5:
                value = (value & 0xff00ff00) | (operand_2 & 0x00ff00ff);
                break;
            case 0x6:
                value = (value & 0xff0000ff) | (operand_2 & 0x00ffff00);
                break;
            case 0x7:
                value = (value & 0xff000000) | (operand_2 & 0x00ffffff);
                break;
            case 0x8:
                value = (value & 0x00ffffff) | (operand_2 & 0xff000000);
                break;
            case 0x9:
                value = (value & 0x00ffff00) | (operand_2 & 0xff0000ff);
                break;
            case 0xa:
                value = (value & 0x00ff00ff) | (operand_2 & 0xff00ff00);
                break;
            case 0xb:
                value = (value & 0x00ff0000) | (operand_2 & 0xff00ffff);
                break;
            case 0xc:
                value = (value & 0x0000ffff) | (operand_2 & 0xffff0000);
                break;
            case 0xd:
                value = (value & 0x0000ff00) | (operand_2 & 0xffff00ff);
                break;
            case 0xe:
                value = (value & 0x000000ff) | (operand_2 & 0xffffff00);
                break;
            case 0xf:
                value = (value & 0x00000000) | (operand_2 & 0xffffffff);
                break;
            default:
                break;
        }

        rfWrite(value, index, CPSR_MODE(rf.cpsr));
        printd(core_id, d_inst, "CPSR: 0x%X", value);
    }
}

void ARMV5::arm_mul(void)
{
    print_inst("mul");
    inst_assert(0, 27, 21);
    inst_assert(B8(1001), 7, 4);
    inst_assert(0, 15, 12);

    uint32_t result;
    rd = (inst >> 16) & 0x000f;
    rs = (inst >> 8) & 0x000f;
    rm = inst & 0x000f;
    update = (inst >> 20) & 0x0001;
    rfRead(&operand_1, rm, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rs, CPSR_MODE(rf.cpsr));
    result = operand_1 * operand_2;
    rfWrite(result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
    }
}

void ARMV5::arm_mvn(void)
{
    print_inst("mvn");
    inst_assert(0, 27, 26);
    inst_assert(B8(1111), 24, 21);
    inst_assert(0, 19, 16);

    /* the rn should be 0 here */
    /* anyway, we suppose it never occurs and ignore this condition */
    rfWrite(~operand_2, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)(~operand_2) < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            ((~operand_2) == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
        }
    }

    printd(core_id, d_inst, "rd%d, op2 = 0x%X", rd, operand_2);
}

void ARMV5::arm_orr(void)
{
    print_inst("orr");
    inst_assert(0, 27, 26);
    inst_assert(B8(1100), 24, 21);

    uint32_t result = operand_1 | operand_2;
    rfWrite(result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
        }
    }
}

void ARMV5::arm_pkhbt(void)
{
    printb(core_id, d_inst, "pkhbt, pc = 0x%X", rf.pc);
    print_inst("pkhbt");
    inst_assert(B8(01101000), 27, 20);
    inst_assert(B8(001), 6, 4);

    rn = (inst & MM(19, 16)) >> 16;
    rd = (inst & MM(15, 12)) >> 12;
    rm = (inst & MM(3, 0));
    imm = (inst & MM(11, 7)) >> 7;

    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

    operand_1 &= MM(15, 0);
    operand_2 >>= imm;
    operand_2 &= MM(31, 16);

    rfWrite(operand_2 | operand_1, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_pkhtb(void)
{
    printb(core_id, d_inst, "pkhtb, pc = 0x%X", rf.pc);
    print_inst("pkhtb");
    inst_assert(B8(01101000), 27, 20);
    inst_assert(B8(101), 6, 4);

    rn = (inst & MM(19, 16)) >> 16;
    rd = (inst & MM(15, 12)) >> 12;
    rm = (inst & MM(3, 0));
    imm = (inst & MM(11, 7)) >> 7;

    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

    uint32_t tmp_1, tmp_2;

    if (imm == 0) {
        if (operand_2 == 0) {
            tmp_1 = 0;
        }

        else {
            tmp_1 = 0xFFFF;
        }
    }

    else {
        tmp_1 = (operand_2 >> imm) & MM(15, 0);
    }

    tmp_2 = operand_1 & MM(31, 16);

    rfWrite(tmp_2 | tmp_1, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_qadd(void)
{
    print_inst("qadd");
    inst_assert(B8(00010000), 27, 20);
    inst_assert(0, 11, 8);
    inst_assert(B8(0101), 7, 4);

    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

    if ((operand_1 > 0) && (operand_2 > 0)) {
        uint32_t result = operand_1 + operand_2;

        if (result < 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x7fffffff, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else if ((operand_1 < 0) && (operand_2 < 0)) {
        uint32_t result = operand_1 + operand_2;

        if (result > 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x80000000, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else {
        CPSR_CLEAR_Q(rf.cpsr);
        rfWrite((operand_1 + operand_2), rd, CPSR_MODE(rf.cpsr));
    }
}

void ARMV5::arm_qsub(void)
{
    print_inst("qsub");
    inst_assert(B8(00010010), 27, 20);
    inst_assert(0, 11, 8);
    inst_assert(B8(0101), 7, 4);

    /* NOTE: Rm is the first operand rather than Rn */
    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rm, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rn, CPSR_MODE(rf.cpsr));

    if ((operand_1 > 0) && (operand_2 < 0)) {
        uint32_t result = operand_1 - operand_2;

        if (result < 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x7fffffff, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else if ((operand_1 < 0) && (operand_2 > 0)) {
        uint32_t result = operand_1 - operand_2;

        if (result > 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x80000000, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else {
        CPSR_CLEAR_Q(rf.cpsr);
        rfWrite((operand_1 - operand_2), rd, CPSR_MODE(rf.cpsr));
    }
}

void ARMV5::arm_qdadd(void)
{
    print_inst("qdadd");
    inst_assert(B8(00010100), 27, 20);
    inst_assert(0, 11, 8);
    inst_assert(B8(0101), 7, 4);

    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

    if ((operand_1 > 0) && ((operand_1 << 1) < 0)) {
        operand_1 = 0x7fffffff;
    }

    else if ((operand_1 < 0) && ((operand_1 << 1) > 0)) {
        operand_1 = 0x80000000;
    }

    else {
        operand_1 <<= 1;
    }

    if ((operand_1 > 0) && (operand_2 > 0)) {
        uint32_t result = operand_1 + operand_2;

        if (result < 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x7fffffff, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else if ((operand_1 < 0) && (operand_2 < 0)) {
        uint32_t result = operand_1 + operand_2;

        if (result > 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x80000000, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else {
        CPSR_CLEAR_Q(rf.cpsr);
        rfWrite((operand_1 + operand_2), rd, CPSR_MODE(rf.cpsr));
    }
}

void ARMV5::arm_qdsub(void)
{
    print_inst("qdsub");
    inst_assert(B8(00010110), 27, 20);
    inst_assert(0, 11, 8);
    inst_assert(B8(0101), 7, 4);

    /* NOTE: Rm is the first operand rather than Rn */
    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rm, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rn, CPSR_MODE(rf.cpsr));

    if ((operand_2 > 0) && ((operand_2 << 1) < 0)) {
        operand_2 = 0x7fffffff;
    }

    else if ((operand_1 < 0) && ((operand_2 << 1) > 0)) {
        operand_2 = 0x80000000;
    }

    else {
        operand_2 <<= 1;
    }

    if ((operand_1 > 0) && (operand_2 < 0)) {
        uint32_t result = operand_1 - operand_2;

        if (result < 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x7fffffff, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else if ((operand_1 < 0) && (operand_2 > 0)) {
        uint32_t result = operand_1 - operand_2;

        if (result > 0) {
            CPSR_SET_Q(rf.cpsr);
            rfWrite(0x80000000, rd, CPSR_MODE(rf.cpsr));
        }

        else {
            CPSR_CLEAR_Q(rf.cpsr);
            rfWrite(result, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else {
        CPSR_CLEAR_Q(rf.cpsr);
        rfWrite((operand_1 - operand_2), rd, CPSR_MODE(rf.cpsr));
    }
}

void ARMV5::arm_rev(void)
{
    print_inst("rev");
    inst_assert(B8(01101011), 27, 20);
    inst_assert(B8(1111), 19, 16);
    inst_assert(B8(1111), 11, 8);
    inst_assert(B8(0011), 7, 4);

    uint32_t result = 0;

    if (inst & 0x0080) { // REV
        result =
            ((operand_2 & 0x000000ff) << 24) |
            ((operand_2 & 0x0000ff00) << 8) |
            ((operand_2 & 0x00ff0000) >> 8) |
            ((operand_2 & 0xff000000) >> 24);
    }

    else {              // REV16
        result =
            ((operand_2 & 0x000000ff) << 8) |
            ((operand_2 & 0x0000ff00) >> 8) |
            ((operand_2 & 0x00ff0000) << 8) |
            ((operand_2 & 0xff000000) >> 8);
    }

    rfWrite(result, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_revsh(void)
{
    print_inst("revsh");
    inst_assert(B8(01101111), 27, 20);
    inst_assert(B8(1111), 15, 12);
    inst_assert(B8(1111), 11, 8);
    inst_assert(B8(1011), 7, 4);

    uint32_t result = 0;

    result =
        ((operand_2 & 0x000000ff) << 8) |
        ((operand_2 & 0x0000ff00) >> 8);
    result |= ((result & 0x00008000) ? 0xffff0000 : 0x00000000);

    rfWrite(result, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_rfe(void)
{
    printb(core_id, d_inst, "rfe");
    print_inst("rfe");
    inst_assert(B8(1111100), 31, 25);
    inst_assert_unset(22);
    inst_assert_set(20);
    inst_assert(0, 15, 12);
    inst_assert(B8(1010), 11, 8);
    inst_assert(0, 7, 0);
}

void ARMV5::arm_rsb(void)
{
    print_inst("rsb");
    inst_assert(0, 27, 26);
    inst_assert(B8(0011), 24, 21);

    uint32_t operand_tmp = -operand_1;
    uint64_t result = (uint64_t)operand_2 + (uint64_t)operand_tmp;
    rfWrite((uint32_t)result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            ((int32_t)result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            ((operand_2 >= operand_1)) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

            /* overflow detection */
            if (((operand_tmp >> 31) ^ (operand_2 >> 31)) != 0) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else {
                CPSR_SET_V(rf.cpsr);
            }
        }
    }
}

void ARMV5::arm_rsc(void)
{
    print_inst("rsc");
    inst_assert(0, 27, 26);
    inst_assert(B8(0111), 24, 21);

    uint32_t operand_tmp = -operand_1;
    uint64_t result = (uint64_t)operand_2 + (uint64_t)operand_tmp - (uint64_t)(!CPSR_C(rf.cpsr));
    rfWrite((uint32_t)result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            ((int32_t)result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (operand_2 >= operand_1) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

            /* overflow detection */
            if (((operand_tmp >> 31) ^ (operand_2 >> 31)) != 0) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else {
                CPSR_SET_V(rf.cpsr);
            }
        }
    }
}

void ARMV5::arm_sbc(void)
{
    print_inst("sbc");
    inst_assert(0, 27, 26);
    inst_assert(B8(0110), 24, 21);

    uint32_t operand_tmp = -operand_2;
    uint64_t result = (uint64_t)operand_1 + (uint64_t)operand_tmp - (uint64_t)(!CPSR_C(rf.cpsr));
    rfWrite((uint32_t)result, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            ((int32_t)result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (operand_1 >= operand_2) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

            /* overflow detection */
            if (((operand_1 >> 31) ^ (operand_tmp >> 31)) != 0) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else {
                CPSR_SET_V(rf.cpsr);
            }
        }
    }
}

void ARMV5::arm_sel(void)
{
    print_inst("sel");
    inst_assert(B8(01101000), 27, 20);
    inst_assert(B8(11111011), 11, 4);

    uint32_t result = 0;
    uint8_t ge = CPSR_GE(rf.cpsr);

    result |= ((ge & 0x0001) ? (operand_1 & 0x000000ff) : (operand_2 & 0x000000ff));
    result |= ((ge & 0x0002) ? (operand_1 & 0x0000ff00) : (operand_2 & 0x0000ff00));
    result |= ((ge & 0x0004) ? (operand_1 & 0x00ff0000) : (operand_2 & 0x00ff0000));
    result |= ((ge & 0x0008) ? (operand_1 & 0xff000000) : (operand_2 & 0xff000000));
    rfWrite(result, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_setend()
{
    print_inst("setend");
    inst_assert(B16(11110001, 00000001), 31, 16);
    inst_assert(0, 15, 10);
    inst_assert(0, 8, 0);

    printb(core_id, d_armv6, "arm_setend");

    if ((inst >> 9) & 0x0001) { // set to big-endian
        CPSR_SET_E(rf.cpsr);
        cout << "set to big-endian" << endl;
    }

    else {                      // set to little-endian
        CPSR_CLEAR_E(rf.cpsr);
        cout << "set to little-endian" << endl;
    }
}

void ARMV5::arm_sev()
{
    print_inst("sev");
    inst_assert(B8(00110010), 27, 20);
    inst_assert(B8(00001111), 19, 12);
    inst_assert(B8(0000), 11, 8);
    inst_assert(B8(00000100), 7, 0);

    //printm(core_id, d_inst,"sev");
    send_event();
}

void ARMV5::arm_smla(void)
{
    printb(core_id, d_inst, "smla");
    print_inst("smla");
    inst_assert(B8(00010000), 27, 20);
    inst_assert_set(7);
    inst_assert_unset(4);

    //printb(core_id, d_inst, "smla not implemented yet");

    bool x = inst & M(5);
    bool y = inst & M(6);
    uint32_t rn_value;

    rfRead(&operand_1, rm, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rs, CPSR_MODE(rf.cpsr));
    rfRead(&rn_value, rn, CPSR_MODE(rf.cpsr));

    printd(core_id, d_inst, "rm: 0x%X, rs: 0x%X, rn: 0x%X, x: %d, y: %d", operand_1, operand_2, rn_value, x, y);

    if (x) { // sign extend rm[31:16]
        operand_1 >>= 16;
    }

    else {   // sign extend rm[15:0]
        operand_1 &= MM(15, 0);
    }

    // do sign extend
    if (operand_1 & M(15)) {
        operand_1 |= MM(31, 16);
    }

    if (y) { // sign extend rs[31:16]
        operand_2 >>= 16;
    }

    else {   // sign extend rs[15:0]
        operand_2 &= MM(15, 0);
    }

    // do sign extend
    if (operand_2 & M(15)) {
        operand_2 |= MM(31, 16);
    }

    uint64_t i64;
    uint32_t i32;

    i64 = (operand_1 * operand_2) + rn_value;
    i32 = i64;
    rfWrite(i32, rd, CPSR_MODE(rf.cpsr));

    if ((i64 - i32) > 0) {
        printd(core_id, d_inst, "overflow");
        CPSR_SET_Q(rf.cpsr);
    }

    printd(core_id, d_inst, "big_result: 0x%X, small_result: 0x%X", i64, i32);
}

void ARMV5::arm_smlal(void)
{
    print_inst("smlal");
    inst_assert(B8(0000111), 27, 21);
    inst_assert(B8(1001), 7, 4);

    int64_t result;
    uint64_t resultHi = 0, resultLo = 0;
    uint8_t rdHi = (inst >> 16) & 0x000f;
    uint8_t rdLo = (inst >> 12) & 0x000f;
    rs = (inst >> 8) & 0x000f;
    rm = inst & 0x000f;
    update = (inst >> 20) & 0x0001;
    rfRead((uint32_t*)(&resultLo), rdLo, CPSR_MODE(rf.cpsr));
    rfRead((uint32_t*)(&resultHi), rdHi, CPSR_MODE(rf.cpsr));
    rfRead(&operand_1, rs, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));
    result = (int64_t)operand_1 * (int64_t)operand_2;
    resultLo += (result & (uint64_t)0xffffffff);
    resultHi += ((result >> 32) + ((resultLo >> 32) & 0x0001));
    rfWrite((uint32_t)resultLo, rdLo, CPSR_MODE(rf.cpsr));
    rfWrite((uint32_t)resultHi, rdHi, CPSR_MODE(rf.cpsr));

    if (update == true) {
        (result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
    }
}

void ARMV5::arm_smlalxy(void)
{
    print_inst("smlalxy");
    inst_assert(B8(00010100), 27, 20);
    inst_assert_set(7);
    inst_assert_unset(4);

    printb(core_id, d_inst, "smlalxy not implemented yet");
}

void ARMV5::arm_smlaw(void)
{
    print_inst("smlaw");
    inst_assert(B8(00010010), 27, 20);
    inst_assert_set(7);
    inst_assert(0, 5, 4);
    printb(core_id, d_inst, "smlaw not implemented yet");
}

void ARMV5::arm_smul(void)
{
    print_inst("smul");
    inst_assert(B8(00010110), 27, 20);
    inst_assert(0, 15, 12);
    inst_assert_set(7);
    inst_assert_unset(4);

    printb(core_id, d_inst, "smul not implemented yet");
}

void ARMV5::arm_smull(void)
{
    print_inst("smull");
    inst_assert(B8(0000110), 27, 21);
    inst_assert(B8(1001), 7, 4);

    int64_t result;
    uint32_t resultHi, resultLo;
    uint8_t rdHi = (inst >> 16) & 0x000f;
    uint8_t rdLo = (inst >> 12) & 0x000f;
    rs = (inst >> 8) & 0x000f;
    rm = inst & 0x000f;
    update = (inst >> 20) & 0x0001;
    rfRead(&operand_1, rs, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));
    result = (int64_t)operand_1 * (int64_t)operand_2;
    resultLo = (uint32_t)result;
    resultHi = (uint32_t)(result >> 32);
    rfWrite(resultLo, rdLo, CPSR_MODE(rf.cpsr));
    rfWrite(resultHi, rdHi, CPSR_MODE(rf.cpsr));

    if (update == true) {
        (result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
    }
}

void ARMV5::arm_smulw(void)
{
    print_inst("smulw");
    inst_assert(B8(00010010), 27, 20);
    inst_assert_set(7);
    inst_assert(B8(10), 5, 4);

    printb(core_id, d_inst, "smulw not implemented yet");
}

void ARMV5::arm_smulx(void)
{
    print_inst("smulx");
    printb(core_id, d_inst, "smulx not implemented yet");
}

void ARMV5::arm_srs(void)
{
    print_inst("srs");
    inst_assert(B8(1111100), 31, 25);
    inst_assert_set(22);
    inst_assert(B8(01101), 20, 16);
    inst_assert(B8(00000101000), 11, 5);
    printb(core_id, d_armv6, "check arm_srs");
}

void ARMV5::arm_stm(void)
{
    print_inst("stm");
    inst_assert(B8(100), 27, 25);
    uint32_t addr;
    uint32_t data;
    //uint32_t psr_tmp = rf.cpsr;
    uint8_t mode;
    int count = 0;
    bool p = (inst >> 24) & 0x0001;
    bool u = (inst >> 23) & 0x0001;
    bool w = (inst >> 21) & 0x0001;
    bool error = false;

    if (update == true) {
        mode = MODE_USR;
    }

    else {
        mode = CPSR_MODE(rf.cpsr);
    }

    for (int i = 0; i <= 15; i++) {
        uint32_t tmp;
        rfRead(&tmp, i, mode);
        printd(core_id, d_armv5_decode, "R%d = 0x%X", i, tmp);
    }

    if (u) { // increment mode
        addr = (p) ? (operand_1 + 4) : operand_1;

        for (int i = 0; i <= 15; i++) {
            if ((inst >> i) & 0x0001) {
                rfRead(&data, i, mode);

                if (data_write(data, addr, CPSR_MODE(rf.cpsr), type_mword) == false) {
                    arm_except_dabort();
                    return;
                    //error = true;
                    //break;
                }

                else {
                    addr += 4;
                    count++;
                }
            }
        }
    }

    else {  // decrement mode
        addr = (p) ? (operand_1 - 4) : operand_1;

        for (int i = 15; i >= 0; i--) {
            if ((inst >> i) & 0x0001) {
                rfRead(&data, i, mode);

                if (data_write(data, addr, CPSR_MODE(rf.cpsr), type_mword) == false) {
                    //rf.cpsr = psr_tmp;  // switch back
                    arm_except_dabort();
                    return;
                    //error = true;
                    //break;
                }

                else {
                    addr -= 4;
                    count++;
                }
            }
        }
    }

    if (w && !error) {
        operand_1 = (u) ? operand_1 + (count << 2) : operand_1 - (count << 2);
        rfWrite(operand_1, rn, CPSR_MODE(rf.cpsr));
    }

    /*
    if(update && !error) {
        rf.cpsr = psr_tmp;
    }
    */

    for (int i = 0; i <= 15; i++) {
        uint32_t tmp;
        rfRead(&tmp, i, mode);
        printd(core_id, d_armv5_decode, "R%d = 0x%X", i, tmp);
    }
}

/*
void ARMV5::arm_stm1(uint32_t start_addr, uint32_t end_addr)
{
    print_inst("stm");
    inst_assert(B8(100), 27, 25);
    inst_assert_unset(22);
    inst_assert_unset(20);

    uint16_t reg_list = SMM(inst, 15, 0);

    uint32_t addr = 0;
    uint32_t ri = 0;


    addr = start_addr;
    if (cp15.c1_u()) {
        printb(core_id, d_armv5, "stm1 addr error");
    }
    else {
        addr &= MM(31, 2);
    }

    for (int i = 0; i <= 15; i++) {
        if (reg_list & M(i)) {
            ri = 0;
            rfRead(&ri, i, CPSR_MODE(rf.cpsr));
            if (data_write(ri, addr, CPSR_MODE(rf.cpsr), 4) == false) {
                arm_except_dabort();
                return ;
            }
            addr += 4;
        }
    }

    if (end_addr != (addr - 4)) {
        printb(core_id, d_armv5, "stm1 end_addr != (addr - 4)");
    }

    for (int i = 0; i <= 15; i++) {
        uint32_t tmp;
        rfRead(&tmp, i, CPSR_MODE(rf.cpsr));
        printd(core_id, d_inst, "R%d = 0x%X", i, tmp);
    }
}

void ARMV5::arm_stm2(uint32_t start_addr, uint32_t end_addr)
{
    print_inst("stm");
    inst_assert(B8(100), 27, 25);
    inst_assert_set(22);
    inst_assert_unset(20);

    bool check1 = CPSR_MODE(rf.cpsr) == MODE_USR;
    bool check2 = CPSR_MODE(rf.cpsr) == MODE_SYS;

    if (check1 || check2) {
        printb(core_id, d_armv5, "stm2 mode error");
    }

    uint16_t reg_list = SMM(inst, 15, 0);

    uint32_t addr = 0;
    uint32_t ri_usr = 0;


    addr = start_addr;
    if (cp15.c1_u()) {
        printb(core_id, d_armv5, "stm2 addr error");
    }
    else {
        addr &= MM(31, 2);
    }

    for (int i = 0; i <= 15; i++) {
        if (reg_list & M(i)) {
            ri_usr = 0;
            rfRead(&ri_usr, i, MODE_USR);
            if (data_write(ri_usr, addr, CPSR_MODE(rf.cpsr), 4) == false) {
                arm_except_dabort();
                return ;
            }
            addr += 4;
        }
    }

    if (end_addr != (addr - 4)) {
        printb(core_id, d_armv5, "stm1 end_addr != (addr - 4)");
    }

    for (int i = 0; i <= 15; i++) {
        uint32_t tmp;
        rfRead(&tmp, i, MODE_USR);
        printd(core_id, d_inst, "R%d = 0x%X", i, tmp);
    }
}
*/

void ARMV5::arm_str()
{
    print_inst("str");
    inst_assert(B8(01), 27, 26);
    inst_assert_unset(22);
    inst_assert_unset(20);

    uint32_t addr;
    uint32_t data;
    uint32_t  tmp = 0;
    bool p = inst & M(24);
    bool u = inst & M(23);
    bool w = inst & M(21);

    uint8_t mode = (!p && w) ? MODE_USR : CPSR_MODE(rf.cpsr);   // STRT or not

    if (p) {
        addr = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
        tmp = addr;
    }

    else {
        addr = operand_1;
        tmp = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
    }

    rfRead(&data, rd, CPSR_MODE(rf.cpsr));

    /*
    switch (addr & 0x0003) {
        case 0:
            break;
        case 1:
            ROTATER(data, 8);
            break;
        case 2:
            ROTATER(data, 16);
            break;
        case 3:
            ROTATER(data, 24);
            break;
    }
    */

    /*
    if(p == false && w == true)
    {
        // STRT
        // ignore
        printb(core_id, d_inst, "strt");
    }
    else
    */
    {
        if (cp15.c1_u() == false) {
            if (data_write(data, addr & CMM(1, 0), mode, type_wstore) == false) {
                arm_except_dabort();
                return;
            }
        }

        else {
            if (data_write(data, addr, mode, type_wstore) == false) {
                arm_except_dabort();
                return;
            }
        }

        //if(!p || w)
        if (p == false) {
            rfWrite(tmp, rn, CPSR_MODE(rf.cpsr));
        }

        else if ((p == true) && (w == true)) {
            rfWrite(tmp, rn, CPSR_MODE(rf.cpsr));
        }
    }

    rn = SMM(inst, 19, 16);
    rd = SMM(inst, 15, 12);
    uint32_t rn_val, rd_val;
    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rd_val, rd, CPSR_MODE(rf.cpsr));
    printd(core_id, d_inst, "addr = 0x%X, rn = 0x%X, rd = 0x%X, data = 0x%X", addr, rn_val, rd_val, data);
}


void ARMV5::arm_strb()
{
    print_inst("strb");
    inst_assert(B8(01), 27, 26);
    inst_assert_set(22);
    inst_assert_unset(20);

    uint32_t addr;
    uint32_t data;
    uint32_t tmp = 0;
    bool p = inst & M(24);
    bool u = inst & M(23);
    bool w = inst & M(21);
    uint8_t mode = (!p && w) ? MODE_USR : CPSR_MODE(rf.cpsr);   // STRBT or not

    if (p) {
        addr = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
        tmp = addr;
    }

    else {
        addr = operand_1;
        tmp = (u == true) ? (operand_1 + operand_2) : (operand_1 - operand_2);
    }

    rfRead(&data, rd, CPSR_MODE(rf.cpsr));
    data &= 0x00ff;

    /*
    if(p == false && w == true)
    {
        printb(core_id, d_inst, "strbt");
        // STRBT
        // ignore
    }
    else
    */
    {
        if (data_write(data, addr, mode, type_byte) == false) {
            arm_except_dabort();
            return;
        }

        if (!p || w) {
            rfWrite(tmp, rn, CPSR_MODE(rf.cpsr));
        }
    }

    uint32_t rd_val;
    rfRead(&rd_val, rd, CPSR_MODE(rf.cpsr));
    uint8_t rn = (inst >> 12) & 0xF;
    uint32_t rn_val = 0;
    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    printd(core_id, d_inst, "addr = 0x%X, rd_val = 0x%X, rn_val = 0x%X", addr, rd_val, rn_val);
}

void ARMV5::arm_strd(uint32_t addr, uint32_t rn_val_old)
{
    print_inst("strd");
    inst_assert(B8(000), 27, 25);
    inst_assert_unset(20);
    inst_assert(B8(1111), 7, 4);

    uint8_t rd = SMM(inst, 15, 12);
    uint32_t rd_val = 0;

    bool check_1 = (rd % 2) == 0;
    bool check_2 = rd != 14;
    bool check_3 = (addr & MM(2, 0)) == 0;

    bool check_4 = cp15.c1_u();
    bool check_5 = (addr & M(2)) == 0;

    if (check_1 && check_2 && check_3 && (check_4 || check_5)) {
        rfRead(&rd_val, rd, CPSR_MODE(rf.cpsr));

        if (data_write(rd_val, addr, CPSR_MODE(rf.cpsr), type_dword) == false) {
            arm_except_dabort();
            return;
        }

        rfRead(&rd_val, rd + 1, CPSR_MODE(rf.cpsr));

        if (data_write(rd_val, addr + 4, CPSR_MODE(rf.cpsr), type_dword) == false) {
            arm_except_dabort();
            return;
        }
    }

    else {
        printb(core_id, d_armv5, "strd error");
    }
}

void ARMV5::arm_strex(void)
{
    print_inst("strex");
    inst_assert(B8(00011000), 27, 20);
    inst_assert(B8(11111001), 11, 4);

    //printb(core_id, d_armv5, "check strex");

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint8_t rm = SMM(inst, 3, 0);

    uint32_t rn_val = 0;
    uint32_t rd_val = 0;
    uint32_t rm_val = 0;

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rm_val, rm, CPSR_MODE(rf.cpsr));

    is_exclusive_local = true;
    is_exclusive_global = true;
    non_cachable = true;

    if (data_write(rm_val, rn_val, CPSR_MODE(rf.cpsr), type_wsync) == false) {
        arm_except_dabort();
        return;
    }

    if (is_exclusive_local) {
        if (non_cachable) {
            if (is_exclusive_global) {
                /* already written in mmu */
                rfWrite(0, rd, CPSR_MODE(rf.cpsr));
                /* already cleared in mmu */
            }

            else {
                rfWrite(1, rd, CPSR_MODE(rf.cpsr));
            }
        }

        else {
            /* already written in mmu */
            rfWrite(0, rd, CPSR_MODE(rf.cpsr));
        }
    }

    else {
        rfWrite(1, rd, CPSR_MODE(rf.cpsr));
    }

    local_monitor.clear_exclusive(core_id);
}

void ARMV5::arm_strexb()
{
    print_inst("strexb");
    inst_assert(B8(00011100), 27, 20);
    inst_assert(B8(11111001), 11, 4);

    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint8_t rm = SMM(inst, 3, 0);
    uint32_t rn_val, rm_val;

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rm_val, rm, CPSR_MODE(rf.cpsr));

    rm_val &= MM(7, 0);

    if (data_write(rm_val, rn_val, CPSR_MODE(rf.cpsr), type_bsync) == false) {
        rfWrite(1, rd, CPSR_MODE(rf.cpsr));
        arm_except_dabort();
        return;
    }

    rfWrite(0, rd, CPSR_MODE(rf.cpsr));
}

void ARMV5::arm_strh(uint32_t addr, uint32_t rn_val_old)
{
    print_inst("strh");
    inst_assert(0, 27, 25);
    inst_assert_unset(20);
    inst_assert(B8(1011), 7, 4);

    uint8_t rd = SMM(inst, 15, 12);
    uint32_t rd_val = 0;

    rfRead(&rd_val, rd, CPSR_MODE(rf.cpsr));
    rd_val &= MM(15, 0);

    if (cp15.c1_u() == false) {
        if ((addr & M(0)) == 0) {
            if (data_write(rd_val, addr, CPSR_MODE(rf.cpsr), type_hword) == false) {
                arm_except_dabort();
                return;
            }
        }

        else {
            printb(core_id, d_armv5, "strh addr error");
        }
    }

    else {
        if (data_write(rd_val, addr, CPSR_MODE(rf.cpsr), type_hword) == false) {
            arm_except_dabort();
            return;
        }
    }
}

void ARMV5::arm_sub(void)
{
    print_inst("sub");
    inst_assert(0, 27, 26);
    inst_assert(B8(0010), 24, 21);

    uint32_t operand_tmp = -operand_2;
    uint64_t result = (uint64_t)operand_1 + (uint64_t)operand_tmp;
    rfWrite((uint32_t)result, rd, CPSR_MODE(rf.cpsr));

    uint32_t rn_val;
    uint32_t rd_val;
    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rd_val, rd, CPSR_MODE(rf.cpsr));

    if (update == true) {
        if (rd == PC) {
            rfRead(&rf.cpsr, SPSR, CPSR_MODE(rf.cpsr));
        }

        else {
            ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
            ((int32_t)result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
            (operand_1 >= operand_2) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);

            /* overflow detection */
            if (((operand_1 >> 31) ^ (operand_tmp >> 31)) != 0) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else if (((result >> 31) == 0x0003) || ((result >> 31) == 0x0000)) {
                CPSR_CLEAR_V(rf.cpsr);
            }

            else {
                CPSR_SET_V(rf.cpsr);
            }
        }
    }

    printd(core_id, d_inst, "op1 = 0x%X, op2 = 0x%X, rn = %X, rd = %X, result = 0x%X", operand_1, operand_2, rn_val, rd_val, result);
}

void ARMV5::arm_swp(void)
{
    print_inst("swp");
    inst_assert(B8(00010000), 27, 20);
    inst_assert(B8(00001001), 11, 4);

    uint32_t data;
    bool result;
    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

    if (cp15.c1_u() == false) {
        if (data_read(&data, operand_1 & CMM(1, 0), CPSR_MODE(rf.cpsr), type_wload) == false) {
            arm_except_dabort();
            return;
        }

        if (data_write(operand_2, operand_1 & CMM(1, 0), CPSR_MODE(rf.cpsr), type_wstore) == false) {
            arm_except_dabort();
            return;
        }

        switch (operand_1 & 0x0003) {
            case 1:
                ROTATER(data, 8);
                break;
            case 2:
                ROTATER(data, 16);
                break;
            case 3:
                ROTATER(data, 24);
                break;
            default:
                break;
        }

        rfWrite(data, rd, CPSR_MODE(rf.cpsr));
    }

    else {
        if (data_read(&data, operand_1, CPSR_MODE(rf.cpsr), type_wsync) == false) {
            arm_except_dabort();
            return;
        }

        if (data_write(operand_2, operand_1, CPSR_MODE(rf.cpsr), type_wsync) == false) {
            arm_except_dabort();
            return;
        }

        rfWrite(data, rd, CPSR_MODE(rf.cpsr));
    }
}

void ARMV5::arm_swpb(void)
{
    print_inst("swapb");
    inst_assert(B8(00010100), 27, 20);
    inst_assert(B8(00001001), 11, 4);

    uint32_t data;
    bool result = true;
    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

    if ((result = data_read(&data, operand_1, CPSR_MODE(rf.cpsr), type_byte)) == true) {
        if ((result = data_write(operand_2, operand_1, CPSR_MODE(rf.cpsr), type_byte)) == true) {
            rfWrite(data, rd, CPSR_MODE(rf.cpsr));
        }
    }

    if (result == false) {
        arm_except_dabort();
    }
}

void ARMV5::arm_sxtab(void)
{
    print_inst("sxtab");
    inst_assert(B8(01101010), 27, 20);
    inst_assert(B8(000111), 9, 4);

    printd(core_id, d_sxtab, "sxtab, pc = 0x%X", rf.pc);
    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint8_t rotate = SMM(inst, 11, 10);
    uint8_t rm = SMM(inst, 3, 0);

    uint32_t rn_val = 0;
    uint32_t rm_val = 0;
    uint32_t rd_val = 0;

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rm_val, rm, CPSR_MODE(rf.cpsr));

    rotate *= 8;
    ROTATER(rm_val, rotate);
    rm_val &= 0xFF;

    if ((rm_val & 0x80) == 0x80) { // minus
        rm_val |= 0xFFFFFF00;
    }

    else {
        //rm_val &= 0xFF;
    }

    if (rn != 15) {
        rd_val = rn_val + rm_val;
    }

    else {
        // arm_sxtb
        inst_assert(B8(1111), 19, 16);
        rd_val = rm_val;
    }

    rfWrite(rd_val, rd, CPSR_MODE(rf.cpsr));
    printd(core_id, d_inst, "rd = 0x%X", rd_val);
}

void ARMV5::arm_sxtab16(void)
{
    printb(core_id, d_inst, "sxtab16");
    print_inst("sxtab16");
    inst_assert(B8(01101000), 27, 20);
    inst_assert(B8(000111), 9, 4);
}

void ARMV5::arm_sxtah(void)
{
    printb(core_id, d_inst, "sxtah");
    print_inst("sxtah");
    inst_assert(B8(01101011), 27, 20);
    inst_assert(B8(000111), 9, 4);
}

void ARMV5::arm_teq(void)
{
    print_inst("teq");
    inst_assert(0, 27, 26);
    inst_assert(B8(10011), 24, 20);
    inst_assert(0, 15, 12);

    /* the rd should be 0 here */
    /* anyway, we suppose it never occur and ignore this condition */
    if (update == true) {
        uint32_t result = operand_1 ^ operand_2;
        ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
        (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
    }
}

void ARMV5::arm_tst(void)
{
    print_inst("tst");
    inst_assert(0, 27, 26);
    inst_assert(B8(10001), 24, 20);
    inst_assert(0, 15, 12);

    /* the rd should be 0 here */
    /* anyway, we suppose it never occur and ignore this condition */
    if (update == true) {
        uint32_t result = operand_1 & operand_2;
        ((int32_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
        (shift_carry_out) ? CPSR_SET_C(rf.cpsr) : CPSR_CLEAR_C(rf.cpsr);
    }

    printd(core_id, d_inst, "%X %X %X", operand_1, operand_2, rf.cpsr);
}

void ARMV5::arm_umaal(void)
{
    print_inst("umaal");
    inst_assert(B8(00000100), 27, 20);
    inst_assert(B8(1001), 7, 4);

    printb(core_id, d_inst, "umaal not implemented yet");
}

void ARMV5::arm_umlal(void)
{
    print_inst("umlal");
    inst_assert(B8(0000101), 27, 21);
    inst_assert(B8(1001), 7, 4);

    uint64_t result;
    uint64_t resultHi = 0, resultLo = 0;
    uint8_t rdHi = (inst >> 16) & 0x000f;
    uint8_t rdLo = (inst >> 12) & 0x000f;
    rs = (inst >> 8) & 0x000f;
    rm = inst & 0x000f;
    update = (inst >> 20) & 0x0001;
    rfRead((uint32_t*)(&resultLo), rdLo, CPSR_MODE(rf.cpsr));
    rfRead((uint32_t*)(&resultHi), rdHi, CPSR_MODE(rf.cpsr));
    rfRead(&operand_1, rs, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));
    result = (uint64_t)operand_1 * (uint64_t)operand_2;
    resultLo += (result & (uint64_t)0xffffffff);
    resultHi += ((result >> 32) + ((resultLo >> 32) & 0x0001));
    rfWrite((uint32_t)resultLo, rdLo, CPSR_MODE(rf.cpsr));
    rfWrite((uint32_t)resultHi, rdHi, CPSR_MODE(rf.cpsr));

    if (update == true) {
        ((int64_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
    }
}

void ARMV5::arm_umull(void)
{
    print_inst("umull");
    inst_assert(B8(0000100), 27, 21);
    inst_assert(B8(1001), 7, 4);

    uint64_t result;
    uint32_t resultHi, resultLo;
    uint8_t rdHi = (inst >> 16) & 0x000f;
    uint8_t rdLo = (inst >> 12) & 0x000f;
    rs = (inst >> 8) & 0x000f;
    rm = inst & 0x000f;
    update = (inst >> 20) & 0x0001;
    rfRead(&operand_1, rs, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));
    result = (uint64_t)operand_1 * (uint64_t)operand_2;
    resultLo = (uint32_t)result;
    resultHi = (uint32_t)(result >> 32);
    rfWrite(resultLo, rdLo, CPSR_MODE(rf.cpsr));
    rfWrite(resultHi, rdHi, CPSR_MODE(rf.cpsr));

    if (update == true) {
        ((int64_t)result < 0) ? CPSR_SET_N(rf.cpsr) : CPSR_CLEAR_N(rf.cpsr);
        (result == 0) ? CPSR_SET_Z(rf.cpsr) : CPSR_CLEAR_Z(rf.cpsr);
    }
}

void ARMV5::arm_uxtab(void)
{
    print_inst("uxtab/uxtb");
    inst_assert(B8(01101110), 27, 20);
    inst_assert(B8(000111), 9, 4);

    printd(core_id, d_uxtab, "uxtab, pc = 0x%X", rf.pc);
    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t rotate = SMM(inst, 11, 10);
    uint8_t rm = SMM(inst, 3, 0);
    uint32_t rn_val, rd_val, rm_val;
    uint32_t result = 0;

    rotate *= 8;
    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rm_val, rm, CPSR_MODE(rf.cpsr));

    printd(core_id, d_inst, "rn = 0x%X, rm = 0x%X, rotate = 0x%X", operand_1, operand_2, rotate);

    if (rn != 15) {
        printd(core_id, d_inst, "type: uxtab");
        ROTATER(rm_val, rotate);
        rm_val &= 0x000000FF;
        rd_val = rn_val + rm_val;
    }

    else {
        printd(core_id, d_inst, "type: uxtb");
        ROTATER(rm_val, rotate);
        rm_val &= 0x000000FF;
        rd_val = rm_val;
    }

    rfWrite(rd_val, rd, CPSR_MODE(rf.cpsr));

    printd(core_id, d_inst, "rd = 0x%X", result);
}

void ARMV5::arm_uxtab16(void)
{
    printb(core_id, d_inst, "uxtab16");
    print_inst("uxtab16");
    inst_assert(B8(01101100), 27, 20);
    inst_assert(B8(000111), 9, 4);

    printb(core_id, d_armv5_decode, "uxtab16");
}

void ARMV5::arm_uxtah(void)
{
    print_inst("uxtah/uxth");
    inst_assert(B8(01101111), 27, 20);
    inst_assert(B8(000111), 9, 4);

    printd(core_id, d_uxtah, "uxtah, pc = 0x%X", rf.pc);
    uint32_t rn_val, rd_val, rm_val;
    uint8_t rn = SMM(inst, 19, 16);
    uint8_t rd = SMM(inst, 15, 12);
    uint32_t rotate = SMM(inst, 11, 10);
    uint8_t rm = SMM(inst, 3, 0);

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rm_val, rm, CPSR_MODE(rf.cpsr));

    rotate *= 8;
    ROTATER(rm_val, rotate);
    rm_val &= 0x0000FFFF;

    if (rn != 15) {
        printd(core_id, d_inst, "mode: uxtah");
        rd_val = rn_val + rm_val;
    }

    else {
        printd(core_id, d_inst, "mode: uxth");
        rd_val = rm_val;
    }

    rfWrite(rd_val, rd, CPSR_MODE(rf.cpsr));

    printd(core_id, d_inst, "rd = 0x%X", rd_val);

}

void ARMV5::arm_wfe()
{
    print_inst("wfe");
    inst_assert(B8(00110010), 27, 20);
    inst_assert(B8(00001111), 19, 12);
    inst_assert(B8(0000), 11, 8);
    inst_assert(B8(00000010), 7, 0);

    //printm(core_id, d_inst, "wfe");
    if (event_registered()) {
        clear_event_reg();
    }

    else {
        wait_for_event();
    }
}

void ARMV5::arm_wfi()
{
    print_inst("wfi");
    inst_assert(B8(00110010), 27, 20);
    inst_assert(B8(00001111), 19, 12);
    inst_assert(B8(0000), 11, 8);
    inst_assert(B8(00000011), 7, 0);

    //printm(core_id, d_inst, "wfi");
    wait(irq_n.negedge_event());
    //printm(core_id, d_inst, "wfi end");
}

void ARMV5::dsp_pld()
{
    print_inst("pld");
    inst_assert(B8(111101), 31, 26);
    inst_assert(B8(101), 22, 20);
    inst_assert(B8(1111), 15, 12);
    // prepare to access the memory
    // do nothing here
}

