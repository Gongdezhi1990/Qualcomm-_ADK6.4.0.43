// *****************************************************************************
// Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
//   %%version
// *****************************************************************************

#ifndef INTERRUPT_INCLUDED
#define INTERRUPT_INCLUDED

#include "interrupt_inc.asm"
#include "kaldwarfregnums.h"
#include "dwarf_constants.h"
#include "io/io_defs.asm"

.MODULE $interrupt;
   .DATASEGMENT DM;

    // Store as a constant the size of the stack required to store the
    // processor's state.  Also create a constant to hold the nested
    // interrupt state
   .CONST   STORE_CHECK_SIZE       $INTERRUPT_STORE_CHECK_SIZE;
   .CONST   RESTORE_CHECK_SIZE     $INTERRUPT_RESTORE_CHECK_SIZE;

#ifndef DISABLE_SHALLOW_SLEEP
   .VAR shallow_sleep_danger = 0;
   .VAR nest_count = 0;
#endif

   // counter to support nested blocking/unblocking of interrupts
   .VAR block_count = 0;


.ENDMODULE;

#ifdef ENABLE_SHALLOW_SLEEP_TIMING
   // Variables to support measuring time spent in shallow sleep
   .VAR/DM $_get_shallow_sleep_exit_time = 0;
   .VAR/DM $_shallow_sleep_entry_time = 0;
   .VAR/DM $_shallow_sleep_exit_time = 0;
   .VAR/DM $_total_shallow_sleep_time = 0;
#endif


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
   .MINIM;

   $_interrupt_block:
   $_block_interrupts:
   $interrupt.block:
   .BlockIntsStart:

   // Register write to block all IRQs, and wait for this to take effect
#ifdef CHIP_DOESNT_HAVE_INT_PRIORITY_BLOCKING
   M[$INT_UNBLOCK] = Null;
#else
   r0 = 3;
   M[$INT_BLOCK_PRIORITY] = r0;
#endif
   nop;
   nop;
   nop;

   //May have had an IRQ while we were waiting for block to take effect
   //If so, the IRQ will have re-enabled interrupts.
   //Check for this, and if so try again....
#ifdef CHIP_DOESNT_HAVE_INT_PRIORITY_BLOCKING
   Null = M[$INT_UNBLOCK];
   if NZ jump $_block_interrupts;
#else
   r0 = M[$INT_BLOCK_PRIORITY];
   if Z jump $_block_interrupts;
#endif
   //Now IRQs are disabled, update the block_count
   r0 = M[$interrupt.block_count];
   r0 = r0 + 1;
   M[$interrupt.block_count] = r0;

   rts;
   .BlockIntsEnd:

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
   .CODESEGMENT INTERRUPT_BLOCK_PM;
   .MINIM;

   $_interrupt_unblock:
   $_unblock_interrupts:
   $interrupt.unblock:
   .UnblockIntsStart:

   r0 = M[$interrupt.block_count];
   if Z call $error;

   // decrement the count
   r0 = r0 - 1;
   M[$interrupt.block_count] = r0;
   // only unblock interrupts if at outer part of nesting
   if NZ rts;
   // unblock interrupts
#ifdef CHIP_DOESNT_HAVE_INT_PRIORITY_BLOCKING
   r0 = 1;
   M[$INT_UNBLOCK] = r0;
#else
   M[$INT_BLOCK_PRIORITY] = 0;
#endif
   rts;
   .UnblockIntsEnd:

.ENDMODULE;

/* Debug frame information for both Block and UnblockInterrupt
 * functions. This is just a blank one because there are no
 * push/pop instructions.
 */
    .section ".debug_frame"
.IBLK_CIE_Pointer:
    .4byte  .IBLK_CIE_End-.IBLK_CIE_Start   /* CIE length */
