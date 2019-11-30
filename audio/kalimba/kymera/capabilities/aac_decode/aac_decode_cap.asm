/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/



#include "stack.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $_populate_aac_asm_funcs
//
// DESCRIPTION:
//    Populates the capability structure with pointers to the asm functions of
//    the codec
//
// INPUTS:
//    - r0 = address to store the frame_decode function
//    - r1 = address to store the silence function
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.populate_aac_asm_funcs;
   .CODESEGMENT PM_FLASH;

$_populate_aac_asm_funcs:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($aac_decode_cap.AAC_DECODE_CAP_ASM.POPULATE_AAC_ASM_FUNCS.PATCH_ID_0, r2)
#endif

   r2 = $aacdec.frame_decode;
   M[r0] = r2;
   r3 = $aacdec.silence_decoder;
   M[r1] = r2;
   rts;

.ENDMODULE;

#ifdef INSTALL_OPERATOR_AAC_SHUNT_DECODER
   
// *****************************************************************************
// MODULE:
//    $_populate_strip_aac_asm_funcs
//
// DESCRIPTION:
//    Populates the capability structure with pointers to the asm functions of
//    the codec
//
// INPUTS:
//    - r0 = address to store the frame_decode function
//    - r1 = address to store the silence function
//    - r2 = address to store bit handling function   
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.populate_strip_aac_asm_funcs;
   .CODESEGMENT PM_FLASH;
   
$_populate_strip_aac_asm_funcs:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($aac_decode_cap.AAC_DECODE_CAP_ASM.POPULATE_STRIP_AAC_ASM_FUNCS.PATCH_ID_0, r3)
#endif

   r3 = $aac_a2dp_strip_and_frame_decode;
   M[r0] = r3;
   /* Are we going to call the silence decoder? It may be irrelevant
    * time being, no stripping function for this. TBD*/
   r3 = $aacdec.silence_decoder;
   M[r1] = r3;
   r3 = $aacdec.get1byte;   
   M[r2] = r3;
   rts;   

.ENDMODULE;

// *****************************************************************************
// MODULE:
//   $aac_a2dp_strip_and_frame_decode
//
// DESCRIPTION:
//    Populates the capability structure with pointers to the asm functions of
//    the codec
//
// INPUTS:
//    - r5 =  The codec structure
//    - r3 =  Pointer to a2dp header structure   
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.aac_a2dp_strip_and_frame_decode;
   .CODESEGMENT PM_FLASH;

$aac_a2dp_strip_and_frame_decode:   
   push rLink;
   
   r7 = r3;
   r8 =  $aac_a2dp_frame_decode;
   r9 =  M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
   call  $a2dp_strip_and_frame_decode;
   
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//   $aac_a2dp_frame_decode
//
// DESCRIPTION:
//    It calls aac decode library and returns the size of frame
//    after a successful decode.
//
// INPUTS:
//    - r5 =  The codec structure
//
// OUTPUTS:
//    - r6 - decode status
//    - r0 - Number of bytes decoded.
//
// TRASHED REGISTERS:
//    
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.aac_a2dp_frame_decode;
   .CODESEGMENT PM_FLASH;

$aac_a2dp_frame_decode:   
   push rLink;
   
   call $aacdec.frame_decode;
   r6 = M[r5 + $codec.DECODER_MODE_FIELD];
   r0 = 0;
   
   Null = r6 - $codec.SUCCESS;
   if NE jump done;
      

   done:   
   jump $pop_rLink_and_rts;
.ENDMODULE;
   
#endif /* INSTALL_OPERATOR_AAC_SHUNT_DECODER */
