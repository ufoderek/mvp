#ifndef _PRIVATE_REGION_H_
#define _PRIVATE_REGION_H_

#include <stdint.h>

class private_region
{
    private:
        uint32_t    mask;
        uint32_t    regs[1024];
        bool        valid[1024];

    public:
        private_region(uint32_t mask);
        void read(uint32_t addr, uint32_t* data);
        void write(uint32_t addr, uint32_t data);
};

#endif

