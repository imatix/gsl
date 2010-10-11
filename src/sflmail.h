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
    Synopsis:   Functions to format and send SMTP messages.  Messages
                can contain attachments, and be sent with "cc"'s "bcc"'s as
                well as the normal "to" receivers.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef _sflmail_included               /*  allow multiple inclusions        */
#define _sflmail_included

typedef struct SMTP {
   char *strSmtpServer;
   char *strMessageBody;
   char *strHtmlMessageBody;
   char *strSubject;
   char *strSenderUserId;
   char *strFullSenderUserId;          /* to be filled with: "realname" <e-mail> */
   char *strDestUserIds;
   char *strFullDestUserIds;           /* to be filled with: "realname" <e-mail> */
   char *strCcUserIds;
   char *strFullCcUserIds;             /* to be filled with: "realname" <e-mail> */
   char *strBccUserIds;
   char *strFullBccUserIds;
   char *strRetPathUserId;
   char *strRrcpUserId;
   char *strMsgComment;
   char *strMailerName;
   char *strBinFiles;
   char *strTxtFiles;
   char strlast_smtp_message[513];
   int  debug;
   char *strDebugFile;
   int  mime;
   int  encode_type;
   int  connect_retry_cnt;
   int  retry_wait_time;
   char *strCharSet;                    /* Character Set (default is US-ASCII)*/
} SMTP;

/* Structure used in POP3                                                    */

typedef struct POP_MSG {
   char *from;                          /* Sender name                       */
   char *subject;                       /* Subject of message                */
   char *body;                          /* Body of message                   */
   long  date;                          /* Date of message                   */
   long  time;                          /* Time of message                   */
   long  size;                          /* Total size of message             */
   Bool  multipart;
} POP_MSG;

typedef struct POP3 {
    Bool      leave_on_server;          /* Leave message on server           */
    long      nb_messages;              /* Number of messages                */
    long      total_size;               /* Total message size                */
    POP_MSG **messages;                 /* Table of messages                 */
    sock_t    handle;                   /* Handle of tcp/ip socket           */
    int       message_max_size;         /* Max size for message in KBytes    */
} POP3;

/* SMTP Error value                                                          */

#define SMTP_NO_ERROR                       0
#define SMTP_ERROR_CONNECT                  1
#define SMTP_ERROR_INIT                     2
#define SMTP_ERROR_INVALID_SENDER           3
#define SMTP_ERROR_INVALID_RECEIPT_USER     4
#define SMTP_ERROR_INVALID_DATA             5
#define SMTP_ERROR_MISSING_ATTACH_FILE      6
#define SMTP_ERROR_ON_CLOSE                 7
#define SMTP_ERROR_ON_QUIT                  8
#define SMTP_ERROR_MISSING_DESTINATION      9
#define SMTP_ERROR_MISSING_SUBJECT          10
#define SMTP_ERROR_MISSING_SERVER_NAME      11
#define SMTP_ERROR_MEMORY                   12

#define MAX_SMTP_ERROR                      13


#define ENCODE_BASE64                       0
#define ENCODE_UUENCODE                     1

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif
int   smtp_send_mail_ex      (SMTP *smtp);
int   smtp_send_mail         (char *strSmtpServer,       char *strMessageBody,
                              char *strSubject,          char *strSenderUserId,
                              char *strFullSenderUserId, char *strDestUserIds,
                              char *strFullDestUserIds,  char *strCcUserIds,
                              char *strFullCcUserIds,    char *strBccUserIds,
                              char *strFullBccUserIds,   char *strRetPathUserId,
                              char *strRrcpUserId,       char *strMsgComment,
                              char *strMailerName,       char *strBinFiles,
                              char *strTxtFiles,         char *strDebugFile,
                              char *charset );
char *smtp_error_description (int error_value);

POP3 *pop3_begin             (char *server, char *user, char *passw,
                              Bool secure, Bool leave_on_server,
                              int message_max_size);
int   pop3_list_message      (POP3 **context);
int   pop3_get_message       (POP3 **context, int message_nb);
int   pop3_delete_message    (POP3 **context, int message_nb);
Bool  pop3_end               (POP3 **context);
Bool  pop3_free              (POP3 **context);
void  set_email_debug        (Bool mode);
#ifdef __cplusplus
}
#endif

#endif

