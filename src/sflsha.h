/*===========================================================================*
 *                                                                           *
 *  sflsha.c - message digest functions (SHA1)                               *
 *                                                                           *
 *  Note: contributed to the SFL, provenance not yet checked.                *
 *  The copyright belongs to the original authors according to               *
 *  their license agreement.  Parts are copyright (c) 2003 iMatix            *
 *  Corporation                                                              *
 *===========================================================================*/

#ifndef _SFLSHA_INCLUDED                /*  Allow multiple inclusions        */
#define _SFLSHA_INCLUDED

/*- Definitions -------------------------------------------------------------*/

#define SHA_DATA_SIZE   64
#define SHA_DIGEST_SIZE 20


/*- Structure ---------------------------------------------------------------*/

typedef struct {
    qbyte digest [5];                   /* Message digest                    */
    qbyte data   [16];                  /* SHS data buffer                   */
    qbyte count_low;
    qbyte count_hi;                     /* 64-bit bit count                  */
} SHA_CONTEXT;


/*- Function prototypes -----------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void sha_init    (SHA_CONTEXT *context);
void sha_update  (SHA_CONTEXT *context, const byte *input,
                  const qbyte input_length);
void sha_final   (SHA_CONTEXT *context, byte *digest);
byte *sha        (const byte *input, const qbyte input_length, byte *digest);

#ifdef __cplusplus
}
#endif

#endif

