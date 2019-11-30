// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef AUDIO_VSE_HEADER_INCLUDED
#define AUDIO_VSE_HEADER_INCLUDED

#ifdef KYMERA
#include "cbuffer_asm.h"
#include "portability_macros.h"
#else
#include "core_library.h"
#endif
                                               
   
   .CONST $VSE_FRAME_SIZE                             60;
   .CONST $VSE_SAMPLE_RATE_32000                      32000;
   .CONST $VSE_SAMPLE_RATE_44100                      44100;
   .CONST $VSE_SAMPLE_RATE_48000                      48000;
   .CONST $VSE_SAMPLE_RATE_96000                      96000;
   .CONST $VSE_SPEAKER_SPACING_5CM                    0.05;
   .CONST $VSE_SPEAKER_SPACING_10CM                   0.10;
   .CONST $VSE_SPEAKER_SPACING_15CM                   0.15;
   .CONST $VSE_SPEAKER_SPACING_20CM                   0.20;
   .CONST $BIN_SYNTH_FILTER_COEFF_SIZE                3;
   .CONST $ITF_COEFF_FILTER_SIZE                      2;
   .CONST $LSF_PEAK_COEFF_FILTER_SIZE                 5;
   .CONST $SAMPLE_RATES_SUPPORTED_COUNT               5;
   
   .CONST $LIMIT_ATTACK_TC_44K                        0.743478864379752;
   .CONST $LIMIT_DECAY_TC_44K                         0.00135961909700633;
   .CONST $LINEAR_DECAY_TC_44K                        0.238228657310286;
   .CONST $LINEAR_ATTACK_TC_44K                       0.00271738962992374;
   
   .CONST $LIMIT_ATTACK_TC_NON_44K                    0.71349520313981;
   .CONST $LIMIT_DECAY_TC_NON_44K                     0.00124921907541908;
   .CONST $LINEAR_ATTACK_TC_NON_44K                   0.221199216928595;
   .CONST $LINEAR_DECAY_TC_NON_44K                    0.00249687760253992;
   
   .CONST $LIMIT_THRESHOLD                            Qfmt_(-0.498289214233, 8);
   .CONST $MAKEUPGAIN                                 1.0;
  
   // 3dv parameter structure
   .CONST $audio_proc.vse.parameter.VSE_CONFIG        0*ADDR_PER_WORD;
   .CONST $audio_proc.vse.parameter.BINAURAL_FLAG     ADDR_PER_WORD + $audio_proc.vse.parameter.VSE_CONFIG;
   .CONST $audio_proc.vse.parameter.SPEAKER_SPACING   ADDR_PER_WORD + $audio_proc.vse.parameter.BINAURAL_FLAG;
   .CONST $audio_proc.vse.parameter.VIRTUAL_ANGLE     ADDR_PER_WORD + $audio_proc.vse.parameter.SPEAKER_SPACING;
   .CONST $audio_proc.vse.parameter.STRUCT_SIZE       ADDR_PER_WORD + $audio_proc.vse.parameter.VIRTUAL_ANGLE;


   // VSE structure
   // -------------------------------- START PUBLIC SECTION ---------------------------------
   .CONST $audio_proc.vse.INPUT_ADDR_FIELD            0*ADDR_PER_WORD;
   .CONST $audio_proc.vse.OUTPUT_ADDR_FIELD           ADDR_PER_WORD + $audio_proc.vse.INPUT_ADDR_FIELD;
   .CONST $audio_proc.vse.PARAM_PTR_FIELD             ADDR_PER_WORD + $audio_proc.vse.OUTPUT_ADDR_FIELD;  
   .CONST $audio_proc.vse.FS                          ADDR_PER_WORD + $audio_proc.vse.PARAM_PTR_FIELD;  
   // -------------------------------- START INTERNAL SECTION ---------------------------------
   .CONST $audio_proc.vse.FRAMEBUFFER_FLAG            ADDR_PER_WORD + $audio_proc.vse.FS;
   .CONST $audio_proc.vse.INPUT_READ_ADDR             ADDR_PER_WORD + $audio_proc.vse.FRAMEBUFFER_FLAG;
   .CONST $audio_proc.vse.OUTPUT_WRITE_ADDR           ADDR_PER_WORD + $audio_proc.vse.INPUT_READ_ADDR;
   .CONST $audio_proc.vse.SAMPLES_TO_PROCESS          ADDR_PER_WORD + $audio_proc.vse.OUTPUT_WRITE_ADDR;
   .CONST $audio_proc.vse.VSE_CUR_BLOCK_SIZE          ADDR_PER_WORD + $audio_proc.vse.SAMPLES_TO_PROCESS;
   .CONST $audio_proc.vse.IPSI_COEFF_PTR_FIELD        ADDR_PER_WORD + $audio_proc.vse.VSE_CUR_BLOCK_SIZE;
   .CONST $audio_proc.vse.CONTRA_COEFF_PTR_FIELD      ADDR_PER_WORD + $audio_proc.vse.IPSI_COEFF_PTR_FIELD;
   .CONST $audio_proc.vse.XTC_COEFF_PTR_FIELD         ADDR_PER_WORD + $audio_proc.vse.CONTRA_COEFF_PTR_FIELD;
   .CONST $audio_proc.vse.DELAY_FIELD                 ADDR_PER_WORD + $audio_proc.vse.XTC_COEFF_PTR_FIELD;
   .CONST $audio_proc.vse.DCB_COEFF_PTR_FIELD         ADDR_PER_WORD + $audio_proc.vse.DELAY_FIELD;
   .CONST $audio_proc.vse.ITF_COEFF_PTR_FIELD         ADDR_PER_WORD + $audio_proc.vse.DCB_COEFF_PTR_FIELD;
   .CONST $audio_proc.vse.LSF_COEFF_PTR_FIELD         ADDR_PER_WORD + $audio_proc.vse.ITF_COEFF_PTR_FIELD;
   .CONST $audio_proc.vse.PEAK_COEFF_PTR_FIELD        ADDR_PER_WORD + $audio_proc.vse.LSF_COEFF_PTR_FIELD;
   .CONST $audio_proc.vse.PTR_HISTORY_BUF_Hi          ADDR_PER_WORD + $audio_proc.vse.PEAK_COEFF_PTR_FIELD;
   .CONST $audio_proc.vse.PTR_HISTORY_BUF_Hc          ADDR_PER_WORD + $audio_proc.vse.PTR_HISTORY_BUF_Hi;
   .CONST $audio_proc.vse.PTR_HISTORY_BUF_PEAK        ADDR_PER_WORD + $audio_proc.vse.PTR_HISTORY_BUF_Hc;
   .CONST $audio_proc.vse.PTR_HISTORY_BUF_LSF         ADDR_PER_WORD + $audio_proc.vse.PTR_HISTORY_BUF_PEAK;
   
   // *****************************************************************************
   //                                         LIMITER SECTION
   // *****************************************************************************
   .CONST $audio_proc.vse.LIMIT_ATTACK_TC        ADDR_PER_WORD + $audio_proc.vse.PTR_HISTORY_BUF_LSF;
   .CONST $audio_proc.vse.LIMIT_DECAY_TC         ADDR_PER_WORD + $audio_proc.vse.LIMIT_ATTACK_TC;
   .CONST $audio_proc.vse.LINEAR_ATTACK_TC       ADDR_PER_WORD + $audio_proc.vse.LIMIT_DECAY_TC;
   .CONST $audio_proc.vse.LINEAR_DECAY_TC        ADDR_PER_WORD + $audio_proc.vse.LINEAR_ATTACK_TC;
   .CONST $audio_proc.vse.LIMIT_THRESHOLD        ADDR_PER_WORD + $audio_proc.vse.LINEAR_DECAY_TC;
   .CONST $audio_proc.vse.MAKEUPGAIN             ADDR_PER_WORD + $audio_proc.vse.LIMIT_THRESHOLD;
   .CONST $audio_proc.vse.LIMITER_GAIN           ADDR_PER_WORD + $audio_proc.vse.MAKEUPGAIN;
   .CONST $audio_proc.vse.LIMITER_GAIN_LOG       ADDR_PER_WORD + $audio_proc.vse.LIMITER_GAIN;
   
   // *****************************************************************************
   //                                   Hi' FILTER
   // *****************************************************************************
   // history buffer for Hi'
   .CONST $audio_proc.vse.START_HISTORY_BUF_Hi       1*ADDR_PER_WORD + $audio_proc.vse.LIMITER_GAIN_LOG;
   .CONST $audio_proc.vse.END_HISTORY_BUF_Hi         3*ADDR_PER_WORD + $audio_proc.vse.START_HISTORY_BUF_Hi; /// y_l'  , y_h', x_in'
      
   // *****************************************************************************
   //                                      Hc' FILTER
   // *****************************************************************************

   // history buffer for Hc'
   .CONST $audio_proc.vse.START_HISTORY_BUF_Hc       0*ADDR_PER_WORD + $audio_proc.vse.END_HISTORY_BUF_Hi;    
   .CONST $audio_proc.vse.Hc_HIST_BUF_SIZE          66*ADDR_PER_WORD + $audio_proc.vse.START_HISTORY_BUF_Hc;
   .CONST $audio_proc.vse.DELAYLINE_SIZE_FIELD       1*ADDR_PER_WORD + $audio_proc.vse.Hc_HIST_BUF_SIZE;
   .CONST $audio_proc.vse.HcOUT_H_1                  0*ADDR_PER_WORD + $audio_proc.vse.DELAYLINE_SIZE_FIELD;  
   .CONST $audio_proc.vse.HcOUT_L_1                  1*ADDR_PER_WORD + $audio_proc.vse.HcOUT_H_1;
   
   // *****************************************************************************
   //                                 DCB
   // *****************************************************************************
   
   // history buffer for DCB
   .CONST $audio_proc.vse.START_HISTORY_BUF_DCB       1*ADDR_PER_WORD + $audio_proc.vse.HcOUT_L_1; 
   .CONST $audio_proc.vse.END_HISTORY_BUF_DCB         3*ADDR_PER_WORD + $audio_proc.vse.START_HISTORY_BUF_DCB; /// y_l'  , y_h', x_in'
   
   // *****************************************************************************
   //                                 EQ
   // *****************************************************************************
   
   // history buffer for EQ
   .CONST $audio_proc.vse.START_HISTORY_BUF_EQ        0*ADDR_PER_WORD + $audio_proc.vse.END_HISTORY_BUF_DCB; 
   .CONST $audio_proc.vse.END_HISTORY_BUF_EQ          3*ADDR_PER_WORD + $audio_proc.vse.START_HISTORY_BUF_EQ; /// y_l'  , y_h', x_in'
   
   // *****************************************************************************
   //                                 ITF
   // *****************************************************************************
   
   // history buffer for DCB
   .CONST $audio_proc.vse.HiftOUT_L_1                 1*ADDR_PER_WORD + $audio_proc.vse.END_HISTORY_BUF_EQ; 
   .CONST $audio_proc.vse.HiftOUT_H_1                 1*ADDR_PER_WORD + $audio_proc.vse.HiftOUT_L_1; 
   
   // *****************************************************************************
   //                                     PEAK FILTER
   // *****************************************************************************

   // history buffer for PEAK FILTER
   .CONST $audio_proc.vse.START_HISTORY_BUF_PEAK       1*ADDR_PER_WORD + $audio_proc.vse.HiftOUT_H_1;    
   .CONST $audio_proc.vse.PEAK_HIST_BUF_SIZE           9*ADDR_PER_WORD + $audio_proc.vse.START_HISTORY_BUF_PEAK;//// y_l,y_l',y_l'',x ,x',x'',y_h,y_h',y_h''
   
   // *****************************************************************************
   //                                     LSF FILTER
   // *****************************************************************************

   // history buffer for LSF FILTER
   .CONST $audio_proc.vse.START_HISTORY_BUF_LSF        1*ADDR_PER_WORD + $audio_proc.vse.PEAK_HIST_BUF_SIZE;    
   .CONST $audio_proc.vse.LSF_HIST_BUF_SIZE            9*ADDR_PER_WORD + $audio_proc.vse.START_HISTORY_BUF_LSF; // y_l,y_l',y_l'',x ,x',x'',y_h,y_h',y_h''
   
   .CONST $audio_proc.vse.out_ipsi                     1*ADDR_PER_WORD + $audio_proc.vse.LSF_HIST_BUF_SIZE; 
   .CONST $audio_proc.vse.out_contra                   1*ADDR_PER_WORD + $audio_proc.vse.out_ipsi;
   .CONST $audio_proc.vse.FILTER_COEFF_FIELD           1*ADDR_PER_WORD + $audio_proc.vse.out_contra;  
   .CONST $audio_proc.vse.STRUC_SIZE                  24 + ($audio_proc.vse.FILTER_COEFF_FIELD >> LOG2_ADDR_PER_WORD);          

#endif // AUDIO_3dv_HEADER_INCLUDED