.IBLK_CIE_Start:
    .4byte  0xFFFFFFFF  /* CIE_id */
    .byte   0x01        /* CIE version */
    .string ""          /* CIE augmentation */
    .byte   0x01        /* code alignment factor */
    .byte   0x04        /* data alignment factor */
    .byte   KalDwarfRegisterNum_RegrLINK    /* return address register */

    .byte   DW_CFA_def_cfa
    .byte   KalDwarfRegisterNum_RegFP
    .byte   0x00
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegSP
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegrMAC
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR0
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR1
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR2
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR3
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR4
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR5
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR6
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR7
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR8
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR9
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR10
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegrLINK
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegFP

    .byte   DW_CFA_nop
    .byte   DW_CFA_nop
.IBLK_CIE_End:

    .4byte  .IBLK_FDE_End-.IBLK_FDE_Start       /* FDE length */
.IBLK_FDE_Start:
    .4byte  .IBLK_CIE_Pointer                   /* CIE_pointer */
    .4byte  $_block_interrupts                  /* initial_location */
    .4byte  .BlockIntsEnd-.BlockIntsStart /* address_range */
    .byte   DW_CFA_advance_loc2
    .2byte  .BlockIntsEnd-.BlockIntsStart
    .byte   DW_CFA_nop
.IBLK_FDE_End:

    .4byte  .IUBLK_FDE_End-.IUBLK_FDE_Start       /* FDE length */
.IUBLK_FDE_Start:
    .4byte  .IBLK_CIE_Pointer                   /* CIE_pointer */
    .4byte  $_unblock_interrupts                  /* initial_location */
    .4byte  .UnblockIntsEnd-.UnblockIntsStart /* address_range */
    .byte   DW_CFA_advance_loc2
    .2byte  .UnblockIntsEnd-.UnblockIntsStart
    .byte   DW_CFA_nop
.IUBLK_FDE_End:
    .previous

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
   .MINIM;

// NOTE: be careful when changing code in here to allow for it in the calculations
//       in the DETAILED_PROFILER section at the bottom.  Similarly, the pydbg
//       stack unwinder (AppsStackUnwinder._unwind in stack_unwinder.py) has
//       hard-coded address offsets for its hand-coded rIntLink restoration
//       logic which may need to be adjusted.

   $interrupt.handler:
   $_interrupt_handler:
   nop;      // B-176121 as a workaround for B-88464

// NOTE: The first instruction of the interrupt handler must not be a
//       write to the rFlags register, as the UM bit is cleared 1
//       instruction after the rest of the flags have a copy of them
//       taken. See B-88464.

   #ifdef DEBUG_STACK_OVERFLOW
      // check the interrupt hasn't fired because of stack overflow
      Null = M[$STACK_OVERFLOW_PC];
      if NZ call $error_stack_overflow;
   #endif
.InterruptEntry:
   pushm <FP(=SP), r0, r1, r2, rFlags>;
.IntPush1_Fp012Fl:


#ifndef MINIMAL_C_INTERRUPT_FRAME
   r0 = M[$ARITHMETIC_MODE];
   push r0;
   M[$ARITHMETIC_MODE] = Null;
#endif

   #ifndef DISABLE_SHALLOW_SLEEP
      r0 = M[$interrupt.nest_count];
      r0 = r0 + 1;
      M[$interrupt.nest_count] = r0;
   #endif

#if CHIP_HAS_SHALLOW_SLEEP_REGISTERS
    r0 = 0;
    M[$GOTO_SHALLOW_SLEEP] = r0;
#else
   // set to fastest non-interrupt clock rate
   r0 = $CLOCK_RATE_MAX_ENUM;
   M[$CLOCK_DIVIDE_RATE] = r0;
#endif /* CHIP_HAS_SHALLOW_SLEEP_REGISTERS */

#ifndef MINIMAL_C_INTERRUPT_FRAME
   // disable any bit reverse addressing on AG1
   r1 = $BR_FLAG_MASK;
   r1 = r1 XOR -1;
   rFlags = rFlags AND r1;
