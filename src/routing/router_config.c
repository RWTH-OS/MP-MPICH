/* $I: router_config.c,v 1.20 2003/06/18 13:22:51 boris Exp $ 
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe  email: mpoeppe@gmx.de
 *
 *
 */

#include "router_config.h"
#include "mpi_router.h"
#include "rhlist.h"
#ifdef WIN32
#define strcmp stricmp
#define strcasecmp stricmp
#endif

#include <string.h>

int MPIR_read_metaconfig(char *fname, struct RouterConfig *rconf,
			 char *my_metahostname, int np_override, int debug )
{
    FILE* infile;
    Snode *pnode, *nodeList;
    int np_mh,my_mh_rank, np_node, np_rest, router_rank, router_id, node_firstrank, mh;
	int pass;
	
    pars_rconf=rconf;
    
    /* initialize rconf */
    rconf->otherhostid=-1;
    rconf->myGlobalRouterRank=-1;
    rconf->split_size   = SPLIT_SIZE;
    rconf->tcp_portbase = TCP_PORTBASE;
    rconf->exchangeOrder=DEF_EXCHANGE_ORDER; /* default byte order for router communication */
    rconf->isHetero=0;
    rconf->isend_num = ISEND_NUM_DEFAULT;
    rconf->my_metahost_rank = -1;
    rconf->router_timeout=300;
    strcpy( rconf->my_metahostname, my_metahostname );
    rconf->np_override = np_override;
    rconf->secondaryDevice = DEVICE_NULL;
    (rconf->secondaryDeviceOpts).portbase = -1;
    (rconf->secondaryDeviceOpts).portRangeLow = -1;
    (rconf->secondaryDeviceOpts).portRangeHigh = -1;
    (rconf->secondaryDeviceOpts).netmask_no_host_bits = -1;
    rconf->npExtra= 0;

    rh_initLists();

    infile=fopen(fname,"r");
    if (!infile){ 
	RERROR("cannot open config file!\n");
	return 0;
    } else {     
        yydebug=debug;
	yyin=infile;

	/* parse config file */
	if (yyparse()!=0) {
	  RERROR1("*** there were errors in the config file %s\n",fname);
	  return 0;
	}
	
	if( fclose( infile ) != 0 ) {
	  RERROR("could not close config file!\n");
	  return 0;
	}
    } 
    
    /* calculate number of application processes for each node in the whole system */
    rconf->np=0; /*recalculate this!*/
    for( mh = 0; mh < rh_getNumMetahosts(); mh++ ) {
	rconf->npExtra_metahost[mh]=0;
	nodeList = metahostlist[mh].nodeList;
	if (rconf->np_override) {
		int np;
		/* spread np_override equally on all metahosts */
		if (rconf->np_override < rconf->nbr_metahosts){
			RERROR("np_override (-metaparam) must be greater or equal than number of metahosts!\n");
			return 0;
		}
		np= rconf->np_override / rconf->nbr_metahosts;
		if ( mh == 0 ) /* the first metahost gets the rest */
			np += rconf->np_override % rconf->nbr_metahosts;
		metahostlist[mh].np=np;
		rconf->np_metahost[mh]=np;
	} 
	/* check nodes for routers and processors */
	pnode = nodeList;
	metahostlist[mh].numProcs=0;
	metahostlist[mh].npFixed=0;
	metahostlist[mh].npExtra=0;
	while( pnode ) {
		/* check if we have too many fixed procs for this node */
		if (pnode->npFixed && pnode->maxNumProcs ) {
			if (pnode->npFixed > pnode->maxNumProcs) {
				RERROR4("too many router processes on meta host %s node %s: can't put %d fixed processes on %d processors\n",  metahostlist[mh].hostname, pnode->nodeName, pnode->npFixed, pnode->maxNumProcs);
					return 0;
			}
		}
		metahostlist[mh].npFixed += pnode->npFixed ;
		metahostlist[mh].npExtra += pnode->npExtraProcs;
		if (pnode->maxNumProcs && (metahostlist[mh].numProcs !=-1)) 
			metahostlist[mh].numProcs+=pnode->maxNumProcs;
		else
			metahostlist[mh].numProcs=-1;
		/* EXTRAPROCS are always started! */
		if (pnode->npExtraProcs) {
			pnode->np += pnode->npExtraProcs;
		}
		pnode=pnode->next;
	}
	
	rconf->npExtra_metahost[mh]=metahostlist[mh].npExtra;
	rconf->npExtra += metahostlist[mh].npExtra;
	
	if (rconf->np_metahost[mh]== -1 /* MAX */ ) { /* fill up meta host with processes, if maxNumProc is given for any node */
		if ( metahostlist[mh].numProcs != -1) {
			rconf->np_metahost[mh] = metahostlist[mh].numProcs-metahostlist[mh].npFixed;
			metahostlist[mh].np=rconf->np_metahost[mh];
		} else {
			RERROR1("Number of processes is 0 (=max) for meta host %s, but processor count is not defined for all nodes",  metahostlist[mh].hostname);
		}
	}
	

	
	/* ad extra procs, because they are comm_local */
	rconf->np_metahost[mh] += metahostlist[mh].npExtra;
	metahostlist[mh].np=rconf->np_metahost[mh];
	/* we have to distribute non-fixed application processes */	
	np_rest = metahostlist[mh].np - metahostlist[mh].npExtra;
	


	/* at this point, we know the number of routers and the maximum number of
	   processes on each node in the whole meta system (saved in the metahostlist
	   by the parser); now we calculate the number of application processes to run on
	   each node */

	/* loop over all nodes on the metahost;
	   we try to put np_node application processes on each node; if we can't, because
	   np_node is bigger than the maximum number of processes that can be placed on that node
	   (taking the router processes, that are already on that node, into account), we put as many
	   on it as possible */
	   
	for (pass=0; (pass < metahostlist[mh].np + metahostlist[mh].npFixed) && np_rest ; pass++) {
		pnode = nodeList;
		while( pnode && np_rest) {
			if ( pnode->npFixed <= pass ) {
				/* maximum given for this node */
				if( (!pnode->maxNumProcs) || (pnode->maxNumProcs > pnode->np + pnode->numRouters ) ) {
					/* maximum not exceeded */
					pnode->np++;
					np_rest--;
				}
			}
			pnode = pnode->next;
		}
	}
	/* if there are still processes left, this is an error in the configuration file */
	if( np_rest ) {
		RERROR2("process distribution error on meta host %s: %d processes left and no node has free processors\n", metahostlist[mh].hostname, np_rest);
	    return 0;
	}
	rconf->np +=  rconf->np_metahost[mh];
    } /* end of for-loop over all metahosts */

    
    if( rconf->my_metahost_rank >= 0 )
	my_mh_rank = rconf->my_metahost_rank;
    else
	my_mh_rank = 0;

    /* this is an important part of the meta setup; for each router process that
       is contained in the routerlist, i.e. that runs on our metahost, we calculate
       the metahostrank, i.e. the rank in MPI_COMM_HOST, that this router shall have;
       each process can then look up in the routerlist if it is to become a router
       or not */
    nodeList = metahostlist[my_mh_rank].nodeList;
    pnode = nodeList;
    node_firstrank = 0;
    /* loop over all nodes in our metahost */
    while( pnode ) {
	/* rank in MPI_COMM_HOST of the first process running on this node */
	router_rank = node_firstrank;
	/* loop over all router processes */
	for( router_id = 0; router_id < rh_getNumRouters(); router_id++ ) {
	    /* if this router runs on pnode, it is associated with a rank */
	    if( strcmp( routerlist[router_id].nodeName, pnode->nodeName ) == 0 ) {
		routerlist[router_id].metahostrank = router_rank;
		router_rank++;
	    }
	}
	node_firstrank += pnode->np + pnode->numRouters;
	pnode=pnode->next;
    }	

    return 1;
}
