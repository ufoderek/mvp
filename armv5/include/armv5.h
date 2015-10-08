#ifndef _ARMV5_H_
#define _ARMV5_H_

#include <stdint.h>

#include <armv5_defs.h>
#include <armv5_decode.h>
#include <armv5_cycle.h>
#include <ahb_master_if.h>
#include <exclusive_monitor.h>
#include <debug_utils.h>
#include <gdbstub.h>
#include <like.h>

using namespace std;

class GDBStub;
class ARMV5: public sc_module, public ahb_master_if
{
    protected:
        unsigned int core_id;

        /* basic */
    public:
        sc_in_clk clk;      // clock signal
        sc_in<bool> irq_n;  // negative edge trigger IRQ signal

        SC_HAS_PROCESS(ARMV5);
        ARMV5(sc_module_name name, uint32_t addr, bool using_gdb, unsigned int gdb_port);
        ~ARMV5();
        /* internal debugging */
    protected:
        char current_inst_name[16];
        void print_inst(const char* inst_name);
        void inst_assert_set(const unsigned int i);
        void inst_assert_unset(const unsigned int i);
        void inst_assert(const uint32_t pattern, unsigned int u, unsigned int l);

        /* decoding */
    protected:
        bool arm_inst_mode0();
        bool arm_inst_mode1();
        bool arm_inst_mode2();
        bool arm_inst_mode3();
        bool arm_inst_mode4();
        bool arm_inst_mode5();
        bool arm_inst_mode6();
        bool arm_inst_mode7();
        //void addr_mode2_imm();
        //void addr_mode2_scaled_reg();
        void addr_mode3_imm();
        void addr_mode3_reg();
        //void addr_mode4();

        /* exception handler */
    protected:
        void arm_except_dabort(uint8_t, uint8_t);
        void arm_except_dabort();
        void arm_except_fabort(uint8_t, uint8_t);
        void arm_except_fabort();
        void arm_except_fiq();
        void arm_except_irq();
        void arm_except_swi();
        void arm_except_undef();

        /* register file */
    protected:
        regFile rf;
        void rfInit(uint32_t);
        void rfRead(uint32_t*, uint16_t, uint8_t mode);
        void rfWrite(uint32_t, uint16_t, uint8_t mode);

        /* coprocessor */
    protected:
        CP cp15;
        void c1_sys_decode(uint32_t n);
        void cpInit();
        bool check_be();
        bool check_ua(uint32_t* addr, uint8_t nbytes);
    public:
        bool cpRead(uint32_t* data, uint16_t cn, uint16_t cm, uint8_t op);
        bool cpWrite(uint32_t data, uint16_t cn, uint16_t cm, uint8_t op);

        /* cache */
    protected:
        bool icache_enable, dcache_enable;
        CACHE icache[CACHE_WAY][CACHE_SET], dcache[CACHE_WAY][CACHE_SET];
        void cacheInit();
        bool dcache_read(uint32_t);
        bool dcache_write(uint32_t);
        void dcache_update(uint32_t);
        bool icache_read(uint32_t);
        void icache_update(uint32_t);

        /* bus */
    protected:
        unsigned int get_word_size(word_type wtype);
        bool check_alignment(uint32_t addr, word_type wtype);
        virtual bool bus_read(uint32_t* data, uint32_t addr, unsigned int length);
        virtual bool bus_write(uint32_t data, uint32_t addr, unsigned int length);
        bool data_read(uint32_t*, uint32_t, uint8_t mode, word_type wtype);
        bool data_write(uint32_t, uint32_t, uint8_t mode, word_type wtype);

        /* mmu */
    protected:
        bool mmu_enable;
        bool permission(uint8_t, uint8_t, bool);
        virtual int vir2phy(uint32_t, uint32_t*, uint8_t, bool, bool*);

        /* instruction execution */
    protected:
        uint8_t op1;
        uint8_t op2;
        uint32_t inst;
        uint32_t operand_1, operand_2;
        uint32_t imm;
        uint8_t shift;
        uint8_t rd, rn, rm, rs;
        uint32_t addr;
        bool shift_carry_out;
        bool update;

        /* other */
    protected:
        uint8_t delay;
        bool vectorHi;
        bool fetchARM();
        bool exeARM();
        bool inst_arm_read();
        void run();

        /* exclusive monitors */
    protected:
        static exclusive_monitor global_monitor;
        bool is_exclusive_local;
        bool is_exclusive_global;
        bool non_cachable;
        exclusive_monitor local_monitor;

        /* ARMV6K events */
    protected:
        // shared event between armv6k cores
        static sc_event shared_event;

        sc_event local_event;
        bool event_reg;

