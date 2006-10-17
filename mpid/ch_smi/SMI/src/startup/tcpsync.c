/* $Id$ */

#include "env/general_definitions.h"
#include "env/smidebug.h"
#include "tcpsync.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef WIN32
#define close(s) closesocket(s)
#endif

static tcp_node_info_t* tcp_create_node_list(int iNbrNodes)
{
    tcp_node_info_t* RetVal;
    
    RetVal = (tcp_node_info_t*) malloc (
	sizeof(tcp_node_info_t) * (iNbrNodes+2)
	);   
    ASSERT_A( (RetVal != NULL), "no memory for opration", SMI_ERR_NOMEM);	
    memset(RetVal, 0, sizeof(tcp_node_info_t) * (iNbrNodes+2)); 
  
    return(RetVal);
}
static void tcp_remove_node_list(tcp_node_info_t* pNodeInfo)
{
    free(pNodeInfo);
}

static void tcp_node_list_add(tcp_node_info_t* pNodeInfo, tcp_ident_t* pIdent)
{
    REMDSECTION("tcp_node_list_add");
    int i,j;

    DSECTENTRYPOINT;
    
    j = -1;

    for(i=0; pNodeInfo[i].szLocalHost[0] != '\0'; i++) {
	DNOTICEI("Host", i);
	DNOTICES("Name", pNodeInfo[i].szLocalHost);
	if (strcmp(pIdent->szLocalHost, pNodeInfo[i].szLocalHost) == 0)
	    j=i;
    }

    /* New host ? */
   DNOTICE("New host ?");
    if ( j == -1) { /* Yes */
	/* Make a new entry */
	DNOTICE("Yes, make a new entry");
	strcpy(pNodeInfo[i].szLocalHost, pIdent->szLocalHost);
	pNodeInfo[i].iNbrProcs = 1;
	pNodeInfo[i].iLowRank = pIdent->iProcRank;
	pNodeInfo[i].iMemKey = pIdent->SmpInfo.iLocalMemKey;
	
    } else { /* No */
	/* Adjust statistics */
	DNOTICE("No, adjust statistics");
	pNodeInfo[j].iNbrProcs++;
	if (pIdent->iProcRank <	pNodeInfo[j].iLowRank) {
	    pNodeInfo[j].iLowRank = pIdent->iProcRank;
	    pNodeInfo[j].iMemKey = pIdent->SmpInfo.iLocalMemKey;
	}
    }

    DSECTLEAVE;
}

static void tcp_mk_smp_info(tcp_node_info_t* pNodeInfo, tcp_ident_t* pIdent)
{
    DSECTION("tcp_mk_smp_info");
    int i,j;

    DSECTENTRYPOINT;
    
    j = -1;

    for(i=0; pNodeInfo[i].szLocalHost[0] != '\0'; i++) {
	DNOTICEI("Host", i);
	DNOTICES("Name", pNodeInfo[i].szLocalHost);
	if (strcmp(pIdent->szLocalHost, pNodeInfo[i].szLocalHost) == 0)
	    j=i;
    }

    ASSERT_A((j != -1),"No such host in list", SMI_ERR_PARAM);
    
    pIdent->SmpInfo.iLocalProcSize = pNodeInfo[j].iNbrProcs;
    pIdent->SmpInfo.iLocalProcRank = pIdent->iProcRank - pNodeInfo[j].iLowRank;
    pIdent->SmpInfo.iLocalMemKey = pNodeInfo[j].iMemKey;
    pIdent->SmpInfo.iNodeSize = i;
    pIdent->SmpInfo.iNodeRank = j;

    DSECTLEAVE;
}

static int _smi_tcp_recv(SOCKTYPE s, char *buf, size_t len, int flags)
{
    REMDSECTION("_smi_tcp_recv");
    size_t          iReceived = 0;
    int             iAct;

    DSECTENTRYPOINT;
    
    DNOTICEI("Total Bytes:",len);
    
    while (iReceived < len) {
	iAct = recv(s, buf, (int)len, flags);
	DNOTICEI("Bytes received:", iAct);
	if (iAct < 0) {
	    DPROBLEM("Transmission was interrupted");
	    DSECTLEAVE
		return (-1);
	}
       	if (iAct == 0)
	    break;
	iReceived += iAct;
    }
    
    if (iReceived != len) {
	DPROBLEM("Transmission was interrupted");
	DSECTLEAVE
	    return (-1);
    }
    
    DSECTLEAVE; return (0);
}

