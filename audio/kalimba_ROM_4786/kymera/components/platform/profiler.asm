// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// *****************************************************************************
// NAME:
//    CPU usage Profiler
//
// DESCRIPTION:
//    This library contains functions to calculate the percentage of cpu
//    time that particular parts of an application are using.
//
//    A five word variable is used by the profiler to maintain book keeping
//    information. When registering an section of code for profiling, a pointer
//    is passed to $profiler.register of this variable.  Alternatively if the
//    variable has it's first location initialised to $profiler.UNINITIALISED
//    then you don't need to call $profiler.register.
//
//    To then profile an area of code, it should be surrounded by calls to
//    $profiler.start and $profiler.stop. Since these subroutines consume
//    some processing power, profiling of code that is commonly called but
//    quick to execute may give inaccurate results.
//
//    The profiling information is updated once a second and a value between
//    zero and one thousand stored in your_var[$profiler.CPU_FRACTION_FIELD].
//    One thousand equates to 100% cpu usage.
//
// *****************************************************************************

#ifndef PROFILER_INCLUDED
#define PROFILER_INCLUDED

#ifdef PROFILER_ON

#include "stack.h"
#include "interrupt.h"
#include "architecture.h"
#include "profiler.h"
#include "profiler_c_asm_defs.h"
#include "portability_macros.h"
#include "faultids_asm_defs.h"
#include "patch_asm_macros.h"

.MODULE $profiler;
   .DATASEGMENT DM;

   .VAR/DM1 last_addr = $profiler.LAST_ENTRY;
   .VAR     timer_id;
   #ifdef DETAILED_PROFILER_ON
      .VAR temp_clks_ls;
      .VAR temp_clks_ms;
      .VAR temp_instrs_ls;
      .VAR temp_instrs_ms;
   #endif
  .VAR sleep_time[$profiler_c.profiler_struct.STRUC_SIZE] = $profiler.UNINITIALISED,0 ...;
#ifdef PROFILE_ENABLED_AT_STARTUP
  .VAR enabled = 1;  /* Startup with profiling enabled */
#else
  .VAR enabled = 0;
#endif
  // 0 (or symbol absent) means unknown. On some platforms this may
  // always be zero (this tells any tool reading it that they need
  // to guess).
  .VAR cpu_speed_mhz = 0;

.ENDMODULE;

// Expose the location of these varibles to C
.set $_profiler_list,  $profiler.last_addr
.set $_sleep_time,  $profiler.sleep_time
.set $_profiler_timer_id,  $profiler.timer_id

// *****************************************************************************
// MODULE:
//    $profiler.initialise
//
// DESCRIPTION:
//    Initialise the profiler library
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r4, r5, r10, DoLoop
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.profiler.initialise;
   .CODESEGMENT PROFILER_INITIALISE_PM;

   $_profiler_initialise:
   $profiler.initialise:

   Null = M[$profiler.enabled];
   if Z rts;
   // push rLink onto stack
   push rLink;
   // initialise profiler timer handler for servicing profiler in 1.024secs
#ifdef K32
   r0 = 1024000;
   r1 = &$profiler.timer_service_routine;
   r2 = Null;
#else
   r0 = 0;
   r1 = 1024000;
   r2 = &$profiler.timer_service_routine;
   r3 = Null;
#endif

   call $_asm_timer_schedule_event_in;
   // Store the returned timer id if we need to cancel later
   M[$profiler.timer_id] = r0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $profiler.register
//
// DESCRIPTION:
//    Register a routine to be profiled
//
// INPUTS:
//    - r0 = pointer to a variable that stores the profiling structure,
//         should be of length $profiler.STRUC_SIZE
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2
//
// *****************************************************************************
.MODULE $M.profiler.register;
   .CODESEGMENT PROFILER_REGISTER_PM;

   $_profiler_register:
   $profiler.register:
   push rLink;
   r2 = r0;
   call $block_interrupts;
   // clear nesting count
   M[r2 + $profiler_c.profiler_struct.NEST_COUNT_FIELD]=NULL;
   // set next profiler address to the previous last_addr
   r1 = M[$profiler.last_addr];
   M[r2 + $profiler_c.profiler_struct.NEXT_FIELD] = r1;
   // set new last_addr to the address of this profiler structure
   M[$profiler.last_addr] = r2;
   call $unblock_interrupts;

   // clear TotalTime, and CPU_Faction fields
   M[r2 + $profiler_c.profiler_struct.CPU_FRACTION_FIELD] = Null;
   M[r2 + $profiler_c.profiler_struct.PEAK_CPU_FRACTION_FIELD] = Null;
   M[r2 + $profiler_c.profiler_struct.START_TIME_FIELD]= Null;
   M[r2 + $profiler_c.profiler_struct.INT_START_TIME_FIELD]= Null;
   M[r2 + $profiler_c.profiler_struct.TOTAL_TIME_FIELD] = Null;
   #ifdef DETAILED_PROFILER_ON
   #ifdef K32
      r1 = 1;
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_AVERAGE_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_AVERAGE_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.CORE_STALLS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.CORE_STALLS_AVERAGE_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.CORE_STALLS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.PREFETCH_WAIT_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.PREFETCH_WAIT_AVERAGE_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.PREFETCH_WAIT_MAX_FIELD] = Null;
      #ifdef CHIP_BASE_CRESCENDO
         M[r2 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_TOTAL_FIELD] = Null;
         M[r2 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_AVERAGE_FIELD] = Null;
         M[r2 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_MAX_FIELD] = Null;
         M[r2 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_TOTAL_FIELD] = Null;
         M[r2 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_AVERAGE_FIELD] = Null;
         M[r2 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_MAX_FIELD] = Null;
      #endif /* CHIP_BASE_CRESCENDO */
      // Performance counters are disabled by default on Crescendo - enable them here
      M[$DBG_COUNTERS_EN] = r1;
      M[$PREFETCH_CONFIG] = r1;
   #else
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_MS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_LS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_AVERAGE_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_MS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.RUN_CLKS_LS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_MS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_LS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_AVERAGE_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_MS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.INSTRS_LS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.STALLS_MS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.STALLS_LS_TOTAL_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.STALLS_AVERAGE_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.STALLS_MS_MAX_FIELD] = Null;
      M[r2 + $profiler_c.profiler_struct.STALLS_LS_MAX_FIELD] = Null;
   #endif
   #endif

   jump $pop_rLink_and_rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $profiler.start
