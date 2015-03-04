/*===========================================================================*
 *                                                                           *
 *  sflmail.c - synchronous email send/receive functions                     *
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

#include "prelude.h"                    /*  Universal header file            */
#include "sflstr.h"
#include "sflsock.h"
#include "sfldate.h"
#include "sflmem.h"
#include "sflmime.h"
#include "sflmail.h"
#include "sflsymb.h"
#include "sflhttp.h"
#include "sflcons.h"

/*- Global variables --------------------------------------------------------*/
Bool sock_error_flag,
     sock_read_flag,
     sock_write_flag;

static Bool
     email_debug = FALSE;

/*- Macros & defines --------------------------------------------------------*/

/*  Macro to encoding a char and make it printable.                          */
#define ENC(c) ((c) ? ((c) & 077) + ' ': '`')

/* Macro to write string to a socket                                         */
#define send_data(sock,strout)                                               \
      if (email_debug) coprintf ("Email Send: %s", (strout));                \
      write_TCP((sock),(strout),strlen((strout)))

#define send_body_data(sock,strout)                                          \
      if (*strout == '.' && *(strout + 1) != '.')                            \
          write_TCP((sock), ".", 1);                                         \
      if (email_debug) coprintf ("Email Send: %s", (strout));                \
      write_TCP((sock),(strout),strlen((strout)))


#define SMTP_SERVER_ERROR    400
#define BUFFER_SIZE          512        /* Maximum size of a line            */
#define LINE_SIZE            77

#define MESSAGE_BOUNDARY "----=_NextPart_000_0144_01BF935D"

#define CLEAN_SEND_MAIL                                                         \
   mem_strfree  (&quoted_subject);                                              \
   close_socket (socket_handle);        /* Close the port up.                */ \
   ip_nonblock = old_ip_nonblock;                                               \
   sock_term ()

void
send_body (sock_t socket_handle, char *body)
{
    char
        buffer [BUFFER_SIZE + 1],
        *p_buffer,
        *end_line;

    p_buffer = body;
    do {
        end_line = strstr (p_buffer, "\r\n");
        if (end_line)
          {
            if ((end_line - p_buffer) > 512)
              {
                strncpy (buffer, p_buffer, 510);
                strcpy (&buffer [510], "\r\n");
                send_body_data (socket_handle, buffer);
                p_buffer += 510;
              }
            else
              {
                 *end_line = '\0';
                 send_body_data (socket_handle, p_buffer);
                 send_body_data (socket_handle, "\r\n");
                 p_buffer = end_line + 2;
              }
          }
        else
        if (         p_buffer
        &&  strused (p_buffer))
          {
            while (strlen (p_buffer) > 510)
              {
                strncpy (buffer, p_buffer, 510);
                strcpy (&buffer [510], "\r\n");
                send_body_data (socket_handle, buffer);
                p_buffer += 510;
              }
            if (         p_buffer
            &&  strused (p_buffer))
                send_body_data (socket_handle, p_buffer);
          }
      } while (end_line);
}
/* Stactic function prototypes                                               */
static int   uuencode            (char *strin, char *strout, char *last_smtp_message);
static void  putgroup            (register char *strgroup, register FILE *fp);
static int   getreply            (sock_t socket_handle, SMTP *smtp);
static char *getfilename         (char *strfullpath);
static void  pop3_get_stat       (POP3 **ctx);
static void  pop3_alloc_messages (POP3 **ctx);
static void  pop3_free_messages  (POP3 **ctx);
static void  pop3_parse_header   (POP_MSG *msg, char *header);
static int   read_line           (sock_t socket, char *buffer);
static Bool  check_socket_read_flag (sock_t handle);

/*  ---------------------------------------------------------------------[<]-
    Function: smtp_send_mail_ex

    Synopsis: Format and send a SMTP message.  This function gives you the
    options of sending to multi receivers, CC's, Bcc's and also send
    UUencoded attachments. Receivers and files are ";" or "," terminated.
    ---------------------------------------------------------------------[>]-*/

