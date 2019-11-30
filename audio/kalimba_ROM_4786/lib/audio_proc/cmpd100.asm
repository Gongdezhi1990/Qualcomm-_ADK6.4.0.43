// *****************************************************************************
// Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "core_library.h"
#include "math_library.h"
#include "cmpd100.h"
#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif

// *****************************************************************************
// MODULE:
//    $M.cmpd100
//
// DESCRIPTION:
//    Contains variable with version info
//
// INPUTS
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    N/A
//
// NOTES:
//    Versioning of all shared modules needs to be made consistent
// **********************************************************************
.MODULE $M.cmpd100;

#ifdef BLD_PRIVATE
   .PRIVATE;
#endif

   .DATASEGMENT DM;

   .VAR version = $CMPD100_VERSION;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $M.cmpd100_analysis
//
// DESCRIPTION:
//   Function to copy two channels of data from one buffer into another.
//
// INPUTS:
//   r8 -  data object containing pointers to input and output data.  Also
//         contains the number of samples to process.
//
// OUTPUTS:
//   - none
//
// *****************************************************************************

.MODULE $M.cmpd100.analysis;
 .CODESEGMENT AUDIO_PROC_CMP100_ANALYSIS_PM;
 .DATASEGMENT DM;

 .VAR/DM MusicComp_Peak;
 .VAR/DM MusicComp_Peak_log;
 .VAR/DM region;
 .VAR/DM inst_gain;

$cmpd100.analysis:


    // check if this algorithm is to be bypassed
    r0 = M[r8 + $cmpd100.OFFSET_CONTROL_WORD];
    r1 = M[r8 + $cmpd100.OFFSET_ENABLE_BIT_MASK];
    r1 = r1 AND r0;
    if NZ rts;

  push rLink;

  // **** Get Input Buffers ****
    r0 = M[r8 + $cmpd100.OFFSET_INPUT_CH1_PTR];
#ifdef BASE_REGISTER_MODE
    call $frmbuffer.get_buffer_with_start_address;
    push r2;
    pop  B0;
#else
    call $frmbuffer.get_buffer;
#endif
    I0  = r0;
    L0  = r1;

    r0 = M[r8 + $cmpd100.OFFSET_INPUT_CH2_PTR];
#ifdef BASE_REGISTER_MODE
    call $frmbuffer.get_buffer_with_start_address;
    push r2;
    pop  B4;
#else
    call $frmbuffer.get_buffer;
#endif
    I4  = r0;
    L4  = r1;

  // Use input frame size
    r10 = r3;

    r2 = 0;
    do loop_find_peak;
        r0 = M[I0,MK1], r1 = M[I4,MK1];

        // r0 = abs(in_left(i))
      Null = r0 - Null;
        if NEG r0 = -r0;

        // r0 = abs(in_left(i))
      Null = r1 - Null;
        if NEG r1 = -r1;

        // Null = abs(in_left(i)) - abs(in_right(i))
        Null = r0 - r1;
        // r0 = max(abs(in_left(i)),abs(in_right(i)))
        if NEG r0 = r1;

        // Null = peak - max(abs(in_left(i)),abs(in_right(i)))
      Null = r2 - r0;
        // r2 = peak
    if NEG r2 = r0;
    loop_find_peak:

    M[MusicComp_Peak] = r2;

    rMAC = r2;
    // output in r0
    call $math.log2_table;
    M[MusicComp_Peak_log] = r0;
    r2 = r0;

  // Clear L registers
    L0 = 0;
    L4 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;
   B4 = M[SP-MK1];
   pop  B0;
