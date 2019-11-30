/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/* Link script for P1 */
OUTPUT_ARCH(QCC512x_apps)

$DATA_RAM_SIZE_BYTES    = 32 * 1024;
$STACK_SIZE_BYTES       = 2 * 1024; /* If you change this, change 
                                     * block_the_stack_away() in excep_test.c 
                                     * accordingly (needs to work for p0 and p1)
                                     */

$SHARED_RAM_SIZE_BYTES  = 32 * 1024;
INCLUDE link_script_common.ld
