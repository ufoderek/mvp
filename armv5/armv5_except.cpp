#include<armv5.h>

void ARMV5::arm_except_dabort(void)
{
    rf.lr[2] = rf.pc + 4;   // restore PC
    rf.spsr[2] = rf.cpsr;   // restore CPSR
    rf.cpsr &= 0xffffffe0;  // set to ABORT mode
    rf.cpsr |= MODE_ABORT;
    CPSR_CLEAR_T(rf.cpsr);  // run in 32-bits ARM mode
    CPSR_SET_I(rf.cpsr);    // disable IRQ
    rf.pc = VECTOR_DABOR;
    delay += CYC_B_PENALTY;
}

void ARMV5::arm_except_fabort(void)
{
    rf.lr[2] = rf.pc;       // restore PC
    rf.spsr[2] = rf.cpsr;   // restore CPSR
    rf.cpsr &= 0xffffffe0;  // set to ABORT mode
    rf.cpsr |= MODE_ABORT;
    CPSR_CLEAR_T(rf.cpsr);  // run in 32-bits ARM mode
    CPSR_SET_I(rf.cpsr);    // disable IRQ
    rf.pc = VECTOR_PABOR;
    delay += CYC_B_PENALTY;
}

void ARMV5::arm_except_fiq(void)
{
    rf.lr[5] = rf.pc + 4;   // restore PC
    rf.spsr[5] = rf.cpsr;   // restore CPSR
    rf.cpsr &= 0xffffffe0;  // set to FIQ mode
    rf.cpsr |= MODE_FIQ;
    CPSR_CLEAR_T(rf.cpsr);  // run in 32-bits ARM mode
    CPSR_SET_F(rf.cpsr);    // disable FIQ
    CPSR_SET_I(rf.cpsr);    // disable IRQ
    rf.pc = VECTOR_FIQ;
    delay += CYC_B_PENALTY;
}

void ARMV5::arm_except_irq(void)
{
    rf.lr[4] = rf.pc + 4;   // restore PC
    rf.spsr[4] = rf.cpsr;   // restore CPSR
    rf.cpsr &= 0xffffffe0;  // set to IRQ mode
    rf.cpsr |= MODE_IRQ;
    CPSR_CLEAR_T(rf.cpsr);  // run in 32-bits ARM mode
    CPSR_SET_I(rf.cpsr);    // disable IRQ
    rf.pc = VECTOR_IRQ;
    delay += CYC_B_PENALTY;
}

void ARMV5::arm_except_swi(void)
{
    /*
    if(using_prog)
        semihost_swi(this, inst & 0x00ffffff);
    else
    */
    {
        //printf("SWI@0x%.8x\n", rf.pc - 4);
        rf.lr[1] = rf.pc;       // restore PC
        rf.spsr[1] = rf.cpsr;   // restore CPSR
        rf.cpsr &= 0xffffffe0;  // set to SVC mode
        rf.cpsr |= MODE_SVC;
        CPSR_CLEAR_T(rf.cpsr);  // run in 32-bits ARM mode
        CPSR_SET_I(rf.cpsr);    // disable IRQ
        rf.pc = VECTOR_SWI;
        delay += CYC_B_PENALTY;
    }
}

void ARMV5::arm_except_undef(void)
{
    //printf("instruction undefined@0x%.8x\n", rf.pc - 4);
    rf.lr[3] = rf.pc;       // restore PC
    rf.spsr[3] = rf.cpsr;   // restore CPSR
    rf.cpsr &= 0xffffffe0;  // set to UNDEF mode
    rf.cpsr |= MODE_UNDEF;
    CPSR_CLEAR_T(rf.cpsr);  // run in 32-bits ARM mode
    CPSR_SET_I(rf.cpsr);    // disable IRQ
    rf.pc = VECTOR_UNDEF;
    delay += CYC_B_PENALTY;
}

