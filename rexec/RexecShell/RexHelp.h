//---------------------------------------------------------------------------
#ifndef RexHelpH
#define RexHelpH

#include <windows.h>
 extern BOOL CheckUserAccountOnChange;

 BOOL IsValidUser( char * lpszUsername,    // string that specifies the user name
 				   char * lpszDomain,      // string that specifies the domain or server
  				   char * lpszPassword);   // string that specifies the password

 BOOL IsValidAccount (char *AccountName);

 char * GetClipboardEntry();

 BOOL GetFullExeName(LPSTR ExeName);
 void ChangeFileExt(LPSTR FileName, LPCSTR extension);

//---------------------------------------------------------------------------
#endif
