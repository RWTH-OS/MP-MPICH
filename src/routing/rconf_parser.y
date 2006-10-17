%{
    /* $Id$
     * (c) 2005 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
     * author: Martin Poeppe email: mpoeppe@gmx.de
     *
     * this is the bison file for the meta-configuration parser.
     * it retrieves the necessary information for the router AND the device
     * setup of all processes.
     *
     * These informations are stored in the structs pars_rconf and 
     * information concerning the network connections are stored in the host- and routerlist
     */
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#ifdef WIN32
#include <malloc.h>
#endif
#include "metaconfig.h"
#include "mpi_router.h"
#include "parser.h"
#include "rdebug.h"
#include "rhlist.h"
#include "auto_router.h"


#ifndef WIN32
#include <netdb.h>
#include <sys/socket.h>
#endif



void yyerror(const char* s);

/*#define PARSER_DEBUG */
#ifdef PARSER_DEBUG
#define PDEBUG(a)      RDEBUG(a)
#define PDEBUG1( a,b ) RDEBUG1(a,b)
#define PDEBUG2(a,b,c) RDEBUG2(a,b,c)
#define PDEBUG3(a,b,c,d)      RDEBUG3(a,b,c,d)
#else
#define PDEBUG( a) 
#define PDEBUG1(a,b)
#define PDEBUG2(a,b,c)
#define PDEBUG3(a,b,c,d)
#endif


#define CONF_ERROR(a) {yyerror(a);YYABORT;}

#include "newstrings.h"

       
    /* forward declarations of functions that are implemented in the third section of this file */
    Snic *rh_newNic( char *adr, int port, EnicType type );
    SnicDef *lookupNicDef( char *name );
    
    void resetGlobals();
    int saveRouter( SnicList *nicStack, ConnDefType e_connDefType, char* routerExec);
    int checkConnSections();
    void checkHeaderSection();
    int checkSubSection();
    Snode * sortInNodeList ( Snode *nodeListHead, Snode *inSortNode );


    Snode * createNodeListRange(char* a,char *b,int *start,int *end, int numprocs, SnicList * nn);
    int excludeNode; /* marks if a node was excluded with ! */

    /* variable declarations */

    /* this pointer is used by MPIR_read_metaconfig() as an interface to the parser,
       so it must not be declared static */
    struct RouterConfig *pars_rconf;

    static int countSubsections = 0;
    static int countHosts = 0;

    /* this flag is zero when 0 routers are declared in a PAIR head, so we don't have to parse connections 
       this is for multidevice configurations */
    static int Bparse_conn=0;

    static char hostA[HOSTNAMELEN], hostB[HOSTNAMELEN]; 
    static int nrRouters;   

    static int *sectionList;
    static int numSections;

    static SnicDef *nicDefList = 0;   /* list of NICDEFs */

    /* this is for metahost declarations */
    static int metaHostDefH_id;
    static Snode *metaHostDefNodelist;
    static char metaHostDefType[MAX_STRING];
    static char metaHostDefFRONTEND[MAX_STRING];
    static char metaHostDefUSER[MAX_STRING];
    static char metaHostDefEXECPATH[MAX_STRING];
    static char metaHostDefEXECNAME[MAX_STRING];
    static char metaHostDefEXECARGS[MAX_STRING];
    static char metaHostDefCONFPATH[MAX_STRING];
    static char metaHostDefCONFNAME[MAX_STRING];
    static char metaHostDefMETAKEY[MAX_STRING];
    static char metaHostDefENVFILE[MAX_STRING];
    static char metaHostDefMPIROOT[MAX_STRING];
    static char metaHostDefDevOption[MAX_STRING];
    static char * metaHostDefROUTEREXEC;
    static char **metaHostExtraProcList;
    static char *routerExec;
    static int metaHostFound=0; /* boolean, marks if the given metahost-name is found in the config */
    static int routerCounter;   
    static ConnDefType lastConnDefType;

    /* declared in lexer */
    extern int ZCounter;
    extern int SCounter;
    extern int yylex( void );
%}

%token <zahl> ZAHL
%token <string> NAME
%token <string> PATH
%token <string> LITERAL
%token <string> IP
%token <string> ATM
%token <string> ATM_PVC_ADDRESS
%token <string> IP_NETMASK
%token AUTO_ROUTER NO_DNS ONLY_DNS NUMHOSTS PORTBASE PORTRANGE ROUTERTIMEOUT HETERO EXCHANGE_ORDER 
%token SPLITSIZE ISEND_NUM IP ATM TCP ATM_SVC ATM_PVC DOPPELPUNKT MINUS KOMMA RETURN 
%token CONNECTIONS PAIR OPTIONS NICDEF METAHOST GLEICH CONNTO CONNTO_BIDIR GETIP NODES ROUTERS NET 
%token SECONDARY_DEVICE EXTRAPROCS MAX NETMASK NOWATCHDOG SMI_VERBOSE SMI_DEBUG

%type <zahl> procnum
%type <pointer_to_snic> nicaddr
%type <string> nicname
%type <pointer_to_int> proccount
%type <pointer_to_snicdef> nicdef
%type <pointer_to_snic> nic
%type <pointer_to_snic> nic_iter
%type <pointer_to_sniclist> niclist
%type <pointer_to_sniclist> remotenics
%type <pointer_to_sniclist> sockconndef
%type <pointer_to_snode> node
%type <pointer_to_snode> nodelist
%type <pointer_to_snode> nodelistentry
%type <pointer_to_snetname> netlist
%type <pointer_to_sniclist> nicrange
%type <pointer_to_snetname> netnames
%type <autoOption> autooption
%type <conn_def_type> connto
%type <string_array> nodeexec
%type <pointer_to_char> routerexec
%type <string> optuser
%type <string> optargs
%type <pointer_to_char> appargs
%%

script          : header optsection connsection
                ;

header          : hostsize hosts {
                      PDEBUG("** Header gelesen\n"); 
		      if (!metaHostFound) {
			  RERROR1("WARNING:metahostname \"%s\" not found in config file\nThe exact name of the metahost most be provided with the -metahost <metahostname> option\n", pars_rconf->my_metahostname);
			  /*	  YYABORT;*/
		      }
	          if (countHosts != pars_rconf->nbr_metahosts ) {
 		      	RERROR2("metaconfig error: %d metahosts specified but %d declared\n", pars_rconf->nbr_metahosts, countHosts);
			CONF_ERROR("");
     		  }

		      checkHeaderSection();
                  }
                ;

hostsize        : NUMHOSTS ZAHL RETURN {
		      resetGlobals();
		      PDEBUG1("NUMHOSTS = %d\n",$2);  
		      pars_rconf->nbr_metahosts=$2;
		      pars_rconf->np =0;
		      pars_rconf->nrp =0;
		      pars_rconf->netDefList=0;
		      /* initialize flag */
		      if ((pars_rconf->my_metahostname!=0) && (strcmp(pars_rconf->my_metahostname, "") != 0))
			  metaHostFound=0;
		      else metaHostFound=1; /* we have no metahost given by now */
                  }
                ;
