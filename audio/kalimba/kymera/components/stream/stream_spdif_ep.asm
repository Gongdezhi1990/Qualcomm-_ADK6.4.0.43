/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_spdif_ep.asm
 * \ingroup stream
 *
 * stream spdif type file. <br>
 *
 * This file contains some asm functions used by spdif endpoint,
 * for speed reason these are done in assembly.
 *
 * Notes<TODO>:
 * These functions will finally be platform independent, currently they
 * most probably aren't. Also 24-bit input support hasn't been added yet.
 *
 * \section sec1 Contains:
 * spdif_detect_sample_rate <br>
 * spdif_copy_raw_data <br>
 * spdif_handle_pause_time <br>
 */

#ifndef STREAM_SPDIF_EP_ASM
#define STREAM_SPDIF_EP_ASM
    // DSP Rate measurement

    // history of amount of new data seen in input port
    // the equivalent time length will be the reading
    // period (normally 1ms) times history length. Increasing
    // this value would achieve more accurate measurement
    // but would need more memory and computations, it also
    // increases the detection lag when switching from one
    // rate to another rate
   .CONST $spdif.RATE_DETECT_AMOUNT_HIST_LENGTH 40;

   // history of detected rates, each time the rate
   // detect is called it would measure the input
   // rate, and keeps a short history of last detected
   // rates. It uses a hysteresis mechanism to switch
   // to/from a rate.
   .CONST $spdif.RATE_DETECT_RATE_HIST_LENGTH   15;
   .CONST $spdif.RATE_DETECT_VALID_THRESHOLD    11;   // to switch to new valid rate
   .CONST $spdif.RATE_DETECT_INVALID_THRESHOLD   6;   // to switch from valid rate to invalid
                                                      // sum of two thresholds must be greater than hist length

    // the accuracy that is needed when measuring the rate
    // the using application shall be able to cope with
    // twice of this amount of mismatch
   .CONST $spdif.RATE_DETECT_ACCURACY        0.005;   // 0.5% tolerance + 0.5% jitter

   // minimum inactive time before pause is detected
   .CONST $spdif.PAUSE_DETECTION_THRESHOLD   4000;

   .CONST $spdif_ep.STREAM_VALID_FIELD                  0*ADDR_PER_WORD;
   .CONST $spdif_ep.NROF_SUPPORTED_RATES_FIELD          1*ADDR_PER_WORD;
   .CONST $spdif_ep.SUPPORTED_RATES_PTR_FIELD           2*ADDR_PER_WORD;
   .CONST $spdif_ep.SUPPORTED_RATES_MASK_FIELD          3*ADDR_PER_WORD;
   .CONST $spdif_ep.DSP_SAMPLING_FREQ_FIELD             4*ADDR_PER_WORD;
   .CONST $spdif_ep.READ_INTERVAL_FIELD                 5*ADDR_PER_WORD;
   .CONST $spdif_ep.TIME_IN_PAUSE_FIELD                 6*ADDR_PER_WORD;
   .CONST $spdif_ep.PAUSE_STATE_FIELD                   7*ADDR_PER_WORD;
   .CONST $spdif_ep.MEASURED_SAMPLING_FREQ_FIELD        8*ADDR_PER_WORD;
   .CONST $spdif_ep.SILENCE_ADDED_RES_FIELD             9*ADDR_PER_WORD;
   .CONST $spdif_ep.NORM_RATE_RATIO_FIELD               10*ADDR_PER_WORD;
   .CONST $spdif_ep.NORM_RATE_RATIO_MS_FIELD            11*ADDR_PER_WORD;
   .CONST $spdif_ep.NORM_RATE_RATIO_LS_FIELD            12*ADDR_PER_WORD;
   .CONST $spdif_ep.RATE_DETECT_HIST_INDEX_FIELD        13*ADDR_PER_WORD;
   .CONST $spdif_ep.RATE_DETECT_HIST_FIELD              14*ADDR_PER_WORD;
