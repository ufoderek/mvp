#include <armv7a.h>

void armv7a::arm_add_sp_plus_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_add_sp_plus_reg");
    inst.check(27, 21, B(100));
    inst.check(19, 16, B(1101));
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((rd == B(1111) && (s == 1)))
        {
            printb(d_inst, "arm_add_sp_plus_reg SUBS PC, LR");
        }

        uint32_t d = rd;
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
        bits _sp(rf.r_read(SP), 32);
        add_with_carry(&result, &carry, &overflow, _sp, shifted, 0);

        if(d == 15)
        {
            rf.alu_write_pc(result.val);
        }

        else
        {
            rf.r_write(d, result.val);
            uint32_t tmp = rf.r_read(d);
            printd(d_inst, "shifted:%X result:%X R%d:%X", shifted.val, result.val, d, tmp);

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

void armv7a::arm_add_sp_plus_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_add_sp_plus_imm");
    inst.check(27, 21, B(001 0100));
    inst.check(19, 16, B(1101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111)) && (s == 1))
        {
            printb(d_inst, "arm_add_sp_plus_imm SUBS PC LR");
        }

        uint32_t d = rd;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits sp(rf.r_read(SP), 32);
        add_with_carry(&result, &carry, &overflow, sp, imm32, 0);
        printd(d_inst, "SP(R%d)=0x%X, imm32=0x%X, result=r%d=0x%X", SP, sp.val, imm32.val, d, result.val);

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

void armv7a::arm_sub_sp_minus_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_sub_sp_minus_reg");
    inst.check(27, 21, B(010));
    inst.check(19, 16, B(1101));
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
            printb(d_inst, "arm_sub_sp_minus_reg SUBS PC LR");
        }

        uint32_t d = rd;
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
        bits sp(rf.r_read(SP), 32);
        bits not_shifted(~shifted.val, shifted.n);
        add_with_carry(&result, &carry, &overflow, sp, not_shifted, 1);

        if(rd == 15)
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

void armv7a::arm_sub_sp_minus_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_sub_sp_minus_reg");
    inst.check(27, 21, B(001 0010));
    inst.check(19, 16, B(1101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((rd == B(1111) & (s == 1)))
        {
            printb(d_inst, "arm_sub_sp_minus_reg SUBS PC LR");
        }

        uint32_t d = rd;
        bool setflags = s == 1;
        bits imm32;
        arm_expand_imm(&imm32, imm12);
        //ESO
        bits result;
        uint32_t carry;
        uint32_t overflow;
        bits _sp(rf.r_read(SP), 32);
        bits not_imm32(~imm32.val, 32);
        add_with_carry(&result, &carry, &overflow, _sp, not_imm32, 1);

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

void armv7a::arm_subs_pc_lr(armv7a_ir& inst)
{
    inst.print_inst("arm_subs_pc_lr");

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t opcode;
        uint32_t n;
        bits imm32;
        bool register_form;
        printd(d_inst, "original cpsr:%X", rf.cpsr());
        printd(d_inst, "original spsr:%X", rf.spsr());
        uint32_t m;
        sr_type shift_t;
        uint32_t shift_n;

        if(inst(25) == 1)
        {
            //Encoding A1
            inst.check(27, 25, 1);
            inst.check(20, 1);
            inst.check(15, 12, B(1111));
            opcode = inst(24, 21);
            uint32_t rn = inst(19, 16);
            bits imm12(inst(11, 0), 12);
            n = rn;
            arm_expand_imm(&imm32, imm12);
            register_form = false;
        }

        else
        {
            //Encoding A2
            inst.check(27, 25, 0);
            inst.check(20, 1);
            inst.check(15, 12, B(1111));
            inst.check(4, 0);
            opcode = inst(24, 21);
            uint32_t rn = inst(19, 16);
            uint32_t imm5 = inst(11, 7);
            uint32_t type = inst(6, 5);
            uint32_t rm = inst(3, 0);
            n = rn;
            m = rm;
            register_form = true;
            decode_imm_shift(&shift_t, &shift_n, type, imm5);
        }

        //ESO

        if(rf.current_inst_set() == InstSet_ThumbEE)
        {
            printb(d_inst, "arm_subs_pc_lr error");
        }

        bits operand2;

        if(register_form)
        {
            bits _rm(rf.r_read(m), 32);
            shift(&operand2, _rm, shift_t, shift_n, rf.cpsr_C());
        }

        else
        {
            operand2 = imm32;
        }

        bits result(0, 32);
        uint32_t carry;
        uint32_t overflow;
        bits _rn(rf.r_read(n), 32);
        bits not_rn(~rf.r_read(n), 32);
        bits not_operand2(~operand2.val, operand2.n);

        switch(opcode)
        {
            case B(0000):
                result.val = _rn.val & operand2.val;
                break;       //AND
            case B(0001):
                result.val = _rn.val ^ operand2.val;
                break;       //EOR
            case B(0010):
                add_with_carry(&result, &carry, &overflow, _rn, not_operand2, 1);
                break;      //SUB
            case B(0011):
                add_with_carry(&result, &carry, &overflow, not_rn, operand2, 1);
                break;       //RSB
            case B(0100):
                add_with_carry(&result, &carry, &overflow, _rn, operand2, 0);
                break;       //ADD
            case B(0101):
                add_with_carry(&result, &carry, &overflow, _rn, operand2, rf.cpsr_C());
                break;     //ADC
            case B(0110):
                add_with_carry(&result, &carry, &overflow, _rn, not_operand2, rf.cpsr_C());
                break;     //SBC
            case B(0111):
                add_with_carry(&result, &carry, &overflow, not_rn, operand2, rf.cpsr_C());
                break;     //RSC
            case B(1100):
                result.val = _rn.val | operand2.val;
                break;       //ORR
            case B(1101):
                result.val = operand2.val;
                break;     //MOV
            case B(1110):
                result.val = _rn.val & not_operand2.val;
                break;       //BIC
            case B(1111):
                result.val = not_operand2.val;
                break;     //MVN
            default:
                printb(d_inst, "arm_subs_pc_lr error 2");
        }

        cpsr_write_by_inst(rf.spsr(), B(1111), true);
        printd(d_inst, "new cpsr:%X", rf.cpsr());
        rf.branch_write_pc(result.val);
    }
}

