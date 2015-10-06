#ifndef _ARMV7A_IR_H_
#define _ARMV7A_IR_H_

#include <bits.h>
#include <bit_opt_v2.h>

class armv7a_ir: public bits
{
    public:
        armv7a_ir();

        armv7a_ir& operator=(const armv7a_ir& right);

        void check(const uint32_t msb, const uint32_t lsb, const uint32_t pattern);
        void check(const uint32_t n, const uint32_t bit);

        char* get_name();
        void set_name(const char* name);

        uint32_t get_pc();
        void set_pc(uint32_t pc);

        void print_inst(const char* name);

        uint32_t cond();

        void vm_id(uint32_t _vm_id);
        uint32_t vm_id();
        void hyp_mode(bool _hyp_mode);
        bool hyp_mode();

    protected:
        char name[64];
        uint32_t pc;

        bool _hyp_mode;
        uint32_t _vm_id;
};

#endif

