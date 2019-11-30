/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  buffer_metadata.h
 *
 * \ingroup buffer
 *
 * Public C header file for buffer metadata
 *
 */

#ifndef BUFFER_METADATA_H
#define BUFFER_METADATA_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "cbuffer_c.h"

#ifdef INSTALL_METADATA

/****************************************************************************
Public Constant Declarations
*/

/* Define this to enable additional checks in buff_metadata_append()
 * and buff_metadata_remove(). Define it to 2 to include a slightly
 * CPU-intensive check */
/* #define METADATA_DEBUG_TRANSPORT */

/* Define this to fault when METADATA_DEBUG_TRANSPORT checks find
 * misuse of the metadata transport API. Otherwise the caller address
 * is just logged.
 */
/* #define METADATA_DEBUG_TRANSPORT_FAULT */

/****************************************************************************
Public Type Declarations
*/

/* Timestamp formats */
typedef enum
{
    METADATA_TIMESTAMP_NONE = 0,
    METADATA_TIMESTAMP_LOCAL = 1
} METADATA_TIMESTAMP;

/* Flag bit definitions and access macros */
#define METADATA_STREAM_START_SHIFT       0
#define METADATA_STREAM_START_MASK        (1 << METADATA_STREAM_START_SHIFT)
#define METADATA_STREAM_START_SET(x)      ((x->flags) |= METADATA_STREAM_START_MASK)
#define METADATA_STREAM_START_UNSET(x)    ((x->flags) &= ~METADATA_STREAM_START_MASK)
#define METADATA_STREAM_START(x)          ((x->flags) & METADATA_STREAM_START_MASK)

#define METADATA_STREAM_END_SHIFT         1
#define METADATA_STREAM_END_MASK          (1 << METADATA_STREAM_END_SHIFT)
#define METADATA_STREAM_END_SET(x)        ((x->flags) |= METADATA_STREAM_END_MASK)
#define METADATA_STREAM_END_UNSET(x)      ((x->flags) &= ~METADATA_STREAM_END_MASK)
#define METADATA_STREAM_END(x)            ((x->flags) & METADATA_STREAM_END_MASK)

#define METADATA_PACKET_START_SHIFT       2
#define METADATA_PACKET_START_MASK        (1 << METADATA_PACKET_START_SHIFT)
#define METADATA_PACKET_START_SET(x)      ((x->flags) |= METADATA_PACKET_START_MASK)
#define METADATA_PACKET_START_UNSET(x)    ((x->flags) &= ~METADATA_PACKET_START_MASK)
#define METADATA_PACKET_START(x)          ((x->flags) & METADATA_PACKET_START_MASK)

#define METADATA_PACKET_END_SHIFT         3
#define METADATA_PACKET_END_MASK          (1 << METADATA_PACKET_END_SHIFT)
#define METADATA_PACKET_END_SET(x)        ((x->flags) |= METADATA_PACKET_END_MASK)
#define METADATA_PACKET_END_UNSET(x)      ((x->flags) &= ~METADATA_PACKET_END_MASK)
#define METADATA_PACKET_END(x)            ((x->flags) & METADATA_PACKET_END_MASK)

/* Reserving 2 bits for timestamp flags in case we want to add more formats */
#define METADATA_TIMESTAMP_SHIFT          4
#define METADATA_TIMESTAMP_MASK           (3 << METADATA_TIMESTAMP_SHIFT)
#define METADATA_TIMESTAMP_SET(x, t, f)   \
    do \
    { \
        (((x)->flags) |= ((f) << METADATA_TIMESTAMP_SHIFT)); \
        (((x)->timestamp) = (t)); \
    } \
    while (0)

#define IS_TIMESTAMPED_TAG(x) \
    ((((x)->flags & METADATA_TIMESTAMP_MASK) != 0) && \
     (((x)->flags & METADATA_TIME_OF_ARRIVAL_MASK) == 0))

/* Time of arrival tag */
#define METADATA_TIME_OF_ARRIVAL_SHIFT    6
#define METADATA_TIME_OF_ARRIVAL_MASK     (1 << METADATA_TIME_OF_ARRIVAL_SHIFT)
#define METADATA_TIME_OF_ARRIVAL_SET(x,t) \
    do \
    { \
        (((x)->flags) |= METADATA_TIME_OF_ARRIVAL_MASK); \
        (((x)->timestamp) = (t)); \
    } \
    while(0)

