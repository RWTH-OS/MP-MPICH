@echo off
echo.
echo This will install the cluster manager sevice on your system.
echo.
echo You can uninstall the service by typing %SYSTEMROOT%\system32\clumad -uninstall
echo.

copy rclumad.exe %SYSTEMROOT%\system32
if NOT EXIST %SYSTEMROOT%\system32\psapi.dll copy psapi.dll %SYSTEMROOT%\system32
copy SCIExt.dll %SYSTEMROOT%\system32
copy rcluma.cpl %SYSTEMROOT%\system32

 %SYSTEMROOT%\system32\rclumad -install 
net start rcluma
