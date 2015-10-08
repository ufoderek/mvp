#include <cstring>
#include <memory>
#include <systemc.h>
#include <main_realvieweb.h>
#include <board_realvieweb.h>
#include <board_realvieweb_addr.h>
#include <signal_redirector.h>

using namespace std;

#ifndef RELEASE
void init_debug_units()
{
    //setp(d_private);
    //setl(d_uxtab);
    //setl(d_sxtab);
    //setl(d_uxtah);
    //setl(d_inst);
    //setl(d_ram);
    //setp(d_gdbstub);
    //setl(d_armv5);
    //setp(d_armv5_gdb);
    //setl(d_armv5_bus);
    setl(d_inst);
    //setl(d_mmu);
    //setl(d_armv5_cp);
    //setl(d_armv6_mmu);
    //setl(d_mmu);
    //setl(d_mmu2);
    //setl(d_fmaster);
    //setl(d_arbiter);
    //setl(d_bus_slave);
    //setl(d_bus);
    //setl(d_gic);
    //setl(d_ssmc);
    //setp(d_gic);
    //setp(d_gic_in);
}
#endif

auto_ptr<board_realvieweb> board;
auto_ptr<signal_redirector> sig_redirector;

/* the SystemC main function */
int sc_main(int argc, char* argv[])
{

    printm(d_main, "disable warning message of duplicated object name");
    sc_report_handler::set_actions(SC_ID_OBJECT_EXISTS_, SC_DO_NOTHING);
    init_debug_units();
    int i = 1;

    bool using_kernel = false;  // the bare OS kernel
    char* path_kernel = NULL;

    bool using_initrd = false;  // the initrd file system or program data
    char* path_initrd = NULL;

    bool using_prog = false;    // the application program
    char* path_prog = NULL;

    bool using_gdb = false;     // the GDB stub
    int gdb_port = 0;

    bool using_lcd = false;     // the LCD panel emulator
    bool log_asm = false;
    bool log_bus = false;

    /* print out the program messages */
    system("clear");

    /* parsing arguments */
    while (i < argc) {
        if (!strcmp(argv[i], "--kernel")) { // kernel
            i++;

            if (i < argc) {
                path_kernel = (char*)malloc(sizeof(char) * (strlen(argv[i]) + 1));
                strcpy(path_kernel, argv[i]);
                using_kernel = true;
            }

            else {
                printm(d_main, "ERROR: hasn't assigned the kernel path");
                exit(EXIT_SUCCESS);
            }
        }

        else if (!strcmp(argv[i], "--initrd")) {  // initrd
            i++;

            if (i < argc) {
                path_initrd = (char*)malloc(sizeof(char) * (strlen(argv[i]) + 1));
                strcpy(path_initrd, argv[i]);
                using_initrd = true;
            }

            else {
                printm(d_main, "hasn't assigned the initrd path");
                exit(EXIT_SUCCESS);
            }
        }

        else if (!strcmp(argv[i], "--prog")) {  // program
            i++;

            if (i < argc) {
                path_prog = (char*)malloc(sizeof(char) * (strlen(argv[i]) + 1));
                strcpy(path_prog, argv[i]);
                using_prog = true;
            }

            else {
                printm(d_main, "hasn't assigned the program path");
                exit(EXIT_SUCCESS);
            }
        }

        else if (!strcmp(argv[i], "--gdb")) {
            using_gdb = true;
        }

        else if (!strcmp(argv[i], "--lcd")) {
            using_lcd = true;
        }

        else if (!strcmp(argv[i], "--port")) {
            i++;
            gdb_port = atoi(argv[i]);
        }

        else {
            printm(d_main, "unknown argument: %s", argv[i]);
            exit(EXIT_SUCCESS);
        }

        i++;
    }

    sig_redirector.reset(new signal_redirector());

    printm(d_main, "initialize board...");
    board.reset(new board_realvieweb("reb", using_gdb, gdb_port));
    printm(d_main, "board initialized");

    if ((using_kernel) == true && (using_prog == true)) {
        printm(d_main, "ERROR: both -kernel and -prog flags are used");
    }

    else if ((using_kernel == false) && (using_prog == false)) {
        printm(d_main, "ERROR: didn't asigned the simulation software!!");
    }

    else {   // set up the virtual platform and run
        if (using_initrd) {
            board->load_initrd(path_initrd);
        }

        if (using_kernel) {
            board->load_kernel(path_kernel);
            board->load_boot();
#ifdef REALVIEWEB_ARMV6K
            board->load_smp_boot();
#endif
            board->load_atag();
            board->start_addr = 0;
        }

        else if (using_prog) {
            board->load_prog(path_prog);
            board->start_addr = ADDR_PROG;
        }

        else {
            /* something might be wrong here */
            printm(d_main, "parameter error");
            exit(EXIT_SUCCESS);
        }

        //sc_set_time_resolution(1, SC_NS);
        /*
        uint32_t tmp = 0;
        board->arm->cpRead(&tmp, 1, 0, 0);
        board->arm->cpWrite(tmp, 1, 0, 0);
        */

        printm(d_main, "start simulating...");
        sc_start();
    }

    return 0;
}
