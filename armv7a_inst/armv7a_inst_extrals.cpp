#include <armv7a.h>

void armv7a::decode_extrals(armv7a_ir& inst)
{
    uint32_t rn = inst(19, 16);
    uint32_t op2 = inst(6, 5);
    uint32_t op1_trim = (inst(22) << 1) | inst(20);

    if(op2 == B(01))
    {
        switch(op1_trim)
        {
            case B(00):
                arm_strh_reg(inst);
                return;
            case B(01):
                arm_ldrh_reg(inst);
                return;
            case B(10):
                arm_strh_imm(inst);
                return;
            case B(11):

                if(rn != B(1111))
                {
                    arm_ldrh_imm(inst);
                }

                else
                {
                    arm_ldrh_ltrl(inst);
                }

                return;
            default:
                printb(core_id, d_armv7a_decode_extrals, "decode error 00");
        }
    }

    else if(op2 == B(10))
    {
        switch(op1_trim)
        {
            case B(00):
                arm_ldrd_reg(inst);
                return;
            case B(01):
                arm_ldrsb_reg(inst);
                return;
            case B(10):

                if(rn != B(1111))
                {
                    arm_ldrd_imm(inst);
                }

                else
                {
                    arm_ldrd_ltrl(inst);
                }

                return;
            case B(11):

                if(rn != B(1111))
                {
                    arm_ldrsb_imm(inst);
                }

                else
                {
                    arm_ldrsb_ltrl(inst);
                }

                return;
            default:
                printb(core_id, d_armv7a_decode_extrals, "decode error 10");
        }
    }

    else if(op2 == B(11))
    {
        switch(op1_trim)
        {
            case B(00):
                arm_strd_reg(inst);
                return;
            case B(01):
                arm_ldrsh_reg(inst);
                return;
            case B(10):
                arm_strd_imm(inst);
                return;
            case B(11):

                if(rn != B(1111))
                {
                    arm_ldrsh_imm(inst);
                }

                else
                {
                    arm_ldrsh_ltrl(inst);
                }

                return;
            default:
                printb(core_id, d_armv7a_decode_extrals, "decode error 11");
        }
    }

    else
    {
        printb(core_id, d_armv7a_decode_extrals, "decode error");
    }
}

void armv7a::arm_strh_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_strh_reg");
    inst.check(27, 25, 0);
    inst.check(22, 0);
    inst.check(20, 0);
    inst.check(11, 4, B(0000 1011));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_strh_reg STRHT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);
        sr_type shift_t = SRType_LSL;
        uint32_t shift_n = 0;

        if((t == 15) || (m == 15))
        {
            printb(d_inst, "arm_strh_reg error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_strh_reg error 2");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_strh_reg error 3");
        }

        //ESO
        null_check_if_thumbee(n);
        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);

        if(armv7a::unaligned_support() || (get_bit(address, 0) == 0))
        {
            bits _rt(rf.r_read(t), 16);

            if(!mem_u_write(address, 2, _rt))
            {
                return;
            }
        }

        else
        {
            printb(d_inst, "arm_strh_reg error 4");
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrh_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrh_reg");
    inst.check(27, 25, 0);
    inst.check(22, 0);
    inst.check(20, 1);
    inst.check(11, 4, B(0000 1011));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "ldrh_reg LDRHT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);
        sr_type shift_t = SRType_LSL;
        uint32_t shift_n = 0;

        if((t == 15) || (m == 15))
        {
            printb(d_inst, "ldrh_reg error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "ldrh_reg error 2");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "ldrh_reg error 3");
        }

        //ESO
        null_check_if_thumbee(n);
        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 2))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }

        if(armv7a::unaligned_support() || (get_bit(address, 0) == 0))
        {
            rf.r_write(t, data.val);
        }

        else
        {
            printb(d_inst, "ldrh_reg error 4");
        }
    }
}

