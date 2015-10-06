#include <armv7a.h>

void armv7a::decode_misc_mhints(armv7a_ir& inst)
{
    uint32_t op1 = inst(26, 20);
    uint32_t rn = inst(19, 16);
    uint32_t op2 = inst(7, 4);
    bool op1_0010_000 = (op1 & B(1111 111)) == B(0010 000);
    bool op1_01xx_xxx = (op1 & B(1100 000)) == B(0100 000);
    bool op1_100x_xx0 = (op1 & B(1110 001)) == B(1000 000);
    bool op1_100x_001 = (op1 & B(1110 111)) == B(1000 001);
    bool op1_100x_101 = (op1 & B(1110 111)) == B(1000 101);
    bool op1_101x_001 = (op1 & B(1110 111)) == B(1010 001);
    bool op1_101x_101 = (op1 & B(1110 111)) == B(1010 101);
    bool op1_1010_111 = (op1 & B(1111 111)) == B(1010 111);
    bool op1_10xx_x11 = (op1 & B(1100 011)) == B(1000 011);
    bool op1_110x_001 = (op1 & B(1110 111)) == B(1100 001);
    bool op1_110x_101 = (op1 & B(1110 111)) == B(1100 101);
    bool op1_111x_001 = (op1 & B(1110 111)) == B(1110 001);
    bool op1_111x_101 = (op1 & B(1110 111)) == B(1110 101);
    bool op1_11xx_x01 = (op1 & B(1100 011)) == B(1100 001);
    bool op1_11xx_x11 = (op1 & B(1100 011)) == B(1100 011);
    bool op2_xx0x = (op2 & B(0010)) == B(0000);
    bool op2_0000 = (op2 & B(1111)) == B(0000);
    bool op2_0001 = (op2 & B(1111)) == B(0001);
    bool op2_0100 = (op2 & B(1111)) == B(0100);
    bool op2_0101 = (op2 & B(1111)) == B(0101);
    bool op2_0110 = (op2 & B(1111)) == B(0110);
    bool op2_xxx0 = (op2 & B(0001)) == B(0000);
    bool rn_xxx0 = (rn & B(0001)) == B(0000);
    bool rn_xxx1 = (rn & B(0001)) == B(0001);
    bool rn_1111 = (rn & B(1111)) == B(1111);

    if(op1_0010_000 && op2_xx0x && rn_xxx0)
    {
        arm_cps(inst);
    }

    else if(op1_0010_000 && op2_0000 && rn_xxx1)
    {
        arm_setend(inst);
    }

    else if(op1_01xx_xxx)
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: simd data processing");
    }

    else if(op1_100x_xx0)
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: simd element or structure load/store");
    }

    else if(op1_100x_001)
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: mp extension, unallocated memory hint, 1");
    }

    else if(op1_100x_101)
    {
        arm_pli_immltrl(inst);
    }

    else if(op1_101x_001 && !rn_1111)
    {
        arm_pld_imm(inst);
    }

    else if(op1_101x_001 && rn_1111)
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: unpredictable, pld");
    }

    else if(op1_101x_101 && !rn_1111)
    {
        arm_pld_imm(inst);
    }

    else if(op1_101x_101 && rn_1111)
    {
        arm_pld_ltrl(inst);
    }

    else if(op1_1010_111 && op2_0001)
    {
        arm_clrex(inst);
    }

    else if(op1_1010_111 && op2_0100)
    {
        arm_dsb(inst);
    }

    else if(op1_1010_111 && op2_0101)
    {
        arm_dmb(inst);
    }

    else if(op1_1010_111 && op2_0110)
    {
        arm_isb(inst);
    }

    else if(op1_10xx_x11)
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: unpredictable 1");
    }

    else if(op1_110x_001 && op2_xxx0)
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: mp extension, unallocated memory hint, 2");
    }

    else if(op1_110x_101 && op2_xxx0)
    {
        arm_pli_reg(inst);
    }

    else if(op1_111x_001 && op2_xxx0)
    {
        arm_pld_reg(inst);
    }

    else if(op1_111x_101 && op2_xxx0)
    {
        arm_pld_reg(inst);
    }

    else if(op1_11xx_x11)
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: unpredictable 2");
    }

    else
    {
        printb(core_id, d_armv7a_decode_misc_mhints, "decode error: 0x%X", inst.val);
    }
}

