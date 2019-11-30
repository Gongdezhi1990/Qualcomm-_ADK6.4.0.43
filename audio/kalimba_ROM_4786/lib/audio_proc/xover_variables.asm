// *****************************************************************************
// Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#include "xover.h"

// *****************************************************************************
// MODULE:
//    $audio_proc.xover.variables
//
// DESCRIPTION:
//    Variables to be used in the XOVER module
//
// *****************************************************************************
.MODULE $audio_proc.xover;
   .DATASEGMENT DM;
   
   // *********************************************************************************
   //                                    BUTTERWORTH FILTER
   // *********************************************************************************
   
   // *****************************************************
   // Butterworth filter number of stages for each order
   // *****************************************************
   .VAR/DMCONST $butterworth_filter_order_1_num_stages  = 1;
   .VAR/DMCONST $butterworth_filter_order_2_num_stages  = 1;
   .VAR/DMCONST $butterworth_filter_order_3_num_stages  = 2;
   .VAR/DMCONST $butterworth_filter_order_4_num_stages  = 2;
   .VAR/DMCONST $butterworth_filter_order_5_num_stages  = 3;
   .VAR/DMCONST $butterworth_filter_order_6_num_stages  = 3;
   .VAR/DMCONST $butterworth_filter_order_7_num_stages  = 4;
   .VAR/DMCONST $butterworth_filter_order_8_num_stages  = 4;
   .VAR/DMCONST $butterworth_filter_order_9_num_stages  = 5;
   .VAR/DMCONST $butterworth_filter_order_10_num_stages = 5;
   .VAR/DMCONST $butterworth_filter_order_11_num_stages = 6;
   .VAR/DMCONST $butterworth_filter_order_12_num_stages = 6;
   
   // *****************************************************************
   // Low-pass Butterworth filter types for all stages under each order
   // *****************************************************************
   .VAR/DMCONST $low_pass_butterworth_filter_order_1_filter_types[1]      = 1;
   .VAR/DMCONST $low_pass_butterworth_filter_order_2_filter_types[1]      = 7;
   .VAR/DMCONST $low_pass_butterworth_filter_order_3_filter_types[2]      = 7, 1;
   .VAR/DMCONST $low_pass_butterworth_filter_order_4_filter_types[2]      = 7, 7;
   .VAR/DMCONST $low_pass_butterworth_filter_order_5_filter_types[3]      = 7, 7, 1;
   .VAR/DMCONST $low_pass_butterworth_filter_order_6_filter_types[3]      = 7, 7, 7;
   .VAR/DMCONST $low_pass_butterworth_filter_order_7_filter_types[4]      = 7, 7, 7, 1;
   .VAR/DMCONST $low_pass_butterworth_filter_order_8_filter_types[4]      = 7, 7, 7, 7;
   .VAR/DMCONST $low_pass_butterworth_filter_order_9_filter_types[5]      = 7, 7, 7, 7, 1;
   .VAR/DMCONST $low_pass_butterworth_filter_order_10_filter_types[5]     = 7, 7, 7, 7, 7;
   .VAR/DMCONST $low_pass_butterworth_filter_order_11_filter_types[6]     = 7, 7, 7, 7, 7, 1;
   .VAR/DMCONST $low_pass_butterworth_filter_order_12_filter_types[6]     = 7, 7, 7, 7, 7, 7;
   
   // *****************************************************************
   // High-pass Butterworth filter types for all stages under each order
   // *****************************************************************
   .VAR/DMCONST $high_pass_butterworth_filter_order_1_filter_types[1]      = 2;
   .VAR/DMCONST $high_pass_butterworth_filter_order_2_filter_types[1]      = 8;
   .VAR/DMCONST $high_pass_butterworth_filter_order_3_filter_types[2]      = 8, 2;
   .VAR/DMCONST $high_pass_butterworth_filter_order_4_filter_types[2]      = 8, 8;
   .VAR/DMCONST $high_pass_butterworth_filter_order_5_filter_types[3]      = 8, 8, 2;
   .VAR/DMCONST $high_pass_butterworth_filter_order_6_filter_types[3]      = 8, 8, 8;
   .VAR/DMCONST $high_pass_butterworth_filter_order_7_filter_types[4]      = 8, 8, 8, 2;
   .VAR/DMCONST $high_pass_butterworth_filter_order_8_filter_types[4]      = 8, 8, 8, 8;
   .VAR/DMCONST $high_pass_butterworth_filter_order_9_filter_types[5]      = 8, 8, 8, 8, 2;
   .VAR/DMCONST $high_pass_butterworth_filter_order_10_filter_types[5]     = 8, 8, 8, 8, 8;
   .VAR/DMCONST $high_pass_butterworth_filter_order_11_filter_types[6]     = 8, 8, 8, 8, 8, 2;
   .VAR/DMCONST $high_pass_butterworth_filter_order_12_filter_types[6]     = 8, 8, 8, 8, 8, 8;
   
   // ***********************************************************************
   // Butterworth filter Q values(in q8.xx) for all stages under each order
   // ***********************************************************************
   .VAR/DMCONST $butterworth_filter_order_1_Q_values[1]    = Qfmt_(1.000000, 8);
   .VAR/DMCONST $butterworth_filter_order_2_Q_values[1]    = Qfmt_(0.707107, 8);
   .VAR/DMCONST $butterworth_filter_order_3_Q_values[2]    = Qfmt_(1.000000, 8), Qfmt_(1.000000, 8);
   .VAR/DMCONST $butterworth_filter_order_4_Q_values[2]    = Qfmt_(1.306563, 8), Qfmt_(0.541196, 8);
   .VAR/DMCONST $butterworth_filter_order_5_Q_values[3]    = Qfmt_(1.618034, 8), Qfmt_(0.618034, 8), Qfmt_(1.000000, 8);
   .VAR/DMCONST $butterworth_filter_order_6_Q_values[3]    = Qfmt_(1.931852, 8), Qfmt_(0.707107, 8), Qfmt_(0.517638, 8);
   .VAR/DMCONST $butterworth_filter_order_7_Q_values[4]    = Qfmt_(2.246980, 8), Qfmt_(0.801938, 8), Qfmt_(0.554958, 8), Qfmt_(1.000000, 8);
   .VAR/DMCONST $butterworth_filter_order_8_Q_values[4]    = Qfmt_(2.562915, 8), Qfmt_(0.899976, 8), Qfmt_(0.601345, 8), Qfmt_(0.509796, 8);
   .VAR/DMCONST $butterworth_filter_order_9_Q_values[5]    = Qfmt_(2.879385, 8), Qfmt_(1.000000, 8), Qfmt_(0.652704, 8), Qfmt_(0.532089, 8), Qfmt_(1.000000, 8);
   .VAR/DMCONST $butterworth_filter_order_10_Q_values[5]   = Qfmt_(3.196227, 8), Qfmt_(1.101345, 8), Qfmt_(0.707107, 8), Qfmt_(0.561163, 8), Qfmt_(0.506233, 8);
   .VAR/DMCONST $butterworth_filter_order_11_Q_values[6]   = Qfmt_(3.513337, 8), Qfmt_(1.203616, 8), Qfmt_(0.763521, 8), Qfmt_(0.594351, 8), Qfmt_(0.521109, 8), Qfmt_(1.000000, 8);
   .VAR/DMCONST $butterworth_filter_order_12_Q_values[6]   = Qfmt_(3.830649, 8), Qfmt_(1.306563, 8), Qfmt_(0.821340, 8), Qfmt_(0.630236, 8), Qfmt_(0.541196, 8), Qfmt_(0.504314, 8);
   
   // *****************************************************
   // Butterworth filter parameters for each order
   // ***************************************************** 
   .VAR/DMCONST $butterworth_filter_order_1_params[4]   = &$butterworth_filter_order_1_num_stages  , &$low_pass_butterworth_filter_order_1_filter_types  , &$high_pass_butterworth_filter_order_1_filter_types  ,  &$butterworth_filter_order_1_Q_values;
   .VAR/DMCONST $butterworth_filter_order_2_params[4]   = &$butterworth_filter_order_2_num_stages  , &$low_pass_butterworth_filter_order_2_filter_types  , &$high_pass_butterworth_filter_order_2_filter_types  ,  &$butterworth_filter_order_2_Q_values;
   .VAR/DMCONST $butterworth_filter_order_3_params[4]   = &$butterworth_filter_order_3_num_stages  , &$low_pass_butterworth_filter_order_3_filter_types  , &$high_pass_butterworth_filter_order_3_filter_types  ,  &$butterworth_filter_order_3_Q_values;
   .VAR/DMCONST $butterworth_filter_order_4_params[4]   = &$butterworth_filter_order_4_num_stages  , &$low_pass_butterworth_filter_order_4_filter_types  , &$high_pass_butterworth_filter_order_4_filter_types  ,  &$butterworth_filter_order_4_Q_values;
   .VAR/DMCONST $butterworth_filter_order_5_params[4]   = &$butterworth_filter_order_5_num_stages  , &$low_pass_butterworth_filter_order_5_filter_types  , &$high_pass_butterworth_filter_order_5_filter_types  ,  &$butterworth_filter_order_5_Q_values;
   .VAR/DMCONST $butterworth_filter_order_6_params[4]   = &$butterworth_filter_order_6_num_stages  , &$low_pass_butterworth_filter_order_6_filter_types  , &$high_pass_butterworth_filter_order_6_filter_types  ,  &$butterworth_filter_order_6_Q_values;
   .VAR/DMCONST $butterworth_filter_order_7_params[4]   = &$butterworth_filter_order_7_num_stages  , &$low_pass_butterworth_filter_order_7_filter_types  , &$high_pass_butterworth_filter_order_7_filter_types  ,  &$butterworth_filter_order_7_Q_values;
   .VAR/DMCONST $butterworth_filter_order_8_params[4]   = &$butterworth_filter_order_8_num_stages  , &$low_pass_butterworth_filter_order_8_filter_types  , &$high_pass_butterworth_filter_order_8_filter_types  ,  &$butterworth_filter_order_8_Q_values;
   .VAR/DMCONST $butterworth_filter_order_9_params[4]   = &$butterworth_filter_order_9_num_stages  , &$low_pass_butterworth_filter_order_9_filter_types  , &$high_pass_butterworth_filter_order_9_filter_types  ,  &$butterworth_filter_order_9_Q_values;
   .VAR/DMCONST $butterworth_filter_order_10_params[4]  = &$butterworth_filter_order_10_num_stages , &$low_pass_butterworth_filter_order_10_filter_types , &$high_pass_butterworth_filter_order_10_filter_types ,  &$butterworth_filter_order_10_Q_values;
   .VAR/DMCONST $butterworth_filter_order_11_params[4]  = &$butterworth_filter_order_11_num_stages , &$low_pass_butterworth_filter_order_11_filter_types , &$high_pass_butterworth_filter_order_11_filter_types ,  &$butterworth_filter_order_11_Q_values;
   .VAR/DMCONST $butterworth_filter_order_12_params[4]  = &$butterworth_filter_order_12_num_stages , &$low_pass_butterworth_filter_order_12_filter_types , &$high_pass_butterworth_filter_order_12_filter_types ,  &$butterworth_filter_order_12_Q_values;  
     
   // ************************************
   // Butterworth filter all parameters
   // ************************************
   .VAR/DMCONST $butterworth_filter_all_params[$audio_proc.xover_2band.BW_MAX_CASES] =  &$butterworth_filter_order_1_params  , &$butterworth_filter_order_2_params   ,  &$butterworth_filter_order_3_params ,
                                                                                &$butterworth_filter_order_4_params  , &$butterworth_filter_order_5_params   ,  &$butterworth_filter_order_6_params ,
                                                                                &$butterworth_filter_order_7_params  , &$butterworth_filter_order_8_params   ,  &$butterworth_filter_order_9_params ,
                                                                                &$butterworth_filter_order_10_params , &$butterworth_filter_order_11_params  ,  &$butterworth_filter_order_12_params; 
     
     
     
      
   // *********************************************************************************
   //                                    LINKWITZ-RILEY FILTER
   // *********************************************************************************
   
   // *****************************************************
   // linkwitz-riley filter number of stages for each order
   // *****************************************************
   .VAR/DMCONST $linkwitz_riley_filter_order_2_num_stages   =  1;
   .VAR/DMCONST $linkwitz_riley_filter_order_4_num_stages   =  2;
   .VAR/DMCONST $linkwitz_riley_filter_order_6_num_stages   =  3;
   .VAR/DMCONST $linkwitz_riley_filter_order_8_num_stages   =  4;
   .VAR/DMCONST $linkwitz_riley_filter_order_10_num_stages  =  5;
   .VAR/DMCONST $linkwitz_riley_filter_order_12_num_stages  =  6;
   
   // ********************************************************************
   // Low-pass linkwitz-riley filter types for all stages under each order
   // ********************************************************************
   .VAR/DMCONST $low_pass_linkwitz_riley_filter_order_2_filter_types[1]       = 7;
   .VAR/DMCONST $low_pass_linkwitz_riley_filter_order_4_filter_types[2]       = 7, 7;
   .VAR/DMCONST $low_pass_linkwitz_riley_filter_order_6_filter_types[3]       = 7, 7, 7;
   .VAR/DMCONST $low_pass_linkwitz_riley_filter_order_8_filter_types[4]       = 7, 7, 7, 7;
   .VAR/DMCONST $low_pass_linkwitz_riley_filter_order_10_filter_types[5]      = 7, 7, 7, 7, 7;
   .VAR/DMCONST $low_pass_linkwitz_riley_filter_order_12_filter_types[6]      = 7, 7, 7, 7, 7, 7;
   
   // ********************************************************************
   // High-pass linkwitz-riley filter types for all stages under each order
   // ********************************************************************
   .VAR/DMCONST $high_pass_linkwitz_riley_filter_order_2_filter_types[1]       = 8;
   .VAR/DMCONST $high_pass_linkwitz_riley_filter_order_4_filter_types[2]       = 8, 8;
   .VAR/DMCONST $high_pass_linkwitz_riley_filter_order_6_filter_types[3]       = 8, 8, 8;
   .VAR/DMCONST $high_pass_linkwitz_riley_filter_order_8_filter_types[4]       = 8, 8, 8, 8;
   .VAR/DMCONST $high_pass_linkwitz_riley_filter_order_10_filter_types[5]      = 8, 8, 8, 8, 8;
   .VAR/DMCONST $high_pass_linkwitz_riley_filter_order_12_filter_types[6]      = 8, 8, 8, 8, 8, 8;
   
   // ***********************************************************************
   // linkwitz-riley filter Q values(in q8.xx) for all stages under each order
   // ***********************************************************************
   .VAR/DMCONST $linkwitz_riley_filter_order_2_Q_values[1]    = Qfmt_(0.500000, 8);
   .VAR/DMCONST $linkwitz_riley_filter_order_4_Q_values[2]    = Qfmt_(0.707107, 8), Qfmt_(0.707107, 8);
   .VAR/DMCONST $linkwitz_riley_filter_order_6_Q_values[3]    = Qfmt_(1.000000, 8), Qfmt_(0.500000, 8), Qfmt_(1.000000, 8);
   .VAR/DMCONST $linkwitz_riley_filter_order_8_Q_values[4]    = Qfmt_(1.306563, 8), Qfmt_(0.541196, 8), Qfmt_(0.541196, 8), Qfmt_(1.306563, 8);
   .VAR/DMCONST $linkwitz_riley_filter_order_10_Q_values[5]   = Qfmt_(1.618034, 8), Qfmt_(0.618034, 8), Qfmt_(0.500000, 8), Qfmt_(0.618034, 8), Qfmt_(1.618034, 8);
   .VAR/DMCONST $linkwitz_riley_filter_order_12_Q_values[6]   = Qfmt_(1.931852, 8), Qfmt_(0.707107, 8), Qfmt_(0.517638, 8), Qfmt_(0.517638, 8), Qfmt_(0.707107, 8), Qfmt_(1.931852, 8);
    
   // *****************************************************
   // linkwitz-riley filter parameters for each order
   // ***************************************************** 
   .VAR/DMCONST $linkwitz_riley_filter_order_2_params[4]    = &$linkwitz_riley_filter_order_2_num_stages  , &$low_pass_linkwitz_riley_filter_order_2_filter_types  , &$high_pass_linkwitz_riley_filter_order_2_filter_types  ,  &$linkwitz_riley_filter_order_2_Q_values;
   .VAR/DMCONST $linkwitz_riley_filter_order_4_params[4]    = &$linkwitz_riley_filter_order_4_num_stages  , &$low_pass_linkwitz_riley_filter_order_4_filter_types  , &$high_pass_linkwitz_riley_filter_order_4_filter_types  ,  &$linkwitz_riley_filter_order_4_Q_values;
   .VAR/DMCONST $linkwitz_riley_filter_order_6_params[4]    = &$linkwitz_riley_filter_order_6_num_stages  , &$low_pass_linkwitz_riley_filter_order_6_filter_types  , &$high_pass_linkwitz_riley_filter_order_6_filter_types  ,  &$linkwitz_riley_filter_order_6_Q_values;
   .VAR/DMCONST $linkwitz_riley_filter_order_8_params[4]    = &$linkwitz_riley_filter_order_8_num_stages  , &$low_pass_linkwitz_riley_filter_order_8_filter_types  , &$high_pass_linkwitz_riley_filter_order_8_filter_types  ,  &$linkwitz_riley_filter_order_8_Q_values;
   .VAR/DMCONST $linkwitz_riley_filter_order_10_params[4]   = &$linkwitz_riley_filter_order_10_num_stages , &$low_pass_linkwitz_riley_filter_order_10_filter_types , &$high_pass_linkwitz_riley_filter_order_10_filter_types ,  &$linkwitz_riley_filter_order_10_Q_values;
   .VAR/DMCONST $linkwitz_riley_filter_order_12_params[4]   = &$linkwitz_riley_filter_order_12_num_stages , &$low_pass_linkwitz_riley_filter_order_12_filter_types , &$high_pass_linkwitz_riley_filter_order_12_filter_types ,  &$linkwitz_riley_filter_order_12_Q_values;
     
   // ************************************
   // linkwitz-riley filter all parameters
   // ************************************
   .VAR/DMCONST $linkwitz_riley_filter_all_params[$audio_proc.xover_2band.LR_MAX_CASES] =  &$linkwitz_riley_filter_order_2_params , &$linkwitz_riley_filter_order_4_params  , &$linkwitz_riley_filter_order_6_params ,
                                                                                   &$linkwitz_riley_filter_order_8_params , &$linkwitz_riley_filter_order_10_params , &$linkwitz_riley_filter_order_12_params;
                                                                                   
                                                                                   
                                                                                   
   // **************************************
   // Headroom bits for Butterworth filter
   // **************************************
   .VAR/DMCONST $bw_filter_headroom_bits[$audio_proc.xover_2band.BW_MAX_CASES] = 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2;
   
   // ******************************************
   // Headroom bits for linkwitz-riley filter
   // ******************************************
   .VAR/DMCONST $lr_filter_headroom_bits[2*$audio_proc.xover_2band.LR_MAX_CASES] = 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1;
                                                                                   
   // ***************************
   // Headroom bits all values
   // ***************************
   .VAR/DMCONST $headroom_bits_all_values[2] = $bw_filter_headroom_bits , $lr_filter_headroom_bits;
                                                                                   
                                                                                                                                                                      
.ENDMODULE;   
