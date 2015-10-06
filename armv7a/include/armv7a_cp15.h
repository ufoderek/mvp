#ifndef _ARMV7A_CP15_H_
#define _ARMV7A_CP15_H_

#include <stdint.h>
#include <bits.h>
#include <bit_opt_v2.h>
#include <armv7a_mmu.h>


#define bit4(x) ( (x) & B(1111) )
#define bit8(x) ( (x) & B(1111 1111) )

#define setcr(crn, opc1, crm, opc2) ( (bit8(31)<<24) | (bit8(0)<<16) | (bit4(crn)<<12) | (bit4(opc1)<<8) | (bit4(crm)<<4) | (bit4(opc2)<<0) )

#define setfd(reg_name, hsb, lsb) ( ((reg_name) & 0xFFFF) | (bit8(hsb)<<24) | (bit8(lsb)<<16) )

#define setcr64(crm, opc1) ( (bit8(63)<<24) | (bit8(0)<<16) | (bit4(0)<<12) | (bit4(opc1)<<8) | (bit4(crm)<<4) | (bit4(0)<<0) )

#define setfd64(reg_name, hsb, lsb) ( ((reg_name) & 0xFFFF) | (bit8(hsb)<<24) | (bit8(lsb)<<16) )

enum armv7a_cp15_reg_names64
{
    TTBR0_LPAE = setcr64(2, 0),
    TTBR1_LPAE = setcr64(2, 1),
    HTTBR_LPAE = setcr64(2, 4),
    VTTBR_LPAE = setcr64(2, 6),

    VTTBR_LPAE_BADDR = setfd64(VTTBR_LPAE, 39, 0),
    VTTBR_LPAE_VMID = setfd64(VTTBR_LPAE, 55, 48)
};

enum armv7a_cp15_reg_names
{
    // we divide 32-bit values into 6 fields
    //    hsb     lsb     crn   opc1   crm  opc2
    // (31,24)|(23,16)|(15,12)|(11,8)|(7,4)|(3,0)

    // Full Registers, hsb=31, lsb=0, other 4 fields are variable

    MIDR = setcr(0, 0, 0, 0),       // = regs[0][0][0][0] = MIDR[31:0]
    CTR = setcr(0, 0, 0, 1),
    MPIDR = setcr(0, 0, 0, 5),
    MMFR0 = setcr(0, 0, 1, 4),
    MMFR1 = setcr(0, 0, 1, 5),
    MMFR2 = setcr(0, 0, 1, 6),
    MMFR3 = setcr(0, 0, 1, 7),
    CCSIDR0 = setcr(0, 1, 0, 0),    //hack
    CCSIDR1 = setcr(0, 1, 0, 6),    //hack
    CCSIDR2 = setcr(0, 1, 0, 7),    //hack
    CLIDR = setcr(0, 1, 0, 1),
    CSSELR = setcr(0, 2, 0, 0),
    VPIDR = setcr(0, 4, 0, 0),

    SCTLR = setcr(1, 0, 0, 0),
    SCR = setcr(1, 0, 1, 0),
    HSCTLR = setcr(1, 4, 0, 0),
    HCR = setcr(1, 4, 1, 0),
    HDCR = setcr(1, 4, 1, 1),
    HCPTR = setcr(1, 4, 1, 2),
    HSTR = setcr(1, 4, 1, 3),

    TTBR0 = setcr(2, 0, 0, 0),
    TTBR1 = setcr(2, 0, 0, 1),
    TTBCR = setcr(2, 0, 0, 2),
    HTCR = setcr(2, 4, 0, 2),
    VTCR = setcr(2, 4, 1, 2),

    DACR = setcr(3, 0, 0, 0),

    DFSR = setcr(5, 0, 0, 0),
    IFSR = setcr(5, 0, 0, 1),
    ADFSR = setcr(5, 0, 1, 0),
    AIFSR = setcr(5, 0, 1, 1),
    HADFSR = setcr(5, 4, 1, 0),
    HAIFSR = setcr(5, 4, 1, 1),
    HSR = setcr(5, 4, 2, 0),

