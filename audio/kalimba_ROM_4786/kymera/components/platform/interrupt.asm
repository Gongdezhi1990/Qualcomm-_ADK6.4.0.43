// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Interrupt Library
//
// DESCRIPTION:
//    This library has functions to setup the interrupt controller, block and
//    unblock interrupts, and also a standard interrupt service routine (ISR).
//    The ISR currently handles just MCU and timer1 interrupt sources, but may
//    be extended if required.
//
//    Upon an interrupt, the Kalimba DSP jumps to location in INT_ADDR
//    (fix 0x0002 for BC7 (gordon)),
//    copies the user mode flags values to the interrupt mode bits (MSbyte).
//
//    Priority levels
//    Hydra Kalimba supports three interrupt priority levels.
//    BC7 Kalimba supports four interrupt priority levels. Level zero does
//    not interrupt the Kalimba but will wake it up from a sleep instruction.
//
//    An interrupt source with a higher interrupt priority may interrupt the
//    ISR if the UM flag is set.
//
//    Interrupt sources
//    Hydra Kalimba DSP has up to 24 interrupt sources and three priority levels.
//    See register documentation for full details.
//    To enable an interrupt source, enable its bitfield in one of
//    $INT_[LOW|MED|HIGH]_PRI_SOURCES_EN and write 1 to $INT_GBL_ENABLE and
//    $INT_UNBLOCK to enable Kalimba DSP interrupts.
//
//    BC7 Kalimba DSP has eight interrupt sources: four software based events;
//    two timer events; a PIO event and an MCU event.  To enable an interrupt
//    source, enable its bitfield in $INT_SOURCES_EN, set its priority to
//    greater than 0 in $INT_PRIORITIES and write 1 to $INT_GBL_ENABLE and
//    $INT_UNBLOCK to enable Kalimba DSP interrupts
//
//    $INT_GBL_ENABLE is used to reset the interrupt handler;
//    no interrupts are recorded or allowed when this is zero.
//    $INT_UNBLOCK is used to prevent interrupts firing,
//    when $INT_UNBLOCK is high, any interrupts will then trigger.
//
//    To block and unblock interrupts, call the $block_interrupts and
//    $unblock_interrupts subroutines; for interrupts to be enabled, the
//    $unblock_interrupts must be called as many times as $block_interrupts
//    has been called.  This allows for nested subroutines to block and
//    unblock interrupts in a consistent manner.
//
// *****************************************************************************

#include "interrupt.h"
#include "profiler.h"
#include "stack.h"
#include "architecture.h"
#include "portability_macros.h"

.MODULE $interrupt;
   .DATASEGMENT DM;

    // Store as a constant the size of the stack required to store the
    // processor's state.  Also create a constant to hold the nested
    // interrupt state
   .CONST   STORE_CHECK_SIZE       $INTERRUPT_STORE_CHECK_SIZE;
   .CONST   RESTORE_CHECK_SIZE     $INTERRUPT_RESTORE_CHECK_SIZE;

#if !defined(DISABLE_SHALLOW_SLEEP) && !CHIP_HAS_SHALLOW_SLEEP_REGISTERS
   .VAR shallow_sleep_danger = 0;
   .VAR nest_count = 0;
#elif defined(INTERRUPT_PROFILER_ON)
   .VAR nest_count = 0;
#endif

   // counter to support nested blocking/unblocking of interrupts
   .VAR block_count = 0;

   #ifdef INTERRUPT_PROFILER_ON
      // Only one interrupt priority is used (except the SW exception) but interrupts
      // can be nested because when the interrupt handler finishes the scheduler context
      // switch function is called. In this manner an interrupt is made from an interrupt
      // handler and the call to the scheduler (which can run a task).Because of this
      // reason the max number of nested interrupt is the maximum number of task priority.
      .CONST MAX_NESTED_NR 5;
      .VAR/DM1 start_time[MAX_NESTED_NR]= 0 ...;              // DM1 to avoid possible stalls
      .VAR total_time = 0;
      .VAR total_time_with_task[MAX_NESTED_NR] = 0 ...;
      #ifdef DETAILED_PROFILER_ON
         #ifdef K32
            .VAR/DM1 start_clocks;                // DM1 to avoid possible stalls
            .VAR/DM1 total_clocks = 0;            // DM1 to avoid possible stalls
            .VAR     start_core_stalls;
            .VAR/DM1 total_core_stalls = 0;       // DM1 to avoid possible stalls
            .VAR     start_instrs;
            .VAR/DM1 total_instrs = 0;            // DM1 to avoid possible stalls
            .VAR     start_prefetch_wait;
            .VAR/DM1 total_prefetch_wait = 0;     // DM1 to avoid possible stalls
            #ifdef CHIP_BASE_CRESCENDO
               .VAR     start_access_stalls;
               .VAR/DM1 total_access_stalls = 0;  // DM1 to avoid possible stalls
               .VAR     start_instr_expand_stalls;
               .VAR/DM1 total_instr_expand_stalls = 0;  // DM1 to avoid possible stalls
            #endif
         #else
            .VAR     start_clocks_ls;
            .VAR/DM1 start_clocks_ms;      // DM1 to avoid possible stalls
            .VAR     total_clocks_ls = 0;
            .VAR/DM1 total_clocks_ms = 0;  // DM1 to avoid possible stalls
            .VAR     start_stalls_ls;
            .VAR     start_stalls_ms;
            .VAR     total_stalls_ls = 0;
            .VAR/DM1 total_stalls_ms = 0;  // DM1 to avoid possible stalls
            .VAR     start_instrs_ls;
            .VAR     start_instrs_ms;
            .VAR     total_instrs_ls = 0;
            .VAR/DM1 total_instrs_ms = 0;  // DM1 to avoid possible stalls
         #endif
      #endif
      .VAR period_start_time;
      .VAR period_start_time_with_task[MAX_NESTED_NR] = 0 ...;
      .VAR/DM1 cpu_fraction;
      .VAR/DM1 cpu_fraction_with_task[MAX_NESTED_NR] = 0 ...;
   #endif


#ifdef CHIP_BASE_HYDRA
    // Table of interrupt handlers
    // Don't add new handlers here unless they really are needed in every build.
    // Normally they should be installed at run time.
    .VAR isr_call_table[NUMBER_INTERRUPT_SOURCES] =
          $_timer_service_routine,           // SOURCE_TIMER1
          $_casual_kick_event,               // SOURCE_TIMER2
          $error_sw_exception,               // SOURCE_SW_ERROR
          $error_unregistered,               // SOURCE_SW0
          $error_unregistered,               // SOURCE_SW1
          $error_unregistered,               // SOURCE_SW2
          $error_unregistered,               // SOURCE_SW3
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_1
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_2
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_3
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_4
          $error_unregistered,               // SOURCE_TBUS_MSG_ADPTR_EVENT
          $error_unregistered,               // SOURCE_AUDIO_EVENT
          $error_unregistered,               // SOURCE_FM_EVENT
          $error_unregistered,               // SOURCE_OUTBOUND_ACCESS_ERROR_EVENT
          $error_unregistered,               // SOURCE_TIME_UPDATE_EVENT
          $error_unregistered,               // SOURCE_RD_STREAM_MON_EVENT
          $error_unregistered,               // SOURCE_WR_STREAM_MON_EVENT
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
// We only support 24 sources or 32 sources. So this will work until we have different
// number of sources to support.
#if NUMBER_INTERRUPT_SOURCES > 24
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
#endif
          $error_unregistered;               // SOURCE_UNMAPPED

