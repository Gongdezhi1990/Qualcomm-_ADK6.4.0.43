// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

/********************************************************************************
Include Files
*/
#include "audio_proc/sra_shared_constants.h"

/********************************************************************************
Constant Definitions
*/
.CONST $sra.SRA_UPRATE      UPRATE_AMT; // The SRA upsampling rate

.CONST $sra.MOVING_STEP (0.0015*(1.0/1000.0)/10.0); // 0.0015: interrupt period, this means it would take 8 seconds for 1hz change for a 1khz tone

