#include <armv7a_cp15.h>
#include <debug_utils.h>

armv7a_cp15::armv7a_cp15()
{
#ifdef CPU_ASSERT
    for(int a = 0; a < 16; a++)
    {
        for(int b = 0; b < 16; b++)
        {
            valid64[a][b] = false;
            for(int c = 0; c < 16; c++)
            {
                for(int d = 0; d < 16; d++)
                {
                    valid[a][b][c][d] = false;
                }
            }
        }
    }
#endif

    reset_a8();
    reset_core_id(0);
}

armv7a_cp15::~armv7a_cp15()
{
#ifdef CPU_ASSERT
    int b = 0;

    printe("list of used cp15 registers:");

    for(int a = 0; a < 16; a++)
    {
        for(int b = 0; b < 16; b++)
        {
            for(int c = 0; c < 16; c++)
            {
                for(int d = 0; d < 16; d++)
                {
                    if(valid[a][b][c][d])
                    {
                        b++;
                        printe("%d-%d-%d-%d", a, b, c, d);
                    }
                }
            }
        }
    }

#endif
}

void armv7a_cp15::reset_core_id(unsigned int core_id)
{
    this->core_id = core_id;
    write(MPIDR, core_id);
}

void armv7a_cp15::reset_a8()
{
    //Ref: qemu-0.15.1/target-arm/helper.c
    write(MIDR, 0x410fc080);
    write(CTR, 0x82048004);
    write(MMFR0 , 0x31100003);
    write(MMFR1 , 0x20000000);
    write(CLIDR, (1 << 27) | (2 << 24) | 3);
    write(CCSIDR0, 0xe007e01a); //16k L1 dcache
    write(CCSIDR1, 0x2007e01a); //16k L1 icache
    write(CCSIDR2, 0xf0000000); //no L2 cache
    write(SCTLR, 0x00c50078); //qemu cortex-a8
    write(SCR, B(110001)); //i guess
    write(TTBR1, 0); //check this
    write(TTBCR, 0); //ARMv7 reset value
    write(FCSEIDR, 0); //no impl, RAZ
    write(CONTEXTIDR, 0); //i guess
    write(PRRR, 0); //unkown reset value
    write(DFSR, 0);//unkown reset value
    write(DFAR, 0);//unkown reset value
    write(IFSR, 0);//unkown reset value
    write(IFAR, 0);//unkown reset value

    write(HSCTLR_M, 0); //disable pl2 stage 1 mmu (hypervisor's mmu)
    write(HCR_VM, 0); //disable pl0,1 stage 2 mmu (vm's mmu)
    write(TTBCR_EAE, 0); //using short-descriptor translation table
    write(HSTR, 0); //don't trap cp15 access
    write(VBAR, 0xFFFF0000); //for compatibility
}

uint32_t armv7a_cp15::read(armv7a_cp15_reg_names name)
{
    uint32_t hsb = get_field(name, 31, 24);
    uint32_t lsb = get_field(name, 23, 16);
    uint32_t crn = get_field(name, 15, 12);
    uint32_t opc1 = get_field(name, 11, 8);
    uint32_t crm = get_field(name, 7, 4);
    uint32_t opc2 = get_field(name, 3, 0);

#ifdef CPU_ASSERT
    if(valid[crn][opc1][crm][opc2])
    {
        return get_field(regs[crn][opc1][crm][opc2], hsb, lsb);
    }
    else
    {
        printb(core_id, d_armv7a_cp15, "cp15 read by name error, CRn=%d, opc1=%d, CRm=%d, opc2=%d", crn, opc1, crm, opc2);
    }
#else
    return get_field(regs[crn][opc1][crm][opc2], hsb, lsb);
#endif
}

uint64_t armv7a_cp15::read64(armv7a_cp15_reg_names64 name)
{
    uint32_t hsb = get_field(name, 31, 24);
    uint32_t lsb = get_field(name, 23, 16);
    uint32_t opc1 = get_field(name, 11, 8);
    uint32_t crm = get_field(name, 7, 4);

#ifdef CPU_ASSERT
    if(valid64[crm][opc1])
    {
        return get_field64(regs64[crm][opc1], hsb, lsb);
    }
    else
    {
        printb(core_id, d_armv7a_cp15, "cp15 read64 by name error, CRm=%d, opc1=%d", crm, opc1)
    }
#else
    return get_field64(regs64[crm][opc1], hsb, lsb);
#endif
}

