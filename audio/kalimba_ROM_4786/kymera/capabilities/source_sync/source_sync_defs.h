/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef SOURCE_SYNC_DEFS_H
#define SOURCE_SYNC_DEFS_H

/*****************************************************************************
Include Files
*/
/* Enable printf format checking in _gcc_warnings builds */
#define AUDIO_LOG_CHECK_FORMATS

#include "types.h"
#include "capabilities.h"
#include "common_conversions.h"
#include "platform/pl_assert.h"
#include "platform/pl_intrinsics.h"
#include "platform/pl_fractional.h"
#include "pl_timers/pl_timers.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "op_msg_helpers.h"
#include "buffer/cbuffer_c.h"
#include "cbops_mgr/cbops_mgr.h"
#include "cbops_mgr/cbops_flags.h"
#ifdef INSTALL_METADATA
#include "buffer/buffer_metadata.h"
#include "hydra_cbuff.h"
#endif /* INSTALL_METADATA */
#include "rate/rate.h"
#include "audio_log/audio_log.h"
#include "stream/stream.h"
#include "fault/fault.h"
#include "source_sync_gen_c.h"


/****************************************************************************
Module Private Macros
*/

#define SRC_SYNC_VERSION_MINOR 3

/* Define this to enable most non-warning trace printouts */
/* #define SOSY_VERBOSE */

/* Define this to number log messages, in order to detect gaps */
#define SOSY_NUMBERED_LOG_MESSAGES

/* Define this in order to only see the first n messages */
#define SOSY_LOG_MESSAGE_LIMIT (MAXINT)

/* Define to check alignment of output metadata and cbuffer pointers
 * before/after transfer */
/* #define SOSY_CHECK_METADATA_TRANSPORT_POINTERS */

/* Define to check for metadata tag leaks */
/* #define SOSY_CHECK_METADATA_TAG_COUNTS */

#ifdef SOSY_VERBOSE

#ifdef SOSY_NUMBERED_LOG_MESSAGES
#define SOSY_PFX_FMT "src_sync #%d "
#define SOSY_PFX_ARG src_sync_trace_serial++
#else /* SOSY_NUMBERED_LOG_MESSAGES */
#define SOSY_PFX_FMT "src_sync 0x%04x "
#define SOSY_PFX_ARG op_extra_data->id
#endif /* SOSY_NUMBERED_LOG_MESSAGES */

#if defined(SOSY_NUMBERED_LOG_MESSAGES)&&defined(SOSY_LOG_MESSAGE_LIMIT)
#define SOSY_MSG_PRE(E) do { if(((op_extra_data->trace_enable & (E)) == (E)) && (src_sync_trace_serial <= src_sync_trace_limit))
#define SOSY_MSG_POST   ; } while(0)
#else
#define SOSY_MSG_PRE(E) do { if((op_extra_data->trace_enable & (E)) == (E))
#define SOSY_MSG_POST   ; } while(0)
#endif

#define SOSY_MSG(E,F)                 SOSY_MSG_PRE(E) L2_DBG_MSG1(SOSY_PFX_FMT F, SOSY_PFX_ARG) SOSY_MSG_POST
#define SOSY_MSG1(E,F,P1)             SOSY_MSG_PRE(E) L2_DBG_MSG2(SOSY_PFX_FMT F, SOSY_PFX_ARG, (P1)) SOSY_MSG_POST
#define SOSY_MSG2(E,F,P1,P2)          SOSY_MSG_PRE(E) L2_DBG_MSG3(SOSY_PFX_FMT F, SOSY_PFX_ARG, (P1),(P2)) SOSY_MSG_POST
#define SOSY_MSG3(E,F,P1,P2,P3)       SOSY_MSG_PRE(E) L2_DBG_MSG4(SOSY_PFX_FMT F, SOSY_PFX_ARG, (P1),(P2),(P3)) SOSY_MSG_POST
#define SOSY_MSG4(E,F,P1,P2,P3,P4)    SOSY_MSG_PRE(E) L2_DBG_MSG5(SOSY_PFX_FMT F, SOSY_PFX_ARG, (P1),(P2),(P3),(P4)) SOSY_MSG_POST
#else /* SOSY_VERBOSE */
#define SOSY_MSG(E,F)                 do{}while(0)
#define SOSY_MSG1(E,F,P1)             do{}while(0)
#define SOSY_MSG2(E,F,P1,P2)          do{}while(0)
#define SOSY_MSG3(E,F,P1,P2,P3)       do{}while(0)
#define SOSY_MSG4(E,F,P1,P2,P3,P4)    do{}while(0)
#endif /* SOSY_VERBOSE */

#ifdef INSTALL_METADATA
#define SRC_SYNC_IF_METADATA(X) X
#else
#define SRC_SYNC_IF_METADATA(X)
#endif

/** Keep this in sync with src_sync_sink_state */
#define SRC_SYNC_FOR_EACH_SINK_STATE(X) \
    X(NOT_CONNECTED) \
    X(FLOWING) \
    X(PENDING) \
    X(STALLED) \
    X(RECOVERING_RESTARTING) \
    X(RECOVERING_FILLING) \
    SRC_SYNC_IF_METADATA( \
        X(RECOVERING_DISCARDING) \
        X(RECOVERING_WAITING_FOR_TAG) \
    )

/** Asserts which check internal consistency (due to refactoring,
 * caching conditions, etc.)
 */
#if defined(SOURCE_SYNC_ENABLE_CHECKS) || defined(RUNNING_ON_KALSIM)
#define SRC_SYNC_DEV_ASSERTS
#endif

#ifdef SRC_SYNC_DEV_ASSERTS
#define SOSY_DEV_ASSERT(COND) PL_ASSERT(COND)
#else
#define SOSY_DEV_ASSERT(COND) do{}while(0)
#endif

/****************************************************************************
Module Private Constant Definitions
*/

enum {
    /** default source buffer size for this operator;
     * the value 0 causes some automatic calculations to take place instead */
    SRC_SYNC_DEFAULT_OUTPUT_BUFFER_SIZE     = 0,

    /** Default buffer size is this factor in Q6.N x system kick period x sample rate */
    SRC_SYNC_AUTO_BUFFER_SS_PERIOD_MULT     = (2 * (1<<(DAWTH-6))),

