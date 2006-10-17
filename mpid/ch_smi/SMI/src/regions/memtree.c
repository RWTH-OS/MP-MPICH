/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <env/smidebug.h>
#include "memtree.h"

/* the tree wich hold all adressareas used by regions */
memtree_t _smi_memtree;

#define MAXDEPTH(pNode) ((pNode == NULL)? -1 : (((pNode)->iRightDepth > (pNode)->iLeftDepth) ? (pNode)->iRightDepth : (pNode)->iLeftDepth))
#define BALANCE(pNode) ((pNode)->iRightDepth - (pNode)->iLeftDepth)

/* datatype dependend functions */
/* the function to compare memareas.
   returnvalues: 
       1 if memarea A is located "right" (up) from memarea B
      -1 if memarea A is located "left" (below) from memarea B
       0 if area A is completey contained in area B
*/ 
static int memtree_memarea_compare (memtree_memarea_t* pAreaA, memtree_memarea_t* pAreaB)
{
    REMDSECTION("memtree_memarea_compare");
  
    DSECTENTRYPOINT;

    DNOTICEP("A->pStart =", pAreaA->pStart);
    DNOTICEI("A->iSize =", pAreaA->iSize);
    DNOTICEP("B->pStart =", pAreaB->pStart);
    DNOTICEI("B->iSize =", pAreaB->iSize);
    
    if ((pAreaA->pStart >= pAreaB->pStart) 
	&& ((size_t)pAreaA->pStart + pAreaA->iSize <= (size_t)pAreaB->pStart + pAreaB->iSize)) {
      DNOTICE ("the keys do match");
      DSECTLEAVE; return 0;
    }

    if (pAreaA->pStart <= pAreaB->pStart) {
      DNOTICE("key A is equal or less");
      DSECTLEAVE; return -1;
    }
    
    if (pAreaA->pStart > pAreaB->pStart) {
      DNOTICE("key A is greater");
      DSECTLEAVE; return 1;
    }
	DERROR ("Illegal area configuration!?");
	return -99;
}

/* 
   the function returns a key, for wich the following is always true:
   avl_memarea_compare(avl_memarea_null(), A) == -1
   avl_memarea_compare(A, avl_memarea_null()) ==  1
   avl_memarea_compare(avl_memarea_null(), avl_memarea_null()) ==  0
   avl_memarea_null() == avl_memarea_null()
   for any possible Key A
*/

static memtree_memarea_t* memtree_memarea_null(void)
{
    static memtree_memarea_t memarea_null = {NULL,0};
    
    return(&memarea_null);
}




/* general functions */

static memtree_node_t* memtree_mknode(memtree_memarea_t* pMemArea, int segid)
{
    REMDSECTION("memtree_mknode");
    memtree_node_t* pTemp;
    
    pTemp = (memtree_node_t*) malloc(sizeof(memtree_node_t));
    ASSERT_A( ( pTemp != NULL ), "no memory left for operation", SMI_ERR_NOMEM); 
    memcpy(&(pTemp->MemArea), pMemArea, sizeof(memtree_memarea_t));
    pTemp->iSegmentId = segid;
    
    pTemp->iLeftDepth = 0;
    pTemp->iRightDepth = 0;
    pTemp->pLeft = NULL;
    pTemp->pRight = NULL;
    pTemp->pParent = NULL;
    pTemp->Hook = hk_null;

    DNOTICEP("created node", pTemp);
    
    return(pTemp);
}

static void memtree_freenode(memtree_node_t* pNode)
{
    free(pNode);
}

static memtree_node_t* memtree_find(memtree_t root, memtree_memarea_t* pMemArea)
{
    REMDSECTION("memtree_find");
    memtree_node_t* pTemp = root->pRight;
    
    DSECTENTRYPOINT;

    DNOTICEP("starting search at", pTemp);
    while ((pTemp != NULL) && (memtree_memarea_compare(pMemArea, &(pTemp->MemArea)) != 0) ) {
      if ( memtree_memarea_compare(pMemArea, &(pTemp->MemArea))  > 0) 
	pTemp = pTemp->pRight;
      else
	pTemp = pTemp->pLeft;
      DNOTICEP("step to adress", pTemp);
    }
    
    DSECTLEAVE; return pTemp;
}

