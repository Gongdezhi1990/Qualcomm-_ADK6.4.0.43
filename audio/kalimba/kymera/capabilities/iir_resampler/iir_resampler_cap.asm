/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "core_library.h"
#include "cbuffer_asm.h"

#include "portability_macros.h"
#include "stack.h"
#include "iir_resampler_private_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
//    void iir_resampler_reset_internal(IIR_RESAMPLER_OP_DATA* op_extra_data)
//
// MODULE:
//    $_iir_resampler_reset_internal
//
// DESCRIPTION:
//    reset/initialize the filter stages for each active channel
//    clear all history data and initializes history buffer pointers
//
// INPUTS:
//    - r0 = OPERATOR_EXTRA_DATA struct address
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    follows C calling convention
//
// *****************************************************************************
.MODULE $M.iir_resampler_reset_internal;
   .CODESEGMENT PM_FLASH;

$_iir_resampler_reset_internal:
   pushm <r4, r5, r6, r7, r8, rLink>;
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($iir_resampler_cap.IIR_RESAMPLER_CAP_ASM.IIR_RESAMPLER_RESET.IIR_RESAMPLER_RESET_INTERNAL.PATCH_ID_0, r6)     // iir_patchers
#endif
   
   
   pushm <I0, I2>;
   push M0;

   // iir_resampler active channel list head
   r6 = M[r0 + $iir_resampler_private.IIR_RESAMPLER_OP_DATA_struct.CHANNEL_LIST_FIELD];
   if Z jump done;

   // iir_resamplerv2 common config
   r7 = M[r0 + $iir_resampler_private.IIR_RESAMPLER_OP_DATA_struct.IIR_RESAMPLERV2_FIELD];

   // iir_resamplerv2 channel[0] struct pointer
   r8 = r7 + $iir_resampler_private.iir_resampler_internal_struct.CHANNEL_FIELD;

   // iir_resamplerv2 channel[0] working data block
   r4 = M[r7 + $iir_resampler_private.iir_resampler_internal_struct.WORKING_FIELD];

   // while(chan)
   next_channel:
      r0 = r7;                     // iir_resamplerv2 common
      call $reset_iir_resampler;

      // r4 has been updated to -> channel[n] working data block

      // iir_resamplerv2 channel[n] working data block
      r8 = r8 + ($iir_resampler_private.iir_resampler_channel_struct.STRUC_SIZE * ADDR_PER_WORD);

      // chan = chan->next
      r6 = M[r6 + $iir_resampler_private.OP_CHANNEL_LIST_struct.NEXT_FIELD];
   if NZ jump next_channel;

done:
   pop M0;
   popm <I0, I2>;
   popm <r4, r5, r6, r7, r8, rLink>;
   rts;

.ENDMODULE;

// *****************************************************************************
//    unsigned iir_resampler_amount_to_use(IIR_RESAMPLER_OP_DATA* op_extra_data, unsigned *process_all_input)
//
// MODULE:
//    $_iir_resampler_amount_to_use
//
// DESCRIPTION:
//    determine the number of input samples the resampler should consume.
//    this is based on the amount of data in the input buffers and amount
//    space in the output buffers. the resampler conversion ratio is used
//    to estimate how many samples may be consumed based on output space.
//    If any buffer is disconnected, channel_disconnected is set to 1.
//
// INPUTS:
//    - r0 = OPERATOR_EXTRA_DATA struct address
//    - r1 = process_all_input - will be set to FALSE when amount to use is 
//           constrained by limited space available in output buffer
//    - r2 = channel_disconnected - will be set to 1 when one of the channels
//           are disconnected
//
// OUTPUTS:
//    - r0 = amount to use
//
// TRASHED REGISTERS:
//    follows C calling convention
//
// *****************************************************************************
.MODULE $M.iir_resampler_amount_to_use;
   .CODESEGMENT PM_FLASH;