static int _smi_tcp_send(SOCKTYPE s, char *buf, size_t len, int flags)
{
    REMDSECTION("_smi_tcp_send");
    int             iSent = 0;
    int             iAct;
    
    DSECTENTRYPOINT;
    
    DNOTICEI("Total Bytes:",len);
    
    while (iSent < (int)len) {
	iAct = send(s, buf, (int)len, flags);
	DNOTICEI("Bytes sent:", iAct);
	if (iAct < 0) {
	    DPROBLEM("Transmission was interrupted");
	    DSECTLEAVE; return (-1);
	}
	if (iAct == 0)
	    break;
	iSent += iAct;
    }
    
    if (iSent != len) {
	DPROBLEM("Transmission was interrupted");
	DSECTLEAVE
	    return (-1);
    }
    
    DSECTLEAVE;	return (0);
}

static int _smi_tcp_ident_valid(tcp_ident_t * idLocal, tcp_ident_t * idRemote)
{
    REMDSECTION("_smi_tcp_ident_valid");
    
    DNOTICEI("local iMagicNumber", idLocal->iMagicNumber);
    DNOTICEI("remote iMagicNumber", idRemote->iMagicNumber);
    if (idLocal->iMagicNumber != idRemote->iMagicNumber)
	return (0);
    
    DNOTICES("local szExecName", idLocal->szExecName);
    DNOTICES("remote szExecName", idRemote->szExecName);
#ifdef WIN32
    if (stricmp(idLocal->szExecName, idRemote->szExecName) != 0)
#else
	if (strcmp(idLocal->szExecName, idRemote->szExecName) != 0)
#endif
	    return (0);
    
    DNOTICES("local szUserName", idLocal->szUserName);
    DNOTICES("remote szUserName", idRemote->szUserName);
#ifdef WIN32
    if (stricmp(idLocal->szUserName, idRemote->szUserName) != 0)
#else
	if (strcmp(idLocal->szUserName, idRemote->szUserName) != 0)
#endif
	    return (0);
    
    DNOTICE("id is valid!");
    return (1);
}

SOCKTYPE _smi_tcp_start_server(int baseport, struct sockaddr_in* pName, int iClients)
{
    DSECTION("_smi_tcp_start_server");
    SOCKTYPE s;
    int i;
    int value;
    
    DSECTENTRYPOINT;
    
    for (i = 0; i < TCP_MAX_PORTTRY; i++) {
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror("socket");
	    SMI_Abort(-1);
	}
	DNOTICEI("trying to bind to port", baseport + i);
	
	value = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));
	memset(pName, 0, sizeof(struct sockaddr_in));
	pName->sin_family = AF_INET;
	pName->sin_port = htons((unsigned short)(baseport + i));
	pName->sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef WIN32
	if (bind(s, (struct sockaddr *) pName, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
	    int             wsaerr = WSAGetLastError();
	    if (wsaerr != WSAEADDRINUSE) {
		perror("bind");
		SMI_Abort(-1);
	    }
	}
#else	/* WIN32 */
	if (bind(s, (struct sockaddr *) pName, sizeof(struct sockaddr_in)) < 0) {
	    close(s);
	    if (errno == EACCES) {
		DERROR("The portadress must be above 1024");
		SMI_Abort(-1);
	    }
	    if (errno != EADDRINUSE) {
		perror("bind");
		SMI_Abort(-1);
	    }
	}
#endif	/* WIN32 */
	else {
	    DNOTICEI("ok, bound to port", baseport + i);
	    break;
	}
    }
    
    if (i == TCP_MAX_PORTTRY) {
	DSECTLEAVE
	    return (-1);
    }
    
    DNOTICE("listening for clients");
    if (listen(s, iClients) < 0) {
	perror("listen");
	SMI_Abort(-1);
    }
    
    DSECTLEAVE
	return(s);
}