#elif defined(CHIP_BASE_BC7)

   #if defined(CHIP_RICK)
      .VAR isr_call_table[11] =
            $_timer_service_routine,           // SOURCE_TIMER1
            $_casual_kick_event,               // SOURCE_TIMER2
            $error_sw_exception,               // SOURCE_SW_ERROR
            $error,                            // SOURCE_SW0
            $error,                            // SOURCE_SW1
            $error,                            // SOURCE_SW2
            $error,                            // SOURCE_SW3
            $message.received_service_routine, // SOURCE_MCU_TO_DSP
            $error,                            // SOURCE_PIO
            $error,                            // SOURCE_UNUSED
            $error;                            // SOURCE_MMU_UNMAPPED

   #elif defined(CHIP_GORDON)

      .VAR isr_call_table[9] =
              $_timer_service_routine,           // SOURCE_TIMER1
              $_casual_kick_event,               // SOURCE_TIMER2
              $message.received_service_routine, // SOURCE_MCU
              $error,                            // SOURCE_PIO
              $error,                            // SOURCE_MMU_UNMAPPED
              $error,                            // SOURCE_SW0
              $error,                            // SOURCE_SW1
              $error,                            // SOURCE_SW2
              $error;                            // SOURCE_SW3

   // alias for BC7
   .CONST $INT_LOAD_INFO_DONT_CLEAR_MASK       $INT_LOAD_INFO_CLR_REQ_MASK;

   #else
   #error CHIP_BASE_BC7 is not supported
   #endif /*CHIP_GORDON */

#elif defined(CHIP_BASE_A7DA_KAS)
     .VAR isr_call_table[32] =
          $_timer_service_routine,                  // INT_SOURCE_LOW_PRI_TIMER1
          $_casual_kick_event,                      // INT_SOURCE_LOW_PRI_TIMER2
          $error,                                   // INT_SOURCE_LOW_PRI_SW_ERROR
          $error,                                   // INT_SOURCE_LOW_PRI_SW0
          $error,                                   // INT_SOURCE_LOW_PRI_SW1
          $error,                                   // INT_SOURCE_LOW_PRI_SW2
          $error,                                   // INT_SOURCE_LOW_PRI_SW3
          $error,                                   // DUMMY PADDING
          $error,                                   // not connected
          $error_apb_slv_error,                     // Kalimba keyhole error: APB pslverr.
          $error,                                   // DMAC_INTR
          $error,                                   // step a: not connected, step b: USP3
          $error,                                   // step a: not connected, step b: UART6
          $error,                                   // step a: not connected, step b: DMAC4
          $error,                                   // AC97
          $error,                                   // DAMC2
          $error,                                   // DMAC3
          $error,                                   // GPIO controller
          $error,                                   // USP0
          $error,                                   // USP1
          $error,                                   // USP2
          $error,                                   // Internal Audio Codec
          $error,                                   // I2S1
          $error,                                   // SPDIF Transmitter
          $message.core0_int1_service_routine,      // IPC interrupt 1 from CA7 core 0
          $message.core1_int1_service_routine,      // IPC interrupt 1 from CA7 core 1
          $message_m3.int1_service_routine,         // IPC interrupt 1 from M3
          $message.core0_int2_service_routine,      // IPC interrupt 2 from CA7 core 0
          $message.core1_int2_service_routine,      // IPC interrupt 2 from CA7 core 1
          $message_m3.int2_service_routine,         // IPC interrupt 2 from M3
          $error,                                   // step a: not connected, step b: DMAC0
          $error;                                   // step a: not connected, step b: SDIO67

#elif defined(CHIP_NAPIER)
    // Table of interrupt handlers
    // Don't add new handlers here unless they really are needed in every build.
    // Normally they should be installed at run time.
    .VAR isr_call_table[NUMBER_INTERRUPT_SOURCES] =
          $_timer_service_routine,           // SOURCE_TIMER1
          $_casual_kick_event,               // SOURCE_TIMER2
          $error_sw_exception,               // SOURCE_SW_ERROR
          $error_unregistered,               // SOURCE_SW0
          $error_unregistered,               // SOURCE_SW1
          $error_unregistered,               // SOURCE_SW2
          $error_unregistered,               // SOURCE_SW3
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_1
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_2
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_3
          $error_unregistered,               // SOURCE_TBUS_INT_ADPTR_EVENT_4
          $error_unregistered,               // SOURCE_TBUS_MSG_ADPTR_EVENT
          $error_unregistered,               // SOURCE_AUDIO_EVENT
          $error_unregistered,               // SOURCE_FM_EVENT
          $error_unregistered,               // SOURCE_OUTBOUND_ACCESS_ERROR_EVENT
          $error_unregistered,               // SOURCE_TIME_UPDATE_EVENT
          $error_unregistered,               // SOURCE_RD_STREAM_MON_EVENT
          $error_unregistered,               // SOURCE_WR_STREAM_MON_EVENT
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
// We only support 24 sources or 32 sources. So this will work until we have different
// number of sources to support.
#if NUMBER_INTERRUPT_SOURCES > 24
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
          $error_unregistered,               // SOURCE_UNMAPPED
#endif
          $error_unregistered;               // SOURCE_UNMAPPED

   #ifdef OPTIONAL_FAST_INTERRUPT_SUPPORT
   .VAR isr_fast_call_table[NUMBER_INTERRUPT_SOURCES] =
          0,                                 // SOURCE_TIMER1
          0,                                 // SOURCE_TIMER2
          0,                                 // SOURCE_SW_ERROR
          0,                                 // SOURCE_SW0
          0,                                 // SOURCE_SW1
          0,                                 // SOURCE_SW2
          0,                                 // SOURCE_SW3
          0,                                 // SOURCE_TBUS_INT_ADPTR_EVENT_1
          0,                                 // SOURCE_TBUS_INT_ADPTR_EVENT_2
          0,                                 // SOURCE_TBUS_INT_ADPTR_EVENT_3
          0,                                 // SOURCE_TBUS_INT_ADPTR_EVENT_4
          0,                                 // SOURCE_TBUS_MSG_ADPTR_EVENT
          0,                                 // SOURCE_AUDIO_EVENT
          0,                                 // SOURCE_FM_EVENT
          0,                                 // SOURCE_OUTBOUND_ACCESS_ERROR_EVENT
          0,                                 // SOURCE_TIME_UPDATE_EVENT
          0,                                 // SOURCE_RD_STREAM_MON_EVENT
          0,                                 // SOURCE_WR_STREAM_MON_EVENT
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
// We only support 24 sources or 32 sources. So this will work until we have different
// number of sources to support.
#if NUMBER_INTERRUPT_SOURCES > 24
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
          0,                                 // SOURCE_UNMAPPED