$_iir_resampler_amount_to_use:
   pushm <r4, r5, r6, r7, r8, r9, rLink>;
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($iir_resampler_cap.IIR_RESAMPLER_CAP_ASM.IIR_RESAMPLER_AMOUNT_TO_USE.IIR_RESAMPLER_AMOUNT_TO_USE.PATCH_ID_0, r6)     // iir_patchers
#endif
   
   
   push I2;
   push M0;

   // save pointer to process_all_input
   push r1;
   // set channel disconnected field to false
   r5 = r2;// r5 should not be crashed by  cbuffer.calc_amount_data/space_in_words;
   M[r5] = 0;

   // iir_resampler active channel list head
   r9 = M[r0 + $iir_resampler_private.IIR_RESAMPLER_OP_DATA_struct.AMT_TO_USE_CHANNEL_LIST_FIELD];

   r6 = MAXINT;         // min_data = MAXINT
   r7 = MAXINT;         // min_space = MAXINT;

   // pointer to iir_resamplerv2_common structure
   r8 = M[r0 + $iir_resampler_private.IIR_RESAMPLER_OP_DATA_struct.IIR_RESAMPLERV2_FIELD];

   // while(chan)
   next_channel:
      // if (chan->source != NULL)
      //   amount_space = cbuffer_calc_amount_space_in_words(chan->source)
      // else
      //   channel_disconnected = True
      //   amount_space = 0
      // if(min_space < min_space), min_space = amount_space
      // if(amount_space < min_space), min_space = amount_space
      r0 = M[r9 + $iir_resampler_private.OP_CHANNEL_LIST_struct.PTERMINAL_BUF_FIELD];
      if NZ jump calculate_amount_of_space;
      r0 = 0;
      M[r5] = r0; // channel_disconnected to true;
      jump calculate_min_of_space;
calculate_amount_of_space:
      call $cbuffer.calc_amount_space_in_words;
calculate_min_of_space:
      r7 = MIN r0;

      // if (chan->sink != NULL)
      //   amount_data = cbuffer_calc_amount_data_in_words(chan->sink);
      // else
      //   channel_disconnected = True
      //   amount_data = 0
      // if(amount_data < min_data), min_data = amount_data
      r0 = M[r9 + ($iir_resampler_private.OP_CHANNEL_LIST_struct.PTERMINAL_BUF_FIELD + 1*ADDR_PER_WORD)];
      if NZ jump calculate_amount_of_data;
      r0 = 0;
      M[r5] = r0; // channel_disconnected  to true;
      jump calculate_min_of_data;
calculate_amount_of_data:
      call $cbuffer.calc_amount_data_in_words;
calculate_min_of_data:
      r6 = MIN r0;

      // chan = chan->next
      r9 = M[r9 + $iir_resampler_private.OP_CHANNEL_LIST_struct.NEXT_FIELD];
   if NZ jump next_channel;

   // r6 is minimum available data
   // r7 is minimum available space

   // check for passthrough
   Null = M[r8 + $iir_resampler_private.iir_resampler_common_struct.FILTER_FIELD];
   if Z jump passthrough_mode;
      // if not passthrough, estimate how much data resampler can consume based
      // on available space
      r1 = r7;
      r0 = r8;
      call $estimate_iir_resampler_consumed;
      // r1 contains estimate
      jump compare_amount_data_to_space;

   passthrough_mode:
      // in passthrough mode, use min(amount_data, amount_space)
      r1 = r7;

   compare_amount_data_to_space:
   // process_all_input = TRUE
   // if(consumed_estimate < min_data)
   //    process_all_input = FALSE
   //    return consumed_estimate
   r3 = 1;
   r0 = r6;
   Null = r0 - r1;
   if GT r3 = Null;
   if Z   r0 = r1;

   // restore pointer and set process_all_input
   pop r1;
   if NZ M[r1] = r3;

   pop M0;
   pop I2;
   popm <r4, r5, r6, r7, r8, r9, rLink>;
   rts;

.ENDMODULE;

