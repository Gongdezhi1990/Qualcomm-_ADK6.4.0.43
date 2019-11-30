// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM function for basic mono operator data processing
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "cbuffer_asm.h"
#include "basic_passthrough_shared_const.h"
#include "basic_passthrough_struct_asm_defs.h" // data structure field offsets
#include "basic_passthrough_gen_asm.h"
#include "portability_macros.h"

.MODULE $M.basic_passthrough_proc;
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
//    rMAC, r2, r3, r4, r10, I7, I6
//
// NOTES: REMOTE Buffers might work (for hydra) assuming they are accessed through a window,
//        but have not been tested.
//        It relies for now on imagined $cbuffer and $common modules' constants
//
// *****************************************************************************

$_basic_passthrough_processing:

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
    * r4 - input gain parameter
    * rMAC - copy loop address
    * M[SP-ADDR_PER_WORD] - samples to process
    * several registers are also used by the copy loops
    */
   r2 = r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.PARAMS_FIELD;
   r4 = M[r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.GAIN_LINEAR_FIELD];
   Null = M[r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.REINITFLAG_FIELD];
   if Z jump basic_pass_thru_init_bypass;
   r4 = M[r2 + $M.BASIC_PASSTHROUGH.PARAMETERS.OFFSET_GAIN];   // in q6.26
   r4 = r4 ASHIFT -2;                 // q8.24
   call $log2_to_pow2_pos_input;
   M[r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.REINITFLAG_FIELD] = Null;
   r4 = r3;
   M[r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.GAIN_LINEAR_FIELD] = r4;
basic_pass_thru_init_bypass:

   rMAC = M[r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.COPY_FUNCTION_FIELD];

   I7 = r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.CHANNEL_FIELD;
   I6 = r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.PEAK_LEVEL_1_FIELD;
   r5 = M[r0 + $basic_passthrough_struct.BASIC_PASSTHROUGH_OP_DATA_struct.NUM_ACTIVE_CHANS_FIELD];

   // There must be at least 1 active channel otherwise this function wouldn't have been called

channel_loop:
   // Initialise r3 to the next channel
   r3 = M[I7, ADDR_PER_WORD];
   // get read pointer, size and start addresses of input buffer if the buffer
   // doesn't exist this and any more channels in the list are inactive
   r0 = M[r3 + $basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.IP_BUFFER_FIELD];
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop B0;
   I0 = r0;
   L0 = r1;

   // now get the output buffer info if its null then its inactive
   r0 = M[r3 + $basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.OP_BUFFER_FIELD];
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop B4;
   I4 = r0;
   L4 = r1;

   r10 = M[SP - ADDR_PER_WORD]; // load number of samples to process into r10;
   r1 = M[I6,0];                // load peak value
   // jump to the appropriate copy routine
   jump rMAC;

   done_copying:
   M[I6, ADDR_PER_WORD] = r1;  // store peak value for the current channel
   // The copy loop reads an extra sample from the input buffer, this needs to
   // be read back 1 sample, we can do this in parallel with the next operation so we do.

   // update read address in input buffer
   r0 = M[r3 + $basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.IP_BUFFER_FIELD], r2 = M[I0, M2];
   r1 = I0;
   call $cbuffer.set_read_address;

   // update write address in output buffer
   r0 = M[r3 + $basic_passthrough_struct.PASSTHROUGH_CHANNEL_struct.OP_BUFFER_FIELD];
   r1 = I4;
   call $cbuffer.set_write_address;

   // Decrement the channel counter
   r5 = r5 - 1;
   if NZ jump channel_loop;

all_channels_processed:
   pop r1;  // take samples to process back off the stack;

   popm <r5, r6>;
   popm <B0, B4>;
   popm <I0, I4, L0, L4>;
   pop rLink;

   rts;


// *****************************************************************************
// MODULE:
//    mono_gain_copy_loop
//    mono_gain_data_loop
//
// DESCRIPTION:
//    Data processing function. Processes a channel applying a gain
//
// INPUTS:
//    - I0, L0, B0 = Input buffer read index, length and base address
//    - I4, L4, B4 = Output buffer write index, length and base address
//    - r10 = number of samples to process
//    - r4 = input gain parameter
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    r2
//
// NOTES: REMOTE Buffers might work (for hydra) assuming they are accessed through a window,
//        but have not been tested.
//        It relies for now on imagined $cbuffer and $common modules' constants
//
// *****************************************************************************
mono_gain_copy_loop:
   // copy audio across with gain in r4
   r2 = M[I0, ADDR_PER_WORD];
   do pass_copy_loop;
      r2 = r2 * r4 (frac);
      r2 = r2 ASHIFT 4;
      r0 = ABS r2;                   // abs(input_value)
      r1 = MAX r0;                   // current peak
      M[I4, ADDR_PER_WORD] = r2, r2 = M[I0, ADDR_PER_WORD];
   pass_copy_loop:
   // This reads one extra sample from I0. This is stepped back in the main
   // loop as this opeation is common and saves a stall this way.
   jump done_copying;


mono_data_copy_loop:
   Null = I0 - I4;
   if Z jump no_copy;

   // copy unprocessed data across
   r2 = M[I0, ADDR_PER_WORD];
   do pass_copy_loop_data;
      Null = Null + Null;
      r0 = ABS r2;                   // abs(input_value)
      r1 = MAX r0;                   // current peak
      M[I4, ADDR_PER_WORD] = r2, r2 = M[I0, ADDR_PER_WORD];
   pass_copy_loop_data:
   // This reads one extra sample from I0. This is stepped back in the main
   // loop as this opeation is common and saves a stall this way.
   jump done_copying;

no_copy:
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

   jump done_copying;


// Alias these loops to C so that there location can be stored in the operator
// structure from C
.set $_mono_audio_loop, mono_gain_copy_loop
.set $_mono_data_loop, mono_data_copy_loop


// *****************************************************************************
// MODULE:
//    $log2_to_pow2_pos_input
//
// DESCRIPTION:
//    Convert log2() gain to linear domain when input is +ve
//
// INPUTS:
//    - r4 = log2() in q8.24
//
// OUTPUTS:
//    - r3 = linear gain in q5.27
//
// TRASHED REGISTERS:
//    - None
//
// *****************************************************************************
$log2_to_pow2_pos_input:

   push rLink;
   pushm <r0,r1,r2,r7>;

   r0 = r4;
   r3 = Qfmt_(1.0,5);          // 1.0 in q.27
   Null = r4;
   if Z jump bpt_pow2_calc_done;
   if NEG jump bpt_pow2_call;

gain_smooth_pos:
   r2 = r4 ASHIFT (8-DAWTH);   // integer
   r1 = r2 ASHIFT (DAWTH - 8); // integer in q.24(arch4)
   r4 = r4 - r1;               // fractional value in q.24(arch4)
   r1 = r2 + (DAWTH-4);        // integer+1 q.27(arch4)
   r0 = r4 - Qfmt_(1.0,8);     // negative fractional part
   r3 = 1 ASHIFT r1;           // pow2(integer_part) in q.27(arch4)

bpt_pow2_call:
   call $math.pow2_taylor;
   r3 = r3 * r0;

bpt_pow2_calc_done:
   popm <r0,r1,r2,r7>;
   pop rLink;
   rts;

.ENDMODULE;


