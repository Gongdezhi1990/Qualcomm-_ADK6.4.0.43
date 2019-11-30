// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
//
// 
//
// *****************************************************************************

#ifndef AEC510_LIB_H_INCLUDED
#define AEC510_LIB_H_INCLUDED

// *****************************************************************************
// AEC algorithm matlab version 510
//
// VERSION HISTORY:
//    1.0.0 - initial version aec500
//    2.0.0 - aec510: run-time HS/HF/CE configurable
// *****************************************************************************

// -----------------------------------------------------------------------------
// AEC510 external user constants
// -----------------------------------------------------------------------------
.CONST $aec510.Num_Primary_Taps              2;
.CONST $aec510.Num_Auxillary_Taps            0;

.CONST $aec510_HF.Num_Auxillary_Taps         3;
.CONST $aec510_HF.Num_Primary_Taps           8;

.CONST $aec510_HF_UWB.Num_Auxillary_Taps     4;
.CONST $aec510_HF_UWB.Num_Primary_Taps       12;

.CONST $aec510.RER_dim                       64;

// FBC maximum filter length 10ms
.CONST $aec510.fbc.nb.FILTER_SIZE            80;
.CONST $aec510.fbc.wb.FILTER_SIZE            160;
.CONST $aec510.fbc.uwb.FILTER_SIZE           240;
.CONST $aec510.fbc.swb.FILTER_SIZE           320;
.CONST $aec510.fbc.fb.FILTER_SIZE            480;

.CONST $aec510.fbc.PERD                      1;
.CONST $aec510.fbc.NIBBLE                    0.001 * 2;
.CONST $aec510.fbc.HFP_B_SZIE                6;

// -----------------------------------------------------------------------------
// AEC510 user parameter structure
// -----------------------------------------------------------------------------

// @DATA_OBJECT AECPARAMETEROBJECT

// @DOC_FIELD_TEXT Reference delay in seconds
// @DOC_FIELD_FORMAT Q7.17 (arch4: Q7.25)
.CONST $aec510.Parameter.REF_DELAY                    0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT AEC tail-length in seconds
// @DOC_FIELD_FORMAT Q7.17 (arch4: Q7.25)
.CONST $aec510.Parameter.TAIL_LENGTH                  1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT CND gain, default 1.0 in Q3.21 (CVC parameter)
// @DOC_FIELD_FORMAT Q3.21 (arch4: Q3.29)
.CONST $aec510.Parameter.OFFSET_CNG_Q_ADJUST          2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT, Comfort noise color selection -1=wht,0=brn,1=pnk,2=blu,3=pur (CVC parameter)
// @DOC_FIELD_FORMAT Flag
.CONST $aec510.Parameter.OFFSET_CNG_NOISE_COLOR       3*ADDR_PER_WORD;
// @DOC_FIELD_TEXT DTC aggressiveness, default 0.5 (Q1.23) (CVC parameter)
// @DOC_FIELD_FORMAT Q1.23 (arch4: Q1.31)
.CONST $aec510.Parameter.OFFSET_DTC_AGRESSIVENESS     4*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Maximum Power Margin, default 2.5 in (Q8.16) (CVC parameter for handsfree)
// @DOC_FIELD_FORMAT Q8.16 (arch4: Q8.24)
.CONST $aec510.Parameter.OFFSET_MAX_LPWR_MARGIN       5*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Flag for repeating AEC filtering
// @DOC_FIELD_TEXT Set to '1' for handsfree
// @DOC_FIELD_TEXT Set to '0' for headset (CVC parameter)
// @DOC_FIELD_FORMAT Flag
.CONST $aec510.Parameter.OFFSET_ENABLE_AEC_REUSE      6*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Reference Power Threshold. Default set to '$aec510.AEC_L2Px_HB' (Q8.16)
// @DOC_FIELD_TEXT CVC parameter for handsfree
// @DOC_FIELD_FORMAT Q8.16 (arch4: Q8.24)
.CONST $aec510.Parameter.OFFSET_AEC_REF_LPWR_HB       7*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Handsfree only. RER Adaptation. Default 0 (Handsfree CVC parameter)
// @DOC_FIELD_FORMAT Q1.23 (arch4: Q1.31)
.CONST $aec510.Parameter.OFFSET_RER_ADAPT             8*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Handsfree only. RER aggresiveness. Default 0x200000 (Q6.18) (Handsfree CVC parameter)
// @DOC_FIELD_FORMAT Q6.18 (arch4: Q6.26)
.CONST $aec510.Parameter.OFFSET_RER_AGGRESSIVENESS    9*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Handsfree only. RER power. Default 2 (Handsfree CVC parameter)
// @DOC_FIELD_FORMAT Integer
.CONST $aec510.Parameter.OFFSET_RER_POWER             10*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Threshold for DTC decision
// @DOC_FIELD_FORMAT Q8.16
.CONST $aec510.Parameter.OFFSET_L2TH_RERDT_OFF        11*ADDR_PER_WORD;
// @DOC_FIELD_TEXT RERDT aggressiveness
// @DOC_FIELD_FORMAT Q6.18
.CONST $aec510.Parameter.OFFSET_RERDT_ADJUST          12*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Handsfree only. RERDT power.
// @DOC_FIELD_FORMAT Integer
.CONST $aec510.Parameter.OFFSET_RERDT_POWER           13*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Speaker and Auto only. Threshold of FBC RER processing.
// @DOC_FIELD_FORMAT Q1.23
.CONST $aec510.Parameter.OFFSET_FBC_TH_RER            14*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Speaker and Auto only. FBC High Pass Pre-filter On/Off
// @DOC_FIELD_FORMAT Integer
.CONST $aec510.Parameter.OFFSET_FBC_HPF_ON            15*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Speaker and Auto only. FBC taillength, in milli-seconds
// @DOC_FIELD_FORMAT Q22.1
.CONST $aec510.Parameter.OFFSET_FBC_FILTER_LENGTH     16*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Pre-AEC noise estimator control
// @DOC_FIELD_FORMAT Q1.N
.CONST $aec510.Parameter.OFFSET_AEC_NS_CNTRL          17*ADDR_PER_WORD;

