#include <armv7a.h>

void armv7a::call_supervisor()
{
    this->take_svc_exception();
}

uint32_t armv7a::exc_vector_base()
{
    if(cp15.read(SCTLR_V) == 1)
    {
        return 0xFFFF0000;
    }
    else if(armv7a::have_security_ext())
    {
        printm_once(d_armv7a_exception, "ignore the use of VBAR by security extension");
        //return cp15.read(VBAR);
        if(cp15.read(SCTLR_V) == 1)
        {
            return 0xFFFF0000;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

void armv7a::enter_hyp_mode(uint32_t new_spsr_value, uint32_t preferred_exceptn_return, uint32_t vect_offset)
{
    rf.cpsr_M(B(11010));
    rf.spsr(new_spsr_value);
    rf.elr_hyp(preferred_exceptn_return);
    rf.cpsr_J(0);
    rf.cpsr_T(cp15.read(HSCTLR_TE));
    rf.cpsr_E(cp15.read(HSCTLR_EE));

    printd(d_armv7a_exception, "enter_hyp_mode, current_pc = 0x%X, excpt_return = 0x%X, elr_hyp=0x%X", rf.current_pc(), preferred_exceptn_return, rf.elr_hyp());

    if(cp15.read(SCR_EA) == 0)
    {
        rf.cpsr_A(1);
    }

    if(cp15.read(SCR_FIQ) == 0)
    {
        rf.cpsr_F(1);
    }

    if(cp15.read(SCR_IRQ) == 0)
    {
        rf.cpsr_I(1);
    }

    rf.cpsr_IT(0);

    //printd(d_armv7a_exception, "branch to 0x%X", cp15.read(HVBAR)+vect_offset);
    rf.branch_to(cp15.read(HVBAR) + vect_offset);

    inst.hyp_mode(true);
}

bool armv7a::generate_coprocessor_exception(uint32_t cp, bool is_64, bool is_write, uint32_t crn,
                                            uint32_t opc1, uint32_t crm, uint32_t opc2, uint32_t rt)
{
    uint32_t ec = 0;
    uint32_t il = 1;
    uint32_t iss = 0;
    uint32_t hsr = 0;

    if(cp == 15)
    {
        ec = is_64 ? 4 : 3;

        set_field(&iss, 19, 17, opc2);
        set_field(&iss, 16, 14, opc1);
        set_field(&iss, 13, 10, crn);
        set_field(&iss, 8, 5, rt);
        set_field(&iss, 4, 1, crm);
        set_bit(&iss, 0, is_write ? 0 : 1);

        set_field(&hsr , 24, 0, iss);
        set_bit(&hsr, 25, il);
        set_field(&hsr, 31, 26, ec);

        cp15.write(HSR, hsr);

        take_hyp_trap_exception();

        return true;
    }
    else if(cp == 14)
    {
        ec = 5;
        return false;
    }
    else
    {
        return false;
    }
}

void armv7a::prefetch_abort(uint32_t v_address, uint64_t ip_address,
                            uint32_t level, bool is_write, fault_type type, bool take_to_hyp_mode,
                            bool second_stage_abort, bool ipa_valid, bool ldfsr_format, bool s2fs1_walk)
{
    //VMSA
    if(memory_system_architecture() == MemArch_VMSA)
    {
        //!take to hyp
        if(!take_to_hyp_mode)
        {
            if(ldfsr_format)
            {
                uint32_t ifsr = encode_ld_fsr(type, level);
                set_bit(&ifsr, 9, 1);
                cp15.write(IFAR, v_address);
                cp15.write(IFSR, ifsr);
            }
            else
            {
                uint32_t fs = encode_sd_fsr(type, level);
                uint32_t ifsr = fs & mask(3, 0);
                set_bit(&ifsr, 10, get_bit(fs, 4));
                cp15.write(IFAR, v_address);
                cp15.write(IFSR, ifsr);
            }
        }
        //take to hyp
        else
        {
            printb(d_armv7a_exception, "prefetch_abort to hyp");
            /*
            cp15.write(HDFAR, hdfar);
            uint32_t hsr_string = 0;
            uint32_t ec;
            uint32_t hdfar = v_address;
            uint32_t hpfar;

            if(ipa_valid)
            {
                set_field(&hdfar, 31, 4, get_field64(ip_address, 39, 12));
            }
            if(second_stage_abort)
            {
                ec = B(100100);
                set_bit(&hsr_string, 24, 0);
            }
            if((type == AsyncExternal) || (type == SyncExternal))
            {
                set_bit(&hsr_string, 9, 1);
            }
            else
            {
                set_bit(&hsr_string, 9, 0);
            }
            set_bit(&hsr_string, 8, tlb_lookup_from_cache_maitenance());
            set_bit(&hsr_string, 7 , s2fs1_walk ? 1 : 0);
            set_bit(&hsr_string, 6, is_write ? 1 : 0);
            set_field(&hsr_string, 5, 0, encode_ldfsr(type, level));
            write_hsr(ec, hsr_string);
            */
        }
    }
    //PMSA
    else
    {
        printb(d_armv7a_cmsa, "prefetch_abort PMSA");
    }

    printd(d_armv7a_exception, "prefetch abort, pc=0x%X", rf.current_pc());
    printd(d_armv7a_exception, "ifsr=0x%X ifar=0x%X", cp15.read(IFSR), cp15.read(IFAR));
    printd(d_inst_clean, "prefetch abort at 0x%08X", rf.current_pc());
    take_prefetch_abort_exception(second_stage_abort);
}

void armv7a::take_data_abort_exception(bool is_alignment_fault, bool second_stage_abort)
{
#ifdef CPU_ASSERT
    if(rf.cpsr_T() == 1)
    {
        printb(d_armv7a_exception, "take_data_abort_exception in thumb mode, pc=0x%X", rf.current_pc());
    }
#endif
    //LR = (address of instruction that generated the abort) + 8
    //= (rf.current_pc() - 4)                             + 8
    //= rf.current_pc() + 4
    //noted that we had done instruction fetch, so the value of pc had been changed (+4)

    uint32_t new_lr_value = rf.current_pc() + 4;
    uint32_t new_spsr_value = rf.cpsr();
    uint32_t vect_offset = 16;
    //ARM Ref Manual B1-1169
    //exception return address = address of instruction that generate the abort
    uint32_t preferred_exceptn_return  = rf.current_pc() - 4;

    bool route_to_monitor = have_security_ext() && (cp15.read(SCR_EA) == 1) && false;
    bool take_to_hyp = have_virt_ext() && have_security_ext() && (cp15.read(SCR_NS) == 1) && (rf.cpsr_M() == B(11010)); //data exception from HYP mode to HYP mode

    const bool is_external_abort = false;
    const bool is_async_abort = false;
    const bool debug_exception = false;

    bool route_to_hyp =
        (
            have_virt_ext() && have_security_ext() && /*!is_secure() &&*/
            (
                second_stage_abort ||
                (
                    (rf.cpsr_M() != B(11010)) && (is_external_abort && is_async_abort && (cp15.read(HCR_AMO) == 1)) ||
                    debug_exception && (cp15.read(HDCR_TDE) == 1)
                ) ||
                (
                    (rf.cpsr_M() == B(10000)) && (cp15.read(HCR_TGE) == 1) &&
                    (is_alignment_fault || (is_external_abort && !is_async_abort))
                )
            )
        );

    if(route_to_monitor)
    {
        printb(d_armv7a_exception, "take_data_abort_exception monitor");
    }
    else if(take_to_hyp)
    {
        printd(d_armv7a_exception, "take_data_abort_exception take_to_hyp");
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, vect_offset);
    }
    else if(route_to_hyp)
    {
        printd(d_armv7a_exception, "take_data_abort_exception route_to_hyp");
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, 20);
    }
    else
    {
        if(have_security_ext() && (rf.cpsr_M() == B(10110)))
        {
            cp15.write(SCR_NS, 0);
        }

        rf.cpsr_M(B(10111));
        rf.spsr(new_spsr_value);
        rf.r_write(14, new_lr_value);
        rf.cpsr_I(1);

        if(!have_security_ext() || have_virt_ext() || (cp15.read(SCR_NS) == 0) || (cp15.read(SCR_AW) == 1))
        {
            rf.cpsr_A(1);
        }

        rf.cpsr_IT(0);
        rf.cpsr_J(0);
        rf.cpsr_T(cp15.read(SCTLR_TE));
        rf.cpsr_E(cp15.read(SCTLR_EE));
        rf.branch_to(exc_vector_base() + vect_offset);
    }
}

void armv7a::take_hvc_exception()
{
#ifdef CPU_ASSERT
    if(rf.cpsr_T() == 1)
    {
        printb(d_armv7a_exception, "take_hvc_exception in thumb mode, pc=0x%X", rf.current_pc());
    }
#endif
    //printb(d_armv7a_exception, "take_hvc_exception");
    it_advance();
    //ARM Ref Manual B1-1168
    //exception return address = instruction after HVC
    uint32_t preferred_exceptn_return = rf.current_pc();
    uint32_t new_spsr_value = rf.cpsr();

    if(rf.cpsr_M() == B(11010))
    {
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, 8);
    }
    else
    {
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, 20);
    }
}

