rem echo off
rem $Id$
rem *** build the binaries, libs etc. for NT-MPICH
rem

if "%1"=="/VS6" goto vstudio6

set VSPARAM=""

if "%1"=="/CLEAN" (
echo Cleaning NT-MPICH binaries.
set VSPARAM="/CLEAN"
if "%2"=="/VS6" goto vstudio6
) else if "%1"=="" (
echo Building NT-MPICH with VS.NET:
echo    version: RELEASE
echo    device: wsock2
echo    fortran bindings: all
echo    C++ bindings: yes
echo    clumad: yes
) else (
echo Illegal option %1. Usage:
echo  build [/CLEAN][/VS6]
goto exit
)


rem *** complete MPI/MPI-IO/MPE with ch_wsock2
if %VSPARAM%=="" (
set VSPARAM=/build
) else (
set VSPARAM=/clean
)

rem *** complete MPI/MPI-IO/MPE with ch_wsock2
devenv %VSPARAM% Release /project ch_wsock2 mpich.sln 

rem *** profiling libraries

devenv %VSPARAM% Release /project ampi mpich.sln
devenv %VSPARAM% Release /project lmpi mpich.sln
devenv %VSPARAM% Release /project tmpi mpich.sln


rem *** fortran bindings
devenv %VSPARAM% "Visual Fortran Release" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Absoft Release" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Intel Release" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Lahey Release" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Salford Release" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "G77 Release" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Visual Fortran Profiling" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Absoft Profiling" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Intel Profiling" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Lahey Profiling" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "Salford Profiling" /project nt_fortran_bindings mpich.sln
devenv %VSPARAM% "G77 Profiling" /project nt_fortran_bindings mpich.sln


rem *** C++
devenv %VSPARAM% Release /project CPP mpich.sln

rem *** clumad
echo NOT COMPILING REMOTE EXECUTION TOOLS
echo clumad compiles with VS7, but does not run
echo due to errors in RPC-stubs
echo please complie with VS6!
echo complie RexecShell with Borland C Builder
rem clumad compiles with VS7, but does not run
rem due to errors in RPC-stubs
rem pushd rexec
rem devenv %VSPARAM% Release /project rcluma rcluma.sln
rem devenv %VSPARAM% Release /project ControlApplet rcluma.sln
rem devenv %VSPARAM% Release /project ntrexec rcluma.sln
rem devenv %VSPARAM% Release /project mpirun rcluma.sln
rem devenv %VSPARAM% Release /project mpi_pro rcluma.sln
rem devenv %VSPARAM% Release /project SCIExt rcluma.sln
rem devenv %VSPARAM% ch_smi /project SCIExtCl rcluma.sln
rem devenv %VSPARAM% ch_wsock /project SVMlib rcluma.sln
rem devenv %VSPARAM% Release /project ch_lfshmem rcluma.sln
rem devenv %VSPARAM% Release /project mpich_nt rcluma.sln
rem devenv %VSPARAM% Release /project SVM_SCI rcluma.sln
rem popd



goto exit



:vstudio6

echo Building NT-MPICH with VS6:
echo    version: RELEASE
echo    device: wsock2
echo    fortran bindings: all
echo    C++ bindings: yes
echo    clumad: yes

rem *** complete MPI/MPI-IO/MPE with ch_wsock2
msdev mpich.dsw /MAKE "ch_wsock2 - Win32 Release" %VSPARAM%

rem *** profiling libraries
pushd mpe
msdev ampi.dsp /MAKE "ampi - Win32 Release" %VSPARAM%
msdev lmpi.dsp /MAKE "lmpi - Win32 Release" %VSPARAM%
msdev tmpi.dsp /MAKE "tmpi - Win32 Release" %VSPARAM%
popd

rem *** fortran bindings
pushd src\nt_fortran_bindings
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Visual Fortran Release" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Absoft Release" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Intel Release" %VSPARAM%
rem msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 FortranPlus Release" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Lahey Release" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Salford Release" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 G77 Release" %VSPARAM%

msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Visual Fortran Profiling" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Absoft Profiling" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Intel Profiling" %VSPARAM%
rem msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 FortranPlus Profiling" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Lahey Profiling" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 Salford Profiling" %VSPARAM%
msdev nt_fortran_bindings.dsp /MAKE "nt_fortran_bindings - Win32 G77 Profiling" %VSPARAM%
popd

rem *** C++
pushd MPI-2-C++
msdev  CPP.dsp /MAKE "CPP - Win32 Release" %VSPARAM%
popd

rem *** clumad
pushd rexec
msdev rcluma.dsp /MAKE "rcluma - Win32 Release" %VSPARAM%
pushd ControlApplet
msdev ControlApplet.dsp /MAKE "ControlApplet - Win32 Release" %VSPARAM%
popd
pushd ntrexec
msdev ntrexec.dsp /MAKE "ntrexec - Win32 Release" %VSPARAM%
popd
pushd mpirun
msdev mpirun.dsp /MAKE "mpirun - Win32 Release" %VSPARAM%
pushd plugins
pushd MPI_Pro
msdev mpi_pro.dsp /MAKE "mpi_pro - Win32 Release" %VSPARAM%
popd
pushd SCIExt
msdev SCIExt.dsp /MAKE "SCIExt - Win32 Release" %VSPARAM%
popd
pushd SCIExtCl
msdev SCIExtCl.dsp /MAKE "SCIExtCl - Win32 ch_smi" %VSPARAM%
popd
rem pushd SVM_SCI
rem msdev SVM_SCI.dsp /MAKE "SVM_SCI - Win32 Release" %VSPARAM%
rem popd
pushd SVMlib
msdev SVMlib.dsp /MAKE "SVMlib - Win32 ch_wsock" %VSPARAM%
popd
pushd ch_lfshmem
msdev ch_lfshmem.dsp /MAKE "ch_lfshmem - Win32 Release" %VSPARAM%
popd
pushd mpich_nt
msdev mpich_nt.dsp /MAKE "mpich_nt - Win32 Release" %VSPARAM%
popd
popd
popd
popd

if "%VSPARAM%"=="/CLEAN" (
echo Deleted all binaries and temporary files of NT-MPICH.
) else (
echo NT-MPICH binaries created.
echo The RexecShell needs to be created using the C++ Builder.
)
:exit
