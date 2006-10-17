/* $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de
 *
 * this functions provide simple lists for metahosts and routers
 *     
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rhlist.h"
#include "rdebug.h"

#include "newstrings.h"

int nummetahosts;
struct SMetahostListEntry *metahostlist;
int numrouters;
struct SRouterListEntry *routerlist;


const  char *nicTypeStrings[]={"ADR_TCP","ADR_ATM_SVC","ADR_ATM_PVC"};
const  char *deviceTypeStrings[]={"null", "ch_smi", "ch_shmem", "ch_usock", "ch_gm", "ch_p4", "ch_mpx"};


int metahostlist_maxsize;
int routerlist_maxsize;

void rh_initLists(){
    metahostlist= (struct SMetahostListEntry *) malloc( METAHOST_LIST_SIZE * sizeof(struct SMetahostListEntry));
    routerlist= (struct SRouterListEntry *) malloc( ROUTER_LIST_SIZE * sizeof(struct SRouterListEntry));
    nummetahosts=0;
    numrouters=0;
    metahostlist_maxsize= METAHOST_LIST_SIZE;
    routerlist_maxsize= ROUTER_LIST_SIZE;
}


void rh_freeLists() {
    free( metahostlist );
    free( routerlist );
}

int rh_addMetahost(char* name, int numproc){    
    if ( nummetahosts == metahostlist_maxsize) {
	struct SMetahostListEntry *tmp;
        RDEBUG("rh_addMetahost: metahostlist full\n");
	tmp = realloc (metahostlist, (metahostlist_maxsize + METAHOST_LIST_SIZE) * sizeof(struct SMetahostListEntry));
	if (!tmp) {
		fprintf(stderr, "realloc failed in rhlist.c\n");
		exit (-1);
	}
	metahostlist_maxsize += METAHOST_LIST_SIZE;
    }
    
    strcpy(metahostlist[nummetahosts].hostname, name);
    metahostlist[nummetahosts].np=numproc;
    metahostlist[nummetahosts].num_of_routers=0;
    metahostlist[nummetahosts].nodeList=0;
    metahostlist[nummetahosts].deviceType=0;
    metahostlist[nummetahosts].devOptions=0;
    metahostlist[nummetahosts].execPath=0;
    metahostlist[nummetahosts].frontend=0;
    metahostlist[nummetahosts].mpiRoot=0;
    metahostlist[nummetahosts].envFile=0;
    metahostlist[nummetahosts].appArgs=0;
    metahostlist[nummetahosts].valid=0;
    metahostlist[nummetahosts].procgroup=0;
    metahostlist[nummetahosts].routerExec=0;
    nummetahosts++;
    return nummetahosts-1;
}

int rh_getMetahostId(char* hostname) {
    int id;
    for (id=0; id < nummetahosts; id++) 
	if (strcmp(hostname, metahostlist[id].hostname)==0) 
	    return id;
    return -1;
}

int rh_getNumRouters() { return numrouters; }

int rh_getNumMetahosts() { return nummetahosts; }

int rh_getRouter(int id) {
    int router;
    
    for (router=0; router < numrouters; router++) {
	    if (routerlist[router].localRouterRank == id) 
	        return router;
	}
    return -1;
}

int rh_addRouter(int host, int id, Snic *lnics, Snic *rnics, int port) {
    if (numrouters == ROUTER_LIST_SIZE) {
    	struct SRouterListEntry *tmp;
        RDEBUG("rh_addMetahost: metahostlist full\n");
	tmp = realloc (metahostlist, (metahostlist_maxsize + METAHOST_LIST_SIZE) * sizeof(struct SMetahostListEntry));
	if (!tmp) {
		fprintf(stderr, "realloc failed in rhlist.c\n");
		exit (-1);
	}
	metahostlist_maxsize += METAHOST_LIST_SIZE;

        RDEBUG("rh_addRouter: routerlist full\n");
        return -1;
    }
    if ( host >= nummetahosts) {
        RDEBUG("rh_addRouter: unknown host handle\n");
        return -1;
    }

    /*   metahostlist[host].num_of_routers++;*/
    
    routerlist[numrouters].host = host;
    routerlist[numrouters].sockfd = -1;
    routerlist[numrouters].port = port;
    routerlist[numrouters].localRouterRank = id;
    routerlist[numrouters].localNicList=lnics;
    routerlist[numrouters].remoteNicList=rnics;
    routerlist[numrouters].nodeName=0;
    routerlist[numrouters].routerExec=0;
    numrouters++;
    return numrouters-1;
}

