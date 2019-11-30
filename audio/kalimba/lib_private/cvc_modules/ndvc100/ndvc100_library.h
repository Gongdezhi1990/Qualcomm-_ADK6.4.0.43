// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    $M.NDVC-alg1_0_0 Library
//
// DESCRIPTION:
//    This library provides the NDVC-alg1_0_0 function that attempts to normalize
//    the receive input signal to some desired level.
//
//    This library provides the following API functions:
//        $M.NDVC_alg1_0_0.Initialize.func
//        $M.NDVC_alg1_0_0.Process.func
// *****************************************************************************

#ifndef _NDVC_ALG1_0_0_LIB_H
#define _NDVC_ALG1_0_0_LIB_H


// *****************************************************************************
// $M.NDVC-alg1_0_0 Library Build Version
//
// 1.0.8
//    Original Narrowband revision
// 2.0.0:
//    Bug fix: (a) alfa_d and alfa_a swapped, (b) Libary module name error (typo)
//    Code change according to Matlab change: $NDVC_Alg1_0_0_LB(HB) added
//    Modified timer: internal fixed timer -> user controllable timer.
//    API changed:-> user attention
//    Optimzed, saved 51 PM words
// *****************************************************************************

   .CONST $NDVC_ALG1_0_0_VERSION          0x020000;   //2.0.0


   .CONST  $ndvc100.MAX_STEPS             8;


// ********************************************************************
// $M.NDVC-alg1_0_0 Data Object Element Offset Definitions
//
// The application using this library must include a data block
// of size BLOCK_SIZE.
//
// The following parameter elements must be initialized by the user:
//    OFFSET_MAXSTEPS
//    OFFSET_PTR_LPDNZ

// ********************************************************************

   // Parameters matching system parameters
// @DATA_OBJECT NDVC_PRAMS_DATAOBJECT

// @DOC_FIELD_TEXT Amount of hysteresis in the step-based volume control
// @DOC_FIELD_FORMAT Q1.23 format (arch4: Q1.31)
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_HYSTERESIS          		0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Attack Time to noise estimation
// @DOC_FIELD_FORMAT Q1.23 format (arch4: Q1.31)
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_ATK_TC              		1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Decay Time to noise estimation
// @DOC_FIELD_FORMAT Q1.23 format (arch4: Q1.31)
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_DEC_TC              		2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Denotes how many volume steps will be used by the NDVC
// @DOC_FIELD_FORMAT Integer
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_NUMVOLSTEPS         		3*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Noise level for maximum gain
// @DOC_FIELD_FORMAT Q8.16 format (arch4: Q8.24)
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_MAXNOISELVL         		4*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Noise level for minimum gain
// @DOC_FIELD_FORMAT Q8.16 format (arch4: Q8.24)
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_MINNOISELVL         		5*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Initial Frequency for the noise estimation
// @DOC_FIELD_FORMAT Integer
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_LB                  		6*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Final Frequency for the noise estimation
// @DOC_FIELD_FORMAT Integer
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.OFFSET_NDVC_HB                  		7*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Size of parameter structure
// @DOC_FIELD_FORMAT Integer
.CONST $M.NDVC_Alg1_0_0.PARAMETERS.BLOCKSIZE                    		8;

// @END  DATA_OBJECT NDVC_PRAMS_DATAOBJECT

// @DATA_OBJECT NDVC_DATAOBJECT

