// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Underrun compensation. It must be at the very end of the chain.
//
// DESCRIPTION:
//
// When using the multichannel operator the following data structure is used:
//    - header:
//          - nr inputs of the section at the end of which it is used (!)
//          - nr outputs that it operates on (== normally nr outs of section)
//          - <nr inputs> indexes for input channels (their values don't matter in this case,
//                        operator only concerns itself with the output buffers)
//          - <nr outputs> indexes for output channels (some may be marked as unused)
// *****************************************************************************

#include "cbops.h"
#include "cbops_underrun_comp_op_asm_defs.h"

.MODULE $M.cbops.underrun_comp_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.underrun_comp_op[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.underrun_comp_op.reset,           // reset function
      &$cbops.underrun_comp_op.amount_to_use,   // amount to use function
      &$cbops.underrun_comp_op.main;            // main function


#ifdef CBOPS_DEBUG_LOGGING
   .CONST $cbops.DEBUG_LOG_SIZE                                       100;
   .VAR $cbops.debug_log[$cbops.DEBUG_LOG_SIZE];
   .VAR $cbops.debug_logidx = 0;
#endif
.ENDMODULE;

// number of consecutive stalls that are tolerated before entering "not started" state again
.CONST $cbops.underrun_comp_op.MAX_CONSECUTIVE_STALLS                 3;

// number of data block size estimation updates we do - so that we can divide easily
.CONST $cbops.underrun_comp_op.NR_DATA_BLOCK_UPDATES                  16;
.CONST $cbops.underrun_comp_op.LOG2_NR_DATA_BLOCK_UPDATES             4;

// Expose the location of this table to C.
// Recommendation would be to standardise create() and possibly some configure() functionss in this table.
.set $_cbops_underrun_comp_table,  $cbops.underrun_comp_op



// *****************************************************************************
// MODULE:
//    $cbops.underrun_comp_op.reset
//
// DESCRIPTION:
//    Reset routine for the multi-channel version underrun compensation cbop.
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1
//
// *****************************************************************************
.MODULE $M.cbops.underrun_comp_op.reset;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   // ** reset routine **
   $cbops.underrun_comp_op.reset:

   r0 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // reset relevant counters, and the "start next kick" & started state flag
   M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD] = Null;
   M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.NR_CONSECUTIVE_STALLS_FIELD] = Null;

   M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SUM_DATA_BLKS_FIELD] = Null;
   M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.DATA_BLK_UPDATES_FIELD] = Null;
   M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.START_NEXT_KICK_FIELD] = Null;

   // a few vital pointers are not forgiven for being NULL at this point - something very
   // nasty happened if they are still uninitialised after zero alloc'ing the struct.
   r1 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
   if Z call $error;

   M[r1] = Null;

   r1 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.TOTAL_INSERTS_PTR_FIELD];
   if Z call $error;

   M[r1] = Null;

   rts;

.ENDMODULE;


// *******************************************************************************
// MODULE:
//    $cbops.underrun_comp_op.amount_to_use
//
// DESCRIPTION:
//    Amount to use function. Most of this used to be located at endpoint level,
//    where it performed the entire tedium several times, i.e. every channel.
//    "Down here" at cbop level one has direct visibility of data amounts and,
//    in case of a sync group, it can do this once for the whole chorus of channels.
//    One major quirk is that if an amount verdict is zero, that dictates it for the
//    chain, and main() funcs of cbops will not get called.
//    In such case, considering extra special role of this cbop, one needs to get to
//    its main() so that insertions can happen even if everything else in chain is
//    whining about total data starvation!
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r9, r10
//
// *******************************************************************************
.MODULE $M.cbops.underrun_comp_op.amount_to_use;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   // ** amount to use function **
$cbops.underrun_comp_op.amount_to_use:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.underrun_comp.amount_to_use.PATCH_ID_0, r1)
#endif
   push rlink;


   /* This operator is last in the chain.   This means the
      amount_to_use is called first.   This is a change from
      cbops sections where this would be called last for the
      section.

      This could affect the amount of input data reported.
      However, none of the preceding operators in the
      enpoints alter the amount of data so the resulting
      operation should be unchanged.

      amount of data (r6) will be the amount of data
      reported in the buffer.
   */

   // Compute num channels in address
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);

   // Get space at output
   r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r0];     // Output index (first channel)
   call $cbops.get_amount_ptr;
   r7 = M[r0];

   // Get data at input
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index (first channel)
   call $cbops.get_amount_ptr;
   // pointer to input amount (r0)
   r5 = M[r0];
   
   r0 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

