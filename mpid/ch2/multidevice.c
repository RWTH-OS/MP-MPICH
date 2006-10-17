/* $Id$ */
#ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <netinet/in.h>     /* in_addr structure */

/* for run-time resolution of symbols */
#include <dlfcn.h>
#include <link.h>

#include "mpi_errno.h"
#include "mpichconf.h"
#include "mpimem.h"
#include "metaconfig.h"
#include "../../src/routing/rhlist.h"

#include "multidevice.h"

#define USOCK_DEVICE_ARG_MAX_LEN 200
#define SMI_DEVICE_ARG_MAX_LEN 200

/* #define MULTIDEVDEBUG */
#ifdef MULTIDEVDEBUG
#define PDEBUG(...) {fprintf(stderr, __VA_ARGS__); fflush(stderr);}
#else
#define PDEBUG(...) 
#endif


#endif /* !WIN32 */

int MPID_selected_primary_device;

#ifndef WIN32

int *MPID_SecondaryDevice_grank_to_devlrank;
int MPID_SecondaryDevice_devsize;
int MPID_SecondaryDevice_argc;
char **MPID_SecondaryDevice_argv;
int MPID_SecondaryDevice_type;
char MPID_SecondaryDevice_cmdline[300];

Snode *MPID_SecondaryDevice_node;

extern int MPID_MyHostRank;

/* prototypes for internal functions */
void *MPID_Get_SymbolPt( const char *, const char *, int * );
int MPID_buildSecondaryDeviceArgs_usock( char *** );
int MPID_secondaryDevice_IP_matches_netmask( const char * );
int MPID_buildSecondaryDeviceArgs_smi( char *** );

/* function to get first IP address of host hostname
 * needs:
 * #include <netdb.h>          // hostent struct, gethostbyname()
 * #include <arpa/inet.h>      // inet_ntoa() to format IP address
 * #include <netinet/in.h>     // in_addr structure
 */ 
char* getFirstIPAddress(const char* hostname)
{
    struct hostent *host;		/* host information */
    struct in_addr in;    		/* internet address */
	if ((host = gethostbyname(hostname)) == NULL) {
		return NULL;
	}
	in.s_addr = *((unsigned long *) host->h_addr_list[0]);
	return inet_ntoa(in);
}

/* function to translate device type number as it derived from the metahost type and 
   secondary device type in a metaconfig file to the device number as it is configured
   in mpichconf.h; error is MPI_SUCCESS if device type is supported and MPI_ERR_INTERN
   otherwise */
int MPID_GetDeviceNbr( int deviceType, int *error )
{
    int deviceNbr;

    *error = MPI_SUCCESS;

    switch( deviceType ) {

#ifdef CH_SMI_PRESENT
    case DEVICE_SMI:
	deviceNbr = DEVICE_ch_smi_nbr;
	break;
#endif

#ifdef CH_SHMEM_PRESENT
    case DEVICE_SHMEM:
	deviceNbr = DEVICE_ch_shmem_nbr;
	break;
#endif

#ifdef CH_USOCK_PRESENT
    case DEVICE_USOCK:
	deviceNbr = DEVICE_ch_usock_nbr;
	break;
#endif

#ifdef CH_MPX_PRESENT
    case DEVICE_MPX:
	deviceNbr = DEVICE_ch_mpx_nbr;
	break;
#endif

#ifdef CH_P4_PRESENT
    case DEVICE_P4:
	deviceNbr = DEVICE_ch_p4_nbr;
	break;
#endif

#ifdef CH_GM_PRESENT
    case DEVICE_GM:
	deviceNbr = DEVICE_ch_gm_nbr;
	break;
#endif

    default:
	fprintf( stderr, "Selected device was not built into the library\n" );
	fflush( stderr );
	*error = MPI_ERR_INTERN;
	break;
    }

    return deviceNbr;
}

/* Function to get pointer to device intialization function;
   it must be given the device type number (as configured in mpichconf.h);
   error_code is MPI_SUCCESS if fucntion was found and MPI_ERR_INTERN otherwise */
