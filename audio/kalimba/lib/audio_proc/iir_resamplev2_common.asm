// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "stack.h"

#ifdef KYMERA

#include "portability_macros.h"
#include "iir_resamplerv2_common_asm_defs.h"
#include "shared_memory_ids_asm_defs.h"

#define IIR_RESAMPLEV2_COMMON_PM	EXT_DEFINED_PM

#else

#include "iir_resamplerv2_common_static.h"

#endif

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif



#define SUPPORTS_DBL_PRECISSION

// *****************************************************************************
// NAME:
//    IIR resampler
// 
// DESCRIPTION:
//    These are the core modules for the IIR resampler
//      $reset_iir_resampler     - reset resampler stage
//      $iir_1stStage_upsample   - process 6th order FIR only upsample stage 
//      $iir_2ndStage_upsample   - process 10th order FIR plus IIR upsample stage
//      $iir_2ndStage_downsample - process 10th order FIR plus IIR downsample stage
//
//    For each IIR filter there is a processing function.  
//    Supported IIR filters are as follows:
//      $iir_resamplerv2.iir_19_s2 - 19th order (2 stages)
//      $iir_resamplerv2.iir_19_s3 - 19th order (3 stages)
//      $iir_resamplerv2.iir_19_s4 - 19th order (4 stages)
//      $iir_resamplerv2.iir_19_s5 - 19th order (5 stages)
//      $iir_resamplerv2.iir_15_s2 - 15th order (2 stages)
//      $iir_resamplerv2.iir_15_s3 - 15th order (3 stages)
//      $iir_resamplerv2.iir_9_s2  - 9th order (2 stages)
//
// *****************************************************************************

// Private Library Exports
.PUBLIC $reset_iir_resampler;
.PUBLIC $iir_1stStage_upsample;
.PUBLIC $iir_2ndStage_upsample;
.PUBLIC $iir_2ndStage_downsample;

.PUBLIC $iir_resamplerv2.iir_9_s2;
.PUBLIC $iir_resamplerv2.iir_15_s3;
.PUBLIC $iir_resamplerv2.iir_15_s2;
.PUBLIC $iir_resamplerv2.iir_19_s5;
.PUBLIC $iir_resamplerv2.iir_19_s4;
.PUBLIC $iir_resamplerv2.iir_19_s3;
.PUBLIC $iir_resamplerv2.iir_19_s2;

#ifdef SUPPORTS_DBL_PRECISSION
.PUBLIC $iir_resamplerv2.iir_19_s2_diir;
.PUBLIC $iir_resamplerv2.iir_19_s3_diir;
.PUBLIC $iir_resamplerv2.iir_19_s4_diir;
.PUBLIC $iir_resamplerv2.iir_19_s5_diir;
.PUBLIC $iir_resamplerv2.iir_15_s3_diir;
.PUBLIC $iir_resamplerv2.iir_15_s2_diir;
.PUBLIC $iir_resamplerv2.iir_9_s2_diir;
#endif

#ifdef KYMERA
.PUBLIC $iir_resamplev2.ReleaseConfig;
.PUBLIC $iir_resamplev2.AllocConfigByRate;
.PUBLIC $iir_resamplev2.AllocConfigById;
#endif

// *****************************************************************************
// MODULE:
//    $_iir_resamplerv2_get_buffer_sizes
//
// DESCRIPTION:
//    Get size of history buffers
//
// INPUTS:
//    r0 - pointer to Configuration
//    r1 - is double precission
//    M1 - MK1 (reserved)
//
// OUTPUTS:
//    r0 - size of history buffers in words
//
// TRASHED REGISTERS:
//    I3, r0-r3
//
// *****************************************************************************
.MODULE $M.iir_buffer_sizes;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
   .DATASEGMENT DM;

// Size of configuration section for various IIR filters
//   Equals: order + 2*num_stages  (arch4: in addrs)
#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)
.VAR/DM_P1_RW    iir_size_table[] =
#elif defined(USE_SHARE_MEM)
.VAR/DM_P0_RW    iir_size_table[] =
#else
.VAR    iir_size_table[] =
#endif 
    23*ADDR_PER_WORD, // $iir_resamplerv2_common.iir_19_s2:  19 + 2*2
    25*ADDR_PER_WORD, // $iir_resamplerv2_common.iir_19_s3:  19 + 3*2
    27*ADDR_PER_WORD, // $iir_resamplerv2_common.iir_19_s4:  19 + 4*2
    29*ADDR_PER_WORD, // $iir_resamplerv2_common.iir_19_s5:  19 + 5*2
    21*ADDR_PER_WORD, // $iir_resamplerv2_common.iir_15_s3:  15 + 3*2
    19*ADDR_PER_WORD, // $iir_resamplerv2_common.iir_15_s2:  15 + 2*2
    13*ADDR_PER_WORD; // $iir_resamplerv2_common.iir_9_s2:   9  + 2*2 

$_iir_resamplerv2_get_buffer_sizes:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_BUFFER_SIZES.IIR_RESAMPLERV2_GET_BUFFER_SIZES.PATCH_ID_0, r3)     // IIR_patchers1
#endif


    // Handle double precission iir
    r3 = 1;
    NULL = r1;
    if Z r3 = NULL;

    push rLink;
    I3 = r0 + $iir_resamplerv2_common.iir_resampler_def_struct.STAGE1_FIELD;
    r2 = r2 XOR r2,  r1 = M[I3,M1];  // Function Pointer, r2=0
    NULL = r1;
    if Z jump jp_size_second_stage;
    // Has a 1st stage
    r0 = M[I3,M1];                // FIR_SIZE_FIELD
    r1 = M[I3,M1];                // IIR_SIZE_FIELD
    r1 = r1 ASHIFT r3;
    r2 = r0 + r1;
    I3 = I3 + ($iir_resamplerv2_common.iir_resampler_stage_def_struct.FRACRATIO_FIELD-$iir_resamplerv2_common.iir_resampler_stage_def_struct.ROUT_FIELD);
    // Check for 1st stage IIR
    NULL = r1;
    if Z jump jp_size_second_stage;
          // 1st stage has an IIR component
          I3 = I3 + ($iir_resamplerv2_common.iir_resampler_stage_def_struct.IIRFUNCTION_FIELD-$iir_resamplerv2_common.iir_resampler_stage_def_struct.FRACRATIO_FIELD);
          r1=M[I3,M1];    // iir function
          Words2Addr(r1); 
    	    r1 = M[$M.iir_buffer_sizes.iir_size_table + r1];
          I3 = I3 + r1;
jp_size_second_stage:
    I3 = I3 + M1;                  // Skip function pointer
    r0 = M[I3,M1];                 // FIR_SIZE_FIELD
    r2 = r2 + r0,   r0 = M[I3,M1]; // IIR_SIZE_FIELD
    r0 = r0 ASHIFT r3;
    r0 = r0 + r2;
    jump $pop_rLink_and_rts;


//void iir_resamplerv2_set_config(void *params,void *lpconfig);
$_iir_resamplerv2_set_config:
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_BUFFER_SIZES.IIR_RESAMPLERV2_SET_CONFIG.PATCH_ID_0, r2)     // IIR_patchers1
#endif
   
   r2 = r0;
   call $block_interrupts;
   M[r2 + $iir_resamplerv2_common.iir_resampler_common_struct.FILTER_FIELD] = r1;
   M[r2 + $iir_resamplerv2_common.iir_resampler_common_struct.RESET_FLAG_FIELD] = NULL;

   call $unblock_interrupts;
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $reset_iir_resampler
//
// DESCRIPTION:
//    Initializes the filter stages
//
// INPUTS:
//    r0 = pointer to common config
//    r4 = Pointer to working buffer
//    r8 = channel struct (reserve)
//
// OUTPUTS:
//    r4 = pointer to working buffer (after channel)
//
// TRASHED REGISTERS:
//    r3,r0,r1,r10,I0,I2,I3
//
// *****************************************************************************
.MODULE $M.reset_iir_resampler;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
   .DATASEGMENT DM;
   
$reset_iir_resampler:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.RESET_IIR_RESAMPLER.RESET_IIR_RESAMPLER.PATCH_ID_0, r3)     // IIR_patchers1
//#endif


   // Clear Reset Flag
   M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.RESET_FLAG_FIELD] = r0;

   // Get resampling configuration
   r3 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.FILTER_FIELD];
   if Z rts;
      
   push rLink;
   push r5;

   // Handle Double Precision
   r5 = 1;
   NULL = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.DBL_PRECISSION_FIELD];
   if Z r5=NULL;

   M0 = MK1;   
   r2 = r2 XOR r2; 

   // 1st Stage Function Pointer
   I2 = r8 + $iir_resamplerv2_common.iir_resampler_channel_struct.PARTIAL2_FIELD;
   I3 = r3 + ($iir_resamplerv2_common.iir_resampler_def_struct.STAGE1_FIELD + ADDR_PER_WORD);
   NULL = M[r3 + $iir_resamplerv2_common.iir_resampler_def_struct.STAGE1_FIELD];   
   if Z jump Reset2ndStage;
      I2 = r8 + $iir_resamplerv2_common.iir_resampler_channel_struct.PARTIAL1_FIELD;
      call reset_iir_stage;
Reset2ndStage:
   I3 = I3 + M0;  // Skip Function Pointer
   // I2 & I3 point to second stage
   call reset_iir_stage;

   pop r5;
   jump $pop_rLink_and_rts;

