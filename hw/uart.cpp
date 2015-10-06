#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <uart.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

UART::UART(sc_module_name name, uint32_t mapping_size, bool on): ahb_slave_if(mapping_size), sc_module(name)
{
    enable_tx = true;
    enable_rx = true;
    enable_uart = on;
    connection = on;
    uartfr = 0x0090;
    uartibrd = 0x0006;
    uartfbrd = 0x0021;
    uartcr = 0x0300 | on;
    uartifls = 0x0012;
    uartlcr = 0;
    uartimsc = 0;
    uartris = 0;
    tx_head = 0;
    tx_count = 0;
    rx_head = 0;
    rx_count = 0;
    force = false;
    using_fifo = false;

    if(connection)
    {
        printm(d_uart, "disable console echo");
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSADRAIN, &newt);
        printm(d_uart, "set stdin to be non-blocking");
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        SC_METHOD(run);
        sensitive << clk.pos();
    }
}

UART::UART(sc_module_name name, uint32_t mapping_size, bool on, const char* fifo_name): ahb_slave_if(mapping_size), sc_module(name)
{
    enable_tx = true;
    enable_rx = true;
    enable_uart = on;
    connection = on;
    uartfr = 0x0090;
    uartibrd = 0x0006;
    uartfbrd = 0x0021;
    uartcr = 0x0300 | on;
    uartifls = 0x0012;
    uartlcr = 0;
    uartimsc = 0;
    uartris = 0;
    tx_head = 0;
    tx_count = 0;
    rx_head = 0;
    rx_count = 0;
    force = false;
    using_fifo = false;

    if(connection)
    {
        printm(d_uart, "using fifo uart client");
        char fifo_name_in[128] = "";
        char fifo_name_out[128] = "";

        strcpy(fifo_name_in, fifo_name);
        strcat(fifo_name_in, ".in.fifo");

        strcpy(fifo_name_out, fifo_name);
        strcat(fifo_name_out, ".out.fifo");

        mknod(fifo_name_in, S_IFIFO | 0666, 0);

        printm(d_uart, "open fifo for read: %s", fifo_name_in);
        ifd = open(fifo_name_in, O_RDONLY);
        if(ifd == -1)
        {
            printb(d_uart, "open %s error", fifo_name_in);
        }
        fcntl(ifd, F_SETFL, fcntl(ifd, F_GETFL) | O_NONBLOCK);

        mknod(fifo_name_out, S_IFIFO | 0666, 0);

        printm(d_uart, "open fifo for write: %s", fifo_name_out);
        ofd = open(fifo_name_out, O_WRONLY);
        if(ofd == -1)
        {
            printb(d_uart, "open %s error", fifo_name_out);
        }

        using_fifo = true;

        SC_METHOD(run);
        sensitive << clk.pos();
    }
}

UART::~UART(void)
{
    if(connection && !using_fifo)
    {
        if(!using_fifo)
        {
            printe("enable console echo");
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            printe("restore stdin");
            fcntl(STDIN_FILENO, F_SETFL, oldf);
        }
        else
        {
            close(ifd);
            close(ofd);
        }
    }
}

void UART::run(void)
{
    bool irq_tx = true;
    bool irq_rx = true;

    if(enable_uart)
    {
        /* UART TX */
        if((tx_count > 0) && enable_tx)
        {
            int index = (UART_BUFF_SIZE + tx_head - tx_count) % UART_BUFF_SIZE;

            if(using_fifo)
            {
                ::write(ofd, &(tx_buff[index]), 1);
            }
            else
            {
                fputc(tx_buff[index], stderr);
            }

            tx_count--;

            if(tx_count == 0)   //TX buffer is empty
            {
                uartfr &= ~(0x01 << 5); //TXFF
                uartfr |= (0x01 << 7); //TXFE
                irq_tx = false;
            }
            else
            {
                uartfr &= ~(0x01 << 5); //TXFF
                uartfr &= ~(0x01 << 7); //TXFE
            }
        }

        /* UART RX */
        if((rx_count < UART_BUFF_SIZE) && enable_rx)
        {
            char key;

            if(hit(&key))
            {
                printd(d_uart, "detect key press");
                rx_buff[rx_head] = key;
                rx_head = (rx_head + 1) % UART_BUFF_SIZE;
                rx_count++;
                irq_rx = false;

                if(rx_count >= UART_BUFF_SIZE)   //RX buffer is full
                {
                    uartfr |= (0x01 << 6); //RXFF
                    uartfr &= ~(0x01 << 4); //RXFE
                }
                else
                {
                    uartfr &= ~(0x01 << 6); //RXFF
                    uartfr &= ~(0x01 << 4); //RXFE
                }
            }
        }
    }

    uartris |= (((!irq_tx) << 5) | ((!irq_rx) << 4));
    irq_n = ((uartris & uartimsc) == 0);
}

