// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $_download_self_test_branch_test
//
// DESCRIPTION:
//    Tests patching in MiniM of branch instructions call, jumpB and do_loop. They
//    need patching because they are always relative (in MiniM)
//
// INPUTS:
//    None
//
// OUTPUTS:
//    - r0 = test result code (0x600D for success or 0xBAD for failure)
//
// TRASHED REGISTERS:
//    None
//
// *****************************************************************************
.MODULE $M.download_self_test_branch_test;
.CODESEGMENT PM;
.DATASEGMENT DM;
.MINIM;

.CONST $MEM_TO_ALLOC 200;

$_download_self_test_branch_test:
    push rLink;
    /* Tests call_k9 instruction patching in MiniM */
    r0 = $MEM_TO_ALLOC;
    /* Don't care which bank */
    r1 = 3;
#ifdef PMALLOC_DEBUG
	.VAR8 $fileName[] = string(__FILE__);
	r2 = &$fileName;
	r3 = __LINE__;
    call $_xzppmalloc_debug;
#else
	call $_xzppmalloc;
#endif
    Null = r0 - Null;
    if Z jump error;
    call $_pfree;
    r0 = $MEM_TO_ALLOC;
    r1 = 3;
    /* Tests jumpB instruction patching in MiniM */
    rLink = return_jumpB;
#ifdef PMALLOC_DEBUG
	r2 = &$fileName;
	r3 = __LINE__;
    jump $_xzppmalloc_debug;
#else
	jump $_xzppmalloc;
#endif
    return_jumpB:
    Null = r0 - Null;
    if Z jump error;
    call $_pfree;
    r0 = 0x600D;
    /* Return 0x600D */
    pop rLink;
    rts;
    error:
    /* Return ERROR */
    r0 = 0xBAD;
    pop rLink;
    rts;
.ENDMODULE;


/* Variables used for the tests below */
.MODULE $M.download_self_test_variables;
.DATASEGMENT DM;

.BLOCK/DM1 $dm1_vars;
    .VAR $dm1_v1[] = 0, 0, 0, 0, 0, 0, 0, 0, 0x600D;
    .VAR $dm1_v2[] = 0, 0, 0, 0, 0, 0, 0, $dm1_v1 + 8*ADDR_PER_WORD;
    .VAR $dm1_v3[] = 0, 0, 0, 0, 0, 0, $dm1_v2 + 7*ADDR_PER_WORD;
    .VAR $dm1_v4[] = 0, 0, 0, 0, 0, $dm1_v3 + 6*ADDR_PER_WORD;
    .VAR $dm1_v5[] = 0, 0, 0, 0, $dm1_v4 + 5*ADDR_PER_WORD;
.ENDBLOCK;

.BLOCK/DM2 $dm2_vars;
    .VAR $dm2_v1[] = 0, 0, 0, $dm1_v5 + 4*ADDR_PER_WORD;
    .VAR $dm2_v2[] = 0, 0, $dm2_v1 + 3*ADDR_PER_WORD;
    .VAR $dm2_vx   = 0;
    .VAR $dm2_v3[] = 0, $dm2_v2 + 2*ADDR_PER_WORD;
    .VAR $dm2_vxx  = 0;
    .VAR $dm2_v4[] = 0, $dm2_v3 + 1*ADDR_PER_WORD;
.ENDBLOCK;

.VAR/DM2 $dm2_v5   = $dm2_v4 + 1*ADDR_PER_WORD;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_download_self_test_var_ref_test_minim
//
// DESCRIPTION:
//    Tests blocks of variables and variables are referenced and relocated correctly.
//    Starts with dm2_v5, after 10 memory accesses we should get the value 0x600D
//
// INPUTS:
//    None
//
// OUTPUTS:
//    - r0 = test result code (0x600D for success or anything else for failure)
//
// TRASHED REGISTERS:
//    None
//
// *****************************************************************************
.MODULE $M.download_self_test_var_ref_test_minim;
.CODESEGMENT PM;
.MINIM;

