/*===========================================================================*
 *  sysman.c - system manager service                                        *
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
 *===========================================================================*/

#include "sfl.h"                        /*  SFL library header file          */
#include "smt3.h"                       /*  SMT kernel header file           */
#include "sysman.h"                     /*  Sysman definitions               */
#include "version.h"                    /*  Version definitions              */

#define USAGE \
    "Syntax: sysman [options...]\n"                                         \
    "Options:\n"                                                            \
    "  -w directory     Working directory for Sysman\n"                     \
    "  -p port          Listen for connections on this port\n"              \
    "  -q               Quiet mode: no messages\n"                          \
    "  -s               Server mode: run as background job\n"               \
    "  -S               Console mode: run as foreground job\n"              \
    "  -t               Trace all socket i/o operations to log file\n"      \
    "  -v               Show Sysman version information\n"                  \
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
        quiet_mode = FALSE,             /*  -q means suppress messages       */
        background = FALSE;             /*  -s means run in background       */
    char
        *workdir,                       /*  Working directory                */
        *port,                          /*  Value for listen port            */
        **argparm;                      /*  Argument parameter to pick-up    */

    /*  First off, switch to user's id                                       */
    set_uid_user ();

    /*  These are the arguments we may get on the command line               */
    workdir    = NULL;
    port       = NULL;

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
                case 'w':
                    argparm = &workdir;  break;
                case 'p':
                    argparm = &port; break;

                /*  These switches have an immediate effect                  */
                case 'q':
                    quiet_mode = TRUE;
                    break;
                case 's':
                    background = TRUE;
                    break;
                case 'S':
                    background = FALSE;
                    break;
                case 't':
                    smtsock_trace (TRUE);
                    break;
                case 'v':
                    coprintf ("Sysman %s", SYSMAN_VERSION);
                    coprintf (PRODUCT);
                    coprintf (BUILDMODEL);
                    coprintf (COPYRIGHT);
                    coprintf ("Built on: %s", BUILDDATE);
                    exit (EXIT_SUCCESS);
                case 'h':
                    coprintf ("Sysman %s", SYSMAN_VERSION);
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
            args_ok = FALSE;
            break;
          }
      }

    /*  If there was a missing parameter or an argument error, quit          */
    if (argparm)
      {
        puts ("Argument missing - type 'sysman -h' for help");
        exit (EXIT_FAILURE);
      }
    else
    if (!args_ok)
      {
        puts ("Invalid arguments - type 'sysman -h' for help");
        exit (EXIT_FAILURE);
      }
      
    /*  Set server working directory if necessary                            */
    if (workdir
    &&  set_curdir (workdir))
      {
        printf ("Can't work in '%s' - %s\n", workdir, strerror (errno));
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
        puts ("Sysman " SYSMAN_VERSION);
        puts (COPYRIGHT);
      }

    if (background)
      {
        const char
           *background_args [] = { "-s", NULL };

        puts ("Moving into the background");
        if (process_server (NULL, NULL, argc, argv, background_args) != 0)
          {
            puts ("Backgrounding failed.  Giving up.");
            exit (EXIT_FAILURE);
          }
      }

    smt_init ();                        /*  Initialise SMT kernel            */
    if (sysmana_init (port) == 0)       /*  Initialise SYSMAN agent          */
        smt_exec_full ();               /*  Run until completed              */
    else
        printf ("Initialisation error\n");
    smt_term ();                        /*  Shut-down SMT kernel             */

    mem_assert ();

    return (EXIT_SUCCESS);
}
