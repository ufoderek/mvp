#include <armv7a.h>

void armv7a::decode_sat(armv7a_ir& inst)
{
    uint32_t op = inst(22, 21);

    switch(op)
    {
        case B(00):
            arm_qadd(inst);
            return;
        case B(01):
            arm_qsub(inst);
            return;
        case B(10):
            arm_qdadd(inst);
            return;
        case B(11):
            arm_qdsub(inst);
            return;
        default:
            printb(core_id, d_armv7a_decode_sat, "decode error");
    }
}
void armv7a::arm_qadd(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qadd not implemented yet.");
}

void armv7a::arm_qsub(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qsub not implemented yet.");
}

void armv7a::arm_qdadd(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qdadd not implemented yet.");
}

void armv7a::arm_qdsub(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qdsub not implemented yet.");
}

