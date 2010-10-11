/*===========================================================================*
 *                                                                           *
 *  smtrdns.c - Reverse-DNS agent                                            *
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
#include "smtrdnsl.h"                   /*  RDNS library functions           */


/*- Definitions -------------------------------------------------------------*/

#undef  AGENT_NAME
#define AGENT_NAME   SMT_RDNS           /*  Our public name                  */
#define BUFFER_SIZE  QUERY_BUFFER_SIZE


typedef struct _USER_DATA               /*  User data list                   */
{
    struct _USER_DATA
          *next, *prev;                 /*    Doubly-linked list             */
    QID
        reply_to;                       /*    Message Queue to reply         */
    qbyte
        tag;                            /*    User tag                       */
} USER_DATA;


typedef struct                          /*  Thread context block:            */
{
    sock_t
        handle;                         /*    Handle for i/o                 */
    word
        readsize;                       /*    Size of input message so far   */
    event_t
        thread_type;                    /*    Thread type indicator          */
    QID
        reply_to;                       /*    Message Queue to reply         */
    qbyte
        user_tag,                       /*    User defined tag               */
        current_ns_ip,                  /*    Current name server ip address */
        ip_address;                     /*    IP address (Network order)     */
    char
        *ip_value,                      /*    IP address in string format    */
        *host_name;                     /*    Host name                      */
    QUERY_BUF
        *query;                         /*    Query buffer                   */
    LIST
        stack,                          /*    Stack list header              */
        reply;                          /*    Reply list header              */
    NS_REQUEST
        *cur_request;                   /*    Current request                */
    SYMTAB
        *invalid_ns_tab;                /*    Invalid Name Server table      */
    byte
        main_req_type;                  /*    Main request type              */
    NS_RR_RESULT
        *rr_result;                     /*    array of RR result from answer */
    dbyte
        rr_result_nbr;                  /*    Number of RR result            */
    Bool
        send_responce;                  /*    True is responce is send       */
} TCB;


/*- Function prototypes -----------------------------------------------------*/

local add_user_data        (TCB *tcb, LIST *head);


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread contect block     */
static QID
    operq,                              /*  Operator console event queue     */
    sockq;                              /*  Socket agent event queue         */

#define MSG_MAX BUFFER_SIZE + 64

static char
    buffer [BUFFER_SIZE + 1];           /*  General-use string buffer        */
extern SYMTAB
    *config;                            /*  Global config file               */
SYMTAB
    *cache_table = NULL;                /*  Symbol tabel for cahe            */
static struct in_addr
    inaddr;                             /*  Used to format IP address        */
static THREAD
    *client_thread;                     /*  Client thread pointer            */
static qbyte
    current_id = 1;                     /*  Current query id                 */

#include "smtrdns.d"                    /*  Include dialog data              */


/********************   INITIALISE AGENT - ENTRY POINT   *********************/

/*  ---------------------------------------------------------------------[<]-
    Function: smtrdns_init

    Synopsis: Initialises the reverse DNS agent.  Returns 0 if initialised
    okay, -1 if there was an error.
    ---------------------------------------------------------------------[>]-*/

int smtrdns_init (void)
{
    AGENT  *agent;                      /*  Handle for our agent             */
    THREAD *thread;                     /*  Handle to various threads        */

#   include "smtrdns.i"                 /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event,     SMT_PRIORITY_MAX);

    /*  Reply events from socket agent                                       */
    declare_smtsock_ok         (ok_event,           0);
    declare_smtsock_connect_ok (ok_event,           0);
    declare_smtsock_closed     (sock_closed_event,  0);
    declare_smtsock_error      (sock_error_event,   0);
    declare_smtsock_timeout    (sock_timeout_event, 0);

    /*  Public methods supported by this agent                               */
    declare_smtrdns_get_host_name (get_host_event,     0);
    declare_smtrdns_get_host_ip   (get_ip_event,       0);

    /*  Reply events from timer agent                                        */
    declare_smttime_reply     (timer_event,      SMT_PRIORITY_LOW);

    /*  Private methods used to pass initial thread events                   */
    method_declare (agent, "_MASTER",        master_event,    0);
    method_declare (agent, "_CLIENT",        client_event,    0);

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        operq = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that socket i/o agent is running, else start it up            */
    smtsock_init ();
    if ((thread = thread_lookup (SMT_SOCKET, "")) != NULL)
        sockq = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that timer agent is running, else start it up                 */
    if (smttime_init ())
        return (-1);

    /*  Create initial thread to manage master port                          */
    if ((thread = thread_create (AGENT_NAME, "")) != NULL)
      {
        SEND (&thread-> queue-> qid, "_MASTER", "");
        ((TCB *) thread-> tcb)-> thread_type = master_event;
      }
    else
        return (-1);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    if (tcb-> thread_type == master_event)
      {
        tcb-> ip_value  = NULL;
        tcb-> host_name = NULL;
        list_reset (&tcb-> stack);      /*  Initialise requests stack        */
        list_reset (&tcb-> reply);      /*  Initialise reply list            */
        cache_table = sym_create_table ();
      }
    tcb-> query          = NULL;
    tcb-> handle         = 0;
    tcb-> readsize       = 0;
    tcb-> current_ns_ip  = 0;
    tcb-> cur_request    = NULL;
    tcb-> invalid_ns_tab = NULL;
    tcb-> rr_result      = NULL;
    tcb-> rr_result_nbr  = 0;
    tcb-> send_responce  = FALSE;
}


