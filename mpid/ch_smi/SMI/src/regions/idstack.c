/* $Id$ */

#include <stdlib.h>
#include "env/smidebug.h"
#include "idstack.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static idstack_node_t* pRoot = NULL;

void _smi_idstack_push(int id)
{
    idstack_node_t* pTemp;
    
    pTemp = (idstack_node_t*) malloc(sizeof(idstack_node_t));
    ASSERT_A( (pTemp!=NULL), "no memory left for operation", SMI_ERR_NOMEM);
    pTemp->id = id;
    pTemp->next = pRoot;
    
    pRoot = pTemp;
}

int _smi_idstack_pop() 
{
    int id;
    idstack_node_t* pTemp;
    
    if (pRoot == NULL) {
	id = -1;
    }
    else {
	id = pRoot->id;
	pTemp = pRoot;
	pRoot = pTemp->next;
	free(pTemp);
    }
    
    return(id);
}

