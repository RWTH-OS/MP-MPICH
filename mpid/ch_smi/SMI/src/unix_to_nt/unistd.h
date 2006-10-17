/* $Id: unistd.h,v 1.1 2004/03/19 22:14:24 joachim Exp $ */

#ifndef _UNISTD_H_
#define _UNISTD_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _SC_PAGESIZE 11

  long sysconf(int);
  int usleep(unsigned useconds);
  int sleep(unsigned seconds);
  /*int gethostname (char * name,int namelen);*/


#ifdef __cplusplus
}
#endif

#endif
