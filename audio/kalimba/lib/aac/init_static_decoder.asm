// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef AAC_INIT_STATIC_DECODER_INCLUDED
#define AAC_INIT_STATIC_DECODER_INCLUDED

#include "aac_library.h"


// *****************************************************************************
// MODULE:
//    $aac.init_static_decoder
//
// DESCRIPTION:
//    Initialise variables for AAC Decoding. This corresponds to init_decoder,
//    and it is the function to call when not using external dynamic tables.
//    It also takes care of message handlers that are only used in VM builds.
//
// INPUTS:
//    - r5 = pointer to decoder structure
//
// OUTPUTS:
//    - decoder data object pointer is set in the decoder structure
//
// TRASHED REGISTERS:
//    r0-r3, r9, r10, DoLoop, I0
//
// *****************************************************************************
.MODULE $M.aacdec.init_static_decoder;
   .CODESEGMENT AACDEC_INIT_STATIC_DECODER_PM;
   .DATASEGMENT DM;

   $aacdec.init_static_decoder:

   // push rLink onto stack
   push rLink;


   // Defined segments for BASE_REGISTER mode circularly aligned non BASE_REGISTER mode
#ifdef BASE_REGISTER_MODE
   #define DMCIRC_seg  DM
   #define DM1CIRC_seg DM1
   #define DM2CIRC_seg DM2
   #define DM1CIRC_SCRATCH_seg DM1_SCRATCH
   #define DM2CIRC_SCRATCH_seg DM2_SCRATCH
#else
   #define DMCIRC_seg  DMCIRC
   #define DM1CIRC_seg DM1CIRC
   #define DM2CIRC_seg DM2CIRC
   #define DM1CIRC_SCRATCH_seg DM1CIRC_SCRATCH
   #define DM2CIRC_SCRATCH_seg DM2CIRC_SCRATCH
#endif


