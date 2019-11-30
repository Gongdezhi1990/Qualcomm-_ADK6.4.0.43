// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

// *****************************************************************************
// DESCRIPTION:
//
//   This header files holds the offset values defined for the AGC-alg400
//   data structure.
//  
// MODIFICATIONS:
//
//      
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    AGC(Automatic Gain Control) Library
//
// DESCRIPTION:
//    This library provides AGC API function calls
//
//    The library provides the following routines:
//        $M.agc.Initialize.func
//        $M.agc.Process.func
//
// *****************************************************************************

#ifndef AGC400_LIB_H
#define AGC400_LIB_H

.CONST $AGC400_VERSION                           0x010000;

// Frame Threshold: 3 secnodes
.CONST   $M.agc400.const.FRAME_THRESHOLD        400;
.CONST   $M.agc400.const.UWB.FRAME_THRESHOLD    600;

// *****************************************************************************
// AGC Data Structure Element Offset Definitions
// 
// The application using this library must include a data block
//  of size BLOCK_SIZE.  The associated parameters should be initialized
// as defined below.
//
//******************************************************************************  
// @DATA_OBJECT PARAMETERS

// @DOC_FIELD_TEXT   initial gain value
// @DOC_FIELD_FORMAT   Q7.17 format
.CONST $M.agc400.Parameter.OFFSET_INITIAL_GAIN_FIELD       0*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   Target Level for AGC (PARAMETER)
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_AGC_TARGET_FIELD         1*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   TC for smoothing AGC gain when increasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_ATTACK_TC_FIELD          2*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   TC for smoothing AGC gain when decreasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_DECAY_TC_FIELD           3*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   TC for input level adjustment when increasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_ALPHA_A_90_FIELD         4*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   TC for input level adjustment when decreasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_ALPHA_D_90_FIELD         5*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   max AGC gain (PARAMETER)
// @DOC_FIELD_FORMAT   Q7.17 format
.CONST $M.agc400.Parameter.OFFSET_G_MAX_FIELD              6*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   Threshold for AGC to compress output sample
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_START_COMP_FIELD         7*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   in/out compression ratio applied when exceeding AGC TARGET  (PARAMETER)
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_COMP_FIELD               8*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   input threshold for forcing vad detection
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_INPUT_THRESHOLD_FIELD    9*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   adaptation rate acceleration factor
// @DOC_FIELD_FORMAT   Q5.19 format
.CONST $M.agc400.Parameter.OFFSET_ATTACK_SPEED_FIELD      10*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   hysterisis threshold1 for agc gain adaptation
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_ADAPT_THRESHOLD1_FIELD  11*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   hysterisis threshold2 for agc gain adaptation
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.Parameter.OFFSET_ADAPT_THRESHOLD2_FIELD  12*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   min AGC gain (PARAMETER)
// @DOC_FIELD_FORMAT   Q7.17 format
.CONST $M.agc400.Parameter.OFFSET_G_MIN_FIELD             13*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   Structure Size
// @DOC_FIELD_FORMAT   Integer
.CONST $M.agc400.Parameter.STRUC_SIZE                     14;

// @END  DATA_OBJECT PARAMETERS

// @DATA_OBJECT  AGCDATAOBJECT

