/* $Id$ */

/* sci_desc is an extension of the resourcelist, which manages                */
/* sci-descriptors. Since sci-descriptors can only manage one remote- and one */
/* local segment, it's job is to make sure, that both slots (remote and       */
/* localsegment) are really used before allocationg a new descriptor          */

#include <pthread.h>

#include "sci_desc.h"
#include "env/general_definitions.h"
#include "env/smidebug.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef NO_SISCI
void _smi_sci_desc_init() {}
void _smi_sci_desc_finalize() {}
#else

/* this is for testing only and should be set to '1' for normal operation */
#define REUSE_DESCRIPTORS  1

/* The list for descriptors with free localsegment slot */
static smi_sci_desc_list_t* pLocListRoot;
/* The list for descriptors with free localsegment slot */
static smi_sci_desc_list_t* pLocIntListRoot;
/* The list for descriptors with free remotesegment and interrupt slot */
static smi_sci_desc_list_t* pRmtListRoot;
/* The list for descriptors with free remotesegment and interrupt slot */
static smi_sci_desc_list_t* pRmtIntListRoot;
/* The list for descriptors with free remote and local segment slot */
static smi_sci_desc_list_t* pRmtLocListRoot;
/* The list for descriptors with free interrupt segment slot */
static smi_sci_desc_list_t* pIntListRoot;
/* The list for descriptors with no free slot */
static smi_sci_desc_list_t* pFullListRoot;

/* the list-operations require a mutex for threadsafety */
static pthread_mutex_t sdMutex;

/* module internal functions for push/pop operations on the lists */
static void sd_node_push_front(smi_sci_desc_list_t** ppRoot, smi_sci_desc_list_t* pEntry) {
    REMDSECTION("sd_node_push_front");
    DSECTENTRYPOINT;

    /* attach the first node to the new node */
    pEntry->pNext = *ppRoot;

    /*  Change external references of new node */
    if (pEntry->pLocRef)
	*(pEntry->pLocRef) = ppRoot;
    if (pEntry->pRmtRef)
	*(pEntry->pRmtRef) = ppRoot;
    if (pEntry->pIntRef)
	*(pEntry->pIntRef) = ppRoot;
    
    /* Is there already a node in the List ? */
    if (pEntry->pNext) { /* Yes */
	/* Change external References of node that moved down a position */
	if (pEntry->pNext->pLocRef)
	    *(pEntry->pNext->pLocRef) = &(pEntry->pNext);
	if (pEntry->pNext->pRmtRef)
	    *(pEntry->pNext->pRmtRef) = &(pEntry->pNext);	
	if (pEntry->pNext->pIntRef)
	    *(pEntry->pNext->pIntRef) = &(pEntry->pNext);	
    }
    
    /* now new node is the top of the list */
    *ppRoot = pEntry;

    DSECTLEAVE;
}

static void sd_node_remove(smi_sci_desc_t Desc) {
    REMDSECTION("sd_node_remove");
    smi_sci_desc_list_t* pTemp;

    DSECTENTRYPOINT;

    /* save pointer of the node to be removed */
    DNOTICE("save pointer of the node to be removed");
    pTemp = *Desc;
    
    /* change next-pointer of preceding node to the sucessor of the removed */
    DNOTICE("change next-pointer of preceding node to the sucessor of the removed");
    *Desc = pTemp->pNext;
    
    /* Is there a successor ? */
    if (pTemp->pNext) { /* Yes */
	/* Change external References of node that moved up a position */
	DNOTICE("Change external References of node that moved up a position");
	if ((*Desc)->pLocRef)
	    *((*Desc)->pLocRef) = Desc;
	if ((*Desc)->pRmtRef)
	    *((*Desc)->pRmtRef) = Desc;
	if ((*Desc)->pIntRef)
	    *((*Desc)->pIntRef) = Desc;
    }
    
    /* Invalidate nextpointer of removed node */
    DNOTICE("Invalidate nextpointer of removed node");
    pTemp->pNext=NULL;
    
    /* Invalidate external references of removed node */
    DNOTICE("Invalidate external references of removed node");
    if (pTemp->pLocRef)
	*(pTemp->pLocRef) = NULL;
    if (pTemp->pRmtRef)
	*(pTemp->pRmtRef) = NULL;
    if (pTemp->pIntRef)
	*(pTemp->pIntRef) = NULL;

    DSECTLEAVE;
}

