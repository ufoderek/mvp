#include <armv7a.h>
#include <bit_opt_v2.h>

/*
   decode_top
    decode_dpmisc
        decode_dpreg
        decode_dpsreg
        decode_misc
            decode_sat
        decode_hmulmacc
        decode_sync
        decode_extrals
        decode_extrals_up
        decode_dpimm
        decode_msrhints
    decode_ls
    decode_media
        decode_paddsub_s
        decode_paddsub_us
        decode_pack_satrev
        decode_smul
        decode_undef
    decode_br_ldmstm
    decode_svccp
        x decode_extls
        x decode_simd_vfp
    decode_uncond
        decode_misc_mhints

 */

void armv7a::decode_top(armv7a_ir& inst)
{
    if(!inst.match(31, 28, B(1111)))
    {
        //if (inst(31, 28) != B(1111)) {
        uint32_t op1 = inst(27, 25);
        uint32_t op = inst(4);
        printd(d_armv7a_decode_top, "condtional, inst=%X, op1=%X, op=%X", inst.val, op1, op);

        switch(op1)
        {
            case B(000):
            case B(001):
                printd(d_armv7a_decode_top, "dpmisc");
                decode_dpmisc(inst);
                return;
            case B(010):
                printd(d_armv7a_decode_top, "ls1");
                decode_ls(inst);
                return;
            case B(011):

                if(op == 0)
                {
                    printd(d_armv7a_decode_top, "ls2");
                    decode_ls(inst);
                }

                else
                {
                    printd(d_armv7a_decode_top, "media");
                    decode_media(inst);
                }

                return;
            case B(100):
            case B(101):
                printd(d_armv7a_decode_top, "br");
                decode_br(inst);
                return;
            case B(110):
            case B(111):
                printd(d_armv7a_decode_top, "supervisor call and coprocessor error");
                decode_svccp(inst);
                return;
            default:
                printb(d_armv7a_decode_top, "decode_top error");
                return;
        }
    }

    else
    {
        printd(d_armv7a_decode_top, "uncond");
        decode_uncond(inst);
    }
}

