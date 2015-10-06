#include <fcntl.h>

#include <board_realviewpb.h>
#include <board_realviewpb_addr.h>

/* load the kernel image to RAM */
void board_realviewpb::load_kernel(char* path_kernel)
{
    printm(d_realviewpb, "loading kernel...");

    if(path_kernel != NULL)
    {
        int fid = open(path_kernel, O_RDONLY);

        if(fid > 0)
        {
            uint32_t addr = ADDR_KERNEL;
            uint32_t buff;
            int count;
            bool result;

            do
            {
                count = read(fid, &buff, 4);
                result = ram->write(buff, addr, 4);
                size_kernel += count;

                if(result == false)
                {
                    printb(d_realviewpb, "cannot load kernel image");
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
            printb(d_realviewpb, "cannot open kernel image");
        }
    }
}

/* load the initrd image or binary data to RAM */
void board_realviewpb::load_initrd(char* path_initrd)
{
    printd(d_realviewpb, "loading initrd...");

    if(path_initrd != NULL)
    {
        int fid = open(path_initrd, O_RDONLY);

        if(fid > 0)
        {
            uint32_t addr = ADDR_INITRD;
            uint32_t buff;
            int count;
            bool result;

            do
            {
                count = read(fid, &buff, 4);
                result = ram->write(buff, addr, 4);
                size_initrd += count;

                if(result == false)
                {
                    printb(d_realviewpb, "cannot load initrd");
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
            printb(d_realviewpb, "cannot open initrd");
        }
    }
}

/* load the application program to RAM */
void board_realviewpb::load_prog(char* path_prog)
{
    if(path_prog != NULL)
    {
        int fid = open(path_prog, O_RDONLY);

        if(fid > 0)
        {
            uint32_t addr = ADDR_PROG;
            uint32_t buff;
            int count;
            bool result;

            do
            {
                count = read(fid, &buff, 4);
                result = ram->write(buff, addr, 4);
                size_prog += count;

                if(result == false)
                {
                    printb(d_realviewpb, "cannot load program");
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
            printb(d_realviewpb, "cannot load program");
        }
    }
}

/* load booting program to RAM */
void board_realviewpb::load_boot()
{
    printm(d_realviewpb, "load boot...");
    /* Reference on QEMU source code */
    //R0 = 0
    //R1 = machine code, linux/arch/arm/tools/mach-types
    //R2 = phy address of parameter list
    /* specify r0, r1, and r2 by machine code directly */
    uint32_t bootloader[] =
    {
        0xe3a00000, /* mov     r0, #0 */
        0xe3a01069, /* mov     r1, #0x?? */
        0xe3811c07, /* orr     r1, r1, #0x??00 */
        0xe59f2000, /* ldr     r2, [pc, #0] */
        0xe59ff000, /* ldr     pc, [pc, #0] */
        ADDR_ATAG + 0x70000000, /* Address of kernel args.  Set by integratorcp_init.  */
        ADDR_KERNEL + 0x70000000 /* Kernel entry point.  Set by integratorcp_init.  */
    };

    for(int i = 0; i < sizeof(bootloader) / sizeof(uint32_t); i++)
    {
        ram->local_access(true, i * 4, bootloader[i], 4);
    }
}

void board_realviewpb::load_smp_boot()
{
    /* Reference on QEMU source code */
    uint32_t smpboot[] =
    {
        0xe59f0020, /* ldr     r0, privbase */ /* r0 = r15(pc) + 0x20, pc = current_pc + 8 */
        0xe3a01001, /* mov     r1, #1 */
        0xe5801100, /* str     r1, [r0, #0x100] */
        0xe3a00201, /* mov     r0, #0x10000000 */
        0xe3800030, /* orr     r0, #0x30 */
        0xe320f003, /* wfi */
        0xe5901000, /* ldr     r1, [r0] */
        0xe1110001, /* tst     r1, r1 */
        0x0afffffb, /* beq     <wfi> */
        0xe12fff11, /* bx      r1 */
        0x10100000 /* privbase: Private memory region base address, current_pc + 8 + 0x20  */
    };

    for(int i = 0; i < sizeof(smpboot) / sizeof(uint32_t); i++)
    {
        smp_boot_rom->local_access(true, i * 4, smpboot[i], 4);
    }
}

/* fill the ATAG table for ARM Linux */
void board_realviewpb::load_atag()
{
    //Ref to qemu-0.15.0/hw/arm_boot.c
    uint32_t atag[] =
    {
        0x5,        // ATAG_CORE size
        0x54410001, // ATAG_CORE
        0x1,
        0x1000,
        0x0,

        0x4,        // ATAG_MEM size
        0x54410002, // ATAG_MEM
        size_ram,   // ram size
        0x70000000, // entry point (also means memory base address here)

        0x4,        // ATAG_INITRD2 size
        0x54420005, // ATAG_INITRD2
        0x70d00000, // initrd addr
        size_initrd,// initrd size

        0x0000,     // ATAG_NONE size
        0x0000      // ATAG_NONE
    };

    for(int i = 0; i < sizeof(atag) / sizeof(uint32_t); i++)
    {
        ram->local_access(true, ADDR_ATAG + i * 4, atag[i], 4);
    }
}

/* to terminate the execution */
board_realviewpb::~board_realviewpb()
{
}

#define print_line() printm(d_realviewpb,"===============================================");

board_realviewpb::board_realviewpb(sc_module_name name, bool using_gdb, unsigned int gdb_port): sc_module(name)
{
    int i;
    uint32_t slave_id = 0;
    start_addr = 0;
    size_initrd = 0;
    size_ram = 128 << 20; //in bytes

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

    /*
    clk_fast.reset(new sc_clock("clk_fast", 0.5, SC_NS, 0.5)); //200MHz clock
    clk_slow.reset(new sc_clock("clk_slow", 25.0, SC_NS, 0.5)); //40MHz clock
    */

    rst_n.write(true); //reset signal
    irq_n.write(true); //IRQ signal to CPU
    virq_n.write(true); //virtual IRQ signal to CPU

    for(i = 0; i < 16 + 16 + 32; i++)   //sgi + ppi + spi = 16 + 16 + 32
    {
        channel[i].write(true);
    }

    always_1 = true;

    print_line();
    printm(d_realviewpb, "AHB Bus");
    my_bus.reset(new ahb("ahb_bus"));

    print_line();
    printm(d_realviewpb, "Fake DRAM, size: %x", size_ram);
    ram.reset(new RAM("RAM", size_ram));
    my_bus->ahb_to_slave_socket.bind(ram->ahb_slave_socket);
    my_bus->add_mapping(slave_id, 0, size_ram);
    my_bus->add_mapping(slave_id++, 0x70000000, size_ram); //high mem

    print_line();
    printm(d_realviewpb, "ARMv7-A");
    arm.reset(new armv7a("armv7a", start_addr, 0, using_gdb, gdb_port));
    //arm.reset(new armv7a_tlb("armv7a", start_addr, 0, using_gdb, gdb_port));
    arm->clk(clk_fast->signal());
    arm->irq_n(irq_n);
    arm->virq_n(virq_n);
    arm->ahb_master_socket.bind(my_bus->ahb_from_master_socket);

    print_line();
    printm(d_realviewpb, "System registers");
    //qemu-0.15.1/hw/arm_sysctl.c
    //pb-a8 = 0x01780500
    //pbx = 0x01820500
    //vexpress = 0x01900500
    status_0.reset(new SYSCTRL_0("sys_reg", 4096, 0x01780500, 0x0e000000));
    status_0->clk(clk_slow->signal());
    status_0->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(status_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x10000000, 4096);

    print_line();
    printm(d_realviewpb, "System controller 0");
    status_1.reset(new SYSCTRL_1("sys_ctrl_0", 4096));
    status_1->clk(clk_slow->signal());
    status_1->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(status_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x10001000, 4096);

    print_line();
    printm(d_realviewpb, "UART 0");
    uart_0.reset(new UART("uart_0", 4096, true)); //set stdin to be non-blocking
    uart_0->clk(clk_slow->signal());
    uart_0->rst_n(rst_n);
    uart_0->irq_n(channel[44]);
    my_bus->ahb_to_slave_socket.bind(uart_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x10009000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 0");
    timer_0.reset(new TIMER("timer_0", 4096));
    timer_0->clk(clk_timer->signal());
    timer_0->rst_n(rst_n);
    timer_0->irq_n(channel[36]);
    my_bus->ahb_to_slave_socket.bind(timer_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x10011000, 4096);

    print_line();
    printm(d_realviewpb, "Timer 2");
    timer_2.reset(new TIMER("timer_2", 4096));
    timer_2->clk(clk_timer->signal());
    timer_2->rst_n(rst_n);
    timer_2->irq_n(channel[37]);
    my_bus->ahb_to_slave_socket.bind(timer_2->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x10012000, 4096);

    print_line();
    printm(d_realviewpb, "Generic Interrupt Controller Distributer (on board)");
    gic_dist.reset(new gic2_dist("gic_dist", 4096, 1, false, 1, 64));
    my_bus->ahb_to_slave_socket.bind(gic_dist->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x1E001000, 4096);

    print_line();
    printm(d_realviewpb, "Generic Interrupt Controller CPU interface (on board)");
    gic_cpu_if.reset(new gic2_cpu_if("gic_cpu_if", 4096, gic_dist, 1, 64));
    gic_cpu_if->clk(clk_slow->signal());
    gic_cpu_if->irq_n[0](irq_n);
    my_bus->ahb_to_slave_socket.bind(gic_cpu_if->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x1E000000, 4096);

    printm(d_realviewpb, "Connecting GIC PPI (IGNORED)");

    for(i = 16; i < 32; i++)
    {
        //gic_dist->ppi_n[0][i - 16] (channel[i]);
    }

    printm(d_realviewpb, "Connecting GIC SPI");

    for(i = 32; i < 64; i++)
    {
        gic_dist->spi_n[i - 32](channel[i]);
    }

    print_line();
    printm(d_realviewpb, "Black Hole");
    bh.reset(new black_hole("bh", 4096));
    my_bus->ahb_to_slave_socket.bind(bh->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0x20000000, 4096);

    print_line();
}

