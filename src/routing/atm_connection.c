/* $Id: atm_connection.c,v 1.10.8.1 2004/12/03 12:27:20 martin Exp $
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 */

#include "atm_connection.h"

#ifdef WIN32
#define  usleep(x) Sleep( (unsigned int) (x /1000))
#endif

#ifdef ROUTER_DEBUG
int sprintf_atmsvc_address(char *buff, struct sockaddr_atmsvc *addr) {
#ifdef WIN32
	sprintf(buff, "[%.2x...%.2x.%.2x.%.2x.%.2x.%.2x.%.2x.%.2x]",
		addr->satm_number.Addr[0],addr->satm_number.Addr[13],
		addr->satm_number.Addr[14],addr->satm_number.Addr[15],
		addr->satm_number.Addr[16],addr->satm_number.Addr[17],
		addr->satm_number.Addr[18],addr->satm_number.Addr[19]);
#else
	sprintf(buff, "[%.2x...%.2x.%.2x.%.2x.%.2x.%.2x.%.2x.%.2x]",
		addr->sas_addr.prv[0], addr->sas_addr.prv[13],addr->sas_addr.prv[14],addr->sas_addr.prv[15],
		addr->sas_addr.prv[16],addr->sas_addr.prv[17],addr->sas_addr.prv[18],addr->sas_addr.prv[19]);
#endif   
	
	return 0;
}
#endif

    
 
/*
XXX
atm_establish_pvc_sock_conn() doesn't work yet, 
because the connection is not synchronized
XXX

  return code
  = 0: OK
  < 0: ERROR
*/
int atm_establish_pvc_sock_conn(int *localsocket_fd, struct sockaddr *loc_addr,
                                int *remotesocket_fd, struct sockaddr *rem_addr,
                                char *qos,
                                unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize,
                                int max_nconn, int serv, int timeout) {
	int ret;
	int addr_size;
	
	QOS bqos;

	addr_size = sizeof(ATM_PVC_PARAMS);
	
#ifdef WIN32
	memset(&bqos,0,sizeof(bqos));
	text2qos(qos,&(((ATM_PVC_PARAMS*)(loc_addr))->PvcQos),0);

	if (WSAIoctl(*remotesocket_fd, SIO_ASSOCIATE_PVC, (ATM_PVC_PARAMS*)loc_addr, addr_size, NULL, 0, &ret, NULL, NULL) != 0) {
		RERROR1("WSAIoctl() failed: %d\r\n",WSAGetLastError());
		return ERR_ATM_CONN_CONNECT;
	}

#else
	text2qos(qos, &bqos, T2Q_DEFAULTS);
	if( setsockopt(*remotesocket_fd, SOL_ATM, SO_ATMQOS, (struct atm_qos *)&bqos, sizeof(struct atm_qos)) == -1) {
		RERROR1("%satm_establish_pvc_sock_conn(): set socket options SO_ATMQOS: ",hostname); perror("");
		return ERR_ATM_CONN_SETSOCKOPT;
	}
	if( connect(*remotesocket_fd, loc_addr, addr_size) < 0 ) {
		RERROR1("%satm_establish_pvc_sock_conn(): error connecting pvc socket: ",hostname); perror("");
		return ERR_ATM_CONN_CONNECT;
	}
#endif
   atm_setMaxSduSize(&bqos,rcvMaxSduSize,sndMaxSduSize);
	/* XXX
	PVC connection should be synchronized at this place!
	*/
	return 0;
}



#ifdef WIN32

