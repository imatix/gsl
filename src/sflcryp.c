/*===========================================================================*
 *                                                                           *
 *  sflcryp.c - simple encryption functions                                  *
 *                                                                           *
 *  Copyright (c) 1991-2010 iMatix Corporation                               *
 *                                                                           *
 *  ------------------ GPL Licensed Source Code ------------------           *
 *  iMatix makes this software available under the GNU General               *
 *  Public License (GPL) license for open source projects.  For              *
 *  details of the GPL license please see www.gnu.org or read the            *
 *  file license.gpl provided in this package.                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU General Public License as           *
 *  published by the Free Software Foundation; either version 3 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, see <http://www.gnu.org/licenses/>.                                 *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/

#define DEFINE_CRYPT_TABLES

#include "prelude.h"                    /*  Universal header file            */
#include "sflcryp.h"                    /*  Prototypes for functions         */


/*- Local function prototypes -----------------------------------------------*/

static void xor_crypt       (byte *buffer, const byte *key);
static Bool crypt_data      (byte *buffer, word buffer_size, int algorithm,
                             const byte *key, Bool encrypt);


/*- IDEA definitions --------------------------------------------------------*/

static qbyte Mul            (qbyte a, qbyte b);
static dbyte MulInv         (dbyte x);
static void idea            (dbyte *dataIn, dbyte *dataOut, dbyte *key);
static void invert_idea_key (dbyte *key, dbyte *invKey);
static void expand_user_key (dbyte *userKey, dbyte *key);

#define mulMod              0x10001L    /*  2**16 + 1                        */
#define addMod              0x10000L    /*  2**16                            */
#define ones                0xFFFF      /*  2**16 - 1                        */

#define nofKeyPerRound      6           /*  Number of used keys per round    */
#define nofRound            8           /*  Number of rounds                 */

#define dataSize            8           /*  8 bytes = 64 bits                */
#define dataLen             4
#define keySize             104         /*  104 bytes = 832 bits             */
#define keyLen              52
#define userKeySize         16          /*  16 bytes = 128 bits              */
#define userKeyLen          8

#define data_t(v)           dbyte v[dataLen]
#define key_t(v)            dbyte v[keyLen]
#define userkey_t(v)        dbyte v[userKeyLen]


/*- MDC definitions ---------------------------------------------------------*/

static void mdc             (qbyte *out1, qbyte *out2, qbyte *in1,
                             qbyte *in2,  qbyte *key1, qbyte *key2);
static void Transform       (qbyte *buffer, qbyte *in);
static void mdc_encrypt     (qbyte *in, qbyte *out, qbyte *key);
static void mdc_decrypt     (qbyte *in, qbyte *out, qbyte *key);

/*  F, G, H and I are the basic MD5 functions                                */

#define F(x, y, z)          (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z)          (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z)          ((x) ^ (y) ^ (z))
#define I(x, y, z)          ((y) ^ ((x) | (~z)))

/*  Rotate x left n bits                                                     */

#define ROTATE_LEFT(x, n)   (((x) << (n)) | ((x) >> (32-(n))))

/*  FF, GG, HH, and II are transformations for rounds 1, 2, 3, and 4         */
/*  Rotation is separate from addition to prevent recomputation              */

#define FF(a, b, c, d, x, s, ac) {                 \
    (a) += F((b), (c), (d)) + (x) + (qbyte)(ac);   \
    (a)  = ROTATE_LEFT ((a), (s));                 \
    (a) += (b);                                    \
}

#define GG(a, b, c, d, x, s, ac) {                 \
    (a) += G((b), (c), (d)) + (x) + (qbyte)(ac);   \
    (a)  = ROTATE_LEFT ((a), (s));                 \
    (a) += (b);                                    \
}

#define HH(a, b, c, d, x, s, ac) {                 \
    (a) += H((b), (c), (d)) + (x) + (qbyte)(ac);   \
    (a)  = ROTATE_LEFT ((a), (s));                 \
    (a) += (b);                                    \
}

#define II(a, b, c, d, x, s, ac) {                 \
    (a) += I((b), (c), (d)) + (x) + (qbyte)(ac);   \
    (a)  = ROTATE_LEFT ((a), (s));                 \
    (a) += (b);                                    \
}