static smi_sci_desc_list_t* sd_node_pop_front(smi_sci_desc_list_t** ppRoot) {
    smi_sci_desc_list_t* pTemp = *ppRoot;
    
    if (pTemp)
	sd_node_remove(ppRoot);    
    return (pTemp);
}

#ifndef SMI_ONLY_ONE_FD

/* init and finalizing routines */
void _smi_sci_desc_init()
{
    DSECTION("smi_sci_desc_init");

    DSECTENTRYPOINT;

    pLocListRoot = NULL;
    pLocIntListRoot = NULL;
    pRmtListRoot = NULL;
    pRmtIntListRoot = NULL;
    pIntListRoot = NULL;
    pFullListRoot = NULL;
    
    DNOTICE("init mutex");
    SMI_INIT_LOCK(&sdMutex);

    DSECTLEAVE;
}

void _smi_sci_desc_finalize() {
    DSECTION("smi_sci_desc_finalize");
    smi_sci_desc_list_t* pTemp;
    sci_error_t sci_error;

    DSECTENTRYPOINT;
    
    DNOTICE("clearing LocList");
    while ((pTemp = sd_node_pop_front(&pLocListRoot)) != NULL) {
	rs_SCIClose(pTemp->sd,0,&sci_error);
	free(pTemp);
    }

    DNOTICE("clearing LocIntList");
    while ((pTemp = sd_node_pop_front(&pLocIntListRoot)) != NULL) {
	rs_SCIClose(pTemp->sd,0,&sci_error);
	free(pTemp);
    }

    DNOTICE("clearing RmtList");
    while ((pTemp = sd_node_pop_front(&pRmtListRoot)) != NULL) {
	rs_SCIClose(pTemp->sd,0,&sci_error);
	free(pTemp);
    }

    DNOTICE("clearing RmtIntList");
    while ((pTemp = sd_node_pop_front(&pRmtIntListRoot)) != NULL) {
	rs_SCIClose(pTemp->sd,0,&sci_error);
	free(pTemp);
    }
	
    DNOTICE("clearing IntList");
    while ((pTemp = sd_node_pop_front(&pIntListRoot)) != NULL) {
	rs_SCIClose(pTemp->sd,0,&sci_error);
	free(pTemp);
    }
	
    DNOTICE("clearing FullList");
    while ((pTemp = sd_node_pop_front(&pFullListRoot)) != NULL) {
	rs_SCIClose(pTemp->sd,0,&sci_error);
	free(pTemp);
    }
    
    DNOTICE("destroy mutex");
    SMI_DESTROY_LOCK(&sdMutex);
    
    DSECTLEAVE;
}

/* transform the smi_sci_desc to an normal sci_desc */
sci_desc_t _smi_trans_scidesc(smi_sci_desc_t* pDesc) 
{
    REMDSECTION("smi_trans_scidesc");
    sci_desc_t RetVal;

    DSECTENTRYPOINT;
    
    SMI_LOCK(&sdMutex);
    RetVal = (**pDesc)->sd;
    SMI_UNLOCK(&sdMutex);
    
    DSECTLEAVE; return(RetVal);
}