#define METADATA_TIME_OF_ARRIVAL_UNSET(x) (((x)->flags) &= ~METADATA_TIME_OF_ARRIVAL_MASK)
#define IS_TIME_OF_ARRIVAL_TAG(x) \
    (((x)->flags & (METADATA_TIMESTAMP_MASK | METADATA_TIME_OF_ARRIVAL_MASK)) \
     == METADATA_TIME_OF_ARRIVAL_MASK)

/* Normally timestamp and time of arrival flags are mutually exclusive because
 * both use the timestamp field. A "Void TTP" tag is indicated by both
 *   METADATA_TIMESTAMP_SET(x, 0, METADATA_TIMESTAMP_LOCAL)
 *   METADATA_TIME_OF_ARRIVAL_SET(x, 0)
 * I.e. the flag fields for both tag types are set, and the time field is zero.
 */
#define METADATA_VOID_TTP_SET(x) \
    do \
    { \
        (((x)->flags) |= (METADATA_TIMESTAMP_LOCAL << METADATA_TIMESTAMP_SHIFT) \
                         | METADATA_TIME_OF_ARRIVAL_MASK); \
        (((x)->timestamp) = 0); \
    } \
    while(0)

#define IS_VOID_TTP_TAG(x) \
    ( \
        (((x)->flags & (METADATA_TIMESTAMP_MASK | METADATA_TIME_OF_ARRIVAL_MASK)) \
         == ( (METADATA_TIMESTAMP_LOCAL << METADATA_TIMESTAMP_SHIFT) | \
              METADATA_TIME_OF_ARRIVAL_MASK ) ) && \
        ((x)->timestamp == 0) \
    )

#define IS_EMPTY_TAG(x) \
    ( \
        ( ( ((x)->flags & ~METADATA_PACKET_START_MASK) & ~METADATA_PACKET_END_MASK) == 0) \
    )

/* Accessor macros for buffer descriptor */
#define BUFF_METADATA_SET(x)              BUF_DESC_METADATA_SET(x->descriptor)
#define BUFF_METADATA_UNSET(x)            BUF_DESC_METADATA_UNSET(x->descriptor)
#define BUFF_METADATA(x)                  BUF_DESC_METADATA(x->descriptor)

/* Each metadata tag has an optional pointer to a private data array
 * consisting of a variable number of items
 *
 * The first word is the number of items
 * Each item has a key to identify it and a variable length data field
 * The length is specified in address units, but each item is always word-aligned
 */

/* Private data keys */
typedef enum
{
    META_PRIV_KEY_TTP_OFFSET = 1,
    META_PRIV_KEY_EOF_CALLBACK,
    META_PRIV_KEY_USER_DATA
} METADATA_PRIV_KEY;

#ifdef METADATA_DEBUG_TRANSPORT
typedef enum
{
    METADBG_TRANSPORT_STARTING = 0,
    METADBG_TRANSPORT_NORMAL   = 1,
    METADBG_TRANSPORT_EMPTY    = 2
} METADATA_DEBUG_TRANSPORT_STATE;
#endif

/* Private data item structure */
typedef struct
{
    METADATA_PRIV_KEY key:8;    /**< identifying key */
    unsigned length:16;         /**< Length of element in address units */
    unsigned data[];            /**< Variable-length array of data */
} metadata_priv_item;

/* Private data overall structure */
typedef struct
{
    unsigned item_count;        /**< number of items */
    unsigned items[];           /**< variable-length list of data items */
} metadata_priv_data;

/**
 * Please keep sizeof(metadata_tag_struct) a multiple of
 * a word. This so that the tag can easily stream in and
 * out of KIP metadata cbuffers from one audio processor
 * to the next, for dual-core / multi-core cases.
 */
struct metadata_tag_struct
{
    metadata_tag *next; /**< next tag linked */
    unsigned flags;     /**< flags associated with the metadata tag */
    unsigned index;     /**< offset of the start of data in the cbuffer (in octets)*/
    unsigned length;    /**< size of the fragment (in octets) */
    unsigned timestamp; /**< TTP timestamp */
    int sp_adjust;      /**< TTP Sample period adjustment, fractional difference from nominal */
    metadata_priv_data *xdata;    /**< private data array (see above) */
};

/* End-of-file callback function / data */
typedef void (*eof_callback)(unsigned data);

typedef struct metadata_eof_callback_ref
{
    unsigned ref_count_local;
    unsigned ref_count_remote;

    eof_callback callback;              /**< callback function */
    unsigned data;                      /**< data to pass to callback */
} metadata_eof_callback_ref;