void armv7a_cp15::write(armv7a_cp15_reg_names name, uint32_t value)
{
    uint32_t hsb = get_field(name, 31, 24);
    uint32_t lsb = get_field(name, 23, 16);
    uint32_t crn = get_field(name, 15, 12);
    uint32_t opc1 = get_field(name, 11, 8);
    uint32_t crm = get_field(name, 7, 4);
    uint32_t opc2 = get_field(name, 3, 0);

#ifdef CPU_ASSERT
    valid[crn][opc1][crm][opc2] = true;
#endif
    set_field(&regs[crn][opc1][crm][opc2], hsb, lsb, value);
}

void armv7a_cp15::write64(armv7a_cp15_reg_names64 name, uint64_t value)
{
    uint32_t hsb = get_field(name, 31, 24);
    uint32_t lsb = get_field(name, 23, 16);
    uint32_t opc1 = get_field(name, 11, 8);
    uint32_t crm = get_field(name, 7, 4);

#ifdef CPU_ASSERT
    valid64[crm][opc1] = true;
#endif
    set_field64(&regs64[crm][opc1], hsb, lsb, value);
}

void armv7a_cp15::read(uint32_t* data, uint32_t crn, uint32_t opc1, uint32_t crm, uint32_t opc2)
{
    uint32_t cr = setcr(crn, opc1, crm, opc2);

    if(cr == MPIDR)
    {
        printm_once(d_armv7a_cp15, "read MPIDR (cpuid = %d), noted that this is NOT complient with ARMv7-A", *data);
    }
    else if(cr == CCSIDR0)
    {
        uint32_t n = read(CSSELR_Level); //means the level-n cache

        if(n == 0)
        {
            *data = regs[0][1][0][0];
        }
        else if(n == 1)
        {
            *data = regs[0][1][0][8];
        }
        else if(n == 2)
        {
            *data = regs[0][1][0][9];
        }
        else
        {
            printb(d_armv7a_cp15, "cp15 read CSSIDR error, n=%d", n);
        }
        return;
    }

#ifdef CPU_ASSERT
    if(valid[crn][opc1][crm][opc2])
    {
        *data = regs[crn][opc1][crm][opc2];
    }
    else
    {
        printb(core_id, d_armv7a_cp15, "cp15 read error, CRn=%d, opc1=%d, CRm=%d, opc2=%d", crn, opc1, crm, opc2);
    }
#else
    *data = regs[crn][opc1][crm][opc2];
#endif
}

void armv7a_cp15::read64(uint32_t* data_h, uint32_t* data_l, uint32_t crm, uint32_t opc1)
{
    uint32_t cr = setcr64(crm, opc1);


#ifdef CPU_ASSERT
    if(valid64[crm][opc1])
    {
        uint64_t tmp = regs64[crm][opc1];
        *data_h = get_field64(tmp, 63, 32);
        *data_l = (uint32_t)tmp;
    }
    else
    {
        printb(core_id, d_armv7a_cp15, "cp15 read64 error, CRm=%d, opc1=%d", crm, opc1);
    }
#else
    uint64_t tmp = regs64[crm][opc1];
    *data_h = get_field64(tmp, 63, 32);
    *data_l = (uint32_t)tmp;
#endif
}

