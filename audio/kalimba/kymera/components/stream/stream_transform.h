/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file stream_transform.h
 * \ingroup stream
 *
 */

#ifndef STREAM_TRANSFORM_H
#define STREAM_TRANSFORM_H

/****************************************************************************
Include Files
*/


/****************************************************************************
Private Type Declarations
*/


/**
 * Direction for rate matching and priming.
 * Tells the opmgr functions that rate-matching/priming occur at the start or end of the chain.
 * Important - keep in sync with $chain_proc_str in opmgr\operators\capabilities.h
 */
typedef enum
{
    INPUT,
    OUTPUT
} RM_DIRECTION;

/**
 * Structure that describes a transform.
 */
struct TRANSFORM
{
    /**
     * Pointer to the source endpoint of the transform
     */
    struct ENDPOINT *source;

    /**
     * Pointer to the sink endpoint of the transform
     */
    struct ENDPOINT *sink;

    /**
     * Unique ID of the transform
     */
    unsigned int id:8;

    /**
     * Flag indicating whether the transform created this buffer. TRUE ==
     * transform created this buffer
     */
    bool created_buffer:1;

    /**
     * Flag indicating whether the transforms buffer is shared with another
     * transform. This is used for in place processing.
     */
    bool shared_buffer:1;

    /**
     * Pointer to next transform in the list
     */
    struct TRANSFORM *next;

    /**
     * Transform specific state information
     */
    tCbuffer *buffer;
};

/* All flags must fit in a uint16 */
struct BUFFER_CONFIG_FLAGS
{
    bool override_source:1;
    bool override_sink:1;
    bool created_buffer:1;
    bool shared_buffer:1;
    bool source_wants_kicks:1;
    bool sink_wants_kicks:1;

#ifdef INSTALL_METADATA_DUALCORE
    bool supports_metadata:1; /* Both endpoints support metadata. */
#endif /* INSTALL_METADATA_DUALCORE */

    /* rest of the bit as any other flags required
     * for the buffer.
     */
    unsigned extra_flags:9;
};

/* This is to mask out the supports_metadata flag from the rest of flags */
#define SUPPORT_META_MASK    0x70

typedef struct
{
    tCbuffer *buffer;

    /* if there is no buffer created, this is the
     * minimum buffer size for negotiation.
     */
    unsigned buffer_size;

    struct BUFFER_CONFIG_FLAGS flags;

} STREAM_TRANSFORM_BUFFER_INFO;

/* Structure that holds intermediate details during stream connect
 * like buffer details and timing information etc
 */
struct STREAM_CONNECT_INFO
{
    STREAM_TRANSFORM_BUFFER_INFO buffer_info;
    ENDPOINT*  ep_to_kick;
};


/****************************************************************************
Private Macro Declarations
*/

#define STREAM_TRANSFORM_GET_INT_ID(id)  \
            (((id) != 0)? id ^ TRANSFORM_COOKIE: 0)

#define STREAM_TRANSFORM_GET_EXT_ID STREAM_TRANSFORM_GET_INT_ID

/****************************************************************************
Public Function Definitions
*/
/**
 * \brief Closes a buffer with no kicks type of transform.It
 *        will destory the buffer.
 *
 * \param t pointer to the transform whose buffer is to be closed
 *
 * \return TRUE if successfully destroyed, FALSE otherwise.
 */
bool stream_transform_close(TRANSFORM *t);

/**
 * \brief returns the buffer associated with this transform
 *
 * \param transform pointer to the transform to get the buffer of.
 *
 * \return Cbuffer associated with the transform.
 */
tCbuffer* stream_transform_get_buffer(TRANSFORM *transform);

#endif /* STREAM_TRANSFORM_H */
