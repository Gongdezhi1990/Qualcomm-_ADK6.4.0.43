// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
//#include "core_library.h"

#ifdef AACDEC_PS_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.variables
//
// DESCRIPTION:
//    Variables
//
// *****************************************************************************
.MODULE $aacdec;
   .DATASEGMENT DM;


   /* -- SBR small constants:  pool into a single combined block for code size efficiency -- */
   .BLOCK/DMCONST ps_small_const_arrays_flash;

      .VAR ps_num_sub_subbands_per_hybrid_qmf_subband_flash[] = 6, 2, 2;

      .VAR ps_hybrid_qmf_sub_subband_offset_flash[] = 0, 6, 8;

      .VAR ps_hybrid_type_b_fir_filter_coefficients_flash[] = 0.01899487526049, -0.07293139167538, 0.30596630545168;

      // number of parameter bands for IID and ICC information : indexed by PS_IID_MODE and PS_ICC_MODE respectively
      .VAR ps_nr_par_table_flash[] = 10, 20, 34, 10, 20, 34, 0, 0;

      // number of parameter bands for IPDOPD information : indexed by PS_IID_MODE = PS_IPD_MODE
      .VAR ps_nr_ipdopd_par_tab_flash[] = 5, 11, 17, 5, 11, 17, 0, 0;

       // number of parametric stereo envelopes that the 32 element time-frame is split into
       // indexed by PS_FRAME_CLASS and PS_NUM_ENV_INDEX
      .VAR ps_num_env_tab_flash[] = 0, 1, 2, 4, 1, 2, 3, 4;

      .VAR ps_map_freq_bands_to_20_par_bands_table_flash[] =
                            1, 0, 0, 1, 2, 3,  // QMF channel_0 = 6 hybrid sub-subbands
                            4,             5,  // QMF channel_1 = 2 hybrid sub-subbands
                            6,             7,  // QMF channel_2 = 2 hybrid sub-subbands
                            8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19;

      .VAR ps_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_flash[] =
                            32, 0, 0, 32, 64, 96,  // QMF channel_0 = 6 hybrid sub-subbands
                            128,             160,  // QMF channel_1 = 2 hybrid sub-subbands
                            192,             224,  // QMF channel_2 = 2 hybrid sub-subbands
                            256, 288, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608;


      .VAR ps_frequency_border_table_20_par_bands_flash[] =
                            // Low Frequency section
                            6, 7, 0, 1, 2, 3, // 6 hybrid sub-subbands +
                            9, 8,             // 2 hybrid sub-subbands +
                            10,11,            // 2 hybrid sub-\subbands - 3 = 7 extra frequencies intoduced by hybrid analysis
                            // High Frequency section : qmf_channel_number = sub-subband index - num_of_extra_freqs
                            10-7, 11-7, 12-7, 13-7, 14-7, 15-7, 16-7, 18-7, 21-7, 25-7, 30-7, 42-7, 71-7;

      .VAR ps_iid_coarse_resolution_scale_factor_table_flash[] =
              0.998422594908967, 0.992168508372590, 0.980669920865967, 0.953462612844232, 0.913051202983263, 0.845726164365877, 0.783030535530853,
              0.707106781186547, 0.621983254865676, 0.533617118572725, 0.407844979956312, 0.301511347115414, 0.195669378418107, 0.124906610995573,
              0.056145429030366;

      .VAR ps_iid_fine_resolution_scale_factor_table_flash[] =
              0.999995009542206, 0.999984219941253, 0.999949996750861, 0.999841932237787, 0.999500374135919, 0.998422594908967, 0.996860042647559,
              0.993764188091722, 0.987672366863856, 0.975844865385273, 0.953462612844232, 0.929081841143710, 0.894002260435438, 0.845726164365877,
              0.783030535530853, 0.707106781186547, 0.621983254865676, 0.533617118572725, 0.448062514290066, 0.369874167606096, 0.301511347115414,
              0.218464459272730, 0.156535513227950, 0.111502174562231, 0.079183407428288, 0.056145429030366, 0.031606978033073, 0.017779982577601,
              0.009999499846593, 0.005623324094457, 0.003162261923507;

      .VAR ps_cos_alpha_table_flash[] =
               1.0000000000, 0.9841239700, 0.9594738210, 0.8946843079, 0.8269340931, 0.7071067812, 0.4533210856, 0.0000000000;

      .VAR ps_sin_alpha_table_flash[] =
               0.0000000000, 0.1774824264, 0.2817977763, 0.4466989918, 0.5622988580, 0.7071067812, 0.8913472911, 1.0000000000;

      .VAR ps_alpha_angle_table_flash[] =
               0.0, 0.089213818581134, 0.142833665101547, 0.231536180345483, 0.298581576069109, 0.392699081698724, 0.550154297180687, 0.785398163397449;

   .ENDBLOCK;

   #include "ps_decorrelation_tables_flash.asm"
   #include "ps_huffman_tables_flash.asm"

.ENDMODULE;

#endif

