#include <bit_opt_v2.h>
#include <gic2_dist.h>


gic2_dist::gic2_dist(sc_module_name name, uint32_t mapping_size, uint32_t it_lines_number, bool using_ppi, uint32_t cpu_count, uint32_t ir_count): ahb_slave_if(mapping_size), sc_module(name)
{
    int i;
    int ir;
    int cpu;

    this->en = false;

    this->it_lines_number = it_lines_number;
    this->using_ppi = using_ppi;
    this->cpu_count = cpu_count;
    this->ir_count = ir_count;

    //ppi_n[cpu_count][16]
    if(using_ppi)
    {
        ppi_n = new sc_in<bool>*[cpu_count];
        ppi_n[0] = new sc_in<bool>[cpu_count * 16];

        for(cpu = 1; cpu < cpu_count; cpu++)
        {
            ppi_n[cpu] = ppi_n[0] + cpu * 16;
        }
    }

    //spi_n[ir_count-32];
    spi_n = new sc_in<bool>[ir_count - 32];

    //*ir_infos[cpu_count][ir_count];
    ir_infos = new ir_info** [cpu_count];
    ir_infos[0] = new ir_info*[cpu_count * ir_count];

    for(cpu = 1; cpu < cpu_count; cpu++)
    {
        ir_infos[cpu] = ir_infos[0] + cpu * ir_count;
    }

    //create infos of non-shared interrupts (0~31)
    for(ir = 0; ir <= 31; ir++)
    {
        for(cpu = 0; cpu < cpu_count; cpu++)
        {
            //since its non-shared, it WILL and MUST forward interrupt to its cpu
            ir_infos[cpu][ir] = new ir_info;
            ir_infos[cpu][ir]->forward[cpu] = true;
        }
    }

    //create infos of shared interrupts ( 32~ (ir_count-1) )
    for(ir = 32; ir < ir_count; ir++)
    {
        ir_infos[0][ir] = new ir_info;

        //other cpus just using same info as cpu0
        for(cpu = 1; cpu < cpu_count; cpu++)
        {
            ir_infos[cpu][ir] = ir_infos[0][ir];
        }
    }

    if(using_ppi)
    {
        SC_METHOD(update_pending_ppi);

        for(i = 0; i <= 16; i++)
        {
            for(cpu = 0; cpu < cpu_count; cpu++)
            {
                sensitive << ppi_n[cpu][i];
            }
        }
    }

    SC_METHOD(update_pending_spi);

    for(i = 32; i < ir_count; i++)
    {
        sensitive << spi_n[i - 32];
    }
}

gic2_dist::~gic2_dist()
{
    for(int cpu = 0; cpu < cpu_count; cpu++)
    {
        int ir;

        //delete non-shared ir_info
        for(ir = 0; ir <= 31; ir++)
        {
            delete ir_infos[cpu][ir];
        }

        //delete shared ir_info
        for(ir = 32; ir < ir_count; ir++)
        {
            delete ir_infos[0][ir];
        }
    }

    if(using_ppi)
    {
        delete[]ppi_n;
    }

    delete[]spi_n;
    delete[]ir_infos;
}

void gic2_dist::update_pending_ppi()
{
    /*
       int i;
       int cpu;

       if (using_ppi && en) {
        for (i = 0; i < 16; i++) {
            for (cpu = 0; cpu < cpu_count; cpu++) {
                if (ir_infos[cpu][i + 16].is_inactive() && (ppi_n[cpu][i] == false) ) {
                    ir_infos[cpu][i + 16].set_pending();
                    printd (d_gic2_dist, "update irq(ppi) %d to pending", i + 16);
                }
            }
        }
       }
     */
}


void gic2_dist::update_pending_spi()
{
    int ir;

    if(en)
    {
        //since spi is shared, all cpus has same information, so ir_infos[0][32]==ir_infos[1][32]
        for(ir = 32; ir < ir_count; ir++)
        {
            if(ir_infos[0][ir]->is_inactive() && (spi_n[ir - 32] == false))
            {
                ir_infos[0][ir]->set_pending();
                printd(d_gic2_dist, "new pending irq %d (spi)", ir);
            }
        }
    }
}


bool gic2_dist::read(uint32_t cpu, uint32_t* data, uint32_t offset)
{
    if(offset == 0x4)
    {
        //GICD_TYPER
        *data = 0;
        set_field(data, 15, 11, B(11111)); //LSPI
        set_bit(data, 10, 1); //has security extension
        set_field(data, 7, 5, cpu_count);
        set_field(data, 4, 0, it_lines_number);
    }
    else if(offset == 0x8)
    {
        //GICD_IIDR
        *data = 0x43B;
    }
    else if((offset >= 0x800) && (offset <= 0x8F8))
    {
        //GICD_ITARGETSRn (for n=0~7, register is READ ONLY and banked for each processor)
        //n = 0:  0 ~  3
        //n = 1:  4 ~  7
        //n = 2:  8 ~  11
        int ir;
        int cpu;
        uint32_t n = (offset - 0x800) / 4;
        uint32_t ir_base = n * 4;

        //make sure n > 7
        if(n <= 7)
        {
            printb(d_gic2_dist, "target register n = %d (< 7)", n);
        }

        //with n > 7, we now deal with shared interrupts
        bool forward;
        *data = 0;

        for(ir = ir_base + 3; ir >= ir_base; ir--)
        {
            for(cpu = cpu_count - 1; cpu >= 0; cpu--)
            {
                forward = ir_infos[cpu][ir]->forward[cpu];

                if(forward)
                {
                    set_bit(data, cpu, 1);
                    printd(d_gic2_dist, "read forward cpu[%d] ir[%d]", cpu, ir);
                }
            }

            *data <= 8;
        }

        printd(d_gic2_dist, "read processor target register[%d]=0x%X ir_base = %d", n, *data, ir_base);
    }
    else
    {
        printb(d_gic2_dist, "read error, offset = 0x%X", offset);
    }

    return true;
}