#ifdef TODO_MITIGATE_CBOPS_DRIFT_IN_NON_TTP_USE_CASE
   // save r7 amount of space available, it will be handy to use in main function   
   M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MIN_SPACE_AVAILABLE_FIELD] = r7;
#endif

   // sanity check a few of pointers that must be valid on each call. If they are NULL, something
   // horrid happened - at least for such cases when e.g. endpoint passed down half-cooked params,
   // it can avoid long debug sessions at a small 'cost'. We check some ptrs here, because things take
   // lots of different routes later when they use these. Others that are not used in every logical branch
   // under the Sun are checked at referencing points (once).
   r1 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.TOTAL_INSERTS_PTR_FIELD];
   if Z call $error;

   r1 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
   if Z call $error;

   /* Save Buffer table pointer */
   push r4;
#ifdef CBOPS_DEBUG_LOGGING
   // log input amount and rm_diff at this point in time
   pushm <r0, r1, r2>;
   r2 = r0;
   r0 = M[$cbops.debug_logidx];
   Null = r0 - ($cbops.DEBUG_LOG_SIZE - 3);
   if GT jump nolog;
   r1 = 0xDDDDDD;
   M[r0 + $cbops.debug_log] = r1;
   r0 = r0 + 1;
   M[r0 + $cbops.debug_log] = r6;
   r0 = r0 + 1;
   // this ptr may be duff, but gets checked when used for real a bit later and throws
   // toys out of the pram if it is null.
   r1 = M[r2 + $cbops_underrun_comp_op.underrun_comp_struct.RM_DIFF_PTR_FIELD];
   r1 = M[r1];
   M[r0 + $cbops.debug_log] = r1;
   r0 = r0 + 1;
   M[$cbops.debug_logidx] = r0;
   nolog:
   popm <r0, r1, r2>;
#endif

   // if block_size <= amount_data then it is not an underrun - so calculate various magic amounts
   r1 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.BLOCK_SIZE_PTR_FIELD];
   if Z call $error;

   r1 = M[r1];
   r2 = r1 - r6;
   if LE jump calc_amounts;
      M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD] = r2;

#ifdef CBOPS_DEBUG_LOGGING
   // log underrun situation, with number of insertions that are needed
   pushm <r0, r1>;
   r0 = M[$cbops.debug_logidx];
   Null = r0 - ($cbops.DEBUG_LOG_SIZE - 2);
   if GT jump nolog2;
   r1 = 0xBBBBBB;
   M[r0 + $cbops.debug_log] = r1;
   r0 = r0 + 1;
   M[r0 + $cbops.debug_log] = r2;
   r0 = r0 + 1;
   M[$cbops.debug_logidx] = r0;
   nolog2:
   popm <r0, r1>;
