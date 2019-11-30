// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef KYMERA
#include "core_library.h"
#endif

#ifndef MATH_FFT_HEADER_INCLUDED
#define MATH_FFT_HEADER_INCLUDED

   .CONST   $fft.STRUC_SIZE            3;
   .CONST   $fft.NUM_POINTS_FIELD      0*ADDR_PER_WORD;
   .CONST   $fft.REAL_ADDR_FIELD       1*ADDR_PER_WORD;
   .CONST   $fft.IMAG_ADDR_FIELD       2*ADDR_PER_WORD;

// In this variant, fft twiddle factors are stored externally ...   
   #if defined(FFT_LOW_RAM)
      .CONST $FFT_LOW_RAM_USED       1;
   #endif

#endif