    /** default sample rate for buffer size calculations */
    SRC_SYNC_DEFAULT_SAMPLE_RATE            = 48000,

    /** default block size for this operator's terminals */
    SRC_SYNC_DEFAULT_BLOCK_SIZE             = 1,

    /** Max number of terminals on either side */
    SRC_SYNC_CAP_MAX_CHANNELS               = 24,

    /** Bit field position of clock index */
    SRC_SYNC_CLOCK_INDEX_LSB_POSN           = 7
};

/** Bitmask of all channel bits; #define because of range */
#define SRC_SYNC_CAP_CHANNELS_MASK          0xFFFFFFU

/**
 * Keep this in sync with SRC_SYNC_FOR_EACH_SINK_STATE.
 * The enum is not defined using that macro in order
 * to allow doxygen comments here.
 */
typedef enum src_sync_sink_state_enum
{
    /** Input is not connected. */
    SRC_SYNC_SINK_NOT_CONNECTED,

    /** Input is flowing at a sustained rate */
    SRC_SYNC_SINK_FLOWING,

    /** Input is limiting transfers but not stalled yet */
    SRC_SYNC_SINK_PENDING,

    /** Waiting longer for input would have caused outputs
     * to underrun. Silence is being inserted.
     */
    SRC_SYNC_SINK_STALLED,

    /** Following a stall, and data being available again, and having
     * determined that the new data constitutes a new start of stream,
     * fill downstream with silence until full, before starting
     * to forward data.
     */
    SRC_SYNC_SINK_RECOVERING_RESTARTING,

    /** Following a stall, and data being available again, and having
     * determined that more silence needs to be still inserted, wait
     * for the downstream to consume the additional silence.
     */
    SRC_SYNC_SINK_RECOVERING_FILLING,

#ifdef INSTALL_METADATA
    /** Following a stall, and data being available again, and having
     * determined that data needs to be discarded, more upstream
     * data is needed before resuming copy.
     */
    SRC_SYNC_SINK_RECOVERING_DISCARDING,

    /** Following a stall of a stream which contained
     * TTP or ToA tags previously, data has arrived again
     * but no tag yet. Discard up to the first tag.
     */
    SRC_SYNC_SINK_RECOVERING_WAITING_FOR_TAG,
#endif /* INSTALL_METADATA */

    SRC_SYNC_NUM_SINK_STATES
} src_sync_sink_state;

#ifdef INSTALL_METADATA
typedef enum src_sync_stall_recovery_type_enum
{
    /** The size of a gap, if any, is not known (usually because
     * the input stream has no TTP or ToA tags.)
     */
    SRC_SYNC_STALL_RECOVERY_UNKNOWN,

    /** The audio before and after the stall had timestamps,
     * so the size of the gap could be determined and looked sane.
     */
    SRC_SYNC_STALL_RECOVERY_GAP,

    /** The metadata after the stall contained a stream start flag,
     * or timestamps were available but did not look like a
     * continuation.
     */
    SRC_SYNC_STALL_RECOVERY_RESTART,

    /** There was metadata with time before the stall but not yet
     * after the stall. Keep waiting for tags.
     */
    SRC_SYNC_STALL_RECOVERY_WAITING_FOR_TAG
} src_sync_stall_recovery_type;

#endif /* INSTALL_METADATA */
#ifdef SOSY_VERBOSE
/* Bits for the argument to the debug opmsg 0x10 */
typedef enum src_sync_trace_enable_enum
{
    SRC_SYNC_TRACE_ALWAYS           = 0,
    SRC_SYNC_TRACE_KICK             = (1<< 0),
    SRC_SYNC_TRACE_REFRESH          = (1<< 1),
    SRC_SYNC_TRACE_TRANSITION       = (1<< 2),
    SRC_SYNC_TRACE_SINK_STATE       = (1<< 3),
    SRC_SYNC_TRACE_COMPUTE_TRANSFER = (1<< 4),
    SRC_SYNC_TRACE_SRC_SPACE        = (1<< 5),
    SRC_SYNC_TRACE_SRC_TERM_SPACE   = (1<< 6),
    SRC_SYNC_TRACE_SRC_SPACE_FILLED = (1<< 7),
    SRC_SYNC_TRACE_SINK_AVAIL       = (1<< 8),
    SRC_SYNC_TRACE_FLOWING          = (1<< 9),
    SRC_SYNC_TRACE_PENDING          = (1<<10),
    SRC_SYNC_TRACE_STALLED          = (1<<11),
    SRC_SYNC_TRACE_RECOVERED        = (1<<12),
    SRC_SYNC_TRACE_RECOVER_DISCARD  = (1<<13),
    SRC_SYNC_TRACE_PERFORM_TRANSFER = (1<<14),
    SRC_SYNC_TRACE_PEEK_RESUME      = (1<<15),
    SRC_SYNC_TRACE_RCV_GAP          = (1<<16),
    SRC_SYNC_TRACE_FWD_SPLICE       = (1<<17),
    SRC_SYNC_TRACE_METADATA         = (1<<18),
    SRC_SYNC_TRACE_RCV_METADATA     = (1<<19),
    SRC_SYNC_TRACE_SEND_METADATA    = (1<<20),
    SRC_SYNC_TRACE_EOF              = (1<<21),
    SRC_SYNC_TRACE_RATE_MATCH       = (1<<22),
    SRC_SYNC_TRACE_RM_TRANSFER      = (1<<23)
} src_sync_trace_enable;