/***************************   CHECK THREAD TYPE   ***************************/

MODULE check_thread_type (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    the_next_event = tcb-> thread_type;
}


/*************************   CREATE REQUEST THREAD   *************************/

MODULE create_request_thread (THREAD *thread)
{
    THREAD
        *child;                         /*  Handle to child thread           */
    TCB
        *main_tcb;
    char
        *thread_name = NULL;

    main_tcb = thread-> tcb;

    if (main_tcb-> main_req_type == REQ_TYPE_HOST)
        thread_name = main_tcb-> ip_value;
    else
        thread_name = main_tcb-> host_name;
    if (thread_name == NULL)
        thread_name = "";
    if ((child = thread_create (AGENT_NAME, thread_name)) != NULL)
      {
        event_send (
            &child-> queue-> qid,       /*  Send to child thread queue       */
            &thread-> event-> sender,   /*  Queue for reply                  */
            "_CLIENT",                  /*  Name of event to send            */
            thread-> event-> body,      /*  Event body                       */
            thread-> event-> body_size, /*    and size                       */
            NULL, NULL, NULL,           /*  No response events               */
            0);                         /*  No timeout                       */
        tcb = (TCB *) child-> tcb;

        list_reset (&tcb-> reply);
        add_user_data (main_tcb, &tcb-> reply);
        tcb-> thread_type   = client_event;
        tcb-> main_req_type = main_tcb-> main_req_type;
        tcb-> host_name     = NULL;
        tcb-> ip_value      = NULL;
        if (tcb-> main_req_type == REQ_TYPE_HOST)
          {
            tcb-> ip_address = main_tcb-> ip_address;
            tcb-> ip_value   = mem_strdup (main_tcb-> ip_value);
            mem_strfree (&main_tcb-> ip_value);
          }
        else
          {
            tcb-> host_name = mem_strdup (main_tcb-> host_name);
            mem_strfree (&main_tcb-> host_name);
          }
      }
}

local
add_user_data  (TCB *tcb, LIST *head)
{
    USER_DATA
        *user;

    user = mem_alloc (sizeof (USER_DATA));
    if (user)
      {
        user-> reply_to = tcb-> reply_to;
        user-> tag      = tcb-> user_tag;
        list_reset (user);
        list_relink_after (user, head);
      }
}


/*************************   PREPARE CLIENT THREAD   *************************/

MODULE prepare_client_thread (THREAD *thread)
{
    int
       index;
    NS_REQUEST
       *new_req,
       *request;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (server_list.ns_count == 0)
        raise_exception (initialisation_error_event);

    /* Create first request on the stack with DNS configuration              */
    list_reset (&tcb-> stack);
    request = rdns_request_alloc (tcb-> ip_address, tcb-> ip_value,
                                  tcb-> host_name,
                                  tcb-> main_req_type);
    if (request)
      {
        request-> ns_ip   = server_list.ns_addr [0].sin_addr.s_addr;
        request-> ns_port = server_list.ns_addr [0].sin_port;
        request-> main_request = TRUE;
        request-> main_index   = 0;
        request-> recursive    = server_list.recursive_accept [0];
        list_relink_after (request, &tcb-> stack);
        for (index = 1; index < server_list.ns_count; index++)
          {
            new_req = rdns_request_alloc (tcb-> ip_address, tcb-> ip_value,
                                          tcb-> host_name,
                                          tcb-> main_req_type);
            new_req-> ns_ip   = server_list.ns_addr [index].sin_addr.s_addr;
            new_req-> ns_port = server_list.ns_addr [index].sin_port;
            new_req-> main_request = TRUE;
            new_req-> main_index   = (byte)index;
            new_req-> recursive    = server_list.recursive_accept [index];
            list_relink_after (new_req, request);
            request = new_req;
          }
      }
    tcb-> cur_request = tcb-> stack.next;
    tcb-> invalid_ns_tab = sym_create_table ();
}


