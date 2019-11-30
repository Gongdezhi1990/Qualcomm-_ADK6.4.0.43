/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup splitter
 * \file  splitter.h
 * \ingroup capabilities
 *
 * Splitter operator public header file. <br>
 *
 */

#ifndef SPLITTER_PRIVATE_H
#define SPLITTER_PRIVATE_H


#include "splitter.h"
#include "buffer.h"
#include "pmalloc/pl_malloc.h"
#include "util.h"
#include "audio_log/audio_log.h"
#include "platform/pl_intrinsics.h"
#include "platform/pl_assert.h"
#include "op_msg_helpers.h"
#include "pl_timers/pl_timers.h"
#include "patch/patch.h"
#include "ttp/ttp.h"
#include "ttp_utilities.h"
#ifdef SPLITTER_OFFLOAD
#include "ipc/ipc_audio_thread.h"
#endif
#ifdef SPLITTER_EXT_BUFFER
#include "ext_buffer/ext_buffer.h"
#include "extmem/extmem.h"
#endif

#ifdef RUNNING_ON_KALSIM
// Use 'new' cbuffer function in the support lib
#include "../support_lib/cbuffer/cbuffer_ex_c_kalsim.h"
#endif

/****************************************************************************
Private Constant Definitions
*/
/** Splitter's minimum metadata tag reframe size in words.  */
#define SPLITTER_MIN_FRAME_SIZE 128

/** Splitter's minimum metadata tags in the internal buffer.  */
#define SPLITTER_MIN_TAGS 8

/** Splitter's minimum internal buffer size to get a reasonable
 * performance.  */
#define SPLITTER_MIN_INTERNAL_BUFFER_SIZE (SPLITTER_MIN_TAGS*SPLITTER_MIN_FRAME_SIZE)

/** Maximum number of splitter channels */
#define SPLITTER_MAX_CHANS 4
/** The maximum number of streams to split the input channel into.
 * Only two outputs are supported! */
#define SPLITTER_MAX_OUTPUTS_PER_CHANNEL 2
/** Maximum number of splitter input channels */
#define SPLITTER_MAX_INPUTS SPLITTER_MAX_CHANS
/** Maximum number of splitter channels */
#define SPLITTER_MAX_OUTPUTS (SPLITTER_MAX_OUTPUTS_PER_CHANNEL * SPLITTER_MAX_CHANS)
/** The block size of the splitter reported to streams */
#define SPLITTER_BLOCK_SIZE 1
/** Value indicating an invalid terminal on the splitter */
#define SPLITTER_INVALID_TERMINAL UINT_MAX

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define SPLITTER_CAP_ID CAP_ID_DOWNLOAD_SPLITTER
#else
#define SPLITTER_CAP_ID CAP_ID_SPLITTER
#endif

/** Splitter self-kick time, as a multiple of system kick period */
#define SPLITTER_SELF_KICK_RATIO 2

/* Maximum time to wait for a buffer access to finish.*/
#define MAX_WAIT_US 20000

#define LAST_TAG_SAMPLES_INVALID    ((unsigned)-1)
#define LAST_TAG_TIMESTAMP_INVALID  ((unsigned)-1)

/* Used by set_buffer_rd_offset */
#define WORD_ALIGNED_MASK                         0xFFFFFFFCUL
#define OFFSET_MASK                               0x3

