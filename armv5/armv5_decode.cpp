#include <armv5.h>
#include <bit_opt.h>

/* fetch 32-bits ARM instruction */
bool ARMV5::fetchARM(void)
{
    bool result = false;
    result = inst_arm_read();
    rf.pc += 4;

    if (result == false) { // instruction prefetch abort exception
        arm_except_fabort();
    }

    else {
        /* judge the instruction condition */
        switch ((inst & MM(31, 28)) >> 28) {
                /* the condition of NV is instruction dependent */
                /* however, we pass it as condition AL here */
            case INST_COND_EQ:
                result = CPSR_Z(rf.cpsr);
                break;
            case INST_COND_NE:
                result = !CPSR_Z(rf.cpsr);
                break;
            case INST_COND_CS:
                result = CPSR_C(rf.cpsr);
                break;
            case INST_COND_CC:
                result = !CPSR_C(rf.cpsr);
                break;
            case INST_COND_MI:
                result = CPSR_N(rf.cpsr);
                break;
            case INST_COND_PL:
                result = !CPSR_N(rf.cpsr);
                break;
            case INST_COND_VS:
                result = CPSR_V(rf.cpsr);
                break;
            case INST_COND_VC:
                result = !CPSR_V(rf.cpsr);
                break;
            case INST_COND_HI:
                result = (CPSR_C(rf.cpsr) && !CPSR_Z(rf.cpsr));
                break;
            case INST_COND_LS:
                result = (!CPSR_C(rf.cpsr) || CPSR_Z(rf.cpsr));
                break;
            case INST_COND_GE:
                result = (CPSR_N(rf.cpsr) == CPSR_V(rf.cpsr));
                break;
            case INST_COND_LT:
                result = (CPSR_N(rf.cpsr) != CPSR_V(rf.cpsr));
                break;
            case INST_COND_GT:
                result = (!CPSR_Z(rf.cpsr) && (CPSR_N(rf.cpsr) == CPSR_V(rf.cpsr)));
                break;
            case INST_COND_LE:
                result = (CPSR_Z(rf.cpsr) || (CPSR_N(rf.cpsr) != CPSR_V(rf.cpsr)));
                break;
            case INST_COND_AL:
                result = true;
                break;
            case B8(1111):
                result = true;
                break;
            default:
                printb(core_id, d_armv5_decode, "condition code error");
                return false;
        }
    }

    return result;
}

/* execute 32-bits ARM instruction */
bool ARMV5::exeARM()
{
    bool result;

    /* to decode the instruction */
    switch ((inst & MM(27, 25)) >> 25) {
        case 0:
            result = arm_inst_mode0();
            break;
        case 1:
            result = arm_inst_mode1();
            break;
        case 2:
            result = arm_inst_mode2();
            break;
        case 3:
            result = arm_inst_mode3();
            break;
        case 4:
            result = arm_inst_mode4();
            break;
        case 5:
            result = arm_inst_mode5();
            break;
        case 6:
            result = arm_inst_mode6();
            break;
        case 7:
            result = arm_inst_mode7();
            break;
        default:
            result = false;
            break;
    }

    /* undefined instruction exception */
    if (result == false) {
        arm_except_undef();
        printb(core_id, d_armv5_decode, "undefined instruction: 0x%X", inst);
    }
}

/* Following functions are based on ARM Reference Manual Figure A3-1 "ARM instruction set summary"
 *      row 1 of the table denoted as (1),
 *      row 2 of the table denoted as (2),
 *      ...
 *
 */

