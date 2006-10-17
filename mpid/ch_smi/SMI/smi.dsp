# Microsoft Developer Studio Project File - Name="smi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=smi - Win32 SMP
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "smi.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "smi.mak" CFG="smi - Win32 SMP"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "smi - Win32 SMP" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 SCI" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 SVM" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 SCI_light" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "smi - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smi - Win32 SMP"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\smi___Wi"
# PROP BASE Intermediate_Dir ".\smi___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\tmp"
# PROP Intermediate_Dir "c:\tmp"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /Ox /I "smi___Wi/" /c /nologo
# ADD F90 /Ox /I "obj\win32/" /c /nologo
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "src\Unix_to_NT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /GX /O2 /I "src include" /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_DEBUG" /D "FORTRAN" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /YX /FD /c
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\win32\smi.lib"
# ADD LIB32 /nologo /out:".\lib\win32\smi.lib"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\smi___W0"
# PROP BASE Intermediate_Dir ".\smi___W0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "obj"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /Ox /I "smi___W0/" /c /nologo
# ADD F90 /Ox /I "obj\win32/" /c /nologo
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "src\Unix_to_NT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /Fr /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\win32\smi.lib"
# ADD LIB32 /nologo /out:".\lib\smi.lib"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\smi___W1"
# PROP BASE Intermediate_Dir ".\smi___W1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\tmp"
# PROP Intermediate_Dir "c:\tmp"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /Ox /I "smi___W1/" /c /nologo
# ADD F90 /Ox /I "obj\win32/" /c /nologo
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "src\Unix_to_NT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /GX /O2 /I "src include" /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "NDEBUG" /D "SVM" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /FR /YX /FD /c
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\win32\smi.lib"
# ADD LIB32 /nologo /out:".\lib\win32\smi.lib"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "smi___Win32_SCI_light"
# PROP BASE Intermediate_Dir "smi___Win32_SCI_light"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib\"
# PROP Intermediate_Dir "obj\"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /Ox /I "obj\win32/" /c /nologo
# ADD F90 /Ox /I "obj\win32/" /c /nologo
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Z7 /Od /Gf /Gy /I "$(DOLPHIN_BASE)\include" /I ".\src\Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /FR /FD /c
# SUBTRACT BASE CPP /u /YX
# ADD CPP /nologo /MT /GX /Z7 /O2 /I "include" /I "$(DOLPHIN_BASE)\include" /I "src\include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "SMI_NOCPP" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /FR /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\lib\win32\smi.lib"
# ADD LIB32 /nologo /out:".\lib\csmi.lib"

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "smi___Win32_Debug"
# PROP BASE Intermediate_Dir "smi___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /Ox /I "obj\win32/" /c /nologo
# ADD F90 /Ox /I "obj\win32/" /c /nologo
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /MT /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /YX /FD /c
# SUBTRACT BASE CPP /u /Fr
# ADD CPP /nologo /MDd /GX /Z7 /Od /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /Fr /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\lib\smi.lib"
# ADD LIB32 /nologo /out:".\lib\smi.lib"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "smi___Win32_Release"
# PROP BASE Intermediate_Dir "smi___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /Ox /I "obj\win32/" /c /nologo
# ADD F90 /Ox /I "obj\win32/" /c /nologo
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /MT /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /Fr /YX /FD /c
# SUBTRACT BASE CPP /u
# ADD CPP /nologo /MD /GX /O2 /I "$(DOLPHIN_BASE)\include" /I "include" /I "src" /I "src/Unix_to_NT" /D "_CONSOLE" /D "_MCBS" /D "_DEBUG" /D "SCIDEV_PRESENT" /D "FORTRAN" /D "DOLPHIN_SISCI" /D "SMI_ONLY_ONE_FD" /D "WIN32" /D "_WINDOWS" /D "PCI" /D "X86" /D "SMI_NONFIXED_MODE" /D "SCISTOREBARRIER_TWOARGS" /D "HAVE_SCIINITIALIZE" /Fr /YX /FD /c
# SUBTRACT CPP /u
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x407
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\lib\smi.lib"
# ADD LIB32 /nologo /out:".\lib\smi.lib"

!ENDIF 

# Begin Target

# Name "smi - Win32 SMP"
# Name "smi - Win32 SCI"
# Name "smi - Win32 SVM"
# Name "smi - Win32 SCI_light"
# Name "smi - Win32 Debug"
# Name "smi - Win32 Release"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\src\regions\address_to_region.c
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\combine_add.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\regions\connect_shreg.c
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\copy.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\copy_every_local.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\copy_gl_dist_local.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\utility\cpuid.c
# End Source File
# Begin Source File

