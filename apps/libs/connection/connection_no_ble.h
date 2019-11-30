/****************************************************************************
Copyright (c) 2011 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    connection_no_ble.h
    
DESCRIPTION
    Header file for the Connection library.
*/


/*!
@file    connection.h
@brief    Header file for the Connection library.

        This file provides documentation for the connection library
        API without BLE functions.
*/

#ifndef    CONNECTION_NO_BLE_H_
#define    CONNECTION_NO_BLE_H_


#include <bdaddr_.h>
#include <library.h>
#include <message.h>
#include <sink_.h>
#include <csb_.h>

/*! \name Connection attempt timeouts.

    Once a connection has been initiated this timeout is set and if the
    connection has not been established by the time this expires, then the
    connect attempt is terminated.
*/

/*!
    @brief Default timeout for L2CAP connection establishment.
    Set this value greater than RTX+ERTX Timeout. To avoid any race with 
    Bluestack Timeout Indication, set this value greater than 120s
    Otherwise Connection Library always Time Out exactly with RTX Timeout and 
    disconnects the connection. This will trigger only when Bluestack is not 
    responding for L2CAP Connection.
*/
#define    DEFAULT_L2CAP_CONNECTION_TIMEOUT    (125)
/*! \} */


/*! \name Default L2CAP Local MTU Maximum

    The L2CAP Default MTU Local is 672 but the maximum local MTU when using 
    streams is 895.
*/
/*!\{ */
/*!
    @brief Default L2CAP Local MTU Maximum
*/
#define DEFAULT_L2CAP_LOCAL_MTU_MAXIMUM 895

/*! \} */


/*! \name RFCOMM Modem Signal bits. 

    See the 3GPP TS 07.10 specification for details
*/

/*! \{   */
/*! 
    @brief Modem Signal Flow Control (FC) - set when the device is unable to 
    accept frames.
*/
#define RFC_MS_FC_BIT   0x02
/*! 
    @brief Modem Signal Ready To Communicate (RTC) - set when the device is 
    ready to communicate.
*/
#define RFC_MS_RTC_BIT  0x04
/*! 
    @brief Modem Signal Ready To Receive (RTR) - set when the device is 
    ready to receive data.
*/
#define RFC_MS_RTS_BIT  0x08
/*! 
    @brief Modem Signal Incoming Call (IC) - set to indicate an incoming call.
*/
#define RFC_MS_IC_BIT   0x40
/*! 
    @brief Modem Signal Data Valid (DV) - set to indicate valid data is being 
    sent.
*/
#define RFC_MS_DV_BIT   0x80
/*! \} */


/*! \name RFCOMM parameter defaults

    Default values to be used with RFCOMM Modem Status.
*/
/*!\{ */
/*!
    @brief Default Modem Signal
*/
#define RFCOMM_DEFAULT_MODEM_SIGNAL \
    (RFC_MS_DV_BIT|RFC_MS_RTS_BIT|RFC_MS_RTC_BIT)

/*!
    @brief Default Break Signal - 0-seconds
*/
#define RFCOMM_DEFAULT_BREAK_SIGNAL 0

/*!
    @brief Default MSC Timeout in milli-seconds 
    This is used during the connection to wait for the remote side to send a 
    Modem Status Indication. If it is not received then the RFCOMM connection 
    proceeds without it.
*/
#define RFCOMM_DEFAULT_MSC_TIMEOUT      500

/*!
    @brief Default payload size for an RFCOMM connection. This is the optimal
    size based on RFCOMM packet, header and buffer size within the Firmware.
*/
#define RFCOMM_DEFAULT_PAYLOAD_SIZE     884

/*! \} */

/*! \name RFCOMM Port Negotiation Baud Rate values */
/*!\{ */
#define RFCOMM_2400_BAUD        0x00    /*!< 2400 Baud */
#define RFCOMM_4800_BAUD        0x01    /*!< 4800 Baud */
#define RFCOMM_7200_BAUD        0x02    /*!< 7200 Baud */
#define RFCOMM_9600_BAUD        0x03    /*!< 9600 Baud */
#define RFCOMM_19200_BAUD       0x04    /*!< 19200 Baud */
#define RFCOMM_38400_BAUD       0x05    /*!< 38400 Baud */
#define RFCOMM_57600_BAUD       0x06    /*!< 57600 Baud */
#define RFCOMM_115200_BAUD      0x07    /*!< 125200 Baud */
#define RFCOMM_230400_BAUD      0x08    /*!< 230400 Baud */
#define RFCOMM_UNKNOWN_BAUD     0xFF    /*!< Unknown Baud */
/*! \} */

/*! \name RFCOMM Port Negotiation Data bit values */
/*!\{ */
#define RFCOMM_DATA_BITS_5          0x00       /*!< 5 data bits */
#define RFCOMM_DATA_BITS_6          0x02       /*!< 6 data bits */
#define RFCOMM_DATA_BITS_7          0x01       /*!< 7 data bits */
#define RFCOMM_DATA_BITS_8          0x03       /*!< 8 data bits */
/*! \} */

/*! \name RFCOMM Port Negotiation Stop Bits values */
/*!\{ */
#define RFCOMM_STOP_BITS_ONE             0x00  /*!< 1 stop bit    */
#define RFCOMM_STOP_BITS_ONE_AND_A_HALF  0x01  /*!< 1.5 stop bits */
/*! \} */

/*! \name RFCOMM Port Negotiation Parity values */
/*!\{ */
#define RFCOMM_PARITY_OFF           0x00    /*!< Parity Off */
#define RFCOMM_PARITY_ON            0x01    /*!< Parity On */
/*! \} */

/*! \name RFCOMM Port Negotiation Parity Types values */
/*!\{ */
#define RFCOMM_PARITY_TYPE_ODD      0x00    /*!< Odd Parity */ 
#define RFCOMM_PARITY_TYPE_MARK     0x01    /*!< Match Parity */
#define RFCOMM_PARITY_TYPE_EVEN     0x02    /*!< Even Parity */
#define RFCOMM_PARITY_TYPE_SPACE    0x03    /*!< Space Parity */
/*! \} */

/*! \name RFCOMM Port Negotiation Flow Control bit masks */
/*!\{ */
#define RFCOMM_FLC_XONXOFF_INPUT    0x01
#define RFCOMM_FLC_XONXOFF_OUTPUT   0x02
#define RFCOMM_FLC_RTR_INPUT        0x04
#define RFCOMM_FLC_RTR_OUTPUT       0x08
#define RFCOMM_FLC_RTC_INPUT        0x10
#define RFCOMM_FLC_RTC_OUTPUT       0x20
/*! \} */

/*! \name RFCOMM Port Negotiation Xon/Xoff Characters */
/*!\{ */
#define RFCOMM_XON_CHAR_DEFAULT     0x11  /*!< DC1 character */
#define RFCOMM_XOFF_CHAR_DEFAULT    0x13  /*!< DC3 character */
/*! \} */

/*! \name RFCOMM Port Negotiation Parameter Mask bit values */
/*!\{ */
#define RFCOMM_PM_BIT_RATE          0x0001
#define RFCOMM_PM_DATA_BITS         0x0002
#define RFCOMM_PM_STOP_BITS         0x0004
#define RFCOMM_PM_PARITY            0x0008
#define RFCOMM_PM_PARITY_TYPE       0x0010
#define RFCOMM_PM_XON               0x0020
#define RFCOMM_PM_XOFF              0x0040
#define RFCOMM_PM_XONXOFF_INPUT     0x0100
#define RFCOMM_PM_XONXOFF_OUTPUT    0x0200
#define RFCOMM_PM_RTR_INPUT         0x0400
#define RFCOMM_PM_RTR_OUTPUT        0x0800
#define RFCOMM_PM_RTC_INPUT         0x1000
#define RFCOMM_PM_RTC_OUTPUT        0x2000
/*! \} */


/*! \name Bluestack Key Value Pair macros
    
    The Bluestack Key / Value table for L2CAP configuration is constructed from
    an array of uint16 values. These macros help format values to uint16.
*/


/*! \{ */
/*!
    @brief Returns a uint16 of 'value'.
*/
#define BKV_UINT16V(value)  ((uint16)((value)&0xFFFF))
/*!
    @brief Get the high uint16 of a uint32 bit value
*/
#define BKV_UINT32H(value)  ((uint16)(((uint32)(value) >> 16) & 0xFFFF))
/*!
    @brief Get the low uint16 of a uint32 bit value
*/
#define BKV_UINT32L(value)  BKV_UINT16V(value)
/*!
    @brief Turn a uint32 value into 2, comma separated uint16 values in 
    big-endian order (high uint16, low uint16).
*/
#define BKV_UINT32V(value)    BKV_UINT32H(value),BKV_UINT32L(value) 
/*!
    @brief Specify a uint16 range in the order most preferred value, least 
    preferred value, results in two comma separated uint16 values.
 */
#define BKV_UINT16R(most_preferred,least_preferred) \
    BKV_UINT16V(least_preferred), BKV_UINT16V(most_preferred)
/*!
    @brief Specify a uint32 range in the order most preferred value, least 
    preferred value, results in four comma separated uint16 values.
*/
#define BKV_UINT32R(most_preferred,least_preferred) \
    BKV_UINT32V(least_preferred),BKV_UINT32V(most_preferred)
/*!
    @brief Macro to combine Flow and Error Control Mode and the Mode fallback 
    mask into a single uint16 value
*/
#define BKV_16_FLOW_MODE(mode,fallback_mask) \
    ((((mode)<<8)&0xFF00)|(fallback_mask))
/*! \} */

/*! \name Bluestack Keys for the L2CAP Configuration Table (conftab)
    These are Key / Value pairs.

    Advanced L2CAP configuration can be performed using a Key / Value table of
    uint16 values. This configuration table is used by Bluestack to negotiate
    the L2CAP parameters with a peer automatically.

    Note 1 - QOS_SERVICE: This uint16_t is encoded as two uint8_ts's:
    The MSO defines the required service type using the L2CA_QOS_TYPE
    defines. Specifying guaranteed mode or no traffic mode requires peer
    to support QoS. If LSO is zero and the service type is best effort,
    the option may be ignored completely. If non-zero and best effort
    the option will always be sent.

    Note 2 - FS_SERVICE: Same encoding as for QOS_SERVICE above, except
    that this key will use the extended flow specification instead of
    the old QoS. Also note that LSO *may* be ignored if both parties
    claim support for flowspecs and are using non-basic mode.

    Note 3 - FLOW_MODE: This uint16_t is encoded as two uint8_t's. The
    MSO defines the preferred mode (using the exact L2CA_FLOW_MODE_*
    value). LSO sets the allowed backoff modes in case the peer doesn't
    support the preferred one. This is encoded as a bitmask using the
    (L2CA_MODE_MASK_* flags). The algorithm described in CSA1 will be
    used for backing off. The old FEC modes can only be used by
    selecting them in the MSO.

    Note 4 - FCS: Three values can be used: 0: Don't care. 1: Require
    FCS and 2: Avoid if possible. Note that if one side requires FCS,
    both parties have to use it, so a value of 2 doesn't guarantee that
    FCS is disabled.

    Note that the conftab size is the number of uint16 entries, not the 
    size returned by sizeof. Use CONFTAB_LEN to portably get the length 
    of a constant structure.
*/    
/*! \{ */
/*! 
    @brief This value indicates the start of a configuration data set in the 
    key value pairs
*/
#define L2CAP_AUTOPT_SEPARATOR           (0x8000)
/*!
    @brief 16 bit, exact - incoming MTU
*/
#define L2CAP_AUTOPT_MTU_IN               ((uint16)0x0001) 
/*!
    @brief 16 bit, minimum - peer MTU
*/
#define L2CAP_AUTOPT_MTU_OUT              ((uint16)0x0102) 
/*!
    @brief 32 bit, range - peer flush (us)  - note that HCI limit still applies
*/
#define L2CAP_AUTOPT_FLUSH_IN             ((uint16)0x0703) 
/*!
    @brief 32 bit, range - local flush (us) - note that HCI limit still applies
*/
#define L2CAP_AUTOPT_FLUSH_OUT            ((uint16)0x0704) 
/*!
    @brief 16 bit, exact - shared service type (note1)
*/
#define L2CAP_AUTOPT_QOS_SERVICE          ((uint16)0x0005) 
/*!
    @brief 32 bit, range - incoming token rate/flowspec interarrival
*/
#define L2CAP_AUTOPT_QOS_RATE_IN          ((uint16)0x0706) 
/*!
    @brief 32 bit, range - outgoing token rate/flowspec interarrival
*/
#define L2CAP_AUTOPT_QOS_RATE_OUT         ((uint16)0x0707) 
/*!
    @brief 32 bit, range - incoming token bucket
*/
#define L2CAP_AUTOPT_QOS_BUCKET_IN        ((uint16)0x0708) 
/*!
    @brief 32 bit, range - outgoing token bucket
*/
#define L2CAP_AUTOPT_QOS_BUCKET_OUT       ((uint16)0x0709) 
/*!
    @brief 32 bit, range - incoming peak bandwidth
*/
#define L2CAP_AUTOPT_QOS_PEAK_IN          ((uint16)0x070a) 
/*!
    @brief 32 bit, range - outgoing peak bandwidth
*/
#define L2CAP_AUTOPT_QOS_PEAK_OUT         ((uint16)0x070b) 
/*!
    @brief 32 bit, range - incoming qos/flowspec access latency
*/
#define L2CAP_AUTOPT_QOS_LATENCY_IN       ((uint16)0x070c) 
/*!
    @brief 32 bit, range - outgoing qos/flowspec access latency
*/
#define L2CAP_AUTOPT_QOS_LATENCY_OUT      ((uint16)0x070d) 
/*!
    @brief 32 bit, range - incoming delay variation
*/
#define L2CAP_AUTOPT_QOS_DELAY_IN         ((uint16)0x070e) 
/*!
    @brief 32 bit, range - outgoing delay variation
*/
#define L2CAP_AUTOPT_QOS_DELAY_OUT        ((uint16)0x070f) 
/*!
    @brief 16 bit, range - incoming max SDU size
*/
#define L2CAP_AUTOPT_FS_SDU_SIZE_IN       ((uint16)0x0310) 
/*!
    @brief 16 bit, range - incoming max SDU size
*/
#define L2CAP_AUTOPT_FS_SDU_SIZE_OUT      ((uint16)0x0311) 
/*!
    @brief 16 bit, exact - shared flow control mode (note3)
*/
#define L2CAP_AUTOPT_FLOW_MODE            ((uint16)0x0012) 
/*!
    @brief 16 bit, range - incoming window size
*/
#define L2CAP_AUTOPT_FLOW_WINDOW_IN       ((uint16)0x0313) 
/*!
    @brief 16 bit, range - peer window size
*/
#define L2CAP_AUTOPT_FLOW_WINDOW_OUT      ((uint16)0x0314) 
/*!
    @brief 16 bit, range - peer maximum retransmit
*/
#define L2CAP_AUTOPT_FLOW_MAX_RETX_IN     ((uint16)0x0315) 
/*!
    @brief 16 bit, range - local maximum retransmit
*/
#define L2CAP_AUTOPT_FLOW_MAX_RETX_OUT    ((uint16)0x0316) 
/*!
    @brief 16 bit, range - incoming max PDU payload size
*/
#define L2CAP_AUTOPT_FLOW_MAX_PDU_IN      ((uint16)0x0317) 
/*!
    @brief 16 bit, range - outgoing maximum PDU size
*/
#define L2CAP_AUTOPT_FLOW_MAX_PDU_OUT     ((uint16)0x0318) 
/*!
    @brief 16 bit, exact - use FCS or not (note4)
*/
#define L2CAP_AUTOPT_FCS                  ((uint16)0x0019) 
/*!
    @brief 16 bit, exact - shared flowspec service type (note2)
*/
#define L2CAP_AUTOPT_FS_SERVICE           ((uint16)0x001A) 

/*!
    @brief 32 bit, exact - cached getinfo ext.feats
*/
#define L2CAP_AUTOPT_EXT_FEATS            ((uint16)0x0420) 
/*!
    @brief 16 bit, exact - ward off reconfiguration attempts
*/
#define L2CAP_AUTOPT_DISABLE_RECONF       ((uint16)0x0021) 

/*!
....@brief 16-bit, exact - intial credits given to the remote device while creating connection
*/
#define L2CAP_AUTOPT_CREDITS              ((uint16)0x0025) 
/*!
....@brief 16-bit, exact - l2ca_conflags_t parameter indicating the connection options to bring up the link.
*/
#define L2CAP_AUTOPT_CONN_FLAGS           ((uint16)0x0026) 
/*!
    *brief Indicates the end of a configuration table (conftab)
*/
#define L2CAP_AUTOPT_TERMINATOR           (0xFF00)

/*! \} */

/*! Helper macro to get the length of a static conftab array */

#define CONFTAB_LEN(conftab_) (sizeof(conftab_)/sizeof(uint16))

/*! \name L2CAP Flow & error control modes

  Basic Mode, Enhanced Retransmission Mode and Streaming Mode are supported.
  Retransmission Mode and Flow Control Mode are NOT supported.
*/

/*! \{  */
/*! 
    @brief L2CAP Basic Mode
*/
#define FLOW_MODE_BASIC             0x00
/*! 
    @brief L2CAP Enhanced Retransmission Mode
*/
#define FLOW_MODE_ENHANCED_RETRANS  0x03
/*! 
    @brief L2CAP Streaming Mode
*/
#define FLOW_MODE_STREAMING         0x04
/*! \} */


/* \name Mode Fallback Masks

  Combining the mode masks, allows configuration of other mode to Fallback to
  if the Flow and Error Control mode selected is not supported by a peer 
  (Note 3).

  Fallback mode precedence is Basic, then Enhanced Retransmission and then 
  Streaming.
*/
/*! \{*/ 
#define MODE_MASK(mode)             (1<<(mode))
/*!
    @brief L2CAP No Fallback Mode Mask
*/
#define MODE_MASK_NONE              0
/*! 
    @brief L2CAP Basic Mode Mask
*/
#define MODE_MASK_BASIC             MODE_MASK(FLOW_MODE_BASIC)
/*! 
    @brief L2CAP Enhanced Retransmission Mode Mask
*/
#define MODE_MASK_ENHANCED_RETRANS  MODE_MASK(FLOW_MODE_ENHANCED_RETRANS)
/*! 
    @brief L2CAP Streaming Mode Mask
*/
#define MODE_MASK_STREAMING         MODE_MASK(FLOW_MODE_STREAMING)
/*! \} */


/*! \{ \brief L2CAP flush timeout (32-bit) - Infinite Flush Timeout 
*/ 
#define DEFAULT_L2CAP_FLUSH_TIMEOUT 0xFFFFFFFFu
/*! \} */

/*!
    @brief L2CAP PSM Registration Flags.
*/
/*! Allow Unicast Connectionless Data Reception.*/
#define L2CAP_PSM_REG_FLAG_ALLOW_RX_UCD 0x0001 
/*! Allow Broadcast Connectionless Data Reception.*/
#define L2CAP_PSM_REG_FLAG_ALLOW_RX_BCD 0x0002 
/*! \} */

/*! \name Connection Library initialisation options

    Flags indicating the connection library initialisation options.
*/

/*! \{  */
/*! 
    @brief No initialisation options specified
*/
#define CONNLIB_OPTIONS_NONE        0x0000
/*! 
    @brief Enable Secure Connections by default
*/
#define CONNLIB_OPTIONS_SC_ENABLE   0x0001
/*! 
    @brief Enable Secure Connections Only Mode by default.
           It implies Secure Connections are turned on as well.
*/
#define CONNLIB_OPTIONS_SCOM_ENABLE 0x0002
/*! \} */

/*! 
    @brief L2CAP Connectionless Data mapping types.
*/
typedef enum
{
    /*! Unicast Connectionless Data (UCD).*/
    l2cap_connectionless_data_unicast,
    /*! Broadcast Connectionless Data (BCD).*/
    l2cap_connectionless_data_broadcast
} l2cap_connectionless_data_type;

/*! 
    @brief Generic Connection library return status.
*/
typedef enum 
{
    /*! The operation has completed successfully. */
    success,                    
    /*! The requested operation has failed. */
    fail                        
} connection_lib_status;


/*! 
    @brief HCI command status.

    The status code returned in many primitives from Bluestack is as defined in
    the HCI section of the Bluetooth specification. If a response is being sent
    from the Connection library to a client task this status code is copied
    directly into the message. These error codes correspond to those in the
    Bluetooth specification, and therefore the descriptions are taken from the
    Bluetooth specification documentation.
*/
typedef enum
{
    /*! The command was successful.*/
    hci_success,                                    
    /*! Indicates that the controller does not understand the HCI Command
      Packet OpCode that the host sent. The OpCode given might not correspond
      to any of the OpCodes specified in this document, or any vendor-specific
      OpCodes, or the command may not have been implemented.*/
    hci_error_illegal_command,                        
    /*! Indicates that a command was sent from the host that should identify a
      connection, but that connection does not exist.*/
    hci_error_no_connection,                        
    /*! Indicates to the host that something in the controller has failed in a
      manner that cannot be described with any other error code.*/
    hci_error_hardware_fail,                        
    /*! Indicates that a page timed out because of the Page Timeout
      configuration parameter. This error code may occur only with the
      HCI_Remote_Name_Request and HCI_Create_Connection commands. */
    hci_error_page_timeout,                            
    /*! Indicates that pairing or authentication failed due to incorrect
      results in the pairing or authentication procedure. This could be due to
      an incorrect PIN or Link Key.*/
    hci_error_auth_fail,                            
    /*! Used when pairing failed because of a missing PIN.*/
    hci_error_key_missing,                            
    /*! Indicates to the host that the controller has run out of memory to
      store new parameters.*/
    hci_error_memory_full,                            
    /*! Indicates that the link supervision timeout has expired for a given
      connection.*/
    hci_error_conn_timeout,                            
    /*! Indicates that an attempt to create another connection failed because
      the controller is already at its limit of the number of connections it
      can support.*/
    hci_error_max_nr_of_conns,                        
    /*! Indicates that the controller has reached the limit to the number of
      synchronous connections that can be achieved to a device. */
    hci_error_max_nr_of_sco,                        
    /*! Indicates that the controller has reached the limit to the number of
      asynchronous connections that can be achieved to a device. */
    hci_error_max_nr_of_acl,                        
    /*! Indicates that the command requested cannot be executed because the
      controller is in a state where it cannot process this command at this
      time. This error shall not be used for command OpCodes where the error
      code Unknown HCI Command is valid.*/
    hci_error_command_disallowed,                    
    /*! Indicates that an incoming connection was rejected due to limited
      resources. */
    hci_error_rej_by_remote_no_res,                    
    /*! Indicates that a connection was rejected due to security requirements
      not being fulfilled, like authentication or pairing. */
    hci_error_rej_by_remote_sec,                    
    /*! Indicates that a connection was rejected because this device does not
      accept the BD_ADDR. This may be because the device will only accept
      connections from specific BD_ADDRs.*/
    hci_error_rej_by_remote_pers,                    
    /*! Indicates that the Connection Accept Timeout has been exceeded for this
      connection attempt */
    hci_error_host_timeout,                            
    /*! Indicates that a feature or parameter value in an LMP message or HCI
      Command is not supported.*/
    hci_error_unsupported_feature,                    
    /*! Indicates that at least one of the HCI command parameters is invalid.*/
    hci_error_illegal_format,                        
    /*! Indicates that the user on the remote device terminated the
      connection.*/
    hci_error_oetc_user,                            
    /*! Indicates that the remote device terminated the connection because of
      low resources. */
    hci_error_oetc_low_resource,                    
    /*! Indicates that the remote device terminated the connection because the
      device is about to power off.*/
    hci_error_oetc_powering_off,                    
    /*! Indicates that the local device terminated the connection.*/
    hci_error_conn_term_local_host,                    
    /*! Indicates that the controller is disallowing an authentication or
      pairing procedure because too little time has elapsed since the last
      authentication or pairing attempt failed. */
    hci_error_auth_repeated,                        
    /*! Indicates that the device does not allow pairing. For example, when a
      device only allows pairing during a certain time window after some user
      input allows pairing.*/
    hci_error_pairing_not_allowed,                    
    /*! Indicates that the controller has received an unknown LMP opcode. */
    hci_error_unknown_lmp_pdu,                        
    /*! Indicates that the remote device does not support the feature
      associated with the issued command or LMP PDU.*/
    hci_error_unsupported_rem_feature,                
    /*! Indicates that the offset requested in the LMP_SCO_link_req message has
      been rejected.*/
    hci_error_sco_offset_rejected,                    
    /*! Indicates that the interval requested in the LMP_SCO_link_req message
      has been rejected. */
    hci_error_sco_interval_rejected,                
    /*! Indicates that the air mode requested in the LMP_SCO_link_req message
      has been rejected. */
    hci_error_sco_air_mode_rejected,                
    /*! Indicates that some LMP message parameters were invalid. This shall be
      used when :
      - The PDU length is invalid.
      - A parameter value must be even.
      - A parameter is outside of the specified range.
      - Two or more parameters have inconsistent values. */
    hci_error_invalid_lmp_parameters,                
    /*! Indicates that no other error code specified is appropriate to use. */
    hci_error_unspecified,                            
    /*! Indicates that an LMP message contains at least one parameter value
      that is not supported by the controller at this time. This is normally
      used after a long negotiation procedure, for example during an
      LMP_hold_req, LMP_sniff_req and LMP_encryption_key_size_req message
      exchanges.*/
    hci_error_unsupp_lmp_param,                        
    /*! Indicates that a controller will not allow a role change at this
      time. */
    hci_error_role_change_not_allowed,                
    /*! Indicates that an LMP transaction failed to respond within the LMP
      response timeout. */
    hci_error_lmp_response_timeout,                    
    /*! Indicates that an LMP transaction has collided with the same
      transaction that is already in progress.*/
    hci_error_lmp_transaction_collision,            
    /*! Indicates that a controller sent an LMP message with an opcode that was
      not allowed.*/
    hci_error_lmp_pdu_not_allowed,                    
    /*! Indicates that the requested encryption mode is not acceptable at this
      time. */
    hci_error_enc_mode_not_acceptable,                
    /*! Indicates that a link key can not be changed because a fixed unit key
      is being used.*/
    hci_error_unit_key_used,                        
    /*! Indicates that the requested Quality of Service is not supported.*/
    hci_error_qos_not_supported,                    
    /*! Indicates that an LMP PDU that includes an instant can not be performed
      because the instant when this would have occurred has passed.*/
    hci_error_instant_passed,                        
    /*! Indicates that it was not possible to pair as a unit key was requested
      and it is not supported.*/
    hci_error_pair_unit_key_no_support,                
    /*! Indicates that an LMP transaction was started that collides with an
      ongoing transaction.*/
    hci_error_different_transaction_collision,        
    /*! Insufficient resources.*/
    hci_error_scm_insufficient_resources,            
    /*! Indicates that the specified quality of service parameters could not be
      accepted at this time, but other parameters may be acceptable.*/
    hci_error_qos_unacceptable_parameter,            
    /*! Indicates that the specified quality of service parameters can not be
      accepted and QoS negotiation should be terminated.*/
    hci_error_qos_rejected,                            
    /*! Indicates that the controller can not perform channel classification
      because it is not supported.*/
    hci_error_channel_class_no_support,                
    /*! Indicates that the HCI command or LMP message sent is only possible on
      an encrypted link.*/
    hci_error_insufficient_security,                
    /*! Indicates that a parameter value requested is outside the mandatory
      range of parameters for the given HCI command or LMP message.*/
    hci_error_param_out_of_mand_range,                
    /*! No longer required.*/
    hci_error_scm_no_longer_reqd,                    
    /*! Indicates that a Role Switch is pending. This can be used when an HCI
      command or LMP message can not be accepted because of a pending role
      switch. This can also be used to notify a peer device about a pending
      role switch.*/
    hci_error_role_switch_pending,                    
    /*! Parameter change pending. */
    hci_error_scm_param_change_pending,                
    /*! Indicates that the current Synchronous negotiation was terminated with
      the negotiation state set to Reserved Slot Violation.*/
    hci_error_resvd_slot_violation,                    
    /*! Indicates that a role switch was attempted but it failed and the
      original piconet structure is restored. The switch may have failed
      because the TDD switch or piconet switch failed.*/
    hci_error_role_switch_failed,                    
    /*! Unrecognised error. */
    hci_error_unrecognised 

                           
} hci_status;


