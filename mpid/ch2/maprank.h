#ifndef MPID_MAP_RANKS_H
#define MPID_MAP_RANKS_H

#include "adi3types.h"


#define MPID_COMM_TO_GRANK(comm,rank) (\
		((rank) > (comm)->np || (rank) < 0) ? -1 : (\
		(comm)->lrank_to_grank[(rank)]))
	
#define MPID_GRANK_TO_DRANK(rank) (\
		((rank) > MPID_devset->ndev || (rank) < 0) ? -1 : (\
		MPID_devset->dev[(rank)]->grank_to_devlrank[(rank)]))

#define MPID_COMM_TO_DRANK(comm,rank) (\
		((rank) > (comm)->np || (rank) < 0) ? -1 : (\
		MPID_devset->dev[(rank)]->grank_to_devlrank[\
		(comm)->lrank_to_grank[(rank)]]))

#define MPID_WIN_TO_DRANK(win,rank) (\
		MPID_COMM_TO_DRANK ((win)->comm,(rank)))
	
#define MPID_WIN_TO_GRANK(win,rank) (\
		MPID_COMM_TO_GRANK ((win)->comm,(rank)))

#define MPID_GRPRANK_TO_GRPRANK(group1,rank1,group2,rank2) {\
		if ((rank1) > (group1)->np || (rank) < 0)\
			(rank2) = -1;\
		else {\
			int _grp, grank=(group1)->lrank_to_grank[rank1];\
			for (_grp=0; _grp<(group2)->np; _grp++) \
				if ((group2)->lrank_to_grank[_grp] == grank) break;\
			if (_grp == (group2)->np) \
				(rank2) = -1;\
			else\
				(rank2) = _grp;\
		}}


/* the following macros are the same, but don't do the range check
 	and hence are faster, it's wise to use them, if you can be shure, that
	the ranks are OK
 */

#define MPID_COMM_TO_GRANK_FAST(comm,rank) (\
		(comm)->lrank_to_grank[(rank)])

#define MPID_GRANK_TO_DRANK_FAST(rank) (\
		MPID_devset->dev[(rank)]->grank_to_devlrank[(rank)])

#define MPID_COMM_TO_DRANK_FAST(comm,rank) (\
		MPID_devset->dev[(rank)]->grank_to_devlrank[\
		(comm)->lrank_to_grank[(rank)]])

#define MPID_WIN_TO_DRANK_FAST(win,rank) (\
		MPID_COMM_TO_DRANK_FAST ((win)->comm,(rank)))

#define MPID_WIN_TO_GRANK_FAST(win,rank) (\
		MPID_COMM_TO_GRANK_FAST ((win)->comm,(rank)))


#define MPID_GRPRANK_TO_GRPRANK_FAST(group1,rank1,group2,rank2) {\
		int _grp, grank=(group1)->lrank_to_grank[rank1];\
		for (_grp=0; _grp<(group2)->np; _grp++) \
			if ((group2)->lrank_to_grank[_grp] == grank) break;\
		if (_grp == (group2)->np) \
			(rank2) = -1;\
		else\
			(rank2) = _grp;\
		}




#endif	/* MPID_MAP_RANKS_H */



/*
 * Overrides for XEmacs and vim so that we get a uniform tabbing style.
 * XEmacs/vim will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:tw=0:ts=4:wm=0:
 */
