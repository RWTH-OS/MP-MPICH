
//////////////////////////////////////////////////////////////////////
// FileFkts.h Headerfile for FileFkts.cpp
// these files are responsible for genaral actions on files
// for further information see FileFkts.txt
//////////////////////////////////////////////////////////////////////

#include <wtypes.h>




DWORD FileGetSize(LPCSTR FileName);
BOOL FileExists(LPCSTR FileName);
BOOL DirectoryExists(LPCSTR FilePath);
BOOL CreateFilePath(LPCSTR FilePath);

void ChangeFileExt(LPSTR FileName, LPCSTR extension);
void ChangeFileName(LPSTR FileName, LPCSTR newname);
void ChangeFilePath(LPSTR FileName, LPCSTR newpath);

void ExtractFilePath(LPCSTR FileName, LPSTR FilePath);
void ExtractFileName(LPCSTR FileName, LPSTR Name);

//FileName is extended with working directory
BOOL ExtendFileName(LPSTR FileName, int size);

BOOL GetFullExeName(LPSTR ExeName);