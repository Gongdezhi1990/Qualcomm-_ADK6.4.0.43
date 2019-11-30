// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef SSR_LIB_H
#define SSR_LIB_H

// -----------------------------------------------------------------------------
// SSR version number
//    v.0.0.1: initial version
//    v.1.0.0:
// -----------------------------------------------------------------------------
.CONST $SSR_VERSION                       0x010200;

// -----------------------------------------------------------------------------
// Defines used by the SSR implementation
// -----------------------------------------------------------------------------
#define FIXP_Q    12
#define ONE          (1<<FIXP_Q)
#define LOGZERO      (floor(-33.2711*ONE))   // log(1/2^48)
#define FIXP_SHIFT   FIXP_Q
#define REG_SIZE     24                // int operands
#define INT_PART     (REG_SIZE-1 - FIXP_Q)      // not considering the sign bit

// -----------------------------------------------------------------------------
// Constants used by the SSR implementation
// -----------------------------------------------------------------------------
.CONST   $M.SSR.SAMP_FREQ           8000;    // [Hz]
.CONST   $M.SSR.FRAME_SIZE          128;
.CONST   $M.SSR.FFT_SIZE            $M.SSR.FRAME_SIZE;
.CONST   $M.SSR.NUM_FILTERS         23;
.CONST   $M.SSR.MFCC_ORDER          12;       // c0 not considered
.CONST   $M.SSR.L2_FFTS2            6;        // log2(FFT_SIZE/2)
.CONST   $M.SSR.ENORM               356;      // enorm = 2.0 / NUM_FILTERS;  ((FixP)(ONE*2.0/NUM_FILTERS))   in Q.12
.CONST   $M.SSR.KLOG2               2839;     // ((FixP)(ONE*0.6931472))          log(2)
.CONST   $M.SSR.MAX_REG             0x7FFFFF;        // defined in limits.h for C
.CONST   $M.SSR.MIN_REG             0x800000;

.CONST   $M.SSR.NMODELS             5;
.CONST   $M.SSR.NSTATES             5;
.CONST   $M.SSR.NPARAMS             ($M.SSR.MFCC_ORDER+1);       // nr of MFCCoeffs
.CONST   $M.SSR.DELTAWIN            2;                    // 1: no deltas; 2: first order delta; 3: second order delta
.CONST   $M.SSR.OBS_SIZE            ($M.SSR.NPARAMS*$M.SSR.DELTAWIN);
.CONST   $M.SSR.REGRESS_COLS        (2*$M.SSR.DELTAWIN+1);
.CONST   $M.SSR.LZERO               ($M.SSR.MIN_REG + (4*ONE));      //  !!! hardcoded empirical - allow some room before Neg overflow
.CONST   $M.SSR.DELTACONST          (409);       // ONE * 0.1
.CONST   $M.SSR.BEAMTHRESHOLD       ($M.SSR.LZERO>>1);

// -----------------------------------------------------------------------------
// final state conditions constants
// -----------------------------------------------------------------------------
.CONST   $M.SSR.WORD_TOO_SHORT      7;     // #frames a recognised word must be longer (reset HMM state machine otherwise)
.CONST   $M.SSR.MIN_FINAL_STATES    2;     // #frames a recognised word must have successive final states

// -----------------------------------------------------------------------------
// uncert detection constants
// -----------------------------------------------------------------------------
// !!! based on Thr only
.CONST   $M.SSR.MIN_SUCC_ST         0;     // must pass at least # times from state n to n+1 (otherwise 'uncert')
.CONST   $M.SSR.MIN_MAIN_ST         0;     // must stay at least # frames on state 3 (otherwise 'uncert')

.CONST   $M.SSR.FORCE_UNCERT        -327680;  // ((FixP)(ONE*(-80.0)))    // a bad enough score, must be below CONFI_THR (LZERO would distort the statistics)
.CONST   $M.SSR.SCORE_BIAS          196608;   // ((FixP)(ONE*48.0))       // cluster the scores around zero (avoid overflows adding only negative numbers)
.CONST   $M.SSR.UNCERT_POS          3;                        // bit position for signaling uncert - ceil(log2(NMODELS))
.CONST   $M.SSR.CANDIDATE_STRENGTH  2;     // the other word was that many times more best_word frames

.CONST   $M.SSR.NOISE_BASE          (-24);  // default for noise_base user config value (input norm to -6dBFS)
.CONST   $M.SSR.UNCERT_OFFSET       (-112); // default for uncert_offset user config value
.CONST   $M.SSR.UTTERANCE_TIME_OUT  3000;   // [msec] default time out