int smtp_send_mail_ex (
   SMTP *smtp)
{
   FILE
      *fpin;
   int
       iCnt;
   sock_t
       socket_handle;
   char
       message_boundary [256],
       strOut           [514],
       strFile          [256],
       strUUEFile       [256],
       buffer           [BUFFER_SIZE + 1],
      *charset,
      *nb_bit,
      *data,
      *p_buffer,
      *strRcptUserIds;
   Bool
       old_ip_nonblock = ip_nonblock;
   int
       rcptUserIdsLen;
  long
      current_date,
      current_time,
      out_size,
      in_size;
  char
     *quoted_subject = NULL,
     *in_buf,
     *out_buf;

   /* Check required parameters                                              */
   if (smtp == NULL)
       return (SMTP_ERROR_CONNECT);
   if (smtp->strDestUserIds == NULL)
       return (SMTP_ERROR_MISSING_DESTINATION);
   if (smtp->strSubject == NULL)
       return (SMTP_ERROR_MISSING_SUBJECT);
   if (smtp->strSmtpServer == NULL)
       return (SMTP_ERROR_MISSING_SERVER_NAME);

   /*  Make sure we block on socket accesses                                 */
   ip_nonblock = FALSE;
   sock_init ();

   /* Open up the SMTP port (25 most of the time). */

   if (smtp-> connect_retry_cnt < 1)
       smtp-> connect_retry_cnt = 3;

   nb_bit = "7";

   if (smtp-> strCharSet == NULL 
   || *smtp-> strCharSet == '\0')
       charset = "US-ASCII";
   else
     {
       charset = smtp-> strCharSet;
       nb_bit = "8";
       if (smtp-> strSubject)
         {
           if (lexcmp (charset, "iso-2022-jp") == 0
           ||  lexcmp (charset, "shift_jis")   == 0
           ||  lexcmp (charset, "utf-8")       == 0) {
               quoted_subject = encode_mimeb_string (NULL, 0,
                                                  (byte *) smtp->strSubject, charset);
           }
           else
               quoted_subject = encode_quoted_string (NULL, 0,
                                                  (byte *) smtp->strSubject, charset);
         }
     }
   socket_handle = connect_socket (smtp-> strSmtpServer,
                                   "smtp", "tcp", NULL,
                                   smtp-> connect_retry_cnt,
                                   smtp-> retry_wait_time);

   if (socket_handle == INVALID_SOCKET
   ||  getreply (socket_handle, smtp) > SMTP_SERVER_ERROR)
     {
       mem_strfree  (&quoted_subject);
       sock_term ();
       return (SMTP_ERROR_CONNECT);
     }

   /* Format a SMTP meassage header.                                         */
   /* Just say hello to the mail server.                                     */
   xstrcpy (strOut, "HELO ", get_hostname (), "\r\n", NULL);
   send_data (socket_handle, strOut);
   if (getreply (socket_handle, smtp) > SMTP_SERVER_ERROR)
     {
       CLEAN_SEND_MAIL;
       return (SMTP_ERROR_INIT);
     }
   /* Tell the mail server who the message is from. */
   xstrcpy (strOut, "MAIL FROM:<", smtp-> strSenderUserId, ">\r\n", NULL);
   send_data (socket_handle, strOut);
   if (getreply (socket_handle, smtp) > SMTP_SERVER_ERROR)
     {
       CLEAN_SEND_MAIL;
       return (SMTP_ERROR_INVALID_SENDER);
     }
   rcptUserIdsLen = 0;
   if (smtp-> strDestUserIds)
       rcptUserIdsLen += strlen (smtp->strDestUserIds) + 1;
   if (smtp-> strCcUserIds)
       rcptUserIdsLen += strlen (smtp->strCcUserIds)   + 1;
   if (smtp-> strBccUserIds)
       rcptUserIdsLen += strlen (smtp->strBccUserIds)  + 1;

   strRcptUserIds = (char *) mem_alloc (rcptUserIdsLen);
   p_buffer = strRcptUserIds;
   data = smtp-> strDestUserIds;
   while (*data)
       *p_buffer++ = *data++;
   if (smtp-> strCcUserIds)
     {
       *p_buffer++ = ';';
       data = smtp-> strCcUserIds;
       while (*data)
           *p_buffer++ = *data++;
     }
   if (smtp-> strBccUserIds)
     {
       *p_buffer++ = ';';
       data = smtp-> strBccUserIds;
       while (*data)
           *p_buffer++ = *data++;
     }
   *p_buffer = '\0';

   /* The following tells the mail server who to send it to.                 */
   iCnt = 0;
   if (*strRcptUserIds) {
       FOREVER
         {
            getstrfld (strRcptUserIds, iCnt++, 0, ",;", buffer);
            if (*buffer)
             {
               xstrcpy (strOut, "RCPT TO:<", buffer, ">\r\n", NULL);
               send_data (socket_handle, strOut);
               if (getreply (socket_handle, smtp) > SMTP_SERVER_ERROR)
                 {
                   CLEAN_SEND_MAIL;
                   return (SMTP_ERROR_INVALID_RECEIPT_USER);
                 }
             }

           else
               break;
         }
    }
    mem_free (strRcptUserIds);

   /* Now give it the Subject and the message to send.                       */
   send_data (socket_handle, "DATA\r\n");
   if (getreply (socket_handle, smtp) > SMTP_SERVER_ERROR)
     {
       CLEAN_SEND_MAIL;
       return (SMTP_ERROR_INVALID_DATA);
     }

   /* Set the date and time of the message.                                  */
   get_date_time_now (&current_date, &current_time);
   xstrcpy ( strOut, "Date: ", encode_mime_time (current_date, current_time),
             " \r\n", NULL );

   /* The following shows all who it was sent to. */
   if ( smtp-> strFullDestUserIds && *smtp-> strFullDestUserIds )
    {
       replacechrswith (smtp-> strFullDestUserIds, ";", ',');
       xstrcat (strOut, "To: ", smtp-> strFullDestUserIds, "\r\n", NULL);
     }
   else
    {
       replacechrswith (smtp-> strDestUserIds, ";", ',');
       xstrcat (strOut, "To: ", smtp-> strDestUserIds, "\r\n", NULL);
    }

   /* Set up the Reply-To path. */
   if (!smtp-> strRetPathUserId || !*smtp-> strRetPathUserId)
       smtp-> strRetPathUserId = smtp-> strSenderUserId;

   if ( strstr( smtp-> strRetPathUserId, "<" ) != NULL &&
        strstr( smtp-> strRetPathUserId, ">" ) != NULL )
       xstrcat (strOut, "Reply-To:",  smtp-> strRetPathUserId, "\r\n", NULL);
   else
       xstrcat (strOut, "Reply-To:<", smtp-> strRetPathUserId, ">\r\n", NULL);

   if ( smtp-> strFullSenderUserId && *smtp-> strFullSenderUserId )
     {
       xstrcat (strOut, "Sender: ", smtp-> strFullSenderUserId, "\r\n", NULL);
       xstrcat (strOut, "From: ",   smtp-> strFullSenderUserId, "\r\n", NULL);
     }
   else
     {
       xstrcat (strOut, "Sender: ", smtp-> strSenderUserId, "\r\n", NULL);
       xstrcat (strOut, "From: ",   smtp-> strSenderUserId, "\r\n", NULL);
     }
   send_data (socket_handle, strOut);

   *strOut = '\0';

   /* Post any CC's. */
   if (smtp->strFullCcUserIds && *smtp->strFullCcUserIds)
     {
       replacechrswith (smtp->strFullCcUserIds, ";", ',');
       xstrcat (strOut, "Cc:", smtp->strFullCcUserIds, "\r\n", NULL );
     }
   else
   if (smtp->strCcUserIds && *smtp->strCcUserIds)
     {
       replacechrswith (smtp->strCcUserIds, ";", ',');
       xstrcat (strOut, "Cc:", smtp->strCcUserIds, "\r\n", NULL );
     }

   /* Post any BCC's. */
   if (smtp->strFullBccUserIds && *smtp->strFullBccUserIds)
     {
       replacechrswith (smtp->strFullBccUserIds, ";", ',');
       xstrcat (strOut, "Bcc:", smtp->strFullBccUserIds, "\r\n", NULL);
     }
   else
   if (smtp->strBccUserIds && *smtp->strBccUserIds)
     {
       replacechrswith (smtp->strBccUserIds, ";", ',');
       xstrcat (strOut, "Bcc:", smtp->strBccUserIds, "\r\n", NULL);
     }
   /* Post any Return-Receipt-To. */
   if (smtp->strRrcpUserId && *smtp->strRrcpUserId)
       xstrcat (strOut, "Return-Receipt-To:", smtp->strRrcpUserId, ">\r\n",
                NULL);

   if (smtp->strMailerName && *smtp->strMailerName)
       xstrcat (strOut, "X-Mailer: ", smtp->strMailerName, "\r\n", NULL);
   else
       strcat  (strOut, "X-Mailer: sflmail function\r\n");

   /* Set the mime version. */
   get_date_time_now (&current_date, &current_time);
   sprintf (message_boundary, "%s.%ld.%ld", MESSAGE_BOUNDARY,
            current_date, current_time);

   if ( smtp->strHtmlMessageBody && *smtp->strHtmlMessageBody )
       xstrcat (strOut, "MIME-Version: 1.0\r\n",
                "Content-Type: multipart/alternative; boundary=\"", 
                message_boundary,"\"\r\n", NULL);
   else
       xstrcat (strOut, "MIME-Version: 1.0\r\n",
                "Content-Type: Multipart/Mixed; boundary=\"", 
                message_boundary,"\"\r\n", NULL);

   send_data (socket_handle, strOut);

   *strOut = '\0';
   /* Write out any message comment included. */
   if (smtp->strMsgComment && *smtp->strMsgComment)
       xstrcpy (strOut, "Comments: ", smtp->strMsgComment, "\r\n", NULL);

   /* Send the subject and message body. */
   if (quoted_subject)
       xstrcat (strOut, "Subject: ", quoted_subject, "\r\n\r\n", NULL);
   else
       xstrcat (strOut, "Subject: ", smtp->strSubject, "\r\n\r\n", NULL);
   send_data (socket_handle, strOut);

   /* Keep rfc822 in mind with all the sections.                             */
    if (smtp->strMessageBody && *smtp->strMessageBody)
      {
        /* check if we got html/alternate files                               */
        if ( smtp->strHtmlMessageBody && *smtp->strHtmlMessageBody )
          {
           xstrcpy (strOut,
                     "\r\n\r\n--", message_boundary, "\r\n",
                     "Content-Type: text/html; charset=", charset, "\r\n",
                     "Content-Transfer-Encoding: 7BIT\r\n",
                     "Content-description: Body of message\r\n\r\n", NULL);
           send_data (socket_handle, strOut);
           send_body (socket_handle, smtp->strHtmlMessageBody);
           send_data (socket_handle, "\r\n");
           xstrcpy (strOut,
                    "\r\n--", message_boundary, "\r\n",
                    "Content-Type: text/plain; charset=", charset, "\r\n",
                    "Content-Transfer-Encoding: ", nb_bit, "BIT\r\n",
                    "Content-description: Body of message\r\n\r\n", NULL);
           send_data (socket_handle, strOut);
           send_body (socket_handle, smtp-> strMessageBody);
           send_data (socket_handle, "\r\n");

         }
       else
         {
           xstrcpy (strOut,
                    "\r\n--", message_boundary, "\r\n",
                    "Content-Type: text/plain; charset=", charset, "\r\n",
                    "Content-Transfer-Encoding: ", nb_bit, "BIT\r\n",
                    "Content-description: Body of message\r\n\r\n", NULL);
           send_data (socket_handle, strOut);
           send_body (socket_handle, smtp-> strMessageBody);
           send_data (socket_handle, "\r\n");
         }
     }
   /* Include any Text type files and Attach them to the message. */
   if (smtp->strTxtFiles && *smtp->strTxtFiles)
     {
       iCnt = 0;
       FOREVER
         {
           getstrfld (smtp->strTxtFiles, iCnt++, 0, ",;", strFile);
           strcrop (strskp (strFile));
           if (*strFile)
             {
               fpin = fopen (strFile, "rb");
               if (!fpin)
                 {
                   strcpy (smtp->strlast_smtp_message, strFile);
                     {
                       CLEAN_SEND_MAIL;
                       return (SMTP_ERROR_MISSING_ATTACH_FILE);
                     }
                 }

               xstrcpy (strOut, "\r\n--", message_boundary, "\r\n",
                       "Content-Type: text/plain; charset=", charset, "\r\n",
                       "Content-Transfer-Encoding: ", nb_bit, "BIT\r\n",
                       "Content-Disposition: attachment; filename=",
                        getfilename (strFile), "\r\n\r\n", NULL);
               send_data (socket_handle, strOut);
               while (fgets (buffer, BUFFER_SIZE, fpin))
                 {
                   if (*buffer == '.')
                       write_TCP (socket_handle, ".", 1);
                   send_data (socket_handle, buffer);
                 }
               fclose (fpin);
             }
           else
               break;
         }
     }

   /* Attach any bin files to the message. */
   if (smtp->strBinFiles && *smtp->strBinFiles)
     {
       iCnt = 0;
       FOREVER
         {
           getstrfld (smtp->strBinFiles, iCnt++, 0, ",;", strFile);
           strcrop (strskp (strFile));
           if (*strFile)
             {
               if (smtp-> encode_type == ENCODE_BASE64)
                 {
                   fpin = fopen (strFile, "rb");
                   if (fpin)
                     {
                       /* Get file size                                      */
                       fseek (fpin, 0, SEEK_END);
                       in_size = ftell (fpin);
                       fseek (fpin, 0, SEEK_SET);
                       /* Alloc conversion buffer                            */
                       in_buf  = mem_alloc (in_size + 1);
                       out_buf = mem_alloc ((int)(in_size * 1.4) + 2);
                       if (in_buf && out_buf)
                         {
                           xstrcpy (strOut, "\r\n--", message_boundary, "\r\n",
                                    "Content-Type: application/octet-stream; name=",
                                     getfilename (strFile), "\r\n",
                                     "Content-Transfer-Encoding: base64\r\n",
                                     "Content-Disposition: attachment; filename=",
                                     getfilename (strFile), "\r\n\r\n", NULL);
                           send_data (socket_handle, strOut);
                           in_size = fread (in_buf, 1, in_size, fpin);
                           out_size = encode_base64 (in_buf, out_buf, in_size);

                           /* Format Encoded buffer into line                   */
                           p_buffer = out_buf;
                           while (out_size > 0)
                             {
                               if (out_size > LINE_SIZE)
                                 {
                                   write_TCP (socket_handle, p_buffer, LINE_SIZE);
                                   write_TCP (socket_handle, "\r\n",  2);
                                   out_size -= LINE_SIZE;
                                   p_buffer += LINE_SIZE;
                                 }
                               else
                                 {
                                   write_TCP (socket_handle, p_buffer, out_size);
                                   write_TCP (socket_handle, "\r\n",  2);
                                   out_size = 0;
                                 }
                             }
                         }
                       else
                         {
                           if (in_buf)  mem_free (in_buf);
                           if (out_buf) mem_free (out_buf);
                           CLEAN_SEND_MAIL;
                           return (SMTP_ERROR_MEMORY);
                         }
                       if (in_buf)  mem_free (in_buf);
                       if (out_buf) mem_free (out_buf);
                       fclose (fpin);
                     }
                   else
                     {
                       CLEAN_SEND_MAIL;
                       return (SMTP_ERROR_MISSING_ATTACH_FILE);
                     }
                 }
               else
                 {
                   strcpy (strUUEFile, strFile);
                   if (strchr (strUUEFile, '.'))
                       *((strchr (strUUEFile, '.'))) = 0;
                   strcat (strUUEFile, ".uue");
                   uuencode (strFile, strUUEFile, smtp->strlast_smtp_message);
                   fpin = fopen (strUUEFile, "rb");
                   if (!fpin)
                     {
                       CLEAN_SEND_MAIL;
                       return (SMTP_ERROR_MISSING_ATTACH_FILE);
                     }
                   xstrcpy (strOut, "\r\n--", message_boundary, "\r\n",
                            "Content-Type: application/octet-stream; name=",
                             getfilename (strFile), "\r\n",
                            "Content-Transfer-Encoding: x-uuencode\r\n",
                            "Content-Disposition: attachment; filename=",
                            getfilename (strFile), "\r\n\r\n", NULL);
                   send_data (socket_handle, strOut);
                   while (fgets (buffer, BUFFER_SIZE, fpin))
                     {
                       if (*buffer == '.')
                           write_TCP (socket_handle, ".", 1);
                       send_data (socket_handle, buffer);
                     }
                   fclose (fpin);

                   if (!smtp->debug)
                      unlink (strUUEFile);
                 }
             }
           else
               break;
         }
     }

    /* This ends the message. */
    xstrcpy (strOut, "\r\n--", message_boundary, "--\r\n\r\n.\r\n", NULL);
    send_data (socket_handle, strOut);
    if (getreply (socket_handle, smtp) > SMTP_SERVER_ERROR)
      {
        CLEAN_SEND_MAIL;
        return (SMTP_ERROR_ON_CLOSE);
      }

    /* Now log off the SMTP port. */
    send_data (socket_handle, "QUIT\r\n");
    if (getreply (socket_handle, smtp) > SMTP_SERVER_ERROR)
      {
        CLEAN_SEND_MAIL;
         return (SMTP_ERROR_ON_QUIT);
      }

    CLEAN_SEND_MAIL;
    return (0);
}

