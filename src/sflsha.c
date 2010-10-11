/*===========================================================================*
 *                                                                           *
 *  sflsha.c - message digest functions (SHA1)                               *
 *                                                                           *
 *  Note: contributed to the SFL, provenance not yet checked.                *
 *  The copyright belongs to the original authors according to               *
 *  their license agreement.  Parts are copyright (c) 2003 iMatix            *
 *  Corporation                                                              *
 *===========================================================================*/

/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   SHA is a message digest algorithm that can be used to
                condense an arbitrary length message down to a 20 byte hash.

                The functions all need to be passed a SHA_CTX which is used
                to hold the SHA context during multiple sha_update ()
                function calls.

                This library contains the SHA-1 digest algorithm.
 ------------------------------------------------------------------</Prolog>-*/

#include "prelude.h"                    /*  Universal header file            */
#include "sflsha.h"                     /*  Prototypes for functions         */


/*- Definitions -------------------------------------------------------------*/

#define f1(x,y,z) (z ^ (x & (y ^ z)))      /* Rounds  0-19                   */
#define f2(x,y,z) (x ^ y ^ z )             /* Rounds 20-39                   */
#define f3(x,y,z) ((x & y) + (z & (x ^ y)))/* Rounds 40-59                   */
#define f4(x,y,z) (x ^ y ^ z)              /* Rounds 60-79                   */

/* The SHA Mysterious Constants                                              */

#define K1      0x5A827999UL               /* Rounds  0-19                   */
#define K2      0x6ED9EBA1UL               /* Rounds 20-39                   */
#define K3      0x8F1BBCDCUL               /* Rounds 40-59                   */
#define K4      0xCA62C1D6UL               /* Rounds 60-79                   */

#define ROTL(n, X)  (((X) << n) | ((X) >> (32 - n)))

/* The initial expanding function.  The hash function is defined over an
   80-word expanded input array W, where the first 16 are copies of the input
   data, and the remaining 64 are defined by

                W[ i ] = W[ i - 16 ] ^ W[ i - 14 ] ^ W[ i - 8 ] ^ W[ i - 3 ]

   This implementation generates these values on the fly in a circular
   buffer

   The updated SHA changes the expanding function by adding a rotate of 1
   bit.  
*/

#define sha_expand(W,i) (W [i & 15] = ROTL (1, (W [i & 15] ^ W [(i - 14) & 15] ^ \
                                      W [(i - 8) & 15] ^ W [(i - 3) & 15]))) 

/* The prototype SHA sub-round.  The fundamental sub-round is:

                a' = e + ROTL (5, a) + f (b, c, d) + k + data;
                b' = a;
                c' = ROTL (30, b);
                d' = c;
                e' = d;

   but this is implemented by unrolling the loop 5 times and renaming the
   variables ( e, a, b, c, d ) = ( a', b', c', d', e' ) each iteration.
   This code is then replicated 20 times for each of the 4 functions, using
   the next 20 values from the W[] array each time 
*/

#define sha_step(a, b, c, d, e, f, k, data) \
        e =  e + ROTL (5, a) + f (b, c, d) + k + data; \
        b = ROTL( 30, b )

#if (BYTE_ORDER == LITTLE_ENDIAN)

#    define long_order(buffer, count) long_reverse (buffer, count)

static void
long_reverse (qbyte *buffer, qbyte count)
{
    qbyte
        value;

    count /= 4;
    while (count--)
      {
        value = *buffer;
        value = ((value & 0xFF00FF00UL) >> 8) | \
                ((value & 0x00FF00FFUL) << 8);
       *buffer++ = (value << 16) | (value >> 16);
      }
}
#else
#    define long_order(buffer, count) 
#endif


#define long2byte(byte_buffer, data)  {                   \
        byte_buffer [0] = (byte)(((data) >> 24) & 0xFF ); \
        byte_buffer [1] = (byte)(((data) >> 16) & 0xFF ); \
        byte_buffer [2] = (byte)(((data) >> 8)  & 0xFF ); \
        byte_buffer [3] = (byte)(( data)        & 0xFF ); \
        byte_buffer += 4; }


/*  ---------------------------------------------------------------------[<]-
    Function: sha_transform

    Synopsis: Perform the SHA transformation.  Note that this code, like MD5,
              seems to break some optimizing compilers due to the complexity
              of the expressions and the size of the basic block.
              It may be necessary to split it into sections,
              e.g. based on the four subrounds
    ---------------------------------------------------------------------[>]-*/