typedef struct metadata_eof_callback
{
    metadata_eof_callback_ref *ref;     /**< local reference for tracking */

#ifdef INSTALL_DUAL_CORE_SUPPORT
    /** These bits are only relevant for tags that cross cores */
    unsigned proc_id; /* Originating core */
    void* parent; /* Pointer to the metadata_eof_callback_ref on the originating
                   * core. This is stored so that we can restore ref field if the
                   * tag goes back to the originating core */
    bool last_remote_copy;
#endif /* INSTALL_DUAL_CORE_SUPPORT */

} metadata_eof_callback;

/****************************************************************************
Private Type Declarations
*/
/** FIFO associated with a buffer transporting metadata.
 *
 */
struct metadata_list_struct
{
    struct {
        metadata_tag *head;     /**< head of a list of tags */
        metadata_tag *tail;     /**< tail of a list of tags */
    } tags;

    unsigned prev_wr_index; /**< previous write index, appending a tag */
    unsigned prev_rd_index; /**< previous read index, removing a tag */

    metadata_list *next;    /**< next linked metadata maintained as a circular linked list */

    unsigned ref_cnt:6;       /**< number of buffers sharing this metadata */
    unsigned usable_octets:2; /**< number of usable octets in a word (0 means all) */
    unsigned buffer_size:16;  /**< size of the buffers using this metadata, in octets */

#ifdef METADATA_DEBUG_TRANSPORT
    unsigned next_tag_index;
    unsigned last_tag_still_covers;
    METADATA_DEBUG_TRANSPORT_STATE metadbg_transport_state;
#endif /* METADATA_DEBUG_TRANSPORT */

};

/****************************************************************************
Public Variable Definitions
*/


/****************************************************************************
Public Function Declarations
*/

/**
 * Initialise the buffer metadata system.
 *
 * \param count Maximum metadata tags (system-wide) to support.
 */
extern void buff_metadata_init(unsigned count);

/**
 * Create a new (empty) metadata tag
 *
 * \return Pointer to new tag, or NULL in case of failure.
 */
extern metadata_tag *buff_metadata_new_tag(void);

/**
 * Check whether there are lots of metadata tags allocated
 *
 * \return TRUE if threshold exceeded
 */
extern bool buff_metadata_tag_threshold_exceeded(void);

/**
 * Release a metadata tag that's no longer in use back to the store.
 *
 * \param tag The metadata tag to return to the store.
 * \param process_eof Whether or not to process any end-of-file information
 * associated with this tag.
 *
 * \note Most callers should set the process_eof parameter to TRUE, unless
 * the tag is getting replaced by a new one carrying the same EOF information.
 */
extern void buff_metadata_delete_tag(metadata_tag *tag, bool process_eof);

/**
 * \brief Helper function that deletes a list of metadata tags that is no
 * longer required.
 * \param list A linked list of metadata tags to release
 */
extern void buff_metadata_tag_list_delete(metadata_tag *list);

/**
 * \brief Make a copy of an existing metadata tag
 *
 * \param tag The tag to make a copy of
 * \return The copy of the tag that was created
 */
extern metadata_tag *buff_metadata_copy_tag(metadata_tag *tag);


extern unsigned priv_data_length(metadata_tag *tag);

/**
 * \brief Add private data to a metadata tag
 *
 * \param tag The tag to add to
 * \param key Numerical identifier for the data
 * \param length Data size (in address units)
 * \param data Pointer to data
 *
 * \return Pointer to data field in tag, or NULL if it couldn't be added
 *
 * \note The data pointer is allowed to be NULL, to allow the data to be
 * allocated and populated separately. If it is non-NULL it will be copied
 * to the newly-added entry.
 *
 */
extern void* buff_metadata_add_private_data(metadata_tag *tag, unsigned key, unsigned length, void *data);

/**
 * \brief Find private data in a metadata tag
 *
 * \param tag The tag to look in
 * \param key Numerical identifier for the data
 * \param out_length Pointer to data size (in address units)
 * \param out_data Pointer to data
 *
 * \return TRUE if found, FALSE if not
 *
 * \note The data is left unchanged.
 */
extern bool buff_metadata_find_private_data(metadata_tag *tag, unsigned key, unsigned *out_length, void **out_data);

/**
 * \brief Add EOF callback struct to a metadata tag
 *
 * \param tag The tag to add to
 * \param callback Function to call when the EOF-tagged data is played
 * \param eof_data Parameter for the callback function
 */
