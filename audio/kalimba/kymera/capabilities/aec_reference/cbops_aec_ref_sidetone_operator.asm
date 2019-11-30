// *****************************************************************************
// Copyright (c) 2018 Qualcomm Technologies International, Ltd.
// *****************************************************************************

// *****************************************************************************
// NAME:
//    aec_ref_sidetone_op cbops operator
//
// DESCRIPTION:
//    Override Cbops operator for in-place mixing sidetone (directly
//    into speaker MMU buffer
//
//                       +--------+
//  SIDETONE_BUFFER ---->|   MIX  |--->SPKR_MMU_BUFFER
//                  +--->|        |      |
//                  |    +--------+      |
//                  |                    |
//                  +--------------------+
//
// SIDETONE_BUFFER is the output of sidetone cbops graph:
//  MIC_BUFFER -> Sidetone filter -> DC_RM -> resampler -> latency-control->SIDETONE_BUFFER
//
// Note: this is only suitable for Hydra archs
// *****************************************************************************

#include "cbops/cbops.h"
#include "cbuffer_asm.h"
#include "cbops_aec_ref_sidetone_op_asm_defs.h"
#ifdef PATCH_LIBS
#include "patch/patch_asm_macros.h"
#endif

.MODULE $M.cbops.aec_ref_sidetone_op;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.aec_ref_sidetone_op[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.aec_ref_sidetone_op.pre_main,            // pre-main function
      &$cbops.aec_ref_sidetone_op.amount_to_use,       // amount to use function
      &$cbops.aec_ref_sidetone_op.post_main;           // post-main function

// Expose the location of this table to C
.set $_cbops_aec_ref_sidetone_table , $cbops.aec_ref_sidetone_op

// *****************************************************************************
// MODULE:
//   $cbops.aec_ref_sidetone_op.amount_to_use
//
// DESCRIPTION:
//   Override operator's amount to use function, works out
//   the amount that needed to mix this run.
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    Assume anything except r4 and r8
//
// *****************************************************************************

// Called before amount_to_use of graph
$cbops.aec_ref_sidetone_op.amount_to_use:
#ifdef PATCH_LIBS
   LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_sidetone_op.amount_to_use.PATCH_ID_0, r7)
#endif
   push rlink;

   // Get data pointer
   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // update sidetone mic buffer
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.MIC_BUF_FIELD];
   call $cbuffer.get_write_address_and_size_and_start_address;
   r6 = r0;
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.MIC_IDX_FIELD];
   call $cbops.get_cbuffer;
   r1 = r6;
   r6 = r0;
   call $cbuffer.set_write_address;

   // update amount to read from mic
   r0 = r6;
   call $cbuffer.calc_amount_data_in_words;
   r6 = r0;
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.MIC_IDX_FIELD];
   call $cbops.get_amount_ptr;
   M[r0] = r6;

   /* See how many samples speakr has moved, we will mix the same
    * number of samples from sidetone buffer.
    */
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_BUFS_FIELD];
   r0 = M[r0];
   call $cbuffer.get_read_address_and_size_and_start_address;
   r3 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.LAST_SPKR_ADDR_FIELD];
   M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.LAST_SPKR_ADDR_FIELD] = r0;
   r3 = r0 - r3;
   if NEG r3 = r3 + r1;
   BUFFER_ADDRS_TO_WORDS_ASM(r3);
   M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.AMOUNT_TO_MIX_FIELD] = r3;
   pop rlink;
   rts;

// *****************************************************************************
// MODULE:
//   $cbops.aec_ref_sidetone_op.pre_main
//
// DESCRIPTION:
//   Override operator's pre main function.
//   Called after amount to use of graph but before main processing of graph
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    Assume anything except r4 and r8
//
// *****************************************************************************
$cbops.aec_ref_sidetone_op.pre_main:
   push rLink;
   /* Force processing */
#ifdef PATCH_LIBS
   LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_sidetone_op.pre_main.PATCH_ID_0, r6)
#endif
   call $cbops.force_processing;
   pop rlink;
   rts;

// *****************************************************************************
// MODULE:
//   $cbops.aec_ref_sidetone_op.post_main
//
// DESCRIPTION:
//   Override operator's post main function.
//   Called after running main processing of graph
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    Assume anything except r4 and r8
//
// *****************************************************************************
$cbops.aec_ref_sidetone_op.post_main:
#ifdef PATCH_LIBS
   LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.aec_ref_sidetone_op.post_main.PATCH_ID_0, r7)
