#ifndef __ENCRYPTION_H__
#define __ENCRYPTION_H__

BOOL EncryptData(char *buffer, char **encrData,DWORD *dwBufferSize);
BOOL DecryptData(char *buffer, DWORD *dwBufferSize);

#endif
