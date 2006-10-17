/* $id$ */

#include <windows.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#define Size 8192
LPVOID Address;

volatile unsigned long counter;



DWORD __stdcall threadFunc(void *) {
  fprintf(stderr, "Thread start \n");
  getc(stdin); 
  return 0;
}


LONG MemoryExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo) { 	// address of exception info 
	DWORD OldProtection;
	
	EXCEPTION_RECORD *ExRec=ExceptionInfo->ExceptionRecord;
	
	if(ExRec->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION) {
		return EXCEPTION_CONTINUE_SEARCH;
	} 
	fprintf(stderr, "Counter == %d\n", counter);
	cerr << (ExRec->ExceptionInformation[0]?"Write":"Read")<<"-violation at "<<(void*)ExRec->ExceptionInformation[1]<<endl;

	
   if(ExRec->ExceptionInformation[1] == 0)
          return EXCEPTION_CONTINUE_SEARCH; 			
          
	 if(++counter<100)	
		return EXCEPTION_CONTINUE_EXECUTION;
	 else {
		VirtualProtect((void*)ExRec->ExceptionInformation[1],4096,PAGE_READWRITE,&OldProtection);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
}



int main() {


 	volatile char a = '\0'; 

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MemoryExceptionFilter);
	counter=0;
	Address=VirtualAlloc(0,Size,MEM_COMMIT,PAGE_NOACCESS);
	if(!Address) {
		cerr<<"VirtualAllocFailed!!!\n";
		return 0;
	}
	
	DWORD id; 
	CreateThread(0, 0, threadFunc, 0, 0, &id); 
	//__try {
		((char*)Address)[4096]=0;
		counter=0;
		a = *((char*)Address); 
	//} __except(MemoryExceptionFilter(GetExceptionInformation())){};
	


return 0;
}
