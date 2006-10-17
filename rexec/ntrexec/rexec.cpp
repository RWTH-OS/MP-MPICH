#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "Debug.h"
#include "asyncwindow.h"
#include "RexecClient.h"
#include "getopt.h"


HANDLE hRemoteProc;
BOOL Run=TRUE,KILLED=FALSE;
int debug_flag = 0;
using namespace std;

extern "C" {

DWORD WINAPI CallbackFunction(char *txt,void *) {
	cerr<<txt<<endl;
	return 0;
}

BOOL WINAPI AccountCallback(char *Account, char *User, char *Domain, char *Password, void *dummy) {
	cout<<Account<<": \tLogin: "<<Domain<<'/'<<User<<" \tPassword: *Don't tell anybody*\n";
	return TRUE;
}

}


DWORD WINAPI ReadInput(LPVOID lpParameter) {
    char buf[BUFSIZE];
    DWORD Read=1;
    // The thread reads from the input handle given as parameter
    // and writes the data to the first process created.
    HANDLE fd = (HANDLE)lpParameter;
    while(ReadFile(fd,buf,BUFSIZE,&Read,0) && Read) {
	RexecSendInput(hRemoteProc,buf,Read);
    }
    return 0;
}

BOOL WINAPI CtrlHandlerFunction(DWORD dwCtrlType ) {
	cerr<<"\nCTRL-C pressed. Waiting for client process to terminate...\n";
	//Run=0;
	KILLED=TRUE;
	KillRemoteProcess(hRemoteProc);
	return TRUE;
}
		
int usage(void) {
	cerr<<"Usage:\nntrexec [-a <account>] | -s | -c <command> | -l ] -h <host> [-- <params>]\n";
	return 1;
}

int UsageOld(void) {
	cerr<<"Usage:\nntrexec <host> <command>\n";
	return 1;
}


void ReadCreds(char *User, char *Domain,char *Password) {
	DWORD fdwOldMode,fdwMode;
	HANDLE hStdin=GetStdHandle(STD_INPUT_HANDLE);
	cout<<"Login: "<<flush;
	cin>>User;

	cout<<"Domain: ";
	cin>>Domain;
	
	GetConsoleMode(hStdin, &fdwOldMode); 		
	fdwMode = fdwOldMode & ~ENABLE_ECHO_INPUT; 
	SetConsoleMode(hStdin, fdwMode); 
	cout<<"Password: "<<flush;
	cin>>Password;
	cout<<endl;	
	SetConsoleMode(hStdin, fdwOldMode); 
}



extern "C" {
extern char *optarg;
extern int optind, opterr, optopt;
}

int main(int argc,char** argv) {

	BOOL DataPresent=FALSE;
	RemoteStartupInfo SI;
	char Dir[MAX_PATH], Command[MAX_PATH], Domain[MAX_PATH],
		 Password[MAX_PATH], User[MAX_PATH],c,Account[128];
	
	DWORD Size=MAX_PATH,res,tid;
	CAsyncWindow *MessageWin;
	HostData host_data;

	memset(&host_data,0,sizeof(host_data));
	//strcpy(Command,"cmd");
	Command[0]=0;
	//Host[0]=0;
	Dir[0] = 0;
	opterr=0;
	    WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
	    cerr<<"WSAStartup failed\n";
	    return WSAGetLastError();
	    
	}
	while((c=getopt(argc,argv,"a:sc:h:ld:"))!=EOF) {
		switch(c) {
		case 'a':if(DataPresent||!optarg||!optarg[0]) return usage(); 
				 
				if(!LoadAccount(optarg,User,Domain,Password)) {
					cerr<<GetLastError()<<": Cannot load account "<<optarg<<endl;
					return 1;
				}
				DataPresent=TRUE;
				break;
		case 's':if(DataPresent) return usage(); 
				
				ReadCreds(User,Domain,Password);
				cout<<"Symbolic name: ";
				cin>>Account;
				if(!StoreAccount(Account,User,Domain,Password)) {
					cerr<<GetLastError()<<": Cannot store account "<<Account<<endl;
					return 1;
				} else {
					cout<<"Account "<<Account<<" stored in registry\n";
				}
				return 0;
				
		case 'c':if(sscanf(optarg,"%s",Command)!=1) return usage(); break;
		case 'h':if(sscanf(optarg,"%s",host_data.Name)!=1) return usage(); break;
		case 'l': cout<<"Stored accounts:\n"; EnumStoredAccounts(AccountCallback,0); return 0;
		case 'd': if(!optarg) return usage();
		           strcpy(Dir,optarg); break;
		case '?': return usage();
		}
	}


	if(!host_data.Name[0]) {
		cout<<"Host to execute on: ";
		cin>>host_data.Name;
	}   

	if(!Dir[0])
	    GetCurrentDirectory(255,Dir);
	char *lauf=Command+strlen(Command);
	if(lauf !=Command) {
		*lauf=32;
		*(lauf+1)=0;
	} else lauf--;
	int len;
	for(int i=optind;i<argc;i++) {
		len=strlen(argv[i]);
		lauf++;
		strncpy(lauf,argv[i],len);
		lauf+=len;
		*lauf=32;
	}
	*lauf=0;
	
	InitializeRexecClient();
	if(!DataPresent) ReadCreds(User,Domain,Password);

	char Title[512];
	sprintf(Title,"%s: %s",host_data.Name,Command);
	SetConsoleTitle(Title);

	if(host_data.Name[0]=='.') GetComputerName(host_data.Name,&Size);
	if(!stricmp(host_data.Name,Domain)) strcpy(Domain,".");

	
	SI.Environment = 0;
	SI.CreationFlags = 0;
	SI.Commandline=Command;
	SI.WorkingDir=Dir;

	host_data.Account = (TAccount*)malloc(sizeof(TAccount));
	host_data.Account->Global = FALSE;
	strcpy(host_data.Account->User,User);
	strcpy(host_data.Account->Domain,Domain);
	strcpy(host_data.Account->Password,Password);
	
	MessageWin = new CAsyncWindow(0);
	SI.Window = MessageWin->Handle;
	MessageWin->AddClient();
	
	
	res=CreateRemoteProcess(&host_data,&SI,&hRemoteProc);
	if(res != RPC_S_OK) {
		LPVOID lpMsgBuf;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
		if(lpMsgBuf) {	
		    cerr<<"Could not create remote Process.\n"<<(char*)lpMsgBuf;
		    LocalFree(lpMsgBuf);
		} else cerr<<"Could not create remote Process.\n";
		return 1;
	}

	SetConsoleCtrlHandler(CtrlHandlerFunction,TRUE);
	
	
	CloseHandle(CreateThread(0,0,ReadInput,GetStdHandle(STD_INPUT_HANDLE),0,&tid));
	MessageWin->MessageLoop();
	/*
	while((WaitForMultipleObjects(2,handles,FALSE,INFINITE)-WAIT_OBJECT_0==1) && res>0) {
		ReadFile(handles[1],buffer,1023,&Read,0);
		res=RexecSendInput(hRemoteProc,buffer,Read);
	}
	*/
	//while(WaitForRemoteEnd(hRemoteProc,INFINITE)==WAIT_FAILED) /*do it again*/;
	delete MessageWin;
	Sleep(100);
	CloseRemoteHandle(hRemoteProc);
	cout<<"Finished"<<endl<<flush;
	WSACleanup();
	//ExitProcess(0);		  
	return 0;
} 