// *****************************************************************************
//    reset_iir_stage
//
// DESCRIPTION:
//    Initializes the filter state and clears the IIR history buffer.
//    Fir Gordon and beyond the IIR and FIR history buffers are allocated
//    from the end of the data structure
//
// INPUTS:
//    I2  - State Variable Pointers (Partial 1 or 2);
//    I3  - Pointer to Filter Stage configuration (skipped function pointer)
//    r4  - Base Address (>= gordon)
//    r2  = 0   (reserved)
//    M0  = MK1 (reserved)  
//    r5  = double precission scaler
//
// OUTPUTS:
//    I2  - State Variable Pointers for next stage
//    I3  - Pointer to Filter Stage configuration for next stage
//    r4  - Base Address (>= gordon) for next stage history buffer
//
// TRASHED REGISTERS:
//    r3,r0,r1,r10,I0
//
// *****************************************************************************
reset_iir_stage:

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.RESET_IIR_RESAMPLER.RESET_IIR_STAGE.PATCH_ID_0, r1)     // IIR_patchers1
//#endif



   M[I2,M0] = r2;               // Clear Partial input
   r3 = M[I3,M0];               // Get FIR Size
   r0 = M[I3,M0];               // Get IIR Size
   // Adjust IIR size for double precission
   r0 = r0 ASHIFT r5;         
   M[I2,M0] = r2;               // Clear sample counter
   M[I2,M0] = r4;               // Set Fir History Ptr
   Words2Addr(r3);              // FIR Size (arch4: in addrs)
   r4 = r4 + r3;
   r1 = r4, M[I2,M0] = r4;      // Set IIR History Ptr

   // Advanced I2,I3 to Next Stage
   I3 = I3 + ($iir_resamplerv2_common.iir_resampler_stage_def_struct.FRACRATIO_FIELD-$iir_resamplerv2_common.iir_resampler_stage_def_struct.ROUT_FIELD);

   // Check if stage has IIR component
   r10 = r0;
   if Z rts;

   Words2Addr(r0);              // IIR Size (arch4: in addrs)
   r4 = r4 + r0;
   
   I0  = r1;                    // IIR History Ptr
   I3 = I3 + ($iir_resamplerv2_common.iir_resampler_stage_def_struct.IIRFUNCTION_FIELD-$iir_resamplerv2_common.iir_resampler_stage_def_struct.FRACRATIO_FIELD);
  
   // Advance I3 over iir coefficients
   // Get Size of IIR filter configuration
   r1=M[I3,M0];    // IIR Process Function Ptr
   Words2Addr(r1); 
   r1 = M[$M.iir_buffer_sizes.iir_size_table + r1];
   I3 = I3 + r1;            

   // Clear IIR History
   do clr_loop;
      M[I0, MK1]=r2;
clr_loop:

   rts;

// *****************************************************************************
//    estimate_iir_resampler_consumed
//
// DESCRIPTION:
//    Estimate consumption based on available  space
//
// INPUTS:
//    r0 = pointer to common config
//    r1 = available space
//
// OUTPUTS:
//    r1 = amount consumed (estimate)
//
// TRASHED REGISTERS:
//    r0,r2,rMAC,I2
//
// *****************************************************************************
$estimate_iir_resampler_consumed:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.RESET_IIR_RESAMPLER.ESTIMATE_IIR_RESAMPLER_CONSUMED.PATCH_ID_0, r2)     // IIR_patchers1
//#endif



   M0 = MK1;   
   r2 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.FILTER_FIELD];
   I2 = r2;

   // Output space in r1
   r2 = M[I2,M0];                              // resample int_ratio
   rMAC  = r2 * r1 (int),   r2 = M[I2,M0];     // resample frac_ratio
   rMAC  = rMAC  + r2*r1;   
   rMAC=rMAC-1;
   if NEG rMAC = NULL;
   // Check 1st Stage
   r2 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.INTERMEDIATE_SIZE_FIELD];
   r1 = rMAC;               r0 = M[I2,M0];     // resample int_ratio, 1st Stage
   // Limit Input by space in temp buffer
   rMAC  = r0 * r2 (int),   r0 = M[I2,M0];     // resample frac_ratio
   rMAC  = rMAC  + r0*r2;
   if Z rts;

   rMAC=rMAC-1; 
   if NEG rMAC = NULL;
   // Limit Transfer
   Null = r1 - rMAC;
   if POS r1 = rMAC;
   rts;


.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $iir_perform_resample
//
// DESCRIPTION:
//    Process samples with resampler
//
// INPUTS:
//    r0 = Pointer to common struct
//    r8 - Pointer to channel structure
//    r4 - Pointer to history buffers
//    r10 - Amount of Input to process
//    I1,L1,B1 - Input buffers
//    I5,L5,B5 - Output Buffers
//
// OUTPUTS:
//    r4 - Pointer to history buffers (after channel)
//    r7 - amount of data produced by resampler
//
// TRASHED REGISTERS:
//    all
//
// *****************************************************************************

#define DPREC_FUNCTION_TABLE_OFFSET 7

.MODULE $M.iir_perform_resample;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
   .DATASEGMENT DM;

#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)
   .VAR/DM_P1_RW main_function_table[]=
#elif defined(USE_SHARE_MEM)
   .VAR/DM_P0_RW main_function_table[]=
#else
   .VAR main_function_table[]=
#endif
         0,                            // $iir_resamplerv2_common.iir_1stStage_none
         $iir_1stStage_upsample,       // $iir_resamplerv2_common.iir_1stStage_upsample
         $iir_2ndStage_upsample,       // $iir_resamplerv2_common.iir_2ndStage_upsample
         $iir_2ndStage_downsample;     // $iir_resamplerv2_common.iir_2ndStage_downsample
   
#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)
  .VAR/DM_P1_RW iir_function_table[]=
#elif defined(USE_SHARE_MEM)
  .VAR/DM_P0_RW iir_function_table[]=
#else
  .VAR iir_function_table[]=
#endif
         $iir_resamplerv2.iir_19_s2, // $iir_resamplerv2_common.iir_19_s2
         $iir_resamplerv2.iir_19_s3, // $iir_resamplerv2_common.iir_19_s3
         $iir_resamplerv2.iir_19_s4, // $iir_resamplerv2_common.iir_19_s4
         $iir_resamplerv2.iir_19_s5, // $iir_resamplerv2_common.iir_19_s5
         $iir_resamplerv2.iir_15_s3, // $iir_resamplerv2_common.iir_15_s3
         $iir_resamplerv2.iir_15_s2, // $iir_resamplerv2_common.iir_15_s2
         $iir_resamplerv2.iir_9_s2  // $iir_resamplerv2_common.iir_9_s2
#ifdef SUPPORTS_DBL_PRECISSION   
         ,$iir_resamplerv2.iir_19_s2_diir, // $iir_resamplerv2_common.iir_19_s2
         $iir_resamplerv2.iir_19_s3_diir, // $iir_resamplerv2_common.iir_19_s3
         $iir_resamplerv2.iir_19_s4_diir, // $iir_resamplerv2_common.iir_19_s4
         $iir_resamplerv2.iir_19_s5_diir, // $iir_resamplerv2_common.iir_19_s5
         $iir_resamplerv2.iir_15_s3_diir, // $iir_resamplerv2_common.iir_15_s3
         $iir_resamplerv2.iir_15_s2_diir, // $iir_resamplerv2_common.iir_15_s2
         $iir_resamplerv2.iir_9_s2_diir  // $iir_resamplerv2_common.iir_9_s2
#endif
         ;
          
$iir_perform_resample:
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_PERFORM_RESAMPLE.IIR_PERFORM_RESAMPLE.PATCH_ID_0, r2)     // IIR_patchers1
#endif
   
   M0 = MK1;

   // Input Scale
   r2 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.INPUT_SCALE_FIELD];
   // Output Scale
   r5 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.OUTPUT_SCALE_FIELD];
   // If Filter Config is NULL, then passthrough
   r3 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.FILTER_FIELD]; 
   if Z jump jp_passthrough;
   
   // Constants
   push FP;

   /* Get Double Precission flag  */
   r9 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.DBL_PRECISSION_FIELD];

   // History Buffers appended to end of data structure
   push r4; 
   pop  FP;

   // 1st Stage Function Pointer
   I2 = r8 + $iir_resamplerv2_common.iir_resampler_channel_struct.PARTIAL2_FIELD;
   I3 = r3 + ($iir_resamplerv2_common.iir_resampler_def_struct.STAGE1_FIELD + ADDR_PER_WORD); // Points to Field After Function Pointer
   r7 = M[r3+$iir_resamplerv2_common.iir_resampler_def_struct.STAGE1_FIELD];                  // First Function Pointer    
   if Z jump Perform2ndStage;
      // Save Buffers on stack
      r0 = M[r0 + $iir_resamplerv2_common.iir_resampler_common_struct.INTERMEDIATE_PTR_FIELD];
      
      push B5; 
      pushm <r0,r5>;
      pushm <I5,L5>;

      // Set Temp Buffer (r0) as output 
      I5 = r0;
      L5 = NULL;     
      r5 = NULL;    // Make Output Is Q8.15  
      I2 = r8 + $iir_resamplerv2_common.iir_resampler_channel_struct.PARTIAL1_FIELD;

      //    I1  - Input (Circ)					
      //    I5  - Output (Temp - Linear)		
      //    r2  - Input Scaling Factor          
      //    r5  - Output Scaling Factor         
      //    r10 - amount of input (consumed)
      //    M0    = MK1
      //    I3  - Config Pointer
      //    I2  - State Variable Pointer
      //    r9  - double precission

      push r9;
      Words2Addr(r7);  
      r7 = M[main_function_table + r7];
      call r7;
      pop  r9;
      // Use Temp Buffer for next Stage Input (Linear)      
      L1  = NULL;
      r10 = r7;
      r2  = NULL;      // Input is Q8.15
        
      // Restore Buffers from Stack 
      popm <I5,L5>;
      popm <r0,r5>;
      I1 = r0;
      pop B5;       

