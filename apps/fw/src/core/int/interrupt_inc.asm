// *****************************************************************************
// Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
//   %%version
//
// $Change$  $DateTime$
// *****************************************************************************

#ifndef INTERRUPT_HEADER_INCLUDED
#define INTERRUPT_HEADER_INCLUDED

#ifdef DEBUG_ON
   #ifndef INTERRUPT_PROFILER_ON
      #define INTERRUPT_PROFILER_ON
   #endif
#endif

// Total stack use by interrupt handler, used in stack size calculation
.CONST $INTERRUPT_STORE_STATE_SIZE 47;

// Constants for stack-checking in the interrupt handler.
// Note that the "store" and "restore" sizes are different because there
// is already some data on the stack at the point of doing the "store" check.
.CONST $INTERRUPT_STORE_CHECK_SIZE 42;
.CONST $INTERRUPT_RESTORE_CHECK_SIZE 46;

#endif
