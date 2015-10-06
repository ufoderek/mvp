#ifndef _ARMV7A_H_
#define _ARMV7A_H_

#include <systemc.h>
#include <ahb_master_if.h>
#include <armv7a_rf.h>
#include <armv7a_ir.h>
#include <armv7a_cp15.h>
#include <armv7a_mmu.h>
#include <bits.h>
#include <gdbstub.h>
#include <cpu_monitor.h>
#include <vector>
#include <map>

using namespace std;

enum sr_type
{
    SRType_ASR = 2,
    SRType_LSL = 0,
    SRType_LSR = 1,
    SRType_ROR = 3,
    SRType_RRX
};

enum fault_type
{
    AccessFlag = 0,
    Alignment,
    Background,
    Domain,
    Permission,
    Translation,
    SyncExternal,
    SyncExternalonWalk,
    SyncParity,
    SyncParityonWalk,
    AsyncParity,
    AsyncExternal,
    DebugEvent,
    TLBConflict,
    Lockdown,
    Coproc,
    ICacheMaint
};

//used by gdbstub
const int SIZE_BKPT = 32;
class BKPT
{
    public:
        bool exist;
        uint32_t addr;
};

class GDBStub;
class armv7a: public sc_module, public ahb_master_if
{
    public:
        sc_in_clk clk;
        sc_in<bool>irq_n;
        sc_in<bool>virq_n;

        SC_HAS_PROCESS(armv7a);
        armv7a(sc_module_name name);
        armv7a(sc_module_name name, const uint32_t& pc, uint32_t core_id,
               const bool using_gdb, const unsigned int gdb_port);
        ~armv7a();

        virtual bool inst_fetch(armv7a_ir& inst);
        virtual void execute();
        virtual void dump_rf();
        virtual void monitor_init();
        virtual void monitor();

    protected:
        //cpu_monitor* monitors[5];
        cpu_monitor* cpu_monitors_hyp;
        vector<cpu_monitor*> cpu_monitors_vm;
        vector<cpu_monitor*> cpu_monitors_proc;
        map<uint32_t, uint32_t> proc_map;

    protected:
        armv7a_ir inst;
        armv7a_rf rf;           //register file
        armv7a_cp15 cp15;       //coprocessor 15
        unsigned int core_id;   //cpu id
        bool wfi;               //indicating wait for interrupt

    protected:          //bus interface
        virtual bool bus_read(uint32_t* data, uint32_t addr, uint32_t length);
        virtual bool bus_read64(uint64_t* data, uint64_t addr, uint32_t length);
        virtual bool bus_write(uint32_t data, uint32_t addr, uint32_t length);
        virtual bool bus_write64(uint64_t data, uint64_t addr, uint32_t length);

    public:             //arch
        static uint32_t arch_version();
        static bool have_lpae();
        static bool have_mp_ext();
        static bool have_security_ext();
        static bool have_virt_ext();
        static bool is_secure();
        static mem_arch memory_system_architecture();
        static bool remap_regs_have_reset_values();
        static bool tlb_lookup_came_from_cache_maintenance();
        static bool unaligned_support();

