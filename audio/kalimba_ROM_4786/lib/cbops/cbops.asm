// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Cbops operators Library
//
// DESCRIPTION:
//    This library provides routines to copy data between buffers/ports
//    while doing some form of processing. The processing is defined with
//    operator functions which are held in a linked list.  Hence it is possible
//    to choose which functions are used and easily add other custom functions
//    as needed.
// *****************************************************************************

#ifdef DEBUG_ON
/* TODO take this out, there seems to be a stack corruption in profiler.stop,
    is looping on rts
   #define ENABLE_PROFILER_MACROS
 */
   #define CBOPS_DEBUG
#endif

#ifdef RICK
   #define DUMMY_READ_FOR_24BIT_CLEANUP
#endif



#include "profiler.h"
#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif
#include "cbops.h"
#include "cbops_c_asm_defs.h"
#include "panicids_asm_defs.h"
#include "stack.h"      // for PUSH_ALL_C

// *****************************************************************************
// MODULE:
//    $cbops.reset
//
// DESCRIPTION:
//    Call the reset function, if there is one, for each operator in the linked
//    list.
//
// INPUTS:
//    - r8 = pointer to cbop_graph structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    everything
//
// NOTES:
//      Also note, $cbops.reset is not yet used in kymera, although there are
//      some functions like initialise_buffer_tables and the higher level
//      cbops_mgr_buffer_reinit, cbops_mgr_update_buffer_info (in cbops_mgr),
//      that could do with some consolidation TODO
//
// *****************************************************************************
.MODULE $M.cbops.reset;
   .CODESEGMENT CBOPS_RESET_PM;
   .DATASEGMENT DM;

#ifdef CBOPS_DEBUG
   .VAR count;
#endif


   $cbops.reset:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.cbops_reset.PATCH_ID_0, r1)
#endif

   push rLink;

   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_reset[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_reset;
      call $profiler.start;
   #endif


   // ** calc amount of data to use **
   // get the operator structure address
   r8 = M[r8 + $cbops_c.cbops_graph_struct.FIRST_FIELD];

   #ifdef CBOPS_DEBUG
      r1 = $cbops.MAX_OPERATORS;
      M[count] = r1;
   #endif


   // ** operators must preserve r8 **
   reset_loop:

      #ifdef CBOPS_DEBUG
         // check we haven't looped more times than $cbops.MAX_OPERATORS
         r1 = M[count];
         r1 = r1 - 1;
         M[count] = r1;
         // linked list corrupt or num_operators>MAX_OPERATORS
         if NEG call $error;
      #endif

      // load the function address of the "reset" function
      r0 = M[r8 + $cbops_c.cbops_op_struct.FUNCTION_VECTOR_FIELD];
      // load the parameter area for the operator
      r8 = r8 + $cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD;
      r0 = M[r0 + $cbops_c.cbops_functions_struct.RESET_FIELD];

      // if it isn't zero then call the reset function
      if NZ call r0;

      // load the next operator structure
      r8 = M[r8 + ($cbops_c.cbops_op_struct.NEXT_OPERATOR_ADDR_FIELD - $cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD)];
   if NZ jump reset_loop;


   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_copy;
      call $profiler.stop;
   #endif

   pop rLink;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_cbops_process_data
//
// DESCRIPTION:
//    This function executes the cbops graph. Calculating how much to process
//    from the input and then instigating the processing.
//
// INPUTS:
//    - r0 = pointer to cbops_graph to process:
//    - r1 = maximum amount to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    C calling convention respected
//
// *****************************************************************************
.MODULE $M.cbops.cbops_process_data;
   .CODESEGMENT CBOPS_COPY_PM;
   .DATASEGMENT DM;

.CONST $CBOPS_LOCAL_BUFFER_PTR	1*ADDR_PER_WORD;
.CONST $CBOPS_LOCAL_GRAPH_PTR	2*ADDR_PER_WORD;
.CONST $CBOPS_LOCAL_SIZE	2*ADDR_PER_WORD;

.CONST $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR $cbops_c.cbops_buffer_struct.STRUC_SIZE*ADDR_PER_WORD;
#ifdef CBOPS_DEBUG
   .VAR count;
   .VAR fw_semaphore;
   .VAR concurrency_flag;
#endif

$_cbops_process_data:

   PUSH_ALL_C

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbops.cbops_process_data.PATCH_ID_0, r4)
#endif

#ifdef CBOPS_ASSERT_ON
   Null = r1 - $cbops.MAX_COPY_SIZE;
   if POS call $error;
