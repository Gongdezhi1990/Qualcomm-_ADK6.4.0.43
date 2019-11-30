// *****************************************************************************
// Copyright (c) 2015 Qualcomm Technologies International, Ltd.
//
// 
//
// *****************************************************************************

#ifndef _DMSS_LIB_H
#define _DMSS_LIB_H

// -----------------------------------------------------------------------------
// DMSS CONSTANTS
// -----------------------------------------------------------------------------
.CONST $dmss.rnr.nb.STATE_SIZE            3 * 25;
.CONST $dmss.rnr.wb.STATE_SIZE            3 * 30;

.CONST $dmss.tp.DIM                       21;
.CONST $dmss.tp.VECTOR_SIZE               2 * $dmss.tp.DIM;
.CONST $dmss.tp.STATE_DM1_SIZE            4 * $dmss.tp.VECTOR_SIZE;
.CONST $dmss.tp.STATE_DM2_SIZE            4 * $dmss.tp.VECTOR_SIZE;

// TP    :  21*2
// RNR   :  65/129
// TR    :  130 nb/wb
.CONST $dmss.SCRATCH_DM1_SIZE             130;

// TP    :  21*2
// RNR   :  25/30
// TR    :  130
.CONST $dmss.SCRATCH_DM2_SIZE             130;

.CONST $dmss100.NUM_PROC                  65;
.CONST $dmss100.NUM_NB_BINS_WB_MODE       56;

// -----------------------------------------------------------------------------
// DMSS USER PARAMETER
// -----------------------------------------------------------------------------
.CONST $dmss.param.RNR_AGGR_FIELD         0;

// -----------------------------------------------------------------------------
// DMSS DATA OBJECT
// -----------------------------------------------------------------------------
// @DATA_OBJECT         DMSS_DATAOBJECT

// @DOC_FIELD_TEXT Pointer to left channel (X0)(real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $dmss.X0_FIELD                     0*ADDR_PER_WORD;

// @DOC_FIELD_TEXT Pointer to right channel (X1) (real/imag/BExp)
// @DOC_FIELD_FORMAT Pointer
.CONST $dmss.X1_FIELD                     1*ADDR_PER_WORD;

.CONST $dmss.PIZ0_FIELD                   2*ADDR_PER_WORD;
.CONST $dmss.PIZ1_FIELD                   3*ADDR_PER_WORD;
.CONST $dmss.POZ0_FIELD                   4*ADDR_PER_WORD;
.CONST $dmss.POZ1_FIELD                   5*ADDR_PER_WORD;
// consecutive start
.CONST $dmss.SCRATCH_DM1_FIELD            6*ADDR_PER_WORD;
.CONST $dmss.SCRATCH_DM2_FIELD            7*ADDR_PER_WORD;
// consecutive end
.CONST $dmss.PARAM_FIELD                  8*ADDR_PER_WORD;
.CONST $dmss.TABLE_FIELD                  9*ADDR_PER_WORD;
.CONST $dmss.TOTAL_ATT_FIELD             10*ADDR_PER_WORD;
.CONST $dmss.RNR_STATE_FIELD             11*ADDR_PER_WORD;
.CONST $dmss.TP_MODE_PTR_FIELD           12*ADDR_PER_WORD;
.CONST $dmss.TP_STATE_DM1_FIELD          13*ADDR_PER_WORD;
.CONST $dmss.TP_STATE_DM2_FIELD          14*ADDR_PER_WORD;
.CONST $dmss.DMS_PTR_GG_FIELD            15*ADDR_PER_WORD;
.CONST $dmss.OMS_PTR_G_FIELD             16*ADDR_PER_WORD;
.CONST $dmss.BEAM0_TR_FIELD              17*ADDR_PER_WORD;
.CONST $dmss.BEAM1_TR_FIELD              18*ADDR_PER_WORD;
.CONST $dmss.PTR_VARIANT_FIELD           19*ADDR_PER_WORD;

.CONST $dmss.INTERNAL_START_FIELD        20*ADDR_PER_WORD;

