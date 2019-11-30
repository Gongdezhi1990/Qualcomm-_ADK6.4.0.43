/****************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ble.h

DESCRIPTION
    BLE functionality
*/

#ifndef _SINK_BLE_H_
#define _SINK_BLE_H_

#include "sink_ble_gap.h"

#include <csrtypes.h>
#include <message.h>


#ifdef GATT_ENABLED
#include "sink_gatt_device.h"


#define BLE_INTERNAL_MESSAGE_BASE 0

/* The bits used to enable BLE Advertising filters*/
#define ANCS_AD_BIT           0           /*!< @brief Bit used to enable ANCS advertising filter */
#define BATTERY_AD_BIT        1           /*!< @brief Bit used to enable Battery advertising filter */
#define DIS_AD_BIT            2           /*!< @brief Bit used to enable DIS advertising filter */
#define HID_AD_BIT            3           /*!< @brief Bit used to enable HID advertising filter */
#define IAS_AD_BIT            4           /*!< @brief Bit used to enable IAS advertising filter */
#define SPC_AD_BIT            5           /*!< @brief Bit used to enable SPC advertising filter */
#define HRS_AD_BIT            6           /*!< @brief Bit used to enable HRS advertising filter */
#define AMS_AD_BIT            7           /*!< @brief Bit used to enable AMS advertising filter */


typedef struct __ble_configuration
{
    uint16 bonding_pairing_period_s;
    uint16 bonding_connection_period_s;
    uint16 scan_interval_fast;
    uint16 scan_window_fast;
    uint16 gap_role_switch_timer_s;
    uint16 whitelist_scan_timeout_when_private_addr_s;
    uint16 scan_timer_fast_s;
    uint16 scan_interval_slow;
    uint16 scan_window_slow;
    uint16 adv_interval_min_fast;
    uint16 adv_interval_max_fast;
    uint16 adv_timer_fast_s;
    uint16 adv_interval_min_slow;
    uint16 adv_interval_max_slow;

    /*Fast Pair Service Advertisement Parameters*/
#ifdef ENABLE_FAST_PAIR    
    uint16 fast_pair_adv_timer_s;
#endif
} ble_configuration_t;


typedef struct __ble_connection_initial_parameters
{
    uint16 conn_interval_min;
    uint16 conn_interval_max;
    uint16 conn_latency;
    uint16 supervision_timeout;
    uint16 conn_attempt_timeout;
    uint16 conn_latency_max;
    uint16 supervision_timeout_min;
    uint16 supervision_timeout_max;
} ble_connection_initial_parameters_t; 


typedef struct __ble_connection_update_parameters
{
    uint16 conn_interval_min;
    uint16 conn_interval_max;
    uint16 conn_latency;
    uint16 supervision_timeout;
    uint16 ce_length_min;
    uint16 ce_length_max;
} ble_connection_update_parameters_t;


typedef struct __ble_advertising_parameters
{
    uint16 interval_min;
    uint16 interval_max;
} ble_advertising_parameters_t;


typedef struct __ble_scanning_parameters
{
    uint16 interval;
    uint16 window;
} ble_scanning_parameters_t;

typedef struct __ble_data
{
    TaskData task;              /* Handler for BLE messages */
    ble_gap_t gap;              /* GAP related data */
    gatt_data_t gatt[MAX_BLE_CONNECTIONS];           /* GATT related data */
    uint16      number_connections;
} ble_data_t;

typedef struct __sink_ble_global_data_t
{
    ble_data_t   ble;
}sink_ble_global_data_t;

/*visible to all BLE files */
extern sink_ble_global_data_t *gBleData;

#define BLE_DATA gBleData->ble

