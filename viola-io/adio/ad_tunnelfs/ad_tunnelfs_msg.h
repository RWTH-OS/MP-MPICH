/**************************************************************************
* TunnelFS                                                                * 
***************************************************************************
*                                                                         * 
* Copyright (c) 2005 Forschungszentrum Juelich GmbH, Germany              * 
* Copyright (c) 2005 University of Applied Science Bonn-Rhein-Sieg,       *
*     Germany                                                             *
*                                                                         * 
* See COPYRIGHT notice in base directory for details                      * 
**************************************************************************/
/**************************************************************************
* File:         ad_tunnelfs_msg.h                                         * 
* Description:  Provide standard message handling routines                * 
*                                                                         * 
* Author(s):    Marc-Andre Hermanns <m.a.hermanns@fz-juelich.de>          * 
* Last change:                                                            * 
*                                                                         * 
* Changelog:                                                              *
**************************************************************************/
#ifndef AD_TUNNELFS_MSG_H
#define AD_TUNNELFS_MSG_H

#define TUNNELFS_NEXT_MSG_ID    ++tunnelfs_msg_id
#define TUNNELFS_CURR_MSG_ID      tunnelfs_msg_id

#define TUNNELFS_CHECK_MSG_ID(send_id, recv_id) \
    if (send_id != recv_id) \
        fprintf(stderr, "AD_TUNNELFS: reply not matching message! send_id=%i recv_id=%i\n", send_id, recv_id);

extern int tunnelfs_msg_id;

#define tunnelfs_msg_get_reply(buf, size, rcvd, src, id) \
    tunnelfs_msg_get_reply_x(buf, size, rcvd, src, id, __FILE__, __LINE__)
#define tunnelfs_msg_get_variables(buf, size, position, rcvd) \
    tunnelfs_msg_get_variables_x(buf, size, position, rcvd, __FILE__, __LINE__)
#define tunnelfs_adjust_buffer(buf, size, min) \
    tunnelfs_adjust_buffer_x(buf, size, min, __FILE__, __LINE__)
#define tunnelfs_msg_send_init(buf, size, argc, argv) \
    tunnelfs_msg_send_init_x(buf, size, argc, argv, __FILE__, __LINE__)
#define tunnelfs_msg_send_end() \
    tunnelfs_msg_send_end_x(__FILE__, __LINE__)


/**
 * Receive reply to a specific message
 * @param reply_buf Buffer for receive operation
 * @param reply_buf_size Size of buffer
 * @param rcvd Number of bytes received
 * @param source Source of receive operation
 * @param msg_id Tunnelfs message id
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_get_reply_x(void **reply_buf, int *reply_buf_size,
                              int *rcvd, int source, int msg_id, char *file,
                              int line);

/**
 * Unpack variable from buffer
 * @param buf Message buffer
 * @param buf_size Size of buffer
 * @param position Reference to position indicator
 * @param rcvd Number of bytes received
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_get_variables_x(void *buf, int buf_size, int *position, int
                                  recvd, char *file, int line);

/**
 * Reallocate a buffer only if necessary
 * @param buf Reference to buffer pointer
 * @param buf_size Reference to size of buffer
 * @param min Required size of buffer
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_adjust_buffer_x(void **buf, int *buf_size, int min, char *file,
                              int line);

/**
 * Send initial message to main server
 * @param buf Reference to message buffer pointer
 * @param buf_size Reference to size of buffer
 * @param argc Number of arguments in argument vector
 * @param argv Vector of character arrays holding arguments 
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_send_init_x(void **buf, int *buf_size, int argc, char
                              **argv, char *file, int line);

/**
 * Send final message to main server
 * @param file name of the file the function is called from
 * @param line line in the file the function is called on
 */
void tunnelfs_msg_send_end_x(char *file, int line);

#endif
