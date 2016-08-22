/*===========================================================================*
 *                                                                           *
 *  smtrdnsl.h - Reverse-DNS functions                                       *
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

#ifndef _SMTRDNSL_INCLUDED
#define _SMTRDNSL_INCLUDED


/*******************************************************/
/* byte order definition must be included in prelude.h */
/*******************************************************/

#ifndef BYTE_ORDER
#define LITTLE_ENDIAN   1234            /* least-significant first (vax, pc) */
#define BIG_ENDIAN      4321            /* most-significant first (IBM, net) */
#define PDP_ENDIAN      3412            /* LSB first in word, MSW first in
                                           long (pdp)                        */
#if defined (__MSDOS__) || defined (__OS2__) || defined (vax) || \
    defined (ns32000) || defined (sun386)  || defined (__QNX__) || \
    defined (MIPSEL)  || defined (_MIPSEL) || \
    defined (BIT_ZERO_ON_RIGHT) || defined (__alpha__) || defined(__alpha) || \
    defined (i386)    || defined (__i386)
#   define BYTE_ORDER      LITTLE_ENDIAN
#elif defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k)
#   define BYTE_ORDER      BIG_ENDIAN
#endif

#endif /* BYTE_ORDER */
#if !defined(BYTE_ORDER) || \
    (BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN && \
    BYTE_ORDER != PDP_ENDIAN)
        /* you must determine what the correct bit order is for
         * your compiler - the next line is an intentional error
         * which will force your compiles to bomb until you fix
         * the above macros.
         */
#  error "Undefined or invalid BYTE_ORDER";
#endif

/*- Defines -----------------------------------------------------------------*/

#define MAX_NS             3            /* Max name servers                  */
#define REQUEST_TIMEOUT    10           /* Min. seconds between request      */
#define RECURSIVE_TIMEOUT  40           /* sec. between recursive request    */
#define QUERY_TIMEOUT      60           /* Query time out (1 min.)           */
#define CONNECT_TIMEOUT    5            /* Max time to connect DN server     */
#define REFRESH_CACHE_TIME 600          /* Refresh cache time (10 min.)      */
#define BAD_RESULT_TTL     120          /* Time To live in cache for bad     *
                                         * result of request                 */

#define REQ_TYPE_HOST      1            /* Request type host name            */
#define REQ_TYPE_IP        2            /* Request type ip address           */

/* Define constants based on RFC 883, RFC 1034, RFC 1035                     */

#define NS_PACKET_SIZE     512          /* Maximum packet size               */
#define NS_MAX_DNAME       1025         /* Maximum domain name               */
#define NS_MAX_CDNAME      255          /* Maximum compressed domain name    */
#define NS_MAX_LABEL       63           /* Maximum length of domain label    */
#define NS_HEAD_FIXED_SIZE 12           /* Size of fixed data in header      */
#define NS_QRY_FIXED_SIZE  4            /* Size of fixed data in query       */
#define NS_RR_FIXED_SIZE   10           /* Size of fixed data in r record    */
#define NS_INT32_SIZE      4            /* Size of data in a 32 bits integer */
#define NS_INT16_SIZE      2            /* Size of data in a 16 bits integer */
#define NS_INT8_SIZE       1            /* Size of data in a 8  bits integer */
#define NS_INADDR_SIZE     4            /* IPv4 T_A                          */
#define NS_IN6ADDR_SIZE    16           /* IPv6 T_AAAA                       */
#define NS_COMPRES_FLAGS   0xc0         /* Flag bits indicating compression. */
#define NS_DEFAULT_PORT    53           /* For both TCP and UDP.             */

#define NS_TIME_OUT        -1
#define NS_NO_INFO         -2
#define NS_ERROR           -3
#define NS_NONAUTH         -4
#define NS_NO_RESPONSE     -5

#define QUERY_BUFFER_SIZE  NS_PACKET_SIZE * 3

/* Currently defined type values for resources and queries.                  */

