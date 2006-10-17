// test.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <tchar.h>
#include <string>
#include <stdio.h>
#include <Windows.h>
#include "runtest.h"
#include "cmdparse.h"
#include "FileFkts.h"
#include "args.h"

using namespace std;



char * WinSysDir;


BOOL CreateRemoteProcess(const char * AppName,const char * BaseCmdLine
						 ,const char * RemoteApp, const char * WDir)
{
	ifstream stdoutfile;
	string captionstring;
	
	DWORD NumBytesToWrite = 0,NumWritten =0;
	char  *CmdLine, *NameOutputFile, *NameErrorFile;
	DWORD CreationFlags = NORMAL_PRIORITY_CLASS , errcode = NOERROR;
	LPSTARTUPINFO lpmyStartupInfo;
	PROCESS_INFORMATION myProcessInformation;
	SECURITY_ATTRIBUTES sa;
	char * ProcEnvironment = NULL;
	char * CurDir = NULL;
	BOOL success = FALSE;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	CmdLine = (char*) calloc(CMDLENGHT,sizeof(char));
	
	NameOutputFile = (char*) calloc(CMDLENGHT,sizeof(char));
	NameErrorFile = (char*) calloc(CMDLENGHT,sizeof(char));
	lpmyStartupInfo = (LPSTARTUPINFO) calloc (1,sizeof(STARTUPINFO));

	//Create file for output of remote processes
	strcpy(NameOutputFile,WDir);
	strcat(NameOutputFile,RemoteApp);
	strcat(NameOutputFile,OUTPUT_EXT);

	//Create file for error-output of remote processes
	strcpy(NameErrorFile,WDir);
	strcat(NameErrorFile,RemoteApp);
	strcat(NameErrorFile,ERROR_EXT);



	if (FileExists(NameOutputFile))
	{
		DeleteFile(NameOutputFile);
	}

	if (FileExists(NameErrorFile))
	{
		DeleteFile(NameErrorFile);
	}
	HANDLE OutFileHandle = CreateFile(NameOutputFile, GENERIC_WRITE, 
		0, &sa, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,NULL);//FILE_SHARE_WRITE

	if (OutFileHandle == INVALID_HANDLE_VALUE){
		cerr<<"Error creating OutPutFile "<<NameOutputFile<<endl;
		exit(2);
	}

	HANDLE ErrorFileHandle = CreateFile(NameErrorFile, GENERIC_WRITE, 
		0, &sa, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,NULL);//FILE_SHARE_WRITE

	if (ErrorFileHandle == INVALID_HANDLE_VALUE){
		cerr<<"Error creating ErrorOutPutFile "<<NameErrorFile<<endl;
		exit(2);
	}

	lpmyStartupInfo->hStdOutput = OutFileHandle;
	lpmyStartupInfo->hStdError = ErrorFileHandle;
	lpmyStartupInfo->dwFlags |= STARTF_USESTDHANDLES;

	//generate parameters for mpiexec
	strcpy(CmdLine,BaseCmdLine);
	strcat (CmdLine," -- ");
	strcat(CmdLine,RemoteApp);

	success = CreateProcess(AppName,CmdLine,NULL,NULL,TRUE,
		CreationFlags,ProcEnvironment,CurDir,lpmyStartupInfo,&myProcessInformation);
	if (!success)
	{
		cerr<<"CreateProcess for "<<RemoteApp<< " failed with errorcode "<<GetLastError()<<endl;
	}
	WaitForSingleObject(myProcessInformation.hProcess,INFINITE);
	CloseHandle(OutFileHandle);	
	CloseHandle(ErrorFileHandle);

	if (FileGetSize(NameErrorFile)==0)
		DeleteFile(NameErrorFile);

	free (CmdLine);
	free (NameOutputFile);
	free (lpmyStartupInfo);
	return success;

}
BOOL CreateCompareFile(const char * RemoteApp, const char * WDir,DWORD flags = NULL)
{
	ifstream stdoutfile;
	string captionstring;

	DWORD NumBytesToWrite = 0,NumWritten =0;
	char  *CmdLine = NULL, *NameOutputFile = NULL, *NameSuccessFile = NULL;
	DWORD CreationFlags = NORMAL_PRIORITY_CLASS , errcode = NOERROR;
	LPSTARTUPINFO lpmyStartupInfo;
	PROCESS_INFORMATION myProcessInformation;
	SECURITY_ATTRIBUTES sa;
	char * ProcEnvironment = NULL;
	char * CurDir = NULL;
	char  * AppName = NULL;
	BOOL success = FALSE;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	CmdLine = (char*) calloc(CMDLENGHT,sizeof(char));

	NameOutputFile = (char*) calloc(CMDLENGHT,sizeof(char));
	NameSuccessFile = (char*) calloc(CMDLENGHT,sizeof(char));
	lpmyStartupInfo = (LPSTARTUPINFO) calloc (1,sizeof(STARTUPINFO));

	
	lpmyStartupInfo->dwFlags = NULL;

/*	//Create file for output of remote processes
	strcpy(NameOutputFile,WDir);
	strcat(NameOutputFile,RemoteApp);
	strcat(NameOutputFile,OUTPUT_EXT);
	
	AppName = (char *) calloc (CMDLENGHT,sizeof(char));
	strcpy(AppName,WinSysDir);
	strcat(AppName,"\\cmd.exe");
	//generate parameters for fc
	strcpy(CmdLine,AppName);
	strcat(CmdLine," /C fc /N /L /w ");
	strcat(CmdLine,NameOutputFile);
	strcat(CmdLine," ");
	ChangeFileExt(NameOutputFile,SUCCESS_EXT);
	strcat(CmdLine,NameOutputFile);
	ChangeFileExt(NameOutputFile,"");
	strcat(CmdLine," 1> ");
	strcat(CmdLine,NameOutputFile);
	strcat(CmdLine,CMP_EXT);
	strcat(CmdLine," 2>&1");*/

	//search for success file
	strcpy(NameSuccessFile,WDir);
	strcat(NameSuccessFile,RemoteApp);
	strcat(NameSuccessFile,SUCCESS_EXT);

	//Create file for output of remote processes
	strcpy(NameOutputFile,WDir);
	strcat(NameOutputFile,RemoteApp);
	strcat(NameOutputFile,CMP_EXT);

	if (!FileExists(NameSuccessFile))
	{
		//FILE *.std does not exists -> message in output file
		FILE *outputfile;
		outputfile = fopen( NameOutputFile, "w" );
		fprintf(outputfile, "ERROR: File %s does not exist\n",NameSuccessFile);
		fclose( outputfile );

	}
	else
	{
		//FILE *.std exists -> check for differences			

		if (FileExists(NameOutputFile))
		{
			DeleteFile(NameOutputFile);
		}

		HANDLE OutFileHandle = CreateFile(NameOutputFile, GENERIC_WRITE, 
			0, &sa, OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL);//FILE_SHARE_WRITE

		lpmyStartupInfo->hStdOutput = OutFileHandle;
		lpmyStartupInfo->hStdError = OutFileHandle;
		lpmyStartupInfo->dwFlags |= STARTF_USESTDHANDLES;

		//Create file for output of remote processes
		strcpy(NameOutputFile,WDir);
		strcat(NameOutputFile,RemoteApp);

		AppName = (char *) calloc (CMDLENGHT,sizeof(char));
		// use chkresult.exe which is located in the same directory as runtests
		GetFullExeName(CmdLine);
		ChangeFileName(CmdLine,"chkresult");
		strcpy(AppName,CmdLine);
		strcat(CmdLine," -v ");
		strcat(CmdLine,NameOutputFile);

		success = CreateProcess(AppName,CmdLine,NULL,NULL,TRUE,
			CreationFlags,ProcEnvironment,CurDir,lpmyStartupInfo,&myProcessInformation);
		if (!success)
		{
			cerr<<"CreateProcess with \n"<<CmdLine<< " \nfailed with errorcode "<<GetLastError()<<endl;
		}
		WaitForSingleObject(myProcessInformation.hProcess,INFINITE);
		CloseHandle(OutFileHandle);	

		ChangeFileExt(NameOutputFile,OUTPUT_EXT);
		if ((flags & DEL_TXT) == DEL_TXT)
			DeleteFile(NameOutputFile);


	}
	if (CmdLine) free (CmdLine);
	if (NameOutputFile) free (NameOutputFile);
	if (NameSuccessFile) free (NameSuccessFile);
	if (lpmyStartupInfo) free (lpmyStartupInfo);
	if (AppName) free (AppName);
	return success;

}
BOOL CheckCmpFile(const char * RemoteApp, const char * WDir,DWORD flags = NULL)
{
	char * CompareFile = NULL;
	char * CompareFileContent = NULL;
	BOOL different = TRUE;
	BOOL print_CompareFileContent=FALSE;
	CompareFile = (char*) calloc(CMDLENGHT,sizeof(char));
	strcpy(CompareFile,WDir);
	strcat(CompareFile,RemoteApp);
	strcat(CompareFile,CMP_EXT);

	DWORD mySize = 0;
	mySize = FileGetSize(CompareFile);
	if (mySize == 0)
		DeleteFile(CompareFile);
	if(!FileExists(CompareFile))
	{
		different = FALSE;
	}

/*	if(!FileExists(CompareFile))
	{
		free (CompareFile);
		return different;
	}
	
	HANDLE OutFileHandle = INVALID_HANDLE_VALUE;
	DWORD numRead = 0;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	mySize = FileGetSize(CompareFile);
	CompareFileContent = (char *)calloc(mySize+1,1);
	OutFileHandle = CreateFile(CompareFile, GENERIC_READ, 
		0, &sa, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (OutFileHandle == INVALID_HANDLE_VALUE){
		free(CompareFile);
		free(CompareFileContent);
		return different;
	}
	ReadFile(OutFileHandle,CompareFileContent,mySize,&numRead,FALSE);
	CloseHandle(OutFileHandle);

	
	int cmpresult = 0;
	char * cmppos = NULL;
	cmppos = strstr((char*)CompareFileContent,DIFF_INDICATOR);
	if (cmppos == NULL)
		different = FALSE;
	cmppos = strstr((char*)CompareFileContent,NO_DIFFERENCE);
	if (cmppos != NULL)
		different = FALSE;

*/
	if ((flags & DISPLAY_ALL) == DISPLAY_ALL)
		print_CompareFileContent=TRUE;
	else 
		if (((flags & DISPLAY_DIFFERENT) == DISPLAY_DIFFERENT) && different)
			print_CompareFileContent=TRUE;


	if (print_CompareFileContent)
	{
		HANDLE OutFileHandle = INVALID_HANDLE_VALUE;
		DWORD numRead = 0;
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;
		mySize = FileGetSize(CompareFile);
		CompareFileContent = (char *)calloc(mySize+1,1);
		OutFileHandle = CreateFile(CompareFile, GENERIC_READ, 
			0, &sa, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,NULL);
		if (OutFileHandle == INVALID_HANDLE_VALUE){
			free(CompareFile);
			free(CompareFileContent);
			return different;
		}
		ReadFile(OutFileHandle,CompareFileContent,mySize,&numRead,FALSE);
		CloseHandle(OutFileHandle);
		printf(CompareFileContent);
	}

	if ((flags & DEL_CMP) == DEL_CMP)
		DeleteFile(CompareFile);

	if (!different && ((flags & DEL_EQUAL) == DEL_EQUAL))
	{
		DeleteFile(CompareFile);
		strcpy(CompareFile,WDir);
		strcat(CompareFile,RemoteApp);
		strcat(CompareFile,OUTPUT_EXT);
		DeleteFile(CompareFile);
	}

	if(CompareFileContent)
	  free(CompareFileContent);
	free (CompareFile);

	return different;
}

