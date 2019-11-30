// *****************************************************************************
// Copyright (c) 2015 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_COMPANDER_HEADER_INCLUDED
#define AUDIO_COMPANDER_HEADER_INCLUDED

#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "core_library.h"
#endif
   
   // *****************************************************************************
   //                               compander parameter structure
   // *****************************************************************************
   // compander configuration word
   // format : M.0
   .CONST $audio_proc.compander.parameter.COMPANDER_CONFIG           0;
   // number of sections in the gain curve
   // format : integer
   .CONST $audio_proc.compander.parameter.NUM_SECTIONS               ADDR_PER_WORD + $audio_proc.compander.parameter.COMPANDER_CONFIG;
   // gain ratio for section 1
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_RATIO_SECTION1        ADDR_PER_WORD + $audio_proc.compander.parameter.NUM_SECTIONS;
   // gain ratio for section 2
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_RATIO_SECTION2        ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_RATIO_SECTION1;
   // gain ratio for section 3
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_RATIO_SECTION3        ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_RATIO_SECTION2;
   // gain ratio for section 4
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_RATIO_SECTION4        ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_RATIO_SECTION3;
   // gain ratio for section 5
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_RATIO_SECTION5        ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_RATIO_SECTION4;
   // gain threshold for section 1
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION1    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_RATIO_SECTION5;
   // gain threshold for section 2
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION2    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION1;
   // gain threshold for section 3
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION3    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION2;
   // gain threshold for section 4
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION4    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION3;
   // gain threshold for section 5
   // format : 5.N
   .CONST $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION5    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION4;
   // gain kneewidth for section 1
   // format : 1.N
   .CONST $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION1    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION5;
   // gain kneewidth for section 2
   // format : 1.N
   .CONST $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION2    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION1;
   // gain kneewidth for section 3
   // format : 1.N
   .CONST $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION3    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION2;
   // gain kneewidth for section 4
   // format : 1.N
   .CONST $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION4    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION3;
   // gain kneewidth for section 5
   // format : 1.N
   .CONST $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION5    ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION4;
   // gain attack time
   // format : 4.N
   .CONST $audio_proc.compander.parameter.GAIN_ATTACK_TC             ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION5;
   // gain release time
   // format : 4.N
   .CONST $audio_proc.compander.parameter.GAIN_RELEASE_TC            ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_ATTACK_TC;
   // level attack time
   // format : 4.N
   .CONST $audio_proc.compander.parameter.LEVEL_ATTACK_TC            ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_RELEASE_TC;
   // level release time
   // format : 4.N
   .CONST $audio_proc.compander.parameter.LEVEL_RELEASE_TC           ADDR_PER_WORD + $audio_proc.compander.parameter.LEVEL_ATTACK_TC;
   // level average time
   // format : 4.N
   .CONST $audio_proc.compander.parameter.LEVEL_AVERAGE_TC           ADDR_PER_WORD + $audio_proc.compander.parameter.LEVEL_RELEASE_TC;
   // makeup gain
   // format : 2.N
   .CONST $audio_proc.compander.parameter.MAKEUP_GAIN                ADDR_PER_WORD + $audio_proc.compander.parameter.LEVEL_AVERAGE_TC;
   // lookahead time
   // format : 1.N
   .CONST $audio_proc.compander.parameter.LOOKAHEAD_TIME             ADDR_PER_WORD + $audio_proc.compander.parameter.MAKEUP_GAIN;
   // level estimation method 1:peak 0:rms
   // format : integer
   .CONST $audio_proc.compander.parameter.LEVEL_ESTIMATION_FLAG      ADDR_PER_WORD + $audio_proc.compander.parameter.LOOKAHEAD_TIME;
   // gain update flag 0:sample-based 1:block-based
   // format : integer
   .CONST $audio_proc.compander.parameter.GAIN_UPDATE_FLAG           ADDR_PER_WORD + $audio_proc.compander.parameter.LEVEL_ESTIMATION_FLAG;
   // gain interpolation flag 0:gain interpolation off 1:gain interpolation on
   // format : integer
   .CONST $audio_proc.compander.parameter.GAIN_INTERP_FLAG           ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_UPDATE_FLAG;
   // Soft Knee section 1 coeff a
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_1_COEFF_A        ADDR_PER_WORD + $audio_proc.compander.parameter.GAIN_INTERP_FLAG;
   // Soft Knee section 1 coeff b
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_1_COEFF_B        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_1_COEFF_A;
   // Soft Knee section 1 coeff c
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_1_COEFF_C        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_1_COEFF_B;
   // Soft Knee section 2 coeff a
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_2_COEFF_A        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_1_COEFF_C;
   // Soft Knee section 2 coeff b
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_2_COEFF_B        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_2_COEFF_A;
   // Soft Knee section 2 coeff c
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_2_COEFF_C        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_2_COEFF_B;
   // Soft Knee section 3 coeff a
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_3_COEFF_A        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_2_COEFF_C;
   // Soft Knee section 3 coeff b
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_3_COEFF_B        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_3_COEFF_A;
   // Soft Knee section 3 coeff c
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_3_COEFF_C        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_3_COEFF_B;
   // Soft Knee section 4 coeff a
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_4_COEFF_A        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_3_COEFF_C;
   // Soft Knee section 4 coeff b
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_4_COEFF_B        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_4_COEFF_A;
   // Soft Knee section 4 coeff c
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_4_COEFF_C        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_4_COEFF_B;
   // Soft Knee section 5 coeff a
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_5_COEFF_A        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_4_COEFF_C;
   // Soft Knee section 5 coeff b
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_5_COEFF_B        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_5_COEFF_A;
   // Soft Knee section 5 coeff c
   // format : 14.N
   .CONST $audio_proc.compander.parameter.SOFT_KNEE_5_COEFF_C        ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_5_COEFF_B;
   // compander parameters structure size
   // format : integer
   .CONST $audio_proc.compander.parameter.STRUCT_SIZE                ADDR_PER_WORD + $audio_proc.compander.parameter.SOFT_KNEE_5_COEFF_C;



   // *****************************************************************************
   //                             compander data object structure
   // *****************************************************************************
   // -------------------------------- START PUBLIC SECTION ---------------------------------
   // pointer to data_objects of all channels
   // format : integer
   .CONST $audio_proc.compander.DATA_OBJECTS_PTR                     0;
   // number of channels
   // format : integer
   .CONST $audio_proc.compander.NUM_CHANNELS                         ADDR_PER_WORD + $audio_proc.compander.DATA_OBJECTS_PTR;
   // sampling rate
   // format : integer
   .CONST $audio_proc.compander.SAMPLE_RATE_FIELD                    ADDR_PER_WORD + $audio_proc.compander.NUM_CHANNELS;
   // pointer to parameters
   // format : integer
   .CONST $audio_proc.compander.PARAM_PTR_FIELD                      ADDR_PER_WORD + $audio_proc.compander.SAMPLE_RATE_FIELD;
   // pointer to channel input
   // format : integer
   .CONST $audio_proc.compander.CHANNEL_INPUT_PTR                    ADDR_PER_WORD + $audio_proc.compander.PARAM_PTR_FIELD;
   // pointer to channel output
   // format : integer
   .CONST $audio_proc.compander.CHANNEL_OUTPUT_PTR                   ADDR_PER_WORD + $audio_proc.compander.CHANNEL_INPUT_PTR;
   // number of samples to process in current frame
   // format : integer
   .CONST $audio_proc.compander.SAMPLES_TO_PROCESS                   ADDR_PER_WORD + $audio_proc.compander.CHANNEL_OUTPUT_PTR;
   // *****************************************************************************
   // -------------------------------- START INTERNAL SECTION -----------------------------------------------------
   // *****************************************************************************
   // flag indicating input is framebuffer/cbuffer
   // format : integer
   .CONST $audio_proc.compander.FRAMEBUFFER_FLAG                     ADDR_PER_WORD + $audio_proc.compander.SAMPLES_TO_PROCESS;
   // input read address for current block
   // format : integer
   .CONST $audio_proc.compander.INPUT_READ_ADDR                      ADDR_PER_WORD + $audio_proc.compander.FRAMEBUFFER_FLAG;
   // output write address for current block
   // format : integer
   .CONST $audio_proc.compander.OUTPUT_WRITE_ADDR                    ADDR_PER_WORD + $audio_proc.compander.INPUT_READ_ADDR;
   // forgetting factor for level_attack_tc
   // format : 1.N
   .CONST $audio_proc.compander.LVL_ALPHA_ATK                        ADDR_PER_WORD + $audio_proc.compander.OUTPUT_WRITE_ADDR;
   // forgetting factor for level_release_tc
   // format : 1.N
   .CONST $audio_proc.compander.LVL_ALPHA_RLS                        ADDR_PER_WORD + $audio_proc.compander.LVL_ALPHA_ATK;
   // forgetting factor for level_average_tc
   // format : 1.N
   .CONST $audio_proc.compander.LVL_ALPHA_AVG                        ADDR_PER_WORD + $audio_proc.compander.LVL_ALPHA_RLS;
   // slope for section 1
   // format : 5.N
   .CONST $audio_proc.compander.SLOPE1                               ADDR_PER_WORD + $audio_proc.compander.LVL_ALPHA_AVG;
   // slope for section 2
   // format : 5.N
   .CONST $audio_proc.compander.SLOPE2                               ADDR_PER_WORD + $audio_proc.compander.SLOPE1;
   // slope for section 3
   // format : 5.N
   .CONST $audio_proc.compander.SLOPE3                               ADDR_PER_WORD + $audio_proc.compander.SLOPE2;
   // slope for section 4
   // format : 5.N
   .CONST $audio_proc.compander.SLOPE4                               ADDR_PER_WORD + $audio_proc.compander.SLOPE3;
   // slope for section 5
   // format : 5.N
   .CONST $audio_proc.compander.SLOPE5                               ADDR_PER_WORD + $audio_proc.compander.SLOPE4;
   // slope for section 6
   // format : 5.N
   .CONST $audio_proc.compander.SLOPE6                               ADDR_PER_WORD + $audio_proc.compander.SLOPE5;
   // Intercept for section 1
   // format : 9.N
   .CONST $audio_proc.compander.INTERCEPT1                           ADDR_PER_WORD + $audio_proc.compander.SLOPE6; 
   // Intercept for section 2
   // format : 9.N
   .CONST $audio_proc.compander.INTERCEPT2                           ADDR_PER_WORD + $audio_proc.compander.INTERCEPT1;
   // Intercept for section 3
   // format : 9.N
   .CONST $audio_proc.compander.INTERCEPT3                           ADDR_PER_WORD + $audio_proc.compander.INTERCEPT2;
   // Intercept for section 4
   // format : 9.N
   .CONST $audio_proc.compander.INTERCEPT4                           ADDR_PER_WORD + $audio_proc.compander.INTERCEPT3;
   // Intercept for section 5
   // format : 9.N
   .CONST $audio_proc.compander.INTERCEPT5                           ADDR_PER_WORD + $audio_proc.compander.INTERCEPT4;
   // Intercept for section 6
   // format : 9.N
   .CONST $audio_proc.compander.INTERCEPT6                           ADDR_PER_WORD + $audio_proc.compander.INTERCEPT5;
   // Frequency of gain update(in samples)
   // format : integer
   .CONST $audio_proc.compander.GAIN_UPDATE_RATE                     ADDR_PER_WORD + $audio_proc.compander.INTERCEPT6;
   // forgetting factor for gain_attack_tc
   // format : 1.N
   .CONST $audio_proc.compander.GAIN_ALPHA_ATK                       ADDR_PER_WORD + $audio_proc.compander.GAIN_UPDATE_RATE;    
   // forgetting factor for gain_release_tc
   // format : 1.N
   .CONST $audio_proc.compander.GAIN_ALPHA_RLS                       ADDR_PER_WORD + $audio_proc.compander.GAIN_ALPHA_ATK; 
   // history value used in level detection
   // format : 1.N
   .CONST $audio_proc.compander.LEVEL_DETECT_LAST_SAMPLE_HIST        ADDR_PER_WORD + $audio_proc.compander.GAIN_ALPHA_RLS;
   // peak/rms value for current block
   // format : 1.N
   .CONST $audio_proc.compander.LEVEL_DETECT_CHN_MAX_OR_MEAN         ADDR_PER_WORD + $audio_proc.compander.LEVEL_DETECT_LAST_SAMPLE_HIST;
   // log2(peak/rms)value for current block
   // format : 8.N
   .CONST $audio_proc.compander.LEVEL_DETECT_CHN_MAX_OR_MEAN_LOG2    ADDR_PER_WORD + $audio_proc.compander.LEVEL_DETECT_CHN_MAX_OR_MEAN;
   // Gain smoothing history (in log2 domain)
   // format : 8.N
   .CONST $audio_proc.compander.GAIN_SMOOTH_HIST                     ADDR_PER_WORD + $audio_proc.compander.LEVEL_DETECT_CHN_MAX_OR_MEAN_LOG2;
   // pointer to lookahead history buffer
   // format : integer
   .CONST $audio_proc.compander.PTR_LOOKAHEAD_HIST                   ADDR_PER_WORD + $audio_proc.compander.GAIN_SMOOTH_HIST;
   // base address for lookahead history buffer
   // format : integer
   .CONST $audio_proc.compander.START_LOOKAHEAD_HIST                 ADDR_PER_WORD + $audio_proc.compander.PTR_LOOKAHEAD_HIST;
   // number of lookahead samples
   // format : integer
   .CONST $audio_proc.compander.LOOKAHEAD_SAMPLES                    ADDR_PER_WORD + $audio_proc.compander.START_LOOKAHEAD_HIST;
   // inverse of the number of channels
   // format : 1.N
   .CONST $audio_proc.compander.NUM_CHANNELS_INV                     ADDR_PER_WORD + $audio_proc.compander.LOOKAHEAD_SAMPLES;
   // inverse of the gain update rate
   // format : 1.N
   .CONST $audio_proc.compander.GAIN_UPDATE_RATE_INV                 ADDR_PER_WORD + $audio_proc.compander.NUM_CHANNELS_INV;
   // Gain smoothing history (in linear domain)
   // format : 1.N
   .CONST $audio_proc.compander.GAIN_SMOOTH_HIST_LINEAR              ADDR_PER_WORD + $audio_proc.compander.GAIN_UPDATE_RATE_INV;  
   // function pointer for peak/rms level_detect function
   // format : integer
   .CONST $audio_proc.compander.LEVEL_DETECT_FUNC_PTR                ADDR_PER_WORD + $audio_proc.compander.GAIN_SMOOTH_HIST_LINEAR;                 
   // compander data_object structure size
   // format : integer
   .CONST $audio_proc.compander.STRUCT_SIZE                          ADDR_PER_WORD + $audio_proc.compander.LEVEL_DETECT_FUNC_PTR;
    
#endif // AUDIO_COMPANDER_HEADER_INCLUDED
