
#include <signal.h>
#include <sys/systeminfo.h>
#include <ucontext.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define size (4*4096)
#define oSize 1
#define oMap 0

void* addr;

void signalHandler(int sig,siginfo_t *sip, void *uap) {
  if(sig==SIGSEGV) {
    if( (*(unsigned *)((ucontext_t*)uap)->uc_mcontext.gregs[REG_PC] & (1<<21))) 
      fprintf(stderr,"Write fault at 0x%x\n",sip->si_addr);
    else fprintf(stderr,"Read fault at 0x%x\n",sip->si_addr);
    if(mprotect(sip->si_addr,4096,PROT_WRITE|PROT_READ)==-1) {
      perror("mprotect()"); exit(1); }
  } else sigaction(sig, NULL, NULL);
}


int main() {
  struct sigaction act;
  FILE* stream;
  int i;
  volatile int res;

  act.sa_flags= SA_RESTART|SA_SIGINFO;
  act.sa_sigaction=signalHandler;
  if(sigaction(SIGSEGV,&act,NULL)) { perror("sigaction()"); return 1;}
  if(!(stream=tmpfile())) { perror("tmpfile()"); return 1;}
  if(ftruncate(stream->_file,(off_t)(size+oSize)) == -1) { perror("ftruncate()"); return 1;}

  if((void*)MAP_FAILED==
    (addr=mmap((caddr_t)0,(size_t)(size+oMap),PROT_NONE,MAP_SHARED,stream->_file,0))){ 
      perror("mmap()"); return 1; }

  for(i=0;i<size;i+=4096) {
    fprintf(stderr,"Trying Address 0x%x \n",i+(char*)addr);
    res=((char*)addr)[i]; /* should result in a readfault */
    fprintf(stderr,"Read value: %d\n-------------\n",res);
    ((char*)addr)[i]='\0'; /* should not result in a writefault because we made
                         the page writable after the readfault (if it occured).*/
  }
 return 0;
}
