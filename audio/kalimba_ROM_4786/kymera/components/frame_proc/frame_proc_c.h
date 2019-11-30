// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// 
// *****************************************************************************



#ifndef FRAME_PROC_C_HEADER_INCLUDED
#define FRAME_PROC_C_HEADER_INCLUDED

#include "cbuffer_c.h"


/**
 * Frame Buffer structure encapsolates a buffer linked into the frame processing
 * architecture.  
 *
 *  
 *
 * \note See http://wiki/AudioCPU/architecture/Buffers for a better explanation of this. 
 */

typedef struct
{
    unsigned *frame_ptr;               /**< pointer, to beginning of frame */
    unsigned frame_size;               /**< size of frame being processed */
    unsigned buffer_size;              /**< size of cBuffer see note (*) */
    unsigned *buffer_addr;             /**< base address of cBuffer see note (*) */   
    
    tCbuffer *cbuffer;                 /**< Pointer to associated cBuffer */
    unsigned initial_frame_size;       /**< initial frame size at start of frame processing */
    void *distribute_function;         /**< pointer to frame distribution function  */
    void *update_function;             /**< pointer to frame update function  */
    
    unsigned threshold;                /**< Threshold value for latency control feature - zero to disable */
    unsigned hysteresis;               /**< Adjustment for sample drop/insert feature */
    
    unsigned num_dropped;              /**< Number of samples dropped from cBuffer */
    unsigned num_inserted;             /**< Number of samples inserted into cBuffer */
    
    unsigned amount_to_drop;           /**< Number of samples to be dropped durring frame update */
    
    /** Note (*) The buffer_size and buffer_addr paramketers are associated with the cBuffer for
                 setting up circular buffer registers */
} tFrmBuffer;



#endif  // FRAME_PROC_C_HEADER_INCLUDED
