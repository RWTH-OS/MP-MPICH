# Microsoft Developer Studio Project File - Name="smi_meta_device" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=smi_meta_device - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "smi_meta_device.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "smi_meta_device.mak" CFG="smi_meta_device - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "smi_meta_device - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi_meta_device - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi_meta_device - Win32 meta Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smi_meta_device - Win32 Release"

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
# ADD CPP /nologo /MD /GX /Ox /Ot /Og /Gf /Gy /I "$(SMIDIR)\src" /I "$(SMIDIR)\src\unix_to_nt" /I "include" /I "$(SMIDIR)\include" /I "..\..\src\coll" /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "..\..\util" /I "C:\Programme\Microsoft SDK\Include" /D "META" /D "NDEBUG" /D "MPID_DEBUG_NONE" /D "FORTRANCAPS" /D "_WINDOWS" /D "_MBCS" /D "WIN32_FORTRAN_STDCALL" /D "HAS_VOLATILE" /D "MPI_SMI" /D "MPID_NOVERIFY" /D "MPID_STAT_NONE" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "smi_meta_device - Win32 Debug"

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
# ADD CPP /nologo /MDd /GX /Z7 /Od /Gf /Gy /I "$(SMIDIR)\src" /I "$(SMIDIR)\src\unix_to_nt" /I "include" /I "$(SMIDIR)\include" /I "..\..\src\coll" /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "..\..\util" /I "C:\Programme\Microsoft SDK\Include" /D "META" /D "_DEBUG" /D "MPID_DEBUG_ALL" /D "_USRDLL" /D "RNDV_STATIC" /D "SINGLECOPY" /D "MPI_SMI" /D "FORTRANCAPS" /D "_WINDOWS" /D "_MBCS" /D "WIN32_FORTRAN_STDCALL" /D "HAS_VOLATILE" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "smi_meta_device - Win32 meta Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "smi_meta_device___Win32_meta_Release"
# PROP BASE Intermediate_Dir "smi_meta_device___Win32_meta_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "smi_meta_device___Win32_meta_Release"
# PROP Intermediate_Dir "smi_meta_device___Win32_meta_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /MT /GX /Ox /Ot /Og /Gf /Gy /I "$(SMIDIR)\src" /I "$(SMIDIR)\src\unix_to_nt" /I "include" /I "$(SMIDIR)\include" /I "..\..\src\coll" /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "..\..\util" /I "C:\Programme\Microsoft SDK\Include" /D "NDEBUG" /D "MPID_DEBUG_NONE" /D "FORTRANCAPS" /D "_WINDOWS" /D "_MBCS" /D "WIN32_FORTRAN_STDCALL" /D "HAS_VOLATILE" /D "MPI_SMI" /D "MPID_NOVERIFY" /D "MPID_STAT_NONE" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD CPP /nologo /MT /GX /Ox /Ot /Og /Gf /Gy /I "$(SMIDIR)\src" /I "$(SMIDIR)\src\unix_to_nt" /I "include" /I "$(SMIDIR)\include" /I "..\..\src\coll" /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "..\..\util" /I "C:\Programme\Microsoft SDK\Include" /D "META" /D "NDEBUG" /D "MPID_DEBUG_NONE" /D "FORTRANCAPS" /D "_WINDOWS" /D "_MBCS" /D "WIN32_FORTRAN_STDCALL" /D "HAS_VOLATILE" /D "MPI_SMI" /D "MPID_NOVERIFY" /D "MPID_STAT_NONE" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
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

# Name "smi_meta_device - Win32 Release"
# Name "smi_meta_device - Win32 Debug"
# Name "smi_meta_device - Win32 meta Release"
# Begin Group "code"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ch2\adi2cancel.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2debug.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2hrecv.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2hrsend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2hsend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2hssend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2init.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2mpack.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2pack.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2probe.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2recv.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2req.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2rsend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2send.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2ssend.c
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

SOURCE=..\ch2\calltrace.c
# End Source File
# Begin Source File

SOURCE=.\checksum_win32.c

!IF  "$(CFG)" == "smi_meta_device - Win32 Release"

