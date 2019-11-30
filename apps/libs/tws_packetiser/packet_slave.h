/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    packet_slave.h

DESCRIPTION
    This header file defines the interface that all packet slaves (e.g. TWS and TWS+) must implement.

*/

#ifndef PACKET_SLAVE_H_
#define PACKET_SLAVE_H_

#include "tws_packet_slave.h"
#include "tws_plus_packet_slave.h"
#include "tws_packetiser.h"
#include <rtime.h>
#include <packetiser_helper.h>

/*! Forward declaration of packet_slave_t */
typedef struct __packet_slave packet_slave_t;

/*!
  @brief Initialise a packet slave.
  @param packet The packet instance.
  @param packet_buffer Pointer to the buffer from which to read the packet.
  @param packet_len The length of the packet.
  @param config The packetiser configuration.
  @return TRUE on successful initialisation, FALSE otherwise.
*/
typedef bool packet_slave_init_t(packet_slave_t *packet,
                                 const uint8 *packet_buffer, uint32 packet_len,
                                 tws_packetiser_slave_config_t *config);

/*!
  @brief Uninitialise a packet slave, freeing any dynamic memory allocated.
  @param packet The packet instance.
*/
typedef void packet_slave_uninit_t(packet_slave_t *packet);

/*!
  @brief Read the packet header from the packet.
  @param packet The packet instance.
  @param [OUT] ttp The frame's time-to-play.
  @param scmst [OUT] The read scmst.
  @param complete [OUT] TRUE if the packet contains a complete audio frame, or the final
         fragment of a frame.
  @return TRUE if the packet header was read correctly and the audio data tag
  was found, otherwise FALSE.
*/
typedef bool packet_slave_read_header_t(packet_slave_t *packet, rtime_t *ttp,
                                        packetiser_helper_scmst_t *scmst,
                                        bool *complete);

/*!
  @brief Read the next mini sample period adjustment from the tws packet.
  @param packet The packet instance.
  @param spadjm [OUT] The frame's mini sample period adjustment.
  @return TRUE if spadjm was read, FALSE otherwise.
  Complete frames and the first fragment of a fragmented audio frame have a spadjm.
*/
typedef bool packet_slave_read_mini_spadj_t(packet_slave_t *packet, rtime_spadj_mini_t *spadjm);

/*!
  @brief Read the next audio frame from the tws packet.
  @param packet The packet instance.
  @param dest The destination to write the frame to.
  @param frame_length The length of the frame (determined by calling
  #twsPacketReadAudioFrameInfo).
  @param frame_number [OUT] The zero based frame number read.
  @return TRUE if frame was read, FALSE otherwise.
*/
typedef bool packet_slave_read_audio_frame_t(packet_slave_t *packet, uint8 *dest,
                                             uint32 frame_length, uint32 *frame_number);

/*!
  @brief Read the frame info for the next frame in the packet.
  @param packet The packet instance.
  @param frame_info [OUT] The frame information.
  @return TRUE if the info could be determined, FALSE otherwise.
*/
typedef bool packet_slave_read_audio_frame_info_t(packet_slave_t *packet, frame_info_t *frame_info);

/*!
  @brief Get the TTP length in bits.
  @param packet The packet instance.
  @return The length of the TTP in bits.
  The length returned must be <=32.
*/
typedef uint32 packet_slave_get_ttp_len_bits_t(packet_slave_t *packet);

/*! Standard functions the packet slaves must implement */
typedef struct __packet_slave_functions
{
    /*! Initialise the overall packet_slaves instance */
    packet_slave_init_t *init;
    /*! Uninitialise the overall packet_slaves instance */
    packet_slave_uninit_t *unInit;
    /*! Read the packet header */
    packet_slave_read_header_t *readHeader;
    /*! Read the packet mini spadj */
    packet_slave_read_mini_spadj_t *readMiniSpadj;
    /*! Read the audio frame */
    packet_slave_read_audio_frame_t *readAudioFrame;
    /*! Read the audio frame info */
    packet_slave_read_audio_frame_info_t *readAudioFrameInfo;
    /*! Get the TTP length in bits */
    packet_slave_get_ttp_len_bits_t *getTTPLenBits;

} packet_slave_functions_t;

/*! The generic packet slave type through which the functionality of the packet slave is accessed */
struct __packet_slave
{
    /*! A union of the different packet slaves */
    union
    {
        tws_packet_slave_t tws;
        tws_plus_packet_slave_t tws_plus;
        /* Add other slave packet types here */
    } slave;
    
    /*! Function pointers to access the packet slave's API */
    const packet_slave_functions_t *funcs;
};

/*! Interface for TWS packet slave defined externally */
extern const packet_slave_functions_t packet_slave_funcs_tws;

/*! Interface for TWS+ packet slave defined externally */
extern const packet_slave_functions_t packet_slave_funcs_tws_plus;

#endif
