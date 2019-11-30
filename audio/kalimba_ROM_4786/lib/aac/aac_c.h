/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

/**
 * \file  aac_c.h
 * \ingroup aac
 *
 * AAC library C header file. <br>
 *
 */

#ifndef AAC_C_H
#define AAC_C_H
/*****************************************************************************
Include Files
*/
#include "mem_utils/memory_table.h"
#include "mem_utils/shared_memory_ids.h"
#include "codec_c.h"

/****************************************************************************
Private Const Declarations
*/

/*
 Note: These are all lengths in words below

 * Max encoded frame size given by AAC-LC 48KHz, stereo, @576kbps
 * -> 1536byte frame -> 16-bit unpacked -> 768 words
 */
#define MAX_AAC_FRAME_SIZE_IN_WORDS                768

#define AAC_AUDIO_SAMPLE_BUFF_SIZE                 1536
#define AAC_FRAME_BUF_LENGTH                       1024

#ifdef AACDEC_PS_ADDITIONS
    #define AAC_TMP_MEM_POOL_LENGTH                3008
#else
#ifdef AACDEC_SBR_ADDITIONS
    #define AAC_TMP_MEM_POOL_LENGTH                2844
#else
    #define AAC_TMP_MEM_POOL_LENGTH                2504
#endif
#endif


#ifdef AACDEC_PS_ADDITIONS
    #define AAC_FRAME_MEM_POOL_LENGTH              2944
#else
    #define AAC_FRAME_MEM_POOL_LENGTH              1696
#endif


#define AAC_TNS_INPUT_HISTORY_LENGTH               12
#define AAC_TOTAL_TABLE_LENGTH                     260

#ifdef AACDEC_ELD_ADDITIONS
    #define AAC_OVERLAP_ADD_LENGTH                 1536
#else
    #define AAC_OVERLAP_ADD_LENGTH                 576
#endif


/* For SBR */
#define AAC_DEC_SBR_X_IMAG_LENGTH                  3520  /* big enough for PS also */
#define AAC_DEC_SBR_X_REAL_LENGTH                  3072
#define AAC_DEC_SBR_X_INPUT_BUF_LENGTH             320
#define AAC_DEC_SBR_V_BUF_LENGTH                   1280
#define AAC_DEC_SBR_SYNTH_TMP_LENGTH               1024
#define AAC_DEC_SBR_INFO_LENGTH                    1185
#define AAC_DEC_SBR_OTHER_IMAG_LENGTH              384

#ifdef AACDEC_SBR_ADDITIONS
    #define AAC_DEC_SHARED_TABLE_LENGTH            21
#else
    #define AAC_DEC_SHARED_TABLE_LENGTH            16
#endif


// For PS
#define AAC_DEC_PS_INFO_LENGTH                     518
#define AAC_DEC_PS_X_HYBRID_RIGHT_IMAG_LENGTH      320
#define AAC_DEC_PS_HYBRID_ALLPASS_BUFFER_LENGTH    240
#define AAC_DEC_PS_QMF_ALLPASS_BUFFER_LENGTH       480
#define AAC_DEC_PS_HYBRID_TYPE_A_BUFFER_LENGTH     13
#define AAC_DEC_PS_LONG_DELAY_BUFFER_LENGTH        168


// For ELD
#define AAC_DEC_ELD_TEMP_U_LENGTH                  64
#define AAC_DEC_ELD_SYNTH_TEMP1_LENGTH             64
#define AAC_DEC_ELD_SYNTH_TEMP2_LENGTH             128
#define AAC_DEC_ELD_SYNTH_TEMP3_LENGTH             128
#define AAC_DEC_ELD_GW_BUF_LENGTH                  640
#define AAC_DEC_ELD_SBR_TEMP_LENGTH                128
#define AAC_DEC_ELD_IFFT_RE_LENGTH                 16




/****************************************************************************
Private Type Definitions
*/

