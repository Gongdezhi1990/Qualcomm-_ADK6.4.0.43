#ifndef _BLUESTACK_L2CAP_PRIM_H_
#define _BLUESTACK_L2CAP_PRIM_H_

/*!

Copyright (c) 2001 - 2019 Qualcomm Technologies International, Ltd.
  

\file   l2cap_prim.h
\{
*/
/*! @{*/

#include "bluestack/hci.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \name Configuration option defines
  \{ */
#define L2CA_OPTION_MTU                 0x01 /*!< Maximum Transmission Unit */
#define L2CA_OPTION_FLUSH               0x02 /*!< Flush timeout */
#define L2CA_OPTION_QOS                 0x03 /*!< Quality of Service */
#define L2CA_OPTION_FLOW                0x04 /*!< Flow and error control*/
#define L2CA_OPTION_FCS                 0x05 /*!< Frame Check Sequence*/
#define L2CA_OPTION_FLOWSPEC            0x06 /*!< Extended flow specification */
#define L2CA_OPTION_EXT_WINDOW          0x07 /*!< Extended Window Size */
#define L2CA_OPTION_PRIORITY            0x0E /*!< Reserved for internal use */
#define L2CA_OPTION_PRIORITY2           0x0F /*!< Reserved for internal use */
#define L2CA_OPTION_PRIORITY3           0x10 /*!< Reserved for internal use */
#define L2CA_OPTION_HINT                0x80 /*!< Hint flag */
/*! \} */

/* How to bit-shift to get the priority. */
#define L2CA_OPTION_PRIORITY_SHIFT      (L2CA_OPTION_PRIORITY - 1)

/*! \name Configuration "options present" bitmask
  \{ */
#define L2CA_OPTION_MASK(option)        (1u<<((option) - 1))
#define L2CA_SPECIFY_MTU                L2CA_OPTION_MASK(L2CA_OPTION_MTU)       /* 0x0001 */
#define L2CA_SPECIFY_FLUSH              L2CA_OPTION_MASK(L2CA_OPTION_FLUSH)     /* 0x0002 */
#define L2CA_SPECIFY_QOS                L2CA_OPTION_MASK(L2CA_OPTION_QOS)       /* 0x0004 */
#define L2CA_SPECIFY_FLOW               L2CA_OPTION_MASK(L2CA_OPTION_FLOW)      /* 0x0008 */
#define L2CA_SPECIFY_FCS                L2CA_OPTION_MASK(L2CA_OPTION_FCS)       /* 0x0010 */
#define L2CA_SPECIFY_FLOWSPEC           L2CA_OPTION_MASK(L2CA_OPTION_FLOWSPEC)  /* 0x0020 */
#define L2CA_SPECIFY_EXT_WINDOW         L2CA_OPTION_MASK(L2CA_OPTION_EXT_WINDOW)/* 0x0040 */
#define L2CA_SPECIFY_PRIORITY_INTERNAL  (L2CA_OPTION_MASK(L2CA_OPTION_PRIORITY) | \
                                        L2CA_OPTION_MASK(L2CA_OPTION_PRIORITY2) | \
                                        L2CA_OPTION_MASK(L2CA_OPTION_PRIORITY3)) /* Reserved for internal use */
/*! \} */


/*! \name MTU defaults and parameters
  \{ */
#define L2CA_MTU_DEFAULT                672
#define L2CA_MTU_MINIMUM                48
#define L2CA_LE_MTU_MINIMUM             23
/*! \} */


/*! \name Flush timeout defaults

    \brief The DEFAULT value is 'infinite' ie. no flush timeout is
    used.  For links that use the extended flowspec, the basic flush
    timeout paramter is not used (IN_FLOWSPEC). When using the BR/EDR
    radio, the maximum flush timeout is limited by the HCI slot value;
    MAX_HCI_LIMIT_MS.

    \{ */
#define L2CA_FLUSH_TO_IN_FLOWSPEC        ((l2ca_timeout_t)0x0000)
#define L2CA_FLUSH_TO_IMMEDIATE          ((l2ca_timeout_t)0x0001)
#define L2CA_FLUSH_TO_INFINITE           ((l2ca_timeout_t)0xFFFF)
#define L2CA_FLUSH_TO_DEFAULT            L2CA_FLUSH_TO_INFINITE
#define L2CA_FLUSH_TO_MAX_HCI_LIMIT_MS   ((uint16_t)((((uint32_t)HCI_MAX_FLUSH_TIMEOUT)*625)/1000))
/*! \} */


/*! \name QoS and FlowSpec service type modes
  \{ */
#define L2CA_QOS_TYPE_NO_TRAFFIC         0x00
#define L2CA_QOS_TYPE_BEST_EFFORT        0x01
#define L2CA_QOS_TYPE_GUARANTEED         0x02
/*! \} */


/*! \name QoS and FLowSpec type mask
  \{ */
#define L2CA_QOS_TYPE_MASK(type)         (1<<(type))
#define L2CA_QOS_TYPE_MASK_NO_TRAFFIC    L2CA_QOS_TYPE_MASK(L2CA_QOS_TYPE_NO_TRAFFIC)
#define L2CA_QOS_TYPE_MASK_BEST_EFFORT   L2CA_QOS_TYPE_MASK(L2CA_QOS_TYPE_BEST_EFFORT)
#define L2CA_QOS_TYPE_MASK_GUARANTEED    L2CA_QOS_TYPE_MASK(L2CA_QOS_TYPE_GUARANTEED)
/*! \} */


/*! \name QOS defaults
  \{ */
#define L2CA_QOS_DEFAULT_SERVICE_TYPE    L2CA_QOS_TYPE_BEST_EFFORT
#define L2CA_QOS_DEFAULT_TOKEN_RATE      0x00000000
#define L2CA_QOS_DEFAULT_TOKEN_BUCKET    0x00000000
#define L2CA_QOS_DEFAULT_PEAK_BW         0x00000000
#define L2CA_QOS_DEFAULT_LATENCY         0xFFFFFFFFU
#define L2CA_QOS_DEFAULT_DELAY_VAR       0xFFFFFFFFU
/*! \} */


/*! \name Extended flow specification defaults and special values
  \{ */
#define L2CA_FLOWSPEC_MAX_SDU            0xFFFF
#define L2CA_FLOWSPEC_INTERAR_DEFAULT    0xFFFFFFFFU
#define L2CA_FLOWSPEC_ACCESS_DEFAULT     0xFFFFFFFFU
#define L2CA_FLOWSPEC_FLUSH_TO_INFINITE  ((l2ca_fs_flush_t)0xFFFFFFFFU)
#define L2CA_FLOWSPEC_FLUSH_TO_IMMEDIATE ((l2ca_fs_flush_t)0x0)
/*! \} */


/*! \name Flow & error control modes
  \{ */
#define L2CA_FLOW_MODE_BASIC             0x00 /*!< Basic mode as in L2CAP 1.1 */
#define L2CA_FLOW_MODE_RTM_OBSOLETE      0x01 /*!< Legacy retransmission mode (not supported) */
#define L2CA_FLOW_MODE_FC_OBSOLETE       0x02 /*!< Legacy control control mode (not supported) */
#define L2CA_FLOW_MODE_ENHANCED_RETRANS  0x03 /*!< Enhanced retransmission mode, CSA1 */
#define L2CA_FLOW_MODE_STREAMING         0x04 /*!< Streaming mode, CSA1 */
/*! \} */


/*! \name L2CAP support modes bitmask for the mode_mask in L2CA_REGISTER_REQ.
    These set of mode masks are valid only on BR/EDR transport. Mode mask shall
    be set to zero for LE transport.
  \{ */
#define L2CA_MODE_MASK(mode)             ((((uint16_t)1)<<(mode)))
#define L2CA_MODE_MASK_BASIC             L2CA_MODE_MASK(L2CA_FLOW_MODE_BASIC)
#define L2CA_MODE_MASK_RTM_OBSOLETE      L2CA_MODE_MASK(L2CA_FLOW_MODE_RTM_OBSOLETE)
#define L2CA_MODE_MASK_FC_OBSOLETE       L2CA_MODE_MASK(L2CA_FLOW_MODE_FC_OBSOLETE)
#define L2CA_MODE_MASK_ENHANCED_RETRANS  L2CA_MODE_MASK(L2CA_FLOW_MODE_ENHANCED_RETRANS)
#define L2CA_MODE_MASK_STREAMING         L2CA_MODE_MASK(L2CA_FLOW_MODE_STREAMING)
/*! \} */

/*! \name Optional Frame Check Sequence (CRC) option values
  \{ */
#define L2CA_FCS_OFF                     0x0000 /*!< Don't use FCS */
#define L2CA_FCS_ON                      0x0001 /*!< Use FCS (default) */
/*! \} */


/*! \name L2CA_GETINFO_REQ info types to query for
  \{ */
#define L2CA_GETINFO_TYPE_UNKNOWN        0x0000 /*!< Unknown */
#define L2CA_GETINFO_TYPE_MTU            0x0001 /*!< Connetionless MTU */
#define L2CA_GETINFO_TYPE_EXT_FEAT       0x0002 /*!< Extended feature mask */
#define L2CA_GETINFO_TYPE_FIX_CH         0x0003 /*!< Fixed channel */
/*! \} */


/*! \name L2CA_GETINFO_CFM expected size of info_data length
  \{ */
#define L2CA_GETINFO_SIZE_MTU            0x0002 /*!< Connetionless MTU */
#define L2CA_GETINFO_SIZE_EXT_FEAT       0x0004 /*!< Extended feature mask */
#define L2CA_GETINFO_SIZE_FIX_CH         0x0008 /*!< Fixed channel */
/*! \} */


/*! \name L2CA_GETINFO_CFM extended features bitfields (EXT_FEAT response)
  \{ */
#define L2CA_EXT_FEAT_FLOW_CONTROL       0x0001 /*!< Legacy flow control mode (not supported) */
#define L2CA_EXT_FEAT_RETRANSMISSION     0x0002 /*!< Legacy retransmission mode (not supported) */
#define L2CA_EXT_FEAT_BIDIR_QOS          0x0004 /*!< Bi-directional QoS */
#define L2CA_EXT_FEAT_ENH_RETRANS        0x0008 /*!< Enhanced: Enhanced retransmission mode */
#define L2CA_EXT_FEAT_STREAMING          0x0010 /*!< Enhanced: Streaming mode */
#define L2CA_EXT_FEAT_OPT_FCS            0x0020 /*!< Enhanced: FCS option */
#define L2CA_EXT_FEAT_EXT_FLOW_SPEC      0x0040 /*!< BR/EDR Extended flow specification support */
#define L2CA_EXT_FEAT_FIXED_CH           0x0080 /*!< AMP: Fixed channels */
#define L2CA_EXT_FEAT_EXT_WIN_SIZE       0x0100 /*!< AMP: Extended window sizes */
#define L2CA_EXT_FEAT_UCD_RECEPTION      0x0200 /*!< Receives UCD data */
#define L2CA_EXT_FEAT_MAX                0x0200 /*!< Largest defined bit */
/*! \} */


/*! \name L2CAP fixed CID channels
  \{ */
#define L2CA_CID_INVALID                 0x0000
#define L2CA_CID_SIGNAL                  0x0001
#define L2CA_CID_CONNECTIONLESS          0x0002
#define L2CA_CID_AMP_MANAGER             0x0003
#define L2CA_CID_ATTRIBUTE_PROTOCOL      0x0004
#define L2CA_CID_LE_SIGNAL               0x0005
#define L2CA_CID_SECURITY_MANAGER        0x0006
#define L2CA_CID_BREDR_SECURITY_MANAGER  0x0007

#define L2CA_CID_DYNAMIC_FIRST           0x0040
#define L2CA_CID_DYNAMIC_LAST            0xFFFF

#define L2CA_LE_CID_DYNAMIC_FIRST        0x0040
#define L2CA_LE_CID_DYNAMIC_LAST         0x007F
/*! \} */


/*! \name L2CA_GETINFO_CFM fixed channel support
  \{ */
#define L2CA_FIXED_MASK(cid)             (1<<(cid))
#define L2CA_FIXED_CH_NULL               (0x0000)
#define L2CA_FIXED_CH_SIGNAL             L2CA_FIXED_MASK(L2CA_CID_SIGNAL)
#define L2CA_FIXED_CH_CONNECTIONLESS     L2CA_FIXED_MASK(L2CA_CID_CONNECTIONLESS)
#define L2CA_FIXED_CH_AMP_MANAGER        L2CA_FIXED_MASK(L2CA_CID_AMP_MANAGER)
#define L2CA_FIXED_CH_BREDR_SECURITY_MANAGER  L2CA_FIXED_MASK(L2CA_CID_BREDR_SECURITY_MANAGER)
/*! \} */


/*! \name L2CAP fixed PSM values and defined ranges
  \{ */
#define L2CA_PSM_INVALID                 0x0000
#define L2CA_PSM_MASK_REQUIRED           0x0001
#define L2CA_PSM_MASK_INVALID            0x0100
#define L2CA_PSM_VALIDATE(a) (((a) & ~L2CA_PSM_MASK_INVALID) | L2CA_PSM_MASK_REQUIRED)
#define L2CA_PSM_FIXED_MIN               L2CA_PSM_VALIDATE(0x0000)
#define L2CA_PSM_FIXED_MAX               L2CA_PSM_VALIDATE(0x0FFF)
#define L2CA_PSM_DYNAMIC_MIN             L2CA_PSM_VALIDATE(0x1000)
#define L2CA_PSM_DYNAMIC_MAX             L2CA_PSM_VALIDATE(0xFFFF)
/*! \} */

/*! \name LE L2CAP fixed PSM values and defined ranges
  \{ */
#define L2CA_LE_PSM_FIXED_MIN               L2CA_PSM_VALIDATE(0x0000)
#define L2CA_LE_PSM_FIXED_MAX               L2CA_PSM_VALIDATE(0x007F)
#define L2CA_LE_PSM_DYNAMIC_MIN             L2CA_PSM_VALIDATE(0x0080)
#define L2CA_LE_PSM_DYNAMIC_MAX             L2CA_PSM_VALIDATE(0x00FF)
/*! \} */


/*! \name L2CAP special "disable reconfiguration" options
  \{ */
#define L2CA_RECONF_ALLOW_ALL            0x0000
#define L2CA_RECONF_DISABLE_ALL          0xFFFF
/*! \} */


/*! \name L2CAP AMP controller IDs
  \{ */
#define L2CA_AMP_CONTROLLER_BT           0x0000 /*!< Bluetooth BR/EDR */
#define L2CA_AMP_CONTROLLER_UNKNOWN      0xFFFF /*!< Not in spec - undefined controller */
/*! \} */

#define L2CA_CONFLAG_ENUM_OFFSET            2
#define L2CA_CONFLAG_ENUM(value)            ((l2ca_conflags_t)(value) << L2CA_CONFLAG_ENUM_OFFSET )
#define L2CA_CONFLAG_ENUM_MASK              0x1C


/*! \name Special channel establishment (connection) options
  \{ */
typedef enum
{
    L2CA_CONNECTION_BR_EDR,                            /*!< Channel to be used for UCD */     
    L2CA_CONNECTION_BR_EDR_BROADCAST,                  /*!< Channel to be used for broadcast connectionless */
    L2CA_CONNECTION_LE_MASTER_DIRECTED,                /*!< Connect as Master */
    L2CA_CONNECTION_LE_MASTER_WHITELIST,               /*!< Connect as Master using whitelist */
    L2CA_CONNECTION_LE_SLAVE_DIRECTED_HIGH_DUTY,       /*!< Start high duty cycle directed adverts */
    L2CA_CONNECTION_LE_SLAVE_WHITELIST,                /*!< Use whitelist for undirected adverts */
    L2CA_CONNECTION_LE_SLAVE_UNDIRECTED,               /*!< Start undirected adverts */
    L2CA_CONNECTION_LE_SLAVE_DIRECTED_LOW_DUTY,        /*!< Start low duty cycle directed adverts */

    /* Used for compile time checking, do not remove it or add anything after it */
    L2CA_CONFLAG_ENUM_MAX

}L2CA_CONNECTION_T;
/*! \} */

/* TODO: This should be removed when Connection/GATT libs APIs are updated*/
#define  L2CA_CONNECTION_LE_SLAVE_DIRECTED L2CA_CONNECTION_LE_SLAVE_DIRECTED_HIGH_DUTY

/*! \name   Special channel establishment (connection) flags

    \brief  The flags allow a  fixed channel to be mapped over a ACL-U or BLE-U logical link.
            The flags field is a part bitmask ( bit 1 and 0) and a part enumeration(\ref L2CA_CONNECTION_T) 

            Valid combinations of logical OR are any values from the bit mask
            ORed with a value from the specific enumeration field.

    \see    L2CA_CONNECTION_T            

  \{ */
#define L2CA_CONFLAG_LOCK_ACL               ((l2ca_conflags_t)0x0001)           /*!< For fixed channels, lock the ACL */
#define L2CA_CONFLAG_PAGE_TIMEOUT           ((l2ca_conflags_t)0x0002)           /*!< Page timeout for adverts */
#define L2CA_CONFLAG_RESERVED_3             ((l2ca_conflags_t)0x0004)           /*!< Reserverd Bit Position 3 */
#define L2CA_CONFLAG_RESERVED_4             ((l2ca_conflags_t)0x0008)           /*!< Reserverd Bit Position 4 */ 
#define L2CA_CONFLAG_RESERVED_5             ((l2ca_conflags_t)0x0010)           /*!< Reserverd Bit Position 5 */ 
#define L2CA_CONFLAG_INCOMING               ((l2ca_conflags_t)0x8000)           /*!< Reserverd */

#define L2CA_CONFLAG_NONE                   ((l2ca_conflags_t)0x0000)           /*!< Deprecated definitions retained for backwards compatibility */
#define L2CA_CONFLAG_BROADCAST              ((l2ca_conflags_t)0x0004)           /*!< Deprecated definitions retained for backwards compatibility */

/*! \} */

#define L2CA_CONFLAG_IS_LE(flags)        ( ((flags) & L2CA_CONFLAG_ENUM_MASK) != L2CA_CONFLAG_ENUM(L2CA_CONNECTION_BR_EDR) &&\
                                           ((flags) & L2CA_CONFLAG_ENUM_MASK) != L2CA_CONFLAG_ENUM(L2CA_CONNECTION_BR_EDR_BROADCAST) )


