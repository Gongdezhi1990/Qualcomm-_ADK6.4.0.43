/****************************************************************************
Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    erasure_coding.h

DESCRIPTION
    Interface to the Erasure Coding library. Encodes K packets into N packets.
*/

#ifndef ERASURE_CODING_H_
#define ERASURE_CODING_H_

#include <csrtypes.h>
#include <hydra_macros.h>
#include <rtime.h>
#include <library.h>

/*! Incomplete definition of the private parameters structure */
struct __ec_params;
typedef struct __ec_params ec_params_t;

/*! The available parameters for the supported ec modes */
extern const ec_params_t ec_params_2_5;
extern const ec_params_t ec_params_3_7;
extern const ec_params_t ec_params_3_8;
extern const ec_params_t ec_params_3_9;

typedef struct
{
    /*! The sink of erasure coded packets. */
    Sink sink;

    /*! The csb radio interval in microseconds */
    rtime_t csb_interval;

    /*! Extra latency the transmitter must take into account when informing its
        client (through #ErasureCodingTxLatency) the amount time taken to transmit
        packets to the remote device */
    rtime_t extra_latency;

    /*! The stream ID to transmit */
    uint8 stream_id;

    /*! The EC codec parameters */
    const ec_params_t *params;

    /*! The lower layer transport's MTU */
    uint16 mtu;

} ec_config_tx_t;

typedef struct
{
    /* The task to which ERASURE_CODING_RX_PACKET_IND will be sent, optional */
    Task packet_client;

    /* The task to which ERASURE_CODING_RX_STREAM_ID_CHANGE_IND and 
      ERASURE_CODING_RX_AFH_CHANNEL_MAP_CHANGE_PENDING_IND will be sent */
    Task change_client;

    /*! The source of erasure coded packets. */
    Source source;

    /*! The csb radio interval in microseconds */
    rtime_t csb_interval;

    /*! The stream ID to expect to receive */
    uint8 stream_id;

    /*! The EC codec parameters */
    const ec_params_t *params;

} ec_config_rx_t;

typedef struct
{
    /*! The task to inform when the packet is transmitted, not used in receive case. */
    Task task;
    /*! In the transmit case, the rendering time of the packet.
        In the receive case, the time at which the packet was decoded. */
    rtime_t timestamp;
    /*! The number of octets in the buffer */
    uint16 size;
    /*! A buffer of data */
    uint8 buffer[1];
} ec_unencoded_packet_t;

typedef struct
{
    /*! Incremented when a EC packet is received with a stream ID that is unsupported */
    uint16 stream_id_unsupported;
    /*! Incremented when a EC packet is received with a invalid coding ID */
    uint16 coding_id_invalid;
    /*! Element incremented when coding ID is received in EC packet */
    uint16 coding_id[16];
    /*! Incremented when EC decode fails */
    uint16 decode_failures;
} ec_stats_rx_t;

/*! Incomplete definition of ec tx state */
struct __ec_state_tx_t;
/*! Incomplete definition of ec rx state */
struct __ec_state_rx_t;

/*! Handle for the ec tx instance */
typedef struct __ec_state_tx_t *ec_handle_tx_t;
/*! Handle for the ec rx instance */
typedef struct __ec_state_rx_t *ec_handle_rx_t;

/****************************************************************************
 * Message Interface
 ****************************************************************************/

/*! Messages sent by the ec library to the client tasks */
typedef enum
{
    ERASURE_CODING_TX_PACKET_CFM = ERASURE_CODING_MESSAGE_BASE,
    ERASURE_CODING_RX_PACKET_IND,
    ERASURE_CODING_RX_STREAM_ID_CHANGE_IND,
    ERASURE_CODING_RX_AFH_CHANNEL_MAP_CHANGE_PENDING_IND,
    ERASURE_CODING_STATS_RX_IND,
    ERASURE_CODING_MESSAGE_TOP
} ec_message_id_t;

/*! Confirm message sent in response to ErasureCodingTxPacketReq() */
typedef struct
{
    /*! Handle for the ec instance */
    ec_handle_tx_t handle;
    /*! The unencoded packet that was transmitted */
    ec_unencoded_packet_t *unencoded;
    /*! The effective number of times the packet was transmitted, 0=not transmitted */
    uint16 times_transmitted;
} ERASURE_CODING_TX_PACKET_CFM_T;

/*! Indication sent to the client task with the decoded packet.
    The client must respond by calling #ErasureCodingRxPacketResponse. */
typedef struct
{
    /*! Handle for the ec instance */
    ec_handle_rx_t handle;
    /*! The unencoded packet that was received */
    ec_unencoded_packet_t *unencoded;
} ERASURE_CODING_RX_PACKET_IND_T;

/*! Indication sent to the client task when a stream ID change is detected by
  in the receiver role.
  The client must respond by calling #ErasureCodingRxStreamIDChangeResponse.
*/
typedef struct
{
    /*! Handle for the ec instance */
    ec_handle_rx_t handle;
    /*! The new stream ID */
    uint8 stream_id;
} ERASURE_CODING_RX_STREAM_ID_CHANGE_IND_T;

