/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Header with definitions for streaming data buffers.
*/

#ifndef BUFFER_MSG_H
#define BUFFER_MSG_H

#include "buffer/buffer.h"

/* Use #ifdef to allow this to be overridden in config file */
#ifndef BUFFER_MSG_RING_SIZE
/**
 * Size of message ring used.
 *
 * Note that one entry must remain empty:  otherwise there
 * is an ambiguity in the case that \c front == \c behind whether
 * the ring is full or empty.
 *
 * Powers of two are recommended as the compiler optimizes modular
 * arithmetic in this case.
 */
#define BUFFER_MSG_RING_SIZE     (16)
#endif

struct BUFFER_MESSAGE_META_FNS;

/**
 * Message buffer structure
 * This encapsulates a standard \c BUFFER implementation.
 */
typedef struct BUFFER_MSG_ss {
    /**
     * Instance of a standard (non-message based) buffer that this type
     * extends.
     */
    BUFFER buf;
    /**
     * The entry before the first valid entry in the message ring.
     * When this is advanced the entry becomes valid.
     */
    uint16 front;
    /**
     * The last entry that has not been despatched by the downstream
     * end.  If \c front == \c back there are no messages to despatch.
     */
    uint16 back;
    /**
     * The last entry for which the data memory has not been freed.
     * If \c back == \c behind then there are no despatched messages
     * waiting to be freed and if \c front == \c back == \c behind
     * then the message ring is empty.
     */
    uint16 behind;

    /**
     * The number of bytes of the message at back that have been consumed
     * by upstream protocol modules. This is used to adjust the returned 
     * length and pointers when the back message is accessed.
     */
    uint16 back_header_bytes_consumed;

    /** 
     * Array of message lengths indexed by front/back/behind
     */
    uint16 msg_lengths[BUFFER_MSG_RING_SIZE];

    /**
     * Pointer to meta data array that contains BUFFER_MSG_RING_SIZE elements
     * of a type specific to the buffer specialisation.
     */
    void * meta;

    /**
     * Optional pointer to a table of functions for deallocating metadata in
     * cases where the provenance of the metadata memory is unknown to the
     * message handling code (e.g. bc_buf_adapter).
     */
    const struct BUFFER_MSG_META_FNS *meta_fns;
} BUFFER_MSG;

/**
 * "Virtual function" table for custom metadata handling
 */
typedef struct BUFFER_MSG_META_FNS
{
    /** Create meta-data for all messages
     *
     * \param b message aware buffer
     * \param meta_data_length fixed meta-data length
     */
    void (* create)(BUFFER_MSG *b, uint16 meta_data_length);

    /** Add a meta-data entry
     *
     * \param b message aware buffer
     * \param meta_length length of meta-data entry to add
     * \param meta_data pointer to meta-data entry to add
     */
    void (* add_entry)(BUFFER_MSG *b, uint16 meta_data_length,
                        const void *meta_data);

    /** Find the meta data entry for the first message
     *
     * \param b The buffer to search meta data
     * \param len Location to store meta data length
     * \return A pointer to the meta data, or NULL if there is no data.
     * If len is not NULL, then *len is set to the number of bytes
     * of meta data available. The returned pointer is valid until
     * the owning message is cleared.
     */
    void* (* peek) (BUFFER_MSG *b, uint16 *len);

    /** Free a meta-data entry
     *
     * \param b message aware buffer
     * \param pos index of the meta-data entry to be freed
     */
    void (* free_entry)(BUFFER_MSG *b, uint16f pos);

    /** Free meta-data for all messages
     *
     * \param b message aware buffer
     */
    void (* destroy)(BUFFER_MSG *b);

} BUFFER_MSG_META_FNS;


/** Add a message to the buffer. This updates front and index.
 */
void buf_add_to_front(BUFFER_MSG *buf, uint16 len);

/** Add a message to the buffer with associated meta data
 * \param buf Pointer to the BUFFER_MSG structure
 * \param len Length of data to be added (should already be in the mmu)
 * \param meta_type Type of the metadata this buffer uses
 * \param meta_data Pointer to the meta data for this message
 */
#define BUF_ADD_TO_FRONT_META(buf, len, meta_type, meta_data)   \
    do {                                                        \
        *BUF_GET_FRONT_META((buf), meta_type) = *(meta_data);\
        buf_add_to_front((buf), (len));                         \
    } while(0)

/**
 * Get a pointer to the meta data for an entry in the message ring.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param entry Entry number in the message ring
 * \param meta_type Type of the metadata this buffer uses
 */
#define BUF_GET_ENTRY_META(buf, entry, meta_type)                \
                                    (((meta_type *)(buf)->meta) + (entry))

/**
 * Get a pointer to the meta data for an entry in the message ring.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param entry Entry number in the message ring
 * \param meta_size_bytes Size of the metadata this buffer uses
 */