enum {
    SRC_SYNC_DEFAULT_TRACE_ENABLE =
            /* Comment out as desired for debugging
             * (TODO: perhaps a MIB key would be more convenient.) */
            /* SRC_SYNC_TRACE_KICK | */
            /* SRC_SYNC_TRACE_REFRESH | */
            /* SRC_SYNC_TRACE_TRANSITION | */
            /* SRC_SYNC_TRACE_SINK_STATE | */
            /* SRC_SYNC_TRACE_COMPUTE_TRANSFER | */
            /* SRC_SYNC_TRACE_SRC_SPACE | */
            /* SRC_SYNC_TRACE_SRC_TERM_SPACE | */
            /* SRC_SYNC_TRACE_SRC_SPACE_FILLED | */
            /* SRC_SYNC_TRACE_SINK_AVAIL | */
            /* SRC_SYNC_TRACE_FLOWING | */
            /* SRC_SYNC_TRACE_PENDING | */
            /* SRC_SYNC_TRACE_STALLED | */
            /* SRC_SYNC_TRACE_RECOVERED | */
            /* SRC_SYNC_TRACE_RECOVER_DISCARD | */
            /* SRC_SYNC_TRACE_PERFORM_TRANSFER | */
            /* SRC_SYNC_TRACE_PEEK_RESUME | */
            /* SRC_SYNC_TRACE_RCV_GAP | */
            /* SRC_SYNC_TRACE_FWD_SPLICE | */
            /* SRC_SYNC_TRACE_METADATA | */
            /* SRC_SYNC_TRACE_RCV_METADATA | */
            /* SRC_SYNC_TRACE_SEND_METADATA | */
            /* SRC_SYNC_TRACE_EOF | */
            /* SRC_SYNC_TRACE_RATE_MATCH | */
            /* SRC_SYNC_TRACE_RM_TRANSFER | */
            0
};
#endif /* SOSY_VERBOSE */

enum {
    /** The maximum value is extreme but so far legal:
     * it is the case of each terminal being used and in a group
     * by itself (i.e. the upper bound on the number of source + sink groups)
     */
    SRC_SYNC_BUFFER_LEVEL_HISTORY_MAX = 2 * SRC_SYNC_CAP_MAX_CHANNELS,

    /* The period, in microseconds, over which to accumulate buffer levels.
     * This should span an entire kick period with a little margin for uneven
     * scheduling.
     */
    SRC_SYNC_BUFFER_LEVEL_HISTORY_PERIOD = 2100,

    /**
     * Extra precision of sample period
     */
    SRC_SYNC_INV_RATE_RESOLUTION = 10,

    /** From stream_endpoint.h: "The number of samples headroom to allow in
     * the buffer to compensate for any rate missmatch variation in buffer
     * levels before the RM system compensates"
     */
    SRC_SYNC_RM_HEADROOM_AMOUNT = 2,

    /** Scale difference between sp_adjust in metadata tags (Q1.N)
     * and rate measurement (QM.22)
     */
    SRC_SYNC_SCALE_SP_ADJUST_TO_RATE_MEASURE =
            (DAWTH-1-STREAM_RATEMATCHING_FIX_POINT_SHIFT)
};


/****************************************************************************
Module Private Type Definitions
*/

/* Forward declare struct types for mutual references */
typedef struct src_sync_terminal_group  SRC_SYNC_TERMINAL_GROUP;
typedef struct src_sync_sink_group      SRC_SYNC_SINK_GROUP;
typedef struct src_sync_source_group    SRC_SYNC_SOURCE_GROUP;

typedef struct src_sync_terminal_entry  SRC_SYNC_TERMINAL_ENTRY;
typedef struct src_sync_sink_entry      SRC_SYNC_SINK_ENTRY;
typedef struct src_sync_source_entry    SRC_SYNC_SOURCE_ENTRY;

typedef struct src_sync_rm_state_struct SRC_SYNC_RM_STATE;

/**
 * Buffer level history
 */
typedef struct src_sync_buffer_level_record
{
    TIME                            timestamp;
    unsigned                        amount;
}
SRC_SYNC_BUFFER_LEVEL_RECORD;

typedef struct src_sync_buffer_level_history
{
    unsigned                        num_entries;
    unsigned                        next_wr_pos;
    SRC_SYNC_BUFFER_LEVEL_RECORD    *entries;
}
SRC_SYNC_BUFFER_LEVEL_HISTORY;

#ifdef INSTALL_METADATA
typedef struct src_sync_sink_metadata_str
{
    /** Tag list removed from current buffer */
    metadata_tag*                   received;

    /** The two index distances returned by buff_metadata_remove */
    unsigned                        rcv_beforeidx;
    unsigned                        rcv_afteridx;

    /** Number of samples into the next buffer that are covered by last tag */
    unsigned                        remaining_octets;

    /** Type of timestamp seen on this stream */
    RATE_TIMESTAMP_TYPE             timestamp_type;

    /** Coarse timestamp at start of next buffer */
    TIME                            ts_start_of_next_buffer;

    /** Sp_adjust from last tag */
    int                             sp_adjust;
}
SRC_SYNC_SINK_METADATA;

typedef struct src_sync_source_metadata_str {
    /** Octets ahead covered by tags */
    unsigned                        remaining_octets;

    /** Reuse tag containing EOF rather than delete */
    metadata_tag*                   eof_tag;

    /** Enable TTP on output */
    bool                            provide_ttp;

} SRC_SYNC_METADATA_DEST;

#endif /* INSTALL_METADATA */

/* Common to source and sink groups */
struct src_sync_terminal_group
{
    SRC_SYNC_TERMINAL_GROUP         *next;
    SRC_SYNC_TERMINAL_ENTRY         *terminals;
    unsigned                        channel_mask;

    /**
     * Nominal sample rate in Hz
     */
    unsigned                        sample_rate;

    /**
     * Nominal sample period, fractional x 2^-10 s
     * (the exponent is SRC_SYNC_INV_RATE_RESOLUTION)
     */
    unsigned                        inv_sample_rate;

    /* amount of space/data in words */
    unsigned                        transfer;

    /* This is used temporarily in set_route */
    unsigned                        tmp_sample_rate;

    /* Number of group (in the order defined in set_..._groups) */
    unsigned                        idx                 : 5;

    /** Set if all terminals in this group are connected.
     * (Maintained by connect/disconnect.)
     */
    bool                            connected           : 1;

#ifdef INSTALL_METADATA
    /** Metadata is enabled by set_sink_groups/set_source_groups */
    bool                            metadata_enabled    : 1;

    tCbuffer*                       metadata_buffer;
#endif /* INSTALL_METADATA */
};

struct src_sync_sink_group
{
    SRC_SYNC_TERMINAL_GROUP         common;

    SRC_SYNC_SOURCE_GROUP*          route_dest;

    /** This flag controls whether to consume data if all terminals in
     * the group are connected and none are routed.
     */
    bool                            purge               : 1;

