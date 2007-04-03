//
//	CreateToken.c NT/2000/XP only
//
//	Create a primary access token for a given user-name
//	(no password required, but needs SYSTEM access)
//
//	To demonstrate: at command-prompt type:
//
// 		at <hh:mm> /INTERACTIVE yourprog.exe
//
//	To start yourprog.exe as system..
//
//	based entirely on the "Gary Nebbett" sample, just
//	did some cleanup to make it look pretty.
//
//	www.catch22.net	
//
#include <windows.h>
#include "ntdll.h"
#include "CreateToken.h"
#include <stdio.h>
#include "helpers.h"


#ifdef __cplusplus
extern "C" {
#endif
	
UINT (WINAPI * ZwCreateToken)(
			  PHANDLE				TokenHandle,
			  ACCESS_MASK			DesiredAccess,
			  POBJECT_ATTRIBUTES	ObjectAttributes,
			  TOKEN_TYPE			Type,
			  PLUID					AuthenticationId,
			  PLARGE_INTEGER		ExpirationTime,
			  PTOKEN_USER			User,
			  PTOKEN_GROUPS			Groups,
			  PTOKEN_PRIVILEGES		Privileges,
			  PTOKEN_OWNER			Owner,
			  PTOKEN_PRIMARY_GROUP	PrimaryGroup,
			  PTOKEN_DEFAULT_DACL	DefaultDacl,
			  PTOKEN_SOURCE			Source
			  );

UINT (WINAPI * RtlNtStatusToDosError)(UINT dwError);

//
//	Enable/Disable privilege with specified name (for current process)
//
BOOL EnablePrivilege(TCHAR *szPrivName, BOOL fEnable)
{
	TOKEN_PRIVILEGES tp;
	LUID	luid;
	HANDLE	hToken;

	if(!LookupPrivilegeValue(NULL, szPrivName, &luid))
		return FALSE;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;
	
	tp.PrivilegeCount			= 1;
	tp.Privileges[0].Luid		= luid;
	tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
	
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

	CloseHandle(hToken);

	return (GetLastError() == ERROR_SUCCESS);
}

//
//	Return information for the specified TOKEN
//
PVOID GetTokenInfo(HANDLE hToken, TOKEN_INFORMATION_CLASS tic, PVOID *pTokenOut)
{
	DWORD n;
	PBYTE p;

	if(pTokenOut == 0)
		return 0;

	if(!GetTokenInformation(hToken, tic, 0, 0, &n) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
		return 0;

	if((p = malloc(n)) == 0)
		return 0;

	if(GetTokenInformation(hToken, tic, p, n, &n))
	{
		*pTokenOut = p;
		return p;
	}
	else
	{
		free(p);
		*pTokenOut = 0;
		return 0;
	}
}

//
// Return the SID of specified user account
//
PSID GetUserSid(LPCTSTR szUserName)
{
	SID		*sid = 0;
	TCHAR	*dom = 0;

	DWORD	sidlen = 0;
	DWORD	domlen = 0;

	SID_NAME_USE snu;

	//
	// with no machine specified, LookupAccountName looks up user's sid in
	// following locations:
	//
	// well-known, built-in, local-machine, primary-domain, trusted-domain
	//
	while(!LookupAccountName(NULL, szUserName, sid, &sidlen, dom, &domlen, &snu))
	{
		if(sid) free(sid);
		if(dom) free(dom);

		if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			return 0;

		sid = malloc(sidlen);
		dom = malloc(domlen);
	}

	printf("domain: %s\n", dom);

	free(dom);
	return sid;
}

//
//	Create a primary access token for specified user account
//
HANDLE CreateToken(LPCTSTR szUserName)
{
	SID_IDENTIFIER_AUTHORITY	nt   = SECURITY_NT_AUTHORITY;
	SECURITY_QUALITY_OF_SERVICE	sqos = { sizeof(sqos), SecurityAnonymous, SECURITY_STATIC_TRACKING, FALSE };

	HANDLE				hToken;
	PSID				sid;
	TOKEN_USER			user;

	LUID				authid	= SYSTEM_LUID;
	OBJECT_ATTRIBUTES	oa		= { sizeof(oa), 0, 0, 0, 0, &sqos };
	TOKEN_SOURCE		source	= {{'*', '*', 'A', 'N', 'O', 'N', '*', '*'}, {0, 0}};
	HANDLE				hToken2 = 0;
	PTOKEN_STATISTICS	stats;

	PVOID				tokarr[5];
	int					i;
	DWORD				status;

	// Get address of Nt/ZwCreateToken from NTDLL.DLL
	ZwCreateToken         = (PVOID)GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwCreateToken");
	RtlNtStatusToDosError = (PVOID)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlNtStatusToDosError");

	if(ZwCreateToken == 0 || RtlNtStatusToDosError == 0)
		return 0;

	// Must have SeCreateToken privilege
	if(!EnablePrivilege(SE_CREATE_TOKEN_NAME, TRUE)){
		DBG("EnablePrivilege failed\n");
	}
	

	// Use an existing process token as our basic for a new token
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken))
		return 0;
	
	// Convert username to a SID
	if((sid = GetUserSid(szUserName)) == 0)
	{
		CloseHandle(hToken);
		return 0;
	}

	user.User.Attributes	= 0;
	user.User.Sid			= sid;

	if(!AllocateLocallyUniqueId(&source.SourceIdentifier))
	{
		free(sid);
		CloseHandle(hToken);
		return 0;
	}

	if(!GetTokenInfo(hToken, TokenStatistics, &stats))
	{
		free(sid);
		CloseHandle(hToken);
		return 0;
	}
	

	//
	//	Undocumented ZwCreateToken service: will not work for us
	//  under WIN2003, will need to do this from WINLOGON process in future?
	//
	status = ZwCreateToken(&hToken2, TOKEN_ALL_ACCESS, &oa, TokenPrimary,
		(PLUID)&authid, 
		(PLARGE_INTEGER)&stats->ExpirationTime,
		&user,
		(PTOKEN_GROUPS)			GetTokenInfo(hToken, TokenGroups,		&tokarr[0]),
		(PTOKEN_PRIVILEGES)		GetTokenInfo(hToken, TokenPrivileges,	&tokarr[1]),
		(PTOKEN_OWNER)			GetTokenInfo(hToken, TokenOwner,		&tokarr[2]),
		(PTOKEN_PRIMARY_GROUP)	GetTokenInfo(hToken, TokenPrimaryGroup, &tokarr[3]),
		(PTOKEN_DEFAULT_DACL)	GetTokenInfo(hToken, TokenDefaultDacl,	&tokarr[4]),
		&source);

	for(i = 0; i < 5; i++)
		free(tokarr[i]);

	free(stats);
	free(sid);

	CloseHandle(hToken);

	SetLastError(RtlNtStatusToDosError(status));

	return hToken2;
}

#ifdef __cplusplus
}
#endif

