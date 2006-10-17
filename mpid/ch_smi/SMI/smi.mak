# Microsoft Developer Studio Generated NMAKE File, Based on smi.dsp
!IF "$(CFG)" == ""
CFG=smi - Win32 SMP
!MESSAGE Keine Konfiguration angegeben. smi - Win32 SMP wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "smi - Win32 SMP" && "$(CFG)" != "smi - Win32 SCI" && "$(CFG)" != "smi - Win32 SVM" && "$(CFG)" != "smi - Win32 SCI_light" && "$(CFG)" != "smi - Win32 Debug" && "$(CFG)" != "smi - Win32 Release"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "smi.mak" CFG="smi - Win32 SMP"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "smi - Win32 SMP" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 SCI" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 SVM" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 SCI_light" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "smi - Win32 SMP"

OUTDIR=c:\tmp
INTDIR=c:\tmp

ALL : ".\lib\win32\smi.lib"


CLEAN :
	-@erase "$(INTDIR)\address_to_region.obj"
	-@erase "$(INTDIR)\combine_add.obj"
	-@erase "$(INTDIR)\connect_shreg.obj"
	-@erase "$(INTDIR)\copy.obj"
	-@erase "$(INTDIR)\copy_every_local.obj"
	-@erase "$(INTDIR)\copy_gl_dist_local.obj"
	-@erase "$(INTDIR)\cpuid.obj"
	-@erase "$(INTDIR)\create_shreg.obj"
	-@erase "$(INTDIR)\dyn_mem.obj"
	-@erase "$(INTDIR)\ensure_consistency.obj"
	-@erase "$(INTDIR)\err.obj"
	-@erase "$(INTDIR)\error_count.obj"
	-@erase "$(INTDIR)\first_proc_on_node.obj"
	-@erase "$(INTDIR)\fortran_binding.obj"
	-@erase "$(INTDIR)\free_shreg.obj"
	-@erase "$(INTDIR)\general.obj"
	-@erase "$(INTDIR)\general_definitions.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\idstack.obj"
	-@erase "$(INTDIR)\init_switching.obj"
	-@erase "$(INTDIR)\internal_regions.obj"
	-@erase "$(INTDIR)\local_seg.obj"
	-@erase "$(INTDIR)\loop.obj"
	-@erase "$(INTDIR)\loop_get.obj"
	-@erase "$(INTDIR)\loop_interface.obj"
	-@erase "$(INTDIR)\loop_partition.obj"
	-@erase "$(INTDIR)\loop_split.obj"
	-@erase "$(INTDIR)\loop_static.obj"
	-@erase "$(INTDIR)\lowlevelmp.obj"
	-@erase "$(INTDIR)\memcpy.obj"
	-@erase "$(INTDIR)\memcpy_base.obj"
	-@erase "$(INTDIR)\memtree.obj"
	-@erase "$(INTDIR)\mmx_memcpy_win.obj"
	-@erase "$(INTDIR)\mutex.obj"
	-@erase "$(INTDIR)\node_name.obj"
	-@erase "$(INTDIR)\node_rank.obj"
	-@erase "$(INTDIR)\node_size.obj"
	-@erase "$(INTDIR)\page_size.obj"
	-@erase "$(INTDIR)\print_regions.obj"
	-@erase "$(INTDIR)\proc_rank.obj"
	-@erase "$(INTDIR)\proc_size.obj"
	-@erase "$(INTDIR)\proc_to_node.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\putget.obj"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\redirect_io.obj"
	-@erase "$(INTDIR)\region_layout.obj"
	-@erase "$(INTDIR)\resource_list.obj"
	-@erase "$(INTDIR)\safety.obj"
	-@erase "$(INTDIR)\salloc.obj"
	-@erase "$(INTDIR)\sci_desc.obj"
	-@erase "$(INTDIR)\sci_shmem.obj"
	-@erase "$(INTDIR)\sciflush.obj"
	-@erase "$(INTDIR)\scistartup.obj"
	-@erase "$(INTDIR)\segment_address.obj"
	-@erase "$(INTDIR)\setup.obj"
	-@erase "$(INTDIR)\sfree.obj"
	-@erase "$(INTDIR)\Shm.obj"
	-@erase "$(INTDIR)\shmem.obj"
	-@erase "$(INTDIR)\shseg_key.obj"
	-@erase "$(INTDIR)\signalization.obj"
	-@erase "$(INTDIR)\sinit.obj"
	-@erase "$(INTDIR)\sisci_memcpy.obj"
	-@erase "$(INTDIR)\smi_fifo.obj"
	-@erase "$(INTDIR)\smi_finalize.obj"
	-@erase "$(INTDIR)\smi_init.obj"
	-@erase "$(INTDIR)\smi_time.obj"
	-@erase "$(INTDIR)\smibarrier.obj"
	-@erase "$(INTDIR)\smisendrecv.obj"
	-@erase "$(INTDIR)\smpstartup.obj"
	-@erase "$(INTDIR)\startup.obj"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\store_barrier.obj"
	-@erase "$(INTDIR)\switch_to_replication.obj"
	-@erase "$(INTDIR)\switch_to_replication_fast.obj"
	-@erase "$(INTDIR)\switch_to_sharing.obj"
	-@erase "$(INTDIR)\switch_to_sharing_fast.obj"
	-@erase "$(INTDIR)\sync_finalize.obj"
	-@erase "$(INTDIR)\sync_init.obj"
	-@erase "$(INTDIR)\tcpsync.obj"
	-@erase "$(INTDIR)\time.obj"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\unistd.obj"
	-@erase "$(INTDIR)\unix_shmem.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchdog.obj"
	-@erase "$(INTDIR)\wc_memcpy_c.obj"
	-@erase ".\lib\win32\smi.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

LINK32=link.exe
MTL=midl.exe
F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /G5 /MT /GX /O2 /I "src include" /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_DEBUG" /D "FORTRAN" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\smi.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\win32\smi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\address_to_region.obj" \
	"$(INTDIR)\combine_add.obj" \
	"$(INTDIR)\connect_shreg.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\copy_every_local.obj" \
	"$(INTDIR)\copy_gl_dist_local.obj" \
	"$(INTDIR)\cpuid.obj" \
	"$(INTDIR)\create_shreg.obj" \
	"$(INTDIR)\dyn_mem.obj" \
	"$(INTDIR)\ensure_consistency.obj" \
	"$(INTDIR)\err.obj" \
	"$(INTDIR)\error_count.obj" \
	"$(INTDIR)\first_proc_on_node.obj" \
	"$(INTDIR)\fortran_binding.obj" \
	"$(INTDIR)\free_shreg.obj" \
	"$(INTDIR)\general.obj" \
	"$(INTDIR)\general_definitions.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\idstack.obj" \
	"$(INTDIR)\init_switching.obj" \
	"$(INTDIR)\internal_regions.obj" \
	"$(INTDIR)\local_seg.obj" \
	"$(INTDIR)\loop.obj" \
	"$(INTDIR)\loop_get.obj" \
	"$(INTDIR)\loop_interface.obj" \
	"$(INTDIR)\loop_partition.obj" \
	"$(INTDIR)\loop_split.obj" \
	"$(INTDIR)\loop_static.obj" \
	"$(INTDIR)\lowlevelmp.obj" \
	"$(INTDIR)\memcpy.obj" \
	"$(INTDIR)\memcpy_base.obj" \
	"$(INTDIR)\memtree.obj" \
	"$(INTDIR)\mmx_memcpy_win.obj" \
	"$(INTDIR)\mutex.obj" \
	"$(INTDIR)\node_name.obj" \
	"$(INTDIR)\node_rank.obj" \
	"$(INTDIR)\node_size.obj" \
	"$(INTDIR)\page_size.obj" \
	"$(INTDIR)\print_regions.obj" \
	"$(INTDIR)\proc_rank.obj" \
	"$(INTDIR)\proc_size.obj" \
	"$(INTDIR)\proc_to_node.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\putget.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\redirect_io.obj" \
	"$(INTDIR)\region_layout.obj" \
	"$(INTDIR)\resource_list.obj" \
	"$(INTDIR)\safety.obj" \
	"$(INTDIR)\salloc.obj" \
	"$(INTDIR)\sci_desc.obj" \
	"$(INTDIR)\sci_shmem.obj" \
	"$(INTDIR)\sciflush.obj" \
	"$(INTDIR)\scistartup.obj" \
	"$(INTDIR)\segment_address.obj" \
	"$(INTDIR)\setup.obj" \
	"$(INTDIR)\sfree.obj" \
	"$(INTDIR)\Shm.obj" \
	"$(INTDIR)\shmem.obj" \
	"$(INTDIR)\shseg_key.obj" \
	"$(INTDIR)\signalization.obj" \
	"$(INTDIR)\sinit.obj" \
	"$(INTDIR)\sisci_memcpy.obj" \
	"$(INTDIR)\smi_fifo.obj" \
	"$(INTDIR)\smi_finalize.obj" \
	"$(INTDIR)\smi_init.obj" \
	"$(INTDIR)\smi_time.obj" \
	"$(INTDIR)\smibarrier.obj" \
	"$(INTDIR)\smisendrecv.obj" \
	"$(INTDIR)\smpstartup.obj" \
	"$(INTDIR)\startup.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\store_barrier.obj" \
	"$(INTDIR)\switch_to_replication.obj" \
	"$(INTDIR)\switch_to_replication_fast.obj" \
	"$(INTDIR)\switch_to_sharing.obj" \
	"$(INTDIR)\switch_to_sharing_fast.obj" \
	"$(INTDIR)\sync_finalize.obj" \
	"$(INTDIR)\sync_init.obj" \
	"$(INTDIR)\tcpsync.obj" \
	"$(INTDIR)\time.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\unistd.obj" \
	"$(INTDIR)\unix_shmem.obj" \
	"$(INTDIR)\watchdog.obj" \
	"$(INTDIR)\wc_memcpy_c.obj"

".\lib\win32\smi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

OUTDIR=.\lib
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\lib
# End Custom Macros

ALL : "$(OUTDIR)\smi.lib" "$(OUTDIR)\smi.bsc"


CLEAN :
	-@erase "$(INTDIR)\address_to_region.obj"
	-@erase "$(INTDIR)\address_to_region.sbr"
	-@erase "$(INTDIR)\combine_add.obj"
	-@erase "$(INTDIR)\combine_add.sbr"
	-@erase "$(INTDIR)\connect_shreg.obj"
	-@erase "$(INTDIR)\connect_shreg.sbr"
	-@erase "$(INTDIR)\copy.obj"
	-@erase "$(INTDIR)\copy.sbr"
	-@erase "$(INTDIR)\copy_every_local.obj"
	-@erase "$(INTDIR)\copy_every_local.sbr"
	-@erase "$(INTDIR)\copy_gl_dist_local.obj"
	-@erase "$(INTDIR)\copy_gl_dist_local.sbr"
	-@erase "$(INTDIR)\cpuid.obj"
	-@erase "$(INTDIR)\cpuid.sbr"
	-@erase "$(INTDIR)\create_shreg.obj"
	-@erase "$(INTDIR)\create_shreg.sbr"
	-@erase "$(INTDIR)\dyn_mem.obj"
	-@erase "$(INTDIR)\dyn_mem.sbr"
	-@erase "$(INTDIR)\ensure_consistency.obj"
	-@erase "$(INTDIR)\ensure_consistency.sbr"
	-@erase "$(INTDIR)\err.obj"
	-@erase "$(INTDIR)\err.sbr"
	-@erase "$(INTDIR)\error_count.obj"
	-@erase "$(INTDIR)\error_count.sbr"
	-@erase "$(INTDIR)\first_proc_on_node.obj"
	-@erase "$(INTDIR)\first_proc_on_node.sbr"
	-@erase "$(INTDIR)\fortran_binding.obj"
	-@erase "$(INTDIR)\fortran_binding.sbr"
	-@erase "$(INTDIR)\free_shreg.obj"
	-@erase "$(INTDIR)\free_shreg.sbr"
	-@erase "$(INTDIR)\general.obj"
	-@erase "$(INTDIR)\general.sbr"
	-@erase "$(INTDIR)\general_definitions.obj"
	-@erase "$(INTDIR)\general_definitions.sbr"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt.sbr"
	-@erase "$(INTDIR)\idstack.obj"
	-@erase "$(INTDIR)\idstack.sbr"
	-@erase "$(INTDIR)\init_switching.obj"
	-@erase "$(INTDIR)\init_switching.sbr"
	-@erase "$(INTDIR)\internal_regions.obj"
	-@erase "$(INTDIR)\internal_regions.sbr"
	-@erase "$(INTDIR)\local_seg.obj"
	-@erase "$(INTDIR)\local_seg.sbr"
	-@erase "$(INTDIR)\loop.obj"
	-@erase "$(INTDIR)\loop.sbr"
	-@erase "$(INTDIR)\loop_get.obj"
	-@erase "$(INTDIR)\loop_get.sbr"
	-@erase "$(INTDIR)\loop_interface.obj"
	-@erase "$(INTDIR)\loop_interface.sbr"
	-@erase "$(INTDIR)\loop_partition.obj"
	-@erase "$(INTDIR)\loop_partition.sbr"
	-@erase "$(INTDIR)\loop_split.obj"
	-@erase "$(INTDIR)\loop_split.sbr"
	-@erase "$(INTDIR)\loop_static.obj"
	-@erase "$(INTDIR)\loop_static.sbr"
	-@erase "$(INTDIR)\lowlevelmp.obj"
	-@erase "$(INTDIR)\lowlevelmp.sbr"
	-@erase "$(INTDIR)\memcpy.obj"
	-@erase "$(INTDIR)\memcpy.sbr"
	-@erase "$(INTDIR)\memcpy_base.obj"
	-@erase "$(INTDIR)\memcpy_base.sbr"
	-@erase "$(INTDIR)\memtree.obj"
	-@erase "$(INTDIR)\memtree.sbr"
	-@erase "$(INTDIR)\mmx_memcpy_win.obj"
	-@erase "$(INTDIR)\mmx_memcpy_win.sbr"
	-@erase "$(INTDIR)\mutex.obj"
	-@erase "$(INTDIR)\mutex.sbr"
	-@erase "$(INTDIR)\node_name.obj"
	-@erase "$(INTDIR)\node_name.sbr"
	-@erase "$(INTDIR)\node_rank.obj"
	-@erase "$(INTDIR)\node_rank.sbr"
	-@erase "$(INTDIR)\node_size.obj"
	-@erase "$(INTDIR)\node_size.sbr"
	-@erase "$(INTDIR)\page_size.obj"
	-@erase "$(INTDIR)\page_size.sbr"
	-@erase "$(INTDIR)\print_regions.obj"
	-@erase "$(INTDIR)\print_regions.sbr"
	-@erase "$(INTDIR)\proc_rank.obj"
	-@erase "$(INTDIR)\proc_rank.sbr"
	-@erase "$(INTDIR)\proc_size.obj"
	-@erase "$(INTDIR)\proc_size.sbr"
	-@erase "$(INTDIR)\proc_to_node.obj"
	-@erase "$(INTDIR)\proc_to_node.sbr"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\progress.sbr"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\pthread.sbr"
	-@erase "$(INTDIR)\putget.obj"
	-@erase "$(INTDIR)\putget.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\redirect_io.obj"
	-@erase "$(INTDIR)\redirect_io.sbr"
	-@erase "$(INTDIR)\region_layout.obj"
	-@erase "$(INTDIR)\region_layout.sbr"
	-@erase "$(INTDIR)\resource_list.obj"
	-@erase "$(INTDIR)\resource_list.sbr"
	-@erase "$(INTDIR)\safety.obj"
	-@erase "$(INTDIR)\safety.sbr"
	-@erase "$(INTDIR)\salloc.obj"
	-@erase "$(INTDIR)\salloc.sbr"
	-@erase "$(INTDIR)\sci_desc.obj"
	-@erase "$(INTDIR)\sci_desc.sbr"
	-@erase "$(INTDIR)\sci_shmem.obj"
	-@erase "$(INTDIR)\sci_shmem.sbr"
	-@erase "$(INTDIR)\sciflush.obj"
	-@erase "$(INTDIR)\sciflush.sbr"
	-@erase "$(INTDIR)\scistartup.obj"
	-@erase "$(INTDIR)\scistartup.sbr"
	-@erase "$(INTDIR)\segment_address.obj"
	-@erase "$(INTDIR)\segment_address.sbr"
	-@erase "$(INTDIR)\setup.obj"
	-@erase "$(INTDIR)\setup.sbr"
	-@erase "$(INTDIR)\sfree.obj"
	-@erase "$(INTDIR)\sfree.sbr"
	-@erase "$(INTDIR)\Shm.obj"
	-@erase "$(INTDIR)\Shm.sbr"
	-@erase "$(INTDIR)\shmem.obj"
	-@erase "$(INTDIR)\shmem.sbr"
	-@erase "$(INTDIR)\shseg_key.obj"
	-@erase "$(INTDIR)\shseg_key.sbr"
	-@erase "$(INTDIR)\signalization.obj"
	-@erase "$(INTDIR)\signalization.sbr"
	-@erase "$(INTDIR)\sinit.obj"
	-@erase "$(INTDIR)\sinit.sbr"
	-@erase "$(INTDIR)\sisci_memcpy.obj"
	-@erase "$(INTDIR)\sisci_memcpy.sbr"
	-@erase "$(INTDIR)\smi_fifo.obj"
	-@erase "$(INTDIR)\smi_fifo.sbr"
	-@erase "$(INTDIR)\smi_finalize.obj"
	-@erase "$(INTDIR)\smi_finalize.sbr"
	-@erase "$(INTDIR)\smi_init.obj"
	-@erase "$(INTDIR)\smi_init.sbr"
	-@erase "$(INTDIR)\smi_time.obj"
	-@erase "$(INTDIR)\smi_time.sbr"
	-@erase "$(INTDIR)\smibarrier.obj"
	-@erase "$(INTDIR)\smibarrier.sbr"
	-@erase "$(INTDIR)\smisendrecv.obj"
	-@erase "$(INTDIR)\smisendrecv.sbr"
	-@erase "$(INTDIR)\smpstartup.obj"
	-@erase "$(INTDIR)\smpstartup.sbr"
	-@erase "$(INTDIR)\startup.obj"
	-@erase "$(INTDIR)\startup.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\store_barrier.obj"
	-@erase "$(INTDIR)\store_barrier.sbr"
	-@erase "$(INTDIR)\switch_to_replication.obj"
	-@erase "$(INTDIR)\switch_to_replication.sbr"
	-@erase "$(INTDIR)\switch_to_replication_fast.obj"
	-@erase "$(INTDIR)\switch_to_replication_fast.sbr"
	-@erase "$(INTDIR)\switch_to_sharing.obj"
	-@erase "$(INTDIR)\switch_to_sharing.sbr"
	-@erase "$(INTDIR)\switch_to_sharing_fast.obj"
	-@erase "$(INTDIR)\switch_to_sharing_fast.sbr"
	-@erase "$(INTDIR)\sync_finalize.obj"
	-@erase "$(INTDIR)\sync_finalize.sbr"
	-@erase "$(INTDIR)\sync_init.obj"
	-@erase "$(INTDIR)\sync_init.sbr"
	-@erase "$(INTDIR)\tcpsync.obj"
	-@erase "$(INTDIR)\tcpsync.sbr"
	-@erase "$(INTDIR)\time.obj"
	-@erase "$(INTDIR)\time.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\unistd.obj"
	-@erase "$(INTDIR)\unistd.sbr"
	-@erase "$(INTDIR)\unix_shmem.obj"
	-@erase "$(INTDIR)\unix_shmem.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchdog.obj"
	-@erase "$(INTDIR)\watchdog.sbr"
	-@erase "$(INTDIR)\wc_memcpy_c.obj"
	-@erase "$(INTDIR)\wc_memcpy_c.sbr"
	-@erase "$(OUTDIR)\smi.bsc"
	-@erase "$(OUTDIR)\smi.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

