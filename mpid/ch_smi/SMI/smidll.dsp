# Microsoft Developer Studio Project File - Name="smidll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=smidll - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "smidll.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "smidll.mak" CFG="smidll - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "smidll - Win32 SCI" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "smidll - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "smidll - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smidll - Win32 SCI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SCI"
# PROP BASE Intermediate_Dir "SCI"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMIDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MT /W3 /GX /Z7 /O2 /I "$(DOLPHIN_BASE)\include" /I "../include" /I "../src" /I "../src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "_MBCS" /D "_USRDLL" /D "SMIDLL_EXPORTS" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib sisci_api_md.lib ws2_32.lib libcimt.lib /nologo /dll /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcimtd.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcpmtd.lib" /pdbtype:sept /libpath:"$(DOLPHIN_BASE)/lib"
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "smidll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "smidll___Win32_Debug"
# PROP BASE Intermediate_Dir "smidll___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "../include" /I "../src" /I "../src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "_MBCS" /D "_USRDLL" /D "SMIDLL_EXPORTS" /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Od /I "$(DOLPHIN_BASE)\include" /I "../include" /I "../src" /I "../src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "_MBCS" /D "_USRDLL" /D "SMIDLL_EXPORTS" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib sisci_api.lib ws2_32.lib libcimt.lib /nologo /dll /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcimtd.lib" /pdbtype:sept /libpath:"$(DOLPHIN_BASE)/lib"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib sisci_api_md.lib ws2_32.lib libcimt.lib /nologo /dll /machine:I386 /pdbtype:sept /libpath:"$(DOLPHIN_BASE)/lib"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "smidll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "smidll___Win32_Release"
# PROP BASE Intermediate_Dir "smidll___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Z7 /O2 /I "$(DOLPHIN_BASE)\include" /I "../include" /I "../src" /I "../src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "_MBCS" /D "_USRDLL" /D "SMIDLL_EXPORTS" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /O2 /I "$(DOLPHIN_BASE)\include" /I "../include" /I "../src" /I "../src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "_MBCS" /D "_USRDLL" /D "SMIDLL_EXPORTS" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib sisci_api_md.lib ws2_32.lib libcimt.lib /nologo /dll /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcimtd.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcpmtd.lib" /pdbtype:sept /libpath:"$(DOLPHIN_BASE)/lib"
# SUBTRACT BASE LINK32 /debug /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib sisci_api_md.lib ws2_32.lib libcimt.lib /nologo /dll /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcimtd.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcpmtd.lib" /pdbtype:sept /libpath:"$(DOLPHIN_BASE)/lib"
# SUBTRACT LINK32 /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "smidll - Win32 SCI"
# Name "smidll - Win32 Debug"
# Name "smidll - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\unix_to_nt\dllmain.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\src\utility\cpuid.h
# End Source File
# Begin Source File

SOURCE=.\smidll.def
# End Source File
# End Target
# End Project
