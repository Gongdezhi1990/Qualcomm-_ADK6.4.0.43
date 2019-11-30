// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "audio_mux_NxM.h"
#include "stack.h"




.PUBLIC  $M.Audio_Mux_NxM.Initialize.func;
.PUBLIC  $M.Audio_Mux_NxM.Process.func;
.PUBLIC  $M.Audio_Mux_NxM.method.Initialize;
.PUBLIC  $M.Audio_Mux_NxM.method.Process;





// *****************************************************************************
// MODULE:
//    $M.Audio_Mux_NxM.method
//
// DESCRIPTION:
//   This is the wrapper function for specific initialize and proc      
// MODIFICATIONS:
//
//
// INPUTS:
//       - r7 - address of audio_mux_NxM data object.  
//
// OUTPUTS:

// TRASHED:
//      assume everything
//
// *****************************************************************************
.MODULE $M.Audio_Mux_NxM.method; 
   .codesegment AUDIO_PROC_STREAM_MIXER_NXM_INITIALIZE_PM;
   .DATASEGMENT DM;

   Initialize:
   $push_rLink_macro;   
		r0 = M[r7 + $M.Audio_Mux_NxM_Obj.PTR_INIT_FUNC_FIELD];
		call r0;
   $pop_rLink_and_rts_macro;   

   Process:
   $push_rLink_macro;   
		r0 = M[r7 + $M.Audio_Mux_NxM_Obj.PTR_PROC_FUNC_FIELD];
		call r0;
   $pop_rLink_and_rts_macro;   
.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $M.Audio_Mux_NxM.Initialize.func
//
// DESCRIPTION:
//   Initialization function should include the following: 
//   Function gen init: fill function history buffer 
//   Input channel mantissa init: fill input channel mantissa cache according to channel mask
//     
// MODIFICATIONS:
//
//
// INPUTS:
//       - r7 - address of audio_mux_NxM data object.  
//
// OUTPUTS:
//    - NONE.
//
// TRASHED:
//      r0~r1, r5~r6, r10, I0, I1, I7, M0, M1
//
// using the func_mode ptr in CurParams to init buff_1 and buff_2 for function generator cache 
// initialize func history table
// input: func mode ptr 
// output:  func hist1 and func hist2 
//  I1 function mode
//  I0 hist 1
//  I7 hist 2
//
// Inititlize the mantissa_X_mask table
// input:   mant ptr in CurParams,   mask ptr in CurParams
// output:  mantissa cache
//   I0 = PTR_IN_CHAN_MANT_FIELD
//   I1 = PTR_OUT_CHAN_MASK_FIELD
//   I7 = MEM_CHAN_MANT_FIELD

// *****************************************************************************

.MODULE $M.Audio_Mux_NxM.Initialize; 
   .codesegment AUDIO_PROC_STREAM_MIXER_NXM_INITIALIZE_PM;
   .DATASEGMENT DM;

func:


   M0 = -1; 
   M1 = MK1;

   push FP;
   
   push r7;
   pop FP;
   
   
#ifdef DEBUG_ON     // channel setup verification
    r0 = M[FP + $M.Audio_Mux_NxM_Obj.NUM_INPUTS_FIELD]; 
    Null = r0 - $M.Audio_Mux_NxM.num_inputs;
    if NZ call $error;
    r0 = M[FP + $M.Audio_Mux_NxM_Obj.NUM_SIGNALS_FIELD]; 
    Null = r0 - $M.Audio_Mux_NxM.num_signals;
    if NZ call $error;
#endif



   // load function parameters
   r6 = M[FP + $M.Audio_Mux_NxM_Obj.NUM_OUTPUTS_FIELD];
   r10 = r6 + $M.Audio_Mux_NxM.num_signals;  
   Words2Addr(r10);
   I0 = M[FP + $M.Audio_Mux_NxM_Obj.MEM_FUNC_HIST_FIELD];     // hist 1
   I7 = I0 + r10;		   // hist 2
   Addr2Words(r10); 
   I1 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_FUNC_MODE_FIELD];

   do lp_init;
      r1 = M0, 	r0 = M[I1, M1];      
      Null = r0 - $M.Audio_Mux_NxM.MODE.sawtooth;  
      if NEG r1 = r1 - r1;
      M[I0, M1] = r1,  M[I7, M1] = r1; 
