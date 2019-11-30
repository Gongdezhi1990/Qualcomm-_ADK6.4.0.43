/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 *
 * \defgroup endpoints Endpoints
 * \ingroup stream
 *
 * \file stream_endpoint.h
 *
 */

#ifndef _STREAM_ENDPOINT_H_
#define _STREAM_ENDPOINT_H_

/****************************************************************************
Include Files
*/
#if defined(INSTALL_HYDRA) || defined(CHIP_BASE_NAPIER)
#include "audio.h"
#endif /* INSTALL_HYDRA */

#ifdef CHIP_BASE_A7DA_KAS
#include "kas_audio.h"
#endif

#include "stream/stream.h"
#include "stream_kick_obj.h"
#include "sched_oxygen/sched_oxygen.h"
#ifdef INSTALL_SCO
#include "sco_data_mgr.h"
#endif /* INSTALL_SCO */
#include "cbops_mgr/cbops_mgr.h"
#include "util.h"

#include "pl_timers/pl_timers.h"


#ifdef TIMED_PLAYBACK_MODE
#include "ttp/timed_playback.h"
#endif
#include "rate/rate.h"

/****************************************************************************
Private Constant Declarations
*/

/** Number of consecutive underruns that make us decide it is a gap / pause in the data
 *  stream.
 */
#define MAX_CONSECUTIVE_UNDERRUNS       3
#define MAX_CONSECUTIVE_UNDERRUNS_BITS  2

/** Number of consecutive updates that yield average perceived data block at sink input */
#define NR_DATA_BLOCK_UPDATES           8
#define NR_DATA_BLOCK_UPDATES_BITS      4

/** Bit mask to identify internal endpoint configure keys
 * (EP_DATA_FORMAT etc.)
 */
#define ENDPOINT_INT_CONFIGURE_KEYS_MASK    (0x10000)

/****************************************************************************
Private Type Declarations
*/
/** Enumeration of keys for the XAP_KAL_MSG_AUDIO_INFO_REQ message.
 *  Note this enum appears to have been forked and extended in ACCMD_INFO_KEY.
 */
typedef enum
{
    /** Key to request the audio sample rate */
    AUDIO_CONFIG_SAMPLE_RATE = 0x0000,
    /** Key to request whether the audio hardware is locally or remotely clocked */
    AUDIO_CONFIG_LOCALLY_CLOCKED = 0x0001,
    /** Key to request the hardware instance of an audio sink/source */
    AUDIO_CONFIG_INSTANCE = 0x0002,                       /*!< Request the hardware instance of an audio sink/source */
    /** Key to request the hardware channel of an audio sink/source */
    AUDIO_CONFIG_CHANNEL = 0x0003,
    /** Key to request the Audio Sample format, i.e. 8,13,16 or 24 bit */
    AUDIO_CONFIG_SAMPLE_FORMAT = 0x0004

} AUDIO_CONFIG;

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
/* Enumeration of bit positions for ACCMD_RATE_MATCH_PARAMETER_ENABLE
 */
enum
{
    RATE_MATCH_ENABLE_HW = 0,
    RATE_MATCH_ENABLE_SW,
    RATE_MATCH_ENABLE_CLRM_MEASURE,
    RATE_MATCH_ENABLE_CLRM_MATCH,
    NUMBER_RATE_MATCH_ENABLE_BITS
};
#endif /* CHIP_BASE_HYDRA */


/****************************************************************************
Private Type Declarations
*/

/**
 * Internal endpoint configure keys
 */
/* These ids have to be different from other keys used in different
 * endpoints, to avoid problems set the 17th bit
 * (all other keys are 16 bits). */
typedef enum
{
    /* cbops manager IDs for selecting which cbops operation to run. */

    /** Get the data format the endpoint is currently configured for */
    EP_DATA_FORMAT = 0x010000,

    /** Endpoint kick period */
    EP_KICK_PERIOD,

    /** Endpoint block size */
    EP_BLOCK_SIZE,

    /** Processing time of upstream operator */
    EP_PROC_TIME,

    /** Flag to tell an operator endpoint that it's kicked by a hard deadline
     * and hence shouldn't run when kicked from upstream
     */
    EP_KICKED_FROM_ENDPOINT,

    /** Configure the override flag for the endpoint and the size of buffer
     * which will be supplied by the overridden endpoint.  Override will be
     * automatically disabled during disconnect. Because of this reason,  "clear" key
     * is not necessary  for this feature.
     * Note: Under Bluecore the size  is ignored.*/
    EP_OVERRIDE_ENDPOINT,

    /** This key is used to get and configure the cbops parameters of an endpoint. */
    EP_CBOPS_PARAMETERS,

    /** Request the type of ratematching support the endpoint possesses. */
    EP_RATEMATCH_ABILITY,

    /** Ratematching enactment setting enable(d)/disable(d) */
    EP_RATEMATCH_ENACTING,

    /** Request the measured rate used for ratematching */
    EP_RATEMATCH_RATE,

    /** Apply a rate adjustment value */
    EP_RATEMATCH_ADJUSTMENT,

    /** Used by the operator endpoint only.*/
    EP_DETAILS,

    /* spdif endpoint sample can change. */
    EP_SAMPLE_RATE,

    /* spdif can have different channel types */
    EP_CHANNEL_ORDER,


    /** Set input hardware gain in platform-independent units
     * (1/60dB steps as a signed number in 32-bit 2's complement,
     * relative to some possibly platform-dependent level). */
    EP_SET_INPUT_GAIN,

    /** Set output hardware gain in platform-independent units
     * (1/60dB steps as a signed number in 32-bit 2's complement,
     * relative to some possibly platform-dependent level). */
    EP_SET_OUTPUT_GAIN,

    /* Set the KIP shadow endpoint data channel */
    EP_SET_DATA_CHANNEL,

    /* Block KIP endpoint to supply remote buffer */
    EP_CLONE_REMOTE_BUFFER,

    /* Shadow endpoint buffer size */
    EP_SET_SHADOW_BUFFER_SIZE,

#ifdef INSTALL_METADATA_DUALCORE
    /* Shadow endpoint metadata flag. */
    EP_METADATA_SUPPORT,

    /* Shadow endpoint metadata data channel ID */
    EP_METADATA_CHANNEL_ID,

    /* Shadow endpoint metadata data channel buffer. */
    EP_METADATA_CHANNEL_BUFFER,
#endif

    /** Ratematch non-enacting EP measurement */
    EP_RATEMATCH_MEASUREMENT,

    /** Ratematch enacting reference */
    EP_RATEMATCH_REFERENCE,

} ENDPOINT_INT_CONFIGURE_KEYS;

/**
 * Structure for extended EP_RATEMATCH_REFERENCE parameters
 */
typedef struct
{
    /** Approximate (averaged) sample period deviation */
    int                 sp_deviation;

    /** Reference rate in rate library format */
    RATE_RELATIVE_RATE  ref;

    /** Handle to get information about the non-enacting endpoint */
    unsigned int        ref_endpoint_id;

} ENDPOINT_RATEMATCH_REFERENCE_PARAMS;

/**
 * Structure for extended EP_RATEMATCH_MEASUREMENT results
 */
typedef struct
{
    /** Approximate (averaged) sample period deviation */
    int                 sp_deviation;

    /** Rate measurement in rate library format */
    RATE_RELATIVE_RATE  measurement;

} ENDPOINT_RATEMATCH_MEASUREMENT_RESULT;

/**
 * Structure to contain get_config results
 */
typedef struct
{
    union {
        /** The default case */
        uint32                          value;

        /** EP_RATEMATCH_MEASUREMENT */
        ENDPOINT_RATEMATCH_MEASUREMENT_RESULT  rm_meas;
    } u;
} ENDPOINT_GET_CONFIG_RESULT;

/**
 * Timing information structure
 * Used to retrieve timing information from an endpoint
 */
typedef struct
{
    /** The period at which an endpoint delivers a block of data if it is time
     * driven. */
    unsigned int period;        /* Expressed in 64ths of a usec */
    /** The size of a block/frame that the endpoint consumes/produces */
    unsigned int block_size;
    /** Flag indicating whether the endpoint has a hard deadline that needs to
     * be met. */
    bool has_deadline:1;
    /** Flag indicating whether the endpoint is clocked locally or remotely.
     * This is only valid for real-endpoints. */
    bool locally_clocked:1;
    /** Flag indicating whether the endpoint wants to receive kicks from the thing
     * it is connected to or not.
     */
    bool wants_kicks:1;
} ENDPOINT_TIMING_INFORMATION;

/**
 * Scheduling information structure
 * Used to store details about the scheduling through a chain of operators
 */
typedef struct SCHEDULING_INFORMATION
{
    unsigned int kick_period;           /* Expressed in 64ths of a usec */
    unsigned int chain_proc_time;
    tCbuffer *far_end_buffer;
    /* TODO this shouldn't be needed but temporarily added to help sco scheduling */
    ENDPOINT *endpoint_deadline;
#ifdef DEBUG_KICK_TIMERS
    int32 initial_wallclock_value;
    int32 last_wallclock_value;
    TIME  initial_kick_time;
#endif
} SCHEDULING_INFORMATION;

/**
 * Enumeration of endpoint types. Used to decide how endpoints are connected.
 */
typedef enum
{
    /** Endpoint is an audio interface */
    endpoint_audio,
    /** Endpoint is a sco link */
    endpoint_sco,
    /** Endpoint is an operator */
    endpoint_operator,
    /** Endpoint is a shunt interface */
    endpoint_shunt,
    /** Endpoint is a A2DP interface */
    endpoint_a2dp,
    /** Endpoint is a File interface */
    endpoint_file,
    /** Endpoint is a raw buffer interface */
    endpoint_raw_buffer,
    /** Endpoint is a tester interface */
    endpoint_tester,
    /* Endpoint is a spdif interface */
    endpoint_spdif,
    /*Endpoint for file simulation */
    endpoint_sim_file,
#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    /* Shadow endpoint */
    endpoint_shadow,
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
    /* Endpoint is timestamped data  */
    endpoint_timestamped,
    /* Endpoint is a usb audio interface */
    endpoint_usb_audio

} ENDPOINT_TYPE;

/**
 * Structure of common endpoint function pointers.
 */
