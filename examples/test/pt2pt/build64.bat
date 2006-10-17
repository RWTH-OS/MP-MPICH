rem echo off
rem $Id: build.bat 3449 2005-04-25 12:07:40Z silke $
rem *** build the binaries, libs etc. for test/pt2pt
rem


rem   make sure this is the correct path to your platform sdk
rem   don't know how to determine automatically 
call "C:\Program Files\Microsoft Platform SDK\SetEnv.cmd" /XP64 /RETAIL


set VSPARAM=""

if "%1"=="/CLEAN" (
echo Cleaning test/pt2pt binaries.
set VSPARAM="/clean"
) else if "%1"=="" (
echo Building test/pt2pt with VS.NET:
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

devenv /useenv %VSPARAM% "Release x64" pt2pt.sln 


goto exit




:exit