//
// DESCRIPTION:
//    Start profiling of a particular routine
//
// INPUTS:
//    - r0 = address of structure to use for profiling
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r1, r2
//
// *****************************************************************************
.MODULE $M.profiler.start;
   .CODESEGMENT PROFILER_START_PM;
   $_profiler_start:
   $profiler.start:

   Null = M[$profiler.enabled];
   if Z rts;
   push rLink;

   push r0;
   call $block_interrupts;
   pop r0;

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($profiler.start.PATCH_ID_1, r1)

   r1 = M[r0 + $profiler_c.profiler_struct.NEXT_FIELD];
   Null = r1 - $profiler.UNINITIALISED;
   if NZ jump already_initialised;
      // the profiler is uninitialised so we'll automatically register it
      // push rLink onto stack
      push r0;
      call $profiler.register;
      pop r0;
   already_initialised:

   // Check if this profiler is currently active
   // Don't update anything if it is
   r1 = M[r0 + $profiler_c.profiler_struct.NEST_COUNT_FIELD];
   if NZ jump done;

   r1 = M[$TIMER_TIME];
   M[r0 + $profiler_c.profiler_struct.START_TIME_FIELD] = r1;

   #ifdef INTERRUPT_PROFILER_ON
      push r3;
      r2 = M[$interrupt.nest_count];

      r3 = $interrupt.MAX_NESTED_NR - 1;
      Null = r2 - r3;
      if GT r2 = r3;
      Words2Addr(r2);
      r1 = M[$interrupt.total_time_with_task + r2];
      M[r0 + $profiler_c.profiler_struct.INT_START_TIME_FIELD] = r1;
      pop r3;
   #endif

      r1 = M[r0 + $profiler_c.profiler_struct.TEMP_COUNT_FIELD];
      r1 = r1 + 1;
      M[r0 + $profiler_c.profiler_struct.TEMP_COUNT_FIELD] = r1;
         
   #ifdef DETAILED_PROFILER_ON
   #ifdef INTERRUPT_PROFILER_ON
      #ifdef K32
         push r3;

         r2 = M[$NUM_RUN_CLKS];
         r3 = M[$interrupt.total_clocks];
         M[r0 + $profiler_c.profiler_struct.RUN_CLKS_START_FIELD] = r2;
         M[r0 + $profiler_c.profiler_struct.INT_START_CLKS_FIELD] = r3;
         r2 = M[$NUM_INSTRS];
         r3 = M[$interrupt.total_instrs];
         M[r0 + $profiler_c.profiler_struct.INSTRS_START_FIELD] = r2;
         M[r0 + $profiler_c.profiler_struct.INT_START_INSTRS_FIELD] = r3;
         r2 = M[$NUM_CORE_STALLS];
         r3 = M[$interrupt.total_core_stalls];
         M[r0 + $profiler_c.profiler_struct.CORE_STALLS_START_FIELD] = r2;
         M[r0 + $profiler_c.profiler_struct.INT_START_CORE_STALLS_FIELD] = r3;
         r2 = M[$PREFETCH_WAIT_OUT_COUNT];
         r3 = M[$interrupt.total_prefetch_wait];
         M[r0 + $profiler_c.profiler_struct.PREFETCH_WAIT_START_FIELD] = r2;
         M[r0 + $profiler_c.profiler_struct.INT_START_PREFETCH_WAIT_FIELD] = r3;
         #ifdef CHIP_BASE_CRESCENDO
            r2 = M[$NUM_MEM_ACCESS_STALLS];
            r3 = M[$interrupt.total_access_stalls];
            M[r0 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_START_FIELD] = r2;
            M[r0 + $profiler_c.profiler_struct.INT_START_ACCESS_STALLS_FIELD] = r3;
            r2 = M[$NUM_INSTR_EXPAND_STALLS];
            r3 = M[$interrupt.total_instr_expand_stalls];
            M[r0 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_START_FIELD] = r2;
            M[r0 + $profiler_c.profiler_struct.INT_START_INSTR_EXPAND_STALLS_FIELD] = r3;
         #endif
         pop r3;
      #else
         push r3;
         r1 = r0;
         r0 = M[$NUM_RUN_CLKS_MS];
         r2 = M[$NUM_RUN_CLKS_LS];
         r3 = M[$NUM_RUN_CLKS_MS];
         r2 = M[$NUM_RUN_CLKS_LS];
         r2 = r2 - 2;
         r3 = r3 - Borrow;
         Null = r3 - r0;
         if LT r3 = r0;
         M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MS_START_FIELD] = r3;
         M[r1 + $profiler_c.profiler_struct.RUN_CLKS_LS_START_FIELD] = r2;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$interrupt.total_clocks_ms];
         M[r1 + $profiler_c.profiler_struct.INT_START_CLKS_MS_FIELD] = r0;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$interrupt.total_clocks_ls];
         M[r1 + $profiler_c.profiler_struct.INT_START_CLKS_LS_FIELD] = r0;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$NUM_INSTRS_MS];
         r2 = M[$NUM_INSTRS_LS];
         r3 = M[$NUM_INSTRS_MS];
         r2 = M[$NUM_INSTRS_LS];
         r2 = r2 - 2;
         r3 = r3 - Borrow;
         Null = r3 - r0;
         if LT r3 = r0;
         M[r1 + $profiler_c.profiler_struct.INSTRS_MS_START_FIELD] = r3;
         M[r1 + $profiler_c.profiler_struct.INSTRS_LS_START_FIELD] = r2;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$interrupt.total_instrs_ms];
         M[r1 + $profiler_c.profiler_struct.INT_START_INSTRS_MS_FIELD] = r0;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$interrupt.total_instrs_ls];
         M[r1 + $profiler_c.profiler_struct.INT_START_INSTRS_LS_FIELD] = r0;
         nop; // to guard against stall if profiler structure is in DM2
         r3 = M[$NUM_CORE_STALLS_MS];
         r2 = M[$NUM_CORE_STALLS_LS];
         M[r1 + $profiler_c.profiler_struct.STALLS_MS_START_FIELD] = r3;
         M[r1 + $profiler_c.profiler_struct.STALLS_LS_START_FIELD] = r2;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$interrupt.total_stalls_ms];
         M[r1 + $profiler_c.profiler_struct.INT_START_STALLS_MS_FIELD] = r0;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$interrupt.total_stalls_ls];
         M[r1 + $profiler_c.profiler_struct.INT_START_STALLS_LS_FIELD] = r0;
         pop r3;
         r0 = r1;
   #endif /* K32 */
   #endif /* INTERRUPT_PROFILER_ON */
   #endif /* DETAILED_PROFILER_ON */

   done:
   r1 = M[r0 + $profiler_c.profiler_struct.NEST_COUNT_FIELD];
   r1 = r1 + 1;
   M[r0 + $profiler_c.profiler_struct.NEST_COUNT_FIELD] = r1;

   call $unblock_interrupts;

   jump $pop_rLink_and_rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $profiler.stop
