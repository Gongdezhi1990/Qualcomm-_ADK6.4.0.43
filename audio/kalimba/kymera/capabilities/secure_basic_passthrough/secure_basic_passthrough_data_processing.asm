// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM function for secure (with licensing) basic mono operator data processing
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "cbuffer_asm.h"
#include "secure_basic_passthrough_shared_const.h"
#include "secure_basic_passthrough_struct_asm_defs.h" // data structure field offsets
#include "portability_macros.h"

.MODULE $M.secure_basic_passthrough_proc;
    .CODESEGMENT BASIC_PASS_PM;

// *****************************************************************************
// MODULE:
//    $_basic_passthrough_processing
//
// DESCRIPTION:
//    Data processing function. Processes each active channel of the operator
//
// INPUTS:
//    - r0 = OPERATOR_EXTRA_DATA struct address (the cap specific part of the
//    data structure)
//    - r1 = number of samples to process
//    - M2 = -ADDR_PER_WORD (This is implict in that it is called from C
//
// OUTPUTS:
//    - r0 = return code (0 for success)
//
// TRASHED REGISTERS:
//    rMAC, r2, r3, r10, I7
//
// NOTES: REMOTE Buffers might work (for hydra) assuming they are accessed through a window,
//        but have not been tested.
//        It relies for now on imagined $cbuffer and $common modules' constants
//
// *****************************************************************************

$_secure_basic_passthrough_processing:

   // for now, we assume we get to the buffer parameters directly with some offset constants
   push rLink;
   pushm <I0, I4, L0, L4>;
   pushm <B0, B4>;
   pushm <r5, r6>;

   push r1; // samples to process is needed for each channel so push it on the stack


   /* Go through each active channel and perform a copy, during the loop the following
    * is used by the loop:
    * I7 - current channel structure pointer
    * r5 - Num active channels counter
    * r3 - current channel structure
    * rMAC - copy loop address
    * M[SP-ADDR_PER_WORD] - samples to process
    * several registers are also used by the copy loops
    */
   Null = M[r0 + $secure_basic_passthrough_struct.SECURE_BASIC_PASSTHROUGH_OP_DATA_struct.REINITFLAG_FIELD];
   if Z jump secure_basic_pass_thru_init_bypass;
   M[r0 + $secure_basic_passthrough_struct.SECURE_BASIC_PASSTHROUGH_OP_DATA_struct.REINITFLAG_FIELD] = Null;

secure_basic_pass_thru_init_bypass:
   rMAC = M[r0 + $secure_basic_passthrough_struct.SECURE_BASIC_PASSTHROUGH_OP_DATA_struct.COPY_FUNCTION_FIELD];

   I7 = r0 + $secure_basic_passthrough_struct.SECURE_BASIC_PASSTHROUGH_OP_DATA_struct.CHANNEL_FIELD;
   r5 = M[r0 + $secure_basic_passthrough_struct.SECURE_BASIC_PASSTHROUGH_OP_DATA_struct.NUM_ACTIVE_CHANS_FIELD];

   // There must be at least 1 active channel otherwise this function wouldn't have been called

secure_channel_loop:
   // Initialise r3 to the next channel
   r3 = M[I7, ADDR_PER_WORD];

   // get read pointer, size and start addresses of input buffer if the buffer
   // doesn't exist this and any more channels in the list are inactive
   r0 = M[r3 + $secure_basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.IP_BUFFER_FIELD];
   call $cbuffer.get_read_address_and_size_and_start_address;
   // I0, L0, B0 = Input buffer read index, length and base address
   push r2;
   pop B0;
   I0 = r0;
   L0 = r1;

   // now get the output buffer info if its null then its inactive
   r0 = M[r3 + $secure_basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.OP_BUFFER_FIELD];
   call $cbuffer.get_write_address_and_size_and_start_address;
   // I4, L4, B4 = Output buffer write index, length and base address   
   push r2;
   pop B4;
   I4 = r0;
   L4 = r1;

   r10 = M[SP - ADDR_PER_WORD]; // load number of samples to process into r10;   

#ifdef INSTALL_LICENSE_CHECK
secure_license_check:
   pushm <rMAC,r0>;
   call $_secure_basic_passthrough_license_ok;
   r2 = r0;
   popm <rMAC,r0>;
   NULL = r2;
   if NZ jump secure_start_copying;
   rMAC = secure_insert_silence;

secure_start_copying:
#endif
   // jump to the appropriate copy routine
   jump rMAC;

secure_done_copying:
   // The copy loop reads an extra sample from the input buffer, this needs to
   // be read back 1 sample, we can do this in parallel with the next operation so we do.

   // update read address in input buffer
   r0 = M[r3 + $secure_basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.IP_BUFFER_FIELD], r2 = M[I0, M2];
   r1 = I0;
   call $cbuffer.set_read_address;

   // update write address in output buffer
   r0 = M[r3 + $secure_basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.OP_BUFFER_FIELD];
   r1 = I4;
   call $cbuffer.set_write_address;

   // Decrement the channel counter
   r5 = r5 - 1;
   if NZ jump secure_channel_loop;

secure_all_channels_processed:
   pop r1;  // take samples to process back off the stack;

   popm <r5, r6>;
   popm <B0, B4>;
   popm <I0, I4, L0, L4>;
   pop rLink;

   rts;


secure_mono_data_copy_loop:
   Null = I0 - I4;
   if Z jump secure_no_copy;

   // copy unprocessed data across
   r2 = M[I0, ADDR_PER_WORD];
   do secure_pass_copy_loop_data;
      M[I4, ADDR_PER_WORD] = r2, r2 = M[I0, ADDR_PER_WORD];
   secure_pass_copy_loop_data:
   // This reads one extra sample from I0. This is stepped back in the main
   // loop as this opeation is common and saves a stall this way.
   jump secure_done_copying;

secure_no_copy:
   // In-place data copy doesn't actually do anything,
   // so just update the pointers
   push M3;
   r2 = r10 ASHIFT LOG2_ADDR_PER_WORD;
   M3 = r2;
   r2 = M[I0, M3];
   r2 = M[I4, M3];
   pop M3;

   // Extra read as above, to simplify code in the main loop
   r2 = M[I0, ADDR_PER_WORD];

   jump secure_done_copying;

#ifdef INSTALL_LICENSE_CHECK
secure_insert_silence:
   r2 = 0;
   do secure_insert_silence_data;
      M[I4, ADDR_PER_WORD] = r2;
   secure_insert_silence_data:
   // This reads one extra sample from I0. This is stepped back in the main
   // loop as this opeation is common and saves a stall this way.
   jump secure_done_copying;
#endif

// Alias these loops to C so that there location can be stored in the operator
// structure from C
.set $_secure_mono_data_loop, secure_mono_data_copy_loop

.ENDMODULE;


