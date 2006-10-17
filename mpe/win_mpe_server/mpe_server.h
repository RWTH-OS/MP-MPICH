
#ifndef __MPE_SERVER_H__
#define __MPE_SERVER_H__

#ifdef MPE_SERVER_EXPORTS
#define MPE_SERVER_API __declspec(dllexport)
#else
#define MPE_SERVER_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MPM_SERVER_STARTING (WM_USER+1)
#define MPM_SERVER_STOPPING (WM_USER+2)
#define MPM_SERVER_STARTED  (WM_USER+3)
#define MPM_SERVER_STOPPED  (WM_USER+4)
#define MPM_SERVER_PENDING  (WM_USER+5)

MPE_SERVER_API DWORD Start_MPE_Server(HWND NotifyWindow);
MPE_SERVER_API VOID Stop_MPE_Server(HWND NotifyWindow);

#ifdef __cplusplus
}
#endif

#endif