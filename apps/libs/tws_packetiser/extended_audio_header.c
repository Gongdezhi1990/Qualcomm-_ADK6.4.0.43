/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    extended_audio_header.c
*/

#include <stdlib.h>
#include <packetiser_helper.h>
#include "extended_audio_header.h"

eah_reader_state_t * eahReaderInit(const uint8 *base, uint32 maxlen)
{
    uint32 nframes, flen;
    uint16 *lengths_new;
    eah_reader_state_t *state = NULL;
    uint16 eah = 0;

    if (base && (NULL != (state = calloc(1, sizeof(*state)))))
    {
        state->lengths = NULL;
        do
        {
            state->eahlen += 2;
            if (state->eahlen > maxlen)
            {
                eahReaderDestroy(state);
                state = NULL;
                break;
            }

            /* Compose the next word in the eah */
            eah = *base++;
            eah |= *base++ << 8;

            /* Extract data from eah */
            nframes = EXTENDED_AUDIO_HEADER_GET_NFRMS(eah);
            flen = EXTENDED_AUDIO_HEADER_GET_FLEN(eah);
            state->sum += (nframes * flen);

            /* Allocate memory for the new frame lengths */
            nframes += state->frames;
            if (nframes &&
                (NULL != (lengths_new = realloc(state->lengths,
                                                nframes * sizeof(state->lengths[0])))))
            {
                state->lengths = lengths_new;
            }
            else
            {
                eahReaderDestroy(state);
                state = NULL;
                break;
            }

            for (; state->frames < nframes; state->frames++)
            {
                state->lengths[state->frames] = flen;
            }

        } while (!(eah & EXTENDED_AUDIO_HEADER_END_MASK));
    }
    return state;
}

void eahReaderDestroy(eah_reader_state_t *state)
{
    if (state)
    {
        if (state->lengths)
        {
            free(state->lengths);
        }
        free(state);
    }
}

uint32 eahReaderGetFrameLen(eah_reader_state_t *state, uint32 index)
{
    if (state && index < state->frames)
    {
        return state->lengths[index];
    }
    return 0;
}

uint32 eahReaderGetFramesLen(eah_reader_state_t *state, uint32 per_frame_overhead)
{
    return state->sum + (state->frames * per_frame_overhead);
}

uint32 eahReaderGetEahLen(eah_reader_state_t *state)
{
    return state->eahlen;
}

bool eahReaderPacketLengthMismatch(eah_reader_state_t *state,
                                   uint32 unread_packet_bytes,
                                   uint32 per_frame_overhead)
{
    return (unread_packet_bytes !=
            (state->eahlen + eahReaderGetFramesLen(state, per_frame_overhead)));
}
