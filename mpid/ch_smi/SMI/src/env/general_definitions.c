/* $Id$ */

/*****************************************************************************/
/*****************************************************************************/
/***                                                                       ***/
/*** This file contains just the instanciations of the global variables of ***/
/*** the corresponding '.h' file, which are used throuout the SMI system.  ***/
/*** All explanations of them can be found in the '.h' file.               ***/
/***                                                                       ***/
/*****************************************************************************/
/*****************************************************************************/

#include "general_definitions.h"

/* these are referenced from smidebug.h */
#if defined(_DEBUG)
int D_REC_DEPTH = -1;
char D_REC_CHAR = '-';
int D_DO_DEBUG = FALSE;
int D_SHOW_TIME = FALSE;
double D_GLOB_TIME;
#endif

mis_t     _smi_mis;               
SMI_LOCK_T _smi_mis_lock;
int       _smi_page_size;           
int       _smi_my_proc_rank;        
int       *_smi_machine_rank;
int       _smi_my_machine_rank;
int       _smi_nbr_procs;
int       _smi_nbr_machines;
int       _smi_mpi_rank;
boolean   _smi_initialized = false;
boolean   _smi_all_on_one;
boolean   _smi_SISCI_MAP_FIXED;
int       *_smi_int_shreg_id;     
int       _smi_int_smp_shreg_id;
int       _smi_ll_sgmt_ok = FALSE;


int	  *_smi_pids;
int       *_smi_sci_rank;          
int       *_smi_nbr_adapters;
adpt_rank_t  *_smi_adpt_sci_id;

/* XXX cache parameters to be determined automatically - how? */
int       _smi_Cachelinesize = 32;
int       _smi_1stLevel_Cachesize = 8192;
int       _smi_2ndLevel_Cachesize = 262144; 

int       _smi_verbose = FALSE;
int       _smi_use_watchdog = TRUE;

int       _smi_DefAdapterNbr   = SMI_DEFAULT_ADAPTER;
int       _smi_adpt_policy     = SMI_ADPT_DEFAULT;
int       _smi_adpt_export     = SMI_ADPT_DEFAULT;
int       _smi_adpt_import     = SMI_ADPT_DEFAULT;
int       _smi_StreambufSize   = SMI_DEFAULT_STREAMBUF_SIZE;
int       _smi_NbrStreambufs   = SMI_DEFAULT_STREAMBUF_NBR;
int       _smi_AdapterType;
char      _smi_AdapterTypeS[64];

smi_args_t _smi_default_args = {0,0,0,0,SMI_DEFAULT_TCP_PORT,"","",smi_startup_none,""};

/* locks for multithreaded usage */
SMI_LOCK_T _smi_region_lock;

smi_error_t SMI_Debug(int bDebug) 
{
#if defined(_DEBUG)
    D_DO_DEBUG= bDebug;
#endif
    return(SMI_SUCCESS);
}

smi_error_t SMI_Debug_time(int bDebug) 
{
#if defined(_DEBUG)
    D_SHOW_TIME = bDebug;
#endif
    return(SMI_SUCCESS);
}
