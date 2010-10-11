!----------------------------------------------------------------------------*
!                                                                            *
!   ggpars.l - Script parser functions                                       *
!                                                                            *
!   Copyright (c) 1996-2010 iMatix Corporation                               *
!                                                                            *
!   This program is free software; you can redistribute it and/or modify     *
!   it under the terms of the GNU General Public License as published by     *
!   the Free Software Foundation; either version 2 of the License, or (at    *
!   your option) any later version.                                          *
!                                                                            *
!   This program is distributed in the hope that it will be useful, but      *
!   WITHOUT ANY WARRANTY; without even the implied warranty of               *
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
!   General Public License for more details.                                 *
!                                                                            *
!   For information on alternative licensing for OEMs, please contact        *
!   iMatix Corporation.                                                      *
!   --------------------------------------------------------------           *
!----------------------------------------------------------------------------*

-schema=smtschm.c

After-Init: <Exceptions>
    (--) Template                           -> Allow-Text
          + Read-Script-Line
          + Generate-Script-Exception
          + Insert-Line-Node
          + Ready-For-Op1-Field
          + Get-Template-Token
    (--) GSL                                -> Expect-Script
          + Read-Script-Line
          + Generate-Script-Exception
          + Generate-Comment-Exception
          + Generate-Line-Exception
          + Get-Script-Command-Token
    (--) Expression                         -> Expect-End-Of-Line
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Script                             -> Expect-Script
          + Generate-Comment-Exception
          + Get-Script-Command-Token
    (--) Line                               -> Allow-Text
          + Insert-Line-Node
          + Ready-For-Op1-Field
          + Get-Template-Token
    (--) Comment                            ->
          + Insert-Comment-Node
          + Return-Ok-Feedback
          + Terminate-The-Thread
    (--) End-Of-File                        ->
          + Rollback-Memory-Allocations
          + Return-Eof-Feedback
          + Terminate-The-Thread

!------------------------------------- Text -----------------------------------

Allow-Text: <Exceptions>
    (--) Text                               ->
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Template-Token
    (--) Substitute                         ->
          + Call-Expect-Substitute
          + Get-Template-Token
    (--) Extend                             ->
          + Insert-Spaces-Only-Node
          + Set-Extend-Flag
          + Return-To-Root-Node
          + Return-Ok-Feedback
          + Terminate-The-Thread
    (--) End-Of-Line                        ->
          + Insert-Spaces-Only-Node
          + Return-To-Root-Node
          + Return-Ok-Feedback
          + Terminate-The-Thread

!----------------------------------- Comment ----------------------------------

Continue-Comment: <Exceptions>
    (--) Close-Comment                      ->
          + Dialog-Return
    (--) End-Of-Line                        ->
          + Read-Script-Line
          + Get-Comment-Token
    (--) Text                               ->
          + Get-Comment-Token

!----------------------------------- Numbers ----------------------------------

Expect-Signed-Number: <Expect-Unsigned-Number>
    (--) Sign                               -> Expect-Unsigned-Number
          + Save-The-Sign
          + Count-Token-Width
          + Get-Expression-Token

Expect-Unsigned-Number: <Exceptions>
    (--) Number                             ->
          + Insert-Text-Node
          + Count-Token-Width
          + Dialog-Return
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Allow-Number: <Expect-Signed-Number>
    (--) $other                             ->
          + Rewind-Token
          + Dialog-Return

!---------------------------------- Strings -----------------------------------

Expect-String: <Exceptions>
    (--) Quote                              -> Continue-String
          + Push-Quote-Character
          + Insert-Literal-Node
          + Count-Token-Width
          + Ready-For-Op1-Field
          + Get-Quoted-Token

Continue-String: <Exceptions>
    (--) Text                               ->
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Quoted-Token
    (--) Substitute                         ->
          + Call-Expect-Substitute
          + Get-Quoted-Token
    (--) Quote                              ->
          + Insert-Spaces-Only-Node
          + Return-To-String
          + Return-To-Parent
          + Count-Token-Width
          + Dialog-Return
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Quoted-Token
    (--) End-Of-Line                        ->
          + Read-Script-Line
          + Get-Quoted-Token

!-------------------------------- Identifier ----------------------------------

