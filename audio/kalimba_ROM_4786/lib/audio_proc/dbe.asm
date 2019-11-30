// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "core_library.h"
#include "dbe.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.initialize
//
// DESCRIPTION:
//    Initialize function for the DBE module
//
// INPUTS:
//    - r8 = pointer to the dbe current_channel(left/right) data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - rMAC,r0,r1,r2,r3,r4,r5,r6,r7,I0,I7
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.dbe.initialize;
   .CODESEGMENT   PM;

$audio_proc.dbe.initialize:

   push rLink;
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DBE_ASM.DBE.INITIALIZE.PATCH_ID_0,r1)     // dbe_patchers
//#endif

   // *************************************
   // Clear channel object.  Protect r10
   // *************************************
   r1  = r10;
   I0  = r8 + $audio_proc.dbe.START_HISTORY_BUF_HP1;
   r10 = ($audio_proc.dbe.STRUC_SIZE - ($audio_proc.dbe.START_HISTORY_BUF_HP1 >> LOG2_ADDR_PER_WORD));
   r0 = NULL;
   do lp_dbe_clr;
      M[I0,ADDR_PER_WORD] = r0;
   lp_dbe_clr:
   r10 = r1;

   // ***************************************************************************
   // Check if xover is bypassed and if not call the xover initialize function
   // ***************************************************************************
   r0 = M[r8 + $audio_proc.dbe.CAP_CONFIG_FIELD];
   r0 = r0 AND $M.DBE.CONFIG.BYPASS_XOVER;
   if Z call $audio_proc.dbe.xover.initialize;

   // ***********************************************************************************
   // dbe_usr_fchp_48kf = DBE_CONVERT_48_2 + DBE_CONVERT_48_1*FC_HP*FS/Fd_SamplingRate;
   // ***********************************************************************************
   r0 = M[r8 + $audio_proc.dbe.PARAM_PTR_FIELD];
   r0 = M[r0 + $audio_proc.dbe.parameter.FC_HP];
   rMAC = r0 * $DBE_CONVERT_48_1;   // FC_HP * DBE_CONVERT_48_1 (q20.4*q1.23=q.28)
   rMAC = rMAC ASHIFT 18;           // <<18 (q.28+18=q.46-24=q.22)
   rMAC = rMAC * $DBE_P1_Q26;       // *0.1 (q.22*q.26=q.49)
   r0 = rMAC ASHIFT -2;             // >> 2 (q.49-2=q.47-24=q.23)
   r0 = r0 * $SAMP_FACTOR;
   r0 = r0 + $DBE_CONVERT_48_2;

   // ***********************************************************************************
   // dbe_usr_fclp_48kf = DBE_CONVERT_48_2 + DBE_CONVERT_48_3*FC_LP*FS/Fd_SamplingRate;
   // ***********************************************************************************
   r1 = M[r8 + $audio_proc.dbe.PARAM_PTR_FIELD];
   r1 = M[r1 + $audio_proc.dbe.parameter.FC_LP] ;
   rMAC = r1 * $DBE_CONVERT_48_3;   // FC_LP * DBE_CONVERT_48_3 (q20.4*q1.23=q.28)
   rMAC = rMAC ASHIFT 18;          // <<18 (q.28+18=q.46-24=q.22)
   rMAC = rMAC * $DBE_P1_Q26;    // *0.1 (q.22*q.26=q.49)
   r1 = rMAC ASHIFT -2;          // >>2 (q.49-2=q.47-24=q.23)
   r1 = r1 * $SAMP_FACTOR;
   r1 = r1 + $DBE_CONVERT_48_2;
   M[r8 + $audio_proc.dbe.USR_FCLP] = r1;

   // ***********************************************************************************
   // dbe_fchp_scaled = dbe_usr_fchp_48kf*Effect_Improve_Factor;
   // ***********************************************************************************
   r2 = r0 * $EFFECT_IMPROVE_FACTOR;

   // ***********************************************************************************
   // dbe_fclpf         = dbe_usr_fclp_48kf*0.25;
   // ***********************************************************************************
   r3 = r1 * 0.25;
   M[r8 + $audio_proc.dbe.FCLP] = r3;

   // ***********************************************************************************
   // dbe_fchpf = dbe_fchp_scaled*0.25;
   // ***********************************************************************************
   r3 = r2 * 0.25;
   M[r8 + $audio_proc.dbe.FCHP] = r3;

   // ***********************************************************************************
   // dbe_fchp2f        = 1.0 - dbe_fchpf.*dbe_fchp_scaled - 0.5*dbe_fchp_scaled;
   // ***********************************************************************************
   r4 = 0.5 * r2;   // 0.5*dbe_fchp_scaled;
   r5 = r3 * r2;    // dbe_fchpf.*dbe_fchp_scaled
   r4 = r4 + r5;    // dbe_fchpf.*dbe_fchp_scaled + 0.5*dbe_fchp_scaled
   r4 = -1.0 + r4;   // 1.0 - dbe_fchpf.*dbe_fchp_scaled - 0.5*dbe_fchp_scaled
   r4 = 0 - r4;
   M[r8 + $audio_proc.dbe.FCHP2] = r4;

   // ***********************************************************************************
   // dbe_fchp3f        = -dbe_fchpf.*dbe_fchp_scaled/16.0;
   // ***********************************************************************************
   r4 = r2 * 0.0625;
   r4 = r3 * r4;
   r4 = -r4;
   M[r8 + $audio_proc.dbe.FCHP3] = r4;

   // ***********************************************************************************
   // Filter coefficients for HP1
   // hp1_a1 = (1-2*fchp)
   // ***********************************************************************************
   r4 = r3 * 2(int);
   r4 = -1.0 + r4;
   r4 = 0 - r4;
   M[r8 + $audio_proc.dbe.HP1_A1] = r4;

   // ***********************************************************************************
   // Filter coefficients for HP2.
   // hp2_b1 = -2*(1+dbe_fchp3) / 2
   // hp2_a1 = -(1+fchp2) / 2
   // hp2_a2 = (1-2*fchp) / 2
   // {hp2_scalefactor = 1}
   // Note: hp2 coefficients are stored divided by two(to prevent overflow). A shift of two will be applied
   // to the filter output to make up for this.
   // ***********************************************************************************
   r4 = M[r8 + $audio_proc.dbe.FCHP3];
   r4 = -1.0 - r4; // FCHP3 is negative
   M[r8 + $audio_proc.dbe.HP2_B1]= r4;

   r4 = M[r8 + $audio_proc.dbe.FCHP2];
   r4 = r4 * -0.5;
   r4 = r4 - 0.5;
   M[r8 + $audio_proc.dbe.HP2_A1]= r4;

   r4 = M[r8 + $audio_proc.dbe.FCHP];
   r4 = 0.5 - r4;
   M[r8 + $audio_proc.dbe.HP2_A2]= r4;
   // ***********************************************************************************
   // Filter coefficients for NHP
   // nhp_a1 = (1-2*fchp)
   // ***********************************************************************************
   r4 = M[r8 + $audio_proc.dbe.FCHP];
   r4 = r4 ASHIFT 1;
   r4 = 1.0 - r4;
   M[r8 + $audio_proc.dbe.NHP_A1] = r4;

   // ***********************************************************************************
   // Filter coefficients for NTP1(NLP1)
   // ntp1_b0 = dbe_usr_fchp_48kf
   // ntp1_a1 = (1-fchp)
   // ***********************************************************************************
   M[r8 + $audio_proc.dbe.NTP1_B1]= r0;
   r4 = M[r8 + $audio_proc.dbe.FCHP];
   r4 = -1.0 + r4;
   r4 = 0 - r4;
   M[r8 + $audio_proc.dbe.NTP1_A1]= r4;

   // ***********************************************************************************
   // Filter coefficients for NTP2(NLP2)
   // ntp2_a1 = (1-fchp)
   // ***********************************************************************************
   M[r8 + $audio_proc.dbe.NTP2_A1]= r4;

   // ***********************************************************************************
   // Filter coefficients for TP1
   // tp1_a1 = (1-2*fclp)
   // ***********************************************************************************
   r3 = M[r8 + $audio_proc.dbe.FCLP];
   r4 = r3 ASHIFT 1;
   r4 = -1.0 + r4;
   r4 = 0 - r4;
   M[r8 + $audio_proc.dbe.TP1_A1] = r4;

   // ***********************************************************************************
   // Filter coefficients for TP2
   // tp2_a1 = (1-2*fclp)
   // tp2_b0 = (fchp+fclp)
   // ***********************************************************************************
   M[r8 + $audio_proc.dbe.TP2_A1]= r4;
   r4 = M[r8 + $audio_proc.dbe.FCHP];
   r5 = M[r8 + $audio_proc.dbe.FCLP];
   r4 = r4 + r5;
   M[r8 + $audio_proc.dbe.TP2_B0]= r4;

   // ***********************************************************************************
   // Gains
   // DBE_GAIN_UPDATE  = 12; % update DBE_GAIN & DBE_NLGAIN at every 12 samples
   // DBE_NLGAIN parameters
   // DBE_NLGAIN0    = 0;
   // DBE_NLGAIN1    = 0;
   // DBE_GAIN = 0;
   // dbe_gain_sm = 0;
   // sqrtGC = 0;
   // ***********************************************************************************
   r4 = 12;
   M[r8 + $audio_proc.dbe.DBE_GAIN_UPDATE] = r4;
   M[r8 + $audio_proc.dbe.DBE_NLGAIN0] = 0;
   M[r8 + $audio_proc.dbe.DBE_NLGAIN1] = 0;
   M[r8 + $audio_proc.dbe.DBE_GAIN] = 0;
   M[r8 + $audio_proc.dbe.dbe_gain_sm] = 0;
   M[r8 + $audio_proc.dbe.sqrtGC] = 0;
   M[r8 + $audio_proc.dbe.RMS_LVL]=0;


   // ***********************************************************************************
   // DBE gain parameters;
   // dbe2_strength   = harm_content/128;
   // fix_gain_lin    = 10^(fix_gain/20);
   // dbe_GC_in = 32/1024;
   // RefLevel_db = -45.5 - 128*dbe_usr_fchp_48kf + 0.5*effect_strength - 0.5*MasterVolume_db;
   // RefLevel_lin = 10^(RefLevel_db/20);
   // RefLevelLim_db = 3 - DBE_LIM_MDB - amp_limit + MasterVolume_db;
   // RefLevelLim_lin = 0.5*10^(RefLevelLim_db/20);
   // ***********************************************************************************
   r4 = M[r8 + $audio_proc.dbe.PARAM_PTR_FIELD];
   r4 = M[r4 + $audio_proc.dbe.parameter.HARM_CONTENT];
   r4 = r4 * 0.01(int);                                        // DBE_user_p.HARM_CONTENT/100
   r4 = r4 * 0.9921875;                                        // dbe2_strength   = harm_content*127 /128;
   M[r8 + $audio_proc.dbe.DBE2_STRENGTH] = r4;                 // dbe2_strength

   // ***********************************************************************************
   // effect_strength = round(DBE_user_p.EFFECT_STRENGTH/100*127);
   // ***********************************************************************************
   r4 = M[r8 + $audio_proc.dbe.PARAM_PTR_FIELD];
   r4 = M[r4 + $audio_proc.dbe.parameter.EFFECT_STRENGTH];     // DBE_user_p.EFFECT_STRENGTH
   Words2Addr(r4);
   I0 = $ref_lim_init_calc + r4;
   r4 = M[I0,0];

   // ***********************************************************************************
   // RefLevel_db = -45.5 - 128*dbe_usr_fchp_48kf + 0.5*effect_strength - 0.5*MasterVolume_db;
   // RefLevel_db/20 can be re written as
   // RefLevel_db/20 =  -0.0177734375 - 0.05 *dbe_usr_fchp_48kf + 0.000248046875* effect_strength
   // ***********************************************************************************
   rMAC =  r0 * -0.05 ;                                        // - 0.05 *dbe_usr_fchp_48kf + 0.000248046875* effect_strength

   // *******************************************************************************************************
   // RefLevel_lin = 10^(RefLevel_db/20);
   // RefLevel_lin = 2 power Z , where Z = RefLevel_db/(20 * log2)  ; 1/ log10(2) = 3.3219
   // *******************************************************************************************************
   rMAC = rMAC ASHIFT 7 (56bit);
   rMAC =  rMAC * Qfmt_(3.321928095, 8);                       // RefLevel_db/(20 * log2) in Q8.xx
   r0 = rMAC;                                                  // r0 = x (negative number). in q8.xx
   call $math.pow2_taylor;                                     // r0 = pow2(x), in q1.xx
   r0 = r0 * 0.005306073;
   r0 = r0 * r4;                                               // in q12.xx format
   M[r8 + $audio_proc.dbe.RefLevel_lin] = r0;

   // ***********************************************************************************
   // RefLevelLim_db = 3 - DBE_LIM_MDB - amp_limit + MasterVolume_db;
   // ***********************************************************************************
   r4 = M[r8 + $audio_proc.dbe.PARAM_PTR_FIELD];
   r4 = M[r4 + $audio_proc.dbe.parameter.AMP_LIMIT];           // amp_limit
   r4 = r4 ASHIFT (12 - DAWTH);                                // convert to 24.0
   r1 = 3 - $DBE_LIM_MDB;                                      // 3 - DBE_LIM_MDB
   r1 = r1 - r4;                                               // RefLevelLim_db = 3 - DBE_LIM_MDB - amp_limit
   r1 = r1 * 0.025 ( int);                                     // (RefLevelLim_db/20) >> 1
   rMAC = r1 * Qfmt_(3.321928095, 7);                          // RefLevel_db/(20 * log2) << 1
   r0 = rMAC;                                                  // r0 = x (negative number). in q8.xx
   call $math.pow2_taylor;                                     // - r0 = pow2(x), in Q1.xx
   r0 = r0 * 0.5;
   M[r8 + $audio_proc.dbe.RefLevelLim_lin] = r0;

   r0 = r8 + $audio_proc.dbe.START_HISTORY_BUF_HP1;
   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_HP1] = r0;           // start at history buf base for HP1 filter

   r0 = r8 + $audio_proc.dbe.START_HISTORY_BUF_HP2;
   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_HP2] = r0;           // start at history buf base for HP2 filter

   r0 = r8 + $audio_proc.dbe.START_HISTORY_BUF_SRC;
   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_SRC] = r0;           // start at history buf base for SRC filter


   // *************************************************************************
   // set the initial value of "dbe_cur_block_size" and "dbe_gain_update_flag"
   // *************************************************************************
   M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE] = Null;


   // *****************************************************************
   // set the "downsample_factor" value based on sample_rate" value
   // *****************************************************************
   r0 = M[r8 + $audio_proc.dbe.SAMPLE_RATE_FIELD];

