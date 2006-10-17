#ifndef __PLUGIN_H__
#define __PLUGIN_H__

// Das ist mein Vorschlag fuer ein Plugin Interface zum
// Frontend. Das ganze wird in etwa so funktionieren:

// Beim Laden der DLL wird die Funktion _PlgDescription
// vom Typ InitFunc aufgerufen. Diese fuellt alle Felder des uebergebenen
// structs PlgDescr aus. (Funktionen, die nicht benoetigt werden,
// bitte mit NULL belegen.)
// Waehlt der Benutzer in der listbox im ConfigDialog
// Diese DLL aus, so wird die Funktion Attach aufgerufen.
// Diese kann wenn benoetigt eigene controls in den Dialog
// einfuegen, sofern noetig (Die IDS teile ich noch mit.)
// Trifft eine message fuer diese controls ein, so wird sie an
// die DLGCallback funktion aufgerufen.
// Ansonsten werden folgende Funktionen aufgerufen:

// Detach wenn der User ein anderes Plugin waehlt.
// In dieser Funktion bitte alle Controls entfernen.

// NewData, wenn beim Refresh die Daten eines Knotens ankommen.

// SelectedChange, wenn der User einen neuen Knoten in das
// Auswahlfeld uebernimmt, bzw. einen loescht.

// DlgClose, wenn der OK Button betaetigt wird.
// Hier kan z.B. die Liste der gewaehlten Knoten noch 
// Veraendert (z.B. umsortiert) werden.

// RefreshEnd, wenn der Refresh-Vorgang abgeschlossen ist.
// Hier kann die Liste der gewaehlten Knoten ebenfalls veraendert werden.

// Convert wird aufgerufen, um aus den UserDaten eine string zu erzeugen,.
// Das funktioniert genau wie _QueryUserData.

