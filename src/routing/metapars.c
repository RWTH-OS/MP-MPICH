/* $Id$
 * (c) 2002 Lehrstuhl fuer Betriebssysteme, RWTH Aachen, Germany
 * author: Martin Poeppe  email: mpoeppe@gmx.de
 *
 * standalone parser which can be used by mpirun.meta to get the command lines
 * for the remote execution an the metahosts
 *  
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "metaconfig.h"
#include "mpichconf.h"

#include "router_config.h"
#include "newstrings.h"

/* #define METAPARS_DEB */

#ifdef METAPARS_DEB
#define PDEBUG(...) {fprintf(stderr, __VA_ARGS__); fflush(stderr);}
#else
#define PDEBUG(...) 
#endif

#define DELIMITER '%'
#define CMD_LINE_LEN 2048
#define CHECK_LEN( cmd , len ) {										\
		int r=cmd;														\
		if (r>=len) {fprintf(stderr,"metapars.c: commandline too long! Increase CMD_LINE_LEN.\n"); \
			exit( EXIT_FAILURE );} }

void createProcGroup(char *pgName, char * execPath, int h, char * metahostname) ;

int main( int argc,char ** argv) {
	int i;
	char ** cmdlines;
	char sourcecmd[200];
	char *wgetcmd=0;
	struct RouterConfig router_cfg;
	int np_override = 0;
	char *metahostname=0,*metaconfigfile;
	int pos;
	char  spid[256];
	char* machinefile=0;
	char *procgroupCreate=0;
	int quiet=0;
	char * pgName=0;
	char *execPath=0;
	char *runNodes=0;
	char *devOptions=0, *appArgs=0;

	sprintf(spid,"%d-%d",getpid(), getuid());
	if (argc < 2) {
	    fprintf(stderr,"usage: %s [ -q ] [-r URL] [-metaparam <n>] [-metahost metahostname] [ -createpg <pgfile>  <program-path>] configfile \n", argv[0]);
	    exit( EXIT_FAILURE );
	}
	pos = 2; /* expected position of configfile argument (executable name is expected at position 1), which should be the last argument */
	for (i = 1; i < argc; i++) {
	    if ((*argv)[i]) {
			if (strcmp( (argv)[i], "-metaparam" ) == 0) {
				if (i+1 >= argc ) {
					fprintf(stderr, "%s: number missing for -metaparam\n",argv[0]);
					exit( EXIT_FAILURE );
				}
				if (!sscanf(argv[i+1],"%d",&np_override)) {
					fprintf(stderr, "%s: cant read number for -metaparam\n",argv[0]);
					exit( EXIT_FAILURE );
				}
				pos+=2;
			}
			if (strcmp( (argv)[i], "-metahost" ) == 0) {
				if (i+1 >= argc ) {
					fprintf(stderr, "%s: name missing for -metahost\n",argv[0]);
					exit( EXIT_FAILURE );
				}
				metahostname=newString(argv[i+1]);
				pos+=2;
			}
			if (strcmp( (argv)[i], "-r" ) == 0) { /* remote config */
				if (i+1 >= argc ) {
					fprintf(stderr, "%s: url missing for -r\n",argv[0]);
					exit( EXIT_FAILURE );
				}
				wgetcmd=newStringAdd("/bin/wgetMetaConfig ", argv[i+1]);
				pos+=2;
			}
			if (strcmp( (argv)[i], "-createpg" ) == 0) { /* remote config */
				if (i+2 >= argc ) {
					fprintf(stderr, "%s filename or program-path missing for -createpg\n",argv[0]);
					exit( EXIT_FAILURE );
				}
				pgName=newString(argv[i+1]);
				PDEBUG("***metapars.c: pgName=%s\n", pgName);
				execPath=newString(argv[i+2]);
				quiet=1;
				pos+=3;
			}
			if (strcmp( (argv)[i], "-q" ) == 0) { /* remote config */
				quiet=1;
				pos+=1;
			}
		
	    }
	}
	
	if (!metahostname) metahostname=newString("");
	
	if ( argc > pos  ) {
	    fprintf(stderr, "%s: too many arguments\n",argv[0]);
	    exit(EXIT_FAILURE);
	}
	
	if ( argc < pos ) {
	    fprintf(stderr, "%s: metaconfig missing\n",argv[0]);
	    exit(EXIT_FAILURE);
	}
	
	metaconfigfile=argv[pos-1];
	
	if (!wgetcmd) {
	    wgetcmd=newString("");
	}
	
	
	if (MPIR_read_metaconfig( metaconfigfile, &router_cfg, metahostname, np_override, 0)) {
	    int h;
	    
	    /* we have to generate one mpirun command for each metahost */
	    cmdlines=(char**) malloc(sizeof (char*) * router_cfg.nbr_metahosts);

	    for (h=0; h< router_cfg.nbr_metahosts; h++) {
			char* frontend;
			char *wget;
			char *cl;
			Snode *pn;
		
			machinefile=newStringAdd4("tmp.machinefile.",metahostlist[h].hostname,".",spid);
			if (metahostlist[h].frontend != 0 && strcmp(metahostlist[h].frontend,"n.a.") != 0)  /* do we have a dedicated frontend? */
				frontend=metahostlist[h].frontend;
			else 
				frontend=metahostlist[h].nodeList->nodeName; /* just the first node in the list */
			if (metahostlist[h].user != 0 && strcmp(metahostlist[h].user,"n.a.") != 0) { /* is there a user name? */
				frontend = newStringAdd4( "-l ", metahostlist[h].user, " ", frontend );
			}
		
		
			cmdlines[h]= (char*) malloc(sizeof (char) * CMD_LINE_LEN); 
		
			if (metahostlist[h].execPath==0 || !strcmp(metahostlist[h].execPath, "n.a.") )
				metahostlist[h].execPath=newString( "@EXECPATH@");
			if (metahostlist[h].execName==0 || !strcmp(metahostlist[h].execName, "n.a.") )
				metahostlist[h].execName=newString( "@EXECNAME@");
		
			if (metahostlist[h].confPath==0 || !strcmp(metahostlist[h].confPath, "n.a.") )
				metahostlist[h].confPath=newString( "@CONFPATH@");
			if (metahostlist[h].confName==0 || !strcmp(metahostlist[h].confName, "n.a.") )
				metahostlist[h].confName=newString( "@CONFNAME@");
		
			if (metahostlist[h].mpiRoot==0 || !strcmp(metahostlist[h].mpiRoot, "n.a.")  )
				metahostlist[h].mpiRoot=newString("@MPIROOT@");
			if (metahostlist[h].envFile==0 || !strcmp(metahostlist[h].envFile, "n.a.")  )
				metahostlist[h].envFile=newString("@ENVFILE@");

			if (metahostlist[h].metaKey==0 || !strcmp(metahostlist[h].metaKey, "n.a.") )
				metahostlist[h].metaKey=newString("@METAKEY@");
		
			if (strlen(wgetcmd) != 0) {
				wget=newStringAdd3(metahostlist[h].mpiRoot, wgetcmd, " @CONFNAME@ ; ");
			} else wget=newString("");
		
			/* check if the procgroup can really be used with the given device */
			if (metahostlist[h].procgroup && ( (metahostlist[h].deviceType == DEVICE_SMI) || (metahostlist[h].deviceType == DEVICE_SHMEM) || (metahostlist[h].deviceType == DEVICE_USOCK) || (metahostlist[h].deviceType == DEVICE_MPX) ) ) {
				fprintf(stderr,"metapars error: procgroup files are not supported by %s.\n",deviceTypeStrings[metahostlist[h].deviceType]);
				exit( EXIT_FAILURE );
			}
			/* create procgoup-file, if -createpg is given AND we have a specifed metahost */		
			createProcGroup( pgName,  execPath,  h, metahostname)  ;
			if (metahostlist[h].procgroup) {
				procgroupCreate=newStringAdd(metahostlist[h].mpiRoot,"/bin/metapars");
				procgroupCreate=newStringAdd4(procgroupCreate, " -metahost ", metahostlist[h].hostname," -createpg ");
				procgroupCreate=newStringAdd3(procgroupCreate, machinefile, " ");
				procgroupCreate=newStringAdd4(procgroupCreate, metahostlist[h].execPath, "/", metahostlist[h].execName);
				procgroupCreate=newStringAdd(procgroupCreate, " ");
				procgroupCreate=newStringAdd4(procgroupCreate, metahostlist[h].confPath, "/", metahostlist[h].confName);
				procgroupCreate=newStringAdd(procgroupCreate, ";");
				runNodes=newStringAdd("-pg ",machinefile);
				PDEBUG("***metapars.c: procgroupCreate=%s\n", procgroupCreate);
		    
			} else 	{ /* traditional -nodes ..... */
				int used=0;
		    
				procgroupCreate=newString("");
				pn=metahostlist[h].nodeList;
				/* here we construct the list of nodes on which the processes are to be run; for every
				   process we make an entry in this list, so that we have total control over the way
				   the device starts the processes */
				runNodes=newStringSize(0, CMD_LINE_LEN);
				CHECK_LEN(snprintf(runNodes, CMD_LINE_LEN, " -np %d", router_cfg.np_metahost[h]+router_cfg.nrp_metahost[h] ), CMD_LINE_LEN);
				used=strlen(runNodes);
				cl=runNodes+used;
#if 0
				if ( metahostlist[h].deviceType != DEVICE_SHMEM) /* ch_shmem priviously did not support -nodes */
#endif
				{ 
					CHECK_LEN(snprintf(cl, CMD_LINE_LEN-used , " -nodes "), CMD_LINE_LEN-used);
					used=strlen(runNodes);
					cl=runNodes+used;
			
					while(pn) {
						int i;
			    
						/* for every process that runs on this node, the name of the node is written to the command
						   line once */
						for( i = 0; i < (pn->np + pn->numRouters); i++ ) {
							CHECK_LEN(snprintf(cl,CMD_LINE_LEN-used,"%s",pn->nodeName), CMD_LINE_LEN-used);
							cl += strlen(pn->nodeName);
							used+=strlen(pn->nodeName)*sizeof(char);
				
							if( i < (pn->np + pn->numRouters) - 1 ) {
								CHECK_LEN(snprintf( cl, CMD_LINE_LEN-used,"," ), CMD_LINE_LEN-used);
								cl++;
								used+=sizeof(char);
							}
						}
						if( (pn->np + pn->numRouters) && pn->next ) {
							CHECK_LEN(snprintf( cl,CMD_LINE_LEN-used, "," ), CMD_LINE_LEN-used);
							cl++; used+=sizeof(char);
						}
						pn=pn->next;
					}
				}
			}
		
			/* create command line */		
			sourcecmd[0] = '\0';
			cl=cmdlines[h];
			devOptions=metahostlist[h].devOptions?metahostlist[h].devOptions:newString("");
			appArgs=metahostlist[h].appArgs?metahostlist[h].appArgs:newString("");
			if( strcmp( metahostlist[h].envFile, "@ENVFILE@" ) != 0 )
				CHECK_LEN(snprintf( sourcecmd,sizeof(sourcecmd), "source %s; ", metahostlist[h].envFile ) ,sizeof(sourcecmd));
		
			CHECK_LEN(snprintf(cmdlines[h], CMD_LINE_LEN, "%s%c cd %s; %s %s %s %s/bin/mpirun @SHOW@ @VERBOSE@ -machine %s %s %s %s @CMDLINEARGS@ %s -- -metahost %s  -metaparam %d -metarun %s/%s -metakey %s", 
							   frontend, /* run mpirun on this node */
							   DELIMITER,
							   metahostlist[h].execPath,
							   wget,
							   sourcecmd,
							   procgroupCreate,
							   metahostlist[h].mpiRoot,
							   deviceTypeStrings[metahostlist[h].deviceType],
							   devOptions,
							   runNodes,
							   metahostlist[h].execName,
							   appArgs, metahostlist[h].hostname, np_override,
							   metahostlist[h].confPath,
							   metahostlist[h].confName,
							   metahostlist[h].metaKey
							   ), CMD_LINE_LEN);
			cl=cl+ strlen(cmdlines[h]);
		
			/***********************/
		
			free(wget);
			if (!quiet) 
				printf("%s%c",cmdlines[h],DELIMITER);
		
	    }
	    free(wgetcmd);
	    exit( EXIT_SUCCESS );
	    
	}
	else {
	    fprintf(stderr, "***** errors found - check your meta configuration file! ******\n");
	    free(wgetcmd);
	    exit( EXIT_FAILURE );
	}    
}