BOOL GetExeList(vector<string>& ExeList,DWORD& flags,char * WDir)
{
	BOOL success = false;
	BOOL use_project_file=true ;
	char * FileName =NULL;
	char * FoundFile = NULL;
	
	if ((flags&FIND_PROJECTS)==FIND_PROJECTS)
	{
		use_project_file= false;
		FileName = (char *) calloc (strlen(WDir)+15,sizeof(char));
		FoundFile = (char *) calloc (CMDLENGHT,sizeof(char));
		strcpy(FileName,WDir);
		strcat(FileName,"*.vcproj");

	}
	if ((flags&EACH_EXE)==EACH_EXE)
	{
		use_project_file= false;
		FileName = (char *) calloc (strlen(WDir)+15,sizeof(char));
		FoundFile = (char *) calloc (CMDLENGHT,sizeof(char));
		strcpy(FileName,WDir);
		strcat(FileName,"*.exe");
	
	}

	if(!use_project_file)
	{
		LPWIN32_FIND_DATA lpFindFileData;
		lpFindFileData = (LPWIN32_FIND_DATA) calloc (1,sizeof(WIN32_FIND_DATA));

		HANDLE myhandle = FindFirstFile(FileName,lpFindFileData);
		if (myhandle != INVALID_HANDLE_VALUE)
		{
			strcpy(FoundFile,lpFindFileData->cFileName);
			ChangeFileExt(FoundFile,"");
			ExeList.push_back(FoundFile);
			while(FindNextFile(myhandle,lpFindFileData))
			{
				strcpy(FoundFile,lpFindFileData->cFileName);
				ChangeFileExt(FoundFile,"");
				ExeList.push_back(FoundFile);
			}
			
			FindClose(myhandle);
		}
		else
		{
			cerr<<"no file found matching: "<<FileName<<endl;
			exit(1);
		}
	}

	if(FileName)
		free(FileName);
	if(FoundFile)
		free(FoundFile);

	if(use_project_file)
	{
		char Project_File[300];
		strcpy(Project_File,PROJECTFILE);
		ChangeFilePath(Project_File,WDir);

		//read list with files to run from file
		if (!FileExists(Project_File))
		{
			cerr<<"file not found: "<<Project_File<<endl;
			exit(1);
			return success;
		}
		ifstream filelist;
		filelist.open(Project_File);
		if (!filelist)
		{
			cerr<<"could not open file: "<<Project_File;
			exit(1);
			return success;
		}

		string HostRead;

		while(!filelist.eof())
		{
			filelist >>HostRead;
			ExeList.push_back(HostRead);
		}
		filelist.close();
	}
	return success;

}

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc == 1) return Usage();

	DWORD flags = 0;
  	
