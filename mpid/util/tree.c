/* $Id$ */

#include "tree.h"

#define DO_AUTOBALANCE 1

#define MAKE_NEW_NODE(t, nn, lson, rson, f, ldepth, rdepth, h, d) \
        nn = MPID_SBalloc(t->node_allocator); \
        nn->leftson = lson; \
        nn->rightson = rson; \
        nn->father = f; \
        nn->left_depth = ldepth; \
        nn->right_depth = rdepth; \
        nn->hook = h; \
        nn->data = d;
        

/* helpmacros for balancing and depth calculation */
#define MAXDEPTH(pNode) ((pNode == NULL)? -1 : (((pNode)->right_depth > (pNode)->left_depth) ? (pNode)->right_depth : (pNode)->left_depth))
#define BALANCE(pNode) ((pNode)->right_depth - (pNode)->left_depth)

MPID_tree_t MPID_tree_init (int (* compare)(void *data1, void *data2), int flags)
{
    MPID_tree_t tree;

    /* allocate memory for aministration structure */
    MPID_ALLOCATE (tree, MPID_tree_t, sizeof(MPID_tree) );

    /* initialize elements */
    tree->root = NULL;
    tree->node_allocator = MPID_SBinit( sizeof(tree_node), 1, 1 );
    tree->compare = compare;

    if (flags & MPID_UTIL_THREADSAFE) {
	MPID_ALLOCATE (tree->lock, MPID_LOCK_T *, sizeof(MPID_LOCK_T) );
	MPID_INIT_LOCK(tree->lock);
    } else
	tree->lock = NULL;

    tree->nbr_entries = 0;
    tree->data_is_malloced = flags & MPID_UTIL_MALLOCED_DATA;
    tree->ignore_dups = flags & MPID_UTIL_IGNORE_DUPLICATES;

    return tree;
}

static void tree_free_node(MPID_tree_t tree, tree_node_t node)
{
    if (tree->data_is_malloced)
	free(node->data);
    MPID_SBfree(tree->node_allocator, node);
}

/* recursive fuction to delete the tree nodes */
static void tree_destroy(MPID_tree_t tree, tree_node_t pRoot)
{
    if (pRoot != NULL) {
	tree_destroy(tree, pRoot->leftson);
	tree_destroy(tree, pRoot->rightson);
	tree_free_node(tree, pRoot);
    }
}

ulong MPID_tree_nbr_entries (MPID_tree_t tree)
{
    return tree->nbr_entries;
}

void MPID_tree_destroy (MPID_tree_t tree)
{
    /* Walk through the complete tree and delete all entries; the included data may 
       be free'd if MPID_TREE_MALLOCED_DATA is given. */    
    tree_destroy(tree, tree->root);
    
    /* delete tree */
    MPID_SBdestroy( tree->node_allocator );
    if (tree->lock) {
	MPID_DESTROY_LOCK (tree->lock);
	free(tree->lock);
    }
    free(tree);
    
    return;
}


static tree_node_t tree_rotate_left(tree_node_t pPivot) 
{
    tree_node_t pNewPivot = pPivot->rightson;
    tree_node_t pOldLeft;
    tree_node_t* ppPivot;
    tree_node_t pRetVal = pPivot;

    if ((pPivot == NULL) || (pPivot->father == NULL))
	return(pRetVal);
    
    /* left rotation only is possible, if right son exists */
    if (pNewPivot != NULL) {
	
	if (pPivot->hook == hk_none) {
	    fprintf(stderr, "assertionfailure (pPivot->hook == hk_none)\n");
	    exit(-1);
	}
	
	/* save old left node of new pivot */
	pOldLeft = pNewPivot->leftson;
	
	/* get adress witch points to old pivot */
	ppPivot = (pPivot->hook == hk_left) ? &(pPivot->father->leftson) : &(pPivot->father->rightson);
	
	/* old pivot becomes left son of new pivot */
	pNewPivot->leftson = pPivot;
	
	/* parent of old pivot has to point to new pivot */
	*ppPivot = pNewPivot;
	
	/* connecting counterparts of new pivot */
	pNewPivot->hook = pPivot->hook;
	pNewPivot->father = pPivot->father;
	
	/* connecting counterparts of old pivot */
	pPivot->hook = hk_left;
	pPivot->father = pNewPivot;
	
	/* old left son of new pivot now becomes right son of old pivot*/
	pPivot->rightson = pOldLeft;
	if (pOldLeft != NULL) {
	    pOldLeft->hook = hk_right;
	    pOldLeft->father = pPivot;
	}
	
	/* adjust depth data of affected nodes */
	pPivot->right_depth = MAXDEPTH(pOldLeft) + 1;
	pNewPivot->left_depth = MAXDEPTH(pPivot) + 1;	    
    }
    
    pRetVal = pNewPivot;      
    return(pRetVal);
}

