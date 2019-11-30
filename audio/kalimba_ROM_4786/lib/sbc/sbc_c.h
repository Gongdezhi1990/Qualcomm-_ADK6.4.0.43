/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sbc_c.h
 * \ingroup sbc
 *
 * SBC library C header file. <br>
 *
 */

#ifndef SBC_C_H
#define SBC_C_H
/*****************************************************************************
Include Files
*/
#include "mem_utils/memory_table.h"
#include "mem_utils/shared_memory_ids.h"

/****************************************************************************
Private Const Declarations
*/
#define SBC_SHARED_TABLE_LENGTH (4)
#define SBC_DEC_SHARED_TABLE_LENGTH (4)
#define SBC_ENC_SHARED_TABLE_LENGTH (3)
#define SBC_DEC_DM1_SCRATCH_TABLE_LENGTH (2)
#define SBC_ENC_DM1_SCRATCH_TABLE_LENGTH (SBC_DEC_DM1_SCRATCH_TABLE_LENGTH + 2)
#define SBC_DEC_DM2_SCRATCH_TABLE_LENGTH (2)
#define SBC_ENC_DM2_SCRATCH_TABLE_LENGTH (SBC_DEC_DM2_SCRATCH_TABLE_LENGTH + 1)


#define SBC_AUDIO_SAMPLE_BUFF_SIZE 256
#define SBC_SYNTHESIS_BUFF_LENGTH 160   /* was .CONST SYNTHESIS_BUFFER_LENGTH */
#define SBC_ANALYSIS_BUFF_LENGTH 80     /* was .CONST ANALYSISS_BUFFER_LENGTH */
#define SBC_SCALE_FACTOR_LENGTH 16
#define SBC_BITS_LENGTH 16
#define SBC_BITNEED_LENGTH 16
#define SBC_ANALYSIS_Y_LENGTH 16
#define MAX_SBC_FRAME_SIZE_IN_WORDS 256

/****************************************************************************
Private Type Definitions
*/

typedef struct
{
    /* Total size 69 words */
    void *audio_sample_js;
    void *analysis_coefs_m8;
    void *analysis_coefs_m4;
    void *analysis_xch1;
    void *analysis_xch2;
    void *analysis_y;
    void *level_coefs;
    void *pre_post_proc_struc;
    unsigned enc_setting_nrof_subbands;
    unsigned enc_setting_nrof_blocks;
    unsigned enc_setting_sampling_freq;
    unsigned enc_setting_channel_mode;
    unsigned enc_setting_allocation_method;
    unsigned enc_setting_bitpool;
    unsigned enc_setting_force_word_align;
    unsigned put_nextword;
    unsigned put_bitpos;
    void *analysis_xch1ptr;
    void *analysis_xch2ptr;
    unsigned wbs_send_frame_counter;
    void *audio_sample;
    void *win_coefs_m8;
    void *win_coefs_m4;
    void *loudness_offset;
    void *scale_factor;
    void *scale_factor_js;
    void *bitneed;
    void *bits;
    unsigned sampling_freq;
    unsigned nrof_blocks;
    unsigned channel_mode;
    unsigned nrof_channels;
    unsigned allocation_method;
    unsigned nrof_subbands;
    unsigned bitpool;
    unsigned framecrc;
    unsigned crc_checksum;
    unsigned force_word_align;
    unsigned join;
    unsigned join_1;
    unsigned join_2;
    unsigned join_3;
    unsigned join_4;
    unsigned join_5;
    unsigned join_6;
    unsigned join_7;
    void *synthesis_coefs_m8;
    void *synthesis_coefs_m4;
    void *levelrecip_coefs;
    void *bitmask_lookup;
    void *synthesis_vch1;
    void *synthesis_vch2;
    unsigned convert_to_mono;
    unsigned frame_underflow;
    unsigned frame_corrupt;
    unsigned num_bytes_available;
    unsigned wbs_sbc_frame_ok;
    unsigned cur_frame_length;
    unsigned return_on_corrupt_frame;
    unsigned get_bitpos;
    void *synthesis_vch1ptr;
    void *synthesis_vch2ptr;
    void *bitslice_lookup;
    /* The last 7 of the asm structure definition fields are for WBS only*/
    void *wbs_frame_buffer_ptr;
    unsigned fields4[6];
} sbc_codec;

/*****************************************************************************
Public Constant Definitions
*/
/** memory shared between sbc enc and dec */
extern const share_malloc_t_entry sbc_shared_malloc_table[];

/** memory shared between encoders */
extern const share_malloc_t_entry sbc_enc_shared_malloc_table[];

/** memory shared between decoders */
extern const share_malloc_t_entry sbc_dec_shared_malloc_table[];

/** Scratch memory used by an encoder/decoder instance */
extern const scratch_malloc_t_entry sbc_scratch_table_dm1[];

/** Scratch memory used by an encoder/decoder instance */
extern const scratch_malloc_t_entry sbc_scratch_table_dm2[];


/** sbcdec_init_tables */
extern void sbcdec_init_tables( void* wbs_data);

/** sbcdec_reset_decoder */
extern void sbcdec_reset_decoder(void* wbs_data);

/** Initialise the sbc encoder (ASM) */
extern void sbc_encode_lib_init(void *encoder);
/** Reset the sbc encoder (ASM) */
extern void sbc_encode_lib_reset(void *encoder);

/** wbs_initialize */
extern void wbs_initialize(sbc_codec* decoder);

/** sco_decoder_wbs_initialize */
extern void sco_decoder_wbs_initialize(void* wbs_data);

extern void sbc_frame_encode(void);

#endif /* SBC_C_H */
