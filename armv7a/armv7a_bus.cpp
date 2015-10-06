#include <armv7a.h>

bool armv7a::bus_read(uint32_t* data, uint32_t addr, uint32_t length)
{
    bool success;
    success = bus_b_access(false, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(data), length);

    if(success)
    {
        printd(core_id, d_armv7a_bus, "bus read 0x%X:0x%X", addr, *data);
        return true;
    }
    else
    {
        printm(core_id, d_armv7a_bus, "bus read 0x%X failed", addr);
        return false;
    }
}

bool armv7a::bus_read64(uint64_t* data, uint64_t addr, uint32_t length)
{
    bool success1;
    bool success2;
    uint32_t tmp;

    if(length == 8)
    {
        success1 = bus_b_access(false, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(&tmp), 4);
        *data = tmp;
        success2 = bus_b_access(false, (sc_dt::uint64)addr + 4, reinterpret_cast<unsigned char*>(&tmp), 4);
        *data |= ((uint64_t)tmp) << 32;
    }
    else
    {
        printb(d_armv7a_bus, "bus_read64 length error (=%d)", length);
    }

    if(success1 && success2)
    {
        printd(core_id, d_armv7a_bus, "bus read64 0x%X:0x%08X%08X", addr, (uint32_t)(*data >> 32), (uint32_t)(*data));
        return true;
    }
    else
    {
        printm(core_id, d_armv7a_bus, "bus read64 0x%X failed", addr);
        return false;
    }
}

bool armv7a::bus_write(uint32_t data, uint32_t addr, uint32_t length)
{
    bool success;
    success = bus_b_access(true, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(&data), length);

    if(success)
    {
        printd(core_id, d_armv7a_bus, "bus write 0x%X:0x%X", addr, data);
        return true;
    }
    else
    {
        printm(core_id, d_armv7a_bus, "bus write 0x%X:0x%X failed", addr, data);
        return false;
    }
}

bool armv7a::bus_write64(uint64_t data, uint64_t addr, uint32_t length)
{
    bool success1;
    bool success2;

    uint32_t tmp;

    if(length == 8)
    {
        tmp = (uint32_t)data;
        success1 = bus_b_access(true, (sc_dt::uint64)addr, reinterpret_cast<unsigned char*>(&tmp), 4);
        tmp = (uint32_t)(data >> 32);
        success2 = bus_b_access(true, (sc_dt::uint64)addr + 4, reinterpret_cast<unsigned char*>(&tmp), 4);
    }
    else
    {
        printb(d_armv7a_bus, "bus_write64 length error (=%d)", length);
    }

    if(success1 && success2)
    {
        printd(core_id, d_armv7a_bus, "bus write64 0x%X:0x%08X%08X", addr, data >> 32, (uint32_t)data);
        return true;
    }
    else
    {
        printm(core_id, d_armv7a_bus, "bus write64 0x%X:0x%08X%08X failed", addr, data >> 32, (uint32_t)data);
        return false;
    }
}

