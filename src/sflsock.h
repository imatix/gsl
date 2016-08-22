/*===========================================================================*
 *                                                                           *
 *  sflsock.h - Socket handling functions                                    *
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
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Provides functions to create, read, and write TCP and UDP
                sockets.  Encapsulates system dependencies.  Tested under
                MS Winsock, UNIX (Linux, AIX, SunOs), OpenVMS.  Some of the
                code in this module was based on the book "Internetworking
                With TCP/IP Volume III: Client-Server Programming And
                Applications BSD Socket Version" by Douglas E. Comer and
                David L. Stevens, published 1993 by Prentice-Hall Inc.
                ISBN 0-13-020272-X.  Defines sock_t which you should use
                for all sockets.  If you need to call a native socket
                function, use a (SOCKET) cast on the sock_t handle.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLSOCK_INCLUDED               /*  Allow multiple inclusions        */
#define SFLSOCK_INCLUDED


/*---------------------------------------------------------------------------
 *  Non-portable definitions - currently for Winsocks
 *  winsock.h is included in the prelude.h file.
 */

#if (defined (__WINDOWS__))
#   define sockerrno     winsock_last_error ()
#   undef INVALID_SOCKET                /*  MSVC defines it in winsock.h     */
#   if (defined (WIN32) && defined (_MSC_VER))
#       pragma comment (lib, "wsock32")

#       if defined (FD_SET)
#           undef  FD_SET
#           define FD_SET(fd, set) do {                                     \
              if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE              \
              &&  !FD_ISSET ((fd), (set)))                                  \
                ((fd_set FAR *)(set))-> fd_array [((fd_set FAR *)(set))->fd_count++]  \
                    = (fd);\
              } while(0)
#       endif
#   endif
#else
#   define sockerrno     errno          /*  Use sockerrno for error number   */
#   undef  SOCKET_ERROR
#   define SOCKET_ERROR  -1             /*  Error on socket function         */
    typedef int SOCKET;                 /*  Use (SOCKET) for system calls    */
#endif


/*---------------------------------------------------------------------------
 *  Standard TCP/IP constants
 */

#define DNS_PORT         53             /*  Domain Name server port          */


/*---------------------------------------------------------------------------
 *  Define the socket type 'sock_t' and various types and constants.
 */

typedef qbyte sock_t;                   /*  Use sock_t for all sockets       */

/*  Argument sizes are "int", on most systems, even though negative values
 *  do not make sense.  GNU libc 2 (aka Linux libc6) changed argument sizes
 *  to be "socklen_t", ie unsigned int, at least as of GNU libc 2.0.7.
 *  OpenVMS Dec C defines these as "unsigned int".
 */

#if (defined (__GLIBC__) && (__GLIBC__ > 1))
typedef socklen_t  argsize_t;           /*  GNU libc: size arg for sock func */

#elif (defined (__VMS__) && !defined (vaxc))
typedef unsigned int argsize_t;         /*  OpenVMS: size arg for sock func  */

#else
typedef int argsize_t;                  /*  Traditional: size for sock func  */
#endif

#undef  INVALID_SOCKET
#define INVALID_SOCKET   (sock_t) -1    /*  Invalid socket handle            */
#undef  SOCKET_LOOPBACK
#define SOCKET_LOOPBACK  0x7f000001L    /*  Loopback address 127.0.0.1       */


/*---------------------------------------------------------------------------
 *  Fake socket definitions
 *
 *  If the system does not support sockets, we'll fake it so that the code
 *  still works.  Under crippleware OS's this will work so that all devices
 *  are always ready for I/O.  Most of this code is adapted from the Linux
 *  time.h file.  We also define some useful structures.
 */

#if (!defined (DOES_SOCKETS))
#   undef  INADDR_ANY
#   define INADDR_ANY       0

    /*  If FAKE_SOCKETS is set, sflsock will fake basic socket i/o           */
#   define FAKE_SOCKETS     1

    /*  Number of descriptors that can fit in an `fd_set'.                   */
#   undef  FD_SETSIZE
#   define FD_SETSIZE       256

    /*  It's easier to assume 8-bit bytes than to get CHAR_BIT.              */
#   define NFDBITS          (sizeof (unsigned long int) * 8)
#   define __FDELT(d)       ((d) / NFDBITS)
#   define __FDMASK(d)      (1 << ((d) % NFDBITS))
#   define FD_ZERO(set)     ((void) memset((void *) (set), 0, sizeof(fd_set)))
#   define FD_SET(d,set)    ((set)->__bits[__FDELT(d)] |=  __FDMASK(d))
#   define FD_CLR(d,set)    ((set)->__bits[__FDELT(d)] &= ~__FDMASK(d))
#   define FD_ISSET(d,set)  ((set)->__bits[__FDELT(d)] &   __FDMASK(d))

    /*  Fake the inet conversion functions                                   */