/* return code
= 0: OK
< 0: ERROR
*/
int atm_establish_svc_sock_conn(int *localsocket_fd, struct sockaddr *loc_addr,
                                int *remotesocket_fd, struct sockaddr *rem_addr,
                                char *qos,
                                unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize,
                                int max_nconn, int serv, int timeout) 
{
	int ret, i, addr_size, *s;
	
	QOS bqos;
	DWORD ConnData=0;         /* connection-specific data */
   
   LPQOS lpvOUTBuffer;
   DWORD cbOUTBuffer;
   DWORD cbBytesReturned;
	
#ifdef ROUTER_DEBUG
	int bytes;
	ATM_CONNECTION_ID connID;
   char loc_host_name[3*ATM_ESA_LEN+1], rem_host_name[3*ATM_ESA_LEN+1];
	sprintf_atmsvc_address(loc_host_name, (struct sockaddr_atmsvc *) loc_addr);
	sprintf_atmsvc_address(rem_host_name, (struct sockaddr_atmsvc *) rem_addr);
	RDEBUG3("%satm_establish_sock_conn: %s -> %s\n",hostname, loc_host_name, rem_host_name);
#endif
	
	memset(&bqos,0,sizeof(bqos));  
	text2qos(qos, &bqos, 0);

	addr_size = sizeof(struct sockaddr_atmsvc);

   if( serv == CONN_CLIENT )
		s = remotesocket_fd;
	else 
		s = localsocket_fd;

   cbOUTBuffer = 1512;
   lpvOUTBuffer = (struct _QualityOfService *) malloc(sizeof(char)*cbOUTBuffer);
   memset(lpvOUTBuffer, 0, cbOUTBuffer);

 	if( serv == CONN_SERV ) { /* server accepts, passive open */
		if (bind(*localsocket_fd, loc_addr, addr_size) == -1) {
			RERROR1("%satm_establish_sock_conn(): error binding svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_BIND;
		}
		if (listen(*localsocket_fd, max_nconn)) {
			RERROR1("%satm_establish_sock_conn(): error listening svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_LISTEN;
		}
		if( (*remotesocket_fd = WSAAccept(*localsocket_fd, NULL,
			NULL, ConditionFunc, (DWORD)ConnData)) == SOCKET_ERROR ) {
			RERROR1("%satm_establish_sock_conn(): error accepting svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_ACCEPT;
		}
   	close(*localsocket_fd);
		/*   connlist[conn].state = CONNSTATE_UP; */
		RDEBUG1("%sconnected to client!\n",hostname);
	}
	else  { /* client connects, active open */

/*      if( WSAIoctl(*remotesocket_fd, SIO_SET_QOS, &bqos,sizeof(bqos), (LPVOID)lpvOUTBuffer, cbOUTBuffer,
         &cbBytesReturned, NULL, NULL) )
         ret = WSAGetLastError();
      */

		if (bind(*remotesocket_fd, loc_addr, addr_size) == -1) {
			RERROR1("%satm_establish_sock_conn(): error binding svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_BIND;
		}
		
		RDEBUG1("%satm_establish_sock_conn(): trying to connect ",hostname);
		for( i=0; i<=100; i++ ) { /* try 100 times to connect */
			RDEBUG(". ");
//         ret = WSAConnect(*remotesocket_fd, rem_addr, addr_size, NULL, NULL, &bqos, NULL);
         ret = WSAConnect(*remotesocket_fd, rem_addr, addr_size, NULL, NULL, NULL, NULL);
#ifdef ROUTER_DEBUG
			if(!ret) {
				if (WSAIoctl(*remotesocket_fd, SIO_GET_ATM_CONNECTION_ID, NULL, 0, (LPVOID) &connID, 
					sizeof(ATM_CONNECTION_ID), &bytes, NULL, NULL) == SOCKET_ERROR)
				{
					RERROR1("Error: WSAIoctl %d\n", WSAGetLastError());
				}
				RDEBUG2("(%d, %d)\n", connID.VPI, connID.VCI);
         }
#endif
			if( !ret )
				break; 
			usleep(1000000);
		}
		if( !ret ) {
			RDEBUG2("\n%satm_establish_sock_conn(): connected to server %s ...\n", hostname, rem_host_name);
		}
		else {
			RERROR1("\n%satm_establish_sock_conn(): error connecting svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_CONNECT;
		}
	}
   /* get qos parameter */
   cbOUTBuffer = 1512;
   memset(lpvOUTBuffer, 0, cbOUTBuffer);
   if( WSAIoctl(*remotesocket_fd, SIO_GET_QOS, NULL, 0, (LPVOID)lpvOUTBuffer, cbOUTBuffer,
      &cbBytesReturned, NULL, NULL) )
      ret = WSAGetLastError();
   
   atm_setMaxSduSize((QOS *)lpvOUTBuffer,rcvMaxSduSize,sndMaxSduSize);
#ifdef ROUTER_DEBUG   
   evaluate_QOS( lpvOUTBuffer );
#endif
   free(lpvOUTBuffer);

	return 0;
}

#else /* LINUX */

/* return code
= 0: OK
< 0: ERROR
*/
int atm_establish_svc_sock_conn(int *localsocket_fd, struct sockaddr *loc_addr,
                                int *remotesocket_fd, struct sockaddr *rem_addr,
                                char *qos,
                                unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize,
                                int max_nconn, int serv) {
	int ret, i, addr_size;
	int *s;
	
	struct atm_sap sap;
	struct atm_qos bqos;
	
#ifdef ROUTER_DEBUG
	char loc_host_name[3*ATM_ESA_LEN+1], rem_host_name[3*ATM_ESA_LEN+1];
	sprintf_atmsvc_address(loc_host_name, (struct sockaddr_atmsvc *) loc_addr);
	sprintf_atmsvc_address(rem_host_name, (struct sockaddr_atmsvc *) rem_addr);
	RDEBUG3("%satm_establish_sock_conn: %s -> %s\n",hostname, loc_host_name, rem_host_name);
#endif
	
	if( serv == CONN_CLIENT )
		s = remotesocket_fd;
	else 
		s = localsocket_fd;
	
	memset(&bqos,0,sizeof(bqos));  
	
	text2qos(qos, &bqos, T2Q_DEFAULTS);
	if( setsockopt(*s, SOL_ATM, SO_ATMQOS, (struct atm_qos *)&bqos, sizeof(struct atm_qos)) == -1) {
		RERROR1("%satm_establish_sock_conn(): set socket options SO_ATMQOS: ",hostname); perror("");
		return ERR_ATM_CONN_SETSOCKOPT;
	}
	memset(&sap, 0, sizeof(sap));
	/* We don't use SAP addressing */
	sap.blli[0].l2_proto = ATM_L2_NONE;
	sap.blli[0].l3_proto = ATM_L3_NONE;
	if( setsockopt(*s,SOL_ATM, SO_ATMSAP, &sap,sizeof(sap)) < 0) {
		RERROR1("%satm_establish_sock_conn(): setsockopt ATMSAP: ",hostname); perror("");
		return ERR_ATM_CONN_SETSOCKOPT;
	}

	text2qos(qos, &bqos, 0);
	addr_size = sizeof(struct sockaddr_atmsvc);
	if( serv == CONN_SERV ) { /* server accepts, passive open */
		if (bind(*localsocket_fd, loc_addr, addr_size) == -1) {
			RERROR1("%satm_establish_sock_conn(): error binding svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_BIND;
		}
		if (listen(*localsocket_fd, max_nconn)) {
			RERROR1("%satm_establish_sock_conn(): error listening svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_LISTEN;
		}
		if( (*remotesocket_fd = accept(*localsocket_fd, NULL, NULL)) == -1) {
			RERROR1("%satm_establish_sock_conn(): error accepting svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_ACCEPT;
		}
		close(*localsocket_fd);
		/*   connlist[conn].state = CONNSTATE_UP; */
		RDEBUG1("%sconnected to client!\n",hostname);
	}
	else  { /* client connects, active open */
		if (bind(*remotesocket_fd, loc_addr, addr_size) == -1) {
			RERROR1("%satm_establish_sock_conn(): error binding svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_BIND;
		}
		
		RDEBUG1("%satm_establish_sock_conn(): trying to connect ",hostname);
		for( i=0; i<=100; i++ ) { /* try to connect 100 times */
			RDEBUG(". ");
			ret = connect(*remotesocket_fd, rem_addr, addr_size);
			if( !ret )
				break;
			usleep(1000000);
		}
		if( !ret ) {
			RDEBUG2("\n%satm_establish_sock_conn(): connected to server %s ...\n", hostname, rem_host_name);
		}
		else {
			RERROR1("\n%satm_establish_sock_conn(): error connecting svc socket: ",hostname); perror("");
			return ERR_ATM_CONN_CONNECT;
		}
	}
   atm_setMaxSduSize(&bqos,rcvMaxSduSize,sndMaxSduSize);
	return 0;
}
#endif /* NOT WIN32 */

/* return value:
number of bytes received on socket fd
XXX 
be sure, that maxSDUsize < Maximum (signed) int value
XXX
*/
int atm_read(void *buffer, unsigned size, int fd, unsigned long maxSDUsize)
{
    char *pos = (char *) buffer;       
    unsigned rest = size;
    int i, received;
	
    FN_IN_DEBUG("atm_read");

    do {
	received = recv(fd, pos, maxSDUsize < rest ? (size_t)maxSDUsize : (size_t)rest , 0);
	if (received == 0)
	    RERROR("atm_read: 0 Bytes received\n");
	if (received > 0) {
	    rest -= received;
	    pos += received;
	} else {
	    FN_OUT_DEBUG("atm_read");
	    return (size - rest);
	}
/*   	printf("recv: %i - %i\n", received, rest); */
    } while (rest > 0);

    FN_OUT_DEBUG("atm_read");
    return size;
}

/* return value:
number of bytes sent on socket fd : OK
-1 : error
*/
int atm_write(void *buffer, unsigned size, int fd, unsigned long maxSDUsize)
{
	char           *pos = (char *) buffer;
	unsigned long  rest;
	int sends, i, sent;

	FN_IN_DEBUG("atm_write");

	sends = size / maxSDUsize;
	rest  = size % maxSDUsize;
	
	for(i=0; i<sends; i++) {
		if( (sent = write(fd,pos,maxSDUsize)) != (signed) maxSDUsize ) {
			sprintf(serr, "%satm_write: send(fd=%d,size=%d) returned %d: ", hostname, fd, maxSDUsize, sent);
			RERROR(serr); perror("");
			return -1; 
		}
/*  		printf("sent: %i\n", sent); */
		pos +=maxSDUsize;
	}
	if( (sent = write(fd,pos,rest)) != (signed) rest ) {
		sprintf(serr, "%satm_write: send(fd=%d,size=%d) returned %d: ", hostname, fd, rest, sent);
		RERROR(serr); perror("");
		return -1;
	}
/*  	printf("sent: %i\n", sent); */
	
	FN_OUT_DEBUG("atm_write");	
	return size;
}


/* return value: 
= 0: OK
< 0: ERROR
*/
int atm_add_socket_pair (int *localsocket, int *remotesocket, 
                         unsigned short sa_family, int serv) {
	int *s;
	
#ifdef WIN32
	LPWSAPROTOCOL_INFO COProtocolInfo;   // current protocol info to examine
	LPWSAPROTOCOL_INFO InstalledProtocols = NULL;
	DWORD BufferSize = 0;       // size of InstalledProtocols buffer
	int i;
	int found_protocol = 0;
	// number of WSAPROTOCOL_INFO structs in the InstalledProtocols buffer 
	static int NumProtocols = 0;
#endif
	
	if( (serv == CONN_CLIENT) || (sa_family == PF_ATMPVC) ) {
		s = remotesocket;
		*localsocket = -1;
	}
	else {
		s = localsocket;
		*remotesocket = -1;
	}
	
#ifdef WIN32
	NumProtocols  = WSAEnumProtocols(NULL, NULL, &BufferSize);
	InstalledProtocols = (LPWSAPROTOCOL_INFO)malloc(BufferSize);
	NumProtocols = WSAEnumProtocols(NULL, (LPVOID)InstalledProtocols, &BufferSize);
	
	if (NumProtocols == SOCKET_ERROR) {
		RERROR1("WSAEnumProtocols failed.  Error Code: %d",WSAGetLastError());
		return ERR_ATM_CONN_SOCKET;
	}
	for (i = 0; i < NumProtocols; i++) {
		COProtocolInfo = &InstalledProtocols[i];
		if(!(COProtocolInfo->dwServiceFlags1 & XP1_CONNECTIONLESS) &&
			(COProtocolInfo->dwServiceFlags1 & XP1_GUARANTEED_ORDER)&&
			(COProtocolInfo->iAddressFamily == AF_ATM))
		{
			found_protocol = 1;
			break;
		}
	}
	if( !found_protocol ) {
		RERROR("No suitable ATM protocol found\n");
		return ERR_ATM_CONN_SOCKET;
	}
	if ((*s = WSASocket(AF_ATM, SOCK_RAW, ATMPROTO_AAL5, COProtocolInfo, 0, 0)) == INVALID_SOCKET) {
		RERROR1("WSASocket: Error = %d\n", WSAGetLastError());
		return ERR_ATM_CONN_SOCKET;
	}
#else
	if ((*s = socket(sa_family,SOCK_DGRAM,ATM_AAL5)) == -1) {
		RERROR1("%sadd_atm_socket_pair: cannot create local socket: ",hostname); perror("");
		return ERR_ATM_CONN_SOCKET;
	}
#endif
	
	return 0;
}

#ifdef WIN32
/* 
this function shold work identically with text2qos under LINUX
now ignores the text and sets QOS structure to some preset values
(CBR, AAL5, pcr=250, tx=rx, maxsdu=1516)
parameter flags is ignored
*/
int text2qos(const char *text, QOS *bqos, int flags)
{
/*	XXX Do  not include any provider-specific information.XXX

   /* BEGIN OF PROVIDER SPECIFIC INFORMATION */
   int size;
   AAL_PARAMETERS_IE ie_aalparams;
   ATM_TRAFFIC_DESCRIPTOR_IE ie_td;
   ATM_BROADBAND_BEARER_CAPABILITY_IE ie_bbc;
   //ATM_BLLI_IE ie_blli;
   ATM_QOS_CLASS_IE ie_qos;
   Q2931_IE *ie_ptr;
   
   ie_aalparams.AALType = AALTYPE_5;
   ie_aalparams.AALSpecificParameters.AAL5Parameters.ForwardMaxCPCSSDUSize = DEFAULT_MAX_SDU_SIZE;
   ie_aalparams.AALSpecificParameters.AAL5Parameters.BackwardMaxCPCSSDUSize = DEFAULT_MAX_SDU_SIZE;
   ie_aalparams.AALSpecificParameters.AAL5Parameters.Mode = AAL5_MODE_MESSAGE;
   ie_aalparams.AALSpecificParameters.AAL5Parameters.SSCSType = AAL5_SSCS_NULL;
   
   size = sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(AAL_PARAMETERS_IE);
   
   ie_td.Forward.PeakCellRate_CLP0 = SAP_FIELD_ABSENT;
   ie_td.Forward.PeakCellRate_CLP01 = SAP_FIELD_ABSENT; // dummy value; max = 318396 cells/s (135Mbit/s) 
   ie_td.Forward.SustainableCellRate_CLP0 = SAP_FIELD_ABSENT;
   ie_td.Forward.SustainableCellRate_CLP01 = SAP_FIELD_ABSENT;
   ie_td.Forward.MaxBurstSize_CLP0 = SAP_FIELD_ABSENT;
   ie_td.Forward.MaxBurstSize_CLP01 = SAP_FIELD_ABSENT;
   ie_td.Forward.Tagging = SAP_FIELD_ABSENT;
   
   ie_td.Backward.PeakCellRate_CLP0 = SAP_FIELD_ABSENT;
   ie_td.Backward.PeakCellRate_CLP01 = SAP_FIELD_ABSENT; // dummy value; max = 318396 cells/s (135Mbit/s)
   ie_td.Backward.SustainableCellRate_CLP0 = SAP_FIELD_ABSENT;
   ie_td.Backward.SustainableCellRate_CLP01 = SAP_FIELD_ABSENT;
   ie_td.Backward.MaxBurstSize_CLP0 = SAP_FIELD_ABSENT;
   ie_td.Backward.MaxBurstSize_CLP01 = SAP_FIELD_ABSENT;
   ie_td.Backward.Tagging = SAP_FIELD_ABSENT;
   
   ie_td.BestEffort = 1;// Note: this must be set to zero for CBR
   
   size += sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(ATM_TRAFFIC_DESCRIPTOR_IE);
   
   ie_bbc.BearerClass = BCOB_X;
   ie_bbc.TrafficType = TT_NOIND;
   ie_bbc.TimingRequirements = TR_NOIND;
   ie_bbc.ClippingSusceptability = CLIP_NOT;
   ie_bbc.UserPlaneConnectionConfig = UP_P2P;
   
   size += sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE);
   
   ie_qos.QOSClassForward = QOS_CLASS0;
   ie_qos.QOSClassBackward = QOS_CLASS0;// this may not be really used since we do
   //  only simplex data xfer 
   size += sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(ATM_QOS_CLASS_IE);
   
   bqos->ProviderSpecific.buf = (char *) malloc(size);
   bqos->ProviderSpecific.len = size;
   memset(bqos->ProviderSpecific.buf, 0, size);
   
   ie_ptr = (Q2931_IE *) bqos->ProviderSpecific.buf;
   ie_ptr->IEType = IE_AALParameters;
   ie_ptr->IELength = sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(AAL_PARAMETERS_IE);
   memcpy(ie_ptr->IE, &ie_aalparams, sizeof(AAL_PARAMETERS_IE));
   
   ie_ptr = (Q2931_IE *) ((char *)ie_ptr + ie_ptr->IELength);
   ie_ptr->IEType = IE_TrafficDescriptor;
   ie_ptr->IELength = sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(ATM_TRAFFIC_DESCRIPTOR_IE);
   memcpy(ie_ptr->IE, &ie_td, sizeof(ATM_TRAFFIC_DESCRIPTOR_IE));
   
   ie_ptr = (Q2931_IE *) ((char *)ie_ptr + ie_ptr->IELength);
   ie_ptr->IEType = IE_BroadbandBearerCapability;
   ie_ptr->IELength = sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE);
   memcpy(ie_ptr->IE, &ie_bbc, sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE));
   
   ie_ptr = (Q2931_IE *) ((char *)ie_ptr + ie_ptr->IELength);
   ie_ptr->IEType = IE_QOSClass;
   ie_ptr->IELength = sizeof(Q2931_IE_TYPE) + sizeof(ULONG) + sizeof(ATM_QOS_CLASS_IE);
   memcpy(ie_ptr->IE, &ie_qos, sizeof(ATM_QOS_CLASS_IE));
   /* END OF PROVIDER SPECIFIC INFORMATION */

	bqos->SendingFlowspec.TokenRate = SAP_FIELD_ABSENT;              /* Bytes/s */
	bqos->SendingFlowspec.TokenBucketSize = SAP_FIELD_ABSENT;      /* Bytes */
	bqos->SendingFlowspec.PeakBandwidth = SAP_FIELD_ABSENT;          /* Bytes/s */
	bqos->SendingFlowspec.Latency = SAP_FIELD_ABSENT;           /* microseconds */
	bqos->SendingFlowspec.DelayVariation = SAP_FIELD_ABSENT;  /* microseconds */
	bqos->SendingFlowspec.ServiceType = SAP_FIELD_ABSENT; /* will most probably be ignored by the service provider */
	bqos->SendingFlowspec.MaxSduSize = DEFAULT_MAX_SDU_SIZE;         /* Bytes */
	bqos->SendingFlowspec.MinimumPolicedSize = -1;/* Bytes */
	
	bqos->ReceivingFlowspec.TokenRate = SAP_FIELD_ABSENT;
	bqos->ReceivingFlowspec.TokenBucketSize = SAP_FIELD_ABSENT;
	bqos->ReceivingFlowspec.PeakBandwidth = SAP_FIELD_ABSENT;
	bqos->ReceivingFlowspec.Latency = SAP_FIELD_ABSENT;
	bqos->ReceivingFlowspec.DelayVariation = SAP_FIELD_ABSENT;
	bqos->ReceivingFlowspec.ServiceType = SAP_FIELD_ABSENT; /* will most probably be ignored by the service provider */
	bqos->ReceivingFlowspec.MaxSduSize = DEFAULT_MAX_SDU_SIZE;
	bqos->ReceivingFlowspec.MinimumPolicedSize = -1;
	
	return 0;
}

/* Flags are unused. Just needed to be linux compatible */
int text2atm(char *text, struct sockaddr *caddr, int length, int flags) {
        sockaddr_atm* baddr = (sockaddr_atm*)caddr;

	if( !text || !baddr )
		return -1;
	memset(baddr, 0, length);
	sscanf(text,"%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x",
		&baddr->satm_number.Addr[0],&baddr->satm_number.Addr[1],&baddr->satm_number.Addr[2],
		&baddr->satm_number.Addr[3],&baddr->satm_number.Addr[4],&baddr->satm_number.Addr[5],
		&baddr->satm_number.Addr[6],&baddr->satm_number.Addr[7],&baddr->satm_number.Addr[8],
		&baddr->satm_number.Addr[9],&baddr->satm_number.Addr[10],&baddr->satm_number.Addr[11],
		&baddr->satm_number.Addr[12],&baddr->satm_number.Addr[13],&baddr->satm_number.Addr[14],
		&baddr->satm_number.Addr[15],&baddr->satm_number.Addr[16],&baddr->satm_number.Addr[17],
		&baddr->satm_number.Addr[18],&baddr->satm_number.Addr[19]);
	baddr->satm_family              = AF_ATM;
	baddr->satm_number.AddressType  = ATM_NSAP;
	baddr->satm_number.NumofDigits  = ATM_ADDR_SIZE;
	baddr->satm_blli.Layer2Protocol = SAP_FIELD_ANY;
	baddr->satm_blli.Layer3Protocol = SAP_FIELD_ABSENT;
	baddr->satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;
	return 0;
}

int CALLBACK ConditionFunc(
                           IN LPWSABUF lpCallerId,
                           IN LPWSABUF lpCallerData,
                           IN OUT LPQOS lpSQOS,
                           IN OUT LPQOS lpGQOS,
                           IN LPWSABUF lpCalleeId,
                           OUT LPWSABUF lpCalleeData,
                           OUT GROUP FAR * g,
                           IN DWORD dwCallbackData
                           )
{
   sockaddr_atm *caller_address;
   sockaddr_atm *callee_address;
   /* only bhli and blli addresses of caller are known at this time */
   caller_address = (sockaddr_atm *) lpCallerId->buf;
   /* the whole atm address of caller is given */
   callee_address = (sockaddr_atm *) lpCalleeId->buf;

   RDEBUG("Conditional function called\n");

#ifdef ROUTER_DEBUG
   evaluate_QOS( lpSQOS );
#endif
   
	return CF_ACCEPT;
}

/* 
   looking for pointer to IE of given type in WSABUF structure 
   returns NULL, if no IE of given type found
*/

Q2931_IE *get_Q2931_IE(WSABUF *buf, Q2931_IE_TYPE type) {
   unsigned long i = 0;
   Q2931_IE *ie_ptr;
   
   ie_ptr = (Q2931_IE *) buf->buf;
   
	while( i < buf->len ) {
	   if(ie_ptr->IEType == type)
         return ie_ptr;
      i = i + ie_ptr->IELength;
      ie_ptr = (Q2931_IE *) ((char *)ie_ptr + ie_ptr->IELength);
   }
   return NULL;
}

int evaluate_QOS (LPQOS qos) {
   unsigned long i;   

   WSABUF *ProviderSpecific;
   Q2931_IE *ie_ptr;
   AAL_PARAMETERS_IE ie_aalparams;
   ATM_TRAFFIC_DESCRIPTOR_IE ie_td;
   ATM_BROADBAND_BEARER_CAPABILITY_IE ie_bbc;
   ATM_BLLI_IE ie_blli;
   ATM_BHLI_IE ie_bhli;
   ATM_CALLED_PARTY_NUMBER_IE ie_called_party_nr;
   ATM_CALLED_PARTY_SUBADDRESS_IE ie_called_party_subaddr;
   ATM_CALLING_PARTY_NUMBER_IE ie_calling_party_nr;
   ATM_CALLING_PARTY_SUBADDRESS_IE ie_calling_party_subaddr;
   ATM_CAUSE_IE ie_cause;
   ATM_QOS_CLASS_IE ie_qos;
   ATM_TRANSIT_NETWORK_SELECTION_IE ie_transit_netw_sel;
	
   printf("==== QOS BEGIN ====\n");
   print_flowspec(qos);

   ProviderSpecific = &qos->ProviderSpecific;
   ie_ptr = (Q2931_IE *)ProviderSpecific->buf;
   i = 0;
   printf("++++PROVIDER SPECIFIC++++\n");
   while( i < ProviderSpecific->len ) {
      switch(ie_ptr->IEType) {
      case (IE_AALParameters):
         memcpy(&ie_aalparams, ie_ptr->IE, sizeof(AAL_PARAMETERS_IE));
         print_ie_aalparams(ie_aalparams);
         break;
      case (IE_TrafficDescriptor):
         memcpy(&ie_td, ie_ptr->IE, sizeof(ATM_TRAFFIC_DESCRIPTOR_IE));
         print_ie_td(ie_td);
         break;
      case (IE_BroadbandBearerCapability):
         memcpy(&ie_bbc, ie_ptr->IE, sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE));
         print_ie_bbc(ie_bbc);
         break;
      case (IE_BHLI):
         memcpy(&ie_bhli, ie_ptr->IE, sizeof(ATM_BHLI_IE));
         printf("ATM_BHLI_IE\n");
         break;
      case (IE_BLLI):
         memcpy(&ie_blli, ie_ptr->IE, sizeof(ATM_BLLI_IE));
         printf("IE_BLLI\n");
         break;
      case (IE_CalledPartyNumber):
         memcpy(&ie_called_party_nr, ie_ptr->IE, sizeof(ATM_CALLED_PARTY_NUMBER_IE));
         printf("IE_CalledPartyNumber\n");
         break;
      case (IE_CalledPartySubaddress):
         memcpy(&ie_called_party_subaddr, ie_ptr->IE, sizeof(ATM_CALLED_PARTY_SUBADDRESS_IE));
         printf("IE_CalledPartySubaddress\n");
         break;
      case (IE_CallingPartyNumber):
         memcpy(&ie_calling_party_nr, ie_ptr->IE, sizeof(ATM_CALLING_PARTY_NUMBER_IE));
         printf("IE_CallingPartyNumber\n");
         break;
      case (IE_CallingPartySubaddress):
         memcpy(&ie_calling_party_subaddr, ie_ptr->IE, sizeof(ATM_CALLING_PARTY_SUBADDRESS_IE));
         printf("IE_CallingPartySubaddress\n");
         break;
      case (IE_Cause):
         memcpy(&ie_cause, ie_ptr->IE, sizeof(ATM_CAUSE_IE));
         print_ie_cause(ie_cause);
         break;
      case (IE_QOSClass):
         memcpy(&ie_qos, ie_ptr->IE, sizeof(ATM_QOS_CLASS_IE));
         print_ie_qos(ie_qos);
         break;
      case (IE_TransitNetworkSelection):
         memcpy(&ie_transit_netw_sel, ie_ptr->IE, sizeof(ATM_TRANSIT_NETWORK_SELECTION_IE));
         printf("ATM_TRANSIT_NETWORK_SELECTION_IE\n");
         break;
      default:
         print_ie_unknown(ie_ptr);
         break;
      }
      i = i + ie_ptr->IELength;
      ie_ptr = (Q2931_IE *) ((char *)ie_ptr + ie_ptr->IELength);
   } /* while (ie_ptr) */

   printf("==== QOS END ====\n");

   return 0;
}

void print_flowspec(LPQOS qos) {
printf("\
SENDING FLOWSPEC\n\
  TokenRate          %d Bytes/sec\n\
  TokenBucketSize    %d Bytes\n\
  PeakBandwidth      %d Bytes/sec\n\
  Latency            %d microseconds\n\
  DelayVariation     %d microseconds\n\
  ServiceType        %d \n\
  MaxSduSize         %d Bytes\n\
  MinimumPolicedSize %d Bytes\n\
RECEIVING FLOWSPEC   \n\
  TokenRate          %d Bytes/sec\n\
  TokenBucketSize    %d Bytes\n\
  PeakBandwidth      %d Bytes/sec\n\
  Latency            %d microseconds\n\
  DelayVariation     %d microseconds\n\
  ServiceType        %d \n\
  MaxSduSize         %d Bytes\n\
  MinimumPolicedSize %d Bytes\n",
qos->SendingFlowspec.TokenRate,
qos->SendingFlowspec.TokenBucketSize,
qos->SendingFlowspec.PeakBandwidth,
qos->SendingFlowspec.Latency,
qos->SendingFlowspec.DelayVariation,
qos->SendingFlowspec.ServiceType,
qos->SendingFlowspec.MaxSduSize,
qos->SendingFlowspec.MinimumPolicedSize,
qos->ReceivingFlowspec.TokenRate,
qos->ReceivingFlowspec.TokenBucketSize,
qos->ReceivingFlowspec.PeakBandwidth,
qos->ReceivingFlowspec.Latency,
qos->ReceivingFlowspec.DelayVariation,
qos->ReceivingFlowspec.ServiceType,
qos->ReceivingFlowspec.MaxSduSize,
qos->ReceivingFlowspec.MinimumPolicedSize);
}

void print_ie_aalparams(AAL_PARAMETERS_IE ie_aalparams) {
char sSSCSType[64];
char sMode[64];
   switch(ie_aalparams.AALSpecificParameters.AAL5Parameters.SSCSType) {
   case 0x00: sprintf(sSSCSType,"AAL5_SSCS_NULL(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.SSCSType); break;
   case 0x01: sprintf(sSSCSType,"AAL5_SSCS_SSCOP_ASSURED(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.SSCSType); break;
   case 0x02: sprintf(sSSCSType,"AAL5_SSCS_SSCOP_NON_ASSURED(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.SSCSType); break;
   case 0x04: sprintf(sSSCSType,"AAL5_SSCS_FRAME_RELAY(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.SSCSType); break;
   default: sprintf(sSSCSType,"_UNKNOWN(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.SSCSType); break; 
   }
   switch(ie_aalparams.AALSpecificParameters.AAL5Parameters.Mode) {
   case 0x00: sprintf(sMode,"AAL5_MODE_MESSAGE(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.Mode); break;
   case 0x01: sprintf(sMode,"AAL5_MODE_STREAMING(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.Mode); break;
   default: sprintf(sMode,"_UNKNOWN(%d)",ie_aalparams.AALSpecificParameters.AAL5Parameters.Mode); break; 
   }
printf("\
ie_aalparams\n\
  type                   %d\n\
  AALSpecificParameters    \n\
    ForwardMaxCPCSSDUSize  %d\n\
    BackwardMaxCPCSSDUSize %d\n\
	Mode                   %s\n\
	SSCSType               %s\n",
ie_aalparams.AALType,
ie_aalparams.AALSpecificParameters.AAL5Parameters.ForwardMaxCPCSSDUSize,
ie_aalparams.AALSpecificParameters.AAL5Parameters.BackwardMaxCPCSSDUSize,
sMode,
sSSCSType);
}

void print_ie_td(ATM_TRAFFIC_DESCRIPTOR_IE ie_td) {
printf("\
ATM_TRAFFIC_DESCRIPTOR_IE\n\
  Forward\n\
    PeakCellRate_CLP0         %d\n\
    PeakCellRate_CLP01        %d\n\
    SustainableCellRate_CLP0  %d\n\
    SustainableCellRate_CLP01 %d\n\
    MaxBurstSize_CLP0         %d\n\
    MaxBurstSize_CLP01        %d\n\
    Tagging                   %d\n\
  Backward\n\
    PeakCellRate_CLP0         %d\n\
    PeakCellRate_CLP01        %d\n\
    SustainableCellRate_CLP0  %d\n\
    SustainableCellRate_CLP01 %d\n\
    MaxBurstSize_CLP0         %d\n\
    MaxBurstSize_CLP01        %d\n\
    Tagging                   %d\n\
  BestEffort                  %d\n",
ie_td.Forward.PeakCellRate_CLP0,
ie_td.Forward.PeakCellRate_CLP01,
ie_td.Forward.SustainableCellRate_CLP0,
ie_td.Forward.SustainableCellRate_CLP01,
ie_td.Forward.MaxBurstSize_CLP0,
ie_td.Forward.MaxBurstSize_CLP01,
ie_td.Forward.Tagging,
ie_td.Backward.PeakCellRate_CLP0,
ie_td.Backward.PeakCellRate_CLP01,
ie_td.Backward.SustainableCellRate_CLP0,
ie_td.Backward.SustainableCellRate_CLP01,
ie_td.Backward.MaxBurstSize_CLP0,
ie_td.Backward.MaxBurstSize_CLP01,
ie_td.Backward.Tagging,
ie_td.BestEffort);
}

void print_ie_bbc(ATM_BROADBAND_BEARER_CAPABILITY_IE ie_bbc) {
   char sBClass[64];
   char sTType[64];
   char sTReqs[64];
   char sClipping[64];
   char sUserPlane[64];

   switch(ie_bbc.BearerClass) {
   case 0x01: sprintf(sBClass,"CCOB_A(%d)",ie_bbc.BearerClass); break;
   case 0x03: sprintf(sBClass,"CCOB_C(%d)",ie_bbc.BearerClass); break;
   case 0x10: sprintf(sBClass,"CCOB_X(%d)",ie_bbc.BearerClass); break;
   default: sprintf(sBClass,"_UNKNOWN(%d)",ie_bbc.BearerClass); break; 
   }
   switch(ie_bbc.TrafficType) {
   case 0x00: sprintf(sTType,"TT_NOIND(%d)",ie_bbc.TrafficType); break;
   case 0x04: sprintf(sTType,"TT_CBR(%d)",ie_bbc.TrafficType); break;
   case 0x06: sprintf(sTType,"TT_VBR(%d)",ie_bbc.TrafficType);  break;
   default:   sprintf(sTType,"_UNKNOWN(%d)",ie_bbc.TrafficType); break;
   }
   switch(ie_bbc.TimingRequirements) {
   case 0x00: sprintf(sTReqs,"TR_NOIND(%d)",ie_bbc.TimingRequirements); break;
   case 0x01: sprintf(sTReqs,"TR_END_TO_END(%d)",ie_bbc.TimingRequirements); break;
   case 0x02: sprintf(sTReqs,"TR_NO_END_TO_END(%d)",ie_bbc.TimingRequirements);  break;
   default:   sprintf(sTReqs,"_UNKNOWN(%d)",ie_bbc.TimingRequirements); break;
   }
   switch(ie_bbc.ClippingSusceptability) {
   case 0x00: sprintf(sClipping, "CLIP_NOT(%d)",ie_bbc.ClippingSusceptability); break;
   case 0x20: sprintf(sClipping, "CLIP_SUS(%d)",ie_bbc.ClippingSusceptability); break;
   default: sprintf(sClipping, "_UNKNOWN(%d)",ie_bbc.ClippingSusceptability); break;
   }
   switch(ie_bbc.UserPlaneConnectionConfig) {
   case 0x00: sprintf(sUserPlane, "UP_P2P(%d)",ie_bbc.UserPlaneConnectionConfig); break;
   case 0x01: sprintf(sUserPlane, "UP_P2MP(%d)",ie_bbc.UserPlaneConnectionConfig); break;
   default: sprintf(sUserPlane, "_UNKNOWN(%d)",ie_bbc.UserPlaneConnectionConfig); break;
   }


printf("\
ATM_BROADBAND_BEARER_CAPABILITY_IE\n\
   BearerClass               %s\n\
   TrafficType               %s\n\
   TimingRequirements        %s\n\
   ClippingSusceptability    %s\n\
   UserPlaneConnectionConfig %s\n",
sBClass,
sTType,
sTReqs,
sClipping,
sUserPlane);
}

void print_ie_cause(ATM_CAUSE_IE ie_cause) {
printf("\
ATM_CAUSE_IE\n\
  Location           %d\n\
  Cause              %d\n\
  DiagnosticsLength  %d\n\
  Diagnostics          \n\
   Condition portion %d\n\
   Rejection Reason  %d\n\
   P-U flag          %d\n\
   N-A flag          %d\n",
ie_cause.Location,
ie_cause.Cause,
ie_cause.DiagnosticsLength,
ie_cause.Diagnostics[0],
ie_cause.Diagnostics[1],
ie_cause.Diagnostics[2],
ie_cause.Diagnostics[3]);
}

void print_ie_qos(ATM_QOS_CLASS_IE ie_qos) {
printf("\
ATM_QOS_CLASS_IE\n\
  QOSClassForward  %d\n\
  QOSClassBackward %d\n",
ie_qos.QOSClassForward,
ie_qos.QOSClassBackward);
}

void print_ie_unknown(Q2931_IE *ie_ptr) {
printf("\
ATM_UNKNOWN_IE\n\
  IELength  %d\n\
  IEType    %x\n",
ie_ptr->IELength,
ie_ptr->IEType);
}

#endif /* WIN32 */


/* return value: 
= 0: OK
< 0: ERROR
*/
int atm_setMaxSduSize (QOS *bqos, unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize) 
{
   
#ifdef WIN32
   Q2931_IE *ie_ptr;
   AAL_PARAMETERS_IE *ie_aal;
   unsigned long rcv_prvd, snd_prvd, rcv_user, snd_user;

   ie_ptr = get_Q2931_IE(&bqos->ProviderSpecific, IE_AALParameters);

   rcv_prvd = 0xFFFFFFFF; /* not specified */
   snd_prvd = 0xFFFFFFFF; /* not specified */
   
   rcv_user = 0xFFFFFFFF; /* not specified */
   snd_user = 0xFFFFFFFF; /* not specified */

   if( ie_ptr ) {
      ie_aal = (AAL_PARAMETERS_IE *) ie_ptr->IE;
      if( ie_aal->AALType == AALTYPE_5 ) {
         rcv_prvd = ie_aal->AALSpecificParameters.AAL5Parameters.ForwardMaxCPCSSDUSize;
         snd_prvd = ie_aal->AALSpecificParameters.AAL5Parameters.BackwardMaxCPCSSDUSize;
      }
   }
   
   if( bqos ) {
      rcv_user = bqos->ReceivingFlowspec.MaxSduSize;
      snd_user = bqos->SendingFlowspec.MaxSduSize;
   }

   /* actually, the maximum allowed SDU size is given by service provider */
   *rcvMaxSduSize = (rcv_user < rcv_prvd) ? rcv_user : rcv_prvd;
   *sndMaxSduSize = (snd_user < snd_prvd) ? snd_user : snd_prvd;
#else
   /* XXX for Linux here */
   if( bqos ) {
      *sndMaxSduSize = bqos->txtp.max_sdu;
      *rcvMaxSduSize = bqos->rxtp.max_sdu;
   }
#endif

   /* check if we have correct values */
   if( *rcvMaxSduSize == 0 || *rcvMaxSduSize == 0xFFFFFFFF )
      *rcvMaxSduSize = DEFAULT_MAX_SDU_SIZE;
   if( *sndMaxSduSize == 0 || *sndMaxSduSize == 0xFFFFFFFF )
      *sndMaxSduSize = DEFAULT_MAX_SDU_SIZE;

   *rcvMaxSduSize = DEFAULT_MAX_SDU_SIZE;
   *rcvMaxSduSize = DEFAULT_MAX_SDU_SIZE;

   return 0;
}
