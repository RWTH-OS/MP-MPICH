/* $Id$ */

#ifndef _MPID_HASH_H
#define _MPID_HASH_H

/* Hash-based data storage and retrieval. */

#include "utildefs.h"
#include "sbcnst2.h"


struct hash_entry {
  unsigned long key;
  void *data;
  int is_current;
	
  /* next entry with identical key (for collision strategy LINKED) */
  struct hash_entry    *next;
};
typedef struct hash_entry* hash_entry_t;

struct hash_table {
  hash_entry_t  *key_table;
  
  MPID_SBHeader  entry_allocator;
  size_t (* get_key)(void *data);
  size_t data_size;
  size_t table_size;
  MPID_LOCK_T *lock;
  int nbr_entries;
  int flags;
};
typedef struct hash_table* MPID_hash_table_t;

#define MPID_HASH_COLLISION_LINKED  4
#define MPID_HASH_COLLISION_NEXT    8

/* Initialize the data structure of a new hash table.

   Input:
   table_size		size of the table (number of key entries) to hash the keys
   data_size        size of the data entries (in bytes) that will be stored.
   key_gen			user-provided function to generate hash-key from data
   flags			Flags to modify the behaviour of the hashing:
					HASH_THREADSAFE		Data structure shall be thread-safe
					HASH_COLLISION_LINKED or HASH_COLLISION_NEXR
										collision strategy to be used
					HASH_MALLOCED_DATA	indicates that the individual data
										entries which will be stored have been
										allocated via malloc(). In this case, all
										existing data entries will be free()'ed 
										when hash_destroy() is called.
   Return Value:
   Handle of new hash table (NULL if call failed)
*/
MPID_hash_table_t MPID_hash_init (size_t table_size, size_t data_size, size_t(* key_gen)(void *data), int flags);


/* Destroy an existing hash table. All existing data entries in the
   table will be free()'ed if the table was created with the HASH_MALLOCED_DATA
   flag.

   Input:
   hash				handle to hash table to be destroyed.
*/
void MPID_hash_destroy (MPID_hash_table_t hash);

/* Store the reference to the data in the hash table.

   Input:
   hash				handle of the related hash table
   data				referene of the data to be stored. This pointer will
					be passed to the key_gen function to get the related key.

   Return Value:
   Hash key of the stored data item (< 0 if it could not be stored).
*/
unsigned long MPID_hash_store (MPID_hash_table_t hash, void *data);

/* Lock a hash table to peform multiple hash_find() / hash_next() queries
   for a single key. This is only required for hash-tables accessed by 
   multiple threads. If HASH_THREADSAFE has not been specified on hash_init(),
   this is a null operation.

   Input:
   hash				handle of the related hash table
*/ 
void MPID_hash_lock (MPID_hash_table_t hash);

/* Find an entry in the hash table via its key.

   Input:
   hash				handle of the related hash table
   key				the key of the entry to find

   Output:
   unique			true if only one entry with a key 'key' does exist
					in the hash table.
                                undefined if not found

   Return Value:
   Pointer to the data of the entry (if found); NULL if not found.
*/
void *MPID_hash_find (MPID_hash_table_t hash, unsigned long key, int *unique);

/* Find the next matching data entry to a given key.

   Input:
   hash				handle of the related hash table
   key				the key of the entry to find

   Return Value:
   Pointer to the data of the entry (if found); NULL if not found.
   if there hasn't been done a MPID_hash_find() with this key before,
   NULL is returned regardless if there is an entry with this key or not
*/
void *MPID_hash_next (MPID_hash_table_t hash, unsigned long key);

/* Unlock a hash table.

   Input:
   hash				handle of the related hash table
*/ 
void MPID_hash_unlock (MPID_hash_table_t hash);

/* Check if an entry exist which matches the supplied data.

   Input:
   hash				handle of the related hash table
   data				pointer to the data entry

   Return Value:
   > 0  a matching data entry could be found
   0    no matching data entry could be found
*/
int MPID_hash_exists (MPID_hash_table_t hash, void *data);

/* If an entry exist which matches the supplied data, the data
   entry is removed from the hash table. If HASH_MALLOCED_DATA has been
   specified, the data entry is free()'ed.

   Input:
   hash				handle of the related hash table
   data				pointer to the partial data entry

   Output:
   found			== 0  no matching data entry could be found
					> 0   found a matching data entry

   Return Value:
   != NULL   The data entry that has been removed
   == NULL   no matching data entry could be found, or it has been free()'ed
*/
void *MPID_hash_data_remove (MPID_hash_table_t hash, void *data, int *found);

/* If a *unique* entry exists which matches the supplied key, the data
   entry is removed from the hash table. If HASH_MALLOCED_DATA has been
   specified, the data entry is free()'ed.

   Input:
   hash				handle of the related hash table
   key				key of data entry

   Output:
   found			== 0  no matching data entry at all could be found,
                          or more than one entry with this key do exist.
					> 0   found a matching unique data entry

   Return Value:
   != NULL   The data entry that has been removed
   == NULL   no matching data entry could be found, or it has been free()'ed
*/
void *MPID_hash_key_remove (MPID_hash_table_t hash, unsigned long key, int *found);

/* Just return the next entry in the hash table if there is one available and
   remove it from the table.
   No key or data is supplied as this function is used to simply empty an
   existing hash table.

   Input:
   hash				handle of the related hash table

   Return Value:
   != NULL   The next data entry that has been removed.
   == NULL   No more data entry available.
*/
void *MPID_hash_empty (MPID_hash_table_t hash);

#endif



/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
