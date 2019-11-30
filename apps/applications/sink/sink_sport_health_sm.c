/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    Handles all the status of different connections when running activity monitoring.
*/

/****************************************************************************
    Header files
*/
#ifdef ACTIVITY_MONITORING
#include <connection_no_ble.h>
#include "sink_peer.h"
#include "sink_pio.h"

#include "sink_sport_health.h"
#include <sport_health_hub.h>
#include "sink_ble_gap.h"
#include "sink_activity_monitoring_config_def.h"
#include "sink_sport_health_sm.h"

#include "sink_debug.h"
#include "sink_configmanager.h"

#include "sink_a2dp.h"
#include "sink_ble.h"
#include "sink_main_task.h"

#ifdef DEBUG_DEV
    #define AP_DEBUG(x) DEBUG(x)
#else
    #define AP_DEBUG(x)
#endif   

#define INIT_PEER_PAIRING_TIMEOUT 6

static sink_device_status device_status;
static bool auto_pairing_enabled = FALSE;

/* Local functions */
static void sportHealthSMCleanUpMessageQueue(void);
static void sportHealthSMDeviceResetConnectionStatus(void);
static void sportHealthSMDeviceSetLeft(uint8 isLeft);
static sink_device_status sportHealthSMDeviceGetStatus(void);
static bool sportHealthSMDeviceIsLeft(void);
static bool sportHealthSMDeviceAgConnected(void);
static void sportHealthSMDeviceInitPeerPairing(void);
static bool sportHealthSMSetTwsAutoPairingFlag(void);
static void sportHealthSMDeviceStartUp(void);
static bool sportHealthSMDeviceBleConnected(void);
static bool sportHealthSMDevicePeerConnected(void);
static void sportHealthSMDeviceStateChanged(sink_status_change_type type);

/*******************************************************************************
NAME
    sportHealthSMCleanUpMessageQueue

DESCRIPTION
    Clear the messages for Activity Monitoring supported gatt notification timers

PARAMETERS
    None

RETURNS
    None
*/
static void sportHealthSMCleanUpMessageQueue(void)
{
    MessageCancelAll(&theSink.task, BLE_INTERNAL_MESSAGE_HR_READ_TIMER);
    MessageCancelAll(&theSink.task, BLE_INTERNAL_MESSAGE_RSC_READ_TIMER);
    MessageCancelAll(&theSink.task, BLE_INTERNAL_MESSAGE_LOGGING_READ_TIMER);
    MessageCancelAll(&theSink.task, BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER);
}

/*******************************************************************************
NAME
    sportHealthSMDeviceResetConnectionStatus

DESCRIPTION
    Reset the status of all connections

PARAMETERS
    None

RETURNS
    None
*/
static void sportHealthSMDeviceResetConnectionStatus(void)
{
    device_status.ble_connected = FALSE;
    device_status.ag_connected = FALSE;
    device_status.peer_connected = FALSE;
    device_status.ble_status_old = FALSE;
    device_status.ag_status_old = FALSE;
    device_status.peer_status_old = FALSE;
}

/*******************************************************************************
NAME
    sportHealthSMDeviceSetLeft

DESCRIPTION
    Set whether the device is left, based on the value of least significant bit of the BDADDR

PARAMETERS
    uint8   Set the isLeft value of local device as the least significant bit of the BDADDR

RETURNS
    None
*/
static void sportHealthSMDeviceSetLeft(uint8 isLeft)
{
    device_status.is_left = isLeft;
}

/*******************************************************************************
NAME
    sportHealthSMDeviceGetStatus

DESCRIPTION
    Get the status of the device

PARAMETERS
    None

RETURNS
    sink_device_status
*/
static sink_device_status sportHealthSMDeviceGetStatus(void)
{
    return device_status;
}

/*******************************************************************************
NAME
    sportHealthSMDeviceIsLeft

DESCRIPTION
    Check whether the device is left, based on the value of least significant bit of BDADDR

PARAMETERS
    None

RETURNS
    bool    True if is left side, False otherwise
*/
static bool sportHealthSMDeviceIsLeft(void)
{
    return (sportHealthSMDeviceGetStatus().is_left ? TRUE:FALSE);
}