#endif
   push rlink;

   // Get data pointer
   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // get speaker buffer
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_BUFS_FIELD];
   r0 = M[r0];
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;
   // save read address for later use
   I6 = r0;
   // I6 = read address of first speaker buffer

   // get threshold
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SAFETY_THRESHOLD_LOW_FIELD];
   BUFFER_WORDS_TO_ADDRS_ASM(r0);
   M0 = r0;

   // r6 = spkr_rd + threshold_low
   r0 = M[I0, M0];
   r6 = I0;

   // r5 = spkr_rd + threshold_high
   I0 = I6;
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SAFETY_THRESHOLD_HIGH_FIELD];
   BUFFER_WORDS_TO_ADDRS_ASM(r0);
   M0 = r0;
   r0 = M[I0, M0];
   r5 = I0;

   r0 = r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_MIXING_OFFSETS_FIELD;
   r0 = M[r0];
   I0 = r0;
   I7 = r0;
   // ====================================================================
   // r0 = mixing point address
   // expect at this time mixing address to be in [r6, r5], circular-wise,
   // if not it needs to be forced back to a suitable point
   //            r0
   // --|----|---^---|-------------------------------------|--
   //   RD   Low     High                                 WR
   //
   // ====================================================================
   Null = r5 - r6;
   if NEG jump neg_part;
   pos_part:
      // expect: r6 <= r0 < r5
      Null = r0 - r5;
      if POS jump reset_mixing_offset;
      Null = r0 - r6;
      if NEG jump reset_mixing_offset;
      jump mixing_point_check_done;

   neg_part:
      // expect: r0 < r5 or r0 >= r6
      Null = r0 - r6;
      if POS jump mixing_point_check_done;
      Null = r0 - r5;
      if NEG jump mixing_point_check_done;

reset_mixing_offset:
   /* reset mixing offset */
   r1 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SAFETY_SET_THRESHOLD_FIELD];
   BUFFER_WORDS_TO_ADDRS_ASM(r1);
   M0 = r1;
   I0 = I6;
   r1 = M[I0, M0];

   // update debug counter showing mixing offset has been re-aligned
   r2 = I0 - r0;
   if NEG r2 = r2 + L0;
   BUFFER_ADDRS_TO_WORDS_ASM(r2);
   r1 = M[r7+ $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SIDETONE_ALIGNED_SAMPLES_FIELD];
   r1 = r1 + r2;
   M[r7+ $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SIDETONE_ALIGNED_SAMPLES_FIELD] = r1;

   // r0 = mixing offset updated
   r0 = I0;
mixing_point_check_done:

   // at this point
   // r0 = speaker buffer mixing address
   // B0 = Speaker buffer base address
   // L0 = speaker buffer length
   // I6 = speaker buffer read address

   // work out the distance between HW read address and
   // mixing address in samples
   push B0;
   pop r1;
   r1 = r0 - I6;
   if NEG r1 = r1 + L0;
   BUFFER_ADDRS_TO_WORDS_ASM(r1);

   // get amount to mix
   r5 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.AMOUNT_TO_MIX_FIELD];
   // r1 = distance before mixing
   // r3 = distance after mixing
   r3 = r5 + r1;

   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_THRESHOLD_FIELD];
   r1 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SAFETY_THRESHOLD_LOW_FIELD];
   r2 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SAFETY_THRESHOLD_HIGH_FIELD];
   r9 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SAFETY_SET_THRESHOLD_FIELD];
   r1 = r0 + r1;   // low threshold after mixing
   r2 = r0 + r2;   // high threshold after mixing
   r9 = r0 + r9;   // good threshold after mixing

   // r6 = samples to insert
   r6 = 0;

   // expect r1 < r3 < r2
   r0 = r1 - r3;
   if GT jump insert_sidetone;
   r0 = r3 - r2;
   if LE jump mix_sidetone;

discard_sidetone:
   // discard here means ignore, we are doing in-place mixing
   // actual discard if needed will be done by sink_overflow_disgard operator
   r1 = r5 - r9;
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SIDETONE_DISCARDED_SAMPLES_FIELD];
   r0 = r0 + r1;
   M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SIDETONE_DISCARDED_SAMPLES_FIELD] = r0;
   r5 = r9;
   jump mix_sidetone;