void armv7a::arm_cps(armv7a_ir& inst)
{
    inst.print_inst("arm_cps");
    inst.check(31, 20, B(1111 0001 0000));
    inst.check(16, 9, 0);
    inst.check(5, 0);
    uint32_t imod = inst(19, 18);
    uint32_t m = inst(17);
    uint32_t a = inst(8);
    uint32_t i = inst(7);
    uint32_t f = inst(6);
    uint32_t mode = inst(4, 0);

    if((mode != 0) && (m == 0))
    {
        printb(d_inst, "arm_cps error");
    }

    uint32_t aif = (a << 2) | (i << 1) | f;

    if(((get_bit(imod, 1) == 1) && (aif == 0)) || ((get_bit(imod, 1) == 0) && (aif != 0)))
    {
        printb(d_inst, "arm_cps error 2");
    }

    bool enable = imod == B(10);
    bool disable = imod == B(11);
    bool change_mode = m == 1;
    bool affectA = a == 1;
    bool affectI = i == 1;
    bool affectF = f == 1;

    if(((imod == B(00)) && (m == 0)) || (imod == B(01)))
    {
        printb(d_inst, "arm_cps error 3");
    }

    //ESO

    if(rf.current_mode_is_not_user())
    {
        uint32_t cpsr_val = rf.cpsr();

        if(enable)
        {
            if(affectA)
            {
                set_bit(&cpsr_val, 8, 0);
            }

            if(affectI)
            {
                set_bit(&cpsr_val, 7, 0);
            }

            if(affectF)
            {
                set_bit(&cpsr_val, 6, 0);
            }
        }

        if(disable)
        {
            if(affectA)
            {
                set_bit(&cpsr_val, 8, 1);
            }

            if(affectI)
            {
                set_bit(&cpsr_val, 7, 1);
            }

            if(affectF)
            {
                set_bit(&cpsr_val, 6, 1);
            }
        }

        if(change_mode)
        {
            set_field(&cpsr_val, 4, 0, mode);
        }

        printd(d_inst, "cpsr_val = 0x%X, mode = 0x%X", cpsr_val, mode);
        cpsr_write_by_inst(cpsr_val, B(1111), true);
    }
}

void armv7a::arm_setend(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_setend not implemented yet.");
}

void armv7a::arm_pli_immltrl(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_pli_immltrl not implemented yet.");
}

void armv7a::arm_pld_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_pld_imm");
    inst.check(31, 24, B(1111 0101));
    inst.check(21, 20, B(01));
    inst.check(15, 12, B(1111));
}

void armv7a::arm_pld_ltrl(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_pld_ltrl not implemented yet.");
}

void armv7a::arm_clrex(armv7a_ir& inst)
{
    inst.print_inst("arm_clrex");
    inst.check(31, 16, B(1111 0101 0111 1111));
    inst.check(15, 0, B(1111 0000 0001 1111));

    if(rf.condition_passed(inst.cond()))
    {
        //ESO
        clear_exclusive_local(core_id);
    }
}

void armv7a::arm_dsb(armv7a_ir& inst)
{
    inst.print_inst("arm_dsb");
    inst.check(31, 16, B(1111 0101 0111 1111));
    inst.check(15, 4, B(1111 0000 0100));
}

void armv7a::arm_dmb(armv7a_ir& inst)
{
    inst.print_inst("arm_dmb");
    inst.check(31, 16, B(1111 0101 0111 1111));
    inst.check(15, 4, B(1111 0000 0101));
}

void armv7a::arm_isb(armv7a_ir& inst)
{
    inst.print_inst("arm_isb");
    inst.check(31, 16, B(1111 0101 0111 1111));
    inst.check(15, 4, B(1111 0000 0110));
}

void armv7a::arm_pli_reg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_pli_reg not implemented yet.");
}

void armv7a::arm_pld_reg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_pld_reg not implemented yet.");
}

