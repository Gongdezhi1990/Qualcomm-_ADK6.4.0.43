// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCENC_INIT_STATIC_ENCODER_INCLUDED
#define SBCENC_INIT_STATIC_ENCODER_INCLUDED

#ifndef KYMERA
#include "kalimba_standard_messages.h"
#endif

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.init_static_encoder
//
// DESCRIPTION:
//    Initialise variables for SBC encoding. This corresponds to init_encoder,
//    and it is the function to call when not using external dynamic tables.
//    It relies on init_static_common, which initialises the variables that
//    are common between encoder and decoder.
//    It also takes care of message handlers that are only used in VM builds.
//
// INPUTS:
//    - r5 = pointer to encoder structure
//
// OUTPUTS:
//    - Encoder data object pointer is set in the encoder structure
//
// TRASHED REGISTERS:
//    r0-r3, r9, r10, DoLoop, I0
//
// *****************************************************************************
.MODULE $M.sbcenc.init_static_encoder;
   .CODESEGMENT SBCENC_INIT_STATIC_ENCODER_PM;
   .DATASEGMENT DM;

   $sbcenc.init_static_encoder:



   // push rLink onto stack
   push rLink;



#ifndef SBC_WBS_ONLY
   //pointer to pre/post processing structure
   .VAR/DM $sbcenc.pre_post_proc_struc[$codec.pre_post_proc.STRUC_SIZE] = 0 ...;

#ifndef KYMERA
   // message structures
   .VAR/DM          $sbcenc.set_bitpool_message_struc[$message.STRUC_SIZE];
   .VAR/DM          $sbcenc.set_encoder_params_message_struc[$message.STRUC_SIZE];
#endif /* KYMERA */

   // sample variables
   .VAR/DM1      $sbcenc.audio_sample_js[256];

#endif

   // filterbank analysis variables
   .CONST        ANALYSIS_BUFFER_LENGTH   80;

   .VAR/DM1      $sbcenc.analysis_y[16];
   .VAR/DM2CIRC  $sbcenc.analysis_xch1[ANALYSIS_BUFFER_LENGTH];
#ifndef SBC_WBS_ONLY
   .VAR/DM2CIRC  $sbcenc.analysis_xch2[ANALYSIS_BUFFER_LENGTH];
#endif
   .BLOCK/DM analysis_xchptrs;
   .VAR          $sbcenc.analysis_xch1ptr;
#ifndef SBC_WBS_ONLY
   .VAR          $sbcenc.analysis_xch2ptr;
#endif
   .ENDBLOCK;


#ifndef SBC_WBS_ONLY
   // analysis subband filterbank matrix (4x8) for M = 4
   .VAR/DM2      $sbcenc.analysis_coefs_m4_ram[32];
#endif
   // analysis subband filterbank matrix for (8x16) M = 8
   .VAR/DM2      $sbcenc.analysis_coefs_m8_ram[128];

   // Lookup of: (2^bits - 1) * 2^7
   // The table is for bits values of 1-16.

   .VAR/DM2      $sbcenc.level_coefs_ram[16];

   // store memory pointer, in case static apps were written such that they don't
   // refer to library's internal data object (neater that way)
   r9 = &$sbc.sbc_common_data_array;
   M[r5 + $codec.ENCODER_DATA_OBJECT_FIELD] = r9;

   // init the common vars - some small overhead, as init static decoder also calls this
   call $sbc.init_static_common;

   // initialise the encoder fields in overall data array
#ifndef SBC_WBS_ONLY
   r0 = &$sbcenc.audio_sample_js;
   M[r9 + $sbc.mem.AUDIO_SAMPLE_JS_FIELD] = r0;
#endif

   r0 = &$sbcenc.analysis_coefs_m8_ram;
   M[r9 + $sbc.mem.ANALYSIS_COEFS_M8_FIELD] = r0;

#ifndef SBC_WBS_ONLY
   r0 = &$sbcenc.analysis_coefs_m4_ram;
   M[r9 + $sbc.mem.ANALYSIS_COEFS_M4_FIELD] = r0;
#endif

   r0 = &$sbcenc.analysis_xch1;
   M[r9 + $sbc.mem.ANALYSIS_XCH1_FIELD] = r0;

#ifndef SBC_WBS_ONLY
   r0 = &$sbcenc.analysis_xch2;
   M[r9 + $sbc.mem.ANALYSIS_XCH2_FIELD] = r0;
#endif

   r0 = &$sbcenc.analysis_y;
   M[r9 + $sbc.mem.ANALYSIS_Y_FIELD] = r0;

   r0 = &$sbcenc.level_coefs_ram;
   M[r9 + $sbc.mem.LEVEL_COEFS_FIELD] = r0;


