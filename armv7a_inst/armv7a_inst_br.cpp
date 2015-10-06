#include <armv7a.h>

void armv7a::decode_br(armv7a_ir& inst)
{
    uint32_t op = inst(25, 20);
    uint32_t rn = inst(19, 16);
    uint32_t r = inst(15);
    bool op_0000_x0 = (op & B(1111 01)) == B(0000 00);
    bool op_0000_x1 = (op & B(1111 01)) == B(0000 01);
    bool op_0010_x0 = (op & B(1111 01)) == B(0010 00);
    bool op_0010_x1 = (op & B(1111 01)) == B(0010 01);
    bool op_0010_01 = (op & B(1111 11)) == B(0010 01);
    bool op_0010_11 = (op & B(1111 11)) == B(0010 11);
    bool op_0100_00 = (op & B(1111 11)) == B(0100 00);
    bool op_0100_10 = (op & B(1111 11)) == B(0100 10);
    bool op_0100_x0 = (op & B(1111 01)) == B(0100 00);
    bool op_0100_x1 = (op & B(1111 01)) == B(0100 01);
    bool op_0110_x0 = (op & B(1111 01)) == B(0110 00);
    bool op_0110_x1 = (op & B(1111 01)) == B(0110 01);
    bool op_0xx1_x0 = (op & B(1001 01)) == B(0001 00);
    bool op_0xx1_x1 = (op & B(1001 01)) == B(0001 01);
    bool op_10xx_xx = (op & B(1100 00)) == B(1000 00);
    bool op_11xx_xx = (op & B(1100 00)) == B(1100 00);
    bool r_0 = r == 0;
    bool r_1 = r == 1;

    if(op_0000_x0)
    {
        arm_stm_da(inst);
    }

    else if(op_0000_x1)
    {
        arm_ldm_da(inst);
    }

    else if(op_0010_x0)
    {
        arm_stm_ia(inst);
    }

    else if(op_0010_01)
    {
        arm_ldm_ia(inst);
    }

    else if(op_0010_11)
    {
        if(rn != B(1101))
        {
            arm_ldm_ia(inst);
        }

        else
        {
            arm_pop(inst);
        }
    }

    else if(op_0100_00)
    {
        arm_stm_db(inst);
    }

    else if(op_0100_10)
    {
        if(rn != B(1101))
        {
            arm_stm_db(inst);
        }

        else
        {
            arm_push(inst);
        }
    }

    else if(op_0100_x1)
    {
        arm_ldm_db(inst);
    }

    else if(op_0110_x0)
    {
        arm_stm_ib(inst);
    }

    else if(op_0110_x1)
    {
        arm_ldm_ib(inst);
    }

    else if(op_0xx1_x0)
    {
        arm_stm_usr(inst);
    }

    else if(op_0xx1_x1 && r_0)
    {
        arm_ldm_usr(inst);
    }

    else if(op_0xx1_x1 && r_1)
    {
        arm_ldm_except(inst);
    }

    else if(op_10xx_xx)
    {
        arm_b(inst);
    }

    else if(op_11xx_xx)
    {
        arm_bl(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_br, "decode error: %X", inst.val);
    }
}

void armv7a::arm_stm_da(armv7a_ir& inst)
{
    inst.print_inst("arm_stm_da");
    inst.check(27, 22, B(100 000));
    inst.check(20, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);
        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;

        if((n == 15) || (bit_count(registers) < 1))
        {
            printb(d_inst, "arm_stm_da error");
        }

        //ESO
        uint32_t address = rf.r_read(n) - 4 * bit_count(registers) + 4;

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if((i == n) && wback && (i != lowest_set_bit(registers)))
                {
                    printb(d_inst, "arm_stm_da error 2");
                }
                else
                {
                    bits data(rf.r_read(i), 32);
                    if(!mem_a_write(address, 4, data))
                    {
                        return;
                    }
                    address += 4;
                }
            }
        }

        if(registers(15) == 1)
        {
            bits data(rf.pc_store_value(), 32);

            if(!mem_a_write(address, 4, data))
            {
                return;
            }
        }

        if(wback)
        {
            rf.r_write(n, rf.r_read(n) - 4 * bit_count(registers));
        }
    }
}

