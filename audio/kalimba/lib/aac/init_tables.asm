/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef AAC_INIT_TABLES_INCLUDED
#define AAC_INIT_TABLES_INCLUDED

#include "aac_library.h"
#include "core_library.h"

//*****************************************************************************
// MODULE:
//    $aacdec.init_tables
//
// DESCRIPTION:
//    This function will populate all the tables in RAM with data in tables in
//    Flash/ROM needed by the AAC decoder.
//
// INPUTS:
//    - R5 pointer to decoder structure, which has valid data object with buffers
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Assume all
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.aacdec.init_tables;
   .CODESEGMENT AACDEC_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $aacdec.init_tables:

   // push rLink onto stack
   push rLink;

   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
   call $aacdec.private.init_tables;

   jump $pop_rLink_and_rts;

.ENDMODULE;


//*****************************************************************************
// *******************  PRIVATE FUNCTIONS FROM THIS POINT ON *******************
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $aacdec.private.init_tables
//
// DESCRIPTION:
//    This function will populate decoder tables in RAM with data in tables in
//    Flash/ROM
//
// INPUTS:
//    - R9 pointer to data object with buffers allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Assume All
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.aacdec.private.init_tables;
   .CODESEGMENT AACDEC_PRIVATE_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $aacdec.private.init_tables:

   // push rLink onto stack
   push rLink;

#ifndef KYMERA
   #if defined(PATCH_LIBS)
      LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.INIT_TABLES_ASM.PRIVATE.INIT_TABLES.PATCH_ID_0, r2)
   #endif