void createProcGroup(char *pgName, char *execPath, int h, char *metahostname) {
	Snode *pn;
	
	if (metahostlist[h].procgroup && pgName!= 0 && strcmp(metahostname,metahostlist[h].hostname)==0) {
		FILE *pgFile=0;
		int i=0;
		int routerCounter=0;

		pgFile=fopen(pgName,"w");
		if (!pgFile) {
			fprintf(stderr,"metapars: could not create procgroup file!\n");
			perror("");
			exit( EXIT_FAILURE );
		}

		pn=metahostlist[h].nodeList;
		/* here we construct the list of nodes on which the processes have to be run; for every
		   process we make an entry in this list, so that we have total control over the way
		   the ch_usock-device starts the processes */
		while(pn) {						
			/* for every process that runs on this node, the name of the node is written to the command
			   line once and the number of processes, except the first which has -1 */
			/* now print the router procs */
			if (pn->numRouters) {
				int router;
				for (router=0; router < pn->numRouters; router++) {
					fprintf(pgFile,"%s %d",pn->nodeName, 1 - ((i==0)?1:0));
					/* check for dedicated router */
					if (routerlist[pn->routerIds[routerCounter]].routerExec) {
						fprintf(pgFile," %s", routerlist[pn->routerIds[routerCounter]].routerExec);
					}
					else {
						if (pn->executable){
							fprintf(pgFile," %s", pn->executable);
							/* following is not working yet, since we do not use procfiles in e.g. ch_smi */
							/*fprintf(pgFile," %s", pn->args);*/
						}
						else 
							fprintf(pgFile," %s", execPath);
					}
					fprintf(pgFile,"\n"); i++;
					routerCounter++;
				}
			}
			
			/* application processes */
			if ((pn->np - pn->npExtraProcs >0 )) {
				fprintf(pgFile,"%s %d",pn->nodeName, pn->np - pn->npExtraProcs - ((i==0)?1:0));
				if (pn->executable){
					fprintf(pgFile," %s", pn->executable);
					PDEBUG("***metapars.c: pn->executable=%s\n", pn->executable);
					/* following is not working yet, since we do not use procfiles in e.g. ch_smi */
					/*fprintf(pgFile," %s", pn->args);*/
				}
				else 
					fprintf(pgFile," %s", execPath);
				fprintf(pgFile,"\n"); i++;
			}
			/*now print the extra processes, they have been excluded above*/
			if(pn->npExtraProcs){
				int j;
				for (j=0; pn->extraProcList[j]; j++)
					{
						if(i==0 && j==0)
							{
								int k, num;
								for(k=0; k<strlen(pn->extraProcList[j])-1; k++)
									{
										if((pn->extraProcList[j])[k]==' ')
											{	
												if(((pn->extraProcList[j])[k+1]>0x30) &&
												   ((pn->extraProcList[j])[k+1]<0x39))
													{
														(pn->extraProcList[j])[k+1]--;
													}
											}
									}
				    
							}
						fprintf(pgFile,"%s", pn->extraProcList[j]);
					}

				i += j;
				fprintf(pgFile,"\n");	 i++;
			}
			pn=pn->next;
		}
		
		fclose(pgFile);
	}
}

/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
