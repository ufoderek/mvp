#include <armv7a.h>

void armv7a::decode_smul(armv7a_ir& inst)
{
    uint32_t op1 = inst(22, 20);
    uint32_t A = inst(15, 12);
    uint32_t op2 = inst(7, 5);
    uint32_t op2t = op2 >> 1;

    if(op1 == B(000))
    {
        if(op2t == B(00))
        {
            if(A != B(1111))
            {
                arm_smlad(inst);
            }

            else
            {
                arm_smuad(inst);
            }
        }

        else if(op2t == B(01))
        {
            if(A != B(1111))
            {
                arm_smlsd(inst);
            }

            else
            {
                arm_smusd(inst);
            }
        }

        else
        {
            printb(core_id, d_armv7a_decode_smul, "decode error 000: %X", inst.val);
        }
    }

    else if(op1 == B(100))
    {
        if(op2t == B(00))
        {
            arm_smlald(inst);
        }

        else if(op2t == B(01))
        {
            arm_smlsld(inst);
        }

        else
        {
            printb(core_id, d_armv7a_decode_smul, "decode error 100: %X", inst.val);
        }
    }

    else if(op1 == B(101))
    {
        if(op2t == B(00))
        {
            if(A != B(1111))
            {
                arm_smmla(inst);
            }

            else
            {
                arm_smmul(inst);
            }
        }

        else if(op2t == B(11))
        {
            arm_smmls(inst);
        }

        else
        {
            printb(core_id, d_armv7a_decode_smul, "decode error 101: %X", inst.val);
        }
    }

    else
    {
        printb(core_id, d_armv7a_decode_smul, "decode error: %X", inst.val);
    }
}

void armv7a::arm_smlad(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smlad not implemented yet.");
}

void armv7a::arm_smuad(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smuad not implemented yet.");
}

void armv7a::arm_smlsd(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smlsd not implemented yet.");
}

void armv7a::arm_smusd(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smusd not implemented yet.");
}

void armv7a::arm_smlald(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smlald not implemented yet.");
}

void armv7a::arm_smlsld(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smlsld not implemented yet.");
}

void armv7a::arm_smmla(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smmla not implemented yet.");
}

void armv7a::arm_smmul(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smmul not implemented yet.");
}

void armv7a::arm_smmls(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_smmls not implemented yet.");
}

