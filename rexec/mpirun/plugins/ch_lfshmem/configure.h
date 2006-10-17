#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__


typedef struct {
    BOOL Global;
    char Num[8];
    DWORD LogFormat;
    DWORD StartServer;
    char Display[256];

} ConfContext;


BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam);
void initContext(ConfContext* Context);

#endif