#ifndef _ARMV6K_H_
#define _ARMV6K_H_

#include <armv6.h>
#include <memory.h>
#include <mp_scu.h>
#include <debug_utils.h>
#include <boost/shared_ptr.hpp>
#include <gic2_dist.h>
#include <gic2_cpu_if.h>

class ARMV6K: public ARMV6
{
        /* basic */
    public:
        SC_HAS_PROCESS(ARMV6K);
        ARMV6K(sc_module_name name, unsigned int core_id, boost::shared_ptr<mp_scu> scu, boost::shared_ptr<gic2_dist> gic_dist, boost::shared_ptr<gic2_cpu_if> gic_cpu_if, uint32_t addr, bool using_gdb, unsigned int gdb_port);
        ~ARMV6K();

        /* SCU */
    protected:
        boost::shared_ptr<mp_scu> scu;
        boost::shared_ptr<gic2_dist> gic_dist;
        boost::shared_ptr<gic2_cpu_if> gic_cpu_if;

        /* ARMV6K pesudo functions */
    public:
        void send_event();
        /*
        bool event_registered(){}
        void clear_event_regester() {}
        */

        /* mmu */
    protected:
        bool bus_read(uint32_t* data, uint32_t addr, unsigned int length);
        bool bus_write(uint32_t data, uint32_t addr, unsigned int length);
};

#endif

