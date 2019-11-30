/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup mixer
 * \file  mixer_struct.h
 * \ingroup capabilities
 *
 * Mixer operator structure shared header file. <br>
 *
 */

#ifndef MIXER_STRUCT_H
#define MIXER_STRUCT_H

/*****************************************************************************
Include Files
*/
#include "buffer/cbuffer_c.h"

#ifdef MIXER_SUPPORTS_STALLS
#include "pl_timers/pl_timers.h"
#endif


/****************************************************************************
Private Constant Definitions
*/

/** Default number of streams */
#define MIXER_DEFAULT_NUMBER_STREAM 3
/** Default number of channels per stream */
#define MIXER_DEFAULT_CHANNEL_PER_STREAM 4
/** Maximum number of mixer input channels (12) */
#define MIXER_MAX_INPUT_CHANS             MIXER_DEFAULT_CHANNEL_PER_STREAM * MIXER_DEFAULT_NUMBER_STREAM
/** Maximum number channels per stream*/
#define MIXER_MAX_INPUT_CHANS_PER_STREAM 6
/** Maximum number of mixer output channels */
#define MIXER_MAX_OUTPUT_CHANS            MIXER_MAX_INPUT_CHANS_PER_STREAM

/** Maximum number of input channels for the channel mixer */
#define CHANNEL_MIXER_MAX_INPUT_CHANS       8
/** Maximum number of output channels for the channel mixer */
#define CHANNEL_MIXER_MAX_OUTPUT_CHANS      8

/* Generic mixer */
#define GEN_MIXER_MAX_CHANNELS     24

/****************************************************************************
Private Type Definitions
*/
/* Gain parameter structure */
typedef struct gen_mixer_gain_def
{
   unsigned    src_terminal;
   unsigned    sink_mask;
   unsigned    gain;
   unsigned    single_source;
}GEN_MIXER_GAIN_DEF;

/* Stream definition structure */
typedef struct gen_mixer_stream
{
   unsigned sink_mask;                /** bitwise set of sink making up group */
#ifdef INSTALL_METADATA
   tCbuffer *metadata_ip_buffer;       /** The input buffer with metadata to transport from */
#endif
#ifdef MIXER_SUPPORTS_STALLS
   unsigned stall_mask;               /** bitwise set of stalled sinks */
#endif
}GEN_MIXER_STREAM;

/* Sink structure */
typedef struct gen_mixer_sink_info
{
   tCbuffer *buffer;                   /* Sink cBuffer */
   struct gen_mixer_sink_info *next;   /* Linked list of active sinks */

   unsigned *base_addr;                /* Cached cBuffer info. */
   unsigned  length;
   unsigned *read_addr;
#ifdef MIXER_SUPPORTS_STALLS
   unsigned  sink_mask;                /* Sink Terminal Mask */
   GEN_MIXER_STREAM *lpgroup;          /* Pointer to stream group */
#endif
}GEN_MIXER_SINK_INFO;

/* Sink Mix structure */
typedef struct gen_mixer_mix_info
{
   unsigned current_gain;              /* Current Gain being applied */
   unsigned target_gain;               /* Gain after transition */
   int      gain_adjust;               /* gain change for transition (per sample) */
   unsigned mix_function;              /* Function to perform mix operation 1,2, or 3 */
   GEN_MIXER_SINK_INFO *sink;          /* pointer to sinks being mixed*/

   struct gen_mixer_mix_info  *next;   /* Next active mix */
}GEN_MIXER_MIX_INFO;

/* Source structure */
typedef struct gen_mixer_source_info
{
   tCbuffer *buffer;                    /* Source cBuffer */
   struct gen_mixer_source_info *next;  /* Linked list of active sources */

   unsigned  configured_sinks;          /* Bitwise mask of configured sinks */

   GEN_MIXER_MIX_INFO *mix_list;
   GEN_MIXER_MIX_INFO  mixes[];         /* Array of configured sinks for mix  */
}GEN_MIXER_SOURCE_INFO;

/* multichannel mixer structure */
typedef struct GEN_MIXER_OP_DATA
{
   unsigned transition_count;       /** Counter for gain transition */
   unsigned samples_to_ramp;
   unsigned inv_samples_to_ramp;
    
   bool  reset_mixers;          /** Signals connectivity change */
   bool  reset_gains;           /** Signals gain change */
   bool  restart_transition;    /** Signals Reset transition*/

   tCbuffer *source_wait_buffer;    /** Previous operation waiting on space */
   tCbuffer *sink_wait_buffer;      /** Previous operation waiting on data  */
    
   unsigned              num_streams;
   GEN_MIXER_STREAM     *streams;                  /** array of streams */

   unsigned                 connected_sources;      /* bit-mask of connected sources */
   unsigned                 max_sources;
   GEN_MIXER_SOURCE_INFO   *source_list;
   GEN_MIXER_SOURCE_INFO  **sources;

   unsigned                 connected_sinks;        /* bit-mask of connected sinks   */

   unsigned                 max_sinks;
   GEN_MIXER_SINK_INFO     *sink_list;
   GEN_MIXER_SINK_INFO     *sinks;

   GEN_MIXER_SINK_INFO     zero_sink;
   
   unsigned                 active_sinks;           /* bit-mask of connected sinks 
                                                       that are complete groups      */
   unsigned                 sample_rate;
   unsigned                 mixer_changed;
   unsigned                 output_buffer_size;

#ifdef MIXER_SUPPORTS_STALLS
   tTimerId                 kick_id;                /**< Timer Task ID  */

   unsigned                 inv_sample_rate;
   unsigned                 time_stamp;
   unsigned                 min_data;               /* Minimum number of samples */
   unsigned                 max_space;              /* Size of output buffers */
   unsigned                 est_latency;            /* Estimated downstream latency in usec */
   unsigned                 max_latency;            /* Maximum latency in usec */
   unsigned                 min_latency;            /* Minimum latency required before triggering stall */
   unsigned                 not_all_stalled;        /* Flag indicating at least one stream is not stalled */
#endif  

#ifdef INSTALL_METADATA
    /** The index of metadata_ip_buffer, where metadata is transfered from */
    unsigned              sinks_with_metadata;     /** bitwise mask of sinks with  metadata            */
    unsigned              sink_group_masters;      /** bitwise mask of sinks providing metadata        */
    unsigned 			  metadata_stream_idx;     /** Sink stream to copy metadata from */ 
    /** Indicator of whether metadata propagated to the output comes follows the primary 
     * input stream configuration*/
    bool 				  metadata_tracks_primary;
    unsigned 			  primary_stream;
    unsigned               source_with_metadata;
    unsigned               source_providing_metadata;
    tCbuffer             *metadata_op_buffer;      /** The output buffer with metadata to transport to */
#endif /* INSTALL_METADATA */
} GEN_MIXER_OP_DATA;

#endif /* MIXER_STRUCT_H */