typedef struct ENDPOINT_FUNCTIONS
{
    /**
     * Close the endpoint
     */
    bool (*close)(ENDPOINT *);

    /**
     * Connect to the endpoint
     */
    bool (*connect)(ENDPOINT *, tCbuffer *, ENDPOINT *, bool *);

    /**
     * Disconnect from the endpoint
     */
    bool (*disconnect)(ENDPOINT *);

    /**
     * Retrieve the buffer details from the endpoint
     */
    bool (*buffer_details)(ENDPOINT *, BUFFER_DETAILS *);

    /**
     * Make the endpoint produce or consume some data
     */
    void (*kick)(ENDPOINT *, ENDPOINT_KICK_DIRECTION);

    /**
     * When the endpoint is responsible for scheduling chain kicks this function
     * is called to perform any real-time scheduling that needs to occur per kick
     */
    void (*sched_kick)(ENDPOINT *, KICK_OBJECT *);

    /**
     * When the endpoint is responsible for scheduling chain kicks this function
     * initiates a kick interrupt source to start producing kicks.
     */
    bool (*start)(ENDPOINT *, KICK_OBJECT *);

    /**
     * When the endpoint is responsible for scheduling chain kicks this function
     * cancels the associated kick interrupt source.
     */
    bool (*stop)(ENDPOINT *);

    /**
     * Configure the endpoint
     */
    bool (*configure)(ENDPOINT *, unsigned, uint32);

    /**
     * Get endpoint configuration
     */
    bool (*get_config)(ENDPOINT *, unsigned, ENDPOINT_GET_CONFIG_RESULT *);

    /**
     * Obtain the timing information from the endpoint
     */
    void (*get_timing_info)(ENDPOINT *, ENDPOINT_TIMING_INFORMATION *);

    /**
     * Synchronise two endpoints
     */
    bool (*sync)(ENDPOINT *, ENDPOINT *);

} ENDPOINT_FUNCTIONS;

/**
 * Information for real endpoint latency control. This only exists for real
 * endpoints, operator endpoints are assumed not needing this now nor in future.
 */
typedef struct ENDPOINT_LATENCY_CTRL_INFO
{
    /** Number of inserted sink silence samples that are not yet compensated for */
    unsigned silence_samples;

    /** Perceived input data block */
    unsigned data_block;

} ENDPOINT_LATENCY_CTRL_INFO;

/** The data specifically required for audio endpoints. The shape of this varies
 * dependent upon the platform one builds for. */

/**
 * Only allow MAX_CONSECUTIVE_UNDERRUNS to reach 2^MAX_CONSECUTIVE_UNDERRUNS_BITS - 1
 * This should prevent arithmetic overflow in underrun_sequence provided it is checked
 * after every increment.
 *
 */
STATIC_ASSERT( ( MAX_CONSECUTIVE_UNDERRUNS < (1 << MAX_CONSECUTIVE_UNDERRUNS_BITS)),
                not_enough_bits_for_underrun_sequence);

/**
 * Only allow NR_DATA_BLOCK_UPDATES to reach 2^NR_DATA_BLOCK_UPDATES_BITS - 1
 * This should prevent arithmetic overflow in data_block_updates provided it is checked
 * after every increment.
 *
 */
STATIC_ASSERT(  (NR_DATA_BLOCK_UPDATES < (1 << NR_DATA_BLOCK_UPDATES_BITS)),
                not_enough_bits_for_data_block_updates);

/**
 * Information needed to manage a deferred kick handler
 */
typedef struct endpoint_deferred_kick
{
    /**
     * Enable deferred kick handling. If false, the
     * endpoint kick function is called directly in the
     * interrupt context.
     * \note Packing :8 causes byte instructions
     * to be emitted i.e. is both space and time efficient.
     */
    bool kick_is_deferred : 8;

    /**
     * Configure deferred kick handling
     */
    bool config_deferred_kick : 8;

    /**
     * Direction of deferred kick
     */
    ENDPOINT_KICK_DIRECTION kick_dir : 8;

    /**
     * Task ID of a task containing just a bg_int handler
     */
    taskid bg_task;

    /**
     * Time taken by the first interrupt handler
     */
    TIME interrupt_handled_time;
} endpoint_deferred_kick;

#ifdef INSTALL_UNINTERRUPTABLE_ANC
/** Structure used to hold ANC configuration data */
typedef struct endpoint_anc_state
{
    /* Instance of the ANC H/W associated with the Endpoint */
    ACCMD_ANC_INSTANCE instance_id;

    /* Input path associated with the endpoint */
    ACCMD_ANC_PATH input_path_id;

#ifdef INSTALL_ANC_STICKY_ENDPOINTS
    /* Flag to indicate that an attempt to close the endpoint has been made while
     * it is in-use by ANC
     */
    bool close_pending;
#endif /* INSTALL_ANC_STICKY_ENDPOINTS */

}endpoint_anc_state;
#endif /* INSTALL_UNINTERRUPTABLE_ANC */

typedef struct endpoint_audio_state
{
    /**
     * The running state of the interrupt source.
     */
    bool running;

    /**
     * Next audio end point that is synchronised with this one. Set to NULL
     * if this end point is not synchronised with any other end point
     */
    struct ENDPOINT *nep_in_sync;

    /**
     * Master end point that all other endpoint in the synchronised chain follow
     * If this end point is not synchronised with any other end point, this end
     * point is the master.
     */
    struct ENDPOINT *head_of_sync;

    /** This is a signed value representing the difference between the rate data
     * is being produced and consumed. This value is normalised.
     */
    unsigned int rm_adjust_amount;

    /** Amount of compensation to perform in the sink endpoint cbops copy to
     * maintain the output buffer level on account of any rate mismatch. */
    int rm_diff;

    /** The cbuffer that audio data arrives from. If the endpoint is a source
     * this represents the port. */
    tCbuffer *source_buf;

    /** The cbuffer that audio data leaves through. If the endpoint is a sink
     * this represents the port. */
    tCbuffer *sink_buf;

    /**
     * The running state of the interrupt source.
     */
    bool is_overridden:1;

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
    /**
     * Is the hardware allocated?
     */
    bool hw_allocated:1;

    /**
     * Is the monitor running?
     */
    bool monitor_enabled:1;

    /**
     * Bit shift to configure on buffer
     */
    unsigned int shift:5;

    /**
     * How often should the audio endpoint notify us of new data?
     * (measured in samples)
     */
    int monitor_threshold;

    /** The last time the hardware produced an interrupt. Used for calculating the
     * actual data rate when the hardware isn't locally clocked.
     */
    RATE_TIME rm_period_start_time;

    /**
     * The time between kicks integrated over recent history to allow for
     * expected run time jitter.
     */
    RATE_TIME rm_int_time;

    /**
     * Expected time between kicks, scaled same as rm_int_time.
     */
    RATE_TIME rm_expected_time;

    /**
     * Rate matching support
     */
    unsigned rm_support :2;

    /** Allow endpoint to advertise SW rate adjustment ability */
    bool rm_enable_sw_rate_adjust   : 1;

    /** Allow endpoint to advertise HW rate adjustment ability */
    bool rm_enable_hw_rate_adjust   : 1;

    /** Allow endpoint to provide CLRM rate measurements */
    bool rm_enable_clrm_measure     : 1;

#ifdef INSTALL_AUDIO_EP_CLRM
    /** Allow CLRM rate matching */
    bool rm_enable_clrm_match       : 1;

    /** Allow CLRM adaptive responsiveness */
    bool rm_enable_clrm_adaptive    : 1;
#endif /* INSTALL_AUDIO_EP_CLRM */

    /**
     * Current rate adjustment value
     */
    int rm_adjust_prev;

    /**
     * Current sp_deviation of HW rate warp capable device (fractional)
     */
    int rm_hw_sp_deviation;

    /**
     * Current reported sp_deviation (fractional)
     */
    int rm_report_sp_deviation;

    /**
     *  The sample rate of the audio hardware, used among others
     *  to compute ToA efficiently.
     */
    unsigned sample_rate;

    /** Sample count/time accumulator */
    RATE_MEASURE rm_measure;

#ifdef INSTALL_AUDIO_EP_CLRM
    /** Rate match state */
    RATE_MATCH_CONTROL rm_control;

    /**
     * Access to the rate_adjust_and_shift operator for rate adjustment
     */
    cbops_op* rate_adjust_op;

    /**
     * A parameter needed when calling cbops directly
     */
    unsigned rate_adjust_op_num_channels;
#endif /* INSTALL_AUDIO_EP_CLRM */

#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
    /**
     * HAL warp update descriptor
     */
    WARP_UPDATE_DESC rm_update_desc;
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */
#endif /* CHIP_BASE_HYDRA */

#ifdef INSTALL_MCLK_SUPPORT
    /* showing whether MCLK is ready to use by this endpoint */
    bool mclk_claimed;
#endif

#ifdef INSTALL_UNINTERRUPTABLE_ANC
    struct endpoint_anc_state anc;
#endif /* INSTALL_UNINTERRUPTABLE_ANC */

#ifdef CHIP_BASE_BC7

    /** Port ID in BC case */
    unsigned portid:5;

    /** Whether this chip is the clock source for the audio hardware. */
    bool locally_clocked:1;

    /**
     * The timer id of the timer generating the next kick. It may be
     * possible to union this with the monitor_enabled field.
     */
    tTimerId kick_id;

    /**
     * The period of the kick timer.
     */
    unsigned int kick_period;

    /**
     * Accumulated sample count for accurate kick time determination
     */
    unsigned int acc_samples;

    /**
     * Accumulated time for accurate kick time determination
     */
    unsigned int acc_kick_time;

    /**
     * The block size in samples, corresponding to the kick period.
     */
    unsigned int block_size;

    /** The sample rate of the audio hardware */
    unsigned sample_rate;

    /** The running count of samples since the rate was last reported. */
    unsigned rm_sample_cnt;

    /** The port buffer offset on the previous kick, used to calculate rm_sample_cnt */
    unsigned rm_buff_offset;

    /** The number of kicks since the rate was last reported. */
    unsigned rm_kick_cnt;

    /** Current estimation of 1/(audio hardware rate). Stored inverted to speed up
     * kick time calculations */
    int rm_curr_rate;

    /** The fractional part of a microsecond carried over from the last kick,
     * this can be accumulated over several kicks until it amounts to a whole
     * microsecond. */
    unsigned rm_kick_frac;

    /** Flag whether the rate matching is in a startup condition (TRUE)
     * or if it should have converged (FALSE) */
    bool rm_starting;

#endif /* CHIP_BASE_BC7 */

#ifdef CHIP_BASE_A7DA_KAS

    unsigned int block_size;

    a7da_audio_port *port;


    /**
      * Pointer to ratematch enacting function.
      * Used for software and hardware ratematching */
      void (*rm_enacting_fn)(ENDPOINT*, int32);

#endif /* CHIP_BASE_A7DA_KAS */
    /** Delta samples - internal, calculated by RM mechanism. Some mechanisms
     *  may not make use of this... so common future proofing.
     */
    unsigned delta_samples;

    /* Channel number when synchronised in a group */
    unsigned channel;

    /** Data started flag (across all channels) */
    bool sync_started;

#ifdef TIMED_PLAYBACK_MODE
    /**
     * Bitfield used to detect the start of the data flow.
     */
    bool data_flow_started:1;

    /**
     * True, if the timed playback takes care of playing the audio.
     */
    bool use_timed_playback:1;

    /**
     * Delay added by the endpoint (microseconds).
     */
    unsigned endpoint_delay_us;

    /**
     * Timed playback module.
     */
    TIMED_PLAYBACK* timed_playback;
#endif

#ifdef INSTALL_AUDIO_SOURCE_TOA_METADATA_GENERATE
    /**
     * Flag to enable metadata generation by source endpoint
     * This is configured by user.
     */
    bool generate_metadata;

    /**
     * minimum tag length in words, this is to make
     * sure endpoint doesn't create a lot of small-length
     * tags
     */
    unsigned min_tag_len;

    /** number of words left to complete last written tags */
    unsigned last_tag_left_words;
#endif /* INSTALL_AUDIO_SOURCE_TOA_METADATA_GENERATE */

    /**
     * Information for latency control purposes.
     * Should be the last member of endpoint_audio_state
     * so as to allow variable size endpoint_audio_state.
     * ie: latency_ctrl_info is allocated only for audio sinks
     */
    struct ENDPOINT_LATENCY_CTRL_INFO latency_ctrl_info;
    /*
     * Don't add anything here - see above !
     */
}endpoint_audio_state;

