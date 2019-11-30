// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCENC_INIT_STATIC_COMMON_INCLUDED
#define SBCENC_INIT_STATIC_COMMON_INCLUDED

#include "sbc.h"

#ifndef KYMERA
#include "kalimba_standard_messages.h"
#endif

// *****************************************************************************
// MODULE:
//    $sbc.init_static_common
//
// DESCRIPTION:
//    Initialise variables for SBC encoder and decoder common parts.
//    It also defines a common data array that matches what would be used in
//    the dynamic build. The fields related to common variables are initialised
//    here, while the fields for encoder and decoder variables are initialised
//    at runtime by the init_static_encoder/decoder functions, respectively.
//
//
// INPUTS:
//    - r5 = pointer to encoder/decoder data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r3, r10, DoLoop, I0
//
// *****************************************************************************
.MODULE $M.sbc.init_static_common;
   .CODESEGMENT SBC_INIT_COMMON_PM;
   .DATASEGMENT DM;


   $sbc.init_static_common:

   // scalefactors variables
   .VAR/DM1      $sbc.scale_factor[16];
#ifndef SBC_WBS_ONLY
   .VAR/DM1      $sbc.scale_factor_js[16];
#endif

   // bit calculation variables
   .VAR/DM1      $sbc.bitneed[16];
   .VAR/DM1      $sbc.bits[16];

   // sample variables
   .VAR/DM1      $sbc.audio_sample[256];


#ifndef SBC_WBS_ONLY
   // subband prototype window for M = 4

   .VAR/DM1      $sbc.win_coefs_m4_ram[40];

#endif
   // subband prototype window for M = 8

   .VAR/DM1      $sbc.win_coefs_m8_ram[80];

// add block to keep the two tables consecutively (when not WBS_ONLY) - so 48 word long contiguous
   .BLOCK/DM2 $sbc.loudness_offset_ram;
#ifndef SBC_WBS_ONLY
   .VAR     $sbc.loudness_offset_m4_ram[16];
#endif
   .VAR     $sbc.loudness_offset_m8_ram[32];
   .ENDBLOCK;

   // The overall data array used in static build - matches dynamic table in case of dynamic library
   // Common part is initialised, the encoder and decoder parts are filled at runtime by encoder and decoder static init functions.
   .VAR $sbc.sbc_common_data_array[$sbc.mem.STRUC_SIZE] =
    // *********************** ENCODER ****************************
    0,                                              /* $sbc.mem.AUDIO_SAMPLE_JS_FIELD */
    0,                                              /* $sbc.mem.ANALYSIS_COEFS_M8_FIELD */
    0,                                              /* $sbc.mem.ANALYSIS_COEFS_M4_FIELD */
    0,                                              /* $sbc.mem.ANALYSIS_XCH1_FIELD */
    0,                                              /* $sbc.mem.ANALYSIS_XCH2_FIELD  */
    0,                                              /* $sbc.mem.ANALYSIS_Y_FIELD  */
    0,                                              /* $sbc.mem.LEVEL_COEFS_FIELD  */
    0,                                              /* $sbc.mem.PRE_POST_PROC_STRUC_FIELD */
    0,                                              /* $sbc.mem.ENC_SETTING_NROF_SUBBANDS_FIELD */
    0,                                              /* $sbc.mem.ENC_SETTING_NROF_BLOCKS_FIELD  */
    0,                                              /* $sbc.mem.ENC_SETTING_SAMPLING_FREQ_FIELD  */
    0,                                              /* $sbc.mem.ENC_SETTING_CHANNEL_MODE_FIELD  */
    0,                                              /* $sbc.mem.ENC_SETTING_ALLOCATION_METHOD_FIELD  */
    0,                                              /* $sbc.mem.ENC_SETTING_BITPOOL_FIELD  */
    0,                                              /* $sbc.mem.ENC_SETTING_FORCE_WORD_ALIGN_FIELD  */
    0,                                              /* $sbc.mem.PUT_NEXTWORD_FIELD  */
    0,                                              /* $sbc.mem.PUT_BITPOS_FIELD  */
    0,                                              /* $sbc.mem.ANALYSIS_XCH1PTR_FIELD   */
    0,                                              /* $sbc.mem.ANALYSIS_XCH2PTR_FIELD  */
    0,                                              /* $sbc.mem.WBS_SEND_FRAME_COUNTER_FIELD  */

    // ********************** COMMON **********************************

    &$sbc.audio_sample,                             /* $sbc.mem.AUDIO_SAMPLE_FIELD  */
    &$sbc.win_coefs_m8_ram,                         /* $sbc.mem.WIN_COEFS_M8_FIELD  */
