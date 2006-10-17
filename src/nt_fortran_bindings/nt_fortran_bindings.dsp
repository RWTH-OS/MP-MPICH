# Microsoft Developer Studio Project File - Name="nt_fortran_bindings" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nt_fortran_bindings - Win32 G77 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "nt_fortran_bindings.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "nt_fortran_bindings.mak" CFG="nt_fortran_bindings - Win32 G77 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "nt_fortran_bindings - Win32 Visual Fortran debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Visual Fortran Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Absoft Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Absoft Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Intel Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Intel Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 FortranPlus Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 FortranPlus Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Lahey Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Lahey Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Intel Profiling" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Absoft Profiling" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Visual Fortran Profiling" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Lahey Profiling" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Salford Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Salford Profiling" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 Salford Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 G77 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 G77 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "nt_fortran_bindings - Win32 G77 Profiling" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Visual Fortran debug"
# PROP BASE Intermediate_Dir "Visual Fortran debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "VisualFortran_debug"
# PROP Intermediate_Dir "VisualFortran_debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "VISUAL_FORTRAN" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /FR /YX /Zl /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_vf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Visual Fortran Release"
# PROP BASE Intermediate_Dir "Visual Fortran Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "VisualFortran_Release"
# PROP Intermediate_Dir "VisualFortran_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "VISUAL_FORTRAN" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_vf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Absoft Debug"
# PROP BASE Intermediate_Dir "Absoft Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Absoft_Debug"
# PROP Intermediate_Dir "Absoft_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "ABSOFT" /D "FORTRANNOUNDERSCORE" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_apf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Absoft Release"
# PROP BASE Intermediate_Dir "Absoft Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Absoft_Release"
# PROP Intermediate_Dir "Absoft_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "ABSOFT" /D "FORTRANNOUNDERSCORE" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_apf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Intel Debug"
# PROP BASE Intermediate_Dir "Intel Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Intel_Debug"
# PROP Intermediate_Dir "Intel_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "FORTRANCAPS" /D "INTEL" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /FR /YX /Zl /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Intel Release"
# PROP BASE Intermediate_Dir "Intel Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Intel_Release"
# PROP Intermediate_Dir "Intel_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANCAPS" /D "INTEL" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "FortranPlus Debug"
# PROP BASE Intermediate_Dir "FortranPlus Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "FortranPlus_Debug"
# PROP Intermediate_Dir "FortranPlus_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "FORTRANUNDERSCORE" /D "WIN32_FORTRAN_STDCALL" /D "NAS_FORTRAN" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpich_nfp.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "FortranPlus Release"
# PROP BASE Intermediate_Dir "FortranPlus Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "FortranPlus_Release"
# PROP Intermediate_Dir "FortranPlus_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANUNDERSCORE" /D "WIN32_FORTRAN_STDCALL" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\mpich_nfp.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Lahey_Release"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Lahey_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Lahey_Release"
# PROP Intermediate_Dir "Lahey_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "INTEL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANUNDERSCORE" /D "LAHEY" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_lf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Lahey_Debug"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Lahey_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Lahey_Debug"
# PROP Intermediate_Dir "Lahey_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "INTEL" /FR /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "FORTRANUNDERSCORE" /D "LAHEY" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /FR /YX /Zl /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_lf.lib" /verbose /nodefaultlib

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Profiling"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Intel_Profiling"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Intel_Profiling"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Intel_Profiling"
# PROP Intermediate_Dir "Intel_Profiling"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "INTEL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANCAPS" /D "INTEL" /D "MPI_BUILD_PROFILING" /D "MPIO_BUILD_PROFILING" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pmpichf_int.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Profiling"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Absoft_Profiling"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Absoft_Profiling"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Absoft_Profiling"
# PROP Intermediate_Dir "Absoft_Profiling"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "ABSOFT" /D "FORTRANNOUNDERSCORE" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "ABSOFT" /D "FORTRANNOUNDERSCORE" /D "MPI_BUILD_PROFILING" /D "MPIO_BUILD_PROFILING" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_apf.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pmpichf_apf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Profiling"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Visual_Fortran_Profiling"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Visual_Fortran_Profiling"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Visual_Fortran_Profiling"
# PROP Intermediate_Dir "Visual_Fortran_Profiling"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "VISUAL_FORTRAN" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\romio\mpi-io" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "VISUAL_FORTRAN" /D "MPI_BUILD_PROFILING" /D "MPIO_BUILD_PROFILING" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_vf.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pmpichf_vf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Profiling"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Lahey_Profiling"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Lahey_Profiling"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Lahey_Profiling"
# PROP Intermediate_Dir "Lahey_Profiling"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORANUNDERSCORE" /D "LAHEY" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANUNDERSCORE" /D "LAHEY" /D "MPI_BUILD_PROFILING" /D "MPIO_BUILD_PROFILING" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_lf.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pmpichf_lf.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Salford_Debug"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Salford_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Salford_Debug"
# PROP Intermediate_Dir "Salford_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "INTEL" /FR /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "FORTRANCAPS" /D "SALFORD" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /FR /YX /Zl /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_sal.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Profiling"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Salford_Profiling"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Salford_Profiling"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Salford_Profiling"
# PROP Intermediate_Dir "Salford_Profiling"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "INTEL" /D "MPI_BUILD_PROFILING" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "MPI_BUILD_PROFILING" /D "FORTRANCAPS" /D "SALFORD" /D "MPIO_BUILD_PROFILING" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\pmpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pmpichf_sal.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_Salford_Release"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_Salford_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Salford_Release"
# PROP Intermediate_Dir "Salford_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\mpid\ch2" /I "..\..\romio\adio\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FORTRANCAPS" /D "INTEL" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANCAPS" /D "SALFORD" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_sal.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_G77_Release"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_G77_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "G77_Release"
# PROP Intermediate_Dir "G77_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\include" /I "..\..\mpid\ch2" /D "NDEBUG" /D "FORTRANCAPS" /D "INTEL" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "FORTRANDOUBLEUNDERSCORE" /D "G77" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /FR /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_gnu.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_G77_Debug"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_G77_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "G77_Debug"
# PROP Intermediate_Dir "G77_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\romio\adio\include" /I "..\..\include" /I "..\..\mpid\ch2" /D "_DEBUG" /D "FORTRANCAPS" /D "INTEL" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /FR /YX /Zl /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "FORTRANDOUBLEUNDERSCORE" /D "G77" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /FR /YX /Zl /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\mpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\mpichf_gnu.lib"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Profiling"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nt_fortran_bindings___Win32_G77_Profiling"
# PROP BASE Intermediate_Dir "nt_fortran_bindings___Win32_G77_Profiling"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "G77_Profiling"
# PROP Intermediate_Dir "G77_Profiling"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\include" /I "..\..\mpid\ch2" /D "NDEBUG" /D "FORTRANCAPS" /D "INTEL" /D "MPI_BUILD_PROFILING" /D "MPIO_BUILD_PROFILING" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\romio\adio\include" /I "..\..\mpid\lfbs_common" /I "..\..\include" /I "..\..\mpid\ch2" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "MPI_BUILD_PROFILING" /D "MPIO_BUILD_PROFILING" /D "FORTRANDOUBLEUNDERSCORE" /D "G77" /D "_MBCS" /D "_LIB" /D "WIN32" /D "USE_STDARG" /D "MP_MPICH" /D "FORTRAN_STATIC_LIB" /D "WIN32_LEAN_AND_MEAN" /D "MPID_NO_IMPORT" /YX /Zl /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\pmpichf_int.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pmpichf_gnu.lib"

