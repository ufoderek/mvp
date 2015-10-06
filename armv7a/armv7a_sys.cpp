#include <armv7a.h>
#include <bit_opt_v2.h>

void armv7a::cpsr_write_by_inst(uint32_t value, uint32_t bytemask, bool is_excpt_return)
{
    printd(d_armv7a_sys, "cpsr_write_by_inst");
    bool privileged = rf.current_mode_is_not_user();
    bool nmfi = cp15.read(SCTLR_NMFI) == 1;
    uint32_t cpsr = rf.cpsr();

    if(get_bit(bytemask, 3) == 1)
    {
        copy_field(&cpsr, value, 31, 27);

        if(is_excpt_return)
        {
            copy_field(&cpsr, value, 26, 24);
        }
    }

    if(get_bit(bytemask, 2) == 1)
    {
        copy_field(&cpsr, value, 19, 16);
    }

    if(get_bit(bytemask, 1) == 1)
    {
        if(is_excpt_return)
        {
            copy_field(&cpsr, value, 15, 10);
        }

        copy_bit(&cpsr, value, 9);

        if(privileged && (armv7a::is_secure() || (cp15.read(SCR_AW) == 1)) || armv7a::have_virt_ext())
        {
            copy_bit(&cpsr, value, 8);
        }
    }

    if(get_bit(bytemask, 0) == 1)
    {
        if(privileged)
        {
            copy_bit(&cpsr, value, 7);
        }

        if(privileged && (armv7a::is_secure() || (cp15.read(SCR_FW) == 1) || armv7a::have_virt_ext()) && (!nmfi || (get_bit(value, 6) == 0)))
        {
            copy_bit(&cpsr, value, 6);
        }

        if(is_excpt_return)
        {
            copy_bit(&cpsr, value, 5);
        }

        if(privileged)
        {
#ifdef CPU_ASSERT
            if(rf.bad_mode(get_field(value, 4, 0)))
            {
                printb(d_armv7a_sys, "cpsr_write_by_inst error 0");
            }
            if(!armv7a::is_secure() && (get_field(value, 4, 0) == B(10110)))
            {
                printb(d_armv7a_sys, "cpsr_write_by_inst error 1");
            }
            /*
            if(!armv7a::is_secure() && (get_field(value, 4, 0) == B(10001)) && (cp15.nsacr_RFR() == 1))  {
                printb(d_armv7a_sys, "cpsr_write_by_inst error 2");
            }
            */
            if((cp15.read(SCR_NS) == 0) && (get_field(value, 4, 0) == B(11010)))
            {
                printb(d_armv7a_sys, "cpsr_write_by_inst error 3");
            }
            if(!armv7a::is_secure() && (rf.cpsr_M() != B(11010)) && (get_field(value, 4, 0) == B(11010)))
            {
                printb(d_armv7a_sys, "cpsr_write_by_inst error 4");
            }
            if((rf.cpsr_M() == B(11010)) && (get_field(value, 4, 0) != B(11010)) && !is_excpt_return)
            {
                printb(d_armv7a_sys, "cpsr_write_by_inst error 5");
            }
#endif
            copy_field(&cpsr, value, 4, 0);
        }
    }

    rf.cpsr(cpsr);
}

void armv7a::spsr_write_by_inst(uint32_t value, uint32_t bytemask)
{
#ifdef CPU_ASSERT
    if(rf.current_mode_is_user_or_system())
    {
        printb(d_armv7a_sys, "spsr_write_by_inst error");
    }
#endif

    uint32_t spsr = rf.spsr();

    if(get_bit(bytemask, 3) == 1)
    {
        copy_field(&spsr, value, 31, 24);
    }

    if(get_bit(bytemask, 2) == 1)
    {
        copy_field(&spsr, value, 19, 16);
    }

    if(get_bit(bytemask, 1) == 1)
    {
        copy_field(&spsr, value, 15, 8);
    }

    if(get_bit(bytemask, 0) == 1)
    {
        copy_field(&spsr, value, 7, 5);

#ifdef CPU_ASSERT
        if(rf.bad_mode(get_field(value, 4, 0)))
        {
            printb(d_armv7a_sys, "spsr_write_by_inst error 2");
        }
#endif
        copy_field(&spsr, value, 4, 0);
    }

    rf.spsr(spsr);
}

