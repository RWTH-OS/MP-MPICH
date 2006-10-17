/* $Id$
   this is a powerful memcpy benchmark for SCI connected systems */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "smi.h"

#define DEBUG 0
#if DEBUG
#define DNOTICE(a) fprintf(stderr, a);
#define DNOTICEI(a, j) fprintf(stderr, a, j);
#else
#define DNOTICE(a) 
#define DNOTICEI(a, j)
#endif

#if defined(_WIN32) && defined(DO_MMX_COPY)
#ifndef DO_MEMCPY
#define DO_MEMCPY
#endif
#define memcpy _smi_mmx_memcpy
#endif

#define LOG_FILE_NAME "membench.out"

char usage[] = "-t <int>|-n <int>|-s <int>|-i <int>|-e <int>|-c <check_type>|-m <adapter_mode>|-d <adapter>|-l|-b|-a|-q|-w|-r|-f|-o";

#define MB_DEF_SSIZE 64
#define MB_DEF_ESIZE (512*1024)
#define MB_DEF_TRIES (128*1024)


/* these are internal SMI symbols - we know what we are doing */
extern int _smi_my_proc_rank;
extern int _smi_nbr_procs;
extern int _smi_mpi_rank;
extern int _smi_mcDMAReadEnabled;


int _smi_ll_bcast(int *buffer, int count, int sender_rank, int my_rank);

char check_usage[] = "-c < nocheck | smi_only | verify_unchecked | verify_checked | fail_counters >";
char adapter_usage[] = "-m < SMI_ADPT_DEFAULT | SMI_ADPT_CYCLIC | SMI_ADPT_SMP | SMI_ADPT_IMPEXP >";

#define MB_CHECK_SZ_SIZE 6
char check_sz[MB_CHECK_SZ_SIZE][64] = {
  "nocheck",
  "smi_only",
  "verify_unchecked",
  "verify_checked",
  "verify_details",
  "fail_counters"
};

typedef enum check_t_ {
  check_nocheck,
  check_smi_only,
  check_verify_unchecked,
  check_verify_checked,
  check_verify_details,
  check_fail_counters,
  check_none
} check_t;

#define MB_ADPTMODE_SZ_SIZE 4
char adpt_mode_sz[MB_ADPTMODE_SZ_SIZE][64] = {
  "SMI_ADPT_DEFAULT",
  "SMI_ADPT_CYCLIC",
  "SMI_ADPT_IMPEXP",
  "SMI_ADPT_SMP"
};

typedef enum adpt_mode_t_ {
  adpt_default,
  adpt_cyclic,
  adpt_impexp,
  adpt_smp,
  adpt_none
} adpt_mode_t;

typedef struct mb_opt_t_ {
  int iNrOfThreads; 		/* options: t */
  int iNrOfTries;		/* options: n */
  int iStartSize;		/* options: s */
  int iEndSize;			/* options: e */
  int iIncrement;		/* options: i */
  int iFlags;			/* options: q,f,c */
  int bWriteLocalToRemote;	/* options: w,r */
  int bLocalOnly;		/* options: l */
  int bBidirectional;		/* options: b */
  int bAsynch;			/* options: a */
  int bVerify;			/* options: c */
  int bCheckSequence;           /* options: c */
  int bCheckDetails;            /* options: c */
  int bOriginalOrder;     	/* oprions: o */
  int bUseDifferentAdapter;     /* options: d */
  int iAdapterMode;             /* optioms: m */
  char szAdapterConf[256];      /* options: d */
} mb_opt_t;

typedef struct time_data_t_ {
  double time;
  double latency;
  double bandwidth;
  int byte_fails;
  int real_fails;
  int detected_fails;
  int detect_mismatch_safe;
  int detect_mismatch_critical;
} time_data_t;

typedef struct thread_data_t_ {
  char* dst;
  char* src;
  int size;
  int nroftries;
  int nrofthreads;
  int bVerify;
  int bCheckSequence;
  int bCheckDetails; 
  int flags;
  double time;
  double latency;
  int byte_fails;
  int real_fails;
  int detected_fails;
  int detect_mismatch_safe;
  int detect_mismatch_critical;
} thread_data_t;

static void reset_thread_data(thread_data_t* pData){
  pData->time = 0;
  pData->latency = 0;
  pData->byte_fails = 0;
  pData->real_fails = 0;
  pData->detected_fails = 0;
  pData->detect_mismatch_safe = 0;
  pData->detect_mismatch_critical = 0; 
}

