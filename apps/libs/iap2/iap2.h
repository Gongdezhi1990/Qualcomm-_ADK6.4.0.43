/*****************************************************************************
Copyright (c) 2011 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    iap2.h

DESCRIPTION
    Interface to the iAP2 library
*/

/*!
    \file   iap2.h
    \brief  Interface to the iAP2 library

    This library implements the iAP2 protocol over Bluetooth and USB Host transports.
*/

#ifndef __IAP2_H__
#define __IAP2_H__

#include <bdaddr_.h>
#include <library.h>
#include <message_.h>
#include <sink.h>
#include <string.h>

#ifndef IAP_MESSAGE_BASE
#define IAP_MESSAGE_BASE 0x7A00
#endif

/*!
    \brief The maximum number of un-ACKed packets in the TX queue.
*/
#define IAP2_MAX_TX_QUEUE_SIZE  (8)

/*!
    \brief iAP2 link instance
*/
typedef struct _iap2_link iap2_link;

/*!
    \brief iAP2 library status codes.
*/
typedef enum
{
    /*! Success */
    iap2_status_success,
    /*! Fail */
    iap2_status_fail,
    /*! Coprocessor access fail */
    iap2_status_coprocessor_fail,
    /*! SDP registration fail*/
    iap2_status_sdp_reg_fail,
    /*! RFCOMM channel registration fail */
    iap2_status_rfcomm_reg_fail,
    /*! Authentication failed during link establishment */
    iap2_status_authentication_fail,
    /*! Out of resources */
    iap2_status_out_of_resources,
    /*! Connection terminated by remote */
    iap2_status_remote_terminated,
    /*! Connection link loss */
    iap2_status_link_loss,
    /*! Link does not support iAP2 */
    iap2_status_incompatible,
    /*! iAP2 link not found */
    iap2_status_link_not_found,
    /*! iAP2 link already existed */
    iap2_status_link_already_existed,
    /*! Timeout when trying to connect */
    iap2_status_timeout,
    /*! Security failed when trying to connect */
    iap2_status_security_fail,
    /*! A parameter is invalid */
    iap2_status_invalid_param,
    /*! A packet exceeds MaxNumRetransmissions */
    iap2_status_max_retransmission,
    /*! The packets in queue exceeds Maximum Number of Outstanding Packets */
    iap2_status_queue_full,
    /*! The iAP2 link is not ready to send messages */
    iap2_status_link_not_ready,

    FORCE_ENUM_TO_MIN_16BIT(iap2_status_t)
} iap2_status_t;

/*!
    \brief Messages sent from iAP2 library to the application.
 */
typedef enum
{
    /*! iAP2 library is initialized */
    IAP2_INIT_CFM = IAP_MESSAGE_BASE,
    /*! A remote device is trying to connect */
    IAP2_CONNECT_IND,
    /*! An iAP2 link is established */
    IAP2_CONNECT_CFM,
    /*! An iAP2 link is terminated by a remote device or disconnected unexpectedly */
    IAP2_DISCONNECT_IND,
    /*! An iAP2 link disconnection which is initiated locally was completed */
    IAP2_DISCONNECT_CFM,
    /*! Confirmation of accessory identification */
    IAP2_IDENTIFICATION_CFM,
    /*! Confirmation of iAP2 link detach request */
    IAP2_LINK_DETACH_CFM,
    /*! Remote host has reset the link */
    IAP2_LINK_RESET_IND,
    /*! Remote host has switched the link into sleep state */
    IAP2_LINK_SLEEP_IND,
    /*! An iAP2 control message is received */
    IAP2_CONTROL_MESSAGE_IND,
    /*! An iAP2 EA session is started */
    IAP2_EA_SESSION_START_IND,
    /*! An iAP2 EA session is stopped */
    IAP2_EA_SESSION_STOP_IND,
    /*! An iAP2 EA data message is received */
    IAP2_EA_SESSION_DATA_IND,
    /*! Confirmation of iAP2 EA protocol handler registration */
    IAP2_EA_REGISTER_HANDLER_CFM,
    /*! Confirmation of iAP2 EA protocol handler unregistration */
    IAP2_EA_UNREGISTER_HANDLER_CFM,
    IAP2_MESSAGE_TOP
} Iap2MessageId;

/*!
    \brief Supported iAP2 transports.
*/
typedef enum
{
    /*! iAP2 over Bluetooth */
    iap2_transport_bluetooth,
    /*! iAP2 over USB Host Mode */
    iap2_transport_usb_host,
    /*! Invalid transport */
    iap2_transport_invalid
} iap2_transport_t;

