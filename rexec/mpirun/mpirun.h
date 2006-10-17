#ifndef __mpirun_h_
#define __mpirun_h_


#define NOTE(m) if(verbose) std::cerr<<m<<std::endl
#define DBM(m) if(debug_flag) std::cerr<<m<<std::endl
extern BOOL verbose,debug_flag;
extern HANDLE *RemoteProcs;

DWORD ReadHosts(char *file,int np,HostData **Hosts, int *listLen,
		HostData *global,PlgDesc *actPlugin);
LPTSTR GetLastErrorText( DWORD error,LPTSTR lpszBuf, DWORD dwSize );
DWORD ScanNetwork(int np, HostData **Hosts, int *listLen,HostData *global, PlgDesc *actPlugin);
BOOL LoadConfig(const char *File,HostData ***Hosts,int *np,HostData *global,char **Plugin);
//SI//23-08-2004 added flag DoPluginCheck, no plugin needed for reboot and shutdown
DWORD GetNodesFromList(char **Nodes,int NumProcs,HostData **Servers,int* Size,HostData *global,PlgDesc *actPlugin,bool DoPluginCheck = true);
BOOL CreateHost(const char *name,HostData **Hosts,int *np,HostData *global,PlgDesc *actPlugin,bool DoPluginCheck = true);
#endif