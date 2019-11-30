// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef DMS100_LIB_H
#define DMS100_LIB_H

// -----------------------------------------------------------------------------
// OMS270 version number
//    v.0.0.1: initial version
//    v.1.0.0: wideband
//    v.1.1.0: wnr
//    v.1.2.0: wideband resource reduction
//    v.2.0.0: PBP
//    v.2.1.0: PBP/LINEAR
//
// DMS100 version number
//    ver 1.0.0 - from OMS270 v.2.1.0
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// OMS270 PBP mode external constants
// -----------------------------------------------------------------------------
.CONST $oms270.MAX_MIN_SEARCH_LENGTH            68;
.CONST $oms270.PBP.DIM_NB                       25;
.CONST $oms270.PBP.DIM_WB                       30;
.CONST $oms270.PBP.NUM_LINEAR_BAND              15;
.CONST $oms270.PBP.MS_BAND                      2;

.CONST $M.oms270.QUE_LENGTH                     ($oms270.MAX_MIN_SEARCH_LENGTH * $oms270.PBP.MS_BAND);
.CONST $M.oms270.narrow_band.STATE_LENGTH       ($oms270.PBP.DIM_NB * 4 + 2 * $oms270.PBP.MS_BAND);
.CONST $M.oms270.wide_band.STATE_LENGTH         ($oms270.PBP.DIM_WB * 4 + 2 * $oms270.PBP.MS_BAND);
.CONST $M.oms270.narrow_band.SCRATCH_LENGTH     ($oms270.PBP.DIM_NB + 2 * ( 65-2-$oms270.PBP.NUM_LINEAR_BAND)); // max(121, $oms270.PBP.DIM_NB * 4 + $oms270.PBP.MS_BAND)
.CONST $M.oms270.wide_band.SCRATCH_LENGTH       ($oms270.PBP.DIM_WB + 2 * (129-2-$oms270.PBP.NUM_LINEAR_BAND)); // max(254, $oms270.PBP.DIM_WB * 4 + $oms270.PBP.MS_BAND)

// -----------------------------------------------------------------------------
// OMS270 Linear mode external constants
// -----------------------------------------------------------------------------
.CONST $oms270.linear.DIM                       65;
.CONST $oms270.linear.narrow_band.MS_BAND       2;
.CONST $oms270.linear.wide_band.MS_BAND         3;

.CONST $oms270.linear.narrow_band.QUE_LENGTH    ($M.oms270.QUE_LENGTH);
.CONST $oms270.linear.wide_band.QUE_LENGTH      ($oms270.MAX_MIN_SEARCH_LENGTH * $oms270.linear.wide_band.MS_BAND);
.CONST $oms270.linear.narrow_band.STATE_LENGTH  ($oms270.linear.DIM * 2 + 2 * $oms270.linear.narrow_band.MS_BAND);
.CONST $oms270.linear.wide_band.STATE_LENGTH    ($oms270.linear.DIM * 2 + 2 * $oms270.linear.wide_band.MS_BAND);
.CONST $oms270.linear.SCRATCH_LENGTH            ($oms270.linear.DIM * 4 + $oms270.linear.wide_band.MS_BAND);   // 263

// -----------------------------------------------------------------------------
// DMS100 external constants
// -----------------------------------------------------------------------------
.CONST $dms100.DIM                              65;
.CONST $dms100.MIN_SEARCH_LENGTH                34;
.CONST $dms100.MS_BAND                          8;
.CONST $dms100.MAX_SS_BAND                      8; 


.CONST $dms100.QUE_LENGTH                       ($dms100.MIN_SEARCH_LENGTH * $dms100.MS_BAND);
.CONST $dms100.STATE_LENGTH                     ($dms100.DIM * 6 + $dms100.MS_BAND * 2);
.CONST $dms100.SCRATCH_LENGTH                   ($dms100.DIM * 9 + $dms100.MS_BAND + $dms100.MAX_SS_BAND * 5);

// -----------------------------------------------------------------------------
// DMS100 PBP mode external constants
// -----------------------------------------------------------------------------
.CONST $dms100.pbp.narrow_band.DIM              25;
.CONST $dms100.pbp.narrow_band.MS_BAND          4;