static memtree_node_t* memtree_find_min_greater(memtree_node_t* pReference)
{
    memtree_node_t* pTemp = pReference;
    
    if ( (pTemp != NULL) && (pTemp->pRight!=NULL) ) {
	pTemp = pTemp->pRight;
	while (pTemp->pLeft != NULL)
	    pTemp = pTemp->pLeft;
    }
    
    return pTemp;
}

static void memtree_swap_contents(memtree_node_t* pA, memtree_node_t* pB)
{
    memtree_memarea_t tmpKey;
    int tmpInfo;
    
    memcpy(&tmpKey, &(pA->MemArea), sizeof(memtree_memarea_t));
    tmpInfo = pA->iSegmentId;
    
    memcpy(&(pA->MemArea), &(pB->MemArea), sizeof(memtree_memarea_t));
    pA->iSegmentId = pB->iSegmentId;
    
    memcpy(&(pB->MemArea), &tmpKey, sizeof(memtree_memarea_t));
    pB->iSegmentId = tmpInfo;
}

static void memtree_rotate_left(memtree_node_t* pPivot) 
{
    REMDSECTION("memtree_rotate_left");
    memtree_node_t* pNewPivot = pPivot->pRight;
    memtree_node_t* pOldLeft;
    memtree_node_t** ppPivot;
    
    DSECTENTRYPOINT;

    DNOTICEP("pPivot", pPivot);
    
    /* left rotation only is possible, uf right son exists */
    if (pNewPivot != NULL) {
	
	if (pPivot->Hook == hk_null) {
	    fprintf(stderr, "assertionfailure (pPivot->Hook == hk_null)\n");
	    exit(-1);
	}
	
	/* save old left node of new pivot */
	pOldLeft = pNewPivot->pLeft;
	
	/* get adress witch points to old pivot */
	ppPivot = (pPivot->Hook == hk_left) ? &(pPivot->pParent->pLeft) : &(pPivot->pParent->pRight);
	
	/* old pivot becomes left son of new pivot */
	pNewPivot->pLeft = pPivot;
	
	/* parent of old pivot has to point to new pivot */
	*ppPivot = pNewPivot;
	
	/* connecting counterparts of new pivot */
	pNewPivot->Hook = pPivot->Hook;
	pNewPivot->pParent = pPivot->pParent;
	
	/* connecting counterparts of old pivot */
	pPivot->Hook = hk_left;
	pPivot->pParent = pNewPivot;
	
	/* old left son of new pivot now becomes right son of new pivot*/
	pPivot->pRight = pOldLeft;
	if (pOldLeft != NULL) {
	    pOldLeft->Hook = hk_right;
	    pOldLeft->pParent = pPivot;
	}
	
	/* adjust depth data of affected nodes */
	pPivot->iRightDepth = MAXDEPTH(pOldLeft) + 1;
	pNewPivot->iLeftDepth = MAXDEPTH(pPivot) + 1;	    
    }
    
    DSECTLEAVE;
}

static void memtree_rotate_right(memtree_node_t* pPivot) 
{
    REMDSECTION("memtree_rotate_right");
    memtree_node_t* pNewPivot = pPivot->pLeft;
    memtree_node_t* pOldRight;
    memtree_node_t** ppPivot;
    
    DSECTENTRYPOINT;

    DNOTICEP("pPivot", pPivot);
	
    /* right rotation only is possible, if left son exists */
    if (pNewPivot != NULL) {
	
	if (pPivot->Hook == hk_null) {
	    fprintf(stderr, "assertionfailure (pPivot->Hook == hk_null)\n");
	    exit(-1);
	}
	
	/* save old right node of new pivot */
	pOldRight = pNewPivot->pRight;
	
	/* get adress witch points to old pivot */
	ppPivot = (pPivot->Hook == hk_left) ? &(pPivot->pParent->pLeft) : &(pPivot->pParent->pRight);
	
	/* old pivot becomes right son of new pivot */
	pNewPivot->pRight = pPivot;
	
	/* parent of old pivot has to point to new pivot */
	*ppPivot = pNewPivot;
	
	/* connecting counterparts of new pivot */
	pNewPivot->Hook = pPivot->Hook;
	pNewPivot->pParent = pPivot->pParent;
	
	/* connecting counterparts of old pivot */
	pPivot->Hook = hk_right;
	pPivot->pParent = pNewPivot;
	    
	/* old right son of new pivot now becomes left son of new pivot*/
	pPivot->pLeft = pOldRight;
	if (pOldRight != NULL) {
	    pOldRight->Hook = hk_left;
	    pOldRight->pParent = pPivot;
	}
	
	/* adjust depth data of affected nodes */
	pPivot->iLeftDepth = MAXDEPTH(pOldRight) + 1;
	pNewPivot->iRightDepth = MAXDEPTH(pPivot) + 1;	       
    }
    
    DSECTLEAVE;
}




