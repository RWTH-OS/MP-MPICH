
#ifndef __STARTSERVER_H__
#define __STARTSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern DWORD (*Start_MPE_Server)(HWND);
extern VOID (*Stop_MPE_Server)(HWND);

BOOL StopMPEServer(void);
BOOL StartMPEServer(void);

#ifdef __cplusplus
}
#endif

#endif