#endif


   r0 = &$aacdec.bitmask_lookup;
   r1 = 17;
   r2 = M[r9 + $aac.mem.BITMASK_LOOKUP_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.two2qtrx_lookup;
   r1 = 4;
   r2 = M[r9 + $aac.mem.TWO2QTRX_LOOKUP_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.x43_lookup1;
   r1 = 36;
   r2 = M[r9 + $aac.mem.X43_LOOKUP1_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.x43_lookup2;
   r1 = 36;
   r2 = M[r9 + $aac.mem.X43_LOOKUP2_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.x43_lookup32_flash;
   r1 = 64;
   r2 = M[r9 + $aac.mem.X43_LOOKUP32_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.tns_max_sfb_long_table;
   r1 = 9;
   r2 = M[r9 + $aac.mem.TNS_MAX_SFB_LONG_TABLE_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.tns_lookup_coefs;
   r1 = 24;
   r2 = M[r9 + $aac.mem.TNS_LOOKUP_COEFS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sample_rate_tags;
   r1 = 12;
   r2 = M[r9 + $aac.mem.SAMPLE_RATE_TAGS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ltp_coefs;
   r1 = 8;
   r2 = M[r9 + $aac.mem.LTP_COEFS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sin2048_coefs;
   r1 = 4;
   r2 = M[r9 + $aac.mem.SIN2048_COEFS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sin256_coefs;
   r1 = 4;
   r2 = M[r9 + $aac.mem.SIN256_COEFS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.kaiser2048_coefs;
   r1 = 36;
   r2 = M[r9 + $aac.mem.KAISER2048_COEFS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.kaiser256_coefs;
   r1 = 36;
   r2 = M[r9 + $aac.mem.KAISER256_COEFS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.huffman_offsets;
   r1 = 12;
   r2 = M[r9 + $aac.mem.HUFFMAN_OFFSETS_FIELD];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sin_const_mdct;
   r1 = 4;
   r2 = M[r9 + $aac.mem.sin_const_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sin_const_imdct;
   r1 = 18;
   r2 = M[r9 + $aac.mem.sin_const_iptr];
   call $aacdec.flash_copy_func;


#ifdef AACDEC_SBR_ADDITIONS
   // Copy the main block of SBR consts
   r0 = &$aacdec.sbr_small_const_arrays_flash;
   r1 = LENGTH($aacdec.sbr_small_const_arrays_flash);
   r2 = M[r9 + $aac.mem.SBR_limiter_bands_compare_ptr];
   call $aacdec.flash_copy_func_size_in_addrs;

   // Fill in the array pointers for each of the arrays of the block
   r0 = M[r9 + $aac.mem.SBR_limiter_bands_compare_ptr];
   r0 = r0 + LENGTH($aacdec.sbr_limiter_bands_compare_flash);
   M[r9 + $aac.mem.SBR_phi_re_sin_ptr]                       = r0;
   r0 = r0 + LENGTH($aacdec.sbr_phi_re_sin_flash);
   M[r9 + $aac.mem.SBR_phi_im_sin_ptr]                       = r0;
   r0 = r0 + LENGTH($aacdec.sbr_phi_im_sin_flash);
   M[r9 + $aac.mem.SBR_h_smooth_ptr]                         = r0;
   r0 = r0 + LENGTH($aacdec.sbr_h_smooth_flash);
   M[r9 + $aac.mem.SBR_goal_sb_tab_ptr]                      = r0;
   r0 = r0 + LENGTH($aacdec.sbr_goal_sb_tab_flash);
   M[r9 + $aac.mem.SBR_log2Table_ptr]                        = r0;
   r0 = r0 + LENGTH($aacdec.SBR_log2Table_flash);
   M[r9 + $aac.mem.SBR_fscale_gt_zero_temp_1_ptr]            = r0;
   r0 = r0 + LENGTH($aacdec.sbr_fscale_gt_zero_temp_1_flash);
   M[r9 + $aac.mem.SBR_fscale_gt_zero_temp_2_ptr]            = r0;
   r0 = r0 + LENGTH($aacdec.sbr_fscale_gt_zero_temp_2_flash);
   M[r9 + $aac.mem.SBR_startMinTable_ptr]                    = r0;
   r0 = r0 + LENGTH($aacdec.sbr_startMinTable_flash);
   M[r9 + $aac.mem.SBR_stopMinTable_ptr]                     = r0;
   r0 = r0 + LENGTH($aacdec.sbr_stopMinTable_flash);
   M[r9 + $aac.mem.SBR_offsetIndexTable_ptr]                 = r0;
   r0 = r0 + LENGTH($aacdec.sbr_offsetIndexTable_flash);
   M[r9 + $aac.mem.SBR_E_pan_tab_ptr]                        = r0;
   r0 = r0 + LENGTH($aacdec.sbr_E_pan_tab_flash);
   M[r9 + $aac.mem.SBR_one_over_x_ptr]                       = r0;
   r0 = r0 + LENGTH($aacdec.sbr_one_over_x_flash);
   M[r9 + $aac.mem.SBR_pow2_table_ptr]                       = r0;
   r0 = r0 + LENGTH($aacdec.sbr_pow2_table_flash);
   M[r9 + $aac.mem.SBR_qmf_stop_channel_offset_ptr]          = r0;
   r0 = r0 + LENGTH($aacdec.sbr_qmf_stop_channel_offset_flash);
   M[r9 + $aac.mem.SBR_offset_ptr]                           = r0;

   r0 = &$aacdec.sbr_est_curr_env_one_over_div_flash;
   r1 = 100;
   r2 = M[r9 + $aac.mem.SBR_est_curr_env_one_over_div_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sbr_synthesis_pre_process_real_flash;
   r1 = 64;
   r2 = M[r9 + $aac.mem.SBR_synthesis_pre_process_real_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sbr_synthesis_pre_process_imag_flash;
   r1 = 64;
   r2 = M[r9 + $aac.mem.SBR_synthesis_pre_process_imag_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sbr_synthesis_post_process_real_flash;
   r1 = 128;
   r2 = M[r9 + $aac.mem.SBR_synthesis_post_process_real_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.sbr_synthesis_post_process_imag_flash;
   r1 = 128;
   r2 = M[r9 + $aac.mem.SBR_synthesis_post_process_imag_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.QMF_filterbank_window_flash;
   r1 = 321;
   r2 = M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.dct4_64_table_flash;
   r1 = 192;
   r2 = M[r9 + $aac.mem.SBR_dct4_64_table_ptr];
   call $aacdec.flash_copy_func;
#endif



#ifdef AACDEC_PS_ADDITIONS
   // Copy the main block of PS consts
   r0 = &$aacdec.ps_small_const_arrays_flash;
   r1 = LENGTH($aacdec.ps_small_const_arrays_flash);
   r2 = M[r9 + $aac.mem.PS_num_sub_subbands_per_hybrid_qmf_subband_ptr];
   call $aacdec.flash_copy_func_size_in_addrs;

   // Fill in the array pointers for each of the arrays of the block
   r0 = M[r9 + $aac.mem.PS_num_sub_subbands_per_hybrid_qmf_subband_ptr];
   r0 = r0 + LENGTH($aacdec.ps_num_sub_subbands_per_hybrid_qmf_subband_flash);
   M[r9 + $aac.mem.PS_hybrid_qmf_sub_subband_offset_ptr]             = r0;
   r0 = r0 + LENGTH($aacdec.ps_hybrid_qmf_sub_subband_offset_flash);
   M[r9 + $aac.mem.PS_hybrid_type_b_fir_filter_coefficients_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_hybrid_type_b_fir_filter_coefficients_flash);
   M[r9 + $aac.mem.PS_nr_par_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_nr_par_table_flash);
   M[r9 + $aac.mem.PS_nr_ipdopd_par_tab_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_nr_ipdopd_par_tab_flash);
   M[r9 + $aac.mem.PS_num_env_tab_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_num_env_tab_flash);
   M[r9 + $aac.mem.PS_map_freq_bands_to_20_par_bands_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_map_freq_bands_to_20_par_bands_table_flash);
   M[r9 + $aac.mem.PS_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_flash);
   M[r9 + $aac.mem.PS_frequency_border_table_20_par_bands_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_frequency_border_table_20_par_bands_flash);
   M[r9 + $aac.mem.PS_iid_coarse_resolution_scale_factor_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_iid_coarse_resolution_scale_factor_table_flash);
   M[r9 + $aac.mem.PS_iid_fine_resolution_scale_factor_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_iid_fine_resolution_scale_factor_table_flash);
   M[r9 + $aac.mem.PS_cos_alpha_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_cos_alpha_table_flash);
   M[r9 + $aac.mem.PS_sin_alpha_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_sin_alpha_table_flash);
   M[r9 + $aac.mem.PS_alpha_angle_table_ptr]     = r0;
   r0 = r0 + LENGTH($aacdec.ps_alpha_angle_table_flash);
#endif



#ifdef AACDEC_ELD_ADDITIONS
   r0 = &$aacdec.ELD_twiddle_tab_flash;
   r1 = $aacdec.TWIDDLE_TABLE_SIZE;
   r2 = M[r9 + $aac.mem.ELD_twiddle_tab_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_pre_cos_flash;
   r1 = 16;
   r2 = M[r9 + $aac.mem.ELD_dct4_pre_cos_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_pre_sin_flash;
   r1 = 16;
   r2 = M[r9 + $aac.mem.ELD_dct4_pre_sin_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_post_cos_flash;
   r1 = 16;
   r2 = M[r9 + $aac.mem.ELD_dct4_post_cos_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_post_sin_flash;
   r1 = 16;
   r2 = M[r9 + $aac.mem.ELD_dct4_post_sin_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_pre_cos_synthesis_flash;
   r1 = 32;
   r2 = M[r9 + $aac.mem.ELD_dct4_pre_cos_synthesis_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_pre_sin_synthesis_flash;
   r1 = 32;
   r2 = M[r9 + $aac.mem.ELD_dct4_pre_sin_synthesis_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_post_cos_synthesis_flash;
   r1 = 32;
   r2 = M[r9 + $aac.mem.ELD_dct4_post_cos_synthesis_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_dct4_post_sin_synthesis_flash;
   r1 = 32;
   r2 = M[r9 + $aac.mem.ELD_dct4_post_sin_synthesis_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_qmf_filterbank_window_flash;
   r1 = 320;
   r2 = M[r9 + $aac.mem.ELD_qmf_filterbank_window_ptr];
   call $aacdec.flash_copy_func;

   r0 = &$aacdec.ELD_qmf_filterbank_window_synthesis_flash;
   r1 = 640;
   r2 = M[r9 + $aac.mem.ELD_qmf_filterbank_window_synthesis_ptr];
   call $aacdec.flash_copy_func;

#endif // AACDEC_ELD_ADDITIONS

   jump $pop_rLink_and_rts;



   // *************************************************************************
   // Description: Subroutine to copy  tables from DMCONST to RAM
   // r0 = DMCONST ADDRESS
   // r2 = RAM ADDRESS
   // r1 = number of coefficients to copy
   // *************************************************************************
   $aacdec.flash_copy_func_size_in_addrs:
   Addr2Words(r1);

   $aacdec.flash_copy_func:
   I1 = r0;
   I4 = r2;
   r10 = r1 - 1;
   r0 = M[I1, MK1];
   do copy_aac_table_to_ram;
      r0 = M[I1, MK1], M[I4, MK1] = r0;
   copy_aac_table_to_ram:
   M[I4, MK1] = r0;
   rts;

.ENDMODULE;

#endif