sample_rate_32k_check:
   Null = r0 - 32000;
   if NZ jump sample_rate_44k_check;
   r1 = 2;
   jump downsample_factor_set;

sample_rate_44k_check:
   Null = r0 - 44100;
   if NZ jump sample_rate_48k_check;
   r1 = 2;
   jump downsample_factor_set;

sample_rate_48k_check:
   Null = r0 - 48000;
   if NZ jump sample_rate_88k_check;
   r1 = 2;
   jump downsample_factor_set;

sample_rate_88k_check:
   Null = r0 - 88200;
   if NZ jump sample_rate_96k_check;
   r1 = 4;
   jump downsample_factor_set;

sample_rate_96k_check:
   Null = r0 - 96000;
   if NZ call $error;
   r1 = 4;

downsample_factor_set:
   M[r8 + $audio_proc.dbe.DBE_DOWNSAMPLE_FACTOR] = r1;

   // ************************************************
   // set the shift factor for the dbe frame_size
   // ************************************************
   r0 = -1;
   r2 = -2;
   Null = r1 - 4;
   if Z r0 = r2;
   M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR] = r0;

   // *****************************************************************
   // Initialize "framebuffer_flag" for the current data_object
   // *****************************************************************
   M[r8 + $audio_proc.dbe.FRAMEBUFFER_FLAG] = Null;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.xover.initialize
//
// DESCRIPTION:
//    Initialize function for the XOVER module
//
// INPUTS:
//    - r8 = pointer to dbe data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r5,r6,r7
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.dbe.xover.initialize;
   .CODESEGMENT   PM;

$audio_proc.dbe.xover.initialize:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DBE_ASM.DBE.XOVER.INITIALIZE.PATCH_ID_0,r1)     // dbe_patchers
//#endif

   // ****************************************************
   // initialize the history buffer pointer
   // ****************************************************
   r0 = r8 + $audio_proc.xover.START_HISTORY_BUF_XOVER;
   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_XOVER] = r0;         // start at history buf base for XOVER filter


   // ****************************************************
   // ****************************************************
   //               COEFFICIENT GENERATION
   // ****************************************************
   // ****************************************************

   // ****************************************************
   // fc_in/fs
   // ****************************************************
   r0 = M[r8 + $audio_proc.dbe.PARAM_PTR_FIELD];
   r5 = M[r0 + $audio_proc.dbe.parameter.XOVER_FC];
   r6 = M[r8 + $audio_proc.dbe.SAMPLE_RATE_FIELD];
   r6 = r6 ASHIFT (DAWTH - 20);  // account for Q format difference (fs:integer, fc:Q20.N)
   call $audio_proc.dbe.float_division;

   // ********************************************************************
   // stmp1 = sin[(fc_in/2fs)*2*pi] = sin[(fc_in/fs)*pi]
   // stmp2 = sin[((fc_in/2fs)+ 0.25)*2*pi] = sin[((fc_in/fs) + 0.5)*pi]
   // tanfnhalf = stmp1/stmp2;
   // ********************************************************************
   r0 = r4;
   call $math.sin;
   r5 = r1;
   r0 = r4 + 0.5;
   call $math.sin;
   r6 = r1;
   call $audio_proc.dbe.float_division;
   push r4;

   // ******************************************
   // a1 = (1-tanfnhalf)/(1+tanfnhalf);
   // g0 = a1;
   // ******************************************
   r4 = r4 * 0.25(frac);
   r6 = 0.25 + r4;
   r5 = 0.25 - r4;
   call $audio_proc.dbe.float_division;
   M[r8 + $audio_proc.xover.G0] = r4;

   // *************************************
   // b2 = (pr1 + a1*a1)/(1 + pr1*a1*a1);
   // g1 = b2/2;
   // pr1 = 0.333333333
   // *************************************
   r0 = M[r8 + $audio_proc.xover.G0];
   r0 = r0 * r0(frac);
   r0 = r0 * 0.25(frac);
   r5 = r0 + 0.08333333325;
   r6 = r0 * 0.33333333333(frac);
   r6 = r6 + 0.25;
   call $audio_proc.dbe.float_division;
   r3 = r4 * 0.5(frac);
   M[r8 + $audio_proc.xover.G1] = r3;
   r7 = r3;

   // *****************************************************
   // a2 = (1-tanfnhalf*tanfnhalf)/(1+tanfnhalf*tanfnhalf);
   // g2 = a2*(1+b2)/2 = 0.5*a2 + 0.5*a2*b2
   // *****************************************************
   pop r4;
   r4 = r4 * r4;
   r4 = r4 * 0.25(frac);
   r6 = 0.25 + r4;
   r5 = 0.25 - r4;
   call $audio_proc.dbe.float_division;

   r5 = r4 * 0.5(frac);
   r7 = r7 * r4(frac);
   r4 = r7 + r5;
   M[r8 + $audio_proc.xover.G2] = r4;

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.float_division
//
// DESCRIPTION:
//   This function calls the floating point division kalimba code and returns the
//   quotient
//
// INPUTS:
//   r5 - dividend
//   r6 - divisor
//
// OUTPUTS:
//    - r4 - fixed-point format quotient
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.float_division;
   .CODESEGMENT   PM;

$audio_proc.dbe.float_division:

   push rLink;
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DBE_ASM.DBE.FLOAT_DIVISION.PATCH_ID_0,r1)     // dbe_patchers
//#endif

   // ***********************************************
   // convert the divisor to floating-point format
   // ***********************************************
   r0 = r6;
   call $kal_float_lib.int_to_float;
   r2 = r0;
   r3 = r1;

   // ***********************************************
   // convert the dividend to floating-point format
   // ***********************************************
   r0 = r5;
   call $kal_float_lib.int_to_float;

   // ************************************************************************
   // call the floating point division code and convert back to fixed-point
   // ************************************************************************
   call $kal_float_lib.div;
   r0 = r0 and 0x0000ff;       // expect sign to be zero, but mask off just in case
   r0 = r0 - 127;              // remove offset from exponent
   r1 = r1 LSHIFT -1;          // shift mantissa so fits twos complement arithmetic
   r4 = r1 ASHIFT r0;          // fixed-point equivalent

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.process
//
// DESCRIPTION:
//   This function does the dbe processing on the inputs. It has 2 entry points.
//   $audio_proc.dbe.frame_process  : Entry point for frame_sync API
//   $audio_proc.dbe.stream_process : Entry point for Kymera API
//   Note : The "DBE_SAMPLES_TO_PROCESS" value must be even
//
// INPUTS:
//   r7 - pointer to left_data_object
//   r8 - pointer to right_data_object (valid only for stereo mode)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Assume everything
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.process;
   .CODESEGMENT   PM;

$audio_proc.dbe.frame_process:


   // *******************************************************************************************************
   // Sets "samples_to_process" and "framebuffer_flag" value for the dbe left/right data objects.
   // Falls through to the $audio_proc.dbe.stream_process() function
   // *******************************************************************************************************

