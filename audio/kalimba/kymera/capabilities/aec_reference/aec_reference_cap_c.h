/**
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 * \defgroup aec_reference_cap
 *
 * \file  aec_reference_cap_c.h
 *
 * AEC Reference Capability
 * \ingroup capabilities
 */
#ifndef AEC_REFERENCE_CAP_C_H
#define AEC_REFERENCE_CAP_C_H

/****************************************************************************
Include Files
*/

#include "capabilities.h"
#include "aec_reference_gen_c.h"
#include "aec_reference.h"
#include "aec_reference_latency.h"

#include "pl_timers/pl_timers.h"
#include "cbops/cbops_c.h"
#include "volume/shared_volume_control.h"
#include "op_msg_utilities.h"
#include "opmgr/opmgr_endpoint_override.h"
#ifdef INSTALL_DELEGATE_RATE_ADJUST_SUPPORT
#include "stream/stream_delegate_rate_adjust.h"
#endif
#ifdef CAPABILITY_DOWNLOAD_BUILD
#include "aec_reference_reuse_rom_functions.h"
#endif

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
/* speaker and mic shifts are done in HW */
#define AEC_REFERENCE_DEFAULT_EP_SHIFT 0
#else
#define AEC_REFERENCE_DEFAULT_EP_SHIFT 8
#endif

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
/* new amount that speaker consumed in calculated by op at
 * the start of handler. This way there will be less jitter
 * in the calculated rate, and better reference synchronisation
 * when SW rate adjustment is used. This is doable only for
 * HYDRA platforms.
 */
#define AEC_REF_CALC_SPKR_RATE_MONITOR_AMOUNT

/* This will enable running an inplace sidetone graph at low
 * period while mai task period is too high for sidetone latency
 *
 * Note: Not enabled yet.
 */
#define AEC_REFERENCE_CAN_PERFORM_INPLACE_SIDETONE_MIXING
#include "hydra_cbuff.h"
#ifdef CAPABILITY_DOWNLOAD_BUILD
#ifndef RUNNING_ON_KALSIM
/* cbuffer_get_read_offset in stre v02 ROM does exactly
 * what cbuffer_get_read_mmu_offset does (for MMU cbuffer inputs) */
#define cbuffer_get_read_mmu_offset cbuffer_get_read_offset
#endif
#endif
#endif /* CHIP_BASE_HYDRA || CHIP_BASE_NAPIER */
/****************************************************************************
Public Type Declarations
*/
#define MAX_NUMBER_MICS     8
#define MAX_NUMBER_SPEAKERS 8

/* Capability Version */
#define AEC_REFERENCE_CAP_VERSION_MINOR            6

/* Task period default and limits */
#define AEC_REFERENCE_DEFAULT_TASK_PERIOD          1000   /* default task period in us */
#define AEC_REFERENCE_MAX_TASK_PERIOD              5000   /* maximum task period in us */
#define AEC_REFERENCE_MIN_TASK_PERIOD              1000   /* minimum task period in us */

/* Enable speaker TTP playback if enabled by build */
#if defined(INSTALL_METADATA) && defined(INSTALL_AEC_REFERENCE_SPKR_TTP)
#define AEC_REFERENCE_SPKR_TTP
#ifndef AEC_REFERENCE_SUPPORT_METADATA
/* TTP support needs support of metadata */
#define AEC_REFERENCE_SUPPORT_METADATA
/* support generating timestamps for mic path */
#define AEC_REFERENCE_GENERATE_MIC_TIMESTAMP
#define AEC_REFERENCE_MIC_METADATA_MIN_TAG_LEN 60
#endif
#include "ttp/timed_playback.h"
#include "ttp/ttp.h"

#include "ttp_utilities.h"

#endif

#define AEC_REF_INPUT_TERMINAL1 0
#define AEC_REF_INPUT_TERMINAL2 1
#define AEC_REF_MIC_TERMINAL1   2
#define AEC_REF_MIC_TERMINAL2   3
#define AEC_REF_MIC_TERMINAL3   4
#define AEC_REF_MIC_TERMINAL4   5
#define AEC_REF_INPUT_TERMINAL3 6
#define AEC_REF_INPUT_TERMINAL4 7
#define AEC_REF_INPUT_TERMINAL5 8
#define AEC_REF_INPUT_TERMINAL6 9
#define AEC_REF_INPUT_TERMINAL7 10
#define AEC_REF_INPUT_TERMINAL8 11
#define AEC_REF_MIC_TERMINAL5   12
#define AEC_REF_MIC_TERMINAL6   13
#define AEC_REF_MIC_TERMINAL7   14
#define AEC_REF_MIC_TERMINAL8   15