#endif

      // 1.) UNDERRUN - so memorise the needed insertions for the chorus of channels
      // (could even be full block_size, if data is totally missing in momentary stall or
      // while we haven't started to receive any data yet)

      push r1;

      // add to total number of insertions
      r1 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.TOTAL_INSERTS_PTR_FIELD];
      r3 = M[r1];

      r3 = r3 + r2;
      M[r1] = r3;
      pop r1;

      // check whether we got out of waiting for start state.
      r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
      Null = M[r2];

      // 1.1.) if not started yet, then calculate a bunch of housekeeping updates
      if Z jump calc_updates;

      // 1.2.) else: we have started to receive data, so now count stalls if we got no data at all
      // this time round - it could be a prolonged stall, in which case we enter "not started" mode
      // and wipe out various stats that will not be compensated for.
      r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.NR_CONSECUTIVE_STALLS_FIELD];
      r2 = r2 + 1;

      // if amount_data > 0, reset the stall count
      Null = r6;
      if GT r2 = Null;

      // if consecutive stall count is larger than preset max number, then revert to "not yet started" state.
      // Something stalled so badly (e.g. pause in a stream) that we will not deal with the accumulating
      // insertions, we act as if we have not yet started up.
      Null = r2 - $cbops.underrun_comp_op.MAX_CONSECUTIVE_STALLS;

      // 1.2.1.) if not above max stall count, just updated the counter, it is "just" a dropout of an entire block
      if LT jump update_stalls;

      // 1.2.2.) else: we are above max consecutive stall count, so enter not yet started state.
      // The consecutive stall count is reset, and rm_diff is set to zero -
      // we begin to wait again for some data to arrive across all channels.
      r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
      M[r2] = Null;
      r2 = Null;

    update_stalls:
      M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.NR_CONSECUTIVE_STALLS_FIELD] = r2;
      jump calc_updates;

   // 2.) NOT UNDERRUN - calculate amounts needed by main()
   calc_amounts:

      // now start to calculate the amount to use for the chorus of channels
      M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.NR_CONSECUTIVE_STALLS_FIELD] = Null;

      /* Is this first time we don't have under-run? If yes, so data started to arrive, reset underrun count, too.
       * We can NOT tell if some of the underruns at the end of a continuous series of underruns were genuine
       * misses or just part of start-up vacuum. But as data starts to come in, later genuine underruns would
       * have corresponding backlogs arriving eventually (unless streaming case of a pause),
       * so at that point, when discard conditions are met, data will be thrown away. Previous amount defaults to
       * block size, as we had full blocks of silences put in so far.
       */
      Null = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.START_NEXT_KICK_FIELD];
      if Z jump check_started_state;

         // 2.1.) If deferred start is happening now: reset next "kick" start flag
         M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.START_NEXT_KICK_FIELD] = Null;

         // put any non-zero value into the "started" flag to make it TRUE
         r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
         M[r2] = r0;

         // zero the insertion count and amount
         r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.TOTAL_INSERTS_PTR_FIELD];
         M[r2] = Null;

         M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD] = Null;

#ifdef CBOPS_DEBUG_LOGGING
         // log started state beginning here
         pushm <r0, r1>;
         r0 = M[$cbops.debug_logidx];
         Null = r0 - ($cbops.DEBUG_LOG_SIZE - 1);
         if GT jump nolog10;
         r1 = 0xEEEEEE;
         M[r0 + $cbops.debug_log] = r1;
         r0 = r0 + 1;
         M[$cbops.debug_logidx] = r0;
         nolog10:
         popm <r0, r1>;
#endif

         jump calc_updates;

    // 2.2.) else: Check started state of the chain, deferred start flag is OFF

    check_started_state:
      // get sync start flag value
      r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
      Null = M[r2];

      if NZ jump normal_ops;

      /* If data has started flowing but it isn't an integer multiple of the block size then wait
       * for a single kick period to ensure that there won't be an underrun before further data arrives.
       */
      // a cheap module calculation - cheaper than actual divide, considering ratio of data amount and
      // block size that is at work in reality.
      r2 = r6;

    calc_modulo:
      // r1 = block size
      r2 = r2 - r1;
      if GT jump calc_modulo;

      // if amount_data % block_size != 0 then we start up, otherwise we wait until next round
      if Z jump started_state;

          // 2.2.1.) if we wait until next kick - deferred start flag set to TRUE via some non-zero value
          M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.START_NEXT_KICK_FIELD] = r0;

#ifdef CBOPS_DEBUG_LOGGING
          // log "next kick start" deferring of started state
          pushm <r0, r1>;
          r0 = M[$cbops.debug_logidx];
          Null = r0 - ($cbops.DEBUG_LOG_SIZE - 1);
          if GT jump nolog3;
          r1 = 0xDADADA;
          M[r0 + $cbops.debug_log] = r1;
          r0 = r0 + 1;
          M[$cbops.debug_logidx] = r0;
          nolog3:
          popm <r0, r1>;
#endif

          // update sum total of insertions done (discards will try to reduce this later on)
          r3 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.TOTAL_INSERTS_PTR_FIELD];
          r2 = M[r3];

          r2 = r2 + r1;

          M[r3] = r2;

          // prev_amount and max_insert is full block_size worth of data
          M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.PREV_AMOUNT_FIELD] = r1;
          M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD] = r1;

          jump calc_updates;

    // 2.2.2.) else: transitioning to started state across all channels
    started_state:
          // started flag = TRUE via some non-zero value
          r3 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
          M[r3] = r0;

