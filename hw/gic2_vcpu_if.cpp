#include <bit_opt_v2.h>
#include <gic2_vcpu_if.h>


gic2_vcpu_if::gic2_vcpu_if(sc_module_name name, uint32_t mapping_size, boost::shared_ptr<gic2_vctrl> gic_vctrl, boost::shared_ptr<gic2_cpu_if>gic_cpu_if): ahb_slave_if(mapping_size), sc_module(name)
{
    this->gic_vctrl = gic_vctrl;
    this->gic_cpu_if = gic_cpu_if;

    group0_en = false;
    group1_en = false;

    busy = false;

    SC_METHOD(forward_pending_virq);
    sensitive << clk.pos();
}

gic2_vcpu_if::~gic2_vcpu_if()
{
}

void gic2_vcpu_if::forward_pending_virq()
{
    if(gic_vctrl->en && this->group0_en)
    {
        //list register is unchanged
        if(busy && (gic_vctrl->list[fw_list].state == LR_Pending))
        {
            virq_n = false;
        }
        //list register is changed (vm-switch) or no pending virq
        else
        {
            //find a pending virq
            int i;
            for(int i = 0; i <= 63; i++)
            {
                fw_list++;
                if(fw_list == 64)
                {
                    fw_list = 0;
                }
                if(gic_vctrl->list[fw_list].state == LR_Pending)
                {
                    busy = true;
                    virq_n = false;
                    printd(d_gic2_vcpu_if, "forward pid = %d, vid = %d", gic_vctrl->list[fw_list].pid, gic_vctrl->list[fw_list].vid);
                    return;
                }
            }
            //nothing to signal cpu
            busy = false;
            virq_n = true;
        }
    }
    else
    {
        busy = false;
        virq_n = true;
    }
}


bool gic2_vcpu_if::read(uint32_t* data, uint32_t offset)
{
    if(offset == 0x0)
    {
        //GICC_CTLR
        *data = group0_en;
        set_bit(data, 1, group1_en);

    }
    else if(offset == 0x0C)
    {
        //GICC_IAR, cpu ack
        uint32_t cpu_id = 0;
        if(busy && (gic_vctrl->list[fw_list].state == LR_Pending))
        {
            busy = false;
            gic_vctrl->list[fw_list].state = LR_Active;

            *data = 0;
            set_field(data, 9, 0, gic_vctrl->list[fw_list].vid);
            set_field(data, 12, 10, cpu_id);

            printd(d_gic2_vcpu_if, "read interrupt ack %d", gic_vctrl->list[fw_list].vid);
        }
        else
        {
            *data = 1023;
            printd(d_gic2_vcpu_if, "read spurious interrupt 1023");
        }
    }
    else
    {
        printb(d_gic2_vcpu_if, "read offset 0x%X error", offset);
    }

    return true;
}


bool gic2_vcpu_if::write(uint32_t data, uint32_t offset)
{
    if(offset == 0x0)
    {
        //GICC_CTLR
        group0_en = get_bit(data, 0);
        group1_en = get_bit(data, 1);

        if(group0_en && group1_en)
        {
            printb(d_gic2_vcpu_if, "enable group0,1 at the same time, not implemented.");
        }
        else if(group0_en)
        {
            printd(d_gic2_vcpu_if, "enable cpu_if");
        }
        else
        {
            printd(d_gic2_vcpu_if, "disable cpu_if");
        }
    }
    else if(offset == 0x4)
    {
        //GICC_PMR, ignore priority mask
        printd(d_gic2_vcpu_if, "ignore priority mask = 0x%X", data);
    }
    else if(offset == 0x10)
    {
        //GICC_EOIR, end of interrupt
        uint32_t vid = get_field(data, 9, 0);
        uint32_t src_cpu = get_field(data, 12, 10);

        /*
        int vid = gic_vctrl->list[busy_list].vid;
        int pid = gic_vctrl->list[busy_list].pid;

        bool check1 = gic_vctrl->list[busy_list].state == LR_Active;
        bool check2 = gic_vctrl->list[busy_list].vid == ir;

        if(check1 && check2)
        {
            gic_vctrl->list[busy_list].state = LR_Invalid;
            gic_cpu_if->write(0, pid, 0x10);
        }
        else
        {
            printb(d_gic2_vcpu_if, "end of interrupt error: vid = %d, pid = %d", vid, pid);
        }
        */

        for(int i = 0; i <= 63; i++)
        {
            if((gic_vctrl->list[i].vid == vid) && (gic_vctrl->list[i].state == LR_Active))
            {
                gic_vctrl->list[i].state = LR_Invalid;
                gic_cpu_if->write(0, gic_vctrl->list[i].pid, 0x10);
                printd(d_gic2_vcpu_if, "end of interrupt: pid = %d, vid = %d", gic_vctrl->list[i].pid, vid);
                return true;
            }
        }
        printb(d_gic2_vcpu_if, "error end of interrupt: vid = %d", vid);

    }
    else
    {
        printb(d_gic2_vcpu_if, "write 0x%X:0x%X error", offset, data);
    }

    return true;
}


bool gic2_vcpu_if::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    bool success;
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        success = this->write(data, local_address);
        //printd (d_gic2_vcpu_if, "write 0x%X:0x%X", local_address, data);
    }
    else
    {
        success = read(&data, local_address);
        //printd (d_gic2_vcpu_if, "read 0x%X:0x%X", local_address, data);
    }

    return success;
}