static thread_data_t get_average(thread_data_t* pData, int num) {
  thread_data_t Temp;
  int i;

  reset_thread_data(&Temp);

  Temp.nroftries=0;
  Temp.size = 0;

  for (i=0;i<num;i++) {
    Temp.size += pData[i].size;
    Temp.nroftries += pData[i].nroftries;
    Temp.time += pData[i].time;
    Temp.latency += pData[i].latency;
    Temp.real_fails+=pData[i].real_fails;
    Temp.detected_fails+=pData[i].detected_fails;
    Temp.byte_fails+= pData->byte_fails;
    Temp.detect_mismatch_critical += pData->detect_mismatch_critical;
    Temp.detect_mismatch_safe += pData->detect_mismatch_safe;
  }

  Temp.size /= num;
  /* The Bandwidth is additive though no nee to divide nroftries */
  /* Temp.nroftries /= num; */
  Temp.time /= (double) num;
  Temp.latency /= (double) num;

  return(Temp);
}

static double get_bandwidth (thread_data_t* pData) 
{
  return (
	  (
	   ( (double)(pData->size * pData->nroftries) ) /(pData->time + pData->latency) 
	   ) / ((double)(1024*1024))
	  );
}

static void* smi_memcpy_thread(void* pVoid)
{
  thread_data_t* pData = (thread_data_t*) pVoid;
  int i,j;
  double t1,t2;
  int err;
  int df=0,rf=0;

  reset_thread_data(pData);

  t1 = SMI_Wtime();
  for(j=0; j<pData->nroftries;j++) {
    if (pData->bVerify) {
      for(i=0; i < pData->size; i++) {
	pData->src[i] = (i % 128);
	pData->dst[i] = 0;
      }
    }

#ifndef DO_MEMCPY
    if(pData->bCheckSequence) {
      SMI_Check_transfer(0);
      df=0;
    }
    err=SMI_Memcpy(pData->dst,pData->src,pData->size,pData->flags);
    if (err!=SMI_SUCCESS) {
      fprintf(stderr,"SMI_Memcpy failed! (err=%d)\n",err);
      SMI_Abort(-1);
    }
    if(pData->bCheckSequence)
      if(SMI_Check_transfer(0) != SMI_SUCCESS){
	df++;
        pData->detected_fails++;
      }
    
#else
    //usleep(1000);
    
    memcpy(pData->dst,pData->src,pData->size);
#endif
   
    if (pData->bVerify) {
      if(pData->nrofthreads==1)
	SMI_Barrier();
      rf = 0;
      for(i=0;i<pData->size;i++) {
	if (pData->dst[i] != pData->src[i]) {
	  if (pData->bCheckDetails) {
	    fprintf(stderr,"SMI_Memcpy - Read/Write ERROR!!! (src: %d   -   dst: %d\n",pData->src[i],pData->dst[i]);
	  }
	  pData->byte_fails++;
	  rf=1;
	  break;
	}
      }
      if(pData->bCheckSequence)
	if(SMI_Check_transfer(0) != SMI_SUCCESS){
	  df++;
	  pData->detected_fails++;
	}
      if (rf==1)  
	pData->real_fails++; 
      if ((rf==1)&&(df==0))
	pData->detect_mismatch_critical++;
      if ((rf==0)&&(df>0))
	pData->detect_mismatch_safe++; 
      if(pData->nrofthreads==1)
	SMI_Barrier();
    }
   
  }
  t2 = SMI_Wtime();
  
  pData->time = t2-t1;

  return(NULL);
}

