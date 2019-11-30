// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change: 1141152 $  $DateTime: 2011/11/02 20:31:09 $
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Buffer Insert operator
//
// DESCRIPTION:
//    This operator keeps an cbops chain fed.
//
//    If the input is below the threshold, then insert data
//
// *****************************************************************************

#include "stack.h"
#include "cbops/cbops.h"
#include "cbuffer_asm.h"
#include "cbops_aec_ref_spkr_op_asm_defs.h"

#ifdef PATCH_LIBS
#include "patch/patch_asm_macros.h"
#endif

.MODULE $M.cbops.aec_ref_spkr_op;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
   
   // ** function vector **
   .VAR $cbops.aec_ref_spkr_op[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.aec_ref_spkr_op.pre_main,            // pre-main function
      &$cbops.aec_ref_spkr_op.amount_to_use,       // amount to use function
#ifndef CHIP_BASE_A7DA_KAS
      &$cbops.aec_ref_spkr_op.post_main;           // post-main function
#else
      0;
#endif

// Expose the location of this table to C
.set $_cbops_aec_ref_spkr_table , $cbops.aec_ref_spkr_op

// *****************************************************************************
// MODULE:
//   $cbops.insert_op.amount_to_use
//
// DESCRIPTION:
//   Get the amount to use, across all channels (it acts "in sync").
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    r0-r3, r5, r7
//
// *****************************************************************************

// Called before amount_to_use of graph 
$cbops.aec_ref_spkr_op.amount_to_use:
          
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_spkr_op.amount_to_use.PATCH_ID_0, r7)
#endif
          
   push rlink;
   
   // Get data pointer
   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
   // Get amount of input
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];    
   call $cbops.get_amount_ptr;
   r5 = M[r0];
   // Save data at input and make input large, will re-adjust for insertion
   M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.AMOUNT_DATA_FIELD] = r5;
   r5 = 0x7FFF;
   M[r0] = r5;

#ifndef CHIP_BASE_A7DA_KAS
   // Get first output index
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);
   r9 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r9];
   call $cbops.get_cbuffer;
   NULL = r0;
   if Z jump aec_ref_spkr_op.amount_to_use_done;

   // First Port (r0);
   r5 = NULL;
   call calc_dac_amount_of_data;
   // r2 is data in port (negative if wrap), r10 is max advance, r0 is adjustment
   // Limit amount of data after transfer to two times maximum advance
   r10 = M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.MAX_ADVANCE_FIELD];
   r3  = r10 ASHIFT 1;
   NULL = r2 + r0;
   if POS r1 = r3 - r2;
   if NEG r1 = Null;

   // Setup the limited transfer at output
   r0 = M[r8 + r9];
   call $cbops.get_amount_ptr;
   M[r0]=r1;
   if Z call $cbops.force_processing;
#endif

aec_ref_spkr_op.amount_to_use_done:
   pop rlink;
   rts;

// Called after amount to use of graph but before main processing of graph
$cbops.aec_ref_spkr_op.pre_main:
   push rLink;

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_spkr_op.pre_main.PATCH_ID_0, r6)
#endif

   // Get amount of input
   r6 = 0;
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];    
   call $cbops.get_amount_ptr;
   r10 = M[r0];
   if LE jump aec_ref_spkr_op.pre_main_done;

   // Continue processing
   r6 = 1;  

   // Get data pointer
   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // Get amount of data and threshold.   Determine if need to insert
   r3 = M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.AMOUNT_DATA_FIELD];
   NULL = r10 - r3;
   if LE jump aec_ref_spkr_op.pre_main_done;

   // Insufficient data, limit transfer to threshold.  Save reduced amount
   r2 = M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.IN_THRESHOLD_FIELD];
   r10 = MIN r2;
   M[r0] = r10;
   // Is there the reduced amount?
   r5 = r10 - r3;
   if LE jump aec_ref_spkr_op.pre_main_done; 

   // Insert (r5) zeroes, and update rd_wr pointer
   r0 = M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.NUM_INSERTS_FIELD];
   r0 = r0 + r5;
   M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.NUM_INSERTS_FIELD]=r0;

   // get number of input channels
   r3 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   I1 = r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;

   Words2Addr(r5);
   M3  = NULL - r5;
   Addr2Words(r5);

 zero_insert_channel:
   // Setup Input Buffer
   r0 = M[I1,0];
   call $cbops.get_buffer_address_and_length;
   I0 = r0;
   if Z jump next_zero_insert_channel;
      L0 = r1;
      push r2;
      pop B0;
      
      // Backup pointer and save
      r10 = r5,  r1 = M[I0,M3];
      r0 = M[I1,0];
      r1 = I0;
      call $cbops.set_buffer_address;
      // write zeroes
      r0 = NULL;
      do lp_copy;
           M[I0,MK1]=r0;
      lp_copy:

   next_zero_insert_channel:
    r0 = M[I1,MK1];
    r3 = r3 - 1;
    if GT jump zero_insert_channel;

    // Reset Buffering control
    L0 = 0;
    push NULL;
    pop B0;