set_buffer_left_data_object:
   // *******************************************************
   // Set "samples_to_process" for the left_data_object
   // *******************************************************
   r0 = M[r7 + $audio_proc.dbe.INPUT_ADDR_FIELD];
   r3 = M[r0 + $frmbuffer.FRAME_SIZE_FIELD];
   M[r7 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS] = r3;

   // *******************************************************
   // Set "framebuffer_flag" for the left_data_object
   // *******************************************************
   r0 = 1;
   M[r7 + $audio_proc.dbe.FRAMEBUFFER_FLAG] = r0;

   Null = M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD];
   if Z jump $audio_proc.dbe.stream_process;

set_buffer_right_data_object:
   // *******************************************************
   // Set "samples_to_process" for the right_data_object
   // *******************************************************
   r0 = M[r8 + $audio_proc.dbe.INPUT_ADDR_FIELD];
   r3 = M[r0 + $frmbuffer.FRAME_SIZE_FIELD];
   M[r8 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS] = r3;

   // *******************************************************
   // Set "framebuffer_flag" for the right_data_object
   // *******************************************************
   r0 = 1;
   M[r8 + $audio_proc.dbe.FRAMEBUFFER_FLAG] = r0;


$audio_proc.dbe.stream_process:

   push rLink;

   // Enable saturate on add/sub
   r3 = M[$ARITHMETIC_MODE];
   push r3;
   r3 = r3 OR $ADDSUB_SATURATE_ON_OVERFLOW_MASK;
   M[$ARITHMETIC_MODE] = r3;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DBE_ASM.DBE.PROCESS.PATCH_ID_1,r1)     // dbe_patchers
#endif


#ifdef DEBUG_ON
   // ************************************************************************************
   // call the "error" function if the "samples_to_process" is odd for the left channel
   // ************************************************************************************
   r4 = M[r7 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS];
   r0 = r4 AND 1;
   if NZ call $error;
   Null = M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD];
   if Z jump dbe_proc;
#endif

#ifdef DEBUG_ON
   // ************************************************************************************
   // call the "error" function if the "samples_to_process" is odd for the right channel
   // ************************************************************************************
   r5 = M[r8 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS];
   r0 = r5 AND 1;
   if NZ call $error;

   // ********************************************************************************************************
   // call the "error" function if the "samples_to_process" is not the same for the left and right channels
   // ********************************************************************************************************
   Null = r4 - r5;
   if NZ call $error;
#endif


#ifdef DEBUG_ON
   // ***************************************************************************************************************
   // call the "error" function if the "samples_to_process" is a non-multiple of 4 for the 88.2K & 96K sample rates
   // ***************************************************************************************************************
   r5 = M[r7 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS];
   r0 = M[r7 + $audio_proc.dbe.SAMPLE_RATE_FIELD];
   Null = r0 - 88200;
   if NZ jump error_sample_rate_96k_check;
   if Z jump check_samples_to_process;

error_sample_rate_96k_check:
   Null = r0 - 96000;
   if NZ jump dbe_proc;

check_samples_to_process:
   r0 = r5 AND 3;
   if NZ call $error;
#endif


dbe_proc:
   // *************************************************************************
   // set the xover and mixer bypass flags to the left_channel data object
   // Note : If xover is bypassed then mixer is always bypassed
   // *************************************************************************
   r0 = M[r7 + $audio_proc.dbe.CAP_CONFIG_FIELD];
   r2 = r0 AND $M.DBE.CONFIG.BYPASS_BASS_OUTPUT_MIX;
   r1 = r0 AND $M.DBE.CONFIG.BYPASS_XOVER;
   M[r7 + $audio_proc.dbe.XOVER_BYPASS_FLAG] = r1;
   M[r7 + $audio_proc.dbe.MIXER_BYPASS_FLAG] = r2;

   // *********************************************************************
   // initialize input_read_addr/output_write_addr for the left_data_object
   // *********************************************************************
   push r8;
   r8 = r7;
   call $audio_proc.dbe.get_buffer_details;
   pop r8;
   M[r7 + $audio_proc.dbe.INPUT_READ_ADDR] = r3;
   M[r7 + $audio_proc.dbe.OUTPUT_WRITE_ADDR] = r4;
   Null = M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD];
   if Z jump dbe_frame_proc;

   // *********************************************************************
   // initialize input_read_addr/output_write_addr for the right_data_object
   // *********************************************************************
   call $audio_proc.dbe.get_buffer_details;
   M[r8 + $audio_proc.dbe.INPUT_READ_ADDR] = r3;
   M[r8 + $audio_proc.dbe.OUTPUT_WRITE_ADDR] = r4;

dbe_frame_proc:
   // ***********************************************************
   // load "samples_to_process" and set "dbe_cur_block_size"
   // **********************************************************
   r3 = M[r7 + $audio_proc.dbe.DBE_SAMPLES_TO_PROCESS];
   r4 = $DBE_FRAME_SIZE;
   r0 = M[r7 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   if NZ r4 = r0;

dbe_frame_proc_loop:
   pushm <r3,r4>;

   // ***************************************************************
   // Update gains at start of each frame (zero==update)
   // ***************************************************************
   r1 = $DBE_FRAME_SIZE-r4;
   M[r7 + $audio_proc.dbe.DBE_GAIN_UPDATE_FLAG] = r1;

   // ***************************************************************
   // set the "dbe_cur_block_size" for the left and right channels
   // ***************************************************************
   Null = r3 - r4;
   if LE r4 = r3;
   M[r7 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE] = r4;
   Null = M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD];
   if Z jump dbe_chan_proc_start;
   M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE] = r4;

dbe_chan_proc_start:
   call $audio_proc.dbe.process_channels;
   popm <r3,r4>;
   r3 = r3 - r4;
   if LE jump dbe_process_done;
   r4 = $DBE_FRAME_SIZE;
   jump dbe_frame_proc_loop;

dbe_process_done:
   // **********************************************
   // initialize "cur_block_size" for the next frame
   // **********************************************
   r2 = NULL - r3;
   M[r7 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE] = r2;

init_cur_block_size_done:

   // Restore arithmetic mode */
   pop r3;
   M[$ARITHMETIC_MODE]=r3;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.process_channels
//
// DESCRIPTION:
//   This function calls the gain calculation functions followed by
//   the filters function for each channel
//
// INPUTS:
//   r7 - pointer to left_data_object
//   r8 - pointer to right_data_object (valid only for stereo mode)
//   r4 - current block_size
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Everything except r7,r8
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.process_channels;
   .CODESEGMENT   PM;

$audio_proc.dbe.process_channels:

   push rLink;

   // ********************************************************************
   // call the xover,dbe,mixer modules sequentially for each channel
   // ********************************************************************
   r5 = 0;             // channel(0/1)
dbe_channel_loop_proc:
   Null = M[r7 + $audio_proc.dbe.XOVER_BYPASS_FLAG];
   if NZ jump bass_enhance_proc;

   // ****************************************************
   // call the xover process for the current channel
   // ****************************************************
   pushm <r5,r7,r8>;
   Null = r5;
   if NZ r7 = r8;
   call $audio_proc.xover.process;
   popm <r5,r7,r8>;


bass_enhance_proc:
   // ****************************************************
   // signal level detection
   // ****************************************************
   pushm <r5,r7,r8>;
   Null = r5;
   if NZ r7 = r8;
   call $audio_proc.sigdetect.process;
   popm <r5,r7,r8>;

   // *****************************************************************************************
   // call the gains_calc() and gains_copy()functions for the left/right channel respectively
   // *****************************************************************************************
   Null = M[r7 + $audio_proc.dbe.DBE_GAIN_UPDATE_FLAG];
   if NZ jump no_update_dbe_gains;
   Null = r5;
   if Z call $audio_proc.dbe.gains_calc;
no_update_dbe_gains:

   // ********************************************************************
   // call the dbe.filters_process() function for the current channel
   // ********************************************************************
   pushm <r5,r7,r8>;
   Null = r5;
   if Z r8 = r7;
   call $audio_proc.dbe.filters_process;
   popm <r5,r7,r8>;

   // ********************************************************************
   // call the output_mixer.process() function for the current channel
   // ********************************************************************
   Null = M[r7 + $audio_proc.dbe.MIXER_BYPASS_FLAG];
   if NZ jump process_next_channel;
   pushm <r7,r8>;
   Null = r5;
   if NZ r7 = r8;
   call $audio_proc.output_mixer.process;
   popm <r7,r8>;

process_next_channel:
   pushm <r7,r8>;
   Null = r5;
   if NZ r7 = r8;
   call $audio_proc.dbe.advance_pointers;
   popm <r7,r8>;

   // ********************************************
   // clear the input/output base/length registers
   // *********************************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
   push Null;
   pop B4;
#endif

   L0 = 0;
   L4 = 0;

   // ************************************************************************
   // exit the loop if mono_mode or both channels processed under stereo_mode
   // ************************************************************************
   Null = M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD];
   if Z jump dbe_channels_proc_done;
   r5 = r5 + 1;
   Null = r5 - $DBE_MAX_CHANNELS;
   if NZ jump dbe_channel_loop_proc;

dbe_channels_proc_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.gains_calc
//
// DESCRIPTION:
//   This function calls the DBENLgain and DBEgain calculation functions
//
// INPUTS:
//   r7 - pointer to left_data_object
//   r8 - pointer to right_data_object(valid only for stereo mode)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r9,rMAC
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.gains_calc;
   .CODESEGMENT   PM;