#define BUF_GET_ENTRY_GENERIC_META(buf, entry, meta_size_bytes)             \
                            (((uint8 *)(buf)->meta) + meta_size_bytes *     \
                            ((entry) % BUFFER_MSG_RING_SIZE))

/**
 * Get a pointer to the meta data for the next message to be added to the
 * message buffer.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param meta_type Type of the metadata this buffer uses
 */
#define BUF_GET_FRONT_META(buf, meta_type)                    \
                                    (((meta_type *)(buf)->meta) + (buf)->front)

/**
 * Get a pointer to the meta data for the next message to be added to the
 * message buffer. This is for generic meta data where the type isn't known.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param meta_size_bytes Size of the metadata this buffer uses
 */
#define BUF_GET_FRONT_GENERIC_META(buf, meta_size_bytes)                    \
                    (((uint8 *)(buf)->meta) + meta_size_bytes * (buf)->front)

/**
 * Get a pointer to the meta data for the next message to be consumed from the
 * message buffer.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param meta_type Type of the metadata this buffer uses
 */
#define BUF_GET_BACK_MSG_META(buf, meta_type)                   \
                                    (((meta_type *)(buf)->meta) + (buf)->back)

/**
 * Get a pointer to the meta data for the next message to be consumed from the
 * message buffer. This is for generic meta data where the type isn't known.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param meta_size_bytes Size of the metadata this buffer uses
 */
#define BUF_GET_BACK_GENERIC_MSG_META(buf, meta_size_bytes)                \
                    (((uint8 *)(buf)->meta) + meta_size_bytes * (buf)->back)

/**
 * Get a pointer to the meta data for the next message to be freed from the
 * message buffer.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param meta_type Type of the metadata this buffer uses
 */
#define BUF_GET_BEHIND_MSG_META(buf, meta_type)                 \
                                    (((meta_type *)(buf)->meta) + (buf)->behind)

/**
 * Get a pointer to the meta data for the next message to be freed from the
 * message buffer. This is for generic meta data where the type isn't known.
 * \param buf Pointer to the BUFFER_MSG structure
 * \param meta_size_bytes Size of the metadata this buffer uses
 */
#define BUF_GET_BEHIND_MSG_GENERIC_META(buf, meta_size_bytes)   \
                    (((uint8 *)(buf)->meta) + meta_size_bytes * (buf)->behind)

/** Fully consumes the message being read, but leaves outdex alone.
 * This is for cases where we consume the message in fragments, so
 * that when we come to throw it away entirely outdex is already
 * where it should be.
 */
void buf_update_back_only(BUFFER_MSG *msg_buf);

/** Fully consumes the message being read. This updates back and outdex.
 */
void buf_update_back(BUFFER_MSG *buf);

/**
 * If possible, undo the last update of back (and outdex).  It's possible
 * if the old back is still ahead of (or equal to) behind.
 */
bool buf_unupdate_back(BUFFER_MSG *buf);

/** Free up a message from the buffer. 
 * This updates behind and tail and frees the mmu pages.
 */
void buf_update_behind_free(BUFFER_MSG *buf);

/** Free up a message from the buffer without freeing mmu pages (allows it 
 * to be overwritten). 
 * This updates behind and tail.
 */
void buf_update_behind(BUFFER_MSG *buf);



/** Advance an index round the ring */
#define BUF_NEXT_MSG(ind)       (uint16)(((ind) + 1) % BUFFER_MSG_RING_SIZE)
#define BUF_PREV_MSG(ind)       (uint16)(((ind) + BUFFER_MSG_RING_SIZE - 1) % BUFFER_MSG_RING_SIZE)


/** Cheap macro to test for any messages to send
 */
#define BUF_ANY_MSGS_TO_SEND(buf)           ((buf)->front != (buf)->back)

/** Cheap macro to indicate number of messages to send
 */
#define BUF_NUM_MSGS_TO_SEND(buf) \
  ((((buf)->front - (buf)->back) + BUFFER_MSG_RING_SIZE) % BUFFER_MSG_RING_SIZE)

/** Cheap macro to test for any messages sent but not freed
 */
#define BUF_ANY_MSGS_SENT(buf)                  ((buf)->back != (buf)->behind)


/** Cheapish macro to test for any messages available
 *
 *  This takes account of the fact that the \c front is not allowed
 *  to advance into the \c behind.
 */
#define BUF_ANY_MSGS_AVAILABLE(buf)                                     \
                        ((buf)->behind != BUF_NEXT_MSG((buf)->front))


/** Cheapish macro to count number of messages available
 *
 *  This takes account of the fact that the \c front is not allowed
 *  to advance into the \c behind.
 */
