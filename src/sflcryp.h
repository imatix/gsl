/*===========================================================================*
 *                                                                           *
 *  sflcryp.h - simple encryption functions                                  *
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
 *  published by the Free Software Foundation; either version 2 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, write to the Free Software Foundation, Inc., 59 Temple              *
 *  Place - Suite 330, Boston, MA 02111-1307, USA.                           *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/

#ifndef SFLCRYP_INCLUDED               /*  Allow multiple inclusions        */
#define SFLCRYP_INCLUDED


/*  Definitions of the encryption algorithms we support                      */

#define CRYPT_IDEA      0               /*  IDEA algorithm                   */
#define CRYPT_MDC       1               /*  MDC algorithm                    */
#define CRYPT_DES       2               /*  DES algorithm                    */
#define CRYPT_XOR       3               /*  A basic XOR algorithm            */
#define CRYPT_TOP       4               /*  We support 4 algorithms          */

/*  We define some tables that key off the encryption algorithm              */

#if (defined (DEFINE_CRYPT_TABLES))
static int
    crypt_block_size [] = {             /*  Block size for each algorithm    */
       8, 32, 8, 16
    };
#define CRYPT_MAX_BLOCK_SIZE  32        /*  Largest block size, in bytes     */
#endif

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

Bool  crypt_encode  (byte *buffer, word buffer_size, int algorithm, 
                     const byte *key);
Bool  crypt_decode  (byte *buffer, word buffer_size, int algorithm, 
                     const byte *key);

#ifdef __cplusplus
}
#endif

#endif
