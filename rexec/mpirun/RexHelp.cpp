//---------------------------------------------------------------------------

#pragma hdrstop

#include "RexHelp.h"
#include "RexecClient.h"

#include <lmaccess.h>

BOOL IsValidUser( char * lpszUsername,
  					char * lpszDomain,
  					char * lpszPassword)

{
    BOOL returnvalue;
    DWORD dwLogonType;
    DWORD dwLogonProvider;
    HANDLE hToken;

    dwLogonType= LOGON32_LOGON_NETWORK;
	//dwLogonType= LOGON32_LOGON_INTERACTIVE;
    dwLogonProvider = LOGON32_PROVIDER_DEFAULT;

    returnvalue = LogonUser(lpszUsername,lpszDomain,lpszPassword,
  							dwLogonType,dwLogonProvider,&hToken);
    if (returnvalue)
    {
    	CloseHandle(hToken);
    }
    else
    {
    	DWORD  errcode = GetLastError();
        SetLastError(errcode);
    }
    return(returnvalue);
}

BOOL IsValidUser2( char * lpszUsername,
  					char * lpszDomain,
  					char * lpszPassword)

{
    //BOOL returnvalue;
    //DWORD dwLogonType;
    //DWORD dwLogonProvider;
    //HANDLE hToken;
	DWORD status;
   
    LPWSTR domainname = NULL;  
    LPWSTR username = NULL;    
    LPWSTR oldpassword = NULL; 

    status = NetUserChangePassword(domainname,username,oldpassword,oldpassword);
    if (0 == status)
    {
		return true;
    }
    else
    {
    	DWORD  errcode = GetLastError();
        SetLastError(errcode);
    }
    return(false);
}



BOOL IsValidAccount (char *AccountName)
{
    char *UserName = NULL,*Domain = NULL,*Password = NULL;
    int maxlen = strlen(AccountName);
    UserName = (char *) calloc (maxlen, sizeof (char));
    Domain = (char *) calloc (maxlen, sizeof (char));
    Password = (char *) calloc (50, sizeof (char));
    BOOL returnvalue;
    returnvalue = LoadAccount(AccountName,UserName,Domain,Password);
    returnvalue = IsValidUser(UserName,Domain,Password);
    free(Password);
    free(Domain);
    free(UserName);
    return returnvalue;
}

//---------------------------------------------------------------------------
