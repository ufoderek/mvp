#ifndef _SEMIHOST_H_
#define _SEMIHOST_H_

#include <stdint.h>
#include <systemc.h>
#include <ahb_slave_if.h>

enum semihost_type
{
    TYPE_OPEN = 0,
    TYPE_READ = 1,
    TYPE_WRITE = 2,
    TYPE_LSEEK = 3,
    TYPE_FSTAT = 4,
    TYPE_CLOSE = 5,
};

typedef enum semihost_offset
{
    OFFSET_TYPE = 0x0,
    OFFSET_FD = 0x4,
    OFFSET_FLAGS = 0x8,
    OFFSET_MODE = 0xC,
    OFFSET_WHENCE = 0x10,
    OFFSET_OFFSET = 0x14,
    OFFSET_COUNT = 0x18,
    OFFSET_RET_VAL = 0x1C,
    OFFSET_SUBMIT = 0x100,
    OFFSET_PATH = 0x200,
    OFFSET_BUF = 0x2000
} semihost_offset;

#define path_size 1024
#define buf_size 10240

class semihost: public ahb_slave_if, public sc_module
{
    public:
        SC_HAS_PROCESS(semihost);
        semihost(sc_module_name name, uint32_t mapping_size);
        ~semihost();

        bool read(uint32_t*, uint32_t, int);
        bool write(uint32_t, uint32_t, int);

        int convert_flags(int flags);
        void do_semihost();

        virtual bool local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length);

    private:
        enum semihost_type type;
        int fd;
        int flags;
        int mode;
        int whence;
        int offset;
        int count;

        uint8_t path[path_size];
        uint8_t buf[buf_size];

        int return_val;
};

#endif