void
sha_transform (qbyte *digest, qbyte *data)
{
    qbyte
        A, B, C, D, E;                  /* Local vars                        */
    qbyte
        exp_data [16];                  /* Expanded data                     */

    ASSERT (digest);
    ASSERT (data);

    /* Set up first buffer and local data buffer                             */
    A = digest [0];
    B = digest [1];
    C = digest [2];
    D = digest [3];
    E = digest [4];

    memcpy (exp_data, data, SHA_DATA_SIZE);

    /* Heavy mangling, in 4 sub-rounds of 20 interations each.               */
    sha_step (A, B, C, D, E, f1, K1, exp_data [ 0]);
    sha_step (E, A, B, C, D, f1, K1, exp_data [ 1]);
    sha_step (D, E, A, B, C, f1, K1, exp_data [ 2]);
    sha_step (C, D, E, A, B, f1, K1, exp_data [ 3]);
    sha_step (B, C, D, E, A, f1, K1, exp_data [ 4]);
    sha_step (A, B, C, D, E, f1, K1, exp_data [ 5]);
    sha_step (E, A, B, C, D, f1, K1, exp_data [ 6]);
    sha_step (D, E, A, B, C, f1, K1, exp_data [ 7]);
    sha_step (C, D, E, A, B, f1, K1, exp_data [ 8]);
    sha_step (B, C, D, E, A, f1, K1, exp_data [ 9]);
    sha_step (A, B, C, D, E, f1, K1, exp_data [10]);
    sha_step (E, A, B, C, D, f1, K1, exp_data [11]);
    sha_step (D, E, A, B, C, f1, K1, exp_data [12]);
    sha_step (C, D, E, A, B, f1, K1, exp_data [13]);
    sha_step (B, C, D, E, A, f1, K1, exp_data [14]);
    sha_step (A, B, C, D, E, f1, K1, exp_data [15]);
    sha_step (E, A, B, C, D, f1, K1, sha_expand (exp_data, 16));
    sha_step (D, E, A, B, C, f1, K1, sha_expand (exp_data, 17));
    sha_step (C, D, E, A, B, f1, K1, sha_expand (exp_data, 18));
    sha_step (B, C, D, E, A, f1, K1, sha_expand (exp_data, 19));
    sha_step (A, B, C, D, E, f2, K2, sha_expand (exp_data, 20));
    sha_step (E, A, B, C, D, f2, K2, sha_expand (exp_data, 21));
    sha_step (D, E, A, B, C, f2, K2, sha_expand (exp_data, 22));
    sha_step (C, D, E, A, B, f2, K2, sha_expand (exp_data, 23));
    sha_step (B, C, D, E, A, f2, K2, sha_expand (exp_data, 24));
    sha_step (A, B, C, D, E, f2, K2, sha_expand (exp_data, 25));
    sha_step (E, A, B, C, D, f2, K2, sha_expand (exp_data, 26));
    sha_step (D, E, A, B, C, f2, K2, sha_expand (exp_data, 27));
    sha_step (C, D, E, A, B, f2, K2, sha_expand (exp_data, 28));
    sha_step (B, C, D, E, A, f2, K2, sha_expand (exp_data, 29));
    sha_step (A, B, C, D, E, f2, K2, sha_expand (exp_data, 30));
    sha_step (E, A, B, C, D, f2, K2, sha_expand (exp_data, 31));
    sha_step (D, E, A, B, C, f2, K2, sha_expand (exp_data, 32));
    sha_step (C, D, E, A, B, f2, K2, sha_expand (exp_data, 33));
    sha_step (B, C, D, E, A, f2, K2, sha_expand (exp_data, 34));
    sha_step (A, B, C, D, E, f2, K2, sha_expand (exp_data, 35));
    sha_step (E, A, B, C, D, f2, K2, sha_expand (exp_data, 36));
    sha_step (D, E, A, B, C, f2, K2, sha_expand (exp_data, 37));
    sha_step (C, D, E, A, B, f2, K2, sha_expand (exp_data, 38));
    sha_step (B, C, D, E, A, f2, K2, sha_expand (exp_data, 39));
    sha_step (A, B, C, D, E, f3, K3, sha_expand (exp_data, 40));
    sha_step (E, A, B, C, D, f3, K3, sha_expand (exp_data, 41));
    sha_step (D, E, A, B, C, f3, K3, sha_expand (exp_data, 42));
    sha_step (C, D, E, A, B, f3, K3, sha_expand (exp_data, 43));
    sha_step (B, C, D, E, A, f3, K3, sha_expand (exp_data, 44));
    sha_step (A, B, C, D, E, f3, K3, sha_expand (exp_data, 45));
    sha_step (E, A, B, C, D, f3, K3, sha_expand (exp_data, 46));
    sha_step (D, E, A, B, C, f3, K3, sha_expand (exp_data, 47));
    sha_step (C, D, E, A, B, f3, K3, sha_expand (exp_data, 48));
    sha_step (B, C, D, E, A, f3, K3, sha_expand (exp_data, 49));
    sha_step (A, B, C, D, E, f3, K3, sha_expand (exp_data, 50));
    sha_step (E, A, B, C, D, f3, K3, sha_expand (exp_data, 51));
    sha_step (D, E, A, B, C, f3, K3, sha_expand (exp_data, 52));
    sha_step (C, D, E, A, B, f3, K3, sha_expand (exp_data, 53));
    sha_step (B, C, D, E, A, f3, K3, sha_expand (exp_data, 54));
    sha_step (A, B, C, D, E, f3, K3, sha_expand (exp_data, 55));
    sha_step (E, A, B, C, D, f3, K3, sha_expand (exp_data, 56));
    sha_step (D, E, A, B, C, f3, K3, sha_expand (exp_data, 57));
    sha_step (C, D, E, A, B, f3, K3, sha_expand (exp_data, 58));
    sha_step (B, C, D, E, A, f3, K3, sha_expand (exp_data, 59));
    sha_step (A, B, C, D, E, f4, K4, sha_expand (exp_data, 60));
    sha_step (E, A, B, C, D, f4, K4, sha_expand (exp_data, 61));
    sha_step (D, E, A, B, C, f4, K4, sha_expand (exp_data, 62));
    sha_step (C, D, E, A, B, f4, K4, sha_expand (exp_data, 63));
    sha_step (B, C, D, E, A, f4, K4, sha_expand (exp_data, 64));
    sha_step (A, B, C, D, E, f4, K4, sha_expand (exp_data, 65));
    sha_step (E, A, B, C, D, f4, K4, sha_expand (exp_data, 66));
    sha_step (D, E, A, B, C, f4, K4, sha_expand (exp_data, 67));
    sha_step (C, D, E, A, B, f4, K4, sha_expand (exp_data, 68));
    sha_step (B, C, D, E, A, f4, K4, sha_expand (exp_data, 69));
    sha_step (A, B, C, D, E, f4, K4, sha_expand (exp_data, 70));
    sha_step (E, A, B, C, D, f4, K4, sha_expand (exp_data, 71));
    sha_step (D, E, A, B, C, f4, K4, sha_expand (exp_data, 72));
    sha_step (C, D, E, A, B, f4, K4, sha_expand (exp_data, 73));
    sha_step (B, C, D, E, A, f4, K4, sha_expand (exp_data, 74));
    sha_step (A, B, C, D, E, f4, K4, sha_expand (exp_data, 75));
    sha_step (E, A, B, C, D, f4, K4, sha_expand (exp_data, 76));
    sha_step (D, E, A, B, C, f4, K4, sha_expand (exp_data, 77));
    sha_step (C, D, E, A, B, f4, K4, sha_expand (exp_data, 78));
    sha_step (B, C, D, E, A, f4, K4, sha_expand (exp_data, 79));

    /* Build message digest                                                  */
    digest [0] = digest [0] + A;
    digest [1] = digest [1] + B;
    digest [2] = digest [2] + C;
    digest [3] = digest [3] + D;
    digest [4] = digest [4] + E;
}


