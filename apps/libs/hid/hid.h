/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    hid.h
DESCRIPTION
    Header file for the HID library.
*/

/*!
@file	hid.h
@brief	Header file for the Human Interface Device library
*/

#ifndef HID_H_
#define HID_H_

#include <library.h>
#include <connection.h>
#include <message.h>
#include <panic.h>

/*!
    @brief Defines if HID profile library supports device or host, default to HID device & host.
*/
#ifndef HID_DEVICE
#define HID_DEVICE
#endif

/*!
    @brief Defines MTU size that L2CAP will advertise.
*/
#ifndef HID_L2CAP_MTU
#define HID_L2CAP_MTU	(672)
#endif

/************************************ Typedefs *****************************/

/*!
    @brief This identifies the device as a general device.
*/
#define HID_MAJOR_DEVICE_CLASS  0x0500
#define	HID_MINOR_MOUSE         0x000080
#define	HID_MINOR_KEYBOARD      0x000040

struct __HID;
/*!
    @brief The Human Interface Device Profile structures.
*/
typedef struct __HID HID;
typedef struct __HID_LIB HID_LIB;

/*!
    @brief Messages sent from HID library to the application.
*/
typedef enum
{
    HID_INIT_CFM = HID_MESSAGE_BASE,
    HID_CONNECT_IND,
    HID_CONNECT_CFM,
    HID_DISCONNECT_IND,		/* sent to a client to tell that HID connection is disconnected by a remote device */
    HID_DISCONNECT_CFM,		/* sent to a client to tell that HID disconnection which is initiated by a local host was completed */
    HID_CONTROL_IND,

#ifdef HID_DEVICE
    HID_GET_REPORT_IND,
    HID_SET_REPORT_IND,
    HID_GET_PROTOCOL_IND,
    HID_SET_PROTOCOL_IND,
    HID_GET_IDLE_IND,
    HID_SET_IDLE_IND,
    HID_DATA_IND,
#endif
    HID_MESSAGE_TOP
} HidMessageId;

/*!
    @brief Possible status codes for the initialiation confirmation.
*/
typedef enum
{
    hid_init_success,			/*!< Successful initialisation.*/
    hid_init_sdp_reg_fail,		/*!< SDP registration failed.*/
    hid_init_l2cap_reg_fail     /*!< L2CAP PSM registration failed.*/
} hid_init_status;

/*!
    @brief Possible status codes for the connection.
*/
typedef enum
{
    hid_connect_success,			/*!< Successful connection.*/
    hid_connect_failed,				/*!< Connection failed. */
    hid_connect_out_of_resources,	/*!< Out of resource. */
    hid_connect_timeout,			/*!< Timeout waiting for connection. */
    hid_connect_disconnected		/*!< Disconnected remotely during setup */
} hid_connect_status;

/*!
    @brief Possible status codes for the disconnection.
*/
typedef enum
{
    hid_disconnect_success,        /*!< Successful disconnection.*/
    hid_disconnect_link_loss,      /*!< Unsuccessful due to the link being lost.*/
    hid_disconnect_timeout,        /*!< Unsuccessful due to time out.*/
    hid_disconnect_violation,      /*!< Disconnection due to protocol violation */
    hid_disconnect_error,		   /*!< Unsuccessful for some other reason.*/
    hid_disconnect_virtual_unplug  /*!< Virtual unplug disconnection */
} hid_disconnect_status;

/*!
    @brief Possible result codes for the HANDSHAKE messages.  See HID profile 7,4,1 table 5.  NOTE: These values must match those defined in the HID profile specification.
*/
typedef enum
{
    hid_success = 0,                /*!< Successful operation.*/
    hid_busy = 1,                   /*!< Device is busy with previous operation.*/
    hid_invalid_id = 2,             /*!< Invalid report ID.*/
    hid_unsupported = 3,            /*!< Operation is unsupported.*/
    hid_invalid_param = 4,          /*!< Invalid parameter.*/
    hid_failed = 14,                /*!< Unknown failure.*/
    hid_timeout = 15,               /*!< Operation timeout.*/
    hid_resource_error = 16         /*!< Not enough resource to complete operation. (Local error, never reported to remote host/device)*/
} hid_status;

/*!
    @brief HID boot modes.  See HID profile 7.4.5, table 10 & 7.4.6, table 11.  NOTE: These values must match those defined in the HID profile specification.
*/
typedef enum
{
    hid_protocol_boot = 0,
    hid_protocol_report = 1
} hid_protocol;

/*!
    @brief HID control operations. See HID profile 7.4.2, table 6.  NOTE: These values must match those defined in the HID profile specification.
*/
typedef enum
{
    hid_control_op_nop = 0,
    hid_control_op_hard_reset = 1,
    hid_control_op_soft_reset = 2,
    hid_control_op_suspend = 3,
    hid_control_op_exit_suspend = 4,
    hid_control_op_unplug = 5
} hid_control_op;

