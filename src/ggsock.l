!----------------------------------------------------------------------------
!   ggsock.l - GSL/socket package dialog
!
!   Generated from ggsock by ggobjt.gsl using GSL/4.
!   DO NOT MODIFY THIS FILE.
!
!   For documentation and updates see http://www.imatix.com.
!----------------------------------------------------------------------------

-schema=smtschm.c
-nostubs


After-Init:
    (--) Ok                                 -> Waiting
          +

Waiting:
    (--) Ok                                 ->
          + Reply-Ok-Result
          + Terminate-The-Thread
    (--) Error                              ->
          + Reply-Error-Result
          + Terminate-The-Thread
    (--) Closed                             ->
          + Reply-Closed-Result
          + Terminate-The-Thread
    (--) Timeout                            ->
          + Reply-Timeout-Result
          + Terminate-The-Thread
    (--) Connect-Ok                         ->
          + Reply-Connect-Ok-Result
          + Terminate-The-Thread
    (--) Readh-Ok                           ->
          + Reply-Readh-Ok-Result
          + Terminate-The-Thread
    (--) Readh-Closed                       ->
          + Reply-Readh-Closed-Result
          + Terminate-The-Thread
    (--) Readh-Timeout                      ->
          + Reply-Readh-Timeout-Result
          + Terminate-The-Thread
    (--) Shutdown                           ->
          + Terminate-The-Thread

