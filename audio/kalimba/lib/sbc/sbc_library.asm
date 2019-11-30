// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#if !defined(KALASM3)
// These will map the PM_DYNAMIC_x and _SCRATCH segments to "normal" groups
// link scratch memory segments to DMxGroups (for lib build only)
//          Name                 CIRCULAR?   Link Order  Group list
.DEFSEGMENT DM_SCRATCH                       5           DM1Group  DM2Group;
.DEFSEGMENT DM1_SCRATCH                      3           DM1Group;
.DEFSEGMENT DM2_SCRATCH                      3           DM2Group;
.DEFSEGMENT DMCIRC_SCRATCH       CIRCULAR    4           DM1Group  DM2Group;
.DEFSEGMENT DM1CIRC_SCRATCH      CIRCULAR    2           DM1Group;
.DEFSEGMENT DM2CIRC_SCRATCH      CIRCULAR    2           DM2Group;

// link dynamic program memory segments to CODEGroup (for lib build only)
//          Name                             Link Order  Group list
.DEFSEGMENT PM_DYNAMIC_1                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_2                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_3                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_4                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_5                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_6                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_7                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_8                     4           CODEGroup;
.DEFSEGMENT PM_DYNAMIC_9                     4           CODEGroup;


//Module Mappings
#define SBC_CRC_CALC_PM                      PM_DYNAMIC_1
#define SBCDEC_GETBITS_PM                    PM_DYNAMIC_1
#define SBCENC_PUTBITS_PM                    PM_DYNAMIC_1
#define SBCENC_QUANTIZE_SAMPLES_PM           PM_DYNAMIC_1
#define SBCDEC_READ_AUDIO_SAMPLES_PM         PM_DYNAMIC_1
#define SBCDEC_READ_SCALE_FACTORS_PM         PM_DYNAMIC_1
#define SBCENC_WRITE_SCALE_FACTORS_PM        PM_DYNAMIC_1
#define SBCENC_ANALYSIS_SUBBAND_FILTER_PM    PM_DYNAMIC_2
#define SBCENC_WRITE_AUDIO_SAMPLES_PM        PM_DYNAMIC_2
#define SBC_CALC_SCALE_FACTORS_PM            PM_DYNAMIC_3
#define SBCDEC_SAMPLE_RECONSTRUCTION_PM      PM_DYNAMIC_3
#define SBCDEC_SYNTHESIS_SUBBAND_FILTER_PM   PM_DYNAMIC_5
#define SBCENC_WRITE_FRAME_HEADER_PM         PM_DYNAMIC_5
#define SBCENC_WRITE_PADDING_BITS_PM         PM_DYNAMIC_6
#define SBC_CALC_BIT_ALLOCATION_PM           PM_DYNAMIC_8
#define SBCENC_RESET_ENCODER_PM              PM_DYNAMIC_9
#define SBCDEC_SILENCE_DECODER_PM            PM_DYNAMIC_9
#ifndef SBC_WBS_ONLY
#define SBCDEC_FIND_SYNC_PM                  PM_DYNAMIC_1 // not profiled
#define SBCENC_JOINT_STEREO_ENCODE_PM        PM_DYNAMIC_1
#define SBCDEC_JOINT_STEREO_DECODE_PM        PM_DYNAMIC_2
#define SBCENC_ENCODER_SET_PARAMETERS_PM     PM_DYNAMIC_3
#define SBC_CALC_FRAME_LENGTH_PM             PM_DYNAMIC_4
#define SBCDEC_GET_FRAME_INFO_PM             PM_DYNAMIC_5
#define SBCDEC_READ_FRAME_HEADER_PM          PM_DYNAMIC_6
#define SBCDEC_READ_FRAME_HEADER_WBS_PM      PM_DYNAMIC_6
#define SBCDEC_FRAME_DECODE_WBS_PM           PM_DYNAMIC_7
#if defined(L2CAP_FRAME)
#define SBCENC_GET_ENCODED_FRAME_INFO_PM     PM_DYNAMIC_8
#endif
#define SBCDEC_SAVE_STATE_PM                 PM_DYNAMIC_9
#define SBCDEC_RESTORE_STATE_PM              PM_DYNAMIC_9
#define SBCENC_SAVE_STATE_PM                 PM_DYNAMIC_9
#define SBCENC_RESTORE_STATE_PM              PM_DYNAMIC_9
#define SBCDEC_PM                            PM_DYNAMIC_9
#define SBC_PM                               PM_DYNAMIC_9
#define SBCENC_PM                            PM_DYNAMIC_9
#define SBCDEC_INIT_DECODER_PM               PM_DYNAMIC_9
#define SBCENC_INIT_ENCODER_PM               PM_DYNAMIC_9
#define SBCENC_INIT_STATIC_ENCODER_PM        PM_DYNAMIC_9
#define SBCDEC_INIT_STATIC_DECODER_PM        PM_DYNAMIC_9
#define SBC_INIT_COMMON_PM                   PM_DYNAMIC_9
#define SBCDEC_READ_PADDING_BITS_PM          PM_DYNAMIC_9
#define SBCDEC_RESET_DECODER_PM              PM_DYNAMIC_9
#define SBCDEC_DEINIT_DECODER_PM             PM_DYNAMIC_9
#define SBCDEC_DEINIT_STATIC_DECODER_PM      PM_DYNAMIC_9
#define SBCENC_DEINIT_ENCODER_PM             PM_DYNAMIC_9
#define SBCENC_DEINIT_STATIC_ENCODER_PM      PM_DYNAMIC_9
#ifdef SBC_LOWRAM
   #define DONT_USE_EXTRA_JS_BUFFER
#endif
#endif


#define SBC_INIT_TABLES_PM                   PM_DYNAMIC_9
#define SBCDEC_INIT_TABLES_PM                PM_DYNAMIC_9
#define SBCENC_INIT_TABLES_PM                PM_DYNAMIC_9

#define SBCENC_FRAME_ENCODE_PM               PM_DYNAMIC_7
#define SBCDEC_FRAME_DECODE_PM               PM_DYNAMIC_7

#include "sbcdec_library.asm"
#include "sbcenc_library.asm"
#include "sbc_api.asm"

#include "initialise_tables.asm"