    /** Configure whether rate matching is supported */
    bool                            rate_adjust_enable  : 1;

#ifdef INSTALL_METADATA
    /** Set for the first connected non-rate-adjusting group with metadata */
    bool                            ts_rate_master      : 1;
#endif /* INSTALL_METADATA */

    /** States through the flowing-pending-stalled-recovering cycle */
    src_sync_sink_state             stall_state;

    /** Number of zero samples written during stall */
    unsigned                        inserted_silence_words;

    /** Number of zero samples remaining to write during recovery
     * in state SRC_SYNC_SINK_RECOVERING_FILLING
     */
    unsigned                        stall_recovery_silence_words;

    /** Number of zero samples remaining to discard during recovery
     * in state SRC_SYNC_SINK_RECOVERING_DISCARDING
     */
    unsigned                        stall_recovery_discard_words;

    /** Limit on the SRC_SYNC_SINK_RECOVERING_DISCARDING state
     * in terms of silence words
     */
    unsigned                        stall_recovery_discard_remaining;

    /** Direction of transitions between copying and silence */
    bool                            copy_before_silence     : 1;

    /** In state RECOVERING_FILLING, stop filling as soon as downstream full.
     * Otherwise, stall_recovery_silence_words samples of silence will
     * always be generated.
     */
    bool                            filling_until_full      : 1;

#ifdef INSTALL_METADATA
    /** Source group where to route the incoming metadata */
    SRC_SYNC_SOURCE_GROUP*          metadata_dest;

    /** Information about the last timestamp seen */
    SRC_SYNC_SINK_METADATA          timestamp_state;
#endif /* INSTALL_METADATA */

    /** Metadata input buffer. Either the metadata connection
     * buffer (common->metadata_buffer) or the output of the
     * rate adjustment stage.
     */
    tCbuffer*                       metadata_input_buffer;

    /** Saved setting from the ratematch manager.
     * It may be set before connect.
     */
    bool                            rm_enact;

    /** Optional rate adjustment fields */
    SRC_SYNC_RM_STATE*              rm_state;
};

struct src_sync_source_group
{
    SRC_SYNC_TERMINAL_GROUP         common;

    /** Minimum amount to transfer to avoid underrun */
    unsigned                        min_transfer_w;

#ifdef INSTALL_METADATA
    SRC_SYNC_METADATA_DEST          metadata_dest;

    /** Sink group which provides metadata */
    SRC_SYNC_SINK_GROUP*            metadata_in;

#ifdef SOSY_CHECK_METADATA_TRANSPORT_POINTERS
    /** Copy of metadata write index before last transfer */
    unsigned                        last_md_write_idx;
#endif
#endif /* INSTALL_METADATA */
};

/* Common to source and sink entries */
struct src_sync_terminal_entry
{
    /* Efficient iteration over connected and routed terminals */
    SRC_SYNC_TERMINAL_ENTRY         *next;

    /** Index of the terminal in the operator */
    unsigned                        idx;

    /** Index of the channel in a stream */
    unsigned                        idx_in_group;

    /* Connection buffer */
    tCbuffer                        *buffer;

    /* Transfer, relative rate and metadata */
    SRC_SYNC_TERMINAL_GROUP*        group;
};

/* Linked list of Sinks */
struct src_sync_sink_entry
{
    SRC_SYNC_TERMINAL_ENTRY         common;

    /** Route destination */
    SRC_SYNC_SOURCE_ENTRY*          source;

    /** Input buffer. If rate adjustment is enabled,
     * this is the output buffer of the SRA cbops chain,
     * otherwise this is the connection buffer.
     */
    tCbuffer*                       input_buffer;
};

typedef struct src_sync_route_entry
{
    /* Designed gain in dB*60 */
    int                             gain_dB;

    /* Gain applied to route (Q5.XX) */
    unsigned                        gain_lin;

    /* Sample rate of route. (zero if no route) */
    unsigned                        sample_rate;

    /* Sample period, fractional x 2^-10 s
     * (the exponent is SRC_SYNC_INV_RATE_RESOLUTION)
     */
    unsigned                        inv_sample_rate;

    /* Pointer to Sink Structure */
    SRC_SYNC_SINK_ENTRY             *sink;

} SRC_SYNC_ROUTE_ENTRY;

/* Linked list of Sources */
struct src_sync_source_entry
{
    SRC_SYNC_TERMINAL_ENTRY         common;

    /** Inverse Transition period for switching Sinks.
        inv_transition is zero for immediate switch
        (inv_transition)  applied to switch out old sink
        (-inv_transition) applied to switch in new sink
        inv_transition = zero when switch complete*/
    int                             inv_transition;

    /** Progress of transition */
    unsigned                        transition_pt;

    /** Current route, containing sink, rate and gain. */
    SRC_SYNC_ROUTE_ENTRY            current_route;

    /** Route to be set after out-transition completes */
    SRC_SYNC_ROUTE_ENTRY            switch_route;

    /** Next source whose route has the same sink, if any */
    SRC_SYNC_SOURCE_ENTRY           *next_split_source;

#ifdef SOSY_CHECK_METADATA_TRANSPORT_POINTERS
    /** Copy of cbuffer write index before last transfer */
    unsigned                        last_cb_write_idx;
#endif
};


