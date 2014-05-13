@ECHO OFF

CALL buildbase.bat ..\vs2012\gsl.sln 11
ECHO.
CALL buildbase.bat ..\vs2010\gsl.sln 10
ECHO.

PAUSE