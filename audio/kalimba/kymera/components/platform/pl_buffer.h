/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 *
 * \file pl_buffer.h
 * \ingroup platform
 *
 * Header file for kalimba buffer functions
 *
 ****************************************************************************/
#if !defined(PL_BUFFER_H)
#define PL_BUFFER_H

#include "types.h"

/* PUBLIC FUNCTION PROTOTYPES ***********************************************/

extern void circular_buffer_copyfrom(void* dest_buf, const void* src_buf_start, int src_buf_offset, int src_buf_size, int length);
extern void circular_buffer_copyto(void* dest_buf_start, int dest_buf_offset, int dest_buf_size, const void* src_buf, int length);
extern void circular_buffer_copybetween(void* dest_buf_start, int dest_buf_offset, int dest_buf_size, 
                                        const void* src_buf_start, int src_buf_offset, int src_buf_size, int length);

#endif // PL_BUFFER_H
