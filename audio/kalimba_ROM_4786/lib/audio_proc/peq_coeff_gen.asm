/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "core_library.h"
#include "peq_coeff_gen.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif

// *****************************************************************************
// MODULE:
//    $M.peq_gen_coeffs
//
// DESCRIPTION:
//    Generate PEQ coefficients from design criteria in OBPM params object
//
// INPUTS:
//    - r6 = sample rate
//    - r7 = $audio_proc.peq.parameter object
//           This is where the coefficients will be stored. The object must be sized to accept $audio_proc.peq_coeff_gen.NUM_STAGES parameters.
//    - r8 = pointer to input parameters with num_bands offset(OBPM parameters)
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    All, except for r6
//
//
// *****************************************************************************
.MODULE $M.peq_gen_coeffs;
    .CODESEGMENT PM;

$peq_cap_gen_coeffs:
    push rLink;
  
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.PEQ_CAP_GEN_COEFFS.PATCH_ID_0,r1)     // compander_patchers1
#endif
  
    r0 = r7;

    //calculate coefficients
    //--------------------------------------------------------------------------

    i0 = r8;            // i0 = &Cur_params+num_bands
    i1 = r0;            // i1 = address of "coefficient params" object

    // first copy number of EQ bands to coefficient storage.
    r0 = m[i0,MK1];
    m[i1,MK1] = r0;
    push r0;                // keep number of bands for further use

    // next we calculate the MasterGain of the eq bank
    call calcPreGain.call_entry;

    calcBandLoop:
        pop r0;
        r0 = r0 - 1;
        if neg jump calcBankCoefs.switchToNewCoefs;
        push r0;

        pushm <i0,i1>; // i0=&OBPM_params, i1=&coeffs_scale
        call coefCalcBiquadBand.call_entry;
        popm <r0,r1>;
        r0 = r0 + $audio_proc.peq_coeff_gen_filter_stage.STRUC_SIZE * ADDR_PER_WORD;     // OBPM parameter increment (4 params per band)
        i0 = r0;
        r1 = r1 + 6*ADDR_PER_WORD;                                                       // coefficient increment (5 coefs per band (ex. scale))
        i1 = r1;

        jump calcBandLoop;

    calcBankCoefs.switchToNewCoefs:

    pop rLink;
    rts;





calcPreGain.call_entry:
//------------------------------------------------------------------------------
    push rLink;
   
    
calcPreGain.jump_entry:

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.CALCPREGAIN_CALL_ENTRY.PATCH_ID_0,r1)     // peq_coeff_patchers1
//#endif


    // g
    r0 = m[i0,MK1];                       // gain
    // constrain parameter to specified range
    r1 = $user_eq.gain_lo_gain_limit;
    r0 = max r1;
    r1 = $user_eq.gain_hi_gain_limit;
    r0 = min r1;
    r1 = $user_eq.gain_param_scale;
    call $kal_float_lib.q_to_float;

    call $kal_filter_coef_lib.__db2lin.call_entry;
    // r0 = exponenet   [S---------------EEEEEEEE]
    // r1 = mantissa    [MMMMMMMMMMMMMMMMMMMMMMMM]

    r0 = r0 and 0x0000ff;       // expect sign to be zero, but mask off just in case
    r0 = r0 - 127;              // remove offset from exponent
    r1 = r1 lshift -1;          // shift mantissa so fits twos complement arithmetic

    m[i1,MK1] = r0;
    m[i1,MK1] = r1;

    pop rLink;
    rts;



//------------------------------------------------------------------------------
coefCalcBiquadBand.call_entry:
    push rLink;
    
coefCalcBiquadBand.jump_entry:

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.COEFCALCBIQUADBAND_CALL_ENTRY.PATCH_ID_0,r7)     // peq_coeff_patchers1
//#endif


