/* Copyright (c) 2010 - 2018 Qualcomm Technologies International, Limited */

#if defined(ARCH_SIZE_32)
        .set $STACK_SIZE,	8096
#else
        .set $STACK_SIZE,       2048
#endif

        .section crt0data, "a"
_dummy:
#if defined(ARCH_SIZE_32)
        .skip 4
#else
        .skip 1
#endif

        .section crt0stack, "a"
#if defined(ARCH_SIZE_32)
        .balign 4
#endif

$c_stack:
	.skip $STACK_SIZE
                
        .section PM_RST, "ax"

$reset:
        /* First instruction should be a NOP for correct operation on real hardware */
        NOP;

        R9 = $c_stack; 
        M[$STACK_START_ADDR] = R9;
        M[$STACK_POINTER]    = R9;
        M[$FRAME_POINTER]    = R9;
        R9 = R9 + $STACK_SIZE;
        M[$STACK_END_ADDR]   = R9; 
        
        M0 = 0; 
#if defined(ARCH_SIZE_32)
        M1 = 4; 
        M2 = -4;
#else
        M1 = 1;
        M2 = -1;
#endif         
        call $_main; 
        jump _terminate; 

$_abort:
        R0 = -1;

$_exit:
        /* For calls to exit, r0 is set to exit code already */

$_finish_no_error:
_terminate:     

        kalcode (0x8D008000);

$_dump_registers:
        kalcode (0x8D028000);
        rts;

        /* Default implementation of putchar that just uses a kalcode;
         * link your own putchar implementation in for functionality that
         * works with, say, a debugger
         */
        .weak $_putchar
$_putchar:
        kalcode (0x8D018000);
        rts;

        /* System time in microseconds. Used by the default libc 
         * implementation of clock()
         */
$__CRT_timer_time:
        r0 = M[$TIMER_TIME];
        rts;

 
