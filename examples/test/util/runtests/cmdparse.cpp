
#include <iostream>
#include <tchar.h>
#include <malloc.h>
#include <string>
#include <iomanip>

#include "cmdparse.h"
#include "args.h"
#include "Encryption.h"
#include "FileFkts.h"



BOOL verbose=FALSE;
BOOL debug_flag=FALSE;



#define NOTE(m) if(verbose) std::cerr<<m<<std::endl
#define SPACE (std::string)"  "<<std::setw(22)

BOOL LoadAccount(char *AccountName,char *UserName,char *Domain,char *Password);


int Usage(void) {
	long flags;
	flags = std::cerr.flags();
	flags |= std::ios::left;
	std::cerr.flags(flags);
	std::cerr<<"Usage:\nruntests [options] [mpiexec-options]\n";
	std::cerr<<"runtests uses mpiexec to run tests for a number of executables.\n";
	std::cerr<<"The names of the executables are extracted from the file projects.txt.\n";
	std::cerr<<"For each executable a file <exename>.txt is generated containing the output.\n";
	std::cerr<<"This file is compared with <exename>.std and compare information is stored in <exename>_cmp.txt.\n";
	std::cerr<<"\nValid options are:\n";
	std::cerr<<SPACE<<"-?"<<"Print this message.\n";
	std::cerr<<SPACE<<"-account name"<<"Load account 'name' from registry.\n";
	std::cerr<<SPACE<<"-delcmp"<<"Delete file <exename>_cmp.txt after run.\n";
	std::cerr<<SPACE<<"-delequal"<<"Delete <exename>.txt and _cmp.txt if txt and std equal.\n";
	std::cerr<<SPACE<<"-deltxt"<<"Delete file <exename>txt after run.\n";
	std::cerr<<SPACE<<"-domain name"<<"Use 'name' as domain for account.\n";
	std::cerr<<SPACE<<"-eachexe"<<"Run test for each executable in working directory.\n";
	std::cerr<<SPACE<<"-findproject"<<"Run test for executables with vcproj-file.\n";
	std::cerr<<SPACE<<"-help"<<"See -?.\n";
	std::cerr<<SPACE<<"-password pass"<<"Use 'pass' as password for account.\n";
	std::cerr<<SPACE<<"-smpiexec"<<"Show which mpiexec is used.\n";
	std::cerr<<SPACE<<"-showall"<<"Show information of each filecompare run.\n";
	std::cerr<<SPACE<<"-showdifferent"<<"Show information of filecompare runs with differences.\n";
	std::cerr<<SPACE<<"-showpassed"<<"Show names of executables with passed tests.\n";
	std::cerr<<SPACE<<"-user name"<<"Use 'name' as login name for account.\n";
	std::cerr<<SPACE<<"-vruntests"<<"Show version of runtests.exe.\n";
	std::cerr<<SPACE<<"-wdirtests dir"<<"Set 'dir' as working directory for tests.\n";
	flags &= ~std::ios::left;
	std::cerr.flags(flags);
	std::cerr<<"For mpiexec options see the mpiexec documentation\n";
	return 1;
}

