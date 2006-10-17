#ifndef _STL2C_MAP_H
#define _STL2C_MAP_H

   /*********************************
--- ** STL - map                  ** -------------------------------------------------------
    *********************************/

#include "stl2c_basic.h"
#include "stl2c_bool.h"

#ifndef STL_size_type
#define STL_size_type unsigned int
#endif

#ifndef STL_map_type
#define STL_map_type stl_t
#endif

#ifndef STL_iterator_type
#define STL_iterator_type stl_c*
#endif

typedef struct _stl_map
{
  void* key;
  void* cont;

} stl_map;


void STL_map_constructor(STL_map_type* map);

void STL_map_destructor(STL_map_type* map);

STL_size_type STL_map_size(STL_map_type *map);

bool STL_map_empty(STL_map_type* map);

STL_iterator_type STL_map_front(STL_map_type* map);

STL_iterator_type STL_map_back(STL_map_type* map);

STL_iterator_type STL_map_begin(STL_map_type* map);

STL_iterator_type STL_map_end(STL_map_type* map);

STL_iterator_type STL_map_iterator_inc(STL_iterator_type it, STL_map_type* map);

STL_iterator_type STL_map_iterator_dec(STL_iterator_type it, STL_map_type* map);

void STL_map_erase(STL_iterator_type it, STL_map_type* map);


   /*********************************
--- ** STL - map templates        ** -------------------------------------------------------
    *********************************/

#define STL_map_template(name, key_type, cont_type) static STL_iterator_type STL_map_insert_ ## name (key_type key, cont_type cont, STL_map_type* map) \
                                                    { \
                                                      STL_size_type i; \
                                                      stl_map* pt; \
                                                      pt=(stl_map*)malloc(sizeof(stl_map)); \
						      pt->key=(void*)malloc(sizeof(key_type)); \
						      memcpy(pt->key, &key, sizeof(key_type)); \
						      pt->cont=(void*)malloc(sizeof(cont_type)); \
						      memcpy(pt->cont, &cont, sizeof(cont_type)); \
						      for(i=0; i<stl_size(map); i++) \
						      { \
							if(memcmp(((stl_map*)stl_get(i, map))->key, &key, sizeof(key_type))==0) break; \
						      } \
						      return stl_insert(i, 1, (void*)pt, map); \
						    } \
                                                    static STL_iterator_type STL_map_find_ ## name (key_type key, STL_map_type* map) \
                                                    { \
					              STL_size_type i; \
						      for(i=0; i<stl_size(map); i++) \
						      { \
							if(memcmp(((stl_map*)stl_get(i, map))->key, &key, sizeof(key_type))==0) \
							{ \
							  return stl_iterator_pos(i, map); \
							} \
						      } \
						      return NULL; \
						    } \
                                                    static STL_size_type STL_map_count_ ## name (key_type key, STL_map_type* map) \
                                                    { \
                                                      STL_size_type i, count=0; \
						      for(i=0; i<stl_size(map); i++) \
						      { \
							if(memcmp(((stl_map*)stl_get(i, map))->key, &key, sizeof(key_type))==0) count++; \
						      } \
						      return count; \
						    } \
                                                    static key_type STL_map_first_ ## name (STL_iterator_type it, STL_map_type* map) \
                                                    { \
						      return *((key_type*)((stl_map*)stl_get_it(it, map))->key); \
						    } \
                                                    static cont_type STL_map_second_ ## name (STL_iterator_type it, STL_map_type* map) \
                                                    { \
						      return *((cont_type*)((stl_map*)stl_get_it(it, map))->cont); \
						    } \
                                                    static cont_type STL_map_get_ ## name (key_type key, STL_map_type* map) \
                                                    { \
						      return *((cont_type*)((stl_map*)stl_get_it(STL_map_find_ ## name (key, map), map))->cont); \
						    }
#endif
