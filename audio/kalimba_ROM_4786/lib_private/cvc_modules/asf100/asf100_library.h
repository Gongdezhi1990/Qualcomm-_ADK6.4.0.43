// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef _ASF_LIB_H
#define _ASF_LIB_H

// -----------------------------------------------------------------------------
// ASF external constants
// -----------------------------------------------------------------------------
.CONST $asf100.BIN_SKIPPED                      1;
.CONST $asf100.NUM_PROC                         63;
.CONST $asf100.SCRATCH_SIZE_DM1               (10 * $asf100.NUM_PROC);
.CONST $asf100.SCRATCH_SIZE_DM2               (10 * $asf100.NUM_PROC);


// -----------------------------------------------------------------------------
// ASF data object structure
// -----------------------------------------------------------------------------

// @DATA_OBJECT ASF_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to left channel (X0)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.Z0_FIELD                         0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to right channel (X1) (real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.Z1_FIELD                         1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to X length (FFT size)
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.MODE_FIELD                       2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to cvc_variant
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.PTR_VARIANT_FIELD                3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Parameters     @DOC_LINK @DATA_OBJECT  ASF_PARAM_OBJECT
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.PARAM_FIELD                      4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DMP Mode       
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.PTR_DMP_MODE_FIELD               5*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Switchable Beamforming flag       
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.bf.BEAM0_SWITCHABLE_FLAG_FIELD   6*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Bypass Beam0 flag
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.bf.BYP_BEAM0_FLAG_FIELD          7*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Power Adjust     @DOC_LINK @DATA_OBJECT  ASF_PARAM_OBJECT
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.POWER_ADJUST_FIELD               8*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DM1 scratch block, with size of '$asf100.SCRATCH_SIZE_DM1'
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.SCRATCH_DM1_FIELD                9*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DM2 scratch block, with size of '$asf100.SCRATCH_SIZE_DM2'
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.SCRATCH_DM2_FIELD               10*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to beamformer function eg. near-end, far-end, etc
// @DOC_FIELD_FORMAT Pointer
.CONST $asf100.BEAM_FUNC_FIELD                 11*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Beam_mode = 0 -> Fixed Delay and Sum Beamforming
// @DOC_FIELD_TEXT           = 1 -> Adaptive MVDR Beamforming
// @DOC_FIELD_FORMAT Integer
.CONST $asf100.BEAM0_MODE_FIELD                12*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Beam_mode = 0 -> Fixed Delay and Sum Beamforming
// @DOC_FIELD_TEXT           = 1 -> Adaptive MVDR Beamforming
// @DOC_FIELD_FORMAT Integer
.CONST $asf100.BEAM1_MODE_FIELD                13*ADDR_PER_WORD;

// Start frequency objects (X0/X1)
.CONST $asf100.X0_FREQ_OBJ                     14*ADDR_PER_WORD;
.CONST $asf100.X1_FREQ_OBJ                     15*ADDR_PER_WORD;
// End frequency objects (X0/X1)

// Pointers to SCRATCH memory
// Start consecutive (W0/W1/Z0_FDSB)
//X0_tmp and X1_tmp(used in wb case)

.CONST $asf100.bf.SCRATCH_X0_TMP_BUF_FREQ_OBJ  16*ADDR_PER_WORD;
.CONST $asf100.bf.SCRATCH_X1_TMP_BUF_FREQ_OBJ  17*ADDR_PER_WORD;
//save X0 
.CONST $asf100.bf.X0_SAVED_FREQ_OBJ            18*ADDR_PER_WORD;
//save X1
.CONST $asf100.bf.X1_SAVED_FREQ_OBJ            19*ADDR_PER_WORD;
//Z0_aux
.CONST $asf100.bf.Z0_AUX_FREQ_OBJ              20*ADDR_PER_WORD;
//Z1_aux
.CONST $asf100.bf.Z1_AUX_FREQ_OBJ              21*ADDR_PER_WORD;
.CONST $asf100.WIND_FLAG_PTR                   22*ADDR_PER_WORD;
.CONST $asf100.INTERNAL_START_FIELD            23*ADDR_PER_WORD;

// .............................................................................
// Internal fields
// .............................................................................

