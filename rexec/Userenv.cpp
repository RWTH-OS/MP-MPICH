#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lm.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <winnls.h>
#include <map>
#include <string.h>
#include <malloc.h>

#include "environment.h"
#include "service.h"
#include "messages.h"
#include "helpers.h"

//#define LOG(m) DBG(m)


#ifndef LOG
#define LOG(m)
#endif 

int strnIcmp (
	      const char * first,
	      const char * last,
	      size_t count
	      )
{
    int f,l;
    
    if ( count )
    {
	do {
	    if ( ((f = (unsigned char)(*(first++))) >= 'a') &&
		(f <= 'z') )
		f = f + 'A' - 'a';
	    
	    if ( ((l = (unsigned char)(*(last++))) >= 'a') &&
		(l <= 'z') )
		l = l + 'A' - 'a';   
	} while ( --count && f && (f == l) );
	return( f - l );
    }
    return( 0 );
}


UserInfo::UserInfo() {
	Environment=(char*)malloc(128); 
	*Environment=0;
	EnvSize=128;
	DriveMask=0;
	UsedSize=1;
}

void UserInfo::LoadSystemEnvironment() {
	TCHAR szRegKey[1024] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
	LONG lErrorCode;
	HKEY hKey;

	//return;
	LOG("Entering UserInfo::LoadSystemEnvironment()")

	lErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey, 0, KEY_READ, &hKey);
	if (lErrorCode != ERROR_SUCCESS) {
		SetLastError(lErrorCode);

		LOG("RegOpenKeyEx failed with "<<lErrorCode)
		LOG("Leaving UserInfo::LoadSystemEnvironment()")
		return ;
	}
	
	LoadEnvFromKey(hKey);
	RegCloseKey(hKey);
	PutEnvString("SystemRoot",getenv("SystemRoot"));
	PutEnvString("SystemDrive",getenv("SystemDrive"));
	PutEnvString("COMPUTERNAME",getenv("COMPUTERNAME"));

	LOG("Leaving UserInfo::LoadSystemEnvironment()")

}

void UserInfo::LoadUserEnvironment(HANDLE User) {
    WCHAR *ucEnv,*actPos,*eqsign;
    DWORD actLen;
    
    LOG("Entering UserInfo::LoadUserEnvironment")
    if(!CreateEnvironmentBlock((void**)&ucEnv,User,FALSE))
	return;
    __try {
	actPos = ucEnv;
	while(*actPos != 0) {
	    actLen = wcslen(actPos);
	    eqsign = wcschr(actPos+1,'=');
	    if(eqsign) {
		*eqsign = 0;
		PutEnvString(actPos,eqsign+1);
	    }
	    actPos += actLen+1;
	    
	}
    } __finally {
	DestroyEnvironmentBlock(ucEnv);
    }
    LOG("Leaving UserInfo::LoadUserEnvironment")
}