    protected:          //misc pseudo
        virtual void arm_expand_imm(bits* imm32, const bits& imm12);
        virtual void arm_expand_imm_c(bits* imm32, uint32_t* carry_out, const bits& imm12, uint32_t carry_in);
        virtual void add_with_carry(bits* result, uint32_t* carry_out, uint32_t* overflow,
                                    const bits& x, const bits& y, uint32_t carry_in);
        virtual uint32_t align(uint32_t x, uint32_t y);
        //virtual void asr();
        virtual void asr_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift);
        virtual uint32_t bit_count(bits& value);
        virtual uint32_t count_leading_zero_bits(const bits& x);
        virtual void decode_imm_shift(sr_type* shift_t, uint32_t* shift_n, uint32_t type, uint32_t imm5);
        virtual sr_type decode_reg_shift(uint32_t type);
        virtual bool is_zero(uint32_t x);
        virtual bool is_zero64(uint64_t x);
        virtual bool is_zero(const bits& x);
        virtual uint32_t is_zero_bit(const bits& x);
        virtual void it_advance();
        virtual uint32_t lowest_set_bit(const bits& x);
        uint32_t ls_instruction_syndrome();
        virtual void lsl(bits* result, const bits& x, uint32_t shift);
        virtual void lsl_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift);
        virtual void lsr(bits* result, const bits& x, uint32_t shift);
        virtual void lsr_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift);
        virtual void null_check_if_thumbee(uint32_t n);
        virtual uint32_t processor_id();
        virtual void ror(bits* result, const bits& x, uint32_t shift);
        virtual void ror_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift);
        virtual void rrx_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t carry_in);
        virtual void shift(bits* result, const bits& value, sr_type type, uint32_t amount,
                           uint32_t carry_in);
        virtual void shift_c(bits* result, uint32_t* carry_out, const bits& value,
                             sr_type type, uint32_t amount, uint32_t carry_in);
        virtual void sign_extend(bits* result, const bits& x, uint32_t i);
        virtual uint32_t sign_extend32(const bits& x);
        virtual uint64_t sign_extend64(const bits& x);
        virtual uint64_t SInt(const bits& x);
        virtual uint64_t UInt(const bits& x);
        virtual void zero_extend(bits* result, const bits& x, uint32_t i);

    protected:          //needed by arm_mrs_bk and arm_msr_bk (imple. in armv7a_bank.cpp)
        virtual void banked_register_access_valid(uint32_t sysm, uint32_t mode);
        virtual void spsr_access_valid(uint32_t sysm, uint32_t mode);

    protected:          //imple. of system level pseudo code
        virtual void cpsr_write_by_inst(uint32_t value, uint32_t bytemask, bool is_excpt_return);
        virtual void spsr_write_by_inst(uint32_t value, uint32_t bytemask);

    protected:          //imple. of exclusive monitor pseudo code
        virtual void clear_exclusive_local(uint32_t processor_id);
        virtual bool exclusive_monitor_pass(uint32_t address, uint32_t size);
        virtual void set_exclusive_monitor(uint32_t address, uint32_t size);
        virtual void clear_exclusive_by_address(const full_address& p_address,
                                                uint32_t processor_id, uint32_t size);

    protected:          //imple. of CMSA pseudo code
        virtual void _mem_read(bits* value, address_descriptor& mem_addr_desc, uint32_t size);
        virtual void _mem_read64(bits64* value, address_descriptor& mem_addr_desc, uint32_t size);
        virtual void _mem_write(address_descriptor& mem_addr_desc, uint32_t size, bits& value);
        virtual void _mem_write64(address_descriptor& mem_addr_desc, uint32_t size, bits64& value);

        virtual void alignment_fault(uint32_t address, bool is_write);

        virtual uint32_t big_endian_reverse(uint32_t value, uint32_t n);

        virtual bool check_permission(permissions perms, uint32_t mva, uint32_t level,
                                      uint32_t domain, bool is_write, bool is_priv,
                                      bool take_to_hyp_mode, bool ldfsr_format,
                                      bool inst_fetch, bool gdb_access);

        virtual void data_abort(uint32_t v_address, uint64_t ip_address, uint32_t domain,
                                uint32_t level, bool is_write, fault_type type,
                                bool take_to_hyp_mode, bool second_stage_abort,
                                bool ipa_valid, bool ldfsr_format, bool s2fs1_walk);

        virtual void default_tex_decode(memory_attributes* result, uint32_t texcb, uint32_t s);

        virtual uint32_t encode_sd_fsr(fault_type type, uint32_t level);
        virtual uint32_t encode_ld_fsr(fault_type type, uint32_t level);

        //aligned reads
        virtual bool mem_a_read(bits* value, uint32_t address, uint32_t size);
        virtual bool mem_a_with_priv_read(bits* value, uint32_t address,
                                          uint32_t size, bool privileged);

        //aligned writes
        virtual bool mem_a_write(uint32_t address, uint32_t size, const bits& value);
        virtual bool mem_a_with_priv_write(uint32_t address, uint32_t size, bool privileged, const bits& value);

        //unaligned reads
        virtual bool mem_u_read(bits* value, uint32_t address, uint32_t size);
        virtual bool mem_u_with_priv_read(bits* value, uint32_t address, uint32_t size, bool privileged);

        //unaligned writes
        virtual bool mem_u_unpriv_write(uint32_t address, uint32_t size, const bits& value);
        virtual bool mem_u_write(uint32_t address, uint32_t size, const bits& value);
        virtual bool mem_u_with_priv_write(uint32_t address, uint32_t size, bool privileged, const bits& value);

        virtual bool translate_address(address_descriptor* desc, uint32_t va,
                                       bool is_priv, bool is_write, uint32_t size,
                                       bool inst_fetch, bool gdb_access);

    protected:          //imple. of VMSA pseudo code
        virtual void call_hypervisor(uint16_t immediate);
        virtual bool check_domain(uint32_t domain, uint32_t mva, uint32_t level,
                                  bool is_write);
        virtual bool check_permission_s2(permissions perms, uint32_t mva, uint64_t ipa,
                                         uint32_t level, bool is_write, bool s2fs1_walk,
                                         bool inst_fetch, bool gdb_access);
        virtual void combine_s1s2_desc(address_descriptor* result, address_descriptor s1_desc,
                                       address_descriptor s2_desc);
        virtual uint32_t convert_attrs_hints(uint32_t rgn);
        virtual uint32_t fcse_translate(uint32_t va);
        virtual void mair_decode(memory_attributes* mem_attrs, uint32_t attr);
        virtual void remapped_tex_decode(memory_attributes* mem_attrs, uint32_t texcb, bool s);
        virtual void s2_attr_decode(memory_attributes* result, uint32_t attr);
        virtual bool second_stage_translate(address_descriptor* result,
                                            address_descriptor s1_out_addr_desc,
                                            uint32_t mva, uint32_t size, bool inst_fetch,
                                            bool gdb_access);
        virtual bool translate_address_v(address_descriptor* result, uint32_t va,
                                         bool is_priv, bool is_write, uint32_t size,
                                         bool inst_fetch, bool gdb_access);
        virtual bool translate_address_v_s1_off(tlb_record* result, uint32_t va,
                                                bool inst_fetch, bool gdb_access);
        virtual bool translation_table_walk_sd(tlb_record* result, uint32_t mva,
                                               bool is_write, uint32_t size,
                                               bool inst_fetch, bool gdb_access);
        virtual bool translation_table_walk_ld(tlb_record* result, uint64_t ia,
                                               uint32_t va, bool is_write, bool stage1,
                                               bool s2fs1_walk, uint32_t size,
                                               bool inst_fetch, bool gdb_access);
        virtual void write_hsr(uint32_t ec, uint32_t hsr_string);

    protected:          //imple. of exceptions pseudo code
        virtual void call_supervisor();
        virtual uint32_t exc_vector_base();
        virtual void enter_hyp_mode(uint32_t new_spsr_value, uint32_t preferred_exceptn_return,
                                    uint32_t vect_offset);
        virtual bool generate_coprocessor_exception(uint32_t cp, bool is_64, bool is_write,
                                                    uint32_t crn, uint32_t opc1, uint32_t crm,
                                                    uint32_t opc2, uint32_t rt);
        virtual void prefetch_abort(uint32_t v_address, uint64_t ip_address, uint32_t level,
                                    bool is_write, fault_type type, bool take_to_hyp_mode,
                                    bool second_stage_abort, bool ipa_valid, bool ldfsr_format,
                                    bool s2fs1_walk);
        virtual void take_data_abort_exception(bool is_alignment_fault, bool second_stage_abort);
        virtual void take_hvc_exception();
        virtual void take_hyp_trap_exception();
        virtual void take_physical_irq_exception();
        virtual void take_prefetch_abort_exception(bool second_stage_abort);
        virtual void take_svc_exception();
        virtual void take_virtual_irq_exception();


    protected:          //instruction decode functions
        virtual void decode_br(armv7a_ir& inst);

        virtual void decode_dpimm(armv7a_ir& inst);
        virtual void decode_dpmisc(armv7a_ir& inst);
        virtual void decode_dpreg(armv7a_ir& inst);
        virtual void decode_dpsreg(armv7a_ir& inst);

        virtual void decode_extrals(armv7a_ir& inst);
        virtual void decode_extrals_up(armv7a_ir& inst);

        virtual void decode_hmulmacc(armv7a_ir& inst);
        virtual void decode_ls(armv7a_ir& inst);

        virtual void decode_media(armv7a_ir& inst);
        virtual void decode_misc(armv7a_ir& inst);
        virtual void decode_misc_mhints(armv7a_ir& inst);
        virtual void decode_msrhints(armv7a_ir& inst);
        virtual void decode_mulmacc(armv7a_ir& inst);

        virtual void decode_pack_satrev(armv7a_ir& inst);
        virtual void decode_paddsub_s(armv7a_ir& inst);
        virtual void decode_paddsub_us(armv7a_ir& inst);

        virtual void decode_sat(armv7a_ir& inst);
        virtual void decode_smul(armv7a_ir& inst);
        virtual void decode_svccp(armv7a_ir& inst);
        virtual void decode_sync(armv7a_ir& inst);
        virtual void decode_top(armv7a_ir& inst);

        virtual void decode_uncond(armv7a_ir& inst);
        virtual void decode_undef(armv7a_ir& inst);

    protected:          //imple. of instruction pseudo code
        //br
        virtual void arm_stm_da(armv7a_ir& inst);
        virtual void arm_ldm_da(armv7a_ir& inst);
        virtual void arm_stm_ia(armv7a_ir& inst);
        virtual void arm_ldm_ia(armv7a_ir& inst);
        virtual void arm_pop(armv7a_ir& inst);
        virtual void arm_stm_db(armv7a_ir& inst);
        virtual void arm_push(armv7a_ir& inst);
        virtual void arm_ldm_db(armv7a_ir& inst);
        virtual void arm_stm_ib(armv7a_ir& inst);
        virtual void arm_ldm_ib(armv7a_ir& inst);
        virtual void arm_stm_usr(armv7a_ir& inst);
        virtual void arm_ldm_usr(armv7a_ir& inst);
        virtual void arm_ldm_except(armv7a_ir& inst);
        virtual void arm_b(armv7a_ir& inst);
        virtual void arm_bl(armv7a_ir& inst);

        //dpimm
        virtual void arm_and_imm(armv7a_ir& inst);
        virtual void arm_eor_imm(armv7a_ir& inst);
        virtual void arm_sub_imm(armv7a_ir& inst);
        virtual void arm_adr(armv7a_ir& inst);
        virtual void arm_rsb_imm(armv7a_ir& inst);
        virtual void arm_add_imm(armv7a_ir& inst);
        //virtual void arm_adr           (armv7a_ir& inst);
        virtual void arm_adc_imm(armv7a_ir& inst);
        virtual void arm_sbc_imm(armv7a_ir& inst);
        virtual void arm_rsc_imm(armv7a_ir& inst);
        virtual void arm_tst_imm(armv7a_ir& inst);
        virtual void arm_teq_imm(armv7a_ir& inst);
        virtual void arm_cmp_imm(armv7a_ir& inst);
        virtual void arm_cmn_imm(armv7a_ir& inst);
        virtual void arm_orr_imm(armv7a_ir& inst);
        //virtual void arm_mov_imm(armv7a_ir& inst); // in dpmisc
        virtual void arm_bic_imm(armv7a_ir& inst);
        virtual void arm_mvn_imm(armv7a_ir& inst);

        //dpmisc
        virtual void arm_mov_imm(armv7a_ir& inst);
        virtual void arm_movt(armv7a_ir& inst);

        //dpreg
        virtual void arm_and_reg(armv7a_ir& inst);
        virtual void arm_eor_reg(armv7a_ir& inst);
        virtual void arm_sub_reg(armv7a_ir& inst);
        virtual void arm_rsb_reg(armv7a_ir& inst);
        virtual void arm_add_reg(armv7a_ir& inst);
        virtual void arm_adc_reg(armv7a_ir& inst);
        virtual void arm_sbc_reg(armv7a_ir& inst);
        virtual void arm_rsc_reg(armv7a_ir& inst);
        virtual void arm_tst_reg(armv7a_ir& inst);
        virtual void arm_teq_reg(armv7a_ir& inst);
        virtual void arm_cmp_reg(armv7a_ir& inst);
        virtual void arm_cmn_reg(armv7a_ir& inst);
        virtual void arm_orr_reg(armv7a_ir& inst);
        virtual void arm_mov_reg(armv7a_ir& inst);
        virtual void arm_lsl_imm(armv7a_ir& inst);
        virtual void arm_lsr_imm(armv7a_ir& inst);
        virtual void arm_asr_imm(armv7a_ir& inst);
        virtual void arm_rrx(armv7a_ir& inst);
        virtual void arm_ror_imm(armv7a_ir& inst);
        virtual void arm_bic_reg(armv7a_ir& inst);
        virtual void arm_mvn_reg(armv7a_ir& inst);

        //dpsreg
        virtual void arm_and_sreg(armv7a_ir& inst);
        virtual void arm_eor_sreg(armv7a_ir& inst);
        virtual void arm_sub_sreg(armv7a_ir& inst);
        virtual void arm_rsb_sreg(armv7a_ir& inst);
        virtual void arm_add_sreg(armv7a_ir& inst);
        virtual void arm_adc_sreg(armv7a_ir& inst);
        virtual void arm_sbc_sreg(armv7a_ir& inst);
        virtual void arm_rsc_sreg(armv7a_ir& inst);
        virtual void arm_tst_sreg(armv7a_ir& inst);
        virtual void arm_teq_sreg(armv7a_ir& inst);
        virtual void arm_cmp_sreg(armv7a_ir& inst);
        virtual void arm_cmn_sreg(armv7a_ir& inst);
        virtual void arm_orr_sreg(armv7a_ir& inst);
        virtual void arm_lsl_reg(armv7a_ir& inst);
        virtual void arm_lsr_reg(armv7a_ir& inst);
        virtual void arm_asr_reg(armv7a_ir& inst);
        virtual void arm_ror_reg(armv7a_ir& inst);
        virtual void arm_bic_sreg(armv7a_ir& inst);
        virtual void arm_mvn_sreg(armv7a_ir& inst);

        //extrals
        virtual void arm_strh_reg(armv7a_ir& inst);
        virtual void arm_ldrh_reg(armv7a_ir& inst);
        virtual void arm_strh_imm(armv7a_ir& inst);
        virtual void arm_ldrh_imm(armv7a_ir& inst);
        virtual void arm_ldrh_ltrl(armv7a_ir& inst);
        virtual void arm_ldrd_reg(armv7a_ir& inst);
        virtual void arm_ldrsb_reg(armv7a_ir& inst);
        virtual void arm_ldrd_imm(armv7a_ir& inst);
        virtual void arm_ldrd_ltrl(armv7a_ir& inst);
        virtual void arm_ldrsb_imm(armv7a_ir& inst);
        virtual void arm_ldrsb_ltrl(armv7a_ir& inst);
        virtual void arm_strd_reg(armv7a_ir& inst);
        virtual void arm_ldrsh_reg(armv7a_ir& inst);
        virtual void arm_strd_imm(armv7a_ir& inst);
        virtual void arm_ldrsh_imm(armv7a_ir& inst);
        virtual void arm_ldrsh_ltrl(armv7a_ir& inst);

        //extrals_up
        virtual void arm_strht(armv7a_ir& inst);
        virtual void arm_ldrht(armv7a_ir& inst);
        virtual void arm_ldrsbt(armv7a_ir& inst);
        virtual void arm_ldrsht(armv7a_ir& inst);

        //hmulmacc
        virtual void arm_smla(armv7a_ir& inst);
        virtual void arm_smlaw(armv7a_ir& inst);
        virtual void arm_smulw(armv7a_ir& inst);
        virtual void arm_smlalxx(armv7a_ir& inst);
        virtual void arm_smul(armv7a_ir& inst);

        //ls
        virtual void arm_str_imm(armv7a_ir& inst);
        virtual void arm_str_reg(armv7a_ir& inst);
        virtual void arm_strt(armv7a_ir& inst);
        virtual void arm_ldr_imm(armv7a_ir& inst);
        virtual void arm_ldr_ltrl(armv7a_ir& inst);
        virtual void arm_ldr_reg(armv7a_ir& inst);
        virtual void arm_ldrt(armv7a_ir& inst);
        virtual void arm_strb_imm(armv7a_ir& inst);
        virtual void arm_strb_reg(armv7a_ir& inst);
        virtual void arm_strbt(armv7a_ir& inst);
        virtual void arm_ldrb_imm(armv7a_ir& inst);
        virtual void arm_ldrb_ltrl(armv7a_ir& inst);
        virtual void arm_ldrb_reg(armv7a_ir& inst);
        virtual void arm_ldrbt(armv7a_ir& inst);

        //media
        virtual void arm_usad8(armv7a_ir& inst);
        virtual void arm_usada8(armv7a_ir& inst);
        virtual void arm_sbfx(armv7a_ir& inst);
        virtual void arm_bfc(armv7a_ir& inst);
        virtual void arm_bfi(armv7a_ir& inst);
        virtual void arm_ubfx(armv7a_ir& inst);

        //misc
        virtual void arm_mrs_bk(armv7a_ir& inst);
        virtual void arm_msr_bk(armv7a_ir& inst);
        virtual void arm_mrs(armv7a_ir& inst);
        virtual void arm_msr_reg_ap(armv7a_ir& inst);
        virtual void arm_msr_reg_sys(armv7a_ir& inst);
        //virtual void arm_msr_reg_sys(armv7a_ir& inst);
        virtual void arm_bx(armv7a_ir& inst);
        virtual void arm_clz(armv7a_ir& inst);
        virtual void arm_bxj(armv7a_ir& inst);
        virtual void arm_blx_reg(armv7a_ir& inst);
        virtual void arm_eret(armv7a_ir& inst);
        virtual void arm_bkpt(armv7a_ir& inst);
        virtual void arm_hvc(armv7a_ir& inst);
        virtual void arm_smc(armv7a_ir& inst);

        //misc_mhints
        virtual void arm_cps(armv7a_ir& inst);
        virtual void arm_setend(armv7a_ir& inst);
        virtual void arm_pli_immltrl(armv7a_ir& inst);
        virtual void arm_pld_imm(armv7a_ir& inst);
        virtual void arm_pld_ltrl(armv7a_ir& inst);
        virtual void arm_clrex(armv7a_ir& inst);
        virtual void arm_dsb(armv7a_ir& inst);
        virtual void arm_dmb(armv7a_ir& inst);
        virtual void arm_isb(armv7a_ir& inst);
        virtual void arm_pli_reg(armv7a_ir& inst);
        virtual void arm_pld_reg(armv7a_ir& inst);

        //msrhints
        virtual void arm_nop(armv7a_ir& inst);
        virtual void arm_yield(armv7a_ir& inst);
        virtual void arm_wfe(armv7a_ir& inst);
        virtual void arm_wfi(armv7a_ir& inst);
        virtual void arm_sev(armv7a_ir& inst);
        virtual void arm_dbg(armv7a_ir& inst);
        virtual void arm_msr_imm_ap(armv7a_ir& inst);

        virtual void arm_msr_imm_sys(armv7a_ir& inst);

        //mulmacc
        virtual void arm_mul(armv7a_ir& inst);
        virtual void arm_mla(armv7a_ir& inst);
        virtual void arm_umaal(armv7a_ir& inst);
        virtual void arm_mls(armv7a_ir& inst);
        virtual void arm_umull(armv7a_ir& inst);
        virtual void arm_umlal(armv7a_ir& inst);
        virtual void arm_smull(armv7a_ir& inst);
        virtual void arm_smlal(armv7a_ir& inst);

        //pack_satrev
        virtual void arm_pkh(armv7a_ir& inst);
        virtual void arm_ssat(armv7a_ir& inst);
        virtual void arm_usat(armv7a_ir& inst);
        virtual void arm_sxtab16(armv7a_ir& inst);
        virtual void arm_sxtb16(armv7a_ir& inst);
        virtual void arm_sel(armv7a_ir& inst);
        virtual void arm_ssat16(armv7a_ir& inst);
        virtual void arm_sxtab(armv7a_ir& inst);
        virtual void arm_sxtb(armv7a_ir& inst);
        virtual void arm_rev(armv7a_ir& inst);
        virtual void arm_sxtah(armv7a_ir& inst);
        virtual void arm_sxth(armv7a_ir& inst);
        virtual void arm_rev16(armv7a_ir& inst);
        virtual void arm_uxtab16(armv7a_ir& inst);
        virtual void arm_uxtb16(armv7a_ir& inst);
        virtual void arm_usat16(armv7a_ir& inst);
        virtual void arm_uxtab(armv7a_ir& inst);
        virtual void arm_uxtb(armv7a_ir& inst);
        virtual void arm_rbit(armv7a_ir& inst);
        virtual void arm_uxtah(armv7a_ir& inst);
        virtual void arm_uxth(armv7a_ir& inst);
        virtual void arm_revsh(armv7a_ir& inst);

        //paddsub_s
        virtual void arm_sadd16(armv7a_ir& inst);
        virtual void arm_sasx(armv7a_ir& inst);
        virtual void arm_ssax(armv7a_ir& inst);
        virtual void arm_ssub16(armv7a_ir& inst);
        virtual void arm_sadd8(armv7a_ir& inst);
        virtual void arm_ssub8(armv7a_ir& inst);

        virtual void arm_qadd16(armv7a_ir& inst);
        virtual void arm_qasx(armv7a_ir& inst);
        virtual void arm_qsax(armv7a_ir& inst);
        virtual void arm_qsub16(armv7a_ir& inst);
        virtual void arm_qadd8(armv7a_ir& inst);
        virtual void arm_qsub8(armv7a_ir& inst);

        virtual void arm_shadd16(armv7a_ir& inst);
        virtual void arm_shasx(armv7a_ir& inst);
        virtual void arm_shsax(armv7a_ir& inst);
        virtual void arm_shsub16(armv7a_ir& inst);
        virtual void arm_shadd8(armv7a_ir& inst);
        virtual void arm_shsub8(armv7a_ir& inst);

        //paddsub_us
        virtual void arm_uadd16(armv7a_ir& inst);
        virtual void arm_uasx(armv7a_ir& inst);
        virtual void arm_usax(armv7a_ir& inst);
        virtual void arm_usub16(armv7a_ir& inst);
        virtual void arm_uadd8(armv7a_ir& inst);
        virtual void arm_usub8(armv7a_ir& inst);

        virtual void arm_uqadd16(armv7a_ir& inst);
        virtual void arm_uqasx(armv7a_ir& inst);
        virtual void arm_uqsax(armv7a_ir& inst);
        virtual void arm_uqsub16(armv7a_ir& inst);
        virtual void arm_uqadd8(armv7a_ir& inst);
        virtual void arm_uqsub8(armv7a_ir& inst);

        virtual void arm_uhadd16(armv7a_ir& inst);
        virtual void arm_uhasx(armv7a_ir& inst);
        virtual void arm_uhsax(armv7a_ir& inst);
        virtual void arm_uhsub16(armv7a_ir& inst);
        virtual void arm_uhadd8(armv7a_ir& inst);
        virtual void arm_uhsub8(armv7a_ir& inst);

        //sat
        virtual void arm_qadd(armv7a_ir& inst);
        virtual void arm_qsub(armv7a_ir& inst);
        virtual void arm_qdadd(armv7a_ir& inst);
        virtual void arm_qdsub(armv7a_ir& inst);

        //smul
        virtual void arm_smlad(armv7a_ir& inst);
        virtual void arm_smuad(armv7a_ir& inst);
        virtual void arm_smlsd(armv7a_ir& inst);
        virtual void arm_smusd(armv7a_ir& inst);
        virtual void arm_smlald(armv7a_ir& inst);
        virtual void arm_smlsld(armv7a_ir& inst);
        virtual void arm_smmla(armv7a_ir& inst);
        virtual void arm_smmul(armv7a_ir& inst);
        virtual void arm_smmls(armv7a_ir& inst);

        //svccp
        virtual void arm_stc(armv7a_ir& inst);
        virtual void arm_ldc_imm(armv7a_ir& inst);
        virtual void arm_ldc_ltrl(armv7a_ir& inst);
        virtual void arm_mcrr(armv7a_ir& inst);
        virtual void arm_mrrc(armv7a_ir& inst);
        virtual void arm_cdp(armv7a_ir& inst);
        virtual void arm_mcr(armv7a_ir& inst);
        virtual void arm_mrc(armv7a_ir& inst);
        virtual void arm_svc(armv7a_ir& inst);

        //sync
        virtual void arm_swp(armv7a_ir& inst);
        virtual void arm_strex(armv7a_ir& inst);
        virtual void arm_ldrex(armv7a_ir& inst);
        virtual void arm_strexd(armv7a_ir& inst);
        virtual void arm_ldrexd(armv7a_ir& inst);
        virtual void arm_strexb(armv7a_ir& inst);
        virtual void arm_ldrexb(armv7a_ir& inst);
        virtual void arm_strexh(armv7a_ir& inst);
        virtual void arm_ldrexh(armv7a_ir& inst);

        //uncond
        virtual void arm_srs(armv7a_ir& inst);
        virtual void arm_rfe(armv7a_ir& inst);
        virtual void arm_bl_imm(armv7a_ir& inst);
        //virtual void arm_ldc_imm(armv7a_ir& inst);
        //virtual void arm_ldc_ltrl(armv7a_ir& inst);
        //virtual void arm_stc(armv7a_ir& inst);
        //virtual void arm_mcrr(armv7a_ir& inst);
        //virtual void arm_mrrc(armv7a_ir& inst);
        //virtual void arm_cdp(armv7a_ir& inst);
        //virtual void arm_mcr(armv7a_ir& inst);
        //virtual void arm_mrc(armv7a_ir& inst);

        //strange instructions (not described in ARM Ref Manual Chapter A5, but used in some instructions)
        virtual void arm_add_sp_plus_reg(armv7a_ir& inst);
        virtual void arm_add_sp_plus_imm(armv7a_ir& inst);
        virtual void arm_sub_sp_minus_reg(armv7a_ir& inst);
        virtual void arm_sub_sp_minus_imm(armv7a_ir& inst);
        virtual void arm_subs_pc_lr(armv7a_ir& inst);

        //undef


    public:             //GDB stub
        bool using_gdb;
        bool using_prog;
        bool gdb_init;
        unsigned int gdb_port;
        GDBStub* gdb;
        virtual void regRead(uint32_t*, uint16_t);
        virtual void regWrite(uint32_t, uint16_t);
        virtual void memRead(uint32_t*, uint32_t, int);
        virtual void memWrite(uint32_t, uint32_t, int);
        virtual void bkptInsert(uint32_t);
        virtual void bkptRemove(uint32_t);
        virtual void bkptRemove();
        virtual void awatchInsert(uint32_t);
        virtual void rwatchInsert(uint32_t);
        virtual void wwatchInsert(uint32_t);
        virtual void awatchRemove(uint32_t);
        virtual void rwatchRemove(uint32_t);
        virtual void wwatchRemove(uint32_t);
        virtual void watchRemove();
    protected:
        BKPT break_point[SIZE_BKPT];
        BKPT rwatch_point[SIZE_BKPT];
        BKPT wwatch_point[SIZE_BKPT];
        virtual bool bkptCheck(uint32_t);
        virtual bool rwatchCheck(uint32_t);
        virtual bool wwatchCheck(uint32_t);

};

#endif