/*! \name Registration flags

    \brief 0x8000 is reserved and shall always be zero.
  \{ */
#define L2CA_REGFLAG_ALLOW_RX_UCD       ((uint16_t)0x0001) /*!< Allow Unicast Connectionless Data Reception */
#define L2CA_REGFLAG_ALLOW_RX_BCD       ((uint16_t)0x0002) /*!< Allow Broadcast Connectionless Data Reception */
#define L2CA_REGFLAG_USE_TP_PRIMS       ((uint16_t)0x0004) /*!< Use transport specific L2CAP primitives */
#define L2CA_REGFLAG_FOR_LE_TP          ((uint16_t)0x0008) /*!< Registration being done on LE transport. 
                                                                This flag shall be set along with L2CA_REGFLAG_USE_TP_PRIMS */
/*! \} */

/*! \name Connection Parameter Update command limits.
  \{ */ 
#define L2CA_CON_INTERVAL_MIN           ((uint16_t)6)
#define L2CA_CON_INTERVAL_MAX           ((uint16_t)3200)
#define L2CA_CON_SLAVE_LATENCY_MAX      ((uint16_t)499)
#define L2CA_CON_TIMEOUT_MIN            ((uint16_t)10)
#define L2CA_CON_TIMEOUT_MAX            ((uint16_t)3200)
/*! \} */

/*! \name Result values for connection parameter update from peripheral.
  \{ */ 
#define L2CAP_CONNECTION_PARAMETER_UPDATE_ACCEPT 0
#define L2CAP_CONNECTION_PARAMETER_UPDATE_REJECT 1
/*! \} */

/*! \name L2CAP Raw Debug Modes.
  \{ */ 
#define L2CA_RAW_MODE_OFF               ((l2ca_raw_t)0x0000)
#define L2CA_RAW_MODE_BYPASS_CHANNEL    ((l2ca_raw_t)0x0001)
#define L2CA_RAW_MODE_BYPASS_CONNECTION ((l2ca_raw_t)0x0002)
#define L2CA_RAW_MODE_SNIFF_HCI_DATA    ((l2ca_raw_t)0x0003)
#define L2CA_RAW_MODE_CRC_REJECTS       ((l2ca_raw_t)0x0004)
/*! \} */

/*! \name L2CAP Raw HCI Packet type.
  \{ */
#define L2CA_RX                         ((l2ca_raw_packet_t)(0x0000))
#define L2CA_TX_START                   ((l2ca_raw_packet_t)(0x0001))
#define L2CA_TX_CONT                    ((l2ca_raw_packet_t)(0x0002))
/*! \} */

/*! \name Auto connect configuration options

  \brief For information about how to encode a conftab table, please
  see the header file bkeyval.h.

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

  Note 4 - FCS: Use the L2CA_FCS_* definitions. Note that the FCS can
  only be disabled if both sides explicitly requests it to be
  disabled, so the off state can never be guaranteed.

  \{ */
#define L2CA_AUTOPT_MTU_IN               ((uint16_t)0x0001) /* 16 bit, exact - incoming MTU */
#define L2CA_AUTOPT_MTU_OUT              ((uint16_t)0x0102) /* 16 bit, minimum - peer MTU */
#define L2CA_AUTOPT_FLUSH_IN             ((uint16_t)0x0703) /* 32 bit, range - peer flush (us)  - note that HCI limit still applies */
#define L2CA_AUTOPT_FLUSH_OUT            ((uint16_t)0x0704) /* 32 bit, range - local flush (us) - note that HCI limit still applies */
#define L2CA_AUTOPT_QOS_SERVICE          ((uint16_t)0x0005) /* 16 bit, exact - shared service type (note1) */
#define L2CA_AUTOPT_QOS_RATE_IN          ((uint16_t)0x0706) /* 32 bit, range - incoming token rate/flowspec interarrival */
#define L2CA_AUTOPT_QOS_RATE_OUT         ((uint16_t)0x0707) /* 32 bit, range - outgoing token rate/flowspec interarrival */
#define L2CA_AUTOPT_QOS_BUCKET_IN        ((uint16_t)0x0708) /* 32 bit, range - incoming token bucket */
#define L2CA_AUTOPT_QOS_BUCKET_OUT       ((uint16_t)0x0709) /* 32 bit, range - outgoing token bucket */
#define L2CA_AUTOPT_QOS_PEAK_IN          ((uint16_t)0x070a) /* 32 bit, range - incoming peak bandwidth */
#define L2CA_AUTOPT_QOS_PEAK_OUT         ((uint16_t)0x070b) /* 32 bit, range - outgoing peak bandwidth */
#define L2CA_AUTOPT_QOS_LATENCY_IN       ((uint16_t)0x070c) /* 32 bit, range - incoming qos/flowspec access latency */
#define L2CA_AUTOPT_QOS_LATENCY_OUT      ((uint16_t)0x070d) /* 32 bit, range - outgoing qos/flowspec access latency */
#define L2CA_AUTOPT_QOS_DELAY_IN         ((uint16_t)0x070e) /* 32 bit, range - incoming delay variation */
#define L2CA_AUTOPT_QOS_DELAY_OUT        ((uint16_t)0x070f) /* 32 bit, range - outgoing delay variation */
#define L2CA_AUTOPT_FS_SDU_SIZE_IN       ((uint16_t)0x0310) /* 16 bit, range - incoming max SDU size */
#define L2CA_AUTOPT_FS_SDU_SIZE_OUT      ((uint16_t)0x0311) /* 16 bit, range - incoming max SDU size */
#define L2CA_AUTOPT_FLOW_MODE            ((uint16_t)0x0012) /* 16 bit, exact - shared flow control mode (note3) */
#define L2CA_AUTOPT_FLOW_WINDOW_IN       ((uint16_t)0x0313) /* 16 bit, range - incoming window size */
#define L2CA_AUTOPT_FLOW_WINDOW_OUT      ((uint16_t)0x0314) /* 16 bit, range - peer window size */
#define L2CA_AUTOPT_FLOW_MAX_RETX_IN     ((uint16_t)0x0315) /* 16 bit, range - peer maximum retransmit */
#define L2CA_AUTOPT_FLOW_MAX_RETX_OUT    ((uint16_t)0x0316) /* 16 bit, range - this option is ignored - it's set by the peer */
#define L2CA_AUTOPT_FLOW_MAX_PDU_IN      ((uint16_t)0x0317) /* 16 bit, range - incoming max PDU payload size */
#define L2CA_AUTOPT_FLOW_MAX_PDU_OUT     ((uint16_t)0x0318) /* 16 bit, range - outgoing maximum PDU size */
#define L2CA_AUTOPT_FCS                  ((uint16_t)0x0019) /* 16 bit, exact - use FCS or not (note4) */
#define L2CA_AUTOPT_FS_SERVICE           ((uint16_t)0x001A) /* 16 bit, exact - shared flowspec service type (note2) */

#define L2CA_AUTOPT_EXT_FEATS            ((uint16_t)0x0420) /* 32 bit, exact - cached getinfo ext.feats */
#define L2CA_AUTOPT_DISABLE_RECONF       ((uint16_t)0x0021) /* 16 bit, exact - ward off reconfiguration attempts */
#define L2CA_AUTOPT_SECURITY_PROTOCOL    ((uint16_t)0x0022) /* 16 bit, exact - reserved for internal use - replacement security protocol ID */
#define L2CA_AUTOPT_SECURITY_CHANNEL     ((uint16_t)0x0023) /* 16 bit, exact - reserved for internal use - replacement security channel */
#define L2CA_AUTOPT_CHANNEL_PRIORITY     ((uint16_t)0x0024) /* 16 bit, exact - set channel priority from 0 (highest) to L2CAP_MAX_TX_QUEUES - 1 (lowest) */
#define L2CA_AUTOPT_CREDITS              ((uint16_t)0x0025) /* 16-bit, exact - intial credits given to the remote device while creating connection */ 
#define L2CA_AUTOPT_CONN_FLAGS           ((uint16_t)0x0026) /* 16-bit, exact - l2ca_conflags_t parameter indicating the connection options to bring up the link.*/
/*! \} */

/*! \name L2CAP internal result codes (not defined in spec)

    \brief Extended result codes that are not defined in the
    specification (except for L2CA_RESULT_SUCCESS) and are detected by
    the local L2CAP entity. When used in signals, they will be part of
    the specific listings and typecasted to the signal result type.

    Important: It is generally illegal to use ANY of these result
    codes in the 'response' member downstream primitives. Only the
    values defined in the spec may be used in responses.

    \{ */

#define L2CA_RES_TABLE \
    L2CA_RES_TABLE_X(L2CA_RESULT_SUCCESS,                   = 0x0000)               L2CA_RES_TABLE_SEP /*!< The success code (defined in spec) */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_NOT_READY,                 = L2CA_ERRORCODE_BASE)  L2CA_RES_TABLE_SEP /*!< Local device not ready */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_FAILED,                    L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Operation failed (peer didn't answer) */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_KEY_MISSING,               L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Link key missing */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_TIMEOUT,                   L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Peer has timed out */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_ILLEGAL_PSM,               L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Illegal PSM (most be odd numbered) */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_OUT_OF_MEM,                L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Not enough memory */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_CONFIG_MISMATCH,           L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< L2CAP modes mismatch */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_CONFIG_ILLEGAL,            L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Illegal configuration parameter */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_RTM_OVERFLOW,              L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Retransmission overflow */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_HARDWARE,                  L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Hardware error */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_UNSUPPORTED_MODE,          L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Unsupported F&EC/QoS mode */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PEER_REJECT_CID,           L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Peer has send reject command 'invalid CID' */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_LINK_LOSS,                 L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Link loss caused disconnection */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_SAR_VIOLATION,             L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< The segmentation protocol was violated */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_FEC_HEADER,                L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Invalid F&EC header */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_MTU_VIOLATION,             L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< MTU was violated */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_MPS_VIOLATION,             L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< MPS was violated */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_FEC_SEQUENCE,              L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< F&EC sequence number error */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_READ_PARTIAL,              L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Data only partially available (non-fatal) */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PKT_MISSING,               L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Missing packets detected */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PKT_MASS_LOSS,             L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Massive packet loss detected */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PHYS_FAILED,               L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< AMP physical link establishment failed */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_ILLEGAL_CID,               L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Illegal CID (out of range or does not exist) */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_INITIATING,                L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Not an error - connection setup initiated */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_RETRYING,                  L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Attempt failed, but retry has been issued */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_EXHAUSTED,                 L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Possible configurations exhausted */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PEER_ABORTED,              L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Peer aborted the operation in mid-air */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_LOCKSTEP_ERROR,            L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Peer violated the lockstep configuration procedure */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_INITIATOR_REFUSED,         L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Move channel fails on responder after initiator refuses */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_ALREADY_REGISTERED,        L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< PSM reserved - it's already been registered */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PHYS_NO_CONNECTION,        L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< AMP physical link is not there */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PHYS_CONN_TERM_LOCAL_HOST, L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< AMP physical link terminated locally */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_PHYS_INSUFFICIENT_SECURITY,L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< AMP physical link security failure */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_LOCAL_ABORTED,             L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Upper layers aborted operation */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_SILENT,                    L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Internal use - silences message */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_CRC_FAILURE,               L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Debugging - CRC check failed */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_RAW_COPY,                  L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Debugging - Raw copy of data */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_RAW,                       L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Debugging - Raw copy of data */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_CONFIG_ONGOING,            L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Configuration on-going, req rejected */\
    L2CA_RES_TABLE_X(L2CA_RESULT_REMOTE_UNSUPPORTED_FEATURE,L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< Unsupported remote feature */\
    L2CA_RES_TABLE_X(L2CA_RESULT_FLOW_VIOLATION,            L2CA_RES_TABLE_DUMMY)   L2CA_RES_TABLE_SEP /*!< LE Flow control protocol violation (either received credit count \
                                                                                                            exceeds 65535 or received LE-frame when credit count is zero) */ \
    L2CA_RES_TABLE_X(L2CA_RESULT_LINK_TRANSFERRED,          L2CA_RES_TABLE_DUMMY)                      /*!< Shadowed Link Disconnect */


#define L2CA_RES_TABLE_DUMMY
#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,

typedef enum
{
    L2CA_RES_TABLE
} L2CA_RESULT_T;

#undef L2CA_RES_TABLE_DUMMY
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP

/*! \} */


/*! \name L2CAP connect and create result codes

    \brief NOT_READY allows L2CAP to reject connect requests because
    LC not initialised and Buffer_Size_Ind not received by
    L2CAP. Also, KEY_MISSING will be used instead of REJ_SECURITY if
    we tried to authenticate an existing key and the remote device did
    not have one. REJ_CONTROL_ID and PHYS_FAILED are only used with
    AMP.
    Note :In the specification, connection results for BREDR and LE transport
          have their own range and it can be overlapping, so the users should
          be aware of the transport before interpreting the connection results
          from specification defined range. 
          However L2CAP internal error codes(from L2CA_ERRORCODE_BASE) mean the 
          same result for both BREDR and LE transport.

    \{ */
#define L2CA_CONRES_TABLE \
    L2CA_RES_TABLE_X(L2CA_CONNECT_SUCCESS,                      = L2CA_RESULT_SUCCESS)                      L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_PENDING,                      = 0x0001)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_REJ_PSM,                      = 0x0002)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_REJ_SECURITY,                 = 0x0003)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_REJ_RESOURCES,                = 0x0004)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_REJ_CONTROL_ID,               = 0x0005)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_REJ_INVALID_SRC_CID,          = 0x0006)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_REJ_SRC_CID_INUSE,            = 0x0007)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_NOT_READY,                    = L2CA_RESULT_NOT_READY)                    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_FAILED,                       = L2CA_RESULT_FAILED)                       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_KEY_MISSING,                  = L2CA_RESULT_KEY_MISSING)                  L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_TIMEOUT,                      = L2CA_RESULT_TIMEOUT)                      L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_PHYS_FAILED,                  = L2CA_RESULT_PHYS_FAILED)                  L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_INVALID_CONFTAB,              = L2CA_RESULT_CONFIG_ILLEGAL)               L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_CONFTAB_EXHAUSTED,            = L2CA_RESULT_EXHAUSTED)                    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_CONFTAB_UNSUPPORTED,          = L2CA_RESULT_UNSUPPORTED_MODE)             L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_INITIATING,                   = L2CA_RESULT_INITIATING)                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_RETRYING,                     = L2CA_RESULT_RETRYING)                     L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_PEER_ABORTED,                 = L2CA_RESULT_PEER_ABORTED)                 L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_OUT_OF_MEM,                   = L2CA_RESULT_OUT_OF_MEM)                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_PHYS_NO_CONNECTION,           = L2CA_RESULT_PHYS_NO_CONNECTION)           L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_PHYS_CONN_TERM_LOCAL_HOST,    = L2CA_RESULT_PHYS_CONN_TERM_LOCAL_HOST)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_PHYS_INSUFFICIENT_SECURITY,   = L2CA_RESULT_PHYS_INSUFFICIENT_SECURITY)   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_CONFIG_MISMATCH,              = L2CA_RESULT_CONFIG_MISMATCH)              L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_HARDWARE,                     = L2CA_RESULT_HARDWARE)                     L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_PEER_REJECT_CID,              = L2CA_RESULT_PEER_REJECT_CID)              L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_LINK_LOSS,                    = L2CA_RESULT_LINK_LOSS)                    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_LOCKSTEP_ERROR,               = L2CA_RESULT_LOCKSTEP_ERROR)               L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONNECT_CONFIG_ONGOING,               = L2CA_RESULT_CONFIG_ONGOING)

#define L2CA_LE_CONRES_TABLE \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_SUCCESS,                   = L2CA_RESULT_SUCCESS)                      L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_PSM,                   = 0x0002)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_RESOURCES,             = 0x0004)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_AUTHENTICATION,        = 0x0005)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_AUTHORIZATION,         = 0x0006)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_ENC_KEY_SIZE,          = 0x0007)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_ENCRYPTION,            = 0x0008)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_INVALID_SRC_CID,       = 0x0009)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REJ_SRC_CID_INUSE,         = 0x000a)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_LE_CONNECT_REMOTE_UNSUPPORTED,        = L2CA_RESULT_REMOTE_UNSUPPORTED_FEATURE)

#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,
typedef enum
{
    L2CA_CONRES_TABLE,
    L2CA_LE_CONRES_TABLE
} l2ca_conn_result_t;
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP
/*! \} */


/*! \name L2CAP AMP move channel error codes

    \brief The move channel is only used with AMP. Note that the codes
    are different from the normal connection setup ones.

    \{ */
#define L2CA_MOVERES_TABLE \
    L2CA_RES_TABLE_X(L2CA_MOVE_SUCCESS,                     = L2CA_RESULT_SUCCESS)                      L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_PENDING,                     = 0x0001)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_REJ_CONTROL_ID,              = 0x0002)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_REJ_SAME_ID,                 = 0x0003)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_REJ_CONFIG,                  = 0x0004)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_COLLISION,                   = 0x0005)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_REFUSED,                     = 0x0006)                                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_INTERNAL_PENDING,            = L2CA_RESULT_UNSUPPORTED_MODE)             L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_LOCAL_BUSY,                  = L2CA_RESULT_NOT_READY)                    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_PHYS_FAILED,                 = L2CA_RESULT_PHYS_FAILED)                  L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_TIMEOUT,                     = L2CA_RESULT_TIMEOUT)                      L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_INITIATING,                  = L2CA_RESULT_INITIATING)                   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_NOT_ALLOWED,                 = L2CA_RESULT_CONFIG_MISMATCH)              L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_INITIATOR_REFUSED,           = L2CA_RESULT_INITIATOR_REFUSED)            L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_PHYS_NO_CONNECTION,          = L2CA_RESULT_PHYS_NO_CONNECTION)           L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_PHYS_CONN_TERM_LOCAL_HOST,   = L2CA_RESULT_PHYS_CONN_TERM_LOCAL_HOST)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MOVE_PHYS_INSUFFICIENT_SECURITY,  = L2CA_RESULT_PHYS_INSUFFICIENT_SECURITY)

#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,
typedef enum
{
    L2CA_MOVERES_TABLE,
    L2CA_MOVE_CONFIRM_SUCCESS = 0,
    L2CA_MOVE_CONFIRM_REFUSED = 1
} l2ca_move_result_t;
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP
/*! \} */


