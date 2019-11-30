/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

*/
/*!
@file tws_packetiser.h
@brief Interface to the TWS Packetiser library.

The TWS packetiser library contains supports for both master and slave roles.

The TWS master packetiser creates packets containing audio frames read (with
metadata) from a kymera Source. The TWS packets are written to a Sink.

The TWS slave packetiser reads packets from a Source. The packets are parsed and the
audio frames within a written (with metadata) to a kymera Sink.

*/

#ifndef TWS_PACKETISER_H_
#define TWS_PACKETISER_H_

#include <source.h>
#include <sink.h>
#include <library.h>
#include <rtime.h>
#include <message.h>
#include <packetiser_helper.h>

/****************************************************************************
 * Typedefs and Enumerations
 ****************************************************************************/
 
 /*! Enumerate the master modes supported by the TWS packetiser */
 typedef enum __tws_packet_master_modes
 {
    /*! Create TWS V4 packets */
    TWS_PACKETISER_MASTER_MODE_TWS,
    /*! Create RTP packets */
    TWS_PACKETISER_MASTER_MODE_RTP,
    /*! Create RTP packets with no header */
    TWS_PACKETISER_MASTER_MODE_NO_HEADER,
    /*! Create TWS+ packets */
    TWS_PACKETISER_MASTER_MODE_TWS_PLUS,
    /*! Always the final item in the enumeration */
    TWS_PACKETISER_MASTER_MODES
 } tws_packet_master_modes_t;

/*! Enumerate the slave modes supported by the TWS packetiser */
 typedef enum __tws_packet_slave_modes
 {
    /*! Read TWS V4 packets */
    TWS_PACKETISER_SLAVE_MODE_TWS,
    /*! Read TWS+ packets */
    TWS_PACKETISER_SLAVE_MODE_TWS_PLUS,
    /*! Always the final item in the enumeration */
    TWS_PACKETISER_SLAVE_MODES
 } tws_packet_slave_modes_t;

/*! Enumerate the codecs supported by the TWS packetiser. */
typedef enum __tws_packetiser_codec
{
    TWS_PACKETISER_CODEC_SBC,
    TWS_PACKETISER_CODEC_AAC,
    TWS_PACKETISER_CODEC_MP3,
    TWS_PACKETISER_CODEC_APTX,
    TWS_PACKETISER_CODEC_APTX_AD
} tws_packetiser_codec_t;

/*! Structure defining the TWS packetiser configuration.
  All members must be set when initialising the library.
*/
typedef struct __tws_packetiser_master_config
{
    /*! The client task */
    Task client;

    /*! The source of audio frames with metadata. */
    Source source;

    /*! The sink of TWS packets. */
    Sink sink;
    
    /*! The master mode */
    tws_packet_master_modes_t mode;

    /*! The codec */
    tws_packetiser_codec_t codec;

    /*! Enable the content protection header in a RTP packet. Only applicable to RTP packets */
    bool cp_header_enable;

    /*! The audio serial copy management system type. Applicable to both RTP and TWS packets */
    packetiser_helper_scmst_t scmst;
    
    /*! The maximum transmission unit of the TWS packet */
    uint16 mtu;

    /* The time before the TTP at which a packet should be transmitted */
    rtime_t time_before_ttp_to_tx;

    /* The last time before the TTP at which a packet may be transmitted */
    rtime_t tx_deadline;

} tws_packetiser_master_config_t;

/*! Structure defining the TWS slave packetiser configuration.
  All members must be set when initialising the library.
*/
typedef struct __tws_packetiser_slave_config
{
    /*! The client task */
    Task client;

    /*! The source of TWS packets. */
    Source source;

    /*! The sink of audio frames with metadata */
    Sink sink;

    /*! The codec */
    tws_packetiser_codec_t codec;

    /*! The sample rate. */
    rtime_sample_rate_t sample_rate;

    /*! The expected audio serial copy management system type. */
    packetiser_helper_scmst_t scmst;

     /*! The slave mode */
    tws_packet_slave_modes_t mode;

    /*! The content protection header in a TWS+ packet is enabled.
        Only applicable to TWS+ packets */
    bool cp_header_enabled;

} tws_packetiser_slave_config_t;

/*! Opaque reference to tws packetiser master. */
struct __tws_packetiser_master;
typedef struct __tws_packetiser_master tws_packetiser_master_t;
/*! Opaque reference to tws packetiser slave. */
struct __tws_packetiser_slave;
typedef struct __tws_packetiser_slave tws_packetiser_slave_t;

/****************************************************************************
 * Message Interface
 ****************************************************************************/

#ifndef TWS_PACKETISER_MESSAGE_BASE
#define TWS_PACKETISER_MESSAGE_BASE 0xaaaa
#endif

/*! Messages sent by the TWS packetiser library to the client_task
  registered with #TwsPacketiserMasterInit() and #TwsPacketiserSlaveInit().
*/
typedef enum
{
    TWS_PACKETISER_SLAVE_SCMST_CHANGE_IND = TWS_PACKETISER_MESSAGE_BASE,
    TWS_PACKETISER_FIRST_AUDIO_FRAME_IND,
    TWS_PACKETISER_MESSAGE_TOP
} tws_packetiser_message_id_t;

/*! Indication sent to the client task when a scmst change is detected by the
  TWS slave. Not applicable for the TWS master. The client does not need to
  respond to this indication.
*/
typedef struct
{
    /*! Pointer to the instance of the TWS master packetiser library */
    tws_packetiser_slave_t *tws_packetiser;
    /*! The new scmst type */
    packetiser_helper_scmst_t scmst;
} TWS_PACKETISER_SLAVE_SCMST_CHANGE_IND_T;

/****************************************************************************
 * TWS Master functions
 ****************************************************************************/

/*!
  @brief Create an instance and initialise the TWS master packetiser.
  @param config The configuration.
  @return The instance of the library created by the function, or NULL
  if the packetiser could not be created.
*/
tws_packetiser_master_t* TwsPacketiserMasterInit(tws_packetiser_master_config_t *config);

/*!
  @brief Get the packet header length in octets.
  @param tp The instance.
  @param number_audio_frames The number of audio frames to transport in the packet.
  @return The header overhead length in octets.
  
  If number_audio_frames is zero, the function will return the number of octets in the primary
  packet header. For non-zero number_audio_frames, the returned value will also include the
  number of header octets per frame.
*/ 
uint32 TwsPacketiserMasterHeaderLength(tws_packetiser_master_t *tp, uint32 number_audio_frames);

/*!
  @brief Destroy the TWS master packetiser instance.
  @param tp The instance to destroy.
*/
void TwsPacketiserMasterDestroy(tws_packetiser_master_t *tp);

/****************************************************************************
 * TWS slave functions
 ****************************************************************************/

/*!
  @brief Create an instance and initialise the TWS slave packetiser.
  @param config The configuration.
  @return The instance of the library created by the function, or NULL
  if the packetiser could not be created.
*/
tws_packetiser_slave_t* TwsPacketiserSlaveInit(tws_packetiser_slave_config_t *config);

/*!
  @brief Destroy the TWS slave packetiser instance.
  @param tp The instance to destroy.
*/
void TwsPacketiserSlaveDestroy(tws_packetiser_slave_t *tp);

#endif
