// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCDEC_INIT_STATIC_DECODER_INCLUDED
#define SBCDEC_INIT_STATIC_DECODER_INCLUDED

#include "sbc.h"

#ifndef KYMERA
#include "kalimba_standard_messages.h"
#endif


//
// MODULE:
//    $sbcdec.init_static_decoder
//
// DESCRIPTION:
//    Initialise variables for sbc decoding.This corresponds to init_decoder,
//    and it is the function to call when not using external dynamic tables.
//    It relies on init_static_common, which initialises the variables that
//    are common between encoder and decoder.
//    It also takes care of message handler that is only used in VM builds.
//
// INPUTS:
//    - r5 = pointer to decoder structure
//
// OUTPUTS:
//    - Decoder data object pointer is set in the decoder structure
//
// TRASHED REGISTERS:
//    r0-r3, r9, r10, DoLoop, I0
//
// *****************************************************************************
.MODULE $M.sbcdec.init_static_decoder;
   .CODESEGMENT SBCDEC_INIT_STATIC_DECODER_PM;
   .DATASEGMENT DM;

   $sbcdec.init_static_decoder:

   // push rLink onto stack
   push rLink;


#ifndef KYMERA
    // message structures
   .VAR/DM       $sbcdec.convert_mono_message_struc[$message.STRUC_SIZE];
#endif

   // lifted from wbs_sco_decode.asm
    .CONST $M.wbs.decoder.WBS_ENCODED_FRAME_SIZE_B       60;
    .VAR/DMCIRC $M.wbs.decoder.frame_buffer[$M.wbs.decoder.WBS_ENCODED_FRAME_SIZE_B/2];

   // getbit variables
   .VAR/DM          $sbcdec.get_bitpos = 16;

    // filterbank synthesis variables
   .CONST       SYNTHESIS_BUFFER_LENGTH   160;

   .VAR/DM2CIRC $sbcdec.synthesis_vch1[SYNTHESIS_BUFFER_LENGTH];
#ifndef SBC_WBS_ONLY
   .VAR/DM2CIRC $sbcdec.synthesis_vch2[SYNTHESIS_BUFFER_LENGTH];
#endif
   .BLOCK/DM synthesis_vchptrs;
   .VAR         $sbcdec.synthesis_vch1ptr;
#ifndef SBC_WBS_ONLY
   .VAR         $sbcdec.synthesis_vch2ptr;
#endif
   .ENDBLOCK;


#ifndef SBC_WBS_ONLY
   // synthesis subband filterbank matrix for (4x8) M = 4
   .VAR/DM2      $sbcdec.synthesis_coefs_m4_ram[32];

#endif
   // synthesis subband filterbank matrix for (8x16) M = 8

   .VAR/DM2      $sbcdec.synthesis_coefs_m8_ram[128];

   // Unsigned lookup of: (2^bits) / (2^bits - 1) * 2^23
   // The table is for bits values of 2-16.

   .VAR/DM2      $sbcdec.levelrecip_coefs_ram[15];

   // Lookup table to slightly speedup generating a bit mask

   .VAR/DM2      $sbcdec.bitmask_lookup_ram[17];


   // store memory pointer, in case static apps were written such that they don't
   // refer to library's internal data object (neater that way)
   r9 = &$sbc.sbc_common_data_array;
   M[r5 + $codec.DECODER_DATA_OBJECT_FIELD] = r9;

   // call the common initilisation for static vars - some small-ish overhead, as encoder will also call this again when static init is called
   call $sbc.init_static_common;

   // initialise the decoder fields in overall data array
   r0 = &$sbcdec.synthesis_coefs_m8_ram;
   M[r9 + $sbc.mem.SYNTHESIS_COEFS_M8_FIELD] = r0;
#ifndef SBC_WBS_ONLY
   r0 = &$sbcdec.synthesis_coefs_m4_ram;
   M[r9 + $sbc.mem.SYNTHESIS_COEFS_M4_FIELD] = r0;
#endif
   r0 = &$sbcdec.levelrecip_coefs_ram;
   M[r9 + $sbc.mem.LEVELRECIP_COEFS_FIELD] = r0;
   r0 = &$sbcdec.bitmask_lookup_ram;
   M[r9 + $sbc.mem.BITMASK_LOOKUP_FIELD] = r0;
   r0 = &$sbcdec.synthesis_vch1;
   M[r9 + $sbc.mem.SYNTHESIS_VCH1_FIELD] = r0;
#ifndef SBC_WBS_ONLY
   r0 = &$sbcdec.synthesis_vch2;
   M[r9 + $sbc.mem.SYNTHESIS_VCH2_FIELD] = r0;
#endif
   // default: FALSE - do not return on corrupt frame
   M[r9 + $sbc.mem.RETURN_ON_CORRUPT_FRAME_FIELD] = 0;
   r0 = 16;
   M[r9 + $sbc.mem.GET_BITPOS_FIELD] = r0;
   r0 = &$M.wbs.decoder.frame_buffer;
   M[r9 + $sbc.mem.WBS_FRAME_BUFFER_PTR_FIELD] = r0;



   // -- reset decoder variables --
   call $sbcdec.reset_decoder;
   call $sbcdec.init_tables;


   // set up message handler for $MESSAGE_SBCDEC_CONVERT_TO_MONO message
#ifndef SBC_WBS_ONLY
   // TODO this is a temporary measure for building for sirfstarv. there is no distinction between
   // BlueCore and non-BlueCore architectures in most libs.
#ifndef SIRFSTARV

   // in external variant it is assumed that this message handler is not used
   r1 = &$sbcdec.convert_mono_message_struc;
   r2 = $MESSAGE_SBCDEC_CONVERT_TO_MONO;
   r3 = &$sbcdec.convert_mono_handler;
   call $message.register_handler;
#endif
#endif
   // pop rLink from stack
   jump $pop_rLink_and_rts;

#ifndef KYMERA
#ifndef SBC_WBS_ONLY
   // **************************************************************************
   // DESCRIPTION:
   // inline message handler for $MESSAGE_SBCDEC_CONVERT_TO_MONO message
   // in external variant it is assumed that this message handler is not used
   $sbcdec.convert_mono_handler:
      // first message argument (r1) is 0 (leave as stereo) or 1 (convert to mono)
      M[r9 + $sbc.mem.CONVERT_TO_MONO_FIELD] = r1;
      rts;
#endif /* SBC_WBS_ONLY */
#endif /* KYMERA */

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $sbcdec.deinit_static_decoder
//
// DESCRIPTION:
//    remove (unregister) message SBC decoder message handlers
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
.MODULE $M.sbcdec.deinit_static_decoder;
   .CODESEGMENT SBCDEC_DEINIT_STATIC_DECODER_PM;
   .DATASEGMENT DM;

   $sbcdec.deinit_static_decoder:

   // push rLink onto stack
   push rLink;

#ifndef KYMERA
#ifndef SBC_WBS_ONLY
   // TODO this is a temporary measure for building for sirfstarv. there is no distinction between
   // BlueCore and non-BlueCore architectures in most libs.
#ifndef SIRFSTARV

   // remove handler
   r3 = $MESSAGE_SBCDEC_CONVERT_TO_MONO;
   call $message.unregister_handler;
#endif /* SBC_WBS_ONLY */
#endif /* KYMERA */
#endif
// pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
