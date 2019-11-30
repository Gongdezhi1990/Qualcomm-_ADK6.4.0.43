// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef _AED100_LIB_H_INCLUDED
#define _AED100_LIB_H_INCLUDED

// -----------------------------------------------------------------------------
// AED100 data object structure definition
// -----------------------------------------------------------------------------
.CONST $aed100.PTR_VAD_FLAG_FIELD                0*ADDR_PER_WORD;
.CONST $aed100.DOA_FIELD                         1*ADDR_PER_WORD;
.CONST $aed100.VOICED_FIELD                      2*ADDR_PER_WORD;
.CONST $aed100.G_IN_FIELD                        3*ADDR_PER_WORD;
.CONST $aed100.G_START_FIELD                     4*ADDR_PER_WORD;
.CONST $aed100.G_LEN_FIELD                       5*ADDR_PER_WORD;
.CONST $aed100.G_FIELD                           6*ADDR_PER_WORD;
.CONST $aed100.GN_FIELD                          7*ADDR_PER_WORD;
.CONST $aed100.GS_FIELD                          8*ADDR_PER_WORD;
.CONST $aed100.GT_FIELD                          9*ADDR_PER_WORD;
.CONST $aed100.VAD_FLAG_FIELD                   10*ADDR_PER_WORD;
.CONST $aed100.STRUC_SIZE                       11;

#endif // _AED100_LIB_H_INCLUDED
