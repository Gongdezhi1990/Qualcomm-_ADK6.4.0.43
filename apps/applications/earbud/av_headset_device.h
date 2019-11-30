/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_device.h
\brief	    Header file for the device management.
*/

#ifndef _AV_HEADSET_DEVICE_H_
#define _AV_HEADSET_DEVICE_H_

#include <connection.h>
#include "av_headset_tasklist.h"

/*! \brief Types of devices. */
typedef enum
{
    DEVICE_TYPE_UNKNOWN = 0, /*!< Remote device type is unknown (default) */
    DEVICE_TYPE_EARBUD,		 /*!< Remote device is an earbud */
    DEVICE_TYPE_HANDSET,     /*!< Remote device is a handset */
} deviceType;


/*! \brief Types of device link modes. */
typedef enum
{
    DEVICE_LINK_MODE_UNKNOWN = 0,			/*!< The Bluetooth link mode is unknown */
    DEVICE_LINK_MODE_NO_SECURE_CONNECTION,  /*!< The Bluetooth link is not using secure connections */
    DEVICE_LINK_MODE_SECURE_CONNECTION,     /*!< The Bluetooth link is using secure connections */
} deviceLinkMode;

/*! \brief TWS Standard version number. */
#define DEVICE_TWS_STANDARD   (0x0000)
/*! \brief TWS+ version number. */
#define DEVICE_TWS_VERSION  (0x0500)
/*! \brief TWS version number not known. */
#define DEVICE_TWS_UNKNOWN  (0xFFFF)

/*! \brief Device supports HFP */
#define DEVICE_PROFILE_HFP     (1 << 0)
/*! \brief Device supports A2DP */
#define DEVICE_PROFILE_A2DP    (1 << 1)
/*! \brief Device supports AVRCP */
#define DEVICE_PROFILE_AVRCP   (1 << 2)
/*! \brief Device supports SCO Forwarding. */
#define DEVICE_PROFILE_SCOFWD   (1 << 3)

/*! Bit in handset flags defining if we need to send the link key to the peer earbud. */
#define DEVICE_FLAGS_HANDSET_LINK_KEY_TX_REQD       (1 << 0)
/*! Bit in handset flags defining if we need to handset address to the peer earbud. */
#define DEVICE_FLAGS_HANDSET_ADDRESS_FORWARD_REQD   (1 << 1)
/*! Bit in handset flags indicating handset has just paired and we shouldn't
 * initiate connection to it.  Flag will be cleared as soon as handset connects to us. */
#define DEVICE_FLAGS_JUST_PAIRED                    (1 << 2)
/*! Bit in handset flags indicating this handset was pre-paired on request from peer. */
#define DEVICE_FLAGS_PRE_PAIRED_HANDSET             (1 << 3)
/*!Bit in flags indicating that whilst device type is peer, it is actually PTS tester. */
#define DEVICE_FLAGS_IS_PTS                         (1 << 4)
/*!Bit in flags indicating that this device has been paired over BLE with a Random Resolvable Address */
#define DEVICE_FLAGS_RRA_PAIRED                     (1 << 5)

/*! Device attributes store in Persistent Store */
typedef struct appDeviceAttributes
{
    uint8 a2dp_num_seids;       /*!< Number of supported SEIDs */
    uint8 a2dp_volume;			/*!< Last set volume for the device */
    uint8 hfp_profile;          /*!< HFP or HSP profile */

    uint8 dev_info_version;     /*!< Version of this structure */
    deviceType type;            /*!< Type of device DEVICE_TYPE_EARBUD or DEVICE_TYPE_HANDSET */
    deviceLinkMode link_mode;   /*!< Device Link mode */
    uint8 supported_profiles;   /*!< Bitmap of supported profiles */
    uint8 connected_profiles;   /*!< Bitmap of connected profiles */
    uint16 tws_version;         /*!< TWS+ version number, MSB major, LSB minor. 0 if standard device */
    uint8 flags;                /*!< Misc. flags */
    uint16 sco_fwd_features;    /*!< Supported SCO forwarding features */
} appDeviceAttributes;

