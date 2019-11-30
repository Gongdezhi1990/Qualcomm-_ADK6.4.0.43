// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM function for vse operator data processing
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)
#include "core_library.h"
#include "cbuffer_asm.h"
#include "portability_macros.h"
#include "vse.h"
#include "vse_gen_asm.h"
#include "stack.h"

#include "vse_wrapper_defs.h"
#include "vse_wrapper_asm_defs.h"

#ifdef PATCH_LIBS
#include "patch_library.h"
#endif





// *****************************************************************************
// MODULE:
//    $_vse_processing
// extern void vse_processing(VSE_OP_DATA *op_data, unsigned samples_to_process,vse_channels *channels);
//
// DESCRIPTION:
//    Data processing function.
//
// INPUTS:
//    - r0 = Pointer to VSE extra_op data
//    - r1 = number of samples to process
//    - r2 = pointer to first active channel
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.vse_proc;
    .CODESEGMENT PM;
$_vse_processing:


#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($vse_cap.VSE_PROCESSING_ASM.VSE_PROC.VSE_PROCESSING.PATCH_ID_0, r3)     // cap_vse_patchers
#endif
	// *****************************
    // check bypass flag
    // *****************************
    r3 = M[r0 + ($vse_wrapper.vse_exop_struct.VSE_CAP_PARAMS_FIELD + $M.VSE.PARAMETERS.OFFSET_VSE_CONFIG)];
    Null = $M.VSE.CONFIG.BYPASS AND r3;
    if NZ jump vse_pass_through;

   r3 = M[r0 + $vse_wrapper.vse_exop_struct.CUR_MODE_FIELD];
   // *****************************
   // check pass-through mode
   // *****************************
   Null = r3 - $M.VSE.SYSMODE.PASS_THRU;
   if Z jump vse_pass_through;
   // *****************************
   // check mute mode
   // *****************************
   Null = r3 - $M.VSE.SYSMODE.MUTE;
   if Z jump vse_mute;

   // *****************************************************************************************
   // Save all registers expected for C
   // *****************************************************************************************
   PUSH_ALL_C

   // ************************************************************************
   // Extract the left and right data objects
   // Set the "samples_to_process" value for the left and right data objects
   // Set the input/output buffer pointers for the left and right data objects
   // ************************************************************************
   r7 = M[r2 + $vse_wrapper.vse_channels_struct.VSE_OBJECT_FIELD];

   r0 = M[r2 + $vse_wrapper.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
   M[r7 + $audio_proc.vse.INPUT_ADDR_FIELD] = r0;
   r0 = M[r2 + $vse_wrapper.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
   M[r7 + $audio_proc.vse.OUTPUT_ADDR_FIELD] = r0;
   M[r7 + $audio_proc.vse.SAMPLES_TO_PROCESS] = r1;

   r5 = M[r2 + $vse_wrapper.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
   r8 = M[r5 + $vse_wrapper.vse_channels_struct.VSE_OBJECT_FIELD];

   r0 = M[r5 + $vse_wrapper.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
   M[r8 + $audio_proc.vse.INPUT_ADDR_FIELD] = r0;
   r0 = M[r5 + $vse_wrapper.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
   M[r8 + $audio_proc.vse.OUTPUT_ADDR_FIELD] = r0;
   M[r8 + $audio_proc.vse.SAMPLES_TO_PROCESS] = r1;

   // May need to swap channels if 2nd item in active list is channel 0
   NULL = M[r5 + $vse_wrapper.vse_channels_struct.CHAN_IDX_FIELD];
   if NZ jump vse_processing_no_swap;
      r8 = r7;
      r7 = M[r5 + $vse_wrapper.vse_channels_struct.VSE_OBJECT_FIELD];
   vse_processing_no_swap:

   // *******************************************************************
   // full processing mode : apply VSE filters to both channels
   // r7 = pointer to vse left_data_object
   // r8 = pointer to vse right_data_object
   // *******************************************************************
   push r1;    // amount
   push r2;    // channel list
   call $audio_proc.vse.stream_process;
   pop r0;     // channel list
   pop r1;     // amount

   // *******************************************************************************************
   // Advance buffer pointers (r0=channel list, r1=amount )
   // *******************************************************************************************
   r2 = r1;
   call $_multi_channel_advance_buffers;

vse_process_done:
   // *****************************************************************************************
   // Restore all registers expected for C
   // *****************************************************************************************
   POP_ALL_C
   rts;

vse_pass_through:
   // r2 = first channel, r1=amount
   r0 = r2;
   r2 = NULL;
   jump $_multi_channel_copy_mute;

vse_mute:
   // r2 = first channel, r1=amount
   r0 = r2;
   r2 = 1;
   jump $_multi_channel_copy_mute;


.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_vse_initialize
// extern void vse_initialize(VSE_OP_DATA *op_data,vse_channels *channels);
//
// DESCRIPTION:
//    Initialize function
//
// INPUTS:
//    - r0 = Pointer to VSE extra_op data
//    - r1 = pointer to first active channel
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.vse_initialize;
    .CODESEGMENT PM;
$_vse_initialize:

    // First active channel
   NULL = r1;
   if Z rts;

   PUSH_ALL_C

   // Clear Init flag
   M[r0 + $vse_wrapper.vse_exop_struct.REINITFLAG_FIELD]=NULL;

#if defined(PATCH_LIBS)
   r9 = r0;
   LIBS_SLOW_SW_ROM_PATCH_POINT($vse_cap.VSE_PROCESSING_ASM.VSE_INITIALIZE.VSE_INITIALIZE.PATCH_ID_0, r7)     // cap_vse_patchers
   r0 = r9;
#endif
   // ***************************
   // Extract the sample rate
   // ***************************
   r7 = M[r0 + $vse_wrapper.vse_exop_struct.SAMPLE_RATE_FIELD];
   r9 = r1;

   vse_channel_loop:
      r8 = M[r9 + $vse_wrapper.vse_channels_struct.VSE_OBJECT_FIELD];
      // *******************************************************
      // Extract the current channel data_object
      // Set the sample_rate field for the current channel
      // *******************************************************
      M[r8 + $audio_proc.vse.FS] = r7;
      // ***********************************************************
      // call the vse.initialize function for the current channel
      // ***********************************************************
      call $audio_proc.vse.initialize;
      // *****************************
      // process next channel
      // *****************************
      r9 = M[r9 + $vse_wrapper.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
      if NZ jump vse_channel_loop;

   POP_ALL_C
   rts;

.ENDMODULE;

