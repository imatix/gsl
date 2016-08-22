!----------------------------------------------------------------------------*
!                                                                            *
!   gsl.l - GSL main program                                                 *
!                                                                            *
!   Copyright (c) 1996-2010 iMatix Corporation                               *
!                                                                            *
!   This program is free software; you can redistribute it and/or modify     *
!   it under the terms of the GNU General Public License as published by     *
!   the Free Software Foundation; either version 3 of the License, or (at    *
!   your option) any later version.                                          *
!                                                                            *
!   This program is distributed in the hope that it will be useful, but      *
!   WITHOUT ANY WARRANTY; without even the implied warranty of               *
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
!   General Public License for more details.                                 *
!                                                                            *
!   For information on alternative licensing for OEMs, please contact        *
!   iMatix Corporation.                                                      *
!                                                                            *
!----------------------------------------------------------------------------*

-schema=smtschm.c
-nosort
-idle

After-Init:
    (--) Ok                                 ->
          +
    (--) Message                            ->
          + Display-Error-Message
    (--) Error                              ->
          + Display-Error-Message
    (--) Fatal                              ->
          + Display-Error-Message
    (--) Shutdown                           ->
          + Terminate-The-Thread