/*!
    \brief iAP2 accessory power providing capabilities.
*/
typedef enum
{
    /*! Accessory does not provide power to the Apple device */
    iap2_power_capability_none      = 0x0,
    /*! Accessory provides power to or draws power from the Apple device */
    iap2_power_capability_advanced  = 0x2
} iap2_power_capability_t;

/*!
    \brief iAP2 accessory App Match actions.
*/
typedef enum
{
    /*! iOS will not prompt the user to find a matching app, and there will not be a Find App For This Accessory button in Settings > General > About > 'Accessory Name'. */
    iap2_app_match_no_action,
    /*! iOS may prompt the user to find a matching app, and there will be a Find App For This Accessory button in Settings > General > About > 'Accessory Name'. */
    iap2_app_match_optional,
    /*! iOS will not prompt the user to find a matching app, but there will be a Find App For This Accessory button in Settings > General > About > 'Accessory Name'. */
    iap2_app_match_no_alert,
    /*! Match Action does not relate to EA sessions. iOS may prompt the user to find a matching app, and there will be a Find App For This Accessory button in Settings -> General -> About -> 'Accessory Name'. */
    iap2_app_match_no_protocol
} iap2_app_match_action_t;

/*!
    \brief iAP2 accessory HID component functions.
*/
typedef enum
{
    /*! HID component is a Keyboard */
    iap2_hid_component_func_keyboard            = 0,
    /*! HID component is a Media Playback Remote */
    iap2_hid_component_func_media_remote        = 1,
    /*! HID component is an AssistiveTouch Pointer */
    iap2_hid_component_func_assistive_touch     = 2,
    /*! HID component is a Form-Fitting Gamepad */
    iap2_hid_component_func_gamepad_ff          = 4,
    /*! HID component is a Non Form-Fitting Gamepad */
    iap2_hid_component_func_gamepad_nff         = 6,
    /*! HID component is an Assistive Switch Control */
    iap2_hid_component_func_assistive_switch    = 7,
    /*! HID component is a Headset */
    iap2_hid_component_func_headset             = 8
} iap2_hid_component_func_t;

