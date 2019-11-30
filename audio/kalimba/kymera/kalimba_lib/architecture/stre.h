//  Copyright (c) 2015 Qualcomm Technologies International, Ltd.
//  All Rights Reserved.
//  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
//  Notifications and licenses are retained for attribution purposes only

#include "io_map.asm"
#include "io_defs.asm"

// Extra defines to work with assembly code

// -- Clock rate constants --
// There is only one clock divide rate in amber, full speed.
   .CONST $CLK_DIV_1                          1;

// -- Flag constants --
//  These can be removed if and when B-107858 is fixed.
   .CONST $N_FLAG                         1;
   .CONST $Z_FLAG                         2;
   .CONST $C_FLAG                         4;
   .CONST $V_FLAG                         8;
   .CONST $UD_FLAG                        16;
   .CONST $SV_FLAG                        32;
   .CONST $BR_FLAG                        64;
   .CONST $UM_FLAG                        128;

   .CONST $NOT_N_FLAG                     (65535-$N_FLAG);
   .CONST $NOT_Z_FLAG                     (65535-$Z_FLAG);
   .CONST $NOT_C_FLAG                     (65535-$C_FLAG);
   .CONST $NOT_V_FLAG                     (65535-$V_FLAG);
   .CONST $NOT_UD_FLAG                    (65535-$UD_FLAG);
   .CONST $NOT_SV_FLAG                    (65535-$SV_FLAG);
   .CONST $NOT_BR_FLAG                    (65535-$BR_FLAG);
   .CONST $NOT_UM_FLAG                    (65535-$UM_FLAG);