#define BUF_NUM_MSGS_AVAILABLE(buf)                                     \
    ((((buf)->behind - ((buf)->front + 1)) + BUFFER_MSG_RING_SIZE) % \
                                                          BUFFER_MSG_RING_SIZE)

/** Cheapish macro to count the number of messages that are in use
 */
#define BUF_NUM_MSGS_USED(buf) \
    ((((buf)->front - (buf)->behind) + BUFFER_MSG_RING_SIZE) % \
                                                          BUFFER_MSG_RING_SIZE)

/** Map in the buffer for writing the current message (at front)
 * This should be followed by a call to \c buf_add_to_front() to inform
 * downstream modules of the data.
 */
#define buf_map_front_msg(msg_buf)                                      \
                        (buf_raw_write_only_map_8bit(&(msg_buf)->buf))

/** Get the length of the message at the back of a message ring.
 *
 * The downstream end uses this to retrieve metadata about data
 * for onward transmission (updating outdex).
 */
#define buf_get_back_msg_len(buf)   (uint16)((buf)->msg_lengths[(buf)->back]  \
                                        - (buf)->back_header_bytes_consumed)

/** Map in the buffer for reading the current message (at back)
 * The downstream protocol modules use this to read the message with the
 * headers of the upstream modules removed.
 */
#define buf_map_back_msg(msg_buf)   (buf_raw_read_map_8bit(&(msg_buf)->buf) \
                                        + (msg_buf)->back_header_bytes_consumed)

/**
 * Map in the buffer for reading and modifying the current message (at back)
 */
#define buf_map_back_msg_rd_wr(msg_buf)   \
    (buf_raw_write_map_8bit_position(&(msg_buf)->buf, (msg_buf)->buf.outdex) \
                                    + (msg_buf)->back_header_bytes_consumed)

/** Consumes header bytes from the message currently being read (at back). 
 */
#define buf_consume_header_bytes(msg_buf, octets)                       \
                            ((msg_buf)->back_header_bytes_consumed =    \
                    (uint16)((msg_buf)->back_header_bytes_consumed + (octets)))


/** Get the length of the message at the behind of a message ring.
 *
 * The downstream end uses this to retrieve metadata about data
 * for buffer freeing (updating tail).
 */
#define buf_get_behind_msg_len(buf)     ((buf)->msg_lengths[(buf)->behind])


/**
 * Function to register that a message buffer will be having entries freed
 * or referenced by a pointer into the mapped memory. This is needed if
 * calls to \c buf_free_entries_from_ptr() need to be used.
 * \param buf Message buffer that needs to be referenced by data pointer
 */
void buf_register_msg_buf_for_ptr_access(BUFFER_MSG * buf);

/**
 * Function to unregister a message buffer that was previously registered
 * by a call to \c buf_register_msg_buf_for_ptr_access().
 * \param buf Message buffer that was previously registered
 */
void buf_unregister_msg_buf_for_ptr_access(BUFFER_MSG * buf);

/**
 * Function to free buffer message entries and pages given a pointer
 * that lies within one of the entries. Updates \c behind and \c free
 * in the message_buf structure. Asserts if the pointer does not match
 * any BUFFER_MSG registered by \c buf_register_msg_buf_for_ptr_access().
 * \param ptr Pointer that points into a previously registered message buffer
 */
void buf_free_entries_from_ptr(void * ptr);

/**
 * Combine the two packets at adjacent message ring entries into a single
 * entry with length equal to the combined length and meta data equal to
 * the first in the message ring. This is only for use on packets that are
 * between back and behind entries not including the one at back.
 * Entries prior to the one specified up to the behind entry are
 * shuffled along the message ring by one to close the gap that this
 * combining produces. The behind pointer is then advanced.
 * \param msg_buf Message buffer with entries to combine
 * \param entry The message ring entry to be combined with the following one.
 * \param meta_size_bytes The size of meta data entries for this buffer
 * \return The combined length if the entries were combined or zero
 * if the combining couldn't be done.
 */
uint16 buf_combine_entries(BUFFER_MSG * msg_buf, uint16 entry,
                                                        uint16 meta_size_bytes);

/**
 * Combine the two packets at adjacent message ring entries into a single
 * entry. Identical to \c buf_combine_entries() but specifying the entry
 * by the offset of the data in the buffer.
 * \param msg_buf Message buffer with entries to combine
 * \param offset Offset of the start of the data in the buffer which
 * specifies the message ring entry to be combined with the following one.
 * \param meta_size_bytes The size of meta data entries for this buffer
 * \return The combined length if the entries were combined or zero
 * if the combining couldn't be done.
 */
uint16 buf_combine_entries_at_offset(BUFFER_MSG * msg_buf, uint16 offset,
                                                        uint16 meta_size_bytes);

#endif /* BUFFER_MSG_H */