int _smi_tcp_bcast_srv(tcp_ident_t* idLocal, int iSender, char *data, size_t size, int iClients, int baseport)
{
    DSECTION("_smi_tcp_bcast_srv");
    SOCKTYPE           s;
    SOCKTYPE* 	   	   pNs;
    int                i;
    /*  tcp_ident_t        idRemote; */
    tcp_ident_t*       pIdents;
    socklen_t                len = sizeof(struct sockaddr);
    struct sockaddr_in name;
    tcp_node_info_t*   pNodeInfo;
    
    DSECTENTRYPOINT;
 
    pNodeInfo = tcp_create_node_list(iClients);
    tcp_node_list_add(pNodeInfo, idLocal);

    s = _smi_tcp_start_server(baseport, &name, iClients);
    if (s == -1)
	return(-1);
    
    ALLOCATE( pNs, SOCKTYPE*, sizeof(SOCKTYPE) * iClients );
    ALLOCATE( pIdents,  tcp_ident_t*, sizeof(tcp_ident_t) * iClients );
    
    for (i = 0; i < iClients;) {
	if ((pNs[i] = accept(s, (struct sockaddr *) &name, &len)) < 0) {
	    perror("accept");
	    SMI_Abort(-1);
	}
	DNOTICEI("syncing with client", i);
	_smi_tcp_recv(pNs[i], (char *) &(pIdents[i]), sizeof(tcp_ident_t), 0);
	_smi_tcp_send(pNs[i], (char *) idLocal, sizeof(tcp_ident_t), 0);
	
	if (_smi_tcp_ident_valid(idLocal, &(pIdents[i]))) {
	    DNOTICE("a valid connection was established");

	    tcp_node_list_add(pNodeInfo, &(pIdents[i]));
	    if (iSender == pIdents[i].iProcRank) {
		ASSERT_A((FALSE),"Please report, that this situation has occured", SMI_ERR_OTHER);
		_smi_tcp_recv(pNs[i], data, size, 0); 
	    } 
	    i++;
	} 
	else {
	    close(pNs[i]);
	    DNOTICE("an invalid connection was aborted");
	}
    }
    /* wait until all clients connected have connected before sending data */
    /* it's possible to send information about the topology this way */
    for (i=0; i<iClients; i++) {
	tcp_mk_smp_info(pNodeInfo, &(pIdents[i]));
	_smi_tcp_send(pNs[i], data, size, 0);
	_smi_tcp_send(pNs[i], (char*) &(pIdents[i].SmpInfo), sizeof(tcp_smp_info_t), 0);
	close(pNs[i]);
    }

    close(s);
    
    /* generate own information */
    tcp_mk_smp_info(pNodeInfo, idLocal);
    
    free(pIdents);
    free(pNs);
    tcp_remove_node_list(pNodeInfo);
    
    DSECTLEAVE;	
    return (0);
}

int _smi_tcp_allgather_srv(tcp_ident_t * idLocal, char* pDataIn, char* pDataOut, size_t size, int iNumProcs, int baseport)
{
    DSECTION("_smi_tcp_allgather_srv");
    struct sockaddr_in name;
    tcp_ident_t        idRemote;
    int                i;
    SOCKTYPE*	       pNs, s;
    int                iClients = iNumProcs-1;
    socklen_t                len = sizeof(struct sockaddr);
    
    DSECTENTRYPOINT;
    
    memcpy(pDataOut, pDataIn, size);
    
    s = _smi_tcp_start_server(baseport, &name, iClients);
    if (s == -1)
	return(-1);
    
    ALLOCATE( pNs, SOCKTYPE*, sizeof(SOCKTYPE) * iClients );
    
    for (i = 0; i < iClients;) {
	if ((pNs[i] = accept(s, (struct sockaddr *) &name, &len)) < 0) {
	    perror("accept");
	    SMI_Abort(-1);
	}
	DNOTICEI("syncing with client", i);
	_smi_tcp_recv(pNs[i], (char *) &idRemote, sizeof(idRemote), 0);
	_smi_tcp_send(pNs[i], (char *) idLocal, sizeof(tcp_ident_t), 0);
	
	if (_smi_tcp_ident_valid(idLocal, &idRemote)) {
	    DNOTICE("a valid connection was established");
	    _smi_tcp_recv(pNs[i], pDataOut + (size * idRemote.iProcRank), size, 0);
	    i++; 
	} 
	else {
	    close(pNs[i]);
	    DNOTICE("an invalid connection was aborted");
	}
    }
    for (i=0; i<iClients; i++) {
	_smi_tcp_send(pNs[i], pDataOut, size * iNumProcs, 0);
	close(pNs[i]);
    }
    
    close(s);
    free (pNs);

    DSECTLEAVE; return (0);
}

