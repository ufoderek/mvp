#ifndef ARMV5_DEFS_H_
#define ARMV5_DEFS_H_

#include <debug_utils.h>
#include <bit_opt.h>

/* macro of 32-bits data rotate left operation */
#define ROTATEL(x, ro) \
    (x) = ((x) << (ro)) | ((x) >> (32 - (ro)))

/* macro of 32-bits data rotate right operation */
#define ROTATER(x, ro) \
    (x) = ((x) >> (ro)) | ((x) << (32 - (ro)))

/* define CPU modes */
#define MODE_USR    0x0010  // mode 0
#define MODE_SYS    0x001f  // mode 0
#define MODE_SVC    0x0013  // mode 1
#define MODE_ABORT  0x0017  // mode 2
#define MODE_UNDEF  0x001b  // mode 3
#define MODE_IRQ    0x0012  // mode 4
#define MODE_FIQ    0x0011  // mode 5

/* macros for CPSR operation */
#define CPSR_MODE(x)        (x & 0x001f)
#define CPSR_T(x)           (((x) >> 5) & 0x0001)
#define CPSR_F(x)           (((x) >> 6) & 0x0001)
#define CPSR_I(x)           (((x) >> 7) & 0x0001)
#define CPSR_A(x)           (((x) >> 8) & 0x0001)
#define CPSR_E(x)           (((x) >> 9) & 0x0001)
#define CPSR_J(x)           (((x) >> 24) & 0x0001)
#define CPSR_Q(x)           (((x) >> 27) & 0x0001)
#define CPSR_V(x)           (((x) >> 28) & 0x0001)
#define CPSR_C(x)           (((x) >> 29) & 0x0001)
#define CPSR_Z(x)           (((x) >> 30) & 0x0001)
#define CPSR_N(x)           (((x) >> 31) & 0x0001)
#define CPSR_GE(x)          (((x) >> 16) & 0x000f)
#define CPSR_SET_T(x)       ((x) |= (0x0001 << 5))
#define CPSR_SET_F(x)       ((x) |= (0x0001 << 6))
#define CPSR_SET_I(x)       ((x) |= (0x0001 << 7))
#define CPSR_SET_A(x)       ((x) |= (0x0001 << 8))
#define CPSR_SET_E(x)       ((x) |= (0x0001 << 9))
#define CPSR_SET_J(x)       ((x) |= (0x0001 << 24))
#define CPSR_SET_Q(x)       ((x) |= (0x0001 << 27))
#define CPSR_SET_V(x)       ((x) |= (0x0001 << 28))
#define CPSR_SET_C(x)       ((x) |= (0x0001 << 29))
#define CPSR_SET_Z(x)       ((x) |= (0x0001 << 30))
#define CPSR_SET_N(x)       ((x) |= (0x0001 << 31))
#define CPSR_CLEAR_T(x)     ((x) &= ~(0x0001 << 5))
#define CPSR_CLEAR_F(x)     ((x) &= ~(0x0001 << 6))
#define CPSR_CLEAR_I(x)     ((x) &= ~(0x0001 << 7))
#define CPSR_CLEAR_A(x)     ((x) &= ~(0x0001 << 8))
#define CPSR_CLEAR_E(x)     ((x) &= ~(0x0001 << 9))
#define CPSR_CLEAR_J(x)     ((x) &= ~(0x0001 << 24))
#define CPSR_CLEAR_Q(x)     ((x) &= ~(0x0001 << 27))
#define CPSR_CLEAR_V(x)     ((x) &= ~(0x0001 << 28))
#define CPSR_CLEAR_C(x)     ((x) &= ~(0x0001 << 29))
#define CPSR_CLEAR_Z(x)     ((x) &= ~(0x0001 << 30))
#define CPSR_CLEAR_N(x)     ((x) &= ~(0x0001 << 31))

/* define register symbols */
#define R00     0
#define R01     1
#define R02     2
#define R03     3
#define R04     4
#define R05     5
#define R06     6
#define R07     7
#define R08     8
#define R09     9
#define R10     10
#define R11     11
#define R12     12
#define SP      13
#define LR      14
#define PC      15
#define CPSR    16
#define SPSR    17
#define FPS     18
#define PID     20
#define SYS     21
#define TTBR    22
#define DOM     23
//#define DFSR  24
//#define   IFSR    25
//#define FAR       26
#define INST    24
#define CYCLE   25

/* the base address of exception vectors */
#define VECTOR_RESET    (vectorHi)? 0xffff0000 : 0x00000000
#define VECTOR_UNDEF    (vectorHi)? 0xffff0004 : 0x00000004
#define VECTOR_SWI      (vectorHi)? 0xffff0008 : 0x00000008
#define VECTOR_PABOR    (vectorHi)? 0xffff000c : 0x0000000c
#define VECTOR_DABOR    (vectorHi)? 0xffff0010 : 0x00000010
#define VECTOR_IRQ      (vectorHi)? 0xffff0018 : 0x00000018
#define VECTOR_FIQ      (vectorHi)? 0xffff001c : 0x0000001c

