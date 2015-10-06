#include <armv7a.h>

void armv7a::decode_svccp(armv7a_ir& inst)
{
    uint32_t op1 = inst(25, 20);
    uint32_t rn = inst(19, 16);
    uint32_t coproc = inst(11, 8);
    uint32_t op = inst(4);
    bool op1_0x_xxxx = (op1 & B(10 0000)) == B(00 0000);
    bool op1_0x_xxx0 = (op1 & B(10 0001)) == B(00 0000);
    bool op1_0x_xxx1 = (op1 & B(10 0001)) == B(00 0001);
    bool op1_00_000x = (op1 & B(11 1110)) == B(00 0000);
    bool op1_00_010x = (op1 & B(11 1110)) == B(00 0100);
    bool op1_00_0100 = (op1 & B(11 1111)) == B(00 0100);
    bool op1_00_0101 = (op1 & B(11 1111)) == B(00 0101);
    bool op1_10_xxxx = (op1 & B(11 0000)) == B(10 0000);
    bool op1_10_xxx0 = (op1 & B(11 0001)) == B(10 0000);
    bool op1_10_xxx1 = (op1 & B(11 0001)) == B(10 0001);
    bool op1_11_xxxx = (op1 & B(11 0000)) == B(11 0000);
    bool op_0 = op == 0;
    bool op_1 = op == 1;
    bool cp_101x = (coproc & B(1110)) == B(1010);
    bool cp_not_101x = !cp_101x;
    bool rn_1111 = rn == B(1111);
    bool rn_not_1111 = !rn_1111;
    bool op1_00_0x0x = (op1 & B(11 1010)) == B(00 0000);
    bool a = !op1_00_0x0x;

    if(op1_0x_xxxx && a && cp_101x)
    {
        printb(core_id, d_armv7a_decode_svccp, "Adv. SIMD, VFP");
    }

    else if(op1_0x_xxx0 && a && cp_not_101x)
    {
        arm_stc(inst);
    }

    else if(op1_0x_xxx1 && a && cp_not_101x && rn_not_1111)
    {
        arm_ldc_imm(inst);
    }

    else if(op1_0x_xxx1 && cp_not_101x && rn_1111)
    {
        arm_ldc_ltrl(inst);
    }

    else if(op1_00_000x)
    {
        printb(core_id, d_armv7a_decode_svccp, "undef");
    }

    else if(op1_00_010x && cp_101x)
    {
        printb(core_id, d_armv7a_decode_svccp, "Adv. SIMD, VFP 2");
    }

    else if(op1_00_0100 && cp_not_101x)
    {
        arm_mcrr(inst);
    }

    else if(op1_00_0101 && cp_not_101x)
    {
        arm_mrrc(inst);
    }

    else if(op1_10_xxxx && op_0 && cp_101x)
    {
        printb(core_id, d_armv7a_decode_svccp, "VFP");
    }

    else if(op1_10_xxxx && op_0 && cp_not_101x)
    {
        arm_cdp(inst);
    }

    else if(op1_10_xxxx && op_1 && cp_101x)
    {
        printb(core_id, d_armv7a_decode_svccp, "Adv. SIMD, VFP 3");
    }

    else if(op1_10_xxx0 && op_1 && cp_not_101x)
    {
        arm_mcr(inst);
    }

    else if(op1_10_xxx1 && op_1 && cp_not_101x)
    {
        arm_mrc(inst);
    }

    else if(op1_11_xxxx)
    {
        arm_svc(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_svccp, "decode error: %X", inst.val);
    }
}

void armv7a::arm_stc(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_stc not implemented yet.");
}

void armv7a::arm_ldc_imm(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldc_imm not implemented yet.");
}

void armv7a::arm_ldc_ltrl(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldc_ltrl not implemented yet.");
}