/*! 
    @brief The device's role.
*/
typedef enum
{
    hci_role_master,            /*!< The device is Master.*/
    hci_role_slave,             /*!< The device is Slave.*/
    hci_role_dont_care          /*!< Don't request a role switch. */
} hci_role;


/*! 
    @brief The HCI Bluetooth version.
*/
typedef enum
{
    hci_version_1_0,            /*!< HCI Version 1.0.*/
    hci_version_1_1,            /*!< HCI Version 1.1.*/
    hci_version_1_2,            /*!< HCI Version 1.2.*/
    hci_version_2_0,            /*!< HCI Version 2.0.*/
    hci_version_2_1,            /*!< HCI Version 2.1.*/
    hci_version_3_0,            /*!< HCI Version 3.0. */
    hci_version_4_0,            /*!< HCI Version 4.0. */
    hci_version_4_1,            /*!< HCI Version 4.1. */
    hci_version_4_2,            /*!< HCI Version 4.2. */
    hci_version_5_0,            /*!< HCI Version 5.0. */
    hci_version_5_1,            /*!< HCI Version 5.1. */
    hci_version_unrecognised    /*!< HCI Version Unrecognised */
} hci_version;


/*! 
    @brief The host Bluetooth version (will return unknown if HCI version is 
    pre 2.1).
*/
typedef enum
{
    bluetooth_unknown,          /*!< Version Unknown.*/
    bluetooth2_0,               /*!< Bluetooth 2.0.*/
    bluetooth2_1,               /*!< Bluetooth 2.1.*/
    bluetooth3_0,               /*!< Bluetooth 3.0.*/
    bluetooth4_0,               /*!< Bluetooth 4.0.*/
    bluetooth4_1,               /*!< Bluetooth 4.1.*/
    bluetooth4_2,               /*!< Bluetooth 4.2.*/
    bluetooth5_0,               /*!< Bluetooth 5.0.*/
    bluetooth5_1                /*!< Bluetooth 5.1.*/
} cl_dm_bt_version;


/*! 
    @brief Synchronous connection link type.
*/
typedef enum
{
    sync_link_unknown,
    sync_link_sco,
    sync_link_esco
} sync_link_type;


/*! 
    @brief Synchronous connection input coding.
*/
typedef enum
{
    sync_input_coding_linear   = (0 << 8),
    sync_input_coding_ulaw     = (1 << 8),
    sync_input_coding_alaw     = (2 << 8),
    sync_input_coding_reserved = (3 << 8)
} sync_input_coding;

/*! 
    @brief Synchronous connection input data format.
*/
typedef enum
{
    sync_input_format_ones_complement = (0 << 6),
    sync_input_format_twos_complement = (1 << 6),
    sync_input_format_sign_magnitude  = (2 << 6),
    sync_input_format_unsigned        = (3 << 6)
} sync_input_format;

/*! 
    @brief Synchronous connection input sample size.
*/
typedef enum
{
    sync_input_size_8_bit  = (0 << 5),
    sync_input_size_16_bit = (1 << 5)
} sync_input_size;

/*! 
    @brief Synchronous connection linear PCM bit position.
*/
typedef enum
{
    sync_pcm_pos_bit_0 = (0 << 2),
    sync_pcm_pos_bit_1 = (1 << 2),
    sync_pcm_pos_bit_2 = (2 << 2),
    sync_pcm_pos_bit_3 = (3 << 2),
    sync_pcm_pos_bit_4 = (4 << 2),
    sync_pcm_pos_bit_5 = (5 << 2),
    sync_pcm_pos_bit_6 = (6 << 2),
    sync_pcm_pos_bit_7 = (7 << 2)
} sync_pcm_pos;

/*! 
    @brief Synchronous connection air coding format.
*/
typedef enum
{
    sync_air_coding_cvsd        = (0 << 0),
    sync_air_coding_ulaw        = (1 << 0),
    sync_air_coding_alaw        = (2 << 0),
    sync_air_coding_transparent = (3 << 0),

    FORCE_ENUM_TO_MIN_16BIT(sync_air_coding)
} sync_air_coding;


/*! 
    @brief Synchronous packet types.

    These are bit-field values. Be aware that some of the values indicate 
    which synchronous packet types may be used while other values indicate the
    reverse, e.g. which synchronous packet types may NOT be used.

*/
typedef enum
{
    /*! HV1 may be used */
    sync_hv1           = 0x0001,
    /*! HV2 may be used */
    sync_hv2           = 0x0002,
    /*! HV3 may be used */
    sync_hv3           = 0x0004,
    /*! HV1, HV2 and HV3 may be used - combined bitfield */
    sync_all_sco       = sync_hv3 | sync_hv2 | sync_hv1,
    /*! EV3 may be used */
    sync_ev3           = 0x0008,
    /*! EV4 may be used */
    sync_ev4           = 0x0010,
    /*! EV5 may be used */
    sync_ev5           = 0x0020,
    /*! EV3, EV4 and EV5 may be used - combined bitfield */
    sync_all_esco      = sync_ev5 | sync_ev4 | sync_ev3,
    /*! 2-EV3 may NOT be used */
    sync_2ev3          = 0x0040,
    /*! 3-EV3 may NOT be used */
    sync_3ev3          = 0x0080,
    /*! 2-EV5 may NOT be used */
    sync_2ev5          = 0x0100,
    /*! 3-EV5 may NOT be used */
    sync_3ev5          = 0x0200,
    /*! 2-EV3, 3-EV3, 2-EV5 and 3-EV5 may NOT be used - combined bitfield */
    sync_all_edr_esco  = sync_3ev5 | sync_2ev5 | sync_3ev3 | sync_2ev3,
    /*! All synchronous packet types as a combined bitfield */
    sync_all_pkt_types = sync_all_edr_esco | sync_all_esco | sync_all_sco
} sync_pkt_type;

/*! 
    @brief Synchronous retransmission effort
*/

typedef enum
{
    /*! No retransmissions. */
    sync_retx_disabled,
    /*! At least one retransmission, optimise for power consumption. */
    sync_retx_power_usage,
    /*! At least one retransmission, optimise for link quality. */
    sync_retx_link_quality,
    /*! Don't care. */
    sync_retx_dont_care = 0xFF,

    FORCE_ENUM_TO_MIN_16BIT(sync_retx_effort)
} sync_retx_effort;

/*! 
    @brief Synchronous connection configuration parameters
*/
typedef struct
{
   uint32            tx_bandwidth;
   uint32            rx_bandwidth;
   uint16            max_latency;
   sync_air_coding   voice_settings;
   sync_retx_effort  retx_effort;
   sync_pkt_type     packet_type;
} sync_config_params;


/*! 
    @brief Synchronous connection negotiated link parameters
*/
typedef struct
{
    /*! eSCO receive bandwidth.  Will be zero for SCO. */
    uint32          rx_bandwidth;   
    /*! eSCO transmit bandwidth.  Will be zero for SCO. */
    uint32          tx_bandwidth;   
    /*! Specifies whether a SCO or eSCO packet type was obtained. */
    sync_link_type  link_type;      
} sync_link_params;


/*! 
    @brief Page scan repetition modes.
*/
typedef enum
{
    /*! Page scan interval <=1.28 seconds and = Page scan window. */
    page_scan_rep_mode_r0,            
    /*! Page scan interval <=1.28 seconds. */
    page_scan_rep_mode_r1,            
    /*! Page scan interval <=2.56 seconds. */
    page_scan_rep_mode_r2,            
    /*! Unknown page scan mode. */
    page_scan_rep_mode_unknown        
} page_scan_rep_mode;


/*! 
    @brief Page scan mode.
*/
typedef enum
{
    page_scan_mode_mandatory,        /*!< Mandatory page scan mode.*/
    page_scan_mode_optional_1,        /*!< Optional page scan mode.*/
    page_scan_mode_optional_2,        /*!< Optional page scan mode.*/
    page_scan_mode_optional_3,        /*!< Optional page scan mode.*/
    page_scan_mode_unknown            /*!< Unknown page scan mode.*/
} page_scan_mode;

/*!
    @brief Page and Inquiry scan type.
*/
typedef enum
{
    /*! Standard Scan */
    hci_scan_type_standard,
    /*! Interlaced Scan */
    hci_scan_type_interlaced
} hci_scan_type;

/*! 
    @brief Scan enable values.

    The scan enable parameter controls the page and inquiry scan mode to the
    device.
*/
typedef enum
{
    /*! No scans enabled. */
    hci_scan_enable_off,            
    /*! Enable inquiry scan. Page scan disabled. */
    hci_scan_enable_inq,            
    /*! Enable page scan. Inquiry scan disabled. */
    hci_scan_enable_page,            
    /*! Enable inquiry and page scan. */
    hci_scan_enable_inq_and_page    
} hci_scan_enable;


/*! 
    @brief Inquiry mode.
*/
typedef enum
{
    inquiry_mode_standard,
    inquiry_mode_rssi,
    inquiry_mode_eir
} inquiry_mode;


/*! 
    @brief Inquiry status.

    This is the status returned in a CL_DM_INQUIRE_RESULT message an indicates
    whether the message contains a valid inquiry result or is just a
    notification that the inquiry has completed.
*/
typedef enum 
{
    /*! Notification that the inquiry process has completed. */
    inquiry_status_ready,    
    /*! The CL_DM_INQUIRE_RESULT message contains a valid inquiry result */
    inquiry_status_result    
} inquiry_status;

/*! 
    @brief Remote name status.

    This is the status returned in a CL_DM_REMOTE_NAME_COMPLETE message 
    an indicates whether the message contains a remote name request or error.
*/

typedef enum
{
    /*! Remote name request completed with a name */
    rnr_success,

    /*! Remote name request completed without a name */
    rnr_completed = hci_error_unrecognised + 1,

    /*! local device cancelled the RNR request */
    rnr_cancelled,

    /*! RNR completed with error */
    rnr_error

} rnr_status;

/*! 
    @brief Specifies that RSSI parameter returned in CL_DM_INQUIRE_RESULT 
    primitive is unknown.
*/
#define CL_RSSI_UNKNOWN ((int16)0x7FFF)

/*! 
    @brief Definition of Authentication (Pairing status).
*/
typedef enum 
{
    /*! Authentication was successful. */
    auth_status_success,                    
    /*! Authentication timed out. */
    auth_status_timeout,                    
    /*! Authentication failed. */
    auth_status_fail,                       
    /*! Authentication failed due to too many repeat attempts. */
    auth_status_repeat_attempts,            
    /*! Authentication failed as remote device is not allowing pairing. */
    auth_status_pairing_not_allowed,        
    /*! Authentication failed as unit keys are not supported. */
    auth_status_unit_key_unsupported,       
    /*! Authentication failed as simple pairing is not supported. */
    auth_status_simple_pairing_unsupported, 
    /*! Authentication failed as host is already busy pairing. */
    auth_status_host_busy_pairing           
} authentication_status;


/*! 
    @brief DM protocol identifier.
*/
typedef enum
{
    protocol_l2cap,             /*!< L2CAP protocol. */
    protocol_rfcomm,            /*!< RFCOMM protocol.*/
    protocol_le_l2cap,          /*!< LE L2CAP protocol. */
    protocol_unknown            /*!< Unknown protocol.*/
} dm_protocol_id;


/*! 
    @brief HCI encryption mode.
*/
typedef enum
{
    /*! Encryption off.*/
    hci_enc_mode_off,                    
    /*! Encrypt point to point traffic.*/
    hci_enc_mode_pt_to_pt,                
    /*! Encrypt point to point and broadcast traffic.*/
    hci_enc_mode_pt_to_pt_and_bcast        
} encryption_mode;


/*! 
    @brief DM security mode.

    Descriptions from the Bluetooth specification documentation.
*/
typedef enum
{
    /*! Security off.*/
    sec_mode0_off,                
    /*! When a Bluetooth device is in security mode 1 it shall never initiate
      any security procedure.*/
    sec_mode1_non_secure,        
    /*! When a Bluetooth device is in security mode 2 it shall not initiate any
      security procedure before a channel establishment request
      (L2CAP_ConnectReq) has been received or a channel establishment procedure
      has been initiated by itself.*/
    sec_mode2_service,
    /*! When a Bluetooth device is in security mode 3 it shall initiate
      security procedures before it sends LMP_link_setup_complete.*/
    sec_mode3_link,        
    /*! When a Bluetooth device is in mode 4 it enforces it's security 
      requirements before it attempts to access services offered by a remote 
      device and before it grants access to services it offers to remote 
      devices.*/
    sec_mode4_ssp,
    /*! Security mode 2a is a CSR custom security mode that authenticates on a
      per device basis.
      
      On starting a new service (incoming and outgoing), the device will be
      authenticated unless it is marked as 'trusted' in the device database
      (see ConnectionSmAddAuthDevice). */
    sec_mode2a_device,
    /*! Unknown security mode.*/
    sec_mode_unknown            
} dm_security_mode;

/*! \name DM Security setting for incoming connections

  These values can be bitwise OR'ed to provide composite security mask. Common
  bitwise masks are already provided.
*/
/*! \{   */
/*! No security - in Mode 4 this is equivalent to Security Level 0*/
#define sec_in_none            0x0000

/*! mode 4 & legacy - results in a  DM_SM_AUTHORISE_IND on connection 
attempt for devices not marked in as 'trusted' in the device database. */
#define sec_in_authorisation   0x0001

/*! legacy - For connections where at least one device is not in mode 4, 
legacy pairing procedure should be used. The users may have to enter PINS.
*/
#define sec_in_authentication  0x0002

/*! legacy - The link should be encrypted after legacy authentication. */
#define sec_in_encryption      0x0004

/*! mode 4 - Where both devices are in Mode 4, Secure Simple Pairing is
used. */
#define sec4_in_ssp            0x0100

/*! mode 4 - Where both devices are in Mode 4, Man-In-The-Middle protection
is used. */
#define sec4_in_mitm           0x0200

/*! mode 4 - Don't support legacy security procedures. Prevents devices 
using the legacy pairing procedures to authenticate devices that do not 
support Secure Simple Pairing. */
#define sec4_in_no_legacy      0x0400

/*! mode 4 - Require a P256 link key and AES encrpytion (i.e. a Secure
Connection. */
#define sec4_in_p256_only      0x4000

/*! mode 4 - Security Level 1. Minimal user interaction required. 
Encryption not necessary (for SDP - for all 2.1+EDR devices, it is mandated
for all other services). MITM not necessary. */
#define sec4_in_level_1        sec4_in_ssp

/*! mode 4 - Security Level 2. MITM not necessary, Encryption necessary. */
#define sec4_in_level_2      ( sec4_in_ssp | \
                               sec_in_authentication | \
                               sec_in_encryption )

/*! mode 4 - security Level 3. MITM necessary, Encryption necessary, user 
interaction is acceptable. */
#define sec4_in_level_3      ( sec4_in_ssp | \
                               sec4_in_mitm | \
                               sec_in_authentication | \
                               sec_in_encryption )

/*! mode 4 - security Level 4. MITM necessary, Encryption necessary, user 
interaction is acceptable, Secure Connection necessary */
#define sec4_in_level_4      ( sec4_in_ssp | \
                               sec4_in_mitm | \
                               sec_in_authentication | \
                               sec_in_encryption | \
                               sec4_in_no_legacy | \
                               sec4_in_p256_only )

/*! Bit Mask of all the possible incoming security bit settings */
#define sec_in_bitmask       ( sec_in_authorisation | \
                               sec_in_authentication | \
                               sec_in_encryption | \
                               sec4_in_ssp | \
                               sec4_in_mitm | \
                               sec4_in_no_legacy | \
                               sec4_in_p256_only )
/*! \} */

/*! 
  @brief DM Security setting for incoming connections.
*/
typedef uint16 dm_security_in;
    
/*! \name DM Security setting for outgoing connections.

  These values can be bitwise OR'ed to provide composite security mask.
  Common bitwise masks are already provided.
*/
/*! \{   */
/*! No security - in Mode 4 this is equivalent to Security Level 0*/
#define sec_out_none           0x0000

    /*! mode 4 & legacy - results in a  DM_SM_AUTHORISE_IND on connection attempt
for devices not marked in as 'trusted' in the device database. */
#define sec_out_authorisation  0x0008

/*! legacy - For connections where at least one device is not in mode 4, 
legacy pairing procedure should be used. The users may have to enter PINS.
*/
#define sec_out_authentication 0x0010

/*! legacy - The link should be encrypted after legacy authentication. */
#define sec_out_encryption     0x0020

/*! mode 4 - Where both devices are in Mode 4, Secure Simple Pairing is
used. */
#define sec4_out_ssp           0x0800

/*! mode 4 - Where both devices are in Mode 4, Man-In-The-Middle protection
is used. */
#define sec4_out_mitm          0x1000

/*! mode 4 - Don't support legacy security procedures. Prevents devices 
using the legacy pairing procedures to authenticate devices that do not 
support Secure Simple Pairing. */
#define sec4_out_no_legacy     0x2000

/*! mode 4 - Require a P256 link key and AES encrpytion (i.e. a Secure
Connection. */
#define sec4_out_p256_only     0x8000

/*! mode 4 - Security Level 1. Minimal user interaction required. 
Encryption not necessary (for SDP - for all 2.1+EDR devices, it is mandated
for all other services). MITM not necessary. */
#define sec4_out_level_1       sec4_out_ssp

/*! mode 4 - Security Level 2. MITM not necessary, Encryption necessary. */
#define sec4_out_level_2     ( sec4_out_ssp | \
                               sec_out_authentication | \
                               sec_out_encryption )

/*! mode 4 - security Level 3. MITM necessary, Encryption necessary, user 
interaction is acceptable. */
#define sec4_out_level_3     ( sec4_out_ssp | \
                               sec4_out_mitm | \
                               sec_out_authentication | \
                               sec_out_encryption )

/*! mode 4 - security Level 4. MITM necessary, Encryption necessary, user 
interaction is acceptable, Secure Connection necessary. */
#define sec4_out_level_4     ( sec4_out_ssp | \
                               sec4_out_mitm | \
                               sec_out_authentication | \
                               sec_out_encryption | \
                               sec4_out_no_legacy | \
                               sec4_out_p256_only )

/*! Bit Mask of all the possible outgoing security bit settings */
#define sec_out_bitmask     ( sec_out_authorisation | \
                              sec_out_authentication | \
                              sec_out_encryption | \
                              sec4_out_ssp | \
                              sec4_out_mitm | \
                              sec4_out_no_legacy | \
                              sec4_out_p256_only )
/*! \} */

/*! 
  @brief DM Security setting for outgoing connections.
*/
typedef uint16 dm_security_out;
    
/*! 
  @brief DM SSP security levels (as defined in 2.1 spec).
*/
typedef enum
{
    ssp_secl4_l0,               /*!< Mode 4 Level 0 - No Security.*/
    ssp_secl4_l1,               /*!< Mode 4 Level 1 - SSP.*/
    ssp_secl4_l2,               /*!< Mode 4 Level 2 - No MITM.*/
    ssp_secl4_l3,               /*!< Mode 4 Level 3 - MITM.*/
    ssp_secl4_l4,               /*!< Mode 4 Level 4 - Secure Connections.*/
    ssp_secl_level_unknown      /*!< Leave security level unchanged.*/
} dm_ssp_security_level;

/*! 
    @brief Write Auth Enable, determines under which circumstances the device 
    will drop existing ACLs to pair during link setup when pairing with mode 3 
    devices. 
*/
typedef enum
{
    /*! Never pair during link setup.*/
    cl_sm_wae_never,            
    /*! Pair during link setup if there are no existing ACLs.*/    
    cl_sm_wae_acl_none,         
    /*! Pair during link setup, first bringing down any existing ownerless 
     ACLs.*/
    cl_sm_wae_acl_owner_none,   
    /*! Pair during link setup, first bringing down any existing ownerless and 
     application owned ACLs.*/
    cl_sm_wae_acl_owner_app,    
    /*! Pair during link setup, first bringing down any existing ownerless and 
     L2CAP owned ACLs.*/
    cl_sm_wae_acl_owner_l2cap,  
    /*! Always pair during link setup bringing down any existing ACLs.*/
    cl_sm_wae_always            
} cl_sm_wae;

/*!
  @brief Bonding requirements
*/
typedef enum
{
    /*! No bonding will take place (i.e. link key not stored) no MITM 
     protection .*/
    cl_sm_no_bonding_no_mitm,                   
    /*! No bonding will take place (i.e. link key not stored) plus MITM 
     protection .*/
    cl_sm_no_bonding_mitm,                      
    /*! Dedicated bonding (i.e. bonding occurs and ACL is dropped) no MITM 
     protection.*/
    cl_sm_dedicated_bonding_no_mitm,            
    /*! Dedicated bonding (i.e. bonding occurs and ACL is dropped) plus MITM 
      protection.*/
    cl_sm_dedicated_bonding_mitm,               
    /*! General bonding (i.e. connection established without dropping ACL after 
     bonding) no MITM protection.*/    
    cl_sm_general_bonding_no_mitm,              
    /*! General bonding (i.e. connection established without dropping ACL after 
     bonding) plus MITM protection.*/    
    cl_sm_general_bonding_mitm,                 
    /*! Unrecognised authentication requirements received.*/
    cl_sm_authentication_requirements_unknown   
} cl_sm_auth_requirements;

/*!
  @brief Link Key Types
*/
typedef enum
{
    /*! No link key.*/
    cl_sm_link_key_none,
    /*! Legacy link key.*/
    cl_sm_link_key_legacy,
    /*! Debug link key.*/
    cl_sm_link_key_debug,
    /*! Unauthenticated link key - BT 4.0 or earlier.*/
    cl_sm_link_key_unauthenticated_p192,
    /*! Authenticated link key - BT 4,0 or earlier.*/
    cl_sm_link_key_authenticated_p192, 
    /*! Cant tell if authenticated or not.*/
    cl_sm_link_key_changed,
    /*! Unauthenticated link key - BT 4.1 or later.*/
    cl_sm_link_key_unauthenticated_p256, 
    /*! Authenticated link key - BT 4.1 or later.*/
    cl_sm_link_key_authenticated_p256 
} cl_sm_link_key_type;                

/*! \name cl_sm_link_key defines for backwards compatibility. */
/*!\{ */
/*! Same as cl_sm_link_key_unathenticated_p192. */
#define cl_sm_link_key_unauthenticated cl_sm_link_key_unauthenticated_p192
/*! Same as cl_sm_link_key_athenticated_p192. */
#define cl_sm_link_key_authenticated cl_sm_link_key_authenticated_p192
/*!\} */

/*!
  @brief Encryption Key Types
*/
typedef enum
{
    /*! No encryption. */
    cl_sm_encryption_none,
    /*! Encryption E0 for BR/EDR or AES-CCM for LE */
    cl_sm_encryption_e0_brdedr_aes_ccm_le,
    /*! Encryption AES-CCM for BR/EDR transport */
    cl_sm_encryption_aes_ccm_bredr 
} cl_sm_encryption_key_type;

/*!
  @brief IO Capability
*/
typedef enum
{
    /*! Display Only.*/
    cl_sm_io_cap_display_only,          
    /*! Display Yes/No.*/
    cl_sm_io_cap_display_yes_no,        
    /*! Keyboard Only.*/
    cl_sm_io_cap_keyboard_only,         
    /*! No IO.*/
    cl_sm_io_cap_no_input_no_output,    
    /*! Keyboard and numeric output */
    cl_sm_io_cap_keyboard_display, 
    /*! Use this to reject the IO capability request */
    cl_sm_reject_request                
} cl_sm_io_capability;        

/*!
  @brief Key press Types (used during passkey entry)
*/
typedef enum
{
    cl_sm_passkey_started,          /*!< User started entering passkey.*/
    cl_sm_passkey_digit_entered,    /*!< User entered digit.*/
    cl_sm_passkey_digit_erased,     /*!< User erased digit.*/
    cl_sm_passkey_cleared,          /*!< User cleared entire passkey.*/
    cl_sm_passkey_complete          /*!< User completed passkey entry.*/
} cl_sm_keypress_type;

/*! 
    @brief Base value for SDP close status enums.
*/
#define SDP_CLOSE_STATUS_BASE    (0x02)


/*! 
    @brief SDP close status.
*/
typedef enum 
{
    /*! The connection was closed by the SDS server.*/
    sdp_close_sdc_sds_disconnected,
    /*! The connection was closed by the SDC client.*/
    sdp_close_sdc_sdc_disconnected,
    /*! A search is not open.*/
    sdp_close_search_not_open,    
    /*! Task requesting the search be closed did not open it.*/
    sdp_close_task_did_not_open_search,
    /*! Unknown SDP Close status.*/
    sdp_close_unknown
} sdp_close_status;


/*! 
    @brief SDP open status.
*/
typedef enum
{
    /*! The open search attempt was successful.*/
    sdp_open_search_ok,                         
    /*! The SDP server is busy.*/
    sdp_open_search_busy,                       
    /*! The open search attempt failed.*/
    sdp_open_search_failed,                     
    /*! A search is already open.*/
    sdp_open_search_open,                       
    /*! The connection was lost.*/
    sdp_open_disconnected,                      
    /*! Unknown status.*/
    sdp_open_unknown                            
} sdp_open_status;


/*! 
    @brief SDP search status.
*/
typedef enum
{
    /*! The search was successful..*/
    sdp_response_success,                   
    /*! Error response PDU.*/
    sdp_error_response_pdu,                 
    /*! No response data.*/
    sdp_no_response_data,                   
    /*! Connection disconnected.*/
    sdp_con_disconnected,                   
    /*! Connection error.*/
    sdp_connection_error,                   
    /*! Configuration error.*/
    sdp_configure_error,                    
    /*! Search data error.*/
    sdp_search_data_error,                  
    /*! Data confirmation error.*/
    sdp_data_cfm_error,                     
    /*! Search busy.*/
    sdp_search_busy,                        
    /*! Invalid PDU header.*/
    sdp_response_pdu_header_error,          
    /*! Invalid PDU size.*/
    sdp_response_pdu_size_error,            
    /*! Search timeout error.*/
    sdp_response_timeout_error,             
    /*! Size too big error.*/
    sdp_search_size_to_big,                 
    /*! Out of memory error.*/
    sdp_response_out_of_memory,             
    /*! Response terminated.*/
    sdp_response_terminated,                
    /*! Connection error: page timeout.*/
    sdp_connection_error_page_timeout,      
    /*! Connection error: rejected PSM.*/
    sdp_connection_error_rej_psm,           
    /*! Connection error: rejected security.*/
    sdp_connection_error_rej_security,      
    /*! Connection error: rejected resources.*/
    sdp_connection_error_rej_resources,     
    /*! Connection error: signal timeout.*/
    sdp_connection_error_signal_timeout,    
    /*! Unknown status.*/
    sdp_search_unknown                      
} sdp_search_status;


/*! 
    @brief L2CAP connection status.

    This is the status returned in a CL_L2CAP_CONNECT_CFM message indicating
    the outcome of the connection attempt.
*/
typedef enum
{
    /*! L2CAP connection successfully established. */
    l2cap_connect_success,
    /*! L2CAP connection is pending. */
    l2cap_connect_pending,
    /*! The L2CAP connect attempt failed because either the local or remote end
      issued a disconnect before the connection was fully established. */
    l2cap_connect_failed = 0x80,                    
    /*! The connection attempt failed due to an internal error in the
      Connection library. */
    l2cap_connect_failed_internal_error,    
    /*! The connection attempt failed because the remote end rejected the
      connection request. */
    l2cap_connect_failed_remote_reject,        
    /*! The connection attempt failed because the remote device rejected our
      configuration request. */
    l2cap_connect_failed_config_rejected,    
    /*! The connection attempt failed due to security requirements. */
    l2cap_connect_failed_security,
    /*! The connection was terminated by the local host. */
    l2cap_connect_terminated_by_host,
    /*! The connection attempt failed because the remote device closed the 
        connection. */
    l2cap_connect_failed_remote_disc,        
    /*! The conftab sent to Bluestack was invalid and rejected immediately. */
    l2cap_connect_failed_invalid_conftab,
    /*! The connection attempt timed out. */
    l2cap_connect_timeout,                    
    /*! The connection attempt failed because of an error. */
    l2cap_connect_error,        
    /*! Unknown status. */
    l2cap_connect_unknown = 0xFF
} l2cap_connect_status;