/*  ---------------------------------------------------------------------[<]-
    Function: smtp_send_mail

    Synopsis: Format and send a SMTP message, by calling the
    smtp_send_mail_ex function.  This function is kept to be compatable
    with previous versions of smtp_send_mail, smtp_send_mail_ex should
    now be used, this will be deleted soon.
    ---------------------------------------------------------------------[>]-*/

int smtp_send_mail (
   char *strSmtpServer,
   char *strMessageBody,
   char *strSubject,
   char *strSenderUserId,
   char *strFullSenderUserId,
   char *strDestUserIds,
   char *strFullDestUserIds,
   char *strCcUserIds,
   char *strFullCcUserIds,
   char *strBccUserIds,
   char *strFullBccUserIds,
   char *strRetPathUserId,
   char *strRrcpUserId,
   char *strMsgComment,
   char *strMailerName,
   char *strBinFiles,
   char *strTxtFiles,
   char *strDebugFile,
   char *strCharSet )
{
   SMTP smtp;

   memset (&smtp, 0, sizeof (SMTP));

   smtp.strSmtpServer       = strSmtpServer;
   smtp.strMessageBody      = strMessageBody;
   smtp.strSubject          = strSubject;
   smtp.strSenderUserId     = strSenderUserId;
   smtp.strFullSenderUserId = strFullSenderUserId;
   smtp.strDestUserIds      = strDestUserIds;
   smtp.strFullDestUserIds  = strFullDestUserIds;
   smtp.strCcUserIds        = strCcUserIds;
   smtp.strFullCcUserIds    = strFullCcUserIds;
   smtp.strBccUserIds       = strBccUserIds;
   smtp.strFullBccUserIds   = strFullBccUserIds;
   smtp.strRetPathUserId    = strRetPathUserId;
   smtp.strRrcpUserId       = strRrcpUserId;
   smtp.strMsgComment       = strMsgComment;
   smtp.strMailerName       = strMailerName;
   smtp.strBinFiles         = strBinFiles;
   smtp.strTxtFiles         = strTxtFiles;
   smtp.connect_retry_cnt   = 3;
   smtp.retry_wait_time     = 0;
   smtp.debug               = 0;
   smtp.strDebugFile        = strDebugFile;
   smtp.encode_type         = ENCODE_BASE64;
   smtp.strCharSet          = strCharSet;

   return smtp_send_mail_ex (&smtp);
}