#ifdef __cplusplus
extern "C" {
#endif

#define HPART(IP)((IP)>>24)
#define CNET(IP) (((IP)&0x00FF0000L) >> 16)
#define BNET(IP) (((IP)&0x0000FF00L) >> 8)
#define ANET(IP)  ((IP)&0x000000FFL)


typedef struct {
    BOOL Global;
    char User[50];
    char Domain[50];
    char Password[128];
} TAccount;

#ifndef __cluma_h__
typedef /* [public][public] */ struct __MIDL_cluma_0001
    {
    /* [ref][string] */ unsigned char ServerString[ 100 ];
    struct 
        {
        unsigned long dwMajorVersion;
        unsigned long dwMinorVersion;
        unsigned long dwBuildNumber;
        unsigned long dwPlatformId;
        unsigned char szCSDVersion[ 128 ];
        }	OS;
    struct 
        {
        unsigned long dwNumberOfProcessors;
        unsigned long dwActiveProcessorMask;
        unsigned short wProcessorArchitecture;
        unsigned short wProcessorLevel;
        unsigned short wProcessorRevision;
        unsigned long Mhz;
        unsigned long dwTotalPhysMem;
        }	HW;
    struct 
        {
        /* [ref][string] */ unsigned char Hostname_ip[ 255 ];
        unsigned long NumEntries;
        /* [length_is] */ unsigned long IPS[ 100 ];
        /* [length_is] */ unsigned long Speeds[ 100 ];
        }	IP;
    }	R_MACHINE_INFO;

#endif

typedef struct {
	// These are used by the config dialog
    char *Executable;
    char *WorkingDir;
    char *UserOptions;
    char *PluginOptions; //si 02/05 add plugin options manually
    BOOL LockIt;
    BOOL LoadProfile;
    DWORD PriorityClass;  //si new

    // This is used by the plugin
    char *Commandline;
    void *Context;
    DWORD ContextSize;

    // This is used by both the config dialog and the plugin
	char *Environment;


    DWORD ExeSize;
    DWORD WDSize;
    DWORD OptSize;
    DWORD PluginOptSize;

    DWORD CmdSize;
    DWORD EnvSize;
    DWORD EnvAllocSize;
} TProcessData;

typedef struct {
    DWORD RefCount;
    char ConsoleUser[256];
    char LockedBy[256];
    BOOL Locked;
    DWORD UserDataSize;
    void *UserData;
    BOOL Valid;
    R_MACHINE_INFO *Configuration;
    DWORD NumProcs;
} TStateData;

typedef struct {
    char Name[128];
    void *handle;
    DWORD LastError;
    unsigned long Security;
    TProcessData *ProcData;
    BOOL Alive;
    TAccount *Account;
    TStateData *State;
    char RPCProtocol[30];
} HostData;


typedef void (WINAPI *InitFunc)(struct _PlgDesc*); 
// Eintragen aller benoetigten Funktionen und Daten in de uebergebenen struct.
// Nicht benoetig Felder bitte mit 0 belegen.

typedef void (WINAPI *AttachFunc)(HWND); 
// AttachFunc(HWND Application, HWND Dialog);
// Hier kann in den Dialog irgendetwas eingefuegt werden, falls gewuenscht.
// Mit Hilfe des Applikationshandles koennen beliebige Fenster erzeugt werden.
// (Z.B. Messaageboxes)

typedef void (WINAPI *DetachFunc)(void);
// Wird aufgerufen, wenn das Plugin abgewahlt wird.
// Dient zum Freigeben von ressourcen. Insbesondere dem Loeschen
// von eigenen Controls im Dialog.


typedef BOOL (WINAPI *NewDataFunc)(HostData*);
// Hier kann der HostDataStruct beliebig veraendert werden.
// Der Rueckgabewert gibt an, ob der Knoten gueltig ist
// (TRUE==Gueltig, FALSE==Ungueltig)

typedef BOOL (WINAPI *SelectedChangeFunc)(HostData**,DWORD,HostData**,DWORD*,HostData*);
// Der erste Parameter ist die Liste aller Knoten, der Zweite dessen Laenge.
// Der dritte enthaelt die Liste der ausgewahlten Knoten und des letzte dessen Laenge.
// Wird aufgerufen, wenn sich die Liste der ausgewahlten Knoten aendert.
// Hier kann dann z.B. die Kommandozeile eingetragen werden.



typedef BOOL (WINAPI *ConvertFunc)(TStateData*,char *,DWORD *);
// Der erste Parameter enthaelt einen Pointer auf einen Array von stateEntries.
// Hier sind wie erlautert auch die UserDaten enthalten. Diese werden in einen 
// string umgewandelt, der im zweiten Parameter uebergeben wird.
// Der dritte Parameter enthealt die Groesse des strings. Solte diese zu klein sein,
// bitte FALSE zurueckgeben und die Groesse anpassen.

typedef void (WINAPI *ConfigureFunc)(HWND,HWND,HostData*,BOOL);

typedef void (WINAPI *ParseFunc)(int *argc, char** argv, HostData *);

#define VersionPlugin_h 11

typedef struct _PlgDesc {
	int PluginVersionNumber; //Si: added to check if appropriate version of plugin.h is used
                                 // is set to VersionPlugin_h in Plugins
	char VisualName[30];	// Name to display in listbox
	char DllName[MAX_PATH]; // Optional name of dll to call when querying the nodes
	HINSTANCE hLib;			// Reserved do not touch it!
	InitFunc Init;			// Reserved do not touch it!
	AttachFunc Attach;		// Called when user selects this plugin
	DetachFunc Detach;		// Called when user changes to a different plugin
	NewDataFunc NewData;	// Called for every node that has been queried
	SelectedChangeFunc SelectedChange; // Called when the user selects a node or deletes one
	SelectedChangeFunc DlgClose; // Called when the user presses OK
	SelectedChangeFunc RefreshEnd; // Called when refresh is finished.
	ConvertFunc Convert;	// Called if a textual representation is needed for
							// user data.
	ConfigureFunc Configure;
	ParseFunc ParseCommandline;
} PlgDesc;

typedef struct {
	void  (*PutString)(HostData *Proc,char *Name,char *Value);
	char* (*GetString)(HostData *Proc,char *Name,char *value,DWORD *BufSize);
	void  (*SetCommandline)(HostData *Proc,char *commandline);
    void* (*GetContext)(HostData *Host,DWORD size);
} EnvFuncs;

typedef struct
    {
    struct
        {
        unsigned long dwMajorVersion;
        unsigned long dwMinorVersion;
        unsigned long dwBuildNumber;
        unsigned long dwPlatformId;
        unsigned char szCSDVersion[ 128 ];
        }	OS;
    struct
        {
        unsigned long dwNumberOfProcessors;
        unsigned long dwActiveProcessorMask;
        unsigned short wProcessorArchitecture;
        unsigned short wProcessorLevel;
        unsigned short wProcessorRevision;
        unsigned long Mhz;
        unsigned long dwTotalPhysMem;
        }	HW;
    struct
        {
        unsigned char Hostname_ip[ 255 ];
        unsigned long NumEntries;
        unsigned long IPS[ 100 ];
        }	IP;
    }	MachineInfo;
#ifdef __cplusplus
}
#endif

#endif