#endif
          0;                                 // SOURCE_UNMAPPED
   #endif // OPTIONAL_FAST_INTERRUPT_SUPPORT
#else
#error Chip is not supported
#endif /* CHIP_BASE_HYDRA */

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $interrupt.initialise
//
// DESCRIPTION:
//    Initialise the interrupt controller
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1
//
// *****************************************************************************
.MODULE $M.interrupt.initialise;
   .CODESEGMENT INTERRUPT_INITIALISE_PM;

   $_interrupt_initialise:
   $interrupt.initialise:

   // -- setup interrupt controller --

#ifdef AMBER_STYLE_INT_CTRL

   // enable interrupts for: timer1. SW error
#ifdef CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
#define INTERRUPT_PRIORITY_HIGH 2
#define INTERRUPT_PRIORITY_LOW 0
   r0 = $INT_SOURCE_TIMER1_ENUM;    // interrupt to enable
   M[$INT_SELECT] = r0;             // select the bank
   r1 = INTERRUPT_PRIORITY_LOW;
   M[$INT_PRIORITY] = r1;           // set priority
   r0 = 1 LSHIFT r0;
   M[$INT_SOURCES_EN] = r0;         // enable the interrupt

   r0 = $INT_SOURCE_TIMER2_ENUM;    // interrupt to enable
   M[$INT_SELECT] = r0;             // select the bank
   r1 = INTERRUPT_PRIORITY_LOW;
   M[$INT_PRIORITY] = r1;           // set priority
   r0 = 1 LSHIFT r0;
   r1 = M[$INT_SOURCES_EN];
   r1 = r1 OR r0;
   M[$INT_SOURCES_EN] = r1;         // enable the interrupt

   r0 = $INT_SOURCE_SW_ERROR_ENUM;
   M[$INT_SELECT] = r0;
   r1 = INTERRUPT_PRIORITY_HIGH;
   M[$INT_PRIORITY] = r1;           // set priority
   r1 = M[$INT_SOURCES_EN];
   r0 = 1 LSHIFT r0;
   r1 = r1 OR r0;
   M[$INT_SOURCES_EN] = r1;         // enable the interrupt

#undef INTERRUPT_PRIORITY_HIGH
#undef INTERRUPT_PRIORITY_LOW
#else // CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
      r0 = ($INT_LOW_PRI_SOURCES_EN_TIMER1_MASK
          + $INT_LOW_PRI_SOURCES_EN_TIMER2_MASK
#ifdef CHIP_RICK
   // TODO: Functionality-specific int enable should be moved eventually into the
   // respective modules, e.g. MCU->DSP msg ints can be enabled when messaging is
   // started up. Below the Gordon-esque INT sources are added.
   + $INT_LOW_PRI_SOURCES_EN_MCU_TO_DSP_EVENT_MASK +
   + $INT_LOW_PRI_SOURCES_EN_MMU_UNMAPPED_EVENT_MASK
#endif
   );

   M[$INT_LOW_PRI_SOURCES_EN] = r0;

   M[$INT_MED_PRI_SOURCES_EN] = 0;

   r0 = $INT_HIGH_PRI_SOURCES_EN_SW_ERROR_MASK;

   M[$INT_HIGH_PRI_SOURCES_EN] = r0;
#endif // CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS

   // Set the hardware to point to the interrupt vector
   r0 = $interrupt.handler;
   M[$_INT_ADDR] = r0;

#elif defined(GORDON_STYLE_INT_CTRL)

   // enable interrupts for: timer1, timer2, mcu/arm, and mmu_unmapped
   r0 = ($INT_SOURCE_TIMER1_MASK +
         $INT_SOURCE_TIMER2_MASK +
         $INT_SOURCE_MCU_MASK +
         $INT_SOURCE_MMU_UNMAPPED_MASK);

   M[$INT_SOURCES_EN] = r0;

   // set int priorities: timer1 = 1,  mcu/arm = 1,  unmapped = 3,
   r0 = ((1 << $INT_SOURCE_TIMER1_POSN*2) +
         (1 << $INT_SOURCE_TIMER2_POSN*2) +
         (1 << $INT_SOURCE_MCU_POSN*2) +
         (3 << $INT_SOURCE_MMU_UNMAPPED_POSN*2));

   M[$INT_PRIORITIES] = r0;
#elif defined(A7DA_KAS_STYLE_INT_CTRL)
  // -- setup interrupt controller --
  // enable interrupts for: timer1 and timer2
  r0 = ($INT_LOW_PRI_SOURCES_EN_TIMER1_MASK +
        $INT_LOW_PRI_SOURCES_EN_TIMER2_MASK);

  M[$INT_LOW_PRI_SOURCES_EN] = r0;


  // enable high priority interrupts for: SW error and keyhole
#ifndef NOCFW_DEBUG
  r0 = ($INT_HIGH_PRI_SOURCES_EN_SW_ERROR_MASK +
        $INT_HIGH_PRI_SOURCES_EN_SLV_ERROR_MASK);
#else
  r0 = ($INT_HIGH_PRI_SOURCES_EN_SW_ERROR_MASK);
#endif /* NOCFW_DEBUG */

  M[$INT_HIGH_PRI_SOURCES_EN] = r0;
  // enable IPC interrupts from core0&core1
  r0 = ($INT_LOW_PRI_SOURCES_EN_IPC_TRGT3_INIT0_INTR1_MASK +
         $INT_LOW_PRI_SOURCES_EN_IPC_TRGT3_INIT1_INTR1_MASK +
         $INT_LOW_PRI_SOURCES_EN_IPC_TRGT3_INIT2_INTR1_MASK +
         $INT_LOW_PRI_SOURCES_EN_IPC_TRGT3_INIT0_INTR2_MASK +
         $INT_LOW_PRI_SOURCES_EN_IPC_TRGT3_INIT1_INTR2_MASK +
         $INT_LOW_PRI_SOURCES_EN_IPC_TRGT3_INIT2_INTR2_MASK);

  M[$INT_LOW_PRI_SOURCES_EXTRA_EN] = r0;

  // Set INT_ADDR to point to the interrupt handler
  r0 = &$interrupt.handler;
  M[$INT_ADDR] = r0;

#else
#error interrupt.initialise is not supported for this Chip
#endif   // AMBER_STYLE_INT_CTRL


   // enable interrupts
   r0 = 1;
   M[$INT_GBL_ENABLE] = r0;
   M[$INT_UNBLOCK] = r0;

   // enable switching to the maximum clock frequency during an interrupt
   M[$INT_CLK_SWITCH_EN] = r0;
   r1 = $CLK_DIV_1;
   M[$INT_CLOCK_DIVIDE_RATE] = r1;

   // switch to user mode
   rFlags = rFlags OR $UM_FLAG;
   rts;

   // force the reset and interrupt handler functions to be pulled in by kalasm2
   // don't remove these lines!!
   jump $reset;
   jump $interrupt.handler;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $block_interrupts
