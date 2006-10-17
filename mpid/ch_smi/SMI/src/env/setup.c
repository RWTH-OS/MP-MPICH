/* $Id$ */

#include "smidebug.h"

#include "setup.h"
#include "general_definitions.h"
#include "startup/startup.h"

#ifdef WIN32
#include <process.h>
#endif
#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* Set the policy how to use the adapters to import and export memory segments 
   and transfer data. This function must only be called if no user-allocated
   shared memory regions do exist. */
smi_error_t SMI_Set_adapter (int adapter)
{
    int lrank, nbr_exp_adpt, nbr_imp_adpt, nbr_adpt;
    int retval = SMI_SUCCESS;
    DSECTION ("SMI_Set_adapter");

    nbr_adpt =  _smi_nbr_adapters[_smi_my_proc_rank];

    ASSERT_R(_smi_initialized, "SMI library not initialized", SMI_ERR_NOINIT);
    ASSERT_R(_smi_mis.nbr_user_regions == 0, "Existing user regions", SMI_ERR_BUSY);
    ASSERT_R(adapter == SMI_ADPT_DEFAULT || adapter == SMI_ADPT_IMPEXP || adapter == SMI_ADPT_SMP
	     || adapter == SMI_ADPT_CYCLIC || adapter == SMI_ADPT_BIDIRECTIONAL
	     || (adapter >= 0 && adapter < nbr_adpt),
	     "Illegal adapter (policy) specification", SMI_ERR_PARAM);

    _smi_adpt_policy = adapter;

    SMI_Local_proc_rank (&lrank);

    switch (_smi_adpt_policy) {
    case SMI_ADPT_DEFAULT:
	DNOTICE ("assigning DEFAULT adapter");
	_smi_adpt_export = _smi_DefAdapterNbr;
	_smi_adpt_import = _smi_DefAdapterNbr;
	break;
    case SMI_ADPT_IMPEXP:
	DNOTICE ("setting IMPORT/EXPORT adapter configuration");
	/* Adapters are assigned to export and import; the export- and import-adapters are then
	   assigned to the active processes in a round-robin fashion. For an uneven number, the 
	   remaining adapter will be assigned for export because outgoing transfer is generally less
	   PCI-friendly then incoming data. */
	if (nbr_adpt == 1) {
	    _smi_adpt_export = _smi_DefAdapterNbr;
	    _smi_adpt_import = _smi_DefAdapterNbr;
	} else {
	    nbr_exp_adpt = nbr_adpt / 2 + nbr_adpt % 2;
	    nbr_imp_adpt = nbr_adpt / 2;
	    _smi_adpt_export = lrank % nbr_exp_adpt;
	    _smi_adpt_import = lrank % nbr_exp_adpt + nbr_exp_adpt;
	}
	break;
    case SMI_ADPT_SMP:
	DNOTICE ("setting SMP adapter configuration");
	/* Distribute the available adapters onto the active processes. */
	_smi_adpt_export = (lrank == 0) ? 0 : lrank % nbr_adpt;
	_smi_adpt_import = _smi_adpt_export;
	break;
    case SMI_ADPT_CYCLIC:
    case SMI_ADPT_BIDIRECTIONAL:
	DWARNING ("CYCLIC and BIDIRECTIONAL not implemented - using default.");
	_smi_adpt_export = _smi_DefAdapterNbr;
	_smi_adpt_import = _smi_DefAdapterNbr;
	retval = SMI_ERR_NOTIMPL;
    default:
	_smi_adpt_export = adapter;
	_smi_adpt_import = adapter;
	break;
    }

    DNOTICEI ("Available adapters:", nbr_adpt);
    DNOTICEI ("Adapter for export:", _smi_adpt_export);
    DNOTICEI ("Adapter for import:", _smi_adpt_import);

    return retval;
}


