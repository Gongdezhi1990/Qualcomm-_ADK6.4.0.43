/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file
 * Internal header for groaning quietly
 *
 */

#ifndef KALSIM_MSGIF_PRIVATE_H
#define KALSIM_MSGIF_PRIVATE_H

#include "kalsim_msgif/kalsim_msgif.h"
#include "hal/hal.h"
#include "sched/bg_int.h"
#include "preserved/preserved.h"
#include "int/int.h"
#include "hydra/hydra_macros.h"
#include "pmalloc/pl_malloc.h"
#include "sched_oxygen/sched_oxygen.h"
#include "platform/pl_interrupt.h"
#include "platform/pl_assert.h"
#include "id/id.h"
#include "stream/stream.h"
#include "opmgr/opmgr.h"
#include "patch/patch.h"
#include "adaptor/adaptor.h"
#include "audio_log/audio_log.h"
#include "subserv/subserv.h"
/* Operators may send unsolicited messages, so grab IDs */
#include "opmgr/opmgr_for_ops.h"
#include "hal/hal_interrupt.h"

typedef enum service_identifier
{
    AUDIO_DATA_SINK_SERVICE = 0,
    AUDIO_DATA_SOURCE_SERVICE = 1,
    AUDIO_DATA_CAP_DOWNLOAD_SERVICE = 2,
    SCO_PROCESSING_SERVICE = 3
} service_identifier;

typedef enum service_response_result
{
    SUCCESS = 0,
    FAILURE = 1
} service_response_result;

typedef enum service_dir
{
    AUDIO_SOURCE = 0,
    AUDIO_SINK = 1
} service_dir;

#define START_SERVICE_REQ_MIN_MSG_LEN      7
#define START_SERVICE_RESP_MIN_MSG_LEN     4
#define STOP_SERVICE_RESP_MSG_LEN          sizeof(generic_service_resp_struct)/sizeof(uint16)
#define AUDIO_DATA_SERV_RESP_PAYLOAD_LEN   7

#define SCO_PROCESSING_SERV_REQ_PAYLOAD_LEN  6
#define SCO_PROCESSING_SERV_RESP_PAYLOAD_LEN  6


#define AUDIO_DATA_SERV_RESP_MSG_LEN       START_SERVICE_RESP_MIN_MSG_LEN + AUDIO_DATA_SERV_RESP_PAYLOAD_LEN
#define SCO_PROCESSING_SERV_RESP_MSG_LEN   START_SERVICE_RESP_MIN_MSG_LEN + SCO_PROCESSING_SERV_RESP_PAYLOAD_LEN


/** Service start response structure */
typedef struct data_service_start_resp_struct{
    uint16    msg_id;
    uint16    serv_tag;
    uint16    serv_start_result;
    uint16    payload_len;
    uint16    endpoint_id;
    uint16    data_read_handle;
    uint16    data_write_handle;
    uint16    meta_read_handle;
    uint16    meta_write_handle;
    uint16    kick_status_bits;
    uint16    kick_block_id;
} data_service_start_resp_struct;

/** Service start response structure */
typedef struct sco_service_start_resp_struct{
    uint16    msg_id;
    uint16    serv_tag;
    uint16    serv_start_result;
    uint16    payload_len;
    uint16    to_air_read_handle;
    uint16    to_air_write_handle;
    uint16    to_air_aux_handle;
    uint16    from_air_read_handle;
    uint16    from_air_write_handle;
    uint16    from_air_aux_handle;
    uint16    wclock_buff_handle;
    uint16    wclock_offset;
} sco_service_start_resp_struct;

/** Generic Service response structure used by
 *  - service stop
 *  - set SCO parameters
 */
typedef struct generic_service_resp_struct{
    uint16    msg_id;
    uint16    serv_tag;
    uint16    result;
} generic_service_resp_struct;

/** Configuration for setting up a kick */
typedef struct {
    uint16      block_id;
    system_bus  dest_addr;
    uint16      status_bits;
} kick_params_t;


/* Structure originating in //depot/app_ss/main/fw/src/core/transport_audio/transport_audio_private.h
   A few fields had to be removed/adapted to make it work with the audio firmware. */