#   define inet_addr(x)     1
#   define inet_ntoa(x)     "127.0.0.1"

#   if (0)
    /*  Define the fd_set structure for select()                             */
    typedef struct {
        qbyte __bits [(FD_SETSIZE + (NFDBITS - 1)) / NFDBITS];
    } fd_set;

    /*  Define the timeval structure for select()                            */
    struct timeval {
        long  tv_sec;                   /*  Seconds                          */
        long  tv_usec;                  /*  Microseconds                     */
    };
#   endif

#   undef  AF_INET
#   define AF_INET          0

    /*  Define net-to-host conversion macros                                 */
#   if (!defined ntohs)
#      if (defined __MSDOS__)
#          define ntohs(x) (((x) & 0xff00U) >> 8) +        \
                           (((x) & 0x00ffU) << 8)
#          define ntohl(x) (((x) & 0xff000000UL) >> 24) +  \
                           (((x) & 0x00ff0000UL) >> 8 ) +  \
                           (((x) & 0x0000ff00UL) << 8 ) +  \
                           (((x) & 0x000000ffUL) << 24)
#          define htons(x) ntohs(x)
#          define htonl(x) ntohl(x)
#      else
#          define ntohs(x) (x)
#          define ntohl(x) (x)
#          define htons(x) (x)
#          define htonl(x) (x)
#      endif                            /*  msdos                            */
#   endif
#endif                                  /*  Define fake sockets              */


/*---------------------------------------------------------------------------
 *  Fix for BeOS and other systems which don't have a working select() function.
 */

#if (defined FAKE_SOCKETS || defined __UTYPE_BEOS)
    /*  Fake a select() function that just returns 1                         */
#   define select(n,rf,wf,xf,t)         1
#endif

/*---------------------------------------------------------------------------
 *  Definitions for compilers which don't have net support
 *  in their header libraries. Expand as required.
 */

#if (defined __UTYPE_BEOS)
    struct protoent {                   /*  Protocol entry                   */
        char  *p_name;
        char **p_aliases;
        int    p_proto;
    };
    struct protoent *getprotobynumber (int);
    struct protoent *getprotobyname   (const char *);
    struct servent  *getservbyport    (int, const char *);
#endif

/*---------------------------------------------------------------------------
 *  Required definitions
 *
 *  These are symbols used by calling programs: if the compiler does not
 *  define them, we do it.
 */

#if (!defined (EWOULDBLOCK))            /*  BSD tends to use EWOULDBLOCK     */
#   if (defined (__WINDOWS__))
#       define EWOULDBLOCK  WSAEWOULDBLOCK
#   else
#       define EWOULDBLOCK  -1
#   endif
#endif

#if (!defined (EINPROGRESS))            /*  Return code for asynch I/O       */
#   if (defined (__WINDOWS__))
#       define EINPROGRESS  WSAEINPROGRESS
#   else
#       define EINPROGRESS  EWOULDBLOCK
#   endif
#endif

#if (!defined (EAGAIN))                 /*  Return code for asynch I/O       */
#   define EAGAIN           EWOULDBLOCK
#endif

#if (!defined (EPIPE))                  /*  Return code for closed socket    */
#   define EPIPE            -1
#endif

#if (!defined (ECONNRESET))             /*  Return code for closed socket    */
#   if (defined (__WINDOWS__))
#       define ECONNRESET   WSAECONNRESET
#   else
#       define ECONNRESET   EPIPE
#   endif
#endif

#if (defined (__UTYPE_HPUX))
#   define FD_SETTYPE (int *)           /*  Some systems use the older       */
#else                                   /*    select() format                */
#   define FD_SETTYPE                   /*  Most use fd_set's                */
#endif

#if (!defined (MAXHOSTNAMELEN))         /*  Some guys don't define this      */
#   define MAXHOSTNAMELEN   256         /*    constant                       */
#endif
#if (!defined (INADDR_NONE))            /*  Some guys don't define this      */
#   define INADDR_NONE      -1          /*    constant                       */
#endif


/*---------------------------------------------------------------------------
 *  Error values returned by connect_error(). These reflect the last problem
 *  detected by one of the passive/connect connection functions.
 */