void armv7a::arm_ldm_da(armv7a_ir& inst)
{
    inst.print_inst("arm_ldm_da");
    inst.check(27, 22, B(100 000));
    inst.check(20, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);
        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;

        if((n == 15) || (bit_count(registers) < 1))
        {
            printb(d_inst, "arm_ldm_da error");
        }

        if(wback && (registers(n) == 1) && (arch_version() >= 7))
        {
            printb(d_inst, "arm_ldm_da error 2");
        }

        //ESO
        uint32_t address = rf.r_read(n) - 4 * bit_count(registers) + 4;

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                bits data;

                if(!mem_a_read(&data, address, 4))
                {
                    return;
                }

                rf.r_write(i, data.val);
                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits data;

            if(!mem_a_read(&data, address, 4))
            {
                return;
            }

            rf.load_write_pc(data.val);
        }

        if(wback && (registers(n) == 0))
        {
            rf.r_write(n, rf.r_read(n) - 4 * bit_count(registers));
        }

        if(wback && (registers(n) == 1))
        {
            printb(d_inst, "arm_ldm_da error 3");
        }
    }
}

void armv7a::arm_stm_ia(armv7a_ir& inst)
{
    inst.print_inst("arm_stm_ia");
    inst.check(27, 22, B(10 0010));
    inst.check(20, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);
        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;

        if((n == 15) || (bit_count(registers) < 1))
        {
            printb(d_inst, "arm_stm_ia error");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n);

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if((i == n) && wback && (i != lowest_set_bit(registers)))
                {
                    printb(d_inst, "arm_stm_ia error 2");
                }

                else
                {
                    bits _ri(rf.r_read(i), 32);

                    if(!mem_a_write(address, 4, _ri))
                    {
                        return;
                    }
                }

                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits _pc(rf.pc_store_value(), 32);

            if(!mem_a_write(address, 4, _pc))
            {
                return;
            }
        }

        if(wback)
        {
            rf.r_write(n, rf.r_read(n) + 4 * bit_count(registers));
        }
    }
}

void armv7a::arm_ldm_ia(armv7a_ir& inst)
{
    inst.print_inst("arm_ldm_ia");
    inst.check(27, 22, B(10 0010));
    inst.check(20, B(1));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);

        if((w == 1) && (rn == B(1101)) && (bit_count(reg_list) >= 2))
        {
            printb(d_inst, "ldm_ia SEE POP");
        }

        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;

        if((n == 15) || ((bit_count(registers) < 1)))
        {
            printb(d_inst, "ldm ia error");
        }

        if(wback && (registers(n) == 1) && (arch_version() >= 7))
        {
            printb(d_inst, "ldm ia error 2");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n);

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                bits data;

                if(!mem_a_read(&data, address, 4))
                {
                    return;
                }

                rf.r_write(i, data.val);
                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits data;

            if(!mem_a_read(&data, address, 4))
            {
                return;
            }

            rf.load_write_pc(data.val);
        }

        if(wback && (registers(n) == 0))
        {
            rf.r_write(n, rf.r_read(n) + 4 * bit_count(registers));
        }

        if(wback && (registers(n) == 1))
        {
            printb(d_inst, "ldm_ia error 3");
        }
    }
}

void armv7a::arm_pop(armv7a_ir& inst)
{
    inst.print_inst("arm_pop");
    bits registers;
    bool unaligned_allowed;

    if(rf.condition_passed(inst.cond()))
    {
        if(inst(27) == 1)
        {
            //Encoding A1
            inst.check(27, 16, B(1000 1011 1101));
            bits reg_list(inst(15, 0), 16);

            if(bit_count(reg_list) < 2)
            {
                printb(d_inst, "arm_pop LDM/LDMIA");
            }

            registers = reg_list;
            unaligned_allowed = false;

            if((registers(13) == 1) && (arch_version() >= 7))
            {
                printb(d_inst, "arm_pop error");
            }
        }

        else
        {
            //Encoding A2
            inst.check(27, 16, B(0100 1001 1101));
            inst.check(11, 0, B(0100));
            uint32_t rt = inst(15, 12);
            uint32_t t = rt;
            registers.val = 0;
            registers.n = 16;
            set_bit(&(registers.val), t, 1);
            unaligned_allowed = true;

            if(t == 13)
            {
                printb(d_inst, "arm_pop error 2");
            }
        }

        //ESO
        null_check_if_thumbee(13);
        uint32_t address = rf.r_read(SP);
        bits data;

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if(unaligned_allowed)
                {
                    if(!mem_u_read(&data, address, 4))
                    {
                        return;
                    }

                    printd(d_inst, "write r%d = 0x%X (read from 0x%X)", i, data.val, address);
                    rf.r_write(i, data.val);
                }

                else
                {
                    if(!mem_a_read(&data, address, 4))
                    {
                        return;
                    }

                    printd(d_inst, "write r%d = 0x%X (read from 0x%X)", i, data.val, address);
                    rf.r_write(i, data.val);
                }

                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            if(unaligned_allowed)
            {
                if(!mem_u_read(&data, address, 4))
                {
                    return;
                }

                printd(d_inst, "write r15 = 0x%X (read from 0x%X)", data.val, address);
                rf.load_write_pc(data.val);
            }

            else
            {
                if(!mem_a_read(&data, address, 4))
                {
                    return;
                }

                printd(d_inst, "write r15 = 0x%X (read from 0x%X)", data.val, address);
                rf.load_write_pc(data.val);
            }
        }

        printd(d_inst, "inst=%X sp=0x%X reg_list=0x%X bitcount=%d", inst.val, rf.r_read(SP), registers.val, bit_count(registers));

        if(registers(13) == 0)
        {
            rf.r_write(SP, rf.r_read(SP) + 4 * bit_count(registers));
        }

        if(registers(13) == 1)
        {
            printb(d_inst, "arm_pop error 3");
        }
    }
}

