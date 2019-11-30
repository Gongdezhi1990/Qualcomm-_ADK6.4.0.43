/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    common.c        

DESCRIPTION
    This file contains functions needed by the connection library for
    doing common tasks such as comparing two bluetooth device addresses.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/hci.h>
#include <app/bluestack/dm_prim.h>
#include <app/bluestack/rfcomm_prim.h>
#include <app/bluestack/l2cap_prim.h>       /* needed for L2CAP error codes */
#include <app/bluestack/sdc_prim.h>
#include <app/bluestack/sds_prim.h>
#include <vm.h>

#define CASE(from,to)  \
        case (from): \
            if ((from)==(to)) \
                goto coerce; \
            else \
                return (to);



/****************************************************************************
NAME    
    connectionConvertHciStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined hci_status.

RETURNS
    hci_status corresponding to the status passed in
*/
hci_status connectionConvertHciStatus(uint16 status)
{
    switch (status)
    {
        CASE(
                HCI_SUCCESS,
                hci_success
            );
        CASE(
                HCI_ERROR_ILLEGAL_COMMAND,
                hci_error_illegal_command
            );
        CASE(
                HCI_ERROR_NO_CONNECTION,
                hci_error_no_connection
            );
        CASE(
                HCI_ERROR_HARDWARE_FAIL,
                hci_error_hardware_fail
            );
        CASE(
                HCI_ERROR_PAGE_TIMEOUT,
                hci_error_page_timeout
            );
        CASE(
                HCI_ERROR_AUTH_FAIL,
                hci_error_auth_fail
            );
        CASE(
                HCI_ERROR_KEY_MISSING,
                hci_error_key_missing
            );
        CASE(
                HCI_ERROR_MEMORY_FULL,
                hci_error_memory_full
            );
        CASE(
                HCI_ERROR_CONN_TIMEOUT,
                hci_error_conn_timeout
            );
        CASE(
                HCI_ERROR_MAX_NR_OF_CONNS,
                hci_error_max_nr_of_conns
            );
        CASE(
                HCI_ERROR_MAX_NR_OF_SCO,
                hci_error_max_nr_of_sco
            );
        CASE(
                HCI_ERROR_MAX_NR_OF_ACL,
                hci_error_max_nr_of_acl
            );
        CASE(
                HCI_ERROR_COMMAND_DISALLOWED,
                hci_error_command_disallowed
            );
        CASE(
                HCI_ERROR_REJ_BY_REMOTE_NO_RES,
                hci_error_rej_by_remote_no_res
            );
        CASE(
                HCI_ERROR_REJ_BY_REMOTE_SEC,
                hci_error_rej_by_remote_sec
            );
        CASE(
                HCI_ERROR_REJ_BY_REMOTE_PERS,
                hci_error_rej_by_remote_pers
            );
        CASE(
                HCI_ERROR_HOST_TIMEOUT,
                hci_error_host_timeout
            );
        CASE(
                HCI_ERROR_UNSUPPORTED_FEATURE,
                hci_error_unsupported_feature
            );
        CASE(
                HCI_ERROR_ILLEGAL_FORMAT,
                hci_error_illegal_format
            );
        CASE(
                HCI_ERROR_OETC_USER,
                hci_error_oetc_user
            );
        CASE(
                HCI_ERROR_OETC_LOW_RESOURCE,
                hci_error_oetc_low_resource
            );
        CASE(
                HCI_ERROR_OETC_POWERING_OFF,
                hci_error_oetc_powering_off
            );
        CASE(
                HCI_ERROR_CONN_TERM_LOCAL_HOST,
                hci_error_conn_term_local_host
            );
        CASE(
                HCI_ERROR_AUTH_REPEATED,
                hci_error_auth_repeated
            );
        CASE(
                HCI_ERROR_PAIRING_NOT_ALLOWED,
                hci_error_pairing_not_allowed
            );
        CASE(
                HCI_ERROR_UNKNOWN_LMP_PDU,
                hci_error_unknown_lmp_pdu
            );
        CASE(
                HCI_ERROR_UNSUPPORTED_REM_FEATURE,
                hci_error_unsupported_rem_feature
            );
        CASE(
                HCI_ERROR_SCO_OFFSET_REJECTED,
                hci_error_sco_offset_rejected
            );
        CASE(
                HCI_ERROR_SCO_INTERVAL_REJECTED,
                hci_error_sco_interval_rejected
            );
        CASE(
                HCI_ERROR_SCO_AIR_MODE_REJECTED,
                hci_error_sco_air_mode_rejected
            );
        CASE(
                HCI_ERROR_INVALID_LMP_PARAMETERS,
                hci_error_invalid_lmp_parameters
            );
        CASE(
                HCI_ERROR_UNSPECIFIED,
                hci_error_unspecified
            );
        CASE(
                HCI_ERROR_UNSUPP_LMP_PARAM,
                hci_error_unsupp_lmp_param
            );
        CASE(
                HCI_ERROR_ROLE_CHANGE_NOT_ALLOWED,
                hci_error_role_change_not_allowed
            );
        CASE(
                HCI_ERROR_LMP_RESPONSE_TIMEOUT,
                hci_error_lmp_response_timeout
            );
        CASE(
                HCI_ERROR_LMP_TRANSACTION_COLLISION,
                hci_error_lmp_transaction_collision
            );
        CASE(
                HCI_ERROR_LMP_PDU_NOT_ALLOWED,
                hci_error_lmp_pdu_not_allowed
            );
        CASE(
                HCI_ERROR_ENC_MODE_NOT_ACCEPTABLE,
                hci_error_enc_mode_not_acceptable
            );
        CASE(
                HCI_ERROR_UNIT_KEY_USED,
                hci_error_unit_key_used
            );
        CASE(
                HCI_ERROR_QOS_NOT_SUPPORTED,
                hci_error_qos_not_supported
            );
        CASE(
                HCI_ERROR_INSTANT_PASSED,
                hci_error_instant_passed
            );
        CASE(
                HCI_ERROR_PAIR_UNIT_KEY_NO_SUPPORT,
                hci_error_pair_unit_key_no_support
            );
        CASE(
                HCI_ERROR_DIFFERENT_TRANSACTION_COLLISION,
                hci_error_different_transaction_collision
            );
        CASE(
                HCI_ERROR_SCM_INSUFFICIENT_RESOURCES,
                hci_error_scm_insufficient_resources
            );
        CASE(
                HCI_ERROR_QOS_UNACCEPTABLE_PARAMETER,
                hci_error_qos_unacceptable_parameter
            );
        CASE(
                HCI_ERROR_QOS_REJECTED,
                hci_error_qos_rejected
            );
        CASE(
                HCI_ERROR_CHANNEL_CLASS_NO_SUPPORT,
                hci_error_channel_class_no_support
            );
        CASE(
                HCI_ERROR_INSUFFICIENT_SECURITY,
                hci_error_insufficient_security
            );
        CASE(
                HCI_ERROR_PARAM_OUT_OF_MAND_RANGE,
                hci_error_param_out_of_mand_range
            );
        CASE(
                HCI_ERROR_SCM_NO_LONGER_REQD,
                hci_error_scm_no_longer_reqd
            );
        CASE(
                HCI_ERROR_ROLE_SWITCH_PENDING,
                hci_error_role_switch_pending
            );
        CASE(
                HCI_ERROR_SCM_PARAM_CHANGE_PENDING,
                hci_error_scm_param_change_pending
            );
        CASE(
                HCI_ERROR_RESVD_SLOT_VIOLATION,
                hci_error_resvd_slot_violation
            );
        CASE(
                HCI_ERROR_ROLE_SWITCH_FAILED,
                hci_error_role_switch_failed
            );        
        coerce: return (hci_status)status;       
        default: 
            CL_DEBUG(("Unrecognised status %d\n", status));
            return hci_error_unrecognised;
    }
}