/* exported functions */

memtree_t _smi_memtree_empty()
{
    return(memtree_mknode(memtree_memarea_null(), 0));
}

memtree_t _smi_memtree_insert(memtree_t root, memtree_memarea_t* pMemArea, int segid)
{
    REMDSECTION("avl_insert");
    memtree_node_t* pNew;

    DSECTENTRYPOINT;

    DNOTICEP("actual node", root);
    DNOTICEI("balance:", BALANCE(root));
    
    DNOTICE("checking if nullkey...");
    if (memtree_memarea_compare(pMemArea, memtree_memarea_null()) != 0) {
	DNOTICE("...no, checking if key is greater than actual root node key...");
	if ( memtree_memarea_compare(pMemArea, &(root->MemArea)) > 0) {
	    DNOTICE("...yes, go right");
	    if (root->pRight != NULL)
		_smi_memtree_insert(root->pRight, pMemArea, segid);
	    else {
		DNOTICEP("inserting new node as right son of Node", root);
		pNew = memtree_mknode(pMemArea, segid);
		pNew->pParent = root;
		pNew->Hook = hk_right;
		root->pRight = pNew;
	    }
	    root->iRightDepth = MAXDEPTH(root->pRight) + 1;
	    
	    if ((memtree_memarea_compare(&(root->MemArea), memtree_memarea_null()) != 0) && (BALANCE(root) > 1)) {
		if (BALANCE(root->pRight) < 0)
		    memtree_rotate_right(root->pRight);
		memtree_rotate_left(root);
	    }
	}
	else {
	    DNOTICE("...no,checking if key is less than actual root node key...");
	    if (memtree_memarea_compare(pMemArea, &(root->MemArea)) < 0) {
		DNOTICE("...yes, go left");
		if (root->pLeft != NULL)
		    _smi_memtree_insert(root->pLeft, pMemArea, segid);
		else {
		    DNOTICEP("inserting new node as left son of Node", root);
		    pNew = memtree_mknode(pMemArea, segid);
		    pNew->pParent = root; 
		    pNew->Hook = hk_left;
		    root->pLeft = pNew;
		}
		root->iLeftDepth = MAXDEPTH(root->pLeft) + 1;
		
		
		if ((memtree_memarea_compare(&(root->MemArea), memtree_memarea_null()) != 0) && (BALANCE(root) < -1)) {
		    if (BALANCE(root->pLeft) > 0)
			memtree_rotate_left(root->pLeft);
		    memtree_rotate_right(root);
		}
	    }
	    else {
		DNOTICE("...no, nothing to do");
	    }
	} 
    }
    else {
 	DNOTICE("...yes, nothing to do");
    }
    
    DSECTLEAVE
	return(root);
}

