/* $Id$ 
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe  email: mpoeppe@gmx.de
 *
 */

#ifdef _rhlist_h
#else
#define _rhlist_h

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include "conn_common.h"
 
#define METAHOST_LIST_SIZE 10
#define ROUTER_LIST_SIZE 10
#define MAX_ADDRESS_LEN 256

#ifdef WIN32
#define strcmp stricmp
#define strcasecmp stricmp
#endif


/* types of supported NICs */
typedef enum _EnicType {ADR_TCP,ADR_ATM_SVC,ADR_ATM_PVC} EnicType;
extern const  char * nicTypeStrings[];


/* types of supported Metahosts (devices) */
#define  NUMDEVS 7
typedef enum _EdeviceType {DEVICE_NULL, DEVICE_SMI, DEVICE_SHMEM, DEVICE_USOCK, DEVICE_GM, DEVICE_P4, DEVICE_MPX} EdeviceType;
extern const char *deviceTypeStrings[];


/* one single network address */
struct _Snic {
  char nicAddress[MAX_ADDRESS_LEN];
  EnicType nicType;
  int port;
  char * nodeName; /* for finding the node easily */

  struct _Snic *next;
} ;

typedef struct _Snic Snic;


/* this describes a single host in a metahost */
struct _Snode {
  char *nodeName;
  Snic *nicList;
  int npSum;	   /* overall number of processes on this node, including routers and extras */
  int npFixed;     /* number of non-application processes */
  int maxNumProcs; /* maximum number of procs on node (apps + routers), 0 if no max given  */
  int np;    /* number of application procs on this */
  int numRouters;  /* how many routers do we have on this node */
  int npExtraProcs;  /* number of extra processes from extraproclist included in numProcs*/
  int *routerIds;  /* list of the metahost-router-ids */
  int numNets;     /* number of Networks connected to this node (multidevice) */
  int *netList;     /* array of Ids of Networks - unused yet and maybe for ever, because multi-device is configured a different way*/
  char * executable; /* this may be an alternative executable on this node, e.g. mpio-procs for viola-io or dedicated router procs */
  char ** extraProcList; /* this is a list of extra executables, which are executed independently of np */
  char * args;		/* args for executable */
  struct _Snode *next;
};

typedef struct _Snode Snode;

/* list of net Definitions */
 struct _SnetDefList {
   char* name;
   int metric;
   char *type;
   struct _SnetDefList  * next;
 };
 typedef  struct _SnetDefList SnetDefList;


/* this describes a metahost */
struct SMetahostListEntry {
  char hostname[255];    /* name of the host */
  unsigned int valid; /* set to 1 if the meta host has been defined */
  int num_of_routers;
  int numProcs;          /* number of processors, -1 if undefined, 0 for maximum */
  int np;           /* number of non-router processes FIXME needed still? */
  int npFixed; 		 /* number of fixed processes (routers+extra) */
  int npExtra;		/* number of extra application processes, included in npFixed and np */
  int numNodes;
  Snode *nodeList;      /* list of nodes which form the metahost */
  EdeviceType deviceType;
  char * devOptions;	/* special command line options for the device */
  char * appArgs;	/* arguments for the application */
  char *execPath,       /* working directory with executable */
    *mpiRoot,           /* directory where MPI is installed */
    *envFile,           /* shell script to source before executing the application */
    *frontend,          /* name of the frontend node, this is the one we can access via ssh or rsh */
    *user,              /* username for ssh login */
    *execName,          /* path to executable */
    *confPath,          /* path to conf file */
    *confName,          /* name of conf file */
    *metaKey;           /* the meta key (an ID e.g. for authentication purpose) */
  int  procgroup;       /* bool, is set to one if metaconfig needs a procgroup file instead of -nodes */
  char* routerExec;     /* default router executable for this meta host, 0 if no dedicated router is used */
};


/* this describes a connection to a remote router */
struct SRouterListEntry {
    int host;              /* entry in metahostlist for metahost to which this router sends */
    char *nodeName;        /* name of the node */
    int sockfd;            /* socket descriptor */
    int port;              /* TCP Port */
    Snic *localNicList;    /* local addresses */
    Snic  *remoteNicList;  /* remote adresses */
    struct sockaddr_in address;
    int serv;              /* serv==1 : server of tcp-connection , 0: client */
    int localRouterRank;
    int conn;              /* connection handle */
    int metahostrank;      /* rank of this router process, seen as global rank from
			      the native device on the metahost (later becomes rank
			      in MPI_COMM_HOST) */
    char * routerExec;     /* path to dedicated router executable */
};

extern struct SMetahostListEntry *metahostlist;
extern struct SRouterListEntry *routerlist;

/* initialization and cleanup */
void rh_initLists();
void rh_freeLists();

/* save a metahostname and the number of MPI-processes in the metahostlist */
int rh_addMetahost(char *name, int numproc);
int rh_addRouter(int host, int id, Snic *lnics, Snic *rnics, int port);

int rh_getRouter(int id);

int rh_getNumRouters();
int rh_getNumMetahosts();
int rh_getMetahostId(char *hostname);

/* output functions for metahost- and routerlist for debugging */
void rh_printMetahostList();
void rh_printRouterList();

/* find a node by name in a nodelist */
Snode *rh_findNicInHosts( Snode *nl, Snic *nics );
Snode * rh_findNodeName( Snode *nl, char * nodeName);

/* utility functions to create and copy nodes */
Snic* rh_copyNic(Snic* pnic);
/* allocates memory for a new Snic, initializes it with the given parameters
   and returns a pointer to it */
Snic *rh_newNic( char *adr, int port, EnicType type );
Snode* rh_newNodeClear();
#endif
/* end of rhlist.h */