/*****************************************************************************/
/*** Returns an identifier of the machine on which the calling process     ***/
/*** resides. Different processors within one SMP make no difference. The  ***/
/*** allocation of storage for the returned name is up to the caller. 256  ***/
/*** bytes are required.                                                   ***/
/*** the only error that might occure is an 2xxx error                     ***/
/*****************************************************************************/
static smi_error_t _smi_get_machine_id(char* m_id)
{
  DSECTION("_smi_get_machine_id");
  int error;
  
#ifdef WIN32
  DWORD i;
  DWORD dummy = 256;
  
  DSECTENTRYPOINT;
  
  if (GetComputerName(m_id, &dummy) == TRUE) {
      error = 0;
      for(i=0;i<dummy;i++)
	  m_id[i] = tolower(m_id[i]);
  } else
    error = -1;
#else
  DSECTENTRYPOINT;
  
  error = gethostname(m_id, 256);
#endif
  
  ASSERT_R(error != -1, "Could not get local hostname", errno+2000);
  DSECTLEAVE; return(SMI_SUCCESS);
}




  

/*****************************************************************************/
/*** Determines the total number of processes and stores it in the global  ***/
/*** variable 'no_processes'.                                              ***/
/*** The only error that may occure is an 1xxx error.                      ***/
/*****************************************************************************/
smi_error_t _smi_get_no_processes(void)
{
   DSECTION("_smi_get_no_processes");
   smi_error_t mpi_error;

   DSECTENTRYPOINT;

   mpi_error = _smi_ll_commsize(&_smi_nbr_procs);

   ASSERT_R(mpi_error == MPI_SUCCESS, "Could not get nr of processes", 1000+mpi_error);
   DSECTLEAVE; return(SMI_SUCCESS);
}


/*****************************************************************************/
/*** Determines the rank of the calling processes within the               ***/
/*** MPI-communicator MPI_COMM_WORLD and stores it in the global variable  ***/
/*** 'mpi_rank'.                                                           ***/
/*** The only error that may occure in an 1xxx error.                      ***/
/*****************************************************************************/
smi_error_t _smi_get_loc_mpi_rank(void)
{
   DSECTION("_smi_get_loc_mpi_rank");
   smi_error_t mpi_error;

   DSECTENTRYPOINT;

   mpi_error = _smi_ll_commrank(&_smi_mpi_rank);

   ASSERT_R(mpi_error == MPI_SUCCESS, "Could not get local proccess rank", 1000+mpi_error);   
   DSECTLEAVE; return(SMI_SUCCESS);
}




/*****************************************************************************/
/*** Determines the size of a single page, as handled by the MMU. If the   ***/
/*** used machines possess different page sizes, the smallest common       ***/
/*** multiple is used. This function requires, that 'no_processes' and     ***/
/*** 'my__smi_local_rank' are already set.                                      ***/
/*** Only 1xxx and 2xxx errors can occure.                                 ***/
/*****************************************************************************/
smi_error_t _smi_get_page_size(void)
{
   DSECTION("_smi_get_page_size");
   int* all_page_sizes;      /* system page size of the machines             */
   int  scm;                 /* smalles common multiple of all page sizes    */
   int  i;                   /* loop counter                                 */
   int  local_page_size;     /* page-size of the machine on which the        */
                             /* calling process resides                      */
   smi_error_t mpi_error;
   
   DSECTENTRYPOINT;
   
#ifdef WIN32
   _smi_page_size = 65536;
   DSECTLEAVE; return(SMI_SUCCESS);
#endif

   /* gather page sizes of all machines */
   ALLOCATE (all_page_sizes, int *, _smi_nbr_procs*sizeof(int));
   
   local_page_size = (int)sysconf(_SC_PAGESIZE);
   ASSERT_R((local_page_size != -1),"Could not get local pagesize",2000+errno);
   
   mpi_error = _smi_ll_allgather(&local_page_size, 1, all_page_sizes, _smi_mpi_rank);
   ASSERT_R((mpi_error == MPI_SUCCESS),"MPIAllgather failed",1000+mpi_error);

   /* determine smallest common multiple */
   scm = all_page_sizes[0];
   for(i=0;i<_smi_nbr_procs;i++) {
      if (all_page_sizes[i] > scm) {
	 while (all_page_sizes[i] > scm || scm%all_page_sizes[i] != 0)
	    scm += scm;
      }
      if (all_page_sizes[i] < scm) {
	  while (scm%all_page_sizes[i] != 0)
	    all_page_sizes[i] += all_page_sizes[i];
	 scm = all_page_sizes[i];
      }
   }

   _smi_page_size = scm;

   free(all_page_sizes);

   DSECTLEAVE; return(SMI_SUCCESS);
 }