/*
 *  uuencode -- internal
 *
 *  Synopsis: Uuencode a file, with the output going to a new file. This
 *  function is used by smtp_send_mail.
 * -------------------------------------------------------------------------*/

static int uuencode (
   char *strIn,
   char *strOut,
   char *strlast_smtp_message)
{
   char strLine[46];
   int iCnt, iLineLen;
   FILE *fpin, *fpout;

   if (!(fpin = fopen (strIn, "rb")))
     {
       strcpy (strlast_smtp_message, strIn);
       return 1;
     }

   if (!(fpout = fopen (strOut, "wb")))
     {
       fclose (fpin);
       strcpy (strlast_smtp_message, "Could not create temp file for write.");
       return 1;
     }

   fprintf (fpout, "begin 666 %s\n", getfilename (strIn));

   while (1)
     {
       iLineLen = fread (strLine, sizeof (char), 45, fpin);
       if (iLineLen <= 0)
           break;

       fputc (ENC (iLineLen), fpout);

       for (iCnt = 0; iCnt < iLineLen; iCnt += 3)
         {
           putgroup (&strLine[iCnt], fpout);
         }

       fputc ('\n', fpout);
     }

   fprintf (fpout, "end\n");

   fclose (fpin);
   fclose (fpout);
   return 0;
}