/*!
    @brief L2CAP LE COC connection status.

    This is the status returned in a CL_L2CAP_TP_CONNECT_CFM message indicating
    the outcome of the connection attempt.
*/
typedef enum
{
    /*! L2CAP connection successfully established. */
    l2cap_tp_connect_success,
    /*! L2CAP connection is pending. */
    l2cap_tp_connect_pending,
    /*! The L2CAP connect attempt failed because either the local or remote end
      issued a disconnect before the connection was fully established. */
    l2cap_tp_connect_failed = 0x80,                    
    /*! The connection attempt failed due to an internal error in the
      Connection library. */
    l2cap_tp_connect_failed_internal_error,    
    /*! The connection attempt failed because the remote end rejected the
      connection request. */
    l2cap_tp_connect_failed_remote_reject,        
    /*! The connection attempt failed because the remote device rejected our
      configuration request. */
    l2cap_tp_connect_failed_config_rejected,    
    /*! The connection attempt failed due to security requirements. */
    l2cap_tp_connect_failed_security,
    /*! The connection was terminated by the local host. */
    l2cap_tp_connect_terminated_by_host,
    /*! The connection attempt failed because the remote device closed the 
        connection. */
    l2cap_tp_connect_failed_remote_disc,        
    /*! The conftab sent to Bluestack was invalid and rejected immediately. */
    l2cap_tp_connect_failed_invalid_conftab,
    /*! The connection attempt timed out. */
    l2cap_tp_connect_timeout,                    
    /*! The connection attempt failed because of an error. */
    l2cap_tp_connect_error,        
    /*! Unknown status. */
    l2cap_tp_connect_unknown = 0xFF
} l2cap_tp_connect_status;

/*!
    @brief L2CAP Map Connectionless Status
*/
typedef enum
{
    /*! Mapping was successful.*/
    l2cap_map_connectionless_success,
    /*! Mapping is initiating.*/
    l2cap_map_connectionless_initiating,
    /*! Ran out of memory while trying to map.*/
    l2cap_map_connectionless_out_of_memory,
    /*! Invalid PSM used.*/
    l2cap_map_connectionless_invalid_psm,
    /*! Invalid CID used.*/
    l2cap_map_connectionless_invalid_cid,
    /*! Mapping timed out.*/
    l2cap_map_connectionless_timed_out,
    /*! Mapping already registered.*/
    l2cap_map_connectionless_already_registered,
    /*! The remote device doesn't support Unicast */
    l2cap_map_connectionless_unicast_unsupported,
    /*! Mapping failed.*/
    l2cap_map_connectionless_failed,
    /*! Unknown status. */
    l2cap_map_connectionless_unknown = 0xFF
} l2cap_map_connectionless_status;


/*! 
    @brief L2CAP disconnect status.

    This is the status returned in an CL_L2CAP_DISCONNECT_IND message
    indicating that the L2CAP connection has been disconnected.
*/
typedef enum
{
    /*! The L2CAP connection was disconnected successfully. */
    l2cap_disconnect_successful,    
    /*! The L2CAP disconnect attempt timed out. */
    l2cap_disconnect_timed_out,        
    /*! The L2CAP disconnect attempt returned an error. */
    l2cap_disconnect_error,            
    /*! The L2CAP connection could not be disconnected because a null sink was
      passed in. */
    l2cap_disconnect_no_connection,
    /*! The L2CAP connection was disconnected due to link loss. */
    l2cap_disconnect_link_loss
} l2cap_disconnect_status;


/*! 
    @brief Power mode.
*/
enum lp_power_mode
{
    /*! Use active power mode.*/
    lp_active,                
    /*! Use sniff power mode.*/
    lp_sniff,                
    /*! Use the passive power mode. Passive mode is a "don't care" setting
      where the local device will not attempt to alter the power mode. */
    lp_passive = 0xff        
};

typedef uint8 lp_power_mode;

/*!
    @brief Link Policy Settings
    This bitmask specifies the link policy settings to update
*/
typedef enum
{ 
    /*! Disable all LM modes */
    lp_disable_all_lm_modes = 0x0000,
    /*! Enable role switching */
    lp_enable_ms_switch     = 0x0001,
    /*! Enable Hold Mode */
    lp_enable_hold          = 0x0002,
    /*! Enable Sniff Mode */
    lp_enable_sniff         = 0x0004,    
    /*! Enable Park Mode*/
    lp_enable_park          = 0x0008,
    /*! Mask for Link policy settings */
    lp_link_policy_mask     = 0x00FF   
} lp_settings;

/*!
    @brief SDS status
*/
typedef enum
{
    sds_status_success,                 /*!< SDS Success. */
    sds_status_pending,                 /*!< SDS Pending. */
    sds_status_unspecified_error,       /*!< SDS Unspecified Error. */
    sds_status_invalid_sr_data_error,   /*!< SDS Invalid SR Data Error. */
    sds_status_invalid_sr_handle_error, /*!< SDS Invalid SR Handle Error. */
    sds_status_insufficient_resources,  /*!< SDS Insufficient resources. */
    sds_status_unknown                  /*!< SDS Unknown Status. */
} sds_status;


/*! 
    @brief Power table structure. 

    Used to define the link policy for an ACL, see #ConnectionSetLinkPolicy.

    @note Apart from 'state' and 'time', the other fields of this structure are
    only used if the lp_power_mode 'state' is 'lp_sniff'. Those fields are as
    for the HCI_Sniff_Mode command.

*/
typedef struct
{
    /*! The power mode, see #lp_power_mode .*/
    lp_power_mode   state;            
    /*! Sniff minimum interval. Time = N x 0.625, Range 0x0002 (1.25 msec) to 
        0xFFFE (40.9), only even values are valid. Only used if lp_power_mode is 
        lp_sniff. */
    uint16          min_interval;    
    /*! Sniff maximum interval. Time = N x 0.625, Range 0x0002 (1.25 msec) to 
        0xFFFE (40.9 sec), only even values are valid. Only used if 
        lp_power_mode is lp_sniff.*/
    uint16          max_interval;    
    /*! Number of Baseband receive slots for sniff attempt. Length = N * 1.25 
        msec, Range 0x0001 (1.25 msec) to 0x7FFF (40.9 sec) Only used if 
        lp_power_mode is lp_sniff.*/
    uint16          attempt;        
    /*! Number of Baseband receive slots for sniff timeout. Length = N * 1.25 
        msec, Range 0x0000 (0 msec) to 0x7FFF (40.9 sec) Only used if 
        lp_power_mode is lp_sniff. */
    uint16          timeout;        
    /*! The time spent in this state of the power table, in seconds. This must 
        be 0 for the last entry in the table. */
    uint16          time;            
} lp_power_table;


/*! 
    @brief L2CAP Quality of Service Parameters

    The Quality of Service parameters are negotiated before an L2CAP connection
    is established.  A detailed explanation of each of these parameters can be
    found in the L2CAP section of the Bluetooth specification.
*/
typedef struct
{
    /*! Level of the service required e.g. best effort. */
    uint8       service_type;    
    /*! Average data rate with which data is transmitted. */
    uint32      token_rate;     
    /*! Specifies a limit on the "burstiness" with which data may be
      transmitted. */
    uint32      token_bucket;   
    /*! This limits how fast L2CAP packets can be sent back-to-back. */
    uint32      peak_bw;        
    /*! Maximum acceptable latency of an L2CAP packet. */
    uint32      latency;        
    /*! Difference between the maximum and minimum acceptable delay of an L2CAP
      packet. */
    uint32      delay_var;      
} qos_flow;

/*! 
    @brief RFCOMM Port parameters.
*/
typedef struct 
{
    uint8    baud_rate;             /*!< The port baud rate.*/
    uint8    data_bits;             /*!< The data bits.*/
    uint8    stop_bits;             /*!< The stop bits.*/
    uint8    parity;                /*!< Parity.*/
    uint8    parity_type;           /*!< The parity type.*/
    uint8    flow_ctrl_mask;        /*!< The flow control mask.*/
    uint8    xon;                   /*!< Xon*/
    uint8    xoff;                  /*!< Xoff*/
    uint16   parameter_mask;        /*!< The parameter mask.*/
} port_par;

/*!
    @brief RFCOMM Configuration Parameters
*/
typedef struct
{   
    /*! Maximum RFCOMM payload size.*/
    uint16  max_payload_size;      
    /*! RFCOMM modem status.*/
    uint8   modem_signal;          
    /*! The break signal, range 0-15 in increments of 200ms.*/
    uint8   break_signal;          
    /*! Modem Status timeout during connection in milliseconds. If the 
    timeout occurs before a RFC_MODEM_STATUS_IND is received, then the 
    connection is completed anyway.*/
    uint16  msc_timeout;           
} rfcomm_config_params;


/*! 
    @brief RFCOMM connection status.
*/
typedef enum
{
    /*! The connection was successful.*/
    rfcomm_connect_success,     
    /*! The connection is pending.*/
    rfcomm_connect_pending,
    /*! The connection failed.*/
    rfcomm_connect_failed,                        
    /*! The server channel was not registered.*/
    rfcomm_connect_channel_not_registered,   
    /*! The client security channel was not set */
    rfcomm_connect_security_not_set,
    /*! The connection was declined by this application.*/
    rfcomm_connect_declined,                        
    /*! The connection was rejected.*/
    rfcomm_connect_rejected,                    
    /*! The client has attempted to connect to a server channel that has already
        been connected to. */
    rfcomm_connect_channel_already_open,
    /*! The connection was rejected mismatched security settings.*/
    rfcomm_connect_rejected_security,
    /*! The Response to an RFCOMM connect request timed out.*/
    rfcomm_connect_res_ack_timeout,
    /*! The connection was rejected because the max_payload_size parameter was 
        outside the allowed range 8..65530. */
    rfcomm_connect_inconsistent_parameters,
    /*! The connection failed because of an underlying L2CAP error.*/
    rfcomm_connect_l2cap_error,
    /*! The connection was disconnected normally by the remote device (it may
        already have a connection open). */
    rfcomm_connect_normal_disconnect
} rfcomm_connect_status;                        


/*! 
    @brief RFCOMM disconnection status.
*/
typedef enum
{
    /*! The disconnection was successful.*/
    rfcomm_disconnect_success,                                    
    /*! The disconnection attempt disconnected normally.*/
    rfcomm_disconnect_normal_disconnect,                        
    /*! The disconnection attempt disconnected abnormally.*/
    rfcomm_disconnect_abnormal_disconnect,                        
    /*! The disconnection attempt was refused remotely.*/
    rfcomm_disconnect_remote_refusal,                            
    /*! Already exists.*/
    rfcomm_disconnect_dlc_already_exists,
    /*! Rejected security.*/
    rfcomm_disconnect_dlc_rej_security,                            
    /*! Invalid server channel.*/
    rfcomm_disconnect_invalid_server_channel,                    
    /*! Unknown primitive encountered.*/
    rfcomm_disconnect_unknown_primitive,                        
    /*! Maximum payload exceeded.*/
    rfcomm_disconnect_max_payload_exceeded,                        
    /*! Inconsistent parameters.*/
    rfcomm_disconnect_inconsistent_parameters,                    
    /*! Credit flow control protocol violation.*/
    rfcomm_disconnect_credit_flow_control_protocol_violation,    
    /*! Sink is unknown. */
    rfcomm_disconnect_unknown_sink,
    /*! Diconnection due to L2CAP layer Link Loss. */
    rfcomm_disconnect_l2cap_link_loss,
    /*! Diconnection for any other L2CAP error reason.*/
    rfcomm_disconnect_l2cap_error,
    /*! Disconnection for unknown reason.*/
    rfcomm_disconnect_unknown                                    
} rfcomm_disconnect_status;


/*! 
    @brief RFCOMM Port Negotiation status.
*/
typedef enum
{
    /*! The Port Negotiation was successful.*/
    rfcomm_portneg_success,
    /*! The Sink indicated in the Port Neg Ind was unknown.*/
    rfcomm_portneg_unknown_sink,
    /*! The Port Negotiation failed. */
    rfcomm_portneg_failed
} rfcomm_portneg_status;


/*!
    @brief RFCOMM Control Status
*/
typedef enum
{
    /*! The Control request was successful. */
    rfcomm_control_success,
    /*! The modem signal was invalid. */
    rfcomm_control_invalid_modem_signal,
    /*! The Control request failed. */
    rfcomm_control_failed
} rfcomm_control_status;

/*!
    @brief RFCOMM Line Status error codes
*/
typedef enum
{
    /*! Overrun Error - Received character overwrote an unread character. */
    rfcomm_line_status_overrun = 0x04,
    /*! Parity Error - Received character's parity was incorrect. */
    rfcomm_line_status_parity = 0x02,
    /*! Framing Error - A Character did not terminate with a stop bit. */
    rfcomm_line_status_framing = 0x01
} rfcomm_line_status_error;


/*!
    @brief RFCOMM Line Status Status
*/
typedef enum
{
    /*! The Line Status request was successful. */
    rfcomm_line_status_success,
    /*! The Line Status error code was invalid */
    rfcomm_line_status_invalid_error,
    /*! The RFCOMM conn_id is not related to a valid sink. */
    rfcomm_line_status_invalid_sink,
    /*! The Control request failed. */
    rfcomm_line_status_failed
} rfcomm_line_status;


/*!
    @brief Man In The Middle (MITM) setting for 
    ConnectionSmIoCapabilityResponse().
*/
typedef enum
{
    /*! Do not use MITM. */
    mitm_not_required,
    /*! Use MITM, connection will fail if IO capability cannot support it. */
    mitm_required,
    /*! Use MITM, unless IO capability indicates it cannot be supported. */
    mitm_flexible
} mitm_setting;


/*!
    @brief Out Of Band (OOB) setting for ConnectionSmIoCapabilityResponse().
*/
typedef enum
{
    /*! No Out Of Band data */
    oob_data_none = 0x00,
    /*! P192 elliptic curve encryption Out Of Band Data - legacy pairing*/
    oob_data_p192 = 0x01,
    /*! P256 elliptic curve encryption Out of Band Data */
    oob_data_p256 = 0x02
} oob_data_setting;

/*!
    @brief Authenticated Payload Timeout Event registration.
    This determines whether the application or BlueStack deals with the event
    or whether it is simply ignored. 
*/
typedef enum
{ 
    /*! The event will be dropped by BlueStack with no further action taken. */
    cl_apt_ignore,
    /*! The event will be internally handled by BlueStack, resulting in a 
      disconnection of the link. */
    cl_apt_bluestack,
    /*! BlueStack will forward the event to the application and take no further 
      action. The application may then decide which action to take. */
    cl_apt_application
} cl_apt_route_event;

    
/*! 
    @brief The message filter is a bitmap that controls which status messages 
    the connection library will send to the application.  Each bit controls one
    or more messages, when the bit is set the connection library will send the 
    message to the application.  See msg_group enum for definitions of the 
    message groups.
*/
typedef uint16 msg_filter;


/*! 
    @brief Message filter groups.  Defines which status messages will be sent 
    to the application.  The msg_filter will have a combination of these values
    bitwise Or'ed together.
*/
typedef enum
{
    /*! Send CL_DM_ACL_OPEN_IND/CL_DM_ACL_CLOSED_IND messages to application.*/
    msg_group_acl = 0x0001,

    /*! Send CL_DM_MODE_CHANGE_EVENT message to application. */
    msg_group_mode_change = 0x0002,

    /*! Send CL_DM_SYNC_CONNECT_CFM message to the application */
    msg_group_sync_cfm = 0x0004
} msg_group;

/*! 
    @brief The base number for connection library messages.
*/
/*
    Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
    CL_INIT_CFM = CL_MESSAGE_BASE,
    CL_DM_ROLE_CFM,
    CL_DM_ROLE_IND,
    CL_DM_LINK_SUPERVISION_TIMEOUT_IND,
    CL_DM_SNIFF_SUB_RATING_IND,

    CL_DM_INQUIRE_RESULT,
    CL_DM_REMOTE_NAME_COMPLETE,
    CL_DM_LOCAL_NAME_COMPLETE,
    CL_DM_READ_INQUIRY_TX_CFM,
    CL_DM_READ_TX_POWER_CFM,
    CL_DM_CLASS_OF_DEVICE_CFM,

    CL_DM_SYNC_REGISTER_CFM,
    CL_DM_SYNC_UNREGISTER_CFM,
    CL_DM_SYNC_CONNECT_CFM,
    CL_DM_SYNC_CONNECT_IND,
    CL_DM_SYNC_DISCONNECT_IND,
    CL_DM_SYNC_RENEGOTIATE_IND,
    
    CL_DM_LOCAL_BD_ADDR_CFM,
    CL_DM_LINK_QUALITY_CFM,
    CL_DM_RSSI_CFM,
    CL_DM_RSSI_BDADDR_CFM,
    CL_DM_REMOTE_FEATURES_CFM,
    CL_DM_LOCAL_VERSION_CFM,
    CL_DM_REMOTE_VERSION_CFM,
    CL_DM_CLOCK_OFFSET_CFM,
    CL_DM_READ_BT_VERSION_CFM,
    CL_DM_ACL_OPENED_IND,
    CL_DM_ACL_CLOSED_IND,
    CL_DM_APT_IND,
    CL_DM_READ_APT_CFM,
    CL_DM_WRITE_APT_CFM,

    CL_SM_INIT_CFM,                      
    CL_SM_REGISTER_OUTGOING_SERVICE_CFM,
    CL_SM_READ_LOCAL_OOB_DATA_CFM,
    CL_SM_AUTHENTICATE_CFM,
    CL_SM_SECURITY_LEVEL_CFM,
    CL_SM_SEC_MODE_CONFIG_CFM,
    CL_SM_PIN_CODE_IND,
    CL_SM_IO_CAPABILITY_REQ_IND,
    CL_SM_REMOTE_IO_CAPABILITY_IND,
    CL_SM_USER_CONFIRMATION_REQ_IND,
    CL_SM_USER_PASSKEY_REQ_IND,
    CL_SM_USER_PASSKEY_NOTIFICATION_IND,
    CL_SM_KEYPRESS_NOTIFICATION_IND,
    CL_SM_AUTHORISE_IND,
    CL_SM_ENCRYPT_CFM,
    CL_SM_ENCRYPTION_KEY_REFRESH_IND,
    CL_SM_ENCRYPTION_CHANGE_IND,

    CL_SDP_REGISTER_CFM,
    CL_SDP_UNREGISTER_CFM,
    CL_SDP_OPEN_SEARCH_CFM,
    CL_SDP_CLOSE_SEARCH_CFM,
    CL_SDP_SERVICE_SEARCH_CFM,
    CL_SDP_SERVICE_SEARCH_REF_CFM,
    CL_SDP_ATTRIBUTE_SEARCH_CFM,
    CL_SDP_ATTRIBUTE_SEARCH_REF_CFM,
    CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM,
    CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM,

    CL_L2CAP_REGISTER_CFM,
    CL_L2CAP_UNREGISTER_CFM,
    CL_L2CAP_CONNECT_CFM,
    CL_L2CAP_CONNECT_IND,
    CL_L2CAP_MAP_CONNECTIONLESS_CFM,
    CL_L2CAP_MAP_CONNECTIONLESS_IND,
    CL_L2CAP_UNMAP_CONNECTIONLESS_IND,
    CL_L2CAP_DISCONNECT_IND,
    CL_L2CAP_DISCONNECT_CFM,
    CL_L2CAP_TIMEOUT_IND,

    CL_RFCOMM_REGISTER_CFM,
    CL_RFCOMM_UNREGISTER_CFM,
    CL_RFCOMM_CLIENT_CONNECT_CFM,
    CL_RFCOMM_SERVER_CONNECT_CFM,
    CL_RFCOMM_CONNECT_IND,
    CL_RFCOMM_DISCONNECT_IND,
    CL_RFCOMM_DISCONNECT_CFM,
    CL_RFCOMM_PORTNEG_IND,
    CL_RFCOMM_PORTNEG_CFM,
    CL_RFCOMM_CONTROL_IND,
    CL_RFCOMM_CONTROL_CFM,
    CL_RFCOMM_LINE_STATUS_IND,
    CL_RFCOMM_LINE_STATUS_CFM,

    CL_DM_LINK_POLICY_IND,
    CL_DM_DUT_CFM,
    CL_DM_MODE_CHANGE_EVENT,
    CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM,
    CL_DM_WRITE_INQUIRY_MODE_CFM,
    CL_DM_READ_INQUIRY_MODE_CFM,
    CL_DM_READ_EIR_DATA_CFM,

    CL_SM_GET_ATTRIBUTE_CFM,
    CL_SM_GET_INDEXED_ATTRIBUTE_CFM,
    CL_SM_ADD_AUTH_DEVICE_CFM,
    CL_SM_GET_AUTH_DEVICE_CFM,
    CL_SM_SET_TRUST_LEVEL_CFM,
    
    CL_DM_REMOTE_EXTENDED_FEATURES_CFM,

    /* New BREDR/BLE messages go above here.
     * BLE ONLY messages inside the compile switch
     */
#ifdef ENABLE_BLE_MESSAGES  /* set in connection.h */
    CL_DM_BLE_ADVERTISING_REPORT_IND,
    CL_DM_BLE_SET_ADVERTISING_DATA_CFM,
    CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM,
    CL_DM_BLE_SECURITY_CFM,
    CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM,
    CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM,
    CL_DM_BLE_SET_SCAN_PARAMETERS_CFM,
    CL_DM_BLE_SET_SCAN_RESPONSE_DATA_CFM,
    CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM,
    CL_DM_BLE_CLEAR_WHITE_LIST_CFM,
    CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM,
    CL_DM_BLE_REMOVE_DEVICE_FROM_WHITE_LIST_CFM,
    CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM,
    CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND,
    CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND,
    CL_DM_BLE_ADVERTISING_PARAM_UPDATE_IND,
    CL_SM_BLE_LINK_SECURITY_IND,
    CL_DM_BLE_CHANNEL_SELECTION_ALGORITHM_IND,
    CL_DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM,
    CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM,
    CL_DM_ULP_PHY_UPDATE_IND,
    CL_DM_ULP_SET_PHY_CFM,
    CL_DM_ULP_SET_DEFAULT_PHY_CFM,
    CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND,
    CL_SM_BLE_READ_RANDOM_ADDRESS_CFM,
    CL_DM_BLE_TRANSMITTER_TEST_CFM,
    CL_DM_BLE_RECEIVER_TEST_CFM,
    CL_DM_BLE_TEST_END_CFM,
#endif /* ENABLE_BLE_MESSAGES */
    
    CL_CSB_SET_RESERVED_LT_ADDR_CFM,
    CL_CSB_SET_CSB_CFM,
    CL_CSB_WRITE_SYNC_TRAIN_PARAMETERS_CFM,
    CL_CSB_START_SYNC_TRAIN_CFM,
    CL_CSB_SYNC_TRAIN_RECEIVED_CFM,
    CL_CSB_RECEIVE_CFM,
    CL_CSB_RECEIVE_TIMEOUT_IND,
    CL_CSB_TRANSMIT_TIMEOUT_IND,
    CL_CSB_DELETE_RESERVED_LT_ADDR_CFM,
    CL_CSB_AFH_MAP_IND,
    CL_CSB_CHANNEL_MAP_CHANGE_IND,
    CL_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM,
    CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM,
    CL_CRYPTO_ENCRYPT_CFM,
    CL_CRYPTO_HASH_CFM,
    CL_CRYPTO_DECRYPT_CFM,

    CL_L2CAP_TP_CONNECT_CFM,
    CL_L2CAP_TP_CONNECT_IND,
    CL_L2CAP_ADD_CREDIT_CFM,
    CL_MESSAGE_TOP
} ConnectionMessageId;
#endif /*end of DO_NOT_DOCUMENT*/


/*! 
    @brief A Connection Library entity has completed it's initialisation
    process.
*/
typedef struct
{
    connection_lib_status status;        /*!< The connection library status. */
    cl_dm_bt_version      version;        /*!< The host Bluetooth version. */
} CL_INIT_CFM_T;


/*! 
    @brief Return as a result of an attempt to switch or discover the role.
    
    If status is set to hci_success then role contains the currently set role.
*/
typedef struct
{
    Sink        sink;                    /*!< The sink.*/
    hci_status  status;                  /*!< The HCI status.*/
    hci_role    role;                    /*!< The HCI role.*/
} CL_DM_ROLE_CFM_T;


/*! 
    @brief Unsolicited indication of a role change
    
    This message is sent in response to an unsolicited role change even being 
    received from Bluestack. A message is sent for every sink on the affected 
    ACL. If status is set to hci_success then role contains the currently set 
    role.
*/
typedef struct
{
    bdaddr      bd_addr;    /*!< The Bluetooth address of the remote device.*/
    hci_status  status;     /*!< The HCI status.*/
    hci_role    role;       /*!< The HCI role.*/
} CL_DM_ROLE_IND_T;

/*! 
    @brief Notification that we are sniff subrating with the remote device.
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    bdaddr      bd_addr;                
    /*! The HCI Status.*/
    hci_status  status;                 
    /*! The Transmit Latency.*/
    uint16      transmit_latency;       
    /*! The Receive Latency.*/
    uint16      receive_latency;        
    /*! The Remote Sniff Timeout.*/
    uint16      remote_timeout;         
    /*! The Local Sniff Timeout.*/
    uint16      local_timeout;          
} CL_DM_SNIFF_SUB_RATING_IND_T;

/*! 
    @brief Notification that the remote device has changed the link supervision
    timeout.
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    bdaddr        bd_addr;                
    /*! The link supervision timeout.*/
    uint16        timeout;                
} CL_DM_LINK_SUPERVISION_TIMEOUT_IND_T;


/*!
    @brief Specifies that RSSI parameter returned in CL_DM_INQUIRE_RESULT 
    primitive is unknown.
*/
#define CL_RSSI_UNKNOWN ((int16)0x7FFF)

/*! 
    @brief Message informing the client of an inquiry result or that the
    inquiry has completed.

    For each device discovered during an inquiry the Connection library will
    send a CL_DM_INQUIRE_RESULT message to the task that started the
    inquiry. In this case the status field will be set to result.  When the
    inquiry completes (either because it has timed out or the maximum number of
    devices have been discovered) a CL_DM_INQUIRE_RESULT will be sent to the
    client task with the status set to ready.  If the status is set to ready
    then the remaining fields in the message will not be valid.
    
    This message has been extended to include EIR data for BT2.1
*/
typedef struct
{
    /*! Status indicating whether the message contains a valid inquiry result
      or is merely an indication that the inquiry has completed. */
    inquiry_status      status;                    
    /*! Bluetooth address of the discovered device. */
    bdaddr              bd_addr;                
    /*! Class of device of the discovered device. */
    uint32              dev_class;                
    /*! Clock offset of the discovered device. */
    uint16              clock_offset;            
    /*! Page scan repetition mode of the discovered device. */
    page_scan_rep_mode  page_scan_rep_mode;        
    /*! Page scan mode of the discovered device. */
    page_scan_mode      page_scan_mode;        
    /*! RSSI of the discovered device.  Set to CL_RSSI_UNKNOWN if value not 
     available. */
    int16               rssi;
    /*! The size of the EIR data recovered. */
    uint8               size_eir_data;
    /*! The EIR data recovered. */
    uint8               eir_data[1];
} CL_DM_INQUIRE_RESULT_T;


/*! 
    @brief This message indicates that a call to ConnectionReadRemoteName has
    completed.

    A connection library client may request to read the remote name of
    device. This message is returned in response to such a request. If the
    remote name was read successfully then the status field should indicate
    this and the message will contain the name read along with its length.
    NOTES: 1. The maximum length of any name read is 31 bytes.  2. DO NOT 
    Free memory allocated to hold the remote name after use.
*/
typedef struct
{
    /*! Indicates the success or failure of the function call.*/           
    rnr_status  status;                    
    /*! The Bluetooth address of the remote device.*/
    bdaddr      bd_addr;                
    /*! Length of the remote name read.*/
    uint16      size_remote_name;        
    /*! Pointer to the remote name. The client should not attempt to free 
        this pointer, the memory will be freed when the message is 
        destroyed. If the client needs access to this data after the message
        has been destroyed it is the client's responsibility to copy it. */ 
    uint8       remote_name[1];            
} CL_DM_REMOTE_NAME_COMPLETE_T;

