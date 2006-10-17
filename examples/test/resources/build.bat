rem echo off
rem $Id: build.bat 3449 2005-04-25 12:07:40Z silke $
rem *** build the binaries, libs etc. for test/resources
rem



set VSPARAM=""

if "%1"=="/CLEAN" (
echo Cleaning test/resources binaries.
set VSPARAM="/clean"
) else if "%1"=="" (
echo Building test/resources with VS.NET:
) else (
echo Illegal option %1. Usage:
echo  build [/CLEAN]
goto exit
)


if %VSPARAM%=="" (
set VSPARAM=/build
) else (
set VSPARAM=/clean
)

devenv %VSPARAM% Release resources.sln 


goto exit




:exit
