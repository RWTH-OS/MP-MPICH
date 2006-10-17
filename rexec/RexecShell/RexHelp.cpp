//---------------------------------------------------------------------------

#pragma hdrstop

#include "RexHelp.h"
#include "RexecClient.h"
#include "vcl/clipbrd.hpp"

BOOL CheckUserAccountOnChange = true;



BOOL IsValidUser( char * lpszUsername,
  					char * lpszDomain,
  					char * lpszPassword)

{
    BOOL returnvalue;
    DWORD dwLogonType;
    DWORD dwLogonProvider;
    HANDLE hToken;

    dwLogonType= LOGON32_LOGON_NETWORK;
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

char * GetClipboardEntry()
{
    int BufSize = 0;
    int TextHandle = 0;
    char *pText;
    char *EntryBuf = 0;

    Clipboard()->Open();
    try
    {
       TextHandle = Clipboard()->GetAsHandle(CF_TEXT);
       pText = (char *)GlobalLock((HGLOBAL)TextHandle);
       if (pText != NULL)
       {
          BufSize = strlen(pText)+1;
          EntryBuf = (char*) calloc(BufSize,sizeof(char));
          strcpy(EntryBuf,pText);
          GlobalUnlock((HGLOBAL)TextHandle);
       }
    }
    catch (...)
    {
     Clipboard()->Close();
     return NULL;
     //throw;
    }
    Clipboard()->Close();
    return EntryBuf;
     /*int RetVal = 0;
     TClipboard * actClipboard = Clipboard();
     int BufSize = 0;//actClipboard->GetTextLen();
     ClipboardEntry = NULL;
     if (!actClipboard->HasFormat(CF_TEXT))
       return  RetVal;
     actClipboard->Open();
     actClipboard->GetComponent
     RetVal = actClipboard->GetTextBuf(ClipboardEntry,BufSize);
     actClipboard->Close();
     return RetVal;  */
}

BOOL GetFullExeName(LPSTR ExeName)
{
    char PathName[1024];
         
    BOOL returnvalue = FALSE;
    DWORD numcopied;
    numcopied = GetModuleFileName(NULL,PathName,1024);
    if (numcopied >0)
    {
        strcpy(ExeName,PathName);
        returnvalue = TRUE;
    }
    return(returnvalue);

}

//////////////////////////////////////////////////////////////////////
void ChangeFileExt(LPSTR FileName, LPCSTR extension)
{
    char *path,*drive,*dir,*fname,*ext;
    DWORD maxlength;
    maxlength = strlen(FileName)+strlen(extension)+1;

    path = (char *) calloc (maxlength,1);
    drive = (char *) calloc (maxlength,1);
    dir = (char *) calloc (maxlength,1);
    fname = (char *) calloc (maxlength,1);
    ext = (char *) calloc (maxlength,1);

    //delete leading and ending " if present
    if (FileName[0] == '"')
        strcpy(path,FileName+1);
    else
        strcpy(path,FileName);
    char * charptr = NULL;
    charptr = strchr(path,'"');
    if (charptr != NULL)
        charptr[0]='\0';

    _splitpath(path,drive,dir,fname,ext);
    strcpy(ext,extension);
    _makepath(path,drive,dir,fname,ext);
    strcpy(FileName,path);

    free(ext);
    free(fname);
    free(dir);
    free(drive);
    free(path);
}

//---------------------------------------------------------------------------