/****************************************************************************
NAME    
    connectionConvertAuthStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined authentication_status.

RETURNS
    authentication_status corresponding to the status passed in
*/
authentication_status connectionConvertAuthStatus(uint16 status)
{
    switch (status)
    {
        CASE(
                HCI_SUCCESS,
                auth_status_success
            );
        CASE(
                HCI_ERROR_PAGE_TIMEOUT,
                auth_status_timeout
            );
        CASE(
                HCI_ERROR_AUTH_REPEATED,
                auth_status_repeat_attempts
            );
        CASE(
                HCI_ERROR_PAIRING_NOT_ALLOWED,
                auth_status_pairing_not_allowed
            );
        CASE(
                HCI_ERROR_UNIT_KEY_USED,
                auth_status_unit_key_unsupported
            );
        CASE(
                HCI_ERROR_SP_NOT_SUPPORTED_BY_HOST,
                auth_status_simple_pairing_unsupported
            );
        CASE(
                HCI_ERROR_HOST_BUSY_PAIRING,
                auth_status_host_busy_pairing
            );
        coerce: return (authentication_status)status;       
        default:
            return auth_status_fail;
    }
}

    
/****************************************************************************
NAME    
    connectionConvertHciVersion

DESCRIPTION
    Convert the hci version returned by BlueStack into a connection lib
    defined hci_version.

RETURNS
    hci_version corresponding to the status passed in
*/
hci_version connectionConvertHciVersion(hci_version_t version)
{
    switch (version)
    {
        CASE(HCI_VER_1_0, hci_version_1_0);
        CASE(HCI_VER_1_1, hci_version_1_1);
        CASE(HCI_VER_1_2, hci_version_1_2);
        CASE(HCI_VER_2_0, hci_version_2_0);
        CASE(HCI_VER_2_1, hci_version_2_1);
        CASE(HCI_VER_3_0, hci_version_3_0);
        CASE(HCI_VER_4_0, hci_version_4_0);
        CASE(HCI_VER_4_1, hci_version_4_1);
        CASE(HCI_VER_4_2, hci_version_4_2);
        CASE(HCI_VER_5_0, hci_version_5_0);
        CASE(HCI_VER_5_1, hci_version_5_1);
        coerce: return (hci_version)version;       
        default: 
            CL_DEBUG(("Unrecognised version %d\n",version));
            return hci_version_unrecognised;        
    }
}


/****************************************************************************
NAME    
    connectionConvertInquiryMode_t

DESCRIPTION
    Convert inquiry mode into the Bluestack type

RETURNS
    hci inquiry mode corresponding to the mode passed in
*/
uint8 connectionConvertInquiryMode_t(inquiry_mode mode)
{
    switch(mode)
    {   
        CASE(inquiry_mode_standard, HCI_INQUIRY_MODE_STANDARD);
        CASE(inquiry_mode_rssi, HCI_INQUIRY_MODE_WITH_RSSI);
        CASE(inquiry_mode_eir, HCI_INQUIRY_MODE_WITH_EIR);
        coerce: return (uint8)mode;       
        default: 
            CL_DEBUG(("Unrecognised mode %d\n",mode));
            return HCI_INQUIRY_MODE_STANDARD;
    }
}


/****************************************************************************
NAME    
    connectionConvertInquiryMode

DESCRIPTION
    Convert Bluestack inquiry mode into the CL defined inquiry mode

RETURNS
    CL inquiry mode corresponding to the mode passed in
*/
inquiry_mode connectionConvertInquiryMode(uint8 mode)
{
    switch (mode)
    {   
        CASE(HCI_INQUIRY_MODE_STANDARD, inquiry_mode_standard);
        CASE(HCI_INQUIRY_MODE_WITH_RSSI, inquiry_mode_rssi);
        CASE(HCI_INQUIRY_MODE_WITH_EIR, inquiry_mode_eir);
        coerce: return (inquiry_mode)mode;       
        default: 
            CL_DEBUG(("Unrecognised mode %d\n",mode));
            return inquiry_mode_standard;
    }
}


/****************************************************************************
NAME    
    connectionConvertSdpOpenStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined sdp_open_status.

RETURNS
    sdp_open_status corresponding to the status passed in
*/
sdp_open_status connectionConvertSdpOpenStatus(uint16 status)
{
    switch(status)
    {
        CASE(SDC_OPEN_SEARCH_OK, sdp_open_search_ok );
        CASE(SDC_OPEN_SEARCH_BUSY, sdp_open_search_busy );
        CASE(SDC_OPEN_SEARCH_OPEN, sdp_open_search_open );
        CASE(SDC_OPEN_DISCONNECTED, sdp_open_disconnected );
        CASE(L2CA_RESULT_FAILED, sdp_open_search_failed);
        coerce: return (sdp_open_status)status;
        default:
            CL_DEBUG_INFO(("Unrecognised SDP Open status %d\n",status));
            return connectionConvertL2capConnectStatus(status);
    }
}