appargs		: {$$=0} 
		| LITERAL { $$=newString($1);}
		;
hosts           : host
                | hosts host
                ;
proccount       : ZAHL { $$=(int *) malloc(sizeof( int));  *$$=$1; }
                | MAX { $$=0; }
                ;
host            : NAME proccount appargs RETURN   {
                      int h_id,np;

		      if ($2) np=*$2;
		      else np=-1; /* MAX */
		      
		      /* check if the meta host is already declared */
		      if ( rh_getMetahostId($1) != -1 ) {
			yyerror("");
			RERROR1("meta host %s already declared. \n", $1);
			YYABORT;
		      }
		      h_id = rh_addMetahost($1, np);
		      /* deviceType defaults to ch_shmem */
		      metahostlist[h_id].deviceType=DEVICE_SHMEM;
		      /* a SMP machine is a single node */
		      metahostlist[h_id].numNodes=1;
		      metahostlist[h_id].appArgs=$3;
		      pars_rconf->my_nrp[h_id]=0;

		      if (np != -1) pars_rconf->np += np;
		      
		      strcpy (pars_rconf->metahostnames[countHosts], $1); 
		      pars_rconf->nrp_metahost[countHosts]=0; /* initialize ! */
		      pars_rconf->np_metahost[countHosts]=np;
		      
		      PDEBUG2("host %s has %d MPI-processes\n", $1, np);

		      if ( strcmp($1,pars_rconf->my_metahostname) == 0) {
			  PDEBUG1("i am host number %d\n", countHosts);
			  pars_rconf->my_metahost_rank=h_id;
			  metaHostFound=1;
		      }		      
		      countHosts++;
                  }
                ;

optsection      : OPTIONS RETURN optionlist  { 
                      /* check metahosts for empty nodelists */
                      int h;
		      for (h=0; h < rh_getNumMetahosts(); h++) {
			  if (!metahostlist[h].nodeList) {
			      /* we need at least one smp node */
			      Snode *nl=rh_newNodeClear();;
			      metahostlist[h].nodeList=nl;
			      /* node and metahost are the same */
			      nl->nodeName=newString(metahostlist[h].hostname);
			      nl->nicList=0;
			      nl->next=0;
			      nl->netList=0;
			      nl->numNets=0;
			      nl->executable=0;

			      metahostlist[h].numNodes=1;
			  }
		      }
                  }
                ;

optionlist      : option
	        | optionlist option
	        ;

option          : RETURN
                | ROUTERTIMEOUT ZAHL RETURN {                  
                      PDEBUG1("ROUTERTIMEOUT is %d\n", $2);
		      pars_rconf->router_timeout=$2;
		  }
                | PORTBASE ZAHL RETURN {                  
                      PDEBUG1("PORTBASE is %d\n", $2);
		      pars_rconf->tcp_portbase=$2;
		  }
                | HETERO ZAHL RETURN {                  
                      PDEBUG1("HETERO is %d\n", $2);
		      pars_rconf->isHetero=$2;
		  }
                | EXCHANGE_ORDER ZAHL RETURN {                  
                      PDEBUG1("EXCHANGE_ORDER is %d\n", $2);
		      pars_rconf->exchangeOrder=$2;
		  }
                | SPLITSIZE ZAHL RETURN {
		      PDEBUG1("SPLITSIZE is %d\n", $2);
		      pars_rconf->split_size=$2;
                  }
                | ISEND_NUM ZAHL RETURN {
		      PDEBUG1("ISEND_NUM is %d\n", $2);
		      pars_rconf->isend_num=$2;
                  }
                | nicdef RETURN {
		      /*save NIC definition in nicDefList */
		      SnicDef *nicDefTEMP;

		      nicDefTEMP = $1;
		      nicDefTEMP->next = nicDefList;
		      nicDefList = nicDefTEMP;

		  }
                | metahostdef RETURN {}
                | netdef RETURN {}
                | secdevdef RETURN {}
                ;

netdef          : NET NAME NAME NAME NAME ZAHL {
  SnetDefList *snetl=0;
                      /* check for first net */
  if (!pars_rconf->netDefList){
    pars_rconf->netDefList= malloc (sizeof(SnetDefList));   
    snetl=pars_rconf->netDefList;
  }else {
    snetl=pars_rconf->netDefList;
    while (snetl->next) snetl=snetl->next;
    snetl->next= malloc (sizeof(SnetDefList));
    snetl=snetl->next;
  }
  snetl->next=0;
  snetl->name=newString($2);
  snetl->type=newString($4);
  snetl->metric=$6;
                  }
                ;

secdevdef : SECONDARY_DEVICE NAME secdevopts {
    if( strcmp( $2, "ch_shmem" ) == 0 )
	pars_rconf->secondaryDevice = DEVICE_SHMEM;
    else if( strcmp( $2, "ch_smi" ) == 0 )
	pars_rconf->secondaryDevice = DEVICE_SMI;
    else if( strcmp( $2, "ch_usock" ) == 0 )
	pars_rconf->secondaryDevice = DEVICE_USOCK;
    else {
	fprintf( stderr, "%s is not a known type for a secondary device\n", $2 );
	YYABORT;
    }
}
;

secdevopts      :
                | '(' secdevoptlist ')' {
                }
                ;

secdevoptlist   : secdevopt
                | secdevopt KOMMA secdevoptlist
                ;

secdevopt       : PORTBASE GLEICH ZAHL {
                (pars_rconf->secondaryDeviceOpts).portbase =$3;
                }
                | PORTRANGE GLEICH ZAHL MINUS ZAHL {
                (pars_rconf->secondaryDeviceOpts).portRangeLow  =$3;
                (pars_rconf->secondaryDeviceOpts).portRangeHigh =$5;
                }
                | NETMASK GLEICH IP_NETMASK {
		    char netmaskString[40];
		    char *t;
		    int no_network_bits;

		    strcpy( netmaskString, $3 );
		    
		    /* find slash in netmask */
		    t = netmaskString;
		    while( *t != '/' )
			t++;

		    
		    /* get number of host bits in netmask */
		    no_network_bits = atoi( t+1 );
		    (pars_rconf->secondaryDeviceOpts).netmask_no_host_bits = 32 - no_network_bits;

		    /* get IP address in netmask */
		    *t = '\0';
		    (pars_rconf->secondaryDeviceOpts).netmask_ip = htonl( inet_addr( netmaskString ) );
                }
                | NOWATCHDOG { (pars_rconf->secondaryDeviceOpts).nowatchdog = 1;
                }
                | SMI_VERBOSE { (pars_rconf->secondaryDeviceOpts).smi_verbose = 1;
                }
                | SMI_DEBUG { (pars_rconf->secondaryDeviceOpts).smi_debug = 1;
                }
                ;

nicdef          : NICDEF nicname nicaddr {
                      /* make a new SnicDef, initialize it with values from nicname and nicaddr and
			 return it value of nicdef */
                      SnicDef *nicDefTEMP;

		      nicDefTEMP = (SnicDef *)malloc( sizeof(SnicDef) );
		      strcpy( nicDefTEMP->nicName, $2 );
		      nicDefTEMP->nic = $3;
		      nicDefTEMP->next = 0;

		      $$ = nicDefTEMP;
                  }
                ;

