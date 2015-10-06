#include <armv7a.h>

void armv7a::decode_dpimm(armv7a_ir& inst)
{
    uint32_t op = inst(24, 20);
    uint32_t rn = inst(19, 16);

    switch(op)
    {
        case B(0000 0):
        case B(0000 1):
            arm_and_imm(inst);
            return;
        case B(0001 0):
        case B(0001 1):
            arm_eor_imm(inst);
            return;
        case B(0010 0):
        case B(0010 1):

            if(rn != B(1111))
            {
                arm_sub_imm(inst);
            }

            else
            {
                arm_adr(inst);
            }

            return;
        case B(0011 0):
        case B(0011 1):
            arm_rsb_imm(inst);
            return;
        case B(0100 0):
        case B(0100 1):

            if(rn != B(1111))
            {
                arm_add_imm(inst);
            }

            else
            {
                arm_adr(inst);
            }

            return;
        case B(0101 0):
        case B(0101 1):
            arm_adc_imm(inst);
            return;
        case B(0110 0):
        case B(0110 1):
            arm_sbc_imm(inst);
            return;
        case B(0111 0):
        case B(0111 1):
            arm_rsc_imm(inst);
            return;
        case B(10001):
            arm_tst_imm(inst);
            return;
        case B(10011):
            arm_teq_imm(inst);
            return;
        case B(10101):
            arm_cmp_imm(inst);
            return;
        case B(10111):
            arm_cmn_imm(inst);
            return;
        case B(1100 0):
        case B(1100 1):
            arm_orr_imm(inst);
            return;
        case B(1101 0):
        case B(1101 1):
            arm_mov_imm(inst);
            return;
        case B(1110 0):
        case B(1110 1):
            arm_bic_imm(inst);
            return;
        case B(1111 0):
        case B(1111 1):
            arm_mvn_imm(inst);
            return;
        default:
            printb(core_id, d_armv7a_decode_dpimm, "decode error");
    }
}

void armv7a::arm_and_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_and_imm");
    inst.check(27, 21, B(001 0000));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_and_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        uint32_t carry;
        arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        //ESO
        bits result(rf.r_read(n)&imm32.val, 32);

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

void armv7a::arm_eor_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_eor_imm");
    inst.check(27, 21, B(001 0001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_eor_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        uint32_t carry;
        arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        //ESO
        bits result(rf.r_read(n) ^ imm32.val, 32);

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

void armv7a::arm_sub_imm(armv7a_ir& inst)
{
    uint32_t rn = inst(19, 16);

    if(rn == B(1101))
    {
        arm_sub_sp_minus_imm(inst);
        return;
    }

    inst.print_inst("arm_sub_imm");
    inst.check(27, 21, B(001 0010));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rn == B(1111)) && (s == 0))
        {
            printb(d_inst, "arm_sub_imm ADR");
        }

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_sub_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_imm32(~imm32.val, 32);
        add_with_carry(&result, &carry, &overflow, _rn, not_imm32, 1);

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

        /*
           if((d==14) && (result.val==0x9dac)){
            printd(d_inst, "sub_imm r%d=0x%X rn=0x%X imm12=0x%X imm32=0x%X", d, result.val, _rn.val, imm12.val, imm32.val);
           }
         */
    }
}

void armv7a::arm_adr(armv7a_ir& inst)
{
    inst.print_inst("arm_adr");

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);
        uint32_t d;
        bits imm32;
        bool add;

        if(inst(23) == 1)
        {
            //Encoding A1
            inst.check(27, 16, B(0010 1000 1111));
            d = rd;
            arm_expand_imm(&imm32, imm12);
            add = true;
        }

        else
        {
            //Encoding A2
            inst.check(27, 16, B(0010 0100 1111));
            d = rd;
            arm_expand_imm(&imm32, imm12);
            add = false;
        }

        //ESO
        uint32_t result;
        result = add ? (align(rf.pc(), 4) + imm32.val) : (align(rf.pc(), 4) - imm32.val);

        if(d == 15)
        {
            rf.alu_write_pc(result);
        }

        else
        {
            rf.r_write(d, result);
        }
    }
}

void armv7a::arm_rsb_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_rsb_imm");
    inst.check(27, 21, B(001 0011));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_rsb_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits not_rn(~rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, not_rn, imm32, 1);

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

