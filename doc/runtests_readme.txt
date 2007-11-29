runtests is a program for Windows platforms to run the tests in the directory %MPI_ROOT%\examples\test automatically
you can also use this program to run tests for your own executables


runtests uses mpiexec to run tests for a number of executables.
The names of the executables are extracted from the file projects.txt in the working directory.
For each executable a file <exename>.txt is generated containing the output of the executable.
This file is compared with <exename>.std and compare information is stored in <exename>_cmp.txt. For this comparation the executable chkresult.exe is used.

Usage: 

runtests [options] [mpiexec-options]

Valid options are:
-?: Print usage message
-account name: Load account 'name' from registry.
-delcmp: Delete file <exename>_cmp.txt after run.
-delequal: Delete <exename>.txt and _cmp.txt if txt and std equal.
-deltxt: Delete file <exename>txt after run.
-domain name: Use 'name' as domain for account.
-eachexe: Run test for each executable in working directory.
-findproject: Run test for executables with vcproj-file.
-help: See -?.
-smpiexec: Show the path of mpiexec which is used by runtests.\n";
-password pass: Use 'pass' as password for account.
-showall: Show information of each filecompare run.
-showdifferent: Show information of filecompare runs with differences.
-showpassed: Show names of executables with passed tests.
-user name: Use: Show version of runtests.exe.
-wdirtests dir: Set 'dir' as working directory for tests.


you can also create a batch file containing the parameters suiting your needs; for example:

C:\nt-mich\bin\runtests -eachexe -wdirtests C:\nt-mpich\examples\test\context -account lfbs/user -showpassed -delequal -n 2 -host p4-01,p4-02

starts the test for each executable in directory C:\nt-mich\examples\test\context, the account lfbs/user is loaded from the registry, all passed tests are displayed, the output files <exename>.txt and <exename_cmp>.txt are deleted if no differences between <exename>.txt and <exename>.std exist