// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

#ifndef RESET_INCLUDED
#define RESET_INCLUDED

// *****************************************************************************
// MODULE:
//    $reset.reset
//
// DESCRIPTION:
//    The reset vector. Control is transferred here by an external agency
// (usually the Curator) after the subsystem is powered on.
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
// NOTES:
//    In firmware that's booted by the Curator, this must be located in
// ROM/LPC/SQIF, and appear in the program address space at the precise (fixed)
// address that Curator expects. This is done by the use of the codesegment
// 'RESET_PM', which is located appropriately by the link script.
// (This must be the only code in 'RESET_PM', obviously.)
//    The Curator must have arranged for ROM/LPC/SQIF to appear in the program
// address space before starting us, of course. So one NVMEM_WINx_CONFIG
// register is already set up before our first instruction (on some chips,
// which one depends on the Curator MIB key AudioAddressMap, which needs to
// match how we were built; currently we're only ever built for NVMEM0 and
// the Curator key only exists for future-proofing.)
//
// In case of multi core, P0 must have setup the NVMEM_WINx before booting up
// the secondary core.
//
// *****************************************************************************
.MODULE $M.reset;
   .CODESEGMENT RESET_PM;
   .MAXIM;  // Curator transfers control to us in MaxiM mode
	
   $reset:
   $_reset:  // label for the SLT
   // On some versions of Kalimba, there are restrictions on the first
   // instruction. What exactly you can get away with is very variable, but
   // a NOP is always safe, so we just do that.
   // See <http://ukbugdb/B-208706#h10300090> for some more detail.
   nop;

#ifdef CHIP_HAS_SWITCHABLE_NVMEM
   // The invention of the AudioAddressMap Curator MIB key means we might
   // have accidentally been started running in a different NVMEM window
   // than what we were built for.
   // If we just carried on, then unpredictable chaos would ensue, probably
   // starting with the first jump we take. So fail hard (by hanging;
   // we're too confused to even call panic).
   // (This may also catch some situations that CHECK_SP_FOR_UNEXPECTED_RESTART
   // would have caught later.)

   // Stash rLink before we overwrite it (uninteresting normally, but vital
   // to diagnose PANIC_AUDIO_UNEXPECTED_RESTART).
   r1 = rLink;

   // Get PC into rLink where we can examine it.
   call get_pc;  // relying on this being assembled as relative call
get_pc:
   // If all is well, rLink should have the value of the get_pc label
   r2 = get_pc;   // in r2 because we'll use it again later
   Null = rLink - r2;
running_from_wrong_location:
   if NZ jump running_from_wrong_location;
#endif // CHIP_HAS_SWITCHABLE_NVMEM

   // Now jump somewhere else ASAP, to make room for the SLT fingerprint.
   // This may be an absolute jump (so we need to be sure we're running
   // from the right window before we take it).
   jump $pre_main;

   // In some builds there's still a bit of extra space here before the
   // SLT fingerprint (which is at a fixed location); that's notionally
   // reserved for anything else that might turn out to be necessary to
   // prime the processor before doing anything as complicated as a jump.

.ENDMODULE;

// Carry on the boot sequence after the reset vector.
.MODULE $M.pre_main;
   .CODESEGMENT PRE_MAIN_PM;
   .DATASEGMENT DM;
   // This code doesn't need to be MAXIM to work, but it reduces the size
   // of the jump instruction in RESET_PM (where size is critical).
   .MAXIM;

   $pre_main:
    
#if defined(INSTALL_DUAL_CORE_SUPPORT) && !defined(AUDIO_SECOND_CORE)
    // P0 and P1 are running from 2 different images
    // Jump to P1 reset code location if processor id = P1
    // More than 2 cores are not considered now. 

   r3 = $MEM_MAP_P1_CODE_START_ADDRESS;
   r0 = M[$PROCESSOR_ID];
   if NZ jump r3;
#endif // INSTALL_DUAL_CORE_SUPPORT and not AUDIO_SECOND_CORE 

#ifdef CHECK_SP_FOR_UNEXPECTED_RESTART

   // Check that the reset was a "normal" one 
   // and that the stack pointer is in its reset state (zero).
   // If it isn't, it's an unexpected reset (probably a NULL pointer call)
   r0 = M[$STACK_POINTER];

   if Z jump reset_ok;

   // This will write to the preserve block (which should work). It'll also
   // try to tell Curator, which might work if we were previously running
   // more or less normally (and this code is in the image we were running,
   // and we haven't e.g. jumped into the ROM when we were supposed to be
   // running from SQIF).
   r0 = 0x65; //PANIC_AUDIO_UNEXPECTED_RESTART