/*! 
    @brief This message indicates that a call to ConnectionReadLocalName has
    completed.

    A connection library client may request to read the local name of
    device. This message is returned in response to such a request. If the
    local name was read successfully then the status field should indicate
    this and the message will contain the name read along with its length.
    NOTES: 1. The maximum length of any name read is 31 bytes. 2. DO NOT 
    Free the memory allocated to hold the local name after use.
*/
typedef struct
{
    /*! Indicates the success or failure of the function call.*/           
    hci_status  status;                    
    /*! Length of the local name read.*/
    uint16      size_local_name;        
    /*! Pointer to the remote name. The client should not attempt to free 
        this pointer, the memory will be freed when the message is 
        destroyed. If the client needs access to this data after the message
        has been destroyed it is the client's responsibility to copy it. */ 
    uint8       local_name[1];            
} CL_DM_LOCAL_NAME_COMPLETE_T;


/*! 
    @brief Message containing the local inquiry TX power.

    A connection library client may request to read the local inquiry TX
    power. This message is returned in response to such a request. If the
    local inquiry TX power was read successfully then the status field should
    indicate this and the message will contain a valid inquiry TX power.
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! Indicates whether the request to read the local inquiry TX power was
      completed successfully. */
    hci_status  status;     
    /*! The local inquiry TX power if the request was successful, otherwise
      invalid. */
    int8        tx_power;    
} CL_DM_READ_INQUIRY_TX_CFM_T;

/*! 
    @brief Message containing the device's TX power for the particular
      connection.

    A connection library client may request to read the local device's TX
    power for a particular connection. This message is returned in response to
    such a request. If the local device's TX power was read successfully then 
    the status field should indicate this and the message will contain a valid 
    TX power for that particular connection. The bluetooth address of the
    remote device which requested the read for the TX power is also returned.
*/
typedef struct
{
    /*! Indicates whether the request to read the local device TX power was
      completed successfully. */
    hci_status  status;
    /*! The local device TX power if the request was successful, otherwise
      invalid. */
    int8        tx_power;
    /*! Typed Bd address of the remote device which is accessing the TX 
      power */
    tp_bdaddr tpaddr;
} CL_DM_READ_TX_POWER_CFM_T;


/*! 
    @brief Message containing the local class of device.

    A connection library client may request to read the local class of
    device. This message is returned in response to such a request. If the
    local class of device was read successfully then the status field should
    indicate this and the message will contain a valid device class.
*/
typedef struct
{
    /*! Indicates whether the request to read the local class of device was
      completed successfully. */
    hci_status    status;     
    /*! The local class of device if the request was successful, otherwise
      invalid. */
    uint32        dev_class;    
} CL_DM_CLASS_OF_DEVICE_CFM_T;


/*! 
    @brief Message received after an attempt is made to register for synchronous
    connection notifications.
*/
typedef struct
{
    /*! Indicates the success or failure of the register attempt. */
    connection_lib_status   status; 
} CL_DM_SYNC_REGISTER_CFM_T;


/*! 
    @brief Message received after an attempt is made to de-register from 
    synchronous connection notifications.
*/
typedef struct
{
    /*! Indicates the success or failure of the unregister attempt. */
    connection_lib_status   status; 
} CL_DM_SYNC_UNREGISTER_CFM_T;


/*! 
    @brief Message received due to a call to ConnectionSyncConnect or 
    ConnectionSyncResponse.
*/
typedef struct
{
    /*! Indicates the success or failure of the connection attempt. */
    hci_status       status;         
    /*! Specifies whether a SCO or eSCO packet type was obtained. */
    sync_link_type   link_type;      
    /*! The Bluetooth address of the device. */
    bdaddr           bd_addr;        
    /*! The Synchronous connection sink. */
    Sink             audio_sink;     
    /*! Receive bandwidth. */
    uint32           rx_bandwidth;   
    /*! Transmit bandwidth. */
    uint32           tx_bandwidth;   
    /*! Link Manager SCO Handle. */
    uint16           sco_handle;     
    /*! Wesco reported for the link at time of creation. */
    uint8            wesco;
} CL_DM_SYNC_CONNECT_CFM_T;


/*! 
    @brief Message received when a remote device wishes to establish a 
    connection.
*/
typedef struct
{
    bdaddr      bd_addr;    /*!< Bluetooth address of remote device. */
} CL_DM_SYNC_CONNECT_IND_T;


/*! 
    @brief Message received when the either the local or remote device has 
    attempted a disconnect.
*/
typedef struct 
{
    /*! HCI status code indicating success or failure of the disconnection.*/
    hci_status      status;         
    /*! HCI status code indicating reason for a remote initiated 
     disconnection.*/
    hci_status      reason;         
    /*! The Synchronous connection sink.*/
    Sink            audio_sink;     
} CL_DM_SYNC_DISCONNECT_IND_T;


/*! 
    @brief Message received when the either the local or remote device has 
    attempted a renegotiation of an existing synchronous connection's 
    parameters.
*/
typedef struct 
{
    /*! HCI status code indicating success or failure of the renegotiation.*/
    hci_status      status;         
    /*! The Synchronous connection sink.*/
    Sink            audio_sink;     
} CL_DM_SYNC_RENEGOTIATE_IND_T;


/*! 
    @brief Message received due to a call to ConnectionReadLocalAddr. 

    Contains the local devices Bluetooth address.
*/
typedef struct
{
    /*! HCI status code.*/
    hci_status    status;         
    /*! Bluetooth address of the local device.*/        
    bdaddr        bd_addr;        
} CL_DM_LOCAL_BD_ADDR_CFM_T;    


/*! 
    @brief Message received due to a call to ConnectionGetLinkQuality.
*/
typedef struct
{
    hci_status      status;         /*!< HCI status code.*/
    uint8           link_quality;   /*!< The link quality setting.*/
    Sink            sink;           /*!< The sink.*/
} CL_DM_LINK_QUALITY_CFM_T;


/*! 
    @brief Message received due to a call to ConnectionGetRssi.
*/
typedef struct
{
    hci_status      status;         /*!< HCI status code.*/
    uint8           rssi;           /*!< RSSI value.*/
    Sink            sink;           /*!< The sink.*/
} CL_DM_RSSI_CFM_T;


/*! 
    @brief Message received due to a call to ConnectionReadRemoteSuppFeatures.
*/
typedef struct
{
    hci_status  status;         /*!< HCI status code.*/
    Sink        sink;           /*!< The sink.*/
    uint16      features[4];    /*!< Features.*/
} CL_DM_REMOTE_FEATURES_CFM_T;



/*! 
    @brief Message received due to a call to ConnectionReadRemoteSuppExtFeatures.
*/
typedef struct
{
    hci_status  status;         /*!< HCI status code.*/
    uint8       page_num;       /*!< Feature Page.*/
    uint8       max_page_num;   /*!< Max Supported Page.*/
    uint16      lmp_ext_features[4];    /*!< Features.*/          
    bdaddr      bd_addr;        /*! Bluetooth address of the remote device.*/  
} CL_DM_REMOTE_EXTENDED_FEATURES_CFM_T;


 /*! 
     @brief Message received due to a call to ConnectionReadRemoteVersion.
 */
typedef struct
{
    hci_status      status;             /*!< HCI status code.*/
    hci_version     hciVersion;         /*!< HCI version.*/
    uint16          hciRevision;        /*!< HCI revision.*/
    uint8           lmpVersion;         /*!< The LMP version.*/
    uint16          manufacturerName;   /*!< The manufacturer name.*/
    uint16          lmpSubVersion;      /*!< The LMP sub version.*/
} CL_DM_LOCAL_VERSION_CFM_T;


 /*! 
     @brief Message received due to a call to ConnectionReadRemoteVersion.
 */
typedef struct
{
    hci_status      status;             /*!< HCI status code.*/
    uint8           lmpVersion;         /*!< The LMP version.*/
    uint16          manufacturerName;   /*!< The manufacturer name.*/
    uint16          lmpSubVersion;      /*!< The LMP sub version.*/
} CL_DM_REMOTE_VERSION_CFM_T;


/*! 
    @brief Message received due to a call to ConnectionReadClockOffset.
*/
typedef struct
{
    hci_status      status;         /*!< HCI status code.*/
    Sink            sink;           /*!< The sink.*/
    uint16          clock_offset;   /*!< The clock offset value.*/
} CL_DM_CLOCK_OFFSET_CFM_T;


/*! 
    @brief Message received due to a call to ConnectionReadBtVersion.
*/
typedef struct
{
    hci_status          status;     /*!< HCI status code.*/
    cl_dm_bt_version    version;    /*!< The Host Bluetooth Version.*/
} CL_DM_READ_BT_VERSION_CFM_T;

/*! 
    @brief Unsolicited informational message alerting the client that an ACL
    has been opened.
*/
typedef struct
{
    /*! The typed Bluetooth address of the remote device. */
    typed_bdaddr                    bd_addr;        
    /*! Flag indicating peer-initiated ACL (TRUE) or locally-initiated
      (FALSE). */
    bool                            incoming;        
    /*! The class of device of the remote device. Valid for incoming
      connections only. */
    uint32                          dev_class;
    /*! HCI status code. If the primitive received from Bluestack contains an 
        HCI status code its value will be used to set this field, otherwise the 
        status will be set to hci_error_unrecognised to indicate this field is 
        not valid and should be ignored. */
    hci_status                      status;
    /*! Flags indicating connection type and direction. See DM_ACL_FLAG_* 
        bitfield flags in app/bluestack/dm_prim.h. */
    uint16                          flags;
    /*! BLE Connection Interval. */
    uint16                          conn_interval;           
    /*! BLE Connection Latency. */
    uint16                          conn_latency;       
    /*! BLE Connection supervision timeout.*/
    uint16                          supervision_timeout;
    /*! BLE Connection clock accuracy. */
    uint8                           clock_accuracy;
} CL_DM_ACL_OPENED_IND_T;


/*! 
    @brief Unsolicited informational message alerting the client that an ACL
    has been closed.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device. */
    typed_bdaddr taddr;    
    /*! HCI status code. If the primitive received from Bluestack contains an 
        HCI status code its value will be used to set this field, otherwise the
        status will be set to hci_error_unrecognised to indicate this field is 
        not valid and should be ignored. */
    hci_status  status;
} CL_DM_ACL_CLOSED_IND_T;

/*! 
    @brief Unsolicited informational message alerting the client that no
    authenticated payload has been seen from the peer within the current
    timeout period.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device. */
    tp_bdaddr tpaddr;    
} CL_DM_APT_IND_T;

/*! 
    @brief Sent in response to ConnectionReadAPT()
*/
typedef struct
{
    /*! Set to success if attribute was sucessfully read. */
    connection_lib_status  status;        

    /*! Bluetooth address of the remote device */
    tp_bdaddr              tpaddr;

    /*! Value of the authenticated payload timeout */
    uint16                 apt;
} CL_DM_READ_APT_CFM_T;

/*! 
    @brief Sent in response to ConnectionWriteAPT()
*/
typedef struct
{
    /*! Set to success if attribute was sucessfully written. */
    connection_lib_status  status;        

    /*! Bluetooth address of the remote device */
    tp_bdaddr              tpaddr;
} CL_DM_WRITE_APT_CFM_T;

/*! 
    @brief Message received due to a call to ConnectionWriteInquiryAccessCode. 
*/
typedef struct
{
    /*! HCI status code.*/
    hci_status    status;         
} CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM_T;    

/*! 
    @brief Message received due to a call to ConnectionWriteInquiryMode. 
*/
typedef struct
{
    /*! HCI status code.*/
    hci_status    status;         
} CL_DM_WRITE_INQUIRY_MODE_CFM_T;    

/*! 
    @brief Message received due to a call to ConnectionReadInquiryMode. 
*/
typedef struct
{
    /*! HCI status code.*/
    hci_status      status;
    inquiry_mode    mode;
} CL_DM_READ_INQUIRY_MODE_CFM_T;    

/*! 
    @brief Message received due to a call to ConnectionReadInquiryMode. 
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! HCI status code.*/
    hci_status      status;
    bool            fec_required;
    uint8           size_eir_data;
    uint8           eir_data[1];
} CL_DM_READ_EIR_DATA_CFM_T;

/*!
    @brief Message sent due to a call to ConnectionSmRegisterOutgoingService().

    It returns the security channel to be used for outgoing connections.

    Currently, this is only returned if the protocol of the Outgoing Service is
    RFCOMM.
*/
typedef struct
{
    bdaddr      bd_addr;
    uint16      security_channel;
} CL_SM_REGISTER_OUTGOING_SERVICE_CFM_T;

/*!
    @brief Size of out of band data.
*/
#define    CL_SIZE_OOB_DATA                    (16)

/*! 
    @brief Message received due to a call to ConnectionSmReadLocalOobData.

    This message will only be sent if the local device is BT2.1
    
*/
typedef struct
{
    /*! HCI status code.*/
    hci_status          status;    
    /*! Transport type #TRANSPORT_BREDR_ACL or #TRANSPORT_BLE_ACL. */
    TRANSPORT_T         transport;
    /*! oob_data. */
    oob_data_setting    oob_data;
    /*! Pointer to the hash C value to send the remote device out of band.*/
    uint8               oob_hash_c[CL_SIZE_OOB_DATA];
    /*! Pointer to the rand R value to send the remote device out of band.*/    
    uint8               oob_rand_r[CL_SIZE_OOB_DATA];
} CL_SM_READ_LOCAL_OOB_DATA_CFM_T;

/*! 
    @brief Message received due to a call to ConnectionSmAuthenticate.

    This message has been extended to include extra information for BT2.1
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    bdaddr                  bd_addr;
    /*! The authentication status. */
    authentication_status   status;        
    /*! The type of key generated. */
    cl_sm_link_key_type     key_type;
    /*! The bonding status */
    bool                    bonded;
} CL_SM_AUTHENTICATE_CFM_T;



/*!
    @brief Message sent to indicate LE link security after pairing.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    bdaddr                  bd_addr;
    /*! The ble link security status */
    bool                      le_link_sc;
}CL_SM_BLE_LINK_SECURITY_IND_T;


/*! 
    @brief Message received when the remote device is requesting a pin code.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    typed_bdaddr    taddr;            
} CL_SM_PIN_CODE_IND_T;


/*! 
    @brief Message received when the remote device is requesting IO capability.
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The Bluetooth address, type and transport of the remote device.*/
    tp_bdaddr   tpaddr;            
    /*! If the SMP pairing is over BR/EDR. */
    bool  sm_over_bredr;
} CL_SM_IO_CAPABILITY_REQ_IND_T;

/*! 
    @brief Message detailing the capabilities of the remote device.
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! Remote authentication requirements.*/
    cl_sm_auth_requirements     authentication_requirements;
    /*! The IO capability of the remote device.*/
    cl_sm_io_capability         io_capability;
    /*! If there is any OOB data present on the remote device.*/
    bool                        oob_data_present;
    /*! The Bluetooth address of the remote device.*/
    tp_bdaddr                   tpaddr;    
} CL_SM_REMOTE_IO_CAPABILITY_IND_T;


/*! 
    @brief Message received when the DM is requesting user confirmation that
    the passkey in the message is the same as displayed on the remote dev.
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    tp_bdaddr   tpaddr;
    /*! The value for the user to compare.*/
    uint32      numeric_value;
    /*! TRUE if a response is required. A response must not be sent if FALSE.*/
    bool        response_required;
} CL_SM_USER_CONFIRMATION_REQ_IND_T;

/*! 
    @brief Message received when the DM is requesting the user enter the passkey
    being displayed on the remote device.
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The Bluetooth address, type and transport of the remote device.*/
    tp_bdaddr    tpaddr;
} CL_SM_USER_PASSKEY_REQ_IND_T;

/*! 
    @brief Message received when the DM sends up a passkey to display to the 
    user (to be entered on the remote device)
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    tp_bdaddr   tpaddr;
    /*! The passkey to display.*/
    uint32      passkey;
} CL_SM_USER_PASSKEY_NOTIFICATION_IND_T;

/*! 
    @brief Message received when the DM sends up a passkey to display to the 
    user (to be entered on the remote device)
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    tp_bdaddr               tpaddr;
    /*! The passkey to display.*/
    cl_sm_keypress_type     type;
} CL_SM_KEYPRESS_NOTIFICATION_IND_T;


/*! 
    @brief This is a request from the Bluestack Security Manager for
    authorisation from the application when an untrusted or unknown device is
    attempting to access a service that requires authorisation in security mode
    2.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    bdaddr          bd_addr;            
    /*! The protocol identifier (protocol_l2cap or protocol_rfcomm).*/
    dm_protocol_id  protocol_id;        
    /*!< The channel*/
    uint32          channel;            
    /*! TRUE for incoming connection, FALSE for outgoing connection */
    bool            incoming;           
} CL_SM_AUTHORISE_IND_T;


/*! 
    @brief This message is received in response to a call to
    ConnectionSmEncrypt.
*/
typedef struct
{
    /*! The status of the encrypt request. */    
    connection_lib_status      status;        
    /*! The sink identifying the connection whose encryption status has been
      changed if request was successful. */
    Sink                       sink;        
    /*! Encryption status of this connection, TRUE if encrypted, FALSE
      otherwise. */
    bool                       encrypted;    
    /*! Encryption type of this connection */
    cl_sm_encryption_key_type  encrypt_type;
} CL_SM_ENCRYPT_CFM_T;


/*! 
    @brief This message is received when the encryption key has been refreshed
    (either by a call to ConnectionSmEncryptionKeyRefresh or by the remote dev)
    
    This message will only be sent if the local device is BT2.1
*/
typedef struct
{
    /*! The status of the encryption key refresh request. */    
    hci_status              status;        
    /*! The sink identifying the connection whose encryption has been 
     refreshed. */
    Sink                    sink;        
    /*! Address of remote device. */
    tp_bdaddr               tpaddr;
} CL_SM_ENCRYPTION_KEY_REFRESH_IND_T;


/*! 
    @brief This message is sent in response to a notification from Bluestack of
    a possible change in the encryption status of a link due to an encryption
    procedure initiated by the remote device.
*/
typedef struct
{
    /*! The sink identifying the connection whose encryption status has been
      changed. */
    Sink                       sink;        
    /*! Encryption status of this connection, TRUE if encrypted, FALSE
      otherwise. */
    bool                       encrypted;    
    /*! Encryption type of this connection */
    cl_sm_encryption_key_type  encrypt_type;
    /*! Address, type and transport of remote device. */
    tp_bdaddr                   tpaddr;
} CL_SM_ENCRYPTION_CHANGE_IND_T;


/*! 
    @brief Message sent due to a call to ConnectionSmSetSecurityMode.
*/
typedef struct
{
    bool success;                /*!< Success (TRUE) or Failure (FALSE).*/
} CL_SM_SECURITY_LEVEL_CFM_T;


/*! 
    @brief Message sent due to a call to ConnectionSmSecModeConfig.
*/
typedef struct
{
    /*! TRUE if successfully entered/left debug mode, FALSE otherwise.*/
    bool        success;        
    /*! The new write auth enable setting.*/
    cl_sm_wae   wae;            
    /*! The new access indication setting.*/
    bool        indications;    
    /*! TRUE if we are in debug mode, FALSE otherwise.*/
    bool        debug_keys;     
} CL_SM_SEC_MODE_CONFIG_CFM_T;


/*! 
    @brief Message sent due to a call to ConnectionRegisterServiceRecord.
*/
typedef struct
{
    /*! The connection library status.*/    
    sds_status              status;            
    /*! The service handle.*/
    uint32                  service_handle;    
} CL_SDP_REGISTER_CFM_T;


/*! 
    @brief Message sent due to a call to ConnectionUnregisterServiceRecord
*/
typedef struct
{
    /*! SDS Status.*/
    sds_status              status;                
    /*! The service handle.*/
    uint32                  service_handle;        
} CL_SDP_UNREGISTER_CFM_T;


/*! 
    @brief Message sent due to a call to ConnectionSdpOpenSearchRequest.
*/
typedef struct
{
    sdp_open_status    status;            /*!< The SDP open status.*/
} CL_SDP_OPEN_SEARCH_CFM_T;


/*! 
    @brief Message sent due to a call to ConnectionSdpCloseSearchRequest.
*/
typedef struct
{
    sdp_close_status status;        /*!< The SDP close status.*/
} CL_SDP_CLOSE_SEARCH_CFM_T;


/*! 
    @brief Message sent due to a call to ConnectionSdpServiceSearchRequest.
*/
typedef struct
{
    /*! The SDP search status.*/
    sdp_search_status   status;                
    /*! The number of records.*/
    uint16              num_records;        
    /*! The error code.*/
    uint16              error_code;            
    /*! The Bluetooth address.*/
    bdaddr              bd_addr;            
    /*! The size of the record list.*/
    uint16              size_records;        
    /*! The record list. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */
    uint8               records[1];            
} CL_SDP_SERVICE_SEARCH_CFM_T;

/*! 
    @brief Message sent due to a call to ConnectionSdpServiceSearchRefRequest.
*/
typedef struct
{
    /*! The SDP search status.*/
    sdp_search_status   status;
    /*! The number of records.*/
    uint16              num_records;
    /*! The error code.*/
    uint16              error_code;
    /*! The Bluetooth address.*/
    bdaddr              bd_addr;
    /*! The size of the record list.*/
    uint16              size_records;
    /*! Reference to the record list. The client shall free this pointer when
      the memory is not needed anymore. The memory will not be freed when the
      message is destroyed. */
    uint8               *records;
} CL_SDP_SERVICE_SEARCH_REF_CFM_T;

/*! 
    @brief Message sent due to a call to ConnectionSdpAttributeSearchRequest.
*/
typedef struct
{
    /*! The SDP search status.*/
    sdp_search_status   status;                    
    /*! The error code.*/
    uint16              error_code;                
    /*! The Bluetooth address.*/
    bdaddr              bd_addr;                
    /*! The size of the attribute list.*/
    uint16              size_attributes;        
    /*! The attribute list. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */
    uint8               attributes[1];            
} CL_SDP_ATTRIBUTE_SEARCH_CFM_T;

/*! 
    @brief Message sent due to a call to
    ConnectionSdpAttributeSearchRefRequest.
*/
typedef struct
{
    /*! The SDP search status.*/
    sdp_search_status   status;
    /*! The error code.*/
    uint16              error_code;
    /*! The Bluetooth address.*/
    bdaddr              bd_addr;
    /*! The size of the attribute list.*/
    uint16              size_attributes;
    /*! Reference to the attribute list. The client shall free this pointer
      when the memory is not needed anymore. The memory will not be freed
      when the message is destroyed. */
    uint8               *attributes;
} CL_SDP_ATTRIBUTE_SEARCH_REF_CFM_T;

/*! 
    @brief Message sent due to a call to
    ConnectionSdpServiceSearchAttributeRequest.
*/
typedef struct
{
    /*! The SDP search status.*/
    sdp_search_status   status;                    
    /*! Is more information to come. Yes(TRUE) or No(FALSE).*/
    bool                more_to_come;            
    /*! The error code.*/
    uint16              error_code;                
    /*! The Bluetooth address.*/
    bdaddr              bd_addr;                
    /*! The size of the attribute list returned.*/
    uint16              size_attributes;        
    /*! The attribute list. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */
    uint8               attributes[1];            

} CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T;

/*! 
    @brief Message sent due to a call to
    ConnectionSdpServiceSearchAttributeRefRequest.
*/
typedef struct
{
    /*! The SDP search status.*/
    sdp_search_status   status;
    /*! Is more information to come. Yes(TRUE) or No(FALSE).*/
    bool                more_to_come;
    /*! The error code.*/
    uint16              error_code;
    /*! The Bluetooth address.*/
    bdaddr              bd_addr;
    /*! The size of the attribute list returned.*/
    uint16              size_attributes;
    /*!< Reference to the attribute list. The client shall free this pointer
      when the memory is not needed anymore. The memory will not be freed
      when the message is destroyed. */
    uint8               *attributes;

} CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM_T;

/*! 
    @brief This message is sent in response to a request to register a PSM with
    the Connection library.
*/
typedef struct
{
    /*! If the PSM was successfully registered the status will be set to
      success, otherwise it will be set to fail. */
    connection_lib_status   status;        
    /*! The PSM the client task requested to register. */
    uint16                  psm;        
} CL_L2CAP_REGISTER_CFM_T;


/*! 
    @brief This message is sent in response to a request to unregister a PSM
    from the Connection library.
*/
typedef struct
{
    /*! If the PSM was successfully unregistered the status will be set to
      success, otherwise it will be set to fail.*/
    connection_lib_status   status;        
    /*! The PSM the client requested to unregister. */
    uint16                  psm;        
} CL_L2CAP_UNREGISTER_CFM_T;


/*! 
    @brief This message returns the result of the L2CAP connect attempt.

    This message is returned to both the initiator and acceptor of the L2CAP
    connection and is used to inform them whether the connection was
    successfully established or not.  Once this message has been received the
    connection can be used to transfer data.
*/
typedef struct
{
    /*! Indicates whether or not the connection was successfully
      established. */
    l2cap_connect_status    status;                    
    /*! The local PSM that was connected to. */
    uint16                  psm_local;                
    /*! Sink identifying the connection. The sink is used to send data to the
      remote device and must be stored by the client task. */
    Sink                    sink;                    
    /*! Unique identifier for the connection attempt, allows the client to
      match this CFM message to the response sent to the connection lib where
      multiple connections are being established simultaneously to the same
      device. */
    uint16                  connection_id;   
    /*! The Bluetooth device address of the connecting device. */
    bdaddr                  addr;
    /*! The MTU advertised by the remote device. */
    uint16                  mtu_remote;                
    /*! The flush timeout in use by the remote device. */
    uint16                  flush_timeout_remote;    
    /*! The Quality of Service settings of the remote device. */
    qos_flow                qos_remote;
    /*! The flow mode agreed with the remote device */
    uint8                   mode;
} CL_L2CAP_CONNECT_CFM_T;


/*! 
    @brief This message returns the result of the L2CAP LE COC connect attempt.

    This message is returned to both the initiator and acceptor of the L2CAP
    connection and is used to inform them whether the connection was
    successfully established or not.  Once this message has been received the
    connection can be used to transfer data.
*/
typedef struct
{
    /*! Indicates whether or not the connection was successfully
      established. */
    l2cap_tp_connect_status    status;                    
    /*! The local PSM that was connected to. */
    uint16                  psm_local;                
    /*! Sink identifying the connection. The sink is used to send data to the
      remote device and must be stored by the client task. */
    Sink                    sink;                    
    /*! Unique identifier for the connection attempt, allows the client to
      match this CFM message to the response sent to the connection lib where
      multiple connections are being established simultaneously to the same
      device. */
    uint16                  connection_id;   
    /*! The Bluetooth device address of the connecting device. */
    tp_bdaddr               taddr;
    /*! The MTU advertised by the remote device. */
    uint16                  mtu_remote;                
    /*! The flush timeout in use by the remote device. */
    uint16                  flush_timeout_remote;    
    /*! The Quality of Service settings of the remote device. */
    qos_flow                qos_remote;
    /*! The flow mode agreed with the remote device */
    uint8                   mode;
} CL_L2CAP_TP_CONNECT_CFM_T;


/*! 
    @brief This message returns the result of the L2CAP LE COC add credit req.
*/
typedef struct
{
    /*! Indicates whether or not the connection was successfully
      established. */
    uint16                  status;
    /*! Actual credits extended to remote */
    uint16                  credits;                
    /*! Unique identifier for the connection attempt, allows the client to
      match this CFM message to the response sent to the connection lib where
      multiple connections are being established simultaneously to the same
      device. */
    uint16                  connection_id;   
} CL_L2CAP_ADD_CREDIT_CFM_T;

/*! 
    @brief This message informs the client task of an incoming L2CAP
    connection.

    This message is used to notify the client task that a remote device is
    attempting to create an L2CAP connection to this device. The client task
    that registered the PSM being connected to will be sent this message. This
    message should not be ignored, the client must respond even if it wishes to
    reject the connection attempt.
*/
typedef struct
{
    /*! Bluetooth address of the remote device that initiated the
      connection. */
    bdaddr        bd_addr;            
    /*! Local PSM that the remote device is attempting to connect to. */
    uint16        psm;   
    /*! The channel identifier, should be copied directly into the response 
        function.*/
    uint8         identifier;
    /*! Unique signal identifier for the connection attempt, should be copied 
        directly into the response function.*/
    uint16        connection_id;        
} CL_L2CAP_CONNECT_IND_T;


