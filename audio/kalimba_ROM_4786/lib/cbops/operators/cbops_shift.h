// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef CBOPS_SHIFT_HEADER_INCLUDED
#define CBOPS_SHIFT_HEADER_INCLUDED

#include "cbops_shift_c_asm_defs.h"

   // parameter structure part for multi-channel cbop, this follows after the 
   // common cbop parameter struct "header" (i.e. after number of in & out channels 
   // and their indexes).
   .CONST   $cbops.shift.SHIFT_AMOUNT_FIELD             $cbops_shift_c.cbops_shift_struct.SHIFT_AMOUNT_FIELD;
   .CONST   $cbops.shift.STRUC_SIZE                     $cbops_shift_c.cbops_shift_struct.STRUC_SIZE;

#endif // CBOPS_SHIFT_HEADER_INCLUDED
