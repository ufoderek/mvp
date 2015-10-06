#include <armv7a.h>
#include <cmath>

void armv7a::call_hypervisor(uint16_t immediate)
{
    uint32_t hsr_string = 0;

    hsr_string = immediate;

    write_hsr(B(010010), hsr_string);

    take_hvc_exception();
}

bool armv7a::check_domain(uint32_t domain, uint32_t mva, uint32_t level,
                          bool is_write)
{

    uint64_t ip_address;
    bool take_to_hyp_mode = false;
    bool second_stage_abort = false;
    bool ipa_valid = false;
    bool ldfsr_format = false;
    bool s2fs1_walk = false;

    uint32_t bitpos = 2 * domain;

    bool permission_check;

    switch(get_field(cp15.read(DACR), bitpos + 1, bitpos))
    {
        case B(00):
            printb(d_armv7a_vmsa, "check_domain data abort");
            //permission_check = false;
            break;
        case B(01):
            permission_check = true;
            break;
        case B(10):
            printb(d_armv7a_vmsa, "check_domain error");
            break;
        case B(11):
            permission_check = false;
            break;
        default:
            printb(d_armv7a_vmsa, "check_domain error 2");
            break;
    }

    return permission_check;
}

bool armv7a::check_permission_s2(permissions perms, uint32_t mva, uint64_t ipa,
                                 uint32_t level, bool is_write, bool s2fs1_walk,
                                 bool inst_fetch, bool gdb_access)
{
    // I'm sorry.
    printm_once(d_armv7a_vmsa, "ignore check_permissions_s2");
    return true;
}

void armv7a::combine_s1s2_desc(address_descriptor* result,
                               address_descriptor s1_desc,
                               address_descriptor s2_desc)
{
    // I'm very sorry.
    printm_once(d_armv7a_vmsa, "WRONG imple. of combine_s1s2_desc");
    *result = s2_desc;
}

uint32_t armv7a::convert_attrs_hints(uint32_t rgn)
{
    uint32_t attributes;
    uint32_t hints;

    if(rgn == B(00))
    {
        attributes = B(00);
        hints = B(00);
    }
    else if(get_bit(rgn, 0) == 1)
    {
        attributes = B(11);
        set_bit(&hints, 1, 1);
        set_bit(&hints, 0, ~get_bit(rgn, 1));
    }
    else
    {
        attributes = B(10);
        hints = B(10);
    }

    return (hints << 2) | attributes;
}

uint32_t armv7a::fcse_translate(uint32_t va)
{
    //If fcseidr is not implement, its value is 0
#ifdef CPU_ASSERT
    return va;

    // will get error, strange
    uint32_t mva;
    if(get_field(va, 31, 25) == 0)
    {
        mva = (cp15.read(FCSEIDR_PID) << 25) | get_field(mva, 24, 0);
    }
    else
    {
        mva = va;
    }
    return mva;
#else
    return va;
#endif
}

void armv7a::mair_decode(memory_attributes* mem_attrs, uint32_t attr)
{
    //TODO: implement this function
    printb(d_armv7a_vmsa, "mair_decode");
}

void armv7a::remapped_tex_decode(memory_attributes* mem_attrs, uint32_t texcb, bool s)
{
    uint32_t region = get_field(texcb, 2, 0);

#ifdef CPU_ASSERT
    if(region == 6)
    {
        printm(d_armv7a_vmsa, "remapped_to_tex_decode imple. defined");
        return;
    }
#endif

    uint32_t s_bit;
    uint32_t hints_attrs;

    switch(get_field(cp15.read(PRRR), 2 * region + 1, 2 * region))
    {
        case B(00):
            mem_attrs->type = MemType_SO;
            mem_attrs->shareable = true;
            mem_attrs->outer_shareable = true;
            break;
        case B(01):
            mem_attrs->type = MemType_Device;
            mem_attrs->shareable = true;
            mem_attrs->outer_shareable = true;
            break;
        case B(10):
            mem_attrs->type = MemType_Normal;
            hints_attrs = convert_attrs_hints(get_field(cp15.read(NMRR),
                                                        2 * region + 1, 2 * region));
            mem_attrs->inner_attrs = get_field(hints_attrs, 1, 0);
            mem_attrs->inner_hints = get_field(hints_attrs, 3, 2);
            hints_attrs = convert_attrs_hints(get_field(cp15.read(NMRR),
                                                        2 * region + 17,
                                                        2 * region + 16));
            mem_attrs->outer_attrs = get_field(hints_attrs, 1, 0);
            mem_attrs->outer_hints = get_field(hints_attrs, 3, 2);

            s_bit = (s == 0) ? cp15.read(PRRR_NS0) : cp15.read(PRRR_NS1);
            mem_attrs->shareable = s_bit == 1;
            mem_attrs->outer_shareable = (s_bit == 1) &&
                                         (get_bit(cp15.read(PRRR), region + 24) == 0);
            break;
        case B(11):
            printb(d_armv7a_vmsa, "remapped_tex_decode reserved");
            break;
        default:
            printb(d_armv7a_vmsa, "remapped_tex_decode error");
            break;
    }
}

void armv7a::s2_attr_decode(memory_attributes* mem_attrs, uint32_t attr)
{
    // I'm very very sorry.
    printm_once(d_armv7a_vmsa, "ignore s2_attr_decode");
    return;
}

