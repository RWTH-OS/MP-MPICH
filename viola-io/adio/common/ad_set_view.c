/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *   $Id: ad_set_view.c,v 1.1.1.1 2005/03/10 16:07:13 mhs Exp $
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"
#include "adio_extern.h"
#ifdef ROMIO_TUNNELFS
#include "ad_tunnelfs.h"
#endif

/* this used to be implemented in every file system as an fcntl.  It makes
 * deferred open easier if we know ADIO_Fcntl will always need a file to really
 * be open. set_view doesn't modify anything related to the open files.
 */
void ADIO_Set_view(ADIO_File fd, ADIO_Offset disp, MPI_Datatype etype, 
		MPI_Datatype filetype, MPI_Info info,  int *error_code) 
{
	int combiner, i, j, k, err, filetype_is_contig;
	MPI_Datatype copy_etype, copy_filetype;
	ADIOI_Flatlist_node *flat_file;
#ifdef ROMIO_TUNNELFS
    int io_server_rank = -1;

    io_server_rank = tunnelfs_server_get_file_master(fd->fd_sys);

    /* before we do anything else we sync the file with the server */
    MPI_File_sync(fd);
    
#endif
	/* free copies of old etypes and filetypes and delete flattened 
       version of filetype if necessary */

	MPI_Type_get_envelope(fd->etype, &i, &j, &k, &combiner);
	if (combiner != MPI_COMBINER_NAMED) MPI_Type_free(&(fd->etype));

	ADIOI_Datatype_iscontig(fd->filetype, &filetype_is_contig);
	if (!filetype_is_contig) ADIOI_Delete_flattened(fd->filetype);

	MPI_Type_get_envelope(fd->filetype, &i, &j, &k, &combiner);
	if (combiner != MPI_COMBINER_NAMED) MPI_Type_free(&(fd->filetype));

	/* set new info */
	ADIO_SetInfo(fd, info, &err);

        /* set new etypes and filetypes */

	MPI_Type_get_envelope(etype, &i, &j, &k, &combiner);
	if (combiner == MPI_COMBINER_NAMED) fd->etype = etype;
	else {
        /* first check, if datatype is known to the server 
         * we only need the layout of it, so we do this _before_ 
         * we make a copy */
#ifdef ROMIO_TUNNELFS        
        if (fd->file_system == ADIO_TUNNELFS)
            tunnelfs_datatype_sync(etype, io_server_rank);
#endif
        /* then we copy the datatype, as the user might free the 
         * original one */
        MPI_Type_contiguous(1, etype, &copy_etype);
        MPI_Type_commit(&copy_etype);

        /* assign the copy to the file descriptor */
  	    fd->etype = copy_etype;

#ifdef ROMIO_TUNNELFS
        /* register new type */
        if (fd->file_system == ADIO_TUNNELFS)
            tunnelfs_datatype_sync(fd->etype, io_server_rank);
#endif
    }
    
	MPI_Type_get_envelope(filetype, &i, &j, &k, &combiner);
	if (combiner == MPI_COMBINER_NAMED) 
	    fd->filetype = filetype;
	else {
        /* again, check first if datatype is known to the server */
#ifdef ROMIO_TUNNELFS
        if (fd->file_system == ADIO_TUNNELFS)
            tunnelfs_datatype_sync(filetype, io_server_rank);
#endif            
        /* then we copy the datatype, as the user might free the 
         * original one */
        MPI_Type_contiguous(1, filetype, &copy_filetype);
        MPI_Type_commit(&copy_filetype);
        
        /* assign the copy to the file descriptor */
        fd->filetype = copy_filetype;

#ifdef ROMIO_TUNNELFS
        /* register new type */
        if (fd->file_system == ADIO_TUNNELFS)
            tunnelfs_datatype_sync(fd->filetype, io_server_rank);
#endif

        ADIOI_Flatten_datatype(fd->filetype);
        /* this function will not flatten the filetype if it turns out
           to be all contiguous. */
    }

    /* now, as types are in sync with tunnelfs server, it has to be notified of
     * the change */
#ifdef ROMIO_TUNNELFS
    if (fd->file_system == ADIO_TUNNELFS)
    {
        void *buf = NULL;
        int buf_size = 0;
        int file_id = 0;
        int temp_size = 0;
        int pack_size = 0;
        int msg_id = TUNNELFS_NEXT_MSG_ID;
        int position = 0;
        MPI_Datatype my_etype = 0;
        MPI_Datatype my_ftype = 0;
        int var_id = 0;
        int recvd = 0;
        int reply_id = 0;
        int rcode = 0;
        int num_infokeys = 0;
        int num_ints = 6;
        int num_char = 0;

        if ((info != MPI_INFO_NULL))
        {
            int i;
            char info_key[MPI_MAX_INFO_KEY+1];
            
            MPI_Info_get_nkeys(info, &num_infokeys);
            for (i=0; i < num_infokeys; i++)
            {
                int val_len = 0;
                int flag = 0;
                
                MPI_Info_get_nthkey(info, i, info_key);
                MPI_Info_get_valuelen(info, info_key, &val_len, &flag);
                num_ints += 2;
                num_char += strlen(info_key) + val_len + 2;
            }
        }
        
        MPI_Pack_size(num_ints, MPI_INT, TUNNELFS_COMM_WORLD, &temp_size);
        pack_size += temp_size;

        MPI_Pack_size(num_char, MPI_CHAR, TUNNELFS_COMM_WORLD, &temp_size);
        pack_size += temp_size;

        MPI_Pack_size(1, MPI_LONG, TUNNELFS_COMM_WORLD, &temp_size);
        pack_size += temp_size;

        tunnelfs_adjust_buffer(&buf, &buf_size, pack_size);

        MPI_Pack(&msg_id, 1, MPI_INT, buf, buf_size, &position,
                 TUNNELFS_COMM_WORLD);
       /* 
        var_id = TUNNELFS_REQ_SET_VIEW;
        MPI_Pack(&var_id, 1, MPI_INT, buf, buf_size, &position, 
                 TUNNELFS_COMM_WORLD);
        */
       
        file_id = tunnelfs_file_get_id(fd);
        MPI_Pack(&file_id, 1, MPI_INT, buf, buf_size, &position,
                 TUNNELFS_COMM_WORLD);

        MPI_Pack(&disp, 1, TUNNELFS_OFFSET, buf, buf_size, &position, 
                 TUNNELFS_COMM_WORLD);

        tunnelfs_datatype_get_id(etype, &my_etype);
        MPI_Pack(&my_etype, 1, MPI_INT, buf, buf_size, &position,
                 TUNNELFS_COMM_WORLD);

        tunnelfs_datatype_get_id(filetype, &my_ftype);
        MPI_Pack(&my_ftype, 1, MPI_INT, buf, buf_size, &position, 
                 TUNNELFS_COMM_WORLD);
        
        /* Pack possible info */
        if ((info == MPI_INFO_NULL))
        {
            num_infokeys = 0;
            MPI_Pack(&num_infokeys, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);
        }
        else
        {
            int i;
            char info_key[MPI_MAX_INFO_KEY+1];
            
            MPI_Info_get_nkeys(info, &num_infokeys);
            MPI_Pack(&num_infokeys, 1, MPI_INT, buf, buf_size, &position,
                     TUNNELFS_COMM_WORLD);
            for (i=0; i < num_infokeys; i++)
            {
                int key_len = 0;
                int val_len = 0;
                int flag = 0;
                char *info_val = NULL;
                
                MPI_Info_get_nthkey(info, i, info_key);
                key_len = strlen(info_key);
                MPI_Pack(&key_len, 1, MPI_INT, buf, buf_size, &position,
                         TUNNELFS_COMM_WORLD);
                MPI_Pack(&info_key, key_len, MPI_CHAR, buf, buf_size,
                         &position, TUNNELFS_COMM_WORLD);
                
                MPI_Info_get_valuelen(info, info_key, &val_len, &flag);
                if (!flag)
                    ERR(TUNNELFS_ERR_NOT_FOUND);
                ALLOC(info_val, val_len+1);
                MPI_Info_get(info, info_key, val_len, info_val, &flag);
                if (!flag)
                    ERR(TUNNELFS_ERR_NOT_FOUND);
                MPI_Pack(&val_len, 1, MPI_INT, buf, buf_size, &position,
                         TUNNELFS_COMM_WORLD);
                if (val_len > 0)
                    MPI_Pack(info_val, val_len, MPI_CHAR, buf, buf_size,
                             &position, TUNNELFS_COMM_WORLD);
            }
        }

        MPI_Send(buf, position, MPI_PACKED, io_server_rank,
                 TUNNELFS_SET_VIEW, TUNNELFS_COMM_WORLD);

        tunnelfs_msg_get_reply(&buf, &buf_size, &recvd, 
                               io_server_rank, msg_id);
        
        position = 0;
        MPI_Unpack(buf, buf_size, &position, &reply_id, 1, MPI_INT,
                   TUNNELFS_COMM_WORLD);
        MPI_Unpack(buf, buf_size, &position, &rcode, 1, MPI_INT,
                   TUNNELFS_COMM_WORLD);

        tunnelfs_msg_get_variables(buf, buf_size, &position, recvd);

        free(buf);
    }
    
    /* ensure that all clients have the opportunity to set the file view before
     * anyone can place an io request */
    MPI_Barrier(fd->comm);

#endif /* ROMIO_TUNNELFS */

	MPI_Type_size(fd->etype, &(fd->etype_size));
	fd->disp = disp;

        /* reset MPI-IO file pointer to point to the first byte that can
           be accessed in this view. */

        ADIOI_Datatype_iscontig(fd->filetype, &filetype_is_contig);
	if (filetype_is_contig) fd->fp_ind = disp;
	else {
	    flat_file = ADIOI_Flatlist;
	    while (flat_file->type != fd->filetype) 
		flat_file = flat_file->next;
	    for (i=0; i<flat_file->count; i++) {
		if (flat_file->blocklens[i]) {
		    fd->fp_ind = disp + flat_file->indices[i];
		    break;
		}
	    }
	}
	*error_code = MPI_SUCCESS;
}
