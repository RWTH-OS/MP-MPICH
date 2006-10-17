
   /*********************************
--- ** STL - list                  ** -------------------------------------------------------
    *********************************/

#include "stl2c_basic.h"
#include "stl2c_list.h"


void STL_list_constructor(STL_list_type* list)
{
  stl_init(0, list);
}

void STL_list_destructor(STL_list_type* list)
{
  while(stl_size(list)>0) free(stl_erase(0, list));
}

STL_size_type STL_list_size(STL_list_type *list)
{
  return (STL_size_type)stl_size(list);
}

bool STL_list_empty(STL_list_type* list)
{
  return (bool)stl_empty(list);
}

void STL_list_pop_front(STL_list_type* list)
{
  void* pt;

  pt=stl_pop_front(list);

  free(pt);
}

void STL_list_pop_back(STL_list_type* list)
{
  void* pt;

  pt=stl_pop_back(list);

  free(pt);
}

STL_iterator_type STL_list_begin(STL_list_type* list)
{
  return (STL_iterator_type)stl_iterator_begin(list);
}

STL_iterator_type STL_list_end(STL_list_type* list)
{
  return NULL;
}

STL_iterator_type STL_list_iterator_inc(STL_iterator_type it, STL_list_type* list)
{
  if(it==(STL_iterator_type)stl_iterator_end(list))
  {
    return NULL;
  }
  else
  {
    return (STL_iterator_type)stl_iterator_inc(it, list);
  }
}

STL_iterator_type STL_list_iterator_dec(STL_iterator_type it, STL_list_type* list)
{
  if(it==NULL)
  {
    return (STL_iterator_type)stl_iterator_end(list);
  }
  else
  {
    return (STL_iterator_type)stl_iterator_dec(it, list);
  }
}

void STL_list_erase(STL_iterator_type it, STL_list_type* list)
{
  void* pt;

  pt=stl_erase_it(it, list);

  free(pt);
}
