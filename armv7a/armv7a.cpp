#include <armv7a.h>
#include <bit_opt_v2.h>
#include <cpu_monitor.h>

armv7a::armv7a(sc_module_name name): sc_module(name)
{
}

armv7a::armv7a(sc_module_name name, const uint32_t& pc, uint32_t core_id, const bool using_gdb, const unsigned int gdb_port): sc_module(name)
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

    //monitor_init();
}

armv7a::~armv7a()
{
    int i;

    delete cpu_monitors_hyp;

    for(i = 0; i < cpu_monitors_vm.size(); i++)
    {
        delete cpu_monitors_vm[i];
    }

    for(i = 0; i < cpu_monitors_proc.size(); i++)
    {
        delete cpu_monitors_proc[i];
    }
}

bool armv7a::inst_fetch(armv7a_ir& inst)
{
#ifdef CPU_ASSERT
    if(rf.current_pc() != align(rf.current_pc(), 4))
    {
        printb(d_armv7a, "inst_fetch with unaligned PC (0x%X)", rf.current_pc());
    }
#endif

    inst.set_pc(rf.current_pc());

    address_descriptor mem_addr_desc;

    if(!translate_address(&mem_addr_desc, rf.current_pc(), rf.current_mode_is_not_user(), false, 4, true, false))
    {
        //take_prefetch_abort_exception has been executed, now pc's value is 0xFFFF000C
        //printb(d_armv7a, "pabrt");
        return false;
    }

    bits data;
    _mem_read(&data, mem_addr_desc, 4);
    inst.val = data.val;
    //printd(d_s2mmu, "INST_FETCH DONE 0x%X:0x%X", mem_addr_desc.p_address.physical_address, inst.val);
    return true;
}

void armv7a::execute()
{
    bool do_irq1 = false;
    bool do_irq2 = false;
    bool do_virq = false;

    //no hypvervisor
    if(cp15.read(HCR_IMO) == 0)
    {
        do_irq1 = (irq_n == false) && (rf.cpsr_I() == 0);
        do_irq2 = (irq_n == false) && wfi;
    }
    //with hypervisor
    else
    {
        //physical irq taken to hyp mode
        if(rf.current_mode_is_hyp())
        {
            do_irq1 = (irq_n == false) && (rf.cpsr_I() == 0);
            do_irq2 = (irq_n == false) && wfi;
        }
        else
        {
            do_irq1 = (irq_n == false);
            do_virq = (virq_n == false) && (rf.cpsr_I() == 0);
        }
    }

    if(do_irq1 || do_irq2)
    {
        if(wfi == true)
        {
            printd(d_inst, "exit wfi");
            wfi = false;
        }
        else
        {
            take_physical_irq_exception();
        }
    }
    else if(do_virq)
    {
        take_virtual_irq_exception();
    }

    //for arm_wfi()
    if(wfi)
    {
        return;
    }

    if(using_gdb)
    {
        if(bkptCheck(rf.current_pc()) || gdb->step)
        {
            gdb->reason = GDB_SIGNAL_TRAP;
            gdb->step = false;
            gdb->stop = true;
        }

        if(gdb->stop)
        {
            gdb->stub();
        }
    }

    if(!inst_fetch(inst))
    {
        //prefetch abort
        return;
    }

    //monitor();

    dump_rf();

    rf.incre_pc();

    decode_top(inst); //decode and execute

}

void armv7a::dump_rf()
{
    if(inst.hyp_mode())
    {
        printd(d_rf_dump, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
               rf.r_read(0), rf.r_read(1), rf.r_read(2), rf.r_read(3), rf.r_read(4), rf.r_read(5), rf.r_read(6),
               rf.r_read(7), rf.r_read(8), rf.r_read(9), rf.r_read(10), rf.r_read(11), rf.r_read(12), rf.r_read(13),
               rf.r_read(14), inst.get_pc(), rf.cpsr());
    }
    else
    {
        switch(inst.vm_id())
        {
            case 0:
                printd(d_rf_dump_vm0, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
                       rf.r_read(0), rf.r_read(1), rf.r_read(2), rf.r_read(3), rf.r_read(4), rf.r_read(5), rf.r_read(6),
                       rf.r_read(7), rf.r_read(8), rf.r_read(9), rf.r_read(10), rf.r_read(11), rf.r_read(12), rf.r_read(13),
                       rf.r_read(14), inst.get_pc(), rf.cpsr());
                break;
            case 1:
                printd(d_rf_dump_vm1, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
                       rf.r_read(0), rf.r_read(1), rf.r_read(2), rf.r_read(3), rf.r_read(4), rf.r_read(5), rf.r_read(6),
                       rf.r_read(7), rf.r_read(8), rf.r_read(9), rf.r_read(10), rf.r_read(11), rf.r_read(12), rf.r_read(13),
                       rf.r_read(14), inst.get_pc(), rf.cpsr());
                break;
            case 2:
                printd(d_rf_dump_vm2, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
                       rf.r_read(0), rf.r_read(1), rf.r_read(2), rf.r_read(3), rf.r_read(4), rf.r_read(5), rf.r_read(6),
                       rf.r_read(7), rf.r_read(8), rf.r_read(9), rf.r_read(10), rf.r_read(11), rf.r_read(12), rf.r_read(13),
                       rf.r_read(14), inst.get_pc(), rf.cpsr());
                break;
            case 3:
                printd(d_rf_dump_vm3, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
                       rf.r_read(0), rf.r_read(1), rf.r_read(2), rf.r_read(3), rf.r_read(4), rf.r_read(5), rf.r_read(6),
                       rf.r_read(7), rf.r_read(8), rf.r_read(9), rf.r_read(10), rf.r_read(11), rf.r_read(12), rf.r_read(13),
                       rf.r_read(14), inst.get_pc(), rf.cpsr());
                break;
        }
    }
}

