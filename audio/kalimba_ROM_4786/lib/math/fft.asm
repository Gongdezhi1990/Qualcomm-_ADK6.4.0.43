// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef MATH_FFT_INCLUDED
#define MATH_FFT_INCLUDED

#include "math_library.h"
#include "core_library.h"

#define FFT_PIPELINED_LAST_STAGES

// *****************************************************************************
// MODULE:
//    $math.fft
//
// DESCRIPTION:
//    An optimised FFT subroutine with a simple interface
//
// INPUTS:
//    - I7 = pointer to fft structure:
//           - $fft.NUM_POINT_FIELD - number of data points
//           - $fft.REAL_FIELD      - ptr to real input data (becomes output data)
//           - $fft.IMAG_FIELD      - ptr to imag input data (becomes output data)
//           - for minimum CPU cycles real and imag buffers must be in different
//             memory banks
//
// OUTPUTS:
//    - The structure pointed to by I7, remains unaltered, but as this is an
//    inplace implementation the input buffers become the output buffers.
//    These output buffers are in bit reversed order
//
// TRASHED REGISTERS:
//    - r0 - r10, DoLoop, I0 - I6, M0 - M3
//
// CPU USAGE:
//    @verbatim
//    No.fft pts:      16    32     64    128     256     512    1024     2048
//
//    No. Clks (BC5): 421   930   2063   4572   10089   22134   48259   104592
//    @endverbatim
//
// *****************************************************************************
.MODULE $M.math.fft;
   .CODESEGMENT MATH_FFT_PM;
   .DATASEGMENT DM;


