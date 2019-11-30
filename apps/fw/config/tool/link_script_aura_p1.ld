/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*   %%version */

/* Link script for P1 */

ENTRY($reset)

$DATA_RAM_SIZE_BYTES    = 32 * 1024;
$STACK_SIZE_BYTES       = 2 * 1024; /* If you change this, change 
                                     * block_the_stack_away() in excep_test.c 
                                     * accordingly (needs to work for p0 and p1)
                                     */

/*See CS-333748-DD for memory layout */
$DATA_ROM_WINDOW        = 0x78000000;

$DATA_RAM_START_ADDRESS = 0x00010000;

INCLUDE link_script_aura_common.ld   /* common to P0 and P1 for Aura */
INCLUDE link_script_common.ld        /* common across chips */