/************************   LOAD NAME SERVER CONFIG   ************************/

MODULE load_name_server_config (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    rdns_init (&server_list);
}


/*************************   GET HOST MESSAGE VALUE   ************************/

MODULE get_host_message_value (THREAD *thread)
{
    struct_smtrdns_get_host_name
        *message;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtrdns_get_host_name (thread-> event-> body, &message);

    tcb-> ip_address = message-> ip_address;
    tcb-> ip_value   = mem_copy (message-> ip_string);
    tcb-> user_tag   = message-> request_tag;

    free_smtrdns_get_host_name (&message);
 
    tcb-> reply_to      = thread-> event-> sender;
    tcb-> main_req_type = REQ_TYPE_HOST;
}


void
add_host_line_in_cache (char *line)
{
    char
        **host_list,
        *ip_value;
    SYMBOL
        *symbol;
    int
        index,
        list_size;

    host_list = tok_split (line);
    if (host_list)
      {
        list_size = tok_size (host_list);
        if (list_size >= 2)
          {
            ip_value = host_list [0];
            for (index = 1;
                 index < list_size
                 && *host_list [index] != '#';
                 index++)
              {
                symbol = sym_lookup_symbol (cache_table, host_list [index]);
                if (symbol == NULL)
                  {
                    symbol = sym_assume_symbol (cache_table, host_list [index],
                                                ip_value);
                    if (symbol)
                      {
                        symbol-> data = mem_alloc (sizeof (long));
                        if (symbol-> data)
                            *(long *)symbol-> data = -1;
                      }
                  }
              }
          }
        tok_free (host_list);
      }
}


/************************   LOAD HOST FILE IN CACHE   ************************/

MODULE load_host_file_in_cache (THREAD *thread)
{
    /*  Load content of host file into cache table.  Skip comment line
     *  beginning with '#' character.  The search key in the cache is the
     *  host name, which is a unique value.
     */

    FILE
        *f_host;
    Bool
        read_line = TRUE;
    char
        *p_char;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    f_host = file_open (get_host_file (), 'r');
    if (f_host == NULL)
        return;
    read_line = file_read (f_host, buffer);
    while (read_line)
      {
        p_char = strskp (buffer);
        if (*p_char != '#')
            add_host_line_in_cache (p_char);
        read_line = file_read (f_host, buffer);
      }
    file_close (f_host);
}


/***************************   LOOKUP IP IN CACHE   **************************/

MODULE lookup_ip_in_cache (THREAD *thread)
{
    SYMBOL
        *symbol;
    long
        current_time;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = not_found_event;
    for (symbol = cache_table-> symbols; symbol; symbol = symbol-> next)
      {
         if(symbol-> value
         && streq (symbol-> value, tcb-> cur_request-> host_address))
           {
             current_time = (long)time (NULL);
             if (symbol-> data          /*  Check Time to live               */
             && (   *(long *)symbol-> data == -1
                 || *(long *)symbol-> data >= current_time))
               {
                 if (strneq (symbol-> name, symbol-> value))
                   {
                     mem_strfree (&tcb-> host_name);
                     tcb-> host_name = mem_strdup (symbol-> name);
                     if (dns_debug_mode)
                         coputs ("Found in cache");
                     the_next_event = found_event;
                   }
                 else                   /*  Found bad result in cache        */
                     the_next_event = found_bad_event;
               }
             else
             if (symbol-> data)         /*  Delete cache entry: expired time */
               {
                 mem_free (symbol-> data);
                 sym_delete_symbol (cache_table, symbol);
               }
             break;
           }
      }
}


/**************************   LOOKUP HOST IN CACHE   *************************/