LINK32=link.exe
MTL=midl.exe
F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /MT /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\smi.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\address_to_region.sbr" \
	"$(INTDIR)\combine_add.sbr" \
	"$(INTDIR)\connect_shreg.sbr" \
	"$(INTDIR)\copy.sbr" \
	"$(INTDIR)\copy_every_local.sbr" \
	"$(INTDIR)\copy_gl_dist_local.sbr" \
	"$(INTDIR)\cpuid.sbr" \
	"$(INTDIR)\create_shreg.sbr" \
	"$(INTDIR)\dyn_mem.sbr" \
	"$(INTDIR)\ensure_consistency.sbr" \
	"$(INTDIR)\err.sbr" \
	"$(INTDIR)\error_count.sbr" \
	"$(INTDIR)\first_proc_on_node.sbr" \
	"$(INTDIR)\fortran_binding.sbr" \
	"$(INTDIR)\free_shreg.sbr" \
	"$(INTDIR)\general.sbr" \
	"$(INTDIR)\general_definitions.sbr" \
	"$(INTDIR)\getopt.sbr" \
	"$(INTDIR)\idstack.sbr" \
	"$(INTDIR)\init_switching.sbr" \
	"$(INTDIR)\internal_regions.sbr" \
	"$(INTDIR)\local_seg.sbr" \
	"$(INTDIR)\loop.sbr" \
	"$(INTDIR)\loop_get.sbr" \
	"$(INTDIR)\loop_interface.sbr" \
	"$(INTDIR)\loop_partition.sbr" \
	"$(INTDIR)\loop_split.sbr" \
	"$(INTDIR)\loop_static.sbr" \
	"$(INTDIR)\lowlevelmp.sbr" \
	"$(INTDIR)\memcpy.sbr" \
	"$(INTDIR)\memcpy_base.sbr" \
	"$(INTDIR)\memtree.sbr" \
	"$(INTDIR)\mmx_memcpy_win.sbr" \
	"$(INTDIR)\mutex.sbr" \
	"$(INTDIR)\node_name.sbr" \
	"$(INTDIR)\node_rank.sbr" \
	"$(INTDIR)\node_size.sbr" \
	"$(INTDIR)\page_size.sbr" \
	"$(INTDIR)\print_regions.sbr" \
	"$(INTDIR)\proc_rank.sbr" \
	"$(INTDIR)\proc_size.sbr" \
	"$(INTDIR)\proc_to_node.sbr" \
	"$(INTDIR)\progress.sbr" \
	"$(INTDIR)\pthread.sbr" \
	"$(INTDIR)\putget.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\redirect_io.sbr" \
	"$(INTDIR)\region_layout.sbr" \
	"$(INTDIR)\resource_list.sbr" \
	"$(INTDIR)\safety.sbr" \
	"$(INTDIR)\salloc.sbr" \
	"$(INTDIR)\sci_desc.sbr" \
	"$(INTDIR)\sci_shmem.sbr" \
	"$(INTDIR)\sciflush.sbr" \
	"$(INTDIR)\scistartup.sbr" \
	"$(INTDIR)\segment_address.sbr" \
	"$(INTDIR)\setup.sbr" \
	"$(INTDIR)\sfree.sbr" \
	"$(INTDIR)\Shm.sbr" \
	"$(INTDIR)\shmem.sbr" \
	"$(INTDIR)\shseg_key.sbr" \
	"$(INTDIR)\signalization.sbr" \
	"$(INTDIR)\sinit.sbr" \
	"$(INTDIR)\sisci_memcpy.sbr" \
	"$(INTDIR)\smi_fifo.sbr" \
	"$(INTDIR)\smi_finalize.sbr" \
	"$(INTDIR)\smi_init.sbr" \
	"$(INTDIR)\smi_time.sbr" \
	"$(INTDIR)\smibarrier.sbr" \
	"$(INTDIR)\smisendrecv.sbr" \
	"$(INTDIR)\smpstartup.sbr" \
	"$(INTDIR)\startup.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\store_barrier.sbr" \
	"$(INTDIR)\switch_to_replication.sbr" \
	"$(INTDIR)\switch_to_replication_fast.sbr" \
	"$(INTDIR)\switch_to_sharing.sbr" \
	"$(INTDIR)\switch_to_sharing_fast.sbr" \
	"$(INTDIR)\sync_finalize.sbr" \
	"$(INTDIR)\sync_init.sbr" \
	"$(INTDIR)\tcpsync.sbr" \
	"$(INTDIR)\time.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\unistd.sbr" \
	"$(INTDIR)\unix_shmem.sbr" \
	"$(INTDIR)\watchdog.sbr" \
	"$(INTDIR)\wc_memcpy_c.sbr"

"$(OUTDIR)\smi.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\smi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\address_to_region.obj" \
	"$(INTDIR)\combine_add.obj" \
	"$(INTDIR)\connect_shreg.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\copy_every_local.obj" \
	"$(INTDIR)\copy_gl_dist_local.obj" \
	"$(INTDIR)\cpuid.obj" \
	"$(INTDIR)\create_shreg.obj" \
	"$(INTDIR)\dyn_mem.obj" \
	"$(INTDIR)\ensure_consistency.obj" \
	"$(INTDIR)\err.obj" \
	"$(INTDIR)\error_count.obj" \
	"$(INTDIR)\first_proc_on_node.obj" \
	"$(INTDIR)\fortran_binding.obj" \
	"$(INTDIR)\free_shreg.obj" \
	"$(INTDIR)\general.obj" \
	"$(INTDIR)\general_definitions.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\idstack.obj" \
	"$(INTDIR)\init_switching.obj" \
	"$(INTDIR)\internal_regions.obj" \
	"$(INTDIR)\local_seg.obj" \
	"$(INTDIR)\loop.obj" \
	"$(INTDIR)\loop_get.obj" \
	"$(INTDIR)\loop_interface.obj" \
	"$(INTDIR)\loop_partition.obj" \
	"$(INTDIR)\loop_split.obj" \
	"$(INTDIR)\loop_static.obj" \
	"$(INTDIR)\lowlevelmp.obj" \
	"$(INTDIR)\memcpy.obj" \
	"$(INTDIR)\memcpy_base.obj" \
	"$(INTDIR)\memtree.obj" \
	"$(INTDIR)\mmx_memcpy_win.obj" \
	"$(INTDIR)\mutex.obj" \
	"$(INTDIR)\node_name.obj" \
	"$(INTDIR)\node_rank.obj" \
	"$(INTDIR)\node_size.obj" \
	"$(INTDIR)\page_size.obj" \
	"$(INTDIR)\print_regions.obj" \
	"$(INTDIR)\proc_rank.obj" \
	"$(INTDIR)\proc_size.obj" \
	"$(INTDIR)\proc_to_node.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\putget.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\redirect_io.obj" \
	"$(INTDIR)\region_layout.obj" \
	"$(INTDIR)\resource_list.obj" \
	"$(INTDIR)\safety.obj" \
	"$(INTDIR)\salloc.obj" \
	"$(INTDIR)\sci_desc.obj" \
	"$(INTDIR)\sci_shmem.obj" \
	"$(INTDIR)\sciflush.obj" \
	"$(INTDIR)\scistartup.obj" \
	"$(INTDIR)\segment_address.obj" \
	"$(INTDIR)\setup.obj" \
	"$(INTDIR)\sfree.obj" \
	"$(INTDIR)\Shm.obj" \
	"$(INTDIR)\shmem.obj" \
	"$(INTDIR)\shseg_key.obj" \
	"$(INTDIR)\signalization.obj" \
	"$(INTDIR)\sinit.obj" \
	"$(INTDIR)\sisci_memcpy.obj" \
	"$(INTDIR)\smi_fifo.obj" \
	"$(INTDIR)\smi_finalize.obj" \
	"$(INTDIR)\smi_init.obj" \
	"$(INTDIR)\smi_time.obj" \
	"$(INTDIR)\smibarrier.obj" \
	"$(INTDIR)\smisendrecv.obj" \
	"$(INTDIR)\smpstartup.obj" \
	"$(INTDIR)\startup.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\store_barrier.obj" \
	"$(INTDIR)\switch_to_replication.obj" \
	"$(INTDIR)\switch_to_replication_fast.obj" \
	"$(INTDIR)\switch_to_sharing.obj" \
	"$(INTDIR)\switch_to_sharing_fast.obj" \
	"$(INTDIR)\sync_finalize.obj" \
	"$(INTDIR)\sync_init.obj" \
	"$(INTDIR)\tcpsync.obj" \
	"$(INTDIR)\time.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\unistd.obj" \
	"$(INTDIR)\unix_shmem.obj" \
	"$(INTDIR)\watchdog.obj" \
	"$(INTDIR)\wc_memcpy_c.obj"

"$(OUTDIR)\smi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

OUTDIR=c:\tmp
INTDIR=c:\tmp
# Begin Custom Macros
OutDir=c:\tmp
# End Custom Macros

ALL : ".\lib\win32\smi.lib" "$(OUTDIR)\smi.bsc"


