#include <windows.h>
#include <iomanip>
#include <string>
#include <iostream>
#include <stdio.h>

#include "cluma.h"
#include "Debug.h"
#include "RexecClient.h"
#include "args.h"
#include "Environment.h"
#include "plugin.h"
#include "asyncwindow.h"
#include "mpirun.h"
#include "update.h"
#include "RexHelp.h"


#define VERSION 1
#define SUBVERSION 96
//version 1.5 support remote reboot/shutdown
//version 1.6 changes in encryption
//version 1.7 suppress error msg on store with no executable
//           ;hostname in CreateBinding error message; add parameter noscan 
//version 1.8 specify priority for remote processes check plugin version
//version 1.9 -shutdown or -reboot accepts host list after parameter -host or from machinefile 
//            if -hostlist is parameter BootComputer 
//version 1.91 flag to switch off account check (7.7.04)
//version 1.92 sleep in input redirection for compatibility with PBS
//version 1.93 bug correction at reboot and shutdown (empty DllName)
//version 1.94 specify additional plugin parameters
//version 1.95 4.8.2005 -plugin none: no generation of plugin parameters 
//version 1.96:3.1.2007 support of RPC calls without user authentication, flag -nouser 


// For users with outdated SDKs ;)
#ifndef CREATE_WITH_USERPROFILE
#define CREATE_WITH_USERPROFILE     0x02000000
#endif

BOOL verbose = FALSE,StopIt=FALSE,debug_flag=FALSE,reboot_flag=FALSE,shutdown_flag=FALSE;
BOOL storeflag=FALSE, NoNetworkScan=FALSE;
BOOL nouser = FALSE;
HANDLE *RemoteProcs=0;
int NumStarted=0;
CAsyncWindow *MessageWin;
CRITICAL_SECTION CS;

extern DWORD ReadHosts(char *file,int np,char ***strList, int *listLen,HostData *global);
extern char *CopyString(const char *str);

#define NOTE(m) if(verbose) std::cerr<<m<<std::endl
#define SPACE (std::string)"  "<<std::setw(22)

extern "C" {
	
    DWORD WINAPI CallbackFunction(char *txt,void *) {
        std::cerr<<txt<<std::endl;
        return 0;
    }
	
    BOOL WINAPI AccountCallback( char *Account,  char *User,  char *Domain,  char *Password, void *dummy) {
        std::cout << SPACE;
        std::cout<<(std::string) Account;
        std::cout<<(std::string) Domain;
        std::cout<<(std::string)"/";
        std::cout<<(std::string)User;
        std::cout<<std::endl;
        return TRUE;
    }
	
    extern LPTSTR GetLastErrorText( DWORD error,LPTSTR lpszBuf, DWORD dwSize );
}



BOOL WINAPI CtrlHandlerFunction(DWORD dwCtrlType ) {
    std::cerr<<"\nCTRL-C pressed. Killing remote processes";
    StopIt = true;
    EnterCriticalSection(&CS);
    for(int i=0;i<NumStarted;++i) {
        if(RemoteProcs[i]) 
            KillRemoteProcess(RemoteProcs[i]);
        std::cerr<<".";
    }
    std::cerr<<std::endl;
    LeaveCriticalSection(&CS);
    return TRUE;
	
}