//------------------------------------------------------------------------------
// check filter type and run appropriate coefficient calculation routine
// - The routines used contain the rts, so jump to the required routines
//------------------------------------------------------------------------------
// on entry i0 = &OBPM PEQ parameters
//          i1 = &PEQ_coeff_object + $audio_proc.peq.parameter.STAGES_SCALES + 6*band#,
//             FORMAT: <b2,b1,b0,a2,a1,SF>
//------------------------------------------------------------------------------

    r0 = m[i0,MK1];

    null = r0 - $user_eq.filter_type.bypass;
    if eq jump calcBypass.jump_entry;

    // 1st order Xpass filters use calcXP1 parameter checking and calling routine

    r7 = $kal_filter_coef_lib.calc_low_pass_1st;
    null = r0 - $user_eq.filter_type.lp_1;
    if eq jump calcXP1.jump_entry;

    r7 = $kal_filter_coef_lib.calc_high_pass_1st;
    null = r0 - $user_eq.filter_type.hp_1;
    if eq jump calcXP1.jump_entry;

    r7 = $kal_filter_coef_lib.calc_all_pass_1st;
    null = r0 - $user_eq.filter_type.ap_1;
    if eq jump calcXP1.jump_entry;

    // 2nd order Xpass filters use calcXP2 parameter checking and calling routine

    r7 = $kal_filter_coef_lib.calc_low_pass_2nd;
    null = r0 - $user_eq.filter_type.lp_2;
    if eq jump calcXP2.jump_entry;

    r7 = $kal_filter_coef_lib.calc_high_pass_2nd;
    null = r0 - $user_eq.filter_type.hp_2;
    if eq jump calcXP2.jump_entry;

    r7 = $kal_filter_coef_lib.calc_all_pass_2nd;
    null = r0 - $user_eq.filter_type.ap_2;
    if eq jump calcXP2.jump_entry;

    // 1st order Shelf filters use calcShelf1 parameter checking and calling routine

    r7 = $kal_filter_coef_lib.calc_low_shelf_1st;
    null = r0 - $user_eq.filter_type.ls_1;
    if eq jump calcShelf1.jump_entry;

    r7 = $kal_filter_coef_lib.calc_high_shelf_1st;
    null = r0 - $user_eq.filter_type.hs_1;
    if eq jump calcShelf1.jump_entry;

    r7 = $kal_filter_coef_lib.calc_tilt_1st;
    null = r0 - $user_eq.filter_type.tlt_1;
    if eq jump calcShelf1.jump_entry;

    // 2nd order Shelf filters use calcShelf2 parameter checking and calling routine

    r7 = $kal_filter_coef_lib.calc_low_shelf_2nd;
    null = r0 - $user_eq.filter_type.ls_2;
    if eq jump calcShelf2.jump_entry;

    r7 = $kal_filter_coef_lib.calc_high_shelf_2nd;
    null = r0 - $user_eq.filter_type.hs_2;
    if eq jump calcShelf2.jump_entry;

    r7 = $kal_filter_coef_lib.calc_tilt_2nd;
    null = r0 - $user_eq.filter_type.tlt_2;
    if eq jump calcShelf2.jump_entry;

    // parametric EQ is all on its own - poor little PEQ

    null = r0 - $user_eq.filter_type.peq;
    if eq jump calcPEQ.jump_entry;

    coefCalcInvalid:

    // not a valid filter type, so exit
    // - coefficients will get switched,
    //   but will be a copy of existing set with no changes

    pop rLink;
    rts;



calcBypass.jump_entry:
//------------------------------------------------------------------------------

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.CALCBYPASS_JUMP_ENTRY.PATCH_ID_0)     // peq_coeff_patchers1
//#endif


    // reserve space on stack for coefficients to be returned
    sp = sp + 6*ADDR_PER_WORD;

    call $kal_filter_coef_lib.calc_bypass;

    jump pop_biquad_coefs_and_rts;


calcXP1.jump_entry:
//------------------------------------------------------------------------------

//#if defined(PATCH_LIBS)
//  LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.CALCXP1_JUMP_ENTRY.PATCH_ID_0,r1)     // peq_coeff_patchers1
//#endif


    // reserve space on stack for coefficients to be returned
    sp = sp + 6*ADDR_PER_WORD;

    // fc
    r0 = m[i0,MK1];                       // fc
    // constrain parameter to specified range
    r1 = $user_eq.xp_2_lo_freq_limit;
    r0 = max r1;
    r1 = $user_eq.xp_2_hi_freq_limit;
    r0 = min r1;
    r1 = $user_eq.freq_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // fc

    r1 = 48000;
    r0 = r6;
    if z r0 = r1;
    call $kal_float_lib.int_to_float;
    pushm <r0,r1>;                      // fs (default to 48kHz if not currently set

    call r7;

    jump pop_biquad_coefs_and_rts;



calcXP2.jump_entry:
//------------------------------------------------------------------------------
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.CALCXP2_JUMP_ENTRY.PATCH_ID_0, r1)     // peq_coeff_patchers1
//#endif

    // reserve space on stack for coefficients to be returned
    sp = sp + 6*ADDR_PER_WORD;

    // fc
    r0 = m[i0,MK1];                       // fc
    // constrain parameter to specified range
    r1 = $user_eq.xp_2_lo_freq_limit;
    r0 = max r1;
    r1 = $user_eq.xp_2_hi_freq_limit;
    r0 = min r1;
    r1 = $user_eq.freq_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // fc

    r0 = m[i0,MK1];                     // gain is unused

    // q
    r0 = m[i0,MK1];
    // constrain parameter to specified range
    r1 = $user_eq.xp_2_lo_q_limit;
    r0 = max r1;
    r1 = $user_eq.xp_2_hi_q_limit;
    r0 = min r1;
    r1 = $user_eq.q_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // q

    r1 = 48000;
    r0 = r6;
    if z r0 = r1;
    call $kal_float_lib.int_to_float;
    pushm <r0,r1>;                      // fs (default to 48kHz if not currently set

    call r7;

    jump pop_biquad_coefs_and_rts;