/****************************************************************************
NAME    
    connectionConvertSdpSearchStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined sdp_search_status.

RETURNS
    sdp_search_status corresponding to the status passed in
*/
sdp_search_status connectionConvertSdpSearchStatus(uint16 status)
{
    switch (status)
    {
        CASE(SDC_SUCCESS, sdp_response_success );
        CASE(SDC_ERROR_RESPONSE_PDU, sdp_error_response_pdu );
        CASE(SDC_NO_RESPONSE_DATA, sdp_no_response_data );
        CASE(SDC_CON_DISCONNECTED, sdp_con_disconnected );
        CASE(SDC_CONNECTION_ERROR_UNKNOWN, sdp_connection_error );
        CASE(SDC_CONFIGURE_ERROR, sdp_configure_error );
        CASE(SDC_SEARCH_DATA_ERROR, sdp_search_data_error );
        CASE(SDC_DATA_CFM_ERROR, sdp_data_cfm_error );
        CASE(SDC_SEARCH_BUSY, sdp_search_busy );
        CASE(SDC_RESPONSE_PDU_HEADER_ERROR, sdp_response_pdu_header_error );
        CASE(SDC_RESPONSE_PDU_SIZE_ERROR, sdp_response_pdu_size_error );
        CASE(SDC_RESPONSE_TIMEOUT_ERROR, sdp_response_timeout_error );
        CASE(SDC_SEARCH_SIZE_TO_BIG, sdp_search_size_to_big );
        CASE(SDC_RESPONSE_OUT_OF_MEMORY, sdp_response_out_of_memory );
        CASE(SDC_RESPONSE_TERMINATED, sdp_response_terminated );
        coerce: return (sdp_search_status)status;       
        default:
            CL_DEBUG_INFO(("Unrecognised SDP Search status %d\n",status));
            return connectionConvertL2capConnectStatus(status);
    }
}


/****************************************************************************
NAME    
    connectionConvertRfcommDisconnectStatus

DESCRIPTION
    Convert the status returned by BlueStack into a connection lib
    defined rfcomm_disconnect_status.

RETURNS
    rfcomm_disconnect_status corresponding to the status passed in
*/
rfcomm_disconnect_status connectionConvertRfcommDisconnectStatus(uint16 status)
{
    switch (status)
    {
        CASE(
                RFC_SUCCESS,
                rfcomm_disconnect_success
            );
        CASE(
                RFC_NORMAL_DISCONNECT,
                rfcomm_disconnect_normal_disconnect
            );
        CASE(
                RFC_ABNORMAL_DISCONNECT,
                rfcomm_disconnect_abnormal_disconnect
            );
        CASE(
                RFC_REMOTE_REFUSAL,
                rfcomm_disconnect_remote_refusal
            );
        CASE(
                RFC_CHANNEL_ALREADY_EXISTS,
                rfcomm_disconnect_dlc_already_exists
            );
        CASE(
                RFC_CONNECTION_REJ_SECURITY,
                rfcomm_disconnect_dlc_rej_security
            );
        CASE(
                RFC_INVALID_CHANNEL,
                rfcomm_disconnect_invalid_server_channel
            );
        CASE(
                RFC_UNKNOWN_PRIMITIVE,
                rfcomm_disconnect_unknown_primitive
            );
        CASE(
                RFC_INVALID_PAYLOAD,
                rfcomm_disconnect_max_payload_exceeded
            );
        CASE(
                RFC_INCONSISTENT_PARAMETERS,
                rfcomm_disconnect_inconsistent_parameters
            );
        CASE(
                RFC_PEER_VIOLATED_FLOW_CONTROL,
                rfcomm_disconnect_credit_flow_control_protocol_violation
            );
        CASE(
                L2CA_DISCONNECT_LINK_LOSS,
                rfcomm_disconnect_l2cap_link_loss
            );
        coerce: return (rfcomm_disconnect_status)status;       
        default: 
            if (status < RFCOMM_ERRORCODE_BASE)
            {
                return rfcomm_disconnect_l2cap_error;
            }
            else
            {
                CL_DEBUG(("Unrecognised status %d\n", status));
                return rfcomm_disconnect_unknown;
            }
    }
}

/****************************************************************************
NAME    
    connectionConvertPageScanRepMode_t

DESCRIPTION
    Convert the page scan mode returned by BlueStack into a connection lib
    defined page_scan_rep_mode.

RETURNS
    page_scan_rep_mode corresponding to the mode passed in
*/
page_scan_rep_mode connectionConvertPageScanRepMode_t(page_scan_rep_mode_t mode)
{
    switch (mode)
    {
        CASE(HCI_PAGE_SCAN_REP_MODE_R0, page_scan_rep_mode_r0);
        CASE(HCI_PAGE_SCAN_REP_MODE_R1, page_scan_rep_mode_r1);
        CASE(HCI_PAGE_SCAN_REP_MODE_R2, page_scan_rep_mode_r2);
        coerce: return (page_scan_rep_mode)mode;       
        default:
            CL_DEBUG(("Unrecognised mode %d\n", mode));
            return page_scan_rep_mode_unknown;
    }
}


/****************************************************************************
NAME    
    connectionConvertPageScanRepMode

DESCRIPTION
    Convert the connection lib defined page_scan_rep_mode into a value to be 
    passed into BlueStack.

RETURNS
    page_scan_rep_mode_t corresponding to the mode passed in
*/
page_scan_rep_mode_t connectionConvertPageScanRepMode(page_scan_rep_mode mode)
{
    switch (mode)
    {
        CASE(page_scan_rep_mode_r0, HCI_PAGE_SCAN_REP_MODE_R0);
        CASE(page_scan_rep_mode_r1, HCI_PAGE_SCAN_REP_MODE_R1);
        CASE(page_scan_rep_mode_r2, HCI_PAGE_SCAN_REP_MODE_R2);
        coerce: return (page_scan_rep_mode_t)mode;       
        case page_scan_rep_mode_unknown:
        default:
            CL_DEBUG(("Invalid page scan rep mode passed in 0x%x\n", mode));
            /* the sensible default should get us a connection eventually */
            return HCI_PAGE_SCAN_REP_MODE_R2;
    }
}


