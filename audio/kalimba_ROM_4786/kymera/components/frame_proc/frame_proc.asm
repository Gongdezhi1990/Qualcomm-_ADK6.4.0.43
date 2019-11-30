// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Frame Processing Library
//
// DESCRIPTION:
//    This library provides support from the Frame Process Architecture.
//    The Frame Processing Architecture provides the a consistent means of
//    processing multiple syncronized data streams broken into discrete frame. 
//
//    The processing for the frame is specified in a N x 3 matrix where N is
//    the number of functions to call to perform the total processing.  The
//    columns are:
//       1) Function Pointer
//       2) Parameter to pass to function via register r7
//       2) Parameter to pass to function via register r8
//
//    The function "run_frame_proc_function_table" is provided to perform the
//    processing for a frame.  It is passed a pointer to the processing 
//    table as a parameter.
//
//    NOTE: While "run_frame_proc_function_table" may be called from C, the
//          functions in the processing table may not be C functions.   This
//          is due to the calls not following C guidelines with respect to
//          call structure and reserved registers (M0,M1,M2)
//
//    The parameters passed to the processing functions are typically pointers
//    to data structures associated with that unit of processing.  
//
//    For each input/ouput stream associated with the frame processing a
//    tFrmBuffer data strucure is declared.   Each unit of processing includes
//    pointers to the tFrmBuffer data strucures for the stream it will consume
//    or produce.   Most frame processing may be in-place using the same stream
//    for its input and output.
//
//    To support abstraction, two utility functions are provided for a processing
//    function to access the tFrmBuffer.  They are:
//       $frmbuffer.get_buffer_with_start_address
//            Get Frame Buffer Information (Pointer, Base Address, Circular Buffer Length)
//            Get Frame Size
//       $frmbuffer.set_frame_size
//            Transfer Frame Size from input to output tFrmBuffer
//
//    NOTE:  The frame size may be changed during processing provided the factor of change
//           is a constant interger.  For example, an up-sample by a factor of 2.
//
//    The First function in the processing table must be "$frame_proc.distribute_streams".
//       This function takes as a parameter (r7) an array of pointers to the tFrmBuffer
//       data structures associated with the input/output streams.  It Ininitizes the
//       tFrmBuffer to start the processing of a frame.
//
//    The Last function in the processing table must be "$frame_proc.update_streams". 
//       This function advances the streams after the frame has been processed.
//       
//    In the tFrmBuffer data structure two function pointers are defined to facilitate
//    The processing (Distribute & Update).   
//    For an input stream they are "$frame_proc.distribute_input_stream" and "$frame_proc.update_input_stream" 
//    For an output stream they are "$frame_proc.distribute_output_stream" and "$frame_proc.update_output_stream"
//
//    These function may drop/insert samples to maintain synchronization and latency control
//       1) If an input stream has insufficient data when the frame process is performed
//          samples are inserted to allow the frame to be processed
//       2) If an output stream has insufficient space when the frame process is performed
//          samples are dropped to make room for the frame
//       3) If an input stream has excess data above a specified threshold samples are dropped
//          to reduce latency (slack).  If the threshold is zero this feature is disabled
//
// *****************************************************************************
#include "stack.h"
#include "portability_macros.h"
#include "frame_proc/frame_proc_asm.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif

// *****************************************************************************
// MODULE:
//	  $frmbuffer.get_buffer_with_start_address
//
// DESCRIPTION:
//    Get frame buffer frame size, ptr,length, and base address
//
// INPUTS:
//    - r0 = pointer to frame buffer structure
//
// OUTPUTS:
//    - r0 = buffer address
//    - r1 = buffer size
//    - r2 = buffer start address   
//    - r3 = frame size
//
// TRASHED REGISTERS:
//    none
//
//
// *****************************************************************************
.MODULE $M.frmbuffer.get_buffer;
   .CODESEGMENT EXT_DEFINED_PM;

