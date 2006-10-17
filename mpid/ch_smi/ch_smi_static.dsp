# Microsoft Developer Studio Project File - Name="ch_smi_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ch_smi_static - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "ch_smi_static.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "ch_smi_static.mak" CFG="ch_smi_static - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "ch_smi_static - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "ch_smi_static - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ch_smi_static - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ch_smi_static___Win32_Release"
# PROP BASE Intermediate_Dir "ch_smi_static___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ch_smi_static___Win32_Release"
# PROP Intermediate_Dir "ch_smi_static___Win32_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "$(SMIDIR)\include" /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "C:\Programme\Microsoft SDK\Include" /I "..\..\src\coll" /I "..\..\unix2nt" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "MPID_DEBUG_NONE" /D "IN_MPICH_DLL" /D "_WINDOWS" /D "HAS_VOLATILE" /D "RNDV_STATIC" /D "SINGLECOPY" /D "USE_STDARG" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ch_smi_static - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ch_smi_static___Win32_Debug"
# PROP BASE Intermediate_Dir "ch_smi_static___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ch_smi_static___Win32_Debug"
# PROP Intermediate_Dir "ch_smi_static___Win32_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(DOLPHIN_BASE)\include" /I "$(SMIDIR)\include" /I "..\lfbs_common" /I ".\\" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "C:\Programme\Microsoft SDK\Include" /I "..\..\src\coll" /I "..\..\unix2nt" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_DEBUG" /D "MPID_DEBUG_ALL" /D "MPID_STATISTICS" /D "_WINDOWS" /D "HAS_VOLATILE" /D "RNDV_STATIC" /D "SINGLECOPY" /D "USE_STDARG" /D "WIN32_LEAN_AND_MEAN" /YX /FD /GZ /c
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

# Name "ch_smi_static - Win32 Release"
# Name "ch_smi_static - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\env\abort.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\accumulate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\address.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2config.h
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\allgather.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\allgatherv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\allocmem.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\allreduce.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\alltoall.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\alltoallv.c
# End Source File
# Begin Source File

SOURCE=..\ch2\attach.h
# End Source File
# Begin Source File

SOURCE=..\..\include\attr.h
# End Source File
# Begin Source File

SOURCE=..\..\src\context\attr_delval.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\attr_getval.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\attr_putval.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\attr_util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\barrier.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\bcast.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\bsend.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\bsend_init.c
# End Source File
# Begin Source File

SOURCE=..\..\src\util\bsendutil2.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\bufattach.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\buffree.c
# End Source File
# Begin Source File

SOURCE=..\ch2\calltrace.h
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\cancel.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cart_coords.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cart_create.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cart_get.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cart_map.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cart_rank.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cart_shift.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cart_sub.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\cartdim_get.c
# End Source File
# Begin Source File

SOURCE=..\ch2\chhetero.h
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\coll.h
# End Source File
# Begin Source File

SOURCE=..\ch2\comm.h
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_create.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_dup.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_free.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_group.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_name_get.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_name_put.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_rank.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_rgroup.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_rsize.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_size.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_split.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_testic.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\comm_util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\commcompare.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\commreq_free.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\context_util.c
# End Source File
# Begin Source File

SOURCE=..\ch2\cookie.h
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\create_recv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\create_send.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\darray.c
# End Source File
# Begin Source File

SOURCE=..\util\dataqueue.c
# End Source File
# Begin Source File

SOURCE=..\util\dataqueue.h
# End Source File
# Begin Source File

SOURCE=..\ch2\datatype.h
# End Source File
# Begin Source File

SOURCE=..\..\src\env\debugutil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\dims_create.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dmpi\dmpipk.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\dup_fn.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\errclass.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\errcreate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\errfree.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\errget.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\errhandler.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\errorstring.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\errset.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\fence.c
# End Source File
# Begin Source File

SOURCE=..\util\fifo.c
# End Source File
# Begin Source File

SOURCE=..\util\fifo.h
# End Source File
# Begin Source File

SOURCE=..\..\src\env\finalize.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\finalized.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\freemem.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\gather.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\gatherv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\get.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\getattr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\getcount.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\getelements.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\getgrp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\getpname.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\getversion.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\global_ops.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\graph_get.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\graph_map.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\graph_nbr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\graphcreate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\graphdimsget.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\graphnbrcnt.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_diff.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_excl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_free.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_incl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_inter.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_rank.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_rexcl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_rincl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_size.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_tranks.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_union.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\group_util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\groupcompare.c
# End Source File
# Begin Source File

