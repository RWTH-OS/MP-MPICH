rem echo off
rem $Id: build.bat 3449 2005-04-25 12:07:40Z silke $
rem *** build the binaries, libs etc. for test/coll
rem



set VSPARAM=""

if "%1"=="/CLEAN" (
echo Cleaning test/coll binaries.
set VSPARAM="/clean"
) else if "%1"=="" (
echo Building test/coll with VS.NET:
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

devenv %VSPARAM% Release /project allgatherf coll.sln 
devenv %VSPARAM% Release /project allred coll.sln 
devenv %VSPARAM% Release /project allredf coll.sln 
devenv %VSPARAM% Release /project allredmany coll.sln 
devenv %VSPARAM% Release /project alltoallv coll.sln 
devenv %VSPARAM% Release /project assocf coll.sln 
devenv %VSPARAM% Release /project barrier coll.sln 
devenv %VSPARAM% Release /project bcast coll.sln 
devenv %VSPARAM% Release /project bcastbug coll.sln 
devenv %VSPARAM% Release /project bcastbug2 coll.sln 
devenv %VSPARAM% Release /project bcastvec coll.sln 
devenv %VSPARAM% Release /project coll1 coll.sln 
devenv %VSPARAM% Release /project coll10 coll.sln 
devenv %VSPARAM% Release /project coll11 coll.sln 
devenv %VSPARAM% Release /project coll12 coll.sln 
devenv %VSPARAM% Release /project coll13 coll.sln 
devenv %VSPARAM% Release /project coll2 coll.sln 
devenv %VSPARAM% Release /project coll3 coll.sln 
devenv %VSPARAM% Release /project coll4 coll.sln 
devenv %VSPARAM% Release /project coll5 coll.sln 
devenv %VSPARAM% Release /project coll6 coll.sln 
devenv %VSPARAM% Release /project coll7 coll.sln 
devenv %VSPARAM% Release /project coll8 coll.sln 
devenv %VSPARAM% Release /project coll9 coll.sln 
devenv %VSPARAM% Release /project grouptest coll.sln 
devenv %VSPARAM% Release /project longuser coll.sln 
devenv %VSPARAM% Release /project redscat coll.sln 
devenv %VSPARAM% Release /project redtest coll.sln 
devenv %VSPARAM% Release /project scantst coll.sln 
devenv %VSPARAM% Release /project scattern coll.sln 
devenv %VSPARAM% Release /project scatterv coll.sln 
devenv %VSPARAM% Release /project shortint coll.sln 


goto exit




:exit
