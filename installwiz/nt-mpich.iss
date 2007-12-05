[_ISTool]
EnableISX=false

[Setup]
AppCopyright=LfBS
AppName=nt-mpich
AppVerName=nt-mpich 1.5.0
DefaultDirName={pf}\nt-mpich
MinVersion=0,4.00.1381sp6
LicenseFile=..\COPYRIGHT
DefaultGroupName=nt-mpich
UsePreviousTasks=false
AlwaysRestart=false
RestartIfNeededByRun=yes
OutputBaseFilename=nt-mpich_setup
UninstallDisplayIcon={app}\bin\RexecShell.exe
PrivilegesRequired=admin
AppPublisher=LfBS
AppPublisherURL=http://www.lfbs.rwth-aachen.de
AppUpdatesURL=http://www.lfbs.rwth-aachen.de/content/mp-mpich-dl
UninstallDisplayName=NT-MPICH
AppVersion=1.5.0


[Files]
Source: ..\doc\mp-mpich_manual.pdf; DestDir: {app}\doc; Components: Cluster_Frontend; Flags: isreadme
Source: ..\doc\runtests_readme.txt; DestDir: {app}\doc; Components: Cluster_Frontend
Source: ..\doc\Quickstart.pdf; DestDir: {app}\doc; Components: Cluster_Frontend; Flags: isreadme

Source: ..\bin\RexecShell.exe; DestDir: {app}\bin; DestName: RexecShell.exe; Components: Cluster_Frontend
Source: ..\bin\mpe_server_frontend.exe; DestDir: {app}\bin; Components: Cluster_Frontend

Source: ..\bin\mpiexec.exe; DestDir: {app}\bin; Components: Cluster_Frontend
Source: ..\bin\ntrexec.exe; DestDir: {app}\bin; Components: Cluster_Frontend
Source: ..\bin\runtests.exe; DestDir: {app}\bin; Components: Cluster_Frontend
Source: ..\bin\chkresult.exe; DestDir: {app}\bin; Components: Cluster_Frontend
Source: ..\bin\Plugins\*.dll; DestDir: {app}\bin\plugins; Components: Cluster_Frontend

Source: ..\bin\rclumad.exe; DestDir: {tmp}\nt-mpich_setup; Components: Cluster_Node; Flags: deleteafterinstall
Source: ..\bin\rcluma.cpl; DestDir: {tmp}\nt-mpich_setup; Components: Cluster_Node; Flags: deleteafterinstall
Source: ..\bin\rcluma-install.bat; DestDir: {tmp}\nt-mpich_setup; Components: Cluster_Node; Flags: deleteafterinstall
Source: ..\bin\rcluma-uninstall.bat; DestDir: {tmp}\nt-mpich_setup; Components: Cluster_Node; Flags: deleteafterinstall
Source: ..\bin\rcluma-uninstall.bat; DestDir: {app}\bin; Components: Cluster_Node
Source: ..\bin\rcluma-update.bat; DestDir: {app}\bin; Components: Cluster_Node

Source: ..\examples\basic\*.sln; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.vcproj; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.dsp; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.dsw; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.c; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.cc; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.cpp; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.h; DestDir: {app}\examples\basic; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\*.f; DestDir: {app}\examples\basic; Components: Examples
Source: ..\examples\basic\*.f90; DestDir: {app}\examples\basic; Components: Examples
Source: ..\examples\basic\README; DestDir: {app}\examples\basic; Components: Examples
Source: ..\examples\basic\build.bat; DestDir: {app}\examples\basic; Components: Examples
Source: ..\examples\basic\PiMfc\*.rc*; DestDir: {app}\examples\basic\PiMfc; Components: Examples; Flags: recursesubdirs
Source: ..\examples\basic\PiMfc\res\*.ico; DestDir: {app}\examples\basic\PiMfc\res; Components: Examples; Flags: recursesubdirs

Source: ..\examples\test\*.sln; Excludes: util*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.vcproj; Excludes: util*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.vfproj; Excludes: util*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.dsp; Excludes: util*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.dsw; Excludes: util*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.c; Excludes: util*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.f; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.h; Excludes: util*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.txt; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\*.std; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\README; Excludes: command*; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\build.bat; DestDir: {app}\examples\test; Components: Examples; Flags: recursesubdirs
Source: ..\examples\test\util\test.h; DestDir: {app}\examples\test\util; Components: Examples
Source: ..\examples\test\util\test.c; DestDir: {app}\examples\test\util; Components: Examples


Source: ..\mpe\contrib\*.dsw; DestDir: {app}\examples\MPE; Components: Examples
Source: ..\mpe\contrib\*.f; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs
Source: ..\mpe\contrib\*.c; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs
Source: ..\mpe\contrib\*.h; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs
Source: ..\mpe\contrib\*.dsp; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs
Source: ..\mpe\contrib\*.in; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs
Source: ..\mpe\contrib\*.pd; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs
Source: ..\mpe\contrib\README; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs
Source: ..\mpe\contrib\*.points; DestDir: {app}\examples\MPE; Components: Examples; Flags: recursesubdirs