$frmbuffer.get_buffer_with_start_address:
   r3  = M[r0 + $frmbuffer.FRAME_SIZE_FIELD];
   r2  = M[r0 + $frmbuffer.BUFFER_START_ADDRESS_FIELD];
   r1  = M[r0 + $frmbuffer.BUFFER_SIZE_FIELD];
   r0  = M[r0 + $frmbuffer.FRAME_PTR_FIELD];
   rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $frmbuffer.set_frame_size
//
// DESCRIPTION:
//    Set frame buffer's frame size
//
// INPUTS:
//    - r0 = pointer to frame buffer structure
//    - r3 = frame size
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    none
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.frmbuffer.set_frame_size;
   .CODESEGMENT EXT_DEFINED_PM;

$frmbuffer.set_frame_size:
   M[r0 + $frmbuffer.FRAME_SIZE_FIELD] = r3;
   rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $frmbuffer.set_frame_address
//
// DESCRIPTION:
//    Set frame buffer's frame address
//
// INPUTS:
//    - r0 = pointer to frame buffer structure
//    - r1 = frame address
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    none
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.frmbuffer.set_frame_address;
   .CODESEGMENT EXT_DEFINED_PM;

$frmbuffer.set_frame_address:
   M[r0 + $frmbuffer.FRAME_PTR_FIELD] = r1;
   rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_run_frame_proc_function_table
//    void run_frame_proc_function_table(unsigned *table_ptr, unsigned *user_obj_ptr);
//
// DESCRIPTION:
//    This function calls a series of functions defined in the NULL termininated
//    processing table passed in through register (r0).  Each entry in the table
//   has the following three fields:
//    1) Ptr to function to be called.
//    2) Value to insert into register (r7) before calling function
//    3) Value to insert into register (r8) before calling function
//
//    Also, before calling function, the user specified global data object (r1)
//    are passed into register (r9).
//
// INPUTS:
//    - r0 holds the address of the processing table to run
//    - r1 holds the global data specified by the user
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    Follows C guidelines
//
// NOTE:
//   There are no limitations on register usage by a processing function
//
// *****************************************************************************
.MODULE $M.frame_proc.run_function_table;
   .CODESEGMENT   EXT_DEFINED_PM;

$_run_frame_proc_function_table:
   push rLink;
   // Save Registers - assume all used in processing
   pushm <r4, r5, r6, r7, r8, r9>;
   pushm <I0, I1, I2, I4, I5, I6, M0, M1, M2, M3, L0, L1, L4, L5>;
   pushm <rMACB2, rMACB1, rMACB0, B0, B1, B4, B5,FP>;
   r4 = M[$ARITHMETIC_MODE];
   push r4;

#if defined(PATCH_LIBS)
   push r0;
   LIBS_SLOW_SW_ROM_PATCH_POINT($run_frame_proc_function_table.PATCH_ID_0, r4)
   pop r0;
#endif

   r4 = r0;
lp_proc_loop:
   // Func
   r5 = M[r4];
   // NULL Function terminates List
   if Z jump jp_done;
   // Data
   r7 = M[r4 + 1*ADDR_PER_WORD];
   // Data
   r8 = M[r4 + 2*ADDR_PER_WORD];
   // User global data
   r9 = r1;
   // Process Module
   push r1;
   push r4;
   call r5;
   pop r4;
   pop r1;
   // Next Module
   r4 = r4 + 3*ADDR_PER_WORD;
   jump lp_proc_loop;
jp_done:
   // Restore Registers
   pop r4;
   M[$ARITHMETIC_MODE]=r4;
   popm <rMACB2, rMACB12, rMACB0, B0, B1, B4, B5,FP>;
   popm <I0, I1, I2, I4, I5, I6, M0, M1, M2, M3, L0, L1, L4, L5>;
   popm <r4, r5, r6, r7, r8, r9>;
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $frame_proc.distribute_streams
//
// DESCRIPTION:
//    Initialize frame buffer objects for frame to be processed
//
// INPUTS:
//    - r7 = pointer to NULL terminated array of $frmbuffer objects
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r7,r9,r10,M1,I0,L0,B0
//
// NOTES:
//    Interupts are blocked to ensure buffers are syncronized 
//
// *****************************************************************************
.MODULE $M.frame_proc.distribute_streams;
   .CODESEGMENT   EXT_DEFINED_PM;

