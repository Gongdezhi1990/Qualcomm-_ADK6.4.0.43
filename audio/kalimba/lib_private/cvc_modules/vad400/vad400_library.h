// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

// *****************************************************************************
// NOTICE TO ANYONE CHANGING THIS FILE:
// IF YOU UPDATE THE VAD DATA OBJECT WITH NEW FIELD(S) THEN
// REMEMBER TO CHANGE THE CORRESPONDING C HEADER FILE 'vad400_c.h'
// WITH THE NEW FIELD(S) AS WELL
// *****************************************************************************

// *****************************************************************************
// NAME:
//    vad400 Library
//
// DESCRIPTION:
//
// *****************************************************************************


#ifndef VAD400_LIB_H
#define VAD400_LIB_H

// library build version
.CONST $VAD400_VERSION 0x010000;

//******************************************************************************
// VAD400 data structure element offset definitions
//
// The following data object is used for all vad400 modules
//
//******************************************************************************

// @DATA_OBJECT VAD_PRAMS_DATAOBJECT

// @DOC_FIELD_TEXT Attack Time to speech estimation of the VAD.
// @DOC_FIELD_FORMAT Q1.23 arch3 | Q1.31 arch4
.CONST   $M.vad400.Parameter.ATTACK_TC_FIELD              0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Decay Time to speech estimation of the VAD.
// @DOC_FIELD_FORMAT Q1.23 arch3 | Q1.31 arch4
.CONST   $M.vad400.Parameter.DECAY_TC_FIELD               1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Attack Time to noise estimation of the VAD.
// @DOC_FIELD_FORMAT Q1.23 arch3 | Q1.31 arch4
.CONST   $M.vad400.Parameter.ENVELOPE_TC_FIELD            2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Initialization time (in sec.) for VAD when the vad detection in 0.
// @DOC_FIELD_FORMAT Integer
.CONST   $M.vad400.Parameter.INIT_FRAME_THRESH_FIELD      3*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Ratio betwen speech and input dymanic range for VAD detection.
// @DOC_FIELD_FORMAT Q1.23 arch3 | Q1.31 arch4
.CONST   $M.vad400.Parameter.RATIO_FIELD                  4*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Minimum amplitude for VAD detection.
// @DOC_FIELD_FORMAT Q12.12 arch3 | Q12.20 arch4
.CONST   $M.vad400.Parameter.MIN_SIGNAL_FIELD             5*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Minimum power envelope for VAD detection.
// @DOC_FIELD_FORMAT Q12.12 arch3 | Q12.20 arch4
.CONST   $M.vad400.Parameter.MIN_MAX_ENVELOPE_FIELD       6*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Dynamic threshold between maximum power and power to avoid ramping in stationary noise.
// @DOC_FIELD_FORMAT Q12.12 arch3 | Q12.20 arch4
.CONST   $M.vad400.Parameter.DELTA_THRESHOLD_FIELD        7*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Threshold in seconds to have detection of stationary noise.
// @DOC_FIELD_FORMAT Integer
.CONST   $M.vad400.Parameter.COUNT_THRESHOLD_FIELD        8*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Size of parameter structure
// @DOC_FIELD_FORMAT Integer
.CONST   $M.vad400.Parameter.OBJECT_SIZE_FIELD            9;

// @END  DATA_OBJECT VAD_PRAMS_DATAOBJECT

// field definitions for vad400 data object

// // @DATA_OBJECT VAD_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to frame fuffer for Processing Modules ($frmbuffer in cbuffer.h)
// @DOC_FIELD_FORMAT Pointer
.CONST   $M.vad400.INPUT_PTR_FIELD              0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Pointer to parameters structure   @DOC_LINK         @DATA_OBJECT  VAD_PRAMS_DATAOBJECT
// @DOC_FIELD_FORMAT Pointer
.CONST   $M.vad400.PARAM_PTR_FIELD              1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Estimate of the amplitude of the signal
// @DOC_FIELD_FORMAT Q12.12 arch3 | Q12.20 arch4
.CONST   $M.vad400.E_FILTER_FIELD               2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Maximum of estimate of the amplitude of the signal
// @DOC_FIELD_FORMAT Q12.12 arch3 | Q12.20 arch4
.CONST   $M.vad400.E_FILTER_MAX_FIELD           3*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Minimum of estimate of the amplitude of the signal
// @DOC_FIELD_FORMAT Q12.12 arch3 | Q12.20 arch4
.CONST   $M.vad400.E_FILTER_MIN_FIELD           4*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Internal counter
// @DOC_FIELD_FORMAT Integer
.CONST   $M.vad400.COUNTER_DELTA_FIELD          5*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Counter used by "initialization time".
// @DOC_FIELD_FORMAT Integer
.CONST   $M.vad400.COUNTER_FIELD                6*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Flag identifying that voice has been detected
// @DOC_FIELD_FORMAT Integer
.CONST   $M.vad400.FLAG_FIELD                   7*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Bypass Flag
// @DOC_FIELD_FORMAT Bool
.CONST   $M.vad400.BYPASS_VAD                   8*ADDR_PER_WORD;
// @DOC_FIELD_TEXT Size of the VAD object
// @DOC_FIELD_FORMAT Integer
.CONST   $M.vad400.OBJECT_SIZE_FIELD            9;

// @END  DATA_OBJECT VAD_DATAOBJECT


.CONST   $M.vad400.DCB_PEQ_COEFF_OFFSET         0*ADDR_PER_WORD;
.CONST   $M.vad400.VAD_PEQ_COEFF_OFFSET         9*ADDR_PER_WORD;


#endif   //_VAD400_LIB_H