insert_sidetone:
   // we don't have enough samples to mix, we needs to mix
   // the difference, it will be mixing using last mixed sample
   r6 = r9 - r5;
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SIDETONE_INSERTED_SAMPLES_FIELD];
   r0 = r0 + r6;
   M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SIDETONE_INSERTED_SAMPLES_FIELD] = r0;
   r5 = r9;
mix_sidetone:

   // M0 = amount to adjust
   M0 = I0 - I7;
   if NEG M0 = M0 + L0;

   // get number of speakers (1 or 2)
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.NR_SPKRS_FIELD];
   M3 = r0;

   // mixing offset address
   I2 = r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_MIXING_OFFSETS_FIELD;

   // Get sidetone buffer and see how much we need to read
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
   call $cbops.get_cbuffer;
   I5 = r0;

   // see how much data is in sidetone buffer
   call $cbuffer.calc_amount_data_in_words;
   r3 = r0;
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
   call $cbops.get_amount_ptr;
   r0 = M[r0];
   r0 = r0 + r3;
   // r0: amount we have
   // r5: amount we need
   // r6: amount to invent
   r1 = r5 - r0;
   if LE jump enough_data;
      // Limit to amount available
      r5 = r5 - r1;
      r6 = r6 + r1;
   enough_data:

   r0 = I5;
   call $cbuffer.get_read_address_and_size_and_start_address;
   I7 = r0;
   L4 = r1;
   push r2;
   pop B4;

   // speaker buffers
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_BUFS_FIELD];
   I3 = r0;

channel_mixing_loop:

      // get base address for this speaker channel
      r0 = M[I3, MK1];
      r0 = M[r0 + $cbuffer.START_ADDR_FIELD];
      push r0;
      pop B0;

      // get sidetone buffer
      I4 = I7;

      // get speaker mixing point
      r0 = M[I2, 0];
      I0 = r0;

      // M0 is amount to fix the mixing point (should be 0 in normal condition)
      r0 = M[I0, M0];

      // get last sidetone sample
      r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.LAST_SIDETONE_SAMPLE_FIELD];

      r2 = 1.0;

      // r5 = sidetone samples to read and mix
      // r6 = sidetone samples to invent and mix
      r10 = r6;
      if Z jump insert_done;
      do insert_loop;
         rMAC = M[I0, 0];
         rMAC = rMAC + r0 * r2;
         M[I0, MK1] = rMAC;
      insert_loop:

      insert_done:
      r10 = r5;
      do mix_loop;
         rMAC = M[I0, 0], r0 = M[I4,MK1];
         rMAC = rMAC + r0 * r2;
         M[I0, MK1] = rMAC;
      mix_loop:
      M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.LAST_SIDETONE_SAMPLE_FIELD] = r0;

      // update mixing point for next time
      r1 = I0;
      M[I2, MK1] = r1;
   // next channel
   M3 = M3 - 1;
   if GT jump channel_mixing_loop;

   // update sidetone buffer
   r0 = I5;
   r1 = I4;
   call $cbuffer.set_read_address;

   pop rlink;
   rts;

// *****************************************************************************
// MODULE:
//   void aec_ref_sidetone_initialise(cbops_op *);
//
// DESCRIPTION:
//   Initialises aece reference cbops sidetone mix operator
//
// INPUTS:
//    - r0 = pointer to operator structure
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    r0 (C callable)
//
// *****************************************************************************
$_aec_ref_sidetone_initialise:
   push rLink;
   pushm <I2,I3>;
   pushm <r1,r2,r3,r7>;

   // here we got pointer to the cbops structure, so need to get to the
   // parameter struct first.
   r7 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];

   // save the read address of first speaker buffer, we use this to see how many
   // samples speaker has read since last run
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_BUFS_FIELD];
   r0 = M[r0];
   call $cbuffer.get_read_address_and_size_and_start_address;

   /* initialise mixing points */
   M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.LAST_SPKR_ADDR_FIELD] = r0;
   r10 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.NR_SPKRS_FIELD];
   r0 = M[r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_BUFS_FIELD];
   I2 = r0;
   I3 = r7 + $cbops_aec_ref_sidetone_op.aec_ref_sidetone_op_struct.SPKR_MIXING_OFFSETS_FIELD;
   do init_mixing_points;
      r0 = M[I2, MK1];   // get speaker buffer
      r0 = M[r0 + $cbuffer.START_ADDR_FIELD];
      M[I3, MK1] = r0;   // mixing point
   init_mixing_points:

   popm <r1,r2,r3,r7>;
   popm <I2,I3>;
   pop rLink;
   rts;
.ENDMODULE;