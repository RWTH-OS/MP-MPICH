
#ifndef __SCHED_H__
#define __SCHED_H__
#ifdef _WIN32
#define sched_yield() Sleep(0)
#endif
#endif
