// Copyright (c) 2016 Qualcomm Technologies International, Ltd.
//   %%version
#include "kaldwarfregnums.h"
#include "dwarf_constants.h"

.MODULE $M.appcmd_call_function;
    .CODESEGMENT PM;
    .MINIM;

$_appcmd_call_function:
.appcmd_call_function_start:
    pushm <FP(=SP), r4, r5, rLink>;
.appcmd_call_function_after_prologue:
    // Number of parameters goes into r4
    rMAC = $_appcmd_parameters + 4;
    r10 = M[rMAC];
    r4 = 0; // Initialise r4 with default number of stack param bytes

    Null = r10 - Null;
    if EQ jump appcmd_call_function_make_call;

    // Load first parameter
    rMAC = rMAC + 4;
    r0 = M[rMAC];
    r10 = r10 - 1;
    Null = r10 - Null;
    if EQ jump appcmd_call_function_make_call;

    // Load second parameter
    rMAC = rMAC + 4;
    r1 = M[rMAC];
    r10 = r10 - 1;
    Null = r10 - Null;
    if EQ jump appcmd_call_function_make_call;

    // Load third parameter
    rMAC = rMAC + 4;
    r2 = M[rMAC];
    r10 = r10 - 1;
    Null = r10 - Null;
    if EQ jump appcmd_call_function_make_call;

    // Load fourth parameter
    rMAC = rMAC + 4;
    r3 = M[rMAC];
    r10 = r10 - 1;
    Null = r10 - Null;
    if EQ jump appcmd_call_function_make_call;

    // Remaining parameters go on the stack, right to left
    r4 = r10 LSHIFT 2; // = number of stack param bytes
    rMAC = rMAC + r4; // = address of last param
    do push_stack_loop;
        r5 = M[rMAC];
        push r5;
        rMAC = rMAC - 4;
    push_stack_loop:

    appcmd_call_function_make_call:
    rMAC = M[$_appcmd_parameters];
    call rMAC;

    // We have to loop here because the obvious "SP = SP - r4" doesn't work.
    r10 = r4 LSHIFT -2; // Re-load r10 with the number of stack params
    do pop_stack_loop;
        SP = SP - 4;
    pop_stack_loop:

    rMAC = $_appcmd_results;
    M[rMAC] = r0;

    popm <FP, r4, r5, rLink>;
    rts;
.appcmd_call_function_end:

.ENDMODULE;


.section ".debug_frame"
.CALL_FN_CIE_Pointer:
    .4byte  .CALL_FN_CIE_End-.CALL_FN_CIE_Start   /* CIE length */
.CALL_FN_CIE_Start:
    .4byte  0xFFFFFFFF  /* CIE_id */
    .byte   0x01        /* CIE version */
    .string ""          /* CIE augmentation */
    .byte   0x01        /* code alignment factor */
    .byte   0x04        /* data alignment factor */
    .byte   KalDwarfRegisterNum_RegrLINK    /* return address register */

    // rMAC, r0, r1, r2, r3 and r10 are scratch registers.  In practice they
    // aren't trashed straight away, but the caller assumes it loses them, so
    // there's no value in providing an unwind rule for them even for addresses
    // for which they are still "same_value"
    .byte   DW_CFA_def_cfa
    .byte   KalDwarfRegisterNum_RegFP
    .byte   0x00
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegSP
    .byte   DW_CFA_same_value
    .byte   KalDwarfRegisterNum_RegrMAC
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR0
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR1
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR2
    .byte   DW_CFA_undefined
    .byte   KalDwarfRegisterNum_RegR3
    .byte   DW_CFA_undefined
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
.CALL_FN_CIE_End:

    .4byte  .CALL_FN_FDE_End - .CALL_FN_FDE_Start   // FDE length
.CALL_FN_FDE_Start:
    .4byte  .CALL_FN_CIE_Pointer                    // Pointer to CIE
    .4byte  .appcmd_call_function_start                  // First instruction
    .4byte  .appcmd_call_function_end - .appcmd_call_function_start // Number of instructions
    // Instructions

     // The prologue pushes FP, R4, R5 and rLINK onto the stack
    .byte DW_CFA_advance_loc2
    .2byte .appcmd_call_function_after_prologue - .appcmd_call_function_start
    .byte DW_CFA_offset + KalDwarfRegisterNum_RegFP
    .byte 0
    .byte DW_CFA_offset + KalDwarfRegisterNum_RegR4
    .byte 1
    .byte DW_CFA_offset + KalDwarfRegisterNum_RegR5
    .byte 2
    .byte DW_CFA_offset + KalDwarfRegisterNum_RegrLINK
    .byte 3

    // The epilogue pops them so they're back to the same value as in the caller
    .byte DW_CFA_advance_loc2
    .2byte .appcmd_call_function_end - .appcmd_call_function_after_prologue
    .byte DW_CFA_same_value
    .byte KalDwarfRegisterNum_RegFP
    .byte DW_CFA_same_value
    .byte KalDwarfRegisterNum_RegR4
    .byte DW_CFA_same_value
    .byte KalDwarfRegisterNum_RegR5
    .byte DW_CFA_same_value
    .byte KalDwarfRegisterNum_RegrLINK

    .byte DW_CFA_nop
    .byte DW_CFA_nop
.CALL_FN_FDE_End:
