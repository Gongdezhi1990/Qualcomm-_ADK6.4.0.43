// *****************************************************************************
//  Copyright (c) 2015 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//  Notifications and licenses are retained for attribution purposes only
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM function for compander operator data processing
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "core_library.h"
#include "cbuffer_asm.h"
#include "portability_macros.h"
#include "compander.h"
#include "compander_gen_asm.h"
#include "stack.h"
#include "compander_wrapper_defs.h"
#include "compander_wrapper_asm_defs.h"

#ifdef KYMERA
#include "pmalloc/pl_malloc_preference.h"
#else
#define MALLOC_PREFERENCE_DM1 1
#define MALLOC_PREFERENCE_DM2 2
#endif

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $_compander_processing
// extern unsigned compander_processing(COMPANDER_OP_DATA *op_data, unsigned samples_to_process, compander_channels *first);
//
// DESCRIPTION:
//    Data processing function.
//
// INPUTS:
//    - r0 = compander "extra data" object
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
.MODULE $M.compander_proc;
    .CODESEGMENT PM;
$_compander_processing:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($compander_cap.COMPANDER_PROCESSING_ASM.COMPANDER_PROC.COMPANDER_PROCESSING.PATCH_ID_0, r3)     // cap_compander_patchers
#endif
   // Check Bypass flag
   r3 = M[r0 + ($compander_wrapper.compander_exop_struct.COMPANDER_CAP_PARAMS_FIELD + $M.COMPANDER.PARAMETERS.OFFSET_COMPANDER_CONFIG)];
   Null = $M.COMPANDER.CONFIG.BYPASS AND r3;
   if NZ jump cmpd_pass_through;

   r3 = M[r0 + $compander_wrapper.compander_exop_struct.CUR_MODE_FIELD];
   // *****************************
   // check pass-through mode
   // *****************************
   Null = r3 - $M.COMPANDER.SYSMODE.PASS_THRU;
   if Z jump cmpd_pass_through;

   // *****************************
   // check mute mode
   // *****************************
   Null = r3 - $M.COMPANDER.SYSMODE.MUTE;
   if Z jump cmpd_mute;

   // *****************************************************************************
   // for now, we assume we get to the buffer parameters directly with some offset constants
   // *****************************************************************************
   PUSH_ALL_C

   // **********************************************************
   // Extract the pointer to the first channel data_object
   // **********************************************************
   r8 = M[r2 + $compander_wrapper.compander_channels_struct.COMPANDER_OBJECT_FIELD];

   // *******************************************
   // Set the number of "samples_to_process"
   // *******************************************
   M[r8 + $audio_proc.compander.SAMPLES_TO_PROCESS] = r1;

   // ***********************************
   // call the compander process function
   // ***********************************
   push r2;    // chan_ptr
   call $audio_proc.compander.stream_process;
   pop  r0;    // chan_ptr

   // *******************************************************************************************
   // Advance buffer pointers (r0=channel list, r1=amount )
   // *******************************************************************************************
   r8 = M[r0 + $compander_wrapper.compander_channels_struct.COMPANDER_OBJECT_FIELD];
   r1 = M[r8 + $audio_proc.compander.SAMPLES_TO_PROCESS];
   r2 = r1;     //same number of samples produced as consumed
   if NZ call $_multi_channel_advance_buffers;

   // return amount processed
   r0 = M[r8 + $audio_proc.compander.SAMPLES_TO_PROCESS];

   POP_ALL_C
   rts;

cmpd_pass_through:
   // r2 = first channel, r1=amount
   push r1;
   push rlink;
   r0 = r2;
   r2 = NULL;  // pass-through
   call $_multi_channel_copy_mute;
   pop rlink;
   pop r0;
   rts;

cmpd_mute:
   // r2 = first channel, r1=amount
   push r1;
   push rlink;
   r0 = r2;
   r2 = 1;  // mute
   call $_multi_channel_copy_mute;
   pop rlink;
   pop r0;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_compander_initialize
//  extern void compander_initialize(COMPANDER_OP_DATA *op_data, unsigned chan_mask, compander_channels *first);
//
// DESCRIPTION:
//    Initialize function
//
// INPUTS:
//    - r0 = compander "extra data" object
//    - r1 = mask of active channels
//    - r2 = pointer to first active channel
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.compander_initialize;
    .CODESEGMENT PM;
