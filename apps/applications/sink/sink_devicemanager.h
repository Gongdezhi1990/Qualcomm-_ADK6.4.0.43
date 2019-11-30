/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_DEVICEMANAGER_H_
#define _SINK_DEVICEMANAGER_H_

#include <connection.h>

#include "sink_configmanager.h"
#include "sink_gatt_server.h"
#include "sink_inquiry.h"
#include "sink_peer.h"
#include "sink_a2dp.h"

/* Link types the sink device supports */
typedef enum
{
    sink_none  = 0x00,
    sink_hfp   = 0x01,
    sink_a2dp  = 0x02,
    sink_avrcp = 0x04,
#ifdef ENABLE_SUBWOOFER
    sink_swat  = 0x08
#else
    sink_tws   = 0x08
#endif
}sink_link_type;

/* Type of devices known by the device manager */
typedef enum
{
    dev_type_none   = 0x0,
    dev_type_ag     = 0x1,  /* Audio Gateway devices */
    dev_type_sub    = 0x2   /* Subwoofer devices */
    
}sink_remote_device_type;

/* Link modes the sink device supports */
typedef enum
{
    sink_mode_unknown          = 0x00,
    sink_no_secure_connection  = 0x01,
    sink_secure_connection_mode   = 0x02
}sink_link_mode;


/* LE Link modes the sink device supports */
typedef enum
{
    sink_no_le_secure_connection  = 0x00,
    sink_le_secure_connection_mode   = 0x01
}sink_le_link_mode;

/* HFP attributes */
typedef struct
{
    uint8 volume;
} hfp_attributes;

/* A2DP attributes */
typedef struct
{
    uint8  volume;
    uint16 clock_mismatch;
    uint16 audio_enhancements;
} a2dp_attributes;

/* sub woofer attributes */
typedef struct
{
    uint8 sub_trim_idx;    
} sub_attributes;


/* All device attributes stored in PS */
typedef struct sink_attributes_str
{
    uint16                      peer_version;
    unsigned                    peer_supported_optional_codecs:8; 
    remote_features             peer_features:7;    /* ENABLE_PEER use only */
    sink_le_link_mode           le_mode:1;
    remote_device               peer_device:2;      /* ENABLE_PEER use only */    
    PeerTwsAudioRouting         master_routing_mode:2;
    PeerTwsAudioRouting         slave_routing_mode:2;
    sink_link_mode              mode:2;             /* ENABLE_BREDR_SC use only */
    sink_link_type              profiles:8;
    hfp_attributes              hfp;
    a2dp_attributes             a2dp;
#ifdef ENABLE_SUBWOOFER
    sub_attributes              sub;                /* ENABLE_SUBWOOFER use only */
#endif /* ENABLE_SUBWOOFER */ 
#ifdef GATT_ENABLED
    gatt_ccd_attributes_t       gatt_client;        /* GATT_ENABLED use only */
#endif /* GATT_ENABLED */
    unsigned                    audio_prompt_play_status:1;
    unsigned                    unused:15;
} sink_attributes;


typedef struct
{
    bdaddr                      bd_addr;
    sink_attributes             attributes;
    sink_remote_device_type     device_type;
} EVENT_UPDATE_ATTRIBUTES_T;


/* Connection mask */
typedef enum
{
    conn_hfp_pri  = 1 << 0,
    conn_hfp_sec  = 1 << 1,
    conn_a2dp_pri = 1 << 2,
    conn_a2dp_sec = 1 << 3
} conn_mask;

#define conn_hfp  (conn_hfp_pri | conn_hfp_sec)
#define conn_a2dp (conn_a2dp_pri | conn_a2dp_sec)

/* device capability */
#define DeviceSupportsProfile(device, profile) ((device->profiles & profile) == profile)

/* timer delay of 500ms to allow a connected device to become disconnected before
   atttempting another connection */
#define ALLOW_DEVICE_TO_DISCONNECT_DELAY 500


/****************************************************************************
NAME
    deviceManagerUpdateAttributesWithDeviceDefaults
    
DESCRIPTION
    Updates the attributes with the defaults for a specific device type.

RETURNS
    void
*/
void deviceManagerUpdateAttributesWithDeviceDefaults(sink_attributes* attributes, sink_remote_device_type device_type);


