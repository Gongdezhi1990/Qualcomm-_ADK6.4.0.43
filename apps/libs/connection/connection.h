/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    connection.h
    
DESCRIPTION
    Header file for the Connection library.
*/


/*!
\defgroup connection connection
\ingroup vm_libs

\brief    Header file for the Connection library.

\section connection_intro INTRODUCTION
        This file provides documentation for the BlueLab connection library
        API including BLE functions.

@{

*/



#ifndef    CONNECTION_H_
#define    CONNECTION_H_

#ifndef DISABLE_BLE
/* Enable BLE message ids in the ConnectionMessageId enumeration, in
 * connection_no_ble.h
 */
#define ENABLE_BLE_MESSAGES
#endif 


#include "connection_no_ble.h"


#ifndef DISABLE_BLE

#define BLE_AD_PDU_SIZE         31
#define BLE_SR_PDU_SIZE         31

/*!
    \brief Enable or disable Bluetooth low energy (BLE) scanning. 

    \param enable Enable scanning if TRUE, otherwise scanning is disabled. 

    Low energy scanning cannot be enabled if the device is advertising itself.
    ConnectionBleAddAdvertisingReportFilter() can be used to filter the 
    advertising reports sent to the VM by Bluestack.

    This is a BT4.0 only feature.
*/
void ConnectionDmBleSetScanEnable(bool enable);


/*!
    \brief Set parameters for Bluetooth low energy (BLE) scanning.

    To use a random address for scanning, it is necessary to generate it
    calling ConnectionDmBleConfigureLocalAddressReq() or 
    ConnectionDmBleConfigureLocalAddressAutoReq() before this.

    To use the public Bluetooth address after using a random one, 
    it is necessary only to use this, setting random_own_address to FALSE

    \param enable_active_scanning If TRUE SCAN_REQ packets may be sent (default:
           FALSE).
    \param random_own_address If TRUE, then use a Random own address (default:
            FALSE).
    \param white_list_only If TRUE then advertising packets from devices that
           are not on the White List for this device will be ignored (default:
           FALSE).
    \param scan_interval Scan interval in steps of 0.625ms, range 0x0004 
           (2.5 ms) to 0x4000 (10.24 s).
    \param scan_window Scan window in steps of 0.625ms, range 0x0004 (2.5ms) to
           0x4000 (10.24 s). Must be less than or equal to the scan_interval 
           parameter value.

    \return A message of type #CL_DM_BLE_SET_SCAN_PARAMETERS_CFM_T 
    is sent to the  task that initialised the Connection library when operation 
    has completed.

    This is a BT4.0 only feature.
*/
void ConnectionDmBleSetScanParametersReq(
        bool    enable_active_scanning,
        bool    random_own_address,
        bool    white_list_only,
        uint16  scan_interval,
        uint16  scan_window
        );

/*!
    \brief Sent in response to ConnectionDmBleSetScanEnable() to the task that
    intialised the Connection library.

    This is a BT4.0 only message.
*/
typedef struct
{
    connection_lib_status           status;     /*!< Status of the operation */
} CL_DM_BLE_SET_SCAN_PARAMETERS_CFM_T;


/*!
    \brief Set the data to be put in BLE Scan Responses sent by this
    device.

    \param size_sr_data The length of the scan response data, maximum is 31 
           octets.
    \param sr_data pointer to the Scan Response data.

    The Scan Response data is copied. The pointer to the data is not
    freed by this function. A #CL_DM_BLE_SET_SCAN_RESPONSE_DATA_CFM_T message 
    will be sent to the task that initialised the Connection library to indicate 
    the status.

    This is a BT4.0 only feature.
*/
void ConnectionDmBleSetScanResponseDataReq(
        uint8 size_sr_data, 
        const uint8 *sr_data
        );

/*!
    \brief Sent in response to setting data for the BLE Scan Response to
    the task that initialised the Connection library.

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Advertising Data status.*/
    connection_lib_status           status;   
}
CL_DM_BLE_SET_SCAN_RESPONSE_DATA_CFM_T;

/*!
    \brief AD type - to be used when setting Advertising Report Filters 
    (ConnectionBleAddAdvertisingReportFilter()).

    This is a BT4.x only type.

*/
typedef enum
{
    /*! Flags. */
    ble_ad_type_flags                       = 0x01,
    /*! Service - More 16-bit UUIDs available. */
    ble_ad_type_more_uuid16                 = 0x02,
    /*! Service - Complete list of 16-bit UUIDs available. */
    ble_ad_type_complete_uuid16             = 0x03,
    /*! Service - More 32-bit UUIDs available. */
    ble_ad_type_more_uuid32                 = 0x04,
    /*! Service - Complete list of 32-bit UUIDs available. */
    ble_ad_type_complete_uuid32             = 0x05,
    /*! Service - More 128-bit UUIDs available. */
    ble_ad_type_more_uuid128                = 0x06,
    /*! Service - Complete list of 128-bit UUIDs available. */
    ble_ad_type_complete_uuid128            = 0x07,
    /*! Local Name - Shortened local name. */
    ble_ad_type_shortened_local_name        = 0x08,
    /*! Local Name - Complete local name. */
    ble_ad_type_complete_local_name         = 0x09,
    /*! TX Power Level. */
    ble_ad_type_tx_power_level              = 0x0A,
    /*! Simple Pairing optional OOB tags. */
    ble_ad_type_ssp_oob_class_of_device     = 0x0D,
    /*! SSP OOB - Hash C. */
    ble_ad_type_ssp_oob_hash_c              = 0x0E,
    /*! SSP OOB - Rand R (R-192). */
    ble_ad_type_ssp_oob_rand_r              = 0x0F,
    /*! Security Manager TK value. */
    ble_ad_type_sm_tk_value                 = 0x10,
    /*! Security Manager OOB Flags. */
    ble_ad_type_sm_oob_flags                = 0x11,
    /*! Slave Connection Interval range. */
    ble_ad_type_slave_conn_interval_range   = 0x12,
    /*! Service solicitation - List of 16-bit Service UUID. */
    ble_ad_type_service_16bit_uuid          = 0x14,
    /*! Service solicitation - List of 128-bit Service UUID. */
    ble_ad_type_service_128bit_uuid         = 0x15,
    /*! Service Data (16-bit default). */
    ble_ad_type_service_data                = 0x16,
    /*! Public Target Address. */
    ble_ad_type_public_target_address       = 0x17,
    /*! Random Target Address. */
    ble_ad_type_random_target_address       = 0x18,
    /*! Appearance. */
    ble_ad_type_appearance                  = 0x19,
    /*! Advertising interval. */
    ble_ad_type_advertising_interval        = 0x1A,
    /*! LE Bluetooth Device Address. */
    ble_ad_type_bluetooth_device_address    = 0x1B,
    /*! LE Role. */
    ble_ad_type_role                        = 0x1C,
    /*! Simple Pairing Hash C-256. */
    ble_ad_type_simple_pairing_hash_c256    = 0x1D,
    /*! Simple Pairing Randomizer R-256. */
    ble_ad_type_simple_pairing_rand_r256    = 0x1E,
    /*! Service solicitation - List of 32-bit Service UUID. */
    ble_ad_type_service_32bit_uuid          = 0x1F,
    /*! Service Data - 32-Bit UUID. */
    ble_ad_type_service_data_32bit          = 0x20,
    /*! Service Data - 128-Bit UUID. */
    ble_ad_type_service_data_128bit         = 0x21,
    /*! LE Secure Connections Confirmation Value. */
    ble_ad_type_connection_conf_value       = 0x22,
    /*! LE Secure Connections Random Value. */
    ble_ad_type_connection_rand_value       = 0x23,
    /*! Universal Resource Indicator. */
    ble_ad_type_uri                         = 0x24,
    /*! Indoor Positioning, Service v1.0 or later. */
    ble_ad_type_indoor_positioning          = 0x25,
    /*! Transport Discovery Data, Service b1.0 or later. */
    ble_ad_type_transport_discovery_data    = 0x26,
    /*! 3D Information Data, 3D Synchronisation Profile, V1.0 or later. */
    ble_ad_type_3d_information_data         = 0x3d,
    /*! Manufacturer Specific Data. */
    ble_ad_type_manufacturer_specific_data  = 0xFF
} ble_ad_type;

/*!
    \brief Bluetooth Low Energy GAP flags

    If any of the flags is non-zero advertisement data shall contain
    the flags within ::ble_ad_type_flags field.
    
    This is a BT4.0 only type.
*/
/*! LE Limited Discoverable Mode */
#define BLE_FLAGS_LIMITED_DISCOVERABLE_MODE     0x01
/*! LE General Discoverable Mode */
#define BLE_FLAGS_GENERAL_DISCOVERABLE_MODE     0x02
/*! BR/EDR Not Supported */
#define BLE_FLAGS_SINGLE_MODE                   0x04
/*! Simultaneous LE and BR/EDR to Same Device Capable (Controller) */
#define BLE_FLAGS_DUAL_CONTROLLER               0x08
/*! Simultaneous LE and BR/EDR to Same Device Capable (Host) */
#define BLE_FLAGS_DUAL_HOST                     0x10

/*!
    \brief Add a Advertising Report filter so that only Advertising Reports 
    matching the defined criteria are reported to the VM.

    \param ad_type The ble_ad_type data in the report to filter on.
    \param interval The step size within the report data to check for the 
    pattern.
    \param size_pattern The length of the pattern data.
    \param *pattern Pointer to he pattern to look for in the report. The data
    is copied, so if a memory slot is used, the application is responsible for
    freeing it.

    \return TRUE if the filter was added, otherwise FALSE.

    Adding a Filter is an OR operation. If multiple filters are added then if 
    any of those filters is satisfied, the advertising report will be sent to 
    the VM. 

    Filters can be cleared using the ConnectionBleClearAdvertisingReportFilter() 
    function. 

    Filters should be set  before calling ConnectionBleSetScanEnable() to enable 
    scanning. 

    The maximum number of filters that can be added is controlled by the PS Key
    PSKEY_BLE_MAX_ADVERT_FILTERS.

    This is a BT4.0 only feature.

*/
bool ConnectionBleAddAdvertisingReportFilter(
            ble_ad_type ad_type, 
            uint16 interval, 
            uint16 size_pattern, 
            const uint8* pattern
            );


/*!
    \brief Clear all Advertising Report Filters.
    \return TRUE if successful, otherwise FALSE.

    Clears all existing Advertising Report Filters that may have been previously
    added using the ConnectionBleAddAdvertisingReportFilter() function.

    This is a BT4.0 only feature.
*/
bool ConnectionBleClearAdvertisingReportFilter(void);





/*!
    \brief Set the data to be put in BLE Advertising Reports sent by this
    device.

    \param size_ad_data The length of the advertising data, maximum is 31 
           octets.
    \param ad_data pointer to the advertising data.

    The advertising data is copied. The pointer to the advertising data is not
    freed by this function. A #CL_DM_BLE_SET_ADVERTISING_DATA_CFM_T message will 
    be sent to the task that initialised the Connection library to indicate the 
    status.

    This is a BT4.0 only feature.
*/
void ConnectionDmBleSetAdvertisingDataReq(
        uint8 size_ad_data, 
        const uint8 *ad_data
        );


/*!
    \brief Sent in response to setting data for the BLE advertising message to
    the task that initialised the Connection library.

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Advertising Data status.*/
    connection_lib_status           status;   
}
CL_DM_BLE_SET_ADVERTISING_DATA_CFM_T;


/*!
    \brief Enable or disable Bluetooth low energy (BLE) advertising. 

    \param enable Enable advertising if TRUE, otherwise advertising is disabled. 

    Advertising cannot be enabled if the device is scanning. Data to be 
    advertised can be set using ConnectionBleAddAdvertisingReportFilter().

    Initiating a GATT Slave Connection will automatically cause broadcast of 
    the set advertising data.

    This is a BT4.0 only feature.
*/
void ConnectionDmBleSetAdvertiseEnable(bool enable);