#ifndef CHIP_HAS_SWITCHABLE_NVMEM
   // rLink from before we hit the reset vector is probably a good clue.
   // (For CHIP_HAS_SWITCHABLE_NVMEM, it was already stashed in r1 above.)
   r1 = rLink;
#endif // CHIP_HAS_SWITCHABLE_NVMEM
   call $_panic_diatribe;
    
reset_ok:

#endif // CHECK_SP_FOR_UNEXPECTED_RESTART

   // We can't stash build_identifier_integer for unit test builds.
   // Arguably we still ought to stash CHIP_HAS_SWITCHABLE_NVMEM, but it's
   // fiddly and not actually going to be an issue so we don't bother with
   // that either. If the platform has AUDIO_SYS_PROC_TEST_REG it will
   // remain at its default value (0) in unit test builds.
#ifndef UNIT_TEST_BUILD

#ifdef CHIP_BASE_CRESCENDO

   // Stash the build ID in a register. This is to give us a clue in the
   // case where we're analysing a coredump and the coredump tool was
   // unable to read the build ID from the program storage into the 'II'
   // tag (this can happen for instance if the program is stored in an
   // external Flash device and that device is in a bad state or its
   // configuration doesn't match our Flash master's; see B-238493).
   //
   // In principle, this could apply to any platform. In practice it's
   // only been an issue on Crescendo and derivatives with external QSPI.
   // So out of laziness we've only included this code on
   // CHIP_BASE_CRESCENDO, which protects for the following assumptions:
   //  - AUDIO_SYS_PROC_TEST_REG register exists
   //  - Constants are mapped in by this point (Hydra architecture)
   //  - use of 32-bit instructions
   // 
   // (This is quite a profligate use of AUDIO_SYS_PROC_TEST_REG bits;
   // if we change our minds later we can allocate one of the spare bits
   // as a flag.)

   r0 = MHU[$_build_identifier_integer];        // a uint16
   r0 = r0 LSHIFT DAWTH-16;                     // put in top 16 bits

#ifdef CHIP_HAS_SWITCHABLE_NVMEM

   // We're now fairly sure that we're running from the NVMEM window we
   // were built to run from. Advertise what that window is in a well-known
   // per-processor register, for the benefit of external tools (coredump,
   // ACAT, etc) -- they need this clue, because there may end up being
   // different valid SLTs in different NVMEM windows, and they need to know
   // which one corresponds to the running firmware, and they can't
   // necessarily find out from Curator.
   // Do this after CHECK_SP_FOR_UNEXPECTED_RESTART, because if that fails
   // then we don't want to confuse things further!

   // r2 is still a random address in the correct window from above
   r2 = r2 LSHIFT -23;   // NVMEM windows are 0x80_0000 big starting at 0
                         // 0=NVMEM0, 1=NVMEM1
   r0 = r0 OR r2;

#endif // CHIP_HAS_SWITCHABLE_NVMEM

   // Stash clues for off-chip entities.
   // No writes to this register are currently expected after this point (by
   // firmware or other entities). Off-chip entities may read it at any time.
   // 
   // Bit allocation of AUDIO_SYS_PROC_TEST_REG on 32-bit platforms (update
   // this comment if you use bits of this register anywhere in the system,
   // even if you don't touch the code here):
   //   0-1    PM NVMEM window firmware is running from (Crescendo has 4)
   //          (CHIP_HAS_SWITCHABLE_NVMEM); must not be touched after this
   //          point
   //   2-15   Reserved, must be zero
   //   16-31  Version number of running firmware (0 is not valid, so if 0
   //          then this register has not been initialised yet); must not
   //          be touchde after this point
   M[$AUDIO_SYS_PROC_TEST_REG] = r0;

#endif // CHIP_BASE_CRESCENDO

#endif // UNIT_TEST_BUILD

   // Set up NVMEM windows for platform
   call $flash.init_pm;

   // Do the rest of the C runtime initialisation.

#ifdef AUDIO_SECOND_CORE

    // Jump to crt routine for secondary core
   r0 = M[$PROCESSOR_ID];
   if NZ jump $_crt_p1;

#endif //AUDIO_SECOND_CORE

   // Jump to crt0 for primary core
   jump $_crt0;

.ENDMODULE;

#endif // RESET_INCLUDED
