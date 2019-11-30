#ifndef BLUESTACK__BLUETOOTH_H    /* Once is enough */
#define BLUESTACK__BLUETOOTH_H

/*!

Copyright (c) 2001 - 2016 Qualcomm Technologies International, Ltd.
  

\file   bluetooth.h

\brief  This file contains the Bluetooth specific type definitions.
*/

#include "bluestack/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \name   Bluestack version
    \brief  Bluestack identifies itself in DM_AM_REGISTER_CFM  
    \{ */
    
#define BLUESTACK_VERSION_MAJOR         0x04    /*!< Major version */
#define BLUESTACK_VERSION_MINOR         0x00    /*!< Minor version */
#define BLUESTACK_VERSION_PATCHLEVEL    0x0000  /*!< Patch level */
/*!  \} */
/*
  JBS: Added so the BlueLab debugger can quiz the firmware to find out
  if it's running the same version of RFCOMM.
*/

#define BLUESTACK_VERSION       32 /* CCL version number */

/*============================================================================*
Public Defines
*============================================================================*/
/*! \name   Primitive segmentation
    \brief  These definitions are used in the get_message and put_message in the
            uint16 parameter. We are using it as an event id
    Note: Values above 0xFF are RESERVED.
    \{ */

#define LC_PRIM          1
#define LM_PRIM          2
#define HCI_PRIM         3
#define DM_PRIM          4
#define L2CAP_PRIM       5
#define RFCOMM_PRIM      6
#define SDP_PRIM         7
#define BCSP_LM_PRIM     8
#define BCSP_HQ_PRIM     9   /*!< BC01 Host Querier (chip management) */
#define BCSP_BCCMD_PRIM  10  /*!< BC01 Command      (chip management) */
#define CALLBACK_PRIM    11  /*!< Used to indicate this is a callback primitive */

#ifdef BTCHIP
/*!<    TCS, BNEP, UDP and TCP already defined in host stack with other value*/
#define TCS_PRIM        12
#define BNEP_PRIM       13
#define TCP_PRIM        14
#define UDP_PRIM        15
#endif
#define FB_PRIM         16
#define BCSP_VM_PRIM    17
#define ATT_PRIM        18
/*! \} */


/*! \name  Protocol Service Multiplexers 
    \{ */
#define SDP_PSM         1
#define RFCOMM_PSM      3 
/*! \} */


/*! \name  Segmentation of Bluestack Primitives
    \{ */
#define PAN_PRIM_BASE    0x0000
#define L2CA_PRIM_BASE   0x0500
#define RFCOMM_PRIM_BASE 0x0600
#define SDC_PRIM_BASE    0x0700
#define SDS_PRIM_BASE    0x0800
#define ATT_PRIM_BASE    0x0900
#define GATT_PRIM_BASE   0x0A00     /*!< Not used with BlueCore. Reserved for CSR1000. */ 
/*! \} */

/*! \name  Segmentation of Bluestack Error codes
    \{ */
#define BLUESTACK_ERRORCODE_BLOCK   0x0800
#define L2CA_SIG_ERRORCODE_BASE     0x0001
#define L2CA_ERRORCODE_BASE         (BLUESTACK_ERRORCODE_BLOCK * 1) 
#define RFCOMM_ERRORCODE_BASE       (BLUESTACK_ERRORCODE_BLOCK * 2)
#define SDC_ERRORCODE_BASE          (BLUESTACK_ERRORCODE_BLOCK * 3)
#define SDS_ERRORCODE_BASE          (BLUESTACK_ERRORCODE_BLOCK * 4)
#define ATT_ERRORCODE_BASE          (BLUESTACK_ERRORCODE_BLOCK * 5)
/*! \} */


/*============================================================================*
Public Data Types
*============================================================================*/

/*! \name  Bluetooth types
    \{ */
typedef uint16_t    con_handle_t;       /*!< HCI Connection handle (14 bit) */
typedef uint8_t     reason_t;           /*!< Error codes (or reasons) */
typedef uint16_t    packet_type_t;      /*!< Packet type (DM1, DH1) */
typedef uint16_t    bt_clock_offset_t;  /*!< Bluetooth clock offset */
typedef uint32_t    device_class_t;     /*!< Lower 3 bytes */
typedef uint8_t     page_scan_mode_t;   /*!< 3 bits from FHS */
typedef uint16_t    psm_t;              /*!< L2CAP PSM */
/*! \} */


/* 
 * Following typedefines are used mostly used for AMP and 
 * are shared through DM and L2CAP interface
 */
typedef uint16_t    l2ca_controller_t;     /* spec only uses 8 LSBs */
typedef uint16_t    l2ca_cid_t;            /* L2CAP Channel Identifier */
typedef uint8_t     l2ca_fs_identifier_t;
typedef uint8_t     l2ca_fs_service_t;
typedef uint16_t    l2ca_fs_mtu_t;
typedef uint32_t    l2ca_fs_flush_t;       /* high-precision flush timeout (us) */
typedef uint32_t    l2ca_bandwidth_t;
typedef uint32_t    l2ca_latency_t;
typedef uint8_t     l2ca_identifier_t;      /* Request/response identifier */

/*! \brief L2CAP extended flow specification options

    The AMP extended flow specification QoS structure. It takes up 16
    bytes over the air.
*/
typedef struct
{
    l2ca_fs_identifier_t   fs_identifier;     /*!< Flow spec identifier */
    l2ca_fs_service_t      fs_service_type;   /*!< Best effort, etc. */
    l2ca_fs_mtu_t          fs_max_sdu;        /*!< Maximum SDU size */
    l2ca_bandwidth_t       fs_interarrival;   /*!< Bandwidth: Time between SDUs, us */
    l2ca_latency_t         fs_latency;        /*!< Maximum latency between tx opportunities, us */
    l2ca_fs_flush_t        fs_flush_to;       /*!< High-resolution flush timeout, us */
} L2CA_FLOWSPEC_T;

typedef struct
{
    uint16_t    buffer_size_sink;   /*!< Bytes of stream buffer size. */
    uint16_t    buffer_size_source; /*!< Bytes of stream buffer size. */
} STREAM_BUFFER_SIZES_T;

typedef uint16_t dm_protocol_id_t;
typedef struct
{
    dm_protocol_id_t protocol_id;
    uint16_t channel;
} DM_SM_SERVICE_T;

typedef struct MBLK_T_tag MBLK_T;


/*! \name UUID's for use by security manager
    \brief Added UUID's for use by security manager SP2 4 12 2000 
    \{ */
typedef uint16_t uuid16_t; 

#define UUID16_SDP     ((uuid16_t)0x0001)
#define UUID16_RFCOMM  ((uuid16_t)0x0003)
#define UUID16_TCS_BIN ((uuid16_t)0x0005)
#define UUID16_L2CAP   ((uuid16_t)0x0100)
#define UUID16_IP      ((uuid16_t)0x0009)
#define UUID16_UDP     ((uuid16_t)0x0002)
#define UUID16_TCP     ((uuid16_t)0x0004)
#define UUID16_TCS_AT  ((uuid16_t)0x0006)
#define UUID16_OBEX    ((uuid16_t)0x0008)
#define UUID16_FTP     ((uuid16_t)0x000A)
#define UUID16_HTTP    ((uuid16_t)0x000C)
#define UUID16_WSP     ((uuid16_t)0x000E)
/*! \} */



/*!  \brief Bluestack type for Bluetooth device address

*/
typedef struct
{
    uint24_t lap;   /*!< Lower Address Part 00..23 */
    uint8_t  uap;   /*!< upper Address Part 24..31 */
    uint16_t nap;   /*!< Non-significant    32..47 */
} BD_ADDR_T;

#define TBDADDR_PUBLIC   ((uint8_t)0x00)
#define TBDADDR_RANDOM   ((uint8_t)0x01)
#define TBDADDR_INVALID  ((uint8_t)0xFF)
typedef struct
{
    uint8_t type;   
    BD_ADDR_T addr;
} TYPED_BD_ADDR_T;

/* Enumeration for ACL type physical transport.
   In essence, this identify a enumeration identify
   a unique physical transport. */
typedef enum
{
    BREDR_ACL,
    LE_ACL,
    NO_PHYSICAL_TRANSPORT = 0xFF
}PHYSICAL_TRANSPORT_T;

typedef struct
{
    TYPED_BD_ADDR_T      addrt;
    PHYSICAL_TRANSPORT_T tp_type;
} TP_BD_ADDR_T;


/*============================================================================*
Public Data
*============================================================================*/
/* None */

/*============================================================================*
Public Functions
*============================================================================*/
/* None */

#ifdef __cplusplus
}
#endif 

#endif /* ndef _BLUETOOTH_H */