$audio_proc.dbe.gains_calc:


   pushm <r5,r8>;
   /* Save Right channel Pointer */
   r5 = r8;

   // *******************************************************
   // DBE gains update functions
   // *******************************************************

   // *****************************************************************************
   // tmp0 = max(abs(Ibuf_dbe(n,1))*(DBE_NLGAIN0^2) -2*dbe_fchpf^2, 0);
   // *****************************************************************************
   r2 = M[r7 +  $audio_proc.dbe.ABS_IBUF];            // abs(Ibuf_dbe(n,1) - left channel
   r1 = M[r7 + $audio_proc.dbe.DBE_NLGAIN0];          // DBE_NLGAIN0
   r2 = r2 * r1;                                      // abs(Ibuf_dbe(n,1) * DBE_NLGAIN0
   r2 = r2 * r1;                                      // abs(Ibuf_dbe(n,1) * DBE_NLGAIN0  * DBE_NLGAIN0
   r3 = M[r7 + $audio_proc.dbe.FCHP];                 // dbe_fchpf
   rMAC = r3 * r3;                                    // dbe_fchpf ^2
   r3 = rMAC ASHIFT 1;                                // r3 =  2*dbe_fchpf^2
   r2 = r2 - r3 ;                                     // abs(Ibuf_dbe(n,1))*(DBE_NLGAIN0^2) -2*dbe_fchpf^2
   r2 = MAX NULL;		                                  // tmp0 = max(abs(Ibuf_dbe(n,1))*(DBE_NLGAIN0^2) -2*dbe_fchpf^2, 0);

   // *****************************************************************************
   // DBE_NLGAIN0 = max(DBE_NLGAIN0 -8*tmp0 + DBE_NL_const, 0);
   // *****************************************************************************
   r2 = r2 ASHIFT 3;                                  // 8*tmp0
   r1 = r1 - r2 ;                                     // DBE_NLGAIN0 -8*tmp0
   r1 = r1 + $DBE_NL_const;                           // DBE_NLGAIN0 -8*tmp0 + DBE_NL_const
   r1 = MAX NULL;                                     // DBE_NLGAIN0 = max(DBE_NLGAIN0 -8*tmp0 + DBE_NL_const, 0);
   M[r7 + $audio_proc.dbe.DBE_NLGAIN0] = r1;          // write DBE_NLGAIN0 to dbe_data_object_left
   M[r7 + $audio_proc.dbe.NTP2_B1] = r1;              // write DBE_NLGAIN0 to NTP2_B1 for dbe_data_object_left
   // min(DBE_NLGAIN0, DBE_NLGAIN1) (r9)
   r9 = r1;

   // get average channel RMS (left and right)
   rMAC = M[r7 + $audio_proc.dbe.RMS_LVL];

   Null = M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD];
   if Z jump mono_mode_dbenlgain;
       // *****************************************************************************
       // tmp1 = max(abs(Ibuf_dbe(n,2))*(DBE_NLGAIN1^2) -2*dbe_fchpf^2, 0);
       // *****************************************************************************
       r2 = M[r5 +  $audio_proc.dbe.ABS_IBUF];            // abs(Ibuf_dbe(n,2) - right channel
       r1 = M[r7 + $audio_proc.dbe.DBE_NLGAIN1];          // DBE_NLGAIN1
       r2 = r2 * r1;                                      // abs(Ibuf_dbe(n,2) * DBE_NLGAIN1
       r2 = r2 * r1;                                      // abs(Ibuf_dbe(n,1) * DBE_NLGAIN1  * DBE_NLGAIN1
       r2 = r2 - r3 ;                                     // r3 =  2*dbe_fchpf^2 ; r2 = abs(Ibuf_dbe(n,2))*(DBE_NLGAIN1^2) -2*dbe_fchpf^2
       r2 = MAX NULL;		                                  // tmp1 = max(abs(Ibuf_dbe(n,2))*(DBE_NLGAIN1^2) -2*dbe_fchpf^2, 0);

       // *****************************************************************************
       // DBE_NLGAIN1 = max(DBE_NLGAIN1 -8*tmp1 + DBE_NL_const, 0);
       // *****************************************************************************
       r2 = r2 ASHIFT 3;                                  // 8*tmp1
       r1 = r1 - r2 ;                                     // DBE_NLGAIN1 -8*tmp1
       r1 = r1 + $DBE_NL_const;                           // DBE_NLGAIN1 -8*tmp1 + DBE_NL_const
       r1 = MAX NULL;                                     // DBE_NLGAIN1 = max(DBE_NLGAIN1 -8*tmp1 + DBE_NL_const, 0);
       M[r7 + $audio_proc.dbe.DBE_NLGAIN1] = r1;          // write DBE_NLGAIN1 to dbe_data_object_left
       // min(DBE_NLGAIN0, DBE_NLGAIN1) (r9)
       r9 = MIN r1;

       // get average channel RMS (left and right)
       r1 = -1.0;
       r0 = M[r5 + $audio_proc.dbe.RMS_LVL];
       rMAC = rMAC - r0*r1;
       rMAC = rMAC ASHIFT -1;
mono_mode_dbenlgain:

//    - r7 = pointer to dbe_data_object_left
//    - r8 = dbe_lim_max
//    - r9 = min(DBE_NLGAIN0, DBE_NLGAIN1)
//    - rMAC = average channel RMS

   // *********************************************************************************
   // Soft start - hold gain_min at zero until RMS level is above {threshold} for {count} frames
   // *********************************************************************************
   //    InputLvl = log2(max(rms_lvl,deltaP));
   //    %% update the counter if the rms value is above the Level Threshold, reset otherwise
   //    InputLvl = log2(max(rms_lvl,deltaP));
   //    if InputLvl < LevelThreshold
   //       counter = 0;
   //    else
   //       counter = counter + 1;
   //    end
   //    %% When the counter exceeds the Time Threshold ramp up is turned on.
   //    if counter < TimeThreshold
   //       sqrtGC = 0;
   //    else
   //       sqrtGC = max(sqrtGC*(1-16*sqrtGC*dbe_GC_in) + 16/65536, 0); % only if dbe_GC_in<1/16, otherwise divide by 16
   //    end
   //
   // step2) threshold and counter logic, generate output
   r0 = M[r7 + $audio_proc.dbe.THRESHOLD_COUNTER];
   r0 = r0 + 1;
   Null = rMAC - $DBE_LEVEL_THRESHOLD;
   if LT r0 = 0;
   r4 = $DBE_SIGDETECT_TIME_THRESHOLD_FRAMES;
   r5 = 0;
   Null = r0 - r4;   // compare THRESHOLD_COUNTER to limit
   if NZ r5 = 1;     // supress if not equal
   if Z r0 = r0 - 1;     // hold at limit
   M[r7 + $audio_proc.dbe.THRESHOLD_COUNTER] = r0; // store counter back
   // r5 = hold_zero

   // *******************************************************************
   // sqrtGC = max(sqrtGC*(1-16*sqrtGC*dbe_GC_in) + 16/65536, 0);
   // $dbe_GC_K = 16/65536
   // *******************************************************************
   r0 = M[r7 + $audio_proc.dbe.sqrtGC];
   rMAC = r0 * $DBE_GC_IN;               // sqrtGC*dbe_GC_in
   r1 = rMAC ASHIFT 4;                   // 16*sqrtGC*dbe_GC_in
   r1 = 1.0 - r1;                        // 1-16*sqrtGC*dbe_GC_in
   rMAC = r0 * r1 ;                      // sqrtGC*(1-16*sqrtGC*dbe_GC_in)
   rMAC = rMAC + $dbe_GC_K;              // A = sqrtGC*(1-16*sqrtGC*dbe_GC_in) + 16/65536
   r0 = rMAC;
   r0 = MAX Null;                        // max(A,0)
   Null = r5;
   if NZ r0 = Null;
   M[r7 + $audio_proc.dbe.sqrtGC] = r0;

   // ****************************************************
   // gain_min = sqrtGC*min(DBE_NLGAIN0, DBE_NLGAIN1);
   // ****************************************************
   rMAC = r0 * r9;                         // gain_min

   // ****************************************************************************************************
   // dbe_gain_tmp = min(512*gain_min*RefLevel_lin,0.75)*DBE_const*(dbe_usr_fclp_48kf+4*dbe_fchpf);
   // ****************************************************************************************************
   r0 = M[r7 + $audio_proc.dbe.RefLevel_lin];   // in Q12.xx format
   rMAC = rMAC * 64(int);
   rMAC = rMAC * r0;
   rMAC = rMAC ASHIFT 14 (56bit);               // 512*gain_min*RefLevel_lin in Q1.xx
   r0 = rMAC ;
   r2 = $DBE_GAIN_TEMP_THRESH;
   r0 = MIN r2;                                 // min(512*gain_min*RefLevel_lin,0.75)
   r1 = M[r7 + $audio_proc.dbe.USR_FCLP];       // r1 = dbe_usr_fclp_48kf;
   r5 = -1.0;
   rMACB = M[r7 + $audio_proc.dbe.FCHP];        // rMACB = dbe_fchpf;
   rMACB = rMACB ASHIFT 2 (56bit);              // rMACB = dbe_fchpf * 4;
   rMACB = rMACB - r1*r5;                       // rMACB = dbe_usr_fclp_48kf + dbe_fchpf * 4
   rMAC = r0 * $DBE_const_num;                  // dbe_gain_tmp = min(512*gain_min*RefLevel_lin,0.75)*(dbe_usr_fclp_48kf+4*dbe_fchpf);
   rMAC = rMAC * rMACB;
   rMAC = rMAC * $DBE_COMP_TC;                  // DBE_COMP_TC*dbe_gain_tmp;
   rMAC = rMAC ASHIFT 2 (56bit);                // rMAC = min(512*gain_min*RefLevel_lin,0.75) * DBE_const *(dbe_usr_fclp_48kf+4*dbe_fchpf);	 // DBE_const in q.21

   // ****************************************************************************************************
   // dbe_gain_sm = (1-DBE_COMP_TC)*dbe_gain_sm + DBE_COMP_TC*dbe_gain_tmp;
   // ****************************************************************************************************
   r1 = M[r7 + $audio_proc.dbe.dbe_gain_sm];    // dbe_gain_sm
   rMAC = rMAC + r1*$ONEMINUS_DBE_COMP_TC;      // + (1-DBE_COMP_TC)*dbe_gain_sm
   M[r7 + $audio_proc.dbe.dbe_gain_sm]=rMAC;

   // **********************************************************************
   // dbe_lim_t = DBE_KLIM/2
   // **********************************************************************
   r2 = $DBE_KLIM/2;

   // **********************************************************************
   // DBE_GAIN = (1-dbe_lim_t)*dbe_gain_sm;
   // **********************************************************************
   rMAC = rMAC - rMAC*r2;         // DBE_GAIN = (1-dbe_lim_t)*dbe_gain_sm;

   M[r7  + $audio_proc.dbe.DBE_GAIN] = rMAC;       // write DBE_GAIN to left_data_object
   M[r7 + $audio_proc.dbe.TP1_B0]    = rMAC ;      // write TP1_B0 for left_data_object

   popm <r5,r8>;
   // *********************************************************************************
   // copy the gains and coefficients from the left_data_object to right_data_object
   // *********************************************************************************
   Null = M[r7 + $audio_proc.dbe.MONO_STEREO_FLAG_FIELD];
   if Z rts;

   M[r8 + $audio_proc.dbe.DBE_GAIN]  = rMAC;                   // write DBE_GAIN to right_data_object
   M[r8 + $audio_proc.dbe.TP1_B0]    = rMAC;                   // write TP1_B0 to right_data_object

   r0 = M[r7 + $audio_proc.dbe.DBE_NLGAIN1];                   // read DBE_NLGAIN1 from left_data_object
   M[r8 + $audio_proc.dbe.DBE_NLGAIN1] = r0;                   // write DBE_NLGAIN1 to right_data_object

   r0 = M[r7 + $audio_proc.dbe.NTP2_B1];                       // read NTP2_B1 from left_data_object
   M[r8 + $audio_proc.dbe.NTP2_B1] = r0;                       // write NTP2_B1 to right_data_object

   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.filters_process
