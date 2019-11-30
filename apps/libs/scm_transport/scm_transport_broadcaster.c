/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    scm_transport_broadcaster.c
*/

#include <scm.h>
#include <scm_transport.h>
#include <packetiser_helper.h>
#include <panic.h>
#include <stdlib.h>

static void scmbTransportSegmentRemove(scm_transport_t *transport, scm_segment_t *segment)
{
    /* inform client that segment has been sent */
    ScmBroadcastSegmentCfm(transport->client.tx, segment->header, segment->u.tx.remaining);

    segment->valid = FALSE;
}

scm_transport_t *ScmbTransportInit(SCMB *client)
{
    uint32 index;
    scm_transport_t *transport = NULL;

    PanicNull(client);

    transport = PanicUnlessMalloc(sizeof(*transport));

    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        transport->segment[index].valid = FALSE;
        transport->segment[index].u.tx.state = SCMB_STATE_WAITING;
    }
    transport->client.tx = client;
    transport->segment_index = 0;

    return transport;
}

void ScmbTransportShutdown(scm_transport_t *transport)
{
    uint32 index;
    PanicNull(transport);
    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        scm_segment_t *segment = &transport->segment[index];
        if (segment->valid)
        {
            scmbTransportSegmentRemove(transport, segment);
        }
    }
    free(transport);
}

bool ScmbTransportSegmentQueue(scm_transport_t *transport, uint8 header,
                               uint8 *payload, uint16 tx_count)
{
    uint32 index;

    PanicNull(transport);
    PanicNull(payload);

    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        scm_segment_t *segment = &transport->segment[index];
        if (!segment->valid)
        {
            segment->valid = TRUE;
            segment->header = header;
            memcpy(segment->payload, payload, SCM_SEGMENT_DATA_SIZE);
            segment->u.tx.remaining = tx_count;
            segment->u.tx.state = SCMB_STATE_WAITING;
            return TRUE;
        }
    }
    return FALSE;
}

uint32 ScmbTransportOfferSpace(scm_transport_t *transport, uint32 space)
{
    uint32 spacex = space;
    scm_segment_t *segment;

    PanicNull(transport);

    for (segment = &transport->segment[0];
         segment != &transport->segment[SCM_NUM_SEGMENTS] && spacex >= SCM_SEGMENT_SIZE;
         segment++)
    {
        if (segment->valid)
        {
            if (segment->u.tx.state != SCMB_STATE_IN_PROGRESS)
            {
                spacex -= SCM_SEGMENT_SIZE;
            }
        }
    }
    return space - spacex;
}

uint32 ScmbTransportWriteSegments(scm_transport_t *transport, uint8 *dest, uint32 space)
{
    uint32 index = transport->segment_index;

    PanicNull(transport);
    PanicNull(dest);

    while (space >= SCM_SEGMENT_SIZE)
    {
        scm_segment_t *segment = &transport->segment[index];

        /* copy this segment if it's valid */
        if (segment->valid)
        {
            /* don't tx segment already in progress */
            if (segment->u.tx.state != SCMB_STATE_IN_PROGRESS)
            {
                /* write segment into buffer, tag, header, then payload */
                dest[0] = (1 + SCM_SEGMENT_DATA_SIZE) +
                    (packetiser_helper_non_audio_type_scm << TAG_NON_AUDIO_TYPE_SHIFT);
                dest[1] = segment->header;
                memcpy(dest + 2, segment->payload, SCM_SEGMENT_DATA_SIZE);

                /* set state to in_progress */
                segment->u.tx.state = SCMB_STATE_IN_PROGRESS;

                /* reduce size left for SCM */
                space -= SCM_SEGMENT_SIZE;
                dest += SCM_SEGMENT_SIZE;
            }
        }

        /* segment wasn't valid, try next one */
        index = (index + 1) % SCM_NUM_SEGMENTS;

        /* exit loop if we've gone around all the segments */
        if (index == transport->segment_index)
            break;
    }

    /* copy next segment next time */
    transport->segment_index = index;

    return space;
}

void ScmbTransportSegmentUpdateLifetime(scm_transport_t *transport, uint16 num_tx)
{
    uint32 index;

    PanicNull(transport);

    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        scm_segment_t *segment = &transport->segment[index];
        if (segment->u.tx.state == SCMB_STATE_IN_PROGRESS)
        {
            segment->u.tx.state = SCMB_STATE_WAITING;
            if (segment->valid)
            {
                uint16 remaining = segment->u.tx.remaining;
                if (remaining > num_tx)
                    segment->u.tx.remaining -= num_tx;
                else
                {
                    segment->u.tx.remaining = 0;
                    scmbTransportSegmentRemove(transport, segment);
                }
            }
        }
    }
}
