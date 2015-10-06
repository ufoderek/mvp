#include <fcntl.h>

#include <board_meteorvb.h>

void board_meteorvb::load_vmm(char* path_vmm, uint32_t addr)
{
    printm(d_meteorvb, "loading vmm...");

    if(path_vmm != NULL)
    {
        int fid = open(path_vmm, O_RDONLY);

        if(fid > 0)
        {
            uint32_t buff;
            int count;
            bool result;

            do
            {
                count = read(fid, &buff, 4);
                result = ram->write(buff, addr, 4);

                if(result == false)
                {
                    printb(d_meteorvb, "cannot load vmm image");
                }
                else
                {
                    addr += 4;
                }
            }
            while(count > 0);
        }

        else
        {
            printb(d_meteorvb, "cannot open vmm image");
        }
    }
}

board_meteorvb::~board_meteorvb(void)
{
}

#define print_line() printm(d_meteorvb,"===============================================");

board_meteorvb::board_meteorvb(sc_module_name name, bool using_gdb, unsigned int gdb_port): sc_module(name)
{
    int i;
    uint32_t slave_id = 0;
    start_addr = 0;
    //0x0 ~ 0x20000000
    size_ram = 1024 << 20; //in bytes

    /*
    40 MHz
    = 40 * 10^6 Hz
    = 4 * 10^7 Hz
    = 0.25 * 10^-7 s
    = 25 * 10^-9 s
    = 25 ns

    200 MHz
    = 200 * 10^6 Hz
    = 2 * 10^8 Hz
    = 0.5 * 10^-8 s
    = 5 * 10^-9 s
    = 5 ns
    */
    clk_fast.reset(new sc_clock("clk_fast", 1, SC_NS, 0.5)); //1000MHz clock
    clk_slow.reset(new sc_clock("clk_slow", 2.5, SC_NS, 0.5)); //400MHz clock
    clk_timer.reset(new sc_clock("clk_timer", 25.0, SC_NS, 0.5)); //40MHz clock

    rst_n.write(true); //reset signal
    irq_n.write(true); //IRQ signal to CPU
    virq_n.write(true); //virtual IRQ signal to CPU

    for(i = 0; i < 16 + 16 + 32; i++)   //sgi + ppi + spi = 16 + 16 + 32
    {
        channel[i].write(true);
    }

    print_line();
    printm(d_meteorvb, "AHB Bus");
    my_bus.reset(new ahb("ahb_bus"));

    print_line();
    printm(d_meteorvb, "Fake DRAM, size: %x", size_ram);
    ram.reset(new RAM("RAM", size_ram));
    my_bus->ahb_to_slave_socket.bind(ram->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0, size_ram);

    print_line();
    printm(d_meteorvb, "ARMv7-A");
    arm.reset(new armv7a("armv7a", start_addr, 0, using_gdb, gdb_port));
    arm->clk(clk_fast->signal());
    arm->irq_n(irq_n);
    arm->virq_n(virq_n);
    arm->ahb_master_socket.bind(my_bus->ahb_from_master_socket);

    /* devices for hypervisor */

    print_line();
    printm(d_meteorvb, "Generic Interrupt Controller Distributer (on board)");
    gic_dist.reset(new gic2_dist("gic_dist", 4096, 1, false, 1, 64));
    my_bus->ahb_to_slave_socket.bind(gic_dist->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50000000, 4096);

    print_line();
    printm(d_meteorvb, "Generic Interrupt Controller CPU interface (on board)");
    gic_cpu_if.reset(new gic2_cpu_if("gic_cpu_if", 4096, gic_dist, 1, 64));
    gic_cpu_if->clk(clk_slow->signal());
    gic_cpu_if->irq_n[0](irq_n);
    my_bus->ahb_to_slave_socket.bind(gic_cpu_if->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50001000, 4096);

    print_line();
    printm(d_meteorvb, "Generic Interrupt Virtual Controller");
    gic_vctrl.reset(new gic2_vctrl("gic_vctrl", 4096));
    my_bus->ahb_to_slave_socket.bind(gic_vctrl->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50002000, 4096);

    print_line();
    printm(d_meteorvb, "Generic Interrupt Virtual CPU Interface");
    gic_vcpu_if.reset(new gic2_vcpu_if("gic_vcpu_if", 4096, gic_vctrl, gic_cpu_if));
    gic_vcpu_if->clk(clk_slow->signal());
    gic_vcpu_if->virq_n(virq_n);
    my_bus->ahb_to_slave_socket.bind(gic_vcpu_if->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50003000, 4096);

    printm(d_meteorvb, "Connecting GIC PPI (IGNORED)");
    for(i = 16; i < 32; i++)
    {
        //gic_dist->ppi_n[0][i - 16] (channel[i]);
    }

    printm(d_meteorvb, "Connecting GIC SPI");
    for(i = 32; i < 64; i++)
    {
        gic_dist->spi_n[i - 32](channel[i]);
    }

    print_line();
    printm(d_meteorvb, "UART 0");
    uart_0.reset(new UART("uart_0", 4096, true)); //set stdin to be non-blocking
    uart_0->clk(clk_slow->signal());
    uart_0->rst_n(rst_n);
    uart_0->irq_n(channel[33]);
    my_bus->ahb_to_slave_socket.bind(uart_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50004000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 0");
    timer_0.reset(new TIMER("timer_0", 4096));
    timer_0->clk(clk_timer->signal());
    timer_0->rst_n(rst_n);
    timer_0->irq_n(channel[34]);
    my_bus->ahb_to_slave_socket.bind(timer_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50005000, 4096);

    print_line();
    printm(d_meteorvb, "System registers");
    sys_reg.reset(new SYSCTRL_0("sys_reg", 4096, 0x01780500, 0x0e000000));
    sys_reg->clk(clk_slow->signal());
    sys_reg->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(sys_reg->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50007000, 4096);

    print_line();
    printm(d_meteorvb, "System controller");
    sys_ctrl.reset(new SYSCTRL_1("sys_ctrl", 4096));
    sys_ctrl->clk(clk_slow->signal());
    sys_ctrl->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(sys_ctrl->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x50008000, 4096);

    /* devices for guest OSes */

    print_line();
    printm(d_meteorvb, "UART 4");
    uart_4.reset(new UART("uart_4", 4096, true, "uart_4"));
    uart_4->clk(clk_slow->signal());
    uart_4->rst_n(rst_n);
    uart_4->irq_n(channel[40]);
    my_bus->ahb_to_slave_socket.bind(uart_4->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60000000, 4096);

    print_line();
    printm(d_meteorvb, "UART 5");
    uart_5.reset(new UART("uart_5", 4096, true, "uart_5"));
    uart_5->clk(clk_slow->signal());
    uart_5->rst_n(rst_n);
    uart_5->irq_n(channel[41]);
    my_bus->ahb_to_slave_socket.bind(uart_5->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60001000, 4096);

    print_line();
    printm(d_meteorvb, "UART 6");
    uart_6.reset(new UART("uart_6", 4096, true, "uart_6"));
    uart_6->clk(clk_slow->signal());
    uart_6->rst_n(rst_n);
    uart_6->irq_n(channel[42]);
    my_bus->ahb_to_slave_socket.bind(uart_6->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60002000, 4096);

    print_line();
    printm(d_meteorvb, "UART 7");
    uart_7.reset(new UART("uart_7", 4096, true, "uart_7"));
    uart_7->clk(clk_slow->signal());
    uart_7->rst_n(rst_n);
    uart_7->irq_n(channel[43]);
    my_bus->ahb_to_slave_socket.bind(uart_7->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60003000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 4");
    timer_4.reset(new TIMER("timer_4", 4096));
    timer_4->clk(clk_timer->signal());
    timer_4->rst_n(rst_n);
    timer_4->irq_n(channel[44]);
    my_bus->ahb_to_slave_socket.bind(timer_4->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60004000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 5");
    timer_5.reset(new TIMER("timer_5", 4096));
    timer_5->clk(clk_timer->signal());
    timer_5->rst_n(rst_n);
    timer_5->irq_n(channel[45]);
    my_bus->ahb_to_slave_socket.bind(timer_5->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60005000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 6");
    timer_6.reset(new TIMER("timer_6", 4096));
    timer_6->clk(clk_timer->signal());
    timer_6->rst_n(rst_n);
    timer_6->irq_n(channel[46]);
    my_bus->ahb_to_slave_socket.bind(timer_6->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60006000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 7");
    timer_7.reset(new TIMER("timer_7", 4096));
    timer_7->clk(clk_timer->signal());
    timer_7->rst_n(rst_n);
    timer_7->irq_n(channel[47]);
    my_bus->ahb_to_slave_socket.bind(timer_7->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60007000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 8");
    timer_8.reset(new TIMER("timer_8", 4096));
    timer_8->clk(clk_timer->signal());
    timer_8->rst_n(rst_n);
    timer_8->irq_n(channel[48]);
    my_bus->ahb_to_slave_socket.bind(timer_8->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60008000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 9");
    timer_9.reset(new TIMER("timer_9", 4096));
    timer_9->clk(clk_timer->signal());
    timer_9->rst_n(rst_n);
    timer_9->irq_n(channel[49]);
    my_bus->ahb_to_slave_socket.bind(timer_9->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x60009000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 10");
    timer_10.reset(new TIMER("timer_10", 4096));
    timer_10->clk(clk_timer->signal());
    timer_10->rst_n(rst_n);
    timer_10->irq_n(channel[50]);
    my_bus->ahb_to_slave_socket.bind(timer_10->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x6000A000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 11");
    timer_11.reset(new TIMER("timer_11", 4096));
    timer_11->clk(clk_timer->signal());
    timer_11->rst_n(rst_n);
    timer_11->irq_n(channel[51]);
    my_bus->ahb_to_slave_socket.bind(timer_11->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x6000B000, 4096);

    /* bigger shared devices */

    print_line();
    printm(d_meteorvb, "Virtual Semihosting Hardward");
    semi.reset(new semihost("semihost", 8 << 20));
    my_bus->ahb_to_slave_socket.bind(semi->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x70800000, 8 << 20);

    /* black holes */

    print_line();
    printm(d_realviewpb, "Black Hole 0");
    bh_0.reset(new black_hole("bh_0", 4096));
    my_bus->ahb_to_slave_socket.bind(bh_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x90000000, 4096);

    print_line();
    printm(d_realviewpb, "Black Hole 1");
    bh_1.reset(new black_hole("bh_1", 4096));
    my_bus->ahb_to_slave_socket.bind(bh_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x90001000, 4096);

    print_line();
    printm(d_realviewpb, "Black Hole 2");
    bh_2.reset(new black_hole("bh_2", 4096));
    my_bus->ahb_to_slave_socket.bind(bh_2->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x90002000, 4096);

    print_line();
    printm(d_realviewpb, "Black Hole 3");
    bh_3.reset(new black_hole("bh_3", 4096));
    my_bus->ahb_to_slave_socket.bind(bh_3->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x90003000, 4096);

    print_line();
}