MODULE lookup_host_in_cache (THREAD *thread)
{
    SYMBOL
        *symbol;
    long
        current_time;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    symbol = sym_lookup_symbol (cache_table, tcb-> cur_request-> host_name);
    if (symbol)
      {
        current_time = (long)time (NULL);
        if (symbol-> data               /*  Check Time to live               */
        && (   *(long *)symbol-> data == -1
            || *(long *)symbol-> data >= current_time))
          {
            if (symbol-> value)
              {
                mem_strfree (&tcb-> ip_value);
                tcb-> ip_value    = mem_strdup (symbol-> value);
                tcb-> ip_address  = inet_addr (tcb-> ip_value);
                if (dns_debug_mode)
                    coputs ("Found in cache");
                the_next_event    = found_event;
              }
            else
                the_next_event    = found_bad_event;
          }
        else
          {
            if (symbol-> data)          /*  Delete cache entry: expired time */
              {
                mem_free (symbol-> data);
                sym_delete_symbol (cache_table, symbol);
              }
            the_next_event = not_found_event;
          }
      }
    else
        the_next_event = not_found_event;
}


/***************************   CONNECT TO SERVER   ***************************/

MODULE connect_to_server (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (dns_debug_mode)
      {
        inaddr.s_addr = tcb-> cur_request-> ns_ip;
        coprintf ("Connect to : %s (%s) port %d",
                  inet_ntoa (inaddr),
                  tcb-> cur_request-> ns_name?
                      tcb-> cur_request-> ns_name: "",
                  ntohs (tcb-> cur_request-> ns_port));
      }
    tcb-> current_ns_ip = tcb-> cur_request-> ns_ip;

    send_smtsock_connect (
        &sockq,
        30,                             /*  Time out for connect             */
        "udp",
        NULL,
        NULL,
        ntohs (tcb-> cur_request-> ns_port),
        ntohl (tcb-> cur_request-> ns_ip),
        0);
}


/**************************   SEND QUERY TO SERVER   *************************/

MODULE send_query_to_server (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtsock_write (
        &sockq,                         /*  Send to specified queue          */
        CONNECT_TIMEOUT,                /*  Timeout for request              */
        tcb-> handle,                   /*  Socket to write to               */
        tcb-> readsize,                 /*  Amount of data to write          */
        (byte *) tcb-> query,           /*  Address of data block            */
        TRUE,                           /*  We do want a reply               */
        0);                             /*  No request tag                   */

    tcb-> readsize = 0;
}


/***************************   GET SOCKET HANDLE   ***************************/

MODULE get_socket_handle (THREAD *thread)
{
    struct_smtsock_connect_ok
        *message;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_connect_ok (thread-> event-> body, &message);
    tcb-> handle = message-> socket;
    free_smtsock_connect_ok (&message);
}


/************************   READ ANSWER FROM SERVER   ************************/

MODULE read_answer_from_server (THREAD *thread)
{
    int
        rc;                             /*  Return code from read            */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    rc = read_TCP (tcb-> handle, buffer + tcb-> readsize,
                                 BUFFER_SIZE - tcb-> readsize);
    if (rc > 0)
      {
        tcb-> readsize += rc;       /*  We read something                */
        memset (tcb-> query, 0, sizeof (QUERY_BUF));
        memcpy (tcb-> query, buffer, tcb-> readsize);
      }
    else
    if (rc == 0 || sockerrno == EPIPE || sockerrno == ECONNRESET)
        raise_exception (sock_closed_event);
    else
    if (sockerrno == EAGAIN || sockerrno == EWOULDBLOCK)
        raise_exception (sock_retry_event);
    else
      {
        trace ("Socket error: %s", sockmsg ());
        raise_exception (sock_error_event);
      }

}


/**********************   CHECK IF ANSWER IS COMPLETE   **********************/

MODULE check_if_answer_is_complete (THREAD *thread)
{
    Bool
        complete;
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    complete = rdns_check_answer_size (tcb-> query, tcb-> readsize);
    if (complete == FALSE)
        raise_exception (read_more_event);
}


/******************************   SIGNAL ERROR   *****************************/

MODULE signal_error (THREAD *thread)
{
    USER_DATA
        *next_data,
        *user_data;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> send_responce = TRUE;
    user_data = tcb-> reply.next;
    while (user_data && (void *)user_data != (void *)&tcb-> reply)
      {
        next_data = user_data-> next;
        send_smtrdns_host_error (&user_data-> reply_to);
        user_data = next_data;
      }
}


/*************************   WAIT FOR SOCKET INPUT   *************************/