/*! \brief appDeviceAttributes structure must be an even number of octets, otherwise
 * ConnectionSmGetIndexedAttributeNowReq will correct the octet in memory
 * after the structure */
STATIC_ASSERT((sizeof(appDeviceAttributes) % 2 == 0), appDeviceAttributes_not_even);

/*! \brief Device manager task data. */
typedef struct
{
    TaskData task;              /*!< Device Manager task */

    bdaddr handset_bd_addr;     /*!< Bluetooth Address of remote device */
    uint16 handset_tws_version; /*!< Handset TWS version, 0 if standard handset */
    bool   handset_connected;   /*!< Is handset currently connected? */
    bool   handset_paired;      /*!< Is handset currently paired? */

    bdaddr peer_bd_addr;        /*!< Address of peer device, 0 if not paired with peer */
    uint16 peer_tws_version;    /*!< Peer TWS version */
    bool   peer_connected;      /*!< Is peer currently connected? */
    TaskList *device_version_client_tasks; /*!< List of tasks interested in device version changes */
} deviceTaskData;


/*! \brief Message IDs for device manager messages to other tasks. */
enum deviceMessages
{
    /*! Message ID for a \ref DEVICE_VERSION_IND_T message sent when
        the device TWS version is updated */
    DEVICE_VERSION_IND = DEVICE_MESSAGE_BASE,
};

/*! Definition of message sent to clients to indicate connection status. */
typedef struct
{
    bdaddr bd_addr;                     /*! BT address of device */   
    uint16 tws_version;                 /*! New TWS version */
    uint16 previous_tws_version;        /*! Previous TWS version */
} DEVICE_VERSION_IND_T;

/*! \brief Initialse the device manager application module. */
extern void appDeviceInit(void);

/*! \brief Initialise a device attributes. 

    \param attributes Attributes for a device to be initialised.
*/
extern void appDeviceInitAttributes(appDeviceAttributes *attributes);

/*! \brief Find device attributes for a given BT address. 

    \param bd_addr Pointer to read-only device BT address.
    \param attributes Attributes of the requested device, if found.
    \return TRUE if device found, FALSE otherwise
*/
extern bool appDeviceFindBdAddrAttributes(const bdaddr *bd_addr, appDeviceAttributes *attributes);

/*! \brief Get peer earbud attributes. 

    \param bd_addr BT address of the peer earbud.
    \param attributes Attributes of the peer earbud if found.
    \return TRUE if peer earbud device found, FALSE otherwise
*/
extern bool appDeviceGetPeerAttributes(bdaddr *bd_addr, appDeviceAttributes *attributes);

/*! \brief Get peer BT address. 

    \param bd_addr BT address of the peer earbud if found. A NULL pointer may be
    passed if the caller just wants to determine if a peer is paired, but doesn't
    want to get the BT address.
    \return TRUE if peer earbud device found, FALSE otherwise
*/
extern bool appDeviceGetPeerBdAddr(bdaddr *bd_addr);

/*! \brief Get handset BT address and attributes.

    Return the first handset BT address and attributes from the PDL starting at
    index. Can be used to iterate through the known handset devices with repeated
    calls incrementing the index parameter.

    Passing a NULL as the index parameter will return the first handset instance
    found.

    \param bd_addr  BT address of handset device at index.
    \param attributes  Attributes of handset device at index.
    \param[in,out] index Pointer to value holding the index. On entry will start 
     searching at this index. On return this holds the index at which the
     handset was found

    \return bool TRUE success, FALSE failure, no more handset attributes.
*/
extern bool appDeviceGetHandsetAttributes(bdaddr *bd_addr,
                                          appDeviceAttributes *attributes,
                                          int *index);

/*! \brief Get handset BT address.

    Return the first handset BT address.

    \param bd_addr BT address of first handset device.
    \return bool TRUE success, FALSE failure and Bluetooth address not set.
*/
extern bool appDeviceGetHandsetBdAddr(bdaddr *bd_addr);