bool armv7a::second_stage_translate(address_descriptor* result,
                                    address_descriptor s1_out_addr_desc,
                                    uint32_t mva, uint32_t size, bool inst_fetch,
                                    bool gdb_access)
{

    tlb_record tlb_record_s2;

    //if(have_virt_ext() && !is_secure() && !rf.current_mode_is_hyp()) //code from ARM Ref Manaul, but we slightly modified here
    if(have_virt_ext() && !rf.current_mode_is_hyp())
    {
        if(cp15.read(HCR_VM) == 1)
        {
            uint64_t s2_ia = s1_out_addr_desc.p_address.physical_address;
            bool is_write = false;
            bool stage1 = false;
            bool s2fs1_walk = true;
            printd(d_s2mmu, "second_stage_translate s2_ia=0x%X mva=0x%X", (uint32_t)s2_ia, (uint32_t)mva);
            if(!translation_table_walk_ld(&tlb_record_s2, s2_ia, mva, is_write, stage1,
                                          s2fs1_walk, size, inst_fetch, gdb_access))
            {
                printb(d_armv7a_vmsa, "second_stage_translation fault");
                return false;
            }
            if(!check_permission_s2(tlb_record_s2.perms, mva, s2_ia, tlb_record_s2.level,
                                    is_write, s2fs1_walk, inst_fetch, gdb_access))
            {
                printb(d_armv7a_vmsa, "second_stage_translation permission fault");
            }
            if(cp15.read(HCR_PTW) == 1)
            {
                printb(d_armv7a_vmsa, "second_stage_translation HCR_PTW = 1");
            }
            combine_s1s2_desc(result, s1_out_addr_desc, tlb_record_s2.addr_desc);
        }
        else
        {
            *result = s1_out_addr_desc;
        }
    }
    else
    {
        *result = s1_out_addr_desc;
    }

    return true;
}

bool armv7a::translate_address_v(address_descriptor* result, uint32_t va,
                                 bool is_priv, bool is_write, uint32_t size,
                                 bool inst_fetch, bool gdb_access)
{
    uint32_t mva;
    uint64_t ia_in;

    bool is_hyp;
    tlb_record tlb_record_s1; //stage 1
    tlb_record tlb_record_s2; //stage 2

    mva = fcse_translate(va);
    is_hyp = rf.current_mode_is_hyp();

    // stage 1 mmu on
    if((is_hyp && (cp15.read(HSCTLR_M) == 1)) || (!is_hyp && (cp15.read(SCTLR_M) == 1)))
    {
        bool uses_ld = is_hyp || (cp15.read(TTBCR_EAE) == 1);

        if(uses_ld)
        {
            printb(d_armv7a_vmsa, "translate_address_v stage 1 using long descripter, check here");

            ia_in = mva;
            if(!translation_table_walk_ld(&tlb_record_s1, ia_in, mva,
                                          is_write, true, false, size, inst_fetch,
                                          gdb_access))
            {
                return false;
            }
            if(!check_permission(tlb_record_s1.perms, mva, tlb_record_s1.level,
                                 tlb_record_s1.domain, is_write, is_priv,
                                 is_hyp, uses_ld, inst_fetch, gdb_access))
            {
                printb(d_armv7a_vmsa, "translate_address_v permission fault");
                return false;
            }
        }
        else
        {
            printd(d_s2mmu, "start stage 1 translate, mva = 0x%X", mva);
            if(!translation_table_walk_sd(&tlb_record_s1, mva, is_write, size,
                                          inst_fetch, gdb_access))
            {
                printd(d_s2mmu, "stage 1 translate false");
                return false;
            }
            // i think Linux doesn't use domain ???
            if(true || check_domain(tlb_record_s1.domain, mva,
                                    tlb_record_s1.level, is_write))
            {
                if(!check_permission(tlb_record_s1.perms, mva,
                                     tlb_record_s1.level, tlb_record_s1.domain,
                                     is_write, is_priv, is_hyp, uses_ld,
                                     inst_fetch, gdb_access))
                {
                    printd(d_s2mmu, "stage 1 translate false");
                    //printb(d_armv7a_vmsa, "translate_address_v permission fault 2");
                    return false;
                }
            }
            else
            {
                //if you do anything here, Linux will die.
                /*
                if(inst_fetch){
                    prefetch_abort(mva, mva, tlb_record_s1.level, is_write, Domain, false, false, false,false, false);
                    return false;
                }
                else{
                    data_abort(mva, mva, tlb_record_s1.domain, tlb_record_s1.level, is_write, Domain, false, false, false,false, false);
                    return false;
                }
                */
            }
            printd(d_s2mmu, "stage 1 translate done");
        }
    }
    //stage 1 mmu off
    else
    {
        if(!translate_address_v_s1_off(&tlb_record_s1, mva, inst_fetch,
                                       gdb_access))
        {
            return false;
        }
    }

    //stage 2 mmu on
    if(have_virt_ext() && /*!is_secure() &&*/ (!is_hyp))
    {
        //stage 2 mmu on
        if(cp15.read(HCR_VM) == 1)
        {
            printd(d_s2mmu, "start stage 2 translate");
            uint64_t s1_output_addr = tlb_record_s1.addr_desc.p_address.physical_address;
            if(!translation_table_walk_ld(&tlb_record_s2, s1_output_addr, mva,
                                          is_write, false, false, size, inst_fetch, gdb_access)) // ????
            {
                printd(d_armv7a_vmsa, "translate_address_v stage 2 error, is_hyp=%d, !is_hyp=%d", is_hyp, !is_hyp);
                return false;
            }

            bool s2fs1_walk = false;
            if(!check_permission_s2(tlb_record_s2.perms, mva, s1_output_addr,
                                    tlb_record_s2.level, is_write, s2fs1_walk,
                                    inst_fetch, gdb_access))
            {
                printb(d_armv7a_vmsa, "translate_address_v permission fault 2");
                return false;
            }
            combine_s1s2_desc(result, tlb_record_s1.addr_desc,
                              tlb_record_s2.addr_desc);
            printd(d_s2mmu, "stage 2 translate done, ipa = 0x%X, pa = 0x%X, pa2 = 0x%X", (uint32_t)s1_output_addr, (uint32_t)result->p_address.physical_address, (uint32_t)tlb_record_s2.addr_desc.p_address.physical_address);
        }
        //stage 2 mmu off
        else
        {
            *result = tlb_record_s1.addr_desc;
        }
    }
    //stage 2 mmu off
    else
    {
        *result = tlb_record_s1.addr_desc;
    }

    return true;
}

