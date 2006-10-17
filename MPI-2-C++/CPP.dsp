# Microsoft Developer Studio Project File - Name="CPP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CPP - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "CPP.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "CPP.mak" CFG="CPP - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "CPP - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "CPP - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CPP - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\\" /I "..\include" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /YX /FD /TP /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\CPP.lib"

!ELSEIF  "$(CFG)" == "CPP - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\\" /I "..\include" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /YX /FD /TP /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\CPPd.lib"

!ENDIF 

# Begin Target

# Name "CPP - Win32 Release"
# Name "CPP - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\comm.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\datatype.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\errhandler.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\exception.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\functions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\group.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\intercomm.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\intracomm.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\src\mpi++.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\op.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=".\src\pmpi++.cpp"
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\request.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\status.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\src\topology.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\comm.h
# End Source File
# Begin Source File

SOURCE=.\src\comm_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\constants.h
# End Source File
# Begin Source File

SOURCE=.\src\datatype.h
# End Source File
# Begin Source File

SOURCE=.\src\datatype_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\errhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\errhandler_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\exception.h
# End Source File
# Begin Source File

SOURCE=.\src\functions.h
# End Source File
# Begin Source File

SOURCE=.\src\functions_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\group.h
# End Source File
# Begin Source File

SOURCE=.\src\group_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\header.h
# End Source File
# Begin Source File

SOURCE=.\src\intercomm.h
# End Source File
# Begin Source File

SOURCE=.\src\intercomm_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\intracomm.h
# End Source File
# Begin Source File

SOURCE=.\src\intracomm_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\list.h
# End Source File
# Begin Source File

SOURCE=.\src\map.h
# End Source File
# Begin Source File

SOURCE=".\src\mpi++.h"
# End Source File
# Begin Source File

SOURCE=.\src\op.h
# End Source File
# Begin Source File

SOURCE=.\src\op_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\pcomm.h
# End Source File
# Begin Source File

SOURCE=.\src\pdatatype.h
# End Source File
# Begin Source File

SOURCE=.\src\perrhandler.h
# End Source File
# Begin Source File

SOURCE=.\src\pexception.h
# End Source File
# Begin Source File

SOURCE=.\src\pgroup.h
# End Source File
# Begin Source File

SOURCE=.\src\pgroup_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\pintercomm.h
# End Source File
# Begin Source File

SOURCE=.\src\pintracomm.h
# End Source File
# Begin Source File

SOURCE=".\src\pmpi++.h"
# End Source File
# Begin Source File

SOURCE=.\src\pop.h
# End Source File
# Begin Source File

SOURCE=.\src\pop_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\prequest.h
# End Source File
# Begin Source File

SOURCE=.\src\prequest_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\pstatus.h
# End Source File
# Begin Source File

SOURCE=.\src\pstatus_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\ptopology.h
# End Source File
# Begin Source File

SOURCE=.\src\request.h
# End Source File
# Begin Source File

SOURCE=.\src\request_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\status.h
# End Source File
# Begin Source File

SOURCE=.\src\status_inln.h
# End Source File
# Begin Source File

SOURCE=.\src\topology.h
# End Source File
# Begin Source File

SOURCE=.\src\topology_inln.h
# End Source File
# End Group
# End Target
# End Project
