
   /*********************************
--- ** stl2c_basic.c               ** -------------------------------------------------------
 |  *********************************
 |
 |   The stl2c library provides some simple emulation features of the well-known
 |   C++ Standard Template Library (STL) to be used in the language C.
 |
 |   Currently only stl2c_vector, stl2c_list (alias stl2c_deque) and stl2_map
 |   are implemented to emulate their counterparts in C++.
 |
 |   The template feature is achieved by using pre-processor macros, which are
 |   defined in the respective header files.
 |
 */

#include "stl2c_basic.h"

   /*********************************
--- ** internal basic functions    ** -------------------------------------------------------
 |  *********************************
 |
 |   All the current STL emulation is based on a simple double chained list.
 |
 |   The member "first" of the stl_t structure points to the first element of this list,
 |   while the "size" member contains the number of elements in the whole list.
 |
 |   Each element possesses a pointer to its next and its previous neighbor and a void-pointer
 |   to the content to be stored. The content itself is NOT stored or copied into the list.
 | 
 |   The list can be accessed by indices or by pointers (iterators) to the elements.
 |   To provide a better access performance, a pointer to the last element being read is
 |   cached in case of a read access (see members "cache_pos" and "cache_pt").
 |
 */

static void stl_reset_run(stl_t* stl)
/*
 |  Reset the cached position in case of a write access:
 |
 */
{
  stl->cache_pos=0;
  stl->cache_pt=stl->first;
}


stl_c* stl_push_front(void* container, stl_t* stl)
/*
 |   Insert an element before the first element
 |   and return a pointer to the new element:
 */    
{
  stl_c *run;

  /* create the new element and store the pointer to the content: */
  run=(stl_c*)malloc(sizeof(stl_c));
  run->container=container;

  if(stl->size==0) /* <-- is the list still empty ? */
  {
    run->forward=run;
    run->backward=run;
    stl->first=run;
  }
  else
  { 
    /* bend the respective pointers: */
    run->forward=stl->first;
    run->backward=stl->first->backward;
    stl->first->backward->forward=run;
    stl->first->backward=run;
    stl->first=run;
  }
  stl->size++;
  
  stl_reset_run(stl);
  
  return run;
}


stl_c* stl_push_back(void* container, stl_t* stl)
/*
 |   Insert an element after the last element
 |   and return a pointer to the new element: 
 */     
{
  stl_c *run;

  /* create the new element and store the pointer to the content: */
  run=(stl_c*)malloc(sizeof(stl_c));
  run->container=container;
 
  if(stl->size==0) /* <-- is the list still empty ? */
  {
    run->forward=run;
    run->backward=run;
    stl->first=run;
  }
  else
  {  
    /* bend the respective pointers: */
    run->forward=stl->first;
    run->backward=stl->first->backward;
    stl->first->backward->forward=run;
    stl->first->backward=run;
  }
  stl->size++;

  stl_reset_run(stl);

  return run;
}


void* stl_pop_front(stl_t* stl)
/*
 |   Remove the first element of the list and return a pointer to its content.
 |   So, the content still exists and must be erased manually!
 */
{
  void* cont=NULL;
  stl_c *run;

  if(stl->size>0)
  {
    run=stl->first;
    run->backward->forward=run->forward;
    run->forward->backward=run->backward;
    stl->first=run->forward;
    cont=run->container;
    free(run);
    stl->size--; 
    if(stl->size==0) stl->first=NULL; /* <-- the list is empty again */
  }

  stl_reset_run(stl);

  return cont;
}

void* stl_pop_back(stl_t* stl)
/*
 |   Remove the last element of the list and return a pointer to its content.
 |   So, the content still exists and must be erased manually!
 */
{
  void* cont=NULL;
  stl_c *run;

  if(stl->size>0)
  {
    run=stl->first->backward;
    run->backward->forward=run->forward;
    run->forward->backward=run->backward;
    cont=run->container;
    free(run);
    stl->size--;
    if(stl->size==0) stl->first=NULL; /* <-- the list is empty again */
  }

  stl_reset_run(stl);

  return cont;
}


