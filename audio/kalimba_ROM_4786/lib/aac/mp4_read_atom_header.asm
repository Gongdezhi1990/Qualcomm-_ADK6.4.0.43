// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.mp4_read_atom_header
//
// DESCRIPTION:
//    Read the header of an atom (the size and name tag)
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - r4 = most significant byte of sub_atom_size
//    - r5 = least significant 3 bytes of sub_atom_size
//    - r6 = least significant 2 bytes of sub_atom_name
//    - r7 = most significant 2 bytes of sub_atom_name
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.aacdec.mp4_read_atom_header;
   .CODESEGMENT AACDEC_MP4_READ_ATOM_HEADER_PM;
   .DATASEGMENT DM;

   $aacdec.mp4_read_atom_header:

   // push rLink onto stack
   push rLink;

   // read MS word of atom size (16bits)
   call $aacdec.get2bytes;
   r4 = r1;

   // read LS word of atom size (16bits)
   call $aacdec.get2bytes;

   // {r4:r5} = atom size (32bit)
   r0 = r4 LSHIFT 16;
   r5 = r1 + r0;
   r4 = r4 LSHIFT -8;

   // read MS word of atom name (16bits)
   call $aacdec.get2bytes;
   r7 = r1;

   // read LS word of atom name (16bits)
   call $aacdec.get2bytes;
   r6 = r1;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
