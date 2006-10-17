#include <stdio.h>
#include <iostream>

#include "helpers.h"
#include "service.h"
#include "messages.h"

#ifdef __cplusplus
extern "C" {
#endif


extern BOOL (WINAPI*ProcessId2SessionId)(DWORD,DWORD *);

#ifdef _DEBUG
void DbgOut(char* str) {
	std::cerr<<str;
}
#endif

void LogLogMessage(DWORD ID,PSID Sid,DWORD numInserts,LPTSTR *lpStrings) {
    char *message=0;
    DWORD res;

    if(!debug_flag) return;

    res=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_FROM_HMODULE,
	          NULL,ID,0,(LPTSTR)&message,0,lpStrings);
    if(!message || !res) {
	DBG("LOGGING: Cannot format message "<<GetLastError());
	DBG("Message ID: "<<ID);
	if(numInserts) {
	    DBG("ID=="<<ID<<" Inserts are:");
	    for(DWORD i=0;i<numInserts;++i)
	       DBG(lpStrings[i]);
	}
    } else {
	DBG("Logging: "<<message);
	LocalFree(message);
    }
}

TOKEN_USER *GetActualUserToken(DWORD *error) {
	HANDLE hThreadToken;
	TOKEN_USER *pTokenUser;
	DWORD dwActSize;

	if(!OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,FALSE,&hThreadToken)) {
	    *error = GetLastError();
	    if(*error == ERROR_NO_TOKEN) {
		if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hThreadToken)) {
		    *error = GetLastError();
		} else *error = ERROR_SUCCESS;
	    } 
	    if(*error != ERROR_SUCCESS) {
		DBG("GetActualUserToken: OpenThreadToken failed "<<*error)
		return 0;
	    }
	}

	dwActSize=0;
	pTokenUser=0;
	GetTokenInformation(hThreadToken,TokenUser,pTokenUser,0,&dwActSize);
	
	if(dwActSize) pTokenUser=(TOKEN_USER*)malloc(dwActSize);
	else pTokenUser=0;

	if(!pTokenUser||!GetTokenInformation(hThreadToken,TokenUser,pTokenUser,dwActSize,&dwActSize) ){
		*error = GetLastError();
		DBG("GetActualUserToken: GetTokenInformation failed "<<*error)
		if(hThreadToken) CloseHandle(hThreadToken);
		if(pTokenUser) free(pTokenUser);
		return 0;
	}
		
	CloseHandle(hThreadToken);
	return pTokenUser;

}

BOOL CompareUser(PSID sid,DWORD *error) {
    
    TOKEN_USER *pTokenUser;
    BOOL equal;
    
    if(!sid) {
	*error = RPC_S_INVALID_ARG;
	return FALSE;
    }
    
    pTokenUser=GetActualUserToken(error);
    if(!pTokenUser) return FALSE;
    
    if(!IsValidSid(pTokenUser->User.Sid)) {
	*error = RPC_S_INVALID_SECURITY_DESC;
	free(pTokenUser);
	return FALSE;
    }
    
    equal=EqualSid(pTokenUser->User.Sid,sid);
    *error = RPC_S_OK;
    free(pTokenUser);
    return equal;
}

BOOL CheckGroupMembership(PSID GroupSid, DWORD *error) {
	HANDLE hThreadToken;
	TOKEN_GROUPS *pTokenGroups;
	DWORD dwActSize;
	BOOL equal;
	DWORD i;


	if(!IsValidSid(GroupSid)) {
		*error = GetLastError();
		return FALSE;
	}

	if(!OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,FALSE,&hThreadToken)) {				
		*error = GetLastError();
		return FALSE;
	}

	dwActSize=0;
	pTokenGroups=0;
	GetTokenInformation(hThreadToken,TokenGroups,pTokenGroups,0,&dwActSize);
	pTokenGroups=(TOKEN_GROUPS*)malloc(dwActSize);

	if(!dwActSize||!GetTokenInformation(hThreadToken,TokenGroups,pTokenGroups,dwActSize,&dwActSize) ){
		*error = GetLastError();
		if(hThreadToken) CloseHandle(hThreadToken);
		if(pTokenGroups) free(pTokenGroups);
		return FALSE;
	}

	CloseHandle(hThreadToken);
		
	equal = FALSE;
	for(i=0;i<pTokenGroups->GroupCount && !equal;i++) {
		equal=EqualSid(pTokenGroups->Groups[i].Sid,GroupSid);
	}
	*error = 0;
	CloseHandle(hThreadToken);
	free(pTokenGroups);
	return equal;
}


