#ifndef __sciext_h__
#define __sciext_h__


/*

  This is no plugin!
  The library SCIExt.dll is used by rclumad to gain information about sci adapters.
  Should be replaced by calls to new sissci-API dll by dolphin.

*/

#ifdef __cplusplus
extern "C" {
#endif

#include <wtypes.h>

#define MAXADAPTERS 10

typedef struct _DriverInfo {
    DWORD      irm_revision;
    DWORD      sc_revision;
    char       sc_revision_string[200];
} DriverInfo;

typedef struct _QueryAdapter {
#if defined(WIN32_NEW_IOCTL)
   sci_error_t    errcode;
#endif
    DWORD      adapterNo;
    DWORD      portNo;
    DWORD      command;
    DWORD      data;
    DWORD      flags;
} QueryAdapter;

typedef struct _QueryAdapterO {
    DWORD      adapterNo;
    DWORD      command;
    DWORD      data;
    DWORD      flags;
} QueryAdapter_old;

typedef struct {
    unsigned int NumAdapters;
    DriverInfo Driver; 
    unsigned int Ids[MAXADAPTERS];
	unsigned int Subnets[MAXADAPTERS];
} NodeData;
    
    
__declspec(dllexport) BOOL WINAPI _QueryUserData(void *SciId, DWORD *size);

#ifdef __cplusplus
}
#endif

#endif /* __sciext_h__ */