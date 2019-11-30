/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*   %%version */

/* Defines common to P0 and P1 */

OUTPUT_ARCH(QCC512x_apps)

$SHARED_RAM_SIZE_BYTES  = 32 * 1024;
$SHARED_RAM_BEGIN		= 0x10000000;

$DATA_RAM_END_ADDRESS   = $DATA_RAM_START_ADDRESS + $DATA_RAM_SIZE_BYTES;

