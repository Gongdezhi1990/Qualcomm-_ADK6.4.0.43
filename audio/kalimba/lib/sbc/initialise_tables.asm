// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef INITIALISE_TABLES_ASM_INCLUDED
#define INITIALISE_TABLES_ASM_INCLUDED

#include "sbc.h"

// TODO: It is suggested to eventually remove the non-Kymera code below
// (for now left intact as lib is used still in various ways). The old "flash" copying
// must be abstracted and made platform-independent, all the copying basically takes
// some external memory data and copies it to RAM. Original copy code happens to work
// from flash on Gordon only, but e.g. Amber copies from ROM.
// The modified (Kymera) code below
// uses the platform component's unpack and copy functions from memory.asm.
//
// *****************************************************************************
// MODULE:
//    $sbc.init_tables
//
// DESCRIPTION:
//    This function will populate all the tables in RAM with data in tables in
//    Flash/ROM. Initialises encoder, decoder and common tables, hence it needs
//    a joint data object pointer. For separate encoder and decoder table init,
//    use sbcenc.init_tables and sbcdec.init_tables functions, respectively.
//
// INPUTS:
//    - R9 pointer to data object with valid data object with buffers
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.sbc.init_tables;
   .CODESEGMENT SBC_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $sbc.init_tables:

   // push rLink onto stack
   push rLink;


   call $sbcdec.private.init_tables;
   call $sbcenc.private.init_tables;
   call $sbc.private.init_tables_common;

   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_sbcdec_init_tables
//
// DESCRIPTION:
//    C callable version of the $sbcdec.init_tables function.
//    This function will populate all the tables in RAM with data in tables in
//    Flash/ROM needed by the SBC decoder.
//
// INPUTS:
//    - R0 pointer to decoder structure, which has valid data object with buffers
//      allocated
//
// OUTPUTS:
//    - none
//
// NOTES:
//    -
//
// *****************************************************************************
.set $_sbcdec_init_tables,  $sbcdec._init_tables

.MODULE $M.sbcdec._init_tables;
   .CODESEGMENT SBC_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $sbcdec._init_tables:


   PUSH_ALL_C

   r9 = r0;

   call $sbcdec.init_tables;

   POP_ALL_C
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $sbcdec.init_tables
//
// DESCRIPTION:
//    This function will populate all the tables in RAM with data in tables in
//    Flash/ROM needed by the SBC decoder.
//
// INPUTS:
//    - R9 (R5 for legacy code) pointer to decoder structure, which has valid data
//      object with buffers allocated.
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.sbcdec.init_tables;
   .CODESEGMENT SBCDEC_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $sbcdec.init_tables:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcdec.INIT_TABLES_ASM.INIT_TABLES.PATCH_ID_0, r2)
#endif
#ifndef KYMERA
   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
#endif
   call $sbcdec.private.init_tables;
   call $sbc.private.init_tables_common;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $sbcenc.init_tables
//
// DESCRIPTION:
//    This function will populate the tables in RAM with data in tables in
//    Flash/ROM needed by the SBC encoder.
//
// INPUTS:
//    - R5 pointer to encoder structure, which has valid data object with buffers
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.sbcenc.init_tables;
   .CODESEGMENT SBCENC_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $sbcenc.init_tables:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcenc.INIT_TABLES_ASM.INIT_TABLES.PATCH_ID_0, r2)
#endif

#ifndef KYMERA
   r9 = M[r5 + $codec.ENCODER_DATA_OBJECT_FIELD];
#endif
   call $sbcenc.private.init_tables;
   call $sbc.private.init_tables_common;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// *******************  PRIVATE FUNCTIONS FROM THIS POINT ON *******************
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $sbcdec.private.init_tables
//
// DESCRIPTION:
//    This function will populate decoder tables in RAM with data in tables in
//    Flash/ROM
//
// INPUTS:
//    - R9 pointer to data object with buffers allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.sbcdec.private.init_tables;
   .CODESEGMENT SBCDEC_PRIVATE_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $sbcdec.private.init_tables:

   // push rLink onto stack
   push rLink;

   // synthesis_coefs_m8
   r2 = M[r9 + $sbc.mem.SYNTHESIS_COEFS_M8_FIELD];
   r0 = &$sbcdec.synthesis_coefs_m8;
   r1 = 128;

   call $mem.ext_window_unpack_to_ram;

