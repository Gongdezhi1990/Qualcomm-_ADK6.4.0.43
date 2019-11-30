/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */
#ifndef CONNECTION_SHIM_LAYER_H
#define CONNECTION_SHIM_LAYER_H

#include <connection.h>


typedef struct
{
    Sink    sink;
    
    uint16  size_data;
    uint8   data[1];
} CL_SYSTEM_MORE_DATA_TEST_EXTRA_T;


typedef struct
{
    hci_status          status; 
    uint16              transport;
    oob_data_setting    oob_data_present;
    uint8               size_oob_data;
    /* Pointer to the start of the OOB data 
     * (hash c then rand r concatenated).
     */
    uint8               oob_data[1];
} CL_SM_READ_LOCAL_OOB_DATA_CFM_TEST_EXTRA_T;

typedef struct
{
    uint16      task;
    bdaddr      bd_addr;            
    uint16      psm;                
    uint16      cid;
    uint16      identifier;     
} CL_L2CAP_CONNECT_IND_TEST_EXTRA_T;


typedef struct
{
    l2cap_connect_status    status;
    uint16                  psm_local;
    Sink                    sink;
    uint16                  connection_id;
    uint16                  mtu_remote;
    uint16                  flush_timeout_remote;
    qos_flow                qos_remote;
    uint8                   flow_mode;
    uint16                  task;
} CL_L2CAP_CONNECT_CFM_TEST_EXTRA_T;

typedef struct
{
    uint8           type;
    bdaddr          bd_addr;        
    bool            incoming;        
    uint32          dev_class;
    hci_status      status;
    uint16          flags;
    uint16          conn_interval;
    uint16          conn_latency;
    uint16          supervision_timeout;
    uint8           clock_accuracy;
} CL_DM_ACL_OPENED_IND_TEST_EXTRA_T;

typedef struct 
{
    uint8           type;
    bdaddr          bd_addr;
    hci_status      status;
} CL_DM_ACL_CLOSED_IND_TEST_EXTRA_T;

typedef struct
{
    uint8           type;
    bdaddr          bd_addr;
    uint16          transport;
} CL_DM_APT_IND_TEST_EXTRA_T;

typedef struct
{
    uint8           type;
    bdaddr          bd_addr;
    uint16          transport;
} CL_SM_USER_PASSKEY_REQ_IND_TEST_EXTRA_T;

typedef struct
{
    uint8           type;
    bdaddr          bd_addr;
    uint16          transport;
    uint32          passkey;
} CL_SM_USER_PASSKEY_NOTIFICATION_IND_TEST_EXTRA_T;


typedef struct
{
    uint8           type;
    bdaddr          bd_addr;
    uint16          transport;
    uint32          numeric_value;
    bool            response_required;
} CL_SM_USER_CONFIRMATION_REQ_IND_TEST_EXTRA_T;

typedef struct
{
    uint8           type;
    bdaddr          bd_addr;
} CL_SM_PIN_CODE_IND_TEST_EXTRA_T;

typedef struct
{
    hci_status      status;        
    Sink            sink;        
    uint8           type;
    bdaddr          bd_addr;
    uint16          transport;
} CL_SM_ENCRYPTION_KEY_REFRESH_IND_TEST_EXTRA_T;

typedef struct
{
    Sink            sink;
    bool            encrypted;
    uint8           type;
    bdaddr          bd_addr;
    uint16          transport;
} CL_SM_ENCRYPTION_CHANGE_IND_TEST_EXTRA_T;

typedef struct
{
    connection_lib_status   status;        
    uint8                   type;
    bdaddr                  bd_addr;
    uint16                  size_psdata;                
    uint8                   psdata[1];                    
} CL_SM_GET_INDEXED_ATTRIBUTE_CFM_TEST_EXTRA_T;

typedef struct
{
    bool                    result;                
} CL_AUTH_SET_PRIORITY_DEVICE_IND_TEST_EXTRA_T;

typedef struct
{
    bool                    result;
    bool                    is_priority_device;
} CL_AUTH_GET_PRIORITY_DEVICE_STATUS_IND_TEST_EXTRA_T;

typedef struct
{
    bool                    result;
} CL_AUTH_IS_PRIORITY_DEVICE_IND_TEST_EXTRA_T;

typedef struct
{
    hci_status              status;
    uint8                   rssi;
    uint8                   type;
    bdaddr                  bd_addr;
    uint16                  transport;
} CL_DM_RSSI_BDADDR_CFM_TEST_EXTRA_T;

typedef struct 
{
    uint8                   type;
    bdaddr                  bd_addr;
    uint16                  transport;
} CL_SM_IO_CAPABILITY_REQ_IND_TEST_EXTRA_T;

typedef struct
{
    uint16                  authentication_requirements;
    uint16                  io_capability;
    bool                    oob_data_present;
    uint8                   type;
    bdaddr                  bd_addr;
    uint16                  transport;
} CL_SM_REMOTE_IO_CAPABILITY_IND_TEST_EXTRA_T;

#ifndef DISABLE_BLE
typedef struct
{
    uint8                           num_reports;         
    ble_advertising_event_type      event_type;
    uint8                           current_addr_type;
    bdaddr                          current_addr;
    uint8                           permanent_addr_type;
    bdaddr                          permanent_addr;
    int8                            rssi;
    uint8                           size_ad_data;
    uint8                           ad_data[1];
}
CL_DM_BLE_ADVERTISING_REPORT_IND_TEST_EXTRA_T;


typedef struct
{
    bool result;
} CL_BLE_ADD_ADVERTISING_FILTER_CFM_TEST_EXTRA_T;

typedef struct
{
    bool result;
} CL_BLE_CLEAR_ADVERTISING_FILTER_CFM_TEST_EXTRA_T;

typedef struct
{
    connection_lib_status   status;
    uint8                   type;
    bdaddr                  bd_addr;
    uint16                  transport;
    uint16                  flags;
} CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_TEST_EXTRA_T;

typedef struct
{
    connection_lib_status   status;  
    uint8                   type;
    bdaddr                  bd_addr;
} CL_DM_BLE_SECURITY_CFM_TEST_EXTRA_T;

typedef struct
{
    connection_lib_status       status;
    ble_local_addr_type         addr_type;
    uint8                       type;
    bdaddr                      bd_addr;
} CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM_TEST_EXTRA_T;
#endif

#define CL_SHIM_MESSAGE_BASE
typedef enum
{
    CL_SYSTEM_MORE_DATA_TEST_EXTRA = CL_MESSAGE_TOP,
    CL_L2CAP_CONNECT_IND_TEST_EXTRA,
    CL_L2CAP_CONNECT_CFM_TEST_EXTRA,
    CL_SM_READ_LOCAL_OOB_DATA_CFM_TEST_EXTRA,
    CL_DM_ACL_OPENED_IND_TEST_EXTRA,
    CL_DM_ACL_CLOSED_IND_TEST_EXTRA,
    CL_DM_APT_IND_TEST_EXTRA,
    CL_SM_USER_PASSKEY_REQ_IND_TEST_EXTRA,
    CL_SM_USER_PASSKEY_NOTIFICATION_IND_TEST_EXTRA,
    CL_SM_USER_CONFIRMATION_REQ_IND_TEST_EXTRA,
    CL_SM_PIN_CODE_IND_TEST_EXTRA,
    CL_SM_ENCRYPTION_KEY_REFRESH_IND_TEST_EXTRA,
    CL_SM_ENCRYPTION_CHANGE_IND_TEST_EXTRA,
    CL_SM_GET_INDEXED_ATTRIBUTE_CFM_TEST_EXTRA,
    CL_AUTH_SET_PRIORITY_DEVICE_IND_TEST_EXTRA, 
    CL_AUTH_GET_PRIORITY_DEVICE_STATUS_IND_TEST_EXTRA, 
    CL_AUTH_IS_PRIORITY_DEVICE_IND_TEST_EXTRA, 
    CL_DM_RSSI_BDADDR_CFM_TEST_EXTRA,
    CL_SM_IO_CAPABILITY_REQ_IND_TEST_EXTRA,
    CL_SM_REMOTE_IO_CAPABILITY_IND_TEST_EXTRA,

#ifndef DISABLE_BLE
    CL_DM_BLE_ADVERTISING_REPORT_IND_TEST_EXTRA,
    CL_BLE_ADD_ADVERTISING_FILTER_CFM_TEST_EXTRA,
    CL_BLE_CLEAR_ADVERTISING_FILTER_CFM_TEST_EXTRA,
    CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_TEST_EXTRA,
    CL_DM_BLE_SECURITY_CFM_TEST_EXTRA,
    CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM_TEST_EXTRA,
#endif

    CL_SHIM_MESSAGE_TOP
} ConnectionShimMessageId;