/*!
    \brief Advertising Event Type.

    This is a BT4.0 only type.
*/
typedef enum
{
    /*! Connectable Undirected Advert. */
    ble_adv_event_connectable_undirected,
    /*! Connectable Directed Advert. */
    ble_adv_event_connectable_directed,
    /*! Discoverable advert. */
    ble_adv_event_discoverable,
    /*! Non-connectable. */
    ble_adv_event_non_connectable,
    /*! Scan Response. */
    ble_adv_event_scan_response,
    /*! Unknown event type.*/
    ble_adv_event_unknown
} ble_advertising_event_type;


/*
    \brief BLE Advertising Reports received that meet the criteria set by the 
    BLE Advertising Filters (ConnectionBleAddAdvertisingReportFilter(), 
    ConnectionBleClearAdvertisingReportFilter()), when scanning has been 
    enabled using ConnectionBleSetScanEnable(). 

    This message will be received by the task that initialised the Connection 
    library.

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Number of reports in this ind. */
    uint8                           num_reports;         
    /*! What type of advert report has been received. */
    ble_advertising_event_type      event_type;
    /*! Current device address. */
    typed_bdaddr                    current_taddr;
    /*! Permanent device address. */
    typed_bdaddr                    permanent_taddr;
    /*! Received Signal Strength Indication of the advertising message. */
    int8                            rssi;
    /*! Length of advertising data. */
    uint8                           size_advertising_data;
    /*! Advertising data. */
    uint8                           advertising_data[1];
}
CL_DM_BLE_ADVERTISING_REPORT_IND_T;


/*! 
    \brief Advertising policy

   Filter policy to filter advertising packets.
*/
typedef enum
{
    /*! Allow scan and connect request from any */
    ble_filter_none = 0x00,

    /*! Allow scan request from white list only, allow any connect */
    ble_filter_scan_only = 0x01,

    /*! Allow any scan, Allow connect from white list only */
    ble_filter_connect_only = 0x02,

    /*! Allow scan and connect request from white list only */
    ble_filter_both = 0x03

} ble_adv_filter_policy;


/*! 
    \brief Advertising type 

    This is used to determine the packet type that is used for advertising
    when advertising is enabled.        
*/
typedef enum
{
    /*! Connectable Undirected Advertising. */
    ble_adv_ind,
    /*! Direct advert - same as high duty */
    ble_adv_direct_ind,
    /*! High duty cycle direct advertising. */
    ble_adv_direct_ind_high_duty,
    /*! Discoverable advertising. */
    ble_adv_scan_ind,
    /*! Non-connectable advertising. */
    ble_adv_nonconn_ind,
    /*! Low duty cycle direct advertising. */
    ble_adv_direct_ind_low_duty
} ble_adv_type;

/*! 
    \brief  BLE Directed Advertising Parameters 

    This structure contains the Direct Address to advertise through
    when the #ble_adv_type is ble_adv_direct_ind.

    If NULL or address is empty then VM will Panic.    
*/
typedef struct
{
    /*! FALSE for public remote address and TRUE for random address*/
    bool random_direct_address;

    /*! Public or random address to be connected */
    bdaddr direct_addr;

} ble_directed_adv_params_t;

/*! 
    \brief  BLE Undirected Advertising Parameters 

    This structure contains the Advertising interval max and min range and
    filtering policy to employ. These are used when the #ble_adv_type is 
    OTHER than ble_adv_direct_ind.

    For #ble_adv_type
        ble_adv_scan_ind
        ble_adv_nonconn_ind
    the advertising interval max range minimum is 0x00A0. If set less, this 
    value shall be used instead.

    If NULL default values (indicated below) will be used. 
*/
typedef struct 
{
    /*! Minimum advertising interval. 
        Range: 0x0020..0x4000 
        Default: 0x0800 (1.28s) */
    uint16 adv_interval_min;

    /*! Maximum advertising interval. 
        Range: 0x0020..0x4000 
        Default: 0x0800 (1.28s) */
    uint16 adv_interval_max;

    /*! Filter policy  - Default: ble_adv_ind */
    ble_adv_filter_policy filter_policy;  

} ble_undirected_adv_params_t;

/*! 
    \brief  Advertising Parameters 

    The param structure used depends on the #ble_adv_type.

    For bls_adv_direct_ind, the directed_adv element shall be used. For all 
    other #ble_adv_type the undirected_adv element shall be used.
*/
typedef union  
{
    /*! Params specific to undirected advertising */
    ble_undirected_adv_params_t undirect_adv; 

    /*! Params specific for directed adverstising */
    ble_directed_adv_params_t   direct_adv;

} ble_adv_params_t;

/*! Channel Map values. Bit wise OR these values to use one or more channels */
#define  BLE_ADV_CHANNEL_37     0x01
#define  BLE_ADV_CHANNEL_38     0x02
#define  BLE_ADV_CHANNEL_39     0x04
#define  BLE_ADV_CHANNEL_ALL \
                  (BLE_ADV_CHANNEL_37|BLE_ADV_CHANNEL_38|BLE_ADV_CHANNEL_39)

/*!
    \brief Set Advertising parameters for  advertising 

    To use a random address for advertising, it is necessary to generate it 
    calling ConnectionDmBleConfigureLocalAddressReq() or 
    ConnectionDmBleConfigureLocalAddressAutoReq() before this.

    To use the public Bluetooth address after using a random one, 
    it is necessary only to use this, setting random_own_address to FALSE.

    \param adv_type Advertising packet type used for advertising.
    \param random_own_address FALSE for public device address 
    \param channel_map  Advertising channels to be used. At least one should 
    be used. If none are set, all channels shall be used by default.
    \param adv_params undirected or directed advertising specific params. If 
    NULL and #ble_adv_type is 'ble_adv_direct_ind' then the CL will Panic. 
    For other #ble_adv_type, parameters are validated by BlueStack.
   
    \return A #CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM_T message will be received 
    indicating if the advertising params has been set successfully, by
    the task that initialised the Connection library.

    This is a BT4.0 only feature.
*/
void ConnectionDmBleSetAdvertisingParamsReq( 
        ble_adv_type adv_type,
        bool random_own_address,
        uint8  channel_map,
        const ble_adv_params_t *adv_params 
        ); 


