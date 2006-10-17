#ifndef _STL2C_VECTOR_H
#define _STL2C_VECTOR_H

   /*********************************
--- ** STL - vector                ** -------------------------------------------------------
    *********************************/

#include "stl2c_basic.h"
#include "stl2c_bool.h"

#ifndef STL_size_type
#define STL_size_type unsigned int
#endif

#ifndef STL_vector_type
#define STL_vector_type stl_t
#endif

void STL_vector_constructor(unsigned int size, STL_vector_type* vector);

void STL_vector_destructor(STL_vector_type* vector);

STL_size_type STL_vector_size(STL_vector_type *vector);

bool STL_vector_empty(STL_vector_type* vector);

void STL_vector_pop_back(STL_vector_type* vector);


   /*********************************
--- ** STL - vector templates      ** -------------------------------------------------------
    *********************************/

#define STL_vector_template(name, type) static void STL_vector_push_back_ ## name (type cont, STL_vector_type* vector) \
                                        { \
                                          void* pt; \
                                          pt=(void*)malloc(sizeof(type)); \
                                          memcpy(pt, &cont, sizeof(type)); \
                                          stl_push_back(pt, vector); \
                                        } \
                                        static void STL_vector_set_ ## name (unsigned int index, type cont, STL_vector_type* vector) \
                                        { \
                                          void*  pt; \
                                          pt=(void*)malloc(sizeof(type)); \
                                          memcpy(pt, &cont, sizeof(type)); \
                                          if(stl_get(index, vector)!=NULL) free(pt); \
                                          stl_set(pt, index, vector); \
                                        } \
                                        static type STL_vector_get_ ## name (unsigned int index, STL_vector_type* vector) \
                                        { \
                                          return *((type*)stl_get(index, vector)); \
                                        }

#endif
