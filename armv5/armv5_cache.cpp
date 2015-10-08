#include<armv5.h>

/* initialize i_cache & d_cache */
void ARMV5::cacheInit(void)
{
    for (int i = 0; i < CACHE_WAY; i++)
        for (int j = 0; j < CACHE_SET; j++) {
            icache[i][j].valid = false;
            dcache[i][j].valid = false;
        }
}

/* search and read data from D-cache */
bool ARMV5::dcache_read(uint32_t addr)
{
    bool hit = false;
    //uint32_t offset = (addr >> 2) & 0x007;
    uint32_t index = (addr >> 5) & 0x00ff;
    uint32_t tag = addr & 0xffffe000;

    if (dcache[0][index].valid == true && dcache[0][index].tag == tag) {
        hit = true;
    }

    else if (dcache[1][index].valid == true && dcache[1][index].tag == tag) {
        hit = true;
    }

    else if (dcache[2][index].valid == true && dcache[2][index].tag == tag) {
        hit = true;
    }

    else if (dcache[3][index].valid == true && dcache[3][index].tag == tag) {
        hit = true;
    }

    else {
        dcache_update(addr);
    }

    return hit;
}

/* search and write data to D-cache */
bool ARMV5::dcache_write(uint32_t addr)
{
    bool hit = false;
    //uint32_t offset = (addr >> 2) & 0x007;
    uint32_t index = (addr >> 5) & 0x00ff;
    uint32_t tag = addr & 0xffffe000;

    if (dcache[0][index].valid == true && dcache[0][index].tag == tag) {
        hit = true;
    }

    else if (dcache[1][index].valid == true && dcache[1][index].tag == tag) {
        hit = true;
    }

    else if (dcache[2][index].valid == true && dcache[2][index].tag == tag) {
        hit = true;
    }

    else if (dcache[3][index].valid == true && dcache[3][index].tag == tag) {
        hit = true;
    }

    else {
        dcache_update(addr);
    }

    return hit;
}

/* to update the D-cache line */
void ARMV5::dcache_update(uint32_t vir)
{
    uint32_t index = (vir >> 5) & 0x00ff;
    uint32_t tag = vir & 0xffffe000;
    int way = 0;
    static int round[CACHE_SET] = {0};

    /* decide which way should be replace by round robin */
    switch (round[index] & 0x0003) {
        case 0:
            way = 0;
            round[index]++;
            break;
        case 1:
            way = 1;
            round[index]++;
            break;
        case 2:
            way = 2;
            round[index]++;
            break;
        case 3:
            way = 3;
            round[index] = 0;
            break;
    }

    dcache[way][index].tag = tag;
    dcache[way][index].valid = true;
}

/* search and read data from I-cache */
bool ARMV5::icache_read(uint32_t addr)
{
    bool hit = false;
    //uint32_t offset = (addr >> 2) & 0x007;
    uint32_t index = (addr >> 5) & 0x00ff;
    uint32_t tag = addr & 0xffffe000;

    if (icache[0][index].valid == true && icache[0][index].tag == tag) {
        hit = true;
    }

    else if (icache[1][index].valid == true && icache[1][index].tag == tag) {
        hit = true;
    }

    else if (icache[2][index].valid == true && icache[2][index].tag == tag) {
        hit = true;
    }

    else if (icache[3][index].valid == true && icache[3][index].tag == tag) {
        hit = true;
    }

    else {
        icache_update(addr);
    }

    return hit;
}

/* to update the I-cache line */
void ARMV5::icache_update(uint32_t vir)
{
    uint32_t index = (vir >> 5) & 0x00ff;
    uint32_t tag = vir & 0xffffe000;
    int way = 0;
    static int round[CACHE_SET] = {0};

    /* decide which way should be replace by round robin */
    switch (round[index] & 0x0003) {
        case 0:
            way = 0;
            round[index]++;
            break;
        case 1:
            way = 1;
            round[index]++;
            break;
        case 2:
            way = 2;
            round[index]++;
            break;
        case 3:
            way = 3;
            round[index] = 0;
            break;
    }

    icache[way][index].tag = tag;
    icache[way][index].valid = true;
}