//
// DESCRIPTION:
//    Stop profiling of a particular routine
//
// INPUTS:
//    - r0 = address of structure to use for profiling
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r1, r2
//
// *****************************************************************************
.MODULE $M.profiler.stop;
   .CODESEGMENT PROFILER_STOP_PM;

   $_profiler_stop:
   $profiler.stop:

   Null = M[$profiler.enabled];
   if Z rts;
   push rLink;

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($profiler.stop.PATCH_ID_1, r1)

   r1 = r0;
   call $block_interrupts;  // 6 instructions, 7 cycles (0 BC5 style stall)

   // Check if this profiler needs to be updated
   r0 = M[r1 + $profiler_c.profiler_struct.NEST_COUNT_FIELD];
   r0 = r0 - 1;
   M[r1 + $profiler_c.profiler_struct.NEST_COUNT_FIELD] = r0;

   if NZ jump done;

   // TODO this might not hold true anymore since macros have been replaced for Amber
   // TotalTime  = TotalTime + (TimerTime - StartTime) - (delta_InterruptTime);
   r0 = M[r1 + $profiler_c.profiler_struct.TOTAL_TIME_FIELD];
   r2 = M[r1 + $profiler_c.profiler_struct.START_TIME_FIELD];
   r0 = r0 - r2;
   r2 = M[r1 + $profiler_c.profiler_struct.INT_START_TIME_FIELD];
   r0 = r0 + r2;
   #ifdef INTERRUPT_PROFILER_ON
      push r1;
      r2 = M[$interrupt.nest_count];
      r1 = $interrupt.MAX_NESTED_NR - 1;
      Null = r2 - r1;
      if GT r2 = r1;
      Words2Addr(r2);
      r1 = M[$interrupt.total_time_with_task + r2];
      r0 = r0 - r1;
      pop r1;
   #endif
   r0 = r0 + M[$TIMER_TIME];
   M[r1 + $profiler_c.profiler_struct.TOTAL_TIME_FIELD] = r0;

   #ifdef DETAILED_PROFILER_ON
   #ifdef INTERRUPT_PROFILER_ON
      push r3;

      #ifdef K32
         // run clocks calculation
         r3 = M[$NUM_RUN_CLKS];
         r0 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_START_FIELD];
         r2 = M[$interrupt.total_clocks];
         r0 = r3 - r0;
         r3 = M[r1 + $profiler_c.profiler_struct.INT_START_CLKS_FIELD];
         r0 = r0 - r2;
         r0 = r0 + r3;
         r3 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_TOTAL_FIELD];
         // Adjust number of clocks with the values:
         // 38 cycles - in $profiler.start before first M[$NUM_RUN_CLKS] read (assuming profiler already INITIALISED)
         // 66 cycles - beneath this point in $profiler.stop including the last calculations for M[$PREFETCH_WAIT_OUT_COUNT]
         // 34 cycles - beneath this point in $profiler.stop for D01 specific calculations 
         //             (M[$NUM_MEM_ACCESS_STALLS] and M[$NUM_INSTR_EXPAND_STALLS])
         // 14 cycles - beneath this point in $profiler.stop for unblocking interrupts, restoring the stack and returning
         //
         // As a worst case scenario we assume that the MAXIMUM field is updated on every pass. 
         #ifdef CHIP_BASE_CRESCENDO
            r2 = r0 - (38+66+34+14);
         #else
            r2 = r0 - (38+66+14);
         #endif
         r0 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MAX_FIELD];;
         r3 = r3 + r2;
         M[r1 + $profiler_c.profiler_struct.RUN_CLKS_TOTAL_FIELD] = r3;
         Null = r0 - r2;
         if GE jump not_rc_max;
            M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MAX_FIELD] = r2;
         not_rc_max:

         // num instructions calculation
         r3 = M[$NUM_INSTRS];
         r0 = M[r1 + $profiler_c.profiler_struct.INSTRS_START_FIELD];
         r2 = M[$interrupt.total_instrs];
         r0 = r3 - r0;
         r3 = M[r1 + $profiler_c.profiler_struct.INT_START_INSTRS_FIELD];
         r0 = r0 - r2;
         r0 = r0 + r3;
         r3 = M[r1 + $profiler_c.profiler_struct.INSTRS_TOTAL_FIELD];
         // Adjust number of intructions with the values:
         // 35 instructions - in $profiler.start before first M[$NUM_INSTRS] read (assuming profiler already INITIALISED)
         // 59 instructions - beneath this point in $profiler.stop including the last calculations for M[$PREFETCH_WAIT_OUT_COUNT]
         // 30 instructions - beneath this point in $profiler.stop for D01 specific calculations 
         //             (M[$NUM_MEM_ACCESS_STALLS] and M[$NUM_INSTR_EXPAND_STALLS])
         // 12 instructions - beneath this point in $profiler.stop for unblocking interrupts, restoring the stack and returning
         //
         // As a worst case scenario we assume that the MAXIMUM field is updated on every pass. 
         #ifdef CHIP_BASE_CRESCENDO
            r2 = r0 - (35+59+30+12);
         #else
            r2 = r0 - (35+59+12);
         #endif
         r0 = M[r1 + $profiler_c.profiler_struct.INSTRS_MAX_FIELD];;
         r3 = r3 + r2;
         M[r1 + $profiler_c.profiler_struct.INSTRS_TOTAL_FIELD] = r3;
         Null = r0 - r2;
         if GE jump not_i_max;
            M[r1 + $profiler_c.profiler_struct.INSTRS_MAX_FIELD] = r2;
         not_i_max:

         // core stalls calculation
         r3 = M[$NUM_CORE_STALLS];
         r0 = M[r1 + $profiler_c.profiler_struct.CORE_STALLS_START_FIELD];
         r2 = M[$interrupt.total_core_stalls];
         r0 = r3 - r0;
         r3 = M[r1 + $profiler_c.profiler_struct.INT_START_CORE_STALLS_FIELD];
         r0 = r0 - r2;
         r0 = r0 + r3;
         r3 = M[r1 + $profiler_c.profiler_struct.CORE_STALLS_TOTAL_FIELD];

         r2 = r0;
         r0 = M[r1 + $profiler_c.profiler_struct.CORE_STALLS_MAX_FIELD];;
         r3 = r3 + r2;
         M[r1 + $profiler_c.profiler_struct.CORE_STALLS_TOTAL_FIELD] = r3;
         Null = r0 - r2;
         if GE jump not_s_max;
            M[r1 + $profiler_c.profiler_struct.CORE_STALLS_MAX_FIELD] = r2;
         not_s_max:

         // prefetch wait calculation
         r3 = M[$PREFETCH_WAIT_OUT_COUNT];
         r0 = M[r1 + $profiler_c.profiler_struct.PREFETCH_WAIT_START_FIELD];
         r2 = M[$interrupt.total_prefetch_wait];
         r0 = r3 - r0;
         r3 = M[r1 + $profiler_c.profiler_struct.INT_START_PREFETCH_WAIT_FIELD];
         r0 = r0 - r2;
         r0 = r0 + r3;
         r3 = M[r1 + $profiler_c.profiler_struct.PREFETCH_WAIT_TOTAL_FIELD];

         r2 = r0;
         r0 = M[r1 + $profiler_c.profiler_struct.PREFETCH_WAIT_MAX_FIELD];;
         r3 = r3 + r2;
         M[r1 + $profiler_c.profiler_struct.PREFETCH_WAIT_TOTAL_FIELD] = r3;
         Null = r0 - r2;
         if GE jump not_pw_max;
            M[r1 + $profiler_c.profiler_struct.PREFETCH_WAIT_MAX_FIELD] = r2;
         not_pw_max:

         #ifdef CHIP_BASE_CRESCENDO
            // memory access stalls calculation
            r3 = M[$NUM_MEM_ACCESS_STALLS];
            r0 = M[r1 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_START_FIELD];
            r2 = M[$interrupt.total_access_stalls];
            r0 = r3 - r0;
            r3 = M[r1 + $profiler_c.profiler_struct.INT_START_ACCESS_STALLS_FIELD];
            r0 = r0 - r2;
            r0 = r0 + r3;
            r3 = M[r1 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_TOTAL_FIELD];

            r2 = r0;
            r0 = M[r1 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_MAX_FIELD];;
            r3 = r3 + r2;
            M[r1 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_TOTAL_FIELD] = r3;
            Null = r0 - r2;
            if GE jump not_mas_max;
               M[r1 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_MAX_FIELD] = r2;
            not_mas_max:

            // instruction expand stalls calculation
            r3 = M[$NUM_INSTR_EXPAND_STALLS];
            r0 = M[r1 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_START_FIELD];
            r2 = M[$interrupt.total_instr_expand_stalls];
            r0 = r3 - r0;
            r3 = M[r1 + $profiler_c.profiler_struct.INT_START_INSTR_EXPAND_STALLS_FIELD];
            r0 = r0 - r2;
            r0 = r0 + r3;
            r3 = M[r1 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_TOTAL_FIELD];

            r2 = r0;
            r0 = M[r1 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_MAX_FIELD];;
            r3 = r3 + r2;
            M[r1 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_TOTAL_FIELD] = r3;
            Null = r0 - r2;
            if GE jump not_ies_max;
               M[r1 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_MAX_FIELD] = r2;
            not_ies_max:

         #endif /* CHIP_BASE_CRESCENDO */
      #else
         push r4;
         r0 = M[$NUM_RUN_CLKS_MS];
         r2 = M[$NUM_RUN_CLKS_LS];
         r3 = M[$NUM_RUN_CLKS_MS];
         r2 = M[$NUM_RUN_CLKS_LS];
         r2 = r2 - 2;
         r3 = r3 - Borrow;
         Null = r3 - r0;
         if LT r3 = r0;
         r4 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_LS_START_FIELD];
         r0 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MS_START_FIELD];
         r2 = r2 - r4; // Instruction moved here to avoid resetting of flags from r0 = M[...]
         r0 = r3 - r0 - Borrow;
         r4 = M[$interrupt.total_clocks_ls];
         r3 = M[$interrupt.total_clocks_ms];
         r2 = r2 - r4; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         r0 = r0 - r3 - Borrow;
         r4 = M[r1 + $profiler_c.profiler_struct.INT_START_CLKS_LS_FIELD];
         r3 = M[r1 + $profiler_c.profiler_struct.INT_START_CLKS_MS_FIELD];
         r2 = r2 + r4; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         r0 = r0 + r3 + Carry;
         r2 = r2 - (53+18+3+5);        // extra clocks between NUM_RUN_CLKS_LS reads in profiler.start and profiler.stop
         r0 = r0 - Borrow;
         r3 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_LS_TOTAL_FIELD];
         r4 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MS_TOTAL_FIELD];
         r3 = r3 + r2; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         M[r1 + $profiler_c.profiler_struct.RUN_CLKS_LS_TOTAL_FIELD] = r3;
         r3 = r4 + r0 + Carry;
         M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MS_TOTAL_FIELD] = r3;
         M[$profiler.temp_clks_ls] = r2;
         M[$profiler.temp_clks_ms] = r0;
         nop; // to guard against stall if profiler structure is in DM2
         r0 = M[$NUM_INSTRS_MS];
         r2 = M[$NUM_INSTRS_LS];
         r3 = M[$NUM_INSTRS_MS];
         r2 = M[$NUM_INSTRS_LS];
         r2 = r2 - 2;
         r3 = r3 - Borrow;
         Null = r3 - r0;
         if LT r3 = r0;
         r4 = M[r1 + $profiler_c.profiler_struct.INSTRS_LS_START_FIELD];
         r0 = M[r1 + $profiler_c.profiler_struct.INSTRS_MS_START_FIELD];
         r2 = r2 - r4; // Instruction moved here to avoid resetting of flags from r0 = M[...]
         r0 = r3 - r0 - Borrow;
         r4 = M[$interrupt.total_instrs_ls];
         r3 = M[$interrupt.total_instrs_ms];
         r2 = r2 - r4; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         r0 = r0 - r3 - Borrow;
         r4 = M[r1 + $profiler_c.profiler_struct.INT_START_INSTRS_LS_FIELD];
         r3 = M[r1 + $profiler_c.profiler_struct.INT_START_INSTRS_MS_FIELD];
         r2 = r2 + r4; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         r0 = r0 + r3 + Carry;
         r2 = r2 - (35+48+3+5);        // extra instructions between NUM_INSTRS_LS reads in profiler.start and profiler.stop
         r0 = r0 - Null -  Borrow;
         r3 = M[r1 + $profiler_c.profiler_struct.INSTRS_LS_TOTAL_FIELD];
         r4 = M[r1 + $profiler_c.profiler_struct.INSTRS_MS_TOTAL_FIELD];
         r3 = r3 + r2; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         M[r1 + $profiler_c.profiler_struct.INSTRS_LS_TOTAL_FIELD] = r3;
         r3 = r4 + r0 + Carry;
         M[r1 + $profiler_c.profiler_struct.INSTRS_MS_TOTAL_FIELD] = r3;
         M[$profiler.temp_instrs_ls] = r2;
         M[$profiler.temp_instrs_ms] = r0;
         nop; // to guard against stall if profiler structure is in DM2
         r3 = M[$NUM_CORE_STALLS_MS];
         r2 = M[$NUM_CORE_STALLS_LS];
         r4 = M[r1 + $profiler_c.profiler_struct.STALLS_LS_START_FIELD];
         r0 = M[r1 + $profiler_c.profiler_struct.STALLS_MS_START_FIELD];
         r2 = r2 - r4; // Instruction moved here to avoid resetting of flags from r0 = M[...]
         r0 = r3 - r0 - Borrow;
         r4 = M[$interrupt.total_stalls_ls];
         r3 = M[$interrupt.total_stalls_ms];
         r2 = r2 - r4; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         r0 = r0 - r3 - Borrow;
         r4 = M[r1 + $profiler_c.profiler_struct.INT_START_STALLS_LS_FIELD];
         r3 = M[r1 + $profiler_c.profiler_struct.INT_START_STALLS_MS_FIELD];
         r2 = r2 + r4; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         r0 = r0 + r3 + Carry;
         r2 = r2 - (1);        // extra BC5 stalls between NUM_STALLS_LS reads in profiler.start and profiler.stop
         r0 = r0 - Borrow;
         r3 = M[r1 + $profiler_c.profiler_struct.STALLS_LS_TOTAL_FIELD];
         r4 = M[r1 + $profiler_c.profiler_struct.STALLS_MS_TOTAL_FIELD];
         r3 = r3 + r2; // Instruction moved here to avoid resetting of flags from r3 = M[...]
         M[r1 + $profiler_c.profiler_struct.STALLS_LS_TOTAL_FIELD] = r3;
         r3 = r4 + r0 + Carry;
         M[r1 + $profiler_c.profiler_struct.STALLS_MS_TOTAL_FIELD] = r3;
         r3 = M[r1 + $profiler_c.profiler_struct.STALLS_MS_MAX_FIELD];
         Null = r3 - r0;
         if GT jump not_s_max;
            if LT jump s_max;
               r3 = M[r1 + $profiler_c.profiler_struct.STALLS_LS_MAX_FIELD];
               Null = r3 - r2;
               if GE jump not_s_max;
            s_max:
               M[r1 + $profiler_c.profiler_struct.STALLS_MS_MAX_FIELD] = r0;
               M[r1 + $profiler_c.profiler_struct.STALLS_LS_MAX_FIELD] = r2;
         not_s_max:

         r0 = M[$profiler.temp_clks_ms];
         r3 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MS_MAX_FIELD];
         Null = r3 - r0;
         if GT jump not_rc_max;
            if LT jump rc_max;
               r2 = M[$profiler.temp_clks_ls];
               r3 = M[r1 + $profiler_c.profiler_struct.RUN_CLKS_LS_MAX_FIELD];
               Null = r3 - r2;
               if GE jump not_rc_max;
            rc_max:
               M[r1 + $profiler_c.profiler_struct.RUN_CLKS_MS_MAX_FIELD] = r0;
               M[r1 + $profiler_c.profiler_struct.RUN_CLKS_LS_MAX_FIELD] = r2;
         not_rc_max:

         r0 = M[$profiler.temp_instrs_ms];
         r3 = M[r1 + $profiler_c.profiler_struct.INSTRS_MS_MAX_FIELD];
         Null = r3 - r0;
         if GT jump not_i_max;
            if LT jump i_max;
               r2 = M[$profiler.temp_instrs_ls];
               r3 = M[r1 + $profiler_c.profiler_struct.INSTRS_LS_MAX_FIELD];
               Null = r3 - r2;
               if GE jump not_i_max;
            i_max:
               M[r1 + $profiler_c.profiler_struct.INSTRS_MS_MAX_FIELD] = r0;
               M[r1 + $profiler_c.profiler_struct.INSTRS_LS_MAX_FIELD] = r2;
         not_i_max:
         pop r4;
      #endif
      pop r3;
   #endif
   #endif

   done:
   call $unblock_interrupts;
   jump $pop_rLink_and_rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $profiler.timer_service_routine