// Global variables
.CONST $dmss.NUMBIN_FIELD                $dmss.INTERNAL_START_FIELD;
// Power monitor internal
.CONST $dmss.BEXP_PIZ0_FIELD             ADDR_PER_WORD + $dmss.NUMBIN_FIELD;
.CONST $dmss.BEXP_PIZ1_FIELD             ADDR_PER_WORD + $dmss.BEXP_PIZ0_FIELD;
.CONST $dmss.BEXP_POZ0_FIELD             ADDR_PER_WORD + $dmss.BEXP_PIZ1_FIELD;
.CONST $dmss.BEXP_POZ1_FIELD             ADDR_PER_WORD + $dmss.BEXP_POZ0_FIELD;
// RNR internal
.CONST $dmss.rnr.DIM_FIELD               ADDR_PER_WORD + $dmss.BEXP_POZ1_FIELD;
.CONST $dmss.rnr.GN_FIELD                ADDR_PER_WORD + $dmss.rnr.DIM_FIELD;
.CONST $dmss.rnr.GS_FIELD                ADDR_PER_WORD + $dmss.rnr.GN_FIELD;
.CONST $dmss.rnr.G_FIELD                 ADDR_PER_WORD + $dmss.rnr.GS_FIELD;
.CONST $dmss.rnr.PHSHB_FIELD             ADDR_PER_WORD + $dmss.rnr.G_FIELD;
.CONST $dmss.rnr.ALFAG0_FIELD            ADDR_PER_WORD + $dmss.rnr.PHSHB_FIELD;
.CONST $dmss.rnr.ALFAG_FIELD             ADDR_PER_WORD + $dmss.rnr.ALFAG0_FIELD;
.CONST $dmss.rnr.G_MN_FIELD              ADDR_PER_WORD + $dmss.rnr.ALFAG_FIELD;
.CONST $dmss.rnr.G_ABS_FIELD             ADDR_PER_WORD + $dmss.rnr.G_MN_FIELD;
.CONST $dmss.rnr.G_CEIL_FIELD            ADDR_PER_WORD + $dmss.rnr.G_ABS_FIELD;
.CONST $dmss.rnr.G_FLOOR_FIELD           ADDR_PER_WORD + $dmss.rnr.G_CEIL_FIELD;
.CONST $dmss.rnr.G_FLAG_FIELD            ADDR_PER_WORD + $dmss.rnr.G_FLOOR_FIELD;

// Target Protection internal
// start concecutive
.CONST $dmss.tp.PIN0_FIELD               ADDR_PER_WORD + $dmss.rnr.G_FLAG_FIELD;
.CONST $dmss.tp.PIN1_FIELD               ADDR_PER_WORD + $dmss.tp.PIN0_FIELD;
.CONST $dmss.tp.PINC_REAL_FIELD          ADDR_PER_WORD + $dmss.tp.PIN1_FIELD;
.CONST $dmss.tp.PINC_IMAG_FIELD          ADDR_PER_WORD + $dmss.tp.PINC_REAL_FIELD;
.CONST $dmss.tp.POUT0_FIELD              ADDR_PER_WORD + $dmss.tp.PINC_IMAG_FIELD;
.CONST $dmss.tp.POUT1_FIELD              ADDR_PER_WORD + $dmss.tp.POUT0_FIELD;
.CONST $dmss.tp.POUTC_REAL_FIELD         ADDR_PER_WORD + $dmss.tp.POUT1_FIELD;
.CONST $dmss.tp.POUTC_IMAG_FIELD         ADDR_PER_WORD + $dmss.tp.POUTC_REAL_FIELD;
// end concecutive
.CONST $dmss.tp.MSC_IN_FIELD             ADDR_PER_WORD + $dmss.tp.POUTC_IMAG_FIELD;
.CONST $dmss.tp.MSC_OUT_FIELD            ADDR_PER_WORD + $dmss.tp.MSC_IN_FIELD;
.CONST $dmss.tp.MSC_ADAPT_FLAG_FIELD     ADDR_PER_WORD + $dmss.tp.MSC_OUT_FIELD;
.CONST $dmss.tp.DMSC_FIELD               ADDR_PER_WORD + $dmss.tp.MSC_ADAPT_FLAG_FIELD;
.CONST $dmss.tp.MODE_COUNT_FIELD         ADDR_PER_WORD + $dmss.tp.DMSC_FIELD;

// Target Ratio calculation internal
.CONST $dmss.tr.X_INPUT_FREQ_OBJ         ADDR_PER_WORD + $dmss.tp.MODE_COUNT_FIELD;
.CONST $dmss.tr.X_OUTPUT_FREQ_OBJ        ADDR_PER_WORD + $dmss.tr.X_INPUT_FREQ_OBJ;

.CONST $dmss.STRUC_SIZE     ($dmss.tr.X_OUTPUT_FREQ_OBJ >> LOG2_ADDR_PER_WORD) + 1;

// @END DATA_OBJECT     DMSS_DATAOBJECT

#endif // _DMSS_LIB_H
