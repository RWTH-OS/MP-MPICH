
   /*********************************
--- ** STL - map                  ** -------------------------------------------------------
    *********************************/

#include "stl2c_basic.h"
#include "stl2c_map.h"

#include <stdio.h>

void STL_map_constructor(STL_map_type* map)
{
  stl_init(0, map);
}

void STL_map_destructor(STL_map_type* map)
{
  stl_map* pt;

  while(stl_size(map)>0)
  {
    pt=(stl_map*)stl_erase(0, map);
    free(pt->key);
    free(pt->cont);
    free(pt);    
  }
}

STL_size_type STL_map_size(STL_map_type *map)
{
  return (STL_size_type)stl_size(map);
}

bool STL_map_empty(STL_map_type* map)
{
  return (bool)stl_empty(map);
}

STL_iterator_type STL_map_begin(STL_map_type* map)
{
  return (STL_iterator_type)stl_iterator_begin(map);
}

STL_iterator_type STL_map_end(STL_map_type* map)
{
  return NULL;
}

STL_iterator_type STL_map_iterator_inc(STL_iterator_type it, STL_map_type* map)
{
  if(it==(STL_iterator_type)stl_iterator_end(map))
  {
    return NULL;
  }
  else
  {
    return (STL_iterator_type)stl_iterator_inc(it, map);
  }
}

STL_iterator_type STL_map_iterator_dec(STL_iterator_type it, STL_map_type* map)
{
  if(it==NULL)
  {
    return (STL_iterator_type)stl_iterator_end(map);
  }
  else
  {
    return (STL_iterator_type)stl_iterator_dec(it, map);
  }
}

void STL_map_erase(STL_iterator_type it, STL_map_type* map)
{
  stl_map* pt;

  if(it!=NULL)
  {
    pt=stl_erase_it(it, map);
    free(pt->key);
    free(pt->cont);
    free(pt);
  }
}
