//---------------------------------------------------------------------------
#define UNICODE
//---------------------------------------------------------------------------

#include <vcl\vcl.h>
#include <windows.h>
#define NET_API_STATUS   DWORD
#define NET_API_FUNCTION __stdcall
#include <Lmserver.h>
#include <Lmerr.h>
#include <lmapibuf.h>
#include <process.h>

#pragma hdrstop
 
#include "NetState.h"
#include "Exclude.h"
#include "RexecClient.h"
#include "PluginManager.h"
#include "Include.h"

#define MACHINES_TXT "Machines.txt"
//---------------------------------------------------------------------------

CServers *Servers;
AnsiString MachinesFile; //si

static int __fastcall ThreadMain(void *S) {
	((CServers*)S)->ThreadFunc();
	EndThread(0);
//	_endthread();
    return 0;
}

static int __fastcall ThreadWhile(void *S) {
	((CServers*)S)->TestFunc(1000);
	EndThread(0);

//	_endthread();
    return 0;
}
void CServers::TestFunc(int n) {
	for(int i=0;i<n;i++)
    {
    Sleep(10);
    }
}

/*void ThreadMain(void *S) {
	((CServers*)S)->ThreadFunc();
	EndThread(0);
//	_endthread();
//    return 0;
}  */

CServers::CServers(BOOL Backup=FALSE) {
	InitializeCriticalSection(&CS);
    InitializeCriticalSection(&clientCS);
    InitializeRexecClient();
	Hosts = (HostData**)malloc(20*sizeof(HostData*));
    NumHosts=0;
    Arraysize=20;
    // increase numClients by calling RegisterClientWindow
    numClients=0;
    hThread=0;
    B=Backup;
    Finish = FALSE;
    Refreshing = false;
    useMachinestxt = false;
    AskedForuseMachinestxt = false;
}

CServers::~CServers() {
	StopProcessing();
	if(!B) Clear();
    DeleteCriticalSection(&CS);
    DeleteCriticalSection(&clientCS);
    delete Hosts;
    ShutdownRexecClient();
}

void CServers::Clear() {
	EnterCriticalSection(&CS);
    for(int i=0;!B&&i<NumHosts;i++) {
        FreeHost(Hosts[i]);
        Hosts[i]=0;
    }
    NumHosts=0;
    LeaveCriticalSection(&CS);
}

BOOL CServers::RegisterClientWindow(HWND client) {
	EnterCriticalSection(&clientCS);
    if(numClients>=20) {
        LeaveCriticalSection(&clientCS);
    	return FALSE;
    }
    // if client window is already registered return true
    for(int i=0;i<numClients&&!Finish;i++)
    	if(Clients[i]==client) {
            LeaveCriticalSection(&clientCS);
        	return TRUE;
        }
    // else add handle to list, increase number of clients
    Clients[numClients++]=client;
    LeaveCriticalSection(&clientCS);
    return TRUE;
}

BOOL CServers::RemoveClientWindow(HWND client) {
	if(Finish) return TRUE;
    EnterCriticalSection(&clientCS);
	for(DWORD i=numClients-1;i>=0 && !Finish;i--)
    	if(Clients[i]==client) {
         memmove(Clients+i,Clients+i+1,(numClients-i-1)*sizeof(HWND));
         numClients--;
         break;
        }
    LeaveCriticalSection(&clientCS);
    return TRUE;
}

int CServers::IndexOf(char *Name) {
	int i;
    int u,l;
    int res;
    l=0;

    EnterCriticalSection(&CS);
    if(!NumHosts || Finish) {
        LeaveCriticalSection(&CS);
    	return -1;
    }
    u=NumHosts-1;
    while(u>=l) {
    	i=l+((u-l+1)>>1);
        res=stricmp(Hosts[i]->Name,Name);
        if(res<0) l=i+1;
        else if(res>0) u=i-1;
		else break;
    }
    LeaveCriticalSection(&CS);
    return (!res)?i:-1;
}


int CServers::Add(HostData* Host) {
    int i;
    EnterCriticalSection(&CS);
    if(Finish) {
        LeaveCriticalSection(&CS);
    	return 0;
    }
	if(Arraysize<NumHosts+1) {
    	Hosts=(HostData**)realloc(Hosts,2*Arraysize*sizeof(HostData*));
        Arraysize*=2;
    }
    i=0;
    while((i<NumHosts)&&(strcmp(Host->Name,Hosts[i]->Name)>0))
        i++;

    if(i<NumHosts)
       memmove(Hosts+i+1,Hosts+i,sizeof(HostData*)*(NumHosts-i));
    NumHosts++;
    Hosts[i]=Host;
    LeaveCriticalSection(&CS);
    return(NumHosts-1);
}