void NormalizeDomainname(char *Domain) {
	DWORD Size=MAX_SIZE;
	char Buffer[MAX_SIZE];
	GetComputerName(Buffer,&Size);
	if(!stricmp(Buffer,Domain) || !stricmp(Domain,"local")) 
		strcpy(Domain,".");
}


BOOL IsUserAdmin(WCHAR *wszDCName) {
	DWORD error;
	if(!LocalAdmins) 
	    LocalAdmins = LookupAliasFromRid(DOMAIN_ALIAS_RID_ADMINS);
	if(!DomainAdmins)
	    DomainAdmins = CreateSidFromRid(wszDCName,DOMAIN_GROUP_RID_ADMINS);

	return (CheckGroupMembership(LocalAdmins,&error) || CheckGroupMembership(DomainAdmins,&error));
}


PSID CreateSidFromRid(LPWSTR TargetComputer,DWORD Rid) {
    PUSER_MODALS_INFO_2 umi2=0;
    NET_API_STATUS nas;
    DWORD err;
    UCHAR SubAuthorityCount;
    PSID pSid;
    
    // 
    // get the account domain Sid on the target machine
    // note: if you were looking up multiple sids based on the same
    // account domain, only need to call this once.
    // 
    DBG("Entering CreateSidFromRid");

    //DBG("Calling NetUserModalsGet");
    nas = NetUserModalsGet(TargetComputer, 2, (LPBYTE *)&umi2);
    
    if(nas != NERR_Success) {
	SetLastError(nas);
	DBG("Failed with "<<nas);
	return 0;
    }
    
    if(!umi2 || !IsValidSid(umi2->usrmod2_domain_id)) {
	DBG("NetUserModalsGet returned invalid data");
	return 0;
    }
    
    DBG("Got modals");
    SubAuthorityCount = *GetSidSubAuthorityCount
	(umi2->usrmod2_domain_id);
    DBG("SubAuthorityCount = "<<(DWORD)SubAuthorityCount);
    
    // 
    // allocate storage for new Sid. account domain Sid + account Rid
    // 
    
    pSid = (PSID)HeapAlloc(GetProcessHeap(), 0,
	GetSidLengthRequired((UCHAR)(SubAuthorityCount + 1)));
    
    if(pSid != NULL) {
	
	if(InitializeSid(
	    pSid,
	    GetSidIdentifierAuthority(umi2->usrmod2_domain_id),
	    (BYTE)(SubAuthorityCount+1)
	    )) {
	    
	    DWORD SubAuthIndex = 0;
	    
	    // 
	    // copy existing subauthorities from account domain Sid into
	    // new Sid
	    // 
	    
	    for( ; SubAuthIndex < SubAuthorityCount ; SubAuthIndex++) {
		DBG("Copying subauthority "<<SubAuthIndex);
		*GetSidSubAuthority(pSid, SubAuthIndex) =
		    *GetSidSubAuthority(umi2->usrmod2_domain_id,
		    SubAuthIndex);
	    }
	    
	    // 
	    // append Rid to new Sid
	    // 
	    DBG("Appending Rid");
	    *GetSidSubAuthority(pSid, SubAuthorityCount) = Rid;
	    
	} else {
	    err = GetLastError();
	    DBG("InitializeSid failed with "<<err);
	    HeapFree(GetProcessHeap(), 0, pSid);
	    NetApiBufferFree(umi2);
	    SetLastError(err);
	    return 0;
	}
    } else {
	err = GetLastError();
	DBG("HeapAlloc failed");
	NetApiBufferFree(umi2);
	SetLastError(err);
	return 0;
    }
    
    NetApiBufferFree(umi2);
    DBG("Leaving CreateSidFromRid");
    return pSid;
}

PSID LookupAliasFromRid(DWORD Rid) {
       SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
       PSID pSid;

       // 
       // Sid is the same regardless of machine, since the well-known
       // BUILTIN domain is referenced.
       // 

       if(AllocateAndInitializeSid(
               &sia,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               Rid,
               0, 0, 0, 0, 0, 0,
               &pSid
               )) {

          
          return pSid;
       }

       return 0;
   }

