# Microsoft Developer Studio Project File - Name="mpi2cpp_test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mpi2cpp_test - Win32 Debug
!MESSAGE Dies ist kein gÅltiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und fÅhren Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mpi2cpp_test.mak".
!MESSAGE 
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mpi2cpp_test.mak" CFG="mpi2cpp_test - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mpi2cpp_test - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "mpi2cpp_test - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mpi2cpp_test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "..\..\\" /I "..\..\..\include" /I "..\..\src" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX"mpi2c++_test.h" /FD /EHsc- /TP /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 cpp.lib mpich.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"..\..\..\lib"

!ELSEIF  "$(CFG)" == "mpi2cpp_test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /ZI /Od /I "..\..\\" /I "..\..\..\include" /I "..\..\src" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cppd.lib mpich.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\lib"

!ENDIF 

# Begin Target

# Name "mpi2cpp_test - Win32 Release"
# Name "mpi2cpp_test - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\allgather.cc
# End Source File
# Begin Source File

SOURCE=.\allreduce.cc
# End Source File
# Begin Source File

SOURCE=.\alltoall.cc
# End Source File
# Begin Source File

SOURCE=.\attr.cc
# End Source File
# Begin Source File

SOURCE=.\badbuf.cc
# End Source File
# Begin Source File

SOURCE=.\bcast.cc
# End Source File
# Begin Source File

SOURCE=.\bcast_struct.cc
# End Source File
# Begin Source File

SOURCE=.\bottom.cc
# End Source File
# Begin Source File

SOURCE=.\bsend.cc
# End Source File
# Begin Source File

SOURCE=.\buffer.cc
# End Source File
# Begin Source File

SOURCE=.\cancel.cc
# End Source File
# Begin Source File

SOURCE=.\cartcomm.cc
# End Source File
# Begin Source File

SOURCE=.\commdup.cc
# End Source File
# Begin Source File

SOURCE=.\commfree.cc
# End Source File
# Begin Source File

SOURCE=.\compare.cc
# End Source File
# Begin Source File

SOURCE=.\dims.cc
# End Source File
# Begin Source File

SOURCE=.\dup_test.cc
# End Source File
# Begin Source File

SOURCE=.\errhandler.cc

!IF  "$(CFG)" == "mpi2cpp_test - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "mpi2cpp_test - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gather.cc
# End Source File
# Begin Source File

SOURCE=.\getcount.cc
# End Source File
# Begin Source File

SOURCE=.\getel.cc
# End Source File
# Begin Source File

SOURCE=.\graphcomm.cc
# End Source File
# Begin Source File

SOURCE=.\group.cc
# End Source File
# Begin Source File

SOURCE=.\groupfree.cc
# End Source File
# Begin Source File

SOURCE=.\initialized1.cc
# End Source File
# Begin Source File

SOURCE=.\initialized2.cc
# End Source File
# Begin Source File

SOURCE=.\intercomm.cc
# End Source File
# Begin Source File

SOURCE=.\interf.cc
# End Source File
# Begin Source File

SOURCE=.\iprobe.cc
# End Source File
# Begin Source File

SOURCE=.\isend.cc
# End Source File
# Begin Source File

SOURCE=.\messages.cc
# End Source File
# Begin Source File

SOURCE=".\mpi2c++_test.cc"
# End Source File
# Begin Source File

SOURCE=.\op_test.cc
# End Source File
# Begin Source File

SOURCE=.\pack_test.cc
# End Source File
# Begin Source File

SOURCE=.\pcontrol.cc
# End Source File
# Begin Source File

SOURCE=.\probe.cc
# End Source File
# Begin Source File

SOURCE=.\procname.cc
# End Source File
# Begin Source File

SOURCE=.\range.cc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\rank_size.cc
# End Source File
# Begin Source File

SOURCE=.\reduce.cc
# End Source File
# Begin Source File

SOURCE=.\reduce_scatter.cc
# End Source File
# Begin Source File

SOURCE=.\request1.cc
# End Source File
# Begin Source File

SOURCE=.\rsend.cc
# End Source File
# Begin Source File

SOURCE=.\scan.cc
# End Source File
# Begin Source File

SOURCE=.\scatter.cc
# End Source File
# Begin Source File

SOURCE=.\send.cc
# End Source File
# Begin Source File

SOURCE=.\sendrecv.cc
# End Source File
# Begin Source File

SOURCE=.\sendrecv_rep.cc
# End Source File
# Begin Source File

SOURCE=.\signal.cc
# End Source File
# Begin Source File

SOURCE=.\split.cc
# End Source File
# Begin Source File

SOURCE=.\ssend.cc
# End Source File
# Begin Source File

SOURCE=.\stack.cc
# End Source File
# Begin Source File

SOURCE=.\start.cc
# End Source File
# Begin Source File

SOURCE=.\startall.cc
# End Source File
# Begin Source File

SOURCE=.\status_test.cc
# End Source File
# Begin Source File

SOURCE=.\struct_gatherv.cc
# End Source File
# Begin Source File

SOURCE=.\test1.cc
# End Source File
# Begin Source File

SOURCE=.\test3.cc
# End Source File
# Begin Source File

SOURCE=.\testall.cc
# End Source File
# Begin Source File

SOURCE=.\testany.cc
# End Source File
# Begin Source File

SOURCE=.\testsome.cc
# End Source File
# Begin Source File

SOURCE=.\topo.cc
# End Source File
# Begin Source File

SOURCE=.\waitall.cc
# End Source File
# Begin Source File

SOURCE=.\waitany.cc
# End Source File
# Begin Source File

SOURCE=.\waitsome.cc
# End Source File
# Begin Source File

SOURCE=.\wtime.cc
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\mpi2c++_test.h"
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