/* just to get rid of the compiler warning for the qsort() call */
static int _smi_strcmp (const void *a, const void *b) 
{
    return strcmp( (char *)a, (char *)b );
}

static int _smi_intcmp (const void *a, const void *b) 
{
    int ia, ib, ret;

    ia = *(int *)a;
    ib = *(int *)b;

    ret = 0;
    if (ia < ib)
	ret = -1;
    else 
	if (ia > ib)
	    ret = 1;

    return ret;
}

/********************************************************************************/
/*** Determines the process rank of the calling process and the machine       ***/
/*** ranks of all processes. Processes, which are executed on the same        ***/
/*** machine shall have consecutive ranks. This function requires, that       ***/
/*** '_smi_nbr_procs' is already set.                                         ***/
/*** Only 1xxx and 2xxx errors can occure.                                    ***/
/********************************************************************************/
smi_error_t _smi_set_ranks(void)
{
   DSECTION("_smi_set_ranks");
   int     i, j;  
#if RANKS_BY_HOSTNAME
   smi_error_t error;
   /* the ranks are determined by the hostnames of the nodes */
   char**  machine_id;         /* the i-th entry contains an identifier of the  */
                               /* machine on which process i resides            */
   char    loc_machine_id[256];/* identifier of the local machine               */
   DSECTENTRYPOINT;

   /* determine machine identifiers for all processes */
   ALLOCATE (machine_id, char**, _smi_nbr_procs*sizeof(char *));
   ALLOCATE (machine_id[0], char*, _smi_nbr_procs*sizeof(char)*256);
   
   for (i = 1; i < _smi_nbr_procs; i++)
      machine_id[i] = &(machine_id[0][i*256]);

   error = _smi_get_machine_id(loc_machine_id);
   ASSERT_R((error==SMI_SUCCESS),"Could not get machine id",error);

   _smi_ll_allgather((int*)loc_machine_id, 256/sizeof(int), (int*)&(machine_id[0][0]), _smi_mpi_rank);
    
   /* determine process rank: add the number of processes which have */
   /* smaller machine identifiers and the number of processes with   */
   /* the same machine identifier, which have a smaller rank inside  MPI  */
   _smi_my_proc_rank = 0;
   for (i=0;i<_smi_nbr_procs;i++) {
      if (  (i<_smi_mpi_rank && strcmp(machine_id[i],machine_id[_smi_mpi_rank])<=0)
	  ||(i>_smi_mpi_rank && strcmp(machine_id[i],machine_id[_smi_mpi_rank])<0)
	  )
	 _smi_my_proc_rank++;
    }

   /* determine machine ranks of all processes: sort machine identifiers, */
   /* count the numer of different machines with smaller machine          */
   /* identifiers; this works because process ranks are monoton           */
   /* in machine identifiers                                              */
   ALLOCATE (_smi_machine_rank,int *,_smi_nbr_procs * sizeof(int));
   qsort(machine_id[0], _smi_nbr_procs, 256, &_smi_strcmp);
   for (i = 0; i < _smi_nbr_procs; i++) {
       _smi_machine_rank[i] = 0;
       for (j = 0; j < i; j++)
	 if (strcmp(machine_id[j], machine_id[j+1]) != 0)
	   _smi_machine_rank[i]++;
   }
   _smi_my_machine_rank = _smi_machine_rank[_smi_my_proc_rank];
   
   free(machine_id[0]);
   free(machine_id);
#else
   /* the ranks are determined by the SCI-ID of the nodes */
   int loc_sci_id, *rmt_sci_ids;
   int def_adpt;

   DSECTENTRYPOINT;
   ALLOCATE (_smi_machine_rank, int *,_smi_nbr_procs * sizeof(int));

   if (_smi_all_on_one) {
       /* all proceses on one node - easy case */
       _smi_my_proc_rank = _smi_mpi_rank;
       for (i = 0; i < _smi_nbr_procs; i++)
	   _smi_machine_rank[i] = 0;
   } else {
       /* processes on distinct node - assign ranks according to SCI ID of nodes */
       ALLOCATE (rmt_sci_ids, int *,_smi_nbr_procs * sizeof(int));

       SMI_Query (SMI_Q_SCI_DEFADAPTER, 0, &def_adpt);
       SMI_Query (SMI_Q_SCI_ID, def_adpt, &loc_sci_id);
       _smi_ll_allgather(&loc_sci_id, 1, rmt_sci_ids, _smi_mpi_rank);
       /* determine the application rank of this process */
       _smi_my_proc_rank = 0;
       for (i = 0; i < _smi_nbr_procs; i++) {
	   if ((i < _smi_mpi_rank && loc_sci_id >= rmt_sci_ids[i])
	       ||(i > _smi_mpi_rank && loc_sci_id > rmt_sci_ids[i]))
	       _smi_my_proc_rank++;
       }
       
       /* determine the machine rank of each process */
       qsort(rmt_sci_ids, _smi_nbr_procs, sizeof(int), &_smi_intcmp);
       for (i = 0; i < _smi_nbr_procs; i++) {
	   _smi_machine_rank[i] = 0;
	   for (j = 0; j < i; j++)
	       if (rmt_sci_ids[j] != rmt_sci_ids[j+1])
		   _smi_machine_rank[i]++;
       }
       _smi_my_machine_rank = _smi_machine_rank[_smi_my_proc_rank];
       
       free (rmt_sci_ids);
   }
#endif

   DSECTLEAVE; return(SMI_SUCCESS);
 }