#endif

  // get data from data object
  I3 = r8 + $cmpd100.OFFSET_NEG_ONE;
  I4 = r8 + $cmpd100.OFFSET_EXPAND_CONSTANT;
  M1 = MK1;

  r3 = M[r8 + $cmpd100.OFFSET_LIMIT_THRESHOLD];
  r4 = M[r8 + $cmpd100.OFFSET_COMPRESS_THRESHOLD];
  r5 = M[r8 + $cmpd100.OFFSET_LINEAR_THRESHOLD];
  r6 = M[r8 + $cmpd100.OFFSET_EXPAND_THRESHOLD];

  // r0 = InputLvl - LimitThreshold
  r0 = r2 - r3;
  if NEG jump CompressComparison;
    r1 = M[r8 + $cmpd100.OFFSET_INV_LIMIT_RATIO];
    // rMAC = (InputLvl - LimitThreshold)*(1/LimitRatio) r1 = ExpandConstant
    // r4 = -1.0
    // I4 = $cmpd100.OFFSET_EXPAND_CONSTANT
    rMAC = r0 * r1, r1 = M[I4,MK1], r4 = M[I3,MK1];
    // rMAC = (InputLvl - LimitThreshold)*(1/LimitRatio) + ExpandConstant
    // r1 = LinearConstant
    rMAC = rMAC - r1 * r4, r1 = M[I4,MK1];
    // rMAC = (InputLvl - LimitThreshold)*(1/LimitRatio) + ExpandConstant
    //      + LinearConstant
    // r1 = CompressConstant
    rMAC = rMAC - r1 * r4, r1 = M[I4,MK1];
    // rMAC = (InputLvl - LimitThreshold)*(1/LimitRatio) + ExpandConstant
    //        + LinearConstant + CompressConstant
    // r4 = pow2(-4)
    rMAC = rMAC - r1 * r4, r4 = M[I3,-MK1];

    // rMAC = (InputLvl - LimitThreshold)*(1/LimitRatio) + ExpandConstant
    //      + LinearConstant + CompressConstant + ExpandThreshold (q4.19)
    rMAC = rMAC + r6 * r4;

        // rMAC = OutputLvl - InputLvl
        rMAC = rMAC - r2 * r4;

    I0 = r8 + $cmpd100.OFFSET_LIMIT_ATTACK_TC;

    M[inst_gain] = rMAC;
    r1 = 5;
    M[region] = r1;
    jump done_comparison;
  CompressComparison:
  // r0 = InputLvl - CompressThreshold
  r0 = r2 - r4;
  if NEG jump LinearComparison;
    r1 = M[r8 + $cmpd100.OFFSET_INV_COMPRESS_RATIO];
    // rMAC = (InputLvl - CompressThreshold)*(1/CompressRatio) r1 = ExpandConstant
    // r4 = -1.0
    // I4 = $cmpd100.OFFSET_EXPAND_CONSTANT
    rMAC = r0 * r1, r1 = M[I4,MK1], r4 = M[I3,MK1];
    // rMAC = (InputLvl - CompressThreshold)*(1/ComrpessRatio) + ExpandConstant
    // r1 = LinearConstant
    rMAC = rMAC - r1 * r4, r1 = M[I4,MK1];
    // rMAC = (InputLvl - CompressThreshold)*(1/CompressRatio) + ExpandConstant
    //      + LinearConstant
    // r4 = pow2(-4)
    rMAC = rMAC - r1 * r4, r4 = M[I3,-MK1];

    // rMAC = (InputLvl - CompressThreshold)*(1/CompressRatio) + ExpandConstant
    //      + LinearConstant + ExpandThreshold (q4.19)
    rMAC = rMAC + r6 * r4;

        // rMAC = OutputLvl - InputLvl
        rMAC = rMAC - r2 * r4;

    I0 = r8 + $cmpd100.OFFSET_COMPRESS_ATTACK_TC;

    M[inst_gain] = rMAC;
    r1 = 4;
    M[region] = r1;
    jump done_comparison;
  LinearComparison:
  // r0 = InputLvl - LinearThreshold
  r0 = r2 - r5;
  if NEG jump ExpandComparison;

    r1 = M[r8 + $cmpd100.OFFSET_INV_LINEAR_RATIO];
    // rMAC = (InputLvl - LinearThreshold)*(1/LinearRatio) r1 = ExpandConstant
    // r4 = -1.0
    // I4 = $cmpd100.OFFSET_EXPAND_CONSTANT
    rMAC = r0 * r1, r1 = M[I4,MK1], r4 = M[I3,MK1];
    // rMAC = (InputLvl - LinearThreshold)*(1/LinearRatio) + ExpandConstant
    // r4 = pow2(-4)
    rMAC = rMAC - r1 * r4, r4 = M[I3,-MK1];

    // rMAC = (InputLvl - LinearThreshold)*(1/LinearRatio) + ExpandConstant
    //      + ExpandThreshold (q4.19)
    rMAC = rMAC + r6 * r4;

        // rMAC = OutputLvl - InputLvl
        rMAC = rMAC - r2 * r4;

    I0 = r8 + $cmpd100.OFFSET_LINEAR_ATTACK_TC;

    M[inst_gain] = rMAC;
    r1 = 3;
    M[region] = r1;
    jump done_comparison;
  ExpandComparison:
  // r0 = InputLvl - ExpandThreshold
  r0 = r2 - r6;
  if NEG jump gain_one;
    r1 = M[r8 + $cmpd100.OFFSET_INV_EXPAND_RATIO];
    // dummy index increment for I3
    r4 = M[I3,MK1];
    // rMAC = (InputLvl - LinearThreshold)*(1/LinearRatio)
    // r4 = pow2(-4)
    rMAC = r0 * r1, r4 = M[I3,-MK1];
    // rMAC = (InputLvl - ExpandThreshold)*(1/ExpandRatio) + ExpandThreshold (q4.19)
    rMAC = rMAC + r6 * r4;

        // rMAC = OutputLvl - InputLvl
        rMAC = rMAC - r2 * r4;

    I0 = r8 + $cmpd100.OFFSET_EXPAND_ATTACK_TC;

    M[inst_gain] = rMAC;
    r1 = 2;
    M[region] = r1;
    jump done_comparison;
  gain_one:
    I0 = r8 + $cmpd100.OFFSET_LINEAR_ATTACK_TC;
    rMAC = 0;
    M[inst_gain] = rMAC;
    r1 = 1;
    M[region] = r1;
  done_comparison:
    // I4 = &MusicCompGain
    r4 = M[r8 + $cmpd100.OFFSET_GAIN_PTR];
    I4 = r4;
    // r1 = MusicCompGain r2 = Attack_tc
    r1 = M[I4,0], r2 = M[I0,MK1];
    // r0 = Inst_Gain - MusicCompGain r3 = Decay_tc
    r0 = rMAC - r1, r3 = M[I0,M1];
    // if InstGain > MusicCompGain --> use attack coefficients
    if NEG r3 = r2;
    // rMAC = (Inst_Gain - MusicCompGain) * tc
    // r4 = -1.0
    rMAC = r0 * r3, r4 = M[I3,0];
    // rMAC = (Inst_Gain - MusicCompGain) * tc + MusicComp_Gain
    rMAC = rMAC - r1 * r4;
    // store gain
    M[I4,0] = rMAC;
   jump $pop_rLink_and_rts;