#endif

   pushm <r0, r1>;
   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_copy[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_copy;
      call $profiler.start;
   #endif

#ifdef CBOPS_DEBUG
    // prove concurrent chains, take semaphore
    r0 = M[fw_semaphore];
    if Z jump ok_fw_sema;
        // raise flag to prove concurrency concurrency
        Null = r0 - M[concurrency_flag];
        if LE jump ok_fw_sema;
            M[concurrency_flag] = r0;
    ok_fw_sema:
    r0 = r0 + 1;
    M[fw_semaphore] = r0;
#endif

    popm <r0, r1>;
    r6  = r1;
    r8  = r0 + $cbops_c.cbops_graph_struct.BUFFERS_FIELD;
    r10 = M[r0 + $cbops_c.cbops_graph_struct.NUM_IO_FIELD];
    // Push local variable onto stack
    push r0;   // Graph*              : M[SP-$CBOPS_LOCAL_GRAPH_PTR]
    push r8;   // Ptr to Buffer Table : M[SP-$CBOPS_LOCAL_BUFFER_PTR]

    /* Check if buffers need refreshing */
    r5 = M[r0 + $cbops_c.cbops_graph_struct.REFRESH_BUFFERS_FIELD];
    if Z jump jp_buf_reset_done;
      /* Update Buffer Table (r8) */
      do lp_get_buffers;
         // Get Buffer
         r0 = M[r8 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
         if Z jump jp_get_next_buffer;
         // Get Buffer Type
         r7 = M[r8 + $cbops_c.cbops_buffer_struct.TYPE_FIELD];
         NULL = r7 AND $cbops_c.CBOPS_IO_SOURCE;
         if NZ jump jp_get_source_buf;
            // Output (sink) or Internal Buffer
            call $cbuffer.get_write_address_and_size_and_start_address;
            jump jp_get_next_buffer;
         jp_get_source_buf:
            // Get Input buffer (source)
            call $cbuffer.get_read_address_and_size_and_start_address;
            // Signal valid source
            r5 = NULL;
         jp_get_next_buffer:
            // Save buffer info
            M[r8 + $cbops_c.cbops_buffer_struct.BASE_FIELD]   = r2;      // store the start address
            M[r8 + $cbops_c.cbops_buffer_struct.SIZE_FIELD]   = r1;      // store the buffer size
            M[r8 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD] = r0;      // store the rd_wr address
            // Go to next buffer
            r8 = r8 + $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR;
       lp_get_buffers:

       r0  = M[SP - $CBOPS_LOCAL_GRAPH_PTR];
       /* Did we find a valid source?  If not abort.   This might happen
         with changes to an ep sync.  Otherwise, clear refresh signal */
       M[r0 + $cbops_c.cbops_graph_struct.REFRESH_BUFFERS_FIELD]=r5;
       if NZ jump jp_processing_done;

       /* Reset cbops pointer, buffer table pointer, and num_io */
       r8  = M[SP-$CBOPS_LOCAL_BUFFER_PTR];
       r10 = M[r0 + $cbops_c.cbops_graph_struct.NUM_IO_FIELD];
    jp_buf_reset_done:

    /* Reset amounts to maximum */
    I0 = r8 + $cbops_c.cbops_buffer_struct.TRANSFER_AMOUNT_FIELD;
    M3 = $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR;
    r1 = 0x7FFF;
    do lp_init_amounts;
         M[I0,M3]=r1;
    lp_init_amounts:

    /* Update amounts (r8) */
    r10 = M[r0 + $cbops_c.cbops_graph_struct.NUM_IO_FIELD];
    do lp_get_amounts;
      // Get cBuffer
      r0 = M[r8 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
      if Z jump jp_get_next_amount;
      // Get Buffer Type
      r7 = M[r8 + $cbops_c.cbops_buffer_struct.TYPE_FIELD];
      NULL = r7 AND $cbops_c.CBOPS_IO_SOURCE;
      if NZ jump jp_get_source;
      NULL = r7 AND $cbops_c.CBOPS_IO_INTERNAL;
      if NZ jump jp_get_internal;
         // Get amount of Space
         call $cbuffer.calc_amount_space_in_words;
         // r0 is space in buffer
         jump jp_update_amount;
      jp_get_internal:
         // If internal use size in samples
         r1 = M[r8 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
         r0 = r1 ASHIFT -LOG2_ADDR_PER_WORD;
         jump jp_update_amount;
      jp_get_source:
         // Get amount of data
         call $cbuffer.calc_amount_data_in_words;
         // r0 is amount of data, limit input to max data
         r0 = MIN r6;
      jp_update_amount:
         // r0 is amount in buffer
         // linked buffer,  Transfer is a pointer
         r1 = M[r8 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
         // Set minimum of linked buffer and amount
         r2 = M[r1];
         r0 = MIN r2;
         // r1 is address of transfer, r0 is amount.   Save amount
         M[r1] = r0;
      jp_get_next_amount:
         r8 = r8 + $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR;
    lp_get_amounts:


    r0 = M[SP - $CBOPS_LOCAL_GRAPH_PTR];
    M[r0 + $cbops_c.cbops_graph_struct.FORCE_UPDATE_FIELD] = NULL;

    /* Pre-amount to use for graph */
    r1 = M[r0 + ($cbops_c.cbops_graph_struct.OVERRIDE_FUNCS_FIELD + $cbops_c.cbops_functions_struct.AMOUNT_TO_USE_FIELD)];
    if Z jump jp_amount_override_done;
      // Buffer array in (r4)
      r4 = M[SP - $CBOPS_LOCAL_BUFFER_PTR];
      // load the parameter area for the operator
      r8 = M[r0 + $cbops_c.cbops_graph_struct.OVERRIDE_DATA_FIELD];
      call r1;
      r0 = M[SP - $CBOPS_LOCAL_GRAPH_PTR];
   jp_amount_override_done:

    // Call amount to use functions of operators starting with
    // last operator and going backwards

    r8 = M[r0 + $cbops_c.cbops_graph_struct.LAST_FIELD];
    #ifdef CBOPS_DEBUG
      r0 = $cbops.MAX_OPERATORS;
      M[count] = r0;
    #endif // CBOPS_DEBUG
    amount_to_use_loop:
      #ifdef CBOPS_DEBUG
         // check we haven't looped more times than $cbops.MAX_OPERATORS
         r1 = M[count];
         r1 = r1 - 1;
         M[count] = r1;
         // linked list corrupt or num_operators>MAX_OPERATORS
	 if NEG call $error;
      #endif // CBOPS_DEBUG
      // Buffer array in (r4)
      r4 = M[SP - $CBOPS_LOCAL_BUFFER_PTR];
      // load the function address of the "amount of data to use" function
      r0 = M[r8 + $cbops_c.cbops_op_struct.FUNCTION_VECTOR_FIELD];
      // load the parameter area for the operator
      r8 = r8 + $cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD;
      // if it isn't NULL then call the amount_to_use function
      r0 = M[r0 + $cbops_c.cbops_functions_struct.AMOUNT_TO_USE_FIELD];
      if NZ call r0;
      #ifdef CBOPS_ASSERT_ON
         r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index (first channel)
         call $cbops.get_amount_ptr;
         Null = r0 - $cbops.MAX_COPY_SIZE;
         if POS call $error;
      #endif

      // load the next operator structure
      r8 = M[r8 + ($cbops_c.cbops_op_struct.PREV_OPERATOR_ADDR_FIELD - $cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD)];
      if NZ jump amount_to_use_loop;

   r0 = M[SP - $CBOPS_LOCAL_GRAPH_PTR];

   /* Post-amount to use for graph */
   r1 = M[r0 + ($cbops_c.cbops_graph_struct.OVERRIDE_FUNCS_FIELD + $cbops_c.cbops_functions_struct.RESET_FIELD)];
   if Z jump jp_post_amount_override_done;
      // Buffer array in (r4)
      r4 = M[SP - $CBOPS_LOCAL_BUFFER_PTR];
      // load the parameter area for the operator
      r8 = M[r0 + $cbops_c.cbops_graph_struct.OVERRIDE_DATA_FIELD];
      call r1;
      r0 = M[SP - $CBOPS_LOCAL_GRAPH_PTR];
      NULL = M[r0 + $cbops_c.cbops_graph_struct.FORCE_UPDATE_FIELD];
      if Z jump jp_processing_done;
   jp_post_amount_override_done:

   // Check if anything to process. Operator may force processing
   r1 = M[r0 + $cbops_c.cbops_graph_struct.FORCE_UPDATE_FIELD];
   if NZ jump jp_continue_processing;
      // Check for a non-zero input buffer
      r10 = M[r0 + $cbops_c.cbops_graph_struct.NUM_IO_FIELD];
      r8  = r0 + $cbops_c.cbops_graph_struct.BUFFERS_FIELD;
      do lp_check_input;
         r7 = M[r8 + $cbops_c.cbops_buffer_struct.TYPE_FIELD];
         NULL = r7 - $cbops_c.CBOPS_IO_SOURCE;
         if NZ jump jp_check_cont;
            NULL = M[r8 + $cbops_c.cbops_buffer_struct.TRANSFER_AMOUNT_FIELD];
            if NZ jump jp_continue_processing;
         jp_check_cont:
         r8 = r8 + $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR;
      lp_check_input:
      // No available input
      jump jp_processing_done;

jp_continue_processing:
   // get the structure address to get address of the first structure
   r8 = M[r0 + $cbops_c.cbops_graph_struct.FIRST_FIELD];
   #ifdef CBOPS_DEBUG
      r0 = $cbops.MAX_OPERATORS;
      M[count] = r0;
   #endif // CBOPS_DEBUG
   operator_functions_loop:
      #ifdef CBOPS_DEBUG
         // check we haven't looped more times than $cbops.MAX_OPERATORS
         r1 = M[count];
         r1 = r1 - 1;
         M[count] = r1;
         // linked list corrupt or num_operators>MAX_OPERATORS
	 if NEG call $error;
      #endif // CBOPS_DEBUG
      // Buffer array in (r4)
      r4 = M[SP - $CBOPS_LOCAL_BUFFER_PTR];
      // load the function address of the "amount of data to use" function
      r0 = M[r8 + $cbops_c.cbops_op_struct.FUNCTION_VECTOR_FIELD];
      // load the parameter area for the operator
      r8 = r8 + $cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD;
      // if it isn't NULL then call the process function
      r0 = M[r0 + $cbops_c.cbops_functions_struct.PROCESS_FIELD];
      if NZ call r0;

#ifdef DUMMY_READ_FOR_24BIT_CLEANUP
      // dummy read from port 11 to make sure any 24 bit reads are all cleaned up
      Null = M[$READ_PORT11_DATA];
#endif

      // load the next operator structure
      r8 = M[r8 + ($cbops_c.cbops_op_struct.NEXT_OPERATOR_ADDR_FIELD - $cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD)];
      if NZ jump operator_functions_loop;


    r0  = M[SP - $CBOPS_LOCAL_GRAPH_PTR];
    /* Post main for graph */
    r1 = M[r0 + ($cbops_c.cbops_graph_struct.OVERRIDE_FUNCS_FIELD + $cbops_c.cbops_functions_struct.PROCESS_FIELD)];
    if Z jump jp_post_main_override_done;
      // Buffer array in (r4)
      r4 = M[SP - $CBOPS_LOCAL_BUFFER_PTR];
      // load the parameter area for the operator
      r8 = M[r0 + $cbops_c.cbops_graph_struct.OVERRIDE_DATA_FIELD];
      call r1;
      r0 = M[SP - $CBOPS_LOCAL_GRAPH_PTR];
    jp_post_main_override_done:

   // ** update the buffers **
   r10 = M[r0 + $cbops_c.cbops_graph_struct.NUM_IO_FIELD];
   r8  = M[SP - $CBOPS_LOCAL_BUFFER_PTR];
   r6  = $cbuffer.set_read_address;
   do lp_update_buffers;
      // Get Buffer Type, No update for Internal buffers
      r7 = M[r8 + $cbops_c.cbops_buffer_struct.TYPE_FIELD];
      NULL = r7 AND $cbops_c.CBOPS_IO_INTERNAL;
      if NZ jump jp_update_done;
         // If linked buffer, transfer is pointer, de-reference
         r5 = M[r8 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
         // If transfer zero then skip buffer update
         r5 = M[r5];
         if Z jump jp_update_done;
         // Get Buffer, SW buffer or Port?
         r0 = M[r8 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
         r2 = M[r8 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
         r1 = M[r8 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD];
	     #ifdef CBOPS_DEBUG // check for valid port/buffer size
            // zero or negative size indicates not correctly initialised
            Null = r2;
            if LE call $error;
         #endif
         // if SW Buffer (length > 1) Advance SW Buffer Pointer
         NULL = r2 - MK1;
         if LE jump lp_update_port;
            L1 = r2;
            I1 = r1;
            r1 = M[r8 + $cbops_c.cbops_buffer_struct.BASE_FIELD];
            push r1;
            pop B1;
            // Dummy Read to advance circular buffer
            Words2Addr(r5);
            M2 = r5;
            r1 = M[I1,M2];
            // Clear B1
	    push NULL;
   	    pop B1;
            // New Rd/wr address (r1)
            r1 = I1;
            M[r8 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD]=r1;
         lp_update_port:

         // r0=buffer, r1=read/wr address
         // Call update function (r6 is ptr to '$cbuffer.set_read_address' )
         r2 = $cbuffer.set_write_address;
         NULL = r7 AND $cbops_c.CBOPS_IO_SOURCE;
         if NZ r2 = r6;
         call r2;
      jp_update_done:
      // Next buffer
      r8 = r8 + $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR;
   lp_update_buffers:

jp_processing_done:
   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_copy;
      call $profiler.stop;
   #endif

#ifdef CBOPS_DEBUG
    // debug prove concurrent chains, release semaphore
    r0 = M[fw_semaphore];
    if Z call $error;        // expected semaphore taken at this point
    r0 = r0 - 1;
    M[fw_semaphore] = r0;
#endif

   // Remove local variables from stack
   SP = SP - $CBOPS_LOCAL_SIZE;
   // Return to C
   POP_ALL_C
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops_utlities
//
// DESCRIPTION:
//    some utility functions used by cbops operators
// *****************************************************************************
.MODULE $M.cbops.cbops_utlities;
   .CODESEGMENT CBOPS_COPY_PM;

// *****************************************************************************
// $cbops.basic_multichan_amount_to_use
//     utility function to calculate amount of use by a multichannel operator,
//     the operator needs to operate in 1:1 ratio.
// INPUTS:
//    r8 - pointer to operator specific data
//    r4 - buffer table
// OUTPUT:
//    r8: amount to use
// TRASHED:
//    r0, r1, r7, r9
// *****************************************************************************
$cbops.basic_multichan_amount_to_use:
   // get number of input channels - transform it into addresses, so that we don't do the latter
#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbops.cbops_utlities.PATCH_ID_0, r9)
#endif

   // for every channel. But let's keep calling it subsequently the "number of channels"
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);
   // Get space at output
   r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r0];                                             // Output index (first channel)
   r1 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index (first channel)

   // If inplace do nothing
   NULL = r0 - r1;
   if Z rts;
   // Advance to amount pointer
   r4 = r4 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD;
   // Pointer to output amount (r0)
   r0 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR (int);
   r0 = M[r4 + r0];
   // Get output space
   r7 = M[r0];
   // Pointer to input amount (r1)
   r1 = r1 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR (int);
   r1 = M[r4 + r1];
   // Get data at input
   r5 = M[r1];
   // data = MIN(data,space), copy to input
   r5 = MIN r7;
   M[r1]=r5;
   rts;


// *****************************************************************************
// $cbops.get_transfer_and_update_multi_channel
//
// INPUTS:
//    r8 - pointer to operator specific data
//    r4 - buffer table
// OUTPUT:
//    r0: transfer amount in samples
//    r9: offset to output channels in addrs (from the start of index table)
// TRASHED:
//    r1, r2
// *****************************************************************************
$cbops.get_transfer_and_update_multi_channel:
   // get number of input channels - transform it into addresses, so that we don't do the latter
   // for every channel. But let's keep calling it subsequently the "number of channels"
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);

   // get the input and output index for first channel
   // returns amount to process in r0
   r1 = r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r1 + 0];     // input index (first channel)
   r1 = M[r1 + r9];    // Output index (first channel)

   // Fall through to $cbops.get_transfer_and_update;

// *****************************************************************************
// $cbops.get_transfer_and_update
//
// INPUTS:
//    r4 - buffer table
//    r1 - output buffer index
//    r0 - input buffer index
// OUTPUT:
//    r0: transfer amount in samples
// TRASHED:
//    r2
// *****************************************************************************
$cbops.get_transfer_and_update:
   // Advance to amount pointer
   r2 = r4 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD;
   // Get transfer amount of input
   r0 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR (int);
   r0 = M[r2 + r0];
   // Transfer amount at input
   r0 = M[r0];
   // Update output
   r1 = r1 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR (int);
   r1 = M[r2 + r1];
   // Set Output to input
   M[r1]=r0;
   rts;


// *****************************************************************************
// $cbops.get_transfer_and_update
//
// INPUTS:
//    r4 - buffer table
//    r0 - buffer index
// OUTPUT:
//    r0: read or write pointer for the buffer
//        (depends whether it's input or output buffer)
//    r1: buffer lengths (in addrs)
//    r2: buffer start address
// *****************************************************************************
$cbops.get_buffer_address_and_length:
   r2 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r2 = r2 + r4;
   r0 = M[r2 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD];
   r1 = M[r2 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
   r2 = M[r2 + $cbops_c.cbops_buffer_struct.BASE_FIELD];
   rts;

// *****************************************************************************
// $cbops.get_transfer_and_update
//
// INPUTS:
//    r4 - buffer table
//    r0 - buffer index
//    M3 - amount to advance the read or write pointer of the buffer (in addrs)
// OUTPUT:
//    None
// TRASHED:
//    r2,I0,L0,B0
// *****************************************************************************
$cbops.advance_buffer_address:
   r2 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r2 = r2 + r4;
   r0 = M[r2 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD];
   if Z rts;
   r1 = M[r2 + $cbops_c.cbops_buffer_struct.SIZE_FIELD];
   // Don't advance ports
   NULL = r1 - MK1;
   if Z rts;
   // Signal buffer refresh
   M[r4 - 2*ADDR_PER_WORD] = r2;
   // Setup Buffer I0
   I0 = r0;
   r0 = M[r2 + $cbops_c.cbops_buffer_struct.BASE_FIELD];
   L0 = r1;
   push r0;
   pop B0;
   // Advance pointer by M3
   r1 = M[I0, M3];
   r1 = I0;
   M[r2 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD]=r1;
   rts;

// *****************************************************************************
// $cbops.set_buffer_address
//
// INPUTS:
//    r4 - buffer table
//    r0 - buffer index
//    r1 - new address to set the buffer pointer to
// OUTPUT:
//    None
// TRASHED:
//    r2
// *****************************************************************************
$cbops.set_buffer_address:
   r2 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r2 = r2 + r4;
   M[r2 + $cbops_c.cbops_buffer_struct.RW_PTR_FIELD]=r1;
   rts;

// *****************************************************************************
// $cbops.get_cbuffer
//
// INPUTS:
//    r4 - buffer table
//    r0 - buffer index
// OUTPUT:
//    r0: pointer to buffer structure for the provided index
// *****************************************************************************
$cbops.get_cbuffer:
   r0 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR(int);
   r0 = r0 + r4;
   r0 = M[r0 + $cbops_c.cbops_buffer_struct.BUFFER_FIELD];
   rts;

// *****************************************************************************
// $cbops.get_cbuffer
//
// INPUTS:
//    r4 - buffer table
//    r0 - buffer index
// OUTPUT:
//    r0: transfer amount for the buffer
// *****************************************************************************
$cbops.get_amount_ptr:
   r0 = r0 * $CBOP_BUFTAB_ENTRY_SIZE_IN_ADDR (int);
   r0 = r0 + r4;
   r0 = M[r0 + $cbops_c.cbops_buffer_struct.TRANSFER_PTR_FIELD];
   rts;

// *****************************************************************************
// $cbops.force_processing
// sets the force_processing flag, so operators will run even if transfer amount
// is 0. It will be cleared once processing done.
//
// INPUTS:
//    r4 - buffer table
// OUTPUT:
//    r0: transfer amount for the buffer
// *****************************************************************************
$cbops.force_processing:
   M[r4 - ($cbops_c.cbops_graph_struct.BUFFERS_FIELD-$cbops_c.cbops_graph_struct.FORCE_UPDATE_FIELD)] = r4;   // Force flag = non-zero
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//  cbops_accessors
//  ***************************************************************************
.MODULE $M.cbops.accessors;
   .CODESEGMENT CBOPS_COPY_PM;

//*****************************************************************************
//    asm: $cbops.get_num_outputs
//    C: unsigned cbops_get_num_outputs(cbops_graph *head);
//
//    asm: $cbops.get_num_inpputs
//    C: unsigned cbops_get_num_inputs(cbops_head *head);
//
// INPUTS: (not all might be relevant)
//    - r0 = pointer to cbops graph
//
// OUTPUTS:
//    - r0 = number of input/output channels in the cbops graph,
//           note: returns used as well as unused channels
//
// TRASHED REGISTERS:
//   I3
//
// *****************************************************************************
   $cbops.get_num_outputs:
   $_cbops_get_num_outputs:
   $cbops.get_num_inputs:
   $_cbops_get_num_inputs:
   // load the pointer to the io[] area in the head structure
   I3 = r0 + $cbops_c.cbops_graph_struct.NUM_IO_FIELD;
   r0 = M[I3,0];
   r0 = r0 LSHIFT -1;
   rts;
.ENDMODULE;