$_compander_initialize:

   PUSH_ALL_C
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($compander_cap.COMPANDER_PROCESSING_ASM.COMPANDER_INITIALIZE.COMPANDER_INITIALIZE.PATCH_ID_0, r6)     // compander_patchers1
#endif

   r6 = r2; // active channels
   if Z jump compander_initialize_done;

   // *********************
   // clear reinit flag
   // *********************
   M[r0 + $compander_wrapper.compander_exop_struct.REINITFLAG_FIELD] = Null;

   // *****************************************************************
   // initialize "lookahead_status" flag to zero(lookahead enabled)
   // *****************************************************************
   M[r0 + $compander_wrapper.compander_exop_struct.LOOKAHEAD_STATUS_FIELD] = Null;

   // Count channels (r3) (maximum of 8 channels)
   r1 = r1 AND 0xFF;
   r3 = ONEBITCOUNT r1;

   // Sample rate
   r2 = M[r0 + $compander_wrapper.compander_exop_struct.SAMPLE_RATE_FIELD];

   // ********************************************************
   // calculate lookahead history buffer size (in samples)
   // ********************************************************
   r1 = r0 + $compander_wrapper.compander_exop_struct.COMPANDER_CAP_PARAMS_FIELD;
   r4 = M[r1 + $audio_proc.compander.parameter.LOOKAHEAD_TIME];
   r4 = r4 * r2;
   // Actual size used is one more word than (time * rate)
   // See use of LOOKAHEAD_SAMPLES in $audio_proc.cmpd.final_gain_apply
   r4 = r4 + 1;

   // ******************************************************
   //  Set pointer to all data_objects in the first channel
   // ******************************************************
   r1 = r0 + $compander_wrapper.compander_exop_struct.COMPANDER_OBJECT_FIELD;
   r8 = M[r6 + $compander_wrapper.compander_channels_struct.COMPANDER_OBJECT_FIELD];
   M[r8 + $audio_proc.compander.DATA_OBJECTS_PTR] = r1;

   r9 = r0; // ext data ptr

   compander_channel_init_loop:
      // r2 = sample rate, r3 = num_channels, r4=look ahead, r6=chan_ptr, r9=ext data ptr, r1=object array

      r8 = M[r6 + $compander_wrapper.compander_channels_struct.COMPANDER_OBJECT_FIELD];

      // Save compander object in array (r1)
      M[r1]=r8;
      r1 = r1 + ADDR_PER_WORD;

      M[r8 + $audio_proc.compander.SAMPLE_RATE_FIELD] = r2;
      M[r8 + $audio_proc.compander.NUM_CHANNELS]      = r3;

      pushm <r1,r2,r3,r4,r6,r9>;

      // ********************************************************************************
      // allocate and set lookahead history buffer pointer for the current data_object
      // ********************************************************************************
      r0 = M[r8 + $audio_proc.compander.START_LOOKAHEAD_HIST];
      if Z jump lookahead_buf_alloc;
         call $_pfree;
         M[r8 + $audio_proc.compander.START_LOOKAHEAD_HIST] = Null;
      lookahead_buf_alloc:
      r0 = r4 * ADDR_PER_WORD(int);
      if Z jump init_func_call;
         r1 = MALLOC_PREFERENCE_DM1;
         call $_xzppmalloc;
         M[r8 + $audio_proc.compander.START_LOOKAHEAD_HIST] = r0;
#ifdef DEBUG_ON
         if Z call $error;
#else
         if NZ jump init_func_call;
            r7 = M[r6 + $compander_wrapper.compander_channels_struct.CHAN_MASK_FIELD];
            r2 = M[r9 + $compander_wrapper.compander_exop_struct.LOOKAHEAD_STATUS_FIELD];   // load "lookahead_status" flag
            r2 = r2 OR r7;                                                                  // update "lookahead_status" flag
            M[r9 + $compander_wrapper.compander_exop_struct.LOOKAHEAD_STATUS_FIELD] = r2;   // write "lookahead_status" flag
#endif
      init_func_call:

      call $audio_proc.cmpd.initialize;

      popm <r1,r2,r3,r4,r6,r9>;

      // next chan
      r6 = M[r6 + $compander_wrapper.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
      if NZ jump compander_channel_init_loop;

compander_initialize_done:

   POP_ALL_C
   rts;

.ENDMODULE;