void UserInfo::MergeEnvironment(char* Env) {
    char *actPos,*eqsign;
    DWORD actLen;
    if(!Env) return;
    __try {
	actPos = Env;
	while(*actPos != 0) {
	    actLen = strlen(actPos);
	    eqsign = strchr(actPos+1,'=');
	    if(eqsign) {
		*eqsign = 0;
		PutEnvString(actPos,eqsign+1);
	    }
	    actPos += actLen+1;
	}
    } __finally {}
}
/*	
void UserInfo::LoadUserEnvironment(HANDLE User) {
	
	TCHAR szRegKey[MAX_PATH],SID[256];
	LONG lErrorCode;
	HKEY hKey;
	PSID pSid;
	DWORD size=256;

	LOG("Entering UserInfo::LoadUserEnvironment()")
	if(!ObtainSid(User,&pSid)) {
		LOG("Could not get SID")
		LOG("Leaving UserInfo::LoadUserEnvironment()")
		return;
	}
	if(!ConvertSid(pSid,SID,&size)) {
		LocalFree(pSid);
		LOG("Could not convert SID")
		LOG("Leaving UserInfo::LoadUserEnvironment()")
		return;
	}
	lstrcpy(szRegKey, SID);
	lstrcat(szRegKey, TEXT("\\Environment"));
	lErrorCode = RegOpenKeyEx(HKEY_USERS, szRegKey, 0, KEY_READ, &hKey);
	if (lErrorCode != ERROR_SUCCESS) {
		SetLastError(lErrorCode);
		AddToMessageLog(IDM_NOTIFY,lErrorCode,"Could not read user environment");
		LOG("Could not read registry")
		LOG("Leaving UserInfo::LoadUserEnvironment()")
		return ;
	}
	
	LoadEnvFromKey(hKey);
	RegCloseKey(hKey);
	LocalFree(pSid);
	LOG("Leaving UserInfo::LoadUserEnvironment()")
}
*/
#define STARTSIZE 256
void UserInfo::LoadEnvFromKey(HKEY key) {
    DWORD index;
    TCHAR *lpName,*lpValue,*path,*buf;
    DWORD NameSize,ValueSize,PathSize,BufSize,
	type,reslen,Size1,Size2,NumValues,
	NeededLen;
    long res;
    
    LOG("Entering UserInfo::LoadEnvFromKey()")
	
	path = (TCHAR*)malloc(STARTSIZE*sizeof(TCHAR));
    buf  = (TCHAR*)malloc(STARTSIZE*sizeof(TCHAR));
    PathSize = STARTSIZE;
    BufSize = STARTSIZE;
    
    if(RegQueryInfoKey(key,0,0,0,0,0,0,&NumValues,&NameSize,&ValueSize,0,0) != ERROR_SUCCESS) {
	LOG("RegQueryInfoKeyFailed");
	LOG("Leaving UserInfo::LoadEnvFromKey()");
    }
    
    ++NameSize;
    ++ValueSize;
    lpName = (TCHAR*)malloc(NameSize*sizeof(TCHAR));
    lpValue = (TCHAR*)malloc(ValueSize*sizeof(TCHAR));
    
    for(index=0;index<NumValues;index++) {
	Size1=NameSize;
	Size2=ValueSize;
	res=RegEnumValue(key,index,lpName,&Size1,0,&type,(unsigned char*)lpValue,&Size2);
	reslen=ExpandEnvironmentStrings(lpValue,buf,BufSize);
	if(reslen>BufSize) {
	    buf=(TCHAR*)realloc(buf,reslen+1);
	    BufSize=reslen+1;
	    reslen=ExpandEnvironmentStrings(lpValue,buf,BufSize);
	}
	    
	if(!_stricmp(lpName,"path")) {
	    Size2 = PathSize;
	    GetEnvString("Path",path,&Size2);
	    NeededLen = Size2+reslen+2;
	    if(NeededLen>PathSize) {
		path = (TCHAR*)realloc(path,NeededLen);
		PathSize = NeededLen;
		GetEnvString("Path",path,&NeededLen);
	    }
	    
	    if(path[0]) strcat(path,";");
	    strcat(path,buf);
	    PutEnvString("Path",path);
	} else PutEnvString(lpName,buf);
    }
    SetLastError(res);
    free(lpName);
    free(lpValue);
    if(path) free(path);
    if(buf) free(buf);
    LOG("Leaving UserInfo::LoadEnvFromKey()")
}


void UserInfo::PutEnvString(wchar_t *Name,wchar_t *value) {
    DWORD len1,len2;
    char *mName,*mValue;

    len1 = wcslen(Name);
    len2 = wcslen(value);
    __try {
	mName = (char*)alloca(len1+1);
	mValue = (char*)alloca(len2+1);

	if(!mName || !mValue) return;

	WideCharToMultiByte( CP_ACP, 0, Name, -1,mName, len1+1, 
		NULL, NULL );
	WideCharToMultiByte( CP_ACP, 0, value, -1,mValue, len2+1, 
		NULL, NULL );
	PutEnvString(mName,mValue);
    } __finally{
    }

}