void armv7a::arm_stm_db(armv7a_ir& inst)
{
    inst.print_inst("arm_stm_db");
    inst.check(27, 22, B(100 100));
    inst.check(20, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);

        if((w == 1) && (rn == B(1101)) && (bit_count(reg_list) >= 2))
        {
            printb(d_inst, "arm_stm_db PUSH");
        }

        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;

        if((n == 15) || (bit_count(registers) < 1))
        {
            printb(d_inst, "arm_stm_db error");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n) - 4 * bit_count(registers);

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if((i == n) && wback && (i != lowest_set_bit(registers)))
                {
                    printb(d_inst, "arm_stm_db error 2");
                }

                else
                {
                    bits _ri(rf.r_read(i), 32);

                    if(!mem_a_write(address, 4, _ri))
                    {
                        return;
                    }
                }

                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits _pc(rf.pc_store_value(), 32);

            if(!mem_a_write(address, 4, _pc))
            {
                return;
            }
        }

        if(wback)
        {
            rf.r_write(n, rf.r_read(n) - 4 * bit_count(registers));
        }
    }
}

void armv7a::arm_push(armv7a_ir& inst)
{
    inst.print_inst("arm_push");

    if(rf.condition_passed(inst.cond()))
    {
        bits registers;
        bool unaligned_allowed;

        if(inst(27) == 1)
        {
            //Encoding A1
            inst.check(27, 16, B(1001 0010 1101));
            bits reg_list(inst(15, 0), 16);

            if(bit_count(reg_list) < 2)
            {
                printb(d_inst, "arm_push STMDB");
            }

            registers = reg_list;
            unaligned_allowed = false;
        }

        else
        {
            //Encoding A2
            inst.check(27, 16, B(0101 0010 1101));
            inst.check(11, 0, B(0100));
            uint32_t rt = inst(15, 12);
            uint32_t t = rt;
            registers.val = 0;
            registers.n = 16;
            set_bit(&(registers.val), t, 1);
            unaligned_allowed = true;

            if(t == 13)
            {
                printb(d_inst, "arm_push error");
            }
        }

        //ESO()
        null_check_if_thumbee(13);
        uint32_t address = rf.r_read(SP) - 4 * bit_count(registers);

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if((i == 13) && (i != lowest_set_bit(registers)))
                {
                    printb(d_inst, "arm_push unknown");
                }
                else
                {
                    //printm(d_inst, "push r%d @ 0x%X", i, address);
                    if(unaligned_allowed)
                    {
                        bits _ri(rf.r_read(i), 32);

                        if(!mem_u_write(address, 4, _ri))
                        {
                            return;
                        }
                    }
                    else
                    {
                        bits _ri(rf.r_read(i), 32);

                        if(!mem_a_write(address, 4, _ri))
                        {
                            return;
                        }
                    }

                    address += 4;
                }
            }
        }

        if(registers(15) == 1)
        {
            if(unaligned_allowed)
            {
                bits _pc(rf.pc_store_value(), 32);

                if(!mem_u_write(address, 4, _pc))
                {
                    return;
                }
            }

            else
            {
                bits _pc(rf.pc_store_value(), 32);

                if(!mem_a_write(address, 4, _pc))
                {
                    return;
                }
            }
        }

        rf.r_write(SP, rf.r_read(SP) - 4 * bit_count(registers));
    }
}

