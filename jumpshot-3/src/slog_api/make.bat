@echo off
if "%JAVA_HOME%" == "" goto NO_JAVA

set JC= %JAVA_HOME%\bin\javac
set JAR= %JAVA_HOME%\bin\jar
set GUI_HOME=..\..
set GUI_LIBDIR=%GUI_HOME%\lib\
set TARGET_LIB=slog.jar
set SLOG_LIB=%GUI_HOME%\lib\slog.jar
set PVIEW_LIB=%GUI_HOME%\lib\preview.jar
set STATS_LIB=%GUI_HOME%\lib\statsviewer.jar
if "%SWING_LIB%" == "" set SWING_LIB=..\..\swing\swing.jar

set CLP=.\;%JAVA_HOME%\lib\classes.zip;%SWING_LIB%;%SLOG_LIB%;%PVIEW_LIB%;%STATS_LIB%

@echo on
for %%i in (iarray.java SLOG_*.java NegativeIntegerException.java) do  %JC% -deprecation -classpath %CLP% %%i%
%JAR% -cf %TARGET_LIB% *.class
copy %TARGET_LIB%  %GUI_LIBDIR%
@echo off

goto FINISH

:NO_JAVA
echo Please set the environment variable JAVA_HOME to the location of your JDK

:FINISH
echo %TARGET_LIB% finished