Perform2ndStage:

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_PERFORM_RESAMPLE.PERFORM2NDSTAGE.PATCH_ID_0, r4)     // IIR_patchers1
//#endif


   r0=M[I3,M0];   // Function Pointer
   nop;
   //    I1  - Input (Circ)					
   //    I5  - Output (Circ)		
   //    r2  - Input Scaling Factor          
   //    r5  - Output Scaling Factor         
   //    r10 - amount of input (consumed)
   //    M0    = MK1
   //    I3  - Config Pointer
   //    I2  - State Variable Pointer
   //    r9  - double precission
   Words2Addr(r0);  
   r0 = M[main_function_table + r0];
   call r0;

   // Return working buffer pointer (after channel)
   r4 = FP;
   pop FP;

jp_done:
   // Clear Circular Registsers
   L1 = NULL;
   L5 = NULL;
   push NULL;
   B0 = M[SP - 1*ADDR_PER_WORD];
   B1 = M[SP - 1*ADDR_PER_WORD];
   B4 = M[SP - 1*ADDR_PER_WORD];
   pop   B5;

   // r7  - amount of aoutput (produced) 
   jump $pop_rLink_and_rts;

jp_passthrough:
    
   // r7 is amount produced 
   r7 = r10;
   r2 = r2 + r5;
   r10 = r10 - 1,    rMAC=M[I1,M0];
   rMAC = rMAC ASHIFT r2;
   do lp_copy;
      rMAC=M[I1,M0],  M[I5,M0]=rMAC;  
      rMAC = rMAC ASHIFT r2;   
   lp_copy:
   M[I5,M0]=rMAC;
   jump jp_done;


.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $iir_1stStage_upsample
//
// DESCRIPTION:
//    Processing 6th order FIR only upsample stage 
//
// INPUTS:
//    I1,L1,B1  - Input Buffer				
//    I5,L5,B5  - Output Buffer				 
//    I2  - State Variable Pointers
//           Partial Samples
//           Polyphase Counter
//           FIR History Buffer
//           IIR History Buffer
//    I3  - Pointer to Filter Stage Config
//           FIR Size = 6
//           IIR Size = 0
//           Rout
//           Input Scale
//           FIR Coefficient Base
//           Fractional Ratio
//    r2  - Input Scaling Factor     
//    r5  - Output Scaling Factor
//    r10 - amout of input (consumed)
//    FP  - Base Address (>= BC7)
//    M0    = MK1 (reserved) 
//    r9    = double precission flag
//
// OUTPUTS:
//    I2  - State Variable Pointers for next stage
//    I3  - Pointer to Filter Stage configuration for next stage
//    FP  - Base Address (>= BC7) for next stage history buffer
//
// TRASHED REGISTERS:
//    all
//
// MIPS:  
//      2*input + 19*output
//
// *****************************************************************************
.MODULE $M.iir_1stStage_upsample;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;

$iir_1stStage_upsample:	   

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_1STSTAGE_UPSAMPLE.IIR_1STSTAGE_UPSAMPLE.PATCH_ID_0, r1)     // IIR_patchers1
#endif

   I3 = I3 + 2*ADDR_PER_WORD;          // Skip FIR & IIR sizes
   r7 = r7 - r7,     r4=M[I3,M0];      // Rout
   Words2Addr(r4);                     // Rout (arch4: in addrs)
   M1 = r4,          r1=M[I3,M0];      // input_scale
   r8 = r1 + r2,     r1=M[I3,M0];      // FIR Coefficient Base
   I6 = r1,          r5=M[I3,M0];      // Fractional Ratio   
   M2 = NULL - r4;
   // M3 = Coefficient Mirror = 2*base + Rout*6 - 1 (arch4: in addrs)
   r2 = r4 * 6 (int);
   M3 = r2 - M0,     r0=M[I2,M0];      // Partial Input N/A   
   M3 = M3 + I6,     r3=M[I2,M0];      // Polyphase Counter  
   M3 = M3 + I6,     r0=M[I2, -MK1];   // FIR History (I2 Points to Polyphase Counter)
   I0 = r0;  
   L0 = 6*ADDR_PER_WORD;
   Addr2Words(r4);                     // Rout (arch4: in words)

   // Set Base Registsers for History Buffers
   push FP;
   pop  B0;
   FP = FP + 6*ADDR_PER_WORD;

   // r3  - Polyphase Counter - [0 ... (Rout-1)]     V
   // r4  - Rout	            			C 
   // r5  - Fractional Ratio           C 
   // r8  - Output Scaling Factor
   // I0  - Fir History  (Circ)			V  
   // I3  - Pointer to Config
   // I6  - Fir Coeffs Base Address	   C
   // r7  - amount produced
   // M1 = Rout
   // M2 = -Rout
   // M3 = Fir Coefficient Reflection
   rMACB = 1.0;
   r9    = 1;

   // First poly phase Index
   // r1 = mod[ input_sample_counter * convert_ratio, 1]*Rout
   r2 = r3 * r5 (int);
   if NEG  r2 = r2 + rMACB;
   r1 = r2 * r4 (frac);
   
   do loop;
      // Loop Through Input
      rMAC = M[I1,M0];						// Input, Coefficient Ptr	
	  M[I0,M0] = rMAC;                      // FIR History x(n)
loop_rpt:
      Words2Addr(r1);                       // fir_phase_idx (arch4: in addrs)
      // Coefficient Ptr : Base + phase (r1)
      I7 = I6 + r1;     
      // Apply FIR Polyphase Kernal (6th order)
      r3 = r3 + r9,				r1 = M[I7,M1], r2 = M[I0,M0];
      // Coef 1
      rMAC = r1 * r2,			r1 = M[I7,M1], r2 = M[I0,M0];
      // Coef 2
      rMAC = rMAC + r1 * r2,	r1 = M[I7,M1], r2 = M[I0,M0];
	  // invert the M reg to perform coef mirroring
      I7 = M3 - I7;				
	  // Coef 3
      rMAC = rMAC + r1 * r2,	r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 4
      rMAC = rMAC + r1 * r2,	r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 5
      rMAC = rMAC + r1 * r2,	r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 6
      rMAC = rMAC + r1 * r2;
   
      // Apply Output Scaling
      r1 = rMAC ASHIFT r8;	

      // Amount Produced
      r7 = r7 + r9,     M[I5,M0] = r1;	      

      // r3 = mod(input_sample_counter,Rout)  
      Null = r3 - r4;
      if GE r3 = r3 - r3;
      // r2 = mod[ input_sample_counter * convert_ratio, 1]
      r2 = r3 * r5 (int);
      if NEG r2 = r2 + rMACB;
      // Polyphase index (r1)
      r1 = r2 * r4 (frac);

      // reprocess last sample (i.e. upsample)
      Null = r5 - r2;
      if LE jump loop_rpt;
      // Can't Have Jump at end of loop
      nop;
   loop:

      // Update State
      r0 = I0,    M[I2,M0]=r3;    // Save Polyphase Counter  
      L0 =NULL,   M[I2,M0]=r0;    // Save FIR History
      I2 = I2 + M0;               // Advance over IIR History Ptr

      // Outputs
      //    r7 = output produced
      //    I2 = State Pointer (next filter)
      //    I3 = Config Pointer (next filter)
      //    M0    = MK1
      rts;

.ENDMODULE;

// Local variables put on stack
#define IIR_FUNCTION_LOCAL_VAR	SP - 1*ADDR_PER_WORD
#define INPUT_SHIFT_LOCAL_VAR   SP - 2*ADDR_PER_WORD

// *****************************************************************************
// MODULE:
//    $iir_2ndStage_upsample
//
// DESCRIPTION:
//    Processing 10th order FIR plus IIR upsample stage
//
// INPUTS:
//    I1,L1,B1  - Input Buffer				
//    I5,L5,B5  - Output Buffer				 
//    I2  - State Variable Pointers
//           Partial Samples
//           Polyphase Counter
//           FIR History Buffer
//           IIR History Buffer
//    I3  - Pointer to Filter Stage Config
//           FIR Size = 10
//           IIR Size
//           Rout
//           input scale
//           output scale
//           FIR Coefficient Base
//           Fractional Ratio
//           Integer ratio (not used = 0)
//           IIR Function Pointer
//           IIR coefficients and scaling factors
//    r2  - Input Scaling Factor     
//    r5  - Output Scaling Factor
//    r10 - amout of input (consumed)
//    FP  - Base Address (>= BC7)
//    M0    = MK1 (reserved) 
//    r9    = double precission flag
//
// OUTPUTS:
//    I2  - State Variable Pointers for next stage
//    I3  - Pointer to Filter Stage configuration for next stage
//    FP  - Base Address (>= BC7) for next stage history buffer
//
// TRASHED REGISTERS:
//    all
//
// MIPS:  
//      2*input + (31+IIR order)*output	 : IIR order = 9,15,or 19
//
// *****************************************************************************
.MODULE $M.iir_2ndStage_upsample;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
   
