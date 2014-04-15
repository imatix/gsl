!----------------------------------------------------------------------------
!   ggscrp.l - GSL/script package dialog
!
!   Generated from ggscrp by ggobjt.gsl using GSL/4.
!   DO NOT MODIFY THIS FILE.
!
!   For documentation and updates see http://www.imatix.com.
!----------------------------------------------------------------------------

-schema=smtschm.c
-nostubs


After-Init:
    (--) File                               -> Parsing
          + Wait-On-Semaphore
          + Open-The-File
          + Parse-Line
    (--) String                             -> Parsing
          + Open-The-Text
          + Parse-Line
    (--) Wait                               ->
          + Wait-On-Semaphore
          + Return-Done
          + Terminate-The-Thread
    (--) Error                              ->
          + Send-File-Error-Message
          + Return-Done
          + Terminate-The-Thread

!- Dialog state while parsing text for direct output

Parsing:
    (--) Ok                                 -> Treating
          + Save-The-Node
          + Generate-Node-Type-Event
    (--) End-Of-File                        ->
          + Confirm-Control-Stack-Is-Empty
          + Return-Done
          + Terminate-The-Thread
    (--) Unmatched-Control                  ->
          + Send-Unmatched-Control-Error
          + Clean-Control-Stack
          + Return-Done
          + Terminate-The-Thread
    (--) Error                              -> Parsing
          + Send-Error-Message
          + Parse-Line

Treating:
    (--) For                                -> Parsing
          + Save-Loop-Start
          + Push-The-Control
          + Parse-Line
    (--) Scope                              -> Parsing
          + Push-The-Control
          + Parse-Line
    (--) While                              -> Parsing
          + Save-Loop-Start
          + Push-The-Control
          + Parse-Line
    (--) If                                 -> Parsing
          + Push-The-Control
          + Parse-Line
    (--) New                                -> Parsing
          + Push-The-Control
          + Parse-Line
    (--) Macro                              -> Parsing
          + Save-Loop-Start
          + Push-The-Control
          + Set-Template-Mode
          + Parse-Line
    (--) Function                           -> Parsing
          + Save-Loop-Start
          + Push-The-Control
          + Set-GSL-Mode
          + Parse-Line
    (--) Template                           -> Parsing
          + Push-The-Control
          + Set-Template-Or-GSL-Mode
          + Parse-Line
    (--) End-If                             -> After-End-If
          + Pop-If-Or-Else-Control
          + Generate-Control-Type-Event
    (--) Else                               -> Parsing
          + Pop-If-Or-For-Control
          + Push-The-Control
          + Parse-Line
    (--) Else-If                            -> Parsing
          + Pop-If-Control
          + Push-If-Control
          + Parse-Line
    (--) End-For                            -> After-End-For
          + Pop-For-Or-Else-Control
          + Generate-Control-Type-Event
    (--) End-Scope                          -> Parsing
          + Pop-Scope-Control
          + Parse-Line
    (--) End-While                          -> Parsing
          + Pop-While-Control
          + Get-Previous-Loop-Start
          + Parse-Line
    (--) End-Macro                          -> Parsing
          + Pop-Macro-Control
          + Get-Previous-Loop-Start
          + Parse-Line
    (--) End-Function                       -> Parsing
          + Pop-Function-Control
          + Get-Previous-Loop-Start
          + Parse-Line
    (--) End-New                            -> Parsing
          + Pop-New-Control
          + Parse-Line
    (--) End-Template                       -> Parsing
          + Pop-Template-Control
          + Parse-Line
    (--) Incoming                           -> Skip-Incoming
          + Read-Script-Line
    (--) Other                              -> Parsing
          + Parse-Line

After-End-If:
    (--) Else                               -> Parsing
          + Confirm-If-Block
          + Parse-Line
    (--) If                                 -> Parsing
          + Parse-Line

After-End-For:
    (--) Else                               -> Parsing
          + Confirm-For-Loop
          + Get-Previous-Loop-Start
          + Parse-Line
    (--) For                                -> Parsing
          + Get-Previous-Loop-Start
          + Parse-Line

Skip-Incoming:
    (--) Ok                                 ->
          + Extend-Script-Line
    (--) Terminator                         -> Parsing
          + Parse-Line
    (--) End-Of-File                        ->
          + Unexpected-EOF-Error
          + Clean-Control-Stack
          + Return-Done
          + Terminate-The-Thread

Defaults:
    (--) Unmatched-Control                  -> Parsing
          + Send-Unmatched-Control-Error
          + Parse-Line
    (--) Anomaly                            ->
          + Signal-Internal-Error
          + Shutdown-Everything
    (--) Shutdown                           ->
          + Terminate-The-Thread