/* decode the 32-bits ARM instructions with mode 0 */
bool ARMV5::arm_inst_mode0(void)
{
    if ((inst & MM(31, 28)) == MM(31, 28)) {
        if ((inst & 0x0fff0000) == 0x01010000) {
            arm_setend();
        }

        else if (((inst & 0x0ff00000) == 0x01000000) && (((inst >> 16) & 0x0001) == 0)) {
            arm_cps();
        }

        else {
            printb(core_id, d_armv5_decode, "mode0 unconditional error");
        }
    }

    else {
        // (1) ~ (5)
        switch (inst & (M(7) | M(4))) {
                // (5)
            case M(7)|M(4):

                switch (inst & MM(7, 4)) {
                        // (5) multiplies
                    case B8(1001)<<4:

                        switch (inst & MM(24, 21)) {
                            case B8(0000)<<21:
                                arm_mul();
                                break;
                            case B8(0001)<<21:
                                arm_mla();
                                break;
                            case B8(0010)<<21:
                                arm_umaal();
                                break;
                            case B8(0100)<<21:
                                arm_umull();
                                break;
                            case B8(0101)<<21:
                                arm_umlal();
                                break;
                            case B8(0110)<<21:
                                arm_smull();
                                break;
                            case B8(0111)<<21:
                                arm_smlal();
                                break;
                            case B8(1000)<<21:
                                arm_swp();
                                break;
                            case B8(1010)<<21:
                                arm_swpb();
                                break;
                            case B8(1100)<<21:

                                if (inst & M(20)) {
                                    arm_ldrex();
                                }

                                else {
                                    arm_strex();
                                }

                                break;
                            case B8(1110)<<21:

                                if (inst & M(20)) {
                                    arm_ldrexb();
                                }

                                else {
                                    arm_strexb();
                                }

                                break;
                            default:
                                printb(core_id, d_armv5_decode, "mode 0 (5) error, inst: %X", inst);
                                break;
                        }

                        break;
                        // (5) extra load/stores
                    case B8(1011)<<4:
                    case B8(1101)<<4:
                    case B8(1111)<<4:

                        if (inst & M(22)) {
                            addr_mode3_imm();
                        }

                        else {
                            addr_mode3_reg();
                        }

                        break;
                    default:
                        printb(core_id, d_armv5_decode, "mode 0 (5): undefined instruction: 0x%X", inst);
                        break;
                }

                break;
                // (3) (4)
            case M(4):

                // (4)
                if ((inst & (M(24) | M(23) | M(20))) == M(24)) {
                    switch (inst & (B8(11111) << 20 | B8(1111) << 4)) {
                        case B8(10000)<<20 | B8(0101)<<4:
                            arm_qadd();
                            break;
                        case B8(10010)<<20 | B8(0001)<<4:
                            arm_bx();
                            break;
                        case B8(10010)<<20 | B8(0011)<<4:
                            arm_blx2();
                            break;
                        case B8(10010)<<20 | B8(0101)<<4:
                            arm_qsub();
                            break;
                        case B8(10010)<<20 | B8(0111)<<4:
                            arm_bkpt();
                            break;
                        case B8(10100)<<20 | B8(0101)<<4:
                            arm_qdadd();
                            break;
                        case B8(10110)<<20 | B8(0001)<<4:
                            arm_clz();
                            break;
                        case B8(10110)<<20 | B8(0101)<<4:
                            arm_qdsub();
                            break;
                        default:
                            printb(core_id, d_armv5_decode, "mode0 undefined instruction: 0x%X", inst);
                            break;
                    }
                }

                // (3)
                else {
                    // shift by register
                    update = (inst >> 20) & 0x0001;
                    rn = (inst >> 16) & 0x000f;
                    rd = (inst >> 12) & 0x000f;
                    rs = (inst >> 8) & 0x000f;
                    rm = inst & 0x000f;
                    shift = (inst >> 5) & 0x0003;
                    imm = (inst >> 7) & 0x001f;
                    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
                    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

                    uint32_t operand_3;
                    rfRead(&operand_3, rs, CPSR_MODE(rf.cpsr));
                    operand_3 &= 0x00ff;

                    switch (shift) {
                        case 0:

                            if (operand_3 == 0) {
                                shift_carry_out = CPSR_C(rf.cpsr);
                            }

                            else if (operand_3 < 32) {
                                shift_carry_out = (operand_2 >> (32 - operand_3)) & 0x0001;
                                operand_2 = operand_2 << operand_3;
                            }

                            else if (operand_3 == 32) {
                                shift_carry_out = operand_2 & 0x0001;
                                operand_2 = 0;
                            }

                            else {
                                shift_carry_out = 0;
                                operand_2 = 0;
                            }

                            break;
                        case 1:

                            if (operand_3 == 0) {
                                shift_carry_out = CPSR_C(rf.cpsr);
                            }

                            else if (operand_3 < 32) {
                                shift_carry_out = (operand_2 >> (operand_3 - 1)) & 0x0001;
                                operand_2 = operand_2 >> operand_3;
                            }

                            else if (operand_3 == 32) {
                                shift_carry_out = ((int32_t)operand_2 < 0);
                                operand_2 = 0;
                            }

                            else {
                                shift_carry_out = 0;
                                operand_2 = 0;
                            }

                            break;
                        case 2:

                            if (operand_3 == 0) {
                                shift_carry_out = CPSR_C(rf.cpsr);
                            }

                            else if (operand_3 < 32) {
                                shift_carry_out = (operand_2 >> (operand_3 - 1)) & 0x0001;
                                operand_2 = (int32_t)operand_2 >> operand_3;
                            }

                            else {
                                shift_carry_out = ((int32_t)operand_2 < 0);
                                operand_2 = ((int32_t)operand_2 >= 0) ? 0 : 0xffffffff;
                            }

                            break;
                        case 3:

                            if (operand_3 == 0) {
                                shift_carry_out = CPSR_C(rf.cpsr);
                            }

                            else if ((operand_3 & 0x000f) == 0) {
                                shift_carry_out = ((int32_t)operand_2 < 0);
                            }

                            else {
                                operand_3 &= 0x000f;
                                shift_carry_out = (operand_2 >> (operand_3 - 1)) & 0x0001;
                                ROTATER(operand_2, operand_3);
                            }

                            break;
                    }

                    switch (inst & MM(24, 21)) {
                        case  0<<21:
                            arm_and();
                            break;
                        case  1<<21:
                            arm_eor();
                            break;
                        case  2<<21:
                            arm_sub();
                            break;
                        case  3<<21:
                            arm_rsb();
                            break;
                        case  4<<21:
                            arm_add();
                            break;
                        case  5<<21:
                            arm_adc();
                            break;
                        case  6<<21:
                            arm_sbc();
                            break;
                        case  7<<21:
                            arm_rsc();
                            break;
                        case  8<<21:
                            arm_tst();
                            break;
                        case  9<<21:
                            arm_teq();
                            break;
                        case 10<<21:
                            arm_cmp();
                            break;
                        case 11<<21:
                            arm_cmn();
                            break;
                        case 12<<21:
                            arm_orr();
                            break;
                        case 13<<21:

                            if (((inst & M(20)) == 0) && ((inst & MM(11, 4)) == 0)) {
                                arm_cpy();
                            }

                            else {
                                arm_mov();
                            }

                            break;
                        case 14<<21:
                            arm_bic();
                            break;
                        case 15<<21:
                            arm_mvn();
                            break;
                    }
                }

                break;
                // (1) (2)
            default:
                // (2)
                // shift by immediate
                update = (inst >> 20) & 0x0001;
                rn = (inst >> 16) & 0x000f;
                rd = (inst >> 12) & 0x000f;
                rs = (inst >> 8) & 0x000f;
                rm = inst & 0x000f;
                shift = (inst >> 5) & 0x0003;
                imm = (inst >> 7) & 0x001f;
                rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
                rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

                switch (inst & (0x1F << 20 | 0xF << 4)) {
                    case B8(10010)<<20 | B8(0010)<<4:
                        arm_bxj();
                        break;
                    case B8(10000)<<20 | B8(0000)<<4:
                    case B8(10100)<<20 | B8(0000)<<4:
                        arm_mrs();
                        break;
                    case B8(10010)<<20 | B8(0000)<<4:
                    case B8(10110)<<20 | B8(0000)<<4:
                        arm_msr();
                        break;
                    case B8(10000)<<20 | B8(1000)<<4:
                    case B8(10000)<<20 | B8(1010)<<4:
                    case B8(10000)<<20 | B8(1100)<<4:
                    case B8(10000)<<20 | B8(1110)<<4:
                        arm_smla();
                        break;
                    case B8(10010)<<20 | B8(1000)<<4:
                    case B8(10010)<<20 | B8(1100)<<4:
                        arm_smlaw();
                        break;
                    case B8(10010)<<20 | B8(1010)<<4:
                    case B8(10010)<<20 | B8(1110)<<4:
                        arm_smulw();
                        break;
                    case B8(10100)<<20 | B8(1000)<<4:
                    case B8(10100)<<20 | B8(1010)<<4:
                    case B8(10100)<<20 | B8(1100)<<4:
                    case B8(10100)<<20 | B8(1110)<<4:
                        arm_smlalxy();
                        break;
                    case B8(10110)<<20 | B8(1000)<<4:
                    case B8(10110)<<20 | B8(1010)<<4:
                    case B8(10110)<<20 | B8(1100)<<4:
                    case B8(10110)<<20 | B8(1110)<<4:
                        arm_smul();
                        break;
                    default:

                        // (1)
                        switch (shift) {
                            case 0:

                                if (imm) {
                                    shift_carry_out = (operand_2 >> (32 - imm)) & 0x0001;
                                    operand_2 <<= imm;
                                }

                                else {
                                    shift_carry_out = CPSR_C(rf.cpsr);
                                }

                                break;
                            case 1:

                                if (imm) {
                                    shift_carry_out = (operand_2 >> (imm - 1)) & 0x0001;
                                    operand_2 >>= imm;
                                }

                                else {
                                    shift_carry_out = ((int32_t)operand_2 < 0);
                                    operand_2 = 0;
                                }

                                break;
                            case 2:

                                if (imm) {
                                    shift_carry_out = (operand_2 >> (imm - 1)) & 0x0001;
                                    operand_2 = (int32_t)operand_2 >> imm;
                                }

                                else {
                                    shift_carry_out = ((int32_t)operand_2 < 0);
                                    operand_2 = ((int32_t)operand_2 >= 0) ? 0 : 0xffffffff;
                                }

                                break;
                            case 3:

                                if (imm) {
                                    shift_carry_out = (operand_2 >> (imm - 1)) & 0x0001;
                                    ROTATER(operand_2, imm);
                                }

                                else {
                                    shift_carry_out = operand_2 & 0x0001;
                                    operand_2 = (CPSR_C(rf.cpsr) << 31) | (operand_2 >> 1);
                                }

                                break;
                        }

                        switch (inst & MM(24, 21)) {
                            case  0<<21:
                                arm_and();
                                break;
                            case  1<<21:
                                arm_eor();
                                break;
                            case  2<<21:
                                arm_sub();
                                break;
                            case  3<<21:
                                arm_rsb();
                                break;
                            case  4<<21:
                                arm_add();
                                break;
                            case  5<<21:
                                arm_adc();
                                break;
                            case  6<<21:
                                arm_sbc();
                                break;
                            case  7<<21:
                                arm_rsc();
                                break;
                            case  8<<21:
                                arm_tst();
                                break;
                            case  9<<21:
                                arm_teq();
                                break;
                            case 10<<21:
                                arm_cmp();
                                break;
                            case 11<<21:
                                arm_cmn();
                                break;
                            case 12<<21:
                                arm_orr();
                                break;
                            case 13<<21:

                                if (((inst & M(20)) == 0) && ((inst & MM(11, 4)) == 0)) {
                                    arm_cpy();
                                }

                                else {
                                    arm_mov();
                                }

                                break;
                            case 14<<21:
                                arm_bic();
                                break;
                            case 15<<21:
                                arm_mvn();
                                break;
                            default:
                                printb(core_id, d_armv5_decode, "undefined instruction (1): 0x%X", inst);
                        }

                        break;
                }
        }
    }

    return true;
}