/********************************************************************************/
/*** Determine the process_ids of all processes                               ***/
/********************************************************************************/

smi_error_t _smi_get_pids()
{
  DSECTION("_smi_get_pids");
  smi_error_t mpi_error;
  
  DSECTENTRYPOINT;

  ALLOCATE(_smi_pids, int *, (sizeof(int) * _smi_nbr_procs));
  _smi_pids[_smi_my_proc_rank] = getpid();

  mpi_error = _smi_ll_allgather(_smi_pids + _smi_my_proc_rank, 1, _smi_pids, _smi_my_proc_rank);
  ASSERT_R((mpi_error==MPI_SUCCESS),"MPIAllgather failed",1000+mpi_error);

  DSECTLEAVE; return(SMI_SUCCESS);
}


/********************************************************************************/
/*** Determine the total number of machines and store this in the global      ***/
/*** variable 'no_machines '.                                                 ***/
/*** This function cannot generate any error but return each time SMI_SUCCESS.***/
/********************************************************************************/
smi_error_t _smi_set_no_machines()
 {
   int i;

   _smi_nbr_machines = 1;
   for(i=0;i<_smi_nbr_procs-1;i++)
      if (_smi_machine_rank[i] != _smi_machine_rank[i+1])
	 _smi_nbr_machines++;

   return(SMI_SUCCESS);
 }
 
 

 
/********************************************************************************/
/*** This function determines whether all processes reside on the same        ***/
/*** machine or on a cluster of distinct machines. The global variable        ***/
/*** 'all_on_one' is set appropriate. This function requires that machine     ***/
/*** ranks and the number of machines are already set.                        ***/
/*** SMI_SUCCESS is returned always.                                          ***/
/********************************************************************************/
smi_error_t _smi_determine_closeness()
 {
   int i;
   
   _smi_all_on_one = true;

   for(i=0;i<_smi_nbr_procs;i++)
      if (_smi_machine_rank[i]!=_smi_machine_rank[0])
	 _smi_all_on_one = false;
   
   return(SMI_SUCCESS);
 }


 











