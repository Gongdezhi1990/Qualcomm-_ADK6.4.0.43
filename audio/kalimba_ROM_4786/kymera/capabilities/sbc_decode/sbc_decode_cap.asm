/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "stack.h"

#ifdef PATCH_LIBS
#include "patch_library.h"
#endif



// *****************************************************************************
// MODULE:
//    $_populate_sbc_asm_funcs
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
.MODULE $M.populate_sbc_asm_funcs;
   .CODESEGMENT PM_FLASH;

$_populate_sbc_asm_funcs:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($sbc_decode_cap.SBC_DECODE_CAP_ASM.POPULATE_SBC_ASM_FUNCS._POPULATE_SBC_ASM_FUNCS.PATCH_ID_0, r2)   
#endif   


   r2 = $sbcdec.frame_decode;
   M[r0] = r2;
   r3 = $sbcdec.silence_decoder;
   M[r1] = r2;
   rts;

.ENDMODULE;

#ifdef INSTALL_OPERATOR_SBC_SHUNT_DECODER 
   
// *****************************************************************************
// MODULE:
//    $_populate_strip_sbc_asm_funcs
//
// DESCRIPTION:
//    Populates the capability structure with pointers to the asm functions of
//    the codec
//
// INPUTS:
//    - r0 = address to store the entry function 
//    - r1 = address to store the decoder fuction
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
.MODULE $M.populate_strip_sbc_asm_funcs;
   .CODESEGMENT PM_FLASH;
   
$_populate_strip_sbc_asm_funcs:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($sbc_decode_cap.SBC_DECODE_CAP_ASM.POPULATE_STRIP_SBC_ASM_FUNCS._POPULATE_STRIP_SBC_ASM_FUNCS.PATCH_ID_0, r3)   
#endif   



   r3 = $a2dp_strip_and_frame_decode; 
   M[r0] = r3;
   r3 = $sbc_a2dp_frame_decode;
   M[r1] = r3;
   r3 = $sbcdec.get1byte;   
   M[r2] = r3;
   rts;   

.ENDMODULE;


// *****************************************************************************
// MODULE:
//   $sbc_a2dp_frame_decode
//
// DESCRIPTION:
//    It calls sbc decode library and returns the size of frame
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
.MODULE $M.sbc_a2dp_frame_decode;
   .CODESEGMENT PM_FLASH;

$sbc_a2dp_frame_decode:   
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbc_decode_cap.SBC_DECODE_CAP_ASM.SBC_A2DP_FRAME_DECODE.SBC_A2DP_FRAME_DECODE.PATCH_ID_0, r1)   
#endif   
   
   call $sbcdec.frame_decode;
   r6 = M[r5 + $codec.DECODER_MODE_FIELD];
   r0 = 0;
   
   Null = r6 - $codec.SUCCESS;
   if NE jump done;
      
   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
   call $sbc.calc_frame_length;
   
   done:   
   jump $pop_rLink_and_rts;
.ENDMODULE;
   
#endif /* INSTALL_OPERATOR_SBC_SHUNT_DECODER */