//get user, domain, password
	Creds UserInfo;
	int ArgsEnd = argc;
	char * WDir;
	ParseCommandline(&ArgsEnd,argv,&UserInfo,flags,&WDir);

//Display copyright
	cout<<endl<<COPYRIGHT<<endl;
//Display additional information about runtests if requested
	char * AppName;
	DWORD  errcode = NOERROR;
	AppName = (char*) calloc(CMDLENGHT,sizeof(char));
	errcode = GetEnvironmentVariable("MPI_ROOT",AppName,CMDLENGHT);
	strcat (AppName,"\\bin\\mpiexec.exe ");
	if((flags & SHOW_MPIEXEC_PATH)==SHOW_MPIEXEC_PATH)
		cout<<endl<<"Using "<<AppName<<endl;
	if((flags & SHOW_RUNTESTS_VERSION)==SHOW_RUNTESTS_VERSION)
		cout<<endl<<"Using runtests version no "<<MAJORVERION<<"."<<MINORVERSION<<endl;

	
	if (!FileExists(AppName))
	{
		cerr<<"file not found :"<<AppName<<endl;
		exit(1);
	}
 

//Get executables to test
	
	 
	vector<string> ExeList;
	GetExeList(ExeList,flags,WDir);

	WinSysDir = (char *)calloc (CMDLENGHT,sizeof(char));
	GetSystemDirectory(WinSysDir,CMDLENGHT);

