#include <armv7a.h>
#include <bit_opt_v2.h>

void armv7a::_mem_read(bits* value, address_descriptor& mem_addr_desc, uint32_t size)
{
    //ARM Ref Manual B2-30
#ifdef CPU_ASSERT
    if(!((size == 1) || (size == 2) || (size == 4)))
    {
        printb(d_armv7a_cmsa, "_mem() read assert error, size = %d", size);
    }
#endif

    bool success;
    success = bus_read(&(value->val), mem_addr_desc.p_address.physical_address, size);
    value->n = size * 8;
    value->val &= mask(value->n - 1, 0);
    //printd(d_armv7a_cmsa, "_mem_read %X:%X" , mem_addr_desc.p_address.physical_address, value->val);
}

void armv7a::_mem_read64(bits64* value, address_descriptor& mem_addr_desc, uint32_t size)
{
    //ARM Ref Manual B2-30
#ifdef CPU_ASSERT
    if(!(size == 8))
    {
        printb(d_armv7a_cmsa, "_mem() read 64 assert error, size = %d", size);
    }
#endif

    bool success;
    success = bus_read64(&(value->val), mem_addr_desc.p_address.physical_address, size);
    value->n = size * 8;
    value->val &= mask64(value->n - 1, 0);
    //printd(d_armv7a_cmsa, "_mem_read %X:%X" , mem_addr_desc.p_address.physical_address, value->val);
}

void armv7a::_mem_write(address_descriptor& mem_addr_desc, uint32_t size, bits& value)
{
#ifdef CPU_ASSERT
    if(!((size == 1) || (size == 2) || (size == 4)))
    {
        printb(d_armv7a_cmsa, "_mem() write assert error, size = %d", size);
    }
#endif

    bool success;
    success = bus_write(value.val, mem_addr_desc.p_address.physical_address, size);
}

void armv7a::_mem_write64(address_descriptor& mem_addr_desc, uint32_t size, bits64& value)
{
#ifdef CPU_ASSERT
    if(!(size == 8))
    {
        printb(d_armv7a_cmsa, "_mem() write 64 assert error, size = %d", size);
    }
#endif

    bool success;
    success = bus_write64(value.val, mem_addr_desc.p_address.physical_address, size);
}

void armv7a::alignment_fault(uint32_t address, bool is_write)
{
    printb(d_armv7a_cmsa, "alignment fault");
}

uint32_t armv7a::big_endian_reverse(uint32_t value, uint32_t n)
{
    printb(d_armv7a_cmsa, "big_endian_reverse");
}

bool armv7a::check_permission(permissions perms, uint32_t mva, uint32_t level,
                              uint32_t domain, bool is_write, bool is_priv,
                              bool take_to_hyp_mode, bool ldfsr_format,
                              bool inst_fetch, bool gdb_access)
{
    bool second_stage_abort = false;
    bool ipa_valid = false;
    bool s2fs1_walk = false;
    uint64_t ipa = 0;

    if(cp15.read(SCTLR_AFE) == 1)
    {
        set_bit(&(perms.ap), 0, 1);
    }

    bool abort = false;

    switch(perms.ap)
    {
        case B(000):
            abort = true;
            break;
        case B(001):
            abort = !is_priv;
            break;
        case B(010):
            abort = !is_priv && is_write;
            break;
        case B(011):
            abort = false;
            break;
        case B(100):
            printb(d_armv7a_cmsa, "check_permission error");
            break;
        case B(101):
            abort = !is_priv || is_write;
            break;
        case B(110):
            abort = is_write;
            break;
        case B(111):
            if(armv7a::memory_system_architecture() == MemArch_VMSA)
            {
                abort = is_write;
            }
            else
            {
                printb(d_armv7a_cmsa, "check_permission not VMSA");
            }
            break;
        default:
            printb(d_armv7a_cmsa, "check_permission error 2, ap:%X", perms.ap);
            break;
    }

    if(abort)
    {
        if(inst_fetch)
        {
            prefetch_abort(mva, ipa, level, is_write, Permission, take_to_hyp_mode,
                           second_stage_abort, ipa_valid, ldfsr_format, s2fs1_walk);
        }
        else
        {
            data_abort(mva, ipa, domain, level, is_write, Permission, take_to_hyp_mode,
                       second_stage_abort, ipa_valid, ldfsr_format, s2fs1_walk);
        }
        return false;
    }

    return true;
}