#include "stack.h"

   .CONST $spdif_ep.pause_state.NOT_IN_PAUSE  0; // ep isn't in pause mod
   .CONST $spdif_ep.pause_state.PAUSE_IDLE    1; // ep is in pause but no silence is being inserted
   .CONST $spdif_ep.pause_state.ACTIVE_PAUSE  2; // ep is in pause state and silence is being inserted
#include "patch/patch_asm_macros.h"
// *****************************************************************************
// MODULE:
//    $_spdif_reset_ep_lib
//
// DESCRIPTION:
//    resets endpoint structure
//
// INPUTS:
//    - r0 = structure
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.spdif_reset_ep_lib;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$_spdif_reset_ep_lib:

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   M[r0 + $spdif_ep.PAUSE_STATE_FIELD] = 0;
   M[r0 + $spdif_ep.SILENCE_ADDED_RES_FIELD] = 0;

   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_spdif_detect_sample_rate
//
// DESCRIPTION:
//    detect sample rate of spdif stream
//
// INPUTS:
//    - r0 = new samples just arrived
//    - r1 = pointer to spdif structure
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.spdif_detect_sample_rate;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;
$_spdif_detect_sample_rate:

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C


   // structure
   r8 = r1;
   // new amount of data arrived
   r7 = r0;

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($spdif_ep.SPDIF_DECODE_EP_ASM.SPDIF_DETECT_SAMPLE_RATE.PATCH_ID_0, r1)

   M0 = ADDR_PER_WORD;
   M1 = -M0;
   // compute the moving average of the
   // amount of data seen in the input
   I2 = r8 + $spdif_ep.RATE_DETECT_HIST_FIELD;
   r1 = 255;
   r1 = MIN r7;
   // invalid if no data in 3 consecutive read
   r0 = M[I2, M0];   // amount[n-1]
   r3 = M[I2, M1];  // amount[n-2]
   r3 = r3 AND 0xFF;
   r2 = r0 AND 0xFF;
   r2 = r2 + r3;
   r2 = r1 + r2;    // r2 = amount[n]+amount[n-1]+amount[n-2]
   if NZ jump input_received;
   // invalid sample rate
   M[r8 + $spdif_ep.MEASURED_SAMPLING_FREQ_FIELD] = 0;

   // clear amount history
   M[r8 + $spdif_ep.RATE_DETECT_HIST_INDEX_FIELD] = 0;
   // input stream is invalid because of no data
   r4 = 0;
   jump set_rate;

   input_received:
   // we have received input
   Null = M[r8 + $spdif_ep.RATE_DETECT_HIST_INDEX_FIELD];
   if NZ jump reset_done;

   // reset the rate history
   r2 = 0;
   I2 = r8 + $spdif_ep.RATE_DETECT_HIST_FIELD;
   r10 = $spdif.RATE_DETECT_RATE_HIST_LENGTH + $spdif.RATE_DETECT_AMOUNT_HIST_LENGTH;
   do reset_rate_loop;
      M[I2, M0] = r2;
   reset_rate_loop:
   reset_done:
   // input and its time is stored in
   // the hist buffer: 0xttttmm
   // mm: amount received
   // tttt: diff time (16-bit)
   r4 = M[r8 + $spdif_ep.READ_INTERVAL_FIELD];
   if NEG r4 = -r4;
   I2 = r8 + $spdif_ep.RATE_DETECT_HIST_FIELD;
   r10 = M[r8 + $spdif_ep.RATE_DETECT_HIST_INDEX_FIELD];
   r2 = 0;
   r3 = r4 LSHIFT 8;
   r3 = r3 OR r1;
   r5 = 0;
   do calc_total_loop;
      // accumulate received values, and diff times
      // r2: values
      // r5: diff times
      r3 = r0, M[I2, M0] = r3;
      r2 = r2 + r1, r0 = M[I2, 0];
      r5 = r5 + r4;
      r1 = r3 AND 0xFF;
      r4 = r3 LSHIFT -8;
   calc_total_loop:
   r1 = M[r8 + $spdif_ep.RATE_DETECT_HIST_INDEX_FIELD];
   Null = r1 - $spdif.RATE_DETECT_AMOUNT_HIST_LENGTH;
   if POS jump calc_total_done;
      r1 = r1 + 1, M[I2, M0] = r3;
      M[r8 + $spdif_ep.RATE_DETECT_HIST_INDEX_FIELD] = r1;
   calc_total_done:
   r1 = r5;
   if Z r1 = r1 + 1;
   // r2 = total samples received
   // r1 = duration in microsecond
   // calculate fs = samples/seconds=r2*1e6/r1
   r0 = 1;
   rMAC = r2 * 2000000;
   rMAC = rMAC + r1*r0;
   r1 = r1 LSHIFT 2;
   Div = rMAC / r1;
   r2 = DivResult;
   M[r8 + $spdif_ep.MEASURED_SAMPLING_FREQ_FIELD] = r2;

   // r2 = estimated sample rate
   // the past RATE_DETECT_AMOUNT_HIST_LENGTH ms
   // find nearest rate
   r0 = M[r8 + $spdif_ep.DSP_SAMPLING_FREQ_FIELD];
   M2 = r0;
   r0 = M[r8 + $spdif_ep.SUPPORTED_RATES_PTR_FIELD];
   I2 = r0;
   r5 = M[r8 + $spdif_ep.NROF_SUPPORTED_RATES_FIELD];
   r10 = r5;
   r5 = r5 + 1;
   r6 = M[r8 + $spdif_ep.SUPPORTED_RATES_MASK_FIELD];
   r3 = 0x1<<0;
   do find_nearest_loop;
      /* get next rate */
      r1 = M[I2, M0];

      /* see if this rate is supported*/
      Null = r6 AND r3;
      if Z jump search_next;

      // check if we are in this rate
      r0 = r2 - r1;      // compute the diff
      r0 = ABS r0;       // abs diff
      r4 = r1 * $spdif.RATE_DETECT_ACCURACY(frac);
      Null = M2 - r1;    // if already in valid fs
      if Z r4 = r4 + r4; // tolerance is doubled
      Null = r0 - r4;    // see if within tolerance range
      if LE jump fs_calculated;
      search_next:
      r3 = r3 LSHIFT 1;
      find_nearest_loop:
   // no proper rate calculated
   r3 = 0;
   fs_calculated:

   // reserve some space in stack for op scratch data
   r7 = SP;
   r0 = r5 ASHIFT LOG2_ADDR_PER_WORD;
   SP = SP + r0;

   r10 = r5;
   r0 = 0;
   I3 = r7;
   do clear_temp_buf_loop;
      M[I3, M0] = r0;
   clear_temp_buf_loop:

   r3 = SIGNDET r3;
   r3 = (DAWTH-1) - r3;

   // store a short history of detected rates
   // and find the one with majority
   I2 = r8 + ($spdif_ep.RATE_DETECT_HIST_FIELD + ($spdif.RATE_DETECT_AMOUNT_HIST_LENGTH*ADDR_PER_WORD));
   r10 = $spdif.RATE_DETECT_RATE_HIST_LENGTH;
   do find_mod_loop;
      r0 = M[I2, 0];
      r1 = r3 LSHIFT LOG2_ADDR_PER_WORD;
      r2 = M[r7 + r1];
      r2 = r2 + 1;
      M[r7 + r1] = r2;
      r3 = r0, M[I2, M0] = r3;
   find_mod_loop:
   // see which rate has majority
   r2 = $spdif.RATE_DETECT_INVALID_THRESHOLD;
   r0 = M[r8 + $spdif_ep.SUPPORTED_RATES_PTR_FIELD];
   I2 = r0;
   r10 = r5 -1;
   I3 = r7;
   r0 = M[I3, M0];
   Null = r0 - $spdif.RATE_DETECT_VALID_THRESHOLD;
   if POS jump rate_decision_ended;

   do rate_decision_loop;
      // get the next sample rate
      r4 = M[I2, M0];     // get the next sample rate

      // see if this rate has majority
      r0 = $spdif.RATE_DETECT_VALID_THRESHOLD;
      Null = r4 - M2;    // lower threshold if
      if Z r0 = r2;      // already in this rate
      r1 = M[I3,M0];
      Null = r1 - r0;    // see if it has good majority
      if POS jump fs_found;
   rate_decision_loop:
   rate_decision_ended:
   r4 = 0;
   // none has the majority,
   // input stream is invalid because of inaccurate rate
   fs_found:
   // move stack pointer back
   r5 = r5 * (-ADDR_PER_WORD)(int);
   SP = SP + r5;

   set_rate:

   // default rate ratio
   rMAC = 0.5;

   // rate has changed, store new detected rate
   M[r8 + $spdif_ep.DSP_SAMPLING_FREQ_FIELD] = r4;
   if Z jump set_drift_ratio;

   // calculate measured_rate/expected_rate
   // TODO: calc 1/fs once
   rMAC = M[r8 + $spdif_ep.MEASURED_SAMPLING_FREQ_FIELD];
   r4 = r4 ASHIFT 2;
   Div = rMAC / r4;

   // smooth Averaging
   r4 = M[r8 + $spdif_ep.NORM_RATE_RATIO_MS_FIELD]; // MSW
   r5 = M[r8 + $spdif_ep.NORM_RATE_RATIO_LS_FIELD]; // LSW
   rMAC = 0.995*r5 (SU);
   rMAC = rMAC ASHIFT -DAWTH (56bit);
   rMAC = rMAC + 0.995*r4(SS);
   r3 = DivResult;
   rMAC = rMAC + r3*0.005;
   set_drift_ratio:
   r4 = rMAC1;
   r5 = rMAC0;
   rMAC = rMAC ASHIFT (24 - DAWTH) (56bit);
   M[r8 + $spdif_ep.NORM_RATE_RATIO_FIELD] = rMAC;
   M[r8 + $spdif_ep.NORM_RATE_RATIO_MS_FIELD] = r4; // MSW
   M[r8 + $spdif_ep.NORM_RATE_RATIO_LS_FIELD] = r5; // LSW
   end:
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;
.ENDMODULE;
// *****************************************************************************
// MODULE:
//    $_spdif_copy_raw_data
//
// DESCRIPTION:
//    copy raw data
//
// INPUTS:
//    - r0 = sink buffer
//    - r1 = source buffer
//    - r2 = amount to read
//    - r3 = amount to write
//    -  M[SP - 1] = amount to shift
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//   availability of data/space in the input/output buffers
//   shall be checked by the caller
// *****************************************************************************
.MODULE $M.spdif_copy_raw_data;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$_spdif_copy_raw_data:

   rMAC = M[SP -ADDR_PER_WORD]; // width

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   r7 = r2;  // amount to read
   r6 = r3;  // amount to write
   r5 = r0;  // sink buffer
   r4 = r1;  // source buffer

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($spdif_ep.SPDIF_DECODE_EP_ASM.SPDIF_COPY_RAW_DATA.PATCH_ID_0, r1)

   M0 = ADDR_PER_WORD;