Expect-Identifier: <Exceptions>
    (--) Literal                            -> Continue-Identifier
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Expression-Token
    (--) Substitute                         -> Continue-Identifier
          + Call-Expect-Substitute
          + Get-Expression-Token
    (--) Open-Bracket                       -> Expect-Bracketed-Identifier
          + Count-Token-Width
          + Get-Bracketed-Identifier-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Continue-Identifier: <Exceptions>
    (--) Literal                            ->
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Expression-Token
    (--) Substitute                         ->
          + Call-Expect-Substitute
          + Get-Expression-Token
    (--) Number                             ->
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Expression-Token
    (--) $other                             ->
          + Return-To-String
          + Rewind-Token
          + Dialog-Return

Expect-Bracketed-Identifier: <Exceptions>
    (--) Literal                            -> Continue-Bracketed-Identifier
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Bracketed-Identifier-Token

Continue-Bracketed-Identifier: <Expect-Bracketed-Identifier>
    (--) Close-Bracket                      ->
          + Count-Token-Width
          + Dialog-Return

Allow-Identifier: <Expect-Identifier>
    (--) $other                             ->
          + Rewind-Token
          + Dialog-Return

!------------------------------- Data Specifier -------------------------------

Expect-Data-Specifier: <Continue-Data-Specifier>
    (--) Open-Bracket                       -> Continue-Data-Specifier
          + Rewind-Token
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Literal                            -> Continue-Data-Specifier
          + Rewind-Token
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Substitute                         -> Continue-Data-Specifier
          + Rewind-Token
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Open-Bracket                       -> Continue-Data-Specifier
          + Rewind-Token
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Number                             -> Continue-Data-Specifier
          + Rewind-Token
          + Call-Expect-Number
          + Get-Expression-Token
    (--) Sign                               -> Continue-Data-Specifier
          + Rewind-Token
          + Call-Expect-Number
          + Restore-The-Sign
          + Copy-The-Operator
          + Get-Expression-Token
    (--) Point                              -> After-Scope-Point
          + Insert-Attribute-Node
          + Count-Token-Width
          + Ready-For-Scope-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Get-Expression-Token

Continue-Data-Specifier: <Exceptions>
    (--) Member                             -> After-Member
          + Insert-Member-Node
          + Count-Token-Width
          + Ready-For-Name-Field
          + Call-Allow-Identifier
          + Return-To-Parent
          + Get-Expression-Token
    (--) Point                              -> After-Scope-Point
          + Insert-Attribute-Node
          + Count-Token-Width
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             ->
          + Return-To-Data-Specifier
          + Rewind-Token
          + Dialog-Return

After-Member: <Exceptions>
    (--) Open                               -> After-Member-Expression
          + Ready-For-Before-Field
          + Count-Token-Width
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             -> Continue-Data-Specifier
          + Rewind-Token
          + Get-Expression-Token

After-Member-Expression: <Exceptions>
    (--) Close                              -> Continue-Data-Specifier
          + Count-Token-Width
          + Get-Expression-Token
    (--) Next-Arg                           -> After-Member-As
          + Ready-For-As-Field
          + Count-Token-Width
          + Call-Expect-Identifier
          + Return-To-Parent
          + Get-Expression-Token

After-Member-As: <Exceptions>
    (--) Close                              -> Continue-Data-Specifier
          + Count-Token-Width
          + Get-Expression-Token

After-Scope-Point: <Exceptions>
    (--) Literal                            -> Continue-Data-Specifier
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Return-To-Parent
          + Get-Expression-Token
    (--) Substitute                         -> Continue-Data-Specifier
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Return-To-Parent
          + Get-Expression-Token
    (--) Open-Bracket                       -> Continue-Data-Specifier
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Return-To-Parent
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             ->
          + Ready-For-Name-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Rewind-Token
          + Dialog-Return

!------------------------------ Substitution ----------------------------------

! We arrive in state Expect-Substitute pointing at child so that spaces get
! counted with substitute width.  Important for determining correct width of
! substitute construct.

Expect-Substitute: <Exceptions>
    (--) Pretty                             -> Expect-Modifier
          + Count-Token-Width
          + Return-To-Parent
          + Ready-For-Op2-Field
          + Get-Modifier-Token
    (--) Format                             -> Expect-Modifier
          + Count-Token-Width
          + Return-To-Parent
          + Ready-For-As-Field
          + Get-Modifier-Token
    (--) Close                              ->
          + Count-Token-Width
          + Return-To-Parent
          + Dialog-Return
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Modifier-Token

