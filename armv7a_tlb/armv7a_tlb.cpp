#include <armv7a_tlb.h>

armv7a_tlb::armv7a_tlb(sc_module_name name, const uint32_t& pc, uint32_t core_id, const bool using_gdb, const unsigned int gdb_port): armv7a(name)
{
    rf.reset_pc(0);
    wfi = false;
    this->using_gdb = using_gdb;
    this->core_id = core_id;
    cp15.reset_a8();
    cp15.reset_core_id(this->core_id);
    gdb = 0;

    if(using_gdb)
    {
        this->using_gdb = using_gdb;
        this->gdb_port = gdb_port;
        gdb_init = false;
        gdb = new GDBStub(this, gdb_port);
    }

    SC_METHOD(execute);
    sensitive << clk.pos();
    dont_initialize();
}

armv7a_tlb::~armv7a_tlb()
{
}

