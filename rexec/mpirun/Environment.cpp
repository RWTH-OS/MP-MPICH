//---------------------------------------------------------------------------
#include <wtypes.h>
#include <winbase.h>
#include <stdio.h>
#include "Environment.h"
#include "Plugin.h"
//---------------------------------------------------------------------------

extern "C" {
    
    __declspec(dllexport)
    char* GetEnvString(HostData *Proc,char *Name,char *value,DWORD *BufSize) {
	char *act,
	    *pos;
	int res;
	DWORD lenAll,lenStName,lenGName,lenValue;
	
	if(!Proc || !Proc->ProcData) return 0;
	
	act=Proc->ProcData->Environment;
	if(value) *value=0;
	if(!Name) {
	    return 0;
	}
	while(act&&*act) {
	    lenAll=strlen(act);
	    pos=strchr(act+1,'='); // act+1 to avoid errors on =X variables
	    if(! pos) {
		act+=(lenAll+1);
		continue;
	    }
	    lenStName=(DWORD)(pos-act);
	    lenGName=strlen(Name);
	    res=strnicmp(act,Name,min(lenStName,lenGName));
	    
	    if(!res) {
		if(lenStName > lenGName) res = 1;
		else if(lenStName < lenGName) res = -1;
		else {
		    lenValue = strlen(pos+1)+1;
		    if(value && lenValue<=*BufSize) strcpy(value,pos+1);
		    *BufSize = lenValue;
		    return act;
		}
	    }
	    if(res>0) {
		*BufSize = 0;
		return act;
	    }
	    act+=(lenAll+1);
	}
	*BufSize = 0;
	return act;
    }
    
    __declspec(dllexport)
    void PutEnvString(HostData *Proc,char *Name,char *value) {
	char *OldPos;
	DWORD oldLen,newLen,OldIndex,oldSize,namelen;
	if(!value) value="";
	namelen = strlen(Name);
	newLen=namelen+strlen(value)+2;
	
	oldLen = 0;
	if(!Proc->ProcData) {
	    Proc->ProcData = (TProcessData*)malloc(sizeof(TProcessData));
	    memset(Proc->ProcData,0,sizeof(TProcessData));
	}
	if(!Proc->ProcData->Environment) {
	    Proc->ProcData->Environment = (char*)malloc(128);
	    Proc->ProcData->EnvAllocSize=128;
	    *Proc->ProcData->Environment=0;
	    Proc->ProcData->EnvSize=1;
	}
	OldPos=GetEnvString(Proc,Name,0,&oldLen);
	if(!OldPos) {
	    return;
	}
	if(oldLen)
	    oldLen += namelen+1;
	OldIndex=(DWORD)(OldPos-Proc->ProcData->Environment);
	if(oldLen>=newLen) {
	    memmove(OldPos+newLen,OldPos+oldLen,Proc->ProcData->EnvSize-(OldIndex+oldLen));
	    Proc->ProcData->EnvSize-=(oldLen-newLen);
	} else {
	    oldSize=Proc->ProcData->EnvSize;
	    Proc->ProcData->EnvSize+=(newLen-oldLen);
	    if(Proc->ProcData->EnvAllocSize<Proc->ProcData->EnvSize) {
		Proc->ProcData->EnvAllocSize=
		    max(Proc->ProcData->EnvSize,2*Proc->ProcData->EnvAllocSize);
		Proc->ProcData->Environment=
		    (char*)realloc(Proc->ProcData->Environment,Proc->ProcData->EnvAllocSize);
		OldPos=Proc->ProcData->Environment+OldIndex;
	    }
	    memmove(OldPos+newLen,OldPos+oldLen,oldSize-OldIndex-oldLen);
	}
	sprintf(OldPos,"%s=%s",Name,value);
    }
    
    __declspec(dllexport)
    void *GetContext(HostData *Host,DWORD size) {
	if(!Host) return 0;
	if(!Host->ProcData) {
	    if(!size) return 0;
	    Host->ProcData = (TProcessData*)malloc(sizeof(TProcessData));
	    memset(Host->ProcData,0,sizeof(TProcessData));
	}
	if(Host->ProcData->ContextSize<size) {
	    Host->ProcData->Context = realloc(Host->ProcData->Context,size);
	    Host->ProcData->ContextSize = size;
	}
	return Host->ProcData->Context;
    }
    
    void EmptyEnvironment(HostData *Proc) {
	if(Proc && Proc->ProcData && Proc->ProcData->Environment) {
	    *Proc->ProcData->Environment = 0;
	    Proc->ProcData->EnvSize=1;
	}
    }
    
    void FreeEnvironment(HostData * Proc) {
	
	if(!Proc || !Proc->ProcData) return;
	if(Proc->ProcData->Environment) {
	    free(Proc->ProcData->Environment);
	}
	Proc->ProcData->Environment = 0;
	Proc->ProcData->EnvSize=0;
	Proc->ProcData->EnvAllocSize=0;
    }
    
    void ProcSetString(char **str,const char *val,DWORD *size) {
	DWORD len;
	
	if(!val) {
	    ProcStrRemove(str,size);
	    return;
	}
	len = strlen(val)+1;
	if(*size<len) {
	    *str = (char*)realloc(*str,len);
	    *size = len;
	}
	strcpy(*str,val);
    }
    
    void ProcStrRemove(char **str,DWORD *size) {
	if(*str) {
	    free(*str);
	    *str = 0;
	}
	*size = 0;
    }
    
    __declspec(dllexport)
    void FreeContext(HostData *Host) {
	if(!Host || !Host->ProcData||!Host->ProcData->Context)
	    return;
	free(Host->ProcData->Context);
	Host->ProcData->Context = 0;
	Host->ProcData->ContextSize = 0;
    }
    
    void FreeProcData(HostData *Proc) {
	if(!Proc || !Proc->ProcData) return;
	FreeContext(Proc);
	FreeEnvironment(Proc);
	ProcStrRemove(&Proc->ProcData->Commandline,&Proc->ProcData->CmdSize);
	ProcStrRemove(&Proc->ProcData->Executable,&Proc->ProcData->ExeSize);
	ProcStrRemove(&Proc->ProcData->WorkingDir,&Proc->ProcData->WDSize);
	ProcStrRemove(&Proc->ProcData->UserOptions,&Proc->ProcData->OptSize);
	ProcStrRemove(&Proc->ProcData->PluginOptions,&Proc->ProcData->PluginOptSize);
	free(Proc->ProcData);
	Proc->ProcData = 0;
    }
    
    void CopyProcData(HostData *dest,HostData *src) {
	TProcessData *d,*s;
	
	if(!src || !src->ProcData) {
	    if(dest && dest->ProcData) FreeProcData(dest);
	    return;
	}
	if(!dest->ProcData) {
	    dest->ProcData = (TProcessData*)malloc(sizeof(TProcessData));
	    memset(dest->ProcData,0,sizeof(TProcessData));
	}
	d = dest->ProcData;
	s = src->ProcData;
	d->LockIt = s->LockIt;
	d->LoadProfile = s->LoadProfile;
	ProcSetString(&d->Executable,s->Executable,&d->ExeSize);
	ProcSetString(&d->WorkingDir,s->WorkingDir,&d->WDSize);
	ProcSetString(&d->UserOptions,s->UserOptions,&d->OptSize);
	ProcSetString(&d->PluginOptions,s->PluginOptions,&d->PluginOptSize);
	if(s->Environment) {
	    if(d->EnvAllocSize<s->EnvSize) {
		d->Environment=(char*)realloc(d->Environment,s->EnvSize);
		d->EnvAllocSize = s->EnvSize;
	    }
	    memcpy(d->Environment,s->Environment,s->EnvSize);
	    d->EnvSize=s->EnvSize;
	} else if(d->Environment) FreeEnvironment(src);
	
	if(s->Context) {
	    if(d->ContextSize!=s->ContextSize) {
		d->Context = realloc(d->Context,s->ContextSize);
		d->ContextSize = s->ContextSize;
	    }
	    memcpy(d->Context,s->Context,s->ContextSize);
	} else if(d->Context) FreeContext(dest);
    }
    
    __declspec(dllexport)
    void SetCommandline(HostData *Proc,char *commandline) {
	if(!Proc) return;
	if(!Proc->ProcData) {
	    Proc->ProcData = (TProcessData*)malloc(sizeof(TProcessData));
	    memset(Proc->ProcData,0,sizeof(TProcessData));
	}
	ProcSetString(&Proc->ProcData->Commandline,commandline,&Proc->ProcData->CmdSize);
    }
    
}