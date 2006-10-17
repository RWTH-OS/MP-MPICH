/* $Id$ */

#include "hash.h"


MPID_hash_table_t MPID_hash_init (size_t table_size, size_t data_size, size_t(* key_gen)(void *data), int flags)
{
  MPID_hash_table_t hash;
  size_t i;

  /* allocate structure for administration of this hash table; pointer to this structure 
     is returned later */
  MPID_ALLOCATE (hash, MPID_hash_table_t, sizeof(struct hash_table) );
  
  /* allocate and initialize hash table; NULL pointer means that this slot is unused */
  MPID_ZALLOCATE (hash->key_table, hash_entry_t *, table_size*sizeof(hash_entry_t) );

  /* initialize mutex lock */
  if (flags & MPID_UTIL_THREADSAFE) {
      MPID_ALLOCATE ( hash->lock, MPID_LOCK_T *, sizeof (MPID_LOCK_T));
      MPID_INIT_LOCK( hash->lock);
  } else
      hash->lock = NULL;

  /* set function to get key from data */
  hash->get_key = key_gen;
  
  /* store flags for later use */
  hash->flags = flags;
  
  /* store size of hash table and size of data */
  hash->table_size = table_size;
  hash->data_size = data_size;
  hash->nbr_entries = 0;

  /* initialize allocator for fixed-sized blocks */
  hash->entry_allocator = MPID_SBinit( sizeof(struct hash_entry), 1, 1 );

  return hash;
}


void MPID_hash_destroy (MPID_hash_table_t hash)
{
  size_t i;
  hash_entry_t entry, next_entry;

  for( i = 0; i < hash->table_size; i++ ) {
    entry = (hash->key_table)[i];
    /* are there any entries ? */
    if( entry != NULL ) {
      do {
	next_entry = entry->next;
	if( hash->flags & MPID_UTIL_MALLOCED_DATA )
	  free( entry->data );
	MPID_SBfree( hash->entry_allocator, entry );
	entry = next_entry;
      } while( entry != NULL );
    }
  }
  
  if (hash->flags & MPID_UTIL_THREADSAFE) {
      MPID_DESTROY_LOCK(hash->lock);
      free (hash->lock);
  }

  /* destroy header for fixed-sized blocks */
  MPID_SBdestroy( hash->entry_allocator );

  /* free hash_table */
  free( hash->key_table );

  /* free structure for administration of hash table */
  free( hash );
}


unsigned long MPID_hash_store (MPID_hash_table_t hash, void *data)
{
  unsigned long key, index;
  hash_entry_t new_entry, existing_entry;

  if (hash->lock != NULL)
      MPID_LOCK(hash->lock);

  /* get key and index for hash table */
  key = (hash->get_key)( data );
  index = key % hash->table_size;

  /* create new entry to be inserted in hash table */
  new_entry = MPID_SBalloc( hash->entry_allocator );
  new_entry->key = key;
  new_entry->data = data;
  new_entry->is_current = 0;
  new_entry->next = NULL;

  /* are there already any entries with the same index ? */
  if ((existing_entry = hash->key_table[index]) != NULL) {
      /* there are already entries with the same index */
      new_entry->next = existing_entry;
      hash->key_table[index] = new_entry;
  } else {
    /* no entries with the same index yet */
    (hash->key_table)[index] = new_entry;
  }
  hash->nbr_entries++;
  
  if (hash->lock != NULL)
      MPID_UNLOCK(hash->lock);

  return key;
}


void MPID_hash_lock (MPID_hash_table_t hash)
{
    if (hash->lock != NULL)
	MPID_LOCK (hash->lock);
}


void *MPID_hash_find (MPID_hash_table_t hash, unsigned long key, int *unique)
{
  unsigned long index;
  void *result = NULL;
  hash_entry_t entry;

  /* calculate hash table index from key */
  index = key % hash->table_size;

  *unique = 1;
  /* find first entry with key */
  entry = (hash->key_table)[index];
  if( entry != NULL ) {
    while( (entry->key != key) && (entry->next != NULL) )
      entry = entry->next;
    if( entry->key == key ) {
      result = entry->data;
      entry->is_current = 1;
      /* search for another entry with the same key (to set unique accordingly) and
	 set is_current = 0 for all following entries with the same key */
      if( entry->next != NULL ) {
	entry = entry->next;
	do {
	  if( entry->key == key ) {
	    *unique = 0;
	    entry->is_current = 0;
	  }
	  entry = entry->next;
	} while( entry != NULL );
      }
    }
  }

  return result;
}


void *MPID_hash_next (MPID_hash_table_t hash, unsigned long key)
{
  unsigned long index;
  hash_entry_t entry;
  void *result = NULL;
  

  /* calculate hash table index from key */
  index = key % hash->table_size;
  
  entry = (hash->key_table)[index];

  if( entry != NULL ) {
    /* find current entry with this key */
    while( !( (entry->key == key) && (entry->is_current == 1) ) && (entry->next != NULL) )
      entry = entry->next;
    if( (entry->key == key) && (entry->is_current == 1) ) {
      /* we have find the current entry with the right key -> mark it a not
	 current anymore and search for the next entry with the right key */
      entry->is_current = 0;
      if( entry->next != NULL ) {
	entry = entry->next;
	while( (entry->key != key) && (entry->next != NULL) )
	  entry = entry->next;
	if( entry->key == key ) {
	  /* we have found the next element, mark it as current */
	  entry->is_current = 1;
	  result = entry->data;
	}
      }
    }
  }

  return result;
}