#ifndef SBC_WBS_ONLY
   // Some app code was found taking the function pointer from inside the struct and if NZ, call it -
   // hence would ensure struct is zero-initialised as per above .VAR statement, to make sure above mentioned call doesn't happen!
   r0 = &$sbcenc.pre_post_proc_struc;
   M[r9 + $sbc.mem.PRE_POST_PROC_STRUC_FIELD] = r0;

   r0 = 8;
   M[r9 + $sbc.mem.ENC_SETTING_NROF_SUBBANDS_FIELD] = r0;
   r0 = 16;
   M[r9 + $sbc.mem.ENC_SETTING_NROF_BLOCKS_FIELD] = r0;
   r0 = $sbc.FS_44100HZ;
   M[r9 + $sbc.mem.ENC_SETTING_SAMPLING_FREQ_FIELD] = r0;
   r0 = $sbc.JOINT_STEREO;
   M[r9 + $sbc.mem.ENC_SETTING_CHANNEL_MODE_FIELD] = r0;
   r0 = $sbc.LOUDNESS;
   M[r9 + $sbc.mem.ENC_SETTING_ALLOCATION_METHOD_FIELD] = r0;
   r0 = 55;
   M[r9 + $sbc.mem.ENC_SETTING_BITPOOL_FIELD] = r0;
#endif
   r0 = 0;
   M[r9 + $sbc.mem.PUT_NEXTWORD_FIELD] = r0;
   r0 = 16;
   M[r9 + $sbc.mem.PUT_BITPOS_FIELD] = r0;



   // -- reset encoder variables --
   call $sbcenc.reset_encoder;
   call $sbcenc.init_tables;

#ifndef KYMERA
   // in external variant it is assumed that this message handler is not used
#ifndef SBC_WBS_ONLY
   // TODO this is a temporary measure for building for sirfstarv. there is no distinction between
   // BlueCore and non-BlueCore architectures in most libs.
#ifndef SIRFSTARV

   // set up message handler for $MESSAGE_SBCENC_SET_BITPOOL message
   r1 = &$sbcenc.set_bitpool_message_struc;
   r2 = $MESSAGE_SBCENC_SET_BITPOOL;
   r3 = &$sbcenc.set_bitpool_handler;
   call $message.register_handler;

   // set up message handler for $MESSAGE_SBCENC_SET_PARAMS message
   r1 = &$sbcenc.set_encoder_params_message_struc;
   r2 = $MESSAGE_SBCENC_SET_PARAMS;
   r3 = &$sbcenc.set_encoder_params_handler;
   call $message.register_handler;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


   // DESCRIPTION:
   //    inline message handler for $MESSAGE_SBCENC_SET_BITPOOL message
   //    in external variant it is assumed that this message handler is not used

   $sbcenc.set_bitpool_handler:
      // first message argument (r1) is the requested bitpool value
      M[r9 + $sbc.mem.ENC_SETTING_BITPOOL_FIELD] = r1;
   rts;

   // DESCRIPTION:
   //    inline message handler for $MESSAGE_SBCENC_SET_PARAMS message
   //    in external variant it is assumed that this message handler is not used

   $sbcenc.set_encoder_params_handler:
      // first message argument (r1) is the requested encoder parameters
      //   bit 8:    force word (16 bit) aligned packets
      //   bit 6-7:  sampling frequency
      //   bit 4-5:  blocks
      //   bit 2-3:  channel_mode
      //   bit 1:    allocation method
      //   bit 0:    subbands
      r0 = r1 LSHIFT -8;
      r0 = r0 AND 1;
      M[r9 + $sbc.mem.ENC_SETTING_FORCE_WORD_ALIGN_FIELD] = r0;

      r0 = r1 LSHIFT -6;
      r0 = r0 AND 3;
      M[r9 + $sbc.mem.ENC_SETTING_SAMPLING_FREQ_FIELD] = r0;

      r0 = r1 LSHIFT -4;
      r0 = r0 AND 3;
      r0 = r0 + 1;
      r0 = r0 LSHIFT 2;
      M[r9 + $sbc.mem.ENC_SETTING_NROF_BLOCKS_FIELD] = r0;

      r0 = r1 LSHIFT -2;
      r0 = r0 AND 3;
      M[r9 + $sbc.mem.ENC_SETTING_CHANNEL_MODE_FIELD] = r0;

      r0 = r1 LSHIFT -1;
      r0 = r0 AND 1;
      M[r9 + $sbc.mem.ENC_SETTING_ALLOCATION_METHOD_FIELD] = r0;

      r0 = r1 AND 1;
      r0 = r0 + 1;
      r0 = r0 LSHIFT 2;
      M[r9 + $sbc.mem.ENC_SETTING_NROF_SUBBANDS_FIELD]     = r0;
   rts;

#endif //#ifndef SIRFSTARV

#endif /* SBC_WBS_ONLY */
#endif /* KYMERA */
    // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $sbcenc.deinit_static_encoder
//
// DESCRIPTION:
//    remove (unregister) message SBC encoder message handlers
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.sbcenc.deinit_static_encoder;
   .CODESEGMENT SBCENC_DEINIT_STATIC_ENCODER_PM;
   .DATASEGMENT DM;

   $sbcenc.deinit_static_encoder:

   // push rLink onto stack
   push rLink;


#ifndef KYMERA
#ifndef SBC_WBS_ONLY
   // TODO this is a temporary measure for building for sirfstarv. there is no distinction between
   // BlueCore and non-BlueCore architectures in most libs.
#ifndef SIRFSTARV

   // remove handlers
   r3 = $MESSAGE_SBCENC_SET_BITPOOL;
   call $message.unregister_handler;

   r3 = $MESSAGE_SBCENC_SET_PARAMS;
   call $message.unregister_handler;

#endif // #ifndef SIRFSTARV

#endif /* SBC_WBS_ONLY */
#endif /* KYMERA */
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
#endif

