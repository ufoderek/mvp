#include <armv7a.h>

void armv7a::decode_hmulmacc(armv7a_ir& inst)
{
    uint32_t op1 = inst(22, 21);
    uint32_t op = inst(5);

    switch(op1)
    {
        case B(00):
            arm_smla(inst);
            return;
        case B(01):

            if(op == 0)
            {
                arm_smlaw(inst);
            }

            else
            {
                arm_smulw(inst);
            }

            return;
        case B(10):
            arm_smlalxx(inst);
            return;
        case B(11):
            arm_smul(inst);
            return;
        default:
            printb(core_id, d_armv7a_decode_hmulmacc, "decode error");
    }
}

void armv7a::arm_smla(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smla not implemented yet.");
}

void armv7a::arm_smlaw(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smlaw not implemented yet.");
}

void armv7a::arm_smulw(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smulw not implemented yet.");
}

void armv7a::arm_smlalxx(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smlalxx not implemented yet.");
}

void armv7a::arm_smul(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smul not implemented yet.");
}

