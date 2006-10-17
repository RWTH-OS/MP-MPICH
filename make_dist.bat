@echo off
rem $Id$
rem Create a binary distribution for Win32 from a compiled source tree

if "%1"=="" goto HELP
SET DEST="%1\NT-MPICH"

echo *** Creating MP-MPICH binary distribution in %DEST% ***
echo *** Press CTRL-C to QUIT or any key to continue ***
PAUSE

md %DEST%

md %DEST%\bin
md %DEST%\bin\plugins
md %DEST%\lib
md %DEST%\doc
md %DEST%\include
md %DEST%\examples
md %DEST%\examples\Basic
md %DEST%\examples\test
md %DEST%\examples\MPE
md %DEST%\examples\test\coll
md %DEST%\examples\test\context
md %DEST%\examples\test\env
md %DEST%\examples\C++
md %DEST%\examples\C++\examples
md %DEST%\examples\C++\test_suite
md %DEST%\examples\test\pt2pt
md %DEST%\examples\test\pt2pt\fairness
md %DEST%\examples\RomIO
md %DEST%\examples\RomIO\std
md %DEST%\examples\test\topol
md %DEST%\slog_api
md %DEST%\slog_api\bin
md %DEST%\slog_api\lib
md %DEST%\slog_api\include
md %DEST%\slog_api\test
md %DEST%\slog_api\doc
md %DEST%\slog_api\doc\html
md %DEST%\jumpshot
md %DEST%\jumpshot\bin
md %DEST%\jumpshot\lib
md %DEST%\jumpshot\lib\data
md %DEST%\jumpshot\lib\images
md %DEST%\jumpshot\lib\logfiles
md %DEST%\jumpshot\swing
md %DEST%\jumpshot-3
md %DEST%\jumpshot-3\bin
md %DEST%\jumpshot-3\bin\images
md %DEST%\jumpshot-3\lib
md %DEST%\jumpshot-3\doc
md %DEST%\jumpshot-3\swing
md %DEST%\profiling
md %DEST%\profiling\lib
md %DEST%\profiling\examples
md %DEST%\www
md %DEST%\www\www1
md %DEST%\www\www3
md %DEST%\www\www4

echo "Creating bin directory"
copy bin\*.exe %DEST%\bin
copy bin\*.dll %DEST%\bin
copy bin\*.cpl %DEST%\bin
copy bin\*.bat %DEST%\bin
copy examples\basic\release\cpi.exe %DEST%\bin
copy rexec\RexecShell\RexecShell.exe %DEST%\bin

copy bin\mpi_proto %DEST%\bin
copy rexec\mpirun\plugins\libs\ch*.dll %DEST%\bin\plugins
copy rexec\mpirun\plugins\libs\mpich_nt.dll %DEST%\bin\plugins
copy rexec\mpirun\plugins\libs\mpi_pro.dll %DEST%\bin\plugins

echo Creating lib directory
copy lib\*.lib %DEST%\lib
copy lib\*.dll %DEST%\lib
copy lib\*.txt %DEST%\lib

echo "Creating examples\basic"
copy examples\basic\*.dsp %DEST%\examples\basic
copy examples\basic\*.dsw %DEST%\examples\basic
copy examples\basic\*.c %DEST%\examples\basic
copy examples\basic\*.cc %DEST%\examples\basic
copy examples\basic\*.f %DEST%\examples\basic
copy examples\basic\*.f90 %DEST%\examples\basic
copy examples\basic\README %DEST%\examples\basic

echo "Creating examples\test"
copy examples\test\*.c %DEST%\examples\test
copy examples\test\*.bat %DEST%\examples\test
copy examples\test\*.txt %DEST%\examples\test
copy examples\test\*.f %DEST%\examples\test
copy examples\test\README. %DEST%\examples\test

SET DIRS=COLL CONTEXT ENV pt2pt topol profile
for %%i in (%DIRS%)  do (
echo "Creating examples\test\%%i"
 copy examples\test\%%i\*.h %DEST%\examples\test\%%i 
 copy examples\test\%%i\*.c %DEST%\examples\test\%%i
 copy examples\test\%%i\*.std %DEST%\examples\test\%%i 
 copy examples\test\%%i\*.f %DEST%\examples\test\%%i
 copy examples\test\%%i\*.dsw %DEST%\examples\test\%%i
 copy examples\test\%%i\*.dsp %DEST%\examples\test\%%i
 copy examples\test\%%i\*.vbs %DEST%\examples\test\%%i
)

echo "Creating examples\testpt2pt\fairness"
copy examples\test\pt2pt\fairness\*.c %DEST%\examples\test\pt2pt\fairness
copy examples\test\pt2pt\fairness\README %DEST%\examples\test\pt2pt\fairness

echo "Creating examples\romio\test"
copy RomIO\test\*.c %DEST%\examples\RomIO
copy RomIO\test\*.cpp %DEST%\examples\RomIO
copy RomIO\test\*. %DEST%\examples\RomIO
copy RomIO\test\*.dsw %DEST%\examples\RomIO
copy RomIO\test\*.dsp %DEST%\examples\RomIO
copy RomIO\test\*.vbs %DEST%\examples\RomIO
copy RomIO\test\std\*.std %DEST%\examples\RomIO\std