void armv7a::arm_ldm_db(armv7a_ir& inst)
{
    inst.print_inst("arm_ldm_db");
    inst.check(27, 22, B(100 100));
    inst.check(20, B(1));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);
        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;

        if((n == 15) || ((bit_count(registers) < 1)))
        {
            printb(d_inst, "ldm db error");
        }

        if(wback && (registers(n) == 1) && (arch_version() >= 7))
        {
            printb(d_inst, "ldm db error 2");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n) - 4 * bit_count(registers);

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                bits data;

                if(!mem_a_read(&data, address, 4))
                {
                    return;
                }

                rf.r_write(i, data.val);
                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits data;

            if(!mem_a_read(&data, address, 4))
            {
                return;
            }

            rf.load_write_pc(data.val);
        }

        if(wback && (registers(n) == 0))
        {
            rf.r_write(n, rf.r_read(n) - 4 * bit_count(registers));
        }

        if(wback && (registers(n) == 1))
        {
            printb(d_inst, "ldm_db error 3");
        }
    }
}

void armv7a::arm_stm_ib(armv7a_ir& inst)
{
    inst.print_inst("arm_stm_ib");
    inst.check(27, 22, B(100 110));
    inst.check(20, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        uint16_t reg_list = inst(15, 0);
        uint32_t n = rn;
        bits registers(reg_list, 16);
        bool wback = w == 1;

        if((n == 15) || (bit_count(registers) < 1))
        {
            printb(d_inst, "arm_stm_ib error");
        }

        //ESO
        uint32_t address = rf.r_read(n) + 4;

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if((i == n) && wback && (i != lowest_set_bit(registers)))
                {
                    printb(d_inst, "arm_stm_ib error 2");
                }

                else
                {
                    bits _ri(rf.r_read(i), 32);

                    if(!mem_a_write(address, 4, _ri))
                    {
                        return;
                    }
                }

                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits _pc(rf.pc_store_value(), 32);

            if(!mem_a_write(address, 4, _pc))
            {
                return;
            }
        }

        if(wback)
        {
            rf.r_write(n, rf.r_read(n) + 4 * bit_count(registers));
        }
    }
}

void armv7a::arm_ldm_ib(armv7a_ir& inst)
{
    inst.print_inst("arm_ldm_ib");
    inst.check(27, 22, B(100 110));
    inst.check(20, B(1));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);
        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;

        if((n == 15) || ((bit_count(registers) < 1)))
        {
            printb(d_inst, "ldm ib error");
        }

        if(wback && (registers(n) == 1) && (arch_version() >= 7))
        {
            printb(d_inst, "ldm ib error 2");
        }

        //ESO
        uint32_t address = rf.r_read(n) + 4;
        bits data;

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if(!mem_a_read(&data, address, 4))
                {
                    return;
                }

                rf.r_write(i, data.val);
                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits data;

            if(!mem_a_read(&data, address, 4))
            {
                return;
            }

            rf.load_write_pc(data.val);
        }

        if(wback && (registers(n) == 0))
        {
            rf.r_write(n, rf.r_read(n) + 4 * bit_count(registers));
        }

        if(wback && (registers(n) == 1))
        {
            printb(d_inst, "ldm_ib error 3");
        }
    }
}

void armv7a::arm_stm_usr(armv7a_ir& inst)
{
    inst.print_inst("arm_stm_usr");
    inst.check(27, 25, B(100));
    inst.check(22, 20, B(100));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(15, 0), 16);
        uint32_t n = rn;
        bits registers = reg_list;
        bool increment = u == 1;
        bool wordhigher = p == u;

        if((n == 15) || (bit_count(registers) < 1))
        {
            printb(d_inst, "arm_stm_usr error");
        }

        //ESO

        if(rf.current_mode_is_user_or_system())
        {
            printb(d_inst, "arm_stm_usr error 2");
        }

        uint32_t length = 4 * bit_count(registers);
        uint32_t address = increment ? rf.r_read(n) : (rf.r_read(n) - length);

        if(wordhigher)
        {
            address += 4;
        }

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                bits data(rf.r_mode_read(i, B(10000)), 32);
                mem_a_write(address, 4, data);
                address += 4;
            }
        }

        if(registers(15) == 1)
        {
            bits _pc(rf.pc_store_value(), 32);
            mem_a_write(address, 4, _pc);
        }
    }
}

void armv7a::arm_ldm_usr(armv7a_ir& inst)
{
    inst.print_inst("arm_ldm_usr");
    inst.check(27, 25, B(100));
    inst.check(22, 1);
    inst.check(22, 20, B(101));
    inst.check(15, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(14, 0), 15);
        uint32_t n = rn;
        bits registers = reg_list;
        bool increment = u == 1;
        bool wordhigher = p == u;

        if((n == 15) || (bit_count(registers) < 1))
        {
            printb(d_inst, "arm_ldm_usr error");
        }

        //ESO

        if(rf.current_mode_is_user_or_system())
        {
            printb(d_inst, "arm_ldm_usr error 2");
        }

        uint32_t length = 4 * bit_count(registers);
        uint32_t address = increment ? rf.r_read(n) : rf.r_read(n) - length;

        if(wordhigher)
        {
            address += 4;
        }

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                bits data;

                if(!mem_a_read(&data, address, 4))
                {
                    return;
                }

                rf.r_mode_write(i, B(10000), data.val);
                printd(d_inst, "write r%d=0x%X, addr=0x%X", i, data.val, address);
                address += 4;
            }
        }
    }
}

