#include <board_realvieweb.h>
#include <board_realvieweb_map.h>
#include <board_realvieweb_interrupt.h>
#include <board_realvieweb_addr.h>

/* load the kernel image to RAM */
void board_realvieweb::load_kernel(char* path_kernel)
{
    int fid;

    printm(d_realvieweb, "loading kernel...");

    if (path_kernel != NULL) {

        printm(d_realvieweb, "kernel path: %s", path_kernel);
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
                    printb(d_realvieweb, "cannot load kernel image");
                }

                else {
                    addr += 4;
                }

            }
            while (count > 0);
        }

        else {
            printb(d_realvieweb, "cannot open kernel image");
        }
    }
}

/* load the initrd image or binary data to RAM */
void board_realvieweb::load_initrd(char* path_initrd)
{
    int fid;

    printd(d_realvieweb, "loading initrd...");

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
                    printb(d_realvieweb, "cannot load initrd");
                }

                else {
                    addr += 4;
                }

            }
            while (count > 0);
        }

        else {
            printb(d_realvieweb, "cannot open initrd");
        }
    }
}

/* load the application program to RAM */
void board_realvieweb::load_prog(char* path_prog)
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
                    printb(d_realvieweb, "cannot load program");
                }

                else {
                    addr += 4;
                }

            }
            while (count > 0);
        }

        else {
            printb(d_realvieweb, "cannot load program");
        }
    }
}

/* load booting program to RAM */
void board_realvieweb::load_boot()
{

    printm(d_realvieweb, "load boot...");
    /********************************************************************************
     * reference "http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html"
     * when Linux booting:
     * r0 = 0x0000
     * r1 = machine ID
     * r2 = ATAG table start address
     *
     * machine ID
     * Versatilepb              --> 0x0183
     *
     * Realvieweb_armv5EJ-S --> 0x033b
     * Realvieweb_ARM1136       --> 0x033b
     * Realvieweb_ARM11MP       --> 0x033b
     *
     * Realviewpb_armv5EJ-S --> 0x0183
     * Realviewpb_ARM1176JZF-S  --> 0x05e0
     * Realviewpb_ARM11MP       --> 0x057f
     ********************************************************************************/

    /* specify r0, r1, and r2 by machine code directly */
    uint32_t bootloader[] = {
        0xe3a00000, /* mov     r0, #0 */
        0xe3a0103b, /* mov     r1, #0x?? */
        0xe3811c03, /* orr     r1, r1, #0x??00 */
        0xe59f2000, /* ldr     r2, [pc, #0] */
        0xe59ff000, /* ldr     pc, [pc, #0] */
        ADDR_ATAG, /* Address of kernel args.  Set by integratorcp_init.  */
        ADDR_KERNEL  /* Kernel entry point.  Set by integratorcp_init.  */
    };

    for (int i = 0; i < sizeof(bootloader) / sizeof(uint32_t); i++) {
        ram->local_access(true, i * 4, bootloader[i], 4);
    }
}

