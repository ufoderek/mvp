#include <cstring>
#include <memory>
#include <systemc.h>

#include <board_realviewpb.h>
#include <board_realviewpb_addr.h>
#include <signal_redirector.h>

using namespace std;

#ifndef RELEASE
BEGIN_DEBUG_UTILS_HELPER
//wait(214664770, SC_NS);
//debug_utils_setl(d_inst_clean);
//debug_utils_setl(d_rf_dump);
//debug_utils_setp(d_gic2_cpu_if);
//debug_utils_setp(d_gic2_dist);
END_DEBUG_UTILS_HELPER
#endif

auto_ptr<board_realviewpb> board;
auto_ptr<signal_redirector> sig_redirector;

int sc_main(int argc, char* argv[])
{
    printm(d_main, "disable SystemC's warning messages of duplicated object names");
    sc_report_handler::set_actions(SC_ID_OBJECT_EXISTS_, SC_DO_NOTHING);

    int i = 1;
    bool using_kernel = false; //the bare OS kernel
    char path_kernel[128];
    bool using_initrd = false; //the initrd file system or program data
    char path_initrd[128];
    bool using_prog = false; //the application program
    char path_prog[128];
    bool using_gdb = false; //the GDB stub
    int gdb_port = 0;

    /* parsing arguments */
    while(i < argc)
    {
        if(!strcmp(argv[i], "--kernel"))
        {
            i++;

            if(i < argc)
            {
                strcpy(path_kernel, argv[i]);
                using_kernel = true;
            }
            else
            {
                printb(d_main, "error loading Linux kernel image file");
            }
        }
        else if(!strcmp(argv[i], "--initrd"))
        {
            i++;

            if(i < argc)
            {
                strcpy(path_initrd, argv[i]);
                using_initrd = true;
            }
            else
            {
                printb(d_main, "error loading initrd image file");
            }
        }
        else if(!strcmp(argv[i], "--prog"))
        {
            i++;

            if(i < argc)
            {
                strcpy(path_prog, argv[i]);
                using_prog = true;
            }
            else
            {
                printb(d_main, "error loading program");
            }
        }
        else if(!strcmp(argv[i], "--gdb"))
        {
            using_gdb = true;
        }
        else if(!strcmp(argv[i], "--port"))
        {
            i++;
            gdb_port = atoi(argv[i]);
        }
        else
        {
            printb(d_main, "unknown argument: %s", argv[i]);
        }

        i++;
    }

    printm(d_main, "initialize signal_redirector");
    sig_redirector.reset(new signal_redirector());
    printm(d_main, "initializing realview platform board...");
    board.reset(new board_realviewpb("rpb", using_gdb, gdb_port));
    printm(d_main, "realview platform board initialized");

    if((using_kernel) == true && (using_prog == true))
    {
        printb(d_main, "using --kernel and --prog flags at the same time");
    }

    else if((using_kernel == false) && (using_prog == false))
    {
        printb(d_main, "too less arguemnts");
    }
    else
    {
        if(using_initrd)
        {
            board->load_initrd(path_initrd);
        }

        if(using_kernel)
        {
            board->load_kernel(path_kernel);
            board->load_boot();
            board->load_atag();
            board->start_addr = 0;
        }
        else if(using_prog)
        {
            board->load_prog(path_prog);
            board->start_addr = ADDR_PROG;
        }
        else
        {
            printb(d_main, "parameter error");
        }

        printm(d_main, "start simulating...");
        sc_start();
    }

    return 0;
}