.ENDMODULE;

.MODULE $M.cmpd100.initialize;
 .CODESEGMENT AUDIO_PROC_CMP100_INITIALIZE_PM;

$cmpd100.initialize:

// get data from data object
  r0 = M[r8 + $cmpd100.OFFSET_EXPAND_THRESHOLD];
  r1 = M[r8 + $cmpd100.OFFSET_LINEAR_THRESHOLD];
  r2 = M[r8 + $cmpd100.OFFSET_COMPRESS_THRESHOLD];
  r3 = M[r8 + $cmpd100.OFFSET_INV_EXPAND_RATIO];
  r4 = M[r8 + $cmpd100.OFFSET_INV_LINEAR_RATIO];

  r5 = r1 - r0;         // r5 = LinearThreshold - ExpandThreshold
  rMAC = r5 * r3;       // rMAC = (LinearThreshold - ExpandThreshold)*(1/ExpandRatio)

  M[r8 + $cmpd100.OFFSET_EXPAND_CONSTANT] = rMAC;

  r5 = r2 - r1;         // r5 = CompressThreshold - LinearThreshold
  rMAC = r5 * r4;       // rMAC = (CompressThreshold - LinearThreshold)*(1/LinearRatio)

  M[r8 + $cmpd100.OFFSET_LINEAR_CONSTANT] = rMAC;

  r3 = M[r8 + $cmpd100.OFFSET_LIMIT_THRESHOLD];
  r4 = M[r8 + $cmpd100.OFFSET_INV_COMPRESS_RATIO];
  r5 = r3 - r2;             // r5 = LimitThreshold - CompressThreshold
  rMAC = r5 * r4;           // rMAC = (LimitThreshold - CompressThreshold)*(1/CompressRatio)

  M[r8 + $cmpd100.OFFSET_COMPRESS_CONSTANT] = rMAC;

  r1 = 0;
  r4 = M[r8 + $cmpd100.OFFSET_GAIN_PTR];
  M[r4] = r1;

  rts;