/*  Initial values for the MD5 Transform hash function                       */

static const qbyte
    ihash[4] = {
        0x67452301L, 0xefcdab89L, 0x98badcfeL, 0x10325476L
    };


/*- DES definitions ---------------------------------------------------------*/

typedef byte des_cblock [8];

typedef struct des_ks_struct
{
    union
     {
        des_cblock _;                   /*  Make sure things are correct     */
        qbyte pad[2];                   /*    on systems with 8 byte longs   */
    } ks;
#   define _    ks._
} des_key_schedule [16];

static int des_encrypt      (qbyte *input, qbyte *output,
                             des_key_schedule *ks, int encrypt);
static int des_set_key      (des_cblock *key,
                             des_key_schedule *schedule);
static int des_ecb_encrypt  (des_cblock *input, des_cblock *output,
                             des_key_schedule *ks, int encrypt);
static int des_key_sched    (des_cblock *key,
                             des_key_schedule *schedule);


/*  ---------------------------------------------------------------------[<]-
    Function: crypt_encode

    Synopsis: Encrypt a buffer with the specified algorithm and specified
    key.  Returns TRUE if the buffer is encrypted sucessfully.  The buffer
    is encrypted in-place.  The algorithm can be one of:
    <Table>
    CRYPT_IDEA      Use IDEA encryption.
    CRYPT_MDC       Use MDC encryption.
    CRYPT_DES       Use DES encryption.
    CRYPT_XOR       Use XOR encryption.
    </Table>
    The minimum buffer size, and key size depends on the algorithm:
    <Table>
    CRYPT_IDEA      Buffer is at least 8 bytes long, key is 16 bytes.
    CRYPT_MDC       Buffer is at least 8 bytes long, key is 8 bytes.
    CRYPT_DES       Buffer is at least 32 bytes long, key is 16 bytes.
    CRYPT_XOR       Buffer is at least 16 bytes long, key is 16 bytes.
    </Table>
    Use crypt_decode() with the same algorithm and key to decrypt the buffer.
    The buffer size must be a multiple of the minimum size shown above.
    If you specify a buffer size of zero the function does nothing but
    returns TRUE.  For portability, the buffer size is limited to 64k.
    ---------------------------------------------------------------------[>]-*/

Bool
crypt_encode (
    byte *buffer,                       /*  Data to encrypt, in-place        */
    word  buffer_size,                  /*  Amount of data to encrypt        */
    int   algorithm,                    /*  What type of encryption          */
    const byte *key)                    /*  Encryption key                   */
{
    return (crypt_data (buffer, buffer_size, algorithm, key, TRUE));
}


/*  ---------------------------------------------------------------------[<]-
    Function: crypt_decode

    Synopsis: Decrypt a buffer that was produced by crypt_encode(). You must
    (obviously, I reckon) use the same algorithm and key as was used to
    encrypt the data.  Returns TRUE if the buffer is decrypted okay.  The
    buffer is encrypted in-place.  The algorithm can be one of:
    <Table>
    CRYPT_IDEA      Use IDEA encryption.
    CRYPT_MDC       Use MDC encryption.
    CRYPT_DES       Use DES encryption.
    CRYPT_XOR       Use XOR encryption.
    </Table>
    The minimum buffer size, and key size depends on the algorithm:
    <Table>
    CRYPT_IDEA      Buffer is at least 8 bytes long, key is 16 bytes.
    CRYPT_MDC       Buffer is at least 8 bytes long, key is 8 bytes.
    CRYPT_DES       Buffer is at least 32 bytes long, key is 16 bytes.
    CRYPT_XOR       Buffer is at least 16 bytes long, key is 16 bytes.
    </Table>
    The buffer size must be a multiple of the minimum size shown above.
    If you specify a buffer size of zero the function does nothing but
    returns TRUE.
    ---------------------------------------------------------------------[>]-*/

Bool
crypt_decode (
    byte *buffer,                       /*  Data to decrypt, in-place        */
    word  buffer_size,                  /*  Amount of data to decrypt        */
    int   algorithm,                    /*  What type of decryption          */
    const byte *key)                    /*  Decryption key                   */
{
    return (crypt_data (buffer, buffer_size, algorithm, key, FALSE));
}


