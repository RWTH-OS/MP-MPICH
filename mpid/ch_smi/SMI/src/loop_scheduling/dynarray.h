/* $Id$ */

#ifndef __DYNARRAY_H
#define __DYNARRAY_H

#include "err.h"
#include "env/general_definitions.h"

//------------------------------------------------------------------------
////////////////////
// dynArray class //
////////////////////
//------------------------------------------------------------------------
template <class TYPE>
class dynArray
{
private:
   TYPE*	elem;
   int	size;
public:
   // constructor
   inline dynArray():elem(NULL),size(0) {}
   inline ~dynArray() {delete []elem;}
   int operator++(); //throw(err);
   TYPE& operator[](const int i); //throw(err);
};

//------------------------------------------------------------------------
//////////////////////////////////
// member functions of dynArray //
////////////////////////////////// 
//------------------------------------------------------------------------
//
// Enlarges the array by one element and returns the id of the last 
// new element
template <class TYPE>
int dynArray<TYPE>::operator++()// throw(err)
{
   TYPE*	temp;
   
   temp=elem;
   elem=new TYPE[size+1];
   if(elem==NULL) 
      throw err("Memory allocation failed in dynArray",__LINE__,SMI_ERR_NOMEM);
   size++;
   for(int i=0;i<size-1;i++)
      elem[i]=temp[i];
   delete []temp; 
   
   return(size-1);
}
//------------------------------------------------------------------------
//
// operator to refer to elements of the dynArray
//
template <class TYPE>
TYPE& dynArray<TYPE>::operator[](const int i) //throw(err)
{
   if(i>size-1)
      throw err("Element not in Array (id > size) in dynArray",__LINE__,SMI_ERR_PARAM);
   return(elem[i]);
}

#endif