.CONST $dms100.pbp.wide_band.DIM                30;
.CONST $dms100.pbp.wide_band.MS_BAND            5;

.CONST $dms100.pbp.narrow_band.SCRATCH_LENGTH   (9 * $dms100.pbp.narrow_band.DIM + $dms100.pbp.narrow_band.MS_BAND + $dms100.MAX_SS_BAND * 5 + 2 * ( 65-2-$oms270.PBP.NUM_LINEAR_BAND));
.CONST $dms100.pbp.wide_band.SCRATCH_LENGTH     (9 * $dms100.pbp.wide_band.DIM   + $dms100.pbp.wide_band.MS_BAND   + $dms100.MAX_SS_BAND * 5 + 2 * (129-2-$oms270.PBP.NUM_LINEAR_BAND));


.CONST $dms100.pbp.QUE_LENGTH                   ($dms100.MIN_SEARCH_LENGTH * $dms100.pbp.wide_band.MS_BAND); 
.CONST $dms100.pbp.STATE_LENGTH                 ($dms100.pbp.wide_band.DIM * 8 + $dms100.MS_BAND * 2);
                                                 
// -----------------------------------------------------------------------------
// OMS270/DMS100 DATA STRUCTURE
// -----------------------------------------------------------------------------

// @DATA_OBJECT OMS_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to output channel (Y)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.Y_FIELD                     0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to mono (left) channel (X)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.X_FIELD                     1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to cvc_variant
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PTR_VARIANT_FIELD           2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT OMS mode object (wideband/narrowband)
// @DOC_FIELD_TEXT Available objects:
// @DOC_FIELD_TEXT    - $oms270.narrow_band.mode
// @DOC_FIELD_TEXT    - $oms270.wide_band.mode
// @DOC_FIELD_TEXT    - $oms270.linear.narrow_band.mode
// @DOC_FIELD_TEXT    - $oms270.linear.wide_band.mode
// @DOC_FIELD_TEXT    - $dms100.narrow_band.mode
// @DOC_FIELD_TEXT    - $dms100.wide_band.mode
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PTR_MODE_FIELD              3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DMS Parameter  @DOC_LINK @DATA_OBJECT DMS_PARAM_OBJECT
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PARAM_FIELD                 4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to cvclib_table
// @DOC_FIELD_FORMAT 
.CONST $M.oms270.PBP_TABLE_FIELD             5*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to internal circular buffer, of size '$M.oms270.QUE_LENGTH'
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.LPX_QUEUE_START_FIELD       6*ADDR_PER_WORD;

// external interpolated G/LPXNZ
// @DOC_FIELD_TEXT Pointer to G
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PTR_G_FIELD                 7*ADDR_PER_WORD;
.CONST $dms100.PTR_G_FIELD                   $M.oms270.PTR_G_FIELD;

// @DOC_FIELD_TEXT Pointer to LpX_nz
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PTR_LPXNZ_FIELD             8*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to internal state memory
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PTR_STATE_FIELD             9*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to scratch memory
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PTR_SCRATCH_FIELD          10*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Harmonicity Threshould, CVC parameter
// @DOC_FIELD_FORMAT Q1.23
.CONST $M.oms270.HARM_THRESH_FIELD          11*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Flag to enable Minimun search (OMS internal process)
// @DOC_FIELD_FORMAT Flag
.CONST $M.oms270.MIN_SEARCH_ON_FIELD        12*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Flag to enable LSA (OMS internal process)
// @DOC_FIELD_FORMAT Flag
.CONST $M.oms270.MMSE_LSA_ON_FIELD          13*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Tone flag, where 0 indicates a tone
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.PTR_TONE_FLAG_FIELD        14*ADDR_PER_WORD;

// -----------------------------------------------------------------------------
// DMS100 Data Object
// -----------------------------------------------------------------------------
// @DOC_FIELD_TEXT Pointer to second (right) channel (D)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $dms100.D_FIELD                      15*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to external DMS mode
// @DOC_FIELD_FORMAT Pointer
.CONST $dms100.PTR_MIC_MODE_FIELD           16*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to parameter of Power Adjustment
// @DOC_FIELD_TEXT In log2 dB in Q8.16 format
// @DOC_FIELD_FORMAT Pointer
.CONST $dms100.PTR_POWR_ADJUST_FIELD        17*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to parameter of DOA0
// @DOC_FIELD_TEXT
// @DOC_FIELD_FORMAT Pointer
.CONST $dms100.PTR_DOA0_FIELD               18*ADDR_PER_WORD;

