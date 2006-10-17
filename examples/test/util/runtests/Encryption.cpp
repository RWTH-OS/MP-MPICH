


#ifndef _WIN32_WINNT  
    #if (_MSC_VER >1200)
      #define _WIN32_WINNT 0x0500
    #else
      #define _WIN32_WINNT 0x0400
    #endif 
#endif

#include <windows.h>

#ifdef __WINCRYPT_H__
#undef __WINCRYPT_H__
#endif 

#include <wincrypt.h>
#include <iostream>

#define BLOCK_SIZE 160

extern BOOL debug_flag; //rexec.cpp
#define DBM(m) if(debug_flag) std::cerr<<m<<std::endl

BOOL CreateKeyset() {
	HCRYPTKEY hXchgKey;
	LONG error;
	HCRYPTPROV hProv;

	std::cerr<<"Creating Keyset...\n";
	//si try CryptAcquireContext first with NULL, then with MS_DEF_PROV
	if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
		DBM("CryptAcquireContext failed try MS_DEF_PROV as parameter");
		if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
		    std::cerr<<GetLastError()<<": Cannot create keyset...\n";
		    return FALSE;
		}
	}
	
	if(!CryptGenKey(hProv,AT_SIGNATURE,0,&hXchgKey))  {
		error=GetLastError();
		DBM("CryptGenKey AT_SIGNATURE failed with errocode "<<error);
		CryptReleaseContext(hProv, 0);
		SetLastError(error);
		return FALSE;
	}

	CryptDestroyKey(hXchgKey);

	if(!CryptGenKey(hProv,AT_KEYEXCHANGE,0,&hXchgKey)) {
		error=GetLastError();
		DBM("CryptGenKey AT_KEYEXCHANGE failed with errocode "<<error);
		CryptReleaseContext(hProv, 0);
		SetLastError(error);
		return FALSE;
	}
	CryptDestroyKey(hXchgKey);
	CryptReleaseContext(hProv, 0);
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
	DWORD dwCount,err;
	DWORD dwBlobLen;
	BOOL res=FALSE;
	DWORD srcIndex=0,destIndex,actSize;
	DWORD error;
	
		// Get a handle to the default provider.
	if(!(res=CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))) {
		err=GetLastError();
		if((err==NTE_KEYSET_NOT_DEF)||(err==NTE_BAD_KEYSET)) {
			res=CreateKeyset();
			if(!res||!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0))
				res=FALSE;
		} else std::cerr<<GetLastError()<<": Cannot Acquire Crypt-Context\n"; 
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
				errcode = GetLastError();
				if (debug_flag)
					DBM("CryptGenKey in CryptGetUserKey failed with errorcode "<<errcode);
				SetLastError(errcode);
				goto done;
			}
		}
		else
		{
			if (debug_flag)
		     	DBM("CryptGetUserKey failed with errorcode "<<errcode);
			SetLastError(errcode);
			goto done;
		}

	}
	
	

	// Create a random block cipher session key.
	if(!CryptGenKey(hProv, CALG_RC4, CRYPT_EXPORTABLE, &hKey)) {
		if (debug_flag)
		{
		    DWORD errcode=GetLastError();
		    DBM("CryptGenKey failed with errorcode "<<errcode);
			SetLastError(errcode);
		}
		goto done;
	}
	
	// Determine the size of the key blob and allocate memory.
	if (!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, NULL, &dwBlobLen)){
		if (debug_flag)
		{
		    DWORD errcode=GetLastError();
		    DBM("CryptExportKey failed with errorcode "<<errcode);
			SetLastError(errcode);
		}
	}
		
	
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
			if (debug_flag)
			{
				DWORD errcode=GetLastError();
				DBM("CryptEncrypt failed with errorcode "<<errcode);
				SetLastError(errcode);
			}
			free(pbBuffer);
			*dwBufferSize=0;
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


BOOL DecryptData(char *buffer, DWORD *dwBufferSize) {
	HCRYPTPROV hProv = 0;
	HCRYPTKEY hKey = 0;
	DWORD dwCount;
	DWORD dwBlobLen;
	BOOL res=FALSE;
	DWORD srcIndex;
	LONG lError;

	//if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0)) { 
	//Si: with MS_DEF_PROV CryptImportKey fails with XP

	
	// Get a handle to the default provider.
	if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0)) {
	DBM("CryptAcquireContext failed try MS_DEF_PROV as parameter");
	  if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0)) { 
		LPVOID lpMsgBuf;
		DWORD errcode = GetLastError();
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,errcode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
			
		std::cerr<<"Could not Acquire Context.\n"<<errcode<<": "<<(char*)lpMsgBuf;
		LocalFree(lpMsgBuf);
		goto done;
	  }
	}
	
	dwBlobLen=*(DWORD*)buffer;
	
	// Determine the size of the key blob and allocate memory.
	if(!CryptImportKey(hProv, (PBYTE)(buffer+sizeof(DWORD)), dwBlobLen, 0, 0, &hKey))  {
		LPVOID lpMsgBuf;
		DWORD errcode = GetLastError();
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,errcode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
			
		std::cerr<<"Could not Import Key.\n"<<errcode<<": "<<(char*)lpMsgBuf;
		LocalFree(lpMsgBuf);
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


