// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbuffer_asm.h"
#include "cbops_iir_resamplerv2_op_asm_defs.h"
#include "iir_resamplerv2_common_asm_defs.h"

// *****************************************************************************
// NAME:
//    IIR resampler cbops operator
//
// DESCRIPTION:
//    This operator uses an IIR and a FIR filter combination to
//    perform sample rate conversion.  The utilization of the IIR
//    filter allows a lower order FIR filter to be used to obtain
//    an equivalent frequency response.  The result is that the
//    IIR resampler uses less MIPs than the current polyphase FIR method.
//    It also provides a better frequency response.
//
//    To further reduce coefficients for a given resampling up to two
//    complete filter stages are supported.  The configurations include.
//          IIR --> FIR(10)
//          FIR(10) --> IIR
//          FIR(6) --> FIR(10) --> IIR
//          FIR(6) --> IIR --> FIR(10)
//          IIR --> FIR(10) --> IIR --> FIR(10)
//          IIR --> FIR(10) --> FIR(10) --> IIR
//          FIR(10) --> IIR --> IIR --> FIR(10)
//          FIR(10) --> IIR --> FIR(10) --> IIR
//
//    The IIR filter may be from 9th order to 19 order.
//
//    The FIR filters are implemented in a polyphase configuration. The FIR(6)
//    filter uses a 6th order polyphase kernal and the FIR(10) filter uses a
//    10th order polyphase kernal.  The filters are symetrical so only half the
//    coefficients need to be stored.
//
//    The operator utilizes its own history buffers.  As a result the input and/or
//    output may be a port.  Also, for downsampling, in-place operation is supported.
//
// *****************************************************************************

// Private Library Exports
.PUBLIC $cbops_iir_resamplev2;