nicname         : NAME {
                      /* check given name for nic and return it as value of nicname */
		      PDEBUG1("name ist %s\n",$1); 
		      if ( strlen($1) >= MAX_NICDEF_NAME_LEN ) {
			  RERROR1("identifier too long (<%d)\n",MAX_NICDEF_NAME_LEN);
			  CONF_ERROR("");
		      }

		      strcpy( $$, $1 );
                  }
                ;

nicaddr         : TCP  IP {
                      /* allocate data structure for nicaddr-data (type and address) and
			 return it at reduce time as value of non-terminal symbol nicaddr;
			 same comment for other alternatives to this rule */

                      Snic *nicTEMP;

                      PDEBUG1("adresse ist %s\n",$2);
		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;
		      nicTEMP->nicType=ADR_TCP; 
		      strcpy(nicTEMP->nicAddress, $2);

		      $$ = nicTEMP;
                  }  
                | ATM_SVC ATM {
                      Snic *nicTEMP;

                      PDEBUG1("adresse ist %s\n",$2);
		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;
		      nicTEMP->nicType=ADR_ATM_SVC;
		      strcpy(nicTEMP->nicAddress, $2);

		      $$ = nicTEMP;
                  }
                | ATM_PVC ATM_PVC_ADDRESS {
                      Snic *nicTEMP;

		      PDEBUG1("adresse ist %s\n",$2);
		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;
		      nicTEMP->nicType=ADR_ATM_PVC;
		      strcpy(nicTEMP->nicAddress, $2);

		      $$ = nicTEMP;
		  }
	        | GETIP '(' NAME ')' {
                      Snic *nicTEMP;
		      struct in_addr intmp;
		      struct hostent *he= gethostbyname( $3);  

		      nicTEMP = (Snic *)malloc( sizeof(Snic) );
		      nicTEMP->port = 0;
		      nicTEMP->next = 0;
		      nicTEMP->nodeName = 0;

		      if (he) {
			  nicTEMP->nicType=ADR_TCP;
			  memcpy(&intmp, he->h_addr, he->h_length);
			  strcpy(nicTEMP->nicAddress, inet_ntoa(intmp));
		      }
		      else {
			  RERROR1("warning: could not resolve hostname %s\n", $3);
			  strcpy(nicTEMP->nicAddress, $3);
		      }

		      $$ = nicTEMP;
		  }
                ;

metahostdef     : metahostdefhead optreturn '{' optreturn mhoptionlist '}' { 
		      metahostlist[metaHostDefH_id].nodeList=metaHostDefNodelist;
		      metahostlist[metaHostDefH_id].numNodes=0;
		      metahostlist[metaHostDefH_id].devOptions=newString(metaHostDefDevOption);
		      /* count nodes */
		      while (metaHostDefNodelist) {
			  metahostlist[metaHostDefH_id].numNodes++;
			  metaHostDefNodelist->npFixed=metaHostDefNodelist->numRouters
			  				+metaHostDefNodelist->npExtraProcs;
			  if (metaHostDefNodelist->maxNumProcs)
			  	if (metaHostDefNodelist->npFixed > metaHostDefNodelist->maxNumProcs) {
					RERROR2("meta config error: number of fixed processes (%d) exeed number of processors (%d) ",metaHostDefNodelist->npFixed,metaHostDefNodelist->maxNumProcs);
					RERROR2("on node %s meta host %s\n",metaHostDefNodelist->nodeName,metahostlist[metaHostDefH_id].hostname);
					CONF_ERROR("")
				}
			  metaHostDefNodelist=metaHostDefNodelist->next;
		      }
		      
		      if ( strcmp(metaHostDefType, "ch_shmem") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_SHMEM;
			  if ( metahostlist[metaHostDefH_id].numNodes != 1) 
			      CONF_ERROR("Metahost with type ch_shmem must have exactly one node");
		      }
		      
		      if ( strcmp(metaHostDefType, "ch_smi") == 0) {
			  /*	  if ( metahostlist[metaHostDefH_id].numNodes > 1) */
			      metahostlist[metaHostDefH_id].deviceType=DEVICE_SMI;
			      /* else metahostlist[metaHostDefH_id].deviceType=DEVICE_SHMEM; */
		      }

		      if ( strcmp(metaHostDefType, "ch_usock") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_USOCK;
		      }

		      if ( strcmp(metaHostDefType, "ch_gm") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_GM;
		      }

		      if ( strcmp(metaHostDefType, "ch_mpx") == 0) {
			  metahostlist[metaHostDefH_id].deviceType=DEVICE_MPX;
		      }

		      if ( strcmp(metaHostDefENVFILE,"") != 0 ) {
			  metahostlist[metaHostDefH_id].envFile=newString(metaHostDefENVFILE);
		      }
		      
		      if ( strcmp(metaHostDefEXECPATH,"") != 0 ) {
			  metahostlist[metaHostDefH_id].execPath=newString(metaHostDefEXECPATH);
		      }

		      if ( strcmp(metaHostDefEXECNAME,"") != 0 ) {
			  metahostlist[metaHostDefH_id].execName=newString(metaHostDefEXECNAME);
		      }

		      if ( strcmp(metaHostDefEXECARGS,"") != 0 ) {
			  metahostlist[metaHostDefH_id].appArgs=newString(metaHostDefEXECARGS);
		      }

		      if ( strcmp(metaHostDefCONFPATH,"") != 0 ) {
			  metahostlist[metaHostDefH_id].confPath=newString(metaHostDefCONFPATH);
		      }
		      if ( strcmp(metaHostDefCONFNAME,"") != 0 ) {
			  metahostlist[metaHostDefH_id].confName=newString(metaHostDefCONFNAME);
		      }

		      if ( strcmp(metaHostDefMETAKEY,"") != 0 ) {
			  metahostlist[metaHostDefH_id].metaKey=newString(metaHostDefMETAKEY);
		      }

		      if ( strcmp(metaHostDefFRONTEND,"") != 0 ) {
			  metahostlist[metaHostDefH_id].frontend=newString(metaHostDefFRONTEND);
		      }
		      if ( strcmp(metaHostDefUSER,"") != 0 ) {
			  metahostlist[metaHostDefH_id].user=newString(metaHostDefUSER);
		      }
		      
		      if ( strcmp(metaHostDefMPIROOT,"") != 0 ) {
			  metahostlist[metaHostDefH_id].mpiRoot=newString(metaHostDefMPIROOT);
		      }
		      if ( metaHostDefROUTEREXEC ) {
			  metahostlist[metaHostDefH_id].routerExec=metaHostDefROUTEREXEC;
			  metaHostDefROUTEREXEC=0;
		      }
                  }
                ;