$_download_self_test_var_ref_test_minim:
    push r10;
    r10 = 10;
    r0 = &$dm2_v5;
    do get_value_loop;
        r0 = M[r0];
    get_value_loop:
    pop r10;
    /* Return GOOD */
    Null = r0 - 0x600D;
    if Z rts;
    /* Return error */
    r0 = 0xBAD;
    rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_download_self_test_var_ref_test_maxim
//
// DESCRIPTION:
//    Tests blocks of variables and variables are referenced and relocated correctly.
//    Starts with dm2_v5, after 10 memory accesses we should get the value 0x600D
//
// INPUTS:
//    None
//
// OUTPUTS:
//    - r0 = test result code (0x600D for success or anything else for failure)
//
// TRASHED REGISTERS:
//    None
//
// *****************************************************************************
.MODULE $M.download_self_test_var_ref_test_maxim;
.CODESEGMENT PM;
.MAXIM;

$_download_self_test_var_ref_test_maxim:
    push r10;
    r10 = 10;
    r0 = &$dm2_v5;
    do get_value_loop;
        r0 = M[r0];
    get_value_loop:
    pop r10;
    /* Return GOOD */
    Null = r0 - 0x600D;
    if Z rts;
    /* Return error */
    r0 = 0xBAD;
    rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_func_increase_one_with_param_asm
//
// DESCRIPTION:
//    Increase input variable by 1 and return
//
// INPUTS:
//    r0 = Input value
//
// OUTPUTS:
//    - r0 = Input value + 1
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.func_increase_one_with_param_asm;
.CODESEGMENT PM;
.MINIM;

$_func_increase_one_with_param_asm:
    r0 = r0 + 1;
    rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_download_self_test_call_from_maxim_to_minim_test_maxim
//
// DESCRIPTION:
//    Tests calling into Minim code section from a Maxim one
//
// INPUTS:
//    None
//
// OUTPUTS:
//    - r0 = test result code (0x600D for success or anything else for failure)
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.download_self_test_call_from_maxim_to_minim_test_maxim;
.CODESEGMENT PM;
.MAXIM;

$_download_self_test_call_from_maxim_to_minim_test_maxim:
    push rLink;
    r0 = 1;
    call $_func_increase_one_with_param_asm;
    Null = r0 - 2;
    if NZ jump ret_BAD;
    r1 = $_func_increase_one_with_param_asm;
    call r1;
    Null = r0 - 3;
    if NZ jump ret_BAD;
    r1 = $_func_increase_one_with_param_c;
    call r1;
    Null = r0 - 4;
    if NZ jump ret_BAD;
    call $_func_increase_one_with_param_c;
    Null = r0 - 5;
    if NZ jump ret_BAD;
    ret_GOOD:
        r0 = 0x600D;
        jump return;
    ret_BAD:
        r0 = 0xBAD;
    return:
        pop rLink;
        rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_download_self_test_variable_size_vars_test_maxim_asm
//
// DESCRIPTION:
//    Tests references to multiple variable sizes
//
// INPUTS:
//    None
//
// OUTPUTS:
//    - r0 = test result code (0x600D for success or anything else for failure)
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.download_self_test_variable_size_vars_test_maxim_asm;
.CODESEGMENT PM;
.DATASEGMENT DM;
.MAXIM;

