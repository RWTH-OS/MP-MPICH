# Microsoft Developer Studio Project File - Name="mpe_client" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mpe_client - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mpe_client.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mpe_client.mak" CFG="mpe_client - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mpe_client - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "mpe_client - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "mpe_client - Win32 meta Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mpe_client - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\\" /I "..\..\include" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mpe_client - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /GX /ZI /Od /I "..\\" /I "..\..\include" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "DEBUG" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mpe_client - Win32 meta Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mpe_client___Win32_meta_Release"
# PROP BASE Intermediate_Dir "mpe_client___Win32_meta_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "mpe_client___Win32_meta_Release"
# PROP Intermediate_Dir "mpe_client___Win32_meta_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\\" /I "..\..\include" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\\" /I "..\..\include" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "mpe_client - Win32 Release"
# Name "mpe_client - Win32 Debug"
# Name "mpe_client - Win32 meta Release"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\mpe_win_client.c
# End Source File
# Begin Source File

SOURCE=..\r_mpe.acf
# End Source File
# Begin Source File

SOURCE=..\r_mpe.idl

!IF  "$(CFG)" == "mpe_client - Win32 Release"

USERDEP__R_MPE="$(inputDir)\r_mpe.acf"	
# Begin Custom Build - Compiling IDL file
InputDir=\home\martin\src\mp-mpich\mpe
InputPath=..\r_mpe.idl

BuildCmds= \
	midl /env win32 /server none /acf $(InputDir)\r_mpe.acf $(InputPath)

"r_mpe_c.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"r_mpe.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mpe_client - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__R_MPE="$(InputDir)\r_mpe.acf"	
# Begin Custom Build - Compiling IDL file
InputDir=\home\martin\src\mp-mpich\mpe
InputPath=..\r_mpe.idl

BuildCmds= \
	midl /env win32 /server none /acf $(InputDir)\r_mpe.acf $(InputPath)

"r_mpe_c.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"r_mpe.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mpe_client - Win32 meta Release"

USERDEP__R_MPE="$(inputDir)\r_mpe.acf"	
# Begin Custom Build - Compiling IDL file
InputDir=\home\martin\src\mp-mpich\mpe
InputPath=..\r_mpe.idl

BuildCmds= \
	midl /env win32 /server none /acf $(InputDir)\r_mpe.acf $(InputPath)

"r_mpe_c.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"r_mpe.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_mpe_c.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\mpe_win_graphics.h
# End Source File
# Begin Source File

SOURCE=.\r_mpe.h
# End Source File
# End Group
# End Target
# End Project