BOOL ConvertSid(PSID pSid, LPTSTR pszSidText, LPDWORD dwBufferLen)   {
	PSID_IDENTIFIER_AUTHORITY psia;
	DWORD dwSubAuthorities;
	DWORD dwSidRev=SID_REVISION;      
	DWORD dwCounter;      
	DWORD dwSidSize;
	//      
	// test if Sid passed in is valid      
	//
	if(!IsValidSid(pSid)) return FALSE;      // obtain SidIdentifierAuthority
	psia=GetSidIdentifierAuthority(pSid);      // obtain sidsubauthority count
	dwSubAuthorities=*GetSidSubAuthorityCount(pSid);      
	//
	// compute buffer length
	// S-SID_REVISION- + identifierauthority- + subauthorities- + NULL      
	//
	dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);      
	//
	// check provided buffer length.
	// If not large enough, indicate proper size and setlasterror      
	//
	if (*dwBufferLen < dwSidSize){         
		*dwBufferLen = dwSidSize;
		SetLastError(ERROR_INSUFFICIENT_BUFFER);         
		return FALSE;      
	}
	//      
	// prepare S-SID_REVISION-      
	//
	dwSidSize=wsprintf(pszSidText, TEXT("S-%lu-"), dwSidRev );      
	//
	// prepare SidIdentifierAuthority      
	//
	if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) ){
		dwSidSize+=wsprintf(pszSidText + lstrlen(pszSidText),
			TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
			(USHORT)psia->Value[0],
			(USHORT)psia->Value[1],
			(USHORT)psia->Value[2],
			(USHORT)psia->Value[3],
			(USHORT)psia->Value[4],
			(USHORT)psia->Value[5]);      
	} else {
			dwSidSize+=wsprintf(pszSidText + lstrlen(pszSidText),
				TEXT("%lu"),
				(ULONG)(psia->Value[5]      )   +
				(ULONG)(psia->Value[4] <<  8)   +
				(ULONG)(psia->Value[3] << 16)   +
				(ULONG)(psia->Value[2] << 24)   );      
	}      
	//
	// loop through SidSubAuthorities      
	//
	for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++){
		dwSidSize+=wsprintf(pszSidText + dwSidSize, TEXT("-%lu"),
			*GetSidSubAuthority(pSid, dwCounter) );      
	}      
	return TRUE;   
}

TOKEN_USER *GetLoggedOnUserToken(HANDLE *hProc) {
    DWORD *procs=0,sid;
    DWORD dwSize=0,dwActSize;
    DWORD dwIndex,dwKeyType;
    DWORD UserSize=256,DomainSize=256;
    TOKEN_USER *pTokenUser;
    char szFileName[MAX_PATH],ShellName[MAX_PATH]="EXPLORER.EXE";
    HANDLE hProcess,hProcToken;
    HMODULE hMod;
    bool finish=false;
    LONG lErrorCode;
    HKEY hKey;
    
    *hProc=0;
    lErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
	"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 
	0, KEY_READ, &hKey);
    if(lErrorCode != ERROR_SUCCESS) {
	AddToMessageLog(IDM_REGISTRY,lErrorCode,
	    "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
	
    } else {
	dwActSize=MAX_PATH;
	lErrorCode = RegQueryValueEx(hKey, TEXT("Shell"), NULL,
	    &dwKeyType, (LPBYTE)ShellName, &dwActSize);
	
	if (lErrorCode != ERROR_SUCCESS) {
	    RegCloseKey(hKey);
	    AddToMessageLog(IDM_SHELL,lErrorCode,0);
	    
	}
	RegCloseKey(hKey);
    }
    
    procs=(DWORD*)malloc(100*sizeof(DWORD));
    dwActSize=100*sizeof(DWORD);
    do{
	if( !lpfEnumProcesses( procs, dwActSize, &dwSize ) ) {
	    AddToMessageLog(IDM_PROCS,GetLastError(),0);
	    if(procs) free(procs);
	    return 0;
	}
	if(dwActSize<=dwSize) {
	    dwActSize*=2;
	    procs=(DWORD*)realloc(procs,dwActSize);
	} else finish=true;
	
    } while(!finish);
    
    dwSize /= sizeof( DWORD );
    for( dwIndex = 0 ; dwIndex < dwSize ; dwIndex++ ){
	if(ProcessId2SessionId && (!ProcessId2SessionId(procs[dwIndex],&sid) || (sid != 0)))
	    continue;
	szFileName[0] = 0 ;
	// Open the process (if we can... security does not
	// permit every process in the system).
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ, 
	    FALSE, procs[ dwIndex ] ) ;			  
	if( hProcess != NULL ) {
	    // Here we call EnumProcessModules to get only the
	    // first module in the process this is important,
	    // because this will be the .EXE module for which we
	    // will retrieve the full path name in a second.
	    if( lpfEnumProcessModules( hProcess, &hMod,
		sizeof( hMod ), &dwActSize ) )  {
		// Get Full pathname:
		if( !lpfGetModuleFileNameEx( hProcess, hMod,
		    szFileName, sizeof( szFileName ) ) )  {
		    szFileName[0] = 0 ; 				   
		}				
	    } else {
		// We could not open the proc. So just try the next
		CloseHandle(hProcess);
		continue;
	    }
	    if(strlen(ShellName)>strlen(szFileName)) {
		// This was not, what we were looking for.
		CloseHandle(hProcess);
		continue;
	    }
	    if( stricmp( szFileName+(strlen(szFileName)-strlen(ShellName)),
		ShellName)!=0) {
		// O.K. not the shell process.
		CloseHandle(hProcess);
		continue;
	    }
	    
	    // We found it.
	    
	    if(!OpenProcessToken(hProcess,TOKEN_QUERY|TOKEN_DUPLICATE,&hProcToken)) {
		AddToMessageLog(IDM_PROCTOKEN,GetLastError(),0);
		CloseHandle(hProcess);
		if(procs) free(procs);
		return 0;
	    }
	    
	    CloseHandle( hProcess ) ;
	    *hProc = hProcToken;
	    dwActSize=0;
	    pTokenUser=0;
	    GetTokenInformation(hProcToken,TokenUser,pTokenUser,0,&dwActSize);
	    
	    pTokenUser=(TOKEN_USER*)malloc(dwActSize);
	    
	    if(!dwActSize||!GetTokenInformation(hProcToken,TokenUser,pTokenUser,dwActSize,&dwActSize) ){
		if(hProcToken) CloseHandle(hProcToken);
		if(procs) free(procs);
		if(pTokenUser) free(pTokenUser);
		*hProc = 0;
		return 0;
	    }
	    if(procs) free(procs);
	    //if(hProcToken) CloseHandle(hProcToken);
	    return pTokenUser;
	}
    }
    if(procs) free(procs);
    return 0;
}

