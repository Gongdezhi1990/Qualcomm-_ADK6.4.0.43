// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Port Synchronization operator
//
// DESCRIPTION:
//    The CBOPS library copies the minimum of the amount of data in the input
// buffers and the amount of space in the output buffers. For the free
// running external IO we want to maintain the minimum amount of data in the port
// to minimize latency.  On the other hand we need to maintain sufficient data in
// the port to prevent a wrap condition where the read pointer (limit) passes the write
// pointer (offset).
//
//  The logic is straight forward.
//      1) Compute the amount of data in the primry port
//      2) Limit the transfer to request to between max_advance and 2*max_advance
//      3) Verify that at least max_advance data is in port after trasfer is complete
//         
//
//  This operator must be the last operator in the cbops graph to ensure proper operation
//
// *****************************************************************************


#include "stack.h"
#include "cbops.h"
#include "cbuffer_asm.h"
#include "cbops_port_wrap_op_asm_defs.h"
#include "cbops_c_asm_defs.h"

// Private Library Exports
.PUBLIC $cbops.port_wrap_op;

.MODULE $M.cbops.port_wrap_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.port_wrap_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,      // reset function
      &$cbops.port_wrap_op.amount_to_use,      // amount to use function
      $cbops.function_vector.NO_FUNCTION;      // main function

.ENDMODULE;

.MODULE $M.cbops.port_wrap_op;
   .CODESEGMENT CBOPS_PORT_WRAP_OPERATOR_PM;
/*****************************************************************************
* MODULE:
*   $cbops.port_wrap_op.amount_to_use
*
* DESCRIPTION:
*   Limit amount of data in port(s) between one and two periods
*   The Period is defined in the MAX_ADVANCE_FIELD parameter
*
* INPUTS:
*    - r4 = pointer to buffer tabele
*    - r8 = pointer to operator structure
*
* OUTPUTS:
*    none (r8) preserved
*
* TRASHED REGISTERS:
*    r0 - r10
*
******************************************************************************/
$cbops.port_wrap_op.amount_to_use:
    push rLink;
    // If first port isn't connected, then do nothing. NOTE that it has zero ins and outs, and
    // zero indexes... so we can get to the parameters by just skipping the first two header fields
    // that state nr ins and nr outs.

    r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
    call $cbops.get_cbuffer;
    NULL = r0;
    if Z jump $pop_rLink_and_rts;

    r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
    // r5 is min(r6,r7)
    // r6 is data
    // r7 is space
    // First Port (r0)
    
    r5 = NULL;
    call calc_dac_amount_of_data;
    // r2 is data in port (negative if wrap), r10 is max advance, r0 is adjustment
    
    // Limit amount of data after transfer to two times maximum advance
    r10 = M[r7 + $cbops_port_wrap_op.port_wrap_op_struct.MAX_ADVANCE_FIELD];
    r3  = r10 ASHIFT 1;
    NULL = r2 + r0;
    if POS r1 = r3 - r2;
    if NEG r1 = Null;

    // Setup the limited transfer
    NULL = r5 - r1;
    if POS r5 = r1;
    r7 = r5;
 
    jump $pop_rLink_and_rts;
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
//    - r5 = amount_written 
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
#else
    // BlueCore: r2 is MMU buffer size in bytes
    r2  = r2 LSHIFT -1;
#endif

    // Adjust space for amount written
#if !defined(CHIP_BASE_BC7) || !defined (RUNNING_ON_KALSIM)     
    r0 = r0 - r5;
#endif

    // r0 is space in port minus one
    r2  = r2 - r0;
    Null = r0 - r2;
    if NEG r2 = Null - r0;

    r0 = M[r7 + $cbops_port_wrap_op.port_wrap_op_struct.BUFFER_ADJ_FIELD];
    r2 = r2 - r0;
    // r2 is number of samples in port, negative if overflow
    jump $pop_rLink_and_rts;

.ENDMODULE;

// Expose the location of the func table to C
.set $_cbops_port_wrap_table,  $cbops.port_wrap_op
