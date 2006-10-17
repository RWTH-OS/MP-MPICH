# Microsoft Developer Studio Project File - Name="wsock2_device" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wsock2_device - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "wsock2_device.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "wsock2_device.mak" CFG="wsock2_device - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "wsock2_device - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "wsock2_device - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wsock2_device - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wsock2_device___Win32_Release"
# PROP BASE Intermediate_Dir "wsock2_device___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /GX /Ox /Ot /Oi /Gf /Gy /I ".\\" /I "..\ntshmem" /I "..\lfbs_common" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "C:\Programme\Microsoft SDK\Include" /D "CH_WSOCK_PRESENT" /D "NDEBUG" /D "MPID_DEBUG_NONE" /D "FORTRANCAPS" /D "_WINDOWS" /D "_MBCS" /D "RNDV_STATIC" /D "SINGLECOPY" /D "WIN32_FORTRAN_STDCALL" /D "HAS_VOLATILE" /D "WSOCK2" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "wsock2_device - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wsock2_device___Win32_Debug"
# PROP BASE Intermediate_Dir "wsock2_device___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /GX /Z7 /Od /Gf /Gy /I ".\\" /I "..\lfbs_common" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "C:\Programme\Microsoft SDK\Include" /D "CH_WSOCK_PRESENT" /D "_DEBUG" /D "MPID_DEBUG_ALL" /D "FORTRANCAPS" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WIN32_FORTRAN_STDCALL" /D "HAS_VOLATILE" /D "RNDV_STATIC" /D "SINGLECOPY" /D "MPID_STATISTICS" /D "WSOCK2" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
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

# Name "wsock2_device - Win32 Release"
# Name "wsock2_device - Win32 Debug"
# Begin Group "code"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ch2\adi2cancel.c
# End Source File
# Begin Source File

SOURCE=..\CH2\adi2hrecv.c
# End Source File
# Begin Source File

SOURCE=..\CH2\adi2hsend.c
# End Source File
# Begin Source File

SOURCE=..\CH2\adi2hssend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2init.c
# End Source File
# Begin Source File

SOURCE=..\CH2\adi2mpack.c
# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2PACK.C
# End Source File
# Begin Source File

SOURCE=..\CH2\adi2probe.c
# End Source File
# Begin Source File

SOURCE=.\adi2recv.c
# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2REQ.C
# End Source File
# Begin Source File

SOURCE=..\CH2\ADI2SEND.C
# End Source File
# Begin Source File

SOURCE=..\CH2\adi2ssend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi3get.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi3mmu.c
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

SOURCE=..\CH2\calltrace.c
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\canceldevs.c
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\chcancel.c
# End Source File
# Begin Source File

SOURCE=.\chdebug.c
# End Source File
# Begin Source File

SOURCE=..\lfbs_common\chnodename.c
# End Source File
# Begin Source File

SOURCE=..\CH2\CHTICK.C
# End Source File
# Begin Source File

SOURCE=..\ch2\chtime.c
# End Source File
# Begin Source File

SOURCE=..\UTIL\CMNARGS.C
# End Source File
# Begin Source File

SOURCE=.\getopt.cpp
# End Source File
# Begin Source File

SOURCE=.\inSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\lfshmempriv.c
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\P2P.C
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\p2pwinprocs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\ProcHandles.cpp
# End Source File
# Begin Source File

SOURCE=..\UTIL\queue.c
# End Source File
# Begin Source File

SOURCE=..\UTIL\SBCNST2.C
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\shmemchkdev.c
# End Source File
# Begin Source File

SOURCE=.\shmeminit.c
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\shmemlargeeager.c
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\shmemneager.c
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\shmemnrndv.c
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\shmemshort.c
# End Source File
# Begin Source File

SOURCE=..\lfbs_common\statistics.c
# End Source File
# Begin Source File

SOURCE=..\ch2\sync.c
# End Source File
# Begin Source File

SOURCE=.\TCPCommunicator.cpp
# End Source File
# Begin Source File

SOURCE=..\UTIL\TR2.C
# End Source File
# Begin Source File

SOURCE=.\wsock_coll.c
# End Source File
# Begin Source File

SOURCE=.\wsockbeager.c
# End Source File
# Begin Source File

SOURCE=.\wsockbrndv.c
# End Source File
# Begin Source File

SOURCE=.\wsockcancel.c
# End Source File
# Begin Source File

SOURCE=.\wsockchkdev.c
# End Source File
# Begin Source File

SOURCE=.\wsockinit.c
# End Source File
# Begin Source File

SOURCE=.\wsockneager.c
# End Source File
# Begin Source File

SOURCE=.\wsockpriv.cpp
# End Source File
# Begin Source File

SOURCE=.\wsockshort.c
# End Source File
# End Group
# Begin Group "headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ch2\adi2config.h
# End Source File
# Begin Source File

SOURCE=.\channel.h
# End Source File
# Begin Source File

SOURCE=.\chconfig.h
# End Source File
# Begin Source File

SOURCE=.\chdef.h
# End Source File
# Begin Source File

SOURCE=..\ch2\chhetero.h
# End Source File
# Begin Source File

SOURCE=..\ch2\dev.h
# End Source File
# Begin Source File

SOURCE=..\ch2\flow.h
# End Source File
# Begin Source File

SOURCE=.\getopt.h
# End Source File
# Begin Source File

SOURCE=.\inSocket.h
# End Source File
# Begin Source File

SOURCE=..\lfbs_common\LogMpid.h
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

SOURCE=..\ch2\mpid_time.h
# End Source File
# Begin Source File

SOURCE=.\mpid_time.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpiddev.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\mpiddev.h
# End Source File
# Begin Source File

SOURCE=.\mpiddev.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpidmpi.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpipt2pt.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\ntshmemdebug.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\P2P.H
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\p2p_common.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\p2p_locks.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\p2p_shmalloc.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\p2p_special.h
# End Source File
# Begin Source File

SOURCE=.\packets.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\shdef.h
# End Source File
# Begin Source File

SOURCE=..\ch_ntshmem\shpackets.h
# End Source File
# Begin Source File

SOURCE=.\SocketException.h
# End Source File
# Begin Source File

SOURCE=..\lfbs_common\statistic.h
# End Source File
# Begin Source File

SOURCE=.\TCPCommunicator.h
# End Source File
# Begin Source File

SOURCE=.\wsock2debug.h
# End Source File
# Begin Source File

SOURCE=.\wsockpriv.h
# End Source File
# End Group
# End Target
# End Project
