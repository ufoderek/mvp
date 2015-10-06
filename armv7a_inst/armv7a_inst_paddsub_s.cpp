#include <armv7a.h>

void armv7a::decode_paddsub_s(armv7a_ir& inst)
{
    uint32_t op1 = inst(21, 20);
    uint32_t op2 = inst(7, 5);

    if((op1 == B(01)) && (op2 == B(000)))
    {
        arm_sadd16(inst);
    }

    else if((op1 == B(01)) && (op2 == B(001)))
    {
        arm_sasx(inst);
    }

    else if((op1 == B(01)) && (op2 == B(010)))
    {
        arm_ssax(inst);
    }

    else if((op1 == B(01)) && (op2 == B(011)))
    {
        arm_ssub16(inst);
    }

    else if((op1 == B(01)) && (op2 == B(100)))
    {
        arm_sadd8(inst);
    }

    else if((op1 == B(01)) && (op2 == B(111)))
    {
        arm_ssub8(inst);
    }

    else if((op1 == B(10)) && (op2 == B(000)))
    {
        arm_qadd16(inst);
    }

    else if((op1 == B(10)) && (op2 == B(001)))
    {
        arm_qasx(inst);
    }

    else if((op1 == B(10)) && (op2 == B(010)))
    {
        arm_qsax(inst);
    }

    else if((op1 == B(10)) && (op2 == B(011)))
    {
        arm_qsub16(inst);
    }

    else if((op1 == B(10)) && (op2 == B(100)))
    {
        arm_qadd8(inst);
    }

    else if((op1 == B(10)) && (op2 == B(111)))
    {
        arm_qsub8(inst);
    }

    else if((op1 == B(11)) && (op2 == B(000)))
    {
        arm_shadd16(inst);
    }

    else if((op1 == B(11)) && (op2 == B(001)))
    {
        arm_shasx(inst);
    }

    else if((op1 == B(11)) && (op2 == B(010)))
    {
        arm_shsax(inst);
    }

    else if((op1 == B(11)) && (op2 == B(011)))
    {
        arm_shsub16(inst);
    }

    else if((op1 == B(11)) && (op2 == B(100)))
    {
        arm_shadd8(inst);
    }

    else if((op1 == B(11)) && (op2 == B(111)))
    {
        arm_shsub8(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_paddsub_s, "decode error: %X", inst.val);
    }
}

void armv7a::arm_sadd16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sadd16 not implemented yet.");
}

void armv7a::arm_sasx(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sasx not implemented yet.");
}

void armv7a::arm_ssax(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ssax not implemented yet.");
}

void armv7a::arm_ssub16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ssub16 not implemented yet.");
}

void armv7a::arm_sadd8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_sadd8 not implemented yet.");
}

void armv7a::arm_ssub8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ssub8 not implemented yet.");
}

void armv7a::arm_qadd16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qadd16 not implemented yet.");
}

void armv7a::arm_qasx(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qasx not implemented yet.");
}

void armv7a::arm_qsax(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qsax not implemented yet.");
}

void armv7a::arm_qsub16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qsub16 not implemented yet.");
}

void armv7a::arm_qadd8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qadd8 not implemented yet.");
}

void armv7a::arm_qsub8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_qsub8 not implemented yet.");
}

void armv7a::arm_shadd16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_shadd16 not implemented yet.");
}

void armv7a::arm_shasx(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_shasx not implemented yet.");
}

void armv7a::arm_shsax(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_shsax not implemented yet.");
}

void armv7a::arm_shsub16(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_shsub16 not implemented yet.");
}

void armv7a::arm_shadd8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_shadd8 not implemented yet.");
}

void armv7a::arm_shsub8(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_shsub8 not implemented yet.");
}

