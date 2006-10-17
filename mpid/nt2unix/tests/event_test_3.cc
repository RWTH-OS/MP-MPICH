#include "nt2unix.h"
#include <iostream.h>
#include <threadsync.h>
#include <threads.h>
#include <errno.h>
#include <string.h>

  int nCount = 1;
  cond_t condition;
  mutex_t mutex;

/*
unsigned long EventSignaler(void*)
{
  cout << "Signaler : SLEEPING\n";
  Sleep (2000);
  
  cout << "Setting..." << endl;
  if (!SetEvent (Event))
    cout << "SetEvent FAILED !" << endl;
  else
    cout << "Event is SET !" << endl;
  

  return 0;
}


unsigned long wait(void*)
{
  unsigned int i = 0;

  
  cout << "WAIT : warte..." << endl;
  for (i = 0; i < 10; i++)
    {
      if (WaitForSingleObject (Event,500) == WAIT_TIMEOUT)
	cout << "WaitTimeout" << endl;
      else
	{
	  cout << "GOT EVENT !!!" << endl;
	  break;
	}
    }

  return 0;
}
*/

int main()
{
  timestruct_t abstime;
  timeval mytime;

  int ret = 0;

  /*
  DWORD Id1;
  HANDLE SignalerHandle;
  
  DWORD Id2;
  HANDLE WaitHandle;
  */
	
  if (cond_init(&condition, 0, NULL))
    cout <<  "cond_init FAILED !" << endl;
  if (mutex_init(&mutex, 0, NULL))
    cout << "mutex_init FAILED !" << endl;
  cout << "Warte..." << endl;
  int err;
  mutex_lock(&mutex);
  if (gettimeofday (&mytime, NULL) == -1)
    cout << "Fehler Gettimeofday" << endl;
  
  abstime.tv_sec = mytime.tv_sec;
  abstime.tv_nsec = (mytime.tv_usec * 1000) + 100000000 ;
  if ((err=cond_timedwait(&condition, &mutex, &abstime)))
    cout << "Fehler: " << err << " : " << strerror(err) << endl;
  else
    cout << "Ok." << endl;
  mutex_unlock(&mutex);
  cout << "un wech....\n";
	
  return 0;
}
	
	
