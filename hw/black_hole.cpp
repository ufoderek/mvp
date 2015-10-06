#include <black_hole.h>
#include <cstring>

using namespace std;

black_hole::black_hole(sc_module_name name, uint32_t mapping_size): ahb_slave_if(mapping_size), sc_module(name)
{
    char filename[128];
    strcpy(filename, name);
    strcat(filename, ".csv");
    fout = fopen(filename, "w");

    buf_size = 0;

    SC_THREAD(sucker);
}

black_hole::~black_hole()
{
    fclose(fout);
}

void black_hole::sucker()
{
    const double ten9 = 1000000000.0;
    const double ten6 = 1000000.0;

    const double period = ten9 / 44100.0;


    //fill buffer for 0.5 ms of pcm
    /*
    while(buf_size <= (0.5 * ten6 / period))
    {
        wait(suck);
    }
    */

    wait(suck);
    printm(d_black_hole, "start eating");

    while(1)
    {
        uint64_t current_time_ns = sc_time_stamp().to_default_time_units();

        if(buf_size >= 4)
        {
            buf_size -= 4;
        }
        else
        {
            buf_size = 0;
        }
        fprintf(fout, "%llu,%u\n", current_time_ns, buf_size);
        wait(period, SC_NS); //input pcm is 44100 Hz
    }
}

bool black_hole::read(uint32_t* data, uint32_t addr, int size)
{
    if(size == 4)
    {
        *data = buf_size;
        printd(d_black_hole, "buf_size = %d", *data);
    }
    else
    {
        printb(d_black_hole, "read error");
    }

    return true;
}

bool black_hole::write(uint32_t data, uint32_t addr, int size)
{
    suck.notify();

    buf_size += size;
    /*
    if(buf_size <= 22676)
    {
        buf_size += size;
        return true;
    }
    else
    {
        return false;
    }
    */

    return true;
}

bool black_hole::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
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
