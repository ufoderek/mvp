/********************************************************************************
 * MVP(Multicore Virtual Platform)
 * (C) Copyright CASLab.EE.NCKU 2009
 * All Right Reserved
 *
 * the ARM PL110 color LCD controller module of MVP emulator
 *
 * Shin-Ying Lee(sing@casmail.ee.ncku.edu.tw)
 * Computer Architecture & System Laboratory
 * Dept. of Electrical Engineering & Inst. of Computer and Communication Engineering
 * National Cheng Kung University, Tainan, Taiwan
 * 2009.11.23
 ********************************************************************************/

/* import header files */
#include<clcd.h>
#include<cstring>
#include<iostream>

/* the constructor */
CLCD::CLCD (sc_module_name name, uint32_t mapping_size, bool using_lcd) : ahb_slave_if (mapping_size), sc_module (name)
{
    /* initialize all registers */
    LCDTime0 = 0;
    LCDTime1 = 0;
    LCDTime2 = 0;
    LCDTime3 = 0;
    LCDUPBASE = 0;
    LCDLPBASE = 0;
    LCDIMSC = 0;
    LCDControl = 0;
    ppl = DEFAULT_WIDTH;
    lpp = DEFAULT_HEIGHT;
    bpp = DEFAULT_DEPTH;
    LcdPwr = false;
    LcdEn = false;
    //panel = NULL;
    /*
        if (using_lcd) {
            //panel = SDL_SetVideoMode(ppl, lpp, bpp, SDL_HWSURFACE | SDL_RESIZABLE);
            //SDL_UpdateRect(panel, 0, 0,  panel->w,  panel->h);

            SC_METHOD(run);
            sensitive << clk.pos();
        }
    */
}

/* the deconstructor */
CLCD::~CLCD (void)
{
    /********************************************************************************
     * NOTE:
     * panel->pixels shoud point to the original allocated fram buffer
     * otherwise, it may couse fatal fails while deconstructing
     ********************************************************************************/
    //SDL_FreeSurface(panel);
}

/* running the CLCD module */
void CLCD::run (void)
{
    if ( (LcdEn && LcdPwr) ) {
        static int count = 0;
        count++;

        if (count >= 0x00001000) {
            /********************************************************************************
             * NOTE:
             * the RGB pixel value ordering of SDL library is reversed with ARM PL110 CLCD
             * hence, it should be fixed before copying to physical frame memory
             * SLD      --> RGB565
             * PL110    --> BGR565
             ********************************************************************************/
            /*
            for (int i = 0; i < (panel->w * panel->h); i++) {
                uint16_t tmp = ((uint16_t*)fbup)[i];
                ((uint16_t*)panel->pixels)[i] =
                    ((tmp & 0x001f) << 11) |    // R
                    ((tmp & 0x07e0)) |          // G
                    ((tmp & 0xf800) >> 11) ;    // B
            }
            */
            //SDL_UpdateRect(panel, 0, 0,  panel->w,  panel->h);
            count = 0;
        }
    }
}

/* resize & modify the LCD panel */
void CLCD::resize (void)
{
    //panel = SDL_SetVideoMode(ppl, lpp, bpp, SDL_HWSURFACE | SDL_RESIZABLE);
    //SDL_UpdateRect(panel, 0, 0,  panel->w,  panel->h);
}

/* read registers of the CLCD module */
bool CLCD::read (uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    /********************************************************************************
     * NOTE:
     * the address mapping of LCDControl and LCDIMSC registers are different with PL110 spec
     * they are reversed on versatile & realview development board
     * there might be something wrong...
     * however, it doesn't work!!
     ********************************************************************************/
    switch (addr) {
        case 0x0000:
            *data = LCDTime0;
            break;
        case 0x0004:
            *data = LCDTime1;
            break;
        case 0x0008:
            *data = LCDTime2;
            break;
        case 0x000c:
            *data = LCDTime3;
            break;
        case 0x0010:
            *data = LCDUPBASE;
            break;
        case 0x0014:
            *data = LCDLPBASE;
            break;
        case 0x0018:
            *data = LCDControl;
            break;
        case 0x001c:
            *data = LCDIMSC;
            break;
#ifdef PLT_VERSATILEPB
        case 0x0fe0:
            *data = 0x10;
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
#else
        case 0x0fe0:
            *data = 0x11;
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
            printb (d_clcd, "write unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

/* write registers of the CLCD module */
bool CLCD::write (uint32_t data, uint32_t addr, int size)
{
    bool result = true;
    int tmp;

    /********************************************************************************
     * NOTE:
     * the address mapping of LCDControl and LCDIMSC registers are different with PL110 spec
     * they are reversed on versatile & realview development board
     * there might be something wrong...
     * however, it doesn't work!!
     ********************************************************************************/
    switch (addr) {
        case 0x0000:
            tmp = ppl;
            LCDTime0 = data;
            ppl = ( ( (data & 0x00ff) >> 2) + 1) << 4;  // pixel per line

            /* resize the panel */
            if (ppl != tmp) {
                resize();
            }

            break;
        case 0x0004:
            tmp = lpp;
            LCDTime1 = data;
            lpp = (data & 0x01ff) + 1;                  // lines per panel

            /* resize the panel */
            if (lpp != tmp) {
                resize();
            }

            break;
        case 0x0008:
            LCDTime2 = data;
            break;
        case 0x000c:
            LCDTime3 = data;
            break;
        case 0x0010:
            LCDUPBASE = data & 0xfffffffc;
            //fbup = (uint16_t*)bus_lookup(LCDUPBASE);
            printm (d_clcd, "CLCD master interface not implemented yet");
            break;
        case 0x0014:
            LCDLPBASE = data & 0xfffffffc;
            //fblp = (uint16_t*)bus_lookup(LCDLPBASE);
            printm (d_clcd, "CLCD master interface not implemented yet");
            break;
        case 0x0018:
            tmp = bpp;
            LCDControl = data;
            LcdPwr = (data >> 11) & 0x0001; // power enable
            LcdEn = data & 0x0001;          // LCD enable

            switch ( (data >> 1) & 0x0007) { // bits per pixel
                case 0:
                    bpp = 1;
                    break;
                case 1:
                    bpp = 2;
                    break;
                case 2:
                    bpp = 4;
                    break;
                case 3:
                    bpp = 8;
                    break;
                case 4:
                    bpp = 16;
                    break;
                case 5:
                    bpp = 24;
                    break;
                default:
                    break;
            }

            /* resize the panel */
            if (bpp != tmp) {
                resize();
            }

            break;
        case 0x001c:
            LCDIMSC = data;
            break;
        default:
            printb (d_clcd, "write unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

bool CLCD::local_access (bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if (write) {
        success = this->write (data, local_address, length);
        printd (d_clcd, "write to phy address %u: %u", addr, data);
    }

    else {
        success = this->read (&data, local_address, length);
        printd (d_clcd, "read from phy address %u: %u", addr, data);
    }

    return success;
}