/*!
    @brief HID report types.  See HID profile 7.4.3, table 7 and
    7.4.4, table 8.  

    NOTE: These values must match those defined in the HID profile
    specification.
*/
typedef enum
{
    hid_report_other = 0,
    hid_report_input = 1,
    hid_report_output = 2,
    hid_report_feature = 3
} hid_report_type;

/*!
    @brief HID library configuration structure.
*/
typedef struct
{
    uint16 service_record_size;
    const uint8 *service_record;
} hid_config;

/*!
    @brief This message is sent to the application when the HID library has initialised.  The application must check the status code to verifiy that initialisation was successful.  The hid_lib pointer should be stored by the application as it is required when making outgoing connections using HidConnect.
*/
typedef struct
{
    HID_LIB	*hid_lib;           /*!< The HID library structure. */
    hid_init_status	status;		/*!< The HID initialisation status.*/
} HID_INIT_CFM_T;

/*!
    @brief This message is sent to the application when the remote host/device it attempting to create a connection. The application must respond by calling HidConnectResponse to indicate whether to allow the conenction or not.
*/
typedef struct
{
    HID	*hid;                   /*!< The newly created HID instance. */
    bdaddr	bd_addr;			/*!< The Bluetooth address of the device connected to.*/
} HID_CONNECT_IND_T;

/*!
    @brief This message is sent to the application when the connection to the remote host/device has been setup or if it has failed.
*/
typedef struct
{
    HID	*hid;                   /*!< The HID instance. */
    hid_connect_status status;	/*!< The HID connection status.*/
    Sink interrupt_sink;        /*!< Sink for the interrupt channel */
} HID_CONNECT_CFM_T;

/*!
    @brief This message is sent to the application whenever the connection to the remote host/device has been disconnected, this can be from a remote disconnection.
*/
typedef struct
{
    HID	*hid;                      /*!< The HID connection instance. */
    hid_disconnect_status status;  /*!< The HID disconnect status.*/
} HID_DISCONNECT_IND_T;

/*!
    @brief This message is sent to the application whenever the connection to the remote host/device has been disconnected, this can be from a local disconnection.
*/
typedef struct
{
    HID	*hid;                      /*!< The HID connection instance. */
    hid_disconnect_status status;  /*!< The HID disconnect status.*/
} HID_DISCONNECT_CFM_T;

/*!
    @brief This message is sent to the application whenever a HID_CONTROL request is received from the remote host/device.  See HID Profile 7.4.2.
*/
typedef struct
{
    HID *hid;                   /*!< The HID connection instance. */
    hid_control_op operation;   /*!< Control operation. */ 
} HID_CONTROL_IND_T;

#ifdef HID_DEVICE
/*!
    @brief This message is sent to the application whenever a GET_REPORT request is received from the host.  See HID Profile 7.4.3.  The application must respond by calling HidGetReportResponse to send the required report to the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_report_type report_type;
    uint8 report_id;
    uint16 report_length;			/*!< Length of report data. */
} HID_GET_REPORT_IND_T;

/*!
    @brief This message is sent to the application whenever a SET_REPORT request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    uint8 report_type;				
    uint16 size_report_data;		/*!< Length of report data. */
    uint8 report_data[1];			/*!< Start of SET_REPORT data, the rest of the data follows. */
} HID_SET_REPORT_IND_T;

/*!
    @brief This message is sent to the application whenever a DATA packet received on the interrupt channel when the applciation hasn't attached a stream.
*/
typedef HID_SET_REPORT_IND_T HID_DATA_IND_T;

/*!
    @brief This message is sent to the application whenever a GET_IDLE request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
} HID_GET_IDLE_IND_T;

/*!
    @brief This message is sent to the application whenever a SET_IDLE request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    uint8 idle_rate;
} HID_SET_IDLE_IND_T;

/*!
    @brief This message is sent to the application whenever a GET_PROTOCOL request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
} HID_GET_PROTOCOL_IND_T;

/*!
    @brief This message is sent to the application whenever a SET_PROTOCOL request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    uint8 protocol;
} HID_SET_PROTOCOL_IND_T;

#endif /* HID_DEVICE */

/*!
    @brief Initialise the HID library for a device.
    @param theAppTask The current application task.
    @param config Pointer to HID configuration structure.
*/
void HidInit(Task theAppTask, const hid_config *config); 

/*!
    @brief Called to initiate a new connection,
    @param hid_lib HID library structure.
    @param Task Task that connection should be associated with.
    @param bd_addr Bluetooth address of device to connect to.
    @param hid_conftab_length The length of the L2CAP configuration table data array.
    @param hid_conftab Pointer to a L2CAP configuration table of uint16 values. These are
    key value pairs defining configuration options to be passed to Bluestack. 
    This pointer will be passed to Bluestack at which time the VM memory slot
    for it will be freed.

    Setting both hid_conftab_length and hid_conftab to 0 will cause the default L2CAP configuration 
    to be used. 
*/
void HidConnect(HID_LIB *hid_lib, Task task, const bdaddr *bd_addr, uint16 hid_conftab_length, const uint16 *hid_conftab);