//
// DESCRIPTION:
//    Block interrupts subroutine, NOTE this may also be called using $interrupt.block
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
// *****************************************************************************
.MODULE $M.interrupt.block;
   .CODESEGMENT INTERRUPT_BLOCK_PM;

   $_interrupt_block:
   $block_interrupts:
   $interrupt.block:

   // Register write to block all IRQs, and wait for this to take effect
   M[$INT_UNBLOCK] = Null;
   nop;
   nop;
   nop;

   //May have had an IRQ while we were waiting for block to take effect
   //If so, the IRQ will have re-enabled interrupts.
   //Check for this, and if so try again....
   Null = M[$INT_UNBLOCK];
   if NZ jump $block_interrupts;

   //Now IRQs are disabled, update the block_count
   r0 = M[$interrupt.block_count];
   r0 = r0 + 1;
   M[$interrupt.block_count] = r0;

   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $unblock_interrupts
//
// DESCRIPTION:
//    Unblock interrupts subroutine, NOTE this may also be called using $interrupt.unblock
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
// *****************************************************************************
.MODULE $M.interrupt.unblock;
   .CODESEGMENT INTERRUPT_UNBLOCK_PM;

   $_interrupt_unblock:
   $unblock_interrupts:
   $interrupt.unblock:

   r0 = M[$interrupt.block_count];
   if Z call $error;

   // decrement the count
   r0 = r0 - 1;
   M[$interrupt.block_count] = r0;
   // only unblock interrupts if at outer part of nesting
   if NZ rts;
   // unblock interrupts
   r0 = 1;
   M[$INT_UNBLOCK] = r0;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $interrupt.register
//
// DESCRIPTION:
//    Registers an interrupt handler for a particular interrupt source, and
//    sets the priority as required.
//
// INPUTS:
//    - r0 - Interrupt Source (eg. $INT_SOURCE_PIO_EVENT)
//    - r1 - Interrupt Priority (1 lowest -> 3 highest, 0 wakeup)
//    - r2 - Function address to call upon interrupt
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r3
//
// *****************************************************************************
.MODULE $M.interrupt.register;
   .CODESEGMENT INTERRUPT_REGISTER_PM;

   $_interrupt_register:
   $interrupt.register:

#ifdef CHIP_BASE_HYDRA
   // check if source is greater than NUMBER_INTERRUPT_SOURCES
   Null = r0 - NUMBER_INTERRUPT_SOURCES;
   if GE rts;
#endif

   // push rLink onto stack
   push rLink;
   push r5;

   // r0 is trashed by blocking interrupts, so set r3 = r0
   r3 = r0;

   // block interrupts first to be safe
   call $block_interrupts;

   // fill in isr_call_table as required
   // TODO: meh, multiply on every interrupt setup ?
   r0 = r3 * ADDR_PER_WORD (int);
   M[$interrupt.isr_call_table + r0] = r2;

   $interrupt.register.call_table_set:

   // enable interrupt source
#ifdef AMBER_STYLE_INT_CTRL
   // we need to decrease this even in CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
   // since we are using the same enum as before (see enum interrupt_priority in
   // components/hal/hal_interrupt.h
   r1 = r1 - 1;
#ifdef CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
   push r2;
   M[$INT_SELECT] = r3;
   M[$INT_PRIORITY] = r1;
   r2 = M[$INT_SOURCES_EN];
   r0 = 1 LSHIFT r3;
   r2 = r2 OR r0;
   M[$INT_SOURCES_EN] = r2;
   pop r2;
#else // CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
   r5 = M[$INT_LOW_PRI_SOURCES_EN + r1];
   r0 = 1 LSHIFT r3;
   r5 = r5 OR r0;
   M[$INT_LOW_PRI_SOURCES_EN + r1] = r5;
#endif // CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
#elif defined(GORDON_STYLE_INT_CTRL)

   r5 = M[$INT_SOURCES_EN];
   r0 = 1 LSHIFT r3;
   r5 = r5 OR r0;
   M[$INT_SOURCES_EN] = r5;

   // form mask for priority
   r3 = r3 * 2 (int);
   r0 = 3 LSHIFT r3;
   r1 = r1 LSHIFT r3;

   // set interrupt priority as requested
   r5 = M[$INT_PRIORITIES];
   r5 = r5 OR r0;
   r5 = r5 - r0;
   M[$INT_PRIORITIES] = r5 + r1;
#elif defined(A7DA_KAS_STYLE_INT_CTRL)

   r1 = r1 - 1;
   // A7da_kas has one register for extra interrupts per each priority
   // multiplying by 2 we avoid them.
   r1 = r1 LSHIFT 1;
   // Checks if need to use extra registers, if needed increment by 1.
   r0 = $KAS_FIRST_EXTRA_INTR_LINE-1;
   Null = r3 - r0;
   if LE jump set_registers;

configure_extra_intr:
   r1 = r1 +1;
   // subtract $KAS_FIRST_EXTRA_INTR_LINE to the interrupt before updating the register
   r3 = r3 - $KAS_FIRST_EXTRA_INTR_LINE;

set_registers:
   r5 = M[$INT_LOW_PRI_SOURCES_EN + r1];
   r0 = 1 LSHIFT r3;
   r5 = r5 OR r0;
   M[$INT_LOW_PRI_SOURCES_EN + r1] = r5;
#else

#error "interrupt.register not implemented for this chip"

#endif   // AMBER_STYLE_INT_CTRL

   // unblock interrupts and exit
   call $unblock_interrupts;

   pop r5;
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $interrupt.handler
//
// DESCRIPTION:
//    Interrupt handler that's compatible with the various library functions
//    CSR supply.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    none (all are saved and restored)
//
// NOTES:
//    The ISR code supports multiple priority (nested interrupts).
//    If nested interrupts are not required then all enabled interrupts
//    should use the same priority level.
//
// *****************************************************************************
.MODULE $M.interrupt.handler;
   .CODESEGMENT INTERRUPT_HANDLER_PM;
   .DATASEGMENT DM;

   .VAR $_SchedInterruptActive = 0;