memtree_t _smi_memtree_delete(memtree_t root, memtree_memarea_t* pMemArea)
{
    REMDSECTION("avl_delete");
    memtree_node_t* pDelete;
    memtree_node_t* pTemp;

    DSECTENTRYPOINT;
    
    DNOTICE("looking for node with given key");
    pDelete = memtree_find(root, pMemArea);
    
    DNOTICE("checking if node exists...");
    if (pDelete != NULL) {
	
	if (pDelete->Hook == hk_null) {
	    fprintf(stderr, "assertionfailure (pDelete->Hook == hk_null)\n");
	    exit(-1);
	}
	
	DNOTICE("...yes, checking if node has both left and right son...");
	if ( (pDelete->pLeft != NULL) && (pDelete->pRight != NULL) ) {
	    
	    DNOTICE("yes, looking for the lowest key greater given key ");
	    pTemp = memtree_find_min_greater(pDelete);
	    
	    
	    DNOTICE("swapping the contents of the node to be deleted and the node found");
 	    memtree_swap_contents(pDelete, pTemp);
	    
	    /* now deleting node that cannot have both left an right son */
	    pDelete = pTemp;	    
	}
	
	
	/* if there's any son, take it */
	pTemp = (pDelete->pLeft != NULL) ? pDelete->pLeft : pDelete->pRight;
	if (pTemp != NULL) {
	    /* creating counterpart for the son which now will become son of the */
	    /* parent from the node to be deleted */
	    DNOTICE("creating counterpart for son which will move up");	    
	    pTemp->pParent = pDelete->pParent;
	    pTemp->Hook = pDelete->Hook;
	}
	
	DNOTICE("checking if node to be deleted is left son...");
	if (pDelete->Hook == hk_left) {
	    DNOTICE("...yes, connecting new son left of old parent");
	    pDelete->pParent->pLeft = pTemp;
	    pDelete->pParent->iLeftDepth -= 1;
	}
	else {
	    DNOTICE("...no, connecting new son rightt of old parent");
	    pDelete->pParent->pRight = pTemp;
	    pDelete->pParent->iRightDepth -= 1;
	}
	
	/* saving pointer to parent */
	pTemp = pDelete->pParent;

	DNOTICE("removing node to be deleted");
	memtree_freenode(pDelete);
	
	DNOTICE("backtracking path from deleted node to root");
	while ((pTemp != NULL) && (pTemp->pParent != NULL)) {
	    
	    DNOTICEP("current node:", pTemp);

	    DNOTICE("recalculating depth of branch with current node as root");
	    if (pTemp->Hook == hk_left) {
		pTemp->pParent->iLeftDepth = MAXDEPTH(pTemp) + 1;		
	    }
	    else {
		pTemp->pParent->iRightDepth = MAXDEPTH(pTemp) + 1;			  
	    }
	    
	    /* taking a look at parent */
	    pTemp = pTemp->pParent;
	    
	    DNOTICE("checking if parant's depth of right branch overweights left more than 1...");
	    if ((memtree_memarea_compare(&(pTemp->MemArea), memtree_memarea_null()) != 0) && (BALANCE(pTemp) > 1)) {
		
		DNOTICE("...yes, making sure right does not counterweight");
		if (BALANCE(pTemp->pRight) < 0)
		    memtree_rotate_right(pTemp->pRight);
		
		DNOTICE("rotating left");
		memtree_rotate_left(pTemp);
	    }
	    else {		
		DNOTICE("...no, checking if parant's depth of left branch overweights right more than 1...");
		if ((memtree_memarea_compare(&(pTemp->MemArea), memtree_memarea_null()) != 0) && (BALANCE(pTemp) < -1)) {
		    
		    DNOTICE("...yes, making sure left does not counterweight");
		    if (BALANCE(pTemp->pLeft) > 0)
			memtree_rotate_left(pTemp->pLeft);
		    
		    DNOTICE("rotating right");
		    memtree_rotate_right(pTemp);
		}
	    }
	}
    }
    else {
	DNOTICE("...no, nothing to do");
    }
    
    DSECTLEAVE
	return(root);
}

int _smi_memtree_getsegid(memtree_t root, memtree_memarea_t* pMemArea) 
{
    memtree_node_t* pTemp = memtree_find(root, pMemArea);
    
    if (pTemp == NULL) 
	return -1;    
    else 
	return pTemp->iSegmentId;
}

boolean _smi_memtree_member(memtree_t root, memtree_memarea_t* pMemArea) {
    return((memtree_find(root, pMemArea) != NULL) ? TRUE : FALSE);
}

void _smi_memtree_kill(memtree_t* pRoot)
{
    if (*pRoot != NULL) {
	_smi_memtree_kill(&((*pRoot)->pLeft));
	_smi_memtree_kill(&((*pRoot)->pRight));
	memtree_freenode(*pRoot);
	*pRoot = NULL;
    }
}

