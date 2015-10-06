#include <armv7a.h>

void armv7a::decode_misc(armv7a_ir& inst)
{
    uint32_t op = inst(22, 21);
    uint32_t op1 = inst(19, 16);
    uint32_t op2 = inst(6, 4);
    bool op2_000 = op2 == B(000);
    bool op2_001 = op2 == B(001);
    bool op2_010 = op2 == B(010);
    bool op2_011 = op2 == B(011);
    bool op2_101 = op2 == B(101);
    bool op2_110 = op2 == B(110);
    bool op2_111 = op2 == B(111);
    bool op_x0 = (op & B(01)) == B(00);
    bool op_x1 = (op & B(01)) == B(01);
    bool op_01 = op == B(01);
    bool op_10 = op == B(10);
    bool op_11 = op == B(11);
    bool op1_xx00 = (op1 & B(0011)) == B(0000);
    bool op1_xx01 = (op1 & B(0011)) == B(0001);
    bool op1_xx1x = (op1 & B(0010)) == B(0010);
    bool B0 = inst(9) == 0;
    bool B1 = inst(9) == 1;

    if(op2_000 && B1 && op_x0)
    {
        arm_mrs_bk(inst);
    }
    else if(op2_000 && B1 && op_x1)
    {
        arm_msr_bk(inst);
    }
    else if(op2_000 && B0 && op_x0)
    {
        arm_mrs(inst);
    }
    else if(op2_000 &&  B0 && op_01 && op1_xx00)
    {
        arm_msr_reg_ap(inst);
    }
    else if(op2_000 &&  B0 && op_01 && op1_xx01)
    {
        arm_msr_reg_sys(inst);
    }
    else if(op2_000 &&  B0 && op_01 && op1_xx1x)
    {
        arm_msr_reg_sys(inst);
    }
    else if(op2_000 && B0 && op_11)
    {
        arm_msr_reg_sys(inst);
    }
    else if(op2_001 && op_01)
    {
        arm_bx(inst);
    }
    else if(op2_001 && op_11)
    {
        arm_clz(inst);
    }
    else if(op2_010 && op_01)
    {
        arm_bxj(inst);
    }
    else if(op2_011 && op_01)
    {
        arm_blx_reg(inst);
    }
    else if(op2_101)
    {
        decode_sat(inst);
    }
    else if(op2_110 && op_11)
    {
        arm_eret(inst);
    }
    else if(op2_111 && op_01)
    {
        arm_bkpt(inst);
    }
    else if(op2_111 && op_10)
    {
        arm_hvc(inst);
    }
    else if(op2_111 && op_11)
    {
        arm_smc(inst);
    }
    else
    {
        printb(core_id, d_armv7a_decode_misc, "decode error");
    }
}

