/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  wbs.c
 * \ingroup  operators
 *
 *  WBS_ENC/WBS_DEC operator common code
 *
 */

/****************************************************************************
Include Files
*/
#include "wbs_private.h"

#include "patch/patch.h"

/***************************** WBS shared tables ******************************/
/** memory shared between enc and dec */
const share_malloc_t_entry wbs_sbc_shared_malloc_table[WBS_SBC_SHARED_TABLE_LENGTH] =
{
    {80, MALLOC_PREFERENCE_DM1, SBC_SHARED_WIN_COEFS_M8, offsetof(sbc_codec, win_coefs_m8)},
    {48, MALLOC_PREFERENCE_NONE, SBC_SHARED_LOUDNESS_OFFSET, offsetof(sbc_codec, loudness_offset)},
    {66, MALLOC_PREFERENCE_FAST, SBC_SHARED_BITSLICE_LOOKUP, offsetof(sbc_codec, bitslice_lookup)},
};

/** memory shared between encoders */
const share_malloc_t_entry wbs_sbc_enc_shared_malloc_table[WBS_SBC_ENC_SHARED_TABLE_LENGTH] =
{
    {128, MALLOC_PREFERENCE_DM2, SBC_ENC_SHARED_ANALYSIS_COEFS_M8, offsetof(sbc_codec, analysis_coefs_m8)},
    {16, MALLOC_PREFERENCE_NONE, SBC_ENC_SHARED_LEVEL_COEFS, offsetof(sbc_codec, level_coefs)}
};

/** memory shared between decoders */
const share_malloc_t_entry wbs_sbc_dec_shared_malloc_table[WBS_SBC_DEC_SHARED_TABLE_LENGTH] =
{
    {128, MALLOC_PREFERENCE_DM2, SBC_DEC_SHARED_SYNTHESIS_COEFS_M8, offsetof(sbc_codec, synthesis_coefs_m8)},
    {15, MALLOC_PREFERENCE_NONE, SBC_DEC_SHARED_LEVELRECIP_COEFS, offsetof(sbc_codec, levelrecip_coefs)},
    {17, MALLOC_PREFERENCE_NONE, SBC_DEC_SHARED_BITMASK_LOOKUP, offsetof(sbc_codec, bitmask_lookup)}
};

/** Scratch memory used by an encoder and decoder instance */
const scratch_malloc_t_entry wbs_scratch_table_dm1[WBS_DM1_SCRATCH_TABLE_LENGTH] =
{
    {WBS_AUDIO_SAMPLE_BUFF_SIZE, offsetof(sbc_codec, audio_sample)},
    {SBC_SCALE_FACTOR_LENGTH, offsetof(sbc_codec, scale_factor)}
};


/****************************************************************************
Public Function Definitions - shared by encoder and decoder
*/
bool wbs_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    unsigned wbs_in_size = WBS_DEC_INPUT_BUFFER_SIZE;
    unsigned wbs_out_size = WBS_DEC_OUTPUT_BUFFER_SIZE;

    if (op_data->cap_data->id == CAP_ID_WBS_ENC)
    {
        wbs_in_size = WBS_ENC_INPUT_BUFFER_SIZE;
        wbs_out_size = WBS_ENC_OUTPUT_BUFFER_SIZE;
    }
    else
    {
        /* for decoder, the output size might have been configured by the user */
        WBS_DEC_OP_DATA* wbs_data = (WBS_DEC_OP_DATA*)(op_data->extra_op_data);
        SCO_COMMON_RCV_OP_DATA* sco_data = &(wbs_data->sco_rcv_op_data);
        wbs_out_size = MAX(wbs_out_size, sco_data->sco_rcv_parameters.output_buffer_size);
    }

    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Currently these have the same value but this isn't guaranteed */
    if (((unsigned *)message_data)[0] & TERMINAL_SINK_MASK)
    {
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = wbs_in_size;
    }
    else
    {
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = wbs_out_size;
    }

#ifdef SCO_RX_OP_GENERATE_METADATA
    if (op_data->cap_data->id == WBS_DEC_CAP_ID)
    {
        L2_DBG_MSG("WBS_RX_OP_GENERATE_METADATA, metadata is supported");
        /* supports metadata in both side  */
        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = 0;
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;
    }
#endif /* SCO_RX_OP_GENERATE_METADATA */

    L4_DBG_MSG2( "wbs_buffer_details (capID=%d)  %d \n", op_data->cap_data->id, ((OP_STD_RSP*)*response_data)->resp_data.data);

    return TRUE;
}