/*! \brief Set a device attributes. 

	\param bd_addr Pointer to read-only device BT address.
    \param attributes Updated attributes to set.
*/
extern void appDeviceSetAttributes(const bdaddr *bd_addr, appDeviceAttributes *attributes);

/*! \brief Get the TWS version for a given BT address. 

	\param bd_addr Pointer to read-only device BT address.
    \return uint16 Device TWS version or DEVICE_TYPE_UNKNOWN if device not found
*/
extern uint16 appDeviceTwsVersion(const bdaddr *bd_addr);

/*! \brief Set the TWS version for a given BT address. 

    \param bd_addr Pointer to read-only device BT address.
    \param tws_version TWS version of the device to set
*/
extern void appDeviceSetTwsVersion(const bdaddr *bd_addr, uint16 tws_version);

/*! \brief Determine if a BR/EDR address matches a known handset device. 

    \note This function should only be called if the address is known
    to be for a standard Bluetooth device (not Bluetooth Low Energy).
    Where a typed Bluetooth Address is used (\ref typed_bdaddr) 
    the type must be \ref TYPED_BDADDR_PUBLIC, but this is not 
    a sufficient check.

    \note \ref appDeviceTypeIsHandset() is an alternative function that
    can be used if the device may not have been fully paired yet.

    \param bd_addr Pointer to read-only device BT address.

    \return bool TRUE if is a handset and attributes have been exchange, 
        FALSE otherwise.
*/
extern bool appDeviceTypeIsHandset(const bdaddr *bd_addr);

/*! \brief Determine if a BR/EDR address matches a handset device. 

    \note this function should only be used if the device may not
    be fully paired. \ref appDeviceIsHandset() also includes a check
    that attribute exchange has completed.

    \note This function should only be called if the address is known
    to be for a standard Bluetooth device (not Bluetooth Low Energy).
    Where a typed Bluetooth Address is used (\ref typed_bdaddr) 
    the type must be \ref TYPED_BDADDR_PUBLIC, but this is not 
    a sufficient check.

    \param bd_addr Pointer to read-only device BT address.

    \return bool TRUE if is a handset, FALSE if not a handset.
*/
extern bool appDeviceIsHandset(const bdaddr *bd_addr);

/*! \brief Determine if a device is an Earbud.

    \note This function should only be called if the address is known
    to be for a standard Bluetooth device (not Bluetooth Low Energy).
    Where a typed Bluetooth Address is used (\ref typed_bdaddr) 
    the type must be \ref TYPED_BDADDR_PUBLIC, but this is not 
    a sufficient check.

    \param bd_addr Pointer to read-only device BT address.

    \return bool TRUE if is an earbud, FALSE if not an earbud.
 */
extern bool appDeviceIsPeer(const bdaddr *bd_addr);

/*! \brief Determine if a device supports A2DP

    \param bd_addr Pointer to read-only device BT address.
    \return bool TRUE if A2DP connected, FALSE if not supported.
*/
bool appDeviceIsA2dpSupported(const bdaddr *bd_addr);

/*! \brief Determine if a device supports AVRCP

    \param bd_addr Pointer to read-only device BT address.
    \return bool TRUE if AVRCP connected, FALSE if not supported.
*/
bool appDeviceIsAvrcpSupported(const bdaddr *bd_addr);

/*! \brief Determine if a device supports HFP

    \param bd_addr Pointer to read-only device BT address.
    \return bool TRUE if HFP connected, FALSE if not supported.
*/
bool appDeviceIsHfpSupported(const bdaddr *bd_addr);

/*! \brief Determine if a device had profiles connected

    \param bd_addr Pointer to read-only device BT address.
    \return bool TRUE if device was connected, FALSE otherwise.
*/
bool appDeviceWasConnected(const bdaddr *bd_addr);