CLEAN :
	-@erase "$(INTDIR)\address_to_region.obj"
	-@erase "$(INTDIR)\address_to_region.sbr"
	-@erase "$(INTDIR)\combine_add.obj"
	-@erase "$(INTDIR)\combine_add.sbr"
	-@erase "$(INTDIR)\connect_shreg.obj"
	-@erase "$(INTDIR)\connect_shreg.sbr"
	-@erase "$(INTDIR)\copy.obj"
	-@erase "$(INTDIR)\copy.sbr"
	-@erase "$(INTDIR)\copy_every_local.obj"
	-@erase "$(INTDIR)\copy_every_local.sbr"
	-@erase "$(INTDIR)\copy_gl_dist_local.obj"
	-@erase "$(INTDIR)\copy_gl_dist_local.sbr"
	-@erase "$(INTDIR)\cpuid.obj"
	-@erase "$(INTDIR)\cpuid.sbr"
	-@erase "$(INTDIR)\create_shreg.obj"
	-@erase "$(INTDIR)\create_shreg.sbr"
	-@erase "$(INTDIR)\dyn_mem.obj"
	-@erase "$(INTDIR)\dyn_mem.sbr"
	-@erase "$(INTDIR)\ensure_consistency.obj"
	-@erase "$(INTDIR)\ensure_consistency.sbr"
	-@erase "$(INTDIR)\err.obj"
	-@erase "$(INTDIR)\err.sbr"
	-@erase "$(INTDIR)\error_count.obj"
	-@erase "$(INTDIR)\error_count.sbr"
	-@erase "$(INTDIR)\first_proc_on_node.obj"
	-@erase "$(INTDIR)\first_proc_on_node.sbr"
	-@erase "$(INTDIR)\fortran_binding.obj"
	-@erase "$(INTDIR)\fortran_binding.sbr"
	-@erase "$(INTDIR)\free_shreg.obj"
	-@erase "$(INTDIR)\free_shreg.sbr"
	-@erase "$(INTDIR)\general.obj"
	-@erase "$(INTDIR)\general.sbr"
	-@erase "$(INTDIR)\general_definitions.obj"
	-@erase "$(INTDIR)\general_definitions.sbr"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt.sbr"
	-@erase "$(INTDIR)\idstack.obj"
	-@erase "$(INTDIR)\idstack.sbr"
	-@erase "$(INTDIR)\init_switching.obj"
	-@erase "$(INTDIR)\init_switching.sbr"
	-@erase "$(INTDIR)\internal_regions.obj"
	-@erase "$(INTDIR)\internal_regions.sbr"
	-@erase "$(INTDIR)\local_seg.obj"
	-@erase "$(INTDIR)\local_seg.sbr"
	-@erase "$(INTDIR)\loop.obj"
	-@erase "$(INTDIR)\loop.sbr"
	-@erase "$(INTDIR)\loop_get.obj"
	-@erase "$(INTDIR)\loop_get.sbr"
	-@erase "$(INTDIR)\loop_interface.obj"
	-@erase "$(INTDIR)\loop_interface.sbr"
	-@erase "$(INTDIR)\loop_partition.obj"
	-@erase "$(INTDIR)\loop_partition.sbr"
	-@erase "$(INTDIR)\loop_split.obj"
	-@erase "$(INTDIR)\loop_split.sbr"
	-@erase "$(INTDIR)\loop_static.obj"
	-@erase "$(INTDIR)\loop_static.sbr"
	-@erase "$(INTDIR)\lowlevelmp.obj"
	-@erase "$(INTDIR)\lowlevelmp.sbr"
	-@erase "$(INTDIR)\memcpy.obj"
	-@erase "$(INTDIR)\memcpy.sbr"
	-@erase "$(INTDIR)\memcpy_base.obj"
	-@erase "$(INTDIR)\memcpy_base.sbr"
	-@erase "$(INTDIR)\memtree.obj"
	-@erase "$(INTDIR)\memtree.sbr"
	-@erase "$(INTDIR)\mmx_memcpy_win.obj"
	-@erase "$(INTDIR)\mmx_memcpy_win.sbr"
	-@erase "$(INTDIR)\mutex.obj"
	-@erase "$(INTDIR)\mutex.sbr"
	-@erase "$(INTDIR)\node_name.obj"
	-@erase "$(INTDIR)\node_name.sbr"
	-@erase "$(INTDIR)\node_rank.obj"
	-@erase "$(INTDIR)\node_rank.sbr"
	-@erase "$(INTDIR)\node_size.obj"
	-@erase "$(INTDIR)\node_size.sbr"
	-@erase "$(INTDIR)\page_size.obj"
	-@erase "$(INTDIR)\page_size.sbr"
	-@erase "$(INTDIR)\print_regions.obj"
	-@erase "$(INTDIR)\print_regions.sbr"
	-@erase "$(INTDIR)\proc_rank.obj"
	-@erase "$(INTDIR)\proc_rank.sbr"
	-@erase "$(INTDIR)\proc_size.obj"
	-@erase "$(INTDIR)\proc_size.sbr"
	-@erase "$(INTDIR)\proc_to_node.obj"
	-@erase "$(INTDIR)\proc_to_node.sbr"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\progress.sbr"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\pthread.sbr"
	-@erase "$(INTDIR)\putget.obj"
	-@erase "$(INTDIR)\putget.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\redirect_io.obj"
	-@erase "$(INTDIR)\redirect_io.sbr"
	-@erase "$(INTDIR)\region_layout.obj"
	-@erase "$(INTDIR)\region_layout.sbr"
	-@erase "$(INTDIR)\resource_list.obj"
	-@erase "$(INTDIR)\resource_list.sbr"
	-@erase "$(INTDIR)\safety.obj"
	-@erase "$(INTDIR)\safety.sbr"
	-@erase "$(INTDIR)\salloc.obj"
	-@erase "$(INTDIR)\salloc.sbr"
	-@erase "$(INTDIR)\sci_desc.obj"
	-@erase "$(INTDIR)\sci_desc.sbr"
	-@erase "$(INTDIR)\sci_shmem.obj"
	-@erase "$(INTDIR)\sci_shmem.sbr"
	-@erase "$(INTDIR)\sciflush.obj"
	-@erase "$(INTDIR)\sciflush.sbr"
	-@erase "$(INTDIR)\scistartup.obj"
	-@erase "$(INTDIR)\scistartup.sbr"
	-@erase "$(INTDIR)\segment_address.obj"
	-@erase "$(INTDIR)\segment_address.sbr"
	-@erase "$(INTDIR)\setup.obj"
	-@erase "$(INTDIR)\setup.sbr"
	-@erase "$(INTDIR)\sfree.obj"
	-@erase "$(INTDIR)\sfree.sbr"
	-@erase "$(INTDIR)\Shm.obj"
	-@erase "$(INTDIR)\Shm.sbr"
	-@erase "$(INTDIR)\shmem.obj"
	-@erase "$(INTDIR)\shmem.sbr"
	-@erase "$(INTDIR)\shseg_key.obj"
	-@erase "$(INTDIR)\shseg_key.sbr"
	-@erase "$(INTDIR)\signalization.obj"
	-@erase "$(INTDIR)\signalization.sbr"
	-@erase "$(INTDIR)\sinit.obj"
	-@erase "$(INTDIR)\sinit.sbr"
	-@erase "$(INTDIR)\sisci_memcpy.obj"
	-@erase "$(INTDIR)\sisci_memcpy.sbr"
	-@erase "$(INTDIR)\smi_fifo.obj"
	-@erase "$(INTDIR)\smi_fifo.sbr"
	-@erase "$(INTDIR)\smi_finalize.obj"
	-@erase "$(INTDIR)\smi_finalize.sbr"
	-@erase "$(INTDIR)\smi_init.obj"
	-@erase "$(INTDIR)\smi_init.sbr"
	-@erase "$(INTDIR)\smi_time.obj"
	-@erase "$(INTDIR)\smi_time.sbr"
	-@erase "$(INTDIR)\smibarrier.obj"
	-@erase "$(INTDIR)\smibarrier.sbr"
	-@erase "$(INTDIR)\smisendrecv.obj"
	-@erase "$(INTDIR)\smisendrecv.sbr"
	-@erase "$(INTDIR)\smpstartup.obj"
	-@erase "$(INTDIR)\smpstartup.sbr"
	-@erase "$(INTDIR)\startup.obj"
	-@erase "$(INTDIR)\startup.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\store_barrier.obj"
	-@erase "$(INTDIR)\store_barrier.sbr"
	-@erase "$(INTDIR)\switch_to_replication.obj"
	-@erase "$(INTDIR)\switch_to_replication.sbr"
	-@erase "$(INTDIR)\switch_to_replication_fast.obj"
	-@erase "$(INTDIR)\switch_to_replication_fast.sbr"
	-@erase "$(INTDIR)\switch_to_sharing.obj"
	-@erase "$(INTDIR)\switch_to_sharing.sbr"
	-@erase "$(INTDIR)\switch_to_sharing_fast.obj"
	-@erase "$(INTDIR)\switch_to_sharing_fast.sbr"
	-@erase "$(INTDIR)\sync_finalize.obj"
	-@erase "$(INTDIR)\sync_finalize.sbr"
	-@erase "$(INTDIR)\sync_init.obj"
	-@erase "$(INTDIR)\sync_init.sbr"
	-@erase "$(INTDIR)\tcpsync.obj"
	-@erase "$(INTDIR)\tcpsync.sbr"
	-@erase "$(INTDIR)\time.obj"
	-@erase "$(INTDIR)\time.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\unistd.obj"
	-@erase "$(INTDIR)\unistd.sbr"
	-@erase "$(INTDIR)\unix_shmem.obj"
	-@erase "$(INTDIR)\unix_shmem.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchdog.obj"
	-@erase "$(INTDIR)\watchdog.sbr"
	-@erase "$(INTDIR)\wc_memcpy_c.obj"
	-@erase "$(INTDIR)\wc_memcpy_c.sbr"
	-@erase "$(OUTDIR)\smi.bsc"
	-@erase ".\lib\win32\smi.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

LINK32=link.exe
MTL=midl.exe
F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /G5 /MT /GX /O2 /I "src include" /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "NDEBUG" /D "SVM" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\smi.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\address_to_region.sbr" \
	"$(INTDIR)\combine_add.sbr" \
	"$(INTDIR)\connect_shreg.sbr" \
	"$(INTDIR)\copy.sbr" \
	"$(INTDIR)\copy_every_local.sbr" \
	"$(INTDIR)\copy_gl_dist_local.sbr" \
	"$(INTDIR)\cpuid.sbr" \
	"$(INTDIR)\create_shreg.sbr" \
	"$(INTDIR)\dyn_mem.sbr" \
	"$(INTDIR)\ensure_consistency.sbr" \
	"$(INTDIR)\err.sbr" \
	"$(INTDIR)\error_count.sbr" \
	"$(INTDIR)\first_proc_on_node.sbr" \
	"$(INTDIR)\fortran_binding.sbr" \
	"$(INTDIR)\free_shreg.sbr" \
	"$(INTDIR)\general.sbr" \
	"$(INTDIR)\general_definitions.sbr" \
	"$(INTDIR)\getopt.sbr" \
	"$(INTDIR)\idstack.sbr" \
	"$(INTDIR)\init_switching.sbr" \
	"$(INTDIR)\internal_regions.sbr" \
	"$(INTDIR)\local_seg.sbr" \
	"$(INTDIR)\loop.sbr" \
	"$(INTDIR)\loop_get.sbr" \
	"$(INTDIR)\loop_interface.sbr" \
	"$(INTDIR)\loop_partition.sbr" \
	"$(INTDIR)\loop_split.sbr" \
	"$(INTDIR)\loop_static.sbr" \
	"$(INTDIR)\lowlevelmp.sbr" \
	"$(INTDIR)\memcpy.sbr" \
	"$(INTDIR)\memcpy_base.sbr" \
	"$(INTDIR)\memtree.sbr" \
	"$(INTDIR)\mmx_memcpy_win.sbr" \
	"$(INTDIR)\mutex.sbr" \
	"$(INTDIR)\node_name.sbr" \
	"$(INTDIR)\node_rank.sbr" \
	"$(INTDIR)\node_size.sbr" \
	"$(INTDIR)\page_size.sbr" \
	"$(INTDIR)\print_regions.sbr" \
	"$(INTDIR)\proc_rank.sbr" \
	"$(INTDIR)\proc_size.sbr" \
	"$(INTDIR)\proc_to_node.sbr" \
	"$(INTDIR)\progress.sbr" \
	"$(INTDIR)\pthread.sbr" \
	"$(INTDIR)\putget.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\redirect_io.sbr" \
	"$(INTDIR)\region_layout.sbr" \
	"$(INTDIR)\resource_list.sbr" \
	"$(INTDIR)\safety.sbr" \
	"$(INTDIR)\salloc.sbr" \
	"$(INTDIR)\sci_desc.sbr" \
	"$(INTDIR)\sci_shmem.sbr" \
	"$(INTDIR)\sciflush.sbr" \
	"$(INTDIR)\scistartup.sbr" \
	"$(INTDIR)\segment_address.sbr" \
	"$(INTDIR)\setup.sbr" \
	"$(INTDIR)\sfree.sbr" \
	"$(INTDIR)\Shm.sbr" \
	"$(INTDIR)\shmem.sbr" \
	"$(INTDIR)\shseg_key.sbr" \
	"$(INTDIR)\signalization.sbr" \
	"$(INTDIR)\sinit.sbr" \
	"$(INTDIR)\sisci_memcpy.sbr" \
	"$(INTDIR)\smi_fifo.sbr" \
	"$(INTDIR)\smi_finalize.sbr" \
	"$(INTDIR)\smi_init.sbr" \
	"$(INTDIR)\smi_time.sbr" \
	"$(INTDIR)\smibarrier.sbr" \
	"$(INTDIR)\smisendrecv.sbr" \
	"$(INTDIR)\smpstartup.sbr" \
	"$(INTDIR)\startup.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\store_barrier.sbr" \
	"$(INTDIR)\switch_to_replication.sbr" \
	"$(INTDIR)\switch_to_replication_fast.sbr" \
	"$(INTDIR)\switch_to_sharing.sbr" \
	"$(INTDIR)\switch_to_sharing_fast.sbr" \
	"$(INTDIR)\sync_finalize.sbr" \
	"$(INTDIR)\sync_init.sbr" \
	"$(INTDIR)\tcpsync.sbr" \
	"$(INTDIR)\time.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\unistd.sbr" \
	"$(INTDIR)\unix_shmem.sbr" \
	"$(INTDIR)\watchdog.sbr" \
	"$(INTDIR)\wc_memcpy_c.sbr"

"$(OUTDIR)\smi.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\win32\smi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\address_to_region.obj" \
	"$(INTDIR)\combine_add.obj" \
	"$(INTDIR)\connect_shreg.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\copy_every_local.obj" \
	"$(INTDIR)\copy_gl_dist_local.obj" \
	"$(INTDIR)\cpuid.obj" \
	"$(INTDIR)\create_shreg.obj" \
	"$(INTDIR)\dyn_mem.obj" \
	"$(INTDIR)\ensure_consistency.obj" \
	"$(INTDIR)\err.obj" \
	"$(INTDIR)\error_count.obj" \
	"$(INTDIR)\first_proc_on_node.obj" \
	"$(INTDIR)\fortran_binding.obj" \
	"$(INTDIR)\free_shreg.obj" \
	"$(INTDIR)\general.obj" \
	"$(INTDIR)\general_definitions.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\idstack.obj" \
	"$(INTDIR)\init_switching.obj" \
	"$(INTDIR)\internal_regions.obj" \
	"$(INTDIR)\local_seg.obj" \
	"$(INTDIR)\loop.obj" \
	"$(INTDIR)\loop_get.obj" \
	"$(INTDIR)\loop_interface.obj" \
	"$(INTDIR)\loop_partition.obj" \
	"$(INTDIR)\loop_split.obj" \
	"$(INTDIR)\loop_static.obj" \
	"$(INTDIR)\lowlevelmp.obj" \
	"$(INTDIR)\memcpy.obj" \
	"$(INTDIR)\memcpy_base.obj" \
	"$(INTDIR)\memtree.obj" \
	"$(INTDIR)\mmx_memcpy_win.obj" \
	"$(INTDIR)\mutex.obj" \
	"$(INTDIR)\node_name.obj" \
	"$(INTDIR)\node_rank.obj" \
	"$(INTDIR)\node_size.obj" \
	"$(INTDIR)\page_size.obj" \
	"$(INTDIR)\print_regions.obj" \
	"$(INTDIR)\proc_rank.obj" \
	"$(INTDIR)\proc_size.obj" \
	"$(INTDIR)\proc_to_node.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\putget.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\redirect_io.obj" \
	"$(INTDIR)\region_layout.obj" \
	"$(INTDIR)\resource_list.obj" \
	"$(INTDIR)\safety.obj" \
	"$(INTDIR)\salloc.obj" \
	"$(INTDIR)\sci_desc.obj" \
	"$(INTDIR)\sci_shmem.obj" \
	"$(INTDIR)\sciflush.obj" \
	"$(INTDIR)\scistartup.obj" \
	"$(INTDIR)\segment_address.obj" \
	"$(INTDIR)\setup.obj" \
	"$(INTDIR)\sfree.obj" \
	"$(INTDIR)\Shm.obj" \
	"$(INTDIR)\shmem.obj" \
	"$(INTDIR)\shseg_key.obj" \
	"$(INTDIR)\signalization.obj" \
	"$(INTDIR)\sinit.obj" \
	"$(INTDIR)\sisci_memcpy.obj" \
	"$(INTDIR)\smi_fifo.obj" \
	"$(INTDIR)\smi_finalize.obj" \
	"$(INTDIR)\smi_init.obj" \
	"$(INTDIR)\smi_time.obj" \
	"$(INTDIR)\smibarrier.obj" \
	"$(INTDIR)\smisendrecv.obj" \
	"$(INTDIR)\smpstartup.obj" \
	"$(INTDIR)\startup.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\store_barrier.obj" \
	"$(INTDIR)\switch_to_replication.obj" \
	"$(INTDIR)\switch_to_replication_fast.obj" \
	"$(INTDIR)\switch_to_sharing.obj" \
	"$(INTDIR)\switch_to_sharing_fast.obj" \
	"$(INTDIR)\sync_finalize.obj" \
	"$(INTDIR)\sync_init.obj" \
	"$(INTDIR)\tcpsync.obj" \
	"$(INTDIR)\time.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\unistd.obj" \
	"$(INTDIR)\unix_shmem.obj" \
	"$(INTDIR)\watchdog.obj" \
	"$(INTDIR)\wc_memcpy_c.obj"

".\lib\win32\smi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

OUTDIR=.\lib
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\lib\ 
# End Custom Macros

ALL : "$(OUTDIR)\csmi.lib" "$(OUTDIR)\smi.bsc"