$iir_2ndStage_upsample:
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_2NDSTAGE_UPSAMPLE.IIR_2NDSTAGE_UPSAMPLE.PATCH_ID_0, r1)     // IIR_patchers1
#endif
   
    
   I3 = I3 + M0;                       // Skip FIR size
   r7 = r7 - r7,     r1=M[I3,M0];      // IIR Size 

   NULL = r9;
   if NZ r1 = r1 + r1; 		       // double precision IIR

   // Set Base Registsers for History Buffers
   push FP;
   pop  B0;
   FP = FP + 10*ADDR_PER_WORD;
   push FP;
   pop  B4;
   Words2Addr(r1);                     // IIR Size (arch4: in addrs)
   FP = FP + r1;
   L4 = r1,          r4=M[I3,M0];      // Rout
   Words2Addr(r4);                     // Rout (arch4: in addrs)
   M1 = r4,          r1=M[I3,M0];      // input_scale
   r6 = r1 + r2,     r1=M[I3,M0];      // output_scale
   r8 = r1 + r5,     r1=M[I3,M0];      // FIR Coefficient Base
   I6 = r1,          r5=M[I3,M0];      // Fractional Ratio 
   M2 = NULL - r4,   r0=M[I2,M0];      // Partial Samples  (NA)
   // M3 = Coefficient Mirror = 2*base + Rout*10 - 1 (arch4: in addrs)
   r2 = r4 * 10 (int);    
   push I2;
   M3 = r2 - M0,     r3=M[I2,M0];      // Polyphase Counter
   M3 = M3 + I6,     r1=M[I2,M0];      // Fir History
   M3 = M3 + I6,     r2=M[I2,M0];      // IIR History
   I0 = r1,          r0=M[I3,M0];      // Integer ratio  
   I4 = r2,          r0=M[I3,M0];      // IIR Function Pointer 
   L0 = 10*ADDR_PER_WORD;
   Addr2Words(r4);                     // Rout (arch4: in words)


#ifdef SUPPORTS_DBL_PRECISSION 
   r1 = r0 + DPREC_FUNCTION_TABLE_OFFSET;
   NULL = r9;
   if NZ r0=r1;
#endif   
   Words2Addr(r0);  
   r0 = M[$M.iir_perform_resample.iir_function_table + r0];
   push r0;				// Save IIR Function as local variable

   rMACB = 1.0;
   r9    = 1;
   
   // r7  - amount produced
   // r0  - IIR Function Pointer
   // r5  - Fractional Ratio				C 
   // r4  - Rout	            			C 
   // r6  - Input Scaling Factor   
   // r8  - Output Scaling Factor
   // I6  - Fir Coeffs Base Address	   C
   // M1 = Rout
   // M2 = -Rout
   // M3 = Fir Coefficient Reflection
   // I0  - Fir History  (Circ)			V   
   // I4  - IIR History (Circ)		   V  
   // r3  - Polyphase Counter - [0 ... (Rout-1)]     V

   // First phase (poly phase)
   // r1 = mod[ input_sample_counter * convert_ratio, 1]*Rout
   r2 = r3 * r5 (int);
   if NEG  r2 = r2 + rMACB;
   r1 = r2 * r4 (frac);
        
   // Note: Put loop in function
   do loop;
      // Loop Through Input
      rMAC = M[I1,M0];          // Input, Coefficient Ptr
      M[I0,M0] = rMAC;          // FIR History x(n)
loop_rpt:
      Words2Addr(r1);           // fir_phase_idx (arch4: in addrs)
      // Coefficient Ptr : base + phase (r1)
      I7 = I6 + r1;
      // Apply FIR Polyphase Kernal (10th order)
      r3 = r3 + r9,           r1 = M[I7,M1], r2 = M[I0,M0];
      // Coef 1
      rMAC = r1 * r2,         r1 = M[I7,M1], r2 = M[I0,M0];
      // Coef 2
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M1], r2 = M[I0,M0];
      // Coef 3
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M1], r2 = M[I0,M0];
      // Coef 4
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M1], r2 = M[I0,M0];
       // invert the M reg to perform coef mirroring
      I7 = M3 - I7;
      // Coef 5
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 6
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 7
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 8
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 9
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];
      // Coef 10
      rMAC = rMAC + r1 * r2;

      // Apply IIR
      I2 = I3;
      r0 = M[IIR_FUNCTION_LOCAL_VAR];
      call r0;		         // Handle IIR,	 
#ifdef SUPPORTS_DBL_PRECISSION 
      M2 = NULL - M1;      // restore M2, trashed by IIR call;
      r4 = M1;             // restore r4 trashed by IIR call
      Addr2Words(r4);      // Rout (arch4: in words)
#endif   
      
      // Apply Output Scaling Factor
      rMAC = rMAC ASHIFT r8,  M[I4,M0] = r1;   // History y(n)
      //increment counter write the result
      r7 = r7 + r9,           M[I5,M0] = rMAC;    

      // r3 = mod(input_sample_counter,Rout)  
      Null = r3 - r4;
      if GE r3 = r3 - r3;
      // rb = mod[ input_sample_counter * convert_ratio, 1]
      r2 = r3 * r5 (int);
      if NEG  r2 = r2 + rMACB;
      // Polohase Index
      r1 = r2 * r4 (frac);      

      // reprocess last sample (i.e. upsample)
      Null = r5 - r2;
      if LE jump loop_rpt;
      // Can't have jump at end of loop
      nop;  
   loop:

   // Update State
   SP = SP - 1*ADDR_PER_WORD;	// remove local variables
   I3 = I2;
   pop I2;
   r0 = I0,    M[I2,M0]=r3;      // Polyphase Counter
   r0 = I4,    M[I2,M0]=r0;      // FIR History
   L0 = NULL,  M[I2,M0]=r0;      // IIR History
   L4 = NULL;                    

   // Outputs
   //    r7 = output produced
   //    I2 = State Pointer (next filter)
   //    I3 = Config Pointer (next filter)
   //    M0    = MK1

   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $iir_2ndStage_downsample
//
// DESCRIPTION:
//    Processing 10th order FIR plus IIR downsample stage
//
// INPUTS:
//    I1,L1,B1  - Input Buffer				
//    I5,L5,B5  - Output Buffer				 
//    I2  - State Variable Pointers
//           Partial Samples
//           Polyphase Counter
//           FIR History Buffer
//           IIR History Buffer
//    I3  - Pointer to Filter Stage Config
//           FIR Size = 10
//           IIR Size
//           Rout
//           input scale
//           output scale
//           FIR Coefficient Base
//           Fractional Ratio
//           Integer ratio (not used = 0)
//           IIR Function Pointer
//           IIR coefficients and scaling factors
//    r2  - Input Scaling Factor     
//    r5  - Output Scaling Factor
//    r10 - amout of input (consumed)
//    FP  - Base Address (>= BC7)
//    M0    = MK1 (reserved) 
//    r9    = double precission flag
//
// OUTPUTS:
//    I2  - State Variable Pointers for next stage
//    I3  - Pointer to Filter Stage configuration for next stage
//    FP  - Base Address (>= BC7) for next stage history buffer
//    r7  -  output produced
//
// TRASHED REGISTERS:
//    all
//
// MIPS:  
//     (10 + IIR Order)*input + 27*output 	 : IIR order = 9,15,or 19
//
// *****************************************************************************
.MODULE $M.iir_2ndStage_downsample;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
 
$iir_2ndStage_downsample:

   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_2NDSTAGE_DOWNSAMPLE.IIR_2NDSTAGE_DOWNSAMPLE.PATCH_ID_0, r1)     // IIR_patchers1
#endif
    
    
   I3 = I3 + M0;                       // Skip FIR size
   r7 = r7 - r7,     r1=M[I3,M0];      // IIR Size 

   NULL = r9;
   if NZ r1 = r1 + r1;			         // double precision IIR	
   
   // Set Base Registsers for History Buffers
   push FP;
   pop  B0;
   FP = FP + 10*ADDR_PER_WORD;
   push FP;
   pop  B4;
   Words2Addr(r1);                     // IIR Size (arch4: in addrs)
   FP = FP + r1;
   L4 = r1,          r4=M[I3,M0];      // Rout
   Words2Addr(r4);                     // Rout (arch4: in addrs)
   M1 = r4,          r1=M[I3,M0];      // input_scale
   r6 = r1 + r2,     r1=M[I3,M0];      // output_scale
   r8 = r1 + r5,     r1=M[I3,M0];      // FIR Coefficient Base
   I6 = r1;
   push I2;
   M2 = NULL - r4,   r1=M[I2,M0];      // Partial Count
   // M3 = Coefficient Mirror = 2*base + Rout*10 - 1 (arch4: in addrs)
   r2 = r4 * 10 (int);
   M3 = r2 - M0,     r3=M[I2,M0];      // polyphase counter
   M3 = M3 + I6; 
   M3 = M3 + I6,     r0=M[I2,M0];      // FIR History 
   I0=r0,            r0=M[I2,M0];      // IIR History
   I4=r0;
   L0=10*ADDR_PER_WORD;
   Addr2Words(r4);                     // Rout (arch4: in words)
   
   push r8;			     // save r8 as local variable 
   push r9;	           // save double precission, will be replace with iir function ptr
   rMACB = 1.0;
   r9    = 1;

   // r1  - partial input
   // r0  - Integer Ratio              C
   // r3  - Polyphase Counter - [0 ... (Rout-1)]   
   // r5  - Fractional Ratio				C 
   //     - IIR Function Pointer       C
   // r4  - Rout	            			C
   // r6  - Input Scaling Factor   
   // r8  - Output Scaling Factor
   // I6  - Fir Coeffs Base Address	   C
   // r7  - amount consumed
   // I0  - Fir History  (Circ)			   
   // I3  - Pointer to Config  
   // I4  - IIR History (Circ)		  
   // M1 = Rout
   // M2 = -Rout
   // M3 = Fir Coefficient Reflection
   
   // modulous sample counter
   Null = r3 - r4,         r0 = M[I3,M0];    // Frac_ratio
   if GE r3 = r3 - r3;     
   // mod[ input_sample_counter * convert_ratio, 1]
   r2 = r3 * r0 (int),     r5 = M[I3,M0];    // Int_ratio
   if NEG r2 = r2 + rMACB;
   // Adjust Number of input samples per output sample
   Null = r0 - r2,         r0 = M[I3,M0];    // IIR Function Pointer
   if GT r5 = r5 + r9;
   // Reduce required input samples by amount already processed (i.e. partial)
   NULL = r1;
   if NZ r5=r1;
   