void armv7a::arm_mcrr(armv7a_ir& inst)
{
    inst.print_inst("arm_mcrr");
    inst.check(27, 20, B(1100 0100));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rt2 = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t coproc = inst(11, 8);
        uint32_t opc1 = inst(7, 4);
        uint32_t crm = inst(3, 0);

#ifdef CPU_ASSERT
        //A1
        if(inst.cond() != B(1111))
        {
            if(get_field(coproc, 3, 1) == B(101))
            {
                printb(d_inst, "arm_mcrr adv. simd");
            }
        }
        //A2
        else
        {
            if(get_field(coproc, 3, 1) == B(101))
            {
                printb(d_inst, "arm_mcrr undefined.");
            }
        }
#endif

        uint32_t t = rt;
        uint32_t t2 = rt2;
        uint32_t cp = coproc;

#ifdef CPU_ASSERT
        if((t == 15) || (t2 == 15))
        {
            printb(d_inst, "arm_mcrr unpredict.");
        }
        if(((t == 13) || (t2 == 13)) && (rf.current_inst_set() != InstSet_ARM))
        {
            printb(d_inst, "arm_mcrr unpredict. 2");
        }
#endif

#ifdef CPU_ASSERT
        //ESO
        if(cp != 15)
        {
            printb(d_inst, "arm_mcrr cp not 15");
        }
#endif

        cp15.write64(rf.r_read(t2), rf.r_read(t), crm, opc1);
    }
}

void armv7a::arm_mrrc(armv7a_ir& inst)
{
    inst.print_inst("arm_mrrc");
    inst.check(27, 20, B(1100 0101));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rt2 = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t coproc = inst(11, 8);
        uint32_t opc1 = inst(7, 4);
        uint32_t crm = inst(3, 0);

#ifdef CPU_ASSERT
        //A1
        if(inst.cond() != B(1111))
        {
            if(get_field(coproc, 3, 1) == B(101))
            {
                printb(d_inst, "arm_mrrc adv. simd");
            }
        }
        //A2
        else
        {
            if(get_field(coproc, 3, 1) == B(101))
            {
                printb(d_inst, "arm_mrrc undefined.");
            }
        }
#endif

        uint32_t t = rt;
        uint32_t t2 = rt2;
        uint32_t cp = coproc;

#ifdef CPU_ASSERT
        if((t == 15) || (t2 == 15) || (t == t2))
        {
            printb(d_inst, "arm_mrrc unpredict.");
        }
        if(((t == 13) || (t2 == 13)) && (rf.current_inst_set() != InstSet_ARM))
        {
            printb(d_inst, "arm_mrrc unpredict. 2");
        }
#endif

#ifdef CPU_ASSERT
        //ESO
        if(cp != 15)
        {
            printb(d_inst, "arm_mrrc cp not 15");
        }
#endif

        uint32_t value_l, value_h;
        cp15.read64(&value_h, &value_l, crm, opc1);

        rf.r_write(t2, value_h);
        rf.r_write(t, value_l);
    }
}

void armv7a::arm_cdp(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_cdp not implemented yet.");
}

