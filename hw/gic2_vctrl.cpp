#include <gic2_vctrl.h>

gic2_vctrl::gic2_vctrl(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    en = false;
}

gic2_vctrl::~gic2_vctrl()
{
}

bool gic2_vctrl::read(uint32_t* data, uint32_t offset)
{
    if(offset == 0x0)
    {
        *data = en;
    }
    else if(offset == 0x30)
    {
        //ELSR0, for list[0]~list[31]
        *data = 0;
        for(int i = 0; i <= 31; i++)
        {
            if(!list[i].valid)
            {
                set_bit(data, i, 1);
            }
        }
    }
    else if(offset == 0x34)
    {
        //ELSR1, for list[32]~list[63]
        *data = 0;
        for(int i = 32; i <= 63; i++)
        {
            if(!list[i - 32].valid)
            {
                set_bit(data, i - 32, 1);
            }
        }
    }
    else if((offset >= 0x100) && (offset <= 0x1FC))
    {
        //list[n]
        uint32_t n = (offset - 0x100) / 4;

        *data = 0;
        set_bit(data, 31, list[n].hw);
        set_bit(data, 30, list[n].group1);
        set_field(data, 29, 28, list[n].state);
        set_field(data, 27, 23, list[n].priority);
        set_field(data, 19, 10, list[n].pid);
        set_field(data, 9, 0, list[n].vid);
        printd(d_gic2_vctrl, "read list[%d], pid = %d, vid = %d", n, list[n].pid, list[n].vid);
    }
    else
    {
        printb(d_gic2_vctrl, "read 0x%X error", offset);
    }

    return true;
}

bool gic2_vctrl::write(uint32_t data, uint32_t offset)
{
    if(offset == 0x0)
    {
        en = get_bit(data, 0);
        printd(d_gic2_vctrl, "%s", en ? "enable" : "disable");
    }
    else if((offset >= 0x100) && (offset <= 0x1FC))
    {
        //list[n]
        uint32_t n = (offset - 0x100) / 4;

        list[n].hw = get_bit(data, 31);
        list[n].group1 = get_bit(data, 30);
        list[n].state = get_field(data, 29, 28);
        list[n].priority = get_field(data, 27, 23);
        list[n].pid = get_field(data, 19, 10);
        list[n].vid = get_field(data, 9, 0);

        printd(d_gic2_vctrl, "write list[%d], pid = %d, vid = %d", n, list[n].pid, list[n].vid);

        if(list[n].state != LR_Invalid)
        {
            printd(d_gic2_vctrl, "write list[%d], pid = %d, vid = %d", n, list[n].pid, list[n].vid);
        }
    }
    else
    {
        printb(d_gic2_vctrl, "write 0x%X:0x%X error", offset, data);
    }

    return true;
}

bool gic2_vctrl::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        success = this->write(data, local_address);
    }
    else
    {
        success = this->read(&data, local_address);
    }

    return success;
}