SOURCE=.\src\regions\create_shreg.c
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\dyn_mem.c
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\ensure_consistency.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\err.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\env\error_count.c
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\first_proc_on_node.c
# End Source File
# Begin Source File

SOURCE=.\src\env\fortran_binding.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\regions\free_shreg.c
# End Source File
# Begin Source File

SOURCE=.\src\utility\general.c
# End Source File
# Begin Source File

SOURCE=.\src\env\general_definitions.c
# End Source File
# Begin Source File

SOURCE=.\src\unix_to_nt\getopt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\regions\idstack.c
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\init_switching.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\regions\internal_regions.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\local_seg.c
# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\loop.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\loop_get.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\loop_interface.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\loop_partition.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\loop_splitting\loop_split.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\loop_static.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\message_passing\lowlevelmp.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\memcpy.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\memcpy_base.c
# End Source File
# Begin Source File

SOURCE=.\src\regions\memtree.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\mmx_memcpy_win.c
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\mutex.c
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\node_name.c
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\node_rank.c
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\node_size.c
# End Source File
# Begin Source File

SOURCE=.\src\env\page_size.c
# End Source File
# Begin Source File

SOURCE=.\src\regions\print_regions.c
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\proc_rank.c
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\proc_size.c
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\proc_to_node.c
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\progress.c
# End Source File
# Begin Source File

SOURCE=.\src\unix_to_nt\pthread.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\putget.c
# End Source File
# Begin Source File

SOURCE=.\src\utility\query.c
# End Source File
# Begin Source File

SOURCE=.\src\env\redirect_io.c
# End Source File
# Begin Source File

SOURCE=.\src\regions\region_layout.c
# End Source File
# Begin Source File

SOURCE=.\src\proper_shutdown\resource_list.c
# End Source File
# Begin Source File

SOURCE=.\src\env\safety.c
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\salloc.c
# End Source File
# Begin Source File

SOURCE=.\src\proper_shutdown\sci_desc.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\sci_shmem.c
# End Source File
# Begin Source File

SOURCE=.\src\startup\sciflush.c
# End Source File
# Begin Source File

SOURCE=.\src\startup\scistartup.c
# End Source File
# Begin Source File

SOURCE=.\src\regions\segment_address.c
# End Source File
# Begin Source File

SOURCE=.\src\env\setup.c
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\sfree.c
# End Source File
# Begin Source File

SOURCE=.\src\Unix_to_NT\sys\Shm.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\shmem.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\shseg_key.c
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\signalization.c
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\sinit.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\sisci_memcpy.c
# End Source File
# Begin Source File

SOURCE=.\src\env\smi_fifo.c
# End Source File
# Begin Source File

SOURCE=.\src\env\smi_finalize.c
# End Source File
# Begin Source File

SOURCE=.\src\env\smi_init.c
# End Source File
# Begin Source File

SOURCE=.\src\utility\smi_time.c
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\smibarrier.c
# End Source File
# Begin Source File

SOURCE=.\src\message_passing\smisendrecv.c
# End Source File
# Begin Source File

SOURCE=.\src\startup\smpstartup.c
# End Source File
# Begin Source File

SOURCE=.\src\startup\startup.c
# End Source File
# Begin Source File

SOURCE=.\src\utility\statistics.c
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\store_barrier.c
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_replication.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_replication_fast.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_sharing.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_sharing_fast.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\synchronization\sync_finalize.c
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\sync_init.c
# End Source File
# Begin Source File

SOURCE=.\src\startup\tcpsync.c
# End Source File
# Begin Source File

SOURCE=.\src\Unix_to_NT\sys\time.c
# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\timer.cpp

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\Unix_to_NT\unistd.c

!IF  "$(CFG)" == "smi - Win32 SMP"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "smi - Win32 SVM"

!ELSEIF  "$(CFG)" == "smi - Win32 SCI_light"

# ADD BASE CPP /FAs
# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "smi - Win32 Debug"

# ADD BASE CPP /FAs
# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "smi - Win32 Release"

# ADD BASE CPP /FAs
# ADD CPP /FAs

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\memory\unix_shmem.c
# End Source File
# Begin Source File

SOURCE=.\src\proper_shutdown\watchdog.c
# End Source File
# Begin Source File

SOURCE=.\src\memory\wc_memcpy_c.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\src\regions\address_to_region.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\barrier.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\combine_add.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\connect_shreg.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\copy.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\copy_every_local.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\copy_gl_dist_local.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\create_shreg.h
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\dyn_mem.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\ensure_consistency.h
# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\err.h
# End Source File
# Begin Source File

SOURCE=.\src\env\error_count.h
# End Source File
# Begin Source File

SOURCE=.\src\error_count.h
# End Source File
# Begin Source File