/*! \name L2CAP disconnect indication/confirm reason codes

    \brief All of these are internal result code. It simply does not
    make sense to report error for a disconnect response. However, we
    are internally able to detect many disconnect issues, so most of
    these are caused by fatal errors in the application (eg. invalid
    mode requests).

    \{ */
#define L2CA_DISCRES_TABLE \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_NORMAL,            = L2CA_RESULT_SUCCESS)          L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_CONFIG_MISMATCH,   = L2CA_RESULT_CONFIG_MISMATCH)  L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_CONFIG_ILLEGAL,    = L2CA_RESULT_CONFIG_ILLEGAL)   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_RTM_OVERFLOW,      = L2CA_RESULT_RTM_OVERFLOW)     L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_HARDWARE,          = L2CA_RESULT_HARDWARE)         L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_UNSUPPORTED_MODE,  = L2CA_RESULT_UNSUPPORTED_MODE) L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_PEER_REJECT_CID,   = L2CA_RESULT_PEER_REJECT_CID)  L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_LINK_LOSS,         = L2CA_RESULT_LINK_LOSS)        L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_OUT_OF_MEM,        = L2CA_RESULT_OUT_OF_MEM)       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_SAR_VIOLATION,     = L2CA_RESULT_SAR_VIOLATION)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_FEC_HEADER,        = L2CA_RESULT_FEC_HEADER)       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_MTU_VIOLATION,     = L2CA_RESULT_MTU_VIOLATION)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_MPS_VIOLATION,     = L2CA_RESULT_MPS_VIOLATION)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_FEC_SEQUENCE,      = L2CA_RESULT_FEC_SEQUENCE)     L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_TIMEOUT,           = L2CA_RESULT_TIMEOUT)          L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_LOCKSTEP_ERROR,    = L2CA_RESULT_LOCKSTEP_ERROR)   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_FLOW_VIOLATION,    = L2CA_RESULT_FLOW_VIOLATION)   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DISCONNECT_LINK_TRANSFERRED,  = L2CA_RESULT_LINK_TRANSFERRED)

#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,
typedef enum
{
    L2CA_DISCRES_TABLE,
    L2CA_STAY_CONNECTED = L2CA_RESULT_SUCCESS /*!< Used internally only */
} l2ca_disc_result_t;
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP
/*! \} */

/*! \name L2CA_CONFIG response codes

    \brief Mixed spec/internal result codes. Note that UNACCEPTABLE is
    used when the entire set of options are rejected, whereas REJECTED
    should always come with a set of options that can otherwise be
    accepted. PENDING is only used with AMP and is handled
    automatically by L2CAP.

    Also note that a configuration or reconfiguration procedure is not
    over until both devices has sent a SUCCESS code.

    \{ */
#define L2CA_CONFRES_TABLE \
    L2CA_RES_TABLE_X(L2CA_CONFIG_SUCCESS,           = L2CA_RESULT_SUCCESS)          L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_UNACCEPTABLE,      = 0x0001)                       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_REJECTED,          = 0x0002)                       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_UNKNOWN,           = 0x0003)                       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_PENDING,           = 0x0004)                       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_FLOWSPEC_REJECTED, = 0x0005)                       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_REJECTED_LOCALLY,  = L2CA_RESULT_CONFIG_ILLEGAL)   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_OUT_OF_MEM,        = L2CA_RESULT_OUT_OF_MEM)       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_CONFIG_TIMEOUT,           = L2CA_RESULT_TIMEOUT)

#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,
typedef enum
{
    L2CA_CONFRES_TABLE
} l2ca_conf_result_t;
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP
/*! \} */


/*! \name L2CA_GETINFO_CFM result codes

    \brief Mixed spec/internal result codes for the information
    request operation. These are only passed as result codes. L2CAP
    will automatically deal with requests from the peer.
 
    \{ */
#define L2CA_INFORES_TABLE \
    L2CA_RES_TABLE_X(L2CA_GETINFO_SUCCESS,          = L2CA_RESULT_SUCCESS)  L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_GETINFO_NOT_SUPPORTED,    = 0x0001)               L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_GETINFO_FAILED,           = L2CA_RESULT_FAILED)   L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_GETINFO_TIMEOUT,          = L2CA_RESULT_TIMEOUT)

#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,
typedef enum
{
    L2CA_INFORES_TABLE
} l2ca_info_result_t;
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP
/*! \} */


/*! \name Data read/write result codes

    \brief All codes are internal. Note that LINK_TERIMNATED,
    NO_CONNECTION and LOST_ACK are only used in the datawrite cases,
    and FAIL, PARTIAL, PARTIAL_END, MISSING and MANY_MISSING are used
    only in the dataread cases. CRC_FAILURE is only used when debugging
    in raw mode. (Normally, packets with CRC errors are just dropped.)

    \{ */
#define L2CA_DATARES_TABLE \
    L2CA_RES_TABLE_X(L2CA_DATA_SUCCESS,         = L2CA_RESULT_SUCCESS)          L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_LINK_TERMINATED, = L2CA_RESULT_LINK_LOSS)        L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_NO_CONNECTION,   = L2CA_RESULT_NOT_READY)        L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_LOST_ACK,        = L2CA_RESULT_FEC_SEQUENCE)     L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_OUT_OF_MEM,      = L2CA_RESULT_OUT_OF_MEM)       L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_FAIL,            = L2CA_RESULT_MTU_VIOLATION)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_PARTIAL,         = L2CA_RESULT_READ_PARTIAL)     L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_PKT_MISSING,     = L2CA_RESULT_PKT_MISSING)      L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_PKT_MASS_LOSS,   = L2CA_RESULT_PKT_MASS_LOSS)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_LOCAL_ABORTED,   = L2CA_RESULT_LOCAL_ABORTED)    L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_SILENT,          = L2CA_RESULT_SILENT)           L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_CRC_FAILURE,     = L2CA_RESULT_CRC_FAILURE)      L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_RAW_COPY,        = L2CA_RESULT_RAW_COPY)         L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_DATA_RAW,             = L2CA_RESULT_RAW)


#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,
typedef enum
{
    L2CA_DATARES_TABLE,
    L2CA_DATA_PARTIAL_END = L2CA_RESULT_SUCCESS
} l2ca_data_result_t;
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP
/*! \} */


/*! \name Miscellaneous result codes

    \brief Result codes for internal L2CAP control like mapping fixed
    channels, connectionless reception, routing etc.

    \{ */
#define L2CA_MISCRES_TABLE \
    L2CA_RES_TABLE_X(L2CA_MISC_SUCCESS,             = L2CA_RESULT_SUCCESS)              L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_INITIATING,          = L2CA_RESULT_INITIATING)           L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_OUT_OF_MEM,          = L2CA_RESULT_OUT_OF_MEM)           L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_NO_CONNECTION,       = L2CA_RESULT_NOT_READY)            L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_INVALID_PSM,         = L2CA_RESULT_ILLEGAL_PSM)          L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_INVALID_CID,         = L2CA_RESULT_ILLEGAL_CID)          L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_FAILED,              = L2CA_RESULT_FAILED)               L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_TIMEOUT,             = L2CA_RESULT_TIMEOUT)              L2CA_RES_TABLE_SEP \
    L2CA_RES_TABLE_X(L2CA_MISC_ALREADY_REGISTERED,  = L2CA_RESULT_ALREADY_REGISTERED)

#define L2CA_RES_TABLE_X(a, b) a b
#define L2CA_RES_TABLE_SEP ,
typedef enum
{
    L2CA_MISCRES_TABLE
} l2ca_misc_result_t;
#undef L2CA_RES_TABLE_X
#undef L2CA_RES_TABLE_SEP
/*! \} */


/*! \name L2CAP primitive segmentation and numbering

    \brief L2CAP primitives occupy the number space from
    L2CA_PRIM_BASE to (L2CA_PRIM_BASE | 0x00FF).

    \{ */
#define L2CA_PRIM_DOWN                   L2CA_PRIM_BASE
#define L2CA_PRIM_UP                     (L2CA_PRIM_BASE | 0x0040)
#define L2CA_PRIM_MAX                    (L2CA_PRIM_BASE | 0x00FF)

typedef enum l2cap_prim_tag
{
    ENUM_L2CA_REGISTER_REQ = L2CA_PRIM_DOWN,
    ENUM_L2CA_AUTO_CONNECT_REQ,
    ENUM_L2CA_AUTO_CONNECT_RSP,
    ENUM_L2CA_DISCONNECT_REQ,
    ENUM_L2CA_DISCONNECT_RSP,
    ENUM_L2CA_PING_REQ,
    ENUM_L2CA_GETINFO_REQ,
    ENUM_L2CA_ROUTE_DATA_REQ,
    ENUM_L2CA_UNROUTE_DATA_REQ,
    ENUM_L2CA_DATAWRITE_REQ,
    ENUM_L2CA_MULTICAST_REQ,
    ENUM_L2CA_DATAREAD_RSP,
    ENUM_L2CA_BUSY_REQ,
    ENUM_L2CA_REGISTER_FIXED_CID_REQ,
    ENUM_L2CA_MAP_FIXED_CID_REQ,
    ENUM_L2CA_MAP_FIXED_CID_RSP,
    ENUM_L2CA_MOVE_CHANNEL_REQ,
    ENUM_L2CA_MOVE_CHANNEL_RSP,
    ENUM_L2CA_UNMAP_FIXED_CID_REQ,
    ENUM_L2CA_RAW_MODE_REQ,
    ENUM_L2CA_RAW_DATA_REQ,
    ENUM_L2CA_UNREGISTER_REQ,
    ENUM_L2CA_DATAWRITE_ABORT_REQ,
    ENUM_L2CA_GET_CHANNEL_INFO_REQ,
    ENUM_L2CA_CONNECTION_PAR_UPDATE_REQ,
    ENUM_L2CA_CONNECTION_PAR_UPDATE_IND,
    ENUM_L2CA_AUTO_TP_CONNECT_REQ,
    ENUM_L2CA_AUTO_TP_CONNECT_RSP,
    ENUM_L2CA_ADD_CREDIT_REQ,

    ENUM_L2CA_REGISTER_CFM = L2CA_PRIM_UP,
    ENUM_L2CA_AUTO_CONNECT_CFM,
    ENUM_L2CA_AUTO_CONNECT_IND,
    ENUM_L2CA_DISCONNECT_IND,
    ENUM_L2CA_DISCONNECT_CFM,
    ENUM_L2CA_TIMEOUT_IND,
    ENUM_L2CA_PING_CFM,
    ENUM_L2CA_GETINFO_CFM,
    ENUM_L2CA_ROUTE_DATA_CFM,
    ENUM_L2CA_UNROUTE_DATA_CFM,
    ENUM_L2CA_UNROUTE_DATA_IND,
    ENUM_L2CA_DATAWRITE_CFM,
    ENUM_L2CA_DATAREAD_IND,
    ENUM_L2CA_BUSY_IND,
    ENUM_L2CA_REGISTER_FIXED_CID_CFM,
    ENUM_L2CA_MAP_FIXED_CID_IND,
    ENUM_L2CA_MAP_FIXED_CID_CFM,
    ENUM_L2CA_MOVE_CHANNEL_CFM,
    ENUM_L2CA_MOVE_CHANNEL_IND,
    ENUM_L2CA_MOVE_CHANNEL_CMP_IND,
    ENUM_L2CA_AMP_LINK_LOSS_IND,
    ENUM_L2CA_MTU_VIOLATION_IND,
    ENUM_L2CA_UNKNOWN_SIGNAL_IND,
    ENUM_L2CA_RAW_DATA_IND,
    ENUM_L2CA_UNREGISTER_CFM,
    ENUM_L2CA_UNMAP_FIXED_CID_IND,
    ENUM_L2CA_DATAWRITE_ABORT_CFM,
    ENUM_L2CA_GET_CHANNEL_INFO_CFM,
    ENUM_L2CA_CONNECTION_PAR_UPDATE_CFM,
    ENUM_L2CA_CONNECTION_PAR_UPDATE_RSP,
    ENUM_L2CA_AUTO_TP_CONNECT_IND,
    ENUM_L2CA_AUTO_TP_CONNECT_CFM,
    ENUM_L2CA_ADD_CREDIT_CFM,

    /* Obsolete/Internal primitives. Not to be used by applications. */
    /* Use L2CA_AUTO_CONNECT_ instead. */
    ENUM_L2CA_CONNECT_REQ = L2CA_PRIM_MAX - 12,
    ENUM_L2CA_CONNECT_RSP = L2CA_PRIM_MAX - 11,
    ENUM_L2CA_CONFIG_REQ = L2CA_PRIM_MAX - 10,
    ENUM_L2CA_CONFIG_RSP = L2CA_PRIM_MAX - 9,
    ENUM_L2CA_CREATE_CHANNEL_REQ = L2CA_PRIM_MAX - 8,
    ENUM_L2CA_CREATE_CHANNEL_RSP = L2CA_PRIM_MAX - 7,
    ENUM_L2CA_CONNECT_IND = L2CA_PRIM_MAX - 6,
    ENUM_L2CA_CONNECT_CFM = L2CA_PRIM_MAX - 5,
    ENUM_L2CA_CONFIG_IND = L2CA_PRIM_MAX - 4,
    ENUM_L2CA_CONFIG_CFM = L2CA_PRIM_MAX - 3,
    ENUM_L2CA_CREATE_CHANNEL_CFM = L2CA_PRIM_MAX - 2,
    ENUM_L2CA_CREATE_CHANNEL_IND = L2CA_PRIM_MAX - 1,
    ENUM_L2CA_DEBUG_DROP_REQ = L2CA_PRIM_MAX
} L2CAP_PRIM_T;

/* Downstream primitives */
#define L2CA_REGISTER_REQ               ((l2cap_prim_t)ENUM_L2CA_REGISTER_REQ)
#define L2CA_AUTO_CONNECT_REQ           ((l2cap_prim_t)ENUM_L2CA_AUTO_CONNECT_REQ)
#define L2CA_AUTO_CONNECT_RSP           ((l2cap_prim_t)ENUM_L2CA_AUTO_CONNECT_RSP)
#define L2CA_DISCONNECT_REQ             ((l2cap_prim_t)ENUM_L2CA_DISCONNECT_REQ)
#define L2CA_DISCONNECT_RSP             ((l2cap_prim_t)ENUM_L2CA_DISCONNECT_RSP)
#define L2CA_PING_REQ                   ((l2cap_prim_t)ENUM_L2CA_PING_REQ)
#define L2CA_GETINFO_REQ                ((l2cap_prim_t)ENUM_L2CA_GETINFO_REQ)
#define L2CA_ROUTE_DATA_REQ             ((l2cap_prim_t)ENUM_L2CA_ROUTE_DATA_REQ)
#define L2CA_UNROUTE_DATA_REQ           ((l2cap_prim_t)ENUM_L2CA_UNROUTE_DATA_REQ)
#define L2CA_DATAWRITE_REQ              ((l2cap_prim_t)ENUM_L2CA_DATAWRITE_REQ)
#define L2CA_MULTICAST_REQ              ((l2cap_prim_t)ENUM_L2CA_MULTICAST_REQ)
#define L2CA_DATAREAD_RSP               ((l2cap_prim_t)ENUM_L2CA_DATAREAD_RSP)
#define L2CA_BUSY_REQ                   ((l2cap_prim_t)ENUM_L2CA_BUSY_REQ)
#define L2CA_REGISTER_FIXED_CID_REQ     ((l2cap_prim_t)ENUM_L2CA_REGISTER_FIXED_CID_REQ)
#define L2CA_MAP_FIXED_CID_REQ          ((l2cap_prim_t)ENUM_L2CA_MAP_FIXED_CID_REQ)
#define L2CA_MAP_FIXED_CID_RSP          ((l2cap_prim_t)ENUM_L2CA_MAP_FIXED_CID_RSP)
#define L2CA_MOVE_CHANNEL_REQ           ((l2cap_prim_t)ENUM_L2CA_MOVE_CHANNEL_REQ)
#define L2CA_MOVE_CHANNEL_RSP           ((l2cap_prim_t)ENUM_L2CA_MOVE_CHANNEL_RSP)
#define L2CA_UNMAP_FIXED_CID_REQ        ((l2cap_prim_t)ENUM_L2CA_UNMAP_FIXED_CID_REQ)
#define L2CA_RAW_MODE_REQ               ((l2cap_prim_t)ENUM_L2CA_RAW_MODE_REQ)
#define L2CA_RAW_DATA_REQ               ((l2cap_prim_t)ENUM_L2CA_RAW_DATA_REQ)
#define L2CA_UNREGISTER_REQ             ((l2cap_prim_t)ENUM_L2CA_UNREGISTER_REQ)
#define L2CA_DATAWRITE_ABORT_REQ        ((l2cap_prim_t)ENUM_L2CA_DATAWRITE_ABORT_REQ)
#define L2CA_GET_CHANNEL_INFO_REQ       ((l2cap_prim_t)ENUM_L2CA_GET_CHANNEL_INFO_REQ)
#define L2CA_AUTO_TP_CONNECT_REQ        ((l2cap_prim_t)ENUM_L2CA_AUTO_TP_CONNECT_REQ)
#define L2CA_AUTO_TP_CONNECT_RSP        ((l2cap_prim_t)ENUM_L2CA_AUTO_TP_CONNECT_RSP)
#define L2CA_ADD_CREDIT_REQ             ((l2cap_prim_t)ENUM_L2CA_ADD_CREDIT_REQ)