/*  -------------------------------------------------------------------------
    Function: crypt_data - internal

    Synopsis:
    -------------------------------------------------------------------------*/

static Bool
crypt_data (
    byte *buffer,                       /*  Data to process, in-place        */
    word  buffer_size,                  /*  Amount of data to process        */
    int   algorithm,                    /*  What type of encryption          */
    const byte *key,                    /*  Encryption key                   */
    Bool  encrypt)                      /*  TRUE=encrypt, FALSE=decrypt      */
{
    word
        index,
        nbr_blocks;
    byte
        block  [CRYPT_MAX_BLOCK_SIZE];
    des_key_schedule
        ks;
    qbyte
        aligned_block  [CRYPT_MAX_BLOCK_SIZE / 4];
    static qbyte
        aligned_intkey [keySize / 4];
    static byte
        *intkey = (byte *) aligned_intkey;

    ASSERT (buffer);
    ASSERT (key);
    ASSERT (algorithm >= 0 && algorithm < CRYPT_TOP);

    if (buffer_size == 0)               /*  Empty buffer is a special case   */
        return (TRUE);

    nbr_blocks = buffer_size / crypt_block_size [algorithm];
    if ((nbr_blocks * crypt_block_size [algorithm]) != buffer_size)
        return (FALSE);                 /*  We want whole number of blocks   */

    if (algorithm == CRYPT_IDEA)
      {
        expand_user_key ((dbyte *) key, (dbyte *) intkey);
        /*  Invert the key for decryption                                    */
        if (!encrypt)
            invert_idea_key ((dbyte *) intkey, (dbyte *) intkey);

        for (index = 0; index < buffer_size; index += 8)
          {
            idea ((dbyte *) (buffer + index),
                  (dbyte *) block,
                  (dbyte *) intkey);
            memcpy ((buffer + index), block, 8);
          }
      }
    else
    if (algorithm == CRYPT_MDC)
      {
        ASSERT (buffer_size % 4 == 0);
        buffer_size /= 4;

        for (index = 0; index < buffer_size; index += 8)
          {
            if (encrypt)
                mdc_encrypt ((qbyte *) buffer + index, aligned_block,
                                                       aligned_intkey);
            else
                mdc_decrypt ((qbyte *) buffer + index, aligned_block,
                                                       aligned_intkey);

            memcpy ((qbyte *) buffer + index, aligned_block, 32);
          }
      }
    else
    if (algorithm == CRYPT_DES)
      {
        des_key_sched ((des_cblock *) key, (des_key_schedule *) ks);
        for (index = 0; index < buffer_size; index += 8)
            des_ecb_encrypt ((des_cblock *) (buffer + index),
                             (des_cblock *) (buffer + index),
                              (des_key_schedule *) ks, encrypt);
      }
    else
    if (algorithm == CRYPT_XOR)
      {
        for (index = 0; index < buffer_size; index += 16)
            xor_crypt (buffer + index, key);
      }
    return (TRUE);                      /*  So far so good                   */
}


/*  -------------------------------------------------------------------------
    Function: expand_user_key - internal

    Synopsis: expand user key of 128 bits to full key of 832 bits
    -------------------------------------------------------------------------*/

static void
expand_user_key (dbyte *userKey, dbyte *key)
{
    register int
        index;

    for (index = 0; index < userKeyLen; index++)
        key [index] = userKey [index];
    /*  Shifts                                                               */
    for (index = userKeyLen; index < keyLen; index++)
      {
        if ((index + 2) % 8 == 0)       /*  For key [14], key [22], ..       */
            key [index] = ((key [index - 7] & 127) << 9)
                         ^ (key [index - 14]       >> 7);
        else
        if ((index + 1) % 8 == 0)       /*  For key [15], key [23], ..       */
            key [index] = ((key [index - 15] & 127) << 9)
                         ^ (key [index - 14]        >> 7);
        else
            key [index] = ((key [index - 7]  & 127) << 9)
                         ^ (key [index - 6]  >> 7);
      }
}


/*  -------------------------------------------------------------------------
    Function: idea - internal

    Synopsis: encryption and decryption algorithm IDEA
    -------------------------------------------------------------------------*/

