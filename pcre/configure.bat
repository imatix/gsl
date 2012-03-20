@echo off
:-  PCRE configuration script
:-
:-  Generated on by iMatix Boom
:-
:-  This script is provided to assist users new to Boom.  It displays the
:-  configuration settings in effect.
:-  
:-  Copyright (c) 1996-2009 iMatix Corporation
:-  All rights reserved.
:-  
:-  This file is licensed under the BSD license as follows:
:-  
:-  Redistribution and use in source and binary forms, with or without
:-  modification, are permitted provided that the following conditions
:-  are met:
:-  
:-  * Redistributions of source code must retain the above copyright
:-    notice, this list of conditions and the following disclaimer.
:-  * Redistributions in binary form must reproduce the above copyright
:-    notice, this list of conditions and the following disclaimer in
:-    the documentation and/or other materials provided with the
:-    distribution.
:-  * Neither the name of iMatix Corporation nor the names of its
:-    contributors may be used to endorse or promote products derived
:-    from this software without specific prior written permission.
:-  
:-  THIS SOFTWARE IS PROVIDED BY IMATIX CORPORATION "AS IS" AND ANY
:-  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
:-  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
:-  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL IMATIX CORPORATION BE
:-  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
:-  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
:-  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
:-  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
:-  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
:-  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
:-  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

goto :init

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
goto :eof

:init
setlocal
call :boom_model_init
if .!IBASE!==. (
        echo boom E: The IBASE variable is not set.  Please set it to the
        echo boom E: location where PCRE should be installed.
    exit /b 1
)

echo boom I: PCRE configured for model           : !BOOM_MODEL!
echo boom I: PCRE will be installed into         : !IBASE!
echo boom I: Run 'boomake build' to build PCRE
exit /b 0