/*!
    \brief iAP2 control session message ID.
*/
typedef enum
{
    /* 65.1 Accessory Authentification */
    RequestAuthenticationCertificate                = (int)0xAA00,
    AuthenticationCertificate                       = (int)0xAA01,
    RequestAuthenticationChallengeResponse          = (int)0xAA02,
    AuthenticationResponse                          = (int)0xAA03,
    AuthenticationFailed                            = (int)0xAA04,
    AuthenticationSucceeded                         = (int)0xAA05,
    AcessoryAuthenticationSerialNumber              = (int)0xAA06,

    /* 65.2 Accessory Identification */
    StartIdentification                             = (int)0x1D00,
    IdentificationInformation                       = (int)0x1D01,
    IdentificationAccepted                          = (int)0x1D02,
    IdentificationRejected                          = (int)0x1D03,
    CancelIdentification                            = (int)0x1D05,
    IdentificationInformationUpdate                 = (int)0x1D06,

    /* 65.3 App Launch */
    RequestAppLaunch                                = (int)0xEA02,

    /* 65.4 Assistive Touch */
    StartAssistiveTouch                             = (int)0x5400,
    StopAssistiveTouch                              = (int)0x5401,
    StartAssistiveTouchInformation                  = (int)0x5402,
    AssistiveTouchInformation                       = (int)0x5403,
    StopAssistiveTouchInformation                   = (int)0x5404,

    /* 65.5 Bluetooth Connection */
    BluetoothComponentInformation                   = (int)0x4E01,
    StartBluetoothConnectionUpdates                 = (int)0x4E03,
    BluetoothConnectionUpdate                       = (int)0x4E04,
    StopBluetoothConnectionUpdates                  = (int)0x4E05,

    /* 65.6 Communications */
    StartCallStateUpdates                           = (int)0x4154,
    CallStateUpdate                                 = (int)0x4155,
    StopCallStateUpdates                            = (int)0x4156,
    StartCommunicationUpdates                       = (int)0x4157,
    CommunicationsUpdate                            = (int)0x4158,
    StopCommunicationsUpdates                       = (int)0x4159,
    InitiateCall                                    = (int)0x415A,
    AcceptCall                                      = (int)0x415B,
    EndCall                                         = (int)0x415C,
    SwapCalls                                       = (int)0x415D,
    MergeCalls                                      = (int)0x415E,
    HoldStatusUpdate                                = (int)0x415F,
    MuteStatusUpdate                                = (int)0x4160,
    SendDTMF                                        = (int)0x4161,
    StartListUpdates                                = (int)0x4170,
    ListUpdate                                      = (int)0x4171,
    StopListUpdates                                 = (int)0x4172,

    /* 65.7 Device Authentication  */
    RequestDeviceAuthenticationCertificate          = (int)0xAA10,
    DeviceAuthenticationCertificate                 = (int)0xAA11,
    RequestDeviceAuthenticationChallengeResponse    = (int)0xAA12,
    DeviceAuthenticationResponse                    = (int)0xAA13,
    DeviceAuthenticationFailed                      = (int)0xAA14,
    DeviceAuthenticationSucceeded                   = (int)0xAA15,

    /* 65.8 Device Notifications  */
    DeviceInformationUpdate                         = (int)0x4E09,
    DeviceLanguageUpdate                            = (int)0x4E0A,
    DeviceTimeUpdate                                = (int)0x4E0B,
    DeviceUUIDUpdate                                = (int)0x4E0C,
    WirelessCarPlayUpdate                           = (int)0x4E0D,

   /* 65.9 External Accessory Protocol */
    StartExternalAccessoryProtocolSession           = (int)0xEA00,
    StopExternalAccessoryProtocolSession            = (int)0xEA01,
    StatusExternalAccessoryProtocolSession          = (int)0xEA03,

    /* 65.10 Human Interface Device */
    StartHID                                        = (int)0x6800,
    DeviceHIDReport                                 = (int)0x6801,
    AccessoryHIDReport                              = (int)0x6802,
    StopHID                                         = (int)0x6803,
    StartNativeHID                                  = (int)0x6806,

    /* 65.11 Location */
    StartLocationInformation                        = (int)0xFFFA,
    GPRMCDataStatusValuesNotification               = (int)0xFFF0,
    LocationInformation                             = (int)0xFFFB,
    StopLocationInformation                         = (int)0xFFFC,

    /* 65.12 Media Library Access */
    StartMediaLibraryInformation                    = (int)0x4C00,
    MediaLibraryInformation                         = (int)0x4C01,
    StopMediaLibraryInformation                     = (int)0x4C02,
    StartMediaLibraryUpdates                        = (int)0x4C03,
    MediaLibraryUpdate                              = (int)0x4C04,
    StopMediaLibraryUpdates                         = (int)0x4C05,
    PlayMediaLibraryCurrentSelection                = (int)0x4C06,
    PlayMediaLibraryItems                           = (int)0x4C07,
    PlayMediaLibraryCollection                      = (int)0x4C08,
    PlayMediaLibrarySpecial                         = (int)0x4C09,

    /* 65.13 Now Playing Updates */
    StartNowPlayingUpdates                          = (int)0x5000,
    NowPlayingUpdate                                = (int)0x5001,
    StopNowPlayingUpdates                           = (int)0x5002,
    SetNowPlayingInformation                        = (int)0x5003,

    /* 65.14 Out-of-Band Bluetooth Pairing */
    StartOOBBTPairing                               = (int)0x00B0,
    OOBBTPairingAccessoryInformation                = (int)0x00B1,
    OOBBTPairingLinkKeyInformation                  = (int)0x00B2,
    OOBBTPairingCompletionInformation               = (int)0x00B3,
    StopOOBBTPairing                                = (int)0x00B4,

    /* 65.15 Power  */
    StartPowerUpdates                               = (int)0xAE00,
    PowerUpdate                                     = (int)0xAE01,
    StopPowerUpdates                                = (int)0xAE02,
    PowerSourceUpdate                               = (int)0xAE03,

    /* 65.16 USB Device mode audio  */
    StartUSBDeviceModeAudio                         = (int)0xDA00,
    USBDeviceModeAudioInformation                   = (int)0xDA01,
    StopUSBDeviceModeAudio                          = (int)0xDA02,

    /* 65.17 Vehicle Status */
    StartVehicleStatusUpdates                       = (int)0xA100,
    VehicleStatusUpdate                             = (int)0xA101,
    StopVehicleStatusUpdates                        = (int)0xA102,

    /* 65.18 VoiceOver  */
    StartVoiceOver                                  = (int)0x5612,
    StopVoiceOver                                   = (int)0x5613,
    RequestVoiceOverMoveCursor                      = (int)0x5601,
    RequestVoiceOverActivateCursor                  = (int)0x5602,
    RequestVoiceOverScrollPage                      = (int)0x5603,
    RequestVoiceOverSpeakText                       = (int)0x5606,
    RequestVoiceOverPauseText                       = (int)0x5608,
    RequestVoiceOverResumeText                      = (int)0x5609,
    StartVoiceOverUpdates                           = (int)0x560B,
    VoiceOverUpdate                                 = (int)0x560C,
    StopVoiceOverUpdates                            = (int)0x560D,
    RequestVoiceOverConfiguration                   = (int)0x560E,
    StartVoiceOverCursorUpdates                     = (int)0x560F,
    VoiceOverCursorUpdate                           = (int)0x5610,
    StopVoiceOverCursorUpdates                      = (int)0x5611,

    /* 65.19 Wifi Information Sharing */
    RequestWiFiInformation                          = (int)0x5700,
    WiFiInformation                                 = (int)0x5701,
    RequestAccessoryWiFiConfigurationInformation    = (int)0x5702,
    AccessoryWiFiConfigurationInformation           = (int)0x5703
} iap2_control_message;