/*
 *  putgroup -- internal
 *
 *  Synopsis: Write out 3 char group to uuendcoded file making it
 *  printable  This function is used by uuencode.
 * -------------------------------------------------------------------------*/

static void putgroup (
   char *strgroup,
   FILE *fp)
{
    register byte ichr1, ichr2, ichr3, ichr4;

    ichr1 =   strgroup [0] >> 2;
    ichr2 = ((strgroup [0] << 4) & 0x30) | ((strgroup [1] >> 4) & 0x0f);
    ichr3 = ((strgroup [1] << 2) & 0x3c) | ((strgroup [2] >> 6) & 0x03);
    ichr4 =   strgroup [2] & 0x3f;

    fputc (ENC (ichr1), fp);
    fputc (ENC (ichr2), fp);
    fputc (ENC (ichr3), fp);
    fputc (ENC (ichr4), fp);
}

/*  Read until we get an end-of-line                                     */
static int
read_line (sock_t socket_handle, char *buffer)
{
    int
        read_size,
        bytes_read = 0;

    buffer [0] = '\0';
    while (strchr (buffer, '\n') == NULL)
      {
        read_size = read_TCP (socket_handle,
                              buffer + bytes_read,
                              BUFFER_SIZE - bytes_read);
        if (read_size <= 0)
            break;
        bytes_read += read_size;
        buffer [bytes_read] = '\0';
      }
    return (bytes_read);
}

/*
 *  getreply -- internal
 *
 *  Synopsis: Get a reply from the SMTP server and see thats it's not
 *  an error. This function is used by smtp_send_mail.
 * -------------------------------------------------------------------------*/

static int
getreply (sock_t socket_handle, SMTP *smtp)
{
    int
       read_size,
       bytes_read = 0;
    FILE
      *fpout;
    char
       buffer [BUFFER_SIZE + 1];

    /*  Read until we get an end-of-line                                     */
    buffer [0] = '\0';
    while (strchr (buffer, '\n') == NULL)
      {
        read_size = read_TCP (socket_handle, buffer + bytes_read, BUFFER_SIZE - bytes_read);
        /*  See if we have not gotten a responce back from the mail server   */
        if (read_size == 0)
            return 777;

        bytes_read += read_size;
        buffer [bytes_read] = '\0';
      }

    /* Save off server reply. */
    strcpy (smtp-> strlast_smtp_message, buffer);
    strcrop (strskp (buffer));

    if (email_debug)
        coprintf ("Email Receive: %s", buffer);

    buffer [3] = '\0';

    if (smtp->debug && smtp->strDebugFile && *smtp->strDebugFile)
      {
        if ((fpout = fopen (smtp->strDebugFile, "a")))
          {
            fputs (smtp->strlast_smtp_message, fpout );
            fclose (fpout);
          }
      }
   return atoi (buffer);
}

/*
 *  getfilename -- internal
 *
 *  Synopsis: Get's the name from the full path of a file. This function
 *  is used by smtp_send_mail.
 * -------------------------------------------------------------------------*/

static char *getfilename (
   char *strFullPath)
{
   int iLen;
   char *strTmp;

   iLen = strlen (strFullPath);
   strTmp = (strFullPath + iLen);
   while (1)
     {
       if (*strTmp == PATHEND || !iLen)
           break;
       strTmp--;
       iLen--;
     }

   if (*strTmp == PATHEND)
       strTmp++;

   return strTmp;
}


/*  ---------------------------------------------------------------------[<]-
    Function: smtp_error_description

    Synopsis: Get error description
    ---------------------------------------------------------------------[>]-*/

