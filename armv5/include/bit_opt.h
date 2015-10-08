#ifndef _BIT_MASK_H_
#define _BIT_MASK_H_

#include <stdint.h>

/* helper macros */
#define HEX__(n) 0x##n##LU

#define B8__(x)                 \
    ((x&0x0000000FLU) ?   1:0)     \
    +((x&0x000000F0LU) ?   2:0)     \
    +((x&0x00000F00LU) ?   4:0)     \
    +((x&0x0000F000LU) ?   8:0)     \
    +((x&0x000F0000LU) ?  16:0)     \
    +((x&0x00F00000LU) ?  32:0)     \
    +((x&0x0F000000LU) ?  64:0)     \
    +((x&0xF0000000LU) ? 128:0)

/* user macros */
#define B8(d)                   ((uint32_t)B8__(HEX__(d)))
#define B16(dmsb,dlsb)          ( ((uint32_t)B8(dmsb)<<8) + B8(dlsb) )
#define B32(dmsb,db2,db3,dlsb)  \
    ( ((uint32_t)B8(dmsb) << 24)    \
      + ((uint32_t)B8(db2)  << 16)    \
      + ((uint32_t)B8(db3)  <<  8)    \
      + B8(dlsb) )

#define UINT(x)                 ( (uint32_t)(x) )
#define _ONE                    ( UINT(0x00000001) )
#define _MAX                    ( UINT(0xFFFFFFFF) )

// one-bit mask
#define M(i)                    ( _ONE<<UINT(i) )

// multiple-bit mask
#define MM(u, l)                ( ( _MAX>>(UINT(31)-UINT(u)) ) & ( _MAX<<UINT(l) ) )

// mask and shift
#define SMM(x, u, l)            ( ( UINT(x) & MM((u),(l)) ) >> (l) )

// one-bit clear mask
#define CM(x)                   ( ~(M((x))) )
// multiple-bit clear mask
#define CMM(u, l)           ( ~(MM((u),(l))) )

#define _B8F(x, u, l)           ( (UINT(x)<<UINT(l)) & MM(UINT(u),UINT(l)) )
#define B8F(d, u, l)            ( _B8F( B8(d), (u), (l) ) )

const uint32_t  BIT_7_4         = B8(10010000);
const uint32_t  BIT_4           = B8(00010000);
const uint32_t  BIT_24_23_20    = ((B8(11) << 23) | (B8(1) << 20));
const uint32_t  BIT_24          = (B8(1) << 24);

#endif
