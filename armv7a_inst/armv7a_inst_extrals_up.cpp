#include <armv7a.h>

void armv7a::decode_extrals_up(armv7a_ir& inst)
{
    uint32_t op = inst(20);
    uint32_t op2 = inst(6, 5);
    uint32_t rt = inst(15, 12);
    bool op2_is1x = (op2 & B(10)) == B(10);
    bool rt_isxxxx0 = (rt & B(0001)) == B(0000);
    bool rt_isxxxx1 = (rt & B(0001)) == B(0001);

    if((op2 == B(01)) && (op == 0))
    {
        arm_strht(inst);
    }

    else if((op2 == B(01)) && (op == 1))
    {
        arm_ldrht(inst);
    }

    else if((op2_is1x) && (op == 0) && (rt_isxxxx0 || rt_isxxxx1))
    {
        printb(core_id, d_armv7a_decode_extrals_up, "unpredictable");
    }

    else if((op2 == B(10)) && (op == 1))
    {
        arm_ldrsbt(inst);
    }

    else if((op2 == B(11)) && (op == 1))
    {
        arm_ldrsht(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_extrals_up, "decode error");
    }
}

void armv7a::arm_strht(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_strht not implemented yet.");
}

void armv7a::arm_ldrht(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrht not implemented yet.");
}

void armv7a::arm_ldrsbt(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrsbt not implemented yet.");
}

void armv7a::arm_ldrsht(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrsht not implemented yet.");
}

