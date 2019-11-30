// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_VSE_C_HEADER_INCLUDED
#define AUDIO_VSE_C_HEADER_INCLUDED

/*****************************************************************************
Include Files
*/
#include "mem_utils/memory_table.h"
#include "mem_utils/shared_memory_ids.h"

/****************************************************************************
Private Const Declarations
*/

/* vse data object size, in AU */
#define VSE_BASE_STRUC_SIZE (sizeof(t_vse_object))
#define VSE_OBJECT_SIZE     VSE_BASE_STRUC_SIZE 

#define VSE_HEADROOM_SHIFTS              0;



/****************************************************************************
Private Type Definitions
*/

   
typedef struct {
   unsigned int vse_config;
   unsigned int binaural_flag;
   unsigned int speaker_spacing;
   unsigned int virtual_angle;
  
} t_vse_params;

// VSE Data Structure    
typedef struct  {
   void* input_buffer;
   void* output_buffer;
   t_vse_params* params_ptr;
   unsigned int  FS;
   unsigned int framebuffer_flag;
   unsigned int* input_read_addr;
   unsigned int* output_write_addr;
   unsigned int samples_to_process;
   unsigned int vse_cur_block_size;
   unsigned int* IPSI_COEFF_PTR_FIELD;
   unsigned int* CONTRA_COEFF_PTR_FIELD;
   unsigned int* XTC_COEFF_PTR_FIELD;
   unsigned int  DELAY_FIELD;
   unsigned int* DCB_COEFF_PTR_FIELD;
   unsigned int* ITF_COEFF_PTR_FIELD;
   unsigned int* LSF_COEFF_PTR_FIELD;
   unsigned int* PEAK_COEFF_PTR_FIELD;
   // History buffers
   unsigned int* PTR_HISTORY_BUF_Hi;
   unsigned int* PTR_HISTORY_BUF_Hc; 
   unsigned int* PTR_HISTORY_BUF_PEAK; 
   unsigned int* PTR_HISTORY_BUF_LSF; 
   // LIMITER SECTION
   unsigned int  LIMIT_ATTACK_TC;                 
   unsigned int  LIMIT_DECAY_TC;                
   unsigned int  LINEAR_ATTACK_TC;               
   unsigned int  LINEAR_DECAY_TC;  
   unsigned int  LIMIT_THRESHOLD;                 
   unsigned int  MAKEUPGAIN;                
   unsigned int  LIMITER_GAIN;               
   unsigned int  LIMITER_GAIN_LOG;  
   // Hi' filter
   unsigned int Hi_hist_buff[4];         
   // Hc' filter
   unsigned int Hc_hist_buff[66];                            
   unsigned int HcOUT_H_1;
   unsigned int HcOUT_L_1;
   // DCB filter
   unsigned int DCB_hist_buff[4];                   
   // EQ filter
   unsigned int EQ_hist_buff[3];                   
   // ITF FILTER
   unsigned int ITF_hist_buff[2];
   // PEAK FILTER
   unsigned int PEAK_hist_buff[9];
   unsigned int PEAK_hist_buff_size;
   // LSF FILTER
   unsigned int LSF_hist_buff[9];
   unsigned int LSF_hist_buff_size;
    // VSE buffers
   unsigned int  *out_ipsi;
   unsigned int  *out_contra;
   // Filter coefficient 
   unsigned int filter_coeff[24];
   
} t_vse_object;
  

#endif // AUDIO_VSE_C_HEADER_INCLUDED