// @DOC_FIELD_TEXT DMS moudule internal fileds start from here
// @DOC_FIELD_FORMAT Field Offset
.CONST $dms100.INTERNAL_FIELD               19*ADDR_PER_WORD;

// User Data Pointers
.CONST $dms100.SPP_FIELD                    0 + $dms100.INTERNAL_FIELD;
.CONST $dms100.LRATIO_INTERPOLATED_FIELD    ADDR_PER_WORD + $dms100.SPP_FIELD;
.CONST $dms100.AUTO_AEC_OMS_FIELD           ADDR_PER_WORD + $dms100.LRATIO_INTERPOLATED_FIELD;
.CONST $dms100.LPDS_FIELD                   ADDR_PER_WORD + $dms100.AUTO_AEC_OMS_FIELD;
.CONST $dms100.LPN_FIELD                    ADDR_PER_WORD + $dms100.LPDS_FIELD;
.CONST $dms100.VAD_T_LIKE_FIELD             ADDR_PER_WORD + $dms100.LPN_FIELD;
.CONST $dms100.SNR_FIELD                    ADDR_PER_WORD + $dms100.VAD_T_LIKE_FIELD;

// Feature Bypass Flags
.CONST $dms100.MASTER_DMS_MODE_FIELD        ADDR_PER_WORD + $dms100.SNR_FIELD;
.CONST $dms100.BYPASS_VAD_S_FIELD           ADDR_PER_WORD + $dms100.MASTER_DMS_MODE_FIELD;
.CONST $dms100.BYPASS_SPP_FIELD             ADDR_PER_WORD + $dms100.BYPASS_VAD_S_FIELD;
.CONST $dms100.BYPASS_GSMOOTH_FIELD         ADDR_PER_WORD + $dms100.BYPASS_SPP_FIELD;
.CONST $dms100.BYPASS_NFLOOR_FIELD          ADDR_PER_WORD + $dms100.BYPASS_GSMOOTH_FIELD;
.CONST $dms100.BYPASS_NLIFT_FIELD           ADDR_PER_WORD + $dms100.BYPASS_NFLOOR_FIELD;
.CONST $dms100.BYPASS_AUTO_TH_FIELD         ADDR_PER_WORD + $dms100.BYPASS_NLIFT_FIELD; 
.CONST $dms100.GSCHEME_FIELD                ADDR_PER_WORD + $dms100.BYPASS_AUTO_TH_FIELD;

// Internal States
.CONST $dms100.VAD_S_VOICED_FIELD           ADDR_PER_WORD + $dms100.GSCHEME_FIELD;
.CONST $dms100.VAD_S_COUNT_FIELD            ADDR_PER_WORD + $dms100.VAD_S_VOICED_FIELD;
.CONST $dms100.VAD_S_LIKE_MEAN_FIELD        ADDR_PER_WORD + $dms100.VAD_S_COUNT_FIELD;
.CONST $dms100.VAD_S_BINL_FIELD             ADDR_PER_WORD + $dms100.VAD_S_LIKE_MEAN_FIELD;
.CONST $dms100.VAD_S_LIKE_MN_FIELD          ADDR_PER_WORD + $dms100.VAD_S_BINL_FIELD;
.CONST $dms100.VAD_S_NOISE_TH_FIELD         ADDR_PER_WORD + $dms100.VAD_S_LIKE_MN_FIELD;
.CONST $dms100.VAD_S_SPEECH_TH_FIELD        ADDR_PER_WORD + $dms100.VAD_S_NOISE_TH_FIELD;
.CONST $dms100.SS_BANDS_FIELD               ADDR_PER_WORD + $dms100.VAD_S_SPEECH_TH_FIELD; 

