#include <cstring>
#include <memory>
#include <systemc.h>

#include <board_meteorvb.h>
//#include <board_meteorvb_addr.h>
#include <signal_redirector.h>

using namespace std;

#ifndef RELEASE
BEGIN_DEBUG_UTILS_HELPER
//debug_utils_setp(d_gic2_dist);
//debug_utils_setp(d_gic2_cpu_if);
//debug_utils_setp(d_gic2_vcpu_if);
//debug_utils_setp(d_gic2_vctrl);
//debug_utils_setp(d_armv7a_exception);
//wait(2344093965, SC_NS);
debug_utils_setl(d_inst_clean);
/*
debug_utils_setl(d_inst_clean_vm0);
debug_utils_setl(d_inst_clean_vm1);
debug_utils_setl(d_rf_dump_vm0);
debug_utils_setl(d_rf_dump_vm1);
*/
END_DEBUG_UTILS_HELPER
#endif

auto_ptr<board_meteorvb> board;
auto_ptr<signal_redirector> sig_redirector;

int sc_main(int argc, char* argv[])
{
    printm(d_main, "disable SystemC's warning messages of duplicated object names");
    sc_report_handler::set_actions(SC_ID_OBJECT_EXISTS_, SC_DO_NOTHING);

    int i = 1;
    bool using_vmm = false; //the vmm
    char path_vmm[128];
    bool using_gdb = false; //the GDB stub
    int gdb_port = 0;

    /* parsing arguments */
    while(i < argc)
    {
        if(!strcmp(argv[i], "--vmm"))
        {
            i++;

            if(i < argc)
            {
                strcpy(path_vmm, argv[i]);
                using_vmm = true;
            }
            else
            {
                printb(d_main, "error loading vmm image file");
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
    printm(d_main, "initializing Meteor Virtual Develepment Board...");
    board.reset(new board_meteorvb("mvb", using_gdb, gdb_port));
    printm(d_main, "Meteor Virtual Develepment Board initialized");

    if(using_vmm)
    {
        board->load_vmm(path_vmm, 0);
        board->start_addr = 0;
    }
    else
    {
        printb(d_main, "parameter error");
    }

    printm(d_main, "start simulating...");
    sc_start();

    return 0;
}
