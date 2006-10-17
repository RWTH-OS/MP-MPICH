# Microsoft Developer Studio Project File - Name="romio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=romio - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "romio.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "romio.mak" CFG="romio - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "romio - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "romio - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "romio - Win32 meta Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "romio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "romio___Win32_Debug"
# PROP BASE Intermediate_Dir "romio___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /G6 /GX /O2 /I ".\include" /I ".\adio\include" /I "..\include" /I "adio\ad_ntfs" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "ROMIO_NTFS" /D "__HAS_MPI_INFO" /D "FORTRANCAPS" /D "__MPICH" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MDd /GX /Zi /Od /I ".\include" /I ".\adio\include" /I "..\include" /I "adio\ad_ntfs" /I "C:\Programme\Microsoft SDK\Include" /D "ROMIO_NTFS" /D "USE_MPI_VERSIONS" /D "MPICH" /D "NDEBUG" /D "_WINDOWS" /D "__HAS_MPI_INFO" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "HAVE_MPI_INFO" /D "HAVE_STATUS_SET_BYTES" /D "HAVE_STRERROR" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /FR /YX /FD /c
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "romio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "romio___Win32_Release"
# PROP BASE Intermediate_Dir "romio___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /G6 /GX /O2 /I ".\include" /I ".\adio\include" /I "..\..\..\include" /I "include" /D "__NTFS" /D "__MPICH" /D "MPICH" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__HAS_MPI_INFO" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "HAVE_MPI_INFO" /D "HAVE_STATUS_SET_BYTES" /D "ROMIO_NTFS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MD /GX /Ox /Ot /Og /Gf /Gy /I ".\include" /I ".\adio\include" /I "..\include" /I "adio\ad_ntfs" /I "C:\Programme\Microsoft SDK\Include" /D "ROMIO_NTFS" /D "USE_MPI_VERSIONS" /D "MPICH" /D "NDEBUG" /D "_WINDOWS" /D "__HAS_MPI_INFO" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "HAVE_MPI_INFO" /D "HAVE_STATUS_SET_BYTES" /D "HAVE_STRERROR" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /Fr /YX /FD /c
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "romio - Win32 meta Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "romio___Win32_meta_Release"
# PROP BASE Intermediate_Dir "romio___Win32_meta_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "romio___Win32_meta_Release"
# PROP Intermediate_Dir "romio___Win32_meta_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /G6 /GX /Ox /Ot /Og /Gf /Gy /I ".\include" /I ".\adio\include" /I "..\include" /I "adio\ad_ntfs" /I "C:\Programme\Microsoft SDK\Include" /D "ROMIO_NTFS" /D "USE_MPI_VERSIONS" /D "MPICH" /D "NDEBUG" /D "_WINDOWS" /D "__HAS_MPI_INFO" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "HAVE_MPI_INFO" /D "HAVE_STATUS_SET_BYTES" /D "HAVE_STRERROR" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /Fr /YX /FD /c
# ADD CPP /nologo /G6 /GX /Ox /Ot /Og /Gf /Gy /I ".\include" /I ".\adio\include" /I "..\include" /I "adio\ad_ntfs" /I "C:\Programme\Microsoft SDK\Include" /D "ROMIO_NTFS" /D "USE_MPI_VERSIONS" /D "MPICH" /D "NDEBUG" /D "_WINDOWS" /D "__HAS_MPI_INFO" /D "FORTRANCAPS" /D "WIN32_FORTRAN_STDCALL" /D "HAVE_MPI_INFO" /D "HAVE_STATUS_SET_BYTES" /D "HAVE_STRERROR" /D "USE_STDARG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "IN_MPICH_DLL" /Fr /YX /FD /c
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "romio - Win32 Debug"
# Name "romio - Win32 Release"
# Name "romio - Win32 meta Release"
# Begin Group "adio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs.h
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_close.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_done.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_fcntl.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_flush.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_hints.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_iread.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_iwrite.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_open.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_rdcoll.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_read.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_resize.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_seek.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_wait.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_wrcoll.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ad_ntfs_write.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\ntfs_error.c

!IF  "$(CFG)" == "romio - Win32 Debug"

!ELSEIF  "$(CFG)" == "romio - Win32 Release"

# ADD CPP /I "C:\Programme\Microsoft SDK\include"

!ELSEIF  "$(CFG)" == "romio - Win32 meta Release"

# ADD BASE CPP /I "C:\Programme\Microsoft SDK\include"
# ADD CPP /I "C:\Programme\Microsoft SDK\include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\Overlapped.cpp
# End Source File
# End Group
# Begin Group "other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\mpi-io\call_errhand.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\close.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\delete.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\file_c2f.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\file_f2c.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\fsync.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_amode.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_atom.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_bytoff.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_errh.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_extent.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_group.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_info.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_posn.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_posn_sh.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_size.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\get_view.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\ioreq_c2f.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\ioreq_f2c.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iotest.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iowait.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iread.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iread_at.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iread_sh.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iwrite.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iwrite_at.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\iwrite_sh.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\mpioimpl.h"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\mpioprof.h"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\open.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\prealloc.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\rd_atallb.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\rd_atalle.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_all.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_allb.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_alle.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_at.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_atall.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_ord.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_ordb.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_orde.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\read_sh.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\seek.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\seek_sh.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\set_atom.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\set_errh.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\set_info.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\set_size.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\set_view.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\wr_atallb.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\wr_atalle.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_all.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_allb.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_alle.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_at.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_atall.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_ord.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_ordb.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_orde.c"
# End Source File
# Begin Source File

SOURCE=".\mpi-io\write_sh.c"
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\adio\common\ad_close.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_delete.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_end.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_flush.c
# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_fstype.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_get_sh_fp.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_hints.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_init.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_open.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_read_coll.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_read_str.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_seek.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_set_sh_fp.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_write_coll.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\ad_write_str.c
# End Source File
# Begin Source File

SOURCE=.\adio\include\adio.h
# End Source File
# Begin Source File

SOURCE=.\adio\include\adio_extern.h
# End Source File
# Begin Source File

SOURCE=.\adio\include\adioi.h
# End Source File
# Begin Source File

SOURCE=.\adio\include\adioi_fs_proto.h
# End Source File
# Begin Source File

SOURCE=.\adio\common\async_list.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\byte_offset.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\eof_offset.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\error.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\flatten.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\get_fp_posn.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\iscontig.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\lock.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\malloc.c
# End Source File
# Begin Source File

SOURCE=.\adio\ad_ntfs\mpio.h
# End Source File
# Begin Source File

SOURCE=.\adio\common\req_malloc.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\shfp_fname.c
# End Source File
# Begin Source File

SOURCE=.\adio\common\status_setb.c
# End Source File
# End Group
# End Target
# End Project