/*!
    \brief Sent in response to setting BLE Advertising parameters with the
    ConnectioniDmBleSetAdvertisingParamsReq() function. 

    This message is sent to the task that initialised the Connection library.

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Indicates if setting the advertising parameters was successful. */
    connection_lib_status           status;   
}
CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM_T;


/*!
    \brief The level of security to be applied to a BLE Connection.

    Authenticated connections are, by default, encrypted.
*/
typedef enum
{
    /*! BLE Connection is encrypted. */
    ble_security_encrypted,
    /*! BLE connection is encrypted and bonded. */
    ble_security_encrypted_bonded,
    /*! BLE connection is to be encrypted and authenticated. */
    ble_security_authenticated,
    /*! BLE connection is to be encrypted, authenticated and bonded. */
    ble_security_authenticated_bonded,
    /*! BLE connection encryption is to be refreshed. */
    ble_security_refresh_encryption,
    /*! BLE Security last - should not be used. */
    ble_security_last
} ble_security_type;

/*!
    \brief The BLE connection type.
    
    Similar to the #gatt_connection_type but the BREDR Master connection
    type is not in context here.
*/
typedef enum
{
    /*! BLE Master Directed. */
    ble_connection_master_directed          = 0x01,
    /*! BLE Master Whitelist. */
    ble_connection_master_whitelist         = 0x02,
    /*! BLE Slave Directed. */
    ble_connection_slave_directed           = 0x03,
    /*! BLE Slave Whitelist. */
    ble_connection_slave_whitelist          = 0x04,
    /*! BLE Slave Undirected.*/
    ble_connection_slave_undirected         = 0x05,
    /*! BLE Connection last - should not be used. */
    ble_connection_last
} ble_connection_type;

/*!
    \brief Start security for a Bluetooth low energy (BLE) connection.

    \param theAppTask The client task.
    \param *taddr Pointer to the address of the remote device.
    \param security The required security (ble_security_type).
    \param conn_type The type of BLE Connection (ble_connection_type).

    This will enable the security for an existing BLE connection. If bonding 
    is required then this will occur as part of the security scenario. 
    A CL_DM_BLE_SECURITY_CFM message will be received indicating the outcome.
*/
void ConnectionDmBleSecurityReq(
        Task                    theAppTask, 
        const typed_bdaddr      *taddr, 
        ble_security_type       security,
        ble_connection_type     conn_type
        );    

/*!
    \brief The BLE Security confirm status.
    
    Status returned to security request from app.
*/
typedef enum
{
    /*! BLE Security cfm success. */
    ble_security_success                          = 0x00,
    /*! BLE Security cfm pairing in progress. */
    ble_security_pairing_in_progress              = 0x01,
    /*! BLE Security cfm link key missing */
    ble_security_link_key_missing                 = 0x02,
    /*! BLE Security cfm failed. */
    ble_security_fail                             = 0x03
}ble_security_status;

/*!
    \brief Returned in response to the ConnectionBleDmSecurityReq() function.

    Indicates if the specified security was successfully set.

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Security cfm status.*/
    ble_security_status           status;  
    /*! The remote device address. */
    typed_bdaddr                    taddr;
} CL_DM_BLE_SECURITY_CFM_T;


/*!
    \brief Bluetooth low energy link key distribution flags.

    CSRK (Signing) keys are not yet supported. 

    This is BT4.0 only .
*/
/*! \{ */
/*! No keys - no bonding, only STK is used. */
#define KEY_DIST_NONE                   (0x0000) 
/*! Responder distributes LTK, EDIV and RAND to the Initiator. */
#define KEY_DIST_RESPONDER_ENC_CENTRAL  (0x0100)
/*! Initiator distributes LTK, EDIV and RAND to the Responder. */
#define KEY_DIST_INITIATOR_ENC_CENTRAL  (0x0001)
/*! Responder distributes the IRK to the Initiator. */
#define KEY_DIST_RESPONDER_ID           (0x0200)
/*! Initiator distributes the IRK to the Responder. */
#define KEY_DIST_INITIATOR_ID           (0x0002)
/*! \} */


/*!
    \brief Bluetooth Low Energy connection and advertisment configuration
    parameters
*/
typedef struct
{
    /*! \brief LE scan interval

    The ttime interval from when the Controller started its last LE scan
    until it begins the subsequent LE scan.

    Scan interval in units of 0.625 ms. The allowed range is between
    0x0004 (2.5 ms) and 0x4000 (10240 ms). */
    uint16 scan_interval;
    /*! \brief LE scan window

    Amount of time for the duration of the LE scan. LE Scan Window shall be
    less than or equal to LE Scan Interval.
    
    Scan window in units of 0.625 ms. The allowed range is between
    0x0004 (2.5 ms) and 0x4000 (10.240 s). */
    uint16 scan_window;
    /*! \brief Minimum value for the connection event interval.

    This shall be less than or equal to Conn Interval Max.

    Connection interval in units of 1.25 ms. The allowed range is between
    0x0006 (7.5 ms) and 0x0c80 (4 s). */
    uint16 conn_interval_min;
    /*! \brief Maximum value for the connection event interval.

    This shall be greater than or equal to Conn Interval Min.

    Connection interval in units of 1.25 ms. The allowed range is between
    0x0006 (7.5 ms) and 0x0c80 (4 s). */
    uint16 conn_interval_max;
    /*! \brief Slave latency for the connection in number of connection events.

    The allowed range is between 0x0000 and 0x01f4. */
    uint16 conn_latency;
    /*! \brief Supervision timeout for the LE Link

    Supervision timeout in units of 10 ms. The allowed range is between
    0x000a (100 ms) and 0x0c80 (32 s). */
    uint16 supervision_timeout;
    /*! \brief LE connection attempt timeout

    Equivalent of Page Timeout in BR/EDR. */
    uint16 conn_attempt_timeout;
    /*! \brief Minimum advertising interval for non-directed advertising.

    The maximum allowed slave latency that is accepted if slave requests
    connection parameter update once connected. */
    uint16 conn_latency_max;
    /*! \brief Minimum allowed supervision timeout

    The minimum allowed supervision timeout that is accepted if slave requests
    connection parameter update once connected. */
    uint16 supervision_timeout_min;
    /*! \brief Maximum allowed supervision timeout

    The maximum allowed supervision timeout that is accepted if slave requests
    connection parameter update once connected. */
    uint16 supervision_timeout_max;
    /*! \brief Own Address type used  in LE connnect requests by the device. 
    
    Available types are TYPED_BDADDR_PUBLIC or TYPED_BDADDR_RANDOM.*/
    uint8 own_address_type;
} ble_connection_params;