/*
This is the FFT routine for Kalimba Architecture 3 (found in BC7/BC8, etc)

This section explains how to optimise an algorithm and translate it into optimal Kalimba
assembly instructions, using the FFT as an example.

The FFT routine consists of a series of nested loops:

Stage loop   (i = 0 to log2(n) )
|
|   Group loop    (j = 0 to 2^(i-1) )
|   |
|   |   Butterfly loop   (k = 0 to 2^(log2(n)-i-1) )
|   |   |
|   |
|

The stage loop iterates log2(n) times (e.g. 5 times for a 32-point FFT, since 2^5 = 32).

The group loop has (1 << i) iterations, i.e. 1, 2, 4, ... n/2

The butterfly loop has (1 << (log2(n) - i) ) iterations, i.e. n/2, ..., 4, 2, 1

This means that in the early stages, the butterfly loop iterates many times within each group loop.
Optimisation therefore needs to be targeted at reducing the number of cycles in this loop.
In later stages, the butterfly loop only iterates a small number of times in each group loop, while
the group loop iterates many times in the stage loop.  This means that the latter stages need to be
optimised for the speed of the group loop.  For this purpose the last three stage loops of the FFT
have been replaced with specially written routines.

The operations required in each loop are as follows:

*Stage loop   (i = 0 to log2(n) )
|
|
|
|   Set up next group loop
|
|
|
|   *Group loop    (j = 0 to 2^(i-1) )
|   |
|   |
|   |
|   |   Load next cos(w0) and -sin(w0)
|   |   Set up next butterfly loop
|   |
|   |
|   |
|   |   *Butterfly loop   (k = 0 to 2^(log2(n)-i-1) )
|   |   |
|   |   |   Load next x0, x1, y0, y1
|   |   |
|   |   |   x1' = x0 - x1 cos(w0) - y1 sin(w0)
|   |   |   y0' = y0 + y1 cos(w0) - x1 sin(w0)
|   |   |   y1' = y0 - y1 cos(w0) + x1 sin(w0)
|   |   |   x0' = x0 + x1 cos(w0) + y1 sin(w0)
|   |   |
|   |   |   Store this x0', x1', y0', y1'
|   |   |
|   |
|


The butterfly loop operations can be rewritten as:
|   x_mac = x1 cos(w0) + y1 sin(w0)
|   y_mac = y1 cos(w0) - x1 sin(w0)
|
|   x0' = x0 + x_mac
|   x1' = x0 - x_mac
|   y0' = y0 + y_mac
|   y1' = y0 - y_mac

x_mac and y_mac can be calculated in two stages:
|   x_mac = x1 * cos(w0)
|   x_mac = x_mac - y1 * -sin(w0)
|   y_mac = y1 * cos(w0)
|   y_mac = y_mac + x1 * -sin(w0)

This allows a pseudo-code to be written for the butterfly loop:
|   Load next x0, x1, y0, y1
|
|   x_mac = x1 * cos(w0)
|   x_mac = x_mac - y1 * -sin(w0)
|   y_mac = y1 * cos(w0)
|   y_mac = y_mac + x1 * -sin(w0)
|
|   x0' = x0 + x_mac
|   x1' = x0 - x_mac
|   y0' = y0 + y_mac
|   y1' = y0 - y_mac
|   Store this x0', x1', y0', y1'
|

This loop contains four multiply/accumulate operations, four add/subtract operations, four loads and
four stores.  Many of these operations can be performed at the same time, since Kalimba instrucions can
include a main instruction and two memory accesses.  Indeed, with Kalimba Architecture 3 in some cases
the main instruction can contain two operations.

For example, the following instruction does a multiply-accumulate, adds two values together, reads a
value from a memory location, writes a value to a different memory location, and increments the pointers
to the next appropriate position, while taking only one cycle to execute:

        rMAC = rMAC + r5 * r6,  r0 = r1 + rMACB,   M[I2, M0] = r0,   r1 = M[I4, M1];
                     ^                  ^               ^               ^
                     |                  |               |               |
         Multiply-accumulate            |               |        Memory read and pointer increment
                                        |               |
                                   Addition      Memory write and pointer increment

Using these instructions, it is possible to reduce the 16 separate tasks in the butterfly loop
to four instructions.  For example, the following operations can be combined:
|   Load next x0, x1, y0, y1
|
|   x_mac = x1 * cos(w0)
|   x_mac = x_mac - y1 * -sin(w0)
|   y_mac = y1 * cos(w0)          -------------------->  y_mac = y1 * cos(w0), x0' = x0 + x_mac
|   y_mac = y_mac + x1 * -sin(w0)              /
|                                             /
|   x0' = x0 + x_mac-------------------------/
|   x1' = x0 - x_mac
|   y0' = y0 + y_mac
|   y1' = y0 - y_mac
|   Store this x0', x1', y0', y1'
|

Furthermore, if we prepare for the butterfly loop by pre-calculating the first x_mac value
and loading the first x0, x1, y0 and y1 values, the loop can be further compressed:

|   *Group loop    (j = 0 to 2^(i-1) )
|   |
|   |
|   |
|   |   Load next cos(w0) and -sin(w0)
|   |
|   |   Load first x0, x1, y0, y1
|   |
|   |   Calculate first x_mac:
|   |   x_mac = x1 * cos(w0)
|   |   x_mac = x_mac - y1 * -sin(w0)
|   |
|   |   *Butterfly loop   (k = 0 to 2^(log2(n)-i-1) )
|   |   |
|   |   |   y_mac = y1 * cos(w0)            x0' = x0 + x_mac
|   |   |   y_mac = y_mac + x1 * -sin(w0)   x1' = x0 - x_mac
|   |   |   x_mac = x1 * cos(w0)            y0' = y0 + y_mac
|   |   |   x_mac = x_mac - y1 * -sin(w0)   y1' = y0 - y_mac
|   |   |
|   |
|

Since two memory accesses are allowed per instruction, the loads and the stores can also fit into
these instructions:

|   *Group loop    (j = 0 to 2^(i-1) )
|   |
|   |
|   |
|   |   Load next cos(w0) and -sin(w0)
|   |
|   |   Load first x0, x1, y0, y1
|   |
|   |   Calculate first x_mac and y_mac:
|   |   x_mac = x1 * cos(w0)
|   |   x_mac = x_mac - y1 * -sin(w0)
|   |   y_mac = y1 * cos(w0)            x0' = x0 + x_mac
|   |   y_mac = y_mac + x1 * -sin(w0)   x1' = x0 - x_mac          Load x1   Store x0'
|   |
|   |   *Butterfly loop   (k = 0 to 2^(log2(n)-i-1) )
|   |   |
|   |   |   x_mac = x1 * cos(w0)            y0' = y0 + y_mac      Load y1   Store x1'
|   |   |   x_mac = x_mac - y1 * -sin(w0)   y1' = y0 - y_mac      Load x0   Store y0'
|   |   |   y_mac = y1 * cos(w0)            x0' = x0 + x_mac      Load y0   Store y1'
|   |   |   y_mac = y_mac + x1 * -sin(w0)   x1' = x0 - x_mac      Load x1   Store x0'
|   |   |
|   |
|

This would appear to fit the butterfly loop into four instructions.  However, when the code is run it
is observed that stall cycles occur, so that it takes more than four cycles for the loop to complete.

The reason for this is that memory accesses must obey certain rules.  There is a physical limit preventing the
same memory bank from being accessed by two things at once.  This means, for example, that one instruction cannot
read two values from Data Memory Bank 1 (DM1) without a stall (extra delay) being induced.

The kalimba processor performs reads the cycle before the instruction is executed, and performs writes on the
instruction itself.  This leads to the following memory access possibilities:

Instruction number
1            Read DM1, Read DM1            Stall (not possible to read twice from same memory bank)
2
3            Read DM1, Write DM1           No stalls (the read occurs while instruction 2 is being executed)
4
5            Read DM1, Write DM1           No stalls for these three instructions
6            Read DM2, Write DM2
7            Read DM1, Write DM1
8
9            Read DM2, Write DM1           No stalls for these four instructions
10           Read DM2, Write DM2
11           Read DM1, Write DM2
12           Read DM1, Write DM1
13

With this in mind, it should be arranged that the real input data is in a different memory bank to the imaginary input data.
This allows optimum use to be made of memory bandwidth.

The stalls present in the butterfly loop can now be explained:
|   |   *Butterfly loop   (k = 0 to 2^(log2(n)-i-1) )
|   |   |
|   |   |   x_mac = x1 * cos(w0)            y0' = y0 + y_mac      Load y1   Store x1'      <- No stall
|   |   |   x_mac = x_mac - y1 * -sin(w0)   y1' = y0 - y_mac      Load x0   Store y0'      <- Stall (x0 load after x1' store to same memory bank)
|   |   |   y_mac = y1 * cos(w0)            x0' = x0 + x_mac      Load y0   Store y1'      <- Stall (y0 load after y0' store to same memory bank)
|   |   |   y_mac = y_mac + x1 * -sin(w0)   x1' = x0 - x_mac      Load x1   Store x0'      <- No stall
|   |   |
|   |

A slight rearrangement of the loads can therefore remove the stalls:
|   |   *Butterfly loop   (k = 0 to 2^(log2(n)-i-1) )
|   |   |
|   |   |   x_mac = x1 * cos(w0)            y0' = y0 + y_mac      Load y1   Store x1'      <- No stall
|   |   |   x_mac = x_mac - y1 * -sin(w0)   y1' = y0 - y_mac      Load y0   Store y0'      <- No stall
|   |   |   y_mac = y1 * cos(w0)            x0' = x0 + x_mac      Load x1   Store y1'      <- No stall
|   |   |   y_mac = y_mac + x1 * -sin(w0)   x1' = x0 - x_mac      Load x0   Store x0'      <- No stall
|   |   |
|   |

Note that there are algorithm-dependent restrictions on when values can be loaded.  For example, if the timing of the
loads of y0 and y1 were swapped in the above loop, then different values of y0 would be used in the
calculation of y0' to the calculation of y1', causing the algorithm to fail.

Since there are eight memory operations in each butterfly loop, and eight index registers on a Kalimba, it is possible
to assign each of the four loads and each of the four stores a separate index register to point to the next value to be
loaded/stored.  This would be simple to arrange as the pointers all need to increment by 1 each access.

This would however use up all of the index registers; it would be useful to have some in reserve (for example to load the
sine and cosine coefficients).  There is a close correlation between the read and write pointers (e.g. x0 read and x0' write)
that can be exploited to use only six of the eight index registers inside the butterfly loop.

Making use of this correlation results in a more complicated pattern of memory accesses for those index registers that have
been assigned to two tasks.  For example, if the x1 read and write pointers are reduced to just one pointer, then it must
follow the following pattern:

   (Note that due to the pipelining of reads, y1[1] is read before y1'[0] is written)

   Going from the first read:
   Load  y1 [0]          Index modify +1
   ============ Start of butterfly loop iteration 0 ==============
   Load  y1 [1]          Index modify -1
   Store y1'[0]          Index modify +2
   ============ Start of butterfly loop iteration 1 ==============
   Load  y1 [2]          Index modify -1
   Store y1'[1]          Index modify +2
   ============ Start of butterfly loop iteration 2 ==============
   ...

The FFT butterfly loop is now essentially complete.  It remains to choose which registers are used for each task.
Firstly, there is a limit on which index registers can be used together in one instruction.  This is because
index registers 0 to 3 (I0, I1, I2, I3) work with one address generator (AG1) while index registers 4 to 7 (I4, I5, I6, I7)
work with the other (AG2).

For example, this instruction is valid because AG1 and AG2 are only used once:
      r0 = M[I0,0], r1 = M[I7,1];
While this instruction is invalid (AG1 is used twice):
      r0 = M[I0,0], r1 = M[I1,1];

This means that index registers should be chosen to work around this limit.

Furthermore, there are limits on which general purpose registers can be used to load the values into.  Only rMAC and r0
to r5 can be used for indexed memory loads/stores.
For example, this instruction is valid:
   r3 = M[I3,2], rMAC = M[I4,-1];
While this instruction is invalid as r6 is used:
   r6 = M[I3,2], rMAC = M[I4,-1];

Kalimba offers 56-bit rMAC registers in addition to the normal 24-bit registers.  These are designed to be used for
multiply-accumulate operations, due to their increased precision.  This means that rMAC and rMACB should be used to
hold x_mac and y_mac.

There are further register restrictions concerning the particular multiply-accumulate operations used for this routine,
which is unusual (since almost all Kalimba instructions allow all general-purpose registers to be used).  In this particular case,
the 'Type C multiply-accumulate with additional add/sub' has restrictions on the registers used in the add/sub.  The result
must go into r0, and the source registers must be r1/r2 and rMAC/rMACB.  Additionally, the indexed  memory accesses in this
special variant are limited to using modify registers rather than modify immediates (that is, r0 = M[I0, M1] is allowed but
r0 = M[I0,1] is not allowed because it specifies the modify amount).

These final restrictions mean that the FFT routine is more tightly restricted than most Kalimba routines, which have a larger
set of registers to choose from.

The butterfly loop is now complete (see below for the resulting code).  It uses the do...loop instruction for zero-overhead looping.

The outer loops also make use of optimisations.  For example, at the end of the group loop (after the butterflies have finished),
the index registers need to be set to point to the correct position to load the values for the next set of butterflies.

In this algorithm, this means incrementing the pointers by 2^(log2(n)-i-1).  This value is stored in M2.  One way to do this would
be to have six different instructions of the form:
   I0 = I0 + M2;
However this would take up six instructions.  Instead, the increments could be packed into two instructions by using dummy reads.
Dummy reads are reads where the read is performed, but the value is discarded.  They can be used to change the value of an index register
without using extra cycles.
Using dummy reads, the instruciotns can be packed in as follows:
   I0 = I0 + M2, r0 = M[I1,M2], r0 = M[I4,M2];
   I3 = I3 + M2, r0 = M[I2,M2], r0 = M[I5,M2];

Futhermore, with the new instructions in Kalimba Architecture 3 it is possible to manipulate the stack to store useful values when
there are no more registers left.  In this FFT routine, the stack is used to store pointers to the input arrays, that would otherwise
require extra instructions to load the addresses from the input structure each stage.

The FFT routines operate differently on a BC5 and on a BC7.  This is because Kalimba Architecture 3
introduced new multiply-accumulate instructions and a second accumulator register (rMACB).  This makes
it possible to speed up the butterfly loop by a factor of two, from eight instructions to four.  A BC5
would need 8 instructions in the butterfly loop.


Explanation of register use during the routine:

Register assignment during bfly loop:
Used registers:                                      BC5 routine used:
   r0 = x0', x1', y0', y1'                           r1 , r4
   r1 = x0                                           r0
   r2 = y0                                           r4
   r3 = x1                                           ==
   r4 = -sin(w0)                                     r2
   r5 = y1                                           ==
   r6 = cos(w0)                                      r4
   r7 = next groups (1,2,...,Npts/4,Npts/2)          M3
   r8 = (unused)                                     ==
   r9 = stage_loop counter                           r7
   r10 = bfly_loop counter                           ==
   rLink = group_loop counter (1,2,...,Npts/4,Npts/2) I5

   rMAC = temporary accumulator register
   rMACB = temporary acumulator register

   I0 -> y0 (copy of I6; read pointer)              N/A
   I1 -> x1                                         ==
   I2 -> x0 (write pointer)                         ==   `
   I3 -> cos(w0)                                    I4
   I4 -> x0 (copy of I2; read pointer)              N/A
   I5 -> y1                                         I3
   I6 -> y0 (write pointer)                         ==
   I7 -> -sin(w0)                                   I0

   M0 = 2
   M1 = 1
   M2 = node_space (Npts/2, Npts/4,...,2,1)
   M3 = -1
   NOTE: on arch4, all modifier reg vals are x4

   Stack (relative to SP at entry):
   3 - Pointer to imaginary array
   2 - Pointer to real array
   1 - I7 (FFT struc pointer)
   0 - rLink (return address)

*/


   $math.fft:

   push rLink,                      // rLink is used as a loop counter in this routine
      r0 = M[I7,MK1];    // r0 = $fft.NUM_POINTS
                                    // Note that I7 will be incremented before it is pushed to the stack
   push I7,
      r4 = M[I7,MK1];    // r4 = $fft.IMAG_ADDR

   M1 = MK1;
   M3 = -MK1;
   M0 = M1 + M1;

   // -- process the n-1 stages of butterflies --
   r9 = r0 ASHIFT -2;                   // r9 = node_space/2 = Npts / 4
   r7 = 1;                             // Initial groups = 1

   push r4, r4 = M[I7,-MK1]; // r4 = $fft.REAL_ADDR
   push r4;

   stage_loop:
   #ifdef FFT_LOW_RAM
      r0 = M[$fft.twiddle_real_address];
      I3 = r0;
      r0 = M[$fft.twiddle_imag_address];
      I7 = r0;
   #else
      I3 = $fft.twiddle_real; // I3 -> C of W0
      I7 = $fft.twiddle_imag; // I7 -> (-S) of W0
   #endif

      Words2Addr(r9);         // on arch4, r9 in words but M2 needs addr
      M2 = r9 + r9;           // M2 = node_space (on arch4, M2 = node_space*4)
      Addr2Words(r9);         // on arch4, restore r9 to words

      rLink = r7,             // rLink = groups (loop counter)
         r3 = M[I3,M1];       // Load cos(w0)

      // on arch4, load rLink with # groups*4, and decrement by 4 (M1)
      Words2Addr(rLink);

      r7 = r7 + r7,           // groups = groups * 2 (for next time);
         r4 = M[I7,M1];       // r4 = -sin(w0) (load -sin(w0))
      r6 = r3;                // Copy across cos(w0)

      I2 = M[SP - 2*ADDR_PER_WORD]; // I2 -> x0 in 1st group of stage
      I4 = I2;                       // I4 -> x0 in 1st group of stage

      I6 = M[SP - ADDR_PER_WORD];   // I6 -> y0 in 1st group of stage

      I1 = I2 + M2;           // I1 -> x1 in 1st group of stage


      I0 = I6;                // I0 -> y0 in 1st group of stage
      I5 = I6 + M2;           // I5 -> y1 in 1st group of stage

      group_loop:
         r10 = r9 + r9,       // r10 = node_space
            r3 = M[I1,M1];    // r3 read from I1 is a real read of x1

            // Load appropriate variables (including sin/cos) and start calculations
            // before bfly loop, will have loaded x0 [0], -sin(w0), cos(w0), x1 [0], x1 [1], y1 [0], y0 [0]

         rMACB = r3 * r6, r0 = r2 + rMAC, r5 = M[I5,M1];

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1], r1 = M[I4, M1];

         rMAC = r3 * r4, r0 = r1 + rMACB    , r3 = M[I1,M3];

         do bfly_loop;
               // 1 ======================================================
               // Store x0', Load x0 (2nd), r0 = x1' [= x0 - x1 cos(w0) - y1 sin(w0) ], rMAC += y1 cos(w0) (for y0' and y1')
               //      I2,1    I4,1
               //      DM1     DM1
            rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M1];
               // 2 ======================================================
               // Store x1', Load y1 , r0 = y0' [= y0 + y1 cos(w0) - x1 sin(w0) ], rMACB = x1(2) cos(w0) (for x0' and x1')
               //      I1,2    I5,-1
               //      DM1     DM2
            rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5, M3];
               // 3 ======================================================
               // Store y0', Load y0, r0 = y1' [= y0 - y1 cos(w0) + x1 sin(w0) ], rMACB -= -y1 sin(w0) (for x0' and x1')
               //      I6,1    I0,1
               //      DM2     DM2
            rMACB = rMACB - r4 * r5, r0 = r2 - rMAC, r2 = M[I0, M1], M[I6, M1] = r0;
               // 4 ======================================================
               // Store y1', Load x1, r0 = x0' [= x0 + x1 cos(w0) + y1 sin(w0) ], rMAC = -x1(1) sin(w0) (for y0' and y1')
               //      I5,2    I1,-1
               //      DM2     DM1
            rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M0] = r0;
         bfly_loop:

         // Put the index registers in the correct position for the next loop (add M2 to I0, I1, I2, I4, I5, I6)
         // Use adds and dummy reads
         I5 = I5 + M2, r0 = M[I4, M2];  // No DM2 read allowed this instruction (would be stall-inducing)
         I0 = I0 + M2, r0 = M[I1,M2], r0 = M[I6, M2];

         // I0, I1, I4, I5 need to be decremented in order to put them back to the correct positions
         I5 = I5 + M3,  r0 = M[I3,M1], r4 = M[I7,M1];             // r4 = -sin(w0) (load -sin(w0))
         r6 = r0, r0 = M[I2,M2];                                  // Load cos(w0) for next iterations, increment

         // Decrement the group loop counter (rLink)
         rLink = rLink - M1,
            r0 = M[I0, M3],
            r3 = M[I4, M3];     // r3 read from I4 is a dummy read

      if NZ jump group_loop;

      r9 = r9 ASHIFT -1;        // node_space = node_space / 2;
      #ifdef FFT_PIPELINED_LAST_STAGES
         Null = r9 ASHIFT -2;   // Check if time to move on to penultimate stage
      #endif
   if NZ jump stage_loop;

   #ifdef FFT_PIPELINED_LAST_STAGES
      jump $math.fft_penpenultimate_stage;
   #else
      jump $math.fft_last_stage;
   #endif