void armv7a::arm_strh_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_strh_imm");
    inst.check(27, 25, 0);
    inst.check(22, 1);
    inst.check(20, 0);
    inst.check(7, 4, B(1011));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm4H = inst(11, 8);
        uint32_t imm4L = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_strh_imm STRHT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = (imm4H << 4) | imm4L;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if(t == 15)
        {
            printb(d_inst, "arm_strh_imm error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_strh_imm error 2");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);

        if(armv7a::unaligned_support() || (get_bit(address, 0) == 0))
        {
            bits _rt(rf.r_read(t), 16);

            if(!mem_u_write(address, 2, _rt))
            {
                return;
            }
        }

        else
        {
            printb(d_inst, "arm_strh_imm error 3");
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrh_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrh_imm");
    inst.check(27, 25, 0);
    inst.check(22, 1);
    inst.check(20, 1);
    inst.check(7, 4, B(1011));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm4H = inst(11, 8);
        uint32_t imm4L = inst(3, 0);

        if(rn == B(1111))
        {
            printb(d_inst, "arm_ldrh_imm LDRH literal");
        }

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrh_imm LDRHT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = (imm4H << 4) | imm4L;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((t == 15) || (wback && (n == t)))
        {
            printb(d_inst, "arm_ldrh_imm error");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 2))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }

        if(armv7a::unaligned_support() || (get_bit(address, 0) == 0))
        {
            rf.r_write(t, data.val);
        }

        else
        {
            printb(d_inst, "arm_ldrh_imm error 2");
        }
    }
}

void armv7a::arm_ldrh_ltrl(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrh_ltrl not implemented yet.");
}