void *MPID_GetInitMsgPassPt( int device_type_nbr, int *error_code )
{
    void *InitMsgPassPt; 

    *error_code = MPI_SUCCESS;

    switch( device_type_nbr ) {
#ifdef CH_SMI_PRESENT
    case DEVICE_ch_smi_nbr:
	InitMsgPassPt = MPID_Get_SymbolPt( "MPID_CH_SMI_InitMsgPass", "libch_smi.so", error_code );
       break;
#endif

#ifdef CH_SHMEM_PRESENT
   case DEVICE_ch_shmem_nbr:
       InitMsgPassPt = MPID_Get_SymbolPt( "MPID_CH_SHMEM_InitMsgPass", "libch_shmem.so", error_code );
       break;
#endif

#ifdef CH_P4_PRESENT	    
   case DEVICE_ch_p4_nbr:
       InitMsgPassPt = MPID_Get_SymbolPt( "MPID_CH_P4_InitMsgPass", "libch_p4.so", error_code );
       break;
#endif

#ifdef CH_GM_PRESENT	    
   case DEVICE_ch_gm_nbr:
       InitMsgPassPt = NULL; /* this is not an error, ch_gm is treated differently */
       break;
#endif

#ifdef CH_USOCK_PRESENT	    
   case DEVICE_ch_usock_nbr:
       InitMsgPassPt = MPID_Get_SymbolPt( "MPID_CH_USOCK_InitMsgPass", "libch_usock.so", error_code );
       break;
#endif

#ifdef CH_MPX_PRESENT	    
   case DEVICE_ch_mpx_nbr:
       InitMsgPassPt = MPID_Get_SymbolPt( "MPID_CH_MPX_InitMsgPass", "libch_mpx.so", error_code );
       break;
#endif

   default:
       fprintf( stderr, "Selected device was not built into the library\n" );
       fflush( stderr );
       *error_code = MPI_ERR_INTERN;
       break;
   }

    return InitMsgPassPt;
}

#ifdef WIN32

/*
  Windows doesn't support the dlopen()/dlsym()/dlerror() functions, instead it implements its own
  API to get a pointer to a function in an executable module (LoadLibrary(), GetProcAddress() ... ).
  Because of this, a Windows version of MPID_Get_SymbolPt() has to be implemented.
*/
#else

/*
  This function can be used to look for the initialization functions for the different
  devices in a multi-device configuration. It looks for <symbolName> in the main program
  and afterwards, if it has not been found, in <dllName>. A pointer to the symbol is returned if
  the symbol has been found (please note: a NULL pointer may be valid!). <*error_code> is set to
  MPI_ERR_INTERN if something went wrong.
*/
void *MPID_Get_SymbolPt( symbolName, dllName, error_code )
    const char *symbolName;
    const char *dllName;
    int *error_code;
{
    void *dll_handle = 0;
    char *dlsym_return_msg;
    void *symbol_pointer = 0;
    
    *error_code = MPI_SUCCESS;

    /* get handle for main program*/
    if( (dll_handle = dlopen( 0, RTLD_NOW )) == 0 ) {
	/* this is an error, handle for main program should always be available */ 
	*error_code = MPI_ERR_INTERN;
	return;
    }

    /* dlerror() returns pointer to an error message if something went wrong during dlsym();
       a NULL pointer returned by dlsym() may be valid, see manpage of dlsym() for details */
    dlsym_return_msg = 0;
    dlerror(); /* clean up old error messages */
    symbol_pointer = dlsym( dll_handle, symbolName );
    dlsym_return_msg = dlerror();

    if( dlsym_return_msg != 0 ) {
	dll_handle = dlopen( dllName, RTLD_LAZY );
	if( dll_handle != 0 )
	    dlsym_return_msg = 0;
	dlerror();
	symbol_pointer = dlsym( dll_handle, symbolName );
	dlsym_return_msg = dlerror();
    }
    if( dlsym_return_msg != 0 ) {
	*error_code = MPI_ERR_INTERN;
	return;
    }

    return symbol_pointer;
}

#endif /* !WIN32 */

#ifdef META

/* This function initializes
   MPID_SecondaryDevice_devsize (number of process that make use of the secondary device) and
   MPID_SecondaryDevice_grank_to_devlrank (mapping from global ranks to device relative ranks) */

