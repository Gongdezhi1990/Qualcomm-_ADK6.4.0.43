// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM function for dbe operator data processing
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "core_library.h"
#include "cbuffer_asm.h"
#include "portability_macros.h"
#include "dbe.h"
#include "dbe_gen_asm.h"
#include "stack.h"

#include "dbe_wrapper_defs.h"
#include "dbe_wrapper_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $_dbe_processing
// extern void dbe_processing(DBE_OP_DATA *op_data, unsigned samples_to_process,dbe_channels *first);
//
// DESCRIPTION:
//    Data processing function.
//
// INPUTS:
//    - r0 = Pointer to DBE extra_op data
//    - r1 = number of samples to process
//    - r2 = first active channel
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.dbe_proc;
	
    .CODESEGMENT PM;
$_dbe_processing:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($dbe_cap.DBE_PROCESSING_ASM.DBE_PROC.DBE_PROCESSING.PATCH_ID_0, r3)     // cap_compander_patchers
#endif
   // *****************************
   // check bypass flag
   // *****************************
   r3 = M[r0 + ($dbe_wrapper.dbe_exop_struct.DBE_CAP_PARAMS_FIELD + $M.DBE.PARAMETERS.OFFSET_DBE_CONFIG)];
   Null = $M.DBE.CONFIG.BYPASS AND r3;
   if NZ jump dbe_pass_through;

   // *****************************
   // check pass-through mode
   // *****************************
   r3 = M[r0 + $dbe_wrapper.dbe_exop_struct.CUR_MODE_FIELD];
   Null = r3 - $M.DBE.SYSMODE.PASS_THRU;
   if Z jump dbe_pass_through;
   // *****************************
   // check mute mode
   // *****************************
   Null = r3 - $M.DBE.SYSMODE.MUTE;
   if Z jump dbe_mute;

   // *****************************************************************************************
   // Save all registers expected for C
   // *****************************************************************************************
   PUSH_ALL_C
   
   // Setup 1st channel (r7)
   r7 = M[r2 + $dbe_wrapper.dbe_channels_struct.DBE_OBJECT_FIELD];
   // Set Input & output buffers 
   r0 = M[r2 + $dbe_wrapper.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];          
   M[r7 + $audio_proc.dbe.INPUT_ADDR_FIELD] = r0;                      
   r0 = M[r2 +$dbe_wrapper.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];          
   M[r7 + $audio_proc.dbe.OUTPUT_ADDR_FIELD] = r0;     
   // Set amount to process
   M[r7 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS] = r1;
   // If mono then set r8=r7
   r8 = r7;
   // Mono or stereo, set r7 & r8
   r5 = M[r2 + $dbe_wrapper.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
   if Z jump dbe_processing_mono_set;
      // Setup 2nd channel (r8)
      r8 = M[r5 + $dbe_wrapper.dbe_channels_struct.DBE_OBJECT_FIELD];
      // Set Input & output buffers 
      r0 = M[r5 + $dbe_wrapper.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];          
      M[r8 + $audio_proc.dbe.INPUT_ADDR_FIELD] = r0;                      
      r0 = M[r5 +$dbe_wrapper.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];          
      M[r8 + $audio_proc.dbe.OUTPUT_ADDR_FIELD] = r0;     
      // Set amount to process
      M[r8 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS] = r1;

      // May need to swap channels if 2nd item in active list is channel 0
      NULL = M[r5 + $dbe_wrapper.dbe_channels_struct.CHAN_IDX_FIELD];
      if NZ jump dbe_processing_stereo_set;
         r8 = r7;
         r7 = M[r5 + $dbe_wrapper.dbe_channels_struct.DBE_OBJECT_FIELD];
dbe_processing_stereo_set:
         r5 = 1;      
dbe_processing_mono_set:
   M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD] = r5;

   // *******************************************************************************************
   // full processing mode : apply the dbe filters to one-channel(mono) or both-channels(stereo)
   // process both channels together in stereo mode
   // *******************************************************************************************
   push r1;    // amount
   push r2;    // channel list
   call $audio_proc.dbe.stream_process;
   pop r0;     // channel list
   pop r1;     // amount

   // *******************************************************************************************
   // Advance buffer pointers (r0=channel list, r1=amount )
   // *******************************************************************************************
   r2 = r1;
   call $_multi_channel_advance_buffers;

dbe_process_done:
   // *****************************************************************************************
   // Restore all registers expected for C
   // *****************************************************************************************
   POP_ALL_C
   rts;


dbe_pass_through:
   // r2 = first channel, r1=amount
   r0 = r2;
   r2 = NULL;  // pass-through
   jump $_multi_channel_copy_mute;

dbe_mute:
   // r2 = first channel, r1=amount
   r0 = r2;
   r2 = 1;     // mute
   jump $_multi_channel_copy_mute;


.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_dbe_initialize
//  void dbe_initialize(DBE_OP_DATA *op_data,dbe_channels *first);
//
// DESCRIPTION:
//    Initialize function
//
// INPUTS:
//    - r0 = Pointer to DBE extra_op data
//    - r1 = first channel pointer
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.dbe_initialize;
    .CODESEGMENT PM;
$_dbe_initialize:
   // First active channel
   NULL = r1;
   if Z rts;

   PUSH_ALL_C
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($dbe_cap.DBE_PROCESSING_ASM.DBE_INITIALIZE.DBE_INITIALIZE.PATCH_ID_0, r9)     // cap_compander_patchers
#endif

   // Clear Init flag
   M[r0 + $dbe_wrapper.dbe_exop_struct.REINITFLAG_FIELD]=NULL;

   // ***************************
   // Extract the sample rate
   // ***************************
   r9 = M[r0 + $dbe_wrapper.dbe_exop_struct.SAMPLE_RATE_FIELD];
   
   dbe_channel_loop:      
      // *******************************************************
      // Set the sample_rate field for the current channel
      // *******************************************************
      r8 = M[r1 + $dbe_wrapper.dbe_channels_struct.DBE_OBJECT_FIELD];
      M[r8 + $audio_proc.dbe.SAMPLE_RATE_FIELD] = r9;
      
      // ***********************************************************
      // call the dbe.initialize function for the current channel
      // ***********************************************************
      push r1;
      call $audio_proc.dbe.initialize;
      pop r1;

      // next chan
      r1 = M[r1 + $dbe_wrapper.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
      if NZ jump dbe_channel_loop;

   POP_ALL_C
   rts;

.ENDMODULE;

