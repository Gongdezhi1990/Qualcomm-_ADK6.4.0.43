// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef ADAPEQ_LIB_H
#define ADAPEQ_LIB_H

.CONST $ADAPEQ_VERSION                       0x01000A;


// Bands_Buffer contains two buffers: 1st half AEQ_BAND_PX + 2nd half band_pwr
.CONST   $M.AdapEq.Num_AEQ_Bands             3;                              // arch4: in words
.CONST   $M.AdapEq.Bands_Buffer_Length       ($M.AdapEq.Num_AEQ_Bands * 2);  // arch4: in words
.CONST   $M.AdapEq.Scratch_Length            65;                             // arch4: in words

// -----------------------------------------------------------------------------
// Constant offsets into the AdapEq Parameters Array
// -----------------------------------------------------------------------------
// @DATA_OBJECT ADAPEQPARAMOBJECT

// @DOC_FIELD_TEXT    Attack time of power average for each band.
// @DOC_FIELD_FORMAT  Q1.23 format (arch4: Q1.31)
.CONST   $M.AdapEqParam.ALFA_A_FIELD                  0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    1 - Attack time of power average for each band.
// @DOC_FIELD_FORMAT  Q1.23 format (arch4: Q1.31)
.CONST   $M.AdapEqParam.ONE_MINUS_ALFA_A_FIELD        1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Decay time of power average for each band
// @DOC_FIELD_FORMAT  Q1.23 format (arch4: Q1.31)
.CONST   $M.AdapEqParam.ALFA_D_FIELD                  2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    1 - Decay time of power average for each band
// @DOC_FIELD_FORMAT  Q1.23 format (arch4: Q1.31)
.CONST   $M.AdapEqParam.ONE_MINUS_ALFA_D_FIELD        3*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to Low Noise Target Power Ratio
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEqParam.TABLE_GOAL_LOW_FIELD          4*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to Low Noise Target Power Ratio
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEqParam.TABLE_GOAL_HIGH_FIELD         7*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to BEX Low Noise Target Power Ratio
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEqParam.TABLE_BEX_GOAL_HIGH2_FIELD    10*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to attenuation for BEX AEQ
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEqParam.TABLE_BEX_ATT_TOTAL_FIELD     13*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    BEX Noise dependent enable flags
// @DOC_FIELD_FORMAT  Flag
.CONST   $M.AdapEqParam.BEX_NOISE_LVL_FLAGS           16*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    BEX Low Inter-Band Step Size
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.BEX_PASS_LOW_FIELD            17*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Step size of gain transition between extended band-1 and band-2.
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.BEX_PASS_HIGH_FIELD           18*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    For low/ref/high band if the power of a lower band is greater than its neighbor higher band
// @DOC_FIELD_TEXT    and if the power difference is above this threshold the signal will be detected as tone signal.
// @DOC_FIELD_TEXT    During tone signal the AEQ will be disabled.
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.AEQ_POWER_TH_FIELD            19*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    The lower bound of AEQ gain allowed for low band and high band.
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.AEQ_MIN_GAIN_TH_FIELD         20*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    The higher bound of AEQ gain allowed for low band and high band.
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.AEQ_MAX_GAIN_TH_FIELD         21*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Transition threshold from low noise state to middle noise state based on NDVC volume.
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.VOL_STEP_UP_TH1_FIELD         22*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Transition threshold from middle noise state to high noise state based on NDVC volume.
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.VOL_STEP_UP_TH2_FIELD         23*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Step size of gain transition between low band and reference band.
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.AEQ_PASS_LOW_FIELD            24*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Inverse Low Band Step
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.INV_AEQ_PASS_LOW_FIELD        25*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Step size of gain transition between reference band and high band.
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.AEQ_PASS_HIGH_FIELD           26*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Inverse High Band Step
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.INV_AEQ_PASS_HIGH_FIELD       27*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Low band starting frequency.
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.LOW_INDEX_FIELD               28*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Low Band Width
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.LOW_BW_FIELD                  29*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Inverse Low Band Width
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.LOG2_LOW_INDEX_DIF_FIELD      30*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Mid Band Width
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.MID_BW_FIELD                  31*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Inverse Mid Band Width
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.LOG2_MID_INDEX_DIF_FIELD      32*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    High Band Width
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.HIGH_BW_FIELD                 33*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Inverse High Band Width
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEqParam.LOG2_HIGH_INDEX_DIF_FIELD     34*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Boundary frequency between low band and middle (reference) band.
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.MID1_INDEX_FIELD              35*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Boundary frequency between middle (reference) band and high band.
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.MID2_INDEX_FIELD              36*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Stop frequency of high band.
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEqParam.HIGH_INDEX_FIELD              37*ADDR_PER_WORD;

// @END  DATA_OBJECT ADAPEQPARAMOBJECT


// -----------------------------------------------------------------------------
// Constant offsets into the AdapEq data objects
// -----------------------------------------------------------------------------
// @DATA_OBJECT ADAPEQDATAOBJECT

