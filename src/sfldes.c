/*===========================================================================*
 *                                                                           *
 *  $(filename) - $(description)                                             *
 *                                                                           *
 *  $(project) $(version)                                                    *
 *  $(copyright)                                                             *
 *                                                                           *
 *  $(license)                                                               *
 *===========================================================================*/

#include "prelude.h"                    /*  Universal header file            */
#include "sfldes.h"                     /*  Prototypes for functions         */

#define c2l(c,l)            (l  = ((qbyte) (*((c)++))),       \
                             l |= ((qbyte) (*((c)++))) <<  8, \
                             l |= ((qbyte) (*((c)++))) << 16, \
                             l |= ((qbyte) (*((c)++))) << 24)

#define l2c(l,c)            (*((c)++) = (byte) (((l))      & 0xff), \
                             *((c)++) = (byte) (((l) >> 8) & 0xff), \
                             *((c)++) = (byte) (((l) >>16) & 0xff), \
                             *((c)++) = (byte) (((l) >>24) & 0xff))

/*  The changes to this macro may help or hinder, depending on the
 *  compiler and the achitecture. gcc2 always seems to do well :-).
 *  Inspired by Dana How <how@isl.stanford.edu>
 *  DO NOT use the alternative version on machines with 8 byte longs.        */

#ifdef ALT_ECB
#define D_ENCRYPT(L,R,S) \
        u=((R^s[S])<<2);        \
        t= R^s[S+1]; \
        t=((t>>2)+(t<<30)); \
        L^= \
        *(LPDWORD)(des_SP+0x0100+((t    )&0xfc))+ \
        *(LPDWORD)(des_SP+0x0300+((t>> 8)&0xfc))+ \
        *(LPDWORD)(des_SP+0x0500+((t>>16)&0xfc))+ \
        *(LPDWORD)(des_SP+0x0700+((t>>24)&0xfc))+ \
        *(LPDWORD)(des_SP+       ((u    )&0xfc))+ \
        *(LPDWORD)(des_SP+0x0200+((u>> 8)&0xfc))+ \
        *(LPDWORD)(des_SP+0x0400+((u>>16)&0xfc))+ \
        *(LPDWORD)(des_SP+0x0600+((u>>24)&0xfc));
#else /* original version */
#define D_ENCRYPT(L,R,S)                           \
        u = (R^s [S]);                             \
        t =  R^s [S + 1];                          \
        t = ((t >> 4) + (t << 28));                \
        L^= des_SPtrans[1][(word) (t    ) & 0x3f]| \
            des_SPtrans[3][(word) (t>> 8) & 0x3f]| \
            des_SPtrans[5][(word) (t>>16) & 0x3f]| \
            des_SPtrans[7][(word) (t>>24) & 0x3f]| \
            des_SPtrans[0][(word) (u    ) & 0x3f]| \
            des_SPtrans[2][(word) (u>> 8) & 0x3f]| \
            des_SPtrans[4][(word) (u>>16) & 0x3f]| \
            des_SPtrans[6][(word) (u>>24) & 0x3f];
#endif

    /* IP and FP
     * The problem is more of a geometric problem that random bit fiddling.

         0  1  2  3  4  5  6  7      62 54 46 38 30 22 14  6
         8  9 10 11 12 13 14 15      60 52 44 36 28 20 12  4
        16 17 18 19 20 21 22 23      58 50 42 34 26 18 10  2
        24 25 26 27 28 29 30 31  to  56 48 40 32 24 16  8  0

        32 33 34 35 36 37 38 39      63 55 47 39 31 23 15  7
        40 41 42 43 44 45 46 47      61 53 45 37 29 21 13  5
        48 49 50 51 52 53 54 55      59 51 43 35 27 19 11  3
        56 57 58 59 60 61 62 63      57 49 41 33 25 17  9  1

        The output has been subject to swaps of the form
        0 1 -> 3 1 but the odd and even bits have been put into
        2 3    2 0
        different words.  The main trick is to remember that
        t=((l>>size)^r)&(mask);
        r^=t;
        l^=(t<<size);
        can be used to swap and move bits between words.

        So l =  0  1  2  3  r = 16 17 18 19
                4  5  6  7      20 21 22 23
                8  9 10 11      24 25 26 27
               12 13 14 15      28 29 30 31
        becomes (for size == 2 and mask == 0x3333)
           t =   2^16  3^17 -- --   l =  0  1 16 17  r =  2  3 18 19
                 6^20  7^21 -- --        4  5 20 21       6  7 22 23
                10^24 11^25 -- --        8  9 24 25      10 11 24 25
                14^28 15^29 -- --       12 13 28 29      14 15 28 29

        Thanks for hints from Richard Outerbridge - he told me IP&FP
        could be done in 15 xor, 10 shifts and 5 ands.
        When I finally started to think of the problem in 2D
        I first got ~42 operations without xors.  When I remembered
        how to use xors :-) I got it to its final state.
        */

