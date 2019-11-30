// *****************************************************************************
// Copyright (c) 2015 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_COMPANDER_C_HEADER_INCLUDED
#define AUDIO_COMPANDER_C_HEADER_INCLUDED

/*****************************************************************************
Include Files
*/
#include "mem_utils/memory_table.h"
#include "mem_utils/shared_memory_ids.h"

/****************************************************************************
Private Const Declarations
*/

/* COMPANDER data object size, in AU */
#define COMPANDER_BASE_STRUC_SIZE (sizeof(t_compander_object))
#define COMPANDER_OBJECT_SIZE     COMPANDER_BASE_STRUC_SIZE 

#define COMPANDER_HEADROOM_SHIFTS              0;



/****************************************************************************
Private Type Definitions
*/
   
typedef struct {
   unsigned int compander_config;
   unsigned int num_sections;
   unsigned int gain_ratio_section1;
   unsigned int gain_ratio_section2;
   unsigned int gain_ratio_section3;
   unsigned int gain_ratio_section4;
   unsigned int gain_ratio_section5;
   unsigned int gain_threshold_section1;
   unsigned int gain_threshold_section2;
   unsigned int gain_threshold_section3;
   unsigned int gain_threshold_section4;
   unsigned int gain_threshold_section5;
   unsigned int gain_kneewidth_section1;
   unsigned int gain_kneewidth_section2;
   unsigned int gain_kneewidth_section3;
   unsigned int gain_kneewidth_section4;
   unsigned int gain_kneewidth_section5;
   unsigned int gain_attack_tc;
   unsigned int gain_release_tc;
   unsigned int level_attack_tc;
   unsigned int level_release_tc;
   unsigned int level_average_tc;
   unsigned int makeup_gain;
   unsigned int lookahead_time;
   unsigned int level_estimation_flag;
   unsigned int gain_update_flag;
   unsigned int gain_interp_flag;
   unsigned int soft_knee_1_coeffs[3];
   unsigned int soft_knee_2_coeffs[3];
   unsigned int soft_knee_3_coeffs[3];
   unsigned int soft_knee_4_coeffs[3];
   unsigned int soft_knee_5_coeffs[3];
  
} t_compander_params;

// compander data structure    
typedef struct  {
   void* data_objects_ptr;
   unsigned int num_channels;
   unsigned int sample_rate;
   t_compander_params* params_ptr;
   void*        channel_input_ptr;
   void*        channel_output_ptr;
   unsigned int samples_to_process;
   unsigned int framebuffer_flag;
   unsigned int* input_read_addr;
   unsigned int* output_write_addr;
   unsigned int lvl_alpha_atk;
   unsigned int lvl_alpha_rls;
   unsigned int lvl_alpha_avg;
   unsigned int slope1;
   unsigned int slope2;
   unsigned int slope3;
   unsigned int slope4;
   unsigned int slope5;
   unsigned int slope6;
   unsigned int intercept1;
   unsigned int intercept2;
   unsigned int intercept3;
   unsigned int intercept4;
   unsigned int intercept5;
   unsigned int intercept6;
   unsigned int gain_update_rate;
   unsigned int gain_alpha_atk;
   unsigned int gain_alpha_rls;
   unsigned int level_detect_last_sample_hist;
   unsigned int level_detect_chn_max_or_mean;
   unsigned int level_detect_chn_max_or_mean_log2;        
   unsigned int gain_smooth_hist;
   unsigned int ptr_lookahead_hist;
   unsigned int* lookahead_hist_buf;
   unsigned int lookahead_samples;
   unsigned int num_channels_inv;
   unsigned int gain_update_rate_inv;
   unsigned int gain_smooth_hist_linear;
   unsigned int level_detect_func_ptr;
   
} t_compander_object;
  

#endif // AUDIO_COMPANDER_C_HEADER_INCLUDED