/* Upstream primitives */
#define L2CA_REGISTER_CFM               ((l2cap_prim_t)ENUM_L2CA_REGISTER_CFM)
#define L2CA_AUTO_CONNECT_CFM           ((l2cap_prim_t)ENUM_L2CA_AUTO_CONNECT_CFM)
#define L2CA_AUTO_CONNECT_IND           ((l2cap_prim_t)ENUM_L2CA_AUTO_CONNECT_IND)
#define L2CA_DISCONNECT_IND             ((l2cap_prim_t)ENUM_L2CA_DISCONNECT_IND)
#define L2CA_DISCONNECT_CFM             ((l2cap_prim_t)ENUM_L2CA_DISCONNECT_CFM)
#define L2CA_TIMEOUT_IND                ((l2cap_prim_t)ENUM_L2CA_TIMEOUT_IND)
#define L2CA_PING_CFM                   ((l2cap_prim_t)ENUM_L2CA_PING_CFM)
#define L2CA_GETINFO_CFM                ((l2cap_prim_t)ENUM_L2CA_GETINFO_CFM)
#define L2CA_ROUTE_DATA_CFM             ((l2cap_prim_t)ENUM_L2CA_ROUTE_DATA_CFM)
#define L2CA_UNROUTE_DATA_CFM           ((l2cap_prim_t)ENUM_L2CA_UNROUTE_DATA_CFM)
#define L2CA_UNROUTE_DATA_IND           ((l2cap_prim_t)ENUM_L2CA_UNROUTE_DATA_IND)
#define L2CA_DATAWRITE_CFM              ((l2cap_prim_t)ENUM_L2CA_DATAWRITE_CFM)
#define L2CA_DATAREAD_IND               ((l2cap_prim_t)ENUM_L2CA_DATAREAD_IND)
#define L2CA_BUSY_IND                   ((l2cap_prim_t)ENUM_L2CA_BUSY_IND)
#define L2CA_REGISTER_FIXED_CID_CFM     ((l2cap_prim_t)ENUM_L2CA_REGISTER_FIXED_CID_CFM)
#define L2CA_MAP_FIXED_CID_IND          ((l2cap_prim_t)ENUM_L2CA_MAP_FIXED_CID_IND)
#define L2CA_MAP_FIXED_CID_CFM          ((l2cap_prim_t)ENUM_L2CA_MAP_FIXED_CID_CFM)
#define L2CA_MOVE_CHANNEL_CFM           ((l2cap_prim_t)ENUM_L2CA_MOVE_CHANNEL_CFM)
#define L2CA_MOVE_CHANNEL_IND           ((l2cap_prim_t)ENUM_L2CA_MOVE_CHANNEL_IND)
#define L2CA_MOVE_CHANNEL_CMP_IND       ((l2cap_prim_t)ENUM_L2CA_MOVE_CHANNEL_CMP_IND)
#define L2CA_AMP_LINK_LOSS_IND          ((l2cap_prim_t)ENUM_L2CA_AMP_LINK_LOSS_IND)
#define L2CA_MTU_VIOLATION_IND          ((l2cap_prim_t)ENUM_L2CA_MTU_VIOLATION_IND)
#define L2CA_UNKNOWN_SIGNAL_IND         ((l2cap_prim_t)ENUM_L2CA_UNKNOWN_SIGNAL_IND)
#define L2CA_RAW_DATA_IND               ((l2cap_prim_t)ENUM_L2CA_RAW_DATA_IND)
#define L2CA_UNREGISTER_CFM             ((l2cap_prim_t)ENUM_L2CA_UNREGISTER_CFM)
#define L2CA_UNMAP_FIXED_CID_IND        ((l2cap_prim_t)ENUM_L2CA_UNMAP_FIXED_CID_IND)
#define L2CA_DATAWRITE_ABORT_CFM        ((l2cap_prim_t)ENUM_L2CA_DATAWRITE_ABORT_CFM)
#define L2CA_GET_CHANNEL_INFO_CFM       ((l2cap_prim_t)ENUM_L2CA_GET_CHANNEL_INFO_CFM)
#define L2CA_AUTO_TP_CONNECT_IND        ((l2cap_prim_t)ENUM_L2CA_AUTO_TP_CONNECT_IND)
#define L2CA_AUTO_TP_CONNECT_CFM        ((l2cap_prim_t)ENUM_L2CA_AUTO_TP_CONNECT_CFM)
#define L2CA_ADD_CREDIT_CFM             ((l2cap_prim_t)ENUM_L2CA_ADD_CREDIT_CFM)

/* Debug primitives */
#define L2CA_DEBUG_DROP_REQ             ((l2cap_prim_t)ENUM_L2CA_DEBUG_DROP_REQ)

/* Obsolete/Internal primitives. Not for use by applications. */
/* Applications should use L2CA_AUTO_CONNECT_ instead. */
#define L2CA_CONNECT_REQ                ((l2cap_prim_t)ENUM_L2CA_CONNECT_REQ)
#define L2CA_CONNECT_RSP                ((l2cap_prim_t)ENUM_L2CA_CONNECT_RSP)
#define L2CA_CONFIG_REQ                 ((l2cap_prim_t)ENUM_L2CA_CONFIG_REQ)
#define L2CA_CONFIG_RSP                 ((l2cap_prim_t)ENUM_L2CA_CONFIG_RSP)
#define L2CA_CREATE_CHANNEL_REQ         ((l2cap_prim_t)ENUM_L2CA_CREATE_CHANNEL_REQ)
#define L2CA_CREATE_CHANNEL_RSP         ((l2cap_prim_t)ENUM_L2CA_CREATE_CHANNEL_RSP)
#define L2CA_CONNECT_IND                ((l2cap_prim_t)ENUM_L2CA_CONNECT_IND)
#define L2CA_CONNECT_CFM                ((l2cap_prim_t)ENUM_L2CA_CONNECT_CFM)
#define L2CA_CONFIG_IND                 ((l2cap_prim_t)ENUM_L2CA_CONFIG_IND)
#define L2CA_CONFIG_CFM                 ((l2cap_prim_t)ENUM_L2CA_CONFIG_CFM)
#define L2CA_CREATE_CHANNEL_CFM         ((l2cap_prim_t)ENUM_L2CA_CREATE_CHANNEL_CFM)
#define L2CA_CREATE_CHANNEL_IND         ((l2cap_prim_t)ENUM_L2CA_CREATE_CHANNEL_IND)

#define L2CA_CONNECTION_PAR_UPDATE_REQ  ((l2cap_prim_t)ENUM_L2CA_CONNECTION_PAR_UPDATE_REQ)
#define L2CA_CONNECTION_PAR_UPDATE_CFM  ((l2cap_prim_t)ENUM_L2CA_CONNECTION_PAR_UPDATE_CFM)

#define L2CA_CONNECTION_PAR_UPDATE_IND  ((l2cap_prim_t)ENUM_L2CA_CONNECTION_PAR_UPDATE_IND)
#define L2CA_CONNECTION_PAR_UPDATE_RSP  ((l2cap_prim_t)ENUM_L2CA_CONNECTION_PAR_UPDATE_RSP)
/*! \} */
/*! \brief Primitive type for l2cap */
typedef uint16_t                        l2cap_prim_t;           /*!< Primitive identity */

typedef uint16_t                        l2ca_conflags_t;        /*!< Special connection flags */

/*! \name Result/response code types
  \{ */
typedef uint16_t                        l2ca_leup_result_t;     /*!< Low energy connection parameter update */
/*! \} */

typedef uint16_t                        l2ca_raw_t;             /*!< Raw debug mode */
typedef uint8_t                         l2ca_raw_packet_t;      /*!< Packet type - L2CA_RX/L2CA_TX_START/L2CA_TX_CONT */

/*! \name L2CAP option typedefs
  \{ */
typedef uint16_t                        l2ca_options_t;         /*!< L2CAP options bitmask */
typedef uint8_t                         l2ca_qos_flags_t;       /*!< QoS flags */
typedef uint8_t                         l2ca_service_type_t;    /*!< QoS service type */
typedef uint32_t                        l2ca_token_rate_t;      /*!< QoS token rate */
typedef uint32_t                        l2ca_token_bucket_t;    /*!< QoS token bucket size */
typedef uint32_t                        l2ca_variation_t;       /*!< QoS variation */
typedef uint8_t                         l2ca_flow_mode_t;       /*!< F&EC mode */
typedef uint8_t                         l2ca_max_retransmit_t;  /*!< F&EC max transmit counter */
typedef uint16_t                        l2ca_mtu_t;             /*!< Maximum transmission unit */
typedef uint16_t                        l2ca_timeout_t;         /*!< Flush timeout */
typedef uint8_t                         l2ca_fcs_t;             /*!< F&EC optional frame check sequence */
typedef uint8_t                         l2ca_window_size_t;     /*!< F&EC window size */
typedef uint16_t                        l2ca_ext_window_size_t; /*!< Extended window size */
/*! \} */


/*! \brief L2CAP quality of service options

    This is the old QoS type definition. It takes up 22 octets over
    the air.
*/
typedef struct
{
    l2ca_qos_flags_t       flags;             /*!< Flags */
    l2ca_service_type_t    service_type;      /*!< Best effort, etc */
    l2ca_token_rate_t      token_rate;        /*!< Token rate */
    l2ca_token_bucket_t    token_bucket;      /*!< Token bucket */
    l2ca_bandwidth_t       peak_bw;           /*!< Peak bandwidth */
    l2ca_latency_t         latency;           /*!< Latency */
    l2ca_variation_t       delay_var;         /*!< Delay variation */
} L2CA_QOS_T;


/*! \brief L2CAP flow and error control options

    This structure controls the L2CAP flow & error control modes and
    the associated paramters. It takes up 9 octets over the air.
*/
typedef struct
{
    l2ca_flow_mode_t       mode;              /*!< Retransmission/flow control mode */
    l2ca_window_size_t     tx_window;         /*!< Size of transmit window */
    l2ca_max_retransmit_t  max_retransmit;    /*!< Maximum number of retransmissions */
    l2ca_timeout_t         retrans_timeout;   /*!< Retransmission timeout, in ms */
    l2ca_timeout_t         monitor_timeout;   /*!< Monitor timeout, in ms */
    l2ca_mtu_t             maximum_pdu;       /*!< Maximum PDU size */
} L2CA_FLOW_T;

/*! \brief L2CAP configuration parameters

    This is the common structure that contains all defined
    configuration options for L2CAP.
*/
typedef struct
{
    l2ca_options_t         options;           /*!< Bitmask for options present: L2CA_SPECIFY_... */
    l2ca_options_t         hints;             /*!< Which of the options are hints */
    uint8_t                unknown_length;    /*!< Total octet length of unknown options */
    uint8_t               *unknown;           /*!< Pointer to unknown options */
    l2ca_mtu_t             mtu;               /*!< MTU */
    l2ca_timeout_t         flush_to;          /*!< Flush timeout */
    L2CA_QOS_T            *qos;               /*!< Quality of service */
    L2CA_FLOW_T           *flow;              /*!< Flow and error control */
    l2ca_fcs_t             fcs;               /*!< Optional FCS */
    L2CA_FLOWSPEC_T       *flowspec;          /*!< Extended AMP flow specification */
    l2ca_ext_window_size_t ext_window;        /*!< Extended window size */
} L2CA_CONFIG_T;


/*! \brief L2CAP configuration parameters on a specific transport

    This is the common structure that contains all defined configuration options
    for L2CAP. This is an extended version of L2CA_CONFIG_T to accomodate 
    configuration information for LE connection oriented channels
*/
typedef struct
{
    l2ca_options_t         options;           /*!< Bitmask for options present: L2CA_SPECIFY_... */
    l2ca_options_t         hints;             /*!< Which of the options are hints */
    uint8_t                unknown_length;    /*!< Total octet length of unknown options */
    uint8_t               *unknown;           /*!< Pointer to unknown options */
    l2ca_mtu_t             mtu;               /*!< MTU */
    l2ca_timeout_t         flush_to;          /*!< Flush timeout */
    L2CA_QOS_T            *qos;               /*!< Quality of service */
    L2CA_FLOW_T           *flow;              /*!< Flow and error control */
    l2ca_fcs_t             fcs;               /*!< Optional FCS */
    L2CA_FLOWSPEC_T       *flowspec;          /*!< Extended AMP flow specification */
    l2ca_ext_window_size_t ext_window;        /*!< Extended window size */
    uint16_t               credits;           /*!< Total credits extended to remote device */
    l2ca_mtu_t             remote_mtu;        /*!< Remote's MTU */
} L2CA_TP_CONFIG_T;

typedef struct
{
    l2cap_prim_t        type;
    l2ca_cid_t          cid;
} L2CA_DOWNSTREAM_CID_COMMON_T;/* autogen_makefile_ignore_this (DO NOT REMOVE THIS COMMENT) */

typedef struct
{
    l2cap_prim_t        type;
    phandle_t           phandle;
    l2ca_cid_t          cid;
} L2CA_UPSTREAM_CID_COMMON_T;/* autogen_makefile_ignore_this (DO NOT REMOVE THIS COMMENT) */



/*! \brief L2CAP connection update req/cfm primitves exchanged between DM and L2CAP

    \msc
        DM,L2CAP,"Peer L2CAP","Peer DM";

        DM->L2CAP                           [ label = "L2CA_CONNECTION_PAR_UPDATE_REQ", URL = "\ref L2CA_CONNECTION_PAR_UPDATE_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="l2cap_connection_par_update_ind" ];
        "Peer L2CAP"->"Peer DM"             [ label = "L2CA_CONNECTION_PAR_UPDATE_IND", URL = "\ref L2CA_CONNECTION_PAR_UPDATE_IND_T" ];
        "Peer DM"->"Peer L2CAP"             [ label = "L2CA_CONNECTION_PAR_UPDATE_RSP", URL = "\ref L2CA_CONNECTION_PAR_UPDATE_RSP_T" ];
        "Peer L2CAP"->L2CAP                 [ label="L2CA_ConnUpdateCfm" ];
        L2CAP->DM                           [ label = "L2CA_CONNECTION_PAR_UPDATE_CFM", URL = "\ref L2CA_CONNECTION_PAR_UPDATE_CFM_T" ];
    \endmsc

*/
typedef struct
{
    l2cap_prim_t        type;           /*!< L2CA_CONNECTION_PAR_UPDATE_REQ */
    phandle_t           phandle;        /*!< phandle for the response */
    TYPED_BD_ADDR_T     addrt;          /*!< Bluetooth device address of remote device */
    uint16_t            min_interval;   /*!< Minimum allowed connection interval */
    uint16_t            max_interval;   /*!< Maximum allowed connection interval */
    uint16_t            latency;        /*!< connection slave latency */
    uint16_t            timeout;        /*!< supervision timeout */
} L2CA_CONNECTION_PAR_UPDATE_REQ_T;

typedef struct
{
    l2cap_prim_t        type;           /*!< L2CA_CONNECTION_PAR_UPDATE_CFM */
    phandle_t           phandle;        /*!< phandle for the response */
    TYPED_BD_ADDR_T     addrt;          /*!< Bluetooth device address of remote device */
    l2ca_misc_result_t  result;         /*!< Result */
} L2CA_CONNECTION_PAR_UPDATE_CFM_T;


/*! \brief L2CAP accept connection update primitves exchanged between DM and L2CAP
    primitives internal between DM and L2CAP to carry connection parameter update
    indication and response between these two modules.
*/
typedef struct
{
    l2cap_prim_t type;              /*!< L2CA_CONNECTION_PAR_UPDATE_IND */
    phandle_t phandle;              /*!< phandle for the response */
    TYPED_BD_ADDR_T bd_addrt;       /*!< Bluetooth device address of remote device */
    l2ca_identifier_t signal_id;    /*!< L2CAP Signal ID - application must respond back to generate a response signal */
    uint16_t conn_interval_min;     /*!< Minimum allowed connection interval */
    uint16_t conn_interval_max;     /*!< Maximum allowed connection interval */
    uint16_t conn_latency;          /*!< connection slave latency */
    uint16_t supervision_timeout;   /*!< link timeout value */
} L2CA_CONNECTION_PAR_UPDATE_IND_T;


typedef struct
{
    l2cap_prim_t type;              /*!< L2CA_CONNECTION_PAR_UPDATE_RSP */
    phandle_t phandle;              /*!< destination handle */
    l2ca_identifier_t signal_id;    /*!< L2CAP Signal ID */
    TYPED_BD_ADDR_T bd_addrt;       /*!< Bluetooth device address  of remote device required to generate connection handle*/
    uint16_t result;                /*!< L2CAP_CONNECTION_PARAMETER_UPDATE_ACCEPT - if parameters are acceptable,
                                      !< L2CAP_CONNECTION_PARAMETER_UPDATE_REJECT - non-acceptable  */
} L2CA_CONNECTION_PAR_UPDATE_RSP_T;



/*! \brief Register PSM request

    Register a PSM so that L2CAP can multiplex between protocols.

    In the case where we are operating under the QTIL scheduler,
    the phandle is the queue identifier for a particular higher
    layer protocol.
    To register a PSM on LE transport use L2CA_REGFLAG_FOR_LE_TP and 
    L2CA_REGFLAG_USE_TP_PRIMS register flags

    \msc
        Application,L2CAP;

        Application->L2CAP                  [ label = "L2CA_REGISTER_REQ", URL = "\ref L2CA_REGISTER_REQ_T" ];
        L2CAP->Application                  [ label = "L2CA_REGISTER_CFM", URL = "\ref L2CA_REGISTER_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_REGISTER_REQ */
    psm_t               psm_local;        /*!< Defines the PSM used by the entity registering with L2CAP */
    phandle_t           phandle;          /*!< Protocol handle of the higher layer entity registering with L2CAP, for example RFCOMMs */
    uint16_t            mode_mask;        /*!< Mask of which L2CAP modes are acceptable, shall be set to zero for LE transport.
                                               see the L2CA_MODE_MASK-defines */
    uint16_t            flags;            /*!< Register flags. */
    context_t           reg_ctx;          /*!< Registration context. */

    STREAM_BUFFER_SIZES_T connection_oriented; /*!< Stream parameters, currently all reserved and shall be set to zero. */
    STREAM_BUFFER_SIZES_T connectionless;      /*!< Stream parameters, currently all reserved and shall be set to zero. */
} L2CA_REGISTER_REQ_T;

/*! \brief Register PSM confirmation

    Confirmation of registration of a PSM with L2CAP.  PSM registration will always
    succeed so no success/failure indication is returned.

    \msc
        Application,L2CAP;

        Application->L2CAP                  [ label = "L2CA_REGISTER_REQ", URL = "\ref L2CA_REGISTER_REQ_T" ];
        L2CAP->Application                  [ label = "L2CA_REGISTER_CFM", URL = "\ref L2CA_REGISTER_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_REGISTER_CFM */
    phandle_t           phandle;          /*!< Protocol handle of the higher layer entity as specified in request */
    psm_t               psm_local;        /*!< PSM as specified in request */
    context_t           reg_ctx;          /*!< Registration context as specified in request */
    uint16_t            mode_mask;        /*!< Mask of which L2CAP modes are acceptable */
    uint16_t            flags;            /*!< Flags as specified in request */
    l2ca_misc_result_t  result;           /*!< Return code - uses L2CA_MISC range */
} L2CA_REGISTER_CFM_T;

