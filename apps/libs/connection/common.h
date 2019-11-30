/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    common.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_COMMON_H_
#define    CONNECTION_COMMON_H_

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/hci.h>
#include <app/bluestack/dm_prim.h>
#include <app/bluestack/rfcomm_prim.h>
#include <app/bluestack/l2cap_prim.h>


/****************************************************************************
NAME    
    connectionConvertHciStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined hci_status.

RETURNS
    hci_status corresponding to the status passed in
*/
hci_status connectionConvertHciStatus(uint16 status);


/****************************************************************************
NAME    
    connectionConvertAuthStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined auth_status.

RETURNS
    auth_status corresponding to the status passed in
*/
authentication_status connectionConvertAuthStatus(uint16 status);
        
        
/****************************************************************************
NAME    
    connectionConvertHciVersion

DESCRIPTION
    Convert the hci version returned by BlueStack into a connection lib
    defined hci_version.

RETURNS
    hci_version corresponding to the status passed in
*/
hci_version connectionConvertHciVersion(hci_version_t version);
        
        
/****************************************************************************
NAME    
    connectionConvertHciRole

DESCRIPTION
    Convert the mode returned by BlueStack into a connection lib
    defined hci_role.

RETURNS
    Bluestack role
*/
hci_role connectionConvertHciRole(uint8 role);


/****************************************************************************
NAME    
    connectionConvertHciRole_t

DESCRIPTION
    Convert the connection lib defined hci_role into the Bluestack role

RETURNS
    Bluestack role
*/
uint8 connectionConvertHciRole_t(hci_role role);


/****************************************************************************
NAME    
    connectionConvertInquiryMode_t

DESCRIPTION
    Convert inquiry mode into the Bluestack type

RETURNS
    hci inquiry mode corresponding to the mode passed in
*/
uint8 connectionConvertInquiryMode_t(inquiry_mode mode);


/****************************************************************************
NAME    
    connectionConvertInquiryMode

DESCRIPTION
    Convert Bluestack inquiry mode into the CL defined inquiry mode

RETURNS
    CL inquiry mode corresponding to the mode passed in
*/
inquiry_mode connectionConvertInquiryMode(uint8 mode);


/****************************************************************************
NAME    
    connectionConvertSdpOpenStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined sdp_open_status.

RETURNS
    sdp_open_status corresponding to the status passed in
*/
sdp_open_status connectionConvertSdpOpenStatus(uint16 status);


/****************************************************************************
NAME    
    connectionConvertSdpSearchStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined sdp_search_status.

RETURNS
    sdp_search_status corresponding to the status passed in
*/
sdp_search_status connectionConvertSdpSearchStatus(uint16 status);


/****************************************************************************
NAME    
    connectionConvertRfcommDisconnectStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined rfcomm_disconnect_status.

RETURNS
    rfcomm_disconnect_status corresponding to the status passed in
*/
rfcomm_disconnect_status connectionConvertRfcommDisconnectStatus(uint16 status);


/****************************************************************************
NAME    
    connectionConvertPageScanRepMode_t

DESCRIPTION
    Convert the page scan mode returned by BlueStack into a connection lib
    defined page_scan_rep_mode.

RETURNS
    page_scan_rep_mode corresponding to the mode passed in
*/
page_scan_rep_mode connectionConvertPageScanRepMode_t(page_scan_rep_mode_t mode);


/****************************************************************************
NAME    
    connectionConvertPageScanRepMode

DESCRIPTION
    Convert the connection lib defined page_scan_rep_mode into a value to be 
    passed into BlueStack.

RETURNS
    page_scan_rep_mode_t corresponding to the mode passed in
*/
page_scan_rep_mode_t connectionConvertPageScanRepMode(page_scan_rep_mode mode);


