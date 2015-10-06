#include <ahb_master_if.h>

ahb_master_if::ahb_master_if(): ahb_master_socket("ahb_master_socket")
{
}

bool ahb_master_if::bus_b_access(bool write, sc_dt::uint64 addr, unsigned char* data, unsigned int length, sc_time delay)
{
    tlm_generic_payload payload;
    payload.set_command(write ? TLM_WRITE_COMMAND : TLM_READ_COMMAND);
    payload.set_address(addr);
    payload.set_data_ptr(data);
    payload.set_data_length(length);
    payload.set_streaming_width(length); //= data_length, means no streaming
    payload.set_byte_enable_ptr(0);
    payload.set_dmi_allowed(false);
    payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

    if(delay.to_seconds() > 0.0)
    {
        wait(delay);
    }

    ahb_master_socket->b_transport(payload, delay);

    if(payload.is_response_error())
    {
        printd(d_ahb_master_if, "transaction returned with error: %s", payload.get_response_string().c_str());
        return false;
    }

    return true;
}

