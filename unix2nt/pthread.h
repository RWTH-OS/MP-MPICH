#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <wtypes.h>
#include <errno.h>

#ifndef __PTHREAD_H__
#define __PTHREAD_H__

/*
 * macros - default initializers defined as in synch.h
 * Any change here should be reflected in synch.h.
 */
#define PTHREAD_MUTEX_INITIALIZER       {0, 0, 0}       /* = DEFAULTMUTEX */
#define PTHREAD_COND_INITIALIZER        {0, 0}          /* = DEFAULTCV */

#define pthread_t HANDLE
#define pthread_mutex_t CRITICAL_SECTION
#define pthread_mutexattr_t int

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1
#define PTHREAD_SCOPE_SYSTEM 0

#ifndef EBUSY
	#define EBUSY 1
#endif

#define ETIMEDOUT WAIT_TIMEOUT


typedef struct _pthread_attr_t
{
	DWORD stacksize;
	DWORD detachstate;
}pthread_attr_t;

struct timespec
{
	int tv_sec, tv_nsec;
};

typedef struct _pthread_cond_t
{
	HANDLE Event;
	int Sleeping;
}pthread_cond_t;

/*typedef struct _pthread_mutexattr_t
{
	int process_shared;
}pthread_mutexattr_t;*/

typedef struct _pthread_condattr_t
{
	int process_shared;
}pthread_condattr_t;


int pthread_create(pthread_t *new_thread_ID,
                   pthread_attr_t *pthread_attr,
                   void * (*start_func)(void *),
				   void *arg);

void pthread_exit(void *retval);

int pthread_kill(pthread_t target_thread, int sig);

int pthread_join(pthread_t target_thread, void **status);

int pthread_mutex_lock (pthread_mutex_t *mp);

int pthread_mutex_unlock (pthread_mutex_t *mp);

int pthread_mutex_trylock (pthread_mutex_t *mp);

int pthread_mutex_init (pthread_mutex_t *mp, pthread_mutexattr_t *attr);

int pthread_mutex_destroy(pthread_mutex_t *mp);

int pthread_mutexattr_init (pthread_mutexattr_t *mutex_attr);

int pthread_mutexattr_destroy (pthread_mutexattr_t *attr);

int pthread_mutexattr_setpshared (pthread_mutexattr_t *attr, int process_shared);

int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *process_shared);


int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr);

int pthread_cond_timedwait(pthread_cond_t *cond,
						   pthread_mutex_t *mutex,
						   const struct timespec *abstime);

int pthread_cond_signal(pthread_cond_t *cond);
#define pthread_cond_broadcast pthread_cond_signal
int pthread_cond_destroy(pthread_cond_t *cond);

int pthread_condattr_init (pthread_condattr_t *cond_attr);

int pthread_condattr_setpshared (pthread_condattr_t *attr, int process_shared);

int pthread_condattr_getpshared (const pthread_condattr_t *attr, int *process_shared);

int pthread_condattr_destroy(pthread_condattr_t *attr);

int pthread_attr_init (pthread_attr_t *attr);

int pthread_attr_setscope (pthread_attr_t *attr, int contentionscope);

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

#endif
