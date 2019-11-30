// Copyright (c) 2018 Qualcomm Technologies International, Ltd.
//   %%version
//!
//! \file uint64_divmod16.asm
//!  Assembler functions called from C code
//!
//! \ingroup platform
//!


.MODULE $M.pl_intrinsics_asm;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
   .MINIM;

// unsigned long long uint64_divmod31_opt(unsigned long long numerator,
//                             unsigned long denominator,
//                             unsigned long *remainder);
//
// Divide a 64 bit unsigned number held in r1,r0 by 31 bit unsigned number held
// in r2. Write the remainder to the address given by r3.
//
// On return r1,r0 holds the quotient and the remainder has been written to
// the address pointed to by r3.
//
// We can almost do this in a single instruction except that the divide
// instruction expects a signed input and output but we want unsigned.
//
// This function is for 31 bit denominators so we don't have a problem with the
// sign bit there.
//
// We can avoid problems with the sign bit in the numerator by using rMAC(B)
// for the numerator and zero padding.
//
// The real trick is to make sure the results of individual divisions never
// exceeds 31 bits.
//
// It's possible to do a 32 bit by 31 bit division giving a 32 bit result by
// using a fixup technique.
//
// First you calculate the approximate quotient:
//
//   q' = [ (n/2)/d ] * 2
//
// Then you work out how much was missing by calculating:
//
//   r' = n - q'*d
//
// If r' is less than d then q' and r' are the real quotient and remainder.
// Otherwise, the real quotient is q' + 1 and the real remainder is r' - d.
//
// The divide takes 17 background cycles so we could try and use those cycles
// to do other work. But we don't have much work to do.

$_uint64_divmod31_opt:
    // Comments include intermediate values for:
    //
    //         [------n-------]     [--d---]     [------q-------]   [--r---]
    //         [--nh--][--nl--]     [--d---]     [--qh--][--ql--]   [--r---]
    //         [--r1--][--r0--]     [--r2--]     [--r1--][--r0--]   [M[r3]-]
    //    A) 0xFFFFFFFFFFFFFFFF / 0x00000002 = 0x7FFFFFFFFFFFFFFF 0x00000001
    //    B) 0x000000007FFFFFFE / 0x7FFFFFFF = 0x0000000000000000 0x7FFFFFFE
    //    C) 0xFFFFFFFDFFFFFFFF / 0x7FFFFFFF = 0x00000001FFFFFFFF 0x7FFFFFFE
    pushm <r4>;

    rMAC = r1 LSHIFT 0 (LO);         // = nh at bit position 0
    Div = rMAC / r2;                 // Start calculating nh / d
  
    // The calculation we just started will overflow if r2 is 1 (or zero but
    // that's a different problem). We can use our delay slots to pick this up
    // and handle manually.
  
    rMAC = r2 - 1;
    if LE jump divide_by_1;          // If d = 1 then rMAC = 0 = r
                                     // If d = 0 then rMAC is irrelevant
  
    rMAC0 = r0;                      // = nl at bit position 0 (for LSW step)
    r4 = r0;                         // = nl saved for later  
  
    r1 = DivResult;                  // = qh = nh/d
                                     // eg. A) 0x7FFFFFFF
                                     //     B) 0x00000000
                                     //     C) 0x00000001
    r0 = DivRemainder;               // = rh (less than d)
  
    rMAC12 = r0 (ZP);                // = rhnl (less than d * 2^32)
                                     // eg. A) 0x00000001FFFFFFFF
                                     //     B) 0x00000000FFFFFFFF
                                     //     C) 0x7FFFFFFEFFFFFFFF
  
    rMAC = rMAC LSHIFT -1 (72bit);   // = rhnl/2 (less than d * 2^31)
                                     // eg. A) 0x00000000FFFFFFFF
                                     //     B) 0x000000007FFFFFFF
                                     //     C) 0x3FFFFFFF7FFFFFFF
  
    Div = rMAC / r2;                 // Start calculating rhnl/2 / d
  
    r0 = DivResult;                  // = (rhnl/2)/d (less than 2^31)
                                     // eg. A) 0x00000007FFFFFFF
                                     //     B) 0x000000000000000
                                     //     C) 0x00000007FFFFFFF
  
  
    r0 = r0 LSHIFT 1;                // = ql' = [ (rhnl/2)/d ] * 2
                                     //                  (less than 2^32)
                                     // eg. A) 0x0000000FFFFFFFE
                                     //     B) 0x000000000000000
                                     //     C) 0x0000000FFFFFFFE
    
    // ql' * d is either ql * d or (ql - 1) * d. ql * d is in the range
    // rhnl - d + 1 to rhnl. Hence, ql' * d is in the range rhnl - 2d + 1 to
    // rhnl. In turn, this means that rhnl - ql' * d is in the range 0 to
    // 2d - 1. Since d is less than 2^31 this means that rhnl - ql' * d is
    // less than 2^32. This allows us to do the arithmetic modulo 2^32 and
    // thus ignore rh.
  
    rMAC = r0 * r2 (int);            // = (ql' * d) mod 2^32
                                     // eg.   [--r1--]   [--r0--]   [-rMAC-]
                                     //  A) 0x7FFFFFFF 0x7FFFFFFF 0xFFFFFFFC
                                     //  B) 0x00000000 0x00000000 0x00000000
                                     //  C) 0x00000001 0x00000001 0x00000002
  
    rMAC = r4 - rMAC;                // = r' = rl' = rhnl - ql'*d
                                     // eg.   [--r1--]   [--r0--]   [-rMAC-]
                                     //  A) 0x7FFFFFFF 0xFFFFFFFE 0x00000003
                                     //  B) 0x00000000 0x00000000 0x7FFFFFFE
                                     //  C) 0x00000001 0xFFFFFFFE 0xFFFFFFFD
  
    Null = rMAC - r2;                // rl' < d?
                                     // eg. A) 0x00000003 - 0x00000002
                                     //     B) 0x7FFFFFFE - 0x7FFFFFFF
                                     //     C) 0xFFFFFFFF - 0xFFFFFFFD
    if NC jump low_no_inc;           // if rl' < d then no need to increment
                                     // eg. A) Not taken
                                     //     B) Taken
                                     //     C) Not taken
  
    r0 = r0 + 1;                     // = ql = ql' + 1 (can't overflow)
    rMAC = rMAC - r2;                // = r = rl = rl' - d
                                     // eg.   [--r1--]   [--r0--]   [-rMAC-]
                                     //  A) 0x7FFFFFFF 0xFFFFFFFF 0x00000001
                                     //  B) n/a
                                     //  C) 0x00000001 0xFFFFFFFF 0x7FFFFFFE
  
low_no_inc:
divide_by_1:
  
    Null = r3 - Null;
    if EQ jump dont_write_remainder;
 
    M[r3] = rMAC;                    // = r = rl
  
dont_write_remainder:
    popm <r4>;
    rts;
  
.ENDMODULE;
