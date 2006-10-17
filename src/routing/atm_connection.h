/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */
#ifndef __ATM_CONNECTION_H
#define __ATM_CONNECTION_H
#include "conn_common.h"
#ifdef WIN32
#include <winsock2.h>
#include <ws2atm.h>
#define PF_ATMSVC AF_ATM
#define PF_ATMPVC AF_MAX+1
#define ATM_ESA_LEN ATM_ADDR_SIZE
#define sockaddr_atmsvc sockaddr_atm

#else
#include "/usr/local/include/atm.h"
typedef struct sockaddr_atmpvc ATM_PVC_PARAMS;
typedef struct atm_qos QOS;
#endif

/* adapt this value to your network provider */
#define DEFAULT_MAX_SDU_SIZE 8192

#include "rdebug.h"

#ifdef bla
#ifndef CONN_SERVER
#define CONN_SERVER 1
#endif

#ifndef CONN_CLIENT
#define CONN_CLIENT 0
#endif
#endif
/* max wait time in microsecs for last try to recv() */
#define RECV_MAX_WAIT 4000000 /* 4 seconds */

/* error codes */
#define ERR_ATM_CONN_BIND       -6
#define ERR_ATM_CONN_LISTEN     -7
#define ERR_ATM_CONN_ACCEPT     -8
#define ERR_ATM_CONN_CONNECT    -9
#define ERR_ATM_CONN_SOCKET     -10
#define ERR_ATM_CONN_SETSOCKOPT -11

int atm_establish_svc_sock_conn(int *localsocket_fd, struct sockaddr *loc_addr, 
                                int *remotesocket_fd, struct sockaddr *rem_addr,
                                char *qos,
                                unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize,
                                int max_nconn, int serv, int timeout);
int atm_establish_pvc_sock_conn(int *localsocket_fd, struct sockaddr *loc_addr, 
                                int *remotesocket_fd, struct sockaddr *rem_addr, 
                                char *qos,
                                unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize,
                                int max_nconn, int serv, int timeout);
int atm_add_socket_pair (int *localsocket, int *remotesocket, 
                         unsigned short sa_family, int serv);
int atm_read(void *buffer, unsigned size, int fd, unsigned long maxSDUsize);
int atm_write(void *buffer, unsigned size, int fd, unsigned long maxSDUsize);
int atm_setMaxSduSize (QOS *bqos, unsigned long *rcvMaxSduSize, unsigned long *sndMaxSduSize);

#ifdef WIN32
int evaluate_QOS (LPQOS ProviderSpecific);
int text2qos (const char *text, QOS *bqos, int flags);
int text2atm (char *text, struct sockaddr *caddr, int length, int flags);
int CALLBACK ConditionFunc(
                           IN LPWSABUF lpCallerId,
                           IN LPWSABUF lpCallerData,
                           IN OUT LPQOS lpSQOS,
                           IN OUT LPQOS lpGQOS,
                           IN LPWSABUF lpCalleeId,
                           OUT LPWSABUF lpCalleeData,
                           OUT GROUP FAR * g,
                           IN DWORD dwCallbackData
                           );
void print_flowspec(LPQOS qos);
void print_ie_aalparams(AAL_PARAMETERS_IE ie_aalparams);
void print_ie_td(ATM_TRAFFIC_DESCRIPTOR_IE ie_td);
void print_ie_bbc(ATM_BROADBAND_BEARER_CAPABILITY_IE ie_bbc);
void print_ie_cause(ATM_CAUSE_IE ie_cause);
void print_ie_qos(ATM_QOS_CLASS_IE ie_qos);
void print_ie_unknown(Q2931_IE *ie_ptr);


#endif

#endif
