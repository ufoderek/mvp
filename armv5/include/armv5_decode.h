#ifndef _INST_H_
#define _INST_H_

/* define the instruction condition */
#define INST_COND_EQ    0x0000
#define INST_COND_NE    0x0001
#define INST_COND_CS    0x0002
#define INST_COND_CC    0x0003
#define INST_COND_MI    0x0004
#define INST_COND_PL    0x0005
#define INST_COND_VS    0x0006
#define INST_COND_VC    0x0007
#define INST_COND_HI    0x0008
#define INST_COND_LS    0x0009
#define INST_COND_GE    0x000a
#define INST_COND_LT    0x000b
#define INST_COND_GT    0x000c
#define INST_COND_LE    0x000d
#define INST_COND_AL    0x000e
#define INST_COND_NV    0x000f

/* define OP code for data processing instrucitons with immediate mode */
#define INST_OP_ADC     0x0005
#define INST_OP_ADD     0x0004
#define INST_OP_AND     0x0000
#define INST_OP_BIC     0x000e
#define INST_OP_CMN     0x000b
#define INST_OP_CMP     0x000a
#define INST_OP_EOR     0x0001
#define INST_OP_MOV     0x000d
#define INST_OP_MSR     0x0009
#define INST_OP_MVN     0x000f
#define INST_OP_ORR     0x000c
#define INST_OP_RSB     0x0003
#define INST_OP_RSC     0x0007
#define INST_OP_SBC     0x0006
#define INST_OP_SUB     0x0002
#define INST_OP_TEQ     0x0009
#define INST_OP_TST     0x0008

/* define OP code for data processing instrucitons without immediate mode */
#define INST_OP_BKPT    0x01200070
#define INST_OP_BLX2    0x01200030
#define INST_OP_BX      0x01200010
#define INST_OP_CLZ     0x01600010
#define INST_OP_LDRD    0x000000d0
#define INST_OP_LDREX   0x01900090
#define INST_OP_LDRH    0x001000b0
#define INST_OP_LDRSB   0x001000d0
#define INST_OP_LDRSH   0x001000f0
#define INST_OP_MLA     0x00200090
#define INST_OP_MLAS    0x00300090
#define INST_OP_MUL     0x00000090
#define INST_OP_MULS    0x00100090
#define INST_OP_SMLAL   0x00e00090
#define INST_OP_SMLALS  0x00f00090
#define INST_OP_SMULL   0x00c00090
#define INST_OP_SMULLS  0x00d00090
#define INST_OP_STRD    0x000000f0
#define INST_OP_STREX   0x01800090
#define INST_OP_STRH    0x000000b0
#define INST_OP_SWP     0x01000090
#define INST_OP_SWPB    0x01400090
#define INST_OP_UMLAL   0x00a00090
#define INST_OP_UMLALS  0x00b00090
#define INST_OP_UMULL   0x00800090
#define INST_OP_UMULLS  0x00900090

//#if (defined(CPU_ARM1176) || defined(CPU_ARM11MP))
/* define OP code for instruction mode 3 */
#define INST_OP_SXTB    0x000a
#define INST_OP_SXTB16  0x0008
#define INST_OP_SXTH    0x000b
#define INST_OP_UXTB    0x000e
#define INST_OP_UXTB16  0x000c
#define INST_OP_UXTH    0x000f
//#endif

#endif
