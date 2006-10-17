
#ifdef __cplusplus
extern "C" {
#endif

void WINAPI _PlgDescription(struct PlgDesc *pDesc);
BOOL WINAPI SCIExtIsValid(struct HostData *spHost);
BOOL WINAPI SCIExtCreateComandline(struct HostData** sppHostAll,
								   DWORD SizeAll, 
								   struct HostData*** spppHostSelected, 
								   DWORD *pSizeSelected);
BOOL WINAPI SCIExtCreateUserInfo(stateEntry *spState, char *output, DWORD *pSize);
void WINAPI *SVMlibDetach(void);

#ifdef __cplusplus
}
#endif