/* capability-specific extra operator data */
typedef struct src_sync_exop
{
    unsigned                forward_kicks;       /* Sources connected with routes */
    unsigned                sinks_connected;     /* Sink terminals connected */
    unsigned                sources_connected;   /* Source terminals connected */

    SRC_SYNC_SINK_ENTRY     *sinks[SRC_SYNC_CAP_MAX_CHANNELS];
    SRC_SYNC_SOURCE_ENTRY   *sources[SRC_SYNC_CAP_MAX_CHANNELS];
    
    SRC_SYNC_SINK_GROUP     *sink_groups;
    SRC_SYNC_SOURCE_GROUP   *source_groups;

    /** Bitmap of sink terminals covered by sink groups */
    unsigned                sink_group_mask;

    /** Bitmap of source terminals covered by source groups */
    unsigned                source_group_mask;

    /** Current CPS parameter values */
    SOURCE_SYNC_PARAMETERS  cur_params;

    CPS_PARAM_DEF           parms_def;

    /** Update internal lists at start of next data processing,
     * based on changes to connection/configuration.
     * \note Use byte access */
    bool                    bRinit : 8;

    /** Prevent process_data from using configuration structures
     * which are being modified. Set/cleared by opmsg/cmd handlers,
     * when cleared, self-kick to ensure updates.
     * \note Use byte access */
    bool                    dont_process : 8;

    /** Cached condition equal to: all routed sinks are unconnected.
     * Only in this state, limit the rate at which silence is produced,
     * based on the system clock and nominal rate.
     * \note Use byte access */
    bool                    all_routed_sinks_unconnected : 8;

    /* CPS statistics fields */
    unsigned                Dirty_flag;
    /** Status bitmap for stalled sink terminals */
    unsigned                stat_sink_stalled;
    /** Status bitmap for stalled sink terminals (cleared by reading) */
    unsigned                stat_sink_stall_occurred;

    /** Minimum of the output buffer sizes (i.e. upper bound
     * on available space), as time */
    unsigned                max_space_t;
    TIME                    time_stamp;
    unsigned                est_latency;

    /** Bitmap of source terminals with pending route switch
     * (i.e. always equal to an aggregate of
     * src_ptr->switch_route.sink != NULL for all sources)
     */
    unsigned                src_route_switch_pending_mask;

    /** Bitmap of sink terminals with rate adjustment enabled
     * (from SET_SINK_GROUPS)
     */
    unsigned                sink_rm_enabled_mask;

    /** Bitmap of sink terminals with rate adjustment initialised */
    unsigned                sink_rm_initialised_mask;

    /** Sample period deviation for the primary clock
     * i.e. all sources, and non-rate-adjusted sinks.
     * Scaled and represented like sp_adjust in metadata tags,
     * i.e. Q1.N not scaled,
     * actual_sample_period = (1 + sp_adjust) nominal_sample_period
     */
    int                     primary_sp_adjust;

    /** True if receiving rate monitor measurements
     * from a downstream sink
     * \note Use byte access */
    bool                    have_primary_monitored_rate : 8;

    /**
     * Buffer size for the next get_buffer_details.
     * If positive, it is a number of samples; if negative,
     * its magnitude is a time in fractional seconds, to be
     * multiplied with the default sample rate (not a route
     * sample rate).
     */
    int                     buffer_size;

    /** Default global sample rate, currently this is only used to calculate
     * buffer sizes
     */
    unsigned                default_sample_rate;

    /** Keep track of maximum buffer space over a kick period */
    SRC_SYNC_BUFFER_LEVEL_HISTORY   source_buffer_history;

    /** Timer Task ID  */
    tTimerId                kick_id;

    /** External operator ID for traces */
    uint16                  id;

#ifdef SOSY_VERBOSE
    /** Bit mask to enable trace categories */
    uint32                  trace_enable;
#endif /* SOSY_VERBOSE */
#ifdef INSTALL_METADATA
    /** TTP-based rate measurement for reference (primary rate) */
    RATE_MEASURE_METADATA   rm_measure_primary;

#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
    /** The following counters are cleared at the
     * start of each process_data.
     * At the end: num_tags_received + num_tags_allocated
     * == num_tags_deleted + num_tags_sent
     */
    /** Counter for received tags (excluding peeked) */
    unsigned                num_tags_received;
    /** Counter for allocated tags */
    unsigned                num_tags_allocated;
    /** Counter for deleted tags (excluding peeked) */
    unsigned                num_tags_deleted;
    /** Counter for sent tags */
    unsigned                num_tags_sent;
    /** Number of cached tags */
    unsigned                num_tags_cached;
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */
#endif /* INSTALL_METADATA */

    /* Fixups */
    unsigned                reserved;

} SRC_SYNC_OP_DATA;

/** To break up src_sync_compute_transfer, bundle the
 * state which needs to be kept in this small structure.
 */
typedef struct src_sync_compute_context
{
    /** The working est_latency is signed because some intermediate
     * results may be negative */
    int est_latency_t;

    /** Amount of data (in time) which needs to be written to outputs
     * to avoid possibility of underrun before the next opportunity
     * to write data */
    unsigned min_transfer_t;

    /** Upper limit on transfers based on space, data available */
    unsigned max_transfer_t;

    /* Event flag */
    bool downstream_filled;

} SRC_SYNC_COMP_CONTEXT;

/****************************************************************************
Module Private Data Declarations
*/

extern const opmsg_handler_lookup_table_entry src_sync_opmsg_handler_table[];

/****************************************************************************
Module Private Inline Function Definitions
*/

/* Make casts safer by checking the input pointer type */
static inline SRC_SYNC_SINK_GROUP* cast_sink_group(SRC_SYNC_TERMINAL_GROUP* p)
{
    return (SRC_SYNC_SINK_GROUP*)p;
}
static inline SRC_SYNC_SOURCE_GROUP* cast_source_group(SRC_SYNC_TERMINAL_GROUP* p)
{
    return (SRC_SYNC_SOURCE_GROUP*)p;
}
static inline SRC_SYNC_SINK_ENTRY* cast_sink_entry(SRC_SYNC_TERMINAL_ENTRY* p)
{
    return (SRC_SYNC_SINK_ENTRY*)p;
}
static inline SRC_SYNC_SOURCE_ENTRY* cast_source_entry(SRC_SYNC_TERMINAL_ENTRY* p)
{
    return (SRC_SYNC_SOURCE_ENTRY*)p;
}
/* Wrap pointer member accesses with casts to avoid accidentally
 * cross-casting e.g. from a source entry to a sink group.
 */
static inline SRC_SYNC_SOURCE_GROUP* next_source_group(SRC_SYNC_SOURCE_GROUP* p)
{
    return cast_source_group(p->common.next);
}
static inline SRC_SYNC_SINK_GROUP* next_sink_group(SRC_SYNC_SINK_GROUP* p)
{
    return cast_sink_group(p->common.next);
}
static inline SRC_SYNC_SOURCE_ENTRY* next_source_entry(SRC_SYNC_SOURCE_ENTRY* p)
{
    return cast_source_entry(p->common.next);
}
static inline SRC_SYNC_SINK_ENTRY* next_sink_entry(SRC_SYNC_SINK_ENTRY* p)
{
    return cast_sink_entry(p->common.next);
}
static inline SRC_SYNC_SOURCE_GROUP* source_group_from_entry(SRC_SYNC_SOURCE_ENTRY* p)
{
    return cast_source_group(p->common.group);
}
static inline SRC_SYNC_SOURCE_ENTRY* source_entries_from_group(SRC_SYNC_SOURCE_GROUP* p)
{
    return cast_source_entry(p->common.terminals);
}
static inline SRC_SYNC_SINK_GROUP* sink_group_from_entry(SRC_SYNC_SINK_ENTRY* p)
{
    return cast_sink_group(p->common.group);
}
static inline SRC_SYNC_SINK_ENTRY* sink_entries_from_group(SRC_SYNC_SINK_GROUP* p)
{
    return cast_sink_entry(p->common.terminals);
}

