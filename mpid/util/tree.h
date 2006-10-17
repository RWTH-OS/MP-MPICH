/* $Id: tree.h,v 1.7 2002/05/22 19:43:46 joachim Exp $ */

#ifndef _MPID_TREE_H
#define _MPID_TREE_H

#include <pthread.h>

#include "utildefs.h"
#include "sbcnst2.h"

/* MPID_tree_t is a binary & balanced tree which can store any kind
   of data sorted by one criterium. */

/* this hook indikates wether a node is a right or a left son of its father */
typedef enum { 
    hk_none, 
    hk_left, 
    hk_right
} hook_t;


/* structure of one node in the tree */
typedef struct t_n {
    struct t_n *leftson;   /* pointer to left son */
    struct t_n *rightson;  /* pointer to right son */
    struct t_n *father;    /* pointer to father node */
    int left_depth;        /* depth of left subtree */
    int right_depth;       /* depth of right subtree */
    hook_t hook;           /* see typedef explanation of hook_t */ 
    void *data;            /* pointer to data content of the node */
} tree_node;

typedef tree_node* tree_node_t;

/* structure for administration of tree */
typedef struct tree {
    tree_node_t root;             /* pointer to root node */
    MPID_SBHeader node_allocator; /* allocator for fixed sized blocks used for the nodes */
    int (* compare)(void *data1, void *data2); /* pointer to user-provided function to compare
						  two data entries; semantics:
						  -1 : data1 is smaller than data2
						   0 : data1 is equal to data2
						   1 : data1 is bigger than data2 */
    MPID_LOCK_T *lock;  /* lock for thread-safety */
    size_t data_size;   /* size of data content in bytes */
    int data_is_malloced;
    int ignore_dups;
    ulong nbr_entries;
} MPID_tree;

typedef MPID_tree * MPID_tree_t;


/* Initialize the data structure for a new sorted tree:

   Input:
   compare     user-provided function to compare two data entries
   flags       flags to modify the behaviour of the tree:
               MPID_TREE_THREADSAFE    data structure shall be threadsafe
               MPID_TREE_MALLOCED_DATA indicates that the individual data
           				      entries which will be stored have been
					      allocated via malloc(). In this case, all
					      existing data entries will be free()'ed 
					      when MPID_tree_destroy() is called.

   Return value:
   Handle of new sorted tree */					      
MPID_tree_t MPID_tree_init (int (* compare)(void *data1, void *data2), int flags);

void MPID_tree_destroy (MPID_tree_t tree);

/* Insert a new data entry in the tree

   Input:
   tree    handle of the tree in which to insert
   data    pointer to data that is to be stored
*/
void MPID_tree_insert (MPID_tree_t tree, void *data);


/* Return the number of entries stored in this tree.

   Input:
   tree    handle of the tree in which to insert
   
   Return value:
   Number of stored entries */
ulong MPID_tree_nbr_entries (MPID_tree_t tree);


/* Return the smallest entry in the tree.

   Input:
   tree    handle of the tree in which to insert

   Return value:
   Pointer to data entry; NULL if tree is empty */
void *MPID_tree_get_smallest (MPID_tree_t tree);

/* Same as MPID_tree_get_smallest, but data is removed from tree if found. */
void *MPID_tree_remove_smallest (MPID_tree_t tree);


/* Return the biggest entry in the tree.

   Input:
   tree    handle of the tree in which to insert

   Return value:
   Pointer to data entry; NULL if tree is empty */
void *MPID_tree_get_biggest (MPID_tree_t tree);

/* Same as MPID_tree_get_biggest, but data is removed from tree if found. */
void *MPID_tree_remove_biggest (MPID_tree_t tree);


/* Remove the first entry in the tree which matches the given data
   (not smaller or bigger). 

   Input:
   tree    handle of the tree in which to insert
   data    ptr to data to be removed

   Return value:
   0       no matching data found
   1       data has been removed */
int MPID_tree_remove (MPID_tree_t tree, void *data);

/* not yet used */
void MPID_tree_lock (MPID_tree_t tree);
void MPID_tree_unlock (MPID_tree_t tree);

#endif
