#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#ifdef MPI
#define VISUAL_NAME "ch_wsock"
#else
#define VISUAL_NAME "SVMlib"
#endif


typedef struct {
    BOOL Global;
    char Port[8];
    char ActIp[20];
    DWORD AutoIp;
    DWORD LogFormat;
    DWORD StartServer;
    char Display[256];
} ConfContext;


BOOL CALLBACK ConfigureLocalProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK ConfigureGlobalProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam);
void initContext(ConfContext* Context);

#endif