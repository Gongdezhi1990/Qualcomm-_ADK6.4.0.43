/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    rtp_packet_master.h

DESCRIPTION
    The rtp packet master's state.
*/

#ifndef RTP_PACKET_MASTER_H_
#define RTP_PACKET_MASTER_H_

#include "tws_packetiser.h"
#include "frame_info.h"

/*! RTP master packet state */
typedef struct __rtp_packet_master
{
    /*! Write ptr in buffer. */
    uint8 *ptr;

    /*! Length of the buffer. */
    uint32 len;

    /*! Points to start of buffer. */
    uint8 *buffer;

    /*! Number of frames written to the packet */
    uint32 frames;

    /*! Enable the content protection header in the RTP packet */
    bool cp_header_enabled;

    /*! The codec type */
    tws_packetiser_codec_t codec;

    /*! RTP header sequence number */
    uint16 sequence_number;

    /*! RTP header time stamp */
    uint32 time_stamp;

    /*! RTP header SSRC */
    uint32 ssrc;

    /*! Store the frame info history for SBC */
    frame_info_history_sbc_t frame_info_history_sbc;

} rtp_packet_master_t;

#endif
