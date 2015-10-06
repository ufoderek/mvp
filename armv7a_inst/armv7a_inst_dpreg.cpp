#include <armv7a.h>

void armv7a::decode_dpreg(armv7a_ir& inst)
{
    uint32_t op1 = inst(24, 20);
    uint32_t op2 = inst(11, 7);
    uint32_t op3 = inst(6, 5);

    switch(op1)
    {
        case B(0000 0):
        case B(0000 1):
            arm_and_reg(inst);
            return;
        case B(0001 0):
        case B(0001 1):
            arm_eor_reg(inst);
            return;
        case B(0010 0):
        case B(0010 1):
            arm_sub_reg(inst);
            return;
        case B(0011 0):
        case B(0011 1):
            arm_rsb_reg(inst);
            return;
        case B(0100 0):
        case B(0100 1):
            arm_add_reg(inst);
            return;
        case B(0101 0):
        case B(0101 1):
            arm_adc_reg(inst);
            return;
        case B(0110 0):
        case B(0110 1):
            arm_sbc_reg(inst);
            return;
        case B(0111 0):
        case B(0111 1):
            arm_rsc_reg(inst);
            return;
        case B(10001):
            arm_tst_reg(inst);
            return;
        case B(10011):
            arm_teq_reg(inst);
            return;
        case B(10101):
            arm_cmp_reg(inst);
            return;
        case B(10111):
            arm_cmn_reg(inst);
            return;
        case B(1100 0):
        case B(1100 1):
            arm_orr_reg(inst);
            return;
        case B(1101 0):
        case B(1101 1):

            if((op2 == 0) && (op3 == 0))
            {
                arm_mov_reg(inst);
            }

            else if((op2 != 0) && (op3 == 0))
            {
                arm_lsl_imm(inst);
            }

            else if(op3 == 1)
            {
                arm_lsr_imm(inst);
            }

            else if(op3 == B(10))
            {
                arm_asr_imm(inst);
            }

            else if((op2 == 0) && (op3 == B(11)))
            {
                arm_rrx(inst);
            }

            else if((op2 != 0) && (op3 == B(11)))
            {
                arm_ror_imm(inst);
            }

            else
            {
                printb(core_id, d_armv7a_decode_dpreg, "data processing decode error 1101");
            }

            return;
        case B(1110 0):
        case B(1110 1):
            arm_bic_reg(inst);
            return;
        case B(1111 0):
        case B(1111 1):
            arm_mvn_reg(inst);
            return;
        default:
            printb(core_id, d_armv7a_decode_dpreg, "data processing decode error 1111");
    }
}

void armv7a::arm_and_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_and_reg");
    inst.check(27, 21, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        //Encoding A1
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_and_reg SUBS PC, LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n)&shifted.val, 32);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_eor_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_eor_reg");
    inst.check(27, 21, B(01));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_eor_reg SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n) ^ shifted.val, 32);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_sub_reg(armv7a_ir& inst)
{
    uint32_t rn = inst(19, 16);

    if(rn == B(1101))
    {
        arm_sub_sp_minus_reg(inst);
        return;
    }

    inst.print_inst("arm_sub_reg");
    inst.check(27, 21, B(10));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_sub_reg SUBS PC,LR ...");
        }

        /*
           if (rn == B(1101)) {
            printb(d_inst, "arm_sub_reg SUB SP");
           }
         */
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_shifted(~shifted.val, shifted.n);
        add_with_carry(&result, &carry, &overflow, _rn, not_shifted, 1);
        printd(d_inst, "shifted:%X-%d", shifted.val, shifted.n);
        printd(d_inst, "not_shifted:%X-%d", not_shifted.val, not_shifted.n);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
                rf.cpsr_V(overflow);
            }
        }
    }
}

void armv7a::arm_rsb_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_rsb_reg");
    inst.check(27, 21, B(000 0011));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_rsb_reg SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits not_rn(~rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, not_rn, shifted, 1);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
                rf.cpsr_V(overflow);
            }
        }

        if((d == 8) && (result.val == 0x9dac))
        {
            printd(d_inst, "rsb rm=%X shifted=%X ~rn=%X result=%X", _rm.val, shifted.val, not_rn.val, result.val);
        }
    }
}

void armv7a::arm_add_reg(armv7a_ir& inst)
{
    uint32_t rn = inst(19, 16);

    if(rn == B(1101))
    {
        arm_add_sp_plus_reg(inst);
        return;
    }

    inst.print_inst("arm_add_reg");
    inst.check(27, 21, B(100));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "add_reg sub pc, lr");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, shifted, 0);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
                rf.cpsr_V(overflow);
            }
        }
    }
}

void armv7a::arm_adc_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_adc_reg");
    inst.check(27, 21, B(101));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "adc_reg sub pc, lr");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, shifted, rf.cpsr_C());

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
                rf.cpsr_V(overflow);
            }
        }
    }
}

void armv7a::arm_sbc_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_sbc_reg");
    inst.check(27, 21, B(000 0110));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_sbc_reg SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_shifted(~shifted.val, shifted.n);
        add_with_carry(&result, &carry, &overflow, _rn, not_shifted, rf.cpsr_C());

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
                rf.cpsr_V(overflow);
            }
        }
    }
}

void armv7a::arm_rsc_reg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_rsc_reg not implemented yet.");
}