#define AEC_REF_NUM_SINK_TERMINALS  16

#define AEC_REF_REFERENCE_TERMINAL  0
#define AEC_REF_SPKR_TERMINAL1      1
#define AEC_REF_SPKR_TERMINAL2      2
#define AEC_REF_OUTPUT_TERMINAL1    3
#define AEC_REF_OUTPUT_TERMINAL2    4
#define AEC_REF_OUTPUT_TERMINAL3    5
#define AEC_REF_OUTPUT_TERMINAL4    6
#define AEC_REF_SPKR_TERMINAL3      7
#define AEC_REF_SPKR_TERMINAL4      8
#define AEC_REF_SPKR_TERMINAL5      9
#define AEC_REF_SPKR_TERMINAL6      10
#define AEC_REF_SPKR_TERMINAL7      11
#define AEC_REF_SPKR_TERMINAL8      12
#define AEC_REF_OUTPUT_TERMINAL5    13
#define AEC_REF_OUTPUT_TERMINAL6    14
#define AEC_REF_OUTPUT_TERMINAL7    15
#define AEC_REF_OUTPUT_TERMINAL8    16


#define AEC_REF_NUM_SOURCE_TERMINALS    17

/* Resampler Inter-stage scratch buffer.

   Worse case when resampling from 48k to 44.1k the first
   stage does an upsample by a factor of 2.1.
      48000.0 (fs in : 21/10) --> 100800.0 (fs internal : 7/16) --> 44100.0 (fs out)

   Space for a period of 2 msec would be 201.6 samples (add a little cushion use 204)
*/
#define AEC_REF_RESAMPLER_TEMP_MIN_BUF_SIZE 204

#define AEC_NUM_SCRATCH_BUFFERS     16

#define CHAN_STATE_MIC_MASK   0x0F000F
#define CHAN_STATE_SPKR_MASK  0x000FF0


#define AEC_REFERENCE_REF_RATE_UPDATE_PERIOD 9 /* in number of timer period (1ms) */

typedef enum
{
    /* Sidetone isn't required or
     * it's irreleavan under current config,
     * (e.g. there is no speaker or no mic)
     */
    AEC_SIDETONE_NOT_REQUIRED,

    /* Sidetone mixing is being done in HW */
    AEC_SIDETONE_IN_HW,

    /* Sidetone mixing is done in SW in traditional way,
     * MIC graph provides sidetone samples and SPKR graph
     * mixes them into speaker.
     */
    AEC_SIDETONE_IN_SW_BY_MIC_SPKR_GRAPH,
#ifdef AEC_REFERENCE_CAN_PERFORM_INPLACE_SIDETONE_MIXING
    /* Sidetone mixing is done in SW using dedicated
     * sidetone graph, this method is used instead of
     * AEC_SIDETONE_IN_SW_BY_MIC_SPKR_GRAPH when speaker
     * and mic graphs are running in long task periods.
     * Available only for Hydra architectures
     */
    AEC_SIDETONE_IN_SW_USING_SEPARATE_GRAPH
#endif
}AEC_REFERENCE_SIDETONE_METHOD;

