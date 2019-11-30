/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_speech_packetiser.c

DESCRIPTION
    Copies data from encoder output source into buffer and sends
*/

#include "ama.h"
#include "ama_speech_packetiser.h"
#include "ama_transport.h"
#include <stdlib.h>
#include <panic.h>
#include "ama_debug.h"
#include <string.h>

bool amaSendMsbcSourceSpeechData(Source source)
{

    #define AMA_HEADER_LEN 4
    #define MSBC_ENC_PKT_LEN 60
    #define MSBC_FRAME_LEN 57
    #define MSBC_FRAME_COUNT 5
    #define MSBC_BLE_FRAME_COUNT 1

    uint8 frames_to_send;
    uint16 payload_posn;
    uint16 lengthSourceThreshold;
    uint8 *buffer = NULL;
    uint8 no_of_transport_pkt = 0;
    uint8 initial_position = 0;

    bool sent_if_necessary = FALSE;

    if(AmaTransportGet() == ama_transport_ble)
    {
        frames_to_send = MSBC_BLE_FRAME_COUNT;
        initial_position = AMA_HEADER_LEN - 1;
    }
    else
    {
        frames_to_send = MSBC_FRAME_COUNT;
        initial_position = AMA_HEADER_LEN;
    }

    lengthSourceThreshold = MSBC_ENC_PKT_LEN * frames_to_send;

    AMA_DEBUG(("In = %d\n", SourceSize(source)));

    while ((SourceSize(source) >= (lengthSourceThreshold + 2)) && no_of_transport_pkt < 3)
    {
        const uint8 *source_ptr = SourceMap(source);
        uint32 copied = 0;
        uint32 frame;

        if(!buffer)
            buffer = PanicUnlessMalloc((MSBC_FRAME_LEN * frames_to_send) + AMA_HEADER_LEN);

        payload_posn = initial_position;
        
        for (frame = 0; frame < frames_to_send; frame++)
        {
            memmove(&buffer[payload_posn], &source_ptr[(frame * MSBC_ENC_PKT_LEN) + 2], MSBC_FRAME_LEN);
            payload_posn += MSBC_FRAME_LEN;
            copied += MSBC_FRAME_LEN;
        }

        sent_if_necessary = amaTranportStreamData(buffer, copied);

        if(sent_if_necessary)
        {
            AMA_DEBUG(("S%d\n", copied));
            SourceDrop(source, lengthSourceThreshold);
        }
        else
        {
            AMA_DEBUG(("F\n"));
            break;
        }
		no_of_transport_pkt++;
    }

    free(buffer);

    AMA_DEBUG(("Remaining = %d\n", SourceSize(source)));

    return sent_if_necessary;
}

