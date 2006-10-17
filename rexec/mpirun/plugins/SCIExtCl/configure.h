#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__


typedef struct {
    BOOL Global;
    char Port[8];
    BOOL Debug;
    BOOL Verbose;
#ifdef MPI
    char Conffile[MAX_PATH];

    DWORD LogFormat;
    DWORD StartServer;
    char Display[256];
#endif
} ConfContext;


BOOL CALLBACK ConfigureProc(HWND dlg,UINT message,WPARAM wParam,LPARAM lParam);
void initContext(ConfContext* Context);

#define CH_SMI_BASE_PORT 25033

#endif