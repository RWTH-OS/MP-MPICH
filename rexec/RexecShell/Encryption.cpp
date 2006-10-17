
#include <windows.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <wincrypt.h>


#define BLOCK_SIZE 160

BOOL CreateKeyset(HCRYPTPROV *hProv) {
	HCRYPTKEY hXchgKey;
	LONG error;
	if(!CryptAcquireContext(hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))
		return FALSE;

    if(!CryptGenKey(*hProv,AT_SIGNATURE,0,&hXchgKey))  {
		error=GetLastError();
		CryptReleaseContext(*hProv, 0);
		SetLastError(error);
		return FALSE;
	}
    CryptDestroyKey(hXchgKey);
    
	if(!CryptGenKey(*hProv,AT_KEYEXCHANGE,CRYPT_EXPORTABLE,&hXchgKey)) {
		error=GetLastError();
		CryptReleaseContext(*hProv, 0);
		SetLastError(error);
		return FALSE;
	}
	CryptDestroyKey(hXchgKey);
	return TRUE;
}
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

BOOL EncryptData(char *buffer, char **encrData,DWORD *dwBufferSize) {
	HCRYPTPROV hProv = 0;
	HCRYPTKEY hKey = 0;
	HCRYPTKEY hXchgKey = 0;
	BYTE *pbBuffer;
	DWORD dwCount;
	DWORD dwBlobLen;
	BOOL res;
	DWORD srcIndex=0,destIndex,actSize;
	DWORD error;
	// Get a handle to the default provider.
    //if(!(res=CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))) {
	if(FALSE == (res=CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))) {
		if((GetLastError()==NTE_KEYSET_NOT_DEF)||(GetLastError()==NTE_BAD_KEYSET))
			res=CreateKeyset(&hProv);
	}

    if(!res) {
		goto done;
	}

	res=FALSE;
    
    // Get a handle to key exchange key.
	if(!CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hXchgKey)) {

	//Si
		DWORD errcode = GetLastError();
		if ( errcode == NTE_NO_KEY)
		{
			//create key exchange key pair
			if (! CryptGenKey(hProv,AT_KEYEXCHANGE,0,&hXchgKey))
			{
				goto done;
			}
		}
		else
		{
			SetLastError(errcode);
			goto done;
		}

	}


	// Create a random block cipher session key.
	if(!CryptGenKey(hProv, CALG_RC4, CRYPT_EXPORTABLE, &hKey)) {
		goto done;
	}
	
	// Determine the size of the key blob and allocate memory.
	CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, NULL, &dwBlobLen);
		
	
	actSize=dwBlobLen+sizeof(DWORD)+*dwBufferSize+64;
	if((pbBuffer = (PBYTE)malloc(actSize)) == NULL) goto done;
	
	*(DWORD*)pbBuffer=dwBlobLen;
	// Export the key into a simple key blob.
	if(!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, pbBuffer+sizeof(DWORD),&dwBlobLen)) {
		free(pbBuffer);
		goto done;
	}
	
	
	destIndex=dwBlobLen+sizeof(DWORD);
	while(srcIndex<*dwBufferSize) {    
		dwCount = min(BLOCK_SIZE,*dwBufferSize-srcIndex);	
		memcpy(pbBuffer+destIndex,buffer+srcIndex,dwCount);
		srcIndex+=dwCount;
		if(actSize<destIndex) {
			pbBuffer=(PBYTE)realloc(pbBuffer,2*destIndex);
			actSize=2*destIndex;
		}
		
		if(!CryptEncrypt(hKey, 0, srcIndex>=*dwBufferSize, 0, pbBuffer+destIndex, &dwCount, actSize-destIndex))    {
			free(pbBuffer);
			*dwBufferSize=0;
            *encrData = 0;
			goto done;    
		}    
		destIndex+=dwCount;
	}
	res=TRUE;
	*dwBufferSize=destIndex;
	*encrData=(char*)pbBuffer;


done:
	error=GetLastError();
	// Destroy the session key.
	if(hKey != 0) CryptDestroyKey(hKey);
	// Destroy the key exchange key.
	if(hXchgKey != 0) CryptDestroyKey(hXchgKey);
	// Release the provider handle.
	if(hProv != 0) CryptReleaseContext(hProv, 0);
	//if(pbBuffer) free(pbBuffer);
	SetLastError(error);
	return res;
}

char *MakeErrorMessage(DWORD ErrorId);
BOOL DecryptData(char *buffer, DWORD *dwBufferSize) {
	HCRYPTPROV hProv = 0;
	HCRYPTKEY hKey = 0;
	DWORD dwCount;
	DWORD dwBlobLen;
	BOOL res=FALSE;
	DWORD srcIndex;
	LONG lError;

	// Get a handle to the default provider.
	if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0)) goto done;

    // Get a handle to key exchange key.

	dwBlobLen=*(DWORD*)buffer;
	// Determine the size of the key blob and allocate memory.
	if(!CryptImportKey(hProv, (PBYTE)(buffer+sizeof(DWORD)), dwBlobLen, 0, 0, &hKey)) {
     //if(GetLastError()==NTE_NO_KEY)
     	MessageBox(0,MakeErrorMessage(GetLastError()),"CryptImportKey",MB_OK|MB_ICONERROR);
     goto done;
    }
	
	srcIndex=dwBlobLen+sizeof(DWORD);
	

	while(srcIndex<*dwBufferSize) {    
		dwCount = min(BLOCK_SIZE,*dwBufferSize-srcIndex);	
		if(!CryptDecrypt(hKey, 0, srcIndex+dwCount>=*dwBufferSize, 0, (PBYTE)buffer+srcIndex, &dwCount)) {
			goto done;    
		}    
		srcIndex+=dwCount;
	}
	*dwBufferSize=dwBlobLen+sizeof(DWORD);
	res=TRUE;

done:
	lError=GetLastError();
	// Destroy the session key.
	if(hKey != 0) CryptDestroyKey(hKey);
	// Destroy the key exchange key.
	if(hProv != 0) CryptReleaseContext(hProv, 0);
	//if(pbBuffer) free(pbBuffer);
	SetLastError(lError);
	return res;
}