void armv7a::arm_ldrd_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrd_reg");
    inst.check(27, 25, 0);
    inst.check(22, 0);
    inst.check(20, 0);
    inst.check(11, 4, B(0000 1101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t rm = inst(3, 0);

        if(get_bit(rt, 0) == 1)
        {
            printb(d_inst, "arm_ldrd_reg error");
        }

        uint32_t t = rt;
        uint32_t t2 = t + 1;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrd_reg error 2");
        }

        if((t2 == 15) || (m == 15) || (m == t) || (m == t2))
        {
            printb(d_inst, "arm_ldrd_reg error 3");
        }

        if(wback && ((n == 15) || (n == t) || (n == t2)))
        {
            printb(d_inst, "arm_ldrd_reg error 4");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_ldrd_reg error 5");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + rf.r_read(m)) : (rf.r_read(n) - rf.r_read(m));
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_a_read(&data, address, 4))
        {
            return;
        }

        rf.r_write(t, data.val);

        if(!mem_a_read(&data, address + 4, 4))
        {
            return;
        }

        rf.r_write(t2, data.val);

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrsb_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrsb_reg");
    inst.check(27, 25, 0);
    inst.check(22, 0);
    inst.check(20, 1);
    inst.check(11, 4, B(0000 1101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrsb_reg ldrsbt");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);
        sr_type shift_t = SRType_LSL;
        uint32_t shift_n = 0;

        if((t == 15) || (m == 15))
        {
            printb(d_inst, "arm_ldrsb_reg error");
        }
        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_ldrsb_reg error 2");
        }
        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_ldrsb_reg error 3");
        }

        //ESO
        null_check_if_thumbee(n);

        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());

        uint32_t offset_addr = add ? rf.r_read(n) + offset.val : rf.r_read(n) - offset.val;
        uint32_t address = index ? offset_addr : rf.r_read(n);

        bits data;
        mem_u_read(&data, address, 1);

        bits _rt;
        sign_extend(&_rt, data, 32);

        rf.r_write(t, _rt.val);

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrd_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrd_imm");
    inst.check(27, 25, 0);
    inst.check(22, 1);
    inst.check(20, 0);
    inst.check(7, 4, B(1101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm4H = inst(11, 8);
        uint32_t imm4L = inst(3, 0);

        if(rn == B(1111))
        {
            printb(d_inst, "arm_ldrd_imm LDRD literal");
        }

        if(get_bit(rt, 0) == 1)
        {
            printb(d_inst, "arm_ldrd_imm error");
        }

        uint32_t t = rt;
        uint32_t t2 = t + 1;
        uint32_t n = rn;
        uint32_t imm32 = (imm4H << 4) | imm4L;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrd_imm error2");
        }

        if(wback && ((n == t) || (n == t2)))
        {
            printb(d_inst, "arm_ldrd_imm error3");
        }

        if(t2 == 15)
        {
            printb(d_inst, "arm_ldrd_imm error4");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_a_read(&data, address, 4))
        {
            return;
        }

        rf.r_write(t, data.val);

        if(!mem_a_read(&data, address + 4, 4))
        {
            return;
        }

        rf.r_write(t2, data.val);

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrd_ltrl(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrd_ltrl not implemented yet.");
}

void armv7a::arm_ldrsb_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrsb_imm");
    inst.check(27, 25, B(000));
    inst.check(22, 1);
    inst.check(20, 1);
    inst.check(7, 4, B(1101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm4H = inst(11, 8);
        uint32_t imm4L = inst(3, 0);

        if(rn == B(1111))
        {
            printb(d_inst, "arm_ldrsb_imm LDRSB literal");
        }

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrsb_reg ldrsbt");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = (imm4H << 4) | imm4L;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((t == 15) || (wback && (n == t)))
        {
            printb(d_inst, "arm_ldrsb error");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 1))
        {
            return;
        }

        rf.r_write(t, sign_extend32(data));

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrsb_ltrl(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrsb_ltrl not implemented yet.");
}

void armv7a::arm_strd_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_strd_reg");
    inst.check(27, 25, 0);
    inst.check(22, 0);
    inst.check(20, 0);
    inst.check(11, 4, B(0000 1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t rm = inst(3, 0);

        if(get_bit(rt, 0) == 1)
        {
            printb(d_inst, "arm_strd_reg error");
        }

        uint32_t t = rt;
        uint32_t t2 = t + 1;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_strd_reg error 2");
        }

        if((t2 == 15) || (m == 15))
        {
            printb(d_inst, "arm_strd_reg error 3");
        }

        if(wback && ((n == 15) || (n == t) || (n == t2)))
        {
            printb(d_inst, "arm_strd_reg error 4");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_strd_reg error 5");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + rf.r_read(m)) : (rf.r_read(n) - rf.r_read(m));
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits _rt(rf.r_read(t), 32);
        bits _rt2(rf.r_read(t2), 32);

        if(!mem_a_write(address, 4, _rt))
        {
            return;
        }

        if(!mem_a_write(address + 4, 4, _rt2))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrsh_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrsh_reg");
    inst.check(27, 25, B(000));
    inst.check(22, 0);
    inst.check(20, 1);
    inst.check(11, 4, B(00001111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrsh_reg ldrsht");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        sr_type shift_t = SRType_LSL;
        uint32_t shift_n = 0;

#ifdef CPU_ASSERT
        if((t == 15) || (m == 15))
        {
            printb(d_inst, "arm_ldrsh error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_ldrsh error 2");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_ldrsh error 3");
        }
#endif

        //ESO
        null_check_if_thumbee(n);

        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());

        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);


        bits data;
        if(!mem_u_read(&data, address, 2))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }

        bits _data;
        if(unaligned_support() || (get_bit(address, 0) == 0))
        {
            sign_extend(&_data, data, 32);
        }
        else
        {
            printb(d_inst, "arm_ldrsh error 4");
        }
        rf.r_write(t, _data.val);
    }
}

void armv7a::arm_strd_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_strd_imm");
    inst.check(27, 25, 0);
    inst.check(22, 1);
    inst.check(20, 0);
    inst.check(7, 4, B(1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm4H = inst(11, 8);
        uint32_t imm4L = inst(3, 0);

        if(get_bit(rt, 0) == 1)
        {
            printb(d_inst, "arm_strd_imm error");
        }

        uint32_t t = rt;
        uint32_t t2 = t + 1;
        uint32_t n = rn;
        uint32_t imm32 = (imm4H << 4) | imm4L;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_strd_imm error2");
        }

        if(wback && ((n == 15) || (n == t) || (n == t2)))
        {
            printb(d_inst, "arm_strd_imm error3");
        }

        if(t2 == 15)
        {
            printb(d_inst, "arm_strd_imm error4");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits _rt(rf.r_read(t), 32);
        bits _rt2(rf.r_read(t2), 32);

        if(!mem_a_write(address, 4, _rt))
        {
            return;
        }

        if(!mem_a_write(address + 4, 4, _rt2))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrsh_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrsh_imm");
    inst.check(27, 25, 0);
    inst.check(22, 1);
    inst.check(20, 1);
    inst.check(7, 4, B(1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm4H = inst(11, 8);
        uint32_t imm4L = inst(3, 0);

        if(rn == B(1111))
        {
            printb(d_inst, "arm_ldrsh_imm LDRSH literal");
        }

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrsh_imm ldrsht");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = (imm4H << 4) | imm4L;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((t == 15) || (wback && (n == t)))
        {
            printb(d_inst, "arm_ldrsh_imm error");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 2))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }

        if(armv7a::unaligned_support() || (get_bit(address, 0) == 0))
        {
            rf.r_write(t, sign_extend32(data));
        }

        else
        {
            printb(d_inst, "arm_ldrsh_imm error 2");
        }
    }
}

void armv7a::arm_ldrsh_ltrl(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrsh_ltrl not implemented yet.");
}