extern bool buff_metadata_add_eof_callback(metadata_tag *tag, eof_callback callback, unsigned eof_data);

/**
 * \brief Retrieve EOF callback struct from a metadata tag
 *
 * \param tag The tag to look in
 * \param cb_struct pointer to callback structure
 * containing function pointer and data parameter
 */
extern bool buff_metadata_get_eof_callback(metadata_tag *tag, metadata_eof_callback *cb_struct);


/**
 * \brief Attach metadata tag to buffer
 *
 * \param buff Pointer to buffer struct to attach metadata to
 * \param tag Pointer to (populated) metadata tag to attach (index will be recalculated)
 * \param octets_pre_written Number of octets that have been written to buffer since last append
 * \param octets_post_written Number of octets yet to be written to buffer past the current index
 * \return TRUE if successful, or FALSE in case of failure.
 */
#ifdef METADATA_DEBUG_TRANSPORT
#define buff_metadata_append(B,T,P,A) buff_metadata_append_dbg((B),(T),(P),(A),0)
extern bool buff_metadata_append_dbg(tCbuffer *buff, metadata_tag *tag, unsigned octets_pre_written, unsigned octets_post_written, unsigned caller_addr);
#else /* METADATA_DEBUG_TRANSPORT */
extern bool buff_metadata_append(tCbuffer *buff, metadata_tag *tag, unsigned octets_pre_written, unsigned octets_post_written);
#endif /* METADATA_DEBUG_TRANSPORT */

/**
 * Sets the usable octets per word. The metadata tags will only index the usable octets
 * after this call.
 *
 * \param buff Pointer to buffer
 * \param usable_octets usable octets per word.
 * \note This will also set the cached buffer size.
 * \note It is the caller's responsibility to make sure the buffer is
 * configured WITH metadata (will panic otherwise).
 */
extern void buff_metadata_set_usable_octets(tCbuffer *buff, unsigned usable_octets);

/**
 * Gets the usable octets per word.
 *
 * \param buff Pointer to buffer
 * \return usable_octets usable octets per word.
 *
 * \note It is the caller's responsibility to make sure the buffer is
 * configured WITH metadata (return unpredictable otherwise).
 */
extern unsigned int buff_metadata_get_usable_octets(tCbuffer *buff);

/**
 * Gets the buffer size (as cached by metadata submodule).
 *
 * \param buff Pointer to buffer structure
 * \return buffer size in octets
 *
 * \note. The returned buffer size is the value cached in the metadata structure,
 *  might be different from the value stored in parent cbuffer structure
 *  (that one is in memory allocation units anyway).
 *  The cached value takes into account the usable_octets,
 *  it is possible that not all octets in a word are usable.
 *
 * \note. To set the cached buffer size use buff_metadata_set_usable_octets.
 */
extern unsigned int buff_metadata_get_buffer_size(tCbuffer *buff);

/**
 * Look at first metadata tag attached to buffer
 *
 * The tag is NOT removed from the buffer.
 *
 * \param buff Pointer to buffer
 *
 * \return pointer to first attached tag, or NULL if none present.
 */

/**
 * Look at first metadata tag attached to buffer without removing the tag from
 * the buffer.
 *
 * \param  buff          Pointer to buffer
 * \return               Pointer to first attached tag
 *
 * \note    If returned_copy is TRUE, the caller is expected to delete the tag
 *          once it's done with it.
 */
extern metadata_tag *buff_metadata_peek(tCbuffer *buff);

/**
 * Look at first metadata tag attached to buffer
 *
 * The tag is NOT removed from the buffer.
 *
 * \param buff Pointer to buffer
 * \param *octets_b4idx Pointer to number of octets not yet processed before the index of the removed tag.
 *
 * \return pointer to first attached tag, or NULL if none present.
 */
extern metadata_tag *buff_metadata_peek_ex(tCbuffer *buff, unsigned *octets_b4idx);

/**
 * Returns the available octets in the metadata.
 *
 * \param buff Pointer to buffer
 *
 * \return available octets in the metadata
 */
extern unsigned buff_metadata_available_octets(tCbuffer *buff);

/**
 * Returns the available space according to the metadata.
 *
 * \param buff Pointer to buffer
 *
 * \return available space in the metadata
 */
extern unsigned buff_metadata_available_space(tCbuffer *buff);

/**
 * Remove the first metadata tag from a buffer
 *
 * \param buff Pointer to buffer
 * \param octets_consumed Number of octets consumed from the data buffer
 *
 * \return Pointer to first removed tag, or NULL if none present or none removed
 * \return *octets_b4idx Pointer to number of octets not yet processed before the index of the removed tag.
 * \return *octets_afteridx Pointer to number of octets already processed after the index of the removed tag.
 */