/****************************************************************************
NAME    
    connectionConvertPageScanMode_t

DESCRIPTION
    Convert the page scan mode returned by BlueStack into a connection lib
    defined page_scan_mode.

RETURNS
    page_scan_mode corresponding to the mode passed in
*/
page_scan_mode connectionConvertPageScanMode_t(page_scan_mode_t mode);


/****************************************************************************
NAME    
    connectionConvertPageScanMode

DESCRIPTION
    Convert the connection lib defined page_scan_mode into a value to be 
    passed into BlueStack.

RETURNS
    page_scan_mode_t as defined by BlueStack
*/
page_scan_mode_t connectionConvertPageScanMode(page_scan_mode mode);


/****************************************************************************
NAME    
    connectionConvertProtocolId_t

DESCRIPTION
    Convert the dm_protocol_id_t defined by BlueStack into a connection lib
    defined dm_protocol_id.

RETURNS
    dm_protocol_id corresponding to the protocol id passed in
*/
dm_protocol_id connectionConvertProtocolId_t(dm_protocol_id_t id);


/****************************************************************************
NAME    
    connectionConvertProtocolId

DESCRIPTION
    Convert from dm_protocol_id defined in connection.h to dm_protocol_id_t 
    defined by BlueStack.

RETURNS
    dm_protocol_id_t corresponding to the dm_protocol_id passed in
*/
dm_protocol_id_t connectionConvertProtocolId(dm_protocol_id id);


/****************************************************************************
NAME    
    connectionConvertHciScanEnable

DESCRIPTION
    Convert the connection lib defined HCI scan enable into the Bluestack
    defined type.

RETURNS
    Blustack HCI scan mode 
*/
uint8 connectionConvertHciScanEnable(hci_scan_enable mode);


/****************************************************************************
NAME    
    connectionConvertSecurityMode_t

DESCRIPTION
    Convert the connection lib defined security mode into the Bluestack
    defined security mode type

RETURNS
    Bluestack security mode
*/
dm_security_mode_t connectionConvertSecurityMode_t(dm_security_mode mode);


/****************************************************************************
NAME	
	connectionConvertDefaultSecurityLevel_t

DESCRIPTION
    Convert the connection lib defined default security level into the Bluestack
    defined security level type

RETURNS
    Bluestack security level
*/
dm_security_level_t connectionConvertSspSecurityLevel_t(dm_ssp_security_level level, 
                                                        bool outgoing_ok,
                                                        bool authorised,
                                                        bool disable_legacy);


/****************************************************************************
NAME    
    connectionConvertWriteAuthEnable_t

DESCRIPTION
    Convert the connection lib defined write auth enable into the Bluestack
    defined write auth enable

RETURNS
    Bluestack write auth enable
*/
uint16_t connectionConvertWriteAuthEnable_t(cl_sm_wae);


/****************************************************************************
NAME    
    connectionConvertAuthenticationRequirements

DESCRIPTION
    Convert the Bluestack defined authentication requirements into the 
    connection lib defined authentication requirements

RETURNS
    connection lib authentication requirements
*/
cl_sm_auth_requirements connectionConvertAuthenticationRequirements(uint8_t authentication_requirements);


/****************************************************************************
NAME    
    connectionConvertLinkKeyType_t

DESCRIPTION
    Convert the connection lib defined link key type into the Bluestack
    defined link key type

RETURNS
    Bluestack link key type
*/
uint8_t connectionConvertLinkKeyType_t(cl_sm_link_key_type link_key_type);


/****************************************************************************
NAME    
    connectionConvertLinkKeyType

DESCRIPTION
    Convert the Bluestack defined link key type into the Connection Lib
    defined link key type

RETURNS
    Connection Lib link key type
*/
cl_sm_link_key_type connectionConvertLinkKeyType(uint8_t link_key_type);


