#include <armv5.h>
#include <armv5_cycle.h>
#include <bit_opt.h>

sc_event ARMV5::shared_event;
ARMV5::ARMV5(sc_module_name name, uint32_t addr, bool using_gdb, unsigned int gdb_port): sc_module(name)
{
    this->core_id = 0;
    cp15.c0_idcode = 0x41069265;
    cp15.c0_cachetype = 0x1dd20d2;
    cp15.c0_cachetype = 0x1d112152;

    gdb = 0;
    this->using_gdb = using_gdb;
    this->gdb_port = gdb_port;
    gdb_init = false;
    using_prog = false;

    bkptRemove();   // clear all break points
    watchRemove();  // clear all watch points
    rfInit(addr);
    cpInit();
    cacheInit();
    delay = 0;
    mmu_enable = false;
    icache_enable = false;
    dcache_enable = false;
    vectorHi = false;

    /* start to simulate */
    SC_THREAD(run);
    sensitive << clk.pos();
    //dont_initialize();

    SC_METHOD(shared_event_listener);
    sensitive << ARMV5::shared_event;

    SC_METHOD(irq_n_listener);
    sensitive << irq_n.neg();

}

/* the deconstructor */
ARMV5::~ARMV5(void)
{
    if (gdb == 0) {
        delete[] gdb;
    }
}

/* read instruction from RAM within 32-bits ARM mode */
bool ARMV5::inst_arm_read(void)
{
    int fault = FAULT_NONE;
    uint32_t addr = 0;
    bool cachable = true;

    if ((rf.pc & B8(11)) != 0) {
        printb(core_id, d_armv5, "instruction address is not word boundary aligned: 0x%X", rf.pc);
    }

    /* address translation */
    if (mmu_enable) {
        fault = vir2phy(rf.pc, &addr, CPSR_MODE(rf.cpsr), false, &cachable);
    }

    else {
        addr = rf.pc;
        fault = FAULT_NONE;
    }

    /* memory access */
    if (fault == FAULT_NONE) {
        if (icache_enable && cachable) {
            if (!icache_read(rf.pc)) {
                delay += CYC_ICACHE_MISS;
            }
        }

        else {
            delay += 2;
        }

        if (bus_read(&inst, addr, 4)) {
            return true;
        }

        else {
            fault = FAULT_EXTERNAL;
        }
    }

    /* fault handler */
    cp15.c5_ifsr = fault;
    cp15.c6_far = rf.pc;
    return false;
}

/* running the ARM CPU core */
void ARMV5::run(void)
{
    while (1) {
        uint32_t old_mode = CPSR_MODE(rf.cpsr);
        static bool in_isr = false;

        printd(d_inst, "PC=%X", rf.pc);
        //printf("%u\n", sc_time_stamp().value());
        delay = 0;

        /* setup GDB connection */
        if (unlikely(using_gdb && !gdb_init)) {
            gdb = new GDBStub(this, gdb_port);
            gdb_init = true;
        }

        /* detecting the interrupt signal and fetching the instruction */
        /*if ((fiq_n == false) && (!CPSR_F(rf.cpsr))) {   // FIQ
            arm_except_fiq();
        }

        else */if ((irq_n == false) && (!CPSR_I(rf.cpsr))) {  // IRQ
            printd(d_armv5, "enter irq, old_pc=0x%X", rf.pc);
            in_isr = true;
            arm_except_irq();
            printd(d_armv5, "enter irq, new_pc=0x%X", rf.pc);
        }


        /* to communicate with GDB server */
        if (unlikely(using_gdb)) {
            /* check if the break point reaching */
            if (unlikely((bkptCheck(rf.pc)) || gdb->step)) {
                gdb->reason = GDB_SIGNAL_TRAP;
                gdb->step = false;
                gdb->stop = true;
            }

            /* pause the running */
            if (unlikely(gdb->stop)) {
                gdb->stub();
            }
        }

        if (unlikely(CPSR_T(rf.cpsr))) {
            printb(core_id, d_armv5, "@0x%.8x 16-bits Thumb mode hasn't been implemented yet!!", rf.pc);

            //if(fetchThumb() == true)
            //  exeThumb();

            //inst_thumb++;
        }

        else if (unlikely(CPSR_J(rf.cpsr))) {
            printb(core_id, d_armv5, "@0x%.8x 8-bits JAVA mode hasn't been implemented yet!!", rf.pc);

            //if(fetchJava() == true)
            //  exeJava();

            //inst_java++;
        }

        else {
            if (likely(fetchARM())) {
                exeARM();
            }

            else {
                delay += CYC_UNDO;
            }
        }

        //delay--;
        //wait(clk.period() * delay);
        wait();
        /* do the work and then wait */
    }
}

bool ARMV5::check_be()
{
    bool b = cp15.c1_b();
    bool e = (CPSR_E(rf.cpsr)) ? true : false;

    if (!b && !e) {
        return true;
    }

    else {
        printb(core_id, d_armv5, "%s endian check error", current_inst_name);
    }
}

bool ARMV5::check_ua(uint32_t* addr, uint8_t nbytes)
{
    if (cp15.version() == 5) {
        if (cp15.c1_a()) {
            switch (nbytes) {
                case 1:
                    return true;
                case 2:

                    if (*addr & M(0)) {
                        printb(core_id, d_armv5, "%s alignment exception", current_inst_name);
                    }

                    break;
                case 4:

                    if (*addr & MM(1, 0)) {
                        printb(core_id, d_armv5, "%s alignment exception", current_inst_name);
                    }

                    break;
                case 8:

                    if (*addr & MM(2, 0)) {
                        printb(core_id, d_armv5, "%s alignment exception", current_inst_name);
                    }

                    break;
                default:
                    printb(core_id, d_armv5, "check_ua() fault");
                    break;
            }

            return true;
        }

        else {
            switch (nbytes) {
                case 1:
                    return true;
                case 2:
                    *addr &= CM(0);
                    return true;
                case 4:
                    *addr &= CMM(1, 0);
                    return true;
                case 8:
                    *addr &= CMM(2, 0);
                    return true;
                default:
                    printb(core_id, d_armv5, "check_ua() fault");
            }

            return false;
        }
    }

    else if (cp15.version() == 6) {
        printb(core_id, d_armv5, "align check");
        bool u = cp15.c1_u();
        bool a = cp15.c1_a();

        if (!u && !a) {
        }
        else if (!u && a) {
        }
        else if (u && !a) {
        }
        else if (u && a) {
        }
        else {
            printb(core_id, d_armv5, "%s align check failed", current_inst_name);
        }
    }

    else {
        printb(core_id, d_armv5, "check_ua() version error");
    }

    return false;
}
