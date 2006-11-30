/* $Id$ */

#include "sortedtree.h"

MPID_sorted_tree_t MPID_sorted_tree_init (size_t data_size, int (* compare)(void *data1, void *data2), int flags)
{
    MPID_sorted_tree_t tree;

    /* allocate memory for aministration structure */
    tree = (MPID_sorted_tree_t) malloc( sizeof(struct tree) );

    /* initialize elements */
    tree->root = NULL;
    tree->node_allocator = MPID_SBinit( sizeof(struct tree_node), 1, 1 );
    tree->compare = compare;
    if( flags & MPID_SORTED_TREE_THREADSAFE )
	MPID_INIT_LOCK(&(tree->lock));
    tree->data_size = data_size;
    tree->flags = flags;

    return tree;
}



/* insert node with content data in subtree beginning at root */
void _MPID_sorted_tree_insert (MPID_sorted_tree tree, tree_node_t root, tree_node_t new_node)
{

    if( (tree->compare)(data, root->data) < 0 ) {
	/* insert in left subtree */
	if( root->leftson != NULL )
	    _MPID_sorted_tree_insert(tree, root->leftson, new_node);
	else {
	    /* there is no left subtree -> insert here */
	    new_node->father = root;
	    root->leftson = new_node;
	}
    }
    else {
	/* insert in right subtree */
	if( root->rightson != NULL )
	    _MPID_sorted_tree_insert(tree, root->rightson, new_node);
	else {
	    /* there is no right subtree ->insert here */
	    new_node->father = root;
	    root->rightson = new_node;
	}
    }
}


void MPID_sorted_tree_insert (MPID_sorted_tree_t tree, void *data)
{
    tree_node_t node, new_node;

    /* make new node */
    new_node = MPID_SBalloc( tree->node_allocator );
    new_node->data = data;
    new_node->leftson = NULL;
    new_node->rightson = NULL;
    
    node = tree->root;
    if( node == NULL ) {
	/* we have no tree yet -> create root node */
	new_node->father = NULL;
	tree->root = new_node;
    }
    else {
	if( (tree->compare)(data, root->data) < 0 ) {
	    /* insert in left subtree */
	    if( node->leftson != NULL )
		_MPID_sorted_tree_insert( tree, node->leftson, new_node);
	    else {
		new_node->father = node;
		node->leftson = new_node;
	    }
	}
	else {
	    /* insert in right subtree */
	    if( node->rightson != NULL )
		_MPID_sorted_tree_insert( tree, node->rightson, new_node);
	    else {
		new_node->father = node;
		node->rightson = new_node;
	    }
	}
    }
}

void *MPID_sorted_tree_get_smallest( MPID_sorted_tree_t tree)
{
    void *result = NULL;
    tree_node_t node;

    node = tree->root;
    if( node != NULL ) {
	while( node->leftson != NULL )
	    node = node->leftson;
	
    }
}


void MPID_sorted_tree_lock (MPID_sorted_tree_t tree)
{
    if( tree->flags & MPID_SORTED_TREE_THREADSAFE )
	MPID_LOCK( &(tree->lock) );
}


void MPID_sorted_tree_unlock (MPID_sorted_tree_t tree)
{
    if( tree->flags & MPID_SORTED_TREE_THREADSAFE )
	MPID_UNLOCK( &(tree->lock) );
}
