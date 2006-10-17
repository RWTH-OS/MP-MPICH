# Microsoft Developer Studio Project File - Name="SCIExtCl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SCIExtCl - Win32 ch_smi
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "SCIExtCl.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "SCIExtCl.mak" CFG="SCIExtCl - Win32 ch_smi"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "SCIExtCl - Win32 ch_smi" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SCIExtCl - Win32 SMI" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SCIExtCl - Win32 ch_smi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SCIExtCl"
# PROP BASE Intermediate_Dir "SCIExtCl"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ch_smi"
# PROP Intermediate_Dir "ch_smi"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\SCIExt" /I "..\\" /I "..\SVMlib" /I "..\..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MPI" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:" 0x10110000" /subsystem:windows /dll /machine:I386 /out:"..\libs/ch_smi.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\libs\ch_smi.dll  ..\..\..\..\bin\Plugins
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SCIExtCl - Win32 SMI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SCIExtC0"
# PROP BASE Intermediate_Dir "SCIExtC0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SMI"
# PROP Intermediate_Dir "SMI"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\SCIExt" /I "..\\" /I "..\SVMlib" /I "..\..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x10130000" /subsystem:windows /dll /machine:I386 /out:"..\libs/SCIExtCl.dll"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\libs\SCIExtCl.dll  ..\..\..\..\bin\Plugins
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SCIExtCl - Win32 ch_smi"
# Name "SCIExtCl - Win32 SMI"
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

SOURCE=..\SCIExt\SCIExt.h
# End Source File
# Begin Source File

SOURCE=.\SCIExtCl.cpp
# End Source File
# Begin Source File

SOURCE=.\SCIExtCl.def
# End Source File
# Begin Source File

SOURCE=.\SCIExtCl.h
# End Source File
# Begin Source File

SOURCE=.\Skript1.rc

!IF  "$(CFG)" == "SCIExtCl - Win32 ch_smi"

# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407 /d "NDEBUG MPI"
# SUBTRACT RSC /d "NDEBUG"

!ELSEIF  "$(CFG)" == "SCIExtCl - Win32 SMI"

# PROP Exclude_From_Build 1
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Skript2.rc

!IF  "$(CFG)" == "SCIExtCl - Win32 ch_smi"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SCIExtCl - Win32 SMI"

# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\SVMlib\stardialog.cpp
# End Source File
# Begin Source File

SOURCE=..\SVMlib\startserver.h
# End Source File
# End Target
# End Project
