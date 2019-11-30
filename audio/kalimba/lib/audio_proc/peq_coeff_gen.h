// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_PEQ_COEFF_GEN_HEADER_INCLUDED
#define AUDIO_PEQ_COEFF_GEN_HEADER_INCLUDED
    
   // FIELD 0: number of bands/biquads: format=UINT
   .CONST $audio_proc.peq_coeff_gen.NUM_BANDS_FIELD         0*ADDR_PER_WORD;
   // FIELD 1: master GAIN: format=12.N (12.12 for K24, 12.20 for K32)
   .CONST $audio_proc.peq_coeff_gen.MASTER_GAIN             1*ADDR_PER_WORD;
   // FIELD 2: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE1_CONFIG           2*ADDR_PER_WORD;
   // FIELD 3: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE2_CONFIG           6*ADDR_PER_WORD;
   // FIELD 4: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE3_CONFIG           10*ADDR_PER_WORD;
   // FIELD 5: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE4_CONFIG           14*ADDR_PER_WORD;
   // FIELD 6: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE5_CONFIG           18*ADDR_PER_WORD;
   // FIELD 7: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE6_CONFIG           22*ADDR_PER_WORD;
   // FIELD 8: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE7_CONFIG           26*ADDR_PER_WORD;
   // FIELD 9: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE8_CONFIG           30*ADDR_PER_WORD;
   // FIELD 10: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE9_CONFIG           34*ADDR_PER_WORD;
   // FIELD 11: stage type: format=$audio_proc.peq_coeff_gen_filter_stage
   .CONST $audio_proc.peq_coeff_gen.STAGE10_CONFIG          38*ADDR_PER_WORD;
   // no inherent limit to number of stages
   
   
   // FIELD 0: stage type: format = $audio_proc.peq_coeff_gen_filter_type
   .CONST $audio_proc.peq_coeff_gen_filter_stage.STAGE_TYPE              0*ADDR_PER_WORD;
   // FIELD 1: stage FC: format=20.N (20.4 for K24, 20.12 for K32)
   .CONST $audio_proc.peq_coeff_gen_filter_stage.STAGE_FC                1*ADDR_PER_WORD;
   // FIELD 2: stage GAIN: format=12.N (12.12 for K24, 12.20 for K32)
   .CONST $audio_proc.peq_coeff_gen_filter_stage.STAGE_GAIN              2*ADDR_PER_WORD;
   // FIELD 3: stage Q: format=8.N (8.16 for K24, 8.24 for K32)
   .CONST $audio_proc.peq_coeff_gen_filter_stage.STAGE_Q                 3*ADDR_PER_WORD;
   .CONST $audio_proc.peq_coeff_gen_filter_stage.STRUC_SIZE              4;

   .CONST $audio_proc.peq_coeff_gen_filter_type.BYPASS                   0;
   .CONST $audio_proc.peq_coeff_gen_filter_type.LOWPASS_1ST_ORDER        1;
   .CONST $audio_proc.peq_coeff_gen_filter_type.HIGHPASS_1ST_ORDER       2;
   .CONST $audio_proc.peq_coeff_gen_filter_type.ALLPASS_1ST_ORDER        3;
   .CONST $audio_proc.peq_coeff_gen_filter_type.LOW_SHELF_1ST_ORDER      4;
   .CONST $audio_proc.peq_coeff_gen_filter_type.HIGH_SHELF_1ST_ORDER     5;
   .CONST $audio_proc.peq_coeff_gen_filter_type.TILT_1ST_ORDER           6;
   .CONST $audio_proc.peq_coeff_gen_filter_type.LOWPASS_2ND_ORDER        7;
   .CONST $audio_proc.peq_coeff_gen_filter_type.HIGHPASS_2ND_ORDER       8;
   .CONST $audio_proc.peq_coeff_gen_filter_type.ALLPASS_2ND_ORDER        9;
   .CONST $audio_proc.peq_coeff_gen_filter_type.LOW_SHELF_2ND_ORDER      10;
   .CONST $audio_proc.peq_coeff_gen_filter_type.HIGH_SHELF_2ND_ORDER     11;
   .CONST $audio_proc.peq_coeff_gen_filter_type.TILT_2ND_ORDER           12;
   .CONST $audio_proc.peq_coeff_gen_filter_type.PEQ                      13;


#endif // AUDIO_PEQ_COEFF_GEN_HEADER_INCLUDED

