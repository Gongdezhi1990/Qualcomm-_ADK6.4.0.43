// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef _MGDC100_LIB_H
#define _MGDC100_LIB_H

// -----------------------------------------------------------------------------
// MGDC DATA OBJECT
// -----------------------------------------------------------------------------
// @DATA_OBJECT         MGDC_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to left channel (X0)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $mgdc100.X0_FIELD                     0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to right channel (X1) (real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $mgdc100.X1_FIELD                     1*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to Parameters     @DOC_LINK @DATA_OBJECT  MGDC_PARAM
// @DOC_FIELD_FORMAT Pointer
.CONST $mgdc100.PARAM_FIELD                  2*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to the varialble holding 'number of fft bins'
// @DOC_FIELD_FORMAT Pointer
.CONST $mgdc100.PTR_VARIANT_FIELD            3*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to an external application mode flag
// @DOC_FIELD_TEXT Output: 2-mic mode: 0, 1-mic mode: non-zero
// @DOC_FIELD_FORMAT Pointer
.CONST $mgdc100.PTR_MICMODE_FIELD            4*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to G (OMS/DMS)
// @DOC_FIELD_FORMAT Pointer
.CONST $mgdc100.PTR_G_OMS_FIELD              5*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to an external Diff_LPz variable
// @DOC_FIELD_FORMAT Pointer
.CONST $mgdc100.PTR_OMS_VAD_FIELD            6*ADDR_PER_WORD;

// @DOC_FIELD_TEXT MGDC Maximum compensation (in dB), Q8.16 (arch4: Q8.24)
// @DOC_FIELD_FORMAT Q8.16 (arch4: Q8.24)
.CONST $mgdc100.FRONTMICBIAS_FIELD           7*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Internal, default 0, Q8.16 (arch4: Q8.24), set by headset system via PS-key
// @DOC_FIELD_FORMAT Q8.16 (arch4: Q8.24)
.CONST $mgdc100.L2FBPXD_FIELD                8*ADDR_PER_WORD;

.CONST $mgdc100.OMNI_MODE_FIELD              9*ADDR_PER_WORD;

// Internal fields
.CONST $mgdc100.MAXCOMP_FIELD                10*ADDR_PER_WORD;
.CONST $mgdc100.TH0_FIELD                    11*ADDR_PER_WORD;
.CONST $mgdc100.TH1_FIELD                    12*ADDR_PER_WORD;
.CONST $mgdc100.L2FBPX0T_FIELD               13*ADDR_PER_WORD;
.CONST $mgdc100.L2FBPX1T_FIELD               14*ADDR_PER_WORD;
.CONST $mgdc100.L2FBPXDST_FIELD              15*ADDR_PER_WORD;
.CONST $mgdc100.EXP_GAIN_FIELD               16*ADDR_PER_WORD;
.CONST $mgdc100.MTS_GAIN_FIELD               17*ADDR_PER_WORD;
.CONST $mgdc100.MEAN_OMS_G_FIELD             18*ADDR_PER_WORD;
.CONST $mgdc100.HOLD_ADAPT_FIELD             19*ADDR_PER_WORD;
.CONST $mgdc100.SWITCH_OUTPUT_FIELD          20*ADDR_PER_WORD;
.CONST $mgdc100.MGDC_UPDATE_FIELD            21*ADDR_PER_WORD;
.CONST $mgdc100.NUM_BIN_FIELD                22*ADDR_PER_WORD;
.CONST $mgdc100.BIN_SKIPPED_FIELD            23*ADDR_PER_WORD;
.CONST $mgdc100.DIM_FIELD                    24*ADDR_PER_WORD;
.CONST $mgdc100.ALFAD_FIELD                  25*ADDR_PER_WORD;
.CONST $mgdc100.ALFAD_ST_FIELD               26*ADDR_PER_WORD;
.CONST $mgdc100.STRUC_SIZE                   27;

// @END DATA_OBJECT     MGDC_DATAOBJECT

// -----------------------------------------------------------------------------
// MGDC USER PARAMETER STRUCTURE
// -----------------------------------------------------------------------------
// @DATA_OBJECT         MGDC_PARAM

// @DOC_FIELD_TEXT MGDC Maximum compensation, default 1.99315685693241720 in Q8.16
// @DOC_FIELD_FORMAT Q8.16 (arch4: Q8.24)
.CONST $mgdc100.param.MAXCOMP_FIELD          0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Threshold for detection of channel lost
// @DOC_FIELD_FORMAT Q8.16 (arch4: Q8.24)
.CONST $mgdc100.param.TH_FIELD               1*ADDR_PER_WORD;

// @END DATA_OBJECT     MGDC_PARAM

#endif // _MGDC100_LIB_H
