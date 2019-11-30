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
    RINGTONE_GENERATOR_OP_DATA *op_data = (RINGTONE_GENERATOR_OP_DATA *)cb_data;

    *(op_data->out_cbuffer->write_ptr)++ = sample;

    /* check if the write pointer has wrapped around */
    if(op_data->out_cbuffer->write_ptr >= ( op_data->out_cbuffer->base_addr + cbuffer_get_size_in_words(op_data->out_cbuffer)) )
    {
        op_data->out_cbuffer->write_ptr -= cbuffer_get_size_in_words(op_data->out_cbuffer);
    }
}

/**
* \brief    Main API function of generating the ringtone audio  
*
* \param    op_data  The ringtone generator specific operator data
* \param    len      The length of how many samples ringtone_tone_play each time
*/
void ringtone_play(RINGTONE_GENERATOR_OP_DATA *op_data, unsigned len)
{
    bool res = TRUE;
    unsigned space;

    
    for(space = cbuffer_calc_amount_space_in_words(op_data->out_cbuffer); space >= len; space = space - len )
    {
        res = TONE_SYNTH_TONE_PLAY(&op_data->info, len);

        if (!res)
        {
           if(!(res = ringtone_sequence_advance(op_data)))
           {
                return;
           }
        }
    }
}