/* decode the 32-bits ARM instructions with mode 1 */
bool ARMV5::arm_inst_mode1(void)
{
    update = (inst >> 20) & 0x0001;
    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    shift = (inst >> 7) & 0x001e;
    imm = inst & 0x00ff;
    ROTATER(imm, shift);
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    operand_2 = imm;
    shift_carry_out = (shift) ? ((int32_t)operand_2 < 0) : CPSR_C(rf.cpsr);

    switch (inst & MM(24, 20)) {
            // (7)
        case B8(10000)<<20:
        case B8(10100)<<20:
            printb(core_id, d_armv5_decode, "mode 1 (7) undefine instruction");
            break;
            // (8)
        case B8(10010)<<20:

            switch (inst & 0xFFFFF) {
                    /* wfe, wfi and sev are actually msr instruction */
                case 0x0F002:
                    arm_wfe();
                    break;
                case 0x0F003:
                    arm_wfi();
                    break;
                case 0x0F004:
                    arm_sev();
                    break;
                default:
                    arm_msr();
                    break;
            }

            break;
        case B8(10110)<<20:
            arm_msr();
            break;
        default:

            // (6)
            switch (inst & MM(24, 21)) {
                case B8(0000)<<21:
                    arm_and();
                    break;
                case B8(0001)<<21:
                    arm_eor();
                    break;
                case B8(0010)<<21:
                    arm_sub();
                    break;
                case B8(0011)<<21:
                    arm_rsb();
                    break;
                case B8(0100)<<21:
                    arm_add();
                    break;
                case B8(0101)<<21:
                    arm_adc();
                    break;
                case B8(0110)<<21:
                    arm_sbc();
                    break;
                case B8(0111)<<21:
                    arm_rsc();
                    break;
                case B8(1000)<<21:
                    arm_tst();
                    break;
                case B8(1001)<<21:
                    arm_teq();
                    break;
                case B8(1010)<<21:
                    arm_cmp();
                    break;
                case B8(1011)<<21:
                    arm_cmn();
                    break;
                case B8(1100)<<21:
                    arm_orr();
                    break;
                case B8(1101)<<21:
                    arm_mov();
                    break;
                case B8(1110)<<21:
                    arm_bic();
                    break;
                case B8(1111)<<21:
                    arm_mvn();
                    break;
                default:
                    printb(core_id, d_armv5_decode, "mode 1 (6) error");
            }

            break;
    }

    return true;
}