// DATA pointers
.CONST $asf100.PX0N_FIELD                 0 + $asf100.INTERNAL_START_FIELD;
.CONST $asf100.PX1N_FIELD                 ADDR_PER_WORD + $asf100.PX0N_FIELD;
.CONST $asf100.PXCRN_FIELD                ADDR_PER_WORD + $asf100.PX1N_FIELD;
.CONST $asf100.PXCIN_FIELD                ADDR_PER_WORD + $asf100.PXCRN_FIELD;
.CONST $asf100.COH_FIELD                  ADDR_PER_WORD + $asf100.PXCIN_FIELD;              // q1.23 (arch4: q1.31)
.CONST $asf100.WNR_G_FIELD                ADDR_PER_WORD + $asf100.COH_FIELD;
.CONST $asf100.BEAM0_CC_REAL_FIELD        ADDR_PER_WORD + $asf100.WNR_G_FIELD;              // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM0_CC_IMAG_FIELD        ADDR_PER_WORD + $asf100.BEAM0_CC_REAL_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_CC_REAL_FIELD        ADDR_PER_WORD + $asf100.BEAM0_CC_IMAG_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_CC_IMAG_FIELD        ADDR_PER_WORD + $asf100.BEAM1_CC_REAL_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM0_PHI_REAL_FIELD       ADDR_PER_WORD + $asf100.BEAM1_CC_IMAG_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM0_PHI_IMAG_FIELD       ADDR_PER_WORD + $asf100.BEAM0_PHI_REAL_FIELD;     // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM_W0_AUX_REAL_FIELD     ADDR_PER_WORD + $asf100.BEAM0_PHI_IMAG_FIELD;     // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM_W0_AUX_IMAG_FIELD     ADDR_PER_WORD + $asf100.BEAM_W0_AUX_REAL_FIELD;   // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM_W1_AUX_REAL_FIELD     ADDR_PER_WORD + $asf100.BEAM_W0_AUX_IMAG_FIELD;   // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM_W1_AUX_IMAG_FIELD     ADDR_PER_WORD + $asf100.BEAM_W1_AUX_REAL_FIELD;   // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_PHI_REAL_FIELD       ADDR_PER_WORD + $asf100.BEAM_W1_AUX_IMAG_FIELD;   // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_PHI_IMAG_FIELD       ADDR_PER_WORD + $asf100.BEAM1_PHI_REAL_FIELD;     // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM0_W1_REAL_FIELD        ADDR_PER_WORD + $asf100.BEAM1_PHI_IMAG_FIELD;     // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM0_W1_IMAG_FIELD        ADDR_PER_WORD + $asf100.BEAM0_W1_REAL_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_W0_REAL_FIELD        ADDR_PER_WORD + $asf100.BEAM0_W1_IMAG_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_W0_IMAG_FIELD        ADDR_PER_WORD + $asf100.BEAM1_W0_REAL_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_W1_REAL_FIELD        ADDR_PER_WORD + $asf100.BEAM1_W0_IMAG_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_W1_IMAG_FIELD        ADDR_PER_WORD + $asf100.BEAM1_W1_REAL_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM0_TR_ASF_FIELD         ADDR_PER_WORD + $asf100.BEAM1_W1_IMAG_FIELD;      // q1.23 (arch4: q1.31)
.CONST $asf100.BEAM1_TR_ASF_FIELD         ADDR_PER_WORD + $asf100.BEAM0_TR_ASF_FIELD;       // q1.23 (arch4: q1.31)
.CONST $asf100.COH_COS_FIELD              ADDR_PER_WORD + $asf100.BEAM1_TR_ASF_FIELD;           
.CONST $asf100.COH_SIN_FIELD              ADDR_PER_WORD + $asf100.COH_COS_FIELD;
.CONST $asf100.COH_G_FIELD                ADDR_PER_WORD + $asf100.COH_SIN_FIELD;
.CONST $asf100.COH_IMAG_FIELD             ADDR_PER_WORD + $asf100.COH_G_FIELD;
// Preprocessing Input channel pointers
.CONST $asf100.PREP_X0_FIELD              ADDR_PER_WORD + $asf100.COH_IMAG_FIELD;
.CONST $asf100.PREP_X1_FIELD              ADDR_PER_WORD + $asf100.PREP_X0_FIELD;