int MPID_buildRankMappingForSecondaryDevice( void )
{
    int mh, mhrank, grank, devlrank;

    /* calculate the number of processes that use the secondary device; every process
       on any metahosts that uses the secondary device is counted */
    MPID_SecondaryDevice_devsize = 0;
    for( mh = 0; mh < MPIR_meta_cfg.nbr_metahosts; mh++ )
	if( MPIR_meta_cfg.metahostUsesSecondaryDevice[mh] == 1 )
	    MPID_SecondaryDevice_devsize += ( MPIR_meta_cfg.np_metahost[mh] + MPIR_meta_cfg.nrp_metahost[mh] );

    /* build rank mapping; processes ordered in linear fashion, hopping over those metahosts
       that don't use the secondary device */
    MPID_SecondaryDevice_grank_to_devlrank = (int *)MALLOC( ( MPIR_meta_cfg.np + MPIR_meta_cfg.nrp ) * sizeof(int) );
    if( !MPID_SecondaryDevice_grank_to_devlrank )
	return MPI_ERR_INTERN;

    devlrank = grank = 0;
    for( mh = 0; mh < MPIR_meta_cfg.nbr_metahosts; mh++ ) {
	if( MPIR_meta_cfg.metahostUsesSecondaryDevice[mh] == 1 ) {
	    for( mhrank = 0; mhrank < ( MPIR_meta_cfg.np_metahost[mh] + MPIR_meta_cfg.nrp_metahost[mh] ); mhrank++ ) {
		MPID_SecondaryDevice_grank_to_devlrank[grank] = devlrank;
		devlrank++;
		grank++;
	    }
	}
	else
	    grank += ( MPIR_meta_cfg.np_metahost[mh] + MPIR_meta_cfg.nrp_metahost[mh] );
    }

    return MPI_SUCCESS;
}

/* This Function creates the command line arguments for the secondary device (MPID_SecondaryDevice_argc
   and MPID_SecondaryDevice_argv). The device type must be supplied in the first argument and a pointer 
   to the "real" argument array (argv) in the second argument. In case of success, MPI_SUCCESS is returned and
   MPI_ERR_INTERN if anything goes wrong. Note: Currently only ch_usock is supported.  */

int MPID_buildSecondaryDeviceArgs( int deviceType, char ***realargv )
 {
    /* Call device-specific function */ 
    switch( deviceType ) {
    case DEVICE_SMI:
        /*return MPI_ERR_INTERN;*/
        return MPID_buildSecondaryDeviceArgs_smi( realargv );
    case DEVICE_SHMEM:
	return MPI_ERR_INTERN;
    case DEVICE_USOCK:
	return MPID_buildSecondaryDeviceArgs_usock( realargv );
	break;
    case DEVICE_MPX:
        return MPI_ERR_INTERN;
    case DEVICE_P4:
        return MPI_ERR_INTERN;
    case DEVICE_GM:
	return MPI_ERR_INTERN;
    default:
        return MPI_ERR_INTERN;
    }
}

/* This function builds the command line arguments for ch_usock as secondary device. A pointer to the "real"
   argument array (argv) must be supplied in the second argument. In case of success, MPI_SUCCESS is returned and
   MPI_ERR_INTERN if anything goes wrong. */

