/* $id$ */

#ifndef __DEBUG_H__
#define __DEBUG_H__

// Some useful debugging macros used throughout the whole system

#ifdef _DEBUG
#include <wtypes.h>
#include <winbase.h>
#include <iostream>

//extern CRITICAL_SECTION _DBGCS;
 
#include <assert.h>

//#define DBG(m) {EnterCriticalSection(&_DBGCS); cerr<<m<<endl; LeaveCriticalSection(&_DBGCS);}
#define DBG(m) { cerr<<m<<endl;}

#define ASSERT(expr) assert(expr);

#else

#define DBG(m) {}

#define ASSERT(expr)


#endif //_DEBUG

#if defined _USED_BY_SMI_ 

#define	SVM_ERROR(m) std::cerr<<"\nSVM_Error: In file "<<__FILE__<<" line "<<__LINE__<<endl<<m<<endl;Sleep(8000)

#else

#define	SVM_ERROR(m) std::cerr<<"\nSVM_Error: In file "<<__FILE__<<" line "<<__LINE__<<endl<<m<<endl

#endif

#endif // __DEBUG_H__	

			  
