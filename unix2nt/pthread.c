#include "pthread.h"

#include <winbase.h>

int pthread_create(pthread_t *new_thread_ID,
                   pthread_attr_t *pthread_attr,
                   void * (*start_func)(void *),
		   void *arg)
{
    LPTHREAD_START_ROUTINE lpStartAddress;
    LPVOID lpParameter;
    DWORD dwStackSize,threadId;
    
    
    lpStartAddress = (LPTHREAD_START_ROUTINE) start_func;
    lpParameter = (LPVOID) arg;
    dwStackSize = pthread_attr->stacksize;
    
    *new_thread_ID = CreateThread (0,
	dwStackSize,
	lpStartAddress,
	lpParameter,
	0,
	&threadId);
    
    if(!*new_thread_ID) return GetLastError();
    
    /*
    if(pthread_attr->detachstate != PTHREAD_CREATE_JOINABLE) {
    CloseHandle(*new_thread_ID);
    }
    */
    
    return 0;
};
	

int pthread_kill(pthread_t target_thread, int sig)
{
	if(sig == 9)
		return !TerminateThread (target_thread, sig);
	 else return ERROR_CALL_NOT_IMPLEMENTED;
};


int	pthread_join(pthread_t target_thread, void **status)
{
	int res = 0;

	if (!target_thread)
		return ESRCH;

	if (WaitForSingleObject (target_thread, INFINITE) == WAIT_FAILED)
		res = ESRCH;
	
	CloseHandle(target_thread);
	return res;
};


int pthread_mutex_lock (pthread_mutex_t *mp)
{
	EnterCriticalSection (mp);
	return 0;
};


int pthread_mutex_unlock (pthread_mutex_t *mp)
{
	LeaveCriticalSection (mp);
	return 0;
};


int pthread_mutex_trylock (pthread_mutex_t *mp)
{

	if (!TryEnterCriticalSection(mp))
		return EBUSY;
	else
		return 0;
};


int pthread_mutex_init (pthread_mutex_t *mp, pthread_mutexattr_t *attr)
{
	InitializeCriticalSection (mp);
	return 0;
};


int pthread_mutex_destroy(pthread_mutex_t *mp)
{
	DeleteCriticalSection (mp);
	return 0;
};


int pthread_mutexattr_init (pthread_mutexattr_t *mutex_attr)
{
	return 0;
};


int pthread_mutexattr_destroy (pthread_mutexattr_t *attr)
{
	return 0;
};


int pthread_mutexattr_setpshared (pthread_mutexattr_t *attr, int process_shared)
{
	return 0;
};


int pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr, int *process_shared)
{
	return 0;
};


int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	int RetVal;

	if (!cond || !mutex)
		return EFAULT;

	cond->Sleeping++;
	do{
		LeaveCriticalSection (mutex);
		RetVal = WaitForSingleObject (cond->Event, INFINITE);
		EnterCriticalSection (mutex);
	}while((WaitForSingleObject(cond->Event, 0)) != WAIT_OBJECT_0);
	ResetEvent (cond->Event);
	cond->Sleeping--;
	if ((RetVal == WAIT_ABANDONED || RetVal == WAIT_OBJECT_0))
		return 0;
	else
		return 1;
};


int pthread_cond_timedwait(pthread_cond_t *cond,
						   pthread_mutex_t *mutex,
						   const struct timespec *abstime)
{
	DWORD RetVal;
	int Timeout;

	if (!abstime || !cond || !mutex)
		return EFAULT;

	Timeout = abstime->tv_sec * 1000;

	cond->Sleeping++;

	do{
		LeaveCriticalSection (mutex);
		RetVal = WaitForSingleObject (cond->Event, Timeout);
		EnterCriticalSection (mutex);
	}while((WaitForSingleObject(cond->Event, 0)) != WAIT_OBJECT_0 && RetVal != WAIT_TIMEOUT);
	ResetEvent (cond->Event);
	cond->Sleeping--;
	if ((RetVal == WAIT_ABANDONED || RetVal == WAIT_OBJECT_0))
		return 0;
	if (RetVal == WAIT_TIMEOUT)
		return ETIMEDOUT;
	else
		return 1;
};


int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr)
{
	cond->Event = CreateEvent (0, TRUE, 0, 0);
	cond->Sleeping = 0;
	if (!cond->Event)
		return 1;
	else
		return 0;
};


int pthread_cond_signal(pthread_cond_t *cond)
{
	if (cond->Sleeping)
		return (int)!SetEvent(cond->Event);
	else
		return 1;
};


int pthread_cond_destroy(pthread_cond_t *cond)
{
	return (int)!CloseHandle (cond->Event);
};


int pthread_condattr_init (pthread_condattr_t *cond_attr)
{
	return 0;
};


int pthread_condattr_setpshared (pthread_condattr_t *attr, int process_shared)
{
	return 0;
};


int pthread_condattr_getpshared (const pthread_condattr_t *attr, int *process_shared)
{
	return 0;
};


int pthread_condattr_destroy(pthread_condattr_t *attr)
{
	return 0;
};


int pthread_attr_init (pthread_attr_t *attr)
{
	memset(attr,0,sizeof(pthread_attr_t));
	return 0;
};


int pthread_attr_setscope (pthread_attr_t *attr, int contentionscope)
{
	
	return 0;
};

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	attr->detachstate = detachstate;
	return 0;
}

pthread_t pthread_self ()
{
	return GetCurrentThread();
};

void pthread_exit(void *retval) {
    ExitThread((DWORD)retval);
}
