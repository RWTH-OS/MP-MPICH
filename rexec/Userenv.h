#ifndef __USERENV_H__
#define __USERENV_H__

#include <windows.h>
#include <winreg.h>
#include <map>

extern CRITICAL_SECTION loginCS;

/*
class UserKey {
public:
	UserKey() {Domain[0]=Name[0]=0;}
	UserKey(char *domain, char *name) {strcpy(Domain,domain);strcpy(Name,name); }

	char Domain[MAX_USER_NAME_LEN],
		 Name[MAX_USER_NAME_LEN];
	
	operator <(const UserKey &other) const {
		int res;
		res=strcmp(Name,other.Name);
		if(!res) res=strcmp(Domain,other.Domain);
		return(res<0);
	}
};
*/

class UserInfo {
public:
	UserInfo(); 
	UserInfo(const UserInfo &other);
	~UserInfo() { if(Environment) free(Environment);}
	void LoadSystemEnvironment();
	void LoadUserEnvironment(HANDLE User);
	void MergeEnvironment(char* Env);
	void PutEnvString(char *Name,char *value);
	void PutEnvString(wchar_t *Name,wchar_t *value);
	char *GetEnvString(char *Name,char *value,DWORD *BufSize);
	char *GetEnv() {return Environment;}
	void DumpEnv();
	DWORD DriveMask;
	
protected:
	void LoadEnvFromKey(HKEY key);
	DWORD EnvSize;
	DWORD UsedSize;
	char *Environment;
};


//typedef std::map<UserKey,UserInfo*> CUserMap;

//extern CUserMap UserMap;

//BOOL LoadUserHive(HANDLE User,NtrexecMessage *msg, HANDLE param,TCHAR *szSid);

#endif
