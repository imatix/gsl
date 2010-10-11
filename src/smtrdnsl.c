/*===========================================================================*
 *                                                                           *
 *  smtrdnsl.c - Reverse-DNS functions                                       *
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

#include "smtpriv.h"                    /*  SMT definitions                  */
#include "smtrdnsl.h"                   /*  Own prototypes                   */


/*- Macros ------------------------------------------------------------------*/

#define NS_PUT16(s, cp) {        \
        dbyte db = (dbyte)(s);   \
        byte *bp = (byte *)(cp); \
        *bp++ = db >> 8;         \
        *bp   = (byte)db;        \
        (cp) += NS_INT16_SIZE;   \
}

#define NS_PUT32(l, cp) {        \
        qbyte db = (qbyte)(l);   \
        byte *bp = (byte *)(cp); \
        *bp++ = db >> 24;        \
        *bp++ = db >> 16;        \
        *bp++ = db >> 8;         \
        *bp   = db;              \
        (cp) += NS_INT32_SIZE;   \
}

#define NS_GET16(s, cp) { \
        register byte *t_cp = (byte *)(cp); \
        (s) = ((dbyte)t_cp[0] << 8) \
            | ((dbyte)t_cp[1]) \
            ; \
        (cp) += NS_INT16_SIZE; \
}

#define NS_GET32(l, cp) { \
        register byte *t_cp = (byte *)(cp); \
        (l) = ((qbyte)t_cp[0] << 24) \
            | ((qbyte)t_cp[1] << 16) \
            | ((qbyte)t_cp[2] << 8) \
            | ((qbyte)t_cp[3]) \
            ; \
        (cp) += NS_INT32_SIZE; \
}


/*- Global variables --------------------------------------------------------*/

char
    *cname = NULL;
Bool
    dns_recursive  = FALSE,
    dns_debug_mode = FALSE;
NS_CTX
    server_list;                        /*  Name server list                 */


/*- Internal function prototypes --------------------------------------------*/

static char         *format_host_ip   (char *host_ip);
static byte         *get_rr_record    (byte *position, byte *message,
                                       NS_RR_RESULT *tab, int tab_size);
static int           format_host_name (const char *src, byte *dst,
                                       short dstsize);
static NS_RR_RESULT *get_first_result (NS_RR_RESULT *tab, int size,
                                       char *dns_name);
static const char   *ns_strerror      (int result);
static const char   *ns_strtype       (int type);
static Bool          ns_special_char  (int ch);
static const char   *ns_strclass      (int class);
static int           ns_name_skip     (const byte **ptrptr, const byte *end);
static int           ns_expand        (const byte *msg, const byte *eom,
                                       const byte *src, char *dst,
                                       int dstsize);
static qbyte         ns_get32         (byte *buffer);
static dbyte         ns_get16         (byte *buffer);


/*****************************************************************************/
/*                     Initialisation of library                             */
/*****************************************************************************/

/*  ---------------------------------------------------------------------[<]-
    Function: rdns_init

    Synopsis: Initialises the Reverse DNS library.
    ---------------------------------------------------------------------[>]-*/

int
rdns_init (NS_CTX *p_ns)
{
    static Bool
        initialise = FALSE;
    int
        index;

    ASSERT (p_ns);

    if (initialise == FALSE)
      {
        memset (p_ns, 0, sizeof (NS_CTX));
        p_ns-> ns_count = (short)get_name_server (
                          (struct sockaddr_in *) p_ns-> ns_addr, MAX_NS);
        for (index = 0; index < p_ns-> ns_count; index++)
            p_ns-> recursive_accept [index] = dns_recursive;

        initialise = TRUE;
      }
    return (0);
}


/*****************************************************************************/
/*                     Request management functions                          */
/*****************************************************************************/

/*  ---------------------------------------------------------------------[<]-
    Function: rdns_request_alloc

    Synopsis: Allocate and initialise a request struct. Return NULL if invalid
              parameters.
    ---------------------------------------------------------------------[>]-*/

NS_REQUEST *
rdns_request_alloc (qbyte ip_address, char *ip_value, char *host_name,
                    byte request_type)
{
    NS_REQUEST
        *request = NULL;
    request = mem_alloc (sizeof (NS_REQUEST));
    if (request)
      {
        memset (request, 0, sizeof (NS_REQUEST));
        switch (request_type)
          {
            case REQ_TYPE_HOST:
                if (ip_value)
                    request-> host_address = mem_strdup (ip_value);
                if (request-> host_address == NULL)
                  {
                    mem_free (request);
                    return (NULL);
                  }
                request-> host_ip = ip_address;
                request-> type    = request_type;
                break;
            case REQ_TYPE_IP:
                if (host_name)
                    request-> host_name = mem_strdup (host_name);
                if (request-> host_name == NULL)
                  {
                    mem_free (request);
                    return (NULL);
                  }
                request-> type    = request_type;
                break;
            default:
                break;
          }
        list_reset (request);
        request-> ns_port   = htons (DNS_PORT); /*  Default DNS port (53)    */
        request-> recursive = dns_recursive;   /*  Default recursive mode    */
      }

    return (request);
}


/*  ---------------------------------------------------------------------[<]-
    Function: rdns_request_free

    Synopsis: Free all memory allocated in a request struct
    ---------------------------------------------------------------------[>]-*/

void
rdns_request_free (NS_REQUEST *request)
{
    ASSERT (request);

    mem_strfree (&request-> host_name);
    mem_strfree (&request-> host_address);
    mem_strfree (&request-> ns_name);
    mem_strfree (&request-> ns_address);
    list_unlink (request);
    mem_free    (request);
}


