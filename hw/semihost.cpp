#include <semihost.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bit_opt_v2.h>

using namespace std;

semihost::semihost(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    //make sure the size is aligned to 32-bit word
    if(mapping_size & mask(1, 0))
    {
        printb(d_semihost, "semihost size is not aligned to 32-bit word");
    }

    printm(d_semihost, "please disable uart's io redirection");
    printm(d_semihost, "fstat is buggy in thoery, but it works now");
    printm(d_semihost, "buf size = %d bytes, path size = %d bytes", buf_size, path_size);
}

semihost::~semihost()
{
}

bool semihost::read(uint32_t* data, uint32_t addr, int size)
{
    switch(addr)
    {
        case OFFSET_RET_VAL:
            printd(d_semihost, "read return val=%d", return_val);
            *data = return_val;
            break;
        default:
            if((addr >= OFFSET_BUF) && (addr <= (OFFSET_BUF + buf_size)))
            {
                *data = (uint8_t)buf[addr - OFFSET_BUF];

                printd(d_semihost, "read buf=%c", *data);
                if(addr == OFFSET_BUF)
                {
                    printd(d_semihost, "read buf");
                }
            }
            else
            {
                printb(d_semihost, "read offset 0x%X error", addr);
            }
            break;
    }

    return true;
}

bool semihost::write(uint32_t data, uint32_t addr, int size)
{
    switch(addr)
    {
        case OFFSET_TYPE:
            printd(d_semihost, "write type=%d", data);
            type = data;
            break;
        case OFFSET_FD:
            printd(d_semihost, "write fd=%d", data);
            fd = data;
            break;
        case OFFSET_FLAGS:
            printd(d_semihost, "write flags=%d", data);
            flags = data;
            break;
        case OFFSET_MODE:
            printd(d_semihost, "write mode=%d", data);
            mode = data;
            break;
        case OFFSET_WHENCE:
            printd(d_semihost, "write whence=%d", data);
            whence = data;
            break;
        case OFFSET_OFFSET:
            printd(d_semihost, "write offset=%d", data);
            offset = data;
            break;
        case OFFSET_COUNT:
            printd(d_semihost, "write count=%d", data);
            count = data;
            break;
        default:
            if(addr == OFFSET_SUBMIT)
            {
                printd(d_semihost, "submit");
                this->do_semihost();
            }
            else if((addr >= OFFSET_PATH) && (addr <= (OFFSET_PATH + path_size)))
            {
                path[addr - OFFSET_PATH] = (uint8_t)data;
                if(addr == OFFSET_PATH)
                {
                    printd(d_semihost, "write path");
                }
            }
            else if((addr >= OFFSET_BUF) && (addr <= (OFFSET_BUF + buf_size)))
            {
                buf[addr - OFFSET_BUF] = (uint8_t)data;
                //printd(d_semihost, "addr=%X offset=%d count=%d data=%c", addr, addr - OFFSET_BUF, count, data);
                if(addr == OFFSET_BUF)
                {
                    printd(d_semihost, "write buf");
                }
            }
            else
            {
                printb(d_semihost, "write to offset 0x%X error", addr);
            }
            break;
    }

    return true;
}

int semihost::convert_flags(int flags)
{
    int ret = 0;

    if((flags & 1) == 0)
    {
        ret |= O_RDONLY;
    }
    if(flags & 1)
    {
        ret |= O_WRONLY;
    }
    if(flags & 2)
    {
        ret |= O_RDWR;
    }
    if(flags & 0x8)
    {
        ret |= O_APPEND;
    }
    if(flags & 0x0200)
    {
        ret |= O_CREAT;
    }
    if(flags & 0x0400)
    {
        ret |= O_TRUNC;
    }
    if(flags & 0x0800)
    {
        ret |= O_EXCL;
    }
    if(flags & 0x2000)
    {
        ret |= O_SYNC;
    }
    if(flags & 0x4000)
    {
        ret |= O_NONBLOCK;
    }
    if(flags & 0x8000)
    {
        ret |= O_NOCTTY;
    }

    return ret;
}

void semihost::do_semihost()
{
    struct stat st;
    uint32_t* f = (uint32_t*)buf;

    switch(type)
    {
        case TYPE_OPEN:
            printd(d_semihost, "do_semihost open, path=%s, flags=%d, mode=%d", path, flags, mode);
            return_val = open(path, convert_flags(flags), mode);
            if(return_val == -1)
            {
                printb(d_semihost, "do_semihost open error (%s)", path);
            }
            break;
        case TYPE_READ:
            return_val = ::read(fd, buf, count);
            if(return_val == -1)
            {
                printb(d_semihost, "do_semihost read error");
            }
            buf[return_val] = '\0';
            printd(d_semihost, "do_semihost read=%s, (%d)", buf, return_val);
            break;
        case TYPE_WRITE:
            printd(d_semihost, "do_semihost write, fd=%d, buf=%s, count=%d", fd, buf, count);
            return_val = ::write(fd, buf, count);
            if(return_val == -1)
            {
                printb(d_semihost, "do_semihost write error");
            }
            break;
        case TYPE_LSEEK:
            printd(d_semihost, "do_semihost lseek");
            return_val = lseek(fd, offset, whence);
            if(return_val == -1)
            {
                printb(d_semihost, "do_semihost lseek error");
            }
            break;
        case TYPE_FSTAT:
            printd(d_semihost, "do_semihost fstat");
            return_val = fstat(fd, &st);
            //this is wrong, thie size of each field is not 4 bytes, but just ignore here
            f[0] = st.st_dev;
            f[1] = st.st_ino;
            f[2] = st.st_mode;
            f[3] = st.st_nlink;
            f[4] = st.st_uid;
            f[5] = st.st_gid;
            f[6] = st.st_rdev;
            f[7] = st.st_size;
            f[8] = st.st_blksize;
            f[9] = st.st_blocks;
            f[10] = st.st_atime;
            f[11] = st.st_mtime;
            f[12] = st.st_ctime;
            if(return_val == -1)
            {
                printb(d_semihost, "do_semihost fstat error");
            }
            break;
        case TYPE_CLOSE:
            printd(d_semihost, "do_semihost close");
            return_val = close(fd);
            if(return_val == -1)
            {
                printb(d_semihost, "do_semihost close error");
            }
            break;
        default:
            printb(d_semihost, "do_semihost type = %d, error", type);
    }
}

bool semihost::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    uint32_t local_address = addr & get_address_mask();

    if(write)
    {
        return this->write(data, local_address, length);
    }
    else
    {
        return this->read(&data, local_address, length);
    }
}