    DFAR = setcr(6, 0, 0, 0),
    IFAR = setcr(6, 0, 0, 2),
    HDFAR = setcr(6, 4, 0, 0),
    HIFAR = setcr(6, 4, 0, 2),
    HPFAR = setcr(6, 4, 0, 4),

    PRRR = setcr(10, 0, 2, 0),
    NMRR = setcr(10, 0, 2, 1),

    VBAR = setcr(12, 0, 0, 0),
    MVBAR = setcr(12, 0, 0, 1),
    ISR = setcr(12, 0, 1, 0),
    HVBAR = setcr(12, 4, 0, 0),

    FCSEIDR = setcr(13, 0, 0, 0),
    CONTEXTIDR = setcr(13, 0, 0, 1),
    TPIDR_URO = setcr(13, 0, 0, 3),
    HTPIDR = setcr(13, 4, 0, 2),

    // Register Fields, modify 2 fields: hsb and lsb, leaving other fields unchanged

    CONTEXTIDR_ASID = setfd(CONTEXTIDR, 7, 0),          // = CONTEXTIDR[7:0] = regs[13][0][0][1][7:0]
    CONTEXTIDR_PROCID = setfd(CONTEXTIDR, 31, 8),       // = CONTEXTIDR[31:8]
    CONTEXTIDR_PROCID_LONG = setfd(CONTEXTIDR, 31, 0),

    CSSELR_Level = setfd(CSSELR, 3, 1),
    CSSELR_InD = setfd(CSSELR, 0, 0),

    FCSEIDR_PID = setfd(FCSEIDR, 31, 25),

    HCR_TGE = setfd(HCR, 27, 27),
    HCR_TVM = setfd(HCR, 26, 26),
    HCR_TTLB = setfd(HCR, 25, 25),
    HCR_TPU = setfd(HCR, 24, 24),
    HCR_TPC = setfd(HCR, 23, 23),
    HCR_TSW = setfd(HCR, 22, 22),
    HCR_TAC = setfd(HCR, 21, 21),
    HCR_TIDCP = setfd(HCR, 20, 20),
    HCR_TSC = setfd(HCR, 19, 19),
    HCR_TID3 = setfd(HCR, 18, 18),
    HCR_TID2 = setfd(HCR, 17, 17),
    HCR_TID1 = setfd(HCR, 16, 16),
    HCR_TID0 = setfd(HCR, 15, 15),
    HCR_TWE = setfd(HCR, 14, 14),
    HCR_TWI = setfd(HCR, 13, 13),
    HCR_DC = setfd(HCR, 12, 12),
    HCR_BSU = setfd(HCR, 11, 10),
    HCR_FB = setfd(HCR, 9, 9),
    HCR_VA = setfd(HCR, 8, 8),
    HCR_VI = setfd(HCR, 7, 7),
    HCR_VF = setfd(HCR, 6, 6),
    HCR_AMO = setfd(HCR, 5, 5),
    HCR_IMO = setfd(HCR, 4, 4),
    HCR_FMO = setfd(HCR, 3, 3),
    HCR_PTW = setfd(HCR, 2, 2),
    HCR_SWIO = setfd(HCR, 1, 1),
    HCR_VM = setfd(HCR, 0, 0),

    HDCR_TDRA = setfd(HDCR, 11, 11),
    HDCR_TDOSA = setfd(HDCR, 10, 10),
    HDCR_TDA = setfd(HDCR, 9, 9),
    HDCR_TDE = setfd(HDCR, 8, 8),
    HDCR_HPME = setfd(HDCR, 7, 7),
    HDCR_TPM = setfd(HDCR, 6, 6),
    HDCR_TPMCR = setfd(HDCR, 5, 5),
    HDCR_HPMN = setfd(HDCR, 4, 0),