metahostdefhead : METAHOST NAME  {     
				metaHostDefNodelist = NULL;
				metaHostDefH_id=rh_getMetahostId( $2);
				if (metaHostDefH_id == -1) {
					yyerror("");
					RERROR1(" meta host %s defined but not declared \n",$2);
					YYABORT;
				}
				if ( metahostlist[metaHostDefH_id].valid ) {
					yyerror("");
					RERROR1(" meta host %s is already defined, double definition is not allowed \n",$2);
					YYABORT;			
				}
				metahostlist[metaHostDefH_id].valid=1;
				
				strcpy(metaHostDefFRONTEND,"n.a.");
				strcpy(metaHostDefENVFILE,"n.a.");
				strcpy(metaHostDefUSER,"n.a.");
				strcpy(metaHostDefEXECPATH,"n.a.");
				strcpy(metaHostDefEXECNAME,"n.a.");
				strcpy(metaHostDefCONFPATH,"n.a.");
				strcpy(metaHostDefCONFNAME,"n.a.");
				strcpy(metaHostDefMETAKEY,"n.a.");
				strcpy(metaHostDefMPIROOT,"n.a.");
				
				/* metaHostDefEXECARGS should be empty */
				strcpy(metaHostDefEXECARGS,"");
		      
				metaHostDefROUTEREXEC=0;
				strcpy(metaHostDefDevOption,"");
			}
			;

optreturn: 
		| RETURN {}
                ;

mhoptionlist  : mhoption ';' optreturn
                |   mhoption ';' optreturn  mhoptionlist
                ;

mhoption	: NAME GLEICH NAME  { 
				int ok=0;
				if (strcmp ($1,"TYPE")==0) { 
					ok=1; 
					strcpy(metaHostDefType, $3); 
				}
				if (strcmp ($1,"FRONTEND")==0) { ok=1; strcpy(metaHostDefFRONTEND, $3); }
				if (strcmp ($1,"EXECNAME")==0) { ok=1; strcpy(metaHostDefEXECNAME, $3); }
				if (strcmp ($1,"CONFNAME")==0) { ok=1; strcpy(metaHostDefCONFNAME, $3); }
				if (strcmp ($1,"METAKEY")==0)  { ok=1; strcpy(metaHostDefMETAKEY,  $3); }
				if (strcmp ($1,"USER")==0) { ok=1; strcpy(metaHostDefUSER, $3); }
				if (!ok) {
					fprintf(stderr, "%s is not an metahost option or right value not a valid string",$1);
					YYABORT;
				}
			}
			| NAME GLEICH PATH {
				int ok=0;		       
				if (strcmp ($1,"EXECPATH")==0) { ok=1; strcpy(metaHostDefEXECPATH, $3); }
				if (strcmp ($1,"CONFPATH")==0) { ok=1; strcpy(metaHostDefCONFPATH, $3); }
				if (strcmp ($1,"ENVFILE")==0) { ok=1; strcpy(metaHostDefENVFILE, $3); }
				if (strcmp ($1,"MPIROOT")==0) { ok=1; strcpy(metaHostDefMPIROOT, $3); }
				if (strcmp ($1,"ROUTEREXEC")==0) { ok=1; metaHostDefROUTEREXEC=newString($3); }
				if (!ok) {
				fprintf(stderr, "%s is not an metahost option or right value not a valid string",$1);
				YYABORT;
				}
			}
			| NODES GLEICH nodelist   {
				if(metaHostDefNodelist!=NULL)
					metaHostDefNodelist = sortInNodeList( metaHostDefNodelist, $3 );
				else
					metaHostDefNodelist = $3;
			}
			| ROUTERS GLEICH nodelist   {
				if(metaHostDefNodelist!=NULL)
					metaHostDefNodelist = sortInNodeList( metaHostDefNodelist, $3 );
				else
					metaHostDefNodelist = $3;
			}
			| EXTRAPROCS GLEICH extraproclist  {
				metahostlist[metaHostDefH_id].procgroup=1;
			}
			| NAME GLEICH LITERAL {
				if (strcmp($1,"DEVOPTS")==0) {
					strcpy( metaHostDefDevOption,  $3);
				}
				else if (strcmp ($1,"EXECARGS")==0) {
					strcpy(metaHostDefEXECARGS, $3);
				}
				else {
					CONF_ERROR("unknown metahost option");
				}
			}
			;
nodelist        : nodelistentry { $$ = $1; }
                | nodelist KOMMA nodelistentry {
		      Snode *nodelistTEMP;
		      
		      nodelistTEMP = $1;
		      if (excludeNode) {
		      	if (! rh_findNodeName($1,$3->nodeName) ) {
				RERROR1("metaconfig error: node %s excluded but not previously defined\n",$3->nodeName);
				CONF_ERROR("")
			}
		      }
		      nodelistTEMP = sortInNodeList( nodelistTEMP, $3 );

		      $$ = nodelistTEMP;
		  }
                ;
nodelistentry   : '!' node {
                         $$=$2;
			 /*                        RERROR1("node %s excluded\n", $2->nodeName);*/
			 excludeNode=1;
                  }
                | node {
		  $$=$1;
		  excludeNode=0;                 
		}
		;
extraproclist	: extraproclistentry { 
		}
                | extraproclist KOMMA extraproclistentry {
		  }
                ;
optuser		: { strcpy($$,""); }/* no user given */
		| NAME { strcpy($$,$1); }
		;
optargs		: { strcpy($$,""); }/* no args given */
		| LITERAL {strcpy($$,$1);}
		;
extraproclistentry   :  NAME DOPPELPUNKT procnum DOPPELPUNKT PATH DOPPELPUNKT optuser DOPPELPUNKT optargs {
		  	Snode *pn;
			char * extraProcListEntryTEMP;
			int i;
			
			pn=rh_findNodeName(metaHostDefNodelist, $1);
			if (!pn) { /* node not defined? */
				RERROR2("metaconfig error in EXTRAPROC: %s not in nodelist of meta host %s.\n", $1, metahostlist[metaHostDefH_id].hostname);	
				CONF_ERROR("")
			}
			extraProcListEntryTEMP=newString("");
			ALLOC_SPRINTF2(extraProcListEntryTEMP,"%s %d ", $1,$3);
			extraProcListEntryTEMP=stringAppend(extraProcListEntryTEMP,$5); /* PATH */
			if (strcmp($7,"") != 0) {
				extraProcListEntryTEMP=stringAppend3(extraProcListEntryTEMP," ",$7); /* optuser */
			}
			if (strcmp($9,"") != 0) {
				extraProcListEntryTEMP=stringAppend3(extraProcListEntryTEMP," ",$9); /* optarg */
			}
			if (!pn->extraProcList) {
				pn->extraProcList=malloc(sizeof(char**));
				pn->extraProcList[0]=0;
			}
			for (i=0; pn->extraProcList[i]!=0; i++)  /**/;
			pn->extraProcList = realloc(pn->extraProcList[i],sizeof(char*)*(i+1+1));
			pn->extraProcList[i]=extraProcListEntryTEMP;
			pn->extraProcList[i+1]=0;
			pn->npExtraProcs +=$3;
		}
                ;	
nodeexec	: { /* no executable given for this node */
			strcpy($$[0],""); 
			strcpy($$[1],"");
		}
		| DOPPELPUNKT PATH {
			strcpy($$[0],$2);
			strcpy($$[1],"");
		}
		/* following is not working yet */
		| DOPPELPUNKT PATH DOPPELPUNKT optargs {
			strcpy($$[0],$2);
			strcpy($$[1],$4);
		}
		;