void CServers::Delete(int index) {
	if(Finish||index<0||index>=NumHosts) return;
    EnterCriticalSection(&CS);
    if(!B) {
    	FreeHost(Hosts[index]);
    }
    CopyMemory(Hosts+index,Hosts+index+1,(NumHosts-index-1)*sizeof(char*));
    NumHosts--;
    LeaveCriticalSection(&CS);
}

void CServers::Delete(char *Host) {
	EnterCriticalSection(&CS);
    Delete(IndexOf(Host));
    LeaveCriticalSection(&CS);
}



BOOL CServers::EnumData(HWND client) {
	BOOL res=TRUE;
    int i;
    EnterCriticalSection(&CS);
    if(Finish) {
    	LeaveCriticalSection(&CS);
        return TRUE;
    }
	SendMessage(client,ENUM_START,0,(LONG)(NumHosts));
    i=0;
    while(i<NumHosts&&!Finish) {
    	if(ExcludeForm->List->IndexOf(Hosts[i]->Name)<0) {
          //Send Message to Windows to add host information
             SendMessage(client,PAR_DATA,(WPARAM)i,(LONG)(Hosts[i]));
             i++;
        } else
            Delete(i);
    }
	SendMessage(client,ENUM_FINISH,0,0);
    if(Refreshing) res=FALSE;
    LeaveCriticalSection(&CS);
	return res;
}

void CServers::StopProcessing() {
    Finish=TRUE;
    MSG Message;
	while(hThread) {
           if(PeekMessage(&Message,0,ENUM_START,REFRESH_START,PM_REMOVE))
           	DispatchMessage(&Message);
        else Sleep(1);                      
    }
}

void CServers::ThreadFunc() {
  DWORD EntriesRead = 0,totalentries = 0;  /*initialize variables*/
  SERVER_INFO_101 *Sv=0;
  HostData *ActHost;
  AnsiString h;
  int i,j;
  DWORD k;
  //thread to refresh
  TStringList *MachinesList = new TStringList;

     if (! useMachinestxt)  //si
     {                                                   
       NetServerEnum(0,101,(unsigned char**)&Sv,-1,
        	&EntriesRead,&totalentries,SV_TYPE_NT,0,0);
	  /*
        NetServerEnum:
        Header: Declared in lmserver.h.
        Import Library: Use netapi32.lib.
      */
    }

     EnterCriticalSection(&clientCS);
     //disable some buttons on client windows
     for(i=0;i<numClients&&!Finish;i++)
     	PostMessage(Clients[i],REFRESH_START,0,(LONG)EntriesRead);
     LeaveCriticalSection(&clientCS);

     
     if (! useMachinestxt)  //si
     {
        for(k=0;k<EntriesRead&&!Finish;k++) {
          h=AnsiString(Sv[k].sv101_name).UpperCase();
          // do not refresh servers in exclude-list
          if(ExcludeForm->List->IndexOf(h)>=0) {
         	Delete(h.c_str());
         	continue;
          }//if
     	ActHost=Refresh(h.c_str()); //hier wird das Fenster blockiert 
        if(!ActHost) continue;
       if (Application->Terminated) //si
        {
        	Finish = true;
            continue;
        }//if

        EnterCriticalSection(&clientCS);
        for(j=0;j<numClients&&!Finish;j++)
        	PostMessage(Clients[j],PAR_DATA,(WPARAM)k,(LONG)ActHost);
            //Sleep(1000);
        LeaveCriticalSection(&clientCS);
      } // for(k=0;k<EntriesRead&&!Finish;k++)


   	  if(Sv) NetApiBufferFree(Sv);
      // refresh servers in include-list
      for(i=0;i<IncludeForm->List->Count&&!Finish;i++) {
    	ActHost=Refresh(IncludeForm->List->Strings[i].c_str());
        if(!ActHost)
        {
            Finish = false;
            continue;
        }
        if (Application->Terminated)    //si
        {
            Finish = true;
            continue;
        }
        EnterCriticalSection(&clientCS);
        for(j=0;j<numClients;j++)
        	PostMessage(Clients[j],PAR_DATA,0,(LONG)ActHost);
        LeaveCriticalSection(&clientCS);
      }

    }  //if (! useMachinestxt)  //si
    else
    {
      MachinesList->LoadFromFile(MachinesFile);
      for(i=0;i<MachinesList->Count&&!Finish;i++) {
    	ActHost=Refresh(MachinesList->Strings[i].c_str());
        if(!ActHost) continue;
        if (Application->Terminated)
        {
        	Finish = true;
            continue;
        }
        EnterCriticalSection(&clientCS);
        for(j=0;j<numClients;j++)
        	PostMessage(Clients[j],PAR_DATA,0,(LONG)ActHost);
        LeaveCriticalSection(&clientCS);
      }
    
  }

    delete MachinesList;
    EnterCriticalSection(&clientCS);
    for(i=numClients-1;i>=0&&!Finish;i--) {
    	PostMessage(Clients[i],REFRESH_FINISH,0,0);
        //RemoveClientWindow(Clients[i]);
    }
    LeaveCriticalSection(&clientCS);
    //CloseHandle(hThread);
    Refreshing=false;
    hThread=0;

}

