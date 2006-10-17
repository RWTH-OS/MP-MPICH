
#include <windows.h>
#include <iostream>
#include <fstream>
#include "RexecClient.h"
#include "Environment.h"
#include "plugin.h"
#include "mpirun.h"
#include "FileFunctions.h"
#include <vector>
#ifndef NET_API_STATUS
#define NET_API_STATUS   DWORD
#define NET_API_FUNCTION __stdcall
#endif
#include <Lmserver.h>
#include <lmapibuf.h>

using namespace std;
typedef std::vector<char*> CharList;

extern DWORD GetExePath(char *path,DWORD size);

char *CopyString(const char *str) {
    char *res;

    res = (char*)malloc(sizeof(str)*(strlen(str)+1));
    strcpy(res,str);
    return res;
}

int PluginCheckValid(PlgDesc *plg,HostData **Servers,DWORD *NumProcs,HostData *data) {
    DWORD Num;
    if(!plg->SelectedChange) return 1;
    Num= *NumProcs+1;
    __try {
        plg->SelectedChange(Servers,*NumProcs+1,Servers,&Num,data);
    } __except(EXCEPTION_EXECUTE_HANDLER ) {
        cerr<<"Plugin caused exception while checking validity\n";
        return 0;
    }

    if(Num != *NumProcs+1) return 0;
    else return 1;
}


BOOL CreateHost(const char *name,HostData **Hosts,int *np,HostData *global,PlgDesc *actPlugin,bool DoPluginCheck) {
    char error[256], * charptr;
    HostData *newHost;
    int i;

    newHost = (HostData *)malloc(sizeof(HostData));
    memset(newHost,0,sizeof(HostData));
    strcpy(newHost->Name,name);
    CopyProcData(newHost,global);
    SetHostAccount(newHost,global->Account);
    newHost->Security = global->Security;
    //set priority for servers
    newHost->ProcData->PriorityClass = global->ProcData->PriorityClass;

    NOTE("Checking host "<<name<<"...");
    for(i=0;i<*np;++i) {
        if(!stricmp(name,Hosts[i]->Name)) {
            NOTE("Host is known");
            CopyStateData(newHost,Hosts[i]);
            break;
        }
    }

    if(i == *np) {
        // If we get here, the host is new...
        NOTE("Host is new. Querying state...");

        //set domain if specified in hostname with backslash
        if (charptr = strchr(newHost->Name,'\\'))
        {
            //domain is specified in hostname
            *charptr = '\0'; //separate domain from host        
            charptr++; //pointer to hostname
            strcpy(newHost->Account->Domain,newHost->Name);
            strcpy(newHost->Name,charptr);
        }

		DWORD errcode = GetHostStatus(newHost,actPlugin->DllName); 
        if( errcode != ERROR_SUCCESS) {
            NOTE(GetLastErrorText(newHost->LastError,error,256));
            FreeHost(newHost);
            return FALSE;
        }
    }
    Hosts[*np] = newHost;
	if(DoPluginCheck)
	{
      if(!PluginCheckValid(actPlugin,Hosts,(DWORD*)np,global)) {
        NOTE("Plugin invalidated host");
        FreeHost(newHost);
        return FALSE;
	  }
	}

    Hosts[*np] = newHost;
    (*np)++;
    ++newHost->State->NumProcs;
    return TRUE;

}

HostData *CreateCopy(HostData *Host,HostData *global) {
    HostData *newHost;

    newHost = (HostData *)malloc(sizeof(HostData));
    memset(newHost,0,sizeof(HostData));
    strcpy(newHost->Name,Host->Name);
    CopyProcData(newHost,global);
    SetHostAccount(newHost,global->Account);
    newHost->Security = global->Security;
    CopyStateData(newHost,Host);
    ++newHost->State->NumProcs;
    return newHost;

}