$frame_proc.distribute_streams:

   push rLink;
   call $block_interrupts;
   
   M1 = MK1;
jp_next_stream:
   // Get Stream Ptr.  Table is NULL Terminated
   r9 = M[r7];      
   if Z jump jp_stream_done;
   // Reset Current Frame Size from master frame size
   r4 = M[r9 + $frmbuffer.INITIAL_FRAME_SIZE_FIELD];
   M[r9 + $frmbuffer.FRAME_SIZE_FIELD]=r4;
   // Get Stream Distribute Function and Data Ptr
   r1 = M[r9+$frmbuffer.DISTRIBUTE_FUNCPTR_FIELD];
   // Get CBuffer for Stream
   r0 = M[r9 + $frmbuffer.CBUFFER_PTR_FIELD];

   // Call stream's Distribution Function
   call r1;

   // Save Frame Pointer and clear circular buffer registers
   r0 = I0;
   M[r9 + $frmbuffer.FRAME_PTR_FIELD]=r0;  
   
   // Increment Stream Table Ptr
   r7 = r7 + 1*ADDR_PER_WORD; 
   jump jp_next_stream;

jp_stream_done:
   // Clear Circular buffering (I0)
   push NULL;
   pop  B0;
   L0 = NULL;
   // Restore Interupts
   call $unblock_interrupts;  
   jump $pop_rLink_and_rts;
.ENDMODULE;   

// *****************************************************************************
// MODULE:
//    $frame_proc.update_streams
//
// DESCRIPTION:
//    Advance Buffer Pointers after frame is processed
//
// INPUTS:
//    - r7 = pointer to NULL terminated array of $frmbuffer objects
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r7,r9,r10,M0,M1,I0,L0,B0
//
// NOTES:
//    Interupts are blocked to ensure buffers are syncronized 
//
// *****************************************************************************
.MODULE $M.frame_proc.update_streams;
   .CODESEGMENT   EXT_DEFINED_PM;

$frame_proc.update_streams:

   push rLink;
   call $block_interrupts;
   
   M1 = MK1;
jp_next_stream:
   // Get Stream Ptr.  Table is NULL Terminated
   r9 = M[r7];      
   if Z jump jp_stream_done;   

   // Get Stream Distribute Function and Data Ptr
   r1 = M[r9+$frmbuffer.UPDATE_FUNCPTR_FIELD];
   // Get Buffer Info
   r0  = M[r9 + $frmbuffer.BUFFER_START_ADDRESS_FIELD];
   push r0;
   pop  B0;
   r0  = M[r9 + $frmbuffer.BUFFER_SIZE_FIELD];
   L0  = r0;
   r0  = M[r9 + $frmbuffer.FRAME_PTR_FIELD];
   I0  = r0;

   // Call stream's Update Function
   call r1;

   // Advance buffer
   r0 = M[I0,M0];
   r1 = I0; 
   // Call Buffer Update Function
   r0 = M[r9 + $frmbuffer.CBUFFER_PTR_FIELD];
   call r2;

   // Increment Stream Table Ptr
   r7 = r7 + 1*ADDR_PER_WORD; 
   jump jp_next_stream;

jp_stream_done:
   // Clear Circular buffering (I0)
   push NULL;
   pop  B0;
   L0 = NULL;
   // Restore Interupts
   call $unblock_interrupts;  
   jump $pop_rLink_and_rts;
.ENDMODULE;   


