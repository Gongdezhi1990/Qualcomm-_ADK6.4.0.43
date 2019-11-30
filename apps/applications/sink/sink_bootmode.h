/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_bootmode.h
 
DESCRIPTION
    Definitions of boot modes.
*/

#ifndef SINK_BOOTMODE_H_
#define SINK_BOOTMODE_H_

/* The default bootmode on a CSRA6810x is 0 */
#define BOOTMODE_DEFAULT        0x00
#define BOOTMODE_DFU            0x01
#define BOOTMODE_CUSTOM         0x02
#define BOOTMODE_USB_LOW_POWER  0x03
#define BOOTMODE_CVC_PRODTEST   0x04
#define BOOTMODE_ALT_FSTAB      0x05


#endif /* SINK_BOOTMODE_H_ */
