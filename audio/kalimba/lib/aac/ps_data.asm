// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_PS_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.ps_data
//
// DESCRIPTION:
//    -
//
// INPUTS:
//    - r8 num_bits_left
//
// OUTPUTS:
//    - r8 = num_bits_left (updated)
//
// TRASHED REGISTERS:
//    - toupdate
//
// *****************************************************************************
.MODULE $M.aacdec.ps_data;
   .CODESEGMENT AACDEC_PS_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.ps_data:

   // push rLink onto stack
   push rLink;


   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r5 = M[r9 + $aac.mem.PS_info_ptr];
   M[r5 + $aacdec.PS_BIT_COUNT_PRE_DATA] = r0;

   // PS_ENABLE_HEADER = getbits(1)
   call $aacdec.get1bit;

   // if(PS_ENABLE_HEADER)
   if Z jump end_if_ps_enable_header;
      // PS_ENABLE_IID = getbits(1)
      call $aacdec.get1bit;
      M[r5 + $aacdec.PS_ENABLE_IID] = r1;
      // if(PS_ENABLE_IID)
      if Z jump end_if_enable_iid_ps_header;
         // PS_IID_MODE = getbits(3)
         call $aacdec.get3bits;
         M[r5 + $aacdec.PS_IID_MODE] = r1;
         // PS_NR_IID_PAR = ps_nr_par_table[PS_IID_MODE]
         Words2Addr(r1);
         r0 = M[r9 + $aac.mem.PS_nr_par_table_ptr];
         r0 = M[r0 + r1];
         M[r5 + $aacdec.PS_NR_IID_PAR] = r0;
         // PS_NR_IPDOPD_PAR = ps_nr_ipdopd_par_tab[PS_IID_MODE]
         r2 = M[r9 + $aac.mem.PS_nr_ipdopd_par_tab_ptr];
         r1 = M[r2 + r1];
         M[r5 + $aacdec.PS_NR_IPDOPD_PAR] = r1;
      end_if_enable_iid_ps_header:

      // PS_ENABLE_ICC = getbits(1)
      call $aacdec.get1bit;
      M[r5 + $aacdec.PS_ENABLE_ICC] = r1;
      // if(PS_ENABLE_ICC)
      if Z jump end_if_enable_icc_ps_header;
         // PS_ICC_MODE = getbits(3)
         call $aacdec.get3bits;
         M[r5 + $aacdec.PS_ICC_MODE] = r1;
         // PS_NR_ICC_PAR = ps_nr_par_table[PS_ICC_MODE]
         Words2Addr(r1);
         r0 = M[r9 + $aac.mem.PS_nr_par_table_ptr];
         r0 = M[r0 + r1];
         M[r5 + $aacdec.PS_NR_ICC_PAR] = r0;
      end_if_enable_icc_ps_header:

      // PS_ENABLE_EXT = getbits(1)
      call $aacdec.get1bit;
      M[r5 + $aacdec.PS_ENABLE_EXT] = r1;
   end_if_ps_enable_header:


   // PS_FRAME_CLASS = getbits(1) : 0 = fixed sized envelopes; 1 = variable sized envelopes
   call $aacdec.get1bit;
   M[r5 + $aacdec.PS_FRAME_CLASS] = r1;
   // PS_NUM_ENV = ps_num_env_tab[PS_FRAME_CLASS][PS_NUM_ENV_INDEX]
   r4 = r1 * (4 * ADDR_PER_WORD) (int);
   // PS_NUM_ENV_INDEX = getbits(2)
   call $aacdec.get2bits;
   Words2Addr(r1);
   r0 = M[r9 + $aac.mem.PS_num_env_tab_ptr];
   r1 = r1 + r0;
   r10 = M[r1 + r4];
   M[r5 + $aacdec.PS_NUM_ENV] = r10;


   // if(PS_FRAME_CLASS == 1)
   Null = r4;
   if Z jump end_if_ps_frame_class;
      // r4 = envelope; PS_BORDER_POSITION[0] = 0 (always)
      r4 = r5 + ADDR_PER_WORD;
      do ps_envelope_borders_loop;
         // PS_BORDER_POSITION[envelope] = getbits(5) + 1
         call $aacdec.get5bits;
         r1 = r1 + 1;
         M[r4 + $aacdec.PS_BORDER_POSITION] = r1;
         r4 = r4 + ADDR_PER_WORD;
      ps_envelope_borders_loop:
   end_if_ps_frame_class:


   // if(PS_ENABLE_IID)
   Null = M[r5 + $aacdec.PS_ENABLE_IID];
   if Z jump end_if_enable_iid;
      r6 = -3;
      r0 = M[r5 + $aacdec.PS_IID_MODE];
      r6 = r6 + r0;
      if LT jump iid_coarse_time_resolution;
         // fine IID time resolution
         I3 = &$aacdec.t_huffman_iid_fine;
         I4 = &$aacdec.f_huffman_iid_fine;
         r7 = (60 * 2);
         jump end_if_iid_coarse_or_fine_resolution;
      iid_coarse_time_resolution:
         // coarse IID time resolution
         I3 = &$aacdec.t_huffman_iid_def;
         I4 = &$aacdec.f_huffman_iid_def;
         r7 = (28 * 2);
      end_if_iid_coarse_or_fine_resolution:


      M[r5 + $aacdec.PS_HUFFMAN_TABLES_TOTAL_SIZE] = r7;
      r0 = I3;
      call $aacdec.sbr_allocate_and_unpack_from_flash;
      I3 = r1;
      r0 = I4;
      call $aacdec.sbr_allocate_and_unpack_from_flash;
      I4 = r1;

      r6 = M[r5 + $aacdec.PS_NR_IID_PAR];
      r5 = M[r5 + $aacdec.PS_NUM_ENV];
      Words2Addr(r5);
      if Z jump end_if_enable_iid;
         // r7 = envelope
         r7 = 0;
         // for envelope=0:PS_NUM_ENV-1,
         ps_iid_huffman_loop:
            r0 = r7 * $aacdec.PS_MAX_NUM_PARAMETERS (int);
            r4 = M[r9 + $aac.mem.PS_info_ptr];
            r1 = r4 + $aacdec.PS_IID_INDEX;
            I5 = r0 + r1;
            r10 = r6;
            I2 = I3;  // I2 = time coded IID huffman table pointer
            // PS_IID_CODING_DIRECTION[envelope] = getbits(1)
            call $aacdec.get1bit;
            r4 = r4 + $aacdec.PS_IID_CODING_DIRECTION;
            M[r4 + r7] = r1;
            if Z I2 = I4;  // I2 = frequency coded IID huffman table pointer
            // huffman decode all IID quantisation levels for this envelope
            do ps_iid_huffman_decode_loop;
               call $aacdec.ps_huff_dec;
               M[I5,MK1] = r1;
            ps_iid_huffman_decode_loop:
         r7 = r7 + ADDR_PER_WORD;
         Null = r5 - r7;
         if GT jump ps_iid_huffman_loop;
   end_if_enable_iid:


   // if(PS_ENABLE_ICC)
   r5 = M[r9 + $aac.mem.PS_info_ptr];
   Null = M[r5 + $aacdec.PS_ENABLE_ICC];
   if Z jump end_if_enable_icc;
      r7 = 14;
      r0 = &$aacdec.t_huffman_icc;
      call $aacdec.sbr_allocate_and_unpack_from_flash;
      I3 = r1;
      r0 = &$aacdec.f_huffman_icc;
      call $aacdec.sbr_allocate_and_unpack_from_flash;
      I4 = r1;

      r0 = M[r5 + $aacdec.PS_HUFFMAN_TABLES_TOTAL_SIZE];
      r0 = r0 + (14*2);
      M[r5 + $aacdec.PS_HUFFMAN_TABLES_TOTAL_SIZE] = r0;


      r6 = M[r5 + $aacdec.PS_NR_ICC_PAR];
      r5 = M[r5 + $aacdec.PS_NUM_ENV];
      if Z jump end_if_enable_icc;
         Words2Addr(r5);
         // r7 = envelope
         r7 = 0;
         // for envelope=0:PS_NUM_ENV-1,
         ps_icc_huffman_loop:
            r0 = r7 * $aacdec.PS_MAX_NUM_PARAMETERS (int);
            r4 = M[r9 + $aac.mem.PS_info_ptr];
            r1 = r4 + $aacdec.PS_ICC_INDEX;
            I5 = r0 + r1;
            r10 = r6;
            I2 = I3;  // time coded ICC huffman table pointer
            // PS_ICC_CODING_DIRECTION[envelope] = getbits(1)
            call $aacdec.get1bit;
            r4 = r4 + $aacdec.PS_ICC_CODING_DIRECTION;
            M[r4 + r7] = r1;
            if Z I2 = I4;  // frequency coded ICC huffman table pointer
            // huffman decode all ICC quantisation levels for this envelope
            do ps_icc_huffman_decode_loop;
               call $aacdec.ps_huff_dec;
               M[I5, MK1] = r1;
            ps_icc_huffman_decode_loop:
         r7 = r7 + ADDR_PER_WORD;
         Null = r5 - r7;
         if GT jump ps_icc_huffman_loop;
   end_if_enable_icc:


   // if(PS_ENABLE_EXT)
   r5 = M[r9 + $aac.mem.PS_info_ptr];
   Null = M[r5 + $aacdec.PS_ENABLE_EXT];
   if Z jump end_if_enable_ext;
      // PS_CNT = getbits(4)
      call $aacdec.get4bits;
      // if(PS_CNT == 15)
      Null = r1 - 15;
      if NZ jump end_if_esc_count;
         // PS_ESC_COUNT = getbits(8)
         call $aacdec.get1byte;
         r1 = r1 + 15;
      end_if_esc_count:
      // r7 = ps_num_ext_bits_left = PS_CNT * 8
      r7 = r1 * 8 (int);
      if Z jump update_num_bits_left;
         // while(ps_num_ext_bits_left > 7)
         ps_extension_loop:
            // PS_EXTENSION_ID = getbits(2)
            r7 = r7 - 2;
            call $aacdec.get2bits;
            // if PS_EXTENSION_ID == 0)
            if Z call $aacdec.ps_extension;
         Null = r7 - 7;
         if GT jump ps_extension_loop;

         r0 = r7;
         if NZ call $aacdec.getbits;
      update_num_bits_left:
   end_if_enable_ext:


   // update num_bits_left
   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r1 = M[r5 + $aacdec.PS_BIT_COUNT_PRE_DATA];
   r0 = r0 - r1;
   r8 = r8 - r0;

   r0 = M[r5 + $aacdec.PS_HUFFMAN_TABLES_TOTAL_SIZE];
   if NZ call $aacdec.frame_mem_pool_free;
   M[r5 + $aacdec.PS_HUFFMAN_TABLES_TOTAL_SIZE] = Null;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