// Scratch pointers
// Start consecutive (PXCRNT:PXCINT)
.CONST $asf100.SCRATCH_PXCRNT_FIELD       ADDR_PER_WORD + $asf100.PREP_X1_FIELD;
.CONST $asf100.SCRATCH_PXCINT_FIELD       ADDR_PER_WORD + $asf100.SCRATCH_PXCRNT_FIELD;
.CONST $asf100.SCRATCH_PX0NT_FIELD        ADDR_PER_WORD + $asf100.SCRATCH_PXCINT_FIELD;
.CONST $asf100.SCRATCH_PX1NT_FIELD        ADDR_PER_WORD + $asf100.SCRATCH_PX0NT_FIELD;
// End consecutive (PXCRNT:PXCINT)
.CONST $asf100.SCRATCH_LPX0T_FIELD        ADDR_PER_WORD + $asf100.SCRATCH_PX1NT_FIELD;   // q8.16 (arch4: q8.24)
.CONST $asf100.SCRATCH_LPX1T_FIELD        ADDR_PER_WORD + $asf100.SCRATCH_LPX0T_FIELD;   // q8.16 (arch4: q8.24)

// Module Bypass Flag
.CONST $asf100.BYPASS_FLAG_WNR_FIELD      ADDR_PER_WORD + $asf100.SCRATCH_LPX1T_FIELD;
.CONST $asf100.BYPASS_FLAG_COH_FIELD      ADDR_PER_WORD + $asf100.BYPASS_FLAG_WNR_FIELD;
.CONST $asf100.BYPASS_FLAG_BF_FIELD       ADDR_PER_WORD + $asf100.BYPASS_FLAG_COH_FIELD;

// .............................................................................
// WNR Object fields
// .............................................................................
.CONST $asf100.wnr.G1_FIELD               ADDR_PER_WORD + $asf100.BYPASS_FLAG_BF_FIELD;
.CONST $asf100.wnr.PHS_FACTOR_EXP_FIELD   ADDR_PER_WORD + $asf100.wnr.G1_FIELD;
.CONST $asf100.wnr.PHS_FACTOR_LB_FIELD    ADDR_PER_WORD + $asf100.wnr.PHS_FACTOR_EXP_FIELD;
.CONST $asf100.wnr.PHS_FACTOR_TR_FIELD    ADDR_PER_WORD + $asf100.wnr.PHS_FACTOR_LB_FIELD;
.CONST $asf100.wnr.PHS_FACTOR_HB_FIELD    ADDR_PER_WORD + $asf100.wnr.PHS_FACTOR_TR_FIELD;
.CONST $asf100.wnr.MEAN_PWR_FIELD         ADDR_PER_WORD + $asf100.wnr.PHS_FACTOR_HB_FIELD;
.CONST $asf100.wnr.MEAN_G_FIELD           ADDR_PER_WORD + $asf100.wnr.MEAN_PWR_FIELD;
.CONST $asf100.wnr.MEAN_CC0_FIELD         ADDR_PER_WORD + $asf100.wnr.MEAN_G_FIELD;
.CONST $asf100.wnr.COH_ATK_FIELD          ADDR_PER_WORD + $asf100.wnr.MEAN_CC0_FIELD;
.CONST $asf100.wnr.COH_DEC_FIELD          ADDR_PER_WORD + $asf100.wnr.COH_ATK_FIELD;
.CONST $asf100.wnr.DETECT_FLAG_FIELD      ADDR_PER_WORD + $asf100.wnr.COH_DEC_FIELD;
.CONST $asf100.wnr.COHERENCE_FIELD        ADDR_PER_WORD + $asf100.wnr.DETECT_FLAG_FIELD;
.CONST $asf100.wnr.WIND_FIELD             ADDR_PER_WORD + $asf100.wnr.COHERENCE_FIELD;
.CONST $asf100.wnr.THRESH_SILENCE_FIELD   ADDR_PER_WORD + $asf100.wnr.WIND_FIELD;
.CONST $asf100.WNROBJ_END_FIELD           0 + $asf100.wnr.THRESH_SILENCE_FIELD;
// WNR Object fields end


// .............................................................................
// BF Object fields
// .............................................................................
// following are used by BF