/*! \brief Determine which profiles were connected to a device.

    \param bd_addr Pointer to read-only device BT address.
    \return uint8 Bitmask of profiles that were connected.
                  DEVICE_PROFILE_HFP | DEVICE_PROFILE_AVRCP | DEVICE_PROFILE_A2DP 
*/
uint8 appDeviceWasConnectedProfiles(const bdaddr *bd_addr);

/*! \brief Writes the device attribute profile connected/supported flags based on
    peer's profile connection state. Is much faster than calling individual
    WasConnected/IsSupported functions for each profile. As it only read attributes
    once and only writes attributes when modified.
    \param bd_addr The peer's handset address.
    \param peer_a2dp_connected TRUE if the peer's A2DP profile is connected to the handset.
    \param peer_avrcp_connected TRUE if the peer's AVRCP profile is connected to the handset.
    \param peer_hfp_connected TRUE if the peer's HFP profile is connected to the handset.
    \param peer_hfp_profile_connected The hfp version of the peer's handset.
    \return uint8 Bitmask of profiles that were known to be supported before this function was called.
*/
uint8 appDeviceSetProfileConnectedAndSupportedFlagsFromPeer(const bdaddr *bd_addr,
    bool peer_a2dp_connected, bool peer_avrcp_connected,
    bool peer_hfp_connected, hfp_profile peer_hfp_profile_connected);

/*! \brief Set flag to indicate HFP was connected or not

    \param bd_addr Pointer to read-only device BT address.
    \param connected TRUE if HFP was connected, FALSE if HFP wasn't connected.
*/
void appDeviceSetHfpWasConnected(const bdaddr *bd_addr, bool connected);

/*! \brief Set flag to indicate A2DP was connected or not

    \param bd_addr Pointer to read-only device BT address.
    \param connected TRUE if A2DP was connected, FALSE if A2DP wasn't connected.
*/
void appDeviceSetA2dpWasConnected(const bdaddr *bd_addr, bool connected);

/*! \brief Set flag to indicate SCOFWD was connected or not

    \param bd_addr Pointer to read-only device BT address.
    \param connected TRUE if SCOFWD was connected, FALSE otherwise.
*/
void appDeviceSetScoFwdWasConnected(const bdaddr *bd_addr, bool connected);

/*! \brief Set flag to indicate A2DP is supported on a device

    \param bd_addr Pointer to read-only device BT address.
*/
void appDeviceSetA2dpIsSupported(const bdaddr *bd_addr);

/*! \brief Set flag to indicate AVRCP is supported on a device

    \param bd_addr Pointer to read-only device BT address.
*/
void appDeviceSetAvrcpIsSupported(const bdaddr *bd_addr);

/*! \brief Set flag to indicate HFP is supported on a device

    \param bd_addr Pointer to read-only device BT address.
    \param profile HFP profile version support by the device
*/
void appDeviceSetHfpIsSupported(const bdaddr *bd_addr, hfp_profile profile);

/*! \brief Set device supported link mode

    \param bd_addr Pointer to read-only device BT address.
    \param link_mode The device supported deviceLinkMode
*/
void appDeviceSetLinkMode(const bdaddr *bd_addr, deviceLinkMode link_mode);

/*! \brief Determine if a device supports secure connections

    \param bd_addr Pointer to read-only device BT address.
    \return bool TRUE if secure connects supported, FALSE otherwise.
*/
bool appDeviceIsSecureConnection(const bdaddr *bd_addr);

/*! \brief Set flag for handset device indicating if link key needs to be sent to 
           peer earbud.
           
    \param handset_bd_addr BT address of handset device.
    \param reqd  TRUE link key TX is required, FALSE link key TX not required.
    \return bool TRUE Success, FALSE failure. 
 */
bool appDeviceSetHandsetLinkKeyTxReqd(bdaddr *handset_bd_addr, bool reqd);

/*! \brief For handset device address determine if link key needs to be sent to
           peer earbud. 
           
    \param handset_bd_addr BT address of handset device.
    \param reqd TRUE Flag is set, link key is required to be sent to peer earbud.
                 FALSE Flag is clear, link key does not need to be sent to peer earbud.  
    \return bool TRUE Success, FALSE failure device not known.
 */
