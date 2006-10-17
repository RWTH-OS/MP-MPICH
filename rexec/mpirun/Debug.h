#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef _DEBUG
#include <iostream>
#include <assert.h>

#define DBG(m) cerr<<GetLastError()<<": "<<m<<endl
#define ASSERT(expr) assert(expr);

#else

#define DBG(m)
#define ASSERT(expr)

#endif //_DEBUG

#endif // __DEBUG_H__