#endif

   #ifdef DEBUG_STACK_OVERFLOW
      // check we've space on the stack for storing the processors state
      r0 = M[$STACK_POINTER];
      r0 = r0 + ($interrupt.STORE_CHECK_SIZE - 1);
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
#ifdef MINIMAL_C_INTERRUPT_FRAME
   pushm <r2, r3, r10, rLink>;
   pushm <rMAC2, rMAC1, rMAC0>;
   pushm <DoLoopStart, DoLoopEnd, DivResult, DivRemainder>;
#else
   pushm <r2, r3, r4, r5, r6, r7, r8, r9, r10, rLink>;
   pushm <I0, I1, I2, I3, I4, I5, I6, I7, M0, M1, M2, M3, L0, L1, L4, L5>;
   pushm <rMAC2, rMAC1, rMAC0, DoLoopStart, DoLoopEnd, DivResult, DivRemainder, rMACB2, rMACB1, rMACB0, B0, B1, B4, B5>;
#endif
.IntPush2:


   // The old value of INT_SAVE_INFO needs to be written to INT_LOAD_INFO 
   // to re-enable interrupts at this level before attempting a context switch.
   // So we put it on the stack last.
   push r1;

#ifdef ENABLE_SHALLOW_SLEEP_TIMING
    // Capture timer time here if this is the first interrupt after shallow
    // sleep
    Null = M[$_get_shallow_sleep_exit_time];
    if Z jump dont_capture_shallow_sleep_exit_time;
    rMAC = M[$TIMER_TIME];
    M[$_shallow_sleep_exit_time] = rMAC;
    M[$_get_shallow_sleep_exit_time] = 0;
dont_capture_shallow_sleep_exit_time:
#endif // ENABLE_SHALLOW_SLEEP_TIMING

   // Acknowledge interrupt and clear it:
   // - This will allow another interrupt to occur if we were in user mode,
   //   but we're not, so it will allow them once the UM flag is set.
   // - For nested interrupt the INT_SOURCE register will now be valid up until we
   //   re-enable the INT_UNBLOCK register.
   M[$INT_ACK] = Null;

#ifndef MINIMAL_C_INTERRUPT_FRAME
   // clear the length registers
   L0 = 0;
   L1 = 0;
   L4 = 0;
   L5 = 0;

   // clear the base registers
   push Null;
   B5 = M[SP - 4];
   B4 = M[SP - 4];
   B1 = M[SP - 4];
   pop B0;

   // Set the modify registers as the compiler expects (CS-124812-UG)
   M0 = 0;
   M1 = 4;
   M2 = -4;
#endif

   // We're not clearing the DOLOOP registers here, so they'll
   // remain as they were in the interrupted code.
   // There shouldn't be any situation where this matters; code using
   // loops will set up the registers first, and there should be no
   // way to hit the loop end without going through such setup code.

   // Inform the scheduler we've had an IRQ
   r0 = M[$_SchedInterruptActive];
   r0 = r0 + 1;
   M[$_SchedInterruptActive] = r0;

   #if defined(DEBUG_INTERRUPTS) || defined(OPMGR_MEASURE_PROCESSING_TIME)
   // Log the time we start handling an interrupt
   Null = M[$_int_logging_enable];
   if NZ call $_log_int_entry;
   #endif

   // see what the interrupt source was
   r2 = M[$INT_SOURCE];
   
   // Interrupt source can be 0-70; 0-17 is low pri, 32-49 med pri
   // and 64-80 high pri. Mask to find the isr call table indx.
   r2 = r2 AND 0x1F;

   // load appropriate routine into r3 for later
#ifdef KAL_ARCH4
   // Shift for byte/word width difference
   r2 = r2 LSHIFT 2;
#endif

#ifdef LOG_IRQS
   r3 = M[r2 + &$_irq_counts];
   r3 = r3 + 1;
   M[r2 + &$_irq_counts]  = r3;
