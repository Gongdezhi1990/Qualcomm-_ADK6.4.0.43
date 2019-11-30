// Copyright (c) 2016 Qualcomm Technologies International, Ltd.
//   %%version
.MODULE $M.crt0;
   .CODESEGMENT PM;
   .MINIM;
   .DATASEGMENT DM;

$reset_minim:
    // Init the stack based on values set in the linker script
    nop;
    // Make an attempt to block interrupts in case this is a branch through
    // zero, so we don't risk panicking on the way to the branch-through-zero
    // check code
    M[$INT_UNBLOCK] = Null;

    // If STACK_START_ADDR is non-zero, we've branched through zero
    // rLink and the stack will still be valid so jumping to the
    // function will look like the last call was to the branch_through_zero
    // check.
    Null = M[$STACK_START_ADDR];
    if NE jump $_excep_branch_through_zero;

    // Turn on performance counters at the start of day. Among other things this
    // lets us see how many clock cycles we've been running for since boot
    r9 = 1;
    M[$PREFETCH_CONFIG] = r9 + Null;
    M[$DBG_COUNTERS_EN] = r9 + Null;

    r9 = $MEM_MAP_STACK_START;

    M[$STACK_START_ADDR] = r9 + Null;
    M[$STACK_POINTER] = r9 + Null;
    M[$FRAME_POINTER] = r9 + Null;
    r9 = r9 + ($STACK_SIZE_BYTES - STACK_OVERRUN_PROTECTION_BYTES);
    M[$STACK_END_ADDR] = r9;

    // Init the M registers for a processor with 32 bit byte addressed data
    M0 = 0;
    M1 = 4;
    M2 = -4;

    // Magic number the stack
    I0 = $MEM_MAP_STACK_START;
    r10 = $STACK_SIZE_BYTES;
    r10 = r10 LSHIFT -2;
    r2 = 0xAAAA;
    do stack_magic_loop;
        M[I0,4] = r2;
    stack_magic_loop:


    // Zero initialise the .bss data region
    I0 = $MEM_MAP_BSS_START;
    r10 = $MEM_MAP_BSS_LENGTH_DWORDS;
    r2 = 0;
    do bss_zero_loop;
        M[I0,4] = r2;
    bss_zero_loop:


    // Initialise the .initc region
    r10 = $MEM_MAP_INITC_LENGTH_DWORDS;
    I1 = $MEM_MAP_INITC_ROM_ADDR;
    I0 = $MEM_MAP_INITC_START;
    do initc_copy_loop;
        r1 = M[I1,4];
        M[I0,4] = r1;
    initc_copy_loop:

    // Call main
    call $_main;
    jump $_exit;

$_abort:
    r0 = Null - 1;

$_exit:
    kalcode(0x8d008000);

    // Infinite loop to stop us returning to failed code
$error:
    jump $error;

$_dump_registers:
    kalcode(0x8d028000);
    rts;


.ENDMODULE;
