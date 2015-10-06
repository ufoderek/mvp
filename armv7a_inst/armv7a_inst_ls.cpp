#include <armv7a.h>

void armv7a::decode_ls(armv7a_ir& inst)
{
    uint32_t A = inst(25);
    uint32_t op1 = inst(24, 20);
    uint32_t rn = inst(19, 16);
    uint32_t B = inst(4);
    bool A0 = A == 0;
    bool A1 = A == 1;
    bool B0 = B == 0;
    bool B1 = B == 1;
    bool op1_xx0x0 = (op1 & B(00101)) == B(00000);
    bool op1_0x010 = (op1 & B(10111)) == B(00010);
    bool op1_xx0x1 = (op1 & B(00101)) == B(00001);
    bool op1_0x011 = (op1 & B(10111)) == B(00011);
    bool op1_xx1x0 = (op1 & B(00101)) == B(00100);
    bool op1_0x110 = (op1 & B(10111)) == B(00110);
    bool op1_xx1x1 = (op1 & B(00101)) == B(00101);
    bool op1_0x111 = (op1 & B(10111)) == B(00111);
    bool rn_1111 = rn == B(1111);

    if(A0 && op1_xx0x0 && !op1_0x010)
    {
        arm_str_imm(inst);
    }
    else if(A1 && op1_xx0x0 && !op1_0x010 && B0)
    {
        arm_str_reg(inst);
    }
    else if(A0 && op1_0x010)
    {
        arm_strt(inst);
    }
    else if(A1 && op1_0x010 && B0)
    {
        arm_strt(inst);
    }
    else if(A0 && op1_xx0x1 && !op1_0x011 && !rn_1111)
    {
        arm_ldr_imm(inst);
    }
    else if(A0 && op1_xx0x1 && !op1_0x011 && rn_1111)
    {
        arm_ldr_ltrl(inst);
    }
    else if(A1 && op1_xx0x1 && !op1_0x011 && B0)
    {
        arm_ldr_reg(inst);
    }
    else if(A0 && op1_0x011)
    {
        arm_ldrt(inst);
    }
    else if(A1 && op1_0x011 && B0)
    {
        arm_ldrt(inst);
    }
    else if(A0 && op1_xx1x0 && !op1_0x110)
    {
        arm_strb_imm(inst);
    }
    else if(A1 && op1_xx1x0 && !op1_0x110 && B0)
    {
        arm_strb_reg(inst);
    }
    else if(A0 && op1_0x110)
    {
        arm_strbt(inst);
    }
    else if(A1 && op1_0x110 && B0)
    {
        arm_strbt(inst);
    }
    else if(A0 && op1_xx1x1 && !op1_0x111 && !rn_1111)
    {
        arm_ldrb_imm(inst);
    }
    else if(A0 && op1_xx1x1 && !op1_0x111 && rn_1111)
    {
        arm_ldrb_ltrl(inst);
    }
    else if(A1 && op1_xx1x1 && !op1_0x111 && B0)
    {
        arm_ldrb_reg(inst);
    }
    else if(A0 && op1_0x111)
    {
        arm_ldrbt(inst);
    }
    else if(A1 && op1_0x111 && B0)
    {
        arm_ldrbt(inst);
    }
    else
    {
        printb(core_id, d_armv7a_decode_ls, "decode error");
    }
}

