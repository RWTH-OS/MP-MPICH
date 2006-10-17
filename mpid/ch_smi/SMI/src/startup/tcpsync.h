/* $Id$ */

#ifndef __SMI_TCPSYNC_H__
#define __SMI_TCPSYNC_H__

#ifndef WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#include <windows.h>
#endif
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef WIN32
#define SOCKTYPE SOCKET
#else
#define SOCKTYPE int
#endif

/* how many ports to try to connect to (incrementing the base port nbr)*/
#define TCP_MAX_PORTTRY 5
/* how many times to try to connect to each port */
#define TCP_MAX_CONNTRY 200
/* how many ms to wait between each connect try */
#define TCP_RETRY_DELAY 100

#define TCP_LEN_EXENAME  192
#define TCP_LEN_USERNAME  60
#define TCP_LEN_HOSTNAME 256

typedef struct tcp_smp_info_t_ {
    int iLocalProcRank;
    int iLocalProcSize;
    int iLocalMemKey;
    int iNodeSize;
    int iNodeRank;
} tcp_smp_info_t;

typedef struct tcp_node_info_t_ {
    char szLocalHost[TCP_LEN_HOSTNAME];
    int iNbrProcs;
    int iLowRank;
    int iMemKey;
} tcp_node_info_t;

typedef struct tcp_ident_t_ {
    int iMagicNumber;
    char szExecName[TCP_LEN_EXENAME];
    char szUserName[TCP_LEN_USERNAME];
    char szLocalHost[TCP_LEN_HOSTNAME];  
    int iProcRank;
    tcp_smp_info_t SmpInfo;
} tcp_ident_t;

typedef enum {
    ts_ok,
    ts_wrongident,
    ts_connrefused,
    ts_connreset
} tcp_status_t;

SOCKTYPE _smi_tcp_start_server(int baseport, struct sockaddr_in* pName, int iClients);

SOCKTYPE _smi_tcp_connect_client(char* hostname, int baseport, tcp_ident_t* idLocal);

int _smi_tcp_bcast_srv(tcp_ident_t* idLocal, int iSender, 
		       char* data, size_t size, 
		       int iClients, int baseport);

int _smi_tcp_bcast_cl(tcp_ident_t* idLocal, int iSender,
		      char* data, size_t size, char* hostname, int baseport);

int _smi_tcp_allgather_srv(tcp_ident_t * idLocal, char* pDataIn, char* pDataOut,
			   size_t size, int iNumProcs, int baseport);

int _smi_tcp_allgather_cl(tcp_ident_t * idLocal, char* pDataIn, char* pDataOut,
			  size_t size, int iNumProcs, char *hostname, int baseport);

int _smi_tcp_mkident(tcp_ident_t* Ident, int iMyRank, int iMagicNumber, char*
		     szExecName, int iMemKey);

int _smi_TcpBroadcast(char* szHostName, int iPortNumber, void* pData, size_t
		      stLen, int iSender, int iNumProcs, tcp_ident_t* idLocal);

int  _smi_TcpAllgather(char *szHostName, int iPortNumber, void *pDataIn, void *pDataOut, size_t stLen,
		       int iNumProcs, tcp_ident_t* idLocal);

int _smi_tcp_init(void);

int _smi_tcp_finalize(void);

#endif /* __SMI_TCPSYNC_H__ */
