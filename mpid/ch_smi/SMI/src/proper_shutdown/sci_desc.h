/* $Id$ */

/* sci_desc is an extension of the resourcelist, which manages                */
/* sci-descriptors. Since sci-descriptors can only manage one remote- and one */
/* local segment, it's job is to make sure, that both slots (remote and       */
/* localsegment) are really used before allocationg a new descriptor          */

#ifndef _SMI_SCI_DESC_H
#define _SMI_SCI_DESC_H

#ifdef HAVE_CONFIG_H
/* WIN32 does not have config.h - see below */
#include "smiconfig.h"
#elif defined WIN32
#include "smiconfig_win32.h"
#else
#error No configuration file found! (smiconfig.h)
#endif

#ifndef NO_SISCI

#include <sisci_api.h>

typedef enum {
    sd_locfree,
    sd_locintfree,
    sd_rmtfree,
    sd_rmtintfree,
    sd_rmtlocfree,
    sd_intfree,
    sd_full
} smi_sci_desc_listtype_t;

typedef struct smi_sci_desc_list_t_ {
    sci_desc_t	sd;
    struct smi_sci_desc_list_t_* pNext;
    struct smi_sci_desc_list_t_*** pLocRef;
    struct smi_sci_desc_list_t_*** pRmtRef;
    struct smi_sci_desc_list_t_*** pIntRef;
    smi_sci_desc_listtype_t ltype;
} smi_sci_desc_list_t;

/* this descriptor needs to change at runtime, so each reference is registered, */
/* wich means, that only the descriptors created by smi_get_***_scidesc itself  */
/* can be used, but no copys or copys of structs containing the descriptor      */
typedef smi_sci_desc_list_t** smi_sci_desc_t;


/* transform the smi_sci_desc to an normal sci_desc */
sci_desc_t _smi_trans_scidesc(smi_sci_desc_t* pDesc);

/* get an sci descriptor with a free localsegemt slot */
void _smi_get_loc_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError);

/* get an sci descriptor with a free remotesegemt slot */
void _smi_get_rmt_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError);

/* get an sci descriptor with a free interrupt slot */
void _smi_get_int_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError);

/* mark localsegemt slot as unused, close descriptor if all slots are unused */
void _smi_free_loc_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError);

/* mark remotesegemt slot as unused, close descriptor if all slots are unused */
void _smi_free_rmt_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError);

/* mark interrupt slot as unused, close descriptor if all slots are unused */
void _smi_free_int_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError);

#endif 

/* init and finalizing routines */
void _smi_sci_desc_init(void);
void _smi_sci_desc_finalize(void);

#endif 
