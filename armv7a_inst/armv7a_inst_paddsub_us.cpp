#include <armv7a.h>

void armv7a::decode_paddsub_us(armv7a_ir& inst)
{
    uint32_t op1 = inst(21, 20);
    uint32_t op2 = inst(7, 5);

    if((op1 == B(01)) && (op2 == B(000)))
    {
        arm_uadd16(inst);
    }

    else if((op1 == B(01)) && (op2 == B(001)))
    {
        arm_uasx(inst);
    }

    else if((op1 == B(01)) && (op2 == B(010)))
    {
        arm_usax(inst);
    }

    else if((op1 == B(01)) && (op2 == B(011)))
    {
        arm_usub16(inst);
    }

    else if((op1 == B(01)) && (op2 == B(100)))
    {
        arm_uadd8(inst);
    }

    else if((op1 == B(01)) && (op2 == B(111)))
    {
        arm_usub8(inst);
    }

    else if((op1 == B(10)) && (op2 == B(000)))
    {
        arm_uqadd16(inst);
    }

    else if((op1 == B(10)) && (op2 == B(001)))
    {
        arm_uqasx(inst);
    }

    else if((op1 == B(10)) && (op2 == B(010)))
    {
        arm_uqsax(inst);
    }

    else if((op1 == B(10)) && (op2 == B(011)))
    {
        arm_uqsub16(inst);
    }

    else if((op1 == B(10)) && (op2 == B(100)))
    {
        arm_uqadd8(inst);
    }

    else if((op1 == B(10)) && (op2 == B(111)))
    {
        arm_uqsub8(inst);
    }

    else if((op1 == B(11)) && (op2 == B(000)))
    {
        arm_uhadd16(inst);
    }

    else if((op1 == B(11)) && (op2 == B(001)))
    {
        arm_uhasx(inst);
    }

    else if((op1 == B(11)) && (op2 == B(010)))
    {
        arm_uhsax(inst);
    }

    else if((op1 == B(11)) && (op2 == B(011)))
    {
        arm_uhsub16(inst);
    }

    else if((op1 == B(11)) && (op2 == B(100)))
    {
        arm_uhadd8(inst);
    }

    else if((op1 == B(11)) && (op2 == B(111)))
    {
        arm_uhsub8(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_paddsub_us, "decode error: %X", inst.val);
    }
}

void armv7a::arm_uadd16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uadd16 not implemented yet.");
}

void armv7a::arm_uasx(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uasx not implemented yet.");
}

void armv7a::arm_usax(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_usax not implemented yet.");
}

void armv7a::arm_usub16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_usub16 not implemented yet.");
}

void armv7a::arm_uadd8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uadd8 not implemented yet.");
}

void armv7a::arm_usub8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_usub8 not implemented yet.");
}

void armv7a::arm_uqadd16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uqadd16 not implemented yet.");
}

void armv7a::arm_uqasx(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uqasx not implemented yet.");
}

void armv7a::arm_uqsax(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uqsax not implemented yet.");
}

void armv7a::arm_uqsub16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uqsub16 not implemented yet.");
}

void armv7a::arm_uqadd8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uqadd8 not implemented yet.");
}

void armv7a::arm_uqsub8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uqsub8 not implemented yet.");
}

void armv7a::arm_uhadd16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uhadd16 not implemented yet.");
}

void armv7a::arm_uhasx(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uhasx not implemented yet.");
}

void armv7a::arm_uhsax(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uhsax not implemented yet.");
}

void armv7a::arm_uhsub16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uhsub16 not implemented yet.");
}

void armv7a::arm_uhadd8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_uhadd8 not implemented yet.");
}

void armv7a::arm_uhsub8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_shsub8 not implemented yet.");
}