/* Time unit conversions */
/**
 * \param usec Non-negative time in microseconds,
 *             expected (though not checked) to be <1e6
 * \return Time in seconds represented as Q1.N
 */
static inline unsigned src_sync_usec_to_sec_frac(unsigned usec)
{
    /* usec << (DAWTH-1-20) "normalizes" usec <= 1e6 */
    return 2 * frac_mult(usec << (DAWTH-1-20), FRACTIONAL(0.5 / (1.0e6 / (1<<20))));
}

/**
 * \param t Non-negative time in seconds represented as Q1.N
 * \return Time in microseconds
 */
static inline unsigned src_sync_time_to_usec(unsigned t)
{
    return (unsigned)frac_mult(t, SECOND);
}

/**
 * \param n Number of samples
 * \param ts Inverse sample rate i.e. time per sample as a
 *          multiple of 2^-SRC_SYNC_INV_RATE_RESOLUTION seconds,
 *          represented as Q1.N
 * \return Time in seconds represented as Q1.N
 * \note Implemented in assembler for 24bit targets
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm unsigned src_sync_samples_to_time(unsigned n, unsigned ts)
{
    @[
      .restrict n:bank1
      .restrict ts:bank1
      .restrict :bank1
      .change rMAC
      ]
    rMAC = @{ts} * @{n} (SU);
    rMAC = rMAC ASHIFT -11 (56bit);
    @{} = rMAC0;
}
#else /* KCC */
static inline unsigned src_sync_samples_to_time(unsigned n, unsigned ts)
{
    int64 c = (int64)n * (int)ts;
    return (unsigned) (c >> SRC_SYNC_INV_RATE_RESOLUTION);
}
#endif /* KCC */

static inline unsigned src_sync_samples_to_usec(unsigned num_samples, unsigned inv_sample_rate)
{
#if DAWTH==24
    /* For now there is no intention to use this, but it should build and be plausible. */
    return src_sync_time_to_usec(src_sync_samples_to_time(num_samples, inv_sample_rate));
#else
    /* To obtain result = 1e6 * num_samples / sample_rate in 32 bit:
     * frac_mult(inv_sample_rate, sample_rate) = 2^10
     * frac_mult(inv_sample_rate, num_samples) = 2^10 * num_samples/sample_rate
     * frac_mult(inv_sample_rate, num_samples * 1e6/2^10) = 1e6 * num_samples / sample_rate = result
     * frac_mult(inv_sample_rate, num_samples * 2^10 * 1e6/2^20) = result
     * FRACTIONAL(1e6/2^20) = round(1e6/2^20*2^31) = 1e6*2^11 = 2048000000
     * This is valid for num_samples < 2^21 which tag lengths or available/space never exceed.
     */
    return frac_mult(inv_sample_rate, frac_mult(num_samples << 10, (SECOND << (DAWTH-1-20))));
#endif
}

/**
 * Convert sample rate in Hz to sample period, fractional x 2^-10 s
 * \param sample_rate in Hz
 * \return sample period, fractional x 2^-10 s
 */
static inline unsigned src_sync_sample_rate_to_inv_sample_rate(unsigned sample_rate)
{
    return pl_fractional_divide( 1<<SRC_SYNC_INV_RATE_RESOLUTION, sample_rate);
}

/* For logging */
static inline unsigned src_sync_sec_frac_to_10usec(unsigned t)
{
    return frac_mult(100000, t);
}

#ifndef SOSY_VERBOSE
/* This function is not inline when verbose logging is enabled */
static inline void src_sync_set_sink_state( SRC_SYNC_OP_DATA *op_extra_data,
                              SRC_SYNC_SINK_GROUP* sink_grp,
                              src_sync_sink_state new_state )
{
    sink_grp->stall_state = new_state;
}
#endif /* SOSY_VERBOSE */

/* Parameter conversions from kick-period-relative to fractional seconds */

static inline unsigned src_sync_get_period(const SRC_SYNC_OP_DATA *op_extra_data)
{
    return src_sync_usec_to_sec_frac(frac_mult(op_extra_data->cur_params.OFFSET_SS_PERIOD, (unsigned)stream_if_get_system_kick_period()));
}

static inline unsigned src_sync_get_max_period(const SRC_SYNC_OP_DATA *op_extra_data)
{
    return src_sync_usec_to_sec_frac(frac_mult(op_extra_data->cur_params.OFFSET_SS_MAX_PERIOD, (unsigned)stream_if_get_system_kick_period())) << 5;
}

static inline unsigned src_sync_get_max_latency(const SRC_SYNC_OP_DATA *op_extra_data)
{
    return src_sync_usec_to_sec_frac(frac_mult(op_extra_data->cur_params.OFFSET_SS_MAX_LATENCY, (unsigned)stream_if_get_system_kick_period())) << 5;
}

static inline unsigned src_sync_get_stall_recovery_default_fill(const SRC_SYNC_OP_DATA *op_extra_data)
{
    return src_sync_usec_to_sec_frac(frac_mult(op_extra_data->cur_params.OFFSET_SS_STALL_RECOVERY_DEFAULT_FILL, (unsigned)stream_if_get_system_kick_period())) << 5;
}


/****************************************************************************
Module Private Function Declarations
*/

extern void src_sync_transfer_route(SRC_SYNC_SOURCE_ENTRY* src_ptr,
    tCbuffer* sink_buffer, unsigned transfer);

extern void src_sync_update_processing(OPERATOR_DATA *op_data);
extern SRC_SYNC_TERMINAL_GROUP* src_sync_find_group(
        SRC_SYNC_TERMINAL_GROUP* groups, unsigned channel_num);