.MODULE $M.cbops.iir_resamplev2;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops_iir_resamplev2[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,         // reset function
      &$cbops.iir_resamplev2.amount_to_use,  // amount to use function
      &$cbops.iir_resamplev2.main;           // main function

.ENDMODULE;

// Simplify access to common_struct
#define RSMPL_FILTER_PTR_OFFSET        ($cbops_iir_resamplerv2_op.iir_resampler_op_struct.COMMON_FIELD+$iir_resamplerv2_common.iir_resampler_common_struct.FILTER_FIELD)
#define RSMPL_RESET_OFFSET             ($cbops_iir_resamplerv2_op.iir_resampler_op_struct.COMMON_FIELD+$iir_resamplerv2_common.iir_resampler_common_struct.RESET_FLAG_FIELD)

// *****************************************************************************
// MODULE:
//    $cbops_iir_resamplev2.amount_to_use
//
// DESCRIPTION:
//    operator amount_to_use function for IIR resampler operator
//
// INPUTS:
//    - r5 = the minimum of the number of input samples available and the
//      amount of output space available
//    - r6 = the number of input samples available
//    - r7 = the amount of output space available
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    none (r5-r8) preserved
//
// TRASHED REGISTERS:
//    r0, r1, r2, r10, I0, I2
//
// *****************************************************************************
.MODULE $M.cbops_iir_resamplev2.amount_to_use;
   .CODESEGMENT CBOPS_IIR_RESAMPLEV2_OPERATOR_PM;


$cbops.iir_resamplev2.amount_to_use:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.iir_resamplev2.amount_to_use.PATCH_ID_0, r9)
#endif

   // get to the start of COMMON parameters (after the nr in/out and index table "header" part)
   r10 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // Get resampling configuration.  If NULL perform passthrough operation
   // The RSMPL_FILTER_PTR_OFFSET will get us to filter ptr field inside common params. NOTE that now in multichannel
   // generalisation the cbops_iir_resamplerv2_op.iir_resampler_op_struct.COMMON_FIELD offset is zero and we
   // above did the "skipping" of header part to get to the params.
   NULL = M[r10 + $cbops_iir_resamplerv2_op.iir_resampler_common_struct.FILTER_FIELD];
   if Z jump $cbops.basic_multichan_amount_to_use;

   push rlink;

   // get number of input channels - transform it into addresses, so that we don't do the latter
   // for every channel. We can rely on the fact that in and out channels are of same number.
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);

   // Using spaec at output get amount of data to consume
   r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r0];     // Output index (first channel)
   call $cbops.get_amount_ptr;
   r1 = M[r0];		// available space
   r0 = r10;		// COMMON parameters
   call $estimate_iir_resampler_consumed;
   
   // Get data at input, r1 is amount to consume
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index (first channel)
   call $cbops.get_amount_ptr;

   r5 = M[r0];
   // r5 = MIN consume and data at input, save as transfer
   r5 = MIN r1;
   M[r0]=r5;

   // Check Reset flag and if NULL reset.
   Null = M[r10 + $cbops_iir_resamplerv2_op.iir_resampler_common_struct.RESET_FLAG_FIELD];
   if NZ jump $pop_rLink_and_rts;

   // Push op struct ptr
   push r8;
   push r10;   // common patameters Ptr M[SP-1]

   // History Buffers appended to end of data structure. Must work out location for each channel, the data is
   // structured as [common][ptr to history[[channel1]...[channelN]
   // Start of working1 is at offset N*sizeof(channel params) relative to start of channel-specific part (that comes after the common part
   // and the index header in multichannel scheme). COMMON_FIELD == 0 for multichannel case as it stands.

   // Offset of first channel's param block
   r8 = r10 + $cbops_iir_resamplerv2_op.iir_resampler_op_struct.CHANNEL_FIELD;
   // Pointer to first channel's working data block (it is actually allocated after the channel parameters)
   r4 = M[r10 + $cbops_iir_resamplerv2_op.iir_resampler_op_struct.WORKING_FIELD];

   /* there might be some channels that fall out of use and will not actually be processed later. Don't reset those, save tedium. */
  reset_channel:
      r0 = M[SP - 1*ADDR_PER_WORD];


      // r0=common config,r4=working buffer,r8 = channel struct
      call $reset_iir_resampler;

      // r4 is working buffer for next channel, so put it to use - and also advance channel params ptr to
      // the next channel's param block.
      r8 = r8 + ($cbops_iir_resamplerv2_op.iir_resampler_channel_struct.STRUC_SIZE * ADDR_PER_WORD);

      // countdown - r9 is in addr per word units already
      r9 = r9 - 1*ADDR_PER_WORD;
   if GT jump reset_channel;

   // restore op struct ptr and close shop
   pop r10;
   pop r8;
   jump $pop_rLink_and_rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops_iir_resamplev2.main
//
// DESCRIPTION:
//    operator main function for IIR resampler operator
//
// INPUTS:
//    - r4 = pointer to internal framework object
//    - r5 = pointer to the list of input and output buffer start addresses (base_addr)
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure:
//    - r10 = data to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    everything
//
// *****************************************************************************
.MODULE $M.cbops_iir_resamplev2.main;      // Upsample
   .CODESEGMENT CBOPS_IIR_RESAMPLEV2_OPERATOR_PM;
   .DATASEGMENT DM;

    // just to really make anyone realise one has to change the offsets if changed stack pile, so avoid magic numbers
   .CONST $cbops_iir_resamplev2.BUFFERS_PTRS                   2*ADDR_PER_WORD;                                           // r4
   .CONST $cbops_iir_resamplev2.PARAM_STRUCT_ADDR              $cbops_iir_resamplev2.BUFFERS_PTRS      + ADDR_PER_WORD;   // r8

   .CONST $cbops_iir_resamplev2.AMOUNT_TO_PROCESS              $cbops_iir_resamplev2.PARAM_STRUCT_ADDR + ADDR_PER_WORD;  
   .CONST $cbops_iir_resamplev2.COMMON_PTR                     $cbops_iir_resamplev2.AMOUNT_TO_PROCESS + ADDR_PER_WORD;  
   .CONST $cbops_iir_resamplev2.WORKING_PTR                    $cbops_iir_resamplev2.COMMON_PTR        + ADDR_PER_WORD;  
   
   .CONST $cbops_iir_resamplev2.LOCAL_SIZE		               $cbops_iir_resamplev2.WORKING_PTR - $cbops_iir_resamplev2.PARAM_STRUCT_ADDR; 