bool ARMV5::arm_inst_mode2(void)
{
    /* Addressing Mode 2: Immediate offset/index */

    if ((inst & MM(31, 28)) == MM(31, 28)) {
        // the PLD DSP instruction for ARMv5TE
        switch (inst & MM(22, 20)) {
            case B8(111)<<20:
                arm_clrex();
                break;
            case B8(101)<<20:
                dsp_pld();
                break;
            default:
                printb(core_id, d_armv5_decode, "inst_mode2 unconditional error");
        }
    }

    else {
        bool b = inst & M(22);
        bool l = inst & M(20);
        rn = (inst & MM(19, 16)) >> 16;
        rd = (inst & MM(15, 12)) >> 12;
        operand_2 = inst & MM(11, 0);
        rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));

        // L bit: load or store
        // B bit: unsigned byte or word
        if (l == true) {
            (b == true) ? arm_ldrb() : arm_ldr();
        }

        else {
            (b == true) ? arm_strb() : arm_str();
        }
    }

    return true;
}

/* decode the 32-bits ARM instructions with mode 3 */
bool ARMV5::arm_inst_mode3(void)
{
    rn = (inst >> 16) & 0x000f;
    rd = (inst >> 12) & 0x000f;
    rm = inst & 0x000f;
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    rfRead(&operand_2, rm, CPSR_MODE(rf.cpsr));

    if ((inst & MM(31, 28)) == MM(31, 28)) {
        printb(core_id, d_armv5_decode, "pld unconditional");
    }

    // (11) (12)
    else if (inst & M(4)) {
        if (((inst & MM(24, 20)) == MM(24, 20)) && ((inst & MM(7, 4)) == MM(7, 4))) {
            // (12)
            printb(core_id, d_armv5_decode, "mode3 (12) undefined instruction: 0x%X", inst);
        }

        else {
            // (11)
            // A3-24
            switch (inst & (B8(11) << 23 | B8(1111) << 4)) {
                    // sign/zero extend (add)
                case B8(01)<<23 | B8(0111)<<4:

                    switch (inst & (B8(111) << 20)) {
                        case B8(010)<<20:
                            arm_sxtab();
                            break;
                            /*
                            case B8(000)<<20:
                                arm_sxtab16();
                                break;
                            case B8(011)<<20:
                                arm_sxtah();
                                break;
                                */
                        case B8(110)<<20:
                            arm_uxtab();
                            //arm_uxtb();
                            break;
                            /*
                                                    case B8(100)<<20:
                            arm_uxtab16();
                            //arm_uxtb16();
                            break;
                            */
                        case B8(111)<<20:
                            arm_uxtah();
                            //arm_uxth();
                            break;
                        default:
                            printb(core_id, d_armv5_decode, "mode3 (11) undefined instruction: 0x%X, pc = 0x%X", inst, rf.pc);
                            break;
                    }

                    break;
                default:
                    printb(core_id, d_armv5_decode, "media instruction (11) not implemented yet: 0x%X", inst);
                    break;
            }
        }
    }

    // (10)
    else {
        // A5-19
        // Scaled register offset/index
        if ((inst & MM(11, 4)) != 0) {
            imm = (inst >> 7) & B8(11111);
            shift = (inst >> 5) & B8(11);

            switch (shift) {
                case 0: // LSL
                    operand_2 <<= imm;
                    break;
                case 1: // LSR

                    if (imm == 0) {
                        operand_2 = 0;
                    }

                    else {
                        operand_2 >>= imm;
                    }

                    break;
                case 2: // ASR

                    if (imm == 0) {
                        if (operand_2 & M(31)) {
                            operand_2 = 0xFFFFFFFF;
                        }

                        else {
                            operand_2 = 0;
                        }
                    }

                    else {
                        operand_2 = (int32_t)operand_2 >> imm;
                    }

                    break;
                case 3: // ROR & RRX

                    if (imm == 0) {
                        operand_2 = (CPSR_C(rf.cpsr) << 31) | (operand_2 >> 1);
                    }

                    else {
                        ROTATER(operand_2, imm);
                    }

                    break;
            }
        }

        // register offset/index
        else {
            // do nothing
        }

        bool b = (inst >> 22) & B8(1);
        bool l = (inst >> 20) & B8(1);

        if (l) { // load
            (b) ? arm_ldrb() : arm_ldr();
        }

        else {  // store
            (b == true) ? arm_strb() : arm_str();
        }
    }

    return true;
}