        // listening on shared_event
        void shared_event_listener();
        // listening on irq_n
        void irq_n_listener();

        // called by listeners above
        void wakup_events();

        void send_event();
        void wait_for_event();
        void clear_event_reg();
        bool event_registered();

        /* gdb */
    public:
        bool using_gdb;
        bool using_prog;
        bool gdb_init;
        unsigned int gdb_port;
        GDBStub* gdb;
        void regRead(uint32_t*, uint16_t);
        void regWrite(uint32_t, uint16_t);
        void memRead(uint32_t*, uint32_t, int);
        void memWrite(uint32_t, uint32_t, int);
        void bkptInsert(uint32_t);
        void bkptRemove(uint32_t);
        void bkptRemove();
        void awatchInsert(uint32_t);
        void rwatchInsert(uint32_t);
        void wwatchInsert(uint32_t);
        void awatchRemove(uint32_t);
        void rwatchRemove(uint32_t);
        void wwatchRemove(uint32_t);
        void watchRemove();
    protected:
        BKPT break_point[SIZE_BKPT];
        BKPT rwatch_point[SIZE_BKPT];
        BKPT wwatch_point[SIZE_BKPT];
        bool bkptCheck(uint32_t);
        bool rwatchCheck(uint32_t);
        bool wwatchCheck(uint32_t);

        /* arm instructions */
    protected:
        void arm_adc();
        void arm_add();
        void arm_and();
        void arm_bic();
        void arm_bkpt();
        void arm_blx();
        void arm_blx2();
        void arm_bx();
        void arm_bxj();
        void arm_cdp();
        void arm_clrex();
        void arm_clz();
        void arm_cps();
        void arm_cpy();
        void arm_cmn();
        void arm_cmp();
        void arm_eor();
        //void arm_ldc();

        // load multiple
        void arm_ldm();
        /*
        void arm_ldm1(uint32_t start_addr, uint32_t end_addr);
        void arm_ldm2(uint32_t start_addr, uint32_t end_addr);
        void arm_ldm3(uint32_t start_addr, uint32_t end_addr);
        */

        // load
        void arm_ldr();
        void arm_ldrb();


        // extra load
        void arm_ldrd(uint32_t addr, uint32_t rn_val_old);
        void arm_ldrsb(uint32_t addr, uint32_t rn_val_old);
        void arm_ldrsh(uint32_t addr, uint32_t rn_val_old);
        void arm_ldrh(uint32_t addr, uint32_t rn_val_old);

        // load exclusive
        void arm_ldrex();
        void arm_ldrexb();

        void arm_mcr();
        void arm_mcrr();
        void arm_mla();
        void arm_mov();
        void arm_mrc();
        void arm_mrs();
        void arm_msr();
        void arm_mul();
        void arm_mvn();
        void arm_orr();

        void arm_pkhbt();
        void arm_pkhtb();

        void arm_qadd();
        void arm_qsub();
        void arm_qdadd();
        void arm_qdsub();

        void arm_rev();
        void arm_revsh();
        void arm_rfe();
        void arm_rsb();
        void arm_rsc();

        void arm_sbc();
        void arm_sel();
        void arm_setend();
        void arm_sev();
        void arm_smla();
        void arm_smlal();
        void arm_smlalxy();
        void arm_smlaw();

        void arm_smul();
        void arm_smull();
        void arm_smulw();
        void arm_smulx();

        void arm_srs();
        //void arm_stc();

        // store multiple
        void arm_stm();
        /*
        void arm_stm1(uint32_t start_addr, uint32_t end_addr);
        void arm_stm2(uint32_t start_addr, uint32_t end_addr);
        */

        // store
        void arm_str();
        void arm_strb();

        // extra store
        void arm_strd(uint32_t addr, uint32_t rn_val_old);
        void arm_strh(uint32_t addr, uint32_t rn_val_old);

        // store exclusive
        void arm_strex();
        void arm_strexb();

        void arm_sxtab();
        void arm_sxtab16();
        void arm_sxtah();

        void arm_sub();
        void arm_swp();
        void arm_swpb();

        void arm_teq();
        void arm_tst();

        void arm_umaal();
        void arm_umlal();
        void arm_umull();

        void arm_uxtab();
        void arm_uxtab16();
        void arm_uxtah();

        void arm_wfe();
        void arm_wfi();

        /* executing ARMv5 DSP extension instructions private methods */

        //void dsp_mcrr();
        //void dsp_mrrc();
        void dsp_pld();

        //void dsp_qdadd();

        //void dsp_qsub();
        //void dsp_smla();
        //void dsp_smlal();
        //void dsp_smlaw();
        //void dsp_smul();
        //void dsp_smulw();

};

#endif