void BlowUp(int np,HostData **Hosts,int listLen,HostData *global) {
    int i,j,k,ProcsPerProcessor=1;
    int add;

    add = np-listLen;
    // Put processes on hosts that have CPUs
    // available. If no more CPUs are free,
    // we start overloading CPUs.
    k = listLen;
    j=0;
    while(add>0) {
        if(j==add) ++ProcsPerProcessor;
        j=add;
        for(i=0;(i<listLen) && (add>0);++i) {
            if(Hosts[i]->State->NumProcs/Hosts[i]->State->Configuration->HW.dwNumberOfProcessors
                < (unsigned int) ProcsPerProcessor) {
                    Hosts[k++] = CreateCopy(Hosts[i],global);
                    --add;
                }
        }   
    }
    if(verbose && ProcsPerProcessor>1)
        NOTE("WARNING: Insufficient number of processors available.");
}

DWORD ReadHosts(char *file,int np,HostData **Hosts, int *listLen,HostData *global,PlgDesc *actPlugin) {
    ifstream s;
    char hostname[256],*ptr,*ptr2;
    DWORD errors=0,passes=0;
    int i;
    char Dir[8],Path[MAX_PATH];

    *listLen = 0;
    if(verbose)
        GetCurrentDirectory(256,hostname);
    NOTE("Searching "<<file<<" in "<<hostname);
//	s.open(file,ios::nocreate|ios::in);  ios::nocreate does not exist anymore ....
	s.open(file,ios::in); 
    if(!s) {
        s.clear();
        if(GetEnvironmentVariable("HOMEDRIVE",Dir,8) &&
            GetEnvironmentVariable("HOMEPATH",Path,MAX_PATH)) {
                sprintf(hostname,"%s%s\\%s",Dir,Path,file);
                NOTE("Not found. Searching for "<<hostname);
//                s.open(hostname,ios::nocreate|ios::in);
                s.open(hostname,ios::in);
            }
            if(!s) {
                s.clear();
                if(GetExePath(Path,MAX_PATH)) {
                    sprintf(hostname,"%s%s",Path,file);
                    NOTE("Not found. Searching for "<<hostname);
//                    s.open(hostname,ios::nocreate|ios::in);
                    s.open(hostname,ios::in);
                }
            }
    }
    if(!s) {
        cerr<<"Could not open hostfile "<<file<<endl;
        return 0;
    }
    hostname[255]=0;

    i =0;
    while(i < np && !s.eof()) {
        hostname[0]=0;
        s.getline(hostname,255);
        ptr = hostname;
        // Remove leading whitespaces
        while(*ptr == ' ' || *ptr == '\t' || *ptr == 10 || *ptr == 13) 
            ++ptr;
        ptr2=ptr; 
        // Remove trailing whitespaces
        while(*ptr2 != 0) {
            if(*ptr2 == ' ' || *ptr2 == '\t' || *ptr2 == 10 || *ptr2 == 13) {
                *ptr2=0;
                break;
            }
            ++ptr2;
        }
        //s>>hostname;
        if(!ptr[0] || ptr[0] =='#') {
            continue;
        } 
        if(!CreateHost(ptr,Hosts,&i,global,actPlugin))
            NOTE("WARNING: Could not query host "<<ptr<<" or it is invalid "<<endl);
    }

    if(i != np && i>0) {
        BlowUp(np,Hosts,i,global);
        i = np;
    }
    *listLen = i;
    return *listLen;

}


BOOL Contains(const CharList *l,const char* name) {
    CharList::const_iterator i;
    for(i=l->begin();i!=l->end();++i) {
        if(!strcmp(*i,name)) {
            return TRUE;
        }
    }
    return FALSE;
}

