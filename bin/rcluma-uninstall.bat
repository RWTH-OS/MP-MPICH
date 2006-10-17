@echo off
echo.
echo This will remove the cluster manager sevice V2 from your system.

net stop rcluma
%SYSTEMROOT%\system32\rclumad -uninstall 
del %SYSTEMROOT%\system32\rclumad.exe 
del %SYSTEMROOT%\system32\SCIExt.dll