#ifdef AACDEC_SBR_ADDITIONS
   // ***** "frame_mem_pool" *****
   // Consists of:
   // [offset]  [data]
   //      0    fmp_start[256]
   //    256    sbr_temp_1[128]
   //    384    sbr_temp_3[128]
   //    512    fmp_remains[FRAME_MEM_POOL_LENGTH - 512]
   // The layout is like this so that in non BASE_REGISTER mode the sbr_temp_x buffers are located at circular boundaries.
   // NOTE: Only once BASE_REGISTER mode is mandatory can these variables all be separated out
   .VAR/DM2CIRC_SCRATCH_seg frame_mem_pool[$aacdec.FRAME_MEM_POOL_LENGTH];
   .CONST frame_mem_pool_OFFSET_fmp_start     (0*ADDR_PER_WORD);
   .CONST frame_mem_pool_OFFSET_sbr_temp_1    (256*ADDR_PER_WORD);
   .CONST frame_mem_pool_OFFSET_sbr_temp_3    (384*ADDR_PER_WORD);
   .CONST frame_mem_pool_OFFSET_fmp_remains   (512*ADDR_PER_WORD);


   // ***** "sbr_x_imag" *****
   // Consists of:
   // [offset]  [data]
   //      0    X_sbr_2env_real[128]
   //    128    X_sbr_curr_real[384]
   //    512    tmp_mem_pool[2560]
   //   3072    sbr_temp_2[128]
   //   3200    sbr_temp_4[128]
   //   3328    tmp_mem_pool_ext[TMP_MEM_POOL_LENGTH - 2560 - 256]
   // The layout is like this so that there a temp block of 2048 with a preceeding 512 persistent block before it for
   // SBR to operate correctly (40x64).  Currently it is also blocked together like this so that in non
   // BASE_REGISTER mode the sbr_temp_x buffers are located at circular boundaries.
   // NOTE: Only once BASE_REGISTER mode is mandatory AND the SBR logic is rewritten to separate the temporary 32x64
   //       block from the persistant 8x64 block can these variables all be separated out
   .VAR/DM1CIRC_seg sbr_x_imag[$aacdec.TMP_MEM_POOL_LENGTH + 512];
   .CONST sbr_x_imag_OFFSET_X_sbr_2env_imag   (0*ADDR_PER_WORD);
   .CONST sbr_x_imag_OFFSET_X_sbr_curr_imag   (128*ADDR_PER_WORD);
   .CONST sbr_x_imag_OFFSET_tmp_mem_pool      (512*ADDR_PER_WORD);
   .CONST sbr_x_imag_OFFSET_sbr_temp_2        (3072*ADDR_PER_WORD);
   .CONST sbr_x_imag_OFFSET_sbr_temp_4        (3200*ADDR_PER_WORD);
   .CONST sbr_x_imag_OFFSET_tmp_mem_pool_ext  (3328*ADDR_PER_WORD);


   // ***** "sbr_x_real" *****
   // Consists of:
   // [offset]  [data]
   //      0    sbr_synthesis_post_process_imag[128]
   //    128    X_sbr_other_real[384]
   //    512    X_sbr_2env_real[128]
   //    640    X_sbr_curr_real[384]
   //   1024    buf_left[1024]
   //   2048    buf_right[1024]
   // The layout is like this so that there a temp block of 2048 with a preceeding 512 persistent block before it for
   // SBR to operate correctly (40x64).  Currently there is also a further preceeding 512 block of variables so that in non
   // BASE_REGISTER mode the buf_left/right can still be located at circular buffer boundaries.
   // NOTE: Only once BASE_REGISTER mode is mandatory AND the SBR logic is rewritten to separate the temporary 32x64
   //       block from the persistant 8x64 block can these variables all be separated out
   .VAR/DM2CIRC_seg sbr_x_real[3072];
   .CONST sbr_x_real_OFFSET_X_sbr_2env_real  (512*ADDR_PER_WORD);
   .CONST sbr_x_real_OFFSET_X_sbr_curr_real  (640*ADDR_PER_WORD);
   .CONST sbr_x_real_OFFSET_buf_left         (1024*ADDR_PER_WORD);
   .CONST sbr_x_real_OFFSET_buf_right        (2048*ADDR_PER_WORD);


   .VAR/DM1CIRC_seg v_buffer_left[$aacdec.SBR_N*10];
   .VAR/DM1CIRC_seg v_buffer_right[$aacdec.SBR_N*10];
   .VAR/DM1CIRC_seg x_input_buffer_left[$aacdec.X_INPUT_BUFFER_LENGTH];
   .VAR/DM1CIRC_seg x_input_buffer_right[$aacdec.X_INPUT_BUFFER_LENGTH];
   .VAR/DM1         X_sbr_other_imag [$aacdec.X_SBR_LEFTRIGHT_SIZE];
   .VAR             sbr_info[$aacdec.SBR_SIZE];
   .VAR             sbr_est_curr_env_one_over_div_ram[100];
   .VAR             dct4_64_table_ram[192];
   .VAR             sbr_small_const_arrays_ram[216];
   .VAR/DM2         sbr_synthesis_pre_process_real_ram[64];
   .VAR/DM2         sbr_synthesis_pre_process_imag_ram[64];
   .VAR/DM1         sbr_synthesis_post_process_real_ram[128];
   .VAR/DM2         QMF_filterbank_window_ram[321];


#if defined(AACDEC_SBR_HALF_SYNTHESIS) || defined(AACDEC_PS_ADDITIONS)
   .VAR synth_temp[1024];
#endif


#else
   .VAR/DM1CIRC_SCRATCH_seg  tmp_mem_pool[$aacdec.TMP_MEM_POOL_LENGTH];
   .VAR/DM2_SCRATCH          frame_mem_pool[$aacdec.FRAME_MEM_POOL_LENGTH];
   .VAR/DM2CIRC_seg          buf_left[1024];
   .VAR/DM2CIRC_seg          buf_right[1024];
#endif


#ifdef AACDEC_PS_ADDITIONS
   .VAR             ps_small_const_arrays_ram[170];
   .VAR             ps_info[518/*PS_INFO_SIZE*/];
   .VAR             ps_X_hybrid_right_imag[320];
   .VAR             ps_hybrid_allpass_feedback_buffer[240];
   .VAR             ps_qmf_allpass_feedback_buffer[480];
   .VAR/DMCIRC_seg  ps_hybrid_type_a_fir_filter_input_buffer[13];
   .VAR             ps_long_delay_band_buffer_real[168/*(PS_DECORRELATION_SHORT_DELAY_BAND - PS_DECORRELATION_NUM_ALLPASS_BANDS - 1)*PS_DECORRELATION_LONG_DELAY_IN_SAMPLES*/];
   .VAR             ps_long_delay_band_buffer_imag[168/*(PS_DECORRELATION_SHORT_DELAY_BAND - PS_DECORRELATION_NUM_ALLPASS_BANDS - 1)*PS_DECORRELATION_LONG_DELAY_IN_SAMPLES*/];