//
// DESCRIPTION:
//    Timer handler for profiler.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r4, r5, r10, DoLoop
//
// NOTES:
//    This computes the cpu usage of each routine being profiled.  This
// information of cpu usage can then be read from the chip using the Matlab
// tool 'kalprofiler'
//
// *****************************************************************************
.MODULE $M.profiler.timer_service_routine;
   .CODESEGMENT PROFILER_TIMER_SERVICE_ROUTINE_PM;

   $_profiler_timer_service_routine:
   $profiler.timer_service_routine:
   NULL = M[$profiler.enabled];
   if Z rts;
   // push rLink onto stack
   push rLink;

   #ifdef INTERRUPT_PROFILER_ON
      // Calc CPU fraction of interrupt servicing
      r0 = M[$interrupt.total_time];
      r1 = M[$interrupt.period_start_time];

      // Interrupt CPU fraction = (TotalTime - PeriodStartTime) >> 10
      r1 = r0 - r1;
      r1 = r1 ASHIFT - 10;
      M[$interrupt.cpu_fraction] = r1;

      // set PeriodStartTime = TotalTime
      M[$interrupt.period_start_time] = r0;

      r10 = $interrupt.MAX_NESTED_NR;
      do calculate_cpu_fraction;
         r2 = r10 - 1;
         Words2Addr(r2);
         r0 = M[$interrupt.total_time_with_task + r2];
         r1 = M[$interrupt.period_start_time_with_task + r2];

         // set PeriodStartTime = TotalTime
         M[$interrupt.period_start_time_with_task + r2] = r0;

         // Interrupt CPU fraction = (TotalTime - PeriodStartTime) >> 10
         // This timer service routine is called every 1.024 secounds which means
         // that the cpu fraction is the total run time/2^10 which is equal with
         // total_run_time >> 10.
         r1 = r0 - r1;
         r1 = r1 ASHIFT - 10;
         M[$interrupt.cpu_fraction_with_task + r2] = r1;
      calculate_cpu_fraction:
   #endif

   // Go through and calc CPU fraction of all routines being monitored
   r10 = $profiler.MAX_PROFILER_HANDLERS;
   r0 = M[$profiler.last_addr];
   do loop;
      // if we're at the last structure in the linked list then finish
      Null = r0 - $profiler.LAST_ENTRY;
      if Z jump done;

      // CPU fraction = TotalTime >> 10
      // 1000 equates to 100% CPU
      r1 = M[r0 + $profiler_c.profiler_struct.TOTAL_TIME_FIELD];
      r1 = r1 ASHIFT - 10;
      M[r0 + $profiler_c.profiler_struct.CPU_FRACTION_FIELD] = r1;
      // Check if the current cpu fraction is the maximum
      r2 = M[r0 + $profiler_c.profiler_struct.PEAK_CPU_FRACTION_FIELD];
      r1 = MAX r2;
      M[r0 + $profiler_c.profiler_struct.PEAK_CPU_FRACTION_FIELD] = r1;
      
      r1 = M[r0 + $profiler_c.profiler_struct.TEMP_COUNT_FIELD];
      M[r0 + $profiler_c.profiler_struct.COUNT_FIELD] = r1;
      M[r0 + $profiler_c.profiler_struct.TEMP_COUNT_FIELD] = Null;
      
      r1 = M[r0 + $profiler_c.profiler_struct.KICK_INC_FIELD];
      M[r0 + $profiler_c.profiler_struct.KICK_TOTAL_FIELD] = r1;
      M[r0 + $profiler_c.profiler_struct.KICK_INC_FIELD] = Null;

      #ifdef DETAILED_PROFILER_ON
         #ifdef K32
            // Average k clocks per second = Total >> 10
            // 5 equates to ~5000 clocks per second
            r1 = M[r0 + $profiler_c.profiler_struct.RUN_CLKS_TOTAL_FIELD];
            r1 = r1 LSHIFT -10;
            M[r0 + $profiler_c.profiler_struct.RUN_CLKS_AVERAGE_FIELD] = r1;
            M[r0 + $profiler_c.profiler_struct.RUN_CLKS_TOTAL_FIELD] = Null;
            // Average k instructions per second = Total >> 10
            // 5 equates to ~5000 instructions per second
            r1 = M[r0 + $profiler_c.profiler_struct.INSTRS_TOTAL_FIELD];
            r1 = r1 LSHIFT -10;
            M[r0 + $profiler_c.profiler_struct.INSTRS_AVERAGE_FIELD] = r1;
            M[r0 + $profiler_c.profiler_struct.INSTRS_TOTAL_FIELD] = Null;
            // Average k core stalls per second = Total >> 10
            // 5 equates to ~5000 core stalls per second
            r1 = M[r0 + $profiler_c.profiler_struct.CORE_STALLS_TOTAL_FIELD];
            r1 = r1 LSHIFT -10;
            M[r0 + $profiler_c.profiler_struct.CORE_STALLS_AVERAGE_FIELD] = r1;
            M[r0 + $profiler_c.profiler_struct.CORE_STALLS_TOTAL_FIELD] = Null;
            // Average k prefetch wait per second = Total >> 10
            // 5 equates to ~5000 prefetch wait per second
            r1 = M[r0 + $profiler_c.profiler_struct.PREFETCH_WAIT_TOTAL_FIELD];
            r1 = r1 LSHIFT -10;
            M[r0 + $profiler_c.profiler_struct.PREFETCH_WAIT_AVERAGE_FIELD] = r1;
            M[r0 + $profiler_c.profiler_struct.PREFETCH_WAIT_TOTAL_FIELD] = Null;
            #ifdef CHIP_BASE_CRESCENDO
               // Average k memory access stalls per second = Total >> 10
               // 5 equates to ~5000 memory access stalls per second
               r1 = M[r0 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_TOTAL_FIELD];
               r1 = r1 LSHIFT -10;
               M[r0 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_AVERAGE_FIELD] = r1;
               M[r0 + $profiler_c.profiler_struct.MEM_ACCESS_STALLS_TOTAL_FIELD] = Null;
               // Average k instruction expand stalls per second = Total >> 10
               // 5 equates to ~5000 instruction expand stalls per second
               r1 = M[r0 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_TOTAL_FIELD];
               r1 = r1 LSHIFT -10;
               M[r0 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_AVERAGE_FIELD] = r1;
               M[r0 + $profiler_c.profiler_struct.INSTR_EXPAND_STALLS_TOTAL_FIELD] = Null;
            #endif /* CHIP_BASE_CRESCENDO */
         #else
            // Average k clocks per second = Total >> 10
            // 5 equates to ~5000 clocks per second
            r1 = M[r0 + $profiler_c.profiler_struct.RUN_CLKS_LS_TOTAL_FIELD];
            r1 = r1 LSHIFT - 10;
            r2 = M[r0 + $profiler_c.profiler_struct.RUN_CLKS_MS_TOTAL_FIELD];
            r2= r2 LSHIFT 14;
            r1 = r1 + r2;
            M[r0 + $profiler_c.profiler_struct.RUN_CLKS_AVERAGE_FIELD] = r1;
            // set Totals = 0
            M[r0 + $profiler_c.profiler_struct.RUN_CLKS_LS_TOTAL_FIELD] = Null;
            M[r0 + $profiler_c.profiler_struct.RUN_CLKS_MS_TOTAL_FIELD] = Null;

            // Average k instructions per second = Total >> 10
            // 5 equates to ~5000 instructions per second
            r1 = M[r0 + $profiler_c.profiler_struct.INSTRS_LS_TOTAL_FIELD];
            r1 = r1 LSHIFT - 10;
            r2 = M[r0 + $profiler_c.profiler_struct.INSTRS_MS_TOTAL_FIELD];
            r2= r2 LSHIFT 14;
            r1 = r1 + r2;
            M[r0 + $profiler_c.profiler_struct.INSTRS_AVERAGE_FIELD] = r1;
            // set Totals = 0
            M[r0 + $profiler_c.profiler_struct.INSTRS_LS_TOTAL_FIELD] = Null;
            M[r0 + $profiler_c.profiler_struct.INSTRS_MS_TOTAL_FIELD] = Null;

            // Average k stalls per second = Total >> 10
            // 5 equates to ~5000 stalls per second
            r1 = M[r0 + $profiler_c.profiler_struct.STALLS_LS_TOTAL_FIELD];
            r1 = r1 LSHIFT - 10;
            r2 = M[r0 + $profiler_c.profiler_struct.STALLS_MS_TOTAL_FIELD];
            r2= r2 LSHIFT 14;
            r1 = r1 + r2;
            M[r0 + $profiler_c.profiler_struct.STALLS_AVERAGE_FIELD] = r1;
            // set Totals = 0
            M[r0 + $profiler_c.profiler_struct.STALLS_LS_TOTAL_FIELD] = Null;
            M[r0 + $profiler_c.profiler_struct.STALLS_MS_TOTAL_FIELD] = Null;
         #endif
      #endif

      // set TotalTime = 0
      M[r0 + $profiler_c.profiler_struct.TOTAL_TIME_FIELD] = Null;

      // read the adddress of the next profiler
      r0 = M[r0 + $profiler_c.profiler_struct.NEXT_FIELD];
   loop:

   // something's gone wrong - either too many handlers,
   // or more likely the linked list has got corrupt.
   // Emit fault and carry on.
   r1 = rLink;
   r0 = $faultids.FAULT_AUDIO_PROFILER_HANDLER_LIST_TOO_LONG;
   call $_fault_diatribe;

   done:

   // kick the timer off again to service profiler in 1.024secs 1000~ 1sec