void ConnectionHandleComplexMessage(Task task, MessageId id, Message message);

void ConnectionL2capConnectRequestTestExtraDefault(
        Task theAppTask,
        const bdaddr *addr,
        uint16 psm_local,
        uint16 psm_remote
        );

void ConnectionL2capConnectRequestTestExtraConftab(
        Task theAppTask,
        const bdaddr *addr,
        uint16 psm_local,
        uint16 psm_remote,
        uint16 size_conftab,
        uint8 *conftab
        );

void ConnectionL2capConnectResponseTestExtraDefault(
        Task theAppTask,
        uint16 task,
        bool response,
        uint16 psm,
        uint16 cid,
        uint8 identifier
        );

void ConnectionL2capConnectResponseTestExtraConftab(
        Task theAppTask,
        uint16 task,
        bool response,
        uint16 psm,
        uint16 cid,
        uint8 identifier,
        uint16 size_conftab,
        uint8 *conftab
        );

void ConnectionRfcommConnectRequestTestExtraDefault(
        Task theAppTask,
        const bdaddr* bd_addr,
        uint16 security_chan,
        uint8 remote_server_chan
        );

void ConnectionRfcommConnectRequestTestExtraParams(
        Task theAppTask,
        const bdaddr* bd_addr,
        uint16 security_chan,
        uint8 remote_server_chan,
        uint16 max_payload_size,
        uint8 modem_signal,
        uint8 break_signal,
        uint16 msc_timeout
        );

void ConnectionRfcommConnectResponseTestExtraDefault(
        Task theAppTask,
        bool response,
        Sink sink,
        uint8 local_server_channel
        );

void ConnectionRfcommConnectResponseTestExtraParams(
        Task theAppTask,
        bool response,
        Sink sink,
        uint8 local_server_channel,
        uint16 max_payload_size,
        uint8 modem_signal,
        uint8 break_signal,
        uint16 msc_timeout
        );

void ConnectionRfcommPortNegRequestTestDefault(
        Task theAppTask,
        Sink sink,
        bool request
        );

void ConnectionRfcommPortNegResponseTestDefault(Task theAppTask, Sink sink);

void ConnectionRfcommPortNegRequestTestExtra(
        Task theAppTask,
        Sink sink,
        bool request,
        uint8 baud_rate,
        uint8 data_bits,
        uint8 stop_bits,
        uint8 parity,
        uint8 parity_type,
        uint8 flow_ctrl_mask,
        uint8 xon,
        uint8 xoff,
        uint16 parameter_mask
        );

void ConnectionRfcommPortNegResponseTestExtra(
        Task theAppTask,
        Sink sink,
        uint8 baud_rate,
        uint8 data_bits,
        uint8 stop_bits,
        uint8 parity,
        uint8 parity_type,
        uint8 flow_ctrl_mask,
        uint8 xon,
        uint8 xoff,
        uint16 parameter_mask
        );