static void
idea (dbyte *dataIn, dbyte *dataOut, dbyte *key)
{
    qbyte
        round,
        x0, x1, x2, x3,
        t0, t1;

    x0 = (qbyte) *(dataIn++);
    x1 = (qbyte) *(dataIn++);
    x2 = (qbyte) *(dataIn++);
    x3 = (qbyte) *(dataIn);

    for (round = nofRound; round > 0; round--)
      {
        x0  = Mul (x0,  (qbyte) * (key++));
        x1  =     (x1 + (qbyte) * (key++)) & ones;
        x2  =     (x2 + (qbyte) * (key++)) & ones;
        x3  = Mul (x3,  (qbyte) * (key++));
        t0  = Mul (     (qbyte) * (key++), x0 ^ x2);
        t1  = Mul (     (qbyte) * (key++), (t0 + (x1 ^ x3)) & ones);
        t0  = (t0 + t1) & ones;
        x0 ^= t1;
        x3 ^= t0;
        t0 ^= x1;
        x1  = x2 ^ t1;
        x2  = t0;
      }
    *(dataOut++) = (dbyte) (Mul (x0,  (qbyte) * (key++)));
    *(dataOut++) = (dbyte) (    (x2 + (qbyte) * (key++)) & ones);
    *(dataOut++) = (dbyte) (    (x1 + (qbyte) * (key++)) & ones);
    *(dataOut)   = (dbyte) (Mul (x3,  (qbyte) * key));
}


/*  -------------------------------------------------------------------------
    Function: invert_idea_key - internal

    Synopsis: invert decryption / encrytion key for IDEA
    -------------------------------------------------------------------------*/

static void
invert_idea_key (dbyte *key, dbyte *invKey)
{
    register int
        i;

    key_t (dk);

    dk [nofKeyPerRound * nofRound + 0] = MulInv (*(key++));
    dk [nofKeyPerRound * nofRound + 1] = (dbyte) (addMod - *(key++)) & ones;
    dk [nofKeyPerRound * nofRound + 2] = (dbyte) (addMod - *(key++)) & ones;
    dk [nofKeyPerRound * nofRound + 3] = MulInv (*(key++));
    for (i = nofKeyPerRound * (nofRound - 1); i >= 0; i -= nofKeyPerRound)
      {
        dk [i + 4] =         *(key++);
        dk [i + 5] =         *(key++);
        dk [i + 0] = MulInv (*(key++));
        if (i > 0)
          {
            dk [i + 2] = (dbyte) (addMod - *(key++)) & ones;
            dk [i + 1] = (dbyte) (addMod - *(key++)) & ones;
          }
        else
          {
            dk [i + 1] = (dbyte) (addMod - *(key++)) & ones;
            dk [i + 2] = (dbyte) (addMod - *(key++)) & ones;
          }
        dk [i + 3] = MulInv (*(key++));
      }
    for (i = 0; i < keyLen; i++)
        invKey [i] = dk [i];
}


/*  -------------------------------------------------------------------------
    Function: Mul

    Synopsis: Multiplication (used in IDEA).
    -------------------------------------------------------------------------*/

static qbyte
Mul (qbyte a, qbyte b)
{
    long
        p;
    qbyte
        q;

    if (a == 0)
        p = mulMod - b;
    else
    if (b == 0)
        p = mulMod - a;
    else
      {
        q = a * b;
        p = (q & ones) - (q >> 16);
        if (p <= 0)
            p += mulMod;
      }
    return ((qbyte) (p & ones));
}


/*  -------------------------------------------------------------------------
    Function: MulInv

    Synopsis: compute inverse of 'x' by Euclidean gcd algorithm
             (used in IDEA).
    -------------------------------------------------------------------------*/

static dbyte
MulInv (dbyte x)
{
    long
        n1, n2,
        q,  r,
        b1, b2,
        t;

    if (x == 0)
        return (0);

    n1 = mulMod;
    n2 = (long)x;
    b2 = 1;
    b1 = 0;
    do
      {
        r = (n1 % n2);
        q = (n1 - r) / n2;
        if (r == 0)
          {
            if (b2 < 0)
                b2 = mulMod + b2;
          }
        else
          {
            n1 = n2;
            n2 = r;
            t  = b2;
            b2 = b1 - q * b2;
            b1 = t;
          }
      } while (r != 0);

    return ((dbyte) b2);
}


