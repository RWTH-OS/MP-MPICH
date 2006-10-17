#ifndef __ENVIRONMENT
#define __ENVIRONMENT


#include "Userenv.h"

//
// Flags that can be set in the dwFlags field
//

#define PI_NOUI         0x00000001      // Prevents displaying of messages
#define PI_APPLYPOLICY  0x00000002      // Apply NT4 style policy

typedef struct _PROFILEINFOA {
    DWORD       dwSize;                 // Set to sizeof(PROFILEINFO) before calling
    DWORD       dwFlags;                // See flags above
    LPSTR       lpUserName;             // User name (required)
    LPSTR       lpProfilePath;          // Roaming profile path (optional, can be NULL)
    LPSTR       lpDefaultPath;          // Default user profile path (optional, can be NULL)
    LPSTR       lpServerName;           // Validating domain controller name in netbios format (optional, can be NULL but group NT4 style policy won't be applied)
    LPSTR       lpPolicyPath;           // Path to the NT4 style policy file (optional, can be NULL)
    HANDLE      hProfile;               // Filled in by the function.  Registry key handle open to the root.
} PROFILEINFOA, FAR * LPPROFILEINFOA;
typedef struct _PROFILEINFOW {
    DWORD       dwSize;                 // Set to sizeof(PROFILEINFO) before calling
    DWORD       dwFlags;                // See flags above
    LPWSTR      lpUserName;             // User name (required)
    LPWSTR      lpProfilePath;          // Roaming profile path (optional, can be NULL)
    LPWSTR      lpDefaultPath;          // Default user profile path (optional, can be NULL)
    LPWSTR      lpServerName;           // Validating domain controller name in netbios format (optional, can be NULL but group NT4 style policy won't be applied)
    LPWSTR      lpPolicyPath;           // Path to the NT4 style policy file (optional, can be NULL)
    HANDLE      hProfile;               // Filled in by the function.  Registry key handle open to the root.
} PROFILEINFOW, FAR * LPPROFILEINFOW;
#ifdef UNICODE
typedef PROFILEINFOW PROFILEINFO;
typedef LPPROFILEINFOW LPPROFILEINFO;
#else
typedef PROFILEINFOA PROFILEINFO;
typedef LPPROFILEINFOA LPPROFILEINFO;
#endif // UNICODE





typedef BOOL (WINAPI *LogoutFunc)(HANDLE,HANDLE);
typedef BOOL (WINAPI *LoginFunc)(HANDLE,void *);
typedef BOOL (WINAPI *CreateEnvFunc)(OUT LPVOID *lpEnvironment,IN HANDLE hToken,IN BOOL bInherit);
typedef BOOL (WINAPI *DestroyEnvFunc)(IN LPVOID  lpEnvironment);



extern LoginFunc LoadUserProfile;
extern LogoutFunc UnloadUserProfile;
extern CreateEnvFunc CreateEnvironmentBlock;
extern DestroyEnvFunc DestroyEnvironmentBlock;

HANDLE MyLoadUserProfile(HANDLE User,char *Username,char *Domainname, UserInfo &UI);
BOOL InitEnvironment();
BOOL FreeEnvironment();


#endif