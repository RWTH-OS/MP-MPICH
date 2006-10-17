/* $Id$ */

#ifndef _856745_MULTIDEVICE_H
#define _856745_MULTIDEVICE_H

extern int MPID_selected_primary_device;

#ifndef WIN32
extern int *MPID_SecondaryDevice_grank_to_devlrank;
extern int MPID_SecondaryDevice_devsize;
extern int MPID_SecondaryDevice_argc;
extern char **MPID_SecondaryDevice_argv;
extern int MPID_SecondaryDevice_type;
extern char MPID_SecondaryDevice_cmdline[300];

int MPID_GetDeviceNbr( int, int * );
void *MPID_GetInitMsgPassPt( int, int * );

#ifdef META
int MPID_buildRankMappingForSecondaryDevice( void );
int MPID_buildSecondaryDeviceArgs( int, char *** );
#endif

#endif /* !WIN32 */

#endif /* _856745_MULTIDEVICE_H */
