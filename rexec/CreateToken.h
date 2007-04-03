
#ifdef __cplusplus
extern "C" {
#endif

HANDLE CreateToken(LPCTSTR szUserName);
BOOL EnablePrivilege(TCHAR *szPrivName, BOOL fEnable);

#ifdef __cplusplus
}
#endif