void armv7a::take_hyp_trap_exception()
{
#ifdef CPU_ASSERT
    if(rf.cpsr_T() == 1)
    {
        printb(d_armv7a_exception, "take_hyp_trap_exception in thumb mode, pc=0x%X", rf.current_pc());
    }
#endif
    printd(d_armv7a_exception, "hyp_trap");
    //ARM Ref Manual B1-1169
    //exception return address = address of the trapped instruction
    uint32_t preferred_exceptn_return = rf.current_pc() - 4;
    uint32_t new_spsr_value = rf.cpsr();

    enter_hyp_mode(new_spsr_value, preferred_exceptn_return, 20);
}

void armv7a::take_physical_irq_exception()
{
#ifdef CPU_ASSERT
    if(rf.cpsr_T() == 1)
    {
        printb(d_armv7a_exception, "take_physical_irq_exception in thumb mode, pc=0x%X", rf.current_pc());
    }
#endif
    //LR = (address of next instruction to execute) + 4
    //= rf.current_pc()                          + 4
    //noted that at this time we hadn't done instruction fetch yet
    printd(d_armv7a_exception, "take_physical_irq_exception");

    uint32_t new_lr_value = rf.current_pc() + 4;
    uint32_t new_spsr_value = rf.cpsr();
    uint32_t vect_offset = 24;

    bool route_to_monitor = have_security_ext() && (cp15.read(SCR_IRQ) == 1);

    bool route_to_hyp =
        (
            have_virt_ext() && have_security_ext() && (cp15.read(SCR_IRQ) == 0) &&
            (cp15.read(HCR_IMO) == 1) /*&& !is_secure()*/
        ) ||
        (rf.cpsr_M() == B(11010));

    if(route_to_monitor)
    {
        printb(d_armv7a_exception, "take_physical_irq_exception to monitor mode");
    }
    else if(route_to_hyp)
    {
        //ARM Ref Manual B1-1169
        //exception return address = address of next instruction to execute
        //noted that if irq comes before instruction fetch, so pc is not +4ed
        //printd(d_armv7a_exception, "take irq exception to hyp mode");
        uint32_t preferred_exceptn_return = rf.current_pc();
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, vect_offset);
    }
    else
    {
        if(rf.cpsr_M() == B(10110))
        {
            cp15.write(SCR_NS, 0);
        }

        rf.cpsr_M(B(10010));

        rf.spsr(new_spsr_value);
        rf.r_write(14, new_lr_value);
        rf.cpsr_I(1);

        if(!have_security_ext() || have_virt_ext()  || (cp15.read(SCR_NS) == 0) || (cp15.read(SCR_AW) == 1))
        {
            rf.cpsr_A(1);
        }

        rf.cpsr_IT(0);
        rf.cpsr_J(0);
        rf.cpsr_T(cp15.read(SCTLR_TE));
        rf.cpsr_E(cp15.read(SCTLR_EE));

        if(cp15.read(SCTLR_VE) == 1)
        {
            printb(d_armv7a_exception, "take_physical_irq_exception imple. define");
        }
        else
        {
            rf.branch_to(exc_vector_base() + vect_offset);
        }
    }
}