// @DOC_FIELD_TEXT   Pointer to Parameters                             @DOC_LINK         @DATA_OBJECT  PARAMETERS
// @DOC_FIELD_FORMAT   Pointer
.CONST $M.agc400.OFFSET_PARAM_PTR_FIELD				 0*ADDR_PER_WORD;               
// @DOC_FIELD_TEXT   Pointer to input stream (new frame)  
// @DOC_FIELD_FORMAT   Pointer
.CONST $M.agc400.OFFSET_PTR_INPUT_FIELD              1*ADDR_PER_WORD;	
// @DOC_FIELD_TEXT   Pointer to output stream (new frame)  
// @DOC_FIELD_FORMAT   Pointer
.CONST $M.agc400.OFFSET_PTR_OUTPUT_FIELD             2*ADDR_PER_WORD;		
// @DOC_FIELD_TEXT  Pointer to the value of Voice Activity Detector (VAD)  . Value = 1 for 'voice activity detected'  Value = 0 for 'no voice activity detected'
// @DOC_FIELD_FORMAT   Pointer
.CONST $M.agc400.OFFSET_PTR_VAD_VALUE_FIELD          3*ADDR_PER_WORD;     
// @DOC_FIELD_TEXT   Frame threhold for AGC gain update
// @DOC_FIELD_FORMAT   Integer 
.CONST $M.agc400.OFFSET_FRAME_THRESHOLD_FIELD       4*ADDR_PER_WORD;	
// @DOC_FIELD_TEXT   hard limiter in the agc
// @DOC_FIELD_FORMAT   Integer
.CONST $M.agc400.OFFSET_HARD_LIMIT_FIELD             5*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   pointer to tone flag to freeze adaptation
// @DOC_FIELD_FORMAT   Pointer
.CONST $M.agc400.OFFSET_PTR_TONE_FLAG_FIELD          6*ADDR_PER_WORD;	 
// @DOC_FIELD_TEXT   running average of abs(input stream)
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.OFFSET_INPUT_LEVEL_FIELD            7*ADDR_PER_WORD;   
// @DOC_FIELD_TEXT   minimum for the running average of abs(input stream)
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.OFFSET_INPUT_LEVEL_MIN_FIELD        8*ADDR_PER_WORD;      
// @DOC_FIELD_TEXT   1 - TC for smoothing AGC gain when increasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.OFFSET_ONE_M_ATTACK_TC_FIELD        9*ADDR_PER_WORD;      
// @DOC_FIELD_TEXT   1 - TC for smoothing AGC gain when decreasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.OFFSET_ONE_M_DECAY_TC_FIELD        10*ADDR_PER_WORD;    
// @DOC_FIELD_TEXT   1 - TC for input level adjustment when increasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.OFFSET_ONE_M_ALPHA_A_90_FIELD      11*ADDR_PER_WORD;   
// @DOC_FIELD_TEXT   TC for input level adjustment when decreasing
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.OFFSET_ONE_M_ALPHA_D_90_FIELD      12*ADDR_PER_WORD;   
// @DOC_FIELD_TEXT   min AGC gain (PARAMETER) 
// @DOC_FIELD_FORMAT   Q7.17 format
.CONST $M.agc400.OFFSET_G_MIN_FIELD                 13*ADDR_PER_WORD;   
// @DOC_FIELD_TEXT   estimated AGC gain value
// @DOC_FIELD_FORMAT   Q7.17 format
.CONST $M.agc400.OFFSET_G_FIELD                     14*ADDR_PER_WORD;   
// @DOC_FIELD_TEXT   applied AGC gain value
// @DOC_FIELD_FORMAT   Q7.17 format
.CONST $M.agc400.OFFSET_G_REAL_FIELD                15*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   1 - in/out compression ratio applied when exceeding AGC TARGET
// @DOC_FIELD_FORMAT   Q1.23 format
.CONST $M.agc400.OFFSET_ONE_M_COMP_FIELD            16*ADDR_PER_WORD;
// @DOC_FIELD_TEXT   power estimation scale factor -0.5*log2(2/frame_size) 
// @DOC_FIELD_FORMAT   Q8.16 format
.CONST $M.agc400.OFFSET_PWR_SCALE_FIELD             17*ADDR_PER_WORD;	
// @DOC_FIELD_TEXT   Frame counter before AGC gain update
// @DOC_FIELD_FORMAT   Integer 
.CONST $M.agc400.OFFSET_FRAME_COUNTER_FIELD         18*ADDR_PER_WORD;
// @DOC_FIELD_TEXT     last persisted gain 
// @DOC_FIELD_FORMAT   Q7.17 format
.CONST $M.agc400.OFFSET_PERSISTED_GAIN_FIELD        19*ADDR_PER_WORD;
// @DOC_FIELD_TEXT  Control word for AGC - Bypass
// @DOC_FIELD_FORMAT  Bit Flag 
.CONST $M.agc400.OFFSET_SYS_CON_WORD_FIELD          20*ADDR_PER_WORD;    
// @DOC_FIELD_TEXT   Mask for Control Word
// @DOC_FIELD_FORMAT   Bit Mask
.CONST $M.agc400.OFFSET_BYPASS_BIT_MASK_FIELD       21*ADDR_PER_WORD;	
// @DOC_FIELD_TEXT     bitmask for agc persist feature
// @DOC_FIELD_FORMAT   Bit Mask
.CONST $M.agc400.OFFSET_BYPASS_PERSIST_FIELD        22*ADDR_PER_WORD;

// @DOC_FIELD_TEXT   Structure Size   
// @DOC_FIELD_FORMAT   Integer                
.CONST $agc400.STRUC_SIZE                           23;

// @END  DATA_OBJECT AGCDATAOBJECT
#endif   // AGC400_LIB_H


