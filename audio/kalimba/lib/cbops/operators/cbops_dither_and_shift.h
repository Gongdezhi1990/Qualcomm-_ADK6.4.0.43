// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************


#ifndef CBOPS_DITHER_HEADER_INCLUDED
#define CBOPS_DITHER_HEADER_INCLUDED

    // defining operator structure
   .CONST   $cbops.dither_and_shift.INPUT_START_INDEX_FIELD              0*ADDR_PER_WORD;  // index of the input buffer
   .CONST   $cbops.dither_and_shift.OUTPUT_START_INDEX_FIELD             1*ADDR_PER_WORD;  // index of the output buffer
   .CONST   $cbops.dither_and_shift.SHIFT_AMOUNT_FIELD                   2*ADDR_PER_WORD;  // amount of shift after dithering
   .CONST   $cbops.dither_and_shift.DITHER_TYPE_FIELD                    3*ADDR_PER_WORD;  // type of dither
   .CONST   $cbops.dither_and_shift.DITHER_FILTER_HIST_FIELD             4*ADDR_PER_WORD;  // history buffer for dithering (size = $cbops.dither_and_shift.FILTER_COEFF_SIZE in words)
   .CONST   $cbops.dither_and_shift.ENABLE_COMPRESSOR_FIELD              5*ADDR_PER_WORD;  // if NZ maps [-inf 0]dB to [-inf 1]dB before dithering
   .CONST   $cbops.dither_and_shift.STRUC_SIZE                           6;

    // defining dither types
   .CONST   $cbops.dither_and_shift.DITHER_TYPE_NONE                     0;  // no dither, just shift
   .CONST   $cbops.dither_and_shift.DITHER_TYPE_TRIANGULAR               1;  // add triangular pdf dither before shifting
   .CONST   $cbops.dither_and_shift.DITHER_TYPE_SHAPED                   2;  // add dither and shape quantization noise, then shift

   #define  FILTER_COEFF_SIZE_5

   #if defined(FILTER_COEFF_SIZE_5)
      .CONST   $cbops.dither_and_shift.FILTER_COEFF_SIZE                 5;  // 7 or 5
   #endif

   #if defined(FILTER_COEFF_SIZE_7)
      .CONST   $cbops.dither_and_shift.FILTER_COEFF_SIZE                 7;  // 7 or 5
   #endif

#endif // CBOPS_DITHER_HEADER_INCLUDED