/*  -------------------------------------------------------------------------
    Function: mdc - internal

    Synopsis: Basic transform for Karn encryption.  Take two 16-byte
    half-buffers, two 48-byte keys (which must be distinct), and use
    the MD5 Transform algorithm to produce two 16-byte output
    half-buffers.

    This is reversible: If we get out1 and out2 from in1, in2, key1, key2,
    then we can get in2 and in1 from out2, out1, key1, key2.

    in1, in2, out1, and out2 should point to 16-byte buffers.
    By convention, in1 and in2 are two halves of a 32-byte input
    buffer, and out1 and out2 are two halves of a 32-byte output
    buffer.

    key1 and key2 should point to 48-byte buffers with different contents.
    -------------------------------------------------------------------------*/

static void
mdc (qbyte *out1, qbyte *out2,
     qbyte *in1,  qbyte *in2,
     qbyte *key1, qbyte *key2)
{
    int
       index;
    qbyte
        buffer [16],
        hash   [4],
        temp   [4];

    memcpy (hash, ihash, sizeof (hash));
    memcpy (buffer, in1, 16);
    memcpy (buffer + 4, key1, 48);
    Transform (hash, buffer);

    for (index = 0; index < 4; ++index)
        temp [index] = buffer [index] = in2 [index] ^ hash [index];

    memcpy (hash, ihash, sizeof (hash));
    memcpy (buffer + 4, key2, 48);
    Transform (hash, buffer);

    for (index = 0; index < 4; ++index)
        out2 [index] = buffer [index] = in1 [index] ^ hash [index];

    memcpy (hash, ihash, sizeof (hash));
    memcpy (buffer + 4, key1, 48);
    Transform (hash, buffer);

    for (index = 0; index < 4; ++index)
        out1 [index] = temp [index] ^ hash [index];
}


/*  -------------------------------------------------------------------------
    Function: Transform -- internal

    Synopsis: Basic MD5 step. Transforms buffer based on in
    -------------------------------------------------------------------------*/