bool gic2_dist::write(uint32_t cpu, uint32_t data, uint32_t offset)
{
    if(offset == 0x0)
    {
        bool group0_en = get_bit(data, 0);
        bool group1_en = get_bit(data, 1);

        if(group0_en && group1_en)
        {
            printb(d_gic2_dist, "enable group0,1 at the same time, not implemented.");
        }
        else if(group0_en || group1_en)
        {
            en = true;
        }
        else
        {
            en = false;
        }
    }
    else if(offset == 0x80)
    {
        //GICD_IGROUPRn, ignore
    }
    else if((offset >= 0x100) && (offset <= 0x17C))
    {
        //GICD_ISENABLERn
        //n = 0:  0 ~ 31 (banked)
        //n = 1: 32 ~ 63 (not banked)
        int i;
        int ir;
        uint32_t n = (offset - 0x100) / 4;
        uint32_t ir_base = n * 32;

        for(i = 0; i < 32; i++)
        {
            ir = ir_base + i;

            if(get_bit(data, i))
            {
                ir_infos[cpu][ir]->en = true;
                printd(d_gic2_dist, "enable irq %d", ir);
            }
        }
    }
    else if((offset >= 0x180) && (offset <= 0x1FC))
    {
        //GICD_ICENABLERn
        //n = 0:  0 ~ 31 (banked)
        //n = 1: 32 ~ 63 (not banked)
        int i;
        int ir;
        uint32_t n = (offset - 0x180) / 4;
        uint32_t ir_base = n * 32;

        for(i = 0; i < 32; i++)
        {
            ir = ir_base + i;

            if(get_bit(data, i))
            {
                ir_infos[cpu][ir]->en = false;
                printd(d_gic2_dist, "disable irq %d", ir);
            }
        }
    }
    else if((offset >= 0x400) && (offset <= 0x7F8))
    {
        //GICD_IPRIORITYn, ignore
    }
    else if((offset >= 0x800) && (offset <= 0x8F8))
    {
        //GICD_ITARGETSRn (for n=0~7, register is READ ONLY and banked for each processor)
        //n = 0:  0 ~  3
        //n = 1:  4 ~  7
        //n = 2:  8 ~  11
        int i;
        int ir;
        uint32_t n = (offset - 0x800) / 4;
        uint32_t ir_base = n * 4;

        //make sure n > 7
        if(n <= 7)
        {
            printb(d_gic2_dist, "target register n = %d (< 7)", n);
        }

        //with n > 7, we now deal with shared interrupts
        bool forward;

        for(i = 0; i < 4; i++)
        {
            ir = ir_base + i;

            for(int cpu = 0; cpu < cpu_count; cpu++)
            {
                forward = get_bit(data, cpu);
                ir_infos[0][ir]->forward[cpu] = forward;
                printd(d_gic2_dist, "%s forwarding irq %d to cpu %d", forward ? "enable" : "disable", ir, cpu);
            }

            data >>= 8;
        }
    }

    else if((offset >= 0xC00) && (offset <= 0xCFC))
    {
        //GICD_ICFGRn, ignore
        uint32_t n = (offset - 0xC00) / 4;

        //printd(d_gic2_dist, "write to GICD_ICFGR");
    }

    else if(offset == 0xF00)
    {
        //GICD_SGIR
        uint32_t target_list_filter = get_field(data, 25, 24);
        uint32_t cpu_target_list = get_field(data, 23, 16);
        uint32_t soft_int_id = get_field(data, 3, 0);

        if(target_list_filter == 0)
        {
            //send to specific cpus
            for(int cpu = 0; cpu < cpu_count; cpu++)
            {
                if(get_bit(cpu_target_list, cpu))
                {
                    ir_infos[cpu][soft_int_id]->set_pending();
                    printd(d_gic2_dist, "send spi[%d] to cpu[%d]", soft_int_id, cpu);
                }
            }
        }

        else if(target_list_filter == 1)
        {
            //send to other cpus
            printb(d_gic2_dist, "send to other cpus");
        }

        else if(target_list_filter == 2)
        {
            //send to itself
            printb(d_gic2_dist, "send to iteself");
        }

        else
        {
            printb(d_gic2_dist, "soft interrupt target_list_filter error (=%d)", target_list_filter);
        }
    }

    else
    {
        printb(d_gic2_dist, "write 0x%X:0x%X error", offset, data);
    }

    return true;
}


bool gic2_dist::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success = false;
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        success = this->write(0, data, local_address);
        //printd (d_gic2_dist, "write 0x%X:0x%X", local_address, data);
    }

    else
    {
        success = read(0, &data, local_address);
        //printd (d_gic2_dist, "read 0x%X:0x%X", local_address, data);
    }

    return success;
}