bool armv7a::translate_address_v_s1_off(tlb_record* result, uint32_t va,
                                        bool inst_fetch, bool gdb_access)
{
    if((cp15.read(HCR_DC) == 0) || is_secure() || rf.current_mode_is_hyp())
    {
        result->addr_desc.mem_attrs.type = MemType_SO;
        result->addr_desc.mem_attrs.shareable = true;
        result->addr_desc.mem_attrs.outer_shareable = true;
    }
    else
    {
        result->addr_desc.mem_attrs.type = MemType_Normal;
        result->addr_desc.mem_attrs.inner_attrs = B(11);
        result->addr_desc.mem_attrs.inner_hints = B(11);
        result->addr_desc.mem_attrs.outer_attrs = B(11);
        result->addr_desc.mem_attrs.outer_hints = B(11);
        result->addr_desc.mem_attrs.shareable = false;
        result->addr_desc.mem_attrs.outer_shareable = false;
        if(cp15.read(HCR_VM) != 1)
        {
            printb(d_armv7a_vmsa, "translate_address_v_s1_off error");
        }
    }

    result->perms.xn = 0;
    result->perms.pxn = 0;
    result->addr_desc.p_address.physical_address = va;
    result->addr_desc.p_address.ns = is_secure() ? 0 : 1;

    return true;
}

