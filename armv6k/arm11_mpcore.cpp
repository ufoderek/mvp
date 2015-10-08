#include <arm11_mpcore.h>
#include <debug_utils.h>

ARM11_MPCORE::ARM11_MPCORE(sc_module_name name, uint32_t addr, uint32_t second_addr, bool using_gdb, unsigned int gdb_port, unsigned int n_cores): sc_module(name)
{
    int i;

    scu.reset(new mp_scu("mp_scu", n_cores));
    gic_dist.reset(new gic2_dist("gic2_dist_internal", 4096, 1, false, n_cores, 64));
    gic_cpu_if.reset(new gic2_cpu_if("gic2_cpu_if_internal", 4096, gic_dist, n_cores, 64));

    switch (n_cores) {
        case 4:
            core3.reset(new ARMV6K("ARMV6K_3", 3, scu, gic_dist, gic_cpu_if, second_addr, false, gdb_port + 3));
        case 3:
            core2.reset(new ARMV6K("ARMV6K_2", 2, scu, gic_dist, gic_cpu_if , second_addr, false, gdb_port + 2));
        case 2:
            core1.reset(new ARMV6K("ARMV6K_1", 1, scu, gic_dist, gic_cpu_if, second_addr, false, gdb_port + 1));
        case 1:
            core0.reset(new ARMV6K("ARMV6K_0", 0, scu, gic_dist, gic_cpu_if, addr, using_gdb, gdb_port + 0));
            break;
        default:
            printb(d_arm11_mpcore, "constructor: error number of cores");

    }

    this->n_cores = n_cores;

    /* clk connection */
    gic_cpu_if->clk(clk_slow);
    core0->clk(clk);
    core1->clk(clk);
    core2->clk(clk);
    core3->clk(clk);

    /* bind interrupts from core tile to mp's gic */
    for (i = 32; i < 64; i++) {
        //scu->gic_irq_n[i](gic_irq_n[i]);
        gic_dist->spi_n[i - 32](gic_irq_n[i]);
    }

    /* bind mp gic's output to each core */
    for (i = 0; i < n_cores; i++) {
        gic_cpu_if->irq_n[i](irq_n[i]);
        gic_cpu_if->virq_n[i](virq_n[i]);
    }

    core0->irq_n(irq_n[0]);
    core1->irq_n(irq_n[1]);
    core2->irq_n(irq_n[2]);
    core3->irq_n(irq_n[3]);

    printm(d_arm11_mpcore, "initialized");
}

ARM11_MPCORE::~ARM11_MPCORE()
{
}


