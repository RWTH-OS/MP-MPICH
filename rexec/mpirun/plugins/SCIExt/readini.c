#include "readini.h"

int ReadIniFile(char* szFileName, SciInfo_t *pSciInfo) {
    FILE* fpFile = fopen(szFileName,"r");
    char szBuffer[BUFFSIZE];
    char szTok[BUFFSIZE];
    int i;

    memset(pSciInfo, 0, sizeof(SciInfo_t));

    if (fpFile==NULL) {
	return ERR_SCIINFO_NOTFOUND;
    }

    pSciInfo->NumAdapter = 1;

    while (fgets(szBuffer, BUFFSIZE-1, fpFile)) {
	/* 
	   Token must be on position 0 followed by Parameter, rest of line is
	   assumed to be empty. No fault tolerance!
	*/ 
	
	if (strstr(szBuffer,TOK_NUMADAPTER) == szBuffer) {
	    pSciInfo->NumAdapter = atoi(szBuffer + strlen(TOK_NUMADAPTER));
	}
	
	for (i=0; i<pSciInfo->NumAdapter; i++) {
	    sprintf(szTok,"%s[%d]=",TOK_SCIID,i);
	    if (strstr(szBuffer,szTok) == szBuffer) {
		pSciInfo->SciId[i] = atoi(szBuffer + strlen(szTok));
	    }

	    sprintf(szTok,"%s[%d]=",TOK_SCIRING,i);
	    if (strstr(szBuffer, szTok) == szBuffer) {
		pSciInfo->SciRing[i] = atoi(szBuffer + strlen(szTok));
	    }
	}
    }
    
    return ERR_SCIINFO_OK;
}

void print_usage(char* progname) {
    printf("usage: %s <sciinifile>\n",progname);
}

int main (int argc, char* argv[]){
    SciInfo_t SciInfo;
    int i;
    
    if (argc != 2 ) {
	print_usage(argv[0]);
	exit(-1);
    }
   

    if(ReadIniFile(argv[1], &SciInfo) != ERR_SCIINFO_OK) {
	printf("The file %s was not found!\n",argv[1]);
    }
    else {
	for (i=0;i<SciInfo.NumAdapter;i++) {
	    printf("Adapter%d SciId=%d, SciRing=%d\n", i, SciInfo.SciId[i], SciInfo.SciRing[i]);
	}
    }
   
    return 0;
}
