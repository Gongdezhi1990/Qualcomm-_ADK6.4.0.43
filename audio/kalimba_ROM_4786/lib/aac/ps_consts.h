// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef PS_CONSTS_HEADER_INCLUDED
#define PS_CONSTS_HEADER_INCLUDED

   .CONST $aacdec.PS_EXT_ID_PARAMETRIC_STEREO                 2;

   .CONST $aacdec.PS_MAX_NUM_PARAMETERS                       34;
   .CONST $aacdec.PS_MIN_NUM_PARAMETERS                       10;
   .CONST $aacdec.PS_NUM_PAR_BANDS_IN_BASELINE_DECORRELATION  20;

   .CONST $aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH        13;
   .CONST $aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS           8;
   .CONST $aacdec.PS_HYBRID_TYPE_B_NUM_SUB_SUBBANDS           2;
   .CONST $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS   3;

   // variables held in tmp[] during Hybrid Analysis
   .CONST $aacdec.PS_HYBRID_ANALYSIS_TEMP_R8                        0*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET                1*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_QMF_SUBBAND                             2*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_TYPE_A_FIR_TEMP_R10                     3*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_ONE                 4*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_TWO                 5*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_THREE               6*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_FOUR                7*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_TYPE_A_FIR_REAL_IMAG_FLAG               8*ADDR_PER_WORD;
   .CONST $aacdec.PS_HYBRID_TYPE_A_FIR_IFFT_LOOP_INDICE             9*ADDR_PER_WORD;


   .CONST $aacdec.PS_IID_NUM_QUANT_STEPS_FINE_RES    15;
   .CONST $aacdec.PS_IID_NUM_QUANT_STEPS_COARSE_RES  7;

   .CONST $aacdec.PS_MAX_NUM_ENVELOPES            6;
   .CONST $aacdec.PS_NUM_SAMPLES_PER_FRAME        32;


   .CONST $aacdec.PS_EXT_DATA_READ                                  0*ADDR_PER_WORD;
   .CONST $aacdec.PS_ENABLE_IID                                     1*ADDR_PER_WORD;
   .CONST $aacdec.PS_ENABLE_ICC                                     2*ADDR_PER_WORD;
   .CONST $aacdec.PS_IID_MODE                                       3*ADDR_PER_WORD;
   .CONST $aacdec.PS_NR_IID_PAR                                     4*ADDR_PER_WORD;
   .CONST $aacdec.PS_ICC_MODE                                       5*ADDR_PER_WORD;
   .CONST $aacdec.PS_NR_ICC_PAR                                     6*ADDR_PER_WORD;
   .CONST $aacdec.PS_NR_IPDOPD_PAR                                  7*ADDR_PER_WORD;
   .CONST $aacdec.PS_ENABLE_EXT                                     8*ADDR_PER_WORD;
   .CONST $aacdec.PS_FRAME_CLASS                                    9*ADDR_PER_WORD;
   .CONST $aacdec.PS_NUM_ENV                                        10*ADDR_PER_WORD;
   .CONST $aacdec.PS_BORDER_POSITION                                11*ADDR_PER_WORD;
   .CONST $aacdec.PS_IID_CODING_DIRECTION                           17*ADDR_PER_WORD;
   .CONST $aacdec.PS_ICC_CODING_DIRECTION                           21*ADDR_PER_WORD;
   .CONST $aacdec.PS_IPD_CODING_DIRECTION                           25*ADDR_PER_WORD;
   .CONST $aacdec.PS_OPD_CODING_DIRECTION                           29*ADDR_PER_WORD;
   .CONST $aacdec.PS_BIT_COUNT_PRE_EXTENSION_DATA                   33*ADDR_PER_WORD;
   .CONST $aacdec.PS_BIT_COUNT_PRE_DATA                             34*ADDR_PER_WORD;
   .CONST $aacdec.PS_IID_INDEX                                      35*ADDR_PER_WORD;
   .CONST $aacdec.PS_ICC_INDEX                                      $aacdec.PS_IID_INDEX + ($aacdec.PS_MAX_NUM_PARAMETERS * $aacdec.PS_MAX_NUM_ENVELOPES*ADDR_PER_WORD);
   .CONST $aacdec.PS_IID_INDEX_PREV                                 $aacdec.PS_ICC_INDEX + ($aacdec.PS_MAX_NUM_PARAMETERS * $aacdec.PS_MAX_NUM_ENVELOPES*ADDR_PER_WORD);
   .CONST $aacdec.PS_ICC_INDEX_PREV                                 $aacdec.PS_IID_INDEX_PREV + $aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD;
   .CONST $aacdec.PS_HUFFMAN_TABLES_TOTAL_SIZE                      $aacdec.PS_ICC_INDEX_PREV + $aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD;
   .CONST $aacdec.PS_G_TRANSIENT_RATIO_ADDR                         512*ADDR_PER_WORD;
   .CONST $aacdec.PS_ALLPASS_FEEDBACK_BUFFER_INDICES                513*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_LONG_DELAY_BAND_BUFFER_INDICE    515*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_SHORT_DELAY_BAND_BUFFER_INDICE   516*ADDR_PER_WORD;
   .CONST $aacdec.PS_SAVE_CONVERT_MONO_TO_STEREO                    517*ADDR_PER_WORD;


   .CONST $aacdec.PS_INFO_SIZE                                      518;


   .CONST $aacdec.PS_ALPHA_DECAY                                 0.76592833836465;
   .CONST $aacdec.PS_ALPHA_SMOOTH                                0.25;
   .CONST $aacdec.PS_ONE_OVER_TRANSIENT_IMPACT_FACTOR            (1.0 / 1.5);
   .CONST $aacdec.PS_NUM_ALLPASS_LINKS                           3;
   .CONST $aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS             22;
   .CONST $aacdec.PS_DECORRELATION_SHORT_DELAY_BAND              35;
   .CONST $aacdec.CORE_AAC_AUDIO_FRAME_OUTPUT_SIZE_IN_WORDS      1024;

   .CONST $aacdec.PS_DECORRELATION_LONG_DELAY_IN_SAMPLES         14;
   .CONST $aacdec.PS_DECORRELATION_SHORT_DELAY_IN_SAMPLES        1;

   .CONST $aacdec.PS_NUM_HYBRID_SUB_SUBBANDS                     12;
   .CONST $aacdec.PS_NUM_NON_HYBRID_QMF_BANDS                    $aacdec.X_SBR_WIDTH - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS;

   .CONST $aacdec.PS_NUM_FREQ_BANDS_WHEN_20_PAR_BANDS            22;
   .CONST $aacdec.PS_NUM_HYBRID_FREQ_BANDS_WHEN_20_PAR_BANDS     10;

   .CONST $aacdec.PS_STEREO_MIXING_ONE_OVER_PI                   0.318309886183791;
   .CONST $aacdec.PS_STEREO_MIXING_SQTR_2_OVER_TWO               0.707106781186548;

   // variables held in tmp[] during Decorrelation
   .CONST $aacdec.PS_INPUT_POWER_MATRIX_BASE_ADDR                            0*ADDR_PER_WORD;             .CONST $aacdec.PS_LONG_DELAY_SAMPLE_BUFFER_INDICE             0*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_SK_IN_REAL_BASE_ADDR                      1*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_SK_IN_IMAG_BASE_ADDR                      2*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE                  3*ADDR_PER_WORD;             .CONST $aacdec.PS_DECORRELATION_INTER_SAMPLE_STRIDE           3*ADDR_PER_WORD;
   .CONST $aacdec.PS_GAIN_TRANSIENT_RATIO_ADDR                               4*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_END_SUBBAND_OF_FREQ_BAND                  5*ADDR_PER_WORD;
   .CONST $aacdec.PS_G_DECAY_SLOPE_FILTER_A                                  6*ADDR_PER_WORD;             .CONST $aacdec.PS_INPUT_POWER_MATRIX_ROW_ADDRESS              6*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_TEMP_R8                                   9*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_PHI_FRACT_REAL                            10*ADDR_PER_WORD;            .CONST $aacdec.PS_PARAMETER_BAND_POWER_SMOOTHED_MANTISSA      10*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_PHI_FRACT_IMAG                            11*ADDR_PER_WORD;            .CONST $aacdec.PS_PARAMETER_BAND_POWER_SMOOTHED_EXPONENT      11*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_DK_OUT_REAL_BASE_ADDR                     12*ADDR_PER_WORD;            .CONST $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE     12*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_DK_OUT_IMAG_BASE_ADDR                     13*ADDR_PER_WORD;            .CONST $aacdec.PS_TRANSIENT_DETECTOR_ADJUSTED_START_SUBBAND   13*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_G_DECAY_SLOPE_FILTER_TABLE_BASE_ADDR      14*ADDR_PER_WORD;            .CONST $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_MINUS_ONE      14*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES           15*ADDR_PER_WORD; /*[0:2]*/  .CONST $aacdec.PS_TRANSIENT_DETECTOR_SUBBAND_LOOP_BOUND       15*ADDR_PER_WORD;
                                                                                                          .CONST $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_START_FREQ     16*ADDR_PER_WORD;
                                                                                                          .CONST $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_END_FREQ       17*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_NON_HYBRID_QMF_CHANNEL_NUMBER             18*ADDR_PER_WORD;            .CONST $aacdec.PS_DECORRELATION_HYBRID_OR_QMF_SUBAND_NUMBER   18*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_HYBRID_MODE                               19*ADDR_PER_WORD;            .CONST $aacdec.PS_TRANSIENT_DETECTOR_HYBRID_MODE              19*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_SUBBAND_LOOP_BOUND                        20*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_REAL             21*ADDR_PER_WORD;            .CONST $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_WIDTH          21*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_IMAG             22*ADDR_PER_WORD;
   .CONST $aacdec.PS_SHORT_DELAY_SAMPLE_BUFFER_INDICE                        23*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_FLASH_TABLES_DM_ADDRESS                   24*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_ADDRESS           25*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_SIZE              26*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_NUMBER_HYBRID_OR_QMF_ALLPASS_FREQS        27*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_REAL_ADDR         28*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_IMAG_ADDR         29*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_TWO_ADDR    30*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ONE_ADDR    31*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ZERO_ADDR   32*ADDR_PER_WORD;
   .CONST $aacdec.PS_DECORRELATION_PHI_FRACT_ALLPASS_REAL_SUBBAND_VALUES     33*ADDR_PER_WORD;  // [0:2]
   .CONST $aacdec.PS_DECORRELATION_PHI_FRACT_ALLPASS_IMAG_SUBBAND_VALUES     36*ADDR_PER_WORD;  // [0:2]
   .CONST $aacdec.PS_DECORRELATION_DELAY_IN_SAMPLES                          39*ADDR_PER_WORD;

   // variables held in tmp[] during Stereo Mixing
   .CONST $aacdec.PS_STEREO_MIXING_HYBRID_MODE                               0*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_FREQ_BAND_LOOP_BOUND                      1*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_END_SUBBAND_OF_FREQ_BAND                  2*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_TEMP_R8                                   3*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_ADJUSTED_END_SUBBAND_OF_FREQ_BAND         4*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_SCALE_FACTOR_ONE                          5*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_SCALE_FACTOR_TWO                          6*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_DELTA_H11                                 7*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_DELTA_H21                                 8*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_DELTA_H12                                 9*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_DELTA_H22                                 10*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_SK_REAL_BASE_ADDR                         11*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_SK_IMAG_BASE_ADDR                         12*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_DK_REAL_BASE_ADDR                         13*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_DK_IMAG_BASE_ADDR                         14*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_INTER_SAMPLE_STRIDE                       15*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_INTER_SUBBAND_STRIDE                      16*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_FREQ_BAND_START_FREQ                      17*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_ENVELOPE                                  18*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_BORDER_END                                19*ADDR_PER_WORD;
   .CONST $aacdec.PS_STEREO_MIXING_IID_ZERO_SCALE_FACTOR_TABLE_POINTER       20*ADDR_PER_WORD;

   .CONST $aacdec.PS_ONE_SQUARED_24_BIT_FRACTIONAL                        1.0;

   .CONST $aacdec.PS_COS_PI_OVER_EIGHT      0.923879532511287;
   .CONST $aacdec.PS_SIN_PI_OVER_EIGHT      0.382683432365090;
   .CONST $aacdec.PS_COS_PI_OVER_FOUR       0.707106781186548;

#endif
