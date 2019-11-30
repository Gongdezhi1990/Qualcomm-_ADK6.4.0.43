// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_PEQ_HEADER_INCLUDED
#define AUDIO_PEQ_HEADER_INCLUDED

#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "core_library.h"
#endif

   // PEQ Parameters (for best performance, allocate in BANK1)

   // PEQ config word: metadata(upper bits) + number of stages (LSBs)
   // Number of stage at lower LSBs should be masked out before any use
   // Use $audio_proc.peq.const.NUM_STAGES_MASK for masking

    .CONST $audio_proc.peq.parameter.NUM_STAGES_FIELD      0*ADDR_PER_WORD;
    .CONST $audio_proc.peq.parameter.GAIN_EXPONENT_FIELD   1*ADDR_PER_WORD;
    .CONST $audio_proc.peq.parameter.GAIN_MANTISA__FIELD   2*ADDR_PER_WORD;
    .CONST $audio_proc.peq.parameter.STAGES_SCALES         3*ADDR_PER_WORD;
    //
    // New format: interleaved ceofficients and scalefactors (sh/dh/hq_peq.x):
    //   COEFF+SCALE_FACORS = <MAX_STAGES * 6> fields: 
    //      b2_0,b1_0,b0_0,a2_0,a1_0,SF_0
    //      b2_1,b1_1,b0_1,a2_1,a1_1,SF_1
    //      b2_N,...,SF_N
    //
    // Legacy format (peq.x):
    //   COEFF+SCALE_FACORS = <MAX_STAGES * 6> fields: 
    //      b2_0,b1_0,b0_0,a2_0,a1_0
    //      b2_1,b1_1,b0_1,a2_1,a1_1
    //      b2_N,...
    //      SF_0,SF_1,SF_N
    
    
   // PEQ Data Structure (for best performance, allocate in BANK2)   
    
   // Pointer to input audio stream (for best performance, allocate in BANK2)
   .CONST $audio_proc.peq.INPUT_ADDR_FIELD               0*ADDR_PER_WORD;
   
   // Pointer to output audio stream (for best performance, allocate in BANK1)
   .CONST $audio_proc.peq.OUTPUT_ADDR_FIELD              1*ADDR_PER_WORD;
   
   // Maximum stages for PEQ
   .CONST $audio_proc.peq.MAX_STAGES_FIELD               2*ADDR_PER_WORD;
   
   // Pointer to PEQ parameters (for best performance, allocate in BANK1)
   .CONST $audio_proc.peq.PARAM_PTR_FIELD                3*ADDR_PER_WORD;  
   
   // ***************** INTERNAL FIELDS DO NOT SET! *********************
   
   // Pointer to PEQ delay buffer
   // Minimum size of the buffer: 2 * (number of stages + 1)
   .CONST $audio_proc.peq.DELAYLINE_START_ADDR_FIELD     4*ADDR_PER_WORD;
   .CONST $audio_proc.peq.DELAYLINE_ADDR_FIELD           5*ADDR_PER_WORD;

   // Pointer to PEQ filter coefficients buffer
   .CONST $audio_proc.peq.COEFS_ADDR_FIELD               6*ADDR_PER_WORD;
   
   // PEQ config word: metadata(upper bits) + number of stages (LSBs)
   // Number of stage at lower LSBs should be masked out before any use
   // Use $audio_proc.peq.const.NUM_STAGES_MASK for masking
   .CONST $audio_proc.peq.NUM_STAGES_FIELD               7*ADDR_PER_WORD;

   // Size of delay line circular buffer
   // This field is set by initialization routine based on NUM_STAGES_FIELD
   .CONST $audio_proc.peq.DELAYLINE_SIZE_FIELD           8*ADDR_PER_WORD;
   
   // Size of filter coefficients circular buffer
   // This field is set by initialization routine based on NUM_STAGES_FIELD
   .CONST $audio_proc.peq.COEFS_SIZE_FIELD               9*ADDR_PER_WORD;
   
   // Number of headroom bits
   .CONST $audio_proc.peq.HEADROOM_BITS                  10*ADDR_PER_WORD;
    
    // Structure size of PEQ data object
   .CONST $audio_proc.peq.STRUC_SIZE                     11;

  
    //  If using base register append delay buffer to data object  
#define PEQ_OBJECT_SIZE(x)  ($audio_proc.peq.STRUC_SIZE + 2*((x)+1) )
#define HQ_PEQ_OBJECT_SIZE(x)  ($audio_proc.peq.STRUC_SIZE + ((3*(x))+2) )
#define DH_PEQ_OBJECT_SIZE(x)  ($audio_proc.peq.STRUC_SIZE + 4*((x)+1) )
    // number of shifts applied to audio to provide extra headroom
    .CONST $audio_proc.peq.LEGACY_PEQ_HEADROOM_SHIFTS    2;
    .CONST $audio_proc.peq.SH_PEQ_HEADROOM_SHIFTS        0;
    .CONST $audio_proc.peq.HQ_PEQ_HEADROOM_SHIFTS        0;
    .CONST $audio_proc.peq.DH_PEQ_HEADROOM_SHIFTS        0;

   // Number of Stage Mask
   .CONST $audio_proc.peq.const.NUM_STAGES_MASK          0xFF;

#endif // AUDIO_PEQ_HEADER_INCLUDED