// NOTE: be careful when changing code in here to allow for it in the calculations
//       in the DETAILED_PROFILER section at the bottom

   $interrupt.handler:

   #ifdef CHIP_DEF_KALIMBA_CLEARS_UM_LATE
      // On versions of the Kalimba from at least Rick up to (but not
      // including) Crescendo d01's, the first instruction of the interrupt
      // handler must not be one that writes to the rFlags register, as the
      // UM bit is cleared 1 instruction after the rest of the flags have a
      // copy of them taken. B-88464
      // Expected symptom of getting this from is rFlags wrongly restored
      // when the interrupt handler exits.
      // For simplicity, we make it a nop always. We could probably get away
      // with something more complicated if this extra instruction is a
      // problem.
      nop;
   #endif // CHIP_DEF_KALIMBA_CLEARS_UM_LATE

      // Detect stack overflow without trashing any registers or stack
      // (only rFlags, which was already saved by hardware).
      // (This is why we're not testing EXCEPTION_TYPE.)
      // NB: in Crescendo d00, this check never fires because the hardware
      // is busted (B-180602), so we fall through and later hit
      // PANIC_AUDIO_LIBRARY_ERROR (B-180661).
   #ifdef DEBUG_STACK_OVERFLOW
      // check the interrupt hasn't fired because of stack overflow
      // (WRT CHIP_DEF_KALIMBA_CLEARS_UM_LATE: this clearly modifies rFlags,
      // so looks dangerous as a first instruction. We seemed to get away
      // with it for a while, probably because a read from a memory-mapped
      // register causes a stall.)
      Null = M[$STACK_OVERFLOW_PC];
      if NZ call $error_stack_overflow;
   #endif // DEBUG_STACK_OVERFLOW

   // (WRT CHIP_DEF_KALIMBA_CLEARS_UM_LATE: pushm is a safe first
   // instruction on all chips before Crescendo d00 as it doesn't modify
   // rFlags, but on Kalimba_Next DSPs like Crescendo, pushm *does* modify
   // rFlags so is not safe.)
   pushm <FP(=SP), r0, r1, r2, rFlags>;

   r0 = M[$ARITHMETIC_MODE];
   push r0;
   M[$ARITHMETIC_MODE] = Null;

   #if (!defined(DISABLE_SHALLOW_SLEEP) && !CHIP_HAS_SHALLOW_SLEEP_REGISTERS) || defined(INTERRUPT_PROFILER_ON)
      r0 = M[$interrupt.nest_count];
      r0 = r0 + 1;
      M[$interrupt.nest_count] = r0;
   #endif

   #ifdef INTERRUPT_PROFILER_ON
      NULL = M[$profiler.enabled];
      if Z jump no_profiler;
      r0 = r0 - 1;
      r1 = $interrupt.MAX_NESTED_NR - 1;
      Null = r0 - r1;
      if GT r0 = r1;
      // store the time at which the ISR was called
      r1 = M[$TIMER_TIME];
      Words2Addr(r0);
      M[$interrupt.start_time + r0] = r1;
      #ifdef DETAILED_PROFILER_ON
         #ifdef K32
            r0 = M[$NUM_RUN_CLKS];
            r1 = M[$NUM_CORE_STALLS];
            r2 = M[$NUM_INSTRS];
            M[$interrupt.start_clocks] = r0;
            M[$interrupt.start_core_stalls] = r1;
            r0 = M[$PREFETCH_WAIT_OUT_COUNT];
            M[$interrupt.start_instrs] = r2;
            M[$interrupt.start_prefetch_wait] = r0;
            #ifdef CHIP_BASE_CRESCENDO
               r0 = M[$NUM_MEM_ACCESS_STALLS];
               r1 = M[$NUM_INSTR_EXPAND_STALLS];
               M[$interrupt.start_access_stalls] = r0;
               M[$interrupt.start_instr_expand_stalls] = r1;
            #endif
         #else
            r0 = M[$NUM_RUN_CLKS_MS];
            r2 = M[$NUM_RUN_CLKS_MS];
            r1 = M[$NUM_RUN_CLKS_LS];
            r1 = r1 - 1;
            r2 = r2 - Borrow;
            Null = r2 - r0;
            if LT r2 = r0;
            M[$interrupt.start_clocks_ls] = r1;
            M[$interrupt.start_clocks_ms] = r2;
            r1 = M[$NUM_CORE_STALLS_LS];
            r2 = M[$NUM_CORE_STALLS_MS];
            M[$interrupt.start_stalls_ls] = r1;
            M[$interrupt.start_stalls_ms] = r2;
            r0 = M[$NUM_INSTRS_MS];
            r2 = M[$NUM_INSTRS_MS];
            r1 = M[$NUM_INSTRS_LS];
            r1 = r1 - 1;
            r2 = r2 - Borrow;
            Null = r2 - r0;
            if LT r2 = r0;
            M[$interrupt.start_instrs_ls] = r1;
            M[$interrupt.start_instrs_ms] = r2;
         #endif
      #endif
no_profiler:
   #endif

#ifdef CHIP_BASE_BC7
   // save current non-interrupt clock rate
   r0 = M[$CLOCK_DIVIDE_RATE];
   push r0;
#endif

#if CHIP_HAS_SHALLOW_SLEEP_REGISTERS
   r0 = 0;
   M[$GOTO_SHALLOW_SLEEP] = r0;
#else // CHIP_HAS_SHALLOW_SLEEP_REGISTERS
   // set to fastest non-interrupt clock rate
   r0 = $CLK_DIV_1;
   M[$CLOCK_DIVIDE_RATE] = r0;
#endif // CHIP_HAS_SHALLOW_SLEEP_REGISTERS

   // disable any bit reverse addressing on AG1
   rFlags = rFlags AND $NOT_BR_FLAG;
   #ifdef DEBUG_STACK_OVERFLOW
      // check we have space on the stack for storing the processor's state
      r0 = M[$STACK_POINTER];
      r0 = r0 + $interrupt.STORE_CHECK_SIZE*ADDR_PER_WORD;
      Null = r0 - M[$STACK_END_ADDR];
      if GT call $error;
   #endif

   // block interrupts
   // This takes a few cycles to take effect, but we don't have to worry
   // about that as UM=0 (effectively blocking interrupts), and won't
   // become 1 for more than a few cycles.
   M[$INT_UNBLOCK] = Null;

   // save interrupt controller state, rIntLink, rFlags, and virtually all other registers!
   r1 = M[$INT_SAVE_INFO];
   r2 = M[$MM_RINTLINK];
   pushm <r2, r3, r4, r5, r6, r7, r8, r9, r10, rLink>;
   pushm <I0, I1, I2, I3, I4, I5, I6, I7, M0, M1, M2, M3, L0, L1, L4, L5>;
   pushm <rMAC2, rMAC1, rMAC0, DoLoopStart, DoLoopEnd, DivResult, DivRemainder, rMACB2, rMACB1, rMACB0, B0, B1, B4, B5>;

   // The old value of INT_SAVE_INFO needs to be written to INT_LOAD_INFO
   // to re-enable interrupts at this level before attempting a context switch.
   // So we put it on the stack last.
   push r1;

   // Acknowledge interrupt and clear it:
   // - This will allow another interrupt to occur if we were in user mode,
   //   but we're not, so it will allow them once the UM flag is set.
   // - For nested interrupt the INT_SOURCE register will now be valid up until we
   //   re-enable the INT_UNBLOCK register.
   M[$INT_ACK] = Null;

   // clear the length registers
   L0 = 0;
   L1 = 0;
   L4 = 0;
   L5 = 0;

   // clear the base registers
   push Null;
   B5 = M[SP - 1*ADDR_PER_WORD];
   B4 = M[SP - 1*ADDR_PER_WORD];
   B1 = M[SP - 1*ADDR_PER_WORD];
   pop B0;

   // Set the modify registers as the compiler expects
   // See the ABI in CS-124812-UG.
   M0 = 0;
   M1 = ADDR_PER_WORD;
   M2 = -ADDR_PER_WORD;

   // We're not clearing the DOLOOP registers here, so they'll
   // remain as they were in the interrupted code.
   // There shouldn't be any situation where this matters; code using
   // loops will set up the registers first, and there should be no
   // way to hit the loop end without going through such setup code.

   // Inform the scheduler we've had an IRQ
   r0 = M[$_SchedInterruptActive];
   r0 = r0 + 1;
   M[$_SchedInterruptActive] = r0;

   // see what the interrupt source was
   r2 = M[$INT_SOURCE];