/*! \brief Unregister PSM request

    Remove a previously registered PSM.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_UNREGISTER_REQ */
    psm_t               psm_local;        /*!< The registered PSM */
    phandle_t           phandle;          /*!< Send result to this phandle */
} L2CA_UNREGISTER_REQ_T;

/*! \brief Unregister PSM confirmation.

    Send to the application to indicate the result of an \ref
    L2CA_UNREGISTER_REQ_T
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_UNREGISTER_CFM */
    phandle_t           phandle;          /*!< Target application handle */
    psm_t               psm_local;        /*!< PSM as specified in unregister request */
    context_t           reg_ctx;          /*!< Registration context */
    l2ca_misc_result_t  result;           /*!< Return code - uses L2CA_MISC range */
} L2CA_UNREGISTER_CFM_T;

/*! \brief Register fixed CID request

    Register a fixed CID so that data can be received and sent on the
    fixed channel. This must happen before any of the
    \ref L2CA_MAP_FIXED_CID_REQ_T, \ref L2CA_MAP_FIXED_CID_CFM_T and
    \ref L2CA_MAP_FIXED_CID_IND_T can be used.

    \msc
        Application,L2CAP;

        Application->L2CAP                  [ label = "L2CA_REGISTER_FIXED_CID_REQ", URL = "\ref L2CA_REGISTER_FIXED_CID_REQ_T" ];
        L2CAP->Application                  [ label = "L2CA_REGISTER_FIXED_CID_CFM", URL = "\ref L2CA_REGISTER_FIXED_CID_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_REGISTER_FIXED_CID_REQ */
    phandle_t           phandle;          /*!< Protocol handle of higher layer entity registering with L2CAP, for example AMP Manager */
    l2ca_cid_t          fixed_cid;        /*!< Channel ID of fixed channel */
    context_t           reg_ctx;          /*!< Reserved - ignored by L2CAP */
    L2CA_CONFIG_T       config;           /*!< Configuration of channel */

    /* Stream parameters are currently reserved and shall be set to zero. */
    STREAM_BUFFER_SIZES_T streams;        /*!< Stream buffer sizes */
} L2CA_REGISTER_FIXED_CID_REQ_T;

/*! \brief Register fixed CID confirmation

    Confirmation of registration of fixed channel registration.

    \msc
        Application,L2CAP;

        Application->L2CAP                  [ label = "L2CA_REGISTER_FIXED_CID_REQ", URL = "\ref L2CA_REGISTER_FIXED_CID_REQ_T" ];
        L2CAP->Application                  [ label = "L2CA_REGISTER_FIXED_CID_CFM", URL = "\ref L2CA_REGISTER_FIXED_CID_CFM_T" ];
    \endmsc
 */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_REGISTER_FIXED_CID_CFM */
    phandle_t           phandle;          /*!< Protocol handle of higher layer entity as specified in request */
    l2ca_cid_t          fixed_cid;        /*!< Channel ID of fixed channel as specified in request */
    context_t           reg_ctx;          /*!< Reserved - should be ignored */
    l2ca_misc_result_t  result;           /*!< Return code - uses L2CA_MISC range */
} L2CA_REGISTER_FIXED_CID_CFM_T;

/*! \brief Automatic connection request

    The \c L2CA_AUTO_CONNECT_REQ \c primitive is used to initiate a new L2CAP channel.

    When the \c cid \c in the auto-connect request is set to 0, a new L2CAP channel
    creation will be attempted to the peer specified by the bd_addr and psm values.

    The \c con_ctx \c value is not used by BlueStack and can be used by the application. 
    This number will be returned in future L2CAP primitives regarding this connection, 
    for example the \ref L2CA_AUTO_CONNECT_CFM_T and \ref L2CA_DISCONNECT_IND_T.

    The \c conftab \c is a special Key/Value pair encoded table that is used to 
    pass configuration options to L2CAP. It is legal to pass the \c NULL \c pointer, 
    otherwise the pointer must point to an array of uint16_t's, and the length 
    specified in conftab_length must be set to the number of uint16_t's in the conftab.
    For information about how to encode a conftab table, please
    see the header file bkeyval.h.

    \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        Application->L2CAP                  [ label="L2CA_AUTO_CONNECT_REQ",    URL="\ref L2CA_AUTO_CONNECT_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_ConnectReq" ];
        L2CAP->Application                  [ label="L2CAP_AUTO_CONNECT_CFM  result=INITIATING", URL="\ref L2CA_AUTO_CONNECT_CFM_T"];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConnectRspPnd" ];
        L2CAP->Application                  [ label="L2CAP_AUTO_CONNECT_CFM  result=PENDING",  URL="\ref L2CA_AUTO_CONNECT_CFM_T"];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_AUTO_CONNECT_IND",    URL="\ref L2CA_AUTO_CONNECT_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_AUTO_CONNECT_RSP",    URL="\ref L2CA_AUTO_CONNECT_RSP_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CA_ConfigRequest"];
        "Peer L2CAP"->L2CAP                 [ label="L2CA_ConfigRequest"];
        "Peer L2CAP"->L2CAP                 [ label="L2CA_ConfigResponse"];
        L2CAP->"Peer L2CAP"                 [ label="L2CA_ConfigResponse"];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_AUTO_CONNECT_CFM",    URL="\ref L2CA_AUTO_CONNECT_CFM_T" ];
        L2CAP->Application                  [ label="L2CA_AUTO_CONNECT_CFM",    URL="\ref L2CA_AUTO_CONNECT_CFM_T" ];
        ---   [ label="Connection Established" ];
        
    \endmsc
    
    \see L2CA_AUTO_CONNECT_CFM_T
    \see L2CA_DISCONNECT_IND_T

*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_AUTO_CONNECT_REQ */
    l2ca_cid_t          cid;              /*!< Channel ID - only used for reconfigurations, otherwise set to 0 */
    psm_t               psm_local;        /*!< PSM of local requesting protocol */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_remote;       /*!< The PSM on the remote device */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    l2ca_controller_t   remote_control;   /*!< Remote controller ID */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
    uint16_t            conftab_length;   /*!< Number of uint16_t's in the 'conftab' table */
    uint16_t           *conftab;          /*!< Configuration table (key,value pairs) */
} L2CA_AUTO_CONNECT_REQ_T;

/*! \brief Automatic connection request

    The \c L2CA_TP_AUTO_CONNECT_REQ \c primitive is used to initiate a new
    L2CAP channel on a specific transport.

    \see L2CA_AUTO_CONNECT_REQ_T

    BR/EDR connection : 
    L2CA_AUTO_TP_CONNECT_REQ_T is an extended version of L2CA_AUTO_CONNECT_REQ_T
    to accomodate LE connection oriented channels. Except for tp_addrt 
    information, usage of this API for BR/EDR connection would remain the same.

    LE Connection : 
    Initial rx credits and MTU values are required to be filled in conftab,
    otherwise L2CAP intiates connection with default values. Rx_credits can be
    extended once connection is established, by using L2CA_ADD_CREDIT_REQ prim.
    Connection can be initiated in the LL-Slave role by using 
    L2CA_AUTOPT_CONN_FLAGS key and appropriate l2ca_conflags_t as value.

*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_AUTO_TP_CONNECT_REQ */
    l2ca_cid_t          cid;              /*!< Channel ID - only used for reconfigurations,0 for LE */
    psm_t               psm_local;        /*!< PSM of local requesting protocol */
    TP_BD_ADDR_T        tp_addrt;         /*!< Address of remote device including type and transport */
    psm_t               psm_remote;       /*!< The PSM on the remote device */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    l2ca_controller_t   remote_control;   /*!< Remote controller ID */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
    uint16_t            conftab_length;   /*!< Number of uint16_t's in the 'conftab' table */
    uint16_t           *conftab;          /*!< Configuration table (key,value pairs) */
} L2CA_AUTO_TP_CONNECT_REQ_T;

/*! \brief Automatic connection response

    L2CA_AUTO_CONNECT_RSP, the auto-connect response that is sent after the 
    application has received a L2CA_AUTO_CONNECT_IND.
    
    The auto-connect response must always be sent when a L2CA_AUTO_CONNECT_IND has 
    been received by the application. The response must contain the same values 
    for identifier and cid as the indication.

    The \c con_ctx \c is an opaque number that can be used for application specific purposes. 
    This value will be returned in subsequent L2CAP related messages, 
    e.g. \ref L2CA_AUTO_CONNECT_CFM_T and \ref L2CA_DISCONNECT_IND_T.

    The \c conftab \c must point to the BlueStack Key/Value Pair table. 
    The \c conftab_length \c value identifies the number of unit16s that make up 
    the BlueStack Key/Value Pair table. 
    It is legal to pass a NULL pointer.
    For information about how to encode a conftab table, please
    see the header file bkeyval.h.

    See MSC of \ref L2CA_AUTO_CONNECT_REQ_T

    \see L2CA_AUTO_CONNECT_IND_T
    \see L2CA_AUTO_CONNECT_REQ_T
    
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_AUTO_CONNECT_RSP */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    l2ca_identifier_t   identifier;       /*!< Used to identify the connect signal */
    l2ca_conn_result_t  response;         /*!< Result code - uses L2CA_CONNECT range (only spec values allowed) */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    uint16_t            conftab_length;   /*!< Number of uint16_t's in the 'conftab' table */
    uint16_t           *conftab;          /*!< Configuration table (key,value pairs) */
} L2CA_AUTO_CONNECT_RSP_T;


/*! \brief Automatic connection response on a specific transport

    L2CA_AUTO_TP_CONNECT_RSP, the auto-connect response that is sent after the 
    application has received a L2CA_AUTO_TP_CONNECT_IND.

    \see L2CA_AUTO_CONNECT_RSP_T

    BR/EDR connection : 
    L2CA_AUTO_TP_CONNECT_RSP_T is an extended version of L2CA_AUTO_CONNECT_RSP_T
    to accomodate LE connection oriented channels. Usage of this API for BR/EDR 
    connection would remain the same as L2CA_AUTO_CONNECT_RSP_T.

    LE Connection : 
    Initial rx credits and MTU values are required to be filled in conftab to 
    prevent L2CAP assuming default values. 
*/

typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_AUTO_TP_CONNECT_RSP */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    l2ca_identifier_t   identifier;       /*!< Used to identify the connect signal */
    l2ca_conn_result_t  response;         /*!< Result code - uses L2CA_CONNECT range (only spec values allowed) */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    uint16_t            conftab_length;   /*!< Number of uint16_t's in the 'conftab' table */
    uint16_t           *conftab;          /*!< Configuration table (key,value pairs) */
} L2CA_AUTO_TP_CONNECT_RSP_T;


/*! \brief Automatic connection indication

    \c L2CA_AUTO_CONNECT_IND \c which used on the responder side when a peer wants to 
    initiate a connection. There are no special parameters for the auto-connect indication.
    L2CAP expects a L2CA_AUTO_CONNECT_RSP
    which allows the application to either accept or reject the connection. 

    See MSC of \ref L2CA_AUTO_CONNECT_REQ_T

    \see L2CA_AUTO_CONNECT_RSP_T
    \see L2CA_AUTO_CONNECT_REQ_T
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_AUTO_CONNECT_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Registration context */
    l2ca_identifier_t   identifier;       /*!< Used to identify the connect signal */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_local;        /*!< PSM of the local device */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
} L2CA_AUTO_CONNECT_IND_T;


/*! \brief Automatic connection indication on specific transport

    \c L2CA_AUTO_TP_CONNECT_IND \c is used on the responder side to indicate 
    an incoming connection request from a peer device on a specific transport. 
    Conftab is reserved for future use. Conftab passed on to the application is 
    owned and freed by the application.
    \see L2CA_AUTO_CONNECT_IND_T
*/

typedef struct
{
    l2cap_prim_t         type;             /*!< Always L2CA_AUTO_TP_CONNECT_IND */
    phandle_t            phandle;          /*!< Destination phandle */
    l2ca_cid_t           cid;              /*!< Channel identifier */
    context_t            reg_ctx;          /*!< Registration context */
    l2ca_identifier_t    identifier;       /*!< Used to identify the connect signal */
    TP_BD_ADDR_T         tp_addrt;         /*!< Address of remote device including type and transport */
    psm_t                psm_local;        /*!< PSM of the local device */
    l2ca_controller_t    local_control;    /*!< Local controller ID */
    l2ca_conflags_t      flags;            /*!< Special connection flags */
    uint16_t             conftab_length;   /*!< Reserved for future use */
    uint16_t            *conftab;          /*!< Reserved for future use, Configuration table (key,value pairs) */
} L2CA_AUTO_TP_CONNECT_IND_T;

/*! \brief Automatic connection confirm

    \c L2CA_AUTO_CONNECT_CFM \c is used both on the initiator and responder side of a 
    connection to indicate the status of a connection. On the initiator side it 
    is used during connection-setup to inform the application of progress
    within the connection setup procedure; initiated ( \c INITIATING \c ), is running 
    ( \c PENDING \c ) or has been completed ( \c SUCCESS \c ). 
    
    On the responder side the primitive is used to indicate that the connection 
    has been established (or has failed).

    The resulting "config" structure contains the values that the peer
    has agreed to use, and that the upper layer may want to take into
    account. Some values are given by the local device (known as the
    request path parameters), and some are given by the peer itself
    (known as the respond path parameters). This directly correlates
    to the in/out direction of the key-value pairs given in the
    "conftab" in the request/respond auto-connect signals.
    Request path parameters are: flush, qos, fcs, flowspec.
    Respond path parameters are: mtu, fec, extended windows.

    See MSC of \ref L2CA_AUTO_CONNECT_REQ_T

    \see L2CA_AUTO_CONNECT_REQ_T
    \see l2ca_conn_result_t
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_AUTO_CONNECT_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Registration context */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_local;        /*!< The PSM of the local device */
    l2ca_conn_result_t  result;           /*!< Result code - uses L2CA_CONNECT range */
    L2CA_CONFIG_T       config;           /*!< Configuration options to use - see note above*/
    uint32_t            ext_feats;        /*!< Peer extended features */
} L2CA_AUTO_CONNECT_CFM_T;


/*! \brief Automatic connection confirm on specific transport 

    \c L2CA_AUTO_TP_CONNECT_CFM \c is used both on the initiator and responder 
    side of a connection to indicate the status of a connection on a specific
    transport. config_ext is extended configuration values other than 
    L2CA_TP_CONFIG_T, it is reserved for future use. It is advised to free 
    config_ext to make user code future proof (to avoid memory leak).

    \see L2CA_AUTO_CONNECT_CFM_T

    BR/EDR connection : 
    L2CA_AUTO_TP_CONNECT_CFM is an extended version of L2CA_AUTO_CONNECT_CFM 
    to accomodate LE connection oriented channels. Except for tp_addrt 
    information, usage of this API for BR/EDR connection would remain the same
    as L2CA_AUTO_CONNECT_CFM.

    LE connection :
    Application can refer rx credits, local and remote MTU values in 
    L2CA_TP_CONFIG_T.
*/

typedef struct
{
    l2cap_prim_t          type;              /*!< Always L2CA_AUTO_TP_CONNECT_CFM */
    phandle_t             phandle;           /*!< Destination phandle */
    l2ca_cid_t            cid;               /*!< Channel identifier */
    context_t             reg_ctx;           /*!< Registration context */
    context_t             con_ctx;           /*!< Opaque context number from connect */
    TP_BD_ADDR_T          tp_addrt;          /*!< Address of remote device including type and transport */
    psm_t                 psm_local;         /*!< The PSM of the local device */
    l2ca_conn_result_t    result;            /*!< Result code - uses L2CA_CONNECT range */
    L2CA_TP_CONFIG_T      config;            /*!< Configuration options to use - see note above*/
    uint32_t              ext_feats;         /*!< Peer extended features */
    l2ca_conflags_t       flags;             /*!< Special connection flags */
    uint16_t              config_ext_length; /*!< Reserved for future use */
    uint16_t             *config_ext;        /*!< Reserved for future use, Configuration table (key,value pairs)*/
} L2CA_AUTO_TP_CONNECT_CFM_T;

/*! \brief L2CAP fixed channel map request.

    In order to be able to transmit data on a fixed channel, a dynamic
    CID must be allocated for the data path. This signal will request
    such a dynamic CID to be allocated for an identified fixed channel
    for a particular Bluetooth address. The assigned CID will be
    returned in the CFM signal.

    Note that the application is responsible for determining whether
    the remote L2CAP entity and application is capable of transferring
    data on a fixed channel. Also notice that the L2CAP configuration
    used on the particular fixed channel is determined in
    \ref L2CA_REGISTER_FIXED_CID_REQ_T. Once the fixed channel has been mapped
    to a dynamic CID, the normal \ref L2CA_DATAWRITE_REQ_T and
    \ref L2CA_DATAREAD_IND_T can be used. The channel can, however, only be
    disconnected indirectly when the ACL is closed!

    \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        "Application"->"L2CAP"              [ label="L2CA_MAP_FIXED_CID_REQ",     URL="\ref L2CA_MAP_FIXED_CID_REQ_T" ];
        "L2CAP"->"Application"              [ label="L2CA_MAP_FIXED_CID_CFM",     URL="\ref L2CA_MAP_FIXED_CID_CFM_T" ];
        Application->L2CAP                  [ label="L2CA_DATAWRITE_REQ",         URL="\ref L2CA_DATAWRITE_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_Data" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_DATAREAD_IND",          URL="\ref L2CA_DATAREAD_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_DATAREAD_RSP",          URL="\ref L2CA_DATAREAD_RSP_T", ID="F&EC only" ];
        "Peer L2CAP"->L2CAP                 [ label="L2CAP_DataAck",                                              ID="F&EC only" ];
        "L2CAP"->"Application"              [ label="L2CA_DATAWRITE_CFM",         URL="\ref L2CA_DATAWRITE_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MAP_FIXED_CID_REQ */
    TYPED_BD_ADDR_T     addrt;            /*!< Bluetooth address of remote device */
    l2ca_cid_t          fixed_cid;        /*!< Fixed channel CID */
    psm_t               cl_local_psm;     /*!< Connectionless PSM (only for fixed_cid == 2) */
    psm_t               cl_remote_psm;    /*!< Connectionless PSM (only for fixed_cid == 2) */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    l2ca_conflags_t     flags;            /*!< Special connection flags  L2CA_CONNECTION_* */
} L2CA_MAP_FIXED_CID_REQ_T;

