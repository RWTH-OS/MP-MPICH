/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef WIN32
#define strcasecmp stricmp
#endif

#define GM_STRONG_TYPES 0

#include "gmpi.h"
#include "gmpi_smpi.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "bnr.h"

#ifndef GM_ORIGINAL
#include "gmchannel.h"
#endif

int MPID_GM_rank, MPID_GM_size;

MPID_Config * MPID_GM_GetConfigInfo (int *, char ***);

static void 
gmpi_getenv (const char *varenv, char **result, char *msg, 
	     unsigned int required)
{
  *result = (char *) getenv (varenv);
  if ((required == 1) && (*result == NULL))
    {
      fprintf (stderr, "<MPICH-GM> Error: Need to obtain %s in %s !\n",
	       msg, varenv);
      gmpi_abort (0);
    }
}


static void
gmpi_allocate_port (int *board_number, unsigned int *port_number)
{
  unsigned int board_id, port_id, count;
  gm_status_t status = GM_SUCCESS;
  
  /* do we know which board to use ? */
  if (*board_number < 0) { 
    for (count = 0; count < 3; count++) {
      for (port_id = 2; port_id < GMPI_MAX_GM_PORTS; port_id++) {
	if (port_id != 3) {
	  for (board_id = 0; board_id < GMPI_MAX_GM_BOARDS; board_id++) {
	    status = gm_open (&gmpi_gm_port, board_id, port_id, 
			      "MPICH-GM", GM_API_VERSION);
	    if (status == GM_SUCCESS) {
	      *board_number = board_id;
	      *port_number = port_id;
	      return;
	    } else {
	      if (status == GM_INCOMPATIBLE_LIB_AND_DRIVER) {
		fprintf (stderr, "[%d] Error: The GM driver and the lib "
			 "used to compile MPICH-GM are not compatible ! "
			 "Please recompile MPICH-GM with a GM lib compatible "
			 "with the current GM driver.\n", MPID_GM_rank);
		gmpi_abort (0);
	      }
	    }
	  }
	}
      }
    }
  }else {
    /* open a port on the allocated board */
    board_id = *board_number;
    for (count = 0; count < 3; count++) {
      for (port_id = 2; port_id < GMPI_MAX_GM_PORTS; port_id++) {
	if (port_id != 3) {
	  status = gm_open (&gmpi_gm_port, board_id, port_id, 
			    "MPICH-GM", GM_API_VERSION);
	  if (status == GM_SUCCESS) {
	    *port_number = port_id;
	    return;
	  } else {
	    if (status == GM_INCOMPATIBLE_LIB_AND_DRIVER) {
	      fprintf (stderr, "[%d] Error: The GM driver and the lib "
		       "used to compile MPICH-GM are not compatible ! "
		       "Please recompile MPICH-GM with a GM lib compatible "
		       "with the current GM driver.\n", MPID_GM_rank);
	      gmpi_abort (0);
	    }
	  }
	}
      }
    }
  }
}


static void
gmpi_allocate_world (unsigned int size)
{
  gmpi.global_node_ids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.local_node_ids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.mpi_pids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.port_ids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.board_ids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.pending_sends = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.dropped_sends = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.host_names = (char **) calloc (size, sizeof (char *));
  gmpi.exec_names = (char **) calloc (size, sizeof (char *));
  gmpi.numa_nodes = (unsigned int *) calloc (size, sizeof (unsigned int));

  gmpi_malloc_assert (gmpi.global_node_ids, "gmpi_getconf", 
		      "malloc: global_node_ids");
  gmpi_malloc_assert (gmpi.local_node_ids, "gmpi_getconf", 
		      "malloc: local_node_ids");
  gmpi_malloc_assert (gmpi.mpi_pids, "gmpi_getconf", "malloc: mpi_pids");
  gmpi_malloc_assert (gmpi.port_ids, "gmpi_getconf", "malloc: port_ids");
  gmpi_malloc_assert (gmpi.board_ids, "gmpi_getconf", "malloc: board_ids");
  gmpi_malloc_assert (gmpi.pending_sends, "gmpi_getconf", 
		      "malloc: pending_sends");
  gmpi_malloc_assert (gmpi.dropped_sends, "gmpi_getconf", 
		      "malloc: dropped_sends");
  gmpi_malloc_assert (gmpi.host_names, "gmpi_getconf", "malloc: host_names");
  gmpi_malloc_assert (gmpi.exec_names, "gmpi_getconf", "malloc: exec_names");
  gmpi_malloc_assert (gmpi.numa_nodes, "gmpi_getconf", "malloc: numa_nodes");
}


