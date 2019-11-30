// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.program_element_config
//
// DESCRIPTION:
//    Get the program element config data
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r6, r10
//
// *****************************************************************************
.MODULE $M.aacdec.program_element_config;
   .CODESEGMENT AACDEC_PROGRAM_ELEMENT_CONFIG_PM;
   .DATASEGMENT DM;

   $aacdec.program_element_config:

   // push rLink onto stack
   push rLink;

   // dummy = getbits(4);  //element_instance_tag
   call $aacdec.get4bits;

   // profile = getbits(2);
   call $aacdec.get2bits;

   // if (profile != 1 && profile != 3) // LC profile
   // {
   //    error;
   // }
   Null = r1 AND 0x0001;
   if Z jump $aacdec.possible_corruption;        // non-LC, non-LTP profile error

   // sampling_frequency_index = getbits(4);
   call $aacdec.get4bits;

   // check sampling freq is in allowed set
   Words2Addr(r1);
   r0 = &$aacdec.sampling_freq_lookup;
   r6 = r1;
   call $mem.ext_window_access_as_ram;
   r1 = r6;
   r0 = M[r0 + r6];
   if NEG jump $aacdec.possible_corruption;        // unsupported frequency error

   // skip all remaining program_element_config data
   .CONST NUM_FRONT_CHANNEL_ELEMENTS (0*ADDR_PER_WORD);
   .CONST NUM_SIDE_CHANNEL_ELEMENTS  (1*ADDR_PER_WORD);
   .CONST NUM_BACK_CHANNEL_ELEMENTS  (2*ADDR_PER_WORD);
   .CONST NUM_LFE_CHANNEL_ELEMENTS   (3*ADDR_PER_WORD);;
   .CONST NUM_ASSOC_DATA_ELEMENTS    (4*ADDR_PER_WORD);
   .CONST NUM_VALID_CC_ELEMENTS      (5*ADDR_PER_WORD);

   // allocate 6 words of temp memory
   r0 = 6;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   r6 = r1;

   // read num_front_channel_elements
   call $aacdec.get4bits;
   M[r6 + NUM_FRONT_CHANNEL_ELEMENTS] = r1;

   // read num_side_channel_elements
   call $aacdec.get4bits;
   M[r6 + NUM_SIDE_CHANNEL_ELEMENTS] = r1;

   // read num_back_channel_elements
   call $aacdec.get4bits;
   M[r6 + NUM_BACK_CHANNEL_ELEMENTS] = r1;

   // read num_lfe_channel_elements
   call $aacdec.get2bits;
   M[r6 + NUM_LFE_CHANNEL_ELEMENTS] = r1;

   // read num_assoc_data_elements
   call $aacdec.get3bits;
   M[r6 + NUM_ASSOC_DATA_ELEMENTS] = r1;

   // read num_valid_cc_elements
   call $aacdec.get4bits;
   M[r6 + NUM_VALID_CC_ELEMENTS] = r1;

   // read mono_mixdown
   call $aacdec.get1bit;
   if NZ call $aacdec.get4bits;

   // read stereo_mixdown
   call $aacdec.get1bit;
   if NZ call $aacdec.get4bits;

   // read matrix_mixdown
   call $aacdec.get1bit;
   if NZ call $aacdec.get3bits;

   // read front channel elements
   r10 = M[r6 + NUM_FRONT_CHANNEL_ELEMENTS];
   do front_channel_elements_loop;
      call $aacdec.get5bits;
   front_channel_elements_loop:

   // read side channel elements
   r10 = M[r6 + NUM_SIDE_CHANNEL_ELEMENTS];
   do side_channel_elements_loop;
      call $aacdec.get5bits;
   side_channel_elements_loop:

   // read back channel elements
   r10 = M[r6 + NUM_BACK_CHANNEL_ELEMENTS];
   do back_channel_elements_loop;
      call $aacdec.get5bits;
   back_channel_elements_loop:

   // read lfe elements
   r10 = M[r6 + NUM_LFE_CHANNEL_ELEMENTS];
   do lfe_channel_elements_loop;
      call $aacdec.get4bits;
   lfe_channel_elements_loop:

   // read assoc_data elements
   r10 = M[r6 + NUM_ASSOC_DATA_ELEMENTS];
   do assoc_data_elements_loop;
      call $aacdec.get4bits;
   assoc_data_elements_loop:

   // read valid_cc elements
   r10 = M[r6 + NUM_ASSOC_DATA_ELEMENTS];
   do valid_cc_elements_loop;
      call $aacdec.get5bits;
   valid_cc_elements_loop:


   // byte align
   call $aacdec.byte_align;

   // read comment_field_bytes
   call $aacdec.get1byte;
   // read comment_field_data
   r10 = r1;
   do comment_field_loop;
      call $aacdec.get1byte;
   comment_field_loop:


   // free the 6 words of temp memory
   r0 = 6;
   call $aacdec.tmp_mem_pool_free;
   Null = r0;
   if NEG jump $aacdec.possible_corruption;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;