void armv7a::data_abort(uint32_t v_address, uint64_t ip_address, uint32_t domain,
                        uint32_t level, bool is_write, fault_type type, bool take_to_hyp_mode,
                        bool second_stage_abort, bool ipa_valid, bool ldfsr_format, bool s2fs1_walk)
{
    //VMSA
    if(memory_system_architecture() == MemArch_VMSA)
    {
        if(!take_to_hyp_mode)
        {
            uint32_t dfsr = 0;
            uint32_t dfar = 0;

            if(!((type == AsyncParity) || (type == AsyncExternal) || (type == DebugEvent)))
            {
                dfar = v_address;
            }
            else if(type == DebugEvent)
            {
                printb(d_armv7a_cmsa, "data_abort debug event");
            }

            if(ldfsr_format)
            {
                printb(d_armv7a_cmsa, "data_abort ldfsr format");
                set_bit(&dfsr, 13, tlb_lookup_came_from_cache_maintenance());
                if((type == AsyncExternal) || (type == SyncExternal))
                {
                    printb(d_armv7a_cmsa, "data_abort external");
                    //set_bit(&dfsr, 12, 1);
                }
                else
                {
                    set_bit(&dfsr, 12, 0);
                }
                set_bit(&dfsr, 11, is_write ? 1 : 0);
                set_bit(&dfsr, 9, 1);
                set_field(&dfsr, 5, 0, encode_ld_fsr(type, level));
            }
            else
            {
                set_bit(&dfsr, 13, tlb_lookup_came_from_cache_maintenance());
                if((type == AsyncExternal) || (type == SyncExternal))
                {
                    printb(d_armv7a_cmsa, "data_abort external 2");
                    set_bit(&dfsr, 12, 1);
                }
                else
                {
                    set_bit(&dfsr, 12, 0);
                }
                set_bit(&dfsr, 11, is_write ? 1 : 0);
                set_bit(&dfsr, 9, 0);
                bool domain_valid = ((type == Domain) ||
                                     (level == 2) &&
                                     ((type == Translation) || (type == AccessFlag) ||
                                      (type == SyncExternalonWalk) || (type == SyncParityonWalk)) ||
                                     (!have_lpae() && (type == Permission)));
                if(domain_valid)
                {
                    set_field(&dfsr, 7, 4, domain);
                }
                else
                {
                }
                uint32_t fs = encode_sd_fsr(type, level);
                set_field(&dfsr, 3, 0, get_field(fs, 3, 0));
                set_bit(&dfsr, 10, get_bit(fs, 4));
            }

            cp15.write(DFAR, dfar);
            cp15.write(DFSR, dfsr);
        }
        //take to hyp
        else
        {
            printd(d_armv7a_cmsa, "data_abort, to hyp, va=0x%X, ipa=0x%X, s2abort=%d, s2fs1=%d, ipa_valid=%d", v_address, (uint32_t)ip_address, second_stage_abort, s2fs1_walk, ipa_valid);

            uint32_t hsr_string = 0;
            uint32_t ec;

            cp15.write(HDFAR, v_address);

            if(ipa_valid)
            {
                uint32_t hpfar = 0;
                set_field(&hpfar, 31, 4, (uint32_t)get_field64(ip_address, 39, 12));
                cp15.write(HPFAR, hpfar);
            }
            if(second_stage_abort)
            {
                ec = B(100100);
                set_field(&hsr_string, 24, 16, ls_instruction_syndrome());
            }
            else
            {
                ec = B(100101);
                set_bit(&hsr_string, 24, 0);
            }
            if((type == AsyncExternal) || (type == SyncExternal))
            {
                printb(d_armv7a_cmsa, "data_abort external 3");
                set_bit(&hsr_string, 9, 1);
            }
            else
            {
                set_bit(&hsr_string, 9, 0);
            }
            set_bit(&hsr_string, 8, tlb_lookup_came_from_cache_maintenance());
            set_bit(&hsr_string, 7 , s2fs1_walk ? 1 : 0);
            set_bit(&hsr_string, 6, is_write ? 1 : 0);
            set_field(&hsr_string, 5, 0, encode_ld_fsr(type, level));
            write_hsr(ec, hsr_string);

            cp15.write(HADFSR, inst.val); //imple. defined, so i put entire instruction here, let dirty work done by hypervisor
        }
    }
    //PMSA
    else
    {
        printb(d_armv7a_cmsa, "data_abort PMSA");
    }

    printd(d_armv7a_cmsa, "dabort, pc=0x%X", rf.current_pc());
    printd(d_armv7a_cmsa, "dfar=0x%X, dfsr=0x%X", cp15.read(DFAR), cp15.read(DFSR));
    take_data_abort_exception(type == Alignment, second_stage_abort);
}