/*! Indication sent to the client task in the receiver role, when the
  received packet header indicates that a AFH channel map change is
  pending. The handler does not need to respond. The handler should
  start scanning for a CSB sync train which will contain the pending
  AFH channel map and the instant at which the new channel map will be
  applied.
*/
typedef struct
{
    /*! Handle for the ec instance */
    ec_handle_rx_t handle;
} ERASURE_CODING_RX_AFH_CHANNEL_MAP_CHANGE_PENDING_IND_T;

/* Sent periodically with EC receive statistics */
typedef struct
{
    /*! Handle for the ec instance */
    ec_handle_rx_t handle;
    /*! The stats */
    ec_stats_rx_t stats;
} ERASURE_CODING_STATS_RX_IND_T;

/**
 * \brief Initialise the transmit erasure coding library.
 * \param config The required config.
 * \return Handle for the instance created.
 */
ec_handle_tx_t ErasureCodingTxInit(const ec_config_tx_t *config);

/**
 * \brief Destroy the transmit erasure coding library.
 * \param handle to instance to destory.
 */
void ErasureCodingTxDestroy(ec_handle_tx_t handle);

/**
 * \brief Initialise the receive erasure coding library.
 * \param config The required config.
 * \return Handle for the instance created.
 */
ec_handle_rx_t ErasureCodingRxInit(const ec_config_rx_t *config);

/**
 * \brief Destroy the transmit erasure coding library.
 * \param handle to instance to destory.
 */
void ErasureCodingRxDestroy(ec_handle_rx_t handle);

/**
 * \brief Read the Erasure Coding MTU.
 * \param handle Handle for the instance.
 * \ return The MTU.
 */
uint16 ErasureCodingTxMTU(ec_handle_tx_t handle);

/**
 * \brief Allocate a packet in which to create the unencoded buffer.
 * \param handle Handle for the instance.
 * \param size The size of the packet the caller wishes to create.
 * \return The allocated packet.
 * This function _must_ be used to allocate packets as it takes into
 * account specific requirements of the ec encoding algorithm.
 * The caller is responsible for freeing the allocated packet.
 * Normally this can be done on recipt of a ERASURE_CODING_TX_PACKET_CFM message.
 */
ec_unencoded_packet_t *ErasureCodingTxAllocatePacket(ec_handle_tx_t handle, uint16 size);

/**
 * \brief Transmit packet request.
 * \param handle The ec instance.
 * \param unencoded The unencoded packet to transmit.
 * \return TRUE of the packet was accepted for transmitted, otherwise FALSE.
 * The library will transmit a ERASURE_CODING_TX_PACKET_CFM message upon completion.
 */
bool ErasureCodingTxPacketReq(ec_handle_tx_t handle, ec_unencoded_packet_t *unencoded);

/**
  * @brief Inform the library a AFH channel map change is pending.
  * @param handle Handle for the instance.
  * Calling this function will cause the library to toggle a bit in the transmitted
  * packet header, which will inform the receivers that there is a AFH channel map
  * change pending.
  */
void ErasureCodingTxAFHChannelMapChangeIsPending(ec_handle_tx_t handle);

/**
  * @brief Get the time taken (latency) to transmit a set of packets from the library.
  * @param handle Handle for the instance.
  * @param encoded. If true the time taken to fully encode and transmit a sequence of
  *        packets is returned. If false, the time taken to transmit a single
  *        unencoded packet is returned.
  */
rtime_t ErasureCodingTxLatency(ec_handle_tx_t handle, bool encoded);

/**
  * @brief Convert local time to transport time.
  * @param handle Handle for the instance.
  * @param local_time The local time.
  * @param[out] trans_time_result The transport time result.
  * @return TRUE if call successful, otherwise FALSE
  */
bool ErasureCodingTxTimeLocalToTransport(ec_handle_tx_t handle,
                                         rtime_t local_time,
                                         rtime_t * trans_time_result);

/**
  * @brief Respond to a ERASURE_CODING_RX_STREAM_ID_CHANGE_IND message.
  * @param handle Handle for the instance.
  * @param new_params The erasure coding parameters to use as a result of
  *        the stream id change. Set to NULL if the new stream is unsupported.
  */
void ErasureCodingRxStreamIDChangeResponse(ec_handle_rx_t handle, const ec_params_t *new_params);

/**
  * @brief Respond to a ERASURE_CODING_RX_PACKET_IND message.
  * @param handle Handle for the instance.
  * This signals to the EC library that the client has completed any processing of
  * the decoded data and the EC library may continue to decode packets.
  * The EC library will free the unencoded packed at this point.
  */
void ErasureCodingRxPacketResponse(ec_handle_rx_t handle);

/**
  * @brief Set the client to receive ERASURE_CODING_RX_PACKET_INDs.
  * @param handle Handle for the instance.
  * @param packet_client The client task. Set to NULL to stop receiving
  *        messages.
  */
void ErasureCodingRxSetPacketClient(ec_handle_rx_t handle, Task packet_client);

/**
  * @brief Convert transport time to local time.
  * @param handle Handle for the instance.
  * @param transport_time The transport time. A 24-bit time value.
  * @param[out] local_result The transport time converted to local time. A 32-bit time value.
  * Only the least significant 24-bits of transport_time are used
  * in this conversion. It is extended to a 32-bit rtime_t value during
  * the conversion to local time.
  * @return TRUE if call successful, otherwise FALSE
  */
bool ErasureCodingRxTimeTransortToLocal(ec_handle_rx_t handle,
                                        rtime_t transport_time,
                                        rtime_t* local_result);


#endif

