# Microsoft Developer Studio Project File - Name="mpirun" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mpirun - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mpirun.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mpirun.mak" CFG="mpirun - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mpirun - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "mpirun - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mpirun - Win32 Release"

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
F90=fl32.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\\" /I "..\\" /I "plugins" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 rpcrt4.lib kernel32.lib user32.lib advapi32.lib ws2_32.lib netapi32.lib mpr.lib /nologo /subsystem:console /machine:I386 /out:"..\..\bin\mpiexec.exe" /implib:Release/mpirun.lib
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "mpirun - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
F90=fl32.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "plugins" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 rpcrt4.lib kernel32.lib user32.lib advapi32.lib ws2_32.lib netapi32.lib mpr.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"..\..\bin\mpiexec.exe" /implib:"debug\mpiexec.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "mpirun - Win32 Release"
# Name "mpirun - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\args.c
# End Source File
# Begin Source File

SOURCE=.\AsyncWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\cluma.acf
# End Source File
# Begin Source File

SOURCE=..\cluma.idl

!IF  "$(CFG)" == "mpirun - Win32 Release"

USERDEP__CLUMA="..\cluma.acf"	
# Begin Custom Build - Creating client stub
InputPath=..\cluma.idl

BuildCmds= \
	midl /nologo /env win32 /acf ..\cluma.acf /server none /out ..\ $(InputPath)

"..\cluma_c.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\cluma.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mpirun - Win32 Debug"

USERDEP__CLUMA="..\cluma.acf"	
# Begin Custom Build - Creating client stub
InputPath=..\cluma.idl

BuildCmds= \
	midl /env win32 /acf ..\cluma.acf /server none /out ..\ $(InputPath)

"..\cluma_c.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\cluma.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\cluma_c.c
# ADD CPP /D _WIN32_WINNT=0x400
# End Source File
# Begin Source File

SOURCE=.\Encryption.cpp
# End Source File
# Begin Source File

SOURCE=.\Environment.cpp
# End Source File
# Begin Source File

SOURCE=.\FileFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\hosts.cpp
# End Source File
# Begin Source File

SOURCE=.\inSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\lists.c
# End Source File
# Begin Source File

SOURCE=.\rexec.cpp
# End Source File
# Begin Source File

SOURCE=.\RexecClient.cpp
# End Source File
# Begin Source File

SOURCE=.\RexHelp.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\args.h
# End Source File
# Begin Source File

SOURCE=.\AsyncWindow.h
# End Source File
# Begin Source File

SOURCE=..\cluma.h
# End Source File
# Begin Source File

SOURCE=.\Debug.h
# End Source File
# Begin Source File

SOURCE=.\Encryption.h
# End Source File
# Begin Source File

SOURCE=.\Environment.h
# End Source File
# Begin Source File

SOURCE=.\FileFunctions.h
# End Source File
# Begin Source File

SOURCE=.\inSocket.h
# End Source File
# Begin Source File

SOURCE=.\lists.h
# End Source File
# Begin Source File

SOURCE=.\mpirun.h
# End Source File
# Begin Source File

SOURCE=.\plugins\Plugin.h
# End Source File
# Begin Source File

SOURCE=.\RexecClient.h
# End Source File
# Begin Source File

SOURCE=.\RexHelp.h
# End Source File
# Begin Source File

SOURCE=..\SocketException.h
# End Source File
# Begin Source File

SOURCE=..\Update.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
