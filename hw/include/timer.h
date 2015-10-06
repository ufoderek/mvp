#ifndef _TIMER_H_
#define _TIMER_H_

#include <systemc.h>
#include <ahb_slave_if.h>

/* the SystemC module of ARM SP804 Timer controller */
class TIMER: public ahb_slave_if, public sc_module
{
    public:
        sc_in_clk clk;              //clock signal
        sc_in<bool>rst_n;           //negative edge trigger reset signal
        sc_out<bool>irq_n;          //negative edge trigger IRQ signal

        SC_HAS_PROCESS(TIMER);
        TIMER(sc_module_name name, uint32_t mapping_size);

        bool read(uint32_t*, uint32_t, int);
        bool write(uint32_t, uint32_t, int);

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
        uint32_t Timer0Load, Timer1Load;
        uint32_t Timer0Value, Timer1Value;;
        uint32_t Timer0Control, Timer1Control;
        bool Timer0Enable, Timer1Enable;
        bool Timer0Mode, Timer1Mode;
        bool Timer0Int, Timer1Int;
        bool Timer0Size, Timer1Size;
        bool Timer0One, Timer1One;
        bool irq0_n, irq1_n;

        void run(void);
};

#endif