/****************************************************************************
NAME    
    connectionConvertPageScanMode_t

DESCRIPTION
    Convert the page scan mode returned by BlueStack into a connection lib
    defined page_scan_mode.

RETURNS
    page_scan_mode corresponding to the mode passed in
*/
page_scan_mode connectionConvertPageScanMode_t(page_scan_mode_t mode)
{
    switch (mode)
    {
        CASE(HCI_PAGE_SCAN_MODE_MANDATORY, page_scan_mode_mandatory);
        CASE(HCI_PAGE_SCAN_MODE_OPTIONAL_1, page_scan_mode_optional_1);
        CASE(HCI_PAGE_SCAN_MODE_OPTIONAL_2, page_scan_mode_optional_2);
        CASE(HCI_PAGE_SCAN_MODE_OPTIONAL_3, page_scan_mode_optional_3);
        coerce: return (page_scan_mode)mode;       
        default:
            CL_DEBUG(("Unrecognised mode %d\n", mode));
            return page_scan_mode_unknown;
    }
}


/****************************************************************************
NAME    
    connectionConvertPageScanMode

DESCRIPTION
    Convert the connection lib defined page_scan_mode into a value to be 
    passed into BlueStack.

RETURNS
    page_scan_mode_t as defined by BlueStack
*/
page_scan_mode_t connectionConvertPageScanMode(page_scan_mode mode)
{
    switch (mode)
    {
        CASE(page_scan_mode_mandatory, HCI_PAGE_SCAN_MODE_MANDATORY);
        CASE(page_scan_mode_optional_1, HCI_PAGE_SCAN_MODE_OPTIONAL_1);
        CASE(page_scan_mode_optional_2, HCI_PAGE_SCAN_MODE_OPTIONAL_2);
        CASE(page_scan_mode_optional_3, HCI_PAGE_SCAN_MODE_OPTIONAL_3);
        coerce: return (page_scan_mode_t)mode;       
        case page_scan_mode_unknown:
        default:
            CL_DEBUG(("Invalid page scan mode passed in 0x%x\n", mode));
            /* this is probably the safest default */
            return HCI_PAGE_SCAN_MODE_MANDATORY;
    }
}



/****************************************************************************
NAME    
    connectionConvertProtocolId_t

DESCRIPTION
    Convert the dm_protocol_id_t defined by BlueStack into a connection lib
    defined dm_protocol_id.

RETURNS
    dm_protocol_id corresponding to the protocol id passed in
*/
dm_protocol_id connectionConvertProtocolId_t(dm_protocol_id_t id)
{
    if (id == SEC_PROTOCOL_L2CAP)
        return protocol_l2cap;
    else if (id == SEC_PROTOCOL_RFCOMM)
        return protocol_rfcomm;
    else if (id == SEC_PROTOCOL_LE_L2CAP)
        return protocol_le_l2cap;
    else if (id >= SEC_PROTOCOL_USER)
        return (dm_protocol_id) id;
    else 
    {
        CL_DEBUG(("Unrecognised id %d\n", (uint16) id));
        return protocol_unknown;
    }
}


/****************************************************************************
NAME    
    connectionConvertProtocolId

DESCRIPTION
    Convert from dm_protocol_id defined in connection.h to dm_protocol_id_t 
    defined by BlueStack.

RETURNS
    dm_protocol_id_t corresponding to the dm_protocol_id passed in
*/
dm_protocol_id_t connectionConvertProtocolId(dm_protocol_id id)
{
    if (id == protocol_l2cap)
        return SEC_PROTOCOL_L2CAP;
    else if (id == protocol_rfcomm)
        return SEC_PROTOCOL_RFCOMM;
    else if (id == protocol_le_l2cap)
        return SEC_PROTOCOL_LE_L2CAP;
    else
        return (dm_protocol_id_t) id;
}


/****************************************************************************
NAME    
    connectionConvertHciScanEnable

DESCRIPTION
    Convert the connection lib defined HCI scan enable into the Bluestack
    defined type.

RETURNS
    Blustack HCI scan mode 
*/
uint8 connectionConvertHciScanEnable(hci_scan_enable mode)
{
    switch (mode)
    {
        CASE(hci_scan_enable_off, HCI_SCAN_ENABLE_OFF);
        CASE(hci_scan_enable_inq, HCI_SCAN_ENABLE_INQ);
        CASE(hci_scan_enable_page, HCI_SCAN_ENABLE_PAGE);
        CASE(hci_scan_enable_inq_and_page, HCI_SCAN_ENABLE_INQ_AND_PAGE);
        coerce: return (uint8)mode;       
        default:
            CL_DEBUG(("Unrecognised mode %d\n",mode));
            /* safest default is to assume connectable and discoverable */
            return HCI_SCAN_ENABLE_INQ_AND_PAGE;

    }
}


/****************************************************************************
NAME    
    connectionConvertSecurityMode_t

DESCRIPTION
    Convert the connection lib defined security mode into the Bluestack
    defined security mode type

RETURNS
    Bluestack security mode
*/
dm_security_mode_t connectionConvertSecurityMode_t(dm_security_mode mode)
{
    switch(mode)
    {
        CASE(sec_mode0_off, SEC_MODE0_OFF)
        CASE(sec_mode1_non_secure, SEC_MODE1_NON_SECURE);
        CASE(sec_mode2_service, SEC_MODE2_SERVICE);
        CASE(sec_mode3_link, SEC_MODE3_LINK);
        CASE(sec_mode4_ssp, SEC_MODE4_SSP);
        CASE( sec_mode2a_device, SEC_MODE2a_DEVICE);
        coerce: return (dm_security_mode_t)mode;
        case sec_mode_unknown:
        default:
            CL_DEBUG(("Unrecognised mode %d\n",mode));
            /* tricky - probably the safest default is to turn
                   off security - at least the device can then be used
            */
            return SEC_MODE0_OFF;
    }
}


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
                                                        bool disable_legacy)
{
    dm_security_level_t secl;
    if(outgoing_ok)
    {
        switch(level)
        {
            case ssp_secl4_l0:
                secl = SECL4_LEVEL_0;
                break;
            case ssp_secl4_l1:
                secl = SECL4_LEVEL_1;
                break;
            case ssp_secl4_l2:
                secl = SECL4_LEVEL_2;
                break;
            case ssp_secl4_l3:
                secl = SECL4_LEVEL_3;
                break;
            case ssp_secl4_l4:
                secl = SECL4_LEVEL_4;
                break;
            case ssp_secl_level_unknown:
            default:
                CL_DEBUG(("Unrecognised mode %d\n", level));
                /* Use the default given in the 2.1 spec */
                secl = SECL4_LEVEL_2;
                break;
        }
        if(authorised)
            secl |= SECL_AUTHORISATION;
        if(disable_legacy)
            secl |= SECL4_NO_LEGACY;
    }
    else
    {
        switch(level)
        {
            case ssp_secl4_l0:
                secl = SECL4_IN_LEVEL_0;
                break;
            case ssp_secl4_l1:
                secl = SECL4_IN_LEVEL_1;
                break;
            case ssp_secl4_l2:
                secl = SECL4_IN_LEVEL_2;
                break;
            case ssp_secl4_l3:
                secl = SECL4_IN_LEVEL_3;
                break;
            case ssp_secl4_l4:
                secl = SECL4_IN_LEVEL_4;
                break;
            case ssp_secl_level_unknown:
            default:
                CL_DEBUG(("Unrecognised mode %d\n", level));
                /* Use the default given in the 2.1 spec */
                secl = SECL4_IN_LEVEL_2;
                break;
        }
        if(authorised)
            secl |= SECL_IN_AUTHORISATION;
        if(disable_legacy)
            secl |= SECL4_IN_NO_LEGACY;
    }
    return secl;
}


