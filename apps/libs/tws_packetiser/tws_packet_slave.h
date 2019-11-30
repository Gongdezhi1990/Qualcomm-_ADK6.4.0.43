/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packet_slave.h

DESCRIPTION
    The TWS packet slave reads the header and audio frames from a TWS packet.

*/

#ifndef TWS_PACKET_SLAVE_H_
#define TWS_PACKET_SLAVE_H_

#include <tws_packetiser.h>
#include <packetiser_helper.h>
#include <rtime.h>
#include "extended_audio_header.h"
#include "frame_info.h"

/*! Standard TWS slave function for reading frame info */
typedef struct __tws_packet_slave tws_packet_slave_t;
typedef bool(*twsPacketReadAudioFrameInfoFn)(tws_packet_slave_t *tp,
                                             frame_info_t *frame_info);

/*! TWS slave packet */
struct __tws_packet_slave
{
    /*! Read ptr in buffer. */
    const uint8 *ptr;

    /*! Length of the packet. */
    uint32 len;

    /*! Points to start of packet. */
    const uint8 * packet;

    /*! Number of frames read from to the packet */
    uint32 frames;

    /*! Extended audio header reader state */
    eah_reader_state_t *eah_reader;

    /*! Function to call to read the audio frame info */
    twsPacketReadAudioFrameInfoFn read_audio_frame_info_fptr;

    /*! The number of samples each frame represents once decoded.
      This parameter will not be used for SBC (where the frame header is used to
      calculate the number of samples or aptX.
      This parameter will need to be set for other codecs where there is more
      than one frame per packet (i.e. the extended audio header is being used).
     */
    uint32 frame_samples;
    
    /*! Store the frame info history for SBC */
    frame_info_history_sbc_t frame_info_history_sbc;

    /*! Fragmentation is only allowed when configured for particular codecs */
    bool fragmentation_allowed;

};

#endif