#ifdef BASE_REGISTER_MODE
   push  B0;
   push  B4;
#endif

   // amount to copy
   r3 = MIN r2;

   // amount to trash
   r7 = r7 - r3;

   // amount of silence
   r6 = r6 - r3;

   // get write buffer
   r0 = r5;
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop  B4;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   I4 = r0;
   L4 = r1;

   Null = r3 + r7;
   if Z jump write_silence;

   // get read buffer
   r0 = r4;
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop  B0;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I0 = r0;
   L0 = r1;

   r10 = r3 -1;
   if NEG jump trash_input;
      r0 = M[I0, M0];
   do copy_loop;
      r0 = r0 LSHIFT rMAC;
      r0 = M[I0, M0], M[I4, M0] = r0;
   copy_loop:
   r0 = r0 LSHIFT rMAC;
   M[I4, M0] = r0;
   trash_input:
   r10 = r7;
   do trash_loop;
      r0 = M[I0, M0];
   trash_loop:

   // set left port read address
   r0 = r4;
   r1 = I0;
   call $cbuffer.set_read_address;
   L0 = 0;
   write_silence:
   r10 = r6;
   r0 = 0;
   do silence_loop;
      M[I4, M0] = r0;
   silence_loop:
   // set output buffer write address
   r0 = r5;
   r1 = I4;
   call $cbuffer.set_write_address;
   L4 = 0;

