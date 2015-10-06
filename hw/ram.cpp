#include <string>
#include <ram.h>
#include <bit_opt_v2.h>

using namespace std;

/* the constructor */
RAM::RAM(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    //make sure the size is aligned to 32-bit word
    if(mapping_size & mask(1, 0))
    {
        printb(d_ram, "RAM size is not aligned to 32-bit word");
    }

    bank = new uint8_t[mapping_size];

    if(bank == 0)
    {
        printb(d_ram, "not enough memory space");
    }

    printm(d_ram, "reset ram to zero");
    //memset(bank, 0, mapping_size);
}

RAM::~RAM()
{
    if(bank != 0)
    {
        delete[]bank;
    }
}

/* read data from the RAM module */
bool RAM::read(uint32_t* data, uint32_t addr, int size)
{
    switch(size)
    {
        case 4:
            *data = *ptr_word(addr);
            break;
        case 1:
            *data = *ptr_byte(addr);
            break;
        case 2:
            *data = *ptr_hword(addr);
            break;
        default:
            printb(d_ram, "read size error");
            break;
    }

    return true;
}

/* write data to the RAM module */
bool RAM::write(uint32_t data, uint32_t addr, int size)
{
    switch(size)
    {
        case 4:
            *ptr_word(addr) = (uint32_t)data;
            break;
        case 1:
            *ptr_byte(addr) = (uint8_t)data;
            break;
        case 2:
            *ptr_hword(addr) = (uint16_t)data;
            break;
        default:
            printb(d_ram, "write size error");
            break;
    }

    return true;
}

bool RAM::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        return this->write(data, local_address, length);
    }

    else
    {
        return this->read(&data, local_address, length);
    }
}