typedef enum __ns_type {
    ns_t_a        = 1,                  /* Host address.                     */
    ns_t_ns       = 2,                  /* Authoritative server.             */
    ns_t_md       = 3,                  /* Mail destination.                 */
    ns_t_mf       = 4,                  /* Mail forwarder.                   */
    ns_t_cname    = 5,                  /* Canonical name.                   */
    ns_t_soa      = 6,                  /* Start of authority zone.          */
    ns_t_mb       = 7,                  /* Mailbox domain name.              */
    ns_t_mg       = 8,                  /* Mail group member.                */
    ns_t_mr       = 9,                  /* Mail rename name.                 */
    ns_t_null     = 10,                 /* Null resource record.             */
    ns_t_wks      = 11,                 /* Well known service.               */
    ns_t_ptr      = 12,                 /* Domain name pointer.              */
    ns_t_hinfo    = 13,                 /* Host information.                 */
    ns_t_minfo    = 14,                 /* Mailbox information.              */
    ns_t_mx       = 15,                 /* Mail routing information.         */
    ns_t_txt      = 16,                 /* Text strings.                     */
    ns_t_rp       = 17,                 /* Responsible person.               */
    ns_t_afsdb    = 18,                 /* AFS cell database.                */
    ns_t_x25      = 19,                 /* X_25 calling address.             */
    ns_t_isdn     = 20,                 /* ISDN calling address.             */
    ns_t_rt       = 21,                 /* Router.                           */
    ns_t_nsap     = 22,                 /* NSAP address.                     */
    ns_t_nsap_ptr = 23,                 /* Reverse NSAP lookup (deprecated). */
    ns_t_sig      = 24,                 /* Security signature.               */
    ns_t_key      = 25,                 /* Security key.                     */
    ns_t_px       = 26,                 /* X.400 mail mapping.               */
    ns_t_gpos     = 27,                 /* Geographical position (withdrawn) */
    ns_t_aaaa     = 28,                 /* Ip6 Address.                      */
    ns_t_loc      = 29,                 /* Location Information.             */
    ns_t_nxt      = 30,                 /* Next domain (security).           */
    ns_t_eid      = 31,                 /* Endpoint identifier.              */
    ns_t_nimloc   = 32,                 /* Nimrod Locator.                   */
    ns_t_srv      = 33,                 /* Server Selection.                 */
    ns_t_atma     = 34,                 /* ATM Address                       */
    ns_t_naptr    = 35,                 /* Naming Authority PoinTeR          */

   /* Query type values which do not appear in resource records.             */
    ns_t_ixfr     = 251,                /* Incremental zone transfer.        */
    ns_t_axfr     = 252,                /* Transfer zone of authority.       */
    ns_t_mailb    = 253,                /* Transfer mailbox records.         */
    ns_t_maila    = 254,                /* Transfer mail agent records.      */
    ns_t_any      = 255,                /* Wildcard match.                   */
    ns_t_max      = 32767
} ns_type;


/* Values for class field                                                    */

typedef enum __ns_class {
    ns_c_in    = 1,                     /* Internet.                         */
                                        /* Class 2 unallocated/unsupported.  */
    ns_c_chaos = 3,                     /* MIT Chaos-net.                    */
    ns_c_hs    = 4,                     /* MIT Hesiod.                       */

    /* Query class values which do not appear in resource records            */
    ns_c_none  = 254,                   /* for prereq. sect. in update req.  */
    ns_c_any   = 255,                   /* Wildcard match.                   */
    ns_c_max   = 32767
} ns_class;


/*  Currently defined response codes.                                        */

typedef enum __ns_rcode {
    ns_r_noerror  = 0,                  /* No error occurred.                */
    ns_r_formerr  = 1,                  /* Format error.                     */
    ns_r_servfail = 2,                  /* Server failure.                   */
    ns_r_nxdomain = 3,                  /* Name error.                       */
    ns_r_notimpl  = 4,                  /* Unimplemented.                    */
    ns_r_refused  = 5,                  /* Operation refused.                */
    /* these are for UPDATE                                                  */
    ns_r_yxdomain = 6,                  /* Name exists                       */
    ns_r_yxrrset  = 7,                  /* RRset exists                      */
    ns_r_nxrrset  = 8,                  /* RRset does not exist              */
    ns_r_notauth  = 9,                  /* Not authoritative for zone        */
    ns_r_notzone  = 10,                 /* Zone of record different          */
    ns_r_max      = 11
} ns_rcode;

/*- Structure definitions ---------------------------------------------------*/


/* Structure for query header.  The order of the fields is machine- and      *
 * compiler-dependent, depending on the byte/bit order and the layout        *
 * of bit fields.  We use bit fields only in int variables, as this          *
 * is all ANSI requires.  This requires a somewhat confusing rearrangement.  */