Expect-Modifier: <Exceptions>
    (--) Text                               ->
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Modifier-Token
    (--) Substitute                         ->
          + Call-Expect-Substitute
          + Get-Modifier-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Modifier-Token
    (--) $other                             -> Expect-Substitute
          + Insert-Empty-Node-If-Needed
          + Return-To-String
          + Rewind-Token
          + Get-Modifier-Token

!------------------------------- Expression -----------------------------------

Expect-Operand: <Exceptions>
    (--) Quote                              -> Allow-Operator
          + Rewind-Token
          + Call-Expect-String
          + Get-Expression-Token
    (--) Sign                               -> After-Sign
          + Save-The-Sign
          + Count-Token-Width
          + Get-Expression-Token
    (--) Unary-Operator                     -> Expect-Operand
          + Count-Token-Width
          + Insert-Unary-Operator-Node
          + Ready-For-Op2-Field
          + Get-Expression-Token
    (--) Number                             -> After-Number
          + Insert-Number-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Number
          + Get-Expression-Token
    (--) Point                              -> After-Point
          + Count-Token-Width
          + Get-Expression-Token
    (--) Literal                            -> Allow-Operator
          + Insert-Symbol-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Open-Bracket                       -> Allow-Operator
          + Insert-Symbol-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Member                             -> Allow-Operator
          + Insert-Symbol-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Substitute                         -> Allow-Operator
          + Insert-Symbol-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Open                               -> Expect-Operand
          + Add-One-Bracket
          + Count-Token-Width
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

After-Sign: <Exceptions>
    (--) Number                             -> After-Number
          + Insert-Number-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Number
          + Get-Expression-Token
    (--) Point                              -> After-Sign-Point
          + Count-Token-Width
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) $other                             -> Expect-Operand
          + Restore-The-Sign
          + Insert-Unary-Operator-Node
          + Ready-For-Op2-Field
          + Rewind-Token
          + Get-Expression-Token

After-Sign-Point: <Exceptions>
    (--) Number                             -> Allow-Operator
          + Insert-Number-Node
          + Ready-For-Op2-Field
          + Rewind-Token
          + Call-Expect-Number
          + Return-To-Parent
          + Restore-The-Sign
          + Copy-The-Operator
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) $other                             -> After-Point
          + Insert-Unary-Operator-Node
          + Ready-For-Op2-Field
          + Rewind-Token
          + Get-Expression-Token

After-Point: <Exceptions>
    (--) Number                             -> Allow-Operator
          + Insert-Number-Node
          + Ready-For-Op2-Field
          + Rewind-Token
          + Call-Expect-Number
          + Return-To-Parent
          + Restore-The-Sign
          + Copy-The-Operator
          + Get-Expression-Token
    (--) Literal                            -> Allow-Operator
          + Insert-Symbol-Node
          + Ready-For-Op1-Field
          + Insert-Attribute-Node
          + Ready-For-Scope-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Call-Continue-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Substitute                         -> Allow-Operator
          + Insert-Symbol-Node
          + Ready-For-Op1-Field
          + Insert-Attribute-Node
          + Ready-For-Scope-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Call-Continue-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Open-Bracket                       -> Allow-Operator
          + Insert-Symbol-Node
          + Ready-For-Op1-Field
          + Insert-Attribute-Node
          + Ready-For-Scope-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Call-Continue-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) $other                             -> Allow-Operator
          + Insert-Symbol-Node
          + Rewind-Token
          + Get-Expression-Token

After-Number: <Exceptions>
    (--) Point                              -> After-Number-Point
          + Count-Token-Width
          + Get-Expression-Token
    (--) Member                             -> Allow-Operator
          + Restore-The-Sign
          + Copy-The-Operator
          + Rewind-Token
          + Call-Continue-Data-Specifier
          + Return-To-Parent
          + Change-Number-To-Symbol-Node
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             -> Allow-Operator
          + Restore-The-Sign
          + Copy-The-Operator
          + Return-To-Parent
          + Rewind-Token
          + Get-Expression-Token