#ifdef INSTALL_SPDIF

/* number of channel status words, each word is
 * 16 bit regardless of architecture
 */
#define SPDIF_NOROF_CHSTS_WORDS 12

/* S/PDIF channel order */
typedef enum
{

/* The channel order numbering are the same for all
 * chip types, however for hydra we get that directly
 * from ACCMD-based definitions in hal.
 */
#ifdef CHIP_BASE_HYDRA
    /* A channel (LEFT) */
    SPCO_CHANNEL_A = SPDIF_CHANNEL_A,

    /* B channel (RIGHT) */
    SPCO_CHANNEL_B = SPDIF_CHANNEL_B,

    /* interleaved channel (LEFT+RIGHT) */
    SPCO_CHANNEL_AB = SPDIF_CHANNEL_A_B_INTERLEAVED

#else
    /* A channel (LEFT) */
    SPCO_CHANNEL_A = 0,

    /* B channel (RIGHT) */
    SPCO_CHANNEL_B = 1,

    /* interleaved channel (LEFT+RIGHT) */
    SPCO_CHANNEL_AB = 3

#endif
} SPDIF_CHANNEL_ORDER;

/** The data specifically required for spdif endpoints. The shape of this varies
 * dependent upon the platform one builds for. */
typedef struct endpoint_spdif_state
{
    /**
     * The running state of the interrupt source.
     */
    bool running;

    /** The cbuffer that audio data arrives from. If the endpoint is a source
     * this represents the port. */
    tCbuffer *source_buf;

    /** The cbuffer that audio data leaves through. If the endpoint is a sink
     * this represents the port. */
    tCbuffer *sink_buf;

    /**
     * The timer id of the timer generating the next kick. It may be
     * possible to union this with the monitor_enabled field.
     */
    tTimerId kick_id;

#ifdef CHIP_BASE_BC7
    /** Port ID in BC case */
    unsigned portid:5;

    /** input sample width */
    unsigned input_width:5;
#endif

    /** Whether this chip is the clock source for the audio hardware. */
    bool locally_clocked:1;

#ifdef CHIP_BASE_HYDRA
    /**
     * Is the hardware allocated?
     */
    bool hw_allocated:1;
    /**
     * Is the monitor running?
     */
    bool monitor_enabled:1;

    /**
     * Bit shift to configure on buffer
     */
    unsigned int shift:5;

    /* flag showing the endpoint is overridden */
    bool is_overridden:1;

    /**
     * How often should the audio endpoint notify us of new data?
     * (measured in samples)
     */
    int monitor_threshold;
#endif

    /** channel order */
    SPDIF_CHANNEL_ORDER channel_order;

    /* HW instance */
    unsigned instance;

    /* the output format of spdif endpoint is always
     * SPDIF_INPUT_DATA_FORMAT (which means it can be either
     * audio, or data and in both cases it can be interleaved
     * as well), so it can only connect to a spdif_decode
     * operator. However to be able to test the endpoint
     * separately we allow to explicitly change the endpoint
     * output format, it shouldn't be used in a real application.
     */
    unsigned output_format;

    /**
     * The period of the kick timer.
     */
    unsigned int kick_period;

    /* current sample rate of the input */
    unsigned int sample_rate;

    /* an A type must be paired with a B type
     */
    struct ENDPOINT *twin_endpoint;

    /* Note: this structure is also used
     * in assembly function
     * */
    struct spdif_extra_states
    {

      /* The sample rate that is detected
       * by FW (external) */
       unsigned fw_sample_rate;

      /* a flag showing fw changed its
       * decision */
       unsigned fw_sample_rate_changed;

#ifdef CHIP_BASE_BC7

       /* for handling higher rates */
       TIME highrate_timer_time;
       unsigned can_restart_interface;
       unsigned confirms_pending;
       bool inactive;
#endif

       /* when stream stalled, the endpoint will generate silence
        * for limited or unlimited amount of time
        */
       int silence_duration;

       /* number of auxiliary bits*/
       unsigned num_aux_bits;

       /* a simple counter that incremented
        * every time channel status updated
        * (no immediate use)
        */
       unsigned channel_status_counter;

       /* save 384 bits of channel status,
        * (atomic update), first half bit will be
        * for channel A, second half for channel B
        */
       unsigned channel_status[2*SPDIF_NOROF_CHSTS_WORDS];

       /* last time the input was read
        */
       TIME last_read_time;

       /* final decision about whether
        * the stream shall be regarded as valid
        */
       unsigned stream_valid;

       /* all supported sample rates */
        unsigned nrof_supported_rates;
        const unsigned *supported_rates;

       /* bitmask for supported sample rates */
        unsigned supported_rates_mask;

       /* This is what DSP thinks about sample rate
        */
       unsigned dsp_sample_rate;

       /* last time the input was read
        */
       int read_interval;

       /* some state variables used
        * for pause handling
        */
       unsigned time_in_pause;
       unsigned pause_state;
       unsigned measured_sample_rate;
       unsigned silence_res;

       /* for rate  mismatch report */
       int norm_rate_ratio;

       /* internally stores double precision mismatch rate */
       int norm_rate_ratio_dp[2];

       /* rate detect in DSP */
       unsigned rate_detect_hist_index;

       /* some history buffer used for rate checking
        * in assembly function. The rate is measured based
        * on a history of amount of data received in last
        * 40 kicks, and final rate is decided based on last
        * 15 measurements.
        */
       unsigned rate_detect_hist[40+15];
    }*extra;
}endpoint_spdif_state;
#endif /* #ifdef INSTALL_SPDIF */

#ifdef INSTALL_USB_AUDIO
/* forward declaration for usb rate measure structure */
typedef struct usb_audio_rate_measure usb_audio_rate_measure;

/** The data specifically required for usb audio endpoints. The shape of this varies
 * dependent upon the platform one builds for. */
typedef struct endpoint_usb_audio_state
{

    /** The running state of the interrupt source. */
    bool running:1;

    /**
     * The timer id of the timer generating the next kick (Tx-only)
     */
    tTimerId kick_id;

    /**
     * The period of the kick timer. (Tx-only)
     */
    unsigned int kick_period;


#ifdef CHIP_BASE_BC7

    /** Port ID */
    unsigned portid;
#endif /* CHIP_BASE_BC7 */

#ifdef CHIP_BASE_HYDRA
    /** private audio data service handle */
    void *service_priv;
#endif

    /** The cbuffer that usb audio data arrives from. If the
     * endpoint is a source this represents the port in bluecore
     * and an mmu buffer in hydra */
    tCbuffer *source_buf;

    /** The cbuffer that usb audio data leaves through. If the
     * endpoint is a sink this represents the port in bluecore
     * and an mmu buffer in hydra */
    tCbuffer *sink_buf;

    /* nominal sample rate of the stream */
    unsigned sample_rate;

    /* number of channels */
    unsigned n_channels;

    /* subframe size in bits */
    unsigned subframe_size;

    /* data format of the endpoint, expected to be
     * USB_AUDIO_DATA_FORMAT to be connectable
     * to usb_audio operator. However it also
     * can be AUDIO_DATA_FORMAT_FIXP for testing
     * purposes
     */
    AUDIO_DATA_FORMAT data_format;

    /* number of frames in a TX packet - integer part
     * Note: the packet rate is 1000Hz, for most common sample
     * rates this will be translated into an integer number of
     * samples, but for 44.khz family of sample rates the
     * packet length is adjusted to keep the average sample
     * rate accurate.
     */
    unsigned frames_in_packet_int;

    /* number of frames in a TX packet - fractional part
    */
    unsigned frames_in_packet_rem;

    /* accumulator for remainder */
    unsigned packet_rem_acc;

    /* the length of (next) packet in frames */
    unsigned packet_len_in_frames;

    /* subframe_size/8 */
    unsigned subframe_in_octets;

    /* subframe_in_octets*n_channels */
    unsigned frame_in_octets;

    /* rate mismatch that will be reported to rate match manager
     * This isn't for TTP purpose.
     */
    unsigned norm_rate_ratio;

    /* whether this ep is performing rate-matching - TX only */
    bool ep_ratematch_enacting;

    /* Whether the rate adjustment is in TTP mode - TX only */
    bool timed_playback;

    /* the sra operator that performs rate adjustment - Tx only */
    cbops_op *rate_adjust_op;

    /* target rate adjust value, used when the endpoint is
     * enacting in non-TTP mode - TX only
     */
    unsigned target_rate_adjust_val;

    /* time stamp for last read packet from input Tx-only */
    TIME last_read_timestamp;

    /* whether last_read_timestamp is valid - TX only */
    bool last_read_timestamp_valid;

    /* current error threshold - Tx only */
    int error_threshold;

    /* few fields for adjusting timer period - Tx only */
    /* for detecting stall in packet consumption */
    unsigned timer_period_adjust_stall_counter;

    /* adjustment starts after seeing movement for some time */
    unsigned timer_period_adjust_normal_counter;

    /* whether we are adjusting the timer period */
    bool timer_period_adjust_normal_mode;

    /* difference between actual and expected packets sent */
    int timer_period_adjust_packet_offset;

    /* few fields for ttp error control */
    TIME_INTERVAL ttp_control_prev_error;

    /* accumulator for averaging error */
    int ttp_control_error_acc;
    unsigned ttp_control_error_acc_counter;

    /* maximum number of packets in sink buffer - Tx only */
    unsigned max_packets_in_tx_output_buffer;

    /** Internal settings of the PID controller - Tx only */
    pid_controller_settings pid_params;

    /** The PID controller internal state - Tx only */
    pid_controller_state pid_state;

    /* rate measure structure */
    usb_audio_rate_measure *rate_measure;

} endpoint_usb_audio_state;

#endif /* #ifdef INSTALL_USB_AUDIO */

#ifdef INSTALL_SCO
/** The data specifically required for sco endpoints. The shape of this varies
 * dependent upon the platform one builds for. */
typedef struct endpoint_sco_state
{
    /**
     * The timer id of the timer generating the next kick. It may be
     * possible to union this with the monitor_enabled field.
     */
    tTimerId kick_id;

    /**
     * The period of the kick timer.
     */
    unsigned int kick_period;

    /**
     * The time at which data is expected to be available. Either for
     * transmission over the air or to have been received over the air.
     */
    TIME data_avail_time;

    /**
     * The processing time of the upstream operator.
     */
    unsigned int proc_time;

    /**
     * The wallclock value when the rate was last reported.
     */
    int32 rm_wclock;

#ifdef DEBUG_KICK_TIMERS
    int32 initial_wallclock_value;
    int32 last_wallclock_value;
    TIME  initial_kick_time;
#endif

#ifdef CHIP_BASE_HYDRA
    /**
     * Pointer to the cbuffer structure that encapsulates the buffer
     * associated with the sco endpoint
     */
    tCbuffer *cbuffer;
#endif /* CHIP_BASE_HYDRA */

#ifdef CHIP_BASE_NAPIER
    /*
     * Hydra:   No processing on buffer samples in EP: --> EP's input and output buffer is the same
     *          --> only 1 buffer needed per EP (in and output buffer of the EP is the same buffer --> cbuffer
     * BC:      Cbops processing applied to input buffer samples --> processed samples will be written to output buffer (which is different from input buffer)
     *          --> 2 buffers needed per EP --> source_buf, sink_buf
     * Napier: Presumed that the error concealment of the RX buffer samples will be prrocessed in the SCO endpoint, the EP' input buffer(s) will be different from the EP's output buffer
     *         SCO_SOURCE:  --> (up to) 3 input buffers (Rx buffers)
     *                      --> 1 output buffer
     *                      --> 3 source buffers / 1 sink buffer
     *         SCO_SINK:    --> 2 output buffers (ping-pong bufferring)
     *                      --> 1 input buffer
     *                      --> 1 source buffer / 2 sink buffers
     *         If SCO_SOURCE and SCO_SINK EP's are handled by the same struct --> 3 source / 2 sink buffers required
     */
#define MAX_NUM_RX_BUFFERS  3
#define MAX_NUM_TX_BUFFERS  2
    int32     source_buf_num;                   /* number of used source buffers */
    int32     sink_buf_num;                     /* number of used sink buffers */
    tCbuffer* source_buf[MAX_NUM_RX_BUFFERS];   /* source buffers are these buffers that sco data are read from */
    tCbuffer* sink_buf[MAX_NUM_TX_BUFFERS];     /* sink buffers are these buffers that sco data leaves through */
    AUDIO_DATA_FORMAT    audio_data_format;     /* indicates the data format of the connected operator */
    bool                 majority_vote_bypass;  /* flag to indicate whether majority voting is bypassed(1) or not(0) */
    uint8                majority_vote_questionable_bits_max; /* majority voting: max. number of uncorrelated bits to status overwrite in case of CRC_FAIL */
    TIME                 time_stamp_init;       /* initial time stamp as reported to SCO_RCV and SCO_WBS upon sco_start */

    unsigned             metadata_TxCnt;               /* metadata counter along with TX SCO data */
    uint16               metadata_backup_len;          /* metadata length as read from metadata of previous SCO packet */
    uint16               metadata_backup_payload_size; /* payload size as read from metadata of previous SCO packet */
    int                  TxBytes2Write;                /* Most recent 2 Bytes that have been recevied by SCO SINK */
    int                  TxNumBytes2Write;             /* Amount of Bytes that have not yet been written to TX buffer by SCO SINK */
    unsigned             TxNumBytesWritten;            /* Amount of Bytes that have been written to TX buffer for current SCO packet by SCO SINK */
#endif /* CHIP_BASE_NAPIER */

#ifdef CHIP_BASE_BC7
    /** Port ID */
    unsigned portid;

    /** The cbuffer that sco data arrives from. If the endpoint is a source
     * this represents the port. */
    tCbuffer *source_buf;

    /** The cbuffer that sco data leaves through. If the endpoint is a sink
     * this represents the port. */
    tCbuffer *sink_buf;
#endif /* CHIP_BASE_BC7 */
    /* For SCO rate measurement in sco_sched_kick */
    int  rate_measurement;
    TIME rm_start_time;

#ifdef INSTALL_SCO_EP_CLRM
    /** Enable CLRM measurements.
     * \note 8 bit packing is implemented as byte access */
    bool rm_enable_clrm_measurement     : 8;

    /** Enable CLRM diagnostic trace output.
     * \note 8 bit packing is implemented as byte access */
    bool rm_enable_clrm_trace           : 8;

    /** Rounding-free rate measurement */
    RATE_MEASURE rm_measure;

    /** Bundle of rate measurement results */
    ENDPOINT_RATEMATCH_MEASUREMENT_RESULT rm_result;
#endif /* INSTALL_SCO_EP_CLRM */

#ifdef INSTALL_SCO_RX_TOA_METADATA_GENERATE
    /* this is for SCO Rx operator only, by default Rx endpoint will
     * generate metadata
     */
    volatile bool generate_metadata;

    /* previous write offset */
    unsigned prev_write_offset;

    /* difference between actual and expected received packets*/
    int packet_offset;
    unsigned packet_offset_counter;
    /* shows whether we are confident about packet offset */
    bool packet_offset_stable;
#endif /* INSTALL_SCO_RX_TOA_METADATA_GENERATE  */

} endpoint_sco_state;
#endif /* INSTALL_SCO */

#ifdef INSTALL_FILE
/** The data specifically required for file endpoints. The shape of this varies
 * dependent upon the platform one builds for. */
typedef struct endpoint_file_state
{
#ifdef CHIP_BASE_BC7
    /** Port ID */
    unsigned portid;

    /** The cbuffer that file data is read from. If the endpoint is a source
     * this represents the port. */
    tCbuffer *source_buf;

    /** The cbuffer that file data is written through. If the endpoint is a sink
     * this represents the port. */
    tCbuffer *sink_buf;
    /**
     * The timer id of the timer generating the next kick.
     */
    tTimerId kick_id;

    /**
     * The period of the kick timer.
     */
    unsigned int kick_period;
#endif /* CHIP_BASE_BC7 */

#ifdef CHIP_BASE_A7DA_KAS
    /**
     * The running state of the interrupt source.
     */
    bool running;

    /**
     * Next audio end point that is synchronised with this one. Set to NULL
     * if this end point is not synchronised with any other end point
     */
    struct ENDPOINT *nep_in_sync;

    /**
     * Master end point that all other endpoint in the synchronised chain follow
     * If this end point is not synchronised with any other end point, this end
     * point is the master.
     */
    struct ENDPOINT *head_of_sync;

    /** This is a signed value representing the difference between the rate data
     * is being produced and consumed. This value is normalised.
     */
    unsigned int rm_adjust_amount;

    /** Amount of compensation to perform in the sink endpoint cbops copy to
     * maintain the output buffer level on account of any rate mismatch. */
    int rm_diff;

    /** The cbuffer that audio data arrives from. If the endpoint is a source
     * this represents the port. */
    tCbuffer *source_buf;

    /** The cbuffer that audio data leaves through. If the endpoint is a sink
     * this represents the port. */
    tCbuffer *sink_buf;

    /**
     * Counter for series of complete under-runs (no input data at all)
     * Only valid for audio sinks
     */
    unsigned underrun_sequence : MAX_CONSECUTIVE_UNDERRUNS_BITS;

    unsigned int block_size;

    a7da_audio_port *port;

    /** Signals when in a valid ratematch period, basically at stream startup.
     */
    bool rm_period_started;

    /**
     * The previous write pointer offset in the endpoint buffer - used for hardware ratematching */
     unsigned rm_prev_wp;

    /**
     * The previous read pointer ofset in the endpoint buffer - used for hardware ratematching */
     unsigned rm_prev_rp;

     /**
      * The number of samples written to the endpoint buffer integrated over recent history.
      * Used for hardware ratematching */
      uint48 rm_int_wsamples;

     /**
      * The number of samples read from the endpoint buffer integrated over recent history.
      * Used for hardware ratematching */
      uint48 rm_int_rsamples;

#endif /* CHIP_BASE_A7DA_KAS */

#ifdef CHIP_BASE_HYDRA
    /**
     * TRUE if the Hydra File endpoint has started, FALSE if stopped or not started.
     */
    bool running;
    /** cbuffer data is read from */
    tCbuffer *source_buf;
    /** cbuffer data is written to */
    tCbuffer *sink_buf;
    /** handle private to the data service */
    void * service_priv;
    /** Internal kick timer ID used to run a backwards kick at the same priority level
     *  as a bus interrupt
     */
    tTimerId internal_kick_timer_id;
    /** BAC handle shift of the underlying source mmu data buffer */
    unsigned shift:5;
    /** BAC handle byte swap configuration of the underlying source mmu data buffer */
    bool byte_swap:1;
    /** Number of usable octets per word in the transform buffer */
    unsigned usable_octets:3;
#endif /* CHIP_BASE_HYDRA */
    /* Channel number when synchronised in a group */
    unsigned channel;
} endpoint_file_state;
#endif /* INSTALL_FILE */

