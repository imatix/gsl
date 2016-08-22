/*===========================================================================*
 *  syscli.c - system manager client                                         *
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
 *===========================================================================*/

#include "sfl.h"                        /*  SFL library header file          */
#include "smt3.h"                       /*  SMT kernel header file           */
#include "sysman.h"                     /*  sysman definitions               */
#include "version.h"                    /*  Version definitions              */

#define USAGE \
    "Syntax: syscli [options...] [command]\n"                            \
    "Options:\n"                                                            \
    "  -p port          Connect to Sysman on this port\n"                   \
    "  -q               Quiet mode: no messages\n"                          \
    "  -t               Trace all socket i/o operations to log file\n"      \
    "  -v               Show Syscli version information\n"                  \
    "  -h               Show summary of command-line options\n"             \
    "\nThe order of arguments is not important. Switches and filenames\n" \
    "are case sensitive. See documentation for detailed information.\n"

int
main (int argc, char *argv [])
{
    int
        argn;                           /*  Argument number                  */
    Bool
        args_ok = TRUE,                 /*  Were the arguments okay?         */
        quiet_mode = FALSE;             /*  -q means suppress messages       */
    char
        *port,                          /*  Port to connect to               */
        **argparm;                      /*  Argument parameter to pick-up    */
    static char
        command [LINE_MAX];             /*  Command to pass to agent         */

    /*  These are the arguments we may get on the command line               */
    port       = NULL;
    strclr (command);

    argparm = NULL;                     /*  Argument parameter to pick-up    */
    for (argn = 1; argn < argc; argn++)
      {
        /*  If argparm is set, we have to collect an argument parameter      */
        if (argparm)
          {
            if (*argv [argn] != '-')    /*  Parameter can't start with '-'   */
              {
                *argparm = strdupl (argv [argn]);
                argparm = NULL;
              }
            else
              {
                args_ok = FALSE;
                break;
              }
          }
        else
        if (*argv [argn] == '-')
          {
            switch (argv [argn][1])
              {
                /*  These switches take a parameter                          */
                case 'p':
                    argparm = &port; break;

                /*  These switches have an immediate effect                  */
                case 'q':
                    quiet_mode = TRUE;
                    break;
                case 't':
                    smtsock_trace (TRUE);
                    break;
                case 'v':
                    coprintf ("Syscli %s", SYSCLI_VERSION);
                    coprintf (PRODUCT);
                    coprintf (BUILDMODEL);
                    coprintf (COPYRIGHT);
                    coprintf ("Built on: %s", BUILDDATE);
                    exit (EXIT_SUCCESS);
                case 'h':
                    coprintf ("Syscli %s", SYSCLI_VERSION);
                    coprintf (COPYRIGHT);
                    coprintf (USAGE);
                    exit (EXIT_SUCCESS);

                /*  Anything else is an error                                */
                default:
                    args_ok = FALSE;
              }
          }
        else
          {
            strcat (command, " ");
            strcat (command, argv [argn]);
          }
      }

    /*  If there was a missing parameter or an argument error, quit          */
    if (argparm)
      {
        puts ("Argument missing - type 'syscli -h' for help");
        exit (EXIT_FAILURE);
      }
    else
    if (!args_ok)
      {
        puts ("Invalid arguments - type 'syscli -h' for help");
        exit (EXIT_FAILURE);
      }
    
    /*  Handle the remaining arguments we got                                */
    if (!port)
        port = SYSMAN_DEFAULT_PORT;
    if (quiet_mode)
      {
        fclose (stdout);                /*  Kill standard output             */
        fclose (stderr);                /*   and standard error              */
      }
    else
      {
        puts ("Syscli " SYSCLI_VERSION);
        puts (COPYRIGHT);
      }

    smt_init ();                        /*  Initialise SMT kernel            */
    sysclia_init (command, port);       /*  Initialise SYSMAN client agent   */
    smt_exec_full ();                   /*  Run until completed              */
    smt_term ();                        /*  Shut-down SMT kernel             */
    mem_assert ();                      /*  Assert memory is clean           */
    return (0);
}