void armv7a::take_prefetch_abort_exception(bool second_stage_abort)
{
#ifdef CPU_ASSERT
    if(rf.cpsr_T() == 1)
    {
        printb(d_armv7a_exception, "take_prefetch_abort_exception in thumb mode, pc=0x%X", rf.current_pc());
    }
#endif

    //LR = (address of aborted instruction) + 4
    //= rf.current_pc()                  + 4
    //noted that pc will increment(+4) only AFTER we have done instruction fetch
    uint32_t new_lr_value = rf.current_pc() + 4;
    uint32_t new_spsr_value = rf.cpsr();
    uint32_t vect_offset = 12;
    //ARM Ref Manual B1-1168
    //exception return address = address of the aborted instruction
    //noted that pc will increment(+4) only AFTER we have done instruction fetch
    uint32_t preferred_exceptn_return = rf.current_pc();

    bool route_to_monitor = have_security_ext() && (cp15.read(SCR_EA) == 1) && false;

    bool take_to_hyp = have_virt_ext() && have_security_ext() && (cp15.read(SCR_NS) == 1) && (rf.cpsr_M() == B(11010));

    bool route_to_hyp = have_virt_ext() && have_security_ext() && !is_secure() &&
                        (second_stage_abort || (false && false && (cp15.read(HDCR_TDE) == 1) && (rf.cpsr_M() != B(11010))));

    if(route_to_monitor)
    {
        printb(d_armv7a_exception, "take_prefetch_abort_exception monitor");
    }
    else if(take_to_hyp)
    {
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, vect_offset);
    }
    else if(route_to_hyp)
    {
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, 20);
    }
    else
    {


        if(have_security_ext() && (rf.cpsr_M() == B(10110)))
        {
            cp15.write(SCR_NS, 0);
        }
        rf.cpsr_M(B(10111));

        rf.spsr(new_spsr_value);
        rf.r_write(14, new_lr_value);
        rf.cpsr_I(1);
        if(!have_security_ext() || have_security_ext() || (cp15.read(SCR_NS) == 0) || (cp15.read(SCR_AW) == 1))
        {
            rf.cpsr_A(1);
        }

        rf.cpsr_IT(0);
        rf.cpsr_J(0);
        rf.cpsr_T(cp15.read(SCTLR_TE));
        rf.cpsr_E(cp15.read(SCTLR_EE));

        rf.branch_to(exc_vector_base() + vect_offset);
    }
}