# ADD CPP /FAc

!ELSEIF  "$(CFG)" == "smi_meta_device - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi_meta_device - Win32 meta Release"

# ADD BASE CPP /FAc
# ADD CPP /FAc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\lfbs_common\chnodename.c
# End Source File
# Begin Source File

SOURCE=..\ch2\chtick.c
# End Source File
# Begin Source File

SOURCE=..\ch2\chtime.c
# End Source File
# Begin Source File

SOURCE=..\util\cmnargs.c
# End Source File
# Begin Source File

SOURCE=.\dev_smi.h
# End Source File
# Begin Source File

SOURCE=.\direct_ff.c
# End Source File
# Begin Source File

SOURCE=.\direct_ff.h
# End Source File
# Begin Source File

SOURCE=.\get_contig.c
# End Source File
# Begin Source File

SOURCE=.\getsametype.c
# End Source File
# Begin Source File

SOURCE=.\job.c
# End Source File
# Begin Source File

SOURCE=.\job.h
# End Source File
# Begin Source File

SOURCE=.\mmu.c
# End Source File
# Begin Source File

SOURCE=.\mmu.h
# End Source File
# Begin Source File

SOURCE=.\mmx_memcpy_win.c
# End Source File
# Begin Source File

SOURCE=.\mpidummy.h
# End Source File
# Begin Source File

SOURCE=.\mutex.c
# End Source File
# Begin Source File

SOURCE=.\mutex.h
# End Source File
# Begin Source File

SOURCE=..\ch2\objtrace.c
# End Source File
# Begin Source File

SOURCE=..\ch2\packdtype.c
# End Source File
# Begin Source File

SOURCE=.\put_contig.c
# End Source File
# Begin Source File

SOURCE=.\putsametype.c
# End Source File
# Begin Source File

SOURCE=..\util\queue.c
# End Source File
# Begin Source File

SOURCE=.\remote_handler.c
# End Source File
# Begin Source File

SOURCE=.\remote_handler.h
# End Source File
# Begin Source File

SOURCE=.\rhcv.c
# End Source File
# Begin Source File

SOURCE=..\util\sbcnst2.c
# End Source File
# Begin Source File

SOURCE=.\sci_memcpy.c
# End Source File
# Begin Source File

SOURCE=.\sci_memcpy.h
# End Source File
# Begin Source File

SOURCE=.\sendrecvstubs.c
# End Source File
# Begin Source File

SOURCE=.\sendrecvstubs.h
# End Source File
# Begin Source File

SOURCE=.\smialltoall.c
# End Source File
# Begin Source File

SOURCE=.\smiarndv.c
# End Source File
# Begin Source File

SOURCE=.\smibcast.c
# End Source File
# Begin Source File

SOURCE=.\smibrndv.c
# End Source File
# Begin Source File

SOURCE=.\smicancel.c
# End Source File
# Begin Source File

SOURCE=.\smicheck.c
# End Source File
# Begin Source File

SOURCE=.\smichkdev.c
# End Source File
# Begin Source File

SOURCE=.\smicoll.c
# End Source File
# Begin Source File

SOURCE=.\smideager.c
# End Source File
# Begin Source File

SOURCE=.\smidebug.c
# End Source File
# Begin Source File

SOURCE=.\smidelayedos.c
# End Source File
# Begin Source File

SOURCE=.\smieager.c
# End Source File
# Begin Source File

SOURCE=.\smigather.c
# End Source File
# Begin Source File

SOURCE=.\smiinit.c
# End Source File
# Begin Source File

SOURCE=.\sminbrndv.c
# End Source File
# Begin Source File

SOURCE=.\smipackdtype.c
# End Source File
# Begin Source File

SOURCE=.\smipackdtype.h
# End Source File
# Begin Source File

SOURCE=.\smipersistent.c
# End Source File
# Begin Source File

SOURCE=.\smipriv.c
# End Source File
# Begin Source File

SOURCE=.\smireduce.c
# End Source File
# Begin Source File

SOURCE=.\smiregionmngmt.c
# End Source File
# Begin Source File

