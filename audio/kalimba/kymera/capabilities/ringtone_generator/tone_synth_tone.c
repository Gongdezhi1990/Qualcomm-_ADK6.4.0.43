/**
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * \defgroup tone_synth
 * \file  tone_synth_tone.c
 * \ingroup  capabilities
 *
 * Tone synthesizer, synthesizer core
 *
 */

#include "tone_synth_private.h"

int16 tone_synth_sin_8bit(uint16 x)
{
    static const unsigned short sintable[33] = {
    0x0003, 0x0609, 0x0d10, 0x1316, 0x191c, 0x1f22, 0x2528, 0x2b2e,
    0x3134, 0x373a, 0x3c3f, 0x4244, 0x474a, 0x4c4f, 0x5154, 0x5658,
    0x5b5d, 0x5f61, 0x6365, 0x6769, 0x6a6c, 0x6e6f, 0x7172, 0x7475,
    0x7677, 0x797a, 0x7a7b, 0x7c7d, 0x7e7e, 0x7f7f, 0x7f7f, 0x7f7f,
    0x7f7f,
    };
    int16 s;

    s = x & 0x3f;
    if (x & 0x40)
        s = 0x40 - s;
    s = sintable[s>>1];

    if (x & 1)
        s = s & 0xff;
    else
        s = s >> 8;

    if (x & 0x80)
        s = -s;

    return s;
}

/**
* \brief    Start a tone: reset phase accumulator and sample counter,
*           set initial amplitude
*
* \param    synth_data  The tone synthesizer context
*/
void tone_synth_tone_start(TONE_SYNTH_DATA *synth_data)
{
    synth_data->tone_data.accumulator = 0;
    synth_data->tone_data.volume = (uint32) synth_data->tone_info.volume << 16;
    synth_data->tone_data.time = 0;
}

/**
* \brief    Initialise a tone
*
* \param    synth_data  The tone synthesizer context
*/
void tone_synth_init(TONE_SYNTH_DATA *synth_data)
{
    synth_data->tone_info.decay = 0x20;
    synth_data->tone_info.tempo = 32000;

    synth_data->tone_info.type = tone_synth_tone_sine;
    synth_data->tone_data.step = 0;
    synth_data->tone_data.duration = 0;
    synth_data->tone_info.volume = TONE_SYNTH_MAX_VOLUME;
    synth_data->tone_data.decay = 0;

    tone_synth_tone_start(synth_data);
}

/**
* \brief    return next n samples of tone audio
*
* \param    op_data  The ringtone generator specific operator data
* \param    samples  Samples of tone audio the function generates each time
*
* \return   TRUE    More samples will be generated
*           FALSE   No more samples of tone audio
*/
bool tone_synth_tone_play(TONE_SYNTH_DATA *synth_data, unsigned num_of_samples)
{
    unsigned i;
    int16  v, samp, s;
    uint32 dv;

    for(i = num_of_samples; i != 0; i -= 1)
    {
        v = (uint16) (synth_data->tone_data.volume >> 16);

        dv = (uint32) synth_data->tone_data.decay;
        if(synth_data->tone_data.volume < dv)
        {
            synth_data->tone_data.volume = 0;
        }
        else
        {
           synth_data->tone_data.volume -= dv;
        }

        samp = (int16)(synth_data->tone_data.accumulator += synth_data->tone_data.step);

        switch(synth_data->tone_info.type)
        {
        default:
        case tone_synth_tone_sine:
            samp = tone_synth_sin_8bit((uint16)(samp >> 8));
            break;
        case tone_synth_tone_square:
            samp = (samp & 0x8000) ? - 0x80 : 0x7f;
            break;
        case tone_synth_tone_saw:
            samp = ((uint16) samp >> 8) - 0x80;
            break;
        case tone_synth_tone_triangle:
            s = ((uint16) samp + 0x4000) >> 7;
            if (s & 0x0100)
                samp = 0x7f - (s & 0x00ff);
            else
                samp = s - 0x80;
            break;
        case tone_synth_tone_triangle2:
            if ((uint16) samp >= 0xc000)
            samp = 0x7f - (((uint16) samp >> 6) & 0x00ff);
            else
            samp = (((((uint16) samp) >> 8) * 343) >> 8) - 0x80;
            break;
        case tone_synth_tone_clipped_sine:
            samp = tone_synth_sin_8bit((uint16) (samp >> 8));
            samp += (samp >> 1);
            if (samp > 0x7f)
                samp = 0x7f;
            else if (samp < -0x80)
                samp = -0x80;
            break;
        case tone_synth_tone_plucked:
            s = (uint16) samp >> 8;
            samp = ((0x7f - (int16) s)*v + tone_synth_sin_8bit(s)*(256-v))>>8;
            break;
        }

        if (synth_data->writer_fn != NULL)
        {
            (synth_data->writer_fn)(synth_data->writer_data, (samp * v) << (DAWTH - 16) );
        }

    }

   /* Written to avoid unsigned wrapping (consider duration = 65535) */
    if (synth_data->tone_data.duration - synth_data->tone_data.time > num_of_samples)
    {
        synth_data->tone_data.time += num_of_samples;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

