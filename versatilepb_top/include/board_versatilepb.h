#ifndef _BOARD_VERSATILEPB_H_
#define _BOARD_VERSATILEPB_H_
// system headers
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <SDL/SDL.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

// systemc header
#include <systemc.h>

// cpu header
#include <armv5.h>

// hardware headers
#include <aaci.h>
#include <arbiter.h>
#include <ahb.h>
#include <calculator.h>
#include <clcd.h>
#include <ctime>
#include <dma.h>
#include <ftp.h>
#include <gpio.h>
#include <mci.h>
#include <mpmc.h>
#include <ps2.h>
#include <ram.h>
#include <rtc.h>
//#include <sig.h>
#include <sci.h>
#include <ssmc.h>
#include <ssp.h>
#include <sysctrl_0.h>
#include <sysctrl_1.h>
#include <timer.h>
#include <uart.h>
#include <vic.h>
#include <watchdog.h>

// versatile headers
#include <board_versatilepb_map.h>
#include <board_versatilepb_interrupt.h>

// debug utils
#include <debug_utils.h>

class board_versatilepb: public sc_module
{
    public:
        SC_HAS_PROCESS(board_versatilepb);
        board_versatilepb(sc_module_name name, bool using_gdb, unsigned int gdb_port);
        ~board_versatilepb();
        /* global variables */
        clock_t time_start, time_end;
        uint32_t size_kernel ;  // size of kernel
        uint32_t size_prog ;        // size of application program
        uint32_t size_initrd ;  // size of initrd image

        auto_ptr<ARMV5> arm;
        auto_ptr<AACI> aaci;
        auto_ptr<arbiter> arb;
        auto_ptr<ahb> my_bus;
        auto_ptr<CLCD> clcd;
        auto_ptr<GPIO> gpio_0, gpio_1, gpio_2, gpio_3;
        auto_ptr<DMA> dma;
        auto_ptr<MCI> mci;
        auto_ptr<MPMC> mpmc;
        auto_ptr<PS2> keyboard, mouse;
        auto_ptr<RAM> ram;
        auto_ptr<RTC> rtc;
        auto_ptr<SCI> sci;
        auto_ptr<SSMC> ssmc;
        auto_ptr<SSP> ssp;
        auto_ptr<SYSCTRL_0> status_0;
        auto_ptr<SYSCTRL_1> status_1;
        auto_ptr<TIMER> timer_0;
        auto_ptr<TIMER> timer_1;
        auto_ptr<UART> uart_0;
        auto_ptr<UART> uart_1;
        auto_ptr<UART> uart_2;
        auto_ptr<UART> uart_3;
        auto_ptr<VIC> vic_0;
        auto_ptr<VIC> vic_1;
        auto_ptr<WATCHDOG> watchdog;

        uint32_t start_addr;
        uint32_t size_ram;
        uint32_t size_intrd;

        auto_ptr<sc_clock> clk_fast;        // 200MHz clock
        auto_ptr<sc_clock> clk_slow;        // 4MHz clock

        sc_signal<bool> rst_n;              // reset signal
        sc_signal<bool> irq_n;              // IRQ signal to CPU
        sc_signal<bool> fiq_n;              // FIQ signal to CPU
        sc_signal<bool> vic_channel[64];    // IRQ signal to VIC_0(PIC) & VIC_1(SIC)
        sc_signal<bool> tmp_fiq;

        /* load the kernel image to RAM */
        void load_kernel(char* path_kernel);
        void load_initrd(char* path_initrd);
        void load_prog(char* path_prog);

        bool using_gdb;
        unsigned int gdb_port;

        /* load booting program to RAM */
        void load_boot(void);

        /* fill the ATAG table for ARM Linux */
        void load_atag(void);
        /* to terminate the execution */
        void bus_exit(void);
};
#endif