MODULE wait_for_socket_input (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtsock_input (
        &sockq,                         /*  Send to specified queue          */
        (dbyte) (tcb-> cur_request-> recursive ? RECURSIVE_TIMEOUT
                                               : REQUEST_TIMEOUT),
        tcb-> handle,
        0);
}


/***************************   GET ANSWER RESULT   ***************************/

MODULE get_answer_result (THREAD *thread)
{
    Bool
        host;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    host = rdns_read_answer (tcb-> query,
                             tcb-> readsize,
                             tcb-> cur_request,
                             tcb-> invalid_ns_tab,
                             tcb-> rr_result,
                             tcb-> rr_result_nbr);

    if (host)                           /*  Found a result                   */
      {
        if (tcb-> main_req_type == REQ_TYPE_HOST)
          {
            if (tcb-> cur_request-> host_address
            &&  streq (tcb-> cur_request-> host_address, tcb-> ip_value))
              {
                tcb-> host_name = mem_strdup (tcb-> cur_request-> host_name);
                the_next_event  = host_name_event;
              }
            else
                the_next_event   = ip_value_event;
          }
        else
          {
            if (tcb-> host_name
            &&  streq (tcb-> host_name, tcb-> cur_request-> host_name))
              {
                tcb-> ip_value = mem_strdup (tcb-> cur_request-> host_address);
                tcb-> ip_address = tcb-> cur_request-> host_ip;
                the_next_event = ip_value_event;
              }
            else
                the_next_event  = host_name_event;
          }
      }
    else
        the_next_event = name_server_event;

    if (tcb-> rr_result)
      {
        mem_free (tcb-> rr_result);
        tcb-> rr_result = NULL;
      }
}


/************************   GET CURRENT REQUEST TYPE   ***********************/

MODULE get_current_request_type (THREAD *thread)
{
    NS_REQUEST
        *request;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    request = tcb-> cur_request;

    if (request-> host_address == NULL
    &&  request-> host_name    == NULL
    &&  request-> ns_ip != 0)
        request-> host_address = mem_strdup (tcb-> ip_value);

    if (request-> type == REQ_TYPE_HOST
    &&  streq (tcb-> ip_value, request-> host_address))
        the_next_event = host_name_event;
    else
    if (request-> type == REQ_TYPE_IP
    &&  request-> host_name != NULL
    &&  tcb-> host_name
    &&  streq (tcb-> host_name, request-> host_name))
        the_next_event = ip_value_event;
    else
        the_next_event = name_server_ip_event;
}


/****************************   GET NEXT REQUEST   ***************************/

MODULE get_next_request (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> cur_request-> host_name
    && !(   tcb-> main_req_type == REQ_TYPE_IP
         && streq (tcb-> host_name, tcb-> cur_request-> host_name))
       )
        sym_assume_symbol (tcb-> invalid_ns_tab,
                           tcb-> cur_request-> host_name, " ");

    rdns_request_free (tcb-> cur_request);
    tcb-> cur_request = tcb-> stack.next;
    current_id++;
    if (tcb-> cur_request != NULL
    &&  (void *)tcb-> cur_request != (void *)&tcb-> stack)
        the_next_event = ok_event;
    else
    if ((void *)tcb-> cur_request == (void *)&tcb-> stack)
        the_next_event = end_event;
    else
        the_next_event = error_event;
}


/**************************   MAKE IP QUERY BUFFER   *************************/

MODULE make_ip_query_buffer (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> query = rdns_make_ip_query (tcb-> cur_request-> host_name,
                                      current_id,
                                      &tcb-> readsize,
                                      tcb-> cur_request-> recursive);
}


/***********************   MAKE REVERSE QUERY BUFFER   ***********************/

MODULE make_reverse_query_buffer (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    tcb-> query = rdns_make_query (tcb-> cur_request-> host_address,
                                   current_id,
                                   &tcb-> readsize,
                                   tcb-> cur_request-> recursive);
}


/**************************   SEND HOST NAME VALUE   *************************/

MODULE send_host_name_value (THREAD *thread)
{
    USER_DATA
        *next_data,
        *user_data;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> ip_address == 0
    &&  tcb-> ip_value)
        tcb-> ip_address = inet_addr (tcb-> ip_value);

    tcb-> send_responce = TRUE;
    user_data = tcb-> reply.next;
    while (user_data && (void *)user_data != (void *)&tcb-> reply)
      {
        next_data = user_data-> next;

        send_smtrdns_host_name (
            &user_data-> reply_to,      /*  Send to specified queue          */
            tcb-> ip_address,
            tcb-> host_name,
            user_data-> tag);

        user_data = next_data;
      }
}