SOCKTYPE _smi_tcp_connect_client(char* hostname, int baseport, tcp_ident_t* idLocal)
{
    DSECTION("_smi_tcp_connect_client");
    SOCKTYPE        s;
    int             value;
    tcp_status_t    status;
    int             i, portplus;
    tcp_ident_t     idRemote;
    struct sockaddr_in name;
    struct hostent *hp;
    
    DSECTENTRYPOINT;
    
    hp = gethostbyname(hostname);
    ASSERT_A((hp != NULL), "unknown host", -1);
    
    i = 0;
    portplus = 0;
    DNOTICEI("trying to connect to port", baseport+portplus);
    DNOTICES("                  of host", hostname);
    do {
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror("socket");
	    SMI_Abort(-1);
	}
	value = 1;
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *) &value, sizeof(value));
	memset(&name, 0, sizeof(struct sockaddr_in));
	name.sin_family = AF_INET;
	name.sin_port = htons((unsigned short)(baseport + portplus));
	memcpy(&name.sin_addr, hp->h_addr_list[0], hp->h_length);
#ifdef WIN32
	if (connect(s, (const struct sockaddr FAR *) & name, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
	    int             wsaerr = WSAGetLastError();
	    switch (wsaerr) {
	    case WSAECONNREFUSED:
		status = ts_connrefused;
		DNOTICE ("connect() failed with ECONNREFUSED - server not yet ready; trying again");
		break;
	    case WSAECONNRESET:
		status = ts_connreset;
		DNOTICE ("connect() failed with ECONNRESET - trying again");
		break;
	    default:
		perror("connect");
		SMI_Abort(-1);
	    }
	}
#else 
	if (connect(s, (struct sockaddr *) & name, sizeof(struct sockaddr_in)) < 0) {
	    switch (errno) {
	    case ECONNREFUSED:
		status = ts_connrefused;
		DNOTICE ("connect() failed with ECONNREFUSED - server not yet ready; trying again");
		break;
	    case ECONNRESET:
		status = ts_connreset;
		DNOTICE ("connect() failed with ECONNRESET - trying again");
		break;
	    default:
		perror("connect");
		SMI_Abort(-1);
	    }
	}
#endif /* WIN32 */
	else {
	    DNOTICE("trying to sync with server");
	    if ( 
		(_smi_tcp_send(s, (char *) idLocal, sizeof(tcp_ident_t), 0) != -1 )
		&&
		(_smi_tcp_recv(s, (char *) &idRemote, sizeof(idRemote), 0) != -1)
		)
		status = (_smi_tcp_ident_valid(idLocal, &idRemote)) 
		    ? ts_ok : ts_wrongident;
	    else
		status = ts_connreset;
	}
	
	if (status != ts_ok) {	 
	    close(s);
	    switch (status) {
	    case ts_connreset:
		DPROBLEM("connection failed, trying again");   
		break;
	    case ts_connrefused:
		i++; 
		if (i == TCP_MAX_CONNTRY) {
		    i = 0;
		    DNOTICE("taking next portaddr");
		    portplus++;
		}
		if (portplus == TCP_MAX_PORTTRY) {
		    DSECTLEAVE
			return (-1);
		}
		usleep(TCP_RETRY_DELAY*1000);
		break;
	    case ts_wrongident:
		if (portplus == TCP_MAX_PORTTRY) {
		    DSECTLEAVE
			return (-1);
		}		
		i = 0;
		DNOTICE("taking next portaddr");
		portplus++;
		break;
	    default:
		DERROR("this may not happen");
		SMI_Abort(-1);
	    }
	}
    } while (status != ts_ok);
    
    DSECTLEAVE;
    return(s);
}

int _smi_tcp_bcast_cl(tcp_ident_t * idLocal, int iSender, char *data, size_t size, char *hostname, int baseport)
{
    DSECTION("_smi_tcp_bcast_cl");
    SOCKTYPE         s;
    
    DSECTENTRYPOINT;
    
    s = _smi_tcp_connect_client(hostname, baseport, idLocal);
    if (s == -1)
		return(-1);
    
    DNOTICE("a valid connection was established");
    if (iSender == idLocal->iProcRank)
	_smi_tcp_send(s, data, size, 0); 
    else
	_smi_tcp_recv(s, data, size, 0);

    _smi_tcp_recv(s, (char*) &(idLocal->SmpInfo), sizeof(tcp_smp_info_t), 0);

    close(s);
    
    DSECTLEAVE; return (0); 
}

