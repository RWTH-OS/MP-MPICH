/* $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 *
 * parstest is a commandline tool to test configuration files 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "router_config.h"


/*
   MPID_ArgSqueeze - Remove all null arguments from an arg vector; 
   update the number of arguments.

   copied from mpid/utils/cmnargs.c
 */
void MPID_ArgSqueeze( Argc, argv )
int  *Argc;
char **argv;
{
    int argc, i, j;

    /* Compress out the eliminated args */
    argc = *Argc;
    j    = 0;
    i    = 0;
    while (j < argc) {
	while (argv[j] == 0 && j < argc) j++;
	if (j < argc) argv[i++] = argv[j++];
    }
    /* Back off the last value if it is null */
    if (!argv[i-1]) i--;
    *Argc = i;
}

void usage(char * exec) {
  fprintf(stderr,"usage: %s [-yydebug] [-mh <metahostname>] <metaconfigfile>\n", exec);
  exit(1);
}

int main( int argc,char ** argv) {
    char metahostname[MPI_MAX_PROCESSOR_NAME];
    char * configFile;
    int debug=0;
    int i;

    struct RouterConfig router_cfg;

    strcpy(metahostname,"");
    for (i = 0; i < argc; i++) 
      if ((*argv)[i]) {
	/* debug parser  */
	if (strcmp( (argv)[i], "-yydebug" ) == 0) {
	  (argv)[i] = 0;
	  debug=1;	 
	} 
      }

    for (i = 0; i < argc; i++) 
      if ((argv)[i]) {
	/* debug parser  */
	if (strcmp( (argv)[i], "-mh" ) == 0) {
	  (argv)[i] = 0;
	  if ( i+1 < argc ) {
	    strcpy(metahostname,(argv)[i+1]);
	    argv[i+1] = 0;
	  } else {
	    fprintf(stderr,"name of metahost missing!\n");
	    usage(argv[0]);	    
	  }
	} 
      }
    MPID_ArgSqueeze(&argc, argv);

    if (argc == 2) {
    	configFile=argv[1];
    } else {
      fprintf(stderr,"meta config file missing!\n");
      usage(argv[0]);
    }
    
#ifdef bla

    if (argc > 3 || argc < 2) {
	fprintf(stderr,"usage: %s [metahostname] configfile\n", argv[0]);
	return 0;
    }
    if (argc == 3) {
    	configFile=argv[2];
	strcpy(metahostname,argv[1]);
    }
    if (argc == 2) {
    	configFile=argv[1];
	strcpy(metahostname,"");
    }
#endif
    if ( MPIR_read_metaconfig( configFile, &router_cfg, metahostname, 0, debug ) ) {
	printf("parsing OK!\n");
	rh_printMetahostList();
	if (strcmp(metahostname,"") !=0 )  {
		printf("====== routerlist meta host %s =======\n", metahostname);
		rh_printRouterList();
	}
    }
    else  { 
	printf("***** errors found! ******\n");
	return -1;
    }
    
    if (strcmp(metahostname,"") ==0 ) {
    	/* parse config for all meta hosts */
	int mh;
	for (mh=0; mh < router_cfg.nbr_metahosts; mh++) {
		strcpy(metahostname,router_cfg.metahostnames[mh]);
		if ( MPIR_read_metaconfig( configFile, &router_cfg, metahostname, 0,debug  ) ) {
	       		printf("====== routerlist meta host %s =======\n", metahostname);
			rh_printRouterList();
    		}
    		else  { 
			printf("***** errors found! ******\n");
			return -1;
    		}
	}
    }
    return 0;
}