node	: NAME procnum netlist nodeexec { 
                /* no niclist found */
				Snode *nodeTEMP;
	  
				nodeTEMP = rh_newNodeClear();
				nodeTEMP->nodeName= newString ($1);
				nodeTEMP->nicList = 0;
				nodeTEMP->maxNumProcs=$2;
				nodeTEMP->np = 0;
				nodeTEMP->numRouters = 0;
				nodeTEMP->netList=0;
				nodeTEMP->numNets=0;
				nodeTEMP->executable=0;
				nodeTEMP->args=0;
		      
				if ( strlen($4[0]) != 0) {
					nodeTEMP->executable=newString($4[0]);
					metahostlist[metaHostDefH_id].procgroup=1;
				}
				if ( strlen($4[1]) != 0) {
					nodeTEMP->args=newString($4[1]);
				}
				
				if ($3 /* netlist */) {
					if (!saveNetList($3, nodeTEMP))
						CONF_ERROR(" ");
				}

				nodeTEMP->next = 0;
		      
				$$ = nodeTEMP;
		}
		| NAME procnum '(' niclist  ')' netlist nodeexec {
				/* we have to save the niclist */
				Snode *nodeTEMP;
				SnicList *sniclistTEMP;

				sniclistTEMP = $4;
				nodeTEMP = rh_newNodeClear();;
				nodeTEMP->nodeName= newString ($1);
				nodeTEMP->nicList=sniclistTEMP->nicList;
				nodeTEMP->maxNumProcs=$2;
		      
				nodeTEMP->netList=0;
				nodeTEMP->numNets=0;
				nodeTEMP->executable=0;
				nodeTEMP->args=0;   

				if ( strlen($7[0]) != 0) {
					nodeTEMP->executable=newString($7[0]);
					metahostlist[metaHostDefH_id].procgroup=1;			
				}
				if ( strlen($7[1]) != 0) {
					nodeTEMP->args=newString($7[1]);
				}
				

				if ($6 /* netlist */) {
					if (!saveNetList($6, nodeTEMP))
						CONF_ERROR(" ");			
				}

				nodeTEMP->np = 0;
				nodeTEMP->numRouters = 0;

				nodeTEMP->next = 0;
		      
				$$ = nodeTEMP;
		}
		| NAME MINUS NAME procnum nicrange { /* range of sequential numbered names */
				int start, end;
				Snode * nodeTEMP;

				nodeTEMP = createNodeListRange($1,$3,&start,&end, $4, $5);

				if (!nodeTEMP) {CONF_ERROR("");};
					PDEBUG2("node-range start %d end %d\n", start, end);
				$$ = nodeTEMP;
		}
		;
nicrange         : { $$=0; }
                | '(' niclist ')' { 
		  if (!$2) {
		    CONF_ERROR(" empty list of networks in node range");
		  }
		  $$=$2; 
                  }
                ;

netlist         : { $$=0; }
                | NET '(' netnames ')' { 
		  if (!$3) {
		    CONF_ERROR(" empty list of networks ");
		  }
		  $$=$3; 
                  }
                ;
netnames        : NAME {
                      SnetName *nname = malloc (sizeof(SnetName));
                      nname->name=newString($1);
                      nname->next=0;
                      $$=nname;
                }
                | netnames KOMMA NAME {
                      SnetName *nname = malloc (sizeof(SnetName));
                      nname->name=newString($3);
                      nname->next=0;
                      ($1)->next=nname;
		      $$=$1;
                }
	        ;
procnum         : { $$=0; }
                | ZAHL { $$=$1; }
                ;

niclist         : nic_iter {
		      SnicList *sniclistTEMP;

		      sniclistTEMP = (SnicList *)malloc( sizeof(SnicList) );
		      sniclistTEMP->next = 0;
		      sniclistTEMP->nicList = $1;
		      
		      $$ = sniclistTEMP;
                  }
                ;

nic_iter        : nic {
                      $$ = $1;
                  }
                | nic_iter KOMMA nic {
   		      Snic *nicTEMP;

		      nicTEMP = $3;
		      nicTEMP->next = $1;
		      
		      $$ = nicTEMP;
		  }
                ;

nic             : IP {  
                      $$ = rh_newNic( $1, 0, ADR_TCP );
		  }
                | IP DOPPELPUNKT ZAHL { 
		      $$ = rh_newNic( $1, $3, ADR_TCP ); 
		  }
                | ATM { 
		      $$ = rh_newNic( $1, 0, ADR_ATM_SVC );
		  }
                | NAME { 
 		      SnicDef *nicDefTEMP;

		      nicDefTEMP = lookupNicDef( $1 );
		      if( !nicDefTEMP ) {
			  RERROR1("NIC %s not defined!\n", $1);
			  CONF_ERROR("")
		      }
         	      PDEBUG1("name is here:%s\n", nicDefTEMP->nicName);

                      $$ = rh_newNic( nicDefTEMP->nic->nicAddress, nicDefTEMP->nic->port, nicDefTEMP->nic->nicType);
		  }
                | NAME DOPPELPUNKT ZAHL {
 		      SnicDef *nicDefTEMP;

		      nicDefTEMP = lookupNicDef( $1 );
		      if( !nicDefTEMP ) {
			  RERROR1("NIC %s not defined!\n", $1);
			  CONF_ERROR("")
		      }
		      PDEBUG2("name is here:%s, port is %d\n", nicDefTEMP->nicName, $3);

                      $$ = rh_newNic( nicDefTEMP->nic->nicAddress, $3, nicDefTEMP->nic->nicType);
                  }
                ;

connsection     : CONNECTIONS RETURN subsections { if (!checkConnSections())  CONF_ERROR("");}
                | CONNECTIONS RETURN AUTO_ROUTER  emptyLines { /* automatic configuration of router processes */		  
		  if (!auto_router(AR_NO_OPT)) { 
		    CONF_ERROR("automatic router configuration failed ");
		  }
                 }
                 | CONNECTIONS RETURN AUTO_ROUTER autooption emptyLines { /* automatic configuration of router processes with options */		  
		  if (!auto_router($4)) { 
		    CONF_ERROR("automatic router configuration failed ");
		  }
                 }

                ;
autooption	: NO_DNS { $$= AR_NO_DNS; }
		| ONLY_DNS { $$= AR_ONLY_DNS; }
		;
subsections     : subsection {}
	        | subsections subsection {}
                ;

subsection      : subhead subbody { 
			routerExec=0;
                      if (routerCounter < nrRouters)   CONF_ERROR("connection missing ");
		      if (routerCounter > nrRouters)   CONF_ERROR("too many connections  ");
                  }
                |  subhead  { 
		    if (Bparse_conn )   CONF_ERROR("connections missing for declared routers. Router number must be 0 for multidevice ");
                  }
                               ;

