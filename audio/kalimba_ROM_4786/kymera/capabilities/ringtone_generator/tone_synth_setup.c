/**
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * \defgroup tone_synth
 * \file  tone_synth_setup.c
 * \ingroup  capabilities

 *  Tone synthesizer calculate synthesis parameters
 *
 */
#include "tone_synth_private.h"

#define BASE_SAMPLE_RATE       8000
#define BASE_NOTE_PITCH        119
#define SEMI_TONES_PER_OCTAVE  12

static uint32 udiv3216(uint16 *r, uint16 d, uint32 n);

static uint32 udiv3216(uint16 *r, uint16 d, uint32 n)
{
    uint32 q;

    q = n/d;
    if (r != NULL)
    {
        *r = (uint16) (n - q*d);
    }

    return q;
}

void tone_synth_set_note(TONE_SYNTH_DATA *synth_data, unsigned pitch_index,
                       unsigned length_num, unsigned length_den,
                       bool is_rest, bool is_tied)
{
    uint16 f, o, s;
    uint32 t;

    static const uint16 note_freq[12] = {
       /* Base is note 119(BASE_NOTE_PITCH)
          running backwards */
       0xfcde, 0xeead, 0xe147, 0xd4a2, 0xc8b3, 0xbd6f,
       0xb2ce, 0xa8c4, 0x9f4c, 0x965b, 0x8dea, 0x85f3
    };

    /* Calculate note duration in terms of number of samples.*/
    /* info->tempo is SEMIBREVE(whole note) duration in no. of samples */
    synth_data->tone_data.duration =  (synth_data->tone_info.tempo / length_den) * length_num;

    /* Alter tone duration depending on the sampling rate */
   {
        uint32 temp = ((uint32)synth_data->tone_data.duration * synth_data->tone_sample_rate);
        synth_data->tone_data.duration = (uint16)(temp / BASE_SAMPLE_RATE);
    }
    /**/

    if (is_rest)
    {
        /* To avoid pop noise at end of tone(B-138115), Rest note is
           treated as a tied note of the same pitch as that of the
           previous note. And decay is set such that tone volume
           decays to 0 in 64 samples. */
        synth_data->tone_data.time = 0;
        synth_data->tone_data.decay = (uint16)(synth_data->tone_data.volume / 64);
    }

    else
    {
        f = BASE_NOTE_PITCH - pitch_index;

        o = (uint16) udiv3216(&s, SEMI_TONES_PER_OCTAVE, (uint32) f);
        synth_data->tone_data.step = note_freq[s]>>o;

        /*Alter tone step*/
        {
        uint32 temp = ((uint32)synth_data->tone_data.step * BASE_SAMPLE_RATE);
        synth_data->tone_data.step = (uint16)(temp / synth_data->tone_sample_rate);
        }
        /**/
        if (is_tied)
        {
            synth_data->tone_data.time = 0;
        }
        else
        {
            tone_synth_tone_start(synth_data);
        }
        /* Calculate volume decrease per sample period. */
        if ((uint16)synth_data->tone_info.decay == 0)
        {
            /* Special value meaning 'no decay'. (It would be
             * meaningless, and give a division by zero in the below
             * expression.) */
            t = 0;
        }
        else
        {
            /*
             Volume decays to 0 in (n = note_duraton*decay/16) samples.
             Volume decay per sample = Init_volume / n
                                     = Init_volume*16/(note_duration*decay)
             */
            t = udiv3216(NULL, (uint16)synth_data->tone_info.decay, udiv3216(NULL, (uint16)synth_data->tone_data.duration, synth_data->tone_data.volume << 8) << 4);
        }
        synth_data->tone_data.decay = (uint16) (t>>8);
    }
}

void tone_synth_set_tempo(TONE_SYNTH_DATA *synth_data, unsigned tempo)
{
    /* Tempo is specified in crotchets(quarter notes) per minute. */
    /* From tempo, we calculate SEMIBREVE(whole note) duration in terms of number of samples. */
    /* Example :
       Let's say tempo = 400(meaning 400 quarter notes per min.)
                       = 400/4 whole notes per min
                       = 400/(4*60) whole notes per sec
       If Fs = 8000Hz, then
       400/(4*60) whole notes per sec = 8000 samples per sec
       400/(4*60) whole notes = 8000 samples
       1 whole note = (8000*4*60)/400 samples
     */
    uint32 t;
    t = udiv3216(NULL, (uint16)tempo, 8000UL*60*4);

    if (t > 65535)
    {
        synth_data->tone_info.tempo = 65535U;
    }
    else
    {
        synth_data->tone_info.tempo = (uint16) t;
    }
}