#ifdef SUPPORTS_DBL_PRECISSION 
   r1 = r0 + DPREC_FUNCTION_TABLE_OFFSET;
   NULL = M[IIR_FUNCTION_LOCAL_VAR];
   if NZ r0=r1;
#endif 
   Words2Addr(r0);
   r0 = M[$M.iir_perform_resample.iir_function_table + r0];
   M[IIR_FUNCTION_LOCAL_VAR]=r0;	     	// Save IIR Function Pointer

   // Check fo sufficient input
   r10 = r10 - r5;
   if NEG jump insufficient_data; 
      
   // loop to generate output 
loop:
      // Polyphase index (r1)
      r1 = r2 * r4 (frac);  
      Words2Addr(r1);           // fir_phase_idx (arch4: in addrs)
      // Coefficient Ptr : base + phase (r1)
      I7 = I6 + r1;           

      r8 = M[IIR_FUNCTION_LOCAL_VAR];   // IIR Function Pointer  
      read_input:
          I2 = I3,      rMAC = M[I1,M0];                    // Get Input        
          call   r8;		                            // Handle IIR
          r5 = r5 - r9, M[I0,M0] = rMAC, M[I4,M0] = r1;     // IIR history y(n),  FIR history x(n)
      if NZ jump read_input;	
  
#ifdef SUPPORTS_DBL_PRECISSION 
      M2 = NULL - M1;   // restore M2, trashed by IIR call 
      r4 = M1;	        // restore r4 trashed by IIR call
      Addr2Words(r4);      // Rout (arch4: in words)
#endif

      // Apply FIR Polyphase Kernal (10th order)
      r3 = r3 + r9,           r1 = M[I7,M1], r2 = M[I0,M0];     // y(n-9)
      // Coef 1
      rMAC = r1 * r2,         r1 = M[I7,M1], r2 = M[I0,M0];     // y(n-8)
      // Coef 2
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M1], r2 = M[I0,M0];     // y(n-7)
      // Coef 3
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M1], r2 = M[I0,M0];     // y(n-6)
      // Coef 4
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M1], r2 = M[I0,M0];     // y(n-5)
      // invert the M reg to perform coef mirroring
      I7 = M3 - I7;
      // Coef 5
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];     // y(n-4)
      // Coef 6
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];     // y(n-3)
      // Coef 7
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];     // y(n-2)
      // Coef 8
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];     // y(n-1)
      // Coef 9
      rMAC = rMAC + r1 * r2,  r1 = M[I7,M2], r2 = M[I0,M0];     // y(n-0)
      // Coef 10
      rMAC = rMAC + r1 * r2;

      // Apply Output Scaling Factor
      r8 = M[INPUT_SHIFT_LOCAL_VAR];
      r1 = rMAC ASHIFT r8;
      //write the result, Reset I2 to point to int_ratio
      r7 = r7 + r9,     M[I5,M0] = r1;
      I3 = I3 - 3*ADDR_PER_WORD;
      
      // modulous sample counter
      Null = r3 - r4,         r0 = M[I3,M0];    // Frac_ratio
      if GE r3 = r3 - r3;     
      // mod[ input_sample_counter * convert_ratio, 1]
      r2 = r3 * r0 (int),     r5 = M[I3,M0];    // Int_ratio
      if NEG r2 = r2 + rMACB;
      // Adjust Number of input samples per output sample
      Null = r0 - r2,         r0 = M[I3,M0];    // IIR Function Pointer
      if GT r5 = r5 + r9;
      
      // Check fo sufficient input
      r10 = r10 - r5;
      if POS jump loop;   

insufficient_data:
      // r10 < 0 amount of insufficient input        
      // r5 is amount of input required per output  
      r4 = r5 + r10;  
      if Z jump done; 
      r5 = r4;
      r8 = M[IIR_FUNCTION_LOCAL_VAR];	// IIR Function Pointer
      read_input_end:
          I2 = I3,      rMAC = M[I1,M0];                    // Get Input
          call   r8;		                            // Handle IIR
          r5 = r5 - r9, M[I0,M0] = rMAC, M[I4,M0] = r1;     // IIR history y(n),  FIR history x(n)
      if NZ jump read_input_end;
      r4 = NULL - r10;
done:

   // Update State
   SP = SP - 2*ADDR_PER_WORD;	// remove local variables
   I3=I2;
   pop I2;
   L0=NULL,    M[I2,M0]=r4;      // Partial input
   r0=I0,      M[I2,M0]=r3;      // polyphase count
   r0=I4,      M[I2,M0]=r0;      // FIR History
   L4=NULL,    M[I2,M0]=r0;      // IIR History

   // Outputs
   //    r7 = output produced
   //    I2 = State Pointer (next filter)
   //    I3 = Config Pointer (next filter)
   //    M0    = MK1
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s2
//
// DESCRIPTION:
//    Process 19th order IIR in 2 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value
//
// TRASHED REGISTERS:
//    r1,r2
//
// MIPS:  
//     rate*26
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s2;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
   
$iir_resamplerv2.iir_19_s2:  
	r2 = rMAC ASHIFT r6,    r1 = M[I2,M0];                 // Shift Input. Coefficient
	rMAC = r2 * r1,         r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-10)

	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-9)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-8)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-7)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-6)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-2)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History g(n-9)

	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History y(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-8)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-7)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-6)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1];				   // Scale Factor

	rMAC = rMAC ASHIFT r1;
	r1   = rMAC;
	rts;
.ENDMODULE;   



// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s3
//
// DESCRIPTION:
//    Process 19th order IIR in 3 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value
//
// TRASHED REGISTERS:
//    r1,r2
//
// MIPS:  
//     rate*28
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s3;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
$iir_resamplerv2.iir_19_s3:
    r2 = rMAC ASHIFT r6,    r1 = M[I2,M0];                 // Shift Input. Coefficient
	rMAC = r2 * r1,         r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-7)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-6)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-2)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History g(n-6)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History y(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-1)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History h(n-6)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History g(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1];				   // Scale Factor
	rMAC = rMAC ASHIFT r1;
	r1   = rMAC;
	rts;
.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s4
//
// DESCRIPTION:
//    Process 19th order IIR in 4 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value
//
// TRASHED REGISTERS:
//    r1,r2
//
// MIPS:  
//     rate*30
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s4;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
$iir_resamplerv2.iir_19_s4:
    r2 = rMAC ASHIFT r6,    r1 = M[I2,M0];                 // Shift Input. Coefficient
	rMAC = r2 * r1,         r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-6)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-2)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History g(n-5)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History y(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-1)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History h(n-4)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History g(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History u(n-4)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History h(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History u(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History u(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History u(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1];				   // Scale Factor
	rMAC = rMAC ASHIFT r1;
	r1   = rMAC;
	rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s5
//
// DESCRIPTION:
//    Process 19th order IIR in 5 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value
//
// TRASHED REGISTERS:
//    r1,r2
//
// MIPS:  
//     rate*32
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s5;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;

$iir_resamplerv2.iir_19_s5:
    r2 = rMAC ASHIFT r6,    r1 = M[I2,M0];                 // Shift Input. Coefficient
	rMAC = r2 * r1,         r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-2)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History g(n-4)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History y(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-1)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History h(n-4)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History g(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History u(n-4)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History h(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History u(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History u(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History u(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History v(n-3)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History u(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History v(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History v(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1];				   // Scale Factor
	rMAC = rMAC ASHIFT r1;
	r1   = rMAC;
	rts;
.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_15_s2
//
// DESCRIPTION:
//    Process 15th order IIR in 2 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value
//
// TRASHED REGISTERS:
//    r1,r2
//
// MIPS:  
//     rate*22
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_15_s2;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
$iir_resamplerv2.iir_15_s2:
    r2 = rMAC ASHIFT r6,    r1 = M[I2,M0];                 // Shift Input. Coefficient
	rMAC = r2 * r1,         r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-8)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-7)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-6)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-2)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History g(n-7)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History y(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-6)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1];				   // Scale Factor
	rMAC = rMAC ASHIFT r1;
	r1   = rMAC;
    rts;
.ENDMODULE;




// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_15_s3
//
// DESCRIPTION:
//    Process 15th order IIR in 3 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value
//
// TRASHED REGISTERS:
//    r1,r2
//
// MIPS:  
//     rate*24
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_15_s3;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
$iir_resamplerv2.iir_15_s3:
    r2 = rMAC ASHIFT r6,    r1 = M[I2,M0];                 // Shift Input. Coefficient
	rMAC = r2 * r1,         r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-6)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-2)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History g(n-5)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History y(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];	   // Scale Factor and History h(n-4)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History g(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History h(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1];				   // Scale Factor
	rMAC = rMAC ASHIFT r1;
	r1   = rMAC;
    rts;
.ENDMODULE;




// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_9_s2
//
// DESCRIPTION:
//    Process 9th order IIR in 2 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value
//
// TRASHED REGISTERS:
//    r1,r2
//
// MIPS:  
//     rate*16
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_9_s2;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
$iir_resamplerv2.iir_9_s2:
	r2 = rMAC ASHIFT r6,    r1 = M[I2,M0];                 // Shift Input. Coefficient
	rMAC = r2 * r1,         r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-5)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-4)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-2)
    rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History y(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4,0];    // Scale Factor and History g(n-4)
	rMAC = rMAC ASHIFT r1,  r1 = M[I2,M0];                                 
	rMAC = rMAC * r1,       r1 = M[I2, MK1], M[I4, MK1]=rMAC;    // Coefficient  and History y(n)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-3)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-2)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1], r2 = M[I4, MK1];    // Coefficient  and History g(n-1)
	rMAC = rMAC - r2 * r1,  r1 = M[I2, MK1];				   // Scale Factor
	rMAC = rMAC ASHIFT r1;
	r1   = rMAC;
   rts;