/** The maximum time the operator will wait for the external memory */
#define EXT_MEM_WAIT 10000
/****************************************************************************
Private Type Definitions
*/
typedef struct internal_metadata_t
{
    /** Buffer holding the internal metadata.  */
    tCbuffer* buffer;

    /* Just as with the read pointers the same applies to the metadata read pointer. */
    unsigned prev_rd_indexes[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

    /* Pointer to the last processed tag for the output. */
    metadata_tag *head_tag[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

}internal_metadata_t;

typedef union data_buffer_t
{
    /* Buffer saved on the DSP memory. */
    tCbuffer* dsp;

#ifdef SPLITTER_EXT_BUFFER
    /* Buffer saved on an sram. */
    EXT_BUFFER* sram;
#endif
}data_buffer_t;

typedef union read_position_t
{
    /* Each output will have a different progress in the buffer.
     * This progress is saved in the form of a read pointer
     * to the internal buffer. */
    int* read_pointers[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

    /* Each output will have a different progress in the buffer.
     * This progress is saved in the form of a read offset
     * to the internal buffer. */
    unsigned read_index[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];
}read_position_t;

/* The buffer which holds the data. */
typedef struct internal_buffer_t
{
    /** Data buffer. */
    data_buffer_t buffer;

    /* Each output will have a different progress in the buffer.
     * This progress is saved in the form of a read offset
     * to the internal buffer. */
    unsigned read_offset[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

    /** Read pointer offset. */
    //read_position_t rd_position;
}internal_buffer_t;

/* The enum holds all the possible output state. */
typedef enum
{
    /* Inactive streams will discard data. */
    INACTIVE = 0,

    /* Active streams will pass data.*/
    ACTIVE,

    /* Streams on hold will buffer data. */
    HOLD,

    NR_OF_STATE,
}SPLITTER_OUTPUT_STATE;

/* Splitter channel descriptor  */
typedef struct splitter_channel_struct
{
   /* Next channel in the list. */
   struct splitter_channel_struct *next;

   /* Channel ID*/
   unsigned id;

   /* Input buffer of the stream. */
   tCbuffer *input_buffer;

   /* Output buffers of the stream */
   tCbuffer *output_buffer[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

   /* internal buffering related data. */
   internal_buffer_t internal;
} SPLITTER_CHANNEL_STRUC;

/**
 * Enum for the splitter workings modes.
 */
typedef enum
{
    /* The input buffer is cloned at the outputs. */
    CLONE_BUFFER = OPMSG_SPLITTER_CLONE_BUFFER,

    /* The data is buffer in a separete buffer. */
    BUFFER_DATA = OPMSG_SPLITTER_BUFFER_DATA,

    /* For sanity checks. */
    NR_OF_MODES
}SPLITTER_MODES;

/**
 * Enum for the splitter buffering locations.
 */
typedef enum
{
    /* The data is buffered in an internal buffer. */
    INTERNAL = OPMSG_SPLITTER_INTERNALLY,

    /* The data is buffer in an external SRAM. */
    SRAM = OPMSG_SPLITTER_SRAM,

    /* For sanity checks. */
    NR_OF_LOCATIONS
}SPLITTER_BUFFER_LOCATION;

/**
 * Enum to indicate if splitter is packing data.
 */
typedef enum
{
    /* Data will not be packed. */
    UNPACKED = OPMSG_SPLITTER_UNPACKED,

    /* Audio data will be packed*/
    PACKED = OPMSG_SPLITTER_PACKED,

    NR_OF_PACKING
}SPLITTER_PACKING;

/**
 * Enum to hold all the possible output stream states.
 * The current implementation only allows two output streams.
 */
typedef enum
{
    OUT_STREAM__0_OFF__1_OFF = 0,
    OUT_STREAM__0_ON___1_OFF,
    OUT_STREAM__0_OFF__1_ON,
    OUT_STREAM__0_ON___1_ON
}SPLITTER_STREAMS;

typedef struct
{
    /** The number of samples consumed */
    unsigned samples_consumed;

    /** The number of samples produced */
    unsigned samples_produced;

    /** The input sample rate */
    uint32 sample_rate;

    /** Timestamp from the last metadata tag processed */
    TIME last_tag_timestamp;

    /** Sample period adjustment value from the last metadata tag */
    unsigned last_tag_spa;

    /** Samples read since the last metadata tag */
    unsigned last_tag_samples;

    /* Error offset ID from last tag */
    unsigned last_tag_err_offset_id;
}SPLITTER_METADATA_REFRAME;

/* A structure containing a set of cbuffer processing function pointers */
typedef struct
{
    /* Cbuffer functiona for the input/output buffers.*/
    unsigned (*unpacked_space)(tCbuffer *cbuffer);
    unsigned (*data)(tCbuffer *cbuffer);

    /* Functions to handle the internal buffer.*/
    unsigned (*packed_space)(tCbuffer *cbuffer);
    unsigned (*pack)(tCbuffer *dst, tCbuffer *src, unsigned to_copy);
    unsigned (*unpack)(tCbuffer *dst, tCbuffer *src, unsigned to_copy);
    void (*discard)(tCbuffer *cbuffer, unsigned to_advance);

#ifdef SPLITTER_EXT_BUFFER
    /* Functions to handle the SRAM buffer.*/
    unsigned (*sram_packed_space)(EXT_BUFFER * eb);
    unsigned (*sram_pack)(EXT_BUFFER *dst, tCbuffer *src, unsigned to_copy);
    unsigned (*sram_unpack)(tCbuffer *dst, EXT_BUFFER *src, unsigned to_copy);
    void (*sram_discard)(EXT_BUFFER *cbuffer, unsigned to_advance);
#endif
    unsigned data_size;
}cbuffer_functions_t;


typedef struct
{
    /** List of splitter channels */
    SPLITTER_CHANNEL_STRUC *channel_list;

    /* Basic internal settings.*/
        /* The working mode of the splitter operator. */
        SPLITTER_MODES working_mode;

        /* Represents the location of the buffered data.
         * This field is ignored when the working mode is
         * CLONE_BUFFER*/
        SPLITTER_BUFFER_LOCATION location;

        /* Represents the location of the buffered data.
         * This field is ignored when the working mode is
         * CLONE_BUFFER or when the data format is different
         * than AUDIO_DATA_FORMAT_FIXP or
         * AUDIO_DATA_FORMAT_FIXP_WITH_METADATA */
        SPLITTER_PACKING packing;

    /** The internal buffer size of the capability. */
    unsigned buffer_size;

    /** The data format to advertise at connect */
    AUDIO_DATA_FORMAT data_format;

    /** The sinks to kick on when kicking backwards */
    unsigned touched_sinks;

    /** The sources to kick on when kicking forwards */
    unsigned touched_sources;


    /* The current output streams states  are saved separately
     * to simplify transitions.*/
        /* Array indicating the current output streams state. */
        SPLITTER_OUTPUT_STATE output_state[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

    /* Bitfields indicating the next output streams state.
     * When these states are different the the current state
     * a transition is needed.*/
        /** Bitfield indicating the next activated output streams. */
        SPLITTER_STREAMS active_streams:SPLITTER_MAX_OUTPUTS_PER_CHANNEL;

        /** Bitfield indicating the next deactivated output streams. */
        SPLITTER_STREAMS inactive_streams:SPLITTER_MAX_OUTPUTS_PER_CHANNEL;

        /** Bitfield indicating the next buffering output streams*/
        SPLITTER_STREAMS hold_streams:SPLITTER_MAX_OUTPUTS_PER_CHANNEL;

    /* Transition indicator. */
        /** If true, the operator din't finish the transition yet. This can happen
         * when transitioning to active, but there is not enough data in the
         * internal buffer. All the other transitions are instant.*/
        bool in_transition;

    /** Buffer that was supplied by buffer_details and should be provided by the
     * connect call. This is mainly stored in case connect fails before splitter_connect()
     * is called and we need to release this memory at a later time. */
    tCbuffer *curr_connecting;

    /** Timer ID used to kick operator to ensure data keeps flowing */
    tTimerId self_kick_timer;

    /* Timestamp where the operator will transition from consume to pass-through  */
    TIME_INTERVAL transition_point;

    /** If the operator reframes the tags the tag length will be equal with
     * the frame size. Furthermore when an output is on hold the operator
     * leaves a frame worth of space free in the internal/external buffer
     * even when reframing is disabled. */
    unsigned frame_size;

    /** Reframe data */
    SPLITTER_METADATA_REFRAME reframe_data;

    /** The amount of data the operator is currently unpacking from the
     * internal buffer.
     * This field is only used when offloading to p1 is enabled. */
    unsigned data_to_unpack[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

    /** Structure holding the internal metadata for all channels and outputs. */
    internal_metadata_t internal_metadata;

    /* Activate this to finish last tag when deactivating an output stream.
     * When an output stream is transitioning to inactive mode the last tag
     * at the output can remain unfinished. When the inactive stream is not
     * connected to another buffer this can cause metadata transport corruption.*/
    bool finish_last_tag:1;

    /** True if splitter reframes the incoming timestamps. */
    bool reframe_enabled:1;

    bool tag_size_to_samples:1;
#ifdef SPLITTER_OFFLOAD
    /** Flag indicating if the internal buffer write and read is
     * offloaded to p1. */
    bool offload_enabled:1;

    /** Flag indicating if operator accesses the internal/SRAM buffer.
     * this is used to avoid threading issues when the operator offloads
     * the buffer access.*/
    bool buffer_access:1;

    /** Flag indicatign the offloding is in use */
    bool offload_activated:1;
#endif

    /** Flag indicating if the operator needs kicking forward.
     * This flag is only used when offloading to p1 is enabled. */
    bool kick_forward:1;

    /** Flag indicating if the operator needs kicking backward.
     * This flag is only used when offloading to p1 is enabled. */
    bool kick_backward:1;

#if defined(SPLITTER_DEBUG)
    /* Debug variables */
    bool internal_buffer_empty:1;
    bool internal_buffer_full:1;
    bool output_buffer_full:1;
#endif
    /* Functions used to get available data and space.
     * Also, contains functions to pack and unpack. */
    cbuffer_functions_t cbuffer;
} SPLITTER_OP_DATA;


typedef struct
{
    unsigned channel_id;
    bool is_input;
    unsigned terminal_num;
    unsigned index;
    tCbuffer *buffer;
}terminal_info_t;


/****************************************************************************
Private Macro Declarations
*/
/**
 * Splitter debug messages.
 */
#if defined(SPLITTER_DEBUG)
#define SPLITTER_DEBUG_INSTR(x)         (x)
#define SPLITTER_MSG(x)                 L1_DBG_MSG(x)
#define SPLITTER_MSG1(x, a)             L1_DBG_MSG1(x, a)
#define SPLITTER_MSG2(x, a, b)          L1_DBG_MSG2(x, a, b)
#define SPLITTER_MSG3(x, a, b, c)       L1_DBG_MSG3(x, a, b, c)
#define SPLITTER_MSG4(x, a, b, c, d)    L1_DBG_MSG4(x, a, b, c, d)
#define SPLITTER_MSG5(x, a, b, c, d, e) L1_DBG_MSG5(x, a, b, c, d, e)
#else
#define SPLITTER_DEBUG_INSTR(x)         ((void)0)
#define SPLITTER_MSG(x)                 ((void)0)
#define SPLITTER_MSG1(x, a)             ((void)0)
#define SPLITTER_MSG2(x, a, b)          ((void)0)
#define SPLITTER_MSG3(x, a, b, c)       ((void)0)
#define SPLITTER_MSG4(x, a, b, c, d)    ((void)0)
#define SPLITTER_MSG5(x, a, b, c, d, e) ((void)0)
#endif /* SPLITTER_DEBUG */

#define SPLITTER_ERRORMSG(x)                 L1_DBG_MSG(x)
#define SPLITTER_ERRORMSG1(x, a)             L1_DBG_MSG1(x, a)
#define SPLITTER_ERRORMSG2(x, a, b)          L1_DBG_MSG2(x, a, b)
#define SPLITTER_ERRORMSG3(x, a, b, c)       L1_DBG_MSG3(x, a, b, c)
#define SPLITTER_ERRORMSG4(x, a, b, c, d)    L1_DBG_MSG4(x, a, b, c, d)
#define SPLITTER_ERRORMSG5(x, a, b, c, d, e) L1_DBG_MSG5(x, a, b, c, d, e)

/****************************************************************************
Private Function Declarations
*/
/********************   splitter.c           ***********************/
bool splitter_transition(SPLITTER_OP_DATA *splitter);

#ifdef INSTALL_CBUFFER_EX
/********************   splitter_buffer_func.asm *******************/
void cbuffer_copy_packed_to_audio_shift(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
void cbuffer_copy_audio_shift_to_packed(tCbuffer * dst, tCbuffer *src, unsigned num_octets);

/********************   splitter_buffer_func.c *********************/
unsigned un_pack_audio(tCbuffer * dst, tCbuffer *src, unsigned num_samples);
unsigned pack_audio(tCbuffer * dst, tCbuffer *src, unsigned num_samples);
void advance_packed(tCbuffer *cbuffer, unsigned num_samples);
unsigned packed_space(tCbuffer *cbuffer);
#ifdef SPLITTER_EXT_BUFFER
void pack_ext_buffer_discard_data(EXT_BUFFER * eb, unsigned int num_samples);
void audio_ext_buffer_discard_data(EXT_BUFFER * eb, unsigned int num_samples);
unsigned int pack_ext_buffer_amount_space(EXT_BUFFER * eb);
unsigned int audio_ext_buffer_amount_space(EXT_BUFFER * eb);
unsigned int audio_ext_buffer_circ_read(tCbuffer * dest, EXT_BUFFER * ext_src, unsigned int num_samples);
unsigned int audio_ext_buffer_circ_write(EXT_BUFFER * ext_dest, tCbuffer * src, unsigned int num_samples);
#endif
#endif

/********************   splitter_metadata.c  ***********************/
#ifdef INSTALL_METADATA
bool create_internal_metadata(SPLITTER_OP_DATA *splitter, data_buffer_t* internal_buff, unsigned buffer_size);
void delete_internal_metadata(SPLITTER_OP_DATA *splitter);
tCbuffer* get_metadata_buffer(SPLITTER_OP_DATA *splitter, bool is_input, unsigned index);
void splitter_metadata_transport_to_internal(SPLITTER_OP_DATA *splitter,  unsigned trans_octets);
void splitter_metadata_copy(SPLITTER_OP_DATA *splitter, unsigned* data_to_copy, unsigned data_to_remove);
void remove_metadata_from_internal(SPLITTER_OP_DATA *splitter, unsigned data_to_remove);
#if defined(SPLITTER_DEBUG)
    void check_metadata_tags_length(SPLITTER_OP_DATA *splitter, tCbuffer *metadata_buffer);
#endif
#endif

/********************   splitter_helper.c    ***********************/
void splitter_timer_task(void *timer_data);
bool get_terminal_info(unsigned terminal_id, tCbuffer *buffer, terminal_info_t *terminal_info, bool read_buffer);
void set_cbuffer_functions(SPLITTER_OP_DATA *splitter);
SPLITTER_CHANNEL_STRUC *get_channel_struct(SPLITTER_OP_DATA *splitter, unsigned channel_id);
void delete_disconnected_channel(SPLITTER_OP_DATA *splitter);
#if defined(SPLITTER_DEBUG)
    void check_buffers_validity(SPLITTER_OP_DATA *splitter);
#endif

/****************************************************************************
Private Inline Function Definitions
*/

/**
 * Check the next output state is valid.
 */
static inline bool check_next_output_state_validity(SPLITTER_OP_DATA *splitter)
{
    /* Check if flags for the output stream indexes are only set for state only. */
    return (
            splitter->active_streams ^ splitter->inactive_streams ^ splitter->hold_streams
           ) == (
            (1<<SPLITTER_MAX_OUTPUTS_PER_CHANNEL) - 1
           );
}

/**
 * Returns the next state for the give output stream index.
 */
static inline SPLITTER_OUTPUT_STATE get_next_output_state(SPLITTER_OP_DATA *splitter, unsigned index)
{
    unsigned channel_mask;
    SPLITTER_DEBUG_INSTR(PL_ASSERT(check_next_output_state_validity(splitter)));
    SPLITTER_DEBUG_INSTR(PL_ASSERT(index < SPLITTER_MAX_OUTPUTS_PER_CHANNEL));

    channel_mask = 1<<index;
    if (splitter->active_streams & channel_mask)
    {
        return ACTIVE;
    }
    else if (splitter->inactive_streams & channel_mask)
    {
        return INACTIVE;
    }
    else if (splitter->hold_streams & channel_mask)
    {
        return HOLD;
    }
    else
    {
        /* This cannot happen, but stop the compiler complaining.*/
        return INACTIVE;
    }
}

/**
 * Returns the current state for the give output stream index.
 */
static inline SPLITTER_OUTPUT_STATE get_current_output_state(SPLITTER_OP_DATA *splitter, unsigned index)
{
    SPLITTER_DEBUG_INSTR(PL_ASSERT(index < SPLITTER_MAX_OUTPUTS_PER_CHANNEL));

    return splitter->output_state[index];
}

/**
 * Sets the current state for the give output stream index.
 */
static inline void set_current_output_state(SPLITTER_OP_DATA *splitter, unsigned index, SPLITTER_OUTPUT_STATE output_state)
{
    SPLITTER_DEBUG_INSTR(PL_ASSERT(index < SPLITTER_MAX_OUTPUTS_PER_CHANNEL));

    splitter->output_state[index] = output_state;
}



/**
 * Helper function to get the metadata buffer for the internal buffer.
 */
static inline tCbuffer* get_internal_metadata_buffer(SPLITTER_OP_DATA *splitter)
{
    /* Couldn't find anything. */
    return splitter->internal_metadata.buffer;
}


/**
 * Returns the splitter instance from an operator data.
 */
static inline SPLITTER_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (SPLITTER_OP_DATA *) base_op_get_instance_data(op_data);
}

/**
 * Checks if the bitfield received is within range.
 */
static inline bool invalid_stream_setting(unsigned stream_bitfield)
{
    return (stream_bitfield > (OPMSG_SPLITTER_STREAM_0 | OPMSG_SPLITTER_STREAM_1));
}


#ifdef SPLITTER_OFFLOAD
/**
 * Functions which waits for the process in the second core to finish.
 */
static inline bool wait_for_buffer_access_end(SPLITTER_OP_DATA *splitter)
{
    /* Wait for the buffer access to finish*/
    TIME time = hal_get_time();
    TIME time_exp = time_add(time, MAX_WAIT_US);
    while (splitter->buffer_access)
    {
        if(time_lt(time_exp, hal_get_time()))
        {
            SPLITTER_ERRORMSG("Splitter timed out while waiting for buffer access to finish!");
            return FALSE;
        }
    }

    return TRUE;
}
#endif

/********************   splitter internal data buffer handlers   ***********************/
#ifdef INSTALL_CBUFFER_EX
/**
 * Returns the read offset of the internal buffer.
 */
static inline unsigned get_buffer_rd_offset(SPLITTER_OP_DATA *splitter, data_buffer_t *buffer)
{

    if (splitter->location == INTERNAL)
    {
        unsigned int offset, ret_val;
        tCbuffer *cbuffer = buffer->dsp;
        unsigned int base_addr = (unsigned int)(uintptr_t)cbuffer->base_addr;

        ret_val = (unsigned int)(uintptr_t)cbuffer_get_read_address_ex(cbuffer, &offset) - base_addr;
        ret_val += offset;
        return ret_val;
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        return ext_buffer_get_read_offset(buffer->sram);
#else
        PL_ASSERT(splitter->location == INTERNAL);
        /* Stop the compiler complaining */
        return 0;
#endif
    }
}

/**
 * Sets the read offset of the internal buffer.
 */
static inline void set_buffer_rd_offset(SPLITTER_OP_DATA *splitter, data_buffer_t *buffer, unsigned offset)
{

    if (splitter->location == INTERNAL)
    {
        tCbuffer *cbuffer = buffer->dsp;
        unsigned int base_addr = (unsigned int)(uintptr_t)cbuffer->base_addr;

        cbuffer_set_read_address_ex(cbuffer,
                                    (unsigned int *)((base_addr + offset) & WORD_ALIGNED_MASK),
                                    (offset) & OFFSET_MASK);
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        /* Update the read index for the output.*/
        ext_buffer_set_read_offset(buffer->sram, offset);
#else
        PL_ASSERT(splitter->location == INTERNAL);
#endif
    }
}
#endif

/**
 * Returns the opposite output stream index.
 */
static inline unsigned opposite_output_index(unsigned output_index)
{
    return (output_index + 1) & 1;
}


/**
 * Converts metadata read index to buffer offset.
 */
static inline unsigned metadata_read_idx_to_buffer_offset(SPLITTER_OP_DATA *splitter, unsigned position)
{
    unsigned read_offset = -1;
    /* convert the index to read offset.*/
    if (splitter->tag_size_to_samples)
    {
        if (splitter->packing == PACKED)
        {
            read_offset = position<<1;
        }
        else
        {
            read_offset = position<<2;
        }
    }
    else if (splitter->cbuffer.data_size  == 1)
    {
        read_offset = position;
    }

    SPLITTER_DEBUG_INSTR(PL_ASSERT(read_offset != -1));

    return read_offset;
}

#ifdef INSTALL_METADATA
/**
 * Returns the space in the metadat buffer.
 */
static inline unsigned get_interal_metadata_space(SPLITTER_OP_DATA *splitter)
{
    unsigned metadata_space_correction = 0;
    tCbuffer *metadata_buffer = get_internal_metadata_buffer(splitter);

    /* Cbuffer leaves a word free to avoid overlaping read and write
     * pointers while the metadata works with offset. To avoid
     * differences the meadata space needs correction.*/
    if (splitter->tag_size_to_samples)
    {
        if (splitter->packing == PACKED)
        {
            metadata_space_correction = 1;
        }
        else
        {
            metadata_space_correction = 0;
        }
    }
    else if (splitter->cbuffer.data_size  == 1)
    {
        metadata_space_correction = 3;
    }

    return (buff_metadata_available_space(metadata_buffer) - metadata_space_correction);
}
#endif

#if defined(SPLITTER_DEBUG)
/**
 * Returns the available space in the internal buffer.
 */
static unsigned inline internal_buffer_space(SPLITTER_OP_DATA *splitter, data_buffer_t *buffer)
{
    if (splitter->location == INTERNAL)
    {
        return splitter->cbuffer.packed_space(buffer->dsp);
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        return splitter->cbuffer.sram_packed_space(buffer->sram);
#else
        PL_ASSERT(splitter->location == INTERNAL);
        /* Avoid compiler warning. */
        return 0;
#endif
    }
}
/**
 * Function checks if the amount of space is correct.
 */
static void inline check_data_space(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel, unsigned space, unsigned output_index)
{
    unsigned buffer_space;
    buffer_space = internal_buffer_space(splitter, &channel->internal.buffer);

    if (space != buffer_space)
    {
        SPLITTER_OUTPUT_STATE state = get_current_output_state(splitter, output_index);
        SPLITTER_MSG4("Splitter check_data_space output index %d state %d (0 inactive, 1 active, 2 hold)\n"
                "  meta space 0x%08x  data space 0x%08x ",
                output_index, state, space, buffer_space);
    }
    PL_ASSERT(space == buffer_space);
}
#endif

/** Converts words to octets */
static inline unsigned words_to_octets(unsigned val)
{
    /* KCC is smart so it will shift.*/
    return val * ADDR_PER_WORD;
}

/** Converts samples to octets */
static inline unsigned samples_to_octets(unsigned val)
{
    /* KCC is smart so it will shift.*/
    return val * OCTETS_PER_SAMPLE;
}

/** Converts octets to samples. The operation rounds down. */
static inline unsigned octets_to_samples(unsigned val)
{
    /* KCC is smart so it will shift.*/
    return val / OCTETS_PER_SAMPLE;
}

#endif /* SPLITTER_PRIVATE_H */
