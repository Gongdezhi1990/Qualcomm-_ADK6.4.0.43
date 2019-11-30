// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef AAC_CONSTS_HEADER_INCLUDED
#define AAC_CONSTS_HEADER_INCLUDED

   // Debug control
   #ifdef DEBUG_ON
      #define DEBUG_AACDEC
      #define ENABLE_PROFILER_MACROS
      #define AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
      #define AACDEC_CALL_ERROR_ON_MALLOC_FAIL
   #endif


   #include "sbr_consts.h"
   #include "eld_consts.h"
   #include "ps_consts.h"


   .CONST $aacdec.MIN_MP4_HEADER_SIZE_IN_BYTES             16;

   .CONST $aacdec.MPEG2_AAC_STREAM                         0;
   .CONST $aacdec.MPEG4_AAC_STREAM                         1;

   // supported Audio Object Types (AOTs)
   .CONST $aacdec.AAC_LC                                   2;
   .CONST $aacdec.AAC_LTP                                  4;
   .CONST $aacdec.SBR                                      5;
   .CONST $aacdec.ER_AAC_ELD                               39;

   // We only support limited sampling frequencies.  This table returns an
   // offset to be used when accessing other frequency dependent tables.
   // A negative number indicates an unsupported frequency
   .CONST $aacdec.NUM_SUPPORTED_FREQS                      9;
   .CONST $aacdec.NUM_SUPPORTED_FREQS_ELD                  5;


   // **************************************************************************
   //                            SCALEFACTOR constants
   // **************************************************************************

   // scalefactor offset
   .CONST $aacdec.SF_OFFSET                                100;
   // requantize extra shift required to always be within 24bit fixed point range
   .CONST $aacdec.REQUANTIZE_EXTRA_SHIFT                   (25*4);

   // **************************************************************************
   //                               TNS constants
   // **************************************************************************

   // TNS structure
   .CONST $aacdec.tns.N_FILT_FIELD                         0*ADDR_PER_WORD;    // size 8                           stored in an indexed way
   .CONST $aacdec.tns.COEF_RES_FIELD                       8*ADDR_PER_WORD;    // size 8   (1x3 long, 8x1 short)   stored in an indexed way x[2] is always for the 3rd filter
   .CONST $aacdec.tns.LENGTH_FIELD                         16*ADDR_PER_WORD;   // size 8   (1x3 long, 8x1 short)   index incremented only if filter exists
   .CONST $aacdec.tns.ORDER_FIELD                          24*ADDR_PER_WORD;   // size 8   (1x3 long, 8x1 short)   index incremented only if filter exists
   .CONST $aacdec.tns.DIRECTION_FIELD                      32*ADDR_PER_WORD;   // size 8   (1x3 long, 8x1 short)   index incremented only if filter exists
   .CONST $aacdec.tns.COEF_COMPRESS_FIELD                  40*ADDR_PER_WORD;   // size 8   (1x3 long, 8x1 short)   index incremented only if filter exists
   .CONST $aacdec.tns.COEF_FIELD                           48*ADDR_PER_WORD;   // size 56  (1x3x12 long, 8x1x7 short) index incremented only if filter exists
   .CONST $aacdec.tns.STRUC_SIZE                           104;


   // TNS constants
   .CONST $aacdec.TNS_MAX_ORDER_SHORT                      7;
   .CONST $aacdec.TNS_MAX_ORDER_LONG                       12;
   .CONST $aacdec.TNS_MAX_SFB_SHORT                        14;

   // Set the shift amount for the internal TNS coef values to allow the actual LPC
   // values to be outside the range -1 <= x < 1.
   .CONST $aacdec.TNS_LPC_SHIFT_AMOUNT                     3;

   // **************************************************************************
   //                              PNS constants
   // **************************************************************************

   // PNS constants for random number generation
   //  RAND_M (the multiplier) has been taken as the optimal value to use with
   //  24bit operation.
   //  See "Optimal Multipliers for pseudo-random number generation by the
   //  linear congruential method" by Itshak Borosh and Harald Niederreiter
   // Note: 1664525 is the best multiplier according to the criteria of "Knuth" for a 32-bit processor.
   //       But for simplicity it's much easier having the same random numbers between processsor
   //       architectures! So the implementation on K32 is to match K24 currently.
   .CONST $aacdec.PNS_RAND_M                               -4508331;
   .CONST $aacdec.PNS_RAND_C                               17;



   // **************************************************************************
   //                              MP4 constants
   // **************************************************************************



   .CONST $aacdec.MAX_AAC_FRAME_SIZE_MP4                   1536;
   .CONST $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES             8;
   .CONST $aacdec.MP4_MOOV_ATOM_MIN_NUM_BYTES              1024;

   // Fast Forward/Fast Rewind related parameters
   .CONST $aacdec.MP4_FF_REW_NULL                          0;
   .CONST $aacdec.MP4_FF_REW_IN_PROGRESS                   1;
   .CONST $aacdec.MP4_FF_REW_DONE                          2;
   .CONST $aacdec.MP4_FF_REW_SEEK_NOT_POSSIBLE             3;

   // Name fields of sub-atoms within MOOV atom
   // most significant and least significant 16-bits stored separately
   .CONST $aacdec.MP4_MDAT_TAG_LS_WORD                     (97  * 1<<8) + 116;
   .CONST $aacdec.MP4_MDAT_TAG_MS_WORD                     (109 * 1<<8) + 100;

   .CONST $aacdec.MP4_MOOV_TAG_LS_WORD                     (111 * 1<<8) + 118;
   .CONST $aacdec.MP4_MOOV_TAG_MS_WORD                     (109 * 1<<8) + 111;

   .CONST $aacdec.MP4_TRAK_TAG_LS_WORD                     (97  * 1<<8) + 107;
   .CONST $aacdec.MP4_TRAK_TAG_MS_WORD                     (116 * 1<<8) + 114;

   .CONST $aacdec.MP4_MDIA_TAG_LS_WORD                     (105 * 1<<8) + 97;
   .CONST $aacdec.MP4_MDIA_TAG_MS_WORD                     (109 * 1<<8) + 100;

   .CONST $aacdec.MP4_MINF_TAG_LS_WORD                     (110 * 1<<8) + 102;
   .CONST $aacdec.MP4_MINF_TAG_MS_WORD                     (109 * 1<<8) + 105;

   .CONST $aacdec.MP4_STBL_TAG_LS_WORD                     (98  * 1<<8) + 108;
   .CONST $aacdec.MP4_STBL_TAG_MS_WORD                     (115 * 1<<8) + 116;

   .CONST $aacdec.MP4_STSD_TAG_LS_WORD                     (115 * 1<<8) + 100;
   .CONST $aacdec.MP4_STSD_TAG_MS_WORD                     (115 * 1<<8) + 116;

   .CONST $aacdec.MP4_MP4A_TAG_LS_WORD                     (52  * 1<<8) + 97;
   .CONST $aacdec.MP4_MP4A_TAG_MS_WORD                     (109 * 1<<8) + 112;

   .CONST $aacdec.MP4_STSZ_TAG_LS_WORD                     (115 * 1<<8) + 122;
   .CONST $aacdec.MP4_STSZ_TAG_MS_WORD                     (115 * 1<<8) + 116;

   .CONST $aacdec.MP4_STCO_TAG_LS_WORD                     (99  * 1<<8) + 111;
   .CONST $aacdec.MP4_STCO_TAG_MS_WORD                     (115 * 1<<8) + 116;

   .CONST $aacdec.MP4_STSS_TAG_LS_WORD                     (115 * 1<<8) + 115;
   .CONST $aacdec.MP4_STSS_TAG_MS_WORD                     (115 * 1<<8) + 116;

   .CONST $aacdec.MP4_STZ2_TAG_LS_WORD                     (122 * 1<<8) + 50; // unused

   // sampling frequencies
   // first three sample rates are not supported
   .CONST $aacdec.OFFSET_TO_SAMPLE_RATE_TAG                3;

   // **************************************************************************
   //                              LTP constants
   // **************************************************************************

   // LTP fields
   .CONST $aacdec.LTP_MAX_SFB_LONG                         40;
   .CONST $aacdec.LTP_MAX_SFB_SHORT                        8;
   .CONST $aacdec.ltp.LAG_FIELD                            0*ADDR_PER_WORD;
   .CONST $aacdec.ltp.COEF_FIELD                           1*ADDR_PER_WORD;
   .CONST $aacdec.ltp.LONG_USED_FIELD                      2*ADDR_PER_WORD;
   .CONST $aacdec.ltp.STRUC_SIZE                           $aacdec.ltp.LONG_USED_FIELD/ADDR_PER_WORD + $aacdec.LTP_MAX_SFB_LONG;

   .CONST $aacdec.mdct.NUM_POINTS_FIELD                    0*ADDR_PER_WORD;
   .CONST $aacdec.mdct.FFT_REAL_ADDR_FIELD                 1*ADDR_PER_WORD;
   .CONST $aacdec.mdct.FFT_IMAG_ADDR_FIELD                 2*ADDR_PER_WORD;
   .CONST $aacdec.mdct.INPUT_ADDR_FIELD                    3*ADDR_PER_WORD;
   .CONST $aacdec.mdct.INPUT_ADDR_BR_FIELD                 4*ADDR_PER_WORD;
   .CONST $aacdec.mdct.STRUC_SIZE                          5;

   // set up for a scaled 512point FFT (gain of (2^-4)/$aacdec.AUDIO_OUT_SCALE_AMOUNT/2)
   .CONST $aacdec.MDCT_SCALE                               0.540029869446153;    // ((2^-4)/16)^(1/9)


   .CONST $aacdec.imdct.NUM_POINTS_FIELD                   0*ADDR_PER_WORD;
   .CONST $aacdec.imdct.INPUT_ADDR_FIELD                   1*ADDR_PER_WORD;
   .CONST $aacdec.imdct.OUTPUT_ADDR_FIELD                  2*ADDR_PER_WORD;
   .CONST $aacdec.imdct.INPUT_ADDR_BR_FIELD                3*ADDR_PER_WORD;
   .CONST $aacdec.imdct.OUTPUT_ADDR_BR_FIELD               4*ADDR_PER_WORD;
   .CONST $aacdec.imdct.STRUC_SIZE                         5;

   // **************************************************************************
   //                        WINDOWING constants
   // **************************************************************************

   // scaling amount from overlap add output to audio pcm data
   //   this value is linked to MDCT_SCALE and REQUANTIZE_EXTRA_SHIFT above
   .CONST $aacdec.AUDIO_OUT_SCALE_AMOUNT                   8;        // =(1<<3)

   // window sequences
   .CONST $aacdec.ONLY_LONG_SEQUENCE                       0;
   .CONST $aacdec.LONG_START_SEQUENCE                      1;
   .CONST $aacdec.EIGHT_SHORT_SEQUENCE                     2;
   .CONST $aacdec.LONG_STOP_SEQUENCE                       3;

   // window shapes
   .CONST $aacdec.SIN_WINDOW                               0;
   .CONST $aacdec.KAISER_WINDOW                            1;

   // **************************************************************************
   //                            HUFFMAN constants
   // **************************************************************************

   // huffman code books
   .CONST $aacdec.ZERO_HCB                                 0;
   .CONST $aacdec.FIRST_PAIR_HCB                           5;
   .CONST $aacdec.ESC_HCB                                  11;
   .CONST $aacdec.QUAD_LEN                                 4;
   .CONST $aacdec.PAIR_LEN                                 2;
   .CONST $aacdec.NOISE_HCB                                13;
   .CONST $aacdec.INTENSITY_HCB2                           14;
   .CONST $aacdec.INTENSITY_HCB                            15;
   .CONST $aacdec.ESC_FLAG                                 16;

   // ICS fields
   .CONST $aacdec.ics.MAX_SFB_FIELD                        0*ADDR_PER_WORD;
   .CONST $aacdec.ics.NUM_SWB_FIELD                        1*ADDR_PER_WORD;
   .CONST $aacdec.ics.NUM_WINDOW_GROUPS_FIELD              2*ADDR_PER_WORD;
   .CONST $aacdec.ics.NUM_WINDOWS_FIELD                    3*ADDR_PER_WORD;
   .CONST $aacdec.ics.WINDOW_SEQUENCE_FIELD                4*ADDR_PER_WORD;
   .CONST $aacdec.ics.WINDOW_GROUP_LENGTH_FIELD            5*ADDR_PER_WORD;    // size 8
   .CONST $aacdec.ics.WINDOW_SHAPE_FIELD                   13*ADDR_PER_WORD;
   .CONST $aacdec.ics.SCALE_FACTOR_GROUPING_FIELD          14*ADDR_PER_WORD;
   .CONST $aacdec.ics.SWB_OFFSET_PTR_FIELD                 15*ADDR_PER_WORD;
   .CONST $aacdec.ics.SECT_SFB_OFFSET_PTR_FIELD            16*ADDR_PER_WORD;
   .CONST $aacdec.ics.NUM_SEC_FIELD                        17*ADDR_PER_WORD;   // size 8
   .CONST $aacdec.ics.SECT_CB_PTR_FIELD                    25*ADDR_PER_WORD;
   .CONST $aacdec.ics.SECT_START_PTR_FIELD                 26*ADDR_PER_WORD;
   .CONST $aacdec.ics.SECT_END_PTR_FIELD                   27*ADDR_PER_WORD;
   .CONST $aacdec.ics.SFB_CB_PTR_FIELD                     28*ADDR_PER_WORD;
   .CONST $aacdec.ics.GLOBAL_GAIN_FIELD                    29*ADDR_PER_WORD;
   .CONST $aacdec.ics.SCALEFACTORS_PTR_FIELD               30*ADDR_PER_WORD;
   .CONST $aacdec.ics.MS_MASK_PRESENT_FIELD                31*ADDR_PER_WORD;
   .CONST $aacdec.ics.MS_USED_PTR_FIELD                    32*ADDR_PER_WORD;
   .CONST $aacdec.ics.NOISE_USED_FIELD                     33*ADDR_PER_WORD;
   .CONST $aacdec.ics.PULSE_DATA_PTR_FIELD                 34*ADDR_PER_WORD;
   .CONST $aacdec.ics.TNS_DATA_PTR_FIELD                   35*ADDR_PER_WORD;
   .CONST $aacdec.ics.PREDICTOR_DATA_PRESENT_FIELD         36*ADDR_PER_WORD;
   .CONST $aacdec.ics.LTP_INFO_PTR_FIELD                   37*ADDR_PER_WORD;
   .CONST $aacdec.ics.LTP_INFO_CH2_PTR_FIELD               38*ADDR_PER_WORD;
   .CONST $aacdec.ics.TNS_ENCODE_DONE_FIELD                39*ADDR_PER_WORD;
   .CONST $aacdec.ics.TNS_N_FILT_TEMP_FIELD                40*ADDR_PER_WORD;
   .CONST $aacdec.ics.PREV_WINDOW_SEQ_EQ_LONG_START_FIELD  41*ADDR_PER_WORD;
   .CONST $aacdec.ics.STRUC_SIZE                           42;

   .CONST $aacdec.pulse.NUMBER_PULSE_FIELD                 0*ADDR_PER_WORD;
   .CONST $aacdec.pulse.PULSE_START_SFB_FIELD              1*ADDR_PER_WORD;
   .CONST $aacdec.pulse.PULSE_OFFSET_FIELD                 2*ADDR_PER_WORD;    // size 5
   .CONST $aacdec.pulse.PULSE_AMP_FIELD                    7*ADDR_PER_WORD;    // size 5
   .CONST $aacdec.pulse.STRUC_SIZE                         12;

   // **************************************************************************
   //                            MISC constants
   // **************************************************************************

   // syntatic elements
   .CONST $aacdec.ID_SCE                                   0;
   .CONST $aacdec.ID_CPE                                   1;
   .CONST vID_CCE                                          2;
   .CONST $aacdec.ID_LFE                                   3;
   .CONST $aacdec.ID_DSE                                   4;
   .CONST $aacdec.ID_PCE                                   5;
   .CONST $aacdec.ID_FIL                                   6;
   .CONST $aacdec.ID_END                                   7;

   // currently we only allow decoding of stereo (1 CPE) and mono (1 SCE)
   .CONST $aacdec.MAX_NUM_SCES                             1;
   .CONST $aacdec.MAX_NUM_CPES                             1;

#endif
