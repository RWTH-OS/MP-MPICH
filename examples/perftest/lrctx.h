/*   $Id: lrctx.h,v 1.1 2000/04/12 16:19:28 joachim Exp $ */

#ifndef _LRCTX
#define _LRCTX

typedef struct {
    /* Control over the number of tests */
    int    minreps,       /* At least this many */
	   maxreps,       /* At most this many */
	   NatThresh;     /* This many > current min terminates test ... */
    double repsThresh;    /* within this fraction of current min */
    /* Results for Linear Regression Analysis */
    double sumtime,
	   sumlen,
	   sumlen2,       /* Sum of the squares of the length */
	   sumlentime,    /* Sum of the squares of the length */
	   sumtime2;      /* Sum of the squares of the time */
    int    ntest;         /* Number of tests */
    } LRctx;

extern LRctx *LRCreate();
extern double LRRunSingleTest();
extern void LRDestory();
#endif
