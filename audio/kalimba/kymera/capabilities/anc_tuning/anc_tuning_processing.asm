// *****************************************************************************
//  Copyright (c) 2015 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//  Notifications and licenses are retained for attribution purposes only
// %%version
//
// $Change: 2763723 $  $DateTime: 2017/04/10 19:05:48 $
// *****************************************************************************

// ASM function ANC_TUNING capability
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "portability_macros.h"
#include "stack.h"
#include "cbuffer_asm.h"
#include "anc_tuning_defs_asm_defs.h"

// *****************************************************************************
// MODULE:
//    $_anc_tuning_processing
//
// DESCRIPTION:
//    TODO - function description 
//
// INPUTS:
//    - r0 = pointer to tuning operator structure
//    - r1 = samples available to process
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.anc_tuning_proc;
    .DATASEGMENT DM;
    .CODESEGMENT PM;
    .MAXIM;

    .VAR gen_silence_data = 0;

// void anc_tuning_processing(ANC_TUNING_OP_DATA *p_ext_data,unsigned num_samples);
$_anc_tuning_processing:

   pushm <r7, r8, r9, rLink>; 
   pushm <I0, I4>;        
   pushm <M0, M1, M2, L0, L4>;
   pushm <B0,B4>;

   r9 = r0; // extra operator data
   r8 = r1; // samples available
//-----------------------------------------------------------------------------
// cycle through linked list of sinks
// call get read address
// cache results inside data obj (read ptr, base address, size)
//-----------------------------------------------------------------------------

   r7 = M[r9 + $anc_tuning_defs.anc_tuning_exop_struct.FIRST_SINK_FIELD];
   anc_tuning_process_channels_next_sink:
        r0 = M[r7 + $anc_tuning_defs.anc_sink_struct.BUFFER_FIELD];
        call $cbuffer.get_read_address_and_size_and_start_address; 
        M[r7 + $anc_tuning_defs.anc_sink_struct.READ_PTR_FIELD]     = r0;
        M[r7 + $anc_tuning_defs.anc_sink_struct.BUFFER_SIZE_FIELD]  = r1;
        M[r7 + $anc_tuning_defs.anc_sink_struct.BUFFER_START_FIELD] = r2;
    r7 = M[r7 + $anc_tuning_defs.anc_sink_struct.NEXT_FIELD];
    if NZ jump anc_tuning_process_channels_next_sink;

//-----------------------------------------------------------------------------
// cycle through linked list of sources
// call get write address
// setup copy

// if sink_ptr!=NULL
//      use read info from sink
// else
//      use dummy global variable for silence
// copy read to write
// update write address for sources
//-----------------------------------------------------------------------------
    r7 = M[r9 + $anc_tuning_defs.anc_tuning_exop_struct.FIRST_SOURCE_FIELD];
    anc_tuning_process_channels_next_source:
        r0 = M[r7 + $anc_tuning_defs.anc_source_struct.BUFFER_FIELD];
        call $cbuffer.get_write_address_and_size_and_start_address;
        I4 = r0;
        L4 = r1;
        push r2;
        pop B4;

        r1 = M[r7 + $anc_tuning_defs.anc_source_struct.SINK_FIELD];
        if Z jump transfer_silence;
            r0 = M[r1 + $anc_tuning_defs.anc_sink_struct.READ_PTR_FIELD];
            I0 = r0;
            r0 = M[r1 + $anc_tuning_defs.anc_sink_struct.BUFFER_SIZE_FIELD];
            L0 = r0;
            r0 = M[r1 + $anc_tuning_defs.anc_sink_struct.BUFFER_START_FIELD];
            jump transfer_data;
        transfer_silence:
            // Dummy Silence buffer
            r0 = &gen_silence_data;
            I0 = r0;
            L0 = MK1;
        transfer_data:
        
        r10 = r8-1;
        push r0;
        pop B0;
        r0 = M[I0,MK1];    
        r2 = M[r7 + $anc_tuning_defs.anc_source_struct.PEAK_FIELD];
        do copy_loop;
            r1 = ABS r0;
            r2 = MAX r1;
            r0=M[I0,MK1],  M[I4,MK1]=r0;
        copy_loop:
        M[I4,MK1]=r0;    
        M[r7 + $anc_tuning_defs.anc_source_struct.PEAK_FIELD]=r2;
        
        r0 = M[r7 + $anc_tuning_defs.anc_source_struct.BUFFER_FIELD];
        r1 = I4;
        call $cbuffer.set_write_address;

    r7 = M[r7 + $anc_tuning_defs.anc_source_struct.NEXT_FIELD];
    if NZ jump anc_tuning_process_channels_next_source;

//-----------------------------------------------------------------------------
// cycle through linked list for sinks
// setup buffer from cached values
// advance read ptr by amount
// update read address for sinks
//-----------------------------------------------------------------------------
   // r8 to octects
   Words2Addr(r8);
   M1 = r8;

   r7 = M[r9 + $anc_tuning_defs.anc_tuning_exop_struct.FIRST_SINK_FIELD];
   anc_tuning_process_channels_next_sink2:
        r0 = M[r7 + $anc_tuning_defs.anc_sink_struct.READ_PTR_FIELD];
        I0 = r0;
        r0 = M[r7 + $anc_tuning_defs.anc_sink_struct.BUFFER_SIZE_FIELD];
        L0 = r0;
        r0 = M[r7 + $anc_tuning_defs.anc_sink_struct.BUFFER_START_FIELD];
        push r0;
        pop B0;
        // I0,L0,B0
        r0 = M[I0,M1];
        r0 = M[r7 + $anc_tuning_defs.anc_sink_struct.BUFFER_FIELD];
        r1 = I0;
        call $cbuffer.set_read_address;
    r7 = M[r7 + $anc_tuning_defs.anc_sink_struct.NEXT_FIELD];
    if NZ jump anc_tuning_process_channels_next_sink2;

   popm <B0,B4>;
   popm <M0, M1, M2, L0, L4>;
   popm <I0, I4>; 
   popm <r7, r8, r9, rLink>;          
   rts;

.ENDMODULE;