#define GMPI_SOCKET_BUFFER_SIZE 128*1024

static void
gmpi_getconf (void)
{
  char *gmpi_eager, *gmpi_shmem, *gmpi_numa_node, *gmpi_recvmode;
  unsigned int i, j, port_id;
  int board_id, k;
  
  setbuf (stdout, NULL);
  setbuf (stderr, NULL);

  gmpi.debug_output_filedesc = stderr;

  if (getenv ("MAN_MPD_FD") != NULL) {
    char attr_buffer[BNR_MAXATTRLEN];
    char val_buffer[BNR_MAXVALLEN];
    /* MPD to spawn processes */
    char my_hostname[256];
    char *hostnames;
    BNR_Group bnr_group;
    
    /* MPD to spawn processes */
    gmpi.mpd = 1;
    i = BNR_Init ();
    i = BNR_Get_group (&bnr_group);
    i = BNR_Get_rank (bnr_group, &MPID_GM_rank);
    i = BNR_Get_size (bnr_group, &MPID_GM_size);
    
    /* data allocation */
    gmpi_allocate_world (MPID_GM_size);
    hostnames = (char *) malloc (MPID_GM_size * 256 * sizeof (char *));
    gmpi_malloc_assert (hostnames, "gmpi_getconf", "malloc: hostnames");
    
    /* open a port */
    board_id = -1;
    port_id = 0;
    gmpi_allocate_port (&board_id, &port_id);
    if (port_id == 0) {
      fprintf (stderr, "[%d] Error: Unable to open a GM port !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* get the GM node id */
    if (gm_get_node_id (gmpi_gm_port, &i) != GM_SUCCESS) {
      fprintf (stderr, "[%d] Error: Unable to get GM local node id !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }

#if GMPI_GM2
    /* convert the GM local node id to the global one. What a mess. */
    if (gm_node_id_to_global_id (gmpi_gm_port, i, &(gmpi.my_global_node_id)) 
	!= GM_SUCCESS) {
      fprintf (stderr, "[%d] Error: Unable to get GM global node id !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
#else
    gmpi.my_global_node_id = i;
#endif
  
    /* Check to see if there is a numa node number for this process */
    gmpi_getenv ("GMPI_NUMA_NODE", &gmpi_numa_node, NULL, 0);

    /* Set the local node number, defaulting to 0 */
    if ((gmpi_numa_node == NULL) 
	|| (sscanf (gmpi_numa_node, "%u",
		    &(gmpi.numa_nodes[MPID_GM_rank])) != 1)) {
      gmpi.numa_nodes[MPID_GM_rank] = 0;
    }
  
    /* build the data to send to master */
    gm_bzero (val_buffer, BNR_MAXVALLEN * sizeof (char));
    gm_get_host_name (gmpi_gm_port, my_hostname);
    snprintf(val_buffer, BNR_MAXVALLEN, "< %u:%u:%u:%u:%s >\n", 
	     port_id, board_id, gmpi.my_global_node_id, 
	     gmpi.numa_nodes[MPID_GM_rank], my_hostname);
    
    /* put our information */
    gm_bzero (attr_buffer, BNR_MAXATTRLEN * sizeof (char));
    snprintf (attr_buffer, BNR_MAXATTRLEN, "MPICH-GM data [%u]\n", 
	      MPID_GM_rank);
    i = BNR_Put (bnr_group, attr_buffer, val_buffer, -1);
    
    /* get other processes data */
    i = BNR_Fence (bnr_group);
    for (j = 0; j < MPID_GM_size; j++) {
      gm_bzero (attr_buffer, BNR_MAXATTRLEN * sizeof (char));
      snprintf (attr_buffer, BNR_MAXATTRLEN, "MPICH-GM data [%u]\n", j);
      i = BNR_Get (bnr_group, attr_buffer, val_buffer);
      
      /* decrypt data */
      if (sscanf (val_buffer, "< %u:%u:%u:%u:%s >", &(gmpi.port_ids[j]), 
		  &(gmpi.board_ids[j]), &(gmpi.global_node_ids[j]), 
		  &(gmpi.numa_nodes[j]), &(hostnames[j*256])) != 5) {
	fprintf (stderr, "[%u] Error: unable to decode data "
		 "from %u !\n", MPID_GM_rank, j);
	gmpi_abort (0);
      }
    }
    
    /* compute the local mapping */
    smpi.num_local_nodes = 0;
    for (j = 0; j < MPID_GM_size; j++) {
      /* Only "local" if same host AND same numa `node' */
      if ((strncmp (my_hostname, &(hostnames[j*256]), 256) == 0) &&
	  (gmpi.numa_nodes[j] == gmpi.numa_nodes[MPID_GM_rank])) {
	if (j == MPID_GM_rank) {
	  smpi.my_local_id = smpi.num_local_nodes;
	}
	smpi.local_nodes[j] = smpi.num_local_nodes;
	smpi.num_local_nodes++;
      } else {
	smpi.local_nodes[j] = -1;
      }
    }
    free (hostnames);
  } else {
    char *gmpi_magic, *gmpi_master, *gmpi_port, *gmpi_slave;
    char *gmpi_id, *gmpi_np, *gmpi_board;
    char buffer[GMPI_SOCKET_BUFFER_SIZE];
    char temp[64];
    gm_u64_t start_time, stop_time;
    gm_u32_t count, magic_number;
    gm_u16_t master_port, slave_port;
    int gmpi_sockfd, gmpi_sockfd2;
    struct hostent *master;
    struct hostent *slave;
    
    /* mpirun with sockets */
    gmpi.mpd = 0;
    gethostname(temp, sizeof (temp)-1);
    gmpi_getenv ("GMPI_MAGIC", &gmpi_magic, "the job magic number", 1);
    gmpi_getenv ("GMPI_MASTER", &gmpi_master, "the master's hostname", 1);
    gmpi_getenv ("GMPI_PORT", &gmpi_port, "the master's port number", 1);
    gmpi_getenv ("GMPI_SLAVE", &gmpi_slave, "the slave's hostname", 1);
    gmpi_getenv ("GMPI_ID", &gmpi_id, "the MPI ID of the process", 1);
    gmpi_getenv ("GMPI_NP", &gmpi_np, "the number of MPI processes", 1);
    gmpi_getenv ("GMPI_BOARD", &gmpi_board, "the specified board", 1);
    gmpi_getenv ("GMPI_NUMA_NODE", &gmpi_numa_node, NULL, 0);

    if (sscanf (gmpi_magic, "%u", &magic_number) != 1) {
      fprintf (stderr, "<MPICH-GM> Error on %s: Bad magic number "
	       "(GMPI_MAGIC is %s) !\n", temp, gmpi_magic);
      gmpi_abort (0);
    }
    gmpi.magic = magic_number;
    
    if ((sscanf (gmpi_np, "%u", &MPID_GM_size) != 1)
	|| (MPID_GM_size < 0)) {
      fprintf (stderr, "<MPICH-GM> Error on %s: Bad number of processes "
	       "(GMPI_NP is %s) !\n", temp, gmpi_np);
      gmpi_abort (0);
    }
    
    if ((sscanf (gmpi_id, "%u", &MPID_GM_rank) != 1)
	|| (MPID_GM_rank < 0) || (MPID_GM_rank >= MPID_GM_size)) {
      fprintf (stderr, "<MPICH-GM> Error on %s: Bad MPI ID "
	       "(GMPI_ID is %s, total number of MPI processes is %u) !\n", 
	       temp, gmpi_np, MPID_GM_size);
      gmpi_abort (0);
    }
  
    if (sscanf (gmpi_port, "%hd", &master_port) != 1) {
      fprintf (stderr, "<MPICH-GM> Error on %s: Bad master port number "
	       "(GMPI_PORT is %s) !\n", temp, gmpi_port);
      gmpi_abort (0);
    }
    
    if (sscanf (gmpi_board, "%u", &board_id) != 1) {
      fprintf (stderr, "<MPICH-GM> Error on %s: Bad board ID "
	       "(GMPI_BOARD is %s) !\n", temp, gmpi_board);
      gmpi_abort (0);
    }
    
    gmpi_getenv ("GMPI_EAGER", &gmpi_eager, NULL, 0);
    gmpi_getenv ("GMPI_SHMEM", &gmpi_shmem, NULL, 0);
    gmpi_getenv ("GMPI_RECV", &gmpi_recvmode, NULL, 0);
    
    /* Set the EAGER/Rendez-vous protocols threshold */
    if (gmpi_eager == NULL) {
      gmpi.eager_size = GMPI_EAGER_SIZE_DEFAULT;
    } else {
      gmpi.eager_size = strtoul(gmpi_eager, NULL, 10);
      if (gmpi.eager_size < 128) {
	gmpi.eager_size = 128;
      }
      
      if (gmpi.eager_size > GMPI_MAX_PUT_LENGTH) {
	gmpi.eager_size = GMPI_MAX_PUT_LENGTH;
      }
    }
    
    /* set the GM receive mode */
    if (gmpi_recvmode == NULL) {
      gmpi.gm_receive_mode = gm_receive;
    } else {
      if (strcasecmp (gmpi_recvmode, "polling") == 0) {
	gmpi.gm_receive_mode = gm_receive;
      } else {
	if (strcasecmp (gmpi_recvmode, "blocking") == 0) {
	  gmpi.gm_receive_mode = gm_blocking_receive_no_spin;
	} else {
	  if (strcasecmp (gmpi_recvmode, "hybrid") == 0) {
	    gmpi.gm_receive_mode = gm_blocking_receive;
	  } else {
	    gmpi.gm_receive_mode = gm_receive;
	  }
	}
      }
    }
    
    /* Set the shared memory support */
    if (gmpi.gm_receive_mode == gm_receive) {
      if (gmpi_shmem == NULL) {
	gmpi.shmem = 1;
      } else {
	if (strcmp (gmpi_shmem, "2") == 0) {
	  gmpi.shmem = 2;
	} else {
	  if (strcmp (gmpi_shmem, "1") == 0) {
	    gmpi.shmem = 1;
	  } else {
	    gmpi.shmem = 0;
	  }
	}
      }
    } else {
      gmpi.shmem = 0;
    }
    
    /* data allocation */
    gmpi_allocate_world (MPID_GM_size);
    
    /* Set the local node number */
    if ((gmpi_numa_node == NULL) 
	|| (sscanf (gmpi_numa_node, "%u",
		    &(gmpi.numa_nodes[MPID_GM_rank])) != 1)) {
      gmpi.numa_nodes[MPID_GM_rank] = 0;
    }

    /* open a port */
    port_id = 0;
    gmpi_allocate_port (&board_id, &port_id);
    if (port_id == 0) {
      fprintf (stderr, "[%u] Error: Unable to open a GM port !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* get the GM node id */
    if (gm_get_node_id (gmpi_gm_port, &i) != GM_SUCCESS) {
      fprintf (stderr, "[%u] Error: Unable to get GM local node id !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
#if GMPI_GM2
    /* convert the GM local node id to the global one. What a mess. */
    if (gm_node_id_to_global_id (gmpi_gm_port, i, &(gmpi.my_global_node_id)) 
	!= GM_SUCCESS) {
      fprintf (stderr, "[%u] Error: Unable to get GM global node id !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
#else
    gmpi.my_global_node_id = i;
#endif
    
    /* get a socket */
    gmpi_sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (gmpi_sockfd < 0) {
      fprintf (stderr, "[%u] Error: Unable to open a socket !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* get the slave's IP address */
    slave = gethostbyname (gmpi_slave);
    if (slave == NULL) { 
      fprintf (stderr, "[%u] Error: Unable to translate "
	       "the hostname of the slave (%s)!\n", 
	       MPID_GM_rank, gmpi_slave);
      gmpi_abort (0);
    }
    
    /* bind the socket to a port */
    gm_bzero ((char *) (&(gmpi.slave_addr)), sizeof (gmpi.slave_addr));
    gmpi.slave_addr.sin_family = AF_INET;
    gm_bcopy ((char *) (slave->h_addr), 
	      (char *) (&(gmpi.slave_addr.sin_addr)),
	      slave->h_length);
    
    for (slave_port = 8000; slave_port < 20000; slave_port++) {
      gmpi.slave_addr.sin_port = gm_hton_u16 (slave_port);
      if (bind(gmpi_sockfd, (struct sockaddr *) &(gmpi.slave_addr), 
	       sizeof (gmpi.slave_addr)) == 0) {
	break;
      }
    }
    if (slave_port >= 20000) {
      fprintf (stderr, "[%u] Error: Unable to bind to a socket on (%s) "
	       "between the port 8000 and 20000 (%s)!\n", 
	       MPID_GM_rank, gmpi_slave);
      gmpi_abort (0);
    }
    
    /* listen for the master */
    if (listen(gmpi_sockfd, 1) != 0) {
      fprintf (stderr, "[%u] Error: Error when listening on the socket !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* get another socket */
    gmpi_sockfd2 = socket (AF_INET, SOCK_STREAM, 0);
    if (gmpi_sockfd2 < 0) {
      fprintf (stderr, "[%u] Error: Unable to open a socket (2) !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* get the master's IP address */
    master = gethostbyname (gmpi_master);
    if (master == NULL) { 
      fprintf (stderr, "[%u] Error: Unable to translate "
	       "the hostname of the master (%s)!\n", 
	       MPID_GM_rank, gmpi_master);
      gmpi_abort (0);
    }
    
    /* connect to the master */
    gm_bzero ((char *) (&(gmpi.master_addr)), sizeof (gmpi.master_addr));
    gmpi.master_addr.sin_family = AF_INET;
    gm_bcopy ((char *) (master->h_addr), 
	      (char *) (&(gmpi.master_addr.sin_addr)),
	      master->h_length);
    gmpi.master_addr.sin_port = gm_hton_u16 (master_port);
    start_time = gm_ticks(gmpi_gm_port);
    while (connect (gmpi_sockfd2, (struct sockaddr *) (&(gmpi.master_addr)), 
		    sizeof (gmpi.master_addr)) < 0) {
      stop_time = gm_ticks(gmpi_gm_port);
      if ((stop_time - start_time) > (2000 * GMPI_INIT_TIMEOUT)) {
	fprintf (stderr, "[%u] Error: Unable to connect to "
		 "the master !\n", MPID_GM_rank);
	gmpi_abort (0);
      }
    }
    
    /*
     * Send the message to the master:
     * <<<magic:ID:port:board:node:numanode:pid::socket_port>>>
     */
    count = 0;
    snprintf(buffer, GMPI_SOCKET_BUFFER_SIZE, 
	     "<<<%u:%u:%u:%u:%u:%u:%u::%u>>>\n", magic_number, 
	     MPID_GM_rank, port_id, board_id, gmpi.my_global_node_id, 
	     gmpi.numa_nodes[MPID_GM_rank], (int) getpid (), slave_port);
    while (count < strlen (buffer)) {
      k = write (gmpi_sockfd2, &(buffer[count]), strlen (buffer) - count);
      if (k < 0) {
	fprintf (stderr, "[%u] Error: write to socket failed !\n", 
		 MPID_GM_rank);
	gmpi_abort (0);
      }
      count += k;
    }
    close (gmpi_sockfd2);
    
    /* wait for the map from the master */
    gmpi_sockfd2 = accept (gmpi_sockfd, 0, 0);
    if (gmpi_sockfd2 < 0) {
      fprintf (stderr, "[%u] Error: Error on socket accept !\n", 
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* Get the whole GM mapping from the master */
    count = 0;
    gm_bzero (buffer, GMPI_SOCKET_BUFFER_SIZE * sizeof(char));
    while (strstr (buffer, "]]]") == NULL) {
      k = read (gmpi_sockfd2, &(buffer[count]), 
		GMPI_SOCKET_BUFFER_SIZE - count);
      if (k < 0) {
	fprintf (stderr, "[%u] Error: read from socket failed !\n", 
		 MPID_GM_rank);
	gmpi_abort (0);
      }
      count += k;
    }
    close (gmpi_sockfd2);
    
    /* check the initial marker */
    j = 0;
    if (strncmp (&(buffer[j]), "[[[", 3) != 0) {
      fprintf (stderr, "[%u] Error: bad format on data from master !\n",
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* Decrypt the global mapping */
    j += 3;
    for (i = 0; i < MPID_GM_size; i++) {
      if (sscanf(&(buffer[j]), "<%u:%u:%u:%u>", &(gmpi.port_ids[i]), 
		 &(gmpi.board_ids[i]), &(gmpi.global_node_ids[i]), 
		 &(gmpi.numa_nodes[i])) != 4) {
	fprintf(stderr, "[%u] Error: unable to decode data "
		"from master !\n", MPID_GM_rank);
	gmpi_abort(0);
      }
      
      snprintf(temp, sizeof (temp), "<%u:%u:%u:%u>", gmpi.port_ids[i], 
	       gmpi.board_ids[i], gmpi.global_node_ids[i], gmpi.numa_nodes[i]);
      j += strlen (temp);
      
      smpi.local_nodes[i] = -1;
    }

    /* check the marker between global map and local map */  
    if (strncmp (&(buffer[j]), "|||", 3) != 0) {
      fprintf (stderr, "[%u] Error: bad format on data from master !\n",
	       MPID_GM_rank);
      gmpi_abort (0);
    }
    
    /* decrypt the local mapping */
    j += 3;
    smpi.num_local_nodes = 0;
    while (strncmp (&(buffer[j]), "]]]", 3) != 0) { 
      if (sscanf (&(buffer[j]), "<%u>", &i) != 1) {
	fprintf (stderr, "[%u] Error: unable to decode master data !\n",
		 MPID_GM_rank);
	gmpi_abort (0);
      }
      
      if (i == MPID_GM_rank) {
	smpi.my_local_id = smpi.num_local_nodes;
      }
      
      if ((gmpi.shmem != 2) 
	  || ((gmpi.shmem == 2) 
	      && (gmpi.board_ids[i] == gmpi.board_ids[MPID_GM_rank]))) {
	smpi.local_nodes[i] = smpi.num_local_nodes;
	smpi.num_local_nodes++;
      }
      
      snprintf(temp, sizeof(temp), "<%u>", i);
      j += strlen (temp);
    }
    
    /* check size of the data from the master */
    j += 3;
    if (j != count) {
      fprintf (stderr, "[%u] Error: amount of data from master !\n",
	       MPID_GM_rank);
      gmpi_abort (0);
    }
  }
  
  /* get local hostname */
  gm_get_host_name(gmpi_gm_port, gmpi.my_hostname);

#if GMPI_GM2
  /* convert the GM global node ids to local ones. What a big mess ! */
  for (i = 0; i < MPID_GM_size; i++) {
    if (gm_global_id_to_node_id (gmpi_gm_port, gmpi.global_node_ids[i], 
				 &(gmpi.local_node_ids[i])) != GM_SUCCESS) {
      fprintf (stderr, "[%u] Error: Unable to translate GM global node id (%u)"
	       "to local node id for the MPI id %u !\n", MPID_GM_rank, 
	       gmpi.global_node_ids[i], i);
      gmpi_abort (0);
    }
  }
#else
  /* There is no difference between the local node ids and the global ones. */
  for (i = 0; i < MPID_GM_size; i++) {
    gmpi.local_node_ids[i] = gmpi.global_node_ids[i];
  }
#endif

  /* check consistency */
  if ((gmpi.port_ids[MPID_GM_rank] != port_id) 
      || (gmpi.board_ids[MPID_GM_rank] != board_id)
      || (gmpi.global_node_ids[MPID_GM_rank] != gmpi.my_global_node_id)) {
    fprintf (stderr, "[%u] Error: inconsistency in collected data !\n", 
	     MPID_GM_rank);
    gmpi_abort (0);
  }
}


/* This function fill the MPID_Config structure to describe 
   the multi-devices configuration */
MPID_Config *MPID_GM_GetConfigInfo (int *argc, char ***argv ) 

{
  MPID_Config * new_config = NULL;
  MPID_Config * return_config = NULL;
  int i, j;

  /* Get the GM mapping and the environnement variables */
  gmpi_getconf();

  /* at least one local node : myself ! */ 
  gmpi_debug_assert(smpi.num_local_nodes != 0);


  /* SELF DEVICE */
  /* if it's the first device, start the linked list of devices.
     Otherwise, add a new one at the end. */
  if (new_config == NULL)
    {
      new_config = (MPID_Config *)malloc(sizeof(MPID_Config));
      gmpi_malloc_assert(new_config,
			 "MPID_GM_GetConfigInfo",
			 "malloc: Self device config");
      return_config = new_config;
    }
  else
    {
      new_config->next = (MPID_Config *)malloc(sizeof(MPID_Config));
      gmpi_malloc_assert(new_config->next,
			 "MPID_GM_GetConfigInfo",
			 "malloc: Self device config");
      new_config = new_config->next;
    }

  /* we don't need to check if we need this device: we need this device ! */
  new_config->device_init = MPID_CH_InitSelfMsg;
  new_config->device_init_name = (char *)malloc(255*sizeof(char));
  gmpi_malloc_assert(new_config->device_init_name,
		     "MPID_GM_GetConfigInfo",
		     "malloc: Self device name");
  sprintf(new_config->device_init_name, "Self device");
  new_config->num_served = 1;
  new_config->granks_served = (int *)malloc(sizeof(int));
  gmpi_malloc_assert(new_config->granks_served,
		     "MPID_GM_GetConfigInfo",
		     "malloc: Self device map");
  new_config->granks_served[0] = MPID_GM_rank;
  new_config->next = NULL;
  
  
  /* SMP DEVICE */
  /* we don't need this device if there's only one process on this node */
  if (smpi.num_local_nodes > 1) 
    {
#if !GM_OS_VXWORKS
      if (gmpi.shmem > 0)
	{
	  if (gmpi.gm_receive_mode == gm_receive) 
	    {
	      if (new_config == NULL) 
		{
		  new_config = (MPID_Config *)malloc(sizeof(MPID_Config));
		  gmpi_malloc_assert(new_config,
				     "MPID_GM_GetConfigInfo",
				     "malloc: SMP device config");
		  return_config = new_config;
		}
	      else 
		{
		  new_config->next = (MPID_Config *)
		    malloc(sizeof(MPID_Config));
		  gmpi_malloc_assert(new_config->next,
				     "MPID_GM_GetConfigInfo",
				     "malloc: SMP device config");
		  new_config = new_config->next;
		}
	      new_config->device_init = MPID_SMP_InitMsgPass;
	      new_config->device_init_name = (char *)
		malloc(255 * sizeof(char));
	      gmpi_malloc_assert(new_config->device_init_name,
				 "MPID_GM_GetConfigInfo",
				 "malloc: SMP device name");
	      sprintf(new_config->device_init_name, "SMP_plug device");
	      new_config->num_served = smpi.num_local_nodes - 1;
	      new_config->granks_served = (int *)
		malloc(new_config->num_served * sizeof(int));
	      gmpi_malloc_assert(new_config->granks_served,
				 "MPID_GM_GetConfigInfo",
				 "malloc: SMP device map");
	      /* setup routes */
	      j = 0;
	      for(i=0; i<MPID_GM_size; i++)
		{
		  if ((i!= MPID_GM_rank) 
		      && (smpi.local_nodes[i] != -1)) 
		    {
		      gmpi_debug_assert(smpi.local_nodes[i] 
					!= smpi.my_local_id);
		      new_config->granks_served[j] = i;
		      j++;
		    }
		}
	      gmpi_debug_assert(j == new_config->num_served);
	      new_config->next = NULL;
	    }
	}
      else
#endif
	{
	  smpi.num_local_nodes = 1;
	  for (i=0; i < MPID_GM_size; i++)
	    {
	      if (i != MPID_GM_rank)
		{
		  smpi.local_nodes[i] = -1;
		}
	    }
	}
    }
  

  /* GM DEVICE */
  if (MPID_GM_size > smpi.num_local_nodes)
    {
      if (new_config == NULL)
	{
	  new_config = (MPID_Config *)malloc(sizeof(MPID_Config));
	  gmpi_malloc_assert(new_config,
			     "MPID_GM_GetConfigInfo",
			     "malloc: GM device config");
	  return_config = new_config;
	}
      else
	{
	  new_config->next = (MPID_Config *)malloc(sizeof(MPID_Config));
	  gmpi_malloc_assert(new_config->next,
			     "MPID_GM_GetConfigInfo",
			     "malloc: GM device config");
	  new_config = new_config->next;
	}
      new_config->device_init = MPID_CH_GM_InitMsgPass;
      new_config->device_init_name = (char *)malloc(255 * sizeof(char));
      gmpi_malloc_assert(new_config->device_init_name,
			 "MPID_GM_GetConfigInfo",
			 "malloc: GM device name");
      sprintf(new_config->device_init_name, "Myricom GM device");
      new_config->num_served = MPID_GM_size - smpi.num_local_nodes;
      new_config->granks_served = (int *)malloc(new_config->num_served 
						* sizeof(int));
      gmpi_malloc_assert(new_config->granks_served,
			 "MPID_GM_GetConfigInfo",
			 "malloc: GM device map");
      /* setup the routes */
      j = 0;
      for(i=0; i<MPID_GM_size; i++)
	if (smpi.local_nodes[i] == -1) {
	  new_config->granks_served[j] = i;
	  j++;
	}
      gmpi_debug_assert(j == new_config->num_served);
      new_config->next = NULL;
    }
  
  return return_config;
}

