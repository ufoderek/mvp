#include <board_versatilepb.h>
#include <board_versatilepb_map.h>
#include <board_versatilepb_interrupt.h>
#include <board_versatilepb_addr.h>

/* load the kernel image to RAM */
void board_versatilepb::load_kernel(char* path_kernel)
{
    int fid;

    printm(d_versatile, "loading kernel...");

    if (path_kernel != NULL) {
        printm(d_versatile, "kernel path: %s", path_kernel);
        fid = open(path_kernel, O_RDONLY);

        if (fid > 0) {
            uint32_t addr = ADDR_KERNEL;
            uint32_t buff;
            int count;
            bool result;

            do {
                count = read(fid, &buff, 4);
                result = ram->write(buff, addr, 4);
                //result = ram->local_access(true, addr, buff, 4);
                size_kernel += count;

                if (result == false) {
                    printb(d_versatile, "cannot load kernel image");
                }

                else {
                    addr += 4;
                }

            }
            while (count > 0);
        }

        else {
            printb(d_versatile, "cannot open kernel image");
        }
    }
}

/* load the initrd image or binary data to RAM */
void board_versatilepb::load_initrd(char* path_initrd)
{
    int fid;

    printd(d_versatile, "loading initrd...");

    if (path_initrd != NULL) {
        fid = open(path_initrd, O_RDONLY);

        if (fid > 0) {
            uint32_t addr = ADDR_INITRD;
            uint32_t buff;
            int count;
            bool result;

            do {
                count = read(fid, &buff, 4);
                result = ram->write(buff, addr, 4);
                //result = ram->local_access(false, addr, buff, 4);
                size_initrd += count;

                if (result == false) {
                    printb(d_versatile, "cannot load initrd");
                }

                else {
                    addr += 4;
                }

            }
            while (count > 0);
        }

        else {
            printb(d_versatile, "cannot open initrd");
        }
    }
}

/* load the application program to RAM */
void board_versatilepb::load_prog(char* path_prog)
{
    int fid;

    if (path_prog != NULL) {
        fid = open(path_prog, O_RDONLY);

        if (fid > 0) {
            uint32_t addr = ADDR_PROG;
            uint32_t buff;
            int count;
            bool result;

            do {
                count = read(fid, &buff, 4);
                result = ram->write(buff, addr, 4);
                //result = ram->local_access(true, addr, buff, 4);
                size_prog += count;

                if (result == false) {
                    printb(d_versatile, "cannot load program");
                }

                else {
                    addr += 4;
                }

            }
            while (count > 0);
        }

        else {
            printb(d_versatile, "cannot load program");
        }
    }
}

/* load booting program to RAM */
void board_versatilepb::load_boot(void)
{

    printm(d_versatile, "load boot...");
    /********************************************************************************
     * reference "http://www.simtec.co.uk/products/SWLINUX/files/bootingg_article.html"
     * when Linux booting:
     * r0 = 0x0000
     * r1 = machine ID
     * r2 = ATAG table start address
     *
     * machine ID
     * Versatilepb              --> 0x0183
     * Realvieweb_ARM926EJ-S    --> 0x033b
     * Realvieweb_ARM11MP       --> 0x033b
     * Realviewpb_ARM926EJ-S    --> 0x0183
     * Realviewpb_ARM1176JZF-S  --> 0x05e0
     * Realviewpb_ARM11MP       --> 0x057f
     ********************************************************************************/

    /* specify r0, r1, and r2 by machine code directly */
    uint32_t bootloader[] = {
        0xe3a00000, /* mov     r0, #0 */
        0xe3a01083, /* mov     r1, #0x?? */
        0xe3811c01, /* orr     r1, r1, #0x??00 */
        0xe59f2000, /* ldr     r2, [pc, #0] */
        0xe59ff000, /* ldr     pc, [pc, #0] */
        ADDR_ATAG, /* Address of kernel args.  Set by integratorcp_init.  */
        ADDR_KERNEL  /* Kernel entry point.  Set by integratorcp_init.  */
    };

    for (int i = 0; i < sizeof(bootloader) / sizeof(uint32_t); i++) {
        ram->local_access(true, i * 4, bootloader[i], 4);
    }
}