#endif


#ifdef AACDEC_ELD_ADDITIONS
   .VAR             eld_temp_u[64];
   .VAR             eld_synthesis.temp1[64];
   .VAR             eld_synthesis.temp2[128];
   .VAR             eld_synthesis.temp3[128];
   .VAR             eld_synthesis.g_w_buffer[640];
   .VAR             eld_twiddle_tab_ram[61];
   .VAR             eld_win_coefs_ram[1920];
   .VAR             eld_sin_const_ram[18];
   .VAR/DM2         eld_dct4_pre_cos_ram[16];
   .VAR/DM2         eld_dct4_pre_sin_ram[16];
   .VAR/DM2         eld_dct4_post_cos_ram[16];
   .VAR/DM2         eld_dct4_post_sin_ram[16];
   .VAR/DM2         eld_dct4_pre_cos_synthesis_ram[32];
   .VAR/DM2         eld_dct4_pre_sin_synthesis_ram[32];
   .VAR/DM2         eld_dct4_post_cos_synthesis_ram[32];
   .VAR/DM2         eld_dct4_post_sin_synthesis_ram[32];
   .VAR/DM2         eld_qmf_filterbank_window_ram[320];
   .VAR/DM2         eld_qmf_filterbank_window_synthesis_ram[640];
   .VAR/DM2CIRC_seg eld_sbr_temp_5[128];
   .VAR/DM1CIRC_seg eld_sbr_temp_6[128];
   .VAR/DM2CIRC_seg eld_sbr_temp_7[128];
   .VAR/DM1CIRC_seg eld_sbr_temp_8[128];
   .VAR             eld_ifft_re[$aacdec.POWER_OF_2_IFFT_SIZE];           // circ
   .VAR/DM2         overlap_add_left[1536];
   .VAR/DM2         overlap_add_right[1536];
#else // AACDEC_ELD_ADDITIONS
   .VAR/DM2         overlap_add_left[576];
   .VAR/DM2         overlap_add_right[576];
#endif // AACDEC_ELD_ADDITIONS

   .VAR/DMCIRC_seg  tns_fir_input_history[$aacdec.TNS_MAX_ORDER_LONG];
   .VAR/DM2         bitmask_lookup_ram[17];
   .VAR/DM2         two2qtrx_lookup_ram[4];
   .VAR             x43_lookup32_ram[64];
   .VAR/DM2         x43_lookup1_ram[36];
   .VAR/DM2         x43_lookup2_ram[36];
   .VAR/DM2         tns_max_sfb_long_table_ram[9];
   .VAR/DM2         tns_lookup_coefs_ram[24];
   .VAR/DM2         sample_rate_tags_ram[12];
   .VAR/DM2         ltp_coefs_ram[8];
   .VAR/DM2         sin2048_coefs_ram[4];
   .VAR/DM2         sin256_coefs_ram[4];
   .VAR/DM2         kaiser2048_coefs_ram[36];
   .VAR/DM2         kaiser256_coefs_ram[36];
   .VAR/DM2         huffman_offsets_ram[12];
   .VAR/DM2         sin_const_ram[4];
   .VAR/DM2         sin_const_imdct_ram[18];
   .VAR             aac_data_array[$aac.mem.STRUC_SIZE] = 0 ...;



   r9 = &aac_data_array;
   M[r5 + $codec.DECODER_DATA_OBJECT_FIELD]       = r9;
   r0 = &frame_mem_pool;
   M[r9 + $aac.mem.FRAME_MEM_POOL_END_PTR]        = r0;
   M[r9 + $aac.mem.FRAME_MEM_POOL_PTR]            = r0;

#ifdef AACDEC_SBR_ADDITIONS
   r0 = sbr_x_imag + sbr_x_imag_OFFSET_tmp_mem_pool;
#else
   r0 = &tmp_mem_pool;
#endif
   M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR]          = r0;
   M[r9 + $aac.mem.TMP_MEM_POOL_PTR]              = r0;
   r0 = &overlap_add_left;
   M[r9 + $aac.mem.OVERLAP_ADD_LEFT_PTR]          = r0;
   r0 = &overlap_add_right;
   M[r9 + $aac.mem.OVERLAP_ADD_RIGHT_PTR]         = r0;