/*!
    @brief Called by application in reponse to a HID_CONNECT_IND.
    @param hid HID connection instance.
    @param Task Task that connection should be associated with.
    @param accept TRUE to connect, FALSE to refuse the connection.
    @param hid_conftab_length The length of the L2CAP configuration table data array.
    @param hid_conftab Pointer to a L2CAP configuration table of uint16 values. These are
    key value pairs defining configuration options to be passed to Bluestack. 
    This pointer will be passed to Bluestack at which time the VM memory slot
    for it will be freed.

    Setting both hid_conftab_length and hid_conftab to 0 will cause the default L2CAP configuration 
    to be used. 

*/
void HidConnectResponse(HID *hid, Task task, bool accept, uint16 hid_conftab_length, const uint16 *hid_conftab);

/*!
    @brief Called to disconnect the specified HID connection.  Application will receive HID_DISCONNECT_IND once connection has been disconnected.
    @param hid HID connection instance.
*/
void HidDisconnect(HID * hid);

/*!
    @brief Called to send a HID_CONTROL message.
    @param hid HID connection instance.
    @param hid_control_op Control operation to send
*/
void HidControl(HID *hid, hid_control_op op);

/*!
    @brief Called by application to send a report on the interrupt channel.
    @param hid HID connection instance.
    @param report_type Type of report being sent to remove device.
    @param report_length Length of report.
    @param data Pointer to report data, the data is copied by the HID library.
*/
void HidInterruptReport(HID *hid, hid_report_type report_type, uint16 size_report_data, const uint8 *report_data);

#ifdef HID_DEVICE
/*!
    @brief Called by application in response to a HID_GET_REPORT_IND message
    @param hid HID connection instance.
    @param report_type Type of report: other, input, output or feature.
    @param report_length Length of report data in bytes.
    @param data Pointer to report data, the data is copied by the HID library.
*/
void HidGetReportResponse(HID *hid, hid_status status, hid_report_type report_type, uint16 size_report_data, const uint8 *report_data);

/*!
    @brief Called by application in response to a HID_SET_REPORT_IND message.
    @param hid HID connection instance.
    @param status Response code to send to host.
*/
void HidSetReportResponse(HID *hid, hid_status status);

/*!
    @brief Called by application in response to a HID_GET_PROTOCOL_IND message.
    @param hid HID connection instance.
    @param status Response code to send to host.
    @param protocol Protocol currently being used, only valid if status is hid_success.
*/
void HidGetProtocolResponse(HID *hid, hid_status status, hid_protocol protocol);

/*!
    @brief Called by application in response to a HID_SET_PROTOCOL_IND message.
    @param hid HID connection instance.
    @param status Response code to send to host.
*/
void HidSetProtocolResponse(HID *hid, hid_status status);

/*!
    @brief Called by application in response to a HID_GET_IDLE_IND message.
    @param hid HID connection instance.
    @param status Response code to send to host.
    @param idle_rate Idle rate to report to host, only valid if status is hid_success.
*/
void HidGetIdleResponse(HID *hid, hid_status status, uint8 idle_rate);

/*!
    @brief Called by application in response to a HID_SET_IDLE_IND message.
    @param hid HID connection instance.
    @param status Response code to send to host.
*/
void HidSetIdleResponse(HID *hid, hid_status status);

#endif /* HID_DEVICE */

/*!
    @brief Status of pin code entry.
*/
typedef enum
{
    hid_pin_ok,
    hid_pin_full,
    hid_pin_complete,
    hid_pin_add,
    hid_pin_delete,
    hid_pin_cleared,
    hid_pin_error
} hid_pin_status;

/*!
    @brief Maximum length of pin code.
*/
#define HID_PIN_MAX_LENGTH (16)

/*!
    @brief Pin code state structure.
*/
typedef struct
{
    bool                pin_entered;
    uint8               pin_code[HID_PIN_MAX_LENGTH];
    int                 pin_code_length;
    uint8               key_down;
} hid_pin;

/*!
    @brief Called by application start pin code entry.
    @param pin Pointer to pin code structure to use.
*/
void HidPinInit(hid_pin *pin);

/*!
    @brief Called by application to handle a keyboard input report.
    @param pin Pointer to pin code structure to use.
    @param data Pointer to report data.
    @param length Length of report data.
*/
hid_pin_status HidPinCodeHandleReport(hid_pin *pin, const uint8 *data, int length);

/*!
    @brief Called by application to retreive entered pin code.
    @param pin Pointer to pin code structure to use.
*/
const uint8 *HidPinCodeData(hid_pin *pin);

/*!
    @brief Called by application to retreive length of entered pin code.
    @param pin Pointer to pin code structure to use.
*/
int HidPinCodeLength(hid_pin *pin);

#endif /* HID_H_ */