typedef enum __ble_internal_message_id
{
    BLE_INTERNAL_MESSAGE_INIT_COMPLETE = BLE_INTERNAL_MESSAGE_BASE,
    BLE_INTERNAL_MESSAGE_EVENT_NO_CONNECTIONS,
    BLE_INTERNAL_MESSAGE_WHITELIST_TIMER,
    BLE_INTERNAL_MESSAGE_FAST_SCAN_TIMER,
    BLE_INTERNAL_MESSAGE_FAST_ADV_TIMER,
    BLE_INTERNAL_MESSAGE_ENCRYPTION_RETRY_TIMER,
    BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES,
    BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED,
    BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER,
    BLE_INTERNAL_MESSAGE_HR_READ_TIMER,
    BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR,
    BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT,
    BLE_INTERNAL_MESSAGE_START_ADVERTISING_ON_ASSOCIATION_END,
#ifdef ACTIVITY_MONITORING
    BLE_INTERNAL_MESSAGE_RSC_READ_TIMER,
    BLE_INTERNAL_MESSAGE_LOGGING_READ_TIMER,
    BLE_INTERNAL_MESSAGE_EVENT_DEBUG_DELAY,
#endif

#ifdef ENABLE_FAST_PAIR
    BLE_INTERNAL_MESSAGE_FAST_PAIR_ADV_TIMER,
#endif

    /* End message limit */
    BLE_INTERNAL_MESSAGE_TOP
} ble_internal_message_id_t;


typedef struct __BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES
{
    uint16 cid;
} BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES_T;

typedef struct __BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED
{
    uint16 cid;
} BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED_T;

typedef struct __BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR
{
    gatt_client_connection_t *connection;
}BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR_T;

#endif /* GATT_ENABLED */


#ifdef GATT_ENABLED
#define sinkGetBleTask() (&BLE_DATA.task)
#else
#define sinkGetBleTask() (NULL)
#endif

/*******************************************************************************
NAME
    sinkBleInitialiseDevice
    
DESCRIPTION
    Initialises BLE functionality in the application.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleInitialiseDevice(void);
#else
#define sinkBleInitialiseDevice() ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleMsgHandler
    
DESCRIPTION
    Handle BLE related messages
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the message
    message The message payload
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkBleMsgHandler(task, id, message) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleIsActiveOnPowerOff
    
DESCRIPTION
    Finds if BLE operation is still active when the application is powered off.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#if defined(GATT_ENABLED) && defined(BLE_ENABLED_WHEN_OFF)
#define sinkBleIsActiveOnPowerOff() (TRUE)
#else
#define sinkBleIsActiveOnPowerOff() (FALSE)
#endif


/*******************************************************************************
NAME
    sinkBleBondableEvent
    
DESCRIPTION
    Acts on a BLE bondable event, which is an attempt to make the application bondable.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleBondableEvent(void);
#else
#define sinkBleBondableEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleConnectionPairingTimeoutEvent
    
DESCRIPTION
    Acts on a BLE Connection timeout event, which should exit Connection 
    or start pairing.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleConnectionPairingTimeoutEvent(void);
#else
#define sinkBleConnectionPairingTimeoutEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleBondableTimeoutEvent
    
DESCRIPTION
    Acts on a BLE bondable timeout event, which should exit bondable mode.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleBondableTimeoutEvent(void);
#else
#define sinkBleBondableTimeoutEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleMasterConnCompleteEvent
    
DESCRIPTION
    Acts on a BLE central connection complete event, which should exit bondable mode.
    
PARAMETERS
    cid     The connection ID for the completed connection.
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleMasterConnCompleteEvent(uint16 cid);
#else
#define sinkBleMasterConnCompleteEvent(cid) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleDisconnectionEvent
    
DESCRIPTION
    Acts on a BLE disconnection event linked to central role,
    which should resume scanning.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleDisconnectionEvent(uint16 cid);
#else
#define sinkBleDisconnectionEvent(cid) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleCancelAdvertisingEvent
    
DESCRIPTION
    Acts on a BLE cancel advertising event.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleCancelAdvertisingEvent(void);
#else
#define sinkBleCancelAdvertisingEvent() ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleSlaveConnIndEvent
    
DESCRIPTION
    Acts on a BLE remote connection success event.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleSlaveConnIndEvent(uint16 cid);
#else
#define sinkBleSlaveConnIndEvent(cid) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleCheckNoConnectionsEvent
    
DESCRIPTION
    Check connections and report if no connections exist.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleCheckNoConnectionsEvent(void);
#else
#define sinkBleCheckNoConnectionsEvent() ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleRetryEncryptionEvent
    
DESCRIPTION
    Retry Encryption upon host busy pairing encryption status.
    
PARAMETERS
    cid     The connection ID for the connection.
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleRetryEncryptionEvent(uint16 cid);
#else
#define sinkBleRetryEncryptionEvent(cid) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleEncryptionCompleteEvent
    
DESCRIPTION
    Post Encryption complete event.
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleEncryptionCompleteEvent(void);
#else
#define sinkBleEncryptionCompleteEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBlePowerOnEvent
    
DESCRIPTION
    BLE power on event.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBlePowerOnEvent(void);
#else
#define sinkBlePowerOnEvent() ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBlePowerOffEvent
    
DESCRIPTION
    BLE power off event.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBlePowerOffEvent(void);
#else
#define sinkBlePowerOffEvent() ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleGetConfiguration
    
DESCRIPTION
    Gets the BLE configurable data.
    
PARAMETERS
    None.
    
RETURNS
    The BLE configuration data.
*/
#ifdef GATT_ENABLED
const ble_configuration_t *sinkBleGetConfiguration(void);
#else
#define sinkBleGetConfiguration() ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleSetAdvertisingParamsDefault
    