#ifdef AACDEC_SBR_ADDITIONS
   r0 = &sbr_x_real + sbr_x_real_OFFSET_buf_left;
   r1 = &sbr_x_real + sbr_x_real_OFFSET_buf_right;
#else
   r0 = &buf_left;
   r1 = &buf_right;
#endif
   M[r9 + $aac.mem.BUF_LEFT_PTR]                  = r0;
   M[r9 + $aac.mem.BUF_RIGHT_PTR]                 = r1;
   r0 = &tns_fir_input_history;
   M[r9 + $aac.mem.TNS_FIR_INPUT_HISTORY_PTR]     = r0;
   r0 = &bitmask_lookup_ram;
   M[r9 + $aac.mem.BITMASK_LOOKUP_FIELD]          = r0;
   r0 = &two2qtrx_lookup_ram;
   M[r9 + $aac.mem.TWO2QTRX_LOOKUP_FIELD]         = r0;
   r0 = &x43_lookup1_ram;
   M[r9 + $aac.mem.X43_LOOKUP1_FIELD]             = r0;
   r0 = &x43_lookup2_ram;
   M[r9 + $aac.mem.X43_LOOKUP2_FIELD]             = r0;
   r0 = &x43_lookup32_ram;
   M[r9 + $aac.mem.X43_LOOKUP32_FIELD]            = r0;
   r0 = &tns_max_sfb_long_table_ram;
   M[r9 + $aac.mem.TNS_MAX_SFB_LONG_TABLE_FIELD]  = r0;
   r0 = &tns_lookup_coefs_ram;
   M[r9 + $aac.mem.TNS_LOOKUP_COEFS_FIELD]        = r0;
   r0 = &sample_rate_tags_ram;
   M[r9 + $aac.mem.SAMPLE_RATE_TAGS_FIELD]        = r0;
   r0 = &ltp_coefs_ram;
   M[r9 + $aac.mem.LTP_COEFS_FIELD]               = r0;
   r0 = &sin2048_coefs_ram;
   M[r9 + $aac.mem.SIN2048_COEFS_FIELD]           = r0;
   r0 = &sin256_coefs_ram;
   M[r9 + $aac.mem.SIN256_COEFS_FIELD]            = r0;
   r0 = &kaiser2048_coefs_ram;
   M[r9 + $aac.mem.KAISER2048_COEFS_FIELD]        = r0;
   r0 = &kaiser256_coefs_ram;
   M[r9 + $aac.mem.KAISER256_COEFS_FIELD]         = r0;
   r0 = &huffman_offsets_ram;
   M[r9 + $aac.mem.HUFFMAN_OFFSETS_FIELD]         = r0;
   r0 = &sin_const_ram;
   M[r9 + $aac.mem.sin_const_ptr]                 = r0;
   r0 = &sin_const_imdct_ram;
   M[r9 + $aac.mem.sin_const_iptr]                = r0;

   M[r9 + $aac.mem.READ_FRAME_FUNCTION]           = 0;


