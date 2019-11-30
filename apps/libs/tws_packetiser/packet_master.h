/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    packet_master.h

DESCRIPTION
    This header file defines the interface that all packet masters (e.g. TWS and RTP) must implement.

*/

#ifndef PACKET_MASTER_H_
#define PACKET_MASTER_H_

#include "tws_packet_master.h"
#include "rtp_packet_master.h"
#include "tws_packetiser.h"
#include <rtime.h>
#include <packetiser_helper.h>

/*! Forward declaration of packet_master_t */
typedef struct __packet_master packet_master_t;

/*!
  @brief Initialise a packet master overall.
  @param p The packet instance, the implementation accesses its specific packet master via the union inside the packet_master_t.
  @param codec The packetiser codec type.
  @param content_protection_header_enabled If applicable, enable the content protection header in the packet.
  @return TRUE on successful initialisation, FALSE otherwise.
*/
typedef bool packet_master_init_t(packet_master_t *p, tws_packetiser_codec_t codec, bool content_protection_header_enabled);

/*!
  @brief Initialise a packet master prior to creating a packet.
  @param p The packet instance, the implementation accesses its specific packet master via the union inside the packet_master_t.
  @param buffer Pointer to the buffer in which to write the packet.
  @param buffer_len The length of the buffer.
  @return TRUE on successful packet initialisation, FALSE otherwise.
*/
typedef bool packet_master_packet_init_t(packet_master_t *p, uint8 *buffer, uint32 buffer_len);

/*!
  @brief Calculate the number of octets written to the packet buffer.
  @param p The packet instance.
  @return The packet length.
*/
typedef uint32 packet_master_packet_length_t(packet_master_t *p);

/*!
  @brief Get the packet header length in octets.
  @param p The packet instance.
  @param number_audio_frames The number of audio frames to transport in the packet.
  @return The header length in octets.
  
  If number_audio_frames is zero, the function will return the number of octets in the main
  packet header. For non-zero number_audio_frames, the returned value will also include the
  number of header octets per frame.
*/
typedef uint32 packet_master_header_length_t(packet_master_t *p, uint32 number_audio_frames);

/*!
  @brief Write a packet header to the packet.
  @param p The packet instance.
  @param ttp If applicable, the TTP of the first audio frame that will be written to the packet.
  @param scmst If applicable, the scmst value to write in the content protection header.
*/
typedef void packet_master_write_header_t(packet_master_t *p, rtime_t ttp, packetiser_helper_scmst_t scmst);

/*!
  @brief Write an audio frame to the packet.
  @param p The packet instance.
  @param src The source data.
  @param frame_length The length of the frame.
  @param fmd The frame's metadata.
  @return TRUE if frame was completely written, FALSE otherwise.
*/
typedef bool packet_master_write_audio_frame_t(packet_master_t *p, const uint8 *src, uint32 frame_length, audio_frame_metadata_t *fmd);

/*!
  @brief Inform the packet master that an audio frame will be dropped.
  @param p The packet instance.
  @param src The source data.
  @param frame_length The length of the frame.
  @param fmd The frame's metadata.
*/
typedef void packet_master_dropped_audio_frame_t(packet_master_t *p, const uint8 *src, uint32 frame_length, audio_frame_metadata_t *fmd);

/*!
  @brief Allow the packet master to finalise the packet prior to transmission.
  @param p The packet instance.
*/
typedef void packet_master_finalise_t(packet_master_t *p);


/*! Standard functions the packet masters must implement */
typedef struct __packet_master_functions
{
    /*! Initialise the overall packet_masters instance */
    packet_master_init_t *init;
    /*! Initialise the packet master packet */
    packet_master_packet_init_t *packetInit;
    /*! Calculate the length of the packet */
    packet_master_packet_length_t *packetLength;
    /*! Calculate the length of the packet header */
    packet_master_header_length_t *headerLength;
    /*! Write the packet header */
    packet_master_write_header_t *writeHeader;
    /*! Write a audio frame to the packet */
    packet_master_write_audio_frame_t *writeAudioFrame;
    /*! Inform the packet master of a frame that will be dropped */
    packet_master_dropped_audio_frame_t *droppedAudioFrame;
    /*! Allow the packet master to finalise the packet prior to transmission */
    packet_master_finalise_t *finalise;

} packet_master_functions_t;

/*! The generic packet master type through which the functionality of the packet master is accessed */
struct __packet_master
{
    /*! A union of the different packet masters */
    union
    {
        tws_packet_master_t tws;
        rtp_packet_master_t rtp;
        /* Add other master packet types here */
    } master;
    
    /*! Function pointers to access the packet master's API */
    const packet_master_functions_t *funcs;
};

/*! Interface for TWS packet master defined externally */
extern const packet_master_functions_t packet_master_funcs_tws;

/*! Interface for RTP packet master defined externally */
extern const packet_master_functions_t packet_master_funcs_rtp;

extern const packet_master_functions_t packet_master_funcs_no_header;

/*! Interface for TWS+ packet master defined externally */
extern const packet_master_functions_t packet_master_funcs_tws_plus;

#endif
