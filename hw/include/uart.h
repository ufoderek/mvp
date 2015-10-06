#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
#include <systemc.h>
#include <ahb_slave_if.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <boost/shared_ptr.hpp>

/* define the buffer size */
#define UART_BUFF_SIZE 32

/* the SystemC module of ARM PL011 UART controller */
class UART: public ahb_slave_if, public sc_module
{
    public:
        sc_in_clk clk;                  //clock signal
        sc_in<bool>rst_n;               //negative edge trigger reset signal
        sc_out<bool>irq_n;              //negative edge trigger IRQ signal
        bool force;

        SC_HAS_PROCESS(UART);
        UART(sc_module_name name, uint32_t mapping_size, bool on);
        UART(sc_module_name name, uint32_t mapping_size, bool on, const char* fifo_name);
        ~UART(void);

        bool read(uint32_t*, uint32_t, int);
        bool write(uint32_t, uint32_t, int);

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:

        int ifd;
        int ofd;
        bool using_fifo;

        unsigned int win;

        char tx_buff[UART_BUFF_SIZE];
        char rx_buff[UART_BUFF_SIZE];
        bool enable_tx, enable_rx, enable_uart;
        bool connection;
        uint32_t uartsr;
        uint32_t uartfr;
        uint32_t uartibrd, uartfbrd;
        uint32_t uartcr;
        uint32_t uartlcr;
        uint32_t uartifls;
        uint32_t uartimsc;
        uint32_t uartris;
        int tx_head, tx_count;
        int rx_head, rx_count;
        int oldf;
        struct termios oldt, newt;

        void run(void);
        void put_tx_buff(char);
        char get_rx_buff(void);
        bool hit(char*);
};

#endif
