#include <windows.h>
#include <winnls.h>
#include <lm.h>
#include <stdio.h>

#include "Environment.h"
#include "userenv.h"
#include "service.h"
#include "messages.h"
#include "helpers.h"

HINSTANCE hUserenv;
LoginFunc LoadUserProfile;
LogoutFunc UnloadUserProfile;
CreateEnvFunc CreateEnvironmentBlock;
DestroyEnvFunc DestroyEnvironmentBlock;

CRITICAL_SECTION loginCS;

// Return values : 1 No Update, 2 Path is provided (If zero length use stdPath)
DWORD GetPolicyUpdatePath(char *path) {
    HKEY hKey;
    DWORD lErrorCode;
    DWORD dwSize=MAX_PATH;
    DWORD dwKeyType;
    DWORD dwUpdateType;
    
    path[0]=0;
    DBG("Getting PolicyUpdatePath")
    lErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Update", 0, KEY_READ, &hKey);
    if (lErrorCode != ERROR_SUCCESS) {
	DBG("Could not open registry ("<<lErrorCode<<")")
	    return 1;
    }
    
    dwSize=sizeof(DWORD);
    lErrorCode = RegQueryValueEx(hKey, TEXT("UpdateMode"), NULL,
	&dwKeyType, (LPBYTE)&dwUpdateType, &dwSize);
    
    if (lErrorCode != ERROR_SUCCESS) {
	DBG("Could not query UpdateMode")
	RegCloseKey(hKey);
	return 1;
    }

    DBG("Update mode is: "<<dwUpdateType)
    // If update is off or interactive update is required we disable policy loading
    if(!dwUpdateType||dwUpdateType==2) {
	RegCloseKey(hKey);
	return 1;
    }
    
    dwSize=MAX_PATH;
    lErrorCode = RegQueryValueEx(hKey, TEXT("NetworkPath"), NULL,
	&dwKeyType, (LPBYTE)path, &dwSize);
    DBG("Update path: "<<path)
    RegCloseKey(hKey);
    return 2;
}


DWORD SetProfileDlgTimeout(DWORD newValue) {
	HKEY hKey;
	DWORD lErrorCode;
	DWORD dwSize=sizeof(DWORD);
	DWORD dwKeyType;
	DWORD dwOldValue;

	lErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 0, KEY_READ|KEY_WRITE, &hKey);
	if (lErrorCode != ERROR_SUCCESS) {
		SetLastError(lErrorCode);
		return (DWORD)-1;
	}
	
	lErrorCode = RegQueryValueEx(hKey, TEXT("ProfileDlgTimeOut"), NULL,
		&dwKeyType, (LPBYTE)&dwOldValue, &dwSize);
	
	if (lErrorCode != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		SetLastError(lErrorCode);
		return (DWORD)-1;
	}

	lErrorCode=RegSetValueEx(hKey,TEXT("ProfileDlgTimeOut"), NULL,
		dwKeyType, (LPBYTE)&newValue, dwSize);

	return dwOldValue;
}




