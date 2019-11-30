/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_hid.h
 
DESCRIPTION
    Interface file to the hid profile initialisation and control functions. 
*/
  
/*!
@file    sink_hid.h
@brief   Interface file to the hid profile initialisation and control functions. 

*/

#ifndef SINK_HID_H_
#define SINK_HID_H_

#include <hid.h>

#ifdef ENABLE_HID
#include <message.h>
#include <sink.h>
#endif

/*******************************************************************************
NAME
    sinkHidHandleMessage

DESCRIPTION
    This function handles HID messages received from the HID library

PARAMETERS
    task         The task the message is delivered
    pId          The ID for the HID message
    pMessage The message payload

RETURNS
    void
*/
#ifdef ENABLE_HID
void sinkHidHandleMessage(Task task, MessageId pId, Message pMessage);
#else
#define sinkHidHandleMessage(task, pId, pMessage) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkHidInit

DESCRIPTION
    This function initialises the HID library

PARAMETERS
    void

RETURNS
    void
*/
#ifdef ENABLE_HID
void sinkHidInit(void);
#else
#define sinkHidInit() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkHidConnectRequest

DESCRIPTION
    This function sends HID connect request to the remote bd address supplied

PARAMETERS
    Bluetooth address of the HID host

RETURNS
    void
*/

#ifdef ENABLE_HID
void sinkHidConnectRequest(bdaddr * pAddr);
#else
#define sinkHidConnectRequest(pAddr) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkHidDisconnectRequest

DESCRIPTION
    This function sends HID disconnect request to the remote bd address supplied

PARAMETERS
    Bluetooth address of the HID host

RETURNS
    void
*/

#ifdef ENABLE_HID
void sinkHidDisconnectRequest(const bdaddr * pAddr);
#else
#define sinkHidDisconnectRequest(pAddr) ((void)(0))
#endif


/******************************************************************************
NAME
    sinkHidDisconnect

DESCRIPTION
     Disconnect all HID links

PARAMETERS
    void

RETURNS
    void
*/
#ifdef ENABLE_HID
void sinkHidDisconnectAllLinks(void);
#else
#define sinkHidDisconnectAllLinks() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkHidUserEvents

DESCRIPTION
    This function handles HID user events received

PARAMETERS
    Id          Message ID for the HID user event

RETURNS
    void
*/
#ifdef ENABLE_HID
bool sinkHidUserEvents(MessageId Id);
#else
#define sinkHidUserEvents(Id) (FALSE)
#endif

/****************************************************************************
NAME
    sinkHidIsEnabled

DESCRIPTION
    Returns TRUE if HID is enabled. FALSE otherwise.

PARAMETERS
    void

RETURNS
    void
*/
#ifdef ENABLE_HID
#define sinkHidIsEnabled() (TRUE)
#else
#define sinkHidIsEnabled() (FALSE)
#endif


#endif /* SINK_HID_H_ */