void armv7a::arm_tst_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_tst_reg");
    inst.check(27, 20, B(1 0001));
    inst.check(15, 12, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t n = rn;
        uint32_t m = rm;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n)&shifted.val, 32);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
    }
}

void armv7a::arm_teq_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_teq_reg");
    inst.check(27, 20, B(0001 0011));
    inst.check(15, 12, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t n = rn;
        uint32_t m = rm;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n) ^ shifted.val, 32);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
    }
}

void armv7a::arm_cmp_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_cmp_reg");
    inst.check(27, 20, B(10101));
    inst.check(15, 12, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t n = rn;
        uint32_t m = rm;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_shifted(~shifted.val, shifted.n);
        add_with_carry(&result, &carry, &overflow, _rn, not_shifted, 1);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
        rf.cpsr_V(overflow);
        printd(d_inst, "cpsr %X", rf.cpsr());
    }
}

void armv7a::arm_cmn_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_cmn_reg");
    inst.check(27, 20, B(0001 0111));
    inst.check(15, 12, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t n = rn;
        uint32_t m = rm;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, shifted, 0);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
        rf.cpsr_V(overflow);
        printd(d_inst, "cpsr %X", rf.cpsr());
    }
}

void armv7a::arm_orr_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_orr_reg");
    inst.check(27, 21, B(000 1100));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_orr_reg SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n) | shifted.val, 32);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_mov_reg(armv7a_ir& inst)
{
    uint32_t s = inst(20);
    uint32_t rd = inst(15, 12);

    if((rd == B(1111)) && (s == 1))
    {
        arm_subs_pc_lr(inst);
        return;
    }

    inst.print_inst("arm_mov_reg");
    inst.check(27, 21, B(1101));
    inst.check(11, 4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t rm = inst(3, 0);
        /*
           if ((rd == B(1111)) && (s == 1)) {
            printb(d_inst, "arm_mov_reg SUBS PC, LR");
           }
         */
        uint32_t d = rd;
        uint32_t m = rm;
        bool setflags = s == 1;
        //ESO
        bits result(rf.r_read(m), 32);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
            }
        }
    }
}

void armv7a::arm_lsl_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_lsl_imm");
    inst.check(27, 21, B(0000 1101));
    inst.check(19, 16, 0);
    inst.check(6, 4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111) && (s == 1)))
        {
            printb(d_inst, "arm_lsl_imm SUBS PC, LR");
        }

        if(imm5 == 0)
        {
            printb(d_inst, "arm_lsl_imm MOV reg");
        }

        uint32_t d = rd;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, B(00), imm5);
        //ESO
        bits result;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&result, &carry, _rm, SRType_LSL, shift_n, rf.cpsr_C());

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_lsr_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_lsr_imm");
    inst.check(27, 21, B(000 1101));
    inst.check(19, 16, 0);
    inst.check(6, 4, B(010));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_lsr_imm SUBS PC,LR");
        }

        uint32_t d = rd;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, B(01), imm5);
        printd(d_inst, "rd=%d rm=%d imm5=%d shift_n=%d", rd, rm, imm5, shift_n);
        //ESO
        bits result;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&result, &carry, _rm, SRType_LSR, shift_n, rf.cpsr_C());
        printd(d_inst, "result=%X, rm_val=%X", result.val, _rm.val);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            printd(d_inst, "rd=%d, val=%X", d, result.val);
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_asr_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_asr_imm");
    inst.check(27, 21, B(000 1101));
    inst.check(19, 16, 0);
    inst.check(6, 4, B(100));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_asr_imm subs pc, lr");
        }

        uint32_t d = rd;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, B(10), imm5);
        //ESO
        bits result;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&result, &carry, _rm, SRType_ASR, shift_n, rf.cpsr_C());

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_rrx(armv7a_ir& inst)
{
    inst.print_inst("arm_rrx");
    inst.check(27, 21, B(1101));
    inst.check(19, 16, 0);
    inst.check(11, 4, B(110));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_rrx subs pc lr");
        }

        uint32_t d = rd;
        uint32_t m = rm;
        bool setflags = s == 1;
        //ESO
        bits result;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&result, &carry, _rm, SRType_RRX, 1, rf.cpsr_C());

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_ror_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_ror_imm");
    inst.check(27, 21, B(000 1101));
    inst.check(19, 16, 0);
    inst.check(6, 4, B(110));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_ror_imm SUBS PC LR");
        }

        if(imm5 == 0)
        {
            printb(d_inst, "arm_ror_imm RRX");
        }

        uint32_t d = rd;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, B(11), imm5);
        //ESO
        bits result;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&result, &carry, _rm, SRType_ROR, shift_n, rf.cpsr_C());

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_bic_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_big_reg");
    inst.check(27, 21, B(000 1110));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_bic_reg SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t not_shifted = (~shifted.val)&mask(shifted.n - 1, 0);
        bits result(rf.r_read(n)&not_shifted, 32);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

void armv7a::arm_mvn_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_mvn_reg");
    inst.check(27, 21, B(1111));
    inst.check(19, 16, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_mvn_reg SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t m = rm;
        bool setflags = s == 1;
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);
        //ESO
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(~shifted.val, shifted.n);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);

            if(setflags)
            {
                rf.cpsr_N(result(31));
                rf.cpsr_Z(is_zero_bit(result));
                rf.cpsr_C(carry);
            }
        }
    }
}

