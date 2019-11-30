// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef CBOPS_MIX_HEADER_INCLUDED
#define CBOPS_MIX_HEADER_INCLUDED

   .CONST   $cbops.mix.MIX_SOURCE_FIELD                  0;
   .CONST   $cbops.mix.MIX_VOL_FIELD                     1;
   .CONST   $cbops.mix.AUDIO_VOL_FIELD                   2;
   .CONST   $cbops.mix.MIXING_STATE_FIELD                3;
   .CONST   $cbops.mix.MIXING_START_LEVEL_FIELD          4;
   .CONST   $cbops.mix.NUMBER_OF_INPUTS_FIELD            5;
   .CONST   $cbops.mix.INPUT_START_INDEX_FIELD           6;

   // The amount of tone data which is buffered before mixing the
   // tone stream with the audio stream, set this to quite a large value
   // so that the audio stream is not limited by tone data.
   .CONST    $cbops.mix.MIX_INPUT_START_LEVEL            118;

   .CONST    $cbops.mix.MIXING_STATE_STOPPED             0;
   .CONST    $cbops.mix.MIXING_STATE_MIXING              1;

#endif // CBOPS_MIX_HEADER_INCLUDED