/*!
    \brief iAP2 library initialisation confirmation.
*/
typedef struct
{
    /*! The iAP2 library initialisation status. */
    iap2_status_t status;
} IAP2_INIT_CFM_T;

/*!
    \brief iAP2 link connection indication.

    This message is received when a remote device tries to make a iAP2 connection.
    Iap2ConnectResponse() function must be called to accept or decline the connection.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The Bluetooth address of the remote device. */
    bdaddr bd_addr;
} IAP2_CONNECT_IND_T;

/*!
    \brief iAP2 link connection confirmation.

    This message is received when an iAP2 link has been setup or it has failed.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The Bluetooth address of the remote device. */
    bdaddr bd_addr;
    /*! The iAP2 link connection status.*/
    iap2_status_t status;
    /*! The iAP2 RFCOMM Sink */
    Sink sink;
} IAP2_CONNECT_CFM_T;

/*!
    \brief iAP2 link disconnection indication.

    This message is received when an iAP2 link has been disconnected from the remote devices or unexpectedly.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The Bluetooth address of the remote device. */
    bdaddr bd_addr;
    /*! The iAP2 link disconnection status.*/
    iap2_status_t status;
} IAP2_DISCONNECT_IND_T;

/*!
    \brief iAP2 link disconnection confirmation.

    This message is received when an iAP2 link has been disconnected locally.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The Bluetooth address of the remote device. */
    bdaddr bd_addr;
    /*! The iAP2 link disconnection status.*/
    iap2_status_t status;
} IAP2_DISCONNECT_CFM_T;

/*!
    \brief iAP2 accessory identification confirmation.

    This message is received when the accessory identification is completed or has failed.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The iAP2 accessory identification status. */
    iap2_status_t status;
    /*! The bit mask of failed message IDs if the accessory identification has been rejected. */
    uint32 error_mask;
} IAP2_IDENTIFICATION_CFM_T;

/*!
    \brief iAP2 link detach confirmation.

    This message is received when an iAP2 link is detached and available for Kalimba.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The iAP2 link detach status. */
    iap2_status_t status;
    /*! The RFCOMM Sink for this iAP2 link */
    Sink sink;
    /*! The saved iAP2 link data size */
    uint16 size_link_data;
    /*! The saved iAP2 link data to be sent to Kalimba */
    uint16 link_data[1];
} IAP2_LINK_DETACH_CFM_T;

/*!
    \brief iAP2 link reset indication.

    This message is received when an iAP2 link is reset by the remote host.
    Application must call Iap2AuthenticationRequest to re-authenticate the accessory.
*/
typedef struct
{
    iap2_link *link;
} IAP2_LINK_RESET_IND_T;

/*!
    \brief iAP2 link sleep indication.

    This message is received when an iAP2 link is entering sleep state.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
} IAP2_LINK_SLEEP_IND_T;

/*!
    \brief iAP2 control message indication.

    This message is received when an iAP2 control message is received.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The control message ID from #iap2_control_message */
    iap2_control_message message_id;
    /*! Message payload size */
    uint16 size_payload;
    /*! Message payload */
    uint8 payload[1];
} IAP2_CONTROL_MESSAGE_IND_T;

/*!
    \brief iAP2 EA session start indication.

    This message is received when an External Accessory session is started.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The EA protocol ID */
    uint8 protocol_id;
    /*! The EA session ID */
    uint16 session_id;
} IAP2_EA_SESSION_START_IND_T;

/*!
    \brief iAP2 EA session stop indication.

    This message is received when an External Accessory session is stopped.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The EA protocol ID */
    uint8 protocol_id;
    /*! The EA session ID */
    uint16 session_id;
} IAP2_EA_SESSION_STOP_IND_T;