subhead         : PAIR NAME  NAME  ZAHL  routerexec RETURN {
		      if (strlen($2) > HOSTNAMELEN-1 && strlen($3) > HOSTNAMELEN-1) {
			  RERROR1("Error in Metaconfig-File line %d: hostname too long!", ZCounter );
			  CONF_ERROR("")
		      }
		      strcpy(hostA,$2); strcpy(hostB,$3); nrRouters=$4;
		      PDEBUG2("%s -> %s:",hostA, hostB); PDEBUG1("%d Router\n", nrRouters);

		      /* check hosts */
		      if (rh_getMetahostId(hostA)<0 ) {
			  RERROR1("undeclared host %s ", hostA);
			  CONF_ERROR("")
		      }
		      if (rh_getMetahostId(hostB) <0 ) {
			  RERROR1("undeclared host %s ", hostB);
			  CONF_ERROR("")
		      }

		      routerCounter=0;
		      if ($4 == 0) {
			/* _no_ routers - multidevice */ 
			Bparse_conn = 0;
			pars_rconf->useRouterFromTo[rh_getMetahostId(hostA)][rh_getMetahostId(hostB)] = 0;
		      }
		      else {
			  Bparse_conn = 1;
			  pars_rconf->useRouterFromTo[rh_getMetahostId(hostA)][rh_getMetahostId(hostB)] = 1;
		      }
		      if ($5) {
		      		routerExec=$5;
		      } else {
		      		routerExec = metahostlist[rh_getMetahostId(hostA)].routerExec;
		      }
		      /* if we have a dedicated router, we need a procgroup file for this mh */
		      if (routerExec)	
		      	   metahostlist[rh_getMetahostId(hostA)].procgroup=1;
		      /* is it me? */
		      if ( strcmp(hostA, pars_rconf->my_metahostname)==0){
			  pars_rconf->my_nrp[rh_getMetahostId(hostB)] = $4;
		      }
		      pars_rconf->nrp_metahost[rh_getMetahostId(hostA)] += $4;
		      pars_rconf->nrp += $4;
		      checkSubSection();   
                  }
                ;
routerexec      : MINUS {$$=0}
                | PATH { $$=newString($1); }
                ;
subbody         : connectiondef
                | subbody connectiondef
                ;

connectiondef   : sockconndef RETURN {
                      if (!saveRouter( $1 , lastConnDefType, routerExec)) {CONF_ERROR("")}		      
		      routerCounter++;
		      if (lastConnDefType==ConnDefType_BI) {
			/* we skip the backward definition section, so we correct this */
			sectionList[rh_getMetahostId(hostB)*pars_rconf->nbr_metahosts+rh_getMetahostId(hostA)]=1;
			pars_rconf->nrp_metahost[rh_getMetahostId(hostB)]++;
			pars_rconf->nrp++;
		      }
                   }
                ;

sockconndef     : niclist connto remotenics {
                      int i = 0;
		      SnicList *sniclistTEMP, *tailStack;
		      Snic *pnic;
    		      lastConnDefType=$2;
		      sniclistTEMP = $1;
		      sniclistTEMP->next = $3;
		      
		      tailStack = sniclistTEMP;

		      while (tailStack){
			  i++;
			  PDEBUG1("%d. niclist:",i); 
			  pnic=tailStack->nicList;
			  while(pnic) {
			      PDEBUG1(":%s:",pnic->nicAddress);
			      pnic=pnic->next;
			  }
			  PDEBUG("\n");
			  tailStack=tailStack->next;
		      }
		      PDEBUG1("%d niclists found!\n",i);

		      $$ = sniclistTEMP;
                  }
		
                ;

connto          : /* nothing */ {  $$=ConnDefType_UNI; }
                | CONNTO {   $$=ConnDefType_UNI; }
                | CONNTO_BIDIR {   $$=ConnDefType_BI; }                
                ;
    
remotenics      : niclist { 
                      $$ = $1;
                  }
                | remotenics niclist {
                      SnicList *sniclistTEMP;

		      sniclistTEMP = $2;
		      sniclistTEMP->next = $1;

		      $$ = sniclistTEMP;
		  }
                ;
emptyLines      : RETURN
                |  emptyLines RETURN
                ;


%%
/* looks for a nic definition with name in nicDefList;
   if found, returns a pointer to it, otherwise 0;
   in case of multiple definitions with the same name, pointer to
   last of them is returned */
SnicDef *lookupNicDef( char *name )
{
    SnicDef *ndSearch, *ndFound = 0;

    ndSearch = nicDefList;
    while( ndSearch ) {
	if( strcmp( ndSearch->nicName, name ) == 0 )
	    ndFound = ndSearch;
	ndSearch = ndSearch->next;
    }

    return ndFound;
}

int addRouter(ConnDefType e_connDefType, char* routerExec, int mh_idA, int mh_idB, Snic * nicsA, Snic * nicsB) {
	    Snode *foundNode;
	    char* nodeName=0;
	    int r_id;
	    int backDirection=0;	    	    
	    int repeats;

	    repeats=(e_connDefType==ConnDefType_BI)?2:1;

	    while ( backDirection < repeats) {
	     if (backDirection == 1) {
	     	int mh_id_swp;
		Snic * nics_swp;
	     	/* make a connection back, swap A and B */
		mh_id_swp=mh_idA;
		mh_idA=mh_idB;
		mh_idB=mh_id_swp;
		nics_swp=nicsA;
		nicsA=nicsB;
		nicsB=nics_swp;
	     }
	     backDirection++;
	    /* search the metahost node this router is running on */
	    foundNode=rh_findNicInHosts(metahostlist[mh_idA].nodeList, nicsA);
	    if (foundNode) 
		nodeName=newString(foundNode->nodeName);
	    else  { /* this might be an error */
	        if ( 1==1) { /* FIXME - this shoud assure there is only on node in this meta host */
		    /* one node only - assume nics are correct */
		    foundNode=metahostlist[mh_idA].nodeList;
		    nodeName=newString(metahostlist[mh_idA].nodeList->nodeName);
		} else {
		    RERROR2("saveRouter: network interface %s address not found for non-SMP metahost %s\n",
			    nicsA->nicAddress,mh_idA);
		    return 0;
		}
	    }
	    if (  pars_rconf->my_metahost_rank == mh_idA ) { /* this is my meta host - save router !*/
	    	if ((r_id=rh_addRouter(mh_idB, metahostlist[mh_idA].num_of_routers, nicsA, nicsB, 	pars_rconf->tcp_portbase))
			   == -1) {
			RERROR("saveRouter: error adding router\n");
			return 0;
	    	}	    	    
	    	routerlist[r_id].nodeName=nodeName;
		if (backDirection == 1) {
			routerlist[r_id].routerExec=routerExec;
		}
		if (backDirection == 2) {
			routerlist[r_id].routerExec=metahostlist[mh_idA].routerExec;
		}
	    }
	    if (foundNode->numRouters==0) {
	    	foundNode->routerIds= (int*) malloc( sizeof(int));
		foundNode->routerIds[0]=-1;
	    } else {		
		foundNode->routerIds= (int*) realloc( foundNode->routerIds, sizeof(int)
							 *(foundNode->numRouters+1)); 
		foundNode->routerIds[foundNode->numRouters+1]=-1;	    
  	    }
	    foundNode->routerIds[foundNode->numRouters]=metahostlist[mh_idA].num_of_routers;
	    foundNode->numRouters++;
	    foundNode->npFixed++;
	    metahostlist[mh_idA].num_of_routers++;
	} /* while */
}
/* the most important action of the parser: save the router data */
int saveRouter( SnicList *nicStack, ConnDefType e_connDefType , char* routerExec){
    SnicList *tailStack;
    SnicList *nStack;
    int mh_idB, mh_idA,r_id;
    Snic *localNics, *remoteNics;   
    
    if (routerCounter>nrRouters) {
	RERROR("too many router declarations ");
	return 0;
    }

    /* first niclist is the routers nic - save it*/
    localNics=nicStack->nicList;

    PDEBUG1("Host=%s ",hostA);
    PDEBUG2("num_of_routers=%d hostA=%s\n", metahostlist[rh_getMetahostId(hostA)].num_of_routers,
	    hostA);
 
    /* we have to save the data for ALL meta-local routers, because we may not know our local router rank,
     * e.g. the ch_shmem device forks the processes on the metahost later in MPID_Init
     */
     mh_idA = rh_getMetahostId(hostA);
	mh_idB = rh_getMetahostId(hostB);
	
	/* build list of routers i have to connect to */
	tailStack=nicStack->next;
	while(tailStack){
	    Snode *foundNode;
	    char* nodeName=0;
	
	    remoteNics=tailStack->nicList;
	    addRouter(e_connDefType, routerExec,  mh_idA, mh_idB, localNics, remoteNics);

	    tailStack = tailStack->next;
	}
    
    
    /* clear niclists */
    tailStack=nicStack;

    while (tailStack) {
	nStack=tailStack->next;
	free(tailStack);
	tailStack=nStack;
    }

    return 1;
}