    HSCTLR_TE = setfd(HSCTLR, 30, 30),
    HSCTLR_EE = setfd(HSCTLR, 25, 25),
    HSCTLR_FI = setfd(HSCTLR, 21, 21),
    HSCTLR_WXN = setfd(HSCTLR, 19, 19),
    HSCTLR_I = setfd(HSCTLR, 12, 12),
    HSCTLR_CP15BEN = setfd(HSCTLR, 5, 5),
    HSCTLR_C = setfd(HSCTLR, 2, 2),
    HSCTLR_A = setfd(HSCTLR, 1, 1),
    HSCTLR_M = setfd(HSCTLR, 0, 0),

    HTCR_SH0 = setfd(HTCR, 13, 12),
    HTCR_ORGN0 = setfd(HTCR, 11, 10),
    HTCR_IRGN0 = setfd(HTCR, 9, 8),
    HTCR_T0SZ = setfd(HTCR, 2, 0),

    HSTR_T0 = setfd(HSTR, 0, 0),
    HSTR_T1 = setfd(HSTR, 1, 1),
    HSTR_T2 = setfd(HSTR, 2, 2),
    HSTR_T3 = setfd(HSTR, 3, 3),
    HSTR_T4 = setfd(HSTR, 4, 4),
    HSTR_T5 = setfd(HSTR, 5, 5),
    HSTR_T6 = setfd(HSTR, 6, 6),
    HSTR_T7 = setfd(HSTR, 7, 7),
    HSTR_T8 = setfd(HSTR, 8, 8),
    HSTR_T9 = setfd(HSTR, 9, 9),
    HSTR_T10 = setfd(HSTR, 10, 10),
    HSTR_T11 = setfd(HSTR, 11, 11),
    HSTR_T12 = setfd(HSTR, 12, 12),
    HSTR_T13 = setfd(HSTR, 13, 13),
    HSTR_T15 = setfd(HSTR, 15, 15),
    HSTR_TTEE = setfd(HSTR, 16, 16),
    HSTR_TJDBX = setfd(HSTR, 17, 17),

    PRRR_NOS7 = setfd(PRRR, 31, 31),
    PRRR_NOS6 = setfd(PRRR, 30, 30),
    PRRR_NOS5 = setfd(PRRR, 29, 29),
    PRRR_NOS4 = setfd(PRRR, 28, 28),
    PRRR_NOS3 = setfd(PRRR, 27, 27),
    PRRR_NOS2 = setfd(PRRR, 26, 26),
    PRRR_NOS1 = setfd(PRRR, 25, 25),
    PRRR_NOS0 = setfd(PRRR, 24, 24),
    PRRR_NS1 = setfd(PRRR, 19, 19),
    PRRR_NS0 = setfd(PRRR, 18, 18),
    PRRR_DS1 = setfd(PRRR, 17, 17),
    PRRR_DS0 = setfd(PRRR, 16, 16),
    PRRR_TR7 = setfd(PRRR, 15, 14),
    PRRR_TR6 = setfd(PRRR, 13, 12),
    PRRR_TR5 = setfd(PRRR, 11, 10),
    PRRR_TR4 = setfd(PRRR, 9, 8),
    PRRR_TR3 = setfd(PRRR, 7, 6),
    PRRR_TR2 = setfd(PRRR, 5, 4),
    PRRR_TR1 = setfd(PRRR, 3, 2),
    PRRR_TR0 = setfd(PRRR, 1, 0),

    SCR_SIF = setfd(SCR, 9, 9),
    SCR_HCE = setfd(SCR, 8, 8),
    SCR_SCD = setfd(SCR, 7, 7),
    SCR_nET = setfd(SCR, 6, 6),
    SCR_AW = setfd(SCR, 5, 5),
    SCR_FW = setfd(SCR, 4, 4),
    SCR_EA = setfd(SCR, 3, 3),
    SCR_FIQ = setfd(SCR, 2, 2),
    SCR_IRQ = setfd(SCR, 1, 1),
    SCR_NS = setfd(SCR, 0, 0),

