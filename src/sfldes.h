/*===========================================================================*
 *                                                                           *
 *  $(filename) - $(description)                                             *
 *                                                                           *
 *  $(project) $(version)                                                    *
 *  $(copyright)                                                             *
 *                                                                           *
 *  $(license)                                                               *
 *===========================================================================*/
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   The encryption/decryption functions were based on the
                cryptosystem library by Andrew Brown <asb@cs.nott.ac.uk>,
                cleaned-up for portability.  Thanks for a great package.
 ------------------------------------------------------------------</Prolog>-*/
#ifndef _SFLDES_INCLUDED                /*  Allow multiple inclusions        */
#define _SFLDES_INCLUDED

/*- Definitions -------------------------------------------------------------*/

typedef byte des_cblock [8];

/*- Structure ---------------------------------------------------------------*/

typedef struct des_keys_struct
{
    union
     {
        des_cblock _;                   /*  Make sure things are correct     */
        qbyte pad[2];                   /*    on systems with 8 byte longs   */
    } ks;
#   define _    ks._
} des_keys [16];

/*- Function prototypes -----------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

int des_key           (des_cblock *key, des_keys *schedule);
int crypt_des       (des_cblock *input, des_cblock *output,
                       des_keys *ks, int encrypt);
int crypt_des3      (des_cblock *input, des_cblock *output,
                       des_keys *ks1, des_keys *ks2,
                       des_keys *ks3, int encrypt);

#ifdef __cplusplus
}
#endif

#endif
