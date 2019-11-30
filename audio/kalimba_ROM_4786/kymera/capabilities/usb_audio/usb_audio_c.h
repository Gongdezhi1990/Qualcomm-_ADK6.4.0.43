/****************************************************************************
* Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef USB_AUDIO_C_H_INCLUDED
#define USB_AUDIO_C_H_INCLUDED
#include "stream/stream.h"
#if defined(INSTALL_TTP) && defined(INSTALL_METADATA)
/* usb audio rx will generate metadata
 * usb audio tx will only transfer metadata and expects timestamped metadata
 */
#define USB_AUDIO_SUPPORT_METADATA
#define USB_AUDIO_RX_DEFAULT_LATENCY_REQUIRED 5000 /* 5ms */
#define USB_AUDIO_RX_DEFAULT_PCM_BUFFER_SIZE  384  /* more than 5ms for 48khz */
#include "ttp/ttp.h"
#include "ttp_utilities.h"
#include "cbops_mgr/cbops_mgr.h"
#include "cbops_mgr/cbops_flags.h"
#endif

/* Maximum number of channels */
#define USB_AUDIO_OP_MAX_CHANNELS 6
#define USB_AUDIO_MIN_SAMPLE_RATE 8000
#define USB_AUDIO_MAX_SAMPLE_RATE 192000

/* USB audio data types
 * This isn't a complete list nor does it imply
 * that we support all the data types.
 */
typedef enum
{
   UADF_T1_PCM = 0x0000,
   UADF_T1_PCM8 = 0x0001,
   UADF_T1_IEEE_FLOAT = 0x0002,
   UADF_T1_ALAW = 0x0003,
   UADF_T1_MULAW = 0x0004,
   UADF_T1_RAW_DATA = 0x001F
}USB_AUDIO_PACKET_DATA_FORMAT;

#ifdef USB_AUDIO_SUPPORT_METADATA
/*
 * SRA_TIMESTAMP_METADATA_TRANSFER structure is used
 * to convert timestamped metadata of an SRA input buffer to
 * SRA output buffer.
 */
typedef struct
{
       /* the time stamp of last written block,
        * this is for the end of block, so if no
        * new input tag received we can work out
        * the time stamp for next block.
        */
       TIME last_output_time_stamp;

       /* shows whether first timestamp tag has been
        * observed in the input tag. If it doesn't see
        * it only will only create blank tag for output
        */
       bool first_ts_tag_observed;

       /* last sp_adjust that seen in
        * the input tags.
        */
       int last_input_sp_adjust;

       /* last rate adjust that was used by sra,
        * needs to be populated by user after each
        * sra run. Leave this to 0 if no sra is used
        * between input and output.
        */
       int current_sra_sp_adjust;

       /* last sra phase,
        * needs to be populated by user before each
        * sra run. Leave this to 0 if no sra is used
        * between input and output.
        */
       int last_sra_phase;

       /* the nominal sample rate of input and output in Hz*/
       unsigned sample_rate;
}SRA_TIMESTAMP_METADATA_TRANSFER;

#endif

/* structure for usb_audio operator data */
typedef struct
{
    /* usb_audio buffer, connects to usb endpoints only */
    tCbuffer* usb_audio_buf;

    /* pcm channels buffers */
    tCbuffer* pcm_channel_buf[USB_AUDIO_OP_MAX_CHANNELS];

    /* scratch buffers used for */
    tCbuffer* pcm_scratch_buf[USB_AUDIO_OP_MAX_CHANNELS];

    /* coded buffer */
    tCbuffer* coded_buf;

    /* nominal sample rate of the stream */
    unsigned sample_rate;

    /* number of channels */
    unsigned nrof_channels;

    /* data type */
    USB_AUDIO_PACKET_DATA_FORMAT data_format;

    /* subframe size */
    unsigned subframe_size;

    /* bit resolution */
    unsigned bit_resolution;

    /* flag showing whether the operator is usb_audio tx or rx operator */
    bool is_usb_audio_tx;

    /* shows that op has been configured */
    bool op_configured;

    /* PCM buffer size set by user,
     * default USB_AUDIO_RX_DEFAULT_PCM_BUFFER_SIZE will
     * be used for PCM channels only */
    unsigned pcm_buffer_size;

    unsigned scratch_buff_size;

    /* usb endpoint connected to this op
     * Used for communication with the endpoint
     */
    ENDPOINT *usb_audio_ep;

#ifdef USB_AUDIO_SUPPORT_METADATA
    ttp_context *time_to_play;
    unsigned target_latency;

    /* metadata buffer for pcm channels - Rx and Tx */
    tCbuffer *pcm_metadata_buffer;

    /* whether the tx op needs to perform rate adjustment - Tx only  */
    bool usb_tx_rate_adjust_enable;

    /** The software rate adjust cbops structure - Tx only */
    void *sra;

    /* structure used for time stamping the output - Tx only*/
    SRA_TIMESTAMP_METADATA_TRANSFER sra_mtrans;

    /* cbops manager - Tx only */
    cbops_mgr *cbops;
#endif
}USB_AUDIO_OP_DATA;