.CONST $dms100.VAD_VOICED_FIELD             ADDR_PER_WORD + $dms100.SS_BANDS_FIELD;
.CONST $dms100.VAD_COUNT_FIELD              ADDR_PER_WORD + $dms100.VAD_VOICED_FIELD;
.CONST $dms100.VAD_LIKE_MEAN_FIELD          ADDR_PER_WORD + $dms100.VAD_COUNT_FIELD;
.CONST $dms100.VAD_SPEECH_ON_FIELD          ADDR_PER_WORD + $dms100.VAD_LIKE_MEAN_FIELD;
.CONST $dms100.DMS_MODE_FIELD               ADDR_PER_WORD + $dms100.VAD_SPEECH_ON_FIELD;
.CONST $dms100.SNR_MN_FIELD                 ADDR_PER_WORD + $dms100.DMS_MODE_FIELD;   
.CONST $dms100.NSN_AGGRT_FIELD              ADDR_PER_WORD + $dms100.SNR_MN_FIELD;
        
// OMS/DMS common fileds
.CONST $M.oms270.DMS_COMMON_FIELD           ADDR_PER_WORD + $dms100.NSN_AGGRT_FIELD;

.CONST $M.oms270.FUNC_MMS_LPXS_FIELD      0*ADDR_PER_WORD + $M.oms270.DMS_COMMON_FIELD;
.CONST $M.oms270.FUNC_MMS_PROC_FIELD      1*ADDR_PER_WORD + $M.oms270.DMS_COMMON_FIELD;
.CONST $M.oms270.PTR_HARM_VALUE_FIELD     2*ADDR_PER_WORD + $M.oms270.DMS_COMMON_FIELD;
.CONST $M.oms270.LTILT_FIELD                ADDR_PER_WORD + $M.oms270.PTR_HARM_VALUE_FIELD;
.CONST $M.oms270.VOICED_FIELD               ADDR_PER_WORD + $M.oms270.LTILT_FIELD;
.CONST $M.oms270.MIN_SEARCH_COUNT_FIELD     ADDR_PER_WORD + $M.oms270.VOICED_FIELD;
.CONST $M.oms270.MIN_SEARCH_LENGTH_FIELD    ADDR_PER_WORD + $M.oms270.MIN_SEARCH_COUNT_FIELD;
.CONST $M.oms270.PTR_LPXS_FIELD             ADDR_PER_WORD + $M.oms270.MIN_SEARCH_LENGTH_FIELD;
.CONST $M.oms270.PTR_LPY_FIELD              ADDR_PER_WORD + $M.oms270.PTR_LPXS_FIELD;
.CONST $M.oms270.VOICED_COUNTER_FIELD       ADDR_PER_WORD + $M.oms270.PTR_LPY_FIELD;
.CONST $M.oms270.PTR_LPX_MIN_FIELD          ADDR_PER_WORD + $M.oms270.VOICED_COUNTER_FIELD;

.CONST $M.oms270.WIND_FIELD                 ADDR_PER_WORD + $M.oms270.PTR_LPX_MIN_FIELD;
.CONST $M.oms270.WNR_ENABLED_FIELD          ADDR_PER_WORD + $M.oms270.WIND_FIELD;
.CONST $M.oms270.INITIALISED_FIELD          ADDR_PER_WORD + $M.oms270.WNR_ENABLED_FIELD;
.CONST $M.oms270.LIKE_MEAN_FIELD            ADDR_PER_WORD + $M.oms270.INITIALISED_FIELD;
.CONST $M.oms270.TEMP_FIELD                 ADDR_PER_WORD + $M.oms270.LIKE_MEAN_FIELD;
.CONST $M.oms270.TEMP1_FIELD                ADDR_PER_WORD + $M.oms270.TEMP_FIELD;
        
// internal G/LPXNZ
// G/LpX_nz (OMS) -> G_G/MS_LpN (DMS)
.CONST $M.oms270.G_G_FIELD                  ADDR_PER_WORD + $M.oms270.TEMP1_FIELD;
.CONST $M.oms270.MS_LPN_FIELD               ADDR_PER_WORD + $M.oms270.G_G_FIELD;
.CONST $M.oms270.PTR_LPX_QUEUE_FIELD        ADDR_PER_WORD + $M.oms270.MS_LPN_FIELD;
.CONST $M.oms270.PTR_LPN_FIELD              ADDR_PER_WORD + $M.oms270.PTR_LPX_QUEUE_FIELD;

