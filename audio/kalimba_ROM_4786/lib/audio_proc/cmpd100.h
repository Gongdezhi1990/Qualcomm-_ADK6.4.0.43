// *****************************************************************************
// Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef CMPD100_LIB_H
#define CMPD100_LIB_H


.CONST  $CMPD100_VERSION                         0x010004;

// cmpd100 data object definitions
.CONST  $cmpd100.OFFSET_CONTROL_WORD            0*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_ENABLE_BIT_MASK         1*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_INPUT_CH1_PTR           2*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_INPUT_CH2_PTR           3*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_OUTPUT_CH1_PTR          4*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_OUTPUT_CH2_PTR          5*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_MAKEUP_GAIN             6*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_GAIN_PTR                7*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_NEG_ONE                 8*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_POW2_NEG4               9*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_EXPAND_THRESHOLD        10*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_LINEAR_THRESHOLD        11*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_COMPRESS_THRESHOLD      12*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_LIMIT_THRESHOLD         13*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_INV_EXPAND_RATIO        14*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_INV_LINEAR_RATIO        15*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_INV_COMPRESS_RATIO      16*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_INV_LIMIT_RATIO         17*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_EXPAND_CONSTANT         18*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_LINEAR_CONSTANT         19*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_COMPRESS_CONSTANT       20*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_EXPAND_ATTACK_TC        21*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_EXPAND_DECAY_TC         22*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_LINEAR_ATTACK_TC        23*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_LINEAR_DECAY_TC         24*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_COMPRESS_ATTACK_TC      25*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_COMPRESS_DECAY_TC       26*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_LIMIT_ATTACK_TC         27*ADDR_PER_WORD;
.CONST  $cmpd100.OFFSET_LIMIT_DECAY_TC          28*ADDR_PER_WORD;

.CONST  $cmpd100.STRUC_SIZE                     29;

#endif