Source: ..\MPI-2-C++\contrib\examples\*.sln; DestDir: {app}\examples\C++; Components: Examples; Flags: recursesubdirs
Source: ..\MPI-2-C++\contrib\examples\*.vcproj; DestDir: {app}\examples\C++; Components: Examples; Flags: recursesubdirs
Source: ..\MPI-2-C++\contrib\examples\*.cc; DestDir: {app}\examples\C++; Components: Examples; Flags: recursesubdirs
Source: ..\MPI-2-C++\contrib\examples\README; DestDir: {app}\examples\C++; Components: Examples; Flags: recursesubdirs
Source: ..\MPI-2-C++\*.h; DestDir: {app}\include\mpi2c++\; Components: Examples Development
Source: ..\MPI-2-C++\src\mpi2c++\*.h; DestDir: {app}\include\mpi2c++\; Components: Examples Development

Source: ..\romio\test\*.dsw; DestDir: {app}\examples\RomIO; Components: Examples; Flags: recursesubdirs
Source: ..\romio\test\*.dsp; DestDir: {app}\examples\RomIO; Components: Examples; Flags: recursesubdirs
Source: ..\romio\test\*.std; DestDir: {app}\examples\RomIO; Components: Examples; Flags: recursesubdirs
Source: ..\romio\test\*.c; DestDir: {app}\examples\RomIO; Components: Examples; Flags: recursesubdirs
Source: ..\romio\test\*.cpp; DestDir: {app}\examples\RomIO; Components: Examples; Flags: recursesubdirs
Source: ..\romio\test\*.vbs; DestDir: {app}\examples\RomIO; Components: Examples; Flags: recursesubdirs


Source: ..\lib\*.lib; DestDir: {app}\lib; Components: Examples Development
Source: ..\lib\*.dll; DestDir: {app}\lib
Source: ..\lib\*.txt; DestDir: {app}\lib; Components: Examples Development


Source: ..\include\*.h; DestDir: {app}\include; Components: Examples Development
Source: ..\mpe\win_mpe_server\*.h; DestDir: {app}\include; Components: Examples Development
Source: ..\mpe\mpe.h; DestDir: {app}\include; Components: Examples Development
Source: ..\mpe\mpe_log.h; DestDir: {app}\include; Components: Examples Development
Source: ..\mpe\mpe_win_graphics.h; DestDir: {app}\include; Components: Examples Development


Source: ..\mpe\profiling\lib\*.*; DestDir: {app}\profiling\lib; Components: Logging
Source: ..\mpe\profiling\examples\*.c; DestDir: {app}\profiling\examples; Components: Logging
Source: ..\mpe\profiling\examples\*.dsp; DestDir: {app}\profiling\examples; Components: Logging
Source: ..\mpe\profiling\examples\*.dsw; DestDir: {app}\profiling\examples; Components: Logging

Source: ..\mpe\slog_api\COPYRIGHT.; DestDir: {app}\slog_api; Components: Logging
Source: ..\mpe\slog_api\Readme.; DestDir: {app}\slog_api; Components: Logging
Source: ..\mpe\slog_api\*.txt; DestDir: {app}\slog_api; Components: Logging
Source: ..\mpe\slog_api\*.dsp; DestDir: {app}\slog_api; Components: Logging
Source: ..\mpe\slog_api\bin\Readme; DestDir: {app}\slog_api\bin; Components: Logging
Source: ..\mpe\slog_api\test\Release\readtest.exe; DestDir: {app}\slog_api\bin; Components: Logging
Source: ..\mpe\slog_api\doc\html\*.*; DestDir: {app}\slog_api\doc\html; Components: Logging
Source: ..\mpe\slog_api\src\*.h; DestDir: {app}\slog_api\include; Components: Logging
Source: ..\mpe\slog_api\lib\slog_api.lib; DestDir: {app}\slog_api\lib; Components: Logging
Source: ..\mpe\slog_api\lib\README.; DestDir: {app}\slog_api\lib; Components: Logging
Source: ..\mpe\slog_api\test\*.dsp; DestDir: {app}\slog_api\test; Components: Logging
Source: ..\mpe\slog_api\test\*.dsw; DestDir: {app}\slog_api\test; Components: Logging
Source: ..\mpe\slog_api\src\slog_readtest.c; DestDir: {app}\slog_api\test; Components: Logging



Source: ..\jumpshot\COPYRIGHT.; DestDir: {app}\jumpshot; Components: Logging
Source: ..\jumpshot\INSTALL.; DestDir: {app}\jumpshot; Components: Logging
Source: ..\jumpshot\README.; DestDir: {app}\jumpshot; Components: Logging
Source: ..\jumpshot\bin\*.bat; DestDir: {app}\jumpshot\bin; Components: Logging
Source: ..\jumpshot\lib\data\*.*; DestDir: {app}\jumpshot\lib\data; Components: Logging
Source: ..\jumpshot\lib\images\*.*; DestDir: {app}\jumpshot\lib\images; Components: Logging
Source: ..\jumpshot\lib\logfiles\*.*; DestDir: {app}\jumpshot\lib\logfiles; Components: Logging
Source: ..\jumpshot\swing\*.jar; DestDir: {app}\jumpshot\swing; Components: Logging

