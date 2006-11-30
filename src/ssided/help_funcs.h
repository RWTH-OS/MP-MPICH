/* $Id$ */

#ifndef MPI_SSIDE_HELP_FUNCS_H
#define MPI_SSIDE_HELP_FUNCS_H


/*
 * test wether two given datatypes are equal.
 * we should do a more sophisticated test
 */
#define MPIR_TEST_DTYPES_EQUAL(dtype1,dtype2)	\
					((dtype1)->self == (dtype2)->self)


#define MIN(a,b)	((a) < (b) ? (a) : (b))


#define MPIR_WIN_RETURN(winptr,error_code,name) \
				return (error_code) ? \
					MPIR_ERROR((winptr)->comm,(error_code),(name)) :\
					error_code

#define MPIR_WIN_NULL_RETURN(name) \
				return MPIR_ERROR(NULL, MPI_ERR_WIN, (name))

#endif	/* MPI_SSIDE_HELP_FUNCS_H */


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