int _smi_tcp_allgather_cl(tcp_ident_t * idLocal, char* pDataIn, char* pDataOut,
			  size_t size, int iNumProcs, char *hostname, int baseport)
{
    DSECTION("_smi_tcp_allgather_cl");
    SOCKTYPE s;
    
    DSECTENTRYPOINT;
    
    s = _smi_tcp_connect_client(hostname, baseport, idLocal);
    if (s == -1)
	return(-1);
    
    DNOTICE("a valid connection was established");
    _smi_tcp_send(s, pDataIn, size, 0); 
    _smi_tcp_recv(s, pDataOut, size * iNumProcs, 0);
    close(s);
    
    DSECTLEAVE; return (0); 
}


int _smi_tcp_mkident(tcp_ident_t* Ident, int iMyRank, int iMagicNumber, char
		     *szExecName, int iMemKey)
{
  Ident->iMagicNumber = iMagicNumber;
#ifdef FILE_IDENT
  strcpy(Ident->szExecName, szExecName);
#else
  /* allow different executables (from different pathes) to communicate */
  strcpy(Ident->szExecName, "anyname");
#endif
  
  if (getenv("USER") != NULL)
      strcpy(Ident->szUserName, getenv("USER"));
  else
      Ident->szUserName[0] = '\0';
  
  gethostname(Ident->szLocalHost, TCP_LEN_HOSTNAME);

  memset(&(Ident->SmpInfo), 0, sizeof(tcp_smp_info_t));
  Ident->SmpInfo.iLocalMemKey= iMemKey;
  Ident->iProcRank = iMyRank;
  
  return (0);
}

int  _smi_TcpBroadcast(char *szHostName, int iPortNumber, void *pData, size_t stLen,
		       int iSender, int iNumProcs, tcp_ident_t * idLocal)
{
    DSECTION("_smi_TcpBroadcast");
    int             RetVal;
    
    DSECTENTRYPOINT;
    
    if (iNumProcs == 0)
	return(SMI_SUCCESS);
    
    if (idLocal->iProcRank == 0) {
	RetVal = _smi_tcp_bcast_srv(idLocal, iSender, (char *) pData, stLen, iNumProcs - 1, iPortNumber);
    } else {
	RetVal = _smi_tcp_bcast_cl(idLocal, iSender, (char *) pData, stLen, szHostName, iPortNumber);
    }
    
    RetVal = (RetVal != -1) ? SMI_SUCCESS : SMI_ERR_OTHER;
    
    DSECTLEAVE;
    return (RetVal);
}

int  _smi_TcpAllgather(char *szHostName, int iPortNumber, void *pDataIn, void *pDataOut, size_t stLen,
		       int iNumProcs, tcp_ident_t* idLocal)
{
    DSECTION("_smi_TcpAllgather");
    int RetVal;
    
    DSECTENTRYPOINT;
    
    if (iNumProcs == 0)
	return(SMI_SUCCESS);
    
    if (idLocal->iProcRank == 0) {
	RetVal = _smi_tcp_allgather_srv(idLocal, (char *) pDataIn, (char*) pDataOut, stLen, iNumProcs, iPortNumber);
    } else {
	RetVal = _smi_tcp_allgather_cl(idLocal,(char *) pDataIn, (char*) pDataOut, stLen, iNumProcs, szHostName, iPortNumber);
    }
    
    RetVal = (RetVal != -1) ? SMI_SUCCESS : SMI_ERR_OTHER;
    
    DSECTLEAVE;
    return (RetVal);
}


int _smi_tcp_init()
{
#ifdef WIN32
    WORD            wVersionRequested;
    WSADATA         wsaData;
    int             err;
    
    wVersionRequested = MAKEWORD(2, 0);
    
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
	DPROBLEM("couldn't find a usable winSock DLL");
	return (-1);
    }
    if (LOBYTE(wsaData.wVersion) != 2	/* || HIBYTE(wsaData.wVersion ) != 0 */ ) {
	DPROBLEM("couldn't find a usable WinSock DLL");
	WSACleanup();
	return (-1);
    }
#endif	/* WIN32 */
    return (0);
}


int _smi_tcp_finalize()
{
#ifdef WIN32
    return ((WSACleanup() != SOCKET_ERROR) ? 0 : -1);
#endif
    return (0);
}