SOURCE=.\src\env\fifo.h
# End Source File
# Begin Source File

SOURCE=.\src\env\finalize.h
# End Source File
# Begin Source File

SOURCE=.\src\finalize.h
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\first_proc_on_node.h
# End Source File
# Begin Source File

SOURCE=.\src\fortran_binding.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\fpu_copy.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\free_shreg.h
# End Source File
# Begin Source File

SOURCE=.\src\utility\general.h
# End Source File
# Begin Source File

SOURCE=.\src\env\general_definitions.h
# End Source File
# Begin Source File

SOURCE=.\src\unix_to_nt\getopt.h
# End Source File
# Begin Source File

SOURCE=.\src\utility\getus.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\idstack.h
# End Source File
# Begin Source File

SOURCE=.\src\init.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\init_switching.h
# End Source File
# Begin Source File

SOURCE=.\src\env\internal_functions.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\internal_regions.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\local_seg.h
# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\loop.h
# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\loop_interface.h
# End Source File
# Begin Source File

SOURCE=.\src\loop_splitting\loop_split.h
# End Source File
# Begin Source File

SOURCE=.\src\message_passing\lowlevelmp.h
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\mem.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\memcpy.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\memcpy_base.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\memtree.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\mutex.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\WIN32\mutex.h
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\node_name.h
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\node_rank.h
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\node_size.h
# End Source File
# Begin Source File

SOURCE=.\src\env\page_size.h
# End Source File
# Begin Source File

SOURCE=.\src\page_size.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\print_regions.h
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\proc_rank.h
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\proc_size.h
# End Source File
# Begin Source File

SOURCE=.\src\proc_node_numbers\proc_to_node.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\progress.h
# End Source File
# Begin Source File

SOURCE="D:\Programme\Microsoft Platform SDK\Include\PropIdl.h"
# End Source File
# Begin Source File

SOURCE=.\src\unix_to_nt\pthread.h
# End Source File
# Begin Source File

SOURCE=.\src\utility\query.h
# End Source File
# Begin Source File

SOURCE=.\src\env\redirect_io.h
# End Source File
# Begin Source File

SOURCE=.\src\redirect_io.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\region_layout.h
# End Source File
# Begin Source File

SOURCE=.\src\proper_shutdown\resource_list.h
# End Source File
# Begin Source File

SOURCE=.\src\env\safety.h
# End Source File
# Begin Source File

SOURCE=.\src\safety.h
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\salloc.h
# End Source File
# Begin Source File

SOURCE=.\src\proper_shutdown\sci_desc.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\sci_shmem.h
# End Source File
# Begin Source File

SOURCE=.\src\startup\scicomm.h
# End Source File
# Begin Source File

SOURCE=.\src\regions\segment_address.h
# End Source File
# Begin Source File

SOURCE=.\src\message_passing\sendrecv.h
# End Source File
# Begin Source File

SOURCE=.\src\env\setup.h
# End Source File
# Begin Source File

SOURCE=.\src\message_passing\setup_comm.h
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\sfree.h
# End Source File
# Begin Source File

SOURCE=.\src\Unix_to_NT\sys\shm.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\shmem.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\shseg_key.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\signalization.h
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\sinit.h
# End Source File
# Begin Source File

SOURCE=.\src\dyn_mem\smem.h
# End Source File
# Begin Source File

SOURCE=.\include\smi.h
# End Source File
# Begin Source File

SOURCE=.\src\env\smi_init.h
# End Source File
# Begin Source File

SOURCE=.\src\utility\smi_time.h
# End Source File
# Begin Source File

SOURCE=.\src\env\smidebug.h
# End Source File
# Begin Source File

SOURCE=.\src\smidebug.h
# End Source File
# Begin Source File

SOURCE=.\src\startup\startup.h
# End Source File
# Begin Source File

SOURCE=.\src\utility\statistics.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\store_barrier.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\svm_shmem.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_replication.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_replication_fast.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_sharing.h
# End Source File
# Begin Source File

SOURCE=.\src\switch_consistency\switch_to_sharing_fast.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\sync.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\sync_finalize.h
# End Source File
# Begin Source File

SOURCE=.\src\synchronization\sync_init.h
# End Source File
# Begin Source File

SOURCE=.\src\startup\tcpsync.h
# End Source File
# Begin Source File

SOURCE=.\src\Unix_to_NT\sys\time.h
# End Source File
# Begin Source File

SOURCE=.\src\loop_scheduling\timer.h
# End Source File
# Begin Source File

SOURCE=.\src\Unix_to_NT\unistd.h
# End Source File
# Begin Source File

SOURCE=.\src\memory\unix_shmem.h
# End Source File
# Begin Source File

SOURCE=.\src\proper_shutdown\watchdog.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