Source: ..\jumpshot-3\readme.*; DestDir: {app}\jumpshot-3; Components: Logging
Source: ..\jumpshot-3\*.txt; DestDir: {app}\jumpshot-3; Components: Logging
Source: ..\jumpshot-3\bin\*.jar; DestDir: {app}\jumpshot-3\bin; Components: Logging
Source: ..\jumpshot-3\bin\*.bat; DestDir: {app}\jumpshot-3\bin; Components: Logging
Source: ..\jumpshot-3\bin\images\*.*; DestDir: {app}\jumpshot-3\bin\images; Components: Logging
Source: ..\jumpshot-3\lib\*.*; DestDir: {app}\jumpshot-3\lib; Components: Logging
Source: ..\jumpshot-3\swing\*.*; DestDir: {app}\jumpshot-3\swing; Components: Logging
Source: ..\jumpshot-3\doc\*.*; DestDir: {app}\jumpshot-3\doc; Components: Logging

Source: ..\www\*.*; DestDir: {app}\www; Components: WWW
Source: ..\www\www1\*.*; DestDir: {app}\www\www1; Components: WWW
Source: ..\www\www3\*.*; DestDir: {app}\www\www3; Components: WWW
Source: ..\www\www4\*.*; DestDir: {app}\www\www4; Components: WWW



Source: AddPath\Release\AddPath.exe; DestDir: {tmp}\nt-mpich_setup; Flags: deleteafterinstall
Source: ..\lib\mpich.dll; DestDir: {sys}; Components: Cluster_Node
Source: ..\bin\SCIExt.dll; DestDir: {tmp}\nt-mpich_setup; Flags: deleteafterinstall
Source: ..\bin\PSAPI.DLL; DestDir: {tmp}\nt-mpich_setup; Flags: deleteafterinstall
Source: ..\examples\basic\Release\cpi.exe; DestDir: {app}\bin; Components: Examples Cluster_Frontend
Source: ..\bin\mpe_server.dll; DestDir: {app}\bin




[Components]
Name: Cluster_Frontend; Description: tools for remote execution; Types: Frontend Complete User_Defined
Name: Cluster_Node; Description: enable this computer to be a cluster node; Types: Node Complete User_Defined
Name: Examples; Description: example source code, headers and libraries; Types: Complete User_Defined
Name: Development; Description: header and library files for development; Types: Complete User_Defined
Name: Logging; Description: tools for profiling, logging and visualization; Types: Complete User_Defined Frontend
Name: WWW; Description: Web pages for MPI and MPE; Types: Complete User_Defined

[Run]
Filename: {tmp}\nt-mpich_setup\rcluma-uninstall.bat; Flags: runminimized; Components: Cluster_Node; Description: Uninstaller Cluster Manager service
Filename: {tmp}\nt-mpich_setup\rcluma-install.bat; Description: Installer Cluster Manager service; Components: Cluster_Node; Flags: runminimized
Filename: {tmp}\nt-mpich_setup\AddPath.exe; Parameters: %MPI_ROOT%\lib

[UninstallRun]
Filename: {app}\bin\rcluma-uninstall.bat; Components: Cluster_Node; Flags: runminimized

[Icons]
Name: {group}\RexecShell; Filename: {app}\bin\RexecShell.exe; WorkingDir: {app}\bin; Comment: RexecShell; Flags: createonlyiffileexists
Name: {group}\mp-mpich Manual; Filename: {app}\doc\mp-mpich_manual.pdf; Flags: createonlyiffileexists
Name: {group}\MPE Graphic Server; Filename: {app}\bin\mpe_server_frontend.exe; IconFilename: {app}\bin\mpe_server_frontend.exe; IconIndex: 0; Flags: createonlyiffileexists
Name: {group}\MPI and MPE; Filename: {app}\www\index.html; Flags: createonlyiffileexists
Name: {group}\Shell mpiexec; Filename: {cmd}; Parameters: "/k ""{sd}&&cd {app}\bin"""; Components: Cluster_Frontend
Name: {group}\Quickstart; Filename: {app}\doc\Quickstart.pdf; Flags: createonlyiffileexists

[Registry]
Root: HKLM; Subkey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: MPI_ROOT; ValueData: {app}; Flags: uninsdeletevalue

[Types]
Name: User_Defined; Description: User Defined; Flags: iscustom
Name: Complete; Description: Complete Installation
Name: Frontend; Description: Cluster Frontend
Name: Node; Description: Cluster Node

[InstallDelete]
Name: {tmp}\nt-mpich; Type: dirifempty