void UserInfo::PutEnvString(char *Name,char *value) {
	char *OldPos;
	DWORD oldLen,newLen,OldIndex,oldSize,namelen;

	LOG("Entering UserInfo::PutEnvString("<<Name<<"="<<value<<")")

	if(!value) value="";
	namelen = strlen(Name);
	newLen=namelen+strlen(value)+2;

	oldLen = 0;
	OldPos=GetEnvString(Name,0,&oldLen);
	if(!OldPos) {
		LOG("Leaving UserInfo::PutEnvString()")
		return;
	}
	if(oldLen) 
	    oldLen += namelen+1;
	OldIndex=(DWORD)(OldPos-Environment);
	if(oldLen>=newLen) {
		memmove(OldPos+newLen,OldPos+oldLen,UsedSize-(OldIndex+oldLen));
		UsedSize-=(oldLen-newLen);
	} else {
		oldSize=UsedSize;
		UsedSize+=(newLen-oldLen);
		if(EnvSize<UsedSize) {
			EnvSize=max(UsedSize,2*EnvSize);
			Environment=(char*)realloc(Environment,EnvSize);
			OldPos=Environment+OldIndex;
		}
		memmove(OldPos+newLen,OldPos+oldLen,oldSize-OldIndex-oldLen);
	}
	sprintf(OldPos,"%s=%s",Name,value);
	LOG("Leaving UserInfo::PutEnvString()")
}