/* decode the 32-bits ARM instructions with mode 4 */
bool ARMV5::arm_inst_mode4(void)
{
    /*
    bool p = inst & M(24);
    bool u = inst & M(23);
    bool s = inst & M(22);
    bool w = inst & M(21);
    bool l = inst & M(20);
    uint8_t rn = SMM(inst, 19, 16);
    update = (inst >> 22) & 0x0001;
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));
    */

    /*
    update = (inst >> 22) & 0x0001;
    if ((inst & MM(31, 28)) == MM(31, 28)) {
        printb(core_id, d_armv5_decode, "inst mode4 error");
        //(l == true) ? arm_rfe() : arm_srs();
    }
    else {
        addr_mode4();
    }
    */

    if ((inst & MM(31, 28)) == MM(31, 28)) {
        printb(core_id, d_armv5_decode, "inst mode4 error");
        //(l == true) ? arm_rfe() : arm_srs();
    }

    bool l = (inst >> 20) & 0x0001;
    update = (inst >> 22) & 0x0001;
    rn = (inst >> 16) & 0x000f;
    rfRead(&operand_1, rn, CPSR_MODE(rf.cpsr));

    if ((inst & MM(31, 28)) == MM(31, 28)) {
        printb(core_id, d_armv5_decode, "inst mode4 error");
        //(l == true)? arm_rfe() : arm_srs();
    }

    else {
        (l == true) ? arm_ldm() : arm_stm();
    }

    return true;
}