void armv7a::arm_str_imm(armv7a_ir& inst)
{
    uint32_t p = inst(24);
    uint32_t u = inst(23);
    uint32_t w = inst(21);
    uint32_t rn = inst(19, 16);
    bits imm12(inst(11, 0), 12);

    if((rn == B(1101)) && (p == 1) && (u == 0) && (w == 1) && (imm12.val == B(100)))
    {
        arm_push(inst);
        return;
    }

    inst.print_inst("arm_str_imm");
    inst.check(27, 25, B(010));
    inst.check(22, 0);
    inst.check(20, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "str_imm strt");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        bits imm32;
        zero_extend(&imm32, imm12, 32);
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "str_imm error");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32.val) : (rf.r_read(n) - imm32.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data(0, 32);

        if(t == 15)
        {
            data.val = rf.pc_store_value();
        }

        else
        {
            data.val = rf.r_read(t);
        }

        if(!mem_u_write(address, 4, data))
        {
            return;
        }

        printd(d_inst, "t=%d, rt=%X", t, rf.r_read(t));

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_str_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_str_reg");
    inst.check(27, 25, B(011));
    inst.check(22, 0);
    inst.check(20, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_str_reg strt");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);

        if(m == 15)
        {
            printb(d_inst, "arm_str_reg error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_str_reg error 2");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_str_reg error 3");
        }

        //ESO
        null_check_if_thumbee(n);
        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(t == 15)
        {
            data.val = rf.pc_store_value();
        }

        else
        {
            data.val = rf.r_read(t);
        }

        if(armv7a::unaligned_support() || (get_field(address, 1, 0) == B(00)) || (rf.current_inst_set() == InstSet_ARM))
        {
            if(!mem_u_write(address, 4, data))
            {
                return;
            }
        }

        else
        {
            printb(d_inst, "arm_str_reg error 4");
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_strt(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_strt not implemented yet.");
}

void armv7a::arm_ldr_imm(armv7a_ir& inst)
{
    uint32_t p = inst(24);
    uint32_t u = inst(23);
    uint32_t w = inst(21);
    uint32_t rn = inst(19, 16);
    bits imm12(inst(11, 0), 12);

    if((rn == B(1101)) && (p == 0) && (u == 1) && (w == 0) && (imm12.val == B(100)))
    {
        arm_pop(inst);
        return;
    }

    inst.print_inst("arm_ldr_imm");
    inst.check(27, 25, B(010));
    inst.check(22, 0);
    inst.check(20, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        bits imm12(inst(11, 0), 12);

        if(rn == B(1111))
        {
            printb(d_inst, "ldr_imm LDR literal");
        }

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "ldr_imm ldrt");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = imm12.val;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if(wback && (n == t))
        {
            printb(d_inst, "ldr_imm error");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 4))
        {
            return;
        }

        printd(d_inst, "addr = %X, data = %X", address, data.val);

        if(wback)
        {
            rf.r_write(n, offset_addr);
            printd(d_inst, "n=%d rn=%X", n, rf.r_read(n));
        }

        if(t == 15)
        {
            if(get_field(address, 1, 0) == B(00))
            {
                rf.load_write_pc(data.val);
            }
            else
            {
                printb(d_inst, "ldr_imm error 2");
            }
        }
        else if(armv7a::unaligned_support() || (get_field(address, 1, 0) == B(00)))
        {
            rf.r_write(t, data.val);
            printd(d_inst, "t=%d rt=%X", n, rf.r_read(t));
        }
        else
        {
            bits _rt;
            ror(&_rt, data, 8 * get_field(address, 1, 0));
            rf.r_write(t, _rt.val);
            printd(d_inst, "t=%d rt=%X", n, rf.r_read(t));
        }
    }
}

void armv7a::arm_ldr_ltrl(armv7a_ir& inst)
{
    inst.print_inst("arm_ldr_ltrl");
    inst.check(27, 24, B(0101));
    inst.check(22, 16, B(001 1111));

    if(rf.condition_passed(inst(31, 28)))
    {
        uint32_t u = inst(23);
        uint32_t rt = inst(15, 12);
        uint32_t imm12 = inst(11, 0);
        uint32_t t = rt;
        uint32_t imm32 = imm12;
        bool add = u == 1;
        //ESO
        null_check_if_thumbee(15);
        uint32_t base = align(rf.pc(), 4);
        uint32_t address = add ? base + imm32 : base - imm32;
        bits data;

        if(!mem_u_read(&data, address, 4))
        {
            return;
        }

        printd(d_inst, "base=%X, addr=%X, t=%d, data=%X", base, address, t, data.val);

        if(t == 15)
        {
            if(get_field(address, 1, 0) == B(00))
            {
                rf.load_write_pc(data.val);
            }
            else
            {
                printb(d_inst, "arm_ldr_ltrl unpredictable");
            }
        }
        else if(armv7a::unaligned_support() || (get_field(address, 1, 0) == B(00)))
        {
            rf.r_write(t, data.val);
        }
        else
        {
            if(rf.current_inst_set() == InstSet_ARM)
            {
                bits result;
                ror(&result, data, 8 * get_field(address, 1, 0));
                rf.r_write(t, result.val);
            }
            else
            {
                printb(d_inst, "arm_ldr_ltrl unknown");
            }
        }
    }
}

void armv7a::arm_ldr_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_ldr_reg");
    inst.check(27, 25, B(011));
    inst.check(22, 0);
    inst.check(20, 1);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldr_reg LDRT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);

        if(m == 15)
        {
            printb(d_inst, "arm_ldr_reg error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_ldr_reg error 2");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_ldr_reg error 3");
        }

        //ESO
        null_check_if_thumbee(n);
        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 4))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }

        if(t == 15)
        {
            if(get_field(address, 1, 0) == B(00))
            {
                rf.load_write_pc(data.val);
            }

            else
            {
                printb(d_inst, "arm_ldr_reg error 4");
            }
        }
        else if(armv7a::unaligned_support() || (get_field(address, 1, 0) == B(00)))
        {
            rf.r_write(t, data.val);
        }
        else
        {
            if(rf.current_inst_set() == InstSet_ARM)
            {
                bits _rt;
                ror(&_rt, data, 8 * get_field(address, 1, 0));
                rf.r_write(t, _rt.val);
            }
            else
            {
                printb(d_inst, "arm_ldr_reg unknown 2");
            }
        }
    }
}