/*! \brief L2CAP fixed channel map indication

    If an application has registered interest in a fixed channel using
    \ref L2CA_REGISTER_FIXED_CID_REQ_T and a peer sends data on that
    particular fixed channel, L2CAP will automatically bring up the
    necessary internal structures and assign a dynamic CID for that
    channel. Before the data sent on the channel is passed to the
    application, the \ref L2CA_MAP_FIXED_CID_IND_T will be sent to the
    application so it can be identified from which device the data was
    sent, and to what specific fixed channel number.

    Note: The connection context number (con_ctx) will not be valid
    before the application has sent a \ref L2CA_MAP_FIXED_CID_RSP_T
    and L2CAP can not guarantee that crossovers will not happen.

    \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        "Peer Application"->"Peer L2CAP"    [ label="L2CA_DATAWRITE_REQ",         URL="\ref L2CA_DATAWRITE_REQ_T" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_Data" ];
        "L2CAP"->"Application"              [ label="L2CA_MAP_FIXED_CID_IND",     URL="\ref L2CA_MAP_FIXED_CID_IND_T" ];
        "L2CAP"->"Application"              [ label="L2CA_DATAREAD_IND",          URL="\ref L2CA_DATAREAD_IND_T" ];
        "Application"->"L2CAP"              [ label="L2CA_DATAREAD_RSP",          URL="\ref L2CA_DATAREAD_RSP_T" ];
        "Application"->"L2CAP"              [ label="L2CA_MAP_FIXED_CID_RSP",     URL="\ref L2CA_MAP_FIXED_CID_RSP_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONNECT_FIXED_CID_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local, dynamic channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration. Only valid for fixed_cid == 2 */
    TYPED_BD_ADDR_T     addrt;            /*!< Bluetooth address of remote device */
    l2ca_cid_t          fixed_cid;        /*!< Fixed channel CID */
    psm_t               cl_local_psm;     /*!< Connectionless PSM (only for fixed_cid == 2) */
    l2ca_conflags_t     flags;            /*!< Special connection flags */
} L2CA_MAP_FIXED_CID_IND_T;

/*! \brief L2CAP fixed channel map confirmation

    Confirmation from L2CAP to relay what dynamic CID was allocated
    for a fixed channel for a particular Bluetooth address. Please see
    \ref L2CA_MAP_FIXED_CID_REQ_T for further information.

*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MAP_FIXED_CID_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local, dynamic channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration. Only valid for fixed_cid == 2 */
    TYPED_BD_ADDR_T     addrt;            /*!< Bluetooth address of remote device */
    l2ca_cid_t          fixed_cid;        /*!< Fixed channel CID */
    psm_t               cl_local_psm;     /*!< Connectionless PSM (only for fixed_cid == 2) */
    context_t           con_ctx;          /*!< Opaque context number from map request */
    l2ca_misc_result_t  result;           /*!< Result code - uses L2CA_MISC range */
    l2ca_conflags_t     flags;            /*!< Special connection flags */
} L2CA_MAP_FIXED_CID_CFM_T;

/*! \brief L2CAP fixed channel map response

    Reply from the application to L2CAP to assign a connection context
    number to a mapped fixed channel.

*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MAP_FIXED_CID_CFM */
    l2ca_cid_t          cid;              /*!< Local, dynamic channel identifier */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    psm_t               ucd_remote_psm;   /*!< Connectionless PSM */
    l2ca_conflags_t     flags;            /*!< Special connection flags */
} L2CA_MAP_FIXED_CID_RSP_T;

/*! \brief L2CAP fixed channel unmap

    If a fixed channel was asked to keep the ACL open, unmapping it
    will allow the ACL to go away if noone else is using it. Note that
    this will not necessarily close the ACL nor the fixed channel.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_UNMAP_FIXED_CID_REQ */
    l2ca_cid_t          cid;              /*!< Local, dynamic channel identifier */
} L2CA_UNMAP_FIXED_CID_REQ_T;

/*! \brief L2CAP fixed channel unmapped

    The ACL of a fixed channel has been closed. The reason code may be
    set in case a fatal error was detected.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DISCONNECT_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local (dynamic) channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration. Only valid for connectionless channel (fixed_cid == 2) */
    context_t           con_ctx;          /*!< Opaque context number from map */
    l2ca_disc_result_t  reason;           /*!< Reason code - uses L2CA_DISCONNECT range */
} L2CA_UNMAP_FIXED_CID_IND_T;

/*! \brief Move a L2CAP channel request, possibly over an AMP

    The \c L2CA_MOVE_CHANNEL_REQ \c primitive is used to initiate to move a channel over an AMP.
    It can also be used to move a channel back to BR/EDR.

    \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        --- [label=" Open on BR/EDR" ];

        "Application"->"L2CAP"              [ label="L2CA_MOVE_CHANNEL_REQ",    URL="\ref L2CA_MOVE_CHANNEL_REQ_T" ];
        "L2CAP"->"Application"              [ label="L2CA_MOVE_CHANNEL_CFM setting-up",    URL="\ref L2CA_MOVE_CHANNEL_CFM_T"];
        "L2CAP"->"Peer L2CAP"               [ label="L2CAP_MoveReq" ];

        "Peer L2CAP"->"Peer Application"    [ label="L2CA_MOVE_CHANNEL_IND",    URL="\ref L2CA_MOVE_CHANNEL_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_MOVE_CHANNEL_RSP",    URL="\ref L2CA_MOVE_CHANNEL_RSP_T" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_MoveRsp pending" ];
        "L2CAP"->"Application"              [ label="L2CA_MOVE_CHANNEL_CFM pending",    URL="\ref L2CA_MOVE_CHANNEL_CFM_T"];

        --- [label=" Logical Channel over AMP setup" ];

        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_MoveRsp success" ];
        "L2CAP"->"Peer L2CAP"               [ label="L2CAP_MoveConfirmReq" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_MoveConfirmRsp" ];

        "L2CAP"->"Application"              [ label="L2CA_MOVE_CHANNEL_CFM",    URL="\ref L2CA_MOVE_CHANNEL_CFM_T" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_MOVE_CHANNEL_CFM",    URL="\ref L2CA_MOVE_CHANNEL_CFM_T" ];
        
        --- [label=" Open on AMP" ];

        
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MOVE_CHANNEL_REQ */
    l2ca_cid_t          cid;              /*!< CID of channel to move */
    l2ca_controller_t   remote_control;   /*!< Remote controller ID */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
} L2CA_MOVE_CHANNEL_REQ_T;

/*! \brief Move a L2CAP channel confirm, possibly over an AMP

    Move channel confirm is used to inform upper layers that a move
    channel has completed. This primitive is only used on the
    initiating side of the original move request.

    See MSC of \ref L2CA_MOVE_CHANNEL_REQ_T
    
    \see L2CA_MOVE_CHANNEL_IND_T
    \see L2CA_MOVE_CHANNEL_RSP_T
    \see L2CA_MOVE_CHANNEL_CFM_T    
    \see L2CA_MOVE_CHANNEL_CMP_IND_T    
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MOVE_CHANNEL_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< CID of channel to move */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect/create channel */
    l2ca_move_result_t  result;           /*!< Result code - uses L2CA_MOVE range */
    l2ca_controller_t   local_control;    /*!< Local controller ID actually in use */
    l2ca_controller_t   remote_control;   /*!< Remote controller ID acutally in use */
} L2CA_MOVE_CHANNEL_CFM_T;

/*! \brief Move a L2CAP channel complete indication, possibly over an AMP

    Move channel complete indication is used to inform upper layers
    that a move channel has completed. The primitive is similar to the
    move channel confirm, the only difference being that this one is
    used on the responding side of the original move-request.    

    See MSC of \ref L2CA_MOVE_CHANNEL_REQ_T

    \see L2CA_MOVE_CHANNEL_REQ_T
    \see L2CA_MOVE_CHANNEL_CFM_T
    \see L2CA_MOVE_CHANNEL_IND_T
    \see L2CA_MOVE_CHANNEL_RSP_T   
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MOVE_CHANNEL_CMP_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< CID of channel to move */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect/create channel */
    l2ca_move_result_t  result;           /*!< Result code - uses L2CA_MOVE range */
    l2ca_controller_t   local_control;    /*!< Local controller ID actually in use */
} L2CA_MOVE_CHANNEL_CMP_IND_T;

/*! \brief Move a L2CAP channel indication, possibly over an AMP

    \c L2CA_MOVE_CHANNEL_IND \c is sent to the application to indicate that a peer wants to 
    move a channel over to a particular AMP indicated in \c local_control \c .

    See MSC of \ref L2CA_MOVE_CHANNEL_REQ_T

    \see L2CA_MOVE_CHANNEL_REQ_T
    \see L2CA_MOVE_CHANNEL_CFM_T
    \see L2CA_MOVE_CHANNEL_RSP_T
    \see L2CA_MOVE_CHANNEL_CMP_IND_T    

*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MOVE_CHANNEL_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< CID of channel to move */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect/create channel */
    l2ca_identifier_t   identifier;       /*!< Used to identify the move signal */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
} L2CA_MOVE_CHANNEL_IND_T;

/*! \brief Move a L2CAP channel response

    L2CA_MOVE_CHANNEL_RSP sent after the application has received a L2CA_MOVE_CHANNEL_IND.
    
    The Move L2CAP channel response must always be sent when a L2CA_MOVE_CHANNEL_IND has 
    been received by the application. The response must contain the same values 
    for identifier and cid as the indication.

    The application shall indicate using \c response \c if it has allowed or refused this operation.

    See MSC of \ref L2CA_MOVE_CHANNEL_REQ_T

    \see L2CA_MOVE_CHANNEL_REQ_T
    \see L2CA_MOVE_CHANNEL_CFM_T
    \see L2CA_MOVE_CHANNEL_IND_T
    \see L2CA_MOVE_CHANNEL_CMP_IND_T

*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MOVE_CHANNEL_RSP */
    l2ca_cid_t          cid;              /*!< CID of channel to move */
    l2ca_identifier_t   identifier;       /*!< Used to identify the move signal */
    l2ca_move_result_t  response;         /*!< Response code - uses L2CA_MOVE range (only spec values allowed) */
} L2CA_MOVE_CHANNEL_RSP_T;

/*! \brief L2CAP AMP link loss indication

    A link loss over an AMP enabled L2CAP channel has been detected.
    The AMP channel traffic has dropped and traffic has been
    halted. The application/peer is expected to either close the
    channnel or to start a move procedure to another baseband.

    The reason element is an HCI error code for the link loss. See hci.h.

    Note that an AMP link loss doesn't necessarily mean that the
    channel is dead for good - if the move procedure succeeds, traffic
    should be able to resume on another controller...
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_AMP_LINK_LOSS_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connection */
    hci_error_t         reason;           /*!< Details HCI reason for link loss */
} L2CA_AMP_LINK_LOSS_IND_T;

/*! \brief Send L2CAP data packet request

    Request to send L2CAP packet.

    Currently only one packet can be sent per
    channel at a time, higher layers may only transmit another packet when they
    receive a \ref L2CA_DATAWRITE_CFM_T primitive.

    The 'con_ctx' field in the primitive is automatically returned in the
    corresponding \ref L2CA_DATAWRITE_CFM_T primitive, it can be used by the higher
    layer to store a context for the L2CAP data packet.

   \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

    --- [ label="Connection Established" ];
        Application->L2CAP                  [ label="L2CA_DATAWRITE_REQ",   URL="\ref L2CA_DATAWRITE_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_Data" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_DATAREAD_IND",    URL="\ref L2CA_DATAREAD_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_DATAREAD_RSP",    URL="\ref L2CA_DATAREAD_RSP_T",     ID="F&EC only" ];
        "Peer L2CAP"->L2CAP                 [ label="L2CAP_DataAck",                                            ID="F&EC only" ];
        "L2CAP"->"Application"              [ label="L2CA_DATAWRITE_CFM",   URL="\ref L2CA_DATAWRITE_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DATAWRITE_REQ */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           req_ctx;          /*!< User context to be returned in L2CA_DATAWRITE_CFM */
    uint16_t            length;           /*!< Length of data (must be 0 to indicate MBLK) */
    MBLK_T             *data;             /*!< Pointer to data/MBLK */
    uint16_t            packets_ack;      /*!< Used to acknowledge L2CAP PDUs in earlier DATAREAD_INDs */
} L2CA_DATAWRITE_REQ_T;

/*! \brief Send L2CAP data packet confirmation

    Confirmation sent when data has been sent.

    The timing of the confirmation is dependant on the channel mode, for
    basic mode the confirmation is generated locally as soon as the L2CAP packet
    has been transmitted, for Retranmission and Flow Control modes the confirmation
    is sent on receiving an acknowledgement from the peer.

    See \ref L2CA_DATAWRITE_REQ_T for more details.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DATAWRITE_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration. Not valid for fixed channels where fixed_cid != 2 */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    context_t           req_ctx;          /*!< User context specified in L2CA_DATAWRITE_REQ */
    uint16_t            length;           /*!< Length of data sent */
    l2ca_data_result_t  result;           /*!< Result code - uses L2CA_DATA range */
} L2CA_DATAWRITE_CFM_T;

/*! \brief Tell the peer that we are not ready to receive more data

    In retransmission mode it's possible to inform the remote peer
    that we are currently not able to receive any more data even if
    the rx/tx window is not yet full. This primitive can toggle the
    status of this field, but data may still get through due to
    crossing signals.

    This signal is only used in retransmission or enhanced
    retransmission mode.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_BUSY_REQ */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    bool_t              busy;             /*!< New status of busy (RNR) flag */
} L2CA_BUSY_REQ_T;

/*! \brief Peer has changed it's busy status

    The remote peer is busy/ready, so data transmissions should either
    be halted or restarted.

    This signal is only used in retransmission or enhanced
    retransmission mode.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_BUSY_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    bool_t              busy;             /*!< New status of busy (RNR) flag */
} L2CA_BUSY_IND_T;

#define L2CA_MAX_MULTICAST_CIDS (7)       /*!< Number of CIDs in array */
/*! \brief Send L2CAP data packet to multiple L2CAP channels

    Send a L2CAP packet to multiple L2CAP channels, this primitive is
    almost immediately converted internally into multiple
    L2CA_DATAWRITE_REQs and hence the client will receive
    L2CA_DATAWRITE_CFMs for each channel as normal and should respect
    flow control has if it had sent the L2CA_DATAWRITE_REQs.

    This is typically used when streaming media data to multiple
    devices and the transport between the client and the L2CAP layer
    is limited, perhaps by a slow UART.  By using multicast, the
    client only has to send the data once over the UART.

    Note that the 'con_ctx' in the L2CA_DATAWRITE_CFMs will be zero
    for multicast writes.

   \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

    --- [ label="Connection Established" ];
        Application->L2CAP                  [ label="L2CA_MULTICAST_REQ",   URL="\ref L2CA_MULTICAST_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_Data" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_DATAREAD_IND",    URL="\ref L2CA_DATAREAD_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_DATAREAD_RSP",    URL="\ref L2CA_DATAREAD_RSP_T",     ID="F&EC only" ];
        "Peer L2CAP"->L2CAP                 [ label="L2CAP_DataAck",                                            ID="F&EC only" ];
        "L2CAP"->"Application"              [ label="L2CA_DATAWRITE_CFM",   URL="\ref L2CA_DATAWRITE_CFM_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_Data" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_DATAREAD_RSP",    URL="\ref L2CA_DATAREAD_RSP_T",     ID="F&EC only" ];
        "Peer L2CAP"->L2CAP                 [ label="L2CAP_DataAck",                                            ID="F&EC only" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_DataAck" ];
        "L2CAP"->"Application"              [ label="L2CA_DATAWRITE_CFM",   URL="\ref L2CA_DATAWRITE_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_MULTICAST_REQ */
    l2ca_cid_t          cids[L2CA_MAX_MULTICAST_CIDS]; /*!< List of CIDs */
    uint16_t            length;           /*!< Length of data (must be 0 to indicate MBLK) */
    MBLK_T             *data;             /*!< Pointer to data/MBLK */
} L2CA_MULTICAST_REQ_T;

/*! \brief L2CAP data indication

    NB, if result is not READ_SUCCESS, this indicates that invalid
    packets have been received. Under these circumstances L2CAP will
    discard the invalid data and the length and data parameters will
    be zero/NULL.

    In Enhanced Retransmission Mode, received packets must be 
    acknowledged by the upper layers for purposes of flow-control.
    The acknowledge field gives the amount of acknowledgement required
    for the accompanied data. In Enhanced Retransmission Mode, this
    will correspond to the number of complete PDU segments that make
    up this SDU (or parital SDU).
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DATAREAD_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration. Not valid for fixed channels, except for connectionless channel */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    uint16_t            length;           /*!< Length of data (must be 0 to indicate MBLK) */
    MBLK_T             *data;             /*!< Pointer to data/MBLK */
    l2ca_data_result_t  result;           /*!< Result code - uses L2CA_DATA range */
    uint16_t            packets;          /*!< The number of L2CAP PDUs (data segments) that are attached or have been lost (depending on result). */
} L2CA_DATAREAD_IND_T;

/*! \brief L2CAP data read response

    Note:
    - If L2CAP 1.2 retransmission or flow control is used, it's
      necessary for the upper layer to respond to data reads to keep the
      flow control going. Note that the upper layers *must* read all the
      data, and that this read can only be successfull (we can't do
      anything about failures anyway at this point)
    - For Credit Based Flow Control mode on LE L2CAP COC to function
      appropriately, application shall provide "packets" value in
      L2CA_DATAREAD_RSP_T same as that notified via "packets" value in
      preceeding L2CA_DATAREAD_IND_T.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DATAREAD_RSP */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    uint16_t            packets;          /*!< The number of L2CAP PDUs (data segments) that are being acknowledged */
} L2CA_DATAREAD_RSP_T;

/*! \brief Disconnect L2CAP by CID

    Perform a normal L2CAP shutdown given the unique local CID.
    Any API call with an invalid CID would return DM_BAD_MESSAGE_IND message
    response.

    Note:
    In disconnection crossover scenario, user may receive L2CA_DISCONNECT_IND
    from Bluecore device(because of remote initiated disconnection) even after
    sending L2CA_DISCONNECT_REQ for the same CID. In such crossover cases user
    is advised to send the L2CA_DISCONNECT_RSP to Bluecore device, once the
    response is received, L2CA_DISCONNECT_CFM will be sent from Bluecore. 
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DISCONNECT_REQ */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           con_ctx;          /*!< Opaque context number from connect */
} L2CA_DISCONNECT_REQ_T;