#ifdef CBOPS_DEBUG_LOGGING
          // log data started state when amount_data % block size == 0
          pushm <r0, r1>;
          r0 = M[$cbops.debug_logidx];
          Null = r0 - ($cbops.DEBUG_LOG_SIZE - 1);
          if GT jump nolog4;
          r1 = 0xDEDEDE;
          M[r0 + $cbops.debug_log] = r1;
          r0 = r0 + 1;
          M[$cbops.debug_logidx] = r0;
          nolog4:
          popm <r0, r1>;
#endif

          r3 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.TOTAL_INSERTS_PTR_FIELD];
          M[r3] = Null;

          // no inserts, prev_amount init to block size value
          M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD] = Null;
          M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.PREV_AMOUNT_FIELD] = r1;

          jump calc_updates;

    normal_ops:
      /* 3.) This is normal operation or we've exited a special case
       * condition then clear underrun_amount as there is no more underrun
       */
      M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD] = Null;

      // calculate rm_diff and final "amount to use" stuff. Also calculate things that discard cbop
      // will find helpful, like estimation of the data lumps that are arriving.

    calc_updates:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbops.underrun_comp.amount_to_use.PATCH_ID_1, r3)
#endif

    /* 3.1.) Update and keep a kick-to-kick "maximum" of deltas of meaningful data amounts (considering that:
     * we may have inserted samples, kick is consuming an audio sink block worth).
     */
    r3 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.PREV_AMOUNT_FIELD];
    r4 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.DATA_BLK_UPDATES_FIELD];
    r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SUM_DATA_BLKS_FIELD];

    // r3 = amount_data - prev_amount
    r3 = r6 - r3;

    // if prev_amount <= amount_data (so we got some lump of data), update data block size cumulative count
    // by (amount_data - prev_amount) + block_size
    if LT jump check_block_updates;

        // 3.1.1.) if lump of data came in, we are not just consuming existing input data, update block size cumulative count
        r2 = r2 + r3;
        r2 = r2 + r1;  // r1 = block size

        // sum_data_blks = sum_data_blks + (amount_data - prev_amount) + block_size
        M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SUM_DATA_BLKS_FIELD] = r2;

#ifdef CBOPS_DEBUG_LOGGING
        // log sum of data block estimations
        pushm <r0, r1>;
        r0 = M[$cbops.debug_logidx];
        Null = r0 - ($cbops.DEBUG_LOG_SIZE - 2);
        if GT jump nolog5;
        r1 = 0xFAFAFA;
        M[r0 + $cbops.debug_log] = r1;
        r0 = r0 + 1;
        M[r0 + $cbops.debug_log] = r2;
        r0 = r0 + 1;
        M[$cbops.debug_logidx] = r0;
        nolog5:
        popm <r0, r1>;
#endif

        // increment count of cumulative block size updates
        r4 = r4 + 1;
        M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.DATA_BLK_UPDATES_FIELD] = r4;

    check_block_updates:
        // 3.1.2.) else if number of data_block_updates >= NR_DATA_BLOCK_UPDATES, proceed to an averaging to yield an
        // estimate of arriving data lumps' pattern. This gets used by discarding logic, and as long as the
        // estimate doesn't undershoot, it should not dig itself into a hole by discarding too much and then
        // causing itself a neat underrun.
        Null = r4 - $cbops.underrun_comp_op.NR_DATA_BLOCK_UPDATES;
        if LT jump prev_amount_update;

           /* Get average of perceived input amounts, and reset the tracking vars to start again */
           r2 = r2 ASHIFT (- $cbops.underrun_comp_op.LOG2_NR_DATA_BLOCK_UPDATES);

           r3 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.DATA_BLOCK_SIZE_PTR_FIELD];
           if Z call $error;
           M[r3] = r2;

           M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.DATA_BLK_UPDATES_FIELD] = Null;
           M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.SUM_DATA_BLKS_FIELD] = Null;

