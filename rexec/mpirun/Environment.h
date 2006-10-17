//---------------------------------------------------------------------------
#ifndef EnvironmentH
#define EnvironmentH
//---------------------------------------------------------------------------
#include "Plugin.h"

extern "C" {

__declspec(dllexport)
void FreeContext(HostData *Host);

__declspec(dllexport)
void *GetContext(HostData *Host,DWORD size);

__declspec(dllexport)
void PutEnvString(HostData *Proc,char *Name,char *value);

__declspec(dllexport)
char* GetEnvString(HostData *Proc,char *Name,char *value,DWORD *BufSize);

__declspec(dllexport)
void SetCommandline(HostData *Proc,char *commandline);

void FreeProcData(HostData *Proc);
void CopyProcData(HostData *dest,HostData *src);
void EmptyEnvironment(HostData *Proc);
void FreeEnvironment(HostData * Proc);
void ProcSetString(char **str,const char *val,DWORD *size);
void ProcStrRemove(char **str,DWORD *size);
}

#endif