static void* smi_imemcpy_thread(void* pVoid)
{
  thread_data_t* pData = (thread_data_t*) pVoid;
  int i,j;
  double t1,t2,t3,t4,t5;
  smi_memcpy_handle h=NULL;
  int err;
  int rf=0,df=0;
 
  reset_thread_data(pData);

  t1 = SMI_Wtime();
  for(j=0,t5=0; j<pData->nroftries;j++) {
    
    if (pData->bVerify) {
      rf=0;
      for(i=0; i < pData->size; i++) {
	pData->src[i] = (i % 256);
	pData->dst[i] = 0;
      }
    }

    err=SMI_Imemcpy(pData->dst,pData->src,pData->size,pData->flags,&h);
    if (err != SMI_SUCCESS) {
      fprintf(stderr,"SMI_Imemcpy failed with err= %d\n",err);
      SMI_Abort(-1);
    }
    
    t3 = SMI_Wtime();
    SMI_Memwait(h);
    t4 = SMI_Wtime(); 
    t5 += t4-t3;
  
    if(pData->bCheckSequence) {
      if(SMI_Check_transfer(0) != SMI_SUCCESS) {
        pData->detected_fails++;
	df=1;
      }
      else
	df=0;
    }
    if (pData->bVerify) {
      if(pData->nrofthreads==1)
	SMI_Barrier();
      for(i=0;i<pData->size;i++) {
	if (pData->dst[i] != pData->src[i]) {
	  if (pData->bCheckDetails) {
	    fprintf(stderr,"*** SMI_Memcpy - read/write error (src: %d - dst: %d\n",pData->src[i],pData->dst[i]);
	  }
	  pData->byte_fails++;
	  rf++;
	  break;
	}
      }
      if (rf>0)  
	pData->real_fails++; 
      if ((rf>0)&&(df==0))
	pData->detect_mismatch_critical++;
      if ((rf==0)&&(df>0))
	pData->detect_mismatch_safe++;
      if(pData->nrofthreads==1)
	SMI_Barrier();
    }
  }
  t2 = SMI_Wtime();
    
  pData->time = t2-t1;
  pData->latency = t2-t1-t5;
 
  return(NULL);
}


static time_data_t test_at_size(char* dst, char* src, int size, mb_opt_t optArgs)
{
  int i;
  time_data_t RetVal;
  pthread_attr_t scope_system_attr;
  thread_data_t* pData = (thread_data_t*) malloc(optArgs.iNrOfThreads*sizeof(thread_data_t));
  pthread_t* pThreadId = (pthread_t*)
    malloc(optArgs.iNrOfThreads*sizeof(pthread_t));
  void* (*thread_routine)(void*) = smi_memcpy_thread;

  if (optArgs.bAsynch == 1) 
    thread_routine = smi_imemcpy_thread;
   
  pthread_attr_init(&scope_system_attr);
  pthread_attr_setscope(&scope_system_attr, PTHREAD_SCOPE_SYSTEM);
    
  for(i=0;i<optArgs.iNrOfThreads;i++) {
    pData[i].dst = dst + size*i;
    pData[i].src = src + size*i;
    pData[i].size = size;
    pData[i].nroftries = optArgs.iNrOfTries;
    pData[i].nrofthreads = optArgs.iNrOfThreads;
    pData[i].bVerify = optArgs.bVerify;
    pData[i].bCheckSequence = optArgs.bCheckSequence;
    pData[i].bCheckDetails = optArgs.bCheckDetails;
    pData[i].flags = optArgs.iFlags;
    pData[i].time = 0;
    pData[i].latency = 0;
    
    if (pthread_create(&(pThreadId[i]), &scope_system_attr, thread_routine, 
		       (void*)(pData+i) ) != 0) {
      fprintf(stderr, "pthread_create() failed\n");
      SMI_Abort(-1);    
    }
  }
  
  for(i=0;i<optArgs.iNrOfThreads;i++) {
    pthread_join(pThreadId[i],NULL);
  }

  *pData = get_average(pData,optArgs.iNrOfThreads);
  RetVal.time = pData->time / (double) pData->nroftries;
  RetVal.latency = pData->latency / (double) pData->nroftries;
  RetVal.bandwidth = get_bandwidth(pData);
  RetVal.real_fails = pData->real_fails;
  RetVal.detected_fails = pData->detected_fails;
  RetVal.byte_fails = pData->byte_fails;
  RetVal.detect_mismatch_safe = pData->detect_mismatch_safe;
  RetVal.detect_mismatch_critical = pData->detect_mismatch_critical;

  free(pData);
  free(pThreadId);

  return(RetVal);
}

static void default_opt(mb_opt_t* pOpt)
{
   pOpt->iNrOfThreads = 1; 
   pOpt->iNrOfTries = MB_DEF_TRIES;
   pOpt->iStartSize = MB_DEF_SSIZE;
   pOpt->iEndSize = MB_DEF_ESIZE;
   pOpt->iIncrement = 0;
   pOpt->iFlags = SMI_MEMCPY_NOVERIFY;
   pOpt->bWriteLocalToRemote = 1;
   pOpt->bLocalOnly = 0;
   pOpt->bBidirectional = 0;
   pOpt->bAsynch = 0;
   pOpt->bVerify = 0; 
   pOpt->bOriginalOrder = 0;
   pOpt->bUseDifferentAdapter = 0;
   pOpt->iAdapterMode = SMI_ADPT_DEFAULT;
   strcpy(pOpt->szAdapterConf,"");
}