void board_realvieweb::load_smp_boot()
{
    uint32_t smpboot[] = {
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

    for (int i = 0; i < sizeof(smpboot) / sizeof(uint32_t); i++) {
        smp_boot_rom->local_access(true, i * 4, smpboot[i], 4);
    }
}

/* fill the ATAG table for ARM Linux */
void board_realvieweb::load_atag(void)
{
    printd(d_realvieweb, "load atag...");
    uint32_t addr = ADDR_ATAG;

    /* ATAG */
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
board_realvieweb::~board_realvieweb(void)
{
    printf("\n");
    printm(d_realvieweb, "board_realvieweb destructor");
    double diff;
    uint32_t pc;

    /* termination information */
    sc_stop();
    time_end = clock();
    diff = (double)(time_end - time_start) / (double)(CLOCKS_PER_SEC);
    /*
    arm->regRead(&pc, PC);
    printm(d_realvieweb, "system terminated@0x%.8x", pc);
    */
    printm(d_realvieweb, "total simulation time = %f seconds", diff);

    //SDL_Quit();
}

/* to start up all hardware modules on the bus */
board_realvieweb::board_realvieweb(sc_module_name name, bool using_gdb, unsigned int gdb_port): sc_module(name)
{
    int i;
    uint32_t slave_id = 0;

    start_addr = 0;
    smp_start_addr = BASE_SMP_BOOT_ROM;
    size_initrd = 0;
    size_ram = 128 << 20;   // in bytes
    size_smp_boot_rom = 4 << 10;

    time_start = clock();

    clk_fast.reset(new sc_clock("clk_fast", 5.0, SC_NS, 2.5, 0, SC_NS, true));      // 200MHz clock
    clk_slow.reset(new sc_clock("clk_slow", 250.0, SC_NS, 125.0, 0, SC_MS, true));  //   4MHz clock

    rst_n.write(true);              // reset signal
    irq_n.write(true);              // IRQ signal to CPU
    virq_n.write(true);              // IRQ signal to CPU

    /* set all IRQ and FIQ channel to be logic high */
    for (i = 0; i < 16 + 16 + 32; i++) { // sgi + ppi + spi = 16 + 16 + 32
        channel[i].write(true);
    }

#ifdef REALVIEWEB_ARMV6K

    /* set all MP GIC's IRQ channel to logic high */
    /* only 32 ~ 47 is used */
    for (i = 0; i < 64; i++) {
        mp_channel[i].write(true);
    }

#endif

    /* initialize bus */
    printm(d_realvieweb, "ahb_bus");
    my_bus.reset(new ahb("ahb_bus"));

    /* initialize the RAM module */
    printm(d_realvieweb, "RAM, size: %x", size_ram);
    ram.reset(new RAM("RAM", size_ram));
    my_bus->ahb_to_slave_socket.bind(ram->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, 0, size_ram);

    /* initialize the ARM processor core */
    printm(d_realvieweb, "arm");

#ifdef REALVIEWEB_ARMV5
    arm.reset(new ARMV5("armv5", start_addr, using_gdb, gdb_port));
#endif

#ifdef REALVIEWEB_ARMV6
    arm.reset(new ARMV6("armv6", start_addr, using_gdb, gdb_port));
#endif

#ifdef REALVIEWEB_ARMV6K
    arm.reset(new ARM11_MPCORE("arm11_mpcore", start_addr, smp_start_addr, using_gdb, gdb_port, 4));
#endif

    /* clock connection */
    arm->clk(clk_fast->signal());
#ifdef REALVIEWEB_ARMV6K
    arm->clk_slow(clk_slow->signal());
#endif

    /* interrupts connection */
#ifdef REALVIEWEB_ARMV6K

    for (i = 0; i < 64; i++) {
        arm->gic_irq_n[i](mp_channel[i]);
    }

#else
    arm->irq_n(irq_n);
#endif

#ifdef REALVIEWEB_ARMV6K
    /* bind scu's bus master interface */
    arm->scu->ahb_master_socket.bind(my_bus->ahb_from_master_socket);
    /* shut up gcc */
    arm->core0->ahb_master_socket.bind(my_bus->ahb_from_master_socket);
    arm->core1->ahb_master_socket.bind(my_bus->ahb_from_master_socket);
    arm->core2->ahb_master_socket.bind(my_bus->ahb_from_master_socket);
    arm->core3->ahb_master_socket.bind(my_bus->ahb_from_master_socket);
    my_bus->ahb_to_slave_socket.bind(arm->gic_dist->ahb_slave_socket);
    slave_id++;
    my_bus->ahb_to_slave_socket.bind(arm->gic_cpu_if->ahb_slave_socket);
    slave_id++;
#else
    /* bind arm's bus master interface */
    arm->ahb_master_socket.bind(my_bus->ahb_from_master_socket);
#endif

    printm(d_realvieweb, "GIC2 Dist");
    gic_dist.reset(new gic2_dist("gic2_dist", 4096, 1, false, 1, 64));
    my_bus->ahb_to_slave_socket.bind(gic_dist->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GIC_0 + 0x1000, 4096);

    printm(d_realvieweb, "GIC2 CPU Interface");
    gic_cpu_if.reset(new gic2_cpu_if("gic2_cpu_if", 4096, gic_dist, 1, 64));
    gic_cpu_if->clk(clk_slow->signal());
#ifdef REALVIEWEB_ARMV6K
    gic_cpu_if->irq_n[0](mp_channel[MP_INT_GIC_1]);
    gic_cpu_if->virq_n[0](virq_n);
#else
    gic_cpu_if->irq_n[0](irq_n);
    gic_cpu_if->virq_n[0](virq_n);
#endif
    my_bus->ahb_to_slave_socket.bind(gic_cpu_if->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GIC_0, 4096);

    printm(d_realvieweb, "GIC2 Dist SPI bounding");

    for (i = 32; i < 64; i++) {
        gic_dist->spi_n[i - 32](channel[i]);
    }

    /* initialize the PL080 AMBA DMA module */
    printm(d_realvieweb, "DMA");
    dma.reset(new DMA("PL080_DMA", 65536));
    dma->clk(clk_slow->signal());
    dma->rst_n(rst_n);
    dma->irq_n(channel[INT_DMA]);
    my_bus->ahb_to_slave_socket.bind(dma->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_DMA, 65536);

    /* initialize the PL011 UART module 0 */
    printm(d_realvieweb, "UART0");
    uart_0.reset(new UART("PL011_UART_0", 4096, true)); //set stdin to be non-blocking
    uart_0->clk(clk_slow->signal());
    uart_0->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    uart_0->irq_n(mp_channel[MP_INT_UART_0]);
#else
    uart_0->irq_n(channel[INT_UART_0]);
#endif
    my_bus->ahb_to_slave_socket.bind(uart_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_0, 4096);

    /* initialize the PL011 UART module 1 */
    printm(d_realvieweb, "UART1");
    uart_1.reset(new UART("PL011_UART_1", 4096, false));
    uart_1->clk(clk_slow->signal());
    uart_1->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    uart_1->irq_n(mp_channel[MP_INT_UART_1]);
#else
    uart_1->irq_n(channel[INT_UART_1]);
#endif
    my_bus->ahb_to_slave_socket.bind(uart_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_1, 4096);

    /* initialize the PL011 UART module 2 */
    printm(d_realvieweb, "UART2");
    uart_2.reset(new UART("PL011_UART_2", 4096, false));
    uart_2->clk(clk_slow->signal());
    uart_2->rst_n(rst_n);
    uart_2->irq_n(channel[INT_UART_2]);
    my_bus->ahb_to_slave_socket.bind(uart_2->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_2, 4096);

    /* initialize the PL011 UART module 3 */
    printm(d_realvieweb, "UART3");
    uart_3.reset(new UART("PL011_UART_3", 4096, false));
    uart_3->clk(clk_slow->signal());
    uart_3->rst_n(rst_n);
    uart_3->irq_n(channel[INT_UART_3]);
    my_bus->ahb_to_slave_socket.bind(uart_3->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_UART_3, 4096);

    /* initialize the SP804 Timer module 0 */
    printm(d_realvieweb, "TIMER0");
    timer_0.reset(new TIMER("SP804_TIMER_0", 4096));
    timer_0->clk(clk_slow->signal());
    timer_0->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    timer_0->irq_n(mp_channel[MP_INT_TIMER_0]);
#else
    timer_0->irq_n(channel[INT_TIMER_0]);
#endif
    my_bus->ahb_to_slave_socket.bind(timer_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_TIMER_0, 4096);

    /* initialize the SP804 Timer module 1 */
    printm(d_realvieweb, "TIMER1");
    timer_1.reset(new TIMER("SP804_TIMER_1", 4096));
    timer_1->clk(clk_slow->signal());
    timer_1->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    timer_1->irq_n(mp_channel[MP_INT_TIMER_1]);
#else
    timer_1->irq_n(channel[INT_TIMER_1]);
#endif
    my_bus->ahb_to_slave_socket.bind(timer_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_TIMER_1, 4096);

    /* initialize the SP805 watchdog */
    printm(d_realvieweb, "WATCHDOG");
    watchdog.reset(new WATCHDOG("SP805_WATCHDOG", 4096));
    watchdog->clk(clk_slow->signal());
    watchdog->rst_n(rst_n);
    watchdog->irq_n(channel[INT_WATCHDOG]);
    my_bus->ahb_to_slave_socket.bind(watchdog->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_WATCHDOG, 4096);

    /* initialize the PL031 real time clock */
    printm(d_realvieweb, "RTC");
    rtc.reset(new RTC("PL031_RTC", 4096));
    rtc->clk(clk_slow->signal());
    rtc->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    rtc->irq_n(mp_channel[MP_INT_RTC]);
#else
    rtc->irq_n(channel[INT_RTC]);
#endif
    my_bus->ahb_to_slave_socket.bind(rtc->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_RTC, 4096);

    /* initialize the PL110 color LCD controller */
    printm(d_realvieweb, "CLCD");
    clcd.reset(new CLCD("PL110_CLCD", 65536, false)); // don't use lcd
    clcd->clk(clk_slow->signal());
    clcd->rst_n(rst_n);
    clcd->irq_n(channel[INT_CLCD]);
    //clcd->using_lcd();
    my_bus->ahb_to_slave_socket.bind(clcd->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_CLCD, 65536);

    /* initialize the PL05 PS2 keyboard & mouse interface of keyboard */
    printm(d_realvieweb, "PS2 KEYBOARD");
    keyboard.reset(new PS2("PL050_KEYBOARD", 4096, PS2Keyboard));
    keyboard->clk(clk_slow->signal());
    keyboard->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    keyboard->irq_n(mp_channel[MP_INT_KMI_0]);
#else
    keyboard->irq_n(channel[INT_KEYBOARD]);
#endif
    my_bus->ahb_to_slave_socket.bind(keyboard->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_KEYBOARD, 4096);

    /* initialize the PL05 PS2 keyboard & mouse interface of mouse */
    printm(d_realvieweb, "PS2 MOUSE");
    mouse.reset(new PS2("PL050_MOUSE", 4096, PS2Mouse));
    mouse->clk(clk_slow->signal());
    mouse->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    mouse->irq_n(mp_channel[MP_INT_KMI_1]);
#else
    mouse->irq_n(channel[INT_MOUSE]);
#endif
    my_bus->ahb_to_slave_socket.bind(mouse->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_MOUSE, 4096);

    /* initialize the PL061 GPIO controller 0 */
    printm(d_realvieweb, "GPIO");
    gpio_0.reset(new GPIO("PL061_GPIO_0", 4096));
    gpio_0->clk(clk_slow->signal());
    gpio_0->rst_n(rst_n);
    gpio_0->irq_n(channel[INT_GPIO_0]);
    my_bus->ahb_to_slave_socket.bind(gpio_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GPIO_0, 4096);

    /* initialize the PL061 GPIO controller 1 */
    printm(d_realvieweb, "GPIO1");
    gpio_1.reset(new GPIO("PL061_GPIO_1", 4096));
    gpio_1->clk(clk_slow->signal());
    gpio_1->rst_n(rst_n);
    gpio_1->irq_n(channel[INT_GPIO_1]);
    my_bus->ahb_to_slave_socket.bind(gpio_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GPIO_1, 4096);

    /* initialize the PL061 GPIO controller 2 */
    printm(d_realvieweb, "GPIO2");
    gpio_2.reset(new GPIO("PL061_GPIO_2", 4096));
    gpio_2->clk(clk_slow->signal());
    gpio_2->rst_n(rst_n);
    gpio_2->irq_n(channel[INT_GPIO_2]);
    my_bus->ahb_to_slave_socket.bind(gpio_2->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_GPIO_2, 4096);

    /* initialize the PL022 synchronous serial port */
    printm(d_realvieweb, "SSP");
    ssp.reset(new SSP("PL022_SSP", 4096));
    ssp->clk(clk_slow->signal());
    ssp->rst_n(rst_n);
    ssp->irq_n(channel[INT_SSP]);
    my_bus->ahb_to_slave_socket.bind(ssp->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SSP, 4096);

    /* initialize the PL131 smart card interface */
    printm(d_realvieweb, "SCI");
    sci.reset(new SCI("PL131_SCI", 4096));
    sci->clk(clk_slow->signal());
    sci->rst_n(rst_n);
    sci->irq_n(channel[INT_SCI]);
    my_bus->ahb_to_slave_socket.bind(sci->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SCI, 4096);

    /* initialize the PL041 advanced audio codec interface */
    printm(d_realvieweb, "AACI");
    aaci.reset(new AACI("PL041_AACI", 4096));
    aaci->clk(clk_slow->signal());
    aaci->rst_n(rst_n);
#ifdef REALVIEWEB_ARMV6K
    aaci->irq_n(mp_channel[MP_INT_AACI]);
#else
    aaci->irq_n(channel[INT_AACI]);
#endif
    my_bus->ahb_to_slave_socket.bind(aaci->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_AACI, 4096);

    /* initialize the PL180 multi-media card interface */
    printm(d_realvieweb, "MCI");
    mci.reset(new MCI("MCI", 4096));
    mci->clk(clk_slow->signal());
    mci->rst_n(rst_n);
    mci->irqA_n(channel[INT_MCI_0]);
    mci->irqB_n(channel[INT_MCI_1]);
    my_bus->ahb_to_slave_socket.bind(mci->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_MCI, 4096);

    /* initialize the SP810 status & system control module */
    printm(d_realvieweb, "SYSCTRL0");
#ifdef REALVIEWEB_ARMV6K
    status_0.reset(new SYSCTRL_0("SP810_SYS_CTRL_0", 4096, 0x11400400, 0x06000000));
#endif

#ifdef REALVIEWEB_ARMV6
    status_0.reset(new SYSCTRL_0("SP810_SYS_CTRL_0", 4096, 0x11400400, 0x04000000));
#endif

#ifdef REALVIEWEB_ARMV5
    status_0.reset(new SYSCTRL_0("SP810_SYS_CTRL_0", 4096, 0x11400400, 0x02000000));
#endif
    status_0->clk(clk_slow->signal());
    status_0->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(status_0->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SYS_CTRL_0, 4096);

    /* initialize the SP810 status & system control module */
    printm(d_realvieweb, "SYSCTRL1");
    status_1.reset(new SYSCTRL_1("SP810_SYS_CTRL_1", 4096));
    status_1->clk(clk_slow->signal());
    status_1->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(status_1->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SYS_CTRL_1, 4096);

    /* initialize the PL093 synchronous static memory controller */
    printm(d_realvieweb, "SSMC");
    ssmc.reset(new SSMC("PL093_SSMC", 64 * 1024));
    ssmc->clk(clk_slow->signal());
    ssmc->rst_n(rst_n);
    my_bus->ahb_to_slave_socket.bind(ssmc->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SSMC, 64 * 1024);

#ifdef REALVIEWEB_ARMV6K
    /* initialize smp boot code */
    printm(d_realvieweb, "SMP BOOT ROM, size: %x", size_smp_boot_rom);
    smp_boot_rom.reset(new RAM("smp_boot_rom", size_smp_boot_rom));
    my_bus->ahb_to_slave_socket.bind(smp_boot_rom->ahb_slave_socket);
    my_bus->add_mapping(slave_id++, BASE_SMP_BOOT_ROM, size_smp_boot_rom);
#endif
}