void armv7a::default_tex_decode(memory_attributes* mem_attrs, uint32_t texcb, uint32_t s)
{
    switch(get_field(texcb, 4, 0))
    {
        case B(00000):
            mem_attrs->type = MemType_SO;
            mem_attrs->shareable = true;
            break;
        case B(00001):
            mem_attrs->type = MemType_Device;
            mem_attrs->shareable = true;
            break;
        case B(00010):
            mem_attrs->type = MemType_Normal;
            mem_attrs->inner_attrs = B(10);
            mem_attrs->inner_hints = B(10);
            mem_attrs->outer_attrs = B(10);
            mem_attrs->outer_hints = B(10);
            mem_attrs->shareable = s == 1;
            break;
        case B(00011):
            mem_attrs->type = MemType_Normal;
            mem_attrs->inner_attrs = B(11);
            mem_attrs->inner_hints = B(10);
            mem_attrs->outer_attrs = B(11);
            mem_attrs->outer_hints = B(10);
            mem_attrs->shareable = s == 1;
            break;
        case B(00100):
            mem_attrs->type = MemType_Normal;
            mem_attrs->inner_attrs = B(00);
            mem_attrs->inner_hints = B(00);
            mem_attrs->outer_attrs = B(00);
            mem_attrs->outer_hints = B(00);
            mem_attrs->shareable = s == 1;
            break;
        case B(00110):
            printb(d_armv7a_cmsa, "default_tex_decode impl defined");
            break;
        case B(00111):
            mem_attrs->type = MemType_Normal;
            mem_attrs->inner_attrs = B(11);
            mem_attrs->inner_hints = B(11);
            mem_attrs->outer_attrs = B(11);
            mem_attrs->outer_hints = B(11);
            mem_attrs->shareable = s == 1;
            break;
        case B(01000):
            mem_attrs->type = MemType_Device;
            mem_attrs->shareable = true;
            break;
        default:
            if(get_bit(texcb, 4) == 1)
            {
                uint32_t hints_attrs;

                mem_attrs->type = MemType_Normal;
                hints_attrs = convert_attrs_hints(get_field(texcb, 1, 0));
                mem_attrs->inner_attrs = get_field(hints_attrs, 1, 0);
                mem_attrs->inner_hints = get_field(hints_attrs, 3, 2);
                hints_attrs = convert_attrs_hints(get_field(texcb, 3, 2));
                mem_attrs->outer_attrs = get_field(hints_attrs, 1, 0);
                mem_attrs->outer_hints = get_field(hints_attrs, 3, 2);
                //mem_attrs->shareable = s == 1;
            }
            else
            {
                printb(d_armv7a_cmsa, "default tex decode unpredictable");
            }
            break;
    }

    mem_attrs->outer_shareable = mem_attrs->shareable;
}