BOOL GetProcessOwner(HANDLE hProcess,char *buf,DWORD *size) {
    
    HANDLE hProcToken=0;
    TOKEN_USER *pTokenUser;
    DWORD dwActSize,UserSize=255,DomainSize=255;
    char UserName[255],DomainName[255];
    SID_NAME_USE NameUse;
    
    if(!OpenProcessToken(hProcess,TOKEN_QUERY,&hProcToken)) 
	return FALSE;
    
    
    CloseHandle( hProcess ) ;
    dwActSize=0;
    pTokenUser=0;
    GetTokenInformation(hProcToken,TokenUser,pTokenUser,0,&dwActSize);
    if(!dwActSize) {
	CloseHandle(hProcToken);
	return FALSE;
    }
    pTokenUser=(TOKEN_USER*)malloc(dwActSize);
    
    if(!pTokenUser||!GetTokenInformation(hProcToken,TokenUser,pTokenUser,dwActSize,&dwActSize) ){
	if(hProcToken) CloseHandle(hProcToken);
	if(pTokenUser) free(pTokenUser);
	return FALSE;
    }

    if(!LookupAccountSid(0,pTokenUser->User.Sid,
	UserName,&UserSize,
	DomainName,&DomainSize,
	&NameUse)) {
	UserSize=255;
	ConvertSid(pTokenUser->User.Sid,UserName,&UserSize);
	free(pTokenUser);
	return TRUE;
    } else {
	if(UserSize+DomainSize>*size) {
	    *size = UserSize+DomainSize;
	    free(pTokenUser);
	    return FALSE;
	}  else {
	    if(stricmp(UserName,"SYSTEM")) {
		*size = UserSize+DomainSize;
		sprintf(buf,"%s/%s",DomainName,UserName);
	    } else {
		*size = 7;
		strcpy(buf,"SYSTEM");
	    }
	    free(pTokenUser);
	    return TRUE;
	}	
    }
    
}

DWORD GetPrimaryToken( HANDLE *newToken) {
    HANDLE token=*newToken;
    DWORD err = ERROR_SUCCESS;

    if(!DuplicateTokenEx(token,TOKEN_ALL_ACCESS,0,SecurityImpersonation,TokenPrimary,newToken))
	err =  GetLastError();
    CloseHandle(token);
    return err;
}

typedef void (WINAPI *PROCEXITPROCESS)(UINT);

