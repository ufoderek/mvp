/********************************************************************************
 * MVP(Multicore Virtual Platform)
 * (C) Copyright CASLab.EE.NCKU 2009
 * All Right Reserved
 *
 * the ARM PL093 synchronous static memory controller module of MVP emulator
 *
 * Shin-Ying Lee(sing@casmail.ee.ncku.edu.tw)
 * Computer Architecture & System Laboratory
 * Dept. of Electrical Engineering & Inst. of Computer and Communication Engineering
 * National Cheng Kung University, Tainan, Taiwan
 * 2009.11.20
 ********************************************************************************/

/* import header files */
#include<cstring>
#include<ssmc.h>

using namespace std;

/* the constructor */
SSMC::SSMC(sc_module_name name, uint32_t mapping_size) : ahb_slave_if(mapping_size), sc_module(name)
{
}

/* read registers of the SSMC module */
bool SSMC::read(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        case 0x0fe0:
            *data = 0x93;
            break;
        case 0x0fe4:
            *data = 0x10;
            break;
        case 0x0fe8:
            *data = 0x14;
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
            printb(d_ssmc, "read unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

/* write registers of the SSMC module */
bool SSMC::write(uint32_t data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
            //case 0x0E000050:
            //  break;
        default:
            printb(d_ssmc, "write unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

bool SSMC::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if (write) {
        printd(d_ssmc, "write to phy address 0x%X: 0x%X", addr, data);
        success = this->write(data, local_address, length);
    }

    else {
        printd(d_ssmc, "read from phy address 0x%X: 0x%X", addr, data);
        success = this->read(&data, local_address, length);
    }

    return success;
}