char *
smtp_error_description (int error_value)
{
    static char * smtp_error [] = {
        "No Error",
        "Error connecting, verify SMTP server/address",
        "Error on init, SMTP server refused connection",
        "Invalid Sender: value",
        "Invalid To: value",
        "Invalid data value",
        "Can't open specified attached file(s)",
        "Error during close of transaction",
        "Error during quit transaction",
        "Missing Destination: value",
        "Missing Subject: value",
        "Missing server name",
        "Insufficient memory"
    };

    if (error_value >= 0
    &&  error_value <  MAX_SMTP_ERROR)
        return (smtp_error [error_value]);
    else
        return ("");
}

#define POP3_CHECK_RETURN if (read_size == 0 || buffer [0] != '+') {        \
                          close_socket (handle); sock_term (); return (NULL); }

/*  ---------------------------------------------------------------------[<]-
    Function: pop3_begin

    Synopsis: Begin a pop3 transaction. message_limit is

    ---------------------------------------------------------------------[>]-*/

POP3 *
pop3_begin (char *server, char *user, char *passw,
            Bool secure, Bool leave_on_server,
            int message_max_size)
{
    POP3
        *pop3 = NULL;
    sock_t
        handle;
    char
        buffer [BUFFER_SIZE + 1];
    int
        read_size;

    ip_nonblock = FALSE;
    sock_init ();

    ASSERT (server);
    ASSERT (user);
    ASSERT (passw);
    ASSERT (message_max_size >= 0);

    handle = connect_TCP (server, "110");
    if (handle != INVALID_SOCKET)
      {
        read_size = read_line (handle, buffer);
        POP3_CHECK_RETURN;

        sprintf (buffer, "USER %s\r\n", user);
        send_data (handle, buffer);
        read_size = read_line (handle, buffer);
        POP3_CHECK_RETURN;

        sprintf (buffer, "PASS %s\r\n", passw);
        send_data (handle, buffer);
        read_size = read_line (handle, buffer);
        POP3_CHECK_RETURN;

        if (email_debug)
            coprintf ("Connected on server %s", server);
        pop3 = mem_alloc (sizeof (POP3));
        if (pop3)
          {
            memset (pop3, 0, sizeof (POP3));
            pop3-> handle           = handle;
            pop3-> leave_on_server  = leave_on_server;
            pop3-> message_max_size = message_max_size;
            pop3_get_stat (&pop3);
          }
      }

    return (pop3);
}


/*  ---------------------------------------------------------------------[<]-
    Function: pop3_list_message

    Synopsis: Get the number of message and header of each message.
    ---------------------------------------------------------------------[>]-*/

int
pop3_list_message (POP3 **ctx)
{
    int
        index,
        read_size,
        header_size = 0,
        header_read = 0,
        nb_messages = 0;
    char
        *header,
        *end,
        buffer [BUFFER_SIZE + 1];

    nb_messages = (*ctx)-> nb_messages;
    pop3_alloc_messages (ctx);

    for (index = 0; index < nb_messages; index++)
      {
        sprintf (buffer, "LIST %d\r\n", index + 1);
        send_data ((*ctx)-> handle, buffer);
        read_size = read_line ((*ctx)-> handle, buffer);

        if (buffer [0] == '+')
          {
            end = strchr (&buffer [4], ' ');
            if (end)
                (*ctx)-> messages [index]-> size = atoi (++end);
          }

        sprintf (buffer, "TOP %d 0\r\n", index + 1);
        send_data ((*ctx)-> handle, buffer);
        read_size = read_line ((*ctx)-> handle, buffer);

        if (buffer [0] == '+')
          {
            header_size = 0;
            end = strchr (&buffer [4], ' ');
            if (end)
                header_size = atoi (++end);
            if (header_size == 0 && (*ctx)-> messages [index]-> size > 0)
                header_size = (*ctx)-> messages [index]-> size;

            if (header_size > 0)
              {
                header = mem_alloc (header_size + 1);
                if (header != NULL)
                  {
                    end = strstr (buffer, "\r\n");
                    if (end)
                      {
                        end += 2;
                        strcpy (header, end);
                        header_read = strlen (header);
                      }
                    do
                      {
                        read_size = read_TCP ((*ctx)-> handle, &header [header_read],
                                      header_size - header_read);
                        if (read_size <= 0)
                            break;
                        header_read += read_size;
                        header [header_read] = '\0';
                      } while ( header_read < header_size
                        &&      header [header_read - read_size]
                        &&     strstr  (&header [header_read - read_size], "\r\n\r\n")     == NULL
                        &&     lexncmp (&header [header_read - read_size], ".\r\n",     3) != 0
                        &&     strstr  (&header [header_read - read_size], "\r\n.\r\n")    == NULL);
                    pop3_parse_header ((*ctx)-> messages [index], header);
                    mem_free (header);
                  }
              }
          }
      }

    return (nb_messages);
}

/*  ---------------------------------------------------------------------[<]-
    Function: pop3_get_message

    Synopsis: Get message. If message_nbr equal 0, get all messages else
    get only the message with this index.
    if a max size is specified in ctx (message_max_size), messages whose
    size is greater than this limit are discarded
    ---------------------------------------------------------------------[>]-*/

