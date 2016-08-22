@echo off
:-
:-  c.bat - Compile ANSI C program (MSVC multiplatform)
:-
:-  Copyright (c) 1996-2010 iMatix Corporation
:-
:-  This program is free software; you can redistribute it and/or modify
:-  it under the terms of the GNU General Public License as published by
:-  the Free Software Foundation; either version 3 of the License, or (at
:-  your option) any later version.
:-
:-  This program is distributed in the hope that it will be useful, but
:-  WITHOUT ANY WARRANTY; without even the implied warranty of
:-  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
:-  General Public License for more details.
:-
:-  For information on alternative licensing for OEMs, please contact
:-  iMatix Corporation.
:-

:-  Start CMD.EXE version detection
verify other 2>nul
setlocal enableextensions
if errorlevel 0 goto __cmd_ok
echo %0: This command requires command extensions version 2 in CMD.EXE.  
echo %0: Please use a supported system (Windows 2000 or newer).
exit
:__cmd_ok
setlocal enabledelayedexpansion
:-  End CMD.EXE version detection

goto init
:help
echo.
echo  C script - iMatix C compile script for Win32
echo.
echo  Syntax of script:
echo  c filename...     Compile ANSI C program(s)
echo  c -c filename...  (Backwards compatible: compile C programs)
echo  c -l main...      Compile and link main program(s)
echo  c -L main...      Link main program(s), no compile
echo  c -C              Report C compiler command syntax
echo  c -r lib file...  Replace object file(s) into library
echo    -li path        Local include path, additional to INCDIR
echo    -ll path        Local library path, additional to LIBDIR
echo    -g              Compile and link with debug information
echo    -p              Use C++ compiler instead of C
echo                    When linking, link with C++ runtime
echo                    When replacing, replace .opp file
echo    -v              (First arg prefix to above): be verbose
echo    -q              (First arg prefix to above): be quiet
echo    -xxxx           Arbitrary switch passed to compiler/linker
echo.
echo  The current version of the script supports MSVC only.  Support for
echo  other compilers will be added back in as this script stabilises.
echo.
echo  You can optionally set these environment variables:
echo.
echo  CCDEFINES - options required for ANSI C compilation
echo  CCNAME    - compiler name, default is msvc
echo  INCDIR    - directory containing user include files, if defined
echo  LIBDIR    - directory containing user libraries, if defined
echo.
echo  When using iMatix boom, if the IBASE environment variable is set 
echo  and conflicts with INCDIR or LIBDIR, c will issue a warning.
goto err_exit

:-
:-  Subroutine: boom_model_init
:-  Initialises BOOM_MODEL to default if not set
:-  Expands BOOM_MODEL components into individual environment variables
:-
:boom_model_init
if "!BOOM_MODEL!"=="" (
    set BOOM_MODEL=release,st
)
set BOOM_MODEL__PLACEHOLDER=1
for /f "usebackq delims==" %%v in (`set BOOM_MODEL_`) do set %%v=
set BOOM_MODEL__PLACEHOLDER=
for %%m in (!BOOM_MODEL!) do set BOOM_MODEL_%%m=1
:-  If the user set BOOM_MODEL but did not include release/debug or st/mt
:-  set these to defaults.
if not "!BOOM_MODEL_RELEASE!"=="1" (
    if not "!BOOM_MODEL_DEBUG!"=="1" (
        set BOOM_MODEL_RELEASE=1
        set BOOM_MODEL=!BOOM_MODEL!,release
    )
)
if not "!BOOM_MODEL_ST!"=="1" (
    if not "!BOOM_MODEL_MT!"=="1" (
        set BOOM_MODEL_ST=1
        set BOOM_MODEL=!BOOM_MODEL!,st
    )
)
if "!BOOM_MODEL_RELEASE!"=="1" set _DEBUG=0
if "!BOOM_MODEL_DEBUG!"=="1"   set _DEBUG=1
if "!BOOM_MODEL_ST!"=="1"      set _MT=0
if "!BOOM_MODEL_MT!"=="1"      set _MT=1
goto :eof

:-
:-  Clean-up symbols and directory
:-
:init
    setlocal
    if exist *.map del *.map
    call :boom_model_init

:-
:-  Parse command line
:-
    set _QUIET=0
    set _VERBOSE=0
    set _USECPP=0
    set _COMPILE=1
    set _REPLACE=0
    set _LINK=0
    set _SYNTAX=0
    if "%1"==""   goto help

:do_switch
    set switch=%1

    if "%1"=="-q"  set _QUIET=1
    if "%1"=="-q"  goto shift_switch
    if "%1"=="-v"  set _VERBOSE=1
    if "%1"=="-v"  goto shift_switch
    if "%1"=="-g"  set _DEBUG=1
    if "%1"=="-g"  goto shift_switch
    if "%1"=="-p"  set _USECPP=1
    if "%1"=="-p"  goto shift_switch
    if "%1"=="-C"  set _SYNTAX=1
    if "%1"=="-C"  goto shift_switch
    if "%1"=="-c"  goto shift_switch
    if "%1"=="-r"  goto preplace
    if "%1"=="-l"  goto pclink
    if "%1"=="-L"  goto plink
    if "%1"=="-ll" goto locallib
    if "%1"=="-li" goto localinc
    if "!switch:~0,1!"=="-" goto extra
    goto ready