SOURCE=..\util\hash.c
# End Source File
# Begin Source File

SOURCE=..\util\hash.h
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\ibsend.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\ic.h
# End Source File
# Begin Source File

SOURCE=..\..\src\context\ic_create.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\ic_merge.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_c2f.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_create.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_delete.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_dup.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_f2c.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_free.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_get.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_getnks.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_getnth.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_getvln.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\info_set.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\init.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\initdte.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\initialize.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\initthread.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\initutil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\inter_fns.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\intra_fns.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\intra_scan.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\iprobe.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\irecv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\irsend.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\isend.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\issend.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\keyval_free.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\keyvalcreate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\mmx_ops.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\mperror.c
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

SOURCE=..\ch2\mpid_time.h
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

SOURCE=..\..\include\mpifort.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mpiimpl.h
# End Source File
# Begin Source File

SOURCE=.\mpimem.h
# End Source File
# Begin Source File

SOURCE=..\..\include\MPIO.H
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

SOURCE=..\..\src\util\mpirutil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\mpitopo.h
# End Source File
# Begin Source File

SOURCE=..\..\src\env\msgqdllloc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\nerrmsg.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\null_copyfn.c
# End Source File
# Begin Source File

SOURCE=..\..\src\context\null_del_fn.c
# End Source File
# Begin Source File

SOURCE=..\ch2\objtrace.h
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\opcreate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\opfree.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\oputil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\pack.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\pack_size.c
# End Source File
# Begin Source File

SOURCE=..\..\include\patchlevel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\profile\pcontrol.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dmpi\pkutil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\probe.c
# End Source File
# Begin Source File

SOURCE=..\..\src\util\ptrcvt.c
# End Source File
# Begin Source File

SOURCE=..\..\include\ptrcvt.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\put.c
# End Source File
# Begin Source File

SOURCE=..\util\queue.c
# End Source File
# Begin Source File

SOURCE=..\util\queue.h
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\recv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\red_scat.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\reduce.c
# End Source File
# Begin Source File

SOURCE=..\ch2\req.h
# End Source File
# Begin Source File

SOURCE=..\ch2\reqalloc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\requestc2f.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\rsend.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\rsend_init.c
# End Source File
# Begin Source File

SOURCE=..\..\include\sbcnst.h
# End Source File
# Begin Source File

SOURCE=..\util\sbcnst2.h
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\scan.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\scatter.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\scatterv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\send.c
# End Source File
# Begin Source File

SOURCE=..\..\include\sendq.h
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\sendrecv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\sendrecv_rep.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\sendutil.c
# End Source File
# Begin Source File

SOURCE=.\smiscatter.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\sse_ops.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\ssend.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\ssend_init.c
# End Source File
# Begin Source File

SOURCE=..\util\stack.c
# End Source File
# Begin Source File

SOURCE=..\util\stack.h
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\start.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\startall.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\statusc2f.c
# End Source File
# Begin Source File

SOURCE=..\..\src\external\statuscancel.c
# End Source File
# Begin Source File

SOURCE=..\..\src\external\statuselm.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\statusf2c.c
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\subarray.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\test.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\testall.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\testany.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\testcancel.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\testsome.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\topo_test.c
# End Source File
# Begin Source File

SOURCE=..\..\src\topol\topo_util.c
# End Source File
# Begin Source File

SOURCE=..\util\tr2.h
# End Source File
# Begin Source File

SOURCE=..\..\src\misc2\type_blkind.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_commit.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_contig.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_extent.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_ff.c
# End Source File
# Begin Source File

SOURCE=..\..\src\external\type_flatten.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_free.c
# End Source File
# Begin Source File

SOURCE=..\..\src\external\type_get_cont.c
# End Source File
# Begin Source File

SOURCE=..\..\src\external\type_get_env.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_hind.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_hvec.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_ind.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_lb.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_size.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_struct.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_ub.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\type_vec.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\unpack.c
# End Source File
# Begin Source File

SOURCE=..\..\src\util\util_hbt.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\wait.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\waitall.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\waitany.c
# End Source File
# Begin Source File

SOURCE=..\..\src\pt2pt\waitsome.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\wincomplete.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\wincreate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\winfree.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\winlock.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\winpost.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\winstart.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\wintest.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\winunlock.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ssided\winwait.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\wtick.c
# End Source File
# Begin Source File

SOURCE=..\..\src\env\wtime.c
# End Source File
# Begin Source File

SOURCE=..\..\src\coll\x86_ops.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
