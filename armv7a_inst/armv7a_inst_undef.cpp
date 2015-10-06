#include <armv7a.h>

void armv7a::decode_undef(armv7a_ir& inst)
{
    printb(core_id, d_armv7a_decode_undef, "decode error: %X", inst.val);
}

