/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packet_private.h

*/

#ifndef TWS_PACKET_PRIVATE_H_
#define TWS_PACKET_PRIVATE_H_

#include <packetiser_helper.h>

/*! The packet time-to-play byte representation */
typedef struct __ttp_bytes
{
    uint8 ttp[3];
} ttp_bytes_t;

/*! The length of the TWS packet header */
#define TWS_HEADER_LEN (sizeof(ttp_bytes_t) + TAG_LENGTH)

#endif
