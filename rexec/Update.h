
#ifndef __UPDATE_H__
#define __UPDATE_H__

#define MAX_BUFFER_SIZE 8192
#define MAGIC_UPDATE_STRING "12UpdateYourself13"

// This struct is used by the client to keep track of the state	
typedef struct	{
    HANDLE hFile;
    byte *buffer;
    DWORD bufSize;
} pipe_state;



#endif