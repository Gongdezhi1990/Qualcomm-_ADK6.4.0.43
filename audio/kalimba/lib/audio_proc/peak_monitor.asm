// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef _PEAK_MONITOR_INCLUDED
#define _PEAK_MONITOR_INCLUDED

#include "core_library.h"
#include "peak_monitor.h"
#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif
#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif


// *****************************************************************************
// MODULE:
//    $M.audio_proc.peak_monitor.Process.func
//
// DESCRIPTION:
//    This routine iterates through a frame of samples and finds and stores the
//    maximum magnitude.
//
// INPUTS:
//    - r7 = Data object
//
// OUTPUTS:
//    The peak value is written into the data object.
//
// TRASHED REGISTERS:
//    M1, I3, I0, L0, r0, r3, r4, r10
//
// *****************************************************************************
.MODULE $M.audio_proc.peak_monitor.Process;

   .CODESEGMENT   AUDIO_PROC_PEAK_MONITOR_PM;

func:

   push rLink;
   r0 = M[r7 + $M.audio_proc.peak_monitor.PTR_INPUT_BUFFER_FIELD];     
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0 = r0;
   L0 = r1;
   pop rLink;
   
   M1 = MK1;
   // Set frame size from input.  First Value
   r10 = r3,    r4 = M[I0,M1]; 
   
   // Current Peak
   r3 = M[r7 + $M.audio_proc.peak_monitor.PEAK_LEVEL];                                   
   do lp_calc_peak;
      // ABS
      r4 = ABS r4;
      // Peak, next value
      r3 = MAX r4, r4 = M[I0, M1];
   lp_calc_peak:
   L0 = Null;
   // New Peak
   M[r7 + $M.audio_proc.peak_monitor.PEAK_LEVEL] = r3;  
   
#ifdef BASE_REGISTER_MODE  
   push Null;
   pop B0;
#endif
   rts;
   
.ENDMODULE;

#endif //_PEAK_MONITOR_INCLUDED
