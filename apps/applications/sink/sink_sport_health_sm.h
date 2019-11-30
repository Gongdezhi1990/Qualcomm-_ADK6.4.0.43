/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_SPORT_HEALTH_SM_H_
#define _SINK_SPORT_HEALTH_SM_H_

#ifdef ACTIVITY_MONITORING
/* Status of the sink device */
typedef struct
{
    bool ble_status_old;
    bool ag_status_old;
    bool peer_status_old;
    bool ble_connected;
    bool ag_connected;
    bool peer_connected;
    uint8 is_left:1;
} sink_device_status;

/* Connection status of the sink device that changed */
typedef enum
{
    ble,
    ag,
    peer
} sink_status_change_type;
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMGetLocalAddr

DESCRIPTION
    Send command to get the bdaddr

PARAMETERS
    None

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMGetLocalAddr(void);
#else
#define sinkSportHealthSMGetLocalAddr()
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMHandleBdAddrCfm

DESCRIPTION
    Handles the read local bdaddr confirmation message

PARAMETERS
    Message the CL_DM_LOCAL_BD_ADDR_CFM message received

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMHandleBdAddrCfm(Message message);
#else
#define sinkSportHealthSMHandleBdAddrCfm(message) UNUSED(message)
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMShutDown

DESCRIPTION
    Handles Power off event

PARAMETERS
    None

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMShutDown(void);
#else
#define sinkSportHealthSMShutDown()
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMHandleBleStartBonding

DESCRIPTION
    Handles BleStartBonding event

PARAMETERS
    None

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMHandleBleStartBonding(void);
#else
#define sinkSportHealthSMHandleBleStartBonding()
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMDeviceSetConnectionStateBle

DESCRIPTION
    Set the BLE connection state

PARAMETERS
    bool    the state of the connection, True if connected, False otherwise

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMDeviceSetConnectionStateBle(bool connected);
#else
#define sinkSportHealthSMDeviceSetConnectionStateBle(connected) UNUSED(connected)
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMDeviceSetConnectionStateAg

DESCRIPTION
    Set the Ag connection state

PARAMETERS
    bool    the state of the connection, True if connected, False otherwise

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMDeviceSetConnectionStateAg(bool connected);
#else
#define sinkSportHealthSMDeviceSetConnectionStateAg(connected) UNUSED(connected)
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMDeviceSetConnectionStatePeer

DESCRIPTION
    Set the Peer connection state

PARAMETERS
    bool    the state of the connection, True if connected, False otherwise

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMDeviceSetConnectionStatePeer(bool connected);
#else
#define sinkSportHealthSMDeviceSetConnectionStatePeer(connected) UNUSED(connected)
#endif

/*******************************************************************************
NAME
    sinkSportHealthSMDeviceStateCheck

DESCRIPTION
    Handle message ids based on which the device witll change its behavior

PARAMETERS
    id      The ID for the message

RETURNS
    None
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSMDeviceStateCheck(MessageId id);
#else
#define sinkSportHealthSMDeviceStateCheck(id) UNUSED(id)
#endif

#endif /* _SINK_SPORT_HEALTH_SM_H_ */