/*!
    \brief Set default Bluetooth Low Energy connection and advertising
    parameters

    To use a random address for connections, it is necessary to generate it 
    calling ConnectionDmBleConfigureLocalAddressReq() or 
    ConnectionDmBleConfigureLocalAddressAutoReq() before this.

    To use the public Bluetooth address after using a random one, 
    it is necessary only to use this, setting the own_address_type parameter of 
    ble_connection_params with #TYPED_BDADDR_PUBLIC.

    \param params The connection and advertising default parameters.

    This will set the default values for Bluetooth Low Energy connections
    establishment and advertising.

    This is a BT4.0 only feature.
    
    \return Message \link CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM_T
    CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM\endlink is sent
    to the client task when operation has finished.
*/
void ConnectionDmBleSetConnectionParametersReq(
    const ble_connection_params *params);

/*!
    \brief Sent in response to ConnectionDmBleSetConnectionParametersReq().

    This is a BT4.0 only message.
*/
typedef struct
{
    connection_lib_status           status;     /*!< Status of the operation */
} CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM_T;

/*!
    \brief Request to update the BLE connection parameters.

    \param theAppTask The client task.
    \param *taddr Pointer to the address of the master.
    \param min_interval Minimum requested connection interval.
    \param max_interval Maximum requested connection interval.
    \param latency Slave latency.
    \param timeout Supervision timeout.
    \param min_ce_length Minimum length of connection.
    \param max_ce_length Maximum length of connection.
    
    The Connection Parameter Update Request allows the LE peripheral to
    request a set of new connection parameters. The LE central device may 
    reject the request.

    If the requesting device is Central then the connection changes will be 
    carried out locally.

    For more information about the parameters see documentation for
    ble_connection_params.

    This is a BT4.0 only feature.

    \return Message \link CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM_T
    CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM\endlink is sent
    to the client task when operation has finished.
*/
void ConnectionDmBleConnectionParametersUpdateReq(
        Task theAppTask,
        typed_bdaddr *taddr,
        uint16 min_interval,
        uint16 max_interval,
        uint16 latency,
        uint16 timeout,
        uint16 min_ce_length,
        uint16 max_ce_length
        );

/*!
    \brief Sent in response to ConnectionDmBleConnectionParametersUpdateReq().

    This is a BT4.0 only message.
*/
typedef struct
{
    typed_bdaddr            taddr;  /*!< The address of the master. */
    connection_lib_status   status; /*!< Status of the operation */
} CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM_T;


/*!
    \brief Read the total size of the BLE device White List.

    This is a BT4.0 only feature.

    \return Message #CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM_T is sent in response.
*/
void ConnectionDmBleReadWhiteListSizeReq(void);

/*!
    \brief Sent in response to ConnectionDmBleReadWhitListSizeReq().
 
    This is a BT4.0 only message.
*/ 
typedef struct
{
    /*! Status of request. */
    connection_lib_status           status;
    /*! Total size of entries that can be stored in the controller. */
    uint8                           white_list_size;
} CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM_T;


/*! 
    \brief Clear the BLE Device White List.

    Clears the White List of devices stored in the controller. This command
    will fail in the following scenarios:
    - Advertising is enabled and the advertising filter policy uses the white 
      List.
    - Scanning is enables and the scanning filter policy uses the white list.
    - The initiator filter policy uses the white list and a BLE Connection is
      being created.

    This is a BT4.0 only feature.

    \return Message #CL_DM_BLE_CLEAR_WHITE_LIST_CFM_T is sent in response.
*/
void ConnectionDmBleClearWhiteListReq(void);

/*! 
    \brief Sent in response to ConnectionDmBleClearWhiteListReq().

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Status of request. */
    connection_lib_status       status;
} CL_DM_BLE_CLEAR_WHITE_LIST_CFM_T;


/*! 
    \brief Add a device to the BLE White List.

    Add a single device to the BLE White list stored in the controller.
    This command will fail in the following scenarios:
    - Advertising is enabled and the advertising filter policy uses the white 
      List.
    - Scanning is enables and the scanning filter policy uses the white list.
    - The initiator filter policy uses the white list and a BLE Connection is
      being created.
    
    \param bd_addr_type Device address type, either #TYPED_BDADDR_PUBLIC or
    #TYPED_BDADDR_RANDOM (bdaddr_.h).
    \param *bd_addr The device bluetooth address.

    This is a BT4.0 only feature.

    \return Message #CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM_T is sent in 
    response.
*/
void ConnectionDmBleAddDeviceToWhiteListReq(
        uint8 bd_addr_type, 
        const bdaddr *bd_addr
        );

/*! 
    \brief Sent in response to ConnectionDmBleAddDeviceToWhiteListReq().

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Status of request. */
    connection_lib_status       status;
} CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM_T;

/*! 
    \brief Remove a device from the BLE White List.

    Remove a single device from the BLE White list stored in the controller.
    This command will fail in the following scenarios:
    - Advertising is enabled and the advertising filter policy uses the white 
      List.
    - Scanning is enables and the scanning filter policy uses the white list.
    - The initiator filter policy uses the white list and a BLE Connection is
      being created.
    
    \param bd_addr_type Device address type, either TYPED_BDADDR_PUBLIC or
    TYPED_BDADDR_RANDOM (bdaddr_.h).
    \param *bd_addr The device bluetooth address.

    This is a BT4.0 only feature.

    \return Message #CL_DM_BLE_REMOVE_DEVICE_FROM_WHITE_LIST_CFM_T is sent in
    response.
*/
void ConnectionDmBleRemoveDeviceFromWhiteListReq(
        uint8 bd_addr_type, 
        const bdaddr *bd_addr
        );