void rh_printMetahostList(){
    int h;

    printf("====== metahostlist =======\n");
    for (h=0; h<nummetahosts; h++) {
	printf("Host %d\tName=%s procs=%d routers=%d type=%s ", h, metahostlist[h].hostname, 
		metahostlist[h].np,
		metahostlist[h].num_of_routers,
		deviceTypeStrings[metahostlist[h].deviceType]);
	if ( metahostlist[h].mpiRoot &&  metahostlist[h].envFile ) {
	    printf(" mpiRoot=%s envFile=%s\n\t", metahostlist[h].mpiRoot, metahostlist[h].envFile );
	}
	if (metahostlist[h].execPath && metahostlist[h].execName && metahostlist[h].appArgs){
	    printf("execPath=%s execName=%s execArgs=%s", metahostlist[h].execPath, 
		metahostlist[h].execName, metahostlist[h].appArgs);
	}
	printf("\n");
	if (metahostlist[h].nodeList) {
	    Snode * pnode=metahostlist[h].nodeList;
	    printf("node list:\n"); 
	    while (pnode) {
		printf("    %s (np=%d/maxnp=%d/numRouters=%d/extra=%d) ",pnode->nodeName, pnode->np, pnode->maxNumProcs, pnode->numRouters, pnode->npExtraProcs);
		if (pnode->numRouters) {
		    int i;
		    printf(" router ids=(");
		    for (i=0; i< pnode->numRouters; i++) {
			if (i != 0) printf(",");
			printf("%d", pnode->routerIds[i]);
		    }
		    printf(") ");
		}
		if (pnode->nicList) {
		    Snic *pnic=pnode->nicList;
		    printf(" nics=(%s:%d", pnic->nicAddress, pnic->port );
		    pnic=pnic->next;

		    while ( pnic ) {
		      printf(",%s:%d", pnic->nicAddress, pnic->port );
			pnic=pnic->next;
		    }
		    printf(")");
		}
		if (pnode->numNets > 0) {
		  int i=0;
		  printf(" networks=(%d",pnode->netList[i]);
		  i++;
		  while (i<pnode->numNets) {
		    printf(",%d", pnode->netList[i]);
		    i++;
		  }
		  printf(")");
		}
		if (pnode->executable ) {
		  printf(" executable=%s ",pnode->executable);
		}
		if (pnode->args) {
		  printf(" arguments=%s ",pnode->args);
		}
		if (pnode->npExtraProcs) {
			int i;
			printf("\n\textra procs=(");
			for (i=0; pnode->extraProcList[i] != 0; i++) {
				printf("\"%s\"", pnode->extraProcList[i]);
				if (pnode->extraProcList[i+1] != 0)
					printf(",\n");
			}
			printf(")\n");
		}
		printf("\n");
		pnode=pnode->next;
	    }
	    printf("\n");
	}
    }
}

void rh_printRouterList(){
    int r,i;
    Snic *rnic,*lnic;    
    for (r=0; r<numrouters; r++) {
	printf("router %d on node %s with metahostrank %d", r, routerlist[r].nodeName, routerlist[r].metahostrank );
	if (routerlist[r].routerExec) {
		printf(" executable %s", routerlist[r].routerExec);
	}
	printf(" target metahost=%s serv=%d id=%d\n", metahostlist[routerlist[r].host].hostname, routerlist[r].serv, routerlist[r].localRouterRank);  
	rnic = routerlist[r].remoteNicList; i=1;
	lnic = routerlist[r].localNicList;
	while (lnic) {
	    printf("conn %d: type %s address %s port %d ",  i, nicTypeStrings[lnic->nicType] ,lnic->nicAddress,lnic->port);
	    printf("--> type %s address %s port %d\n", nicTypeStrings[rnic->nicType],rnic->nicAddress,rnic->port);
	    i++; rnic=rnic->next; lnic=lnic->next;	    
	}
    }
}


Snode * rh_findNodeName(Snode *nl, char * name) {
    while (nl) {
   	if ( strcmp(nl->nodeName, name) ==0 )
	    break;
  	nl=nl->next;
    }
    return nl;
}

Snode * rh_findNicInHosts( Snode *nl, Snic * nics) {
    Snode * foundNode=0;
    while (nl && !foundNode) {
	Snic* pnic;

	pnic=nl->nicList;
	while (pnic && !foundNode ) {
	    if ( strcmp(pnic->nicAddress,nics->nicAddress) ==0)
		foundNode=nl;
	    pnic=pnic->next;
	}
	nl=nl->next;
    }
    return foundNode;
}

/* make a copy of a Snic */
Snic* rh_copyNic(Snic* pnic) {
    Snic *nicTEMP;

    nicTEMP = (Snic *)malloc( sizeof(Snic) );
    strcpy (nicTEMP->nicAddress, 
	    pnic->nicAddress);
    nicTEMP->next = 0;
    nicTEMP->nodeName = newString(pnic->nodeName);

    nicTEMP->port = pnic->port;
    nicTEMP->nicType = pnic->nicType;
    
    return nicTEMP;  
}
/* allocates memory for a new Snic, initializes it with the given parameters
   and returns a pointer to it */
Snic *rh_newNic( char *adr, int port, EnicType type )
{
    Snic *nicTEMP;

    nicTEMP = (Snic *)malloc( sizeof(Snic) );
    if (adr)
    	strcpy( nicTEMP->nicAddress, adr );
    else
    	strcpy( nicTEMP->nicAddress, "" );
    nicTEMP->next = 0;
    nicTEMP->port = port;
    nicTEMP->nicType = type;
    nicTEMP->nodeName=0;
    
    return nicTEMP;
}
Snic *rh_newNicClear()
{
    Snic *nicTEMP;

    nicTEMP = (Snic *)malloc( sizeof(Snic) );
    strcpy( nicTEMP->nicAddress, "" );
    nicTEMP->next = 0;
    nicTEMP->port = 0;
    nicTEMP->nicType = 0;
    nicTEMP->nodeName=0;
        
    return nicTEMP;
}
Snode *rh_newNodeClear()
{
	Snode* nodeTEMP;
	
	nodeTEMP = (Snode *)malloc( sizeof(Snode) );
	nodeTEMP->nodeName=0;
	nodeTEMP->nicList = 0;
	nodeTEMP->maxNumProcs=0;
	nodeTEMP->np = 0;
	nodeTEMP->numRouters = 0;
	nodeTEMP->netList=0;
	nodeTEMP->numNets=0;
	nodeTEMP->executable=0;
	nodeTEMP->args=0;
	nodeTEMP->npExtraProcs=0;
	nodeTEMP->extraProcList=0;
	nodeTEMP->npFixed=0;
	nodeTEMP->npSum=0;
	
	return nodeTEMP;
}