#define PERM_OP(a,b,t,n,m)                  \
    ((t)  = ((((a) >> (n)) ^ (b)) & (m)),   \
     (b) ^= (t),                            \
     (a) ^= ((t) << (n)))

#define ITERATIONS 16

#define HPERM_OP(a,t,n,m)                      \
    ((t) = ((((a) << (16 - (n))) ^ (a)) & (m)),\
     (a) = (a) ^ (t) ^ (t >> (16 - (n))))

static qbyte des_SPtrans [8][64] = {
    /*  nibble 0  */
  { 0x00820200L, 0x00020000L, 0x80800000L, 0x80820200L,
    0x00800000L, 0x80020200L, 0x80020000L, 0x80800000L,
    0x80020200L, 0x00820200L, 0x00820000L, 0x80000200L,
    0x80800200L, 0x00800000L, 0x00000000L, 0x80020000L,
    0x00020000L, 0x80000000L, 0x00800200L, 0x00020200L,
    0x80820200L, 0x00820000L, 0x80000200L, 0x00800200L,
    0x80000000L, 0x00000200L, 0x00020200L, 0x80820000L,
    0x00000200L, 0x80800200L, 0x80820000L, 0x00000000L,
    0x00000000L, 0x80820200L, 0x00800200L, 0x80020000L,
    0x00820200L, 0x00020000L, 0x80000200L, 0x00800200L,
    0x80820000L, 0x00000200L, 0x00020200L, 0x80800000L,
    0x80020200L, 0x80000000L, 0x80800000L, 0x00820000L,
    0x80820200L, 0x00020200L, 0x00820000L, 0x80800200L,
    0x00800000L, 0x80000200L, 0x80020000L, 0x00000000L,
    0x00020000L, 0x00800000L, 0x80800200L, 0x00820200L,
    0x80000000L, 0x80820000L, 0x00000200L, 0x80020200L },

    /* nibble 1 */
  { 0x10042004L, 0x00000000L, 0x00042000L, 0x10040000L,
    0x10000004L, 0x00002004L, 0x10002000L, 0x00042000L,
    0x00002000L, 0x10040004L, 0x00000004L, 0x10002000L,
    0x00040004L, 0x10042000L, 0x10040000L, 0x00000004L,
    0x00040000L, 0x10002004L, 0x10040004L, 0x00002000L,
    0x00042004L, 0x10000000L, 0x00000000L, 0x00040004L,
    0x10002004L, 0x00042004L, 0x10042000L, 0x10000004L,
    0x10000000L, 0x00040000L, 0x00002004L, 0x10042004L,
    0x00040004L, 0x10042000L, 0x10002000L, 0x00042004L,
    0x10042004L, 0x00040004L, 0x10000004L, 0x00000000L,
    0x10000000L, 0x00002004L, 0x00040000L, 0x10040004L,
    0x00002000L, 0x10000000L, 0x00042004L, 0x10002004L,
    0x10042000L, 0x00002000L, 0x00000000L, 0x10000004L,
    0x00000004L, 0x10042004L, 0x00042000L, 0x10040000L,
    0x10040004L, 0x00040000L, 0x00002004L, 0x10002000L,
    0x10002004L, 0x00000004L, 0x10040000L, 0x00042000L },

    /* nibble 2 */
  { 0x41000000L, 0x01010040L, 0x00000040L, 0x41000040L,
    0x40010000L, 0x01000000L, 0x41000040L, 0x00010040L,
    0x01000040L, 0x00010000L, 0x01010000L, 0x40000000L,
    0x41010040L, 0x40000040L, 0x40000000L, 0x41010000L,
    0x00000000L, 0x40010000L, 0x01010040L, 0x00000040L,
    0x40000040L, 0x41010040L, 0x00010000L, 0x41000000L,
    0x41010000L, 0x01000040L, 0x40010040L, 0x01010000L,
    0x00010040L, 0x00000000L, 0x01000000L, 0x40010040L,
    0x01010040L, 0x00000040L, 0x40000000L, 0x00010000L,
    0x40000040L, 0x40010000L, 0x01010000L, 0x41000040L,
    0x00000000L, 0x01010040L, 0x00010040L, 0x41010000L,
    0x40010000L, 0x01000000L, 0x41010040L, 0x40000000L,
    0x40010040L, 0x41000000L, 0x01000000L, 0x41010040L,
    0x00010000L, 0x01000040L, 0x41000040L, 0x00010040L,
    0x01000040L, 0x00000000L, 0x41010000L, 0x40000040L,
    0x41000000L, 0x40010040L, 0x00000040L, 0x01010000L },

    /* nibble 3 */
  { 0x00100402L, 0x04000400L, 0x00000002L, 0x04100402L,
    0x00000000L, 0x04100000L, 0x04000402L, 0x00100002L,
    0x04100400L, 0x04000002L, 0x04000000L, 0x00000402L,
    0x04000002L, 0x00100402L, 0x00100000L, 0x04000000L,
    0x04100002L, 0x00100400L, 0x00000400L, 0x00000002L,
    0x00100400L, 0x04000402L, 0x04100000L, 0x00000400L,
    0x00000402L, 0x00000000L, 0x00100002L, 0x04100400L,
    0x04000400L, 0x04100002L, 0x04100402L, 0x00100000L,
    0x04100002L, 0x00000402L, 0x00100000L, 0x04000002L,
    0x00100400L, 0x04000400L, 0x00000002L, 0x04100000L,
    0x04000402L, 0x00000000L, 0x00000400L, 0x00100002L,
    0x00000000L, 0x04100002L, 0x04100400L, 0x00000400L,
    0x04000000L, 0x04100402L, 0x00100402L, 0x00100000L,
    0x04100402L, 0x00000002L, 0x04000400L, 0x00100402L,
    0x00100002L, 0x00100400L, 0x04100000L, 0x04000402L,
    0x00000402L, 0x04000000L, 0x04000002L, 0x04100400L },

    /* nibble 4 */
  { 0x02000000L, 0x00004000L, 0x00000100L, 0x02004108L,
    0x02004008L, 0x02000100L, 0x00004108L, 0x02004000L,
    0x00004000L, 0x00000008L, 0x02000008L, 0x00004100L,
    0x02000108L, 0x02004008L, 0x02004100L, 0x00000000L,
    0x00004100L, 0x02000000L, 0x00004008L, 0x00000108L,
    0x02000100L, 0x00004108L, 0x00000000L, 0x02000008L,
    0x00000008L, 0x02000108L, 0x02004108L, 0x00004008L,
    0x02004000L, 0x00000100L, 0x00000108L, 0x02004100L,
    0x02004100L, 0x02000108L, 0x00004008L, 0x02004000L,
    0x00004000L, 0x00000008L, 0x02000008L, 0x02000100L,
    0x02000000L, 0x00004100L, 0x02004108L, 0x00000000L,
    0x00004108L, 0x02000000L, 0x00000100L, 0x00004008L,
    0x02000108L, 0x00000100L, 0x00000000L, 0x02004108L,
    0x02004008L, 0x02004100L, 0x00000108L, 0x00004000L,
    0x00004100L, 0x02004008L, 0x02000100L, 0x00000108L,
    0x00000008L, 0x00004108L, 0x02004000L, 0x02000008L },

    /* nibble 5 */
  { 0x20000010L, 0x00080010L, 0x00000000L, 0x20080800L,
    0x00080010L, 0x00000800L, 0x20000810L, 0x00080000L,
    0x00000810L, 0x20080810L, 0x00080800L, 0x20000000L,
    0x20000800L, 0x20000010L, 0x20080000L, 0x00080810L,
    0x00080000L, 0x20000810L, 0x20080010L, 0x00000000L,
    0x00000800L, 0x00000010L, 0x20080800L, 0x20080010L,
    0x20080810L, 0x20080000L, 0x20000000L, 0x00000810L,
    0x00000010L, 0x00080800L, 0x00080810L, 0x20000800L,
    0x00000810L, 0x20000000L, 0x20000800L, 0x00080810L,
    0x20080800L, 0x00080010L, 0x00000000L, 0x20000800L,
    0x20000000L, 0x00000800L, 0x20080010L, 0x00080000L,
    0x00080010L, 0x20080810L, 0x00080800L, 0x00000010L,
    0x20080810L, 0x00080800L, 0x00080000L, 0x20000810L,
    0x20000010L, 0x20080000L, 0x00080810L, 0x00000000L,
    0x00000800L, 0x20000010L, 0x20000810L, 0x20080800L,
    0x20080000L, 0x00000810L, 0x00000010L, 0x20080010L },

    /* nibble 6 */
  { 0x00001000L, 0x00000080L, 0x00400080L, 0x00400001L,
    0x00401081L, 0x00001001L, 0x00001080L, 0x00000000L,
    0x00400000L, 0x00400081L, 0x00000081L, 0x00401000L,
    0x00000001L, 0x00401080L, 0x00401000L, 0x00000081L,
    0x00400081L, 0x00001000L, 0x00001001L, 0x00401081L,
    0x00000000L, 0x00400080L, 0x00400001L, 0x00001080L,
    0x00401001L, 0x00001081L, 0x00401080L, 0x00000001L,
    0x00001081L, 0x00401001L, 0x00000080L, 0x00400000L,
    0x00001081L, 0x00401000L, 0x00401001L, 0x00000081L,
    0x00001000L, 0x00000080L, 0x00400000L, 0x00401001L,
    0x00400081L, 0x00001081L, 0x00001080L, 0x00000000L,
    0x00000080L, 0x00400001L, 0x00000001L, 0x00400080L,
    0x00000000L, 0x00400081L, 0x00400080L, 0x00001080L,
    0x00000081L, 0x00001000L, 0x00401081L, 0x00400000L,
    0x00401080L, 0x00000001L, 0x00001001L, 0x00401081L,
    0x00400001L, 0x00401080L, 0x00401000L, 0x00001001L },

    /* nibble 7 */
  { 0x08200020L, 0x08208000L, 0x00008020L, 0x00000000L,
    0x08008000L, 0x00200020L, 0x08200000L, 0x08208020L,
    0x00000020L, 0x08000000L, 0x00208000L, 0x00008020L,
    0x00208020L, 0x08008020L, 0x08000020L, 0x08200000L,
    0x00008000L, 0x00208020L, 0x00200020L, 0x08008000L,
    0x08208020L, 0x08000020L, 0x00000000L, 0x00208000L,
    0x08000000L, 0x00200000L, 0x08008020L, 0x08200020L,
    0x00200000L, 0x00008000L, 0x08208000L, 0x00000020L,
    0x00200000L, 0x00008000L, 0x08000020L, 0x08208020L,
    0x00008020L, 0x08000000L, 0x00000000L, 0x00208000L,
    0x08200020L, 0x08008020L, 0x08008000L, 0x00200020L,
    0x08208000L, 0x00000020L, 0x00200020L, 0x08008000L,
    0x08208020L, 0x00200000L, 0x08200000L, 0x08000020L,
    0x00208000L, 0x00008020L, 0x08008020L, 0x08200000L,
    0x00000020L, 0x08208000L, 0x00208020L, 0x00000000L,
    0x08000000L, 0x08200020L, 0x00008000L, 0x00208020L } };