#ifndef SBC_WBS_ONLY
   // synthesis_coefs_m4
   // TODO can't we just use one of the tables depending on the setting?
   r2 = M[r9 + $sbc.mem.SYNTHESIS_COEFS_M4_FIELD];
   r0 = &$sbcdec.synthesis_coefs_m4;
   r1 = 32;

   Null = r2;
   if NZ call $mem.ext_window_unpack_to_ram;
#endif

   // levelrecip_coefs
   r2 = M[r9 + $sbc.mem.LEVELRECIP_COEFS_FIELD];
   r0 = &$sbcdec.levelrecip_coefs;
   r1 = 15;

   call $mem.ext_window_unpack_to_ram;

   // bitmask_lookup
   r2 = M[r9 + $sbc.mem.BITMASK_LOOKUP_FIELD];
   r0 = &$sbcdec.bitmask_lookup;
   r1 = 17;

   call $mem.ext_window_unpack_to_ram;

   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $sbcenc.private.init_tables
//
// DESCRIPTION:
//    This function will populate encoder tables in RAM with data in tables in
//    Flash/ROM
//
// INPUTS:
//    - R9 pointer to data object with buffers allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.sbcenc.private.init_tables;
   .CODESEGMENT SBCENC_PRIVATE_INIT_TABLES_PM;
   .DATASEGMENT DM;

   $sbcenc.private.init_tables:

   // push rLink onto stack
   push rLink;

   // analysis_coefs_m8
   r2 = M[r9 + $sbc.mem.ANALYSIS_COEFS_M8_FIELD];
   r0 = &$sbcenc.analysis_coefs_m8;
   r1 = 128;

   call $mem.ext_window_unpack_to_ram;

#ifndef SBC_WBS_ONLY
   // analysis_coefs_m4
   // TODO can't we just use one of the tables depending on the setting?
   r2 = M[r9 + $sbc.mem.ANALYSIS_COEFS_M4_FIELD];
   r0 = &$sbcenc.analysis_coefs_m4;
   r1 = 32;
   Null = r2;
   if NZ call $mem.ext_window_unpack_to_ram;
#endif

   // level_coefs
   r2 = M[r9 + $sbc.mem.LEVEL_COEFS_FIELD];
   r0 = &$sbcenc.level_coefs;
   r1 = 16;

   call $mem.ext_window_unpack_to_ram;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $sbc.private.init_tables_common
//
// DESCRIPTION:
//    This function will populate common tables in RAM with data in tables in
//    Flash/ROM
//
// INPUTS:
//    - r9 = pointer to data object, which has valid pointers
//           to allocated buffers.
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// NOTES:
//    -
//
// *****************************************************************************
.MODULE $M.sbc.private.init_tables_common;
   .CODESEGMENT SBC_PRIVATE_INIT_TABLES_COMMON_PM;
   .DATASEGMENT DM;

   $sbc.private.init_tables_common:

   // push rLink onto stack
   push rLink;

   // -- common tables
   // this is common table init,
   // and may have only encoder or only decoder side used, use R9 from previous encoder or decoder-specific
   // table initialisation.

   // win_coefs_m8
   r2 = M[r9 + $sbc.mem.WIN_COEFS_M8_FIELD];
   r0 = &$sbc.win_coefs_m8;
   r1 = 80;

   call $mem.ext_window_unpack_to_ram;

#ifndef SBC_WBS_ONLY
   // win_coefs_m4
   r2 = M[r9 + $sbc.mem.WIN_COEFS_M4_FIELD];
   r0 = &$sbc.win_coefs_m4;
   r1 = 40;
   Null = r2;
   if NZ call $mem.ext_window_unpack_to_ram;
#endif

   // loudness_offset
   r2 = M[r9 + $sbc.mem.LOUDNESS_OFFSET_FIELD];
   r0 = &$sbc.loudness_offset;
#ifndef SBC_WBS_ONLY
   r1 = 48;
#else
   r1 = 32;
#endif
   call $mem.ext_window_copy_to_ram;

   // bitslice_lkup
   r2 = M[r9 + $sbc.mem.BITSLICE_LOOKUP_FIELD];
   r0 = &$sbc.bitslice_lkup_block;
   r1 = 66;

   call $mem.ext_window_unpack_to_ram;

   jump $pop_rLink_and_rts;

.ENDMODULE;
#endif // INITIALISE_TABLES_ASM_INCLUDED
