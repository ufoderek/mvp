/* import header files */
#include <cstdlib>
#include <cstring>
#include <gdbstub.h>
#include <iostream>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <debug_utils.h>

using namespace std;

/* the constructor */
GDBStub::GDBStub(armv7a* arm, unsigned int port)
{
    stop = true;
    step = false;
    threadNum = -1;
    reason = GDB_SIGNAL_TRAP;
    this->arm = arm;
    this->port = port;
    sock_gdb = -1;
    sock_cpu = -1;
    printm(d_gdbstub, "using port: %u", port);
    setup();
}

/* the deconstructor */
GDBStub::~GDBStub()
{
    printm(d_gdbstub, "gdb disconnect");
    close(sock_gdb);
    close(sock_cpu);
}

/* the stub to prompt GDB */
void GDBStub::stub()
{
    char buff[SIZE_PKT];
    checkReason();

    do
    {
        memset(buff, '\0', SIZE_PKT);

        if(read(sock_gdb, buff, SIZE_PKT) > 1)
        {
            parse(buff);
        }
    }
    while(stop);
}

/* to build up the connection with GDB */
void GDBStub::setup(void)
{
    int length;
    printm(d_gdbstub, "waiting for GDB connection...");

    /* create connection */
    if((sock_cpu = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printb(d_gdbstub, "cannnot create socket connection");
    }

    else
    {
        length = sizeof(addr_gdb);
        addr_cpu.sin_family = AF_INET;
        addr_cpu.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_cpu.sin_port = htons(port);

        if(bind(sock_cpu, (struct sockaddr*)&addr_cpu, sizeof(addr_cpu)) == -1)
        {
            printb(d_gdbstub, "port binding error");
        }

        else
        {
            listen(sock_cpu, 1);
            sock_gdb = accept(sock_cpu, (struct sockaddr*)&addr_gdb, (socklen_t*)&length);
            printd(d_gdbstub, "GDB connection established");
        }
    }
}

/* to handle the continue command */
inline void GDBStub::pkt_continue(char* str)
{
    printd(d_gdbstub, "pkt_continue");

    if(str[0] != '\0')
    {
        int i = 0;
        uint32_t addr = 0;

        while(str[i] != '\0')
        {
            addr <<= 4;
            addr += CharToInt(str[i]);
            i++;
        }

        arm->regRead(&addr, PC);
    }

    step = false;
    stop = false;
    /* Notice: don't reply to GDB at this time */
}

/* to handle the break point & watch point insert command */
inline void GDBStub::pkt_break_insert(char* str)
{
    char buff[16];

    do
    {
        bool result = true;
        uint32_t addr = 0;
        int len = 0;
        int i = 2;
        memset(buff, '\0', 16);

        /*get address */
        while(str[i] != ',')
        {
            addr <<= 4;
            addr += CharToInt(str[i]);
            i++;
        }

        i++;

        /*get length */
        while(str[i] != '\0')
        {
            len <<= 4;
            len += CharToInt(str[i]);
            i++;
        }

        /* gdt break point type */
        printd(d_gdbstub, "pkt_break_insert: %x", addr);

        switch(str[0])
        {
                /* NOTE: here we ignore the difference of software and hardware break point */
            case '0':
                arm->bkptInsert(addr);
                break;       //software break point
            case '1':
                arm->bkptInsert(addr);
                break;       //hardware break point
            case '2':
                arm->wwatchInsert(addr);
                break;     //write watch point
            case '3':
                arm->rwatchInsert(addr);
                break;     //read watch point
            case '4':
                arm->awatchInsert(addr);
                break;     //access watch point
            default:
                result = false;
                break;
        }

        (result) ? strcpy(buff, "$OK") : strcpy(buff, "$E");
        checksumAdd(buff);
        //cout << buff << endl;
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* to handle the break point & watch point insert command */
inline void GDBStub::pkt_break_remove(char* str)
{
    char buff[16];

    do
    {
        bool result = true;
        uint32_t addr = 0;
        int len = 0;
        int i = 2;
        memset(buff, '\0', 16);

        /*get address */
        while(str[i] != ',')
        {
            addr <<= 4;
            addr += CharToInt(str[i]);
            i++;
        }

        i++;

        /*get length */
        while(str[i] != '\0')
        {
            len <<= 4;
            len += CharToInt(str[i]);
            i++;
        }

        /* break point type */
        switch(str[0])
        {
                /* NOTE: here we ignore the difference of software and hardware break point */
            case '0':
                arm->bkptRemove(addr);
                break;       //memory break point
            case '1':
                arm->bkptRemove(addr);
                break;       //hardware break point
            case '2':
                arm->wwatchRemove(addr);
                break;     //write watch point
            case '3':
                arm->rwatchRemove(addr);
                break;     //read watch point
            case '4':
                arm->awatchRemove(addr);
                break;     //access watch point
            default:
                result = false;
                break;
        }

        (result) ? strcpy(buff, "$OK") : strcpy(buff, "$E");
        checksumAdd(buff);
        //cout << buff << endl;
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* to handle the kill command */
inline void GDBStub::pkt_kill(void)
{
    /* NOTE: don't reply to GDB at this time */
    //kill(getpid(), SIGINT);     // interrupt the execution
    //kill(getpid(), SIGKILL);    // sucide and terminate myself
    exit(EXIT_SUCCESS);
}

/* to handle the query command */
inline void GDBStub::pkt_query(char* str)
{
    char buff[SIZE_PKT];

    do
    {
        memset(buff, '\0', SIZE_PKT);

        if(!strcmp(str, "Supported"))
        {
            snprintf(buff, SIZE_PKT, "$PacketSize=%x", SIZE_PKT);
        }

        //snprintf(buff, SIZE_PKT, "$PacketSize=%x;qXfer:features:read+", SIZE_PKT);
        else if(!strcmp(str, "Xfer:features:read:target.xml:0,ffb"))
        {
            strcpy(buff, "$");
        }

        //strcpy(buff, "$l<?xml version=\"1.0\"?><!DOCTYPE target SYSTEM \"gdb-target.dtd\"><target><xi:include href=\"arm-core.xml\"/><xi:include href=\"arm-vfp.xml\"/></target>");
        else if(!strcmp(str, "C"))
        {
            strcpy(buff, "$QC1");
        }

        else if(!strcmp(str, "Offsets"))
        {
            strcpy(buff, "$");
        }

        else if(!strcmp(str, "Symbol::"))
        {
            strcpy(buff, "$");
        }

        else
        {
            strcpy(buff, "$E");
        }

        checksumAdd(buff);
        //cout << buff << endl;
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* to handle the memory read command */
inline void GDBStub::pkt_memRead(char* str)
{
    printd(d_gdbstub, "pkt_memRead: [%s]", str);
    char buff[SIZE_PKT];

    do
    {
        uint32_t addr = 0;
        uint32_t len = 0;
        int i = 0;
        memset(buff, '\0', SIZE_PKT);
        buff[0] = '$';

        /* get address */
        while(str[i] != ',')
        {
            addr <<= 4;
            addr += CharToInt(str[i]);
            i++;
        }

        i++;

        /* get length */
        while(str[i] != '\0')
        {
            len <<= 4;
            len += CharToInt(str[i]);
            i++;
        }

        i = 0;
        /********************************************************************************
        * NOTE:
        * GDB doesn't automatically transform the address to 4-bytes alignment
        * thus, read byte once instead of word, or it may get incorrect data sometimes
        ********************************************************************************/
        printd(d_gdbstub, "pkt_memRead: addr=%X len=%X", addr, len);

        while(i < len)
        {
            uint32_t data;
            arm->memRead(&data, addr, 1);
            addr++;
            buff[++i] = IntToChar((data >> 4) & 0x000f);
            buff[++i] = IntToChar((data >> 0) & 0x000f);
        }

        checksumAdd(buff);
        printd(d_gdbstub, "pkt_memRead: result=%s", buff);
        //write(sock_gdb, buff, strlen(buff));
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* to handle the memory write command */
inline void GDBStub::pkt_memWrite(char* str)
{
}

/* to handle the halt reason command */
inline void GDBStub::pkt_reason(void)
{
    printm(d_gdbstub, "pkt_reason");
    char buff[SIZE_PKT];

    do
    {
        memset(buff, '\0', SIZE_PKT);
        //snprintf(buff, SIZE_PKT, "$T%02xthread:%02x;", reason, arm->num + 1);
        printm(d_gdbstub, "pkt_reason");
        snprintf(buff, SIZE_PKT, "$T%02xthread:%02x;", reason, 0 + 1);
        checksumAdd(buff);
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* to handle the read register command */
inline void GDBStub::pkt_regRead(char* str)
{
    char buff[16];

    do
    {
        int i = 0;
        uint16_t index = 0;
        memset(buff, '\0', 16);
        buff[0] = '$';

        while(str[i] != '\0')
        {
            /* NOTE: GDB use dec instead of hex here */
            index *= 10;
            index += CharToInt(str[i]);
            i++;
        }

        uint32_t data;
        i = 0;
        /* NOTE: GDB use R19 to represent the CPSR instead of R16 which we usually does */
        arm->regRead(&data, ((index == 19) ? CPSR : index));
        buff[++i] = IntToChar((data >> 4) & 0x000f);
        buff[++i] = IntToChar((data >> 0) & 0x000f);
        buff[++i] = IntToChar((data >> 12) & 0x000f);
        buff[++i] = IntToChar((data >> 8) & 0x000f);
        buff[++i] = IntToChar((data >> 20) & 0x000f);
        buff[++i] = IntToChar((data >> 16) & 0x000f);
        buff[++i] = IntToChar((data >> 28) & 0x000f);
        buff[++i] = IntToChar((data >> 24) & 0x000f);
        checksumAdd(buff);
        //cout << buff << endl;
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* to handle the read all register command */
inline void GDBStub::pkt_regRead(void)
{
    char buff[SIZE_PKT];

    do
    {
        int i = 0;
        memset(buff, '\0', SIZE_PKT);
        buff[0] = '$';

        for(uint16_t index = 0; index <= CPSR; index++)
        {
            uint32_t data;
            arm->regRead(&data, index);
            buff[++i] = IntToChar((data >> 4) & 0x000f);
            buff[++i] = IntToChar((data >> 0) & 0x000f);
            buff[++i] = IntToChar((data >> 12) & 0x000f);
            buff[++i] = IntToChar((data >> 8) & 0x000f);
            buff[++i] = IntToChar((data >> 20) & 0x000f);
            buff[++i] = IntToChar((data >> 16) & 0x000f);
            buff[++i] = IntToChar((data >> 28) & 0x000f);
            buff[++i] = IntToChar((data >> 24) & 0x000f);
        }

        checksumAdd(buff);
        //cout << buff << endl;
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* to handle the step command */
inline void GDBStub::pkt_step(char* str)
{
    if(str[0] != '\0')
    {
        uint32_t addr = 0;
        int i = 0;

        while(str[i] != '\0')
        {
            addr <<= 4;
            addr += CharToInt(str[i]);
            i++;
        }

        arm->regWrite(addr, PC);
    }

    printm(d_gdbstub, "step");
    step = true;
    stop = false;
    /* Notice: don't reply to GDB at this time */
}

/* to handle the thread operation command */
inline void GDBStub::pkt_thread(char* str)
{
    char buff[8];

    do
    {
        memset(buff, '\0', sizeof(buff));

        switch(str[0])
        {
            case 'c':
            case 'm':
            case 'M':
            case 'g':
            case 'G':
                strcpy(buff, "$OK");
                break;
            default:
                strcpy(buff, "$E");
                break;
        }

        checksumAdd(buff);
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* recieve an unknown command */
inline void GDBStub::pkt_unknow(void)
{
    do
    {
        char buff[8] = "$";
        checksumAdd(buff);
        write(sock_gdb, buff, strlen(buff));
    }
    while(!waitACK());
}

/* check the CPU stop reason */
inline void GDBStub::checkReason(void)
{
    char buff[SIZE_PKT];

    switch(reason)
    {
        case GDB_SIGNAL_TRAP:

            do
            {
                memset(buff, '\0', SIZE_PKT);
                //snprintf(buff, SIZE_PKT, "$T%02xthread:%02x;", reason, arm->num + 1);
                snprintf(buff, SIZE_PKT, "$T%02xthread:%02x;", reason, 0 + 1);
                checksumAdd(buff);
                write(sock_gdb, buff, SIZE_PKT);
            }
            while(!waitACK());

            break;
        default:
            break;
    }
}

/* to parse the GDB command */
inline void GDBStub::parse(char* str)
{
    char buff[SIZE_PKT];
    memset(buff, '\0', SIZE_PKT);
    //cout << str << endl;

    /* calculate and remove checksum */
    if(!checksumCmp(str))
    {
        sendACK(false); //checksum error
    }

    else
    {
        sendACK(true);
        int i = 1; //ignore the '$' character in header

        /* remove checksum information */
        while(str[i] != '#')
        {
            buff[i - 1] = str[i];
            i++;
        }

        /* parse the command */
        switch(buff[0])
        {
            case '-':
                break;      //acknowledgment
            case '+':
                break;      //acknowledgment
            case '!':
            case '?':
                pkt_reason();
                break;
            case 'a':
                pkt_unknow();
                break;        //reserved
            case 'A':
            case 'b':
            case 'B':
            case 'c':
                pkt_continue(&buff[1]);
                break;
            case 'C':
            case 'd':
            case 'D':
            case 'e':
                pkt_unknow();
                break;        //reserved
            case 'E':
                pkt_unknow();
                break;        //reserved
            case 'f':
                pkt_unknow();
                break;        //reserved
            case 'F':
                pkt_unknow();
                break;        //reserved
            case 'g':
                pkt_regRead();
                break;
            case 'G':
            case 'h':
                pkt_unknow();
                break;        //reserved
            case 'H':
                pkt_thread(&buff[1]);
                break;
            case 'i':
            case 'I':
            case 'j':
                pkt_unknow();
                break;        //reserved
            case 'J':
                pkt_unknow();
                break;        //reserved
            case 'k':
                pkt_kill();
                break;
            case 'K':
                pkt_unknow();
                break;        //reserved
            case 'l':
                pkt_unknow();
                break;        //reserved
            case 'L':
                pkt_unknow();
                break;        //reserved
            case 'm':
                pkt_memRead(&buff[1]);
                break;
            case 'M':
                pkt_memWrite(&buff[1]);
                break;
            case 'n':
                pkt_unknow();
                break;        //reserved
            case 'N':
                pkt_unknow();
                break;        //reserved
            case 'o':
                pkt_unknow();
                break;        //reserved
            case 'O':
                pkt_unknow();
                break;        //reserved
            case 'p':
                pkt_regRead(&buff[1]);
                break;
            case 'P':
            case 'q':
                pkt_query(&buff[1]);
                break;
            case 'Q':
            case 'r':
            case 'R':
            case 's':
                pkt_step(&buff[1]);
                break;
            case 'S':
            case 't':
            case 'T':
            case 'u':
                pkt_unknow();
                break;        //reserved
            case 'U':
                pkt_unknow();
                break;        //reserved
            case 'v':
                pkt_unknow();
                break;        //reserved
            case 'V':
                pkt_unknow();
                break;        //reserved
            case 'w':
                pkt_unknow();
                break;        //reserved
            case 'W':
                pkt_unknow();
                break;        //reserved
            case 'x':
                pkt_unknow();
                break;        //reserved
            case 'X':
            case 'y':
                pkt_unknow();
                break;        //reserved
            case 'Y':
                pkt_unknow();
                break;        //reserved
            case 'z':
                pkt_break_remove(&buff[1]);
                break;
            case 'Z':
                pkt_break_insert(&buff[1]);
                break;
            default:
                pkt_unknow();
                break;
        }
    }
}

/* translate a char hex number to integer */
inline uint8_t GDBStub::CharToInt(char input)
{
    switch(input)
    {
        case '0':
            return 0x0;
            break;
        case '1':
            return 0x1;
            break;
        case '2':
            return 0x2;
            break;
        case '3':
            return 0x3;
            break;
        case '4':
            return 0x4;
            break;
        case '5':
            return 0x5;
            break;
        case '6':
            return 0x6;
            break;
        case '7':
            return 0x7;
            break;
        case '8':
            return 0x8;
            break;
        case '9':
            return 0x9;
            break;
        case 'a':
        case 'A':
            return 0xa;
            break;
        case 'b':
        case 'B':
            return 0xb;
            break;
        case 'c':
        case 'C':
            return 0xc;
            break;
        case 'd':
        case 'D':
            return 0xd;
            break;
        case 'e':
        case 'E':
            return 0xe;
            break;
        case 'f':
        case 'F':
            return 0xf;
            break;
        default:
            return 0x0;
            break;
    }
}

/* translate a char hex number to integer */
inline char GDBStub::IntToChar(uint8_t input)
{
    static const char digits[] = "0123456789abcdef";
    input &= 0x000f;
    return digits[input];
}

/* to compare the checksum of GDB packet string */
inline bool GDBStub::checksumCmp(char* str)
{
    uint8_t sum = 0;
    bool result = false;
    int i = 1;

    /* Notice: the checksum is recorded as two digits within the input string in hex mode */
    while(str[i] != '\0')
    {
        if(str[i] == '#')
        {
            int value = 0;
            char tmp;
            tmp = str[++i];
            value = CharToInt(tmp) << 4;
            tmp = str[++i];
            value += CharToInt(tmp);
            result = (value == sum);
            break;
        }

        else
        {
            sum += (uint8_t)str[i];
            i++;
        }
    }

    return result;
}

/* to add the checksum of GDB packet string */
inline void GDBStub::checksumAdd(char* str)
{
    uint8_t sum = 0;
    int i = 1;

    while(i < strlen(str))
    {
        sum += (uint8_t)str[i];
        i++;
    }

    str[i++] = '#';
    str[i++] = IntToChar((sum >> 4) & 0x000f);
    str[i++] = IntToChar(sum & 0x000f);
    str[i++] = '\0';
}

/* wait ACK signal from GDB */
inline bool GDBStub::waitACK(void)
{
    char ch;

    while(read(sock_gdb, &ch, sizeof(char)) <= 0)
    {
        ;
    }

    if(ch == '+')   //ACK correct
    {
        return true;
    }

    else   //ACK error
    {
        return false;
    }
}

/* send ACK signal to GDB */
inline void GDBStub::sendACK(bool cond)
{
    char ack = (cond) ? '+' : '-';
    write(sock_gdb, &ack, sizeof(char));
}