#ifdef K32
   r0 = 1024000;
   r1 = &$profiler.timer_service_routine;
   r2 = Null;
#else
   r0 = 0;
   r1 = 1024000;
   r2 = &$profiler.timer_service_routine;
   r3 = Null;
#endif

   call $_asm_timer_schedule_event_in;
   // Store the returned timer id if we need to cancel later
   M[$profiler.timer_id] = r0;
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $profiler.enable
//
// DESCRIPTION:
//    Enable profiler and reinitialise if necessary
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r4, r5, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.profiler.enable;
   .CODESEGMENT PROFILER_START_PM;
   $_profiler_enable:
   $profiler.enable:
   // Check if the profiler has been already enabled
   NULL = M[$profiler.enabled];
   if NZ rts;
   // push rLink onto stack
   push rLink;
   r1 = 1;
   M[$profiler.enabled] = r1;
   call $profiler.initialise;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $profiler.disable
//
// DESCRIPTION:
//    Disable profiler
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r1
//
// *****************************************************************************
.MODULE $M.profiler.disable;
   .CODESEGMENT PROFILER_START_PM;
   $_profiler_disable:
   $profiler.disable:

   r1 = 0;
   M[$profiler.enabled] = r1;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $profiler.set_cpu_speed
//
// DESCRIPTION:
//    Set nominal CPU speed.
//    This is not used by on-chip code. It's here for off-chip tools to
//    read in order to convert %age figures into MIPS, in case on-chip code
//    has more information about the CPU speed.
//
// INPUTS:
//    - r0: CPU speed in MHz, or 0 for unknown
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
.MODULE $M.profiler.set_cpu_speed;
   .CODESEGMENT PROFILER_START_PM;
   $_profiler_set_cpu_speed:
   $profiler.set_cpu_speed:

   M[$profiler.cpu_speed_mhz] = r0;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $get_profiler_state