qbyte des_skb [8][64] = {
    /*  For C bits (numbered as per FIPS 46) 1 2 3 4 5 6                     */
  { 0x00000000L, 0x00000010L, 0x20000000L, 0x20000010L,
    0x00010000L, 0x00010010L, 0x20010000L, 0x20010010L,
    0x00000800L, 0x00000810L, 0x20000800L, 0x20000810L,
    0x00010800L, 0x00010810L, 0x20010800L, 0x20010810L,
    0x00000020L, 0x00000030L, 0x20000020L, 0x20000030L,
    0x00010020L, 0x00010030L, 0x20010020L, 0x20010030L,
    0x00000820L, 0x00000830L, 0x20000820L, 0x20000830L,
    0x00010820L, 0x00010830L, 0x20010820L, 0x20010830L,
    0x00080000L, 0x00080010L, 0x20080000L, 0x20080010L,
    0x00090000L, 0x00090010L, 0x20090000L, 0x20090010L,
    0x00080800L, 0x00080810L, 0x20080800L, 0x20080810L,
    0x00090800L, 0x00090810L, 0x20090800L, 0x20090810L,
    0x00080020L, 0x00080030L, 0x20080020L, 0x20080030L,
    0x00090020L, 0x00090030L, 0x20090020L, 0x20090030L,
    0x00080820L, 0x00080830L, 0x20080820L, 0x20080830L,
    0x00090820L, 0x00090830L, 0x20090820L, 0x20090830L },

    /*  For C bits (numbered as per FIPS 46) 7 8 10 11 12 13                 */
  { 0x00000000L, 0x02000000L, 0x00002000L, 0x02002000L,
    0x00200000L, 0x02200000L, 0x00202000L, 0x02202000L,
    0x00000004L, 0x02000004L, 0x00002004L, 0x02002004L,
    0x00200004L, 0x02200004L, 0x00202004L, 0x02202004L,
    0x00000400L, 0x02000400L, 0x00002400L, 0x02002400L,
    0x00200400L, 0x02200400L, 0x00202400L, 0x02202400L,
    0x00000404L, 0x02000404L, 0x00002404L, 0x02002404L,
    0x00200404L, 0x02200404L, 0x00202404L, 0x02202404L,
    0x10000000L, 0x12000000L, 0x10002000L, 0x12002000L,
    0x10200000L, 0x12200000L, 0x10202000L, 0x12202000L,
    0x10000004L, 0x12000004L, 0x10002004L, 0x12002004L,
    0x10200004L, 0x12200004L, 0x10202004L, 0x12202004L,
    0x10000400L, 0x12000400L, 0x10002400L, 0x12002400L,
    0x10200400L, 0x12200400L, 0x10202400L, 0x12202400L,
    0x10000404L, 0x12000404L, 0x10002404L, 0x12002404L,
    0x10200404L, 0x12200404L, 0x10202404L, 0x12202404L },

    /*  For C bits (numbered as per FIPS 46) 14 15 16 17 19 20               */
  { 0x00000000L, 0x00000001L, 0x00040000L, 0x00040001L,
    0x01000000L, 0x01000001L, 0x01040000L, 0x01040001L,
    0x00000002L, 0x00000003L, 0x00040002L, 0x00040003L,
    0x01000002L, 0x01000003L, 0x01040002L, 0x01040003L,
    0x00000200L, 0x00000201L, 0x00040200L, 0x00040201L,
    0x01000200L, 0x01000201L, 0x01040200L, 0x01040201L,
    0x00000202L, 0x00000203L, 0x00040202L, 0x00040203L,
    0x01000202L, 0x01000203L, 0x01040202L, 0x01040203L,
    0x08000000L, 0x08000001L, 0x08040000L, 0x08040001L,
    0x09000000L, 0x09000001L, 0x09040000L, 0x09040001L,
    0x08000002L, 0x08000003L, 0x08040002L, 0x08040003L,
    0x09000002L, 0x09000003L, 0x09040002L, 0x09040003L,
    0x08000200L, 0x08000201L, 0x08040200L, 0x08040201L,
    0x09000200L, 0x09000201L, 0x09040200L, 0x09040201L,
    0x08000202L, 0x08000203L, 0x08040202L, 0x08040203L,
    0x09000202L, 0x09000203L, 0x09040202L, 0x09040203L },

    /*  For C bits (numbered as per FIPS 46) 21 23 24 26 27 28               */
  { 0x00000000L, 0x00100000L, 0x00000100L, 0x00100100L,
    0x00000008L, 0x00100008L, 0x00000108L, 0x00100108L,
    0x00001000L, 0x00101000L, 0x00001100L, 0x00101100L,
    0x00001008L, 0x00101008L, 0x00001108L, 0x00101108L,
    0x04000000L, 0x04100000L, 0x04000100L, 0x04100100L,
    0x04000008L, 0x04100008L, 0x04000108L, 0x04100108L,
    0x04001000L, 0x04101000L, 0x04001100L, 0x04101100L,
    0x04001008L, 0x04101008L, 0x04001108L, 0x04101108L,
    0x00020000L, 0x00120000L, 0x00020100L, 0x00120100L,
    0x00020008L, 0x00120008L, 0x00020108L, 0x00120108L,
    0x00021000L, 0x00121000L, 0x00021100L, 0x00121100L,
    0x00021008L, 0x00121008L, 0x00021108L, 0x00121108L,
    0x04020000L, 0x04120000L, 0x04020100L, 0x04120100L,
    0x04020008L, 0x04120008L, 0x04020108L, 0x04120108L,
    0x04021000L, 0x04121000L, 0x04021100L, 0x04121100L,
    0x04021008L, 0x04121008L, 0x04021108L, 0x04121108L },

    /*  For D bits (numbered as per FIPS 46) 1 2 3 4 5 6                     */
  { 0x00000000L, 0x10000000L, 0x00010000L, 0x10010000L,
    0x00000004L, 0x10000004L, 0x00010004L, 0x10010004L,
    0x20000000L, 0x30000000L, 0x20010000L, 0x30010000L,
    0x20000004L, 0x30000004L, 0x20010004L, 0x30010004L,
    0x00100000L, 0x10100000L, 0x00110000L, 0x10110000L,
    0x00100004L, 0x10100004L, 0x00110004L, 0x10110004L,
    0x20100000L, 0x30100000L, 0x20110000L, 0x30110000L,
    0x20100004L, 0x30100004L, 0x20110004L, 0x30110004L,
    0x00001000L, 0x10001000L, 0x00011000L, 0x10011000L,
    0x00001004L, 0x10001004L, 0x00011004L, 0x10011004L,
    0x20001000L, 0x30001000L, 0x20011000L, 0x30011000L,
    0x20001004L, 0x30001004L, 0x20011004L, 0x30011004L,
    0x00101000L, 0x10101000L, 0x00111000L, 0x10111000L,
    0x00101004L, 0x10101004L, 0x00111004L, 0x10111004L,
    0x20101000L, 0x30101000L, 0x20111000L, 0x30111000L,
    0x20101004L, 0x30101004L, 0x20111004L, 0x30111004L },

    /*  For D bits (numbered as per FIPS 46) 8 9 11 12 13 14                 */
  { 0x00000000L, 0x08000000L, 0x00000008L, 0x08000008L,
    0x00000400L, 0x08000400L, 0x00000408L, 0x08000408L,
    0x00020000L, 0x08020000L, 0x00020008L, 0x08020008L,
    0x00020400L, 0x08020400L, 0x00020408L, 0x08020408L,
    0x00000001L, 0x08000001L, 0x00000009L, 0x08000009L,
    0x00000401L, 0x08000401L, 0x00000409L, 0x08000409L,
    0x00020001L, 0x08020001L, 0x00020009L, 0x08020009L,
    0x00020401L, 0x08020401L, 0x00020409L, 0x08020409L,
    0x02000000L, 0x0A000000L, 0x02000008L, 0x0A000008L,
    0x02000400L, 0x0A000400L, 0x02000408L, 0x0A000408L,
    0x02020000L, 0x0A020000L, 0x02020008L, 0x0A020008L,
    0x02020400L, 0x0A020400L, 0x02020408L, 0x0A020408L,
    0x02000001L, 0x0A000001L, 0x02000009L, 0x0A000009L,
    0x02000401L, 0x0A000401L, 0x02000409L, 0x0A000409L,
    0x02020001L, 0x0A020001L, 0x02020009L, 0x0A020009L,
    0x02020401L, 0x0A020401L, 0x02020409L, 0x0A020409L },

    /*  For D bits (numbered as per FIPS 46) 16 17 18 19 20 21               */
  { 0x00000000L, 0x00000100L, 0x00080000L, 0x00080100L,
    0x01000000L, 0x01000100L, 0x01080000L, 0x01080100L,
    0x00000010L, 0x00000110L, 0x00080010L, 0x00080110L,
    0x01000010L, 0x01000110L, 0x01080010L, 0x01080110L,
    0x00200000L, 0x00200100L, 0x00280000L, 0x00280100L,
    0x01200000L, 0x01200100L, 0x01280000L, 0x01280100L,
    0x00200010L, 0x00200110L, 0x00280010L, 0x00280110L,
    0x01200010L, 0x01200110L, 0x01280010L, 0x01280110L,
    0x00000200L, 0x00000300L, 0x00080200L, 0x00080300L,
    0x01000200L, 0x01000300L, 0x01080200L, 0x01080300L,
    0x00000210L, 0x00000310L, 0x00080210L, 0x00080310L,
    0x01000210L, 0x01000310L, 0x01080210L, 0x01080310L,
    0x00200200L, 0x00200300L, 0x00280200L, 0x00280300L,
    0x01200200L, 0x01200300L, 0x01280200L, 0x01280300L,
    0x00200210L, 0x00200310L, 0x00280210L, 0x00280310L,
    0x01200210L, 0x01200310L, 0x01280210L, 0x01280310L },

    /*  For D bits (numbered as per FIPS 46) 22 23 24 25 27 28               */
  { 0x00000000L, 0x04000000L, 0x00040000L, 0x04040000L,
    0x00000002L, 0x04000002L, 0x00040002L, 0x04040002L,
    0x00002000L, 0x04002000L, 0x00042000L, 0x04042000L,
    0x00002002L, 0x04002002L, 0x00042002L, 0x04042002L,
    0x00000020L, 0x04000020L, 0x00040020L, 0x04040020L,
    0x00000022L, 0x04000022L, 0x00040022L, 0x04040022L,
    0x00002020L, 0x04002020L, 0x00042020L, 0x04042020L,
    0x00002022L, 0x04002022L, 0x00042022L, 0x04042022L,
    0x00000800L, 0x04000800L, 0x00040800L, 0x04040800L,
    0x00000802L, 0x04000802L, 0x00040802L, 0x04040802L,
    0x00002800L, 0x04002800L, 0x00042800L, 0x04042800L,
    0x00002802L, 0x04002802L, 0x00042802L, 0x04042802L,
    0x00000820L, 0x04000820L, 0x00040820L, 0x04040820L,
    0x00000822L, 0x04000822L, 0x00040822L, 0x04040822L,
    0x00002820L, 0x04002820L, 0x00042820L, 0x04042820L,
    0x00002822L, 0x04002822L, 0x00042822L, 0x04042822L } };