//start processes
	int i;
	char *BaseCmdLine, *CmdLine, *NameOutputFile;
	DWORD CreationFlags = NORMAL_PRIORITY_CLASS ;
	LPSTARTUPINFO lpmyStartupInfo;
	char * ProcEnvironment = NULL;
	char * CurDir = NULL;
	BOOL success = FALSE;
	
	CmdLine = (char*) calloc(CMDLENGHT,sizeof(char));
	BaseCmdLine = (char*) calloc(CMDLENGHT,sizeof(char));	
	NameOutputFile = (char*) calloc(CMDLENGHT,sizeof(char));	
	lpmyStartupInfo = (LPSTARTUPINFO) calloc (1,sizeof(STARTUPINFO));


//create general command line containing mpiexec and common parameters	
	strcpy (BaseCmdLine,AppName);
	strcat (BaseCmdLine,"-user ");
	strcat (BaseCmdLine,UserInfo.Name);
	strcat (BaseCmdLine," -domain ");
	strcat (BaseCmdLine,UserInfo.Domain);
	strcat (BaseCmdLine," -password ");
	strcat (BaseCmdLine,UserInfo.Password);
	strcat (BaseCmdLine," -wdir ");
	strcat (BaseCmdLine,WDir);
	strcat (BaseCmdLine," -path ");
	strcat (BaseCmdLine,WDir);

	//copy additional commandline parameters to mpiexec parameters
	if (ArgsEnd > 1)
	{
		int i;
		for (i=1;i<ArgsEnd;i++)
		{
			strcat (BaseCmdLine," ");
			if (strchr(argv[i],' '))
			{   //this is a combined parameter -> add quotes
				strcat (BaseCmdLine,"\"");
				strcat (BaseCmdLine,argv[i]);
				strcat (BaseCmdLine,"\"");
			}
			else
			  strcat (BaseCmdLine,argv[i]);
		}
	}

	for(i=0;i<ExeList.size();i++)
	{
		CreateRemoteProcess(AppName,BaseCmdLine,ExeList[i].c_str(),WDir);
		CreateCompareFile(ExeList[i].c_str(),WDir,flags);
		if (CheckCmpFile(ExeList[i].c_str(),WDir,flags))
		{
			cout<<"*****  differences at test "<<ExeList[i].c_str()<<" occured *****"<<endl;
		}
		if((flags & DISPLAY_PASSED)==DISPLAY_PASSED)
			cout<<"passed test "<<ExeList[i].c_str()<<endl;

	}

	free (WinSysDir);

	return 0;
}