static tree_node_t tree_rotate_right(tree_node_t pPivot) 
{
    tree_node_t pNewPivot = pPivot->leftson;
    tree_node_t pOldRight;
    tree_node_t* ppPivot;
    tree_node_t pRetVal = pPivot;
    
    if ((pPivot == NULL) || (pPivot->father == NULL))
	return(pRetVal);
	
    /* right rotation only is possible, if left son exists */
    if (pNewPivot != NULL) {
	
	if (pPivot->hook == hk_none) {
	    fprintf(stderr, "assertionfailure (pPivot->hook == hk_none)\n");
	    exit(-1);
	}
	
	/* save old right node of new pivot */
	pOldRight = pNewPivot->rightson;
	
	/* get adress witch points to old pivot */
	ppPivot = (pPivot->hook == hk_left) ? &(pPivot->father->leftson) : &(pPivot->father->rightson);
	
	/* old pivot becomes right son of new pivot */
	pNewPivot->rightson = pPivot;
	
	/* parent of old pivot has to point to new pivot */
	*ppPivot = pNewPivot;
	
	/* connecting counterparts of new pivot */
	pNewPivot->hook = pPivot->hook;
	pNewPivot->father = pPivot->father;
	
	/* connecting counterparts of old pivot */
	pPivot->hook = hk_right;
	pPivot->father = pNewPivot;
	    
	/* old right son of new pivot now becomes left son of old pivot*/
	pPivot->leftson = pOldRight;
	if (pOldRight != NULL) {
	    pOldRight->hook = hk_left;
	    pOldRight->father = pPivot;
	}
	
	/* adjust depth data of affected nodes */
	pPivot->left_depth = MAXDEPTH(pOldRight) + 1;
	pNewPivot->right_depth = MAXDEPTH(pPivot) + 1;	       
    }
    
    pRetVal = pNewPivot;
    return(pRetVal);
}


/* Insert 'new_node' with content data in subtree of 'tree' beginning at 'root' */
static void tree_insert (MPID_tree_t tree, tree_node_t root, void *new_data)
{    
    int cmp_result; 
    tree_node_t new_node;

#if (0 && DO_AUTOBALANCE)
    printf("--\n");
    printf("-new_node %x\n",new_node);
    printf("-root %x\n",root);
    printf("-root->leftson %x\n",root->leftson);
    printf("-root->rightson %x\n",root->rightson);
    printf("-root->father %x\n",root->father);
    printf("--\n");
#endif

    if (new_data != NULL && root != NULL) {
        cmp_result = tree->compare(new_data, root->data);
	/* checking if key is greater than actual root node key... */
	if (cmp_result < 0) { 
	    /* ...yes, go right */
	    if (root->rightson != NULL)
		tree_insert(tree, root->rightson, new_data);
	    else {
		/* inserting new node as right son of node */
	        MAKE_NEW_NODE (tree, new_node, NULL, NULL, root, 0, 0, hk_right, new_data);
		root->rightson = new_node; 
	    }
	    tree->nbr_entries++;
	    root->right_depth = MAXDEPTH(root->rightson) + 1;

#if  DO_AUTOBALANCE
	    /* balance if neccesary */
	    if (root != tree->root && (BALANCE(root) > 1)) {
		if (0&&(BALANCE(root->rightson) < 0))
		    tree_rotate_right(root->rightson);
		tree_rotate_left(root);
	    }
#endif
	} else {
	  if (cmp_result == 0 && tree->ignore_dups) {
	    /* entry already exists -> forget it and return! */
	    return;
	  } else {
	    /* ...no, go left */
	    if (root->leftson != NULL)
		tree_insert(tree, root->leftson, new_data);
	    else {
		/* inserting new node as left son of node */
	        MAKE_NEW_NODE (tree, new_node, NULL, NULL, root, 0, 0, hk_left, new_data);
		root->leftson = new_node; 
	    }
	    tree->nbr_entries++;
	    root->left_depth = MAXDEPTH(root->leftson) + 1;
	    
#if DO_AUTOBALANCE
	    /* balance if neccesary */	    
	    if (root != tree->root && (BALANCE(root) < -1)) {
		if (BALANCE(root->leftson) > 0)
		    tree_rotate_left(root->leftson);
		tree_rotate_right(root);
	    }
#endif
	  }
	}
    } else {
	if (new_data != NULL) {
	    MAKE_NEW_NODE (tree, new_node, NULL, NULL, NULL, 0, 0, hk_none, new_data);
	    tree->root = new_node;
	}
    }
}