    SCTLR_TE = setfd(SCTLR, 30, 30),
    SCTLR_AFE = setfd(SCTLR, 29, 29),
    SCTLR_TRE = setfd(SCTLR, 28, 28),
    SCTLR_NMFI = setfd(SCTLR, 27, 27),
    SCTLR_EE = setfd(SCTLR, 25, 25),
    SCTLR_VE = setfd(SCTLR, 24, 24),
    SCTLR_U = setfd(SCTLR, 22, 22),
    SCTLR_FI = setfd(SCTLR, 21, 21),
    SCTLR_UWXN = setfd(SCTLR, 20, 20),
    SCTLR_WXN = setfd(SCTLR, 19, 19),
    SCTLR_HA = setfd(SCTLR, 17, 17),
    SCTLR_RR = setfd(SCTLR, 14, 14),
    SCTLR_V = setfd(SCTLR, 13, 13),
    SCTLR_I = setfd(SCTLR, 12, 12),
    SCTLR_Z = setfd(SCTLR, 11, 11),
    SCTLR_SW = setfd(SCTLR, 10, 10),
    SCTLR_B = setfd(SCTLR, 7, 7),
    SCTLR_CP15BEN = setfd(SCTLR, 5, 5),
    SCTLR_C = setfd(SCTLR, 2, 2),
    SCTLR_A = setfd(SCTLR, 1, 1),
    SCTLR_M = setfd(SCTLR, 0, 0),

    TTBCR_N = setfd(TTBCR, 2, 0),
    TTBCR_PD0 = setfd(TTBCR, 4, 4),
    TTBCR_PD1 = setfd(TTBCR, 5, 5),
    TTBCR_EAE = setfd(TTBCR, 31, 31),

    TTBCR_SH1 = setfd(TTBCR, 29, 28),
    TTBCR_ORGN1 = setfd(TTBCR, 27, 26),
    TTBCR_IRGN1 = setfd(TTBCR, 25, 24),
    TTBCR_EPD1 = setfd(TTBCR, 23, 23),
    TTBCR_A1 = setfd(TTBCR, 22, 22),
    TTBCR_T1SZ = setfd(TTBCR, 18, 16),
    TTBCR_SH0 = setfd(TTBCR, 13, 12),
    TTBCR_ORGN0 = setfd(TTBCR, 11, 10),
    TTBCR_IRGN0 = setfd(TTBCR, 9, 8),
    TTBCR_EPD0 = setfd(TTBCR, 7, 7),
    TTBCR_T0SZ = setfd(TTBCR, 2, 0),

    VTCR_SH0 = setfd(VTCR, 13, 12),
    VTCR_ORGN0 = setfd(VTCR, 11, 10),
    VTCR_IRGN0 = setfd(VTCR, 9, 8),
    VTCR_SL0 = setfd(VTCR, 7, 6),
    VTCR_S = setfd(VTCR, 4, 4),
    VTCR_T0SZ = setfd(VTCR, 3, 0)
};

class armv7a_cp15
{
    public:

        uint32_t regs[16][16][16][16];  // [crn][opc1][crm][opc2]
        uint64_t regs64[16][16];    // [crm][opc1]
#ifdef CPU_ASSERT
        bool valid[16][16][16][16];
        bool valid64[16][16];
#endif
        armv7a_cp15();
        ~armv7a_cp15();

        void reset_core_id(unsigned int core_id);
        void reset_a8();

        //read by name
        uint32_t read(armv7a_cp15_reg_names name);
        uint64_t read64(armv7a_cp15_reg_names64 name);

        //write by name
        void write(armv7a_cp15_reg_names name, uint32_t value);
        void write64(armv7a_cp15_reg_names64 name, uint64_t value);

        //read by op
        void read(uint32_t* data, uint32_t crn, uint32_t opc1, uint32_t crm, uint32_t opc2);
        void read64(uint32_t* data_h, uint32_t* data_l, uint32_t crm, uint32_t opc1);

        //write by op
        void write(uint32_t data, uint32_t crn, uint32_t opc1, uint32_t crm, uint32_t opc2);
        void write64(uint32_t data_h, uint32_t data_l, uint32_t crm, uint32_t opc1);


    private:
        unsigned int core_id;
};

#endif