#ifdef WIN32
extern int opterr,optind,optopt;
extern char *optarg;
#endif

static void reset_get_opt( void ) 
{
  opterr = 1;
  optind = 1;
  optopt = 0;
  optarg = 0; 
}

static int get_args(int argc, char** argv, mb_opt_t* pArgs)
{
  char c;
  int optM=0,optD=0,optT=0,optN=0,optS=0,optI=0,optE=0,optL=0,optB=0,optA=0,optQ=0,optW=0,optR=0,optF=0,optC=0,optO=0;
  int iTemp;
 
  default_opt(pArgs);
  
  while((c = getopt(argc, argv, "t:n:s:i:e:c:d:m:lbaqwrfo")) != EOF)
    switch(c) {
    case 'd':
      DNOTICE("Option -d found");
      pArgs->bUseDifferentAdapter = 1;
      strcpy(pArgs->szAdapterConf,optarg);
      optD++;
      break;

    case 't':
      DNOTICE("Option -t found");
      pArgs->iNrOfThreads = atoi(optarg);
      DNOTICEI("iNrOfThreads has been set to",pArgs->iNrOfThreads);
      optT++;
      break;
 
    case 'n':
      DNOTICE("Option -n found");
      pArgs->iNrOfTries = atoi(optarg);
      DNOTICEI("NrOfTries has been set to",pArgs->iNrOfTries);
      optN++;
      break;
 
    case 's': 
      DNOTICE("Option -s found");
      pArgs->iStartSize = atoi(optarg);
      DNOTICEI("iStartSize has been set to",pArgs->iStartSize);
      optS++;
      break;
 
    case 'i':
      DNOTICE("Option -i found");
      pArgs->iIncrement = atoi(optarg);
      DNOTICEI("iIncrement has been set to",pArgs->iIncrement); 
      optI++;
      break;
 
    case 'e':
      DNOTICE("Option -e found");
      pArgs->iEndSize = atoi(optarg);
      DNOTICEI("iEndSize has been set to",pArgs->iEndSize);
      optE++;
      break;
 
    case 'l':
      DNOTICE("Option -l found");
      pArgs->bLocalOnly = 1;
      optL++;
      break;
 
    case 'b':
      DNOTICE("Option -b found");
      pArgs->bBidirectional = 1;
      optB++;
      break;
      
    case 'a':
      DNOTICE("Option -a found");
      pArgs->bAsynch = 1;
      optA++;
      break;
      
    case 'q':
      DNOTICE("Option -q found");
      pArgs->iFlags = pArgs->iFlags | SMI_MEMCPY_ENQUEUE;
      optQ++;
      break;
 
    case 'w':
      DNOTICE("Option -w found");
      pArgs->bWriteLocalToRemote = 1;
      optW++;
      break;

    case 'r':
      DNOTICE("Option -r found");
      pArgs->bWriteLocalToRemote = 0;
      optR++;
      break;

    case 'f':
      DNOTICE("Option -f found");
      pArgs->iFlags = pArgs->iFlags | SMI_MEMCPY_FORCE_DMA;
      optF++;
      break;

    case 'o':
      DNOTICE("Option -o found");
      pArgs->bOriginalOrder = 1;
      optO++;
      break;
      
    case 'm':
      DNOTICE("Option -m found");
      for(iTemp = 0; iTemp < MB_ADPTMODE_SZ_SIZE; iTemp++) {
	if (strcmp(optarg, adpt_mode_sz[iTemp]) == 0)
	  break;
      }
      
      switch((adpt_mode_t)iTemp) {
      case adpt_default:
	pArgs->iAdapterMode = SMI_ADPT_DEFAULT;
	break; 
      case adpt_cyclic:
	pArgs->iAdapterMode = SMI_ADPT_CYCLIC;
	break;
      case adpt_smp:
	pArgs->iAdapterMode = SMI_ADPT_SMP;
       break; 
      case adpt_impexp:
	pArgs->iAdapterMode = SMI_ADPT_IMPEXP;
	break;
      default:
	DNOTICE("Illegal Adapter Mode found");
	fprintf(stderr,"usage: %s\n",adapter_usage);
	SMI_Abort(-1);
	break;
      } 
      break;
      
    case 'c':
      DNOTICE("Option -c found");
      for(iTemp = 0; iTemp < MB_CHECK_SZ_SIZE; iTemp++) {
	if (strcmp (optarg, check_sz[iTemp]) == 0)
	  break;
      }
      
      switch((check_t)iTemp) {
      case check_nocheck:
	 pArgs->iFlags = ((pArgs->iFlags)|(SMI_MEMCPY_NOVERIFY));
	 pArgs->bVerify=0;
	 pArgs->bCheckSequence=0;
	 break;
	 
      case check_smi_only:  
	pArgs->iFlags = ((pArgs->iFlags) & (~SMI_MEMCPY_NOVERIFY));
	pArgs->bVerify=0;
	pArgs->bCheckSequence=0;
	break;
      case check_verify_unchecked:
	 pArgs->iFlags = ((pArgs->iFlags)|(SMI_MEMCPY_NOVERIFY));
	 pArgs->bVerify=1;
	 pArgs->bCheckSequence=0;
	 pArgs->bCheckDetails=0; 
	 break;
      case check_verify_checked:
	pArgs->iFlags = ((pArgs->iFlags) & (~SMI_MEMCPY_NOVERIFY));
	pArgs->bVerify=1;
	pArgs->bCheckSequence=0;
	pArgs->bCheckDetails=0; 

	break;
      case check_verify_details:
	pArgs->iFlags = ((pArgs->iFlags) | SMI_MEMCPY_NOVERIFY);
	pArgs->bVerify=1;
	pArgs->bCheckSequence=0;
	pArgs->bCheckDetails=1; 
	break;
      case check_fail_counters:
	pArgs->iFlags = ((pArgs->iFlags)|(SMI_MEMCPY_NOVERIFY));
	pArgs->bVerify=1;
	pArgs->bCheckSequence=1;
	pArgs->bCheckDetails=0; 
	break;
      default:
	DNOTICE("Illegal Check Option found");
	fprintf(stderr,"usage: %s\n",check_usage);
	SMI_Abort(-1);
	break;
      } 
      
      pArgs->iFlags = ((pArgs->iFlags) & (~SMI_MEMCPY_NOVERIFY));
      optC++;
      break;
      
    default:
      DNOTICE("Illegal Option found");
      fprintf(stderr,"usage: %s %s\n",argv[0],usage);
      SMI_Abort(-1);
    }
 
  if (optC>1) { 
    fprintf(stderr,"There is more than one -c option\n");
    SMI_Abort(-1);
  }

  if (optD>1) { 
    fprintf(stderr,"There is more than one -d option\n");
    SMI_Abort(-1);
  }
  
  if (optC<1) {
    pArgs->iFlags = ((pArgs->iFlags)|(SMI_MEMCPY_NOVERIFY));
    pArgs->bVerify=0;
    pArgs->bCheckSequence=0;
  }
  
  if (optM>1) {
    fprintf(stderr,"There is more than one -m option\n");
    SMI_Abort(-1);
  }

  if ((optM>0) && (optD>0)) {
    fprintf(stderr,"-d and -m option cannot be used at the same time\n");
    SMI_Abort(-1);
  }
  
  if((optW>0)&&(optR>0)) {
    fprintf(stderr,"Are you silly?\nWhat do you want now, READ (-r) OR WRITE (-w) ?\n");
    SMI_Abort(-1);
  }
  
  if((optA>0)&&(optL>0)) {
    fprintf(stderr,"Asynchronous local benchmark is not possible\n");
    SMI_Abort(-1);
  }

#if 0
  if((optA>0)&&(optR>0)&&(mcDMAReadEnabled==FALSE)) {
    fprintf(stderr,"Asynchronous read benchmark is not implemented\n");
    SMI_Abort(-1);
  } 
#endif

  if (pArgs->iNrOfThreads < 1) {
    fprintf(stderr,"At least one thread required\n");
    SMI_Abort(-1);
  }

  reset_get_opt();

  return(0);
}