/*! 
    \brief Sent in response to ConnectionDmBleRemoveDeviceToWhiteListReq().

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Status of request. */
    connection_lib_status       status;
} CL_DM_BLE_REMOVE_DEVICE_FROM_WHITE_LIST_CFM_T;


/*! 
    \brief Add devices in the Trusted Device List to the low energy white-
    list.

    Devices in the non-volatile Trusted Device List (TDL) will be added to
    the Bluetooth low energy (BLE) white list. 

    \param ble_only_devices If TRUE, then only devices with a BLE link key type
    will be added to the white-list. If FALSE, then all devices in the TDL will
    be added to the white-list.

    This is a BT4.0 only feature.

    \return For each device added to the white-list, the task which initialised
    the Connection library will receive a 
    #CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM_T message.
*/
void ConnectionDmBleAddTdlDevicesToWhiteListReq(bool ble_only_devices);

/*! 
    \brief Sent in to indicate that a Secure Simple Pairing procedure has 
    completed. 

    This message is only sent for a BLE link.

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! Status of Pairing procedure. */
    connection_lib_status       status;
    /*! Address of the remote Bluetooth Device. */
    tp_bdaddr                   tpaddr;
    /*! Flags. */
    uint16                      flags;
    /*! The remote device permanent address when using a Resolvable Random
        address.
     */
    typed_bdaddr                permanent_taddr; /* TODO: I will get to this. */
} CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T;

/*! 
    \brief Check the link keys stored in the Paired Device List, to see if
    an IRK key has been stored, indicating that this device has bonded to 
    another that is using the BLE Privacy feature.

    The IRK key is used for BLE Privacy. If this key has been stored, then 
    it indicates that at least one bonded device is using the BLE Privacy 
    feature. This may change the App behaviour in regard to whether it uses
    a whitelist to filter connection adverts from peripheral devices. 

    This is only applicable to BLE connections.

    This is a BT4.0 only feature.

*/
bool ConnectionBondedToPrivacyEnabledDevice(void);

/*!
    \brief Defaults for BLE Resolvable Private Address regeneration timer.
*/
/*! \{ */
/*! Minimum RPA TGAP Time out - 1 s  */
#define BLE_RPA_TIMEOUT_MINIMUM     (0x0001)  
/*! Default RPA TGAP Time out - 900 s = 15 m */
#define BLE_RPA_TIMEOUT_DEFAULT     (0x0384)
/*! Maximum RPA TGAP Time out - 41400 s = ~11.5 h */
#define BLE_RPA_TIMEOUT_MAXIMUM     (0xA1B8)
/*! \} */

/*! 
    \brief Permanent Address Type to configure

    Used with the ConnectionDmBleConfigureLocalAddressReq() function.
*/
typedef enum
{
    /*! Use the specified static address. */
    ble_local_addr_write_static,
    /*! Generate a static address. */
    ble_local_addr_generate_static,
    /*! Generate non-resolvable address. */
    ble_local_addr_generate_non_resolvable,
    /*! Generate a resolvable address. */
    ble_local_addr_generate_resolvable,
    
    /*! Always the last enum in this type - do not use. */
    ble_local_addr_last
} ble_local_addr_type;

/*!
    \brief Configure the local address to be used in connections.

    If the option to generate an address is used, the address generated by
    Bluestack will be returned in the 
    #CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM_T message returned in response.

    If the ble_local_addr_type is \link ble_local_addr_generate_resolvable 
    \endlink then a Tgap Timeout of #BLE_RPA_TIMEOUT_DEFAULT (15 m) will be
    used to regenerate the RPA. The timer will only start when the generated
    RPA is first used for advertising or scanning.

    This has to be used to generate a random address for use with BLE.
    To configure the generated address after this, it is necessary to use
    ConnectionDmBleSetConnectionParametersReq() for connections,
    ConnectionDmBleSetScanParametersReq() for scanning and
    ConnectionDmBleSetAdvertisingParamsReq() for advertising.

    \param addr_type The address type to configure see the 
    \link ble_local_addr_type \endlink.

    \param static_taddr The address to use, when the option \link
    ble_local_addr_write_static \endlink is used. Setting this parameter to
    0 will pass a null address.

    This is only applicable to BLE connections.

    This is a BT4.0 only feature.

*/
void ConnectionDmBleConfigureLocalAddressReq(
        ble_local_addr_type     addr_type,
        const typed_bdaddr*     static_taddr
        );

/*!
    \brief Configure the local address to be used in connections with a timer
    to regenerate Resolvable Private Addresses (RPA), if that type is selected.

    If the option to generate an address is used, the address generated by
    Bluestack will be returned in the 
    #CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM_T message returned in response.

    If the ble_local_addr_type is \link ble_local_addr_generate_resolvable 
    \endlink then a Tgap Timeout will be used to regenerate the RPA. The timer 
    will only start when the generated RPA is first used for advertising, 
    scanning or connection.

    This has to be used to generate a random address for use with BLE.
    To configure the generated address after this, it is necessary to use
    ConnectionDmBleSetConnectionParametersReq() for connections,
    ConnectionDmBleSetScanParametersReq() for scanning and
    ConnectionDmBleSetAdvertisingParamsReq() for advertising.

    \param addr_type The address type to configure see the 
    \link ble_local_addr_type \endlink.

    \param static_taddr The address to use, when the option \link
    ble_local_addr_write_static \endlink is used. Setting this parameter to
    0 will pass a null address.

    \param rpa_tgap_timeout The time, in seconds, after which an RPA will be
    regenerated, if that address type is requested. The timeout must be in the 
    range #BLE_RPA_TIMEOUT_MINIMUM (30 s) to #BLE_RPA_TIMEOUT_MAXIMUM (~11.5 h).

    This is only applicable to BLE connections.
*/

void ConnectionDmBleConfigureLocalAddressAutoReq(
        ble_local_addr_type     addr_type,
        const typed_bdaddr*     static_taddr,
        uint16                  rpa_tgap_timeout
        );

