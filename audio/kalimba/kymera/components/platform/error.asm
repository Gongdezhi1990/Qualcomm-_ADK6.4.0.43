// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

#ifndef ERROR_INCLUDED
#define ERROR_INCLUDED

#include "architecture.h"

// *****************************************************************************
// MODULE:
//    $error
//
// DESCRIPTION:
//    General-purpose error handler
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// NOTES
//    If exception handlers are enabled, the we'll jump to an appropriate one.
//    The handler isn't expected to return. 
//
//    Otherwise, once in $error you can look at rLink to
//    see where the error came from.
//
// *****************************************************************************
.MODULE $M.error;
   .CODESEGMENT ERROR_PM;

   $error:
   rFlags = rFlags AND $NOT_UM_FLAG;   // block interrupts quickly by coming
                                       // out of user mode
   // Are handlers enabled
   Null = M[$error_handlers_enabled];
   if Z jump error_loop;

   r0 = rLink;
   jump $_pl_lib_error_handler;

   error_loop:
      // if xIDE is attached let it hit a breakpoint so that
      // the user knows we've gone into error
      break;
   jump error_loop;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $error_unregistered
//
// DESCRIPTION:
//    Unregistered-interrupt error handler
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    none
//
// NOTES
//    If exception handlers are enabled, the we'll jump to an appropriate one.
//    The handler isn't expected to return. 
//
//    Otherwise, once in $error_unregistered you can look at rLink to
//    see where the error came from.
//
// *****************************************************************************
.MODULE $M.error_unregistered;
   .CODESEGMENT ERROR_PM;

   $error_unregistered:
   rFlags = rFlags AND $NOT_UM_FLAG;   // block interrupts quickly by coming
                                       // out of user mode
   // Are handlers enabled
   Null = M[$error_handlers_enabled];
   if NZ jump $_pl_unknown_interrupt_handler;

   error_loop:
      // if xIDE is attached let it hit a breakpoint so that
      // the user knows we've gone into error
      break;
   jump error_loop;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $error_sw_exception
//
// DESCRIPTION:
//    Software exception error handler
//    This is only used for memory-access exceptions
//    Stack overflow exceptions are filtered out in the interrupt handler
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// NOTES
//    If exception handlers are enabled, the we'll jump to an appropriate one.
//    The handler isn't expected to return. 
//
//    Otherwise, once in $error_sw_exception you can look at rIntLink to
//    see where the error came from.
//
// *****************************************************************************
.MODULE $M.error_sw_exception;
   .CODESEGMENT ERROR_PM;

   $error_sw_exception:
   rFlags = rFlags AND $NOT_UM_FLAG;   // block interrupts quickly by coming
                                       // out of user mode

   // Get exception address to pass to the handler
#if CHIP_HAS_EXCEPTION_PC
   // This points exactly at the instruction that caused the
   // exception.
   r0 = M[$EXCEPTION_PC];
#else
   // rIntLink (PC when the exception interrupted execution) is the
   // best we have. It is likely to be a few instructions on from the
   // instruction that caused the exception (more if interrupts were
   // blocked).
   r0 = M[$MM_RINTLINK];
#endif

   // Are handlers enabled
   Null = M[$error_handlers_enabled];
   if Z jump error_loop;
   
   // Handler records/reports r0
   jump $_pl_sw_exception_handler;

   error_loop:
      // if xIDE is attached let it hit a breakpoint so that
      // the user knows we've gone into error
      break;
   jump error_loop;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $error_stack_overflow
//
// DESCRIPTION:
//    Stack overflow error handler
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r9
//
// NOTES
//    If exception handlers are enabled, the we'll jump to an appropriate one.
//    The handler isn't expected to return. 
//
//    Otherwise, once in $error_stack_overflow you can look at rIntLink to
//    see where the error came from.
//
// *****************************************************************************
.MODULE $M.error_stack_overflow;
   .CODESEGMENT ERROR_PM;
   .DATASEGMENT DM;

   // In units of words
