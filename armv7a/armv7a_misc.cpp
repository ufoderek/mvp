#include <armv7a.h>

#ifdef CPU_ASSESRT
#define pseudo_assert(expression,arg2,...) {\
        if(!(expression)){\
            printm_base(true,d_armv7a_misc,arg2,##__VA_ARGS__);\
        }\
    }
#else
#define pseudo_assert(expression,arg2,...) {;}
#endif

void armv7a::arm_expand_imm(bits* imm32, const bits& imm12)
{
    uint32_t carry;
    arm_expand_imm_c(imm32, &carry, imm12, rf.cpsr_C());
}

void armv7a::arm_expand_imm_c(bits* imm32, uint32_t* carry_out, const bits& imm12, uint32_t carry_in)
{
    bits unrotated_value(imm12(7, 0), 32);
    shift_c(imm32, carry_out, unrotated_value, SRType_ROR, 2 * imm12(11, 8), carry_in);
}

void armv7a::add_with_carry(bits* result, uint32_t* carry_out, uint32_t* overflow, const bits& x, const bits& y, uint32_t carry_in)
{
    uint64_t unsigned_sum = UInt(x) + UInt(y) + (uint64_t)carry_in;
    uint64_t signed_sum = SInt(x) + SInt(y) + (uint64_t)carry_in;
    result->val = unsigned_sum & mask(x.n - 1, 0);
    result->n = x.n;
    *carry_out = (UInt(*result) == unsigned_sum) ? 0 : 1;
    *overflow = (SInt(*result) == signed_sum) ? 0 : 1;
}

uint32_t armv7a::align(uint32_t x, uint32_t y)
{
    //x is address
    //y is size in bytes
    switch(y)
    {
        case 1:
            return x;
        case 2:
            return x & mask(31, 1);
        case 4:
            return x & mask(31, 2);
            /*
                    case 8:
               return x & mask(31, 3);
             */
        default:
            printb(d_armv7a_misc, "align(0x%X,%d) error", x, y);
    }
}

void armv7a::asr_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift)
{
    pseudo_assert(shift > 0, "asr_c assert error");
    //pseudo_assert(shift < 32, "asr_c assert error 2");

    if(shift >= 32)
    {
        /*
        if(x(x.n - 1) == 1)
        {
            result->val = mask(x.n - 1, 0);
        }
        else
        {
            result->val = 0;
        }
        result->n = x.n;
        return;
        */
        shift = 32;
    }

    //code modified by ufoderek
    uint64_t extended_x = sign_extend64(x);
    result->val = get_field64(extended_x, shift + x.n - 1, shift);
    result->n = x.n;
    *carry_out = get_bit(extended_x, shift - 1);
}

uint32_t armv7a::bit_count(bits& value)
{
    //GCC's built-in function which returns the number of 1-bits of the input unsigned integer
    return (uint32_t)__builtin_popcount(value.val);
}

uint32_t armv7a::count_leading_zero_bits(const bits& x)
{
    uint32_t count = 0;

    for(int i = x.n - 1; i >= 0; i--)
    {
        if(x(i) == 0)
        {
            count++;
        }
        else
        {
            break;
        }
    }

    return count;
}

void armv7a::decode_imm_shift(sr_type* shift_t, uint32_t* shift_n, uint32_t type, uint32_t imm5)
{
    switch(type)
    {
        case B(00):
            *shift_t = SRType_LSL;
            *shift_n = imm5;
            break;
        case B(01):
            *shift_t = SRType_LSR;
            *shift_n = (imm5 == 0) ? 32 : imm5;
            break;
        case B(10):
            *shift_t = SRType_ASR;
            *shift_n = (imm5 == 0) ? 32 : imm5;
            break;
        case B(11):

            if(imm5 == 0)
            {
                *shift_t = SRType_RRX;
                *shift_n = 1;
            }
            else
            {
                *shift_t = SRType_ROR;
                *shift_n = imm5;
            }

            break;
        default:
            //this error could never happen
            printb(d_armv7a_misc, "decode_imm_shift error");
    }
}

sr_type armv7a::decode_reg_shift(uint32_t type)
{
    sr_type shift_t;

    switch(type)
    {
        case B(00):
            shift_t = SRType_LSL;
            break;
        case B(01):
            shift_t = SRType_LSR;
            break;
        case B(10):
            shift_t = SRType_ASR;
            break;
        case B(11):
            shift_t = SRType_ROR;
            break;
        default:
            printb(d_armv7a_misc, "decode_reg_shift error");
    }

    return shift_t;
}

bool armv7a::is_zero(uint32_t x)
{
    return x == 0;
}

bool armv7a::is_zero64(uint64_t x)
{
    return x == (uint64_t)0;
}

bool armv7a::is_zero(const bits& x)
{
    //use simplified method
    return x.val == 0;
}

uint32_t armv7a::is_zero_bit(const bits& x)
{
    if(is_zero(x))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void armv7a::it_advance()
{
    //ARM Ref Manual v7 A2-18
    //For Thumb and ThumbEE only, so ignore here
    printm_once(d_armv7a, "ignore it_advance");
}

uint32_t armv7a::lowest_set_bit(const bits& x)
{
    for(int i = 0; i < x.n; i++)
    {
        if(x(i) == 1)
        {
            return i;
        }
    }

    return x.n;
}

uint32_t armv7a::ls_instruction_syndrome()
{
    printd(d_armv7a_misc, "ls_instruction_syndrome inst=0x%X", inst.val);
    return 0;
}

void armv7a::lsl(bits* result, const bits& x, uint32_t shift)
{
    pseudo_assert(shift >= 0, "lsl assert error");
    //pseudo_assert(shift < 32, "lsl assert error 2"); //hit but no need

    if(shift == 0)
    {
        (*result) = x;
    }
    else if(shift > 32)
    {
        result->val = 0;
        result->n = x.n;
    }
    else
    {
        uint32_t carry_out;
        lsl_c(result, &carry_out, x, shift);
    }
}

void armv7a::lsl_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift)
{
    pseudo_assert(shift > 0, "lsl_c assert error");
    //pseudo_assert(shift < 32, "lsl_c assert error 2"); //hit

    if(shift > 32)
    {
        result->val = 0;
        result->n = x.n;
        *carry_out = 0;
        return;
    }

    uint64_t extended_x = (uint64_t)x.val << (uint64_t)shift;
    result->val = get_field64(extended_x, x.n - 1, 0);
    result->n = x.n;
    *carry_out = get_bit64(extended_x, x.n);
}