int checkConnSections(){    
    int i,j;
    for(i=0; i< pars_rconf->nbr_metahosts;i++)    
	for(j=0; j< pars_rconf->nbr_metahosts;j++)
	    if (i!=j)
		if (!sectionList[i*pars_rconf->nbr_metahosts+j]) {
		    RERROR2("Subsection missing: %s -> %s\n",pars_rconf->metahostnames[i], 
			   pars_rconf->metahostnames[j]);
		    return 0;
		    
		}		   
    return 1;
}


void checkHeaderSection(){
    int i;
    
    PDEBUG("** initialisiere sectionList\n");
    /* prepare list of host-to-host pairs */
    numSections= pars_rconf->nbr_metahosts*(pars_rconf->nbr_metahosts-1);
    sectionList= (int*) malloc(sizeof(int)* pars_rconf->nbr_metahosts*pars_rconf->nbr_metahosts);
    for (i=0; i< pars_rconf->nbr_metahosts*pars_rconf->nbr_metahosts; i++){
	sectionList[i]=0;
    }
    
    pars_rconf->nbr_metahosts=countHosts;

}


int checkSubSection(){
    int i;
    int id1, id2;

    #ifdef bla
    if (countSubsections == numSections) { 
    	RERROR("too many Subsections!\n"); return 0;
    }
#endif
    /* check host-to-host pair in list */
    id1=rh_getMetahostId(hostA); id2=rh_getMetahostId(hostB);

    PDEBUG1("this section id is %d\n",id1* (pars_rconf->nbr_metahosts) + id2);
    PDEBUG("Sectionlist:"); for(i=0; i<numSections; i++)  PDEBUG1(" %d",sectionList[i]); PDEBUG("\n");
    PDEBUG2("id1: %d id2: %d\n",id1,id2);

    if (sectionList[ id1 * (pars_rconf->nbr_metahosts) + id2 ]) {
	RERROR2("section %s->%s already exists! ",hostA,hostB); 
	return 0;
    }
    sectionList[ id1 * (pars_rconf->nbr_metahosts) + id2] = 1;
    countSubsections++;
    
    return 1;
}


/* sortInNodeList sorts a node list alphabetically into another list of nodes
 * this is important for ch_smi because the process ranks are distributed in
 * this order
 * if a node is already in the list, its properties are updated.
 * inSortNodeList is consumed, i.e. it cannot be used anymore after a call to
 * this function.
 * the new root of the list is returned
 */
Snode *  sortInNodeList ( Snode *nodeListHead, Snode *inSortNodeList ){
    Snode *inSortNode = inSortNodeList;

    while (inSortNode) {
	Snode *pn, *nextInSortNode; 
	
	pn = nodeListHead;
	
	/* we have to save the next node because the next-field will be overwritten */
	nextInSortNode=inSortNode->next;

	if (!pn) {
	    /* first node */
	  if ( !excludeNode )
	    nodeListHead = inSortNode;
	}
	else {
	    /* sort in list */
	    Snode * oldpn = 0;
	    Snode *oldoldpn = 0;

	    while(pn && strcmp(inSortNode->nodeName, pn->nodeName) >= 0) {
	      oldoldpn=oldpn;
	      oldpn=pn;
	      pn=pn->next;
	    }
	    
	    if (!oldpn) {
	      /* inSortNode gets first node */
	      if ( !excludeNode ) {
		nodeListHead=inSortNode;
		inSortNode->next = pn;
	      }
	    }
	    else {
	      if (strcmp(inSortNode->nodeName, oldpn->nodeName)==0) { /* update or exclude */
		if ( excludeNode ) {
		  /* remove node oldpn from list */
		  /*		  RERROR1("removing node %s\n",oldpn->nodeName);*/

		  if (oldoldpn != 0)
		    oldoldpn->next=oldpn->next;
		  else 
		    nodeListHead=oldpn->next;

		  free(oldpn->nodeName);
		  free(oldpn);
		} else {

		  if ( inSortNode->maxNumProcs > 0) 
		    oldpn->maxNumProcs = inSortNode->maxNumProcs;
		  if ( inSortNode->nicList ) 
		    oldpn->nicList = inSortNode->nicList;
    		  if ( inSortNode->executable ) 
		    oldpn->executable = inSortNode->executable;

		}
	      }
	      else {
		inSortNode->next=pn;
		oldpn->next=inSortNode;
	      }
	    }
	}
	/* process next item in the inSortNodeList */
	inSortNode=nextInSortNode;
    } /* while(inSortNode) */
    return nodeListHead;
}


