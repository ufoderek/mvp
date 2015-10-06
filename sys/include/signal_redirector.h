#ifndef _SIGNAL_REDIRECTOR_H_
#define _SIGNAL_REDIRECTOR_H_

#include <signal.h>
#include <debug_utils.h>

void sig_int(int num)
{
    exit(EXIT_SUCCESS);
    //ungetc(0x03, stdin); /* ctrl + C, enable to kill process inside virtual OS */
}

void sig_kill(int num)
{
    exit(EXIT_SUCCESS);
}

void sig_quit(int num)
{
    exit(EXIT_SUCCESS);
    //ungetc(0x1c, stdin); /* ctrl + \ */
}

void sig_stop(int num)
{
    exit(EXIT_SUCCESS);
}

void sig_tstp(int num)
{
    exit(EXIT_SUCCESS);
    //ungetc(0x1a, stdin); /* ctrl + Z */
}

void sig_term(int num)
{
    exit(EXIT_SUCCESS);
}

class signal_redirector
{
    public:

        signal_redirector(void)
        {
            printm(d_sig, "redirecting signal...");
            signal(SIGINT, sig_int);
            signal(SIGKILL, sig_kill);
            signal(SIGQUIT, sig_quit);
            signal(SIGSTOP, sig_stop);
            signal(SIGTERM, sig_term);
            signal(SIGTSTP, sig_tstp);
        }

        ~signal_redirector()
        {
            printe("restoring signal");
            signal(SIGINT, SIG_DFL);
            signal(SIGKILL, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGSTOP, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
        }

};

#endif
