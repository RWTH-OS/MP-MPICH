/* $Id$ 
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 */
#ifndef __MPICONFIG_H
#define __MPICONFIG_H

#define META_MPI_MAX_METAHOSTS 16 /* max. nbr of metahosts in a MetaMPI environment */
#define META_MPI_MAX_PROCS 1024   /* max. nbr of processes on one metahost */
#define META_MPI_MAX_RP 16        /* max. nbr of routing processes on one metahost */

#define META_COOKIE -31313

#define META_SEMA_ID 13131L
#define META_SEMA_NBR 3
#define META_SEMA_PERMS 0400
#define META_MSG_TYPE 1313L
#define META_POLL_INTVL 10

#define INIT_META_MSGSIZE 1024*1024

#include <arpa/inet.h>
#include "mpi.h"

/* device-specific options for the secondary device */
typedef struct _SecondaryDeviceOpt_t {
  int portbase;
  int portRangeLow, portRangeHigh;
  uint32_t netmask_ip;
  int netmask_no_host_bits;
  int nowatchdog;
  int smi_verbose;
  int smi_debug;
} SecondaryDeviceOpt_t;

/* basically, MPIR_meta_cfg (which will be a global variable, sorry, but it is
   needed at various places) keeps the information found in the routing config file */
typedef struct _MPIR_MetaConfig MPIR_MetaConfig;
struct _MPIR_MetaConfig {
  int isMeta;                               /* do we have a Meta configuration ? */
  int nbr_metahosts;                        /* nbr of metahosts in the meta system */
  int my_metahost_rank;                          /* rank of the local metahost */
  char my_metahostname[MPI_MAX_PROCESSOR_NAME];  /* name of the local metahost */
  char nodeName[MPI_MAX_PROCESSOR_NAME];     /* name of the local host */
  char metahostnames[META_MPI_MAX_METAHOSTS][MPI_MAX_PROCESSOR_NAME];
  int secondaryDevice;                      /* Device for direct (router-less) communication between metahosts */
  SecondaryDeviceOpt_t secondaryDeviceOpts; /* structure with device-specific options */
  int useRouters;                           /* is there any process that this process reaches via a router process ? */
  int useSecondaryDevice;                   /* is there any process that this process reaches via the secondary device ? */
  int *useRouterToMetahost;                 /* useRouterToMetahost[i] == 1 if this process sends to metahost i via router */
  int metahostUsesSecondaryDevice[META_MPI_MAX_METAHOSTS];
  char *metakey;                            /* string containing the "magic meta key" */
  int np;                                   /* nbr of "real" MPI-procs in the whole system 
					       (corresponds to -np value) */
  int np_override;                          /* if not 0, this overrides the np derived from the 
					       configuration file*/
  int np_metahost[META_MPI_MAX_METAHOSTS];  /* nbr of "real" MPI-procs on each metahost */
  int nrp;                                  /* nbr of Routing-procs in the whole system */
  int nrp_metahost[META_MPI_MAX_METAHOSTS]; /* nbr of routing procs on each metahost */
  int npExtra;				    /* number of extra processes in the whole system */
  int *npExtra_ranks;                       /* ranks of extra processes in MPI_COMM_META */
  int npExtra_metahost[META_MPI_MAX_METAHOSTS]; /* number of extra processes on each meta host */
  int *npExtra_local_ranks;                 /* ranks of extra processes local to each metahost 
                                             * in MPI_COMM_LOCAL */
  int metahost_firstrank;                   /* grank of first global (COMM_ALL) proc on this metahost */

  char rpname[MPI_MAX_PROCESSOR_NAME];      /* name of the routing executable */
  char rpargs[MPI_MAX_PROCESSOR_NAME];      /* args for the routing process */
  int my_nrp[META_MPI_MAX_METAHOSTS];       /* nbr of routing procs on this metahost 
					       towards the other metahosts */
  int routerAutoCfg;                        /* boolean, true if the router processes have to be detected 
					       automatically. If false, the routers have to be specified by
					       the commandline with -router <n> */
  int extra;                                /* boolean: true if this is an extra process */
  int router;                               /* boolean: true if this is a router process */
  int dedicated_rp;                         /* boolean: true if the router procs are separate
					       executables to be started via fork()&execvp() */
  int is_hetero;                            /* boolean: true if the cluster is heterogenous */
  int *granks_to_router;                    /* granks_to_router[i], where i is the MPI_COMM_ALL-rank of an
					       application process on another metahost to send to,
					       is the MPI_COMM_ALL-rank of
					       the router process via which to send the message */
  int my_routingid;                         /* used by MPI router processes only */
  int my_localrank;                         /* used by regular MPI processes only, rank in MPI_COMM_LOCAL */
  int my_rank_on_metahost;                  /* rank in MPI_COMM_HOST */
  int *isRouter;                            /* arry of int. The index is the metahost rank, while
					       the content determines if this rank is a router or not */
  int myNodeNr;                             /* number of hosts on my metahost  */
};
extern MPIR_MetaConfig MPIR_meta_cfg;

#endif

