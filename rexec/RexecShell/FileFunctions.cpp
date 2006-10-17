//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "RexecClient.h"
#include "FileFunctions.h"
#include "Encryption.h"
#include <malloc.h>
//---------------------------------------------------------------------------
DWORD ReadString(HANDLE f,char **buf) {
	DWORD size,Read;

    if(!buf) return 0;
    if(!ReadFile(f,&size,sizeof(DWORD),&Read,0) || !size)
     return 0;

    if(!*buf) {
    	if(size) *buf = (char*)malloc(size);
        else *buf = 0;
	    if(!*buf) return 0;
    }
    ReadFile(f,*buf,size,&Read,0);
    (*buf)[Read]=0;
    return Read;

}

DWORD ReadConfigData(HANDLE f,HostData *Host, DWORD fileversion) {
   DWORD size,Read,ContextSize;
   TProcessData *P;
   if(!Host) return 0;
   if(!Host->ProcData) {
   		Host->ProcData = (TProcessData*)malloc(sizeof(TProcessData));
        memset(Host->ProcData,0,sizeof(TProcessData));
   }
   P = Host->ProcData;

   Read = ReadString(f,&P->Executable);
   P->ExeSize = Read;
   size = Read;
   Read = ReadString(f,&P->WorkingDir);
   P->WDSize = Read;
   size += Read;
   Read = ReadString(f,&P->UserOptions);
   P->OptSize = Read;
   size += Read;
   P->CmdSize = 0;

   ReadFile(f,&P->EnvSize,sizeof(DWORD),&Read,0);
   if(Read && P->EnvSize) {
        if(P->EnvAllocSize < P->EnvSize)
	    	P->Environment=(char*)realloc(P->Environment,P->EnvSize);
	   	P->EnvAllocSize = P->EnvSize;
        ReadFile(f,P->Environment,P->EnvSize,&Read,0);
        size += Read;
   }
   ReadFile(f,&P->LockIt,sizeof(P->LockIt),&Read,0);
   size += Read;
   ReadFile(f,&P->LoadProfile,sizeof(P->LoadProfile),&Read,0);
   size += Read;


   ReadFile(f,&ContextSize,sizeof(ContextSize),&Read,0);
   if(Read && ContextSize) {
   		if(ContextSize != P->ContextSize) {
        	P->Context = realloc(P->Context,ContextSize);
            P->ContextSize = ContextSize;
        }
        ReadFile(f,P->Context,ContextSize,&Read,0);
        size += Read;
   }
   if (fileversion > 1)
   {  //added at FILE_VERSION 2
      ReadFile(f,&P->PriorityClass,sizeof(DWORD),&Read,0);
      size += Read;
      Read = ReadString(f,&P->PluginOptions);
      if (Read){
         size += Read;
         P->PluginOptSize = Read;
      }
   }
   return size;
}

DWORD ReadAccount(HANDLE f,HostData *Host) {
	DWORD size;
    char *N;
    if(!Host) return 0;
    if(!Host->Account) {
    	Host->Account = (TAccount *)malloc(sizeof(TAccount));
        Host->Account->User[0]=0;
        Host->Account->Domain[0]=0;
    }
    Host->Account->Password[0]=0;
    N = Host->Account->User;
    size = ReadString(f,&N);
    N = Host->Account->Domain;
    size += ReadString(f,&N);
    return size;
}

DWORD ReadAccountEncrypted(HANDLE f,HostData *Host) {
	DWORD size,DataSize,Read;
    char *buffer;
    if(!Host) return 0;
    if(!ReadFile(f,&DataSize,sizeof(DWORD),&Read,0) || !Read)
    	return 0;
    size = Read;
	buffer = (char*)alloca(DataSize);
    if(!buffer) return size;
    if(!ReadFile(f,buffer,DataSize,&Read,0) || !Read)
	    return size;
    size += Read;
    if(DecryptData(buffer,&DataSize))
        SetHostAccount(Host,(TAccount*)(buffer+DataSize));
    else SetHostAccount(Host,0);
    return size;
}

DWORD ReadHost(HANDLE f,HostData **Host) {
	char *N;
	if(!Host) return 0;

    if(!*Host) {
    	*Host =(HostData*)malloc(sizeof(HostData));
        memset(*Host,0,sizeof(HostData));
    }
    N = (*Host)->Name;
    return ReadString(f,&N);
}

