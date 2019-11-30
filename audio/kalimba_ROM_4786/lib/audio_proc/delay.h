// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef AUDIO_DELAY_HEADER_INCLUDED
#define AUDIO_DELAY_HEADER_INCLUDED

   // Pointer to input audio stream
   .CONST $audio_proc.delay.INPUT_ADDR_FIELD             0*ADDR_PER_WORD;
   // Pointer to output audio stream
   .CONST $audio_proc.delay.OUTPUT_ADDR_FIELD            1*ADDR_PER_WORD;
   // Pointer to delay buffer
   // Allocate to different data bank with input/output stream for lower cycles
   .CONST $audio_proc.delay.DBUFF_ADDR_FIELD             2*ADDR_PER_WORD;
   // Delay length in samples
   .CONST $audio_proc.delay.DELAY_FIELD                  3*ADDR_PER_WORD;
   // Structure size of delay data object
   .CONST $audio_proc.delay.STRUC_SIZE                   4;

#endif // AUDIO_DELAY_HEADER_INCLUDED
