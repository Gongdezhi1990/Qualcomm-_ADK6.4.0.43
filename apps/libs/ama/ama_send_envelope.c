/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_send_envelope.c

DESCRIPTION
    Sends control envelope to transport
*/

#include <panic.h>
#include <stdlib.h>
#include "ama_debug.h"
#include "ama_transport.h"
#include "ama_send_envelope.h"


void amaSendEnvelope(ControlEnvelope* control_envelope_out)
{
#define AMA_HEADER_SIZE_SMALL_ENVELOPE 3
#define AMA_HEADER_SIZE_LARGE_ENVELOPE 4
#define SIZE_LARGE_ENVELOPE 0xff

    uint16 header_size = AMA_HEADER_SIZE_SMALL_ENVELOPE;
    size_t envelope_size = control_envelope__get_packed_size(control_envelope_out);
    uint8 *packed_envelope;

    if(envelope_size > SIZE_LARGE_ENVELOPE)
    {
        header_size = AMA_HEADER_SIZE_LARGE_ENVELOPE;
    }

    packed_envelope = PanicUnlessMalloc(envelope_size + header_size);

    if(control_envelope__pack(control_envelope_out, (packed_envelope + header_size)) != envelope_size)
    {
        AMA_DEBUG(("AMA Error building packed envelope %d\n", envelope_size));
    }

    /* don't include the header size ... transport will take care of that */
    amaTranportSendProtoBuf(packed_envelope, envelope_size);

    free(packed_envelope);
}