lp_init:

    // initialize the mantissa_X_mask table
   I0 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_IN_CHAN_MANT_FIELD];
   I1 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_OUT_CHAN_MASK_FIELD];
   I7 = M[FP + $M.Audio_Mux_NxM_Obj.MEM_CHAN_MANT_FIELD];
	// r6 = num_outputs 
	// for every output stream, cache its input matissa according to mask value
    //TBD: to be optimized later
   r1 = -1;  
lp_output:  // for i=0:num_outputs-1
   r10 = $M.Audio_Mux_NxM.num_inputs + $M.Audio_Mux_NxM.num_signals;
   r5 = M[I1, M1];    // load mask[i]
   do lp_input;  // for j=0:num_inputs-1
      r5 = r5 LSHIFT r1,   r0 = M[I0, M1];    // load mantissa 
      Null = r5 and 1;    // is the input channel masked
      if NZ r0 = r0 - r0;
      M[I7, M1] = r0; // store and load next mantissa at the  same time
   lp_input:
   r6 = r6 - 1;  
   if NZ jump lp_output;
    
	pop FP;
	
    rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $M.Audio_Mux_NxM.Process.func 
//
// DESCRIPTION:
//
//    Weighted Mix of N inputs into M outputs with optional Function Generator
//
// INPUTS:
//    - r7 = address of the DM1 data object
//    - r8 = address of the DM2 data objet
// OUTPUTS:

// TRASHED:
//    - everything
//
//     PMEMORY:  
//
// CPU USAGE ESTIMATE: stall not included
//    general overhead: 44 
//    stream buffer loop: (n+m)*10
//    func genc loop:  (f+m)*( 22 + b*func_gen_cycles )
//    input cache loop:   n * (8 + b)
//    mixer loop:         m * (18 + b*(n+f+3))
//    where:
//      b = blocksize (# samples to process)
//      n = num_inputs
//      m = num_outptus
//      f = num_internal_functions
//
//   full load: b=96,n=12,m=10,f=2, all funcs masked (~12MIPS@48k)
//   bypassed output saves ~0.8MIPS per channel.
//    
//  init I/O stream buffer ptr
//  input: I/O streams table
//  output: cache to store stream ptr
//  scratch memory required by ever frame processing.
//	I7 stream table
//  I1 stream buff table
//
//  func_gen for output streams and internal streams.
//  input:  mask ptr, func parameter ptrs, 
//  output:  output stream.
//   I3: hist 1 							
//   I4: hist 2 
//   I7: channel mask
//   I2: mode
//   I6: param   
//   I5: scale
//   I0: offset to output stream buffer table
//   I1: I/O stream buffer table 

//  input streams cache
//  input: input stream cache  
//  output: input data cache
//  I2: input stream BUFF
//  I5: input stream data  
//
//
//  mixer
//  input: channel mask ptr/channel exp ptr from CurParams,  mantissa cache, input cache 
// output: output buffer derived from ouput stream table. 
// I1: output buffer pointer from I0
// I2: output stream table 
// I3: mantissa x mask cache 
// I4: input cache
// I6: chan exp table ptr 
// I7: out mask table pointer
// r6: framesize
// r8: framesize
// r9: num_outputs
// *****************************************************************************
.MODULE $M.Audio_Mux_NxM.Process; 
   .codesegment AUDIO_PROC_STREAM_MIXER_NXM_PROCESS_PM;
   .DATASEGMENT DM;
   
   .VAR  ModeTable[] =
      sin_func,
      sqwave_func,
      Wnoise_func,   
      Triang_func,   
      sawtooth_func,
      sine_sweep_func;

func:

   $push_rLink_macro;

    push FP;
	
	push r7;
	pop  FP;

#ifdef DEBUG_ON
    r0 = M[FP + $M.Audio_Mux_NxM_Obj.NUM_INPUTS_FIELD]; 
    Null = r0 - $M.Audio_Mux_NxM.num_inputs;
    if NZ call $error;
    r0 = M[FP + $M.Audio_Mux_NxM_Obj.NUM_SIGNALS_FIELD]; 
    Null = r0 - $M.Audio_Mux_NxM.num_signals;
    if NZ call $error;