#ifdef A7DA_KAS_STYLE_INT_CTRL
   // Interrupt source can be 0-95; 0-31 is low pri, 32-63 med pri
   // and 64-95 high pri. mod by 32 to find the isr call table indx
   r2 = r2 AND 0x1F;
   // r2 contains M[$INT_SOURCE]%32
   // Store int_source, it is needed for the kas_bouncer
   push r2;
#endif

#ifdef AMBER_STYLE_INT_CTRL
#ifndef CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
   // If CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS is defined, then we
   // dont need to go through this code since r2 will have the currect interrupt
   // source

   // Interrupt source can be 0-71; 0-23 is low pri, 24-47 med pri
   // and 48-71 high pri. mod by 24 to find the isr call table indx
   // but division is costly, takes 12 cycles. Instead do a
   // loop unrolled repeated subtraction.
   // Rick has Amber-style int controller, but number of int sources per level is 48.
   r1 = r2 - NUM_INT_SOURCES_PER_PRIORITY;
   // if source >= NUM_INT_SOURCES_PER_PRIORITY, subtract NUM_INT_SOURCES_PER_PRIORITY. else nop
   if GE r2 = r1;
   r1 = r2 - NUM_INT_SOURCES_PER_PRIORITY;
   // if source >= NUM_INT_SOURCES_PER_PRIORITY, subtract NUM_INT_SOURCES_PER_PRIORITY. else nop
   if GE r2 = r1;
   // r2 contains M[$INT_SOURCE]%NUM_INT_SOURCES_PER_PRIORITY
#endif // !CHIP_HAS_SEPARATE_INTERRUPT_ENABLE_AND_PRIO_REGS
#endif   // AMBER_STYLE_INT_CTRL

   // load appropriate routine into r3 for later
   r1 = r2 * ADDR_PER_WORD (int);
   r3 = M[r1 + &$interrupt.isr_call_table];

   // switch to user mode
   rFlags = $UM_FLAG;
   // unblock interrupts if already unblocked before interrupt occured
   r0 = 1;
   Null = M[$interrupt.block_count];
   if NZ r0 = 0;

   M[$INT_UNBLOCK] = r0;

#ifdef A7DA_KAS_STYLE_INT_CTRL
   // store keyhole data, check only busy to allow error to fall through
   keyhole_save_wait:
      r0 = M[$KALIMBA_KAS_KEY_STATUS];
      Null = r0 - $KALIMBA_KAS_KEY_STATUS__BUSY__MASK;
      if Z jump keyhole_save_wait;

   r0 = M[$KALIMBA_KAS_KEY_ADDR_MSW]; push r0;
   r0 = M[$KALIMBA_KAS_KEY_ADDR_LSW]; push r0;
   r0 = M[$KALIMBA_KAS_KEY_DATA_MSW]; push r0;
   r0 = M[$KALIMBA_KAS_KEY_DATA_LSW]; push r0;
   r0 = M[$KALIMBA_KAS_KEY_BYTE_EN ]; push r0;
#endif

   // call the appropriate interrupt source service routine
   call r3;

#ifdef A7DA_KAS_STYLE_INT_CTRL
   // restore keyhole data
   keyhole_restore_wait:
      r0 = M[$KALIMBA_KAS_KEY_STATUS];
      Null = r0 - $KALIMBA_KAS_KEY_STATUS__BUSY__MASK;
      if Z jump keyhole_restore_wait;

   pop r0; M[$KALIMBA_KAS_KEY_BYTE_EN ] = r0;
   pop r0; M[$KALIMBA_KAS_KEY_DATA_LSW] = r0;
   pop r0; M[$KALIMBA_KAS_KEY_DATA_MSW] = r0;
   pop r0; M[$KALIMBA_KAS_KEY_ADDR_LSW] = r0;
   pop r0; M[$KALIMBA_KAS_KEY_ADDR_MSW] = r0;
#endif

#ifdef A7DA_KAS_STYLE_INT_CTRL
   // retrieve int_source
   pop r2;
   // r2 contain the int_source, kas_intr_bounce only takes into account
   // the external interrupt signals, so it needs subtract 8... annoying
   r2 = r2 - $KAS_INTR_BOUNCE_SHIFT;
   // No need to protect for bounce if it is internal interrupt
   if LE jump no_bounce;
   // Store bounce value
   r1 = M[$KALIMBA_KAS_INTR_BOUNCE];
   // interrupt bouncing enable, de-assert the line
   r0 = 1 LSHIFT r2;
   r1 = r1 OR r0;
   M[$KALIMBA_KAS_INTR_BOUNCE] = r1;
   // interrupt bouncing disable
   r0 = r1 XOR r0;
   M[$KALIMBA_KAS_INTR_BOUNCE] = r0;
   no_bounce:
#endif

   #ifdef INTERRUPT_PROFILER_ON
      NULL = M[$profiler.enabled];
      if Z jump no_profiler2;
      r0 = M[$interrupt.nest_count];
      r0 = r0 - 1;
      r1 = $interrupt.MAX_NESTED_NR - 1;
      Null = r0 - r1;
      if GT r0 = r1;
      // increment the total time that we've serviced interrupts for
      // TotalTime = TotalTime + (TIMER_TIME - StartTime)
      r1 = M[$interrupt.total_time];
      Words2Addr(r0);
      r2 = M[$interrupt.start_time + r0];
      r1 = r1 - r2;
      r1 = r1 + M[$TIMER_TIME];
      M[$interrupt.total_time] = r1;

      #ifdef DETAILED_PROFILER_ON
         #ifdef K32
            r0 = M[$NUM_RUN_CLKS];
            r1 = M[$NUM_CORE_STALLS];
            r2 = M[$NUM_INSTRS];

            r0 = r0 + M[$interrupt.total_clocks];
            r1 = r1 + M[$interrupt.total_core_stalls];
            r2 = r2 + M[$interrupt.total_instrs];
            r0 = r0 - M[$interrupt.start_clocks];
            r1 = r1 - M[$interrupt.start_core_stalls];
            r2 = r2 - M[$interrupt.start_instrs];

            // Extra clocks around NUM_RUN_CLKS reads (before reading the initial
            // $NUM_RUN_CLKS value + clocks from reading the second $NUM_RUN_CLKS until
            // the rti) in this function
            #ifdef CHIP_BASE_CRESCENDO
               r0 = r0 + (27 + 144);
            #else
               r0 = r0 + (27 + 134);
            #endif
            // Extra instructions around NUM_INSTRS reads (before reading the initial
            // $NUM_INSTRS value + instructions from reading the second $NUM_INSTRS
            // until the rti) in this function
            #ifdef CHIP_BASE_CRESCENDO
               r2 = r2 + (18 + 77);
            #else
               r2 = r2 + (18 + 69);
            #endif
            M[$interrupt.total_clocks] = r0;
            r0 = M[$PREFETCH_WAIT_OUT_COUNT];
            M[$interrupt.total_core_stalls] = r1;
            r0 = r0 + M[$interrupt.total_prefetch_wait];
            M[$interrupt.total_instrs] = r2;
            r0 = r0 - M[$interrupt.start_prefetch_wait];
            M[$interrupt.total_prefetch_wait] = r0;

            #ifdef CHIP_BASE_CRESCENDO
               r0 = M[$NUM_MEM_ACCESS_STALLS];
               r1 = M[$NUM_INSTR_EXPAND_STALLS];
               r0 = r0 + M[$interrupt.total_access_stalls];
               r1 = r1 + M[$interrupt.total_instr_expand_stalls];
               r0 = r0 - M[$interrupt.start_access_stalls];
               r1 = r1 - M[$interrupt.start_instr_expand_stalls];
               M[$interrupt.total_access_stalls] = r0;
               M[$interrupt.total_instr_expand_stalls] = r1;
            #endif
            nop;
         #else
            r0 = M[$NUM_RUN_CLKS_MS];
            r2 = M[$NUM_RUN_CLKS_MS];
            r1 = M[$NUM_RUN_CLKS_LS];
            r1 = r1 - 1;
            r2 = r2 - Borrow;
            Null = r2 - r0;
            if LT r2 = r0;

            // TODO these numbers are unlikely to be correct
            // Check these numbers before using DETAILED_PROFILER_ON
            // Extra clocks around NUM_RUN_CLKS_LS reads (cloks before reading the
            // initial value + clocks until the context switch) in this function
            r1 = r1 + (24 + 58);
            r2 = r2 + Carry;
            r1 = r1 + M[$interrupt.total_clocks_ls];
            r2 = r2 + M[$interrupt.total_clocks_ms] + Carry;
            r1 = r1 - M[$interrupt.start_clocks_ls];
            r2 = r2 - M[$interrupt.start_clocks_ms] - Borrow;
            M[$interrupt.total_clocks_ls] = r1;
            M[$interrupt.total_clocks_ms] = r2;
            r0 = M[$NUM_INSTRS_MS];
            r2 = M[$NUM_INSTRS_MS];
            r1 = M[$NUM_INSTRS_LS];
            r1 = r1 - 1;
            r2 = r2 - Borrow;
            Null = r2 - r0;
            if LT r2 = r0;

            // TODO these numbers are unlikely to be correct
            // Check these numbers before using DETAILED_PROFILER_ON
            // Extra instructions around NUM_RUN_CLKS_LS reads (instructions before reading
            // the initial value + instructions until the context switch) in this function
            r1 = r1 + (31 + 42);
            r2 = r2 + Carry;
            r1 = r1 + M[$interrupt.total_instrs_ls];
            r2 = r2 + M[$interrupt.total_instrs_ms] + Carry;
            r1 = r1 - M[$interrupt.start_instrs_ls];
            r2 = r2 - M[$interrupt.start_instrs_ms] - Borrow;
            M[$interrupt.total_instrs_ls] = r1;
            M[$interrupt.total_instrs_ms] = r2;
            r1 = M[$NUM_CORE_STALLS_LS];
            r2 = M[$NUM_CORE_STALLS_MS];
            r1 = r1 + M[$interrupt.total_stalls_ls];
            r2 = r2 + M[$interrupt.total_stalls_ms] + Carry;
            r1 = r1 - M[$interrupt.start_stalls_ls];
            r2 = r2 - M[$interrupt.start_stalls_ms] - Borrow;
            M[$interrupt.total_stalls_ls] = r1;
            M[$interrupt.total_stalls_ms] = r2;
            nop;
         #endif
      #endif
no_profiler2:
   #endif
   // Disable all interrupts while we check for a possible context switch.
   // They will be unblocked if the scheduler has anything to do.
   // (Need to do this before restoring INT_LOAD_INFO, otherwise we could
   // get interrupt stack frames for the same level nested to arbitrary
   // depth.)
   call $block_interrupts;

   // Inform the scheduler we're leaving an IRQ
   r0 = M[$_SchedInterruptActive];
   r0 = r0 - 1;
   M[$_SchedInterruptActive] = r0;

   // Get the previous value of INT_SAVE_INFO back from the stack
   // and write to INT_LOAD_INFO to allow interrupts at this level
   // if a context switch happens.
   pop r1;
   r1 = r1 OR $INT_LOAD_INFO_DONT_CLEAR_MASK;
   M[$INT_LOAD_INFO] = r1;

   // Check for any context switch
   call $_exit_irq_check_context_switch;

   // pop processor state from the stack
   #ifdef DEBUG_STACK_OVERFLOW
      // check we have enough data on the stack for restoring the processor's state
      r0 = M[$STACK_POINTER];
      r0 = r0 - $interrupt.RESTORE_CHECK_SIZE*ADDR_PER_WORD;
      Null = r0 - M[$STACK_START_ADDR];
      if LT call $error;
   #endif

   // we must block interrupts (by clearing UM flag) before popping rFlags otherwise
   // if an interrupt occurs the MS 8bit of rFlags (the interrupt copy) will get lost
   // Interrupts also need to be blocked from the point of restoring MM_RINTLINK
   // up to the end of the interrupt handler
   rFlags = 0;

   // Re-enable interrupts (still actually blocked until the rti as UM flag is clear)
   call $unblock_interrupts;

   // restore registers from the stack
   // There's a hazard with popping the Div* registers -- if a division
   // were in progress in the background, the registers wouldn't be restored.
   // But there shouldn't be a division within 12 cycles of here...
   popm <rMAC2, rMAC12, rMAC0, DoLoopStart, DoLoopEnd, DivResult, DivRemainder, rMACB2, rMACB12, rMACB0, B0, B1, B4, B5>;
   popm <I0, I1, I2, I3, I4, I5, I6, I7, M0, M1, M2, M3, L0, L1, L4, L5>;
   popm <r2, r3, r4, r5, r6, r7, r8, r9, r10, rLink>;
   M[$MM_RINTLINK] = r2;

   #ifdef INTERRUPT_PROFILER_ON
      NULL = M[$profiler.enabled];
      if Z jump no_profiler3;
      r0 = M[$interrupt.nest_count];
      r0 = r0 - 1;
      r1 = $interrupt.MAX_NESTED_NR - 1;
      Null = r0 - r1;
      if GT r0 = r1;
      // increment the total time that we've serviced interrupts for
      // TotalTime = TotalTime + (TIMER_TIME - StartTime)
      Words2Addr(r0);
      r1 = M[$interrupt.total_time_with_task + r0];
      r2 = M[$interrupt.start_time + r0];
      r1 = r1 - r2;
      r1 = r1 + M[$TIMER_TIME];
      M[$interrupt.total_time_with_task + r0] = r1;
