/*===========================================================================*
 *                                                                           *
 *  smtpipe.c - Transfer pipe agent                                          *
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


/*- Definitions -------------------------------------------------------------*/

#define AGENT_NAME      "SMTPIPE"       /*  Our public name                  */
#define SINGLE_THREADED TRUE            /*  Single-threaded agent            */


/*- Global variables used in this source file only --------------------------*/

static QID
    tranq,                              /*  Transfer agent event queue       */
    operq;                              /*  Operator console event queue     */
static XML_ITEM
    *pipes;                             /*  List of pipe definitions         */
static char
    *filename;                          /*  Pipe definition file, argument   */


#include "smtpipe.d"                    /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

/*  ---------------------------------------------------------------------[<]-
    Function: smtpipe_init

    Synopsis: Initialises the SMT pipe agent.  Returns 0 if initialised
    okay, -1 if there was an error.  
    ---------------------------------------------------------------------[>]-*/

int
smtpipe_init (char *p_filename)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle for initial thread        */

#   include "smtpipe.i"                 /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Reply events from timer agent                                        */
    declare_smttime_reply     (timer_event,    SMT_PRIORITY_LOW);

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        operq = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that transfer agent is running, else start it up              */
    smttran_init ();
    if ((thread = thread_lookup (SMT_TRANSFER, "")) != NULL)
        tranq = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that timer agent is running, else start it up                 */
    if (smttime_init ())
        return (-1);

    /*  Create initial, unnamed thread                                       */
    thread = thread_create (AGENT_NAME, "");
    filename = p_filename;              /*  Get name of pipe definition      */
    
    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    the_next_event = ok_event;
}


/*************************   LOAD PIPE DEFINITIONS   *************************/

MODULE load_pipe_definitions (THREAD *thread)
{
    if (file_where ('r', "PATH", filename, NULL))
      {
        pipes = NULL;
        if (xml_load_file (&pipes, "PATH", filename, FALSE) != XML_NOERROR)
          {
            send_smtoper_error (&operq,
                                strprintf ("smtpipe: error in '%s': %s",
                                           filename,
                                           xml_error ()));
            raise_exception (exception_event);
          }
      }
    else
      {
        send_smtoper_error (&operq,
                            strprintf ("smtpipe: '%s' not found", 
                                       filename));
        send_smtoper_error (&operq,
                            strerror (errno));
        raise_exception (exception_event);
      }
}


/*************************   CREATE TRANSFER PIPES   *************************/

MODULE create_transfer_pipes (THREAD *thread)
{
    XML_ITEM
        *pipe,                          /*  XML pipe item                    */
        *instance;                      /*  XML instance item                */
    XML_ATTR
        *attr;                          /*  XML attribute                    */
    char
        *pipe_name;
    qbyte
        inrate,                         /*  Pipe input rate                  */
        outrate,                        /*  Pipe output rate                 */
        units;                          /*  Transfer rate multiplier         */

    FORCHILDREN (pipe, xml_first_child (pipes))
      {
        pipe_name = xml_get_attr (pipe, "NAME", NULL);
        if (!pipe_name)
          {
            send_smtoper_error (&operq,
                                strprintf ("smtpipe: syntax error in '%s' - no NAME", 
                                           filename));
            continue;
          }

        inrate  = atol (xml_get_attr (pipe, "INRATE",  "0"));
        outrate = atol (xml_get_attr (pipe, "OUTRATE", "0"));
        if (inrate  == 0)
            inrate  = atol (xml_get_attr (pipe, "RATE", "0"));
        if (outrate == 0)
            outrate = atol (xml_get_attr (pipe, "RATE", "0"));

        if (inrate == 0 || outrate == 0)
          {
            send_smtoper_error (&operq,
                                strprintf ("smtpipe: pipe '%s' badly defined",
                                           pipe_name));
            continue;
          }
        units    = atol (xml_get_attr (pipe, "UNITS", "1"));
        inrate  *= units;
        outrate *= units;

        /*  Create each pipe instance that is defined                        */
        FORCHILDREN (instance, pipe)
          {
            attr = xml_attr (instance, "NAME");
            if (attr == NULL)
              {
                send_smtoper_error (&operq,
                                    strprintf ("smtpipe: pipe '%s' instance has no name", 
                                               pipe_name));
                continue;
              }
            send_smttran_pipe_create (&tranq,
                                      xml_attr_value (attr),
                                      inrate, 
                                      outrate);
          }
      }
}


/***************************   SLEEP FOR A WHILE   ***************************/

MODULE sleep_for_a_while (THREAD *thread)
{
    /*  Ask timer to send us an event after 10 seconds                       */
    smttime_request_alarm (0, 1000, 0);
}


/************************   RELOAD PIPES IF CHANGED   ************************/

MODULE reload_pipes_if_changed (THREAD *thread)
{
    if (xml_refresh (&pipes))
      {
        send_smttran_clear_pipes (&tranq);
        create_transfer_pipes (thread);
      }
}


/*************************   DROP PIPE DEFINITIONS   *************************/

MODULE drop_pipe_definitions (THREAD *thread)
{
    if (pipes)
        xml_free (pipes);
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    the_next_event = terminate_event;
}

