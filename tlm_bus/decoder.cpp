#include <decoder.h>

decoder::decoder()
{
    uint32_t i;
    printm(d_decoder, "allocating memory...");
    map = new char[map_size];
    printm(d_decoder, "memory allocated");

    for(i = 0; i < map_size; i++)
    {
        map[i] = -1;
    }

    printd(d_decoder, "map_size: 0x%X, check i: 0x%X", map_size, i);
}

decoder::~decoder()
{
    if(map != 0)
    {
        delete[]map;
    }
}

bool decoder::add_mapping(unsigned int slave_id, uint32_t base_address, uint32_t mapping_size)
{
    uint32_t end_address;
    end_address = ((base_address + mapping_size) >> shift);
    base_address >>= shift;
    printm(d_decoder, "id: %u, 0x%X - 0x%X (%d KiB)", slave_id, base_address << shift, end_address << shift, mapping_size / 1024);
    //printm(d_decoder, "base address: 0x%X, end address: 0x%X", base_address << shift, end_address << shift);

    for(uint32_t i = base_address; i < end_address; i++)
    {
        if(map[i] == -1)
        {
            map[i] = slave_id;
        }

        else
        {
            printb(d_decoder, "mapping overlapped");
        }
    }

    return true;
}

bool decoder::decode(unsigned int& slave_id, uint32_t address)
{
    uint32_t inter_addr = address >> shift;

    if(inter_addr < map_size)
    {
        int tmp_id = map[inter_addr];

        if(tmp_id >= 0)
        {
            slave_id = tmp_id;
            /*
               if(slave_id !=0)
               {
                printd(d_decoder, "decode address to bus_id: %d", tmp_id);
               }
             */
            return true;
        }
    }

    printm(d_decoder, "decoding error, bus_id: %u, phy address: 0x%X", slave_id, address);
    printd(d_decoder, "should exit here");
    return false;
}

