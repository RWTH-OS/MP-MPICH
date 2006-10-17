@echo off
if "%JAVA_HOME%" == "" goto NO_JAVA

set JC= %JAVA_HOME%\bin\javac
set JAR= %JAVA_HOME%\bin\jar
set GUI_HOME=..\..
set GUI_LIBDIR=%GUI_HOME%\lib\
set TARGET_LIB=js_classes.jar

if "%SWING_LIB%" == "" set SWING_LIB=..\..\swing\swing.jar

set CLP=.\;%JAVA_HOME%\lib\classes.zip;%SWING_LIB%;..\js_classes

set SRC=Dlgs.java COLORUTIL.java Structs.java Dlgs.java COLORUTIL.java ApltFileDlg.java  CanOptions.java ClogDisplay.java  ClogReader.java  ColoredRect.java 
set SRC=%SRC% HistCanvas.java HistImage.java Histwin.java JProcess.java  Mainwin.java MyButton.java MyImage.java 
set SRC=%SRC% MyJPanel.java  MyTextField.java PHistCanvas.java PVertScaleCanvas.java PrintCanvas.java 
set SRC=%SRC% PrintDlg.java ProcDlg.java ProcessState.java ProgramCanvas.java ROUTINES.java RecordHandler.java 
set SRC=%SRC%  StateButtons.java SwingWorker.java VertScaleCanvas.java jumpshot.java

echo You will get some errors while compiling the first two files. This is OK.


@echo on
for %%i in (%SRC%) do  %JC% -deprecation -d ..\js_classes -classpath %CLP% %%i%
cd ..\js_classes
%JAR% -cf %GUI_LIBDIR%\%TARGET_LIB% *.class
@echo off

goto FINISH

:NO_JAVA
echo Please set the environment variable JAVA_HOME to the location of your JDK

:FINISH
echo %TARGET_LIB% finished