/**************************   CHECK SERVER ADDRESS   *************************/

MODULE check_server_address (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> cur_request-> ns_ip != 0)
        the_next_event = ok_event;
    else
        the_next_event = error_event;
}


/********************   CLEAN PREVIOUS REQUEST RESOURCE   ********************/

MODULE clean_previous_request_resource (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> handle)
        close_socket (tcb-> handle);

    if (tcb-> query)
        mem_free (tcb-> query);
    tcb-> handle = 0;
    tcb-> query  = NULL;
}


/*****************   CHECK REQUEST IN INVALID SERVER TABLE   *****************/

MODULE check_request_in_invalid_server_table (THREAD *thread)
{
    char
        *value = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> cur_request-> host_name)
        value = tcb-> cur_request-> host_name;

    the_next_event = ok_event;
    if (value)
      {
        if (sym_lookup_symbol (tcb-> invalid_ns_tab, value) != NULL)
            the_next_event = invalid_event;
      }
}


/**************************   SIGNAL END OF SEARCH   *************************/

MODULE signal_end_of_search (THREAD *thread)
{
    USER_DATA
        *next_data,
        *user_data;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> send_responce = TRUE;
    user_data = tcb-> reply.next;
    while (user_data && (void *)user_data != (void *)&tcb-> reply)
      {
        next_data = user_data-> next;
        send_smtrdns_host_end (&user_data-> reply_to);
        user_data = next_data;
      }
}


/**************************   CREATE HOST REQUEST   **************************/

MODULE create_host_request (THREAD *thread)
{
    NS_REQUEST
        *request;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> cur_request
    &&  tcb-> cur_request-> type == REQ_TYPE_IP)
      {
        request = rdns_request_alloc (tcb-> ip_address, tcb-> ip_value, NULL,
                                      REQ_TYPE_HOST);
        if (request)
          {
            request-> ns_ip   = tcb-> cur_request-> host_ip;
            request-> ns_name = mem_strdup (tcb-> cur_request-> host_name);
            list_relink_after (request, tcb-> cur_request);
          }
      }
}


/******************   PURGE EQUIVALENT REQUEST FROM STACK   ******************/

MODULE purge_equivalent_request_from_stack (THREAD *thread)
{
    NS_REQUEST
        *request,
        *next;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    request = tcb-> cur_request-> next;
    while (request
    &&     (void *)request-> next != (void *)&tcb-> stack)
      {
        next = request-> next;
        if (request-> type == tcb-> cur_request-> type)
          {
            if (request-> type == (byte)REQ_TYPE_IP
            &&  request-> host_name
            &&  tcb-> cur_request-> host_name
            &&  streq (request-> host_name, tcb-> cur_request-> host_name)
               )
                rdns_request_free (request);
            else
            if (request-> type == (byte)REQ_TYPE_HOST
            &&  request-> host_address
            &&  tcb-> cur_request-> host_address
            &&  streq (request-> host_address, tcb-> cur_request-> host_address)
               )
                rdns_request_free (request);
          }
        request = next;
      }
}


/**************************   GET IP MESSAGE VALUE   *************************/

MODULE get_ip_message_value (THREAD *thread)
{
    struct_smtrdns_get_host_ip
        *message;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtrdns_get_host_ip (thread-> event-> body, &message);

    tcb-> host_name = mem_copy (message-> host_name);
    tcb-> user_tag  = message-> request_tag;

    free_smtrdns_get_host_ip (&message);

    tcb-> reply_to = thread-> event-> sender;
    tcb-> main_req_type = REQ_TYPE_IP;
}


/***************************   CREATE IP REQUEST   ***************************/

MODULE create_ip_request (THREAD *thread)
{
    NS_REQUEST
        *request;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> cur_request
    &&  tcb-> cur_request-> type == REQ_TYPE_IP)
      {
        request = rdns_request_alloc (tcb-> ip_address, tcb-> ip_value,
                                      tcb-> host_name,
                                      REQ_TYPE_IP);
        if (request)
          {
            request-> ns_ip   = tcb-> cur_request-> host_ip;
            request-> ns_name = mem_copy (tcb-> cur_request-> host_name);
            list_relink_after (request, tcb-> cur_request);
          }
      }
}


/*************************   GET MAIN REQUEST TYPE   *************************/