.ENDMODULE;

#ifdef SUPPORTS_DBL_PRECISSION
// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s2_diir
//
// DESCRIPTION:
//    Process 19th order IIR in 2 stages
//    use double precision Y history
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value (MSW)
//
// TRASHED REGISTERS:
//   r0, r1, r2, r4, M2
//
// MIPS:  
//     rate*59
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s2_diir;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
   
$iir_resamplerv2.iir_19_s2_diir:   
    rMAC = rMAC ASHIFT r6 (56bit),                  r1 = M[I2, MK1];     //  read a0 Coefficient, Input LSB.  // store double in rMAC
    r0 = rMAC LSHIFT DAWTH; 										  // XLSB
    r2 = rMAC LSHIFT Null;     										  // XMSB                    
    rMAC =        r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];	  // start YLSB,    ylsb-10          a1
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-9           a2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-8           a3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-7           a4
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-6           a5
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-5           a6
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-4           a7
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-3           a8
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-2           a9
    M2 = -18*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],  r1 = M[I2, 0];    //                ylsb-1           a10
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];     			  //                ymsb-1     	     a10    
    I2 = I2 - 10*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);    						        // 
    rMAC = rMAC - r0 * r1 (SS),                     r1 = M[I2, MK1];    // start YMSB                     a0
    rMAC = rMAC + r2 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-10          a1
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-9           a2
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-8           a3
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-7           a4
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-6           a5
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-5           a6
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-4           a7
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-3           a8
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1];  //                msb-2           a9
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1];    //                dummy           scale out
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1];    // OUT SCALE      GLSB-9    		b0
    r2   = rMAC LSHIFT Null;	    								  // keep YMSB
    r4   = rMAC LSHIFT DAWTH;		    							  // get YLSB  
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];   // start GLSB     YLSB			b1                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-8          b2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-7          b3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-6      	b4
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-5      	b5
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-4      	b6                                   
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-3      	b7                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-2      	b8                                    
    M2 = -16*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];   //                GLSB-1      	b9                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];        			  //                GMSB-1      	b9                                   
    I2 = I2 -  9*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1];    // start GMSB     GMSB-9          b0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1];    //                YMSB        	b1 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-8          b2 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-7          b3 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-6          b4
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-5          b5  
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-4          b6 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-3          b7 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1];  //                GMSB-2          b8  
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1];    //            GLSB/GMSB output    scale out  
    rMAC = rMAC ASHIFT r1 (56bit);

    r0 = rMAC LSHIFT DAWTH;
    r1 = rMAC LSHIFT Null;
    M[I4, M0] = r0;
    rts;
.ENDMODULE;    

// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s3
//
// DESCRIPTION:
//    Process 19th order IIR in 3 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value (MSW)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r4, M2
//
// MIPS:  
//     rate*66
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s3_diir;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
$iir_resamplerv2.iir_19_s3_diir:
    rMAC = rMAC ASHIFT r6 (56bit),                  r1 = M[I2, MK1]; //  				       a0  
    r0 = rMAC LSHIFT DAWTH; 						// XLSB
    r2 = rMAC LSHIFT Null;              			// XMSB              
    rMAC =        r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; // start LSB,  lsb-7     a1
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-6     a2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-5     a3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-4     a4
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-3     a5
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-2     a6
    M2 = -12*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],  r1 = M[I2, 0]; //             lsb-1     a7
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];     			   //             msb-1,    a7    
    I2 = I2 -  7*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),                     r1 = M[I2, MK1]; // MSB                   a0
    rMAC = rMAC + r2 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-7     a1
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-6     a2
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-5     a3
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-4     a4
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-3     a5
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*ADDR_PER_WORD]; //             msb-2     a6
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1(56bit),   r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   glsb-6    b0          
    r2   = rMAC LSHIFT Null;			    					   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// YLSB       ylsb      b1                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  glsb-5    b2                                   
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-4    b3                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//        	  gLSB-3    b4                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-2    b5                                    
    M2 = -10*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 		  	  gLSB-1    b6                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 		  	  gMSB-1    b6                         
    I2 = I2 -  6*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // msb         GMSB-6    b0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; //             ymsb      b1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-5    b2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-4    b3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-3    b4     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             GMSB-2    b5     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1(56bit),   r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   HLSB-6    c0          
    r2   = rMAC LSHIFT Null;									   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// GLSB * b0    GLSB     c1                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			   HLSB-5   c2                                  
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			   HLSB-4   c3                                  
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//              HLSB-3   c4                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			   HLSB-2   c5                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 			   HLSB-1   c6                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 			   HMSB-1   c6                        
    I2 = I2 -  6*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // HMSB-1*b6    HMSB-6   c0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; // GMSB*b0      GMSB     c1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //              GMSB-5   c2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //              GMSB-4   c3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //              GMSB-3   c4     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //              GMSB-2   c5     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //       adv to output   scale out     
    rMAC = rMAC ASHIFT r1 (56bit);
    r0 = rMAC LSHIFT DAWTH;
    r1 = rMAC LSHIFT Null;
    M[I4, M0] = r0;
    rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s4_diir
//
// DESCRIPTION:
//    Process 19th order IIR in 4 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value (MSW)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r4, M2
//
// MIPS:  
//     rate*74
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s4_diir;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;

$iir_resamplerv2.iir_19_s4_diir:
    rMAC = rMAC ASHIFT r6 (56bit),                  r1 = M[I2, MK1]; //  				       a0  
    r0 = rMAC LSHIFT DAWTH; 						// XLSB
    r2 = rMAC LSHIFT Null;              			// XMSB              
    rMAC =        r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; // start LSB,  lsb-6     a1
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-5     a2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-4     a3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-3     a4
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-2     a5
    M2 = -10*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],  r1 = M[I2, 0]; //             lsb-1     a6
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];     			   //             msb-1,    a6    
    I2 = I2 -  6*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),                     r1 = M[I2, MK1]; // MSB                   a0
    rMAC = rMAC + r2 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-6     a1
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-5     a2
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-4     a3
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-3     a4
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             msb-2     a5
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   glsb-5    b0          
    r2   = rMAC LSHIFT Null;			    					   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// YLSB        ylsb      b1                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-4    b2                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//        	  gLSB-3    b3                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-2    b4                                    
    M2 = -8*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 		  	  gLSB-1    b5                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 		  	  gMSB-1    b5                         
    I2 = I2 -  5*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // msb         GMSB-5    b0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; //             ymsb      b1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-4    b2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-3    b3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             GMSB-2    b4     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   HLSB-4    c0          
    r2   = rMAC LSHIFT Null;									   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// GLSB * c0    GLSB     c1                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//              HLSB-3   c2                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			   HLSB-2   c3                                     
    M2 = -6*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 			   HLSB-1   c4                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 			   HMSB-1   c4                        
    I2 = I2 -  4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // HMSB-1*b6    HMSB-4   c0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; // GMSB*c0      GMSB     c1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //              HMSB-3   c2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //              HMSB-2   c3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //       	   dummy   scale out     
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   ILSB-4    d0          
    r2   = rMAC LSHIFT Null;									   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// HLSB * d0    HLSB     d1                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//              ILSB-3   d2                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			   ILSB-2   d3                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 			   ILSB-1   d4                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 			   IMSB-1   d4                        
    I2 = I2 - 4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // IMSB-1*d4    IMSB-4   d0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; // HMSB*d0      HMSB     d1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //              IMSB-3   d2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //              IMSB-2   d3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //       adv to output   scale out     
    rMAC = rMAC ASHIFT r1 (56bit);
    r0 = rMAC LSHIFT DAWTH;
    r1 = rMAC LSHIFT Null;
    M[I4, M0] = r0;
    rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_19_s5_diir
//
// DESCRIPTION:
//    Process 19th order IIR in 5 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value (MSW)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r4, M2
//
// MIPS:  
//     rate*80
// TBD: MIPS estimation after code optimization
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_19_s5_diir;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;

$iir_resamplerv2.iir_19_s5_diir:
    rMAC = rMAC ASHIFT r6 (56bit),                  r1 = M[I2, MK1]; //  				       a0  
    r0 = rMAC LSHIFT DAWTH; 						// XLSB
    r2 = rMAC LSHIFT Null;              			// XMSB              
    rMAC =        r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; // start LSB,  lsb-4     a1
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-3     a2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-2     a3
    M2 = -6*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],  r1 = M[I2, 0]; //             lsb-1     a4
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];     			   //             msb-1,    a4    
    I2 = I2 -  4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),                     r1 = M[I2, MK1]; // MSB                   a0
    rMAC = rMAC + r2 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-4     a1
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-3     a2
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             msb-2     a3
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   glsb-4    b0          
    r2   = rMAC LSHIFT Null;			    					   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// YLSB        ylsb      b1                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-3    b2                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-2    b3                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 		  	  gLSB-1    b4                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 		  	  gMSB-1    b4                         
    I2 = I2 -  4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // msb         GMSB-4    b0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; //             ymsb      b1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-3    b2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             GMSB-2    b3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   HLSB-4    c0          
    r2   = rMAC LSHIFT Null;									   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// GLSB * c0    GLSB     c1                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//              HLSB-3   c2                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			   HLSB-2   c3                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 			   HLSB-1   c4                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 			   HMSB-1   c4                        
    I2 = I2 -  4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // HMSB-1*c4    HMSB-4   c0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; // GMSB*c0      GMSB     c1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //              HMSB-3   c2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //              HMSB-2   c3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //       	   dummy    scale out     
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   ILSB-4    d0          
    r2   = rMAC LSHIFT Null;									   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// HLSB * d0    HLSB     d1                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//              ILSB-3   d2                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			   ILSB-2   d3                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 			   ILSB-1   d4                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 			   IMSB-1   d4                        
    I2 = I2 - 4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // IMSB-1*d4    IMSB-4   d0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; // HMSB*d0      HMSB     d1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //              IMSB-3   d2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //              IMSB-2   d3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //       	   dummy   scale out     
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   JLSB-3    e0          
    r2   = rMAC LSHIFT Null;									   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// ILSB * e0    ILSB     e1                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];//              JLSB-2   e2                                     
    M2 = -4*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 			   JLSB-1   e3                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 			   JMSB-1   e3                        
    I2 = I2 - 3*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // JMSB-1*e4    JMSB-3   e0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; // IMSB*e0      IMSB     e1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //              IMSB-2   e2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //       adv to output   scale out     
    rMAC = rMAC ASHIFT r1 (56bit);
    r0 = rMAC LSHIFT DAWTH;
    r1 = rMAC LSHIFT Null;
    M[I4, M0] = r0;
    rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_15_s2
