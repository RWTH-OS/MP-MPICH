/* $Id$ */

#ifndef _SMI_MUTEX_H_
#define _SMI_MUTEX_H_

#include "sync.h"
#include "env/general_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************************/
/*** Function Headers                                                         ***/
/********************************************************************************/

void _smi_mutex_module_init(void);
void _smi_mutex_module_finalize(void);


/*************************************/
/* Error-Codes: SMI_ERR_NOINIT       */
/*              SMI_ERR_PARAM        */
/*      		Systemerrors         */
/*              MPI Errors	         */
/* and errors form all barrier inits */
/*************************************/
smi_error_t SMI_MUTEX_INIT(int*, int, int);

/*************************************/
/* Error-Codes: SMI_ERR_NOINIT       */
/*              SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_Mutex_destroy(int);

/*************************************/
/* Error-Codes: SMI_ERR_NOINIT       */
/*              SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_Mutex_lock(int);

/*************************************/
/* Error-Codes: SMI_ERR_NOINIT       */
/*              SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_Mutex_trylock(int, int*);


/*************************************/
/* Error-Codes: SMI_ERR_NOINIT       */
/*              SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_Mutex_unlock(int);

/*************************************/
/* Error-Codes from Cmalloc          */
/*************************************/
smi_error_t SMI_Lamport_init(mutex_id*);	   

/*************************************/
/* Error-Codes from Cfree            */
/*************************************/
smi_error_t SMI_Lamport_destroy(mutex_id*);

/*************************************/
/* Error-Codes: SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_Lamport_lock(mutex_id*);

/*************************************/
/* Error-Codes: SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_Lamport_unlock(mutex_id*);

/*************************************/
/* Error-Codes from Cmalloc          */
/*************************************/
smi_error_t SMI_BurnsLynch_init(mutex_id*);

/*************************************/
/* Error-Codes from Cfree            */
/*************************************/
smi_error_t SMI_BurnsLynch_destroy(mutex_id*);

/*************************************/
/* Error-Codes: SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_BurnsLynch_lock(mutex_id*);

/*************************************/
/* Error-Codes: SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_BurnsLynch_trylock(mutex_id*, int*);

/*************************************/
/* Error-Codes: SMI_ERR_PARAM        */
/*************************************/
smi_error_t SMI_BurnsLynch_unlock(mutex_id*);

/*********************************************************************************/
/*** This functions discribe the mutex algorithm from Schulz                   ***/
/*********************************************************************************/ 
smi_error_t SMI_SchulzLock_init(mutex_id *ID);
smi_error_t SMI_SchulzLock_destroy(mutex_id *ID);
smi_error_t SMI_SchulzLock_lock(mutex_id *ID);
smi_error_t SMI_SchulzLock_trylock(mutex_id *ID, int* result);
smi_error_t SMI_SchulzLock_unlock(mutex_id *ID);

#ifdef __cplusplus
}
#endif


#endif
