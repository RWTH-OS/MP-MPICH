/* $Id$ 
 * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe email: mpoeppe@gmx.de 
 */

#include <netdb.h>
#include <sys/socket.h>

#include "rdebug.h"
#include "rhlist.h"
#include "router_config.h"
 
#include "newstrings.h"
#include "auto_router.h" 
 /*
 * auto_router tries to configure the routers in a simple automatic way.
 * We need one NIC defined for each metahost or valid names for an dns lookup. 
 */

 int auto_router(EautoAutoOption e_AutoOption) {
  int imetahost, jmetahost, i, j;
  Snic ***mh_nics;
  Snode *pnode;
  int routerCounter;

	/* if there is a secondary device defined, there is no need for a router */ 
  if (pars_rconf->secondaryDevice != DEVICE_NULL) {
    	for (i = 0; i < pars_rconf->nbr_metahosts; i++) {
    		for (j = 0; j < pars_rconf->nbr_metahosts; j++) {
  				pars_rconf->useRouterFromTo[i][j] = 0;
    		}
			pars_rconf->nrp_metahost[i] = 0;
			pars_rconf->my_nrp[i] = 0;
    	}
		pars_rconf->nrp = 0;
		return 1;
  } 

  mh_nics= (Snic***) calloc(pars_rconf->nbr_metahosts, sizeof(Snic**));

  /* first, we have to find nbr_metahosts-1 nics for every meta host */
  for (imetahost =0; imetahost < pars_rconf->nbr_metahosts; imetahost ++) {
    int i;
    int nbr_metahosts=pars_rconf->nbr_metahosts;

    mh_nics[imetahost]= (Snic**) malloc(sizeof(Snic**) * nbr_metahosts);
    pnode=metahostlist[imetahost].nodeList;

    i=0;
    if ( e_AutoOption != AR_ONLY_DNS ) {
        while(pnode && i < nbr_metahosts-1 /* need one nic for each other */) {
           if (pnode->nicList) {
                Snic *pnic=pnode->nicList;
                while (pnic && i < nbr_metahosts) {
	               /* we have to copy the nic, because the next pointer has to be 0
	               for the router entry */
	               mh_nics[imetahost][i]=rh_copyNic(pnic);
	               mh_nics[imetahost][i]->nodeName=newString(pnode->nodeName);
	               i++;    
	               pnic=pnic->next;
                }
           }
           pnode=pnode->next;
           /* if we don't use DNS and have found 1 nic at last, we have to use the nics round robin */
           if (!pnode && e_AutoOption == AR_NO_DNS && i > 0 ) 
                pnode=metahostlist[imetahost].nodeList;
        } /* while */
        if ( i == 0 && e_AutoOption == AR_NO_DNS) {
           /* no nics found and no DNS allowed - lets end here! */
	       RERROR("AUTO_ROUTER NO_DNS: no nics found and no DNS allowed - cannot make router connections!\n");
	   return -1;
        }
    }
    /* check what we have found */
    if ( i < nbr_metahosts-1 ) {
      int portOffset=1; /* this is for duplicated nics */
      int inodes;
      /* try to use the nodes round robin */
      pnode=metahostlist[imetahost].nodeList;
      for (inodes=0; inodes < nbr_metahosts - 1 - i; inodes++) {
	Snic *nicTEMP;
	struct in_addr intmp;
	struct hostent *he= gethostbyname( pnode->nodeName );  
	
	nicTEMP = rh_newNic(0,0, ADR_TCP);
	nicTEMP->port = portOffset;

	if (he) {
	  nicTEMP->nicType=ADR_TCP;
	  memcpy(&intmp, he->h_addr, he->h_length);
	  strcpy(nicTEMP->nicAddress, inet_ntoa(intmp));

	  mh_nics[imetahost][i+inodes]=nicTEMP;
	  mh_nics[imetahost][i+inodes]->nodeName= newString( pnode->nodeName);
	  /* we have created a new nic, and we must put a copy into pnode->nicList */
	  nicTEMP=rh_copyNic(mh_nics[imetahost][i+inodes]);
	  nicTEMP->next=pnode->nicList;
	  pnode->nicList=nicTEMP;
	}
	else {
	  RERROR1("auto_router warning: could not resolve hostname %s\n", pnode->nodeName);
	  strcpy(nicTEMP->nicAddress, pnode->nodeName);
	  return -1;
	}
	pnode=pnode->next;
	if (!pnode) {
	  pnode=metahostlist[imetahost].nodeList;
	  portOffset++;
	}
      }
    }
  }
  
  /* map meta hosts the simplest way */
  for (imetahost =0; imetahost < pars_rconf->nbr_metahosts; imetahost ++) { /* connect imetahost to ... */
    routerCounter=0;
    for (jmetahost =0; jmetahost < pars_rconf->nbr_metahosts; jmetahost ++) {
      if ( imetahost != jmetahost ){
	  /* set up symmetric connection between metahost i and j */
	  /* save the router connection */
	if (  strcmp(metahostlist[imetahost].hostname, pars_rconf->my_metahostname)==0 ) { /* it is me! */
	  int targetNic, r_id;
	  Snode *foundNode;
	 
	  pars_rconf->my_nrp[jmetahost]=1;


	  if ( imetahost < jmetahost )
	    targetNic=imetahost;
	  else
	    targetNic=imetahost-1;

	  if ((r_id=rh_addRouter(jmetahost, routerCounter, mh_nics[imetahost][routerCounter],
				 mh_nics[jmetahost][targetNic], pars_rconf->tcp_portbase))
	      == -1) {
	    RERROR("auto_router: error adding router\n");
	    exit(-1);
	  }
	  routerlist[r_id].nodeName= newString(mh_nics[imetahost][routerCounter]->nodeName); 
	  /* search the metahost node this router is running on */
	  foundNode=rh_findNicInHosts(metahostlist[imetahost].nodeList, mh_nics[imetahost][routerCounter]);
	  if (foundNode->numRouters==0) {
	    foundNode->routerIds= (int*) malloc( sizeof(int));
	    foundNode->routerIds[0]=-1;
	  } else {	    
	    foundNode->routerIds= (int*) realloc( foundNode->routerIds, sizeof(int)
						  *(foundNode->numRouters+1)); 	    
	      foundNode->routerIds[foundNode->numRouters+1]=-1;
	  }
	  
	  foundNode->routerIds[foundNode->numRouters]=r_id;
	  foundNode->numRouters++;	  	  
	}            
	else { /* not for me */
	  /* This router process doesn't run on our metahost. We therefore do not update the routerlist,
	     but we have to have the right number of routers stored for each node to produce the correct
	     cmdline. */
	  int mh_id;
	  Snode *foundNode;
	  
	  mh_id = imetahost;
	  if ( !(foundNode = rh_findNicInHosts( metahostlist[mh_id].nodeList, mh_nics[imetahost][routerCounter] ) ) )
	    /* assume NICs are correct */
	    foundNode = metahostlist[mh_id].nodeList;
	  if( foundNode->numRouters == 0 ) {
	    foundNode->routerIds = (int *)malloc( sizeof(int) );
	    foundNode->routerIds[0]=-1;
	  }
	  else {
	    foundNode->routerIds = (int *)realloc( foundNode->routerIds, sizeof(int) * (foundNode->numRouters + 1) );
	    foundNode->routerIds[foundNode->numRouters]=-1;
	  }
	  foundNode->routerIds[foundNode->numRouters]=routerCounter;

	  foundNode->numRouters++;
	}
	routerCounter++;
	metahostlist[imetahost].num_of_routers++;
	pars_rconf->nrp_metahost[imetahost]++;
      }/* if imetahost == jmetahost */
    } /* for jmetahost ... */
  } /* for imetahost ... */
  pars_rconf->nrp=pars_rconf->nbr_metahosts * (pars_rconf->nbr_metahosts-1);
}
