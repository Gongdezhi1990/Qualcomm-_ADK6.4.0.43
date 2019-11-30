// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// 
//
// $Change$  $DateTime$
// *****************************************************************************

#ifndef PROFILER_HEADER_INCLUDED
#define PROFILER_HEADER_INCLUDED

#ifdef PROFILER_ON
#include "profiler_c_asm_defs.h"
#endif

   // r0 - r2 are explicitly saved and restored on the stack, since some profiler usages require this.
   #if defined(ENABLE_PROFILER_MACROS)
      #define PROFILER_START_ASM(addr)    \
         push r0;                      \
         push r1;                      \
         r0 = addr;                    \
         call $profiler.start;         \
         pop r1;                       \
         pop r0;

      #define PROFILER_STOP_ASM(addr)     \
         push r0;                      \
         push r1;                      \
         push r2;                      \
         r0 = addr;                    \
         call $profiler.stop;          \
         pop r2;                       \
         pop r1;                       \
         pop r0;

   #else
      #define PROFILER_START(addr)
      #define PROFILER_STOP(addr)
   #endif



   // set the maximum possible number of handlers - this is only used to detect
   // corruption in the linked list, and so can be quite large
   .CONST   $profiler.MAX_PROFILER_HANDLERS   50;

   .CONST   $profiler.LAST_ENTRY              0;

   // by initialising profiler structures to the value of $profiler.UNINITIALISED
   // it will indicate to $profiler.start that the stucture needs registering
   // and so it will automatically call $profiler.register the frst time that
   // it is used.
   .CONST   $profiler.UNINITIALISED           -1;



// *****************************************************************************
// MODULE:
//    CODE_COVERAGE_DEF(x)
//
// DESCRIPTION:
//    Macro to create a code coverage variable.
//    Insert this macro in the variable region of a module that includes test points
//
// INPUTS:
//    - x = Number of entry points in module
//
// OUTPUTS:
//    -
//
// TRASHED REGISTERS:
//    none
//
// NOTES:
//       Do not terminate macro with a ';' semi-colon
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    CODE_COVERAGE_ENTRY(x)
//
// DESCRIPTION:
//    Macro to increment a code coverage variable
//    Insert this macro into code where you want to test cverage
//
// INPUTS:
//    - x = Index (Zero-Based) of code coverage variable in module
//
// OUTPUTS:
//    -
//
// TRASHED REGISTERS:
//    none
//
// NOTES:
//       Do not terminate macro with a ';' semi-colon
//       These macros are used in conjunction with the Matlab utility functions
//       kalcoverage_reset and kalcoverage which zero all coverage variables
//       and read all coverage variables respectively
// *****************************************************************************
#ifdef DEBUG_ON
   #define CODE_COVERAGE_DEF(x)    \
      .VAR CODE_COVERAGE[x] = 0

   #define CODE_COVERAGE_ENTRY(x)  \
      push r0;               \
      r0 = M[CODE_COVERAGE + x];    \
      r0 = r0 + 1;                  \
      M[CODE_COVERAGE + x] = r0;    \
      pop r0

#else // DEBUG_ON

   #define CODE_COVERAGE_DEF(x)
   #define CODE_COVERAGE_ENTRY(x)

#endif // DEBUG_ON


#endif // PROFILER_HEADER_INCLUDED