/* Extended data for Capability */
typedef struct aec_ref_root {
    tCbuffer *input_stream[AEC_REF_NUM_SINK_TERMINALS];          /**< Pointer to Sink Terminals  */
    tCbuffer *output_stream[AEC_REF_NUM_SOURCE_TERMINALS];       /**< Pointer to Source Terminals  */

    tCbuffer *scratch_bufs[AEC_NUM_SCRATCH_BUFFERS];            /**< Pointer to buffers  */

    tCbuffer  *sidetone_buf;                                    /**< Pointer to sidetone buffer  */
    unsigned  *resampler_temp_buffer;

    AEC_REFERENCE_PARAMETERS params;                            /**< Current parameters */

    SHARED_VOLUME_CONTROL_OBJ *shared_volume_ptr;               /**< Pointer to volume information */

    unsigned cap_id;            /**< Capability ID  */
    unsigned Cur_mode;          /**< Current operating mode  */
    unsigned Ovr_Control;       /**< OBPM override setting  */
    bool     ReInitFlag;        /**< Initialization flag  */

    tTimerId kick_id;           /**< Timer Task ID  */

    CPS_PARAM_DEF parms_def;

    unsigned task_period;       /* Operator's Task Period in microseconds,
                                 * set by user via operator messge, possible values are:
                                 * 1000, 1250, 1600, 2000, 2500, 3125, 4000 and 5000
                                 * default value=1000us
                                 */



    unsigned input_rate;        /**< Input Sample Rate */
    unsigned output_rate;       /**< Output Sample Rate */

    unsigned    mic_rate;               /**< Sink Sample Rate */
    unsigned    mic_rate_ability;       /**< Sink rate matching ability from IO*/
    unsigned    mic_rate_enactment;     /**< Sink rate matching - enabled*/
    unsigned    mic_rate_adjustment;    /**< Sink rate adjustment */
    cbops_graph *mic_graph;             /**< Pointer to Sink cbops graph */
    cbops_op    *mic_rate_monitor_op;   /**< Pointer to Sink cbops rate monitor  */
    cbops_op    *mic_sidetone_op;       /**< Pointer to cbops sidetone filter */
    int         mic_shift;              /**< Input data shift to 24 bit */

    unsigned   spkr_rate;               /**< Source Sample Rate  */
    unsigned   spkr_rate_ability;       /**< Source rate matching ability */
    unsigned   spkr_rate_adjustment;    /**< Source rate adjustment */
    unsigned   spkr_rate_enactment;     /**< Source rate matching - enabled*/
    cbops_graph *spkr_graph;            /**< Pointer to Source cbops graph */
    cbops_op    *spkr_rate_monitor_op;  /**< Pointer to Source cbops rate monitor */
    int         spkr_shift;             /**< Output data shift to 24 bit */
    unsigned    spkr_kick_size;         /**< Input threshold for kicks */

    unsigned    channel_status;         /**< Bitwise flag of terminal configuration */
    unsigned    num_spkr_channels;      /**< Number of speaker channels */
    unsigned    num_mic_channels;       /**< Number of MIC channels */

    unsigned    output_period_size;     /* Output data per period */

    unsigned   using_sidetone;          /* Global for enabling sidetone feature */

    unsigned   source_kicks;            /* mask of sources to kick */
    unsigned   sink_kicks;              /* mask of sinks to kick */

    cbops_op    *mic_sw_rateadj_op;     /**< Pointer to cbops sw rate adjustment operator */
    cbops_op    *spkr_sw_rateadj_op;    /**< Pointer to cbops sw rate adjustment operator*/

    OVERRIDE_EP_HANDLE  mic_endpoint;      /**< Handle for overridden mic endpoint connected*/
    OVERRIDE_EP_HANDLE  spkr_endpoint;     /**< Handle for overridden speaker endpoint connected*/

    OVERRIDE_EP_HANDLE  mic_out_endpoint;      /**< Handle for overridden mic endpoint connected*/
    OVERRIDE_EP_HANDLE  spkr_in_endpoint;     /**< Handle for overridden speaker endpoint connected*/

    unsigned    spkr_rate_meas;
    unsigned    mic_rate_meas;
    aec_latency_common sync_block;


	cbops_op    *ref_sw_rateadj_op;     /* cbops rate adjust for reference path */

	unsigned resampler_temp_buffer_size;                        /* buffer size for temp buffer used in resampler */

    unsigned task_frequency;    /* Operator's main task runs per second */
    unsigned task_period_frac;  /* Task period as a fraction of second */
    unsigned kick_period;       /* Kick period = task_period/decim_factor */
    unsigned max_io_rate;       /* max rate among all mics/spkrs/inputs and outputs*/
    unsigned kick_period_frac;  /* Kick period as a fraction of second (used for sidetone graph) */
#ifdef AEC_REFERENCE_CAN_PERFORM_INPLACE_SIDETONE_MIXING
    unsigned task_decim_factor; /* decimation factor for task period to
                                 * run sidetone mixing more frequently */
    unsigned task_decim_counter; /* counter holding decimation state */
    cbops_graph *sidetone_graph; /* standalone cbops graph for sidetone mixing */
    tCbuffer *sidetone_mic_buf;  /* clone cbuffer structure for mic input to sidetone graph */
#endif
    bool hw_sidetone_available;  /* Whether hw sidetone mixing is available (not implemented yet)*/

    AEC_REFERENCE_SIDETONE_METHOD sidetone_method; /* current sidetone mixing method */

    cbops_op    *mic_mute_op;           /* Pointer to cbops mute operator */
#ifdef ENABLE_SW_DAC_DC_OFFSET_COMPENSATION
    /* dc offset op will be added to speaker graph,
     * if HW has enabled some dc offset.
     */
    cbops_op       *spkr_dc_offset_op;
#endif
    bool        spkr_sidetone_active;   /* flag showing speaker graph is mixing sidetone */
    bool        spkr_ref_active;        /* flag showing speaker graph is outputting reference signal */
    unsigned    spkr_ref_input_idx;     /* the buffer index used for reference path */
    unsigned    spkr_ref_scratch_idx;   /* the scratch buffer used in reference sub path */
    unsigned    spkr_stmix_in_idx;      /* buffer index where sidetone mix gets its input */
    unsigned    spkr_ref_idx;           /* buffer index reserved for reference output */
    unsigned    spkr_st_in_idx;         /* buffer index reserved for sidetone input */
    unsigned    spkr_num_ref_ops;       /* number of operators in reference sub-path */
    unsigned    spkr_ref_rs_idx;        /* buffer index used for resampler in reference path */
    cbops_op    *spkr_ref_point_op;     /* cbops operator to insert reference path after */
    cbops_op    *spkr_st_point_op;      /* cbops operator to insert sidetone part after */
    cbops_op    *spkr_ref_last_op;      /* last cbops operator in speaker graph's ref path */
    cbops_op    *spkr_stmix_op;         /* speaker sidetone mix operator */
    cbops_op    *spkr_ref_rs_op;        /* cbops op for resampler in reference path */

#ifdef INSTALL_DELEGATE_RATE_ADJUST_SUPPORT
    unsigned mic_ext_rate_adjust_op;  /**< external rate adjust operator for using by mic path */
    unsigned spkr_ext_rate_adjust_op; /**< external rate adjust operator for using by spkr path */
#endif

    unsigned spkr_in_threshold;              /* minimum expected amount in the speaker input buffer */
    unsigned spkr_out_threshold;             /* expected amount in the speaker output buffer */
    unsigned spkr_sink_delay;                /* extra latency that happens after samples leaving
                                              * speaker audio buffer, e.g. in an external coded */
#ifdef AEC_REFERENCE_SPKR_TTP
    bool spkr_timed_playback_mode;           /* flag showing whether speaker graph is in timed playback mode*/
    TIME spkr_last_timestamp;                /* timestamp associated with last read sample */
    bool spkr_last_timestamp_valid;          /* whether spkr_last_timestamp is valid */
    bool spkr_void_tag_observed;             /* flag shows last time looked at metadata a void tag seen at head */
    unsigned spkr_void_tag_counter;          /* counter showing the duration we have seen void tags */
    TIME_INTERVAL spkr_error_threshold;      /* current error threshold for TTP playback */
    TIME_INTERVAL spkr_last_ttp_error;       /* last ttp error */
    int spkr_ttp_error_acc;                  /* accumulator for ttp error averaging */
    unsigned spkr_ttp_error_acc_cnt;         /* counter for ttp error averaging */
    unsigned spkr_extra_delay;               /* total delay to add when computing ttp error, this will be
                                              * spkr_sink_delay plus extra internal delays like algorithmic
                                              * delay in cbops processing */
    unsigned spkr_ttp_adjust_type;           /* doing HW or SW rate adjust, separate for TTP for further
                                              * flexibility */

    /** Internal settings of the PID controller - Tx only */
    pid_controller_settings spkr_pid_params;

    /** The PID controller internal state - Tx only */
    pid_controller_state spkr_pid_state;

#endif
#ifdef AEC_REFERENCE_SUPPORT_METADATA
    tCbuffer *spkr_input_metadata_buffer;    /* metadata buffer used for speaker graph input */
#endif
#ifdef AEC_REFERENCE_GENERATE_MIC_TIMESTAMP
    tCbuffer *mic_metadata_buffer;           /* metadata buffer for mic outputs */
    unsigned mic_metadata_tag_left_words;    /* words left to complete previously written tag */
    unsigned mic_metadata_min_tag_len;       /* minimum tag len for mic output metadata */
    bool mic_metadata_enable;                /* flag showing whether mic metadata is supported */
    unsigned mic_target_latency;             /* target latency for mic output timestamp tags */
    ttp_context *mic_time_to_play;           /* time to play context */
#endif /* AEC_REFERENCE_GENERATE_MIC_TIMESTAMP */

    unsigned ref_update_counter;             /* counter to decimate updating of the rate for reference path */
#ifdef AEC_REF_CALC_SPKR_RATE_MONITOR_AMOUNT
    unsigned spkr_last_offset;               /* previous buffer offset */
    unsigned spkr_new_amount;                /* new amount read by speaker */
    bool spkr_flow_started;                  /* speaker has started consuming */
#endif
    bool mic_sync_enable;                    /* flag showing backend input and output have
                                              * same clock so we want to sync mic to spkr input
                                              */
    unsigned output_buffer_size;             /* required buffer size for output terminals, if 0 default
                                              * value of 8.7ms will be used */
    unsigned input_buffer_size;              /* required buffer size for input terminals, if 0 default
                                              * value of 3ms will be used */
} AEC_REFERENCE_OP_DATA;