int
pop3_get_message (POP3 **ctx, int message_nbr)
{
    int
        index,
        read_size,
        message_size,
        buffer_size,
        message_read = 0,
        nb_messages = 0,
        nb_files;
    char
        *header,
        *body,
        *end,
        buffer [BUFFER_SIZE + 1];
    SYMTAB
        *form = NULL,
        *files = NULL;
    SYMBOL
        *symbol;

    if ((*ctx)-> nb_messages == 0)
      {
        if (message_nbr == 0)
            pop3_get_stat (ctx);
        else
            (*ctx)-> nb_messages = message_nbr;
      }

    pop3_alloc_messages (ctx);

    for (index = (message_nbr == 0)? 0: message_nbr - 1;
         index < (*ctx)-> nb_messages
         && (message_nbr == 0 || index == message_nbr - 1);
         index++)
      {
        /*  Get message size before retrieve, some POP3 server
            (like MS Exchange) don't give the message size after
            the '+OK' response :=(
         */
        message_size = 0;
        sprintf (buffer, "LIST %d\r\n", index + 1);

        if (email_debug)
            coprintf ("Send command: %s", buffer);
        send_data ((*ctx)-> handle, buffer);
        read_size = read_line ((*ctx)-> handle, buffer);
        /*  If server closed connection, don't continue                 */
        if (read_size <= 0)
          {
            (*ctx)-> nb_messages = index;
            break;
          }

        if (email_debug)
            coprintf ("Receive: %s", buffer);

        if (buffer [0] == '+')
          {
            end = strchr (&buffer [4], ' ');
            if (end)
                message_size = atoi (++end);
          }

        if (message_size > 0
        && (! (*ctx)-> message_max_size
        ||  (message_size > (*ctx)-> message_max_size * 1024)))
          {
             (*ctx)-> messages [index]-> size = message_size;

             buffer_size = message_size + 16380; /* Add 512 char for the first status line */
             header = mem_alloc (buffer_size + 1);
             if (header != NULL)
               {
                 memset (header, 0, buffer_size + 1);
                 sprintf (buffer, "RETR %d\r\n", index + 1);

                 if (email_debug)
                     coprintf ("Send command: %s", buffer);

                 send_data ((*ctx)-> handle, buffer);
                 read_size = read_TCP ((*ctx)-> handle, header, buffer_size);

                 if (read_size < 0)
                     read_size = 0;
                 header [read_size] = '\0';

                 if (header [0] == '+')
                   {
                     /* Cut the first line with POP3 return value            */
                    end = strstr (header, "\r\n");
                    if (end)
                      {
                        end += 2;
                        memmove (header, end, read_size - (end - header) + 1);
                        message_read = strlen (header);
                      }
                    while (strstr (header, "\r\n.\r\n") == NULL)
                      {
                        read_size = read_TCP ((*ctx)-> handle, &header [message_read],
                                          buffer_size - message_read);
                        if (read_size <= 0)
                            break;
                        message_read += read_size;
                        header [message_read] = '\0';
                      }

                    /*  Get . by itself on a line                            */
                    if (check_socket_read_flag ((*ctx)-> handle) == TRUE)
                        read_line ((*ctx)-> handle, buffer);

                    body = strstr (header, "\r\n\r\n");
                    if (body)
                      {
                        *body = '\0';
                        body += 4;
                      }

                    if (email_debug)
                      {
                        coprintf ("Message size: %ld\nRead size = %ld",
                                  message_size, message_read);
                        coprintf ("Header: %s", header);
                      }
                    pop3_parse_header ((*ctx)-> messages [index], header);

                    if (body)
                      {
                        if (email_debug)
                            coprintf ("Body: \n%s", body);
                        nb_messages++;
                        if (!(*ctx)-> messages [index]-> multipart)
                            (*ctx)-> messages [index]-> body = mem_strdup (body);
                        else
                          {
                            if (email_debug)
                                coprintf ("Decode multipart body");
                            body = strstr (body, "--");
                            if (body)
                              {
                                nb_files = http_multipart_mem ((byte*) body, strlen (body), &form, &files);
                                symbol = sym_lookup_symbol (form, "bodymessage");
                                if (symbol)
                                    (*ctx)-> messages [index]-> body = mem_strdup (symbol-> value);
                                else
                                if (email_debug)
                                    coprintf ("Error, no body find after decode multipart");

                                if (form)
                                  {
                                    sym_delete_table (form);
                                    form = NULL;
                                  }
                                if (files)
                                  {
                                    sym_delete_table (files);
                                    files = NULL;
                                  }
                              }
                            else
                            if (email_debug)
                                coprintf ("Error in multipart, don't find '--' string");
                          }
                      }
                    else
                    if (email_debug)
                        coprintf ("Error: no body find");
                  }

              mem_free (header);

              if ((*ctx)-> leave_on_server == FALSE)
                  pop3_delete_message (ctx, index + 1);
              }
            /* else, message size exeeds allowed limit. Message is discarded
             * and not removed from the server                               */
          }
      }

    return (nb_messages);
}

/*  ---------------------------------------------------------------------[<]-
    Function: pop3_end

    Synopsis: End Pop3 session.
    ---------------------------------------------------------------------[>]-*/

