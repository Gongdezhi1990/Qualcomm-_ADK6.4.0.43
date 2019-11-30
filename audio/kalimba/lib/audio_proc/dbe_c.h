// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_DBE_C_HEADER_INCLUDED
#define AUDIO_DBE_C_HEADER_INCLUDED

/*****************************************************************************
Include Files
*/
#include "mem_utils/memory_table.h"
#include "mem_utils/shared_memory_ids.h"

/****************************************************************************
Private Const Declarations
*/

/* dbe data object size, in AU */
#define DBE_BASE_STRUC_SIZE (sizeof(t_dbe_object))
#define DBE_OBJECT_SIZE     DBE_BASE_STRUC_SIZE 

/* "coefficient params" object size */
#define DBE_PARAMS_OBJECT_SIZE           8

#define DBE_HEADROOM_SHIFTS              0;



/****************************************************************************
Private Type Definitions
*/

   
typedef struct {
   unsigned int dbe_config;
   unsigned int effect_strength; /* format: unsigned integer (0-100) */
   unsigned int amp_limit;       /* format: 12.12(arch3/5), 12.20(arch4) */
   unsigned int fc_lp;           /* format: 20.4(arch3/5), 20.12(arch4) */
   unsigned int fc_hp;           /* format: 20.4(arch3/5), 20.12(arch4) */
   unsigned int harm_content;    /* format: unsigned integer (0-100) */
   unsigned int xover_fc;        /* format: 20.4(arch3/5), 20.12(arch4) */
   unsigned int mix_balance;     /* format: unsigned integer (0-100) */
  
} t_dbe_params;

// DBE Data Structure    
typedef struct  {
   void* input_buffer;
   void* output_buffer;
   unsigned int mono_stereo_flag;
   unsigned int sample_rate;
   t_dbe_params* params_ptr;
   unsigned int  cap_config;
   unsigned int framebuffer_flag;
   unsigned int* input_read_addr;
   unsigned int* output_write_addr;
   unsigned int dbe_samples_to_process;
   unsigned int dbe_cur_block_size;
   unsigned int dbe_gain_update_flag;
   unsigned int xover_bypass_flag;
   unsigned int mixer_bypass_flag;
   unsigned int dbe_downsample_factor;
   unsigned int dbe_frame_shift_factor;
   unsigned int* PTR_HISTORY_BUF_XOVER;
   unsigned int* PTR_HISTORY_BUF_HP1;
   unsigned int* PTR_HISTORY_BUF_HP2;
   unsigned int* PTR_HISTORY_BUF_SRC;
   unsigned int USR_FCHP;
   unsigned int USR_FCLP;
   unsigned int FCHP;
   unsigned int FCLP;
   unsigned int FCHP2;
   unsigned int FCHP3;
   unsigned int DBE2_STRENGTH;
   unsigned int RMS_LVL;
   unsigned int THRESHOLD_COUNTER;
   
   // Intermediate buffers
   unsigned int *hp1_out;                 // scratch ptr ok
   unsigned int *hp3_out;                 // scratch ptr ok
   unsigned int *hp2_out;                 // scratch ptr ok
   unsigned int *ntp_tp_filters_buf;      // scratch ptr ok
   unsigned int *high_freq_output_buf;    // scratch ptr ok
   //HP1 FILTER
   unsigned int HP1_A1;
   unsigned int HP1OUT_L; 
   unsigned int HP1OUT_L_1; 
   unsigned int HP1IN; 
   unsigned int HP1IN_1; 
   unsigned int HP1OUT_H; 
   unsigned int HP1OUT_H_1; 
   //NTP1 FILTER
   unsigned int NTP1_B1;                   
   unsigned int NTP1_A1;                     
   unsigned int NTP1_IN_1;                  
   unsigned int NTP1_OUT_1;    
   //NTP2 FILTER
   unsigned int NTP2_B1;                   
   unsigned int NTP2_A1;                     
   unsigned int NTP2_IN_1;                  
   unsigned int NTP2_OUT_1;                 
   //NHP FILTER
   unsigned int NHP_A1;                   
   unsigned int NHPIN_1;                     
   unsigned int NHPOUT_1;
   // TP1 FILTER
   unsigned int TP1_B0;                   
   unsigned int TP1_A1;                     
   unsigned int TP1OUT_1_LEFT; 
   unsigned int ABS_IBUF;
   // TP2 FILTER
   unsigned int TP2_B0;                   
   unsigned int TP2_A1;                     
   unsigned int TP2IN2_1; 
   unsigned int TP2OUT_1;	
   // HP2 FILTER
   unsigned int HP2_A1;                   
   unsigned int HP2_A2;                     
   unsigned int HP2_B1;
   unsigned int HP2OUT_L;                   
   unsigned int HP2OUT_L_1;                     
   unsigned int HP2OUT_L_2;
   unsigned int HP2IN;                   
   unsigned int HP2IN_1;                     
   unsigned int HP2IN_2;
   unsigned int HP2OUT_H;                   
   unsigned int HP2OUT_H_1;                     
   unsigned int HP2OUT_H_2;
   unsigned int MIXER1_HP1_HIST;                     
   unsigned int MIXER1_NHP_HIST;
   //DBE GAINs
   unsigned int DBE_GAIN_UPDATE;                   
   unsigned int DBE_NLGAIN0;                     
   unsigned int DBE_NLGAIN1;
   unsigned int fix_gain_lin;                     
   unsigned int dbe_GC_in;
   unsigned int RefLevel_db;                   
   unsigned int RefLevel_lin;                     
   unsigned int RefLevelLim_db;
   unsigned int RefLevelLim_lin;                   
   unsigned int DBE_GAIN;                     
   unsigned int dbe_gain_sm;
   unsigned int sqrtGC;
   unsigned int Ibuf_dbe_left;
   unsigned int Ibuf_dbe_right;
   unsigned int SRC_B1;                   
   unsigned int SRC_B2;                     
   unsigned int SRC_B3;
   unsigned int SRC_B4;                     
   unsigned int SRCIN;
   unsigned int SRCIN_1;                   
   unsigned int SRCIN_2;                     
   unsigned int SRCIN_3;
   unsigned int SRCIN_4;
   unsigned int SRCIN_5;                   
   unsigned int SRCIN_6;                     
   unsigned int SRCIN_7;
   unsigned int SRCIN_8;
   unsigned int SRC_hist_buf_size;

   // XOVER FILTER
   unsigned int *xover_g0;
   unsigned int *xover_g1;
   unsigned int *xover_g2;
   unsigned int *xover_ap2_l;
   unsigned int *xover_ap2_l1;
   unsigned int *xover_ap2_l2;
   unsigned int *xover_ap_in;
   unsigned int *xover_ap_in1;
   unsigned int *xover_ap_in2;
   unsigned int *xover_ap2;
   unsigned int *xover_ap2_1;
   unsigned int *xover_ap2_2;
   unsigned int *xover_ap1_l;
   unsigned int *xover_ap1_l1;
   unsigned int *xover_ap1;
   unsigned int *xover_ap1_1;
   
} t_dbe_object;
  

#endif // AUDIO_DBE_C_HEADER_INCLUDED