bool armv7a::translation_table_walk_sd(tlb_record* result, uint32_t mva,
                                       bool is_write, uint32_t size,
                                       bool inst_fetch, bool gdb_access)
{
    address_descriptor l1_desc_addr;
    address_descriptor l1_desc_addr2;
    address_descriptor l2_desc_addr;
    address_descriptor l2_desc_addr2;

    bool take_to_hyp_mode = false;
    uint64_t ia;
    bool ipa_valid = false;
    bool stage2 = false;
    bool ldfsr_format = false;
    bool s2fs1_walk = false;

    uint32_t domain;

    uint32_t n = cp15.read(TTBCR_N);
    uint32_t ttbr;
    bool disabled;
    uint32_t level;
    bool first_iteration;
    bool table_rw;
    bool table_user;
    bool table_xn;
    bool table_pxn;
    bool lookup_finished;
    bool block_translate;
    int32_t offset;

    if((n == 0) || (is_zero(get_field(mva, 31, 32 - n))))
    {
        ttbr = cp15.read(TTBR0);
        disabled = cp15.read(TTBCR_PD0) == 1;
    }
    else
    {
        ttbr = cp15.read(TTBR1);
        disabled = cp15.read(TTBCR_PD1) == 1;
        n = 0;
    }

    if(have_security_ext() && (disabled == 1))
    {
        level = 1;
        printb(d_armv7a_vmsa, "trans_walk_sd data abort");
        return false;
    }

    l1_desc_addr.p_address.physical_address = (get_field(ttbr, 31, 14 - n) << (14 - n)) |
                                              (get_field(mva, 31 - n, 20) << 2);
    l1_desc_addr.p_address.ns = is_secure() ? 0 : 1;
    l1_desc_addr.mem_attrs.type = MemType_Normal;
    l1_desc_addr.mem_attrs.shareable = get_bit(ttbr, 1) == 1;
    l1_desc_addr.mem_attrs.outer_shareable = (get_bit(ttbr, 5) == 0) &&
                                             (get_bit(ttbr, 1) == 1);
    uint32_t hints_attrs = convert_attrs_hints(get_field(ttbr, 4, 3));
    l1_desc_addr.mem_attrs.outer_attrs = get_field(hints_attrs, 1, 0);
    l1_desc_addr.mem_attrs.outer_hints = get_field(hints_attrs, 3, 2);

    if(have_mp_ext())
    {
        printb(d_armv7a_vmsa, "tras_table_walk_sd mp_ext");
        hints_attrs = convert_attrs_hints((get_bit(ttbr, 0) << 1) | get_bit(ttbr, 6));
        l1_desc_addr.mem_attrs.outer_attrs = get_field(hints_attrs, 1, 0);
        l1_desc_addr.mem_attrs.outer_hints = get_field(hints_attrs, 3, 2);
    }
    else
    {
        if(get_bit(ttbr, 0) == 0)
        {
            hints_attrs = convert_attrs_hints(B(00));
            l1_desc_addr.mem_attrs.outer_attrs = get_field(hints_attrs, 1, 0);
            l1_desc_addr.mem_attrs.outer_hints = get_field(hints_attrs, 3, 2);
        }
        else
        {
            printm_once(d_armv7a_vmsa, "impl. defined mem_attrs");
            l1_desc_addr.mem_attrs.outer_attrs = B(11);
            l1_desc_addr.mem_attrs.outer_hints = B(11);
        }
    }

    //only 1 stage of translation
    //if(!have_mp_ext() || is_secure()) //code from ARM Ref Manual, we slightly modified here
    if(false)
    {
        l1_desc_addr2 = l1_desc_addr;
    }
    else
    {
        if(!second_stage_translate(&l1_desc_addr2, l1_desc_addr, mva, size, inst_fetch, gdb_access))
        {
            printb(d_armv7a_vmsa, "tras_table_walk_sd stage 1 level 1 second stage translate fault");
            return false;
        }
    }

    bits l1_desc;
    bits l2_desc;
    bool s;
    uint32_t ap;
    bool ng;
    uint32_t texcb;
    bool xn;
    bool pxn;
    bool ns;
    uint32_t block_size;
    uint8_t physical_address_ext;
    uint64_t physical_address;

    _mem_read(&l1_desc, l1_desc_addr2, 4);

    //printd(d_s2mmu, "stage 1 l1_desc 0x%X:0x%X", l1_desc_addr2, l1_desc.val);

    switch(l1_desc(1, 0))
    {
        case B(00):
            // fault
            //printd(d_s2mmu, "stage 1 invalid");
            level = 1;
            if(!gdb_access)
            {
                if(inst_fetch)
                {
                    prefetch_abort(mva, ia, level, is_write, Translation,
                                   take_to_hyp_mode, stage2, ipa_valid, ldfsr_format, s2fs1_walk);
                }
                else
                {
                    data_abort(mva, ia, domain, level, is_write, Translation,
                               take_to_hyp_mode, stage2, ipa_valid, ldfsr_format, s2fs1_walk);
                }
                return false;
            }
            break;
        case B(01):
            //printd(d_s2mmu, "stage 1 page");
            //large page or small page
            domain = l1_desc(8, 5);
            level = 2 ;
            pxn = l1_desc(2);
            ns = l1_desc(3);

            l2_desc_addr.p_address.physical_address = (l1_desc(31, 10) << 10) |
                                                      (get_field(mva, 19, 12) << 2);
            l2_desc_addr.p_address.physical_address_ext = 0;
            l2_desc_addr.p_address.ns = is_secure() ? 0 : 1;
            l2_desc_addr.mem_attrs = l1_desc_addr.mem_attrs;

            //if(!have_virt_ext() || is_secure()) //code from ARM Ref Manual, but we slightly modified here
            if(false)
            {
                l2_desc_addr2 = l2_desc_addr;
            }
            else
            {
                if(!second_stage_translate(&l2_desc_addr2, l2_desc_addr, mva, size, inst_fetch, gdb_access))
                {
                    printb(d_armv7a_vmsa, "tras_table_walk_sd stage 1 level 2 second stage translate fault");
                    return false;
                }
            }

            _mem_read(&l2_desc, l2_desc_addr2, 4);

            if(l2_desc(1, 0) == B(00))
            {
                //???
                if(!gdb_access)
                {
                    if(inst_fetch)
                    {
                        prefetch_abort(mva, ia, level, is_write, Translation,
                                       take_to_hyp_mode, stage2, ipa_valid, ldfsr_format, s2fs1_walk);
                    }
                    else
                    {
                        data_abort(mva, ia, domain, level, is_write, Translation,
                                   take_to_hyp_mode, stage2, ipa_valid, ldfsr_format, s2fs1_walk);
                    }
                    return false;
                }
            }

            s = l2_desc(10);
            ap = (l2_desc(9) << 2) | l2_desc(5, 4);
            ng = l2_desc(11);

            if((cp15.read(SCTLR_AFE) == 1) && (l2_desc(4) == 0))
            {
                if(cp15.read(SCTLR_HA) == 0)
                {
                    printb(d_armv7a_vmsa, "trans_walk_sd l2_desc abort 2");
                    return false;
                }
                else
                {
                    printb(d_armv7a_vmsa, "trans_walk_sd l2_desc error");
                    return false;
                }
            }

            //large page
            if(l2_desc(1) == 0)
            {
                texcb = (l2_desc(14, 12) << 2) | l2_desc(3, 2);
                xn = l2_desc(15);
                block_size = 64;
                physical_address_ext = 0;
                physical_address = (l2_desc(31, 16) << 16) | get_field(mva, 15, 0);
            }
            //small page
            else
            {
                texcb = (l2_desc(8, 6) << 2) | l2_desc(3, 2);
                xn = l2_desc(0);
                block_size = 4;
                physical_address_ext = 0;
                physical_address = (l2_desc(31, 12) << 12) | get_field(mva, 11, 0);
            }
            break;
        case B(10):
        case B(11):
            printd(d_s2mmu, "stage 1 section");
            //section or supersection
            texcb = (l1_desc(14, 12) << 2) | l1_desc(3, 2);
            s = l1_desc(16);
            ap = (l1_desc(15) << 2) | l1_desc(11, 10);
            xn = l1_desc(4);
            pxn = l1_desc(0);
            ng = l1_desc(17);
            level = 1;
            ns = l1_desc(19);

            if((cp15.read(SCTLR_AFE) == 1) && ((l1_desc(10) == 0)))
            {
                if(cp15.read(SCTLR_HA) == 0)
                {
                    printb(d_armv7a_vmsa, "section data abort");
                    return false;
                }
                else
                {
                    printb(d_armv7a_vmsa, "section error");
                    return false;
                }
            }

            //section
            if(l1_desc(18) == 0)
            {
                domain = l1_desc(8, 5);
                block_size = 1024;
                physical_address_ext = 0;
                physical_address = (l1_desc(31, 20) << 20) | get_field(mva, 19, 0);
            }
            //supersection
            else
            {
                domain = 0;
                block_size = 32768;
                physical_address_ext = (l1_desc(8, 5) << 4) || l1_desc(23, 20);
                physical_address = (l1_desc(31, 24) << 24) | get_field(mva, 23, 0);
            }
            break;
    }//end of switch

    if(cp15.read(SCTLR_TRE) == 0)
    {
        if(remap_regs_have_reset_values())
        {
            default_tex_decode(&(result->addr_desc.mem_attrs) , texcb, s);
        }
        else
        {
            printm_once(d_armv7a_vmsa, "trans_walk_sd imple. defined mem_attrs");
            remapped_tex_decode(&(result->addr_desc.mem_attrs) , texcb, s);
        }
    }
    else
    {
        if(cp15.read(SCTLR_M) == 0)
        {
            default_tex_decode(&(result->addr_desc.mem_attrs) , texcb, s);
        }
        else
        {
            remapped_tex_decode(&(result->addr_desc.mem_attrs) , texcb, s);
        }
    }

    result->perms.ap = ap;
    result->perms.xn = xn;
    result->perms.pxn = pxn;
    result->ng = ng;
    result->domain = domain;
    result->level = level;
    result->block_size = block_size;
    result->addr_desc.p_address.physical_address = ((uint64_t)physical_address_ext << 40) |
                                                   physical_address;
    result->addr_desc.p_address.ns = is_secure() ? ns : 1;

    if((result->addr_desc.mem_attrs.type == MemType_Device) ||
       (result->addr_desc.mem_attrs.type == MemType_SO))
    {
        if(mva != align(mva, size))
        {
            printb(d_armv7a_vmsa, "trans_walk_sd alignment fault");
            return false;
        }
    }

    return true;
}