#ifdef INSTALL_SIMULATION_FILE
/** The data specifically required for simulation file endpoints. The shape of this varies
 * dependent upon the platform one builds for. */
typedef struct endpoint_sim_file_state
{
#ifdef CHIP_BASE_BC7
    /** Port ID */
    unsigned portid;

    /** The cbuffer that file data is read from. If the endpoint is a source
     * this represents the port. */
    tCbuffer *source_buf;

    /** The cbuffer that file data is written through. If the endpoint is a sink
     * this represents the port. */
    tCbuffer *sink_buf;
    /**
     * The timer id of the timer generating the next kick.
     */
    tTimerId kick_id;

    /**
     * The period of the kick timer.
     */
    unsigned int kick_period;
#endif /* CHIP_BASE_BC7 */


    /* Channel number when synchronised in a group */
    unsigned channel;
} endpoint_sim_file_state;
#endif /* INSTALL_FILE */

#ifdef INSTALL_TIMESTAMPED_ENDPOINT
/** The data specifically required for timestamped endpoints. The shape of this varies
 * dependent upon the platform one builds for */
typedef struct endpoint_timestamped_state
{
    tCbuffer *source_buf;
    tCbuffer *sink_buf;
    /** handle private to the data service */
    void * service_priv;
    /** Internal kick timer ID used to run a kick from a connected operator
     *  at the same priority level as a bus interrupt
     */
    tTimerId internal_kick_timer_id;
    /** Count of octets uncopied from previously-tagged data */
    unsigned tag_octets_remaining;
    /** TRUE if the endpoint has started, FALSE if stopped or not started */
    bool running;
    /** TRUE if the initial data alignment has been completed */
    bool aligned;
    /** BAC handle shift of the underlying source mmu data buffer */
    unsigned shift:5;
} endpoint_timestamped_state;
#endif /* INSTALL_TIMESTAMPED_ENDPOINT */

#ifdef INSTALL_RAW_BUFFER
typedef struct endpoint_raw_buffer_state
{
    tCbuffer *source_buf;
} endpoint_raw_buffer_state;
#endif /* INSTALL_RAW_BUFFER */

#ifdef INSTALL_AUDIO_DATA_SERVICE_TESTER
typedef struct endpoint_tester_state
{
    tCbuffer *source_data_buf;
    tCbuffer *sink_data_buf;
    void * service_priv;
    uint16 priv_hdr_len;
    /** Internal kick timer ID used to run an operator kick at the same priority level
     *  as a bus interrupt
     */
    tTimerId internal_kick_timer_id;
} endpoint_tester_state;
#endif

#if defined(INSTALL_A2DP) ||\
    defined(INSTALL_SHUNT)

/** The data specifically required for a2dp endpoints. The shape of this varies
 * dependent upon the platform one builds for. */
typedef struct endpoint_a2dp_state
{
    /* The average A2DP buffer level integrated over a large number of samples */
    uint32 average_level;

    /* The target A2DP buffer level (latency) */
    unsigned target_level;

    /** This is a signed fractional value representing the difference between
     * the rate data is being produced and consumed. This value is normalised.
     */
    unsigned int rm_adjust_amount;
#ifndef TIMED_PLAYBACK_MODE
    /** Indicates whether the endpoint wants to process data. Controls operator
     * so fills a whole word otherwise the operator would have to know the bit
     * field pattern in this structure.*/
    bool playing;
#endif
    /** The cbuffer that a2dp data arrives from. If the
     * endpoint is a source this represents the port in bluecore. */
    tCbuffer *source_buf;

    /** The cbuffer that a2dp data leaves through. If the
     * endpoint is a sink this represents the port in bluecore.*/
    tCbuffer *sink_buf;

    /** The running state of the interrupt source. */
    bool running:1;

    /** Indicates if the endpoint has stalled */
    bool stalled:1;

    /** Flag indicating whether the last port copy read an odd octet */
    bool read_an_octet:1;

#ifdef CHIP_BASE_BC7

    /** The timer id of the timer generating the next kick. */
    tTimerId kick_id;
    /** Port ID */
    unsigned portid;

    /** Number of kicks where no data has arrived over the air, used to detect
     * a stall. */
    unsigned no_kick_counter;
#endif /* CHIP_BASE_BC7 */

#ifdef CHIP_BASE_HYDRA
    /** private audio data service handle */
    void *service_priv;
#endif /* CHIP_BASE_HYDRA */
#if defined (CHIP_BASE_HYDRA) || defined (CHIP_BASE_NAPIER)
    /** The timer id of the timer that checks if the a2dp stream has stalled */
    tTimerId stall_timer_id;
    /** The timer id of the timer used to retry pushing data forward */
    tTimerId self_kick_timer_id;
#endif

#ifdef INSTALL_SHUNT
    /** The L2CAP channel ID used by the endpoint */
     unsigned int cid;
#endif /* INSTALL_SHUNT */

} endpoint_a2dp_state, endpoint_shunt_state;

#endif /* INSTALL_A2DP || INSTALL_SHUNT */


#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
/** The data specifically required for shadow endpoints. */
typedef struct endpoint_shadow_state
{
    /** Data channel ID for KIP EP
     * This is preserved here to search the KIP transform connected
     * to this endpoint using the data channel id
     */
    uint16 channel_id;

#ifdef INSTALL_METADATA_DUALCORE
    /** Flag to indicate if this shadow EP represents an endpoint that
     * supports metadata */
    bool supports_metadata:1;

    /** IPC data channel ID for transporting metadata tags */
    uint16 meta_channel_id;

    /** buffer associated with the metadata channel */
    tCbuffer *metadata_shared_buf;
#endif /* INSTALL_METADATA_DUALCORE */

    /** If the endpoint is a source, this is what data channel connects
     *  across IPC and the Cbuffer is owned by the remote processor
     *  If the endpoint is a sink, this is what data channel extends to
     * the remote processor through IPC. This is same as the local cbuffer
     * connected to kip.
     */
    tCbuffer *buffer;

    /* The buffer size requirement for the endpoint. This is valid
     * only when buffer is NULL.
     */
    uint16 buffer_size;

    /* data format */
    AUDIO_DATA_FORMAT data_format;

    /* local buffer. If this is set, shadow endpoint cloned the ipc buffer
     * and must be freed on disconnect.
     */
    bool cloned_buffer;

    /** Flag indicating if a kick is currently in progress. This is a whole word
     * to avoid read-modify-write races. This is used to avoid the kick handler
     * thread safety issues. */
    bool kick_in_progress;

    /** remote kick is pending */
    bool remote_kick;

    /** The direction that a kick was received from when another kick was already
     * in progress. */
    ENDPOINT_KICK_DIRECTION kick_blocked;

} endpoint_shadow_state;
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */

/**
 * Structure that describes an endpoint
 */
struct ENDPOINT
{
    /**
     * Table of common endpoint function pointers
     */
    const struct ENDPOINT_FUNCTIONS *functions;

    /**
     * The key of the endpoint. Each endpoint within a specific endpoint type
     * and direction will have a unique key. Keys should not be treated as
     * unique across different endpoint types or endpoint directions.
     */
    unsigned int key;

    /**
     * The id of the endpoint. This is unique across all endpoint of the same
     * direction, i.e. it is possible for a source and a sink endpoint to have
     * the same id.
     */
    unsigned int id:16;

    /**
     * Flag to say if the endpoint can be closed
     */
    unsigned int can_be_closed:1;

    /**
     * Flag to say if the endpoint can be destroyed
     */
    unsigned int can_be_destroyed:1;

    /**
     * Flag to say if an endpoint is created at connection time
     * and hence needs to be destroyed when disconnected.
     * Currently this is only true for operator endpoints.
     */
    unsigned int destroy_on_disconnect:1;

    /**
     * Flag to say if an endpoint is a source or a sink
     */
    ENDPOINT_DIRECTION direction:1;

    /**
     * Enum to say how the endpoint should be connected
     */
    ENDPOINT_TYPE stream_endpoint_type:4;

    /* End of word */

    /**
     * Flag to say if an endpoint is a 'real' endpoint
     * (where real means "at the end of a chain)
     */
    bool is_real:1;

    /**
     * Flag to say if an endpoint has been started
     * (is processing data).
     */
    bool is_enabled:1;

    /**
     * Connection id of the owner of this endpoint
     */
    unsigned int con_id:16;

    /**
     * Flag to say if an endpoint uses the rate match interface
     * even though it is not a 'real' endpoint
     */
    bool is_rate_match_aware:1;

    /**
     * Pointer to cbops_manager that encapsulates the cbops
     * information associated with the endpoint
     */
    struct cbops_mgr *cbops;

    /**
     * Pointer to the endpoint that this endpoint has a connection to.
     */
    struct ENDPOINT *connected_to;

    /**
     * Endpoint to kick. Can be null if the endpoint doesn't need to kick anything.
     */
    struct ENDPOINT *ep_to_kick;

    /**
     * Pointer to next endpoint in the list.
     */
    struct ENDPOINT *next;

    /**
     * Fields for running the kick function as a bg_int
     */
    struct endpoint_deferred_kick deferred;

    /**
     * Endpoint specific state information
     *
     * As this is variable length this MUST be at the end of the
     * structure
     */
    union ENDPOINT_STATE
    {
        struct endpoint_audio_state audio;
#ifdef INSTALL_SCO
        struct endpoint_sco_state sco;
#endif /* INSTALL_SCO */
        struct endpoint_operator_state
        {
            /**
             * This is currently used as an on/off switch for DC remove.
             * It can be easily repurposed to indicate to an operator endpoint the 'per-chain'
             * data processing required like cbops (with details on which cbops operators are required),
             * rate matching decision.
             *
             */
            unsigned int cbops_flags;