// Scratch pointer fields
.CONST $M.oms270.SCRATCH_LPXT_FIELD         ADDR_PER_WORD + $M.oms270.PTR_LPN_FIELD;
.CONST $M.oms270.SCRATCH_LIKE_FIELD         ADDR_PER_WORD + $M.oms270.SCRATCH_LPXT_FIELD;
.CONST $M.oms270.SCRATCH_NZLIFT_FIELD       ADDR_PER_WORD + $M.oms270.SCRATCH_LIKE_FIELD;
.CONST $M.oms270.SCRATCH_LPNZLIFT_FIELD     ADDR_PER_WORD + $M.oms270.SCRATCH_NZLIFT_FIELD;
.CONST $M.oms270.SCRATCH_LTILT_FIELD        ADDR_PER_WORD + $M.oms270.SCRATCH_LPNZLIFT_FIELD;
.CONST $dms100.SCRATCH_LPNMS_BANDSUM_FIELD  ADDR_PER_WORD + $M.oms270.SCRATCH_LTILT_FIELD; 
.CONST $dms100.SCRATCH_TLRATIO_FIELD        ADDR_PER_WORD + $dms100.SCRATCH_LPNMS_BANDSUM_FIELD;
.CONST $dms100.SCRATCH_VAD_S_SPH_TH_T_FIELD ADDR_PER_WORD + $dms100.SCRATCH_TLRATIO_FIELD;
.CONST $dms100.SCRATCH_VAD_S_NSE_TH_T_FIELD ADDR_PER_WORD + $dms100.SCRATCH_VAD_S_SPH_TH_T_FIELD;
.CONST $dms100.SCRATCH_VAD_S_LIKE_BAND_FIELD ADDR_PER_WORD + $dms100.SCRATCH_VAD_S_NSE_TH_T_FIELD;
.CONST $dms100.SCRATCH_VAD_S_LIKE_MN_FIELD  ADDR_PER_WORD + $dms100.SCRATCH_VAD_S_LIKE_BAND_FIELD;
.CONST $dms100.SCRATCH_VAD_S_NOISE_TH_FIELD ADDR_PER_WORD + $dms100.SCRATCH_VAD_S_LIKE_MN_FIELD;
.CONST $dms100.SCRATCH_VAD_S_SPEECH_TH_FIELD ADDR_PER_WORD + $dms100.SCRATCH_VAD_S_NOISE_TH_FIELD;
.CONST $dms100.SCRATCH_VAD_S_COUNT_FIELD    ADDR_PER_WORD + $dms100.SCRATCH_VAD_S_SPEECH_TH_FIELD;

// Scratch pointers, used in DMS 2mic PBP mode only
.CONST $dms100.SCRATCH_LPDT_FIELD           ADDR_PER_WORD + $dms100.SCRATCH_VAD_S_COUNT_FIELD;
.CONST $dms100.SCRATCH_CALCLPY_FIELD        ADDR_PER_WORD + $dms100.SCRATCH_LPDT_FIELD;

.CONST $M.oms270.BYPASS_FIELD               ADDR_PER_WORD + $dms100.SCRATCH_CALCLPY_FIELD;   

// Internal field to differentiate between the OMS/DMS mode
.CONST $M.oms270.OMS_MODE_FLAG_FIELD        ADDR_PER_WORD + $M.oms270.BYPASS_FIELD;

// DMS parameters
// Q8.16
.CONST $dms100.LPS_FLOOR_FIELD               ADDR_PER_WORD + $M.oms270.OMS_MODE_FLAG_FIELD;
.CONST $dms100.PTR_NS_POWER_FIELD            ADDR_PER_WORD + $dms100.LPS_FLOOR_FIELD;

.CONST $dms100.STRUC_SIZE                   ($dms100.PTR_NS_POWER_FIELD >> LOG2_ADDR_PER_WORD) + 1;;
.CONST $M.oms270.STRUC_SIZE                  $dms100.STRUC_SIZE;

// @END  DATA_OBJECT OMS_DATAOBJECT

// -----------------------------------------------------------------------------
// WNR data object
// -----------------------------------------------------------------------------

