#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__


typedef struct {
    BOOL Global;
    BOOL Polling;
    BOOL Singlethreaded;
    BOOL Verbose;
    char Port[8];
    char ActIp[20];
    DWORD AutoIp;
} ConfContext;


BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam);
void initContext(ConfContext* Context);


#endif