.ENDMODULE;


#ifdef FFT_PIPELINED_LAST_STAGES

// $****************************************************************************
// PRIVATE FUNCTION:
//    $fft_penpenultimate_stage
//
// DESCRIPTION:
//    Carries out the pen-penultimate stage of the fft
//
// *****************************************************************************
.MODULE $M.math.fft_penpenultimate_stage;
   .CODESEGMENT MATH_FFT_PENPENULTIMATE_STAGE_PM;
   .DATASEGMENT DM;

   $math.fft_penpenultimate_stage:

/*    This stage performs the same operations as the normal FFT loops, but
      uses extra pipelining to reduce the number of cycles */

   #ifdef FFT_LOW_RAM
      r0 = M[$fft.twiddle_real_address];
      I3 = r0;
      r0 = M[$fft.twiddle_imag_address];
      I7 = r0;
   #else
      I3 = $fft.twiddle_real; // I3 -> C of W0
      I7 = $fft.twiddle_imag; // I7 -> (-S) of W0
   #endif

      r10 = r7 - 1,                 // r10 = groups (loop counter) -1
         r3 = M[I3,M1];             // Load cos(w0)

      r7 = r7 + r7,                 // groups = groups * 2 (for next time);
            r4 = M[I7,M1];          // r4 = -sin(w0) (load -sin(w0))
      r6 = r3;                      // Copy across cos(w0)


      I2 = M[SP - 2*ADDR_PER_WORD];   // I2 -> x0 in 1st group of stage
      I4 = I2;                      // I4 -> x0 in 1st group of stage
      I6 = M[SP - ADDR_PER_WORD];   // I6 -> y0 in 1st group of stage
      I1 = I2 + 4*ADDR_PER_WORD;    // I1 -> x1 in 1st group of stage
      I0 = I6;                      // I0 -> y0 in 1st group of stage
      I5 = I6 + 4*ADDR_PER_WORD;    // I5 -> y1 in 1st group of stage

      // Original values of M registers:
      // M0 = 2
      // M1 = 1
      // M2 = 2
      // M3 = -1
      // NOTE: on arch4, all modifier reg vals are x4

      // Ramp up to loop
      M2 = M1 + M1, r3 = M[I1,M1]; // I1 read +1
      M0 = 6*MK1;
      //M3 = -1; // Already set to this

      rMACB = r3 * r6, r0 = r2 + rMAC, r5 = M[I5,M1]; // I5 read +1

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1], r1 = M[I4, M1]; // I0 read +1, I4 read +1

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3]; // I1 read -1

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M1]; // I2 write +1, I4 read +1

      // Second set of four

      rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M2] = r0, r5 = M[I5,M3]; // I1 write + 2, I5 read -1

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1],  M[I6, M1] = r0; // I0 read + 1, I6 write +1

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M2] = r0; // I1 read -1 , I5 write + 2

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M1];// I2 write + 1, I4 read + 1

      // Third set of four

      rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M2] = r0, r5 = M[I5,M3]; // I1 write + 2, I5 read -1

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1],  M[I6, M1] = r0; // I0 read + 1, I6 write +1

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M2] = r0; // I1 read -1 , I5 write + 2

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M0];// I2 write + 1, I4 read + 5

      // Fourth set of 4

      rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M3]; // I1 write +6, I5 read -1

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M0],  M[I6, M1] = r0; // I0 read +5, I6 write +1

      // Set back I0 and I4 to the correct locn

      I0 = I0 + M3,  rMAC = M[I4, M3]; // Do a dummy read with I4 (DM1 so no stall)


      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M0] = r0; /* r3 = temp store of cos */    // I3 read +1, I5 write +6

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M0] = r0, r1 = M[I4, M1]; // I2 write +5, I4 read +1

      // Set back I2 to the correct locn

      r6 = r3, r3 = M[I1,M1]; /* copy across cos */ // I1 read -5

      I1 = I1 - M0, // (To make the I1 -5 store happen)
         r5 = M[I2,M3], // Dummy read to decrement I2
         r4 = M[I7,M1];

      do penpenultimate_loop;

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M1]; // I1 write + 6, I5 read -5


         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1],  M[I6, M0] = r0; // I0 read + 1, I6 write +5

         // Set back I6 to the correct locn with dummy read*** // Stall because just wrote to I6

         I5 = I5 - M0, rMAC = M[I6,M3];  // Set back I5 to the correct locn ***

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M0] = r0; // I1 read -1 , I5 write + 6

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M1];// I2 write + 1, I4 read + 1

         // Second set of four

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M2] = r0, r5 = M[I5,M3]; // I1 write + 2, I5 read -1

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1],  M[I6, M1] = r0; // I0 read + 1, I6 write +1

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M2] = r0; // I1 read -1 , I5 write + 2

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M1];// I2 write + 1, I4 read + 1

         // Third set of four

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M2] = r0, r5 = M[I5,M3]; // I1 write + 2, I5 read -1

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1],  M[I6, M1] = r0; // I0 read + 1, I6 write +1

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M2] = r0; // I1 read -1 , I5 write + 2

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M0];// I2 write + 1, I4 read + 5

         // Fourth set of 4

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M3]; // I5 read -1, I1 write +6

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M0],  M[I6, M1] = r0; // I6 write +1, I0 read + 5

         // Set back I0, I4 to the correct locn ***

         I0 = I0 + M3,  rMAC = M[I4, M3]; // Do a dummy read with I4 (DM1 so no stall)

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M0] = r0; /* r3 = temp store of cos */ // I5 write +6

         // Can put I7 read here

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M0] = r0, r1 = M[I4, M1]; // I4 read +1, I2 write + 5

         // Stall here for same reason as in penultimate loop

         r6 = r3, r3 = M[I1,M1]; /* copy across cos */ // I1 read -5

         // Set back I2 to the correct locn

         I1 = I1 - M0, r5 = M[I2, M3],   r4 = M[I7,M1];

      penpenultimate_loop:

      rMACB = rMACB * Null, r0 = r2 + rMAC, M[I1, M1] = r0, r5 = M[I5,M1]; // (like instruction 1) (including I5 dummy read of -5)

      I5 = I5 - M0; // Set back I5 to the correct locn ***

      rMACB = rMACB * Null, r0 = r2 - rMAC,  M[I6, M2] = r0; // (like instruction 2)

      M[I5, M1] = r0;

      jump $math.fft_penultimate_stage;

