// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// $Change: 2815177 $  $DateTime: 2017/07/05 17:59:25 $
// *****************************************************************************
#ifndef USER_EQ_HEADER_INCLUDED
#define USER_EQ_HEADER_INCLUDED

#include "portability_macros.h"


    // eq definition table

    .const $user_eq.DEFINITION_TABLE_MAX_NUM_BANKS 0;
    .const $user_eq.DEFINITION_TABLE_MAX_NUM_BANDS 1;
    .const $user_eq.DEFINITION_TABLE_LEFT_DM_PTR   2;
    .const $user_eq.DEFINITION_TABLE_RIGHT_DM_PTR  3;
    .const $user_eq.DEFINITION_TABLE_COEFS_A_PTR   4;
    .const $user_eq.DEFINITION_TABLE_COEFS_B_PTR   5;
    
    .const $user_eq.DEFINITION_TABLE_SIZE          6;



    .const $user_eq.num_bands                      0;
    .const $user_eq.pre_gain                       1;
    
    // filter types
    
    .const $user_eq.filter_type.bypass             0;
        
    .const $user_eq.filter_type.lp_1               1;
    .const $user_eq.filter_type.hp_1               2;
    .const $user_eq.filter_type.ap_1               3;
        
    .const $user_eq.filter_type.ls_1               4;
    .const $user_eq.filter_type.hs_1               5;
    .const $user_eq.filter_type.tlt_1              6;

    .const $user_eq.filter_type.lp_2               7;
    .const $user_eq.filter_type.hp_2               8;
    .const $user_eq.filter_type.ap_2               9;
        
    .const $user_eq.filter_type.ls_2              10;
    .const $user_eq.filter_type.hs_2              11;
    .const $user_eq.filter_type.tlt_2             12;
        
    .const $user_eq.filter_type.peq               13;
    
    // filter parameter constants
    
    .const $user_eq.freq_param_scale              20;
    .const $user_eq.gain_param_scale              12;
    .const $user_eq.q_param_scale                 8;

    
    // master gain limits
    .const $user_eq.gain_lo_gain_limit           Qfmt_(-60.0, $user_eq.gain_param_scale);
    .const $user_eq.gain_hi_gain_limit           Qfmt_(20.0, $user_eq.gain_param_scale);
    
    // filter limits related to nyquist frequency
    .const $user_eq.hi_nyquist_freq_limit   0.453515;       //  3,628 Hz @ 8 kHz sampling rate
                                                            //  7,256 Hz @ 16 kHz sampling rate
                                                            // 14,512 Hz @ 32 kHz sampling rate
                                                            // 20,000 Hz @ 44.1 kHz sampling rate
                                                            // 21,769 Hz @ 48 kHz sampling rate
        
    // first order xp filter limits
    .const $user_eq.xp_1_lo_freq_limit          Qfmt_(20.0, $user_eq.freq_param_scale);
    .const $user_eq.xp_1_hi_freq_limit          Qfmt_(24000.0, $user_eq.freq_param_scale);
        
    // second order xp filter limits
    .const $user_eq.xp_2_lo_freq_limit          Qfmt_(20.0, $user_eq.freq_param_scale);
    .const $user_eq.xp_2_hi_freq_limit          Qfmt_(24000.0, $user_eq.freq_param_scale);
    .const $user_eq.xp_2_lo_q_limit             Qfmt_(0.25, $user_eq.q_param_scale);
    .const $user_eq.xp_2_hi_q_limit             Qfmt_(100.0, $user_eq.q_param_scale);
    
    // first order shelf limits
    .const $user_eq.shelf_1_lo_freq_limit       Qfmt_(20.0, $user_eq.freq_param_scale);
    .const $user_eq.shelf_1_hi_freq_limit       Qfmt_(24000.0, $user_eq.freq_param_scale);
    .const $user_eq.shelf_1_lo_gain_limit       Qfmt_(-60.0, $user_eq.gain_param_scale);
    .const $user_eq.shelf_1_hi_gain_limit       Qfmt_(20.0, $user_eq.gain_param_scale);
        
    // second order shelf limits
    .const $user_eq.shelf_2_lo_freq_limit       Qfmt_(20.0, $user_eq.freq_param_scale);
    .const $user_eq.shelf_2_hi_freq_limit       Qfmt_(24000.0, $user_eq.freq_param_scale);
    .const $user_eq.shelf_2_lo_gain_limit       Qfmt_(-60.0, $user_eq.gain_param_scale);
    .const $user_eq.shelf_2_hi_gain_limit       Qfmt_(20.0, $user_eq.gain_param_scale);
    .const $user_eq.shelf_2_lo_q_limit          Qfmt_(0.25, $user_eq.q_param_scale);
    .const $user_eq.shelf_2_hi_q_limit          Qfmt_(100.0, $user_eq.q_param_scale);
        
    // parametric filter limits
    .const $user_eq.peq_lo_freq_limit           Qfmt_(20.0, $user_eq.freq_param_scale);
    .const $user_eq.peq_hi_freq_limit           Qfmt_(24000.0, $user_eq.freq_param_scale);
    .const $user_eq.peq_lo_gain_limit           Qfmt_(-60.0, $user_eq.gain_param_scale);
    .const $user_eq.peq_hi_gain_limit           Qfmt_(20.0, $user_eq.gain_param_scale);
    .const $user_eq.peq_lo_q_limit              Qfmt_(0.25, $user_eq.q_param_scale);
    .const $user_eq.peq_hi_q_limit              Qfmt_(100.0, $user_eq.q_param_scale);

#endif // USER_EQ_HEADER_INCLUDED