bool UART::read(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch(addr)
    {
        case 0x0000:
            *data = get_rx_buff();
            break;
        case 0x0018:
            *data = uartfr;
            break;
        case 0x0024:
            *data = uartibrd;
            break;
        case 0x0028:
            *data = uartfbrd;
            break;
        case 0x002c:
            *data = uartlcr;
            break;
        case 0x0030:
            *data = uartcr;
            break;
        case 0x0034:
            *data = uartifls;
            break;
        case 0x0038:
            *data = uartimsc;
            break;
        case 0x003c:
            *data = uartris;
            break;
        case 0x0040:
            *data = uartris & uartimsc;
            break;
        case 0x0fe0:
            *data = 0x11;
            break;
        case 0x0fe4:
            *data = 0x10;
            break;
        case 0x0fe8:
            *data = 0x14;
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
            printm(d_uart, "read unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

bool UART::write(uint32_t data, uint32_t addr, int size)
{
    bool result = true;

    switch(addr)
    {
        case 0x0000:
            put_tx_buff(data);
            break;
        case 0x0024:        /* baud rate register of integer part */
            uartibrd = data;
            break;
        case 0x0028:        /* baud rate register of fraction part */
            uartfbrd = data;
            break;
        case 0x002c:
            uartlcr = data;
            break;
        case 0x0030:
            uartcr = data;
            enable_tx = (data >> 8) & 0x01;
            enable_rx = (data >> 9) & 0x01;
            enable_uart = data & 0x01;
            force = !force;
            break;
        case 0x0034:
            uartifls = data;
            break;
        case 0x0038:
            uartimsc = data;
            force = !force;
            break;
        case 0x0044:
            uartris &= ~data;
            force = !force;
            break;
        default:
            printb(d_uart, "write unknown: 0x%X", addr);
            result = false;
            break;
    }

    return result;
}

void UART::put_tx_buff(char data)
{
    if(tx_count < UART_BUFF_SIZE)
    {
        tx_buff[tx_head] = data;
        tx_head = (tx_head + 1) % UART_BUFF_SIZE;
        tx_count++;

        if(tx_count >= UART_BUFF_SIZE)   //TX buffer is full
        {
            uartfr |= (0x01 << 5); //TXFF
            uartfr &= ~(0x01 << 7); //TXFE
        }
        else
        {
            uartfr &= ~(0x01 << 5); //TXFF
            uartfr &= ~(0x01 << 7); //TXFE
        }
    }
    else
    {
        /* TX buffer is full, drop the data */
        uartfr |= (0x01 << 5); //TXFF
        uartfr &= ~(0x01 << 7); //TXFE
    }
}

char UART::get_rx_buff(void)
{
    char data;

    if(rx_count != 0)
    {
        int index = (UART_BUFF_SIZE + rx_head - rx_count) % UART_BUFF_SIZE;
        data = rx_buff[index];
        rx_count--;

        if(rx_count == 0)   //RX buffer is empty
        {
            uartfr &= ~(0x01 << 6); //RXFF
            uartfr |= (0x01 << 4); //RXFE
        }
        else
        {
            uartfr &= ~(0x01 << 6); //RXFF
            uartfr &= ~(0x01 << 4); //RXFE
        }
    }
    else
    {
        /* RX buffer is empty, return NULL */
        uartfr &= ~(0x01 << 6); //RXFF
        uartfr |= (0x01 << 4); //RXFE
        data = 0;
    }

    return data;
}

bool UART::hit(char* key)
{
    if(using_fifo)
    {
        int size = ::read(ifd, key, 1);
        if(size <= 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        *key = (char)getchar();
    }
    return(*key != EOF);
}

bool UART::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        success = this->write(data, local_address, length);
        printd(d_uart, "write to phy address %u: %u", addr, data);
    }
    else
    {
        success = this->read(&data, local_address, length);
        printd(d_uart, "read from phy address %u: %u", addr, data);
    }

    return success;
}