            /** The bgint task of the underlying operator, this is provided to accelerate kicking from
             * endpoints. */
            BGINT_TASK op_bg_task;
        }operator;
#ifdef INSTALL_FILE
        struct endpoint_file_state file;
#endif
#ifdef INSTALL_SIMULATION_FILE
       struct endpoint_sim_file_state sim_file;
#endif
#ifdef INSTALL_RAW_BUFFER
        struct endpoint_raw_buffer_state raw_buffer;
#endif

#ifdef INSTALL_SHUNT
        struct endpoint_a2dp_state shunt;
#endif /* INSTALL_SHUNT */

#ifdef INSTALL_A2DP
        struct endpoint_a2dp_state a2dp;
#endif /* INSTALL_A2DP */
#ifdef INSTALL_SPDIF
       struct endpoint_spdif_state spdif;
#endif /* #ifdef INSTALL_SPDIF */

#ifdef INSTALL_USB_AUDIO
        struct endpoint_usb_audio_state usb_audio;
#endif /* #ifdef INSTALL_USB_AUDIO */

#ifdef INSTALL_AUDIO_DATA_SERVICE_TESTER
        struct endpoint_tester_state tester;
#endif /* INSTALL_AUDIO_DATA_SERVICE_TESTER */
#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
        struct endpoint_shadow_state shadow;
#endif /*INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
#ifdef INSTALL_TIMESTAMPED_ENDPOINT
        struct endpoint_timestamped_state timestamped;
#endif
    } state;
};


/****************************************************************************
Private Macro Declarations
*/

#define DEFINE_ENDPOINT_FUNCTIONS(name, \
    close,connect,disconnect,buffer_details,kick,sched_kick,start,stop, \
    configure,get_config,timing_info,sync) \
  const ENDPOINT_FUNCTIONS endpoint_##name = \
  { close,connect,disconnect,buffer_details,kick,sched_kick,start,stop, \
    configure,get_config,timing_info,sync }

#define IS_ENDPOINT_AUDIO_SOURCE(tag, dir) ((endpoint_##tag == endpoint_audio) && (dir == SOURCE))

/*
 * Do not include latency ctrl for Audio Source endpoints
 */
#define ENDPOINT_STATE_SIZE(tag, direction) (sizeof(struct endpoint_##tag##_state) - (IS_ENDPOINT_AUDIO_SOURCE(tag, direction) * sizeof(ENDPOINT_LATENCY_CTRL_INFO)))

#define STREAM_NEW_ENDPOINT(tag, key, direction, owner) \
          (stream_new_endpoint(&endpoint_##tag##_functions, key, ENDPOINT_STATE_SIZE(tag, direction), direction, endpoint_##tag, owner))

#define STREAM_NEW_ENDPOINT_NO_STATE(tag, key, direction, owner) \
          (stream_new_endpoint(&endpoint_##tag##_functions, key, 0, direction, endpoint_##tag, owner))

/*
 *  These are the cookies that convert between an internal id and an external
 *  id as seen by an off-chip application. (Note that these are not applied
 *  to operator endpoint ids).
 */
#define SOURCE_EP_COOKIE 0x0EA1
#define SINK_EP_COOKIE   0x0A5E

/**
 * Convert an internal endpoint id to an external endpoint id or vice-versa.
 * For operator endpoints the internal and external ID's are the same therefore
 * the ID won't be toggled.
 */
#define TOGGLE_EP_ID_BETWEEN_INT_AND_EXT(id) \
    if ((id & STREAM_EP_TYPE_MASK) == STREAM_EP_EXT_SOURCE) \
        id ^= SOURCE_EP_COOKIE; \
    else if ((id & STREAM_EP_TYPE_MASK) == STREAM_EP_EXT_SINK) \
        id ^= SINK_EP_COOKIE;

#define STREAM_EP_IS_OPEP_ID(id) (((id) & STREAM_EP_OP_BIT) == STREAM_EP_OP_BIT)
#define STREAM_EP_IS_REALEP_ID(id) (((id) & STREAM_EP_SHADOW_TYPE_MASK) == STREAM_EP_EP_BIT)
#define STREAM_EP_IS_SINK_EP(id) (((id) & STREAM_EP_TYPE_MASK) == STREAM_EP_EXT_SINK)

#define STREAM_EP_IS_OPERATOR(ep) ((ep)->stream_endpoint_type == endpoint_operator)
#define STREAM_EP_IS_REAL(ep) ((ep)->is_real)

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#define STREAM_GET_SHADOW_EP_ID(id)  ((id) & STREAM_EP_SHADOW_ID_MASK)
#define STREAM_EP_ID_FROM_SHADOW_ID(id) ((id) | STREAM_EP_SHADOW_TYPE_MASK )
#define STREAM_EP_IS_SHADOW_ID(id) (((id) & STREAM_EP_SHADOW_TYPE_MASK) == 0)

#define STREAM_EP_IS_SHADOW(ep) ((ep)->stream_endpoint_type == endpoint_shadow)
#define STREAM_EP_GET_SHADOW_EP(source_ep, sink_ep) ( (STREAM_EP_IS_SHADOW(source_ep)? source_ep : sink_ep) );

#define EP_TO_KICK_FOR_SHADOW_SOURCE(source, sink) ((STREAM_EP_IS_SHADOW(source) && STREAM_EP_IS_OPERATOR(sink))? sink: NULL)
#define EP_TO_KICK_FOR_SHADOW_SINK(source, sink) ((STREAM_EP_IS_SHADOW(sink) && STREAM_EP_IS_OPERATOR(source)) ? source: NULL)

#else
#define STREAM_EP_IS_SHADOW_ID(id) FALSE
#define STREAM_EP_IS_SHADOW(ep) FALSE
#define STREAM_EP_GET_SHADOW_EP(source_ep, sink_ep) NULL
#define EP_TO_KICK_FOR_SHADOW_SINK(source, sink) NULL
#define EP_TO_KICK_FOR_SHADOW_SOURCE(source, sink) NULL
#endif

/*
 * Opmgr has equivalent API function. Using macro internally will be more
 * efficient than calling that function.
 */
#define GET_TERMINAL_FROM_OPIDEP(opidep) \
         (((opidep) & STREAM_EP_CHAN_MASK) >> STREAM_EP_CHAN_POSN)

#define GET_BASE_EPID_FROM_EPID(epid) ((epid) & ~(STREAM_EP_CHAN_MASK |  STREAM_EP_SHADOW_MASK))

#if defined(AUDIO_SECOND_CORE)
#define STREAM_EP_IS_REAL_SHADOW_ID(id) (KIP_SECONDARY_CONTEXT() && \
                                         STREAM_EP_IS_REALEP_ID(id))
#else
#define STREAM_EP_IS_REAL_SHADOW_ID(id)  FALSE
#endif

/**
 * Propagates a kick for a given endpoint.
 *
 * \param *endpoint - pointer to the endpoint that will kick.
 * \param direction - the direction of the kick.
 */
static inline void propagate_kick(ENDPOINT* endpoint, ENDPOINT_KICK_DIRECTION direction)
{
    ENDPOINT* endpoint_to_kick = endpoint->ep_to_kick;
    if (endpoint_to_kick)
    {
        endpoint_to_kick->functions->kick(endpoint_to_kick, direction);
    }
}

/****************************************************************************
Functions from stream.c
*/

/**
 * \brief gets the first endpoint in the source or sink endpoint list
 *
 * \param dir direction of the endpoint, i.e. source or sink
 *
 */
ENDPOINT *stream_first_endpoint(ENDPOINT_DIRECTION dir);

/**
 * \brief gets the external id from the endpoint
 *
 * \param *endpoint pointer to the endpoint that we want the id
 *        from
 *
 */
unsigned stream_external_id_from_endpoint(ENDPOINT *endpoint);

/**
 * \brief obtains whether the endpoint is a source or a sink
 *
 * \param *endpoint pointer to the endpoint that we want the
 *        direction from
 *
 */
ENDPOINT_DIRECTION stream_direction_from_endpoint(ENDPOINT *endpoint);

/**
 * \brief create a new endpoint structure
 *
 * \param *functions pointer to the function table for the
 *        endpoint <br>
 * \param key unique key for the type of endpoint <br> Note that
 *        keys are not unique across different endpoint
 *        types <br>
 * \param state_size is the amount of data that the endpoints
 *        state requires <br>
 * \param dir direction of the endpoint, i.e. is it a source or
 *        a sink endpoint <br>
 * \param ep_type type of endpoint used for connection purposes
 *        <br>
 * \param con_id connection ID of the originator of this
 *        endpoint <br>
 *
 */
ENDPOINT *stream_new_endpoint(const ENDPOINT_FUNCTIONS *functions,
                              unsigned int key,
                              unsigned int state_size,
                              ENDPOINT_DIRECTION dir,
                              ENDPOINT_TYPE ep_type,
                              unsigned int con_id);

/**
 * \brief returns a pointer to a endpoint based on the endpoints
 *        key and functions <br> It also requires whether the
 *        endpoint is a source or a sink
 *
 * \param key unique key for the type of endpoint <br> Note that
 *        keys are not unique across different endpoint
 *        types <br>
 * \param dir direction of the endpoint, i.e. is it a source or
 *        a sink endpoint <br>
 * \param *functions pointer to the function table for the
 *        endpoint <br>
 *
 */
ENDPOINT *stream_get_endpoint_from_key_and_functions(unsigned int key,
                ENDPOINT_DIRECTION dir, const ENDPOINT_FUNCTIONS *functions);

/**
 * \brief Causes the endpoint to stop functioning by first
 *        tearing down any active connections, and then calling
 *        stream_destroy_endpoint
 *
 * \param *endpoint pointer to endpoint to be closed
 *
 */
bool stream_close_endpoint (ENDPOINT *endpoint);

/**
 * \brief remove endpoint from endpoint list and free the memory
 *        used by the endpoint.
 *
 * \param *endpoint pointer to endpoint to be destroyed
 *
 */
bool stream_destroy_endpoint (ENDPOINT *endpoint);

/**
 * \brief get endpoint head of synchronisation endpoint endpoint list.
 *
 * \param *endpoint pointer to endpoint to be destroyed
 *
 */
ENDPOINT *stream_get_head_of_sync(ENDPOINT* ep);

/**
 * \brief get next endpoint in sync from endpoint list
 *
 * \param *endpoint pointer to endpoint
 *
 */
ENDPOINT *stream_get_nep_in_sync(ENDPOINT* ep);

/**
 * \brief Get endpoint configuration with a single uint32 result
 * \param ep Pointer to endpoint
 * \param key
 * \param value Pointer to result variable
 */
bool stream_get_endpoint_config(ENDPOINT* ep, unsigned key, uint32* value);

/****************************************************************************
Functions from stream_audio.c
*/

/**
 * \brief Get a pointer to an audio endpoint specified by
 *        hardware, instance and channel. <br> If the endpoint
 *        doesn't exist then streams will attempt to create the
 *        endpoint
 *
 * \param con_id connection ID of the originator of this request
 * \param dir direction of the endpoint (i.e. a source or a
 *        sink) <br>
 * \param hardware the audio hardware requested e.g.
 *        PCM/FM etc... <br>
 * \param int the instance of that audio interface
 *        e.g. PCM1 or PCM2 OR the port number of the DSP interface.<br>
 * \param int the channel on the physical interface e.g.
 *        Left or Right codec <br>
 * \param pending set to TRUE by the endpoint if it needs to wait for
 *        further information before it can be used <br>
 *
 *
 */
ENDPOINT *stream_audio_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned int hardware,
                                    unsigned num_params,
                                    unsigned *params,
                                    bool *pending);
#ifdef INSTALL_SPDIF
/**
 * stream_spdif_get_endpoint
 *
 * \brief get spdif-type end point
 *
 * \param con_id connection id
 * \param dir direction, expected to be SOURCE.
 * \param port port number associated with this end point.
 * \param unused unused
 * \param *pending if the function returns success this will mean whether the endpoint creation is yet to get complete.
 *
 * \return the created end point, NULL will be returned if the endpoint cannot be created.
 */
ENDPOINT *stream_spdif_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned num_params,
                                    unsigned *params,
                                    bool *pending);
#endif
#ifdef INSTALL_MCLK_SUPPORT
/**
 * \brief Activate mclk output for an audio interface

 * \param ep endpoint
 * \param activate_output if not 0, user wants to activate mclk OUTPUT for this endpoint, otherwise it will
 *        de-activate the output. Activation/De-activation request will only be done if:
 *        - the endpoint can have mclk output (e.g i2s master)
 *        - interface wants to route the MCLK output via GPIO, Note that the MCLK output can be generated from
 *          internal clock too.
 * \param enable_mclk makes the mclk available to use by the endpoint (instead of root clock). For an interface
 *        to use MCLK we need to make sure that the MCLK is available and stable this should be able to be done
 *        automatically before an interface gets activated(normally at connection point), so we might deprecate
 *        this flag in future.
 * \param pending set to TRUE by the endpoint if it needs to wait before the mclk becomes available
 */
bool stream_audio_activate_mclk(ENDPOINT *ep, unsigned activate_output, unsigned enable_mclk, bool *pending);
#endif /* #ifdef INSTALL_MCLK_SUPPORT */

/****************************************************************************
Functions from stream_operator.c
*/

/**
 * \brief Get a pointer to an operator endpoint specified by key
 *
 * \param key the key of the endpoint
 *
 * \return pointer to the endpoint, or NULL if not found
 *
 */
ENDPOINT *stream_operator_get_endpoint_from_key(unsigned key);

/**
 * \brief Get a pointer to an operator endpoint specified by opid, channel and
 *        direction
 *
 * \param opid the endpoint to get
 * \param channel the endpoint source or sink channel index of the operator
 * \param dir the endpoint direction (\c SOURCE or \c SINK)
 * \param con_id connection ID of the originator of this request
 *
 * \return pointer to the endpoint, or NULL if not found
 *
 */
ENDPOINT *stream_operator_get_endpoint(unsigned int opid, unsigned int channel,
                                       ENDPOINT_DIRECTION dir, unsigned int con_id);

#ifdef INSTALL_SCO
/****************************************************************************
Functions from stream_sco.c
*/

/**
 * \brief Get a pointer to the sco endpoint specified by the hci handle
 *        and direction
 *
 * \param con_id connection ID of the originator of this request
 * \param dir whether a source or sink is requested
 * \param num_params the number of parameters provided
 * \param params a pointer to the creation parameters
 *
 * \return pointer to the sco endpoint or NULL if not found
 *
 */
ENDPOINT *stream_sco_get_endpoint(unsigned con_id, ENDPOINT_DIRECTION dir,
                                    unsigned num_params, unsigned *params);

/**
 * \brief Get the hci_handle from a sco endpoint
 *
 * \param endpoint pointer to the sco endpoint
 *
 * \return hci_handle
 *
 * \* note This is a function in the SCO endpoint file so that
 *         if the key is ever changed then this function should
 *         be changed along side.
 */
unsigned int stream_sco_get_hci_handle(ENDPOINT *endpoint);
#endif /* INSTALL_SCO */

#ifdef INSTALL_FILE
/****************************************************************************
Functions from stream_file.c
*/
/**
 * \brief Get a pointer to a file endpoint.
 *
 * \param con_id connection ID of the originator of this request
 * \param dir whether a source or sink is requested
 * \param num_params the number of parameters provided
 * \param params a pointer to the creation parameters
 *
 * \return pointer to the sco endpoint or NULL if not found
 *
 */
ENDPOINT *stream_file_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned num_params,
                                    unsigned *params);
