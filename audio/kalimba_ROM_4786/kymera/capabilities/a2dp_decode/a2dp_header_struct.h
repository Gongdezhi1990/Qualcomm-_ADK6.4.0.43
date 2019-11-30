/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
*    \file a2dp_header_struct.h
*
*     Header file for A2DP header structure
*/

#ifndef _A2DP_HEADER_STRUCT_H_
#define _A2DP_HEADER_STRUCT_H_

#include "a2dp_header_defs.h"

/****************************************************************************
Public Type Declarations
*/

/** A2DP header structure */
typedef struct A2DP_HEADER_PARAMS
{
    /** header type to strip 
     *  bit 0    - B-Frame PDU ( L2CAP Basic Mode)
     *  bit 1    - RTP header present 
     *  bit 2    - MP header present ( 1 byte in case of SBC )
     *  bit 3    - Content protection header present
     *  bit 4    - I-Frame PDU ( L2CAP ERTX, Streaming modes)( not supported now)
     *  bit 5 onwards    - unused  
     *  Other bits are reserved now.
     */
    unsigned type;

    /** Size of header to strip in bytes */
    unsigned hdr_size;

    /** L2CAP channel id valid only when L2CAP header is present */
    unsigned cid;

    /** packet length in bytes left to process in the current packet
     * Currently for SBC, integral number of frames in a single packet is 
     * expected and bytes_left being zero triggers stripping header for SBC
     * and less than 4 triggers striping header for aptX.
     */  
    unsigned bytes_left;

    /* Decode function after stripping the header */
    void (*strip_decode_frame)(void);

    /**
    * bit handling function to read bits from the buffer 
    */
    void (*get_bits)(void);

#ifdef INSTALL_OPERATOR_APTX_SHUNT_DECODER

    /** 0 if word aligned*/
    unsigned  odd_byte_pos; 

    /** data aligned and ready for decode*/ 
    unsigned data_aligned;

#endif

} A2DP_HEADER_PARAMS;


#endif  /* _A2DP_HEADER_STRUCT_H_ */