/*!
    \brief iAP2 EA session data indication.

    This message is received when an External Accessory data message is received.
*/
typedef struct
{
    /*! The iAP2 link instance. */
    iap2_link *link;
    /*! The EA Protocol identifier */
    uint8 protocol_id;
    /*! The EA session ID */
    uint16 session_id;
    /*! Data message payload size */
    uint16 size_payload;
    /*! Data message payload */
    uint8 payload[1];
} IAP2_EA_SESSION_DATA_IND_T;

/*!
    \brief iAP2 EA Protocol handler indication.

    This message is received when a task is registered to handle an
    External Accessory Protocol.
*/
typedef struct
{
    /*! The EA Protocol identifier */
    uint8 protocol_id;
    /*! The registration status. */
    iap2_status_t status;
} IAP2_EA_REGISTER_HANDLER_CFM_T;

/*!
    \brief iAP2 EA Protocol handler indication.

    This message is received when a task is unregistered from handling
    an External Accessory Protocol.
*/
typedef struct
{
    /*! The EA Protocol identifier */
    uint8 protocol_id;
    /*! The unregistration status. */
    iap2_status_t status;
} IAP2_EA_UNREGISTER_HANDLER_CFM_T;



/*!
    \brief The preferred iAP2 link parameters.
*/
typedef struct
{
    /*! Maximum Number of Outstanding Packets (1 to 127). */
    uint8 MaxNumOutstandingPackets;
    /*! Maximum Received Packet Length (24 to 65535). Note that there is a 12-byte header for EA data packets. */
    uint16 MaxRxPacketLength;
    /*! Retransmission Timeout (20 to 65535). */
    uint16 RetransmissionTimeout;
    /*! Cumulative Acknowledgement Timeout (10 to half of the RetransmissionTimeout). */
    uint16 CumulativeAckTimeout;
    /*! Maximum Number of Retransmissions (1 to 30). */
    unsigned MaxNumRetransmissions:5;
    /*! Maximum Cumulative Acknowledgements (0 to 127 or MaxNumOutstandingPackets). */
    unsigned MaxCumulativeAcks:8;
    /*! If the accessory presents File Transfer Session */
    unsigned hasFileTransferSession:1;
    /*! If the accessory presents External Accessory Session */
    unsigned hasExternalAccessorySession:1;
    /*! If the accessory supports ZeroACK/ZeroRetransmit link */
    unsigned supportZeroAckLink:1;
} iap2_link_param;

/*!
    \brief The iAP2 ExternalAccessoryProtocol parameters.
*/
typedef struct
{
    /*! The unique ExternalAccessoryProtocol identifier. */
    uint8 proto_id;
    /*! The unique protocol name, usually reverse DNS format. */
    const char *name;
    /*! The App Match action. */
    iap2_app_match_action_t match_action;
} iap2_ea_protocol;

/*!
    \brief The iAP2 HID component parameters.
*/
typedef struct
{
    /*! The unique iAP2 HIDComponentIdentifier. */
    uint16 component_id;
    /*! The name for this component. */
    const char *name;
    /*! The HIDComponentFunction. */
    iap2_hid_component_func_t function;
} iap2_hid_component;


/*!
    \brief iAP2 library configuration structure.
*/
typedef struct
{
    /*! NULL terminated local device name string. */
    const char *device_name;
    /*! Local Bluetooth address */
    bdaddr local_bdaddr;
    /*! NULL terminated ModelIdentifier string. */
    const char *model_id;
    /*! NULL terminated Manufacturer string. */
    const char *manufacturer;
    /*! NULL terminated SerialNumber string. */
    const char *serial_number;
    /*! NULL terminated FirmwareVersion string. */
    const char *firmware_version;
    /*! NULL terminated HardwareVersion string. */
    const char *hardware_version;
    /*! Optional NULL terminated App Match Team ID string. */
    const char *app_match_team_id;
    /*! NULL terminated CurrentLanguage string. */
    const char *current_language;
    /*! The PowerProvidingCapability of the accessory. */
    iap2_power_capability_t power_capability;
    /*! The MaximumCurrentDrawnFromDevice parameter. */
    uint16 max_current_draw;
    /*! The number of messages sent by accessory */
    uint16 messages_tx_count;
    /*! The array of messages sent by accessory */
    iap2_control_message *messages_tx;
    /*! The number of messages received by accessory */
    uint16 messages_rx_count;
    /*! The array of messages received by accessory */
    iap2_control_message *messages_rx;
    /*! The number of iAP2HIDComponents, may be 0. */
    uint16 iap2_hid_component_count;
    /*! Optional array of iAP2HIDComponents. */
    const iap2_hid_component *iap2_hid_components;
    /*! Optional BluetoothHIDComponent, can have at most 1. */
    const iap2_hid_component *bt_hid_component;
} iap2_config;

