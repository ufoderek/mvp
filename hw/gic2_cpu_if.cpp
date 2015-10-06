#include <bit_opt_v2.h>
#include <gic2_cpu_if.h>


gic2_cpu_if::gic2_cpu_if(sc_module_name name, uint32_t mapping_size, boost::shared_ptr<gic2_dist>dist, uint32_t cpu_count, uint32_t ir_count): ahb_slave_if(mapping_size), sc_module(name)
{
    this->cpu_count = cpu_count;
    this->ir_count = ir_count;
    this->dist = dist;

    this->irq_n = new sc_out<bool>[cpu_count];
    this->if_infos = new cpu_if_info[cpu_count];

    SC_METHOD(forward_pending_irq);
    sensitive << clk.pos();
}

gic2_cpu_if::~gic2_cpu_if()
{
    delete[]irq_n;
    delete[]if_infos;
}

void gic2_cpu_if::forward_pending_irq()
{
    uint32_t cpu;

    for(cpu = 0; cpu < cpu_count; cpu++)
    {
        //if cpu if is busy and interface is enabled
        if(if_infos[cpu].busy && if_infos[cpu].en && dist->en)
        {
            //keep forwarding irq
            irq_n[cpu] = false;
        }
        //if cpu if is not busy and interface is enabled
        else if(!if_infos[cpu].busy && if_infos[cpu].en && dist->en)
        {
            uint32_t ir = if_infos[cpu].fw_ir;

            //find next pending ir
            for(int c = 0; c < ir_count; c++)
            {
                ir++;

                if(ir >= ir_count)
                {
                    ir = 0;
                }

                //now forwarding this pending interrupt and set cpu interface busy
                if(dist->ir_infos[cpu][ir]->forward[cpu] && dist->ir_infos[cpu][ir]->is_pending())
                {
                    printd(cpu, d_gic2_cpu_if, "forward irq %d for cpu_if[%d], now cpu if is busy", ir, cpu);
                    if_infos[cpu].fw_ir = ir;
                    if_infos[cpu].busy = true;
                    irq_n[cpu] = false;
                    return;
                }
            }
            //find nothing, no interrupt to forward
            irq_n[cpu] = true;
        }
        else
        {
            irq_n[cpu] = true;
        }
    }
}


bool gic2_cpu_if::read(uint32_t cpu, uint32_t* data, uint32_t offset)
{
    if(offset == 0x0C)
    {
        //GICC_IAR, cpu ack
        uint32_t fw_ir = if_infos[cpu].fw_ir;

        if(dist->ir_infos[cpu][fw_ir]->is_pending() && if_infos[cpu].busy)
        {
            dist->ir_infos[cpu][fw_ir]->set_active();
            if_infos[cpu].busy = false;
            *data = 0;
            set_field(data, 9, 0, fw_ir);

            if(fw_ir < 16)
            {
                //SGI, so set source cpu id
                uint32_t src_cpu = dist->ir_infos[cpu][fw_ir]->get_src_cpu();
                set_field(data, 12, 10, src_cpu);
                printd(cpu, d_gic2_cpu_if, "read soft interrupt ack %d from cpu %d", fw_ir, src_cpu);
            }
            else
            {
                printd(cpu, d_gic2_cpu_if, "read interrupt ack %d", fw_ir);
            }
        }
        else
        {
            *data = 1023;
            printd(cpu, d_gic2_cpu_if, "read spurious interrupt 1023");
        }
    }
    else
    {
        printb(cpu, d_gic2_cpu_if, "read offset 0x%X error", offset);
    }

    return true;
}


bool gic2_cpu_if::write(uint32_t cpu, uint32_t data, uint32_t offset)
{
    if(offset == 0x0)
    {
        //GICC_CTLR
        bool group0_en = get_bit(data, 0);
        bool group1_en = get_bit(data, 1);

        if(group0_en && group1_en)
        {
            printb(cpu, d_gic2_cpu_if, "enable group0,1 at the same time, not implemented.");
        }
        else if(group0_en || group1_en)
        {
            if_infos[cpu].en = true;
            printd(cpu, d_gic2_cpu_if, "enable cpu_if[%d]", cpu);
        }
        else
        {
            if_infos[cpu].en = false;
            printd(cpu, d_gic2_cpu_if, "disable cpu_if[%d]", cpu);
        }
    }
    else if(offset == 0x4)
    {
        //GICC_PMR, ignore priority mask
        printd(cpu, d_gic2_cpu_if, "ignore priority mask = 0x%X", data);
    }
    else if(offset == 0x10)
    {
        //GICC_EOIR, end of interrupt
        uint32_t ir = get_field(data, 9, 0);
        uint32_t src_cpu = get_field(data, 12, 10);
        //uint32_t fw_ir = if_infos[cpu].fw_ir;

        /*
        if(ir != fw_ir)
        {
            //check for right ack interrupt id
            printb(cpu, d_gic2_cpu_if, "end of interrupt %d doesn't match with forwarded interrupt %d", ir, fw_ir);
        }
        */

        if((ir < 16) && (src_cpu != dist->ir_infos[cpu][ir]->get_src_cpu()))
        {
            //SGI, check for right source cpu id
            printb(cpu, d_gic2_cpu_if, "end of soft interrupt %d doesn't match source cpu %d", ir, dist->ir_infos[cpu][ir]->get_src_cpu());
        }

        /*
        dist->ir_infos[cpu][fw_ir]->set_inactive();
        dist->ir_infos[cpu][fw_ir]->clear_src_cpu();
        */
        dist->ir_infos[cpu][ir]->set_inactive();
        dist->ir_infos[cpu][ir]->clear_src_cpu();
        printd(cpu, d_gic2_cpu_if, "end of interrupt: %d", ir);
    }
    else
    {
        printb(d_gic2_cpu_if, "write 0x%X:0x%X error", offset, data);
    }

    return true;
}


bool gic2_cpu_if::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        success = this->write(0, data, local_address);
        //printd (d_gic2_cpu_if, "write 0x%X:0x%X", local_address, data);
    }
    else
    {
        success = this->read(0, &data, local_address);
        //printd (d_gic2_cpu_if, "read 0x%X:0x%X", local_address, data);
    }

    return success;
}