uint32_t armv7a::encode_sd_fsr(fault_type type, uint32_t level)
{
    uint32_t result = 0;

    switch(type)
    {
            /*
            case AccessFlag:
                if(level == 1)
                {
                    set_field(&result, 4, 0, B(00011));
                }
                else
                {
                    set_field(&result, 4, 0, B(00110));
                }
                break;
                */
        case Alignment:
            set_field(&result, 4, 0, B(00001));
            break;
        case Permission:
            set_field(&result, 4, 2, B(011));
            set_bit(&result, 0, 1);
            set_bit(&result, 1, get_bit(level, 1));
            break;
        case Domain:
            set_field(&result, 4, 2, B(010));
            set_bit(&result, 0, 1);
            set_bit(&result, 1, get_bit(level, 1));
            break;
        case Translation:
            set_field(&result, 4, 2, B(001));
            set_bit(&result, 0, 1);
            set_bit(&result, 1, get_bit(level, 1));
            break;
            /*
            case SyncExternal:
            set_field(&result, 4, 0, B(01000));
            break;
            case SyncExternalonWalk:
            set_field(&result, 4, 2, B(011));
            set_bit(&result, 0, 0);
            set_bit(&result, 1, get_bit(level, 1));
            break;
            case SyncParity:
            set_field(&result, 4, 0, B(11001));
            break;
            case SyncParityonWalk:
            set_field(&result, 4, 2, B(111));
            set_bit(&result, 0, 0);
            set_bit(&result, 1, 1);
            break;
            case AsyncParity:
            set_field(&result, 4, 0, B(11000));
            break;
            case AsyncExternal:
            set_field(&result, 4, 0, B(10110));
            break;
            case DebugEvent:
            set_field(&result, 4, 0, B(00010));
            break;
            case TLBConflict:
            set_field(&result, 4, 0, B(10000));
            break;
            case Lockdown:
            set_field(&result, 4, 0, B(10100));
            break;
            case Coproc:
            set_field(&result, 4, 0, B(11010));
            break;
            case ICacheMaint:
            set_field(&result, 4, 0, B(00100));
            break;
            */
        default:
            printb(d_armv7a_cmsa, "encode_sdfsr error, type = %d", type);
            break;
    }

    return result;
}

uint32_t armv7a::encode_ld_fsr(fault_type type, uint32_t level)
{
    uint32_t result = 0;

    switch(type)
    {
            /*
            case AccessFlag:
                set_field(&result, 5, 2, B(0010));
                set_field(&result, 1, 0, get_field(level, 1, 0));
                break;
                */
        case Alignment:
            set_field(&result, 5, 0, B(10001));
            break;
        case Permission:
            set_field(&result, 5, 2, B(0011));
            set_field(&result, 1, 0 , get_field(level, 1, 0));
            break;
        case Translation:
            set_field(&result, 5, 2, B(0001));
            set_field(&result, 1, 0, get_field(level, 1, 0));
            break;
            /*
            case SyncExternal:
            set_field(&result, 5, 0, B(010000));
            break;
            case SyncExternalonWalk:
            set_field(&result, 5, 2, B(0101));
            set_field(&result, 1, 0, get_field(level, 1, 0));
            break;
            case SyncParity:
            set_field(&result, 5, 0, B(011000));
            break;
            case SyncParityonWalk:
            set_field(&result, 5, 2, B(0111));
            set_field(&result, 1, 0, get_field(level, 1, 0));
            break;
            case AsyncParity:
            set_field(&result, 5, 0, B(011001));
            break;
            case AsyncExternal:
            set_field(&result, 5, 0, B(010001));
            break;
            case DebugEvent:
            set_field(&result, 5, 0, B(100010));
            break;
            case TLBConflict:
            set_field(&result, 5, 0, B(110000));
            break;
            case Lockdown:
            set_field(&result, 5, 0, B(110100));
            break;
            case Coproc:
            set_field(&result, 5, 0, B(111010));
            break;
            */
        default:
            printb(d_armv7a_cmsa, "encode_ldfsr error, type = %d", type);
            break;
    }

    return result;
}

bool armv7a::mem_a_read(bits* value, uint32_t address, uint32_t size)
{
    return mem_a_with_priv_read(value, address, size, rf.current_mode_is_not_user());
}

bool armv7a::mem_a_with_priv_read(bits* value, uint32_t address, uint32_t size, bool privileged)
{
    uint32_t va;

    if(address == align(address, size))
    {
        va = address;
    }
    else if((cp15.read(SCTLR_A) == 1) || (cp15.read(SCTLR_U) == 1))
    {
        alignment_fault(address, false);
        return false;
    }
    else
    {
        va = align(address, size);
    }

    address_descriptor mem_addr_desc;

    if(!translate_address(&mem_addr_desc, va, privileged, false, size, false, false))
    {
        return false;
    }

    _mem_read(value, mem_addr_desc, size);
    printd(d_armv7a_cmsa, "mem_a_read %X:%X:%X", va, mem_addr_desc.p_address.physical_address, value->val);

    if(rf.cpsr_E() == 1)
    {
        value->val = big_endian_reverse(value->val, size);
    }

    return true;
}