#ifdef CBOPS_DEBUG_LOGGING
           // log the estimated data block size
           pushm <r0, r1>;
           r0 = M[$cbops.debug_logidx];
           Null = r0 - ($cbops.DEBUG_LOG_SIZE - 2);
           if GT jump nolog6;
           r1 = 0xFAFFFF;
           M[r0 + $cbops.debug_log] = r1;
           r0 = r0 + 1;
           M[r0 + $cbops.debug_log] = r2;
           r0 = r0 + 1;
           M[$cbops.debug_logidx] = r0;
           nolog6:
           popm <r0, r1>;
#endif

    prev_amount_update:
      /* 3.1.3.) else this kick leaves behind the amount of input data we saw when kicked, and possibly some
       * extra silence samples we have to add.
       */

      // prev_amount = amount_data + underrun_amount
      r2 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD];
      r2 = r6 + r2;
      M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.PREV_AMOUNT_FIELD] = r2;


      //  4.) --- now proceed to figuring out final data amount verdicts and rm_diff updates ---
      // NOTE: This used to be preparatory step before cbops copy was called at endpoint level in the
      // endpoint kick routine.

      r3 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.RM_DIFF_PTR_FIELD];
      if Z call $error;
      r2 = M[r3];

      r4 = r2;
      if LT r4 = Null - r2;
      Null = r4 - r1; // r1 = block size

      // 4.1.) if ABS(rm_diff) > block_size then rm_diff = 0
      if LE jump rm_diff_ok;

         r2 = 0;
         M[r3] = Null;

    rm_diff_ok:
      // 4.2.) else rm_diff is usable, phew

      // amount_to_use = block_size - max_insert - rm_diff;
      r3 = M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD];
      r3 = r1 - r3;
      r3 = r3 - r2;

      M[r0 + $cbops_underrun_comp_op.underrun_comp_struct.AMOUNT_FIELD] = r3;

#ifdef CBOPS_DEBUG_LOGGING
      // log the "amount_use" signed value and the rm_diff updated value
      pushm <r0, r1>;
      r0 = M[$cbops.debug_logidx];
      Null = r0 - ($cbops.DEBUG_LOG_SIZE - 3);
      if GT jump nolog7;
      r1 = 0xBEBEBE;
      M[r0 + $cbops.debug_log] = r1;
      r0 = r0 + 1;
      M[r0 + $cbops.debug_log] = r3;
      r0 = r0 + 1;
      M[r0 + $cbops.debug_log] = r2;
      r0 = r0 + 1;
      M[$cbops.debug_logidx] = r0;
      nolog7:
      popm <r0, r1>;
#endif

      // 5.) Set "final" copy amount up, this is passed to cbops copy
      // copy_amount = amount_to_use < 0 ? 0 : amount_to_use;
      Null = r3;
      if LT r3 = 0;

      // r5 = min(r3, amount_data)
      r5 = MIN r3;

      // copy_amount = MIN(r5, output_amount)
      r5 = MIN r7;

#ifdef CBOPS_DEBUG_LOGGING
      // log the final copy amount value
      pushm <r0, r1>;
      r0 = M[$cbops.debug_logidx];
      Null = r0 - ($cbops.DEBUG_LOG_SIZE - 2);
      if GT jump nolog8;
      r1 = 0xCCCCCC;
      M[r0 + $cbops.debug_log] = r1;
      r0 = r0 + 1;
      M[r0 + $cbops.debug_log] = r5;
      r0 = r0 + 1;
      M[$cbops.debug_logidx] = r0;
      nolog8:
      popm <r0, r1>;
#endif

      // 6.) if zero copy amount resulted, fall through to main() as it would not be called at all for zero amount,
      // but we still need to act on the needed insertions (e.g. there is no data whatsoever, so
      // we need to insert a whole block to the output buffers otherwise they wrap).
      // A zero r5 amount returned by this function essentially prohibits the chain from
      // actually consuming data in the subsequent call to main() functions. However, this operator
      // still needs to do things in that case, too... so we need to circumvent the checks that
      // make cbop main() functions not be called if amount is zero.

      /* Restore buffer table pointer */
      pop r4; 

      // Save r5 as input amount
      r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
      call $cbops.get_amount_ptr;
      // 6.1.) if main will be properly called by cbops framework, just return, all done
      //       if zero output space, force processing so main is called.
      M[r0]=r5;
      if Z call $cbops.force_processing;

      pop rlink;
      rts;

