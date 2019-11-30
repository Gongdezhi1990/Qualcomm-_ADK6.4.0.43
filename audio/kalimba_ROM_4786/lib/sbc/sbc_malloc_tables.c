/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sbc_malloc_tables.c
 * \ingroup  sbc
 *
 *  Malloc tables needed for SBC
 *
 */

/****************************************************************************
Include Files
*/
#include "sbc_c.h"

/****************************************************************************
Private Constant Definitions
*/
/** memory shared between enc and dec */
const share_malloc_t_entry sbc_shared_malloc_table[SBC_SHARED_TABLE_LENGTH] =
{
    {80, MALLOC_PREFERENCE_DM1, SBC_SHARED_WIN_COEFS_M8, offsetof(sbc_codec, win_coefs_m8)},
    {40, MALLOC_PREFERENCE_DM1, SBC_SHARED_WIN_COEFS_M4, offsetof(sbc_codec, win_coefs_m4)},
    {48, MALLOC_PREFERENCE_NONE, SBC_SHARED_LOUDNESS_OFFSET, offsetof(sbc_codec, loudness_offset)},
    {66, MALLOC_PREFERENCE_FAST, SBC_SHARED_BITSLICE_LOOKUP, offsetof(sbc_codec, bitslice_lookup)},
};

/** memory shared between encoders */
const share_malloc_t_entry sbc_enc_shared_malloc_table[SBC_ENC_SHARED_TABLE_LENGTH] =
{
    {128, MALLOC_PREFERENCE_DM2, SBC_ENC_SHARED_ANALYSIS_COEFS_M8, offsetof(sbc_codec, analysis_coefs_m8)},
    {32, MALLOC_PREFERENCE_DM2, SBC_ENC_SHARED_ANALYSIS_COEFS_M4, offsetof(sbc_codec, analysis_coefs_m4)},
    {16, MALLOC_PREFERENCE_NONE, SBC_ENC_SHARED_LEVEL_COEFS, offsetof(sbc_codec, level_coefs)}
};

/** memory shared between decoders */
const share_malloc_t_entry sbc_dec_shared_malloc_table[SBC_DEC_SHARED_TABLE_LENGTH] =
{
    {128, MALLOC_PREFERENCE_DM2, SBC_DEC_SHARED_SYNTHESIS_COEFS_M8, offsetof(sbc_codec, synthesis_coefs_m8)},
    {32, MALLOC_PREFERENCE_DM2, SBC_DEC_SHARED_SYNTHESIS_COEFS_M4, offsetof(sbc_codec, synthesis_coefs_m4)},
    {15, MALLOC_PREFERENCE_NONE, SBC_DEC_SHARED_LEVELRECIP_COEFS, offsetof(sbc_codec, levelrecip_coefs)},
    {17, MALLOC_PREFERENCE_NONE, SBC_DEC_SHARED_BITMASK_LOOKUP, offsetof(sbc_codec, bitmask_lookup)}
};


/** DM1 Scratch memory used by an encoder and decoder instance */
const scratch_malloc_t_entry sbc_scratch_table_dm1[SBC_ENC_DM1_SCRATCH_TABLE_LENGTH] =
{
    {SBC_AUDIO_SAMPLE_BUFF_SIZE, offsetof(sbc_codec, audio_sample)},
    {SBC_SCALE_FACTOR_LENGTH, offsetof(sbc_codec, scale_factor)},
    /* Fields after this point are only used by the encoder capability */
    {SBC_AUDIO_SAMPLE_BUFF_SIZE, offsetof(sbc_codec, audio_sample_js)},
    {SBC_SCALE_FACTOR_LENGTH, offsetof(sbc_codec, scale_factor_js)}
};

/** DM2 Scratch memory used by an encoder and decoder instance */
const scratch_malloc_t_entry sbc_scratch_table_dm2[SBC_ENC_DM2_SCRATCH_TABLE_LENGTH] =
{
    {SBC_BITS_LENGTH, offsetof(sbc_codec, bits)},
    {SBC_BITNEED_LENGTH, offsetof(sbc_codec, bitneed)},
    /* Fields after this point are only used by the encoder capability */
    {SBC_ANALYSIS_Y_LENGTH, offsetof(sbc_codec, analysis_y)},
};