#ifdef SBC_WBS_ONLY
    0,                                              /* This is not used in SBC_WBS_ONLY */
#else
    &$sbc.win_coefs_m4_ram,                         /* $sbc.mem.WIN_COEFS_M4_FIELD  */
#endif
    &$sbc.loudness_offset_ram,                      /* $sbc.mem.LOUDNESS_OFFSET_FIELD - put table for M=4 in a block with sbc.loudness_offset_m8 to ensure contiguous area */
    &$sbc.scale_factor,                             /* $sbc.mem.SCALE_FACTOR_FIELD   */
#ifdef SBC_WBS_ONLY
    0,
#else
    &$sbc.scale_factor_js,                          /* $sbc.mem.SCALE_FACTOR_JS_FIELD    */
#endif
    &$sbc.bitneed,                                  /* $sbc.mem.BITNEED_FIELD  */
    &$sbc.bits,                                     /* $sbc.mem.BITS_FIELD */
    0,                                              /* $sbc.mem.SAMPLING_FREQ_FIELD  */
    0,                                              /* $sbc.mem.NROF_BLOCKS_FIELD    */
    0,                                              /* $sbc.mem.CHANNEL_MODE_FIELD   */
    0,                                              /* $sbc.mem.NROF_CHANNELS_FIELD   */
    0,                                              /* $sbc.mem.ALLOCATION_METHOD_FIELD  */
    0,                                              /* $sbc.mem.NROF_SUBBANDS_FIELD   */
    0,                                              /* $sbc.mem.BITPOOL_FIELD   */
    0,                                              /* $sbc.mem.FRAMECRC_FIELD     */
    0,                                              /* $sbc.mem.CRC_CHECKSUM_FIELD    */
    0,                                              /* $sbc.mem.FORCE_WORD_ALIGN_FIELD   */
    0,                                              /* $sbc.mem.JOIN_FIELD  */
    0,                                              /* $sbc.mem.JOIN_1_FIELD  */
    0,                                              /* $sbc.mem.JOIN_2_FIELD  */
    0,                                              /* $sbc.mem.JOIN_3_FIELD   */
    0,                                              /* $sbc.mem.JOIN_4_FIELD  */
    0,                                              /* $sbc.mem.JOIN_5_FIELD    */
    0,                                              /* $sbc.mem.JOIN_6_FIELD  */
    0,                                              /* $sbc.mem.JOIN_7_FIELD      */

    // *********************** DECODER ****************************
    0,                                              /* $sbc.mem.SYNTHESIS_COEFS_M8_FIELD  */
    0,                                              /* $sbc.mem.SYNTHESIS_COEFS_M4_FIELD   */
    0,                                              /* $sbc.mem.LEVELRECIP_COEFS_FIELD  */
    0,                                              /* $sbc.mem.BITMASK_LOOKUP_FIELD  */
    0,                                              /* $sbc.mem.SYNTHESIS_VCH1_FIELD   */
    0,                                              /* $sbc.mem.SYNTHESIS_VCH2_FIELD   */
    0,                                              /* $sbc.mem.CONVERT_TO_MONO_FIELD  */
    0,                                              /* $sbc.mem.FRAME_UNDERFLOW_FIELD   */
    0,                                              /* $sbc.mem.FRAME_CORRUPT_FIELD  */
    0,                                              /* $sbc.mem.NUM_BYTES_AVAILABLE_FIELD  */
    0,                                              /* $sbc.mem.WBS_SBC_FRAME_OK_FIELD  */
    0,                                              /* $sbc.mem.CUR_FRAME_LENGTH_FIELD   */
    0,                                              /* $sbc.mem.RETURN_ON_CORRUPT_FRAME_FIELD    */
    0,                                              /* $sbc.mem.GET_BITPOS_FIELD  */
    0,                                              /* $sbc.mem.SYNTHESIS_VCH1PTR_FIELD  */
    0,                                              /* $sbc.mem.SYNTHESIS_VCH2PTR_FIELD  */
    0,                                              /* $sbc.mem.WBS_FRAME_BUFFER_PTR_FIELD  */
    0,                                              /* $sbc.mem.WBS_SYNC_FIELD   */
    0,                                              /* $sbc.mem.WBS_BYTES_IN_FRAME_BUFFER_FIELD  */
    0,                                              /* $sbc.mem.WBS_STICKY_BFI_FIELD   */
    0,                                              /* $sbc.mem.WBS_PAYLOAD_ALIGN_FIELD  */
    0,                                              /* $sbc.mem.TIMESTAMP_T1_FIELD  */
    0;                                              /* $sbc.mem.TIMESTAMP_T2_FIELD */

   rts;

.ENDMODULE;



#endif