After-Number-Point: <Exceptions>
    (--) Number                             -> Allow-Operator
          + Return-To-Parent
          + Restore-The-Sign
          + Copy-The-Operator
          + Ready-For-Op2-Field
          + Rewind-Token
          + Call-Expect-Number
          + Return-To-Parent
          + Get-Expression-Token
    (--) Literal                            -> Allow-Operator
          + Restore-The-Sign
          + Copy-The-Operator
          + Insert-Attribute-Node
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Return-To-Parent
          + Call-Continue-Data-Specifier
          + Return-To-Parent
          + Change-Number-To-Symbol-Node
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Substitute                         -> Allow-Operator
          + Restore-The-Sign
          + Copy-The-Operator
          + Insert-Attribute-Node
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Return-To-Parent
          + Call-Continue-Data-Specifier
          + Return-To-Parent
          + Change-Number-To-Symbol-Node
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Open-Bracket                       -> Allow-Operator
          + Restore-The-Sign
          + Copy-The-Operator
          + Insert-Attribute-Node
          + Ready-For-Name-Field
          + Rewind-Token
          + Call-Expect-Identifier
          + Return-To-Parent
          + Call-Continue-Data-Specifier
          + Return-To-Parent
          + Change-Number-To-Symbol-Node
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) $other                             -> Allow-Operator
          + Rewind-Token
          + Get-Expression-Token

Allow-Operand: <Expect-Operand>
    (--) $other                             ->
          + Return-To-Expression
          + Rewind-Token
          + Dialog-Return

Allow-Arguments: <Exceptions>
    (--) Open                               -> Continue-Arguments
          + Change-To-Call-Node
          + Ready-For-Op1-Field
          + Count-Token-Width
          + Call-Allow-Operand
          + Get-Expression-Token
    (--) $other                             ->
          + Rewind-Token
          + Dialog-Return

Continue-Arguments: <Exceptions>
    (--) Next-Arg                           ->
          + Insert-Operator-Node
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Call-Allow-Operand
          + Get-Expression-Token
    (--) Close                              ->
          + Count-Token-Width
          + Return-To-Arguments
          + Return-To-Parent
          + Dialog-Return
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

!--------------------------------- Operator -----------------------------------

Allow-Operator: <Exceptions>
    (--) Unary-Operator                     -> Expect-Operand
          + Insert-Operator-Node
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Get-Expression-Token
    (--) Sign                               -> Expect-Operand
          + Insert-Operator-Node
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Get-Expression-Token
    (--) Operator                           -> Expect-Operand
          + Insert-Operator-Node
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Get-Expression-Token
    (--) Default-Operator                   -> Allow-Operand
          + Insert-Operator-Node
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Get-Expression-Token
    (--) Close                              -> Allow-Operator
          + Close-One-Bracket
          + Count-Token-Width
          + Get-Expression-Token
    (--) No-Bracket                         ->
          + Return-To-Expression
          + Rewind-Token
          + Dialog-Return
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             ->
          + Return-To-Expression
          + Rewind-Token
          + Dialog-Return

!--------------------------------- Script -------------------------------------