extern SRC_SYNC_SINK_ENTRY *src_sync_alloc_sink(
        SRC_SYNC_OP_DATA *op_extra_data,unsigned term_idx);
extern SRC_SYNC_SOURCE_ENTRY *src_sync_alloc_source(
        SRC_SYNC_OP_DATA *op_extra_data,unsigned term_idx);
extern bool src_sync_alloc_buffer_histories(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_groups,
        SRC_SYNC_SOURCE_GROUP* source_groups);

#ifdef SOSY_VERBOSE
extern void src_sync_set_sink_state(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp,
        src_sync_sink_state new_state );
#endif /* SOSY_VERBOSE */

/* source_sync_utils.c */
extern bool src_sync_alloc_buffer_history(
        SRC_SYNC_BUFFER_LEVEL_HISTORY* hist, unsigned num_entries);
extern void src_sync_free_buffer_history(
        SRC_SYNC_BUFFER_LEVEL_HISTORY* hist);
extern void src_sync_put_buffer_history(
        SRC_SYNC_BUFFER_LEVEL_HISTORY* hist, TIME t, unsigned amount);
extern unsigned src_sync_get_buffer_history_min(
        SRC_SYNC_BUFFER_LEVEL_HISTORY* hist, TIME now, TIME max_age);
extern unsigned src_sync_get_buffer_history_max(
        SRC_SYNC_BUFFER_LEVEL_HISTORY* hist, TIME now, TIME max_age);
extern char src_sync_sign_and_magnitude(int value, unsigned* magnitude);

/* source_sync_opmsg.c */
extern void src_sync_trace_params(const SRC_SYNC_OP_DATA* op_extra_data);

#if defined(INSTALL_METADATA)
/* source_sync_metadata.c */
extern unsigned src_sync_metadata_count_tags(const metadata_tag* tags);
extern metadata_tag* src_sync_metadata_find_last_tag(metadata_tag* tag);
extern bool src_sync_metadata_move_eof(
        metadata_tag *dest_tag, metadata_tag *src_tag);
extern unsigned src_sync_get_next_timestamp(
        TIME timestamp, unsigned nr_of_samples,
        unsigned sample_rate, int sp_adjust);
extern unsigned src_sync_get_prev_timestamp(
        const metadata_tag* tag, unsigned nr_of_samples,
        unsigned sample_rate );
#ifdef SOSY_CHECK_METADATA_TRANSPORT_POINTERS
extern void src_sync_check_md_transport_pre(SRC_SYNC_OP_DATA* op_extra_data);
extern void src_sync_check_md_transport_post(SRC_SYNC_OP_DATA* op_extra_data);
#endif /* SOSY_CHECK_METADATA_TRANSPORT_POINTERS */
#ifdef SOSY_CHECK_METADATA_TAG_COUNTS
extern void src_sync_clear_tag_counts(SRC_SYNC_OP_DATA* op_extra_data);
extern void src_sync_check_tag_counts(SRC_SYNC_OP_DATA* op_extra_data);
#endif /* SOSY_CHECK_METADATA_TAG_COUNTS */


/* source_sync_metadata_rcv.c */
extern bool src_sync_get_input_metadata(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp,
        SRC_SYNC_SINK_METADATA* md, tCbuffer* input_metadata_buffer,
        unsigned* remove_octets );
extern unsigned src_sync_get_sink_metadata(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp,
        unsigned words );

extern src_sync_stall_recovery_type src_sync_peek_resume(
        SRC_SYNC_OP_DATA* op_extra_data, metadata_tag* tag,
        SRC_SYNC_SINK_METADATA* ts_state, unsigned rcv_beforeidx_octets,
        unsigned available_samples, unsigned sample_rate,
        unsigned* p_gap_samples, unsigned sink_grp_idx );

/* source_sync_metadata_send.c */
extern void src_sync_metadata_drop_tags(
        SRC_SYNC_OP_DATA* op_extra_data,
        metadata_tag* tags,
        SRC_SYNC_METADATA_DEST* dest_state );
extern void src_sync_metadata_silence(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_TERMINAL_GROUP* dest_common,
        SRC_SYNC_METADATA_DEST* dest_state, unsigned silence_octets );
extern void src_sync_metadata_forward(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_SINK_METADATA* sink,
        unsigned octets_length, SRC_SYNC_TERMINAL_GROUP* dest_common,
        SRC_SYNC_METADATA_DEST* dest_state );

#endif /* INSTALL_METADATA */

/* source_sync.c */
/* Message handlers */
extern bool src_sync_create(OPERATOR_DATA *op_data, void *message_data,
                            unsigned *response_id, void **response_data);
extern bool src_sync_destroy(OPERATOR_DATA *op_data, void *message_data,
                             unsigned *response_id, void **response_data);
extern bool src_sync_start(OPERATOR_DATA *op_data, void *message_data,
                           unsigned *response_id, void **response_data);
extern bool src_sync_reset(OPERATOR_DATA *op_data, void *message_data,
                           unsigned *response_id, void **response_data);
extern bool src_sync_stop(OPERATOR_DATA *op_data, void *message_data,
                          unsigned *response_id, void **response_data);
extern bool src_sync_connect(OPERATOR_DATA *op_data, void *message_data,
                             unsigned *response_id, void **response_data);
extern bool src_sync_disconnect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool src_sync_buffer_details(
        OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data);
extern bool src_sync_get_sched_info(
        OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data);
extern void src_sync_cleanup(SRC_SYNC_OP_DATA *op_extra_data);
extern bool src_sync_stop_reset(OPERATOR_DATA *op_data,void **response_data);

/* Synchronisation */
extern void src_sync_suspend_processing(SRC_SYNC_OP_DATA *op_extra_data);
extern void src_sync_resume_processing(SRC_SYNC_OP_DATA *op_extra_data);

