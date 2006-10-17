#include <stdio.h>
#include <unistd.h>

#include "smi.h"

/*
  This program can be used to test the mutex functionality of SMI. 
*/

#define LOCAL_ONLY 0x100000

#define SEGSIZE (512*1024)
#define NBR_ENTRIES 100

static int iShregId;
static int *pShreg; 
static int listLock;

/* 
   Set this to 1 to disable mutex sync. of the list. You can test if the test
   works this way :-)
*/
#define NOSYNC 0

#if NOSYNC
#define SMI_Mutex_lock(i)
#define SMI_Mutex_unlock(i)
#endif

typedef struct node_t_{
  int iValue;
  struct node_t_* pNext;
} node_t;

static void list_init(node_t** ppRoot)
{
  smi_error_t error;
  
  error = SMI_Cmalloc(sizeof(node_t), iShregId, (void **)ppRoot);
  if (error != SMI_SUCCESS) {
    fprintf(stderr, "No memory left\n");
    SMI_Abort(-1);
  }
  (*ppRoot)->pNext = NULL;
/*
  SMI_Mutex_init(&listLock);
*/
  SMI_MUTEX_INIT(&listLock, BL_MUTEX,0 /*LOCAL_ONLY*/);
  SMI_Barrier();
}

static void list_append(node_t* pRoot, int iValue)
{
  node_t* pTemp;
  smi_error_t error;
  
  error = SMI_Imalloc(sizeof(node_t), iShregId, (void **)&pTemp);
  if (error != SMI_SUCCESS) {
    fprintf(stderr, "No memory left\n");
    SMI_Abort(-1);
  }
  
  SMI_Mutex_lock(listLock);

  while(pRoot->pNext != NULL)
    pRoot = pRoot->pNext;

  /* force race conditions */
  usleep(1);

  pRoot->pNext = pTemp;
  pTemp->iValue = iValue;
  pTemp->pNext = NULL;

  SMI_Mutex_unlock(listLock);
}

static int list_print(node_t* pRoot)
{
    int i = 0;
    SMI_Mutex_lock(listLock);
    while(pRoot->pNext != NULL) {
	pRoot = pRoot->pNext;
	printf("%d ",pRoot->iValue); fflush(stdout);
	i++;
    }
    printf("\n");
    
    SMI_Mutex_unlock(listLock);
    
    return(i);
}

static void list_remove(node_t* pRoot)
{
  smi_error_t error;

  SMI_Mutex_lock(listLock);

  if(pRoot->pNext != NULL)
    list_remove(pRoot->pNext);
  
  SMI_Ifree((char*)pRoot->pNext);
  pRoot->pNext = NULL;

  SMI_Mutex_unlock(listLock);
}

int main (int argc, char *argv[]) 
{
    smi_region_info_t reginfo;
    smi_error_t error;
    int rank, size, i, nbr_entries = NBR_ENTRIES;
    node_t* pRoot;
    int count;
    
    SMI_Init(&argc, &argv);
    
    SMI_Proc_rank(&rank);
    SMI_Proc_size(&size);    
    
    /* create the shared memory region */
    SMI_Init_reginfo(&reginfo, SEGSIZE, 0, 0, SMI_ADPT_DEFAULT, 0, 0, NULL);
    error = SMI_Create_shreg(SMI_SHM_UNDIVIDED, &reginfo, &iShregId, (void **)&pShreg);
    if (error != SMI_SUCCESS) {
	fprintf(stderr,"could not create shared region (%d)\n",error);
	SMI_Abort(-1);
    }
    
    /* use this region with the memory manager for dynamic allocation */
    error = SMI_Init_shregMMU(iShregId);
    if (error != SMI_SUCCESS) {
	fprintf(stderr,"could not init shregMMU (%d)\n",error);
	SMI_Abort(-1);
    }
    
    list_init(&pRoot);
    
    for(i = 1; i <= nbr_entries; i++) {
	list_append(pRoot, i+rank*1000);
#if SPECIAL_RACE_CONDITION
	/* force raceconditions */
	usleep(i);
#endif
    }
    
    SMI_Barrier();
    
    if (rank == 0) { 
	count = list_print(pRoot);
	if (count == nbr_entries * size)
	    printf("The mutex seems to work\n");
	else
	    printf("Some entries are missing!\n");
    }

    SMI_Barrier();
    list_remove(pRoot);
    
    SMI_Free_shreg(iShregId);
    SMI_Finalize();
    return (0);
}
