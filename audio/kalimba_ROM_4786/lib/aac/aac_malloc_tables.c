/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  aac_malloc_tables.c
 * \ingroup  aac
 *
 *  Malloc tables needed for AAC
 *
 */

/****************************************************************************
Include Files
*/
#include "aac_c.h"

/****************************************************************************
Private Constant Definitions
*/


/** memory shared between decoders */
const share_malloc_t_entry aac_dec_shared_malloc_table[] =
{
    {17,   MALLOC_PREFERENCE_DM2,  AAC_BITMASK_SHARED_LOOKUP_FIELD,                   offsetof(aac_codec, bitmask_lookup_field)},
    {4,    MALLOC_PREFERENCE_DM2,  AAC_TWO2QTRX_SHARED_LOOKUP_FIELD,                  offsetof(aac_codec, two2qtrx_lookup_field)},
    {36,   MALLOC_PREFERENCE_DM2,  AAC_X43_SHARED_LOOKUP1_FIELD,                      offsetof(aac_codec, x43_lookup1_field)},
    {36,   MALLOC_PREFERENCE_DM2,  AAC_X43_SHARED_LOOKUP2_FIELD,                      offsetof(aac_codec, x43_lookup2_field)},
    {64,   MALLOC_PREFERENCE_DM2,  AAC_X43_SHARED_LOOKUP32_FIELD,                     offsetof(aac_codec, x43_lookup32_field)},
    {9,    MALLOC_PREFERENCE_DM2,  AAC_TNS_MAX_SFB_LONG_TABLE_SHARED_FIELD,           offsetof(aac_codec, tns_max_sfb_long_table_field)},
    {24,   MALLOC_PREFERENCE_DM2,  AAC_TNS_LOOKUP_COEFS_SHARED_FIELD,                 offsetof(aac_codec, tns_lookup_coefs_field)},
    {12,   MALLOC_PREFERENCE_DM2,  AAC_SAMPLE_RATE_TAGS_SHARED_FIELD,                 offsetof(aac_codec, sample_rate_tags_field)},
    {8,    MALLOC_PREFERENCE_DM2,  AAC_LTP_COEFS_SHARED_FIELD,                        offsetof(aac_codec, ltp_coefs_field)},
    {4,    MALLOC_PREFERENCE_DM2,  AAC_SIN2048_COEFS_SHARED_FIELD,                    offsetof(aac_codec, sin2048_coefs_field)},
    {4,    MALLOC_PREFERENCE_DM2,  AAC_SIN256_COEFS_SHARED_FIELD,                     offsetof(aac_codec, sin256_coefs_field)},
    {36,   MALLOC_PREFERENCE_DM2,  AAC_KAISER2048_COEFS_SHARED_FIELD,                 offsetof(aac_codec, kaiser2048_coefs_field)},
    {36,   MALLOC_PREFERENCE_DM2,  AAC_KAISER256_COEFS_SHARED_FIELD,                  offsetof(aac_codec, kaiser256_coefs_field)},
    {12,   MALLOC_PREFERENCE_DM2,  AAC_HUFFMAN_OFFSETS_SHARED_FIELD,                  offsetof(aac_codec, huffman_offsets_field)},
    {4,    MALLOC_PREFERENCE_DM2,  AAC_SIN_CONST_SHARED_PTR,                          offsetof(aac_codec, sin_const_ptr)},
    {18,   MALLOC_PREFERENCE_DM2,  AAC_SIN_CONST_SHARED_IPTR,                         offsetof(aac_codec, sin_const_iptr)},
#ifdef AACDEC_SBR_ADDITIONS
    {321,  MALLOC_PREFERENCE_DM2,  AAC_SBR_QMF_FILTERBANK_WINDOW_FIELD,               offsetof(aac_codec, SBR_qmf_filterbank_window_ptr)},
    {216,  MALLOC_PREFERENCE_NONE, AAC_SBR_SMALL_CONST_ARRAYS_FIELD,                  offsetof(aac_codec, SBR_limiter_bands_compare_ptr)},
    {192,  MALLOC_PREFERENCE_NONE, AAC_SBR_DCT4_64_TABLE_FIELD,                       offsetof(aac_codec, SBR_dct4_64_table_ptr)},
    {100,  MALLOC_PREFERENCE_NONE, AAC_SBR_ONE_OVER_DIV_FIELD,                        offsetof(aac_codec, SBR_est_curr_env_one_over_div_ptr)},
    {64,   MALLOC_PREFERENCE_DM2,  AAC_SBR_SYNTH_PRE_PROCESS_REAL_FIELD,              offsetof(aac_codec, SBR_synthesis_pre_process_real_ptr)},
    {64,   MALLOC_PREFERENCE_DM2,  AAC_SBR_SYNTH_PRE_PROCESS_IMAG_FIELD,              offsetof(aac_codec, SBR_synthesis_pre_process_imag_ptr)},
    {128,  MALLOC_PREFERENCE_DM1,  AAC_SBR_SYNTH_POST_PROCESS_REAL_FIELD,             offsetof(aac_codec, SBR_synthesis_post_process_real_ptr)},
#endif
#ifdef AACDEC_PS_ADDITIONS
    {170,  MALLOC_PREFERENCE_NONE, AAC_PS_SMALL_CONST_ARRAYS_FIELD,                   offsetof(aac_codec, PS_num_sub_subbands_per_hybrid_qmf_subband_ptr)},
#endif
#ifdef AACDEC_ELD_ADDITIONS
    {61,   MALLOC_PREFERENCE_NONE, AAC_ELD_TWIDDLE_TABLE_FIELD,                       offsetof(aac_codec, ELD_twiddle_tab_ptr)},
    {1920, MALLOC_PREFERENCE_NONE, AAC_ELD_WIN_COEFS_TABLE_FIELD,                     offsetof(aac_codec, ELD_win_coefs_ptr)},
    {18,   MALLOC_PREFERENCE_NONE, AAC_ELD_SIN_CONST_TABLE_FIELD,                     offsetof(aac_codec, ELD_sin_const_ptr)},
    {16,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_PRE_COS_TABLE_FIELD,                  offsetof(aac_codec, ELD_dct4_pre_cos_ptr)},
    {16,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_PRE_SIN_TABLE_FIELD,                  offsetof(aac_codec, ELD_dct4_pre_sin_ptr)},
    {16,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_POST_COS_TABLE_FIELD,                 offsetof(aac_codec, ELD_dct4_post_cos_ptr)},
    {16,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_POST_SIN_TABLE_FIELD,                 offsetof(aac_codec, ELD_dct4_post_sin_ptr)},
    {32,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_PRE_COS_SYNTHESIS_TABLE_FIELD,        offsetof(aac_codec, ELD_dct4_pre_cos_synthesis_ptr)},
    {32,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_PRE_SIN_SYNTHESIS_TABLE_FIELD,        offsetof(aac_codec, ELD_dct4_pre_sin_synthesis_ptr)},
    {32,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_POST_COS_SYNTHESIS_TABLE_FIELD,       offsetof(aac_codec, ELD_dct4_post_cos_synthesis_ptr)},
    {32,   MALLOC_PREFERENCE_DM2,  AAC_ELD_DCT4_POST_SIN_SYNTHESIS_TABLE_FIELD,       offsetof(aac_codec, ELD_dct4_post_sin_synthesis_ptr)},
    {320,  MALLOC_PREFERENCE_DM2,  AAC_ELD_QMF_FILTERBANK_WINDOW_TABLE_FIELD,         offsetof(aac_codec, ELD_qmf_filterbank_window_ptr)},
    {640,  MALLOC_PREFERENCE_DM2,  AAC_ELD_QMF_FILTERBANK_WINDOW_SYNTH_TABLE_FIELD,   offsetof(aac_codec, ELD_qmf_filterbank_window_synthesis_ptr)},
#endif
};
