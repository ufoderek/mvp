#ifndef _AHB_MASTER_IF_H_
#define _AHB_MASTER_IF_H_

#include <stdint.h>

#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
using namespace tlm;
using namespace tlm_utils;

#include <tlm_payload_pool.h>
#include <debug_utils.h>

//Initiator module generating generic payload transactions

class ahb_master_if
{
    public:
        //TLM-2 socket, defaults to 32-bits wide, base protocol
        simple_initiator_socket<ahb_master_if>ahb_master_socket;
        ahb_master_if();
        ~ahb_master_if()
        {
        }

    protected:
        bool bus_b_access(bool write, sc_dt::uint64 addr, unsigned char* data, unsigned int length, sc_time delay = SC_ZERO_TIME);

    private:
        //tlm_payload_pool payload_pool;
};

#endif