/* checks if a and b have the same form <prefix><number> and returns
   the start and end number if true
   the return value is a pointer to a node list, or 0 if an error occurs 
*/
Snode * createNodeListRange(char* a,char *b,int *start,int *end, int numprocs, SnicList* nicrange){

  int len, prefixLen,digitLen,suffixPos,digitPos;
    char *prefix,*suffix;
    Snode * rangeNodeList;
    unsigned int it_nicRange[4];

    RDEBUG("in createNodeListRange:\n");
    RDEBUG2("%s .... %s\n",a,b);
    
    /* check the syntax of the range */
    if( strlen(a) != strlen(b)) {
	RDEBUG("node range boundaries differ in length\n");
	return 0;
    }    
    
    /* allocate strings */
    prefix=(char*)malloc(sizeof(char)*strlen(a));
    suffix=(char*)malloc(sizeof(char)*strlen(a));

    /* look for range number delimiter | in a and compare with b */
    prefixLen=0;
    while ( prefixLen < strlen(a) && a[prefixLen] != '|' && a[prefixLen] == b[prefixLen]){
      prefix[prefixLen]=a[prefixLen];
      prefixLen++;
    }
    /* what happend? */
    if (a[prefixLen] != b[prefixLen]) {
      RERROR2("meta configuration error: differing prefixes in node range %s - %s\n",a,b);
      return 0;
    }
    if (prefixLen == strlen(a)) { 
      RERROR2("meta configuration error: no range digits delimiters |dd| found in node range %s - %s\n",a,b);
      return 0;
    }
    if (prefixLen == 0) {
      RERROR2("meta configuration error: no prefix found in node range %s - %s\n",a,b);
      return 0;
    }
    prefix[prefixLen]=0; /* close prefix string */
    prefixLen--;

    /* look for second range number delimiter | in a and check digits from a and b */
    suffixPos=prefixLen+2;
    digitPos=suffixPos;
    digitLen=0;
    if (suffixPos == strlen(a)) {
      RERROR2("meta configuration error: only 1 range digits delimiters |dd| found in node range %s - %s\n",a,b);
      return 0;
    }
    while ( suffixPos < strlen(a) && a[suffixPos] != '|' 
	    &&  isdigit(a[suffixPos]) && isdigit(b[suffixPos]) ) {
      suffixPos++;
      digitLen++;
    }

    /* what happend? */
    if (suffixPos == strlen(a)) { 
      RERROR2("meta configuration error: no second range digits delimiters |dd| found in node range %s - %s\n",a,b);
      return 0;
    }
    if ( a[suffixPos] == '|') {
      if (b[suffixPos] != '|' ) {
	RERROR2("meta configuration error: b must have range digits delimiters in the same position as a (a=%s - b=%s)\n",a,b);
	return 0;
      }
      /* ok, proceed! */
    } else {
      if (!isdigit(a[suffixPos])){
	RERROR1("meta configuration error: non-digit %c found in lower range digits:  %s\n",a);
	return 0;
      }
      if (!isdigit(b[suffixPos])){
	RERROR1("meta configuration error: non-digit %c found in upper range digits:  %s\n",b);
	return 0;
      }
      if ( b[suffixPos] != '|') {
	RERROR2("meta configuration error: b must have range digits delimiters in the same position as a (a=%s - b=%s)\n",a,b);
	return 0;
      }
    }
    if (digitLen == 0){
      RERROR2("meta configuration error: no range digits found in node range %s - %s\n",a,b);
      return 0;
    }
    suffixPos++;
    strcpy(suffix,"");
    {
      int pos=0;
      while (suffixPos + pos <= strlen(a)) {
	suffix[pos]=a[suffixPos+pos];
	pos++;
      }
    }
	
    /* get start and end */
    *start = atoi( a+ digitPos);
    *end = atoi( b+ digitPos);
   
    /* check if a nicrange is provided */
    if  (nicrange) {
      Snic *nicStart=nicrange->nicList;
      struct in_addr intmp;
      if( (intmp.s_addr = inet_addr( nicStart->nicAddress )) == (in_addr_t)-1 ) {
	RERROR1(" wrong ip %s ",nicStart->nicAddress);
	return 0;
      }
      it_nicRange[0]=(unsigned int) ((unsigned char*)&intmp)[0];
      it_nicRange[1]=(unsigned int) ((unsigned char*)&intmp)[1];
      it_nicRange[2]=(unsigned int) ((unsigned char*)&intmp)[2];
      it_nicRange[3]=(unsigned int) ((unsigned char*)&intmp)[3];
    }


    /* now build a nodeList */
    {
      int nr;
      char *c_digits;
      char *leadingZeros;
      char * tempname;
      Snode *oldpnode=0;    

      c_digits = malloc(sizeof(char) * (digitLen+1));
      leadingZeros = malloc(sizeof(char) * (digitLen+1));
      tempname=newString(a);

	for ( nr=*start; nr <= *end; nr++ ) {
	    Snode * pnode;
	    int i, dec, count=0;

	    sprintf(c_digits,"%d",nr);
	    strcpy(leadingZeros,"");

	    for (i=0; i < digitLen-strlen(c_digits); i++)
	      strcat(leadingZeros,"0");

	    strcpy(tempname,prefix);
	    strcat(tempname,leadingZeros);
	    strcat(tempname,c_digits);
	    strcat(tempname,suffix);
	    		
	    pnode=rh_newNodeClear();

	    /* link new element */
	    if (oldpnode)
		oldpnode->next = pnode;
	    else  /* save root of this list */
		rangeNodeList = pnode;
	  
	    pnode->next=0;
	    pnode->numNets=0;
	    pnode->netList=0;
	    /*	    if (netnames) saveNetList(netnames, pnode);*/
	    pnode->nicList=0;
	    pnode->numRouters=0;
	    pnode->routerIds=0;
	    pnode->maxNumProcs=numprocs;
	    pnode->nodeName= newString(tempname);
	    PDEBUG1("adding node %s\n",tempname);	    
	    if (nicrange) {
	      char  anic[200];
	      
	      sprintf(anic,"%d.%d.%d.%d",it_nicRange[0],it_nicRange[1],it_nicRange[2],it_nicRange[3]);
	      pnode->nicList= rh_newNic(anic,nicrange->nicList->port , ADR_TCP);
	      PDEBUG1("with address %s\n",anic);
	      it_nicRange[3]++;
	      if  (it_nicRange[3] == 256) {
		it_nicRange[3]=0;
		it_nicRange[2]++;
	      }
	    }
	    oldpnode=pnode;
	}
	free(c_digits);
	free(leadingZeros);
	free(tempname);
    } 
    
    free(prefix);
    free(suffix);

    return rangeNodeList;
}

int getNetId(char* name) {
  SnetDefList* nl = pars_rconf->netDefList;
  int i=0;
  while (nl) {
    if (strcmp(nl->name, name) == 0) 
      return i;
    nl=nl->next;
    i++;
  }
  return -1;
}

int saveNetList(SnetName * netnames, Snode *  node) {
  int i;
  SnetName * nn = netnames;
  while (nn) {
    node->numNets++;
    nn=nn->next;			  
  }
  node->netList= malloc (node->numNets*sizeof(int));

  nn=netnames;
  i=0;
  while (nn) {
    node->netList[i]=getNetId(nn->name);
    if (node->netList[i] == -1)
      RERROR1("undefined network %s ",nn->name);
    nn=nn->next;
    i++;
  }
}

/* reset globals for multiple parsing */
void resetGlobals() {
    countSubsections = 0;
    countHosts = 0;
    Bparse_conn=0;

    nrRouters=0;   

    numSections=0;

    nicDefList = 0;

    metaHostFound=0;
    routerCounter=0;   
    SCounter=1;
    ZCounter=1;

}

void yyerror(const char* s)
{
   fprintf(stderr, "error in meta config file, line %d near column %d %s ******\n",
	   ZCounter,SCounter,s);
}