CLEAN :
	-@erase "$(INTDIR)\address_to_region.obj"
	-@erase "$(INTDIR)\address_to_region.sbr"
	-@erase "$(INTDIR)\connect_shreg.obj"
	-@erase "$(INTDIR)\connect_shreg.sbr"
	-@erase "$(INTDIR)\cpuid.obj"
	-@erase "$(INTDIR)\cpuid.sbr"
	-@erase "$(INTDIR)\create_shreg.obj"
	-@erase "$(INTDIR)\create_shreg.sbr"
	-@erase "$(INTDIR)\dyn_mem.obj"
	-@erase "$(INTDIR)\dyn_mem.sbr"
	-@erase "$(INTDIR)\error_count.obj"
	-@erase "$(INTDIR)\error_count.sbr"
	-@erase "$(INTDIR)\first_proc_on_node.obj"
	-@erase "$(INTDIR)\first_proc_on_node.sbr"
	-@erase "$(INTDIR)\free_shreg.obj"
	-@erase "$(INTDIR)\free_shreg.sbr"
	-@erase "$(INTDIR)\general.obj"
	-@erase "$(INTDIR)\general.sbr"
	-@erase "$(INTDIR)\general_definitions.obj"
	-@erase "$(INTDIR)\general_definitions.sbr"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt.sbr"
	-@erase "$(INTDIR)\idstack.obj"
	-@erase "$(INTDIR)\idstack.sbr"
	-@erase "$(INTDIR)\internal_regions.obj"
	-@erase "$(INTDIR)\internal_regions.sbr"
	-@erase "$(INTDIR)\local_seg.obj"
	-@erase "$(INTDIR)\local_seg.sbr"
	-@erase "$(INTDIR)\lowlevelmp.obj"
	-@erase "$(INTDIR)\lowlevelmp.sbr"
	-@erase "$(INTDIR)\memcpy.obj"
	-@erase "$(INTDIR)\memcpy.sbr"
	-@erase "$(INTDIR)\memcpy_base.obj"
	-@erase "$(INTDIR)\memcpy_base.sbr"
	-@erase "$(INTDIR)\memtree.obj"
	-@erase "$(INTDIR)\memtree.sbr"
	-@erase "$(INTDIR)\mmx_memcpy_win.obj"
	-@erase "$(INTDIR)\mmx_memcpy_win.sbr"
	-@erase "$(INTDIR)\mutex.obj"
	-@erase "$(INTDIR)\mutex.sbr"
	-@erase "$(INTDIR)\node_name.obj"
	-@erase "$(INTDIR)\node_name.sbr"
	-@erase "$(INTDIR)\node_rank.obj"
	-@erase "$(INTDIR)\node_rank.sbr"
	-@erase "$(INTDIR)\node_size.obj"
	-@erase "$(INTDIR)\node_size.sbr"
	-@erase "$(INTDIR)\page_size.obj"
	-@erase "$(INTDIR)\page_size.sbr"
	-@erase "$(INTDIR)\print_regions.obj"
	-@erase "$(INTDIR)\print_regions.sbr"
	-@erase "$(INTDIR)\proc_rank.obj"
	-@erase "$(INTDIR)\proc_rank.sbr"
	-@erase "$(INTDIR)\proc_size.obj"
	-@erase "$(INTDIR)\proc_size.sbr"
	-@erase "$(INTDIR)\proc_to_node.obj"
	-@erase "$(INTDIR)\proc_to_node.sbr"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\progress.sbr"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\pthread.sbr"
	-@erase "$(INTDIR)\putget.obj"
	-@erase "$(INTDIR)\putget.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\redirect_io.obj"
	-@erase "$(INTDIR)\redirect_io.sbr"
	-@erase "$(INTDIR)\region_layout.obj"
	-@erase "$(INTDIR)\region_layout.sbr"
	-@erase "$(INTDIR)\resource_list.obj"
	-@erase "$(INTDIR)\resource_list.sbr"
	-@erase "$(INTDIR)\safety.obj"
	-@erase "$(INTDIR)\safety.sbr"
	-@erase "$(INTDIR)\salloc.obj"
	-@erase "$(INTDIR)\salloc.sbr"
	-@erase "$(INTDIR)\sci_desc.obj"
	-@erase "$(INTDIR)\sci_desc.sbr"
	-@erase "$(INTDIR)\sci_shmem.obj"
	-@erase "$(INTDIR)\sci_shmem.sbr"
	-@erase "$(INTDIR)\sciflush.obj"
	-@erase "$(INTDIR)\sciflush.sbr"
	-@erase "$(INTDIR)\scistartup.obj"
	-@erase "$(INTDIR)\scistartup.sbr"
	-@erase "$(INTDIR)\segment_address.obj"
	-@erase "$(INTDIR)\segment_address.sbr"
	-@erase "$(INTDIR)\setup.obj"
	-@erase "$(INTDIR)\setup.sbr"
	-@erase "$(INTDIR)\sfree.obj"
	-@erase "$(INTDIR)\sfree.sbr"
	-@erase "$(INTDIR)\Shm.obj"
	-@erase "$(INTDIR)\Shm.sbr"
	-@erase "$(INTDIR)\shmem.obj"
	-@erase "$(INTDIR)\shmem.sbr"
	-@erase "$(INTDIR)\shseg_key.obj"
	-@erase "$(INTDIR)\shseg_key.sbr"
	-@erase "$(INTDIR)\signalization.obj"
	-@erase "$(INTDIR)\signalization.sbr"
	-@erase "$(INTDIR)\sinit.obj"
	-@erase "$(INTDIR)\sinit.sbr"
	-@erase "$(INTDIR)\sisci_memcpy.obj"
	-@erase "$(INTDIR)\sisci_memcpy.sbr"
	-@erase "$(INTDIR)\smi_fifo.obj"
	-@erase "$(INTDIR)\smi_fifo.sbr"
	-@erase "$(INTDIR)\smi_finalize.obj"
	-@erase "$(INTDIR)\smi_finalize.sbr"
	-@erase "$(INTDIR)\smi_init.obj"
	-@erase "$(INTDIR)\smi_init.sbr"
	-@erase "$(INTDIR)\smi_time.obj"
	-@erase "$(INTDIR)\smi_time.sbr"
	-@erase "$(INTDIR)\smibarrier.obj"
	-@erase "$(INTDIR)\smibarrier.sbr"
	-@erase "$(INTDIR)\smisendrecv.obj"
	-@erase "$(INTDIR)\smisendrecv.sbr"
	-@erase "$(INTDIR)\smpstartup.obj"
	-@erase "$(INTDIR)\smpstartup.sbr"
	-@erase "$(INTDIR)\startup.obj"
	-@erase "$(INTDIR)\startup.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\store_barrier.obj"
	-@erase "$(INTDIR)\store_barrier.sbr"
	-@erase "$(INTDIR)\sync_finalize.obj"
	-@erase "$(INTDIR)\sync_finalize.sbr"
	-@erase "$(INTDIR)\sync_init.obj"
	-@erase "$(INTDIR)\sync_init.sbr"
	-@erase "$(INTDIR)\tcpsync.obj"
	-@erase "$(INTDIR)\tcpsync.sbr"
	-@erase "$(INTDIR)\time.obj"
	-@erase "$(INTDIR)\time.sbr"
	-@erase "$(INTDIR)\unistd.obj"
	-@erase "$(INTDIR)\unistd.sbr"
	-@erase "$(INTDIR)\unix_shmem.obj"
	-@erase "$(INTDIR)\unix_shmem.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchdog.obj"
	-@erase "$(INTDIR)\watchdog.sbr"
	-@erase "$(INTDIR)\wc_memcpy_c.obj"
	-@erase "$(INTDIR)\wc_memcpy_c.sbr"
	-@erase "$(OUTDIR)\csmi.lib"
	-@erase "$(OUTDIR)\smi.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

LINK32=link.exe
MTL=midl.exe
F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /MT /GX /Z7 /O2 /I "include" /I "$(DOLPHIN_BASE)\include" /I "src\include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "SMI_NOCPP" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\smi.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\address_to_region.sbr" \
	"$(INTDIR)\connect_shreg.sbr" \
	"$(INTDIR)\cpuid.sbr" \
	"$(INTDIR)\create_shreg.sbr" \
	"$(INTDIR)\dyn_mem.sbr" \
	"$(INTDIR)\error_count.sbr" \
	"$(INTDIR)\first_proc_on_node.sbr" \
	"$(INTDIR)\free_shreg.sbr" \
	"$(INTDIR)\general.sbr" \
	"$(INTDIR)\general_definitions.sbr" \
	"$(INTDIR)\getopt.sbr" \
	"$(INTDIR)\idstack.sbr" \
	"$(INTDIR)\internal_regions.sbr" \
	"$(INTDIR)\local_seg.sbr" \
	"$(INTDIR)\lowlevelmp.sbr" \
	"$(INTDIR)\memcpy.sbr" \
	"$(INTDIR)\memcpy_base.sbr" \
	"$(INTDIR)\memtree.sbr" \
	"$(INTDIR)\mmx_memcpy_win.sbr" \
	"$(INTDIR)\mutex.sbr" \
	"$(INTDIR)\node_name.sbr" \
	"$(INTDIR)\node_rank.sbr" \
	"$(INTDIR)\node_size.sbr" \
	"$(INTDIR)\page_size.sbr" \
	"$(INTDIR)\print_regions.sbr" \
	"$(INTDIR)\proc_rank.sbr" \
	"$(INTDIR)\proc_size.sbr" \
	"$(INTDIR)\proc_to_node.sbr" \
	"$(INTDIR)\progress.sbr" \
	"$(INTDIR)\pthread.sbr" \
	"$(INTDIR)\putget.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\redirect_io.sbr" \
	"$(INTDIR)\region_layout.sbr" \
	"$(INTDIR)\resource_list.sbr" \
	"$(INTDIR)\safety.sbr" \
	"$(INTDIR)\salloc.sbr" \
	"$(INTDIR)\sci_desc.sbr" \
	"$(INTDIR)\sci_shmem.sbr" \
	"$(INTDIR)\sciflush.sbr" \
	"$(INTDIR)\scistartup.sbr" \
	"$(INTDIR)\segment_address.sbr" \
	"$(INTDIR)\setup.sbr" \
	"$(INTDIR)\sfree.sbr" \
	"$(INTDIR)\Shm.sbr" \
	"$(INTDIR)\shmem.sbr" \
	"$(INTDIR)\shseg_key.sbr" \
	"$(INTDIR)\signalization.sbr" \
	"$(INTDIR)\sinit.sbr" \
	"$(INTDIR)\sisci_memcpy.sbr" \
	"$(INTDIR)\smi_fifo.sbr" \
	"$(INTDIR)\smi_finalize.sbr" \
	"$(INTDIR)\smi_init.sbr" \
	"$(INTDIR)\smi_time.sbr" \
	"$(INTDIR)\smibarrier.sbr" \
	"$(INTDIR)\smisendrecv.sbr" \
	"$(INTDIR)\smpstartup.sbr" \
	"$(INTDIR)\startup.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\store_barrier.sbr" \
	"$(INTDIR)\sync_finalize.sbr" \
	"$(INTDIR)\sync_init.sbr" \
	"$(INTDIR)\tcpsync.sbr" \
	"$(INTDIR)\time.sbr" \
	"$(INTDIR)\unistd.sbr" \
	"$(INTDIR)\unix_shmem.sbr" \
	"$(INTDIR)\watchdog.sbr" \
	"$(INTDIR)\wc_memcpy_c.sbr"

"$(OUTDIR)\smi.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\csmi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\address_to_region.obj" \
	"$(INTDIR)\connect_shreg.obj" \
	"$(INTDIR)\cpuid.obj" \
	"$(INTDIR)\create_shreg.obj" \
	"$(INTDIR)\dyn_mem.obj" \
	"$(INTDIR)\error_count.obj" \
	"$(INTDIR)\first_proc_on_node.obj" \
	"$(INTDIR)\free_shreg.obj" \
	"$(INTDIR)\general.obj" \
	"$(INTDIR)\general_definitions.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\idstack.obj" \
	"$(INTDIR)\internal_regions.obj" \
	"$(INTDIR)\local_seg.obj" \
	"$(INTDIR)\lowlevelmp.obj" \
	"$(INTDIR)\memcpy.obj" \
	"$(INTDIR)\memcpy_base.obj" \
	"$(INTDIR)\memtree.obj" \
	"$(INTDIR)\mmx_memcpy_win.obj" \
	"$(INTDIR)\mutex.obj" \
	"$(INTDIR)\node_name.obj" \
	"$(INTDIR)\node_rank.obj" \
	"$(INTDIR)\node_size.obj" \
	"$(INTDIR)\page_size.obj" \
	"$(INTDIR)\print_regions.obj" \
	"$(INTDIR)\proc_rank.obj" \
	"$(INTDIR)\proc_size.obj" \
	"$(INTDIR)\proc_to_node.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\putget.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\redirect_io.obj" \
	"$(INTDIR)\region_layout.obj" \
	"$(INTDIR)\resource_list.obj" \
	"$(INTDIR)\safety.obj" \
	"$(INTDIR)\salloc.obj" \
	"$(INTDIR)\sci_desc.obj" \
	"$(INTDIR)\sci_shmem.obj" \
	"$(INTDIR)\sciflush.obj" \
	"$(INTDIR)\scistartup.obj" \
	"$(INTDIR)\segment_address.obj" \
	"$(INTDIR)\setup.obj" \
	"$(INTDIR)\sfree.obj" \
	"$(INTDIR)\Shm.obj" \
	"$(INTDIR)\shmem.obj" \
	"$(INTDIR)\shseg_key.obj" \
	"$(INTDIR)\signalization.obj" \
	"$(INTDIR)\sinit.obj" \
	"$(INTDIR)\sisci_memcpy.obj" \
	"$(INTDIR)\smi_fifo.obj" \
	"$(INTDIR)\smi_finalize.obj" \
	"$(INTDIR)\smi_init.obj" \
	"$(INTDIR)\smi_time.obj" \
	"$(INTDIR)\smibarrier.obj" \
	"$(INTDIR)\smisendrecv.obj" \
	"$(INTDIR)\smpstartup.obj" \
	"$(INTDIR)\startup.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\store_barrier.obj" \
	"$(INTDIR)\sync_finalize.obj" \
	"$(INTDIR)\sync_init.obj" \
	"$(INTDIR)\tcpsync.obj" \
	"$(INTDIR)\time.obj" \
	"$(INTDIR)\unistd.obj" \
	"$(INTDIR)\unix_shmem.obj" \
	"$(INTDIR)\watchdog.obj" \
	"$(INTDIR)\wc_memcpy_c.obj"

"$(OUTDIR)\csmi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

OUTDIR=.\smi___Win32_Debug
INTDIR=.\smi___Win32_Debug
# Begin Custom Macros
OutDir=.\smi___Win32_Debug
# End Custom Macros

ALL : ".\lib\smi.lib" "$(OUTDIR)\smi.bsc"


