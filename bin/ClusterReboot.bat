@ echo off

set /a index=0 

set P2_CLUSTER=p2-00,p2-01,p2-02,p2-03
set P3_CLUSTER=p3-01,p3-02,p3-03,p3-04,p3-05,p3-06,p3-07,p3-08
set P4_CLUSTER=p4-01,p4-02,p4-03,p4-04,p4-05,p4-06,p4-07,p4-08


rem if Not Exist .\mpiexec.exe echo could not find .\mpiexec.exe & goto :DONE

if "%1"=="" (goto :HOWTOUSE)

:NEXT_ARG
if {%1}=={} goto :DONE
set /a index+=1
call :PROCESS_ARG %1
shift /1
goto :NEXT_ARG

:HOWTOUSE
echo usage: ClusterReboot [P2_CLUSTER/P3_CLUSTER/P4_CLUSTER/Node1 Node2 Node3]
echo will reboot the given nodes to default operating system
echo hint: set environment variable CLUSTER_ACCOUNT for identification
goto :DONE

:PROCESS_ARG
rem echo Parameter: %1

set MPI_COMMAND=mpiexec -reboot

IF defined CLUSTER_ACCOUNT set MPI_COMMAND=mpiexec -account %CLUSTER_ACCOUNT% -reboot
	
rem echo MPI_COMMAND=%MPI_COMMAND%
if {%1}=={P2_CLUSTER} set MPI_COMMAND=%MPI_COMMAND% -hostlist -host %P2_CLUSTER% & goto :EXECUTE

if {%1}=={P3_CLUSTER} set MPI_COMMAND=%MPI_COMMAND% -hostlist -host %P3_CLUSTER% & goto :EXECUTE

if {%1}=={P4_CLUSTER} set MPI_COMMAND=%MPI_COMMAND% -hostlist -host %P4_CLUSTER% & goto :EXECUTE

set MPI_COMMAND=%MPI_COMMAND% %1

:EOF

:EXECUTE
echo executing %MPI_COMMAND%
%MPI_COMMAND%

rem If Errorlevel 1 Echo Last command did not complete successfully

:EOF

:DONE 

rem echo you specified %index% parameters