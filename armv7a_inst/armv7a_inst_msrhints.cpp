#include <armv7a.h>

void armv7a::decode_msrhints(armv7a_ir& inst)
{
    uint32_t op = inst(22);
    uint32_t op1 = inst(19, 16);
    uint32_t op2 = inst(7, 0);
    bool op_0 = op == 0;
    bool op_1 = op == 1;
    bool op1_0000 = (op1 & B(1111)) == B(0000);
    bool op1_0100 = (op1 & B(1111)) == B(0100);
    bool op1_1x00 = (op1 & B(1011)) == B(1000);
    bool op1_xx01 = (op1 & B(0011)) == B(0001);
    bool op1_xx1x = (op1 & B(0010)) == B(0010);
    bool op2_0000_0000 = (op2 & B(1111 1111)) == B(0000 0000);
    bool op2_0000_0001 = (op2 & B(1111 1111)) == B(0000 0001);
    bool op2_0000_0010 = (op2 & B(1111 1111)) == B(0000 0010);
    bool op2_0000_0011 = (op2 & B(1111 1111)) == B(0000 0011);
    bool op2_0000_0100 = (op2 & B(1111 1111)) == B(0000 0100);
    bool op2_1111_xxxx = (op2 & B(1111 0000)) == B(1111 0000);

    if(op_0 && op1_0000 && op2_0000_0000)
    {
        arm_nop(inst);
    }

    else if(op_0 && op1_0000 && op2_0000_0001)
    {
        arm_yield(inst);
    }

    else if(op_0 && op1_0000 && op2_0000_0010)
    {
        arm_wfe(inst);
    }

    else if(op_0 && op1_0000 && op2_0000_0011)
    {
        arm_wfi(inst);
    }

    else if(op_0 && op1_0000 && op2_0000_0100)
    {
        arm_sev(inst);
    }

    else if(op_0 && op1_0000 && op2_1111_xxxx)
    {
        arm_dbg(inst);
    }

    else if(op_0 && op1_0100)
    {
        arm_msr_imm_ap(inst);
    }

    else if(op_0 && op1_1x00)
    {
        arm_msr_imm_ap(inst);
    }

    else if(op_0 && op1_xx01)
    {
        arm_msr_imm_sys(inst);
    }

    else if(op_0 && op1_xx1x)
    {
        arm_msr_imm_sys(inst);
    }

    else if(op_1)
    {
        arm_msr_imm_sys(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_msrhints, "decode error: 0x%X, pc=%X", inst.val, rf.current_pc());
    }
}

void armv7a::arm_nop(armv7a_ir& inst)
{
    inst.print_inst("arm_nop");
    inst.check(27, 16, B(0011 0010 0000));
    inst.check(15, 0, B(1111 0000 0000 0000));
}

void armv7a::arm_yield(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_yield not implemented yet.");
}

void armv7a::arm_wfe(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_wfe not implemented yet.");
}

void armv7a::arm_wfi(armv7a_ir& inst)
{
    inst.print_inst("arm_wfi");
    inst.check(27, 16, B(0011 0010 0000));
    inst.check(15, 0, B(1111 0000 0000 0011));

    if(rf.condition_passed(inst.cond()))
    {
        //ESO
        if(have_virt_ext() && /*!is_secure() &&*/ !rf.current_mode_is_hyp() &&
           cp15.read(HCR_TWI) == 1)
        {
            uint32_t hsr_string = 0;
            hsr_string = 0;//shoude be 0 here?
            printm_once(d_inst, "wfi strange hsr_string, please check newer version of ARM Ref Manual");
            write_hsr(B(000001), hsr_string);
            take_hyp_trap_exception();
        }
        else
        {
            wfi = true;
        }
    }
}

void armv7a::arm_sev(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sev not implemented yet.");
}

void armv7a::arm_dbg(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_dbg not implemented yet.");
}

void armv7a::arm_msr_imm_ap(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_msr_imm_ap not implemented yet.");
}

void armv7a::arm_msr_imm_sys(armv7a_ir& inst)
{
    inst.print_inst("arm_msr_imm_sys");
    inst.check(27, 23, B(00 110));
    inst.check(21, 20, B(10));
    inst.check(15, 12, B(1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t r = inst(22);
        uint32_t mask = inst(19, 16);
        bits imm12(inst(11, 0), 12);

        if((mask == B(0000)) && (r == 0))
        {
            printb(d_inst, "arm_msr_imm_sys related encodings");
        }

        bits imm32;
        arm_expand_imm(&imm32, imm12);
        bool write_spsr = r == 1;

        if(mask == B(0000))
        {
            printb(d_inst, "arm_msr_imm_sys error");
        }

        //ESO

        if(write_spsr)
        {
            spsr_write_by_inst(imm32.val, mask);
        }

        else
        {
            cpsr_write_by_inst(imm32.val, mask, false);
        }
    }
}