CLEAN :
	-@erase "$(INTDIR)\address_to_region.obj"
	-@erase "$(INTDIR)\address_to_region.sbr"
	-@erase "$(INTDIR)\combine_add.obj"
	-@erase "$(INTDIR)\combine_add.sbr"
	-@erase "$(INTDIR)\connect_shreg.obj"
	-@erase "$(INTDIR)\connect_shreg.sbr"
	-@erase "$(INTDIR)\copy.obj"
	-@erase "$(INTDIR)\copy.sbr"
	-@erase "$(INTDIR)\copy_every_local.obj"
	-@erase "$(INTDIR)\copy_every_local.sbr"
	-@erase "$(INTDIR)\copy_gl_dist_local.obj"
	-@erase "$(INTDIR)\copy_gl_dist_local.sbr"
	-@erase "$(INTDIR)\cpuid.obj"
	-@erase "$(INTDIR)\cpuid.sbr"
	-@erase "$(INTDIR)\create_shreg.obj"
	-@erase "$(INTDIR)\create_shreg.sbr"
	-@erase "$(INTDIR)\dyn_mem.obj"
	-@erase "$(INTDIR)\dyn_mem.sbr"
	-@erase "$(INTDIR)\ensure_consistency.obj"
	-@erase "$(INTDIR)\ensure_consistency.sbr"
	-@erase "$(INTDIR)\err.obj"
	-@erase "$(INTDIR)\err.sbr"
	-@erase "$(INTDIR)\error_count.obj"
	-@erase "$(INTDIR)\error_count.sbr"
	-@erase "$(INTDIR)\first_proc_on_node.obj"
	-@erase "$(INTDIR)\first_proc_on_node.sbr"
	-@erase "$(INTDIR)\fortran_binding.obj"
	-@erase "$(INTDIR)\fortran_binding.sbr"
	-@erase "$(INTDIR)\free_shreg.obj"
	-@erase "$(INTDIR)\free_shreg.sbr"
	-@erase "$(INTDIR)\general.obj"
	-@erase "$(INTDIR)\general.sbr"
	-@erase "$(INTDIR)\general_definitions.obj"
	-@erase "$(INTDIR)\general_definitions.sbr"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt.sbr"
	-@erase "$(INTDIR)\idstack.obj"
	-@erase "$(INTDIR)\idstack.sbr"
	-@erase "$(INTDIR)\init_switching.obj"
	-@erase "$(INTDIR)\init_switching.sbr"
	-@erase "$(INTDIR)\internal_regions.obj"
	-@erase "$(INTDIR)\internal_regions.sbr"
	-@erase "$(INTDIR)\local_seg.obj"
	-@erase "$(INTDIR)\local_seg.sbr"
	-@erase "$(INTDIR)\loop.obj"
	-@erase "$(INTDIR)\loop.sbr"
	-@erase "$(INTDIR)\loop_get.obj"
	-@erase "$(INTDIR)\loop_get.sbr"
	-@erase "$(INTDIR)\loop_interface.obj"
	-@erase "$(INTDIR)\loop_interface.sbr"
	-@erase "$(INTDIR)\loop_partition.obj"
	-@erase "$(INTDIR)\loop_partition.sbr"
	-@erase "$(INTDIR)\loop_split.obj"
	-@erase "$(INTDIR)\loop_split.sbr"
	-@erase "$(INTDIR)\loop_static.obj"
	-@erase "$(INTDIR)\loop_static.sbr"
	-@erase "$(INTDIR)\lowlevelmp.obj"
	-@erase "$(INTDIR)\lowlevelmp.sbr"
	-@erase "$(INTDIR)\memcpy.obj"
	-@erase "$(INTDIR)\memcpy.sbr"
	-@erase "$(INTDIR)\memcpy_base.obj"
	-@erase "$(INTDIR)\memcpy_base.sbr"
	-@erase "$(INTDIR)\memtree.obj"
	-@erase "$(INTDIR)\memtree.sbr"
	-@erase "$(INTDIR)\mmx_memcpy_win.obj"
	-@erase "$(INTDIR)\mmx_memcpy_win.sbr"
	-@erase "$(INTDIR)\mutex.obj"
	-@erase "$(INTDIR)\mutex.sbr"
	-@erase "$(INTDIR)\node_name.obj"
	-@erase "$(INTDIR)\node_name.sbr"
	-@erase "$(INTDIR)\node_rank.obj"
	-@erase "$(INTDIR)\node_rank.sbr"
	-@erase "$(INTDIR)\node_size.obj"
	-@erase "$(INTDIR)\node_size.sbr"
	-@erase "$(INTDIR)\page_size.obj"
	-@erase "$(INTDIR)\page_size.sbr"
	-@erase "$(INTDIR)\print_regions.obj"
	-@erase "$(INTDIR)\print_regions.sbr"
	-@erase "$(INTDIR)\proc_rank.obj"
	-@erase "$(INTDIR)\proc_rank.sbr"
	-@erase "$(INTDIR)\proc_size.obj"
	-@erase "$(INTDIR)\proc_size.sbr"
	-@erase "$(INTDIR)\proc_to_node.obj"
	-@erase "$(INTDIR)\proc_to_node.sbr"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\progress.sbr"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\pthread.sbr"
	-@erase "$(INTDIR)\putget.obj"
	-@erase "$(INTDIR)\putget.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\redirect_io.obj"
	-@erase "$(INTDIR)\redirect_io.sbr"
	-@erase "$(INTDIR)\region_layout.obj"
	-@erase "$(INTDIR)\region_layout.sbr"
	-@erase "$(INTDIR)\resource_list.obj"
	-@erase "$(INTDIR)\resource_list.sbr"
	-@erase "$(INTDIR)\safety.obj"
	-@erase "$(INTDIR)\safety.sbr"
	-@erase "$(INTDIR)\salloc.obj"
	-@erase "$(INTDIR)\salloc.sbr"
	-@erase "$(INTDIR)\sci_desc.obj"
	-@erase "$(INTDIR)\sci_desc.sbr"
	-@erase "$(INTDIR)\sci_shmem.obj"
	-@erase "$(INTDIR)\sci_shmem.sbr"
	-@erase "$(INTDIR)\sciflush.obj"
	-@erase "$(INTDIR)\sciflush.sbr"
	-@erase "$(INTDIR)\scistartup.obj"
	-@erase "$(INTDIR)\scistartup.sbr"
	-@erase "$(INTDIR)\segment_address.obj"
	-@erase "$(INTDIR)\segment_address.sbr"
	-@erase "$(INTDIR)\setup.obj"
	-@erase "$(INTDIR)\setup.sbr"
	-@erase "$(INTDIR)\sfree.obj"
	-@erase "$(INTDIR)\sfree.sbr"
	-@erase "$(INTDIR)\Shm.obj"
	-@erase "$(INTDIR)\Shm.sbr"
	-@erase "$(INTDIR)\shmem.obj"
	-@erase "$(INTDIR)\shmem.sbr"
	-@erase "$(INTDIR)\shseg_key.obj"
	-@erase "$(INTDIR)\shseg_key.sbr"
	-@erase "$(INTDIR)\signalization.obj"
	-@erase "$(INTDIR)\signalization.sbr"
	-@erase "$(INTDIR)\sinit.obj"
	-@erase "$(INTDIR)\sinit.sbr"
	-@erase "$(INTDIR)\sisci_memcpy.obj"
	-@erase "$(INTDIR)\sisci_memcpy.sbr"
	-@erase "$(INTDIR)\smi_fifo.obj"
	-@erase "$(INTDIR)\smi_fifo.sbr"
	-@erase "$(INTDIR)\smi_finalize.obj"
	-@erase "$(INTDIR)\smi_finalize.sbr"
	-@erase "$(INTDIR)\smi_init.obj"
	-@erase "$(INTDIR)\smi_init.sbr"
	-@erase "$(INTDIR)\smi_time.obj"
	-@erase "$(INTDIR)\smi_time.sbr"
	-@erase "$(INTDIR)\smibarrier.obj"
	-@erase "$(INTDIR)\smibarrier.sbr"
	-@erase "$(INTDIR)\smisendrecv.obj"
	-@erase "$(INTDIR)\smisendrecv.sbr"
	-@erase "$(INTDIR)\smpstartup.obj"
	-@erase "$(INTDIR)\smpstartup.sbr"
	-@erase "$(INTDIR)\startup.obj"
	-@erase "$(INTDIR)\startup.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\store_barrier.obj"
	-@erase "$(INTDIR)\store_barrier.sbr"
	-@erase "$(INTDIR)\switch_to_replication.obj"
	-@erase "$(INTDIR)\switch_to_replication.sbr"
	-@erase "$(INTDIR)\switch_to_replication_fast.obj"
	-@erase "$(INTDIR)\switch_to_replication_fast.sbr"
	-@erase "$(INTDIR)\switch_to_sharing.obj"
	-@erase "$(INTDIR)\switch_to_sharing.sbr"
	-@erase "$(INTDIR)\switch_to_sharing_fast.obj"
	-@erase "$(INTDIR)\switch_to_sharing_fast.sbr"
	-@erase "$(INTDIR)\sync_finalize.obj"
	-@erase "$(INTDIR)\sync_finalize.sbr"
	-@erase "$(INTDIR)\sync_init.obj"
	-@erase "$(INTDIR)\sync_init.sbr"
	-@erase "$(INTDIR)\tcpsync.obj"
	-@erase "$(INTDIR)\tcpsync.sbr"
	-@erase "$(INTDIR)\time.obj"
	-@erase "$(INTDIR)\time.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\unistd.obj"
	-@erase "$(INTDIR)\unistd.sbr"
	-@erase "$(INTDIR)\unix_shmem.obj"
	-@erase "$(INTDIR)\unix_shmem.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchdog.obj"
	-@erase "$(INTDIR)\watchdog.sbr"
	-@erase "$(INTDIR)\wc_memcpy_c.obj"
	-@erase "$(INTDIR)\wc_memcpy_c.sbr"
	-@erase "$(OUTDIR)\smi.bsc"
	-@erase ".\lib\smi.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

LINK32=link.exe
MTL=midl.exe
F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /MDd /GX /Z7 /Od /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\smi.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\address_to_region.sbr" \
	"$(INTDIR)\combine_add.sbr" \
	"$(INTDIR)\connect_shreg.sbr" \
	"$(INTDIR)\copy.sbr" \
	"$(INTDIR)\copy_every_local.sbr" \
	"$(INTDIR)\copy_gl_dist_local.sbr" \
	"$(INTDIR)\cpuid.sbr" \
	"$(INTDIR)\create_shreg.sbr" \
	"$(INTDIR)\dyn_mem.sbr" \
	"$(INTDIR)\ensure_consistency.sbr" \
	"$(INTDIR)\err.sbr" \
	"$(INTDIR)\error_count.sbr" \
	"$(INTDIR)\first_proc_on_node.sbr" \
	"$(INTDIR)\fortran_binding.sbr" \
	"$(INTDIR)\free_shreg.sbr" \
	"$(INTDIR)\general.sbr" \
	"$(INTDIR)\general_definitions.sbr" \
	"$(INTDIR)\getopt.sbr" \
	"$(INTDIR)\idstack.sbr" \
	"$(INTDIR)\init_switching.sbr" \
	"$(INTDIR)\internal_regions.sbr" \
	"$(INTDIR)\local_seg.sbr" \
	"$(INTDIR)\loop.sbr" \
	"$(INTDIR)\loop_get.sbr" \
	"$(INTDIR)\loop_interface.sbr" \
	"$(INTDIR)\loop_partition.sbr" \
	"$(INTDIR)\loop_split.sbr" \
	"$(INTDIR)\loop_static.sbr" \
	"$(INTDIR)\lowlevelmp.sbr" \
	"$(INTDIR)\memcpy.sbr" \
	"$(INTDIR)\memcpy_base.sbr" \
	"$(INTDIR)\memtree.sbr" \
	"$(INTDIR)\mmx_memcpy_win.sbr" \
	"$(INTDIR)\mutex.sbr" \
	"$(INTDIR)\node_name.sbr" \
	"$(INTDIR)\node_rank.sbr" \
	"$(INTDIR)\node_size.sbr" \
	"$(INTDIR)\page_size.sbr" \
	"$(INTDIR)\print_regions.sbr" \
	"$(INTDIR)\proc_rank.sbr" \
	"$(INTDIR)\proc_size.sbr" \
	"$(INTDIR)\proc_to_node.sbr" \
	"$(INTDIR)\progress.sbr" \
	"$(INTDIR)\pthread.sbr" \
	"$(INTDIR)\putget.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\redirect_io.sbr" \
	"$(INTDIR)\region_layout.sbr" \
	"$(INTDIR)\resource_list.sbr" \
	"$(INTDIR)\safety.sbr" \
	"$(INTDIR)\salloc.sbr" \
	"$(INTDIR)\sci_desc.sbr" \
	"$(INTDIR)\sci_shmem.sbr" \
	"$(INTDIR)\sciflush.sbr" \
	"$(INTDIR)\scistartup.sbr" \
	"$(INTDIR)\segment_address.sbr" \
	"$(INTDIR)\setup.sbr" \
	"$(INTDIR)\sfree.sbr" \
	"$(INTDIR)\Shm.sbr" \
	"$(INTDIR)\shmem.sbr" \
	"$(INTDIR)\shseg_key.sbr" \
	"$(INTDIR)\signalization.sbr" \
	"$(INTDIR)\sinit.sbr" \
	"$(INTDIR)\sisci_memcpy.sbr" \
	"$(INTDIR)\smi_fifo.sbr" \
	"$(INTDIR)\smi_finalize.sbr" \
	"$(INTDIR)\smi_init.sbr" \
	"$(INTDIR)\smi_time.sbr" \
	"$(INTDIR)\smibarrier.sbr" \
	"$(INTDIR)\smisendrecv.sbr" \
	"$(INTDIR)\smpstartup.sbr" \
	"$(INTDIR)\startup.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\store_barrier.sbr" \
	"$(INTDIR)\switch_to_replication.sbr" \
	"$(INTDIR)\switch_to_replication_fast.sbr" \
	"$(INTDIR)\switch_to_sharing.sbr" \
	"$(INTDIR)\switch_to_sharing_fast.sbr" \
	"$(INTDIR)\sync_finalize.sbr" \
	"$(INTDIR)\sync_init.sbr" \
	"$(INTDIR)\tcpsync.sbr" \
	"$(INTDIR)\time.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\unistd.sbr" \
	"$(INTDIR)\unix_shmem.sbr" \
	"$(INTDIR)\watchdog.sbr" \
	"$(INTDIR)\wc_memcpy_c.sbr"

"$(OUTDIR)\smi.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\smi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\address_to_region.obj" \
	"$(INTDIR)\combine_add.obj" \
	"$(INTDIR)\connect_shreg.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\copy_every_local.obj" \
	"$(INTDIR)\copy_gl_dist_local.obj" \
	"$(INTDIR)\cpuid.obj" \
	"$(INTDIR)\create_shreg.obj" \
	"$(INTDIR)\dyn_mem.obj" \
	"$(INTDIR)\ensure_consistency.obj" \
	"$(INTDIR)\err.obj" \
	"$(INTDIR)\error_count.obj" \
	"$(INTDIR)\first_proc_on_node.obj" \
	"$(INTDIR)\fortran_binding.obj" \
	"$(INTDIR)\free_shreg.obj" \
	"$(INTDIR)\general.obj" \
	"$(INTDIR)\general_definitions.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\idstack.obj" \
	"$(INTDIR)\init_switching.obj" \
	"$(INTDIR)\internal_regions.obj" \
	"$(INTDIR)\local_seg.obj" \
	"$(INTDIR)\loop.obj" \
	"$(INTDIR)\loop_get.obj" \
	"$(INTDIR)\loop_interface.obj" \
	"$(INTDIR)\loop_partition.obj" \
	"$(INTDIR)\loop_split.obj" \
	"$(INTDIR)\loop_static.obj" \
	"$(INTDIR)\lowlevelmp.obj" \
	"$(INTDIR)\memcpy.obj" \
	"$(INTDIR)\memcpy_base.obj" \
	"$(INTDIR)\memtree.obj" \
	"$(INTDIR)\mmx_memcpy_win.obj" \
	"$(INTDIR)\mutex.obj" \
	"$(INTDIR)\node_name.obj" \
	"$(INTDIR)\node_rank.obj" \
	"$(INTDIR)\node_size.obj" \
	"$(INTDIR)\page_size.obj" \
	"$(INTDIR)\print_regions.obj" \
	"$(INTDIR)\proc_rank.obj" \
	"$(INTDIR)\proc_size.obj" \
	"$(INTDIR)\proc_to_node.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\putget.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\redirect_io.obj" \
	"$(INTDIR)\region_layout.obj" \
	"$(INTDIR)\resource_list.obj" \
	"$(INTDIR)\safety.obj" \
	"$(INTDIR)\salloc.obj" \
	"$(INTDIR)\sci_desc.obj" \
	"$(INTDIR)\sci_shmem.obj" \
	"$(INTDIR)\sciflush.obj" \
	"$(INTDIR)\scistartup.obj" \
	"$(INTDIR)\segment_address.obj" \
	"$(INTDIR)\setup.obj" \
	"$(INTDIR)\sfree.obj" \
	"$(INTDIR)\Shm.obj" \
	"$(INTDIR)\shmem.obj" \
	"$(INTDIR)\shseg_key.obj" \
	"$(INTDIR)\signalization.obj" \
	"$(INTDIR)\sinit.obj" \
	"$(INTDIR)\sisci_memcpy.obj" \
	"$(INTDIR)\smi_fifo.obj" \
	"$(INTDIR)\smi_finalize.obj" \
	"$(INTDIR)\smi_init.obj" \
	"$(INTDIR)\smi_time.obj" \
	"$(INTDIR)\smibarrier.obj" \
	"$(INTDIR)\smisendrecv.obj" \
	"$(INTDIR)\smpstartup.obj" \
	"$(INTDIR)\startup.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\store_barrier.obj" \
	"$(INTDIR)\switch_to_replication.obj" \
	"$(INTDIR)\switch_to_replication_fast.obj" \
	"$(INTDIR)\switch_to_sharing.obj" \
	"$(INTDIR)\switch_to_sharing_fast.obj" \
	"$(INTDIR)\sync_finalize.obj" \
	"$(INTDIR)\sync_init.obj" \
	"$(INTDIR)\tcpsync.obj" \
	"$(INTDIR)\time.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\unistd.obj" \
	"$(INTDIR)\unix_shmem.obj" \
	"$(INTDIR)\watchdog.obj" \
	"$(INTDIR)\wc_memcpy_c.obj"

".\lib\smi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

OUTDIR=.\smi___Win32_Release
INTDIR=.\smi___Win32_Release
# Begin Custom Macros
OutDir=.\smi___Win32_Release
# End Custom Macros

ALL : ".\lib\smi.lib" "$(OUTDIR)\smi.bsc"