/*! 
    \brief Sent in response to ConnectionDmBleConfigureLocalAddressReq().

    If the status indicates success then the 'random_taddr' field is the 
    device address that will be used for BLE connections.

    This is a BT4.0 only message.
*/
typedef struct 
{
    /*! Status of configuring the local device address. */
    connection_lib_status       status;
    /*! local address type that has been configured. */
    ble_local_addr_type         addr_type;
    /*! The random address that will be used (if status is 'success') */
    typed_bdaddr                random_taddr;
} CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM_T;

/*! 
    \brief Flags used to identify whether it is the local or peer device
    random address to read when calling the 
    ConnectionSmBleReadRandomAddressReq() function.

    These enum values represent bit flags.
*/
typedef enum {
    /*! Read the Random Address configured for the local device. */
    ble_read_random_address_local = 0x0001,
    /*! Read the Random Address of a peer device. */
    ble_read_random_address_peer  = 0x0002
} ble_read_random_address_flags;

/*! 
    \brief Read Random Address of either the local device or a peer device.

    \param flags Indicate whether it is the local device or a peer device, 
    that the Read request is for. See #ble_read_random_address_flags.

    \param peer_tpaddr Transport typed Bluetooth Device Address of the Peer 
    device. If the request is to read the Local device's Random address, this 
    will be ignored.
*/
void ConnectionSmBleReadRandomAddressReq(
    ble_read_random_address_flags   flags,
    const tp_bdaddr                 *peer_tpaddr
    );

/*!
    \brief Received in response to ConnectionSmBleReadRandomAddressReq().
*/
typedef struct
{
    /*! Result status of the request */
    connection_lib_status           status;
    /*! Peer device address, as provided in the original request. */
    tp_bdaddr                       peer_tpaddr;
    /*! Flags from the request, indicating if it was for local or peer device.*/
    ble_read_random_address_flags   flags;
    /*! Local or peer device Random Address, depending on request flags. */
    tp_bdaddr                       random_tpaddr;
} CL_SM_BLE_READ_RANDOM_ADDRESS_CFM_T;

/*! 
    \brief Sent in when a BLE Update to connection parameters is sent from
    a remote device. 

    The application must respond to accept (or reject) the update to connection 
    parameters using the ConnectionDmBleAcceptConnectionParUpdateResponse() 
    function.

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! The remote device address. */
    typed_bdaddr    taddr;
    /*! L2CAP signal identifier of the connection. */
    uint16          id;             
    /*! The minimum allowed connection interval. */
    uint16          conn_interval_min;
    /*! The maximum allowed connection interval. */
    uint16          conn_interval_max;
    /*! The connection slave latency. */
    uint16          conn_latency;
    /*! Link supervision time out. */
    uint16          supervision_timeout;
} CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T;

/*! 
    \brief Sent when a BLE Connection Update command was issued
    by the Host or if the connection parameters are updated following a request
    from the peer device. If no parameters are updated following a request from
    the peer device then this event shall not be issued

    This is a BT4.0 only message.
*/
typedef struct
{
    /*! The remote device address. */
    typed_bdaddr    taddr;
    /*! L2CAP signal identifier of the connection. */
    uint16          id;             
    /*! The status of request. */
    uint16          status;
    /*! The negotiated connection interval. */
    uint16          conn_interval;
    /*! The connection slave latency. */
    uint16          conn_latency;
    /*! Link supervision time out. */
    uint16          supervision_timeout;
} CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T;

/*!
    \brief Used to accept (or reject) an update to the parameters of a 
    BLE connection from a remote device.

    This function should be called in response to receiving a \link
    CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T \endlink message.

    \param accept_update TRUE to accept the parameter updates.
    \param taddr The remote device address. 
    \param id The L2CAP signal identifier as indicated in the IND message.
    \param conn_interval_min The minimum allowed connection interval.
    \param conn_interval_max The maximum allowed connection interval.
    \param conn_latency The connection slave latency.
    \param supervision_timeout Link supervision time out.

    This is only applicable to BLE connections.

    This is a BT4.0 only feature.
*/
void ConnectionDmBleAcceptConnectionParUpdateResponse(
        bool                accept_update,
        const typed_bdaddr  *taddr,
        uint16              id,
        uint16              conn_interval_min,
        uint16              conn_interval_max,
        uint16              conn_latency,
        uint16              supervision_timeout
        );

/*! 
    \brief Indication received when BLE Advertising parameters have been 
    updated. 

    NOTE: The application should only consider parameters relevant to the 
    advertising type and ignore the others.

    This is a BT4.2 only message.
*/
typedef struct
{
    /*! Minimum advertising interval.  Range: 0x0020..0x4000,
        Default: 0x0800 (1.28s) */
    uint16          adv_interval_min;
    /*! Maximum advertising interval.  Range: 0x0020..0x4000,
        Default: 0x0800 (1.28s) */
    uint16          adv_interval_max;
    /*! Advertising type. */
    ble_adv_type    advertising_type;
    /*! Own address type: #TYPED_BDADDR_PUBLIC or #TYPED_BDADDR_RANDOM. */
    uint8           own_address_type;
    /*! Direct address type: #TYPED_BDADDR_PUBLIC or #TYPED_BDADDR_RANDOM. */
    uint8           direct_address_type;
    /*! Directed advertising Bluetooth device address. */
    bdaddr          direct_bd_addr;
    /*! Advertising channel map. */
    uint8           advertising_channel_map;
    /*! Advertising filter policy. */
    uint8           advertising_filter_policy;
} CL_DM_BLE_ADVERTISING_PARAM_UPDATE_IND_T;

/****************************************************************************
NAME
    ConnectionDmBleCheckTdlDeviceAvailable

FUNCTION
    Check in the TDL if any LE device is paired
RETURNS

*/
bool ConnectionDmBleCheckTdlDeviceAvailable(void);

/*! 
    @brief Send to the task that initialised the connection library to confirm
    that the slave latency has changed.
*/
typedef struct
{
    /*! The remote device address. */
    tp_bdaddr       taddr;
    /*! Request was to Enable/disable zero LE slave latency */
    bool            zero_latency;           
    /*! HCI status code. Success or failure*/
    hci_status      status;                                          
} CL_DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM_T;