// @DATA_OBJECT OMSWNR_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to WNR user parameter object  @DOC_LINK @DATA_OBJECT WNR_PRAMOBJECT
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.wnr.PTR_WNR_PARAM_FIELD           0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Pointer to receive path VAD flag
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.wnr.PTR_RCVVAD_FLAG_FIELD         1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Pointer to send path VAD flag
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.wnr.PTR_SNDVAD_FLAG_FIELD         2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Pointer to external wind_flag variable
// @DOC_FIELD_FORMAT Pointer
.CONST $M.oms270.wnr.PTR_WIND_FLAG_FIELD           3*ADDR_PER_WORD;

// Internal fields
.CONST $M.oms270.wnr.SND_VAD_COUNT_FIELD           4*ADDR_PER_WORD;
.CONST $M.oms270.wnr.HOLD_FIELD                    5*ADDR_PER_WORD;
.CONST $M.oms270.wnr.MAX_ROLLOFF_BIN               6*ADDR_PER_WORD;
.CONST $M.oms270.wnr.POWER_THRES_FIELD             7*ADDR_PER_WORD;
.CONST $M.oms270.wnr.POWER_LEVEL_FIELD             8*ADDR_PER_WORD;
.CONST $M.oms270.wnr.COUNT_FIELD                   9*ADDR_PER_WORD;
// The following 3 fields need to be consecutive and in order
.CONST $M.oms270.wnr.HIGH_BIN_FIELD               10*ADDR_PER_WORD;
.CONST $M.oms270.wnr.LOW_BIN_FIELD                11*ADDR_PER_WORD;
.CONST $M.oms270.wnr.ROLLOFF_IDX_FIELD            12*ADDR_PER_WORD;
.CONST $M.oms270.wnr.FIX_ROLLOFF_BIN              13*ADDR_PER_WORD;
.CONST $M.oms270.wnr.FUNC_WIND_DETECT_FIELD       14*ADDR_PER_WORD;
.CONST $M.oms270.wnr.FUNC_WIND_REDUCTION_FIELD    15*ADDR_PER_WORD;
.CONST $M.oms270.wnr.FUNC_WIND_POWER_FIELD        16*ADDR_PER_WORD;
.CONST $M.oms270.wnr.STRUC_SIZE                   17;

// @END  DATA_OBJECT OMSWNR_DATAOBJECT


// -----------------------------------------------------------------------------
// WNR control parameter block offset
// -----------------------------------------------------------------------------
// @DATA_OBJECT WNR_PRAMOBJECT

// @DOC_FIELD_TEXT WNR aggressiveness, default 1.0, CVC parameter
// @DOC_FIELD_FORMAT Q1.23
.CONST $M.oms270.param.WNR_AGRESSIVENESS_FIELD     0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT WNR power threshold (dB in log2), below which signal is seen as non-wind, CVC parameter
// @DOC_FIELD_FORMAT Integer
.CONST $M.oms270.param.WNR_POWER_THRESHOLD_FIELD   1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT WNR detection hold (number of frames), CVC parameter
// @DOC_FIELD_FORMAT Integer
.CONST $M.oms270.param.WNR_HOLD_FIELD              2*ADDR_PER_WORD;

// @END  DATA_OBJECT WNR_PRAMOBJECT


// -----------------------------------------------------------------------------
// DMS100 Parameter Structure
// -----------------------------------------------------------------------------
// @DATA_OBJECT DMS_PARAM_OBJECT

// @DOC_FIELD_TEXT DMS aggressiveness, default 1.0
// @DOC_FIELD_FORMAT Q1.23
.CONST $dms100.param.AGRESSIVENESS_FIELD           0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Residual Noise floor (in log2 dB)
// @DOC_FIELD_FORMAT    Q8.16 format
.CONST $dms100.param.RESIDUAL_NOISE_FIELD          1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Non-Stationary Noise Suppression Aggresiveness
// @DOC_FIELD_FORMAT Q1.23
.CONST $dms100.param.NSN_AGGR_FIELD                2*ADDR_PER_WORD;

// @END  DATA_OBJECT DMS_PARAM_OBJECT

#endif // DMS100_LIB_H