.ENDMODULE;

// $****************************************************************************
// PRIVATE FUNCTION:
//    $fft_penultimate_stage
//
// DESCRIPTION:
//    Carries out the penultimate stage of the fft
//
// *****************************************************************************
.MODULE $M.math.fft_penultimate_stage;
   .CODESEGMENT MATH_FFT_PENULTIMATE_STAGE_PM;
   .DATASEGMENT DM;

   $math.fft_penultimate_stage:

   #ifdef FFT_LOW_RAM
      r0 = M[$fft.twiddle_real_address];
      I3 = r0;
      r0 = M[$fft.twiddle_imag_address];
      I7 = r0;
   #else
      I3 = $fft.twiddle_real; // I3 -> C of W0
      I7 = $fft.twiddle_imag; // I7 -> (-S) of W0
   #endif

      r10 = r7 - 1,          // r10 = groups (loop counter) -1
         r3 = M[I3,M1];            // Load cos(w0)

      r7 = r7 + r7,          // groups = groups * 2 (for next time);
         r4 = M[I7,M1];             // r4 = -sin(w0) (load -sin(w0))
      r6 = r3;                    // Copy across cos(w0)


      I2 = M[SP - 2*ADDR_PER_WORD]; // I2 -> x0 in 1st group of stage
      I4 = I2;                      // I4 -> x0 in 1st group of stage
      I6 = M[SP - ADDR_PER_WORD];   // I6 -> y0 in 1st group of stage
      I1 = I2 + 2*ADDR_PER_WORD;    // I1 -> x1 in 1st group of stage
      I0 = I6;                      // I0 -> y0 in 1st group of stage
      I5 = I6 + 2*ADDR_PER_WORD;    // I5 -> y1 in 1st group of stage


      // Original values of M registers:
      // M0 = 2
      // M1 = 1
      // M2 = 2
      // M3 = -1
      // NOTE: on arch4, all modifier reg vals are x4
      // Ramp up to loop


      M2 = 3*MK1;   // = +3 now value of M2

      M0 = M2 + M1,  r3 = M[I1,M1]; // I1 read +1
      // M3 = -1; // Already set to this

      rMACB = r3 * r6, r0 = r2 + rMAC, r5 = M[I5,M1]; // I5 read +1

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1], r1 = M[I4, M1]; // I0 read +1, I4 read +1

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3]; // I1 read -1

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M2]; // I2 write +1, I4 read +3

      // Second set of 4

      rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M3]; // I1 write +4, I5 read -1

      // I1 = I1 + 1; // So that I1 write +4 can happen (optimise with a dummy read?)

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M2],  M[I6, M1] = r0; // I0 read +3, I6 write +1

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M0] = r0; /* r3 = temp store of cos */    // I3 read +1, I5 write +4

      // I5 = I5 + 1; // So that I5 write +4 can happen (optimise with a dummy read?)

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M2] = r0, r1 = M[I4, M1]; // I2 write +3, I4 read +1

      r6 = r3, r3 = M[I1,M1]; /* copy across cos */ // I1 read -3

      I1 = I1 - M0, // (To make the I1 -3 store happen)
        r4 = M[I7,M1];


      // M3 now superfluous

      do penultimate_loop;

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M1]; // I1 write + 4, I5 read -3

         I5 = I5 - M0;  // Lots of space to do something useful...

         // Could put r4 = I7 read here (then couple with a dummy decrement of I5, I1 by 2)

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1],  M[I6, M2] = r0; // I0 read + 1, I6 write +3

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M0] = r0; // I1 read -1 , I5 write + 4

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M2];// I2 write + 1, I4 read + 3

         // Second set of 4

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M3]; // I5 read -1, I1 write +4

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M2],  M[I6, M1] = r0; // I6 write +1, I0 read + 3

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M0] = r0; /* r3 = temp store of cos */ // I5 write +4

         // Can put I7 read here

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M2] = r0, r1 = M[I4, M1]; // I4 read +1, I2 write + 3

         // Stall here (due to having to read I1 (DM1) after a write to I2 (DM1)
         // But need to increment I1 after the read not before, so can't rearrange to remove the stall

         r6 = r3, r3 = M[I1,M1]; /* copy across cos */

         I1 = I1 - M0, r4 = M[I7,M1];      // I1 read -3

      penultimate_loop:

      rMACB = rMACB * Null, r0 = r2 + rMAC, M[I1, M1] = r0, r5 = M[I5,M3]; // (like instruction 1) (including I5 dummy read of -1 turned to -3)

      I5 = I5 - 2*ADDR_PER_WORD;

      rMACB = rMACB * Null, r0 = r2 - rMAC,  M[I6, M2] = r0; // (like instruction 2)

      M[I5, M1] = r0;

      jump $math.fft_last_stage;

