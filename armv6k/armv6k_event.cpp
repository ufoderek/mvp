#include <armv6k.h>

void ARMV6K::send_event()
{
    printm(core_id, d_inst, "sev");
    scu->event_reg = true;
}

