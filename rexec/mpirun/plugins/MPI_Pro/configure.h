#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__


typedef struct {
    BOOL Global;
    BOOL Debug;
    BOOL Verbose;
    char Port[8];
} ConfContext;


BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam);
void initContext(ConfContext* Context);


#endif