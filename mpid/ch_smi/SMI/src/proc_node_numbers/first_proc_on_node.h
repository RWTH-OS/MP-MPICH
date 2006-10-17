/* $Id: first_proc_on_node.h,v 1.1 2004/03/19 22:14:18 joachim Exp $ */

#ifndef _FIRST_PROC_ON_NODE_H_
#define _FIRST_PROC_ON_NODE_H_

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************************/
/*** Returns the process rank of the process with the lowest rank residing on  ***/
/*** the specified node; Library internal function, it is assumed that the     ***/
/*** parameter is valid.                                                       ***/
/*********************************************************************************/
int _smi_first_proc_on_node(int);

/*********************************************************************************/
/*** Returns the process rank of the process with the hoghest rank residing on ***/
/*** the specified node; Library internal function. No error checking, it is   ***/
/*** assumed that the  parameter is valid.                                     ***/
/*********************************************************************************/
int _smi_last_proc_on_node(int );

#ifdef __cplusplus
}
#endif


#endif