#define IP_NOERROR          0           /*  No errors                        */
#define IP_NOSOCKETS        1           /*  Sockets not supported            */
#define IP_BADHOST          2           /*  Host not known                   */
#define IP_BADSERVICE       3           /*  Service or port not known        */
#define IP_BADPROTOCOL      4           /*  Invalid protocol specified       */
#define IP_SOCKETERROR      5           /*  Error creating socket            */
#define IP_CONNECTERROR     6           /*  Error making connection          */
#define IP_BINDERROR        7           /*  Error binding socket             */
#define IP_LISTENERROR      8           /*  Error preparing to listen        */

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 *  The ip_portbase is added to the port number when creating a service;
 *  you can set this variable before calling passive_TCP or passive_UDP.
 */

extern int
    ip_portbase;

/*---------------------------------------------------------------------------
 *  The ip_nonblock flag determines whether sockets are blocking or not.
 *  Under Windows sockets are never blocking.  Under UNIX they may be, or
 *  not.  For portability to Windows, this flag is set to TRUE by default.
 *  The main consequence of this is that after a connect_xxx() call you may
 *  need to perform a select() on the socket for writing to determine when
 *  the connection has suceeded.
 */

extern Bool
    ip_nonblock;

/*---------------------------------------------------------------------------
 *  The connect_errlist table provides messages for the connect_error()
 *  values.
 */

extern char
    *connect_errlist [];

/*---------------------------------------------------------------------------
 *  The ip_passive address is used for passive socket connections.  Initially
 *  this is set to INADDR_ANY.  It is used for one passive socket creation
 *  and reset to INADDR_ANY thereafter.
 */

extern qbyte
    ip_passive;

extern int
    ip_sockets;                         /*  Number of open sockets           */


/*---------------------------------------------------------------------------
 *  Function prototypes
 */

int    sock_init            (void);
int    sock_term            (void);
sock_t passive_TCP          (const char *service, int queue);
sock_t passive_UDP          (const char *service);
sock_t passive_socket       (const char *service, const char *protocol,
                             int queue);
sock_t connect_TCP          (const char *host, const char *service);
sock_t connect_UDP          (const char *host, const char *service);
sock_t connect_TCP_fast     (const struct sockaddr_in *sin);
sock_t connect_UDP_fast     (const struct sockaddr_in *sin);
sock_t connect_socket       (const char *host, const char *service, const char
                             *protocol, const struct sockaddr_in *sin,
                             int retry_max, int retry_delay);
int    connect_to_peer      (sock_t handle,
                             const struct sockaddr_in *sin);
int    connect_error        (void);
sock_t accept_socket        (sock_t master);
sock_t create_socket        (const char *protocol);
int    address_end_point    (const char *host, const char *service, const
                             char *protocol, struct sockaddr_in *sin);
int    get_sock_addr        (sock_t handle, struct sockaddr_in *sin,
                             char *name, int namesize);
int    get_peer_addr        (sock_t handle, struct sockaddr_in *sin,
                             char *name, int namesize);
void   build_sockaddr       (struct sockaddr_in *sin, qbyte host, dbyte port);
char  *socket_localaddr     (sock_t handle);
char  *socket_peeraddr      (sock_t handle);
Bool   socket_is_alive      (sock_t handle);
int    socket_error         (sock_t handle);
int    socket_nodelay       (sock_t handle);
int    read_TCP             (sock_t handle, void *buffer, size_t length);
int    write_TCP            (sock_t handle, const void *buffer, size_t length);
int    read_UDP             (sock_t handle, void *buffer, size_t length,
                             const struct sockaddr_in *sin);
int    write_UDP            (sock_t handle, const void *buffer, size_t length,
                             const struct sockaddr_in *sin);
int    close_socket         (sock_t handle);
int    sock_select          (int nfds, fd_set *readfds, fd_set *writefds,
                             fd_set *errorfds, struct timeval *timeout);
char  *get_hostname         (void);
qbyte  get_hostaddr         (void);
qbyte *get_hostaddrs        (void);
const  char *sockmsg        (void);
Bool   socket_is_permitted  (const char *address, const char *mask);
char  *sock_ntoa            (qbyte address);
char  *get_host_file        (void);
int    get_name_server      (struct sockaddr_in *ns_address, int ns_max);

#if (defined (__WINDOWS__))
int    winsock_last_error   (void);
#endif

#ifdef __cplusplus
}
#endif

/*  Macros for compatibility with previous versions                          */
#define socket_hostaddr(handle) socket_peeraddr (handle)


#endif                                  /*  Include sflsock.h                */
