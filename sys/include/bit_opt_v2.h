#ifndef _BIT_OPT_V2_H_
#define _BIT_OPT_V2_H_

#include <stdint.h>
#include <boost/utility/binary.hpp>

#define B(binary_num) (uint32_t)(BOOST_BINARY_UL(binary_num))

inline const uint32_t mask(uint32_t msb, uint32_t lsb)
{
    const uint32_t t = 0xFFFFFFFF;
    return (t << lsb) & (t>>(31 - msb));
}

inline const uint64_t mask64(uint32_t msb, uint32_t lsb)
{
    const uint64_t t = 0xFFFFFFFFFFFFFFFFULL;
    return (t << lsb) & (t>>(63 - msb));
}

inline const uint32_t mask(uint32_t n)
{
    return mask(n, n);
}

inline const uint64_t mask64(uint32_t n)
{
    return mask64(n, n);
}

inline uint32_t get_field(uint32_t src, uint32_t msb, uint32_t lsb)
{
    return (src & mask(msb, lsb)) >> lsb;
}

inline uint64_t get_field64(uint64_t src, uint32_t msb, uint32_t lsb)
{
    return (src & mask64(msb, lsb)) >> lsb;
}

inline uint32_t get_bit(uint32_t src, uint32_t n)
{
    return get_field(src, n, n);
}

inline uint64_t get_bit64(uint64_t src, uint32_t n)
{
    return get_field64(src, n, n);
}

inline const uint32_t imask(uint32_t msb, uint32_t lsb)
{
    return ~mask(msb, lsb);
}

inline const uint64_t imask64(uint32_t msb, uint32_t lsb)
{
    return ~mask64(msb, lsb);
}

inline const uint32_t imask(const uint32_t n)
{
    return ~mask(n, n);
}

inline const uint64_t imask64(const uint32_t n)
{
    return ~mask64(n, n);
}

inline void set_field(uint32_t* src, uint32_t msb, uint32_t lsb, uint32_t field)
{
    *src = (*src & imask(msb, lsb)) | ((field << lsb)&mask(msb, lsb));
}

inline void set_field64(uint64_t* src, uint32_t msb, uint32_t lsb, uint64_t field)
{
    *src = (*src & imask64(msb, lsb)) | ((field << lsb)&mask64(msb, lsb));
}

inline void set_bit(uint32_t* src, uint32_t n, bool b)
{
    if(b)
    {
        *src |= mask(n);
    }

    else
    {
        *src &= imask(n);
    }
}

inline void copy_field(uint32_t* dst, uint32_t src, uint32_t msb, uint32_t lsb)
{
    set_field(dst, msb, lsb, get_field(src, msb, lsb));
}

inline void copy_bit(uint32_t* dst, uint32_t src, uint32_t n)
{
    set_bit(dst, n, get_bit(src, n));
}

/*
   const uint32_t B0 = ((uint32_t)(0x1)) << 0;
   const uint32_t B1 = ((uint32_t)(0x1)) << 1;
   const uint32_t B2 = ((uint32_t)(0x1)) << 2;
   const uint32_t B3 = ((uint32_t)(0x1)) << 3;
   const uint32_t B4 = ((uint32_t)(0x1)) << 4;
   const uint32_t B5 = ((uint32_t)(0x1)) << 5;
   const uint32_t B6 = ((uint32_t)(0x1)) << 6;
   const uint32_t B7 = ((uint32_t)(0x1)) << 7;
   const uint32_t B8 = ((uint32_t)(0x1)) << 8;
   const uint32_t B9 = ((uint32_t)(0x1)) << 9;

   const uint32_t B10 = ((uint32_t)(0x1)) << 10;
   const uint32_t B11 = ((uint32_t)(0x1)) << 11;
   const uint32_t B12 = ((uint32_t)(0x1)) << 12;
   const uint32_t B13 = ((uint32_t)(0x1)) << 13;
   const uint32_t B14 = ((uint32_t)(0x1)) << 14;
   const uint32_t B15 = ((uint32_t)(0x1)) << 15;
   const uint32_t B16 = ((uint32_t)(0x1)) << 16;
   const uint32_t B17 = ((uint32_t)(0x1)) << 17;
   const uint32_t B18 = ((uint32_t)(0x1)) << 18;
   const uint32_t B19 = ((uint32_t)(0x1)) << 19;

   const uint32_t B20 = ((uint32_t)(0x1)) << 20;
   const uint32_t B21 = ((uint32_t)(0x1)) << 21;
   const uint32_t B22 = ((uint32_t)(0x1)) << 22;
   const uint32_t B23 = ((uint32_t)(0x1)) << 23;
   const uint32_t B24 = ((uint32_t)(0x1)) << 24;
   const uint32_t B25 = ((uint32_t)(0x1)) << 25;
   const uint32_t B26 = ((uint32_t)(0x1)) << 26;
   const uint32_t B27 = ((uint32_t)(0x1)) << 27;
   const uint32_t B28 = ((uint32_t)(0x1)) << 28;
   const uint32_t B29 = ((uint32_t)(0x1)) << 29;

   const uint32_t B30 = ((uint32_t)(0x1)) << 30;
   const uint32_t B31 = ((uint32_t)(0x1)) << 31;
 */

#endif
