# Microsoft Developer Studio Project File - Name="shmem_device" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=shmem_device - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "shmem_device.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "shmem_device.mak" CFG="shmem_device - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "shmem_device - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "shmem_device - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "shmem_device - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "shmem_device___Win32_Release"
# PROP BASE Intermediate_Dir "shmem_device___Win32_Release"
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
# ADD CPP /nologo /MD /GX /Ox /Ot /Oi /Gf /Gy /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "..\..\src\coll" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "MPID_DEBUG_NONE" /D "_WINDOWS" /D "_MBCS" /D "RNDV_STATIC" /D "SINGLECOPY" /D "HAS_VOLATILE" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /YX /FD /c
# SUBTRACT CPP /Og /Os
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "shmem_device___Win32_Debug"
# PROP BASE Intermediate_Dir "shmem_device___Win32_Debug"
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
# ADD CPP /nologo /MDd /Gm /GX /ZI /Od /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "..\..\src\coll" /I "C:\Programme\Microsoft SDK\Include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "IN_MPICH_DLL" /D "RNDV_STATIC" /D "SINGLECOPY" /D "HAS_VOLATILE" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "shmem_device - Win32 Release"
# Name "shmem_device - Win32 Debug"
# Begin Group "code"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\CH2\adi2cancel.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\adi2hrecv.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\adi2hssend.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2INIT.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2PACK.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\adi2probe.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2RECV.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2REQ.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ch2\adi2rsend.c
# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2SEND.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\adi2ssend.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ch2\adi3mmu.c
# End Source File
# Begin Source File

SOURCE=..\CH2\calltrace.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\canceldevs.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chcancel.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lfbs_common\chnodename.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CH2\CHTICK.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\UTIL\CMNARGS.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lfshmempriv.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\P2P.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\p2pwinprocs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ProcHandles.cpp

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\UTIL\queue.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\UTIL\SBCNST2.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shmemchkdev.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shmemdebug.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shmeminit.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shmemlargeeager.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shmemneager.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shmemnrndv.c
# End Source File
# Begin Source File

SOURCE=.\shmemshort.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lfbs_common\statistics.c

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\UTIL\TR2.C

!IF  "$(CFG)" == "shmem_device - Win32 Release"

!ELSEIF  "$(CFG)" == "shmem_device - Win32 Debug"

# ADD CPP /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "MPID_STATISTICS"
# SUBTRACT CPP /D "_LIB"

!ENDIF 

# End Source File
# End Group
# Begin Group "headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ch2\attach.h
# End Source File
# Begin Source File

SOURCE=..\..\include\attr.h
# End Source File
# Begin Source File

SOURCE="..\..\..\..\..\..\..\Programme\Microsoft Visual Studio\VC98\Include\BASETSD.H"
# End Source File
# Begin Source File

SOURCE=..\ch2\calltrace.h
# End Source File
# Begin Source File

SOURCE=.\chconfig.h
# End Source File
# Begin Source File

SOURCE=..\ch2\chhetero.h
# End Source File
# Begin Source File

SOURCE=..\CH2\chpackflow.h
# End Source File
# Begin Source File

SOURCE=..\util\cmnargs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\coll.h
# End Source File
# Begin Source File

SOURCE=..\ch2\comm.h
# End Source File
# Begin Source File

SOURCE=..\ch2\cookie.h
# End Source File
# Begin Source File

SOURCE=..\ch2\datatype.h
# End Source File
# Begin Source File

SOURCE=..\ch2\dev.h
# End Source File
# Begin Source File

SOURCE=..\ch2\flow.h
# End Source File
# Begin Source File

SOURCE=..\lfbs_common\LogMpid.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpi.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpid.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpid_bind.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpid_debug.h
# End Source File
# Begin Source File

SOURCE=.\mpid_time.h
# End Source File
# Begin Source File

SOURCE=.\mpiddev.h
# End Source File
# Begin Source File

SOURCE=..\..\INCLUDE\MPIDEFS.H
# End Source File
# Begin Source File

SOURCE=..\..\include\mpidmpi.h
# End Source File
# Begin Source File

SOURCE=..\..\src\env\mpierrstrings.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpimem.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpiops.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpiprof.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpipt2pt.h
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\mpitopo.h
# End Source File
# Begin Source File

SOURCE=.\ntshmemdebug.h
# End Source File
# Begin Source File

SOURCE=..\ch2\objtrace.h
# End Source File
# Begin Source File

SOURCE=.\P2P.H
# End Source File
# Begin Source File

SOURCE=.\p2p_common.h
# End Source File
# Begin Source File

SOURCE=.\p2p_locks.h
# End Source File
# Begin Source File

SOURCE=.\p2p_shmalloc.h
# End Source File
# Begin Source File

SOURCE=.\p2p_special.h
# End Source File
# Begin Source File

SOURCE=..\..\include\patchlevel.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ptrcvt.h
# End Source File
# Begin Source File

SOURCE=..\ch2\req.h
# End Source File
# Begin Source File

SOURCE=..\ch2\reqalloc.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sbcnst.h
# End Source File
# Begin Source File

SOURCE=..\util\sbcnst2.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sendq.h
# End Source File
# Begin Source File

SOURCE=.\shdef.h
# End Source File
# Begin Source File

SOURCE=.\shpackets.h
# End Source File
# Begin Source File

SOURCE=..\lfbs_common\statistic.h
# End Source File
# Begin Source File

SOURCE=..\util\tr2.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\ch2\adi2hsend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2mpack.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi3get.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi3put.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi3rhc.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi3win.c
# End Source File
# Begin Source File

SOURCE=..\ch2\sync.c
# End Source File
# End Target
# End Project