// *****************************************************************************
// MODULE:
//    $cbops.underrun_comp.main
//
// DESCRIPTION:
//    Operator that compensates for underrun by inserting samples, in sync across
//    all of its channels.
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-4, r9, I0, L0, DoLoop, M3
//
// *****************************************************************************

$cbops.underrun_comp_op.main:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.underrun_comp.main.PATCH_ID_0, r1)
#endif

   push rLink;

   // start profiling if enabled
#ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_underrun_comp_op[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_underrun_comp_op;
      call $profiler.start;
#endif
   // get number of input channels - transform it into addresses, so that we don't do the latter
   // for every channel. But let's keep calling it subsequently the "number of channels"
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);

   // Get amount of data current written to output buffers
   r5 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r5];
   call $cbops.get_amount_ptr;
   r10 = M[r0];

   r0 = r10;
   Words2Addr(r0);
   M3 = r0;

   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // pointer to rm_diff value
   r2 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.RM_DIFF_PTR_FIELD];

   // 2.) take actions depending on whether we are in started state or not (i.e. have we had data arriving already or not)
   r3 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.SYNC_START_PTR_FIELD];
   Null = M[r3];

   if Z jump not_started;

   // 2.1.) we are in started state: get the signed amount_to_use (its abs() value was the copy amount "verdict"
   // calculated in amount_to_use()
      r1 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.AMOUNT_FIELD];

   // rm_diff = previous op's written_amount - amount_to_use
   // NOTE: strictly speaking, the written amount here is what this op was given to process,
   // so we find the "amount written" from r10 passed to this op (framework set it based on
   // previous operator's written amount).
   r1 = r10 - r1;

   // get old rm_diff value
   r0 = M[r2];

   // check delta_samples ptr and if not NULL, the parameter value
      r3 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.DELTA_SAMPLES_PTR_FIELD];

   // if delta samples ptr is NULL, then it is not in use at all - we act as if it's non-zero
   // (e.g. Hydra case).
   Null = r3;
   if Z jump delta_samples_unused;

      // is delta_samples non-zero?
      // 2.1.1.) if yes, we need some tweaking to do, otherwise skip any rm_diff tweaking
      Null = M[r3];
      if Z jump out_ptr_didnt_move;

    delta_samples_unused:

      // ######## store updated rm_diff value  --- ### only if delta_samples is non-zero! (so always on Hydra, not always on BC)
      M[r2] = r1;

      // if old rm_diff wasn't zero, then tweak things
      Null = r0;
      if Z jump rm_updated;

         // adjust prev_amount if there were changes in rm_diff,
         // prev_amount_tweak = old rm_diff - new rm_diff value
         r0 = r0 - r1;

         // prev_amount = prev_amount + prev_amount_tweak
         r1 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.PREV_AMOUNT_FIELD];
         r1 = r1 + r0;
         M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.PREV_AMOUNT_FIELD] = r1;

         jump rm_updated;

      out_ptr_didnt_move:
      // 2.1.2.) else the output pointer didn't move.
      // This probably means the PCM clock has stopped. Don't try to compensate for missing samples,
      // so wipe rm_diff value. In a sense, this is the same situation to not having even started yet,
      // so just a separate descriptive label for this logic branch that falls into not_started below.

 // 2.2.) else we have not yet started in terms of data arrivals to process
 not_started:
   // rm_diff = 0
   M[r2] = Null;

 // 3.) Check whether insertions are needed or not
 rm_updated:

#ifdef TODO_MITIGATE_CBOPS_DRIFT_IN_NON_TTP_USE_CASE
   Null = r10;
   if Z jump normal_under_run_process;

   // get the size of first output buffer (in r5)
   r5 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r5];
   call $cbops.get_buffer_address_and_length;
   r5 = r1;
   Addr2Words(r5);

   // get the amount of data already in the buffer (r6)
   r6 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.MIN_SPACE_AVAILABLE_FIELD];
   r6 = r5 - r6;
   r6 = r6 - 1;
   if NEG call $error;
   // plus amount of data after this copy
   r6 = r6 + r10;

   // get average copy amount
   r2 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.BLOCK_SIZE_PTR_FIELD];
   r2 = M[r2];

   // should be at least two extra samples when leaving this ISR
   r3 = r2 - r6;
   r3 = r3 + 2;
   if NEG jump fullness_process_end;

   // Is nearly empty, this shouldn't happen
   // but we add a good amount of silence so it wont happen
   // again, at least for long time
   r2 = r2 * 3 (int);
   r2 = r2 LSHIFT -1;  // 1.5x the average size
   r5 = r5 - 1;
   r2 = MIN r5;       // up to the size of buffer-1
   r3 = r2 - r6;      // samples to insert

   // limit to the amount space we have
   r2 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.MIN_SPACE_AVAILABLE_FIELD];
   r2 = r2 - r10; // amount space left
   r2 = r2 - 2;   // knock out 2
   r3 = MIN r2;