//
// DESCRIPTION:
//   This function calls a series of filters operating on the input.
//   The function is called separately for the left/right channels.
//
// INPUTS:
//    - r8 = pointer to dbe data object (left/right)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Assume everything
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.filters_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.filters_process:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DBE_ASM.DBE.FILTERS_PROCESS.PATCH_ID_0, r1)     // dbe_patchers
//#endif


   // ************************************************
   // load the input_read_addr and output_write_addr
   // set the size/base_addresses for the buffers
   // ************************************************
   r0 = M[r8 + $audio_proc.dbe.INPUT_READ_ADDR];
   I0 = r0;
   r0 = M[r8 + $audio_proc.dbe.OUTPUT_WRITE_ADDR];
   I4 = r0;
   call $audio_proc.dbe.get_buffer_details;

   // **************************************
   // HP1 and HP3 filters
   // **************************************
   push I4;
   push L4;
   call $audio_proc.dbe.hp1hp3_process;

   // **************************************
   // NTP1 filter(low-pass)
   // **************************************
   r7 = 0;
   call $audio_proc.dbe.ntp_process;

   // **************************************
   // NTP2 filter(low-pass)
   // **************************************
   r7 = 1;
   call $audio_proc.dbe.ntp_process;

   // **************************************
   // NHP filter
   // **************************************
   call $audio_proc.dbe.nhp_process;

   // **************************************
   // Mixer1 process
   // **************************************
   call $audio_proc.dbe.mixer1_process;

   // **************************************
   // TP1 filter
   // **************************************
   call $audio_proc.dbe.tp1_process;

   // **************************************
   // TP2 filter
   // **************************************
   call $audio_proc.dbe.tp2_process;

   // **************************************
   // HP2 filter
   // **************************************
   call $audio_proc.dbe.hp2_process;

   // **************************************
   // Sample rate converter (up-sampler)
   // **************************************
   pop L4;
   pop I4;

   r1 = $audio_proc.dbe.src_process_downsample2;
   r2 = $audio_proc.dbe.src_process_downsample4;
   r0 = M[r8 + $audio_proc.dbe.DBE_DOWNSAMPLE_FACTOR];
   Null = r0 - 4;
   if Z r1 = r2;
   call r1;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.hp1hp3_process
//
// DESCRIPTION:
//  This function implements the high-pass filters hp1 and hp3
//  The difference equation for the filters is as follows :
//  hp1_out = y = x - x' +(1- 2*fchp)*y'
//  hp3_out = y = x - x'[Transfer function : 1-z^-1)]
//
//  INPUTS:
//    - r8 = pointer to dbe data object
//    - I0 = pointer to input buffer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r5,r6,r7,r10,rMAC
//
// NOTES
//
// *****************************************************************************

.MODULE $M.audio_proc.dbe.hp1hp3_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.hp1hp3_process:

   push rLink;

   // ************************************
   // Get output buffer
   // ************************************
   r0 = M[r8 + $audio_proc.dbe.hp1_out];
   I4 = r0;
   L4 = 0;
   r0 = M[r8 + $audio_proc.dbe.hp3_out];
   I5 = r0;

   // *************************************************
   // Setup I1/B1/L1 for history buffer access
   // *************************************************
   r0 = r8 + $audio_proc.dbe.START_HISTORY_BUF_HP1;
   push r0;
   pop B1;
   r0 = M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_HP1];
   I1 = r0;
   L1 = $audio_proc.dbe.HP1_HIST_BUF_SIZE - $audio_proc.dbe.START_HISTORY_BUF_HP1;

   // ******************************************************
   // load the HP1 filter coefficient
   // ******************************************************
   r3 = M[r8 + $audio_proc.dbe.HP1_A1];

   // ******************************
   // load the downsample_factor
   // set the loop counter
   // ******************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r5 = M[r8 + $audio_proc.dbe.DBE_DOWNSAMPLE_FACTOR];
   r4 = M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR];
   r10 = r10 ASHIFT r4;

   // ******************************
   // Set the modify registers
   // ******************************
   M0 = 0*ADDR_PER_WORD;
   M1 = 1*ADDR_PER_WORD;
   Words2Addr(r5);                       // arch4: downsample factor in addrs
   M2 = r5;
   M3 = -3*ADDR_PER_WORD;

   r6 = -DAWTH;
   r7 = -1.0;
   r4 = M[I1, M1];         // dummy read
   r4 = M[I0, M2];        // I1 = &y_l' , r4 = x_in
   r4 = r4 * 0.125;
   r0 = M[I1,M1];                        // r0 = y_l' , I1 = &x_in

   do hp1_loop;
      //  hp1_out = y = x - x' +(1- 2*fchp)*y' = x - x' + r3 * y'
      rMAC = r3 * r0 (SU), M[I1,M1] = r4;                 // y = r3 * y_l' , write x_in , I1 = &x_in'
      rMAC = rMAC ASHIFT r6 (56bit) , r5 = M[I1,MK2];     // shift the computed value to rMAC0, r5 = x_in' , I1 = &y_h'
      rMAC = rMAC + r7 * r5 , r0 = M[I1,M0];              // y = r3 * y_l' - x_in' , r0 = y_h'
      rMAC = rMAC - r7 * r4;                              // y = r3 * y_l' - x_in' + x_in
      rMAC = rMAC + r3 * r0 , r2 = M[I1,-MK1];            // y = r3 * (y_l' + y_h') - x_in' + x_in ,dummy_read ,I1 = &hp1_out_h
      r1 = rMAC ASHIFT Null;                              // y_h_out
      r0 = rMAC LSHIFT DAWTH;                             // y_l_out , write hp1_out_h(y_h_out) ,
      r4 = r4 + r5*r7 , M[I1,M3] = r1;
      M[I4,M1] = rMAC;       // hp3_out , I1 = &hp1_out_l, write output
      M[I5,M1] = r4 , r4 = M[I0,M2];                      // write hp3_out , r4 = x_in
      r4 = r4 * 0.125;
   hp1_loop:

   r1 = M[I1,-MK1];  // dummy read
   M[I1,-MK1] = r0;
   r0 = I1;
   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_HP1] = r0;  // start at history buf base


   // ***********************************
   // reset base and length registers
   // ***********************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
#endif
   push Null;
   pop B1;

   L0 = 0;
   L1 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.ntp_process
//
// DESCRIPTION:
//  This function implements the low-pass filters ntp1 and ntp2
//  The difference equations for the filters are as follows :
//  ntp1_out = y = fchp*x + (1-fchp)*y'      = r3 * x + r6 * y'
//  nt2_out = y = dbe_nlgain*x + (1-fchp)*y' = r3 * x + r6 * y'
//
// INPUTS:
//    - r8 = pointer to dbe data object
//    - r7 - flag indicating ntp1/ntp2 (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r2,r3,r4,r5,r6,r10,rMAC
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.ntp_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.ntp_process:

   push rLink;

   // ********************************
   // Setup ntp1/ntp2 configuration
   // ********************************
   Null = r7;
   if NZ jump ntp2_filter_setup;

ntp1_filter_setup:
      r0 = M[r8 + $audio_proc.dbe.hp1_out];
      I0 = r0;                                         // input to ntp1
      r0 = M[r8 + $audio_proc.dbe.ntp_tp_filters_buf];
      I1 = r0;                                         // output for ntp1
      r3 = M[r8 + $audio_proc.dbe.NTP1_B1];            // filter coefficient fchp
      r4 = r8 + $audio_proc.dbe.NTP1_OUT_1;            // pointer to ntp1 'y' history
      r5 = r8 + $audio_proc.dbe.NTP1_IN_1;             // pointer to ntp1 'x' history
      jump ntp_filter_process;

ntp2_filter_setup:
      r0 = M[r8 + $audio_proc.dbe.ntp_tp_filters_buf];
      I0 = r0;                                         // input to ntp2
      I1 = I0;                                         // output for ntp2(in place)
      r3 = M[r8 + $audio_proc.dbe.NTP2_B1];            // filter coefficient dbe_nlgain
      r4 = r8 + $audio_proc.dbe.NTP2_OUT_1;            // pointer to ntp2 'y' history
      r5 = r8 + $audio_proc.dbe.NTP2_IN_1;             // pointer to ntp1 'x' history

ntp_filter_process:
   // ******************************
   // set the loop counter
   // ******************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r0 = M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR];
   r10 = r10 ASHIFT r0;

   r6 = M[r8 + $audio_proc.dbe.NTP1_A1];               // (1-fchp) Note: This value is common to NTP1/NTP2
   r0 = M[r5];                                         // load x'
   r2 = M[r4];                                         // load the history value y'
   M1 = 1*ADDR_PER_WORD;

   // ntp1_out = y = fchp*x + (1-fchp)*y' = r3 * x + r6 * y'
   // ntp2_out = y = dbe_nlgain*x + (1-fchp)*y' = r3 * x + r6 * y'
   do ntp_loop;
      rMAC = r3 * r0 , r0 = M[I0,M1];                  // y = r3*x , load x
      rMAC = rMAC + r6 * r2;                           // r3*x + (1-fchp)*y'
      r2 = rMAC , M[I1,M1] = rMAC;                     // set y' = y(for next iteration) , write y
   ntp_loop:

   M[r4] = r2;                                         // write y' to history
   M[r5] = r0;                                         // write x' to history
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.nhp_process
//
// DESCRIPTION:
//  This function implements the high-pass filter nhp
//  The difference equation for the filter is as follows :
//  nhp_out = y = 2*x'*x - 2*x'*x' + (1-2*fchp)*y'
//
// INPUTS:
//    - r8 = pointer to dbe data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r2,r3,r4,r10,rMAC
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.nhp_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.nhp_process:

   push rLink;


   r0 = M[r8 + $audio_proc.dbe.ntp_tp_filters_buf];
   I0 = r0;                     // input buffer
   I1 = I0;                     // output buffer (in-place)

   // *******************************
   // load the filter coefficient
   // *******************************
   M1 = 1*ADDR_PER_WORD;

   // ******************************
   // set the loop counter
   // ******************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r4 = M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR];
   r10 = r10 ASHIFT r4;
   r3 = M[r8 + $audio_proc.dbe.NHP_A1];      // (1-2*fchp)
   r3 = r3 * 0.5(frac);                      // 0.5*(1-2*fchp)

   r2 = M[r8 + $audio_proc.dbe.NHPOUT_1];    // load y'
   r4 = M[r8 + $audio_proc.dbe.NHPIN_1];     // load x'
   do nhp_loop;
      //  nhp_out = y = 2*x'*x - 2*x'*x' + (1-2*fchp)*y' = 2[x'*x -x'*x' + 0.5*(1-2*fchp)*y']
      rMAC = r3 * r2, r0 = M[I0,MK1];          // y = 0.5*(1-2*fchp)*y' , load x
      rMAC = rMAC + r4 * r0;                 // y = 0.5*(1-2*fchp)*y' + x'*x
      rMAC = rMAC - r4 * r4;                 // y = 0.5*(1-2*fchp)*y' + x'*x - x'*x'
      rMAC = rMAC ASHIFT 1;                  // y = 2*x'*x - 2*x'*x' + (1-2*fchp)*y'
      r4 = r0 , M[I1,M1] = rMAC;             // r4 = x'(for next iteration) , write y
      r2 = rMAC ;                            // set y' = y(for next iteration)
   nhp_loop:

   M[r8 + $audio_proc.dbe.NHPIN_1] = r0;  // write x' to history for next frame
   M[r8 + $audio_proc.dbe.NHPOUT_1] = r2; // write y' to history for next frame

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.tp1_process
//
// DESCRIPTION:
//  This function implements the low-pass filter tp1
//  The difference equation for the filter is as follows :
//  tp1_out = y = (-2*dbe_gain)*x + (1-2*fclp)*y'
//
// INPUTS:
//    - r8 = pointer to dbe data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    -r1,r2,r3,r4,r10,rMAC
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.tp1_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.tp1_process:

   push rLink;

   r1 = M[r8 + $audio_proc.dbe.ntp_tp_filters_buf];
   I1 = r1;                    // input buffer
   I3 = I1;                    // output buffer (in-place)

   // ******************************
   // set the loop counter
   // ******************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r4 = M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR];
   r10 = r10 ASHIFT r4;

