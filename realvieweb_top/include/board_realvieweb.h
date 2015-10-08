#ifndef _BOARD_REALVIEWEB_H_
#define _BOARD_REALVIEWEB_H_
// system headers
#include <fcntl.h>
#include <memory>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

// systemc header
#include <systemc.h>
#include <boost/shared_ptr.hpp>

// cpu header

#ifdef REALVIEWEB_ARMV5
#include <armv5.h>
#endif

#ifdef REALVIEWEB_ARMV6
#include <armv6.h>
#endif

#ifdef REALVIEWEB_ARMV6K
#include <arm11_mpcore.h>
#endif

// hardware headers
#include <aaci.h>
#include <arbiter.h>
#include <ahb.h>
#include <clcd.h>
#include <ctime>
#include <dma.h>
#include <gpio.h>
//#include <gic.h>
#include <gic2_dist.h>
#include <gic2_cpu_if.h>
#include <mci.h>
#include <mpmc.h>
#include <ps2.h>
#include <ram.h>
#include <rtc.h>
#include <sci.h>
#include <ssmc.h>
#include <ssp.h>
#include <sysctrl_0.h>
#include <sysctrl_1.h>
#include <timer.h>
#include <uart.h>
#include <watchdog.h>

// versatile headers
#include <board_realvieweb_map.h>
#include <board_realvieweb_interrupt.h>

// debug utils
#include <debug_utils.h>

class board_realvieweb: public sc_module
{
    public:

        SC_HAS_PROCESS(board_realvieweb);
        board_realvieweb(sc_module_name name, bool using_gdb, unsigned int gdb_port);
        ~board_realvieweb();

        clock_t time_start, time_end;

        uint32_t size_kernel ;  // size of kernel
        uint32_t size_prog ;        // size of application program
        uint32_t size_initrd ;  // size of initrd image

        uint32_t size_ram;
        uint32_t size_smp_boot_rom;

        uint32_t start_addr;
        uint32_t smp_start_addr;

#ifdef REALVIEWEB_ARMV5
        auto_ptr<ARMV5> arm;
#endif

#ifdef REALVIEWEB_ARMV6
        auto_ptr<ARMV6> arm;
#endif

#ifdef REALVIEWEB_ARMV6K
        auto_ptr<ARM11_MPCORE> arm;
#endif

        auto_ptr<AACI> aaci;
        auto_ptr<arbiter> arb;
        auto_ptr<ahb> my_bus;
        auto_ptr<CLCD> clcd;
        //auto_ptr<GIC> gic_1, gic_2, gic_3, gic_4;
        boost::shared_ptr<gic2_dist> gic_dist;
        auto_ptr<gic2_cpu_if> gic_cpu_if;

        auto_ptr<GPIO> gpio_0, gpio_1, gpio_2, gpio_3;
        auto_ptr<DMA> dma;
        auto_ptr<MCI> mci;
        auto_ptr<PS2> keyboard, mouse;
        auto_ptr<RAM> ram;
        auto_ptr<RTC> rtc;
        auto_ptr<SCI> sci;
        auto_ptr<SSMC> ssmc;
        auto_ptr<SSP> ssp;
        auto_ptr<SYSCTRL_0> status_0;
        auto_ptr<SYSCTRL_1> status_1;
        auto_ptr<TIMER> timer_0, timer_1;
        auto_ptr<UART> uart_0, uart_1, uart_2, uart_3;
        auto_ptr<WATCHDOG> watchdog;
        auto_ptr<RAM> smp_boot_rom;

        auto_ptr<sc_clock> clk_fast;                    // 200MHz clock
        auto_ptr<sc_clock> clk_slow;                    // 4MHz clock

        sc_signal<bool> rst_n;              // reset signal
        sc_signal<bool> irq_n;              // IRQ signal to CPU
        sc_signal<bool> virq_n;              // IRQ signal to CPU
        sc_signal<bool> channel[16 + 16 + 32]; // sgi + ppi + spi
        sc_signal<bool> mp_channel[64];     // IRQ signal to MP's GIC

        /* load the kernel image to RAM */
        void load_kernel(char* path_kernel);
        void load_initrd(char* path_initrd);
        void load_prog(char* path_prog);

        bool using_gdb;
        unsigned int gdb_port;

        /* load booting program to RAM */
        void load_boot();
        void load_smp_boot();

        /* fill the ATAG table for ARM Linux */
        void load_atag();
        /* to terminate the execution */
        void bus_exit();
};
#endif