/*  ---------------------------------------------------------------------[<]-
    Function: sha_init

    Synopsis: Initialise SHA context.
    ---------------------------------------------------------------------[>]-*/

void
sha_init (SHA_CONTEXT *context)
{

    ASSERT (context);
    /* Clear all fields                                                      */
    memset (context, 0, sizeof (SHA_CONTEXT));

    context-> digest [0] = 0x67452301UL;
    context-> digest [1] = 0xEFCDAB89UL;
    context-> digest [2] = 0x98BADCFEUL;
    context-> digest [3] = 0x10325476UL;
    context-> digest [4] = 0xC3D2E1F0UL;

    /* Initialise bit count                                                  */
    context-> count_low = 0;
    context-> count_hi  = 0;
}

/*  ---------------------------------------------------------------------[<]-
    Function: sha_update

    Synopsis: Update SHA for a block of data.
    ---------------------------------------------------------------------[>]-*/

void
sha_update (SHA_CONTEXT *context, const byte *input, const qbyte input_length)
{
    qbyte
        length,
        tmp,
        data_count;
    byte
        *p_input,
        *p_data;

    ASSERT (context);
    ASSERT (input);

    length  = input_length;
    p_input = (byte *)input;

    /* Update bit count                                                      */
    tmp = context-> count_low;
    context-> count_low = tmp + (length << 3); 
    if (context-> count_low < tmp)
        context-> count_hi++;           /* Carry from low to high            */
    context-> count_hi += length >> 29;

    /* Get count of bytes already in data                                    */
    data_count = (int) (tmp >> 3) & 0x3F;

    /* Handle any leading odd-sized chunks                                   */
    if (data_count)
      {
        p_data     = (byte *)context-> data + data_count;
        data_count = SHA_DATA_SIZE - data_count;
        if (length < data_count)
          {
            memcpy (p_data, p_input, length);
            return;
          }
        memcpy        (p_data,   p_input, data_count);
        long_order    (context-> data,    SHA_DATA_SIZE);
        sha_transform (context-> digest,  context-> data);
        p_input += data_count;
        length  -= data_count;
      }

    /* Process data in SHA_DATA_SIZE chunks                                  */
    while (length >= SHA_DATA_SIZE)
      {
        memcpy        (context-> data,   p_input, SHA_DATA_SIZE);
        long_order    (context-> data,   SHA_DATA_SIZE);
        sha_transform (context-> digest, context-> data);
        p_input += SHA_DATA_SIZE;
        length  -= SHA_DATA_SIZE;
      }

    /* Handle any remaining bytes of data.                                   */
    memcpy (context-> data, p_input, length);
}

