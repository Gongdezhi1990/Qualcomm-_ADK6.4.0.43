/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    scm_transport.h

DESCRIPTION
    Transport for sub-channel messaging (SCM).

    Handles broadcasting and receiving SCM segements.  Fragmentation and
    re-assembly is handled by VM SCM library.

    Functions with names starting Scmb are used to broadcast segments.
    Functions with names starting Scbr are used to receive segments.
*/

#ifndef SCM_TRANSPORT_H_
#define SCM_TRANSPORT_H_

#include <rtime.h>
#include <scm.h>

/* Number of concurrent segments allowed */
#define SCM_NUM_SEGMENTS   (8)

/** Size of SCM segment in octets, tag + header + data */
#define SCM_SEGMENT_SIZE (1 + 1 + SCM_SEGMENT_DATA_SIZE)

/** The receiver transport function ScmrTransportSegmentCheck() must be
    called at this interval in order to free stale received segments */
#define SCMR_TRANSPORT_KICK_PERIOD_MS 100

/** @defgroup SCM broadcast segment states
 *
 * @{
 */
/** Segment is waiting for transmission */
#define SCMB_STATE_WAITING (0)
/** Segment is being transmitted */
#define SCMB_STATE_IN_PROGRESS (1)
/** @} */

typedef struct __scm_segment
{
    /*! TRUE is segment is valid, FALSE if invalid */
    bool valid;
    /*! Segment header */
    uint8 header;
    /*! 24 bits of segment payload */
    uint8 payload[SCM_SEGMENT_DATA_SIZE];
    union
    {
        /*! Broadcaster state */
        struct
        {
            /*! Number of transmissions remaining for this segment */
            uint16 remaining;
            /*! Transmission state */
            uint16 state;
        } tx;

        /*! Receiver state */
        struct
        {
            /*! Last time this segment was received */
            rtime_t time;
        } rx;
    } u;
} scm_segment_t;

/**
* \brief SCM transport structure.
* Used for both broadcasting and receiving segments. To change role, the
* structure must be shutdown in present role and initialised in new role.
*/
typedef struct __scm_transport
{
    /*! The SCM broadcast or receiver client */
    union
    {
        SCMB *tx;
        SCMR *rx;
    } client;

    /*! The segments stored for transmission or on reception */
    scm_segment_t segment[SCM_NUM_SEGMENTS];
    /*! The active segment index */
    uint32 segment_index;
} scm_transport_t;

/**
 * \brief  Allocate and initialise SCM transport for broadcasting.
 *
 * \param client Pointer to the SCM client instance.
 * \return The initialised SCM transport structure.
 */
scm_transport_t *ScmbTransportInit(SCMB *client);

/**
 * \brief  Shutdown the SCM transport broadcaster
 *
 * \param transport Pointer to SCM transport structure.
 *
 * Releases any resources owned by SCM broadcaster transport.
 */
void ScmbTransportShutdown(scm_transport_t *transport);

/**
 * \brief Offer space in a packet for the SCM broadcaster transport to use.
 *
 * \param transport Pointer to SCM transport structure.
 * \param space The amount of space on offer for the transport to use.
 * \return The amount of space the SCM transport requires.
 */
uint32 ScmbTransportOfferSpace(scm_transport_t *transport, uint32 space);

/**
 * \brief  Write SCM segment(s) to destination.
 *
 * \param transport Pointer to SCM transport structure.
 * \param dest Write address where SCM segment(s) will be written.
 * \param space Number of octets available for SCM segments.
 *
 * Writes any segments waiting to be broadcast up to space number of octets.
 */
uint32 ScmbTransportWriteSegments(scm_transport_t *transport, uint8 *dest, uint32 space);

/**
 * \brief  Update broadcast lifetime of segments
 *
 * \param transport Pointer to SCM transport structure.
 * \param num_tx Number of times the segments(s) have been transmitted.
 *
 * Updates the broadcast lifetime of the segments previously written to a packet
 * using #ScmbTransportWriteSegments.  Once the lifetime reaches zero the segment
 * will be freed and the SCM client will be informed.
 */
void ScmbTransportSegmentUpdateLifetime(scm_transport_t *transport, uint16 num_tx);

/**
 * \brief  Queue segment for broadcast
 *
 * \param transport Pointer to SCM transport structure.
 * \param header    SCM segment header.
 * \param payload   Address of SCM segment payload.
 * \param tx_count  Number of transmissions required for this segment.
 * \return TRUE is the segment was queued, FALSE if there was no space
 * available and the segment was not queued.
 */
bool ScmbTransportSegmentQueue(scm_transport_t *transport, uint8 header,
                               uint8 *payload, uint16 tx_count);

/**
 * \brief  Allocate and initialise SCM transport for receiving.
 *
 * \param client Pointer to the SCM client instance.
 * \return The initialised SCM transport structure.
 */
scm_transport_t *ScmrTransportInit(SCMR *client);

/**
 * \brief  Shutdown SCM transport for receiving.
 *
 * \param transport Pointer to SCM transport structure.
 *
 * Releases any resources owned by SCM receiver.
 */
void ScmrTransportShutdown(scm_transport_t *transport);

/**
 * \brief  Check for any stale segments
 *
 * \param transport Pointer to SCM transport structure.
 *
 * Scans through list of received segments and free any that are considered
 * stale (not received with last second). The SCM client is informed when a
 * segment has expired.
 */
void ScmrTransportSegmentCheck(scm_transport_t *transport);

/**
 * \brief  Read SCM segment from the source.
 *
 * \param transport Pointer to SCM transport structure.
 * \param src Pointer to source containing SCM segment
 *
 * Reads SCM segment from source and stores it in list of received segments if
 * it hasn't been received before. Once list hits it's maximum size the oldest
 * segment is removed and the SCM client is informed that the segment has
 * expired. The SCM client is also informed of the new segment.
 */
void ScmrTransportReadSegment(scm_transport_t *transport, const uint8 *src);

#endif