void armv7a::arm_mcr(armv7a_ir& inst)
{
    inst.print_inst("arm_mcr");

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t opc1;
        uint32_t crn;
        uint32_t rt;
        uint32_t coproc;
        uint32_t opc2;
        uint32_t crm;
        uint32_t t;
        uint32_t cp;


        if(inst.cond() != B(1111))
        {
            //Encoding A1
            inst.check(27, 24, B(1110));
            inst.check(20, 0);
            inst.check(4, 1);
            opc1 = inst(23, 21);
            crn = inst(19, 16);
            rt = inst(15, 12);
            coproc = inst(11, 8);
            opc2 = inst(7, 5);
            crm = inst(3, 0);

            if(get_field(coproc, 3, 1) == B(101))
            {
                printb(d_inst, "arm_mcr Adv. SIMD");
            }

            t = rt;
            cp = coproc;

            if((t == 15) || ((t == 13) && (rf.current_inst_set() != InstSet_ARM)))
            {
                printb(d_inst, "arm_mcr error");
            }
        }
        else
        {
            //Encoding A2
            inst.check(31, 24, B(1111 1110));
            inst.check(20, 0);
            inst.check(4, 1);
            opc1 = inst(23, 21);
            crn = inst(19, 16);
            rt = inst(15, 12);
            coproc = inst(11, 8);
            opc2 = inst(7, 5);
            crm = inst(3, 0);
            t = rt;
            cp = coproc;

            if((t == 15) || ((t == 13) && (rf.current_inst_set() != InstSet_ARM)))
            {
                printb(d_inst, "arm_mcr error");
            }
        }

        bool trap_cp15_c7 = (coproc == 15) && cp15.read(HSTR_T7) && (crn == 7);
        bool trap_cp15_c8 = (coproc == 15) && cp15.read(HSTR_T8) && (crn == 8);
        bool trap_cp15 = trap_cp15_c7 || trap_cp15_c8;
        bool trap_cp14 = false;
        bool trap_cp13 = false;

        if(trap_cp15 || trap_cp14 || trap_cp13)
        {
            if(generate_coprocessor_exception(coproc, false, true, crn, opc1, crm, opc2, rt))
            {
                //trapped
                return;
            }
        }

        //ESO

#ifdef CPU_ASSERT
        if(cp != 15)
        {
            printb(d_inst, "arm_mcr cp != 15");
        }
#endif

        cp15.write(rf.r_read(t), crn, opc1, crm, opc2);
    }
}

void armv7a::arm_mrc(armv7a_ir& inst)
{
    inst.print_inst("arm_mrc");
    inst.check(27, 24, B(1110));
    inst.check(20, 1);
    inst.check(4, 1);

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t opc1 = inst(23, 21);
        uint32_t crn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t coproc = inst(11, 8);
        uint32_t opc2 = inst(7, 5);
        uint32_t crm = inst(3, 0);
        uint32_t t;
        uint32_t cp;

        bool trap_cp15_c7 = (coproc == 15) && cp15.read(HSTR_T7) && (crn == 7);
        bool trap_cp15_c8 = (coproc == 15) && cp15.read(HSTR_T8) && (crn == 8);
        bool trap_cp15 = trap_cp15_c7 || trap_cp15_c8;
        bool trap_cp14 = false;
        bool trap_cp13 = false;

        if(trap_cp15 || trap_cp14 || trap_cp13)
        {
            if(generate_coprocessor_exception(coproc, false, false, crn, opc1, crm, opc2, rt))
            {
                //trapped
                return;
            }
        }

        if(inst.cond() != B(1111))
        {
            //Encoding A1
            if(get_field(coproc, 3, 1) == B(101))
            {
                printb(d_inst, "mrc SIMD VFP");
            }

            t = rt;
            cp = coproc;

            if((t == 13) && (rf.current_inst_set() != InstSet_ARM))
            {
                printb(d_inst, "mrc error");
            }
        }
        else
        {
            //Encoding A2
            t = rt;
            cp = coproc;

            if((t == 13) && (rf.current_inst_set() != InstSet_ARM))
            {
                printb(d_inst, "mrc error");
            }
        }

        //ESO

        if(cp != 15)
        {
            printb(d_inst, "arm_mrc cp != 15");
        }
        else
        {
            uint32_t value;
            cp15.read(&value, crn, opc1, crm, opc2);

            if(t != 15)
            {
                rf.r_write(t, value);
            }
            else
            {
                rf.cpsr_N(get_bit(value, 31));
                rf.cpsr_Z(get_bit(value, 30));
                rf.cpsr_C(get_bit(value, 29));
                rf.cpsr_V(get_bit(value, 28));
            }
        }
    }
}

void armv7a::arm_svc(armv7a_ir& inst)
{
    inst.print_inst("arm_svc");
    inst.check(27, 24, B(1111));

    if(rf.condition_passed(inst.cond()))
    {
        bits imm24(inst(23, 0), 24);
        bits imm32(0, 32);
        zero_extend(&imm32, imm24, 32);
        if(imm24.val == 0xFFFFFF)
        {
            printb(d_inst, "svc imm=0xFFFFFF, exit simulation");
        }
        call_supervisor();
    }
}