#ifdef BASE_REGISTER_MODE
   pop  B4;
   pop  B0;
#endif
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;
.ENDMODULE;
// *****************************************************************************
// MODULE:
//    $_spdif_handle_pause_time
//
// DESCRIPTION:
//    copy raw data
//
// INPUTS:
//    - r0 = silent duration
//    - r1 = pointer to spdif structure
//    - r2 = active
//    - r3 = sample rate
// OUTPUTS:
//    - amount of silence added this time
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.spdif_detect_and_handle_pause;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;
$_spdif_handle_pause_state:

$_spdif_detect_and_handle_pause:
   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   // patch point
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($spdif_ep.SPDIF_DECODE_EP_ASM.SPDIF_DETECT_AND_HANDLE_PAUSE.PATCH_ID_0, r7)

   // r1 = structure
   // r6 = pause state
   // r3 = sample rate
   r7 = 0;
   r6 = $spdif_ep.pause_state.NOT_IN_PAUSE;

   // no pause handling if the input is or has become active
   Null = r2;
   if NZ jump end;

   // get the pause data
   r6 = M[r1 + $spdif_ep.PAUSE_STATE_FIELD];
   r2 = M[r1 + $spdif_ep.READ_INTERVAL_FIELD];
   if NEG r2 = 0;
   r4 = M[r1 + $spdif_ep.TIME_IN_PAUSE_FIELD];

   // r6 = pause state
   Null = r6 - $spdif_ep.pause_state.PAUSE_IDLE;
   if Z jump p_idle;

   Null = r6 - $spdif_ep.pause_state.ACTIVE_PAUSE;
   if Z jump p_insert;

