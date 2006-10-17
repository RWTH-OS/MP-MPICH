//---------------------------------------------------------------------------
#ifndef RexHelpH
#define RexHelpH

#include <windows.h>

 BOOL IsValidUser( char * lpszUsername,    // string that specifies the user name
 				   char * lpszDomain,      // string that specifies the domain or server
  				   char * lpszPassword);   // string that specifies the password

 BOOL IsValidAccount (char *AccountName);
//---------------------------------------------------------------------------
#endif
