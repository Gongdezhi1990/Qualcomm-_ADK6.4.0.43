// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef CBOPS_UPSAMPLE_MIX_HEADER_INCLUDED
#define CBOPS_UPSAMPLE_MIX_HEADER_INCLUDED

   .CONST   $cbops.upsample_mix.INPUT_START_INDEX_FIELD             0;
   .CONST   $cbops.upsample_mix.TONE_SOURCE_FIELD                   2;
   .CONST   $cbops.upsample_mix.TONE_VOL_FIELD                      3;
   .CONST   $cbops.upsample_mix.AUDIO_VOL_FIELD                     4;
   .CONST   $cbops.upsample_mix.RESAMPLE_COEFS_ADDR_FIELD           5;
   .CONST   $cbops.upsample_mix.RESAMPLE_COEFS_SIZE_FIELD           6;
   .CONST   $cbops.upsample_mix.RESAMPLE_BUFFER_ADDR_FIELD          7;
   .CONST   $cbops.upsample_mix.RESAMPLE_BUFFER_SIZE_FIELD          8;
   .CONST   $cbops.upsample_mix.UPSAMPLE_RATIO_FIELD                9;
   .CONST   $cbops.upsample_mix.INTERP_RATIO_FIELD                  10;
   .CONST   $cbops.upsample_mix.INTERP_COEF_CURRENT_FIELD           11;
   .CONST   $cbops.upsample_mix.INTERP_LAST_VAL_FIELD               12;
   .CONST   $cbops.upsample_mix.TONE_PLAYING_STATE_FIELD            13;
   .CONST   $cbops.upsample_mix.TONE_DATA_AMOUNT_READ_FIELD         14;
   .CONST   $cbops.upsample_mix.TONE_DATA_AMOUNT_FIELD              15;
   .CONST   $cbops.upsample_mix.LOCATION_IN_LOOP_FIELD              16;
   .CONST   $cbops.upsample_mix.STRUC_SIZE                          17;
   // this is the amount of tone data which is buffered before mixing the
   // tone stream with the audio stream, set this to quite a large value
   // so that the audio stream is not limited by tone data.
   .CONST   $cbops.upsample_mix.TONE_START_LEVEL                    118;
   // this is amount of data the routine consumes before updating the read
   // handles. NOTE this value has no effect if the tone source is a cbuffer.
   // If however the tone source is a PORT this stops the routine sending too
   // many message to the firmware, default is one message per 9ms, ie 8kHz
   // -> 8samples per ms, 72 samples -> 9ms.
   .CONST   $cbops.upsample_mix.TONE_BLOCK_SIZE                     72;

   .CONST   $cbops.upsample_mix.TONE_PLAYING_STATE_STOPPED          0;
   .CONST   $cbops.upsample_mix.TONE_PLAYING_STATE_PLAYING          1;

   .CONST   $cbops.upsample_mix.NO_BUFFER                          -1;

   // the length of this delay line is given by (QUALITY_FACTOR * 2)
   // length = 5 * 2 = 10
   .CONST $cbops.upsample_mix.RESAMPLE_BUFFER_LENGTH_HIGH_QUALITY   10;

   // the length of this delay line is given by (QUALITY_FACTOR * 2)
   // length = 2 * 2 = 4
   .CONST $cbops.upsample_mix.RESAMPLE_BUFFER_LENGTH_LOW_QUALITY    4;

#endif // CBOPS_UPSAMPLE_MIX_HEADER_INCLUDED