/* decode the 32-bits ARM instructions with mode 5 */
bool ARMV5::arm_inst_mode5(void)
{
    if ((inst & MM(31, 28)) == MM(31, 28)) {
        /* we haven't implemented BLX instruction yet */
        //arm_blx();
        printb(core_id, d_armv5_decode, "blx1 not implemented yet: 0x%X", inst);
    }

    else {   // implement the 32-bits ARM B & BL instruction
        uint32_t tmp = rf.pc + 4;
        uint32_t offset;

        if ((inst >> 24) & 0x0001) {
            rfWrite(rf.pc, LR, CPSR_MODE(rf.cpsr));
        }

        offset = (inst & 0x00ffffff) << 2;
        offset = (offset & 0x02000000) ? (offset | 0xfc000000) : offset; // signed extension
        rf.pc = tmp + (int32_t)offset;
    }

    return true;
}

/* decode the 32-bits ARM instructions with mode 6 */
bool ARMV5::arm_inst_mode6(void)
{
    // (15) , coprocessor load/store
    if ((inst & MM(31, 28)) == MM(31, 28)) {
        printb(core_id, d_armv5_decode, "inst_mode6 unconditional");
    }

    else {
        switch (inst & MM(27, 20)) {
            case B8(11000100)<<20:
                arm_mcrr();
                break;
            case B8(11000101)<<20:
                printb(core_id, d_armv5_decode, "arm_mrrc()");
                //arm_mrrc();
                break;
            default:
                printb(core_id, d_armv5_decode, "arm_ldc()");
                //arm_ldc();
                break;
        }
    }

    return true;
}

/* decode the 32-bits ARM instructions with mode 7 */
bool ARMV5::arm_inst_mode7(void)
{
    if ((inst & MM(31, 28)) == MM(31, 28)) {
        printb(core_id, d_armv5_decode, "mode 7 unconditional not implemented yet.");

        if ((inst & MM(27, 24)) == MM(27, 24)) {
            printb(core_id, d_armv5_decode, "mode7 undefined instruction space, A3-42");
        }
    }

    else if ((inst & MM(27, 24)) == MM(27, 24)) {
        arm_except_swi();
    }

    else {
        rn = (inst >> 16) & 0x000f;
        rd = (inst >> 12) & 0x000f;
        rs = (inst >> 8) & 0x000f;
        rm = inst & 0x000f;
        operand_1 = (inst >> 21) & 0x0007;
        operand_2 = (inst >> 5) & 0x0007;

        if (inst & 0x0010) {
            (inst & 0x00100000) ? arm_mrc() : arm_mcr();
        }

        else {
            arm_cdp();
        }
    }

    return true;
}