#endif /* LOG_IRQS */

   r3 = M[r2 + &$_isr_call_table];

   // switch to user mode
   rFlags = $UM_FLAG_MASK;

#ifdef CHIP_DOESNT_HAVE_INT_PRIORITY_BLOCKING
   // unblock interrupts if already unblocked before interrupt occured
   r0 = 1;
   Null = M[$interrupt.block_count];
   if NZ r0 = 0;
   M[$INT_UNBLOCK] = r0;
#else
   // If interrupt blocking from c code is using the INT_BLOCK_PRIORITY
   // register rather than INT_UNBLOCK then we don't need this check and
   // we can unconditionally unblock.
   r0 = 1;
   M[$INT_UNBLOCK] = r0;
#endif

   // call the appropriate interrupt source service routine
   call r3;
.IsrReturned:
   // Disable all interrupts while we check for a possible context switch.
   // They will be unblocked if the scheduler has anything to do.
   // (Need to do this before restoring INT_LOAD_INFO, otherwise we could
   // get interrupt stack frames for the same level nested to arbitrary
   // depth.)
   call $_block_interrupts;

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

   #if defined(DEBUG_INTERRUPTS) || defined(OPMGR_MEASURE_PROCESSING_TIME)
   // Log the time we finish handling an interrupt
   Null = M[$_int_logging_enable];
   if NZ call $_log_int_exit;
   #endif

   // pop processor state from the stack
   #ifdef DEBUG_STACK_OVERFLOW
      // check we've enough data on the stack for restoring the processor's state
      r0 = M[$STACK_POINTER];
      r0 = r0 - $interrupt.RESTORE_CHECK_SIZE;
      Null = r0 - M[$STACK_START_ADDR];
      if LT call $error;
   #endif

   // we must block interrupts (by clearing UM flag) before popping rFlags otherwise
   // if an interrupt occurs the MS 8bit of rFlags (the interrupt copy) will get lost
   // Interrupts also need to be blocked from the point of restoring MM_RINTLINK
   // up to the end of the interrupt handler 
   rFlags = 0;

   // Re-enable interrupts (still actually blocked until the rti as UM flag is clear)
   call $_unblock_interrupts;

   // restore registers from the stack

   // There's a hazard with popping the Div* registers -- if a division
   // were in progress in the background, the registers wouldn't be restored.
   // But there shouldn't be a division within 12 cycles of here...
#ifdef MINIMAL_C_INTERRUPT_FRAME
   popm <DoLoopStart, DoLoopEnd, DivResult, DivRemainder>;
   popm <rMAC2, rMAC12, rMAC0>;
   popm <r2, r3, r10, rLink>;
#else
   popm <rMAC2, rMAC12, rMAC0, DoLoopStart, DoLoopEnd, DivResult, DivRemainder, rMACB2, rMACB12, rMACB0, B0, B1, B4, B5>;
   popm <I0, I1, I2, I3, I4, I5, I6, I7, M0, M1, M2, M3, L0, L1, L4, L5>;
   popm <r2, r3, r4, r5, r6, r7, r8, r9, r10, rLink>;
#endif
.IntPop2:
   M[$MM_RINTLINK] = r2;


#ifndef MINIMAL_C_INTERRUPT_FRAME
   // restore the arithmetic mode
   pop r0;
   M[$ARITHMETIC_MODE] = r0;
#endif

#if !defined(DISABLE_SHALLOW_SLEEP) && !CHIP_HAS_SHALLOW_SLEEP_REGISTERS
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
   r0 = $CLOCK_RATE_MAX_ENUM;
   rti;

safe_rti:
#endif //DISABLE_SHALLOW_SLEEP

   // Normal ISR exit, restore the last few registers and return

.IntPop1_Fp012Fl:
   popm <FP, r0, r1, r2, rFlags>;
   rti;
.InterruptExit:

.ENDMODULE;