// -----------------------------------------------------------------------------
// 1-mic config parameters constants
// -----------------------------------------------------------------------------
.CONST   $M.SSR.VOICE_DISAPPEARED   4;     // #frames to monitor OMS.voiced to detect false starts
.CONST   $M.SSR.MIN_VALID_WORD      18;    // min #frames for declaring False Start (if under)

// -----------------------------------------------------------------------------
// OMS startpoint detection constants
// -----------------------------------------------------------------------------
.CONST   $M.SSR.LPDNZ_PAD           8192;  // ((FixP)(ONE*(2.0)))
.CONST   $M.SSR.L2_LPDNZ_TRANS      1;         // this is a log2 of the above (note: only works for power of 2, otherwise a Div will be needed)
.CONST   $M.SSR.LPDNZ_MARGIN        3*ONE;     // ((FixP)(ONE*(3.0)))

.CONST   $M.SSR.MAX_AGGRESS         0x7FFFFF;  // ((int)(0x7FFFFF*1.0))      // Q23

// -----------------------------------------------------------------------------
// dynamic confidence threshold constants
// -----------------------------------------------------------------------------
.CONST   $M.SSR.A_CONFI_THR_Yes     -6963; // ((FixP)(ONE*(-1.7)))
.CONST   $M.SSR.A_CONFI_THR_No      -6144; // ((FixP)(ONE*(-1.5)))

// -----------------------------------------------------------------------------
// Constant offsets into the SSR data objects
// -----------------------------------------------------------------------------
.CONST $M.SSR.SSR_STRUC.CBUFFER_IN_OFFSET          0;
.CONST $M.SSR.SSR_STRUC.NOISE_BASE_OFFSET          $M.SSR.SSR_STRUC.CBUFFER_IN_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.UNCERT_OFFSET              $M.SSR.SSR_STRUC.NOISE_BASE_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.TIME_OUT_OFFSET            $M.SSR.SSR_STRUC.UNCERT_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.RECOGNISED_WORD_OFFSET     $M.SSR.SSR_STRUC.TIME_OUT_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.UNCERT_FLAG_OFFSET         $M.SSR.SSR_STRUC.RECOGNISED_WORD_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.FINISH_OFFSET              $M.SSR.SSR_STRUC.UNCERT_FLAG_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.CONFIDENCE_SCORE_OFFSET    $M.SSR.SSR_STRUC.FINISH_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.ASRDEC_PTR_OFFSET          $M.SSR.SSR_STRUC.CONFIDENCE_SCORE_OFFSET + 1;
// Oms section
// @DOC_FIELD_TEXT  Pointer to the FFT-frequency object, as used by OMS (send)
// @DOC_FIELD_FORMAT  Pointer
.CONST $M.SSR.SSR_STRUC.FFT_FREQ_OBJ               $M.SSR.SSR_STRUC.ASRDEC_PTR_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.VOICED_OFFSET              $M.SSR.SSR_STRUC.FFT_FREQ_OBJ + 1;
.CONST $M.SSR.SSR_STRUC.GAIN_OFFSET                $M.SSR.SSR_STRUC.VOICED_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.LIKE_MEAN_OFFSET           $M.SSR.SSR_STRUC.GAIN_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.LPX_NZ_OFFSET              $M.SSR.SSR_STRUC.LIKE_MEAN_OFFSET + 1;
.CONST $M.SSR.SSR_STRUC.BLOCK_SIZE                 $M.SSR.SSR_STRUC.LPX_NZ_OFFSET + 1;

