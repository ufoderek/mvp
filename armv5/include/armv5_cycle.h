#ifndef _CYCLE_H_
#define _CYCLE_H_

/* executing cycles for ARMV5 instructions */
/* general system parameters */
#define CYC_B_PENALTY   2   // branch
#define CYC_LPC_PENALTY 4   // load to PC
#define CYC_ICACHE_MISS 8
#define CYC_DCACHE_MISS 16
#define CYC_COPROCESSOR 1   // access coprecessor
#define CYC_UNDO        1   // condition not pass

/* general ARM instructions */
#define CYC_ARM_ADC     1
#define CYC_ARM_ADD     1
#define CYC_ARM_AND     1
#define CYC_ARM_B       1
#define CYC_ARM_BIC     1
#define CYC_ARM_BL      1
#define CYC_ARM_BLX     1
#define CYC_ARM_BLX2    1
#define CYC_ARM_BX      1
#define CYC_ARM_CDP     (1 + CYC_COPROCESSOR)
#define CYC_ARM_CLZ     1
#define CYC_ARM_CMN     1
#define CYC_ARM_CMP     1
#define CYC_ARM_EOR     1
#define CYC_ARM_LDC
#define CYC_ARM_LDM     count
#define CYC_ARM_LDR     1
#define CYC_ARM_LDRB    1
#define CYC_ARM_LDRBT   1
#define CYC_ARM_LDRH    1
#define CYC_ARM_LDRSB   1
#define CYC_ARM_LDRSH   1
#define CYC_ARM_MCR     (1 + CYC_COPROCESSOR)
#define CYC_ARM_MLA     2
#define CYC_ARM_MOV     1
#define CYC_ARM_MRC     (1 + CYC_COPROCESSOR)
#define CYC_ARM_MRS     2
#define CYC_ARM_MSR     1
#define CYC_ARM_MUL     2
#define CYC_ARM_MVN     1
#define CYC_ARM_ORR     1
#define CYC_ARM_RSB     1
#define CYC_ARM_RSC     1
#define CYC_ARM_SBC     1
#define CYC_ARM_SMLAL   3
#define CYC_ARM_SMULL   3
#define CYC_ARM_STC
#define CYC_ARM_STM     count
#define CYC_ARM_STR     1
#define CYC_ARM_STRB    1
#define CYC_ARM_STRH    1
#define CYC_ARM_SUB     1
#define CYC_ARM_SWI     1
#define CYC_ARM_SWP     2
#define CYC_ARM_SWPB    2
#define CYC_ARM_TEQ     1
#define CYC_ARM_TST     1
#define CYC_ARM_UMLAL   2
#define CYC_ARM_UMULL   2

/* DSP instructions */
#define CYC_DSP_LDRD    2
#define CYC_DSP_MCRR    (2 + CYC_COPROCESSOR)
#define CYC_DSP_MRRC    (2 + CYC_COPROCESSOR)
#define CYC_DSP_PLD     1
#define CYC_DSP_QADD    1
#define CYC_DSP_QDADD   1
#define CYC_DSP_QDSUB   1
#define CYC_DSP_QSUB    1
#define CYC_DSP_SMLA    1
#define CYC_DSP_SMLAL   2
#define CYC_DSP_SMLAW   1
#define CYC_DSP_SMUL    1
#define CYC_DSP_SMULW   1
#define CYC_DSP_STRD    2
#endif