bool armv7a::translation_table_walk_ld(tlb_record* result, uint64_t ia, uint32_t va, bool is_write, bool stage1, bool s2fs1_walk, uint32_t size, bool inst_fetch, bool gdb_access)
{
    /* You are not expected to understand this. */

    address_descriptor walk_addr;
    address_descriptor walk_addr2;

    uint32_t domain;
    bool ldfsr_format = true;
    uint64_t base_address = 0;
    bool base_found = false;
    bool disabled = false;

    bool lookup_secure;
    int32_t t0_size;
    uint32_t current_level;
    uint32_t ba_lower_bound;
    uint32_t start_bit;
    uint64_t t1_size;

    bool first_iteration;
    bool table_rw;
    bool table_user;
    bool table_xn;
    bool table_pxn;
    bool lookup_finished;
    bool block_translate;
    uint32_t offset;
    uint64_t ia_select;
    uint64_t lookup_address;
    uint64_t output_address;
    uint32_t attrs;

    //stage1 memory attributes (HYP and non-HYP)
    if(stage1)
    {
        //stage1 memory attributes (HYP)
        if(rf.current_mode_is_hyp())
        {
            printb(d_armv7a_vmsa, "HYP trans_walk_ld stage 1");
            lookup_secure = false;
            t0_size = cp15.read(HTCR_T0SZ);
            if((t0_size == 0) || is_zero(get_field(ia, 31, 32 - t0_size)))
            {
                current_level = (get_field(cp15.read(HTCR_T0SZ), 2, 1) == B(00)) ? 1 : 2;
                ba_lower_bound = 9 * current_level - t0_size - 4;
                base_address = get_field64(cp15.read64(HTTBR_LPAE), 39, ba_lower_bound) << ba_lower_bound;
                if(!is_zero64(get_field64(cp15.read64(HTTBR_LPAE), ba_lower_bound - 1, 3)))
                {
                    printb(d_armv7a_vmsa, "trans_walk ld error");
                }
                base_found = true;
                start_bit = 31 - t0_size;

                walk_addr.mem_attrs.type = MemType_Normal;
                uint32_t hints_attrs = convert_attrs_hints(cp15.read(HTCR_IRGN0));
                walk_addr.mem_attrs.inner_hints = get_field(hints_attrs, 3, 2);
                walk_addr.mem_attrs.inner_attrs = get_field(hints_attrs, 1, 0);
                hints_attrs = convert_attrs_hints(cp15.read(HTCR_ORGN0));
                walk_addr.mem_attrs.outer_hints = get_field(hints_attrs, 3, 2);
                walk_addr.mem_attrs.outer_attrs = get_field(hints_attrs, 1, 0);
                walk_addr.mem_attrs.shareable = get_bit(cp15.read(HTCR_SH0), 1) == 1;
                walk_addr.mem_attrs.outer_shareable = cp15.read(HTCR_SH0) == B(10);
                walk_addr.p_address.ns = 1;
            }
        }//end of stage1 memory attributes (HYP)
        //stage1 memory attributes (non-HYP)
        else
        {
            lookup_secure = is_secure();
            t0_size = cp15.read(TTBCR_T0SZ);
            if((t0_size == 0) || is_zero(get_field(ia, 31, 32 - t0_size)))
            {
                current_level = (get_field(cp15.read(TTBCR_T0SZ), 2, 1) == 0) ? 1 : 2;
                ba_lower_bound = 9 * current_level - t0_size - 4;
                base_address = get_field64(cp15.read64(TTBR0_LPAE), 39, ba_lower_bound) << ba_lower_bound;
                if(!is_zero64(get_field64(cp15.read64(TTBR0_LPAE), ba_lower_bound - 1, 3)))
                {
                    printb(d_armv7a_vmsa, "trans_walk_ld error 2");
                }
                base_found = true;
                disabled = cp15.read(TTBCR_EPD0) == 1;
                start_bit = 31 - t0_size;

                walk_addr.mem_attrs.type = MemType_Normal;
                uint32_t hints_attrs = convert_attrs_hints(cp15.read(TTBCR_IRGN0));
                walk_addr.mem_attrs.inner_hints = get_field(hints_attrs, 3, 2);
                walk_addr.mem_attrs.inner_attrs = get_field(hints_attrs, 1, 0);
                hints_attrs = convert_attrs_hints(cp15.read(TTBCR_ORGN0));
                walk_addr.mem_attrs.outer_hints = get_field(hints_attrs, 3, 2);
                walk_addr.mem_attrs.outer_attrs = get_field(hints_attrs, 1, 0);
                walk_addr.mem_attrs.shareable = get_bit(cp15.read(TTBCR_SH0), 1) == 1;
                walk_addr.mem_attrs.outer_shareable = cp15.read(TTBCR_SH0) == B(10);
            }

            t1_size = cp15.read(TTBCR_T1SZ);

            if(((t1_size == 0) && !base_found) || (get_field(ia, 31, 32 - t1_size) == get_field(mask(31, 0), 31, 32 - t1_size)))
            {
                current_level = (get_field(cp15.read(TTBCR_T1SZ), 2, 1) == B(00)) ? 1 : 2;
                ba_lower_bound = 9 * current_level - t1_size - 4;
                base_address = get_field64(cp15.read64(TTBR1_LPAE), 39, ba_lower_bound) << ba_lower_bound;
                if(!is_zero64(get_field64(cp15.read64(TTBR1_LPAE), ba_lower_bound - 1, 3)))
                {
                    printb(d_armv7a_vmsa, "trans_walk_ld error 3");
                }
                base_found = true;
                disabled = cp15.read(TTBCR_EPD1) == 1;
                start_bit = 31 - t1_size;

                walk_addr.mem_attrs.type = MemType_Normal;
                uint32_t hints_attrs = convert_attrs_hints(cp15.read(TTBCR_IRGN1));
                walk_addr.mem_attrs.inner_hints = get_field(hints_attrs, 3, 2);
                walk_addr.mem_attrs.inner_attrs = get_field(hints_attrs, 1, 0);
                hints_attrs = convert_attrs_hints(cp15.read(TTBCR_ORGN1));
                walk_addr.mem_attrs.outer_hints = get_field(hints_attrs, 3, 2);
                walk_addr.mem_attrs.outer_attrs = get_field(hints_attrs, 1, 0);
                walk_addr.mem_attrs.shareable = get_bit(cp15.read(TTBCR_SH1), 1) == 1;
                walk_addr.mem_attrs.outer_shareable = cp15.read(TTBCR_SH1) == B(10);
            }
        }//end of stage1 memory attributes (non-HYP)
    }//end of stage1 memory attributes
    //stage2 memory attributes
    else
    {
        //printd(d_s2mmu, "trans_walk_ld stage 2, ia=0x%08X%08X", (uint32_t)ia >> 32, (uint32_t)ia);

        t0_size = cp15.read(VTCR_T0SZ);
        //4-bit sign extend hack
        if(get_bit(t0_size, 3) == 1)
        {
            t0_size &= mask(31, 4);
        }
        uint32_t s_level  = cp15.read(VTCR_SL0);
        ba_lower_bound = 14 - t0_size - 9 * s_level;

        //printd(d_s2mmu, "trans_walk_ld start level = %d", s_level);
        //printd(d_s2mmu, "trans_walk_ld VTTBR=0x%X", (uint32_t)cp15.read64(VTTBR_LPAE));

#ifdef CPU_ASSERT
        if((s_level == 0) && (t0_size < -2))
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 4");
        }
        if((s_level == 1) && (t0_size > 1))
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 5");
        }
        if(get_bit(cp15.read(VTCR_SL0), 1) == 1)
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 6");
        }
        if(is_zero64(get_field64(cp15.read64(VTTBR_LPAE), ba_lower_bound - 1, 3)) == false)
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 7");
        }