// -----------------------------------------------------------------------------
// Constant offsets into the HMM data objects
// -----------------------------------------------------------------------------
.CONST $M.SSR.HMMDEF.NAME_OFFSET      		   0;
.CONST $M.SSR.HMMDEF.NSTATES_OFFSET    	   $M.SSR.HMMDEF.NAME_OFFSET + 1;
.CONST $M.SSR.HMMDEF.GMMS_OFFSET   		      $M.SSR.HMMDEF.NSTATES_OFFSET + 1;
.CONST $M.SSR.HMMDEF.EMISSION_TYPE_OFFSET    $M.SSR.HMMDEF.GMMS_OFFSET + 1;
.CONST $M.SSR.HMMDEF.START_PROB_OFFSET       $M.SSR.HMMDEF.EMISSION_TYPE_OFFSET + 1;
.CONST $M.SSR.HMMDEF.END_PROB_OFFSET         $M.SSR.HMMDEF.START_PROB_OFFSET + 1;
.CONST $M.SSR.HMMDEF.TRANSMAT_OFFSET         $M.SSR.HMMDEF.END_PROB_OFFSET + 1;
.CONST $M.SSR.HMMDEF.TIDX_OFFSET      		   $M.SSR.HMMDEF.TRANSMAT_OFFSET + 1;
.CONST $M.SSR.HMMDEF.PARTIAL_LIKE_OFFSET     $M.SSR.HMMDEF.TIDX_OFFSET + 1;
.CONST $M.SSR.HMMDEF.MAX_LIKE_OFFSET         $M.SSR.HMMDEF.PARTIAL_LIKE_OFFSET + 1;
.CONST $M.SSR.HMMDEF.MAX_STATE_OFFSET        $M.SSR.HMMDEF.MAX_LIKE_OFFSET + 1;
.CONST $M.SSR.HMMDEF.BLOCK_SIZE              $M.SSR.HMMDEF.MAX_STATE_OFFSET + 1;

// -----------------------------------------------------------------------------
// Constant offsets into the Decoder data objects
// -----------------------------------------------------------------------------
.CONST $M.SSR.DECODER_STRUCT.HMM_SET_OFFSET              0;
.CONST $M.SSR.DECODER_STRUCT.FINISH_OFFSET               $M.SSR.DECODER_STRUCT.HMM_SET_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.RESET_OFFSET                $M.SSR.DECODER_STRUCT.FINISH_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.BEST_WORD_OFFSET            $M.SSR.DECODER_STRUCT.RESET_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.BEST_SCORE_OFFSET           $M.SSR.DECODER_STRUCT.BEST_WORD_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.BEST_STATE_OFFSET           $M.SSR.DECODER_STRUCT.BEST_SCORE_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.OBS_OFFSET                  $M.SSR.DECODER_STRUCT.BEST_STATE_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.OBS_REGRESS_OFFSET          $M.SSR.DECODER_STRUCT.OBS_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.LOG_ENERGY_OFFSET           $M.SSR.DECODER_STRUCT.OBS_REGRESS_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.CONFIDENCE_SCORE_OFFSET     $M.SSR.DECODER_STRUCT.LOG_ENERGY_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.NR_BEST_FRAMES_OFFSET       $M.SSR.DECODER_STRUCT.CONFIDENCE_SCORE_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.SUCC_STA_CNT_OFFSET         $M.SSR.DECODER_STRUCT.NR_BEST_FRAMES_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.NR_MAIN_STATE_OFFSET        $M.SSR.DECODER_STRUCT.SUCC_STA_CNT_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.FINISH_CNT_OFFSET           $M.SSR.DECODER_STRUCT.NR_MAIN_STATE_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.RELIABILITY_OFFSET          $M.SSR.DECODER_STRUCT.FINISH_CNT_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.DECODER_STARTED_OFFSET      $M.SSR.DECODER_STRUCT.RELIABILITY_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.FRAME_COUNTER_OFFSET        $M.SSR.DECODER_STRUCT.DECODER_STARTED_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.VOICE_GONE_CNT_OFFSET       $M.SSR.DECODER_STRUCT.FRAME_COUNTER_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.AFTER_RESET_CNT_OFFSET      $M.SSR.DECODER_STRUCT.VOICE_GONE_CNT_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.SCORE_OFFSET                $M.SSR.DECODER_STRUCT.AFTER_RESET_CNT_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.SUM_RELI_OFFSET             $M.SSR.DECODER_STRUCT.SCORE_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.NOISE_ESTIMATE_OFFSET       $M.SSR.DECODER_STRUCT.SUM_RELI_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.NOISE_FRAME_COUNTER_OFFSET  $M.SSR.DECODER_STRUCT.NOISE_ESTIMATE_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.INITIALIZED_OFFSET          $M.SSR.DECODER_STRUCT.NOISE_FRAME_COUNTER_OFFSET + 1;
// external buffers
.CONST $M.SSR.DECODER_STRUCT.FBANK_COEFFS_OFFSET         $M.SSR.DECODER_STRUCT.INITIALIZED_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.MFC_COEFFS_OFFSET           $M.SSR.DECODER_STRUCT.FBANK_COEFFS_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.PARTIAL_LIKE_OFFSET         $M.SSR.DECODER_STRUCT.MFC_COEFFS_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.PARTIAL_LIKE_NEXT_OFFSET    $M.SSR.DECODER_STRUCT.PARTIAL_LIKE_OFFSET + 1;
.CONST $M.SSR.DECODER_STRUCT.BLOCK_SIZE                  $M.SSR.DECODER_STRUCT.PARTIAL_LIKE_NEXT_OFFSET + 1;

