
#ifndef DEBUGNT2U_HEADER
#define DEBUGNT2U_HEADER
#include <threads.h>
#include <iostream>

#ifdef _DEBUG
//extern CRITICAL_SECTION _DBGCS;
#define DBG(m) { std::cerr<<"[nt2unix:"<<thr_self()<<": "<<m<<"]"<<std::endl<<std::flush; }
#else
#define DBG(m) {}
#endif

// #define _DEBUGSYNC

#ifdef _DEBUGSYNC
#define DBGS(m) { std::cerr<<"[nt2unix:"<<thr_self()<<": "<<m<<"]"<<std::endl<<std::flush; }
#else
#define DBGS(m) {}
#endif
#endif //DEBUGNT2U_HEADER