MODULE get_main_request_type (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    if (tcb-> main_req_type == REQ_TYPE_HOST)
        the_next_event = host_name_event;
    else
        the_next_event = ip_value_event;
}


/*************************   SEND IP ADDRESS VALUE   *************************/

MODULE send_ip_address_value (THREAD *thread)
{
    USER_DATA
        *next_data,
        *user_data;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> send_responce = TRUE;
    user_data = tcb-> reply.next;
    while (user_data && (void *)user_data != (void *)&tcb-> reply)
      {
        next_data = user_data-> next;

        send_smtrdns_host_ip (
            &user_data-> reply_to,      /*  Send to specified queue          */
            (qbyte)tcb-> ip_address,
            tcb-> host_name, 
            user_data-> tag);

        user_data = next_data;
      }
}

/******************************   ADD TO CACHE   *****************************/

MODULE add_to_cache (THREAD *thread)
{
    SYMBOL
        *symbol;
    time_t
        current_time;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (cache_table
    &&  tcb-> host_name
    &&  tcb-> ip_address)
      {
        symbol = sym_assume_symbol (cache_table, tcb-> host_name,
                                    tcb-> ip_value);
        if (symbol-> data)
            mem_free (symbol-> data);
        symbol-> data = mem_alloc (sizeof (long));
        if (symbol-> data)
          {
            current_time = time (NULL);
            *(long *)symbol-> data =   (long)current_time
                                     + tcb-> cur_request-> ttl;
          }
      }
}

/**************************   CREATE TIMEOUT ALARM   *************************/

MODULE create_timeout_alarm (THREAD *thread)
{
    /*  Ask timer to send us an event after the monitor timeout              */
    smttime_request_alarm (
        (qbyte) 0,
        (qbyte)QUERY_TIMEOUT * 100, 
        0);
}


/**************************   FLUSH TIMEOUT ALARM   **************************/

MODULE flush_timeout_alarm (THREAD *thread)
{
    smttime_request_flush ();
}


/*****************************   SIGNAL TIMEOUT   ****************************/

MODULE signal_timeout (THREAD *thread)
{
    USER_DATA
        *next_data,
        *user_data;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> send_responce = TRUE;
    user_data = tcb-> reply.next;
    while (user_data && (void *)user_data != (void *)&tcb-> reply)
      {
        next_data = user_data-> next;
        send_smtrdns_host_timeout (&user_data-> reply_to);
        user_data = next_data;
      }
}

/****************************   GET NUMBER OF RR   ***************************/

MODULE get_number_of_rr (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> rr_result_nbr = rdns_get_nbr_rr_result (tcb-> query);
    if (tcb-> rr_result_nbr > 0)
      {
        tcb-> rr_result = mem_alloc (tcb-> rr_result_nbr
                                     * sizeof (NS_RR_RESULT));
        if (tcb-> rr_result)
            memset (tcb-> rr_result, 0,
                    tcb-> rr_result_nbr * sizeof (NS_RR_RESULT));
      }
}

/*********************   ADD USER DATA IN CLIENT THREAD   ********************/

MODULE add_user_data_in_client_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    add_user_data (tcb, &((TCB *)client_thread-> tcb)-> reply);
    if (tcb-> main_req_type == REQ_TYPE_HOST)
        mem_strfree (&tcb-> ip_value);
    else
        mem_strfree (&tcb-> host_name);
}


/***********************   SEARCH THREAD FOR REQUEST   ***********************/

MODULE search_thread_for_request (THREAD *thread)
{
    char
         *thread_name = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> main_req_type == REQ_TYPE_HOST)
        thread_name = tcb-> ip_value;
    else
        thread_name = tcb-> host_name;

    if (thread_name == NULL)
      {
        the_next_event = not_found_event;
        return;
      }
    if ((client_thread = thread_lookup (AGENT_NAME, thread_name)) != NULL)
      {
        if (((TCB *)client_thread-> tcb)-> send_responce == FALSE)
            the_next_event = found_event;
        else
            the_next_event = not_found_event;
      }
    else
        the_next_event = not_found_event;
}


/*****************************   REFRESH CACHE   *****************************/