/****************************************************************************
NAME    
    deviceManagerClearAttributes
    
DESCRIPTION
    Clear sink_attributes struct

RETURNS
    void
*/
void deviceManagerClearAttributes(sink_attributes* attributes);


/****************************************************************************
NAME    
    deviceManagerGetDefaultAttributes
    
DESCRIPTION
    Get the default attributes for a specific device type. 
    All attributes not related to the device type will be cleared.

RETURNS
    void
*/
void deviceManagerGetDefaultAttributes(sink_attributes* attributes, sink_remote_device_type device_type);


/****************************************************************************
NAME    
    deviceManagerGetAttributes
    
DESCRIPTION
    Get the attributes for a given device, or check device is in PDL if 
    attributes is NULL.

RETURNS
    TRUE if device in PDL, otherwise FALSE
*/
bool deviceManagerGetAttributes(sink_attributes* attributes, const bdaddr* dev_addr);


/****************************************************************************
NAME    
    deviceManagerGetIndexedAttributes
    
DESCRIPTION
    Get the attributes for a given index of device

RETURNS
    TRUE if successful, otherwise FALSE
*/
bool deviceManagerGetIndexedAttributes(uint16 index, sink_attributes* attributes, typed_bdaddr* dev_addr);

/****************************************************************************
NAME
    deviceManagerGetAddrFromProfile

DESCRIPTION
    Get the attributes for a specific device profile

RETURNS
    TRUE if device profile is in PDL, otherwise FALSE
*/
bool deviceManagerGetAddrFromProfile(sink_link_type profile, typed_bdaddr* dev_addr);
        
/****************************************************************************
NAME
    deviceManagerStoreAttributes
    
DESCRIPTION
    Stores given attribute values against a given device in PS.

RETURNS
    void
*/
void deviceManagerStoreAttributes(sink_attributes* attributes, const bdaddr* dev_addr);


/****************************************************************************
NAME
    deviceManagerStoreDefaultAttributes
    
DESCRIPTION
    Stores the default attributes against a given device in PS.

RETURNS
    void
*/
void deviceManagerStoreDefaultAttributes(const bdaddr* dev_addr, sink_remote_device_type device_type);


/****************************************************************************
NAME
    deviceManagerUpdateAttributes
    
DESCRIPTION
    Stores the current attribute values for a given HFP/A2DP connection in
    PS.

RETURNS
    void
*/
void deviceManagerUpdateAttributes(const bdaddr* bd_addr, sink_link_type link_type, hfp_link_priority hfp_priority, a2dp_index_t a2dp_priority);

/****************************************************************************
NAME
    deviceManagerMarkTrusted
    
DESCRIPTION
     Sets a device as trusted

RETURNS
    void
*/
void deviceManagerMarkTrusted(const bdaddr* dev_addr);

/****************************************************************************
NAME
    deviceManagerDelayedUpdateAttributes
    
DESCRIPTION
    Store attributes contained in EVENT_UPDATE_ATTRIBUTES_T in PS

RETURNS
    void
*/
void deviceManagerDelayedUpdateAttributes(const EVENT_UPDATE_ATTRIBUTES_T* update);


/****************************************************************************
NAME
    deviceManagerSetPriority
    
DESCRIPTION
    Set a device's priority in the PDL

RETURNS
    void
*/
uint8 deviceManagerSetPriority(const bdaddr* dev_addr);

/****************************************************************************
NAME
    deviceManagerHandleProfilesConnection

DESCRIPTION
   Handle other profiles level connection with remote AG

RETURNS
    void
*/
void deviceManagerHandleProfilesConnection(const bdaddr * dev_addr);

/****************************************************************************
 NAME
 deviceManagerUpdatePriorityDevices

 DESCRIPTION
 Forces a type of device on top of the PDL

RETURNS
    void
*/
void deviceManagerUpdatePriorityDevices(void);

