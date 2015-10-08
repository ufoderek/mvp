#include <mp_scu.h>
#include <bit_opt.h>

mp_scu::mp_scu(sc_module_name name, unsigned int n_cores): sc_module(name)
{
    int i;

    this->n_cores = n_cores;
}

mp_scu::~mp_scu()
{
}

bool mp_scu::read(unsigned int core_id, uint32_t* data, uint32_t addr, unsigned int length)
{
    bool result;

    if ((addr & MM(31, 13)) ==  0x10100000) {
        uint32_t offset = addr & MM(12, 0);

        if (offset <= 0x00FF) {
            return scu_read(core_id, data, addr & 0xFF);
        }

        else {
            printb(d_mp_scu, "core %u read, addr: 0x%X", core_id, addr);
        }
    }

    else {
        return bus_read(data, addr, length);
    }
}

bool mp_scu::write(unsigned int core_id, uint32_t data, uint32_t addr, unsigned int length)
{
    bool result;

    if ((addr & MM(31, 13)) ==  0x10100000) {
        uint32_t offset = addr & MM(12, 0);

        if (offset <= 0x00FF) {
            return scu_write(core_id, data, addr & 0xFF);
        }

        else {
            printb(d_mp_scu, "core %u write: 0x%X, addr: 0x%X", core_id, data, addr);
        }
    }

    else {
        return bus_write(data, addr, length);
    }
}

bool mp_scu::bus_read(uint32_t* data, uint32_t addr, unsigned int length)
{
    return bus_b_access(false, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(data), length);
}

bool mp_scu::bus_write(uint32_t data, uint32_t addr, unsigned int length)
{
    return bus_b_access(true, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(&data), length);
}

bool mp_scu::scu_read(unsigned int core_id, uint32_t* data, uint32_t offset)
{
    switch (offset) {
        case 0x00:
            *data = scu_regs.control;
            break;
        case 0x04:
            *data = 0xF << 4 | 3;
            break;
        default:
            printb(d_mp_scu, "core %u read, offset: 0x%X", core_id, offset);
    }

    return true;
}

bool mp_scu::scu_write(unsigned int core_id, uint32_t data, uint32_t offset)
{
    switch (offset) {
        case 0x00:
            printm(d_mp_scu, "core %u write scu control: 0x%X", core_id, scu_regs.control);
            scu_regs.control = data;
            break;
        default:
            printb(d_mp_scu, "core %u write, offset: 0x%X, data: 0x%X", core_id, offset, data);
    }

    return true;
}