//
// DESCRIPTION:
//    Process 15th order IIR in 2 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value (MSW)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r4, M2
//
// MIPS:  
//     rate*51
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_15_s2_diir;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;

$iir_resamplerv2.iir_15_s2_diir:
    rMAC = rMAC ASHIFT r6 (56bit),                  r1 = M[I2, MK1];     //  read a0 Coefficient, Input LSB.  // store double in rMAC
    r0 = rMAC LSHIFT DAWTH; 										  // XLSB
    r2 = rMAC LSHIFT Null;     										  // XMSB                    
    rMAC =        r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];	  // start YLSB,    ylsb-8           a1
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-7           a2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-6           a3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-5           a4
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-4           a5
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-3           a6
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-2           a7
    M2 = -14*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],  r1 = M[I2, 0];    //                ylsb-1           a8
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];     				  //                ymsb-1     	     a8    
    I2 = I2 -  8*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);    								// 
    rMAC = rMAC - r0 * r1 (SS),                     r1 = M[I2, MK1];    // start YMSB                     a0
    rMAC = rMAC + r2 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-8           a1
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-7           a2
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-6           a3
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-5           a4
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-4           a5
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-3           a6
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1];    //                msb-2           a7
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1];    //                dummy           scale out
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1];    // OUT SCALE      GLSB-7    		b0
    r2   = rMAC LSHIFT Null;	    								  // keep YMSB
    r4   = rMAC LSHIFT DAWTH;		    							  // get YLSB  
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];   // start GLSB     YLSB			b1                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-6          b2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-5      	b3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-4      	b4                                   
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-3      	b5                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-2      	b6                                    
    M2 = -12*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];   //                GLSB-1      	b7                                   
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];        			  //                GMSB-1      	b7                                   
    I2 = I2 -  7*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1];    // start GMSB     GMSB-7          b0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1];    //                YMSB        	b1 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-6          b2 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-5          b3  
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-4          b4 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-3          b5 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1];    //                GMSB-2          b6  
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1];    //            GLSB/GMSB output    scale out  
    rMAC = rMAC ASHIFT r1 (56bit);
    r0 = rMAC LSHIFT DAWTH;
    r1 = rMAC LSHIFT Null;
    M[I4, M0] = r0;
    rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_15_s3
//
// DESCRIPTION:
//    Process 15th order IIR in 3 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value (MSW)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r4, M2
//
// MIPS:  
//     rate*59
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_15_s3_diir;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;
$iir_resamplerv2.iir_15_s3_diir:
    rMAC = rMAC ASHIFT r6 (56bit),                  r1 = M[I2, MK1]; //  				        a0  
    r0 = rMAC LSHIFT DAWTH; 						// XLSB
    r2 = rMAC LSHIFT Null;              			// XMSB              
    rMAC =        r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; // start LSB,  lsb-6     a1
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-5     a2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-4     a3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-3     a4
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             lsb-2     a5
    M2 = -10*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],  r1 = M[I2, 0]; //             lsb-1     a6
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];     			   //             msb-1,    a6    
    I2 = I2 -  6*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),                     r1 = M[I2, MK1]; // MSB                   a0
    rMAC = rMAC + r2 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-6     a1
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-5     a2
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-4     a3
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             msb-3     a4
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             msb-2     a5
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1(56bit),   r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   glsb-5    b0          
    r2   = rMAC LSHIFT Null;			    					   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// YLSB        ylsb      b1                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  glsb-4    b2                                   
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-3    b3                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 		  	  gLSB-2    b4                                    
    M2 = -8*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 		  	  gLSB-1    b5                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 		  	  gMSB-1    b5                         
    I2 = I2 - 5*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // msb         GMSB-5    b0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; //             ymsb      b1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-4    b2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-3    b3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             GMSB-2    b4     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //             dummy     scale out
    rMAC = rMAC ASHIFT r1(56bit),   r0 = M[I4, 0],  r1 = M[I2, MK1]; // OUT SCALE   HLSB-4    c0          
    r2   = rMAC LSHIFT Null;									   // keep MSB
    r4   = rMAC LSHIFT DAWTH;									   // get LSB
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];// GLSB * b0   GLSB     c1                                    
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			  HLSB-3   c2                                  
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];// 			  HLSB-2   c3                                     
    M2 = -6*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];// 			  HLSB-1   c4                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];                 // 			  HMSB-1   c4                        
    I2 = I2 - 4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1]; // HMSB-1*c4   HMSB-4   c0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1]; // GMSB*b0     GMSB     c1     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1]; //             GMSB-3   c2     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1]; //             GMSB-2   c3     
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1]; //       adv to output   scale out     
    rMAC = rMAC ASHIFT r1 (56bit);
    r0 = rMAC LSHIFT DAWTH;
    r1 = rMAC LSHIFT Null;
    M[I4, M0] = r0;
    rts;
.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $iir_resamplerv2.iir_9_s2
//
// DESCRIPTION:
//    Process 9th order IIR in 2 stages
//
// INPUTS:
//    I4,L4,B4 - iir history  (circ)
//    I2       - descriptor - Scale Factors, Coefficients
//    r6       - input scale
//    rMAC     - input value/output value
///
// OUTPUTS:
//    I2   - State Variable Pointers for next stage
//    rMAC - Filter result
//    r1   - Last History Value (MSW)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r4, M2
//
// MIPS:  
//     rate*39
//
// *****************************************************************************
.MODULE $M.iir_resamplerv2.iir_9_s2_diir;
   .CODESEGMENT IIR_RESAMPLEV2_COMMON_PM;

$iir_resamplerv2.iir_9_s2_diir:
    rMAC = rMAC ASHIFT r6 (56bit),                  r1 = M[I2, MK1];     // read a0 Coefficient, Input LSB.  
    r0 = rMAC LSHIFT DAWTH; 									  // XLSB
    r2 = rMAC LSHIFT Null;     									  // XMSB                    
    rMAC =        r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];	  // start YLSB,    ylsb-5          a1
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-4          a2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-3          a3
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                ylsb-2          a4
    M2 = -8*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],  r1 = M[I2, 0];    //                ylsb-1          a5
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];     				  //                ymsb-1     	    a5    
    I2 = I2 -  5*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);    								// 
    rMAC = rMAC - r0 * r1 (SS),                     r1 = M[I2, MK1];    // start YMSB                     a0
    rMAC = rMAC + r2 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-5           a1
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-4           a2
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                msb-3           a3
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1];    //                msb-2           a4
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1];    //                dummy           scale out
    rMAC = rMAC ASHIFT r1 (56bit),  r0 = M[I4, 0],  r1 = M[I2, MK1];    // OUT SCALE      GLSB-4    		b0
    r2   = rMAC LSHIFT Null;	    								  // keep YMSB
    r4   = rMAC LSHIFT DAWTH;		    							  // get YLSB  
    rMAC =        r4 * r1 (US),     M[I4, 2*MK1] = r4,   r1 = M[I2, MK1];   // start GLSB     YLSB			b1                                     
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-3          b2
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, 2*MK1],   r1 = M[I2, MK1];   //                GLSB-2      	b3                                    
    M2 = -6*ADDR_PER_WORD;
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, MK1],   r1 = M[I2, 0];   //                GLSB-1      	b4                                   
    rMAC = rMAC - r0 * r1 (US),     r0 = M[I4, M2];        			  //                GMSB-1      	b4                                   
    I2 = I2 - 4*ADDR_PER_WORD;
    rMAC = rMAC ASHIFT -DAWTH (56bit);
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 0],  r1 = M[I2, MK1];    // start GMSB     GMSB-4          b0
    rMAC = rMAC + r2 * r1 (SS),     M[I4, 2*MK1] = r2,  r1 = M[I2, MK1];    //                YMSB        	b1 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, MK1];    //                GMSB-3          b2 
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, 2*MK1],  r1 = M[I2, 2*MK1];    //                GMSB-2          b3  
    rMAC = rMAC - r0 * r1 (SS),     r0 = M[I4, MK1],  r1 = M[I2, MK1];    //            GLSB/GMSB output    scale out  
    rMAC = rMAC ASHIFT r1 (56bit);
    r0 = rMAC LSHIFT DAWTH;
    r1 = rMAC LSHIFT Null;
    M[I4, M0] = r0;
   rts;

.ENDMODULE;
#endif