hist_ptr_loaded:
   r1 = M[r8 + $audio_proc.dbe.TP1OUT_1_LEFT];  // load history value y'(left)
   r3 = M[r8 + $audio_proc.dbe.TP1_B0];         // (dbe_gain)
   r4 = M[r8 + $audio_proc.dbe.TP1_A1];         // (1-2*fclp)
   M1 = 1*ADDR_PER_WORD;
   do tp1_loop;
      // tp1_out = y = (-2*dbe_gain)*x + (1-2*fclp)*y'
      rMAC = r4 * r1 , r2 = M[I1,M1];           // y = (1-2*fclp)*y' , load x
      rMAC = rMAC - r3 * r2;                    // y = (1-2*fclp)*y' -dbe_gain)*x
      rMAC = rMAC - r3 * r2;                    // y = (1-2*fclp)*y' -2*dbe_gain)*x
      r1 = rMAC , M[I3,M1] = rMAC;              // y' = y , write y
   tp1_loop:
   M[r8 + $audio_proc.dbe.TP1OUT_1_LEFT] = r1;  // write y' to history for next frame


   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.tp2_process
//
// DESCRIPTION:
//  This function implements the low-pass filter tp2
//  The difference equation for the filter is as follows :
//  tp2_out = y = x1 - x1' + (fchp+fclp)*x2 + (1-2*fclp)*y'
//  Note1 : This function implements mixer2 as well
//  Note2 : This does not include the HP3 filter which has is already
//  done along with HP1
//
// INPUTS:
//    - r8 = pointer to dbe data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    -r0,r1,r2,r3,r4,r5,r10,rMAC
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.tp2_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.tp2_process:

   push rLink;

   // *********************************
   // Get input buffers
   // *********************************
   r1 = M[r8 + $audio_proc.dbe.hp3_out];
   I1 = r1;                                    // x1 buffer
   r1 = M[r8 + $audio_proc.dbe.ntp_tp_filters_buf];
   I4 = r1;                                    // x2 buffer

   // *********************************
   // Get output buffer
   // *********************************
   I0 = I4;                                    // in place processing

   // ******************************
   // set the loop counter
   // ******************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r4 = M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR];
   r10 = r10 ASHIFT r4;

   // ******************************
   // load history values
   // ******************************
   r3 = M[r8 + $audio_proc.dbe.TP2_B0];    // (fchp + fclp)
   r4 = M[r8 + $audio_proc.dbe.TP2_A1];    // (1 - 2*fclp)
   r2 = M[r8 + $audio_proc.dbe.TP2IN2_1];  // load x2'
   r1 = M[r8 + $audio_proc.dbe.TP2OUT_1];  // load y'
   M1 = 1*ADDR_PER_WORD;
   r5 = -1.0;
   do tp2_loop;
      // tp2_out = y = x1 + (fchp+fclp)*x2 + (1-2*fclp)*y'
      rMAC = r4 * r1;                                     // y = (1-2*fclp)*y', load x2
      rMAC = rMAC  + r3 * r2 , r0 = M[I1,M1];             // y = (1-2*fclp)*y' + (fchp+fclp)*x2 , load x1
      rMAC = rMAC - r0*r5 , r2 = M[I4,MK1];               // y = (1-2*fclp)*y' - x1' + (fchp+fclp)*x2 + x1 , load x2
      r1 = rMAC , M[I0,M1] = rMAC;                        // y' = y , write y
   tp2_loop:

   M[r8 + $audio_proc.dbe.TP2IN2_1] = r2;      // write x2' to history for next frame
   M[r8 + $audio_proc.dbe.TP2OUT_1] = r1;      // write y' to history for next frame

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.hp2_process
//
// DESCRIPTION:
//  This function implements the high-pass filter hp2
//  The difference equation for the filter is as follows :
//  hp2_out = y = x - 2*(1+dbe_fchp3)*x' + x'' + (1+fchp2)*y' - (1-2*fchp)*y''
//
// INPUTS:
//    - r8 = pointer to dbe data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    -r0,r1,r2,r3,r4,r5,r6,r7,r9,r10,rMAC
//
// NOTES
//


// *****************************************************************************
.MODULE $M.audio_proc.dbe.hp2_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.hp2_process:

   push rLink;


   // *********************************
   // Get input buffer
   // *********************************
   r0 = M[r8 + $audio_proc.dbe.ntp_tp_filters_buf];
   I0 = r0;

   // *********************************
   // Get output buffer
   // *********************************
   r0 = M[r8 + $audio_proc.dbe.hp2_out];
   I5 = r0;

   // *************************************************
   // Setup I1/B1/L1 for history buffer access
   // *************************************************
   r0 = r8 + $audio_proc.dbe.START_HISTORY_BUF_HP2;
   push r0;
   pop B1;
   r0 = M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_HP2];
   I1 = r0;
   L1 = $audio_proc.dbe.HP2_HIST_BUF_SIZE - $audio_proc.dbe.START_HISTORY_BUF_HP2;

   // ******************************
   // set the loop counter
   // ******************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r4 = M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR];
   r10 = r10 ASHIFT r4;

   // ******************************
   // set loop constants
   // load history values
   // ******************************
   M1 = 1*ADDR_PER_WORD;
   r3 = M[r8 + $audio_proc.dbe.DBE_DOWNSAMPLE_FACTOR];
   Words2Addr(r3);                      // arch4: downsample factor in addrs
   M2 = r3;
   M3 = -2*ADDR_PER_WORD;
   M0 = 3*ADDR_PER_WORD;
   r7 = -0.5; // B0/B2
   r3 = M[r8 + $audio_proc.dbe.HP2_B1]; // dbe_fchp3(to avoid saturation) , actual coefficient = 2*(1+dbe_fchp3)
   r6 = M[r8 + $audio_proc.dbe.HP2_A1]; // fchp2(to avoid saturation) , actual coefficient = (1+fchp2)
   r9 = M[r8 + $audio_proc.dbe.HP2_A2]; // (1-2*fchp)
   r4 = M[I0, MK1];                     // r4 = x
   r0 = M[I1,MK1];                      // dummy read
   r0 = M[I1,MK1];                      // r0 = yL', I1 = &yL''
   r2 = DAWTH;
   rMAC = 0;
   do hp2_loop;
      // hp2_out = y = ( x/2 - 2*(1+dbe_fchp3)/2*x' + x''/2 + (1+fchp2)*y'/2 - (1-2*fchp)*y''/2 ) << 1
      rMAC = rMAC - r6 * r0 (SU) , r0 = M[I1, M1];     // - A1*Y_L'     r0 = yL'', I1 = &x
      rMAC = rMAC - r9 * r0 (SU), M[I1, M1] = r4;      // - A2*Y_L''    write x , I1 = &x'
      rMAC = rMAC ASHIFT -DAWTH (56bit);                  // >> 24,        shift the computed values to the lower 24 bits
      rMAC = rMAC - r7 * r4 , r5 = M[I1, M1];          // + B0*X,       r5 = x' , I1 = x''
      rMAC = rMAC + r3 * r5 , r0 = M[I1, 2*ADDR_PER_WORD];           // + B1*X'       r0 = x'' , I1 = &y'
      rMAC = rMAC - r7 * r0 , r0 = M[I1, M1];          // + B2*X''      r0 = yH'
      rMAC = rMAC - r6 * r0, r0 = M[I1, M3];           // - A1*Y_H'     r0 = y'', I1 = &y
      rMAC = rMAC - r9 * r0, r4 = M[I0, MK1];          // - A2*Y_H''    r4 = x(next sample)
      rMAC = rMAC ASHIFT 1(56bit);                     // << COEFF_SCALE
      r1 = rMAC ASHIFT Null;                           // get y_h_out
      r0 = rMAC LSHIFT r2, M[I1,M0] = r1;              // get y_l_out , write y_h_outr
      rMAC = rMAC - rMAC, M[I5,M2] = rMAC , M[I1,M1] = r0 ;  // write y_out, write y_l_out to history , I1 = &yL'' , M2 = 2 , insert 0 in b/w
   hp2_loop:

   r2 = M[I1,-MK1]; // dummy read , I1 = &yL'
   M[I1,-MK1] = r0; // r2 = yL' , I1 =&yL
   r2 = I1;

   r0 = I1;
   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_HP2] = r0;  // start at history buf base

   push Null;
   pop B1;

   L1 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.mixer1_process
//
// DESCRIPTION:
//  This function implements the mixer 1 of the DBE module
//  Obuf0_mix1 = (1-dbe2_strength)*HP1_buf0 + dbe2_strength*NHP_buf0
//  Obuf1_mix1 = (1-dbe2_strength)*HP1_buf1 + dbe2_strength*NHP_buf1
//
// INPUTS:
//    - r8 = pointer to dbe data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    -r0,r1,r4,r5,r10,rMAC
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.mixer1_process;
   .CODESEGMENT   PM;

