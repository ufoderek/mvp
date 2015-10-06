#include <armv7a.h>
#include <bit_opt_v2.h>

uint32_t armv7a::arch_version()
{
    return 7;
}

bool armv7a::have_lpae()
{
    return true;
}

bool armv7a::have_mp_ext()
{
    return false;
}

bool armv7a::have_security_ext()
{
    return true;
}

bool armv7a::have_virt_ext()
{
    return true;
}

bool armv7a::is_secure()
{
    return true;
}

mem_arch armv7a::memory_system_architecture()
{
    return MemArch_VMSA;
}

bool armv7a::remap_regs_have_reset_values()
{
    return false;
}

bool armv7a::tlb_lookup_came_from_cache_maintenance()
{
    return false;
}

bool armv7a::unaligned_support()
{
    return true;
}