Expect-Script: <Exceptions>
    (--) Comment                            ->
          + Insert-Comment-Node
          + Return-Ok-Feedback
          + Terminate-The-Thread
    (--) End-Of-Line                        ->
          + Insert-Comment-Node
          + Return-Ok-Feedback
          + Terminate-The-Thread
    (--) Close                              -> Expect-End-Of-Line
          + Insert-Close-Node
          + Get-Expression-Token
    (--) Else                               -> Expect-End-Of-Line
          + Insert-Else-Node
          + Get-Expression-Token
    (--) End-If                             -> Expect-End-Of-Line
          + Insert-End-If-Node
          + Get-Expression-Token
    (--) End-For                            -> Expect-End-Of-Line
          + Insert-End-For-Node
          + Ready-For-Op1-Field
          + Call-Allow-Identifier
          + Get-Expression-Token
    (--) End-Scope                          -> Expect-End-Of-Line
          + Insert-End-Scope-Node
          + Ready-For-Op1-Field
          + Call-Allow-Identifier
          + Get-Expression-Token
    (--) End-Macro                          -> Expect-End-Of-Line
          + Insert-End-Macro-Node
          + Ready-For-Op1-Field
          + Call-Allow-Identifier
          + Get-Expression-Token
    (--) End-Function                       -> Expect-End-Of-Line
          + Insert-End-Function-Node
          + Ready-For-Op1-Field
          + Call-Allow-Identifier
          + Get-Expression-Token
    (--) End-New                            -> Expect-End-Of-Line
          + Insert-End-New-Node
          + Ready-For-Op1-Field
          + Call-Allow-Identifier
          + Get-Expression-Token
    (--) End-While                          -> Expect-End-Of-Line
          + Insert-End-While-Node
          + Get-Expression-Token
    (--) Output                             -> Expect-End-Of-Line
          + Insert-Output-Node
          + Ready-For-Op1-Field
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Append                             -> Expect-End-Of-Line
          + Insert-Append-Node
          + Ready-For-Op1-Field
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Echo                               -> Expect-End-Of-Line
          + Insert-Echo-Node
          + Ready-For-Op1-Field
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Abort                              -> Expect-End-Of-Line
          + Insert-Abort-Node
          + Ready-For-Op1-Field
          + Call-Allow-Operand
          + Get-Expression-Token
    (--) Define                             -> Expect-Assign
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
!- Literals/Substitute, point, member, number and sign may be either
!- implicit .invoke or .define
    (--) Literal                            -> Expect-Assign-Or-Args
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Open-Bracket                       -> Expect-Assign-Or-Args
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Substitute                         -> Expect-Assign-Or-Args
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Point                              -> Expect-Assign-Or-Args
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Member                             -> Expect-Assign-Or-Args
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Number                             -> Expect-Assign-Or-Args
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Sign                               -> Expect-Assign-Or-Args
          + Save-The-Sign
          + Insert-Define-Node
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
!- Macro/function stuff
    (--) Macro                              -> Expect-End-Of-Line
          + Insert-Macro-Node
          + Ready-For-Op1-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Macro-Args
          + Get-Expression-Token
    (--) Function                           -> Expect-End-Of-Line
          + Insert-Function-Node
          + Ready-For-Op1-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Macro-Args
          + Get-Expression-Token
    (--) Return                             -> Expect-End-Of-Line
          + Insert-Return-Node
          + Ready-For-Op1-Field
          + Call-Allow-Operand
          + Get-Expression-Token
    (--) Invoke                             -> Expect-End-Of-Line
          + Insert-Call-Node
          + Ready-For-Op1-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Call-Allow-Arguments
          + Get-Expression-Token
!- Script stuff
    (--) Include                            -> Expect-End-Of-Line
          + Insert-GSL-Node
          + Ready-For-Scope-Field
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Gsl                                -> Expect-Source
          + Insert-Gsl-Node
          + Get-Script-Token
    (--) Direct                             -> Expect-Direct
          + Insert-Direct-Node
          + Get-Script-Token
    (--) XML                                -> Expect-Source
          + Insert-XML-Node
          + Call-Allow-Locator
          + Get-Script-Token
    (--) Template                           -> Expect-End-Of-Line
          + Insert-Template-Node
          + Ready-For-Op1-Field
          + Call-Allow-Number
          + Get-Expression-Token
    (--) End-Template                       -> Expect-End-Of-Line
          + Insert-End-Template-Node
          + Get-Expression-Token
    (--) Save                               -> Expect-Save
          + Insert-Save-Node
          + Ready-For-Scope-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Get-Expression-Token
!- Basic control structures
    (--) If                                 -> Expect-End-Of-Line
          + Insert-If-Node
          + Ready-For-Op1-Field
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Elsif                              -> Expect-End-Of-Line
          + Insert-Elsif-Node
          + Ready-For-Op1-Field
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Move                               -> Expect-End-Of-Line
          + Insert-Move-Node
          + Ready-For-Scope-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Call-Allow-Locator
          + Get-Expression-Token
    (--) Copy                               -> Expect-End-Of-Line
          + Insert-Copy-Node
          + Ready-For-Scope-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Call-Allow-Locator
          + Get-Expression-Token
    (--) Delete                             -> Expect-End-Of-Line
          + Insert-Delete-Node
          + Ready-For-Scope-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Call-Allow-Locator
          + Get-Expression-Token
    (--) New                                -> Expect-New
          + Insert-New-Node
          + Ready-For-Op1-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) For                                -> Expect-For
          + Insert-For-Node
          + Ready-For-Op1-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Scope                              -> Expect-Scope
          + Insert-Scope-Node
          + Ready-For-Scope-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Get-Script-Token
    (--) While                              -> Expect-End-Of-Line
          + Insert-While-Node
          + Ready-For-Op1-Field
          + Call-Expect-Operand
          + Get-Expression-Token
    (--) Next                               -> Expect-End-Of-Line
          + Insert-Next-Node
          + Ready-For-Op1-Field
          + Call-Allow-Identifier
          + Get-Expression-Token
    (--) Last                               -> Expect-End-Of-Line
          + Insert-Last-Node
          + Ready-For-Op1-Field
          + Call-Allow-Identifier
          + Get-Expression-Token
    (--) Sort                               -> Expect-For
          + Insert-Sort-Node
          + Ready-For-Op1-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Separate-Name-From-Scope
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Expect-End-Of-Line: <Exceptions>
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) End-Of-Line                        ->
          + Return-To-Root-Node
          + Return-Ok-Feedback
          + Terminate-The-Thread
    (--) Comment                            ->
          + Return-To-Root-Node
          + Return-Ok-Feedback
          + Terminate-The-Thread
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Expect-Assign: <Exceptions>
    (--) Simple-Assign                      -> Expect-End-Of-Line
          + Copy-The-Operator
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Call-Allow-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Arithmetic-Assign                  -> Expect-End-Of-Line
          + Copy-The-Operator
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Expect-Assign-Or-Args: <Exceptions>
    (--) Simple-Assign                      -> Expect-End-Of-Line
          + Copy-The-Operator
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Call-Allow-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Arithmetic-Assign                  -> Expect-End-Of-Line
          + Copy-The-Operator
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Open                               -> Expect-End-Of-Line
          + Change-To-Call-Node
          + Rewind-Token
          + Call-Allow-Arguments
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Allow-Macro-Args: <Exceptions>
    (--) Open                               -> After-Open-Macro-Args
          + Ready-For-Op1-Field
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other
          + Rewind-Token
          + Dialog-Return