#endif

        if((t0_size == -8) || (is_zero64(get_field64(ia, 39, 32 - t0_size))))
        {
            current_level = 2 - s_level;
            base_address = get_field64(cp15.read64(VTTBR_LPAE), 39, ba_lower_bound) << ba_lower_bound;
            base_found = true;
            start_bit = 31 - t0_size;
        }
        //printd(d_s2mmu, "trans_walk_ld t0_size = %d", t0_size);
        //printd(d_s2mmu, "trans_walk_ld base_addr = 0x%X", (uint32_t)base_address);
        //printd(d_s2mmu, "trans_walk_ld current_level = %d", current_level);

        lookup_secure = false;

        walk_addr.mem_attrs.type = MemType_Normal;
        uint32_t hints_attrs = convert_attrs_hints(cp15.read(VTCR_IRGN0));
        walk_addr.mem_attrs.inner_hints = get_field(hints_attrs, 3, 2);
        walk_addr.mem_attrs.inner_attrs = get_field(hints_attrs, 1, 0);
        hints_attrs = convert_attrs_hints(cp15.read(VTCR_ORGN0));
        walk_addr.mem_attrs.outer_hints = get_field(hints_attrs, 3, 2);
        walk_addr.mem_attrs.outer_attrs = get_field(hints_attrs, 1, 0);
        walk_addr.mem_attrs.shareable = get_bit(cp15.read(VTCR_SH0), 1) == 1;
        walk_addr.mem_attrs.outer_shareable = cp15.read(VTCR_SH0) == B(10);
    }//end of stage2 memory attributes

    //base address not found ???
    if(!base_found || disabled)
    {
        if(!stage1)
        {
            printb(d_armv7a_vmsa, "stage 2 base addr not found");
        }
        bool take_to_hyp_mode = rf.current_mode_is_hyp() || !stage1;
        uint32_t level = 0;
        bool ipa_valid = true;
        printb(d_armv7a_vmsa, "trans_walk_ld data abort");
        data_abort(va, ia, domain, level, is_write, Translation, take_to_hyp_mode, !stage1,
                   ipa_valid, ldfsr_format, s2fs1_walk);
        return false;
    }

    first_iteration = true;
    table_rw = true;
    table_user = true;
    table_xn = false;
    table_pxn = false;

    //starting translation table walk (up to 3-level)
    while(1)
    {
        lookup_finished = true;
        block_translate = false; //true means last level of translation (level 1 and level 2 block, or level 3 page)
        offset = 9 * current_level;

        printd(d_s2mmu, "translating Level #%d", current_level);

        if(first_iteration)
        {
            ia_select = get_field64(ia, start_bit, 39 - offset) << 3;
            printd(d_s2mmu, "ia_select[%d:%d] = 0x%X", start_bit, 39 - offset, ia_select);
        }
        else
        {
            ia_select = get_field64(ia, 47 - offset, 39 - offset) << 3;
            printd(d_s2mmu, "ia_select[%d:%d] = 0x%X", 47 - offset, 39 - offset, ia_select);
        }

        lookup_address = base_address | ia_select;

        first_iteration = false;

        walk_addr.p_address.physical_address = get_field64(lookup_address, 39, 0);

        printd(d_s2mmu, "trans_walk_ld level %d desc_addr = 0x%X", current_level, (uint32_t)walk_addr.p_address.physical_address);

        if(lookup_secure)
        {
            walk_addr.p_address.ns = 0;
        }
        else
        {
            walk_addr.p_address.ns = 1;
        }

        bits64 descriptor;
        if(!have_virt_ext() && !stage1 || is_secure() || rf.current_mode_is_hyp())
        {
            _mem_read64(&descriptor, walk_addr, 8);
        }
        else
        {
            if(!second_stage_translate(&walk_addr2, walk_addr, get_field64(ia, 31, 0),
                                       size, inst_fetch, gdb_access))
            {
                if(!stage1)
                {
                    printb(d_armv7a_vmsa, "stage 2 trans walk ld error 8");
                }
                printb(d_armv7a_vmsa, "trans_walk_ld error 8");
                return false;
            }
            _mem_read64(&descriptor, walk_addr2, 8);
        }

        printd(d_s2mmu, "desc = 0x%08X%08X", (uint32_t)(descriptor.val >> 32), (uint32_t)descriptor.val);

        //invalid in all levels
        if(descriptor(0) == 0)
        {
            printd(d_s2mmu, "invalid entry");
            bool take_to_hyp_mode = rf.current_mode_is_hyp() || !stage1;
            bool ipa_valid = true;
            if(!gdb_access)
            {
                if(inst_fetch)
                {
                    printb(d_s2mmu, "trans_walk_ld pabort desc(0)==0, ipa=0x%X, take_to_hyp=%d", ia, take_to_hyp_mode);
                }
                else
                {
                    printd(d_s2mmu, "trans_walk_ld dabort desc(0)==0, ipa=0x%X, take_to_hyp=%d", ia, take_to_hyp_mode);
                    data_abort(va, ia, domain, current_level, is_write, Translation,
                               take_to_hyp_mode, !stage1, ipa_valid, ldfsr_format, s2fs1_walk);
                }
            }
            return false;
        }
        else
        {
            //level 1 / level 2 block
            if(descriptor(1) == 0)
            {
                if(current_level == 3)
                {
                    printd(d_s2mmu, "level 3 invalid");
                    printb(d_armv7a_vmsa, "trans_walk_ld dabort level 3 desc(1)==0");
                    bool take_to_hyp_mode = rf.current_mode_is_hyp() || !stage1;
                    bool ipa_valid = true;
                    data_abort(va, ia, domain, current_level, is_write, Translation,
                               take_to_hyp_mode, !stage1, ipa_valid, ldfsr_format, s2fs1_walk);
                    return false;
                }
                else
                {
                    printd(d_s2mmu, "level %d block", current_level);
                    block_translate = true;
                }
            }
            //level 1 table / level 2 table / level 3 page
            else
            {
                //level 3 page
                if(current_level == 3)
                {
                    printd(d_s2mmu, "level 3 page");
                    block_translate = true;
                }
                //level 1 table / level 2 table
                else
                {
                    printd(d_s2mmu, "level %d table", current_level);
                    base_address = descriptor(39, 12) << 12;
                    lookup_secure = lookup_secure && (descriptor(63) == 0);
                    table_rw = table_rw && (descriptor(62) == 0);
                    table_user = table_user && (descriptor(61) == 0);
                    table_pxn = table_pxn && (descriptor(59) == 1);
                    table_xn = table_xn && (descriptor(60) == 1);
                    lookup_finished = false;
                }
            }
        }


        if(block_translate) //last level (block or page)
        {
            output_address = (descriptor(39, 39 - offset) << (39 - offset)) | get_field64(ia, 38 - offset, 0);
            attrs = (descriptor(54, 52) << 10) | descriptor(11, 2);

            if(stage1)
            {
                if(table_xn)
                {
                    set_bit(&attrs, 12, 1);
                }
                if(table_pxn)
                {
                    set_bit(&attrs, 11, 1);
                }
                if(is_secure() && !lookup_secure)
                {
                    set_bit(&attrs, 9, 1);
                }
                if(!table_rw)
                {
                    set_bit(&attrs, 5, 1);
                }
                if(!table_user)
                {
                    set_bit(&attrs, 4, 0);
                }
                if(!lookup_secure)
                {
                    set_bit(&attrs, 3, 1);
                }
            }
        }
        else
        {
            current_level = current_level + 1;
        }

        //printd(d_s2mmu, "done translating current Level");

        if(lookup_finished)
        {
            break;
        }
    }//end of translation table walk

    //printd(d_s2mmu , "translation done");

    //check access flag
    if(get_bit(attrs, 8) == 0)
    {
        printb(d_armv7a_vmsa, "trans_walk_ld attr[8]==0");
        bool take_to_hyp_mode = rf.current_mode_is_hyp() || !stage1;
        bool ipa_valid = true;
        data_abort(va, ia, domain, current_level, is_write, AccessFlag, take_to_hyp_mode,
                   !stage1, ipa_valid, ldfsr_format, s2fs1_walk);
        return false;
    }

    //set access permissions
    result->perms.xn = get_bit(attrs, 12);
    result->perms.pxn = get_bit(attrs, 11);
    result->contiguous_hint = get_bit(attrs, 10);
    result->ng = get_bit(attrs, 9);

    set_field(&(result->perms.ap), 2, 1, get_field(attrs, 5, 4));

    set_bit(&(result->perms.ap), 0, 1);

    if(stage1)
    {
        mair_decode(&(result->addr_desc.mem_attrs), get_field(attrs, 2, 0));
    }
    else
    {
        s2_attr_decode(&(result->addr_desc.mem_attrs), get_field(attrs, 3, 0));
    }

    if((result->addr_desc.mem_attrs.type == MemType_Device) || (result->addr_desc.mem_attrs.type == MemType_SO))
    {
        if(va != align(va, size))
        {
            printb(d_armv7a_vmsa, "trans_walk_ld alignment fault");
            bool take_fault_in_hyp_mode = !stage1 || rf.current_mode_is_hyp();
            //alignment_fault_v(va, false, take_fault_in_hyp_mode);
        }
    }

    if(result->addr_desc.mem_attrs.type == MemType_Normal)
    {
        result->addr_desc.mem_attrs.shareable = get_bit(attrs, 7) == 1;
        result->addr_desc.mem_attrs.outer_shareable = get_field(attrs, 7, 6) == B(10);
    }
    else
    {
        result->addr_desc.mem_attrs.shareable = true;
        result->addr_desc.mem_attrs.outer_shareable = true;
    }

    result->level = current_level;
    result->block_size = (uint32_t)pow((long double)512, 3 - (int32_t)current_level) * 4;
    result->addr_desc.p_address.physical_address = get_field64(output_address, 39, 0);

    printd(d_s2mmu, "stage 2 va = 0x%X, pa = 0x%X", va, (uint32_t)output_address);

    if(stage1)
    {
        result->addr_desc.p_address.ns = get_bit(attrs, 3);
    }
    else
    {
        result->addr_desc.p_address.ns = 1;
    }