void stl_create(unsigned int size, stl_t** stl)
/*
 |   Create a new list for a stl_t _pointer_
 |   --> also the anchor element is to be created
 |   "size" is the initial number of elements. 
 |   
 |   You MUST call this function (or stl_init)
 |   at the very beginning to initialze the list !!!
 */
{
  unsigned int i;

  (*stl) = (stl_t*) malloc (sizeof(stl_t));
  (**stl).size=0;
  (**stl).first=NULL;
  
  stl_reset_run(*stl);
 
  /* Initialize the list with "size" elements: */
  for(i=0; i<size; i++) stl_push_front(NULL, *stl);
}


void stl_init(unsigned int size, stl_t* stl)
/*
 |   Create a new list for a stl_t _variable_
 |   --> The anchor element already exists.
 |   "size" is the initial number of elements.
 |   
 |   You MUST call this function (or stl_create)
 |   at the very beginning to initialze the list !!!
 */
{
  unsigned int i;
  
  stl->size=0;
  stl->first=NULL;

  stl_reset_run(stl);
  
  /* Initialize the list with "size" elements: */
  for(i=0; i<size; i++) stl_push_front(NULL, stl);
}


void* stl_get(unsigned int pos, stl_t* stl)     
/*
 |   Return the pointer to the content of the
 |   element at position "pos":
 */
{  
  stl_c *run;
  unsigned int i;

  if(stl->size>0)
  {  
    /*
     |   Start at the cached position and run
     |   in the right direction:  
     */
    run=stl->cache_pt;
    if(pos>stl->cache_pos)
    {
      /* run forward through the list: */
      for(i=stl->cache_pos; i<pos; i++) run=run->forward;
      stl->cache_pt=run;
      stl->cache_pos=pos%stl->size; /* <-- if we reach the end, we just go on at the beginning */
      /* return the content-pointer of that element, where the run stops: */
      return run->container;
    }
    else
    {
      /* run backward through the list: */ 
      for(i=stl->cache_pos; i>pos; i--) run=run->backward; /* <-- "pos" is unsigend!  */
      stl->cache_pt=run;
      stl->cache_pos=pos;
      /* return the content-pointer where the run stops: */
      return run->container;
    }
  }
  return NULL;
}


void* stl_get_it(stl_c* iterator, stl_t* stl)
/*
 |   Return the pointer to the content of that element,
 |   the "iterator" points to:
 */
{  
  stl_c *run;
  unsigned int i=0;

  if(stl->size>0)
  {
    /* start at the cached pointer and run through the list: */
    for(run=stl->cache_pt; run->forward!=stl->cache_pt; run=run->forward)
    {
      i++;
      if(iterator==run)
      {
	/* 
	 |   if we have found the right element, we update
	 |   the cache-pointer and return the content-pointer:
    	 */
	stl->cache_pt=run;
	stl->cache_pos=(stl->cache_pos+i)%stl->size; /* <-- if we reach the end, we just go on at the beginning */
	return run->container;
      }
    }
    if(iterator==run)
    {
      stl->cache_pt=run;
      stl->cache_pos=(stl->cache_pos+i)%stl->size;
      return run->container;
    }
  }
  
  return NULL; /* <-- if the element could not be found */
}


void stl_set(void* container, unsigned int pos, stl_t* stl)
/*
 |   Update the content-pointer for the element at 
 |   position "pos":
 */
{  
  stl_c *run;
  unsigned int i;

  if(stl->size>0)
  {
    /*
     |   Start at the cached position and run
     |   in the right direction:  
     */
    run=stl->cache_pt;
    if(pos>stl->cache_pos)
    {
      /* run forward through the list: */
      for(i=stl->cache_pos; i<pos; i++) run=run->forward;
      /* update the cached pointer: */
      stl->cache_pt=run;
      stl->cache_pos=pos%stl->size; /* <-- if we reach the end, then we go on at the beginning */
      run->container=container;
    }
    else
    {
      /* run backward through the list: */
      for(i=stl->cache_pos; i>pos; i--) run=run->backward;
      stl->cache_pt=run;
      stl->cache_pos=pos;
      run->container=container;
    }
  }
}


