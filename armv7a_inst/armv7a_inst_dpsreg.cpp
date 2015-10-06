#include <armv7a.h>

void armv7a::decode_dpsreg(armv7a_ir& inst)
{
    uint32_t op1 = inst(24, 20);
    uint32_t op2 = inst(6, 5);

    switch(op1)
    {
        case B(0000 0):
        case B(0000 1):
            arm_and_sreg(inst);
            return;
        case B(0001 0):
        case B(0001 1):
            arm_eor_sreg(inst);
            return;
        case B(0010 0):
        case B(0010 1):
            arm_sub_sreg(inst);
            return;
        case B(0011 0):
        case B(0011 1):
            arm_rsb_sreg(inst);
            return;
        case B(0100 0):
        case B(0100 1):
            arm_add_sreg(inst);
            return;
        case B(0101 0):
        case B(0101 1):
            arm_adc_sreg(inst);
            return;
        case B(0110 0):
        case B(0110 1):
            arm_sbc_sreg(inst);
            return;
        case B(0111 0):
        case B(0111 1):
            arm_rsc_sreg(inst);
            return;
        case B(10001):
            arm_tst_sreg(inst);
            return;
        case B(10011):
            arm_teq_sreg(inst);
            return;
        case B(10101):
            arm_cmp_sreg(inst);
            return;
        case B(10111):
            arm_cmn_sreg(inst);
            return;
        case B(1100 0):
        case B(1100 1):
            arm_orr_sreg(inst);
            return;
        case B(1101 0):
        case B(1101 1):

            if(op2 == B(00))
            {
                arm_lsl_reg(inst);
            }

            else if(op2 == B(01))
            {
                arm_lsr_reg(inst);
            }

            else if(op2 == B(10))
            {
                arm_asr_reg(inst);
            }

            else if(op2 == B(11))
            {
                arm_ror_reg(inst);
            }

            else
            {
                printb(core_id, d_armv7a_decode_dpsreg, "decode error");
            }

            return;
        case B(1110 0):
        case B(1110 1):
            arm_bic_sreg(inst);
            return;
        case B(1111 0):
        case B(1111 1):
            arm_mvn_sreg(inst);
            return;
        default:
            printb(core_id, d_armv7a_decode_dpsreg, "decode error");
            return;
    }
}

void armv7a::arm_and_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_and_sreg");
    inst.check(27, 21, 0);
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;
        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_and_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n)&shifted.val, 32);
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

void armv7a::arm_eor_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_eor_sreg");
    inst.check(27, 21, 1);
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;
        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_eor_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n) ^ shifted.val, 32);
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

void armv7a::arm_sub_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_sub_sreg");
    inst.check(27, 21, B(10));
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;
        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_and_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_shifted(~shifted.val, shifted.n);
        add_with_carry(&result, &carry, &overflow, _rn, not_shifted, 1);
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

void armv7a::arm_rsb_sreg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_rsb_sreg not implemented yet.");
}

void armv7a::arm_add_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_add_sreg");
    inst.check(27, 21, B(000 0100));
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;
        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_add_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, shifted, 0);
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

void armv7a::arm_adc_sreg(armv7a_ir& inst)
{
    inst.check(27, 21, B(000 0101));
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;

        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_adc_sreg error");
        }

        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        bits _rm(rf.r_read(m), 32);
        shift(&shifted, _rm, shift_t, shift_n, rf.cpsr_C());

        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, shifted, rf.cpsr_C());

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

void armv7a::arm_sbc_sreg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sbc_sreg not implemented yet.");
}

void armv7a::arm_rsc_sreg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_rsc_sreg not implemented yet.");
}

void armv7a::arm_tst_sreg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_tst_sreg not implemented yet.");
}

void armv7a::arm_teq_sreg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_teq_sreg not implemented yet.");
}

void armv7a::arm_cmp_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_cmp_sreg");
    inst.check(27, 20, B(0001 0101));
    inst.check(15, 12, 0);
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;
        sr_type shift_t = decode_reg_shift(type);

        if((n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_cmp_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
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
    }
}

void armv7a::arm_cmn_sreg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_cmn_sreg not implemented yet.");
}

void armv7a::arm_orr_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_orr_sreg");
    inst.check(27, 21, B(000 1100));
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;
        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_orr_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(rf.r_read(n) | shifted.val, 32);
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

void armv7a::arm_lsl_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_lsl_reg");
    inst.check(27, 21, B(000 1101));
    inst.check(19, 16, 0);
    inst.check(7, 4, B(0001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((d == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_lsl_reg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(m), 7, 0);
        bits result;
        uint32_t carry;
        bits _rn(rf.r_read(n), 32);
        shift_c(&result, &carry, _rn, SRType_LSL, shift_n, rf.cpsr_C());
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

void armv7a::arm_lsr_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_lsr_reg");
    inst.check(27, 21, B(000 1101));
    inst.check(19, 16, 0);
    inst.check(7, 4, B(0011));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((d == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_lsr_reg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(m), 7, 0);
        bits result;
        uint32_t carry;
        bits _rn(rf.r_read(n), 32);
        shift_c(&result, &carry, _rn, SRType_LSR, shift_n, rf.cpsr_C());
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

void armv7a::arm_asr_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_asr_reg");
    inst.check(27, 21, B(000 1101));
    inst.check(19, 16, 0);
    inst.check(7, 4, B(0101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((d == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_asr_reg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(m), 7, 0);
        bits result;
        uint32_t carry;
        bits _rn(rf.r_read(n), 32);
        shift_c(&result, &carry, _rn, SRType_ASR, shift_n, rf.cpsr_C());
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

void armv7a::arm_ror_reg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ror_reg not implemented yet.");
}

void armv7a::arm_bic_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_bic_sreg");
    inst.check(27, 21, B(000 1110));
    inst.check(7, 0);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t s = rs;
        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (n == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_bic_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t not_shifted = (~shifted.val)&mask(shifted.n - 1, 0);
        bits result(rf.r_read(n)&not_shifted, 32);
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

void armv7a::arm_mvn_sreg(armv7a_ir& inst)
{
    inst.print_inst("arm_mvn_sreg");
    inst.check(27, 21, B(000 1111));
    inst.check(19, 16, 0);
    inst.check(7, 4);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t _s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t rs = inst(11, 8);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t m = rm;
        uint32_t s = rs;
        bool setflags = _s == 1;
        sr_type shift_t = decode_reg_shift(type);

        if((d == 15) || (m == 15) || (s == 15))
        {
            printb(d_inst, "arm_mvn_sreg error");
        }

        //ESO
        uint32_t shift_n = get_field(rf.r_read(s), 7, 0);
        bits shifted;
        uint32_t carry;
        bits _rm(rf.r_read(m), 32);
        shift_c(&shifted, &carry, _rm, shift_t, shift_n, rf.cpsr_C());
        bits result(~shifted.val, shifted.n);
        rf.r_write(d, result.val);

        if(setflags)
        {
            rf.cpsr_N(result(31));
            rf.cpsr_Z(is_zero_bit(result));
            rf.cpsr_C(carry);
        }
    }
}

