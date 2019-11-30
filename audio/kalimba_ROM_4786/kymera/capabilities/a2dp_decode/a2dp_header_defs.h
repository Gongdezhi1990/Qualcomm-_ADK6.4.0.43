/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
*    \file a2dp_header_defs.h
*
*     Header file for A2DP header definitions
*/

/****************************************************************************
Public Type Declarations
*/
#ifndef _A2DP_HEADER_DEFS_H_
#define _A2DP_HEADER_DEFS_H_

/** A2DP strip header */
#define A2DP_STRIP_BFRAME              0x01
#define A2DP_STRIP_RTP                 0x02
#define A2DP_STRIP_MP                  0x04
#define A2DP_STRIP_CP                  0x08
#define A2DP_BFRAME_HDR_SIZE           4
#define A2DP_RTP_HDR_SIZE              12
#define A2DP_MP_HDR_SIZE               1
#define A2DP_CP_HDR_SIZE               1

#endif /*_A2DP_HEADER_DEFS_H*/