Bool
pop3_end (POP3 **context)
{
    Bool
        feedback = FALSE;
    char
        buffer [BUFFER_SIZE + 1];

    ASSERT (context);

    if ((*context)-> handle)
      {
        strcpy (buffer, "QUIT\r\n");
        send_data ((*context)-> handle, buffer);
        read_line ((*context)-> handle, buffer);

        close_socket ((*context)-> handle);
        (*context)-> handle = 0;

        sock_term ();
      }

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: pop3_free

    Synopsis: Free all resource allocated by this POP3 context. End session
    If needed.
    ---------------------------------------------------------------------[>]-*/

Bool
pop3_free (POP3 **context)
{
    Bool
        feedback = FALSE;

    pop3_end (context);
    pop3_free_messages (context);

    mem_free (*context);
    *context = NULL;

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: pop3_get_stat

    Synopsis: Get number of messages and total size.
    ---------------------------------------------------------------------[>]-*/

static void
pop3_get_stat (POP3 **ctx)
{
    int
        read_size,
        nb_messages = 0;
    long
        size = 0;
    char
        *separator,
        buffer [BUFFER_SIZE + 1];

    sprintf (buffer, "STAT\r\n");
    send_data ((*ctx)-> handle, buffer);
    read_size = read_line ((*ctx)-> handle, buffer);
    if (read_size == 0 || buffer [0] != '+')
        return;

    if (email_debug)
        coprintf ("Receive: %s", buffer);

    separator = strchr (buffer, ' ');
    if (*separator)
      {
        separator++;
        nb_messages = atoi (separator);

        separator = strchr (separator, ' ');
        if (*separator)
          {
            separator++;
            size = atol (separator);
          }
      }
    (*ctx)-> nb_messages = nb_messages;
    (*ctx)-> total_size  = size;
}


/*  ---------------------------------------------------------------------[<]-
    Function: pop3_free_messages

    Synopsis: Free all allocated messages structure.
    ---------------------------------------------------------------------[>]-*/

static void
pop3_free_messages (POP3 **ctx)
{
    POP_MSG
        *msg;
    int
        index;
    ASSERT (ctx);

    if ((*ctx)-> messages)
      {
        index = 0;
        msg = (*ctx)-> messages [index++];
        while (msg)
          {
            mem_strfree (&msg-> from);
            mem_strfree (&msg-> subject);
            mem_strfree (&msg-> body);
            mem_free (msg);
            msg = (*ctx)-> messages [index++];
          }
        mem_free ((*ctx)-> messages);
        (*ctx)-> messages = NULL;
      }

}

/*  ---------------------------------------------------------------------[<]-
    Function: pop3_delete_message

    Synopsis: Delete the message from pop3 server
    ---------------------------------------------------------------------[>]-*/

int   
pop3_delete_message (POP3 **ctx, int message_nb)
{
    int
        feedback = FALSE;
    char
        buffer [255];

    if (email_debug)
        coprintf ("Delete message %d", message_nb);

    sprintf (buffer, "DELE %d\r\n", message_nb);
    send_data ((*ctx)-> handle, buffer);
    read_line ((*ctx)-> handle, buffer);
    if (*buffer == '+')
        feedback = TRUE;

    if (email_debug)
        coprintf ("    feedback = %d (%s)", feedback, buffer);

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: set_email_debug

    Synopsis: Set email debug mode on/off.
    ---------------------------------------------------------------------[>]-*/

void  
set_email_debug (Bool mode)
{
    email_debug = mode;
}

/*  -------------------------------------------------------------------------
    Function: pop3_alloc_messages

    Synopsis: Alloc table of messages struct. Use nb_messages in context
    for number of message.
    -------------------------------------------------------------------------*/

static void
pop3_alloc_messages (POP3 **ctx)
{
    POP_MSG
        *msg;
    int
        index;

    pop3_free_messages (ctx);

    if ((*ctx)-> nb_messages > 0)
      {
        (*ctx)-> messages = mem_alloc (sizeof (POP_MSG *) * ((*ctx)-> nb_messages + 1));
        for (index = 0; index < (*ctx)-> nb_messages; index++)
          {
            msg = mem_alloc (sizeof (POP_MSG));
            if (msg)
              {
                memset (msg, 0, sizeof (POP_MSG));
                (*ctx)-> messages [index] = msg;
              }
          }
        (*ctx)-> messages [index] = NULL;
      }
}

/*  -------------------------------------------------------------------------
    Function: pop3_parse_header

    Synopsis: Alloc table of messages struct. Use nb_messages in context
    for number of message.
    -------------------------------------------------------------------------*/

void   multipart_decode_header (char *header, SYMTAB *table);

static void
pop3_parse_header (POP_MSG *msg, char *header)
{
    char
        *separator,
        *line;

    mem_strfree (&msg-> subject);
    mem_strfree (&msg-> from);
    msg-> date = 0;
    msg-> time = 0;

    line = strtok (header, "\r\n");
    while (line)
      {
        separator = strchr (line, ':');
        if (separator)
          {
            *separator++ = '\0';

            /* Remove white space                                            */
            while (*separator == ' ') separator++;

            if (lexcmp (line, "Subject") == 0)
                msg-> subject = mem_strdup (separator);
            else
            if (lexcmp (line, "Date") == 0)
                decode_mime_time (separator, &msg-> date, &msg-> time);
            else
            if (lexcmp (line, "From") == 0)
              {
                mem_strfree (&msg-> from);
                msg-> from = mem_strdup (separator);
              }
            else
            if (lexcmp (line, "Sender") == 0)
              {
                mem_strfree (&msg-> from);
                msg-> from = mem_strdup (separator);
              }
            else
            if (lexcmp (line, "Content-Type") == 0)
              {
                separator = strlwc (separator);
                msg-> multipart =  strstr (separator, "multipart") != NULL
                                   ? TRUE
                                   : FALSE;
              }
          }
        line = strtok (NULL, "\r\n");
      }
    if (email_debug)
      {
        if (msg-> subject)
            coprintf ("Subject: %s", msg-> subject);
        if (msg-> from)
            coprintf ("From:    %s", msg-> from);
        coprintf (    "Date:    %ld %ld", msg-> date, msg-> time);
      }
}


static Bool
check_socket_read_flag (sock_t handle)
{
    fd_set
        read_set;                       /*  Sockets to check for input       */
    struct timeval
        timeout;                        /*  Timeout for select()             */
    int
        rc;


    if (handle == 0)
        return (FALSE);

    memset (&read_set,  0, sizeof (fd_set));
    memset (&timeout,   0, sizeof (timeout));

    FD_SET (handle, &read_set);

    rc = sock_select (handle + 1,&read_set, NULL, NULL, &timeout);

    if (rc == SOCKET_ERROR)             /*  Error from socket call           */
        return (FALSE);      

    if (rc == 0)                        /*  No input or output activity      */
        return (FALSE);

    if (FD_ISSET (handle, &read_set))
        return (TRUE);

    return (FALSE);
}