void armv7a::arm_add_imm(armv7a_ir& inst)
{
    uint32_t rn = inst(19, 16);

    if(rn == B(1101))
    {
        arm_add_sp_plus_imm(inst);
        return;
    }

    inst.print_inst("arm_add_imm");
    inst.check(27, 21, B(001 0100));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rn == B(1111)) && (s == 0))
        {
            printb(d_inst, "arm_add_imm ADR");
        }

        if(rn == B(1101))
        {
            printb(d_inst, "arm_add_imm add SP plus imm");
        }

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_add_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, imm32, 0);
        printd(d_inst, "d:%d result:%X n:%d rn:%X imm32:%X", d, result.val, n, _rn.val, imm32.val);

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

void armv7a::arm_adc_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_adc_imm");
    inst.check(27, 21, B(001 0101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_adc_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, imm32, rf.cpsr_C());

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

void armv7a::arm_sbc_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_sbc_imm");
    inst.check(27, 21, B(001 0110));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_sbc_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_imm32(~imm32.val, 32);
        add_with_carry(&result, &carry, &overflow, _rn, not_imm32, rf.cpsr_C());

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

void armv7a::arm_rsc_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_rsc_imm");
    inst.check(27, 21, B(001 0111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_rsc_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits not_rn(~rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, not_rn, imm32, rf.cpsr_C());

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

void armv7a::arm_tst_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_tst_imm");
    inst.check(27, 20, B(0011 0001));
    inst.check(15, 12, B(0));

    if(rf.condition_passed(inst.cond()))
    {
        //Encoding A1
        uint32_t rn = inst(19, 16);
        bits imm12(inst(11, 0), 12);
        uint32_t n = rn;
        bits imm32;
        uint32_t carry;
        arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        //ESO
        bits result(rf.r_read(n)&imm32.val, 32);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
    }
}

void armv7a::arm_teq_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_teq_imm");
    inst.check(27, 20, B(0011 0011));
    inst.check(15, 12, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        bits imm12(inst(11, 0), 12);
        uint32_t n = rn;
        bits imm32;
        uint32_t carry;
        arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        //ESO
        bits result(rf.r_read(n) ^ imm32.val, 32);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
    }
}

void armv7a::arm_cmp_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_cmp_imm");
    inst.check(27, 20, B(0011 0101));
    inst.check(15, 12, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        bits imm12(inst(11, 0), 12);
        uint32_t n = rn;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_imm32(~imm32.val, 32);
        add_with_carry(&result, &carry, &overflow, _rn, not_imm32, 1);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
        rf.cpsr_V(overflow);
    }
}

void armv7a::arm_cmn_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_cmn_imm");
    inst.check(27, 20, B(0011 0111));
    inst.check(15, 12, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        bits imm12(inst(11, 0), 12);
        uint32_t n = rn;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        add_with_carry(&result, &carry, &overflow, _rn, imm32, 0);
        rf.cpsr_N(result(31));
        rf.cpsr_Z(is_zero_bit(result));
        rf.cpsr_C(carry);
        rf.cpsr_V(overflow);
    }
}

void armv7a::arm_orr_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_orr_imm");
    inst.check(27, 21, B(001 1100));

    if(rf.condition_passed(inst(31, 28)))
    {
        //Encoding A1
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_orr_imm SUBS PC,LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = (s == 1);
        bits imm32;
        uint32_t carry;
        arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        //ESO
        bits result(rf.r_read(n) | imm32.val, 32);

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

//in dpmisc
/*
   void armv7a::arm_mov_imm(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_mov_imm not implemented yet.");
   }
 */

void armv7a::arm_bic_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_bic_imm");
    inst.check(27, 21, B(001 1110));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_bic_imm SUBS PC LR");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        bool setflags = s == 1;
        bits imm32;
        uint32_t carry;
        arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        //ESO
        bits result(rf.r_read(n) & (~imm32.val), 32);

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

void armv7a::arm_mvn_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_mvn_imm");
    inst.check(27, 21, B(001 1111));
    inst.check(19, 16, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_mvn_imm SUBS PC LR");
        }

        uint32_t d = rd;
        bool setflags = s == 1;
        bits imm32;
        uint32_t carry;
        arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        //ESO
        bits result(~imm32.val, 32);

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
