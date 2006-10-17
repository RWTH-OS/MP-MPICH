#ifndef _STL2C_LIST_H
#define _STL2C_LIST_H

   /*********************************
--- ** STL - list                  ** -------------------------------------------------------
    *********************************/

#include "stl2c_basic.h"
#include "stl2c_bool.h"

#ifndef STL_size_type
#define STL_size_type unsigned int
#endif

#ifndef STL_list_type
#define STL_list_type stl_t
#endif

#ifndef STL_iterator_type
#define STL_iterator_type stl_c*
#endif

void STL_list_constructor(STL_list_type* list);

void STL_list_destructor(STL_list_type* list);

STL_size_type STL_list_size(STL_list_type *list);

bool STL_list_empty(STL_list_type* list);

void STL_list_pop_back(STL_list_type* list);

void STL_list_pop_front(STL_list_type* list);

STL_iterator_type STL_list_front(STL_list_type* list);

STL_iterator_type STL_list_back(STL_list_type* list);

STL_iterator_type STL_list_begin(STL_list_type* list);

STL_iterator_type STL_list_end(STL_list_type* list);

STL_iterator_type STL_list_iterator_inc(STL_iterator_type it, STL_list_type* list);

STL_iterator_type STL_list_iterator_dec(STL_iterator_type it, STL_list_type* list);

void STL_list_erase(STL_iterator_type it, STL_list_type* list);


   /*********************************
--- ** STL - list templates        ** -------------------------------------------------------
    *********************************/

#define STL_list_template(name, type) static void STL_list_push_back_ ## name (type cont, STL_list_type* list) \
                                       { \
                                         void* pt; \
                                         pt=(void*)malloc(sizeof(type)); \
                                         memcpy(pt, &cont, sizeof(type)); \
                                         stl_push_back(pt, list); \
                                        } \
                                        static void STL_list_push_front_ ## name (type cont, STL_list_type* list) \
                                        { \
                                          void* pt; \
                                          pt=(void*)malloc(sizeof(type)); \
                                          memcpy(pt, &cont, sizeof(type)); \
                                          stl_push_front(pt, list); \
                                        } \
                                        static STL_iterator_type STL_list_insert_ ## name (STL_iterator_type it, type cont, STL_list_type* list) \
                                        { \
                                          void* pt; \
                                          pt=(void*)malloc(sizeof(type)); \
                                          memcpy(pt, &cont, sizeof(type)); \
                                          if(it==NULL) \
                                          { \
                                            return (STL_iterator_type)stl_insert_it(stl_iterator_end(list), 0, pt, list); \
                                          } \
                                          else \
                                          { \
                                            return (STL_iterator_type)stl_insert_it(it, 1, pt, list); \
                                          } \
                                        } \
                                        static type STL_list_get_ ## name (STL_iterator_type it, STL_list_type* list) \
                                        { \
                                          return *((type*)stl_get_it(it, list)); \
                                        } \
                                        static type STL_list_front_ ## name (STL_list_type* list) \
                                        { \
                                          return *((type*)stl_get_it(stl_iterator_begin(list), list)); \
                                        } \
                                        static type STL_list_back_ ## name (STL_list_type* list) \
                                        { \
                                          return *((type*)stl_get_it(stl_iterator_end(list), list)); \
                                        }

#endif