!ENDIF 

# Begin Target

# Name "nt_fortran_bindings - Win32 Visual Fortran debug"
# Name "nt_fortran_bindings - Win32 Visual Fortran Release"
# Name "nt_fortran_bindings - Win32 Absoft Debug"
# Name "nt_fortran_bindings - Win32 Absoft Release"
# Name "nt_fortran_bindings - Win32 Intel Debug"
# Name "nt_fortran_bindings - Win32 Intel Release"
# Name "nt_fortran_bindings - Win32 FortranPlus Debug"
# Name "nt_fortran_bindings - Win32 FortranPlus Release"
# Name "nt_fortran_bindings - Win32 Lahey Release"
# Name "nt_fortran_bindings - Win32 Lahey Debug"
# Name "nt_fortran_bindings - Win32 Intel Profiling"
# Name "nt_fortran_bindings - Win32 Absoft Profiling"
# Name "nt_fortran_bindings - Win32 Visual Fortran Profiling"
# Name "nt_fortran_bindings - Win32 Lahey Profiling"
# Name "nt_fortran_bindings - Win32 Salford Debug"
# Name "nt_fortran_bindings - Win32 Salford Profiling"
# Name "nt_fortran_bindings - Win32 Salford Release"
# Name "nt_fortran_bindings - Win32 G77 Release"
# Name "nt_fortran_bindings - Win32 G77 Debug"
# Name "nt_fortran_bindings - Win32 G77 Profiling"
# Begin Group "MPI bindings"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\SRC\ENV\abortf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\addressf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\allgatherf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\allgathervf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\allreducef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\alltoallf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\alltoallvf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\attr_delvalf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\attr_getvalf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\attr_putvalf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\barrierf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\bcastf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\bsend_initf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\bsendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\bufattachf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\buffreef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\cancelf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cart_coordsf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cart_createf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cart_getf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cart_mapf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cart_rankf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cart_shiftf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cart_subf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\cartdim_getf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_createf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_dupf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_freef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_groupf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_namegetf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_nameputf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_rankf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_rgroupf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_rsizef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_sizef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_splitf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\comm_testicf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\commcomparef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\commreqfreef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\create_recvf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\create_sendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\darrayf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\dims_createf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\dup_fnf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\errclassf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\errcreatef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\errfreef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\errgetf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\errorstringf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\errsetf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\finalizedf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\finalizef.c
# End Source File
# Begin Source File