:shift_switch
    shift
    goto do_switch

:extra
    set EXTRA=!EXTRA! /!switch:~1!
    shift
    goto do_switch

:locallib
    set _LOCALLIBDIR=%2
    shift
    shift
    goto do_switch

:localinc
    set _LOCALINCDIR=%2
    shift
    shift
    goto do_switch

:-  Compile and link main programs
:pclink
    set _LINK=1
    shift
    if "%1"=="" goto help
    rem %1 is now first program to compile and link
    goto ready

:-  Link main programs without compilation
:plink
    set _LINK=1
    set _COMPILE=0
    shift
    if "%1"=="" goto help
    rem %1 is now first program to link
    goto ready

:-  Replace compiled programs into library
:preplace
    set _COMPILE=0
    set _REPLACE=1
    shift
    if "%1"=="" goto help
    set _LIB=%1
    set _LIB=%_LIB:.lib=%.lib
    shift
    if "%1"=="" goto help
    rem %1 is now first file to replace into _LIB
    goto ready

:-
:-  Determine compiler name and location, and directories to use
:-
:ready
    if not !IBASE!.==. (
        if !INCDIR!.==. (
            set INCDIR=!IBASE!\include
        ) else (
            if not !INCDIR!.==!IBASE!\include. (
                echo W: INCDIR=!INCDIR! is in conflict with IBASE=!IBASE!
            )
        )
        if !LIBDIR!.==. (
            set LIBDIR=!IBASE!\lib
        ) else (
            if not !LIBDIR!.==!IBASE!\lib. (
                echo W: LIBDIR=!LIBDIR! is in conflict with IBASE=!IBASE!
            )
        )
    )
    if "!CCNAME!"==""   goto msvc_init
    if "!CCNAME!"=="cl" goto msvc_init
    echo Unknown compiler '!CCNAME!' defined - aborting
    goto err_exit

:-
:-  MS Visual C/C++
:-  For MSVC to work from the command line, the vcvars32.bat script must be used.
:-  This overrides any settings made to the CCDIR variable.
:-
:msvc_init
    if exist "!VCINSTALLDIR!\bin\cl.exe" goto msvc_go
    echo You have either not installed MSVC, or not configured it correctly.
    echo During installation, make sure you register the environment variables
    echo needed for command-line use of the compiler.  Check the vcvars32.bat
    echo script for correctness.  In a console box, the VCINSTALLDIR variable must
    echo point correctly to the MSVC application directory.  You can set this
    echo in the system environment variables, or autoexec.bat.
    goto err_exit
:msvc_go
    set CCDIR=!VCINSTALLDIR!

    :-  Detect MSVC version
    if defined VS71COMNTOOLS (
        set _MSVCVER=71
    ) else if defined VS80COMNTOOLS (
        set _MSVCVER=80
    ) else if defined VS90COMNTOOLS (
        set _MSVCVER=90
    ) else (
        set _MSVCVER=60
    )

    :-  Prepare commands for compilation, replacement, and linking
    set CCOPTS=!CCDEFINES! !EXTRA!
    if defined _LOCALINCDIR set CCOPTS=!CCOPTS! /I"!_LOCALINCDIR!"
    if defined INCDIR       set CCOPTS=!CCOPTS! /I"!INCDIR!"
    set CCOPTS=!CCOPTS! /I"!CCDIR!\include"
    set _CC="!CCDIR!\bin\cl"   /nologo /c /W3 /D"WIN32" !CCOPTS!
    set _LR="!CCDIR!\bin\lib"  /nologo
    set _LL="!CCDIR!\bin\link" /nologo /stack:128000 /subsystem:CONSOLE /opt:NOREF /incremental:NO
    if !_DEBUG!==1  set _CC=!_CC! /MTd /Z7 /D"DEBUG" /Od
    if !_DEBUG!==0  set _CC=!_CC! /MT
    if !_MT!==1     set _CC=!_CC! /D"BASE_THREADSAFE"
    if !_DEBUG!==1  set _LL=!_LL! /DEBUG
    if !_USECPP!==1 set _CC=!_CC! /TP /GX
    if !_MSVCVER! GEQ 71 (
        set _CCLIBDIR=
        set _CC=!_CC! /Y-
        if !_DEBUG!==1 set _CC=!_CC! /RTC1
        if !_DEBUG!==0 set _CC=!_CC! /Ox
        if !_MSVCVER! GEQ 80 set _CC=!_CC! /D_CRT_SECURE_NO_DEPRECATE 
    ) else (
        set _CCLIBDIR="!CCDIR!\lib\"
        set _LR=!_LR! /libpath="!CCDIR!\lib"
        set _LL=!_LL! /libpath="!CCDIR!\lib" /PDB:NONE
        if !_DEBUG!==1 set _CC=!_CC! /GZ
        if !_DEBUG!==0 set _CC=!_CC! /Og
    )
    if !_SYNTAX!==1 goto msvc_syntax