#endif /* INSTALL_FILE */


#ifdef INSTALL_SIMULATION_FILE

/****************************************************************************
Functions from stream_simulation_file.c
*/
/**
 * \brief Get a pointer to a simulation file endpoint.
 *
 * \param con_id connection ID of the originator of this request
 * \param dir whether a source or sink is requested
 * \param num_params the number of parameters provided
 * \param params a pointer to the creation parameters
 *
 * \return pointer to the sco endpoint or NULL if not found
 *
 */
ENDPOINT *stream_sim_file_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned num_params,
                                    unsigned *params);

#endif /*INSTALL_SIMULATION_FILE */

#ifdef INSTALL_RAW_BUFFER
/****************************************************************************
Functions from stream_raw_buffer_hydra.c
*/
/**
 * \brief Get a pointer to a raw_buffer endpoint.
 *
 * \param con_id connection ID of the originator of this request
 * \param instance instance ID of the raw buffer
 *
 * \return pointer to raw buffer ep found/created or NULL otherwise
 *
 */

ENDPOINT *stream_raw_buffer_get_endpoint(unsigned con_id, unsigned instance);
#endif /* INSTALL_RAW_BUFFER */

/****************************************************************************
Functions from stream_a2dp
*/
#ifdef CHIP_BASE_BC7
/**
 * \brief Get a pointer to an a2dp endpoint.
 *
 * \param con_id connection ID of the originator of this request
 * \param dir whether a source or sink is requested
 * \param num_params the number of parameters provided
 * \param params a pointer to the creation parameters
 */
ENDPOINT *stream_a2dp_get_endpoint(unsigned con_id, ENDPOINT_DIRECTION dir,
                                    unsigned num_params, unsigned *params);
#endif /* CHIP_BASE_BC7 */

/****************************************************************************
Functions from stream_shunt.c
*/
#ifdef INSTALL_SHUNT
/**
 * \brief Get a pointer to a shunt endpoint specified by ACL handle,
 *        L2CAP Channel ID and direction. If the endpoint doesn't exist
 *        then streams will attempt to create the endpoint.
 *
 * \param con_id connection ID of the originator of this request
 * \param dir direction of the endpoint (i.e. a source or a sink)
 * \param pointer to parameters. There will be 2 parameters and its contents
 *        may be different between platforms.
 *
 * \return pointer to the shunt endpoint or NULL if not found and unable to
 *        create the endpoint.
 *
 */
INLINE_SECTION
static inline ENDPOINT *stream_shunt_get_endpoint(unsigned con_id,
                                                  ENDPOINT_DIRECTION dir,
                                                  unsigned num_params,
                                                  unsigned *params)
{
    return stream_a2dp_get_endpoint(con_id, dir, num_params, params);
}
#endif /* !INSTALL_SHUNT */

/****************************************************************************
Functions from stream_schedule_timers.c
*/

/**
 * \brief Sets the timing information for a real source, <br>
 *        where a real source is defined as not an operator pass
 *        through
 *
 * \param source_ep pointer to the source endpoint
 *
 *
 *  For more details on this see:
 *      http://wiki/Kymera/Design/Scheduling
 *
 */
extern void set_timing_information_for_real_source(ENDPOINT *source_ep);

/**
 * \brief Sets the timing information for a real sink, <br>
 *        where a real sink is defined as not an operator pass
 *        through
 *
 * \param sink_ep pointer to the sink endpoint
 *
 *
 *  For more details on this see:
 *      http://wiki/Kymera/Design/Scheduling
 *
 */
extern void set_timing_information_for_real_sink(ENDPOINT *sink_ep);

/**
 * \brief Enables the endpoint so that it can be kicked and scheduled periodically.
 *
 * \param *ep Pointer to the endpoint structure which needs to be enabled
 */
void stream_enable_endpoint(ENDPOINT *ep);

/**
 * \brief Disables the endpoint so that the kicks and scheduling can stop.
 *
 * \param *ep Pointer to the endpoint structure which needs to be disabled
 */
void stream_disable_endpoint(ENDPOINT *ep);

/**
 * \brief Create an endpoint using external id
 *
 * \param ep_id  The external endpoint id
 * \param ep_id  The connection id
 *
 * \return Endpoint if successful else NULL.
 */
ENDPOINT* stream_create_endpoint( unsigned ep_id, unsigned con_id);

/**
 * \brief Check the type and destroy the endpoint
 *        It destroys only operator endpoint and KIP endpoint
 *
 * \param ep_id  The endpoint id
 *
 * \return Endpoint if successfully else NULL.
 */
bool stream_destroy_endpoint_id( unsigned ep_id);

/****************************************************************************
Functions from stream_connect.c
*/
/**
 * \brief Creates a transform structure and places it in the transform list.
 *        This function then returns a pointer to the transform.
 *
 * \param *source_ep pointer to the source endpoint in the connection
 * \param *sink_ep pointer to the sink endpoint in the connection
 */
TRANSFORM *stream_new_transform(ENDPOINT *source_ep, ENDPOINT *sink_ep, unsigned transform_id);

/**
 * \brief Called during the creation of a transform to resolve differing
 *        source and sink data formats.
 *
 * \param source_ep pointer to the source endpoint in the connection
 * \param sink_ep pointer to the sink endpoint in the connection
 * \return \c TRUE if successful, \c FALSE otherwise
 */
bool stream_resolve_endpoint_data_formats(ENDPOINT *source_ep, ENDPOINT *sink_ep);