$audio_proc.dbe.mixer1_process:

   push rLink;


   r0 = M[r8 + $audio_proc.dbe.hp1_out];
   I0 = r0;                                                   // input buffer1
   r0 = M[r8 + $audio_proc.dbe.ntp_tp_filters_buf];
   I4 = r0;                                                   // input buffer2
   I5 = I4;                                                   // output buffer (in place)

   r4 = M[r8 + $audio_proc.dbe.DBE2_STRENGTH];                // dbe2_strength
   r5 = 1.0 - r4;                                             // (1-dbe2_strength)

   // ******************************
   // set the loop counter
   // ******************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r0 = M[r8 + $audio_proc.dbe.DBE_FRAME_SHIFT_FACTOR];
   r10 = r10 ASHIFT r0;

   // ******************************
   // load history values
   // ******************************
   r0 = M[r8 + $audio_proc.dbe.MIXER1_HP1_HIST];              // load hp1_hist;
   r1 = M[r8 + $audio_proc.dbe.MIXER1_NHP_HIST];              // load nhp_hist;

   do mixer1_loop;
      rMAC = r5 * r0;                                         // Obuf_mix = (1-dbe2_strength)*hp1_out ,
      rMAC = rMAC + r4 * r1 , r1 = M[I4,MK1];                 // Obuf_mix = (1-dbe2_strength)*hp1_out + dbe2_strength*nhp_out , load nhp_out
      M[I5,MK1] = rMAC , r0 = M[I0,MK1];                      // write Obuf_mix , load hp1_out
   mixer1_loop:

   M[r8 + $audio_proc.dbe.MIXER1_HP1_HIST] = r0;              // preload hp1_hist;
   M[r8 + $audio_proc.dbe.MIXER1_NHP_HIST] = r1;

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.src_process_downsample2
//
// DESCRIPTION:
//  This function implements sample rate conversion
//  The difference equation for the filter is as follows :
//  src_out = y = (v*x' + u*x''+ u*x''' + v*x'''')
//  coefficients : u = 1/4 , v = 3/4;
//  sample rates : 32K , 44.1K , 48K
//
// INPUTS:
//    - r8 = pointer to dbe data object
//    - I4 = pointer to output buffer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r3,r7,r10,rMAC,I0,I1,L1,B1,M1
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.src_process_downsample2;
   .CODESEGMENT   PM;

$audio_proc.dbe.src_process_downsample2:

   push rLink;


   // *********************************
   // Get input buffer
   // *********************************
   r0 = M[r8 + $audio_proc.dbe.hp2_out];
   I0 = r0;

   // *************************************************
   // Setup I1/B1/L1 for history buffer access
   // *************************************************
   r0 = r8 + $audio_proc.dbe.START_HISTORY_BUF_SRC;
   push r0;
   pop B1;
   r0 = M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_SRC];
   I1 = r0;
   L1 = $audio_proc.dbe.SRCIN_5 - $audio_proc.dbe.START_HISTORY_BUF_SRC;

   // *************************************************
   // load history and set the loop counter
   // *************************************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r3 = $DBE_SRC_v_factor2;                      // r3 = v;
   r7 = $DBE_SRC_u_factor2;                      // r7 = u;
   M1 = -2*ADDR_PER_WORD;

   r0 = M[I1,MK1];                               //  dummy read
   r0 = M[I1,MK1];                               //  r0 = x', I1 = &x''

   do src_loop;
      // y = 4*(v*x' + u*x''+ u*x''' + v*x'''')
      rMAC = r0 * r3 , r1 = M[I1,MK1];           // y = x'*v , r1 = x'', I1 = &x''', r3 = v
      rMAC = rMAC + r1 * r7 , r0 = M[I1,MK1];    // y = v *x' + u *x'' , r0 = x''', I1 = &x'''',r8 = u
      rMAC = rMAC + r0 * r7 , r1 = M[I1,MK1];    // y = v *x' + u *x''+ v* x''' , r1 = x'''', I1 = &x',r3 = v
      rMAC = rMAC + r1 * r3 , r0 = M[I0,MK1];    // y = v *x' + u *x'' + v* x''' + u *x'''' , r0 = x ,r8 = u
      rMAC = rMAC ASHIFT 3(56bit);
      M[I4,MK1] = rMAC , M[I1,MK1] = r0;         // store y , x' = r0 , I1 = &x''
   src_loop:

   r1 = M[I1,M1];                                // dummy read   I1 = &x
   r1 = I1;                                      // I1 = &x

   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_SRC] = r1;

   push Null;
   pop B1;

   L1 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *********************************************************************************************************
// MODULE:
//    $audio_proc.dbe.src_process_downsample4
//
// DESCRIPTION:
//  This function implements Sample rate converter for 88.2K and 96K sample rates
//  The difference equation for the filter is as follows :
//  src_out = y = (u*x' + v*x''+ w*x''' + z*x'''' + z*x''''' + w*x'''''' + v*x''''''' + u*x'''''''')
//  coefficients : u = 0.125 , v = 0.375 , w = 0.625 , z = 0.875
//  sample rates : 88.2K , 96K
//
// INPUTS:
//    - r8 = pointer to dbe data object
//    - I4 = pointer to output buffer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r3,r4,r5,r7,r10,rMAC,I0,I1,L1,B1,M1
//
// NOTES
//
// *********************************************************************************************************
.MODULE $M.audio_proc.dbe.src_process_downsample4;
   .CODESEGMENT   PM;

$audio_proc.dbe.src_process_downsample4:

   push rLink;


   // *********************************
   // Get input buffer
   // *********************************
   r0 = M[r8 + $audio_proc.dbe.hp2_out];
   I0 = r0;

   // *************************************************
   // Setup I1/B1/L1 for history buffer access
   // *************************************************
   r0 = r8 + $audio_proc.dbe.START_HISTORY_BUF_SRC;
   push r0;
   pop B1;
   r0 = M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_SRC];
   I1 = r0;
   L1 = $audio_proc.dbe.SRC_HIST_BUF_SIZE - $audio_proc.dbe.START_HISTORY_BUF_SRC;

   // *************************************************
   // load history and set the loop counter
   // *************************************************
   r10 = M[r8 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r3 = $DBE_SRC_u_factor4;                    // r3 = u;
   r4 = $DBE_SRC_v_factor4;                    // r4 = v;
   r5 = $DBE_SRC_w_factor4;                    // r5 = w;
   r7 = $DBE_SRC_z_factor4;                    // r7 = z;
   M1 = -2*ADDR_PER_WORD;

   r0 = M[I1,MK1];                             //  dummy read
   r0 = M[I1,MK1];                             //  r0 = x', I1 = &x''

   do src_loop;
      // y = (u*x' + v*x''+ w*x''' + z*x'''' + z*x''''' + w*x'''''' + v*x''''''' + u*x'''''''')
      rMAC = r0 * r3 , r1 = M[I1,MK1];           // y = u*x' , r1 = x'' , I1 = &x''' , r3 = u
      rMAC = rMAC + r1 * r4 , r0 = M[I1,MK1];    // y = u*x' + v*x'' , r0 = x''', I1 = &x'''' , r4 = v
      rMAC = rMAC + r0 * r5 , r1 = M[I1,MK1];    // y = u*x' + v*x'' + w*x''' , r1 = x'''', I1 = &x''''' , r5 = w
      rMAC = rMAC + r1 * r7 , r0 = M[I1,MK1];    // y = u*x' + v*x'' + w*x''' + z*x'''' , r0 = x''''' , I1 = &x'''''', r7 = z
      rMAC = rMAC + r0 * r7 , r1 = M[I1,MK1];    // y = u*x' + v*x'' + w*x''' + z*x'''' + z*x''''' , r1 = x'''''' , I1 = &x''''''' , r7 = z
      rMAC = rMAC + r1 * r5 , r0 = M[I1,MK1];    // y = u*x' + v*x'' + w*x''' + z*x'''' + z*x''''' + w*x'''''' , r0 = x''''''' , I1 = &x'''''''' , r5 = w
      rMAC = rMAC + r0 * r4 , r1 = M[I1,MK1];    // y = u*x' + v*x'' + w*x''' + z*x'''' + z*x''''' + w*x'''''' + v*x''''''' , r1 = x'''''''' , I1 = &x' , r4 = v
      rMAC = rMAC + r1 * r3 , r0 = M[I0,MK1];    // y = u*x' + v*x'' + w*x''' + z*x'''' + z*x''''' + w*x'''''' + v*x''''''' + u*x'''''''' , r0 = x , r3 = u
      rMAC = rMAC ASHIFT 3(56bit);
      M[I4,MK1] = rMAC , M[I1,MK1] = r0;         // store output y , store x' = r0 , I1 = &x''
   src_loop:

   r1 = M[I1,M1]; // dummy read I1 = &x
   r1 = I1;       // I1 = &x

   M[r8 + $audio_proc.dbe.PTR_HISTORY_BUF_SRC] = r1;

   push Null;
   pop B1;

   L1 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.xover.process
//
// DESCRIPTION:
//    Separates the input into low and high frequency bands
//
// INPUTS:
//   r7 - pointer to current channel data_object
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - Assume everything
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.xover.process;
   .CODESEGMENT   PM;


$audio_proc.xover.process:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DBE_ASM.XOVER.PROCESS.PATCH_ID_0, r1)     // dbe_patchers
//#endif



   // *******************************************************
   // load the input_read_addr
   // set the size/base_address for the input_buffer
   // *******************************************************
   r0 = M[r7 + $audio_proc.dbe.INPUT_READ_ADDR];
   I0 = r0;
   r8 = r7;
   call $audio_proc.dbe.get_buffer_details;

   // **************************************************************
   // do in-place processing for the xover module low_freq output
   // **************************************************************
#ifdef BASE_REGISTER_MODE
   push B0;
   pop B4;