int Usage(void) {
    long flags;
    flags = std::cerr.flags();
    flags |= std::ios::left;
    std::cerr.flags(flags);
    std::cerr<<"Usage:\nmpiexec [options] [--] program <parameters>\n";
    std::cerr<<"Valid options are:\n";
    std::cerr<<SPACE<<"-?"<<"Print this message.\n";
    std::cerr<<SPACE<<"-account name"<<"Load account 'name' from registry.\n";
    std::cerr<<SPACE<<"-accounts"<<"List all accounts stored in registry.\n";
    std::cerr<<SPACE<<"-configfile file"<<"Load RexecShell config file 'file'.\n";
    std::cerr<<SPACE<<" "<<"This supersedes all other commandline parameters.\n";
    std::cerr<<SPACE<<"-debug"<<"Generate debug messages.\n";
    std::cerr<<SPACE<<"-domain name"<<"Use 'name' as domain for account.\n";
    std::cerr<<SPACE<<"-gc"<<"Graphical configure. Don't pass the commandline\n";
    std::cerr<<SPACE<<" "<<"to the plugin but use a dialog box instead.\n";
    std::cerr<<SPACE<<"-help"<<"See -?.\n";
    std::cerr<<SPACE<<"-host n1[,n2[,...]]"<<"Use nodes n1.... Supersedes -machinefile.\n";
    std::cerr<<SPACE<<"-lock"<<"Lock all nodes for exclusive use.\n";
    std::cerr<<SPACE<<"-loud"<<"Print informational messages.\n";
    std::cerr<<SPACE<<"-machinefile file"<<"Use 'file' as machinefile (default: machines.txt).\n";
    std::cerr<<SPACE<<"-n num"<<"Start 'num' processes.\n";
	std::cerr<<SPACE<<"-noaccountcheck"<<"Do not check if the account is valid on local host.\n";//SI 7.7.04
    std::cerr<<SPACE<<"-noscan"<<"Do not scan the network for valid hosts.\n";
	std::cerr<<SPACE<<"-nouser"<<"Create process without user account, has to be switched on in rclumad.\n";//SI 3.1.06
    std::cerr<<SPACE<<"-password pass"<<"Use 'pass' as password for account.\n";
    std::cerr<<SPACE<<"-path path"<<"Use 'path' to prefix executable name\n";
    std::cerr<<SPACE<<"-plugin file"<<"Use 'file.dll' as actual plugin (default: ch_wsock).\n";
	std::cerr<<SPACE<<"-pp \"p1[ p2[...]]\""<<"specify additional parameters for plugin\n";
    std::cerr<<SPACE<<"-plugins"<<"List available plugins.\n";
    std::cerr<<SPACE<<"-priority priority"<<"remote process priority: idle, normal (default), high\n";
    std::cerr<<SPACE<<"-profile"<<"Load user's profile on remote nodes.\n";
    //std::cerr<<SPACE<<"-put filename<<"Transfer program to specified hosts, using name filename\n";
    std::cerr<<SPACE<<"-reboot name"<<"Reboot remote computer 'name'\n";
    std::cerr<<SPACE<<"-shutdown name"<<"Shut down remote computer 'name'\n";
    std::cerr<<SPACE<<"-store"<<"Store given account in registry.\n";
    std::cerr<<SPACE<<"-test"<<"Do not start processes. Just print commands.\n";
    std::cerr<<SPACE<<"-user name"<<"Use 'name' as login name for account.\n";
    std::cerr<<SPACE<<"-version"<<"Display version number of mpiexec.\n";//si
    std::cerr<<SPACE<<"-wdir dir"<<"Set 'dir' as working directory for remote processes.\n";
    flags &= ~std::ios::left;
    std::cerr.flags(flags);
    std::cerr<<"For plugin options see the plugin's documentation\n";
    return 1;
}


void DetachPlugin(PlgDesc *plg) {
	if(!plg->Detach) return; //no plugin specified
    __try {
        if(plg->Detach) plg->Detach();
    } __except(EXCEPTION_EXECUTE_HANDLER ) {
        NOTE("Plugin caused exception during destruction\n");
    }
    FreeLibrary(plg->hLib);
}


int PluginConfigure(PlgDesc *plg,HostData *data) { 
	if(!plg->Configure) return 0; //no plugin specified
    __try {
        plg->Configure(0,GetDesktopWindow(),data,TRUE);
    } __except(EXCEPTION_EXECUTE_HANDLER ) {
        std::cerr<<"Plugin caused exception while configuring\n";
        return 1;
    }
    return 0;
}
int PluginParse(PlgDesc *plg,int *argc, char **argv,HostData *data) {
	if(!plg->ParseCommandline) return 0; //no plugin specified
    __try {
        plg->ParseCommandline(argc,argv,data);
    } __except(EXCEPTION_EXECUTE_HANDLER ) {
        std::cerr<<"Plugin caused exception while parsing commandline\n";
        return 1;
    }
	
    return 0;
}

int PluginCreateCommand(PlgDesc *plg,HostData **Servers,DWORD *NumProcs,HostData *data) {
	if(!plg->DlgClose) return 0; //no plugin specified
    __try {
        plg->DlgClose(Servers,*NumProcs,Servers,NumProcs,data);
    } __except(EXCEPTION_EXECUTE_HANDLER ) {
        std::cerr<<"Plugin caused exception while creating commandline\n";
        return 1;
    }
    return 0;
}

int compare( const void *arg1, const void *arg2 )
{
    /* Compare all of both strings: */
    return stricmp( (*( HostData** ) arg1)->Name, (*(HostData**) arg2)->Name);
}

DWORD WINAPI ReadInput(LPVOID lpParameter) {
    char buf[BUFSIZE];
    DWORD Read;
    // The thread reads from the input handle given as parameter
    // and writes the data to the first process created.
    HANDLE fd = (HANDLE)lpParameter;
    while(ReadFile(fd,buf,BUFSIZE,&Read,0)) {
//SI//02-08-2004 Sleep if Read == 0 because when started with PBS mpiexec repeats on reading 0 Bytes
		if (Read>0)
			RexecSendInput((HANDLE)RemoteProcs[0], (char*)buf, Read);
		else		
			Sleep(500);

#ifdef _DEBUG
		std::cerr<<"ReadInput:  ReadFile delivered "<<Read<<" Bytes\n";
#endif
    }
    return 0;
}

