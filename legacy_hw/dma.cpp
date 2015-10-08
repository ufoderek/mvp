/********************************************************************************
 * MVP(Multicore Virtual Platform)
 * (C) Copyright CASLab.EE.NCKU 2009
 * All Right Reserved
 *
 * the ARM PL080 AMBA DMA controller module of MVP emulator
 *
 * Shin-Ying Lee(sing@casmail.ee.ncku.edu.tw)
 * Computer Architecture & System Laboratory
 * Dept. of Electrical Engineering & Inst. of Computer and Communication Engineering
 * National Cheng Kung University, Tainan, Taiwan
 * 2009.11.19
 ********************************************************************************/

/* import header files */
#include<cstring>
#include<dma.h>
#include<iostream>

using namespace std;

/* the constructor */
DMA::DMA(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    IntStatus = 0;
}

/* running the AMBA DMA module */
void DMA::run(void)
{
}

/* read registers of the AMBA DMA module */
bool DMA::read(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        case 0x0000:
            *data = IntStatus;
            break;

#ifdef PLT_VERSATILEPB
        case 0x0fe0:
            *data = 0x80;
            break;
        case 0x0fe4:
            *data = 0x10;
            break;
        case 0x0fe8:
            *data = 0x04;
            break;
        case 0x0fec:
            *data = 0x0a;
            break;
#else
        case 0x0fe0:
            *data = 0x81;
            break;
        case 0x0fe4:
            *data = 0x10;
            break;
        case 0x0fe8:
            *data = 0x05;
            break;
        case 0x0fec:
            *data = 0x00;
            break;
#endif

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
            printb(d_dma, "read unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

/* read registers of the AMBA DMA module */
bool DMA::write(uint32_t data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        default:
            printb(d_dma, "write unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

bool DMA::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if (write) {
        success = this->write(data, local_address, length);
        printd(d_dma, "write to phy address %u: %u", addr, data);
    }

    else {
        success = this->read(&data, local_address, length);
        printd(d_dma, "read from phy address %u: %u", addr, data);
    }

    return success;
}