/*! \brief L2CAP raw data mode

    Enable or disable L2CAP raw data mode support for given CID and/or
    related ACL. You should never use this feature unless you're
    debugging L2CAP itself or doing low-level testing! When turned on, you
    will receive data in \ref L2CA_RAW_DATA_IND_T
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_RAW_MODE_REQ */
    l2ca_cid_t          cid;              /*!< Change settings for this CID */
    l2ca_raw_t          raw_mode;         /*!< See defines. */
    phandle_t           phandle;          /*!< Handle to receive ACL data */
} L2CA_RAW_MODE_REQ_T;

/*! \brief L2CAP raw data request

    Send raw L2CAP data to a particular CID/ACL. You should never use
    this feature unless you're debugging L2CAP itself or doing
    low-level testing!

    Raw data writes will generate normal \ref L2CA_DATAWRITE_CFM_T primitives.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_RAW_DATA_REQ */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    uint16_t            raw_length;       /*!< Length in header, 0 to automatically fill in */
    l2ca_timeout_t      flush_to;         /*!< Flush timeout for this packet */
    uint16_t            length;           /*!< Length of data, 0 for MBLKs */
    MBLK_T             *data;             /*!< Pointer to data/MBLK */
} L2CA_RAW_DATA_REQ_T;

/*! \brief L2CAP raw data indication

    Receive raw L2CAP data. You should never use this feature unless
    you're debugging L2CAP itself or doing low-level testing!  Before
    you can receive raw data, this must be turned on using \ref
    L2CA_RAW_MODE_REQ_T. Depending on the raw mode chosen, the data
    provided here might be just a copy for information, or L2CAP might
    be expecting the application to deal with any required responses.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_RAW_DATA_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local (or fixed) channel ID */
    TYPED_BD_ADDR_T     addrt;            /*!< Remote Bluetooth address */
    l2ca_data_result_t  result;
    l2ca_raw_packet_t   packet_type;      /*!< L2CA_RX, L2CA_TX_START, L2CA_TX_CONT */
    uint16_t            length;           /*!< Length of data */
    MBLK_T             *data;             /*!< Pointer to data/MBLK, without length,cid */
} L2CA_RAW_DATA_IND_T;

/*! \brief L2CAP debug mode for dropping packets

    Setup L2CAP to drop a fraction of rx/tx packets, useful for
    testing retransmission modes etc.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DEBUG_DATA_IND */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    uint16_t            rx_interval;      /*!< Interval to drop incoming data */
    uint16_t            rx_number;        /*!< Number of incoming packets to drop */
    uint16_t            tx_interval;      /*!< Interval to drop outgoing data */
    uint16_t            tx_number;        /*!< Number of outgoing packets to drop */
} L2CA_DEBUG_DROP_REQ_T;

/*! \brief L2CAP disconnect response

    Response to request to disconnect L2CAP channel. Disconnection is
    non-negotiable
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DISCONNECT_RSP */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    l2ca_identifier_t   identifier;       /*!< Used to identify the disconnect signal */
} L2CA_DISCONNECT_RSP_T;

/*! \brief L2CAP disconnect indication

    Indication of request to disconnect L2CAP channel.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DISCONNECT_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_identifier_t   identifier;       /*!< Used to identify the disconnect signal */
    l2ca_disc_result_t  reason;           /*!< Reason code - uses L2CA_DISCONNECT range */
} L2CA_DISCONNECT_IND_T;

/*! \brief L2CAP disconnect confirm

    Confirmation of request to disconnect L2CAP channel.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DISCONNECT_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_disc_result_t  result;           /*!< Successful or timed out */
} L2CA_DISCONNECT_CFM_T;

/*! \brief L2CAP configuration timeout

    Indicates that the RTX or ERTX timer has expired. This will occur
    an implementation dependant number of times.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_TIMEOUT_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_identifier_t   identifier;       /*!< Identifier of the timed out signal */
} L2CA_TIMEOUT_IND_T;


/*! \brief L2CAP signal MTU violation

    Indicates that an L2CAP signalling MTU violation has occurred. If
    the application receives this signal, the last signal (possibly a
    L2CA_CONFIG_REQ) should be resent such that L2CAP can retry with a
    lower MTU.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_TIMEOUT_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_identifier_t   identifier;       /*!< Identifier of the violating signal */
    l2ca_mtu_t          signal_mtu;       /*!< The maximum signalling MTU */
} L2CA_MTU_VIOLATION_IND_T;


/*! \brief L2CAP signal unknown

    This signal indicates that the last L2CAP signal was unknown. This
    may happen if the application tried to send an AMP signal to a
    legacy L2CAP implementation.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_TIMEOUT_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Local channel ID */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_identifier_t   identifier;       /*!< Identifier of the violating signal */
} L2CA_UNKNOWN_SIGNAL_IND_T;


/*! \brief L2CAP ping request

    Ping request to remote device (send echo request signal)
    Need the phandle otherwise do not know where to send
    the confirmation to
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_PING_REQ */
    phandle_t           phandle;          /*!< phandle for requesting protocol */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth device address */
    uint16_t            length;           /*!< number of bytes pointed to by data */
    uint8_t            *data;             /*!< data to be transmitted (and returned in PING_CFM command) */
    context_t           req_ctx;          /*!< User context to be returned in L2CA_PING_CFM */
    l2ca_conflags_t     flags;            /*!< Reserved */
} L2CA_PING_REQ_T;

/*! \brief L2CAP ping reply

    Ping confirm to previous request
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_PING_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth device address */
    uint16_t            length;           /*!< Number of bytes pointed to by data */
    uint8_t            *data;             /*!< Returned data (should match the PING_REQ data */
    l2ca_misc_result_t  result;           /*!< Result code - uses L2CA_MISC range */
    context_t           req_ctx;          /*!< Returned req_ctx from the request signal*/
    l2ca_conflags_t     flags;            /*!< Reserved */
} L2CA_PING_CFM_T;

/*! \brief Get information request

    Request information about L2CAP from remote device
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_GETINFO_REQ */
    phandle_t           phandle;          /*!< phandle for requesting protocol */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth device address */
    uint16_t            info_type;        /*!< Type of information requested */
    context_t           req_ctx;          /*!< User context to be returned in L2CA_GETINFO_CFM */
    l2ca_conflags_t     flags;            /*!< Reserved */
} L2CA_GETINFO_REQ_T;


/*! \brief Get information confirm

    Confirmation to information request
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_GETINFO_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth device address */
    uint16_t            info_type;        /*!< Type of information */
    uint16_t            length;           /*!< length of data in info_data */
    uint8_t            *info_data;        /*!< Data payload - little endian format (as in spec) */
    l2ca_info_result_t  result;           /*!< Success, failure, etc */
    context_t           req_ctx;          /*!< Returned req_ctx from the request signal*/
    l2ca_conflags_t     flags;            /*!< Reserved */
} L2CA_GETINFO_CFM_T;

/*! \brief Route Data Primitives Request

    Route data primitives to another task/queue.  This primitive allows data
    primitives to be sent to an alternative task/queue and all other signalling
    primitives to be sent to the orignal task/queue.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_ROUTE_DATA_REQ */
    phandle_t           phandle;          /*!< Task/queue handle to send data primtives to */
    l2ca_cid_t          cid;              /*!< Channel ID to re-route */
} L2CA_ROUTE_DATA_REQ_T;

/*! \brief Data route confirm

    Generated in response to L2CA_ROUTE_DATA_REQ. If result is
    ROUTE_SUCCESS, the data is now being routed.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_ROUTE_DATA_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< CID of channel being routed */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_mtu_t          out_mtu;          /*!< Outgoing MTU for channel */
    l2ca_misc_result_t  result;           /*!< Result code - uses L2CA_MISC range */
} L2CA_ROUTE_DATA_CFM_T;

/*! \brief Disable data routing

    Request to return the routing of data primities to the phandle
    that established the L2CAP connection.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_UNROUTE_DATA_REQ */
    phandle_t           phandle;          /*!< Requesting application */
    l2ca_cid_t          cid;              /*!< CID of channel to unroute */
} L2CA_UNROUTE_DATA_REQ_T;

/*! \brief Disable data routing confirm

    Generated in response to L2CA_UNROUTE_DATA_REQ.  If result is
    ROUTE_SUCCESS, the data is now being routed to the phandle used to
    create the connection.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_UNROUTE_DATA_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< CID of unrouted channel */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_misc_result_t  result;           /*!< Result code - uses L2CA_MISC range */
} L2CA_UNROUTE_DATA_CFM_T;

/*! \brief Unroute data indication

    Sent to the phandle currently receiving data for a CID when that
    CID is destroyed. This is informational because the
    L2CA_DISCONNECT_nnn primitives will have been posted to the
    original phandle.
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_UNROUTE_DATA_IND */
    phandle_t           phandle;          /*!< Requesting application */
    l2ca_cid_t          cid;              /*!< CID of channel */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
} L2CA_UNROUTE_DATA_IND_T;

/*! \brief Abort queued TX data on channel.

    This functionality is not enabled for L2CAP on chip. VM applications
    that send this primitive will receive L2CA_DATAWRITE_ABORT_CFM
    immediately, but no data primitives will have been aborted.

    This will cause all unsent SDUs queued for transmission to be discarded
    on this channel. It will not affect any SDU that has already been wholely
    or partially transmitted. L2CAP will send an L2CA_DATAWRITE_CFM
    (result L2CA_DATA_LOCAL_ABORTED) for each SDU discarded and will send
    L2CA_DATAWRITE_ABORT_CFM afterwards. */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DATAWRITE_ABORT_REQ */
    l2ca_cid_t          cid;              /*!< CID of channel */
} L2CA_DATAWRITE_ABORT_REQ_T;

/*! \brief Confirmation of L2CA_DATAWRITE_ABORT_REQ */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_DATAWRITE_ABORT_CFM */
    phandle_t           phandle;          /*!< Requesting application */
    l2ca_cid_t          cid;              /*!< CID of channel */
    context_t           reg_ctx;          /*!< Registration context */
    context_t           con_ctx;          /*!< Opaque context number from connect */
} L2CA_DATAWRITE_ABORT_CFM_T;

/*! \brief Common upstream primitive

     Fields common to all "upstream" L2CAP primitives so we can use a
     union type to quickly decode common members.
*/
typedef struct
{
    l2cap_prim_t         type;            /*!< Unique primtive identifier */
    phandle_t            phandle;         /*!< Destination phandle */
} L2CA_UPSTREAM_COMMON_T ;





/* Internal/Obsolete primitives.

   These primitives must not be sent by applications.
   They are for L2CAP internal use only. They are unsupported and may be
   removed in future versions. Use L2CA_AUTOCONNECT_ instead.
*/


/*! \brief L2CAP connect request

    These are primitives that are used by L2CAP internally. They must
    not be sent by the application. Use L2CA_AUTO_CONNECT_ instead.

    Connect request to remote device
    Note the addition of a psm_local - otherwise there is no way of
    knowing where to send the confirm to.

    \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        Application->L2CAP                  [ label="L2CA_CONNECT_REQ",     URL="\ref L2CA_CONNECT_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_ConnectReq" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_CONNECT_IND",     URL="\ref L2CA_CONNECT_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_CONNECT_RSP",     URL="\ref L2CA_CONNECT_RSP_T" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConnectRspPnd" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConnectRsp" ];
        L2CAP->Application                  [ label="L2CA_CONNECT_CFM",     URL="\ref L2CA_CONNECT_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONNECT_REQ */
    psm_t               psm_local;        /*!< PSM of local requesting protocol */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_remote;       /*!< The PSM on the remote device */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    l2ca_conflags_t     flags;            /*!< Special connection flags */
    DM_SM_SERVICE_T     service;          /*!< Security Manager substitute service */
} L2CA_CONNECT_REQ_T;

/*! \brief L2CAP connect response

    These are primitives that are used by L2CAP internally. They must
    not be sent by the application. Use L2CA_AUTO_CONNECT_ instead.

    Response to an a incoming L2CAP connection indication. 
    The identifier and cid fields should be copied from the \ref L2CA_CONNECT_IND_T primitive.

   \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        "Peer Application"->"Peer L2CAP"    [ label="L2CA_CONNECT_REQ",     URL="\ref L2CA_CONNECT_REQ_T" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConnectReq" ];
        "L2CAP"->"Application"              [ label="L2CA_CONNECT_IND",     URL="\ref L2CA_CONNECT_IND_T" ];
        "Application"->"L2CAP"              [ label="L2CA_CONNECT_RSP",     URL="\ref L2CA_CONNECT_RSP_T" ];
        "L2CAP"->"Peer L2CAP"               [ label="L2CAP_ConnectRspPnd" ];
        "L2CAP"->"Peer L2CAP"               [ label="L2CAP_ConnectRsp" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_CONNECT_CFM",     URL="\ref L2CA_CONNECT_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONNECT_RSP */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    l2ca_identifier_t   identifier;       /*!< Used to identify the connect signal */
    l2ca_conn_result_t  response;         /*!< Result code - uses L2CA_CONNECT range */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
} L2CA_CONNECT_RSP_T;

/*! \brief L2CAP connect indication

    These are primitives that are used by L2CAP internally. They must
    not be sent by the application. Use L2CA_AUTO_CONNECT_ instead.

    Indication of an incoming L2CAP connection.

   \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        "Peer Application"->"Peer L2CAP"    [ label="L2CA_CONNECT_REQ",     URL="\ref L2CA_CONNECT_REQ_T" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConnectReq" ];
        "L2CAP"->"Application"              [ label="L2CA_CONNECT_IND",     URL="\ref L2CA_CONNECT_IND_T" ];
        "Application"->"L2CAP"              [ label="L2CA_CONNECT_RSP",     URL="\ref L2CA_CONNECT_RSP_T" ];
        "L2CAP"->"Peer L2CAP"               [ label="L2CAP_ConnectRspPnd" ];
        "L2CAP"->"Peer L2CAP"               [ label="L2CAP_ConnectRsp" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_CONNECT_CFM",     URL="\ref L2CA_CONNECT_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONNECT_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    l2ca_identifier_t   identifier;       /*!< Used to identify the connect signal */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_local;        /*!< PSM of the local device */
    l2ca_conflags_t     flags;            /*!< Special connection flags */
} L2CA_CONNECT_IND_T;

/*! \brief L2CAP connect confirmation

    These are primitives that are used by L2CAP internally. They must
    not be sent by the application. Use L2CA_AUTO_CONNECT_ instead.

   \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

        Application->L2CAP                  [ label="L2CA_CONNECT_REQ",     URL="\ref L2CA_CONNECT_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_ConnectReq" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CA_CONNECT_IND",     URL="\ref L2CA_CONNECT_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CA_CONNECT_RSP",     URL="\ref L2CA_CONNECT_RSP_T" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConnectRspPnd" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConnectRsp" ];
        L2CAP->Application                  [ label="L2CA_CONNECT_CFM",     URL="\ref L2CA_CONNECT_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONNECT_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_local;        /*!< The PSM of the local device */
    l2ca_conn_result_t  result;           /*!< Result code - uses L2CA_CONNECT range */
    l2ca_conflags_t     flags;            /*!< Special connection flags */
} L2CA_CONNECT_CFM_T;

/*! \brief L2CAP configuration request

    These are primitives that are used by L2CAP internally. They must
    not be sent by the application. Use L2CA_AUTO_CONNECT_ instead.

    Configuration request for a particular channel.  Bitwise AND the
    options field bit flags to identify which of the following three
    option fields are used.  The hints field specify which of the
    present options that are only hints.

    Flush timeouts are now limited to the values valid at the HCI
    layer, thus the L2CAP specified value may differ from that echoed
    back in the config confirmation, as the confirmation will show the
    actual value used.

    BlueStack will automatically handle segmentation and reassembly of
    configuration packets.

   \msc
        Application,L2CAP,"Peer L2CAP","Peer Application";

    --- [ label="Connection Established" ];
        Application->L2CAP                  [ label="L2CA_CONFIG_REQ",      URL="\ref L2CA_CONFIG_REQ_T" ];
        L2CAP->"Peer L2CAP"                 [ label="L2CAP_ConfigReq" ];
        "Peer L2CAP"->"Peer Application"    [ label="L2CAP_CONFIG_IND",     URL="\ref L2CA_CONFIG_IND_T" ];
        "Peer Application"->"Peer L2CAP"    [ label="L2CAP_CONFIG_RSP",     URL="\ref L2CA_CONFIG_RSP_T" ];
        "Peer L2CAP"->"L2CAP"               [ label="L2CAP_ConfigRsp" ];
        L2CAP->Application                  [ label="L2CA_CONFIG_CFM",      URL="\ref L2CA_CONFIG_CFM_T" ];
    \endmsc
*/
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONFIG_REQ */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    L2CA_CONFIG_T       config;           /*!< Configuration options */
} L2CA_CONFIG_REQ_T;

/*! \brief Internal. L2CAP configure confirmation */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONFIG_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_conf_result_t  result;           /*!< Response code - uses L2CA_CONFIG range */
    L2CA_CONFIG_T       config;           /*!< Cofniguration options */
} L2CA_CONFIG_CFM_T;

/*! \brief Internal. L2CAP configure indication */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONFIG_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    l2ca_identifier_t   identifier;       /*!< Used to identify the config signal */
    L2CA_CONFIG_T       config;           /*!< Configuration options */
} L2CA_CONFIG_IND_T;

/*! \brief Internal. L2CAP configure response */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CONFIG_RSP */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    l2ca_identifier_t   identifier;       /*!< Used to identify the config signal */
    l2ca_conf_result_t  response;         /*!< Response code - uses L2CA_CONFIG range (only spec values allowed) */
    L2CA_CONFIG_T       config;           /*!< Configuration options */
} L2CA_CONFIG_RSP_T;

/*! \brief Internal. Create a L2CAP channel request, possibly over an AMP */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CREATE_CHANNEL_REQ */
    psm_t               psm_local;        /*!< PSM of local requesting protocol */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_remote;       /*!< The PSM on the remote device */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
    l2ca_controller_t   remote_control;   /*!< Remote controller ID */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
    DM_SM_SERVICE_T     service;          /*!< Security Manager substitute service */
} L2CA_CREATE_CHANNEL_REQ_T;