/****************************************************************************
NAME    
    connectionConvertWriteAuthEnable_t

DESCRIPTION
    Convert the connection lib defined write auth enable into the Bluestack
    defined write auth enable

RETURNS
    Bluestack write auth enable
*/
uint16_t connectionConvertWriteAuthEnable_t(cl_sm_wae write_auth_enable)
{
    switch(write_auth_enable)
    {
        CASE(cl_sm_wae_never, DM_SM_WAE_NEVER);
        CASE(cl_sm_wae_acl_none, DM_SM_WAE_ACL_NONE);
        CASE(cl_sm_wae_acl_owner_none, DM_SM_WAE_ACL_OWNER_NONE);
        CASE(cl_sm_wae_acl_owner_app, DM_SM_WAE_ACL_OWNER_APP);
        CASE(cl_sm_wae_acl_owner_l2cap, DM_SM_WAE_ACL_OWNER_L2CAP);
        CASE(cl_sm_wae_always, DM_SM_WAE_ALWAYS);
        coerce: return (uint16_t)write_auth_enable;       
        default:
            CL_DEBUG(("Unrecognised wae %d\n", write_auth_enable));
            return DM_SM_WAE_ACL_NONE;
    }
}

/****************************************************************************
NAME    
    connectionConvertAuthenticationRequirements

DESCRIPTION
    Convert the Bluestack defined authentication requirements into the 
    connection lib defined authentication requirements

RETURNS
    connection lib authentication requirements
*/
cl_sm_auth_requirements connectionConvertAuthenticationRequirements(
        uint8_t authentication_requirements
        )
{
    switch(authentication_requirements)
    {
        CASE(
                HCI_MITM_NOT_REQUIRED_NO_BONDING,
                cl_sm_no_bonding_no_mitm
            );
        CASE(
                HCI_MITM_REQUIRED_NO_BONDING,
                cl_sm_no_bonding_mitm
            );
        CASE(
                HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING,
                cl_sm_dedicated_bonding_no_mitm
            );
        CASE(
                HCI_MITM_REQUIRED_DEDICATED_BONDING,
                cl_sm_dedicated_bonding_mitm
            );
        CASE(
                HCI_MITM_NOT_REQUIRED_GENERAL_BONDING,
                cl_sm_general_bonding_no_mitm
            );
        CASE(
                HCI_MITM_REQUIRED_GENERAL_BONDING,
                cl_sm_general_bonding_mitm
            );
        coerce: return (cl_sm_auth_requirements)authentication_requirements;       
        default:
            CL_DEBUG((
                        "Unrecognised authentication requirements %d\n",
                        authentication_requirements
                     ));
            return cl_sm_authentication_requirements_unknown;
    }
}


/****************************************************************************
NAME    
    connectionConvertLinkKeyType_t

DESCRIPTION
    Convert the connection lib defined link key type into the Bluestack
    defined link key type

RETURNS
    Bluestack security level
*/
uint8_t connectionConvertLinkKeyType_t(cl_sm_link_key_type link_key_type)
{
    switch(link_key_type)
    {
        CASE(cl_sm_link_key_none, DM_SM_LINK_KEY_NONE);
        CASE(cl_sm_link_key_legacy, DM_SM_LINK_KEY_LEGACY);
        CASE(cl_sm_link_key_debug, DM_SM_LINK_KEY_DEBUG);
        CASE(
                cl_sm_link_key_unauthenticated_p192,
                DM_SM_LINK_KEY_UNAUTHENTICATED_P192
                );
        CASE(
                cl_sm_link_key_authenticated_p192, 
                DM_SM_LINK_KEY_AUTHENTICATED_P192
                );
        CASE(cl_sm_link_key_changed, DM_SM_LINK_KEY_CHANGED);
        CASE(
                cl_sm_link_key_unauthenticated_p256,
                DM_SM_LINK_KEY_UNAUTHENTICATED_P256
                );
        CASE(
                cl_sm_link_key_authenticated_p256, 
                DM_SM_LINK_KEY_AUTHENTICATED_P256
                );
        coerce: return (uint8_t)link_key_type;       
        default:
            CL_DEBUG(("Unrecognised link key type %d\n", link_key_type));
            return DM_SM_LINK_KEY_NONE;
    }
}

/****************************************************************************
NAME    
    connectionConvertLinkKeyType

DESCRIPTION
    Convert the Bluestack defined link key type into the Connection Lib
    defined link key type

RETURNS
    Connection Lib security level
*/
cl_sm_link_key_type connectionConvertLinkKeyType(uint8_t link_key_type)
{
    switch(link_key_type)
    {
        CASE(DM_SM_LINK_KEY_NONE, cl_sm_link_key_none);
        CASE(DM_SM_LINK_KEY_LEGACY, cl_sm_link_key_legacy);
        CASE(DM_SM_LINK_KEY_DEBUG, cl_sm_link_key_debug);
        CASE(
                DM_SM_LINK_KEY_UNAUTHENTICATED_P192,
                cl_sm_link_key_unauthenticated_p192
                );
        CASE(
                DM_SM_LINK_KEY_AUTHENTICATED_P192,
                cl_sm_link_key_authenticated_p192
                );
        CASE(DM_SM_LINK_KEY_CHANGED, cl_sm_link_key_changed);
        CASE(
                DM_SM_LINK_KEY_UNAUTHENTICATED_P256,
                cl_sm_link_key_unauthenticated_p256
                );
        CASE(
                DM_SM_LINK_KEY_AUTHENTICATED_P256,
                cl_sm_link_key_authenticated_p256
                );
        coerce: return (cl_sm_link_key_type)link_key_type;       
        default:
            CL_DEBUG(("Unrecognised link key type %d\n", link_key_type));
            return cl_sm_link_key_none;
    }
}