typedef struct audio_sink_source_struct {
    /** Linked list pointer */
    struct audio_sink_source_struct * p_next;

    /** Callback in the clent that is used to indicate data received (Audio
     * source) or more space available (Audio Sink). Can be NULL if not
     * required.
     */
/*    audio_data_link_callback_t *callback;*/

    /** Type from the stream.xml STREAM_DEVICE enumeration. Passed by the
     * client to the audio subsytem during service start.
     */
/*    stream_device type;*/
    STREAM_DEVICE type;

    /**
     * ID of the endpoint on the audio subsystem that
     * this transport link is being connected to (or zero if not applicable).
     */
    uint16 connected_endpoint_id;

    /**
     * Context supplied by the client through the
     * \c transport_audio_set_client_context() call  and returned
     * to it through the \c transport_audio_client_context_from_handle()
     * call.
     */
    void * client_context;

    /** Whether this endpoint is a Sink (TRUE) or a Source (FALSE). */
    bool sink;

    /**
     * ID allocated by the audio subsystem during service start. It is
     * used in Accmds to configure the endpoint.
     */
    uint16 audio_endpoint_id;

    /**
     * Pointer to memoy allocated for the ies array during service start.
     * It is freed on completion of the start.
     */
    uint16 * option_ies_array;

    /** Local buffer that holds the data and meta data for exchange with the
     * audio subsystem.
     */
    /*BUFFER_MSG * buffer;*/
    /**
     * Size of the meta data entries in the \c buffer. This may be different
     * to \c meta_b.entry_len which is the size of meta data entries in the
     * audio meta data buffer.
     */
    uint8 buf_meta_size;

    struct {
        /** For an Audio Sink the offset of the data_wr_h up to which data was
         * queued to be written to. For an Audio source it is the offset of the
         * data_rd_h where data has been queued to be read from.
         */
        mmu_offset last_offset;

        /** Read handle for the audio data buffer */
        mmu_handle rd_h;
        /** Write handle for the audio data buffer */
        mmu_handle wr_h;
        /** MMU handle of the buffer whose handle offset the audio subsystem
         * updates to mirror that of the \c wr_h (for audio sources) or the
         * \c rd_h (for audio sinks) */
        mmu_handle surrogate_mmu;
        /** Size in bytes of the audio data buffer */
        uint16 size_bytes;
    } data_b;

    struct {
        /** For an Audio Sink the offset of the meta_wr_h up to which meta data
         * was queued to be written to. For an Audio Source it is the offset of
         * the meta_rd_h where data has been queued to be read from.
         */
        mmu_offset last_offset;
        /** Read handle for the audio meta data buffer */
        mmu_handle rd_h;
        /** Write handle for the audio meta data buffer */
        mmu_handle wr_h;
        /** MMU handle of the buffer whose handle offset the audio subsystem
         * updates to mirror that of the \c wr_h (for audio sources) or the
         * \c rd_h (for audio sinks) */
        mmu_handle surrogate_mmu;
        /** Size in bytes of the audio meta data buffer */
        uint16 size_bytes;
        /** Size of each entry in the audio meta data buffer */
        uint8 entry_len;
    } meta_b;

    SUBSERV_TAG audio_sstag;            /**< sstag needed to stop the service */
    /*audio_transport_run_state_t run_state;*/  /**< Run state of the service*/
    /** Parameters used to kick the audio subsystem to notify it of data
     * flow. */
    kick_params_t audio_kick;
    /** Whether the audio requires kicking for this endpoint */
    bool audio_kick_used;

    /** Number of bytes consumed by the sink from its buffer since the last
     * flow control message was sent. Only applicable to audio sink endpoints
     * that are connected to the audio data test tunnel.
     */
    uint16 bytes_consumed;

    /** Number of packets consumed by the sink from its buffer since the last
     * flow control message was sent. Only applicable to audio sink endpoints
     * that are connected to the audio data test tunnel.
     */
    uint8  pkts_consumed;

    /** Number of transfers queued in the dma */
    uint8 queued_msgs;

    /** Number of bytes queued in the dma */
    uint32 queued_bytes;

    /** Indication from the interrupt context to the background interrupt
     * that this link has either consumed data (audio sink) or produced data
     * (audio source). It is cleared before doing a client callback.
     */
    bool data_moved_indication_needed;

    /**
     * Provides the standard meta data of packet boundary and fragment size
     * for audio sink endpoints.
     */
    uint16 * source_pkt_info;
} audio_sink_source_t;

void start_audio_data_service(uint16 in_pdu_len, uint16 *in_pdu, service_dir direction);
void start_sco_processing_service(uint16 in_pdu_len, uint16 *in_pdu);

extern void fileserv_data_report_callback(uint16* pdu, uint16 data_len_octets);

void subserv_service_advice(void);

void set_sco_parameters(uint16 in_pdu_len, uint16 *in_pdu, SUBSERV_TAG sstag);

void stop_service(service_identifier type, SUBSERV_TAG sstag);

#endif /* KALSIM_MSGIF_PRIVATE_H */


