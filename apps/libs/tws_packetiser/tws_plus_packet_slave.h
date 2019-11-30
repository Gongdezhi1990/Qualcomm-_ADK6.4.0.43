/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_plus_packet_slave.h

DESCRIPTION
    The TWS+ packet slave reads the header and audio frames from a TWS+ packet.

*/

#ifndef TWS_PLUS_PACKET_SLAVE_H_
#define TWS_PLUS_PACKET_SLAVE_H_

#include <tws_packetiser.h>
#include "frame_info.h"

/*! TWS+ slave packet */
typedef struct
{
    /*! Read ptr in buffer. */
    const uint8 *ptr;

    /*! Length of the packet. */
    uint32 len;

    /*! Points to start of packet. */
    const uint8 *packet;

    /*! Number of frames read from the packet */
    uint32 frames;
    
    /*! Store the frame info history for SBC */
    frame_info_history_sbc_t frame_info_history_sbc;

    /*! The packet slave shares the packetiser's config */
    tws_packetiser_slave_config_t *config;

} tws_plus_packet_slave_t;

#endif
