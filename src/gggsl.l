!----------------------------------------------------------------------------
!   gggsl.l - GSL/gsl control package dialog
!
!   Generated from gggsl by ggobjt.gsl using GSL/4.
!   DO NOT MODIFY THIS FILE.
!
!   For documentation and updates see http://www.imatix.com.
!----------------------------------------------------------------------------

-schema=smtschm.c
-nostubs


Starting:
    (--) Execute                            -> Executing
          +
          
Executing:
    (--) Ok                                 ->
          + Return-Done
          + Terminate-The-Thread
    (--) Message                            ->
          + Forward-Message-As-Call-Message
    (--) Error                              ->
          + Forward-Message-As-Call-Error
          + Terminate-The-Thread
    (--) Fatal                              ->
          + Forward-Message-As-Call-Error
          + Terminate-The-Thread
          
Defaults:
    (--) Shutdown                           ->
          + Terminate-The-Thread