/****************************************************************************
NAME    
    connectionConvertIoCapability_t

DESCRIPTION
    Convert the Connection Lib defined IO capability into the Bluestack
    defined IO capability

RETURNS
    Bluestack IO capability
*/
uint8_t connectionConvertIoCapability_t(cl_sm_io_capability io_capability)
{
    switch(io_capability)
    {
        CASE(cl_sm_io_cap_display_only, HCI_IO_CAP_DISPLAY_ONLY);
        CASE(cl_sm_io_cap_display_yes_no, HCI_IO_CAP_DISPLAY_YES_NO);
        CASE(cl_sm_io_cap_keyboard_only, HCI_IO_CAP_KEYBOARD_ONLY);
        CASE(cl_sm_io_cap_no_input_no_output, HCI_IO_CAP_NO_INPUT_NO_OUTPUT);
        CASE(cl_sm_io_cap_keyboard_display, HCI_IO_CAP_KEYBOARD_DISPLAY);
        coerce: return (uint8_t)io_capability;       
        default:
            CL_DEBUG(("Unrecognised IO capability %d\n", io_capability));
            return HCI_IO_CAP_NO_INPUT_NO_OUTPUT;
    }
}

/****************************************************************************
NAME    
    connectionConvertIoCapability

DESCRIPTION
    Convert the Bluestack defined IO capability into the Connection Lib
    defined IO capability

RETURNS
    Connection Lib IO capability
*/
cl_sm_io_capability connectionConvertIoCapability(uint8_t io_capability)
{
    switch(io_capability)
    {
        CASE(HCI_IO_CAP_DISPLAY_ONLY, cl_sm_io_cap_display_only);
        CASE(HCI_IO_CAP_DISPLAY_YES_NO, cl_sm_io_cap_display_yes_no);
        CASE(HCI_IO_CAP_KEYBOARD_ONLY,  cl_sm_io_cap_keyboard_only);
        CASE(HCI_IO_CAP_NO_INPUT_NO_OUTPUT, cl_sm_io_cap_no_input_no_output);
        CASE(HCI_IO_CAP_KEYBOARD_DISPLAY, cl_sm_io_cap_keyboard_display);
        coerce: return (cl_sm_io_capability)io_capability;       
        default:
            CL_DEBUG(("Unrecognised IO capability %d\n", io_capability));
            return cl_sm_io_cap_no_input_no_output;
    }
}

/****************************************************************************
NAME    
    connectionConvertKeypressType_t

DESCRIPTION
    Convert the connection lib defined keypress type into the Bluestack
    defined keypress notification type

RETURNS
    Bluestack keypress type
*/
uint8_t connectionConvertKeypressType_t(cl_sm_keypress_type type)
{
    switch(type)
    {
        CASE(
                cl_sm_passkey_started,
                HCI_NOTIFICATION_TYPE_PASSKEY_STARTED
            );
        CASE(
                cl_sm_passkey_digit_entered,
                HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ENTERED
            );
        CASE(
                cl_sm_passkey_digit_erased,
                HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ERASED
            );
        CASE(
                cl_sm_passkey_cleared,
                HCI_NOTIFICATION_TYPE_PASSKEY_CLEARED
            );
        CASE(
                cl_sm_passkey_complete,
                HCI_NOTIFICATION_TYPE_PASSKEY_COMPLETED
            );
        coerce: return (uint8_t)type;       
        default:
            CL_DEBUG(("Unrecognised keypress type %d\n", type));
            return HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ENTERED;
    }
}

/****************************************************************************
NAME    
    connectionConvertKeypressType

DESCRIPTION
    Convert the Bluestack defined keypress notification type into the 
    Connection Lib defined keypress type

RETURNS
    Connection Lib keypress type
*/
cl_sm_keypress_type connectionConvertKeypressType(uint8_t type)
{
    switch(type)
    {
        CASE(
                HCI_NOTIFICATION_TYPE_PASSKEY_STARTED,
                cl_sm_passkey_started
            );
        CASE(
                HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ENTERED,
                cl_sm_passkey_digit_entered
            );
        CASE(
                HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ERASED,
                cl_sm_passkey_digit_erased
            );
        CASE(
                HCI_NOTIFICATION_TYPE_PASSKEY_CLEARED,
                cl_sm_passkey_cleared
            );
        CASE(
                HCI_NOTIFICATION_TYPE_PASSKEY_COMPLETED,
                cl_sm_passkey_complete
            );
        coerce: return (cl_sm_keypress_type)type;       
        default:
            CL_DEBUG(("Unrecognised keypress type %d\n", type));
            return cl_sm_passkey_digit_entered;
    }
}


/****************************************************************************
NAME    
    connectionConvertHciRole

DESCRIPTION
    Convert the mode returned by BlueStack into a connection lib defined 
    hci_role.

RETURNS
    Bluestack role
*/
hci_role connectionConvertHciRole(uint8 role)
{
    switch(role)
    {
        CASE(HCI_MASTER, hci_role_master);
        CASE(HCI_SLAVE, hci_role_slave);
        coerce: return (hci_role)role;       
        default:
            CL_DEBUG(("Unrecognised role %d",role));
            return hci_role_dont_care;
    }
}


/****************************************************************************
NAME    
    connectionConvertHciRole_t

DESCRIPTION
    Convert the connection lib defined hci_role into the Bluestack role

RETURNS
    Bluestack role
*/
uint8 connectionConvertHciRole_t(hci_role role)
{
    switch(role)
    {
        CASE(hci_role_master, HCI_MASTER);          
        CASE(hci_role_slave, HCI_SLAVE);
        coerce: return (uint8)role;       
        case hci_role_dont_care:
        default:
            CL_DEBUG(("Unrecognised role %d\n",role));
            return HCI_MASTER_SLAVE_UNKNOWN;            
    }
}


