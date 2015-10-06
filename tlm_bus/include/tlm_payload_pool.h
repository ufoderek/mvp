#ifndef _TLM_PAYLOAD_POOL_
#define _TLM_PAYLOAD_POOL_

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <cstdio>
#include <stack>
#include <systemc.h>
#include <tlm.h>
using namespace std;
using namespace tlm;
#include <debug_utils.h>

class tlm_payload_pool: public tlm::tlm_mm_interface
{
    public:
        tlm_payload_pool()
        {
            //allocate 5 free payloads before simulation
            for(int i = 0; i < 5; i++)
            {
                pool.push(new tlm_generic_payload(this));
            }
        }

        ~tlm_payload_pool()
        {
            while(!pool.empty())
            {
                //stack.top().reset();  // free the payload extensions, which we didn't use
                delete pool.top();
                pool.pop();
            }
        }

        tlm_generic_payload* allocate()
        {
            tlm_generic_payload* payload;

            if(pool.empty())
            {
                printd(d_tlm_payload_pool, "allocate a payload from memory");
                payload = new tlm_generic_payload(this);
            }

            else
            {
                printd(d_tlm_payload_pool, "get a payload from stack");
                payload = pool.top();
                pool.pop();
            }

            return payload;
        }

        void free(tlm_generic_payload* junk)
        {
            printd(d_tlm_payload_pool, "push the junk payload to stack");
            pool.push(junk);
        }

        virtual const char* get_debug_header()
        {
            return "trans_pool";
        }

    private:
        stack<tlm_generic_payload*>pool;
};
#endif

