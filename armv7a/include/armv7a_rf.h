#ifndef _ARMV7A_RF_H_
#define _ARMV7A_RF_H_

#include <bits.h>
#include <stdint.h>

enum r_name
{
    RName_0usr = 0,
    RName_1usr,
    RName_2usr,
    RName_3usr,
    RName_4usr,
    RName_5usr,
    RName_6usr,
    RName_7usr,
    RName_8usr,
    RName_9usr,
    RName_10usr,
    RName_11usr,
    RName_12usr,
    RName_SPusr,
    RName_LRusr,
    RName_PC,

    RName_SPsvc,
    RName_LRsvc,

    RName_SPmon,
    RName_LRmon,

    RName_SPabt,
    RName_LRabt,

    RName_SPund,
    RName_LRund,

    RName_SPirq,
    RName_LRirq,

    RName_SPhyp,
    RName_ELRhyp,

    RName_8fiq,
    RName_9fiq,
    RName_10fiq,
    RName_11fiq,
    RName_12fiq,
    RName_SPfiq,
    RName_LRfiq,

    RName_count
};

enum _extra_reg_names
{
    SP = 13,
    LR = 14,
    PC = 15
};

enum r_bank
{
    usr = 0, fiq, irq, svc, mon, abt, hyp, und, r_bank_count
};

enum inst_set
{
    InstSet_ARM, InstSet_Thumb, InstSet_Jazelle, InstSet_ThumbEE
};

class armv7a_rf
{
    public:
        armv7a_rf();
        ~armv7a_rf();

        bool bad_mode(uint32_t mode);
        bool current_mode_is_hyp();
        bool current_mode_is_not_user();
        bool current_mode_is_user_or_system();
        r_bank r_bank_select(uint32_t mode);
        r_name look_up_r_name(uint32_t n, uint32_t mode);

        uint32_t r_mode_read(uint32_t n, uint32_t mode);
        void r_mode_write(uint32_t n, uint32_t mode, uint32_t value);

        uint32_t r_read(uint32_t n);
        void r_write(uint32_t n, uint32_t value);

        void reset_pc(uint32_t addr);
        void incre_pc();
        uint32_t pc();
        uint32_t current_pc();

        /* Pseudo code of ARM Ref Manual v7 */
        void alu_write_pc(uint32_t addr);
        void branch_to(uint32_t addr);
        void branch_write_pc(uint32_t address);
        void bx_write_pc(uint32_t addr);
        bool condition_passed(uint32_t cond);
        inst_set current_inst_set();
        void load_write_pc(uint32_t address);
        uint32_t pc_store_value();

        uint32_t apsr();

        uint32_t cpsr();
        uint32_t cpsr_M();
        uint32_t cpsr_T();
        uint32_t cpsr_F();
        uint32_t cpsr_I();
        uint32_t cpsr_A();
        uint32_t cpsr_E();
        uint32_t cpsr_IT();
        uint32_t cpsr_GE();
        uint32_t cpsr_J();
        uint32_t cpsr_Q();
        uint32_t cpsr_V();
        uint32_t cpsr_C();
        uint32_t cpsr_Z();
        uint32_t cpsr_N();

        void cpsr(uint32_t val);
        void cpsr_M(uint32_t mode);
        void cpsr_T(bool b);
        void cpsr_F(bool b);
        void cpsr_I(bool b);
        void cpsr_A(bool b);
        void cpsr_E(bool b);
        void cpsr_IT(uint32_t it);
        void cpsr_GE(uint32_t ge);
        void cpsr_J(bool b);
        void cpsr_Q(bool b);
        void cpsr_V(bool b);
        void cpsr_C(bool b);
        void cpsr_Z(bool b);
        void cpsr_N(bool b);

        uint32_t spsr();
        void spsr(uint32_t value);

        uint32_t spsr_bank(r_bank b);
        void spsr_bank(r_bank b, uint32_t value);

        uint32_t elr_hyp();
        void elr_hyp(uint32_t value);

        uint32_t _r[RName_count];

        uint32_t _cpsr;
        uint32_t _spsr[r_bank_count];
};

#endif
