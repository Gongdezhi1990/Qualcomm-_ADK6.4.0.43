// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifdef DEBUG_ON
   #define DEBUG_MATH
   #define ENABLE_PROFILER_MACROS
#endif

#include "profiler.h"
#include "sin.asm"
#include "cos.asm"
#include "sqrt.asm"
#include "inv_qdrt.asm"
#include "pow2.asm"
#include "log2.asm"
#include "rand.asm"
#include "vector_complex_multiply.asm"
#ifndef TODO_CRESCENDO_CVC_TEST
#include "iir.asm"
#endif // TODO_CRESCENDO_CVC_TEST
#include "atan.asm"
#include "div48.asm"
#include "misc.asm"
#include "filter_coefcalc_lib.asm"
#include "float_lib.asm"

// FFT: hard coded to support up to 512 point complex FFT
#define FFT_TWIDDLE_NEED_512_POINT  // AAC
#define FFT_TWIDDLE_NEED_256_POINT  // CVC 2 channel
#define FFT_TWIDDLE_NEED_128_POINT  // CVC 1 channal
#define FFT_LOW_RAM
#include "fft_twiddle.h"
#include "address_bitreverse.asm"
#include "fft.asm"
#include "fft_twiddle_alloc.asm"
