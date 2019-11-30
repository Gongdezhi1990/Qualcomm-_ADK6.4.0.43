/**
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
 * \defgroup ringtone_generator
 * \file  ringtone_play.c
 * \ingroup  capabilities

 *  Ringtone generator play function
 *
 */
/****************************************************************************/

#include "ringtone_generator_cap.h"


/**
 * \brief   Callback to output one sample
 * \param   cb_data Caller context
 * \param   value   Sample value
 */
void ringtone_write(void* cb_data, int sample)
{
    RINGTONE_GENERATOR_OP_DATA *rgop_data = (RINGTONE_GENERATOR_OP_DATA *)cb_data;

    *(rgop_data->out_cbuffer->write_ptr)++ = sample;

    /* check if the write pointer has wrapped around */
    if(rgop_data->out_cbuffer->write_ptr >= ( rgop_data->out_cbuffer->base_addr + cbuffer_get_size_in_words(rgop_data->out_cbuffer)) )
    {
        rgop_data->out_cbuffer->write_ptr -= cbuffer_get_size_in_words(rgop_data->out_cbuffer);
    }
}

/**
* \brief    Main API function of generating the ringtone audio
*
* \param    rgop_data   The ringtone generator specific operator data
* \param    gen_samples How many samples to generate each time
*/
void ringtone_play(RINGTONE_GENERATOR_OP_DATA *rgop_data, unsigned gen_samples)
{
    bool res = TRUE;
    unsigned space, len;
    int gen_spl_i = gen_samples;

    space = cbuffer_calc_amount_space_in_words(rgop_data->out_cbuffer);
    PL_ASSERT( space >= gen_samples);
    PL_ASSERT( gen_spl_i >= 0);
    len = rgop_data->tone_data_block_size;
    while( gen_spl_i >= (int)len )
    {
        res = TONE_SYNTH_TONE_PLAY(&rgop_data->info, len);
        if (!res)
        {
            if(!(res = ringtone_sequence_advance(rgop_data)))
            {
                break;
            }
        }
        gen_spl_i -= len;
    }
}