DWORD ScanNetwork(int np, HostData **Hosts, int *listLen,HostData *global, PlgDesc *actPlugin) {
    DWORD EntriesRead,totalentries;
    int i=0,j=0;
    SERVER_INFO_101 *Sv=0;
    char mName[255];
    ifstream s;
    CharList Exclude;
    char Path[MAX_PATH];

    GetExePath(Path,MAX_PATH);
    strcat(Path,"Excluded.rsh");
    NOTE("\nOpening "<<Path);
//    s.open(Path,ios::in|ios::nocreate);
    s.open(Path,ios::in);
    if(!s) {
        NOTE("Could not open "<<Path);
    } else {
        while(!s.eof()) {
            mName[0]=0;
            s>>mName;
            if(mName[0]) {
                Exclude.push_back(CopyString(mName));
            }
        }
        s.close();
    }
    NetServerEnum(0,101,(unsigned char**)&Sv,-1,
        &EntriesRead,&totalentries,SV_TYPE_NT,0,0);

    while(i < np && (DWORD)j<EntriesRead) {
        WideCharToMultiByte( CP_ACP, 0, (LPWSTR)Sv[j++].sv101_name, -1,mName, 255, 
            NULL, NULL );
        if(Contains(&Exclude,mName)) continue;
        CreateHost(mName,Hosts,&i,global,actPlugin);
        if(!verbose) cerr<<".";
    }
    if(!verbose) cerr<<endl;

    if(Sv) NetApiBufferFree(Sv);
    if(i != np && i>0) {
        BlowUp(np,Hosts,i,global);
        i = np;
    }
    *listLen = i;
    for(i=0; (DWORD)i < Exclude.size(); ++i) 
		free(Exclude[i]);
    return *listLen;
}

DWORD GetNodesFromList(char **Nodes,int np,HostData **Servers,int* Size,HostData *global,PlgDesc *actPlugin,bool DoPluginCheck) {
    int i,j;
    i=j=0;
    while(i < np && j<*Size) {
        CreateHost(Nodes[j],Servers,&i,global,actPlugin,DoPluginCheck);
        free(Nodes[j++]);
    }
    free(Nodes);
    if(i != np && i>0) {
        BlowUp(np,Servers,i,global);
        i = np;
    }
    *Size = i;
    return i;
}

BOOL LoadConfig(const char *File,HostData ***Hosts,int *np,HostData *global,char **Plugin) {
    HANDLE file;
    DWORD type;
    HostData *actData;
    char PlgName[30],*N = PlgName;
    char error[256];
    int alloced=0;
    bool ignoreContext=false;
    *np = 0;


    file = CreateFileA(File,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,0);
    if(file == INVALID_HANDLE_VALUE) {
        cerr<<"Cannot open file!\n"<<GetLastErrorText(GetLastError(),error,256)<<endl;
        return FALSE;
    }
    type = ReadType(file);
    if(type != MAGIC) {
        CloseHandle(file);
        cerr<<"Invalid file format!\n";
        return FALSE;
    }
    type = ReadType(file);
    if(type != FILE_VERSION) {
        CloseHandle(file);
        cerr<<"Wrong file version!\n";
        return FALSE;
    }


    actData = global;
    FreeProcData(actData);
    SetHostAccount(actData,0);

    do {
        type = ReadType(file);
        switch(type) {
        case TYPE_CONFIG: ReadConfigData(file,actData);
            if(ignoreContext) FreeContext(actData);
            break;
        case TYPE_ACCOUNT: ReadAccount(file,actData); break;
        case TYPE_ACCOUNT_ENCRYPTED: ReadAccountEncrypted(file,actData); break;
        case TYPE_HOST: 
            actData = 0;
            ReadHost(file,&actData);
            if(actData) {

                if(alloced <= *np) {
                    alloced +=4;
                    *Hosts = (HostData**)realloc(*Hosts,alloced *sizeof(HostData*));
                }
                (*Hosts)[(*np)++] = actData;
            }
            break;
        case TYPE_PLUGIN: ReadString(file,Plugin);
            break;

        case TYPE_VERSION:
        case TYPE_GLOBAL: actData = global;
        case TYPE_NONE:
        default: break;
        }
    } while(type != TYPE_NONE);
    CloseHandle(file);
    return TRUE;
}