static int des_encrypt_block (qbyte *input, qbyte *output, des_keys *ks,
                        int encrypt);
static int des_set_key (des_cblock *key, des_keys *schedule);

/*  ---------------------------------------------------------------------[<]-
    Function: des_key - internal

    Synopsis: You need to call this function before you use the des encrypt
              function to operate on your data. It performs some initial
              operations on your key, presumably to make the operation of the
              cipher faster.

              Always returns zero.

        key   : pointer to a 64 bit key. a `des_cblock' is simply an 8-byte
                unsigned character array so you can just pass the address of
                an 8 bytes array as this parameter.

        sched : pointer to an address of a des_keys structure that the
                function will fill in with the DES key schedule information
                that you will need to pass to the encryption/decryption
                function.

    ---------------------------------------------------------------------[>]-*/

int
des_key (des_cblock *key, des_keys *schedule)
{
    return (des_set_key (key, schedule));
}


/*  ---------------------------------------------------------------------[<]-
    Function: crypt_des

    Synopsis: main function for DES cipher

    input   : pointer to an 8 byte block to be encrypted/decrypted
    output  : pointer to an 8 byte block to hold the results of the
             encryption/decryption
    ks      : pointer to the des_keys structure that you were given
              by the des_key() function
    encrypt : TRUE if you encrypting, FALSE if you are decrypting
    ---------------------------------------------------------------------[>]-*/