/*!
    \brief The task handler function to receive iAP2 library messages.

    All iAP2 library messages defined in in the #Iap2MessageId will be received by this handler.
    The application must implement this function to receive the messages.

    \param task The task.
    \param id The message ID defined in #Iap2MessageId.
    \param message The message structure.
*/
void iap2MessageHandler(Task task, MessageId id, Message message);

/*!
    \brief This function is called to initialise the iAP2 library.

    An #IAP2_INIT_CFM message will be received indicating if initialising the library is successful.

    \param task The client task.
    \param config Pointer to iAP2 configuration structure.
*/
void Iap2Init(Task task, iap2_config *config);

/*!
    \brief This function is called to initiate a new iAP2 connection.

    An #IAP2_CONNECT_CFM message will be received indicating if the connection is successful.

    \param bd_addr Bluetooth address of device to connect to.
    \param timeout The time in milliseconds the connection attempt will timeout.
    \param param Preferred link parameters.
*/
void Iap2ConnectRequest(bdaddr *bd_addr, uint16 timeout, const iap2_link_param *param);


/*!
    \brief This function is called to register an External Accessory Protocol with the iAP2 library.

    \param name The protocol name in Reverse DNS notation.
    \param match_action iAP2 accessory App Match action.
    \return Protocol ID for the named protocol, 0 on error.
*/
uint8 Iap2EARegisterProtocol(const char * const name, iap2_app_match_action_t match_action);

/*!
    \brief This function is called to request the device to launch a named application.

    \param link The iAP2 link instance, as indicated in the #IAP2_CONNECT_IND_T message.
    \param app_name The application name in Reverse DNS notation.
 */
void Iap2AppLaunchReq(iap2_link *link, char *app_name);

/*!
    \brief This function is called to cancel an iAP2 connection request.

    The iAP2 connection request associated with this Bluetooth address will be canceled.

    \param bd_addr Bluetooth address of device to cancel the connection request.
*/
void Iap2ConnectCancelRequest(bdaddr *bd_addr);

/*!
    \brief This function is called to respond to an iAP2 connection indication.

    Respond to a #IAP2_CONNECT_IND from a remote device trying to create an iAP2 connection.

    \param link The iAP2 link instance, as indicated in the #IAP2_CONNECT_IND_T message.
    \param accept #TRUE to accept the connection, #FALSE to reject it.
    \param param Preferred link parameters.
 */
void Iap2ConnectResponse(iap2_link *link, bool accept, const iap2_link_param *param);

/*!
    \brief This function is called to disconnect an iAP2 connection.

    An #IAP2_DISCONNECT_CFM message will be received indicating the result of the disconnection attempt.

    \param link The iAP2 link instance of the iAP2 connection to disconnect.
 */
void Iap2DisconnectRequest(iap2_link *link);

/*!
    \brief This function is called to perform iAP2 Accessory Identification.

    An #IAP2_IDENTIFICATION_CFM message will be received indicating if the accessory identification is successful.
    This function must be called immediately upon receiving the #IAP2_CONNECT_CFM message when the link connection is successful.
    If the identification information is rejected, this function may be called with different identification information to re-identify.

    \param link The iAP2 link instance.
*/
void Iap2IdentificationRequest(iap2_link *link);

/*!
    \brief This function is called to send an iAP2 control session message.

    If the message is queued successfully, the malloced \e payload will be handled and freed by the library.
    Otherwise, the application must deallocate the memory after use.

    \param link The iAP2 link instance.
    \param message_id The iAP2 control session message ID.
    \param size_payload Length of the message payload.
    \param payload The message payload.
    \param static_payload #TRUE if the payload uses a static buffer, otherwise the buffer must be malloced.
    \return #iap2_status_success if the message is queued successfully, otherwire error.
*/
iap2_status_t Iap2ControlMessage(iap2_link *link,
                                      iap2_control_message message_id,
                                      uint16 size_payload,
                                      const uint8 *payload,
                                      bool static_payload);

/*!
    \brief This function is called to send an iAP2 External Accessory session message.

    If the message is queued successfully, the malloced \e payload will be handled and freed by the library.
    Otherwise, the application must deallocate the memory after use.

    \param link The iAP2 link instance.
    \param ea_session_id The iAP2 External Accessory session ID.
    \param size_payload Length of the message payload.
    \param payload The message payload.
    \param static_payload #TRUE if the payload uses a static buffer, otherwise the buffer must be malloced.
    \return #iap2_status_success if the message is queued successfully, otherwire error.
*/
iap2_status_t Iap2EAMessage(iap2_link *link,
                                uint16 session_id,
                                uint16 size_payload,
                                const uint8 *payload,
                                bool static_payload);

