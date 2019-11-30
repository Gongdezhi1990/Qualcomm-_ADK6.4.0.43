// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef _AUDIO_MUX_NxM_LIB_H
#define _AUDIO_MUX_NxM_LIB_H


#include "portability_macros.h"



// Include the modes here
.CONST $M.Audio_Mux_NxM.MODE.sine         0;
.CONST $M.Audio_Mux_NxM.MODE.sqwave       1;
.CONST $M.Audio_Mux_NxM.MODE.Wnoise       2; 
.CONST $M.Audio_Mux_NxM.MODE.Triang       3; 
.CONST $M.Audio_Mux_NxM.MODE.sawtooth     4;
.CONST $M.Audio_Mux_NxM.MODE.sine_sweep   5;


// stream mixer
.CONST 		$M.Audio_Mux_NxM_Obj.STREAM_TABLE_FIELD				0*ADDR_PER_WORD;	// ptr to I/O STREAM TABLE size: (num_inputs + num_outputs)
.CONST      $M.Audio_Mux_NxM_Obj.NUM_INPUTS_FIELD         		1*ADDR_PER_WORD;    // number of inputs, used only for setup validation
.CONST      $M.Audio_Mux_NxM_Obj.NUM_OUTPUTS_FIELD         		2*ADDR_PER_WORD;    // number of outputs
.CONST      $M.Audio_Mux_NxM_Obj.NUM_SIGNALS_FIELD         		3*ADDR_PER_WORD;    // number of internal function generators, used only for setup validation
.CONST      $M.Audio_Mux_NxM_Obj.PTR_OUT_CHAN_EXP_FIELD         4*ADDR_PER_WORD;    // ptr to output channel exponent table of size: num_outputs
.CONST      $M.Audio_Mux_NxM_Obj.PTR_OUT_CHAN_MASK_FIELD        5*ADDR_PER_WORD;	// ptr to input channel mask table of size: num_outputs
.CONST      $M.Audio_Mux_NxM_Obj.PTR_IN_CHAN_MANT_FIELD         6*ADDR_PER_WORD;    // ptr to input channel weigthing mantissa table of size: num_outputs*(num_inputs + num_signals)
.CONST      $M.Audio_Mux_NxM_Obj.PTR_FUNC_MODE_FIELD            7*ADDR_PER_WORD;    // ptr to function generator mode table of size: num_outputs + num_signals
.CONST      $M.Audio_Mux_NxM_Obj.PTR_FUNC_PARAM_FIELD           8*ADDR_PER_WORD;    // ptr to function generator param table of size: num_outputs + num_signals
.CONST      $M.Audio_Mux_NxM_Obj.PTR_FUNC_SCALE_FIELD           9*ADDR_PER_WORD;    // ptr to function generator scale table of size: num_outputs + num_signals 
.CONST      $M.Audio_Mux_NxM_Obj.MEM_FUNC_HIST_FIELD           10*ADDR_PER_WORD;	// ptr to function generator history data memory of size: (NUM_INPUTS+NUM_SIGNALS)*2
.CONST      $M.Audio_Mux_NxM_Obj.MEM_CHAN_MANT_FIELD           11*ADDR_PER_WORD;    // ptr to input channel weighting table of size: (NUM_INPUTS+NUM_SIGNALS)*NUM_OUTPUTS
.CONST      $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_BUFF_FIELD      12*ADDR_PER_WORD;    // scratch mem for i/o stream buffers, size: (NUM_INPUTS+NUM_OUTPUTS)*$M.Audio_Mux_NxM_Obj.ENTRY_SIZE
.CONST      $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_DATA_FIELD      13*ADDR_PER_WORD;  	// scratch mem to cache stream data of size: (NUM_INPUTS+NUM_SIGNALS)* framesize (should be derived from stream directly)
.CONST      $M.Audio_Mux_NxM_Obj.PTR_INIT_FUNC_FIELD      	   14*ADDR_PER_WORD;    // init function pointer 
.CONST      $M.Audio_Mux_NxM_Obj.PTR_PROC_FUNC_FIELD      	   15*ADDR_PER_WORD;    // proc function pointer
.CONST      $M.Audio_Mux_NxM_Obj.STRUC_SIZE       			   16;  




#if 	defined(MUX_IN_14)	// 12+2
// defined for 12 input streams and 2 internal signals 
.CONST $M.Audio_Mux_NxM.num_signals 2;
.CONST $M.Audio_Mux_NxM.num_inputs  12;
.CONST $M.Audio_Mux_NxM.mask_inputs 32766;  // 0x7FFE 
.CONST $M.Audio_Mux_NxM.signals_bit_offset ($M.Audio_Mux_NxM.num_inputs);
.CONST $M.Audio_Mux_NxM.mask_signals 6;  // 1 << ($M.Audio_Mux_NxM.num_signals+1) - 2
#elif   defined(MUX_IN_12)	// 10+2
// defined for 12 input streams and 2 internal signals 
.CONST $M.Audio_Mux_NxM.num_signals 2;
.CONST $M.Audio_Mux_NxM.num_inputs  10;
.CONST $M.Audio_Mux_NxM.mask_inputs 8190;  // 0x1FFE 
.CONST $M.Audio_Mux_NxM.signals_bit_offset ($M.Audio_Mux_NxM.num_inputs);
.CONST $M.Audio_Mux_NxM.mask_signals 6;  // 1 << ($M.Audio_Mux_NxM.num_signals+1) - 2
#else 	// 8 + 0
.CONST $M.Audio_Mux_NxM.num_signals 0;
.CONST $M.Audio_Mux_NxM.num_inputs  8;
.CONST $M.Audio_Mux_NxM.mask_inputs 510;  // 0x1FE 
.CONST $M.Audio_Mux_NxM.signals_bit_offset ($M.Audio_Mux_NxM.num_inputs);
.CONST $M.Audio_Mux_NxM.mask_signals 0;  // 1 << ($M.Audio_Mux_NxM.num_signals+1) - 2
#endif



// Macros for structure size and position
#ifdef     BASE_REGISTER_MODE
.CONST     $M.Audio_Mux_NxM_Obj.ENTRY_SIZE                             3*ADDR_PER_WORD;
#else
.CONST     $M.Audio_Mux_NxM_Obj.ENTRY_SIZE                             2*ADDR_PER_WORD;
#endif

#endif

