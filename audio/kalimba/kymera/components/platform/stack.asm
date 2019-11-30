// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Stack Library
//
// DESCRIPTION:
//    This library has functions and macros to provide a simple software
//    stack.
//
//    The Kalimba DSP has an rLink register that is used to hold a subroutine
//    return address.  When a subroutine call is performed by a CALL
//    instruction, rLink is set to the subroutine return address.  The
//    subroutine return is preformed by an RTS instruction, its effect is
//    equivalent to a JUMP to the value of the rLink register.
//
//    When a subroutine is nested within another, a method of saving and
//    restoring the value of the rLink register at the start and end of the
//    subroutine is required.  A software stack that allows pushing and
//    popping the rLink register is the most convienient way to achieve this.
//    This functionality is supported by the Stack library.
//
//    The default size of the stack is 32 words, this may be changed by editing
//    the definition of $stack.SIZE and rebuilding the library
//
//    When developing applications it is useful to be able to catch stack
//    overflows or underflows to find bugs in a program.  By defining
//    DEBUG_STACK_OVERFLOW macro at the top of your application source code
//    extra code is inserted to detect stack over and underflows.
//
//
//    Planned use is as follows:
//    // note. this shows legacy (pre-BC5, i.e. KAL_ARCH2) use of macros
//    @verbatim
//    .MODULE $test;
//
//       // push rLink onto stack
//       push rLink;
//       // developers are encouraged to break away with old traditions and just
//       // push rLink;
//
//       ...    // your code
//       ...
//
//       // pop rLink from stack
//       jump $pop_rLink_and_rts;
//       // developers are encouraged to break away with old traditions and just
//       // pop rLink;
//       // rts;
//
//    .ENDMODULE;
//    @endverbatim
//    TODO_CONSIDER_DELETE the above use note when code is mature enough
//
// *****************************************************************************
#ifndef STACK_INCLUDED
#define STACK_INCLUDED

// FIXME: having these hardcoded on causes extra power consumption, so
// arguably this should be run-time configurable.
// However, Dave Hargreaves says that the likely power penalty is
// negligible (2014-09).
#define ENABLE_DEBUG_PERFORMANCE_COUNTERS

#ifdef DEBUG_ON
   #ifndef DEBUG_STACK_OVERFLOW
   #define DEBUG_STACK_OVERFLOW
   #endif
#endif

#include "stack.h"
#include "architecture.h"
#include "interrupt.h"
#include "portability_macros.h"

.MODULE $stack;
#ifdef STACK_IN_DM2
    // For some test builds we put the stack in DM2 RAM to make everything fit
   .DATASEGMENT STACK_DM2;
#else
   .DATASEGMENT STACK_DM;
#endif

   .CONST C_EXTRA_STACK 400;

   // $stack.SIZE has units of words
   .CONST   $stack.SIZE  32 + $INTERRUPT_STORE_STATE_SIZE*NUM_USED_INTERRUPT_LEVELS
                            + C_EXTRA_STACK*NUM_USED_TASK_PRIORITIES;

   // stack buffer should not be allocated at address 0; so allocated in the ZI region.
   .VAR     buffer[$stack.SIZE];

#ifdef AUDIO_SECOND_CORE
    // using the same stack size for P1 as well.
   .VAR/STACK_P1_DM  p1_buffer[$stack.SIZE];
#endif // AUDIO_SECOND_CORE 

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $stack.initialise
//
// DESCRIPTION:
//    Initialise the software stack
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
// NOTE
//    we are deliberately moving away from backward compatibility with BC3 (KAL_ARCH1)
//    (kymera doesn't even support BC5, KAL_ARCH2 that has SP and doesn't use r9)
//    we still keep the pop_rLink_and_rts (below) since it is used extensively
//    throughout kalimba libs (also all the macros defined in stack.h)
//    TODO_CONSIDER_DELETE this note when code is mature enough
//
// *****************************************************************************

.MODULE $M.stack.initialise;
   .CODESEGMENT STACK_INITIALISE_PM;
   .DATASEGMENT DM;

   #ifdef DEBUG_ON
      .CONST MAGIC_KEY    0x445248;
      .VAR stack_initialised_magic_key = MAGIC_KEY;
   #endif

   $stack.initialise:
   $_stack_initialise:

   #ifdef DEBUG_ON
      // The stack should only be initialised once - we check that it
      // hasn't been initialised before by confirming the magic key is set.
      r0 = M[stack_initialised_magic_key];
      Null = r0 - MAGIC_KEY;
      // If error is called here it will most probably be because of memory
      // corruption in the stack buffer or some rogue code jumping to address 0
      // or the like.
      if NZ call $error;
      M[stack_initialised_magic_key] = Null;
   #endif

#ifdef AUDIO_SECOND_CORE
   r0 = M[$PROCESSOR_ID];
   if Z jump stack_p0_set;
    
   r1 = &$stack.p1_buffer + ($stack.SIZE-1)*ADDR_PER_WORD;
   r0 = &$stack.p1_buffer;
   jump stack_set_ok;
 
    stack_p0_set:
#endif // AUDIO_SECOND_CORE
   r1 = &$stack.buffer + ($stack.SIZE-1)*ADDR_PER_WORD;
   r0 = &$stack.buffer;

   stack_set_ok:
   M[$STACK_END_ADDR] = r1;
   M[$STACK_START_ADDR] = r0;
   M[$STACK_POINTER] = r0;
   M[$FRAME_POINTER] = r0;

   // Zero the stack contents
   I0 = r0;
   r10 = $stack.SIZE;
   r1 = 0;
   
   do zero_stack_loop;
   M[I0, MK1] = r1;
zero_stack_loop:

   #if defined(ENABLE_DEBUG_PERFORMANCE_COUNTERS)
       // enable the performance counters in debug mode
       r0 = 1;
       M[$DBG_COUNTERS_EN] = r0;
   #endif
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $stack.pop_rLink_and_rts
//
// DESCRIPTION:
//    Shared code to pop rLink from the stack and rts.
//    Important: Code must be called with a jump instruction rather than a call.
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
// *****************************************************************************
.MODULE $M.pop_rLink_and_rts;
   .CODESEGMENT POP_RLINK_AND_RTS_PM;

   $pop_rLink_and_rts:
   pop rLink;
   rts;
.ENDMODULE;


#endif