/****************************************************************************
Private Function Definitions
*/

/*  add connection statistic
        4 sample rates
        terminal connectivity word

        | A1 | A2 | SPKR channels (8-bits) | MIC channels (4-bits)

        A1:  0 = full SPKR Channels, 1=channel Mixing/Split
        A2:  0 = Mixing, 1 = Splitting

    */


/* Receive capability handler functions declarations */
extern bool aec_reference_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool aec_reference_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

#ifdef CAPABILITY_DOWNLOAD_BUILD

#ifdef RUNNING_ON_KALSIM

#define set_aec_reference_not_running_from_rom() TRUE

#else
    /* let the patches know it's not running from ROM */
    extern void set_aec_reference_not_running_from_rom(void);
#endif /* RUNNING_ON_KALSIM */
#endif /* CAPABILITY_DOWNLOAD_BUILD */

/* Data processing function */
extern void aec_reference_timer_task(void *kick_object);

/* Operator message handlers */
extern bool aec_reference_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool aec_reference_opmsg_ep_get_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_ep_configure(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_ep_clock_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool aec_reference_set_rates(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_mute_mic_output(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_set_task_period(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_set_task_period(AEC_REFERENCE_OP_DATA *op_extra_data, unsigned task_period, unsigned decim_factor);


extern bool aec_reference_opmsg_enable_mic_sync(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data,
                                         unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern void aec_reference_update_sidetone_status(AEC_REFERENCE_OP_DATA *op_extra_data);
#ifdef AEC_REFERENCE_SPKR_TTP
extern void aec_reference_spkr_ttp_run(AEC_REFERENCE_OP_DATA  *op_extra_data, unsigned *max_to_process);
extern void aec_reference_spkr_ttp_init(AEC_REFERENCE_OP_DATA  *op_extra_data);
extern void aec_reference_spkr_ttp_update_last_timestamp(AEC_REFERENCE_OP_DATA   *op_extra_data, unsigned amount_read);
extern void aec_reference_spkr_ttp_terminate(AEC_REFERENCE_OP_DATA  *op_extra_data);
#endif

#ifdef INSTALL_DELEGATE_RATE_ADJUST_SUPPORT
extern void aec_reference_spkr_check_external_rate_adjust_op(AEC_REFERENCE_OP_DATA *op_extra_data);
extern void aec_reference_mic_check_external_rate_adjust_op(AEC_REFERENCE_OP_DATA *op_extra_data);
#endif

#ifdef AEC_REFERENCE_GENERATE_MIC_TIMESTAMP
extern bool aec_reference_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data,
                                     unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_set_ttp_params(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aec_reference_opmsg_set_latency_limits(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern void aec_reference_mic_generate_metadata_with_ttp(AEC_REFERENCE_OP_DATA *op_extra_data, unsigned samples);
#endif /* AEC_REFERENCE_GENERATE_MIC_TIMESTAMP */

extern void aec_reference_set_mic_gains(OPERATOR_DATA *op_data);
extern void aec_reference_cleanup(OPERATOR_DATA *op_data);
extern bool build_mic_graph(AEC_REFERENCE_OP_DATA *op_extra_data);
extern bool build_spkr_graph(AEC_REFERENCE_OP_DATA *op_extra_data);
extern bool validate_channels_and_build(OPERATOR_DATA *op_data);
extern bool aec_reference_update_mic_channel_status(AEC_REFERENCE_OP_DATA *op_extra_data);
extern bool aec_reference_update_spkr_channel_status(AEC_REFERENCE_OP_DATA *op_extra_data);
extern bool aec_reference_update_ref_channel_status(AEC_REFERENCE_OP_DATA *op_extra_data);
extern void aec_reference_update_sidetone_method(AEC_REFERENCE_OP_DATA *op_extra_data);

extern bool aec_reference_build_graphs(AEC_REFERENCE_OP_DATA* op_extra_data, bool build_spkr_graph, bool build_mic_graph);

extern bool aec_reference_spkr_include_sidetone(AEC_REFERENCE_OP_DATA* op_extra_data, bool include_sidetone);
extern void aec_reference_cleanup_mic_graph(AEC_REFERENCE_OP_DATA *op_extra_data);
extern void aec_reference_cleanup_graphs(AEC_REFERENCE_OP_DATA *op_extra_data);
extern void aec_reference_cleanup_spkr_graph(AEC_REFERENCE_OP_DATA *op_extra_data);
extern bool aec_reference_spkr_include_ref_path(AEC_REFERENCE_OP_DATA* op_extra_data, bool include_ref_path);


extern bool aec_reference_stop_reset(OPERATOR_DATA *op_data,void **response_data);
extern bool ups_params_aec(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank, uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);
extern void aec_reference_update_mic_reference_sync(AEC_REFERENCE_OP_DATA * op_extra_data);

inline static bool IsSpeakerTerminal(unsigned term_idx)
{
    return ( (term_idx==AEC_REF_SPKR_TERMINAL1)
          || (term_idx==AEC_REF_SPKR_TERMINAL2)
          || ( (term_idx>=AEC_REF_SPKR_TERMINAL3)
              && (term_idx<=AEC_REF_SPKR_TERMINAL8) ) );
}
inline static bool IsSpeakerInputTerminal(unsigned term_idx)
{
    return ( (term_idx==AEC_REF_INPUT_TERMINAL1)
          || (term_idx==AEC_REF_INPUT_TERMINAL2)
          || ( (term_idx>=AEC_REF_INPUT_TERMINAL3)
              && (term_idx<=AEC_REF_INPUT_TERMINAL8) ) );
}
inline static bool IsMicrophoneTerminal(unsigned term_idx)
{
    term_idx &= ~TERMINAL_SINK_MASK;

    if(term_idx < AEC_REF_MIC_TERMINAL1)
    {
        return FALSE;
    }
    if(term_idx <= AEC_REF_MIC_TERMINAL4)
    {
        return TRUE;
    }
    return (term_idx >= AEC_REF_MIC_TERMINAL5);

}

inline static bool IsMicrophoneOutputTerminal(unsigned term_idx)
{
    return ((AEC_REF_OUTPUT_TERMINAL1 <= term_idx &&
            AEC_REF_OUTPUT_TERMINAL4 >= term_idx) ||
            (AEC_REF_OUTPUT_TERMINAL5 <= term_idx &&
                         AEC_REF_OUTPUT_TERMINAL8 >= term_idx)
            );
}


inline static unsigned MicrophoneTerminalByIndex(unsigned i)
{
    if(i<4)
    {
        return (i + AEC_REF_MIC_TERMINAL1);
    }
    else
    {
        return ((i-4)+ AEC_REF_MIC_TERMINAL5);
    }
}

inline static unsigned OutputTerminalByIndex(unsigned i)
{
    if(i<4)
    {
        return (i + AEC_REF_OUTPUT_TERMINAL1);
    }
    else
    {
        return ((i-4) + AEC_REF_OUTPUT_TERMINAL5);
    }
}

/* macro to get a speaker terminal index by speaker number,
 * they aren't contiguous - see definitions of AEC_REF_SPKR_TERMINAL
 * numbering at the beginning of this file.
 */
#define SpeakerTerminalByIndex(idx)      ( (idx)<2 ?  AEC_REF_SPKR_TERMINAL1+(idx)  : (AEC_REF_SPKR_TERMINAL3-2)+(idx)  )

/* macro to get a speaker input terminal index by speaker number,
 * they aren't contiguous - see definitions of AEC_REF_INPUT_TERMINAL
 * numbering at the beginning of this file.
 */
#define SpeakerInputTerminalByIndex(idx) ( (idx)<2 ?  AEC_REF_INPUT_TERMINAL1+(idx) : (AEC_REF_INPUT_TERMINAL3-2)+(idx) )

/* macro to get a mic output terminal index by mic number,
 * they aren't contiguous - see definitions of AEC_REF_OUTPUT_TERMINAL
 * numbering at the beginning of this file.
 */
#define MicrophoneOutputTerminalByIndex(idx) ( (idx)<2 ?  AEC_REF_OUTPUT_TERMINAL1+(idx) : (AEC_REF_OUTPUT_TERMINAL5-4)+(idx) )

/* Utility macros to get/set channel status for speaker, microhone and reference paths */
#define MIC_CHSTS_MASK  CHAN_STATE_MIC_MASK
#define SPKR_CHSTS_MASK (CHAN_STATE_SPKR_MASK                  |  \
                         AEC_REFERENCE_CONSTANT_CONN_TYPE_PARA |  \
                         AEC_REFERENCE_CONSTANT_CONN_TYPE_MIX)
#define REFERENCE_CHSTS_MASK AEC_REFERENCE_CONSTANT_CONN_TYPE_REF
#define GetMicChannelStatus(p)    ((p->channel_status) & MIC_CHSTS_MASK)
#define SetMicChannelStatus(p,x)  p->channel_status = (((p->channel_status) & (~MIC_CHSTS_MASK))|(x))
#define GetSpkrChannelStatus(p)   ((p->channel_status) & SPKR_CHSTS_MASK)
#define SetSpkrChannelStatus(p,x) p->channel_status = (((p->channel_status) & (~SPKR_CHSTS_MASK))|(x))
#define GetRefChannelStatus(p)   ((p->channel_status) & REFERENCE_CHSTS_MASK)
#define SetRefChannelStatus(p,x) p->channel_status = (((p->channel_status) & (~REFERENCE_CHSTS_MASK))|(x))

#endif  /* AEC_REFERENCE_CAP_C_H */


cbops_op* create_aec_ref_spkr_op(unsigned nr_inputs,unsigned* input_idx,unsigned in_threshold,unsigned nr_outputs,unsigned* output_idx,unsigned out_threshold, unsigned max_jitter);
#ifdef AEC_REFERENCE_CAN_PERFORM_INPLACE_SIDETONE_MIXING
extern cbops_op* create_aec_ref_sidetone_op(tCbuffer *mic_buf,
                                     unsigned mic_idx,
                                     tCbuffer **spkr_bufs,
                                     unsigned nr_spkrs,
                                     unsigned *sidetone_idx,
                                     unsigned spkr_threshold,
                                     unsigned safety_threshold);
extern void aec_ref_sidetone_initialise(cbops_op *);
extern bool build_sidetone_graph(AEC_REFERENCE_OP_DATA* op_extra_data);
extern void aec_reference_cleanup_sidetone_graph(AEC_REFERENCE_OP_DATA *op_extra_data);
#endif
unsigned get_aec_ref_cbops_inserts_total(cbops_op *op);
unsigned get_aec_ref_cbops_insert_op_insert_total(cbops_op *op);
unsigned get_aec_ref_cbops_wrap_op_insert_total(cbops_op *op);