/*  ---------------------------------------------------------------------[<]-
    Function: is_request_host_exist

    Synopsis: Return true if a request in the stack content a request for
              this host name.
    ---------------------------------------------------------------------[>]-*/

Bool
is_request_host_exist (NS_REQUEST *head, char *host_name, byte type)
{
    Bool
        find = FALSE;
    NS_REQUEST
        *request;
    request = head;
    while (request-> next != head)
      {
        if (request-> host_name
        &&  streq (request-> host_name, host_name)
        &&  request-> type == type)
          {
            find = TRUE;
            break;
          }
        request = request-> next;
      }

    return (find);
}


/*  ---------------------------------------------------------------------[<]-
    Function: is_request_ns_exist

    Synopsis: Return true if a request in the stack content a request for
              this ip address.
    ---------------------------------------------------------------------[>]-*/

Bool
is_request_ns_exist (NS_REQUEST *head, qbyte ip_addr, byte type)
{
    Bool
        find = FALSE;
    NS_REQUEST
        *request;
    request = head;
    while (request-> next != head)
      {
        if (request-> ns_ip == ip_addr
        &&  request-> type  == type)
          {
            find = TRUE;
            break;
          }
        request = request-> next;
      }

    return (find);
}


/*****************************************************************************/
/*            Name Server Query and Answer management functions              */
/*****************************************************************************/

/*  ---------------------------------------------------------------------[<]-
    Function: rdns_make_query

    Synopsis: Make the query to send in name server.
    Warning : Free query buffer with mem_free after use.
    ---------------------------------------------------------------------[>]-*/

QUERY_BUF *
rdns_make_query (char *host_ip, long id, word *size, Bool recursive)
{
    QUERY_BUF
        *query = NULL;
    byte
        *data,
        *query_host;

    ASSERT (host_ip);
    ASSERT (size);

    query = mem_alloc (sizeof (QUERY_BUF));
    if (query == NULL)
        return (NULL);

    if (dns_debug_mode)
        coprintf ("Request ip address %s", host_ip);

    memset (query, 0, sizeof (QUERY_BUF));
    query_host = (byte *)format_host_ip (host_ip);
    query-> query_header.id      = htons((unsigned short)id);
    query-> query_header.opcode  = 0;   /*  QUERY                            */
    query-> query_header.rd      = recursive;
    query-> query_header.rcode   = ns_r_noerror;
    query-> query_header.qdcount = htons(1);

    data = query-> query_buffer + NS_HEAD_FIXED_SIZE;
    strcpy ((char *)data, (char *)query_host);
    data += strlen ((char *)query_host) + 1;
    NS_PUT16 (ns_t_ptr, data);         /* Set type PTR                       */
    NS_PUT16 (ns_c_in,  data);         /* Set class Internet                 */

    *size = data - query-> query_buffer;
    return (query);
}

/*  ---------------------------------------------------------------------[<]-
    Function: rdns_make_ip_query

    Synopsis: Make the query for request the ip value for a host name.
    Warning : Free query buffer with mem_free after use.
    ---------------------------------------------------------------------[>]-*/

QUERY_BUF *
rdns_make_ip_query (char *host_name, long id, word *size, Bool recursive)
{
    QUERY_BUF
        *query = NULL;
    byte
        *data;
    int
        len;

    ASSERT (host_name);
    ASSERT (size);

    query = mem_alloc (sizeof (QUERY_BUF));
    if (query == NULL)
        return (NULL);

    if (dns_debug_mode)
        coprintf ("Request ip address for host name %s", host_name);

    memset (query, 0, sizeof (QUERY_BUF));
    query-> query_header.id     = htons((unsigned short)id);
    query-> query_header.opcode = 0;    /*  QUERY                            */
    query-> query_header.rd     = recursive;
    query-> query_header.rcode  = ns_r_noerror;
    query-> query_header.qdcount = htons(1);

    data = query-> query_buffer + NS_HEAD_FIXED_SIZE;
    len = format_host_name (host_name, data,
                            sizeof (QUERY_BUF) - NS_HEAD_FIXED_SIZE);
    if (len > 0)
        data += len;
    NS_PUT16 (ns_t_a,  data);          /* Set type Host Address              */
    NS_PUT16 (ns_c_in, data);          /* Set class Internet                 */

    *size = data - query-> query_buffer;
    return (query);
}


/*  ---------------------------------------------------------------------[<]-
    Function: rdns_read_answer

    Synopsis: Read answer buffer from a query. Return TRUE if the answer
              content the host name.
    ---------------------------------------------------------------------[>]-*/

