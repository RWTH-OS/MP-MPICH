/* $id */

#include <windows.h>
#include <iostream>

using namespace std;

int main() {
	LARGE_INTEGER l;
	DWORD w; 
	BOOL res,res1;
	HANDLE Mapping;
	LPVOID Address=0,Address1=0;
	int Value=0;
	DWORD OldProt;
	

	l.QuadPart = 1; 
	cerr << l.u.LowPart << " " << l.u.HighPart << endl;
	 
	cerr << sizeof(LONGLONG) << endl;
	
	Mapping=CreateFileMapping((HANDLE)0xFFFFFFFF,0,SEC_RESERVE|PAGE_READWRITE,0,8192,0);
	if(!Mapping) {
		cerr<<GetLastError()<<" test1: Cannot create FileMapping\n";
		return 1;
	}


	//Address=(LPVOID)0x80001000;	
	Address=0;
	Address=MapViewOfFileEx(Mapping,0,0,0,0,Address);
	Address1=MapViewOfFile(Mapping,FILE_MAP_ALL_ACCESS,0,0,0);
	
	CloseHandle(Mapping);
	cerr<<"Addresses are: "<<Address<<" "<<Address1<<endl;
	
	if(!Address||!Address1) {
		cerr<<GetLastError()<<" Cannot map view\n";
		return 1;
	}

	

	VirtualAlloc(Address,8192,MEM_COMMIT,PAGE_READWRITE);

	
	cerr<<"Writing 122 to Address[0]\n";
	((int*)Address)[0]=122;

	VirtualProtect(Address1,8192,PAGE_READWRITE,&OldProt);
	cerr<<"Reading from Address1[0]\nResult: "<<((int*)Address1)[0]<<endl;
   res1=UnmapViewOfFile(Address1);	
	res= UnmapViewOfFile(Address);
   if(!res || !res1)
    cerr<<"Unmapping failed "<<res<<" "<<res1<<endl;
	return 0;
}