int main(int argc, char* argv[])
{
  FILE* fpFile;
  mb_opt_t Args;
  mb_opt_t PreconArgs;
  time_data_t times;
  smi_region_info_t regdesc;
  smi_error_t error;
  char *local, *remote, *swap, *dummy;
  char fname[256], szVersionName[256], szHostName[256];
  int localId, remoteId, segsize, iInput, dummyId;
  int i,j, err, byte_err, be;
  int myid, mysmiid, numprocs; 
  size_t size;
  
  SMI_Init(&argc,&argv);
  SMI_Proc_size(&numprocs);
  SMI_Proc_rank(&mysmiid);
  size = 256;
  SMI_Get_node_name(szHostName, &size);
  SMI_Query(SMI_Q_SCI_API_VERSION,256,szVersionName);
  
  gethostname(szHostName,256);

  SMI_Debug(0);
  SMI_Watchdog(0); 

  if ((numprocs&1) != 0) {
    fprintf(stderr,"please use a even number of processes!\n");
    SMI_Abort(-1);
  }
  
  get_args(argc,argv,&Args);

  if (Args.bUseDifferentAdapter == 1)
    if (strlen(Args.szAdapterConf) != numprocs) {
      fprintf(stderr,"the number of digits behind -d option should match the number of processes!\n");
      SMI_Abort(-1);
    }

  if (Args.bOriginalOrder==1)
    myid = _smi_mpi_rank; 
  else
    myid = mysmiid;

  if ((numprocs > 2) || (Args.bBidirectional == 1))
    sprintf(fname,"%s.%d",LOG_FILE_NAME,myid);
  else 
    strcpy(fname,LOG_FILE_NAME);

  if (((myid&1)==0) || (Args.bBidirectional == 1))
    fpFile = fopen(fname,"w");
  
  segsize = Args.iEndSize * Args.iNrOfThreads * (Args.bBidirectional+1);

  for (i=0; i<(numprocs/2);i++) {
    iInput = mysmiid;
    _smi_ll_bcast(&iInput,1,(i<<1),myid);
    dummy = 0;
    dummyId = 0;

    memset(&regdesc,0,sizeof(regdesc));
    if (Args.bUseDifferentAdapter == 1)
      regdesc.adapter = Args.szAdapterConf[myid]-'0';
    else
      regdesc.adapter = Args.iAdapterMode;
    regdesc.size = segsize;
    regdesc.owner = iInput;
   
    err = SMI_Create_shreg(SMI_SHM_UNDIVIDED, &regdesc, &dummyId, (void **) &dummy);
    if (err != SMI_SUCCESS) {
      fprintf(stderr, "[%d] not enough shared memory available\n", iInput);
      SMI_Abort(-1);
    }
    if ((myid>>1) == i) {
      if ((myid&1) == 0) {
	  localId = dummyId;
	  local = dummy;
      } else {
	  remoteId = dummyId;
	  remote = dummy;
      }
    }  

    iInput = mysmiid;
    _smi_ll_bcast(&iInput,1,(i<<1)+1,myid);
    dummy = 0;
    dummyId = 0;
    
    memset(&regdesc,0,sizeof(regdesc));
    if (Args.bUseDifferentAdapter == 1)
      regdesc.adapter = Args.szAdapterConf[myid]-'0';
    else
      regdesc.adapter = Args.iAdapterMode;
    regdesc.size = segsize;
    regdesc.owner = iInput;

    SMI_Create_shreg(SMI_SHM_UNDIVIDED, &regdesc, &dummyId, (void **) &dummy); 
    if (err!=SMI_SUCCESS) {
      fprintf(stderr, "[%d] not enough shared memory available\n",err);
      SMI_Abort(-1);
    }
    if ((myid>>1) == i) {
      if((myid&1)==0) {
	remoteId = dummyId;
	remote = dummy; 
      }
      else {
	localId = dummyId;
	local = dummy;	
      }
    } 
  } 

  if (Args.bLocalOnly == 1) {
    printf("performing LOCAL benchmark\n");
    remote = local;
    local = (char*)malloc(Args.iEndSize);
  }

  if (Args.bWriteLocalToRemote == 0) {
    printf("performing READ benchmark\n");
    swap = remote;
    remote = local;
    local = swap;
  }

  if (((myid&1) == 0)||(Args.bBidirectional==1)) {
    if (Args.bAsynch==1) {
      printf("Preconnect DMA-Transfer\n");
      default_opt(&PreconArgs);
      PreconArgs.iNrOfTries = 1;
      PreconArgs.iFlags = SMI_MEMCPY_FORCE_DMA;
      PreconArgs.bAsynch = 1;
      test_at_size(remote,local,64*1024, PreconArgs);
    }
 
    fprintf(fpFile,"# This benchmark file was created on '%s' \n", szHostName); 
    fprintf(fpFile,"# ... using SCI-Api Version: \t\"%s\"\n",szVersionName);
    fprintf(fpFile,"# ... using options:         \t\"");
    for (i=0;i<argc;i++) {
      fprintf(fpFile,"%s ",argv[i]);
    }
    fprintf(fpFile,"\"\n");
    fprintf(fpFile,"\n\n");
    fprintf(fpFile,"# blocksize\titerations");
    if(Args.bCheckSequence==0)
      fprintf(fpFile,"\ttime (msec)\t\tbandwidth (MB/sec)");
    if(Args.bAsynch==1)
      fprintf(fpFile,"\tlatency (msec)");
    if(Args.bVerify==1) {
      fprintf(fpFile,"\tbyte_failes");
      fprintf(fpFile,"\tfails_per_MB");
    }
    if(Args.bCheckSequence==1)
      fprintf(fpFile,"\treal_failes\tdetected_failes\tsafe_mismatch\tcritical_mismatch");
    
    fprintf(fpFile,"\n");
    for(
      size = Args.iStartSize;
      size <= Args.iEndSize; 
      size = (Args.iIncrement == 0) ? (size<<1):(size+Args.iIncrement) ,
	Args.iNrOfTries = (Args.iIncrement == 0) ?  (Args.iNrOfTries>>1) :
	(Args.iNrOfTries-Args.iIncrement) 
      ) {

      if (Args.iNrOfTries<1)
	Args.iNrOfTries=1;
      
      times = test_at_size(remote + ((myid&1)*size*Args.iNrOfThreads),
			   local +((myid&1)*size*Args.iNrOfThreads), size, Args);
      /* print results to stdout and file */
      if (myid == 0) {
	  printf("%d byte: \t %9.4f ms\t %6.3f MB/s\n",size, times.time*1000, times.bandwidth);
	  fflush (stdout);
      }
      fprintf(fpFile,"%d\t\t%d\t",size,Args.iNrOfTries);
      if(Args.bCheckSequence==0)
	fprintf(fpFile,"\t%f\t\t%f\t",(times.time*1000),times.bandwidth);
      if (Args.bAsynch==1) {
	fprintf(fpFile,"\t%f",times.latency*1000);
      }
      if (Args.bVerify) {
	printf("%d transmissions contained wrong values\n",times.real_fails);
	fprintf(fpFile,"\t%d\t\t%f",times.byte_fails,(double)(times.byte_fails)*1024*1024/(double)(Args.iNrOfTries*size));
      }
      if (Args.bCheckSequence) {
	printf("%d transmissions would have detected as failed by SMI\n",times.detected_fails);	
	fprintf(fpFile,"\t%d\t%d\t%d\t%d",times.real_fails,times.detected_fails,
		times.detect_mismatch_safe,times.detect_mismatch_critical);
      }  
      fprintf(fpFile,"\n");
    } 
    
    fclose(fpFile);
  }
  else { 
    if((Args.bVerify==1)&&(Args.iNrOfThreads==1)) {
      fprintf(stderr,"receiver-side errorcheck:\n");
      for(
	size = Args.iStartSize;
	size <= Args.iEndSize; 
	size = (Args.iIncrement == 0) ? (size<<1):(size+Args.iIncrement) ,
	  Args.iNrOfTries = (Args.iIncrement == 0) ?  (Args.iNrOfTries>>1) :
	  (Args.iNrOfTries-Args.iIncrement) 
	) {
	err=0;
	byte_err=0;
	for(j=0;j<Args.iNrOfTries;j++) {
	  be = 0;  
	  SMI_Barrier();
	  for (i=0;i<size;i++) { 
	    if (local[i]!=(i%128)) {
	    byte_err++;
	    be=1;
	    }
	} 
	  SMI_Barrier();
	  err += be;
	}
	fprintf(stderr,"byte mismatch: %d, %d transmissions failed\n",byte_err,err);
      }
    }
    else {
      for(
	size = Args.iStartSize;
	size <= Args.iEndSize; 
	size = (Args.iIncrement == 0) ? (size<<1):(size+Args.iIncrement) ,
	  Args.iNrOfTries = (Args.iIncrement == 0) ?  (Args.iNrOfTries>>1) :
	  (Args.iNrOfTries-Args.iIncrement) 
	) {
      }
    }
  }

  SMI_Finalize();

  return(0);
}