BOOL CServers::Refresh() {
    //int id;
    unsigned long id;
    int i;
	EnterCriticalSection(&CS);
    if(Refreshing || Finish) {
    	LeaveCriticalSection(&CS);
        return FALSE;
    } else Refreshing=true;
    int button;
    AnsiString MachinesPath; //si

    EnterCriticalSection(&clientCS);
    //  Send message REFRESH_START to all client windows
    // to disable some window elements in client windows
    for(i=0;i<numClients;i++)
    	SendMessage(Clients[i],REFRESH_START,0,0);
    LeaveCriticalSection(&clientCS);
    if (!AskedForuseMachinestxt)//si
    {
    	AskedForuseMachinestxt = true;
        MachinesFile = getenv("USERPROFILE");
        MachinesFile =  MachinesFile + "\\" + MACHINES_TXT;
        if (FileExists(MachinesFile))
        {
    		button = Application->MessageBox(
            "User-file Machines.txt found. Do you want to use only hosts specified in this file?",
              "choose hosts",MB_YESNO);
        	useMachinestxt = ( IDYES == button);
        }
        else
        {
                MachinesPath = Application->ExeName;
                MachinesPath =ExtractFilePath(MachinesPath);
                MachinesFile =  MachinesPath + MACHINES_TXT;
        	if (FileExists(MachinesFile))
        	{
    			button = Application->MessageBox(
            	"File Machines.txt found. Do you want to use only hosts specified in this file?",
              	  "choose hosts",MB_YESNO);
        		useMachinestxt = ( IDYES == button);
        	}
        }
    }
    //hThread=(HANDLE)_beginthreadNT(ThreadMain,0,this,0,0,&id);
    //hThread=(HANDLE)_beginthreadNT(ThreadMain,0,this,0,0,&id);
    //hThread=(HANDLE)_beginthread(ThreadMain,0,this,0,0,&id);
    hThread=(HANDLE)BeginThread(0,0,ThreadMain,this,0,id); //-> separate thread for CServers::ThreadFunc()

    //hThread=(HANDLE)BeginThread(0,0,ThreadWhile,this,0,id); //-> separate thread for CServers::ThreadFunc()
    CloseHandle(hThread);
    LeaveCriticalSection(&CS);
	return (hThread==INVALID_HANDLE_VALUE)?FALSE:TRUE;
}

HostData *CServers::Refresh(char *Host) {
	HostData *ActHost;
    PlgDesc *plg;
    int index;
	EnterCriticalSection(&CS);
    if(Finish) {
    	LeaveCriticalSection(&CS);
        return 0;
    }
	index=IndexOf(Host);
    if(index<0) {
    	 ActHost=(HostData*)malloc(sizeof(HostData));
         memset(ActHost,0,sizeof(HostData));
         strcpy(ActHost->Name,Host);
         Add(ActHost);
    } else {
    	ActHost=Hosts[index];
    }
    plg = PluginManager.GetActualPlugin();
    //for (int i =0;i<5;i++)
    Sleep(1);
    if(plg)
	    GetHostStatus(ActHost,plg->DllName); //Fenster wird blockiert
    else
    	GetHostStatus(ActHost,"");
    LeaveCriticalSection(&CS);
    return ActHost;
}


void CServers::RefreshOne(char * name) {
  DWORD EntriesRead,totalentries;
  SERVER_INFO_101 *Sv=0;
  HostData *ActHost;
  AnsiString h;
  int i,j;
  DWORD k;
  //thread to refresh
  


     /*EnterCriticalSection(&clientCS);
     //disable some buttons on client windows
     for(i=0;i<numClients&&!Finish;i++)
     	PostMessage(Clients[i],REFRESH_START,0,(LONG)EntriesRead);
     LeaveCriticalSection(&clientCS);    */



    	ActHost=Refresh(name);

        EnterCriticalSection(&clientCS);
        for(j=0;j<numClients;j++)
        	PostMessage(Clients[j],PAR_DATA,0,(LONG)ActHost);
        LeaveCriticalSection(&clientCS);


    EnterCriticalSection(&clientCS);
    for(i=numClients-1;i>=0&&!Finish;i--) {
    	PostMessage(Clients[i],REFRESH_FINISH,0,0);
        //RemoveClientWindow(Clients[i]);
     }
    LeaveCriticalSection(&clientCS);
    //CloseHandle(hThread);
    Refreshing=false;
    hThread=0;

}