void armv7a::monitor_init()
{
    const unsigned int size = 800;

    cpu_monitors_hyp = new cpu_monitor("CASL Hypervisor", "Simulation Time (ms)", "CPU Usage (%)", size);

    cpu_monitors_vm.push_back(new cpu_monitor("Guest 0", "Simulation Time (ms)", "CPU Usage (%)", size));
    cpu_monitors_vm.push_back(new cpu_monitor("Guest 1", "Simulation Time (ms)", "CPU Usage (%)", size));
    cpu_monitors_vm.push_back(new cpu_monitor("Guest 2", "Simulation Time (ms)", "CPU Usage (%)", size));
    cpu_monitors_vm.push_back(new cpu_monitor("Guest 3", "Simulation Time (ms)", "CPU Usage (%)", size));
}

void armv7a::monitor()
{
    const uint64_t interval_ms = 15;
    const uint64_t interval_ns = interval_ms  * 1000000;

    static uint64_t inst_count_ns = 0;
    static uint64_t hyp = 0;
    static bool hyp_valid = false;
    static uint64_t vm[4] = {0, 0, 0, 0};
    static bool vm_valid[4] = {false, false, false, false};
    static vector<uint64_t> proc;

    //default time unit = 1 ns, don't ask me why
    uint64_t current_time_ns = sc_time_stamp().to_default_time_units();
    uint64_t current_time_ms = current_time_ns / 1000000;

    inst_count_ns++;

    if(rf.current_mode_is_hyp())
    {
        //count hypervisor
        hyp_valid = true;
        hyp++;
    }
    else
    {
        //count vm
        uint32_t vm_id = cp15.read64(VTTBR_LPAE_VMID);
        if(!vm_valid[vm_id])
        {
            if(current_time_ms > 1)
            {
                cpu_monitors_vm[vm_id]->add(current_time_ms - 1, 0);
            }
        }
        vm_valid[vm_id] = true;
        vm[vm_id]++;

        //count process
        /*
        if(rf.current_mode_is_user_or_system())
        {
            uint32_t proc_id = cp15.read(CONTEXTIDR_ASID);
            uint32_t index = (vm_id << 16) | (proc_id & 0xFFFF);

            //new process
            if(proc_map.find(index) == proc_map.end())
            {
                char title[128];

                sprintf(title, "Guest %u Process %u", index & 0xFFFF, index >> 16);

                proc_map[index] = cpu_monitors_proc.size();

                cpu_monitors_proc.push_back(new cpu_monitor(title, "Simulation Time (ms)", "CPU Usage (%)", 800));
                proc.push_back(0);
            }

            proc[proc_map[index]]++;
        }
        */
    }

    if(current_time_ms <= 900)
    {
        if((inst_count_ns % interval_ns) == 0)
        {
            int i;

            //plot vm
            for(i = 0; i < cpu_monitors_vm.size(); i++)
            {
                if(vm_valid[i])
                {
                    cpu_monitors_vm[i]->add(current_time_ms, (double)vm[i] / (double)inst_count_ns * 100);
                    cpu_monitors_vm[i]->plot();
                    vm[i] = 0;
                }
            }
            //plot process
            /*
            for(i = 0; i < cpu_monitors_proc.size(); i++)
            {
                cpu_monitors_proc[i]->add(current_time_ms, (double)proc[i] / (double)inst_count_ns * 100);
                //cpu_monitors_proc[i]->plot();
                proc[i] = 0;
            }
            */
            //plot hypervisor
            if(hyp_valid)
            {
                cpu_monitors_hyp->add(current_time_ms, (double)hyp / (double)inst_count_ns * 100);
                cpu_monitors_hyp->plot();
                hyp = 0;
            }

            inst_count_ns = 0;
        }
    }
}