DESCRIPTION
    Sets the default advertising parameters.
    
PARAMETERS
    None.
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleSetAdvertisingParamsDefault(uint16 adv_interval_min, uint16 adv_interval_max);
#else
#define sinkBleSetAdvertisingParamsDefault(adv_interval_min, adv_interval_max) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleSetMasterConnectionParamsDefault
    
DESCRIPTION
    Sets the default connection parameters when Master of the connection.
    
PARAMETERS
    None.
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleSetMasterConnectionParamsDefault(uint16 scan_interval, uint16 scan_window);
#else
#define sinkBleSetMasterConnectionParamsDefault(scan_interval, scan_window) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleSetMasterConnectionParamsUpdate
    
DESCRIPTION
    Sets the updated connection parameters when Master of the connection.
    
PARAMETERS
    taddr  Address of the remote device to update connection parameters with.
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleSetMasterConnectionParamsUpdate(typed_bdaddr *taddr);
#else
#define sinkBleSetMasterConnectionParamsUpdate(taddr) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleSetSlaveConnectionParamsUpdate
    
DESCRIPTION
    Sets the updated connection parameters when Slave of the connection.
    
PARAMETERS
    taddr  Address of the remote device to update connection parameters with.
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleSetSlaveConnectionParamsUpdate(typed_bdaddr *taddr);
#else
#define sinkBleSetSlaveConnectionParamsUpdate(taddr) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleGetAdvertisingParameters
    
DESCRIPTION
    Gets the advertising parameters based on speed setting.
    
PARAMETERS
    fast_adv    Should be TRUE if fast advertising. FALSE otherwise.
    adv_params  The advertising parameters will be set into this variable.
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleGetAdvertisingParameters(bool fast_adv, ble_advertising_parameters_t *adv_params);
#else
#define sinkBleGetAdvertisingParameters(fast_adv, adv_params) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleGetScanningParameters
    
DESCRIPTION
    Gets the scanning parameters based on speed setting.
    
PARAMETERS
    fast_scan    Should be TRUE if fast advertising. FALSE otherwise.
    scan_params  The scan parameters will be set into this variable.
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleGetScanningParameters(bool fast_scan, ble_scanning_parameters_t *scan_params);
#else
#define sinkBleGetScanningParameters(fast_scan, scan_params) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleDeleteMarkedDevices
    
