' $Id: runtests.vbs,v 1.2 2000/12/04 08:12:02 karsten Exp $

' This VBScript code will bbe included by the runtests.vbs
' scripts located in the subdirectories.
' Please call them with cscript.
' E.g. cscript runtests.vbs

Option explicit

CONST TESTFILE="TESTFILE.DAT"
const dsw="test.dsw"

Dim User
Dim Domain
Dim Password 
Dim host
Dim DevPath

Dim FSO,File,Shell,exec,run,Dir
Main

Sub Print(txt) 
WScript.echo txt  
End Sub

Sub RunTest(n,m)
Call RT(n,m,"*** "&n&" ***","")
End Sub

Sub RunTest2(name,num,str)
Call RT(name,num,str,"")
End Sub

Sub RunTest3(name,num,str,files) 
Call RT(name,num,str,files)
End Sub

Sub RT(name,num,str,files)
	Dim file,filename,command,res
	On Error Resume next
	if CreateBinary(name) <> TRUE Then
		Print "Could not create binary """ & name & ".exe"""
		Set file = fso.OpenTextFile("comp.out", 1,FALSE)
		if(Err.Number = 0) Then
			command = file.ReadAll
			Print command
			file.close
		End If
		Err.Clear
		Exit Sub
	End If
	filename = name&".txt"
	Print(str)
	command = "cmd /C """ & RUN & " -wdir " & dir & " -path " & dir & " -n " & num & " "& name & "  -fname "&TESTFILE& "  1>  " & dir&"\"&filename & " 2>&1"""
	'Print command
	res = Shell.Run(command,0,TRUE)
	if(Err.Number <> 0) Then
		Print "Could not start " &Chr(10) & Chr(13) & command
		Err.Clear
	End If

	res = Shell.run("cmd /C "" fc /N /L /w " & filename & " std\"& name& ".std 1> fc_out.txt  2>&1""",0,TRUE)
	Set file = fso.OpenTextFile("fc_out.txt", 1,FALSE)
	if(Err.Number = 0) Then
		command = file.ReadAll
		Print command
		file.close
	Else 
		Print "FC failed!"
		Err.Clear
	End If
	fso.DeleteFile(filename)
End Sub

Sub AppendFiles(ofile,files) 
Dim Names,ifile,name
On Error Resume next
if files = "" then Exit Sub
Names = Split(files)
For Each name in names
	set ifile = FSO.OpenTextFile(name)
	if Err.Number = 0 Then
		Do While ifile.AtEndOfStream <> True
			ofile.WriteLine(iFile.ReadLine)
		Loop
		ifile.close
		Call fso.deleteFile(name,TRUE)
	Else Err.Clear
	End If
Next
End Sub

Function CreateBinary(name) 
	Dim exe,Command
	exe = name & ".exe"
	if fso.FileExists(exe) Then
		CreateBinary = TRUE
		Exit Function
	End If
	Command = DevPath & " " & dsw & " /out comp.out  /make """ & name & " - Win32 Release"""
	Call Shell.run(command,0,TRUE)
	if Err.Number <> 0 Then
		Print "Could not start " &Chr(10) & Chr(13) & command
		Err.Clear
		CreateBinary = FALSE
		Exit Function
	End If
	if fso.FileExists(exe) Then
		CreateBinary = TRUE
	Else
		CreateBinary = FALSE
	End If

End Function

Function GetDir
Dim d
Dim Drive
Dim DriveAndPath

D = fso.GetParentFolderName(WScript.ScriptFullName)
if InStrRev(D,":") =2 Then
	DriveAndPath = Split(D,":",2,1)
	set Drive = fso.GetDrive(fso.GetDriveName(D))
	if Drive.ShareName = "" then
		if MsgBox("This is not a network file system." & Chr(10) &Chr(13) & "Continue nevertheless?",vbYesNo+vbQuestion) = vbNo Then 
			D = ""
		End If	
	Else 
		D = Drive.ShareName & DriveAndPath(1)
	End If
End If
GetDir = D
End Function

Sub GetAccount
	Do
		User = InputBox("Please enter a username","User")
	Loop Until User <> ""
	Do
		Domain = InputBox("Please enter domain for "& User,"Domain")
	Loop Until Domain <> ""

	Do
		Password = InputBox("Please enter Password for "&Domain&"/"&User,"Password")
	Loop Until Password <> ""

	host  = InputBox("Please enter a comma seperated list of hosts (optinal)"&chr(13)&"Please don't use any spaces!","Hosts")
	if Host <> "" Then Host = " -host " & Host
	User = " -user " & User
	Domain = " -domain " & Domain
	Password = " -password " & Password
End Sub

Sub RunAll
Print "**** Testing I/O functions ****"


RunTest2 "simple",4,"**** Testing simple.c ****"
RunTest2  "async",4,"**** Testing async.c ****"
RunTest2 "atomicity",4, "**** Testing atomicity.c ****"
RunTest2 "coll_test",4, "**** Testing coll_test.c ****"
RunTest2 "excl",4,"**** Testing excl.c ****"
RunTest2 "file_info",4,"**** Testing file_info.c ****"
RunTest2 "i_noncontig",2,"**** Testing i_noncontig.c ****"
RunTest2 "noncontig",2, "**** Testing noncontig.c ****"
RunTest2 "noncontig_coll",2, "**** Testing noncontig_coll.c ****"
RunTest2 "misc",4,"**** Testing misc.c ****"
RunTest2 "shared_fp",4,"**** Testing shared_fp.c ****"
RunTest2 "split_coll",4, "**** Testing split_coll.c ****"
RunTest2 "psimple",4, "**** Testing psimple.c ****"

End Sub


Sub Main
Dim DllName, Env
set Shell = WScript.CreateObject("WScript.Shell") 
set FSO = WScript.CreateObject("Scripting.FileSystemObject")
On Error Resume next
DevPath = Shell.RegRead("HKLM\Software\Microsoft\VisualStudio\6.0\Setup\VsCommonDir")
DevPath = DevPath & "\MsDev98\bin\msdev.exe"
if(Err.Number <> 0) or (fso.FileExists(DevPath) <> TRUE) Then
	Err.Clear
	if MsgBox("Could not find Visual Studio (msdev.exe)." & Chr(10) &Chr(13) & "Continue nevertheless?",vbYesNo+vbQuestion) = vbNo Then 
		Exit Sub
	Else
		DevPath = "msdev.exe"
	End If
End If
DevPath = """" & DevPath & """"
DllName = Shell.ExpandEnvironmentStrings("%MPI_ROOT%")

if DllName = "%MPI_ROOT%" Then
	DllName = "..\.."
	DllName = fso.GetAbsolutePathName(DllName)
	Set Env = Shell.Environment("PROCESS")
	Env("MPI_ROOT") = DllName
End If

exec = DllName & "\bin\mpiexec"
DllName = DllName & "\lib\mpich.dll"

if fso.FileExists("mpich.dll") <> TRUE Then
	if fso.FileExists(DllName) <> TRUE Then
		Print "Could not find "& DllName& " Exiting"
		Exit Sub
	Else 
		call fso.copyFile(DllName,".\")
	End If
End If
Dir = GetDir 
if Dir = "" then Exit Sub End If
GetAccount
run = exec & host & User & Domain & Password
RunAll
Print "All tests finished. Deleting temorary files..."
On Error Resume next
fso.DeleteFile("fc_out.txt")
fso.deleteFile("comp.out")
fso.deleteFile("*.exe")
fso.DeleteFolder("Release")
fso.DeleteFile("mpich.dll")
fso.DeleteFile(TESTFILE&"*")
Print "Done"
End Sub