// End consecutive (W0/W1/Z0_FDISB)
.CONST $asf100.bf.SCRATCH_PS0T_FIELD         ADDR_PER_WORD + $asf100.WNROBJ_END_FIELD;          // q1.23 (arch4: q1.31)
.CONST $asf100.bf.SCRATCH_NPX0_FIELD         ADDR_PER_WORD + $asf100.bf.SCRATCH_PS0T_FIELD;     // q4.20 (arch4: q4.28)
.CONST $asf100.bf.SCRATCH_PS1T_FIELD         ADDR_PER_WORD + $asf100.bf.SCRATCH_NPX0_FIELD;     // q1.23 (arch4: q1.31)
.CONST $asf100.bf.SCRATCH_NEG_D_FIELD        ADDR_PER_WORD + $asf100.bf.SCRATCH_PS1T_FIELD;     // q3.21 (arch4: q3.29)
.CONST $asf100.bf.BEAM0_W0_REAL_FIELD        ADDR_PER_WORD + $asf100.bf.SCRATCH_NEG_D_FIELD;    // q1.23 (arch4: q1.31)
.CONST $asf100.bf.BEAM_TEMP_FIELD            ADDR_PER_WORD + $asf100.bf.BEAM0_W0_REAL_FIELD;
.CONST $asf100.bf.BF_SCRATCH_ADD_REAL        ADDR_PER_WORD + $asf100.bf.BEAM_TEMP_FIELD;    
.CONST $asf100.bf.BF_SCRATCH_ADD_IMAG        ADDR_PER_WORD + $asf100.bf.BF_SCRATCH_ADD_REAL;
.CONST $asf100.BF_END_FIELD                  0 + $asf100.bf.BF_SCRATCH_ADD_IMAG;
// BF Object fields end

.CONST $asf100.STRUC_SIZE                    1 + ($asf100.BF_END_FIELD >> LOG2_ADDR_PER_WORD);

// @END  DATA_OBJECT ASF_DATAOBJECT



// -----------------------------------------------------------------------------
// ASF parameter structure
// -----------------------------------------------------------------------------
// @DATA_OBJECT  DATA_OBJECT ASF_PARAM_OBJECT

// @DOC_FIELD_TEXT      Microphone distance Parameters (in meter), range is .005 to .070 (request to bring upper range to .15)
// @DOC_FIELD_TEXT      use cases : headset - 0.5cm to 7 cm ; binaural headset - 15cm to 20cm ; handset - 7cm to 15cm ; handsfree(speaker phone) - 3cm to 20cm
// @DOC_FIELD_FORMAT    Q1.23 format (arch4: q1.31), in meter
.CONST $asf100.param.MIC_DIST_FIELD             0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Speech Degration Factor for Target Capture Beam. Range is 0.1 to 1.0
// @DOC_FIELD_FORMAT    Q1.23 format (arch4: q1.31)
.CONST $asf100.bf.param.BETA_FIELD              1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Direction of arrival of target 0
// @DOC_FIELD_FORMAT    Integer
.CONST $asf100.bf.param.DOA0                    2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Direction of arrival of target 1
// @DOC_FIELD_FORMAT    Integer
.CONST $asf100.bf.param.DOA1                    3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Wind Gain Aggressiveness .
// @DOC_FIELD_TEXT      The larger the value the more noise is reduced. 0: no wind reduction.
// @DOC_FIELD_FORMAT    Q1.23 format (arch4: q1.31)
.CONST $asf100.wnr.param.GAIN_AGGR_FIELD        4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Silence Threshold, in log2 dB.
// @DOC_FIELD_TEXT      Signal will be treated as non-wind silence if power is below this threshold.
// @DOC_FIELD_FORMAT    Q8.16 format (arch4: q8.24)
.CONST $asf100.wnr.param.WNR_WIND_POW_TH_FIELD  5*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      One microphone Wind detection Hold (number of frames)
// @DOC_FIELD_FORMAT    Integer
.CONST $asf100.wnr.param.HOLD_FIELD             6*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Wind Noise Reduction Phase Threshold. Range is 0 to 1.0
// @DOC_FIELD_FORMAT    Q1.23 format (arch4: q1.31)
.CONST $asf100.wnr.param.THRESH_PHASE_FIELD     7*ADDR_PER_WORD;

// @DOC_FIELD_TEXT      Wind Noise Reduction Coherence Threshold. Range is 0 to 1.0
// @DOC_FIELD_FORMAT    Q1.23 format (arch4: q1.31)
.CONST $asf100.wnr.param.THRESH_COHERENCE_FIELD 8*ADDR_PER_WORD;


// @END  DATA_OBJECT ASF_PARAM_OBJECT

#endif   // _ASF_LIB_H
