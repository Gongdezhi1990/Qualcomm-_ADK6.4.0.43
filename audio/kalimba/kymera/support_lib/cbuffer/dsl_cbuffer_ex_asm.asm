/**************************************************************************/
/* Copyright (c) 2018-2018 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declared in cbuffer_c.h, are resolved
 * to absolute addresses exported from the patch build.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"

#if !defined(PATCH_ENTRY_CBUFFER_COPY_EX)
#include "cbuffer_ex_asm.asm"
#else

#ifdef DISABLE_PATCH_BUILD_ID_CHECK

.const $_cbuffer_copy_ex                       PATCH_ENTRY_CBUFFER_COPY_EX;
.const $_cbuffer_get_read_offset_ex            PATCH_ENTRY_CBUFFER_GET_READ_OFFSET_EX;
.const $_cbuffer_get_usable_octets             PATCH_ENTRY_CBUFFER_GET_USABLE_OCTETS;
.const $_cbuffer_set_usable_octets             PATCH_ENTRY_CBUFFER_SET_USABLE_OCTETS;
.const $_cbuffer_get_read_address_ex           PATCH_ENTRY_CBUFFER_GET_READ_ADDRESS_EX;
.const $cbuffer.get_read_address_ex            PATCH_ENTRY_CBUFFER_GET_READ_ADDRESS_EX_ASM;
.const $_cbuffer_get_write_address_ex          PATCH_ENTRY_CBUFFER_GET_WRITE_ADDRESS_EX;
.const $cbuffer.get_write_address_ex           PATCH_ENTRY_CBUFFER_GET_WRITE_ADDRESS_EX_ASM;
.const $cbuffer.calc_amount_data_ex            PATCH_ENTRY_CBUFFER_CALC_AMOUNT_DATA_EX;
.const $_cbuffer_calc_amount_data_ex           PATCH_ENTRY_CBUFFER_CALC_AMOUNT_DATA_EX;
.const $cbuffer.calc_amount_space_ex           PATCH_ENTRY_CBUFFER_CALC_AMOUNT_SPACE_EX;
.const $_cbuffer_calc_amount_space_ex          PATCH_ENTRY_CBUFFER_CALC_AMOUNT_SPACE_EX;
.const $cbuffer.set_read_address_ex            PATCH_ENTRY_CBUFFER_SET_READ_ADDRESS_EX;
.const $_cbuffer_set_read_address_ex           PATCH_ENTRY_CBUFFER_SET_READ_ADDRESS_EX;
.const $cbuffer.set_write_address_ex           PATCH_ENTRY_CBUFFER_SET_WRITE_ADDRESS_EX;
.const $_cbuffer_set_write_address_ex          PATCH_ENTRY_CBUFFER_SET_WRITE_ADDRESS_EX;
.const $cbuffer.advance_read_ptr_ex            PATCH_ENTRY_CBUFFER_ADVANCE_READ_PTR_EX;
.const $_cbuffer_advance_read_ptr_ex          PATCH_ENTRY_CBUFFER_ADVANCE_READ_PTR_EX;
.const $cbuffer.advance_write_ptr_ex           PATCH_ENTRY_CBUFFER_ADVANCE_WRITE_PTR_EX;
.const $_cbuffer_advance_write_ptr_ex          PATCH_ENTRY_CBUFFER_ADVANCE_WRITE_PTR_EX;
.const $_cbuffer_copy_audio_shift_to_packed    PATCH_ENTRY_CBUFFER_COPY_AUDIO_SHIFT_TO_PACKED;
.const $_cbuffer_copy_packed_to_audio_shift    PATCH_ENTRY_CBUFFER_COPY_PACKED_TO_AUDIO_SHIFT;

#else /* DISABLE_PATCH_BUILD_ID_CHECK */

.MODULE $M.download_support_lib.cbuffer_copy_ex;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_copy_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_COPY_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_get_read_offset_ex;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_get_read_offset_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_GET_READ_OFFSET_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_get_usable_octets;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_get_usable_octets:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_GET_USABLE_OCTETS;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_set_usable_octets;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_set_usable_octets:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_SET_USABLE_OCTETS;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer.get_write_address_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.get_write_address_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_GET_WRITE_ADDRESS_EX_ASM;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_get_write_address_ex;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_get_write_address_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_GET_WRITE_ADDRESS_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer.get_read_address_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.get_read_address_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_GET_READ_ADDRESS_EX_ASM;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_get_read_address_ex;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_get_read_address_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_GET_READ_ADDRESS_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_calc_amount_data_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.calc_amount_data_ex:
$_cbuffer_calc_amount_data_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_CALC_AMOUNT_DATA_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_calc_amount_space_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.calc_amount_space_ex:
$_cbuffer_calc_amount_space_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_CALC_AMOUNT_SPACE_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_set_read_address_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.set_read_address_ex:
$_cbuffer_set_read_address_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_SET_READ_ADDRESS_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_set_write_address_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.set_write_address_ex:
$_cbuffer_set_write_address_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_SET_WRITE_ADDRESS_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_advance_read_ptr_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.advance_read_ptr_ex:
$_cbuffer_advance_read_ptr_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_ADVANCE_READ_PTR_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_advance_write_ptr_ex;
.CODESEGMENT PM;
.MINIM;

$cbuffer.advance_write_ptr_ex:
$_cbuffer_advance_write_ptr_ex:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_ADVANCE_WRITE_PTR_EX;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_copy_audio_shift_to_packed;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_copy_audio_shift_to_packed:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_COPY_AUDIO_SHIFT_TO_PACKED;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.cbuffer_copy_packed_to_audio_shift;
.CODESEGMENT PM;
.MINIM;

$_cbuffer_copy_packed_to_audio_shift:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CBUFFER_COPY_PACKED_TO_AUDIO_SHIFT;

L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;


#endif /* DISABLE_PATCH_BUILD_ID_CHECK */

#endif /* defined(PATCH_ENTRY_CBUFFER_COPY_EX) */