/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "stack.h"          // for PUSH_ALL_C macros
#include "codec_library.h"  // for ENCODER_DATA_OBJECT_FIELD
#include "wbs_struct_asm_defs.h"
#include "sco_struct_asm_defs.h"
#include "opmgr_for_ops_asm_defs.h"


#ifdef PATCH_LIBS
#include "patch_library.h"
#endif



.MODULE $M.wbs_enc_c_stubs;
    .CODESEGMENT PM;

// void wbsenc_init_encoder(OPERATOR_DATA *op_data)
$_wbsenc_init_encoder:
    pushm <r7, r8, r9, rLink>;

    r7 = r0;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($wbs_cap.WBS_C_STUBS_ASM.WBS_ENC_C_STUBS._WBSENC_INIT_ENCODER.PATCH_ID_0,r9)
#endif


    // set up the sbc library to point to the correct data
    r0 = M[r7 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
    r9 = M[r0 + $wbs_struct.WBS_ENC_OP_DATA_struct.CODEC_DATA_FIELD];

    call $wbsenc.init_encoder;

    popm <r7, r8, r9, rLink>;
    rts;


// void wbsenc_process_frame( OPERATOR_DATA *op_data)
$_wbsenc_process_frame:
    PUSH_ALL_C

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($wbs_cap.WBS_C_STUBS_ASM.WBS_ENC_C_STUBS._WBSENC_PROCESS_FRAME.PATCH_ID_0,r9)
#endif



    // set up the sbc library to point to the correct data
    r7 = M[r0 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
    r9 = M[r7 + $wbs_struct.WBS_ENC_OP_DATA_struct.CODEC_DATA_FIELD];

    call $wbsenc.process_frame;

    POP_ALL_C
    rts;


// void wbs_enc_reset_sbc_data(OPERATOR_DATA *op_data)
$_wbs_enc_reset_sbc_data:
    pushm <r4, r5, r9, rLink>;

#if defined(PATCH_LIBS)
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($wbs_cap.WBS_C_STUBS_ASM.WBS_ENC_C_STUBS._WBS_ENC_RESET_SBC_DATA.PATCH_ID_0)
#endif

    push I0;

#ifdef DEBUG_ON
//#ifdef SCO_DEBUG
/* first part of a tool helping preservation of registers that C compiler cares about
   should be copied (along with its couterpart, see below for exit) into the function
   you are investigating,
   after the entry pushm (pushing registers expected to be trashed)
   NOTE. be aware the counterpart involves some manual / specific configuration
 */
    PUSH_ALL_C
    push r0;
    r0 = 0xDEADBF;
    r4 = r0;    r5 = r0;    r6 = r0;    r7 = r0;    r8 = r0;    r9 = r0;
    I0 = r0;    I1 = r0;    I2 = r0;    I4 = r0;    I5 = r0;    I6 = r0;
    M0 = r0;    M1 = r0;    M2 = r0;
    // L regs expected on zero by asm libs, so this is not much we can do to detect trashing
    // as a general guidance, preserve the corresponding L when I reg was trashed
    r0 = Null;
    L0 = r0;    L1 = r0;    L4 = r0;    L5 = r0;
    pop r0;
#endif

   r4 = M[r0 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
   r9 = M[r4 + $wbs_struct.WBS_ENC_OP_DATA_struct.CODEC_DATA_FIELD];

   // setup the SBC working data
   // initialise other parts of the data structure
   r1 = 16;
   M[r9 + $sbc.mem.PUT_BITPOS_FIELD] = r1;
   M[r9 + $sbc.mem.WBS_SEND_FRAME_COUNTER_FIELD] = Null;

   // reset the sbc encoder - TODO: unless lib is re-worked again, it needs R5 normally as codec structure,
   // which inside has the encoder data object pointer at $codec.ENCODER_DATA_OBJECT_FIELD offset.
   // So trick it into finding the right start address by moving backwards by that amount...
   r5 = r4 + ($wbs_struct.WBS_ENC_OP_DATA_struct.CODEC_DATA_FIELD - $codec.ENCODER_DATA_OBJECT_FIELD);
   call $sbcenc.init_encoder;

#ifdef DEBUG_ON
//#ifdef SCO_DEBUG
/* counterpart of tool helping preservation of registers that C compiler cares about
    NOTE. involves some manual / specific configuration:
           comment the regs in exit pop/popm (disregard rLink)
           they are expected to be trashed
    should be called before the exit popm
 */
    push r0;
    r0 = 0xDEADBF;
//    Null = r4 - r0;     if NZ call $error;
//    Null = r5 - r0;     if NZ call $error;
    Null = r6 - r0;     if NZ call $error;
    Null = r7 - r0;     if NZ call $error;
    Null = r8 - r0;     if NZ call $error;
//    Null = r9 - r0;     if NZ call $error;

//    Null = I0 - r0;     if NZ call $error;
    Null = I1 - r0;     if NZ call $error;
    Null = I2 - r0;     if NZ call $error;
    Null = I4 - r0;     if NZ call $error;
    Null = I5 - r0;     if NZ call $error;
    Null = I6 - r0;     if NZ call $error;
    Null = M0 - r0;     if NZ call $error;
    Null = M1 - r0;     if NZ call $error;
    Null = M2 - r0;     if NZ call $error;

    // expect asm code to return trashed L regs on zero if they were trashed
    Null = L0 - 0;      if NZ call $error;
    Null = L1 - 0;      if NZ call $error;
    Null = L4 - 0;      if NZ call $error;
    Null = L5 - 0;      if NZ call $error;

    pop r0;

    POP_ALL_C
#endif

    pop I0;
    popm <r4, r5, r9, rLink>;
    rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_wbsdec_init_dec_param
//
// DESCRIPTION:
//    Function for initialising the wbs decoder parameters.
//
// INPUTS:
//    - r0 = pointer to operator data
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    r4, r9
// NOTES:
//
// *****************************************************************************
.MODULE $M.wbs_dec._wbsdec_init_dec_param;
   .CODESEGMENT PM;

$_wbsdec_init_dec_param:

   pushm <r4, r9, rLink>;

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($wbs_cap.WBS_C_STUBS_ASM.WBS_DEC._WBSDEC_INIT_DEC_PARAM._WBSDEC_INIT_DEC_PARAM.PATCH_ID_0,r4)
#endif


   r4 = M[r0 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
   r9 = M[r4 + $wbs_struct.WBS_DEC_OP_DATA_struct.CODEC_DATA_FIELD];
   call $sco_decoder.wbs.init_param;

   popm <r4, r9, rLink>;
   rts;

.ENDMODULE;


// *****************************************************************************
// FUNCTION:
//    $_sco_decoder_wbs_validate
//
// DESCRIPTION:
//    C callable version of the $sco_decoder.wbs.validate function.
//    Validates the packet. Must be called before decode.
//
// INPUTS:
//    r0 - wbs_dec.CODEC_DATA_FIELD
//    r1 - payload length in words
//    r2 - pointer which will be populated with the wbs packet length.
//    r3 - pointer which will be populated with the amount the input
//         buffer advanced (in words)
//
// OUTPUTS:
//    r0 - Output in samples, 0 to abort
//
// CPU USAGE:
//    D-MEMORY: xxx
//    P-MEMORY: xxx
//    CYCLES:   xxx
//
// NOTES:
// *****************************************************************************

.MODULE $M.sco_decoder.wbs._validate;

   .DATASEGMENT DM;
   .CODESEGMENT SBCDEC_WBS_VALIDATE_PM;

$_sco_decoder_wbs_validate:



   PUSH_ALL_C

   // save OP DATA
   r7 = r0;

   // payload length
   r5 = r1;// this should be already in words.

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($wbs_cap.WBS_C_STUBS_ASM.SCO_DECODER.WBS._VALIDATE._SCO_DECODER_WBS_VALIDATE.PATCH_ID_0,r1)
#endif

   // save the pointer to the wbs packet length
   push r2;

   // No read pointer advancing by default
   M[r3] = 0;
   // save the pointer, so we can update it if changed
   push r3;

   //r6 is trashed in $sco_decoder.wbs.validate


   // get extra op data
   r3 = M[r7 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
   // get the Input buffer
   r0 = M[r3 + $sco_struct.SCO_TERMINAL_BUFFERS_struct.OP_BUFFER_FIELD];
   // INPUTS:
   // r0 = pointer to cbuffer structure
   call $cbuffer.calc_amount_space_in_words;
   // OUTPUTS:
   // r0 = amount of space (for new data) in words or addresses
   // r2 = buffer size in words or addresses
   // save the amount of space available in output buffer
   r9 = r0;

   // get extra op data
   r3 = M[r7 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
   // get the Input buffer
   r0 = M[r3 + $sco_struct.SCO_TERMINAL_BUFFERS_struct.IP_BUFFER_FIELD];
   // save the input buffer for later use
   push r0;
   // INPUTS:
   // r0 = pointer to cbuffer structure
   call $cbuffer.get_read_address_and_size_and_start_address;
   // OUTPUTS:
   // r0 = read address
   // r1 = buffer size in addresses (locations)
   // r2 = buffer start address

   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   r0 = r9;

   // save initial read address
   push I0;

   // r7 is alreadey op data
   // r8
   r1 = M[r7 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
   r9 = M[r1 + $wbs_struct.WBS_DEC_OP_DATA_struct.CODEC_DATA_FIELD];


   // The packet will be reprocessed next call
   // r0 is output space
   // r5  still contains packet size. Validate will use it as dummy payload size
   // I0,L0 is pointer to input buffer.
   // set up the sbc library r9 to point to the decoder data object
   call $sco_decoder.wbs.validate;
   // r1 is output packet size in samples or return code
   // r5- wbs packet length
   // I0- point to payload if wbs packet found
   // L0: unchanged
   // AR- Validate returns in r1 either 120 or 240 (1 or 2 frames worth of data) if everything
   // AR- is alright. If it returns 0, 1 or 2, then there is no data
   // AR- 0 means there is not enough space in output buffer
   // AR- 1 means there is not enough data in the input buffer
   // AR- 2 means there is some data in input buffer, but unable to identify the WBS frame sync.
   r7 = r1; // save the return value
   // r5 containing the wbs packet length. This register should not be trashed even if the cbuffer is a
   // port.

   // restore initial read address
   pop r6;

   pop r0; // reload the input buffer
   r1 = I0;

   // see how much read pointer has advanced;
   r4 = r1 - r6;
   if NEG r4 = r4 + L0;
   r4 = r4 ASHIFT (-LOG2_ADDR_PER_WORD); // output is in words
   if NEG r4 = 0; // this shall not happen, just for safeguard

   // restore pointer to amount_advanced
   pop r6;

   r3 = M[r9 + $sbc.mem.WBS_BYTES_IN_FRAME_BUFFER_FIELD];
   if NZ jump return;
   // we dont have anything in the internal buffer of the wbs so it is safe to discard
   // the end of the previouse packet in case we found the sync.
   Null = r7 - $M.wbs.decoder.VALIDATE_NOSYNC;
   if Z jump return;

   // populate the amount advanced
   M[r6] = r4;

   call $cbuffer.set_read_address;

return:
   // populate the wbs packet legnth
   pop r2;
   M[r2] = r5;

   r0 = r7; // populate the return value
   POP_ALL_C
   rts;

.ENDMODULE;


// *****************************************************************************
// FUNCTION:
//    $sco_decoder.wbs.process:
//
// DESCRIPTION:
//    Decoding SCO WBS packets into DAC audio samples.
//
//    The SCO c-buffer contains WBS packet words to be decoded into DAC
//    audio samples. Refer to the function description of Frame_encode for
//    WBS packet word definition.  DAC audio could be mono-channel(left
//    only) or stereo-channels(both left and right).
//
// INPUTS:
// r0 wbs_dec.CODEC_DATA_FIELD
// r1 IP_BUFFER pointer
// r2 PLC data
// r3 pointer to the stream PACKET
// OUTPUTS:
//    r0    - Output packet status

// TRASHED REGISTERS:
//    Assumes everything
//
// CPU USAGE:
//    D-MEMORY: xxx
//    P-MEMORY: xxx
//    CYCLES:   xxx
//
// NOTES:
// *****************************************************************************

.MODULE $M.sco_decoder.wbs._process;

   .DATASEGMENT DM;
   .CODESEGMENT SBCDEC_WBS_PROC_PM;

$_sco_decoder_wbs_process:
// r0 op data
// r1 pointer to the stream PACKET
// r2 validate retval
// r3 wbs_packet_length

   PUSH_ALL_C

   // save OP DATA
   r7 = r0;
   // save pointer to the packet
   r9 = r1;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($wbs_cap.WBS_C_STUBS_ASM.SCO_DECODER.WBS._PROCESS._SCO_DECODER_WBS_PROCESS.PATCH_ID_0,r1)
#endif


   // save validate return
   r6 = r2;
   // save the wbs packet size
   r5 = r3;


   // get extra op data
   r3 = M[r7 + $opmgr_for_ops.OPERATOR_DATA_struct.EXTRA_OP_DATA_FIELD];
   // get the Input buffer
   r0 = M[r3 + $sco_struct.SCO_TERMINAL_BUFFERS_struct.IP_BUFFER_FIELD];

   // INPUTS:
   // r0 = pointer to cbuffer structure
   call $cbuffer.get_read_address_and_size_and_start_address;
   // OUTPUTS:
   // r0 = read address
   // r1 = buffer size in addresses (locations)
   // r2 = buffer start address

   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   // get packet timestamp
   r1 = M[r9 + $sco_struct.sco_metadata_struct.TIME_STAMP_FIELD];
#ifdef K32
   r8 = M[r9 + $sco_struct.sco_metadata_struct.STATUS_FIELD];
   r2 = r8 AND 0xff;
#else
   r2 = M[r9 + $sco_struct.sco_metadata_struct.STATUS_FIELD];
#endif // K32
   // r5 should be already the wbs packet size.

   //get the op buffer
   r8 = M[r3 + $sco_struct.SCO_TERMINAL_BUFFERS_struct.OP_BUFFER_FIELD];
   r9 = M[r3 + $wbs_struct.WBS_DEC_OP_DATA_struct.CODEC_DATA_FIELD];


// INPUTS:
//    r1    - Packet timestamp
//    r2    - Packet status
//    r5    - payload size in bytes
//    I0,L0,B0 - Input CBuffer
//    R9    - data object pointer
//
//    decoupled variant
//    r8    - Output CBuffer
//    r6    - Validate return code
   call $sco_decoder.wbs.process;
// OUTPUTS:
//    r5    - Output packet status
//    r7    - PLC Data Object
//    I0    - Input buffer updated (This is not alway true)

   r0 = r5;

   POP_ALL_C
   rts;
.ENDMODULE;