char *UserInfo::GetEnvString(char *Name,char *value,DWORD *BufSize) {
    char *act=Environment,
	*pos; 
    int res;
    DWORD lenAll,lenStName,lenGName,lenValue;
    
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
	/*if(lenStName!=lenGName) {
	act+=(lenAll+1);
	continue;
	}*/
	res=strnIcmp(act,Name,min(lenStName,lenGName));
	
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


void UserInfo::DumpEnv() {
    char* act=Environment;
    DWORD len;
    while(*act) {
	len=strlen(act)+1;
	std::cerr<<act<<std::endl;
	act+=len;
    }
}

/*

// Specify from and to without final backslash!
BOOL CopyDirectory(char *from, char* to,SECURITY_ATTRIBUTES *sattr) {
	DWORD attr;
	char newSrc[MAX_PATH],newDest[MAX_PATH];
	WIN32_FIND_DATA findData;
	WIN32_FILE_ATTRIBUTE_DATA attrData;
	HANDLE hSearch;
	BOOL res;
	

	attr=GetFileAttributes(to);
	if(attr==0xFFFFFFFF) {
		if(!CreateDirectory(to,sattr)) {
			return FALSE;
		}
	} else if((attr&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY) return FALSE;
	
	if(from[strlen(from)-1]=='\\') from[strlen(from)-1]=0;
	if(to[strlen(to)-1]=='\\') to[strlen(to)-1]=0;
	sprintf(newSrc,"%s\\*",from);

	hSearch=FindFirstFile(newSrc,&findData);
	if(hSearch==INVALID_HANDLE_VALUE) return TRUE;

	do {
		if(!strcmp(".",findData.cFileName)||!strcmp("..",findData.cFileName)) continue;
		sprintf(newDest,"%s\\%s",to,findData.cFileName);
		sprintf(newSrc,"%s\\%s",from,findData.cFileName);
		
		if((findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY) {
			CopyDirectory(newSrc,newDest,sattr);			
		} else {
			res=GetFileAttributesEx(newDest,GetFileExInfoStandard,&attrData);
			if(!res||(CompareFileTime(&attrData.ftLastWriteTime,&findData.ftLastWriteTime)<0)) {
				CopyFile(newSrc,newDest,FALSE);
			} 
		}
	} while (FindNextFile(hSearch,&findData));
	
	CloseHandle(hSearch);
	return TRUE;
}


BOOL CopyProfile(char *remotePath,char *localPath,PSID UserSid) { 
	
	
	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY,
	PSID System=0,Admin=0;
	SECURITY_ATTRIBUTES attr;
	PSECURITY_DESCRIPTOR sd=0;
	DWORD ACLSize,aceSize;
	PACL acl=0;
	ACCESS_ALLOWED_ACE *ace=0;
	BOOL res=FALSE;

	if(!AllocateAndInitializeSid(
		&sia,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&Admin
		)) {
		return FALSE;
	}
	
	
	if(!AllocateAndInitializeSid(
		&sia,
		1,
		SECURITY_LOCAL_SYSTEM_RID,
		0,
		0, 0, 0, 0, 0, 0,
		&System
		)) {
		goto cleanup;
	}
	
	
	
	attr.bInheritHandle=FALSE;
	sd=LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
	if(!sd) return RTN_ERROR;
	if(!InitializeSecurityDescriptor(sd,SECURITY_DESCRIPTOR_REVISION)) {
		goto cleanup;
	}
	
	ACLSize=3*sizeof(ACCESS_ALLOWED_ACE)+GetLengthSid(Admin)+GetLengthSid(System)+
		  GetLengthSid(UserSid)-3*sizeof(DWORD)+sizeof (ACL);
	
	acl=(PACL)LocalAlloc(LPTR,ACLSize);
	if(!acl) 
		goto cleanup;
	
	ZeroMemory(acl,ACLSize);
	if(!InitializeAcl(acl,ACLSize,ACL_REVISION)) {
		goto cleanup;
	}
	
	
	aceSize=GetLengthSid(Admin);
	if(aceSize<GetLengthSid(System)) aceSize=GetLengthSid(System);
	if(aceSize<GetLengthSid(UserSid)) aceSize=GetLengthSid(UserSid);
	
	aceSize=sizeof(ACCESS_ALLOWED_ACE)+aceSize-sizeof(DWORD);
	ace=(ACCESS_ALLOWED_ACE*)LocalAlloc(LPTR,aceSize);
	ZeroMemory(ace,aceSize);
	
	ace->Header.AceType=ACCESS_ALLOWED_ACE_TYPE;
	ace->Header.AceFlags=CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE;
	ace->Header.AceSize=sizeof(ACCESS_ALLOWED_ACE)+GetLengthSid(Admin)-sizeof(DWORD);
	ace->Mask=GENERIC_ALL;
	
	if(!CopySid(GetLengthSid(Admin),&(ace->SidStart),Admin)) 
		goto cleanup;
	
	if(!AddAce(acl,ACL_REVISION,MAXDWORD,(LPVOID)ace,ace->Header.AceSize)) {
		goto cleanup;
	}
	
	
	ace->Header.AceSize=sizeof(ACCESS_ALLOWED_ACE)+GetLengthSid(System)-sizeof(DWORD);
	if(!CopySid(GetLengthSid(System),&(ace->SidStart),System)) {
		goto cleanup;
	}

	if(!AddAce(acl,ACL_REVISION,MAXDWORD,(LPVOID)ace,ace->Header.AceSize)) {
		cerr<<GetLastError()<<": AddAce\n";
		goto cleanup;
	}


	ace->Header.AceSize=sizeof(ACCESS_ALLOWED_ACE)+GetLengthSid(UserSid)-sizeof(DWORD);
	if(!CopySid(GetLengthSid(UserSid),&(ace->SidStart),UserSid)) {
		goto cleanup;
	}

	if(!AddAce(acl,ACL_REVISION,MAXDWORD,(LPVOID)ace,ace->Header.AceSize)) {
		goto cleanup;
	}

	
	
	if(!SetSecurityDescriptorDacl(sd,TRUE,acl,FALSE)) 
		goto cleanup;
	
	
	attr.nLength=sizeof(attr);
	attr.lpSecurityDescriptor=sd;
	
	res=CopyDirectory(remotePath,localPath,&attr);
cleanup:
	if(sd) LocalFree(sd);
	if(acl) LocalFree(acl);
	if(ace) LocalFree(ace);
	if(Admin) FreeSid(Admin);
	if(System) FreeSid(System);
	return res;
} 


BOOL CreateLocalProfilePath(char *path,PSID UserSid,char *UserName) {
	HANDLE hFile;
	char actPath[MAX_PATH];
	DWORD counter=0;
	SECURITY_INFORMATION si=DACL_SECURITY_INFORMATION;
	PSECURITY_DESCRIPTOR pSD=0;
	ACL_SIZE_INFORMATION aclSizeInfo;
	PACL pAcl=0;
	DWORD size=0,sizeNeeded;
	BOOL res=FALSE;
	BOOL DaclPresent,DaclDefault;
	ACCESS_ALLOWED_ACE *tmpAce;

	ExpandEnvironmentStrings("%SYSTEMROOT%\\Profiles\\",actPath,MAX_PATH);
	
	sprintf(path,"%s%s",actPath,UserName);
	while(!res) {
		hFile=CreateFile(path,GENERIC_READ|READ_CONTROL,FILE_SHARE_READ|FILE_SHARE_WRITE,
							 0,OPEN_EXISTING,0,0);
		if(hFile==INVALID_HANDLE_VALUE) {
			if(GetLastError()==ERROR_FILE_NOT_FOUND) {
				res=TRUE;
				goto cleanup;
			}
			else if(GetLastError==ERROR_ACCESS_DENIED) {
				sprintf(path,"%s%s.%03u",actPath,UserName,counter);
				counter++;
				continue;
			} else goto cleanup;
		}

		sizeNeeded=0;
		GetUserObjectSecurity(hFile,&si,pSD,size,&sizeNeeded));
		if(sizeNeeded&&sizeNeeded>size) {
			pSD=(PSECURITY_DESCRIPTOR) realloc(pSD,sizeNeeded);
			size=sizeNeeded;
		}
		if(GetSecurityDescriptorDacl(pSD,&DaclPresent,&pAcl,&DaclDefault)&&DaclPresent) {
			GetAclInformation(pAcl,&aclSizeInfo,sizeof(aclSizeInfo),AclSizeInformation);
			for(int i=0;i<aclSizeInfo.AceCount;i++) {
				GetAce(pAcl,i,&tmpAce);
				if(tmpAce->Header.AceType!=ACCESS_ALLOWED_ACE_TYPE) continue;
				if(EqualSid(UserSid,(PSID)&(tmpAce->SidStart))) {
					res=TRUE;
					goto cleanup;
				}
			}
		}
		CloseHandle(hFile);
		hFile=INVALID_HANDLE_VALUE;
		sprintf(path,"%s%s.%03u",actPath,UserName,counter);
		counter++;
	}

cleanup:
	if(pSD) free(pSD);
	if(hFile!=INVALID_HANDLE_VALUE) CloseHandle(hFile);
	return res;
}

BOOL LoadUserHive(HANDLE User,NtrexecMessage *msg, HANDLE param,TCHAR *szSid) {
	TCHAR szLocalProfilePath[256],szRemoteProfilePath[256];
	//TCHAR szSid[256];      
	DWORD dwBufferLen=256,size=256;
	BOOL AddEntry=FALSE,res=FALSE;
	PSID UserSid=0;
	WCHAR wszUserName[256],wszDomainName[256];  // Unicode user and Domain name
    LPBYTE ComputerName=0;
	struct _USER_INFO_3 *ui;         // User structure

	char message[256],*path;
	

	CUserMap::iterator it;
	
	it=UserMap.find(UserKey(msg->Domain,msg->UserName));
	if(it==UserMap.end()) {
		it=UserMap.insert(CUserMap::value_type(UserKey(msg->Domain,msg->UserName),new UserInfo)).first;
	}

	(*it).second->LoginCount++;
	if((*it).second->HiveLoaded) return FALSE;
		
	ZeroMemory(szSid, (sizeof(szSid)/sizeof(TCHAR)));
	ZeroMemory(szRemoteProfilePath, (sizeof(szRemoteProfilePath)/sizeof(TCHAR)));
	ZeroMemory(szLocalProfilePath, (sizeof(szLocalProfilePath)/sizeof(TCHAR)));
	dwBufferLen = (sizeof(szSid)/sizeof(TCHAR));         
	//
	// obtain the sid         
	//
	dwBufferLen=256;
	
	if(!ObtainSid(User,&UserSid)) {
		NotifyLog("GetSID failed. ");
		return FALSE;
	}

	if (!ObtainSidString(UserSid, szSid, &dwBufferLen))  {
		NotifyLog("GetSIDString failed. ");
		goto cleanup;
	}

	GetComputerName(message,&size);
	if(_stricmp(msg->Domain,message)!=0) {
		
		MultiByteToWideChar( CP_ACP, 0, msg->Domain,strlen(msg->Domain)+1, 
				wszDomainName,sizeof(wszDomainName)/sizeof(wszDomainName[0]) );
		NetGetDCName( NULL, wszDomainName, &ComputerName );
		WideCharToMultiByte( CP_ACP, 0, (unsigned short *)ComputerName, -1,
        message, 256, NULL, NULL );
	} else	sprintf(message,"\\\\%s",msg->Domain);

	(*it).second->PutEnvString("LOGONSERVER",message);
	(*it).second->PutEnvString("USERDOMAIN",msg->Domain);
	(*it).second->PutEnvString("USERNAME",msg->UserName);

	
	if(CheckUserHive(szSid)) {
		//NotifyLog("Hive already loaded ");
		(*it).second->HiveLoaded=TRUE;
		(*it).second->PutEnvString("USERPROFILE",szLocalProfilePath);
		(*it).second->LoadSystemEnvironment();
		(*it).second->LoadUserEnvironment(szSid);
		goto cleanup; 
	}

	//         
	// obtain profile path         
	//
	if (!ObtainProfilePath(szSid, szLocalProfilePath,(sizeof(szLocalProfilePath)/sizeof(TCHAR)))) {	
		if(!CreateLocalProfilePath(szLocalProfilePath,UserSid,msg->UserName)) 
			goto cleanup;
		SetProfilePath(szLocalProfilePath);
		
		AddEntry=TRUE;
	}

	// Now get the path of the original profile
    MultiByteToWideChar( CP_ACP, 0, msg->UserName,strlen(msg->UserName)+1, 
		wszUserName,sizeof(wszUserName)/sizeof(wszUserName[0]) );
	
    if( NetUserGetInfo( (LPWSTR) ComputerName,(LPWSTR) &wszUserName, 3, (LPBYTE *) &ui ) )
    {
        NotifyLog("Cannot get user info: ");
        if(LoadRemote) {
			if(ComputerName) NetApiBufferFree(ComputerName);
			return FALSE;
		}
    }
	
	if(ComputerName) NetApiBufferFree(ComputerName);

	// Convert the Unicode profile path name to ANSI.
    WideCharToMultiByte( CP_ACP, 0, ui->usri3_profile, -1,szRemoteProfilePath, 256, NULL, NULL );
	
	NetApiBufferFree(ui);
	
	if (!Privilege(SE_RESTORE_NAME, TRUE)) {
		NotifyLog("Cannot activate Privilege ");
		return FALSE;
	}

	if(LoadRemote) (*it).second->PutEnvString("USERPROFILE",szRemoteProfilePath);
	strcat(szRemoteProfilePath, "\\ntuser.dat"); 

	if(LoadRemote) {
		if(!Hive(szSid,szRemoteProfilePath,TRUE)) {
			sprintf(message,"Load Hive1 %s ",szRemoteProfilePath);
			NotifyLog(message);
			Privilege(SE_RESTORE_NAME, FALSE);
			
			return FALSE;
		}
		(*it).second->HiveLoaded=TRUE;
		(*it).second->UnloadHive=TRUE;
		(*it).second->LoadSystemEnvironment();
		(*it).second->LoadUserEnvironment(szSid);
		Privilege(SE_RESTORE_NAME, FALSE);
		return TRUE;
	}

	
	LocalFile=CreateFile(szLocalProfilePath,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
							 0,OPEN_EXISTING,0,0);
	if(LocalFile==INVALID_HANDLE_VALUE){
		sprintf(message,"Open file %s ",szRemoteProfilePath);
		NotifyLog(message);
		LoadRemote=TRUE;
	}
	
	RemoteFile=CreateFile(szRemoteProfilePath,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
							 0,OPEN_EXISTING,0,0);
	if(RemoteFile==INVALID_HANDLE_VALUE){
		sprintf(message,"Open file %s ",szLocalProfilePath);
		NotifyLog(message);
		CloseHandle(LocalFile);
		if(LoadRemote) {
			Privilege(SE_RESTORE_NAME, FALSE);
			return FALSE;
		}
	}

	if(LoadRemote) {
		path=szRemoteProfilePath;
		if(!Hive(szSid,szRemoteProfilePath,TRUE)) {
			sprintf(message,"LoadHive2 %s ",szRemoteProfilePath);
			NotifyLog(message);
			Privilege(SE_RESTORE_NAME, FALSE);
			return FALSE;
		}
		(*it).second->HiveLoaded=TRUE;
		(*it).second->UnloadHive=TRUE;
		(*it).second->LoadSystemEnvironment();
		(*it).second->LoadUserEnvironment(szSid);
		*strstr(szRemoteProfilePath,"\\ntuser.dat")=0;

		(*it).second->PutEnvString("USERPROFILE",szRemoteProfilePath);
		Privilege(SE_RESTORE_NAME, FALSE);
		return TRUE;
	}

	GetFileTime(LocalFile,0,0,&LastLocalWriteTime);
	GetFileTime(RemoteFile,0,0,&LastRemoteWriteTime);

	CloseHandle(RemoteFile);
	CloseHandle(LocalFile);

	if(CompareFileTime(&LastLocalWriteTime,&LastRemoteWriteTime)<0)
		LoadRemote=TRUE;
	
	if(LoadRemote) {
		if(!Hive(szSid,szRemoteProfilePath,TRUE)) {
			sprintf(message,"LoadHive3 %s ",szRemoteProfilePath);
			NotifyLog(message);
			Privilege(SE_RESTORE_NAME, FALSE);
			return FALSE;
		}
		*strstr(szRemoteProfilePath,"\\ntuser.dat")=0;
		(*it).second->PutEnvString("USERPROFILE",szRemoteProfilePath);
	} else {
		
		if(!Hive(szSid,szLocalProfilePath,TRUE)) {
			sprintf(message,"LoadHive3 %s ",szLocalProfilePath);
			NotifyLog(message);
			Privilege(SE_RESTORE_NAME, FALSE);
			return FALSE;
		}
		*strstr(szLocalProfilePath,"\\ntuser.dat")=0;
		(*it).second->PutEnvString("USERPROFILE",szLocalProfilePath);
	}

	if (!Privilege(SE_RESTORE_NAME, FALSE)) 
		NotifyLog("Cannot deactivate Privilege");

	(*it).second->UnloadHive=TRUE;
	(*it).second->HiveLoaded=TRUE;	
	(*it).second->LoadSystemEnvironment();
	(*it).second->LoadUserEnvironment(szSid);

	res=TRUE;
cleanup:
	if(UserSid) LocalFree(UserSid);
	return res;

}
 */