@echo off

if "%1" == "clean" goto CLEAN

if "%JAVA_HOME%" == "" goto NO_JAVA

echo Building slog_api
cd slog_api
call make.bat

echo Building StatsViewer
cd ..\StatsViewer
call make.bat

echo Building GraphicPreview
cd ..\GraphicPreview
call make.bat

echo Building Jumpshot
cd ..\main
call make.bat
cd ..

goto FINISH

:CLEAN

cd slog_api
del *.class *.jar

cd ..\StatsViewer
del *.class *.jar

cd ..\GraphicPreview
del *.class *.jar

cd ..\main
del *.class *.jar
cd ..

goto FINISH

:NO_JAVA
echo Please set the environment variable JAVA_HOME to the location of your JDK

:FINISH
echo Build of jumpshot finished