/*!
    \brief This function is called to register a task to handle External Accessory Protocol messages.

    \param task The task to receive EAP messages.
    \param protocol_id the EAP Identifier which this task will handle.
*/
void Iap2EARegisterHandler(Task task, uint8 protocol_id);

/*!
    \brief This function is called to unregister a task handling External Accessory Protocol messages.

    \param task The task receiving EAP messages.
    \param protocol_id the EAP Identifier.
*/
void Iap2EAUnregisterHandler(Task task, uint8 protocol_id);

/*!
    \brief Get the existing iAP2 link instance associated with a remote device Bluetooth address.

    \param remote_bdaddr The Bluetooth address of a remote device.
    \return The iAP2 link instance if success, otherwise NULL.
*/
iap2_link *Iap2GetLinkForBdaddr(bdaddr *remote_bdaddr);

/*!
    \brief Get the remote device Bluetooth address associated with an existing iAP2 link.

    \param link The iAP2 link instance.
    \param remote_bdaddr The pointer to a bdaddr structure that the remote device address will be stored.
    \return #iap2_status_success if the remote address is retrived successfully, otherwise error.
*/
iap2_status_t Iap2GetBdaddrForLink(iap2_link *link, bdaddr *remote_bdaddr);

/*!
    \brief Get how many bytes can be sent over an iAP2 link, accounting for protocol overhead.

    \param link The iAP2 link instance.
    \return Number of bytes avaliable, 0 on error (e.g. there is no valid sink).
*/
uint16 Iap2GetSlackForLink(iap2_link *link);

/*!
    \brief Read the uint8 iAP2 control parameter data from control session message payload.

    \param payload The iAP2 control session message payload to read the parameter data from.
    \param size_payload The size of \e payload.
    \param param_id The parameter ID.
    \param value The buffer to hold parameter data. NULL to check the parameter existence.
    \return TRUE on success, FALSE if the parameter not found or payload malformed.
*/
bool Iap2CtrlParamReadUint8(uint8 *payload, uint16 size_payload, uint16 param_id, uint8 *value);

/*!
    \brief Read the uint16 iAP2 control parameter data from control session message payload.

    \param payload The iAP2 control session message payload to read the parameter data from.
    \param size_payload The size of \e payload.
    \param param_id The parameter ID.
    \param value The buffer to hold parameter data. NULL to check the parameter existence.
    \return TRUE on success, FALSE if the parameter not found or payload malformed.
*/
bool Iap2CtrlParamReadUint16(uint8 *payload, uint16 size_payload, uint16 param_id, uint16 *value);

/*!
    \brief Read the uint32 iAP2 control parameter data from control session message payload.

    \param payload The iAP2 control session message payload to read the parameter data from.
    \param size_payload The size of \e payload.
    \param param_id The parameter ID.
    \param value The buffer to hold parameter data. NULL to check the parameter existence.
    \return TRUE on success, FALSE if the parameter not found or payload malformed.
*/
bool Iap2CtrlParamReadUint32(uint8 *payload, uint16 size_payload, uint16 param_id, uint32 *value);

/*!
    \brief Read the enumeration iAP2 control parameter data from control session message payload.

    \param payload The iAP2 control session message payload to read the parameter data from.
    \param size_payload The size of \e payload.
    \param param_id The parameter ID.
    \param value The buffer to hold parameter data. NULL to check the parameter existence.
    \return TRUE on success, FALSE if the parameter not found or payload malformed.
*/
#define Iap2CtrlParamReadEnum(payload, size_payload, param_id, value) Iap2CtrlParamReadUint8(payload, size_payload, param_id, (uint8)value)

/*!
    \brief Read the boolean iAP2 control parameter data from control session message payload.

    \param payload The iAP2 control session message payload to read the parameter data from.
    \param size_payload The size of \e payload.
    \param param_id The parameter ID.
    \param value The buffer to hold parameter data. NULL to check the parameter existence.
    \return TRUE on success, FALSE if the parameter not found or payload malformed.
*/
#define Iap2CtrlParamReadBoolean(payload, size_payload, param_id, value) Iap2CtrlParamReadUint8(payload, size_payload, param_id, value)