static void
Transform (qbyte *buffer, qbyte *in)
{
    qbyte
        a = buffer [0],
        b = buffer [1],
        c = buffer [2],
        d = buffer [3];

    /*  Round 1  */
#   define S11 7
#   define S12 12
#   define S13 17
#   define S14 22
    FF (a, b, c, d, in [0] , S11, 3614090360UL);   /* 1                      */
    FF (d, a, b, c, in [1] , S12, 3905402710UL);   /* 2                      */
    FF (c, d, a, b, in [2] , S13, 606105819UL);    /* 3                      */
    FF (b, c, d, a, in [3] , S14, 3250441966UL);   /* 4                      */
    FF (a, b, c, d, in [4] , S11, 4118548399UL);   /* 5                      */
    FF (d, a, b, c, in [5] , S12, 1200080426UL);   /* 6                      */
    FF (c, d, a, b, in [6] , S13, 2821735955UL);   /* 7                      */
    FF (b, c, d, a, in [7] , S14, 4249261313UL);   /* 8                      */
    FF (a, b, c, d, in [8] , S11, 1770035416UL);   /* 9                      */
    FF (d, a, b, c, in [9] , S12, 2336552879UL);   /* 10                     */
    FF (c, d, a, b, in [10], S13, 4294925233UL);   /* 11                     */
    FF (b, c, d, a, in [11], S14, 2304563134UL);   /* 12                     */
    FF (a, b, c, d, in [12], S11, 1804603682UL);   /* 13                     */
    FF (d, a, b, c, in [13], S12, 4254626195UL);   /* 14                     */
    FF (c, d, a, b, in [14], S13, 2792965006UL);   /* 15                     */
    FF (b, c, d, a, in [15], S14, 1236535329UL);   /* 16                     */

    /*  Round 2    */
#   define S21 5
#   define S22 9
#   define S23 14
#   define S24 20
    GG (a, b, c, d, in [1] , S21, 4129170786UL);   /* 17                     */
    GG (d, a, b, c, in [6] , S22, 3225465664UL);   /* 18                     */
    GG (c, d, a, b, in [11], S23, 643717713UL);    /* 19                     */
    GG (b, c, d, a, in [0] , S24, 3921069994UL);   /* 20                     */
    GG (a, b, c, d, in [5] , S21, 3593408605UL);   /* 21                     */
    GG (d, a, b, c, in [10], S22, 38016083UL);     /* 22                     */
    GG (c, d, a, b, in [15], S23, 3634488961UL);   /* 23                     */
    GG (b, c, d, a, in [4] , S24, 3889429448UL);   /* 24                     */
    GG (a, b, c, d, in [9] , S21, 568446438UL);    /* 25                     */
    GG (d, a, b, c, in [14], S22, 3275163606UL);   /* 26                     */
    GG (c, d, a, b, in [3] , S23, 4107603335UL);   /* 27                     */
    GG (b, c, d, a, in [8] , S24, 1163531501UL);   /* 28                     */
    GG (a, b, c, d, in [13], S21, 2850285829UL);   /* 29                     */
    GG (d, a, b, c, in [2] , S22, 4243563512UL);   /* 30                     */
    GG (c, d, a, b, in [7] , S23, 1735328473UL);   /* 31                     */
    GG (b, c, d, a, in [12], S24, 2368359562UL);   /* 32                     */

    /* Round 3   */
#   define S31 4
#   define S32 11
#   define S33 16
#   define S34 23
    HH (a, b, c, d, in [5] , S31, 4294588738UL);   /* 33                     */
    HH (d, a, b, c, in [8] , S32, 2272392833UL);   /* 34                     */
    HH (c, d, a, b, in [11], S33, 1839030562UL);   /* 35                     */
    HH (b, c, d, a, in [14], S34, 4259657740UL);   /* 36                     */
    HH (a, b, c, d, in [1] , S31, 2763975236UL);   /* 37                     */
    HH (d, a, b, c, in [4] , S32, 1272893353UL);   /* 38                     */
    HH (c, d, a, b, in [7] , S33, 4139469664UL);   /* 39                     */
    HH (b, c, d, a, in [10], S34, 3200236656UL);   /* 40                     */
    HH (a, b, c, d, in [13], S31, 681279174UL);    /* 41                     */
    HH (d, a, b, c, in [0] , S32, 3936430074UL);   /* 42                     */
    HH (c, d, a, b, in [3] , S33, 3572445317UL);   /* 43                     */
    HH (b, c, d, a, in [6] , S34, 76029189UL);     /* 44                     */
    HH (a, b, c, d, in [9] , S31, 3654602809UL);   /* 45                     */
    HH (d, a, b, c, in [12], S32, 3873151461UL);   /* 46                     */
    HH (c, d, a, b, in [15], S33, 530742520UL);    /* 47                     */
    HH (b, c, d, a, in [2] , S34, 3299628645UL);   /* 48                     */

    /*  Round 4    */
#   define S41 6
#   define S42 10
#   define S43 15
#   define S44 21
    II (a, b, c, d, in [0] , S41, 4096336452UL);   /* 49                     */
    II (d, a, b, c, in [7] , S42, 1126891415UL);   /* 50                     */
    II (c, d, a, b, in [14], S43, 2878612391UL);   /* 51                     */
    II (b, c, d, a, in [5] , S44, 4237533241UL);   /* 52                     */
    II (a, b, c, d, in [12], S41, 1700485571UL);   /* 53                     */
    II (d, a, b, c, in [3] , S42, 2399980690UL);   /* 54                     */
    II (c, d, a, b, in [10], S43, 4293915773UL);   /* 55                     */
    II (b, c, d, a, in [1] , S44, 2240044497UL);   /* 56                     */
    II (a, b, c, d, in [8] , S41, 1873313359UL);   /* 57                     */
    II (d, a, b, c, in [15], S42, 4264355552UL);   /* 58                     */
    II (c, d, a, b, in [6] , S43, 2734768916UL);   /* 59                     */
    II (b, c, d, a, in [13], S44, 1309151649UL);   /* 60                     */
    II (a, b, c, d, in [4] , S41, 4149444226UL);   /* 61                     */
    II (d, a, b, c, in [11], S42, 3174756917UL);   /* 62                     */
    II (c, d, a, b, in [2] , S43,  718787259UL);   /* 63                     */
    II (b, c, d, a, in [9] , S44, 3951481745UL);   /* 64                     */

    buffer [0] += a;
    buffer [1] += b;
    buffer [2] += c;
    buffer [3] += d;
}


