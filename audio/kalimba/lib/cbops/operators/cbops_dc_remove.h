// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************


#ifndef CBOPS_DC_REMOVE_HEADER_INCLUDED
#define CBOPS_DC_REMOVE_HEADER_INCLUDED

#include "cbops_dc_remove_c_asm_defs.h"

   // Params for the multi-channel variant. Offset if from the start of the
   // cbop-specific param area that follows after the common param struct header.
   // There is one DC estimate per channel (some channels may be unused after creation).
   .CONST   $cbops.dc_remove.DC_ESTIMATE_FIELD          $cbops_dc_remove_c.cbops_dc_remove_struct.DC_ESTIMATE_FIELD;
   .CONST   $cbops.dc_remove.STRUC_SIZE                 $cbops_dc_remove_c.cbops_dc_remove_struct.STRUC_SIZE;

   // with fs=48KHz. Value of 0.0005 gives a 3dB point at 4Hz. (0.1dB @ 25Hz)
   // this is assumed to be acceptable for all sample rates
   // value of 0.0003 gives better than 0.1dB at 20Hz which is required by
   // some applications
   .CONST   $cbops.dc_remove.FILTER_COEF                      0.0003;

#endif // CBOPS_DC_REMOVE_HEADER_INCLUDED