#endif




// ****************************************************************************
// function generator
// start with stream setup, output streams used as temporary storage for each function


   // setup M registers
   M0 = 0;
   M1 = MK1;


// SP.  Cludge translate to frm buffers
   // function generator
   r0 = M[FP + $M.Audio_Mux_NxM_Obj.NUM_OUTPUTS_FIELD];
   Words2Addr(r0);  
   M3 = r0;
   
   
   
   // it still take streams from CurrParams and setup the system
   
   // extract the stream buffer address and size for I/O streams
   // $M.Audio_Mux_NxM.num_outputs + $M.Audio_Mux_NxM.num_inputs;
   r10 = M3 + $M.Audio_Mux_NxM.num_inputs; 
   I7  = M[FP + $M.Audio_Mux_NxM_Obj.STREAM_TABLE_FIELD];
   I1  = M[FP + $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_BUFF_FIELD]; 
   do lp_extract_frmbuffers;
        r0 = M[I7,MK1];    // input/output frmbuffer   
        
#ifndef BASE_REGISTER_MODE
        call $frmbuffer.get_buffer;
//    - r0 = buffer address
//    - r1 = buffer size
//    - r2 = buffer start address   <base address variant>
//    - r3 = frame size
        M[I1,MK1] = r0;   // Buffer Address
        M[I1,MK1] = r1;   // Buffer Length

#else
        call $frmbuffer.get_buffer_with_start_address;
//    - r0 = buffer address
//    - r1 = buffer size
//    - r2 = buffer start address   <base address variant>
//    - r3 = frame size
        M[I1,MK1] = r0;   // Buffer Address
        M[I1,MK1] = r1;   // Buffer Length
        M[I1,MK1] = r2;   // BASE ADDRESS
        
#endif

   lp_extract_frmbuffers: 


   r6 = r3;		// Set frame size from input/output 

   
   // based on channel mask field in CurParams, compute function wave for each output. 
   // I0: offset to output stream buffers 
   // I1: output buffer pointer from I0
   // I2: function mode table
   // I3: hist 1 phase table pointer 
   // I4: hist 2 parameter pointer 
   // I5: scale info
   // I6: param, mainly phase 
   // I7: out mask table pointer
  
   I0 = M[FP + $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_BUFF_FIELD];  				   	// I0 points to stream buff table
   I0 = I0 + $M.Audio_Mux_NxM.num_inputs * $M.Audio_Mux_NxM_Obj.ENTRY_SIZE;        	//advance to ourput stream buff 
   I3 = M[FP + $M.Audio_Mux_NxM_Obj.MEM_FUNC_HIST_FIELD]; 							// point to hist 1 table
    // M3 used as loop count should include signal generators
    // M3 as loop count use stepsize  ADDR_PER_WORD in ARCH4
   M3 = M3 + $M.Audio_Mux_NxM.num_signals*ADDR_PER_WORD;							// num of FUNC GEN  =  NUM_OUTPUTS + NUM_SIGNALS
   I4 = I3 + M3; 												// I4 points to hist2 table 
   
   I7 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_OUT_CHAN_MASK_FIELD];  	// I7 points to mask table
   I2 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_FUNC_MODE_FIELD]; 		// I2 points to func mode table
   I6 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_FUNC_PARAM_FIELD]; 		// I6 points to func param table  
   I5 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_FUNC_SCALE_FIELD]; 		// I5 points to func scale table

	// used to compute mask
	r9 = $M.Audio_Mux_NxM.mask_signals;
   
