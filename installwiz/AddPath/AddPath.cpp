// AddPath.cpp : Definiert den Einsprungpunkt für die Konsolenanwendung.
//

#include "windows.h"


int main(int argc, char* argv[])
{

    HKEY hKey = HKEY_LOCAL_MACHINE;         // handle to open key
    char  lpSubKey[500];  // address of name of subkey to open
    DWORD ulOptions = 0;   // reserved
    REGSAM samDesired = KEY_ALL_ACCESS; // security access mask
    HKEY hkResult = 0;    // address of handle to open key
    LONG  myResult;
	char *Value,*newPath,*ptrchar=NULL;  // buffer for returned string
    LONG cbValue=999;
	DWORD dwSize=MAX_PATH;
    DWORD dwKeyType=REG_EXPAND_SZ;
	
		
	if (argc<2)
		return -1;
    newPath = argv[1];

	strcpy(lpSubKey,"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\");  // address of name of subkey to open
    myResult=RegOpenKeyEx(hKey,lpSubKey,ulOptions,samDesired,&hkResult);

	Value=(char*) calloc(5000,sizeof(char));   

	myResult = RegQueryValueEx(hkResult, TEXT("Path"), NULL,
	&dwKeyType, (LPBYTE)Value, &dwSize);

	if (myResult!=NO_ERROR)
		return -1;

	strcat(Value,";");
	
	ptrchar =strstr(Value,newPath);
	
	while (ptrchar != NULL) 
	{
		//new path is already included in path
		// check if only subpath is included
		ptrchar = ptrchar + strlen(newPath);
		if ((ptrchar[0] == '#0') || (ptrchar[0] == ';')) 
		{
			//path is already included
			RegCloseKey(hkResult);
			return 0;
		}
		ptrchar =strstr(ptrchar,newPath);
	}

	strcat(Value,newPath);

	RegSetValueEx(hkResult,TEXT("Path"),0,dwKeyType, (LPBYTE)Value,strlen(Value)+1);


	
	RegCloseKey(hkResult);

	return 0;
}
