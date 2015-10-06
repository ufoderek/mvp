#include <armv7a.h>

void armv7a::decode_uncond(armv7a_ir& inst)
{
    uint32_t op1 = inst(27, 20);
    uint32_t rn = inst(19, 16);
    uint32_t op = inst(4);
    bool op1_0xxx_xxxx = (op1 & B(1000 0000)) == B(0000 0000);
    bool op1_100x_x1x0 = (op1 & B(1110 0101)) == B(1000 0100);
    bool op1_100x_x0x1 = (op1 & B(1110 0101)) == B(1000 0001);
    bool op1_101x_xxxx = (op1 & B(1110 0000)) == B(1010 0000);
    bool op1_1100_0x11 = (op1 & B(1111 1011)) == B(1100 0011);
    bool op1_1100_1xx1 = (op1 & B(1111 1001)) == B(1100 1001);
    bool op1_1101_xxx1 = (op1 & B(1111 0001)) == B(1101 0001);
    //bool op1_1100_1xx1 = (op1 & B(1111 1001)) == B(1100 1001);
    bool op1_1100_0x10 = (op1 & B(1111 1011)) == B(1100 0010);
    bool op1_1100_1xx0 = (op1 & B(1111 1001)) == B(1100 1000);
    bool op1_1101_xxx0 = (op1 & B(1111 0001)) == B(1101 0000);
    bool op1_1100_0100 = (op1 & B(1111 1111)) == B(1100 0100);
    bool op1_1100_0101 = (op1 & B(1111 1111)) == B(1100 0101);
    bool op1_1110_xxxx = (op1 & B(1111 0000)) == B(1110 0000);
    bool op1_1110_xxx0 = (op1 & B(1111 0001)) == B(1110 0000);
    bool op1_1110_xxx1 = (op1 & B(1111 0001)) == B(1110 0001);
    bool op_0 = op == 0;
    bool op_1 = op == 1;
    bool rn_1111 = rn == B(1111);

    if(op1_0xxx_xxxx)
    {
        decode_misc_mhints(inst);
    }

    else if(op1_100x_x1x0)
    {
        arm_srs(inst);
    }

    else if(op1_100x_x0x1)
    {
        arm_rfe(inst);
    }

    else if(op1_101x_xxxx)
    {
        arm_bl_imm(inst);
    }

    else if(op1_1100_0x11 && !rn_1111)
    {
        arm_ldc_imm(inst);
    }

    else if(op1_1100_1xx1 && rn_1111)
    {
        arm_ldc_ltrl(inst);
    }

    else if(op1_1101_xxx1 && rn_1111)
    {
        arm_ldc_ltrl(inst);
    }

    else if(op1_1100_0x10 || op1_1100_1xx0 || op1_1101_xxx0)
    {
        arm_stc(inst);
    }

    else if(op1_1100_0100)
    {
        arm_mcrr(inst);
    }

    else if(op1_1100_0101)
    {
        arm_mrrc(inst);
    }

    else if(op1_1110_xxxx && op_0)
    {
        arm_cdp(inst);
    }

    else if(op1_1110_xxx0 && op_1)
    {
        arm_mcr(inst);
    }

    else if(op1_1110_xxx1 && op_1)
    {
        arm_mrc(inst);
    }

    else
    {
        printb(core_id, d_armv7a_decode_uncond, "unconditional instructions decode error");
    }
}

void armv7a::arm_srs(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_srs not implemented yet.");
}

void armv7a::arm_rfe(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_rfe not implemented yet.");
}

void armv7a::arm_bl_imm(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_bl_imm not implemented yet.");
}

/*
   void armv7a::arm_ldc_imm(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_ldc_imm not implemented yet.");
   }

   void armv7a::arm_ldc_ltrl(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_ldc_ltrl not implemented yet.");
   }

   void armv7a::arm_stc(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_stc not implemented yet.");
   }

   void armv7a::arm_mcrr(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_mcrr not implemented yet.");
   }

   void armv7a::arm_mrrc(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_mrrc not implemented yet.");
   }

   void armv7a::arm_cdp(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_cdp not implemented yet.");
   }

   void armv7a::arm_mcr(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_mcr not implemented yet.");
   }

   void armv7a::arm_mrc(armv7a_ir& inst)
   {
    printb(core_id, d_inst, "arm_mrc not implemented yet.");
   }
 */