#ifdef KYMERA
.MODULE $M.iir_resampler.DynMemory;
   .CODESEGMENT EXT_DEFINED_PM;
   .DATASEGMENT DM;

   .VAR/DMCONST16 ML_Table[] =   0x0000,0x1002,$shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_2,
                                 0x0000,0x1003,$shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_3,
                                 0x0000,0x1004,$shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_4,
                                 0x0000,0x1006,$shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_6,
                                 0x0000,0x100C,$shared_memory_ids.IIRV2_RESAMPLER_Up_1_Down_12,
                                 0x0000,0x2001,$shared_memory_ids.IIRV2_RESAMPLER_Up_2_Down_1,
                                 0x0000,0x2003,$shared_memory_ids.IIRV2_RESAMPLER_Up_2_Down_3,
                                 0x0000,0x3001,$shared_memory_ids.IIRV2_RESAMPLER_Up_3_Down_1,
                                 0x0000,0x3002,$shared_memory_ids.IIRV2_RESAMPLER_Up_3_Down_2,
                                 0x0000,0x4001,$shared_memory_ids.IIRV2_RESAMPLER_Up_4_Down_1,
                                 0x0000,0x5001,$shared_memory_ids.IIRV2_RESAMPLER_Up_5_Down_1,
                                 0x0000,0x6001,$shared_memory_ids.IIRV2_RESAMPLER_Up_6_Down_1,
                                 0x0000,0x8001,$shared_memory_ids.IIRV2_RESAMPLER_Up_8_Down_1,
                                 0x0000,0xC001,$shared_memory_ids.IIRV2_RESAMPLER_Up_12_Down_1,
                                 0x0002,0x8001,$shared_memory_ids.IIRV2_RESAMPLER_Up_40_Down_1,
                                 0x0005,0x01B9,$shared_memory_ids.IIRV2_RESAMPLER_Up_80_Down_441,
                                 0x0009,0x30A0,$shared_memory_ids.IIRV2_RESAMPLER_Up_147_Down_160,
                                 0x0009,0x3140,$shared_memory_ids.IIRV2_RESAMPLER_Up_147_Down_320,
                                 0x0009,0x3280,$shared_memory_ids.IIRV2_RESAMPLER_Up_147_Down_640,
                                 0x000A,0x0093,$shared_memory_ids.IIRV2_RESAMPLER_Up_160_Down_147,
                                 0x000A,0x01B9,$shared_memory_ids.IIRV2_RESAMPLER_Up_160_Down_441,
                                 0x0014,0x0093,$shared_memory_ids.IIRV2_RESAMPLER_Up_320_Down_147,
                                 0x0014,0x01B9,$shared_memory_ids.IIRV2_RESAMPLER_Up_320_Down_441,
                                 0x001B,0x9050,$shared_memory_ids.IIRV2_RESAMPLER_Up_441_Down_80,
                                 0x001B,0x90A0,$shared_memory_ids.IIRV2_RESAMPLER_Up_441_Down_160,
                                 0x001B,0x9140,$shared_memory_ids.IIRV2_RESAMPLER_Up_441_Down_320,
                                 0x001B,0x9280,$shared_memory_ids.IIRV2_RESAMPLER_Up_441_Down_640,
                                 0x0028,0x0093,$shared_memory_ids.IIRV2_RESAMPLER_Up_640_Down_147,
                                 0x0028,0x01B9,$shared_memory_ids.IIRV2_RESAMPLER_Up_640_Down_441;

   // table mapping share_id's to the corresponding low_mips configuration
   .VAR/DMCONST16 Low_MIPS_Table[] =   
      $shared_memory_ids.IIRV2_RESAMPLER_Up_147_Down_160, $shared_memory_ids.IIRV2_RESAMPLER_Up_147_Down_160_low_mips,
      $shared_memory_ids.IIRV2_RESAMPLER_Up_160_Down_147, $shared_memory_ids.IIRV2_RESAMPLER_Up_160_Down_147_low_mips;

//void  iir_resamplerv2_release_config(void *lpconfig);
$_iir_resamplerv2_release_config:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_RESAMPLER.DYNMEMORY.IIR_RESAMPLERV2_RELEASE_CONFIG.PATCH_ID_0, r1)     // IIR_patchers1
//#endif


   // Is there a config
   NULL = r0;
   if Z rts;
   
   push rLink;
   // Need ID
   push r0;             // Mem Pointer
   call $_shared_id;
   pop  r1;             // r1=Mem Pointer, r0=ID (-1 if invalid)
   // Is Valid
   r0 = r0 + 1;
   if Z jump $pop_rLink_and_rts;
   r0 = r0 - 1;
   r1 = $M.iir_resamplev2.DynamicMem.DynTable_Main;
   call $_DynLoaderReleaseSharedAllocations;
   jump $pop_rLink_and_rts;
   
// void* iir_resamplerv2_allocate_config_by_id(unsigned id);
$_iir_resamplerv2_allocate_config_by_id:
    push rLink;
    
//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_RESAMPLER.DYNMEMORY.IIR_RESAMPLERV2_ALLOCATE_CONFIG_BY_ID.PATCH_ID_0, r1)     // IIR_patchers1
//#endif
    
    push NULL;      // Result is in local variable  
    
    // Get Config
    //    r0 - Pointer to return address
    //    r1 - Pointer to Memory Descriptor (16-bit address space)
    //    r2 - Pointer to Memory Descriptor (24-bit address space)
    //    r3 - Shared Memory Identifier    
    r3 = r0;
    r0 = SP - 1*ADDR_PER_WORD;
    r2 = NULL; // No External References
    r1 = $M.iir_resamplev2.DynamicMem.DynTable_Main;
    call $_DynLoaderProcessSharedAllocations;
    // Get Result (allocated Pointer)
    pop r0; 
    jump $pop_rLink_and_rts;


// void* iir_resamplerv2_allocate_config_by_rate(unsigned in_rate,unsigned out_rate,unsigned low_mips)
$_iir_resamplerv2_allocate_config_by_rate:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_RESAMPLER.DYNMEMORY.IIR_RESAMPLERV2_ALLOCATE_CONFIG_BY_RATE.PATCH_ID_0)     // IIR_patchers1
//#endif

   // ID in r0
   call $_iir_resamplerv2_get_id_from_rate;
   Null = r0;
   if Z jump $pop_rLink_and_rts;

   // allocate shared memory and return pointer in r0
   call $_iir_resamplerv2_allocate_config_by_id;
   jump $pop_rLink_and_rts;

// unsigned iir_resamplerv2_get_id_from_rate(unsigned in_rate,unsigned out_rate,unsigned low_mips);
$_iir_resamplerv2_get_id_from_rate:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_RESAMPLER.DYNMEMORY.IIR_RESAMPLERV2_GET_ID_FROM_RATE.PATCH_ID_0)     // IIR_patchers1
//#endif

   push r2;
   push r4;
   // Check for pass-through
   NULL = r0 - r1;
   if Z jump lp_exit_none;
   
   r4 = r0;
   r3 = r1;
   // Sort X,Y   X>Y
   NULL = r0 - r1;
   if POS jump AllocConfig.dont_swap;
      r2 = r1;
      r1 = r0;
      r0 = r2;
   AllocConfig.dont_swap:

   // Find Common Denominator (r0)
AllocConfig.loopRem:
   rMAC = r1 ASHIFT 0 (LO);
   Div  = rMAC/r0;
   r2   = DivRemainder;

#ifdef KAL_ARCH3
   // For ARCH3 there needs to be a nop here to work round B-89595.
   // It can be removed when the toolchain adds it for us (B-181616).
   Nop;
#endif

   if Z jump AllocConfig.foundDen;
      r1 = r0;
      r0 = r2;
      jump AllocConfig.loopRem;  
AllocConfig.foundDen:

    // Get L,M
    rMAC = r3 ASHIFT 0 (LO);
    Div  = rMAC/r0;
    I3   = &ML_Table;
    rMAC = r4 ASHIFT 0 (LO);
    r3   = DivResult;
    Div  = rMAC/r0;
    r3   = r3 LSHIFT 12;
    r2   = DivResult;
    r2   = r2 AND 0xFFF;
    r3   = r2 OR r3,         r1 = M[I3,M1];   //User Value
    
    // Find Descriptor ID
    r10 = (LENGTH(ML_Table) >> LOG2_ADDR_PER_WORD)/3;
    r4  = 16;
    do AllocConfig.search;
        r1 = r1 LSHIFT r4,     r2 = M[I3,M1];
        r2 = r2 AND 0xFFFF;                    //Mask upper 16 bits
        r2= r2 OR r1,          r0 = M[I3,M1];   //Key
        r2 = r2 - r3,          r1 = M[I3,M1];
        if Z jump AllocConfig.foundId;
    AllocConfig.search:
    // Descriptor Not Found.  Return Error
lp_exit_none:
    r0 = NULL;

AllocConfig.foundId:
    // ID in r0
    pop r4;
    pop r2;
    if NZ jump AllocConfig.getLowMipsID;
    rts;

// if the low_mips flag is set, check if a low_mips resampler configuration 
// exists corresponding to the selected share_id (r0). if yes, switch to the
// share_id for the low_mips configuration
AllocConfig.getLowMipsID:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.IIR_RESAMPLEV2_COMMON_ASM.IIR_RESAMPLER.DYNMEMORY.ALLOCCONFIG.GETLOWMIPSID.PATCH_ID_0)     // IIR_patchers1
//#endif

    r10 = (LENGTH(Low_MIPS_Table) >> LOG2_ADDR_PER_WORD)/2;
    I3   = &Low_MIPS_Table;
    
    r2 = M[I3,M1];
    do AllocConfig.lowMipsLoop;
        Null = r2 - r0, r2 = M[I3,M1];
        if Z r0 = r2, r2 = M[I3,M1];
    AllocConfig.lowMipsLoop:
    rts;

.ENDMODULE;
#endif