CLEAN :
	-@erase "$(INTDIR)\address_to_region.obj"
	-@erase "$(INTDIR)\address_to_region.sbr"
	-@erase "$(INTDIR)\combine_add.obj"
	-@erase "$(INTDIR)\combine_add.sbr"
	-@erase "$(INTDIR)\connect_shreg.obj"
	-@erase "$(INTDIR)\connect_shreg.sbr"
	-@erase "$(INTDIR)\copy.obj"
	-@erase "$(INTDIR)\copy.sbr"
	-@erase "$(INTDIR)\copy_every_local.obj"
	-@erase "$(INTDIR)\copy_every_local.sbr"
	-@erase "$(INTDIR)\copy_gl_dist_local.obj"
	-@erase "$(INTDIR)\copy_gl_dist_local.sbr"
	-@erase "$(INTDIR)\cpuid.obj"
	-@erase "$(INTDIR)\cpuid.sbr"
	-@erase "$(INTDIR)\create_shreg.obj"
	-@erase "$(INTDIR)\create_shreg.sbr"
	-@erase "$(INTDIR)\dyn_mem.obj"
	-@erase "$(INTDIR)\dyn_mem.sbr"
	-@erase "$(INTDIR)\ensure_consistency.obj"
	-@erase "$(INTDIR)\ensure_consistency.sbr"
	-@erase "$(INTDIR)\err.obj"
	-@erase "$(INTDIR)\err.sbr"
	-@erase "$(INTDIR)\error_count.obj"
	-@erase "$(INTDIR)\error_count.sbr"
	-@erase "$(INTDIR)\first_proc_on_node.obj"
	-@erase "$(INTDIR)\first_proc_on_node.sbr"
	-@erase "$(INTDIR)\fortran_binding.obj"
	-@erase "$(INTDIR)\fortran_binding.sbr"
	-@erase "$(INTDIR)\free_shreg.obj"
	-@erase "$(INTDIR)\free_shreg.sbr"
	-@erase "$(INTDIR)\general.obj"
	-@erase "$(INTDIR)\general.sbr"
	-@erase "$(INTDIR)\general_definitions.obj"
	-@erase "$(INTDIR)\general_definitions.sbr"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt.sbr"
	-@erase "$(INTDIR)\idstack.obj"
	-@erase "$(INTDIR)\idstack.sbr"
	-@erase "$(INTDIR)\init_switching.obj"
	-@erase "$(INTDIR)\init_switching.sbr"
	-@erase "$(INTDIR)\internal_regions.obj"
	-@erase "$(INTDIR)\internal_regions.sbr"
	-@erase "$(INTDIR)\local_seg.obj"
	-@erase "$(INTDIR)\local_seg.sbr"
	-@erase "$(INTDIR)\loop.obj"
	-@erase "$(INTDIR)\loop.sbr"
	-@erase "$(INTDIR)\loop_get.obj"
	-@erase "$(INTDIR)\loop_get.sbr"
	-@erase "$(INTDIR)\loop_interface.obj"
	-@erase "$(INTDIR)\loop_interface.sbr"
	-@erase "$(INTDIR)\loop_partition.obj"
	-@erase "$(INTDIR)\loop_partition.sbr"
	-@erase "$(INTDIR)\loop_split.obj"
	-@erase "$(INTDIR)\loop_split.sbr"
	-@erase "$(INTDIR)\loop_static.obj"
	-@erase "$(INTDIR)\loop_static.sbr"
	-@erase "$(INTDIR)\lowlevelmp.obj"
	-@erase "$(INTDIR)\lowlevelmp.sbr"
	-@erase "$(INTDIR)\memcpy.obj"
	-@erase "$(INTDIR)\memcpy.sbr"
	-@erase "$(INTDIR)\memcpy_base.obj"
	-@erase "$(INTDIR)\memcpy_base.sbr"
	-@erase "$(INTDIR)\memtree.obj"
	-@erase "$(INTDIR)\memtree.sbr"
	-@erase "$(INTDIR)\mmx_memcpy_win.obj"
	-@erase "$(INTDIR)\mmx_memcpy_win.sbr"
	-@erase "$(INTDIR)\mutex.obj"
	-@erase "$(INTDIR)\mutex.sbr"
	-@erase "$(INTDIR)\node_name.obj"
	-@erase "$(INTDIR)\node_name.sbr"
	-@erase "$(INTDIR)\node_rank.obj"
	-@erase "$(INTDIR)\node_rank.sbr"
	-@erase "$(INTDIR)\node_size.obj"
	-@erase "$(INTDIR)\node_size.sbr"
	-@erase "$(INTDIR)\page_size.obj"
	-@erase "$(INTDIR)\page_size.sbr"
	-@erase "$(INTDIR)\print_regions.obj"
	-@erase "$(INTDIR)\print_regions.sbr"
	-@erase "$(INTDIR)\proc_rank.obj"
	-@erase "$(INTDIR)\proc_rank.sbr"
	-@erase "$(INTDIR)\proc_size.obj"
	-@erase "$(INTDIR)\proc_size.sbr"
	-@erase "$(INTDIR)\proc_to_node.obj"
	-@erase "$(INTDIR)\proc_to_node.sbr"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\progress.sbr"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\pthread.sbr"
	-@erase "$(INTDIR)\putget.obj"
	-@erase "$(INTDIR)\putget.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\redirect_io.obj"
	-@erase "$(INTDIR)\redirect_io.sbr"
	-@erase "$(INTDIR)\region_layout.obj"
	-@erase "$(INTDIR)\region_layout.sbr"
	-@erase "$(INTDIR)\resource_list.obj"
	-@erase "$(INTDIR)\resource_list.sbr"
	-@erase "$(INTDIR)\safety.obj"
	-@erase "$(INTDIR)\safety.sbr"
	-@erase "$(INTDIR)\salloc.obj"
	-@erase "$(INTDIR)\salloc.sbr"
	-@erase "$(INTDIR)\sci_desc.obj"
	-@erase "$(INTDIR)\sci_desc.sbr"
	-@erase "$(INTDIR)\sci_shmem.obj"
	-@erase "$(INTDIR)\sci_shmem.sbr"
	-@erase "$(INTDIR)\sciflush.obj"
	-@erase "$(INTDIR)\sciflush.sbr"
	-@erase "$(INTDIR)\scistartup.obj"
	-@erase "$(INTDIR)\scistartup.sbr"
	-@erase "$(INTDIR)\segment_address.obj"
	-@erase "$(INTDIR)\segment_address.sbr"
	-@erase "$(INTDIR)\setup.obj"
	-@erase "$(INTDIR)\setup.sbr"
	-@erase "$(INTDIR)\sfree.obj"
	-@erase "$(INTDIR)\sfree.sbr"
	-@erase "$(INTDIR)\Shm.obj"
	-@erase "$(INTDIR)\Shm.sbr"
	-@erase "$(INTDIR)\shmem.obj"
	-@erase "$(INTDIR)\shmem.sbr"
	-@erase "$(INTDIR)\shseg_key.obj"
	-@erase "$(INTDIR)\shseg_key.sbr"
	-@erase "$(INTDIR)\signalization.obj"
	-@erase "$(INTDIR)\signalization.sbr"
	-@erase "$(INTDIR)\sinit.obj"
	-@erase "$(INTDIR)\sinit.sbr"
	-@erase "$(INTDIR)\sisci_memcpy.obj"
	-@erase "$(INTDIR)\sisci_memcpy.sbr"
	-@erase "$(INTDIR)\smi_fifo.obj"
	-@erase "$(INTDIR)\smi_fifo.sbr"
	-@erase "$(INTDIR)\smi_finalize.obj"
	-@erase "$(INTDIR)\smi_finalize.sbr"
	-@erase "$(INTDIR)\smi_init.obj"
	-@erase "$(INTDIR)\smi_init.sbr"
	-@erase "$(INTDIR)\smi_time.obj"
	-@erase "$(INTDIR)\smi_time.sbr"
	-@erase "$(INTDIR)\smibarrier.obj"
	-@erase "$(INTDIR)\smibarrier.sbr"
	-@erase "$(INTDIR)\smisendrecv.obj"
	-@erase "$(INTDIR)\smisendrecv.sbr"
	-@erase "$(INTDIR)\smpstartup.obj"
	-@erase "$(INTDIR)\smpstartup.sbr"
	-@erase "$(INTDIR)\startup.obj"
	-@erase "$(INTDIR)\startup.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\store_barrier.obj"
	-@erase "$(INTDIR)\store_barrier.sbr"
	-@erase "$(INTDIR)\switch_to_replication.obj"
	-@erase "$(INTDIR)\switch_to_replication.sbr"
	-@erase "$(INTDIR)\switch_to_replication_fast.obj"
	-@erase "$(INTDIR)\switch_to_replication_fast.sbr"
	-@erase "$(INTDIR)\switch_to_sharing.obj"
	-@erase "$(INTDIR)\switch_to_sharing.sbr"
	-@erase "$(INTDIR)\switch_to_sharing_fast.obj"
	-@erase "$(INTDIR)\switch_to_sharing_fast.sbr"
	-@erase "$(INTDIR)\sync_finalize.obj"
	-@erase "$(INTDIR)\sync_finalize.sbr"
	-@erase "$(INTDIR)\sync_init.obj"
	-@erase "$(INTDIR)\sync_init.sbr"
	-@erase "$(INTDIR)\tcpsync.obj"
	-@erase "$(INTDIR)\tcpsync.sbr"
	-@erase "$(INTDIR)\time.obj"
	-@erase "$(INTDIR)\time.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\unistd.obj"
	-@erase "$(INTDIR)\unistd.sbr"
	-@erase "$(INTDIR)\unix_shmem.obj"
	-@erase "$(INTDIR)\unix_shmem.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchdog.obj"
	-@erase "$(INTDIR)\watchdog.sbr"
	-@erase "$(INTDIR)\wc_memcpy_c.obj"
	-@erase "$(INTDIR)\wc_memcpy_c.sbr"
	-@erase "$(OUTDIR)\smi.bsc"
	-@erase ".\lib\smi.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

LINK32=link.exe
MTL=midl.exe
F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /MD /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\smi.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\address_to_region.sbr" \
	"$(INTDIR)\combine_add.sbr" \
	"$(INTDIR)\connect_shreg.sbr" \
	"$(INTDIR)\copy.sbr" \
	"$(INTDIR)\copy_every_local.sbr" \
	"$(INTDIR)\copy_gl_dist_local.sbr" \
	"$(INTDIR)\cpuid.sbr" \
	"$(INTDIR)\create_shreg.sbr" \
	"$(INTDIR)\dyn_mem.sbr" \
	"$(INTDIR)\ensure_consistency.sbr" \
	"$(INTDIR)\err.sbr" \
	"$(INTDIR)\error_count.sbr" \
	"$(INTDIR)\first_proc_on_node.sbr" \
	"$(INTDIR)\fortran_binding.sbr" \
	"$(INTDIR)\free_shreg.sbr" \
	"$(INTDIR)\general.sbr" \
	"$(INTDIR)\general_definitions.sbr" \
	"$(INTDIR)\getopt.sbr" \
	"$(INTDIR)\idstack.sbr" \
	"$(INTDIR)\init_switching.sbr" \
	"$(INTDIR)\internal_regions.sbr" \
	"$(INTDIR)\local_seg.sbr" \
	"$(INTDIR)\loop.sbr" \
	"$(INTDIR)\loop_get.sbr" \
	"$(INTDIR)\loop_interface.sbr" \
	"$(INTDIR)\loop_partition.sbr" \
	"$(INTDIR)\loop_split.sbr" \
	"$(INTDIR)\loop_static.sbr" \
	"$(INTDIR)\lowlevelmp.sbr" \
	"$(INTDIR)\memcpy.sbr" \
	"$(INTDIR)\memcpy_base.sbr" \
	"$(INTDIR)\memtree.sbr" \
	"$(INTDIR)\mmx_memcpy_win.sbr" \
	"$(INTDIR)\mutex.sbr" \
	"$(INTDIR)\node_name.sbr" \
	"$(INTDIR)\node_rank.sbr" \
	"$(INTDIR)\node_size.sbr" \
	"$(INTDIR)\page_size.sbr" \
	"$(INTDIR)\print_regions.sbr" \
	"$(INTDIR)\proc_rank.sbr" \
	"$(INTDIR)\proc_size.sbr" \
	"$(INTDIR)\proc_to_node.sbr" \
	"$(INTDIR)\progress.sbr" \
	"$(INTDIR)\pthread.sbr" \
	"$(INTDIR)\putget.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\redirect_io.sbr" \
	"$(INTDIR)\region_layout.sbr" \
	"$(INTDIR)\resource_list.sbr" \
	"$(INTDIR)\safety.sbr" \
	"$(INTDIR)\salloc.sbr" \
	"$(INTDIR)\sci_desc.sbr" \
	"$(INTDIR)\sci_shmem.sbr" \
	"$(INTDIR)\sciflush.sbr" \
	"$(INTDIR)\scistartup.sbr" \
	"$(INTDIR)\segment_address.sbr" \
	"$(INTDIR)\setup.sbr" \
	"$(INTDIR)\sfree.sbr" \
	"$(INTDIR)\Shm.sbr" \
	"$(INTDIR)\shmem.sbr" \
	"$(INTDIR)\shseg_key.sbr" \
	"$(INTDIR)\signalization.sbr" \
	"$(INTDIR)\sinit.sbr" \
	"$(INTDIR)\sisci_memcpy.sbr" \
	"$(INTDIR)\smi_fifo.sbr" \
	"$(INTDIR)\smi_finalize.sbr" \
	"$(INTDIR)\smi_init.sbr" \
	"$(INTDIR)\smi_time.sbr" \
	"$(INTDIR)\smibarrier.sbr" \
	"$(INTDIR)\smisendrecv.sbr" \
	"$(INTDIR)\smpstartup.sbr" \
	"$(INTDIR)\startup.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\store_barrier.sbr" \
	"$(INTDIR)\switch_to_replication.sbr" \
	"$(INTDIR)\switch_to_replication_fast.sbr" \
	"$(INTDIR)\switch_to_sharing.sbr" \
	"$(INTDIR)\switch_to_sharing_fast.sbr" \
	"$(INTDIR)\sync_finalize.sbr" \
	"$(INTDIR)\sync_init.sbr" \
	"$(INTDIR)\tcpsync.sbr" \
	"$(INTDIR)\time.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\unistd.sbr" \
	"$(INTDIR)\unix_shmem.sbr" \
	"$(INTDIR)\watchdog.sbr" \
	"$(INTDIR)\wc_memcpy_c.sbr"

"$(OUTDIR)\smi.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\smi.lib" 
LIB32_OBJS= \
	"$(INTDIR)\address_to_region.obj" \
	"$(INTDIR)\combine_add.obj" \
	"$(INTDIR)\connect_shreg.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\copy_every_local.obj" \
	"$(INTDIR)\copy_gl_dist_local.obj" \
	"$(INTDIR)\cpuid.obj" \
	"$(INTDIR)\create_shreg.obj" \
	"$(INTDIR)\dyn_mem.obj" \
	"$(INTDIR)\ensure_consistency.obj" \
	"$(INTDIR)\err.obj" \
	"$(INTDIR)\error_count.obj" \
	"$(INTDIR)\first_proc_on_node.obj" \
	"$(INTDIR)\fortran_binding.obj" \
	"$(INTDIR)\free_shreg.obj" \
	"$(INTDIR)\general.obj" \
	"$(INTDIR)\general_definitions.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\idstack.obj" \
	"$(INTDIR)\init_switching.obj" \
	"$(INTDIR)\internal_regions.obj" \
	"$(INTDIR)\local_seg.obj" \
	"$(INTDIR)\loop.obj" \
	"$(INTDIR)\loop_get.obj" \
	"$(INTDIR)\loop_interface.obj" \
	"$(INTDIR)\loop_partition.obj" \
	"$(INTDIR)\loop_split.obj" \
	"$(INTDIR)\loop_static.obj" \
	"$(INTDIR)\lowlevelmp.obj" \
	"$(INTDIR)\memcpy.obj" \
	"$(INTDIR)\memcpy_base.obj" \
	"$(INTDIR)\memtree.obj" \
	"$(INTDIR)\mmx_memcpy_win.obj" \
	"$(INTDIR)\mutex.obj" \
	"$(INTDIR)\node_name.obj" \
	"$(INTDIR)\node_rank.obj" \
	"$(INTDIR)\node_size.obj" \
	"$(INTDIR)\page_size.obj" \
	"$(INTDIR)\print_regions.obj" \
	"$(INTDIR)\proc_rank.obj" \
	"$(INTDIR)\proc_size.obj" \
	"$(INTDIR)\proc_to_node.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\putget.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\redirect_io.obj" \
	"$(INTDIR)\region_layout.obj" \
	"$(INTDIR)\resource_list.obj" \
	"$(INTDIR)\safety.obj" \
	"$(INTDIR)\salloc.obj" \
	"$(INTDIR)\sci_desc.obj" \
	"$(INTDIR)\sci_shmem.obj" \
	"$(INTDIR)\sciflush.obj" \
	"$(INTDIR)\scistartup.obj" \
	"$(INTDIR)\segment_address.obj" \
	"$(INTDIR)\setup.obj" \
	"$(INTDIR)\sfree.obj" \
	"$(INTDIR)\Shm.obj" \
	"$(INTDIR)\shmem.obj" \
	"$(INTDIR)\shseg_key.obj" \
	"$(INTDIR)\signalization.obj" \
	"$(INTDIR)\sinit.obj" \
	"$(INTDIR)\sisci_memcpy.obj" \
	"$(INTDIR)\smi_fifo.obj" \
	"$(INTDIR)\smi_finalize.obj" \
	"$(INTDIR)\smi_init.obj" \
	"$(INTDIR)\smi_time.obj" \
	"$(INTDIR)\smibarrier.obj" \
	"$(INTDIR)\smisendrecv.obj" \
	"$(INTDIR)\smpstartup.obj" \
	"$(INTDIR)\startup.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\store_barrier.obj" \
	"$(INTDIR)\switch_to_replication.obj" \
	"$(INTDIR)\switch_to_replication_fast.obj" \
	"$(INTDIR)\switch_to_sharing.obj" \
	"$(INTDIR)\switch_to_sharing_fast.obj" \
	"$(INTDIR)\sync_finalize.obj" \
	"$(INTDIR)\sync_init.obj" \
	"$(INTDIR)\tcpsync.obj" \
	"$(INTDIR)\time.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\unistd.obj" \
	"$(INTDIR)\unix_shmem.obj" \
	"$(INTDIR)\watchdog.obj" \
	"$(INTDIR)\wc_memcpy_c.obj"

