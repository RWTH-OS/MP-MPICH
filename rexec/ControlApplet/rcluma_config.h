
#ifndef __RCLUMA_CONFIGURE___
#define __RCLUMA_CONFIGURE___

#ifdef __cplusplus
extern"C" {
#endif

typedef struct {
    BOOL Logging;
    char *LogFile;
    DWORD NumThreads;
    BOOL Running;
    BOOL Changed;
    DWORD OpenResult;
    SC_HANDLE hService;
	BOOL NoUser;
} CONFIGURE_T;

void SetDialogData(HWND dlg,CONFIGURE_T *C);
void GetDialogData(HWND dlg,CONFIGURE_T *C);

#ifdef __cplusplus
}
#endif


#endif
