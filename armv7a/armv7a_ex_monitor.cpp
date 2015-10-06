#include <armv7a.h>

void armv7a::clear_exclusive_local(uint32_t processor_id)
{
}

bool armv7a::exclusive_monitor_pass(uint32_t address, uint32_t size)
{
    return true;
}

void armv7a::set_exclusive_monitor(uint32_t address, uint32_t size)
{
    printd(d_armv7a_ex_monitor, "set_exclusive_monitor");
}

void armv7a::clear_exclusive_by_address(const full_address& p_address, uint32_t processor_id, uint32_t size)
{
    printd(d_armv7a_ex_monitor, "clear_exclusive_by_address");
}