// @DOC_FIELD_TEXT    Configuration Word for AdapEq
// @DOC_FIELD_FORMAT  Bit Mask
.CONST   $M.AdapEq.CONTROL_WORD_FIELD        0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Bitmask to bypass AdapEq
// @DOC_FIELD_FORMAT  Bit Mask
.CONST   $M.AdapEq.BYPASS_BIT_MASK_FIELD     $M.AdapEq.CONTROL_WORD_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Bitmask to enable or disable AdapEq BEX
// @DOC_FIELD_FORMAT  Bit Mask
.CONST   $M.AdapEq.BEX_BIT_MASK_FIELD        $M.AdapEq.BYPASS_BIT_MASK_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Number of FFT bins
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEq.NUM_FREQ_BINS             $M.AdapEq.BEX_BIT_MASK_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to parameter block
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEq.PTR_PARAMS_FIELD          $M.AdapEq.NUM_FREQ_BINS + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to frequency object for X (FFT of input signal x)
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEq.PTR_X_FREQ_OBJ            $M.AdapEq.PTR_PARAMS_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to frequency object for Z
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEq.PTR_Z_FREQ_OBJ            $M.AdapEq.PTR_X_FREQ_OBJ + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Frame counter
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEq.AEQ_EQ_COUNTER_FIELD      $M.AdapEq.PTR_Z_FREQ_OBJ + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Number of frames to wait before adaptation starts
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEq.AEQ_EQ_INIT_FRAME_FIELD   $M.AdapEq.AEQ_EQ_COUNTER_FIELD + ADDR_PER_WORD;

.CONST   $M.AdapEq.AEQ_GAIN_LOW_FIELD        $M.AdapEq.AEQ_EQ_INIT_FRAME_FIELD + ADDR_PER_WORD;

.CONST   $M.AdapEq.AEQ_GAIN_HIGH_FIELD       $M.AdapEq.AEQ_GAIN_LOW_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to the value of Voice Activity Detector (VAD)  .
// @DOC_FIELD_TEXT    Value = 1 for 'voice activity detected'  Value = 0 for 'no voice activity detected'
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEq.PTR_VAD_AGC_FIELD         $M.AdapEq.AEQ_GAIN_HIGH_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Average time constant for initial calculation of the powers
// @DOC_FIELD_FORMAT  Q1.23 format (arch4: Q1.31)
.CONST   $M.AdapEq.ALFA_ENV_FIELD            $M.AdapEq.PTR_VAD_AGC_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    1 - Average time constant for initial calculation of the powers
// @DOC_FIELD_FORMAT  Q1.23 format (arch4: Q1.31)
.CONST   $M.AdapEq.ONE_MINUS_ALFA_ENV_FIELD  $M.AdapEq.ALFA_ENV_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to AEQ bands buffer
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEq.PTR_AEQ_BAND_PX_FIELD     $M.AdapEq.ONE_MINUS_ALFA_ENV_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Current state of the AEQ (low noise / mid noise / high noise)
// @DOC_FIELD_FORMAT  Integer
.CONST   $M.AdapEq.STATE_FIELD               $M.AdapEq.PTR_AEQ_BAND_PX_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to step size
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEq.PTR_VOL_STEP_UP_FIELD     $M.AdapEq.STATE_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Tone power if any is detected .
// @DOC_FIELD_FORMAT  Q8.16 format (arch4: Q8.24)
.CONST   $M.AdapEq.AEQ_TONE_POWER_FIELD      $M.AdapEq.PTR_VOL_STEP_UP_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Output flag for AEQ Tone Detection
// @DOC_FIELD_FORMAT  Flag
.CONST   $M.AdapEq.AEQ_POWER_TEST_FIELD      $M.AdapEq.AEQ_TONE_POWER_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Pointer to the AEQ scratch buffer
// @DOC_FIELD_FORMAT  Pointer
.CONST   $M.AdapEq.PTR_SCRATCH_G_FIELD       $M.AdapEq.AEQ_POWER_TEST_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Flag for BEX enabling logic
// @DOC_FIELD_FORMAT  Flag
.CONST   $M.AdapEq.AEQ_SCRATCH_BEQ_ENABLED   $M.AdapEq.PTR_SCRATCH_G_FIELD + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    log of the sum of pXt
// @DOC_FIELD_FORMAT  Q  8.16 format (arch4: Q8.24)
.CONST   $M.AdapEq.AEQ_SCRATCH_log2_sum_pXt  $M.AdapEq.AEQ_SCRATCH_BEQ_ENABLED + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    linear gain
// @DOC_FIELD_FORMAT  Q  1.23 format (arch4: Q1.31)
.CONST   $M.AdapEq.AEQ_SCRATCH_lin_gain      $M.AdapEq.AEQ_SCRATCH_log2_sum_pXt + ADDR_PER_WORD;
// @DOC_FIELD_TEXT    Difference in band power between mid and high bands
// @DOC_FIELD_FORMAT  TBD
.CONST   $M.AdapEq.AEQ_SCRATCH_diff_high     $M.AdapEq.AEQ_SCRATCH_lin_gain + ADDR_PER_WORD;
.CONST   $M.AdapEq.STRUC_SIZE                ($M.AdapEq.AEQ_SCRATCH_diff_high >> LOG2_ADDR_PER_WORD) + 1;


// @END  DATA_OBJECT ADAPEQDATAOBJECT


#endif