/* get an sci descriptor with a free localsegemt slot */
void _smi_get_loc_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError){
    REMDSECTION("smi_get_loc_scidesc");
    smi_sci_desc_list_t* pTemp;

    DSECTENTRYPOINT;

    DNOTICE("get mutex");
    SMI_LOCK(&sdMutex);
    *pError = SCI_ERR_OK;

    DNOTICE("trying to get an SCI descriptor in-use");
    if (REUSE_DESCRIPTORS) {
	/* is there any descriptor with a free localsegment slot ? */
	pTemp = sd_node_pop_front(&pLocListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_full;
	    pTemp->pLocRef = pDesc;
	    sd_node_push_front(&pFullListRoot, pTemp);
	    goto exit;
	}
	pTemp = sd_node_pop_front(&pRmtLocListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_rmtfree;
	    pTemp->pLocRef = pDesc;
	    sd_node_push_front(&pRmtListRoot, pTemp);
	    goto exit;
	}
	pTemp = sd_node_pop_front(&pLocIntListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_intfree;
	    pTemp->pLocRef = pDesc;
	    sd_node_push_front(&pIntListRoot, pTemp);
	    goto exit;
	}
    }
    
    /* create a new node */
    AALLOCATE(pTemp, smi_sci_desc_list_t*, sizeof(smi_sci_desc_list_t)); 
    rs_SCIOpen(&(pTemp->sd),0,pError);
    if (*pError != SCI_ERR_OK) { 
	DPROBLEM("could not open a new SCI descriptor");
	free(pTemp);
	*pDesc = NULL;
    } else { 
	/* add the new node to the list of descriptors with free localsegment slot */
	DNOTICE("opened new SCI descriptor");
	pTemp->ltype = sd_rmtintfree;
	pTemp->pRmtRef = NULL;
	pTemp->pIntRef = NULL;
	pTemp->pLocRef = pDesc;
	sd_node_push_front(&pRmtIntListRoot, pTemp);
    }

 exit:    
    SMI_UNLOCK(&sdMutex);

    /* Exportparameter *pDesc is implicitly set by the push-function */
    DSECTLEAVE; return;
}

/* get an sci descriptor with a free remotesegemt slot */
void _smi_get_rmt_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError){
    REMDSECTION("smi_get_rmt_scidesc");
    smi_sci_desc_list_t* pTemp;

    DSECTENTRYPOINT;

    DNOTICE("get mutex");
    SMI_LOCK(&sdMutex);
    *pError = SCI_ERR_OK;

    DNOTICE("trying to get an SCI descriptor in-use");
    /* is there any descriptor with a free remotesegment slot ? */
    if (REUSE_DESCRIPTORS) {
	pTemp = sd_node_pop_front(&pRmtListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_full;
	    pTemp->pRmtRef = pDesc;
	    sd_node_push_front(&pFullListRoot, pTemp);
	    goto exit;
	}
	pTemp = sd_node_pop_front(&pRmtIntListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_intfree;
	    pTemp->pRmtRef = pDesc;
	    sd_node_push_front(&pIntListRoot, pTemp);
	    goto exit;
	}
	pTemp = sd_node_pop_front(&pRmtLocListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_locfree;
	    pTemp->pRmtRef = pDesc;
	    sd_node_push_front(&pLocListRoot, pTemp);
	    goto exit;
	}
    }
    
    /* create a new node */
    AALLOCATE(pTemp, smi_sci_desc_list_t*, sizeof(smi_sci_desc_list_t)); 
    rs_SCIOpen(&(pTemp->sd),0,pError);
    if (*pError != SCI_ERR_OK) { 
	DPROBLEM("could not open a new SCI descriptor");
	free(pTemp);
	*pDesc = NULL;
    } else {
	/* add the new node to the list of descriptors with free localsegment slot */
	DNOTICE("opened new SCI descriptor");
	pTemp->ltype = sd_locintfree;
	pTemp->pRmtRef = pDesc;
	pTemp->pLocRef = NULL;
	pTemp->pIntRef = NULL;
	sd_node_push_front(&pLocIntListRoot, pTemp);
    }

 exit:    
    SMI_UNLOCK(&sdMutex);

    /* Exportparameter *pDesc is implicitly set by the push-function */
    DSECTLEAVE;
}

