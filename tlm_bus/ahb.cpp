#include <ahb.h>

ahb::ahb(sc_module_name name): sc_module(name), ahb_from_master_socket("ahb_from_master_socket"), ahb_to_slave_socket("ahb_to_slave_socket")
{
    ahb_from_master_socket.register_b_transport(this, &ahb::b_transport);
}

bool ahb::add_mapping(uint32_t slave_id, uint32_t base_address, uint32_t mapping_size)
{
    return my_decoder.add_mapping(slave_id, base_address, mapping_size);
}

void ahb::b_transport(int id, tlm_generic_payload& payload, sc_time& delay)
{
    unsigned int slave_id;
    bool success = false;
    uint32_t addr = (uint32_t)payload.get_address();
    success = my_decoder.decode(slave_id, addr);

    if(!success)
    {
        printd(d_ahb, "address decoding error: %X", addr);
        payload.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    }

    else
    {
        ahb_to_slave_socket[slave_id]->b_transport(payload, delay);
    }

    printd(d_ahb, "transfer done");
}