/*!
    @brief This message informs the client task of an incoming L2CAP LE COC
    connection.

    This message is used to notify the client task that a remote device is
    attempting to create an L2CAP LE COC connection to this device. The client 
    task that registered the PSM being connected to will be sent this message.
    This message should not be ignored, the client must respond even if it 
    wishes to reject the connection attempt.
*/
typedef struct
{
    /*! Bluetooth address of the remote device that initiated the
      connection. */
    tp_bdaddr        taddr;            
    /*! Local PSM that the remote device is attempting to connect to. */
    uint16        psm;   
    /*! The channel identifier, should be copied directly into the response 
        function.*/
    uint8         identifier;
    /*! Unique signal identifier for the connection attempt, should be copied 
        directly into the response function.*/
    uint16        connection_id;        
} CL_L2CAP_TP_CONNECT_IND_T;

/*!
    @brief This message returns the result of a request to map connectionless
    data to a remote device - ConnectionL2capMapConnectionlessRequest().

    This message returns the status of the request and, if successful, a Sink 
    for sending connectionless data.
*/
typedef struct
{
    /*! Indicates if the mapping was successful or the reason for failure. */
    l2cap_map_connectionless_status     status;
    /*! The local PSM used for the mapping. */
    uint16                              local_psm;
    /*! The Sink associated with the mapping, if successful.*/
    Sink                                sink;
    /*! The Bluetooth device address the device mapped to.*/
    bdaddr                              addr;
    /*! The L2CAP Fixed CID used for connectionless data.*/
    uint16                              fixed_cid;
} CL_L2CAP_MAP_CONNECTIONLESS_CFM_T;


/*!
    @brief This message indicates L2CAP connectionless data has been received
    and supplies the Source for the data to be read from.

    For Unicast Data, the ACL connection will remain until unmapped, either by 
    the local or the remote device.
*/
typedef struct
{
    /*! Indicates the type of connectionless data being received.*/
    l2cap_connectionless_data_type      type;
    /*! The Source for the data to be read from.*/
    Source                              source;
    /*! The Bluetooth device address the device mapped to.*/
    bdaddr                              addr;
    /*! The L2CAP Fixed CID used for connectionless data.*/
    uint16                              fixed_cid;
    /*! Local Connectionless PSM.*/
    uint16                              local_psm;
} CL_L2CAP_MAP_CONNECTIONLESS_IND_T;

/*!
    @brief This message indicates that the ACL for Unicast Connectionless Data
    has been closed and that the connectionless data for the remote device 
    associated with the sink has been unmapped.
*/
typedef struct
{
    /*! The sink of the stream associated with the connectionless data and 
     remote device.*/
    Sink                    sink;
    /*! The status or reason for the unmapping of the connectionless data 
    ('disconnect' is not the correct terminology here but these are the result
    codes used by Bluestack). */
    l2cap_disconnect_status status;
}
CL_L2CAP_UNMAP_CONNECTIONLESS_IND_T;

/*! 
    @brief This message informs the App that an L2CAP connection that it
    owns has been disconnected. The sink will remain valid for reading any
    remaining data that may be in the buffer until the App responds to this
    message with a call to ConnectionL2capDisconnectRsp().
*/
typedef struct
{
    /*! L2CAP link identifier which needs to be passed back in the Disconnect 
      response. */
    uint8                       identifier;
    /*! Indicates the L2CAP connection has been disconnected and the status of
      the disconnect. */
    l2cap_disconnect_status     status;        
    /*! Sink identifying the L2CAP connection that was disconnected. */
    Sink                        sink;        
} CL_L2CAP_DISCONNECT_IND_T;

/*! 
    @brief This message is sent in response to the App calling the 
    ConnectionL2capDisconnectReq() function.

    Once this message has  been received the sink is no longer valid and cannot 
    be used to send data to the remote end.
*/
typedef struct
{
    /*! Indicates the L2CAP connection has been disconnected and the status of
      the disconnect. */
    l2cap_disconnect_status     status;        
    /*! Sink identifying the L2CAP connection that was disconnected. */
    Sink                        sink;        
} CL_L2CAP_DISCONNECT_CFM_T;


/* 
    @brief This message informs the client that an L2CAP time out has occurred.
    This can happen during connection or disconnection. The App can ignore this,
    as the there will also be a CFM indicating the timeout as well.
*/
typedef struct
{
    /*! If a sink has been established, the sink the timeout occurred for is
        indicated even though it may no longer be valid. This may be 0 if no
        sink is valid. */
    Sink    sink;
} CL_L2CAP_TIMEOUT_IND_T;


/*! 
    @brief The RFCOMM register confirm message.
*/
typedef struct
{
    /*! The connection library status.*/
    connection_lib_status    status;                
    /*! The server channel.*/
    uint8                    server_channel;        
} CL_RFCOMM_REGISTER_CFM_T;

/*! 
    @brief The RFCOMM unregister confirm message.
*/
typedef struct
{
    /*! The connection library status. */
    connection_lib_status   status;
    /*! The server channel.*/
    uint8                    server_channel;        
} CL_RFCOMM_UNREGISTER_CFM_T;

/*! 
    @brief Message sent due to a call to ConnectionRfcommConnectRequest.
*/
typedef struct
{
    /*! The RFCOMM connection status.*/
    rfcomm_connect_status   status;                
    /*! The server channel.*/
    uint8                   server_channel;        
    /*! The RFCOMM payload size.*/
    uint16                  payload_size;            
    /*! The sink.*/
    Sink                    sink;    
    /*! Device address. */
    bdaddr                  addr;
} CL_RFCOMM_CLIENT_CONNECT_CFM_T;


/*! 
    @brief Message sent due to a call to ConnectionRfcommConnectResponse.
*/
typedef struct
{
    /*! The RFCOMM connection status.*/
    rfcomm_connect_status   status;                
    /*! The server channel.*/
    uint8                   server_channel;        
    /*! The RFCOMM payload size.*/
    uint16                  payload_size;            
    /*! The sink.*/
    Sink                    sink;    
    /*! Device address. */
    bdaddr                  addr;
} CL_RFCOMM_SERVER_CONNECT_CFM_T;


/*! 
    @brief Received when a remote device wishes open an RFCOMM connection.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    bdaddr  bd_addr;                    
    /*! The local server channel.*/
    uint8   server_channel;                
    /*! The sink associated with the connection. */
    Sink    sink;
} CL_RFCOMM_CONNECT_IND_T;


/*! 
    @brief Received when a remote device wishes to close an RFCOMM connection.
*/
typedef struct
{
    /*!< The RFCOMM disconnection status.*/
    rfcomm_disconnect_status    status;    
    /*!< The sink.*/
    Sink                        sink;      
} CL_RFCOMM_DISCONNECT_IND_T;

/*!
    @brief Message sent due to a call to ConnectionRfcommDisconnectReq
*/

typedef struct
{
    /*!< The RFCOMM disconnection status.*/
    rfcomm_disconnect_status    status;
    /*!< The sink.*/
    Sink                        sink;
} CL_RFCOMM_DISCONNECT_CFM_T;

/*!
    @brief RFCOMM Port Negotiation Indication
*/

typedef struct
{
    /*!< The sink.*/
    Sink     sink;
    /*!< Whether this is a Port parameter request (TRUE) or negotiation. */
    bool     request;
    /*!< The port Parameters of the remote device. */
    port_par port_params;
} CL_RFCOMM_PORTNEG_IND_T;

/*!
    @brief RFCOMM Port Negotiation Confirmation
*/

typedef struct
{
    /*!< The RFCOMM Port Negotiation status. */
    rfcomm_portneg_status   status;
    /*!< The sink.*/
    Sink                    sink;
    /*!< The port Parameters of the remote device. */
    port_par                port_params;
} CL_RFCOMM_PORTNEG_CFM_T;
    
/*! 
    @brief Received when a remote device wishes to send control signals.
*/
typedef struct
{
    /*! The sink.*/
    Sink    sink;      
    /*! The break signal, range 0-15 in increments of 200ms.*/
    uint8  break_signal;  
    /*! The modem signal.*/
    uint8  modem_signal;            
} CL_RFCOMM_CONTROL_IND_T;

/*! 
    @brief Received in response to ConnectionRfcommControlSignalRequest.
*/
typedef struct
{
    /*! The sink.*/
    Sink                    sink;       
    /*! The RFCOMM Control Request Status.*/
    rfcomm_control_status   status;     
} CL_RFCOMM_CONTROL_CFM_T;

/*!
    @brief RFCOMM Line Status Confirmation
*/
typedef struct
{
    /*!< The sink.*/
    Sink                    sink;
    /*!< The RFCOMM Line Status request status. */
    rfcomm_line_status      status;
} CL_RFCOMM_LINE_STATUS_CFM_T;

/*!
    @brief RFCOMM Line Status Indication
*/
typedef struct
{
    /*!< The sink.*/
    Sink                        sink;
    /*!< The error flag, TRUE indicates an error is indicated. */
    bool                        error;
    /*!< The line status error code. */
    rfcomm_line_status_error    line_status;
} CL_RFCOMM_LINE_STATUS_IND_T;


/*!
    @brief Sent to the task that initialised the connection library to confirm
    that Device Under Test (DUT) mode has been entered.
*/
typedef struct
{
    /*! Set to success if DUT mode was entered successfully, fail otherwise. */
    connection_lib_status    status;        
} CL_DM_DUT_CFM_T;

/*! 
    @brief Send to the task 
*/
typedef struct
{
    /*! The Bluetooth address of the remote device. */
    bdaddr          bd_addr;        
    /*! New power mode. */
    lp_power_mode   mode;
    /*! Sniff internal. */
    uint16          interval;
} CL_DM_MODE_CHANGE_EVENT_T;

/*! 
    @brief Send to the task that initialised the connection library to send
    back the requested attribute
*/
typedef struct
{
    /*! Set to success if attribute was successfully read, fail otherwise. */
    connection_lib_status   status;        
    /*! Bluetooth address of requested device. */
    bdaddr                  bd_addr;
    /*! The size of the attribute data. */
    uint16                  size_psdata;                
    /*! The attribute data. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */
    uint8                   psdata[1];                    
} CL_SM_GET_ATTRIBUTE_CFM_T;


/*! 
    @brief Send to the task that initialised the connection library to send
    back the requested attribute and Bluetooth address.
*/
typedef struct
{
    /*! Set to success if attribute was successfully read, fail otherwise. */
    connection_lib_status   status;        
    /*! Bluetooth address of requested device. */
    typed_bdaddr            taddr;
    /*! The size of the attribute data. */
    uint16                  size_psdata;                
    /*! The attribute data. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */
    uint8                   psdata[1];                    
} CL_SM_GET_INDEXED_ATTRIBUTE_CFM_T;


/*! 
    @brief Send to the task that initialised the connection library to confirm
    that the Device was added, to the task that called 
    ConnectionSmAddAuthDevice(), after initialisation.
*/
typedef struct
{
    /*! Set to success if device was successfully added. */
    connection_lib_status  status;        

    /*! Bluetooth address of the remote device */
    bdaddr                 bd_addr;
} CL_SM_ADD_AUTH_DEVICE_CFM_T;


/*! 
    @brief Sent in response to ConnectionSetTrustLevel().
*/
typedef struct
{
    /*! Set to success if device was successfully added. */
    connection_lib_status  status;        

    /*! Bluetooth address of the remote device */
    bdaddr                 bd_addr;
} CL_SM_SET_TRUST_LEVEL_CFM_T;

/*! 
    @brief Send to the task that initialised the connection library to confirm
    that the Device was added to the trusted device list.
*/
typedef struct
{
    /*! Set to success if device was found. */
    connection_lib_status   status;        

    /*! Bluetooth address of the remote device */
    bdaddr                  bd_addr;
    /*! Is the device trusted */
    uint16                  trusted;
    /*! Type of link key */
    cl_sm_link_key_type     link_key_type;
    /*! Size of the linkkey */
    uint16                  size_link_key;
    /*! The linkkey. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */
    uint16                  link_key[1];
} CL_SM_GET_AUTH_DEVICE_CFM_T;

/*!
    @brief Send to the registered CSB task to confirm the status of a request
    to reserve an LTADDR.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
    /*! LTADDR reserved. */
    uint16      lt_addr;
} CL_CSB_SET_RESERVED_LT_ADDR_CFM_T;

/*!
    @brief Send to the registered CSB task to confirm the status of a request
    to delete a reserved LTADDR.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
} CL_CSB_DELETE_RESERVED_LT_ADDR_CFM_T;

/*!
    @brief Send to the registered CSB task to confirm the request to start or
    stop transmission of a connectionless slave broadcast.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
    /*! LTADDR of the CSB. */
    uint8       lt_addr;
    /*! Interval of CSB packets. */
    uint16      interval;    
} CL_CSB_SET_CSB_CFM_T;

/*!
    @brief Send to the registered CSB task to confirm the configuration of the
    synchronisation train.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
    /*! Configured sync train interval. */
    uint16      sync_train_ref_interval;    
} CL_CSB_WRITE_SYNC_TRAIN_PARAMETERS_CFM_T;

/*!
    @brief Send to the registered CSB task to confirm that the synchronisation
    train has completed.

    A message isn't received when the sync train is started, instead this message
    is received once the sync train has completed its run for the configured
    #sync_train_timeout period.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
} CL_CSB_START_SYNC_TRAIN_CFM_T;
        
/*!
    @brief Send to the registered CSB task to confirm discovery of the sync train
    and the discovered parameters, or to confirm failure to find the sync train.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
    /*! Bluetooth address of the discovered sync train. */
    bdaddr      bd_addr;
    /*! Clock offset of the CSB. */
    uint32      clock_offset;
    /*! AFH Map of the CSB broadcaster. */
    AfhMap      afh_map;
    /*! LTADDR of the CSB. */
    uint8       lt_addr;
    /*! Next CSB clock. */
    uint32      next_csb_clock;
    /*! Interval between CSB packets. */
    uint16      interval;
    /*! Single octet of application data in the sync train. */
    uint8       service_data;
} CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T;

/*!
    @brief Send to the registered CSB task to confirm status of request to
    receive a connectionless slave broadcast (CSB).
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
    /*! Bluetooth address of the CSB. */
    bdaddr      bd_addr;
    /*! LTADDR of the CSB. */
    uint8       lt_addr;
} CL_CSB_RECEIVE_CFM_T;

/*!
    @brief Send to the registered CSB task to indicate that CSB transmission
    has reached supervision_timeout and failed to transmit a CSB packet.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
    /*! LTADDR of the CSB. */
    uint8       lt_addr;
} CL_CSB_TRANSMIT_TIMEOUT_IND_T;

/*!
    @brief Send to the registered CSB task to indicate the CSB reception has
    timed out, CSB packets have not been received for supervision_timeout slots.
*/
typedef struct
{
    /*! Success status. */
    hci_status  status;
    /*! Bluetoo address of the CSB. */
    bdaddr      bd_addr;
    /*! LTADDR of the CSB. */
    uint8       lt_addr;
} CL_CSB_RECEIVE_TIMEOUT_IND_T;

/*!
    @brief Send to the registered CSB to indicate a new AFH map is available.
*/
typedef struct 
{
    /*! Clock instant at which the map was calculated. */
    uint32 clock;
    /*! New AFH map. */
    AfhMap map;
} CL_CSB_AFH_MAP_IND_T;

/*!
    @brief Send to the registered CSB to indicate a new AFH map has been
    applied.
*/
typedef struct
{
    /*! New map that has been applied. */
    AfhMap map;
} CL_CSB_CHANNEL_MAP_CHANGE_IND_T;

/* These types are used for both BLE and NON-BLE bluestack message
 * handling.
 */

/****************************************************************************
 Public API definition
 ***************************************************************************/

/*! 
    @brief This function is called to initialise the Multipoint Connection
    Manager.

    @param theAppTask The client task.
*/
void ConnectionInit(Task theAppTask );

/*! 
    @brief This function is called to initialise the Multipoint Connection
    Manager.  It also allows the application to define which status messages
    it would like to receive.

    @param theAppTask The client task.
    @param msgFilter Pointer to message filter.  See documentation for 
    msg_filter and msg_group for more details.
*/
void ConnectionInitEx(Task theAppTask, const msg_filter *msgFilter );

/*! 
    @brief This function is called to initialise the Multipoint Connection
    Manager.  It also allows the application to define which status messages
    it would like to receive.
    It also allows the caller to specify a number of devices to store in the 
    Trusted Device List (TDL) This has to be between a minimum of 1 device and 
    a maximum of 8 devices (default = 8)

    @param theAppTask The client task.
    @param msgFilter Pointer to message filter.  See documentation for 
    @param TdlNumberOfDevices The number of devices
    msg_filter and msg_group for more details.
*/
void ConnectionInitEx2(
        Task theAppTask,
        const msg_filter *msgFilter,
        uint16 TdlNumberOfDevices  
        );

/*! @brief This function is called to initialise the Multipoint Connection
    Manager. It allows the application to; define a filter for which messages
    it will receive, define the maximum number of bonded devices to be stored
    in the Trusted Device List, with a minimum of 1 and a maximum of 8, the
    latter being the default, and to define options for configuring the
    upper stack during initialisation.

    @param theAppTask The client task.
    @param msgFilter Pointer to message filter.  See documentation for 
    msg_filter and msg_group for more details.
    @param TdlNumberOfDevices Number of devices to store in the 
    Trusted Device List (TDL)
    @param options Options. See documentation for CONNLIB_OPTIONS_* for
    more details
*/
void ConnectionInitEx3(
        Task theAppTask,
        const msg_filter *msgFilter,
        uint16 TdlNumberOfDevices,
        uint16 options
        );

/*! 
    @brief This function will allow a profile library to configure the Security
    Manager to permit SDP browsing without the need for authentication.

    @param enable By setting this parameter to TRUE the security manager
    disables authentication.  Set to FALSE to re-enable security (i.e. return
    to default setting).

    The default for all connections is to enforce authentication before a
    remote device is permitted to browse service records.
*/
void ConnectionSmSetSdpSecurityIn(bool enable);


/*! 
    @brief This function will allow a profile library to configure the Security
    Manager to permit the local device to initiate SDP browsing without locally
    enforcing authentication.

    @param enable Set this parameter to TRUE to disable SDP security and FALSE
    to re-enable it (i.e. to return to default setting).

    @param bd_addr The Bluetooth address of the remote device. The policy will
    be applied to this remote device only.

    The default for all connections is to enforce Authentication before a
    remote device is permitted to browse service records.
*/
void ConnectionSmSetSdpSecurityOut(bool enable, const bdaddr *bd_addr);


/*! 
    @brief This function changes the current role for a particular connection.
    @param theAppTask The client task.
    @param sink The remote sink of the connection.
    @param role The new role.
    
    A CL_DM_ROLE_CFM message which contains the outcome of this function call
    is returned.
*/
void ConnectionSetRole(Task theAppTask, Sink sink, hci_role role);


/*! 
    @brief This function returns the current role for a particular connection.
    @param theAppTask The client task.
    @param sink The remote sink of the connection.
        
    A CL_DM_ROLE_CFM message containing the current role is generated as a
    result of this function call.
*/
void ConnectionGetRole(Task theAppTask, Sink sink);


/*! 
    @brief Configures the inquiry mode.

    @param theAppTask The client task. This is the task that the inquiry
    results will be sent to. In most cases this will be the task initiating the
    inquiry.
    
    @param mode The mode for any subsequent inquiry operation.
    
    A CL_DM_WRITE_INQUIRY_MODE_CFM message which contains the outcome of this 
    function call is returned.
*/    
void ConnectionWriteInquiryMode(Task theAppTask, inquiry_mode mode);


/*! 
    @brief Obtains the current inquiry mode.

    @param theAppTask The client task. This is the task that the inquiry
    results will be sent to. In most cases this will be the task initiating the
    inquiry.
    
    A CL_DM_READ_INQUIRY_MODE_CFM message which contains the current inquiry 
    mode is generated as a result of this function call.
*/    
void ConnectionReadInquiryMode(Task theAppTask);


/*! 
    @brief Initiate an inquiry to discover other Bluetooth devices.

    @param theAppTask The client task. This is the task that the inquiry
    results will be sent to. In most cases this will be the task initiating the
    inquiry.
    
    @param inquiry_lap The LAP from which the inquiry access code should be
    derived.  See the "Bluetooth Assigned Numbers" document in the Bluetooth
    specification. The General/ Unlimited Inquiry Access Code (GIAC) is
    specified in this document as 0x9e8b33 and this is the code most
    applications will use when performing an inquiry.
    
    @param max_responses Maximum number of responses. Once this many devices
    have been discovered the inquiry process will terminate and the client task
    will be notified. Set to zero to indicate unlimited number of devices. In
    this case the inquiry will terminate when the timeout expires. It is not
    recommended that this field is set to zero or a value higher than ten as
    this may result in firmware stability issues.

    @param timeout Maximum amount of time for the inquiry before it is
    terminated. The timeout is specified as defined in the HCI section of the
    Bluetooth specification.  The time the inquiry is performed for is in fact
    timeout * 1.28 seconds. The allowed values of timeout are in the range 0x01
    to 0x30. This corresponds to an inquiry timeout range of 1.28 to 61.44
    seconds.
    
    @param class_of_device Class of device filter. Set to zero to be notified
    of all devices within range regardless of their class of device. If the
    class_of_device is set only devices with those bits set in their class of
    device will be reported to the client task. This filter can be made as
    specific or as general as necessary. For example, to discover all devices
    with major device class set to audio/ video set the class of device field
    to 0x400. This will result in devices with, for example, class of device
    set to 0x200404 and 0x200408 to be reported.  If, however, we want to limit
    our device search even further and only discover devices with their class
    of device set to headset, for example, the class_of_device field should be
    set to 0x200404. Please note that the filter reports devices that have a
    minimum of the specified bits set, however it is possible that a device
    also has other bits set in its class of device field. In that case that
    device will be reported back to the client task.
        
    This function initiates a Bluetooth inquiry to locate Bluetooth devices
    currently in range and discoverable. It allows a class of device filter to
    be specified so only devices with a specific class of device are reported
    back to the client task.

    Inquiry results are sent to the task specified in the theAppTask parameter
    by sending a CL_DM_INQUIRE_RESULT message with its status set to indicate
    that the message contains a valid result. Once the inquiry process
    completes (either because the maximum number of devices has been reached or
    because the inquiry timeout has expired) a CL_DM_INQUIRE_RESULT message
    with status set to ready will be sent to the client task.
*/
void ConnectionInquire(
        Task theAppTask,
        uint32 inquiry_lap,
        uint8 max_responses,
        uint8 timeout,
        uint32 class_of_device
        );


/*! 
    @brief Initiate an periodic inquiry to discover other Bluetooth devices.

    @param theAppTask The client task. This is the task that the inquiry
    results will be sent to. In most cases this will be the task initiating the
    inquiry.
    
    @param min_period Maximum amount of time specified between consecutive
    inquiries.  The timeout is specified as defined in the HCI section of the
    Bluetooth specification.  The time the inquiry is performed for is in fact
    timeout * 1.28 seconds. The allowed values of timeout are in the range 0x02
    to 0xFFFE. This corresponds to an inquiry timeout range of 2.56 to 83883.52
    seconds.
    
    @param max_period Maximum amount of time specified between consecutive
    inquiries.   The timeout is specified as defined in the HCI section of the
    Bluetooth specification.  The maximum period the inquiry is performed for
    is in fact timeout * 1.28 seconds. The allowed values of timeout are in
    the range 0x03 to 0xFFFF. This corresponds to an inquiry timeout range of 
    3.84 to 83884.8 seconds.
    
    @param inquiry_lap The LAP from which the inquiry access code should be
    derived.  See the "Bluetooth Assigned Numbers" document in the Bluetooth
    specification. The General/ Unlimited Inquiry Access Code (GIAC) is
    specified in this document as 0x9e8b33 and this is the code most
    applications will use when performing an inquiry.
    
    @param max_responses Maximum number of responses. Once this many devices
    have been discovered the inquiry process will terminate and the client task
    will be notified. Set to zero to indicate unlimited number of devices. In
    this case the inquiry will terminate when the timeout expires. It is not
    recommended that this field is set to zero or a value higher than ten as
    this may result in firmware stability issues.

    @param timeout Maximum amount of time for the inquiry before it is
    terminated. The timeout is specified as defined in the HCI section of the
    Bluetooth specification.  The time the inquiry is performed for is in fact
    timeout * 1.28 seconds. The allowed values of timeout are in the range 0x01
    to 0x30. This corresponds to an inquiry timeout range of 1.28 to 61.44
    seconds.
    
    @param class_of_device Class of device filter. Set to zero to be notified
    of all devices within range regardless of their class of device. If the
    class_of_device is set only devices with those bits set in their class of
    device will be reported to the client task. This filter can be made as
    specific or as general as necessary. For example, to discover all devices
    with major device class set to audio/ video set the class of device field
    to 0x400. This will result in devices with, for example, class of device
    set to 0x200404 and 0x200408 to be reported.  If, however, we want to limit
    our device search even further and only discover devices with their class
    of device set to headset, for example, the class_of_device field should be
    set to 0x200404. Please note that the filter reports devices that have a
    minimum of the specified bits set, however it is possible that a device
    also has other bits set in its class of device field. In that case that
    device will be reported back to the client task.
        
    This function initiates a Bluetooth inquiry to locate Bluetooth devices
    currently in range and discoverable. It allows a class of device filter to
    be specified so only devices with a specific class of device are reported
    back to the client task.

    Inquiry results are sent to the task specified in the theAppTask parameter
    by sending a CL_DM_INQUIRE_RESULT message with its status set to indicate
    that the message contains a valid result. Once the inquiry process
    completes (either because the maximum number of devices has been reached or
    because the inquiry timeout has expired) a CL_DM_INQUIRE_RESULT message
    with status set to ready will be sent to the client task.
*/
void ConnectionInquirePeriodic(
        Task theAppTask,
        uint16 min_period,
        uint16 max_period,
        uint32 inquiry_lap,
        uint8 max_responses,
        uint8 timeout,
        uint32 class_of_device
        );


/*! 
    @brief Cancel an active inquiry process.
    @param theAppTask The client task cancelling the inquiry.

    Once the inquiry has been cancelled successfully a CL_DM_INQUIRE_RESULT
    message will be sent to the client task with its status set to ready to
    indicate that the inquiry process has been terminated.
*/
void ConnectionInquireCancel(Task theAppTask);


/*! 
    @brief This function is called to read the remote name of the device with
    the specified Bluetooth device address.
    
    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.

    A CL_DM_REMOTE_NAME_COMPLETE message will be sent to the initiating task on
    completion of the request.
*/
void ConnectionReadRemoteName(Task theAppTask, const bdaddr *bd_addr);


/*! 
    @brief This function is called to cancel the outstanding  
    read remote name request. 
    
    @param theAppTask The client task. This must be the same task provided
    in ConnectionReadRemoteName() otherwise this request will be ignored.
 
    @param bd_addr The Bluetooth address of the remote device.

    A CL_DM_REMOTE_NAME_COMPLETE message will be sent to the initiating task
    with the result of Remote Name Request operation
*/
void ConnectionReadRemoteNameCancel(Task theAppTask, const bdaddr *bd_addr);


/*! 
    @brief This function is called to read the local name of the device.
    
    @param theAppTask The client task.

    A CL_DM_LOCAL_NAME_COMPLETE message will be sent to the initiating task on
    completion of the request.
*/
void ConnectionReadLocalName(Task theAppTask);


/*! 
    @brief This function is called to write the inquiry transmit tx power level.
    
    @param tx_power The tx power to write.

    This is a BT2.1 only feature
*/
void ConnectionWriteInquiryTx(int8 tx_power);


/*! 
    @brief This function is called to read the inquiry response tx power level.
    
    @param theAppTask The client task.

    A CL_DM_READ_INQUIRY_TX_CFM message will be sent to the initiating task on
    completion of the request.

    This is a BT2.1 only feature
*/
void ConnectionReadInquiryTx(Task theAppTask);

/*! 
    @brief This function is called to read the TX power of the bluetooth radio 
       for a particular connection
    
    @param theAppTask The client task.
    @param *tpaddr Pointer to typed bd address of the remote device trying to
    access the TX power

    A CL_DM_READ_TX_POWER_CFM message will be sent to the initiating task on
    completion of the request.
*/
void ConnectionReadTxPower(Task theAppTask, const tp_bdaddr *tpaddr);


