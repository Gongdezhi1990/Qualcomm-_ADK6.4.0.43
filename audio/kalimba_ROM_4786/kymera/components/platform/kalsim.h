// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// 
//
// $Change$  $DateTime$
// *****************************************************************************

#ifndef INCLUDED_KALSIM_H
#define INCLUDED_KALSIM_H

// Macros used to control debug functions in Kalsim
// These have no effect on real hardware

// TERMINATE
// Shuts down Kalsim
// Causes an infinite loop on a real chip
// Usage:  TERMINATE

#define TERMINATE                \
   kalcode(0x8D008000);          \
   terminate_loop:               \
   jump terminate_loop;


// DUMP_REGISTERS
// Shows the registers on the screen, including the PC
// Usage:  DUMP_REGISTERS

#define DUMP_REGISTERS     \
   kalcode(0x8D028000);

// PUTCHAR_R0
// Prints the contents of R0 as an ascii character
// Usage: PUTCHAR_R0

#define PUTCHAR_R0         \
   kalcode(0x8D018000);


// PUTCHAR (register)
// Prints the contents of the passed register as an ascii character.  Requires stack to be initialised.
// Usage: PUTCHAR (r6)

#define PUTCHAR(register)  \
   push r0;                \
   r0 = register;          \
   PUTCHAR_R0              \
   pop r0;                 


// DUMP_DM_R0_R1
// Dumps data memory to a file, between the addresses in r0 and r1.
// Usage: DUMP_DM_R0_R1

#define DUMP_DM_R0_R1      \
   kalcode(0x8D038000);


// DUMP_DM (start_address, end_address)
// Dumps data memory to a file, between the addresses specified.  Required stack to be initialised.
// Usage: DUMP_DM (&$codec_in, &$codec_in + 0x200)

#define DUMP_DM(start_address, end_address)     \
   push r0;                                     \
   push r1;                                     \
   r0 = start_address;                          \
   r1 = end_address;                            \
   DUMP_DM_R0_R1                                \
   pop r1;                                      \
   pop r0;


// DUMP_DM_R0_R1
// Dumps data memory to a file, between the addresses in r0 and r1.
// Usage: DUMP_DM_R0_R1

#define DUMP_PM_R0_R1         \
   kalcode(0x8D048000);


// DUMP_DM (start_address, end_address)
// Dumps data memory to a file, between the addresses specified.  Required stack to be initialised.
// Usage: DUMP_DM (&$codec_in, &$codec_in + 0x200)

#define DUMP_PM(start_address, end_address)     \
   push r0;                                     \
   push r1;                                     \
   r0 = start_address;                          \
   r1 = end_address;                            \
   DUMP_PM_R0_R1                                \
   pop r1;                                      \
   pop r0;

#endif // INCLUDED_KALSIM_H