#endif
   I4 = I0;
   L4 = L0;

   // *************************************************
   // get high_freq output_buffer of current channel
   // *************************************************
   r0 = M[r7 + $audio_proc.xover.high_freq_output_buf];
   I5 = r0;

   // **********************************************
   // Setup I1/B1/L1 for history buffer access
   // **********************************************
   r0 = r7 + $audio_proc.xover.START_HISTORY_BUF_XOVER;
   push r0;
   pop B1;
   r0 = M[r7 + $audio_proc.dbe.PTR_HISTORY_BUF_XOVER];
   I1 = r0;
   L1 = $audio_proc.dbe.STRUC_SIZE*ADDR_PER_WORD - $audio_proc.xover.START_HISTORY_BUF_XOVER;

   // ***************************
   // load coefficients
   // ***************************
   r3 = M[r7 + $audio_proc.xover.G0];
   r0 = M[r7 + $audio_proc.xover.G1];
   r8 = r0;
   r0 = M[r7 + $audio_proc.xover.G2];
   r9 = r0;

   // *****************************************
   // set modify registers and loop_counter
   // *****************************************
   r10 = M[r7 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   M0 = 4*ADDR_PER_WORD;
   M1 = 1*ADDR_PER_WORD;
   M2 = 2*ADDR_PER_WORD;
   M3 = -6*ADDR_PER_WORD;

   // ******************************
   // save pointer to data object
   // ******************************
   push r7;

   // ***********************
   // set loop constants
   // ***********************
   r6 = 0.5;
   r7 = -DAWTH;

   // *****************
   // prime ap_loop
   // *****************
   r4 = M[I0, MK1];       // r4 = ap_in
   r0 = M[I1, MK1];       // I1=&yL'
   r0 = M[I1, MK1];       // r0=yL', I1=&yL''
   rMAC = rMAC - rMAC;  // zero MAC

   do ap_loop;
      // Add some headroom
      r4 = r4 ASHIFT -1;
      // ap2_out = y = 2*[g1*x -g2*x' + .5*x'' + g2*y' - g1*y''] = 2*g1*x - 2*g2*x' + x'' + 2*g2*y' - 2*g1*y''
      rMACB = r9*r0 (SU), r0 = M[I1, M1];                       // = g2*yL', r0=yL'', I1=&ap_in
      rMACB = rMACB - r8*r0 (SU), M[I1, M1] = r4;               // - g1*yL'', wrt ap_in, I1=&x'
      rMACB = rMACB ASHIFT r7 (56bit), r5 = M[I1, M1];          // r5=x', I1=&x''
      rMACB = rMACB + r8*r4, r0 = M[I1, M2];                    // + g1*x,  r0=x'', I1=&y'
      rMACB = rMACB - r9*r5;                                    // - g2*x'
      rMACB = rMACB + r6*r0, r0 = M[I1, M1];                    // + .5*x'', r0=y', I1=&y''
      rMACB = rMACB + r9*r0, r0 = M[I1, M2];                    // + g2*y', r0=y'', I1=&ap1L'
      rMACB = rMACB - r8*r0, r2 = M[I1, M3];                    // - g1*y'', r2=ap1L'
      rMACB = rMACB ASHIFT 1 (56bit);                           // *2,
      r0 = rMACB ASHIFT Null, r1 = M[I1, M2];                   // r1=dummy, I1=&ap2
      r1 = rMACB LSHIFT DAWTH;                                  // r0/r1 = ap2
      // ap1_out = y = g0*x - x' + g0*y'
      rMAC = rMAC + r3*r2(SU), M[I1, M3] = r0;                  // = +g0*yL', save ap2, I1=&ap2L
      rMAC = rMAC ASHIFT r7 (56bit), M[I1, -MK1] = r1;          // save ap2L, I1=&y'
      rMAC = rMAC + r3*r4, r0 = M[I1, -MK1];                    // + g0*x, r0=y', I1=&y
      rMAC = rMAC + -1.0*r5;                                    // - x'
      rMAC = rMAC + r3*r0, r4 = M[I0, MK1];                     // + g0*y', r4=x
      r5 = rMAC LSHIFT DAWTH;
      r0 = rMAC ASHIFT Null;
      // hp_out = (ap1+ap2)/2  :::  lp_out = (ap1-ap2)/2
      r1   = rMAC + rMACB;                                      // ap1 + ap2,
      rMAC = rMAC - rMACB,  M[I1, -MK1] = r0;                   // ap1 - ap2, save ap1, I1=&ap1L'
      r2 = M[I1, -MK1], M[I5, MK1] = r1;                        // store hp_out, r2=dummy, I1 = &ap1L
      rMAC = rMAC - rMAC, M[I1, M0] = r5, M[I4, M1] = rMAC;     // store ap1_L, store lp_out, I1=&ap2L (ie I1=I1-1)
      r0 = M[I1, MK1]; // load ap2L',
   ap_loop:

   // *******************************************************
   // set the history buffer pointer to the right index
   // *******************************************************
   r0 = M[I1, -MK1];
   r0 = M[I1, -MK1];
   r0 = I1;
   pop r7;
   M[r7 + $audio_proc.dbe.PTR_HISTORY_BUF_XOVER] = r0;          // start at history buf base

   // **********************************
   // reset base and length registers
   // **********************************
   push Null;
   pop B1;
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
   push Null;
   pop B4;
#endif

   L0 = 0;
   L1 = 0;
   L4 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $audio_proc.sigdetect.process
//
// DESCRIPTION:
//    %%  Running RMS of a channel
//    Len = length(avg_bass); % length of the frame @ original sample rate
//    for i=1:Len
//       rms_lvl = (1-RMS_TC)*rms_lvl + RMS_TC*abs(avg_bass(i));
//    end
//
// INPUTS:
//   r7 - pointer to current channel data_object
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    r0,r1,r2,r3,r4,r8,r10,rMAC,I0
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.sigdetect.process;
   .CODESEGMENT   PM;


$audio_proc.sigdetect.process:
   push rLink;


   r8 = r7;
   call $audio_proc.dbe.get_buffer_details;
   // Use pointer advanced to current frame
   r3 = M[r7 + $audio_proc.dbe.INPUT_READ_ADDR];
   I0 = r3;
   // I0/L0/B0 = input

   r10 = M[r7 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   rMAC = M[r7 + $audio_proc.dbe.RMS_LVL];
   r3 = $DBE_SIGDETECT_RMS_ALFA;
   r4 = $DBE_SIGDETECT_RMS_1M_ALFA;
   r0 = M[I0, MK1];

   // *********************************************************************************
   // save first sample value of input buffer to be used for gain calculation
   // *********************************************************************************
   r1 = r0 * 0.125;
   r1 = ABS r1;
   M[r7 +  $audio_proc.dbe.ABS_IBUF]=r1;

   do signal_detect_loop;
      rMAC = rMAC*r4;
      r0 = ABS r0;
      rMAC = rMAC + r0*r3, r0 = M[I0, MK1];
   signal_detect_loop:

   M[r7 + $audio_proc.dbe.RMS_LVL] = rMAC;

#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
#endif
   L0 = 0;

   pop rLink;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.output_mixer.process
//
// DESCRIPTION:
//    Mixes the low and high frequencies to produce a full-band output.
//    The weights used for mixing are based on the "mix_balance" value.
//
// INPUTS:
//   r7 - pointer to current channel data_object
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r4,r6,r8,I0,L0,B0,I5,L5,B5,I4,L4,B4,I6
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.output_mixer.process;
   .CODESEGMENT   PM;


$audio_proc.output_mixer.process:
   push rLink;

    // *******************************************************
   // load the output_write_addr
   // set the size/base_address for the output_buffer
   // *******************************************************
   r0 = M[r7 + $audio_proc.dbe.OUTPUT_WRITE_ADDR];
   I0 = r0;
   r8 = r7;
   call $audio_proc.dbe.get_buffer_details;
   L0 = L4;
#ifdef BASE_REGISTER_MODE
   push B4;
   pop B0;
#endif

   // *****************************************************
   // do in-place processing for the mixer module output
   // *****************************************************
#ifdef BASE_REGISTER_MODE
   push B0;
   pop B5;
#endif
   I5 = I0;
   L5 = L0;

   // **********************************************
   // get read pointer of high_freq output buffer
   // **********************************************
   r0 = M[r7 + $audio_proc.xover.high_freq_output_buf];
   I6 = r0;

   // **********************************************
   // load mix_balance value
   // **********************************************
   r0 = M[r7 + $audio_proc.dbe.PARAM_PTR_FIELD];
   r4 = M[r0 + $audio_proc.dbe.parameter.MIX_BALANCE];
   r4 = r4 * 0.01(int);

   r10 = M[r7 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   r6 = 1.0 - r4;
   do mix_freq_loop;
      r0 = M[I0,MK1] , r1 = M[I6,MK1];
      rMAC = r0 * r4;
      rMAC = rMAC + r1 * r6;
      rMAC = rMAC ASHIFT 1;
      M[I5,MK1] = rMAC;
   mix_freq_loop:


   // ***********************************
   // reset base and length registers
   // ***********************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
   push Null;
   pop B4;
   push Null;
   pop B5;
#endif

   L0 = 0;
   L4 = 0;
   L5 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.dbe.advance_pointers
//
// DESCRIPTION:
//   Advances the read/write pointers of the input/output respectively
//
// INPUTS:
//   r7 = pointer to the current channel data_object
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r4,I0,L0,B0,I4,L4,B4,M0
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.dbe.advance_pointers;
   .CODESEGMENT   PM;


$audio_proc.dbe.advance_pointers:
   push rLink;

   // ****************************************************************
   // extract the <length,base_address> for the input/output buffers
   // load the "cur_block_size"
   // ****************************************************************
   r8 = r7;
   call $audio_proc.dbe.get_buffer_details;
   r4 = M[r7 + $audio_proc.dbe.DBE_CUR_BLOCK_SIZE];
   Words2Addr(r4);
   M0 = r4;

   // ***********************************************************
   // advance the input read address cur_block_size
   // ***********************************************************
   r0 = M[r7 + $audio_proc.dbe.INPUT_READ_ADDR];
   I0 = r0;
   r0 = M[I0,M0];
   r0 = I0;
   M[r7 + $audio_proc.dbe.INPUT_READ_ADDR] = r0;

   // ***********************************************************
   // advance the output write address cur_block_size
   // ***********************************************************
   r0 = M[r7 + $audio_proc.dbe.OUTPUT_WRITE_ADDR];
   I4 = r0;
   r0 = M[I4,M0];
   r0 = I4;
   M[r7 + $audio_proc.dbe.OUTPUT_WRITE_ADDR] = r0;

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************************************
// MODULE:
//    $audio_proc.dbe.get_buffer_details
//
// DESCRIPTION:
//   Extracts the <read_address,size,base_address> details for the input/output framebuffers/cbuffers
//   for the current channel
//
// INPUTS:
//   r8 = pointer to the current channel data_object
//
// OUTPUTS:
//    - <r3,L0,B0> = <read_address,size,base_address> for the input framebuffer/cbuffer
//    - <r4,L4,B4> = <read_address,size,base_address> for the output framebuffer/cbuffer
//
// TRASHED REGISTERS:
//    - r0,r1,r2
//
// NOTES
//
// *****************************************************************************************************
.MODULE $M.audio_proc.dbe.get_buffer_details;
   .CODESEGMENT   PM;


$audio_proc.dbe.get_buffer_details:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DBE_ASM.DBE.GET_BUFFER_DETAILS.PATCH_ID_0, r1)     // dbe_patchers
//#endif

   Null = M[r8 + $audio_proc.dbe.FRAMEBUFFER_FLAG];
   if NZ jump frame_buffer_details;

   // ************************************************************
   // get read pointer, size and start address of input cbuffer
   // ************************************************************
   r0 = M[r8 + $audio_proc.dbe.INPUT_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop B0;
#else
    call $cbuffer.get_read_address_and_size;
#endif
   r3 = r0;
   L0 = r1;

   // ************************************************************
   // get write pointer, size and start address of output cbuffer
   // ************************************************************
   r0 = M[r8 + $audio_proc.dbe.OUTPUT_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop B4;
#else
    call $cbuffer.get_write_address_and_size;
#endif
   r4 = r0;
   L4 = r1;
   jump buffer_details_done;

frame_buffer_details:
   // ****************************************************************
   // get write pointer, size and start address of output framebuffer
   // ****************************************************************
   r0 = M[r8 + $audio_proc.dbe.OUTPUT_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   r4 = r0;
   L4 = r1;

   // ****************************************************************
   // get read pointer, size and start address of input framebuffer
   // ****************************************************************
   r0 = M[r8 + $audio_proc.dbe.INPUT_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   r3 = r0;
   L0 = r1;

buffer_details_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;


