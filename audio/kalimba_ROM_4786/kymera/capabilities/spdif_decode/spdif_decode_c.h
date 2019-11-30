/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef SPDIF_DECODE_C_H_INCLUDED
#define SPDIF_DECODE_C_H_INCLUDED

/* Maximum supported sample rate
 * TODO: 192khz also supported in hydra archs */
#define SPDIF_DECODE_MAX_RATE 96000

/* The input buffer shall be able to buffer this amount of audio */
#define SPDIF_DECODE_MIN_INPUT_BUFFER_MS 10

/* Minimum buffer size for two-channel config,
 * x2 to consider interleaved mode */
#define SPDIF_DECODE_INPUT_BUFFER_SIZE   ((SPDIF_DECODE_MIN_INPUT_BUFFER_MS*SPDIF_DECODE_MAX_RATE*2)/1000)

/* Buffer size for PCM output channels */
#define SPDIF_DECODE_OUTPUT_BUFFER_SIZE  384

/* Buffer size for CODED output channels */
#define SPDIF_DECODE_CODED_BUFFER_SIZE   1973

/* Maximum time we wait for decoder
 * to consume the remaining inputs
 */
#define SPDIF_DECODE_MAX_WAIT_FOR_DECODER_DONE 10000
#define SPDIF_DECODE_MAX_WAIT_FOR_CLIENT_RESPONSE 3000000

/* macro used to check whether client has successfully loaded
 * a requested decoder
 */
#define SPDIF_DECODE_DECODER_LOAD_FAILED(dt,dl_dt) ((dt != dl_dt) && (dt==(dl_dt&0x7FFF)))
/* define output terminals */
typedef enum
{
    SPDIF_DECODE_SOURCE_CODED_TERMINAL = 0,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_0 = 1,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_1 = 2,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_2 = 3,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_3 = 4,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_4 = 5,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_5 = 6,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_6 = 7,
    SPDIF_DECODE_SOURCE_PCM_TERMINAL_7 = 8,
    SPDIF_DECODE_SOURCE_MAX_OUTPUT_TERMINALS
} SPDIF_DECODE_SOURCE_TERMINAL;
#define SPDIF_DECODE_SOURCE_PCM_TERMINAL_LEFT SPDIF_DECODE_SOURCE_PCM_TERMINAL_0
#define SPDIF_DECODE_SOURCE_PCM_TERMINAL_RIGHT SPDIF_DECODE_SOURCE_PCM_TERMINAL_1
#define SPDIF_DECODE_SOURCE_IS_DECODER_TERMINAL(id) ((id) == SPDIF_DECODE_SOURCE_CODED_TERMINAL)
#define SPDIF_DECODE_SOURCE_IS_PCM_TERMINAL(id) (((id) < SPDIF_DECODE_SOURCE_MAX_OUTPUT_TERMINALS) && (id >= SPDIF_DECODE_SOURCE_PCM_TERMINAL_LEFT))

/* macro to touch all pcm output terminals */
#define TOUCH_ALL_PCM_OUTPUTS_TERMINALS(sp_op_data) (((1<<sp_op_data->nrof_output_channels)-1)<<SPDIF_DECODE_SOURCE_PCM_TERMINAL_0)

/* Maximum number of output PCM channels */
#define SPDIF_DECODE_MAX_OUTPUT_CHANNELS (SPDIF_DECODE_SOURCE_MAX_OUTPUT_TERMINALS-1)

/* define input terminals */
typedef enum
{
    SPDIF_DECODE_SINK_EP_TERMINAL = 0,
    SPDIF_DECODE_SINK_EP2_TERMINAL = 1,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_0 = 2,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_1 = 3,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_2 = 4,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_3 = 5,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_4 = 6,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_5 = 7,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_6 = 8,
    SPDIF_DECODE_SINK_DECODER_TERMINAL_7 = 9,
    SPDIF_DECODE_SINK_MAX_INPUT_TERMINALS
} SPDIF_DECODE_SINK_TERMINAL;
#define SPDIF_DECODE_SINK_IS_DECODER_TERMINAL(id) ((id) >= SPDIF_DECODE_SINK_DECODER_TERMINAL_0)
#define SPDIF_DECODE_SINK_IS_EP_TERMINAL(id) ((id) <= SPDIF_DECODE_SINK_EP2_TERMINAL)

