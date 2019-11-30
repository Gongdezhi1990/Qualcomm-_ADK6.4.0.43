/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server_api.h
    
DESCRIPTION
    Contains routines responsible for building & sending external messages.
*/

#ifndef GATT_GAP_SERVER_API_H_
#define GATT_GAP_SERVER_API_H_

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>
#include <panic.h>

/* Macros for creating messages */
#define MAKE_GATT_GAP_SERVER_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T)

/*******************************************************************************
NAME
    gattGapServerSendReadDeviceNameInd
    
DESCRIPTION
    Build and send a GATT_GAP_SERVER_READ_DEVICE_NAME_IND message.
    
PARAMETERS
    gap_server  The task to send the message to.
    cid   The connection ID of the device requesting to read the characteristic.
    offset offset
    
RETURNS
    TRUE if the message was sent, FALSE otherwise.
*/
bool gattGapServerSendReadDeviceNameInd(GGAPS *gap_server, uint16 cid, uint16 offset);


/*******************************************************************************
NAME
    gattGapServerSendReadAppearanceInd
    
DESCRIPTION
    Build and send a GATT_GAP_SERVER_READ_APPEARANCE_IND message.
    
PARAMETERS
    gap_server  The task to send the message to.
    cid   The connection ID of the device requesting to read the characteristic.
    
RETURNS
    TRUE if the message was sent, FALSE otherwise.
*/
bool gattGapServerSendReadAppearanceInd(const GGAPS *gap_server, uint16 cid);

#endif
