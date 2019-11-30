/**************************************************************************/
/* Copyright (c) 2018-2018 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declared in ext_buffer.h, are resolved
 * to absolute addresses exported from the patch build.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"

#ifdef DISABLE_PATCH_BUILD_ID_CHECK

.const $_ext_buffer_create                PATCH_ENTRY_EXT_BUFFER_CREATE;
.const $_ext_buffer_destroy               PATCH_ENTRY_EXT_BUFFER_DESTROY;
.const $_ext_buffer_amount_data           PATCH_ENTRY_EXT_AMOUNT_DATA;
.const $_ext_buffer_amount_space          PATCH_ENTRY_EXT_AMOUNT_SPACE;
.const $_ext_buffer_circ_read             PATCH_ENTRY_EXT_CIRC_READ;
.const $_ext_buffer_circ_write            PATCH_ENTRY_EXT_CIRC_WRITE;
.const $_ext_buffer_random_access_read    PATCH_ENTRY_EXT_RANDOM_ACCESS_READ;
.const $_ext_buffer_random_access_write   PATCH_ENTRY_EXT_RANDOM_ACCESS_WRITE;
.const $_ext_buffer_get_read_offset       PATCH_ENTRY_EXT_GET_READ_OFFSET
.const $_ext_buffer_set_read_offset       PATCH_ENTRY_EXT_SET_READ_OFFSET
.const $_ext_buffer_get_write_offset      PATCH_ENTRY_EXT_GET_WRITE_OFFSET
.const $_ext_buffer_set_write_offset      PATCH_ENTRY_EXT_SET_WRITE_OFFSET
.const $_ext_buffer_discard_data          PATCH_ENTRY_EXT_DISCARD_DATA
.const $_ext_buffer_pack                  PATCH_ENTRY_EXT_BUFFER_PACK
.const $_ext_buffer_unpack                PATCH_ENTRY_EXT_BUFFER_UNPACK

#else /* DISABLE_PATCH_BUILD_ID_CHECK */

.MODULE $M.download_support_lib.ext_buffer_create;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_create:
#ifdef PATCH_ENTRY_EXT_BUFFER_CREATE
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_BUFFER_CREATE;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_destroy;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_destroy:
#ifdef PATCH_ENTRY_EXT_BUFFER_DESTROY
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_BUFFER_DESTROY;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_amount_data;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_amount_data:
#ifdef PATCH_ENTRY_EXT_AMOUNT_DATA
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_AMOUNT_DATA;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_amount_space;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_amount_space:
#ifdef PATCH_ENTRY_EXT_AMOUNT_SPACE
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_AMOUNT_SPACE;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_random_access_read;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_random_access_read:
#ifdef PATCH_ENTRY_EXT_RANDOM_ACCESS_READ
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_RANDOM_ACCESS_READ;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_random_access_write;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_random_access_write:
#ifdef PATCH_ENTRY_EXT_RANDOM_ACCESS_WRITE
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_RANDOM_ACCESS_WRITE;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_circ_read;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_circ_read:
#ifdef PATCH_ENTRY_EXT_CIRC_READ
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_CIRC_READ;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_circ_write;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_circ_write:
#ifdef PATCH_ENTRY_EXT_CIRC_WRITE
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_CIRC_WRITE;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_get_read_offset;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_get_read_offset:
#ifdef PATCH_ENTRY_EXT_GET_READ_OFFSET
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_GET_READ_OFFSET;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_set_read_offset;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_set_read_offset:
#ifdef PATCH_ENTRY_EXT_SET_READ_OFFSET
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_SET_READ_OFFSET;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_get_write_offset;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_get_write_offset:
#ifdef PATCH_ENTRY_EXT_GET_WRITE_OFFSET
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_GET_WRITE_OFFSET;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_set_write_offset;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_set_write_offset:
#ifdef PATCH_ENTRY_EXT_SET_WRITE_OFFSET
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_SET_WRITE_OFFSET;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_discard_data;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_discard_data:
#ifdef PATCH_ENTRY_EXT_DISCARD_DATA
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_DISCARD_DATA;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_pack;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_pack:
#ifdef PATCH_ENTRY_EXT_BUFFER_PACK
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_BUFFER_PACK;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

.MODULE $M.download_support_lib.ext_buffer_unpack;
.CODESEGMENT PM;
.MINIM;

$_ext_buffer_unpack:
#ifdef PATCH_ENTRY_EXT_BUFFER_UNPACK
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXT_BUFFER_UNPACK;
#endif
L_pb_mismatch:
    /* Stub: return (uintptr_t 0); */
    r0 = 0;
    rts;

.ENDMODULE;

#endif