p_play:
   // no need to insert silence until we are in pause
   // for some time
   r4 = 0;
   r6 = $spdif_ep.pause_state.ACTIVE_PAUSE;
   M[r1 + $spdif_ep.SILENCE_ADDED_RES_FIELD] = 0;
p_insert:
   // insert mode
   r5 = 1.0;
   r4 = r4 + r2;
   if V r4 = r5;
   M[r1 + $spdif_ep.TIME_IN_PAUSE_FIELD] = r4;

#ifdef CHIP_BASE_BC7
   // check the detection threshold
   r5 = r4 - $spdif.PAUSE_DETECTION_THRESHOLD;
   if LE jump end;
   r2 = MIN r5;
#endif

   // see if we need to insert indefinite amount
   Null = r0;
   if NEG jump insert_silence;

   // see if we have inserted max amount
   r0 = r5 - r0;
   if NEG jump insert_silence;

   // end silence insertion and go to idle mode
   r6 = $spdif_ep.pause_state.PAUSE_IDLE;
   r2 = r2 - r0;
   if LE jump end;
   insert_silence:
   // work out how much silence must be added this time
   rMAC = M[r1 + $spdif_ep.SILENCE_ADDED_RES_FIELD];
   rMAC = rMAC LSHIFT -DAWTH (56bit);
   rMAC = rMAC + r3 * r2;
   r0 = 2000000;
   Div = rMAC/r0;
   r0 = DivRemainder;
   M[r1 + $spdif_ep.SILENCE_ADDED_RES_FIELD] = r0;
   r7 = DivResult;
p_idle:
/* no action */
end:
   M[r1 + $spdif_ep.PAUSE_STATE_FIELD] = r6;
   r0 = r7;
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;

.ENDMODULE;

#endif