/*! \brief Internal. Create a L2CAP channel confirm, possibly over an AMP */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CREATE_CHANNEL_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    context_t           con_ctx;          /*!< Opaque context number from connect */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_local;        /*!< The PSM of the local device */
    l2ca_conn_result_t  result;           /*!< Result code - uses L2CA_CONNECT range */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
} L2CA_CREATE_CHANNEL_CFM_T;

/*! \brief Internal. Create a L2CAP channel indication, possibly over an AMP */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CREATE_CHANNEL_IND */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    context_t           reg_ctx;          /*!< Opaque context number from registration */
    l2ca_identifier_t   identifier;       /*!< Used to identify the create signal */
    BD_ADDR_T           bd_addr;          /*!< Bluetooth address of remote device */
    psm_t               psm_local;              /*!< PSM of the local device */
    l2ca_controller_t   local_control;    /*!< Local controller ID */
} L2CA_CREATE_CHANNEL_IND_T;

/*! \brief Internal. Create a L2CAP channel response, possibly over an AMP */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_CREATE_CHANNEL_RSP */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    l2ca_identifier_t   identifier;       /*!< Used to identify the create signal */
    l2ca_conn_result_t  response;         /*!< Result code - uses L2CA_CONNECT range (only spec values allowed) */
    context_t           con_ctx;          /*!< Opaque context number returned in other signals */
} L2CA_CREATE_CHANNEL_RSP_T;

/*! \brief Request HCI connection handle and remote CID associated with
    channel.
   
    Note that this primitive is not supported in a VM/Bluelab application.
    Attempting to send this primitive from such an application will panic
    the firmware. */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_GET_CHANNEL_INFO_REQ */
    l2ca_cid_t          cid;              /*!< Channel identifier */
} L2CA_GET_CHANNEL_INFO_REQ_T;

/*! \brief Confirmation of L2CA_GET_CHANNEL_INFO_REQ, containing
    HCI connection handle and remote CID associated with channel */
typedef struct
{
    l2cap_prim_t        type;             /*!< Always L2CA_GET_CHANNEL_INFO_CFM */
    phandle_t           phandle;          /*!< Destination phandle */
    l2ca_cid_t          cid;              /*!< Channel identifier */
    l2ca_cid_t          remote_cid;       /*!< Remote device's channel identifier or L2CA_CID_INVALID. */
    uint16_t            conn_handle;      /*!< HCI connection handle */
} L2CA_GET_CHANNEL_INFO_CFM_T;


/*! \brief Request L2CAP to extend additional credits to the peer.

    Application can request L2CAP to increase the total number of rx credits 
    extended to a peer any time after the connection set up. This procedure to 
    assign total number of credits should be ideally done only once. Credits 
    supplied in this primitive would add to the total number of rx credits
    extened to the peer.

    Credit management is done by L2CAP. Application need to just specify total 
    number of credits to be extended to the remote by considering its buffer 
    size available for incoming data. Application can know the total 
    number of rx credits extended to the peer by sending a request 
    with 0 credits.
*/

typedef struct
{
    l2cap_prim_t        type;       /*!< Always L2CA_ADD_CREDIT_REQ */
    l2ca_cid_t          cid;        /*!< Channel identifier */
    context_t           context;    /*!< Opaque context number returned in response*/
    uint16_t            credits;    /*!< Extended these credits to remote device */
} L2CA_ADD_CREDIT_REQ_T;


/*! \brief Confirmation of L2CA_ADD_CREDIT_REQ. 

    Confirmation would be sent to the user with actual number of rx credits 
    that is extended to the remote device out of requested rx credit extension.
    It is possible that L2CAP would extend none or less than the requested 
    number of rx credits based on memory availability. Unless the result 
    indicates operation us successful credits value shall be ignored.
*/

typedef struct
{
    l2cap_prim_t        type;       /*!< Always L2CA_ADD_CREDIT_CFM */
    phandle_t           phandle;    /*!< Destination phandle */
    l2ca_cid_t          cid;        /*!< Channel identifier */
    context_t           context;    /*!< Opaque context number from request */
    uint16_t            credits;    /*!< Actual credits extended to remote */
    L2CA_RESULT_T       result;     /*!< Result of L2CA_ADD_CREDIT_REQ */
} L2CA_ADD_CREDIT_CFM_T;


/*! \brief Union of all L2CAP primitives */
typedef union
{
    /* Common fields */
    l2cap_prim_t                        type;                           /*!< Shared for all primitives */
    L2CA_UPSTREAM_COMMON_T              l2ca_upstream_common;           /*!< Shared for all upstream primitives */

    L2CA_DOWNSTREAM_CID_COMMON_T        l2ca_downstream_cid_common;
    L2CA_UPSTREAM_CID_COMMON_T          l2ca_upstream_cid_common;

    /* Downstream */
    L2CA_REGISTER_REQ_T                 l2ca_register_req;              /*!< See \ref L2CA_REGISTER_REQ_T */
    L2CA_CONNECT_REQ_T                  l2ca_connect_req;               /*!< See \ref L2CA_CONNECT_REQ_T */
    L2CA_CONNECT_RSP_T                  l2ca_connect_rsp;               /*!< See \ref L2CA_CONNECT_RSP_T */
    L2CA_CONFIG_REQ_T                   l2ca_config_req;                /*!< See \ref L2CA_CONFIG_REQ_T */
    L2CA_CONFIG_RSP_T                   l2ca_config_rsp;                /*!< See \ref L2CA_CONFIG_RSP_T */
    L2CA_DISCONNECT_REQ_T               l2ca_disconnect_req;            /*!< See \ref L2CA_DISCONNECT_REQ_T */
    L2CA_DISCONNECT_RSP_T               l2ca_disconnect_rsp;            /*!< See \ref L2CA_DISCONNECT_RSP_T */
    L2CA_PING_REQ_T                     l2ca_ping_req;                  /*!< See \ref L2CA_PING_REQ_T */
    L2CA_GETINFO_REQ_T                  l2ca_getinfo_req;               /*!< See \ref L2CA_GETINFO_REQ_T */
    L2CA_DATAWRITE_REQ_T                l2ca_datawrite_req;             /*!< See \ref L2CA_DATAWRITE_REQ_T */
    L2CA_DATAREAD_RSP_T                 l2ca_dataread_rsp;              /*!< See \ref L2CA_DATAREAD_RSP_T */
    L2CA_BUSY_REQ_T                     l2ca_busy_req;                  /*!< See \ref L2CA_BUSY_REQ_T */
    L2CA_RAW_MODE_REQ_T                 l2ca_raw_mode_req;              /*!< See \ref L2CA_RAW_MODE_REQ_T */
    L2CA_RAW_DATA_REQ_T                 l2ca_raw_data_req;              /*!< See \ref L2CA_RAW_DATA_REQ_T */
    L2CA_UNREGISTER_REQ_T               l2ca_unregister_req;            /*!< See \ref L2CA_UNREGISTER_REQ_T */
    L2CA_DATAWRITE_ABORT_REQ_T          l2ca_datawrite_abort_req;       /*!< See \ref L2CA_DATAWRITE_ABORT_REQ_T */

#ifdef BUILD_FOR_HOST
    L2CA_GET_CHANNEL_INFO_REQ_T         l2ca_get_channel_info_req;      /*!< See \ref L2CA_GET_CHANNEL_INFO_REQ_T */
#endif

#ifdef INSTALL_L2CAP_ROUTER_SUPPORT
    L2CA_ROUTE_DATA_REQ_T               l2ca_route_data_req;            /*!< See \ref L2CA_ROUTE_DATA_REQ_T */
    L2CA_UNROUTE_DATA_REQ_T             l2ca_unroute_data_req;          /*!< See \ref L2CA_UNROUTE_DATA_REQ_T */
#endif /* ROUTER */
#ifdef INSTALL_L2CAP_CONNLESS_SUPPORT
    L2CA_MULTICAST_REQ_T                l2ca_multicast_req;             /*!< See \ref L2CA_MULTICAST_REQ_T */
#endif /* CONNLESS */
#ifdef INSTALL_L2CAP_FIXED_CHANNEL_BASE_SUPPORT
#ifdef INSTALL_L2CAP_FIXED_CHANNEL_SUPPORT
    L2CA_REGISTER_FIXED_CID_REQ_T       l2ca_register_fixed_cid_req;    /*!< See \ref L2CA_REGISTER_FIXED_CID_REQ_T */
#endif /*INSTALL_L2CAP_FIXED_CHANNEL_SUPPORT */
    L2CA_MAP_FIXED_CID_REQ_T            l2ca_map_fixed_cid_req;         /*!< See \ref L2CA_MAP_FIXED_CID_REQ_T */
    L2CA_MAP_FIXED_CID_RSP_T            l2ca_map_fixed_cid_rsp;         /*!< See \ref L2CA_MAP_FIXED_CID_RSP_T */
    L2CA_UNMAP_FIXED_CID_REQ_T          l2ca_unmap_fixed_cid_req;       /*!< See \ref L2CA_UNMAP_FIXED_CID_REQ_T */
#endif /* INSTALL_L2CAP_FIXED_CHANNEL_BASE_SUPPORT */
#ifdef INSTALL_AMP_SUPPORT
    L2CA_CREATE_CHANNEL_REQ_T           l2ca_create_channel_req;        /*!< See \ref L2CA_CREATE_CHANNEL_REQ_T */
    L2CA_CREATE_CHANNEL_RSP_T           l2ca_create_channel_rsp;        /*!< See \ref L2CA_CREATE_CHANNEL_RSP_T */
    L2CA_MOVE_CHANNEL_REQ_T             l2ca_move_channel_req;          /*!< See \ref L2CA_MOVE_CHANNEL_REQ_T */
    L2CA_MOVE_CHANNEL_RSP_T             l2ca_move_channel_rsp;          /*!< See \ref L2CA_MOVE_CHANNEL_RSP_T */
#endif /* AMP */
    L2CA_AUTO_CONNECT_REQ_T             l2ca_auto_connect_req;          /*!< See \ref L2CA_AUTO_CONNECT_REQ_T */
    L2CA_AUTO_CONNECT_RSP_T             l2ca_auto_connect_rsp;          /*!< See \ref L2CA_AUTO_CONNECT_RSP_T */

    /* Upstream */
    L2CA_REGISTER_CFM_T                 l2ca_register_cfm;              /*!< See \ref L2CA_REGISTER_CFM_T */
    L2CA_CONNECT_IND_T                  l2ca_connect_ind;               /*!< See \ref L2CA_CONNECT_IND_T */
    L2CA_CONNECT_CFM_T                  l2ca_connect_cfm;               /*!< See \ref L2CA_CONNECT_CFM_T */
    L2CA_CONFIG_IND_T                   l2ca_config_ind;                /*!< See \ref L2CA_CONFIG_IND_T */
    L2CA_CONFIG_CFM_T                   l2ca_config_cfm;                /*!< See \ref L2CA_CONFIG_CFM_T */
    L2CA_DISCONNECT_IND_T               l2ca_disconnect_ind;            /*!< See \ref L2CA_DISCONNECT_IND_T */
    L2CA_DISCONNECT_CFM_T               l2ca_disconnect_cfm;            /*!< See \ref L2CA_DISCONNECT_CFM_T */
    L2CA_TIMEOUT_IND_T                  l2ca_timeout_ind;               /*!< See \ref L2CA_TIMEOUT_IND_T */
    L2CA_PING_CFM_T                     l2ca_ping_cfm;                  /*!< See \ref L2CA_PING_CFM_T */
    L2CA_GETINFO_CFM_T                  l2ca_getinfo_cfm;               /*!< See \ref L2CA_GETINFO_CFM_T */
    L2CA_DATAWRITE_CFM_T                l2ca_datawrite_cfm;             /*!< See \ref L2CA_DATAWRITE_CFM_T */
    L2CA_DATAREAD_IND_T                 l2ca_dataread_ind;              /*!< See \ref L2CA_DATAREAD_IND_T */
    L2CA_BUSY_IND_T                     l2ca_busy_ind;                  /*!< See \ref L2CA_BUSY_IND_T */
    L2CA_MTU_VIOLATION_IND_T            l2ca_mtu_violation_ind;         /*!< See \ref L2CA_MTU_VIOLATION_IND_T */
    L2CA_UNKNOWN_SIGNAL_IND_T           l2ca_unknown_signal_ind;        /*!< See \ref L2CA_UNKNOWN_SIGNAL_IND_T */
    L2CA_RAW_DATA_IND_T                 l2ca_raw_data_ind;              /*!< See \ref L2CA_RAW_DATA_IND_T */
    L2CA_UNREGISTER_CFM_T               l2ca_unregister_cfm;            /*!< See \ref L2CA_UNREGISTER_CFM_T */
    L2CA_DATAWRITE_ABORT_CFM_T          l2ca_datawrite_abort_cfm;       /*!< See \ref L2CA_DATAWRITE_ABORT_CFM_T */

#ifdef BUILD_FOR_HOST
    L2CA_GET_CHANNEL_INFO_CFM_T         l2ca_get_channel_info_cfm;      /*!< See \ref L2CA_GET_CHANNEL_INFO_CFM_T */
#endif

#ifdef INSTALL_L2CAP_ROUTER_SUPPORT
    L2CA_ROUTE_DATA_CFM_T               l2ca_route_data_cfm;            /*!< See \ref L2CA_ROUTE_DATA_CFM_T */
    L2CA_UNROUTE_DATA_CFM_T             l2ca_unroute_data_cfm;          /*!< See \ref L2CA_UNROUTE_DATA_CFM_T */
    L2CA_UNROUTE_DATA_IND_T             l2ca_unroute_data_ind;          /*!< See \ref L2CA_UNROUTE_DATA_IND_T */
#endif
#ifdef INSTALL_L2CAP_FIXED_CHANNEL_BASE_SUPPORT
#ifdef INSTALL_L2CAP_FIXED_CHANNEL_SUPPORT
    L2CA_REGISTER_FIXED_CID_CFM_T       l2ca_register_fixed_cid_cfm;    /*!< See \ref L2CA_REGISTER_FIXED_CID_CFM_T */
#endif /* INSTALL_L2CAP_FIXED_CHANNEL_SUPPORT */
    L2CA_MAP_FIXED_CID_IND_T            l2ca_map_fixed_cid_ind;         /*!< See \ref L2CA_MAP_FIXED_CID_IND_T */
    L2CA_MAP_FIXED_CID_CFM_T            l2ca_map_fixed_cid_cfm;         /*!< See \ref L2CA_MAP_FIXED_CID_CFM_T */
    L2CA_UNMAP_FIXED_CID_IND_T          l2ca_unmap_fixed_cid_ind;       /*!< See \ref L2CA_UNMAP_FIXED_CID_IND_T */
#endif /* INSTALL_L2CAP_FIXED_CHANNEL_BASE_SUPPORT */
#ifdef INSTALL_AMP_SUPPORT
    L2CA_CREATE_CHANNEL_CFM_T           l2ca_create_channel_cfm;        /*!< See \ref L2CA_CREATE_CHANNEL_CFM_T */
    L2CA_CREATE_CHANNEL_IND_T           l2ca_create_channel_ind;        /*!< See \ref L2CA_CREATE_CHANNEL_IND_T */
    L2CA_MOVE_CHANNEL_CFM_T             l2ca_move_channel_cfm;          /*!< See \ref L2CA_MOVE_CHANNEL_CFM_T */
    L2CA_MOVE_CHANNEL_IND_T             l2ca_move_channel_ind;          /*!< See \ref L2CA_MOVE_CHANNEL_IND_T */
    L2CA_MOVE_CHANNEL_CMP_IND_T         l2ca_move_channel_cmp_ind;      /*!< See \ref L2CA_MOVE_CHANNEL_CMP_IND_T */
    L2CA_AMP_LINK_LOSS_IND_T            l2ca_amp_link_loss_ind;         /*!< See \ref L2CA_AMP_LINK_LOSS_IND_T */
#endif /* AMP */
    L2CA_AUTO_CONNECT_CFM_T             l2ca_auto_connect_cfm;          /*!< See \ref L2CA_AUTO_CONNECT_CFM_T */
    L2CA_AUTO_CONNECT_IND_T             l2ca_auto_connect_ind;          /*!< See \ref L2CA_AUTO_CONNECT_IND_T */

#ifdef INSTALL_L2CAP_DEBUG
    /* Debugging interface */
    L2CA_DEBUG_DROP_REQ_T               l2ca_debug_drop_req;            /*!< See \ref L2CA_DEBUG_DROP_REQ_T */
#endif

#ifdef INSTALL_ULP
    L2CA_CONNECTION_PAR_UPDATE_REQ_T    l2ca_connection_par_update_req;
    L2CA_CONNECTION_PAR_UPDATE_CFM_T    l2ca_connection_par_update_cfm;
    L2CA_CONNECTION_PAR_UPDATE_IND_T    l2ca_connection_par_update_ind;
    L2CA_CONNECTION_PAR_UPDATE_RSP_T    l2ca_connection_par_update_rsp;
#endif

    L2CA_AUTO_TP_CONNECT_REQ_T          l2ca_auto_tp_connect_req;       /*!< See \ref L2CA_AUTO_TP_CONNECT_REQ_T */
    L2CA_AUTO_TP_CONNECT_RSP_T          l2ca_auto_tp_connect_rsp;       /*!< See \ref L2CA_AUTO_TP_CONNECT_RSP_T */
    L2CA_AUTO_TP_CONNECT_IND_T          l2ca_auto_tp_connect_ind;       /*!< See \ref L2CA_AUTO_TP_CONNECT_IND_T */
    L2CA_AUTO_TP_CONNECT_CFM_T          l2ca_auto_tp_connect_cfm;       /*!< See \ref L2CA_AUTO_TP_CONNECT_CFM_T */
    L2CA_ADD_CREDIT_REQ_T               l2ca_add_credit_req;            /*!< see \ref L2CA_ADD_CREDIT_REQ_T */
    L2CA_ADD_CREDIT_CFM_T               l2ca_add_credit_cfm;            /*!< see \ref L2CA_ADD_CREDIT_CFM_T */
} L2CA_UPRIM_T;

#ifdef __cplusplus
}
#endif

#endif

/*! @} */
