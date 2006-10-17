/* $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
  */

#ifndef __ROUTER_CONFIG_H
#define __ROUTER_CONFIG_H
#include <stdio.h>
/*#include <netdb.h>*/
#include "rhlist.h"

#include "metaconfig.h"


/* max. length of  a niclist string */
#define NICLISTLEN 400
#ifndef MPI_MAXHOSTNAMELEN
#define MPI_MAXHOSTNAMELEN 255
#endif

struct RouterConfig {
  int   router_rank_on_metahost; /* if we are the jth router on our metahost, this should become j-1 */
  int   otherhostid;
  int   tcp_portbase;
  int 	isHetero;   /* is this a heterogenous (regarding to endianess) meta configuration ? */
  int 	exchangeOrder;
  int 	split_size;
  int   isend_num;  /* maximum number of pending nonblocking send operations that are supported by
		       the router; can be changed at startup with ISEND_NUM in meta config file;
		       defaults to ISEND_NUM_DEFAULT */
    
  int myGlobalRouterRank;                          /* this is the global router id */
  int num_router;
  int nbr_metahosts;

    /* ---------------------------- */
  int np;                                   /* nbr of "real" MPI-procs in the whole system 
					       (corresponds to -np value) */
  int secondaryDevice;                      /* Device for direct (router-less) communication between metahosts */
  SecondaryDeviceOpt_t secondaryDeviceOpts; /* structure with device-specific options */
  int nrp_metahost[META_MPI_MAX_METAHOSTS]; /* nbr of routing procs on each metahost */
  int nrp;                                  /* nbr of Routing-procs in the whole system */
  int useRouterFromTo[META_MPI_MAX_METAHOSTS][META_MPI_MAX_METAHOSTS]; /* useRouterFromTo[i][j] == 1 if procs on metahost i
									  use routers to send to procs on metahost j, 0 otherwise */
  int npExtra;				    /* number of extra processes in the whole system */
  int npExtra_metahost[META_MPI_MAX_METAHOSTS]; /* number of extra processes on each meta host */

  char my_metahostname[MPI_MAX_PROCESSOR_NAME];  /* name of the local metahost */
  int my_nrp[META_MPI_MAX_METAHOSTS];       /* nbr of routing procs on this metahost 
					       towards the other metahosts */
  int np_override;                          /* if not 0, this overrides the np derived from the 
					       configuration file*/
  char metahostnames[META_MPI_MAX_METAHOSTS][MPI_MAX_PROCESSOR_NAME];
  int np_metahost[META_MPI_MAX_METAHOSTS];  /* nbr of "real" MPI-procs on each metahost */
  int my_metahost_rank;                          /* rank of the local metahost */

  int router_timeout; /* time to wait for other routers - 0=infinity */

  SnetDefList * netDefList; /* this ist the list of nets for multidevice */    
};

/* this is the global struct for the router configuration
 * implemented in mpi_router.c
 */
extern struct RouterConfig MPIR_RouterConfig;


extern int yydebug;

extern int yyparse();

extern FILE* yyin;
/* share with parser */
extern struct RouterConfig *pars_rconf;

/* This function serves as a wrapper for the bison-generated parser, every call
   to the parser should be done via MPIR_read_metaconfig().

   Parameters:
   fname                name of the meta configuration file to be parsed
   rconf                everything that the parser gets out of the meta configuration file
                        is saved here
*/
int MPIR_read_metaconfig(char *fname, struct RouterConfig *rconf,
			 char *my_metahostname,	int np_override , int debug );
#endif

