/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    scm_transport_receiver.c
*/

#include <scm.h>
#include <scm_transport.h>
#include <packetiser_helper.h>
#include <rtime.h>
#include <panic.h>
#include <stdlib.h>
#include <system_clock.h>

static void scmrTransportSegmentRemove(scm_transport_t *transport, scm_segment_t *segment)
{
    /* callback to application */
    ScmReceiverSegmentExpiredInd(transport->client.rx, segment->header);

    /* clear segment */
    segment->valid = FALSE;
}

scm_transport_t *ScmrTransportInit(SCMR *client)
{
    uint32 index;
    scm_transport_t *transport = NULL;

    PanicNull(client);

    transport = PanicUnlessMalloc(sizeof(*transport));

    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        transport->segment[index].valid = FALSE;
    }
    transport->client.rx = client;
    transport->segment_index = 0;

    return transport;
}

void ScmrTransportShutdown(scm_transport_t *transport)
{
    uint32 index;

    /* Expire any still valid segments */
    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        scm_segment_t *segment = &transport->segment[index];
        if (segment->valid)
        {
            scmrTransportSegmentRemove(transport, segment);
        }
    }
    free(transport);
}

void ScmrTransportSegmentCheck(scm_transport_t *transport)
{
    uint32 index;
    rtime_t time = SystemClockGetTimerTime();

    PanicNull(transport);

    /* check for stale segments */
    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        scm_segment_t *segment = &transport->segment[index];
        if (segment->valid)
        {
            if (rtime_gt(rtime_sub(time, segment->u.rx.time), 1000000))
            {
                scmrTransportSegmentRemove(transport, segment);
            }
        }
    }
}

void ScmrTransportReadSegment(scm_transport_t *transport, const uint8 *src)
{
    uint32 index;
    scm_segment_t *segment;
    uint8 header;
    rtime_t trx;

    PanicNull(transport);
    if (!src)
        Panic();

    header = *src++;
    trx = SystemClockGetTimerTime();

    /* check if this is a duplicate */
    for (index = 0; index < SCM_NUM_SEGMENTS; index++)
    {
        segment = &transport->segment[index];
        if (segment->valid && segment->header == header)
        {
            /* Is a duplicate, update last time received */
            segment->u.rx.time = trx;
            return;
        }
    }

    /* Current segment */
    segment = &transport->segment[transport->segment_index];

    /* Tell client segment has expired if overwriting existing segment */
    if (segment->valid)
    {
        scmrTransportSegmentRemove(transport, segment);
    }

    /* not a duplicate, so store it */
    segment->header = header;
    memcpy(segment->payload, src, SCM_SEGMENT_DATA_SIZE);
    segment->valid = TRUE;
    segment->u.rx.time = trx;

    transport->segment_index = (transport->segment_index + 1) % SCM_NUM_SEGMENTS;

    /* Tell client segment of new segment */
    ScmReceiverSegmentInd(transport->client.rx, segment->header, segment->payload);
}