/* Define data types */
typedef enum
{
   SPDT_UNKNOWN = 0,
   SPDT_MUTED =   1,
   SPDT_PCM   =   2,
   SPDT_TYPE_NULL = 3,
   SPDT_TYPE_AC3  = 4,
   SPDT_TYPE_PAUSE = 5,
   SPDT_TYPE_MPEG1_LAYER1 = 6,
   SPDT_TYPE_MPEG1_LAYER23 = 7,
   SPDT_TYPE_MPEG2_EXTENTION = 8,
   SPDT_TYPE_MPEG2_AAC = 9,
   SPDT_TYPE_MPEG2_LAYER1_LSF = 10,
   SPDT_TYPE_MPEG2_LAYER2_LSF = 11,
   SPDT_TYPE_MPEG2_LAYER3_LSF = 12,
   SPDT_TYPE_DTS1 = 13,
   SPDT_TYPE_DTS2 = 14,
   SPDT_TYPE_DTS3 = 15,
   SPDT_TYPE_ATRAC = 16,
   SPDT_TYPE_ATRAC3 = 17,
   SPDT_TYPE_ATRACX = 18,
   SPDT_TYPE_DTSHD  = 19,
   SPDT_TYPE_WMAPRO = 20,
   SPDT_TYPE_MPEG2_AAC_LSF = 21,
   SPDT_TYPE_EAC3 = 22,
   SPDT_TYPE_TRUEHD = 23,
   SPDT_TYPE_USER_DEFINED = 34
} SPDIF_DATA_TYPE;

/* macro to check whether data type is uncompressed */
#define SPDIF_DATA_TYPE_IS_PCM(dt) ((dt <= SPDT_TYPE_NULL)||(dt==SPDT_TYPE_PAUSE))

/* define operator's state */
typedef enum
{
   SPOS_NORMAL_PCM,
   SPOS_NORMAL_CODED,
   SPOS_WAITING_RATE_ADAPT,
   SPOS_WAITING_DECODER_DONE,
   SPOS_WAITING_DATA_TYPE_ADAPT
} SPDIF_OP_STATE;

/* structure for S/PDIF decode operator data */
typedef struct
{
    /* state of the operator */
    SPDIF_OP_STATE state;

    /* output sample rate confirmed by the client */
    unsigned output_sample_rate;

    /* A flag showing decoder has finished
     * processing of all coded audio, is used when
     * switching from a coded data type
     */
    bool decoder_done;

    /* the data type that is supported by current
     * loaded decoder. This is used to speed up
     * the transition process when switching from
     * a coded data type to uncompressed audio.
     */
    SPDIF_DATA_TYPE current_decoder_data_type;

    /* operator id for current running decoder */
    unsigned decoder_opid;

    /* decoder number of output channels */
    unsigned decoder_nrof_output_channels;

    /* Minimum input buffer size requested
     * by the client, this is to make sure
     * we can achieve a target latency.
     */
    unsigned min_input_buffer_size;

    /* An internal timer used for limiting the
     * time we stay in one transient state, e.g.
     * when data type changes, we wait for a
     * certain amount of time for the current
     * decoder to consume all the inputs and
     * produces last audio samples.
     */
    TIME timeout_start_time;

    /* input buffers from a running decoder */
    tCbuffer* decoded_output_buffers[SPDIF_DECODE_MAX_OUTPUT_CHANNELS];

    /* Anything below this point will be used by asm
     * frame_decode function, so if any field is updated
     * the corresponding asm file needs also being updated
     */
    bool valid;
    unsigned op_sample_rate;
    tCbuffer* input_buffer;
    tCbuffer* input_buffer_b;
    unsigned nrof_output_channels;
    tCbuffer* output_buffer_pcm_left;
    tCbuffer* output_buffer_pcm_right;
    tCbuffer* extra_output_buffers[SPDIF_DECODE_MAX_OUTPUT_CHANNELS -2];
    tCbuffer* output_buffer_coded;
    unsigned supported_data_types[2];
    unsigned user_data_type_period;
    bool reset_needed;
    SPDIF_DATA_TYPE op_data_type;
    bool chsts_data_mode;
    SPDIF_DATA_TYPE ep_data_type;
    unsigned invalid_burst_count;
    unsigned null_burst_count;
    unsigned pause_burst_count;
    unsigned unsupported_burst_count;
    unsigned extra_fields[20];
}SPDIF_DECODE_OP_DATA;