void MPID_hash_unlock (MPID_hash_table_t hash)
{
  if (hash->lock != NULL)
      MPID_UNLOCK(hash->lock);
}


int MPID_hash_exists (MPID_hash_table_t hash, void *data)
{
  unsigned long key, index;
  hash_entry_t entry;
  int result = 0;

  if (hash->lock != NULL)
      MPID_LOCK(hash->lock);

  /* get key and hash table index */
  key = (hash->get_key)(data);
  index = key % hash->table_size;

  entry = (hash->key_table)[index];
  while( entry != NULL ) {
    /* try to find an entry with the right key */
    while( (entry->key != key) && (entry->next != NULL) )
      entry = entry->next;
    if( entry->key == key ) {
      /* check if data is the same */
      if( memcmp( entry->data, data, hash->data_size ) == 0 ) {
	result = 1;
	break;
      }
    }
    entry = entry->next;
  }

  if (hash->lock != NULL)
      MPID_UNLOCK(hash->lock);

  return result;
}


void *MPID_hash_data_remove (MPID_hash_table_t hash, void *data, int *found)
{
  unsigned long key, index;
  hash_entry_t entry, last_entry;
  void *result = NULL;

  if (hash->lock != NULL)
      MPID_LOCK(hash->lock);

  /* get key and hash table index */
  key = (hash->get_key)(data);
  index = key % hash->table_size;

  *found = 0;
  entry = (hash->key_table)[index];
  while( entry != NULL ) {
    /* try to find an entry with the right key */
    while( (entry->key != key) && (entry->next != NULL) )
      entry = entry->next;
    if( entry->key == key ) {
      /* check if data is the same */
      if( memcmp( entry->data, data, hash->data_size ) == 0 ) {

	*found = 1;
	result = data;

	/* remove entry from list */
	if( entry == (hash->key_table)[index] )
	  (hash->key_table)[index] = entry->next;
	else {
	  last_entry = (hash->key_table)[index];
	  while( last_entry->next != entry )
	    last_entry = last_entry->next;
	  last_entry->next = entry->next;
	}

	/* destroy entry */
	if( hash->flags & MPID_UTIL_MALLOCED_DATA )
	  free( entry->data );
	MPID_SBfree( hash->entry_allocator, entry );

	hash->nbr_entries--;
	break;
      }
    }
    entry = entry->next;
  }

  if (hash->lock != NULL)
      MPID_UNLOCK(hash->lock);

  return result;
}

void *MPID_hash_key_remove (MPID_hash_table_t hash, unsigned long key, int *found)
{
  unsigned long index;
  hash_entry_t entry, other_entry;
  int unique = 1;
  void *result = 0;
  
  /* calculate hash table index from key */
  index = key % hash->table_size;
  *found = 0;

  if (hash->lock != NULL)
      MPID_LOCK(hash->lock);

  /* find first entry with key */
  entry = (hash->key_table)[index];
  if( entry != NULL ) {
      while( (entry->key != key) && (entry->next != NULL) )
	  entry = entry->next;
      if( entry->key == key ) {
	  /* search for another entry with the same key to check if entry is unique */
	  if( entry->next != NULL ) {
	      other_entry = entry->next;
	      do {
		  if( other_entry->key == key ) {
		      unique = 0;
		      break;
		  }
		  other_entry = other_entry->next;
	      } while( other_entry != NULL );
	  }
	  /* if we haven't found another entry with the same key -> remove this entry from list 
	     and destroy ist */
	  if( unique ) {
	      result = entry->data;
	      *found = 1;

	      /* remove entry from list */
	      if( entry == (hash->key_table)[index] )
		  (hash->key_table)[index] = entry->next;
	      else {
		  other_entry = (hash->key_table)[index];
		  while( other_entry->next != entry )
		      other_entry = other_entry->next;
		  other_entry->next = entry->next;
	      }

	      /* destroy entry */
	      if( hash->flags & MPID_UTIL_MALLOCED_DATA )
		  free( entry->data );
	      MPID_SBfree( hash->entry_allocator, entry );
	      
	      hash->nbr_entries--;
	  }
      }
  }
  
  if (hash->lock != NULL)
      MPID_UNLOCK(hash->lock);

  return result;
}


void *MPID_hash_empty (MPID_hash_table_t hash)
{
  void *result = NULL;
  hash_entry_t entry;
  size_t i = 0;

  if (hash->lock != NULL)
      MPID_LOCK(hash->lock);

  /* find an entry in the hash table */
  while( (i < hash->table_size) && ((hash->key_table)[i] == NULL) )
    i++;
  if( i < hash->table_size ) {

    entry = (hash->key_table)[i];
    result = entry->data;

    /* remove entry from list */
    (hash->key_table)[i] = entry->next;

    /* destroy entry */
    if( hash->flags & MPID_UTIL_MALLOCED_DATA )
      free( entry->data );
    MPID_SBfree( hash->entry_allocator, entry );
    
    hash->nbr_entries--;
  }

  if (hash->lock != NULL)
      MPID_UNLOCK(hash->lock);

  return result;
}