fullness_process_end:
   // see if we need to insert
   r6 = r3;
   if LE jump done;
   jump insert_now;
normal_under_run_process:
#endif // TODO_MITIGATE_CBOPS_DRIFT_IN_NON_TTP_USE_CASE

   // 3.1.) if we have samples to insert: do insertions as needed - we don't use number of samples to process (r10)
   // that was passed in as argument.
   r6 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.MAX_INSERT_FIELD];
   if Z jump done;


insert_now:

#ifdef CBOPS_DEBUG_LOGGING
   // log the number of insertions when non-zero inserts needed
   pushm <r0, r1>;
   r0 = M[$cbops.debug_logidx];
   Null = r0 - ($cbops.DEBUG_LOG_SIZE - 2);
   if GT jump nolog9;
   r1 = 0xAAAAAA;
   M[r0 + $cbops.debug_log] = r1;
   r0 = r0 + 1;
   M[r0 + $cbops.debug_log] = r6;
   r0 = r0 + 1;
   M[$cbops.debug_logidx] = r0;
   nolog9:
   popm <r0, r1>;
#endif

   // do the insertions for each channel with same amount
   // channel count
   r3 = 0;   

 process_channel:
      // get the output index for current channel
      r5 = r3 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
      r5 = r5 + r9;
      // Setup Output Buffer
      r0 = M[r8 + r5];    
      call $cbops.get_buffer_address_and_length;
   // get the offset to the write buffer to use. The index for the current output channel
   // is after the set of input channel indexes (not all of them may be in use, but they are
   // always present in param struct).
      I0 = r0;
      if Z jump next_channel;
      L0 = r1;
      push r2;
      pop B0;

      // Number of inserts
      r10 = r6;

      // If it is not a port advance over written data
      NULL = r1 - MK1;
      if Z jump jp_advance;
         r0 = M[I0,M3];
      jp_advance:
  
    // the last sample value written on this channel (channel-specific parameter).
    // If not grabbed somehow from previous copy operator, then use silence...
    r1 = M[r7 + $cbops_underrun_comp_op.underrun_comp_struct.INSERTION_VALS_PTR_FIELD];
    if Z jump just_zero_insert;

    r1 = r1 + r3;
    r1 = M[r1];

    jump start_inserts;

 just_zero_insert:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.underrun_comp.main.PATCH_ID_1, r1)
#endif

    r1 = 0;

 start_inserts:


    // Repeat last available sample for insertion, r10 times
    do inserts;
        M[I0, MK1] = r1;
    inserts:

 next_channel:

   // we move to next output channel
   r3 = r3 + 1*ADDR_PER_WORD;
   Null = r3 - r9;
   if LT jump process_channel;

   // zero length and base
   L0 = 0;
   push NULL;
   pop B0;


   // 3.1.2.) else it is a normal exit, we got properly called by cbops fw
   // get fw ptr and amount to process off the stack
   r5 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r5];
   call $cbops.get_amount_ptr;
   // get max_insert off the stack
   // this cbop "wrote" an amount that was amount_to_process (the amount written by
   // previous cbop) plus the max_insert amount.
   r6 = r6 + M[r0];
   // update amount_written (although this is last cbop in chain, the update of
   // output buffer write pointers are to happen based on this amount - so
   // pass the total amount to cbops.process and it will update output cbuffer
   // structs accordingly. In ye older dayes, endpoint had to do this per channel
   // in two steps from "above").
   M[r0]=r6;
 
   // 3.2.) else the hopefully normal case: nothing had to be inserted, we are done.
  done:
   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_underrun_comp_op;
      call $profiler.stop;
   #endif
   pop rLink;
   rts;

.ENDMODULE;