DWORD ReadType(HANDLE f) {
	DWORD type = TYPE_NONE,Read;
    if(!ReadFile(f,&type,sizeof(DWORD),&Read,0) || !Read)
    	return TYPE_NONE;

    return type;
}

DWORD WriteString(HANDLE f,char *buf) {
	DWORD size,Written;
    if(!buf) size = 0;
	else size = strlen(buf)+1;
    if(!WriteFile(f,&size,sizeof(DWORD),&Written,0))
     	return 0;
    if(size) WriteFile(f,buf,size,&size,0);
    return size;
}

DWORD WriteConfigData(HANDLE f,HostData *Host) {
   TProcessData *P;
   DWORD Written,size,type;
   if(!Host) return 0;
   P = Host->ProcData;
   if(!P) return 0;

   type = TYPE_CONFIG;
   if(!WriteFile(f,&type,sizeof(DWORD),&Written,0))
   	return 0;

   size =  WriteString(f,P->Executable);
   size += WriteString(f,P->WorkingDir);
   size += WriteString(f,P->UserOptions);

   if(P->EnvSize>1) {
   		WriteFile(f,&P->EnvSize,sizeof(DWORD),&Written,0);
   		if(Written)
	        WriteFile(f,P->Environment,P->EnvSize,&Written,0);
        size += Written+sizeof(DWORD);
   } else {
   		type=0;
        WriteFile(f,&type,sizeof(DWORD),&Written,0);
        size += sizeof(DWORD);
   }
   WriteFile(f,&P->LockIt,sizeof(P->LockIt),&Written,0);
   size += Written;
   WriteFile(f,&P->LoadProfile,sizeof(P->LockIt),&Written,0);
   size += Written;
   
   WriteFile(f,&P->ContextSize,sizeof(P->ContextSize),&Written,0);
   size += Written;
   if(Written && P->ContextSize)
   		WriteFile(f,P->Context,P->ContextSize,&Written,0);
   size += Written;
   //added at file version 2
   WriteFile(f,&P->PriorityClass,sizeof(DWORD),&Written,0);
   size += sizeof(DWORD);

   size += WriteString(f,P->PluginOptions);

   return size;
}

DWORD WriteAccount(HANDLE f,HostData *Host) {
	DWORD type,size,Written;

	if(!Host || !Host->Account) return 0;
	type = TYPE_ACCOUNT;
   	if(!WriteFile(f,&type,sizeof(DWORD),&Written,0))
   		return 0;
   	size  = Written;
    size += WriteString(f,Host->Account->User);
    size += WriteString(f,Host->Account->Domain);
    return size;
}

DWORD WriteAccountEncrypted(HANDLE f,HostData *Host) {
	DWORD type,size,Written,bufsize=sizeof(TAccount);
    char *resbuf=0;

	if(!Host || !Host->Account) return 0;
    //Si 14.06.05: check return value of EncryptData, just write if it succeeds
    if (!EncryptData((char*)Host->Account,&resbuf,&bufsize))
    	return 0;
    type = TYPE_ACCOUNT_ENCRYPTED;
    if(!WriteFile(f,&type,sizeof(DWORD),&Written,0))
   		return 0;
   	size  = Written;
	if(!WriteFile(f,&bufsize,sizeof(DWORD),&Written,0)) {
   	  	if(resbuf) free(resbuf);
    	return size;
    }
    size += Written;
    if(!WriteFile(f,resbuf,bufsize,&Written,0))
    	return size;
    size += Written;
  	if(resbuf) free(resbuf);
    return size;
}

DWORD WriteHost(HANDLE f,HostData *Host) {
	DWORD type,size,Written;

	if(!Host) return 0;
    type = TYPE_HOST;
   	if(!WriteFile(f,&type,sizeof(DWORD),&Written,0))
   		return 0;
	size = Written;
    size += WriteString(f,Host->Name);
    if(Host->ProcData)
    	size += WriteConfigData(f,Host);
    if(Host->Account)
    	size += WriteAccountEncrypted(f,Host);

    return size;
}

DWORD WritePlugin(HANDLE f,PlgDesc *p) {
	DWORD type,size,Written;

	if(!p) return 0;
    type = TYPE_PLUGIN;
   	if(!WriteFile(f,&type,sizeof(DWORD),&Written,0))
   		return 0;
	size = Written;
	size += WriteString(f,p->VisualName);
    
    return size;
}