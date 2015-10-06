#include <ahb_slave_if.h>

ahb_slave_if::ahb_slave_if(uint32_t mapping_size): ahb_slave_socket("ahb_slave_socket")
{
    if(is_power_of_two(mapping_size))
    {
        ahb_slave_socket.register_b_transport(this, &ahb_slave_if::b_transport);
        this->address_mask = mapping_size - 1;
    }
    else
    {
        printb(d_ahb_slave_if, "illegal mapping size");
    }
}

void ahb_slave_if::b_transport(tlm_generic_payload& payload, sc_time& delay)
{
    tlm_command cmd = payload.get_command();
    uint32_t addr = (uint32_t)payload.get_address();
    uint32_t* data_ptr = (uint32_t*)payload.get_data_ptr();
    unsigned int length = payload.get_data_length();

    /*
       if(addr & get_address_mask() >= 0)
       {
        payload.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
       }
       else*/

    if(length > 4)
    {
        printb(d_ahb_slave_if, "length error");
        payload.set_response_status(TLM_BURST_ERROR_RESPONSE);
    }

    else
    {
        bool success;
        uint32_t data;

        if(cmd == TLM_READ_COMMAND)
        {
            success = local_access(false, addr, data, length);
            printd(d_ahb_slave_if, "read from 0x%X: 0x%X", addr, data);

            if(success)
            {
                *data_ptr = data;
                payload.set_response_status(TLM_OK_RESPONSE);
            }

            else
            {
                printm(d_ahb_slave_if, "read error");
                payload.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
            }

            //memcpy(ptr, &mem[adr], len);
        }

        else if(cmd == TLM_WRITE_COMMAND)
        {
            data = *data_ptr;
            success = local_access(true, addr, data, length);
            printd(d_ahb_slave_if, "write to 0x%X: 0x%X", addr, data);

            if(success)
            {
                payload.set_response_status(TLM_OK_RESPONSE);
            }

            else
            {
                printb(d_ahb_slave_if, "write error");
                payload.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
            }

            //memcpy(&mem[adr], ptr, len);
        }

        else
        {
            printb(d_ahb_slave_if, "no such command");
            payload.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        }
    }
}
uint32_t ahb_slave_if::get_address_mask()
{
    return address_mask;
}

inline bool ahb_slave_if::is_power_of_two(uint32_t x)
{
    return((x != 0) && ((x & (~x + 1)) == x));
}