static tree_node_t tree_find_min_greater(tree_node_t pReference)
{
    tree_node_t pTemp = pReference;
    
    if ( (pTemp != NULL) && (pTemp->rightson != NULL) ) {
	pTemp = pTemp->rightson;
	while (pTemp->leftson != NULL)
	    pTemp = pTemp->leftson;
    }
    
    return pTemp;
}

/* Remove 'rem_node' from 'tree' and reorder it. */
static void tree_remove (MPID_tree_t tree, tree_node_t pDelete)
{
    tree_node_t pTemp;
    void* pData;
    
    /* checking if node exists */
    if (pDelete != NULL) {
	
	if ((pDelete->hook == hk_none) && (pDelete != tree->root)) {
	    fprintf(stderr, "assertionfailure (pDelete->hook == hk_none) for none rootnode\n");
	    fprintf(stderr,"pDelete(%p) tree->root(%p)\n", pDelete, tree->root);
	    exit(-1);
	}
	
	/* checking if node has both left and right son */
	if ( (pDelete->leftson != NULL) && (pDelete->rightson != NULL) ) {
	    
	    /* yes, looking for the lowest key greater given key */
	    pTemp = tree_find_min_greater(pDelete);	    
	   	    
	    /* swapping the contents of the node to be deleted and the node found */
 	    pData = pDelete->data;
	    pDelete->data = pTemp->data;
	    pTemp->data = pData;
	    
	    /* now deleting node that cannot have both left an right son */
	    pDelete = pTemp;	    
	}
	
	/* if there's any son, take it */
	pTemp = (pDelete->leftson != NULL) ? pDelete->leftson : pDelete->rightson;
	if (pTemp != NULL) {
	    /* creating counterpart for the son which now will become son of the */
	    /* parent from the node to be deleted */
	    pTemp->father = pDelete->father;
	    pTemp->hook = pDelete->hook;
	}
		
	/* checking if note to be deleted is: */
	switch(pDelete->hook) {
	    
	    /* root node */
	case hk_none:
	    /* saving pointer to root */
	    tree->root = pTemp;
	    pTemp = NULL;
	    break;
	    
	    /* left son */
	case hk_left:
	    /* connecting new son left of old parent */
	    pDelete->father->leftson = pTemp;
	    pDelete->father->left_depth -= 1;	    
	    /* saving pointer to parent */
	    pTemp = pDelete->father;
	    break;
	    
	    /* right son */
	case hk_right:
	    /* connecting new son rightt of old parent */
	    pDelete->father->rightson = pTemp;
	    pDelete->father->right_depth -= 1;
	    /* saving pointer to parent */
	    pTemp = pDelete->father;
	    break;
	    
	default:
	    break;
	}
	
	/* removing node to be deleted */
	tree_free_node(tree, pDelete);

#if DO_AUTOBALANCE
	/* backtracking path from deleted node to root */
	while ((pTemp != NULL) && (pTemp->father != NULL)) {
	    
	    /* recalculating depth of branch with current node as root */
	    if (pTemp->hook == hk_left) {
		pTemp->father->left_depth = MAXDEPTH(pTemp) + 1;		
	    }
	    else {
		pTemp->father->right_depth = MAXDEPTH(pTemp) + 1;			  
	    }
	    
	    /* taking a look at parent */
	    pTemp = pTemp->father;
	    
	    /* checking if parent's depth of right branch overweights left more than 1 */
	    if ((tree->root != pTemp) && (BALANCE(pTemp) > 1)) {
		
		/* making sure right does not counterweight */
		if (BALANCE(pTemp->rightson) < 0)
		    tree_rotate_right(pTemp->rightson);
		
		/* rotating left */
		tree_rotate_left(pTemp);
	    }
	    else {		
		/* checking if parant's depth of left branch overweights right more than 1 */
		if ((tree->root != pTemp) && (BALANCE(pTemp) < -1)) {
		    
		    /* making sure left does not counterweight */
		    if (BALANCE(pTemp->leftson) > 0)
			tree_rotate_left(pTemp->leftson);
		    
		    /* rotating right */
		    tree_rotate_right(pTemp);
		}
	    }
	}
#endif /* DO_AUTOBALANCE */
    }
    
    return;
}