int MPID_buildSecondaryDeviceArgs_usock( char ***realargv )
{
    int grank, mh, device_lrank, i, found;
    Snode *master_node, *my_node;
    Snic *my_nic, *master_nic;
    char masterNicAddress[30];
    int fd, portbase;
    int socket_option_value;
    struct sockaddr_in address;
    const char* ip;
    
    PDEBUG("Starting MPID_buildSecondaryDeviceArgs_usock()\n", MPID_SecondaryDevice_cmdline );

    /* find first metahost that uses the secondary device */
    mh = 0;
    while( MPIR_meta_cfg.metahostUsesSecondaryDevice[mh] == 0 )
	mh++;
    if( mh > MPIR_meta_cfg.nbr_metahosts - 1 ) {
	fprintf( stderr, "MultiDevice-Error: Wrong metahost number\n" );
	fflush( stderr );

      return MPI_ERR_INTERN; /* something's wrong, maybe no secondary device needed */
    }

    /* make first node of that metahost master node (on which the 
       master process runs) */
    master_node = metahostlist[mh].nodeList;
    
    master_nic = master_node->nicList;
    
    if (master_nic == NULL)
    {
	    /* if NULL, use first found IP-Address */
		master_nic = (Snic*) malloc(sizeof(Snic));
		master_nic->nicType == ADR_TCP;
		master_nic->nodeName = metahostlist[mh].nodeList[0].nodeName;
    	ip = getFirstIPAddress(metahostlist[mh].nodeList[0].nodeName);
		if (ip)
			memcpy(&master_nic->nicAddress, ip, strlen(ip));

		PDEBUG("Multidevice.c: Master: nodeName:%s IP:%s\n", metahostlist[mh].nodeList[0].nodeName, master_nic->nicAddress);

		if (master_nic->nicAddress == NULL) { 
			fprintf( stderr, "MultiDevice-Error: Could not resolve hostname %s\n", metahostlist[mh].nodeList[0].nodeName);
			fflush( stderr );
			return MPI_ERR_INTERN;
		}
    }
    else
    /* try all TCP NICs on node */
    	for (master_nic = master_node->nicList; master_nic != NULL; master_nic = master_nic->next) {
			if( master_nic->nicType == ADR_TCP ) {
	    		/* if a netmask was specified and master_nic doesn't match it, we try the next NIC */
	    		if ((MPIR_meta_cfg.secondaryDeviceOpts.netmask_no_host_bits != -1) && !MPID_secondaryDevice_IP_matches_netmask( master_nic->nicAddress ))
					continue;
	    		break;
			}
    	}

  	if (master_nic == NULL) {
		fprintf( stderr, "MultiDevice-Error: No TCP NIC found for master node\n" );
		fflush( stderr );
		return MPI_ERR_INTERN;
	}

    strncpy( masterNicAddress, master_nic->nicAddress, 30 );

    /* get device rank for secondary device */
    grank = MPIR_meta_cfg.metahost_firstrank + MPIR_meta_cfg.my_rank_on_metahost;
    device_lrank = MPID_SecondaryDevice_grank_to_devlrank[grank];

    /* allocate space for device arguments */
    MPID_SecondaryDevice_argc = 9;
    MPID_SecondaryDevice_argv = (char **)MALLOC( MPID_SecondaryDevice_argc * sizeof(char *) );
    for( i = 0; i < MPID_SecondaryDevice_argc; i++ )
	MPID_SecondaryDevice_argv[i] = (char *)MALLOC( USOCK_DEVICE_ARG_MAX_LEN * sizeof(char) );

    /* name of executable */
    strncpy( MPID_SecondaryDevice_argv[0], (*realargv)[0], USOCK_DEVICE_ARG_MAX_LEN );

    /* device relative process rank */
    strcpy( MPID_SecondaryDevice_argv[1], "-r" );
    sprintf( MPID_SecondaryDevice_argv[2], "%d", device_lrank );

    /* Important note: Here we assume that the process with device relative rank 0 runs on the
       "master node" which we have chosen above and so we make it the master process. According to the
       way in which we build the rank mapping for the secondary device (in MPID_buildRankMappingForSecondaryDevice() ),
       this is the process with rank 0 in MPI_COMM_HOST running on the first metahost that makes use of
       the secondary device. Thus it must me assured that the "second stage" mpirun places that process 
       on the first node of that mnetahost. This assumption does hold for all our primary devices as far as I can
       see. A cleaner way to do this would be to somehow "ask" the primary device on which node its first process
       is running, but I can see no simple way to implement this at the moment. */

    if( device_lrank == 0 ) {
	/* master process gets the number of processes that are known to the secondary device */
	strcpy( MPID_SecondaryDevice_argv[3], "-n" );
	sprintf( MPID_SecondaryDevice_argv[4], "%d", MPID_SecondaryDevice_devsize );
    }
    else {
	/* the other processes get the name of the host on which the master process is running */
	strcpy( MPID_SecondaryDevice_argv[3], "-m" );
	sprintf( MPID_SecondaryDevice_argv[4], "%s", masterNicAddress );
    }

    /* port number */
    portbase = MPIR_meta_cfg.secondaryDeviceOpts.portbase;

    if(portbase == -1) {
      int portRangeLow  = MPIR_meta_cfg.secondaryDeviceOpts.portRangeLow;
      int portRangeHigh = MPIR_meta_cfg.secondaryDeviceOpts.portRangeHigh;

      if( (portRangeLow == -1) || (portRangeHigh == -1) || (portRangeHigh < portRangeLow) ) {
	portRangeLow = 49152;
	portRangeHigh= 65535;
      }

      /* select a "random" port out of the range: */
      portbase = portRangeLow + (atoi(MPIR_meta_cfg.metakey)%(portRangeHigh-portRangeLow));
    }

    strcpy( MPID_SecondaryDevice_argv[5], "-p" );
    sprintf( MPID_SecondaryDevice_argv[6], "%d", portbase );
    
    /* find my node */
    if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
	fprintf( stderr, "MultiDevice-Error: Could not create socket\n" );
	fflush( stderr );

	return MPI_ERR_INTERN;
    }

    /* we don't want to actually do anything with this socket */ 
    socket_option_value = 1;
    if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &socket_option_value, sizeof(socket_option_value) ) != 0 ) {
	fprintf( stderr, "MultiDevice-Error: Could not set socket option\n" );
	fflush( stderr );

	return MPI_ERR_INTERN;
    }

    address.sin_family = AF_INET;
    /* address.sin_port = htons( portbase + MPID_MyHostRank ); */
    /* --> Let the OS chose a free port: */
    address.sin_port = 0;   

    my_node = metahostlist[MPIR_meta_cfg.my_metahost_rank].nodeList;
    found = 0;
    while( my_node != NULL ) {
	
	if (my_node->nicList == NULL) {
	    /* if NULL, use first found IP-Address */
		my_node->nicList = (Snic*) malloc(sizeof(Snic));
		my_node->nicList->nicType == ADR_TCP;
		my_node->nicList->nodeName = my_node->nodeName;
    	ip = getFirstIPAddress(my_node->nodeName);
		if (ip)
			memcpy(&my_node->nicList->nicAddress, ip, strlen(ip));

		PDEBUG("Multidevice.c: nodeName:%s IP:%s\n", my_node->nodeName, my_node->nicList->nicAddress);

		if (my_node->nicList->nicAddress == NULL) { 
			fprintf( stderr, "MultiDevice-Error: Could not resolve hostname %s\n", my_node->nodeName);
			fflush( stderr );
			return MPI_ERR_INTERN;
		}
	}

	/* try all TCP NICs on node */
	for( my_nic = my_node->nicList; my_nic != NULL; my_nic = my_nic->next ) {
	    if( my_nic->nicType == ADR_TCP ) {
		/* if a netmask was specified and my_nic doesn't match it, we try the next NIC */
		if( (MPIR_meta_cfg.secondaryDeviceOpts.netmask_no_host_bits != -1) && !MPID_secondaryDevice_IP_matches_netmask( my_nic->nicAddress ) )
		    continue;
		address.sin_addr.s_addr = inet_addr( my_nic->nicAddress );
		if( bind( fd, (struct sockaddr *)&address, sizeof(address) ) == 0 ) {
		    /* found my node */
		    close( fd );
		    found = 1;
		    break;
		}
	    }
	}
	if( found )
	    break;
	my_node = my_node->next;
    }

    if( my_node == NULL ) {
	fprintf( stderr, "MultiDevice-Error: Unable to find my node\n" );
	fflush( stderr );

      return MPI_ERR_INTERN;
    }

    /* bind to address */
    strcpy( MPID_SecondaryDevice_argv[7], "-b" );
    sprintf( MPID_SecondaryDevice_argv[8], "%s", my_nic->nicAddress );

    /* this is for verbose output */
    MPID_SecondaryDevice_type = DEVICE_USOCK;
    sprintf( MPID_SecondaryDevice_cmdline, "%s %s %s %s %s %s %s %s %s",
	     MPID_SecondaryDevice_argv[0], MPID_SecondaryDevice_argv[1],
	     MPID_SecondaryDevice_argv[2],MPID_SecondaryDevice_argv[3],
	     MPID_SecondaryDevice_argv[4], MPID_SecondaryDevice_argv[5],
	     MPID_SecondaryDevice_argv[6], MPID_SecondaryDevice_argv[7],
	     MPID_SecondaryDevice_argv[8] );
	     
    PDEBUG("Ending MPID_buildSecondaryDeviceArgs_smi() with cmdLine:\n%s\n", MPID_SecondaryDevice_cmdline );
	return MPI_SUCCESS;
}