/****************************************************************************
NAME    
    connectionConvertBtVersion

DESCRIPTION
    Convert the Bluestack defined BT Version into the 
    Connection Lib defined BT Version

RETURNS
    Connection Lib version
*/
cl_dm_bt_version connectionConvertBtVersion(uint8_t version)
{
    switch(version)
    {
        CASE(BT_VERSION_2p0, bluetooth2_0);
        CASE(BT_VERSION_2p1, bluetooth2_1);
        CASE(BT_VERSION_3p0, bluetooth3_0);
        CASE(BT_VERSION_4p0, bluetooth4_0);
        CASE(BT_VERSION_4p1, bluetooth4_1);
        CASE(BT_VERSION_4p2, bluetooth4_2);
        CASE(BT_VERSION_5p0, bluetooth5_0);
        CASE(BT_VERSION_5p1, bluetooth5_1);
        coerce: return (cl_dm_bt_version)version;       
        case BT_VERSION_CURRENT:
        default:
            CL_DEBUG(("Unrecognised BT Version_t %d \n",version));
            return bluetooth_unknown;
    }
}


/****************************************************************************
NAME    
    connectionConvertSdsResult

DESCRIPTION
    Convert the Bluestack defined SDS_RESPONSE_T value into the 
    Connection Lib defined sds_status.

RETURNS
    Connection Lib sds_status
*/
sds_status connectionConvertSdsResult(uint16_t result)
{
    switch(result)
    {
        CASE(
                SDS_SUCCESS,
                sds_status_success
            );
        CASE(
                SDS_PENDING,
                sds_status_pending
            );
        CASE(
                SDS_ERROR_UNSPECIFIED,
                sds_status_unspecified_error
            );
        CASE(
                SDS_ERROR_INVALID_SR_DATA,
                sds_status_invalid_sr_data_error
            );
        CASE(
                SDS_ERROR_INVALID_SR_HANDLE,
                sds_status_invalid_sr_handle_error
            );
        CASE(
                SDS_ERROR_INSUFFICIENT_RESOURCES,
                sds_status_insufficient_resources
            );
        coerce: return (sds_status)result;       
        default:
            CL_DEBUG_INFO(("Unrecognised sds_status %d \n",result));
            return sds_status_unknown;
    }
}

/****************************************************************************
NAME    
    connectionConvertL2capConnectStatus

DESCRIPTION
    Convert the Bluestack defined l2ca_conn_result_t value into the 
    Connection Lib defined l2cap_connect_status.

RETURNS
    Connection Lib l2cap_connect_status
*/
l2cap_connect_status connectionConvertL2capConnectStatus(uint16 result)
{
    switch(result)
    {
        case L2CA_CONNECT_SUCCESS:
            return l2cap_connect_success;
        case L2CA_CONNECT_PENDING:
            return l2cap_connect_pending;
        case L2CA_CONNECT_FAILED:
            return l2cap_connect_failed;
        case L2CA_CONNECT_INVALID_CONFTAB:
            return l2cap_connect_failed_invalid_conftab;
        case L2CA_CONNECT_REJ_PSM:
        case L2CA_CONNECT_CONFTAB_EXHAUSTED:
        case L2CA_CONNECT_CONFTAB_UNSUPPORTED:
        case L2CA_CONNECT_CONFIG_MISMATCH:
            return l2cap_connect_failed_config_rejected;
        case L2CA_CONNECT_PEER_ABORTED:
            return l2cap_connect_failed_remote_disc;
        case L2CA_CONNECT_REJ_RESOURCES:
            return l2cap_connect_failed_remote_reject;
        case L2CA_CONNECT_TIMEOUT:
            return l2cap_connect_timeout;
        case L2CA_CONNECT_REJ_SECURITY:
            return l2cap_connect_failed_security;
        case L2CA_CONNECT_PHYS_CONN_TERM_LOCAL_HOST:
            return l2cap_connect_terminated_by_host;
        case L2CA_CONNECT_REJ_CONTROL_ID:
        case L2CA_CONNECT_LINK_LOSS:
        case L2CA_CONNECT_NOT_READY:
        case L2CA_CONNECT_KEY_MISSING:
        case L2CA_CONNECT_PHYS_FAILED:
        case L2CA_CONNECT_INITIATING:
        case L2CA_CONNECT_RETRYING:
        case L2CA_CONNECT_OUT_OF_MEM:
        case L2CA_CONNECT_PHYS_NO_CONNECTION:
        case L2CA_CONNECT_PHYS_INSUFFICIENT_SECURITY:
        case L2CA_CONNECT_HARDWARE:
        case L2CA_CONNECT_PEER_REJECT_CID:
        case L2CA_CONNECT_LOCKSTEP_ERROR:
            CL_DEBUG_INFO(("L2CAP Error: 0x%X\n",result));
            return l2cap_connect_error;
        default:
            CL_DEBUG(("Unrecognised l2ca_conn_result_t 0x%X \n",result));
            return l2cap_connect_unknown;
    }
}