void armv7a::arm_mrs_bk(armv7a_ir& inst)
{
    inst.print_inst("arm_mrs_bk");
    inst.check(27, 23, B(00010));
    inst.check(21, 20, B(00));
    inst.check(11, 9, B(001));
    inst.check(7, 0, 0);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t r = inst(22);
        uint32_t m1 = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t m = inst(8);

        uint32_t d = rd;
        bool read_spsr = r == 1;

#ifdef CPU_ASSERT
        if(d == 15)
        {
            printb(d_inst, "arm_mrs_bk error");
        }
#endif

        uint32_t sysm = (m << 4) | m1;

        //ESO
#ifdef CPU_ASSERT
        if(!rf.current_mode_is_not_user())
        {
            printb(d_inst, "arm_mrs_bk error 2");
        }
#endif

        uint32_t mode = rf.cpsr_M();
        if(read_spsr)
        {
            spsr_access_valid(sysm, mode);
            switch(sysm)
            {
                case B(01110):
                    rf.r_write(d, rf.spsr_bank(fiq));
                    printd(d_inst, "arm_mrs_bk R%d = spsr_fiq = 0x%X", d, rf.spsr_bank(fiq));
                    break;
                case B(10000):
                    rf.r_write(d, rf.spsr_bank(irq));
                    printd(d_inst, "arm_mrs_bk R%d = spsr_irq = 0x%X", d, rf.spsr_bank(irq));
                    break;
                case B(10010):
                    rf.r_write(d, rf.spsr_bank(svc));
                    printd(d_inst, "arm_mrs_bk R%d = spsr_svc = 0x%X", d, rf.spsr_bank(svc));
                    break;
                case B(10100):
                    rf.r_write(d, rf.spsr_bank(abt));
                    printd(d_inst, "arm_mrs_bk R%d = spsr_abt = 0x%X", d, rf.spsr_bank(abt));
                    break;
                case B(10110):
                    rf.r_write(d, rf.spsr_bank(und));
                    printd(d_inst, "arm_mrs_bk R%d = spsr_und = 0x%X", d, rf.spsr_bank(und));
                    break;
                case B(11100):
                    rf.r_write(d, rf.spsr_bank(mon));
                    printd(d_inst, "arm_mrs_bk R%d = spsr_mon = 0x%X", d, rf.spsr_bank(mon));
                    break;
                case B(11110):
                    rf.r_write(d, rf.spsr_bank(hyp));
                    printd(d_inst, "arm_mrs_bk R%d = spsr_hyp = 0x%X", d, rf.spsr_bank(hyp));
                    break;
                default:
                    printb(d_inst, "arm_mrs_bk error 3");
                    break;
            }
        }
        else
        {
            banked_register_access_valid(sysm, mode);

            if(get_field(sysm, 4, 3) == B(00))
            {
                uint32_t m = get_field(sysm, 2, 0) + 8;
                rf.r_write(d, rf.r_mode_read(m, B(10000)));
                printd(d_inst, "arm_mrs_bk R%d = R%d_usr = 0x%X", d, m, rf.r_read(d));
            }
            else if(get_field(sysm, 4, 3) == B(01))
            {
                uint32_t m = get_field(sysm, 2, 0) + 8;
                rf.r_write(d, rf.r_mode_read(m, B(10001)));
                printd(d_inst, "arm_mrs_bk R%d = R%d_fiq = 0x%X", d, m, rf.r_read(d));
            }
            else if(get_field(sysm, 4, 3) == B(11))
            {
                if(get_bit(sysm, 1) == 0)
                {
                    uint32_t m = get_bit(sysm, 0) + 13;
                    rf.r_write(d, rf.r_mode_read(m, B(10110)));
                    printd(d_inst, "arm_mrs_bk R%d = R%d_mon = 0x%X", d, m, rf.r_read(d));
                }
                else
                {
                    if(get_bit(sysm, 0) == 1) //ARM Ref Manual B9-1961, i think this is a bug
                    {
                        rf.r_write(d, rf.r_mode_read(13, B(11010)));
                        printd(d_inst, "arm_mrs_bk R%d = R%d_inst = 0x%X", d, m, rf.r_read(d));
                    }
                    else
                    {
                        rf.r_write(d, rf.elr_hyp());
                        printd(d_inst, "arm_mrs_bk R%d = ELR_HYP = 0x%X", d, rf.elr_hyp());
                    }
                }
            }
            else
            {
                uint32_t target_mode = 0;
                set_bit(&target_mode, 0, get_bit(sysm, 2) | get_bit(sysm, 1));
                set_bit(&target_mode, 1, 1);
                set_bit(&target_mode, 2, get_bit(sysm, 2) & ((~get_bit(sysm, 1))&mask(0)));
                set_bit(&target_mode, 3, get_bit(sysm, 2) & get_bit(sysm, 1));
                set_bit(&target_mode, 4, 1);

                if(mode == target_mode)
                {
                    printb(d_inst, "arm_mrs_bk error 4");
                }
                else
                {
                    m = get_bit(sysm, 0) + 13;
                    rf.r_write(d, rf.r_mode_read(m, target_mode));
                    printd(d_inst, "arm_mrs_bk read R%d_(%d) = 0x%X, sysm=0x%X", m, target_mode, rf.r_read(d), sysm);

                    printm_once(d_inst, "\"mrs Rd,SP_svc\" actually do \"mrs Rd,LR_svc\" and vice versa, please check future version of ARM Ref Manual");
                }
            }
        }
    }
}