#ifdef METADATA_DEBUG_TRANSPORT
#define buff_metadata_remove(B,C,P,A) buff_metadata_remove_dbg((B),(C),(P),(A),0)
extern metadata_tag *buff_metadata_remove_dbg(tCbuffer *buff, unsigned octets_consumed, unsigned *octets_b4idx, unsigned *octets_afteridx, unsigned caller_addr);
#else /* METADATA_DEBUG_TRANSPORT */
extern metadata_tag *buff_metadata_remove(tCbuffer *buff, unsigned octets_consumed, unsigned *octets_b4idx, unsigned *octets_afteridx);
#endif /* METADATA_DEBUG_TRANSPORT */

/**
 * Enable use of metadata on a buffer
 *
 * \param buff Pointer to buffer
 */
extern void buff_metadata_enable(tCbuffer *buff);


/**
 * Returns the read offset of the buffer metadata.
 *
 * \param buff Pointer to buffer
 * \return returns the metadata read offest.
 */
extern unsigned buff_metadata_get_read_offset(tCbuffer *buff);

/**
 * Returns the write offset of the buffer metadata.
 *
 * \param buff Pointer to buffer
 * \return returns the metadata write offest.
 */
extern unsigned buff_metadata_get_write_offset(tCbuffer *buff);

/**
 * Returns the head tag of the buffer metadata.
 *
 * \param buff Pointer to buffer
 * \return returns the metadata head.
 */
extern metadata_tag* buff_metadata_get_head(tCbuffer *buff);

/**
 * \brief Sets up metadata structure for a new connection. Metadata is
 * rationalised between existing connections that can share. Multiple channels
 * between the same entity share the same metadata structure, which is reference
 * counted. If the two sides are attempting to share different metadata instances
 * with the same connection then metadata will be ignored on this connection.
 * Where the source can share but the output differs then a new linked metadata
 * instances is created.
 *
 * \param buff The buffer for the connection to setup metadata for.
 * \param src_metadata The buffer provided by the source of the connection that has the same metadata.
 * \param sink_metadata The buffer provided by the sink of the connection that has the same metadata.
 */
extern bool buff_metadata_connect(tCbuffer *buff, tCbuffer *src_metadata, tCbuffer *sink_metadata);
/**
 * \brief Releases the metadata structure that is associated with the provided
 * buffer. The buffer itself is not freed.
 *
 * \param buff The buffer that no longer wants metadata
 */
extern void buff_metadata_release(tCbuffer *buff);

/**
 * Transport metadata for operators that have an opinion about how much data they want to process.
 * \brief Move all meta tags from src->metadata to dst->metadata list.
 *       All entries that reference data in src until the position of the read pointer
 *       (not including the read pointer) will have to be consumed.
 *
 * \param src source data cbuffer
 * \param dst destination data cbuffer
 * \param tran_octets number of octets to transport
 *
 * \return the last tag transported across (could be NULL if none transported).
 */
extern metadata_tag *metadata_strict_transport( tCbuffer *src, tCbuffer *dst, unsigned tran_octets);



/**
 * \brief Helper function to identify if a buffer is configured with metadata support.
 *
 * \param buff Pointer to the buffer to test for metadata support. Pointer must be a valid buffer.
 *
 * \return TRUE if the buffer supports metadata. FALSE if it doesn't.
 */