/* USB audio RX and TX share same code and structure definitions, macros
 * below test whether an operator is TX or RX.
 */
#define USB_AUDIO_IS_TX_OPERATOR(usb_audio_data) (usb_audio_data->is_usb_audio_tx)
#define USB_AUDIO_IS_RX_OPERATOR(usb_audio_data) (!USB_AUDIO_IS_TX_OPERATOR(usb_audio_data))

/* USB audio data can only be received/sent from/to usb audio
 * endpoints. So always one side of the usb audio operator is
 * connected to a usb audio endpoint, this will be input side
 * for RX operator and  output side for TX operator:
 *                              +-------------------+
 * +-----------------+          |                   |------>CH0
 * | usb audio Rx EP |--------->| usb audio Rx OP   |
 * +-----------------+          |                   |------>CH1
 *                              +-------------------+
 *
 *         +-------------------+
 *  CH0--->|                   |          +-----------------+
 *         | usb audio Tx OP   |--------->| usb audio Tx EP |
 *  CH1--->|                   |          +-----------------+
 *         +-------------------+
 *
 * This macro tests whether a terminal is on the endpoint side,
 * and is for both TX and Rx operators.
 */
#define USB_AUDIO_TERMINAL_IS_USB_AUDIO_EP(usb_audio_data, terminal_id)    \
    (((terminal_id & TERMINAL_SINK_MASK) != 0) == USB_AUDIO_IS_RX_OPERATOR(usb_audio_data))

/****************************************************************************
Private Function Definitions
*/
extern bool usb_audio_get_data_format (OPERATOR_DATA * op_data,void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_start (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_stop (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_reset (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_destroy (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_create (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_connect (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_disconnect (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_buffer_details (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool usb_audio_get_sched_info (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern void usb_audio_process_data (OPERATOR_DATA * op_data, TOUCHED_TERMINALS *touched);
extern bool usb_audio_opmsg_set_connection_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                            OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool usb_audio_configure_connected_usb_audio_endpoint(OPERATOR_DATA * op_data);
extern bool usb_audio_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                            OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef USB_AUDIO_SUPPORT_METADATA
extern void usb_audio_rx_generate_metadata_with_ttp(USB_AUDIO_OP_DATA *usb_audio_data, unsigned samples);
extern bool usb_audio_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                            OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool usb_audio_opmsg_set_latency_limits(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                            OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool usb_audio_opmsg_set_ttp_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length,
                                           OP_OPMSG_RSP_PAYLOAD **resp_data);
extern void usb_audio_tx_transfer_metadata(USB_AUDIO_OP_DATA *usb_audio_data, unsigned consumed_samples, unsigned written_frames);
extern metadata_tag *timestamp_metadata_reframe_with_sra(SRA_TIMESTAMP_METADATA_TRANSFER *sra_mtrans,
                                              metadata_tag *input_mtag,
                                              unsigned input_mtag_b4ix,
                                              unsigned consumed_samples,
                                              unsigned written_frames);
#endif /* #ifdef USB_AUDIO_SUPPORT_METADATA */


/****************************************************************************
OBPM Function Definitions
*/
extern bool usb_audio_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool usb_audio_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool usb_audio_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool usb_audio_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool usb_audio_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

#endif /* #ifndef USB_AUDIO_C_H_INCLUDED */

/*  LocalWords:  ifndef
 */