void ConnectionSyncConnectRequestTestExtraDefault(Task theAppTask, Sink sink);

void ConnectionSyncConnectRequestTestExtraParams(
        Task theAppTask,
        Sink sink,
        uint32 tx_bandwidth,
        uint32 rx_bandwidth,
        uint16 max_latency,
        uint16 voice_settings,
        sync_retx_effort retx_effort,
        sync_pkt_type packet_type
        );

void ConnectionSyncConnectResponseTestExtraDefault(
        Task theAppTask,
        const bdaddr* bd_addr,
        bool accept
        );

void ConnectionSyncConnectResponseTestExtraParams(
        Task theAppTask,
        const bdaddr* bd_addr,
        bool accept,
        uint32 tx_bandwidth,
        uint32 rx_bandwidth,
        uint16 max_latency,
        uint16 voice_settings,
        sync_retx_effort retx_effort,
        sync_pkt_type packet_type
        );

void ConnectionSyncRenegotiateTestExtraDefault(Task theAppTask, Sink sink);

void ConnectionSyncRenegotiateTestExtraParams(
        Task theAppTask,
        Sink sink,
        uint32 tx_bandwidth,
        uint32 rx_bandwidth,
        uint16 max_latency,
        uint16 voice_settings,
        sync_retx_effort retx_effort,
        sync_pkt_type packet_type
        );

void ConnectionSdpServiceSearchAttributeRequestTestExtra(
        Task theAppTask,
        const bdaddr *addr,
        uint16 max_attributes,
        uint16 size_search_pattern,
        uint16 size_attribute_list,
        uint16 size_search_attribute_list,
        const uint8 *search_attribute_list
        );

void ConnectionSendSinkDataTestExtra(Sink sink, uint16 size_data, uint8* data);

void ConnectionWriteInquiryAccessCodeTestExtra(
        Task theAppTask,
        uint8 *iac,
        uint16 num_iac
        );

void ConnectionSmIoCapabilityResponseTestExtra(
        uint8               type,
        bdaddr*             bd_addr,
        uint16              transport,
        cl_sm_io_capability io_capability,
        uint16              mitm,
        bool                bonding,
        uint16              key_distribution,
        uint16              oob_setting,
        uint8               size_oob_data,
        uint8*              oob_data
        );

void ConnectionSmIoCapabilityResponseTestExtraDefault(
        bdaddr* bd_addr,
        cl_sm_io_capability io_capability,
        uint16 mitm,
        bool bonding
        );

void ConnectionSendSinkAutoDataTestExtra(Sink sink, uint16 size_data);

void ConnectionEnterDutModeTestExtra(uint8 dummy);

void ConnectionReadRemoteVersionBdaddrTestExtra(
        Task theAppTask,
        uint8 bdaddr_typed,
        const bdaddr * addr
        );

void ConnectionSmAddAuthDeviceTestExtra(
        Task theAppTask,
        const bdaddr *peer_bd_addr,
        uint16 trusted,
        uint16 bonded,
        uint8 key_type,
        uint16 size_link_key,
        const uint8* link_key
        );

#ifndef DISABLE_BLE

void ConnectionBleAddAdvertisingReportFilterTestExtra(
        Task theAppTask,
        ble_ad_type ad_type,
        uint16 interval,
        uint16 size_pattern,
        const uint8 * pattern
        );

void ConnectionBleClearAdvertisingReportFilterTestExtra(
        Task theAppTask,
        uint16 dummy
        );

void ConnectionDmBleSecurityReqTestExtra(
        Task                    theAppTask, 
        uint8                   type, 
        const bdaddr            *addr, 
        ble_security_type       security,
        ble_connection_type     conn_type
        );

void ConnectionSetLinkPolicyTestExtra(uint8 dummy);

