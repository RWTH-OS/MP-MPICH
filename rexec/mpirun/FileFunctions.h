//---------------------------------------------------------------------------
#ifndef FileFunctionsH
#define FileFunctionsH
#include "plugin.h"
//---------------------------------------------------------------------------

#define TYPE_NONE    0
#define TYPE_GLOBAL  1
#define TYPE_CONFIG  2
#define TYPE_ACCOUNT 3
#define TYPE_HOST    4
#define TYPE_PLUGIN  5
#define TYPE_VERSION 6
#define TYPE_ACCOUNT_ENCRYPTED 7
#define MAGIC        0xbadbabe
#define FILE_VERSION 1

DWORD ReadString(HANDLE f,char **buf);
DWORD ReadConfigData(HANDLE f,HostData *Host);
DWORD ReadAccount(HANDLE f,HostData *Host);
DWORD ReadAccountEncrypted(HANDLE f,HostData *Host);
DWORD ReadHost(HANDLE f,HostData **Host);
DWORD ReadType(HANDLE f);

DWORD WriteString(HANDLE f,char *buf);
DWORD WriteConfigData(HANDLE f,HostData *Host);
DWORD WriteAccount(HANDLE f,HostData *Host);
DWORD WriteAccountEncrypted(HANDLE f,HostData *Host);
DWORD WriteHost(HANDLE f,HostData *Host);
DWORD WritePlugin(HANDLE f,PlgDesc *p);



#endif