// *****************************************************************************
//    unsigned iir_resampler_processing(IIR_RESAMPLER_OP_DATA* op_extra_data, unsigned amount_to_use)
//
// MODULE:
//    $_iir_resampler_processing
//
// DESCRIPTION:
//    perform resampler processing on all active channels
//
// INPUTS:
//    - r0 = OPERATOR_EXTRA_DATA struct address
//    - r1 = number of input samples to consume
//
// OUTPUTS:
//    - r0 = number of output samples produced
//
// TRASHED REGISTERS:
//    follows C calling convention
//
// *****************************************************************************
.MODULE $M.iir_resampler_processing;
   .CODESEGMENT PM_FLASH;

   .CONST IIR_RESAMPLEV2     1*ADDR_PER_WORD;
   .CONST AMOUNT_TO_USE      2*ADDR_PER_WORD;
   .CONST CHAN               3*ADDR_PER_WORD;
   .CONST CHANNEL_PTR        4*ADDR_PER_WORD;
   .CONST LOCAL_DATA_SIZE    CHANNEL_PTR;

$_iir_resampler_processing:
   PUSH_ALL_C
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($iir_resampler_cap.IIR_RESAMPLER_CAP_ASM.IIR_RESAMPLER_PROCESSING.IIR_RESAMPLER_PROCESSING.PATCH_ID_0, r2)     // iir_patchers
#endif
   
   
   r2 = M[r0 + $iir_resampler_private.IIR_RESAMPLER_OP_DATA_struct.CHANNEL_LIST_FIELD];
   r0 = M[r0 + $iir_resampler_private.IIR_RESAMPLER_OP_DATA_struct.IIR_RESAMPLERV2_FIELD];

   // iir_resamplerv2 channel struct pointer
   r8 = r0 + $iir_resampler_private.iir_resampler_internal_struct.CHANNEL_FIELD;

   // save: iir_resamplev2, amount_to_use, chan, channel_ptr
   pushm <FP(=SP), r0, r1, r2, r8>;

   // iir_resamplerv2 channel[0] working data block
   r4 = M[r0 + $iir_resampler_private.iir_resampler_internal_struct.WORKING_FIELD];

   // r8 = iir_resamplerv2_channel pointer

   // while(chan)
   r9 = M[FP + CHAN];
   next_channel:

      // setup input buffers I1/L1/B1
      r0 = M[r9 + ($iir_resampler_private.OP_CHANNEL_LIST_struct.PTERMINAL_BUF_FIELD + 1*ADDR_PER_WORD)];
      call $cbuffer.get_read_address_and_size_and_start_address;
      push r2;
      pop B1;
      I1 = r0;
      L1 = r1;

      // setup output buffers I5/L5/B5
      r0 = M[r9 + $iir_resampler_private.OP_CHANNEL_LIST_struct.PTERMINAL_BUF_FIELD];
      call $cbuffer.get_write_address_and_size_and_start_address;
      push r2;
      pop B5;
      I5 = r0;
      L5 = r1;

      // iir_resamplerv2_common
      r0 = M[FP + IIR_RESAMPLEV2];

      // amount of input data to process
      r10 = M[FP + AMOUNT_TO_USE];

      call $iir_perform_resample;

      // r4 has been updated to -> channel[n] working data block
      // r7 = amount_produced

      // increment iir_resamplerv2_channel pointer
      r8 = M[FP + CHANNEL_PTR];
      r8 = r8 + ($iir_resampler_private.iir_resampler_channel_struct.STRUC_SIZE * ADDR_PER_WORD);
      M[FP + CHANNEL_PTR] = r8;

      // chan = chan->next
      r9 = M[FP + CHAN];
      r9 = M[r9 + $iir_resampler_private.OP_CHANNEL_LIST_struct.NEXT_FIELD];
      M[FP + CHAN] = r9;
   if NZ jump next_channel;

   SP = SP - LOCAL_DATA_SIZE;
   pop FP;

   // return amount produced (r7) (should be same across all channels)
   r0 = r7;

   POP_ALL_C
   rts;

.ENDMODULE;