MODULE refresh_cache (THREAD *thread)
{
    long
        current_time;
    SYMBOL
        *symbol,
        *next;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    current_time = (long)time (NULL);
    symbol = cache_table-> symbols;

    while (symbol)
      {
        next = symbol-> next;

        /* Delete old value of cache                                         */
        if (symbol-> data
        &&  *(long *)symbol-> data != -1
        &&  *(long *)symbol-> data < current_time)
          {
            if (symbol-> prev)
                symbol-> prev-> next = symbol-> next;
            else
                cache_table-> symbols = symbol-> next;

            if (symbol-> next)
                symbol-> next-> prev = symbol-> prev;

            if (symbol-> h_prev)
                symbol-> h_prev-> h_next = symbol-> h_next;
            else
                cache_table-> hash [symbol-> hash] = symbol-> h_next;

            if (symbol-> h_next)
                symbol-> h_next-> h_prev = symbol-> h_prev;

            cache_table-> size--;
            mem_free (symbol-> value);
            mem_free (symbol-> data);
            mem_free (symbol);
         }
        symbol = next;
      }
}


/************************   SET REFRESH CACHE TIMER   ************************/

MODULE set_refresh_cache_timer (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    smttime_request_alarm (
        (qbyte) 0,
        (qbyte)REFRESH_CACHE_TIME * 100, 
        0);
}


/************************   ADD BAD RESULT TO CACHE   ************************/

MODULE add_bad_result_to_cache (THREAD *thread)
{
    SYMBOL
        *symbol;
    time_t
        current_time;
    char
        *ip_value;

    tcb = thread-> tcb;                 /*  Point to thread's context        */
    if (cache_table
    &&  (   tcb-> host_name
         || tcb-> ip_value
         || tcb-> ip_address))
      {
        if (tcb-> host_name)
            symbol = sym_assume_symbol (cache_table, tcb-> host_name, NULL);
        else
        if (tcb-> ip_value)
            symbol = sym_assume_symbol (cache_table, tcb-> ip_value,
                                                     tcb-> ip_value);
        else
          {
            inaddr.s_addr = tcb-> ip_address;
            ip_value = inet_ntoa (inaddr);
            symbol = sym_assume_symbol (cache_table, ip_value, ip_value);
          }

        if (symbol-> data)
            mem_free (symbol-> data);
        symbol-> data = mem_alloc (sizeof (long));
        if (symbol-> data)
          {
            current_time = time (NULL);
            *(long *) symbol-> data = (long) current_time + BAD_RESULT_TTL;
          }
      }
}


/*************************   CHECK RECURSIVE VALUE   *************************/

MODULE check_recursive_value (THREAD *thread)
{
    Bool
        ra;
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ra  = rdns_is_recursive (tcb-> query);
    if (tcb-> cur_request-> recursive == 1
    &&  ra == FALSE)
        the_next_event = invalid_event;
    else
        the_next_event = ok_event;
}


/***********************   SET NEW VALUE OF RECURSIVE   **********************/

MODULE set_new_value_of_recursive (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> cur_request-> recursive = FALSE;
    if (tcb-> cur_request-> main_request)
        server_list.recursive_accept [tcb-> cur_request-> main_index] = FALSE;
}


/***********************   FREE ALL SERVER RESOURCES   ***********************/

MODULE free_all_server_resources (THREAD *thread)
{
    SYMBOL
        *symbol,                        /*  Pointer to symbol                */
        *next = NULL;                   /*    and to next symbol in list     */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (cache_table)
      {
        for (symbol = cache_table-> symbols; symbol; symbol = next)
          {
            next = symbol-> next;
            if (symbol-> value)
                mem_free (symbol-> value);
            if (symbol-> data)
                mem_free (symbol-> data);
            mem_free (symbol);
          }
        mem_free (cache_table);
      }
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    NS_REQUEST
        *next,
        *request;
    USER_DATA
        *next_data,
        *user_data;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_strfree (&tcb-> ip_value);
    mem_strfree (&tcb-> host_name);

    if (tcb-> handle)
        close_socket (tcb-> handle);

    if (tcb-> query)
        mem_free (tcb-> query);

    /* Free stack of requests                                                */
    request = tcb-> stack.next;
    while ((void *)request != (void *)&tcb-> stack)
      {
        next = request-> next;
        rdns_request_free (request);
        request = next;
      }

    /* Free reply list                                                       */
    user_data = tcb-> reply.next;
    while ((void *)user_data != (void *)&tcb-> reply)
      {
        next_data = user_data-> next;
        mem_free (user_data);
        user_data = next_data;
      }
    if (tcb-> invalid_ns_tab)
        sym_delete_table (tcb-> invalid_ns_tab);

    if (tcb-> rr_result)
        mem_free (tcb-> rr_result);

    the_next_event = terminate_event;
}