".\lib\smi.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("smi.dep")
!INCLUDE "smi.dep"
!ELSE 
!MESSAGE Warning: cannot find "smi.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "smi - Win32 SMP" || "$(CFG)" == "smi - Win32 SCI" || "$(CFG)" == "smi - Win32 SVM" || "$(CFG)" == "smi - Win32 SCI_light" || "$(CFG)" == "smi - Win32 Debug" || "$(CFG)" == "smi - Win32 Release"
SOURCE=.\src\regions\address_to_region.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\address_to_region.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\address_to_region.obj"	"$(INTDIR)\address_to_region.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\address_to_region.obj"	"$(INTDIR)\address_to_region.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\address_to_region.obj"	"$(INTDIR)\address_to_region.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\address_to_region.obj"	"$(INTDIR)\address_to_region.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\address_to_region.obj"	"$(INTDIR)\address_to_region.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\combine_add.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\combine_add.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\combine_add.obj"	"$(INTDIR)\combine_add.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\combine_add.obj"	"$(INTDIR)\combine_add.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\combine_add.obj"	"$(INTDIR)\combine_add.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\combine_add.obj"	"$(INTDIR)\combine_add.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\connect_shreg.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\connect_shreg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\connect_shreg.obj"	"$(INTDIR)\connect_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\connect_shreg.obj"	"$(INTDIR)\connect_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\connect_shreg.obj"	"$(INTDIR)\connect_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\connect_shreg.obj"	"$(INTDIR)\connect_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\connect_shreg.obj"	"$(INTDIR)\connect_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\copy.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\copy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\copy.obj"	"$(INTDIR)\copy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\copy.obj"	"$(INTDIR)\copy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\copy.obj"	"$(INTDIR)\copy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\copy.obj"	"$(INTDIR)\copy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\copy_every_local.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\copy_every_local.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\copy_every_local.obj"	"$(INTDIR)\copy_every_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\copy_every_local.obj"	"$(INTDIR)\copy_every_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\copy_every_local.obj"	"$(INTDIR)\copy_every_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\copy_every_local.obj"	"$(INTDIR)\copy_every_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\copy_gl_dist_local.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\copy_gl_dist_local.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\copy_gl_dist_local.obj"	"$(INTDIR)\copy_gl_dist_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\copy_gl_dist_local.obj"	"$(INTDIR)\copy_gl_dist_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\copy_gl_dist_local.obj"	"$(INTDIR)\copy_gl_dist_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\copy_gl_dist_local.obj"	"$(INTDIR)\copy_gl_dist_local.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\utility\cpuid.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\cpuid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\cpuid.obj"	"$(INTDIR)\cpuid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\cpuid.obj"	"$(INTDIR)\cpuid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\cpuid.obj"	"$(INTDIR)\cpuid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\cpuid.obj"	"$(INTDIR)\cpuid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\cpuid.obj"	"$(INTDIR)\cpuid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\create_shreg.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\create_shreg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\create_shreg.obj"	"$(INTDIR)\create_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\create_shreg.obj"	"$(INTDIR)\create_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\create_shreg.obj"	"$(INTDIR)\create_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\create_shreg.obj"	"$(INTDIR)\create_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\create_shreg.obj"	"$(INTDIR)\create_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\dyn_mem\dyn_mem.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\dyn_mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\dyn_mem.obj"	"$(INTDIR)\dyn_mem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\dyn_mem.obj"	"$(INTDIR)\dyn_mem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\dyn_mem.obj"	"$(INTDIR)\dyn_mem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\dyn_mem.obj"	"$(INTDIR)\dyn_mem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\dyn_mem.obj"	"$(INTDIR)\dyn_mem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\ensure_consistency.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\ensure_consistency.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\ensure_consistency.obj"	"$(INTDIR)\ensure_consistency.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\ensure_consistency.obj"	"$(INTDIR)\ensure_consistency.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\ensure_consistency.obj"	"$(INTDIR)\ensure_consistency.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\ensure_consistency.obj"	"$(INTDIR)\ensure_consistency.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_scheduling\err.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\err.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\err.obj"	"$(INTDIR)\err.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\err.obj"	"$(INTDIR)\err.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\err.obj"	"$(INTDIR)\err.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\err.obj"	"$(INTDIR)\err.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\error_count.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\error_count.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\error_count.obj"	"$(INTDIR)\error_count.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\error_count.obj"	"$(INTDIR)\error_count.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\error_count.obj"	"$(INTDIR)\error_count.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\error_count.obj"	"$(INTDIR)\error_count.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\error_count.obj"	"$(INTDIR)\error_count.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proc_node_numbers\first_proc_on_node.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\first_proc_on_node.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\first_proc_on_node.obj"	"$(INTDIR)\first_proc_on_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\first_proc_on_node.obj"	"$(INTDIR)\first_proc_on_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\first_proc_on_node.obj"	"$(INTDIR)\first_proc_on_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\first_proc_on_node.obj"	"$(INTDIR)\first_proc_on_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\first_proc_on_node.obj"	"$(INTDIR)\first_proc_on_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\fortran_binding.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\fortran_binding.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\fortran_binding.obj"	"$(INTDIR)\fortran_binding.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\fortran_binding.obj"	"$(INTDIR)\fortran_binding.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\fortran_binding.obj"	"$(INTDIR)\fortran_binding.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\fortran_binding.obj"	"$(INTDIR)\fortran_binding.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\free_shreg.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\free_shreg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\free_shreg.obj"	"$(INTDIR)\free_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\free_shreg.obj"	"$(INTDIR)\free_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\free_shreg.obj"	"$(INTDIR)\free_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\free_shreg.obj"	"$(INTDIR)\free_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\free_shreg.obj"	"$(INTDIR)\free_shreg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\utility\general.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\general.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\general.obj"	"$(INTDIR)\general.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\general.obj"	"$(INTDIR)\general.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\general.obj"	"$(INTDIR)\general.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\general.obj"	"$(INTDIR)\general.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\general.obj"	"$(INTDIR)\general.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\general_definitions.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\general_definitions.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\general_definitions.obj"	"$(INTDIR)\general_definitions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\general_definitions.obj"	"$(INTDIR)\general_definitions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\general_definitions.obj"	"$(INTDIR)\general_definitions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\general_definitions.obj"	"$(INTDIR)\general_definitions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\general_definitions.obj"	"$(INTDIR)\general_definitions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\unix_to_nt\getopt.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\getopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\getopt.obj"	"$(INTDIR)\getopt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\getopt.obj"	"$(INTDIR)\getopt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\getopt.obj"	"$(INTDIR)\getopt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\getopt.obj"	"$(INTDIR)\getopt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\getopt.obj"	"$(INTDIR)\getopt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\idstack.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\idstack.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\idstack.obj"	"$(INTDIR)\idstack.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\idstack.obj"	"$(INTDIR)\idstack.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\idstack.obj"	"$(INTDIR)\idstack.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\idstack.obj"	"$(INTDIR)\idstack.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\idstack.obj"	"$(INTDIR)\idstack.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\init_switching.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\init_switching.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\init_switching.obj"	"$(INTDIR)\init_switching.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\init_switching.obj"	"$(INTDIR)\init_switching.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\init_switching.obj"	"$(INTDIR)\init_switching.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\init_switching.obj"	"$(INTDIR)\init_switching.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\internal_regions.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\internal_regions.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\internal_regions.obj"	"$(INTDIR)\internal_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\internal_regions.obj"	"$(INTDIR)\internal_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\internal_regions.obj"	"$(INTDIR)\internal_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\internal_regions.obj"	"$(INTDIR)\internal_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\internal_regions.obj"	"$(INTDIR)\internal_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\local_seg.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\local_seg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\local_seg.obj"	"$(INTDIR)\local_seg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\local_seg.obj"	"$(INTDIR)\local_seg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\local_seg.obj"	"$(INTDIR)\local_seg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\local_seg.obj"	"$(INTDIR)\local_seg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\local_seg.obj"	"$(INTDIR)\local_seg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_scheduling\loop.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\loop.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\loop.obj"	"$(INTDIR)\loop.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\loop.obj"	"$(INTDIR)\loop.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\loop.obj"	"$(INTDIR)\loop.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\loop.obj"	"$(INTDIR)\loop.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_scheduling\loop_get.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\loop_get.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\loop_get.obj"	"$(INTDIR)\loop_get.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\loop_get.obj"	"$(INTDIR)\loop_get.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\loop_get.obj"	"$(INTDIR)\loop_get.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\loop_get.obj"	"$(INTDIR)\loop_get.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_scheduling\loop_interface.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\loop_interface.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\loop_interface.obj"	"$(INTDIR)\loop_interface.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\loop_interface.obj"	"$(INTDIR)\loop_interface.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\loop_interface.obj"	"$(INTDIR)\loop_interface.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\loop_interface.obj"	"$(INTDIR)\loop_interface.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_scheduling\loop_partition.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\loop_partition.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\loop_partition.obj"	"$(INTDIR)\loop_partition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\loop_partition.obj"	"$(INTDIR)\loop_partition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\loop_partition.obj"	"$(INTDIR)\loop_partition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\loop_partition.obj"	"$(INTDIR)\loop_partition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_splitting\loop_split.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\loop_split.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\loop_split.obj"	"$(INTDIR)\loop_split.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\loop_split.obj"	"$(INTDIR)\loop_split.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\loop_split.obj"	"$(INTDIR)\loop_split.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\loop_split.obj"	"$(INTDIR)\loop_split.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_scheduling\loop_static.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\loop_static.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\loop_static.obj"	"$(INTDIR)\loop_static.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\loop_static.obj"	"$(INTDIR)\loop_static.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\loop_static.obj"	"$(INTDIR)\loop_static.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\loop_static.obj"	"$(INTDIR)\loop_static.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\message_passing\lowlevelmp.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\lowlevelmp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\lowlevelmp.obj"	"$(INTDIR)\lowlevelmp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\lowlevelmp.obj"	"$(INTDIR)\lowlevelmp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\lowlevelmp.obj"	"$(INTDIR)\lowlevelmp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\lowlevelmp.obj"	"$(INTDIR)\lowlevelmp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\lowlevelmp.obj"	"$(INTDIR)\lowlevelmp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\memcpy.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\memcpy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\memcpy.obj"	"$(INTDIR)\memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\memcpy.obj"	"$(INTDIR)\memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\memcpy.obj"	"$(INTDIR)\memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\memcpy.obj"	"$(INTDIR)\memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\memcpy.obj"	"$(INTDIR)\memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\memcpy_base.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\memcpy_base.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\memcpy_base.obj"	"$(INTDIR)\memcpy_base.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\memcpy_base.obj"	"$(INTDIR)\memcpy_base.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\memcpy_base.obj"	"$(INTDIR)\memcpy_base.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\memcpy_base.obj"	"$(INTDIR)\memcpy_base.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\memcpy_base.obj"	"$(INTDIR)\memcpy_base.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\memtree.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\memtree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\memtree.obj"	"$(INTDIR)\memtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\memtree.obj"	"$(INTDIR)\memtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\memtree.obj"	"$(INTDIR)\memtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\memtree.obj"	"$(INTDIR)\memtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\memtree.obj"	"$(INTDIR)\memtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\mmx_memcpy_win.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\mmx_memcpy_win.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\mmx_memcpy_win.obj"	"$(INTDIR)\mmx_memcpy_win.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\mmx_memcpy_win.obj"	"$(INTDIR)\mmx_memcpy_win.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\mmx_memcpy_win.obj"	"$(INTDIR)\mmx_memcpy_win.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\mmx_memcpy_win.obj"	"$(INTDIR)\mmx_memcpy_win.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\mmx_memcpy_win.obj"	"$(INTDIR)\mmx_memcpy_win.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\synchronization\mutex.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\mutex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\mutex.obj"	"$(INTDIR)\mutex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\mutex.obj"	"$(INTDIR)\mutex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\mutex.obj"	"$(INTDIR)\mutex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\mutex.obj"	"$(INTDIR)\mutex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\mutex.obj"	"$(INTDIR)\mutex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proc_node_numbers\node_name.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\node_name.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\node_name.obj"	"$(INTDIR)\node_name.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\node_name.obj"	"$(INTDIR)\node_name.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\node_name.obj"	"$(INTDIR)\node_name.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\node_name.obj"	"$(INTDIR)\node_name.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\node_name.obj"	"$(INTDIR)\node_name.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proc_node_numbers\node_rank.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\node_rank.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\node_rank.obj"	"$(INTDIR)\node_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\node_rank.obj"	"$(INTDIR)\node_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\node_rank.obj"	"$(INTDIR)\node_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\node_rank.obj"	"$(INTDIR)\node_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\node_rank.obj"	"$(INTDIR)\node_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proc_node_numbers\node_size.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\node_size.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\node_size.obj"	"$(INTDIR)\node_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\node_size.obj"	"$(INTDIR)\node_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\node_size.obj"	"$(INTDIR)\node_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\node_size.obj"	"$(INTDIR)\node_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\node_size.obj"	"$(INTDIR)\node_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\page_size.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\page_size.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\page_size.obj"	"$(INTDIR)\page_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\page_size.obj"	"$(INTDIR)\page_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\page_size.obj"	"$(INTDIR)\page_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\page_size.obj"	"$(INTDIR)\page_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\page_size.obj"	"$(INTDIR)\page_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\print_regions.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\print_regions.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\print_regions.obj"	"$(INTDIR)\print_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\print_regions.obj"	"$(INTDIR)\print_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\print_regions.obj"	"$(INTDIR)\print_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\print_regions.obj"	"$(INTDIR)\print_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\print_regions.obj"	"$(INTDIR)\print_regions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proc_node_numbers\proc_rank.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\proc_rank.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\proc_rank.obj"	"$(INTDIR)\proc_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\proc_rank.obj"	"$(INTDIR)\proc_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\proc_rank.obj"	"$(INTDIR)\proc_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\proc_rank.obj"	"$(INTDIR)\proc_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\proc_rank.obj"	"$(INTDIR)\proc_rank.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proc_node_numbers\proc_size.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\proc_size.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\proc_size.obj"	"$(INTDIR)\proc_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\proc_size.obj"	"$(INTDIR)\proc_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\proc_size.obj"	"$(INTDIR)\proc_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\proc_size.obj"	"$(INTDIR)\proc_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\proc_size.obj"	"$(INTDIR)\proc_size.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proc_node_numbers\proc_to_node.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\proc_to_node.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\proc_to_node.obj"	"$(INTDIR)\proc_to_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\proc_to_node.obj"	"$(INTDIR)\proc_to_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\proc_to_node.obj"	"$(INTDIR)\proc_to_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\proc_to_node.obj"	"$(INTDIR)\proc_to_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\proc_to_node.obj"	"$(INTDIR)\proc_to_node.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\synchronization\progress.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\progress.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\progress.obj"	"$(INTDIR)\progress.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\progress.obj"	"$(INTDIR)\progress.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\progress.obj"	"$(INTDIR)\progress.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\progress.obj"	"$(INTDIR)\progress.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\progress.obj"	"$(INTDIR)\progress.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\unix_to_nt\pthread.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\pthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\pthread.obj"	"$(INTDIR)\pthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\pthread.obj"	"$(INTDIR)\pthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\pthread.obj"	"$(INTDIR)\pthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\pthread.obj"	"$(INTDIR)\pthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\pthread.obj"	"$(INTDIR)\pthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\putget.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\putget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\putget.obj"	"$(INTDIR)\putget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\putget.obj"	"$(INTDIR)\putget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\putget.obj"	"$(INTDIR)\putget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\putget.obj"	"$(INTDIR)\putget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\putget.obj"	"$(INTDIR)\putget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\utility\query.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\query.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\redirect_io.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\redirect_io.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\redirect_io.obj"	"$(INTDIR)\redirect_io.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\redirect_io.obj"	"$(INTDIR)\redirect_io.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\redirect_io.obj"	"$(INTDIR)\redirect_io.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\redirect_io.obj"	"$(INTDIR)\redirect_io.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\redirect_io.obj"	"$(INTDIR)\redirect_io.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\region_layout.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\region_layout.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\region_layout.obj"	"$(INTDIR)\region_layout.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\region_layout.obj"	"$(INTDIR)\region_layout.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\region_layout.obj"	"$(INTDIR)\region_layout.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\region_layout.obj"	"$(INTDIR)\region_layout.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\region_layout.obj"	"$(INTDIR)\region_layout.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proper_shutdown\resource_list.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\resource_list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\resource_list.obj"	"$(INTDIR)\resource_list.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\resource_list.obj"	"$(INTDIR)\resource_list.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\resource_list.obj"	"$(INTDIR)\resource_list.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\resource_list.obj"	"$(INTDIR)\resource_list.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\resource_list.obj"	"$(INTDIR)\resource_list.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\safety.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\safety.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\safety.obj"	"$(INTDIR)\safety.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\safety.obj"	"$(INTDIR)\safety.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\safety.obj"	"$(INTDIR)\safety.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\safety.obj"	"$(INTDIR)\safety.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\safety.obj"	"$(INTDIR)\safety.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\dyn_mem\salloc.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\salloc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\salloc.obj"	"$(INTDIR)\salloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\salloc.obj"	"$(INTDIR)\salloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\salloc.obj"	"$(INTDIR)\salloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\salloc.obj"	"$(INTDIR)\salloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\salloc.obj"	"$(INTDIR)\salloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proper_shutdown\sci_desc.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sci_desc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sci_desc.obj"	"$(INTDIR)\sci_desc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sci_desc.obj"	"$(INTDIR)\sci_desc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sci_desc.obj"	"$(INTDIR)\sci_desc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sci_desc.obj"	"$(INTDIR)\sci_desc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sci_desc.obj"	"$(INTDIR)\sci_desc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\sci_shmem.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sci_shmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sci_shmem.obj"	"$(INTDIR)\sci_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sci_shmem.obj"	"$(INTDIR)\sci_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sci_shmem.obj"	"$(INTDIR)\sci_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sci_shmem.obj"	"$(INTDIR)\sci_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sci_shmem.obj"	"$(INTDIR)\sci_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\startup\sciflush.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sciflush.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sciflush.obj"	"$(INTDIR)\sciflush.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sciflush.obj"	"$(INTDIR)\sciflush.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sciflush.obj"	"$(INTDIR)\sciflush.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sciflush.obj"	"$(INTDIR)\sciflush.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sciflush.obj"	"$(INTDIR)\sciflush.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\startup\scistartup.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\scistartup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\scistartup.obj"	"$(INTDIR)\scistartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\scistartup.obj"	"$(INTDIR)\scistartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\scistartup.obj"	"$(INTDIR)\scistartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\scistartup.obj"	"$(INTDIR)\scistartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\scistartup.obj"	"$(INTDIR)\scistartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\regions\segment_address.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\segment_address.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\segment_address.obj"	"$(INTDIR)\segment_address.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\segment_address.obj"	"$(INTDIR)\segment_address.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\segment_address.obj"	"$(INTDIR)\segment_address.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\segment_address.obj"	"$(INTDIR)\segment_address.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\segment_address.obj"	"$(INTDIR)\segment_address.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\setup.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\setup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\setup.obj"	"$(INTDIR)\setup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\setup.obj"	"$(INTDIR)\setup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\setup.obj"	"$(INTDIR)\setup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\setup.obj"	"$(INTDIR)\setup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\setup.obj"	"$(INTDIR)\setup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\dyn_mem\sfree.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sfree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sfree.obj"	"$(INTDIR)\sfree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sfree.obj"	"$(INTDIR)\sfree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sfree.obj"	"$(INTDIR)\sfree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sfree.obj"	"$(INTDIR)\sfree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sfree.obj"	"$(INTDIR)\sfree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\Unix_to_NT\sys\Shm.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\Shm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\Shm.obj"	"$(INTDIR)\Shm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\Shm.obj"	"$(INTDIR)\Shm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\Shm.obj"	"$(INTDIR)\Shm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\Shm.obj"	"$(INTDIR)\Shm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\Shm.obj"	"$(INTDIR)\Shm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\shmem.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\shmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\shmem.obj"	"$(INTDIR)\shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\shmem.obj"	"$(INTDIR)\shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\shmem.obj"	"$(INTDIR)\shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\shmem.obj"	"$(INTDIR)\shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\shmem.obj"	"$(INTDIR)\shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\shseg_key.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\shseg_key.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\shseg_key.obj"	"$(INTDIR)\shseg_key.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\shseg_key.obj"	"$(INTDIR)\shseg_key.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\shseg_key.obj"	"$(INTDIR)\shseg_key.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\shseg_key.obj"	"$(INTDIR)\shseg_key.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\shseg_key.obj"	"$(INTDIR)\shseg_key.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\synchronization\signalization.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\signalization.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\signalization.obj"	"$(INTDIR)\signalization.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\signalization.obj"	"$(INTDIR)\signalization.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\signalization.obj"	"$(INTDIR)\signalization.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\signalization.obj"	"$(INTDIR)\signalization.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\signalization.obj"	"$(INTDIR)\signalization.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\dyn_mem\sinit.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sinit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sinit.obj"	"$(INTDIR)\sinit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sinit.obj"	"$(INTDIR)\sinit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sinit.obj"	"$(INTDIR)\sinit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sinit.obj"	"$(INTDIR)\sinit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sinit.obj"	"$(INTDIR)\sinit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\sisci_memcpy.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sisci_memcpy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sisci_memcpy.obj"	"$(INTDIR)\sisci_memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sisci_memcpy.obj"	"$(INTDIR)\sisci_memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sisci_memcpy.obj"	"$(INTDIR)\sisci_memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sisci_memcpy.obj"	"$(INTDIR)\sisci_memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sisci_memcpy.obj"	"$(INTDIR)\sisci_memcpy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\smi_fifo.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\smi_fifo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\smi_fifo.obj"	"$(INTDIR)\smi_fifo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\smi_fifo.obj"	"$(INTDIR)\smi_fifo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\smi_fifo.obj"	"$(INTDIR)\smi_fifo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\smi_fifo.obj"	"$(INTDIR)\smi_fifo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\smi_fifo.obj"	"$(INTDIR)\smi_fifo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\smi_finalize.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\smi_finalize.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\smi_finalize.obj"	"$(INTDIR)\smi_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\smi_finalize.obj"	"$(INTDIR)\smi_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\smi_finalize.obj"	"$(INTDIR)\smi_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\smi_finalize.obj"	"$(INTDIR)\smi_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\smi_finalize.obj"	"$(INTDIR)\smi_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\env\smi_init.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\smi_init.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\smi_init.obj"	"$(INTDIR)\smi_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\smi_init.obj"	"$(INTDIR)\smi_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\smi_init.obj"	"$(INTDIR)\smi_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\smi_init.obj"	"$(INTDIR)\smi_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\smi_init.obj"	"$(INTDIR)\smi_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\utility\smi_time.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\smi_time.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\smi_time.obj"	"$(INTDIR)\smi_time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\smi_time.obj"	"$(INTDIR)\smi_time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\smi_time.obj"	"$(INTDIR)\smi_time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\smi_time.obj"	"$(INTDIR)\smi_time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\smi_time.obj"	"$(INTDIR)\smi_time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\synchronization\smibarrier.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\smibarrier.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\smibarrier.obj"	"$(INTDIR)\smibarrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\smibarrier.obj"	"$(INTDIR)\smibarrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\smibarrier.obj"	"$(INTDIR)\smibarrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\smibarrier.obj"	"$(INTDIR)\smibarrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\smibarrier.obj"	"$(INTDIR)\smibarrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\message_passing\smisendrecv.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\smisendrecv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\smisendrecv.obj"	"$(INTDIR)\smisendrecv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\smisendrecv.obj"	"$(INTDIR)\smisendrecv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\smisendrecv.obj"	"$(INTDIR)\smisendrecv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\smisendrecv.obj"	"$(INTDIR)\smisendrecv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\smisendrecv.obj"	"$(INTDIR)\smisendrecv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\startup\smpstartup.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\smpstartup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\smpstartup.obj"	"$(INTDIR)\smpstartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\smpstartup.obj"	"$(INTDIR)\smpstartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\smpstartup.obj"	"$(INTDIR)\smpstartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\smpstartup.obj"	"$(INTDIR)\smpstartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\smpstartup.obj"	"$(INTDIR)\smpstartup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\startup\startup.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\startup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\startup.obj"	"$(INTDIR)\startup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\startup.obj"	"$(INTDIR)\startup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\startup.obj"	"$(INTDIR)\startup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\startup.obj"	"$(INTDIR)\startup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\startup.obj"	"$(INTDIR)\startup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\utility\statistics.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\statistics.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\synchronization\store_barrier.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\store_barrier.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\store_barrier.obj"	"$(INTDIR)\store_barrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\store_barrier.obj"	"$(INTDIR)\store_barrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\store_barrier.obj"	"$(INTDIR)\store_barrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\store_barrier.obj"	"$(INTDIR)\store_barrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\store_barrier.obj"	"$(INTDIR)\store_barrier.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\switch_to_replication.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\switch_to_replication.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\switch_to_replication.obj"	"$(INTDIR)\switch_to_replication.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\switch_to_replication.obj"	"$(INTDIR)\switch_to_replication.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\switch_to_replication.obj"	"$(INTDIR)\switch_to_replication.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\switch_to_replication.obj"	"$(INTDIR)\switch_to_replication.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\switch_to_replication_fast.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\switch_to_replication_fast.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\switch_to_replication_fast.obj"	"$(INTDIR)\switch_to_replication_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\switch_to_replication_fast.obj"	"$(INTDIR)\switch_to_replication_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\switch_to_replication_fast.obj"	"$(INTDIR)\switch_to_replication_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\switch_to_replication_fast.obj"	"$(INTDIR)\switch_to_replication_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\switch_to_sharing.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\switch_to_sharing.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\switch_to_sharing.obj"	"$(INTDIR)\switch_to_sharing.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\switch_to_sharing.obj"	"$(INTDIR)\switch_to_sharing.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\switch_to_sharing.obj"	"$(INTDIR)\switch_to_sharing.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\switch_to_sharing.obj"	"$(INTDIR)\switch_to_sharing.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\switch_consistency\switch_to_sharing_fast.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\switch_to_sharing_fast.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\switch_to_sharing_fast.obj"	"$(INTDIR)\switch_to_sharing_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\switch_to_sharing_fast.obj"	"$(INTDIR)\switch_to_sharing_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\switch_to_sharing_fast.obj"	"$(INTDIR)\switch_to_sharing_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\switch_to_sharing_fast.obj"	"$(INTDIR)\switch_to_sharing_fast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\synchronization\sync_finalize.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sync_finalize.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sync_finalize.obj"	"$(INTDIR)\sync_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sync_finalize.obj"	"$(INTDIR)\sync_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sync_finalize.obj"	"$(INTDIR)\sync_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sync_finalize.obj"	"$(INTDIR)\sync_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sync_finalize.obj"	"$(INTDIR)\sync_finalize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\synchronization\sync_init.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\sync_init.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\sync_init.obj"	"$(INTDIR)\sync_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\sync_init.obj"	"$(INTDIR)\sync_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\sync_init.obj"	"$(INTDIR)\sync_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\sync_init.obj"	"$(INTDIR)\sync_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\sync_init.obj"	"$(INTDIR)\sync_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\startup\tcpsync.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\tcpsync.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\tcpsync.obj"	"$(INTDIR)\tcpsync.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\tcpsync.obj"	"$(INTDIR)\tcpsync.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\tcpsync.obj"	"$(INTDIR)\tcpsync.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\tcpsync.obj"	"$(INTDIR)\tcpsync.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\tcpsync.obj"	"$(INTDIR)\tcpsync.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\Unix_to_NT\sys\time.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\time.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\time.obj"	"$(INTDIR)\time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\time.obj"	"$(INTDIR)\time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\time.obj"	"$(INTDIR)\time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\time.obj"	"$(INTDIR)\time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\time.obj"	"$(INTDIR)\time.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\loop_scheduling\timer.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\timer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\timer.obj"	"$(INTDIR)\timer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\timer.obj"	"$(INTDIR)\timer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\timer.obj"	"$(INTDIR)\timer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\timer.obj"	"$(INTDIR)\timer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\Unix_to_NT\unistd.c