/* get an sci descriptor with a free interrupt slot */
void _smi_get_int_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError){
    REMDSECTION("smi_get_int_scidesc");
    smi_sci_desc_list_t* pTemp;

    DSECTENTRYPOINT;

    DNOTICE("get mutex");
    SMI_LOCK(&sdMutex);
    *pError = SCI_ERR_OK;

    DNOTICE("trying to get an SCI descriptor in-use");
    /* is there any descriptor with a free remotesegment slot ? */
    if (REUSE_DESCRIPTORS) {
	pTemp = sd_node_pop_front(&pIntListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_full;
	    pTemp->pIntRef = pDesc;
	    sd_node_push_front(&pFullListRoot, pTemp);
	    goto exit;
	}
	pTemp = sd_node_pop_front(&pRmtIntListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_rmtfree;
	    pTemp->pIntRef = pDesc;
	    sd_node_push_front(&pRmtListRoot, pTemp);
	    goto exit;
	}
	pTemp = sd_node_pop_front(&pLocIntListRoot);
	if (pTemp != NULL) {	
	    pTemp->ltype = sd_locfree;
	    pTemp->pIntRef = pDesc;
	    sd_node_push_front(&pLocListRoot, pTemp);
	    goto exit;
	}
    }
	
    /* create a new node */
    AALLOCATE(pTemp, smi_sci_desc_list_t*, sizeof(smi_sci_desc_list_t)); 
    rs_SCIOpen(&(pTemp->sd),0,pError);
    if (*pError != SCI_ERR_OK) { 
	DPROBLEM("could not open a new SCI descriptor");
	free(pTemp);
	*pDesc = NULL;
    } else {
	/* add the new node to the list of descriptors with free localsegment slot */
	DNOTICE("opened new SCI descriptor");
	pTemp->ltype = sd_rmtlocfree;
	pTemp->pIntRef = pDesc;
	pTemp->pRmtRef = NULL;
	pTemp->pLocRef = NULL;
	sd_node_push_front(&pRmtLocListRoot, pTemp);
    }

 exit:    
    SMI_UNLOCK(&sdMutex);

    /* Exportparameter *pDesc is implicitly set by the push-function */
    DSECTLEAVE;
}

/* mark localsegemt slot as unused, close descriptor if all slots are unused */
void _smi_free_loc_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError) 
{
    smi_sci_desc_list_t* pTemp;

    SMI_LOCK(&sdMutex);
    
    pTemp = **pDesc;

    switch (pTemp->ltype) {
    case sd_locfree:
    case sd_locintfree:
    case sd_rmtlocfree:
	DERROR("internal list-structure error");
	SMI_Abort(-1);
	break;
    case sd_rmtintfree:
	/* descriptor becomes unused => will be completely removed */
	sd_node_remove(*pDesc);
	rs_SCIClose(pTemp->sd,0, pError);
	free(pTemp);
	break;
    case sd_full:
	/* descriptor now has a free localsegment slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_locfree;
	pTemp->pLocRef = NULL;	
	sd_node_push_front(&pLocListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    case sd_rmtfree:
	/* descriptor now has a free remote- and localsegment slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_rmtlocfree;
	pTemp->pLocRef = NULL;	
	sd_node_push_front(&pRmtLocListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    case sd_intfree:
	/* descriptor now has a free localsegment and interrupt slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_locintfree;
	pTemp->pLocRef = NULL;	
	sd_node_push_front(&pLocIntListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    default:
       	DERROR("internal list-structure error (invalid type)");
	SMI_Abort(-1);
    }

    /* invalidate the given descriptor */
    *pDesc = NULL;
    
    SMI_UNLOCK(&sdMutex);
}

/* mark remotesegemt slot as unused, close descriptor if all slots are unused */
void _smi_free_rmt_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError) 
{  
    smi_sci_desc_list_t* pTemp;
    
    SMI_LOCK(&sdMutex);
    
    pTemp = **pDesc;
   
    switch(pTemp->ltype) {
    case sd_locintfree:
	/* descriptor becomes unused => will be completely removed */
	sd_node_remove(*pDesc);
	rs_SCIClose(pTemp->sd,0,pError);
	free(pTemp);	
	break;
    case sd_rmtfree:
    case sd_rmtintfree:
    case sd_rmtlocfree:
	DERROR("internal list-structure error");
	SMI_Abort(-1);
	break;
    case sd_full:
	/* descriptor now has a free remotesegment slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_rmtfree;
	pTemp->pRmtRef = NULL;	
	sd_node_push_front(&pRmtListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    case sd_locfree:
	/* descriptor now has a free remotes and local slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_rmtlocfree;
	pTemp->pRmtRef = NULL;	
	sd_node_push_front(&pRmtLocListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    case sd_intfree:
	/* descriptor now has a free remote and interrupt slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_rmtintfree;
	pTemp->pRmtRef = NULL;	
	sd_node_push_front(&pRmtIntListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    default:
       	DERROR("internal list-structure error (invalid type)");
	SMI_Abort(-1);
    }

    /* invalidate the given descriptor */
    *pDesc = NULL;

    SMI_UNLOCK(&sdMutex);
}

/* mark interrupt slot as unused, close descriptor if all slots are unused */
void _smi_free_int_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError) 
{  
    smi_sci_desc_list_t* pTemp;
    
    SMI_LOCK(&sdMutex);
    
    pTemp = **pDesc;
   
    switch(pTemp->ltype) {
    case sd_rmtlocfree:
	/* descriptor becomes unused => will be completely removed */
	sd_node_remove(*pDesc);
	rs_SCIClose(pTemp->sd,0,pError);
	free(pTemp);	
	break;
    case sd_locintfree:
    case sd_rmtintfree:
    case sd_intfree:
	DERROR("internal list-structure error");
	SMI_Abort(-1);
	break;
    case sd_full:
	/* descriptor now has a free interrupt slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_intfree;
	pTemp->pIntRef = NULL;	
	sd_node_push_front(&pIntListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    case sd_locfree:
	/* descriptor now has a free interrupt and local slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_locintfree;
	pTemp->pIntRef = NULL;	
	sd_node_push_front(&pLocIntListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    case sd_rmtfree:
	/* descriptor now has a free interrupt and remote slot */
	sd_node_remove(*pDesc);
	pTemp->ltype = sd_rmtintfree;
	pTemp->pIntRef = NULL;	
	sd_node_push_front(&pRmtIntListRoot, pTemp);
	*pError = SCI_ERR_OK;
	break;
    default:
       	DERROR("internal list-structure error (invalid type)");
	SMI_Abort(-1);
    }

    /* invalidate the given descriptor */
    *pDesc = NULL;

    SMI_UNLOCK(&sdMutex);
}

