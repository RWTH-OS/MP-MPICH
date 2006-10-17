#ifndef _STL2C_DEQUE_H
#define _STL2C_DEQUE_H

   /*********************************
--- ** STL - list                  ** -------------------------------------------------------
    *********************************/

#include "stl2c_basic.h"
#include "stl2c_bool.h"

#ifndef STL_size_type
#define STL_size_type unsigned int
#endif

#ifndef STL_deque_type
#define STL_deque_type stl_t
#endif

#ifndef STL_iterator_type
#define STL_iterator_type stl_c*
#endif

#define STL_deque_constructor STL_list_constructor 

#define STL_deque_destructor STL_list_destructor

#define STL_deque_size STL_list_size

#define STL_deque_empty STL_list_empty

#define STL_deque_pop_back STL_list_pop_back

#define STL_deque_pop_front STL_list_pop_front

#define STL_deque_front STL_list_front

#define STL_deque_back STL_list_back

#define STL_deque_begin STL_list_begin

#define STL_deque_end STL_list_end

#define STL_deque_iterator_inc STL_list_iterator_inc

#define STL_deque_iterator_dec STL_list_iterator_dec

#define STL_deque_erase STL_list_erase

#include "stl2c_list.h"


   /*********************************
--- ** STL - deque templates        ** -------------------------------------------------------
    *********************************/

#define STL_deque_template(name, type) static void STL_deque_push_back_ ## name (type cont, STL_deque_type* deque) \
                                       { \
                                         void* pt; \
                                         pt=(void*)malloc(sizeof(type)); \
                                         memcpy(pt, &cont, sizeof(type)); \
                                         stl_push_back(pt, deque); \
                                        } \
                                        static void STL_deque_push_front_ ## name (type cont, STL_deque_type* deque) \
                                        { \
                                          void* pt; \
                                          pt=(void*)malloc(sizeof(type)); \
                                          memcpy(pt, &cont, sizeof(type)); \
                                          stl_push_front(pt, deque); \
                                        } \
                                        static STL_iterator_type STL_deque_insert_ ## name (STL_iterator_type it, type cont, STL_deque_type* deque) \
                                        { \
                                          void* pt; \
                                          pt=(void*)malloc(sizeof(type)); \
                                          memcpy(pt, &cont, sizeof(type)); \
                                          if(it==NULL) \
                                          { \
                                            return (STL_iterator_type)stl_insert_it(stl_iterator_end(deque), 0, pt, deque); \
                                          } \
                                          else \
                                          { \
                                            return (STL_iterator_type)stl_insert_it(it, 1, pt, deque); \
                                          } \
                                        } \
                                        static type STL_deque_get_ ## name (STL_iterator_type it, STL_deque_type* deque) \
                                        { \
                                          return *((type*)stl_get_it(it, deque)); \
                                        } \
                                        static type STL_deque_front_ ## name (STL_deque_type* deque) \
                                        { \
                                          return *((type*)stl_get_it(stl_iterator_begin(deque), deque)); \
                                        } \
                                        static type STL_deque_back_ ## name (STL_deque_type* deque) \
                                        { \
                                          return *((type*)stl_get_it(stl_iterator_end(deque), deque)); \
                                        }

#endif