void MPID_tree_insert (MPID_tree_t tree, void *data)
{
    tree_node_t node, new_node;
    int cmp_result;

    if (tree->lock != NULL)
	MPID_LOCK(tree->lock);

    node = tree->root;
    if (node == NULL) {
	/* we have no tree yet -> create root node */
        MAKE_NEW_NODE (tree, new_node, NULL, NULL, NULL, 0, 0, hk_none, data);
	tree->root = new_node;
    } else {
        cmp_result = (tree->compare)(data, node->data);
	if (cmp_result < 0) {
	    /* insert in left subtree */
	    if( node->leftson != NULL )
		tree_insert( tree, node->leftson, data);
	    else {
  	        MAKE_NEW_NODE (tree, new_node, NULL, NULL, node, 0, 0, hk_left, data);
		node->leftson = new_node;

		tree->nbr_entries++;
	    }
	} else {
	  if (cmp_result != 0 || !tree->ignore_dups) {
	    /* insert in right subtree */
	    if (node->rightson != NULL)
		tree_insert( tree, node->rightson, data);
	    else {
  	        MAKE_NEW_NODE (tree, new_node, NULL, NULL, node, 0, 0, hk_right, data);
		node->rightson = new_node;

		tree->nbr_entries++;
	    }
	  }
       }
    }

    if (tree->lock != NULL)
	MPID_UNLOCK(tree->lock);
    return;
}

/* Return the node with the smallest (max == 0 )or biggest (max == 1) data entry. */
static tree_node_t tree_get (tree_node_t start_node, int max)
{
    tree_node_t node = start_node;
    
    if (start_node != NULL) {
	/* Find the left-most or right-most entry in the tree, starting at 'start_node'. */
	if (max == 0) {
	    while (node->leftson != NULL)
		node = node->leftson;
	} 
	else {
	    while (node->rightson != NULL)
		node = node->rightson;
	}
    }
    return node;
}

void *MPID_tree_get_smallest( MPID_tree_t tree)
{
    void *result = NULL;
    tree_node_t node;

    if (tree->lock != NULL)
	MPID_LOCK(tree->lock);

    if (tree->root != NULL) {
	node = tree_get (tree->root, 0);
    }
    if (node != NULL)
	result = node->data;

    if (tree->lock != NULL)
	MPID_UNLOCK(tree->lock);
    return result;
}

void *MPID_tree_remove_smallest( MPID_tree_t tree)
{
    void *result = NULL;
    tree_node_t node;

    if (tree->lock != NULL)
	MPID_LOCK(tree->lock);

    if (tree->root != NULL) {
	node = tree_get (tree->root, 0);
	if (node != NULL) {
	    result = node->data;
	    tree_remove (tree, node);
	    tree->nbr_entries--;
	}
    }

    if (tree->lock != NULL)
	MPID_UNLOCK(tree->lock);    
    return result;
}

void *MPID_tree_get_biggest( MPID_tree_t tree)
{
    void *result = NULL;
    tree_node_t node;

    if (tree->lock != NULL)
	MPID_LOCK(tree->lock);

    if (tree->root != NULL) {
	node = tree_get (tree->root, 1);
    }
    if (node != NULL)
	result = node->data;

    if (tree->lock != NULL)
	MPID_UNLOCK(tree->lock);    
    return result;
}

void *MPID_tree_remove_biggest( MPID_tree_t tree)
{
    void *result = NULL;
    tree_node_t node;
    
    if (tree->lock != NULL)
	MPID_LOCK(tree->lock);
    
    if (tree->root != NULL) {
	node = tree_get (tree->root, 1);
	if (node != NULL) {
	    result = node->data;
	    tree_remove (tree, node);
	}
    }
    tree->nbr_entries--;

    if (tree->lock != NULL)
	MPID_UNLOCK(tree->lock);    
    return result;
}


static tree_node_t tree_find(MPID_tree_t tree, void *data)
{
    tree_node_t pFound = NULL;
    tree_node_t pTemp = tree->root;

    if (data == NULL) 
	return(NULL);
    
    while ((pTemp != NULL) && (pFound == NULL)) {
	if (tree->compare(data, pTemp->data) == 0) 
	    pFound = pTemp;
	else {
	    if (tree->compare(data, pTemp->data) < 0) 
		pTemp = pTemp->leftson; 
	    else
		pTemp = pTemp->rightson;
	}
    }
    
    return(pFound);
}

int MPID_tree_remove (MPID_tree_t tree, void *data) {
    tree_node_t pDelete;
    int iResult = 0;
    
    if (tree->lock != NULL)
	MPID_LOCK(tree->lock);
    
    if (tree->root != NULL) {
	pDelete = tree_find (tree, data);
    }
    
    if (pDelete != NULL) {
	iResult = 1;
	tree_remove (tree, pDelete);
	tree->nbr_entries--;
    }
    
    if (tree->lock != NULL)
	MPID_UNLOCK(tree->lock);    
    
    return(iResult);
}


void MPID_tree_lock (MPID_tree_t tree)
{
    if (tree->lock != NULL)
	MPID_LOCK(tree->lock);
}


void MPID_tree_unlock (MPID_tree_t tree)
{
    if (tree->lock != NULL)
	MPID_UNLOCK(tree->lock);
}