/*******************************************************************************
NAME
    sportHealthSMDeviceAgConnected

DESCRIPTION
    Get the status of Ag connection

PARAMETERS
    None

RETURNS
    bool    True if Ag is connected, False otherwise
*/
static bool sportHealthSMDeviceAgConnected(void)
{
    return device_status.ag_connected;
}

/*******************************************************************************
NAME
    sportHealthSMDeviceAgConnected

DESCRIPTION
    Get the status of Ag connection

PARAMETERS
    None

RETURNS
    bool    True if Ag is connected, False otherwise
*/
static void sportHealthSMDeviceInitPeerPairing(void)
{
    MessageCancelAll(&theSink.task, EventUsrCancelPairing);
    if (sportHealthSMDeviceIsLeft())
    {
        MessageCancelAll(&theSink.task, EventUsrPeerSessionInquire);
        MessageSend(&theSink.task, EventUsrPeerSessionInquire, NULL);
        /* Left earbud should enter AG pairing mode after peer pairing,
         * proper action will be taken on receiption of this event,  */
        MessageSendLater(&theSink.task, EventUsrCancelPairing, 0, D_SEC(INIT_PEER_PAIRING_TIMEOUT));
    }
    else
    {
        MessageCancelAll(&theSink.task, EventUsrPeerSessionConnDisc);
        MessageSend(&theSink.task, EventUsrPeerSessionConnDisc, NULL);
        MessageSendLater(&theSink.task, EventUsrCancelPairing, 0, D_SEC(INIT_PEER_PAIRING_TIMEOUT));
    }
}

/*******************************************************************************
NAME
    sportHealthSMSetTwsAutoPairingFlag

DESCRIPTION
    Read the auto pairing enable value and set the flag

PARAMETERS
    None

RETURNS
    bool    auto pairing enabled or not
*/
static bool sportHealthSMSetTwsAutoPairingFlag(void)
{
    activity_monitoring_config_def_t *activity_monitoring_config_data = NULL;

    if(configManagerGetReadOnlyConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID,
                         (const void **)&activity_monitoring_config_data) > 0)
    {
        auto_pairing_enabled = activity_monitoring_config_data->auto_pairing_enabled;
        configManagerReleaseConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID);
    }

    return auto_pairing_enabled;
}

/* The sequence of pairing/connection when device start up is
 * Try to do peer pairing for TBD time, once connected with peer within the timeout,
 * right earbud start ble adv for sport & health, left earbud starts BREDR pairing procedure
 * with handset.
 * If no peer connected, the earbud of which ever side start ble discoverable & connectable
 * for sport & health with TBD timeout, goes to BREDR pairing procedure once ble connected.
 *
 * The behaviour for finding a peer afterwards needs TBD.
 */
static void sportHealthSMDeviceStartUp(void)
{
    sportHealthSMDeviceResetConnectionStatus();
    if (sportHealthSMSetTwsAutoPairingFlag())
    {
        MessageSend(&theSink.task, EventUsrPowerOn, NULL);
        sportHealthSMDeviceInitPeerPairing();
    }
}

/*******************************************************************************
NAME
    sportHealthSMDeviceBleConnected

DESCRIPTION
    Get the connection status over ble

PARAMETERS
    None

RETURNS
    bool    the state of the connection, True if connected, False otherwise
*/
static bool sportHealthSMDeviceBleConnected(void)
{
    return device_status.ble_connected;
}

/*******************************************************************************
NAME
    sportHealthSMDevicePeerConnected

DESCRIPTION
    Get the connection status with peer

PARAMETERS
    None

RETURNS
    bool    the state of the connection, True if connected, False otherwise
*/
static bool sportHealthSMDevicePeerConnected(void)
{
    return device_status.peer_connected;
}