/* fill the ATAG table for ARM Linux */
void board_versatilepb::load_atag(void)
{
    printd(d_versatile, "load atag...");
    uint32_t addr = ADDR_ATAG;

    /* ATAG_CORE */
    uint32_t data;

    data = 0x0005;
    ram->local_access(true, addr, data, 4);
    data = ATAG_CORE;
    ram->local_access(true, addr += 4, data, 4);
    data = 0x0001;
    ram->local_access(true, addr += 4, data, 4);
    data = 0x1000;
    ram->local_access(true, addr += 4, data, 4);
    data = 0x0000;
    ram->local_access(true, addr += 4, data, 4);

    /* ATAG_MEM */
    data = 0x0004;
    ram->local_access(true, addr += 4, data, 4);
    data = ATAG_MEM;
    ram->local_access(true, addr += 4, data, 4);
    data = size_ram;
    ram->local_access(true, addr += 4, data, 4);
    data = BASE_RAM;
    ram->local_access(true, addr += 4, data, 4);

    /* ATAG_INITRD2 */
    if (size_initrd > 0) {
        data = 0x0004;
        ram->local_access(true, addr += 4, data, 4);
        data = ATAG_INITRD2;
        ram->local_access(true, addr += 4, data, 4);
        data = ADDR_INITRD;
        ram->local_access(true, addr += 4, data, 4);
        data = size_initrd;
        ram->local_access(true, addr += 4, data, 4);

    }

    /* ATAG_CMDLINE */

    /* ATAG_NONE */
    data = 0x0000;
    ram->local_access(true, addr += 4, data, 4);
    data = ATAG_NONE;
    ram->local_access(true, addr += 4, data, 4);
}

/* to terminate the execution */
board_versatilepb::~board_versatilepb(void)
{
    printf("\n");
    printm(d_versatile, "board_versatilepb destructor");
    double diff;
    uint32_t pc;

    /* termination information */
    sc_stop();
    time_end = clock();
    diff = (double)(time_end - time_start) / (double)(CLOCKS_PER_SEC);
    arm->regRead(&pc, PC);
    printm(d_versatile, "system terminated@0x%.8x", pc);
    printm(d_versatile, "total simulation time = %f seconds", diff);

    SDL_Quit();
}