.ENDMODULE;

.MODULE $M.cmpd100.applygain;
 .CODESEGMENT AUDIO_PROC_CMP100_APPLYGAIN_PM;
 .DATASEGMENT DM;

 .VAR/DM lin_gain;

$cmpd100.applygain:

    // check if this algorithm is to be bypassed
    r0 = M[r8 + $cmpd100.OFFSET_CONTROL_WORD];
    r1 = M[r8 + $cmpd100.OFFSET_ENABLE_BIT_MASK];
    r1 = r1 AND r0;
    if NZ rts;

  push rLink;

  // **** Get Input Buffers ****
    r0 = M[r8 + $cmpd100.OFFSET_INPUT_CH1_PTR];
#ifdef BASE_REGISTER_MODE
    call $frmbuffer.get_buffer_with_start_address;
    push r2;
    pop  B0;
#else
    call $frmbuffer.get_buffer;
#endif
    I0  = r0;
    L0  = r1;

    r0 = M[r8 + $cmpd100.OFFSET_INPUT_CH2_PTR];
#ifdef BASE_REGISTER_MODE
    call $frmbuffer.get_buffer_with_start_address;
    push r2;
    pop  B4;
#else
    call $frmbuffer.get_buffer;
#endif
    I4  = r0;
    L4  = r1;

  // Use input frame size
    r10 = r3;

  // **** Get output buffers *****

  // Update output frame size from input
    r0 = M[r8 + $cmpd100.OFFSET_OUTPUT_CH1_PTR];
    call $frmbuffer.set_frame_size;
#ifdef BASE_REGISTER_MODE
    call $frmbuffer.get_buffer_with_start_address;
    push r2;
    pop  B1;
#else
    call $frmbuffer.get_buffer;
#endif
    I1 = r0;
    L1 = r1;

  // Update output frame size from input
    r0 = M[r8 + $cmpd100.OFFSET_OUTPUT_CH2_PTR];
  r3 = r10;
    call $frmbuffer.set_frame_size;
#ifdef BASE_REGISTER_MODE
    call $frmbuffer.get_buffer_with_start_address;
    push r2;
    pop  B5;
#else
    call $frmbuffer.get_buffer;
#endif
    I5 = r0;
    L5 = r1;

    r2 = M[r8 + $cmpd100.OFFSET_MAKEUP_GAIN];
    r3 = M[r8 + $cmpd100.OFFSET_GAIN_PTR];
    // r0 = gain
    r0 = M[r3];

    // pow2 function is expecting a q8.16 number so we need
    // to shift the log2 value prior to computing pow2 function
    r0 = r0 ASHIFT 4;

    // subtract off integer 3 to prevent pow2 being greater
    // than 1.0. Need to shift signal after gain application
    // by 3 bits to compensate
    r0 = r0 - Qfmt_(3.0, 8);
    call $math.pow2_table;

    M[lin_gain] = r0;

    // r4 = gain * makeup_gain q4.19
    r4 = r0 * r2 (frac);

    do loop_copy_data;
      r0 = M[I0,MK1], r1 = M[I4,MK1];

      rMAC = r0 * r4;
      r0 = rMAC ASHIFT 7;
      rMAC = r1 * r4 (frac);
      r1 = rMAC ASHIFT 7;

      M[I1,MK1] = r0, M[I5,MK1] = r1;
    loop_copy_data:

// Clear L registers
    L0 = 0;
    L1 = 0;
    L4 = 0;
    L5 = 0;
#ifdef BASE_REGISTER_MODE
    push Null;
    B4 = M[SP-MK1];
  B5 = M[SP-MK1];
  B1 = M[SP-MK1];
    pop  B0;
#endif
    jump $pop_rLink_and_rts;
.ENDMODULE;