#else /* SMI_ONLY_ONE_FD */

void _smi_sci_desc_init() 
{
    smi_sci_desc_list_t* pTemp;
    
    AALLOCATE(pTemp, smi_sci_desc_list_t *, sizeof(smi_sci_desc_list_t));
    pTemp->pRmtRef = NULL;
    pTemp->pLocRef = NULL;
    pTemp->pIntRef = NULL;
    pTemp->ltype = sd_full;
    pTemp->sd = 0;
    
    sd_node_push_front(&pFullListRoot, pTemp);
}

void _smi_sci_desc_finalize() 
{
    smi_sci_desc_list_t* pTemp;
    sci_error_t sci_error;

    pTemp = pFullListRoot;
    sd_node_remove(&pFullListRoot);
    if (pTemp->sd != 0)
        rs_SCIClose(pTemp->sd,0,&sci_error);
    free(pTemp);	
}

sci_desc_t _smi_trans_scidesc(smi_sci_desc_t* pDesc) 
{
    return((**pDesc)->sd);
}

void _smi_get_loc_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError) 
{
    *pDesc = &pFullListRoot;
    *pError = SCI_ERR_OK;
    if ((**pDesc)->sd == 0)
        rs_SCIOpen(&((**pDesc)->sd),0,pError);
}
void _smi_get_int_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError) 
{
    *pDesc = &pFullListRoot;
    *pError = SCI_ERR_OK;
    if ((**pDesc)->sd == 0)
        rs_SCIOpen(&((**pDesc)->sd),0,pError);
}
void _smi_get_rmt_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError)
{
    *pDesc = &pFullListRoot; 
    *pError = SCI_ERR_OK;
    if ((**pDesc)->sd == 0)
        rs_SCIOpen(&((**pDesc)->sd),0,pError);
}
void _smi_free_loc_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError)
{
    *pDesc = NULL; 
    *pError = SCI_ERR_OK;
}
void _smi_free_rmt_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError)
{
    *pDesc = NULL; 
    *pError = SCI_ERR_OK;
}
void _smi_free_int_scidesc(smi_sci_desc_t* pDesc, sci_error_t* pError)
{
    *pDesc = NULL; 
    *pError = SCI_ERR_OK;
}

#endif /* SMI_ONLY_ONE_FD */

#endif /* NO_SISCI */