DWORD GetExePath(char *path,DWORD size) {
    char *pos;
    DWORD nchar;
	
    if(!path || !GetModuleFileName(0,path,size)) return 0;
    pos = strrchr(path,'\\');
    if(!pos) {
        if(size>2) {
            strcpy(path,".\\");
            nchar = 3;
        } else nchar = 0;
    } else {
        pos++;
        *pos=0;
        nchar = strlen(path)+1;
    }
    return nchar;
	
}

//#define PLGPATH "..\\Plugins\\libs\\"
#define PLGPATH "Plugins\\"
void ListPlugins() {
    char dir[MAX_PATH],Path[MAX_PATH];
    HANDLE hSearch;
    WIN32_FIND_DATAA FileData;
    char *Name,*pos;
	
    if(!GetExePath(Path,MAX_PATH)) return;
    sprintf(dir,"%s%s*.dll",Path,PLGPATH);
    NOTE("Searching for "<<dir);
    hSearch=FindFirstFileA(dir,&FileData);
    if(hSearch==INVALID_HANDLE_VALUE) return;
    do {
        if(!FileData.cFileName[0]) Name=FileData.cAlternateFileName;
        else Name=FileData.cFileName;
        pos = strrchr(Name,'.');
        if(pos) *pos = 0;
        std::cout<<Name<<std::endl;
    } while(FindNextFileA(hSearch,&FileData));
    FindClose(hSearch);
	
}

BOOL LoadPlugin(const char *Name,PlgDesc *desc) {
	
    char DLL[MAX_PATH],Path[MAX_PATH];
	
    if(!GetExePath(Path,MAX_PATH)) return FALSE;
	
    sprintf(DLL,"%s%s%s.dll",Path,PLGPATH,Name);
    memset(desc,0,sizeof(*desc));
	
    NOTE("Trying to load "<<DLL);
	
    desc->hLib = LoadLibraryA(DLL);
    if(!desc->hLib) {
        std::cerr<<"Could not load "<<DLL<<":\n"<<
			GetLastErrorText(GetLastError(),Path,MAX_PATH)<<std::endl;
        return FALSE;
    }
    desc->Init=(InitFunc)GetProcAddress(desc->hLib,"_PlgDescription");
	
    if(!desc->Init) {
        FreeLibrary(desc->hLib);
        std::cerr << "Could not find  _PlgDescription() in" << DLL << ":\n" <<
			(const char*)GetLastErrorText(GetLastError(),Path,MAX_PATH) << std::endl;
        return FALSE;
    }
	
	
	
	
    __try {
        desc->Init(desc);
		
        /* Si: check plugin version number*/
        if (VersionPlugin_h != desc->PluginVersionNumber)
        {
            std::cerr<<"WARNING: Pluginfile " << DLL <<" is not up to date\n";
        }
		
        if(desc->Attach) desc->Attach(GetDesktopWindow());
    } __except(EXCEPTION_EXECUTE_HANDLER ) {
        NOTE("Plugin cause exception during initialization\n");
    }
    NOTE("Plugin loaded.");
    return TRUE;
}

BOOL MakeUNCPath(const char *path,char **uncPath) {
    DWORD PathSize,RemoteLength,err;
    BOOL FreePath=FALSE;
    char *FullPath=0,drive[3],*RemoteName;
    PathSize = GetFullPathName(path,0,0,0);
    if(PathSize) {
        FullPath = (char*)malloc(PathSize+1);
        PathSize = GetFullPathName(path,PathSize+1,FullPath,&RemoteName);   
    }
    if(!PathSize) FullPath = (char*)path;
    if(FullPath[1] == ':') {
        drive[0]=FullPath[0];
        drive[1]=':';
        drive[2] = 0;
        RemoteLength = 0;
        if(WNetGetConnection(drive,FullPath,&RemoteLength) != ERROR_MORE_DATA) {
            if(FullPath==path) {
                *uncPath=CopyString(FullPath);
            }
            else *uncPath = FullPath;
            return TRUE;
        }
        RemoteName = 0;
        do {
            RemoteName = (char*) realloc(RemoteName,++RemoteLength);
            err = WNetGetConnection(drive,RemoteName,&RemoteLength);
            if(err != NO_ERROR && err != ERROR_MORE_DATA) strcpy(RemoteName,drive);
        } while(err == ERROR_MORE_DATA);
        *uncPath = (char*)malloc(strlen(FullPath)+RemoteLength);
        sprintf(*uncPath,"%s%s",RemoteName,FullPath+2);
        free(RemoteName);
        if(FullPath != path) free(FullPath);
        return TRUE;
    } else *uncPath = CopyString(FullPath);
	
    if(FullPath != path) free(FullPath);
    else *uncPath = CopyString(path);
    return TRUE;
}