typedef struct {
    unsigned id: 16;                    /* query identification number       */
#if BYTE_ORDER == BIG_ENDIAN
    /* fields in third byte                                                  */
    unsigned qr: 1;                     /* response flag                     */
    unsigned opcode: 4;                 /* purpose of message                */
    unsigned aa: 1;                     /* authoritive answer                */
    unsigned tc: 1;                     /* truncated message                 */
    unsigned rd: 1;                     /* recursion desired                 */
    /* fields in fourth byte                                                 */
    unsigned ra: 1;                     /* recursion available               */
    unsigned unused :1;                 /* unused bits                       */
    unsigned ad: 1;                     /* authentic data from named         */
    unsigned cd: 1;                     /* checking disabled by resolver     */
    unsigned rcode :4;                  /* response code                     */
#elif BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
    /* fields in third byte                                                  */
    unsigned rd :1;                     /* recursion desired                 */
    unsigned tc :1;                     /* truncated message                 */
    unsigned aa :1;                     /* authoritive answer                */
    unsigned opcode :4;                 /* purpose of message                */
    unsigned qr :1;                     /* response flag                     */
    /* fields in fourth byte                                                 */
    unsigned rcode :4;                  /* response code                     */
    unsigned cd: 1;                     /* checking disabled by resolver     */
    unsigned ad: 1;                     /* authentic data from named         */
    unsigned unused: 1;                 /* unused bits                       */
    unsigned ra: 1;                     /* recursion available               */
#endif
    /* remaining bytes                                                       */
    unsigned qdcount: 16;               /* number of question entries        */
    unsigned ancount: 16;               /* number of answer entries          */
    unsigned nscount: 16;               /* number of authority entries       */
    unsigned arcount: 16;               /* number of resource entries        */
} NS_HEADER;

typedef union {
    NS_HEADER query_header;
    byte      query_buffer [QUERY_BUFFER_SIZE];
} QUERY_BUF;


/* Request structure used in stack                                           */

typedef struct _NS_REQUEST              /*  Name Server Request              */
{
    struct _NS_REQUEST
          *next, *prev;                 /*    Doubly-linked list             */
    char  *host_name;                   /*    Host name                      */
    char  *host_address;                /*    Host ip address (a.b.c.d)      */
    qbyte  host_ip;                     /*    Host ip (network order)        */
    char  *ns_name;                     /*    Name server name               */
    char  *ns_address;                  /*    Name server address (a.b.c.d)  */
    qbyte  ns_ip;                       /*    Name server ip (network order) */
    dbyte  ns_port;                     /*    Name server port               */
    byte   type;                        /*    Type o request (ip address, ...*/
    long   ttl;                         /*    Time To Live                   */
    Bool   recursive;                   /*    Try recursive request          */
    Bool   main_request;                /*    Request created from serverlist*/
    byte   main_index;                  /*    Index in server list           */
} NS_REQUEST;


/* NS_RR_RESULT structure used to store answer result                        */

typedef struct {
  char
      ns_name [NS_MAX_CDNAME + 1];       /*    Domain name server name       */
  struct in_addr
      ns_addr;                           /*    Domain name server address    */
  long
      ttl;                               /*    Time To Live                  */
} NS_RR_RESULT;


/*  DNS configuration for the computer                                       */

typedef struct {
   struct sockaddr_in
       ns_addr [MAX_NS];                /*  Address of name server           */
   short
       ns_count;                        /*  number of name servers           */
   Bool
       recursive_accept [MAX_NS];       /*  Accept recursive query           */
} NS_CTX;

/*- Global variables --------------------------------------------------------*/

extern Bool   dns_debug_mode;           /* Debuf mode flag                   */
extern Bool   dns_recursive;            /* Use Recursive mode if possible    */
extern NS_CTX server_list;              /* Name server list (computer config)*/

/*- Function prototypes -----------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

int        rdns_init              (NS_CTX *p_ns);
QUERY_BUF *rdns_make_query        (char *host_ip, long id, word *size,
                                   Bool recursive);
QUERY_BUF *rdns_make_ip_query     (char *host_name, long id, word *size,
                                   Bool recursive);
Bool       rdns_read_answer       (QUERY_BUF *answer, word size,
                                   NS_REQUEST *current, SYMTAB *invalid_ns,
                                   NS_RR_RESULT *rr_tab, dbyte rr_nbr);
NS_REQUEST *rdns_request_alloc    (qbyte ip_address, char *ip_value,
                                   char *host_name, byte request_type);
void       rdns_request_free      (NS_REQUEST *request);
Bool       is_request_host_exist  (NS_REQUEST *head, char *hos_name, byte type);
Bool       is_request_ns_exist    (NS_REQUEST *head, qbyte ip_addr,  byte type);
dbyte      rdns_get_nbr_rr_result (QUERY_BUF *answer);
Bool       rdns_is_recursive      (QUERY_BUF *answer);
Bool       rdns_check_answer_size (QUERY_BUF *answer, dbyte read_size);
#ifdef __cplusplus
}
#endif

#endif
