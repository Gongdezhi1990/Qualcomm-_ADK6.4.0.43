/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_packet_handler.c

DESCRIPTION
    Contains functions related to sending / recieving SWAT packets
*/

#ifndef _SWAT_PACKET_HANDLER_H_
#define _SWAT_PACKET_HANDLER_H_

#include "swat_private.h"

/* Firmware includes */
#include <source.h>


/* TODO: PACKET PAYLOAD LENGTH AND SIGNAL TYPE SHOULD BOTH BE IN THE SAME BYTE ACCORDING TO SPEC! */
#define PKT_SIGNAL_ID     0x0  /* Offset to the signal ID in the SWAT packet */
#define PKT_PAYLOAD_LEN   0x1  /* Offset to the payload length in the SWAT packet */
#define PKT_SIGNAL_TYPE   0x2  /* Offset to the signal type in the SWAT packet */
#define PKT_PAYLOAD       0x3  /* Offset to the first byte in the payload */


/*****************************************************************************
FUNCTION:
    swatSendData

PARAMETERS:
    device - the remote device to send the data to
    data - pointer to the data packet to send
    len - the length of the data packet to send

DESCRIPTION:
    Function to send data to the remote device
*/
bool swatSendData(remoteDevice *device, uint8 *data, uint16 len);


/*****************************************************************************
FUNCTION:
    swatHandleSwatSignallingData

DESCRIPTION:
    Function to handle incoming data on the signalling channel
*/
void swatHandleSwatSignallingData(Source source);


#endif /* _SWAT_PACKET_HANDLER_H_ */
