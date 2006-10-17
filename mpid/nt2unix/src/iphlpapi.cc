/***************************************************************************
                          iphlpapi.cc  -  description
                             -------------------
    begin                : Don Aug 15 2002
    copyright            : (C) 2002 by silke
    email                : silke@oskar
 ***************************************************************************/

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nt2unix.h"
#include "iphlpapi.h"

DWORD
WINAPI
GetIfEntry(
    IN OUT PMIB_IFROW   pIfRow
    )
{
/*
 #warning "GetIfEntry is still a dummy, but returns a structure which containes sample data :-)"
 */
    int index;


    /* index of device to query */
    index = pIfRow->dwIndex;
//    printf("called GetIfEntry for interface %d \n",index);

    /* name of the device */
    /* strcpy(pIfRow->wszName, "dummy-device"); */
    
    /* device type */
    /* 
       one of:
       MIB_IF_TYPE_OTHER 1
       MIB_IF_TYPE_ETHERNET 6
       MIB_IF_TYPE_TOKENRING 9
       MIB_IF_TYPE_FDDI 15
       MIB_IF_TYPE_PPP 23
       MIB_IF_TYPE_LOOPBACK 24
       MIB_IF_TYPE_SLIP 28
    */
    pIfRow->dwType = 6; /* MIB_IF_TYPE_ETHERNET */

/* Maximum Transmission unit */
    pIfRow->dwMtu = 1500;
    
    /* Interfacespeed in bits per second */
    pIfRow->dwSpeed = 100 * 1024 * 1024;
    
    /* length of phys address */
    pIfRow->dwPhysAddrLen = 8;

    /* physical address of device */
    pIfRow->bPhysAddr;
    
    /* Specifies if the interface is administrativly enabled or disabled */
    pIfRow->dwAdminStatus = 1;
    
    /* Operational Status of the interface */
    /* 
       one of:
       MIB_IF_OPER_STATUS_NON_OPERATIONAL
       MIB_IF_OPER_STATUS_UNREACHABLE
       MIB_IF_OPER_STATUS_DISCONNECTED
       MIB_IF_OPER_STATUS_CONNECTING
       MIB_IF_OPER_STATUS_CONNECTED
       MIB_IF_OPER_STATUS_OPERATIONAL
    */
    pIfRow->dwOperStatus = 0 /* MIB_IF_OPER_STATUS_OPERATIONAL/* dunno, no info found */;

    /* Last time the oprerational status has changed */
    pIfRow->dwLastChange = 1;
    
    /* Number of octets of data received through interface */
    pIfRow->dwInOctets = 1;

    /* Number of unicast packets received through interface */
    pIfRow->dwInUcastPkts = 1;

    /* Number of non-unicast packets received through interface */
    pIfRow->dwInNUcastPkts = 1;

    /* Number of incoming packets which have been discarded */
    pIfRow->dwInDiscards = 0;

    /* Number of incoming packets which have been discarded because of errors */ 
    pIfRow->dwInErrors = 0;

    /* Number of incoming packets which have been discarded because the protocol 
       was unknown */
    pIfRow->dwInUnknownProtos = 0;

    /* Number of octets of data sent through interface */
    pIfRow->dwOutOctets = 1;

    /* Number of unicast packets sent through interface */
    pIfRow->dwOutUcastPkts = 1;

    /* Number of non-unicast packets sent through interface */
    pIfRow->dwOutNUcastPkts = 1;

    /* Number of outgoing packets which have been discarded */
    pIfRow->dwOutDiscards = 0;

    /* Number of outgoing packets which have been discarded because of errors */ 
    pIfRow->dwOutErrors = 0;

    /* Number of ouput queue length */
    pIfRow->dwOutQLen = 10;
    
    /* length of bDescr member (in what?)*/
    pIfRow->dwDescrLen = 16;
    
    /* Contains a description of the interface */
    pIfRow->bDescr;
    
    return (0);
}

DWORD
WINAPI
GetIpAddrTable(
    OUT    PMIB_IPADDRTABLE pIpAddrTable,
    IN OUT PULONG          pdwSize,
    IN     BOOL             bOrder
    )
/*
 #warning GetIpAddrTable is not fully implemented, only the default ip is returned as one-entry-table
 */
{
    struct hostent *pHEnt;
    char szHostName[256];	
    /* static MIB_IPADDRTABLE ipaddrtable; */
    struct in_addr in;

//printf("called GetIpAddrTable\n");

    if (*pdwSize < sizeof(MIB_IPADDRTABLE)) {
	*pdwSize = sizeof(MIB_IPADDRTABLE);
	return(1);
    }
    
    if (gethostname(szHostName, 255))
	return(1);
    

    pHEnt = gethostbyname(szHostName);
    memcpy(&in.s_addr,*pHEnt->h_addr_list, sizeof (in.s_addr));
    
    pIpAddrTable->dwNumEntries = pHEnt->h_length;
    
    pIpAddrTable->dwNumEntries = 1;
    pIpAddrTable->table[0].dwAddr = in.s_addr;
/*
 #warning hard coded netmask 255.255.255.0 is assumed
 */
    pIpAddrTable->table[0].dwMask = 0xffffff00;
    pIpAddrTable->table[0].dwBCastAddr = 
	pIpAddrTable->table[0].dwAddr & pIpAddrTable->table[0].dwMask
	+ 0xffffffff ^ pIpAddrTable->table[0].dwMask;
    pIpAddrTable->table[0].dwReasmSize = 1;
    /* printf("%u\n", pIpAddrTable->table[0].dwAddr); */
       
    return(0);
}


	
	