// Shallow sleep entry code
// Not strictly interrupt handling, but it's here 
// because it's intimately linked with the checking stuff at ISR exit.

.MODULE $M.enter_shallow_sleep;
    .CODESEGMENT PM;
    .MINIM;

$_enter_shallow_sleep:
.start_safe_sleep:
#ifndef DISABLE_SHALLOW_SLEEP

#ifdef CHIP_DEF_REMOTE_REQ_EXCEPTION_IN_SHALLOW_SLEEP_WORKAROUND_B_193365
    /* Clear the CPU1 read port */
    rMAC = $VM_CPU1_REQUESTS_READ_MASK;
    M[$VM_CPU1_CLEAR_PORT]=rMAC;
    /* Disable the CPU1 remote VM access exception before shallow sleep */
    rMAC = M[$APPS_BANKED_EXCEPTIONS_P1_ENABLES];
    r0 = $APPS_BANKED_EXCEPTIONS_P0__EXCEPTION_SOURCE_VM_CPU1_REMOTE_REQ_MASK;
    r0 = -1 - r0;
    rMAC = rMAC AND r0;
    M[$APPS_BANKED_EXCEPTIONS_P1_ENABLES] = rMAC;
#endif /* CHIP_DEF_REMOTE_REQ_EXCEPTION_IN_SHALLOW_SLEEP_WORKAROUND_B_193365 */

#ifdef ENABLE_SHALLOW_SLEEP_TIMING
    // Log timer time going into shallow sleep and set flag for the
    // first interrupt after shallow sleep to capture the exit time.
    rMAC = 1;
    M[$_get_shallow_sleep_exit_time] = rMAC;
    rMAC = M[$TIMER_TIME];
    M[$_shallow_sleep_entry_time] = rMAC;
#endif // ENABLE_SHALLOW_SLEEP_TIMING

#ifdef DEBUG_SLEEP_ON_PIO
    r0 = M[$APPS_SYS_PIO_DRIVE + (DEBUG_SLEEP_ON_PIO/32)];
    r0 = r0 OR (1<<(DEBUG_SLEEP_ON_PIO & 0x1f));
    M[$APPS_SYS_PIO_DRIVE + (DEBUG_SLEEP_ON_PIO/32)] = r0;
#endif

#if CHIP_HAS_SHALLOW_SLEEP_REGISTERS
    r0 = 1;
    M[$ALLOW_GOTO_SHALLOW_SLEEP] = r0;
    // Check whether an interrupt caused some work to be needed just before
    // entering shallow sleep.
    r1 = M[$_background_work_pending];
    if NZ jump no_shallow_sleep;

goto_shallow_sleep:
    M[$GOTO_SHALLOW_SLEEP] = r0;

    nop;    // Processor Sleeps on this instruction until an interrupt fires

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
    // oportunity (an interrupt fired just before setting it)
    r0 = 0;
    M[$ALLOW_GOTO_SHALLOW_SLEEP] = r0;

#else /* CHIP_HAS_SHALLOW_SLEEP_REGISTERS */
    // Shallow sleep = divide rate of zero (clocks stopped)
    r0 = $CLOCK_STOPPED_ENUM;

    // Set a flag to note that we're in the danger area
    r1 = 1;
    M[$interrupt.shallow_sleep_danger] = r1;
    // Any time after this, an interrupt may come in and change our
    // r0 to CLOCK_RATE_MAX_ENUM!

    // If the above was interrupted, we might have just missed the flag
    // But that's OK, as we can check if the interrupt(s) have given
    // us something to do 

    r1 = M[$_background_work_pending];
    if NZ jump no_shallow_sleep;

    // Get ready to set the clock divider back to full speed
    r2 = $CLOCK_DIVIDE_RATE;
    r1 = $CLOCK_RATE_MAX_ENUM;
  
    // Now store the (possibly modified by interrupt) divide rate
    M[r2] = r0;

    nop;    // Processor Sleeps on this instruction until an interrupt fires

    // Set the divide rate back to full speed immediately on wakeup.
    M[r2] = r1;

