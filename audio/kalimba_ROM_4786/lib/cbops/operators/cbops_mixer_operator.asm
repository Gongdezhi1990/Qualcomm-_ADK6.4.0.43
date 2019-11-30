// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// Part of ADK 2.0
//
// *****************************************************************************


// *****************************************************************************
// NAME:
//    Signal Mix operator
//
// DESCRIPTION:
//    Mixes two signals and shifts output
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbuffer_asm.h"
#include "cbops_mixer_op_asm_defs.h"

// Private Library Exports
.PUBLIC $cbops.mixer_op;


.MODULE $M.cbops.mixer_op;
   .CODESEGMENT CBOPS_MIXER_OPERATOR_PM;
   .DATASEGMENT DM;

    // ** function vector **
   .VAR $cbops.mixer_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.basic_multichan_amount_to_use, // amount to use function
      &$cbops.mixer_op.main;                // main function


// *****************************************************************************
// MODULE:
//    $cbops.mixer_op.main
//
// DESCRIPTION:
//    Mix 2 signals and shift output
//
// INPUTS:
//    - r4 = pointer to internal framework object
//    - r8 = pointer to operator structure:
//    - r10 = data to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    rMAC, r0, r1, r4, r5, r6, r10, I0/L0/B0, I1/L1/B1, I4/L4/B4, M1, doLoop
//
// *****************************************************************************
$cbops.mixer_op.main:
    push rlink;

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.mixer_op.main.PATCH_ID_0, r1)
#endif

    // Make sure we are processing at least one sample
    r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];                         // input index (first channel)
    r1 = M[r8 + ($cbops.param_hdr.CHANNEL_INDEX_START_FIELD + 2*ADDR_PER_WORD)];     // Output index (first channel)
    call $cbops.get_transfer_and_update;    
    r10 = r0 - 1;
    if NEG jump jp_done;

    // Setup Buffers
    // Setup input & output. If/when needed, generalise this to N-to-1 downmixer.
    // Get the input/output indexes - these could be checked for unused channel marker,
    // but there would be zero point in using this specific operator
    // if any of its channels are unused. If/when generalised, then make sure indexes are
    // checked as inputs could be added or they could vanish after creation.
    r0 = M[r8 + ($cbops.param_hdr.CHANNEL_INDEX_START_FIELD + 0*ADDR_PER_WORD)];     // input 1 index
    call $cbops.get_buffer_address_and_length;
    I0 = r0;
    L0 = r1;
    push r2;
    pop B0;

    r0 = M[r8 + ($cbops.param_hdr.CHANNEL_INDEX_START_FIELD + 1*ADDR_PER_WORD)];     // input 2 index
    call $cbops.get_buffer_address_and_length;
    I1 = r0;
    L1 = r1;
    push r2;
    pop B1;

    r0 = M[r8 + ($cbops.param_hdr.CHANNEL_INDEX_START_FIELD + 2*ADDR_PER_WORD)];     // output index
    call $cbops.get_buffer_address_and_length;
    I4 = r0;
    L4 = r1;
    push r2;
    pop B4;

    // Setup mix ratio. Alg-specific params start after the 2+1 channel indexes
    r0 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
    M1 = MK1;
    r6 = M[r0 + $cbops_mixer_op.mixer_op_struct.SHIFT_FIELD];
    r5 = M[r0 + $cbops_mixer_op.mixer_op_struct.MIX_VALUE_FIELD];
    r4 = 1.0;
    r4 = r4 - r5,   r0 = M[I0,M1];

    // Mix signals
    rMAC = r0 * r5,         r0=M[I1,M1];
    rMAC = rMAC + r0 * r4;
    r1   = rMAC ASHIFT r6,  r0 = M[I0,M1];

    do lp_mix;
        rMAC = r0 * r5,         r0=M[I1,M1], M[I4,M1]=r1;
        rMAC = rMAC + r0 * r4;
        r1   = rMAC ASHIFT r6,  r0 = M[I0,M1];
    lp_mix:

    L0=0,   M[I4,M1]=r1;
    L1=0;
    L4=0;
    push NULL;
    B0 = M[SP - 1*ADDR_PER_WORD];
    B1 = M[SP - 1*ADDR_PER_WORD];
    pop B4;

jp_done:
    pop rlink;
    rts;


.ENDMODULE;
// Expose the location of this table to C
.set $_cbops_mixer_table,  $cbops.mixer_op
