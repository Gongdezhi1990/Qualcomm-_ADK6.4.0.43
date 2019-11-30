// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************


// *****************************************************************************
// NAME:
//    ADF200 Library
//
// DESCRIPTION:
//    This library provides the ADF 2.0.0 function for two mic processing.
// *****************************************************************************

#ifndef _ADF200_LIB_H
#define  _ADF200_LIB_H

// Library Build Version
.CONST $adf200_VERSION           0x020000; //2.0.0

// System Proportions
.CONST $adf200.BIN_SKIPPED       9;
.CONST $adf200.ADF_num_proc      55;

.CONST $adf200.DATA_SIZE_DM1		(17 * $adf200.ADF_num_proc);
.CONST $adf200.DATA_SIZE_DM2		(11 * $adf200.ADF_num_proc);

.CONST $adf200.SCRATCH_SIZE_DM1  (5 * $adf200.ADF_num_proc);
.CONST $adf200.SCRATCH_SIZE_DM2  (1 * $adf200.ADF_num_proc);

//********************************************************************
// ADF200 Data Structure Element Offset Definitions
//
// The following parameter block is used for all ADF200 process functions.
//
// The application using this library must include a data block of
// size STRUCT_SIZE.  The associated parameters should be initialized
// as defined below.
//
//********************************************************************

// Input/Output Block Offsets


// @DATA_OBJECT ADFDATAOBJECT

// @DOC_FIELD_TEXT Pointer to left channel (X0)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.X0_FIELD                      0;

// @DOC_FIELD_TEXT Pointer to right channel (X1) (real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.X1_FIELD                      1;

// Buffer Pointers
// @DOC_FIELD_TEXT Pointer to DM1 data block, with size of '$adf200.DATA_SIZE_DM1'
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.OFFSET_PTR_DM1data            2;

// @DOC_FIELD_FORMAT Pointer
// @DOC_FIELD_TEXT Pointer to DM2 data block, with size of '$adf200.DATA_SIZE_DM2'
.CONST $adf200.OFFSET_PTR_DM2data            3;

// Scratch Memory
// LPwrX0 = &$M.AEC_500.dm1.scratch1
// LPwrX1 = &$M.AEC_500.dm1.scratch1 + ADF_num_proc
// LpX1t  = &$M.AEC_500.dm1.scratch1 + 2*ADF_num_proc
// adaptFactor01 = &$M.AEC_500.dm1.scratch1 + 3*ADF_num_proc
// adaptFactor10 = &$M.AEC_500.dm1.scratch1 + 4*ADF_num_proc
// @DOC_FIELD_TEXT Pointer to DM1 scratch block, with size of '$adf200.SCRATCH_SIZE_DM1'
// @DOC_FIELD_FORMAT Pointer
.CONST   $adf200.OFFSET_SCRATCH_DM1          4;

// sgn_xdGa01 = &$M.AEC_500.dm2.scratch1
// @DOC_FIELD_TEXT Pointer to DM2 scratch block, with size of '$adf200.SCRATCH_SIZE_DM2'
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.OFFSET_SCRATCH_DM2            5;

// @DOC_FIELD_TEXT Pointer to adf_IntRatio0 buffer
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.PTR_INTRATIO0_FIELD           6;

// @DOC_FIELD_TEXT Pointer to adf_IntRatio1 buffer
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.PTR_INTRATIO1_FIELD           7;

// L_PostP_ctrl buffer or 0
// @DOC_FIELD_TEXT Pointer to L_PostP_ctrl[] buffer to store such variables or 0 to skip calculation.
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.OFFSET_L_POSTP_CTRL_PTR       8;

// @DOC_FIELD_TEXT Pointer to function of application preprocessing
// Do not trash M0/M1, do not trash FP(ARCH2) or r9(ARCH2).
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.FUNC_APP_PREP_FIELD           9;

// @DOC_FIELD_TEXT Pointer to ASF object used in pre processing
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.ASF_PTR                       10;

// @DOC_FIELD_TEXT Pointer to function of application post-processing
// @DOC_FIELD_FORMAT Pointer
.CONST $adf200.FUNC_APP_PP_FIELD             11;

// Z0=V0,Z1=V1
.CONST $adf200.OFFSET_SCRATCH_V0_FREQ_OBJ    12;
.CONST $adf200.OFFSET_Z0_FREQ_OBJ            12;

.CONST $adf200.OFFSET_SCRATCH_V1_FREQ_OBJ    13;
.CONST $adf200.OFFSET_Z1_FREQ_OBJ            13;

.CONST $adf200.POWER_UPHOLD_COUNT            14;
.CONST $adf200.BYPASS_FIELD                  15;
.CONST $adf200.HOLD_FIELD                    16;

.CONST $adf200.OFFSET_X0_FREQ_OBJ            17;
.CONST $adf200.OFFSET_X1_FREQ_OBJ            18;

.CONST $adf200.OFFSET_INTERNAL_START         19;

// -----------------------------------------------------------------------------
// Internal Variables Start from here
// -----------------------------------------------------------------------------

.CONST $adf200.OFFSET_muAt    		       $adf200.OFFSET_INTERNAL_START;      

// SP. Keep ct_Px0, dL2Px0FB, &L2Px0t0 Consequtive and in order
.CONST $adf200.OFFSET_ct_Px0    		       $adf200.OFFSET_muAt+1;
.CONST $adf200.OFFSET_dL2Px0FB  		       $adf200.OFFSET_ct_Px0+1;
.CONST $adf200.OFFSET_L2Px0t0  		       $adf200.OFFSET_dL2Px0FB+1;
// SP. Keep ct_Px1, dL2Px1FB, &L2Px1t0 Consequtive and in order
.CONST $adf200.OFFSET_ct_Px1    		       $adf200.OFFSET_L2Px0t0+1;
.CONST $adf200.OFFSET_dL2Px1FB  		       $adf200.OFFSET_ct_Px1+1;
.CONST $adf200.OFFSET_L2Px1t0  		       $adf200.OFFSET_dL2Px1FB+1;

.CONST $adf200.OFFSET_ct_init  		       $adf200.OFFSET_L2Px1t0+1;

.CONST $adf200.OFFSET_ADF_LALFALPZ              $adf200.OFFSET_ct_init+1;

// Size of the block
.CONST   $adf200.SCRATCH_BExpXx2             			$adf200.OFFSET_ADF_LALFALPZ+1;
.CONST   $adf200.SCRATCH_mean_dLpVt          			$adf200.SCRATCH_BExpXx2 + 1;
.CONST   $adf200.SCRATCH_T0_dLpVXt           			$adf200.SCRATCH_mean_dLpVt + 1;
.CONST   $adf200.SCRATCH_BExp_X              			$adf200.SCRATCH_T0_dLpVXt + 1;
.CONST   $adf200.SCRATCH_rptCounter          			$adf200.SCRATCH_BExp_X + 1;
.CONST   $adf200.STRUCT_SIZE                            $adf200.SCRATCH_rptCounter+1;
// @END  DATA_OBJECT ADFDATAOBJECT


.CONST   $MAX_24                 0x7fffff;
.CONST   $MIN_24                 0x800000;
.CONST   $MIN_48_MSB             0x800000;
.CONST   $MIN_48_LSB             0;
.CONST   $MAX_48_MSB             0x7fffff;
.CONST   $MAX_48_LSB             0xffffff;

#endif // end of _ADF200_LIB_H