/****************************************************************************
NAME    
    connectionConvertIoCapability_t

DESCRIPTION
    Convert the Connection Lib defined IO capability into the Bluestack
    defined IO capability

RETURNS
    Bluestack IO capability
*/
uint8_t connectionConvertIoCapability_t(cl_sm_io_capability io_capability);
        
    
/****************************************************************************
NAME    
    connectionConvertIoCapability

DESCRIPTION
    Convert the Bluestack defined IO capability into the Connection Lib
    defined IO capability

RETURNS
    Connection Lib IO capability
*/
cl_sm_io_capability connectionConvertIoCapability(uint8_t io_capability);


/****************************************************************************
NAME    
    connectionConvertKeypressType_t

DESCRIPTION
    Convert the connection lib defined keypress type into the Bluestack
    defined keypress notification type

RETURNS
    Bluestack keypress type
*/
uint8_t connectionConvertKeypressType_t(cl_sm_keypress_type type);


/****************************************************************************
NAME    
    connectionConvertKeypressType

DESCRIPTION
    Convert the Bluestack defined keypress notification type into the 
    Connection Lib defined keypress type

RETURNS
    Connection Lib keypress type
*/
cl_sm_keypress_type connectionConvertKeypressType(uint8_t type);

/****************************************************************************
NAME    
    connectionConvertBtVersion

DESCRIPTION
    Convert the Bluestack defined BT Version into the 
    Connection Lib defined BT Version

RETURNS
    Connection Lib Version
*/
cl_dm_bt_version connectionConvertBtVersion(uint8_t version);

/****************************************************************************
NAME	
	connectionConvertSdsResult

DESCRIPTION
	Convert the Bluestack defined SDS_RESPONSE_T value into the 
	Connection Lib defined sds_status.

RETURNS
	Connection Lib sds_status
*/
sds_status connectionConvertSdsResult(uint16_t result);

/****************************************************************************
NAME	
	connectionConvertL2capConnectStatus

DESCRIPTION
	Convert the Bluestack defined l2ca_conn_result_t value into the 
	Connection Lib defined l2cap_connect_status.

RETURNS
	Connection Lib l2cap_connect_status
*/
l2cap_connect_status connectionConvertL2capConnectStatus(uint16 result);

/****************************************************************************
NAME	
	connectionConvertL2capTpConnectStatus

DESCRIPTION
	Convert the Bluestack defined l2ca_tp_conn_result_t value into the 
	Connection Lib defined l2cap_tp_connect_status.

RETURNS
	Connection Lib l2cap_tp_connect_status
*/
l2cap_connect_status connectionConvertL2capTpConnectStatus(uint16 result);

/****************************************************************************
NAME	
	connectionConvertL2capMapFixedCidResult

DESCRIPTION
	Convert the Bluestack defined l2ca_misc_result_t value into the 
	Connection Lib defined l2cap_map_connectionless_status.

RETURNS
	Connection Lib l2cap_connect_status
*/
l2cap_map_connectionless_status connectionConvertL2capMapFixedCidResult(uint16 result);

#ifndef DISABLE_BLE
/****************************************************************************
NAME	
	connectionConvertBleEventType

DESCRIPTION
	Convert the Bluestack defined events (HCI_ULP_EV_ADVERT_* defines in hci.h
	to the CL type ble_advertising_event_type.

RETURNS
	ble_advertising_event_type
*/
ble_advertising_event_type connectionConvertBleEventType(uint8 event_type);
#endif

/****************************************************************************
NAME    
    connectionConvertAPTRouteEvent

DESCRIPTION
    Convert the connection lib defined APT route event setting into the 
    BlueStack defined APT route event setting.

RETURNS
    BlueStack form of route event setting
*/
DM_SM_APT_ROUTE_T connectionConvertAPTRouteEvent(cl_apt_route_event apt_route_event);

/****************************************************************************
NAME    
    connectionConvertEncryptionType

DESCRIPTION
    Convert the Bluestack defined Encryption Type into the Connection Lib
    defined Encryption type

RETURNS
    Connection Lib Encryption Type
*/
cl_sm_encryption_key_type connectionConvertEncryptionType(uint16 encrypt_type);


#endif    /* CONNECTION_COMMON_H_ */