/*!
    \brief Enable or disable BLE slave latency

    \param taddr The remote device address.     
    \param zero_latency Request zero latency if TRUE, otherwise disabled. 

    Send a Zero Slave Latency request

    This is a BT4.0 only feature.
*/
void ConnectionDmUlpEnableZeroSlaveLatency(
        const tp_bdaddr  *tpaddr,
        bool  zero_latency);

/*! \brief Bluetooth Low Energy Channel Selection Algorithm employed for 
    a connection.

    Values 0x02 to 0xFF are reserved for future use.
*/
typedef enum
{
    /*! BLE Channel Selection Algorithm #1. */
    channel_selection_algorithm_1 = 0x00,
    /*! BLE Channel Selection Algorithm #2. */
    channel_selection_algorithm_2 = 0x01,
    /*! Invalid value indicated from Bluestack. */
    channel_selection_algorithm_invalid
} channel_selection_algorithm_t;
                    

/*!
    \brief Indicates which BLE Channel Selection Algorithm has been used
    by the link controller.

    This indication should follow the CL_DM_ACL_OPENED_IND indication message.
    The application may use this information to enable specific services, such
    as isochronous services.
*/
typedef struct 
{
    /*! The remote device address. */
    tp_bdaddr                       tpaddr;
    /*! The channel selection algorithm used in connection. */
    channel_selection_algorithm_t   selected_algorithm;
} CL_DM_BLE_CHANNEL_SELECTION_ALGORITHM_IND_T;


/*!
    \brief Set PHY preferences for a given LE connection.  

    \param tpaddr The remote device address.     
    \param min_tx Minimum preferred tx rate
    \param max_tx Maximum preferred tx rate
    \param min_rx Minimum preferred rx rate
    \param max_rx Minimum preferred rx rate
    \param flags  Reserved for future use, set to zero 
    
    Actual PHY parameters selected will be indicated in CM_DM_ULP_SET_PHY_CFM
*/
void ConnectionDmUlpSetPhy(
        const tp_bdaddr  *tpaddr,
        const uint8 min_tx,
        const uint8 max_tx,
        const uint8 min_rx,
        const uint8 max_rx,
        const uint8 flags
        );

/*!
    \brief Set default PHY preferences for all upcoming
    connections after this request is processed successfully

    \param min_tx Minimum preferred tx rate
    \param max_tx Maximum preferred tx rate
    \param min_rx Minimum preferred rx rate
    \param max_rx Minimum preferred rx rate
    \param flags  Reserved for future use, set to zero 
*/
void ConnectionDmUlpSetDefaultPhy( 
        const uint8 min_tx,
        const uint8 max_tx,
        const uint8 min_rx,
        const uint8 max_rx,
        const uint8 flags
        );        
        
/*! \brief Valid PHY rate values that can be used depending on an 
    application's PHY preferences for a connection or as default 
    preference
*/
typedef enum
{
    phy_rate_min    = 0x00,
    /* Coded PHY, S=8 */
    phy_rate_125K   = 0x01,
    /* Coded PHY, S=2 */
    phy_rate_500K   = 0x02,
    /* Uncoded PHY */
    phy_rate_1M     = 0x03,
    /* Uncoded PHY */
    phy_rate_2M     = 0x04,
    phy_rate_MAX    = 0xff
} phy_rate;


/*! \brief Valid PHY type values that are used to indicate
    PHY type of a link.
 */
typedef enum
{
    /* Uncoded PHY */
    phy_type_1M    = 0x01,
    /* Uncoded PHY */
    phy_type_2M    = 0x02,
    /* Coded PHY */
    phy_type_coded = 0x03
} phy_type;


/*!
    \brief Indicates a change in PHY of a connection

*/
typedef struct 
{
    /*! The remote device address. */
    tp_bdaddr       tpaddr;
    /*! updated TX PHY type of the connection */
    phy_type        tx_phy_type;  
    /*! updated RX PHY type of the connection */
    phy_type        rx_phy_type;  
} CL_DM_ULP_PHY_UPDATE_IND_T;



/*!
    \brief Indicates a confirmation of a PHY change

*/
typedef struct 
{
    /*! The remote device address. */
    tp_bdaddr       tpaddr;
    /*! updated TX PHY type of the connection */
    phy_type        tx_phy_type;  
    /*! updated RX PHY type of the connection */
    phy_type        rx_phy_type;  
    /*! hci status, non zero is failure */
    hci_status      status;
} CL_DM_ULP_SET_PHY_CFM_T;

/*!
    \brief Indicates a confirmation of a default 
    PHY change

*/
typedef struct 
{
    /*! hci status, non zero is failure */
    hci_status      status;
} CL_DM_ULP_SET_DEFAULT_PHY_CFM_T;

/*!
    \brief Request to read the BLE Advertising channel transmit power

    \param theAppTask The client task.

    A #CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM_T message will be sent
    in response.
*/
void ConnectionDmBleReadAdvertisingChannelTxPower(Task theAppTask);

/*!
    \brief The BLE Advertising Channel Transmit Power, as requested.
*/
typedef struct 
{
    /*! Advetising Channel Transmit power. */
    int8 tx_power;
    /*! hci status code. */
    hci_status      status;                                          
} CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM_T;

/*!
    \brief Received in response to ConnectionBleTransmitterTest().
*/
typedef struct
{
    hci_status      status;
} CL_DM_BLE_TRANSMITTER_TEST_CFM_T;

/*!
    \brief Received in response to ConnectionBleReceiverTest().
*/
typedef struct
{
    hci_status      status;
} CL_DM_BLE_RECEIVER_TEST_CFM_T;

/*!
    \brief Received in response to ConnectionBleTestEnd().
*/
typedef struct
{
    hci_status      status;
    uint16          number_of_rx_packets;
} CL_DM_BLE_TEST_END_CFM_T;

#endif /* DISABLE_BLE */

#endif    /* CONNECTION_H_ */

/** @} */