void armv7a::arm_ldm_except(armv7a_ir& inst)
{
    inst.print_inst("arm_ldm_except");
    inst.check(27, 25, B(100));
    inst.check(22, 1);
    inst.check(20, 1);
    inst.check(15, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t p = inst(24);
        uint32_t u = inst(23);
        uint32_t w = inst(21);
        uint32_t rn = inst(19, 16);
        bits reg_list(inst(14, 0), 15);
        uint32_t n = rn;
        bits registers = reg_list;
        bool wback = w == 1;
        bool increment = u == 1;
        bool wordhigher = p == u;

        if(n == 15)
        {
            printb(d_inst, "arm_ldm_except error");
        }

        if(wback && (registers(n) == 1) && (arch_version() >= 7))
        {
            printb(d_inst, "arm_ldm_except error 2");
        }

        //ESO

        if(rf.current_mode_is_user_or_system())
        {
            printb(d_inst, "arm_ldm_except error 3");
        }

        uint32_t length = 4 * bit_count(registers) + 4;
        uint32_t address = increment ? rf.r_read(n) : rf.r_read(n) - length;

        if(wordhigher)
        {
            address += 4;
        }

        bits data;

        for(int i = 0; i <= 14; i++)
        {
            if(registers(i) == 1)
            {
                if(!mem_a_read(&data, address, 4))
                {
                    return;
                }

                rf.r_write(i, data.val);
                address += 4;
            }
        }

        if(!mem_a_read(&data, address, 4))
        {
            return;
        }

        uint32_t new_pc_value = data.val;

        if(wback && (registers(n) == 0))
        {
            rf.r_write(n, increment ? (rf.r_read(n) + length) : (rf.r_read(n) - length));
        }

        if(wback && (registers(n) == 1))
        {
            printb(d_inst, "arm_ldm_except error 4");
        }

        cpsr_write_by_inst(rf.spsr(), B(1111), true);
        rf.branch_write_pc(new_pc_value);
        printd(d_inst, "ldm_except return from exception to PC:%X", new_pc_value);
    }
}

void armv7a::arm_b(armv7a_ir& inst)
{
    inst.print_inst("arm_b");
    inst.check(27, 24, B(1010));

    if(rf.condition_passed(inst.cond()))
    {
        //Encoding A1
        bits imm24(inst(23, 0), 24);
        bits imm32;
        imm24.val <<= 2;
        imm24.n += 2;
        sign_extend(&imm32, imm24, 32);
        printd(d_inst, "branch to %X", rf.pc() + imm32.val);
        rf.branch_write_pc(rf.pc() + imm32.val);
    }
}

void armv7a::arm_bl(armv7a_ir& inst)
{
    inst.print_inst("arm_bl");
    bits imm32;
    inst_set target_inst_set;

    if(rf.condition_passed(inst.cond()))
    {
        if(inst.cond() != B(1111))
        {
            //Encoding A1
            inst.check(27, 24, B(1011));
            bits imm24(inst(23, 0), 24);
            imm24.val <<= 2;
            imm24.n += 2;
            sign_extend(&imm32, imm24, 32);
            target_inst_set = InstSet_ARM;
        }

        else
        {
            //Encoding A2
            inst.check(31, 25, B(1111 101));
            printb(d_inst, "arm_bl thumb error");
            uint32_t h = inst(24);
            bits imm24(inst(23, 0), 24);
            imm24.val = (imm24.val << 2) | (h << 1);
            imm24.n += 2;
            sign_extend(&imm32, imm24, 32);
            target_inst_set = InstSet_Thumb;
        }

        //ESO

        if(rf.current_inst_set() == InstSet_ARM)
        {
            rf.r_write(LR, rf.pc() - 4);
        }

        else
        {
            printb(d_inst, "arm_blx error");
        }

        uint32_t target_address;

        if(target_inst_set == InstSet_ARM)
        {
            target_address = align(rf.pc(), 4) + imm32.val;
        }

        else
        {
            printb(d_inst, "arm_blx error 2");
        }

        //select_inst_set(target_inst_set);
        rf.branch_write_pc(target_address);
    }
}