/****************************************************************************
NAME    
    deviceManagerRemoveDevice
    
DESCRIPTION
    Remove given device from the PDL

RETURNS
    void
*/
#define deviceManagerRemoveDevice(dev_addr) ConnectionSmDeleteAuthDevice(dev_addr)


/****************************************************************************
NAME    
    deviceManagerRemoveAllDevices
    
DESCRIPTION
    Remove all devices from the PDL

RETURNS
    void
*/
void deviceManagerRemoveAllDevices(void);


/****************************************************************************
NAME    
    deviceManagerProfilesConnected
    
DESCRIPTION
    compare passed in bdaddr against those of the current connected devices,
    if a match is found returns true

RETURNS
    TRUE or FALSE
*/
conn_mask deviceManagerProfilesConnected(const bdaddr * bd_addr);

/****************************************************************************
NAME    
    deviceManagerGetProfilesConnected
    
DESCRIPTION
    Get mask of all connected profiles.

RETURNS
    conn_mask indicating which profiles are connected.
*/
conn_mask deviceManagerGetProfilesConnected(void);

/****************************************************************************
NAME    
    deviceManagerNumConnectedDevs
    
DESCRIPTION
   determines the number of different connected devices, a device may connected
   both hfp and a2dp or only one of each
RETURNS
    number of connected devices
*/
uint16 deviceManagerNumConnectedDevs(void);

/****************************************************************************
NAME    
    deviceManagerNumOtherConnectedDevs
    
DESCRIPTION
   determines the number of different connected devices other than the one provided, a device may connected
   both hfp and a2dp or only one of each
RETURNS
    number of connected devices
*/
uint16 deviceManagerNumOtherConnectedDevs(const bdaddr *bd_addr);

/****************************************************************************
NAME    
    deviceManagerNumConnectedPeerDevs
    
DESCRIPTION
   determines the number of different connected peer devices, a device will only
   have connected a2dp
RETURNS
    number of connected peer devices
*/
uint16 deviceManagerNumConnectedPeerDevs(void);


/****************************************************************************
NAME    
    deviceManagerCanConnect
    
DESCRIPTION
   Determines if max number of possible connections have been made
RETURNS
   TRUE if connection can be made, FALSE otherwise
*/
bool deviceManagerCanConnect(void);

/****************************************************************************
NAME    
    deviceManagerIsSameDevice
    
DESCRIPTION
    Determines if the supplied HF and AV devices are actually one and the same

RETURNS
    TRUE if the devices are the same, FALSE otherwise
*/
bool deviceManagerIsSameDevice(a2dp_index_t a2dp_link, hfp_link_priority hfp_link);


/****************************************************************************
NAME    
    deviceManagerDeviceDisconnectedInd
    
DESCRIPTION
    Finds which device has been disconnected(primary or secondary) and sends the appropriate event to notify the same

RETURNS
    void
*/
void deviceManagerDeviceDisconnectedInd(const bdaddr* dev_addr);

/****************************************************************************
NAME    
    deviceManagerDisconnectNonGaiaDevices
    
DESCRIPTION
    Disconnect all the devices which are not connected to GAIA (assuming that sink can support only one GAIA session)

RETURNS
    void
*/
void deviceManagerDisconnectNonGaiaDevices(void);

/****************************************************************************
NAME    
    deviceManagerGetAttributeHfpVolume
    
DESCRIPTION
    Gets the HFP volume attribute associated with a paired device.
    The address of the paired device must be supplied as ag_addr.
    If the function returns TRUE, hfp_vol will contain the attribute value.

RETURNS
    TRUE if the HFP volume attribute was found, FALSE otherwise.
*/
bool deviceManagerGetAttributeHfpVolume(const bdaddr *ag_addr, uint16 *hfp_vol);

/****************************************************************************
NAME    
    deviceManagerIsAgSourceConnected
    
DESCRIPTION
    Determines whether there is at least one AG/Source connected with the device

RETURNS
    void
*/
#define deviceManagerIsAgSourceConnected() \
        (deviceManagerNumConnectedDevs() == deviceManagerNumConnectedPeerDevs() ? FALSE : TRUE)