void armv7a::arm_msr_bk(armv7a_ir& inst)
{
    inst.print_inst("arm_msr_bk");
    inst.check(27, 23, B(00010));
    inst.check(21, 20, B(10));
    inst.check(15, 9, B(1111 00 1));
    inst.check(7, 4, B(0000));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t r = inst(22);
        uint32_t m1 = inst(19, 16);
        uint32_t m = inst(8);
        uint32_t rn = inst(3, 0);

        uint32_t n = rn;
        bool write_spsr = r == 1;

#ifdef CPU_ASSERT
        if((n == 13) || (n == 15))
        {
            printb(d_inst, "arm_msr_bk error");
        }
#endif

        uint32_t sysm = (m << 4) | m1;

        //ESO

#ifdef CPU_ASSERT
        if(!rf.current_mode_is_not_user())
        {
            printb(d_inst, "arm_msr_bk error 2");
        }
#endif

        uint32_t mode = rf.cpsr_M();
        if(write_spsr)
        {
            spsr_access_valid(sysm, mode);
            switch(sysm)
            {
                case B(01110):
                    rf.spsr_bank(fiq, rf.r_read(n));
                    printd(d_inst, "arm_msr_bk spsr_fiq = R%d = 0x%X", n, rf.r_read(n));
                    break;
                case B(10000):
                    rf.spsr_bank(irq, rf.r_read(n));
                    printd(d_inst, "arm_msr_bk spsr_irq = R%d = 0x%X", n, rf.r_read(n));
                    break;
                case B(10010):
                    rf.spsr_bank(svc, rf.r_read(n));
                    printd(d_inst, "arm_msr_bk spsr_svc = R%d = 0x%X", n, rf.r_read(n));
                    break;
                case B(10100):
                    rf.spsr_bank(abt, rf.r_read(n));
                    printd(d_inst, "arm_msr_bk spsr_abt = R%d = 0x%X", n, rf.r_read(n));
                    break;
                case B(10110):
                    rf.spsr_bank(und, rf.r_read(n));
                    printd(d_inst, "arm_msr_bk spsr_und = R%d = 0x%X", n, rf.r_read(n));
                    break;
                case B(11100):
                    rf.spsr_bank(mon, rf.r_read(n));
                    printd(d_inst, "arm_msr_bk spsr_mon = R%d = 0x%X", n, rf.r_read(n));
                    break;
                case B(11110):
                    rf.spsr_bank(hyp, rf.r_read(n));
                    printd(d_inst, "arm_msr_bk spsr_hyp = R%d = 0x%X", n, rf.r_read(n));
                    break;
                default:
                    printb(d_inst, "arm_msr_bk error 3");
                    break;
            }
        }
        else
        {
            banked_register_access_valid(sysm, mode);

            if(get_field(sysm, 4, 3) == B(00))
            {
                uint32_t m = get_field(sysm, 2, 0) + 8;
                rf.r_mode_write(m, B(10000), rf.r_read(n));
                printd(d_inst, "arm_msr_bk R%d_usr = R%d = 0x%X", m, n, rf.r_read(n));
            }
            else if(get_field(sysm, 4, 3) == B(01))
            {
                uint32_t m = get_field(sysm, 2, 0) + 8;
                rf.r_mode_write(m, B(10001), rf.r_read(n));
                printd(d_inst, "arm_msr_bk R%d_fiq = R%d = 0x%X", m, n, rf.r_read(n));
            }
            else if(get_field(sysm, 4, 3) == B(11))
            {
                if(get_bit(sysm, 1) == 0)
                {
                    uint32_t m = get_bit(sysm, 0) + 13;
                    rf.r_mode_write(m, B(10110), rf.r_read(n));
                    printd(d_inst, "inst=0x%X sysm=0x%X", inst.val, sysm);
                    printd(d_inst, "arm_msr_bk R%d_mon = R%d = 0x%X", m, n, rf.r_read(n));
                }
                else
                {
                    if(get_bit(sysm, 0) == 1) //ARM Ref Manual B9-1965, i think this is a bug
                    {
                        rf.r_mode_write(13, B(10110), rf.r_read(n));
                        printd(d_inst, "arm_msr_bk R13_hyp = 0x%X", rf.r_read(n));
                    }
                    else
                    {
                        rf.elr_hyp(rf.r_read(n));
                        printd(d_inst, "arm_msr_bk ELR_HYP = 0x%X", rf.elr_hyp());
                    }
                }
            }
            else
            {
                uint32_t target_mode = 0;
                set_bit(&target_mode, 0, get_bit(sysm, 2) | get_bit(sysm, 1));
                set_bit(&target_mode, 1, 1);
                set_bit(&target_mode, 2, get_bit(sysm, 2) & ((~get_bit(sysm, 1))&mask(0)));
                set_bit(&target_mode, 3, get_bit(sysm, 2) & get_bit(sysm, 1));
                set_bit(&target_mode, 4, 1);
                if(mode == target_mode)
                {
                    printb(d_inst, "arm_msr_bk error 4");
                }
                else
                {
                    uint32_t m = get_bit(sysm, 0) + 13;
                    rf.r_mode_write(m, target_mode, rf.r_read(n));

                    printd(d_inst, "arm_msr_bk write R%d_(%d) = 0x%X, sysm=0x%X", m, target_mode, rf.r_read(n), sysm);

                    printm_once(d_inst, "\"msr SP_svc,Rn\" actually do \"msr LR_svc,Rn\" and vice versa, please check future version of ARM Ref Manual");
                }
            }
        }
    }
}

