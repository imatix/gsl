/*===========================================================================*
 *                                                                           *
 *  sflsyst.c - System type identification                                   *
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
#include "sflmem.h"                     /*  Memory-access functions          */
#include "sflstr.h"                     /*  String manipulation functions    */
#include "sflprint.h"                   /*  snprintf functions               */
#include "sflsyst.h"                    /*  Prototypes for functions         */


/*  ---------------------------------------------------------------------[<]-
    Function: sys_assert

    Synopsis: Displays an 'assertion failed' message and aborts the program.
    This function is required by prelude.h if you compile with the DEBUG
    symbol.
    ---------------------------------------------------------------------[>]-*/

void
sys_assert (const char *File, unsigned Line)
{
#if (defined (__WINDOWS__))
    char
        line_str [10],
        *buffer;                        /*  Formatted error message          */
    MSG
        msg;
    Bool
        quit;
    int
        rc;                             /*  MessageBox return code           */

    snprintf (line_str, sizeof (line_str), "%u", Line);
    buffer = xstrcpy (NULL, "Module ", File, ", line ", line_str, NULL);

    /*  If WM_QUIT is in the queue the message box won't show                */
    quit = PeekMessage (&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
    rc   = MessageBox  (NULL, buffer, "Assertion failed!",
                        MB_TASKMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);
    mem_free (buffer);
    if (quit)
        PostQuitMessage (msg.wParam);
    if (rc != IDABORT)
        return;
#else
    fflush  (stdout);
    fprintf (stderr, "\nAssertion failed: %s, line %u\n", File, Line);
    fflush  (stderr);
#endif
    abort   ();
}


/*  ---------------------------------------------------------------------[<]-
    Function: sys_name

    Synopsis: Returns a static buffer with the type or name of OS.  If the
    full argument is true, tries to report the OS version information as
    well.
    ---------------------------------------------------------------------[>]-*/

char *
sys_name (Bool full)
{
#if (defined (__WINDOWS__))
#   if (defined (WIN32))
    static char
        name [30];

    OSVERSIONINFO
        version_info;

    if (full)
      {
        version_info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (GetVersionEx (&version_info))
          {
            if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT)
                snprintf (name, sizeof (name), "Windows NT %ld.%ld",
                                              version_info.dwMajorVersion,
                                              version_info.dwMinorVersion);
            else
            if (version_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                snprintf (name, sizeof (name), "Windows 95 %ld.%ld",
                                               version_info.dwMajorVersion,
                                               version_info.dwMinorVersion);
            else
                snprintf (name, sizeof (name), "Windows %ld.%ld with win32s",
                                               version_info.dwMajorVersion,
                                               version_info.dwMinorVersion);
          }
      }
    else
        strcpy (name, "Windows");
        
    return (name);
#   else
        return (full? "Windows 3.x": "Windows");
#   endif
#elif (defined (__UNIX__))
#   if (defined (__UTYPE_AUX))
        return ("UNIX Type: Apple AUX");
#   elif (defined (__UTYPE_DECALPHA))
        return ("UNIX Type: Digital UNIX (Alpha)");
#   elif (defined (__UTYPE_IBMAIX))
        return ("UNIX Type: IBM RS/6000 AIX");
#   elif (defined (__UTYPE_HPUX))
        return ("UNIX Type: HP/UX");
#   elif (defined (__UTYPE_LINUX))
        return ("UNIX Type: Linux");
#   elif (defined (__UTYPE_MIPS))
        return ("UNIX Type: MIPS");
#   elif (defined (__UTYPE_NETBSD))
        return ("UNIX Type: NetBSD");
#   elif (defined (__UTYPE_NEXT))
        return ("UNIX Type: NeXT");
#   elif (defined (__UTYPE_SCO))
        return ("UNIX Type: SCO UNIX");
#   elif (defined (__UTYPE_IRIX))
        return ("UNIX Type: Silicon Graphics IRIX");
#   elif (defined (__UTYPE_SUNOS))
        return ("UNIX Type: SunOS");
#   elif (defined (__UTYPE_SUNSOLARIS))
        return ("UNIX Type: Sun Solaris");
#   elif (defined (__UTYPE_UNIXWARE))
        return ("UNIX Type: SCO UNIXWare");
#   else
        return ("UNIX Type: Generic");
#   endif
#elif (defined (__VMS__))
    return ("UNIX Type: Digital OpenVMS");
#elif (defined (__OS2__))
    return ("UNIX Type: IBM OS/2");
#elif (defined (__MSDOS__))
    return ("MS-DOS");
#else
    return ("Unknown");
#endif
}