/*  -------------------------------------------------------------------------
    Function: mdc_encrypt - internal

    Synopsis: encrypt a buffer using the MDC algorithm.
    -------------------------------------------------------------------------*/

static void
mdc_encrypt (qbyte *in, qbyte *out, qbyte *key)
{
    mdc (out, &out [4],
         in,  &in  [4],
         key, &key [12]);
}


/*  -------------------------------------------------------------------------
    Function: mdc_decrypt - internal

    Synopsis: decrypt a buffer using the MDC algorithm.
    -------------------------------------------------------------------------*/

static void
mdc_decrypt (qbyte *in, qbyte *out, qbyte *key)
{
    mdc (&out [4], out,
         &in  [4], in,
         key,      &key [12]);
}


/*- DES encryption ----------------------------------------------------------*/

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
#define D_ENCRYPT(L,R,S)                      \
        u = (R^s [S]);                        \
        t =  R^s [S + 1];                     \
        t = ((t >> 4) + (t << 28));           \
        L^=     des_SPtrans[1][(word) (t    ) & 0x3f]| \
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

qbyte des_SPtrans [8][64] = {
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



/*  -------------------------------------------------------------------------
    Function: des_ecb_encrypt - internal

    Synopsis: main function for DES cipher

    input   : pointer to an 8 byte block to be encrypted/decrypted
    output  : pointer to an 8 byte block to hold the results of the
             encryption/decryption
    ks      : pointer to the des_key_schedule structure that you were given
              by the des_key_sched() function
    encrypt : TRUE if you encrypting, FALSE if you are decrypting
    -------------------------------------------------------------------------*/

static int
des_ecb_encrypt (des_cblock *input, des_cblock *output,
                 des_key_schedule *ks, int encrypt)
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
    des_encrypt ((qbyte *) ll, (qbyte *) ll, ks, encrypt);
    l0 = ll [0];
    l1 = ll [1];
    l2c (l0, out);
    l2c (l1, out);

    return (0);
}


/*  -------------------------------------------------------------------------
    Function: des_encrypt - internal

    Synopsis:
    -------------------------------------------------------------------------*/

static int
des_encrypt (qbyte *input, qbyte *output, des_key_schedule *ks, int encrypt)
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
des_set_key (des_cblock *key, des_key_schedule *schedule)
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


/*  -------------------------------------------------------------------------
    Function: des_key_sched - internal

    Synopsis: You need to call this function before you use the des_ecb_encrypt
              function to operate on your data. It performs some initial
              operations on your key, presumably to make the operation of the
              cipher faster.

              Always returns zero.

        key   : pointer to a 64 bit key. a `des_cblock' is simply an 8-byte
                unsigned character array so you can just pass the address of
                an 8 bytes array as this parameter.

        sched : pointer to an address of a des_key_schedule structure that the
                function will fill in with the DES key schedule information
                that you will need to pass to the encryption/decryption
                function.

    -------------------------------------------------------------------------*/

static int
des_key_sched (des_cblock *key, des_key_schedule *schedule)
{
    return (des_set_key (key, schedule));
}


/*  -------------------------------------------------------------------------
    Function: xor_crypt - internal

    Synopsis: Encrypts / decrypts a 16-byte block using a 16-byte key.
    -------------------------------------------------------------------------*/

static void
xor_crypt (byte *buffer, const byte *key)
{
    buffer  [0] ^= key  [0];
    buffer  [1] ^= key  [1];
    buffer  [2] ^= key  [2];
    buffer  [3] ^= key  [3];
    buffer  [4] ^= key  [4];
    buffer  [5] ^= key  [5];
    buffer  [6] ^= key  [6];
    buffer  [7] ^= key  [7];
    buffer  [8] ^= key  [8];
    buffer  [9] ^= key  [9];
    buffer [10] ^= key [10];
    buffer [11] ^= key [11];
    buffer [12] ^= key [12];
    buffer [13] ^= key [13];
    buffer [14] ^= key [14];
    buffer [15] ^= key [15];
}