void armv7a::lsr(bits* result, const bits& x, uint32_t shift)
{
    pseudo_assert(shift >= 0, "lsr assert error");
    pseudo_assert(shift < 32, "lsr assert error 2");

    if(shift == 0)
    {
        (*result) = x;
    }
    else
    {
        uint32_t carry_out;
        lsr_c(result, &carry_out, x, shift);
    }
}

void armv7a::lsr_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift)
{
    pseudo_assert(shift > 0, "lsr_c assert error");
    //pseudo_assert(shift < 32, "lsr_c assert error 2"); //hit

    if(shift > 32)
    {
        result->val = 0;
        result->n = x.n;
        *carry_out = 0;
        return;
    }

    uint64_t extended_x = x.val;
    result->val = get_field64(extended_x, shift + x.n - 1, shift);
    result->n = x.n;
    *carry_out = get_bit64(extended_x, shift - 1);
}

void armv7a::null_check_if_thumbee(uint32_t n)
{
    printm_once(d_armv7a_misc, "ignore null_check_if_thumbee");
}

uint32_t armv7a::processor_id()
{
    return core_id;
}

void armv7a::ror(bits* result, const bits& x, uint32_t shift)
{
    if(x.n == 0)
    {
        (*result) = x;
    }
    else
    {
        uint32_t carry_out;
        ror_c(result, &carry_out, x, shift);
    }
}

void armv7a::ror_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t shift)
{
    //pseudo_assert(shift != 0, "ror_c assert error");
    //pseudo_assert(shift < 32, "ror_c assert error");  //hit, but no need?
    if(shift == 0)
    {
        (*result) = x;
        *carry_out = 0;
    }
    else
    {
        uint32_t m = shift % x.n;
        bits r1, r2;
        lsr(&r1, x, m);
        lsl(&r2, x, x.n - m);
        result->val = r1.val | r2.val;
        result->n = x.n;
        printd(d_inst, "x=%X:%d shift:%d lsr_r1:%X lsr_r2:%X result:%X, result.n:%d", x.val, x.n, shift, r1.val, r2.val, result->val, result->n);
        *carry_out = (*result)(x.n - 1);
    }
}

void armv7a::rrx_c(bits* result, uint32_t* carry_out, const bits& x, uint32_t carry_in)
{
    //carry_in = get_bit(carry_in, 0);
    //width:         1-bit                 (n-2) bit
    result->val = (carry_in << (x.n - 1)) | x(x.n - 1, 1);
    result->n = x.n;
    *carry_out = x(0);
}

void armv7a::shift(bits* result, const bits& value, sr_type type, uint32_t amount, uint32_t carry_in)
{
    uint32_t carry_out;
    shift_c(result, &carry_out, value, type, amount, carry_in);
}

void armv7a::shift_c(bits* result, uint32_t* carry_out, const bits& value, sr_type type, uint32_t amount, uint32_t carry_in)
{
    if(amount == 0)
    {
        (*result) = value;
        *carry_out = carry_in;
        printd(d_armv7a_misc, "shift_c amount=0");
    }
    else
    {
        switch(type)
        {
            case SRType_LSL:
                lsl_c(result, carry_out, value, amount);
                break;
            case SRType_LSR:
                lsr_c(result, carry_out, value, amount);
                break;
            case SRType_ASR:
                asr_c(result, carry_out, value, amount);
                break;
            case SRType_ROR:
                ror_c(result, carry_out, value, amount);
                break;
            case SRType_RRX:
                rrx_c(result, carry_out, value, carry_in);
                break;
            default:
                printb(d_armv7a_misc, "shift_c switch error");
        }
    }
}

void armv7a::sign_extend(bits* result, const bits& x, uint32_t i)
{
    if(i > 32)
    {
        i = 32;
    }

    if(x.n == 32)
    {
        (*result) = x;
        return;
    }

    if(x(x.n - 1) == 1)
    {
        result->val = x.val | mask(i - 1, x.n);
    }
    else
    {
        result->val = x.val & mask(x.n - 1, 0);
    }

    result->n = i;
}

uint32_t armv7a::sign_extend32(const bits& x)
{
    //written by ufoderek
    const uint32_t one = 1;
    uint32_t m = one << (x.n - 1);
    return (m ^(uint32_t)x.val) - m;
}

uint64_t armv7a::sign_extend64(const bits& x)
{
    //written by ufoderek
    const uint64_t one = 1;
    uint64_t m = one << (x.n - 1);
    return (m ^(uint64_t)x.val) - m;
}

uint64_t armv7a::SInt(const bits& x)
{
    return sign_extend64(x);
}

uint64_t armv7a::UInt(const bits& x)
{
    //return "unsigned representation of x"
    return (uint64_t)x.val;
}

void armv7a::zero_extend(bits* result, const bits& x, uint32_t i)
{
    result->val = x(x.n - 1, 0);
    result->n = (i > 32) ? 32 : i;
}