/* This function returns 1 if the given IP-Address-String matches the netmask
   saved in MPIR_meta_cfg.secondaryDeviceOpts, 0 otherwise */
int MPID_secondaryDevice_IP_matches_netmask( const char *IPString )
{
    uint32_t ip_address;

    ip_address = htonl( inet_addr( IPString ) );
    ip_address = (ip_address >> MPIR_meta_cfg.secondaryDeviceOpts.netmask_no_host_bits) << MPIR_meta_cfg.secondaryDeviceOpts.netmask_no_host_bits;

    if( MPIR_meta_cfg.secondaryDeviceOpts.netmask_ip ^ ip_address )
	return 0;
    else
	return 1;
}


int MPID_buildSecondaryDeviceArgs_smi( char ***realargv )
{
    int grank, mh, device_lrank, i, id;
    Snode *master_node;
    
    PDEBUG("Starting MPID_buildSecondaryDeviceArgs_smi()\n", MPID_SecondaryDevice_cmdline );

    /* find first metahost that uses the secondary device */
    mh = 0;
    while( MPIR_meta_cfg.metahostUsesSecondaryDevice[mh] == 0 )
	mh++;
    if( mh > MPIR_meta_cfg.nbr_metahosts - 1 ) {
	fprintf( stderr, "MultiDevice-Error: Wrong metahost number\n" );
	fflush( stderr );

      return MPI_ERR_INTERN; /* something's wrong, maybe no secondary device needed */
    }

    /* make first node of that metahost master node (on which the 
       master process runs) */
    master_node = metahostlist[mh].nodeList;

    /* get device rank for secondary device */
    grank = MPIR_meta_cfg.metahost_firstrank + MPIR_meta_cfg.my_rank_on_metahost;
    device_lrank = MPID_SecondaryDevice_grank_to_devlrank[grank];

    /* allocate space for device arguments */
    MPID_SecondaryDevice_argc = 13;
    MPID_SecondaryDevice_argv = (char **)MALLOC( MPID_SecondaryDevice_argc * sizeof(char *) );
    for( i = 0; i < MPID_SecondaryDevice_argc; i++ )
	MPID_SecondaryDevice_argv[i] = (char *)MALLOC( SMI_DEVICE_ARG_MAX_LEN * sizeof(char) );

    /* name of executable */
    strncpy( MPID_SecondaryDevice_argv[0], (*realargv)[0], SMI_DEVICE_ARG_MAX_LEN );

	/* number of processes that are known to the secondary device */
	strcpy( MPID_SecondaryDevice_argv[1], "-n" );
	sprintf( MPID_SecondaryDevice_argv[2], "%d", MPID_SecondaryDevice_devsize );

    /* device relative process rank */
    strcpy( MPID_SecondaryDevice_argv[3], "-r" );
    sprintf( MPID_SecondaryDevice_argv[4], "%d", device_lrank );

	/* the name of the host on which the master process is running */
	strcpy( MPID_SecondaryDevice_argv[5], "-h" );
	sprintf( MPID_SecondaryDevice_argv[6], "%s", master_node->nodeName );

	/* id-number for process */
	id = atoi(MPIR_meta_cfg.metakey) % 99;
	strcpy( MPID_SecondaryDevice_argv[7], "-m" );
	sprintf( MPID_SecondaryDevice_argv[8], "%d", id );

	if (MPIR_meta_cfg.secondaryDeviceOpts.nowatchdog == 1)
		strcpy( MPID_SecondaryDevice_argv[9], "-w" );

	if (MPIR_meta_cfg.secondaryDeviceOpts.smi_verbose == 1)
		strcpy( MPID_SecondaryDevice_argv[10], "-v" );

	if (MPIR_meta_cfg.secondaryDeviceOpts.smi_debug == 1)
		strcpy( MPID_SecondaryDevice_argv[11], "-s" );
	
	/* end of SMI-params */
	strcpy( MPID_SecondaryDevice_argv[12], "--" );

    /* this is for verbose output */
    MPID_SecondaryDevice_type = DEVICE_SMI;
    sprintf( MPID_SecondaryDevice_cmdline, "%s %s %s %s %s %s %s %s %s %s %s %s %s",
	     MPID_SecondaryDevice_argv[0], MPID_SecondaryDevice_argv[1],
	     MPID_SecondaryDevice_argv[2], MPID_SecondaryDevice_argv[3],
	     MPID_SecondaryDevice_argv[4], MPID_SecondaryDevice_argv[5],
	     MPID_SecondaryDevice_argv[6], MPID_SecondaryDevice_argv[7],
	     MPID_SecondaryDevice_argv[8], MPID_SecondaryDevice_argv[9],
	     MPID_SecondaryDevice_argv[10], MPID_SecondaryDevice_argv[11],
	     MPID_SecondaryDevice_argv[12]  );

    PDEBUG("Ending MPID_buildSecondaryDeviceArgs_smi() with cmdLine:\n%s\n", MPID_SecondaryDevice_cmdline );

    return MPI_SUCCESS;
}

#endif /* META */

#endif /* !WIN32 */