lp_funcgen: // for i=0 to numoutputs-1

   // check if it's internal signal, if so use cache as output buffer
   Null = M3 - ($M.Audio_Mux_NxM.num_signals*ADDR_PER_WORD);	// internal signals?
   if GT  jump  output_stream_setup;
	// internal_stream_setup:  
   I1 = M[FP + $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_DATA_FIELD];
   r3 = ADDR_PER_WORD * ($M.Audio_Mux_NxM.num_signals + $M.Audio_Mux_NxM.num_inputs) - M3;   // (num_inputs + num_signals - remaining stream count)*ADDR_PER_WORD
   r3 = r3 * r6 (int), r0 = M[I3,M0], r4 = M[I4,M0]; 	// (num_inputs + num_signals - remaining stream count)*frame_size*ADDR_PER_WORD
   I1 = I1 + r3,    r2 = M[I6, M1];            	// r8 = param[i] 	   // advance to internal stream cache
   L1 = Null;									// for internal signal buffer, use linear addressing

   r3 = r3 - r3;  			   				   	// internal signal always enabled first
   // bypass internal stream generation if masked 
	// is internal stream masked?
   r9 = r9 LSHIFT -1;	
   Null = r9 and 1;
   // not masked, continue 
   if Z jump continue_func_gen;
   // masked, zero out cache directly 
   r10 = r10 + r6;
   push I0; //matched the pop I0 used
    do lp_cache_reset;
        M[I1,MK1] = r3;
lp_cache_reset:
    nop;

	jump func_complete;
  
   
continue_func_gen:   
   jump  func_gen_start;
   
output_stream_setup:

   // setup I1/L1 for output[i]
   r1 = M[I0, M1], r4 = M[I4,M0]; // r10=ct, r1=output addr[i], r4 = hist2[i], stall
   I1 = r1, r1 = M[I0, M1];
   
#ifndef BASE_REGISTER_MODE   
   L1 = r1, r0 = M[I3,M0];    // l1 = buflen[i], r0=hist1[i] 
   r3 = M[I7, M1];            // r3 = mask
   r2 = M[I6, M1];            // r8 = param[i]
#else
   L1 = r1,  r1 = M[I0, M1];    
   push r1,  r3 = M[I7, M1],  r0 = M[I3,M0]; 
   pop  B1,  r2 = M[I6, M1];
#endif


	// internal stream masked by all outputs?
	r1 = r3 LSHIFT -$M.Audio_Mux_NxM.signals_bit_offset;
	r9 = r9 AND r1;


func_gen_start:


   r8 = r2;
   r10 = r10 + r6, r1 = M[I2,M1], r5 = M[I5, M1];  // r1=mode[i], r5 = scale[i]. free ins slot. stall
   Words2Addr(r1);
   r1 = M[ModeTable + r1];             

   push I0;
   
   Null = r3 AND 1;
   if NZ jump no_func;

   jump r1;

sin_func:                              
   // Sinusoidal wave function
   // r0 has current_phase
   // r4 = -1.0 (na)
   do tones_loop;
      // result in r1   ; modifies r1,r2,r3,rMAC, & I0
      call $math.sin;                          
      // Scale factor in r5
      rMAC = r1 * r5;                       
      // Store the result to output; r8 has Phase_step_size
      r0 = r0 + r8, M[I1,M1] = rMAC;     
tones_loop:
   // r0=current_phase updated
   jump func_complete;

sqwave_func:                           
   // Square wave function
   // r0 has current_step
   // r4 = -1.0 (na)
   do sq_loop;          
      // Put +1 or -1
      r1 = r0 ASHIFT DAWTH;                  
      // Scale factor in r5
      rMAC = r1 * r5;                       
      // Store output; r8 has step_size      
      r0 = r0 + r8, M[I1,M1] = rMAC;      
sq_loop:
   // r0 = Update current step
   jump func_complete;
 
sawtooth_func:                         
   // Sawtooth wave
   // r0 has current_step
   // r4 = -1.0 (na)
   do saw_loop;
      // r5 has scale factor; 
      rMAC = r0 * r5;                     
      // r8 has stepsize; Store output
      r0 = r0 - r8, M[I1,M1] = rMAC;    
saw_loop:
   // r0 = Update current step
   jump func_complete;
   
Triang_func:                           
   // Triangular wave function
   // r0 has Initial step size   
   // r4 = -1.0
   r4 = -1.0;
   do triang_loop;
      r1 = r0; 
      if NEG r1 = r1 * r4 (frac);   
      r1 = r1 - 0.5;
      r1 = r1 ASHIFT 1;
      // r5 has scale factor
      rMAC = r1 * r5;                   
      // r8 has step_size; Store output 
      r0 = r0 + r8, M[I1,M1] = rMAC;     