no_shallow_sleep:

    // Out of danger now
    M[$interrupt.shallow_sleep_danger] = 0;
#endif /* CHIP_HAS_SHALLOW_SLEEP_REGISTERS */

#ifdef DEBUG_SLEEP_ON_PIO
    r0 = M[$APPS_SYS_PIO_DRIVE + (DEBUG_SLEEP_ON_PIO/32)];
    r0 = r0 AND ~(1<<(DEBUG_SLEEP_ON_PIO & 0x1f));
    M[$APPS_SYS_PIO_DRIVE + (DEBUG_SLEEP_ON_PIO/32)] = r0;
#endif

#ifdef ENABLE_SHALLOW_SLEEP_TIMING
    // If the interrupt has logged the exit time then use it to calculate the
    // time spent in shallow sleep
    nop;
    nop;
    Null = M[$_get_shallow_sleep_exit_time];
    if NZ jump no_exit_time_measurement;
    rMAC = M[$_shallow_sleep_exit_time];
    rMAC = rMAC - M[$_shallow_sleep_entry_time];
    rMAC = rMAC + M[$_total_shallow_sleep_time];
    M[$_total_shallow_sleep_time] = rMAC;
no_exit_time_measurement:
#endif  // ENABLE_SHALLOW_SLEEP_TIMING

#ifdef CHIP_DEF_REMOTE_REQ_EXCEPTION_IN_SHALLOW_SLEEP_WORKAROUND_B_193365
    /* Re-enable the CPU1 remote VM access exception now we are out of
     * shallow sleep
     */
    rMAC = M[$APPS_BANKED_EXCEPTIONS_P1_ENABLES];
    r0 = $APPS_BANKED_EXCEPTIONS_P0__EXCEPTION_SOURCE_VM_CPU1_REMOTE_REQ_MASK;
    rMAC = rMAC OR r0;
    M[$APPS_BANKED_EXCEPTIONS_P1_ENABLES] = rMAC;
#endif /* CHIP_DEF_REMOTE_REQ_EXCEPTION_IN_SHALLOW_SLEEP_WORKAROUND_B_193365 */

#endif
    rts;
.end_safe_sleep:
.ENDMODULE;

/* Debug frame information for _enter_shallow_sleep
 * This is just a blank one because there are no
 * push/pop instructions.
 */
    .section ".debug_frame"
.SLEEP_CIE_Pointer:
    .4byte  .SLEEP_CIE_End-.SLEEP_CIE_Start   /* CIE length */
.SLEEP_CIE_Start:
    .4byte  0xFFFFFFFF  /* CIE_id */
    .byte   0x01        /* CIE version */
    .string ""          /* CIE augmentation */
    .byte   0x01        /* code alignment factor */
    .byte   0x04        /* data alignment factor */
    .byte   KalDwarfRegisterNum_RegrLINK    /* return address register */

    .byte   DW_CFA_def_cfa
    .byte   KalDwarfRegisterNum_RegFP
    .byte   0x00
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegSP
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegrMAC
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR0
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR1
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR2
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR3
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR4
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR5
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR6
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR7
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR8
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR9
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR10
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegrLINK
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegFP

    .byte   DW_CFA_nop
    .byte   DW_CFA_nop
.SLEEP_CIE_End:

    .4byte  .SLEEP_FDE_End-.SLEEP_FDE_Start     /* FDE length */
.SLEEP_FDE_Start:
    .4byte  .SLEEP_CIE_Pointer                  /* CIE_pointer */
    .4byte  .start_safe_sleep                   /* initial_location */
    .4byte  .end_safe_sleep - .start_safe_sleep /* address_range */
    .byte   DW_CFA_advance_loc2
    .2byte  .end_safe_sleep - .start_safe_sleep
    .byte   DW_CFA_nop
.SLEEP_FDE_End:

    .previous