.ENDMODULE;

#endif //FFT_PIPELINED_LAST_STAGES

// $****************************************************************************
// PRIVATE FUNCTION:
//    $fft_last_stage
//
// DESCRIPTION:
//    Carries out the last stage of the fft
//
// *****************************************************************************
.MODULE $M.math.fft_last_stage;
   .CODESEGMENT MATH_FFT_LAST_STAGE_PM;
   .DATASEGMENT DM;

   $math.fft_last_stage:
   #ifdef FFT_LOW_RAM
      r0 = M[$fft.twiddle_real_address];
      I3 = r0;
      r0 = M[$fft.twiddle_imag_address];
      I7 = r0;
   #else
      I3 = $fft.twiddle_real; // I3 -> C of W0
      I7 = $fft.twiddle_imag; // I7 -> (-S) of W0
   #endif

      r10 = r7 - 1,               // r10 = groups - 1 (loop counter)
         r3 = M[I3,M1];            // Load cos(w0)

      r6 = r3;                     // Copy across cos(w0)

      I2 = M[SP - 2*ADDR_PER_WORD]; // I2 -> x0 in 1st group of stage
      I4 = I2;                       // I4 -> x0 in 1st group of stage

      I6 = M[SP - ADDR_PER_WORD];   // I6 -> y0 in 1st group of stage

      I1 = I2 + ADDR_PER_WORD;      // I1 -> x1 in 1st group of stage


      I0 = I6;                     // I0 -> y0 in 1st group of stage
      I5 = I6 + ADDR_PER_WORD;    // I5 -> y1 in 1st group of stage

      M3 = M3 + M3,                //  -1 to -2
         r4 = M[I7,MK1];

      M0 = 2*MK1;

      // Ramp up to loop

      M2 = M0 - M3,   // = +4
         r3 = M[I1,M0];

      rMACB = r3 * r6, r0 = r2 + rMAC, r5 = M[I5,M0];

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M0], r1 = M[I4, M0];

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1]; // r3 used to load next cos

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M0] = r0, r4 = M[I7,M1];

      r6 = r3,
         r3 = M[I1,M3]; // I2, I1 both in DM1 (stall)

      r1 = M[I4, M0];

      do last_loop;

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M2] = r0, r5 = M[I5,M3]; /* Minus 2 from I5 */

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M0],  M[I6, M0] = r0;

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M2] = r0; /* r3 = temp store of cos */ // I3, I6 both in DM2 (stall)

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M0] = r0, r1 = M[I4, M0]; // I5, I5 both in DM2 (stall)

         r4 = M[I7,MK1];

         r6 = r3, r3 = M[I1,M3]; /* copy across cos */

      last_loop:

      SP = SP - 2*ADDR_PER_WORD, popm <I7>;

      rMACB = rMACB * Null, r0 = r2 + rMAC, M[I1, M1] = r0, r5 = M[I5,M3]; // (like instruction 1) (including I5 dummy read)

      rMACB = rMACB * Null, r0 = r2 - rMAC,  M[I6, M0] = r0; // (like instruction 2)

      pop rLink, rMAC = M[I7,-MK1]; // Decrement I7 back to where it was when the function was called

      M[I5, M1] = r0; // Write last sample during what is otherwise a stall cycle

      rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $math.ifft
//
// DESCRIPTION:
//    An optimised IFFT subroutine with a simple interface
//
// INPUTS:
//    - I7 = pointer to fft structure:
//           - $fft.NUM_POINT_FIELD - number of data points
//           - $fft.REAL_FIELD      - ptr to real input data (becomes output data)
//           - $fft.IMAG_FIELD      - ptr to imag input data (becomes output data)
//           - for minimum CPU cycles real and imag buffers must be in different
//             memory banks
//
// OUTPUTS:
//    - The structure pointed to by I7, remains unaltered, but as this is an
//    inplace implementation the input buffers become the output buffers.
//    These output buffers are in bit reversed order
//
// TRASHED REGISTERS:
//    - r0 - r10, DoLoop, I0 - I6, M0 - M3
//
// CPU USAGE:
//    @verbatim
//    No.fft pts:      16    32     64    128     256     512    1024     2048
//
//    No. Clks (BC5): 428   945   2094   4635   10216   22389   48770   105615
//    @endverbatim
//
// *****************************************************************************
// *****************************************************************************
// MODULE:
//    $math.scaleable_fft
//
// DESCRIPTION:
//    An optimised scaleable FFT subroutine with a simple interface
//
// INPUTS:
//    - I7 = pointer to fft structure:
//           - $fft.NUM_POINT_FIELD - number of data points
//           - $fft.REAL_FIELD      - ptr to real input data (becomes output data)
//           - $fft.IMAG_FIELD      - ptr to imag input data (becomes output data)
//           - for minimum CPU cycles real and imag buffers must be in different
//             memory banks
//    - r8 = scaling value (1.0 for fft is standard, 0.5 for ifft is standard)
//           - This scaling factor is applied per fft stage.
//           - Hence for a 2^5 = 32 point fft, having a scaling factor of 0.8
//             will produce an fft output that is scaled by 0.8^5 = 0.32768
//
// OUTPUTS:
//    - The structure pointed to by I7, remains unaltered, but as this is an
//    inplace implementation the input buffers become the output buffers.
//    These output buffers are in bit reversed order
//
// TRASHED REGISTERS:
//    r1 - r10, I0 - I6, M0 - M3
//
// CPU USAGE:
//    !!!! NEEDS UPDATING !!!!!
//    @verbatim
//    No.fft pts:      16     32     64    128     256     512    1024     2048
//
//    No. Clks (BC5): 474   1071   2412   5401   12006   26483   57984   126093
//    @endverbatim
//
// *****************************************************************************
// *****************************************************************************
// MODULE:
//    $math.scaleable_ifft
//
// DESCRIPTION:
//    An optimised scaleable IFFT subroutine with a simple interface
//
// INPUTS:
//    - I7 = pointer to fft structure:
//           - $fft.NUM_POINT_FIELD - number of data points
//           - $fft.REAL_FIELD      - ptr to real input data (becomes output data)
//           - $fft.IMAG_FIELD      - ptr to imag input data (becomes output data)
//    - r8 = scaling value (1.0 for fft is standard, 0.5 for ifft is standard)
//
// OUTPUTS:
//    - The structure pointed to by I7, remains unaltered, but as this is an
//    inplace implementation the input buffers become the output buffers.
//    These output buffers are in bit reversed order
//
// TRASHED REGISTERS:
//    r1 - r10, I0 - I6, M0 - M3
//
// CPU USAGE:
//    !!!! NEEDS UPDATING !!!!!
//    @verbatim
//    No.fft pts:      16     32     64    128     256     512    1024     2048
//
//    No. Clks (BC5): 474   1071   2412   5401   12006   26483   57984   126093
//    @endverbatim
//
// *****************************************************************************