#ifdef AACDEC_SBR_ADDITIONS
   r0 = &sbr_small_const_arrays_ram;
   M[r9 + $aac.mem.SBR_limiter_bands_compare_ptr]       = r0;
   r0 = &sbr_x_real;
   M[r9 + $aac.mem.SBR_x_real_ptr]                      = r0;
   M[r9 + $aac.mem.SBR_synthesis_post_process_imag_ptr] = r0;
   r0 = &sbr_x_imag;
   M[r9 + $aac.mem.SBR_x_imag_ptr]                      = r0;

   r0 = &sbr_x_imag + sbr_x_imag_OFFSET_X_sbr_2env_imag;
   M[r9 + $aac.mem.SBR_X_2env_imag_ptr]                         = r0;
   r0 = &sbr_x_imag + sbr_x_imag_OFFSET_X_sbr_curr_imag;
   M[r9 + $aac.mem.SBR_X_curr_imag_ptr]                         = r0;

   r0 = &sbr_x_real + sbr_x_real_OFFSET_X_sbr_2env_real;
   M[r9 + $aac.mem.SBR_X_2env_real_ptr]                         = r0;
   r0 = &sbr_x_real + sbr_x_real_OFFSET_X_sbr_curr_real;
   M[r9 + $aac.mem.SBR_X_curr_real_ptr]                         = r0;

   r0 = &v_buffer_left;
   M[r9 + $aac.mem.SBR_v_buffer_left_ptr]                       = r0;
   r0 = &v_buffer_right;
   M[r9 + $aac.mem.SBR_v_buffer_right_ptr]                      = r0;
   r0 = &sbr_x_real + (128*ADDR_PER_WORD);
   M[r9 + $aac.mem.SBR_X_sbr_other_real_ptr]                    = r0;
   r0 = &X_sbr_other_imag;
   M[r9 + $aac.mem.SBR_X_sbr_other_imag_ptr]                    = r0;
   #if defined(AACDEC_SBR_HALF_SYNTHESIS) || defined(AACDEC_PS_ADDITIONS)
      r0 = &synth_temp;
      M[r9 + $aac.mem.SBR_synth_temp_ptr]                       = r0;
   #endif
   r0 = &sbr_synthesis_post_process_real_ram;
   M[r9 + $aac.mem.SBR_synthesis_post_process_real_ptr]         = r0;
   r0 = QMF_filterbank_window_ram;
   M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr]               = r0;
   r0 = &sbr_synthesis_pre_process_real_ram;
   M[r9 + $aac.mem.SBR_synthesis_pre_process_real_ptr]          = r0;
   r0 = &sbr_synthesis_pre_process_imag_ram;
   M[r9 + $aac.mem.SBR_synthesis_pre_process_imag_ptr]          = r0;
   r0 = &sbr_est_curr_env_one_over_div_ram;
   M[r9 + $aac.mem.SBR_est_curr_env_one_over_div_ptr]           = r0;
   r0 = &x_input_buffer_left;
   M[r9 + $aac.mem.SBR_x_input_buffer_left_ptr]                 = r0;
   r0 = &x_input_buffer_right;
   M[r9 + $aac.mem.SBR_x_input_buffer_right_ptr]                = r0;
   r0 = &dct4_64_table_ram;
   M[r9 + $aac.mem.SBR_dct4_64_table_ptr]                       = r0;
   r0 = &frame_mem_pool + frame_mem_pool_OFFSET_sbr_temp_1;
   M[r9 + $aac.mem.SBR_temp_1_ptr]                              = r0;
   r0 = &sbr_x_imag + sbr_x_imag_OFFSET_sbr_temp_2;
   M[r9 + $aac.mem.SBR_temp_2_ptr]                              = r0;
   r0 = &frame_mem_pool + frame_mem_pool_OFFSET_sbr_temp_3;
   M[r9 + $aac.mem.SBR_temp_3_ptr]                              = r0;
   r0 = &sbr_x_imag + sbr_x_imag_OFFSET_sbr_temp_4;
   M[r9 + $aac.mem.SBR_temp_4_ptr]                              = r0;
   r0 = &sbr_info;
   M[r9 + $aac.mem.SBR_info_ptr]                                = r0;
#endif



#ifdef AACDEC_PS_ADDITIONS
   r0 = &ps_small_const_arrays_ram;
   M[r9 + $aac.mem.PS_num_sub_subbands_per_hybrid_qmf_subband_ptr] = r0;
   r0 = ps_info;
   M[r9 + $aac.mem.PS_info_ptr]                                    = r0;
   r0 = &frame_mem_pool + frame_mem_pool_OFFSET_fmp_remains;
   M[r9 + $aac.mem.PS_fmp_remains_ptr]                             = r0;
   r0 = ps_hybrid_allpass_feedback_buffer;
   M[r9 + $aac.mem.PS_hybrid_allpass_feedback_buffer_ptr]          = r0;
   r0 = ps_qmf_allpass_feedback_buffer;
   M[r9 + $aac.mem.PS_qmf_allpass_feedback_buffer_ptr]             = r0;
   r0 = ps_hybrid_type_a_fir_filter_input_buffer;
   M[r9 + $aac.mem.PS_hybrid_type_a_fir_filter_input_buffer_ptr]   = r0;
   r0 = ps_long_delay_band_buffer_real;
   M[r9 + $aac.mem.PS_long_delay_band_buffer_real_ptr]             = r0;
   r0 = ps_long_delay_band_buffer_imag;
   M[r9 + $aac.mem.PS_long_delay_band_buffer_imag_ptr]             = r0;
   r0 = &synth_temp;
   M[r9 + $aac.mem.PS_X_hybrid_real_address + (0*ADDR_PER_WORD)]   = r0;
   r1 = r0 + (2*($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2) * $aacdec.PS_NUM_SAMPLES_PER_FRAME  * ADDR_PER_WORD);
   M[r9 + $aac.mem.PS_X_hybrid_real_address + (1*ADDR_PER_WORD)]   = r1;
   r1 = r0 + (($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2) * $aacdec.PS_NUM_SAMPLES_PER_FRAME * ADDR_PER_WORD);
   M[r9 + $aac.mem.PS_X_hybrid_imag_address + (0*ADDR_PER_WORD)]   = r1;
   r1 = ps_X_hybrid_right_imag;
   M[r9 + $aac.mem.PS_X_hybrid_imag_address + (1*ADDR_PER_WORD)]   = r1;