static inline bool buff_has_metadata(tCbuffer *buff)
{
    if (BUFF_METADATA(buff))
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * \brief Helper function for adding a constant delay to the metadata appended
 * to a buffer.
 *
 * \Note This function is typically called at operator start
 *
 * \param buff The buffer to delay the metadata of
 * \param delay_octets The number of octets of delay to apply to the metadata
 * appended to the buffer.
 */
extern void buff_metadata_add_delay(tCbuffer *buff, unsigned delay_octets);

/**
 * \brief Helper function for removing a constant delay to metadata appended
 * to a buffer.
 *
 * \Note This function is typically called at operator stop/reset
 *
 * \param buff The buffer to remove the metadata delay from
 * \param delay_octets The number of octets of delay to remove from the metadata
 * appended to the buffer.
 */
extern void buff_metadata_remove_delay(tCbuffer *buff, unsigned delay_octets);

/**
 * \brief Helper function for aligning a buffer's metadata with the buffer's
 * current read & write pointer positions
 * NOTE: This function only works if the underlying buffer is a purely software buffer
 *
 * \param buff The buffer to align the metadata of
 */
extern void buff_metadata_align_to_buff_ptrs(tCbuffer *buff);

/**
 * \brief Helper function for aligning a buffer's metadata with the buffer's
 * current read pointer position
 * NOTE: This function only works if the underlying buffer is a purely software buffer
 *
 * \param buff The buffer to align the metadata of
 */
extern void buff_metadata_align_to_buff_read_ptr(tCbuffer *buff);

/**
 * \brief Helper function for aligning a buffer's metadata with the buffer's
 * current write pointer position
 * NOTE:
 *    - This function only works if the underlying buffer is a purely software buffer
 *    - only the owner of write side of the buffer shall use this function
 *    - don't use this function if buffer has added delay to metadata, or fix the delay after using this function
 *
 * \param buff The buffer to align the metadata of
 */
extern void buff_metadata_align_to_buff_write_ptr(tCbuffer *buff);

#ifdef INSTALL_METADATA_DUALCORE

/**
 * \brief Handler for remote core EOF indications
 *
 * \param cb_ref Pointer to the local callback reference data
 */
extern void buff_metadata_eof_remote_cb(metadata_eof_callback_ref *cb_ref);

/**
 * \brief   Pops all available tags from the shared IPC channel so that they can
 *          be moved to the local shadow endpoint. The tags form a linked-list
 *          that are returned through its head (return value) and tail (argument).
 *
 * \param  shared_buffer The IPC channel buffer for moving metadata tags.
 * \param  tail          A place-holder for the function to return the tail tag.
 * \return               Returns the head of a linked-list of metadata tags. The
 *                       tail is also returned for convenience through the tail
 *                       argument.
 */
extern metadata_tag* buff_metadata_pop_tags_from_KIP(tCbuffer* shared_buffer, metadata_tag** tail);

/**
 * \brief   Pushes tags to the KIP buffer. It is possible that the KIP buffer is
 *          full. This function therefore returns the last tag it attempted to push
 *          so that the caller can update the originating linked-list accordingly.
 *
 * @param  shared_buffer The IPC channel buffer for moving metadata tags.
 * @param  first_tag     Pointer to the first tag being pushed to the shared buffer.
 * @return Returns the last tag the function attempted to push to the IPC buffer
 *         so that the caller can update the shadow endpoint linked-list accordingly.
 */
extern metadata_tag* buff_metadata_push_tags_to_KIP(tCbuffer* shared_buffer, metadata_tag* first_tag);

/**
 * \brief  Flushes tags created by KIP transfer from one core to another.
 *         This may be needed in start-up phase of a chain, where a kick
 *         arrives from the other core, while a decoder has already run
 *         and skipped the first few tags by incrementing the buffer index.
 *         his function attempts to re-align the buffer's index and the 
 *         tag index, by removing tags with indexes that preceed the 
 *         buffer index.
 *
 * @param  buffer      The buffer that the tags refer to.
 * @param  head_tag    Pointer to the head tag (buff_metadata_pop_tags_from_KIP).
 * @param  last_tag    Pointer to the last tag (buff_metadata_pop_tags_from_KIP).
 * @return Returns     TRUE if tags are now synchronised (no tags left with an
 *                     index before that of metadata index). FALSE if tags are
 *                     not yet synchronised (last tag has an index before that
 *                     of metadata index).
 */
extern bool buff_metadata_flush_KIP_tags(
               tCbuffer *buffer, metadata_tag **head_tag, metadata_tag **tail_tag);

#endif /* INSTALL_METADATA_DUALCORE */


#ifdef DESKTOP_TEST_BUILD
/**
 * \brief Helper function for unit tests that displays the contents of a metadata_list
 * structure.
 *
 * \param meta The metadata_list structure to display the contents of.
 */
extern void print_metadata(metadata_list *meta);

/**
 * \brief Helper function for unit tests that displays the contents of a metadata_tag
 * structure.
 *
 * \param meta The metadata_tag structure to display the contents of.
 */
extern void print_metadata_tag(metadata_tag *tag);
/**
 * \brief Helper function for unit tests that displays the contents of a metadata_tag
 * private / user data.
 *
 * \param meta The metadata_tag structure to display the data for.
 */
extern void print_metadata_priv(metadata_tag *tag);

#endif /* DESKTOP_TEST_BUILD */

#endif /* INSTALL_METADATA */

#endif /* BUFFER_METADATA_H */