void ParseCommandline(int *ArgsEnd,char **argv,HostData*** Hosts, int *NumProcs,
                      HostData *global,char **Plugin) {
	
	char *Dir,*NewDir,*Domain,
		*Password,*User,
		*Account,*Priority;
	
	Dir = 0;
	Domain = 0;
	Password = 0;
	User = 0;
	Account = 0;
	Priority = 0;
	*Plugin = "ch_wsock";
	*NumProcs = 1;
	
	memset(global,0,sizeof(*global));
	SetCommandline(global,0);
	global->Security = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
	
	if(IsArgPresent(ArgsEnd,argv,"-soft")) {
		std::cerr<<"Sorry, -soft is not supported\n";
	}
	
	if(IsArgPresent(ArgsEnd,argv,"-arch")) {
		std::cerr<<"Sorry, -arch is not supported\n";
	}
	if(GetStringArg(ArgsEnd,argv,"-file",&Account)) {
		Account = 0;
		std::cerr<<"Sorry, -file is not supported\n";
	}
	
	
	GetIntArg(ArgsEnd,argv,"-n",(int*)NumProcs);
	GetStringArg(ArgsEnd,argv,"-account",&Account);
	if(Account) {
		User = (char*)malloc(50*sizeof(char));
		Domain = (char*)malloc(50*sizeof(char));
		Password  = (char*)malloc(128*sizeof(char));
		if(!LoadAccount(Account,User,Domain,Password)) {
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
			NOTE("Loaded account User:"<<User<<"  Domain:"<<Domain<<" from registry");
		}
	}
	
	GetStringArg(ArgsEnd,argv,"-wdir",&Dir);
	if(!Dir) {
		Dir = (char*)malloc(MAX_PATH*sizeof(char));
		GetCurrentDirectory(MAX_PATH,Dir);
		MakeUNCPath(Dir,&NewDir);
		free(Dir);
	} else {
		MakeUNCPath(Dir,&NewDir);
	}
	
	ProcSetString(&global->ProcData->WorkingDir,NewDir,&global->ProcData->WDSize);
	free(NewDir);
	
	
	GetStringArg(ArgsEnd,argv,"-user",&User);
	GetStringArg(ArgsEnd,argv,"-domain",&Domain);
	GetStringArg(ArgsEnd,argv,"-password",&Password);
	GetStringArg(ArgsEnd,argv,"-plugin",Plugin);
	global->ProcData->LoadProfile = IsArgPresent(ArgsEnd,argv,"-profile");
	global->ProcData->LockIt = IsArgPresent(ArgsEnd,argv,"-lock");
	NoNetworkScan = IsArgPresent(ArgsEnd,argv,"-noscan");
	
	
	if(!User) {
		User = (char*)calloc(50,sizeof(char));
		if (!nouser){
			std::cout<<"Login: "<<std::flush;
			std::cin>>User;
		}
	}
	
	if(!Domain) {
		Domain = (char*)calloc(50,sizeof(char));
		if (!nouser){
			std::cout<<"Domain: ";
			std::cin>>Domain;
		}
	}
	
	if(!Password) {
		DWORD fdwOldMode,fdwMode;
		HANDLE hStdin=GetStdHandle(STD_INPUT_HANDLE);
		
		Password  = (char*)calloc(128,sizeof(char));
		if (!nouser){
			GetConsoleMode(hStdin, &fdwOldMode);      
			fdwMode = fdwOldMode & ~ENABLE_ECHO_INPUT; 
			SetConsoleMode(hStdin, fdwMode); 
			std::cout<<"Password: "<<std::flush;
			std::cin>>Password;
			std::cout<<std::endl;   
			SetConsoleMode(hStdin, fdwOldMode); 
		}
	}
	
	if(IsArgPresent(ArgsEnd,argv,"-store") && !nouser) {
		char name[255];
		sprintf(name,"%s/%s",Domain,User);
		StoreAccount(name,User,Domain,Password);
		NOTE("Stored account "<<name<<" in registry");
		storeflag = TRUE;
	}
	global->Account = (TAccount*)malloc(sizeof(TAccount));
	global->Account->Global = FALSE;
	strcpy(global->Account->User,User);
	strcpy(global->Account->Domain,Domain);
	strcpy(global->Account->Password,Password);
	
	//extract priority for remote processes
	global->ProcData->PriorityClass=NORMAL_PRIORITY_CLASS;
	GetStringArg(ArgsEnd,argv,"-priority",&Priority);
	if (Priority) {
		strupr(Priority);
		if (!(strcmp(Priority,"REALTIME")))
			global->ProcData->PriorityClass=REALTIME_PRIORITY_CLASS;
		if (!(strcmp(Priority,"IDLE")))
			global->ProcData->PriorityClass=IDLE_PRIORITY_CLASS;
		if (!(strcmp(Priority,"HIGH")))
			global->ProcData->PriorityClass=HIGH_PRIORITY_CLASS;
	}
	
}
					  
