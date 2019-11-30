// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef _HARMONICITY_H
#define _HARMONICITY_H

// -----------------------------------------------------------------------------
// Constants - internal
// -----------------------------------------------------------------------------
.CONST $harmonicity.DELAY_LENGTH                27;

.CONST $harmonicity.nb.FRAME_SIZE               60;
.CONST $harmonicity.nb.NUM_FRAME                3;
.CONST $harmonicity.nb.DELAY_START              53;
.CONST $harmonicity.nb.DECIMATION               2;
.CONST $harmonicity.nb.WINDOW_LENGTH            30;

.CONST $harmonicity.wb.FRAME_SIZE               120;
.CONST $harmonicity.wb.NUM_FRAME                3;
.CONST $harmonicity.wb.DELAY_START              107;
.CONST $harmonicity.wb.DECIMATION               4;
.CONST $harmonicity.wb.WINDOW_LENGTH            30;

.CONST $harmonicity.ub.FRAME_SIZE               120;
.CONST $harmonicity.ub.NUM_FRAME                4;
.CONST $harmonicity.ub.DELAY_START              160;
.CONST $harmonicity.ub.DECIMATION               6;
.CONST $harmonicity.ub.WINDOW_LENGTH            20;

// -----------------------------------------------------------------------------
// Data Object
// -----------------------------------------------------------------------------

// @DATA_OBJECT HARMONICITY_OBJECT

// @DOC_FIELD_TEXT Pointer to input frame of time signal 'x'.
// @DOC_FIELD_FORMAT Pointer
.CONST $harmonicity.INP_X_FIELD                 MK1 * 0;

// @DOC_FIELD_TEXT Pointer to Variant Flag (0/1/2/3/4 - nb/wb/uwb/sb/fb)
// @DOC_FIELD_FORMAT Flag Pointer
.CONST $harmonicity.VARIANT_FIELD               MK1 * 1;

// @DOC_FIELD_TEXT Pointer to history buffer, nb/wb require 3 frames, ub 4 frames
// @DOC_FIELD_FORMAT Pointer
.CONST $harmonicity.HISTORY_FIELD               MK1 * 2;

// @DOC_FIELD_TEXT Flag to turn on/off the process, flag set is on, flag clear is off.
// @DOC_FIELD_FORMAT Flag
.CONST $harmonicity.FLAG_BYPASS_FIELD           MK1 * 3;

// Internal fields
.CONST $harmonicity.HARM_VALUE_FIELD            MK1 * 4;
.CONST $harmonicity.FRAME_SIZE_FIELD            MK1 * 5;
.CONST $harmonicity.DELAY_START_FIELD           MK1 * 6;
.CONST $harmonicity.DECIMATION_FIELD            MK1 * 7;
.CONST $harmonicity.NUM_FRAME_FIELD             MK1 * 8;
.CONST $harmonicity.CURRENT_FRAME_FIELD         MK1 * 9;
.CONST $harmonicity.WINDOW_LENGTH_FIELD         MK1 * 10;
.CONST $harmonicity.AMDF_MEM_START_FIELD        MK1 * 11;

.CONST $harmonicity.STRUC_SIZE                 ($harmonicity.AMDF_MEM_START_FIELD  >> LOG2_ADDR_PER_WORD) + $harmonicity.DELAY_LENGTH;

// @END  DATA_OBJECT HARMONICITY_OBJECT

#endif // _HARMONICITY_H