!IF  "$(CFG)" == "smi - Win32 SMP"

CPP_SWITCHES=/nologo /G5 /MT /GX /O2 /I "src include" /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_DEBUG" /D "FORTRAN" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\unistd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

CPP_SWITCHES=/nologo /MT /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /FAs /Fa"$(INTDIR)\\" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\unistd.obj"	"$(INTDIR)\unistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

CPP_SWITCHES=/nologo /G5 /MT /GX /O2 /I "src include" /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "NDEBUG" /D "SVM" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\unistd.obj"	"$(INTDIR)\unistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

CPP_SWITCHES=/nologo /MT /GX /Z7 /O2 /I "include" /I "$(DOLPHIN_BASE)\include" /I "src\include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "SMI_NOCPP" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\unistd.obj"	"$(INTDIR)\unistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /GX /Z7 /Od /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /FAs /Fa"$(INTDIR)\\" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\unistd.obj"	"$(INTDIR)\unistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "smi - Win32 Release"

CPP_SWITCHES=/nologo /MD /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /FAs /Fa"$(INTDIR)\\" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\smi.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\unistd.obj"	"$(INTDIR)\unistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\memory\unix_shmem.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\unix_shmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\unix_shmem.obj"	"$(INTDIR)\unix_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\unix_shmem.obj"	"$(INTDIR)\unix_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\unix_shmem.obj"	"$(INTDIR)\unix_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\unix_shmem.obj"	"$(INTDIR)\unix_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\unix_shmem.obj"	"$(INTDIR)\unix_shmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\proper_shutdown\watchdog.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\watchdog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\watchdog.obj"	"$(INTDIR)\watchdog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\watchdog.obj"	"$(INTDIR)\watchdog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\watchdog.obj"	"$(INTDIR)\watchdog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\watchdog.obj"	"$(INTDIR)\watchdog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\watchdog.obj"	"$(INTDIR)\watchdog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\memory\wc_memcpy_c.c

!IF  "$(CFG)" == "smi - Win32 SMP"


"$(INTDIR)\wc_memcpy_c.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI"


"$(INTDIR)\wc_memcpy_c.obj"	"$(INTDIR)\wc_memcpy_c.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SVM"


"$(INTDIR)\wc_memcpy_c.obj"	"$(INTDIR)\wc_memcpy_c.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"


"$(INTDIR)\wc_memcpy_c.obj"	"$(INTDIR)\wc_memcpy_c.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Debug"


"$(INTDIR)\wc_memcpy_c.obj"	"$(INTDIR)\wc_memcpy_c.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "smi - Win32 Release"


"$(INTDIR)\wc_memcpy_c.obj"	"$(INTDIR)\wc_memcpy_c.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

