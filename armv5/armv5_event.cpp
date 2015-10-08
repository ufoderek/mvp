#include <armv5.h>

void ARMV5::shared_event_listener()
{
    wakup_events();
}

void ARMV5::irq_n_listener()
{
    if ((irq_n == false) && (!CPSR_I(rf.cpsr))) {
        wakup_events();
    }
}

void ARMV5::wakup_events()
{
    local_event.notify(SC_ZERO_TIME);
    event_reg = true;
}

void ARMV5::send_event()
{
    ARMV5::shared_event.notify(SC_ZERO_TIME);
}

void ARMV5::wait_for_event()
{
    wait(local_event);
}

void ARMV5::clear_event_reg()
{
    event_reg = false;
}

bool ARMV5::event_registered()
{
    return event_reg;
}

