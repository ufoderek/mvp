#ifndef _BOARD_METEORVB_H_
#define _BOARD_METEORVB_H_

#include <memory>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include <systemc.h>

#include <armv7a.h>
#include <ahb.h>
#include <black_hole.h>
#include <gic2_cpu_if.h>
#include <gic2_dist.h>
#include <gic2_vcpu_if.h>
#include <gic2_vctrl.h>
#include <sysctrl_0.h>
#include <sysctrl_1.h>
#include <ram.h>
#include <timer.h>
#include <uart.h>
#include <semihost.h>

class board_meteorvb: public sc_module
{
    public:

        SC_HAS_PROCESS(board_meteorvb);
        board_meteorvb(sc_module_name name, bool using_gdb, unsigned int gdb_port);
        ~board_meteorvb();

        uint32_t size_ram;
        uint32_t size_rom;
        uint32_t start_addr;

        sc_signal<bool>rst_n;                       //reset signal
        sc_signal<bool>irq_n;                       //IRQ signal to CPU
        sc_signal<bool>virq_n;                      //virtual IRQ
        sc_signal<bool>channel[16 + 16 + 32];          //sgi + ppi + spi

        void load_vmm(char* path_vmm, uint32_t addr);

        bool using_gdb;
        unsigned int gdb_port;

        auto_ptr<sc_clock>clk_fast;                             //2000MHz clock
        auto_ptr<sc_clock>clk_slow;                             //400MHz clock
        auto_ptr<sc_clock>clk_timer;                             //40MHz clock

        auto_ptr<armv7a>                arm;
        auto_ptr<ahb>                   my_bus;
        auto_ptr<SYSCTRL_0>             sys_reg;
        auto_ptr<SYSCTRL_1>             sys_ctrl;
        auto_ptr<RAM>                   ram;

        boost::shared_ptr<gic2_dist>    gic_dist;
        boost::shared_ptr<gic2_cpu_if>  gic_cpu_if;
        boost::shared_ptr<gic2_vctrl>   gic_vctrl;
        auto_ptr<gic2_vcpu_if>          gic_vcpu_if;
        auto_ptr<UART>                  uart_0;
        auto_ptr<TIMER>                 timer_0;
        auto_ptr<UART>                  uart_4, uart_5, uart_6, uart_7;
        auto_ptr<TIMER>                 timer_4, timer_5, timer_6, timer_7;
        auto_ptr<TIMER>                 timer_8, timer_9, timer_10, timer_11;
        auto_ptr<black_hole>            bh_0, bh_1, bh_2, bh_3;

        auto_ptr<semihost>              semi;
};
#endif