BOOL InjectExitProc(HANDLE hProcess, DWORD ExitCode) {
	
	// Kernel32.DLL's HINSTANCE is used to get the
	// address of LoadLibraryA or LoadLibraryW and
	// FreeLibrary. Hopefully Kernel32.dll is mapped
        // to the same address in the remote process...
	HINSTANCE hinstKrnl = GetModuleHandle(__TEXT("Kernel32"));
	PROCEXITPROCESS fnExitProc;
	DWORD dwThreadId = 0;
	HANDLE hThread = NULL;
	
	fnExitProc = (PROCEXITPROCESS )GetProcAddress(hinstKrnl,"ExitProcess");
	if(!fnExitProc) return FALSE;
	
	hThread = CreateRemoteThread(hProcess, NULL, 0, 
		(LPTHREAD_START_ROUTINE)fnExitProc,
		(void*)ExitCode, 0, &dwThreadId);
	if (hThread == NULL) return FALSE;
	
	CloseHandle(hThread);
	return(TRUE);
}


HANDLE hToken; 
TOKEN_PRIVILEGES tkp; 

error_status_t ShutDown(BOOL restart)
{
 
  // Get a token for this process. 
 
  if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
  {
    DBG("OpenProcessToken failed"); 
	return GetLastError();
  }
 
// Get the LUID for the shutdown privilege. 
 
  LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 
 
  tkp.PrivilegeCount = 1;  // one privilege to set    
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
  // Get the shutdown privilege for this process. 
 
  SetLastError(ERROR_SUCCESS);

  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
 
  // Cannot test the return value of AdjustTokenPrivileges. 
  DWORD errocde = GetLastError();
  if ( errocde != ERROR_SUCCESS) 
  {
    DBG("AdjustTokenPrivileges failed"); 
	return  errocde;
  }
 
// Shut down the system and force all applications to close. 
 
  OSVERSIONINFO VersionInfo={sizeof(VersionInfo)};
  GetVersionEx(&VersionInfo);

  UINT uFlags;

  char DateTime[255];
  GetDateTimeString(DateTime);


 
  if (restart)
  {
    uFlags = (EWX_REBOOT);
	DBG(DateTime<<": Reboot Computer");
  }
  else
  {
	uFlags = (EWX_POWEROFF);
	DBG(DateTime<<": Shutdown Computer");
  }

  
  if (VersionInfo.dwMajorVersion >= 5)
  {	
	#define EWX_FORCEIFHUNG     0x00000010
	uFlags = (uFlags | EWX_FORCEIFHUNG);
	DBG("EWX_FORCEIFHUNG");
  }
  else
	uFlags = (uFlags | EWX_FORCE);	
 
  DBG(std::flush);
  //close logfile
  if(o) {
	o->close();
	delete o;
	o=0;
   }

  Sleep(1);
  
 if (!ExitWindowsEx(uFlags, 0)) 
  {
    DBG("ExitWindowsEx failed"); 
		return GetLastError();
  }
return ERROR_SUCCESS;
}


char * GetDateTimeString(char * DateTimeString)
{
    if (DateTimeString == NULL)
    {
        return NULL;
    }
    else
    {
        SYSTEMTIME SystemTime;
		
        GetLocalTime(&SystemTime);
	

        char * tmp;
        tmp = (char *) calloc(50,1);
		itoa(SystemTime.wYear,tmp,10);
        strcpy(DateTimeString,tmp);
		strcat(DateTimeString,"/");
		itoa(SystemTime.wMonth,tmp,10);
        strcat(DateTimeString,tmp);
		strcat(DateTimeString,"/");
        itoa(SystemTime.wDay,tmp,10);
        strcat(DateTimeString,tmp);
        strcat(DateTimeString," ");
        
        itoa(SystemTime.wHour,tmp,10);
        if (strlen(tmp)<2)
            strcat(DateTimeString,"0");
        strcat(DateTimeString,tmp);
        strcat(DateTimeString,":");
        itoa(SystemTime.wMinute,tmp,10);
        if (strlen(tmp)<2)
            strcat(DateTimeString,"0");
        strcat(DateTimeString,tmp);
        strcat(DateTimeString,":");
        itoa(SystemTime.wSecond,tmp,10);
        if (strlen(tmp)<2)
            strcat(DateTimeString,"0");
        strcat(DateTimeString,tmp);
        strcat(DateTimeString," ");
        
        free(tmp);
       
    }  
	return DateTimeString;
} 

#ifdef __cplusplus
}
#endif