/* total entries of break & watch points */
#define SIZE_BKPT   32

/* memeory fault definition */
#define FAULT_NONE          0x00
#define FAULT_ALIGN         0x01
#define FAULT_EXTFIRST      0x0c
#define FAULT_EXTSECOND     0x0e
#define FAULT_SECTTRANS     0x05
#define FAULT_PAGETRANS     0x07
#define FAULT_SECTDOMAIN    0x09
#define FAULT_PAGEDOMAIN    0x0b
#define FAULT_SECTPERMI     0x0d
#define FAULT_PAGEPERMI     0x0f
#define FAULT_EXTERNAL      0x08

/* cache structure definition */
#define CACHE_WAY   4
#define CACHE_SET   256
#define CACHE_LINE  8
#define CACHE_SIZE  (CACHE_WAY * CACHE_SET * CACHE_LINE * 4)

/* register files for ARM processor */
typedef struct regFile {
    /* general purpose registers */
    uint32_t regs[8];       // $r0 ~ $r7

    /* banked general purpose registers */
    uint32_t reg_usr[5];    // $r8 ~ $r12
    uint32_t reg_fiq[5];    // $r8 ~ $r12
    uint32_t sp[6];
    uint32_t lr[6];

    /* program counter */
    uint32_t pc;

    /* status registers */
    uint32_t cpsr;
    uint32_t spsr[6];   // actually, spsr[0] doesn't exist
} regFile;

/* system control coprocessor of ARM processor */
class CP
{
    public:
        /* register c0 */
        uint32_t c0_idcode;
        uint32_t c0_cachetype;
        uint32_t c0_tcmstatus;
        uint32_t c0_mpu_type;
        uint32_t c0_cpuid;
        uint32_t c0_mmfr0;
        uint32_t c0_mmfr1;
        uint32_t c0_mmfr2;
        uint32_t c0_mmfr3;

        uint8_t version() {
            uint8_t ver = SMM(c0_idcode, 19, 16);

            switch (ver) {
                case 1:
                case 2:
                    return 4;
                case 3:
                case 4:
                case 5:
                case 6:
                    return 5;
                case 7:
                    return 6;
                case 0xF:
                    return 0xF;
                default:
                    printb(d_armv5, "ARM ISA version error");
            }
        }

        /* register c1 */
        uint32_t c1_sys;        // system control register
        uint32_t c1_aux;
        bool c1_mmu() {
            return c1_sys & M(0);
        }
        bool c1_a() {
            return c1_sys & M(1);
        }
        bool c1_dcache() {
            return c1_sys & M(2);
        }
        bool c1_wbuffer() {
            return c1_sys & M(3);
        }
        bool c1_b() {
            return c1_sys & M(7);
        }
        bool c1_s() {
            return c1_sys & M(8);
        }
        bool c1_r() {
            return c1_sys & M(9);
        }
        bool c1_f() {
            return c1_sys & M(10);
        }
        bool c1_u() {
            return c1_sys & M(22);
        }
        bool c1_xp() {
            return c1_sys & M(23);
        }
        bool c1_ee() {
            return c1_sys & M(25);
        }
        bool c1_nmfi() {
            return c1_sys & M(27);
        }
        bool c1_tex_remap() {
            return c1_sys & M(28);
        }
        bool c1_force_ap() {
            return c1_sys & M(29);
        }

        /* register c2 */
        uint32_t c2_ttbr;       // MMU translation table base
        uint32_t c2_ttbr1;
        uint32_t c2_ttbcr;

        /* register c3 */
        uint32_t c3_domain[16]; // MMU domain access control register & MPU write buffer control

        /* register c5 */
        uint32_t c5_dfsr;       // data fault status register
        uint32_t c5_ifsr;       // instruction fault status register

        /* register c6 */
        uint32_t c6_far;        // fault address register

        /* register c7 */

        /* register c8 */

        /* register c9 */
        uint32_t c9_cache;      // cache lockdown register
        uint32_t c9_tcm;        // TCM region register

        /* register c10*/
        uint32_t c10_tlb;       // TLB lockdown register

        /* register c13 */
        uint32_t c13_fcse;      // FCSE PID
        uint32_t c13_context;   // context ID
        uint32_t c13_context_id1;
        uint32_t c13_context_id2;
        uint32_t c13_context_id3;
};

/* structure for record break & watch points */
typedef struct BKPT {
    bool exist;
    uint32_t addr;
} BKPT;

/* the cache structure */
typedef struct CACHE {
    bool valid;
    uint32_t tag;
    //  uint32_t data[CACHE_LINE];
} CACHE;

enum word_type {
    type_byte,
    type_bsync,
    type_hword,
    type_wload,
    type_wstore,
    type_wsync,
    type_mword,
    type_dword
};

#endif