void armv7a::arm_mrs(armv7a_ir& inst)
{
    if(rf.current_mode_is_not_user())
    {
        //MRS (system instruction, ARM Ref B6-10)
        inst.print_inst("arm_mrs");
        inst.check(27, 23, B(00010));
        inst.check(21, 16, B(00 1111));
        inst.check(11, 0, 0);

        if(rf.condition_passed(inst.cond()))
        {
            uint32_t r = inst(22);
            uint32_t rd = inst(15, 12);
            uint32_t d = rd;
            bool read_spsr = r == 1;

            if(d == 15)
            {
                printb(d_inst, "mrs unpredictable");
            }

            //ESO

            if(read_spsr)
            {
                if(rf.current_mode_is_user_or_system())
                {
                    printb(d_inst, "arm_mrs error");
                }

                else
                {
                    rf.r_write(d, rf.spsr());
                }
            }
            else
            {
                rf.r_write(d, rf.cpsr()&B(1111 1000 1111 1111 0000 0011 1101 1111));
            }
        }
    }

    else
    {
        printb(d_inst, "mrs user mode");
        //MRS (ARM Ref A8-206)
        inst.print_inst("arm_mrs");
        inst.check(27, 16, B(0001 0000 1111));
        inst.check(11, 0, 0);

        if(rf.condition_passed(inst.cond()))
        {
            uint32_t rd = inst(15, 12);
            uint32_t d = rd;

            if(d == 15)
            {
                printb(d_inst, "mrs unpredictable");
            }

            //ESO
            rf.r_write(d, rf.apsr());
        }
    }
}

void armv7a::arm_msr_reg_ap(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_msr_reg_ap not implemented yet.");
}

void armv7a::arm_msr_reg_sys(armv7a_ir& inst)
{
    inst.print_inst("arm_msr_reg_sys");
    inst.check(27, 23, B(10));
    inst.check(21, 20, B(10));
    inst.check(15, 4, B(1111 0000 0000));

    if(rf.condition_passed(inst.cond()))
    {
        //Encoding A1
        uint32_t r = inst(22);
        uint32_t mask = inst(19, 16);
        uint32_t rn = inst(3, 0);
        uint32_t n = rn;
        bool write_spsr = r == 1;

        if(mask == B(0000))
        {
            printb(d_inst, "arm_msr_reg_sys unpredictable");
        }

        if(n == 15)
        {
            printb(d_inst, "arm_msr_reg_sys unpredictable n==15");
        }

        //ESO

        if(write_spsr)
        {
            spsr_write_by_inst(rf.r_read(n), mask);
        }
        else
        {
            cpsr_write_by_inst(rf.r_read(n), mask, false);
        }
    }
}

