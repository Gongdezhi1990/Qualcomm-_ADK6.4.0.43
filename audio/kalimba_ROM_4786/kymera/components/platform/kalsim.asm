// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

#include "kalsim.h"
#include "architecture.h"

// This file includes a bunch of routines that dump debug info for use in kalsim
// builds. Not sure whether they're very useful, especially putchar since it
// conflicts with pl_trace.


// -- dump register to stdout -- !!For kalsim use only!!
.MODULE $M.dump_registers;
   .CODESEGMENT PM;

   $_dump_registers:
   DUMP_REGISTERS
   rts;
.ENDMODULE;



// -- dump memory to stdout -- !!For kalsim use only!!
.MODULE $M.dump_memory;
   .CODESEGMENT PM;

   $_dump_memory:
   DUMP_DM_R0_R1
   rts;
.ENDMODULE;





// -- dump pm memory to stdout -- !!For kalsim use only!!
.MODULE $M.dump_pm_memory;
   .CODESEGMENT PM;

   $_dump_pm_memory:
   DUMP_PM_R0_R1
   rts;
.ENDMODULE;





// Writes a 7-bit ASCII value into the debug output stream.
// If a debugger is attached, the char is written into a packed buffer.
// Otherwise a magic break instruction is used that will cause kalsim // to print the char to stdout.
// Inputs:
//    r0 - the 7-bit ASCII character
// Trashes:
//    r0
// Time taken:
//    If debugger attached: about 20 cycles
//    Not attached:         5 cycles
.MODULE $M.putchar;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

#if defined(CHIP_NAPIER)
   .VAR/DM_SHARED_ZI $_stdout_buf[1024];
#else
   .VAR $_stdout_buf[1024];
#endif
   .VAR $_stdout_read_addr = &$_stdout_buf;
   .VAR $_stdout_write_addr = &$_stdout_buf;
   .VAR $_stdout_bitpos = 16;
   .VAR $_debugger_is_attached = 0;

   $_putchar:
   $putchar:

   // Is the debugger attached?
   Null = M[$_debugger_is_attached];
   if NZ jump debugger_attached;

      // No, so execute a magic shift instruction. Kalsim will interpret
      // this as a putchar request
      PUTCHAR_R0
      rts;


   debugger_attached:

      // Mask the character to 7bits, we only support 7 bit ascii
      r0 = r0 AND 0x7f;

      // Shift up the input char to the correct place
      r1 = M[$_stdout_bitpos];
      r0 = r0 LSHIFT r1;

      // OR it into the buffer
      r2 = M[$_stdout_write_addr];
      r3 = M[r2];
      r0 = r3 OR r0;
      M[r2] = r0;

      // Decrement stdout_bitpos
      r1 = r1 - 8;
      if POS jump word_not_full;

         // Increment write addr
         r2 = r2 + 1;

         // Do we need to wrap?
         Null = r2 - ($_stdout_buf + $_stdout_buf.length);
         if NEG jump no_wrap;
            r2 = $_stdout_buf;
         no_wrap:

         // Wait while the buffer is full
         is_buffer_full:
            r1 = M[$_stdout_read_addr];
            Null = r2 - r1;
         if Z jump is_buffer_full;

         // Write back the write addr
         M[$_stdout_write_addr] = r2;

         // Clear the new slot
         M[r2] = Null;

         // Move stdout_bitpos back to start of word
         r1 = 16;

      word_not_full:

      // Save stdout_bitpos
      M[$_stdout_bitpos] = r1;

      rts;
.ENDMODULE;





// fastdebug_printf(const char *string, ...)
// A fast version of printf that just sends to the host a pointer to the printf format string
// along with the argument list.
// NOTE:
// - For speed reasons it very simply analyses the format string assuming that every % equates to an argument,
//   Furthermore it assumes each argument has sizeof(arg)==sizeof(int). If this is not true the processing of the
//   argument concerned, and all later arguments, will be corrupted.
// - If no debugger is attched this function returns straight away
//
// Inputs:
//    Top of stack: printf format string, followed by arguments
// Time taken:
//    If debugger attached: ~40+(6*str_len) cycles
//    Not attached:         2 cycles
.MODULE $M.fastdebug_printf;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $_fastdebug_printf:
   $fastdebug_printf:

   // fastdebug printf is only supported if a debugger is attached
   Null = M[$_debugger_is_attached];
   if Z rts;


   // work out the number of arguments that we have, for speed reasons we
   // currently just count the number of %'s in the format string
   r1 = SP - 1;
   r1 = M[r1];
   count_args_loop:
      r2 = M[r1];
      if Z jump end_of_fmt_string;
      Null = r2 - '%';
      if Z r0 = r0 + 1;
      r1 = r1 + 1;
   jump count_args_loop;
   end_of_fmt_string:


   // check that we have space in the debug buffer for num_args (r0) + 3 values
   is_there_enough_space:
      r2 = LENGTH($_stdout_buf);
      r1 = M[$_stdout_read_addr];
      r1 = r1 - M[$_stdout_write_addr];
      if LE r1 = r1 + r2;
      r1 = r1 - 4;
      Null = r1 - r0;
   if LT jump is_there_enough_space;

   // r1 = write address for all code below
   r1 = M[$_stdout_write_addr];


   // move write pointer on to next whole word
   r2 = M[$_stdout_bitpos];
   Null = r2 - 16;
   if Z jump already_at_new_whole_word;
      // set stdout_bitpos to start of word
      r2 = 16;
      M[$_stdout_bitpos] = r2;


      // increment write pointer, and handle buffer wrapping
      r1 = r1 + 1;
      r2 = $_stdout_buf;
      Null = r1 - ($_stdout_buf + LENGTH($_stdout_buf));
      if POS r1 = r2;
   already_at_new_whole_word:


   // form initial word: bit23 set + syncbits + packetlength
   // here the packet length is (num_args + 1)
   r2 = r0 + (0x8aaa00 + 1);
   M[r1] = r2;

   // increment write pointer, and handle buffer wrapping
   r1 = r1 + 1;
   r2 = $_stdout_buf;
   Null = r1 - ($_stdout_buf + LENGTH($_stdout_buf));
   if POS r1 = r2;



   // write the pointer to the printf format string (top of stack)
   r3 = SP - 1;
   r2 = M[r3];
   M[r1] = r2;

   // increment write pointer, and handle buffer wrapping
   r1 = r1 + 1;
   r2 = $_stdout_buf;
   Null = r1 - ($_stdout_buf + LENGTH($_stdout_buf));
   if POS r1 = r2;



   // write all of the arguments
   args_loop:
      Null = r0;
      if Z jump args_done;
      r0 = r0 - 1;

      r3 = r3 - 1;
      r2 = M[r3];
      M[r1] = r2;

      // increment write pointer, and handle buffer wrapping
      r1 = r1 + 1;
      r2 = $_stdout_buf;
      Null = r1 - ($_stdout_buf + LENGTH($_stdout_buf));
      if POS r1 = r2;

   jump args_loop;

   args_done:

   // Write back the write addr
   M[$_stdout_write_addr] = r1;

   rts;
.ENDMODULE;