void armv7a_cp15::write(uint32_t data, uint32_t crn, uint32_t opc1, uint32_t crm, uint32_t opc2)
{

    uint32_t cr = setcr(crn, opc1, crm, opc2);
    uint32_t* ptr = &regs[crn][opc1][crm][opc2];

    if(cr == SCTLR)
    {
        bool mmu_old = this->read(SCTLR_M);
        *ptr = data;
        bool mmu_new = this->read(SCTLR_M);

        if(mmu_new != mmu_old)
        {
            printd(d_armv7a_cp15, "MMU %s", mmu_new ? "On" : "Off");
        }
    }
    else if(cr == TTBR0)
    {
        printd(d_armv7a_cp15, "write to TTRB0: 0x%X", data);
    }
    else if(cr == TTBR1)
    {
        printd(d_armv7a_cp15, "write to TTBR1: 0x%X", data);
    }
    else if(cr == TTBCR)
    {
        printd(d_armv7a_cp15, "write to TTBCR: 0x%X", data);
    }
    else if(cr == DACR)
    {
        printd(d_armv7a_cp15, "write to DACR: 0x%X", data);
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 5) && (opc2 == 0))
    {
        //invalidate all inst caches to PoU, also flashes branch target cache
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 5) && (opc2 == 1))
    {
        //invalidate inst caches by mva to pou
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 5) && (opc2 == 4))
    {
        //ISB
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 5) && (opc2 == 6))
    {
        //invalidate entire branch predictor array
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 10) && (opc2 == 1))
    {
        //clean data cache line by mva to poc
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 10) && (opc2 == 4))
    {
        //DSB
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 10) && (opc2 == 5))
    {
        //DMB
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 11) && (opc2 == 1))
    {
        //clean data cache line by mva to pou
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 14) && (opc2 == 1))
    {
        //clean and invalidate data cache line by mva to poc
    }
    else if((crn == 7) && (opc1 == 0) && (crm == 14) && (opc2 == 2))
    {
        //clean and invalidate data or unified cache line by set/way
    }
    else if((crn == 7) && (opc1 == 7) && (crm == 8) && (opc2 == 2))
    {
        printb(d_armv7a_cp15, "exit simulation, exit code = %d", data);
    }
    else if((crn == 8) && (opc1 == 0) && (crm == 7) && (opc2 == 0))
    {
        //invalidate unified tlb
    }
    else if((crn == 8) && (opc1 == 0) && (crm == 7) && (opc2 == 1))
    {
        //invalidate unified tlb entry by mva
    }
    else if((crn == 8) && (opc1 == 0) && (crm == 7) && (opc2 == 2))
    {
        //invalidate unified tlb by asid match
    }
    else if(cr == PRRR)
    {
        printd(d_armv7a_cp15, "write to PRRR: 0x%X", data);
    }
    else if(cr == NMRR)
    {
        printd(d_armv7a_cp15, "write to NMRR: 0x%X", data);
    }
    else if((crn == 13) && (opc1 == 0) && (crm == 0) && (opc2 == 1))
    {
        *ptr = data;
        printd(d_armv7a_cp15, "CONTEXTIDR_ASID: %X", read(CONTEXTIDR_ASID));
    }

#ifdef CPU_ASSERT
    valid[crn][opc1][crm][opc2] = true;
#endif
    regs[crn][opc1][crm][opc2] = data;
}

void armv7a_cp15::write64(uint32_t data_h, uint32_t data_l, uint32_t crm, uint32_t opc1)
{
    uint32_t cr = setcr64(crm, opc1);

    printd(d_armv7a_cp15, "write64 crm=%d opc1=%d data=0x%08X%08X", crm, opc1, data_h, data_l);

#ifdef CPU_ASSERT
    valid64[crm][opc1] = true;
#endif
    regs64[crm][opc1] = ((uint64_t)data_h << 32) | data_l;

    switch(cr)
    {
        case TTBR0_LPAE:
            printd(d_armv7a_cp15, "write TTBR0_LPAE=0x%08X%08X", data_h, data_l);
            break;
        case TTBR1_LPAE:
            printd(d_armv7a_cp15, "write TTBR1_LPAE=0x%08X%08X", data_h, data_l);
            break;
        case HTTBR_LPAE:
            printd(d_armv7a_cp15, "write HTTBR_LPAE=0x%08X%08X", data_h, data_l);
            break;
        case VTTBR_LPAE:
            printd(d_armv7a_cp15, "write VTTBR_LPAE=0x%08X%08X", data_h, data_l);
            break;
    }
}