triang_loop:
   // r0 = Update current step size
   jump func_complete;

Wnoise_func:                           
   // White Noise
   do wn_loop;
      // r0 = ouput. 5 cycles
      call $math.rand;                
      // Scale factor in r5
      rMAC = r0 * r5;                       
      // Store output
      M[I1,M1] = rMAC;                    
wn_loop:
   jump func_complete;
   
sine_sweep_func:                       
   // Sine_sweep wave
   // r0 records freqs reqd   
   // r4 has current step for Particular freq
   do tones_sweep_loop;
      // result in r1   , 22 cycles
      call $math.sin;                       
      // r5 has scale factor, Mix
      rMAC = r1 * r5;                    
      // r4 has step size; Store output
      r0 = r0 + r4, M[I1,M1] = rMAC;  
      // r8 has the step size for the next freq
      r4 = r4 + r8;                    
tones_sweep_loop:
   // r0 = current_phase updated
   // r4 = current step updated 

   jump func_complete;   

no_func:    // no function, just clear r10 for next one
   r10 = 0;
   
func_complete:

   pop I0;

   // decrement channel counter, update history1/2
   M3 = M3 - M1, M[I3,M1] = r0, M[I4,M1] = r4;     // stall
   if NZ jump lp_funcgen;

   // no need to reset L1 = 0, B1 = 0, setup again in channel mixer

    // figure out how to define the cache, since the signal_generator has been cached already



// ****************************************************************************
// Channel mixer

   // mixer run for the entire frame and stored the result in output stream buffer.

   // some parameters setup for input cache
   // I1: input buffer pointer derived from I2
   // I2: input stream buff table
   // I5: input cache 
   // r6: num_iputs
   // r8: framesize


   // cache inputs
   r8 = r6; // r8 = frame size
   I2 = M[FP + $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_BUFF_FIELD];	// I2 = ptr input channel table
   I5 = M[FP + $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_DATA_FIELD]; 
   r6 = $M.Audio_Mux_NxM.num_inputs;
   
   lp_cache_inputs:
      // Load I0/L0 with output_addr[i]
      r10 = r8, r0 = M[I2, M1];
      I1 = r0, r0 = M[I2, M1];               // I1/L1 points to input[i]

#ifndef  BASE_REGISTER_MODE      
      L1 = r0;                               // free ag1 slot
#else
      L1 = r0,   r0 = M[I2, M1];             // free ag1 slot for base address
      push r0;
      pop  B1;                               
#endif

      r0 = M[I1, M1];
      do lp_cache_inputs_sample;
         M[I5, M1] = r0, r0 = M[I1, M1];
      lp_cache_inputs_sample:
      r6 = r6 - 1;
   if NZ jump lp_cache_inputs;
   
   L1 = 0;  
#ifdef  BASE_REGISTER_MODE      
   push 0; 
   pop B1;
#endif


   // I1: output buffer pointer from I0
   // I2: output stream table 
   // I3: mantissa x mask cache 
   // I4: input cache
   // I6: chan exp table ptr 
   // I7: out mask table pointer
   // r6: framesize
   // r8: framesize
   // r9: num_outputs




   // Channel mixer

   r0 = ADDR_PER_WORD * (1 - ($M.Audio_Mux_NxM.num_inputs + $M.Audio_Mux_NxM.num_signals));  // M2 = -(num_inputs + num_signals - 1);
   r1 = r8 * r0 (int);		// - r8 * ($M.Audio_Mux_NxM.num_inputs + $M.Audio_Mux_NxM.num_signals -1) (int);
   M0 = r1 + ADDR_PER_WORD;
   M2 = r0;

   Words2Addr(r8);
   M3 = r8;
   Addr2Words(r8);


   I6 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_OUT_CHAN_EXP_FIELD];      // I6 points to exponent table
   I3 = M[FP + $M.Audio_Mux_NxM_Obj.MEM_CHAN_MANT_FIELD]; 	// I3 points to mantissaXenable table
   // I2 already point to output channel table after input cache process 
   I7 = M[FP + $M.Audio_Mux_NxM_Obj.PTR_OUT_CHAN_MASK_FIELD];  // setup for indexing in data obj

   // for i=0 to num_outputs-1
   r9 = M[FP + $M.Audio_Mux_NxM_Obj.NUM_OUTPUTS_FIELD];
   lp_output:  
   
      // setup for sample loop:
      r10 = r8;
      I4 = M[FP + $M.Audio_Mux_NxM_Obj.CACHE_STREAMS_DATA_FIELD]; // I4 points to input cache
      // Load I1/L1 with output_addr[i]
      r0 = M[I2, M1];                                
      I1 = r0, r0 = M[I2, M1];   
      
