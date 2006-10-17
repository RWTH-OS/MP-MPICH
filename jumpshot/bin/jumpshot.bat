@echo off
set JUMPSHOT_HOME=..

set JAVA_LIBS=%JAVA_HOME%\lib\classes.zip
set SWING_LIBS=%JUMPSHOT_HOME%\swing\swing.jar
set JVM=%JAVA_HOME%\bin\javaw

set GUI_LIBDIR=%JUMPSHOT_HOME%\lib
set MAIN_LIB=%GUI_LIBDIR%\js_classes.jar

set CLASSPT=%JAVA_LIBS%;%SWING_LIBS%;%MAIN_LIB%;.

"%JVM%" -ms32m -mx256m -classpath "%CLASSPT%" jumpshot %1 %2 %3 %4