calcShelf1.jump_entry:
//------------------------------------------------------------------------------

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.CALCSHELF1_JUMP_ENTRY.PATCH_ID_0, r1)     // peq_coeff_patchers1
//#endif

    // reserve space on stack for coefficients to be returned
    sp = sp + 6*ADDR_PER_WORD;

    // fc
    r0 = m[i0,MK1];                       // fc
    // constrain parameter to specified range
    r1 = $user_eq.xp_2_lo_freq_limit;
    r0 = max r1;
    r1 = $user_eq.xp_2_hi_freq_limit;
    r0 = min r1;
    r1 = $user_eq.freq_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // fc

    // g
    r0 = m[i0,MK1];                       // gain (need to sign extend)
    // constrain parameter to specified range
    r1 = $user_eq.shelf_1_lo_gain_limit;
    r0 = max r1;
    r1 = $user_eq.shelf_1_hi_gain_limit;
    r0 = min r1;
    r1 = $user_eq.gain_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // g

    r1 = 48000;
    r0 = r6;
    if z r0 = r1;
    call $kal_float_lib.int_to_float;
    pushm <r0,r1>;                      // fs (default to 48kHz if not currently set

    call r7;

    jump pop_biquad_coefs_and_rts;



calcShelf2.jump_entry:

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.CALCSHELF2_JUMP_ENTRY.PATCH_ID_0, r1)     // peq_coeff_patchers1
//#endif


    // reserve space on stack for coefficients to be returned
    sp = sp + 6*ADDR_PER_WORD;

    // fc
    r0 = m[i0,MK1];                       // fc
    // constrain parameter to specified range
    r1 = $user_eq.xp_2_lo_freq_limit;
    r0 = max r1;
    r1 = $user_eq.xp_2_hi_freq_limit;
    r0 = min r1;
    r1 = $user_eq.freq_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // fc

    // g
    r0 = m[i0,MK1];                       // gain (need to sign extend)
    // constrain parameter to specified range
    r1 = $user_eq.shelf_1_lo_gain_limit;
    r0 = max r1;
    r1 = $user_eq.shelf_1_hi_gain_limit;
    r0 = min r1;
    r1 = $user_eq.gain_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // g

    // q
    r0 = m[i0,MK1];
    // constrain parameter to specified range
    r1 = $user_eq.shelf_2_lo_q_limit;
    r0 = max r1;
    r1 = $user_eq.shelf_2_hi_q_limit;
    r0 = min r1;
    r1 = $user_eq.q_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // q

    r1 = 48000;
    r0 = r6;
    if z r0 = r1;
    call $kal_float_lib.int_to_float;
    pushm <r0,r1>;                      // fs (default to 48kHz if not currently set

    call r7;

    jump pop_biquad_coefs_and_rts;



calcPEQ.jump_entry:

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_COEFF_GEN_ASM.PEQ_GEN_COEFFS.CALCPEQ_JUMP_ENTRY.PATCH_ID_0, r1)     // peq_coeff_patchers1
//#endif

    // reserve space on stack for coefficients to be returned
    sp = sp + 6*ADDR_PER_WORD;

    // fc
    r0 = m[i0,MK1];                       // fc
    // constrain parameter to specified range
    r1 = $user_eq.xp_2_lo_freq_limit;
    r0 = max r1;
    r1 = $user_eq.xp_2_hi_freq_limit;
    r0 = min r1;
    r1 = $user_eq.freq_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // fc

    // g
    r0 = m[i0,MK1];                       // gain (need to sign extend)
    // constrain parameter to specified range
    r1 = $user_eq.shelf_1_lo_gain_limit;
    r0 = max r1;
    r1 = $user_eq.shelf_1_hi_gain_limit;
    r0 = min r1;
    r1 = $user_eq.gain_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // g

    // q
    r0 = m[i0,MK1];
    // constrain parameter to specified range
    r1 = $user_eq.shelf_2_lo_q_limit;
    r0 = max r1;
    r1 = $user_eq.shelf_2_hi_q_limit;
    r0 = min r1;
    r1 = $user_eq.q_param_scale;
    call $kal_float_lib.q_to_float;
    pushm <r0,r1>;                      // q

    r1 = 48000;
    r0 = r6;
    if z r0 = r1;
    call $kal_float_lib.int_to_float;
    pushm <r0,r1>;                      // fs (default to 48kHz if not currently set

    call $kal_filter_coef_lib.calc_peq;

    jump pop_biquad_coefs_and_rts;



pop_biquad_coefs_and_rts:
//------------------------------------------------------------------------------

    pop r0;             // b2
    m[i1,MK1] = r0;
    pop r0;             // b1
    m[i1,MK1] = r0;
    pop r0;             // b0
    m[i1,MK1] = r0;
    pop r0;             // a2
    m[i1,MK1] = r0;
    pop r0;             // a1
    m[i1,MK1] = r0;
    pop r0;             // scale
    m[i1,MK1] = r0;

    pop rLink;
    rts;




.ENDMODULE;