#endif


#ifdef AACDEC_ELD_ADDITIONS
   r0 = &eld_ifft_re;
   M[r9 + $aac.mem.ELD_ifft_re_ptr]                         = r0;
   r0 = eld_temp_u;
   M[r9 + $aac.mem.ELD_temp_u_ptr]                          = r0;
   r0 = eld_synthesis.temp1;
   M[r9 + $aac.mem.ELD_synthesis_temp1_ptr]                 = r0;
   r0 = eld_synthesis.temp2;
   M[r9 + $aac.mem.ELD_synthesis_temp2_ptr]                 = r0;
   r0 = eld_synthesis.temp3;
   M[r9 + $aac.mem.ELD_synthesis_temp3_ptr]                 = r0;
   r0 = eld_synthesis.g_w_buffer;
   M[r9 + $aac.mem.ELD_synthesis_g_w_buffer_ptr]            = r0;
   r0 = eld_twiddle_tab_ram;
   M[r9 + $aac.mem.ELD_twiddle_tab_ptr]                     = r0;
   r0 = eld_win_coefs_ram;
   M[r9 + $aac.mem.ELD_win_coefs_ptr]                       = r0;
   r0 = eld_sin_const_ram;
   M[r9 + $aac.mem.ELD_sin_const_ptr]                       = r0;
   r0 = eld_sbr_temp_5;
   M[r9 + $aac.mem.ELD_sbr_temp_5_ptr]                      = r0;
   r0 = eld_sbr_temp_6;
   M[r9 + $aac.mem.ELD_sbr_temp_6_ptr]                      = r0;
   r0 = eld_sbr_temp_7;
   M[r9 + $aac.mem.ELD_sbr_temp_7_ptr]                      = r0;
   r0 = eld_sbr_temp_8;
   M[r9 + $aac.mem.ELD_sbr_temp_8_ptr]                      = r0;
   r0 = eld_dct4_pre_cos_ram;
   M[r9 + $aac.mem.ELD_dct4_pre_cos_ptr]                    = r0;
   r0 = eld_dct4_pre_sin_ram;
   M[r9 + $aac.mem.ELD_dct4_pre_sin_ptr]                    = r0;
   r0 = eld_dct4_post_cos_ram;
   M[r9 + $aac.mem.ELD_dct4_post_cos_ptr]                   = r0;
   r0 = eld_dct4_post_sin_ram;
   M[r9 + $aac.mem.ELD_dct4_post_sin_ptr]                   = r0;
   r0 = eld_dct4_pre_cos_synthesis_ram;
   M[r9 + $aac.mem.ELD_dct4_pre_cos_synthesis_ptr]          = r0;
   r0 = eld_dct4_pre_sin_synthesis_ram;
   M[r9 + $aac.mem.ELD_dct4_pre_sin_synthesis_ptr]          = r0;
   r0 = eld_dct4_post_cos_synthesis_ram;
   M[r9 + $aac.mem.ELD_dct4_post_cos_synthesis_ptr]         = r0;
   r0 = eld_dct4_post_sin_synthesis_ram;
   M[r9 + $aac.mem.ELD_dct4_post_sin_synthesis_ptr]         = r0;
   r0 = eld_qmf_filterbank_window_ram;
   M[r9 + $aac.mem.ELD_qmf_filterbank_window_ptr]           = r0;
   r0 = eld_qmf_filterbank_window_synthesis_ram;
   M[r9 + $aac.mem.ELD_qmf_filterbank_window_synthesis_ptr] = r0;
#endif // AACDEC_ELD_ADDITIONS

   call $aacdec.init_decoder;

   jump $pop_rLink_and_rts;

.ENDMODULE;
#endif