/*!
    \brief Get a read address and size for the iAP2 control parameter data from control session message payload.

    \param payload The iAP2 control session message payload to read the parameter data from.
    \param size_payload The size of \e payload.
    \param param_id The parameter ID.
    \param read_size The size of the parameter data.
    \param read_ptr The pointer to the parameter data.
    \return TRUE on success, FALSE if the parameter not found or payload malformed.
*/
bool Iap2CtrlParamGetReadAddress(uint8 *payload, uint16 size_payload, uint16 param_id, uint16 *read_size, uint8 **read_ptr);

/*!
    \brief Verify if a parameter is presented in the control session message payload.

    \param payload The iAP2 control session message payload to read the parameter data from.
    \param size_payload The size of \e payload.
    \param param_id The parameter ID.
    \return TRUE on success, FALSE if the parameter not found or payload malformed.
*/
#define Iap2CtrlParamVerify(payload, size_payload, param_id) Iap2CtrlParamGetReadAddress(payload, size_payload, param_id, NULL, NULL)

/*!
    \brief Write the iAP2 control session message parameter structure with uint8 data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
uint16 Iap2CtrlParamWriteUint8(uint8 *buffer, uint16 param_id, uint8 value);

/*!
    \brief Write the iAP2 control session message parameter structure with uint16 data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
uint16 Iap2CtrlParamWriteUint16(uint8 *buffer, uint16 param_id, uint16 value);

/*!
    \brief Write the iAP2 control session message parameter structure with uint32 data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
uint16 Iap2CtrlParamWriteUint32(uint8 *buffer, uint16 param_id, uint32 value);

/*!
    \brief Write the iAP2 control session message parameter structure with NULL terminated UTF-8 string to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param string The NULL terminated UTF-8 string.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
uint16 Iap2CtrlParamWriteString(uint8 *buffer, uint16 param_id, const char *string);

/*!
    \brief Write the iAP2 control session message parameter structure with blob data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param data The pointer to the parameter data.
    \param size_data The data size.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
uint16 Iap2CtrlParamWriteBlob(uint8 *buffer, uint16 param_id, uint8 *data, uint16 size_data);

/*!
    \brief Send data to the EA session associated with the given transport.

    \param transport_id The EA transport identifier.
    \param data Pointer to the data to send.
    \param size_data The data size.
    \return TRUE if the data was successfully queued for transmission.
*/
bool Iap2EASendData(uint16 transport_id, const uint8 *data, uint16 size_data);

/*!
    \brief Get how many bytes can be sent to the EA session associated with the given transport.
    \param transport_id The EA transport identifier.
    \return Number of bytes avaliable, 0 on error (e.g. there is no associated link).
*/
uint16 Iap2EAGetSlackForProtocol(uint8 protocol_id);

/*!
    \brief Write the iAP2 control session message parameter structure with int8 data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
#define Iap2CtrlParamWriteInt8(buffer, param_id, value) Iap2CtrlParamWriteUint8(buffer, param_id, (uint8)value)

/*!
    \brief Write the iAP2 control session message parameter structure with int16 data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
#define Iap2CtrlParamWriteInt16(buffer, param_id, value) Iap2CtrlParamWriteUint16(buffer, param_id, (uint16)value)

/*!
    \brief Write the iAP2 control session message parameter structure with int32 data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
#define Iap2CtrlParamWriteInt32(buffer, param_id, value) Iap2CtrlParamWriteUint32(buffer, param_id, (uint32)value)

/*!
    \brief Write the iAP2 control session message parameter structure with enumeration data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
#define Iap2CtrlParamWriteEnum(buffer, param_id, value) Iap2CtrlParamWriteUint8(buffer, param_id, (uint8)value)

/*!
    \brief Write the iAP2 control session message parameter structure with boolean data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param value The parameter data.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
#define Iap2CtrlParamWriteBoolean(buffer, param_id, value) Iap2CtrlParamWriteUint8(buffer, param_id, (uint8)value)

/*!
    \brief Write the iAP2 control session message parameter structure without payload data to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
#define Iap2CtrlParamWriteNone(buffer, param_id) Iap2CtrlParamWriteBlob(buffer, param_id, NULL, 0)

/*!
    \brief Write the iAP2 control session message parameter header with payload size to the memory buffer.

    \param buffer The memory buffer to write the parameter structure to.
    \param param_id The parameter ID.
    \param size_payload The parameter payload data size.
    \return If \e buffer is NULL, returns the minimum length of the buffer necessary to hold the parameter
            structure. Otherwise, returns the number of bytes actually wrote.
*/
#define Iap2CtrlParamWriteHeader(buffer, param_id, size_payload) Iap2CtrlParamWriteBlob(buffer, param_id, NULL, size_payload)

#endif /* __IAP2_H__ */
