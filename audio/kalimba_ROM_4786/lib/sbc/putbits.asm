// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCENC_PUTBITS_INCLUDED
#define SBCENC_PUTBITS_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.putbits
//
// DESCRIPTION:
//    Put bits into buffer
//
// INPUTS:
//    - r0 = number of bits to put in buffer
//    - r1 = the data to put in the buffer
//    - I0 = buffer pointer to write words to
//    - r5 = pointer to encoder structure with valid data object
//
// OUTPUTS:
//    - I0 = buffer pointer to write words to (updated)
//
// TRASHED REGISTERS:
//    r2, r3
//
// NOTES:
//  Equivalent Matlab code:
//
//  @verbatim
//  function put_bits(bits, NoBits);
//
//  global OutStream;  % Outfile related variables
//
//  % OutStream.FID             - file ID
//  % OutStream.BitPos          - current position of bit to be written (MSB (8) first)
//  % OutStream.DataByte        - buffer for 1 byte of data
//  % OutStream.bit_count       - number of bits written
//
//  for bitno = NoBits : -1: 1,
//
//     bit = bitget(bits,bitno);
//     OutStream.DataByte = bitset(OutStream.DataByte, OutStream.BitPos,bit);
//
//     OutStream.BitPos = OutStream.BitPos - 1;
//
//     if (OutStream.BitPos == 0)   % then write byte to the file
//        NoWritten = fwrite(OutStream.FID,OutStream.DataByte,'uint8');
//        if NoWritten == 0
//          error('Could''t write to output file');
//        end
//        OutStream.BitPos = 8;
//     end
//
//     OutStream.bit_count = OutStream.bit_count + 1;
//  end
//  @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcenc.putbits;
   .CODESEGMENT SBCENC_PUTBITS_PM;
   .DATASEGMENT DM;


    $sbcenc.putbits_external:

   // -- Load memory structure pointer
   // This pointer should have been initialised externally
   r9 = M[r5 + $codec.ENCODER_DATA_OBJECT_FIELD];

    $sbcenc.putbits:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($sbcenc.PUTBITS_ASM.PUTBITS.PATCH_ID_0, r2)
#endif

   // put_bitpos should be initialised to 16 and put_nextword to 0

   r2 = M[r9 + $sbc.mem.PUT_BITPOS_FIELD];

   // r2 = shift amount
   r2 = r2 - r0;
   // see if another word needs to be written
   if LE jump anotherword;

      // shift new data to the left
      r3 = r1 LSHIFT r2;

      // update bitpos
      M[r9 + $sbc.mem.PUT_BITPOS_FIELD] = r2;
      // get any previous data to write
      // TODO this can be optimised if PUT_NEXTWORD_FIELD is the first field in the array
      r2 = M[r9 + $sbc.mem.PUT_NEXTWORD_FIELD];
      // add in the new data
      r3 = r3 + r2;
      M[r9 + $sbc.mem.PUT_NEXTWORD_FIELD] = r3;

      rts;

   anotherword:

      push r0;
      // shift current word right if needed
      r3 = r1 LSHIFT r2;
      // TODO this can be optimised if PUT_NEXTWORD_FIELD is the first field in the array
      // add in any previous data to write
      r0 = M[r9 + $sbc.mem.PUT_NEXTWORD_FIELD];
      r3 = r3 + r0;
      r3 = r3 AND 0xffff;
      // write the data
      M[I0, MK1] = r3;
      r2 = r2 + 16;
      // any remaining data put in MSBs of sbc_nextword
      r3 = r1 LSHIFT r2;
      M[r9 + $sbc.mem.PUT_NEXTWORD_FIELD] = r3;
      // update bitpos
      M[r9 + $sbc.mem.PUT_BITPOS_FIELD] = r2;
      pop r0;

      rts;

.ENDMODULE;

#endif
