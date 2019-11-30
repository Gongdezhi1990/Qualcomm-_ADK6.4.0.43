#include "portability_macros.h"
#include "stack.h"

#include "@@@plib_name@@@.h"

// *****************************************************************************
// MODULE:
//    $M.@@@plib_name@@@_entry
//
// DESCRIPTION:
//    Example of an entry point function. This could be part of your private library API.
//    Public entry symbols are defined in @@@plib_name@@@.symbols
//
// INPUTS:
//   - Your input registers
//
// OUTPUTS:
//   - Your output registers
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.@@@plib_name@@@_entry;
   .CODESEGMENT PM;
   .MAXIM;

$_@@@plib_name@@@_entry:
    PUSH_ALL_C
    /*
     * TODO Assembly processing code goes here ...
     */
    /* call into your private library code */
    r1 = $MODE;
    call $@@@plib_name@@@_secret;

    POP_ALL_C
    rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $M.@@@plib_name@@@_secret
//
// DESCRIPTION:
//    Example of a module that gets scrambled. (This is the default behaviour when
//    the name is not included in @@@plib_name@@@.symbols)
//
// INPUTS:
//   - Your input registers
//
// OUTPUTS:
//   - Your output registers
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.@@@plib_name@@@_secret;
   .CODESEGMENT PM;
   .MAXIM;
   .CONST $SECRET_KEY 0xAD54C770;
   .CONST $SECRET_VALUE 0x1A58986F;

$@@@plib_name@@@_secret:

    /*
     * TODO Assembly processing code goes here ...
     */
    Null = r1 - $MODE;
    if NZ jump ret_false;
    r0 = r0 XOR $SECRET_KEY;
    Null = r0 - $SECRET_VALUE;
    if NZ jump ret_false;
    ret_true:
        r0 = 1;
        jump just_ret;
    ret_false:
        r0 = 0;
    just_ret:
        rts;

.ENDMODULE;