void ConnectionDmBleSetAdvertisingParamsReqTestExtraDefault(
        ble_adv_type            adv_type,
        bool                    random_own_address,
        uint8                   channel_map,
        uint16                  adv_interval_min,
        uint16                  adv_interval_max, 
        ble_adv_filter_policy   filter_policy
        );

void ConnectionDmBleSetAdvertisingParamsReqTestExtra(
        bool                    random_own_address,
        uint8                   channel_map,
        bool                    random_direct_address,
        const bdaddr            *bd_addr
        );

void ConnectionL2capConnectionParametersUpdateReqTestExtra(
        uint8                   type,
        const bdaddr            *addr,
        uint16                  min_interval,
        uint16                  max_interval,
        uint16                  latency,
        uint16                  timeout
        );

void ConnectionDmBleSetConnectionParametersReqTestExtra(
        uint16      size_param_arr,
        const uint8 *param_arr
        );

void ConnectionDmBleAddDeviceToWhiteListReqTestExtra(
        uint8 type, 
        const bdaddr *bd_addr
        );

void ConnectionDmBleRemoveDeviceFromWhiteListReqTestExtra(
        uint8 type,
        const bdaddr *bd_addr
        );

void ConnectionDmBleReadWhiteListSizeReqTestExtra(uint8 dummy);

void ConnectionDmBleClearWhiteListReqTestExtra(uint8 dummy);

void ConnectionDmBleConfigureLocalAddressReqTestExtra(
        uint16                  local,
        uint8                   type,
        const bdaddr            *bd_addr    
        );

void ConnectionDmBleConnectionParametersUpdateReqTestExtra(
        Task theAppTask,
        uint8  type,
        bdaddr *bd_addr,
        uint16 min_interval,
        uint16 max_interval,
        uint16 latency,
        uint16 timeout,
        uint16 min_ce_length,
        uint16 max_ce_length
        );        

void ConnectionDmBleAcceptConnectionParUpdateResponseTestExtra(
        bool                accept_update,
        uint8               type,
        const bdaddr        *bd_addr,
        uint16              id,
        uint16              conn_interval_min,
        uint16              conn_interval_max,
        uint16              conn_latency,
        uint16              supervision_timeout
        );

#endif /* DISABLE_BLE */

void ConnectionGetRssiBdaddrTestExtraDefault(
        Task            theAppTask,
        uint8           type,
        const bdaddr    *bd_addr,
        TRANSPORT_T     transport
        );

void ConnectionSmUserConfirmationResponseTestExtra(
        uint8           type,
        const bdaddr*   bd_addr,
        uint16          transport,
        bool            confirm
        );

void ConnectionSmUserPasskeyResponseTestExtra(
        uint8           type,
        const bdaddr*   bd_addr,
        uint16          transport,
        bool            cancelled,
        uint32          passkey
        );

void ConnectionSmSendKeypressNotificationRequestTestExtra(
        uint8               type,
        const bdaddr*       bd_addr,
        uint16              transport,
        uint16              keypress_type
        );

void ConnectionSmEncryptionKeyRefreshTestExtra(
        uint8 type, 
        const bdaddr* bd_addr
        );

void ConnectionSmPinCodeResponseTestExtra(
        uint8           type,
        const bdaddr*   bd_addr,
        uint16          size_pin_code,
        const uint8*    pin_code
        );
        
/* 
 *  B-134381
 */
void ConnectionAuthSetPriorityDeviceTestExtra(
        Task theAppTask,
        const bdaddr *bd_addr,
        bool is_priority_device
        );

/* 
 *  B-185975
 */

void ConnectionAuthGetPriorityDeviceStatusTestExtra(
        Task theAppTask,
        const bdaddr* bd_addr, 
        bool *is_priority_device
        );

void ConnectionAuthIsPriorityDeviceTestExtra(
        Task theAppTask,
        const bdaddr* bd_addr
        );        
        
#endif /* CONNECTION_SHIM_LAYER_H */