void ParseCommandline(int *ArgsEnd,char **argv,Creds* UserInfo, DWORD& flags,char ** WDir){

	char *Domain,
		*Password,*User,
		*Account;

	if(IsArgPresent(ArgsEnd,argv,"-?")||IsArgPresent(ArgsEnd,argv,"-help"))
	{
		Usage();
		exit(2);
	}
	if(IsArgPresent(ArgsEnd,argv,"-showpassed"))
		flags |= DISPLAY_PASSED;
	if(IsArgPresent(ArgsEnd,argv,"-showdifferent"))
		flags |= DISPLAY_DIFFERENT;
	if(IsArgPresent(ArgsEnd,argv,"-showall"))
		flags |= DISPLAY_ALL;
	if(IsArgPresent(ArgsEnd,argv,"-findprojects"))
		flags |= FIND_PROJECTS;
	if(IsArgPresent(ArgsEnd,argv,"-eachexe"))
		flags |= EACH_EXE;
	if(IsArgPresent(ArgsEnd,argv,"-delcmp"))
		flags |= DEL_CMP;
	if(IsArgPresent(ArgsEnd,argv,"-deltxt"))
		flags |= DEL_TXT;
	if(IsArgPresent(ArgsEnd,argv,"-delequal"))
		flags |= DEL_EQUAL;
	if(IsArgPresent(ArgsEnd,argv,"-smpiexec"))
		flags |= SHOW_MPIEXEC_PATH;
	if(IsArgPresent(ArgsEnd,argv,"-vruntests"))
		flags |= SHOW_RUNTESTS_VERSION;
		

	
	*WDir=NULL;
	char * tmpDir = NULL;
	
	GetStringArg(ArgsEnd,argv,"-wdirtests",&tmpDir);
	if(!(tmpDir)) {
		//no given working directory -> use directory of runtests location
		(*WDir) = (char*) calloc(MAX_PATH,sizeof(char));
		tmpDir = (char*) calloc(MAX_PATH,sizeof(char));
		GetFullExeName(tmpDir);
		ExtractFilePath(tmpDir,tmpDir);
		strcpy(*WDir,tmpDir);
		free (tmpDir); tmpDir=NULL;
	}
	else
	{	
		//copy content to new string instead of using pointer to original argument
		(*WDir) = (char*) calloc(MAX_PATH,sizeof(char));
		strcpy(*WDir,tmpDir);
	}



	char * strptr = &(*WDir)[strlen(*WDir)];
	if (strptr[0]!='\\')
	{
		strptr[0]='\\';
		strptr++;
		strptr[0]='\0';
	}

	Domain = 0;
	Password = 0;
	User = 0;
	Account = 0;

	if (*ArgsEnd == 1)
	{
		//

	}

	

	GetStringArg(ArgsEnd,argv,"-account",&Account);
	if(Account){
		User = (char*)malloc(50*sizeof(char));
		Domain = (char*)malloc(50*sizeof(char));
		Password = (char*)malloc(128*sizeof(char));
		if(!LoadAccount(Account,User,Domain,Password)){
			std::cerr<<"Cannot load account "<<Account<<" from registry\n";
			free(User);
			User = 0;
			free(Domain);
			Domain = 0;
			free(Password);
			Password = 0;
		}
		else
		{
			NOTE("Loaded account User:"<<User<<" Domain:"<<Domain<<" from registry");
		}
	}


	GetStringArg(ArgsEnd,argv,"-user",&User);
	GetStringArg(ArgsEnd,argv,"-domain",&Domain);
	GetStringArg(ArgsEnd,argv,"-password",&Password);


	if(!User){
		User = (char*)malloc(50*sizeof(char));
		std::cout<<"Login: "<<std::flush;
		std::cin>>User;
	}

	if(!Domain){
		Domain = (char*)malloc(50*sizeof(char));
		std::cout<<"Domain: ";
		std::cin>>Domain;
	}

	if(!Password){
		DWORD fdwOldMode,fdwMode;
		HANDLE hStdin=GetStdHandle(STD_INPUT_HANDLE);

		Password = (char*)malloc(128*sizeof(char));
		GetConsoleMode(hStdin, &fdwOldMode);
		fdwMode = fdwOldMode & ~ENABLE_ECHO_INPUT;
		SetConsoleMode(hStdin, fdwMode);
		std::cout<<"Password: "<<std::flush;
		std::cin>>Password;
		std::cout<<std::endl;
		SetConsoleMode(hStdin, fdwOldMode);
	}

	strcpy(UserInfo->Name,User);
	strcpy(UserInfo->Domain,Domain);
	strcpy(UserInfo->Password,Password);




	/*if(Password) {
		free(Password);
		Password=NULL;
	}
	if(User) {
		free(User);
		User=NULL;
	}
	if(Domain) {
		free(Domain);
		Domain=NULL;
	}
	if(Account) {
		free(Account);
		Account=NULL;
	}*/
}

/*************************************************************************************/

#define CREDKEY "Software\\lfbs\\rexec\\Credentials"
BOOL LoadAccount(char *AccountName,char *UserName,char *Domain,char *Password) {
	
	LONG lError;
	HKEY hKey;
	DWORD MaxValSize;
	PBYTE ValueData;
	DWORD ValueType;
	Creds *result;	

	lError=RegOpenKeyEx(HKEY_CURRENT_USER,CREDKEY,0,KEY_READ,&hKey);

	if(lError!=ERROR_SUCCESS) {
		SetLastError(lError);
		return FALSE;
	}

	lError=RegQueryInfoKey(hKey,0,0,0,0,0,0,0,0,&MaxValSize,0,0);
	if(lError !=ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(lError);
		return FALSE;
	}
	ValueData=(PBYTE)alloca(MaxValSize);

	lError=RegQueryValueEx(hKey,AccountName,0,&ValueType,(PBYTE) ValueData,&MaxValSize);
	if(lError !=ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(lError);
		return FALSE;
	}
	RegCloseKey(hKey);

	if(!DecryptData((char*)ValueData,&MaxValSize)) 
		return FALSE;

	result=(Creds*)(ValueData+MaxValSize);

	strcpy(UserName,result->Name);
	strcpy(Domain,result->Domain);
	strcpy(Password,result->Password);

	return TRUE;
}//LoadAccount