void stl_set_it(void* container, stl_c* iterator, stl_t* stl)
/*
 |   Update the content-pointer for that element,
 |   the "iterator" points to:
 */
{  
  stl_c *run;
  unsigned int i=0;

  if(stl->size>0)
  {
    /* start at the cached pointer and run through the list: */
    for(run=stl->cache_pt; run->forward!=stl->cache_pt; run=run->forward)
    {
      i++;
      if(iterator==run)
      {
	/* 
	 |   if we have found the right element, we update content-pointer
	 |   and the cache-pointer:
    	 */	
	stl->cache_pt=run;
	stl->cache_pos=(stl->cache_pos+i)%stl->size;
	run->container=container;  
      }
    }
    if(iterator==run)
    {
      stl->cache_pt=run;
      stl->cache_pos=(stl->cache_pos+i)%stl->size; 
      run->container=container;        
    }
  }
}


stl_c* stl_iterator_pos(unsigned int pos, stl_t* stl)
/*
 |   Return a pointer (iterator) to the element at
 |   the position "pos":
 */
{  
  stl_c *run;
  unsigned int i;

  if(stl->size>0)
  {
    /* is this position cached? */
    if(stl->cache_pos==pos) return stl->cache_pt;

    /* run through the list to the right position: */
    run=stl->first;
    for(i=0; i<pos; i++) run=run->forward;

    /* update the cache-pointer: */
    stl->cache_pt=run;
    stl->cache_pos=(stl->cache_pos+i)%stl->size; 

    return run;    
  }
  else return NULL; /* <-- if the list is empty */
}


stl_c* stl_iterator_begin(stl_t* stl)
/*
 |   Return a pointer (iterator) to the element at
 |   the first position of the list:
 */
{  
  if(stl->size>0)
  {
    return stl->first;
  }
  else return NULL;
}


stl_c* stl_iterator_end(stl_t* stl)
/*
 |   Return a pointer (iterator) to the element at
 |   the last position of the list:
 */
{  
  if(stl->size>0)
  {
    return stl->first->backward;
  }
  else return NULL;
}


stl_c* stl_iterator_inc(stl_c* iterator, stl_t* stl)
/*
 |   Return an increased iterator of the list:
 |
 */
{
  if(stl->size>0)
  {
    return iterator->forward;
  }
  else return NULL;
}


stl_c* stl_iterator_dec(stl_c* iterator, stl_t* stl)
/*
 |   Return an decreased iterator of the list:
 |
 */
{
  if(stl->size>0)
  {
    return iterator->backward;
  }
  else return NULL;
}


stl_c* stl_insert(unsigned int pos, int before, void* container, stl_t* stl)
/*
 |   Insert a new element "before" the position "pos".
 |   So, if "before" is true, the element is inserted _before_ the element at
 |   position "pos", else it is inserted after that element.
 |   Return value is a pointer to the new element.
 */   
{
  unsigned int i;
  stl_c *run1, *run2;

  /* create the new element and store the pointer to the content: */
  run1=(stl_c*)malloc(sizeof(stl_c));
  run1->container=container;
 
  if(stl->size==0)  /* <-- is the list still empty ? */
  {
    run1->forward=run1;
    run1->backward=run1;
    stl->first=run1;
  }
  else
  {
    /* start at the first element and run to the right position: */
    run2=stl->first; /* XXX (we probably would better start at the cached position !!!) */
    for(i=0; i<pos; i++) run2=run2->forward;

    if(before)
    {
      /* bend the respective pointers to insert the element before "run2": */
      run1->forward=run2;
      run1->backward=run2->backward;  
      run2->backward->forward=run1;
      run2->backward=run1;
      if(run2==stl->first) stl->first=run1; /* <-- if the new element becomes the first one in the list */
    }
    else
    {
      /* bend the respective pointers to insert the element after "run2": */
      run1->backward=run2;
      run1->forward=run2->forward;  
      run2->forward->backward=run1;
      run2->forward=run1;
    }
  }
  stl->size++;

  stl_reset_run(stl); /* <-- reset the cached pointer */

  return run1;
}