typedef struct
{
    unsigned int num_bytes_available;
    unsigned int convert_mono_to_stereo_field;
    unsigned int read_frame_function;
    unsigned int skip_function;
    unsigned int skip_amount_ms;
    unsigned int skip_amount_ls;
    unsigned int frame_underflow;
    unsigned int frame_version;
    unsigned int frame_length;
    unsigned int no_raw_data_blocks_in_frame;
    unsigned int protection_absent;
    unsigned int id3_skip_num_bytes;
    unsigned int frame_corrupt;
    unsigned int possible_frame_corruption;
    unsigned int frame_corrupt_errors;
    unsigned int lostsync_errors;
    unsigned int frame_count;
    unsigned int frame_garbage_errors;
    unsigned int get_bitpos;
    unsigned int read_bit_count;
    unsigned int frame_num_bits_avail;
    unsigned int getbits_saved_i0;
    unsigned int getbits_saved_l0;
    unsigned int getbits_saved_bitpos;
    unsigned int sf_index_field;
    unsigned int channel_configuration_field;
    unsigned int audio_object_type_field;
    unsigned int extension_audio_object_type_field;
    unsigned int sbr_present_field;
    unsigned int latm_audio_mux_version;
    unsigned int latm_audio_mux_version_a;
    unsigned int latm_mux_slot_length_bytes;
    unsigned int latm_current_subframe;
    unsigned int latm_num_subframes;
    unsigned int latm_prevbitpos;
    unsigned int latm_tarabufferfullnesss;
    unsigned int latm_asc_len;
    unsigned int latm_latm_buffer_fullness;
    unsigned int latm_other_data_len_bits;
    unsigned int channel_count;
    unsigned int mp4_moov_atom_size_ms;
    unsigned int mp4_moov_atom_size_ls;
    unsigned int mp4_sequence_flags_initialised;
    unsigned int mp4_discard_amount_ms;
    unsigned int mp4_discard_amount_ls;
    unsigned int found_first_mdat;
    unsigned int found_moov;
    unsigned int mdat_size[3];
    unsigned int sample_count[2];
    unsigned int mdat_processed;
    unsigned int temp_bit_count;
    unsigned int stsz_offset[2];
    unsigned int stss_offset[2];
    unsigned int ff_rew_skip_amount[2];
    unsigned int mp4_frame_count;
    unsigned int mp4_ff_rew_status;
    unsigned int mdat_offset [2];
    unsigned int mp4_file_offset[2];
    unsigned int fast_fwd_samples_ms;
    unsigned int fast_fwd_samples_ls;
    unsigned int avg_bit_rate;
    unsigned int mp4_decoding_started;
    unsigned int mp4_header_parsed;
    unsigned int mp4_in_moov;
    unsigned int mp4_in_discard_atom_data;
    unsigned int mdct_information[5];
    unsigned int imdct_info[5];
    unsigned int amount_unpacked;
    unsigned int common_window;
    unsigned int current_ics_ptr;
    unsigned int current_spec_ptr;
    unsigned int current_channel;
    unsigned int ics_left[42];
    unsigned int ics_right[42];
    unsigned int num_sces;
    unsigned int num_cpes;
    unsigned int tmp[40];
    unsigned int codec_struc;
    unsigned int ics_info_done_field;
    unsigned int left_spec_blksigndet[2];
    unsigned int right_spec_blksigndet[2];
    unsigned int *current_spec_blksigndet_ptr;
    unsigned int temp_i5;
    unsigned int saved_i0;
    unsigned int saved_bitpos;
    unsigned int *frame_mem_pool_end_ptr;
    unsigned int *tmp_mem_pool_end_ptr;
    unsigned int *buf_left_ptr;
    unsigned int *buf_right_ptr;
    unsigned int *overlap_add_left_ptr;
    unsigned int *overlap_add_right_ptr;
    unsigned int *frame_mem_pool_ptr;
    unsigned int *tmp_mem_pool_ptr;
    unsigned int *tns_fir_input_history_ptr;
    unsigned int *bitmask_lookup_field;
    unsigned int write_bytepos;
    unsigned int pns_rand_num;
    unsigned int previous_window_shape[2];
    unsigned int previous_window_sequence[2];
    unsigned int swb_offset_field[52];
   /* tables */
    unsigned int *two2qtrx_lookup_field;
    unsigned int *x43_lookup1_field;
    unsigned int *x43_lookup2_field;
    unsigned int *x43_lookup32_field;
    unsigned int *tns_max_sfb_long_table_field;
    unsigned int *tns_lookup_coefs_field;
    unsigned int *sample_rate_tags_field;
    unsigned int *ltp_coefs_field;
    unsigned int *sin2048_coefs_field;
    unsigned int *sin256_coefs_field;
    unsigned int *kaiser2048_coefs_field;
    unsigned int *kaiser256_coefs_field;
    unsigned int *huffman_offsets_field;
    unsigned int syntatic_element_func_table[8];
    unsigned int read_frame_func_table[3];
    unsigned int mp4_ff_rew_state;
    unsigned int moov_size_lo;
    unsigned int moov_size_hi;
    unsigned int huffman_cb_table[12];
    unsigned int huffman_packed_list[12];
    unsigned int huffman_table_sizes_div_2[12];
    unsigned int set_mp4_file_type_message_struc[4];
    unsigned int saved_current_subframe;
    unsigned int *sin_const_ptr;
    unsigned int *sin_const_iptr;
    unsigned int fft_table_init_done;

#ifdef AACDEC_SBR_ADDITIONS
    unsigned int  SBR_limiter_band_g_boost_mantissa[6];
    unsigned int  SBR_limiter_band_g_boost_exponent[6];
    unsigned int  SBR_in_synth;
    unsigned int  SBR_in_synth_loops;
    unsigned int  SBR_dct_dst;
    unsigned int  SBR_fft_pointer_struct[3];
    unsigned int  SBR_x_input_buffer_write_pointers[2];
    unsigned int  SBR_v_cbuffer_struc_address[2];
    unsigned int  SBR_v_left_cbuffer_struc[14];
    unsigned int  SBR_v_right_cbuffer_struc[14];
    unsigned int *SBR_limiter_bands_compare_ptr;
    unsigned int *SBR_phi_re_sin_ptr;
    unsigned int *SBR_phi_im_sin_ptr;
    unsigned int *SBR_h_smooth_ptr;
    unsigned int *SBR_goal_sb_tab_ptr;
    unsigned int *SBR_log2Table_ptr;
    unsigned int *SBR_fscale_gt_zero_temp_1_ptr;
    unsigned int *SBR_fscale_gt_zero_temp_2_ptr;
    unsigned int *SBR_startMinTable_ptr;
    unsigned int *SBR_stopMinTable_ptr;
    unsigned int *SBR_offsetIndexTable_ptr;
    unsigned int *SBR_E_pan_tab_ptr;
    unsigned int *SBR_one_over_x_ptr;
    unsigned int *SBR_pow2_table_ptr;
    unsigned int *SBR_qmf_stop_channel_offset_ptr;
    unsigned int *SBR_offset_ptr;
    unsigned int  SBR_bs_frame_class[2];
    unsigned int  SBR_bs_num_env[2];
    unsigned int  SBR_bs_pointer[2];
    unsigned int  SBR_bs_invf_mode[10];
    unsigned int  SBR_bs_add_harmonic_flag[2];
    unsigned int  SBR_t_E[12];
    unsigned int  SBR_t_Q[6];
    unsigned int *SBR_E_orig_mantissa_base_ptr[2];
    unsigned int  SBR_Q_orig[20];
    unsigned int  SBR_Q_orig2[20];
    unsigned int  SBR_bs_transient_position[2];
    unsigned int *SBR_x_real_ptr;
    unsigned int *SBR_x_imag_ptr;
    unsigned int *SBR_v_buffer_right_ptr;
    unsigned int *SBR_X_sbr_other_real_ptr;
    unsigned int *SBR_X_sbr_other_imag_ptr;
    unsigned int *SBR_synth_temp_ptr;
    unsigned int *SBR_synthesis_post_process_real_ptr;
    unsigned int *SBR_synthesis_post_process_imag_ptr;
    unsigned int *SBR_qmf_filterbank_window_ptr;
    unsigned int *SBR_synthesis_pre_process_real_ptr;
    unsigned int *SBR_synthesis_pre_process_imag_ptr;
    unsigned int *SBR_est_curr_env_one_over_div_ptr;
    unsigned int *SBR_x_input_buffer_left_ptr;
    unsigned int *SBR_x_input_buffer_right_ptr;
    unsigned int *SBR_dct4_64_table_ptr;
    unsigned int *SBR_temp_1_ptr;
    unsigned int *SBR_temp_2_ptr;
    unsigned int *SBR_temp_3_ptr;
    unsigned int *SBR_temp_4_ptr;
    unsigned int *SBR_info_ptr;
    unsigned int *SBR_x_input_buffer_write_base_pointers[2];
    unsigned int  SBR_numTimeSlotsRate;
    unsigned int  SBR_numTimeSlotsRate_adjusted;
    unsigned int *SBR_X_2env_real_ptr;
    unsigned int *SBR_X_2env_imag_ptr;
    unsigned int *SBR_X_curr_real_ptr;
    unsigned int *SBR_X_curr_imag_ptr;
    unsigned int *SBR_v_buffer_left_ptr;
#endif

#ifdef AACDEC_PS_ADDITIONS
    unsigned int *PS_num_sub_subbands_per_hybrid_qmf_subband_ptr;
    unsigned int *PS_hybrid_qmf_sub_subband_offset_ptr;
    unsigned int *PS_hybrid_type_b_fir_filter_coefficients_ptr;
    unsigned int *PS_nr_par_table_ptr;
    unsigned int *PS_nr_ipdopd_par_tab_ptr;
    unsigned int *PS_num_env_tab_ptr;
    unsigned int *PS_map_freq_bands_to_20_par_bands_table_ptr;
    unsigned int *PS_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_ptr;
    unsigned int *PS_frequency_border_table_20_par_bands_ptr;
    unsigned int *PS_iid_coarse_resolution_scale_factor_table_ptr;
    unsigned int *PS_iid_fine_resolution_scale_factor_table_ptr;
    unsigned int *PS_cos_alpha_table_ptr;
    unsigned int *PS_sin_alpha_table_ptr;
    unsigned int *PS_alpha_angle_table_ptr;
    unsigned int  PS_present;
    unsigned int *PS_X_hybrid_real_address[2];
    unsigned int *PS_X_hybrid_imag_address[2];
    unsigned int  PS_hybrid_type_a_ifft_struc[3];
    unsigned int  PS_iid_index_prev[34];
    unsigned int  PS_icc_index_prev[34];
    unsigned int  PS_h11_previous_envelope[22];
    unsigned int  PS_h21_previous_envelope[22];
    unsigned int  PS_h12_previous_envelope[22];
    unsigned int  PS_h22_previous_envelope[22];
    unsigned int  PS_time_history_real[36];
    unsigned int  PS_time_history_imag[36];
    unsigned int  PS_power_peak_decay_nrg_prev[40];
    unsigned int  PS_power_smoothed_peak_decay_diff_nrg_prev[40];
    unsigned int  PS_smoothed_input_power_prev[40];
    unsigned int  PS_prev_frame_last_two_hybrid_samples_real[40];
    unsigned int  PS_prev_frame_last_two_hybrid_samples_imag[40];
    unsigned int  PS_prev_frame_last_two_qmf_samples_real[40];
    unsigned int  PS_prev_frame_last_two_qmf_samples_imag[40];
    unsigned int  PS_short_delay_band_buffer_real[30];
    unsigned int  PS_short_delay_band_buffer_imag[30];
    unsigned int *PS_info_ptr;
    unsigned int *PS_fmp_remains_ptr;
    unsigned int *PS_hybrid_allpass_feedback_buffer_ptr;
    unsigned int *PS_qmf_allpass_feedback_buffer_ptr;
    unsigned int *PS_hybrid_type_a_fir_filter_input_buffer_ptr;
    unsigned int *PS_long_delay_band_buffer_real_ptr;
    unsigned int *PS_long_delay_band_buffer_imag_ptr;
#endif

#ifdef AACDEC_ELD_ADDITIONS
    unsigned int *ELD_temp_u_ptr;
    unsigned int *ELD_synthesis_temp1_ptr;
    unsigned int *ELD_synthesis_temp2_ptr;
    unsigned int *ELD_synthesis_temp3_ptr;
    unsigned int *ELD_synthesis_g_w_buffer_ptr;
    unsigned int  ELD_frame_length_flag;
    unsigned int  ELD_sbr_sampling_rate;
    unsigned int  ELD_sbr_crc_flag;
    unsigned int  ELD_delay_shift;
    unsigned int  ELD_sbr_numTimeSlots;
    unsigned int *ELD_twiddle_tab_ptr;
    unsigned int *ELD_win_coefs_ptr;
    unsigned int *ELD_sin_const_ptr;
    unsigned int *ELD_sbr_temp_5_ptr;
    unsigned int *ELD_sbr_temp_6_ptr;
    unsigned int *ELD_sbr_temp_7_ptr;
    unsigned int *ELD_sbr_temp_8_ptr;
    unsigned int *ELD_dct4_pre_cos_ptr;
    unsigned int *ELD_dct4_pre_sin_ptr;
    unsigned int *ELD_dct4_post_cos_ptr;
    unsigned int *ELD_dct4_post_sin_ptr;
    unsigned int *ELD_dct4_pre_cos_synthesis_ptr;
    unsigned int *ELD_dct4_pre_sin_synthesis_ptr;
    unsigned int *ELD_dct4_post_cos_synthesis_ptr;
    unsigned int *ELD_dct4_post_sin_synthesis_ptr;
    unsigned int *ELD_qmf_filterbank_window_ptr;
    unsigned int *ELD_qmf_filterbank_window_synthesis_ptr;
    unsigned int  ELD_sbr_present;
    unsigned int *ELD_ifft_re_ptr;
#endif
} aac_codec;