// @END  DATA_OBJECT AECPARAMETEROBJECT


// -----------------------------------------------------------------------------
// AEC510 data object structure
// -----------------------------------------------------------------------------

// @DATA_OBJECT AECDATAOBJECT

// @DOC_FIELD_TEXT AEC mode object (wideband/narrowband)
// @DOC_FIELD_TEXT Available objects:
// @DOC_FIELD_TEXT    - $aec510.mode.narrow_band
// @DOC_FIELD_TEXT    - $aec510.mode.wide_band
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.MODE_FIELD                    0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to AEC Parameters
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.PARAM_FIELD                   1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Variant Flag
// @DOC_FIELD_FORMAT Flag Pointer
.CONST $aec510.VARIANT_FIELD                 2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Maximum Primary Filter Taps
// @DOC_FIELD_FORMAT Integer
.CONST $aec510.MAX_FILTER_LENGTH_FIELD       3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Handsfree flag
// @DOC_FIELD_TEXT Headset use case if cleared
// @DOC_FIELD_TEXT Handsfree use case if set
// @DOC_FIELD_FORMAT Flag
.CONST $aec510.HF_FLAG_FIELD                 4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT DTC Enhancement Flag
// @DOC_FIELD_FORMAT Integer
.CONST $aec510.FLAG_DTC_ENH                  5*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to pre-AEC OMS G
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.OMS_G_FIELD                   6*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to pre-AEC OMS MS_LpN
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.OMS_LPN_FIELD                 7*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to AEC reference input stream
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.X_STREAM_FIELD                8*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to AEC reference delayed stream
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.X_STREAM_DELAY_FIELD          9*ADDR_PER_WORD;


