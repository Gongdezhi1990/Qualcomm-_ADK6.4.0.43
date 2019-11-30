// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"
#include "codec_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.main
//
// DESCRIPTION:
//    invoke codec functions
//
// INPUTS:
//    - r0 = requested function
//    - Other input arguments depends on requested function
//
// OUTPUTS:
//    - Depends on function called
//
// TRASHED REGISTERS:
//    Depends on function called
//
// *****************************************************************************
.MODULE $M.aacdec.main;
   .CODESEGMENT AACDEC_MAIN_PM;
   .DATASEGMENT DM;

   $aacdec.main:
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($aacdec.AACDEC_API_ASM.AACDEC_MAIN.MAIN.PATCH_ID_0)
#endif
   

   // check functions
   Null = r0 - $codec.FRAME_DECODE;
   if Z jump $aacdec.frame_decode;              // in: r5, out: none, trashed: all
   Null = r0 - $codec.INIT_DECODER;
   if Z jump $aacdec.init_decoder;              // in: none, out: none, trashed: r0-r3
   Null = r0 - $codec.RESET_DECODER;
   if Z jump $aacdec.reset_decoder;             // in: none, out: none, trashed: r0, r10, DoLoop, I0, I4
   Null = r0 - $codec.SILENCE_DECODER;
   if Z jump $aacdec.silence_decoder;           // in: none, out: none, trashed: r0, r10, DoLoop, I0, I4
   Null = r0 - $codec.SUSPEND_DECODER;
   if Z jump $aacdec.suspend_decoder;           // in: I0, out: r0, I0 trashed: r1-2
   Null = r0 - $codec.RESUME_DECODER;
   if Z jump $aacdec.resume_decoder;            // in: I0, out: I0, trashed: r0-2
   Null = r0 - $codec.STORE_BOUNDARY_SNAPSHOT;
   if Z jump $aacdec.store_boundary_snapshot;   // in: I0, out: r0, I0 trashed: r1-2
   Null = r0 - $codec.RESTORE_BOUNDARY_SNAPSHOT;
   if Z jump $aacdec.restore_boundary_snapshot; // in: I0, out: I0 trashed: r0-2
   Null = r0 - $codec.FAST_SKIP;
   if Z jump $aacdec.aac_ff_rew;                // in: r1, r2, out: r4-6, trashed: all

   // accecc interfaces
   Null = r0 - $codec.SET_SKIP_FUNCTION;
   if NZ jump not_set_skip_function;
      // INPUTS:
      //    - r1 = skip function pointer
      // OUTPUTS:
      //    - none
      M[r9 + $aac.mem.SKIP_FUNCTION] = r1;
      rts;
   not_set_skip_function:

   Null = r0 - $codec.SET_AVERAGE_BITRATE;
   if NZ jump not_set_average_bitrate;
      // INPUTS:
      //    - r1 = average bitrate
      // OUTPUTS:
      //    - none
      M[r9 + $aac.mem.AVG_BIT_RATE] = r1;
      rts;
   not_set_average_bitrate:

   Null = r0 - $codec.SET_FILE_TYPE;
   if Z jump $aacdec.set_mp4_file_type_handler; // in: r1 = file type, 0 means mp4, 1 adts, 2 latm, r2,
                                                // out: none, trashed: r1

   // unknown command
   rts;

.ENDMODULE;
