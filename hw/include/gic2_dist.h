#ifndef _GIC2_DIST_H_
#define _GIC2_DIST_H_

#include <systemc.h>
#include <ahb_slave_if.h>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

enum ir_state
{
    Pending,
    Active,
    Inactive
};

class ir_info
{
    public:
        ir_info()
        {
            en = false;

            for(int i = 0; i < 8; i++)
            {
                forward[i] = false;
            }

            src_cpu = 0;
            set_inactive();
        }
        bool is_pending()
        {
            return state == Pending;
        }
        bool is_inactive()
        {
            return state == Inactive;
        }
        void set_pending()
        {
            state = Pending;
        }
        void set_active()
        {
            state = Active;
        }
        void set_inactive()
        {
            state = Inactive;
        }
        uint32_t get_src_cpu()
        {
            return src_cpu;
        }
        void clear_src_cpu()
        {
            src_cpu = 0;
        }
        bool en;
        bool forward[8];
        uint32_t src_cpu;
        ir_state state;
};

class gic2_dist: public ahb_slave_if, public sc_module
{
    public:
        sc_in<bool>** ppi_n;         //banked interrupt input: array [cpu_count][16], for PPI 16~32
        sc_in<bool>*spi_n;          //interrupt input: array [32~ir_count], for SPI 33~63

        SC_HAS_PROCESS(gic2_dist);
        gic2_dist(sc_module_name name, uint32_t mapping_size, uint32_t it_lines_number, bool using_ppi, uint32_t cpu_count, uint32_t ir_count);
        ~gic2_dist();

        void update_pending_ppi();
        void update_pending_spi();

        bool read(uint32_t cpu, uint32_t* data, uint32_t offset);
        bool write(uint32_t cpu, uint32_t data, uint32_t offset);
        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

        ir_info*** ir_infos; //array [cpu_count][ir_count]

        bool en;
    private:
        bool using_ppi;
        uint32_t it_lines_number;
        uint32_t ir_count;
        uint32_t cpu_count;

};

#endif

