/* $Id: test_file_mapping_3.cpp,v 1.1.2.1 2004/11/25 16:19:24 carsten Exp $ */

#include <windows.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#define Size (16*1024*1024)
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
          
	return EXCEPTION_CONTINUE_SEARCH;
}



int main() {

	DWORD old; 
	HANDLE h = (HANDLE)0xFFFFFFFF; 
	int i; 
 	volatile unsigned char a = '\0'; 

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MemoryExceptionFilter);
	counter=0;
	
	h = CreateFileMapping(h, 0, SEC_RESERVE | PAGE_READWRITE,
	  0, Size, 0);

	Address = MapViewOfFileEx(h, 0, 0, 0, 0, (void*)0x80000000);
	  if(!Address) {
		cerr<<"MapViewOfFile failed!!!\n";
		cerr<<WSAGetLastError(); 
		return 0;
	    }

	VirtualProtect(Address, Size, PAGE_EXECUTE_READWRITE, &old); 	
	a = ((char*)Address)[0];
	((char*)Address)[1] = 1;
	VirtualProtect(Address, Size, PAGE_EXECUTE_READ, &old); 	
	for (i = 0; i<= 100; i++) {
	  a = ((char volatile*)Address)[0];
	  fprintf(stderr, "i == %d\n, a == %d", i, a);
	} 


	VirtualProtect(Address, 4096, PAGE_NOACCESS, &old); 	
	//a = ((char*)Address)[0];

 	return 0;
}