bool armv7a::mem_a_write(uint32_t address, uint32_t size, const bits& value)
{
    return mem_a_with_priv_write(address, size, rf.current_mode_is_not_user(), value);
}

bool armv7a::mem_a_with_priv_write(uint32_t address, uint32_t size, bool privileged, const bits& value)
{
    uint32_t va;

    if(address == align(address, size))
    {
        va = address;
    }
    else if((cp15.read(SCTLR_A) == 1) || (cp15.read(SCTLR_U) == 1))
    {
        alignment_fault(address, false); //????
        return false;
    }
    else
    {
        va = align(address, size);
    }

    address_descriptor mem_addr_desc;

    if(!translate_address(&mem_addr_desc, va, privileged, true, size, false, false))
    {
        return false;
    }

    if(mem_addr_desc.mem_attrs.shareable)
    {
        clear_exclusive_by_address(mem_addr_desc.p_address, processor_id(), size);
    }

    bits new_value = value;

    if(rf.cpsr_E() == 1)
    {
        new_value.val = big_endian_reverse(value.val, size);
    }

    _mem_write(mem_addr_desc, size, new_value);
    printd(d_armv7a_cmsa, "mem_a_write %X:%X:%X", va, mem_addr_desc.p_address.physical_address, new_value.val);
    return true;
}

bool armv7a::mem_u_read(bits* value, uint32_t address, uint32_t size)
{
    return mem_u_with_priv_read(value, address, size, rf.current_mode_is_not_user());
}

bool armv7a::mem_u_with_priv_read(bits* value, uint32_t address, uint32_t size, bool privileged)
{
    if((cp15.read(SCTLR_A) == 0) && cp15.read(SCTLR_U) == 0)
    {
        address = align(address, size);
    }

    if(address == align(address, size))
    {
        return mem_a_with_priv_read(value, address, size, privileged);
    }
    else if(cp15.read(SCTLR_A) == 1)
    {
        alignment_fault(address, false);
        return false;
    }
    else
    {
        bits tmp;
        value->val = 0;
        value->n = size * 8;

        for(int i = 0; i <= (size - 1); i++)
        {
            if(!mem_a_with_priv_read(&tmp, address + i, 1, privileged))
            {
                return false;
            }
            set_field(&(value->val), 8 * i + 7, 8 * i, tmp.val);
        }

        if(rf.cpsr_E() == 1)
        {
            value->val = big_endian_reverse(value->val, size);
        }
    }

    return true;
}

bool armv7a::mem_u_unpriv_write(uint32_t address, uint32_t size, const bits& value)
{
    return mem_u_with_priv_write(address, size, false, value);
}

bool armv7a::mem_u_write(uint32_t address, uint32_t size, const bits& value)
{
    return mem_u_with_priv_write(address, size, rf.current_mode_is_not_user(), value);
}

bool armv7a::mem_u_with_priv_write(uint32_t address, uint32_t size, bool privileged, const bits& value)
{
    if((cp15.read(SCTLR_A) == 0) && (cp15.read(SCTLR_U) == 0))
    {
        address = align(address, size);
    }

    if(address == align(address, size))
    {
        return mem_a_with_priv_write(address, size, privileged, value);
    }
    else if(cp15.read(SCTLR_A) == 1)
    {
        alignment_fault(address, true);
        return false;
    }
    else
    {
        bits new_value = value;

        if(rf.cpsr_E() == 1)
        {
            new_value.val = big_endian_reverse(value.val, size);
        }

        bits tmp(0, 8);

        for(int i = 0; i <= (size - 1); i++)
        {
            tmp.val = new_value(8 * i + 7, 8 * i);

            if(!mem_a_with_priv_write(address + i, 1, privileged, tmp))
            {
                return false;
            }
        }
    }

    return true;
}

bool armv7a::translate_address(address_descriptor* desc, uint32_t va, bool is_priv, bool is_write, uint32_t size, bool inst_fetch, bool gdb_access)
{
    switch(armv7a::memory_system_architecture())
    {
        case MemArch_VMSA:
            return translate_address_v(desc, va, is_priv, is_write, size, inst_fetch, gdb_access);
            break;
        case MemArch_PMSA:
            printb(d_armv7a_cmsa, "translate_address PMSA");
            break;
        default:
            printb(d_armv7a_cmsa, "translate_address error");
            break;
    }

    return false;
}

