// Copyright (c) 2016 Qualcomm Technologies International, Ltd.
//   %%version
#include "kaldwarfregnums.h"
#include "dwarf_constants.h"

.MODULE $M.crt0_rst;
    .CODESEGMENT PM_RST;

    /* This code sits at the base of memory and is just a jump to the
     * crt0 initialisation code. This jump switches the processor into minim
     * mode. The jump needs to be in its own source file so it can be built
     * as maxim code (the assembler is switched to maxim or minim mode on a
     * per-module or per-file basis and the linker cannot put maxim and minim
     * code in the same section). The processor runs in maxim mode at reset
     * by default.
     */
    .MAXIM;

$reset:
    nop;
    jump $reset_minim;

    /* We implement memcpy here to override the library version so it can use
     * maxim instructions (which should be faster than minim) and so that we
     * can save and restore the index registers to make it interrupt safe.
     */
$_memcpy:
    pushm <I3, I7>;
.memcpy_push:
    I3 = r1;
    I7 = r0;
    r3 = r1 OR r0;
    Null = r3 AND 0x3;
    if NE jump L_2;
    r10 = r2 LSHIFT -2;
    if EQ jump L_2;
    r10 = r10 - 1;
    rMAC = M[I3,M1];
    r2 = r2 AND 0x3;
    do L_0;
    rMAC = M[I3,M1], M[I7,M1] = rMAC;
L_0:
    M[I7,M1] = rMAC;

L_2:
    r10 = r2 + Null;
    r3 = Null + Null;
    do L_1;
    rMAC = MBS[I3 + r3];
    MB[I7 + r3] = rMAC;
    r3 = r3 + 1;

L_1:
    popm <I3, I7>;
.memcpy_pop:
    rts;
.memcpy_end:

.ENDMODULE;

/* Debug frame information for memcpy.  */
    .section ".debug_frame"
.MEMCPY_CIE_Pointer:
    .4byte  .MEMCPY_CIE_End-.MEMCPY_CIE_Start   /* CIE length */
.MEMCPY_CIE_Start:
    .4byte  0xFFFFFFFF  /* CIE_id */
    .byte   0x01        /* CIE version */
    .string ""          /* CIE augmentation */
    .byte   0x01        /* code alignment factor */
    .byte   0x04        /* data alignment factor */
    .byte   KalDwarfRegisterNum_RegrLINK    /* return address register */

    .byte   DW_CFA_def_cfa
    .byte   KalDwarfRegisterNum_RegFP
    .byte   0x00
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegSP
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegrMAC
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR0
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR1
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR2
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR3
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR4
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR5
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR6
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR7
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR8
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR9
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegR10
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegrLINK
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegFP

    .byte   DW_CFA_nop
    .byte   DW_CFA_nop
.MEMCPY_CIE_End:

    .4byte  .MEMCPY_FDE_End-.MEMCPY_FDE_Start       /* FDE length */
.MEMCPY_FDE_Start:
    .4byte  .MEMCPY_CIE_Pointer                   /* CIE_pointer */
    .4byte  $_memcpy                  /* initial_location */
    .4byte  .memcpy_end-$_memcpy /* address_range */

    .byte   DW_CFA_advance_loc2
    .2byte  .memcpy_push-$_memcpy
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegI3
    .byte   0x00
    .byte   DW_CFA_offset + KalDwarfRegisterNum_RegI7
    .byte   0x01

    .byte   DW_CFA_advance_loc2
    .2byte  .memcpy_pop-.memcpy_push
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegI3
    .byte   DW_CFA_restore + KalDwarfRegisterNum_RegI7

.MEMCPY_FDE_End:
