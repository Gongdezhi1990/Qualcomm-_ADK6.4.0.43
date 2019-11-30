/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    rtp.h

DESCRIPTION
    Common definitions for RTP.

*/

#ifndef RTP_H_
#define RTP_H_

/*
  RTP Header definitions
*/
#define RTP_MINIMUM_HEADER_SIZE         12
#define RTP0_VERSION_2                  0x80
#define RTP0_VERSION_MASK               0xc0
#define RTP0_PADDING                    0x20
#define RTP0_EXTENSION                  0x10
#define RTP0_CSRC_COUNT_MASK            0x0f
#define RTP1_MARKER                     0x80
#define RTP1_PAYLOAD_TYPE_MASK          0x7f
#define RTP1_PAYLOAD_TYPE_FIRST_DYNAMIC 96
#define RTP1_PAYLOAD_TYPE_LAST_DYNAMIC  127
#define RTP_SEQ_INVALID                 0

#define SCMS_HEADER_SIZE 1
#define SBC_MEDIA_PAYLOAD_HEADER_SIZE 1

#define SBC_MAX_FRAMES 15

#endif
