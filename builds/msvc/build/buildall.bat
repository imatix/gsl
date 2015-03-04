@ECHO OFF
ECHO.
ECHO Downloading gsl dependencies from NuGet
REM Yes this is redundant, but much simpler then a central repo.
CALL nuget.exe install ..\vs2012\gsl\packages.config -o ..\vs2012\packages
CALL nuget.exe install ..\vs2010\gsl\packages.config -o ..\vs2010\packages
ECHO.
CALL buildbase.bat ..\vs2012\gsl.sln 11
ECHO.
CALL buildbase.bat ..\vs2010\gsl.sln 10
ECHO.
PAUSE