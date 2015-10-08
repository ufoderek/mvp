#include <armv5.h>
#include <stdio.h>
#include <bit_opt.h>

void ARMV5::inst_assert_set(const unsigned int i)
{
#ifdef INST_ASSERT

    if ((inst & M(i)) != M(i)) {
        printb(core_id, d_inst_assert, "instruction %s bit:%u assert error, inst: 0x%X", current_inst_name, i, inst);
    }

#endif
}

void ARMV5::inst_assert_unset(const unsigned int i)
{
#ifdef INST_ASSERT

    if ((inst & M(i)) == M(i)) {
        printb(core_id, d_inst_assert, "instruction %s bit:%u assert error, inst: 0x%X", current_inst_name, i, inst);
    }

#endif
}

void ARMV5::inst_assert(const uint32_t pattern, unsigned int u, unsigned int l)
{
#ifdef INST_ASSERT

    if ((inst & MM(u, l)) != (pattern << l)) {
        printb(core_id, d_inst_assert, "instruction %s assert error, inst: 0x%X", current_inst_name, inst);
    }

#endif
}