#ifndef BASE_REGISTER_MODE      
      L1 = r0;              // I0,I1/L0,L1 points to output[i]
      r4 = M[I6, M1];       // get first exponent
      r2 = M[I3, M1], r1 = M[I4, M3];        // get first mantissa, input
#else
      L1 = r0, r0 = M[I2, M1];              
      push r0, r4 = M[I6, M1];                  
      pop  B1, r2 = M[I3, M1], r1 = M[I4, M3];        
#endif

      rMAC = rMAC - rMAC;

      r0 = M[I7, MK1];
      

    // insert bypass logic here
      r5 = r0 AND $M.Audio_Mux_NxM.mask_inputs;
    Null = r5  -  $M.Audio_Mux_NxM.mask_inputs;
    // if not all inputs are masked
    if  NZ   jump  start_mixer; 
    // all inputs are masked, so check fungen
    Null = r0 AND  1;   
    if Z  jump  jp_done_lp_sample;  // only function, done in lp_funcgen
    do lp_mem_reset;
        M[I1,MK1] = rMAC;
lp_mem_reset:
    nop;
    jump   jp_done_lp_sample;

start_mixer:


      r0 = r0 AND 1;
      if NZ jump jp_nofunc;

      do lp_sample;                          // for j=0 to blksize-1
         // for k=0 to num_inputs-1       
         //    acc += input[k] * mantissa[j*8+k] * enabled[j*8+k] (x8)
         rMAC = M[I1, 0]; // acc=first output (from funct gen) // stall
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input2 
#if    defined(MUX_IN_14)         
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input3
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input4
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input5
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input6
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input7
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input8 
#elif  defined(MUX_IN_12)
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input3
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input4
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input5
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input6
#endif      // default_mux_in_8
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input9
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input10
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input11
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input12
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // signal_left
         rMAC = rMAC + r2*r1, r2 = M[I3, M2], r1 = M[I4, M0];  // signal_right
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // next input1
         rMAC = rMAC ASHIFT r4; 
         M[I1, M1] = rMAC; // store output
      lp_sample:

      jump jp_done_lp_sample;

jp_nofunc:

      do lp_sample_nofunc;                          // for j=0 to blksize-1
         // for k=1 to 8         
         //    acc += input[k] * mantissa[j*8+k] * enabled[j*8+k] (x8)
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input2 // stall 
#if    defined(MUX_IN_14)          
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input3
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input4
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input5
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input6
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input7
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input8
#elif  defined(MUX_IN_12)         
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input3
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input4
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input5
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input6
#endif      // default mux_in_8
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input9
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input10
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input11
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // input12
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // signal_left
         rMAC = rMAC + r2*r1, r2 = M[I3, M2], r1 = M[I4, M0];  // signal_right
         rMAC = rMAC + r2*r1, r2 = M[I3, M1], r1 = M[I4, M3];  // next input1
         rMAC = rMAC ASHIFT r4;  // acc <<= exp[j]
         rMAC = rMAC - rMAC, M[I1, M1] = rMAC; // store output, xfer
      lp_sample_nofunc:

jp_done_lp_sample:

      I3 = I3 + ADDR_PER_WORD*($M.Audio_Mux_NxM.num_signals + $M.Audio_Mux_NxM.num_inputs - 1); // advance I3 to next output's mantissa

      r9 = r9 - 1;
   if NZ jump lp_output;
   
   L1 = 0;   
#ifdef BASE_REGISTER_MODE
   push 0;
   pop  B1;    
#endif
   
   pop FP;
   
   jump $pop_rLink_and_rts;   
   

.ENDMODULE;