// @DOC_FIELD_TEXT NDVC bypass flag
// @DOC_FIELD_FORMAT Integer Flag
.CONST $ndvc100.OFFSET_BYPASS_FLAG              0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT NDVC Maximum volume steps (table size)
// @DOC_FIELD_FORMAT Integer
.CONST $M.NDVC_Alg1_0_0.OFFSET_MAXSTEPS         1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Pointer to frequency bin noise estimate
// @DOC_FIELD_FORMAT Pointer
.CONST $M.NDVC_Alg1_0_0.OFFSET_PTR_LPDNZ        2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT NDVC Parameters                                      @DOC_LINK         @DATA_OBJECT  NDVC_PRAMS_DATAOBJECT
// @DOC_FIELD_FORMAT Pointer
.CONST $M.NDVC_Alg1_0_0.OFFSET_PTR_PARAMS       3*ADDR_PER_WORD;
// @DOC_FIELD_TEXT NDVC Current Volume
// @DOC_FIELD_FORMAT Integer
.CONST $M.NDVC_Alg1_0_0.OFFSET_CURVOLLEVEL      4*ADDR_PER_WORD;
// @DOC_FIELD_TEXT NDVC Filtered mean(LpD_nz(LPDNZ_LB:LPDNZ_HB)
// @DOC_FIELD_FORMAT Q8.16 format (arch4: Q8.24)
.CONST $M.NDVC_Alg1_0_0.OFFSET_FILTSUMLPDNZ     5*ADDR_PER_WORD;
// @DOC_FIELD_TEXT NDVC Noise Delta
// @DOC_FIELD_FORMAT Q1.23 format (arch4: Q1.31)
.CONST $M.NDVC_Alg1_0_0.OFFSET_NOISE_DELTA      6*ADDR_PER_WORD;
// @DOC_FIELD_TEXT NDVC Inverse of numbinsummed
// @DOC_FIELD_FORMAT Q1.23 format (arch4: Q1.31)
.CONST $M.NDVC_Alg1_0_0.OFFSET_INV_NUMBIN       7*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Table of volume steps
// @DOC_FIELD_FORMAT Integer
.CONST $M.NDVC_Alg1_0_0.OFFSET_NOISELVLTBL      8*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Size of the NDVC data block (Add in MAX table size)
// @DOC_FIELD_FORMAT Integer
.CONST $ndvc100.STRUC_SIZE                      8;

// @END  DATA_OBJECT NDVC_DATAOBJECT

// *****************************************************************************
// MODULE:
//    $M.NDVC_alg1_0_0.Initialize.func
//
// DESCRIPTION:
//    Compute noiselvl table based on min/max noise level and number of steps
//    Initialize filt_sumLpD_nz to minimum noise level
//    Initialize current and previous volume index to 0
//    Initialize noise delta value based on hysteresis and noise step size
//    Reset timer value
//
// MODIFICATIONS:
//    07-03-09 Fri wms cleaned for component archive after code review
//    07-02-22 Thu wms modified for wideband expantion, and optimized
//    06-xx-yy zzz mv initial version for libaray format
//
// INPUTS:
//    r7 = &$NDVC_alg1_0_0_data_block;
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    rMAC, r0-r6, r10, I3, I4, I7, M1, DivResult
//
// CPU USAGE:
//    D-MEMORY: 0
//    P-MEMORY: 23
//    CYCLES:   meaningless
//
// NOTES:
//    Make sure that the minimum size of PTR_NOISELVLTBL is OFFSET_NUMVOLSTEPS
// *****************************************************************************


//*****************************************************************************
// MODULE:
//    $M.NDVC_alg1_0_0.Process.func
//
// DESCRIPTION:
//
// MODIFICATIONS:
//    07-03-09 Fri wms cleaned for component archive after code review
//    07-02-22 Thu wms debuged, modified for wideband expantion, and optimized
//    06-xx-yy zzz mv initial version for libaray format
//
// INPUTS:
//    r7 = &$NDVC_alg1_0_0_data_block;
//
// OUTPUTS:
//    Volume index updated in $M.NDVC_Alg1_0_0.OFFSET_CURVOLLEVEL
//
// TRASHED REGISTERS:
//    rMAC,r0-r6,r10,I3,I4,I7,M1 + $CVC.VolumeControl.Set_DAC_Adjustment
//
// CPU USAGE:
//    D-MEMORY: 0
//    P-MEMORY: 33
//    CYCLES:   32 + (HB-LB+1) + stalls + $CVC.VolumeControl.Set_DAC_Adjustment
//
// NOTES:
//    Upon volume change, if the timer is expired, the routine will
//    reset the tiemr and jump to $CVC.VolumeControl.Set_DAC_Adjustment.
//*****************************************************************************

#endif   //_NDVC_ALG1_0_0_LIB_H