bool appDeviceGetHandsetLinkKeyTxReqd(bdaddr *handset_bd_addr, bool *reqd);

/*! \brief Set device volume

    \param bd_addr Pointer to read-only device BT address.
    \param volume The device volume
    \note The volume should only be set for handsets.
*/
bool appDeviceSetVolume(const bdaddr *bd_addr, uint8 volume);

/*! \brief Get device volume

    \param bd_addr Pointer to read-only device BT address.
    \param[out] volume The device's volume
    \note The volume should only be requested for handsets.
*/
bool appDeviceGetVolume(const bdaddr *bd_addr, uint8 *volume);


/*! \brief For a handset device attributes determine if link key needs to be
           sent to peer earbud.
           
    \param attr  Pointer to handset device attributes.
    \return bool TRUE Need to send link key to peer earbud.
                 FALSE Do not need to send link key to peer earbud.
*/
bool appDeviceHandsetAttrIsLinkKeyTxReqd(appDeviceAttributes* attr);

/*! \brief Determine if a handset is connected.

    \return bool TRUE handset is connected, FALSE handset is not connected.
*/
bool appDeviceIsHandsetConnected(void);

/*! \brief Determine if a handset has A2DP disconnected.

    \return bool TRUE handset is disconnected, FALSE handset is connected.
*/
bool appDeviceIsHandsetA2dpDisconnected(void);

/*! \brief Determine if a handset has A2DP connected.

    \return bool TRUE handset is connected, FALSE handset is not connected.
*/
bool appDeviceIsHandsetA2dpConnected(void);

/*! \brief Determine if a handset is streaming A2DP.

    \return bool TRUE handset is streaming, FALSE handset is not streaming.
*/
bool appDeviceIsHandsetA2dpStreaming(void);

/*! \brief Determine if a handset AVRCP disconnected.

    \return bool TRUE handset is disconnected, FALSE handset is connected.
*/
bool appDeviceIsHandsetAvrcpDisconnected(void);

/*! \brief Determine if a handset has AVRCP connected.

    \return bool TRUE handset is connected, FALSE handset is not connected.
*/
bool appDeviceIsHandsetAvrcpConnected(void);

/*! \brief Determine if a handset has HFP connected.

    \return bool TRUE handset is connected, FALSE handset is not connected.
*/
bool appDeviceIsHandsetHfpConnected(void);

/*! \brief Determine if a handset has HFP disconnected.

    \return bool TRUE handset is disconnected, FALSE handset is not disconnected.
*/
bool appDeviceIsHandsetHfpDisconnected(void);

/*! \brief Determine if a connected to peer earbud.

    \return bool TRUE peer is connected, FALSE peer is not connected.
*/
bool appDeviceIsPeerConnected(void);

/*! \brief Determine if a connected with A2DP to peer earbud.

    \return bool TRUE peer is connected, FALSE peer is not connected.
*/
bool appDeviceIsPeerA2dpConnected(void);

/*! \brief Determine if a connected with AVRCP to peer earbud.

    \return bool TRUE peer is connected, FALSE peer is not connected.
*/
bool appDeviceIsPeerAvrcpConnected(void);

/*! \brief Determine if a connected with AVRCP to peer earbud for AV usage.

    \return bool TRUE peer is connected, FALSE peer is not connected.
*/
bool appDeviceIsPeerAvrcpConnectedForAv(void);

/*! \brief Determine if SCOFWD is connected to peer earbud.

    \return bool TRUE SCOFWD to peer is connected, FALSE otherwise.
*/
bool appDeviceIsPeerScoFwdConnected(void);

/*! \brief Set flag for handset device indicating if address needs to be sent to peer earbud. 

    \param handset_bd_addr BT address of handset device.
    \param reqd  TRUE Flag is set, link key is required to be sent to peer earbud.
                 FALSE Flag is clear, link key does not need to be sent to peer earbud.  
    \return bool TRUE Success, FALSE failure device not known.
*/
bool appDeviceSetHandsetAddressForwardReq(bdaddr *handset_bd_addr, bool reqd);