/*! 
    @brief This function is called to set the page scan parameters for the
    device.
    
    @param interval The interval to use.
    @param window The window to use.
*/
void ConnectionWritePagescanActivity(uint16 interval, uint16 window);


/*! 
    @brief This function is called to set the inquiry scan parameters for the
    device.
    
    @param interval The interval to use.
    @param window The window to use.
*/
void ConnectionWriteInquiryscanActivity(uint16 interval, uint16 window);

/*! 
    @brief This function is called to set the inquiry access code the device 
    will respond to when in inquiry scan mode.

    @param theAppTask The client task that the result will be returned to. In
    most cases this will be the task originating the request but this does not
    have to be the case.
    @param iac Pointer to table of access codes.
    @param num_iac Number of entries in table.
*/
void ConnectionWriteInquiryAccessCode(
        Task theAppTask,
        const uint32 *iac,
        uint16 num_iac
        );

/*!
    @brief This function is called to set the inquiry scan type.
    @param type Inquiry scan type

    The Inquiry Scan Type configuration parameter indicates whether inquiry
    scanning will be done using non-interlaced scan or interlaced scan.

    Default value: Standard Scan
*/
void ConnectionWriteInquiryScanType(hci_scan_type type);

/*!
    @brief This function is called to set the page scan type.
    @param type Inquiry scan type

    The Page Scan Type configuration parameter indicates whether page
    scanning will be done using non-interlaced scan or interlaced scan.

    Default value: Standard Scan
*/
void ConnectionWritePageScanType(hci_scan_type type);

/*! 
    @brief This function is called to set the scan mode.
    @param mode Scan mode
    - hci_scan_enable_off          0
    - hci_scan_enable_inq          1
    - hci_scan_enable_page         2
    - hci_scan_enable_inq_and_page 3
*/
void ConnectionWriteScanEnable(hci_scan_enable mode);


/*! 
    @brief This function is called to change the local name of the device.
    
    @param size_local_name The length of the local_name string in bytes.
    
    @param local_name The name to change the local name to. This can be a
    maximum of 32 characters.
*/
void ConnectionChangeLocalName(uint16 size_local_name, const uint8 *local_name);


/*!
    @brief This function is called to change the Extended Inquiry Response
    data of the device.

    @param fec_required 0x01 if FEC encoding is required, 0x00 if not. 

    @param size_eir_data The length of the EIR data in bytes. 

    @param eir_data Pointer to the EIR data. The data can be maximum 240 bytes.
    Any data over 240-byte will be ignored.

    This is a BT2.1 only feature
*/
void ConnectionWriteEirData(
        uint8 fec_required,
        uint8 size_eir_data,
        const uint8 *eir_data
        );


/*! 
    @brief This function is called to read the local EIR data.
    
    @param theAppTask The client task.
 
    A CL_DM_READ_EIR_DATA_CFM message will be sent to the initiating task on
    completion of the request.

    This is a BT2.1 only feature
*/
void ConnectionReadEirData(Task theAppTask);


/*! 
    @brief This function is called to set the class of device of the local
    device to the supplied value.
    
    @param cod Class of Device.
*/
void ConnectionWriteClassOfDevice(uint32 cod);


/*! 
    @brief Read the local class of device.

    @param theAppTask The client task that the result will be returned to. In
    most cases this will be the task originating the request but this does not
    have to be the case.

    This function is called to read the local class of device. The value read
    is returned in a CL_DM_CLASS_OF_DEVICE_CFM message.
*/
void ConnectionReadClassOfDevice(Task theAppTask);


/*! 
    @brief This function is called to cache the page mode for a specific
    device.  

    @param bd_addr The Bluetooth address of the remote device.  
    @param page_scan_mode The page scan mode.  
    @param page_scan_rep_mode Page scan repetition mode.
*/
void ConnectionWriteCachedPageMode(
        const bdaddr *bd_addr,
        page_scan_mode page_scan_mode,
        page_scan_rep_mode page_scan_rep_mode
        );


/*! 
    @brief This function is called to cache the clock offset for a specific
    device.

    @param bd_addr The Bluetooth address of the remote device.
    @param clk_offset The clock offset.
*/
void ConnectionWriteCachedClockOffset(const bdaddr *bd_addr, uint16 clk_offset);


/*! 
    @brief Read the local Bluetooth version
 
    @param theAppTask The client task that the result will be returned to. In
    most cases this will be the task originating the request but this does not
    have to be the case.
    
    This function results in a CL_DM_READ_BT_VERSION_CFM message
*/
void ConnectionReadBtVersion(Task theAppTask);


/*! 
    @brief This function is called to clear the cache of parameters stored for
    a specific device.
    
    @param bd_addr The Bluetooth address of the remote device.
*/
void ConnectionClearParameterCache(const bdaddr *bd_addr);


/*! 
    @brief Set the flush timeout parameter for the specified connection. 

    @param sink The sink specifying the connection whose flush timeout
    parameter is being set. The flush timeout is set on a per ACL basis so
    calling this function will affect all profile instances using the ACL.

    @param flush_timeout The flush timeout value to be set. This is specified
    as in the HCI section of the Bluetooth specification. A flush timeout value
    of zero means no automatic flush. The allowed range of values for this
    parameter is 0x0001 - 0x07FF where the timeout is calculated in multiples
    of 625us. This gives an allowed range for the flush timeout of 0.625ms -
    1279.375ms.

    The flush timeout allows ACL packets to be flushed automatically by the
    baseband if the timeout expires and the packet has not bee transmitted (for
    a more comprehensive explanation please refer to the Bluetooth
    specification). By default the flush timeout is set to be infinite which
    means that the retransmissions are carried out until physical link loss
    occurs.
*/
void ConnectionWriteFlushTimeout(Sink sink, uint16 flush_timeout);


/*! 
    @brief This function is used by tasks to indicate to the connection library
    that the task may be setting up a Synchronous connection (or that the device
    it is connected to might initiate one).  
    
    @param theAppTask The client task.
*/
void ConnectionSyncRegister(Task theAppTask);


/*! 
    @brief This function is used by tasks to indicate to the connection library
    that they do not expect to use Synchronous connections. 

    @param theAppTask The client task.
*/
void ConnectionSyncUnregister(Task theAppTask);


/*! 
    @brief The function initiates the creation of a Synchronous connection to a
    remote device.

    @param theAppTask The client task.
    @param sink The remote sink to connect to.
    @param config_params Specifies the connection type and operating parameters.
*/
void ConnectionSyncConnectRequest(
        Task theAppTask,
        Sink sink,
        const sync_config_params *config_params
        );


/*! 
    @brief This function is used to accept or reject an incoming Synchronous 
    connection request.

    @param theAppTask The client task.
    @param bd_addr The address of the remote device, passed to the client in the
    CL_DM_SYNC_CONNECT_IND message
    @param accept The response to indicate acceptance, or otherwise, of the 
    incoming request.
    @param config_params Specifies the connection type and operating parameters.
*/
void ConnectionSyncConnectResponse(
        Task theAppTask,
        const bdaddr *bd_addr,
        bool accept,
        const sync_config_params *config_params
        );


/*! 
    @brief The function requests disconnection of an existing Synchronous 
    connection.

    @param sink The remote sink.
    @param reason The reason for the disconnection.
*/
void ConnectionSyncDisconnect(Sink sink, hci_status reason);


/*! 
    @brief This function is used to adjust the parameters for an existing 
    Synchronous connection.

    @param theAppTask The client task.
    @param sink The remote sink.
    @param config_params Specifies the connection type and operating parameters.
*/
void ConnectionSyncRenegotiate(
        Task theAppTask,
        Sink sink,
        const sync_config_params *config_params
        );


/*! 
    @brief This function reads the local device address.

    A CL_DM_LOCAL_BD_ADDR_CFM message containing the address will be sent as a
    result of this function call.
*/
void ConnectionReadLocalAddr(Task theAppTask);


/*! 
    @brief This function is used to obtain the link quality as defined in the
    HCI specification.

    @param theAppTask The client task.
    @param sink The sink to use.

    A CL_DM_LINK_QUALITY_CFM message will be sent as a result of this function
    call.
*/
void ConnectionGetLinkQuality(Task theAppTask, Sink sink);


/*! 
    @brief This function is used to obtain the RSSI value as defined in the 
    HCI specification.
    @param theAppTask The client task.
    @param sink The sink to use.

    A CL_DM_RSSI_CFM message will be sent as a result of this function call. 
*/
void ConnectionGetRssi(Task theAppTask, Sink sink);

/*! 
    @brief This function is used to obtain the RSSI value as defined in the 
    HCI specification.
    @param theAppTask The client task.
    @param tpaddr The Bluetooth device address, type and transport of the remote 
    device.

    A CL_DM_RSSI_BDADDR_CFM message will be sent as a result of this function
    call. 
*/
void ConnectionGetRssiBdaddr(Task theAppTask, const tp_bdaddr *tpaddr);

/*! 
    @brief Message received due to a call to ConnectionGetRssiBdaddr().
*/
typedef struct
{
    hci_status      status;         /*!< HCI status code.*/
    uint8           rssi;           /*!< RSSI value.*/
    tp_bdaddr       tpaddr;         /*!< The address of the remote device. */
} CL_DM_RSSI_BDADDR_CFM_T;

/*! 
    @brief This function is used to obtain the supported features of the remote
    device.

    @param theAppTask The client task.
    @param sink The sink to use.

    A CL_DM_REMOTE_FEATURES_CFM message will be sent as a result of this
    function call.
*/
void ConnectionReadRemoteSuppFeatures(Task theAppTask, Sink sink);

/*! 
    @brief This function is used to obtain the extended supported features of 
    the remote device.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth device address of the remote device.
    @param page_num Extended Feature page_num of the remote device.

    A CL_DM_REMOTE_EXTENDED_FEATURES_CFM message will be sent as a result of
    this function call.
*/
void ConnectionReadRemoteSuppExtFeatures(Task theAppTask, const bdaddr *bd_addr, uint16 page_num);

/*! 
    @brief This function is used to obtain the version information of the 
    local device.

    @param theAppTask The client task.

    A CL_DM_LOCAL_VERSION_CFM message will be sent as a result of this
    function call.
*/
void ConnectionReadLocalVersion(Task theAppTask);


/*! 
    @brief This function is used to obtain the version information of the 
    remote device.

    @param theAppTask The client task.
    @param sink The sink to use.

    A CL_DM_REMOTE_VERSION_CFM message will be sent as a result of this
    function call.
*/
void ConnectionReadRemoteVersion(Task theAppTask, Sink sink);

/*! 
    @brief This function is used to obtain the version information of the 
    remote device.

    @param theAppTask The client task.
    @param tpaddr The Bluetooth address, type and transport for the remote 
    device.

    A CL_DM_REMOTE_VERSION_CFM message will be sent as a result of this
    function call.
*/
void ConnectionReadRemoteVersionBdaddr(
        Task theAppTask,
        const tp_bdaddr *tpaddr
        );


/*! 
    @brief This function is used to obtain the clock offset of the remote
    device.

    @param theAppTask The client task.
    @param sink The sink to use.
    
    A CL_DM_CLOCK_OFFSET_CFM message will be sent as a result of this function
    call.
*/
void ConnectionReadClockOffset(Task theAppTask, Sink sink);


/*! 
    @brief This function is called to request to place the Bluestack Security
    Manager into the specified security mode.

    @param theAppTask The client task.
    @param sec_mode The security mode to use.
    - sec_mode0_off
    - sec_mode1_non_secure
    - sec_mode2_service
    - sec_mode3_link
    - sec_mode4_ssp
    @param enc_mode The encryption mode to use.
    - hci_enc_mode_off
    - hci_enc_mode_pt_to_pt
    - hci_enc_mode_pt_to_pt_and_bcast
    A CL_SM_SECURITY_LEVEL_CFM message will be sent as a result of this
    function call.

    It is recommended that the security mode be changed only when the system is
    not in the process of creating or accepting connections.  This
    recommendation will be enforced by the Security Manager. Once a device has 
    entered sec_mode4_ssp the Security Manager will not allow it to change to
    a legacy security mode.
*/
void ConnectionSmSetSecurityMode(
        Task theAppTask,
        dm_security_mode sec_mode,
        encryption_mode enc_mode
        );


/*! 
    @brief This function is called to register the security requirements for
    a service when the Bluestack Security Controller is in Security Mode 4. 
    Security requirements registered using this function can be unregistered 
    using ConnectionSmUnRegisterIncomingService

    @param protocol_id The protocol ID
    
    @param channel The channel or PSM to which the security level is to be 
    applied
    
    @param ssp_sec_level The default security level for Bluestack to use
    - ssp_secl4_l0
    - ssp_secl4_l1
    - ssp_secl4_l2
    - ssp_secl4_l3
    - ssp_secl4_l4
    
    @param outgoing_ok Set this TRUE to set the level for incoming AND outgoing
           connections, FALSE for just incoming connections. This parameter does
           not apply when setting the default level.
           
    @param authorised Set this TRUE to require authorisation, FALSE otherwise
    
    @param disable_legacy Set this TRUE to block pairing with legacy devices, 
           FALSE to enable pairing with legacy devices. 
*/
void ConnectionSmSetSecurityLevel(
        dm_protocol_id protocol_id,
        uint16 channel,
        dm_ssp_security_level ssp_sec_level,
        bool outgoing_ok,
        bool authorised,
        bool disable_legacy
        );


/*! 
    @brief This configures the DM's security mode settings

    @param theAppTask The client task.
    @param write_auth_enable Determines under which conditions we tear down 
    an existing ACL to pair with mode 3 legacy devices during link setup
    @param debug_keys Set TRUE to enable use of debug keys
    @param legacy_auto_pair_missing_key Enables pairing on authentication 
    failure with key missing status for legacy devices. This option should only 
    be used to maintain interoperability where it is not possible to prompt the 
    user to delete the link key manually and retry.it is recommended to keep 
    this Value TRUE to maintain interoperability with legacy devices. 
    
    A CL_SM_SEC_MODE_CONFIG_CFM message will be sent as a result of this
    function call.
*/
void ConnectionSmSecModeConfig(
        Task theAppTask,
        cl_sm_wae write_auth_enable,
        bool debug_keys,
        bool legacy_auto_pair_missing_key
        );


/*! 
    @brief This function is called to register the security requirements for
    access to a service when the Bluestack Security Controller is in Security
    Mode 2 or 3.

    @param protocol_id The protocol identifier (protocol_l2cap or
    protocol_rfcomm).
    
    @param channel Channel for the protocol defined by the protocol_id that the
    access is being requested on (e.g. RFCOMM server channel number).
    
    @param security - A bitwise setting of the security to be used. See the
    documentation for dm_security_in.
    

    The registered security level is applied to all incoming connections on the
    specified 'channel'.
*/
void ConnectionSmRegisterIncomingService(
        dm_protocol_id protocol_id,
        uint16 channel,
        dm_security_in security
        );


/*! 
    @brief This function is called to unregister the security requirements for
    a service previously registered.
    
    @param protocol_id The protocol identifier (protocol_l2cap or
    protocol_rfcomm).
    
    @param channel Channel for the protocol defined by the protocol_id that the
    access is being requested on (e.g. RFCOMM server channel number).
*/
void ConnectionSmUnRegisterIncomingService(
        dm_protocol_id protocol_id,
        uint16 channel
        );


/*! 
    @brief This function is called to register the security requirements
    outgoing connections on the specified protocol and channel on the specified
    remote device. For the RFCOMM protocol a CL_SM_REGISTER_OUTGOING_SERVICE_CFM
    message will be received, confirming the channel. For L2CAP, there is no CFM
    message.
    @param theAppTask The client task, this is only essential when defining an
    outgoing service for the RFCOMM protocol.
    @param bd_addr The Bluetooth address of the remote device.
    @param protocol_id The protocol identifier (protocol_l2cap or
    protocol_rfcomm).
    @param channel Security channel for the protocol defined by the protocol_id 
    that the access is being requested on. For RFCOMM, this can be generated 
    by Bluestack for an outgoing connection by setting the value to 0. The 
    generated channel is returned in the CL_SM_REGISTER_OUTGOING_SERVICE_CFM
    message.
    @param security - A bitwise setting of the security to be used. See the
    documentation for dm_security_out.

    This is typically used to control security when connecting to a remote
    RFCOMM server channel.
*/
void ConnectionSmRegisterOutgoingService(
        Task theAppTask,
        const bdaddr* bd_addr,
        dm_protocol_id protocol_id,
        uint16 channel,
        dm_security_out security
        );

/*! 
    @brief Specialises ConnectionSmRegisterOutgoingService() for defining an 
    L2CAP Outgoing service for which the App Task does not need to be defined.
*/

#define ConnectionSmRegisterOutgoingL2cap(bd_addr,channel,security_level) \
    ConnectionSmRegisterOutgoingService( \
            NULL, \
            (bd_addr), \
            protocol_l2cap, \
            (channel), \
            (security_level) \
            )

/*! 
    @brief Specialises ConnectionSmRegisterOutgoingService() for defining an 
    RFCOMM Outgoing service for which the channel does not need to be defined.
    NOTE: If an RFCOMM channel is to be recommended then the 
    ConnectionSmRegisterOutgoingService() function should be used instead.
*/

#define ConnectionSmRegisterOutgoingRfcomm(theAppTask,bd_addr,security_level) \
    ConnectionSmRegisterOutgoingService( \
            (theAppTask), \
            (bd_addr), \
            protocol_rfcomm, \
            0, \
            (security_level) \
            )

/*! 
    @brief This function is called to unregister the security requirements for
    a service that was previously registered.

    @param bd_addr The Bluetooth address of the remote device.

    @param protocol_id The protocol identifier (protocol_l2cap or
    protocol_rfcomm).

    @param channel Channel for the protocol defined by the protocol_id that the
    access is being requested on (e.g. RFCOMM server channel number).
*/
void ConnectionSmUnRegisterOutgoingService(
        const bdaddr* bd_addr,
        dm_protocol_id protocol_id,
        uint16 channel
        );


/*! 
    @brief This function is called to retrieve a new set of Out Of Band 
    parameters to be exchanged in an out of band inquiry. Only the most recently
    retrieved hash C and rand R may be used by the remote device during pairing.
    Before a new OOB transfer this command must be used to obtain a new hash C
    and rand R. 

    @param theAppTask The client task.
    @param transport The ACL transport type, BREDR or BLE.

    A CL_SM_READ_LOCAL_OOB_DATA_CFM message will be sent as a result of this
    function call.
*/
void ConnectionSmReadLocalOobData(Task theAppTask, TRANSPORT_T transport);


/*! 
    @brief This function is called to authenticate a remote device.

    @param theAppTask The client task.

    @param bd_addr The Bluetooth address of the remote device.

    @param timeout Specifies the maximum amount of time in seconds for the
    bonding process. If this time is exceeded then the process is aborted.
    This value must be set to a minimum of 60 seconds to allow 30 seconds
    for the IO capability response and 30 seconds for user input, less than
    this would violate the 2.1 spec. The recommended value for this timeout
    is 90 seconds.

    This normally involves going through the Bonding or Pairing process.  

    A CL_SM_AUTHENTICATE_CFM message will be sent as a result of this function
    call.
*/
void ConnectionSmAuthenticate(
        Task theAppTask,
        const bdaddr* bd_addr,
        uint16 timeout
        );


/*! 
    @brief This function is called to cancel an authentication request

    @param theAppTask The client task.

    @param force Force bringing down the ACL to end the bonding process
    even if L2CAP connections are open.
*/
void ConnectionSmCancelAuthenticate(Task theAppTask, bool force);


/*! 
    @brief This function is called to enable /disable encryption on a link
    where security modes 2 or 3 are active.
    
    @param theAppTask The client task.

    @param sink The sink to use.

    @param encrypt Set to TRUE to enable encryption on the link specified by
    sink, or to FALSE to disable it.

    If this function is called with the security mode set to 1, the 
    request will be rejected.

    A CL_SM_ENCRYPT_CFM message will be sent as a result of this function call.
*/
void ConnectionSmEncrypt(Task theAppTask, Sink sink, uint16 encrypt);


/*! 
    @brief This function is called to refresh the encryption key on a link

    @param taddr The Bluetooth address of the remote device.

    This is a BT2.1, or greater, feature. TYPED_BDADDR_RANDOM address types can 
    only be used with BT4.0 or above.  

    A CL_SM_ENCRYPTION_KEY_REFRESH_IND message will be sent as a result of this
    function call.
*/
void ConnectionSmEncryptionKeyRefresh(const typed_bdaddr* taddr);


/*! 
    @brief This function is called to refresh the encryption key on a link

    @param sink The sink to use.

    This is a BT2.1 only feature

    A CL_SM_ENCRYPTION_KEY_REFRESH_IND message will be sent as a result of this
    function call.
*/
void ConnectionSmEncryptionKeyRefreshSink(Sink sink);


/*! 
    @brief This function is called to get the current APT value.

    @param theAppTask The application task

    @param tp_addrt The Bluetooth address of the remote device.

    A CL_DM_READ_APT_CFM message will be sent as a result of this
    function call.
*/
void ConnectionReadAPT(Task theAppTask, const tp_bdaddr* tp_addrt);

/*! 
    @brief This function is called to set the current APT value.

    @param theAppTask The client application task

    @param tp_addrt The Bluetooth address of the remote device.

    @param apt The APT value to set

    @param apt_route_event How to handle the APT Expired event.
    See: #cl_apt_route_event

    A CL_DM_WRITE_APT_CFM message will be sent as a result of this
    function call.
*/
void ConnectionWriteAPT(Task theAppTask, 
                        const tp_bdaddr* tp_addrt, 
                        uint16 apt,
                        cl_apt_route_event apt_route_event);

/*! 
    @brief This function is called to respond to DM_SM_AUTHORISE_IND. 

    @param bd_addr The Bluetooth address of the remote device.

    @param protocol_id The protocol identifier (protocol_l2cap or
    protocol_rfcomm).

    @param channel Channel for the protocol defined by the protocol_id that the
    access is being requested on (e.g. RFCOMM server channel number).  

    @param incoming TRUE for incoming connection request. FALSE for outgoing
    connection request

    @param authorised TRUE for granted. FALSE for refused.

    This is a request from the Bluestack Security Manager for authorisation 
    from the application when an untrusted or unknown device is attempting 
    to access a service that requires authorisation in security mode 2.
*/
void ConnectionSmAuthoriseResponse(
        const bdaddr* bd_addr,
        dm_protocol_id protocol_id,
        uint32 channel,
        bool incoming,
        bool authorised
        );


/*! 
    @brief This function is called in response to CL_SM_PIN_CODE_IND

    @param taddr The Bluetooth address of the remote device.
    @param size_pin_code The length of the pin code.
    @param pin_code The pin code to use.

    If the Security Manager does not have a link key code for the required
    device then it will request a pin code from the application task.  If the
    pin code is not entered then the length will be set to 0.
*/
void ConnectionSmPinCodeResponse(
        const typed_bdaddr* taddr,
        uint8 size_pin_code,
        const uint8* pin_code
        ); 


/*!
    @brief This function is called to change the link key for a connection with
    the specified device
    
    @param sink The sink to use.
    
    On return the link key will have been changed (currently the change complete
    message from Bluestack is ignored)
*/
void ConnectionSmChangeLinkKey(Sink sink);



/*!
    @brief This function is called to open an ACL connection. 
    
    @param bd_addr The Bluetooth address of the device to create ACL connection 
    with.
*/
void ConnectionDmAclOpen(
        const bdaddr* bd_addr
        );
              

/*!
    @brief This function is called to close the ACL connection. 
    
    @param bd_addr The Bluetooth address of the device to close ACL connection 
    with.

    @param detach_all If TRUE then close ALL ACL connections, the bd_addr is 
    ignored.
*/
void ConnectionDmAclClose(
        const bdaddr* bd_addr,
        bool detach_all
        );              
              
/*!
    @brief This function is called to force close the ACL connection
    irrespective of any L2CAP connections that are active.
    Do not use this if it can be avoided.
    
    @param bd_addr The Bluetooth address of the device to force close ACL  
    connection with.
    
    @param reason The HCI Reason code

    @param detach_all If TRUE then force close ALL ACL connections, the bd_addr
    is ignored.
*/
void ConnectionDmAclDetach(
        const bdaddr* bd_addr,
        hci_status reason,
        bool detach_all
        );

/*! 
    @brief This function is called to remove an authenticated device from the
    paired device list.

    @param type The address type - Public or Random.
    @param bd_addr The Bluetooth address of the remote device.

    On return, the device will have been deleted from the paired device list.
*/
void ConnectionSmDeleteAuthDeviceReq(uint8 type, const bdaddr* bd_addr);

/*!
    @brief Specialises ConnectionSmDeleteAuthDeviceReq() to provide backward 
    compatibilty for BR/EDR only connections.
*/
#define ConnectionSmDeleteAuthDevice(bd_addr) \
            ConnectionSmDeleteAuthDeviceReq(\
                TYPED_BDADDR_PUBLIC, \
                (bd_addr) )

/*! 
    @brief This function is called to remove all authenticated devices from the
    paired device list and any associated attribute data, if used (see the 
    ConnectionSmPutAttributeReq() function).

    @param ps_base User persistent data is now stored in the connection
    library's private PS Keys and so this parameter no longer has any effect. 
    It is kept for backwards compatibility and will be deprecated in the future.

    On return, all devices will have been deleted from the paired device list.
    The user persistent attribute data associated with a device will also be 
    deleted.
*/
void ConnectionSmDeleteAllAuthDevices(uint16 ps_base);


/*!
    @brief This function is called to add an authenticated device to the paired
    device list.

    @param theAppTask The client task.
    @param peer_bd_addr The Bluetooth address of the remote device.
    @param key_type The type of link key
    @param size_link_key Size of link key in octets.
    @param link_key Pointer to link key.
    @param trusted TRUE(trusted) or FALSE(not trusted).
    @param bonded TRUE(bonded) or FALSE(not bonded).
*/
void ConnectionSmAddAuthDevice(
        Task theAppTask,
        const bdaddr *peer_bd_addr,
        uint16 trusted,
        uint16 bonded,
        uint8 key_type,
        uint16 size_link_key,
        const uint16* link_key
        );

/*!
    @brief This function is called to get an authenticated device from the 
    paired device list.

    @param theAppTask The client task.
    @param peer_bd_addr The Bluetooth address of the remote device.
    
    A CL_SM_GET_AUTH_DEVICE_CFM message will be sent to the client task 
    containing the device information.
*/
void ConnectionSmGetAuthDevice(Task theAppTask, const bdaddr *peer_bd_addr);