#ifdef CPU_ASSERT
    if(stage1 && rf.current_mode_is_hyp())
    {
        if(get_bit(attrs, 4) != 1)
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 9");
        }
        if(!table_user)
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 10");
        }
        if(get_bit(attrs, 11) != 0)
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 11");
        }
        if(!table_pxn)
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 12");
        }
        if(get_bit(attrs, 9) != 0)
        {
            printb(d_armv7a_vmsa, "trans_walk_ld error 13");
        }
    }
#endif

    return true;

}

void armv7a::write_hsr(uint32_t ec, uint32_t hsr_string)
{
    printd(d_armv7a_vmsa, "write_hsr");

    uint32_t hsr_value = 0;

    set_field(&hsr_value, 31, 26, ec);

    if((get_field(ec, 5, 3) != B(100)) || ((get_bit(ec, 2) == 1) && (get_bit(hsr_string, 24) == 1)))
    {
        printm_once(d_armv7a_vmsa, "write_hsr inst length = 16/32, ignore 16 here");
        set_bit(&hsr_value, 25, 1);
    }

    if((get_field(ec, 5, 4) == B(00)) && (get_field(ec, 3, 0) != B(0000)))
    {
        if(rf.current_inst_set() == InstSet_ARM)
        {
            set_bit(&hsr_value, 24, 1);
            //printm_once(d_armv7a_vmsa, "ignore writing condition to HSR");
            //printb(d_armv7a_vmsa, "write_hsr missing condition code");
            set_field(&hsr_value, 23, 20, get_field(inst.val, 31, 28));
        }
        else
        {
            printb(d_armv7a_vmsa, "write_hsr thumb");
        }

        set_field(&hsr_value, 19, 0, get_field(hsr_string, 19, 0));
    }
    else
    {
        set_field(&hsr_value, 24, 0, hsr_string);
    }

    cp15.write(HSR, hsr_value);
}

