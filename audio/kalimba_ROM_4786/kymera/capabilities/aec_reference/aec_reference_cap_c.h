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
#include "ps/ps.h"
/****************************************************************************
Public Type Declarations
*/
#define MAX_NUMBER_MICS     8
#define MAX_NUMBER_SPEAKERS 8

/* Capability Version */
#define AEC_REFERENCE_CAP_VERSION_MINOR            2

/* Enable speaker TTP gate if enabled by build */
#if defined(INSTALL_METADATA) && defined(INSTALL_AEC_REFERENCE_SPKR_TTP_GATE)
#define AEC_REFERENCE_SPKR_TTP_GATE
#ifndef AEC_REFERENCE_SUPPORT_METADATA
/* speaker TTP gate needs support of metadata */
#define AEC_REFERENCE_SUPPORT_METADATA
#endif
#include "ttp/timed_playback.h"
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

/* TODO Size this properly */
#define AEC_REF_RESAMPLER_TEMP_BUF_SIZE 64  /* 2* 48k * 625 usec */

#define AEC_NUM_SCRATCH_BUFFERS     16

#define CHAN_STATE_MIC_MASK   0x0F000F
#define CHAN_STATE_SPKR_MASK  0x000FF0


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

    unsigned task_period;       /**< Task Period */
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
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
    bool spkr_ttp_gate_enable;               /* shows that whether speaker ttp gate enabled by user */
    unsigned spkr_ttp_gate_state;            /* processing state */
    unsigned spkr_ttp_gate_counter;          /* counter for time gate is closed */
    unsigned spkr_ttp_gate_delay_period;     /* delay period in ms (or more accurately in number of ISRs)*/
    bool spkr_post_ttp_gate_drift_control;   /* post gate: enable drift control */
    int spkr_post_ttp_gate_average_error;    /* post gate: average error */
    int spkr_post_ttp_gate_active;           /* post gate: whether drift control is currently active */
    int spkr_post_ttp_gate_kp;               /* post gate: conversion factor from error to warp value */
#endif
#ifdef AEC_REFERENCE_SUPPORT_METADATA
    tCbuffer *spkr_input_metadata_buffer;    /* metadata buffer used for speaker graph input */
    TIME last_read_timestamp;                /* timestamp associated with last read sample */
    bool last_read_timestamp_valid;          /* whether last_read_timestamp is valid */
#endif
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

#ifdef AEC_REFERENCE_SPKR_TTP_GATE
bool aec_reference_opmsg_enable_spkr_ttp_gate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
void aec_reference_check_speaker_ttp_gate(AEC_REFERENCE_OP_DATA  *op_extra_data, TIME_INTERVAL error, unsigned *max_to_process);
int aec_reference_spkr_post_ttp_gate_fixing_value(AEC_REFERENCE_OP_DATA  *op_extra_data);
void aec_reference_spkr_ttp_gate_init(AEC_REFERENCE_OP_DATA  *op_extra_data);
#endif
#ifdef AEC_REFERENCE_SUPPORT_METADATA
bool aec_reference_get_spkr_ttp_error(AEC_REFERENCE_OP_DATA   *op_extra_data, TIME cur_time, TIME_INTERVAL *error);
void aec_reference_update_spkr_last_read_timestamp(AEC_REFERENCE_OP_DATA   *op_extra_data, unsigned amount_read);
#endif

extern void aec_reference_set_mic_gains(OPERATOR_DATA *op_data);
extern void aec_reference_cleanup(OPERATOR_DATA *op_data);
extern bool build_mic_graph(AEC_REFERENCE_OP_DATA *op_extra_data);
extern bool build_spkr_graph(AEC_REFERENCE_OP_DATA *op_extra_data);
extern bool validate_channels_and_build(OPERATOR_DATA *op_data);
extern bool aec_reference_stop_reset(OPERATOR_DATA *op_data,void **response_data);
extern bool ups_params_aec(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank, uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);

inline static bool IsSpeakerTerminal(unsigned term_idx)
{
    return ( (term_idx==AEC_REF_SPKR_TERMINAL1)
          || (term_idx==AEC_REF_SPKR_TERMINAL2)
          || (term_idx>=AEC_REF_SPKR_TERMINAL3) );
}
inline static bool IsSpeakerInputTerminal(unsigned term_idx)
{
    return ( (term_idx==AEC_REF_INPUT_TERMINAL1)
          || (term_idx==AEC_REF_INPUT_TERMINAL2)
          || (term_idx>=AEC_REF_INPUT_TERMINAL3));
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


#define SpeakerTerminalByIndex(idx)      ( (idx)<2 ?  AEC_REF_SPKR_TERMINAL1+(idx)  : (AEC_REF_SPKR_TERMINAL3-2)+(idx)  )
#define SpeakerInputTerminalByIndex(idx) ( (idx)<2 ?  AEC_REF_INPUT_TERMINAL1+(idx) : (AEC_REF_INPUT_TERMINAL3-2)+(idx) )

#endif  /* AEC_REFERENCE_CAP_C_H */


cbops_op* create_aec_ref_spkr_op(unsigned nr_inputs,unsigned* input_idx,unsigned in_threshold,unsigned nr_outputs,unsigned* output_idx,unsigned out_threshold);

unsigned get_aec_ref_cbops_inserts_total(cbops_op *op);
unsigned get_aec_ref_cbops_insert_op_insert_total(cbops_op *op);
unsigned get_aec_ref_cbops_wrap_op_insert_total(cbops_op *op);
