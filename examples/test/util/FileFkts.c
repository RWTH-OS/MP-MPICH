
#include "FileFkts.h"
#include <malloc.h>
#include <stdlib.h>


#ifdef unicode 

#define _splitpath _wsplitpath
#define _makepath _wmakepath
#define _fullpath _wfullpath

#endif


//////////////////////////////////////////////////////////////////////
DWORD FileGetSize(LPCTSTR FileName)
{
	//get the size of a file without opening it 
	//you can get the size even if the file is exclusively opened by another application

	LPWIN32_FIND_DATA lpFindFileData;
	DWORD returnvalue = 0;
	HANDLE myhandle;

	lpFindFileData = (LPWIN32_FIND_DATA) malloc (sizeof(WIN32_FIND_DATA));

    FindFirstFile(FileName,lpFindFileData);
    if (myhandle != INVALID_HANDLE_VALUE)
	{
	  if ((lpFindFileData->nFileSizeHigh)==0)
      {
	  	  //filesize smaller than MAXDWORD
	      returnvalue = lpFindFileData->nFileSizeLow;
      }
      FindClose(myhandle);
    }

	free(lpFindFileData); lpFindFileData = NULL;
    
 	return(returnvalue);
}

//////////////////////////////////////////////////////////////////////
BOOL FileExists(LPCSTR FileName)
{
    //find a file without opening it 
	//you can find the file even if the file is exclusively opened by another application

	LPWIN32_FIND_DATA lpFindFileData;
	BOOL returnvalue = FALSE;
	HANDLE myhandle;

	lpFindFileData = (LPWIN32_FIND_DATA) malloc (sizeof(WIN32_FIND_DATA));

    
	myhandle = FindFirstFile(FileName,lpFindFileData);
	if (myhandle != INVALID_HANDLE_VALUE)
	{
		//function succeds
      DWORD dwAttribTest;
      dwAttribTest = (lpFindFileData->dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY;
      if (dwAttribTest != FILE_ATTRIBUTE_DIRECTORY)
      {
	  	  
	      returnvalue = TRUE;
      }
	    
      FindClose(myhandle);
	}
	free(lpFindFileData); lpFindFileData = NULL;
 	return(returnvalue);
}

//////////////////////////////////////////////////////////////////////
BOOL DirectoryExists(LPCSTR FilePath)
{
    //find a directory without opening it 

	LPWIN32_FIND_DATA lpFindFileData;
	BOOL returnvalue = FALSE;
	HANDLE myhandle;

	lpFindFileData = (LPWIN32_FIND_DATA) calloc (1,sizeof(WIN32_FIND_DATA));

    myhandle = FindFirstFile(FilePath,lpFindFileData);
	if (myhandle != INVALID_HANDLE_VALUE)
	{
		//function succeeds
      DWORD dwAttribTest;
      dwAttribTest = (lpFindFileData->dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY;
      if (dwAttribTest == FILE_ATTRIBUTE_DIRECTORY)
      {
	  	 
	      returnvalue = TRUE;
      }
	    
      FindClose(myhandle);
	}
	free(lpFindFileData); lpFindFileData = NULL;
 	return(returnvalue);
}

//////////////////////////////////////////////////////////////////////
BOOL CreateFilePath(LPCSTR FilePath)
{
	char * dirbuf,* pdir, * fullpath;
	BOOL ok = TRUE,direx = FALSE;
    int index,pathlength;
	BOOL erz;

    if (DirectoryExists(FilePath))
        return(TRUE);
   SetLastError(ERROR_INVALID_NAME);
    if (strlen(FilePath)<1)
        return(FALSE);

    
    // Wildcards are not allowed    
    pdir = strchr(FilePath,'*');
    if (pdir != NULL)
        return(FALSE);
    pdir = strchr(FilePath,'?');
    if (pdir != NULL)
        return(FALSE);

    

    dirbuf = (char *) calloc (strlen(FilePath)+2,sizeof(char));
    fullpath = (char *) calloc (strlen(FilePath)+2,sizeof(char));

    strcpy(fullpath,FilePath);

    if(fullpath[strlen(fullpath)-1] != '\\')
        fullpath[strlen(fullpath)] = '\\';

    pathlength = strlen(fullpath);

    erz = CreateDirectory(fullpath,NULL);

    //skip device letter
    pdir = strchr(fullpath,':');
    if (pdir != NULL)
      pdir = strchr(fullpath,'\\');
    if (pdir == NULL)
       pdir = (char *) fullpath;
    else
      pdir ++;

    direx = DirectoryExists(fullpath); 
    erz = TRUE;
    while ((pdir<fullpath+pathlength) && (!direx) && (erz))
    {    
        pdir = strchr(pdir,'\\');

        pdir ++;

        index = pdir-fullpath;

        strncpy(dirbuf,fullpath,index);
        if (! DirectoryExists(dirbuf))
          erz = CreateDirectory(dirbuf,NULL);
        direx = DirectoryExists(fullpath); 
    }
    free(dirbuf); 
    free(fullpath);
    return(DirectoryExists(FilePath));

}

//////////////////////////////////////////////////////////////////////
void ChangeFileExt(LPSTR FileName, LPCSTR extension)
{
    char *path,*drive,*dir,*fname,*ext;
    DWORD maxlength;
	char * charptr = NULL;

    maxlength = strlen(FileName)+strlen(extension)+1;

    path = (char *) calloc (maxlength,1);
    drive = (char *) calloc (maxlength,1);
    dir = (char *) calloc (maxlength,1);
    fname = (char *) calloc (maxlength,1);
    ext = (char *) calloc (maxlength,1);

    //delete leading and ending " if present
    if (FileName[0] == '"')
        strcpy(path,FileName+1);
    else
        strcpy(path,FileName);
    
    charptr = strchr(path,'"');
    if (charptr != NULL)
        charptr[0]='\0';

    _splitpath(path,drive,dir,fname,ext);
    strcpy(ext,extension);
    _makepath(path,drive,dir,fname,ext);
    strcpy(FileName,path);

    free(ext);
    free(fname);
    free(dir);
    free(drive);
    free(path);
}

//////////////////////////////////////////////////////////////////////
void ChangeFileName(LPSTR FileName, LPCSTR newname)
{
    char *path,*drive,*dir,*fname,*ext;
    DWORD maxlength;
	char * charptr = NULL;

    maxlength = strlen(FileName)+strlen(newname)+1;

    path = (char *) calloc (maxlength,1);
    drive = (char *) calloc (maxlength,1);
    dir = (char *) calloc (maxlength,1);
    fname = (char *) calloc (maxlength,1);
    ext = (char *) calloc (maxlength,1);

    //delete leading and ending " if present
    if (FileName[0] == '"')
        strcpy(path,FileName+1);
    else
        strcpy(path,FileName);
    
    charptr = strchr(path,'"');
    if (charptr != NULL)
        charptr[0]='\0';

    _splitpath(path,drive,dir,fname,ext);
    strcpy(fname,newname);
    _makepath(path,drive,dir,fname,ext);
    strcpy(FileName,path);
   

    free(ext);
    free(fname);
    free(dir);
    free(drive);
    free(path);
}

//////////////////////////////////////////////////////////////////////
void ChangeFilePath(LPSTR FileName, LPCSTR newpath)
{
    char *path,*drive,*dir,*fname,*ext;
    char *hlppath,*newdrive,*newdir,*newfname,*newext;
    DWORD maxlength;
	char * charptr = NULL;

    maxlength = strlen(FileName)+strlen(newpath)+2+1; 
    // +1: '\0' +2: if there are leading end ending backslash added
    path = (char *) calloc (maxlength,1);
    drive = (char *) calloc (maxlength,1);
    dir = (char *) calloc (maxlength,1);
    fname = (char *) calloc (maxlength,1);
    ext = (char *) calloc (maxlength,1);

    hlppath = (char *) calloc (maxlength,1);
    newdrive = (char *) calloc (maxlength,1);
    newdir = (char *) calloc (maxlength,1);
    newfname = (char *) calloc (maxlength,1);
    newext = (char *) calloc (maxlength,1);

    //delete leading and ending " if present
    if (FileName[0] == '"')
        strcpy(path,FileName+1);
    else
        strcpy(path,FileName);
    
    charptr = strchr(path,'"');
    if (charptr != NULL)
        charptr[0]='\0';

    strcpy(hlppath,newpath);
    if ((hlppath[strlen(hlppath)-1]) != '\\')
        strcat(hlppath,"\\");

    _splitpath(path,drive,dir,fname,ext);
    _splitpath(hlppath,newdrive,newdir,newfname,newext);
    
    
    strcpy(drive,newdrive);

    strcpy(dir,newdir);

    _makepath(path,drive,dir,fname,ext);
    strcpy(FileName,path);
   
    free(newext);
    free(newfname);
    free(newdir);
    free(newdrive);
    free(hlppath);

    free(ext);
    free(fname);
    free(dir);
    free(drive);
    free(path);
}

//////////////////////////////////////////////////////////////////////
void ExtractFilePath(LPCSTR FileName, LPSTR FilePath)
{
    char *path,*drive,*dir,*fname,*ext;
    DWORD maxlength;
	char * charptr = NULL;

    maxlength = strlen(FileName)+1;

    path = (char *) calloc (maxlength,1);
    drive = (char *) calloc (maxlength,1);
    dir = (char *) calloc (maxlength,1);
    fname = (char *) calloc (maxlength,1);
    ext = (char *) calloc (maxlength,1);

    //delete leading and ending " if present
    if (FileName[0] == '"')
        strcpy(path,FileName+1);
    else
        strcpy(path,FileName);
    
    charptr = strchr(path,'"');
    if (charptr != NULL)
        charptr[0]='\0';
    
    _splitpath(path,drive,dir,fname,ext);

    strcpy(FilePath,drive);
    strcat(FilePath,dir);

    free(ext);
    free(fname);
    free(dir);
    free(drive);
    free(path);
}

//////////////////////////////////////////////////////////////////////
void ExtractFileName(LPCSTR FileName, LPSTR Name)
{
    char *path,*drive,*dir,*fname,*ext;
    DWORD maxlength;
	char * charptr = NULL;

    maxlength = strlen(FileName)+1;

    path = (char *) calloc (maxlength,1);
    drive = (char *) calloc (maxlength,1);
    dir = (char *) calloc (maxlength,1);
    fname = (char *) calloc (maxlength,1);
    ext = (char *) calloc (maxlength,1);

    //delete leading and ending " if present
    if (FileName[0] == '"')
        strcpy(path,FileName+1);
    else
        strcpy(path,FileName);
    
    charptr = strchr(path,'"');
    if (charptr != NULL)
        charptr[0]='\0';
    _splitpath(path,drive,dir,fname,ext);

    strcpy(Name,fname);
    strcat(Name,ext);

    free(ext);
    free(fname);
    free(dir);
    free(drive);
    free(path);
}

//////////////////////////////////////////////////////////////////////
BOOL ExtendFileName(LPSTR FileName, int size)
{
    char *path,*extpath,* perg;
    BOOL returnvalue = FALSE;
   
    path = (char *) calloc (size+1,1);
    extpath = (char *) calloc (size+1,1);

    strcpy(path,FileName);
    perg = _fullpath(extpath,path,size);
    
    if (perg != NULL)
    {
        returnvalue = TRUE;
        strcpy(FileName,extpath);

    }

    return(returnvalue); 
}
//////////////////////////////////////////////////////////////////////

BOOL GetFullExeName(LPSTR ExeName)
{
    char PathName[_MAX_PATH];
         
    BOOL returnvalue = FALSE;
    DWORD numcopied;
    numcopied = GetModuleFileName(NULL,PathName,_MAX_PATH);
    if (numcopied >0)
    {
        strcpy(ExeName,PathName);
        returnvalue = TRUE;
    }
    return(returnvalue);

}