$_download_self_test_variable_size_vars_test_maxim_asm:
    .VAR8 firstByte = 0xC;
    .VAR firstWord = 0x87654321;
    .VAR16 firstHalf = 0xABCD;
    .VAR8 secondByte = 0x2A;
    .VAR8 thirdByte = 0x2B;
    .VAR8 fourthByte = 0x2C;
    .VAR8 fifthByte = 0x2D;
    .VAR8 ByteArray[3] = 0x2E, 0x2F, 0x30;
    .VAR secondWord = 0x63;
    .VAR8 sixthByte = 0x31;
    .VAR secondHalf[3] = 0xBEEF,0xDEAD, 0xABCD;
    .VAR16 secondHalf1 = 0xBEEF;
    push rLink;
    r0 = MBU[firstByte];
    Null = r0 - 0xC;
    if NZ jump ret_BAD;
    r0 = M[firstWord];
    Null = r0 - 0x87654321;
    if NZ jump ret_BAD;
    r0 = MHU[firstHalf];
    Null = r0 - 0xABCD;
    if NZ jump ret_BAD;
    r0 = MBU[secondByte];
    Null = r0 - 0x2A;
    if NZ jump ret_BAD;
    r0 = MBU[thirdByte];
    Null = r0 - 0x2B;
    if NZ jump ret_BAD;
    r0 = MBU[fourthByte];
    Null = r0 - 0x2C;
    if NZ jump ret_BAD;
    r0 = MBU[fifthByte];
    Null = r0 - 0x2D;
    if NZ jump ret_BAD;
    r0 = MBU[ByteArray+1];
    Null = r0 - 0x2F;
    if NZ jump ret_BAD;
    r0 = M[secondWord];
    Null = r0 - 0x63;
    if NZ jump ret_BAD;
    r0 = MBU[sixthByte];
    Null = r0 - 0x31;
    if NZ jump ret_BAD;
    r0 = M[secondHalf + 2*ADDR_PER_WORD];
    Null = r0 - 0xABCD;
    if NZ jump ret_BAD;
    r0 = MHU[secondHalf1];
    Null = r0 - 0xBEEF;
    if NZ jump ret_BAD;
    ret_GOOD:
        r0 = 0x600D;
        jump return;
    ret_BAD:
        r0 = 0xBAD;
    return:
        pop rLink;
        rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_download_self_test_variable_size_vars_test_minim_asm
//
// DESCRIPTION:
//    Tests references to multiple variable sizes
//
// INPUTS:
//    None
//
// OUTPUTS:
//    - r0 = test result code (0x600D for success or anything else for failure)
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.download_self_test_variable_size_vars_test_minim_asm;
.CODESEGMENT PM;
.DATASEGMENT DM;
.MINIM;

$_download_self_test_variable_size_vars_test_minim_asm:
    .VAR8 firstByte = 0xC;
    .VAR firstWord = 0x87654321;
    .VAR16 firstHalf = 0xABCD;
    .VAR8 secondByte = 0x2A;
    .VAR8 thirdByte = 0x2B;
    .VAR8 fourthByte = 0x2C;
    .VAR8 fifthByte = 0x2D;
    .VAR8 ByteArray[3] = 0x2E, 0x2F, 0x30;
    .VAR secondWord = 0x63;
    .VAR8 sixthByte = 0x31;
    .VAR secondHalf[3] = 0xBEEF,0xDEAD, 0xABCD;
    .VAR16 secondHalf1 = 0xBEEF;
    push rLink;
    r0 = MBU[firstByte];
    Null = r0 - 0xC;
    if NZ jump ret_BAD;
    r0 = M[firstWord];
    Null = r0 - 0x87654321;
    if NZ jump ret_BAD;
    r0 = MHU[firstHalf];
    Null = r0 - 0xABCD;
    if NZ jump ret_BAD;
    r0 = MBU[secondByte];
    Null = r0 - 0x2A;
    if NZ jump ret_BAD;
    r0 = MBU[thirdByte];
    Null = r0 - 0x2B;
    if NZ jump ret_BAD;
    r0 = MBU[fourthByte];
    Null = r0 - 0x2C;
    if NZ jump ret_BAD;
    r0 = MBU[fifthByte];
    Null = r0 - 0x2D;
    if NZ jump ret_BAD;
    r0 = MBU[ByteArray+1];
    Null = r0 - 0x2F;
    if NZ jump ret_BAD;
    r0 = M[secondWord];
    Null = r0 - 0x63;
    if NZ jump ret_BAD;
    r0 = MBU[sixthByte];
    Null = r0 - 0x31;
    if NZ jump ret_BAD;
    r0 = M[secondHalf + 2*ADDR_PER_WORD];
    Null = r0 - 0xABCD;
    if NZ jump ret_BAD;
    r0 = MHU[secondHalf1];
    Null = r0 - 0xBEEF;
    if NZ jump ret_BAD;
    ret_GOOD:
        r0 = 0x600D;
        jump return;
    ret_BAD:
        r0 = 0xBAD;
    return:
        pop rLink;
        rts;
.ENDMODULE;