DWORD Update(HostData *Config,char *filename,char **Hosts,DWORD numHosts) 
{
	DWORD res,i;
	char error[256];
	RemoteStartupInfo SI;
	HANDLE hRemoteProc;
	
	memset(&SI,0,sizeof(SI));
	
	SI.Commandline = MAGIC_UPDATE_STRING;
	for(i = 0;i<numHosts;++i) {
		
		
		std::cout<<"\nUpdating "<<Hosts[i]<<std::endl<<std::flush;
		strcpy(Config->Name,Hosts[i]);
		DBM("Sending new binary to server");
		res = SendServerBinary(Config,filename);
		if(res != ERROR_SUCCESS) {
			std::cerr<<"SendServer Binary failed:\n"<<GetLastErrorText(res,error,256)<<std::endl;
			CloseHost(Config);
			continue;
		}
		MessageWin = new CAsyncWindow(0);
		SI.Window = MessageWin->Handle;
		
		MessageWin->AddClient();
		res = CreateRemoteProcess(Config,&SI,&hRemoteProc);
		if(res != ERROR_SUCCESS) {
			std::cerr<<std::endl<<"Could not start update process on "<<Hosts[i]<<":\n"<<
				GetLastErrorText(res,error,256)<<std::endl;
			MessageWin->RemoveClient();
		} else {
			MessageWin->MessageLoop();
			CloseRemoteHandle(hRemoteProc);
		}
		CloseHost(Config);
		delete MessageWin;
	}
	
	
	return 0;
}