/*  ---------------------------------------------------------------------[<]-
    Function: sha_final

    Synopsis: Final wrapup - pad to SHA_DATA_SIZE-byte boundary with
              the bit pattern 1 0* (64-bit count of bits processed, MSB-first)

              The result of sha hashing is copy to digest buffer.
    ---------------------------------------------------------------------[>]-*/

void
sha_final (SHA_CONTEXT *context, byte *digest)
{
    int
       index,
       count;
    byte
       *p_data;

    ASSERT (context);
    ASSERT (digest);

    /* Compute number of bytes mod 64                                        */
    count = (int) context-> count_low;
    count = (count >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is   */
    /* always at least one byte free                                         */
    p_data = (byte *)context-> data + count;
    *p_data++ = 0x80;

    /* Bytes of padding needed to make 64 bytes                              */
    count = SHA_DATA_SIZE - 1 - count;

    /* Pad out to 56 mod 64                                                  */
    if (count < 8)
      {
        /* Two lots of padding:  Pad the first block to 64 bytes             */
        memset        (p_data,         0, count);
        long_order    (context-> data, SHA_DATA_SIZE);
        sha_transform (context-> digest, context-> data);

        /* Now fill the next block with 56 bytes                             */
        memset (context-> data, 0, SHA_DATA_SIZE - 8);
      }
    else
    /* Pad block to 56 bytes                                                 */
    memset (p_data, 0, count - 8);

   /* Append length in bits and transform                                    */
    context-> data [14] = context-> count_hi;
    context-> data [15] = context-> count_low;

    long_order    (context-> data,   SHA_DATA_SIZE - 8);
    sha_transform (context-> digest, context-> data);
    for (index = 0; index < 5; index++)
        long2byte (digest, context-> digest [index]);

    /* For security consideration                                            */
    memset (context, 0, sizeof (SHA_CONTEXT));
}


/*  ---------------------------------------------------------------------[<]-
    Function: sha

    Synopsis: main SHA routine. If digest is NULL, use an internal digest.
    Returns the digest of message.
    ---------------------------------------------------------------------[>]-*/

byte *
sha (const byte *input, const qbyte input_length, byte *digest)
{
    SHA_CONTEXT
        context;
    byte
        *p_digest;
    static byte
        tmp_digest [SHA_DIGEST_SIZE];

    p_digest = digest? digest: tmp_digest;

    sha_init   (&context);
    sha_update (&context, input, input_length);
    sha_final  (&context, p_digest);

    return (p_digest);
}