void armv7a::arm_ldrt(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrt not implemented yet.");
}

void armv7a::arm_strb_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_strb_imm");
    inst.check(27, 25, B(010));
    inst.check(22, 1);
    inst.check(20, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm12 = inst(11, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_strb_imm STRBT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = imm12;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if(t == 15)
        {
            printb(d_inst, "arm_strb_imm error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_strb_imm error 2");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits _rt(rf.r_read(t), 8);

        if(!mem_u_write(address, 1, _rt))
        {
            return;
        }

        printd(d_inst, "n=%d rn=%X imm32=%X", n, rf.r_read(n), imm32);

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_strb_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_strb_reg");
    inst.check(27, 25, B(011));
    inst.check(22, 1);
    inst.check(20, 0);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_strb_reg error");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);

        if((t == 15) || (m == 15))
        {
            printb(d_inst, "arm_strb_reg error 2");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_strb_reg error 3");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_strb_reg error 4");
        }

        //ESO
        null_check_if_thumbee(n);
        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits _rt(rf.r_read(t), 8);

        if(!mem_u_write(address, 1, _rt))
        {
            return;
        }

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_strbt(armv7a_ir& inst)
{
    inst.print_inst("arm_strbt");

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t t;
        uint32_t n;
        uint32_t m;
        bool post_index;
        bool add;
        bool register_form;
        bits imm32;
        sr_type shift_t;
        uint32_t shift_n;

        if(inst(25) == 0)
        {
            //Encoding A1
            inst.check(27, 24, B(0100));
            inst.check(22, 20, B(110));
            uint32_t u = inst(23);
            uint32_t rn = inst(19, 16);
            uint32_t rt = inst(15, 12);
            bits imm12(inst(11, 0), 12);
            t = rt;
            n = rn;
            post_index = true;
            add = u == 1;
            register_form = false;
            imm32.val = imm12.val;
            imm32.n = 32;

            if((t == 15) || (n == 15) || (n == t))
            {
                printb(d_inst, "arm_strbt error");
            }
        }
        else
        {
            //Encoding A2
            inst.check(27, 24, B(0110));
            inst.check(22, 20, B(110));
            inst.check(4, 0);
            uint32_t u = inst(23);
            uint32_t rn = inst(19, 16);
            uint32_t rt = inst(15, 12);
            uint32_t imm5 = inst(11, 7);
            uint32_t type = inst(6, 5);
            uint32_t rm = inst(3, 0);
            t = rt;
            n = rn;
            post_index = true;
            add = u == 1;
            register_form = true;
            decode_imm_shift(&shift_t, &shift_n, type, imm5);

            if((t == 15) || (n == 15) || (n == t) || (m == 15))
            {
                printb(d_inst, "arm_strbt error 2");
            }

            if((arch_version() < 6) && (m == n))
            {
                printb(d_inst, "arm_strbt error 3");
            }
        }

        //ESO
        null_check_if_thumbee(n);
        bits offset;

        if(register_form)
        {
            bits _rm(rf.r_read(m), 32);
            shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());
        }
        else
        {
            offset = imm32;
        }

        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = post_index ? rf.r_read(n) : offset_addr;
        bits _rt(rf.r_read(t), 8);

        if(!mem_u_unpriv_write(address, 1, _rt))
        {
            return;
        }

        if(post_index)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrb_imm(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrb_imm");
    inst.check(27, 25, B(010));
    inst.check(22, 1);
    inst.check(20, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm12 = inst(11, 0);

        if(rn == B(1111))
        {
            printb(d_inst, "arm_ldrb_imm LDRB literal");
        }

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrb_imm LDRBT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = imm12;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);

        if((t == 15) || (wback && (n == t)))
        {
            printb(d_inst, "arm_ldrb_imm error");
        }

        //ESO
        uint32_t offset_addr = add ? (rf.r_read(n) + imm32) : (rf.r_read(n) - imm32);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 1))
        {
            return;
        }

        rf.r_write(t, data.val);

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrb_ltrl(armv7a_ir& inst)
{
    printb(d_inst, "arm_ldrb_ltrl not imple. yet");
    inst.print_inst("arm_ldrb_ltrl");
    inst.check(27, 24, B(010 1));
    inst.check(22, 16, B(101 1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t u = inst(23);
        uint32_t rt = inst(15, 12);
        uint32_t imm12 = inst(11, 0);

        uint32_t t = rt;
        uint32_t imm32 = imm12;
        bool add = u == 1;

#ifdef CPU_ASSERT
        if(t == 15)
        {
            printb(d_inst, "arm_ldrb_ltrl error");
        }
#endif

        //ESO
        null_check_if_thumbee(15);

        uint32_t base = align(rf.pc(), 4);

        uint32_t address = add ? base + imm32 : base - imm32;

        bits data;
        mem_u_read(&data, address, 1);

        rf.r_write(t, data.val);
    }
}

void armv7a::arm_ldrb_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrb_reg");
    inst.check(27, 25, B(011));
    inst.check(22, 1);
    inst.check(20, 1);
    inst.check(4, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t imm5 = inst(11, 7);
        uint32_t type = inst(6, 5);
        uint32_t rm = inst(3, 0);

        if((p == 0) && (w == 1))
        {
            printb(d_inst, "arm_ldrb_reg LDRBT");
        }

        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t m = rm;
        bool index = p == 1;
        bool add = u == 1;
        bool wback = (p == 0) || (w == 1);
        sr_type shift_t;
        uint32_t shift_n;
        decode_imm_shift(&shift_t, &shift_n, type, imm5);

        if((t == 15) || (m == 15))
        {
            printb(d_inst, "arm_ldrb_reg error");
        }

        if(wback && ((n == 15) || (n == t)))
        {
            printb(d_inst, "arm_ldrb_reg error 2");
        }

        if((arch_version() < 6) && wback && (m == n))
        {
            printb(d_inst, "arm_ldrb_reg error 3");
        }

        //ESO
        null_check_if_thumbee(n);
        bits offset;
        bits _rm(rf.r_read(m), 32);
        shift(&offset, _rm, shift_t, shift_n, rf.cpsr_C());
        uint32_t offset_addr = add ? (rf.r_read(n) + offset.val) : (rf.r_read(n) - offset.val);
        uint32_t address = index ? offset_addr : rf.r_read(n);
        bits data;

        if(!mem_u_read(&data, address, 1))
        {
            return;
        }

        rf.r_write(t, data.val);

        if(wback)
        {
            rf.r_write(n, offset_addr);
        }
    }
}

void armv7a::arm_ldrbt(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrbt not implemented yet.");
}