HANDLE MyLoadUserProfile(HANDLE User,char *UserName,char *Domainname, UserInfo &UI) {
	PROFILEINFO ProfInfo;
	//HANDLE hProfile;
	LPBYTE ComputerName=0;
	char ServerName[256],CentralProfile[MAX_PATH],DefaultPath[MAX_PATH],
		 PolicyPath[MAX_PATH],Homedir[MAX_PATH],Homedrive[5],*pos;
	WCHAR wszUserName[256],wszDomainName[256];
	DWORD size=256;
	_USER_INFO_3 *ui;
	BOOL ret,local=FALSE;

	ZeroMemory(&ProfInfo,sizeof(ProfInfo));
	ProfInfo.dwSize = sizeof(ProfInfo);
	ProfInfo.dwFlags = PI_NOUI;

	DBG("Loading users's profile")
	// Create WideChar domainName
	MultiByteToWideChar( CP_ACP, 0, Domainname,strlen(Domainname)+1, 
				wszDomainName,sizeof(wszDomainName)/sizeof(wszDomainName[0]) );

	// Create WChar UserName
	MultiByteToWideChar( CP_ACP, 0, UserName,strlen(UserName)+1, 
			wszUserName,sizeof(wszUserName)/sizeof(wszUserName[0]) );
	
	ServerName[0]=0;
	if(Domainname[0] != '.') {
		NetGetDCName( NULL, wszDomainName, &ComputerName );
		WideCharToMultiByte( CP_ACP, 0, (unsigned short *)ComputerName, -1,
				     ServerName, 256, NULL, NULL );
	} else	{
		local=TRUE;
		GetComputerName(Domainname,&size);
		sprintf(ServerName,"\\\\%s",Domainname);
		ComputerName=0;
	}

	DBG("Account server name is: "<<ServerName)

	if( NetUserGetInfo( (LPWSTR)ComputerName,(LPWSTR) &wszUserName, 3, (LPBYTE *) &ui ) ){
		AddToMessageLog(IDM_USERINFO,GetLastError(),UserName);    
		DBG("NetUserGetInfo failed "<<GetLastError())
		if(ComputerName) NetApiBufferFree(ComputerName);		
		return FALSE;
	}
	
	if(!local&&ComputerName) NetApiBufferFree(ComputerName);

	
	WideCharToMultiByte( CP_ACP, 0, ui->usri3_profile, -1,CentralProfile, MAX_PATH, 
		NULL, NULL );
	WideCharToMultiByte( CP_ACP, 0, ui->usri3_home_dir_drive, -1,Homedrive, 5, 
		NULL, NULL );

	WideCharToMultiByte( CP_ACP, 0, ui->usri3_home_dir, -1,Homedir, MAX_PATH, 
		NULL, NULL );
	
	// Extract the homepath from the homedir
	if(Homedir[0]!='\\') {
		//It is a local path, just remove the driveletter
		UI.PutEnvString("HOMEPATH",Homedir+2);
		if(!Homedrive[0]) {
		    Homedrive[0]=Homedir[0];
		    Homedrive[1]= ':';
		}
	} else {
		//We have a remote path
		// So we have to search the 4.\ and create a homeshare variable.
		pos = strchr(Homedir+2,'\\');
		if(pos) pos = strchr(pos+1,'\\');
		if(pos) {
			UI.PutEnvString("HOMEPATH",pos);
			*pos=0;
			UI.PutEnvString("HOMESHARE",Homedir);
		}
	}

	// Global policies only for domain users... (I don't know if this is correct)
	if(!local) 	    
	    ProfInfo.dwFlags |= GetPolicyUpdatePath(PolicyPath);
	
	// Create default policy path if needed
	if((ProfInfo.dwFlags&2)==2&&!PolicyPath[0])	
	    sprintf(PolicyPath,"%s\\netlogon\\ntconfig.pol",ServerName);
	
	NetApiBufferFree(ui);
	if(!local) {
		ProfInfo.lpServerName=ServerName;
		sprintf(DefaultPath,"%s\\netlogon\\Default User",ServerName);
		ProfInfo.lpDefaultPath=DefaultPath;
	}

	if(CentralProfile[0]=='\\') ProfInfo.lpProfilePath=CentralProfile;
	ProfInfo.lpPolicyPath=PolicyPath;
	ProfInfo.lpUserName=UserName;

	EnterCriticalSection(&loginCS);
	ret=LoadUserProfile(User,&ProfInfo);
	if(!ret) DBG("LoadUserProfile returned "<<ret<<" Error: "<<GetLastError())
	LeaveCriticalSection(&loginCS);
	

	UI.PutEnvString("HOMEDRIVE",Homedrive);
	if(ret) {
	    UI.LoadUserEnvironment(User);
	    return ProfInfo.hProfile;
	} else return 0;
}

	
BOOL InitEnvironment() {
    DBG("Entering InitEnvironment()");
    hUserenv=LoadLibrary("userenv.dll");
    if(!hUserenv) {
	AddToMessageLog(IDM_LOADLIBRARY,GetLastError(),"userenv.dll ");
	DBG("Could not load userenv.dll");
	return FALSE;
    }
    LoadUserProfile=(LoginFunc)GetProcAddress(hUserenv,"LoadUserProfileA");
    if(!LoadUserProfile) {
	AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"LoadUserProfileA");
	return FALSE;
    }
    UnloadUserProfile=(LogoutFunc)GetProcAddress(hUserenv,"UnloadUserProfile");
    if(!UnloadUserProfile) {
	AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"UnloadUserProfileA");
	return FALSE;
    }
    
    CreateEnvironmentBlock=(CreateEnvFunc)GetProcAddress(hUserenv,"CreateEnvironmentBlock");
    if(!UnloadUserProfile) {
	AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"CreateEnvironmentBlock");
	return FALSE;
    }
    
    DestroyEnvironmentBlock=(DestroyEnvFunc)GetProcAddress(hUserenv,"DestroyEnvironmentBlock");
    if(!UnloadUserProfile) {
	AddToMessageLog(IDM_PROCADDRESS,GetLastError(),"DestroyEnvironmentBlock");
	return FALSE;
    }
    InitializeCriticalSection(&loginCS);
    DBG("Leaving InitEnvironment()");
    return TRUE;
}

BOOL FreeEnvironment() {
	FreeLibrary(hUserenv);
	DeleteCriticalSection(&loginCS);
	return TRUE;
}