/* Debug frame information for InterruptEntry.
 * This is a minimal dwarf frame in that it only documents the push/pops
 * associated with registers needed for stack unwinding (FP, rINTLINK, rLINK,
 * rMAC). The effect of this is that it may require more effort to debug
 * the interrupted function in some cases (by looking on the stack manually).
 */ 
    .section ".debug_frame"
.ISR_CIE_Pointer:
    .4byte  .ISR_CIE_End-.ISR_CIE_Start /* CIE length */
.ISR_CIE_Start:
    .4byte  0xFFFFFFFF  /* CIE_id */
    .byte   0x01    /* CIE version */
    .string ""  /* CIE augmentation */
    .byte   0x01    /* code alignment factor */
    .byte   0x04    /* data alignment factor */
    .byte   KalDwarfRegisterNum_RegrINTLINK    /* return address register */
    .byte   DW_CFA_def_cfa
    .byte   KalDwarfRegisterNum_RegFP
    .byte   0x00

    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegSP
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegrMAC
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR0
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR1
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR2
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR3
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR4
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR5
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR6
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR7
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR8
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR9
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR10
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegrLINK
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegFP

    .byte   DW_CFA_nop
    .byte   DW_CFA_nop

.ISR_CIE_End:
    .4byte  .ISR_FDE_End-.ISR_FDE_Start /* FDE length */
.ISR_FDE_Start:
    .4byte  .ISR_CIE_Pointer    /* CIE_pointer */
    .4byte  .InterruptEntry /* initial_location */
    .4byte  .InterruptExit-.InterruptEntry  /* address_range */
    .byte   DW_CFA_advance_loc2
    .2byte  .IntPush1_Fp012Fl-.InterruptEntry
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegFP
    .byte   0x00
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR0
    .byte   0x01
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR1
    .byte   0x02
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR2
    .byte   0x03
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegFlags
    .byte   0x04

#ifdef MINIMAL_C_INTERRUPT_FRAME
    .byte   DW_CFA_advance_loc2
    .2byte  .IntPush2-.IntPush1_Fp012Fl
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrINTLINK
    .byte   0x05
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR3
    .byte   0x06
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR10
    .byte   0x07
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrLINK
    .byte   0x08
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrMAC2
    .byte   0x09
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrMAC1
    .byte   0x0A
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrMAC0
    .byte   0x0B


    .byte   DW_CFA_advance_loc2
    .2byte  .IntPop2-.IntPush2
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrINTLINK
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR3
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR10
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrLINK
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrMAC2
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrMAC12
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrMAC0
#else
    .byte   DW_CFA_advance_loc2
    .2byte  .IntPush2-.IntPush1_Fp012Fl
    /* These are offset by +1 compared to minimal due to the push of arith mode */
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrINTLINK
    .byte   0x06
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR3
    .byte   0x07
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR4
    .byte   0x08
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR5
    .byte   0x09
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR6
    .byte   0x0A
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR7
    .byte   0x0B
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR8
    .byte   0x0C
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR9
    .byte   0x0D
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegR10
    .byte   0x0E
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrLINK
    .byte   0x0F
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrMAC2
    .byte   0x1F
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrMAC1
    .byte   0x20
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegrMAC0
    .byte   0x21

    .byte   DW_CFA_advance_loc2
    .2byte  .IntPop2-.IntPush2
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrINTLINK
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR3
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR4
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR5
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR6
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR7
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR8
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR9
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR10
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrLINK
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrMAC2
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrMAC12
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegrMAC0
#endif // MINIMAL_C_INTERRUPT_FRAME

    .byte   DW_CFA_advance_loc2
    .2byte  .IntPop1_Fp012Fl-.IntPop2
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegFP
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR0
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR1
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegR2
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegFlags
    .byte   DW_CFA_nop
    .byte   DW_CFA_nop
.ISR_FDE_End:
    .previous

#endif // INTERRUPT_INCLUDED