#define EXCEPTION_STACK_SIZE 32

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
   // Local stack for the exception handler
   .VAR/DM2_P0_ZI $error_stack_exception.stack[EXCEPTION_STACK_SIZE];
    // exception stack handler can be out of private ram 
   .VAR/DM2_P1_ZI $error_stack_exception.stack_p1[EXCEPTION_STACK_SIZE];
#else
   .VAR  $error_stack_exception.stack[EXCEPTION_STACK_SIZE];
#endif

   $error_stack_overflow:
   rFlags = rFlags AND $NOT_UM_FLAG;   // block interrupts quickly by coming
                                       // out of user mode


#ifdef AUDIO_SECOND_CORE
   // Set up the local stack 
   Null = M[$PROCESSOR_ID]; 
   if Z jump set_p0_stack;
      r0 = &$error_stack_exception.stack_p1 + (EXCEPTION_STACK_SIZE-1)*ADDR_PER_WORD;
      r9 = &$error_stack_exception.stack_p1;
      jump set_stack;

   set_p0_stack:
#endif // AUDIO_SECOND_CORE

      r0 = &$error_stack_exception.stack + (EXCEPTION_STACK_SIZE-1)*ADDR_PER_WORD;
      r9 = &$error_stack_exception.stack;
  
   set_stack:
   M[$STACK_END_ADDR] = r0;
   M[$STACK_START_ADDR] = r9;

   // Save existing stack pointer
   r0 = M[$STACK_POINTER];
   M[$STACK_POINTER] = r9;

   // Put original stack pointer on the exception stack 
   push r0;

   // Get overflow address to pass to the handler
   r0 = M[$STACK_OVERFLOW_PC];

   // Are handlers enabled
   Null = M[$error_handlers_enabled];
   if NZ jump $_pl_stack_exception_handler;

   error_loop:
      // if xIDE is attached let it hit a breakpoint so that
      // the user knows we've gone into error
      break;
   jump error_loop;
.ENDMODULE;

#ifdef CHIP_BASE_A7DA_KAS
// *****************************************************************************
// MODULE:
//    $error_apb_slv_error
//
// DESCRIPTION:
//    Keyhole error handler
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// NOTES
//    If exception handlers are enabled, the we'll jump to an appropriate one.
//    The handler isn't expected to return.
//
//    Otherwise, once in $error you can look at rLink to
//    see where the error came from.
//
// *****************************************************************************
.MODULE $M.error_apb_slv_error;
   .CODESEGMENT ERROR_PM;

   $error_apb_slv_error:
   rFlags = rFlags AND $NOT_UM_FLAG;   // block interrupts quickly by coming
                                       // out of user mode
   // Are handlers enabled
   Null = M[$error_handlers_enabled];
   if Z jump error_loop;

   jump $_pl_apb_slv_error;

   error_loop:
      // if xIDE is attached let it hit a breakpoint so that
      // the user knows we've gone into error
      break;
   jump error_loop;
.ENDMODULE;
#endif /* CHIP_BASE_A7DA_KAS */

// *****************************************************************************
// MODULE:
//    $error.enable_exception_handlers
//
// DESCRIPTION:
//    Enable handler functions for memory exceptions
//    We need this because there's a chance we may handle an exception early in 
//    the boot sequence, before we're ready to deal with it (i.e. the sssm has
//    finished and panic is initialised).
//
// INPUTS:
//    - r0 contains TRUE/FALSE (0 or 1) to disable/enable exception handlers.
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - none
//
// *****************************************************************************
.MODULE $M.error.enable_exception_handlers;
   .CODESEGMENT EXT_DEFINED_PM;
   .DATASEGMENT DM;
   
   .VAR $error_handlers_enabled = 0;

   $_error_enable_exception_handlers:
   $error.enable_exception_handlers:
   M[$error_handlers_enabled] = r0;
   rts;

.ENDMODULE;

#endif
