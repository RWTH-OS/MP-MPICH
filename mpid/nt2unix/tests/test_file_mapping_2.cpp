/* $id$ */

#include <windows.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#define Size (1*1024*1024)

LPVOID Address;

volatile unsigned long counter;

LONG MemoryExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo) { 	// address of exception info 
	DWORD OldProtection;
	
	EXCEPTION_RECORD *ExRec=ExceptionInfo->ExceptionRecord;
	
	if(ExRec->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION) {
		return EXCEPTION_CONTINUE_SEARCH;
	} 
	cerr << (ExRec->ExceptionInformation[0]?"Write":"Read")<<"-violation at "<<(void*)ExRec->ExceptionInformation[1]<<endl;

        if(ExRec->ExceptionInformation[1] == 0)
          return EXCEPTION_CONTINUE_SEARCH; 			

//	if ((counter * 4096 + (DWORD)Address) !=
//	     (DWORD)ExRec->ExceptionInformation[1]) 
//	   cerr << "***** UUUUUUUUUUUUUUUUUUPS ?!?!?"<<endl;

	counter++;          
	if(counter%2) VirtualAlloc((void*)ExRec->ExceptionInformation[1],4096, MEM_COMMIT, PAGE_READONLY);
	else  VirtualAlloc((void*)ExRec->ExceptionInformation[1],4096, MEM_COMMIT, PAGE_READWRITE);
	return EXCEPTION_CONTINUE_EXECUTION;
}



int main() {


 	volatile char a = '\0'; 
 	HANDLE f; 
 	DWORD i; 

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MemoryExceptionFilter);
	counter=0;
	
	f = CreateFileMapping((HANDLE)0xFFFFFFFF, 0, SEC_RESERVE|PAGE_READWRITE,
	                       0, Size, 0); 
	Address=MapViewOfFile(f, 0, 0, 0, Size);
	if(!Address) {
		cerr<<"VirtualAllocFailed!!!\n";
		return 0;
	}

	for (i = 0; i < Size; i+=4096) {
		a = ((char*)Address)[i];
		//cerr<<" i=="<<(LPVOID)i<<endl;
		//a++;
		((char*)Address)[i] = a;
	
	}
	return 0;
	
	counter = 0; 
	if (!VirtualProtect(Address, Size, PAGE_NOACCESS, 0))
	  perror("VirtualProtect()");
	for (i = 0; i < Size; i+=4096) {
		cerr<<" i=="<<(LPVOID)i<<endl;
		a = ((char*)Address)[i];
		// ((char*)Address)[i] = a;
		a++;
	}

	cerr<<"expected: "<<Size/4096<<endl;
	fprintf(stderr, "last %x\n",  (DWORD)Address+Size-4096);
	cerr<<"got == "<<counter<<endl;
	return 0;
}
