#include <armv7a.h>

void armv7a::decode_media(armv7a_ir& inst)
{
    uint32_t op1 = inst(24, 20);
    uint32_t rd = inst(15, 12);
    uint32_t op2 = inst(7, 5);
    uint32_t rn = inst(3, 0);
    bool op1_000xx = (op1 & B(11100)) == B(00000);
    bool op1_001xx = (op1 & B(11100)) == B(00100);
    bool op1_01xxx = (op1 & B(11000)) == B(01000);
    bool op1_10xxx = (op1 & B(11000)) == B(01000);
    bool op1_11000 = (op1 & B(11111)) == B(11000);
    bool op1_1101x = (op1 & B(11110)) == B(11010);
    bool op1_1110x = (op1 & B(11110)) == B(11100);
    bool op1_1111x = (op1 & B(11110)) == B(11110);
    bool op1_11111 = (op1 & B(11111)) == B(11111);
    bool op2_000 = (op2 & B(111)) == B(000);
    bool op2_x10 = (op2 & B(011)) == B(010);
    bool op2_x00 = (op2 & B(011)) == B(000);
    bool op2_111 = (op2 & B(111)) == B(111);
    bool rd_1111 = rd == B(1111);
    bool rn_1111 = rn == B(1111);

    if(op1_000xx)
    {
        decode_paddsub_s(inst);
    }

    else if(op1_001xx)
    {
        decode_paddsub_us(inst);
    }

    else if(op1_01xxx)
    {
        decode_pack_satrev(inst);
    }

    else if(op1_10xxx)
    {
        decode_smul(inst);
    }

    else if(op1_11000 && op2_000 && rd_1111)
    {
        arm_usad8(inst);
    }

    else if(op1_11000 && op2_000 && !rd_1111)
    {
        arm_usada8(inst);
    }

    else if(op1_1101x && op2_x10)
    {
        arm_sbfx(inst);
    }

    else if(op1_1110x && op2_x00 && rn_1111)
    {
        arm_bfc(inst);
    }

    else if(op1_1110x && op2_x00 && !rn_1111)
    {
        arm_bfi(inst);
    }

    else if(op1_1111x && op2_x10)
    {
        arm_ubfx(inst);
    }

    else if(op1_11111 && op2_111)
    {
        decode_undef(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_media, "decode error");
    }
}

void armv7a::arm_usad8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_usad8 not implemented yet.");
}

void armv7a::arm_usada8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_usada8 not implemented yet.");
}

void armv7a::arm_sbfx(armv7a_ir& inst)
{
    inst.print_inst("arm_sbfx");
    inst.check(27, 21, B(011 1101));
    inst.check(6, 4, B(101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t widthm1 = inst(20, 16);
        uint32_t rd = inst(15, 12);
        uint32_t lsb = inst(11, 7);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t lsbit = lsb;
        uint32_t widthminus1 = widthm1;

        if((d == 15) || (n == 15))
        {
            printb(d_inst, "arm_sbfx error");
        }

        //ESO
        uint32_t msbit = lsbit + widthminus1;

        if(msbit <= 31)
        {
            bits _rn(get_field(rf.r_read(n), msbit, lsbit), widthm1 + 1);
            bits _rd;
            sign_extend(&_rd, _rn, 32);
            rf.r_write(d, _rd.val);
        }

        else
        {
            printb(d_inst, "arm_sbfx error 2");
        }
    }
}

void armv7a::arm_bfc(armv7a_ir& inst)
{
    inst.print_inst("arm_bfc");
    inst.check(27, 21, B(011 1110));
    inst.check(6, 0, B(001 1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t msb = inst(20, 16);
        uint32_t rd = inst(15, 12);
        uint32_t lsb = inst(11, 7);
        uint32_t d = rd;
        uint32_t msbit = msb;
        uint32_t lsbit = lsb;

        if(d == 15)
        {
            printb(d_inst, "arm_bfc error");
        }

        //ESO

        if(msbit >= lsbit)
        {
            uint32_t _rd = rf.r_read(d);
            set_field(&_rd, msbit, lsbit, 0);
            rf.r_write(d, _rd);
        }

        else
        {
            printb(d_inst, "arm_bfc error 2");
        }
    }
}

void armv7a::arm_bfi(armv7a_ir& inst)
{
    inst.print_inst("arm_bfi");
    inst.check(27, 21, B(011 1110));
    inst.check(6, 4, B(001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t msb = inst(20, 16);
        uint32_t rd = inst(15, 12);
        uint32_t lsb = inst(11, 7);
        uint32_t rn = inst(3, 0);

        if(rn == B(1111))
        {
            printb(d_inst, "arm_bfi, see bfc");
        }

        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t msbit = msb;
        uint32_t lsbit = lsb;

        if(d == 15)
        {
            printb(d_inst, "arm_bfi error");
        }

        //ESO

        if(msbit >= lsbit)
        {
            uint32_t _rd = rf.r_read(d);
            uint32_t _rn = rf.r_read(n);
            set_field(&_rd, msbit, lsbit, get_field(_rn, msbit - lsbit, 0));
            rf.r_write(d, _rd);
        }

        else
        {
            printb(d_inst, "arm_bfi error 2");
        }
    }
}

void armv7a::arm_ubfx(armv7a_ir& inst)
{
    inst.print_inst("arm_ubfx");
    inst.check(27, 21, B(011 1111));
    inst.check(6, 4, B(101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t widthm1 = inst(20, 16);
        uint32_t rd = inst(15, 12);
        uint32_t lsb = inst(11, 7);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t lsbit = lsb;
        uint32_t widthminus1 = widthm1;

        if((d == 15) || (n == 15))
        {
            printb(d_inst, "arm_ubfx error");
        }

        //ESO
        uint32_t msbit = lsbit + widthminus1;

        if(msbit <= 31)
        {
            uint32_t _rd = get_field(rf.r_read(n), msbit, lsbit);
            rf.r_write(d, _rd);
        }

        else
        {
            printb(d_inst, "arm_ubfx error 2");
        }
    }
}