int
crypt_des (des_cblock *input, des_cblock *output,
                 des_keys *ks, int encrypt)
{
    static qbyte
        l0, l1,
        ll [2];
    static byte
        *in,
        *out;

    in  = (byte *)input;
    out = (byte *)output;
    c2l (in,l0);
    c2l (in,l1);
    ll [0] = l0;
    ll [1] = l1;
    des_encrypt_block ((qbyte *) ll, (qbyte *) ll, ks, encrypt);
    l0 = ll [0];
    l1 = ll [1];
    l2c (l0, out);
    l2c (l1, out);

    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: crypt_des3

    Synopsis: DES ECB3 cipher

    input   : pointer to an 8 byte block to be encrypted/decrypted
    output  : pointer to an 8 byte block to hold the results of the
             encryption/decryption
    ks      : pointer to the des_keys structure that you were given
              by the des_key() function
    encrypt : TRUE if you encrypting, FALSE if you are decrypting
    ---------------------------------------------------------------------[>]-*/

int
crypt_des3 (des_cblock *input, des_cblock *output,
                 des_keys *ks1, des_keys *ks2,
                 des_keys *ks3, int encrypt)
{
    if (encrypt)
      {
        crypt_des (input,  output, ks1, TRUE);
        crypt_des (output, output, ks2, FALSE);
        crypt_des (output, output, ks3, TRUE);
      }
    else
      {
        crypt_des (input,  output, ks3, FALSE);
        crypt_des (output, output, ks2, TRUE);
        crypt_des (output, output, ks1, FALSE);
      }
    return (0);
}


/*  -------------------------------------------------------------------------
    Function: des_encrypt_block - internal

    Synopsis:
    -------------------------------------------------------------------------*/

static int
des_encrypt_block (qbyte *input, qbyte *output, des_keys *ks, int encrypt)
{
    static qbyte
        l, r, *s, t, u;
#ifdef ALT_ECB
    static byte
        *des_SP = (byte *) des_SPtrans;
#endif
    static int
        i;

    l = input [0];
    r = input [1];

    /*  do IP */
    PERM_OP (r, l, t, 4 , 0x0f0f0f0fL);
    PERM_OP (l, r, t, 16, 0x0000ffffL);
    PERM_OP (r, l, t, 2 , 0x33333333L);
    PERM_OP (l, r, t, 8 , 0x00ff00ffL);
    PERM_OP (r, l, t, 1 , 0x55555555L);

    t = (r << 1)|(r >> 31);
    r = (l << 1)|(l >> 31);
    l = t;

    /*  Clear the top bits on machines with 8byte longs                      */
    l &= 0xffffffffL;
    r &= 0xffffffffL;

    s = (qbyte *) ks;

    if (encrypt)
      {
        for (i = 0; i < 32; i += 4)
          {
            D_ENCRYPT (l, r, i + 0);    /*  1                                */
            D_ENCRYPT (r, l, i + 2);    /*  2                                */
          }
      }
    else
      {
        for (i = 30; i > 0; i -= 4)
          {
            D_ENCRYPT (l, r, i - 0);    /*  16                               */
            D_ENCRYPT (r, l, i - 2);    /*  15                               */
          }
      }
    l = (l >> 1)|(l << 31);
    r = (r >> 1)|(r << 31);

    /*  Clear the top bits on machines with 8byte longs                      */
    l &= 0xffffffffL;
    r &= 0xffffffffL;

    PERM_OP (r, l, t, 1 , 0x55555555L);
    PERM_OP (l, r, t, 8 , 0x00ff00ffL);
    PERM_OP (r, l, t, 2 , 0x33333333L);
    PERM_OP (l, r, t, 16, 0x0000ffffL);
    PERM_OP (r, l, t, 4 , 0x0f0f0f0fL);

    output [0] = l;
    output [1] = r;

    return(0);
}


/*  -------------------------------------------------------------------------
    Function: des_set_key - internal

    Synopsis:
    -------------------------------------------------------------------------*/

static int
des_set_key (des_cblock *key, des_keys *schedule)
{
    static qbyte
        c, d, t, s, *k;
    static byte
        *in;
    static int
        index;
    static char
        shifts2 [16] = {0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0};

    k  = (qbyte *) schedule;
    in = (byte *) key;

    c2l (in, c);
    c2l (in, d);

    PERM_OP  (d, c, t , 4, 0x0f0f0f0fL);
    HPERM_OP (c, t, -2,    0xcccc0000L);
    HPERM_OP (d, t, -2,    0xcccc0000L);
    PERM_OP  (d, c, t , 1, 0x55555555L);
    PERM_OP  (c, d, t , 8, 0x00ff00ffL);
    PERM_OP  (d, c, t , 1, 0x55555555L);
    d =    (((d & 0x000000ffL) << 16)| (d & 0x0000ff00L)      |
            ((d & 0x00ff0000L) >> 16)|((c & 0xf0000000L) >> 4));
    c &= 0x0fffffffL;

    for (index = 0; index < ITERATIONS; index++)
      {
        if (shifts2 [index])
          {
            c = ((c >> 2) | (c << 26));
            d = ((d >> 2) | (d << 26));
          }
        else
          {
            c = ((c >> 1) | (c << 27));
            d = ((d >> 1) | (d << 27));
          }
        c &= 0x0fffffffL;
        d &= 0x0fffffffL;

        /*  Could be a few less shifts but I am to lazy at this point in     */
        /*  time to investigate                                              */
        s = des_skb [0] [(word) ((c)        & 0x3f                      )] |
            des_skb [1] [(word) (((c >>  6) & 0x03) | ((c >>  7) & 0x3c))] |
            des_skb [2] [(word) (((c >> 13) & 0x0f) | ((c >> 14) & 0x30))] |
            des_skb [3] [(word) (((c >> 20) & 0x01) | ((c >> 21) & 0x06)   |
                                                      ((c >> 22) & 0x38))];

        t = des_skb [4] [(word) (((d)       & 0x3f)                     )] |
            des_skb [5] [(word) (((d >>  7) & 0x03) | ((d >>  8) & 0x3c))] |
            des_skb [6] [(word) (((d >> 15) & 0x3f)                     )] |
            des_skb [7] [(word) (((d >> 21) & 0x0f) | ((d >> 22) & 0x30))];

        /*  Table contained 0213 4657                                        */
        *(k++) = ((t << 16) | (s & 0x0000ffffL)) & 0xffffffffL;
        s      = ((s >> 16) | (t & 0xffff0000L));
        s      =  (s << 4)  | (s >> 28);
        *(k++) = s & 0xffffffffL;
      }
    return(0);
}
