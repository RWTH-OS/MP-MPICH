# Microsoft Developer Studio Project File - Name="mpe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mpe - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mpe.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mpe.mak" CFG="mpe - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mpe - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "mpe - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "mpe - Win32 meta Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mpe - Win32 Release"

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
# ADD BASE F90 /compile_only /nologo /warn:nofileopt
# ADD F90 /browser /compile_only /nologo /warn:nofileopt
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /I "slog_api\src" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "C2S_BYTESWAP" /D "MPE_GRAPHICS" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /link50compat

!ELSEIF  "$(CFG)" == "mpe - Win32 Debug"

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
# ADD BASE F90 /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD F90 /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /Gm /GX /ZI /Od /I "..\include" /I "slog_api\src" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "C2S_BYTESWAP" /D "MPE_GRAPHICS" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /link50compat

!ELSEIF  "$(CFG)" == "mpe - Win32 meta Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mpe___Win32_meta_Release"
# PROP BASE Intermediate_Dir "mpe___Win32_meta_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "mpe___Win32_meta_Release"
# PROP Intermediate_Dir "mpe___Win32_meta_Release"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /browser /compile_only /nologo /warn:nofileopt
# ADD F90 /browser /compile_only /nologo /warn:nofileopt
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\include" /I "slog_api\src" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "C2S_BYTESWAP" /D "MPE_GRAPHICS" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /FR /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /I "slog_api\src" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "C2S_BYTESWAP" /D "MPE_GRAPHICS" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /link50compat
# ADD LIB32 /nologo /link50compat

!ENDIF 

# Begin Target

# Name "mpe - Win32 Release"
# Name "mpe - Win32 Debug"
# Name "mpe - Win32 meta Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;f90;for;f;fpp"
# Begin Source File

SOURCE=.\c2s_util.c
# End Source File
# Begin Source File

SOURCE=.\clog.c
# End Source File
# Begin Source File

SOURCE=.\clog2alog.c
# End Source File
# Begin Source File

SOURCE=.\clog_merge.c
# End Source File
# Begin Source File

SOURCE=.\clog_sysio.c
# End Source File
# Begin Source File

SOURCE=.\clog_time.c
# End Source File
# Begin Source File

SOURCE=.\clog_util.c
# End Source File
# Begin Source File

SOURCE=.\dbxerr.win.c
# End Source File
# Begin Source File

SOURCE=.\DECOMP.C
# End Source File
# Begin Source File

SOURCE=.\EXAMINE.C
# ADD CPP /D "BUILD_C"
# End Source File
# Begin Source File

SOURCE=.\GETGRANK.C
# End Source File
# Begin Source File

SOURCE=.\mpe_counter.c
# End Source File
# Begin Source File

SOURCE=.\MPE_IO.C
# End Source File
# Begin Source File

SOURCE=.\mpe_log.c
# End Source File
# Begin Source File

SOURCE=.\MPE_SEQ.C
# End Source File
# Begin Source File

SOURCE=.\mpehname.c
# End Source File
# Begin Source File

SOURCE=.\PRIVTAGS.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\clog.h
# End Source File
# Begin Source File

SOURCE=.\clog2slog.h
# End Source File
# Begin Source File

SOURCE=.\clog_merge.h
# End Source File
# Begin Source File

SOURCE=.\clog_time.h
# End Source File
# Begin Source File

SOURCE=.\MPE.H
# End Source File
# Begin Source File

SOURCE=.\MPE_LOG.H
# End Source File
# Begin Source File

SOURCE=.\mpe_win_graphics.h
# End Source File
# Begin Source File

SOURCE=.\MPECONF.H
# End Source File
# Begin Source File

SOURCE=.\mpeexten.h
# End Source File
# End Group
# End Target
# End Project
