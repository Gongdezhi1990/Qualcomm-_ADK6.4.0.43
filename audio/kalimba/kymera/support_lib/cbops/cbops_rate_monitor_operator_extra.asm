/**************************************************************************/
/* Copyright (c) 2018 Qualcomm Technologies International, Ltd.           */
/**************************************************************************/
#include "stack.h"
#include "cbops.h"
#include "cbuffer_asm.h"
#include "cbops_rate_monitor_op_asm_defs.h"
#include "portability_macros.h"

#ifdef CAPABILITY_DOWNLOAD_BUILD
#ifndef RUNNING_ON_KALSIM
/* 3 extra fields for cbops rate monitor structure */
#define $RM_EXTRA_START ($cbops_rate_monitor_op.rate_monitor_op_struct.STRUC_SIZE*ADDR_PER_WORD)
#define $cbops_rate_monitor_op.rate_monitor_op_struct.NEW_AMOUNT_PTR_FIELD   ($RM_EXTRA_START+(0*ADDR_PER_WORD))
#define $cbops_rate_monitor_op.rate_monitor_op_struct.EXPECTED_ACC_SUM_FIELD ($RM_EXTRA_START+(1*ADDR_PER_WORD))
#define $cbops_rate_monitor_op.rate_monitor_op_struct.ACC_DRIFT_FIELD        ($RM_EXTRA_START+(2*ADDR_PER_WORD))
#endif
#endif
// *****************************************************************************
// MODULE:
//   void set_rate_monitor_new_amount_ptr(cbops_op *op, const void *new_amount)
//
// DESCRIPTION:
// configure rate monitor operator to receive new amount
// directly
//
// INPUTS:
//    - r0 = pointer to operator structure
//    - r1 = new_amount Pointer containing new amount for the operator
//           if not NULL, the operator will read new amount from this address
//           rather than using the cbops transfer amount
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    None
//
// *****************************************************************************
.MODULE $cbops.rate_monitor_op.set_rate_monitor_new_amount_ptr;
.CODESEGMENT PM;
.MAXIM;
$_set_rate_monitor_new_amount_ptr:
   NULL = r0;
   if Z rts;
   push r3;
   r3 = M[r0 +   ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
   M[r3 + $cbops_rate_monitor_op.rate_monitor_op_struct.NEW_AMOUNT_PTR_FIELD] = r1;
   pop r3;
   rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//   void rate_monitor_op_initialise(cbops_op *op, unsigned target,bool bHwSw,unsigned meas_period_msec);
//
// DESCRIPTION:
//   ADC/DAC rate matching amount to use functions
//
// INPUTS:
//    - r0 = pointer to operator structure
//    - r1 = Target Rate (Hz)
//    - r2 = mode
//    - r3 = measurement period in msec

//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    r0, r1
//
// *****************************************************************************
.MODULE $cbops.rate_monitor_op.rate_monitor_op_initialise;
.CODESEGMENT PM;
.MAXIM;
$_rate_monitor_op_initialise:
   push rLink;
   // run init function from ROM
   call $_rate_monitor_op_initiatlize;
   // then do the extra init
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.ACC_DRIFT_FIELD] = Null;
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.EXPECTED_ACC_SUM_FIELD] = Null;
   M[r0 + $cbops_rate_monitor_op.rate_monitor_op_struct.STALL_FIELD] = NULL;
   pop rLink;
   rts;
.ENDMODULE;
