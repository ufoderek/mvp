#include <armv7a.h>

void armv7a::decode_pack_satrev(armv7a_ir& inst)
{
    uint32_t op1 = inst(22, 20);
    uint32_t a = inst(19, 16);
    uint32_t op2 = inst(7, 5);
    bool op1_000 = (op1 & B(111)) == B(000);
    bool op1_01x = (op1 & B(110)) == B(010);
    bool op1_11x = (op1 & B(110)) == B(110);
    bool op1_010 = (op1 & B(111)) == B(010);
    bool op1_011 = (op1 & B(111)) == B(011);
    bool op1_100 = (op1 & B(111)) == B(100);
    bool op1_110 = (op1 & B(111)) == B(110);
    bool op1_111 = (op1 & B(111)) == B(111);
    bool op2_xx0 = (op2 & B(001)) == B(000);
    bool op2_011 = (op2 & B(111)) == B(011);
    bool op2_101 = (op2 & B(111)) == B(101);
    bool op2_001 = (op2 & B(111)) == B(001);
    bool a_1111 = a == B(1111);

    if(op1_000 && op2_xx0)
    {
        arm_pkh(inst);
    }

    else if(op1_01x && op2_xx0)
    {
        arm_ssat(inst);
    }

    else if(op1_11x && op2_xx0)
    {
        arm_usat(inst);
    }

    else if(op1_000 && op2_011 && !a_1111)
    {
        arm_sxtab16(inst);
    }

    else if(op1_000 && op2_011 && a_1111)
    {
        arm_sxtb16(inst);
    }

    else if(op1_000 && op2_101)
    {
        arm_sel(inst);
    }

    else if(op1_010 && op2_001)
    {
        arm_ssat16(inst);
    }

    else if(op1_010 && op2_011 && !a_1111)
    {
        arm_sxtab(inst);
    }

    else if(op1_010 && op2_011 && a_1111)
    {
        arm_sxtb(inst);
    }

    else if(op1_011 && op2_001)
    {
        arm_rev(inst);
    }

    else if(op1_011 && op2_011 && !a_1111)
    {
        arm_sxtah(inst);
    }

    else if(op1_011 && op2_011 && a_1111)
    {
        arm_sxth(inst);
    }

    else if(op1_011 && op2_101)
    {
        arm_rev16(inst);
    }

    else if(op1_100 && op2_011 && !a_1111)
    {
        arm_uxtab16(inst);
    }

    else if(op1_100 && op2_011 && a_1111)
    {
        arm_uxtb16(inst);
    }

    else if(op1_110 && op2_001)
    {
        arm_usat16(inst);
    }

    else if(op1_110 && op2_011 && !a_1111)
    {
        arm_uxtab(inst);
    }

    else if(op1_110 && op2_011 && a_1111)
    {
        arm_uxtb(inst);
    }

    else if(op1_111 && op2_001)
    {
        arm_rbit(inst);
    }

    else if(op1_111 && op2_011 && !a_1111)
    {
        arm_uxtah(inst);
    }

    else if(op1_111 && op2_011 && a_1111)
    {
        arm_uxth(inst);
    }

    else if(op1_111 && op2_101)
    {
        arm_revsh(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_pack_satrev, "decode error");
    }
}

void armv7a::arm_pkh(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_pkh not implemented yet.");
}

void armv7a::arm_ssat(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ssat not implemented yet.");
}

void armv7a::arm_usat(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_usat not implemented yet.");
}

void armv7a::arm_sxtab16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sxtab16 not implemented yet.");
}

void armv7a::arm_sxtb16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sxtb16 not implemented yet.");
}

void armv7a::arm_sel(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sel not implemented yet.");
}

void armv7a::arm_ssat16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ssat16 not implemented yet.");
}

void armv7a::arm_sxtab(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sxtab not implemented yet.");
}

void armv7a::arm_sxtb(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sxtb not implemented yet.");
}

void armv7a::arm_rev(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_rev not implemented yet.");
}

void armv7a::arm_sxtah(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sxtah not implemented yet.");
}

void armv7a::arm_sxth(armv7a_ir& inst)
{
    //printb(d_inst, "arm_sxth");
    inst.print_inst("arm_sxth");
    inst.check(27, 16, B(0110 1011 1111));
    inst.check(9, 4, B(111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rd = inst(15, 12);
        uint32_t rotate = inst(11, 10);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t m = rm;
        uint32_t rotation = rotate << 3;

        if((d == 15) || (m == 15))
        {
            printb(d_inst, "arm_sxth error");
        }

        //ESO
        bits rotated;
        bits _rm(rf.r_read(m), 32);
        ror(&rotated, _rm, rotation);
        bits result;
        rotated.val &= mask(15, 0);
        rotated.n = 16;
        sign_extend(&result, rotated, 32);
        rf.r_write(d, result.val);
    }
}

void armv7a::arm_rev16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_rev16 not implemented yet.");
}

void armv7a::arm_uxtab16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uxtab16 not implemented yet.");
}

void armv7a::arm_uxtb16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uxtb16 not implemented yet.");
}

void armv7a::arm_usat16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_usat16 not implemented yet.");
}

void armv7a::arm_uxtab(armv7a_ir& inst)
{
    //printb(d_inst, "arm_uxtab");
    inst.print_inst("arm_uxtab");
    inst.check(27, 20, B(0110 1110));
    inst.check(9, 4, B(111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rotate = inst(11, 10);
        uint32_t rm = inst(3, 0);

        if(rn == B(1111))
        {
            printb(d_inst, "uxtab to uxtb");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t rotation = rotate << 3;

        if((d == 15) || (m == 15))
        {
            printb(d_inst, "uxtab error");
        }

        //ESO
        bits rotated;
        bits _rm(rf.r_read(m), 32);
        ror(&rotated, _rm, rotation);
        rf.r_write(d, rf.r_read(n) + rotated(7, 0));
    }
}

void armv7a::arm_uxtb(armv7a_ir& inst)
{
    inst.print_inst("arm_uxtb");
    inst.check(27, 16, B(0110 1110 1111));
    inst.check(9, 4, B(00 0111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rd = inst(15, 12);
        uint32_t rotate = inst(11, 10);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t m = rm;
        uint32_t rotation = rotate << 3;
        printd(d_inst, "uxtb rotate:%d rotation:%d", rotate, rotation);

        if((d == 15) || (m == 15))
        {
            printb(d_inst, "arm_uxtb error");
        }

        //ESO
        bits rotated;
        bits _rm(rf.r_read(m), 32);
        ror(&rotated, _rm, rotation);
        rf.r_write(d, rotated(7, 0));
    }
}

void armv7a::arm_rbit(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_rbit not implemented yet.");
}

void armv7a::arm_uxtah(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uxtah not implemented yet.");
}

void armv7a::arm_uxth(armv7a_ir& inst)
{
    inst.print_inst("arm_uxth");
    inst.check(27, 16, B(0110 1111 1111));
    inst.check(9, 4, B(00 0111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rd = inst(15, 12);
        uint32_t rotate = inst(11, 10);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t m = rm;
        uint32_t rotation = rotate << 3;

        if((d == 15) || (m == 15))
        {
            printb(d_inst, "arm_uxth error");
        }

        //ESO
        bits rotated;
        bits _rm(rf.r_read(m), 32);
        ror(&rotated, _rm, rotation);
        rf.r_write(d, rotated(15, 0));
    }
}

void armv7a::arm_revsh(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_revsh not implemented yet.");
}