SET DIRS=life mandel mastermind test
copy mpe\contrib\*.dsw %DEST%\examples\MPE
for %%i in (%DIRS%) do (
  echo "Creating examples\mpe\%%i"
  md %DEST%\examples\MPE\%%i
  copy mpe\contrib\%%i\*.* %DEST%\examples\MPE\%%i
)

copy "MPI-2-C++\contrib\examples\*.cc" "%DEST%\examples\C++\examples"
copy "MPI-2-C++\contrib\examples\*." "%DEST%\examples\C++\examples"
copy "MPI-2-C++\contrib\examples\*.dsp" "%DEST%\examples\C++\examples"
copy "MPI-2-C++\contrib\examples\*.dsw" "%DEST%\examples\C++\examples"
copy "MPI-2-C++\contrib\examples\*.cc" "%DEST%\examples\C++\test_suite"
copy "MPI-2-C++\contrib\examples\*.h" "%DEST%\examples\C++\test_suite"
copy "MPI-2-C++\contrib\examples\*.dsw" "%DEST%\examples\C++\test_suite"
copy "MPI-2-C++\contrib\examples\*.dsp" "%DEST%\examples\C++\test_suite"
copy "copy "MPI-2-C++\Enable-cc extentions.reg" %DEST%\bin

echo Copying headers
copy include\*.h %DEST%\include
copy mpe\win_mpe_server\*.h %DEST%\include
copy MPI-2-~1\src\*.h %DEST%\include
copy MPI-2-~1\*.H %DEST%\include
copy mpe\mpe.h %DEST%\include
copy mpe\mpe_log.h %DEST%\include
copy mpe\mpe_win_graphics.h %DEST%\include

echo Creating profiling\lib
copy mpe\profiling\lib\*.* %DEST%\profiling\lib
echo Creating profiling\examples
copy mpe\profiling\examples\*.c %DEST%\profiling\examples
copy mpe\profiling\examples\*.dsp %DEST%\profiling\examples
copy mpe\profiling\examples\*.dsw %DEST%\profiling\examples

copy mpe\slog_api\COPYRIGHT. %DEST%\slog_api
copy mpe\slog_api\Readme. %DEST%\slog_api
copy mpe\slog_api\CFLAGS.txt %DEST%\slog_api
copy mpe\slog_api\slog_api.dsp %DEST%\slog_api
copy mpe\slog_api\bin\Readme %DEST%\slog_api\bin
copy mpe\slog_api\test\Release\readtest.exe %DEST%\slog_api\bin
copy mpe\slog_api\doc\html\*.* %DEST%\slog_api\doc\html
copy mpe\slog_api\src\*.h %DEST%\slog_api\include
copy mpe\slog_api\lib\slog_api.lib %DEST%\slog_api\lib
copy mpe\slog_api\lib\README. %DEST%\slog_api\lib
copy mpe\slog_api\test\readtest.dsp %DEST%\slog_api\test
copy mpe\slog_api\test\readtest.dsw %DEST%\slog_api\test
copy mpe\slog_api\src\slog_readtest.c %DEST%\slog_api\test

copy jumpshot\COPYRIGHT. %DEST%\jumpshot
copy jumpshot\INSTALL. %DEST%\jumpshot
copy jumpshot\README. %DEST%\jumpshot
copy jumpshot\bin\*.bat %DEST%\jumpshot\bin
copy jumpshot\lib\*.* %DEST%\jumpshot\lib
copy jumpshot\lib\data\*.* %DEST%\jumpshot\lib\data
copy jumpshot\lib\images\*.* %DEST%\jumpshot\lib\images
copy jumpshot\lib\logfiles\*.* %DEST%\jumpshot\lib\logfiles
copy jumpshot\swing\*.* %DEST%\jumpshot\swing

copy jumpshot-3\readme.clog %DEST%\jumpshot-3
copy jumpshot-3\readme.slog %DEST%\jumpshot-3
copy jumpshot-3\*.txt %DEST%\jumpshot-3
copy jumpshot-3\bin\*.jar %DEST%\jumpshot-3\bin
copy jumpshot-3\bin\*.bat %DEST%\jumpshot-3\bin
copy jumpshot-3\bin\images\*.* %DEST%\jumpshot-3\bin\images
copy jumpshot-3\lib\*.* %DEST%\jumpshot-3\lib
copy jumpshot-3\swing\*.* %DEST%\jumpshot-3\swing
copy jumpshot-3\doc\*.* %DEST%\jumpshot-3\doc

copy doc\*.pdf %DEST%\doc

copy www\*.* %DEST%\www
copy www\www1\*.* %DEST%\www\www1
copy www\www3\*.* %DEST%\www\www3
copy www\www4\*.* %DEST%\www\www4

SET MPI_ROOT=%DEST%

echo.
echo *** Created MP-MPICH binary distribution in %DEST% ***
echo.
goto END

:HELP
echo.
echo *** USAGE : make_dist folder
echo *** Creates a MP-MPICH binary distribtution for Win32 in folder\NT-MPICH 
echo     Specify full pathname to folder; folder\NT-MPICH will be created
echo.

:END