no_profiler3:
   #endif

   // restore the arithmetic mode
#ifdef CHIP_BASE_BC7
   // and non-interrupt clock rate for BC7
   popm <r0, r1>;
   M[$CLOCK_DIVIDE_RATE] = r1;
#else
   pop r0;
#endif
   M[$ARITHMETIC_MODE] = r0;

#if !defined(DISABLE_SHALLOW_SLEEP) && !CHIP_HAS_SHALLOW_SLEEP_REGISTERS
#ifdef CHIP_BASE_A7DA_KAS
   // Signal wakeup
   M[$interrupt.shallow_sleep_danger]=NULL;
   r0 = M[$interrupt.nest_count];
   r0 = r0 - 1;
   M[$interrupt.nest_count] = r0;
#else
   // An interrupt at the point of stopping the clock can wedge the system
   // until another interrupt occurs, so make sure we don't do that.

   // First check if we're about to return to background code
   // i.e not in a nested interrupt

   r0 = M[$interrupt.nest_count];
   r0 = r0 - 1;
   M[$interrupt.nest_count] = r0;

   if NZ jump safe_rti;

   // Now check for the danger area

   r0 = M[$interrupt.shallow_sleep_danger];

   if Z jump safe_rti;

   // ISR exit at point of enabling shallow sleep
   // Make sure we set the right (fast) clock divider
   popm <FP, r0, r1, r2, rFlags>;
   r0 = $CLK_DIV_1;
   rti;

safe_rti:
#endif
#elif defined(INTERRUPT_PROFILER_ON)
    // The interrupt profiler is using the nested count. Decrement it.
   r0 = M[$interrupt.nest_count];
   r0 = r0 - 1;
   M[$interrupt.nest_count] = r0;
#endif

   // Normal ISR exit, restore the last few registers and return

   popm <FP, r0, r1, r2, rFlags>;
   rti;

.ENDMODULE;


#ifndef DISABLE_SHALLOW_SLEEP

// Shallow sleep entry code
// Not strictly interrupt handling, but it's here because on some platforms
// it's intimately linked with the checking stuff at ISR exit.


.MODULE $M.safe_enable_shallow_sleep;
.CODESEGMENT INT_SHALLOW_SLEEP_PM;

$_safe_enable_shallow_sleep:

goto_sleep:
#if CHIP_HAS_SHALLOW_SLEEP_REGISTERS
    r0 = 1;
    M[$ALLOW_GOTO_SHALLOW_SLEEP] = r0;
    // Check whether an interrupt caused some work to be needed just before
    // entering shallow sleep.
    r1 = M[$_TotalNumMessages];
    if NZ jump no_shallow_sleep;

goto_shallow_sleep:
    // Will trigger shallow sleep, unless ALLOW_GOTO_SHALLOW_SLEEP==0
    // (which might have been done by dorm_wake() in an interrupt)
    M[$GOTO_SHALLOW_SLEEP] = r0;

    nop;    // Processor sleeps on this instruction until an interrupt fires

    // If the interrupt had work for the scheduler to do it will have
    // cleared this register. Otherwise we can go back to sleep.
    Null = M[$ALLOW_GOTO_SHALLOW_SLEEP];
    if NZ jump goto_shallow_sleep;

    // The processor must have been woken by an interrupt so switch off
    // the shallow sleep. Normally the interrupt code will have already
    // done this but we need to do it here in case the interrupt
    // happened just before we set the register to 1.
    r0 = 0;
    M[$GOTO_SHALLOW_SLEEP] = r0;

no_shallow_sleep:
    // Turn off the allow shallow sleep in case we abandoned it at the last
    // opportunity (an interrupt fired just before setting it)
    r0 = 0;
    M[$ALLOW_GOTO_SHALLOW_SLEEP] = r0;

#else // CHIP_HAS_SHALLOW_SLEEP_REGISTERS

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
    // Shallow sleep = divide rate of zero (clocks stopped)
    r0 = 0;
#elif defined(CHIP_BASE_A7DA_KAS)
    // Do not go to shallow sleep for A7DA_KAS.
    r0 = 1;
#elif  defined(CHIP_BASE_BC7)
    // BC7 has a completely different way of going low power, it will slow down the clock
    r0 = $CLK_DIV_MAX;
    // TODO_CONSIDER_DELETE
    // pre-hydra defined this in timer.asm dependent on architecture, but we only care about gordon
    //.CONST   MAX_CLK_DIV_RATE       ($CLK_DIV_MAX < $CLK_DIV_1024) ? $CLK_DIV_MAX : $CLK_DIV_1024;
#else
    #error "safe_enable_shallow_sleep not implemented for this chip"
#endif

    // Set a flag to note that we're in the danger area
    r1 = 1;
    M[$interrupt.shallow_sleep_danger] = r1;
    // Any time after this, an interrupt may come in and change our
    // r0 to CLK_DIV_1!

    // If the above was interrupted, we might have just missed the flag
    // But that's OK, as we can check if the interrupt(s) have given
    // us something to do

    r1 = M[$_TotalNumMessages];
    if NZ jump no_shallow_sleep;

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_A7DA_KAS) || defined(CHIP_BASE_NAPIER)
    // Get ready to set the clock divider back to full speed
    r1 = $CLK_DIV_1;
#elif  defined(CHIP_BASE_BC7)
    // save current clock rate
    r1 = M[$CLOCK_DIVIDE_RATE];
#else
    #error "safe_enable_shallow_sleep not implemented for this chip"
#endif

    // Now store the (possibly modified by interrupt) divide rate
    M[$CLOCK_DIVIDE_RATE] = r0;
    // hydra: by writing 0 in this register the processor clock stops!
    // the next intruction executed will be after an interrupt was served

#ifdef CHIP_BASE_A7DA_KAS
   wait:
      NULL = M[$interrupt.shallow_sleep_danger];
      if NZ jump wait;
#endif /* CHIP_BASE_A7DA_KAS */

#ifdef CHIP_BASE_BC7
    wait:
        Null = r0 - $CLK_DIV_MAX;  // ISR will change that when shallow_sleep_danger
        if Z jump wait;
        // TODO DEBUG_ON timeout based on TIMER_TIME
#endif

    // Set the divide rate back to full speed immediately on wakeup.
    // This is needed for SPI / TBus wakeups,
    // where the interrupt handler doesn't run.
    M[$CLOCK_DIVIDE_RATE] = r1;
no_shallow_sleep:

    // Out of danger now
    M[$interrupt.shallow_sleep_danger] = 0;

#endif // CHIP_HAS_SHALLOW_SLEEP_REGISTERS

    rts;

.ENDMODULE;

#endif
