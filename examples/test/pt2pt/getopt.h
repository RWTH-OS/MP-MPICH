/* $Id: getopt.h,v 1.2 2002/02/25 17:09:20 joachim Exp $ */

extern char * optarg; 
extern int    optind;  
int getopt ( int argc, char **argv, char *optstring); 