:msvc_next
    if "%1"=="" goto exit
    set _BASE=%1
    set _BASE=!_BASE:.c=!
    set _BASE=!_BASE:.cpp=!
    set _BASE=!_BASE:.obj=!
    set _BASE=!_BASE:.opp=!
    if !_USECPP!==0 (
        set _SRC=!_BASE!.c
        set _OBJ=!_BASE!.obj
    ) else (
        set _SRC=!_BASE!.cpp
        set _OBJ=!_BASE!.opp
        set _CC=!_CC! /Fo!_OBJ!
    )

:msvc_compile
    if !_COMPILE!==0 goto msvc_replace
    if not exist !_SRC! (
        echo !_SRC! not found
        goto err_exit
    )
    if !_QUIET!==0   echo Compiling !_SRC!...
    if !_VERBOSE!==1 echo (!_CC! !_SRC!)
    set SYNTAX=!_CC:"=!
    set SYNTAX=!SYNTAX:\=/!
    !_CC! /D CCOPTS="\"!SYNTAX!\"" !_SRC!
    if errorlevel 1 (
        echo Compile errors in !_SRC!
        goto err_exit
    )

:msvc_replace
    if !_REPLACE!==0 goto msvc_link
    if not exist !_OBJ! (
        echo !_OBJ! not found
        goto err_exit
    )
    if !_VERBOSE!==1 echo Replacing !_OBJ! into !_LIB!...
    if !_VERBOSE!==1 echo (!_LR! !_LIB! !_OBJ!)
    if exist !_LIB! (
        !_LR! !_LIB! !_OBJ! >nul
    ) else (
        echo Creating new library !_LIB!...
        !_LR! /out:!_LIB! !_OBJ! >nul
    )

:msvc_link
    if !_LINK!==0 goto msvc_done
    :-  Build link library list
    rem>library.lst
    :-  List all local libraries first
    for %%a in (*.lib) do echo %%a>>library.lst
    :-  List libraries in _LOCALLIBDIR if defined
    if defined _LOCALLIBDIR for %%a in (!_LOCALLIBDIR!\*.lib) do echo %%a>>library.lst
    :-  List libraries in LIBDIR if defined
    if defined LIBDIR for %%a in (!LIBDIR!\*.lib) do echo %%a>>library.lst
    :-  List standard MSVC libraries needed for console programs
    if !_DEBUG!==1 (
        echo !_CCLIBDIR!libcmtd.lib   >>library.lst
        if !_USECPP!==1 echo !_CCLIBDIR!libcpmtd.lib  >>library.lst
    ) else (
        echo !_CCLIBDIR!libcmt.lib   >>library.lst
        if !_USECPP!==1 echo !_CCLIBDIR!libcpmt.lib  >>library.lst
    )
    echo !_CCLIBDIR!ws2_32.lib   >>library.lst
    echo !_CCLIBDIR!kernel32.lib >>library.lst
    echo !_CCLIBDIR!user32.lib   >>library.lst
    echo !_CCLIBDIR!gdi32.lib    >>library.lst
    echo !_CCLIBDIR!comdlg32.lib >>library.lst
    echo !_CCLIBDIR!advapi32.lib >>library.lst
    echo !_CCLIBDIR!netapi32.lib >>library.lst
    echo !_CCLIBDIR!shell32.lib  >>library.lst
    echo !_CCLIBDIR!odbc32.lib   >>library.lst
    echo !_CCLIBDIR!user32.lib   >>library.lst
    echo !_CCLIBDIR!wsock32.lib  >>library.lst
    echo !_CCLIBDIR!winmm.lib    >>library.lst
    echo !_CCLIBDIR!oldnames.lib >>library.lst
    echo !_CCLIBDIR!mpr.lib      >>library.lst
    echo !_CCLIBDIR!uuid.lib     >>library.lst
    echo !_CCLIBDIR!rpcrt4.lib   >>library.lst

    if not exist !_OBJ! (
        echo !_OBJ! not found
        goto err_exit
    )
    if !_QUIET!==0   echo Building !_BASE!.exe...
    if !_VERBOSE!==1 echo (!_LL! !_OBJ!)
    !_LL! !_OBJ! @library.lst
    if errorlevel 1 (
        echo Link errors in !_OBJ!
        goto err_exit
    )
    if exist !_BASE!.map del !_BASE!.map
    if exist !_BASE!.exp del !_BASE!.exp

:msvc_done
    shift
    goto msvc_next

:msvc_syntax
    if !_SYNTAX!==0 goto msvc_done

    echo Compiling using MS Visual C/C++ in !CCDIR!
    echo Compiler: !_CC!
    echo Library:  !_LR!
    echo Linker:   !_LL! with these libraries
    type library.lst
    goto exit

:err_exit
    if exist library.lst del library.lst
    if exist c.lst       del c.lst
    endlocal
    exit /b 1

:exit
    if exist library.lst del library.lst
    if exist c.lst       del c.lst
    endlocal