/*! 
    @brief This function is called in response to 
    #CL_SM_IO_CAPABILITY_REQ_IND_T.

    If Out Of Band (OOB) from the remote device is to be set in this function,
    the P256 (secure) must always be the first part of that OOB data. If both
    P256 and P192 are used then the oob_rand_r and oob_hash_c parameters must
    contain both, except in the case of P192 encryption for BLE, when only 
    HASH_C OOB data is present.

    For BR/EDR connections (transport = TRANSPORT_BREDR_ACL).

    oob_data_setting    | oob_rand_r              | oob_hash_c
    --------------------|-------------------------|-------------------------    
    p192 (legacy)       | p192 r_rand (16-octets) | p192 hash_c (16-octets)
    p256 (secure)       | p256 r_rand (16-octets) | p256 hash_c (16-octets)
    p192 and p256       | p256 + p192 (32-octets) | p256 + p192 (32-octets)

    For BLE connections (transport = TRANSPORT_BLE_ACL) and p192 legacy pairing,
    only the HASH_C OOB data is used. 

    oob_data_setting    | oob_rand_r              | oob_hash_c
    --------------------|-------------------------|-------------------------    
    p192 (legacy)       | no date (0-octets)      | p192 hash_c (16-octets)
    p256 (secure)       | p256 r_rand (16-octets) | p256 hash_c (16-octets)
    p192 and p256       | p256 (16-octets)        | p256 + p192 (32-octets)

    @param tpaddr The Bluetooth address, type and transport of the remote 
    device.
    @param io_capability The IO capability of the device - set this to send a 
    negative response.
    @param mitm Indicates whether Man In The Middle protection should be used
    if the IO capability of both devices allow it (mitm_flexible), must be used
    (mitm_required), which may cause pairing to fail if the remote device cannot 
    support the required IO, or MITM is not required (mitm_not_required).
    @param bonding If this is set TRUE the CL will consider the remote device
    bonded. If this is set FALSE the CL will consider the remote device non
    bonded, or will reject the IO capability request if dedicated bonding is in
    progress, whether initiated locally or remotely.  Bonded devices will be 
    added to the Trusted Device List and registered with the DM on successful 
    completion of pairing.  Non Bonded devices will be registered with the DM 
    on successful completion of pairing.
    @param key_distribution Indicates the BLE link keys to be distributed for a
    connection of that type. ENC_CENTRAL is required for bonding and need only 
    be set for the initiator. See the Bluetooth low energy link key distribution 
    flags;
        #KEY_DIST_NONE,
        #KEY_DIST_RESPONDER_ENC_CENTRAL,
        #KEY_DIST_INITIATOR_ENC_CENTRAL
    @param oob_data This is a bit field parameter. If set to #oob_data_none, 
    this equivalent to setting to field to 0. Either or both #oob_data_p192 
    (legacy) and #oob_data_p256 (secure) can be defined. 
    @param oob_hash_c The remote device's out of band hash value (must be set 
    NULL if unavailable). For BLE legacy pairing, only the oob_hash_c is 
    required. The data pointed to by this parameter is copied.
    @param oob_rand_r The remote device's out of band random value (must be 
    set NULL if unavailable). The data pointed to by this parameter is copied.

*/
void ConnectionSmIoCapabilityResponse(
        const tp_bdaddr*    tpaddr,
        cl_sm_io_capability io_capability,
        mitm_setting        mitm,
        bool                bonding,
        uint16              key_distribution,
        uint16              oob_data,
        uint8*              oob_hash_c,
        uint8*              oob_rand_r
        ); 


/*! 
    @brief This function is called in response to 
    CL_SM_USER_CONFIRMATION_REQ_IND message, if the response_required flag is 
    set to TRUE in the message.

    @param tpaddr The Bluetooth address, type and transport of the remote 
    device.
    @param confirm TRUE for confirmed, FALSE otherwise

    This is a BT2.1 only feature
*/
void ConnectionSmUserConfirmationResponse(
        const tp_bdaddr*    tpaddr,
        bool                confirm
        );
    

/*! 
    @brief This function is called in response to CL_SM_USER_PASSKEY_REQ_IND

    @param tpaddr The Bluetooth address of the remote device.

    @param cancelled TRUE if user cancelled entry, FALSE otherwise

    @param passkey The passkey the user entered

    This is a BT2.1 only feature
*/
void ConnectionSmUserPasskeyResponse(
        const tp_bdaddr*    tpaddr,
        bool                cancelled,
        uint32              passkey
        );


/*! 
    @brief This function is called to notify the remote device
    of a user key press during passkey entry.

    @param tp_addr The Bluetooth address of the remote device.

    @param type The type of key press to notify remote device of

    This is a BT2.1 only feature
*/
void ConnectionSmSendKeypressNotificationRequest(
        const tp_bdaddr*    tp_addr,
        cl_sm_keypress_type type
        );


/*! 
    @brief This function is called to write attribute data to the persistent
    store assuming an entry for the device already exists in the
    paired device list.

    This function replaces ConnectionSmPutAttribute(), which did not support
    typed_bdaddr addresses. A macro for the ConnectionSmPutAttribute() has been
    added for backwards compatibility that defaults to the TYPED_BDADDR_PUBLIC
    address type.

    @param ps_base User persistent data is now stored in the connection
    library's private PS Keys and so this parameter no longer has any effect. 
    It is kept for backwards compatibility and will be deprecated in the future.
    
    @param addr_type The address type - Public or Random.
    @param bd_addr The Bluetooth address of the device
    @param size_psdata The length of the data to be written
    @param psdata Pointer to the data

    On return, the data will have been written into the persistent store
    VM Connection Library data key Attribute base  + index of the device 
    in the trusted device list
 
*/
void ConnectionSmPutAttributeReq(
        uint16 ps_base,
        uint8 addr_type,
        const bdaddr* bd_addr,
        uint16 size_psdata,
        const uint8* psdata
        );

/*!
    @brief Specialises ConnectionSmPutAttributeReq() to provide backward 
    compatibilty for BR/EDR only connections.
*/
#define ConnectionSmPutAttribute(ps_base, bd_addr, size_psdata, psdata) \
            ConnectionSmPutAttributeReq( \
                    (ps_base), \
                    TYPED_BDADDR_PUBLIC, \
                    (bd_addr), \
                    (size_psdata), \
                    (psdata))

/*! 
    @brief This function is called to read attribute data from the persistent
    store assuming an entry for the device already exists in the
    paired device list.

    This function replaces ConnectionSmGetAttribute(), which did not support
    typed_bdaddr addresses. A macro for the ConnectionSmGetAttribute() has been
    added for backwards compatibility that defaults to the TYPED_BDADDR_PUBLIC
    address type.

    @param ps_base User persistent data is now stored in the connection
    library's private PS Keys and so this parameter no longer has any effect. 
    It is kept for backwards compatibility and will be deprecated in the future.
    
    @param addr_type The address type - Public or Random.
    @param bd_addr The Bluetooth address of the device
    @param size_psdata The length of the data to be written

    A CL_SM_GET_ATTRIBUTE_CFM message, containing the requested attribute
    will be sent in response.
*/
void ConnectionSmGetAttributeReq(
        uint16 ps_base,
        uint8 addr_type,
        const bdaddr* bd_addr,
        uint16 size_psdata
        );

/*!
    @brief Specialises ConnectionSmGetAttributeReq() to provide backward 
    compatibilty for BR/EDR only connections.
*/
#define ConnectionSmGetAttribute(ps_base, bd_addr, size_psdata, psdata) \
            ConnectionSmGetAttributeReq( \
                    (ps_base), \
                    TYPED_BDADDR_PUBLIC, \
                    (bd_addr), \
                    (size_psdata))

/*! 
    @brief This function is called to read attribute data from the persistent
    store, assuming an entry for the device already exists in the
    paired device list, returning that data immediately.

    This function replaces ConnectionSmGetAttributeNow(), which did not support
    typed_bdaddr addresses. A macro for the ConnectionSmGetAttributeNow() has 
    been added for backwards compatibility that defaults to the 
    TYPED_BDADDR_PUBLIC address type.

    @param ps_base User persistent data is now stored in the connection
    library's private PS Keys and so this parameter no longer has any effect. 
    It is kept for backwards compatibility and will be deprecated in the future.
    
    @param addr_type The address type - Public or Random.
    @param bd_addr The Bluetooth address of the device
    @param size_psdata The length of the data to be written
    @param psdata Pointer to buffer in which data will be returned.

    On return, the requested attribute data will be in the psdata pointer, which
    was passed as a parameter. 
*/
bool ConnectionSmGetAttributeNowReq(
        uint16 ps_base,
        uint8 addr_type,
        const bdaddr* bd_addr,
        uint16 size_psdata,
        uint8 *psdata
        );

/*!
    @brief Specialises ConnectionSmGetAttributeNowReq() to provide backward 
    compatibilty for BR/EDR only connections.
*/
#define ConnectionSmGetAttributeNow(ps_base, bd_addr, size_psdata, psdata) \
            ConnectionSmGetAttributeNowReq( \
                    (ps_base), \
                    TYPED_BDADDR_PUBLIC, \
                    (bd_addr), \
                    (size_psdata), \
                    (psdata))

/*! 
    @brief This function is called to retrieve Bluetooth address and attribute
    data from the persistent store for a device stored in the trusted device 
    table, assuming an entry for the device already exists.

    @param ps_base User persistent data is now stored in the connection
    library's private PS Keys and so this parameter no longer has any effect. 
    It is kept for backwards compatibility and will be deprecated in the future.
    
    @param index Index into trusted device table (0 is most recently paired
    device),
    @param size_psdata The length of the data to be written,

    A CL_SM_GET_INDEXED_ATTRIBUTE_CFM message, containing the requested 
    attribute will be sent in response.
*/
void ConnectionSmGetIndexedAttribute(
        uint16 ps_base,
        uint16 index,
        uint16 size_psdata
        );

/*! 
    @brief This function is called to retrieve Bluetooth address and attribute
    data from the persistent store for a device stored in the trusted device 
    table, assuming an entry for the device already exists.

    @param ps_base User persistent data is now stored in the connection
    library's private PS Keys and so this parameter no longer has any effect. 
    It is kept for backwards compatibility and will be deprecated in the future.
    
    @param index Index into trusted device table (0 is most recently paired 
    device),
    @param size_psdata The length of the data to be written,
    @param psdata Pointer to buffer in which data will be returned.
    @param taddr The Bluetooth address of the device.

    On return, the requested attribute data will be in the psdata pointer, which
    was passed as a parameter. 
*/
bool ConnectionSmGetIndexedAttributeNowReq(
        uint16          ps_base,
        uint16          index,
        uint16          size_psdata,
        uint8           *psdata,
        typed_bdaddr    *taddr
        );

/*! 
    @brief This function is called to mark a device as trusted or untrusted.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param trusted TRUE(trusted) or FALSE(not trusted).

    If a device attempts to make a connection and it is not trusted then an 
    CL_DM_AUTHORISE_IND message will be sent to request whether this device
    is allowed to connect. This procedure applies even though the two devices
    have exchanged link keys.

    A  CL_SM_SET_TRUST_LEVEL_CFM_T message will be returned to indicate the 
    status of the request, e.g. If the device is not in the paired device list 
    then the status will be Failure.
*/
void ConnectionSmSetTrustLevel(
        Task theAppTask,
        const bdaddr* bd_addr,
        uint16 trusted
        );


/*!
    @brief This function is called update the trusted device index with the most
    recently used device

    @param bd_addr The Bluetooth address of the most recently used device
*/
void ConnectionSmUpdateMruDevice(const bdaddr *bd_addr);


/*! 
    @brief Register the supplied service record with the local SDP server.
    Will cause a CL_SDP_REGISTER_CFM message to be sent back to the specified
    task.

    @param theAppTask The client task.
    @param size_service_record The number of bytes in the service record.
    @param service_record The service record to register.
*/
void ConnectionRegisterServiceRecord(
        Task theAppTask,
        uint16 size_service_record,
        const uint8 *service_record
        );


/*! 
    @brief Unregister a record that has previously been registered with the SDP
    server

    @param theAppTask The client task.
    @param service_record_hdl The service record handle.
*/
void ConnectionUnregisterServiceRecord(
        Task theAppTask,
        uint32 service_record_hdl
        );


/*! 
    @brief Configure the SDP server to use a particular size of L2CAP MTU.

    @param mtu The size of L2CAP MTU to use.
*/
void ConnectionSetSdpServerMtu(uint16 mtu);


/*! 
    @brief Configure the SDP client to use a particular size of L2CAP MTU.

    @param mtu The size of L2CAP MTU to use.
*/
void ConnectionSetSdpClientMtu(uint16 mtu);


/*! 
    @brief Open a search session to the specified remote device.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
*/
void ConnectionSdpOpenSearchRequest(Task theAppTask, const bdaddr* bd_addr);


/*! 
    @brief Close the existing SDP search session.

    @param theAppTask The client task.
*/
void ConnectionSdpCloseSearchRequest(Task theAppTask);


/*! 
    @brief Perform a service search on the specified remote device. Will issue
    a CL_SDP_SERVICE_SEARCH_CFM message.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param max_num_recs The maximum number of records.
    @param size_search_pattern The size of search_pattern.
    @param search_pattern The pattern to search for.
*/
void ConnectionSdpServiceSearchRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 max_num_recs,
        uint16 size_search_pattern,
        const uint8 *search_pattern
        );

/*! 
    @brief Perform a service search on the specified remote device, and pass
    the result as a reference. Will issue a CL_SDP_SERVICE_SEARCH_REF_CFM
    message.

    This is useful if the application needs to store the result, or if the
    result is too large to fit in a message.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param max_num_recs The maximum number of records.
    @param size_search_pattern The size of search_pattern.
    @param search_pattern The pattern to search for.
*/
void ConnectionSdpServiceSearchRefRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 max_num_recs,
        uint16 size_search_pattern,
        const uint8 *search_pattern
        );

/*! 
    @brief Perform an attribute search on the specified remote device

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param max_num_recs The maximum number of records.
    @param service_hdl The service handle.
    @param size_attribute_list The size of the attribute_list.
    @param attribute_list The attributes list.
*/
void ConnectionSdpAttributeSearchRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 max_num_recs,
        uint32 service_hdl,
        uint16 size_attribute_list,
        const uint8 *attribute_list
        );

/*! 
    @brief Perform an attribute search on the specified remote device, and pass
    the result as a reference. Will issue a CL_SDP_ATTRIBUTE_SEARCH_REF_CFM
    message.

    This is useful if the application needs to store the result, or if the
    result is too large to fit in a message.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param max_num_recs The maximum number of records.
    @param service_hdl The service handle.
    @param size_attribute_list The size of the attribute_list.
    @param attribute_list The attributes list.
*/
void ConnectionSdpAttributeSearchRefRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 max_num_recs,
        uint32 service_hdl,
        uint16 size_attribute_list,
        const uint8 *attribute_list
        );

/*! 
    @brief Perform a service and an attribute search on the specified remote
    device.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param max_attributes The maximum number of attributes.
    @param size_search_pattern The size of the search_pattern.
    @param search_pattern The pattern to search for.
    @param size_attribute_list The size of attribute_list.
    @param attribute_list The attribute list.

*/
void ConnectionSdpServiceSearchAttributeRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 max_attributes,
        uint16 size_search_pattern,
        const uint8 *search_pattern,
        uint16 size_attribute_list,
        const uint8 *attribute_list
        );

/*! 
    @brief Perform a service and an attribute search on the specified remote
    device, and pass the result as a reference. Will issue a
    CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM message.

    This is useful if the application needs to store the result, or if the
    result is too large to fit in a message.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param max_attributes The maximum number of attributes.
    @param size_search_pattern The size of the search_pattern.
    @param search_pattern The pattern to search for.
    @param size_attribute_list The size of attribute_list.
    @param attribute_list The attribute list.

*/
void ConnectionSdpServiceSearchAttributeRefRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 max_attributes,
        uint16 size_search_pattern,
        const uint8 *search_pattern,
        uint16 size_attribute_list,
        const uint8 *attribute_list
        );

/*! 
    @brief Terminate the current SDP primitive search.

    @param theAppTask The client task.
*/
void ConnectionSdpTerminatePrimitiveRequest(Task theAppTask);


/*! 
    @brief Register an L2CAP PSM with the Connection library.

    @param theAppTask The client task. The CL_L2CAP_REGISTER_CFM message is 
    returned to this task. This is also the task associated with the PSM
    being registered. All incoming connection indications for this PSM will
    be forwarded to this client task.
    
    @param psm The PSM being registered with the Connection library.
    
    @param flags L2CAP PSM Registration Flags.

    Before a connection can be accepted or initiated on a particular PSM it 
    must firstly be registered with the Connection library. If a connection 
    indication is received for a PSM that does not have a task associated 
    with it this connection will be rejected by the Connection library.

    The client task will receive a CL_L2CAP_REGISTER_CFM message from the 
    Connection library indicating the outcome of this request.
*/
void ConnectionL2capRegisterRequest(Task theAppTask, uint16 psm, uint16 flags);

/*! 
    @brief Unregister an L2CAP PSM from the Connection library.

    @param theAppTask The client task. The CL_L2CAP_UNREGISTER_CFM message is 
    retuned to this task. Only the task that registered the PSM with the 
    Connection library can unregister it.

    @param psm The PSM being unregistered from the Connection library.

    Once a PSM has been unregistered connections can no longer be accepted 
    or initiated on that PSM. All incoming connection requests will be 
    automatically rejected by the Connection library.

    The client task will receive a CL_L2CAP_UNREGISTER_CFM message from the 
    Connection library indicating the outcome of this request.
*/
void ConnectionL2capUnregisterRequest(Task theAppTask, uint16 psm);


/*! 
    @brief Initiate the creation of an L2CAP connection to a particular device.
    To use default L2CAP configuration parameters set the conftab_length and 
    conftab_pointer to 0.

    @param theAppTask The client task initiating the L2CAP connection. The
    response indicating the outcome of the connect request will be returned to
    this task.
    
    @param bd_addr The address of the remote device being connected to. 
    
    @param psm_local The local PSM being connected. This PSM must have been
    registered with the Connection library before a connection can be
    initiated.

    @param psm_remote The remote PSM being connected to.

    @param conftab_length The length of the configuration table data array. i.e. 
    The number of uint16 entries in the array.

    @param conftab Pointer to a configuration table of uint16 values. These are
    key value pairs defining configuration options to be passed to Bluestack. 
    This pointer will be passed to Bluestack at which time the VM memory slot
    for it will be freed.
    
    The client task will receive a CL_L2CAP_CONNECT_CFM message from the 
    Connection library indicating the outcome of this request.
*/

void ConnectionL2capConnectRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 psm_local,
        uint16 psm_remote,
        uint16 conftab_length,
        const uint16* conftab
        );


/*! 
    @brief Initiate the creation of an L2CAP LE COC connection to a particular
    device.  To use default L2CAP configuration parameters set the 
    conftab_length and conftab_pointer to 0.

    @param theAppTask The client task initiating the L2CAP connection. The
    response indicating the outcome of the connect request will be returned to
    this task.
    
    @param taddr The address of the remote device being connected to. 
    
    @param psm_local The local PSM being connected. This PSM must have been
    registered with the Connection library before a connection can be
    initiated.

    @param psm_remote The remote PSM being connected to.

    @param conftab_length The length of the configuration table data array.

    @param conftab Pointer to a configuration table of uint16 values. These are
    key value pairs defining configuration options to be passed to Bluestack. 
    This pointer will be passed to Bluestack at which time the VM memory slot
    for it will be freed.
    
    The client task will receive a CL_L2CAP_TP_CONNECT_CFM message from the 
    Connection library indicating the outcome of this request.
*/

void ConnectionL2capTpConnectRequest(
        Task theAppTask,
        const tp_bdaddr *taddr,
        uint16 psm_local,
        uint16 psm_remote,
        uint16 conftab_length,
        uint16* conftab
        );

/*! 
    @brief Request L2CAP to extend additional credits to the peer.

    @param theAppTask The client task initiating the L2CAP connection. The
    response indicating the outcome of the connect request will be returned to
    this task.
    
    @param context Opaque context number returned in response. 

    @param connection_id The conneciton ID of the L2CAP link.

    @param credits Extended these credits to remote device. 
    
    The client task will receive a CL_L2CAP_ADD_CREDIT_CFM message from the 
    Connection library indicating the outcome of this request.
*/

void ConnectionL2capAddCreditRequest(
        Task theAppTask,
        uint16 connection_id,
        uint16 credits
        );

/*! 
    @brief Response to a notification of an incoming L2CAP connection. 
    To use default L2CAP configuration parameters set the conftab_length and 
    conftab_pointer to 0.

    @param theAppTask The client task to which subsequent L2CAP CL messages 
    will be sent once the connection is established.
    
    @param response Set to TRUE to accept the incoming connection or FALSE 
    to reject it. If rejecting the connection the PSM and connection_id fields 
    still have to be set but the configuration parameter can be set to null.
    
    @param psm The PSM on the local device that the remote end is trying to 
    connect to. The psm field from the CL_L2CAP_CONNECT_IND message can be
    copied directly.
    
    @param connection_id The connection identifier. This is the connection_id
    field from the CL_L2CAP_CONNECT_IND message that is being responded to.
    This field should be copied from the CL_L2CAP_CONNECT_IND message directly.
    It is necessary so that if two connections to the same PSM are being
    established the Connection library knows which particular connection is
    being responded to.
    
    @param identifier The signal identifier. This is the identifier field from 
    the CL_L2CAP_CONNECT_IND message that must be responded to. This field 
    should be copied from the CL_L2CAP_CONNECT_IND message directly.

    @param conftab_length The length of the configuration table data array. i.e. 
    The number of uint16 entries in the array.

    @param conftab Pointer to a configuration table of uint16 values. These are
    key value pairs defining configuration options to be passed to Bluestack. 
    This pointer will be passed to Bluestack at which time the VM memory slot
    for it will be freed.

    On an indication of an incoming L2CAP connection the Connection library
    sends the client task that registered the PSM being connected to a 
    CL_L2CAP_CONNECT_IND message. In response to this message the client task 
    must call this function to indicate whether it wishes to proceed with this
    connection or not.

    The client task will receive a CL_L2CAP_CONNECT_CFM message from the 
    Connection library indicating the outcome of the connection attempt.
*/
void ConnectionL2capConnectResponse(
        Task theAppTask,
        bool response,
        uint16 psm,
        uint16 connection_id,
        uint8 identifier,
        uint16 conftab_length,
        const uint16* conftab
        );


/*! 
    @brief Response to a notification of an incoming L2CAP LE COC connection. 
    To use default L2CAP configuration parameters set the conftab_length and 
    conftab_pointer to 0.

    @param theAppTask The client task to which subsequent L2CAP CL messages 
    will be sent once the connection is established.
    
    @param response Set to TRUE to accept the incoming connection or FALSE 
    to reject it. If rejecting the connection the PSM and connection_id fields 
    still have to be set but the configuration parameter can be set to null.
    
    @param psm The PSM on the local device that the remote end is trying to 
    connect to. The psm field from the CL_L2CAP_TP_CONNECT_IND message can be
    copied directly.
    
    @param connection_id The connection identifier. This is the connection_id
    field from the CL_L2CAP_TP_CONNECT_IND message that is being responded to.
    This field should be copied from the CL_L2CAP_TP_CONNECT_IND message directly.
    It is necessary so that if two connections to the same PSM are being
    established the Connection library knows which particular connection is
    being responded to.
    
    @param identifier The signal identifier. This is the identifier field from 
    the CL_L2CAP_TP_CONNECT_IND message that must be responded to. This field 
    should be copied from the CL_L2CAP_TP_CONNECT_IND message directly.

    @param conftab_length The length of the configuration table data array.

    @param conftab Pointer to a configuration table of uint16 values. These are
    key value pairs defining configuration options to be passed to Bluestack. 
    This pointer will be passed to Bluestack at which time the VM memory slot
    for it will be freed.

    On an indication of an incoming L2CAP connection the Connection library
    sends the client task that registered the PSM being connected to a 
    CL_L2CAP_TP_CONNECT_IND message. In response to this message the client task 
    must call this function to indicate whether it wishes to proceed with this
    connection or not.

    The client task will receive a CL_L2CAP_TP_CONNECT_CFM message from the 
    Connection library indicating the outcome of the connection attempt.
*/
void ConnectionL2capTpConnectResponse(
        Task theAppTask,
        bool response,
        uint16 psm,
        uint16 connection_id,
        uint8 identifier,
        uint16 conftab_length,
        uint16* conftab
        );

/*! 
    @brief Request to disconnect an L2CAP connection.

    @param theAppTask The client task. This should be the task that owns the 
    L2CAP connection. Only a task that owns a particular connection can issue 
    a disconnect request for it. 

    @param sink The sink identifying the L2CAP connection. 

    The client task will receive a CL_L2CAP_DISCONNECT_CFM message from the 
    Connection library indicating the outcome of this request.  
*/
void ConnectionL2capDisconnectRequest(Task theAppTask, Sink sink);


/*! 
    @brief Response to a CL_L2CAP_DISCONNECT_IND message.

    @param identifier L2CAP identifier for the link, passed up in the 
    CL_L2CAP_DISCONNECT_IND message.

    @param sink The sink identifying the L2CAP connection. 

    Once this message has been sent, the sink is no longer valid.
*/
void ConnectionL2capDisconnectResponse(uint8 identifier, Sink sink);

/*!
    @brief Request to map L2CAP connectionless data to a remote device.

    @param theAppTask The client task. Only this task can unmap the L2CAP 
    connectionless data from he remote device.

    @param bd_addr The Bluetooth address of the remote device that the L2CAP
    connectionless data is to be mapped to.

    @param psm_local The local PSM being connected. This PSM must have been
    registered with the Connection library before a connection can be
    initiated.

    @param psm_remote The remote PSM being connected to.

    @param type The type of connectionless data to be mapped.

    The client task will receive a CL_L2CAP_MAP_CONNECTIONLESS_CFM message from
    the Connection library indicating the outcome of this request and containing
    a Sink for the connectionless data to be sent on.
*/
void ConnectionL2capMapConnectionlessRequest(
        Task theAppTask,
        const bdaddr *bd_addr,
        uint16 psm_local,
        uint16 psm_remote,
        l2cap_connectionless_data_type type
        );

/*!
    @brief Respond to the message CL_L2CAP_MAP_CONNECTIONLESS_IND.

    @param theAppTask The client task that will receive further messages (such
    as CL_L2CAP_UNMAP_CONNECTIONLESS_IND)for the Source associated with 
    this connectionless data.

    @param source The Source associated with the connectionless data.

    @param type The type of connectionless data as specified in the IND message.
*/
void ConnectionL2capMapConnectionlessResponse(
        Task theAppTask,
        Source source,
        l2cap_connectionless_data_type type
        );

/*!
    @brief Unmap connectionless data from a remote device.

    @param sink The Sink identify the stream for the connectionless data to a
    remote device.

    This will result in a CL_L2CAP_UNMAP_CONNECTIONLESS_IND message when the 
    associated ACL has been close, at both this and the remote device.

    This is only required for Unicast Connectionless Data (UCD), for which the 
    ACL has been locked and cannot be closed until it is unmapped by this 
    function.

*/
void ConnectionL2capUnmapConnectionlessRequest(Sink sink);

/*! 
    @brief This function is called to allocate an RFCOMM server channel. The 
    application can suggest the server channel number it would like (range 1
    to 30). Bluestack will use the suggested server channel number if it can,
    otherwise it will return a dynamically allocated server channel number 
    instead. The response to this will be a response, a CL_RFCOMM_REGISTER_CFM, 
    sent to the specified App Task.

    @param theAppTask The client task where the CFM message will be sent.
    @param suggested_server_channel This can be the channel the application
    would like to suggest. If the application does not care then this should
    be set to 0 and Bluestack will allocate a server channel dynamically.
*/
void ConnectionRfcommAllocateChannel(
        Task theAppTask,
        uint8 suggested_server_channel
        );

/*! 
    @brief This function is called to deallocate an RFCOMM server channel. In
    response, a CL_RFCOMM_UNREGISTER_CFM message will be sent to the specified
    task.

    @param theAppTask The client task where the CFM message will be sent.
    @param local_server_channel The server channel to be deallocated.
*/

void ConnectionRfcommDeallocateChannel(
        Task theAppTask,
        uint8 local_server_channel
        );

/*! 
    @brief This function is called to request an RFCOMM connection to the
    device with the specified Bluetooth device Address and server channel.

    @param theAppTask The client task.
    @param bd_addr The Bluetooth address of the remote device.
    @param security_channel The local server channel or security channel.
    @param remote_server_chan The remote server channel.
    @param config RFCOMM configuration parameters, set to 0 the following 
    default values will be used for the structure:
    - max_payload_size = RFCOMM_DEFAULT_PAYLOAD_SIZE (884)
    - modem_signal     = RFCOMM_DEFAULT_MODEM_SIGNAL (0x8C)
    - break_signal     = RFCOMM_DEFAULT_BREAK_SIGNAL (0)
    - msc_timeout      = RFCOMM_DEFAULT_MSC_TIMEOUT  (500) Milliseconds.
*/
void ConnectionRfcommConnectRequest(
        Task theAppTask,
        const bdaddr* bd_addr,
        uint16 security_channel,
        uint8 remote_server_chan,
        const rfcomm_config_params *config
        );

/*! 
    @brief This function is called to respond to a CL_RFCOMM_CONNECT_IND.
    Default values for the Modem Status parameters (not included) are assumed.

    @param theAppTask The client task.
    @param response Accept connection(TRUE) or refuse (FALSE).
    @param sink As indicated in the CL_RFCOMM_CONNECT_IND message.
    @param local_server_channel The server channel.
    @param config RFCOMM configuration parameters, set to 0 the following 
    default values will be used for the structure:
    - max_payload_size = RFCOMM_DEFAULT_PAYLOAD_SIZE (884)
    - modem_signal     = RFCOMM_DEFAULT_MODEM_SIGNAL (0x8C)
    - break_signal     = RFCOMM_DEFAULT_BREAK_SIGNAL (0)
    - msc_timeout      = RFCOMM_DEFAULT_MSC_TIMEOUT  (500) Milliseconds.
*/
void ConnectionRfcommConnectResponse(
        Task theAppTask,
        bool response,
        const Sink sink,
        uint8 local_server_channel,
        const rfcomm_config_params *config
        );