After-Open-Macro-Args: <Exceptions>
    (--) Literal                            -> Continue-Macro-Args
          + Rewind-Token
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Substitute                         -> Continue-Macro-Args
          + Rewind-Token
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Open-Bracket                       -> Continue-Macro-Args
          + Rewind-Token
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Close                              ->
          + Return-To-Parent
          + Dialog-Return

Continue-Macro-Args: <Exceptions>
    (--) Next-Arg                           ->
          + Insert-Operator-Node
          + Count-Token-Width
          + Ready-For-Op2-Field
          + Call-Expect-Identifier
          + Get-Expression-Token
    (--) Close                              ->
          + Return-To-Arguments
          + Return-To-Parent
          + Dialog-Return
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Expect-Source: <Exceptions>
    (--) From                               -> Expect-End-Of-Line
          + Ready-For-Scope-Field
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Incoming                           -> Expect-Terminator
          + Ready-For-Name-Field
          + Get-Template-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             -> Expect-End-Of-Line
          + Ready-For-Op1-Field
          + Rewind-Token
          + Call-Expect-Operand
          + Return-To-Parent
          + Rewind-Token
          + Get-Expression-Token

Expect-Terminator: <Exceptions>
    (--) Text                               -> Expect-End-Of-Line
          + Insert-Text-Node
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Expression-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Expression-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Expect-Direct: <Exceptions>
    (--) From                               -> Expect-End-Of-Line
          + Ready-For-Scope-Field
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Incoming                           -> Expect-Terminator
          + Ready-For-Name-Field
          + Get-Template-Token
    (--) Quote                              -> Continue-Direct
          + Push-Quote-Character
          + Ready-For-Op1-Field
          + Get-Direct-Quoted-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token

Continue-Direct: <Exceptions>
    (--) Text                               ->
          + Insert-Text-Node
          + Count-Token-Width
          + Get-Direct-Quoted-Token
    (--) End-Of-Line                        ->
          + Read-Script-Line
          + Get-Direct-Quoted-Token
    (--) Quote                              -> Expect-End-Of-Line
          + Insert-Spaces-Only-Node
          + Return-To-String
          + Return-To-Parent
          + Get-Expression-Token

Allow-Locator: <Exceptions>
    (--) To                                 ->
          + Ready-For-To-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Get-Script-Token
    (--) Before                             ->
          + Ready-For-Before-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Get-Script-Token
    (--) After                              ->
          + Ready-For-After-Field
          + Call-Expect-Data-Specifier
          + Return-To-Parent
          + Get-Script-Token
    (--) As                                 ->
          + Ready-For-As-Field
          + Call-Expect-Identifier
          + Return-To-Parent
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             ->
          + Rewind-Token
          + Dialog-Return

Expect-Save: <Exceptions>
    (--) As                                 -> Expect-End-Of-Line
          + Ready-For-As-Field
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Expression-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             -> Expect-End-Of-Line
          + Rewind-Token
          + Get-Expression-Token