DESCRIPTION
    Delete the last failed encryption paired device
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleDeleteMarkedDevices(void);
#else
#define sinkBleDeleteMarkedDevices() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleSimplePairingCompleteInd
    
DESCRIPTION
    Handle received CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND as a result of BLE bonding procedure.
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleSimplePairingCompleteInd(const CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T *ind);
#else
#define sinkBleSimplePairingCompleteInd(message) ((void)(0))
#endif


/*******************************************************************************
NAME
    ConnectionDmBleConnectionUpdateCompleteInd
    
DESCRIPTION
    Handle received CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND as a result of BLE Connection Parameter procedure.
    
PARAMETERS
    CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void ConnectionDmBleConnectionUpdateCompleteInd(const CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T* ind);
#else
#define ConnectionDmBleConnectionUpdateCompleteInd(ind) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkBleExitDiscoverableModeEvent
    
DESCRIPTION
    Triggers BLE ExitDiscoverableMode event on exiting discoverable mode(BR/EDR)
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#if defined GATT_ENABLED && defined ENABLE_FAST_PAIR
void sinkBleExitDiscoverableModeEvent(void);
#else 
#define sinkBleExitDiscoverableModeEvent() ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkBleClearWhiteListEvent
    
DESCRIPTION
    Triggers BLE ClearWhitelist event, which is part of deviceManagerRemoveAllDevices
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleClearWhiteListEvent(void);
#else
#define sinkBleClearWhiteListEvent() ((void)(0))
#endif


/****************************************************************************
NAME
    SinkBleConnectionParameterIsOutOfRange

DESCRIPTION
    This fucntion compares the connection paramter update values are in the acceptable range for
    1.Minimum allowed connection interval.
    2.Maximum allowed connection interval.
    3.Connection latency.

PARAMETERS
    ind  CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND message from remote device

RETURNS
    Returns TRUE if the connection parameter are not in the acceptable range or else
    returns FALSE.
*/
#ifdef GATT_ENABLED
bool SinkBleConnectionParameterIsOutOfRange(const CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T *ind);
#else
#define SinkBleConnectionParameterIsOutOfRange(ind) (TRUE)
#endif


/****************************************************************************
NAME    
    sinkBleCheckBdAddrtIsBonded
    
DESCRIPTION
    Determines if the typed Bluetooth address of the remote client device is in the paired device list.
    
PARAMETERS
    client_taddr  The typed Bluetooth address of the client device.
    public_taddr  Pointer to where public address will be placed
    
RETURNS    
    TRUE if the cient address was found in the paired device list, FALSE otherwise.

*/
#ifdef GATT_ENABLED
bool sinkBleCheckBdAddrtIsBonded(const typed_bdaddr *client_taddr, typed_bdaddr *public_taddr);
#else
#define sinkBleCheckBdAddrtIsBonded(client_taddr, public_taddr) (FALSE)
#endif

/*******************************************************************************
NAME
    sinkBleAssociationEvent
    
DESCRIPTION
    Acts on a BA BLE association event, which is an attempt to make the application assoicatable.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleAssociationEvent(void);
#else
#define sinkBleAssociationEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleCancelAssociationEvent
    
DESCRIPTION
    Acts on a BA BLE cancel association event, which is an attempt to cancel ongoing association.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleCancelAssociationEvent(void);
#else
#define sinkBleCancelAssociationEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleStartBroadcastEvent
    
DESCRIPTION
    Acts on a BA BLE start broadcast event, which is an attempt to start the non-conn IV adverts/scan for 
    non-conn IV adverts.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleStartBroadcastEvent(void);
#else
#define sinkBleStartBroadcastEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleStopBroadcastEvent
    
DESCRIPTION
    Acts on a BA BLE stop broadcast event, which is an attempt to stop the non-conn IV adverts/scan for 
    non-conn IV adverts.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef GATT_ENABLED
void sinkBleStopBroadcastEvent(void);
#else
#define sinkBleStopBroadcastEvent() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleDisconnectCentralLELink
    
DESCRIPTION
    Disconnects central LE link
    
PARAMETERS
    None
    
RETURNS
    TRUE if it was able to trigger central disconnection else FALSE
*/
#if defined GATT_ENABLED && defined ENABLE_BROADCAST_AUDIO
bool sinkBleDisconnectCentralLELink(void);
#else
#define sinkBleDisconnectCentralLELink() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleDisconnectOneLELink
    
