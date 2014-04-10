!----------------------------------------------------------------------------
!   ggthrd.l - GSL/thrd package dialog
!
!   Generated from ggthrd by ggobjt.gsl using GSL/4.
!   DO NOT MODIFY THIS FILE.
!
!   For documentation and updates see http://www.imatix.com.
!----------------------------------------------------------------------------

-schema=smtschm.c
-nostubs


After-Init:
    (--) Parse                              -> Parsing
          +
    (--) Run                                -> Running
          + Spawn-GSL-Thread
    (--) Parse-And-Run                      -> Parsing-Before-Run
          +

Parsing:
    (--) Message                            ->
          + Log-Message
    (--) Ok                                 ->
          + Reply-Parsed-Ok-Result
          + Terminate-The-Thread
    (--) Error                              ->
          + Reply-Parse-Error-Result
          + Destroy-The-Thread
          + Terminate-The-Thread
    (--) Shutdown                           ->
          + Terminate-The-Thread

Running:
    (--) Ok                                 ->
          + Set-Thread-Finished-Status
          + Terminate-The-Thread
    (--) Error                              ->
          + Set-Thread-Error-Status
          + Terminate-The-Thread
    (--) Shutdown                           ->
          + Terminate-The-Thread

Parsing-Before-Run:
    (--) Message                            ->
          + Log-Message
    (--) Ok                                 -> Running
          + Reply-Running-Ok-Result
          + Spawn-GSL-Thread
    (--) Error                              ->
          + Reply-Parse-Error-Result
          + Destroy-The-Thread
          + Terminate-The-Thread
    (--) Shutdown                           ->
          + Terminate-The-Thread

Defaults:
    (--) Interrupt                          ->
          + Shutdown-Running-GSL-Thread
          + Terminate-The-Thread


