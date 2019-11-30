// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef STREAM_MIXER_HEADER_INCLUDED
#define STREAM_MIXER_HEADER_INCLUDED

.CONST      $M.audio_proc.stream_mixer.OFFSET_INPUT_CH1_PTR      0*ADDR_PER_WORD;
.CONST      $M.audio_proc.stream_mixer.OFFSET_INPUT_CH2_PTR      1*ADDR_PER_WORD;
.CONST      $M.audio_proc.stream_mixer.OFFSET_OUTPUT_PTR         2*ADDR_PER_WORD;
.CONST      $M.audio_proc.stream_mixer.OFFSET_PTR_CH1_MANTISSA   3*ADDR_PER_WORD;
.CONST      $M.audio_proc.stream_mixer.OFFSET_PTR_CH2_MANTISSA   4*ADDR_PER_WORD;
.CONST      $M.audio_proc.stream_mixer.OFFSET_PTR_EXPONENT       5*ADDR_PER_WORD;
.CONST      $M.audio_proc.stream_mixer.STRUC_SIZE                6;

#endif //STREAM_MIXER_HEADER_INCLUDED
