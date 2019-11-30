// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBC_API_INCLUDED
#define SBC_API_INCLUDED

#include "sbc.h"

.MODULE $M.sbcdec.get_sampling_frequency;
   .CODESEGMENT SBCDEC_GET_SAMPLING_FREQUENCY_PM;
   .DATASEGMENT DM;
    $sbcdec.get_sampling_frequency:
     .VAR sampling_freq_hz[4] = 16000, 32000, 44100, 48000;


      r0 = M[r9 + $sbc.mem.BITPOOL_FIELD];
      if Z rts;
      r0 = M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD];

      r0 = M[r0 + sampling_freq_hz];
      rts;
.ENDMODULE;

#endif
#endif