.MODULE $M.math.ifft;
   .CODESEGMENT MATH_IFFT_PM;
   .DATASEGMENT DM;

/*
This is the IFFT / scaleable FFT routine for Kalimba Architecture 3 (found in BC7/BC8, etc)
These are all scaled FFTs with different scaling factors, and hence use the same routine.

Explanation of register use during the routine:

Register assignment during bfly loop:
Used registers:                                      BC5 routine used:
   r0 = x0', x1', y0', y1'                           r1 , r4
   r1 = x0                                           r0
   r2 = y0                                           r4
   r3 = x1                                           ==
   r4 = -sin(w0)                                     r2
   r5 = y1                                           ==
   r6 = cos(w0)                                      r4
   r7 = Scale factor                                 (varies)
   r8 = Scale factor                                 ==
   r9 = stage_loop counter                           (varies)
   r10 = bfly_loop counter                           ==
   rLink = group_loop counter (1,2,...,Npts/4,Npts/2) I5

   rMAC = temporary accumulator register
   rMACB = temporary acumulator register

   I0 -> y0 (copy of I6; read pointer)              N/A
   I1 -> x1                                         ==
   I2 -> x0 (write pointer)                         ==   `
   I3 -> cos(w0)                                    I4
   I4 -> x0 (copy of I2; read pointer)              N/A
   I5 -> y1                                         I3
   I6 -> y0 (write pointer)                         ==
   I7 -> -sin(w0)                                   I0

   M0 = 2
   M1 = 1
   M2 = node_space (Npts/2, Npts/4,...,2,1)
   M3 = -1
   NOTE: on arch4, all modifier reg vals are x4

   Stack (relative to SP at entry):
   3 - Pointer to imaginary array
   2 - Pointer to real array
   1 - I7 (FFT struc pointer)
   0 - rLink (return address)

   $fft.next_groups = next groups (1,2,...,Npts/4,Npts/2)  M3

*/

   .var/DM $fft.next_groups = 1;    // Variable won't get created if ifft / scaled fft not called

   $math.ifft:
   // set up as an IFFT
   r8 = 0.5;
   jump $math.scaleable_ifft;

   $math.scaleable_fft:

   // set up as an FFT
   r7 = r8;
   jump start;

   $math.scaleable_ifft:

   // set up as an IFFT
   r7 = -r8;

   start:

   // rLink is used as a loop counter in this routine
   push rLink,
      r0 = M[I7,MK1]; // r0 = $fft.NUM_POINTS
                                 // Note that I7 will be incremented before it is pushed to the stack
   push I7,
      r4 = M[I7,MK1]; // r4 = $fft.IMAG_ADDR

   M1 = MK1;
   M3 = -MK1;
   M0 = M1 + M1;

   // -- process the n-1 stages of butterflies --
   r9 = r0 ASHIFT -2;                      // r9 = node_space/2 = Npts / 4
   r0 = 1;                                 // Initial groups = 1
   M[$fft.next_groups] = r0;
   rLink = Null;                           // Zero loop counter

   push r4, r4 = M[I7,-MK1];    // r4 = $fft.REAL_ADDR
   push r4;

   stage_loop:
      I1 = $fft.next_groups;
   #ifdef FFT_LOW_RAM
      r0 = M[$fft.twiddle_real_address];
      I3 = r0;
      r0 = M[$fft.twiddle_imag_address];
      I7 = r0;
   #else
      I3 = $fft.twiddle_real; // I3 -> C of W0
      I7 = $fft.twiddle_imag; // I7 -> (-S) of W0
   #endif
      Words2Addr(r9);         // on arch4, r9 in words but M2 needs addr
      M2 = r9 + r9,           // M2 = node_space (on arch4, M2 = node_space*4)
         r0 = M[I1,M1];       // load groups
      Addr2Words(r9);         // on arch4, restore r9 to words

      rLink = rLink + r0,     // rLink = groups (loop counter)
         r3 = M[I3,M1],       // Load cos(w0)
         r4 = M[I7,M1];       // r4 = -sin(w0) (load -sin(w0))

      // on arch4, load rLink with # groups*4, and decrement by 4 (M1)
      Words2Addr(rLink);

      r0 = r0 + r0,           // groups = groups * 2 (for next time);
         r0 = M[I1,M3];       // dummy decrement of I1 for groups (r0 won't get overwritten)

      r6 = r3 * r8 (frac),    // Copy across cos(w0)
         M[I1,M1] = r0;       // store next groups

      r4 = r4 * r7 (frac);    // Multiply -sin(w0) by the appropriate IFFT factor

      I2 = M[SP - 2*ADDR_PER_WORD]; // I2 -> x0 in 1st group of stage
      I4 = I2;                       // I4 -> x0 in 1st group of stage


      I6 = M[SP - ADDR_PER_WORD];   // I6 -> y0 in 1st group of stage

      I1 = I2 + M2;             // I1 -> x1 in 1st group of stage

      I0 = I6 + ADDR_PER_WORD; // I0 -> y0 in 1st group of stage
                                // (+ 1 is immediately removed to give better cycle usage)
      I5 = I6 + M2;             // I5 -> y1 in 1st group of stage

      group_loop:
         r10 = r9 + r9,      // r10 = node_space
            r3 = M[I1,M1];   // r3 read from I1 is a real read of x1

         // Load appropriate variables (including sin/cos) and start calculations
         // before bfly loop, will have loaded x0 _0_, -sin(w0), cos(w0), x1 _0_, x1 _1_, y1 _0_, y0 _0_

         rMACB = r3 * r6, r0 = r2 + rMAC, r5 = M[I5,M1];

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r0 = M[I0, M3], r1 = M[I4, M1]; // I0 dummy read

         r1 = r1 * r8 (frac), r2 = M[I0, M1]; // Scale x0 by r8 (This loses a bit of precision)
         r2 = r2 * r8 (frac);                 // Scale y0 by r8 (This loses a bit of precision)

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3];

         do bfly_loop;

                  // 1 ======================================================
                  // Store x0', Load x0 (2nd), r0 = x1' [= x0 - x1 cos(w0) - y1 sin(w0) ], rMAC += y1 cos(w0) (for y0' and y1')
                  //      I2,1    I4,1
                  //      DM1     DM1
            rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M1];
                  // 2 ======================================================
                  // Store x1', Load y1 , r0 = y0' [= y0 + y1 cos(w0) - x1 sin(w0) ], rMACB = x1(2) cos(w0) (for x0' and x1')
                  //      I1,2    I5,-1
                  //      DM1     DM2
            rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5, M3];
                  // 3 ======================================================
                  // Store y0', Load y0, r0 = y1' [= y0 - y1 cos(w0) + x1 sin(w0) ], rMACB -= -y1 sin(w0) (for x0' and x1')
                  //      I6,1    I0,1
                  //      DM2     DM2
            rMACB = rMACB - r4 * r5, r0 = r2 - rMAC, r2 = M[I0, M1], M[I6, M1] = r0;

            r1 = r1 * r8 (frac); // Scale x0 by r8 (This loses a bit of precision)
            r2 = r2 * r8 (frac); // Scale y0 by r8 (This loses a bit of precision)

                  // 4 ======================================================
                  // Store y1', Load x1, r0 = x0' [= x0 + x1 cos(w0) + y1 sin(w0) ], rMAC = -x1(1) sin(w0) (for y0' and y1')
                  //      I5,2    I1,-1
                  //      DM2     DM1
            rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M0] = r0;

         bfly_loop:

         // Load cos (w0) and x1 for the next iteration

         // Do things to the index registers (add M2 to I0, I1, I2, I4, I5, I6)
         // Use adds and dummy reads
         I5 = I5 + M2,  r0 = M[I4, M2];               // dummy read  - NB Can't read from DM2 in this instruction

         // I0, I1, I4, I5 need to be decremented in order to put them back to the correct positions
         I5 = I5 + M3,  r0 = M[I3,M1], r4 = M[I7,M1]; // r4 = -sin(w0) (load -sin(w0))
         r6 = r0 * r8 (frac), r0 = M[I2,M2];          // Load cos(w0) for next iterations, increment

         r4 = r4 * r7 (frac),                         // Scale -sin(w0)
            r0 = M[I1,M2], r0 = M[I6, M2];            // dummy reads

         // Decrement the group loop counter (rLink)
         rLink = rLink - M1,
            r0 = M[I0, M2],
            r3 = M[I4, M3];     // r3 read from I4 is a dummy read

      if NZ jump group_loop;

      r9 = r9 ASHIFT -1;        // node_space = node_space / 2;

      #ifdef FFT_PIPELINED_LAST_STAGES
         // Temporary way to check if time to move on to penultimate stage (need to adjust all the loops properly)
         Null = r9 ASHIFT -1;
      #endif

   if NZ jump stage_loop;

   #ifdef FFT_PIPELINED_LAST_STAGES
      jump $math.scaleable_fft_penultimate_stage;
   #else
      jump $math.scaleable_fft_last_stage;
   #endif
