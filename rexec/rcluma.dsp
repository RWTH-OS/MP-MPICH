# Microsoft Developer Studio Project File - Name="rcluma" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=rcluma - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "rcluma.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "rcluma.mak" CFG="rcluma - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "rcluma - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "rcluma - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rcluma - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=fl32.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gz /MT /W3 /GX /Zd /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 rpcrt4.lib kernel32.lib user32.lib advapi32.lib netapi32.lib ws2_32.lib Iphlpapi.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\bin\rclumad.exe"

!ELSEIF  "$(CFG)" == "rcluma - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
F90=fl32.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gz /MTd /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcimt.lib rpcrt4.lib kernel32.lib user32.lib advapi32.lib netapi32.lib ws2_32.lib Iphlpapi.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libcimtd.lib" /out:"Debug/rclumad.exe" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "rcluma - Win32 Release"
# Name "rcluma - Win32 Debug"
# Begin Source File

SOURCE=.\access.c
# End Source File
# Begin Source File

SOURCE=.\cluma.acf
# End Source File
# Begin Source File

SOURCE=.\cluma.h
# End Source File
# Begin Source File

SOURCE=.\cluma.idl

!IF  "$(CFG)" == "rcluma - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__CLUMA="cluma.acf"	
# Begin Custom Build - Creating server stub
InputPath=.\cluma.idl

BuildCmds= \
	midl /nologo /client none /env win32 $(InputPath)

"cluma_s.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"cluma.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "rcluma - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__CLUMA="cluma.acf"	
# Begin Custom Build - Creating server stub
InputPath=.\cluma.idl

BuildCmds= \
	midl /client none /env win32 $(InputPath)

"cluma_s.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"cluma.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cluma_s.c
# ADD CPP /D _WIN32_WINNT=0x400
# End Source File
# Begin Source File

SOURCE=.\Environment.h
# End Source File
# Begin Source File

SOURCE=.\EnvLoadSys.cpp
# End Source File
# Begin Source File

SOURCE=.\helpers.cpp
# End Source File
# Begin Source File

SOURCE=.\helpers.h
# End Source File
# Begin Source File

SOURCE=.\inSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\inSocket.h
# End Source File
# Begin Source File

SOURCE=.\messages.h
# End Source File
# Begin Source File

SOURCE=.\messages.mc

!IF  "$(CFG)" == "rcluma - Win32 Release"

# Begin Custom Build - Creating message map
InputPath=.\messages.mc

"messages.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mc $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "rcluma - Win32 Debug"

# Begin Custom Build - Creating message map
InputPath=.\messages.mc

BuildCmds= \
	mc $(InputPath)

"messages.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MSG00001.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\messages.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\MSG00001.bin
# End Source File
# Begin Source File

SOURCE=.\process.cpp
# End Source File
# Begin Source File

SOURCE=.\Script1.rc
# End Source File
# Begin Source File

SOURCE=.\SERVER.C
# End Source File
# Begin Source File

SOURCE=.\SERVICE.C
# End Source File
# Begin Source File

SOURCE=.\SERVICE.H
# End Source File
# Begin Source File

SOURCE=.\SocketException.h
# End Source File
# Begin Source File

SOURCE=.\state.cpp
# End Source File
# Begin Source File

SOURCE=.\Update.cpp
# End Source File
# Begin Source File

SOURCE=.\Update.h
# End Source File
# Begin Source File

SOURCE=.\Userenv.cpp
# End Source File
# Begin Source File

SOURCE=.\Userenv.h
# End Source File
# End Target
# End Project