$cbops.iir_resamplev2.main:
   pushm <FP(=SP), rLink>;

#ifdef PATCH_LIBS
    /* r0 and r1 can get treashed here, based on next call
     * to get_transfer_and_update_multi_channel function
     */
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.iir_resamplev2.main.PATCH_ID_0, r1)
#endif

   call $cbops.get_transfer_and_update_multi_channel;
   // r0=transfer, r9=num_channels
   r10 = r0;
   if LE jump jp_do_nothing;
   
   pushm <r4,r8>;   // save cbops fw ptrs, buffer start, rd/wr and size table ptrs, op struct ptr
   SP = SP + $cbops_iir_resamplev2.LOCAL_SIZE;

   // Setup pointers to channel indices
   I0 = r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD; // input index ptr
   I2 = I0 + r9;					 // output index ptr

   // Save input to consume
   M[FP + $cbops_iir_resamplev2.AMOUNT_TO_PROCESS] = r10;

   // save COMMON pointer
   r0 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
   M[FP + $cbops_iir_resamplev2.COMMON_PTR] = r0;
   
   // Pointer to first channel's working data block (it is actually allocated after the channel parameters)
   r4 = M[r0 + $cbops_iir_resamplerv2_op.iir_resampler_op_struct.WORKING_FIELD];
   M[FP + $cbops_iir_resamplev2.WORKING_PTR] = r4;

   // Offset of first channel's param block
   r8 = r0 + $cbops_iir_resamplerv2_op.iir_resampler_op_struct.CHANNEL_FIELD;

      
 process_channel:
      M0 = MK1;

      // Setup input & output
      r4 = M[FP + $cbops_iir_resamplev2.BUFFERS_PTRS];
      r0 = M[I0,M0];                // get the input index
      call $cbops.get_buffer_address_and_length;
      I1 = r0, r0 = M[I2,M0];       // r0 = output index
      if Z jump next_channel;
      L1 = r1;
      push r2;
      pop B1;
      
      call $cbops.get_buffer_address_and_length;
      I5 = r0;
      if Z jump next_channel;
      L5 = r1;
      push r2;
      pop B5;

      pushm <r8, r9>;
      pushm <I0, I2>;
      
      // r8 = start of this channel's param block, I1,L1,B1=input, I5,L5,B5=output 
      r0  = M[FP + $cbops_iir_resamplev2.COMMON_PTR];
      r4  = M[FP + $cbops_iir_resamplev2.WORKING_PTR];
      r10 = M[FP + $cbops_iir_resamplev2.AMOUNT_TO_PROCESS];
      
      call $iir_perform_resample;

      M[FP + $cbops_iir_resamplev2.WORKING_PTR] = r4;

      popm <I0, I2>;
      popm <r8, r9>;

 next_channel:
   // get to next channel's param block in r8
   r8 = r8 + ($cbops_iir_resamplerv2_op.iir_resampler_channel_struct.STRUC_SIZE * ADDR_PER_WORD);

   // update channel counter
   r9 = r9 - 1*ADDR_PER_WORD;
   if GT jump process_channel;

   // Record amount produced (r7) - this is now same across all channels, so taking just last one
   SP = SP - $cbops_iir_resamplev2.LOCAL_SIZE;
   popm <r4,r8>;

   // Update amount produced by channels
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);
   r5 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r5];     // Output Index (first channel);
   call $cbops.get_amount_ptr;
   M[r0] = r7;

jp_do_nothing:
   popm <FP, rLink>;
   rts;

.ENDMODULE;
// Expose the location of this table to C
.set $_cbops_iir_resampler_table,  $cbops_iir_resamplev2