/****************************************************************************
NAME    
    connectionConvertL2capTpConnectStatus

DESCRIPTION
    Convert the Bluestack defined l2ca_tp_conn_result_t value into the 
    Connection Lib defined l2cap_connect_status.

RETURNS
    Connection Lib l2cap_tp_connect_status
*/
l2cap_connect_status connectionConvertL2capTpConnectStatus(uint16 result)
{
    switch(result)
    {
        case L2CA_LE_CONNECT_SUCCESS:
            return l2cap_tp_connect_success;
        case L2CA_CONNECT_PENDING:
            return l2cap_tp_connect_pending;
        case L2CA_CONNECT_FAILED:
            return l2cap_tp_connect_failed;
        case L2CA_CONNECT_INVALID_CONFTAB:
            return l2cap_tp_connect_failed_invalid_conftab;
        case L2CA_LE_CONNECT_REJ_PSM:
        case L2CA_CONNECT_CONFTAB_EXHAUSTED:
        case L2CA_CONNECT_CONFTAB_UNSUPPORTED:
        case L2CA_CONNECT_CONFIG_MISMATCH:
            return l2cap_tp_connect_failed_config_rejected;
        case L2CA_CONNECT_PEER_ABORTED:
            return l2cap_connect_failed_remote_disc;
        case L2CA_LE_CONNECT_REJ_RESOURCES:
            return l2cap_tp_connect_failed_remote_reject;
        case L2CA_CONNECT_TIMEOUT:
            return l2cap_tp_connect_timeout;
        case L2CA_CONNECT_REJ_SECURITY:
            return l2cap_tp_connect_failed_security;
        case L2CA_CONNECT_PHYS_CONN_TERM_LOCAL_HOST:
            return l2cap_tp_connect_terminated_by_host;
        case L2CA_CONNECT_LINK_LOSS:
        case L2CA_CONNECT_NOT_READY:
        case L2CA_CONNECT_KEY_MISSING:
        case L2CA_CONNECT_PHYS_FAILED:
        case L2CA_CONNECT_INITIATING:
        case L2CA_CONNECT_RETRYING:
        case L2CA_CONNECT_OUT_OF_MEM:
        case L2CA_CONNECT_PHYS_NO_CONNECTION:
        case L2CA_CONNECT_PHYS_INSUFFICIENT_SECURITY:
        case L2CA_CONNECT_HARDWARE:
        case L2CA_CONNECT_PEER_REJECT_CID:
        case L2CA_CONNECT_LOCKSTEP_ERROR:
        /* L2CAP LE COC specific values */
        case L2CA_LE_CONNECT_REJ_AUTHENTICATION:
        case L2CA_LE_CONNECT_REJ_AUTHORIZATION:
        case L2CA_LE_CONNECT_REJ_ENC_KEY_SIZE:
        case L2CA_LE_CONNECT_REJ_ENCRYPTION:
        case L2CA_LE_CONNECT_REJ_INVALID_SRC_CID:
        case L2CA_LE_CONNECT_REJ_SRC_CID_INUSE:
            CL_DEBUG_INFO(("L2CAP LE COC Error: 0x%X\n",result));
            return l2cap_tp_connect_error;
        default:
            CL_DEBUG(("Unrecognised l2ca_tp_conn_result_t 0x%X \n",result));
            return l2cap_tp_connect_unknown;
    }
}

/*****************************************************************************/
l2cap_map_connectionless_status connectionConvertL2capMapFixedCidResult(
        uint16 result
        )
{
    switch(result)
    {
        CASE(
                L2CA_MISC_SUCCESS,
                l2cap_map_connectionless_success
            );
        CASE(
                L2CA_MISC_INITIATING,
                l2cap_map_connectionless_initiating
            );
        CASE(
                L2CA_MISC_OUT_OF_MEM,
                l2cap_map_connectionless_out_of_memory
            );
        CASE(
                L2CA_MISC_INVALID_PSM,
                l2cap_map_connectionless_invalid_psm
            );
        CASE(
                L2CA_MISC_INVALID_CID,
                l2cap_map_connectionless_invalid_cid
            );
        CASE(
                L2CA_MISC_FAILED,
                l2cap_map_connectionless_failed
            );
        CASE(
                L2CA_MISC_TIMEOUT,
                l2cap_map_connectionless_timed_out
            );
        CASE(
                L2CA_MISC_ALREADY_REGISTERED,
                l2cap_map_connectionless_already_registered
            );
        coerce: return (sds_status)result;       
        default:
            CL_DEBUG_INFO(("Unrecognised sds_status %d \n",result));
            return l2cap_map_connectionless_unknown;
    }    
}

#ifndef DISABLE_BLE
/*****************************************************************************/
ble_advertising_event_type connectionConvertBleEventType(uint8 event_type)
{
    switch(event_type)
    {
        CASE(
                HCI_ULP_EV_ADVERT_CONNECTABLE_UNDIRECTED,
                ble_adv_event_connectable_undirected
            );
        CASE(
                HCI_ULP_EV_ADVERT_CONNECTABLE_DIRECTED,
                ble_adv_event_connectable_directed
            );
        CASE(
                HCI_ULP_EV_ADVERT_DISCOVERABLE,
                ble_adv_event_discoverable
            );
        CASE(
                HCI_ULP_EV_ADVERT_NON_CONNECTABLE,
                ble_adv_event_non_connectable
            );
        CASE(
                HCI_ULP_EV_ADVERT_SCAN_RESPONSE,
                ble_adv_event_scan_response
            );
        coerce: return (ble_advertising_event_type)event_type;       
        default:
            CL_DEBUG_INFO(("Unknown event_type %d\n", event_type));
            return ble_adv_event_unknown;
    }
}
#endif

/****************************************************************************
NAME    
    connectionConvertAPTRouteEvent

DESCRIPTION
    Convert the connection lib defined APT route event setting into the 
    BlueStack defined APT route event setting.

RETURNS
    Bluestack form of route event setting
*/
DM_SM_APT_ROUTE_T connectionConvertAPTRouteEvent(cl_apt_route_event apt_route_event)
{
    switch(apt_route_event)
    {
        CASE(cl_apt_ignore, DM_SM_APT_IGNORE);
        CASE(cl_apt_bluestack, DM_SM_APT_BLUESTACK);
        CASE(cl_apt_application, DM_SM_APT_APPLICATION);
        coerce: return (DM_SM_APT_ROUTE_T)apt_route_event;       
        default:
            CL_DEBUG(("Unrecognised route event option %d\n", apt_route_event));
            return DM_SM_APT_BLUESTACK;
    }
}

/****************************************************************************
NAME    
    connectionConvertEncryptionType

DESCRIPTION
    Convert the Bluestack defined Encryption Type into the Connection Lib
    defined Encryption type

RETURNS
    Connection Lib Encryption Type
*/
cl_sm_encryption_key_type connectionConvertEncryptionType(uint16 encrypt_type)
{
    switch(encrypt_type)
    {
        CASE(DM_SM_ENCR_NONE, cl_sm_encryption_none);
        /* DM_SM_ENCR_ON_BREDR_E0 = DM_SM_ENCR_ON_LE_AES_CCM */
        CASE(DM_SM_ENCR_ON_LE_AES_CCM, cl_sm_encryption_e0_brdedr_aes_ccm_le);
        CASE(DM_SM_ENCR_ON_BREDR_AES_CCM,  cl_sm_encryption_aes_ccm_bredr);
        coerce: return (cl_sm_encryption_key_type)encrypt_type;       
        default:
            CL_DEBUG(("Unrecognised encryption type %d\n", encrypt_type));
            return cl_sm_encryption_none;
    }
}