int main(int argc,char** argv) {
	
	BOOL SeperatorPresent=FALSE;
	bool Loaded;
	RemoteStartupInfo SI;
	bool checkuseraccount = true; //SI 7.7.04
	char *Command,
		*MachineFile,*Plugin=0,
		*Params,error[256],
		*ConfFile=0,*path=0,
		**Nodes=0,*BootComputer=0,*PluginParams=0;
	
	int Size=0,ParamSize = 0,NumProcs,NumHosts=0;
	DWORD result,tid;
	
	int ArgsEnd,CommandStart,i;
	int test;
	HANDLE inFd;
	HostData **Servers=0,GlobalSettings;
	PlgDesc actPlugin;
	
	if(argc == 1) return Usage();    
	memset(&GlobalSettings,0,sizeof(GlobalSettings));
	
	
	inFd = GetStdHandle(STD_INPUT_HANDLE);
	
	ArgsEnd=argc;
	CommandStart = 1;
	
	for(i=1;i<argc;++i) {
		if(argv[i][0]== '-' && argv[i][1]=='-') {
			ArgsEnd=i;
			CommandStart = i+1;
			SeperatorPresent=TRUE;
			break;
		}
	}
	
	verbose = IsArgPresent(&ArgsEnd,argv,"-loud");
	debug_flag = IsArgPresent(&ArgsEnd,argv,"-debug");
	test = IsArgPresent(&ArgsEnd,argv,"-test");
	nouser = IsArgPresent(&ArgsEnd,argv,"-nouser");
	checkuseraccount = !(IsArgPresent(&ArgsEnd,argv,"-noaccountcheck"));
	if (checkuseraccount)
		checkuseraccount = !(IsArgPresent(&ArgsEnd,argv,"-nac"));
	if(nouser)
		checkuseraccount=false;
	
	GetStringArg(&ArgsEnd,argv,"-reboot",&BootComputer);
	reboot_flag = (BootComputer != 0);
	if (! reboot_flag)
	{
		GetStringArg(&ArgsEnd,argv,"-shutdown",&BootComputer);
		shutdown_flag =(BootComputer != 0);
		
	}
	
	if (shutdown_flag || reboot_flag)
		NoNetworkScan = true; 
	
	if(debug_flag) verbose = 1;
	
	if(IsArgPresent(&ArgsEnd,argv,"-help") || IsArgPresent(&ArgsEnd,argv,"-?"))
		return Usage();
	
	if(IsArgPresent(&ArgsEnd,argv,"-version")) {
		
		std::cout<<"mpiexec version:  "<<VERSION<<"."<<SUBVERSION<<std::endl;
		return 0;
	}
	else
	{
		//always print version in debug modus
#ifdef _DEBUG
		std::cerr<<"mpiexec version:  "<<VERSION<<"."<<SUBVERSION<<std::endl;
#endif	
	}
	
	if(IsArgPresent(&ArgsEnd,argv,"-accounts")) {
		long flags;
		flags = std::cout.flags();
		flags |= std::ios::left;
		std::cout.flags(flags);
		std::cout<<SPACE<<"Name"<<"Login\n";
		EnumStoredAccounts(AccountCallback,0);
		return 0;
	}
	
	if(IsArgPresent(&ArgsEnd,argv,"-plugins")) {
		ListPlugins();
		return 0;
	}
	
	InitializeRexecClient();
	
	
	if(GetStringListArg(&ArgsEnd,argv,"-update",&Nodes,&NumHosts)) {
		ParseCommandline(&ArgsEnd,argv,&Servers,&NumProcs,&GlobalSettings,&Plugin);
		
		return Update(&GlobalSettings,argv[1],Nodes,NumHosts);
	}
	GetStringArg(&ArgsEnd,argv,"-configfile",&ConfFile);
	
	
	if(ConfFile) {
		NOTE("Loading RexecShell configuration "<<ConfFile);
		LoadConfig(ConfFile,&Servers,(int*)&NumProcs,&GlobalSettings,&Plugin);
		Loaded = true; 
	} else {
		MachineFile = "machines.txt";
		Loaded = false;
		GetStringArg(&ArgsEnd,argv,"-machinefile",&MachineFile);
		GetStringListArg(&ArgsEnd,argv,"-host",&Nodes,&NumHosts);
		ParseCommandline(&ArgsEnd,argv,&Servers,&NumProcs,&GlobalSettings,&Plugin);
	}
	
	//now username, domain and password are known
	//check if account is valid
	if (checkuseraccount){
	
	  if (!IsValidUser(GlobalSettings.Account->User,GlobalSettings.Account->Domain,GlobalSettings.Account->Password))
	  {
		DWORD errcode = GetLastError();
		if (errcode == 1326)
		{
			std::cerr<<std::endl<<std::endl<<"WARNING!"<<std::endl<< 
				"The given account (domain/username/password) is not accepted by the local host."<< std::endl
				<<"Please check if it's valid for the remote machines."<<std::endl<<std::endl;
		}
	  }
	}
	
	
	
	
	
	//try to shut down / reboot remote computer
	if ((reboot_flag || shutdown_flag))
	{
		DWORD errcode;
		int NumServer =0;
		strcpy(actPlugin.DllName,"");//SI//23-08-2004 default chars as name effects service crash
		
		if (strcmp("-hostlist",BootComputer) != 0)
		{
			//specified one computer for reboot/shutdown
			
			NumHosts = 1;
			
			Nodes = (char **) calloc(1, sizeof(char*));
			Nodes[0] = (char *) calloc(255, sizeof(char));
			strcpy(Nodes[0],BootComputer); //copy necessary GetNodesFromList frees Nodes
			
		}
		NumProcs = NumHosts; //choose one computer once
		Servers = (HostData**)calloc(sizeof(HostData*),NumProcs);
		errcode = GetNodesFromList(Nodes,NumProcs,Servers,(int*)&NumHosts,&GlobalSettings,&actPlugin,false);//create Server[0]
		
		if (NumHosts == 0)
			std::cerr << "No valid host found"<<std::endl;
		for (NumServer=0;NumServer<NumHosts;NumServer++)
		{
			if (Servers[NumServer] != 0)
			{
				errcode=ShutDown(Servers[NumServer],reboot_flag);
				if (errcode != ERROR_SUCCESS)
					std::cerr << errcode<<": Could not shut down remote computer "<<Servers[NumServer]->Name <<std::endl;
			}
			else
			{
				std::cerr << "connection to "<<Servers[NumServer]->Name<<" failed";
				errcode = 1;
			}
		}
		return(errcode);
	}
	

	//test if no plugin shall be used -> manual commandline generation
	if(strcmp(Plugin,"none")==0)
	{
		actPlugin.Attach=NULL;
		actPlugin.Detach=NULL;
		actPlugin.NewData=NULL;
		actPlugin.SelectedChange=NULL;
		actPlugin.DlgClose=NULL;
		actPlugin.RefreshEnd=NULL;
		actPlugin.Convert=NULL;		
		actPlugin.Configure=NULL;
		actPlugin.ParseCommandline=NULL;
		actPlugin.PluginVersionNumber=0;
		strcpy(actPlugin.VisualName,"");
		strcpy(actPlugin.DllName,"");
	
	}
	else
	  if(!LoadPlugin(Plugin,&actPlugin)) {
		return 1;
	  }

	/* Read additional parameters for plugin */
	GetStringArg(&ArgsEnd,argv,"-pluginparams",&PluginParams);
	if (!PluginParams)
		GetStringArg(&ArgsEnd,argv,"-pp",&PluginParams);
	if (PluginParams)
	{	
		ProcSetString(&GlobalSettings.ProcData->PluginOptions,PluginParams,&GlobalSettings.ProcData->PluginOptSize);
		DBM("Set plugin params: "<<PluginParams);				
	}
	
	if(!Loaded) {
		//do not use configfile
		GetStringArg(&ArgsEnd,argv,"-path",&path);
		if(IsArgPresent(&ArgsEnd,argv,"-gc")) {
			DBM("Calling PluginConfigure");
			if(PluginConfigure(&actPlugin,&GlobalSettings)) 
				return 1;
		} else {
			DBM("Calling PluginParse");
			if(PluginParse(&actPlugin,&ArgsEnd,argv,&GlobalSettings)) 
				return 1;
		}
		if((!SeperatorPresent && ArgsEnd < 2)||
			(SeperatorPresent && CommandStart >= argc)) {
			if (! storeflag)  //suppress message on storing account
				std::cerr<<"Command to start is missing\n";
			return 1;
		} 
		
		if((SeperatorPresent && ArgsEnd > 1)) {
			std::cerr<<ArgsEnd-1<<" Invalid Commandline parameter(s)\n";
			for(i=1;i<ArgsEnd;++i) std::cerr<<argv[i]<<" ";
			std::cerr<<std::endl;
			return 1;
		}
		
		if(SeperatorPresent) ArgsEnd = argc;
		
		Size = 0;
		for(i=CommandStart+1;i<ArgsEnd;++i) 
		{
			Size += strlen(argv[i])+1;
			Size += 2; //Si if quotes necessary
		}
		
		if(Size) {
			Params = (char*)malloc((Size+1)*sizeof(char));
			*Params = 0;
			for(i=CommandStart+1;i<ArgsEnd;++i) {
				//Si check if argument contains spaces
				char * pspace = NULL;
				pspace=strchr(argv[i],' ');
				if (pspace == NULL)
				{
					strcat(Params,argv[i]);
				}
				else
				{
					//Si add quotes
					strcat(Params,"\"");
					strcat(Params,argv[i]);
					strcat(Params,"\"");
				}
				strcat(Params," ");
			}
			ProcSetString(&GlobalSettings.ProcData->UserOptions,Params,&GlobalSettings.ProcData->OptSize);
			DBM("Set user params: "<<Params);
			free(Params);
		} 
		if(path) {
			Command = (char*)malloc(strlen(path)+strlen(argv[CommandStart])+2);
			sprintf(Command,"%s\\%s",path,argv[CommandStart]);
			ProcSetString(&GlobalSettings.ProcData->Executable,
				Command,&GlobalSettings.ProcData->ExeSize);
			DBM("Set command: "<<Command);
			free(Command);
		} else {
			ProcSetString(&GlobalSettings.ProcData->Executable,
				argv[CommandStart],&GlobalSettings.ProcData->ExeSize);
			DBM("Set command: "<<argv[CommandStart]);
		}
		
		MakeUNCPath(GlobalSettings.ProcData->Executable,&Command);
		ProcSetString(&GlobalSettings.ProcData->Executable,
			Command,&GlobalSettings.ProcData->ExeSize);
		DBM("Converted command to "<<Command);
		free(Command);
		
		
		Servers = (HostData**)malloc(sizeof(HostData*)*NumProcs);
		
		if(NumHosts)
		{
			//DoPluginCheck if actPlugin.ParseCommandline!=NULL
			GetNodesFromList(Nodes,NumProcs,Servers,(int*)&NumHosts,&GlobalSettings,&actPlugin,(bool)actPlugin.ParseCommandline);
			if (NumHosts < NumProcs)
				std::cerr <<"Not enough valid hosts in parameter list.\n";
		}
		if(NumHosts != NumProcs) {
			ReadHosts(MachineFile,NumProcs,Servers,(int*)&Size,&GlobalSettings,&actPlugin);
			if(Size != NumProcs) {
				std::cerr<<"\""<<MachineFile<<"\" does not contain any valid hosts.\n";
				if (NoNetworkScan)
				{
					std::cerr<<"Scanning of network suppressed.\n"; 
				}   
				else
				{
					//scanning network if no valid hosts were found
					std::cerr<<"Scanning network...\n";
					ScanNetwork(NumProcs,Servers,(int*)&Size,&GlobalSettings,&actPlugin);
				}
				if(Size != NumProcs) {
					std::cerr<<"Could not find any valid hosts\nExiting...\n";
					return 1;
				}
			}
		}
	} else { //if(!Loaded)
		//use configfile
		for(i=0;i<NumProcs;++i) {
			if(!Servers[i]->ProcData)
				CopyProcData(Servers[i],&GlobalSettings);
			if(!Servers[i]->Account) 
				SetHostAccount(Servers[i],GlobalSettings.Account);
			Servers[i]->Security = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
			NOTE("Checking host "<<Servers[i]->Name);
			if(GetHostStatus(Servers[i],actPlugin.DllName) != ERROR_SUCCESS) {
				std::cerr<<"Could not get state of host "<<Servers[i]->Name<<std::endl;
				std::cerr<<GetLastErrorText(Servers[i]->LastError,error,256)<<std::endl;
				return 1;
			}
			
		}
	}
	
	qsort(Servers,NumProcs,sizeof(HostData*),compare);     
	if(PluginCreateCommand(&actPlugin,Servers,(DWORD*)&NumProcs,&GlobalSettings))
		return 1;
	
	MessageWin = new CAsyncWindow(0);
	SI.Window = MessageWin->Handle;
	RemoteProcs = (HANDLE*)malloc(sizeof(HANDLE)*NumProcs);
	memset(RemoteProcs,0,sizeof(HANDLE)*NumProcs);
	
	InitializeCriticalSection(&CS);
	
	SetConsoleCtrlHandler(CtrlHandlerFunction,TRUE);
	for(i=0;i<NumProcs && !StopIt;++i) {
		DBM("Configuring "<<Servers[i]->Name);
		SI.Commandline = (char*)malloc(
			Servers[i]->ProcData->ExeSize+
			Servers[i]->ProcData->WDSize+
			Servers[i]->ProcData->CmdSize+
			Servers[i]->ProcData->OptSize+
			Servers[i]->ProcData->PluginOptSize+4);
		
		strcpy(SI.Commandline,Servers[i]->ProcData->Executable);

		if(Servers[i]->ProcData->PluginOptions) {
    	    strcat(SI.Commandline," ");
            strcat(SI.Commandline,Servers[i]->ProcData->PluginOptions);
		}
		
		if(Servers[i]->ProcData->Commandline) {
			strcat(SI.Commandline," ");
			strcat(SI.Commandline,Servers[i]->ProcData->Commandline);
		}
		
		if(Servers[i]->ProcData->UserOptions) {
			strcat(SI.Commandline," ");
			strcat(SI.Commandline,Servers[i]->ProcData->UserOptions);
		}
		
		SI.Environment = Servers[i]->ProcData->Environment;
		SI.EnvSize     = Servers[i]->ProcData->EnvSize;
		SI.WorkingDir  = Servers[i]->ProcData->WorkingDir;
		
		
		if(!nouser && (Servers[i]->ProcData->LoadProfile ==1))
			SI.CreationFlags = CREATE_WITH_USERPROFILE;
		else
			SI.CreationFlags = 0;
		
		//enhancement with priority class
		
		SI.CreationFlags = SI.CreationFlags +(Servers[i]->ProcData->PriorityClass);
		
		if(!SI.WorkingDir) SI.WorkingDir  = "";
		
		DBM("Working dir is: "<<SI.WorkingDir);
		if(verbose || test)
			std::cerr<<"["<<Servers[i]->Name<<"]:  "<<SI.Commandline<<std::endl;
		
		EnterCriticalSection(&CS);
		if(StopIt) {
			LeaveCriticalSection(&CS);
			break;
		}
		MessageWin->AddClient();
		
		if(!test) {
			if(Servers[i]->ProcData->LockIt) {
				result=LockServer(Servers[i]);
				if(result != ERROR_SUCCESS) {
					std::cerr<<"\nCould not lock server "<<Servers[i]->Name<<":\n"<<
						GetLastErrorText(result,error,256)<<std::endl;
					Servers[i]->ProcData->LockIt = FALSE;
				}
			}
			NOTE("Starting process on "<<Servers[i]->Name);

			if(nouser)
				result=CreateRemoteProcessNoUser(Servers[i],&SI,RemoteProcs+i);
			else
				result=CreateRemoteProcess(Servers[i],&SI,RemoteProcs+i);
			
			if(result != ERROR_SUCCESS) {
				std::cerr<<std::endl<<SI.Commandline<<"\nCould not start process on "<<Servers[i]->Name<<":\n"<<
					GetLastErrorText(result,error,256)<<std::endl;
				MessageWin->RemoveClient();
				if(Servers[i]->ProcData->LockIt ) {
					if(UnlockServer(Servers[i]) != ERROR_SUCCESS) {
						std::cerr<<"\nCould not unlock server "<<Servers[i]->Name<<":\n"<<
							GetLastErrorText(result,error,256)<<std::endl;
					}
				}
			} 
			++NumStarted ;
		} 
		LeaveCriticalSection(&CS);
		free(SI.Commandline);
	}
	
	SetConsoleTitle(GlobalSettings.ProcData->Executable);
	if(!test) {
		CloseHandle(CreateThread(0,0,ReadInput,inFd,0,&tid));
		MessageWin->MessageLoop();
	}
	
	SetConsoleCtrlHandler(CtrlHandlerFunction,FALSE);
	DeleteCriticalSection(&CS);
	NOTE("Processes finished\n");
	for(i=0;i<NumProcs;++i) {
		if(Servers[i]->ProcData->LockIt ) {
			if(UnlockServer(Servers[i]) != ERROR_SUCCESS) {
				std::cerr<<"\nCould not unlock server "<<Servers[i]->Name<<":\n"<<
					GetLastErrorText(result,error,256)<<std::endl;
			}
		}
		CloseHost(Servers[i]);
		CloseRemoteHandle(RemoteProcs[i]);
	}
	
	delete MessageWin;
	
	ShutdownRexecClient();
	DetachPlugin(&actPlugin);
	return 0;
} 
					  
					  
