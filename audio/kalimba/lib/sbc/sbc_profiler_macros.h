// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef SBC_PROFILER_MACROS_H
#define SBC_PROFILER_MACROS_H

   // initialise profiling and macros if enabled
   #ifdef PROFILE_SBCDEC

      #define PROFILER_START_SBCDEC(addr)  \
         r0 = addr;                        \
         call $profiler.start;
      #define PROFILER_STOP_SBCDEC(addr)   \
         r0 = addr;                        \
         call $profiler.stop;

   #else
      #define PROFILER_START_SBCDEC(addr)
      #define PROFILER_STOP_SBCDEC(addr)
   #endif




   #ifdef PROFILE_SBCENC

      #define PROFILER_START_SBCENC(addr)  \
         r0 = addr;                        \
         call $profiler.start;
      #define PROFILER_STOP_SBCENC(addr)   \
         r0 = addr;                        \
         call $profiler.stop;

   #else
      #define PROFILER_START_SBCENC(addr)
      #define PROFILER_STOP_SBCENC(addr)
   #endif

#endif