/* to start up all hardware modules on the bus */
board_versatilepb::board_versatilepb(sc_module_name name, bool using_gdb, unsigned int gdb_port): sc_module(name)
{
    int i;
    uint32_t slave_id = 0;
    start_addr = 0;
    size_initrd = 0;
    size_ram = 128 << 20;   // in bytes

    time_start = clock();

    printm(d_versatile, "initialize hardware modules...");

    printm(d_versatile, "ahb bus");
    my_bus.reset(new ahb("my_bus"));

    clk_fast.reset(new sc_clock("clk_fast", 5.0, SC_NS, 2.5, 0, SC_NS, true));      // 200MHz clock
    clk_slow.reset(new sc_clock("clk_slow", 250.0, SC_NS, 125.0, 0, SC_MS, true));  //   4MHz clock

    rst_n.write(true);              // reset signal
    irq_n.write(true);              // IRQ signal to CPU

    /* set all IRQ and FIQ channel to be logic high */
    for (i = 0; i < 64; i++) {
        vic_channel[i].write(true);
    }

    /* initialize the SDL UI system */
    SDL_Init(SDL_INIT_VIDEO);

    /* initialize bus */

    /* initialize the RAM module */
    printm(d_versatile, "RAM, size: %x", size_ram);
    ram.reset(new RAM("RAM", size_ram));
    my_bus->ahb_to_slave_socket.bind(ram->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0, size_ram);

    /* initialize the ARM processor core */
    printm(d_versatile, "ARMv5");
    arm.reset(new ARMV5("ARMv5", start_addr, using_gdb, gdb_port));
    arm->clk(clk_fast->signal());
    arm->irq_n(irq_n);
    arm->ahb_master_socket.bind(my_bus->ahb_from_master_socket);
    //arb->add_master_by_weight(0, 2);

    /* initialize the PL190 VIC module 0(PIC) */
    printm(d_versatile, "VIC0");
    vic_0.reset(new VIC("PL190_VIC_0_PIC", 65536));
    vic_0->clk(clk_fast->signal());
    vic_0->rst_n(rst_n);
    vic_0->irq_n(irq_n);
    my_bus->ahb_to_slave_socket.bind(vic_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_VIC_0, 65536);

    for (int i = 0; i < 32; i++) {
        vic_0->channel[i](vic_channel[i]);
    }

    /* initialize the PL190 VIC module 1(SIC) */
    printm(d_versatile, "VIC1");
    vic_1.reset(new VIC("PL190_VIC_1_SIC", 4096));
    vic_1->clk(clk_fast->signal());
    vic_1->rst_n(rst_n);
    vic_1->irq_n(vic_channel[INT_VIC_0]);
    my_bus->ahb_to_slave_socket.bind(vic_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_VIC_1, 4096);

    for (int i = 0; i < 32; i++) {
        vic_1->channel[i](vic_channel[i + 32]);
    }

    /* initialize the PL080 AMBA DMA module */
    printm(d_versatile, "DMA");
    dma.reset(new DMA("PL080_DMA", 65536));
    dma->clk(clk_slow->signal());
    dma->rst_n(rst_n);
    dma->irq_n(vic_channel[INT_DMA]);
    my_bus->ahb_to_slave_socket.bind(dma->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_DMA, 65536);

    /* initialize the PL011 UART module 0 */
    printm(d_versatile, "UART0");
    uart_0.reset(new UART("PL011_UART_0", 4096, true)); //set stdin to be non-blocking
    uart_0->clk(clk_slow->signal());
    uart_0->rst_n(rst_n);
    uart_0->irq_n(vic_channel[INT_UART_0]);
    my_bus->ahb_to_slave_socket.bind(uart_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_0, 4096);

    /* initialize the PL011 UART module 1 */
    printm(d_versatile, "UART1");
    uart_1.reset(new UART("PL011_UART_1", 4096, false));
    uart_1->clk(clk_slow->signal());
    uart_1->rst_n(rst_n);
    uart_1->irq_n(vic_channel[INT_UART_1]);
    my_bus->ahb_to_slave_socket.bind(uart_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_1, 4096);

    /* initialize the PL011 UART module 2 */
    printm(d_versatile, "UART2");
    uart_2.reset(new UART("PL011_UART_2", 4096, false));
    uart_2->clk(clk_slow->signal());
    uart_2->rst_n(rst_n);
    uart_2->irq_n(vic_channel[INT_UART_2]);
    my_bus->ahb_to_slave_socket.bind(uart_2->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_2, 4096);

    /* initialize the PL011 UART module 3 */
    printm(d_versatile, "UART3");
    uart_3.reset(new UART("PL011_UART_3", 4096, false));
    uart_3->clk(clk_slow->signal());
    uart_3->rst_n(rst_n);
    uart_3->irq_n(vic_channel[INT_UART_3]);
    my_bus->ahb_to_slave_socket.bind(uart_3->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_3, 4096);

    /* initialize the SP804 Timer module 0 */
    printm(d_versatile, "TIMER0");
    timer_0.reset(new TIMER("SP804_TIMER_0", 4096));
    timer_0->clk(clk_slow->signal());
    timer_0->rst_n(rst_n);
    timer_0->irq_n(vic_channel[INT_TIMER_0]);
    my_bus->ahb_to_slave_socket.bind(timer_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_TIMER_0, 4096);

    /* initialize the SP804 Timer module 1 */
    printm(d_versatile, "TIMER1");
    timer_1.reset(new TIMER("SP804_TIMER_1", 4096));
    timer_1->clk(clk_slow->signal());
    timer_1->rst_n(rst_n);
    timer_1->irq_n(vic_channel[INT_TIMER_1]);
    my_bus->ahb_to_slave_socket.bind(timer_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_TIMER_1, 4096);

    /* initialize the SP805 watchdog */
    printm(d_versatile, "WATCHDOG");
    watchdog.reset(new WATCHDOG("SP805_WATCHDOG", 4096));
    watchdog->clk(clk_slow->signal());
    watchdog->rst_n(rst_n);
    watchdog->irq_n(vic_channel[INT_WATCHDOG]);
    my_bus->ahb_to_slave_socket.bind(watchdog->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_WATCHDOG, 4096);

    /* initialize the PL031 real time clock */
    printm(d_versatile, "RTC");
    rtc.reset(new RTC("PL031_RTC", 4096));
    rtc->clk(clk_slow->signal());
    rtc->rst_n(rst_n);
    rtc->irq_n(vic_channel[INT_RTC]);
    my_bus->ahb_to_slave_socket.bind(rtc->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_RTC, 4096);

    /* initialize the PL110 color LCD controller */
    printm(d_versatile, "CLCD");
    clcd.reset(new CLCD("PL110_CLCD", 65536, false)); // don't use lcd
    clcd->clk(clk_slow->signal());
    clcd->rst_n(rst_n);
    clcd->irq_n(vic_channel[INT_CLCD]);
    //clcd->using_lcd();
    my_bus->ahb_to_slave_socket.bind(clcd->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_CLCD, 65536);

    /* initialize the PL05 PS2 keyboard & mouse interface of keyboard */
    printm(d_versatile, "PS2 KEYBOARD");
    keyboard.reset(new PS2("PL050_KEYBOARD", 4096, PS2Keyboard));
    keyboard->clk(clk_slow->signal());
    keyboard->rst_n(rst_n);
    keyboard->irq_n(vic_channel[INT_KEYBOARD]);
    my_bus->ahb_to_slave_socket.bind(keyboard->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_KEYBOARD, 4096);

    /* initialize the PL05 PS2 keyboard & mouse interface of mouse */
    printm(d_versatile, "PS2 MOUSE");
    mouse.reset(new PS2("PL050_MOUSE", 4096, PS2Mouse));
    mouse->clk(clk_slow->signal());
    mouse->rst_n(rst_n);
    mouse->irq_n(vic_channel[INT_MOUSE]);
    my_bus->ahb_to_slave_socket.bind(mouse->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_MOUSE, 4096);

    /* initialize the PL061 GPIO controller 0 */
    printm(d_versatile, "GPIO");
    gpio_0.reset(new GPIO("PL061_GPIO_0", 4096));
    gpio_0->clk(clk_slow->signal());
    gpio_0->rst_n(rst_n);
    gpio_0->irq_n(vic_channel[INT_GPIO_0]);
    my_bus->ahb_to_slave_socket.bind(gpio_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GPIO_0, 4096);

    /* initialize the PL061 GPIO controller 1 */
    printm(d_versatile, "GPIO1");
    gpio_1.reset(new GPIO("PL061_GPIO_1", 4096));
    gpio_1->clk(clk_slow->signal());
    gpio_1->rst_n(rst_n);
    gpio_1->irq_n(vic_channel[INT_GPIO_1]);
    my_bus->ahb_to_slave_socket.bind(gpio_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GPIO_1, 4096);

    /* initialize the PL061 GPIO controller 2 */
    printm(d_versatile, "GPIO2");
    gpio_2.reset(new GPIO("PL061_GPIO_2", 4096));
    gpio_2->clk(clk_slow->signal());
    gpio_2->rst_n(rst_n);
    gpio_2->irq_n(vic_channel[INT_GPIO_2]);
    my_bus->ahb_to_slave_socket.bind(gpio_2->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GPIO_2, 4096);

    /* initialize the PL061 GPIO controller 3 */
    printm(d_versatile, "GPIO3");
    gpio_3.reset(new GPIO("PL061_GPIO_3", 4096));
    gpio_3->clk(clk_slow->signal());
    gpio_3->rst_n(rst_n);
    gpio_3->irq_n(vic_channel[INT_GPIO_3]);
    my_bus->ahb_to_slave_socket.bind(gpio_3->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GPIO_3, 4096);

    /* initialize the PL022 synchronous serial port */
    printm(d_versatile, "SSP");
    ssp.reset(new SSP("PL022_SSP", 4096));
    ssp->clk(clk_slow->signal());
    ssp->rst_n(rst_n);
    ssp->irq_n(vic_channel[INT_SSP]);
    my_bus->ahb_to_slave_socket.bind(ssp->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SSP, 4096);

    /* initialize the PL131 smart card interface */
    printm(d_versatile, "SCI");
    sci.reset(new SCI("PL131_SCI", 4096));
    sci->clk(clk_slow->signal());
    sci->rst_n(rst_n);
    sci->irq_n(vic_channel[INT_SCI]);
    my_bus->ahb_to_slave_socket.bind(sci->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SCI, 4096);

    /* initialize the PL041 advanced audio codec interface */
    printm(d_versatile, "AACI");
    aaci.reset(new AACI("PL041_AACI", 4096));
    aaci->clk(clk_slow->signal());
    aaci->rst_n(rst_n);
    aaci->irq_n(vic_channel[INT_AACI]);
    my_bus->ahb_to_slave_socket.bind(aaci->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_AACI, 4096);

    /* initialize the PL180 multi-media card interface */
    printm(d_versatile, "MCI");
    mci.reset(new MCI("MCI", 4096));
    mci->clk(clk_slow->signal());
    mci->rst_n(rst_n);
    mci->irqA_n(vic_channel[INT_MCI_A]);
    mci->irqB_n(vic_channel[INT_MCI_B]);
    my_bus->ahb_to_slave_socket.bind(mci->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_MCI, 4096);

    /* initialize the SP810 status & system control module */
    printm(d_versatile, "SYSCTRL0");
    status_0.reset(new SYSCTRL_0("SP810_SYS_CTRL_0", 4096, 0x41007004, 0x02000000));
    status_0->clk(clk_slow->signal());
    status_0->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(status_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SYS_CTRL_0, 4096);

    /* initialize the SP810 status & system control module */
    printm(d_versatile, "SYSCTRL1");
    status_1.reset(new SYSCTRL_1("SP810_SYS_CTRL_1", 4096));
    status_1->clk(clk_slow->signal());
    status_1->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(status_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SYS_CTRL_1, 4096);

    /* initialize the PL093 synchronous static memory controller */
    printm(d_versatile, "SSMC");
    ssmc.reset(new SSMC("PL093_SSMC", 65536));
    ssmc->clk(clk_slow->signal());
    ssmc->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(ssmc->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SSMC, 65536);

    /* initialize the GX175 multi port memory controller */
    printm(d_versatile, "MPMC");
    mpmc.reset(new MPMC("GX175_MPMC", 65536));
    mpmc->clk(clk_slow->signal());
    mpmc->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(mpmc->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_MPMC, 65536);

    /* initialize the virtual calculator */
    //  printf("%s => 0x%.8x ~ 0x%.8x\n", "Virtual Calculator", BASE_CALCULATOR, (uint32_t)(END_CALCULATOR));
    //  calculator = new CALCULATOR("calculator");
    //  calculator->clk(clk_slow->signal());
    //  calculator->rst_n(rst_n);
    //  calculator->irq_n(vic_channel[INT_CALCULATOR]);

    /* initialize the file transfer module */
    //  printf("%s => 0x%.8x ~ 0x%.8x\n", "FTP", BASE_FTP, (uint32_t)(END_FTP));
    //  ftp = new FTP();
}