//
// DESCRIPTION:
//    Get the profiler state
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - r0
//
// TRASHED REGISTERS:
//
// *****************************************************************************
.MODULE $M.profiler.state;
   .CODESEGMENT PROFILER_START_PM;
   $_get_profiler_state:
   $profiler.state:

   r0 = M[$profiler.enabled];
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $deregister_all
//
// DESCRIPTION:
//    Clears the profiler list completely when the profiler is disabled.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2
//
// *****************************************************************************
.MODULE $M.profiler.deregister_all;
   .CODESEGMENT PROFILER_START_PM;
   $_profiler_deregister_all:
   $profiler.deregister_all:

   // Check if list is already empty
   r1 = $_profiler_list;
   if Z jump exit;
continue_loop:
   // Get next node
   r0 = M[r1 + $profiler_c.profiler_struct.NEXT_FIELD];
   // End of the list?
   Null = r1;
   if Z jump update_list;
   Null = r0 - $profiler.UNINITIALISED;
   if Z jump update_list;
   // Update previous node
   r2 = $profiler.UNINITIALISED;
   M[r1 + $profiler_c.profiler_struct.NEXT_FIELD] = r2;
   // Put next node in r1 to be uninitialised.
   r1 = r0;
   jump continue_loop;
update_list:
   r0 = $_profiler_list;
   M[r0] = NULL;

exit:
   rts;

.ENDMODULE;


#endif /* PROFILER_ON */
#endif