SOURCE=.\smiregionmngmt.h
# End Source File
# Begin Source File

SOURCE=.\smirndv.c
# End Source File
# Begin Source File

SOURCE=.\smiscatter.c
# End Source File
# Begin Source File

SOURCE=.\smiseager.c
# End Source File
# Begin Source File

SOURCE=.\smiself.c
# End Source File
# Begin Source File

SOURCE=.\smishort.c
# End Source File
# Begin Source File

SOURCE=.\smistat.c
# End Source File
# Begin Source File

SOURCE=.\smitypes.h
# End Source File
# Begin Source File

SOURCE=.\smiwincreate.c
# End Source File
# Begin Source File

SOURCE=.\smiwinfree.c
# End Source File
# Begin Source File

SOURCE=.\smiwinlock.c
# End Source File
# Begin Source File

SOURCE=.\smiwinunlock.c
# End Source File
# Begin Source File

SOURCE=.\sside_macros.h
# End Source File
# Begin Source File

SOURCE=.\sside_memcpy.c
# End Source File
# Begin Source File

SOURCE=.\sside_memcpy.h
# End Source File
# Begin Source File

SOURCE=.\ssidesetup.c
# End Source File
# Begin Source File

SOURCE=.\ssidesetup.h
# End Source File
# Begin Source File

SOURCE=..\ch2\sync.c
# End Source File
# Begin Source File

SOURCE=..\util\tr2.c
# End Source File
# Begin Source File

SOURCE=..\util\tree.c
# End Source File
# Begin Source File

SOURCE=.\uniqtag.c
# End Source File
# Begin Source File

SOURCE=.\uniqtag.h
# End Source File
# Begin Source File

SOURCE=.\wc_memcpy_win32.c
# End Source File
# Begin Source File

SOURCE=.\winincr.c
# End Source File
# Begin Source File

SOURCE=.\winsync.c
# End Source File
# End Group
# Begin Group "headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ch2\adi2config.h
# End Source File
# Begin Source File

SOURCE=..\ch2\attach.h
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

SOURCE=..\..\include\errhandler.h
# End Source File
# Begin Source File

SOURCE=..\ch2\flow.h
# End Source File
# Begin Source File

SOURCE=.\getus.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpi.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpi_errno.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpi_error.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpicoll.h
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

SOURCE=.\mpid_debug.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpiddev.h
# End Source File
# Begin Source File

SOURCE=.\mpiddev.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpidefs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpidmpi.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpiimpl.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpimem.h
# End Source File
# Begin Source File

SOURCE=.\mpimem.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpiprof.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpipt2pt.h
# End Source File
# Begin Source File

SOURCE=..\ch2\objtrace.h
# End Source File
# Begin Source File

SOURCE=..\..\include\patchlevel.h
# End Source File
# Begin Source File

SOURCE=\\DATA\GLOBAL\software\SMI\src\unix_to_nt\pthread.h
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

SOURCE=\\DATA\GLOBAL\software\SMI\include\smi.h
# End Source File
# Begin Source File

SOURCE=.\smicheck.h
# End Source File
# Begin Source File

SOURCE=.\smicoll.h
# End Source File
# Begin Source File

SOURCE=.\smidebug.h
# End Source File
# Begin Source File

SOURCE=.\smidef.h
# End Source File
# Begin Source File

SOURCE=.\smieager.h
# End Source File
# Begin Source File

SOURCE=.\smipackets.h
# End Source File
# Begin Source File

SOURCE=.\smiqueue.h
# End Source File
# Begin Source File

SOURCE=.\smirndv.h
# End Source File
# Begin Source File

SOURCE=.\smistack.h
# End Source File
# Begin Source File

SOURCE=.\smistat.h
# End Source File
# Begin Source File

SOURCE=.\smisync.h
# End Source File
# Begin Source File

SOURCE=..\util\tr2.h
# End Source File
# Begin Source File

SOURCE=\\DATA\GLOBAL\software\SMI\src\unix_to_nt\unistd.h
# End Source File
# End Group
# End Target
# End Project