.ENDMODULE;

#ifdef FFT_PIPELINED_LAST_STAGES

// $****************************************************************************
// PRIVATE FUNCTION:
//    $scaleable_fft_penultimate_stage
//
// DESCRIPTION:
//    Carries out the penultimate stage of the scaleable fft or ifft
//
// *****************************************************************************
.MODULE $M.math.scaleable_fft_penultimate_stage;
   .CODESEGMENT MATH_SCALEABLE_FFT_PENULTIMATE_STAGE_PM;
   .DATASEGMENT DM;

   $math.scaleable_fft_penultimate_stage:

      I1 = $fft.next_groups;
   #ifdef FFT_LOW_RAM
      r0 = M[$fft.twiddle_real_address];
      I3 = r0;
      r0 = M[$fft.twiddle_imag_address];
      I7 = r0;
   #else
      I3 = $fft.twiddle_real; // I3 -> C of W0
      I7 = $fft.twiddle_imag; // I7 -> (-S) of W0
   #endif

      Words2Addr(r9);         // on arch4, r9 in words but M2 needs addr
      M2 = r9 + r9,              // M2 = node_space (on arch4, M2 = node_space*4)
         r0 = M[I1,M1];          // load groups
      Addr2Words(r9);         // on arch4, restore r9 to words

      r10 = r0 - 1,              // r10 = groups (loop counter) -1
         r3 = M[I3,M1];          // Load cos(w0)

      r0 = r0 + r0,              // groups = groups * 2 (for next time);
         r0 = M[I1,M3],          // dummy decrement of I1 for groups (r0 won't get overwritten)
         r4 = M[I7,M1];          // r4 = -sin(w0) (load -sin(w0))

      r6 = r3 * r8 (frac),       // Copy across cos(w0)
         M[I1,M1] = r0;          // store next groups

      I2 = M[SP - 2*ADDR_PER_WORD]; // I2 -> x0 in 1st group of stage
      I4 = I2;                       // I4 -> x0 in 1st group of stage
      I6 = M[SP - ADDR_PER_WORD];   // I6 -> y0 in 1st group of stage
      I1 = I2 + 2*ADDR_PER_WORD;    // I1 -> x1 in 1st group of stage
      I0 = I6;                       // I0 -> y0 in 1st group of stage
      I5 = I6 + 2*ADDR_PER_WORD;    // I5 -> y1 in 1st group of stage

      // Original values of M registers:
      // M0 = 2
      // M1 = 1
      // M2 = 2
      // M3 = -1
      // NOTE: on arch4, all modifier regs are x4
      // Ramp up to loop

      M2 = 3*MK1;                    // = +3 now value of M2
      M0 = M2 + M1,
         r3 = M[I1,M1];          // I1 read +1
      // M3 = -1;                // Already set to this

      rMACB = r3 * r6, r0 = r2 + rMAC, r5 = M[I5,M1]; // I5 read +1

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1], r1 = M[I4, M1]; // I0 read +1, I4 read +1

      r1 = r1 * r8 (frac); // Scale x0 by r8 (This loses a bit of precision)
      r2 = r2 * r8 (frac); // Scale y0 by r8 (This loses a bit of precision)

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3]; // I1 read -1

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M2]; // I2 write +1, I4 read +3

      // Second set of 4

      rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M3]; // I1 write +4, I5 read -1

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M2],  M[I6, M1] = r0; // I0 read +3, I6 write +1

      r1 = r1 * r8 (frac); // Scale x0 by r8 (This loses a bit of precision)
      r2 = r2 * r8 (frac); // Scale y0 by r8 (This loses a bit of precision)

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M0] = r0; /* r3 = temp store of cos */    // I3 read +1, I5 write +4

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M2] = r0, r1 = M[I4, M1]; // I2 write +3, I4 read +1

      r6 = r3 * r8 (frac), r3 = M[I1,M1]; /* copy across cos */ // I1 read -3

      I1 = I1 - M0, // (To make the I1 -3 store happen)
         r4 = M[I7,M1];

      r4 = r4 * r7 (frac); // Scale sine

      // M3 now superfluous

      do penultimate_loop;

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M1]; // I1 write + 4, I5 read -3

         I5 = I5 - M0;

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M1],  M[I6, M2] = r0; // I0 read + 1, I6 write +3

         r1 = r1 * r8 (frac); // Scale x0 by r8 (This loses a bit of precision)
         r2 = r2 * r8 (frac); // Scale y0 by r8 (This loses a bit of precision)

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I1,M3], M[I5, M0] = r0; // I1 read -1 , I5 write + 4

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M1] = r0, r1 = M[I4, M2];// I2 write + 1, I4 read + 3

         // Second set of 4

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M0] = r0, r5 = M[I5,M3]; // I5 read -1, I1 write +4

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M2],  M[I6, M1] = r0; // I6 write +1, I0 read + 3

         r1 = r1 * r8 (frac); // Scale x0 by r8 (This loses a bit of precision)
         r2 = r2 * r8 (frac); // Scale y0 by r8 (This loses a bit of precision)

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M0] = r0; /* r3 = temp store of cos */ // I5 write +4

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M2] = r0, r1 = M[I4, M1]; // I4 read +1, I2 write + 3

         r6 = r3 * r8 (frac), r3 = M[I1,M1]; /* copy across cos */

         I1 = I1 - M0, r4 = M[I7,M1];      // I1 read -3

         r4 = r4 * r7 (frac); // Scale sine

      penultimate_loop:

      rMACB = rMACB * Null, r0 = r2 + rMAC, M[I1, M1] = r0, r5 = M[I5,M3]; // (like instruction 1) (including I5 dummy read of -1 turned to -3)

      I5 = I5 - 2*ADDR_PER_WORD;

      rMACB = rMACB * Null, r0 = r2 - rMAC,  M[I6, M2] = r0; // (like instruction 2)

      M[I5, M1] = r0;

      jump $math.scaleable_fft_last_stage;

.ENDMODULE;

#endif // FFT_PIPELINED_LAST_STAGES

