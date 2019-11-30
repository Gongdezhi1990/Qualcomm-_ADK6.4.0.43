#include "@@@cap_name@@@_struct_asm_defs.h"

// *****************************************************************************
// MODULE:
//    $M.@@@cap_name@@@_proc_func
//
// DESCRIPTION:
//    Define here your processing function
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
.MODULE $M.@@@cap_name@@@_proc_func;
   .CODESEGMENT PM;
   .MAXIM;

$_@@@cap_name@@@_proc_func:

    /*
     * TODO Assembly processing code goes here ...
     */

    r0 = M[r0 + $@@@cap_name@@@_struct.@@@cap_name^U@@@_OP_DATA_struct.MY_DATA_FIELD];
    rts;

.ENDMODULE;