// *****************************************************************************
// MODULE:
//    $frame_proc.distribute_output_stream
//
// DESCRIPTION:
//    Distribute function for output streams
//
// INPUTS:
//    - r4 = Frame Size
//    - r0 = cBuffer Pointer
//    - r9 = $frmbuffer object pointer (reserve)
//    - M1 = MK1  (reserve)
//    - r7 = stream array (reserve)
//
// OUTPUTS:
//    - I0 = Frame Pointer
//    - B0 = Start Address
//    - L0 = Buffer Length
//
// TRASHED REGISTERS:
//    r0,r1,r2,r3,r10
// NOTES:
//    The only control for Output Streams is to drop samples
//    if there is insufficent space for the frame.   Drops 
//    a little extra (JITTER) to reduce frequency of drops
//
// *****************************************************************************
// *****************************************************************************
// MODULE:
//    $frame_proc.update_output_stream
//
// DESCRIPTION:
//    This function is used for output stream by the update function.  It
//    drops/inserts samples as required.
//
// INPUTS:
//    - r0 - pointer to cbuffer structure
//    - r4,M0 = Frame Size
//    - r9 = $frmbuffer object pointer (reserve)
//    - M1 = MK1  (reserve)
//    - r7 = stream array (reserve)
//    - I0,L0,B0 = circular buffer
//
// OUTPUTS:
//    - M0 - amount to advance buffer 
//    - r2 - Function for advance
//    - I0 = Frame Pointer
//    - B0 = Start Address
//    - L0 = Buffer Length
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************

.MODULE $M.frame_proc.output_stream;
   .CODESEGMENT   EXT_DEFINED_PM;

$frame_proc.distribute_output_stream:

   push rLink;
 
#if defined(PATCH_LIBS)
   push r0;
   LIBS_SLOW_SW_ROM_PATCH_POINT($frame_proc.distribute_output_stream.PATCH_ID_0, r1)
   pop r0;
#endif 

   // Get Buffer Info for distribution
   push r0;
   call $cbuffer.get_write_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;
   M[r9 + $frmbuffer.BUFFER_SIZE_FIELD]=r1;
   M[r9 + $frmbuffer.BUFFER_START_ADDRESS_FIELD]=r2;

   // Check Amount of Space in Output
   pop r0;
   call $cbuffer.calc_amount_space_in_addrs;
   Words2Addr(r4);                       // arch4: r4 = frame size in addr
   M0 = r0 - r4;
   if POS jump $pop_rLink_and_rts;

   // Make room in buffer by dropping samples (drop a little extra - Jitter)
   r0 = M[r9 + $frmbuffer.JITTER_FIELD];
   Words2Addr(r0);                       // arch4: r0 = jitter samples in addr
   M0 = M0 - r0;
   r1 = M[I0,M0];
   r1 = M[r9 + $frmbuffer.DROPPED_SAMPLES_FIELD];
   Words2Addr(r1);                       // arch4: r1 = dropped samples in addr
   r1 = r1 - M0;
   Addr2Words(r1);                       // arch4: r1 = dropped samples in words
   M[r9 + $frmbuffer.DROPPED_SAMPLES_FIELD]=r1;

   jump $pop_rLink_and_rts;

$frame_proc.update_output_stream:
   // Get Frame Size
   r4 = M[r9 + $frmbuffer.FRAME_SIZE_FIELD];
   Words2Addr(r4);
   M0 = r4;
   // SP.  No Latency Control for Output Streams  
   r2 = $cbuffer.set_write_address;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $frame_proc.distribute_input_stream
