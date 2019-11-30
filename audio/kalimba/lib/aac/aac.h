// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef AAC_HEADER_INCLUDED
#define AAC_HEADER_INCLUDED

/* Please leave these defines commented out to give us an
   indication of the best place to enable them; in Kymera
   they should be passed from the build configuration. */
/* THIS IS THE PLACE TO GLOBALLY ENABLE SBR */
//#define AACDEC_SBR_ADDITIONS
/* THIS IS THE PLACE TO GLOBALLY ENABLE PS */
//#define AACDEC_PS_ADDITIONS
/* THIS IS THE PLACE TO GLOBALLY ENABLE ELD */
//#define AACDEC_ELD_ADDITIONS


   // -- Define build options check --
   // Never can have have PS additions but not SBR additions
   #ifndef AACDEC_SBR_ADDITIONS
      #ifdef AACDEC_PS_ADDITIONS
         #error "If PS has been enabled then SBR must also be"
      #endif
   #endif

   // Never can have have ELD additions but not SBR additions
   #ifndef AACDEC_SBR_ADDITIONS
      #ifdef AACDEC_ELD_ADDITIONS
         #error "If ELD has been enabled then SBR must also be"
      #endif
   #endif

   // Never can have have ELD additions but not PS additions
   #ifndef AACDEC_PS_ADDITIONS
      #ifdef AACDEC_ELD_ADDITIONS
         #error "If ELD has been enabled then PS must also be"
      #endif
   #endif



   // Pull in all formats by default, this allows a message from the VM
   // to configure which format we're working with.
   #define AACDEC_MP4_FILE_TYPE_SUPPORTED
   #define AACDEC_ADTS_FILE_TYPE_SUPPORTED
   #define AACDEC_LATM_FILE_TYPE_SUPPORTED



   // Plain AAC outputs 1024 samples per frame, AAC+SBR outputs 2048 samples per frame.
   // "SBR_HALF_SYNTHESIS" makes each call to frame_decode only perform half of the synthesis
   // filterbank resulting in 1024 samples being output per call, hence smaller output
   // buffers required
   #ifdef AACDEC_SBR_ADDITIONS
      #ifndef AACDEC_ELD_ADDITIONS
         #define AACDEC_SBR_HALF_SYNTHESIS
      #endif
   #endif




   // -- General AAC high level constants --
   #ifdef AACDEC_SBR_ADDITIONS
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
          .CONST $aacdec.MAX_AUDIO_FRAME_SIZE_IN_WORDS      1024;
      #else
          .CONST $aacdec.MAX_AUDIO_FRAME_SIZE_IN_WORDS      2048;
      #endif
   #else
      .CONST $aacdec.MAX_AUDIO_FRAME_SIZE_IN_WORDS          1024;
   #endif

    // **************************************************************************
    //                          MEMORY POOL constants
    // **************************************************************************

   // Note: These are lengths in words
    #ifdef AACDEC_SBR_ADDITIONS
       #ifdef AACDEC_PS_ADDITIONS
           .CONST $aacdec.TMP_MEM_POOL_LENGTH               3008;
       #else
           .CONST $aacdec.TMP_MEM_POOL_LENGTH               2844;
       #endif
    #else
       .CONST $aacdec.TMP_MEM_POOL_LENGTH                   2504;
    #endif


    #ifdef AACDEC_PS_ADDITIONS
        .CONST $aacdec.FRAME_MEM_POOL_LENGTH                2944;
    #else
        .CONST $aacdec.FRAME_MEM_POOL_LENGTH                1696;
    #endif





   .CONST $aacdec.MAX_AAC_FRAME_SIZE_IN_BYTES               1536; //  needed for 8KHz @80kbps -> 1536byte frame
   .CONST $aacdec.MIN_AAC_FRAME_SIZE_IN_BYTES               1536;
   .CONST $aacdec.MIN_MP4_FRAME_SIZE_IN_BYTES               1536;
   .CONST $aacdec.MIN_ADTS_FRAME_SIZE_IN_BYTES              24;   // Needed for VBR silence frames
   .CONST $aacdec.MIN_LATM_FRAME_SIZE_IN_BYTES              16;   // Needed for VBR silence frames
   .CONST $aacdec.OVERLAP_ADD_LENGTH                        576;  // for NON    ELD CASE
   .CONST $aacdec.BUF_PTR_LENGTH                            1024;
   .CONST $aacdec.CAN_IDLE                                  0;
   .CONST $aacdec.DONT_IDLE                                 1;


   // -- This enables garbage detection for latm streams, helps preventing long stalls on
   // garbage inputs
   #define AACDEC_ENABLE_LATM_GARBAGE_DETECTION

   #include "aac_consts.h"

#endif
