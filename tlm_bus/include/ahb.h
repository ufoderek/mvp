#ifndef _AHB_H_
#define _AHB_H_

#include <sys/types.h>

#include <systemc.h>
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>
using namespace tlm;
using namespace tlm_utils;

#include <decoder.h>
#include <debug_utils.h>

class ahb: public sc_module
{
    public:
        unsigned int clk_period_ns;

        multi_passthrough_target_socket<ahb>ahb_from_master_socket;
        //simple_target_socket<ahb> from_master_socket;
        multi_passthrough_initiator_socket<ahb>ahb_to_slave_socket;

        SC_HAS_PROCESS(ahb);
        ahb(sc_module_name nm);

        virtual void b_transport(int id, tlm_generic_payload& trans, sc_time& delay);
        bool add_mapping(unsigned int slave_id, uint32_t base_address, uint32_t mapping_size);

    protected:
        decoder my_decoder;

};
#endif
