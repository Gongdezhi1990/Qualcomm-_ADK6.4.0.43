/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 
FILE NAME
    transport_accessory.h
 
DESCRIPTION
    This file provides necessary routines for creating and managing a transport
    link using the Accessory protocol.
*/

#ifndef _TRANSPORT_ACCESSORY_H_
#define _TRANSPORT_ACCESSORY_H_

#include <message.h>

void transportAccessoryInit(void);
void transportAccessoryRegisterReq(Task task, uint8 transport_id);
void transportAccessoryDeregisterReq(Task task, uint8 transport_id);
void handleAccessoryMessage(Task task, MessageId id, Message message);

#endif /* _TRANSPORT_ACCESSORY_H_ */
