/* $id$ */

#include <wtypes.h>
#include <stdio.h>
#include <iostream.h>

#define Size 4095
LPVOID Address;

unsigned long counter;




LONG MemoryExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo) { 	// address of exception info 
	DWORD OldProtection;
	
	EXCEPTION_RECORD *ExRec=ExceptionInfo->ExceptionRecord;
	
	if(ExRec->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION) {
		return EXCEPTION_CONTINUE_SEARCH;
	} 
	 if(++counter<10000)	
		return EXCEPTION_CONTINUE_EXECUTION;
	 else {
		VirtualProtect(Address,Size,PAGE_READWRITE,&OldProtection);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
}



int _cdecl main() {


SYSTEMTIME t0,t1,r;

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MemoryExceptionFilter);
	counter=0;
	Address=VirtualAlloc(0,Size,MEM_COMMIT,PAGE_NOACCESS);
	if(!Address) {
		cerr<<"VirtualAllocFailed!!!\n";
		return 0;
	}
	GetSystemTime(&t0);
	//__try {
		*((char*)Address)=0; 
	//} __except(MemoryExceptionFilter(GetExceptionInformation())){};
	
		GetSystemTime(&t1);

	if(t1.wMilliseconds<t0.wMilliseconds) {
			r.wMilliseconds=t1.wMilliseconds+1000-t0.wMilliseconds;
			t0.wSecond++;
	} else r.wMilliseconds=t1.wMilliseconds-t0.wMilliseconds;
	
	if(t1.wSecond<t0.wSecond) {
			r.wSecond=t1.wSecond+60-t0.wSecond;
			t0.wMinute++;
	} else r.wSecond=t1.wSecond-t0.wSecond;

	if(t1.wMinute<t0.wMinute) {
			r.wMinute=t1.wMinute+60-t0.wMinute;
	} else	r.wMinute=t1.wMinute-t0.wMinute;

	cerr<<"The time for 10000 iterations is: "<<r.wMinute<<":"<<r.wSecond<<":"<<(float)r.wMilliseconds<<endl<<endl;

return 0;
}