// @DOC_FIELD_TEXT Pointer to AEC reference channel X (real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.X_FIELD                       10*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to real part of receive buffer X_buf, size of 'Num_FFT_Freq_Bins*Num_Primary_Taps'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.XBUF_REAL_FIELD               11*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to imaginary part of receive buffer X_buf, size of 'Num_FFT_Freq_Bins*Num_Primary_Taps'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.XBUF_IMAG_FIELD               12*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to BExp_X_buf (internal array, permanant), size of 'Num_Primary_Taps+1'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.XBUF_BEXP_FIELD               13*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to left channel FBC object
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.PTR_FBC_OBJ_FIELD             14*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to AEC (left) channel D (real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.D_FIELD                       15*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to real part of Ga, size of 'Num_FFT_Freq_Bins*Num_Primary_Taps'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.GA_REAL_FIELD                 16*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to imaginary part of Ga, size of 'Num_FFT_Freq_Bins*Num_Primary_Taps'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.GA_IMAG_FIELD                 17*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to BExp_Ga (internal array, permanant), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.GA_BEXP_FIELD                 18*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to second channel AEC object
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.DM_OBJ_FIELD                  19*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to LPwrX0 (internal array, permanant), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.LPWRX0_FIELD                  20*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to LPwrX1 (internal array, permanant), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.LPWRX1_FIELD                  21*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to RatFE (internal array, permanant), size of RER_dim
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.RATFE_FIELD                   22*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to imaginary part of Gr (RER internal complex array)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.RER_GR_IMAG_FIELD             23*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to real part of Gr (RER internal complex array)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.RER_GR_REAL_FIELD             24*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to SqGr (RER internal real array)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.RER_SQGR_FIELD                25*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to L2absGr (RER internal real array)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.RER_L2ABSGR_FIELD             26*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to LPwrD (RER internal real array)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.RER_LPWRD_FIELD               27*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to LpZ_nz (internal array, permanant), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.CNG_LPZNZ_FIELD               28*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to currently selected CNG noise shaping table (internal array, permanant), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer Q2.22
.CONST $aec510.CNG_CUR_NZ_TABLE_FIELD        29*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to L_adaptA (internal array, scratch in DM1), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_LADAPTA_FIELD          30*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to a scratch memory in DM2 with size of '2*$M.CVC.Num_FFT_Freq_Bins + 1'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_EXP_MTS_ADAPT_FIELD    31*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Attenuation (internal array, scratch in DM1), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_ATTENUATION_FIELD      32*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to W_ri (RER internal interleaved complex array)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_W_RI_FIELD             33*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to L_adaptR (RER internal real array)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_LADAPTR_FIELD          34*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DTC_lin array, size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_DTC_LIN_FIELD          35*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Scratch pointer to channel structure T
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_T_FIELD                36*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Scratch pointer to channel structure ET
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.ET_FIELD                      37*ADDR_PER_WORD;

// @DOC_FIELD_TEXT DTC status array for each frequency bins, scratch
// @DOC_FIELD_TEXT Size of Number of FFT bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.SCRPTR_RERDT_DTC_FIELD        38*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to real part of Gb, size of 'RER_dim*Num_Auxillary_Taps'
// @DOC_FIELD_TEXT Handsfree only. For headset set to '0'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.GB_REAL_FIELD                 39*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to imaginary part of Gb, size of 'RER_dim*Num_Auxillary_Taps'
// @DOC_FIELD_TEXT Handsfree only. For headset set to '0'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.GB_IMAG_FIELD                 40*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to BExp_Gb (internal array, permanant), size of RER_dim
// @DOC_FIELD_TEXT Handsfree only. For headset set to '0'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.GB_BEXP_FIELD                 41*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to L_RatSqG (internal array, permanant), size of RER_dim
// @DOC_FIELD_TEXT Handsfree only. For headset set to '0'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.L_RATSQG_FIELD                42*ADDR_PER_WORD;

// Internal AEC Data - Variables

// Time
.CONST $aec510.FRAME_SIZE_FIELD              $aec510.L_RATSQG_FIELD + ADDR_PER_WORD;
.CONST $aec510.REF_DELAY_FIELD               $aec510.FRAME_SIZE_FIELD + ADDR_PER_WORD;
.CONST $aec510.TAIL_LENGTH                   $aec510.REF_DELAY_FIELD + ADDR_PER_WORD;

// PREP
.CONST $aec510.G_OMS_IN2_FIELD              $aec510.TAIL_LENGTH + ADDR_PER_WORD;
.CONST $aec510.L2PXT_FIELD                  $aec510.G_OMS_IN2_FIELD + ADDR_PER_WORD;
.CONST $aec510.L2PDT_FIELD                  $aec510.L2PXT_FIELD + ADDR_PER_WORD;

// DTC
.CONST $aec510.L_MUA_FIELD                  $aec510.L2PDT_FIELD + ADDR_PER_WORD;
.CONST $aec510.L_MUB_FIELD                  $aec510.L_MUA_FIELD + ADDR_PER_WORD;
.CONST $aec510.L_DTC_HFREQ_FEF_FIELD        $aec510.L_MUB_FIELD + ADDR_PER_WORD;
.CONST $aec510.DTC_AVG_FIELD                $aec510.L_DTC_HFREQ_FEF_FIELD + ADDR_PER_WORD;
.CONST $aec510.DTC_PROB_FIELD               $aec510.DTC_AVG_FIELD + ADDR_PER_WORD;
.CONST $aec510.DTC_AVGRFE_FIELD             $aec510.DTC_PROB_FIELD + ADDR_PER_WORD;
.CONST $aec510.DTC_STDRFE_FIELD             $aec510.DTC_AVGRFE_FIELD + ADDR_PER_WORD;
.CONST $aec510.mn_L_RatSqGt                 $aec510.DTC_STDRFE_FIELD + ADDR_PER_WORD;

.CONST $aec510.OFFSET_L_RatSqG              $aec510.mn_L_RatSqGt + ADDR_PER_WORD;
.CONST $aec510.OFFSET_dL2PxFB				$aec510.OFFSET_L_RatSqG + ADDR_PER_WORD;
.CONST $aec510.OFFSET_L2Pxt0				$aec510.OFFSET_dL2PxFB + ADDR_PER_WORD;
.CONST $aec510.OFFSET_DTC_dLpX				$aec510.OFFSET_L2Pxt0 + ADDR_PER_WORD;
.CONST $aec510.OFFSET_DTC_LpXt_prev			$aec510.OFFSET_DTC_dLpX + ADDR_PER_WORD;

.CONST $aec510.OFFSET_tt_dtc				$aec510.OFFSET_DTC_LpXt_prev + ADDR_PER_WORD;
.CONST $aec510.OFFSET_ct_init			    $aec510.OFFSET_tt_dtc + ADDR_PER_WORD;
.CONST $aec510.OFFSET_ct_Px				    $aec510.OFFSET_ct_init + ADDR_PER_WORD;
.CONST $aec510.OFFSET_tt_cng				$aec510.OFFSET_ct_Px + ADDR_PER_WORD;
.CONST $aec510.OFFSET_L_DTC                 $aec510.OFFSET_tt_cng + ADDR_PER_WORD;

// RERDT
.CONST $aec510.OFFSET_LPXFB_RERDT           $aec510.OFFSET_L_DTC + ADDR_PER_WORD;
.CONST $aec510.RERDT_DTC_ACTIVE_FIELD       $aec510.OFFSET_LPXFB_RERDT + ADDR_PER_WORD;

// RER variables
.CONST $aec510.RER_AGGR_FIELD               $aec510.RERDT_DTC_ACTIVE_FIELD + ADDR_PER_WORD;
.CONST $aec510.RER_BEXP_FIELD               $aec510.RER_AGGR_FIELD + ADDR_PER_WORD;
.CONST $aec510.RER_E_FIELD                  $aec510.RER_BEXP_FIELD + ADDR_PER_WORD;
.CONST $aec510.RER_L2PET_FIELD              $aec510.RER_E_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_PXRS_FIELD            $aec510.RER_L2PET_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_PXRD_FIELD            $aec510.OFFSET_PXRS_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_PDRS_FIELD            $aec510.OFFSET_PXRD_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_PDRD_FIELD            $aec510.OFFSET_PDRS_FIELD + ADDR_PER_WORD;

// CNG
.CONST $aec510.OFFSET_OMS_AGGRESSIVENESS    $aec510.OFFSET_PDRD_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_CNG_offset			$aec510.OFFSET_OMS_AGGRESSIVENESS + ADDR_PER_WORD;
.CONST $aec510.CNG_OMS_G_FIELD              $aec510.OFFSET_CNG_offset + ADDR_PER_WORD;

// HD
.CONST $aec510.OFFSET_AEC_COUPLING          $aec510.CNG_OMS_G_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_HD_L_AECgain          $aec510.OFFSET_AEC_COUPLING + ADDR_PER_WORD;

// scratch variables
.CONST $aec510.LPWRX_MARGIN_FIELD           $aec510.OFFSET_HD_L_AECgain + ADDR_PER_WORD;
.CONST $aec510.MN_PWRX_DIFF_FIELD           $aec510.LPWRX_MARGIN_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_TEMP_FIELD            $aec510.MN_PWRX_DIFF_FIELD + ADDR_PER_WORD;

// NB/WB
.CONST $aec510.OFFSET_NUM_FREQ_BINS         $aec510.OFFSET_TEMP_FIELD + ADDR_PER_WORD;
.CONST $aec510.OFFSET_LPWRX_MARGIN_OVFL     $aec510.OFFSET_NUM_FREQ_BINS + ADDR_PER_WORD;
.CONST $aec510.OFFSET_LPWRX_MARGIN_SCL      $aec510.OFFSET_LPWRX_MARGIN_OVFL + ADDR_PER_WORD;

// HS/HF
.CONST $aec510.OFFSET_NUM_PRIMARY_TAPS      $aec510.OFFSET_LPWRX_MARGIN_SCL + ADDR_PER_WORD;
.CONST $aec510.OFFSET_NUM_AUXILLARY_TAPS    $aec510.OFFSET_NUM_PRIMARY_TAPS + ADDR_PER_WORD;
.CONST $aec510.OFFSET_AEC_L_MUA_ON          $aec510.OFFSET_NUM_AUXILLARY_TAPS + ADDR_PER_WORD;
.CONST $aec510.OFFSET_AEC_L_MUB_ON          $aec510.OFFSET_AEC_L_MUA_ON + ADDR_PER_WORD;
.CONST $aec510.OFFSET_AEC_ALFA_A            $aec510.OFFSET_AEC_L_MUB_ON + ADDR_PER_WORD;
.CONST $aec510.OFFSET_AEC_L_ALFA_A          $aec510.OFFSET_AEC_ALFA_A + ADDR_PER_WORD;
.CONST $aec510.OFFSET_DTC_scale_dLpX        $aec510.OFFSET_AEC_L_ALFA_A + ADDR_PER_WORD;

.CONST $aec510.FLAG_FBC_LRM_FIELD           $aec510.OFFSET_DTC_scale_dLpX + ADDR_PER_WORD;
.CONST $aec510.FLAG_AEC_LRM_FIELD           $aec510.FLAG_FBC_LRM_FIELD    + ADDR_PER_WORD;
.CONST $aec510.FLAG_BYPASS_CNG_FIELD        $aec510.FLAG_AEC_LRM_FIELD    + ADDR_PER_WORD;
.CONST $aec510.FLAG_BYPASS_RER_FIELD        $aec510.FLAG_BYPASS_CNG_FIELD + ADDR_PER_WORD;
.CONST $aec510.FLAG_BYPASS_RERDT_FIELD      $aec510.FLAG_BYPASS_RER_FIELD + ADDR_PER_WORD;
.CONST $aec510.FLAG_BYPASS_FBC_FIELD        $aec510.FLAG_BYPASS_RERDT_FIELD + ADDR_PER_WORD;

.CONST $aec510.STRUCT_SIZE                 ($aec510.FLAG_BYPASS_FBC_FIELD >> LOG2_ADDR_PER_WORD) + 1;

// @END  DATA_OBJECT AECDATAOBJECT


// -----------------------------------------------------------------------------
// AEC510 dual microphone (second channel) data object structure
// -----------------------------------------------------------------------------

// @DATA_OBJECT AECDM_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to external microphone mode
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.dm.PTR_MIC_MODE_FIELD           0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to rightt channel FBC object
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.dm.PTR_FBC1_OBJ_FIELD           1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to AEC (right) channel (D1)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.dm.D1_FIELD                     2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to real part of Ga1, size of 'Num_FFT_Freq_Bins*Num_Primary_Taps'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.dm.GA1_REAL_FIELD               3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to imaginary part of Ga1, size of 'Num_FFT_Freq_Bins*Num_Primary_Taps'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.dm.GA1_IMAG_FIELD               4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to BExp_Ga1 (internal array, permanant), size of Num_FFT_Freq_Bins
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.dm.GA1_BEXP_FIELD               5*ADDR_PER_WORD;

.CONST $aec510.dm.STRUCT_SIZE                  6;
// @END  DATA_OBJECT AECDM_DATAOBJECT

// -----------------------------------------------------------------------------
// AEC510 FDNLP/VSM sub-module object structure
// -----------------------------------------------------------------------------
// FDNLP - (Handsfree)
.CONST $M.FDNLP_500.OFFSET_VSM_HB              0*ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_VSM_LB              $M.FDNLP_500.OFFSET_VSM_HB + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_VSM_MAX_ATT         $M.FDNLP_500.OFFSET_VSM_LB + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_FDNLP_HB            $M.FDNLP_500.OFFSET_VSM_MAX_ATT + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_FDNLP_LB            $M.FDNLP_500.OFFSET_FDNLP_HB + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_FDNLP_MB            $M.FDNLP_500.OFFSET_FDNLP_LB + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_FDNLP_NBINS         $M.FDNLP_500.OFFSET_FDNLP_MB + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_FDNLP_ATT           $M.FDNLP_500.OFFSET_FDNLP_NBINS + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_FDNLP_ATT_THRESH    $M.FDNLP_500.OFFSET_FDNLP_ATT + ADDR_PER_WORD;
.CONST $M.FDNLP_500.OFFSET_FDNLP_ECHO_THRESH   $M.FDNLP_500.OFFSET_FDNLP_ATT_THRESH + ADDR_PER_WORD;
.CONST $M.FDNLP_500.STRUCT_SIZE                ($M.FDNLP_500.OFFSET_FDNLP_ECHO_THRESH >> LOG2_ADDR_PER_WORD) + 1;


// -----------------------------------------------------------------------------
// AEC510 NLP user parameter structure
// -----------------------------------------------------------------------------

.CONST $aec510.nlp.Parameter.OFFSET_HD_THRESH_GAIN           0*ADDR_PER_WORD;
.CONST $aec510.nlp.Parameter.OFFSET_TIER2_THRESH             1*ADDR_PER_WORD;
.CONST $aec510.nlp.Parameter.OFFSET_TIER1_CONFIG             $aec510.nlp.Parameter.OFFSET_TIER2_THRESH + ADDR_PER_WORD;
.CONST $aec510.nlp.Parameter.OFFSET_TIER2_CONFIG             $aec510.nlp.Parameter.OFFSET_TIER1_CONFIG + $M.FDNLP_500.STRUCT_SIZE*ADDR_PER_WORD;

.CONST $aec510.nlp.Parameter.HF_OBJECT_SIZE                  ($aec510.nlp.Parameter.OFFSET_TIER2_CONFIG >> LOG2_ADDR_PER_WORD) + $M.FDNLP_500.STRUCT_SIZE;
.CONST $aec510.nlp.Parameter.HS_OBJECT_SIZE                  2;


// -----------------------------------------------------------------------------
// AEC510 NLP object structure
// -----------------------------------------------------------------------------

// @DATA_OBJECT NLPDATAOBJECT

// @DOC_FIELD_TEXT Pointer to AEC master object
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.AEC_OBJ_PTR                  0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Non-Linear Processing Parameters
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.OFFSET_PARAM_PTR             1*ADDR_PER_WORD;

// FDNLP - VSM
// @DOC_FIELD_TEXT Pointer to current system call state flag
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.OFFSET_CALLSTATE_PTR         2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to receive path signal VAD flag
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.OFFSET_PTR_RCV_DETECT        3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Attenuation, same array as used in AEC main object
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.OFFSET_SCRPTR_Attenuation    4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to scratch memory with size of Num_FFT_Freq_Bins + RER_dim
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.OFFSET_SCRPTR                5*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Function pointer for FDNLP
// @DOC_FIELD_TEXT To enable: set '$aec510.FdnlpProcess'
// @DOC_FIELD_TEXT To disable: set '0'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.FDNLP_FUNCPTR                6*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Function pointer for VSM
// @DOC_FIELD_TEXT To enable: set '$aec510.VsmProcess'
// @DOC_FIELD_TEXT To disable: set '0'
// @DOC_FIELD_FORMAT Pointer
.CONST $aec510.nlp.VSM_FUNCPTR                  7*ADDR_PER_WORD;

// SP.  Internal FNDLP Data
.CONST $aec510.nlp.OFFSET_PTR_RatFE             $aec510.nlp.VSM_FUNCPTR + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_PTR_SqGr              $aec510.nlp.OFFSET_PTR_RatFE + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_PTR_L2ABSGR           $aec510.nlp.OFFSET_PTR_SqGr + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_SCRPTR_absGr          $aec510.nlp.OFFSET_PTR_L2ABSGR + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_SCRPTR_temp           $aec510.nlp.OFFSET_SCRPTR_absGr + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_PTR_CUR_CONFIG        $aec510.nlp.OFFSET_SCRPTR_temp + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_hd_ct_hold            $aec510.nlp.OFFSET_PTR_CUR_CONFIG + $M.FDNLP_500.STRUCT_SIZE*ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_hd_att                $aec510.nlp.OFFSET_hd_ct_hold + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_G_vsm                 $aec510.nlp.OFFSET_hd_att + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_fdnlp_cont_test       $aec510.nlp.OFFSET_G_vsm + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_mean_len              $aec510.nlp.OFFSET_fdnlp_cont_test + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_Vad_ct_burst          $aec510.nlp.OFFSET_mean_len + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_Vad_ct_hang           $aec510.nlp.OFFSET_Vad_ct_burst + ADDR_PER_WORD; // must follow ct_burst

.CONST $aec510.nlp.FLAG_BYPASS_HD_FIELD         $aec510.nlp.OFFSET_Vad_ct_hang + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_HC_TIER_STATE         $aec510.nlp.FLAG_BYPASS_HD_FIELD + ADDR_PER_WORD;
.CONST $aec510.nlp.FLAG_HD_MODE_FIELD           $aec510.nlp.OFFSET_HC_TIER_STATE + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_NUM_FREQ_BINS         $aec510.nlp.FLAG_HD_MODE_FIELD + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_D_REAL_PTR            $aec510.nlp.OFFSET_NUM_FREQ_BINS + ADDR_PER_WORD;
.CONST $aec510.nlp.OFFSET_D_IMAG_PTR            $aec510.nlp.OFFSET_D_REAL_PTR + ADDR_PER_WORD;

.CONST $aec510.nlp.STRUCT_SIZE                  ($aec510.nlp.OFFSET_D_IMAG_PTR >> LOG2_ADDR_PER_WORD) + 1;

// @END  DATA_OBJECT NLPDATAOBJECT


// -----------------------------------------------------------------------------
// FBC data object structure
// -----------------------------------------------------------------------------

// @DATA_OBJECT AECFBC_DATAOBJECT

.CONST $aec510.fbc.STREAM_D_FIELD               0*ADDR_PER_WORD;
.CONST $aec510.fbc.PTR_VADX_FIELD               1*ADDR_PER_WORD;
.CONST $aec510.fbc.G_A_FIELD                    2*ADDR_PER_WORD;
.CONST $aec510.fbc.G_B_FIELD                    3*ADDR_PER_WORD;
.CONST $aec510.fbc.PERD_FIELD                   4*ADDR_PER_WORD;
.CONST $aec510.fbc.NIBBLE_FIELD                 5*ADDR_PER_WORD;
.CONST $aec510.fbc.HPF_STREAM_FIELD             6*ADDR_PER_WORD;
.CONST $aec510.fbc.HPF_FILTER_FIELD             7*ADDR_PER_WORD;

// Internal fields
.CONST $aec510.fbc.FILTER_LENGTH_FIELD          8*ADDR_PER_WORD;
.CONST $aec510.fbc.ALFA_ERLE_FIELD              $aec510.fbc.FILTER_LENGTH_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.HD_TS_FIELD                  $aec510.fbc.ALFA_ERLE_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.SIL_TS_FIELD                 $aec510.fbc.HD_TS_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.G_A_AMP_FIELD                $aec510.fbc.SIL_TS_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.G_B_AMP_FIELD                $aec510.fbc.G_A_AMP_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.X_BUF_PWR_GW_FIELD           $aec510.fbc.G_B_AMP_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.X_BUF_PWR_MSW_FIELD          $aec510.fbc.X_BUF_PWR_GW_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.X_BUF_PWR_LSW_FIELD          $aec510.fbc.X_BUF_PWR_MSW_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.MU_MANTISA_FIELD             $aec510.fbc.X_BUF_PWR_LSW_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.MU_EXP_FIELD                 $aec510.fbc.MU_MANTISA_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.MU2_MANTISA_FIELD            $aec510.fbc.MU_EXP_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.MU2_EXP_FIELD                $aec510.fbc.MU2_MANTISA_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.DERLE_AMP_FIELD              $aec510.fbc.MU2_EXP_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.DERLE_FIL_FIELD              $aec510.fbc.DERLE_AMP_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.ERLE_FOLD_FIELD              $aec510.fbc.DERLE_FIL_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.HD_FOLD_FIELD                $aec510.fbc.ERLE_FOLD_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.HD_FLAG_FIELD                $aec510.fbc.HD_FOLD_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.HD_CNTR_FIELD                $aec510.fbc.HD_FLAG_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.TH_CNTR_FIELD                $aec510.fbc.HD_CNTR_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.SIL_CNTR_FIELD               $aec510.fbc.TH_CNTR_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.DIVERGE_FLAG_FIELD           $aec510.fbc.SIL_CNTR_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.LRAT_0_FIELD                 $aec510.fbc.DIVERGE_FLAG_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.LRAT_1_FIELD                 $aec510.fbc.LRAT_0_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.LRAT_2_FIELD                 $aec510.fbc.LRAT_1_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.L2P_IBUF_D_FIELD             $aec510.fbc.LRAT_2_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.L2P_OBUF_D_FIELD             $aec510.fbc.L2P_IBUF_D_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.L2P_OBUF_D_1_FIELD           $aec510.fbc.L2P_OBUF_D_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.L2P_OBUF_D_2_FIELD           $aec510.fbc.L2P_OBUF_D_1_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.L2_HD_GAIN_FIELD             $aec510.fbc.L2P_OBUF_D_2_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.HD_GAIN_FIELD                $aec510.fbc.L2_HD_GAIN_FIELD + ADDR_PER_WORD;
.CONST $aec510.fbc.IBUF_D_PRE_PWR_FIELD         $aec510.fbc.HD_GAIN_FIELD  + ADDR_PER_WORD;
.CONST $aec510.fbc.OBUF_D_PRE_PWR_FIELD         $aec510.fbc.IBUF_D_PRE_PWR_FIELD  + ADDR_PER_WORD;
.CONST $aec510.fbc.L2P_PREP_FBC_FIELD           $aec510.fbc.OBUF_D_PRE_PWR_FIELD  + ADDR_PER_WORD;
.CONST $aec510.fbc.L2P_PWR_DIFFERENCE_FIELD     $aec510.fbc.L2P_PREP_FBC_FIELD  + ADDR_PER_WORD;        
.CONST $aec510.fbc.STREAM_D_DLY_B_FIELD         MK1 + $aec510.fbc.L2P_PWR_DIFFERENCE_FIELD;
.CONST $aec510.fbc.STREAM_X_DLY_B_FIELD         MK1 + $aec510.fbc.STREAM_D_DLY_B_FIELD;
.CONST $aec510.fbc.STREAM_D_HI_FIELD            MK1 + $aec510.fbc.STREAM_X_DLY_B_FIELD;
.CONST $aec510.fbc.STREAM_X_HI_FIELD            MK1 + $aec510.fbc.STREAM_D_HI_FIELD;
.CONST $aec510.fbc.FLAG_BYPASS_HPF_FIELD        MK1 + $aec510.fbc.STREAM_X_HI_FIELD;
.CONST $aec510.fbc.TEMP0_FIELD                  MK1 + $aec510.fbc.FLAG_BYPASS_HPF_FIELD;
.CONST $aec510.fbc.TEMP1_FIELD                  MK1 + $aec510.fbc.TEMP0_FIELD;
.CONST $aec510.fbc.STRUCT_SIZE                 ($aec510.fbc.TEMP1_FIELD >> LOG2_ADDR_PER_WORD) + 1;

// @END  DATA_OBJECT AECFBC_DATAOBJECT
#endif // AEC510_LIB_H_INCLUDED