void armv7a::take_svc_exception()
{
    printd(d_armv7a_exception, "check take_svc_exception, pc=0x%X", rf.current_pc());

#ifdef CPU_ASSERT
    if(rf.cpsr_T() == 1)
    {
        printb(d_armv7a_exception, "take_svc_exception in thumb mode");
    }
#endif

    it_advance();
    //LR = instruction after SVC
    //= (address of svc instruction) + 4
    //= (rf.current_pc() - 4)        + 4
    //= rf.current_pc()
    uint32_t new_lr_value = rf.current_pc();
    uint32_t new_spsr_value = rf.cpsr();
    uint32_t vect_offset = 8;

    bool take_to_hyp = have_virt_ext() && have_security_ext() && (cp15.read(SCR_NS) == 1) && (rf.cpsr_M() == B(11010));
    bool route_to_hyp = have_virt_ext() && have_security_ext() && !is_secure() && (cp15.read(HCR_TGE) == 1) && (rf.cpsr_M() == B(10000));

    //ARM Ref Manual B1-1168
    //exception return address = address after SVC instruction
    uint32_t preferred_exceptn_return = rf.current_pc();

    if(take_to_hyp)
    {
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, vect_offset);
    }
    else if(route_to_hyp)
    {
        enter_hyp_mode(new_spsr_value, preferred_exceptn_return, 20);
    }
    else
    {
        if(rf.cpsr_M() == B(10110))
        {
            cp15.write(SCR_NS, 0);
        }
        rf.cpsr_M(B(10011));

        rf.spsr(new_spsr_value);
        rf.r_write(14, new_lr_value);
        rf.cpsr_I(1);
        rf.cpsr_IT(0);
        rf.cpsr_J(0);
        rf.cpsr_T(cp15.read(SCTLR_TE));
        rf.cpsr_E(cp15.read(SCTLR_EE));
        rf.branch_to(exc_vector_base() + vect_offset);
    }
}

void armv7a::take_virtual_irq_exception()
{
#ifdef CPU_ASSERT
    if(rf.cpsr_T() == 1)
    {
        printb(d_armv7a_exception, "take_virtual_irq_exception in thumb mode");
    }
#endif

    printd(d_armv7a_exception, "take_virtual_irq_exception");

    //LR = (address of next instruction to execute) + 4
    //   = (rf.current_pc()) + 4
    //because we take irq exception before instruction fetch
    uint32_t new_lr_value = rf.current_pc() + 4;
    uint32_t  new_spsr_value = rf.cpsr();
    uint32_t vect_offset = 24;

    rf.cpsr_M(B(10010));

    rf.spsr(new_spsr_value);
    rf.r_write(14, new_lr_value);
    rf.cpsr_I(1);
    rf.cpsr_A(1);
    rf.cpsr_IT(0);
    rf.cpsr_J(0);
    rf.cpsr_T(cp15.read(SCTLR_TE));
    rf.cpsr_E(cp15.read(SCTLR_EE));

    if(cp15.read(SCTLR_VE) == 1)
    {
        printb(d_armv7a_exception, "take_virtual_irq_exception imple. define");
    }
    else
    {
        rf.branch_to(exc_vector_base() + vect_offset);
    }
}

