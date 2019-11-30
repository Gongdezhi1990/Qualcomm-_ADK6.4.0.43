// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef  SOLA_LIB_H
#define  SOLA_LIB_H

//---------------------------------------------------------------------------
//  CONSTANTS
//---------------------------------------------------------------------------
// @DOC_FIELD_TEXT    Maximum buffer threshold
// @DOC_FIELD_FORMAT 
.CONST   $sola100.T_Th_H                                     1.8;
// @DOC_FIELD_TEXT    Minimum buffer threshold
// @DOC_FIELD_FORMAT   
.CONST   $sola100.T_Th_L			                               0.01;
// @DOC_FIELD_TEXT    Hang time for the VAD
// @DOC_FIELD_FORMAT 
.CONST   $sola100.VAD_Th_HANG 	  	 	                       27;
// @DOC_FIELD_TEXT    Burst time for the VAD
// @DOC_FIELD_FORMAT 
.CONST   $sola100.VAD_Th_BURST		  	                       7;


//---------------------------------------------------------------------------
// TSM PARAMETERS
//---------------------------------------------------------------------------
// @DOC_FIELD_TEXT    Expansion rate (always greater than one)
// @DOC_FIELD_FORMAT  Q2.22 format
.CONST   $sola100.PARAMETERS.OFFSET_ALFA                            0;
// @DOC_FIELD_TEXT    Fast-Forward rate
// @DOC_FIELD_FORMAT  
.CONST   $sola100.PARAMETERS.OFFSET_BETA                            1;
// @DOC_FIELD_TEXT    Parameter structure size
// @DOC_FIELD_FORMAT
.CONST   $sola100.PARAMETERS.STRUC_SIZE   		  	                  2;



//---------------------------------------------------------------------------
// @DATA_OBJECT SOLA100_DATAOBJECT
//---------------------------------------------------------------------------
// @DOC_FIELD_TEXT Pointer to TSM parameter array
// @DOC_FIELD_FORMAT Pointer
.CONST $sola100.OFFSET_PARAM_PTR_FIELD                              0;
// @DOC_FIELD_TEXT Pointer to input frame
// @DOC_FIELD_FORMAT Pointer
.CONST $sola100.OFFSET_PTR_INPUT_FIELD                              1;
// @DOC_FIELD_TEXT Pointer to output frame 
// @DOC_FIELD_FORMAT Pointer
.CONST $sola100.OFFSET_PTR_OUTPUT_FIELD                             2;
// @DOC_FIELD_TEXT    Window length
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_WINDOW_LENGTH                              3;
// @DOC_FIELD_TEXT  Decimation of each xcorr
// @DOC_FIELD_FORMAT   
.CONST   $sola100.OFFSET_X_DECIM                                    4;
// @DOC_FIELD_TEXT  Decimation of the skew axis sampling
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_K_DECIM                                    5;
// @DOC_FIELD_TEXT    Pointer to Cross-Fade window
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_PTR_CROSS_FADE_WIN                         6;        
// @DOC_FIELD_TEXT  Pointer to TSM buffer
// @DOC_FIELD_FORMAT Pointer
.CONST $sola100.OFFSET_TSM_BUF_PTR_FIELD                            7;
// @DOC_FIELD_TEXT   TSM buffer size
// @DOC_FIELD_FORMAT Integer
.CONST $sola100.OFFSET_TSM_BUF_SIZE_FIELD                           8;            // 16360 for NB and 32720 for WB
// @DOC_FIELD_TEXT Pointer to scratch memory address 
// @DOC_FIELD_FORMAT Pointer
.CONST $sola100.OFFSET_SCRATCH_ADDR_FIELD                           9;

//Internal Fields

// @DOC_FIELD_TEXT    Overlap Segment Length
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_W_OV                                       10;
// @DOC_FIELD_TEXT    Maximum shift interval on the analysis window
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_K_MAX                                      11;
// @DOC_FIELD_TEXT    Similarity point length of output
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_W_SIM                                      12;
// @DOC_FIELD_TEXT    Synthesis Window Size
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_SYNTH_WIN_SIZE                             13;
// @DOC_FIELD_TEXT    Synthesis Window Size
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_ANALYSIS_WIN_SIZE                          14;
// @DOC_FIELD_TEXT    Output buffer 1
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_OBUF1                                      14;                         
// @DOC_FIELD_TEXT    Output buffer 2
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_OBUF2                                      15;                         
// @DOC_FIELD_TEXT    Y buffer position
// @DOC_FIELD_FORMAT 
.CONST   $sola100.OFFSET_DELAY                                      16;        
// @DOC_FIELD_TEXT    
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_RXP                                        17;      
// @DOC_FIELD_TEXT    
// @DOC_FIELD_FORMAT  
.CONST   $sola100.OFFSET_XP                                         18; 
// @DOC_FIELD_TEXT Data Object structure size
// @DOC_FIELD_FORMAT  Integer
.CONST   $sola100.STRUC_SIZE                                        19;

.CONST   $sola100.TSM_BUFFER_READ_PTR                               20;

.CONST   $sola100.TSM_BUFFER_WRITE_PTR                              21;
// @END  DATA_OBJECT SOLA100_DATAOBJECT
   
#endif   //_SOLA_LIB_H