// Mixture structure
.CONST $M.SSR.MIXTURE_STRUCT.MIX_ID_OFFSET      0;
.CONST $M.SSR.MIXTURE_STRUCT.PRIORS_OFFSET      $M.SSR.MIXTURE_STRUCT.MIX_ID_OFFSET + 1;
.CONST $M.SSR.MIXTURE_STRUCT.GCONST_OFFSET      $M.SSR.MIXTURE_STRUCT.PRIORS_OFFSET + 1;
.CONST $M.SSR.MIXTURE_STRUCT.MEANS_OFFSET       $M.SSR.MIXTURE_STRUCT.GCONST_OFFSET + 1;
.CONST $M.SSR.MIXTURE_STRUCT.COVARS_OFFSET      $M.SSR.MIXTURE_STRUCT.MEANS_OFFSET + 1;
.CONST $M.SSR.MIXTURE_STRUCT.BLOCK_SIZE_OFFSET  $M.SSR.MIXTURE_STRUCT.COVARS_OFFSET + 1;

// State structure
.CONST $M.SSR.STATEDEF.NMIX_OFFSET              0;
.CONST $M.SSR.STATEDEF.MIX_OFFSET               $M.SSR.STATEDEF.NMIX_OFFSET + 1;
.CONST $M.SSR.STATEDEF.BLOCK_SIZE_OFFSET        $M.SSR.STATEDEF.MIX_OFFSET + 1;

// -----------------------------------------------------------------------------
// Constant offsets into the OMS data objects
// -----------------------------------------------------------------------------
.CONST $M.SSR.OMS_STRUC.OMS_OBJ_OFFSET          0;
.CONST $M.SSR.OMS_STRUC.FFT_REAL_OFFSET         $M.SSR.OMS_STRUC.OMS_OBJ_OFFSET + 1;
.CONST $M.SSR.OMS_STRUC.FFT_IMAG_OFFSET         $M.SSR.OMS_STRUC.FFT_REAL_OFFSET + 1;
.CONST $M.SSR.OMS_STRUC.SCALE_FACTOR_OFFSET     $M.SSR.OMS_STRUC.FFT_IMAG_OFFSET + 1;
.CONST $M.SSR.OMS_STRUC.VOICED_OFFSET           $M.SSR.OMS_STRUC.SCALE_FACTOR_OFFSET + 1;
.CONST $M.SSR.OMS_STRUC.GAIN_OFFSET             $M.SSR.OMS_STRUC.VOICED_OFFSET + 1;
.CONST $M.SSR.OMS_STRUC.LIKE_MEAN_OFFSET        $M.SSR.OMS_STRUC.GAIN_OFFSET + 1;
.CONST $M.SSR.OMS_STRUC.LPX_NZ_OFFSET           $M.SSR.OMS_STRUC.LIKE_MEAN_OFFSET + 1;
.CONST $M.SSR.OMS_STRUC.BLOCK_SIZE_OFFSET       $M.SSR.OMS_STRUC.LPX_NZ_OFFSET + 1;

// -----------------------------------------------------------------------------
// OMS scratch variables
// -----------------------------------------------------------------------------

// global scope: stay during entire or most of the process
#define SSR_S_best_score                        $scratch.s0
#define SSR_S_best_state                        $scratch.s1
#define SSR_S_best_word                         $scratch.s2
#define SSR_S_ptrDecoder                        $scratch.s3
#define SSR_S_ipredom                           $scratch.s5
#define SSR_S_n                                 $scratch.s6

// *****************************************************************************
// MODULE:
//    $M.__.func
//
// DESCRIPTION:
//
//  INPUTS
//    - r7 =
//
//  OUTPUTS:
//    - r7 =
//
// TRASHED REGISTERS:
//    r0, r1, r8, r10, I0, I1, I4, I5, loop
//
// CPU USAGE:
//
// NOTES:
// **********************************************************************

// *****************************************************************************
// MODULE:
//    $M.ssr.process.func
//
// DESCRIPTION:
//    SSR process
//
// INPUTS
//    - r7 =
//
//  OUTPUTS:
//
// TRASHED REGISTERS:
//    - Assumes everything
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************

#endif