stl_c* stl_insert_it(stl_c* iterator, int before, void* container, stl_t* stl)
/*
 |   Insert a new element "before" the element, the "iterator" points to.
 |   So, if "before" is true, the element is inserted _before_ the element at
 |   position "pos", else it is inserted after that element.
 |   Return value is a pointer to the new element.
 */   
{
  stl_c *run1, *run2;
  
  /* create the new element and store the pointer to the content: */ 
  run1=(stl_c*)malloc(sizeof(stl_c));
  run1->container=container;

  if(stl->size==0) /* <-- is the list still empty ? */
  {
    run1->forward=run1;
    run1->backward=run1;
    stl->first=run1; 
    stl->size++;
  }
  else
  {
    /* start at the first element and run to the right position: */
    for(run2=stl->first; (run2->forward!=stl->first)&&(iterator!=run2); run2=run2->forward);
    /* XXX (we probably would better start at the cached position !!!) */

    if((iterator!=run2)&&(before)) run2=stl->first; /* <-- insert at the first position, if the iterator could not be found*/
    if(before)
    {
      /* bend the respective pointers to insert the element before "run2": */
      run1->forward=run2;
      run1->backward=run2->backward;
      run2->backward->forward=run1;
      run2->backward=run1;	
      if(run2==stl->first) stl->first=run1;
    }
    else
    {  
      /* bend the respective pointers to insert the element after "run2": */
      run1->backward=run2;
      run1->forward=run2->forward;  
      run2->forward->backward=run1;
      run2->forward=run1;
    }
    stl->size++;
  }

  stl_reset_run(stl); /* <-- reset the cached pointer */

  return run1;
}


void* stl_erase(unsigned int pos, stl_t* stl)
/*
 |   Erase the element at position "pos".
 |   Remember, that only the list-element is erased!
 |   The content still exists and can be reached by the returned pointer.
 */   
{
  stl_c *run;
  void* cont;
  unsigned int i;
 
  if(stl->size>0)
  {
    /* start at the first element and run to the right position: */
    run=stl->first;
    for(i=0; i<pos; i++) run=run->forward;
    if(run==stl->first) stl->first=run->forward;

    /* bend the respective pointers to close the chain again: */
    run->backward->forward=run->forward;
    run->forward->backward=run->backward;
    cont=run->container;
    free(run); /* <-- free the list element */
    stl->size--;
    if(stl->size==0) stl->first=NULL; /* <-- the list is empty again */
  }

  stl_reset_run(stl); /* <-- reset the cached pointer */

  return cont;
}

void* stl_erase_it(stl_c* iterator, stl_t* stl)
/*
 |   Erase the element the iterator points to.
 |   If the iterator points to no legal element, nothing is done.
 |   Remember, that only the list-element is erased!
 |   The content still exists and can be reached by the returned pointer.
 */   
{
  stl_c *run;
  void* cont;
  
  if(stl->size>0)
  { 
    /* start at the first element and run to the right position: */
    for(run=stl->first; (run->forward!=stl->first)&&(iterator!=run); run=run->forward);
    if(run==stl->first) stl->first=run->forward;

    if(iterator==run) /* <-- if the iterator could be found */
    {
      /* bend the respective pointers to close the chain again: */
      run->backward->forward=run->forward;
      run->forward->backward=run->backward;
      cont=run->container;
      free(run); /* <-- free the list element */
      stl->size--;
      if(stl->size==0) stl->first=NULL; /* <-- the list is empty again */
    }
  }

  stl_reset_run(stl);

  return cont;
}

void stl_delete(stl_t* stl)
/*
 |   Delete the whole list:
 */
{
  while(stl->size!=0) stl_erase(0,stl);
}

void stl_destruct(stl_t** stl)
/*
 |   Delete the whole list an free the anchor:
 */
{
  while((**stl).size!=0) stl_erase(0,*stl); 
  free(stl);
}

unsigned int stl_size(stl_t* stl)
/*
 |   Return the number of elements in the list:
 */
{
  return stl->size;
}

int stl_empty(stl_t* stl)
/*
 |   Return 1, if the list is empty:
 */
{
  if(stl->size==0) return 1;
  else return 0;
}
