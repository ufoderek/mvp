#ifndef _GDBSTUB_H_
#define _GDBSTUB_H_

#include <armv7a.h>
//#include<board_versatilepb_addr.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* maximum packet size */
#define SIZE_PKT 1024

/* system stop reasons */
#define GDB_SIGNAL_0 0
#define GDB_SIGNAL_INT 2
#define GDB_SIGNAL_TRAP 5
#define GDB_SIGNAL_UNKNOWN 143
//#define REASON_START  0
//#define REASON_BKPT       1
//#define REASON_ERR        2




#define SP 13
#define LR 14
#define PC 15
#define CPSR 16
#define SPSR 17
#define FPS 18
#define PID 20
#define SYS 21
#define TTBR 22
#define DOM 23
//#define DFSR  24
//#define   IFSR    25
//#define FAR       26
#define INST 24
#define CYCLE 25




class armv7a;
class GDBStub
{
    public:
        /* public data */
        bool stop;
        bool step;
        int reason;         //stop reason

        /* the constructor & deconstructor */
        GDBStub(armv7a* arm, unsigned int port);
        ~GDBStub();

        /* public methods */
        void stub();

    private:
        /* private data */
        armv7a* arm;
        unsigned int port;
        int threadNum;                          //the thread for operation
        int sock_gdb;                           //socket descriptor of GDB
        int sock_cpu;                           //socket descriptor of MVP emulator
        struct sockaddr_in addr_gdb;            //socket address of GDB
        struct sockaddr_in addr_cpu;            //socket address of MVP emulator

        /* private methods */
        void setup(void);
        inline void pkt_continue(char*);
        inline void pkt_break_insert(char*);
        inline void pkt_break_remove(char*);
        inline void pkt_kill(void);
        inline void pkt_query(char*);
        inline void pkt_memRead(char*);
        inline void pkt_memWrite(char*);
        inline void pkt_reason(void);
        inline void pkt_regRead(char*);
        inline void pkt_regRead(void);
        inline void pkt_readWrite(char*);
        inline void pkt_step(char*);
        inline void pkt_thread(char*);
        inline void pkt_unknow(void);
        inline void checkReason(void);
        inline void parse(char*);
        inline uint8_t CharToInt(char);
        inline char IntToChar(uint8_t);
        inline bool checksumCmp(char*);
        inline void checksumAdd(char*);
        inline bool waitACK(void);
        inline void sendACK(bool);
};

#endif