Bool
rdns_read_answer (QUERY_BUF *answer, word size, NS_REQUEST *current,
                  SYMTAB *invalid_ns, NS_RR_RESULT *rr_tab, dbyte rr_nbr)
{
    NS_HEADER
        *header;
    NS_REQUEST
        *new_req,
        *request;
    dbyte
        index;
    short
        ancount,
        nscount,
        arcount,
        qdcount;
    byte
        *buffer,
        *end_pointer,
        *position;
    Bool
        feedback = FALSE;

    ASSERT (answer);
    ASSERT (current);

    if (size == 0)
        return (FALSE);

    buffer      = (byte *)answer;
    end_pointer =  buffer + sizeof (QUERY_BUF);

    /*  Find first satisfactory answer.                                      */
    header  = (NS_HEADER *)answer;
    ancount = ntohs ((unsigned short) header-> ancount);
    nscount = ntohs ((unsigned short) header-> nscount);
    arcount = ntohs ((unsigned short) header-> arcount);
    qdcount = ntohs ((unsigned short) header-> qdcount);
    if (dns_debug_mode)
        coprintf("Return code = %d (%s), ancount=%d",
                  header-> rcode,
                  ns_strerror (header-> rcode),
                  ancount);

    if ((ancount == 0 && nscount == 0 && arcount == 0)
    ||  rr_tab == NULL)
        return (FALSE);

    cname = NULL;
    position = answer-> query_buffer + NS_HEAD_FIXED_SIZE;
    if (qdcount)
      {
         position += ns_name_skip ((const byte **)&position, end_pointer)
                     + NS_QRY_FIXED_SIZE;
         while (--qdcount > 0 && position && position < end_pointer)
             position += ns_name_skip ((const byte **)&position, end_pointer)
                         + NS_QRY_FIXED_SIZE;
      }
    if (ancount)
      {
        if (!header->aa)
            if (dns_debug_mode)
                coprintf("The following answer is not authoritative:");
        if (current-> type == REQ_TYPE_IP
        &&  current-> host_name)
             strcpy (rr_tab [0].ns_name, current-> host_name);
        while (--ancount >= 0 && position && position < end_pointer)
          {
            position = (byte *)get_rr_record (position, buffer, rr_tab, 1);
            if (current-> type == REQ_TYPE_IP
            &&  rr_tab [0].ns_addr.s_addr != 0)
              {                         /* Found host address ip             */
                mem_strfree (&current-> host_address);
                current-> host_ip = rr_tab [0].ns_addr.s_addr;
                current-> host_address = mem_strdup (
                                             inet_ntoa(rr_tab [0].ns_addr));
                current-> ttl = rr_tab [0].ttl;
                return (TRUE);
              }
            else
            if (current-> type == REQ_TYPE_HOST
            &&  cname)                  /* Found the host name               */
              {
                mem_strfree (&current-> host_name);
                current-> host_name = mem_strdup (cname);
                current-> ttl = rr_tab [0].ttl;
                return (TRUE);
              }
         }
      }

    if (nscount)
      {
        if (dns_debug_mode)
            coprintf("For authoritative answers, see:");
        while (--nscount >= 0 && position && position < end_pointer)
              position = get_rr_record (position, buffer, rr_tab, rr_nbr);
      }
    if (arcount)
      {
        if (dns_debug_mode)
            coprintf("Additional information:");
        while (--arcount >= 0 && position && position < end_pointer)
              position = get_rr_record (position, buffer, rr_tab, rr_nbr);
      }

    if (header-> rcode != ns_r_noerror)
        return (FALSE);

    /* Create new request in request stack                                   */
    request = current;
    for (index = 0; index < rr_nbr; index++)
      {
        if (strused (rr_tab [index].ns_name)
        ||  rr_tab [index].ns_addr.s_addr != 0)
          {
            if (rr_tab [index].ns_addr.s_addr != 0)
              {
                if (is_request_ns_exist (current,
                    rr_tab [index].ns_addr.s_addr, current-> type))
                    continue;
              }
            else
                if (is_request_host_exist (current,
                    rr_tab [index].ns_name, current-> type))
                    continue;

            if (invalid_ns != NULL
            &&  sym_lookup_symbol (invalid_ns, rr_tab [index].ns_name) != NULL)
                continue;

            if (current-> type == REQ_TYPE_HOST)
              {
                if (rr_tab [index].ns_addr.s_addr != 0)
                  {
                    new_req = rdns_request_alloc (current-> host_ip,
                                                  current-> host_address,
                                                  NULL,
                                                  current-> type);
                    if (new_req)
                     {
                       new_req-> ns_ip = rr_tab [index].ns_addr.s_addr;
                       new_req-> ns_name = mem_strdup (rr_tab [index].ns_name);
                     }
                  }
                else
                  {
                    new_req = rdns_request_alloc (0,
                                                  NULL,
                                                  rr_tab [index].ns_name,
                                                  REQ_TYPE_IP);
                    if (new_req)
                        new_req-> ns_ip = current-> ns_ip;
                  }
              }
            else
              {
                if (rr_tab [index].ns_addr.s_addr != 0)
                  {
                    new_req = rdns_request_alloc (0,
                                                  NULL,
                                                  current-> host_name,
                                                  current-> type);
                    if (new_req)
                     {
                       new_req-> ns_ip = rr_tab [index].ns_addr.s_addr;
                       new_req-> ns_name = mem_strdup (rr_tab [index].ns_name);
                     }
                  }
                else
                  {
                    new_req = rdns_request_alloc (0,
                                                  NULL,
                                                  rr_tab [index].ns_name,
                                                  REQ_TYPE_IP);
                    if (new_req)
                        new_req-> ns_ip = current-> ns_ip;
                  }
              }
            if (new_req)
              {
                list_relink_after (new_req, request);
                request = new_req;
              }

          }
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: rdns_get_nbr_rr_result

    Synopsis: Get the number of RR record for authority or ressource entry.
              if the answer content RR record for answer entry, return
              this number of RR record
    ---------------------------------------------------------------------[>]-*/

dbyte
rdns_get_nbr_rr_result (QUERY_BUF *answer)
{
    NS_HEADER
        *header;
    dbyte
        value,
        rr_nbr = 0;

    ASSERT (answer);
    header  = (NS_HEADER *)answer;
    value   = ntohs ((unsigned short)header->ancount);
    if (value > 0)
        rr_nbr =  value;
    else
      {
        rr_nbr = ntohs ((unsigned short)header->nscount);
        value  = ntohs ((unsigned short)header->arcount);
        if (value > rr_nbr)
            rr_nbr = value;
      }
    return (rr_nbr);
}


/*  ---------------------------------------------------------------------[<]-
    Function: rdns_check_answer_size

    Synopsis: return TRUE if answer is complete.
    ---------------------------------------------------------------------[>]-*/

Bool
rdns_check_answer_size (QUERY_BUF *answer, dbyte read_size)
{
    NS_HEADER
        *header;
    short
        ancount,
        nscount,
        arcount,
        qdcount;
    byte
        *buffer,
        *end_pointer,
        *position;
    Bool
        feedback = FALSE;

    ASSERT (answer);

    if (read_size < NS_HEAD_FIXED_SIZE)
        return (FALSE);

    buffer      = (byte *)answer;
    end_pointer =  buffer + read_size;

    /*  Find first satisfactory answer.                                      */
    header  = (NS_HEADER *)answer;
    ancount = ntohs ((unsigned short) header-> ancount);
    nscount = ntohs ((unsigned short) header-> nscount);
    arcount = ntohs ((unsigned short) header-> arcount);
    qdcount = ntohs ((unsigned short) header-> qdcount);


    cname = NULL;
    position = answer-> query_buffer + NS_HEAD_FIXED_SIZE;

    if (qdcount)
      {
         position += ns_name_skip ((const byte **)&position, end_pointer)
                     + NS_QRY_FIXED_SIZE;
         while (--qdcount > 0 && position && position < end_pointer)
             position += ns_name_skip ((const byte **)&position, end_pointer)
                         + NS_QRY_FIXED_SIZE;
      }
    if (ancount)
      {
        while (--ancount >= 0 && position && position < end_pointer)
            position = (byte *)get_rr_record (position, buffer, NULL, 0);
      }

    if (nscount)
      {
        while (--nscount >= 0 && position && position < end_pointer)
              position = get_rr_record (position, buffer, NULL, 0);
      }
    if (arcount)
      {
        while (--arcount >= 0 && position && position < end_pointer)
              position = get_rr_record (position, buffer, NULL, 0);
      }

    if (position != NULL
    &&  position >= (end_pointer - NS_QRY_FIXED_SIZE)
    &&  qdcount  <= 0
    &&  ancount  <= 0
    &&  nscount  <= 0
    &&  arcount  <= 0
       )
        feedback = TRUE;

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: rdns_is_recursive

    Synopsis: Return TRUE if DNS server accept recursive query.
    ---------------------------------------------------------------------[>]-*/

Bool
rdns_is_recursive (QUERY_BUF *answer)
{
    NS_HEADER
        *header;

    ASSERT (answer);
    header  = (NS_HEADER *)answer;

    return ((header-> ra == 1)? TRUE: FALSE);
}
/*  -------------------------------------------------------------------------
    Function: format_host_ip - internal

    Synopsis: Format the ip address 'a.b.c.d' to a suite of label.
              a label is length of data + data
              Exemple 'd.c.b.a.IN-ADDR.ARPA' => d_len d c_len c b_len.
              The suite of label end with a 0 length.
              (see RFC 1035 for encode Domain Name).
    -------------------------------------------------------------------------*/

static char *
format_host_ip (char *host_ip)
{
    static unsigned int
        count,
        h [4];
    int
        index;
    static char
        buffer [NS_MAX_CDNAME + 1],
        value  [4];
    char
        *p   = buffer + 1,
        *len = buffer,
        *val;

    memset (buffer, 0, NS_MAX_CDNAME + 1);

    count = sscanf (host_ip, "%u.%u.%u.%u", &h [0], &h [1], &h [2], &h [3]);
    if (count > 0)
      {
        for (index = count - 1; index >= 0; index--)
          {
            sprintf (value, "%u", h [index]);
            strcat  (buffer, value);
            val = value;
            while (*val)
                *p++ = *val++;
            *len = (char)(p - len - 1);
            len = p++;
          }
      }
    *len = 7;
    strcpy (p, "in-addr");
    len += 8;
    p += 8;
    *len = 4;
    strcpy (p, "arpa");

    return (buffer);
}


/*  -------------------------------------------------------------------------
    Function: format_host_name - internal

    Synopsis: Convert a ascii string into an encoded domain name as per
    RFC1035.  Return -1 if error else the new size.
    -------------------------------------------------------------------------*/

static int
format_host_name (const char *source, byte *dest, short destsiz)
{
    byte
        *label,
        *label_length,
        *end;
    int c,
        n;
    char
        *cp;
    static char
        digits[] = "0123456789";

    label_length = dest;
    end = dest + destsiz;
    label = label_length++;

    while ((c = *source++) != 0)
      {
        if (c == '\\')                 /*  Escape sequence                   */
          {
            c = *source++;
            if ((cp = strchr(digits, c)) != NULL)
              {
                n = (cp - digits) * 100;
                if ((c = *source++) == 0
                ||  (cp = strchr(digits, c)) == NULL)
                    return (-1);
                n += (cp - digits) * 10;
                if ((c = *source++) == 0
                ||  (cp = strchr(digits, c)) == NULL)
                    return (-1);
                n += (cp - digits);
                if (n > 255)
                    return (-1);
                c = n;
              }
          }
        else
        if (c == '.')                   /*  Label separator                  */
          {
            c = (label_length - label - 1);
            if ((c & NS_COMPRES_FLAGS) != 0)
                return (-1);
            if (label >= end)
                return (-1);
            *label = (byte)c;
            if (*source == '\0')
              {
                if (c != 0)
                  {
                    if (label_length >= end)
                        return (-1);
                    *label_length++ = '\0';
                  }
                if ((label_length - dest) > NS_MAX_CDNAME)
                    return (-1);
                return (label_length - dest);
              }
            if (c == 0)
                return (-1);
            label = label_length++;
            continue;
          }
        if (label_length >= end)
            return (-1);
        *label_length++ = (byte) c;
      }
    c = (label_length - label - 1);
    if ((c & NS_COMPRES_FLAGS) != 0)
        return (-1);
    if (label >= end)
        return (-1);
    *label = (byte)c;
    if (c != 0)
      {
        if (label_length >= end)
            return (-1);
        *label_length++ = 0;
      }
    if ((label_length - dest) > NS_MAX_CDNAME)
        return (-1);
    return (label_length - dest);
}


/*  -------------------------------------------------------------------------
    Function: ns_name_skip - internal

    Synopsis: Advance *ptrptr to skip over the compressed name it points at.
              return: 0 on success, -1 (with errno set) on failure.
    -------------------------------------------------------------------------*/

static int
ns_name_skip (const byte **ptrptr, const byte *end)
{
    const byte
        *pointer;
    dbyte
        value;

    pointer = *ptrptr;
    while (pointer < end && (value = *pointer++) != 0)
      {
        /* Check for indirection.                                            */
        switch (value & NS_COMPRES_FLAGS)
          {
            case 0:                     /* Normal case, n == len             */
                pointer += value;
                continue;
            case NS_COMPRES_FLAGS:      /* Indirection                       */
                pointer++;
                break;
            default:                    /* Illegal type                      */
                return (-1);
          }
        break;
      }
    if (pointer > end)
        return (-1);
    *ptrptr = pointer;

    return (0);
}


/*  -------------------------------------------------------------------------
    Function: ns_expand - internal

    Synopsis: Expand compressed domain name 'comp_dn' to full domain name.
              'msg' is a pointer to the begining of the message,
              'eom' points to the first location after the message,
              'dst' is a pointer to a buffer of size 'length' for the result.
              Return size of compressed name or -1 if there was an error.
    -------------------------------------------------------------------------*/

static int
ns_expand (const byte *msg, const byte *eom, const byte *src,
           char *dst, int dstsiz)
{
    int
        result_size;
    byte
        tmp [NS_MAX_CDNAME];
    byte
        current_c,
        *pointer;
    char
        *target,
        *end;
    dbyte
        checked,
        label_length;
    static char
        digits[] = "0123456789";

    /* Unpack the domain name from a message, source may be compressed.      */

    result_size = -1;
    checked   = 0;
    target    = (char *)tmp;
    pointer   = (byte *)src;
    end       = (char *)tmp + NS_MAX_CDNAME;

    if (pointer < msg || pointer >= eom)
                return (-1);

    /* Fetch next label in domain name.                                      */
    while ((label_length = *pointer++) != 0)
      {
        /* Check for indirection.                                            */
        switch (label_length & NS_COMPRES_FLAGS)
          {
            case 0:
                /* Limit checks.                                             */
                if (target  + label_length + 1 >= end
                ||  pointer + label_length     >= eom)
                    return (-1);
                checked  += label_length + 1;
                *target++ = (byte)label_length;
                memcpy(target, pointer, label_length);
                target  += label_length;
                pointer += label_length;
                break;

            case NS_COMPRES_FLAGS:
                if (pointer >= eom)
                    return (-1);
                if (result_size < 0)
                    result_size = pointer - src + 1;
                pointer = (byte *)msg + (((label_length & 0x3f) << 8)
                          | (*pointer & 0xff));
                if (pointer < msg || pointer >= eom)
                    return (-1);
                checked += 2;
                /* Check for loops in the compressed name; if we've looked at
                   the whole message, there must be a loop.                  */
                if (checked >= eom - msg)
                    return (-1);
                break;

            default:
                return (-1);            /*  Flag error                       */
          }
      }
    *target = '\0';
    if (result_size < 0)
        result_size = pointer - src;


    /* Convert an encoded domain name to printable ascii as per RFC1035.
       notes: The root is returned as "."
              All other domains are returned in non absolute form            */

    pointer = tmp;
    target  = dst;
    end     = dst + dstsiz;

    while ((label_length = *pointer++) != 0) /* Get length of label          */
      {
        /* Some kind of compression pointer.                                 */
        if ((label_length & NS_COMPRES_FLAGS) != 0)
            return (-1);
        if (target != dst)
          {
            if (target >= end)
                return (-1);
            *target++ = '.';
          }
        if (target + label_length >= end)
            return (-1);
        while (label_length > 0)
          {
            current_c = *pointer++;
            if (ns_special_char (current_c))
              {
                 if (target + 1 >= end)
                     return (-1);
                 *target++ = '\\';
                 *target++ = (char)current_c;
              }
            else /* Non printable character                                  */
            if (!(current_c > 0x20 && current_c < 0x7f))
              {
                if (target + 3 >= end)
                    return (-1);
                *target++ = '\\';
                *target++ = digits [ current_c / 100];
                *target++ = digits [(current_c % 100) / 10];
                *target++ = digits [ current_c % 10];
              }
            else
              {
                if (target >= end)
                    return (-1);
                *target++ = (char)current_c;
              }
            label_length--;
          }
      }

    /*  If Root domain                                                       */
    if (target == dst)
      {
        if (target >= end)
            return (-1);
        *target++ = '.';
      }
    if (target >= end)
        return (-1);
    *target++ = '\0';

    if (result_size > 0
    &&  dst[0] == '.')
        dst[0] = '\0';

    return (result_size);
}


static byte*
get_cdname (byte *position, byte *message, char *name, int namelen)
{
     int
        n;
    n = ns_expand(message, message + 512, position, name, namelen - 2);

    if (n < 0)
        return (NULL);
    if (strnull (name))
      {
         name[0] = '.';
         name[1] = '\0';
      }
    return (position + n);
}



static NS_RR_RESULT *
get_first_result (NS_RR_RESULT *tab, int size, char *dns_name)
{
    int
        cur_index = 0,
        index;
    NS_RR_RESULT
        *current = tab;

    for (index = 0; index < size; index++)
      {
        if (strused (tab [index].ns_name))
          {
            current = &tab [index];
            cur_index = index;
            if (streq (current-> ns_name, dns_name))
                break;
          }
      }
    if (current != NULL
    && !streq (current-> ns_name, dns_name))
      {
        if (cur_index < size - 1)
            current = &tab [cur_index + 1];
        else
        if (size == 1)
            current = tab;
      }
    return (current);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_rr_record

    Synopsis: Print resource record fields in human readable form.
    ---------------------------------------------------------------------[>]-*/

static byte*
get_rr_record (byte *position, byte *message, NS_RR_RESULT *tab, int tab_size)
{
    int
       type,
       class,
       dlen,
       n,
       c,
       proto;
    qbyte
       ttl = 0;
    struct in_addr
       inaddr;
    byte
       *other_pos;
    struct protoent
       *protop;
    struct servent
       *servp;
    static char
       buffer   [NS_MAX_DNAME],
       cnamebuf [NS_MAX_DNAME],
       name     [NS_MAX_DNAME],
       punc;
    NS_RR_RESULT
       *rr_result;

    memset (buffer, 0, NS_MAX_DNAME);

    position = (byte *) get_cdname (position, message, name, sizeof (name));
    if (position == NULL)
        return (NULL);                  /*  Compression error                */

    NS_GET16(type, position);
    NS_GET16(class, position);
    NS_GET32(ttl, position);
    if (dns_debug_mode)
      {
        sprintf (buffer, "name:%s TTL:%ld %s %s",
                          name, (long) ttl, ns_strclass(class), ns_strtype(type));
        if (strlen (buffer) > 255)
            buffer [255] = '\0';
        coprintf (buffer);
      }
    punc = '\t';

    NS_GET16(dlen, position);
    other_pos = position;

        /*
         * Print type specific data, if appropriate.
         */
    switch (type)
      {
        case ns_t_a:
            memcpy(&inaddr, position, NS_INADDR_SIZE);
            if (dns_debug_mode)
                sprintf (&strterm (buffer), "%c%s", punc, inet_ntoa (inaddr));
            position += dlen;
            rr_result = get_first_result (tab, tab_size, name);
            if (rr_result)
              {
                memcpy (&rr_result-> ns_addr, &inaddr,
                        sizeof (struct in_addr));
                rr_result-> ttl = ttl;
              }
            break;
        case ns_t_cname:
            if (ns_expand(message, message + 512, position, cnamebuf,
                          sizeof(cnamebuf)) >= 0)
              {
                if (tab != NULL)
                  {
                    strcpy (tab-> ns_name, cnamebuf);
                    tab-> ttl = ttl;
                  }
                cname = cnamebuf;
              }
        case ns_t_mb:
        case ns_t_mg:
        case ns_t_mr:
        case ns_t_ns:
        case ns_t_ptr:
            position = (byte *) get_cdname (position, message,
                                            name, sizeof(name));
            if (dns_debug_mode)
                sprintf (&strterm (buffer), "%c%s",punc, name);
            cname = name;
            rr_result = get_first_result (tab, tab_size, name);
            if (rr_result)
              {
                strcpy (rr_result-> ns_name, name);
                rr_result-> ttl = ttl;
              }
            break;
        case ns_t_hinfo:
        case ns_t_isdn:
          {
            const byte *end = position + dlen;
            n = *position++;
            if (n != 0)
              {
                if (dns_debug_mode)
                    sprintf (&strterm (buffer), "%c%.*s", punc, n, position);
                position += n;
              }
            if (position < end
            &&  (n = *position++))
              {
                if (dns_debug_mode)
                    sprintf (&strterm (buffer), "%c%.*s", punc, n, position);
                position += n;
              }
            else
            if (type == ns_t_hinfo && dns_debug_mode)
                sprintf (&strterm (buffer),
                         "\n; *** Warning *** OS-type missing");
          }
            break;

        case ns_t_soa:
            position = (byte *) get_cdname (position, message,
                                            name, sizeof(name));
            if (dns_debug_mode)
                sprintf (&strterm (buffer),"\t%s", name);
            position = (byte *) get_cdname (position, message,
                                            name, sizeof(name));
            if (dns_debug_mode)
              {
                sprintf (&strterm (buffer)," %s", name);
                sprintf (&strterm (buffer),"(\n\t\t\t%ld\t;serial (version)",
                                (long) ns_get32(position));
              }
            position += NS_INT32_SIZE;
            if (dns_debug_mode)
                sprintf (&strterm (buffer),
                         "\n\t\t\t%ld\t;refresh period",
                         (long) ns_get32 (position));

            message += NS_INT32_SIZE;
            if (dns_debug_mode)
                sprintf (&strterm (buffer),
                         "\n\t\t\t%ld\t;retry refresh this often",
                         (long) ns_get32 (position));

            position += NS_INT32_SIZE;
            if (dns_debug_mode)
                sprintf (&strterm (buffer),
                         "\n\t\t\t%ld\t;expiration period",
                         (long) ns_get32 (position));

            position += NS_INT32_SIZE;
            if (dns_debug_mode)
                sprintf (&strterm (buffer),
                         "\n\t\t\t%ld\t;minimum TTL\n\t\t\t)",
                         (long) ns_get32(position));

            position += NS_INT32_SIZE;
            break;

        case ns_t_mx:
        case ns_t_afsdb:
        case ns_t_rt:
            if (dns_debug_mode)
                sprintf (&strterm (buffer),"\t%d ", ns_get16(position));
            position += sizeof (dword);
            position = (byte *) get_cdname(position, message, name,
                                            sizeof(name));
            if (dns_debug_mode)
                sprintf (&strterm (buffer),"%s", name);
            break;

        case ns_t_srv:
            if (dns_debug_mode)
                sprintf (&strterm (buffer)," %d", ns_get16(position));
            position += sizeof (dbyte);
            if (dns_debug_mode)
                sprintf (&strterm (buffer)," %d", ns_get16(position));
            position += sizeof(dbyte);
            if (dns_debug_mode)
                sprintf (&strterm (buffer)," %d", ns_get16(position));
            position += sizeof(dbyte);
            position = (byte *)get_cdname(position, message, name,
                                              sizeof(name));
            if (dns_debug_mode)
                sprintf (&strterm (buffer),"%s",name);
            break;

        case ns_t_naptr:
            /* Order                                                         */
            if (dns_debug_mode)
                sprintf (&strterm (buffer)," %d", ns_get16(position));
            position += sizeof (dbyte);
            /* Preference                                                    */
            if (dns_debug_mode)
                sprintf (&strterm (buffer)," %d", ns_get16(position));
            position += NS_INT16_SIZE;
            /* Flags                                                         */
            n = *position++;
            if (dns_debug_mode)
              {
                if (n)
                    sprintf (&strterm (buffer),"%c%.*s", punc, n, position);
                else
                    sprintf (&strterm (buffer),"%c\"\"",punc);
              }
            position += n;
            /* Service                                                       */
            n = *position++;
            if (dns_debug_mode)
              {
                if (n)
                    sprintf (&strterm (buffer),"%c%.*s", punc, n, position);
                else
                    sprintf (&strterm (buffer),"%c\"\"",punc);
              }
            position += n;
            /* Regexp                                                        */
            n = *position++;
            if (dns_debug_mode)
              {
                if (n)
                    sprintf (&strterm (buffer),"%c%.*s", punc, n, position);
                else
                    sprintf (&strterm (buffer),"%c\"\"",punc);
              }
            position += n;
            /* replacement                                                   */
            position = (byte *) get_cdname (position, message,
                                            name, sizeof(name));
            if (dns_debug_mode)
                sprintf (&strterm (buffer),"%s", name);
            break;
        case ns_t_minfo:
        case ns_t_rp:
            position = (byte *) get_cdname (position, message,
                                            name, sizeof(name));
            if (dns_debug_mode)
              {
                if (type == ns_t_rp)
                  {
                    char *point = NULL;
                    if ((point = strchr(name, '.')) != NULL)
                        *point = '@';
                  }
                sprintf (&strterm (buffer),"%c%s", punc, name);
              }
            position = (byte *) get_cdname (position, message,
                                            name, sizeof(name));
            if (dns_debug_mode)
                sprintf (&strterm (buffer)," %s", name);
            break;

        case ns_t_x25:
            n = *position++;
            if (n != 0)
              {
                if (dns_debug_mode)
                    sprintf (&strterm (buffer),"%c%.*s", punc, n, position);
                position += n;
              }
            break;

        case ns_t_txt:
            if (dns_debug_mode)
              {
                int
                   n, j;
                char
                   *buf_pos;
                const byte
                    *end = position + dlen;

                while (position < end)
                  {
                    sprintf (&strterm (buffer), " \"");
                    buf_pos = &strterm (buffer);
                    n = *position++;
                    if (n != 0)
                        for (j = n; j > 0 && position < end ; j --)
                          {
                            if (*position == '\n'
                            ||  *position == '"'
                            ||  *position == '\\')
                                *buf_pos++ = '\\';
                            *buf_pos++ = *position++;
                          }
                    *buf_pos++ = '"';
                    *buf_pos = '\0';
                  }
              }
            else
                position += dlen;
            break;

        case ns_t_wks:
            if (dlen < NS_INT32_SIZE + 1)
                break;
            memcpy(&inaddr, position, NS_INADDR_SIZE);
            position += NS_INT32_SIZE;
            proto = *position++;
            protop = (void *)getprotobynumber(proto);
            if (dns_debug_mode)
              {
                if (protop)
                    sprintf (&strterm (buffer),"%c%s %s", punc,
                                    inet_ntoa(inaddr), protop->p_name);
                else
                    sprintf (&strterm (buffer),"%c%s %d", punc,
                                    inet_ntoa(inaddr), proto);
              }
             n = 0;
             while (position < other_pos + dlen)
               {
                 c = *position++;
                 do
                   {
                     if (c & 0200)
                       {
                          servp = NULL;
                          if (protop)
                              servp = (void *)getservbyport(htons((unsigned short) n),
                                                    protop-> p_name);
                          if (servp && dns_debug_mode)
                              sprintf (&strterm (buffer)," %s", servp->s_name);
                          else
                              sprintf (&strterm (buffer)," %d", n);
                       }
                     c <<= 1;
                   } while (++n & 07);
               }
             break;
        default:
            if (dns_debug_mode)
                sprintf (&strterm (buffer),"%c???", punc);
            position += dlen;
            break;
      }
    if (dns_debug_mode)
      {
        if (position != other_pos + dlen)
            sprintf (&strterm (buffer),"packet size error (%p != %p)\n",
                                       position, other_pos + dlen);

        if (strlen (buffer) > 255)
            buffer [255] = '\0';
        coprintf (buffer);
      }
    return (position);
}


/*  -------------------------------------------------------------------------
    Function: ns_strerror - internal

    Synopsis: Return the text format or errorcode
    -------------------------------------------------------------------------*/

static const char *
ns_strerror (int result)
{
    switch(result)
      {
        case ns_r_noerror:  return ("Success");                  
        case ns_r_formerr:  return ("Format error");             
        case ns_r_servfail: return ("Server failed");            
        case ns_r_nxdomain: return ("Non-existent domain");      
        case ns_r_notimpl:  return ("Not implemented");          
        case ns_r_refused:  return ("Query refused");            
        case NS_NO_INFO:    return ("No information");           
        case NS_ERROR:      return ("Unspecified error");        
        case NS_TIME_OUT:   return ("Timed out");                
        case NS_NONAUTH:    return ("Non-authoritative answer"); 
        default:            return ("BAD ERROR VALUE");
      }
}


/*  -------------------------------------------------------------------------
    Function: ns_strtype - internal

    Synopsis: Return a string for the type.
    -------------------------------------------------------------------------*/

static const char *
ns_strtype(int type)
{
    switch (type)
      {
        case ns_t_a:        return ("has address");
        case ns_t_cname:    return ("is a nickname for");
        case ns_t_mx:       return ("mail is handled");
        case ns_t_txt:      return ("descriptive text");
        case ns_t_afsdb:    return ("DCE or AFS service from");
        case ns_t_ns:       return ("name server");
        case ns_t_md:       return ("mail destination (deprecated)");
        case ns_t_mf:       return ("mail forwarder (deprecated)");
        case ns_t_soa:      return ("start of authority");
        case ns_t_mb:       return ("mailbox");
        case ns_t_mg:       return ("mail group member");
        case ns_t_mr:       return ("mail rename");
        case ns_t_null:     return ("null");
        case ns_t_wks:      return ("well-known service (deprecated)");
        case ns_t_ptr:      return ("domain name pointer");
        case ns_t_hinfo:    return ("host information");
        case ns_t_minfo:    return ("mailbox information");
        case ns_t_rp:       return ("responsible person");
        case ns_t_x25:      return ("X25 address");
        case ns_t_isdn:     return ("ISDN address");
        case ns_t_rt:       return ("router");
        case ns_t_nsap:     return ("nsap address");
        case ns_t_nsap_ptr: return ("domain name pointer");
        case ns_t_sig:      return ("signature");
        case ns_t_key:      return ("key");
        case ns_t_px:       return ("mapping information");
        case ns_t_gpos:     return ("geographical position (withdrawn)");
        case ns_t_aaaa:     return ("IPv6 address");
        case ns_t_loc:      return ("location");
        case ns_t_nxt:      return ("next valid name (unimplemented)");
        case ns_t_eid:      return ("endpoint identifier (unimplemented)");
        case ns_t_nimloc:   return ("NIMROD locator (unimplemented)");
        case ns_t_srv:      return ("server selection");
        case ns_t_atma:     return ("ATM address (unimplemented)");
        case ns_t_ixfr:     return ("incremental zone transfer");
        case ns_t_axfr:     return ("zone transfer");
        case ns_t_mailb:    return ("mailbox-related data (deprecated)");
        case ns_t_maila:    return ("mail agent (deprecated)");
        case ns_t_naptr:    return ("URN Naming Authority");
        case ns_t_any:      return ("\"any\"");
      }
    return ("");
}


/*  -------------------------------------------------------------------------
    Function: ns_strclass - internal

    Synopsis: Return a mnemonic for class.
    -------------------------------------------------------------------------*/

static const char *
ns_strclass (int class)
{
    switch (class)
      {
        case ns_c_in: return ("IN");
        case ns_c_hs: return ("HS");
      }
    return ("");
}

/*  -------------------------------------------------------------------------
    Function: ns_special_char - internal

    Synopsis: Thinking in noninternationalized USASCII (per the DNS spec),
              is this character special ("in need of quoting") ?
              Return boolean value.
    -------------------------------------------------------------------------*/


static Bool
ns_special_char (int ch)
{
    switch (ch)
     {
        case 0x22:                      /* '"'                               */
        case 0x2E:                      /* '.'                               */
        case 0x3B:                      /* ';'                               */
        case 0x5C:                      /* '\\'                              */
        case 0x40:                      /* '@'                               */
        case 0x24:                      /* '$'                               */
            return (TRUE);
        default:
            return (FALSE);
     }
}


static qbyte
ns_get32 (byte *buffer)
{
    qbyte
        feedback;
    NS_GET32 (feedback, buffer);
    return (feedback);
}

static dbyte
ns_get16 (byte *buffer)
{
    dbyte
        feedback;
    NS_GET16 (feedback, buffer);
    return (feedback);
}
