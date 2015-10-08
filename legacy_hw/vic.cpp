/********************************************************************************
 * MVP(Multicore Virtual Platform)
 * (C) Copyright CASLab.EE.NCKU 2009
 * All Right Reserved
 *
 * the ARM PL190 vector interrupt controller module of MVP emulator
 *
 * Shin-Ying Lee(sing@casmail.ee.ncku.edu.tw)
 * Computer Architecture & System Laboratory
 * Dept. of Electrical Engineering & Inst. of Computer and Communication Engineering
 * National Cheng Kung University, Tainan, Taiwan
 * 2009.07.08
 ********************************************************************************/

/* import header files */
#include<cstring>
#include<vic.h>

using namespace std;

/* the constructor */
VIC::VIC(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    irq_status = 0x00000000;
    fiq_status = 0x00000000;
    int_select = 0x00000000;
    int_enable = 0x00000000;
    soft_int = 0x00000000;
    isr_addr = 0x00000000;
    default_addr = 0x00000000;
    protection = false;
    itcr = false;
    clear = false;
    vect = false;
    force = false;

    for (int i = 0; i < 16; i++) {
        vect_addr[i] = 0;
        vect_cntl[i] = 0;
    }

    /* start to simulate */
    SC_METHOD(run);
    sensitive
            << channel[0x00]
            << channel[0x01]
            << channel[0x02]
            << channel[0x03]
            << channel[0x04]
            << channel[0x05]
            << channel[0x06]
            << channel[0x07]
            << channel[0x08]
            << channel[0x09]
            << channel[0x0a]
            << channel[0x0b]
            << channel[0x0c]
            << channel[0x0d]
            << channel[0x0e]
            << channel[0x0f]
            << channel[0x10]
            << channel[0x11]
            << channel[0x12]
            << channel[0x13]
            << channel[0x14]
            << channel[0x15]
            << channel[0x16]
            << channel[0x17]
            << channel[0x18]
            << channel[0x19]
            << channel[0x1a]
            << channel[0x1b]
            << channel[0x1c]
            << channel[0x1d]
            << channel[0x1e]
            << channel[0x1f]
            << force;
}

/* running the VIC module */
void VIC::run(void)
{
    raw_status = 0;

    for (int i = 0; i < 32; i++) {
        if (channel[i] == false) {
            raw_status |= 0x01 << i;
        }
    }

    fiq_status = raw_status & int_enable & int_select;
    irq_status = raw_status & int_enable & (~int_select);

    if (fiq_status != 0) {
        irq_n = true;
    }

    else if (irq_status != 0) {
        irq_n = false;
    }

    else {
        irq_n = true;
    }
}

/* read registers of the VIC module */
bool VIC::read(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        case 0x0000:
            *data = irq_status;
            break;
        case 0x0004:
            *data = fiq_status;
            break;
        case 0x0008:
            *data = raw_status;
            break;
        case 0x0010:
            *data = int_enable;
            break;
        case 0x0018:
            *data = soft_int;
            break;
        case 0x0020:
            *data = protection;
            break;
        case 0x0030:
            *data = isr_addr;
            break;
        case 0x0034:
            *data = default_addr;
            break;
        case 0x0300:
            *data = itcr;
            break;
        case 0x0FE0:
            *data = 0x0192;
            break;
        case 0x0FE4:
            *data = 0x0011;
            break;
        case 0x0FE8:
            *data = 0x0004;
            break;
        case 0x0FEC:
            *data = 0x0000;
            break;
        default:
            printm(d_vic, "read unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

/* write registers of the VIC module */
bool VIC::write(uint32_t data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        case 0x0000:
        case 0x0004:
        case 0x0008:
            break;
        case 0x000c:
            int_select = data;
            break;
        case 0x0010:
            int_enable |= data;
            force = !force;
            break;
        case 0x0014:
            int_enable &= (~data);
            force = !force;
            break;
        case 0x0018:
            soft_int |= data;
            break;
        case 0x001c:
            soft_int &= (~data);
            break;
        case 0x0020:
            protection = data & 0x0001;
            break;
        case 0x0030:
            /* clear vector interrupt service */
            clear = true;
            isr_addr = 0;
            force = !force;
            break;
        case 0x0034:
            default_addr = data;
            break;
        case 0x0100:
        case 0x0104:
        case 0x0108:
        case 0x010c:
        case 0x0110:
        case 0x0114:
        case 0x0118:
        case 0x011c:
        case 0x0120:
        case 0x0124:
        case 0x0128:
        case 0x012c:
        case 0x0130:
        case 0x0134:
        case 0x0138:
        case 0x013c:
            vect_addr[(addr - 0x0100) >> 2] = data;
            break;
        case 0x0200:
        case 0x0204:
        case 0x0208:
        case 0x020c:
        case 0x0210:
        case 0x0214:
        case 0x0218:
        case 0x021c:
        case 0x0220:
        case 0x0224:
        case 0x0228:
        case 0x022c:
        case 0x0230:
        case 0x0234:
        case 0x0238:
        case 0x023c:
            vect_cntl[(addr - 0x0200) >> 2] = data;
            break;
        case 0x0300:
            itcr = data & 0x0001;
            break;
        default:
            printm(d_vic, "write unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

bool VIC::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if (write) {
        success = this->write(data, local_address, length);
        printd(d_vic, "write to phy address %u: %u", addr, data);
    }

    else {
        success = this->read(&data, local_address, length);
        printd(d_vic, "read from phy address %u: %u", addr, data);
    }

    return success;
}