// $****************************************************************************
// PRIVATE FUNCTION:
//    $scaleable_fft_last_stage
//
// DESCRIPTION:
//    Carries out the last stage of the scaleable fft or ifft
//
// *****************************************************************************
.MODULE $M.math.scaleable_fft_last_stage;
   .CODESEGMENT MATH_SCALEABLE_FFT_LAST_STAGE_PM;
   .DATASEGMENT DM;

   $math.scaleable_fft_last_stage:

      // r7 and r8 contain the appropriate scaling factors

      I1 = $fft.next_groups;
   #ifdef FFT_LOW_RAM
      r0 = M[$fft.twiddle_real_address];
      I3 = r0;
      r0 = M[$fft.twiddle_imag_address];
      I7 = r0;
   #else
      I3 = $fft.twiddle_real; // I3 -> C of W0
      I7 = $fft.twiddle_imag; // I7 -> (-S) of W0
   #endif

      r0 = M[I1,M1];             // load groups
      r10 = r0 - 1,              // r10 = groups - 1 (loop counter)
         r3 = M[I3,M1];          // Load cos(w0)

      r6 = r3 * r8 (frac);       // Copy across cos(w0) and scale

      I2 = M[SP - 2*ADDR_PER_WORD]; // I2 -> x0 in 1st group of stage
      I4 = I2;                       // I4 -> x0 in 1st group of stage

      I6 = M[SP - ADDR_PER_WORD];   // I6 -> y0 in 1st group of stage

      I1 = I2 + ADDR_PER_WORD;      // I1 -> x1 in 1st group of stage


      I0 = I6;                   // I0 -> y0 in 1st group of stage
      I5 = I6 + ADDR_PER_WORD;   // I5 -> y1 in 1st group of stage

      M3 = M3 + M3,              //  -1 to -2
         r4 = M[I7,MK1];
      r4 = r4 * r7 (frac);       // Scale sin(w0)
      M0 = 2*ADDR_PER_WORD;     // Already 2 when penultimate stage not used

      // Ramp up to loop

      M2 = M0 - M3,   // = +4
         r3 = M[I1,M0];

      rMACB = r3 * r6, r0 = r2 + rMAC, r5 = M[I5,M0];

      rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M0], r1 = M[I4, M0];

      r1 = r1 * r8 (frac); // Scale x0 by r8 (This loses a bit of precision)
      r2 = r2 * r8 (frac); // Scale y0 by r8 (This loses a bit of precision)

      rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1]; // r3 used to load next cos // I3, I7 both in DM1 (stall)

      rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M0] = r0, r4 = M[I7,M1];

      r6 = r3 * r8 (frac),
         r3 = M[I1,M3]; // I2, I1 both in DM1 (stall)

      r4 = r4 * r7 (frac), r1 = M[I4, M0];

      do last_loop;

         rMACB = r3 * r6, r0 = r2 + rMAC, M[I1, M2] = r0, r5 = M[I5,M3]; /* Minus 2 from I5 */

         rMACB = rMACB - r5 * r4, r0 = r2 - rMAC, r2 = M[I0, M0],  M[I6, M0] = r0;

         r1 = r1 * r8 (frac); // Scale x0 by r8 (This loses a bit of precision)

         rMAC = r3 * r4, r0 = r1 + rMACB, r3 = M[I3,M1], M[I5, M2] = r0; /* r3 = temp store of cos */ // I3, I6 both in DM2 (stall)

         rMAC = rMAC + r5 * r6, r0 = r1 - rMACB, M[I2, M0] = r0, r1 = M[I4, M0]; // I5, I5 both in DM2 (stall)

         r2 = r2 * r8 (frac), // Scale y0 by r8 (This loses a bit of precision)
            r4 = M[I7,MK1];

         r4 = r4 * r7 (frac); // Scale sine by r7

         r6 = r3 * r8 (frac), r3 = M[I1,M3]; /* copy across cos and scale by r8 */

      last_loop:

      SP = SP - 2*ADDR_PER_WORD, popm <I7>;

      rMACB = rMACB * Null, r0 = r2 + rMAC, M[I1, M1] = r0, r5 = M[I5,M3]; // (like instruction 1) (including I5 dummy read)

      rMACB = rMACB * Null, r0 = r2 - rMAC,  M[I6, M0] = r0; // (like instruction 2)

      pop rLink, rMAC = M[I7,-MK1]; // Decrement I7 back to where it was when the function was called

      M[I5, M1] = r0; // Write last sample during what is otherwise a stall cycle

      rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $math.bitreverse_array
//
// DESCRIPTION:
//    A routine to bit-reverse an array
//
// INPUTS:
//    - I4 = pointer to input array
//    - I0 = bit reversed pointer to output array
//    - r10 = array size (must be an integer power of two)
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    r0, r1, I0, I4, M1, M2
//
// CPU USAGE:
//    10 + N Cycles
//
// NOTES:
//    1) The output buffer must have ben declared as circular so that the
//       bitreverse addressing to operate correctly.
//    2) Input and output buffers cannot overlap in memory
//    3) The bit reverse flag in rFlags WILL BE CLEARED after this function
// *****************************************************************************
.MODULE $M.math.bitreverse_array;
   .CODESEGMENT MATH_BITREVERSE_ARRAY_PM;
   .DATASEGMENT DM;

$math.bitreverse_array:
   M1 = MK1;

   // calculate modifier
   r0 = SIGNDET r10;

   r0 = r0 + 1 - LOG2_ADDR_PER_WORD;

   r1 = 1;
   // First load
   r1 = r1 LSHIFT r0, r0 = M[I4,MK1];
   M2 = r1;

   // enable bit reverse addressing on AG1
   rFlags = rFlags OR $BR_FLAG;

   // main loop (one extra load)
   do loop;
      r0 = M[I4,M1], M[I0,M2] = r0;
   loop:

   // disable Bit Reverse addressing on AG1
   rFlags = rFlags AND $NOT_BR_FLAG;
   rts;

.ENDMODULE;


#if defined(FFT_LOW_RAM)
// *****************************************************************************
// MODULE:
//    $math.load_fft_twiddle_factors
//
// DESCRIPTION:
//    A routine to load fft twiddle factors from flash
//
// INPUTS:
//    - r0 = number of fft points needed (should be a power of 2 up to $fft.NUM_POINTS)
//    - r1 = pointer to a buffer to load the real twiddle factors into
//    - r2 = pointer to a buffer to load the imaginary twiddle factors into
//
// OUTPUTS:
//    - Nothing
//
// TRASHED REGISTERS:
//    - r0-r6, doLoop, I0, I1, I4, M0, M1
//
// CPU USAGE:
//    -
//
// NOTES:
//    1) loads $fft.twiddle_real_address and $fft.twiddle_imag_address
//       with r1 and r2
// *****************************************************************************
.MODULE $M.math.load_fft_twiddle_factors;
   .CODESEGMENT MATH_LOAD_FFT_TWIDDLE_FACTORS_PM;
   .DATASEGMENT DM;

   $math.load_fft_twiddle_factors:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($math.FFT_TWIDDLE.LOAD_FFT_TWIDDLE_FACTORS.PATCH_ID_0)
#endif

   Null = r0 - $fft.NUM_POINTS;
   // Exit if we are trying to load more factors than those we have in ROM
   if GT jump $pop_rLink_and_rts;
   push r0;
   // store input arguments
   M[$fft.twiddle_real_address] = r1;
   M[$fft.twiddle_imag_address] = r2;

   // copy the real part of twiddle factors
   r2 = r1;             // Real pointer, (Destination Address)
   r1 = r0 ASHIFT -1;   // Number of factors
   r0 = &$fft.twiddle_real;  // Flash address
   call $mem.ext_window_unpack_to_ram;

   // generate imaginary fft twiddle factors from the real ones
   r0 = M[$fft.twiddle_real_address];
   I0 = r0 + 3*ADDR_PER_WORD;
   r0 = M[$fft.twiddle_imag_address];
   I4 = r0 + 2*ADDR_PER_WORD;
   // The first and second elements of imag are 0 and -1 respectively
   // written literally cause negating 1 is slightly inaccurate
   M[r0] = Null;
   r1 = -1.0;
   M[r0 + 1*ADDR_PER_WORD] = r1;

   pop r0;
   r10 = r0 ASHIFT -2;
   r10 = r10 - 1;

   M0 = 3*MK1;
   M1 = MK1;
   r0 = M[I0, -MK1];
   do imag_loop;
      M[I4, M1] = r0,
       r1 = M[I0, M0];
      r1 = -r1;
      M[I4, MK1] = r1,
       r0 = M[I0, -MK1];
   imag_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif // defined(FFT_LOW_RAM)
#endif // MATH_FFT_INCLUDED
