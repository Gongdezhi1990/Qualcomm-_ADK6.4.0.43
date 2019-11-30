// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef MIC_GROUPING_HEADER_INCLUDED
#define MIC_GROUPING_HEADER_INCLUDED

   
   .CONST $audio_proc.mic_grouping.TWO_OVER_THREE                  0.66666666666;
   .CONST $audio_proc.mic_grouping.ONE_OVER_THREE                  0.33333333333;
   .CONST $audio_proc.mic_grouping.ONE_OVER_TWO                    0.5;
   .CONST $audio_proc.mic_grouping.IS_THREE_MICS                   3;
   
    
   // MIC grouping data object
   // Pointer to left channel audio stream 
   .CONST $audio_proc.mic_grouping.LEFT_CHANNEL_FIELD                  0*ADDR_PER_WORD;
   // Pointer to right channel audio stream
   .CONST $audio_proc.mic_grouping.RIGHT_CHANNEL_FIELD                 1*ADDR_PER_WORD;
   // Pointer to mic3 channel stream
   .CONST $audio_proc.mic_grouping.MIC3_CHANNEL_FIELD                  2*ADDR_PER_WORD;
   // Pointer to mic4 channel stream
   .CONST $audio_proc.mic_grouping.MIC4_CHANNEL_FIELD                  3*ADDR_PER_WORD;
   // Number of microphones 
   .CONST $audio_proc.mic_grouping.NUM_MICS_FIELD                      4*ADDR_PER_WORD;
   // Structure size of mic grouping data object
   .CONST $audio_proc.mic_grouping.STRUC_SIZE                          5;
   
#endif // MIC_GROUPING_HEADER_INCLUDED
