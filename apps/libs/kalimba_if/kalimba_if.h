/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    kalimba_if.h

DESCRIPTION
    Common kalimba interface
*/

/*!
@file   kalimba_if.h
@brief  Common kalimba interface definitions 
*/

#ifndef KALIMBA_IF_H
#define KALIMBA_IF_H

/* DSP Output ports */
#define DSP_OUTPUT_PORT_NONE            0xFFFF
#define DSP_OUTPUT_PORT_SUB_ESCO        0
#define DSP_OUTPUT_PORT_SUB_L2CAP       1
#define DSP_OUTPUT_PORT_RELAY_L2CAP     2
#define DSP_OUTPUT_PORT_USB             3
#define DSP_OUTPUT_PORT_PRI_LEFT        4
#define DSP_OUTPUT_PORT_PRI_RIGHT       5
#define DSP_OUTPUT_PORT_SEC_LEFT        6
#define DSP_OUTPUT_PORT_SEC_RIGHT       7
#define DSP_OUTPUT_PORT_SUB_WIRED       8
#define DSP_OUTPUT_PORT_AUX_LEFT        9
#define DSP_OUTPUT_PORT_AUX_RIGHT       10

#define KALIMBA_MSG_FILE_INDEX          (0x1067)
#define KALIMBA_MSG_FILE_NAME           (0x1068)

#define KALIMBA_MSG_RELAY_CONTROL       (0x715C)

#endif /*KALIMBA_IF_H*/