DESCRIPTION
    Disconnects one LE link for reserving it for broadcast association.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#if defined GATT_ENABLED && defined ENABLE_BROADCAST_AUDIO
void sinkBleDisconnectOneLELink(void);
#else
#define sinkBleDisconnectOneLELink() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleStartAdvertisingOnAssociationEnd
    
DESCRIPTION
    Starts advertising standalone connectable adverts once BA association ends
    
PARAMETERS
    None
    
RETURNS
    None
*/
#if defined GATT_ENABLED && defined ENABLE_BROADCAST_AUDIO
void sinkBleStartAdvertisingOnAssociationEnd(void);
#else
#define sinkBleStartAdvertisingOnAssociationEnd() ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkBleHandleConnParamFailures
    
DESCRIPTION
    Handle specific failure cases when CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM or CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_IND is received
    This handling is only used if BLE upgrade is in progress and there is a need to retry the connection parameter
    The initial request is for a connection interval of 15ms by default and in case of failure, it will retry for a connection interval of 30ms.
    This function keeps retrying until there is a parameter accept success
    
PARAMETERS
    status  status of the CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_REQ
    
RETURNS
    None
*/
#if defined GATT_ENABLED && defined ENABLE_GAIA && defined ENABLE_UPGRADE
void sinkBleHandleConnParamFailures(uint16 status);
#else
#define sinkBleHandleConnParamFailures(status) UNUSED(status)
#endif

/*******************************************************************************
NAME
    sinkBleResetParamsForUpgrade
    
DESCRIPTION
    Reset any BLE parameters required to start applying a new connection parameter update 
    This is only needed for Applying of Upgrade parameters (and not for Revert)

PARAMETERS
    void
    
RETURNS
    None
*/
#if defined GATT_ENABLED && defined ENABLE_GAIA && defined ENABLE_UPGRADE
void sinkBleResetParamsForUpgrade(void);
#else
#define sinkBleResetParamsForUpgrade() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleApplyParamsForUpgrade
    
DESCRIPTION
    Apply BLE params for Upgrade: Set BLE Slave Connection Params For Upgrade
    
PARAMETERS
    cid     Connection Id
    apply If true, apply Upgrade parameters, else revert to original connection parameters
    
RETURNS
    None.
*/
#if defined GATT_ENABLED && defined ENABLE_GAIA && defined ENABLE_UPGRADE
void sinkBleApplyParamsForUpgrade(uint16 cid, bool apply);
#else
#define sinkBleApplyParamsForUpgrade(cid, apply) ((void)(0))
#endif

#ifdef GATT_BISTO_COMM_SERVER
void sinkBleApplyConnectionParamsForVoiceCapture(uint16 cid, bool apply);
#else
#define sinkBleApplyConnectionParamsForVoiceCapture(cid, apply) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleIndicateSCOStatusChanged
    
DESCRIPTION
    Indicate BLE state machine that SCO link status is changed
    
PARAMETERS
    None
    
RETURNS
    None
*/
#if defined GATT_ENABLED && defined ENABLE_FAST_PAIR
void sinkBleIndicateSCOStatusChanged(void);
#else
#define sinkBleIndicateSCOStatusChanged() ((void)(0))
#endif

#endif /* _SINK_BLE_H_ */
