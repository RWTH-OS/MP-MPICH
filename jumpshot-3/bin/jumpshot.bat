@echo off
if "%JAVA_HOME%" == "" goto NO_JAVA

set JC= %JAVA_HOME%\bin\javac
set JAR= %JAVA_HOME%\bin\jar
set GUI_HOME=..
set GUI_LIBDIR=%GUI_HOME%\lib\
set JVM=%JAVA_HOME%\bin\java

set SWING_LIB=%JAVA_HOME%\swing-1.1.1\swingall.jar
if "%SWING_LIB%" == "" set SWING_LIB=..\swing\swing.jar

set CLP=.\;%JAVA_HOME%\lib\classes.zip;%SWING_LIB%;%GUI_LIBDIR%\jumpshot.jar;%GUI_LIBDIR%\slog.jar
set CLP=%CLP%;%GUI_LIBDIR%\preview.jar;%GUI_LIBDIR%\statsviewer.jar;%GUI_LIBDIR%\images.jar

"%JVM%" -ms32m -mx256m -classpath "%CLP%" jumpshot %1 %2 %3 %4

goto FINISH

:NO_JAVA
echo Please set the environment variable JAVA_HOME to the location of your JDK

:FINISH
