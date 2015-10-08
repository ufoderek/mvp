/********************************************************************************
 * MVP(Multicore Virtual Platform)
 * (C) Copyright CASLab.EE.NCKU 2009
 * All Right Reserved
 *
 * the ARM PL050 PS2 keyboard & mouse interface module of MVP emulator
 *
 * Shin-Ying Lee(sing@casmail.ee.ncku.edu.tw)
 * Computer Architecture & System Laboratory
 * Dept. of Electrical Engineering & Inst. of Computer and Communication Engineering
 * National Cheng Kung University, Tainan, Taiwan
 * 2009.11.24
 ********************************************************************************/

/* import header files */
#include<cstring>
#include<iostream>
#include<ps2.h>

/* the constructor */
PS2::PS2(sc_module_name name, uint32_t mapping_size, PS2Dev_t dev): ahb_slave_if(mapping_size), sc_module(name)
{
    /* initialize all registers */
    type = dev;
    KMICR = 0;
    KMISTAT = 0x0043;
    KMIDATAr = 0;
    KMIDATAt = 0;
    KMICLKDIV = 0;
    KMIIR = 0;
    KMIRXINTen = false;
    KMITXINTen = false;
    KmiEn = false;
    run_callback = (dev == PS2Keyboard) ? &PS2::run_rx_keyboard : &PS2::run_rx_mouse;

    /* start to simulate */
    //  SC_METHOD(run);
    //  sensitive << clk.pos();
}

/* running ARM PL050 PS2 module */
void PS2::run(void)
{
    bool IRQrx, IRQtx;

    if (KmiEn == true) {
        static int count = 0;

        count++;

        if (count >= 0x00020000) {
            if (KMIDATAt == 0) {            // TX
                KMIDATAt = 0;
                KMIIR |= 0x02;
            }

            else {                          // RX
                (this->*run_callback)();
            }

            count = 0;
        }
    }

    IRQrx = !(((KMIIR & 0x01) != 0) && KMIRXINTen);
    IRQtx = !(((KMIIR & 0x02) != 0) && KMITXINTen);
    irq_n = IRQrx & IRQtx;
}

/* running the PS2 keyboard RX module */
void PS2::run_rx_keyboard(void)
{
}

/* running the PS2 mouse RX module */
void PS2::run_rx_mouse(void)
{
}

/* read registers of the PS2 module */
bool PS2::read(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        case 0x0000:
            *data = KMICR;
            break;
        case 0x0004:
            *data = KMISTAT;
            break;
        case 0x0008:
            *data = KMIDATAr;
            KMIIR &= 0xfe;      // clear RX IRQ
            break;
        case 0x000c:
            *data = KMICLKDIV;
            break;
        case 0x0010:
            *data = KMIIR;
            break;
        case 0x0fe0:
            *data = 0x50;
            break;
        case 0x0fe4:
            *data = 0x10;
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
            printb(d_ps2, "write unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

/* write registers of the PS2 module */
bool PS2::write(uint32_t data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        case 0x0000:
            KMICR = data;
            KMIRXINTen = (data >> 4) & 0x01;
            KMITXINTen = (data >> 3) & 0x01;
            KmiEn = (data >> 2) & 0x01;
            break;
        case 0x0008:
            KMIDATAt = data;
            KMIIR &= 0xfd;      // clear TX IRQ
            break;
        case 0x000c:
            KMICLKDIV = data;
            break;
        default:
            printb(d_ps2, "write unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

bool PS2::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if (write) {
        success = this->write(data, local_address, length);
        printd(d_ps2, "write to phy address %u: %u", addr, data);
    }

    else {
        success = this->read(&data, local_address, length);
        printd(d_ps2, "read from phy address %u: %u", addr, data);
    }

    return success;
}

