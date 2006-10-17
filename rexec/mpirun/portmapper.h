
#ifndef _PORTMAPPER_H_
#define _PORTMAPPER_H_


#define MSG_BASE 3455
#define MSG_TAG_QUERY_SIZE	MSG_BASE  // Sent by a client to get the number 
									  // of services with the given name are registered.
#define MSG_TAG_QUERY		(MSG_BASE+1)
#define MSG_TAG_DATA		(MSG_BASE+2) // Sent by a service to register its port.
#define MSG_TAG_DELETE		(MSG_BASE+3) // Sent by a service to delete its port.
#define MSG_TAG_LOCK_SCREEN (MSG_BASE+4)
#define MSG_TAG_UNLOCK_SCREEN (MSG_BASE+5)
#define MSG_TAG_HELO		(MSG_BASE+6)

#define PORTMAPPER_PORT	2342 // If there is no entry in the services, we use this port.

#define MAX_USER_NAME_LEN 256
#define MAX_APP_NAME_LEN 256
#define SERVICES_ENTRY_NAME "ntrexec_lock"
#define EMPTY		0
#define APPLICATION 1
#define RESERVED	2
#define LOGIN		3
#define ERROR_RES	4

#ifdef __cplusplus
extern "C" {
#endif


// Used for QUERY_SIZE, DELETE and DATA

struct stateEntry {
	char userName[MAX_USER_NAME_LEN];
	char appName[MAX_APP_NAME_LEN];
	unsigned long accessType;
    unsigned long userDataSize;
    char buffer[4];
	stateEntry* next();

};
	
struct simpleMsg {
	unsigned long tag;
	unsigned long sequNumber;
	unsigned long querySize;
	struct stateEntry data;
};



#ifdef __cplusplus
}
#endif

/*
BOOL ObtainProfilePath(LPTSTR pszSid, LPTSTR pszProfilePath,DWORD dwPathSize);
BOOL Privilege(LPTSTR pszPrivilege, BOOL bEnable);
BOOL Hive(LPTSTR pszSid, LPTSTR pszProfilePath, BOOL bLoad);
BOOL CheckUserHive(LPTSTR pszSid);
*/


/*
BOOL ObtainSid(
        HANDLE hToken,           // handle to an process access token
        PSID   *psid             // ptr to the buffer of the logon sid        
		);

void RemoveSid(
        PSID *psid               // ptr to the buffer of the logon sid        
	);

BOOL AddTheAceWindowStation(
        HWINSTA hwinsta,         // handle to a windowstation
        PSID    psid             // logon sid of the process        
	);
*/
char *NotifyLog(char * msg);
BOOL ObtainSidString(PSID pSid, LPTSTR pszSid, LPDWORD pdwBufferLen);
BOOL ObtainSid(HANDLE hToken, PSID *ppSid);
void GetLoginStatus(stateEntry *buffer);


typedef BOOL (_stdcall *LogoutFunc)(HANDLE,HANDLE);
typedef BOOL (_stdcall *LoginFunc)(HANDLE,void *);
typedef BOOL (WINAPI *UserDataFunc)(void*,DWORD*);

extern LoginFunc LoadUserProfile;
extern LogoutFunc UnloadUserProfile;
extern stateEntry LockState;

struct ProfileInfo {
	DWORD dwInternal;
	DWORD dwFlags;
	char *lpUserName;
	char *lpCentralPath;
	char *lpDefaultPath;
	char *lpServerName;
	char *lpPolicyPath;
	HANDLE hProfile;
	DWORD Padding[32]; // To be shure we have valid memory
};



#endif