/*!
    @brief This function is called to request a disconnection of the RFCOMM
    channel.  

    @param theAppTask The client task.  
    @param sink The RFCOMM channel to disconnect.

    This function should only be called if the RFCOMM connection is active.
*/
void ConnectionRfcommDisconnectRequest(Task theAppTask, Sink sink);

/*!
    @brief This function is called to respond to  a disconnection of the RFCOMM
    channel, indicated by a CL_RFCOMM_DISCONNECT_IND message.
 
    @param sink The RFCOMM channel to disconnect.

    This function should only be called if the RFCOMM connection is active.
*/
void ConnectionRfcommDisconnectResponse(Sink sink);


/*!
    @brief This function is called to request Port parameter negotiation with
    the remote device. If the 'request' parameter is set to TRUE, then instead
    of negotiating the port parameters, this will request the remote device's 
    own port parameters, which will be received in a CL_RFCOMM_PORTNEG_IND 
    message. 

    @param theAppTask The client task.
    @param sink The sink.
    @param request If TRUE, then request the remote devices port settings.
    @param port_params The port parameters to be negotiated.
    
*/
void ConnectionRfcommPortNegRequest(
        Task theAppTask,
        Sink sink,
        bool request,
        port_par *port_params
        );


/*!
    @brief This function is called to respond to a CL_RFCOMM_PORTNEG_IND. The
    port_params.parameter_mask field bits are set to '1' to indicate which new 
    parameters have been accepted, '0' to indicate which, if any are rejected.
    Setting the port_params field to 0 will automatically initialise a 
    port_params structure, including setting all the bits of the parameter_mask
    field to 1.

    @param theAppTask The client task.
    @param sink The sink.
    @param port_params The port parameters to be negotiated.
    
*/
void ConnectionRfcommPortNegResponse(
        Task theAppTask,
        Sink sink,
        const port_par* port_params
        );


/*!
    @brief This function is called to send control signals to the remote
    device.

    @param theAppTask The client task.
    @param sink The sink.
    @param break_signal 0-3-seconds in 200ms increments e.g. range 0-15 where 
    1 = 200ms and 15 = 3-seconds
    @param modem_signal The modem signals.

    This function should only be called if the RFCOMM connection is active.
*/
void ConnectionRfcommControlSignalRequest(
        Task theAppTask,
        Sink sink,
        uint8 break_signal,
        uint8 modem_signal
        );

/*!
    @brief This function is called to send the RFCOMM Line Status to the remote
    device. It should only be called when there is a line status error to 
    report.

    @param theAppTask The client task.
    @param sink The sink.
    @param error Error flag, TRUE if an error is to be indicated. 
    @param line_status The line status error.

    This function should only be called if the RFCOMM connection is active.
*/
void ConnectionRfcommLineStatusRequest(
        Task theAppTask,
        Sink sink,
        bool error,
        rfcomm_line_status_error line_status
        );


/*!
    @brief This function is called to place the device in "Device Under Test"
    (DUT) mode.

    This function places the local device into DUT mode. It makes the device
    discoverable/ connectible, disables security and then issues the
    DM_HCI_ENABLE_DEVICE_UT_MODE message to Bluestack. A CL_DM_DUT_CFM message
    is returned to the task that initialised the connection library to indicate
    whether DUT mode was entered successfully.
*/
void ConnectionEnterDutMode(void);

/*!
    @brief This function is called to initiate a BLE transmitter test.

    A CL_DM_BLE_TRANSMITTER_TEST_CFM message is returned to the task that
    initialised the connection library to indicate the outcome of the operation.

    @param tx_channel Transmit channel: 0 to 39
    @param data_length Packet length: 0 to 37 (no DLE) or 0 to 255 (with DLE)
    @param test_pattern Test pattern to use. One of

    HCI_ULP_TEST_PATTERN_PRBS9
    HCI_ULP_TEST_PATTERN_11110000
    HCI_ULP_TEST_PATTERN_10101010
    HCI_ULP_TEST_PATTERN_PRBS15
    HCI_ULP_TEST_PATTERN_ALL_1
    HCI_ULP_TEST_PATTERN_ALL_0
    HCI_ULP_TEST_PATTERN_00001111
    HCI_ULP_TEST_PATTERN_01010101
*/
void ConnectionBleTransmitterTest(uint8 tx_channel, uint8 data_length, uint8 test_pattern);

/*!
    @brief This function is called to initiate a BLE receiver test.

    A CL_DM_BLE_RECEIVER_TEST_CFM message is returned to the task that
    initialised the connection library to indicate the outcome of the operation.
*/
void ConnectionBleReceiverTest(uint8 rx_channel);

/*!
    @brief This function is called to terminate a BLE transmitter or receiver test.

    A CL_DM_BLE_TEST_END_CFM message is returned to the task that
    initialised the connection library to indicate the outcome of the operation.
*/
void ConnectionBleTestEnd(void);

/*!
    @brief This function is called to set the link supervision timeout.

    @param sink The sink.
    @param timeout The timeout in 0.625ms units.
*/
void ConnectionSetLinkSupervisionTimeout(Sink sink, uint16 timeout);


/*!
    @brief This function sets a link policy power table for a connection sink.

    The link policy power table allows the device to switch between different
    power modes based on the current activity of the associated ACL connection.

    Example: State, min_interval, max_interval, attempt, timeout, time
    @code{.c}
    const lp_power_table default_powertable[3]=
    {
        {lp_active,  0,     0,      0,      0,      5},
        {lp_passive, 0,     0,      0,      0,      20},
        {lp_sniff,   800,   800,    1,      8,      0}
    };
    @endcode 

    @param sink The sink.
    @param size_power_table The number of entries in the power table.
    @param power_table The power table.
*/
void ConnectionSetLinkPolicy(
        Sink sink,
        uint16 size_power_table,
        lp_power_table const *power_table
        );

/*!
    @brief This function updates link policy settings for a connection.

    For the link policy settings bitmask.

    @param bd_addr The bd address of the device to update settings.
    @param lps link policy settings bitmask
*/
void ConnectionWriteLinkPolicySettings(
        const bdaddr* bd_addr,
        const lp_settings lps
        );


/*!
    @brief This function is called to set up preferred sub-rating parameters to 
    be used when the device enters sniff mode.

    @param sink The sink.
    @param max_remote_latency The maximum time the remote device need not be 
    present when subrating (in 0.625ms units).  The minimum value is 2 (1.25ms),
    although values lower than the current sniff interval will be rounded up. A
    value equal to the current sniff interval is equivalent to sniff mode
    without subrating on the remote device.
    @param min_remote_timeout The minimum time the remote device should stay in 
    sniff before entering subrating mode (in 0.625ms units).
    @param min_local_timeout The minimum time the local device should stay in 
    sniff before entering subrating mode (in 0.625ms units).

    This is a BT2.1 only feature
*/
void ConnectionSetSniffSubRatePolicy(
        Sink sink,
        uint16 max_remote_latency,
        uint16 min_remote_timeout,
        uint16 min_local_timeout
        );

/*!
    @brief Configure the page timeout to be used when attempting a connection 
    to a remote device.

    @param page_timeout New page timeout to use in connection attempts. This 
    value is specified in Bluetooth slots. Set to zero to reset the page timeout
    back to a default value.

    This function will set the default page timeout used for all subsequent 
    connection attempts.  The page timeout is not reset back to a default value
    by the connection library. To reset the page timeout back to a default value
    this function will need to be called again.

    This function does not return a message to the client task.

    Changing the default page timeout can seriously impair the ability of a
    device to create an ACL. Use of this function is not recommended unless
    absolutely necessary.
*/
void ConnectionSetPageTimeout(uint16 page_timeout);

/*!
    @brief This function is called to set the priority flag of a device stored
    in the trusted device list.
    
    @param bd_addr Bluetooth address of the device in question.
    @param is_priority_device Priority device flag for the device.

    @return TRUE if record updated, otherwise FALSE
*/
bool ConnectionAuthSetPriorityDevice(
        const bdaddr* bd_addr,
        bool is_priority_device
        );

/****************************************************************************
    @brief This function is called to get the priority flag of a device
    stored in the trusted device list. Return value indicates whether
    the device is in the TDL in the first place 
    (to mimic ConnectionAuthGetPriorityDevice).

    @param bd_addr Bluetooth address of the device in question.
    @param is_priority_device Priority device flag for the device.

    @return TRUE if device exists, otherwise FALSE
*/
bool ConnectionAuthGetPriorityDeviceStatus(
        const bdaddr* bd_addr, 
        bool *is_priority_device
        );

/****************************************************************************
    @brief This function is called to test the priority flag status of a device
    stored in the trusted device list. It assumes the device is already
    in the TDL.

    @param bd_addr Bluetooth address of the device in question.

    @return TRUE if device is protected, otherwise FALSE (also if device does not exist).
*/
bool ConnectionAuthIsPriorityDevice(
        const bdaddr* bd_addr
        );
        
/*!
    @brief Indicate how many devices are stored in the Trusted Device List.
    
    This device will read the index of the trusted device list and count how 
    many entries there are.

    @return Number of devices in the Trusted Device Index.
*/
uint16 ConnectionTrustedDeviceListSize(void);

/*!
    @brief Reserve an LTADDR for use with a connectionless slave broadcast. 
    
    @param lt_addr LTADDR to reserve.
*/
void ConnectionCsbSetReservedLtAddr(uint16 lt_addr);

/*!
    @brief Delete a reserved LTADDR.
    
    @param lt_addr LTADDR to delete.
*/
void ConnectionCsbDeleteReservedLtAddr(uint16 lt_addr);

/*!
    @brief Control broadcast of a connectionless slave broadcast (CSB). 
    
    @param enable TRUE to start a CSB, FALSE to stop a CSB.
    @param lt_addr A reserved LTADDR over which to run the CSB.
    @param lpo_allowed TRUE BR/EDR controller is permitted to sleep, FALSE 
                       it is not. 
    @param packet_type Bitmask of permitted packets types to use for the CSB.
    @param interval_min Minimum interval between CSB packet in slots.
    @param interval_max Maximum interval between CSB packets in slots.
    @param supervision_timeout Duration in slots after which the BR/EDR
                               controller reports a CSB timeout event if
                               it is unable to transmit a CSB packet.
*/
void ConnectionCsbSet(bool enable, uint16 lt_addr, bool lpo_allowed,
                      uint16 packet_type, uint16 interval_min, uint16 interval_max,
                      uint16 supervision_timeout);
/*!
    @brief Configure the parameters used for transmission of a CSB synchronisation train.
    
    @param interval_min Minimum interval between sync train packets in slots. 
    @param interval_max Maximum interval between sync train packets in slots.
    @param sync_train_timeout Duration in slots to continue sending the sync train.
    @param service_data Single octet of application specific data to include in the
                        sync train.
*/
void ConnectionCsbWriteSyncTrainParameters(uint16 interval_min, uint16 interval_max,
                                           uint32 sync_train_timeout, uint8 service_data);
/*!
    @brief Enable the synchronisation train to start transmitting. 

    The sync train will continue for #sync_train_timeout and then stop.
*/
void ConnectionCsbStartSyncTrain(void);

/*!
    @brief Attempt to discover the synchronisation train.
    
    @param bd_addr Bluetooth address of the device transmitting the sync train. 
    @param sync_scan_timeout Duration in slots to search for the sync train. 
    @param sync_scan_window Duration in slots to listen for a sync train packet
                            on a single frequency.
    @param sync_scan_interval Duration in slots between the start of consecutive
                              scan windows.
*/
void ConnectionCsbReceiveSyncTrain(const bdaddr *bd_addr, uint16 sync_scan_timeout, uint16 sync_scan_window, uint16 sync_scan_interval);

/*!
    @brief Control reception of a connectionless slave broadcast (CSB). 
    
    @param enable TRUE to start receiving a CSB, FALSE to stop receiving a CSB. 
    @param bd_addr Bluetooth address of the device transmitting a CSB. 
    @param lt_addr LTADDR of the CSB. 
    @param interval Interval between CSB packet instances, in slots. 
    @param clock_offset Clock offset.
    @param next_csb_clock CLK for next CSB instant. 
    @param supervision_timeout CSB receive timeout in slots.
    @param remote_timing_accuracy Timing accuracy of the CSB broadcaster in ppm.
    @param skip Number of consecutive CSB instants the receiver may skip after
                a CSB packet is successfully received.
    @param packet_type Bitmask of permitted packet types.
    @param afh_map AFH map received from the CSB broadcaster.
*/
void ConnectionCsbReceive(bool enable, const bdaddr *bd_addr, uint8 lt_addr, uint16 interval,
                          uint32 clock_offset, uint32 next_csb_clock, uint16 supervision_timeout, uint8 remote_timing_accuracy,
                          uint16 skip, uint16 packet_type, const AfhMap afh_map);

/*!
    @brief Response to a #CL_CSB_TRANSMIT_TIMEOUT_IND message.

    Must be sent in response to the timeout indication.
    Once called, further CSB transmissions are permitted.
    
    @param lt_addr LTADDR of the CSB.
*/
void ConnectionCsbTransmitTimeoutResponse(uint16 lt_addr);

/*!
    @brief Response to a #CL_CSB_RECEIVE_TIMEOUT_IND message.

    Must be sent in response to the timeout indication.
    Once called, further CSB receive operations are permitted.
    
    @param bd_addr Bluetooth address of the CSB broadcaster. 
    @param lt_addr LTADDR of the CSB. 
*/
void ConnectionCsbReceiveTimeoutResponse(const bdaddr *bd_addr, uint8 lt_addr);


/*!
    @brief Set a task to receive CSB related messages.
    
    After this call the connection library will send CSB related messages to
    the specified task. If not used, the connection library will send CSB
    messages to the application task registered in ConnectionInit().

    @param csbTask Task to which CSB messages will be sent.

    @return Task Current csbTask if exists or NULL if none currently registered.
*/
Task ConnectionCsbRegisterTask(Task csbTask);

/*!
    @brief Delete the task registered to receive CSB messages. 
    
    The connection library will no longer send CSB messages to the task that
    was registered. CSB messages will be sent to the application task.

    @param csbTask Task to unregister.
*/
/*void ConnectionCsbUnregisterTask(Task csbTask);*/

/******************************************************************************
 *                      CL Crypto pubilc defines                              *
 ******************************************************************************/

 /* Care must be taken to keep these definitions in line with what the firmware
    expects and is able to handle. */
#define CL_CRYPTO_LOCAL_PVT_KEY_LEN         16
#define CL_CRYPTO_REMOTE_PUB_KEY_LEN        32
#define CL_CRYPTO_AES_DATA_LEN              8
#define CL_CRYPTO_AES_KEY_LEN               8

#define CL_CRYPTO_PUBLIC_KEY_LEN            32 
#define CL_CRYPTO_PRIVATE_KEY_LEN           16 
#define CL_CRYPTO_SECRET_KEY_LEN            16 

#define CL_CRYPTO_SHA_HASH_LEN              16 

/*  This specific define is the most vulnerable to change from the firmware; if
    the firmware's maximum block size changes, this will need to be changed
    accordingly. */
#define CL_CRYPTO_SHA_DATA_LEN              16

typedef enum {
    cl_crypto_ecc_p192          =   1,
    cl_crypto_ecc_p256          =   2,
    cl_crypto_ecc_unknown_type  /* Always keep this last in the enum. */
}   cl_ecc_type;


typedef enum {
    cl_crypto_single_block      =   0,
    cl_crypto_data_start        =   1,
    cl_crypto_data_conti        =   2,
    cl_crypto_data_end          =   3,
    cl_crypto_unknown_operation /* Always keep this last in the enum. */
}   cl_hash_operation;


typedef enum {
    cl_crypto_success           =   0,
    cl_crypto_int_success       =   1,
    cl_crypto_busy              =   2,
    cl_crypto_invalid_param     =   3,
    cl_crypto_generic_fail      =   4,
    cl_crypto_error             =   5,  /* When status is set to this, the cl_crypto_error_status field will contain details. */
    cl_crypto_unknown_status    /* Always keep this last in the enum. */
}   cl_crypto_status;

typedef enum {
    cl_no_error                     =   0,
    cl_ecc_invalid_key_type         =   1,
    cl_ecdh_invalid_key_type        =   2,
    cl_ecdh_empty_public_key        =   3,
    cl_ecdh_empty_private_key       =   4,
    cl_aes_empty_data_array         =   5,
    cl_aes_empty_key_array          =   6,
    cl_sha_empty_data_array         =   7,
    cl_sha_cont_empty_data_array    =   8
}   cl_crypto_error_status;

typedef struct
{
    cl_ecc_type             key_type;                                   /* DM_CRYPTO_ECC_P192/DM_CRYPTO_ECC_P256 */
    uint8                   status;                                     /* Generation success or failed */
    cl_crypto_error_status  error;                                      /* Only valid if the status is cl_crypto_error. */
    uint16                  public_key[CL_CRYPTO_PUBLIC_KEY_LEN];       /* Public key, zero if status is failure */
    uint16                  private_key[CL_CRYPTO_PRIVATE_KEY_LEN];     /* Private key, zero if status is failure */
} CL_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM_T;

typedef struct
{
    cl_ecc_type             key_type;                                       /* DM_CRYPTO_ECC_P192/DM_CRYPTO_ECC_P256 */
    uint8                   status;                                         /* Generation success or failed */
    cl_crypto_error_status  error;                                          /* Only valid if the status is cl_crypto_error. */
    uint16                  shared_secret_key[CL_CRYPTO_SECRET_KEY_LEN];    /* Shared secret key(dhkey), zero if status is not success */
} CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM_T;

typedef struct
{
    uint8                   flags;                                      /* reserved */
    uint8                   status;                                     /* Generation success or failed */
    cl_crypto_error_status  error;                                      /* Only valid if the status is cl_crypto_error. */
    uint16                  encrypted_data[CL_CRYPTO_AES_DATA_LEN];     /* Encrypted data */
} CL_CRYPTO_ENCRYPT_CFM_T;

typedef struct
{
    cl_hash_operation       operation;                          /* Operation type from request */
    uint8                   flags;                              /* Reserved */
    uint8                   status;                             /* Generation success or failed */
    cl_crypto_error_status  error;                              /* Only valid if the status is cl_crypto_error. */
    uint16                  hash[CL_CRYPTO_SHA_HASH_LEN];       /* Hash of the supplied data */
} CL_CRYPTO_HASH_CFM_T;

typedef struct
{
    uint8                   flags;                                      /* reserved */
    uint8                   status;                                     /* success or failed */
    cl_crypto_error_status  error;                                      /* error status */
    uint16                  decrypted_data[CL_CRYPTO_AES_DATA_LEN];    /* decrypted data */
} CL_CRYPTO_DECRYPT_CFM_T;

/******************************************************************************
 *                      CRYPTO FUNCTION PROTOTYPES                                                            *
 ******************************************************************************/

/*            Important note concerning multiple encryption calls
 *  If the firmware is already handling any crypto operation, and another task
 *  requests another crypto operation, the firmware will respond with a
 *  DM_CRYPTO_BUSY message. The user will have to handle this response and send
 *  their request at a later time.
 *
 *  It is important to note that whenever data is passed to any of the crypto
 *  functions as an array, the endianness of the word groups must be flipped.
 *  Essentially, the least significant word must be the array's zero-th element,
 *  and the most significant word must be the array's last element.
 *  
 *  Consequently, the data returned by these functions also has its endianness
 *  flipped in the same way. However, in addition to endianness, the sequence of
 *  octets within each word is also flipped, so if the returned value's most
 *  significant word is 88d4, the returned array's zero-th element with be d488.
 */

 /*!
    @brief  Send a message to Bluestack requesting the generation of a public/
            private encryption key pair using ECC.
    
    @param theAppTask   The task that called this crypto function.
    
    @param key_type     The type of encryption required (192 or 256 bit). The 
                        only valid input types are listed above in the ecc_type 
                        enum (cl_crypto_ecc_p192 or cl_crypto_ecc_p256).

    @return None.   A confirmation message containing the generated key pair is 
                    returned from the Bluestack and is handled by the message 
                    handler in bluestack_handler.c.
*/
 void ConnectionGeneratePublicPrivateKey(Task theAppTask, cl_ecc_type key_type);

 /*!
    @brief  Send a message to Bluestack requesting the generation of a shared 
            secret encryption key from a public/private key pair using ECHD. 
            Additional notes on array parameters and endianness are included in 
            the source file.

    @param theAppTask       The task that called this crypto function.
    @param key_type         The type of encryption required (192 or 256 bit). 
                            The only valid input types are listed above in the 
                            ecc_type enum (cl_crypto_ecc_p192 or cl_crypto_ecc_p256).
    @param private_key      An array holding the private key to be used to 
                            generate the shared secret. The array must be 16 
                            words large (32 bytes).
    @param public_key       An array holding the public key to be used to 
                            generate the shared secret. The array must be 32 
                            words large (64 bytes).

    @return None.   A confirmation message containing the shared secret key is 
                    returned from the Bluestack and is handled by the message 
                    handler in bluestack_handler.c.
*/
 void ConnectionGenerateSharedSecretKey(Task theAppTask, cl_ecc_type key_type, uint16 private_key[CL_CRYPTO_LOCAL_PVT_KEY_LEN], uint16 public_key[CL_CRYPTO_REMOTE_PUB_KEY_LEN]);

/*!
    @brief  Send a message to Bluestack requesting the encryption of a 128-bit 
            block of data using a 128-bit encryption key using AES. Additional 
            notes on array parameters and endianness are included in the source file.

    @param theAppTask       The task that called this crypto function.
    @param data_array       An array holding the data to be encrypted. 
                            The array must be 8 words large (16 bytes).
    @param key_array        An array holding the key to be used to encrypt the 
                            submitted data. The array must be 8 words large (16 bytes).

    @return None.   A confirmation message containing the encrypted data is 
                    returned from the Bluestack and is handled by the message 
                    handler in bluestack_handler.c.
*/

 void ConnectionEncryptBlockAes(Task theAppTask, uint16 data_array[CL_CRYPTO_AES_DATA_LEN], uint16 key_array[CL_CRYPTO_AES_KEY_LEN]);

/*!                  Usage for SHA256 Block Encryption                         
 *  The user is expected to keep track of the current index progress within each
 *  array they intend to pass as data to be hashed. This can be done by
 *  incrementing said index every time a crypto_int_success message is received
 *  for the particular task, or any other way the user deems appropriate.
 *
 *  The functions also expect the user to pass the length of the data array as
 *  a parameter, since there is no way for the function to calculate their size
 *  after they've been collapsed to a pointer.
 *  
 *  Both the index and length must be in octets, despite the arrays themselves
 *  holding words. If the array contains an odd number of octets, the user can
 *  include the last octet by itself, since the API will pad it with zeroes.
 */

/*!
    @brief  Send a message to Bluestack requesting the encryption of a data 
            block of arbitrary length using SHA256.

    @param theAppTask       The task that called this crypto function.
    @param data_array       An array holding the data to be encrypted. The 
                            array can be of any length.
    @param array_size       The size of the array passed as the previous 
                            argument in octets.

    @return None.   A confirmation message containing the encrypted data (if the
                    data block was not greater than 32 bytes) or signalling 
                    interim success is returned from the Bluestack and is 
                    handled by the message handler in bluestack_handler.c.
*/
 void ConnectionEncryptBlockSha256(Task theAppTask, uint16 *data_array, uint16 array_size);
 
/*!
    @brief  Send a message to Bluestack requesting the continuation of the 
            SHA256 encryption operation for the remaining data.

    @param theAppTask       The task that called this crypto function.
    @param data_array       An array holding the data to be encrypted. The 
                            array can be of any length. This must be the same 
                            array used in the first call to SHA256 encryption.
    @param array_size       The size of the array passed as the previous 
                            argument in octets.
    @param current_index    The index position of the next hash block's first 
                            character in the passed array in octets. In 
                            practice, this number can only be multiples of 32.

    @return None.   A confirmation message containing the encrypted data (if 
                    the remaining data block was not greater than 32 bytes) or 
                    signalling interim success is returned from the Bluestack 
                    and is handled by the message handler in bluestack_handler.c.
*/
 void ConnectionEncryptBlockSha256Continue(Task theAppTask, uint16 *data_array, uint16 array_size, uint16 current_index);

/*!
    @brief  Send a message to Bluestack requesting the decryption of a 128-bit 
            block of data using a 128-bit decryption key using AES. 
            
    @param theAppTask       The task that called this crypto function.
    @param data_array       An array holding the data to be decrypted. 
                            The array must be 8 words large (16 bytes).
    @param key_array        An array holding the key to be used to decrypt the 
                            submitted data. The array must be 8 words large (16 bytes).

    @return None.   
*/
void ConnectionDecryptBlockAes(Task theAppTask, uint16 data_array[CL_CRYPTO_AES_DATA_LEN], uint16 key_array[CL_CRYPTO_AES_DATA_LEN]);

 /*****************************************************************************
  *                    CL GATT Caching PS function prototypes                 *
  *****************************************************************************/
 /*!
    @brief  This function is called to write GATT-related attribute data to the
            persistent store assuming an entry for the device already exists
            in the paired device list.

    @param  taddr The typed Bluetooth address struct of the device
    @param  bd_addr The Bluetooth address of the device
    @param  size_psdata The length of the data to be written
    @param  psdata Pointer to the data to be written

    On return, the data will have been written into the persistent store
    VM Connection Library data key GATT Attribute base  + index of the device
    in the trusted device list.

 */
 void ConnectionPutGattAttribute(
         typed_bdaddr* taddr,
         uint16 size_psdata,
         const uint8* psdata
         );

 /*!
    @brief  This function is called to read GATT-related attribute data from
            the persistentstore, assuming there is already an entry for the device
            exists in the paired device list, returning that data immediately.

    @param  taddr The typed Bluetooth address struct of the device
    @param  size_psdata The length of the data to be written
    @param  psdata Pointer to buffer in which data will be returned
    
    @return A boolean to indicate if data was successfully retrieved.

    On return, the requested attribute data will be in the psdata pointer, which
    was passed as a parameter.
 */
 bool ConnectionGetGattAttribute(
         typed_bdaddr* taddr,
         uint16 size_psdata,
         uint8 *psdata
         );

/*!
    @brief  This function is called to retrieve the stored GATT database 
            hash from the persistent store.
    
    @param  size_psdata The length of the data to be written
    @param  psdata Pointer to buffer in which data will be returned
    
    @return A boolean to indicate if data was successfully retrieved.
    
    On return, the requested attribute data will be in the psdata pointer, which
    was passed as a parameter.
*/
bool ConnectionGetDatabaseHash(uint16 size_psdata, uint8 *psdata);

/*!
    @brief  This function is called to store the generated GATT database 
            hash in the persistent store.
    
    @param  size_psdata The length of the data to be written
    @param  psdata Pointer to the data to be written

    On return, the data will have been written into the persistent store
    VM Connection Library data key DB Hash
*/
void ConnectionPutDatabaseHash(uint16 size_psdata, uint8 *psdata);

/*!
    @brief  This function is called at boot to check if any trusted clients exist in
            the TDL, update their related data to change-unaware if a DB change has
            occured, and return the details of any trusted clients that have enabled
            robust caching to the GATT library so that Bluestack can be notified.
            
    @param  hashes_matched  A boolean indicating if the generated and stored DB hashes matched.
    @param  device_list     A list to be populated with the addresses of clients which support
                            robust caching
    @param  status_list     A list to be populated with the change aware status of the clients
                            which support robust caching. The indexes must match.
    
    @return A count of how many trusted devices support robust caching.
    
    On return, any updates to trusted clients' change aware status will have been stored
    in PS, and the details of trusted clients which support robust caching would be placed
    in the list passed as an argument.
*/
uint8 ConnectionGattTdlBootSequence(bool hashes_matched, tp_bdaddr * device_list, bool * status_list);

#endif    /* CONNECTION_NO_BLE_H_ */