/* declaring functions implemented in ASM */
/**
 * spdif_soft_mute_output_buffers
 * \brief soft mute output buffers in transitions
 *
 * \param buffers list of syncronised buffers to soft mute
 * \param nrof_buffers number of buffers in the list
 * \param max_fade_samples max samples to fade, the rest will be muted
 * \param offset_sample offset, fading starts from this sample
 *
 * \return number of samples faded out
 */
extern unsigned spdif_soft_mute_output_buffers(tCbuffer** buffers, unsigned nrof_buffers, unsigned max_fade_samples, unsigned offset_sample);

/**
 * spdif_copy_channels
 * \brief copy a set of input channels to output channels, used for
 *        routing the decoder output to PCM channels
 *
 * \param inp_buffers array of input channels
 * \param out_buffers array of output channels
 * \param nchans max number of channels
 * \param max_copy max samples to copy
 *
 * \return number of samples faded out
 */
extern unsigned spdif_copy_channels(tCbuffer** inp_buffers, tCbuffer** out_buffers, unsigned nchans, unsigned max_copy);
extern unsigned spdif_frame_decode (void *);

/****************************************************************************
Private Function Declarations
*/
extern bool spdif_decode_get_data_format (OPERATOR_DATA * op_data,void *message_data, unsigned *response_id, void **response_data);
extern bool spdif_decode_start (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool spdif_decode_reset (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool spdif_decode_create (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool spdif_decode_connect (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool spdif_decode_disconnect (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool spdif_decode_buffer_details (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool spdif_decode_get_sched_info (OPERATOR_DATA * op_data, void *message_data, unsigned *response_id, void **response_data);
extern void spdif_decode_process_data (OPERATOR_DATA * op_data, TOUCHED_TERMINALS *touched);
extern bool spdif_decode_set_output_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool spdif_decode_set_supported_data_types(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool spdif_decode_set_decoder_data_type(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool spdif_decode_new_chsts_from_ep(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern void spdif_decode_send_message_to_the_client(OPERATOR_DATA *op_data, unsigned msg_id, unsigned length, const unsigned *payload);
extern void spdif_decode_signal_reset_to_decoder(SPDIF_DECODE_OP_DATA *spdif_decode_data);
extern void spdif_decode_signal_end_of_input_to_decoder(SPDIF_DECODE_OP_DATA *spdif_decode_data);
extern void spdif_decode_update_nrof_output_channels(SPDIF_DECODE_OP_DATA *spdif_decode_data);
extern void spdif_decode_process_state (OPERATOR_DATA * op_data);
extern void spdif_decode_reset_operator_state (SPDIF_DECODE_OP_DATA *spdif_decode_data);
extern tCbuffer **spdif_decode_get_buffer_from_terminal_id(OPERATOR_DATA * op_data, unsigned terminal_id, bool *can_change);

#endif /* #ifndef SPDIF_DECODE_C_H_INCLUDED */
