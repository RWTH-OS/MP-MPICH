#define WIN32_LEAN_AND_MEAN

#include <windows.h>



BOOL DLLMain(HINSTANCE hLib,DWORD reason) {

    // For performance reasons...
    if(reason == DLL_PROCESS_ATTACH)
	DisableThreadLibraryCalls(hLib);
    
    return TRUE;

}