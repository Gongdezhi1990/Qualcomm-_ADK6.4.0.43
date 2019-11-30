
// *****************************************************************************
// Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.       
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM functions for processing source
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "source_sync_defs_asm_defs.h"

.MODULE $M.src_sync_process_mod;
    .DATASEGMENT DM;
    .CODESEGMENT PM;

// *****************************************************************************
//    $_src_sync_transfer_route
//    void src_sync_transfer_route(src_ptr, sink_ptr->buffer, transfer)
//
// INPUTS:
//      r0 = SRC_SYNC_SOURCE_ENTRY* src_ptr
//      r1 = tCbuffer*              sink_buffer
//      r2 = unsigned               transfer  words to copy from sink to source
//
// OUTPUTS:
//      none
// DESCRIPTION:
//    Perform transfer for one route.  Function is C compatible.
//    This is the inner loop code from $_src_sync_perform_transfer.
//
// *****************************************************************************

$_src_sync_transfer_route:
    pushm <FP(=SP),r4,r6,r7,rLink>;
    pushm <I0,I4>;
    pushm <L0,L4>;
    pushm <B0,B4>;

    /* Rearrange registers as the older code allocated them */
    r7 = r0; // src_ptr
    r6 = r1; // sink_buffer
    r4 = r2; // amount_data

    // r4=transfer, r6=sink_buffer
    // Get Source Buffer
    r0 = M[r7 + $source_sync_defs.src_sync_source_entry_struct.COMMON_FIELD +
                $source_sync_defs.src_sync_terminal_entry_struct.BUFFER_FIELD];
    call $cbuffer.get_write_address_and_size_and_start_address;
    I4 = r0;
    L4 = r1;
    push r2;
    pop  B4;

    r0 = r6;
    call $cbuffer.get_read_address_and_size_and_start_address;
    I0 = r0;
    L0 = r1;
    push r2;
    pop  B0;

    // Get gain
    r2 = M[r7 + $source_sync_defs.src_sync_source_entry_struct.CURRENT_ROUTE_FIELD +
                $source_sync_defs.src_sync_route_entry_struct.GAIN_LIN_FIELD];

    // Prepare loop counter, common for normal / transition
    r10 = r4 - 1;

    r6 = M[r7 + $source_sync_defs.src_sync_source_entry_struct.INV_TRANSITION_FIELD];
    if NEG jump transition_out;   // transition to new sink
    if NZ  jump transition_in;    // transitioned to new sink

#ifdef KAL_ARCH4
    Null = r2 - 0x8000000;
#else
    Null = r2 - 0x80000;
#endif
    if NZ  jump transfer_with_gain;

    // Normal transfer without gain
    r0 = M[I0,M1];
    do unity_gain_transfer_loop;
        M[I4,M1] = r0,
          r0 = M[I0,M1];
    unity_gain_transfer_loop:
    M[I4,M1] = r0;

    // Update source buffer
    update_source_buf:
    r1 = I4;
    r0 = M[r7 + $source_sync_defs.src_sync_source_entry_struct.COMMON_FIELD +
                $source_sync_defs.src_sync_terminal_entry_struct.BUFFER_FIELD];
    call $cbuffer.set_write_address;

    popm <B0,B4>;
    popm <L0,L4>;
    popm <I0,I4>;
    popm <FP,r4,r6,r7,rLink>;
    rts;

    transfer_with_gain:
    // Normal transfer
    r1 = 4;       // Shift value, scale the max gain via r2 to 16
    r0 = M[I0,M1];
    rMAC = r0 * r2;
    do normal_transfer_loop;
        rMAC = rMAC ASHIFT r1 (56bit),
          r0 = M[I0,M1];
        rMAC = r0 * r2,
          M[I4,M1] = rMAC;
    normal_transfer_loop:
    rMAC = rMAC ASHIFT r1 (56bit);
    M[I4,M1] = rMAC;
    jump update_source_buf;

    transition_out:
    // r2 = gain, r10 = transfer-1, r6 = inv_transition (neg)

    r3 = M[r7 + $source_sync_defs.src_sync_source_entry_struct.TRANSITION_PT_FIELD];

    r0 = M[I0,M1];
    r3 = r3 + r6;           // transition_pt += inv_transition (neg)
    r3 = MAX NULL;
    r0 = r0 * r3 (frac);    // data * transition_pt^2
    r0 = r0 * r3 (frac);
    rMAC = r0 * r2;
    do transition_out_loop;
        rMAC = rMAC ASHIFT 4 (56bit);   // scale the max gain via r2 to 16
        r3 = r3 + r6,
          r0 = M[I0,M1];         // transition_pt += inv_transition (neg)
        r3 = MAX NULL;
        r0 = r0 * r3 (frac);    // data * transition_pt^2
        r0 = r0 * r3 (frac),
          M[I4,M1] = rMAC;
        rMAC = r0 * r2;
    transition_out_loop:
    rMAC = rMAC ASHIFT 4 (56bit);  // scale the max gain via r2 to 16
    M[I4,M1] = rMAC;

    M[r7 + $source_sync_defs.src_sync_source_entry_struct.TRANSITION_PT_FIELD] = r3;

    jump update_source_buf;

    transition_in:
    // r2 = gain, r10 = transfer-1, r6 = inv_transition (pos)

    r3 = M[r7 + $source_sync_defs.src_sync_source_entry_struct.TRANSITION_PT_FIELD];
    r1 = MAXINT;

    r0 = M[I0,M1];
    r3 = r3 + r6;           // transition_pt += inv_transition (pos) - saturate
    if V r3 = r1;
    r0 = r0 * r3 (frac);    // data * transition_pt^2
    r0 = r0 * r3 (frac);
    rMAC = r0 * r2;
    do transition_in_loop;
        rMAC = rMAC ASHIFT 4 (56bit);   // scale the max gain via r2 to 16
        r3 = r3 + r6,
          r0 = M[I0,M1];         // transition_pt += inv_transition (pos) - saturate
        if V r3 = r1;
        r0 = r0 * r3 (frac);    // data * transition_pt^2
        r0 = r0 * r3 (frac),
          M[I4,M1] = rMAC;
        rMAC = r0 * r2;
    transition_in_loop:
    rMAC = rMAC ASHIFT 4 (56bit);      // scale the max gain via r2 to 16
    M[I4,M1] = rMAC;

    M[r7 + $source_sync_defs.src_sync_source_entry_struct.TRANSITION_PT_FIELD] = r3;

    NULL = r3 - r1;
    if NZ jump update_source_buf;

        // Transition Complete
        M[r7 + $source_sync_defs.src_sync_source_entry_struct.INV_TRANSITION_FIELD] = NULL;
        jump update_source_buf;

.ENDMODULE;

