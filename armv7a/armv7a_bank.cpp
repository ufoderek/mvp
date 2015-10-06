#include <armv7a.h>
#include <bit_opt_v2.h>

void armv7a::banked_register_access_valid(uint32_t sysm, uint32_t mode)
{
    if(get_field(sysm, 4, 3) == B(00))
    {
        if(get_field(sysm, 2, 0) == B(111))
        {
            printb(d_armv7a_bank, "error 1");
        }
        else if(get_field(sysm, 2, 0) == B(110))
        {
            if((mode == B(11010)) || (mode == B(11111)))
            {
                printb(d_armv7a_bank, "error 2");
            }
        }
        else if(get_field(sysm, 2, 0) == B(101))
        {
            if(mode == B(11111))
            {
                printb(d_armv7a_bank, "error 3");
            }
        }
        else if(mode != B(10001))
        {
            printb(d_armv7a_bank, "error 4");
        }
    }
    else if(get_field(sysm, 4, 3) == B(01))
    {
        if((get_field(sysm, 2, 0) == B(111)) || (mode == B(10001)) || (/*(cp15.read(SCR_RFR) == 1) &&*/ !is_secure()))
        {
            printb(d_armv7a_bank, "error 5");
        }
    }
    else if(get_field(sysm, 4, 3) == B(11))
    {
        if(get_bit(sysm, 2) == 0)
        {
            printb(d_armv7a_bank, "error 6");
        }
        else if(get_bit(sysm, 1) == 0)
        {
            if(!is_secure() || (mode == B(10110)))
            {
                printb(d_armv7a_bank, "error 7");
            }
        }
        else
        {
            if(mode != B(10110))
            {
                /* If this comment is removed the program will blow up
                printb(d_armv7a_bank, "error 8");
                */
            }
        }
    }
}

void armv7a::spsr_access_valid(uint32_t sysm, uint32_t mode)
{
    switch(sysm)
    {
        case B(01110):
            if((!is_secure() /* && (cp15.read(SCR_RFR) == 1)*/) || (mode == B(10001)))
            {
                printb(d_armv7a_bank, "error 9");
            }
            break;
        case B(10000):
            if(mode == B(10010))
            {
                printb(d_armv7a_bank, "error 10");
            }
            break;
        case B(10010):
            if(mode == B(10011))
            {
                printb(d_armv7a_bank, "error 11");
            }
            break;
        case B(10100):
            if(mode == B(10111))
            {
                printb(d_armv7a_bank, "error 12");
            }
            break;
        case B(10110):
            if(mode == B(11011))
            {
                printb(d_armv7a_bank, "error 13");
            }
            break;
        case B(11100):
            if((mode == B(10110)) || !is_secure())
            {
                printb(d_armv7a_bank, "error 14");
            }
            break;
        case B(11110):
            if(mode == B(10110))
            {
                printb(d_armv7a_bank, "error 15");
            }
            break;
        default:
            printb(d_armv7a_bank, "error 15");
            break;
    }//end switch
}

