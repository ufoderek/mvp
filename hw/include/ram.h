#ifndef _RAM_H_
#define _RAM_H_

#include <stdint.h>
#include <systemc.h>
#include <ahb_slave_if.h>

/* module of RAM */
class RAM: public ahb_slave_if, public sc_module
{
    public:
        SC_HAS_PROCESS(RAM);
        RAM(sc_module_name name, uint32_t mapping_size);
        ~RAM();

        bool read(uint32_t*, uint32_t, int);
        bool write(uint32_t, uint32_t, int);

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
        uint8_t* bank;

        inline uint32_t* ptr_word(uint32_t addr)
        {
            return (uint32_t*)(bank + addr);
        }

        inline uint16_t* ptr_hword(uint32_t addr)
        {
            return (uint16_t*)(bank + addr);
        }

        inline uint8_t* ptr_byte(uint32_t addr)
        {
            return (uint8_t*)(bank + addr);
        }
};

#endif
