#ifndef _BOARD_REALVIEWPB_H_
#define _BOARD_REALVIEWPB_H_

#include <memory>
#include <stdint.h>
//#include <boost/shared_ptr.hpp>

#include <systemc.h>
#include <debug_utils.h>

#include <armv7a.h>
#include <armv7a_tlb.h>
#include <ahb.h>
#include <gic2_cpu_if.h>
#include <gic2_dist.h>
#include <ram.h>
#include <sysctrl_0.h>
#include <sysctrl_1.h>
#include <timer.h>
#include <uart.h>
#include <black_hole.h>

class board_realviewpb: public sc_module
{
    public:

        SC_HAS_PROCESS(board_realviewpb);
        board_realviewpb(sc_module_name name, bool using_gdb, unsigned int gdb_port);
        ~board_realviewpb();

        uint32_t size_kernel;           //size of kernel
        uint32_t size_prog;             //size of application program
        uint32_t size_initrd;           //size of initrd image

        uint32_t size_ram;

        uint32_t start_addr;
        uint32_t smp_start_addr;

        auto_ptr<armv7a>arm;
        //auto_ptr<armv7a_tlb>arm;

        auto_ptr<ahb>my_bus;
        boost::shared_ptr<gic2_dist>gic_dist;
        auto_ptr<gic2_cpu_if>gic_cpu_if;
        auto_ptr<RAM>ram;
        auto_ptr<SYSCTRL_0>status_0;
        auto_ptr<SYSCTRL_1>status_1;
        auto_ptr<TIMER>timer_0, timer_2;
        auto_ptr<UART>uart_0;
        auto_ptr<RAM>smp_boot_rom;
        auto_ptr<black_hole>bh;

        auto_ptr<sc_clock>clk_fast;                             //2000MHz clock
        auto_ptr<sc_clock>clk_slow;                             //400MHz clock
        auto_ptr<sc_clock>clk_timer;                             //40MHz clock

        sc_signal<bool>rst_n;                       //reset signal
        sc_signal<bool>irq_n;                       //IRQ signal to CPU
        sc_signal<bool>virq_n;                      //virtual IRQ
        sc_signal<bool>channel[16 + 16 + 32];          //sgi + ppi + spi
        sc_signal<bool>always_1;

        void load_kernel(char* path_kernel);
        void load_initrd(char* path_initrd);
        void load_prog(char* path_prog);

        bool using_gdb;
        unsigned int gdb_port;

        void load_boot();
        void load_smp_boot();

        void load_atag();
};
#endif
