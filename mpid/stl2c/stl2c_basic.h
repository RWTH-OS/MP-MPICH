#ifndef _STL2C_BASIC_H
#define _STL2C_BASIC_H

   /*********************************
--- ** STL - basic functions       ** -------------------------------------------------------
    *********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _stl_c
{
  struct _stl_c * forward;
  struct _stl_c * backward;
  void*  container;

} stl_c;

typedef struct _stl_t
{
  unsigned int size;
  unsigned int cache_pos;
  stl_c* first;
  stl_c* cache_pt;

} stl_t;

stl_c* stl_push_front(void* container, stl_t* stl);

stl_c* stl_push_back(void* container, stl_t* stl);

void* stl_pop_front(stl_t* stl);

void* stl_pop_back(stl_t* stl);

void stl_create(unsigned int size, stl_t** stl);

void stl_init(unsigned int size, stl_t* stl);

void* stl_get(unsigned int pos, stl_t* stl);

void* stl_get_it(stl_c* iterator, stl_t* stl);

void stl_set(void* container, unsigned int pos, stl_t* stl);

void stl_set_it(void* container, stl_c* iterator, stl_t* stl);

stl_c* stl_iterator_pos(unsigned int pos, stl_t* stl);

stl_c* stl_iterator_begin(stl_t* stl);

stl_c* stl_iterator_end(stl_t* stl);

stl_c* stl_iterator_inc(stl_c* iterator, stl_t* stl);

stl_c* stl_iterator_dec(stl_c* iterator, stl_t* stl);

stl_c* stl_insert(unsigned int pos, int before, void* container, stl_t* stl);

stl_c* stl_insert_it(stl_c* iterator, int before, void* container, stl_t* stl);

void* stl_erase(unsigned int pos, stl_t* stl);

void* stl_erase_it(stl_c* iterator, stl_t* stl);

void stl_delete(stl_t* stl);

void stl_destruct(stl_t** stl);

unsigned int stl_size(stl_t* stl);

int stl_empty(stl_t* stl);

#endif