/** memory shared between decoders */
#ifdef AACDEC_ELD_ADDITIONS
   extern const share_malloc_t_entry aac_dec_shared_malloc_table[37];
#else
#ifdef AACDEC_PS_ADDITIONS
   extern const share_malloc_t_entry aac_dec_shared_malloc_table[24];
#else
#ifdef AACDEC_SBR_ADDITIONS
   extern const share_malloc_t_entry aac_dec_shared_malloc_table[23];
#else
   extern const share_malloc_t_entry aac_dec_shared_malloc_table[16];
#endif
#endif
#endif

/** to be kept in sync with $aac.dummy_decode.STRUC - see
    samples_in_packet.asm for a detailed description of fields  */
typedef struct
{
    DECODER         *codec_struc;         /* I */
    tCbuffer        *in_cbuffer;          /* I */
    unsigned int    payload_size;         /* I */
    void            *frame_dec_struc;     /* O */
    unsigned int    bit_position;         /* I/O */
    tCbuffer        *cbuff_clone;         /* Status */
    unsigned int    payload_left;         /* Status */
} dummy_decoder;

/** memory shared between decoders */
extern const share_malloc_t_entry aac_dec_shared_malloc_table[];

/****************************************************************************
Public Function Declarations
*/

/**
 * Calculates the number of audio samples contained in a payload.
 *
 * \param dec_str pointer to a dummy_decoder structure.
 * \return none
 */
extern void aacdec_samples_in_packet(dummy_decoder *dec_str);

/**
 * Calculates the number of audio samples contained in a LATM(AAC-LC) payload.
 *
 * \param dec_str pointer to a dummy_decoder structure.
 * \return none
 */
extern void aacdec_samples_in_packet_lc(dummy_decoder *dec_str);

#endif /* AAC_C_H */
