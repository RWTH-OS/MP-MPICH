
   /*********************************
--- ** STL - vector                ** -------------------------------------------------------
    *********************************/

#include "stl2c_basic.h"
#include "stl2c_vector.h"

void STL_vector_constructor(unsigned int size, STL_vector_type* vector)
{
  stl_init(size, vector);
}

void STL_vector_destructor(STL_vector_type* vector)
{
  while(stl_size(vector)>0) free(stl_erase(0, vector));
}

STL_size_type STL_vector_size(STL_vector_type *vector)
{
  return (STL_size_type)stl_size(vector);
}

bool STL_vector_empty(STL_vector_type* vector)
{
  return (bool)stl_empty(vector);
}

void STL_vector_pop_back(STL_vector_type* vector)
{
  void* pt;

  pt=stl_pop_back(vector);

  free(pt);
}
