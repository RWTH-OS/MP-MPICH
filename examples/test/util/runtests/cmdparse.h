#include <Windows.h>

struct Creds {
	char Name[50];
	char Domain[50];
	char Password[128];
};

#define DISPLAY_DIFFERENT 1
#define DISPLAY_ALL 2
#define DISPLAY_PASSED 4

#define FIND_PROJECTS 32
#define EACH_EXE 64

#define DEL_CMP 512
#define DEL_TXT 1024
#define DEL_EQUAL 2048

#define SHOW_MPIEXEC_PATH 16384
#define SHOW_RUNTESTS_VERSION 32768

void ParseCommandline(int *ArgsEnd,char **argv,Creds* UserInfo, DWORD& flags,char ** WDir);
int Usage(void);

