# Microsoft Developer Studio Project File - Name="usock_device" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=usock_device - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "usock_device.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "usock_device.mak" CFG="usock_device - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "usock_device - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "usock_device - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "usock_device - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "usock_device___Win32_Release"
# PROP BASE Intermediate_Dir "usock_device___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "usock_device___Win32_Release"
# PROP Intermediate_Dir "usock_device___Win32_Release"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\..\ch_ntshmem" /I "..\lfbs_common" /I "..\..\include" /I "..\ch2" /I "..\util" /I "..\..\mpi-2-c++\src" /I "C:\Programme\Microsoft SDK\Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "MPID_DEBUG_NONE" /D "FORTRANCAPS" /D "_WINDOWS" /D "RNDV_STATIC" /D "SINGLECOPY" /D "WIN32_FORTRAN_STDCALL" /D "HAS_VOLATILE" /D "WSOCK2" /D "IN_MPICH_DLL" /D "USE_STDARG" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "usock_device - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "usock_device___Win32_Debug"
# PROP BASE Intermediate_Dir "usock_device___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "usock_device___Win32_Debug"
# PROP Intermediate_Dir "usock_device___Win32_Debug"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
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

# Name "usock_device - Win32 Release"
# Name "usock_device - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\ch2\adi2cancel.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2config.h
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2hrecv.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2hsend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2hssend.c
# End Source File
# Begin Source File

SOURCE=..\ch_wsock2\adi2init.c
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

SOURCE=..\ch_wsock2\adi2recv.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2req.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2send.c
# End Source File
# Begin Source File

SOURCE=..\ch2\adi2ssend.c
# End Source File
# Begin Source File

SOURCE=..\ch2\attach.h
# End Source File
# Begin Source File

SOURCE=..\ch2\calltrace.c
# End Source File
# Begin Source File

SOURCE=..\ch2\calltrace.h
# End Source File
# Begin Source File

SOURCE=.\channel.h
# End Source File
# Begin Source File

SOURCE=.\chconfig.h
# End Source File
# Begin Source File

SOURCE=.\chdebug.c
# End Source File
# Begin Source File

SOURCE=.\chdef.h
# End Source File
# Begin Source File

SOURCE=..\ch2\chhetero.h
# End Source File
# Begin Source File

SOURCE=..\lfbs_common\chnodename.c
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

SOURCE=.\getopt.cpp
# End Source File
# Begin Source File

SOURCE=.\insocket.cpp
# End Source File
# Begin Source File

SOURCE=.\insocket.h
# End Source File
# Begin Source File

SOURCE=.\lfshmempriv.c

!IF  "$(CFG)" == "usock_device - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "usock_device - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ch2\mpid.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpid_bind.h
# End Source File
# Begin Source File

SOURCE=.\mpid_time.h
# End Source File
# Begin Source File

SOURCE=.\mpiddev.h
# End Source File
# Begin Source File

SOURCE=..\ch2\mpimem.h
# End Source File
# Begin Source File

SOURCE=.\mydebug.h
# End Source File
# Begin Source File

SOURCE=.\mydebug_c.h
# End Source File
# Begin Source File

SOURCE=..\ch2\objtrace.c
# End Source File
# Begin Source File

SOURCE=.\p2p.c

!IF  "$(CFG)" == "usock_device - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "usock_device - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\p2p.h
# End Source File
# Begin Source File

SOURCE=.\p2pwinprocs.c

!IF  "$(CFG)" == "usock_device - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "usock_device - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\packets.h
# End Source File
# Begin Source File

SOURCE=..\ch2\req.h
# End Source File
# Begin Source File

SOURCE=..\ch2\reqalloc.h
# End Source File
# Begin Source File

SOURCE=..\ch2\reqrndv.h
# End Source File
# Begin Source File

SOURCE=..\util\sbcnst2.c
# End Source File
# Begin Source File

SOURCE=..\util\sbcnst2.h
# End Source File
# Begin Source File

SOURCE=.\shdef.h
# End Source File
# Begin Source File

SOURCE=.\shmeminit.c

!IF  "$(CFG)" == "usock_device - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "usock_device - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\socketexception.h
# End Source File
# Begin Source File

SOURCE=.\TCPCommunicator.cpp
# End Source File
# Begin Source File

SOURCE=.\TCPCommunicator.h
# End Source File
# Begin Source File

SOURCE=..\util\tr2.c
# End Source File
# Begin Source File

SOURCE=..\util\tr2.h
# End Source File
# Begin Source File

SOURCE=.\usock2debug.h
# End Source File
# Begin Source File

SOURCE=.\usockbeager.c
# End Source File
# Begin Source File

SOURCE=.\usockbrndv.c
# End Source File
# Begin Source File

SOURCE=.\usockcancel.c
# End Source File
# Begin Source File

SOURCE=.\usockchkdev.c
# End Source File
# Begin Source File

SOURCE=.\usockinit.c
# End Source File
# Begin Source File

SOURCE=.\usockneager.c
# End Source File
# Begin Source File

SOURCE=.\usockpriv.cpp
# End Source File
# Begin Source File

SOURCE=.\usockpriv.h
# End Source File
# Begin Source File

SOURCE=.\usockshort.c
# End Source File
# Begin Source File

SOURCE=.\wsock2debug.h
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
