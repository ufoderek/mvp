#include <cstring>
#include <iostream>
#include <timer.h>

using namespace std;

TIMER::TIMER(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    /* initialize all registers */
    Timer0Load = 0;
    Timer1Load = 0;
    Timer0Value = 0xffffffff;
    Timer0Value = 0xffffffff;
    Timer0Control = 0x0020;
    Timer1Control = 0x0020;
    Timer0Enable = false;
    Timer1Enable = false;
    Timer0Mode = false; //free-running mode
    Timer1Mode = false; //free-running mode
    Timer0Int = true;
    Timer1Int = true;
    Timer0Size = false; //16-bits mode
    Timer1Size = false; //16-bits mode
    Timer0One = false;
    Timer1One = false;
    irq0_n = true;
    irq1_n = true;

    SC_METHOD(run);
    sensitive << clk.pos();
}

void TIMER::run(void)
{
    if(Timer0Enable)
    {
        uint32_t count = (Timer0Size) ? (Timer0Value & 0xffffffff) : (Timer0Value & 0x0000ffff);
        //count = (count > 0x00050000)? (count >> 1) : (count - 1);
        count--;

        if((count == 0))
        {
            Timer0Value =
                (Timer0Mode) ?
                ((Timer0Size) ? (Timer0Load) : ((Timer0Value & 0xffff0000) | (Timer0Load & 0x0000ffff))) : //periodic
                    ((Timer0Size) ? (0xffffffff) : (Timer0Value & 0xffff0000 | 0x0000ffff));                        //free-running
            Timer0Enable = (Timer0One) ? false : true;
            irq0_n = (Timer0Int) ? false : true;
        }
        else
    {
            Timer0Value = (Timer0Size) ? (count) : ((Timer0Value & 0xffff0000) | (count & 0x0000ffff));
            irq0_n = irq0_n;
        }
    }

    if(Timer1Enable)
    {
        uint32_t count = (Timer1Size) ? (Timer1Value & 0xffffffff) : (Timer1Value & 0x0000ffff);
        //count = (count > 0x00050000)? (count >> 1) : (count - 1);
        count--;

        if((count == 0))
        {
            Timer1Value =
                (Timer1Mode) ?
                ((Timer1Size) ? (Timer1Load) : ((Timer1Value & 0xffff0000) | (Timer1Load & 0x0000ffff))) : //periodic
                    ((Timer1Size) ? (0xffffffff) : (Timer1Value & 0xffff0000 | 0x0000ffff));                        //free-running
            Timer1Enable = (Timer1One) ? false : true;
            irq1_n = (Timer1Int) ? false : true;
        }
        else
    {
            Timer1Value = (Timer1Size) ? (count) : ((Timer1Value & 0xffff0000) | (count & 0x0000ffff));
            irq1_n = irq1_n;
        }
    }

    irq_n = irq0_n & irq1_n;
}

/* read registers of the SP804 Timer module */
bool TIMER::read(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch(addr)
    {
            /* timer 0 */
        case 0x0000:
            *data = Timer0Load;
            break;
        case 0x0004:
            *data = Timer0Value;
            printd(d_timer, "timer0 read: 0x%X", Timer0Value);
            break;
            /* timer 1 */
        case 0x0020:
            *data = Timer1Load;
            break;
        case 0x0024:
            *data = Timer1Value;
            printd(d_timer, "timer1 read: 0x%X", Timer1Value);
            break;
            /* general */
        default:
            printb(d_timer, "read unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

/* write registers of the SP804 Timer module */
bool TIMER::write(uint32_t data, uint32_t addr, int size)
{
    bool result = true;

    switch(addr)
    {
            /* timer 0 */
        case 0x0000:
            Timer0Load = (Timer0Size) ? (data) : ((Timer0Load & 0xffff0000) | (data & 0x0000ffff));
            Timer0Value = data;
            printd(d_timer, "value = 0x%X", Timer0Value);
            break;
        case 0x0004:
            break;
        case 0x0008:
            Timer0Control = data;
            Timer0Enable = (data & 0x0080) ? true : false;
            Timer0Mode = (data & 0x0040) ? true : false;
            Timer0Int = (data & 0x0020) ? true : false;
            Timer0Size = (data & 0x0002) ? true : false;
            Timer0One = (data & 0x0001) ? true : false;
            printd(d_timer, "timer0 %s", Timer0Enable ? "on" : "off");
            break;
        case 0x000c:
            irq0_n = true;
            break;
            /* timer 1 */
        case 0x0020:
            Timer1Load = (Timer1Size) ? (data) : ((Timer1Load & 0xffff0000) | (data & 0x0000ffff));
            Timer1Value = data;
            break;
        case 0x0024:
            break;
        case 0x0028:
            Timer1Control = data;
            Timer1Enable = (data & 0x0080) ? true : false;
            Timer1Mode = (data & 0x0040) ? true : false;
            Timer1Int = (data & 0x0020) ? true : false;
            Timer1Size = (data & 0x0002) ? true : false;
            Timer1One = (data & 0x0001) ? true : false;
            printd(d_timer, "timer1 %s", Timer1Enable ? "on" : "off");
            break;
        case 0x002c:
            irq1_n = true;
            break;
            /* general */
        default:
            printb(d_timer, "write unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

bool TIMER::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        success = this->write(data, local_address, length);
        printd(d_timer, "write to phy address %u: %u", addr, data);
    }
    else
    {
        success = this->read(&data, local_address, length);
        printd(d_timer, "read from phy address %u: %u", addr, data);
    }

    return success;
}