/**
 * \brief If the chain is complete, this function should
 *        set everything up for scheduling, priming and
 *        rate matching, and cbops if required
 *
 * \param src_ep source endpoint in the chain to work on.
 *
 * \param sink_ep source endpoint in the chain to work on.
 */
bool stream_chain_update(ENDPOINT *src_ep, ENDPOINT *sink_ep);

/**
 * \brief Returns the value that is most suitable for zeroing buffer content.
 *  It relies on endpoint data format so this has to be set up already.
 *
 * \param ep  endpoint in the chain to look at.
 *
 * \return The "zero" value for the endpoint buffer.
 */
unsigned int get_ep_buffer_zero_value(ENDPOINT* ep);

/****************************************************************************
Functions from stream.c
*/

/**
 * \brief connect a source endpoint to a sink endpoint
 *
 * \param *source_ep pointer to the source endpoint
 * \param *sink_ep pointer to the sink endpoint
 * \param transform_id In case of multicore: if on secondary core(s) and non-zero: the internal
 *                     transform ID is imposed by caller, otherwise if zero it is generated internally.
 *
 */
TRANSFORM *stream_connect_endpoints(ENDPOINT *source_ep, ENDPOINT *sink_ep,
                                    STREAM_CONNECT_INFO *state_info,
                                    unsigned transform_id);

/**
 * \brief Get the endpoint buffer for the stream connection
 *
 * \param *source_ep pointer to the source endpoint
 * \param *sink_ep pointer to the sink endpoint
 * \param state_info - Connects state information will have the endpoint
 *                     buffer information.
 *
 * \return TRUE on getting the buffer successfully
 */
bool stream_connect_get_buffer( ENDPOINT *source_ep,
                                ENDPOINT *sink_ep,
                                STREAM_CONNECT_INFO* state_info);

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
ENDPOINT* stream_shadow_ep_from_data_channel(uint16 data_chan_id);
ENDPOINT *stream_shadow_get_endpoint(unsigned int epid);

/**
 * \brief kick the kip endpoints on receiving kip signals
 *
 * \param data_channel_id data channel id
 * \param kick_dir  The kick direction
 *
 * \return void
 */
void stream_kick_kip_eps( uint16 data_chan_id,
                          ENDPOINT_KICK_DIRECTION kick_dir);

#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */

/****************************************************************************
Functions from stream_monitor_interrupt.c
*/

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)

/**
 * \brief Enables read-monitor interrupts of specified type for the given endpoint.
 *
 * \param ep pointer to the audio endpoint that should be monitored
 * \param handle mmu_handle associated with the local buffer attached to the endpoint.
 * \param ko pointer to the KICK_OBJECT that will be kicked
 * \param etype type of event used by the monitor interrupt
 *
 * \note  We expect ep->state.audio.monitor_threshold to contain the monitor
 *        threshold. This specifies the number of samples between interrupts.
 *        In all scenarios we care about, one sample (be it 8-bit or 16-bit)
 *        takes up one word in buffer memory.
 *
 * \return Returns False if no dedicated read monitor interrupt is available
 */
extern bool stream_monitor_int_rd_enable_with_event_type(ENDPOINT* ep,
                                                         mmu_handle handle,
                                                         KICK_OBJECT *ko,
                                                         KICK_EVENT_TYPE etype);
/* Default enable with event type 0 */
#define stream_monitor_int_rd_enable(e,h,k) \
            stream_monitor_int_rd_enable_with_event_type((e),(h),(k),KICK_EVENT_TYPE_0)

/**
 * \brief Enables write-monitor interrupts of specified type for the given endpoint.
 *
 * \param ep pointer to the endpoint that should be monitored
 * \param handle mmu_handle associated with the local buffer attached to the endpoint.
 * \param ko pointer to the KICK_OBJECT that will be kicked
 * \param etype type of event used by the monitor interrupt
 *
 * \note  We expect ep->state.audio.monitor_threshold to contain the monitor
 *        threshold. This specifies the number of samples between interrupts.
 *        In all scenarios we care about, one sample (be it 8-bit or 16-bit)
 *        takes up one word in buffer memory.
 *
 * \return Returns False if no dedicated write monitor interrupt is available
 */
extern bool stream_monitor_int_wr_enable_with_event_type(ENDPOINT* ep,
                                                         mmu_handle handle,
                                                         KICK_OBJECT *ko,
                                                         KICK_EVENT_TYPE etype);
/* Default enable with event type 0 */
#define stream_monitor_int_wr_enable(e,h,k) \
            stream_monitor_int_wr_enable_with_event_type((e),(h),(k),KICK_EVENT_TYPE_0)

#ifdef INSTALL_SPDIF
extern bool stream_spdif_monitor_int_wr_enable_with_event_type(ENDPOINT* ep,
                                                         mmu_handle handle,
                                                         KICK_OBJECT *ko,
                                                         KICK_EVENT_TYPE etype);

/* Default enable with event type 0 */
#define stream_spdif_monitor_int_wr_enable(e,h,k) \
            stream_spdif_monitor_int_wr_enable_with_event_type((e),(h),(k),KICK_EVENT_TYPE_0)
#endif /*# ifdef INSTALL_SPDIF */
#endif          /* CHIP_BASE_HYDRA || CHIP_BASE_NAPIER */

/****************************************************************************
Dummy Functions for function tables
*/

/* These are not going in to doxygen */
void stream_kick_dummy(ENDPOINT *ep, bool aod_valid, ENDPOINT_KICK_DIRECTION kick_dir);
void stream_sched_kick_dummy(ENDPOINT *ep, KICK_OBJECT *ko);
bool stream_close_dummy(ENDPOINT *ep);
bool stream_sync_sids_dummy(ENDPOINT *ep1, ENDPOINT *ep2);
ENDPOINT *stream_head_of_sync_dummy(ENDPOINT *ep);

/******************************************************************************
 * Functions declarations related to deferred kicks (i.e. delegated
 * from interrupt context to high priority bg_int)
 */
bool stream_set_deferred_kick(ENDPOINT* ep, bool deferred);
void stream_destroy_deferred_kick(ENDPOINT* ep);

/******************************************************************************
 * Protected Functions Declarations
 *
 * Functions only available to implementations of the base audio endpoint
 */

/*
 * \brief Generates a stream key based on the hardware, instance and channel
 *
 * \param hardware audio hardware to be used <br>
 * \param instance physical instance of hardware to be used <br>
 * \param channel channel, slot or port on hardware interface to be used <br>
 *
 */
extern unsigned create_stream_key(unsigned int hardware, unsigned int instance,
                                unsigned int channel);

/**
 * \brief Retrieves the audio hardware type of an audio endpoint.
 *
 * \param ep The endpoint to get the hardware type of
 *
 * \return The hardware type of the endpoint
 */
extern unsigned get_hardware_type(ENDPOINT *ep);

/**
 * \brief Retrieves the audio hardware instance of an audio endpoint
 *
 * \param ep The endpoint to get the hardware instance of
 *
 * \return The hardware instance of the endpoint.
 */
extern unsigned get_hardware_instance(ENDPOINT *ep);

/**
 * \brief Retrieves the audio hardware channel of an audio endpoint
 *
 * \param ep The endpoint to get the hardware channel of
 *
 * \return The hardware channel of the endpoint.
 */
extern unsigned get_hardware_channel(ENDPOINT *ep);

/*
 * \brief Add an endpoint to the synchronisation list.
 *
 * \param ep1 endpoint
 * \param ep2 endpoint
 */
bool add_to_sync_list(ENDPOINT *ep1, ENDPOINT *ep2);

/*
 * \brief Remove an endpoint to the synchronisation list.
 *
 * \param ep endpoint
 *
 */
bool remove_from_sync_list(ENDPOINT *ep);

/**
 * \brief tell audio to sync two endpoints
 *
 * Audio endpoints are synchronised by hardware control, i.e., audio hardware reads both
 * endpoints at same time. This is useful for stereo processing. This function can be
 * used to sync two endpoints (or 2 separate group of endpoints) or to unsync an
 * endpoint from its sync group.
 * The synchronisation group is also maintained as a linked list in the endpoint state.
 * Each endpoint stores the head of the list and the next in the list. Storing the head
 * of list is easier to search through the list. However, when an endpoint is added to/
 * removed from the head, all the endpoints in the list have to be updated with the
 * change.
 *
 * \param *ep1 pointer to the first endpoint to sync
 * \param *ep2 pointer to the second endpoint to sync
 *
 * \return Whether the request succeeded.
 */
bool sync_endpoints (ENDPOINT *ep1, ENDPOINT *ep2);

/**
 * \brief Gets the ratematching capability an audio endpoint has in it's current
 * configuration.
 *
 * \param endpoint The endpoint that is being queried for its ratematching ability
 * \param value pointer where the ratematching capability is returned
 *
 * \return TRUE if value was populated.
 */
bool audio_get_config_rm_ability(ENDPOINT *endpoint, uint32 *value);

/**
 * \brief Sets up an audio Endpoint to perform or stop performing rateadjustment.
 *
 * \param endpoint  The endpoint that is being asked to perform rateadjustment.
 * \param value  The configure value containing a bool indicating enable or disable
 *
 * \return TRUE if the request was satisfied.
 */
bool audio_configure_rm_enacting(ENDPOINT *endpoint, uint32 value);

/**
 * \brief get the audio data format of the underlying hardware associated with
 * the endpoint. This function MUST be implemented by all implementations of an
 * audio endpoint.
 *
 * \param endpoint pointer to the endpoint to get the data format of.
 *
 * \return the data format of the underlying hardware
 */
AUDIO_DATA_FORMAT audio_get_data_format (ENDPOINT *endpoint);

/******************************************************************************
 * Protected Functions Declarations
 *
 * Functions only available to implementations of the base sco endpoint
 */
/**
 * \brief  TODO
 */
AUDIO_DATA_FORMAT sco_get_data_format (ENDPOINT *endpoint);

void sco_sched_kick(ENDPOINT *endpoint, KICK_OBJECT *ko);
bool sco_start(ENDPOINT *endpoint, KICK_OBJECT *ko);
bool sco_stop(ENDPOINT *endpoint);
bool sco_configure(ENDPOINT *endpoint, unsigned int key, uint32 value);
bool sco_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
void sco_common_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);

#endif /*_STREAM_ENDPOINT_H_*/
