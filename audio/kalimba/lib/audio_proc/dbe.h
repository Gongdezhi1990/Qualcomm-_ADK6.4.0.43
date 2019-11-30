// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_DBE_HEADER_INCLUDED
#define AUDIO_DBE_HEADER_INCLUDED

#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "core_library.h"
#endif
#include "portability_macros.h"
   
   // *****************************************************************************
   // Constants used in the DBE code
   // *****************************************************************************
   .CONST $DBE_FRAME_SIZE                             24;
   .CONST $DBE_CONVERT_48_1                           0.004583333333;
   .CONST $DBE_CONVERT_48_2                           0.005208333333;
   .CONST $DBE_CONVERT_48_3                           0.005208333333;
   .CONST $DBE_LIM_MDB                                30;
   .CONST $DBE_COMP_TC                                0.023437500000;
   .CONST $ONEMINUS_DBE_COMP_TC                       0.976562500000;
   .CONST $DBE_LIM_MLIN                               32;
   .CONST $DBE_KLIM                                   0.25;
   .CONST $SAMP_FACTOR                                1.00000000000;        // FS/Fd_SamplingRate
   .CONST $EFFECT_IMPROVE_FACTOR                      0.880000000000;
   .CONST $dbe_GC_K                                   0.000244140625;
   .CONST $DBE_NL_const                               0.000061035156;
   .CONST $DBE_GAIN_TEMP_THRESH                       0.75;
   .CONST $DBE_GC_IN                                  0.03125;
   .CONST $DBE_LIMT_T_THRESH                          0.015625;
   .CONST $DBE_RefLevel_db_C1                         0.017773437500;
   .CONST $DBE_RefLevel_db_C2                        -0.050000000000;
   .CONST $DBE_RefLevel_db_C3                         0.000248046875;
   .CONST $DBE_const_num                              Qfmt_(3.75, 3);       // 3.75 in Q21               // (original value = 15/4)
   .CONST $DBE_SRC_v_factor2                          0.25;                 // 1/4
   .CONST $DBE_SRC_u_factor2                          0.75;                 // 3/4 
   .CONST $DBE_SRC_u_factor4                          0.125;                // 1/8
   .CONST $DBE_SRC_v_factor4                          0.375;                // 3/8                                                                 
   .CONST $DBE_SRC_w_factor4                          0.625;                // 5/8
   .CONST $DBE_SRC_z_factor4                          0.875;                // 7/8                                                                 
   .CONST $DBE_MAX_CHANNELS                           2;
   .CONST $DBE_P1_Q26                                 0.8;

   .CONST $DBE_SIGDETECT_RMS_ALFA                     0.00004166579862319;       
   .CONST $DBE_SIGDETECT_RMS_1M_ALFA                  (1.0 - $DBE_SIGDETECT_RMS_ALFA);
   .CONST $DBE_SIGDETECT_TIME_THRESHOLD_FRAMES        100;                  
   .CONST $DBE_LEVEL_THRESHOLD                        0.0003162277660168;   // -70 / (20*log10(2))

   
   // *****************************************************************************
   // DBE parameter structure
   // *****************************************************************************
   .CONST $audio_proc.dbe.parameter.DBE_CONFIG        0*ADDR_PER_WORD;
   .CONST $audio_proc.dbe.parameter.EFFECT_STRENGTH   ADDR_PER_WORD + $audio_proc.dbe.parameter.DBE_CONFIG; 
   .CONST $audio_proc.dbe.parameter.AMP_LIMIT         ADDR_PER_WORD + $audio_proc.dbe.parameter.EFFECT_STRENGTH;
   .CONST $audio_proc.dbe.parameter.FC_LP             ADDR_PER_WORD + $audio_proc.dbe.parameter.AMP_LIMIT;
   .CONST $audio_proc.dbe.parameter.FC_HP             ADDR_PER_WORD + $audio_proc.dbe.parameter.FC_LP;
   .CONST $audio_proc.dbe.parameter.HARM_CONTENT      ADDR_PER_WORD + $audio_proc.dbe.parameter.FC_HP;
   .CONST $audio_proc.dbe.parameter.XOVER_FC          ADDR_PER_WORD + $audio_proc.dbe.parameter.HARM_CONTENT;
   .CONST $audio_proc.dbe.parameter.MIX_BALANCE       ADDR_PER_WORD + $audio_proc.dbe.parameter.XOVER_FC;
   .CONST $audio_proc.dbe.parameter.STRUCT_SIZE       ADDR_PER_WORD + $audio_proc.dbe.parameter.MIX_BALANCE;  

   // *****************************************************************************
   // DBE data object structure
   // *****************************************************************************
   // -------------------------------- START PUBLIC SECTION ---------------------------------
   .CONST $audio_proc.dbe.INPUT_ADDR_FIELD            0*ADDR_PER_WORD;
   .CONST $audio_proc.dbe.OUTPUT_ADDR_FIELD           ADDR_PER_WORD + $audio_proc.dbe.INPUT_ADDR_FIELD;
   .CONST $audio_proc.dbe.MONO_STEREO_FLAG_FIELD      ADDR_PER_WORD + $audio_proc.dbe.OUTPUT_ADDR_FIELD;
   .CONST $audio_proc.dbe.SAMPLE_RATE_FIELD           ADDR_PER_WORD + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD;
   .CONST $audio_proc.dbe.PARAM_PTR_FIELD             ADDR_PER_WORD + $audio_proc.dbe.SAMPLE_RATE_FIELD;
   // -------------------------------- START INTERNAL SECTION ---------------------------------
   .CONST $audio_proc.dbe.CAP_CONFIG_FIELD            ADDR_PER_WORD + $audio_proc.dbe.PARAM_PTR_FIELD;
   .CONST $audio_proc.dbe.FRAMEBUFFER_FLAG            ADDR_PER_WORD + $audio_proc.dbe.CAP_CONFIG_FIELD;
   .CONST $audio_proc.dbe.INPUT_READ_ADDR             ADDR_PER_WORD + $audio_proc.dbe.FRAMEBUFFER_FLAG;
   .CONST $audio_proc.dbe.OUTPUT_WRITE_ADDR           ADDR_PER_WORD + $audio_proc.dbe.INPUT_READ_ADDR;
   .CONST $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS      ADDR_PER_WORD + $audio_proc.dbe.OUTPUT_WRITE_ADDR;
   .CONST $audio_proc.dbe.DBE_CUR_BLOCK_SIZE          ADDR_PER_WORD + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS;
   .CONST $audio_proc.dbe.DBE_GAIN_UPDATE_FLAG        ADDR_PER_WORD + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE;
   .CONST $audio_proc.dbe.XOVER_BYPASS_FLAG           ADDR_PER_WORD + $audio_proc.dbe.DBE_GAIN_UPDATE_FLAG;
   .CONST $audio_proc.dbe.MIXER_BYPASS_FLAG           ADDR_PER_WORD + $audio_proc.dbe.XOVER_BYPASS_FLAG;
   .CONST $audio_proc.dbe.DBE_DOWNSAMPLE_FACTOR       ADDR_PER_WORD + $audio_proc.dbe.MIXER_BYPASS_FLAG;
   .CONST $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR      ADDR_PER_WORD + $audio_proc.dbe.DBE_DOWNSAMPLE_FACTOR;
   .CONST $audio_proc.dbe.PTR_HISTORY_BUF_XOVER       ADDR_PER_WORD + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR;
   .CONST $audio_proc.dbe.PTR_HISTORY_BUF_HP1         ADDR_PER_WORD + $audio_proc.dbe.PTR_HISTORY_BUF_XOVER;
   .CONST $audio_proc.dbe.PTR_HISTORY_BUF_HP2         ADDR_PER_WORD + $audio_proc.dbe.PTR_HISTORY_BUF_HP1;
   .CONST $audio_proc.dbe.PTR_HISTORY_BUF_SRC         ADDR_PER_WORD + $audio_proc.dbe.PTR_HISTORY_BUF_HP2;
   .CONST $audio_proc.dbe.USR_FCHP                    ADDR_PER_WORD + $audio_proc.dbe.PTR_HISTORY_BUF_SRC;
   .CONST $audio_proc.dbe.USR_FCLP                    ADDR_PER_WORD + $audio_proc.dbe.USR_FCHP;
   .CONST $audio_proc.dbe.FCHP                        ADDR_PER_WORD + $audio_proc.dbe.USR_FCLP;
   .CONST $audio_proc.dbe.FCLP                        ADDR_PER_WORD + $audio_proc.dbe.FCHP;
   .CONST $audio_proc.dbe.FCHP2                       ADDR_PER_WORD + $audio_proc.dbe.FCLP;
   .CONST $audio_proc.dbe.FCHP3                       ADDR_PER_WORD + $audio_proc.dbe.FCHP2;
   .CONST $audio_proc.dbe.DBE2_STRENGTH               ADDR_PER_WORD + $audio_proc.dbe.FCHP3;
   .CONST $audio_proc.dbe.RMS_LVL                     ADDR_PER_WORD + $audio_proc.dbe.DBE2_STRENGTH;
   .CONST $audio_proc.dbe.THRESHOLD_COUNTER           ADDR_PER_WORD + $audio_proc.dbe.RMS_LVL;

   // *****************************************************************************
   // intermediate buffers
   // *****************************************************************************
   .CONST $audio_proc.dbe.hp1_out                     ADDR_PER_WORD + $audio_proc.dbe.THRESHOLD_COUNTER;
   .CONST $audio_proc.dbe.hp3_out                     ADDR_PER_WORD + $audio_proc.dbe.hp1_out;
   .CONST $audio_proc.dbe.hp2_out                     ADDR_PER_WORD + $audio_proc.dbe.hp3_out;
   .CONST $audio_proc.dbe.ntp_tp_filters_buf          ADDR_PER_WORD + $audio_proc.dbe.hp2_out;
   .CONST $audio_proc.xover.high_freq_output_buf      ADDR_PER_WORD + $audio_proc.dbe.ntp_tp_filters_buf;
   
   // *****************************************************************************
   //                                   HP1 FILTER
   // *****************************************************************************
   // coefficients
   .CONST $audio_proc.dbe.HP1_A1                      ADDR_PER_WORD + $audio_proc.xover.high_freq_output_buf;
   // history buffer for HP1
   .CONST $audio_proc.dbe.START_HISTORY_BUF_HP1       ADDR_PER_WORD + $audio_proc.dbe.HP1_A1;
   .CONST $audio_proc.dbe.HP1OUT_L                    0 + $audio_proc.dbe.START_HISTORY_BUF_HP1;  
   .CONST $audio_proc.dbe.HP1OUT_L_1                  ADDR_PER_WORD + $audio_proc.dbe.HP1OUT_L;   
   .CONST $audio_proc.dbe.HP1IN                       ADDR_PER_WORD + $audio_proc.dbe.HP1OUT_L_1;  
   .CONST $audio_proc.dbe.HP1IN_1                     ADDR_PER_WORD + $audio_proc.dbe.HP1IN;    
   .CONST $audio_proc.dbe.HP1OUT_H                    ADDR_PER_WORD + $audio_proc.dbe.HP1IN_1;  
   .CONST $audio_proc.dbe.HP1OUT_H_1                  ADDR_PER_WORD + $audio_proc.dbe.HP1OUT_H;  
   .CONST $audio_proc.dbe.HP1_HIST_BUF_SIZE           ADDR_PER_WORD + $audio_proc.dbe.HP1OUT_H_1;  
   
   // *****************************************************************************
   //                                    NTP1 FILTER
   // *****************************************************************************
   // coefficients
   .CONST $audio_proc.dbe.NTP1_B1                     0 + $audio_proc.dbe.HP1_HIST_BUF_SIZE;
   .CONST $audio_proc.dbe.NTP1_A1                     ADDR_PER_WORD + $audio_proc.dbe.NTP1_B1;
   // history buffer for NTP1
   .CONST $audio_proc.dbe.NTP1_IN_1                   ADDR_PER_WORD + $audio_proc.dbe.NTP1_A1;
   .CONST $audio_proc.dbe.NTP1_OUT_1                  ADDR_PER_WORD + $audio_proc.dbe.NTP1_IN_1;
   
   // *****************************************************************************
   //                                    NTP2 FILTER
   // *****************************************************************************
   // coefficients
   .CONST $audio_proc.dbe.NTP2_B1                     ADDR_PER_WORD + $audio_proc.dbe.NTP1_OUT_1;
   .CONST $audio_proc.dbe.NTP2_A1                     ADDR_PER_WORD + $audio_proc.dbe.NTP2_B1;
   // history buffer for NTP2
   .CONST $audio_proc.dbe.NTP2_IN_1                   ADDR_PER_WORD + $audio_proc.dbe.NTP2_A1;
   .CONST $audio_proc.dbe.NTP2_OUT_1                  ADDR_PER_WORD + $audio_proc.dbe.NTP2_IN_1;
   
   // *****************************************************************************
   //                                    NHP FILTER
   // *****************************************************************************
   // coefficients
   .CONST $audio_proc.dbe.NHP_A1                      ADDR_PER_WORD + $audio_proc.dbe.NTP2_OUT_1;
   // history buffer for NHP
   .CONST $audio_proc.dbe.NHPIN_1                     ADDR_PER_WORD + $audio_proc.dbe.NHP_A1;
   .CONST $audio_proc.dbe.NHPOUT_1                    ADDR_PER_WORD + $audio_proc.dbe.NHPIN_1;
   
   // *****************************************************************************
   //                                    TP1 FILTER
   // *****************************************************************************
   // coefficients
   .CONST $audio_proc.dbe.TP1_B0                      ADDR_PER_WORD + $audio_proc.dbe.NHPOUT_1;
   .CONST $audio_proc.dbe.TP1_A1                      ADDR_PER_WORD + $audio_proc.dbe.TP1_B0;
   // history buffer for TP1
   .CONST $audio_proc.dbe.TP1OUT_1_LEFT               ADDR_PER_WORD + $audio_proc.dbe.TP1_A1;
   .CONST $audio_proc.dbe.ABS_IBUF                    ADDR_PER_WORD + $audio_proc.dbe.TP1OUT_1_LEFT;
   
   // *****************************************************************************
   //                                    TP2 FILTER
   // *****************************************************************************
   // coefficients
   .CONST $audio_proc.dbe.TP2_B0                      ADDR_PER_WORD + $audio_proc.dbe.ABS_IBUF;
   .CONST $audio_proc.dbe.TP2_A1                      ADDR_PER_WORD + $audio_proc.dbe.TP2_B0;
   // history buffer for TP2
   .CONST $audio_proc.dbe.TP2IN2_1                    ADDR_PER_WORD + $audio_proc.dbe.TP2_A1;
   .CONST $audio_proc.dbe.TP2OUT_1                    ADDR_PER_WORD + $audio_proc.dbe.TP2IN2_1;
   
   // *****************************************************************************
   //                                    HP2 FILTER
   // *****************************************************************************
   // coefficients
   .CONST $audio_proc.dbe.HP2_A1                      ADDR_PER_WORD + $audio_proc.dbe.TP2OUT_1;
   .CONST $audio_proc.dbe.HP2_A2                      ADDR_PER_WORD + $audio_proc.dbe.HP2_A1;
   .CONST $audio_proc.dbe.HP2_B1                      ADDR_PER_WORD + $audio_proc.dbe.HP2_A2;
   // history buffer for TP2
   .CONST $audio_proc.dbe.START_HISTORY_BUF_HP2       ADDR_PER_WORD + $audio_proc.dbe.HP2_B1;
   .CONST $audio_proc.dbe.HP2OUT_L                    0 + $audio_proc.dbe.START_HISTORY_BUF_HP2;  
   .CONST $audio_proc.dbe.HP2OUT_L_1                  ADDR_PER_WORD + $audio_proc.dbe.HP2OUT_L;   
   .CONST $audio_proc.dbe.HP2OUT_L_2                  ADDR_PER_WORD + $audio_proc.dbe.HP2OUT_L_1;
   .CONST $audio_proc.dbe.HP2IN                       ADDR_PER_WORD + $audio_proc.dbe.HP2OUT_L_2;  
   .CONST $audio_proc.dbe.HP2IN_1                     ADDR_PER_WORD + $audio_proc.dbe.HP2IN;    
   .CONST $audio_proc.dbe.HP2IN_2                     ADDR_PER_WORD + $audio_proc.dbe.HP2IN_1;
   .CONST $audio_proc.dbe.HP2OUT_H                    ADDR_PER_WORD + $audio_proc.dbe.HP2IN_2;  
   .CONST $audio_proc.dbe.HP2OUT_H_1                  ADDR_PER_WORD + $audio_proc.dbe.HP2OUT_H;  
   .CONST $audio_proc.dbe.HP2OUT_H_2                  ADDR_PER_WORD + $audio_proc.dbe.HP2OUT_H_1; 
   .CONST $audio_proc.dbe.HP2_HIST_BUF_SIZE           ADDR_PER_WORD + $audio_proc.dbe.HP2OUT_H_2;
   
   // *****************************************************************************
   //                                    MIXER1
   // *****************************************************************************
   .CONST $audio_proc.dbe.MIXER1_HP1_HIST             0 + $audio_proc.dbe.HP2_HIST_BUF_SIZE;
   .CONST $audio_proc.dbe.MIXER1_NHP_HIST             ADDR_PER_WORD + $audio_proc.dbe.MIXER1_HP1_HIST;
   
   // *****************************************************************************
   //                                    DBE GAINS
   // *****************************************************************************
   .CONST $audio_proc.dbe.DBE_GAIN_UPDATE            ADDR_PER_WORD + $audio_proc.dbe.MIXER1_NHP_HIST;
   .CONST $audio_proc.dbe.DBE_NLGAIN0                ADDR_PER_WORD + $audio_proc.dbe.DBE_GAIN_UPDATE;
   .CONST $audio_proc.dbe.DBE_NLGAIN1                ADDR_PER_WORD + $audio_proc.dbe.DBE_NLGAIN0;
   .CONST $audio_proc.dbe.fix_gain_lin               ADDR_PER_WORD + $audio_proc.dbe.DBE_NLGAIN1;
   .CONST $audio_proc.dbe.dbe_GC_in                  ADDR_PER_WORD + $audio_proc.dbe.fix_gain_lin;
   .CONST $audio_proc.dbe.RefLevel_db                ADDR_PER_WORD + $audio_proc.dbe.dbe_GC_in;
   .CONST $audio_proc.dbe.RefLevel_lin               ADDR_PER_WORD + $audio_proc.dbe.RefLevel_db;
   .CONST $audio_proc.dbe.RefLevelLim_db             ADDR_PER_WORD + $audio_proc.dbe.RefLevel_lin;
   .CONST $audio_proc.dbe.RefLevelLim_lin            ADDR_PER_WORD + $audio_proc.dbe.RefLevelLim_db;
   .CONST $audio_proc.dbe.DBE_GAIN                   ADDR_PER_WORD + $audio_proc.dbe.RefLevelLim_lin;
   .CONST $audio_proc.dbe.dbe_gain_sm                ADDR_PER_WORD + $audio_proc.dbe.DBE_GAIN;
   .CONST $audio_proc.dbe.sqrtGC                     ADDR_PER_WORD + $audio_proc.dbe.dbe_gain_sm;

   // *****************************************************************************
   //                                    DBE GAIN Input 
   // *****************************************************************************
   .CONST $audio_proc.dbe.Ibuf_dbe_left              ADDR_PER_WORD + $audio_proc.dbe.sqrtGC;
   .CONST $audio_proc.dbe.Ibuf_dbe_right             ADDR_PER_WORD + $audio_proc.dbe.Ibuf_dbe_left;
   
   // *****************************************************************************
   //            Sample Rate Conversion: interpolation filter @ fs = 48kHz
   // *****************************************************************************
   .CONST $audio_proc.dbe.SRC_B1                      ADDR_PER_WORD + $audio_proc.dbe.Ibuf_dbe_right;
   .CONST $audio_proc.dbe.SRC_B2                      ADDR_PER_WORD + $audio_proc.dbe.SRC_B1;
   .CONST $audio_proc.dbe.SRC_B3                      ADDR_PER_WORD + $audio_proc.dbe.SRC_B2;
   .CONST $audio_proc.dbe.SRC_B4                      ADDR_PER_WORD + $audio_proc.dbe.SRC_B3;
   
   // *****************************************************************************
   // history buffer for SRC
   // *****************************************************************************
   .CONST $audio_proc.dbe.START_HISTORY_BUF_SRC       ADDR_PER_WORD + $audio_proc.dbe.SRC_B4;
   .CONST $audio_proc.dbe.SRCIN                       0 + $audio_proc.dbe.START_HISTORY_BUF_SRC;  
   .CONST $audio_proc.dbe.SRCIN_1                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN;   
   .CONST $audio_proc.dbe.SRCIN_2                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN_1;
   .CONST $audio_proc.dbe.SRCIN_3                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN_2;  
   .CONST $audio_proc.dbe.SRCIN_4                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN_3;
   .CONST $audio_proc.dbe.SRCIN_5                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN_4;    
   .CONST $audio_proc.dbe.SRCIN_6                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN_5;    
   .CONST $audio_proc.dbe.SRCIN_7                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN_6;    
   .CONST $audio_proc.dbe.SRCIN_8                     ADDR_PER_WORD + $audio_proc.dbe.SRCIN_7;        
   .CONST $audio_proc.dbe.SRC_HIST_BUF_SIZE           ADDR_PER_WORD + $audio_proc.dbe.SRCIN_8;
   
   // *****************************************************************************
   // coefficients for the XOVER filter
   // *****************************************************************************
   .CONST $audio_proc.xover.G0                        ADDR_PER_WORD + $audio_proc.dbe.SRC_HIST_BUF_SIZE;
   .CONST $audio_proc.xover.G1                        ADDR_PER_WORD + $audio_proc.xover.G0;
   .CONST $audio_proc.xover.G2                        ADDR_PER_WORD + $audio_proc.xover.G1;
   
   // *****************************************************************************
   // history buffer for XOVER
   // *****************************************************************************
   .CONST $audio_proc.xover.START_HISTORY_BUF_XOVER   ADDR_PER_WORD + $audio_proc.xover.G2;
   .CONST $audio_proc.xover.AP2L                      0 + $audio_proc.xover.START_HISTORY_BUF_XOVER;
   .CONST $audio_proc.xover.AP2L_1                    ADDR_PER_WORD + $audio_proc.xover.AP2L;  
   .CONST $audio_proc.xover.AP2L_2                    ADDR_PER_WORD + $audio_proc.xover.AP2L_1;  
   .CONST $audio_proc.xover.APIN                      ADDR_PER_WORD + $audio_proc.xover.AP2L_2;  
   .CONST $audio_proc.xover.APIN_1                    ADDR_PER_WORD + $audio_proc.xover.APIN;  
   .CONST $audio_proc.xover.APIN_2                    ADDR_PER_WORD + $audio_proc.xover.APIN_1;  
   .CONST $audio_proc.xover.AP2                       ADDR_PER_WORD + $audio_proc.xover.APIN_2;  
   .CONST $audio_proc.xover.AP2_1                     ADDR_PER_WORD + $audio_proc.xover.AP2;  
   .CONST $audio_proc.xover.AP2_2                     ADDR_PER_WORD + $audio_proc.xover.AP2_1;  
   .CONST $audio_proc.xover.AP1L                      ADDR_PER_WORD + $audio_proc.xover.AP2_2;  
   .CONST $audio_proc.xover.AP1L_1                    ADDR_PER_WORD + $audio_proc.xover.AP1L;  
   .CONST $audio_proc.xover.AP1                       ADDR_PER_WORD + $audio_proc.xover.AP1L_1;  
   .CONST $audio_proc.xover.AP1_1                     ADDR_PER_WORD + $audio_proc.xover.AP1;  
   
   .CONST $audio_proc.dbe.STRUC_SIZE                  1 + ($audio_proc.xover.AP1_1 >> LOG2_ADDR_PER_WORD);
#endif // AUDIO_DBE_HEADER_INCLUDED
