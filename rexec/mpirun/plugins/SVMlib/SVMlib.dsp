# Microsoft Developer Studio Project File - Name="SVMlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SVMlib - Win32 ch_wsock
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "SVMlib.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "SVMlib.mak" CFG="SVMlib - Win32 ch_wsock"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "SVMlib - Win32 ch_wsock" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SVMlib - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SVMlib - Win32 ch_wsock"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SVMlib__"
# PROP BASE Intermediate_Dir "SVMlib__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ch_wsock"
# PROP Intermediate_Dir "ch_wsock"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MPI" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x11000000" /subsystem:windows /dll /machine:I386
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x10120000" /subsystem:windows /dll /machine:I386 /def:".\SVMlib.def" /out:"..\libs\ch_wsock.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\libs\ch_wsock.dll  ..\..\..\..\bin\Plugins
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SVMlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SVMlib___Win32_Debug"
# PROP BASE Intermediate_Dir "SVMlib___Win32_Debug"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MPI" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\\" /I "..\..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MPI" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x10120000" /subsystem:windows /dll /machine:I386 /def:".\SVMlib.def" /out:"..\libs\ch_wsock.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x10120000" /subsystem:windows /dll /debug /machine:I386 /def:".\SVMlib.def" /out:"..\..\..\..\bin\Plugins\ch_wsock.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ENDIF 

# Begin Target

# Name "SVMlib - Win32 ch_wsock"
# Name "SVMlib - Win32 Debug"
# Begin Source File

SOURCE=..\..\args.c
# End Source File
# Begin Source File

SOURCE=..\..\args.h
# End Source File
# Begin Source File

SOURCE=.\configure.cpp
# End Source File
# Begin Source File

SOURCE=.\configure.h
# End Source File
# Begin Source File

SOURCE=..\..\lists.c
# End Source File
# Begin Source File

SOURCE=..\..\lists.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Skript1.rc
# End Source File
# Begin Source File

SOURCE=.\stardialog.cpp
# End Source File
# Begin Source File

SOURCE=.\startserver.h
# End Source File
# Begin Source File

SOURCE=.\SVMlib.cpp
# End Source File
# Begin Source File

SOURCE=.\SVMlib.def

!IF  "$(CFG)" == "SVMlib - Win32 ch_wsock"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SVMlib - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SVMlib.h
# End Source File
# End Target
# End Project
