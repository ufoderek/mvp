#include <cstring>
#include <iostream>
#include <sysctrl_1.h>

using namespace std;

SYSCTRL_1::SYSCTRL_1(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    led = 0;
    lock = 0;
    counter_24M = 0;
    counter_100 = 0;
    SC_METHOD(run);
    sensitive << clk.pos();
}

void SYSCTRL_1::run(void)
{
    /********************************************************************************
    * NOTE:
    * Originally, the clock rate is running under 100Hz & 24MHz
    * but our simulation clock rate is 4MHz
    * The incremental value should be fixed here
    ********************************************************************************/
    /* the 100Hz system counter */
    static int div = 0;

    if(div >= 40000)
    {
        counter_100++;
        div = 0;
    }
    else
    {
        div++;
    }

    /* the 24MHz system counter */
    counter_24M += 6;
}

bool SYSCTRL_1::read_1(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch(addr)
    {
        case 0x0000:
            *data = 0;
            break;
        case 0x0fe0:
            *data = 0x80;
            break;
        case 0x0fe4:
            *data = 0x11;
            break;
        case 0x0fe8:
            *data = 0x04;
            break;
        case 0x0fec:
            *data = 0x00;
            break;
        case 0x0ff0:
            *data = 0x0d;
            break;
        case 0x0ff4:
            *data = 0xf0;
            break;
        case 0x0ff8:
            *data = 0x05;
            break;
        case 0x0ffc:
            *data = 0xb1;
            break;
        default:
            printb(d_sysctrl_1, "read unknow: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

/* write registers of the system SYSCTRL_1 module */
bool SYSCTRL_1::write_1(uint32_t data, uint32_t addr, int size)
{
    bool result = true;

    switch(addr)
    {
        case 0x0000:
            break;
        default:
            printb(d_sysctrl_1, "write unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

bool SYSCTRL_1::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        return this->write_1(data, local_address, length);
    }
    else
    {
        return this->read_1(&data, local_address, length);
    }
}
