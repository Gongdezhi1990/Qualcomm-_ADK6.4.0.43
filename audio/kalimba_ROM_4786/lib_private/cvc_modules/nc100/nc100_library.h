// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef _NC100_LIB_H
#define  _NC100_LIB_H

// -----------------------------------------------------------------------------
// HISTORY:
//    12-11-08 Wed wms 1.0.0: Initial Version, from ADF100
// -----------------------------------------------------------------------------
.CONST  $NC_VERSION                       0x010000;


// -----------------------------------------------------------------------------
// EXTERNAL CONSTANTS
// -----------------------------------------------------------------------------
.CONST $nc100.NB_Num_FFT_Freq_Bins    65;
.CONST $nc100.WB_Num_FFT_Freq_Bins   129;
.CONST $nc100.BIN_SKIPPED              1;
.CONST $nc100.NUM_PROC                63;
//.CONST $nc100.NUM_TAPS               5;
.CONST $nc100.NUM_TAPS_MAX             6;

.CONST $nc100.DM1_DATA_SIZE		   ((2 + 2*$nc100.NUM_TAPS_MAX) * $nc100.NUM_PROC);
.CONST $nc100.DM2_DATA_SIZE		   ((0 + 2*$nc100.NUM_TAPS_MAX) * $nc100.NUM_PROC + $nc100.NUM_TAPS_MAX+1);
.CONST $nc100.DM1_SCRATCH_SIZE      ((0 + 3*$nc100.NUM_PROC + 4*$nc100.NB_Num_FFT_Freq_Bins));

// -----------------------------------------------------------------------------
// DATA OBJECT
// -----------------------------------------------------------------------------
// @DATA_OBJECT         NC_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to left channel (X0)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.X0_FIELD                              0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to right channel (X1) (real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.X1_FIELD                              1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to NC Parameter  @DOC_LINK @DATA_OBJECT NC_PARAM_OBJECT
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.PARAM_FIELD                           2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT NC L_alfaPx[] Pointer.
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.L_ALFAPX_PTR_FIELD                    3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT NC LRatio_interpolated Pointer.
// @DOC_FIELD_FORMAT Pointer 
.CONST $nc100.LRATIO_PTR_FIELD                      4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT NC SNR_mn Field.
// @DOC_FIELD_FORMAT 
.CONST $nc100.SNR_MN_FIELD                          5*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DM1 data block, with size of '$nc100.DM1_DATA_SIZE'
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.DM1_DATA_FIELD                        6*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DM2 data block, with size of '$nc100.DM2_DATA_SIZE'
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.DM2_DATA_FIELD                        7*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to DM1 scratch block, with size of '$nc100.DM1_SCRATCH_SIZE'
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.DM1_SCRATCH_FIELD                     8*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to NC_ctrl[] buffer, with size of $nc100.NUM_PROC
// @DOC_FIELD_TEXT DM2 allocation is recommended.
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.NC_CTRL_FIELD                         9*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to OMS G array
// @DOC_FIELD_FORMAT Pointer
.CONST $nc100.G_OMS_FIELD                          10*ADDR_PER_WORD;

.CONST $nc100.OFFSET_FNLMS_E_FREQ_OBJ              11*ADDR_PER_WORD;
.CONST $nc100.OFFSET_FNLMS_D_FREQ_OBJ              12*ADDR_PER_WORD;
.CONST $nc100.PTR_VARIANT_FIELD                    13*ADDR_PER_WORD;

//
// Internal fields
//
.CONST $nc100.NUM_FFT_FREQ_BIN_FIELD               14*ADDR_PER_WORD;
.CONST $nc100.NTAPS_FIELD                          15*ADDR_PER_WORD;                      
.CONST $nc100.LMUP_FIELD                           16*ADDR_PER_WORD;  
.CONST $nc100.L_ALFAPX_FIELD                       17*ADDR_PER_WORD;
.CONST $nc100.X0_WB2NB_REAL_FIELD                  18*ADDR_PER_WORD;
.CONST $nc100.X0_WB2NB_IMAG_FIELD                  19*ADDR_PER_WORD;
.CONST $nc100.X0_WB2NB_EXP_FIELD                   20*ADDR_PER_WORD;  
.CONST $nc100.X1_WB2NB_REAL_FIELD                  21*ADDR_PER_WORD;
.CONST $nc100.X1_WB2NB_IMAG_FIELD                  22*ADDR_PER_WORD;        
.CONST $nc100.X1_WB2NB_EXP_FIELD                   23*ADDR_PER_WORD;
.CONST $nc100.CONST_ONE_FIELD                      24*ADDR_PER_WORD;
.CONST $nc100.FNLMS_X_REAL_FIELD                   25*ADDR_PER_WORD;
.CONST $nc100.FNLMS_X_IMAG_FIELD                   26*ADDR_PER_WORD;
.CONST $nc100.FNLMS_X_BEXP_FIELD                   27*ADDR_PER_WORD;
.CONST $nc100.FNLMS_XBUF_REAL_FIELD                28*ADDR_PER_WORD;
.CONST $nc100.FNLMS_XBUF_IMAG_FIELD                29*ADDR_PER_WORD;
.CONST $nc100.FNLMS_XBUF_BEXP_FIELD                30*ADDR_PER_WORD;
.CONST $nc100.FNLMS_GP_REAL_FIELD                  31*ADDR_PER_WORD;
.CONST $nc100.FNLMS_GP_IMAG_FIELD                  32*ADDR_PER_WORD;
.CONST $nc100.FNLMS_GP_BEXP_FIELD                  33*ADDR_PER_WORD;

.CONST $nc100.FNLMS_FIELD                          34*ADDR_PER_WORD;
// Pointer to a scratch memory in DM2 with size of '2*$nc100.NUM_PROC + 1'
.CONST $nc100.OFFSET_FNLMS_SCRPTR_Exp_Mts_adapt    0 + $nc100.FNLMS_FIELD;
.CONST $nc100.FNLMS_END_FIELD                      $nc100.OFFSET_FNLMS_SCRPTR_Exp_Mts_adapt;
// SP.  End Match

.CONST $nc100.STRUCT_SIZE   ($nc100.FNLMS_END_FIELD >> LOG2_ADDR_PER_WORD) + 1;

// @END DATA_OBJECT     NC_DATAOBJECT


// -----------------------------------------------------------------------------
// NC100 Parameter Structure
// -----------------------------------------------------------------------------
// @DATA_OBJECT NC_PARAM_OBJECT

// @DOC_FIELD_TEXT  NC_TAP
// @DOC_FIELD_FORMAT Q1.23
.CONST $nc100.param.NC_TAP_FIELD                   0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT  NC    Aggresive, default 0.8
// @DOC_FIELD_FORMAT    Q1.23 format
.CONST $nc100.param.AGGRESIVE_FIELD                1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT  NC RptP, default 1
// @DOC_FIELD_FORMAT Q1.23
.CONST $nc100.param.RPTP_FIELD                     2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT  NC ctrl_bias
// @DOC_FIELD_FORMAT Q1.23
.CONST $nc100.param.CTRL_BIAS_FIELD                3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT  NC ctrl_trans, default 2
// @DOC_FIELD_FORMAT 
.CONST $nc100.param.CTRL_TRANS_FIELD               4*ADDR_PER_WORD;


// @END  DATA_OBJECT DMS_PARAM_OBJECT


#endif // _NC100_LIB_H