aec_ref_spkr_op.pre_main_done:
   // If there is data, then do processing
   NULL = r6;
   if NZ call $cbops.force_processing;
   pop rlink;
   rts;

// unsigned get_aec_ref_cbops_inserts_total(cbops_op *op);
$_get_aec_ref_cbops_inserts_total:
    r0 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
    r1 = M[r0 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.WRAP_COUNT_FIELD];
    r0 = M[r0 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.NUM_INSERTS_FIELD];
    r0 = r0 + r1;
    rts;

// unsigned get_aec_ref_cbops_insert_op_insert_total(cbops_op *op);
$_get_aec_ref_cbops_insert_op_insert_total:
    r0 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
    r0 = M[r0 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.NUM_INSERTS_FIELD];
    rts;

// unsigned get_aec_ref_cbops_wrap_op_insert_total(cbops_op *op);
$_get_aec_ref_cbops_wrap_op_insert_total:
    r0 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
    r0 = M[r0 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.WRAP_COUNT_FIELD];
    rts;

#ifndef CHIP_BASE_A7DA_KAS
// Called after main processing of graph before buffer update
$cbops.aec_ref_spkr_op.post_main:

   // Check for Buffer Wrapping
    push rLink;

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_spkr_op.post_main.PATCH_ID_0, r9)
#endif

    // Get first output index
    r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
    Words2Addr(r9);
    r9 = r9 + r8;
    I4 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;

    // Get first buffer entry 
    r0 = M[I4,0];
    r3 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR (int);
    r3 = r3 + r4;

    // Get Cbuffer Ptr
    r0 = M[r3 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
    if Z jump $pop_rLink_and_rts;

#if !defined(CHIP_BASE_BC7)   
    // Get transfer amount (r5)
    r1 = M[r3 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
    r5 = M[r1];
#else
    r5 = NULL;
#endif
    r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD]; 
    r6 = M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.MAX_ADVANCE_FIELD];
    // Compute amount of data in source
    call calc_dac_amount_of_data;
    // r2 is number of samples in port, negative if overflow
    r10 = r6 - r2;
    if LE jump $pop_rLink_and_rts;

    // Increment Wrap count
    r1 = M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.WRAP_COUNT_FIELD];
    r1 = r1 + r10;    
    M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.WRAP_COUNT_FIELD]=r1;
    
    // setup amounts for insertions
    r5 = r5 + r10;
    r6 = r10;

    // Number of Ports
    r9 = M[r8 + $cbops.param_hdr.NR_OUTPUT_CHANNELS_FIELD];

    // Perform insertion
 process_channel:
    r0 = M[I4,0];
    call $cbops.get_buffer_address_and_length;
    I0 = r0;
    if Z jump process_channel_next;
      // Insert r6 zeros
      r10 = r6;
      L0 = r1;
      push r2;
      pop B0;
      r1  = Null;
      do lp_insert_loop;
         M[I0, MK1] = r1;
      lp_insert_loop:
      // Update amount (r5)
      r0 = M[I4,MK1];
      call $cbops.get_amount_ptr;
      M[r0]=r5;
    process_channel_next:
    r9 = r9 - 1;
    if GT jump process_channel;

    // Clear cicular buffer
    L0=NULL;
    push NULL;
    pop B0;

    pop rlink;
    rts;


// *****************************************************************************
// MODULE:
//   calc_dac_amount_of_data
//
// DESCRIPTION:
//   Calculate amount of data in port
//
//
// INPUTS:
//    - r0 = cbuffer structure
//    - r5 = amount transferred
//
// OUTPUTS:
//    - r2 = data words in port (size-space)
//      negative if a wrap occured.  Assumes that advance is
//      always less than half the buffer
//
// TRASHED REGISTERS:
//    r0,r1,r2
//
// *****************************************************************************
calc_dac_amount_of_data:
    push rLink;
    call $cbuffer.calc_amount_space_in_words;

#ifdef CHIP_BASE_HYDRA
    // Hydra: r2 is local buffer size in addr
    Addr2Words(r2);
    // Adjust space for amount written, buffer not port
    r0 = r0 - r5;
#else
    // BlueCore: r2 is MMU buffer size in bytes
    r2  = r2 LSHIFT -1;
#endif

    // r0 is space in port minus one
    r2  = r2 - r0;
    Null = r0 - r2;
    if NEG r2 = Null - r0;

    r0 = M[r7 + $cbops_aec_ref_spkr_op.aec_ref_op_struct.BUFFER_ADJ_FIELD];
    r2 = r2 - r0;
    // r2 is number of samples in port, negative if overflow
    pop rLink;
    rts;

#endif

.ENDMODULE;