//
// DESCRIPTION:
//    Distribute function for input streams
//
// INPUTS:
//    - r0 - pointer to cbuffer structure
//    - r4 = Frame Size
//    - r5 = cBuffer Pointer
//    - r9 = $frmbuffer object pointer (reserve)
//    - M1 = MK1  (reserve)
//    - r7 = stream array (reserve)
//
// OUTPUTS:
//    - I0 = Frame Pointer
//    - B0 = Start Address
//    - L0 = Buffer Length
//
// TRASHED REGISTERS:
//    r0,r1,r2,r3,r10
//
// NOTES:
//    
//
// *****************************************************************************
// *****************************************************************************
// MODULE:
//    $frame_proc.update_input_stream
//
// DESCRIPTION:
//    This function is used for input stream by the update function.  It
//    drops/inserts samples as required.
//
// INPUTS:
//    - r4,M0 = Frame Size
//    - r9 = $frmbuffer object pointer (reserve)
//    - M1 = MK1  (reserve)
//    - r7 = stream array (reserve)
//
// OUTPUTS:
//    - M0 - amount to advance buffer 
//    - r2 - Function for advance
//    - I0,L0,B0 = circular buffer
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
.MODULE $M.frame_proc.input_stream;
   .CODESEGMENT   EXT_DEFINED_PM;

$frame_proc.distribute_input_stream:

   push rLink;

#if defined(PATCH_LIBS)
   push r0;
   LIBS_SLOW_SW_ROM_PATCH_POINT($frame_proc.distribute_input_stream.PATCH_ID_0, r1)
   pop r0;
#endif 

   // Get Buffer Info
   push r0;
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;
   M[r9 + $frmbuffer.BUFFER_SIZE_FIELD]=r1;
   M[r9 + $frmbuffer.BUFFER_START_ADDRESS_FIELD]=r2;

   // Check Amount of Data
   M[r9 + $frmbuffer.AMOUNT_TO_DROP_FIELD]=NULL;
   pop r0;
   call $cbuffer.calc_amount_data_in_words;
   r10 = r0 - r4;
   if NEG jump frame_proc.insert_samples;

   // Check amunt of data in buffer minus frame (r10) against threshold
   r0  = M[r9 + $frmbuffer.THRESHOLD_FIELD];
   if Z jump $pop_rLink_and_rts;
   r10 = r10 - r0;
   if LE jump $pop_rLink_and_rts;

   // Drop amount above Threshold plus some extra (Jitter)
   r0 = M[r9 + $frmbuffer.JITTER_FIELD];
   r10 = r10 + r0;
   M[r9 + $frmbuffer.AMOUNT_TO_DROP_FIELD]=r10;

   r0 = M[r9 + $frmbuffer.DROPPED_SAMPLES_FIELD];
   r0 = r0 + r10;
   M[r9 + $frmbuffer.DROPPED_SAMPLES_FIELD]=r0;

   jump $pop_rLink_and_rts;

   // r10 is amount to insert (negative)
frame_proc.insert_samples:
   // insert a little extra (jitter)
   r0 = M[r9 + $frmbuffer.JITTER_FIELD];
   r10 = r0 - r10;

   // Update statistics
   r0 = M[r9 + $frmbuffer.INSERTED_SAMPLES_FIELD];
   r0 = r0 + r10;
   M[r9 + $frmbuffer.INSERTED_SAMPLES_FIELD] = r0;

   // Repeat First Sample and back up Frame Ptr
   //  Insertion Preceeds available data because input can only
   //  modify read pointer.
   r0 = M[I0,-MK1];
   do insert_loop;
      M[I0,-MK1] = r0;
   insert_loop:
   r0 = M[I0,MK1];
   jump $pop_rLink_and_rts;


$frame_proc.update_input_stream:

   // Get Frame Size (use initial because it may have changed)
   r4 = M[r9 + $frmbuffer.INITIAL_FRAME_SIZE_FIELD];
   Words2Addr(r4);
   M0 = r4;

   // Adjust buffer for dropped samples
   r2 = M[r9 + $frmbuffer.AMOUNT_TO_DROP_FIELD];
   Words2Addr(r2);                        // arch4: r2 = amount to drop in addrs
   M0 = M0 + r2;   
   r2 = $cbuffer.set_read_address;
   rts;
.ENDMODULE;