/* Data processing function */
extern void src_sync_set_sink_recovering_restarting(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP *sink_grp);
extern void src_sync_refresh_sink_list(SRC_SYNC_OP_DATA *op_extra_data);
extern void src_sync_refresh_source_list(SRC_SYNC_OP_DATA *op_extra_data);
extern void src_sync_refresh_connections(SRC_SYNC_OP_DATA* op_extra_data);
extern void src_sync_refresh_forward_routes(SRC_SYNC_OP_DATA* op_extra_data);
#ifdef INSTALL_METADATA
extern void src_sync_refresh_metadata_routes(SRC_SYNC_OP_DATA* op_extra_data);
#endif
extern bool src_sync_perform_transitions(SRC_SYNC_OP_DATA* op_extra_data);
extern unsigned src_sync_calc_sink_group_available_data(SRC_SYNC_SINK_GROUP* sink_grp);
extern void src_sync_compute_space(
    SRC_SYNC_OP_DATA* op_extra_data, unsigned* p_min_src_space_t,
    unsigned* p_max_src_space_t);
extern int src_sync_compute_transfer_space(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_COMP_CONTEXT* comp);
extern int src_sync_compute_transfer_sinks(
        SRC_SYNC_OP_DATA* op_extra_data, SRC_SYNC_COMP_CONTEXT* comp);
extern unsigned src_sync_perform_transfer(SRC_SYNC_OP_DATA *op_extra_data);
extern void src_sync_timer_task(void *kick_object);
extern void src_sync_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);


extern void src_sync_free_buffer_histories(SRC_SYNC_OP_DATA *op_extra_data);

extern SRC_SYNC_SINK_GROUP* src_sync_find_sink_group(
        SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num);
extern SRC_SYNC_SOURCE_GROUP* src_sync_find_source_group(
        SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num);

extern bool src_sync_valid_route(const SRC_SYNC_ROUTE_ENTRY* route);

#ifdef INSTALL_METADATA
extern bool src_sync_connect_metadata_buffer( SRC_SYNC_TERMINAL_ENTRY* entry,
                                              SRC_SYNC_TERMINAL_GROUP* group);
extern void src_sync_find_alternate_metadata_buffer(
        unsigned connected,
        SRC_SYNC_TERMINAL_ENTRY* entry,
        SRC_SYNC_TERMINAL_ENTRY** all_entries);
#endif /* INSTALL_METADATA */

extern unsigned src_sync_route_write_silence( SRC_SYNC_OP_DATA *op_extra_data,
                                              SRC_SYNC_SINK_GROUP *sink_grp,
                                              unsigned words );
extern unsigned src_sync_route_copy( SRC_SYNC_OP_DATA *op_extra_data,
                                     SRC_SYNC_SINK_GROUP *sink_grp,
                                     unsigned words );
#ifdef INSTALL_METADATA
extern unsigned src_sync_route_discard_input( SRC_SYNC_OP_DATA *op_extra_data,
                                              SRC_SYNC_SINK_GROUP *sink_grp,
                                              unsigned words );
#endif /* INSTALL_METADATA */

/* Op msg handlers */
extern bool src_sync_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool src_sync_opmsg_ep_get_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_ep_configure(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_opmsg_ep_clock_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool src_sync_set_route(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_get_route(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_set_sink_groups(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_set_source_groups(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool src_sync_set_trace_enable(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool src_sync_ups_params(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);

/* Helpers for set_sink_groups, set_source_groups, set_route */
extern bool src_sync_disjoint_union_group_masks(
        const unsigned* data_ptr, unsigned num_groups,
        unsigned* p_union_mask );
extern bool src_sync_parse_sink_flags(
        SRC_SYNC_TERMINAL_GROUP* grp_ptr, unsigned x_msw);
extern bool src_sync_parse_source_flags(
        SRC_SYNC_TERMINAL_GROUP* grp_ptr, unsigned x_msw);
extern bool src_sync_populate_groups(
        SRC_SYNC_TERMINAL_GROUP* groups, const unsigned* data_ptr,
        unsigned num_groups,
        bool (*parse_flags)(SRC_SYNC_TERMINAL_GROUP*,unsigned) );
extern void src_sync_clear_group_sample_rate(SRC_SYNC_TERMINAL_GROUP* groups);
extern bool src_sync_mismatched_group_sample_rate(
        SRC_SYNC_TERMINAL_GROUP* groups, unsigned channel_idx,
        unsigned sample_rate);
extern unsigned src_sync_get_num_groups(const SRC_SYNC_TERMINAL_GROUP*);
extern void src_sync_clear_all_routes(SRC_SYNC_OP_DATA *op_extra_data);

/* From source_sync_rate_adjust.c */
#ifdef INSTALL_METADATA
extern void src_sync_ra_set_rate(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp,
        unsigned rate);
extern void src_sync_ra_set_primary_rate(
        SRC_SYNC_OP_DATA *op_extra_data, unsigned rate);
extern void src_sync_ra_update_delay(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp);
#endif /* INSTALL_METADATA */
extern bool src_sync_is_sink_rm_enabled(
        const SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num);
extern void src_sync_check_primary_clock_connected(
        SRC_SYNC_OP_DATA *op_extra_data);
extern bool src_sync_rm_enact(
        SRC_SYNC_OP_DATA *op_extra_data, bool is_sink, unsigned terminal_num,
        bool enable);
extern bool src_sync_rm_adjust(
        SRC_SYNC_OP_DATA *op_extra_data, bool is_sink, unsigned terminal_num,
        uint32 value);
extern tCbuffer* src_sync_get_input_buffer(
        SRC_SYNC_SINK_GROUP* sink_grp, SRC_SYNC_SINK_ENTRY* sink_ptr);
#ifdef INSTALL_METADATA
extern tCbuffer* src_sync_get_input_metadata_buffer(
        SRC_SYNC_SINK_GROUP* sink_grp);
#endif /* INSTALL_METADATA */
extern bool src_sync_rm_init(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp);
extern bool src_sync_rm_fini(
        SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp);
extern void src_sync_rm_process( SRC_SYNC_OP_DATA *op_extra_data, unsigned *back_kick);

/****************************************************************************
Module Private Data Declarations
*/
#if defined(SOSY_VERBOSE) && defined(SOSY_NUMBERED_LOG_MESSAGES)
extern unsigned src_sync_trace_serial;
#ifdef SOSY_LOG_MESSAGE_LIMIT
extern unsigned src_sync_trace_limit;
#endif
#endif /* DEBUG_NUMBERED_LOG_MESSAGES */

#endif /* SOURCE_SYNC_DEFS_H */



