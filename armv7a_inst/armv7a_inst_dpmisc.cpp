#include <armv7a.h>

void armv7a::decode_dpmisc(armv7a_ir& inst)
{
    uint32_t op = inst(25);
    uint32_t op1 = inst(24, 20);
    uint32_t op2 = inst(7, 4);
    bool op1_10xx0 = (op1 & B(11001)) == B(10000);
    bool op1_0xxxx = (op1 & B(10000)) == B(00000);
    bool op1_1xxxx = (op1 & B(10000)) == B(10000);
    bool op1_0xx1x = (op1 & B(10010)) == B(00010);
    bool op1_10000 = (op1 & B(11111)) == B(10000);
    bool op1_10100 = (op1 & B(11111)) == B(10100);
    bool op1_10x10 = (op1 & B(11011)) == B(10010);
    bool op2_xxx0 = (op2 & B(0001)) == B(0000);
    bool op2_0xx1 = (op2 & B(1001)) == B(0001);
    bool op2_0xxx = (op2 & B(1000)) == B(0000);
    bool op2_1xx0 = (op2 & B(1001)) == B(1000);
    bool op2_1001 = (op2 & B(1111)) == B(1001);
    bool op2_1011 = (op2 & B(1111)) == B(1011);
    bool op2_11x1 = (op2 & B(1101)) == B(1101);

    if(op == 0)
    {
        if(!op1_10xx0 && op2_xxx0)
        {
            decode_dpreg(inst);
        }

        else if(!op1_10xx0 && op2_0xx1)
        {
            decode_dpsreg(inst);
        }

        else if(op1_10xx0 && op2_0xxx)
        {
            decode_misc(inst);
        }

        else if(op1_10xx0 && op2_1xx0)
        {
            decode_hmulmacc(inst);
        }

        else if(op1_0xxxx && op2_1001)
        {
            decode_mulmacc(inst);
        }

        else if(op1_1xxxx && op2_1001)
        {
            decode_sync(inst);
        }

        else if(!op1_0xx1x && op2_1011)
        {
            decode_extrals(inst);
        }

        else if(!op1_0xx1x && op2_11x1)
        {
            decode_extrals(inst);
        }

        else if(op1_0xx1x && op2_1011)
        {
            decode_extrals_up(inst);
        }

        else if(op1_0xx1x && op2_11x1)
        {
            decode_extrals_up(inst);
        }

        else
        {
            printb(core_id, d_armv7a_decode_dpmisc, "decode error, op == 0");
        }
    }

    else
    {
        if(!op1_10xx0)
        {
            decode_dpimm(inst);
        }

        else if(op1_10000)
        {
            arm_mov_imm(inst);
        }

        else if(op1_10100)
        {
            arm_movt(inst);
        }

        else if(op1_10x10)
        {
            decode_msrhints(inst);
        }

        else
        {
            printb(core_id, d_armv7a_decode_dpmisc, "decode error, op == 1");
        }
    }
}

void armv7a::arm_mov_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_mov_imm");

    if(rf.condition_passed(inst.cond()))
    {
        bits imm32(0, 32);
        bool setflags;
        uint32_t d;
        uint32_t carry;

        //Encoding A1
        if(inst(21) == 1)
        {
            inst.check(27, 21, B(001 1101));
            uint32_t s = inst(20);
            uint32_t rd = inst(15, 12);
            bits imm12(inst(11, 0), 12);

            if((rd == B(1111)) && (s == 1))
            {
                printb(d_inst_dpmisc, "arm_mov_imm SUBS PC, LR");
            }

            d = rd;
            setflags = s == 1;
            arm_expand_imm_c(&imm32, &carry, imm12, rf.cpsr_C());
        }

        //Encoding A2
        else
        {
            inst.check(27, 20, B(0011 0000));
            uint32_t rd = inst(15, 12);
            bits imm4_12((inst(19, 16) << 12) | inst(11, 0), 16);
            d = rd;
            setflags = false;
            zero_extend(&imm32, imm4_12, 32);

            if(d == 15)
            {
                printb(d_inst, "arm_mov_imm unpredictable");
            }
        }

        //ESO
        bits result(imm32.val, 32);

        if(d == 15)
        {
            printd(d_inst, "mov_imm d == 15");
            rf.alu_write_pc(result.val);
        }

        else
        {
            printd(d_inst, "mov_imm d != 15");
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

void armv7a::arm_movt(armv7a_ir& inst)
{
    //printb(core_id, d_inst, "arm_movt not implemented yet.");
    inst.check(27, 20, B(0011 0100));
    inst.print_inst("arm_movt");

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t imm4 = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t imm12 = inst(11, 0);

        uint32_t d = rd;
        uint32_t imm16 = (imm4 << 12) | imm12;

#ifdef CPU_ASSERT
        if(d == 15)
        {
            printb(d_inst, "movt error");
        }
#endif

        uint32_t rd_val = rf.r_read(d);

        set_field(&rd_val, 31, 16, imm16);

        rf.r_write(d, rd_val);
    }
}