Expect-New: <Allow-Locator>
    (--) Noalias                            ->
          + Ready-For-As-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Get-Script-Token
    (--) Nostack                            ->
          + Unset-Stacked-Flag
          + Get-Script-Token
    (--) $other                             -> Expect-End-Of-Line
          + Rewind-Token
          + Get-Expression-Token

Expect-For: <Exceptions>
    (--) As                                 ->
          + Ready-For-As-Field
          + Call-Expect-Identifier
          + Return-To-Parent
          + Get-Script-Token
    (--) Noalias                            ->
          + Ready-For-As-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Get-Script-Token
    (--) Where                              ->
          + Ready-For-Before-Field
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Script-Token
    (--) By                                 ->
          + Ready-For-After-Field
          + Call-Expect-Operand
          + Return-To-Parent
          + Get-Script-Token
    (--) Nostack                            ->
          + Unset-Stacked-Flag
          + Get-Script-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             -> Expect-End-Of-Line
          + Rewind-Token
          + Get-Expression-Token

Expect-Scope: <Exceptions>
    (--) As                                 ->
          + Ready-For-As-Field
          + Call-Expect-Identifier
          + Return-To-Parent
          + Get-Script-Token
    (--) Noalias                            ->
          + Ready-For-As-Field
          + Insert-Empty-Text-Node
          + Return-To-Parent
          + Get-Script-Token
    (--) Nostack                           ->
          + Unset-Stacked-Flag
          + Get-Script-Token
    (--) Spaces                             ->
          + Count-Token-Width
          + Get-Script-Token
    (--) Extend                             ->
          + Set-Extend-Flag
          + Read-Script-Line
          + Get-Script-Token
    (--) Open-Comment                       ->
          + Call-Continue-Comment
          + Get-Expression-Token
    (--) $other                             -> Expect-End-Of-Line
          + Rewind-Token
          + Get-Expression-Token

Defaults:
    (--) End-Of-File                        ->
          + Signal-Unexpected-End-Of-File
          + Rollback-Memory-Allocations
          + Return-Error-Feedback
          + Terminate-The-Thread
    (--) Other                              ->
          + Signal-Invalid-Token
          + Rollback-Memory-Allocations
          + Return-Error-Feedback
          + Terminate-The-Thread
    (--) $other                             ->
          + Signal-Invalid-Token
          + Rollback-Memory-Allocations
          + Return-Error-Feedback
          + Terminate-The-Thread
    (--) Shutdown                           ->
          + Rollback-Memory-Allocations
          + Terminate-The-Thread

Exceptions:
    (--) Expect-Data-Specifier              -> Expect-Data-Specifier
          + Get-Expression-Token
    (--) Expect-Substitute                  -> Expect-Substitute
          + Insert-Substitute-Node
          + Count-Token-Width
          + Ready-For-Op1-Field
          + Call-Expect-Operand
          + Get-Modifier-Token
    (--) Expect-Operand                     -> Expect-Operand
          + Get-Expression-Token
    (--) Expect-Identifier                  -> Expect-Identifier
          + Get-Expression-Token
    (--) Continue-Comment                   -> Continue-Comment
          + Get-Comment-Token
    (--) Expect-Number                      -> Expect-Signed-Number
          + Get-Expression-Token
    (--) Expect-Identifier                  -> Expect-Identifier
          + Get-Expression-Token
    (--) Expect-String                      -> Expect-String
          + Get-Expression-Token
    (--) Continue-Data-Specifier            -> Continue-Data-Specifier
          + Get-Expression-Token
    (--) Allow-Operand                      -> Allow-Operand
          + Get-Expression-Token
    (--) Allow-Identifier                   -> Allow-Identifier
          + Get-Expression-Token
    (--) Allow-Macro-Args                   -> Allow-Macro-Args
          + Get-Expression-Token
    (--) Allow-Locator                      -> Allow-Locator
          + Get-Expression-Token
    (--) Allow-Number                       -> Allow-Number
          + Get-Expression-Token
    (--) Allow-Arguments                    -> Allow-Arguments
          + Get-Expression-Token
    (--) Anomaly                            ->
          + Signal-Internal-Error
          + Rollback-Memory-Allocations
          + Return-Error-Feedback
          + Terminate-The-Thread
    (--) Error
          + Rollback-Memory-Allocations
          + Return-Error-Feedback
          + Terminate-The-Thread