void armv7a::arm_bx(armv7a_ir& inst)
{
    inst.print_inst("arm_bx");
    inst.check(27, 4, B(0001 0010 1111 1111 1111 0001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rm = inst(3, 0);
        uint32_t m = rm;
        //ESO
        rf.bx_write_pc(rf.r_read(m));
    }
}

void armv7a::arm_clz(armv7a_ir& inst)
{
    inst.print_inst("arm_clz");
    inst.check(27, 16, B(0001 0110 1111));
    inst.check(11, 4, B(1111 0001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rd = inst(15, 12);
        uint32_t rm = inst(3, 0);
        uint32_t d = rd;
        uint32_t m = rm;

        if((d == 15) || (m == 15))
        {
            printb(d_inst, "arm_clz error");
        }

        //ESO
        bits _rm(rf.r_read(m), 32);
        uint32_t result = count_leading_zero_bits(_rm);
        printd(d_inst, "clz %X %d", rf.r_read(m), result);
        rf.r_write(d, result);
    }
}

void armv7a::arm_bxj(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_bxj not implemented yet.");
}

void armv7a::arm_blx_reg(armv7a_ir& inst)
{
    inst.print_inst("arm_blx_reg");
    inst.check(27, 4, B(0001 0010 1111 1111 1111 0011));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rm = inst(3, 0);
        uint32_t m = rm;

        if(m == 15)
        {
            printb(d_inst, "arm_blx_reg error");
        }

        //ESO
        uint32_t target = rf.r_read(m);

        if(rf.current_inst_set() == InstSet_ARM)
        {
            uint32_t next_inst_addr = rf.pc() - 4;
            rf.r_write(LR, next_inst_addr);
        }
        else
        {
            printb(d_inst, "arm_blx_reg error 2");
            uint32_t next_inst_addr = rf.pc() - 2;
            set_bit(&next_inst_addr, 0, 1);
            rf.r_write(LR, next_inst_addr);
        }

        rf.bx_write_pc(target);
    }
}

void armv7a::arm_eret(armv7a_ir& inst)
{
    inst.print_inst("arm_eret");
    inst.check(27, 16, B(0001 0110 0000));
    inst.check(15, 0, B(0000 0000 0110 1110));

    if(rf.condition_passed(inst.cond()))
    {
        //ESO
#ifdef CPU_ASSERT
        if(rf.current_mode_is_user_or_system() || (rf.current_inst_set() == InstSet_ThumbEE))
        {
            printb(d_inst, "arm_eret error");
        }
#endif
        uint32_t new_pc_value = rf.current_mode_is_hyp() ? rf.elr_hyp() : rf.r_read(14);
        cpsr_write_by_inst(rf.spsr(), B(1111), true);

#ifdef CPU_ASSERT
        if((rf.cpsr_M() == B(11010)) && (rf.cpsr_J() == 1) && (rf.cpsr_T() == 1))
        {
            printb(d_inst, "arm_eret error 2");
        }
#endif

        rf.branch_write_pc(new_pc_value);
    }

    inst.vm_id(cp15.read64(VTTBR_LPAE_VMID));
    inst.hyp_mode(false);
}

void armv7a::arm_bkpt(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_bkpt not implemented yet.");
}

void armv7a::arm_hvc(armv7a_ir& inst)
{
    inst.print_inst("arm_hvc");
    inst.check(27, 20, B(0001 0100));
    inst.check(5, 4, B(0111));

    if(inst.cond() != B(1110))
    {
        printb(d_inst, "arm_hvc error");
    }

    uint32_t imm12 = inst(19, 8);
    uint32_t imm4 = inst(3, 0);
    uint32_t imm16 = (imm12 << 4) | imm4;

    //ESO
    printm_once(d_inst, "simplified implementation of arm_hvc");

    call_hypervisor(imm16);
}

void armv7a::arm_smc(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smc not implemented yet.");
}


