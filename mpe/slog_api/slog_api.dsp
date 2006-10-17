# Microsoft Developer Studio Project File - Name="slog_api" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=slog_api - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "slog_api.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "slog_api.mak" CFG="slog_api - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "slog_api - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "slog_api - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "slog_api - Win32 meta Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "slog_api - Win32 Release"

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
# ADD CPP /nologo /MD /GX /O2 /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOWHERE" /D "CHECKTIMEORDER" /D "COMPRESSION" /D "HAVE_ALLOCA" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\slog_api.lib" /link50compat

!ELSEIF  "$(CFG)" == "slog_api - Win32 Debug"

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
# ADD CPP /nologo /MDd /GX /ZI /Od /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "NOWHERE" /D "CHECKTIMEORDER" /D "COMPRESSION" /D "HAVE_ALLOCA" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\slog_apid.lib" /link50compat

!ELSEIF  "$(CFG)" == "slog_api - Win32 meta Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "slog_api___Win32_meta_Release"
# PROP BASE Intermediate_Dir "slog_api___Win32_meta_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "slog_api___Win32_meta_Release"
# PROP Intermediate_Dir "slog_api___Win32_meta_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /GX /O2 /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOWHERE" /D "CHECKTIMEORDER" /D "COMPRESSION" /D "HAVE_ALLOCA" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD CPP /nologo /GX /O2 /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOWHERE" /D "CHECKTIMEORDER" /D "COMPRESSION" /D "HAVE_ALLOCA" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\slog_api.lib" /link50compat
# ADD LIB32 /nologo /out:"lib\slog_api.lib" /link50compat

!ENDIF 

# Begin Target

# Name "slog_api - Win32 Release"
# Name "slog_api - Win32 Debug"
# Name "slog_api - Win32 meta Release"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\bswp_fileio.c
# End Source File
# Begin Source File

SOURCE=.\src\fbuf.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_assoc.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_bbuf.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_bebits.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_fileio.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_header.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_impl.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_irec_common.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_irec_read.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_irec_write.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_preview.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_profile.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_pstat.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_recdefs.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_tasklabel.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_ttable.c
# End Source File
# Begin Source File

SOURCE=.\src\slog_vtrarg.c
# End Source File
# Begin Source File

SOURCE=.\src\str_util.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\bswp_fileio.h
# End Source File
# Begin Source File

SOURCE=.\src\fbuf.h
# End Source File
# Begin Source File

SOURCE=.\src\slog.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_assoc.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_bbuf.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_bebits.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_fileio.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_header.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_impl.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_preview.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_profile.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_pstat.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_recdefs.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_tasklabel.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_ttable.h
# End Source File
# Begin Source File

SOURCE=.\src\slog_vtrarg.h
# End Source File
# Begin Source File

SOURCE=.\src\str_util.h
# End Source File
# End Group
# End Target
# End Project