/****************************************************************************
NAME    
    deviceManagerIsOnlyAgSourceConnected
    
DESCRIPTION
    Determines whether there is only AG Source connected with the device

RETURNS
    void
*/
#define deviceManagerIsOnlyAgSourceConnected() \
        ((deviceManagerNumConnectedDevs() && (!deviceManagerNumConnectedPeerDevs())) ? TRUE : FALSE)


/****************************************************************************
NAME    
    deviceManagerIsPeerConnected
    
DESCRIPTION
    Determines whether there is at least one peer device connected with the device

RETURNS
    void
*/
#define deviceManagerIsPeerConnected() \
        ((0 < deviceManagerNumConnectedPeerDevs()) ? TRUE : FALSE)

/****************************************************************************
NAME    
    deviceManagerIsBothAgSourceAndPeerConnected
    
DESCRIPTION
    Determines whether there is at least one peer device and one AG source device is connected 
    with the device

RETURNS
    void
*/
#define deviceManagerIsBothAgSourceAndPeerConnected() \
        (((deviceManagerNumConnectedDevs() > deviceManagerNumConnectedPeerDevs()) && deviceManagerNumConnectedPeerDevs()) ? TRUE : FALSE)

/****************************************************************************
NAME
    deviceManagerDisconnectAG

DESCRIPTION
    Disconnect the device whose address is passed in, disconnect all possible
    profiles with the exception of BLE profiles since this device might also
    be an RC device

RETURNS
    void
*/
void deviceManagerDisconnectAG(bdaddr * device_address);

/****************************************************************************
NAME    
    handleEventSysUpdateDevicesConnectedStatus
    
DESCRIPTION
    Handle the system event to update the state of the following flags:
        - do we have any AG connected?
        - do we have any peer device connected?

RETURNS
    void
*/
void handleEventSysUpdateDevicesConnectedStatus(void);


/****************************************************************************
NAME    
    deviceManagerGetConnectedDeviceLinkKey
    
DESCRIPTION
    Gets the link key for the connected device

RETURNS
    void
*/
void deviceManagerGetConnectedDeviceLinkKey(void);


/****************************************************************************
NAME    
    deviceManagerExtractDeviceLinkKey
    
DESCRIPTION
    Extract the link key being returned from the connection library

RETURNS
    void
*/
void deviceManagerExtractDeviceLinkKey(const CL_SM_GET_AUTH_DEVICE_CFM_T *cfm);

/****************************************************************************
NAME    
    stub_deviceManagerSetAsSourceConnected
    
DESCRIPTION
    Set the device manager as AG source connected

RETURNS
    void
*/
void stub_deviceManagerSetAsSourceConnected(bool ag_flag);

/****************************************************************************
NAME    
    stub_deviceManagerSetAsPeerConnected
    
DESCRIPTION
    Set the device manager as Peer connected

RETURNS
    void
*/
void stub_deviceManagerSetAsPeerConnected(bool peer_flag);

/****************************************************************************
NAME    
    stub_deviceManagerIsSourceConnected
    
DESCRIPTION
    Check if the device manager is connected to AG source

RETURNS
    Returns TRUE if the device manager is AG source connected
*/
uint16 stub_deviceManagerIsSourceConnected(void);

/****************************************************************************
NAME    
    stub_deviceManagerIsPeerConnected
    
DESCRIPTION
    Check if the device manager is connected to Peer

RETURNS
    Returns TRUE if the device manager is Peer connected
*/
uint16 stub_deviceManagerIsPeerConnected(void);
/****************************************************************************
NAME    
    deviceManagerGetProfileAddr
    
DESCRIPTION
    Get bluetooth address from connection mask

RETURNS
    TRUE if connection is valid, otherwise FALSE
*/
bool deviceManagerGetProfileAddr(conn_mask mask, bdaddr* dev_addr);
/****************************************************************************
NAME
    deviceManagerResetAudioPromptPlayDeviceAttribute

DESCRIPTION
    Reset device attribute which tracks audio prompt play status so that audio
    prompts are played only after power cycle and not during reconnections by
    same device.

RETURNS
    void
*/
void deviceManagerResetAudioPromptPlayDeviceAttribute(void);

#endif /* _SINK_DEVICEMANAGER_H_ */