/*******************************************************************************
NAME
    sportHealthSMDeviceStateChanged

DESCRIPTION
    Handles when the device connection status changes

PARAMETERS
    sink_status_change_type

RETURNS
    bool    auto pairing enabled or not
*/
static void sportHealthSMDeviceStateChanged(sink_status_change_type type)
{
    if (!auto_pairing_enabled)
        return;

    AP_DEBUG((" *DEV: sportHealthSMDeviceStateChanged isLeft:%d type:%d, peer:%d, ble:%d, ag:%d\n", sportHealthSMDeviceIsLeft(),
               type, sportHealthSMDevicePeerConnected(), sportHealthSMDeviceBleConnected(), sportHealthSMDeviceAgConnected()));
    switch (type)
    {
    case peer:
    {
        if (sportHealthSMDevicePeerConnected())
        {
            MessageCancelAll(&theSink.task, EventUsrCancelPairing);
            MessageCancelAll(&theSink.task, EventSysBleBondableTimeout);

            if (sportHealthSMDeviceIsLeft())
            {
                AP_DEBUG(("DEV: sportHealthSMDeviceStateChanged Left side connected to Peer, will disable algo and stop ADV\n"));
                sh_seq_enable(0);
                MessageCancelAll(&theSink.task, EventUsrBleStartBonding);

                gapStopAdvertising(gapGetAdvSpeed());
            }
            if (!sportHealthSMDeviceAgConnected() && !sportHealthSMDeviceBleConnected())
            {
                /* The left earbud should start handset pairing after paired with peer
                 * and no ble nor ag connections are up, the right one should start ble adv */
                if (sportHealthSMDeviceIsLeft())
                    MessageSend(&theSink.task, EventUsrEnterPairing, NULL);
                else
                    MessageSend(&theSink.task, EventUsrBleStartBonding, NULL);
            }
            if (sportHealthSMDeviceAgConnected() && sportHealthSMDeviceBleConnected())
            {
                /* This state should not happen based on the current state machine as peer pairing
                 * only happens when there is no ag connection nor ble connection.
                 * The chip cannot handle running algo and maintian two a2dp connections simultaneously,
                 * the left earbud should drop the ble connection once all three connections are up,
                 * the right one should drop BREDR connection with the ag */
            }
        }
        else
        {
            sportHealthSMDeviceInitPeerPairing();
            if (sportHealthSMDeviceIsLeft())
                sinkSportHealthEnableDefault();
        }
    }
        break;
    case ble:
    {
        MessageCancelAll(&theSink.task, EventUsrBleStartBonding);
        MessageCancelAll(&theSink.task, EventSysBleBondableTimeout);
        if (sportHealthSMDeviceBleConnected())
        {
            gapStopAdvertising(gapGetAdvSpeed());
            if (!sportHealthSMDeviceAgConnected()&&((!sportHealthSMDevicePeerConnected()&& !sportHealthSMDeviceIsLeft())||sportHealthSMDeviceIsLeft()))
            {
                /* This happens when earbud runs solo without peer */
                MessageSend(&theSink.task, EventUsrEnterPairing, NULL);
            }
        }
        else
        {
            sportHealthSMCleanUpMessageQueue();
            if (!sportHealthSMDeviceAgConnected() && !sportHealthSMDevicePeerConnected())
                sportHealthSMDeviceInitPeerPairing();
            else
               MessageSend(&theSink.task, EventUsrBleStartBonding, NULL);
        }
    }
        break;
    case ag:
    {
        if (!sportHealthSMDeviceAgConnected())
        {
            if (!sportHealthSMDeviceBleConnected() && !sportHealthSMDevicePeerConnected())
                sportHealthSMDeviceInitPeerPairing();
            else
               MessageSend(&theSink.task, EventUsrEnterPairing, NULL);
        }
        else
        {
            if (!sportHealthSMDeviceBleConnected() && !sportHealthSMDevicePeerConnected())
                MessageSend(&theSink.task, EventUsrBleStartBonding, NULL);
        }

    }
        break;

    default:
        break;
    }
}

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
void sinkSportHealthSMGetLocalAddr(void)
{
    ConnectionReadLocalAddr(&theSink.task);
}

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
void sinkSportHealthSMHandleBdAddrCfm(Message message)
{
    CL_DM_LOCAL_BD_ADDR_CFM_T *cfm = (CL_DM_LOCAL_BD_ADDR_CFM_T *)message;
    if (cfm->status != success)
        Panic();
    sportHealthSMDeviceSetLeft(cfm->bd_addr.lap & 0x01);

    AP_DEBUG(("setLocalAddr, bdaddr: %04X %02X %06lX \n",
              cfm->bd_addr.nap,
              cfm->bd_addr.uap,
              cfm->bd_addr.lap));

    AP_DEBUG(("Is Left [%d]\n", cfm->bd_addr.lap & 0x01));

    sportHealthSMDeviceStartUp();
}

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
void sinkSportHealthSMShutDown(void)
{
    PioSetPowerPin(FALSE);
    sh_seq_enable(0);
}

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
void sinkSportHealthSMHandleBleStartBonding(void)
{
    if ((sportHealthSMDeviceIsLeft() && sportHealthSMDevicePeerConnected()) || sportHealthSMDeviceBleConnected())
    {
        AP_DEBUG(("HS : BLE Stop ADV\n"));
        gapStopAdvertising(gapGetAdvSpeed());
    }
}

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
void sinkSportHealthSMDeviceSetConnectionStateBle(bool connected)
{
    if (!auto_pairing_enabled)
        return;
    device_status.ble_connected = connected;
    if (device_status.ble_connected != device_status.ble_status_old)
    {
        sportHealthSMDeviceStateChanged(ble);
        device_status.ble_status_old = device_status.ble_connected;
    }
}

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
void sinkSportHealthSMDeviceSetConnectionStateAg(bool connected)
{
    if (!auto_pairing_enabled)
        return;
    device_status.ag_connected = connected;
    if (device_status.ag_connected != device_status.ag_status_old)
    {
        sportHealthSMDeviceStateChanged(ag);
        device_status.ag_status_old = device_status.ag_connected;
    }
}

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
void sinkSportHealthSMDeviceSetConnectionStatePeer(bool connected)
{
    if (!auto_pairing_enabled)
        return;
    device_status.peer_connected = connected;
    if (device_status.peer_connected != device_status.peer_status_old)
    {
        sportHealthSMDeviceStateChanged(peer);
        device_status.peer_status_old = device_status.peer_connected;
    }
}

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
void sinkSportHealthSMDeviceStateCheck(MessageId id)
{
    if (auto_pairing_enabled)
    {
        AP_DEBUG((" *DEV: sinkSportHealthSMDeviceStateCheck isLeft:%d MessageId:0x%X, peer:%d, ble:%d, ag:%d\n",sportHealthSMDeviceIsLeft(),
                   id, sportHealthSMDevicePeerConnected(), sportHealthSMDeviceBleConnected(), sportHealthSMDeviceAgConnected()));
        switch (id)
        {
        case EventUsrCancelPairing:
        {
            /* This event will be triggered when timeout happens for init peer pairing
             * without finding a peer, where earbud should run in solo mode regardless of the side.
             * Ble for sport&health has higher priority, will interleave ble adv
             * with peer pairing until either connect is up before moving to next state.
             * Peer pairing failling after peer lost should also trigger this event to start running solo.
             */
            if (!sportHealthSMDeviceBleConnected())
            {
                MessageCancelAll(&theSink.task, EventUsrBleStartBonding);
                MessageSend(&theSink.task, EventUsrBleStartBonding, NULL);
            }
            if (!sportHealthSMDeviceAgConnected())
            {
                MessageCancelAll(&theSink.task, EventUsrEnterPairing);
                MessageSend(&theSink.task, EventUsrEnterPairing, NULL);
            }
        }
            break;
        case EventUsrEnterPairing:
        {
            /* The default configuration for the timeout event(EventSysPairingFail) following this event
             * is "Return to connectable mode", nothing to be done here. */
        }
            break;
        case EventUsrBleStartBonding:
        {

        }
            break;
        case EventSysBleBondableTimeout:
        {
            /* Init peer pairing if no connections are available when Ble adv timeout.
             * Restart Ble adv if either ag or peer is connected and no ble connection.
             * It should not happen where the Left earbud has peer connected when receiving this event. */
            if (!sportHealthSMDeviceBleConnected() && !sportHealthSMDevicePeerConnected() && !sportHealthSMDeviceAgConnected())
                sportHealthSMDeviceInitPeerPairing();
            if ((!sportHealthSMDevicePeerConnected() && sportHealthSMDeviceIsLeft() && !sportHealthSMDeviceBleConnected()) || !sportHealthSMDeviceIsLeft())
            {
                MessageCancelAll(&theSink.task, EventUsrBleStartBonding);
                MessageSend(&theSink.task, EventUsrBleStartBonding, NULL);
            }
        }
            break;
        }
    }
}

#endif