SOURCE=..\env\fstrutils.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\gatherf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\gathervf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\getcountf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\getelementsf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\getpnamef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\getversionf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\graph_getf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\graph_mapf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\graph_nbrf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\graphcreatef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\graphdimsgtf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\graphnbrcntf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_difff.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_exclf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_freef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_inclf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_interf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_rankf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_rexclf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_rinclf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_sizef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\group_unionf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\groupcomparf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\grouptranksf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\ibsendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\ic_createf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\ic_mergef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_createf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_deletef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_dupf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_freef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_getf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_getnksf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_getnthf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_getvlnf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\info_setf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\initf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\initializef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\iprobef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\irecvf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\irsendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\isendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\issendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\keyval_freef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\keyvalcreatf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\null_copyfnf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\CONTEXT\null_del_fnf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\opcreatef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\opfreef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\pack_sizef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\packf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PROFILE\pcontrolf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\probef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\recvf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\red_scatf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\reducef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\rsend_initf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\rsendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\scanf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\scatterf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\COLL\scattervf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\sendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\sendrecvf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\sendrecvrepf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\ssend_initf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\ssendf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\startallf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\startf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\EXTERNAL\statuscancelf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\EXTERNAL\statuselmf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\subarrayf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\testallf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\testanyf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\testcancelf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\testf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\testsomef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\TOPOL\topo_testf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\MISC2\type_blkindf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_commitf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_contigf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_extentf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_freef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\EXTERNAL\type_get_envf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\EXTERNAL\type_getcontf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_hindf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_hvecf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_indf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_lbf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_sizef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_structf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_ubf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\type_vecf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\unpackf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\waitallf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\waitanyf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\waitf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\PT2PT\waitsomef.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\wtickf.c
# End Source File
# Begin Source File

SOURCE=..\..\SRC\ENV\wtimef.c
# End Source File
# End Group
# Begin Group "Romio bindings"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\closef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\deletef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\fsyncf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_amodef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_atomf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_bytofff.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_errhf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_extentf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_groupf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_infof.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_posn_shf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_posnf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_sizef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\get_viewf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\iotestf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\iowaitf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\iread_atf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\iread_shf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\ireadf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\iwrite_atf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\iwrite_shf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\iwritef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\openf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\preallocf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\rd_atallbf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\rd_atallef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_allbf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_allef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_allf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_atallf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_atf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_ordbf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_ordef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_ordf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\read_shf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\readf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\seek_shf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\seekf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\set_atomf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\set_errhf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\set_infof.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\set_sizef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\set_viewf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\wr_atallbf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\wr_atallef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_allbf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_allef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_allf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_atallf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_atf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_ordbf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_ordef.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_ordf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\write_shf.c"
# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\writef.c"
# End Source File
# End Group
# Begin Group "MPE bindings"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=..\..\mpe\dbxerrf.c
# End Source File
# Begin Source File

SOURCE=..\..\mpe\decompf.c
# End Source File
# Begin Source File

SOURCE=..\..\mpe\examine.c
# End Source File
# Begin Source File

SOURCE=..\..\mpe\getgrankf.c
# End Source File
# Begin Source File

SOURCE=..\..\mpe\mpe_counterf.c
# End Source File
# Begin Source File

SOURCE=..\..\mpe\mpe_graphicsf.c
# End Source File
# Begin Source File

SOURCE=..\..\mpe\mpe_logf.c
# End Source File
# Begin Source File

SOURCE=..\..\mpe\mpe_seqf.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\mpid\lfbs_common\chnodename.c
# End Source File
# Begin Source File

SOURCE=.\vf_2char_bindings.c

!IF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran debug"

# ADD CPP /Gd

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Release"

# ADD CPP /Gd

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Profiling"

# ADD CPP /Gz

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vf_char_bindings.c

!IF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran debug"

# ADD CPP /Gd

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Release"

# ADD CPP /Gd

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Profiling"

# ADD CPP /Gz

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\romio\mpi-io\fortran\vf_io_char_bindings.c"

!IF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran debug"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Release"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 FortranPlus Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Intel Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Absoft Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Visual Fortran Profiling"

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Lahey Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Profiling"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 Salford Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "nt_fortran_bindings - Win32 G77 Profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Target
# End Project