/*! \brief For a handset device attributes determine if address needs to be
           sent to peer earbud.
           
    \param attr  Handset attributes to check.  
    \return bool TRUE if address needs to be forward, FALSE otherwise
*/
bool appDeviceHandsetAttrIsAddressForwardReqd(appDeviceAttributes* attr);

/*! \brief For handset device address determine if address needs to be sent to
           peer earbud. 
           
    \param handset_bd_addr BT address of handset device.
    \param reqd  A pointer to unused bool  
    \return bool TRUE if address needs to be forward, FALSE if it doesn't.          
*/
bool appDeviceGetHandsetAddressForwardReqd(bdaddr *handset_bd_addr, bool *reqd);

/*! \brief Determine if a BT address is for a TWS+ handset.

    \param handset_bd_addr Pointer to read-only handset BT address.
    \return bool TRUE address is for TWS+ handset, FALSE either not a handset or not TWS+ handset.
*/
bool appDeviceIsTwsPlusHandset(const bdaddr *handset_bd_addr);

/*! \brief Determine if a device has just paired.

    \param bd_addr Pointer to read-only BT device address.
    \return bool TRUE address is just paired device, FALSE not just paired.
*/
bool appDeviceHasJustPaired(const bdaddr *bd_addr);

/*! \brief Delete device from pair device list and cache.

    \param bd_addr Pointer to read-only device BT address to delete.
    \return bool TRUE if device deleted, FALSE if device not delete due to being connected.
*/
bool appDeviceDelete(const bdaddr *bd_addr);

/*! \brief Determine if there is any profile (A2DP,AVRCP,HFP) connected to a handset.

    \return bool TRUE if any profile is connected, FALSE no profiles are connected. 
 */
bool appDeviceIsHandsetAnyProfileConnected(void);

/*! \brief Register a client task to receive device version updates.

    \param[in] client_task Task which will receive CON_MANAGER_DEVICE_TYPE_IND message
 */
extern void appDeviceRegisterDeviceVersionClient(Task client_task);

/*! \brief Determine if device is PTS tester.

    \param bd_addr Pointer to read-only device BT address to check.
    \return bool TRUE if device is PTES tester, FALSE if it isn't.
*/
bool appDeviceIsPts(const bdaddr *bd_addr);


/*! \brief Set SCO forwarding supported features

    \param bd_addr Pointer to read-only device BT address to set featured for.
    \param supported_features Support featured for device.
*/
void appDeviceSetScoForwardFeatures(const bdaddr *bd_addr, uint16 supported_features);

/*! \brief Determine if MIC forwarding with peer is supported.

    \return bool TRUE if MIC forwarding is supported.
*/
bool appDeviceIsPeerMicForwardSupported(void);


/*! \brief Determine if out of band RING forwarding with peer is supported.

    \return bool TRUE if out of band RING forwarding is supported.
*/
bool appDeviceIsPeerRingForwardSupported(void);

/*! \brief Update the most recently used device in the PDL and update cache.
    \param bd_addr Pointer to the device BT address most recently used.
*/
void appDeviceUpdateMruDevice(const bdaddr *bd_addr);


/*! Store whether the requested device has BLE pairing.

    \param bd_addr      Pointer to the BT address of the requested device
    \param ble_random_resolvable_paired Flag indicating if BLE resolvable pairing supplied.
*/
bool appDeviceSetBlePairing(const bdaddr *bd_addr, bool ble_random_resolvable_paired);

/*! Find whether the requested device has BLE pairing.

    \param bd_addr      Pointer to the BT address of the requested device

    \returns TRUE if the device was found and has been paired over BLE, using a Random Resolvable
        Address (RRA).  FALSE if the device could not be found, or is not paired over BLE.
*/
bool appDeviceHasBlePairing(const bdaddr *bd_addr);

#endif
