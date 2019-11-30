#ifndef BLUESTACK__HCI_H    /* Once is enough */
#define BLUESTACK__HCI_H

/*!

Copyright (c) 2001 - 2019 Qualcomm Technologies International, Ltd.
  

\file   hci.h

\brief  This file contains HCI specific type definitions.
*/

#include "hydra_types.h"
#include "bluestack/bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

/* search_string="hci_event_code_t" */
/* conversion_rule="ADD_UNDERSCORE_AND_UPPERCASE_T" */

/*============================================================================*
                            Public Defines
 *============================================================================*/
/******************************************************************************
   HCI The variable argumnet lists of several HCI commands are split into
   a header with pointers to pmalloc'd space. This define defines the size of
   this pmalloc space. This allows the variable argument commands to be
   resized according to memory constraints on the host controller.
   If on XAP this in words.

 *****************************************************************************/
#define HCI_VAR_ARG_POOL_SIZE   32

/******************************************************************************
   Type Definitions
 *****************************************************************************/
typedef uint16_t hci_packet_boundary_flag_t;
typedef uint16_t hci_connection_handle_t;
typedef uint16_t hci_op_code_t;            /* used for commands  */
typedef uint8_t  hci_event_code_t;         /* used for events    */
typedef uint8_t  hci_return_t;             /* return value (error if !success) */
typedef uint8_t  hci_error_t;
typedef uint16_t hci_pkt_type_t;
typedef uint8_t  hci_role_t;               /* whether host controller is master or slave */
typedef uint8_t  hci_key_flag_t;           /* type of link key */
typedef uint8_t  hci_link_enc_t;           /* whether link encryption on or off */
typedef uint8_t  hci_qos_type_t;           /* service type for QoS */
typedef uint32_t hci_event_mask_t;         /* event mask type.  This is 8 bytes in fact
                                              declared as an array[2].  Only 4 ls bytes used */
typedef uint8_t  hci_version_t;            /* version info */
typedef uint8_t  hci_bt_mode_t;            /* used for indicating state change */
typedef uint8_t  hci_reason_t;             /* Reason for command */
typedef uint8_t  page_scan_rep_mode_t;     /* Page scan repetition mode */
typedef uint16_t clock_offset_t;           /* Clock offset */
typedef uint16_t link_policy_settings_t;   /* Link policy settings */
typedef uint8_t  filter_type_t;            /* Event filter type */
typedef uint8_t  filter_condition_type_t;  /* Inquiry Result Filter Condition Type */
typedef uint8_t  pin_type_t;               /* Pin Type */
typedef uint8_t  read_all_flag_t;
typedef uint8_t  delete_all_flag_t;
typedef uint8_t  auto_accept_t;
typedef uint8_t  flushable_packet_type_t;  /* Type of packet to be flushed during
                                              enhanced flush operation */
#ifdef INSTALL_AMP_SUPPORT
typedef uint32_t l2ca_hci_buffer_size_t; /*!< Buffer size with AMP */
#else
typedef uint16_t l2ca_hci_buffer_size_t; /*!< Buffer size */
#endif


/******************************************************************************
   Defines for the OGF opcodes group - these occupy the 6 msb of the opcode
   hence the values
 *****************************************************************************/
#define HCI_OGF_BIT_OFFSET             ((uint8_t)10) /* Number of bit shifts */

#define HCI_LINK                       ((hci_op_code_t)0x0400)
#define HCI_POLICY                     ((hci_op_code_t)0x0800)
#define HCI_HOST_BB                    ((hci_op_code_t)0x0C00)
#define HCI_INFO                       ((hci_op_code_t)0x1000)
#define HCI_STATUS                     ((hci_op_code_t)0x1400)
#define HCI_TEST                       ((hci_op_code_t)0x1800)
/*                                     ((hci_op_code_t)0x1C00)*/
#define HCI_ULP                        ((hci_op_code_t)0x2000)
#define HCI_MAX_OGF                    ((hci_op_code_t)0x2400)
#define HCI_MANUFACTURER_EXTENSION     ((hci_op_code_t)0xFC00)
#define HCI_OPCODE_GROUP_MASK          ((hci_op_code_t)0xFC00)
#define HCI_OPCODE_MASK                ((hci_op_code_t)0x03FF)

#define OGF_IS_MANUFACTURER_EXTENSION(opcode) \
            (((opcode) & HCI_OPCODE_GROUP_MASK) == HCI_MANUFACTURER_EXTENSION)

/*---------------------------------------------------------------------------*\
   These defines are needed so that the hci generator can identify
   the reply (_RET) macros.
\*---------------------------------------------------------------------------*/
#define HCI_LINK_RET                   HCI_LINK
#define HCI_POLICY_RET                 HCI_POLICY
#define HCI_HOST_BB_RET                HCI_HOST_BB
#define HCI_INFO_RET                   HCI_INFO
#define HCI_STATUS_RET                 HCI_STATUS
#define HCI_TEST_RET                   HCI_TEST
#define HCI_ULP_RET                    HCI_ULP

/******************************************************************************
   Miscellaneous
 *****************************************************************************/
#define HCI_NOP                         (hci_op_code_t) 0x0000

/******************************************************************************
   OCF opcode defines - Link Control Commands
 *****************************************************************************/
#define HCI_INQUIRY                    ((hci_op_code_t)HCI_LINK | 0x0001)
#define HCI_INQUIRY_CANCEL             ((hci_op_code_t)HCI_LINK | 0x0002)
#define HCI_PERIODIC_INQUIRY_MODE      ((hci_op_code_t)HCI_LINK | 0x0003)
#define HCI_EXIT_PERIODIC_INQUIRY_MODE ((hci_op_code_t)HCI_LINK | 0x0004)
#define HCI_CREATE_CONNECTION          ((hci_op_code_t)HCI_LINK | 0x0005)
#define HCI_DISCONNECT                 ((hci_op_code_t)HCI_LINK | 0x0006)
#define HCI_ADD_SCO_CONNECTION         ((hci_op_code_t)HCI_LINK | 0x0007)
#define HCI_CREATE_CONNECTION_CANCEL   ((hci_op_code_t)HCI_LINK | 0x0008)
#define HCI_ACCEPT_CONNECTION_REQ      ((hci_op_code_t)HCI_LINK | 0x0009)
#define HCI_REJECT_CONNECTION_REQ      ((hci_op_code_t)HCI_LINK | 0x000A)
#define HCI_LINK_KEY_REQ_REPLY         ((hci_op_code_t)HCI_LINK | 0x000B)
#define HCI_LINK_KEY_REQ_NEG_REPLY     ((hci_op_code_t)HCI_LINK | 0x000C)
#define HCI_PIN_CODE_REQ_REPLY         ((hci_op_code_t)HCI_LINK | 0x000D)
#define HCI_PIN_CODE_REQ_NEG_REPLY     ((hci_op_code_t)HCI_LINK | 0x000E)
#define HCI_CHANGE_CONN_PKT_TYPE       ((hci_op_code_t)HCI_LINK | 0x000F)
#define HCI_AUTH_REQ                   ((hci_op_code_t)HCI_LINK | 0x0011)
#define HCI_SET_CONN_ENCRYPTION        ((hci_op_code_t)HCI_LINK | 0x0013)
#define HCI_CHANGE_CONN_LINK_KEY       ((hci_op_code_t)HCI_LINK | 0x0015)
#define HCI_MASTER_LINK_KEY            ((hci_op_code_t)HCI_LINK | 0x0017)
#define HCI_REMOTE_NAME_REQ            ((hci_op_code_t)HCI_LINK | 0x0019)
#define HCI_REMOTE_NAME_REQ_CANCEL     ((hci_op_code_t)HCI_LINK | 0x001A)
#define HCI_READ_REMOTE_SUPP_FEATURES  ((hci_op_code_t)HCI_LINK | 0x001B)
#define HCI_READ_REMOTE_EXT_FEATURES   ((hci_op_code_t)HCI_LINK | 0x001C)
#define HCI_READ_REMOTE_VER_INFO       ((hci_op_code_t)HCI_LINK | 0x001D)
#define HCI_READ_CLOCK_OFFSET          ((hci_op_code_t)HCI_LINK | 0x001F)
#define HCI_MAX_LINK_OCF_V1_1          ((hci_op_code_t) 0x0020)

/* 1.2 Features */
#define HCI_READ_LMP_HANDLE               ((hci_op_code_t)HCI_LINK | 0x0020)
#define HCI_EXCHANGE_FIXED_INFO           ((hci_op_code_t)HCI_LINK | 0x0021)
#define HCI_EXCHANGE_ALIAS_INFO           ((hci_op_code_t)HCI_LINK | 0x0022)
#define HCI_PRIVATE_PAIRING_REQ_REPLY     ((hci_op_code_t)HCI_LINK | 0x0023)
#define HCI_PRIVATE_PAIRING_REQ_NEG_REPLY ((hci_op_code_t)HCI_LINK | 0x0024)
#define HCI_GENERATED_ALIAS               ((hci_op_code_t)HCI_LINK | 0x0025)
#define HCI_ALIAS_ADDRESS_REQ_REPLY       ((hci_op_code_t)HCI_LINK | 0x0026)
#define HCI_ALIAS_ADDRESS_REQ_NEG_REPLY   ((hci_op_code_t)HCI_LINK | 0x0027)
#define HCI_SETUP_SYNCHRONOUS_CONN        ((hci_op_code_t)HCI_LINK | 0x0028)
#define HCI_ACCEPT_SYNCHRONOUS_CONN_REQ   ((hci_op_code_t)HCI_LINK | 0x0029)
#define HCI_REJECT_SYNCHRONOUS_CONN_REQ   ((hci_op_code_t)HCI_LINK | 0x002A)

/* Simple Pairing */
#define HCI_IO_CAPABILITY_REQUEST_REPLY             ((hci_op_code_t)HCI_LINK | 0x002B)
#define HCI_USER_CONFIRMATION_REQUEST_REPLY         ((hci_op_code_t)HCI_LINK | 0x002C)
#define HCI_USER_CONFIRMATION_REQUEST_NEG_REPLY     ((hci_op_code_t)HCI_LINK | 0x002D)
#define HCI_USER_PASSKEY_REQUEST_REPLY              ((hci_op_code_t)HCI_LINK | 0x002E)
#define HCI_USER_PASSKEY_REQUEST_NEG_REPLY          ((hci_op_code_t)HCI_LINK | 0x002F)
#define HCI_REMOTE_OOB_DATA_REQUEST_REPLY           ((hci_op_code_t)HCI_LINK | 0x0030)
#define HCI_REMOTE_OOB_DATA_REQUEST_NEG_REPLY       ((hci_op_code_t)HCI_LINK | 0x0033)
#define HCI_IO_CAPABILITY_REQUEST_NEG_REPLY         ((hci_op_code_t)HCI_LINK | 0x0034)

/* CSA2 Audio Routing */
#define HCI_ENHANCED_SETUP_SYNC_CONN                ((hci_op_code_t)HCI_LINK | 0x003D)
#define HCI_ENHANCED_ACCEPT_SYNC_CONN_REQ           ((hci_op_code_t)HCI_LINK | 0x003E)

/* CSB Features */
#define HCI_TRUNCATED_PAGE               ((hci_op_code_t)HCI_LINK | 0x003F)
#define HCI_TRUNCATED_PAGE_CANCEL        ((hci_op_code_t)HCI_LINK | 0x0040)
#define HCI_SET_CSB                      ((hci_op_code_t)HCI_LINK | 0x0041)
#define HCI_SET_CSB_RECEIVE              ((hci_op_code_t)HCI_LINK | 0x0042)
#define HCI_START_SYNCHRONIZATION_TRAIN  ((hci_op_code_t)HCI_LINK | 0x0043)
#define HCI_RECEIVE_SYNCHRONIZATION_TRAIN ((hci_op_code_t)HCI_LINK | 0x0044)
/* Secure Connections */
#define HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY  ((hci_op_code_t)HCI_LINK | 0x0045)
#define HCI_MAX_LINK_OCF                            ((hci_op_code_t) 0x0046)

/******************************************************************************
   OP CODE defines - Link Policy Commands
 *****************************************************************************/
#define HCI_HOLD_MODE                  ((hci_op_code_t)HCI_POLICY | 0x0001)
#define HCI_SNIFF_MODE                 ((hci_op_code_t)HCI_POLICY | 0x0003)
#define HCI_EXIT_SNIFF_MODE            ((hci_op_code_t)HCI_POLICY | 0x0004)
#define HCI_QOS_SETUP                  ((hci_op_code_t)HCI_POLICY | 0x0007)
#define HCI_ROLE_DISCOVERY             ((hci_op_code_t)HCI_POLICY | 0x0009)
#define HCI_SWITCH_ROLE                ((hci_op_code_t)HCI_POLICY | 0x000B)
#define HCI_READ_LINK_POLICY_SETTINGS  ((hci_op_code_t)HCI_POLICY | 0x000C)
#define HCI_WRITE_LINK_POLICY_SETTINGS ((hci_op_code_t)HCI_POLICY | 0x000D)
#define HCI_MAX_POLICY_OCF_V1_1        ((hci_op_code_t) 0x000E)

/* 1.2 Features */
#define HCI_READ_DEFAULT_LINK_POLICY_SETTINGS  ((hci_op_code_t)HCI_POLICY | 0x000E)
#define HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS ((hci_op_code_t)HCI_POLICY | 0x000F)
#define HCI_FLOW_SPEC                  ((hci_op_code_t)HCI_POLICY | 0x0010)
#define HCI_SNIFF_SUB_RATE             ((hci_op_code_t)HCI_POLICY | 0x0011)
#define HCI_MAX_POLICY_OCF             ((hci_op_code_t) 0x0012)

/******************************************************************************
   OP CODE defines - Host Controller and Baseband Commands
 *****************************************************************************/
#define HCI_SET_EVENT_MASK             ((hci_op_code_t)HCI_HOST_BB | 0x0001)
#define HCI_RESET                      ((hci_op_code_t)HCI_HOST_BB | 0x0003)
#define HCI_SET_EVENT_FILTER           ((hci_op_code_t)HCI_HOST_BB | 0x0005)
#define HCI_FLUSH                      ((hci_op_code_t)HCI_HOST_BB | 0x0008)
#define HCI_READ_PIN_TYPE              ((hci_op_code_t)HCI_HOST_BB | 0x0009)
#define HCI_WRITE_PIN_TYPE             ((hci_op_code_t)HCI_HOST_BB | 0x000A)
#define HCI_CREATE_NEW_UNIT_KEY        ((hci_op_code_t)HCI_HOST_BB | 0x000B)
#define HCI_READ_STORED_LINK_KEY       ((hci_op_code_t)HCI_HOST_BB | 0x000D)
#define HCI_WRITE_STORED_LINK_KEY      ((hci_op_code_t)HCI_HOST_BB | 0x0011)
#define HCI_DELETE_STORED_LINK_KEY     ((hci_op_code_t)HCI_HOST_BB | 0x0012)
#define HCI_CHANGE_LOCAL_NAME          ((hci_op_code_t)HCI_HOST_BB | 0x0013)
#define HCI_READ_LOCAL_NAME            ((hci_op_code_t)HCI_HOST_BB | 0x0014)
#define HCI_READ_CONN_ACCEPT_TIMEOUT   ((hci_op_code_t)HCI_HOST_BB | 0x0015)
#define HCI_WRITE_CONN_ACCEPT_TIMEOUT  ((hci_op_code_t)HCI_HOST_BB | 0x0016)
#define HCI_READ_PAGE_TIMEOUT          ((hci_op_code_t)HCI_HOST_BB | 0x0017)
#define HCI_WRITE_PAGE_TIMEOUT         ((hci_op_code_t)HCI_HOST_BB | 0x0018)
#define HCI_READ_SCAN_ENABLE           ((hci_op_code_t)HCI_HOST_BB | 0x0019)
#define HCI_WRITE_SCAN_ENABLE          ((hci_op_code_t)HCI_HOST_BB | 0x001A)
#define HCI_READ_PAGESCAN_ACTIVITY     ((hci_op_code_t)HCI_HOST_BB | 0x001B)
#define HCI_WRITE_PAGESCAN_ACTIVITY    ((hci_op_code_t)HCI_HOST_BB | 0x001C)
#define HCI_READ_INQUIRYSCAN_ACTIVITY  ((hci_op_code_t)HCI_HOST_BB | 0x001D)
#define HCI_WRITE_INQUIRYSCAN_ACTIVITY ((hci_op_code_t)HCI_HOST_BB | 0x001E)
#define HCI_READ_AUTH_ENABLE           ((hci_op_code_t)HCI_HOST_BB | 0x001F)
#define HCI_WRITE_AUTH_ENABLE          ((hci_op_code_t)HCI_HOST_BB | 0x0020)
#define HCI_READ_ENC_MODE              ((hci_op_code_t)HCI_HOST_BB | 0x0021)
#define HCI_WRITE_ENC_MODE             ((hci_op_code_t)HCI_HOST_BB | 0x0022)
#define HCI_READ_CLASS_OF_DEVICE       ((hci_op_code_t)HCI_HOST_BB | 0x0023)
#define HCI_WRITE_CLASS_OF_DEVICE      ((hci_op_code_t)HCI_HOST_BB | 0x0024)
#define HCI_READ_VOICE_SETTING         ((hci_op_code_t)HCI_HOST_BB | 0x0025)
#define HCI_WRITE_VOICE_SETTING        ((hci_op_code_t)HCI_HOST_BB | 0x0026)
#define HCI_READ_AUTO_FLUSH_TIMEOUT    ((hci_op_code_t)HCI_HOST_BB | 0x0027)
#define HCI_WRITE_AUTO_FLUSH_TIMEOUT   ((hci_op_code_t)HCI_HOST_BB | 0x0028)
#define HCI_READ_NUM_BCAST_RETXS       ((hci_op_code_t)HCI_HOST_BB | 0x0029)
#define HCI_WRITE_NUM_BCAST_RETXS      ((hci_op_code_t)HCI_HOST_BB | 0x002A)
#define HCI_READ_HOLD_MODE_ACTIVITY    ((hci_op_code_t)HCI_HOST_BB | 0x002B)
#define HCI_WRITE_HOLD_MODE_ACTIVITY   ((hci_op_code_t)HCI_HOST_BB | 0x002C)
#define HCI_READ_TX_POWER_LEVEL        ((hci_op_code_t)HCI_HOST_BB | 0x002D)
#define HCI_READ_SCO_FLOW_CON_ENABLE   ((hci_op_code_t)HCI_HOST_BB | 0x002E)
#define HCI_WRITE_SCO_FLOW_CON_ENABLE  ((hci_op_code_t)HCI_HOST_BB | 0x002F)
#define HCI_SET_HCTOHOST_FLOW_CONTROL  ((hci_op_code_t)HCI_HOST_BB | 0x0031)
#define HCI_HOST_BUFFER_SIZE           ((hci_op_code_t)HCI_HOST_BB | 0x0033)
#define HCI_HOST_NUM_COMPLETED_PACKETS ((hci_op_code_t)HCI_HOST_BB | 0x0035)
#define HCI_READ_LINK_SUPERV_TIMEOUT   ((hci_op_code_t)HCI_HOST_BB | 0x0036)
#define HCI_WRITE_LINK_SUPERV_TIMEOUT  ((hci_op_code_t)HCI_HOST_BB | 0x0037)
#define HCI_READ_NUM_SUPPORTED_IAC     ((hci_op_code_t)HCI_HOST_BB | 0x0038)
#define HCI_READ_CURRENT_IAC_LAP       ((hci_op_code_t)HCI_HOST_BB | 0x0039)
#define HCI_WRITE_CURRENT_IAC_LAP      ((hci_op_code_t)HCI_HOST_BB | 0x003A)
#define HCI_READ_PAGESCAN_PERIOD_MODE  ((hci_op_code_t)HCI_HOST_BB | 0x003B)
#define HCI_WRITE_PAGESCAN_PERIOD_MODE ((hci_op_code_t)HCI_HOST_BB | 0x003C)
#define HCI_READ_PAGESCAN_MODE         ((hci_op_code_t)HCI_HOST_BB | 0x003D)
#define HCI_WRITE_PAGESCAN_MODE        ((hci_op_code_t)HCI_HOST_BB | 0x003E)
#define HCI_MAX_HOST_BB_OCF_V1_1       ((hci_op_code_t) 0x003f)

/* 1.2 Features */
#define HCI_SET_AFH_CHANNEL_CLASS      ((hci_op_code_t)HCI_HOST_BB | 0x003F)
#define HCI_READ_INQUIRY_SCAN_TYPE     ((hci_op_code_t)HCI_HOST_BB | 0x0042)
#define HCI_WRITE_INQUIRY_SCAN_TYPE    ((hci_op_code_t)HCI_HOST_BB | 0x0043)
#define HCI_READ_INQUIRY_MODE          ((hci_op_code_t)HCI_HOST_BB | 0x0044)
#define HCI_WRITE_INQUIRY_MODE         ((hci_op_code_t)HCI_HOST_BB | 0x0045)
#define HCI_READ_PAGE_SCAN_TYPE        ((hci_op_code_t)HCI_HOST_BB | 0x0046)
#define HCI_WRITE_PAGE_SCAN_TYPE       ((hci_op_code_t)HCI_HOST_BB | 0x0047)
#define HCI_READ_AFH_CHANNEL_CLASS_M   ((hci_op_code_t)HCI_HOST_BB | 0x0048)
#define HCI_WRITE_AFH_CHANNEL_CLASS_M  ((hci_op_code_t)HCI_HOST_BB | 0x0049)
#define HCI_READ_ANON_MODE             ((hci_op_code_t)HCI_HOST_BB | 0x004A)
#define HCI_WRITE_ANON_MODE            ((hci_op_code_t)HCI_HOST_BB | 0x004B)
#define HCI_READ_ALIAS_AUTH_ENABLE     ((hci_op_code_t)HCI_HOST_BB | 0x004C)
#define HCI_WRITE_ALIAS_AUTH_ENABLE    ((hci_op_code_t)HCI_HOST_BB | 0x004D)
#define HCI_READ_ANON_ADDR_CHANGE_PARAMS  ((hci_op_code_t)HCI_HOST_BB | 0x004E)
#define HCI_WRITE_ANON_ADDR_CHANGE_PARAMS ((hci_op_code_t)HCI_HOST_BB | 0x004F)
#define HCI_RESET_FIXED_ADDRESS_ATTEMPTS_COUNTER  ((hci_op_code_t)HCI_HOST_BB | 0x0050)

/* 2.1 Features */
#define HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA   ((hci_op_code_t)HCI_HOST_BB | 0x0051)
#define HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA  ((hci_op_code_t)HCI_HOST_BB | 0x0052)
#define HCI_REFRESH_ENCRYPTION_KEY                ((hci_op_code_t)HCI_HOST_BB | 0x0053)
#define HCI_READ_SIMPLE_PAIRING_MODE              ((hci_op_code_t)HCI_HOST_BB | 0x0055)
#define HCI_WRITE_SIMPLE_PAIRING_MODE             ((hci_op_code_t)HCI_HOST_BB | 0x0056)
#define HCI_READ_LOCAL_OOB_DATA                   ((hci_op_code_t)HCI_HOST_BB | 0x0057)
#define HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL  ((hci_op_code_t)HCI_HOST_BB | 0x0058)
#define HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL    ((hci_op_code_t)HCI_HOST_BB | 0x0059)

#define HCI_ENHANCED_FLUSH             ((hci_op_code_t)HCI_HOST_BB | 0x005F)
#define HCI_SEND_KEYPRESS_NOTIFICATION ((hci_op_code_t)HCI_HOST_BB | 0x0060)
#define HCI_SET_EVENT_MASK_PAGE_2      ((hci_op_code_t)HCI_HOST_BB | 0x0063)

/* BT3.0 Features */
#define HCI_READ_ENH_TX_POWER_LEVEL    ((hci_op_code_t)HCI_HOST_BB | 0x0068)

/* Tokyo features */
#define HCI_READ_LE_HOST_SUPPORT       ((hci_op_code_t)HCI_HOST_BB | 0x006C)
#define HCI_WRITE_LE_HOST_SUPPORT      ((hci_op_code_t)HCI_HOST_BB | 0x006D)

/* CSB Features */
#define HCI_SET_RESERVED_LT_ADDR               ((hci_op_code_t)HCI_HOST_BB | 0x0074)
#define HCI_DELETE_RESERVED_LT_ADDR            ((hci_op_code_t)HCI_HOST_BB | 0x0075)
#define HCI_SET_CSB_DATA                       ((hci_op_code_t)HCI_HOST_BB | 0x0076)
#define HCI_READ_SYNCHRONIZATION_TRAIN_PARAMS  ((hci_op_code_t)HCI_HOST_BB | 0x0077)
#define HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS ((hci_op_code_t)HCI_HOST_BB | 0x0078)

/* Secure Connections */
#define HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT        ((hci_op_code_t)HCI_HOST_BB | 0x0079)
#define HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT       ((hci_op_code_t)HCI_HOST_BB | 0x007A)
#define HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT          ((hci_op_code_t)HCI_HOST_BB | 0x007B)
#define HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT         ((hci_op_code_t)HCI_HOST_BB | 0x007C)
#define HCI_READ_LOCAL_OOB_EXTENDED_DATA                ((hci_op_code_t)HCI_HOST_BB | 0x007D)

#define HCI_MAX_HOST_BB_OCF            ((hci_op_code_t) 0x007E)


/******************************************************************************
   OP CODE defines - Informational Parameters
 *****************************************************************************/
#define HCI_READ_LOCAL_VER_INFO        ((hci_op_code_t)HCI_INFO | 0x0001)
/* 1.2 Feature */
#define HCI_READ_LOCAL_SUPP_COMMANDS   ((hci_op_code_t)HCI_INFO | 0x0002)
#define HCI_READ_LOCAL_SUPP_FEATURES   ((hci_op_code_t)HCI_INFO | 0x0003)
/* 1.2 Feature */
#define HCI_READ_LOCAL_EXT_FEATURES    ((hci_op_code_t)HCI_INFO | 0x0004)
#define HCI_READ_BUFFER_SIZE           ((hci_op_code_t)HCI_INFO | 0x0005)
#define HCI_READ_COUNTRY_CODE          ((hci_op_code_t)HCI_INFO | 0x0007)
#define HCI_READ_BD_ADDR               ((hci_op_code_t)HCI_INFO | 0x0009)
#define HCI_MAX_INFO_OCF_V1_1          ((hci_op_code_t) 0x000A)

/* CSA2 Audio Routing Continues */
#define HCI_READ_LOCAL_SUPP_CODECS     ((hci_op_code_t)HCI_INFO | 0x000B)

#define HCI_MAX_INFO_OCF               ((hci_op_code_t) 0x000C)

/******************************************************************************
   OP CODE defines - Status Parameters
 *****************************************************************************/
#define HCI_READ_FAILED_CONTACT_COUNT  ((hci_op_code_t)HCI_STATUS | 0x0001)
#define HCI_RESET_FAILED_CONTACT_COUNT ((hci_op_code_t)HCI_STATUS | 0x0002)
#define HCI_GET_LINK_QUALITY           ((hci_op_code_t)HCI_STATUS | 0x0003)
#define HCI_READ_RSSI                  ((hci_op_code_t)HCI_STATUS | 0x0005)
#define HCI_MAX_STATUS_OCF_V1_1        ((hci_op_code_t) 0x0006)

/* 1.2 Features */
#define HCI_READ_AFH_CHANNEL_MAP       ((hci_op_code_t)HCI_STATUS | 0x0006)
#define HCI_READ_CLOCK                 ((hci_op_code_t)HCI_STATUS | 0x0007)

/* 3.0 Features */
#define HCI_READ_ENCRYPTION_KEY_SIZE   ((hci_op_code_t)HCI_STATUS | 0x0008)

/* TCC Feature */
#define HCI_SET_TRIGGERED_CLOCK_CAPTURE ((hci_op_code_t)HCI_STATUS | 0x000D)

#define HCI_MAX_STATUS_OCF             ((hci_op_code_t) 0x000E)

/******************************************************************************
   OP CODE defines - Testing Parameters
 *****************************************************************************/
#define HCI_READ_LOOPBACK_MODE               ((hci_op_code_t)HCI_TEST | 0x0001)
#define HCI_WRITE_LOOPBACK_MODE              ((hci_op_code_t)HCI_TEST | 0x0002)
#define HCI_ENABLE_DUT_MODE                  ((hci_op_code_t)HCI_TEST | 0x0003)
#define HCI_MAX_TEST_OCF_V1_1                ((hci_op_code_t) 0x0004)
#define HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE  ((hci_op_code_t)HCI_TEST | 0x0004)
#define HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE ((hci_op_code_t)HCI_TEST | 0x000A)
#define HCI_MAX_TEST_OCF                     ((hci_op_code_t) 0x000B)

/******************************************************************************
  OP CODE defines - ULP
 ******************************************************************************/
#define HCI_ULP_SET_EVENT_MASK                            ((hci_op_code_t)HCI_ULP | 0x0001)
#define HCI_ULP_READ_BUFFER_SIZE                          ((hci_op_code_t)HCI_ULP | 0x0002)
#define HCI_ULP_READ_LOCAL_SUPPORTED_FEATURES             ((hci_op_code_t)HCI_ULP | 0x0003)
/* #define HCI_ULP_SET_LOCAL_USED_FEATURES                ((hci_op_code_t)HCI_ULP | 0x0004) */
#define HCI_ULP_SET_RANDOM_ADDRESS                        ((hci_op_code_t)HCI_ULP | 0x0005)
#define HCI_ULP_SET_ADVERTISING_PARAMETERS                ((hci_op_code_t)HCI_ULP | 0x0006)
#define HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER         ((hci_op_code_t)HCI_ULP | 0x0007)
#define HCI_ULP_SET_ADVERTISING_DATA                      ((hci_op_code_t)HCI_ULP | 0x0008)
#define HCI_ULP_SET_SCAN_RESPONSE_DATA                    ((hci_op_code_t)HCI_ULP | 0x0009)
#define HCI_ULP_SET_ADVERTISE_ENABLE                      ((hci_op_code_t)HCI_ULP | 0x000A)
#define HCI_ULP_SET_SCAN_PARAMETERS                       ((hci_op_code_t)HCI_ULP | 0x000B)
#define HCI_ULP_SET_SCAN_ENABLE                           ((hci_op_code_t)HCI_ULP | 0x000C)
#define HCI_ULP_CREATE_CONNECTION                         ((hci_op_code_t)HCI_ULP | 0x000D)
#define HCI_ULP_CREATE_CONNECTION_CANCEL                  ((hci_op_code_t)HCI_ULP | 0x000E)
#define HCI_ULP_READ_WHITE_LIST_SIZE                      ((hci_op_code_t)HCI_ULP | 0x000F)
#define HCI_ULP_CLEAR_WHITE_LIST                          ((hci_op_code_t)HCI_ULP | 0x0010)
#define HCI_ULP_ADD_DEVICE_TO_WHITE_LIST                  ((hci_op_code_t)HCI_ULP | 0x0011)
#define HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST             ((hci_op_code_t)HCI_ULP | 0x0012)
#define HCI_ULP_CONNECTION_UPDATE                         ((hci_op_code_t)HCI_ULP | 0x0013)
#define HCI_ULP_SET_HOST_CHANNEL_CLASSIFICATION           ((hci_op_code_t)HCI_ULP | 0x0014)
#define HCI_ULP_READ_CHANNEL_MAP                          ((hci_op_code_t)HCI_ULP | 0x0015)
#define HCI_ULP_READ_REMOTE_USED_FEATURES                 ((hci_op_code_t)HCI_ULP | 0x0016)
#define HCI_ULP_ENCRYPT                                   ((hci_op_code_t)HCI_ULP | 0x0017)
#define HCI_ULP_RAND                                      ((hci_op_code_t)HCI_ULP | 0x0018)
#define HCI_ULP_START_ENCRYPTION                          ((hci_op_code_t)HCI_ULP | 0x0019)
#define HCI_ULP_LONG_TERM_KEY_REQUEST_REPLY               ((hci_op_code_t)HCI_ULP | 0x001A)
#define HCI_ULP_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY      ((hci_op_code_t)HCI_ULP | 0x001B)
#define HCI_ULP_READ_SUPPORTED_STATES                     ((hci_op_code_t)HCI_ULP | 0x001C)
#define HCI_ULP_RECEIVER_TEST                             ((hci_op_code_t)HCI_ULP | 0x001D)
#define HCI_ULP_TRANSMITTER_TEST                          ((hci_op_code_t)HCI_ULP | 0x001E)
#define HCI_ULP_TEST_END                                  ((hci_op_code_t)HCI_ULP | 0x001F)
#define HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY ((hci_op_code_t)HCI_ULP | 0x0020)
#define HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY ((hci_op_code_t)HCI_ULP | 0x0021)
/* DLE */
#define HCI_ULP_SET_DATA_LENGTH                           ((hci_op_code_t)HCI_ULP | 0x0022)
#define HCI_ULP_READ_SUGGESTED_DEFAULT_DATA_LENGTH        ((hci_op_code_t)HCI_ULP | 0x0023)
#define HCI_ULP_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH       ((hci_op_code_t)HCI_ULP | 0x0024)

#define HCI_ULP_READ_LOCAL_P256_PUBLIC_KEY                ((hci_op_code_t)HCI_ULP | 0x0025)
#define HCI_ULP_GENERATE_DHKEY                            ((hci_op_code_t)HCI_ULP | 0x0026)
#define HCI_ULP_ADD_DEVICE_TO_RESOLVING_LIST              ((hci_op_code_t)HCI_ULP | 0x0027)
#define HCI_ULP_REMOVE_DEVICE_FROM_RESOLVING_LIST         ((hci_op_code_t)HCI_ULP | 0x0028)
#define HCI_ULP_CLEAR_RESOLVING_LIST                      ((hci_op_code_t)HCI_ULP | 0x0029)
#define HCI_ULP_READ_RESOLVING_LIST_SIZE                  ((hci_op_code_t)HCI_ULP | 0x002A)
#define HCI_ULP_READ_PEER_RESOLVABLE_ADDRESS              ((hci_op_code_t)HCI_ULP | 0x002B)
#define HCI_ULP_READ_LOCAL_RESOLVABLE_ADDRESS             ((hci_op_code_t)HCI_ULP | 0x002C)
#define HCI_ULP_SET_ADDRESS_RESOLUTION_ENABLE             ((hci_op_code_t)HCI_ULP | 0x002D)
#define HCI_ULP_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT    ((hci_op_code_t)HCI_ULP | 0x002E)

/* HCI commands will be added here for LE Secure Connections and Privacy */

/* DLE */
#define HCI_ULP_READ_MAXIMUM_DATA_LENGTH                  ((hci_op_code_t)HCI_ULP | 0x002f)

/* 2Mbit LE */
#define HCI_ULP_READ_PHY                                  ((hci_op_code_t)HCI_ULP | 0x0030)      
#define HCI_ULP_SET_DEFAULT_PHY                           ((hci_op_code_t)HCI_ULP | 0x0031) 
#define HCI_ULP_SET_PHY                                   ((hci_op_code_t)HCI_ULP | 0x0032) 
#define HCI_ULP_ENHANCED_RECEIVER_TEST                    ((hci_op_code_t)HCI_ULP | 0x0033)
#define HCI_ULP_ENHANCED_TRANSMITTER_TEST                 ((hci_op_code_t)HCI_ULP | 0x0034)

#define HCI_ULP_SET_PRIVACY_MODE                           ((hci_op_code_t)HCI_ULP | 0x004E)

#define HCI_MAX_ULP_OCF                                   ((hci_op_code_t)0x004F)

/******************************************************************************
        Event defines - these are events as per the HCI definition
 *****************************************************************************/
#define HCI_EV_INQUIRY_COMPLETE                      ((hci_event_code_t)0x01)
#define HCI_EV_INQUIRY_RESULT                        ((hci_event_code_t)0x02)
#define HCI_EV_CONN_COMPLETE                         ((hci_event_code_t)0x03)
#define HCI_EV_CONN_REQUEST                          ((hci_event_code_t)0x04)
#define HCI_EV_DISCONNECT_COMPLETE                   ((hci_event_code_t)0x05)
#define HCI_EV_AUTH_COMPLETE                         ((hci_event_code_t)0x06)
#define HCI_EV_REMOTE_NAME_REQ_COMPLETE              ((hci_event_code_t)0x07)
#define HCI_EV_ENCRYPTION_CHANGE                     ((hci_event_code_t)0x08)
#define HCI_EV_CHANGE_CONN_LINK_KEY_COMPLETE         ((hci_event_code_t)0x09)
#define HCI_EV_MASTER_LINK_KEY_COMPLETE              ((hci_event_code_t)0x0A)
#define HCI_EV_READ_REM_SUPP_FEATURES_COMPLETE       ((hci_event_code_t)0x0B)
#define HCI_EV_READ_REMOTE_VER_INFO_COMPLETE         ((hci_event_code_t)0x0C)
#define HCI_EV_QOS_SETUP_COMPLETE                    ((hci_event_code_t)0x0D)
#define HCI_EV_COMMAND_COMPLETE                      ((hci_event_code_t)0x0E)
#define HCI_EV_COMMAND_STATUS                        ((hci_event_code_t)0x0F)
#define HCI_EV_HARDWARE_ERROR                        ((hci_event_code_t)0x10)
#define HCI_EV_FLUSH_OCCURRED                        ((hci_event_code_t)0x11)
#define HCI_EV_ROLE_CHANGE                           ((hci_event_code_t)0x12)
#define HCI_EV_NUMBER_COMPLETED_PKTS                 ((hci_event_code_t)0x13)
#define HCI_EV_MODE_CHANGE                           ((hci_event_code_t)0x14)
#define HCI_EV_RETURN_LINK_KEYS                      ((hci_event_code_t)0x15)
#define HCI_EV_PIN_CODE_REQ                          ((hci_event_code_t)0x16)
#define HCI_EV_LINK_KEY_REQ                          ((hci_event_code_t)0x17)
#define HCI_EV_LINK_KEY_NOTIFICATION                 ((hci_event_code_t)0x18)
#define HCI_EV_LOOPBACK_COMMAND                      ((hci_event_code_t)0x19)
#define HCI_EV_DATA_BUFFER_OVERFLOW                  ((hci_event_code_t)0x1A)
#define HCI_EV_MAX_SLOTS_CHANGE                      ((hci_event_code_t)0x1B)
#define HCI_EV_READ_CLOCK_OFFSET_COMPLETE            ((hci_event_code_t)0x1C)
#define HCI_EV_CONN_PACKET_TYPE_CHANGED              ((hci_event_code_t)0x1D)
#define HCI_EV_QOS_VIOLATION                         ((hci_event_code_t)0x1E)
#define HCI_EV_PAGE_SCAN_MODE_CHANGE                 ((hci_event_code_t)0x1F)
#define HCI_EV_PAGE_SCAN_REP_MODE_CHANGE             ((hci_event_code_t)0x20)
/* 1.2 Events */
#define HCI_EV_FLOW_SPEC_COMPLETE                    ((hci_event_code_t)0x21)
#define HCI_EV_INQUIRY_RESULT_WITH_RSSI              ((hci_event_code_t)0x22)
#define HCI_EV_READ_REM_EXT_FEATURES_COMPLETE        ((hci_event_code_t)0x23)
#define HCI_EV_FIXED_ADDRESS                         ((hci_event_code_t)0x24)
#define HCI_EV_ALIAS_ADDRESS                         ((hci_event_code_t)0x25)
#define HCI_EV_GENERATE_ALIAS_REQ                    ((hci_event_code_t)0x26)
#define HCI_EV_ACTIVE_ADDRESS                        ((hci_event_code_t)0x27)
#define HCI_EV_ALLOW_PRIVATE_PAIRING                 ((hci_event_code_t)0x28)
#define HCI_EV_ALIAS_ADDRESS_REQ                     ((hci_event_code_t)0x29)
#define HCI_EV_ALIAS_NOT_RECOGNISED                  ((hci_event_code_t)0x2A)
#define HCI_EV_FIXED_ADDRESS_ATTEMPT                 ((hci_event_code_t)0x2B)
#define HCI_EV_SYNC_CONN_COMPLETE                    ((hci_event_code_t)0x2C)
#define HCI_EV_SYNC_CONN_CHANGED                     ((hci_event_code_t)0x2D)

/* 2.1 Events */
#define HCI_EV_SNIFF_SUB_RATE                        ((hci_event_code_t)0x2E)
#define HCI_EV_EXTENDED_INQUIRY_RESULT               ((hci_event_code_t)0x2F)
#define HCI_EV_ENCRYPTION_KEY_REFRESH_COMPLETE       ((hci_event_code_t)0x30)
#define HCI_EV_IO_CAPABILITY_REQUEST                 ((hci_event_code_t)0x31)
#define HCI_EV_IO_CAPABILITY_RESPONSE                ((hci_event_code_t)0x32)
#define HCI_EV_USER_CONFIRMATION_REQUEST             ((hci_event_code_t)0x33)
#define HCI_EV_USER_PASSKEY_REQUEST                  ((hci_event_code_t)0x34)
#define HCI_EV_REMOTE_OOB_DATA_REQUEST               ((hci_event_code_t)0x35)
#define HCI_EV_SIMPLE_PAIRING_COMPLETE               ((hci_event_code_t)0x36)
#define HCI_EV_LST_CHANGE                            ((hci_event_code_t)0x38)
#define HCI_EV_ENHANCED_FLUSH_COMPLETE               ((hci_event_code_t)0x39)
#define HCI_EV_USER_PASSKEY_NOTIFICATION             ((hci_event_code_t)0x3B)
#define HCI_EV_KEYPRESS_NOTIFICATION                 ((hci_event_code_t)0x3C)
#define HCI_EV_REM_HOST_SUPPORTED_FEATURES           ((hci_event_code_t)0x3D)

#define HCI_MAX_BREDR_EVENT_OPCODE_V4_0              ((hci_event_code_t)0x3E)

/* TCC + CSB Events */
#define HCI_EV_TRIGGERED_CLOCK_CAPTURE               ((hci_event_code_t)0x4E)
#define HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE        ((hci_event_code_t)0x4F)
#define HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED        ((hci_event_code_t)0x50)
#define HCI_EV_CSB_RECEIVE                           ((hci_event_code_t)0x51)
#define HCI_EV_CSB_TIMEOUT                           ((hci_event_code_t)0x52)
#define HCI_EV_TRUNCATED_PAGE_COMPLETE               ((hci_event_code_t)0x53)
#define HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT           ((hci_event_code_t)0x54)
#define HCI_EV_CSB_CHANNEL_MAP_CHANGE                ((hci_event_code_t)0x55)
#define HCI_EV_INQUIRY_RESPONSE_NOTIFICATION         ((hci_event_code_t)0x56)

/* 4.1 Events */
#define HCI_EV_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED ((hci_event_code_t)0x57)

#define HCI_MAX_EVENT_OPCODE                         ((hci_event_code_t)0x58)

/* ULP main event opcode*/
#define HCI_EV_ULP                              ((uint8_t)0x3E)

/* Sub-opcodes */
#define HCI_EV_ULP_CONNECTION_COMPLETE                 ((hci_event_code_t)0x01)
#define HCI_EV_ULP_ADVERTISING_REPORT                  ((hci_event_code_t)0x02)
#define HCI_EV_ULP_CONNECTION_UPDATE_COMPLETE          ((hci_event_code_t)0x03)
#define HCI_EV_ULP_READ_REMOTE_USED_FEATURES_COMPLETE  ((hci_event_code_t)0x04)
#define HCI_EV_ULP_LONG_TERM_KEY_REQUEST               ((hci_event_code_t)0x05)
#define HCI_EV_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST ((hci_event_code_t)0x06)
#define HCI_EV_ULP_DATA_LENGTH_CHANGE                  ((hci_event_code_t)0x07)
#define HCI_EV_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE    ((hci_event_code_t)0x08)
#define HCI_EV_ULP_GENERATE_DHKEY_COMPLETE             ((hci_event_code_t)0x09)
#define HCI_EV_ULP_ENHANCED_CONNECTION_COMPLETE        ((hci_event_code_t)0x0A)
#define HCI_EV_ULP_DIRECT_ADVERTISING_REPORT           ((hci_event_code_t)0x0B)
#define HCI_EV_ULP_PHY_UPDATE_COMPLETE                 ((hci_event_code_t)0x0C) 
#define HCI_EV_ULP_CHANNEL_SELECTION_ALGORITHM         ((hci_event_code_t)0x14) 

/*
 * Note:
 * The HCI_EV_MANUFACTURER_EXTENSION definition below, uses uint8_t instead of
 * hci_event_code_t, as we do not wish the auto hci generating code
 * to take this number as the last element in the event primitives
 * hence creating 0xFF locations when only HCI_MAX_EVENT_OPCODE will be used.
 */
#define HCI_EV_MANUFACTURER_EXTENSION           ((uint8_t)0xFF)

/******************************************************************************
   OP CODE define - Vendor Specific events
 *****************************************************************************/
#define HCI_CSB_AFH_MAP_AVAILABLE          ((hci_event_code_t)0x01)
/* New Vendor events add here */

/******************************************************************************
        HCI_COMMAND_COMPLETE defines - these are events as per the HCI definition
 *****************************************************************************/

/******************************************************************************
   OCF opcode defines - Link Control Commands
 *****************************************************************************/
#define HCI_CREATE_CONNECTION_CANCEL_RET   ((hci_op_code_t)HCI_LINK_RET | 0x0008)
#define HCI_LINK_KEY_REQ_REPLY_RET         ((hci_op_code_t)HCI_LINK_RET | 0x000B)
#define HCI_LINK_KEY_REQ_NEG_REPLY_RET     ((hci_op_code_t)HCI_LINK_RET | 0x000C)
#define HCI_PIN_CODE_REQ_REPLY_RET         ((hci_op_code_t)HCI_LINK_RET | 0x000D)
#define HCI_PIN_CODE_REQ_NEG_REPLY_RET     ((hci_op_code_t)HCI_LINK_RET | 0x000E)
#define HCI_REMOTE_NAME_REQ_CANCEL_RET     ((hci_op_code_t)HCI_LINK_RET | 0x001A)
#define HCI_MAX_LINK_RET_OCF               ((hci_op_code_t) 0x000F)

/******************************************************************************
   OP CODE defines - Link Policy Commands
 *****************************************************************************/
#define HCI_ROLE_DISCOVERY_RET             ((hci_op_code_t)HCI_POLICY_RET | 0x0009)
#define HCI_READ_LINK_POLICY_SETTINGS_RET  ((hci_op_code_t)HCI_POLICY_RET | 0x000C)
#define HCI_WRITE_LINK_POLICY_SETTINGS_RET ((hci_op_code_t)HCI_POLICY_RET | 0x000D)
#define HCI_MAX_POLICY_RET_OCF             ((hci_op_code_t) 0x000E)

/******************************************************************************
   OP CODE defines - Host Controller and Baseband Commands
 *****************************************************************************/
#define HCI_FLUSH_RET                      ((hci_op_code_t)HCI_HOST_BB_RET | 0x0008)
#define HCI_READ_PIN_TYPE_RET              ((hci_op_code_t)HCI_HOST_BB_RET | 0x0009)
#define HCI_READ_STORED_LINK_KEY_RET       ((hci_op_code_t)HCI_HOST_BB_RET | 0x000D)
#define HCI_WRITE_STORED_LINK_KEY_RET      ((hci_op_code_t)HCI_HOST_BB_RET | 0x0011)
#define HCI_DELETE_STORED_LINK_KEY_RET     ((hci_op_code_t)HCI_HOST_BB_RET | 0x0012)
#define HCI_READ_LOCAL_NAME_RET            ((hci_op_code_t)HCI_HOST_BB_RET | 0x0014)
#define HCI_READ_CONN_ACCEPT_TIMEOUT_RET   ((hci_op_code_t)HCI_HOST_BB_RET | 0x0015)
#define HCI_READ_PAGE_TIMEOUT_RET          ((hci_op_code_t)HCI_HOST_BB_RET | 0x0017)
#define HCI_READ_SCAN_ENABLE_RET           ((hci_op_code_t)HCI_HOST_BB_RET | 0x0019)
#define HCI_READ_PAGESCAN_ACTIVITY_RET     ((hci_op_code_t)HCI_HOST_BB_RET | 0x001B)
#define HCI_READ_INQUIRYSCAN_ACTIVITY_RET  ((hci_op_code_t)HCI_HOST_BB_RET | 0x001D)
#define HCI_READ_AUTH_ENABLE_RET           ((hci_op_code_t)HCI_HOST_BB_RET | 0x001F)
#define HCI_READ_ENC_MODE_RET              ((hci_op_code_t)HCI_HOST_BB_RET | 0x0021)
#define HCI_READ_CLASS_OF_DEVICE_RET       ((hci_op_code_t)HCI_HOST_BB_RET | 0x0023)
#define HCI_READ_VOICE_SETTING_RET         ((hci_op_code_t)HCI_HOST_BB_RET | 0x0025)
#define HCI_READ_AUTO_FLUSH_TIMEOUT_RET    ((hci_op_code_t)HCI_HOST_BB_RET | 0x0027)
#define HCI_WRITE_AUTO_FLUSH_TIMEOUT_RET   ((hci_op_code_t)HCI_HOST_BB_RET | 0x0028)
#define HCI_READ_NUM_BCAST_RETXS_RET       ((hci_op_code_t)HCI_HOST_BB_RET | 0x0029)
#define HCI_READ_HOLD_MODE_ACTIVITY_RET    ((hci_op_code_t)HCI_HOST_BB_RET | 0x002B)
#define HCI_READ_TX_POWER_LEVEL_RET        ((hci_op_code_t)HCI_HOST_BB_RET | 0x002D)
#define HCI_READ_SCO_FLOW_CON_ENABLE_RET   ((hci_op_code_t)HCI_HOST_BB_RET | 0x002E)
#define HCI_READ_LINK_SUPERV_TIMEOUT_RET   ((hci_op_code_t)HCI_HOST_BB_RET | 0x0036)
#define HCI_WRITE_LINK_SUPERV_TIMEOUT_RET  ((hci_op_code_t)HCI_HOST_BB_RET | 0x0037)
#define HCI_READ_NUM_SUPPORTED_IAC_RET     ((hci_op_code_t)HCI_HOST_BB_RET | 0x0038)
#define HCI_READ_CURRENT_IAC_LAP_RET       ((hci_op_code_t)HCI_HOST_BB_RET | 0x0039)
#define HCI_READ_PAGESCAN_PERIOD_MODE_RET  ((hci_op_code_t)HCI_HOST_BB_RET | 0x003B)
#define HCI_READ_PAGESCAN_MODE_RET         ((hci_op_code_t)HCI_HOST_BB_RET | 0x003D)
#define HCI_READ_AFH_CHANNEL_CLASS_M_RET   ((hci_op_code_t)HCI_HOST_BB_RET | 0x0048)
#define HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_RET   ((hci_op_code_t)HCI_HOST_BB_RET | 0x0051)
#define HCI_READ_LOCAL_OOB_DATA_RET        ((hci_op_code_t)HCI_HOST_BB_RET | 0x0057)
#define HCI_READ_ENH_TX_POWER_LEVEL_RET    ((hci_op_code_t)HCI_HOST_BB_RET | 0x0068)
#define HCI_READ_LE_HOST_SUPPORT_RET       ((hci_op_code_t)HCI_HOST_BB_RET | 0x006C)
#define HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT_RET    ((hci_op_code_t)HCI_HOST_BB_RET | 0x0079)
#define HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_RET      ((hci_op_code_t)HCI_HOST_BB_RET | 0x007B)
#define HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_RET     ((hci_op_code_t)HCI_HOST_BB_RET | 0x007C)
#define HCI_READ_LOCAL_OOB_EXTENDED_DATA_RET        ((hci_op_code_t)HCI_HOST_BB_RET | 0x007D)
#define HCI_MAX_HOST_BB_RET_OCF            ((hci_op_code_t) 0x0075)

/******************************************************************************
   OP CODE defines - Informational Parameters
 *****************************************************************************/
#define HCI_READ_LOCAL_VER_INFO_RET        ((hci_op_code_t)HCI_INFO_RET | 0x0001)
#define HCI_READ_LOCAL_SUPP_COMMANDS_RET   ((hci_op_code_t)HCI_INFO_RET | 0x0002)
#define HCI_READ_LOCAL_SUPP_FEATURES_RET   ((hci_op_code_t)HCI_INFO_RET | 0x0003)
#define HCI_READ_LOCAL_EXT_FEATURES_RET    ((hci_op_code_t)HCI_INFO_RET | 0x0004)
#define HCI_READ_BUFFER_SIZE_RET           ((hci_op_code_t)HCI_INFO_RET | 0x0005)
#define HCI_READ_COUNTRY_CODE_RET          ((hci_op_code_t)HCI_INFO_RET | 0x0007)
#define HCI_READ_BD_ADDR_RET               ((hci_op_code_t)HCI_INFO_RET | 0x0009)
#define HCI_READ_LOCAL_SUPP_CODECS_RET     ((hci_op_code_t)HCI_INFO_RET | 0x000B)
#define HCI_MAX_INFO_RET_OCF               ((hci_op_code_t) 0x000C)

/******************************************************************************
   OP CODE defines - Status Parameters
 *****************************************************************************/
#define HCI_READ_FAILED_CONTACT_COUNT_RET  ((hci_op_code_t)HCI_STATUS_RET | 0x0001)
#define HCI_RESET_FAILED_CONTACT_COUNT_RET ((hci_op_code_t)HCI_STATUS_RET | 0x0002)
#define HCI_GET_LINK_QUALITY_RET           ((hci_op_code_t)HCI_STATUS_RET | 0x0003)
#define HCI_READ_RSSI_RET                  ((hci_op_code_t)HCI_STATUS_RET | 0x0005)
#define HCI_READ_AFH_CHANNEL_MAP_RET       ((hci_op_code_t)HCI_STATUS_RET | 0x0006)
#define HCI_READ_CLOCK_RET                 ((hci_op_code_t)HCI_STATUS_RET | 0x0007)
#define HCI_READ_ENCRYPTION_KEY_SIZE_RET   ((hci_op_code_t)HCI_STATUS_RET | 0x0008)
#define HCI_MAX_STATUS_RET_OCF             ((hci_op_code_t) 0x0009)

/******************************************************************************
   OP CODE defines - Testing Parameters
 *****************************************************************************/
#define HCI_READ_LOOPBACK_MODE_RET         ((hci_op_code_t)HCI_TEST_RET | 0x0001)
#define HCI_WRITE_LOOPBACK_MODE_RET        ((hci_op_code_t)HCI_TEST | 0x0002)
#define HCI_ENABLE_DUT_MODE_RET            ((hci_op_code_t)HCI_TEST | 0x0003)
#define HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE_RET ((hci_op_code_t)HCI_TEST | 0x0004)
#define HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE_RET ((hci_op_code_t)HCI_TEST_RET | 0x000A)
#define HCI_MAX_TEST_RET_OCF               ((hci_op_code_t) 0x000B)

/******************************************************************************
   HCI_COMMAND, Argument Length Definitions.
 *****************************************************************************/
/* Link Control (HCI_LINK) */
#define HCI_INQUIRY_PARAM_LEN                          ((uint8_t)  5)
#define HCI_INQUIRY_CANCEL_PARAM_LEN                   ((uint8_t)  0)
#define HCI_PERIODIC_INQUIRY_MODE_PARAM_LEN            ((uint8_t)  9)
#define HCI_EXIT_PERIODIC_INQUIRY_MODE_PARAM_LEN       ((uint8_t)  0)
#define HCI_CREATE_CONNECTION_PARAM_LEN                ((uint8_t) 13)
#define HCI_DISCONNECT_PARAM_LEN                       ((uint8_t)  3)
#define HCI_ADD_SCO_CONNECTION_PARAM_LEN               ((uint8_t)  4)
#define HCI_CREATE_CONNECTION_CANCEL_PARAM_LEN         ((uint8_t)  6)
#define HCI_ACCEPT_CONNECTION_REQ_PARAM_LEN            ((uint8_t)  7)
#define HCI_REJECT_CONNECTION_REQ_PARAM_LEN            ((uint8_t)  7)
#define HCI_LINK_KEY_REQ_REPLY_PARAM_LEN               ((uint8_t) 22)
#define HCI_LINK_KEY_REQ_NEG_REPLY_PARAM_LEN           ((uint8_t)  6)
#define HCI_PIN_CODE_REQ_REPLY_PARAM_LEN               ((uint8_t) 23)
#define HCI_PIN_CODE_REQ_NEG_REPLY_PARAM_LEN           ((uint8_t)  6)
#define HCI_CHANGE_CONN_PKT_TYPE_PARAM_LEN             ((uint8_t)  4)
#define HCI_AUTH_REQ_PARAM_LEN                         ((uint8_t)  2)
#define HCI_SET_CONN_ENCRYPTION_PARAM_LEN              ((uint8_t)  3)
#define HCI_CHANGE_CONN_LINK_KEY_PARAM_LEN             ((uint8_t)  2)
#define HCI_MASTER_LINK_KEY_PARAM_LEN                  ((uint8_t)  1)
#define HCI_REMOTE_NAME_REQ_PARAM_LEN                  ((uint8_t) 10)
#define HCI_REMOTE_NAME_REQ_CANCEL_PARAM_LEN           ((uint8_t)  6)
#define HCI_READ_REMOTE_SUPP_FEATURES_PARAM_LEN        ((uint8_t)  2)
#define HCI_READ_REMOTE_EXT_FEATURES_PARAM_LEN         ((uint8_t)  3)
#define HCI_READ_REMOTE_VER_INFO_PARAM_LEN             ((uint8_t)  2)
#define HCI_READ_CLOCK_OFFSET_PARAM_LEN                ((uint8_t)  2)
#define HCI_READ_LMP_HANDLE_PARAM_LEN                  ((uint8_t)  2)
#define HCI_EXCHANGE_FIXED_INFO_PARAM_LEN              ((uint8_t)  2)
#define HCI_EXCHANGE_ALIAS_INFO_PARAM_LEN              ((uint8_t)  8)
#define HCI_PRIVATE_PAIRING_REQ_REPLY_PARAM_LEN        ((uint8_t)  2)
#define HCI_PRIVATE_PAIRING_REQ_NEG_REPLY_PARAM_LEN    ((uint8_t)  2)
#define HCI_GENERATED_ALIAS_PARAM_LEN                  ((uint8_t)  8)
#define HCI_ALIAS_ADDRESS_REQ_REPLY_PARAM_LEN          ((uint8_t) 12)
#define HCI_ALIAS_ADDRESS_REQ_NEG_REPLY_PARAM_LEN      ((uint8_t)  6)
#define HCI_SETUP_SYNCHRONOUS_CONN_PARAM_LEN           ((uint8_t) 17)
#define HCI_ACCEPT_SYNCHRONOUS_CONN_REQ_PARAM_LEN      ((uint8_t) 21)
#define HCI_REJECT_SYNCHRONOUS_CONN_REQ_PARAM_LEN      ((uint8_t)  7)
#define HCI_IO_CAPABILITY_REQUEST_REPLY_LEN            ((uint8_t)  9)
#define HCI_USER_CONFIRMATION_REQUEST_REPLY_LEN        ((uint8_t)  6)
#define HCI_USER_CONFIRMATION_REQEST_NEG_REPLY_LEN     ((uint8_t)  6)
#define HCI_USER_PASSKEY_REQUEST_REPLY_LEN             ((uint8_t) 10)
#define HCI_USER_PASSKEY_REQUEST_NEG_REPLY_LEN         ((uint8_t)  6)
#define HCI_REMOTE_OOB_DATA_REQUEST_REPLY_LEN          ((uint8_t) 38)
#define HCI_REMOTE_OOB_DATA_REQUEST_NEG_REPLY_LEN      ((uint8_t)  6)
#define HCI_IO_CAPABILITY_REQUEST_NEG_REPLY_LEN        ((uint8_t)  7)
#define HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY_LEN ((uint8_t) 70)
#define HCI_TRUNCATED_PAGE_PARAM_LEN                   ((uint8_t) 9)
#define HCI_TRUNCATED_PAGE_CANCEL_PARAM_LEN            ((uint8_t) 6)
#define HCI_START_SYNCHRONIZATION_TRAIN_PARAM_LEN      ((uint8_t) 0)
#define HCI_SET_CSB_PARAM_LEN                          ((uint8_t) 11)
#define HCI_RECEIVE_SYNCHRONIZATION_TRAIN_PARAM_LEN    ((uint8_t) 12)
#define HCI_SET_CSB_RECEIVE_PARAM_LEN                  ((uint8_t) 34)
#define HCI_ENHANCED_SETUP_SYNC_CONN_PARAM_LEN         ((uint8_t) 59)
#define HCI_ENHANCED_ACCEPT_SYNC_CONN_REQ_PARAM_LEN    ((uint8_t) 63)

/* Link Policy (HCI_POLICY) */
#define HCI_HOLD_MODE_PARAM_LEN                  ((uint8_t) 6)
#define HCI_SNIFF_MODE_PARAM_LEN                 ((uint8_t) 10)
#define HCI_EXIT_SNIFF_MODE_PARAM_LEN            ((uint8_t) 2)
#define HCI_QOS_SETUP_PARAM_LEN                  ((uint8_t) 20)
#define HCI_ROLE_DISCOVERY_PARAM_LEN             ((uint8_t) 2)
#define HCI_SWITCH_ROLE_PARAM_LEN                ((uint8_t) 7)
#define HCI_READ_LINK_POLICY_SETTINGS_PARAM_LEN  ((uint8_t) 2)
#define HCI_WRITE_LINK_POLICY_SETTINGS_PARAM_LEN ((uint8_t) 4)
#define HCI_READ_DEFAULT_LINK_POLICY_SETTINGS_PARAM_LEN  ((uint8_t) 0)
#define HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_PARAM_LEN ((uint8_t) 2)
#define HCI_FLOW_SPEC_PARAM_LEN                  ((uint8_t) 21)
#define HCI_SNIFF_SUB_RATE_PARAM_LEN             ((uint8_t) 8)

/* Controller And Baseband (HCI_HOST_BB) */
#define HCI_SET_EVENT_MASK_PARAM_LEN             ((uint8_t) 8)
#define HCI_RESET_PARAM_LEN                      ((uint8_t) 0)
#define HCI_SET_EVENT_FILTER_PARAM_LEN           ((uint8_t) 8) /* Variable */
#define HCI_FLUSH_PARAM_LEN                      ((uint8_t) 2)
#define HCI_READ_PIN_TYPE_PARAM_LEN              ((uint8_t) 0)
#define HCI_WRITE_PIN_TYPE_PARAM_LEN             ((uint8_t) 1)
#define HCI_CREATE_NEW_UNIT_KEY_PARAM_LEN        ((uint8_t) 0)
#define HCI_READ_STORED_LINK_KEY_PARAM_LEN       ((uint8_t) 7)
#define HCI_WRITE_STORED_LINK_KEY_PARAM_LEN      ((uint8_t) 23) /* Variable */
#define HCI_DELETE_STORED_LINK_KEY_PARAM_LEN     ((uint8_t) 7)
#define HCI_CHANGE_LOCAL_NAME_PARAM_LEN          ((uint8_t) 248)
#define HCI_READ_LOCAL_NAME_PARAM_LEN            ((uint8_t) 0)
#define HCI_READ_CONN_ACCEPT_TIMEOUT_PARAM_LEN   ((uint8_t) 0)
#define HCI_WRITE_CONN_ACCEPT_TIMEOUT_PARAM_LEN  ((uint8_t) 2)
#define HCI_READ_PAGE_TIMEOUT_PARAM_LEN          ((uint8_t) 0)
#define HCI_WRITE_PAGE_TIMEOUT_PARAM_LEN         ((uint8_t) 2)
#define HCI_READ_SCAN_ENABLE_PARAM_LEN           ((uint8_t) 0)
#define HCI_WRITE_SCAN_ENABLE_PARAM_LEN          ((uint8_t) 1)
#define HCI_READ_PAGESCAN_ACTIVITY_PARAM_LEN     ((uint8_t) 0)
#define HCI_WRITE_PAGESCAN_ACTIVITY_PARAM_LEN    ((uint8_t) 4)
#define HCI_READ_INQUIRYSCAN_ACTIVITY_PARAM_LEN  ((uint8_t) 0)
#define HCI_WRITE_INQUIRYSCAN_ACTIVITY_PARAM_LEN ((uint8_t) 4)
#define HCI_READ_AUTH_ENABLE_PARAM_LEN           ((uint8_t) 0)
#define HCI_WRITE_AUTH_ENABLE_PARAM_LEN          ((uint8_t) 1)
#define HCI_READ_ENC_MODE_PARAM_LEN              ((uint8_t) 0)
#define HCI_WRITE_ENC_MODE_PARAM_LEN             ((uint8_t) 1)
#define HCI_READ_CLASS_OF_DEVICE_PARAM_LEN       ((uint8_t) 0)
#define HCI_WRITE_CLASS_OF_DEVICE_PARAM_LEN      ((uint8_t) 3)
#define HCI_READ_VOICE_SETTING_PARAM_LEN         ((uint8_t) 0)
#define HCI_WRITE_VOICE_SETTING_PARAM_LEN        ((uint8_t) 2)
#define HCI_READ_AUTO_FLUSH_TIMEOUT_PARAM_LEN    ((uint8_t) 2)
#define HCI_WRITE_AUTO_FLUSH_TIMEOUT_PARAM_LEN   ((uint8_t) 4)
#define HCI_READ_NUM_BCAST_RETXS_PARAM_LEN       ((uint8_t) 0)
#define HCI_WRITE_NUM_BCAST_RETXS_PARAM_LEN      ((uint8_t) 1)
#define HCI_READ_HOLD_MODE_ACTIVITY_PARAM_LEN    ((uint8_t) 0)
#define HCI_WRITE_HOLD_MODE_ACTIVITY_PARAM_LEN   ((uint8_t) 1)
#define HCI_READ_TX_POWER_LEVEL_PARAM_LEN        ((uint8_t) 3)
#define HCI_READ_SCO_FLOW_CON_ENABLE_PARAM_LEN   ((uint8_t) 0)
#define HCI_WRITE_SCO_FLOW_CON_ENABLE_PARAM_LEN  ((uint8_t) 1)
#define HCI_SET_HCTOHOST_FLOW_CONTROL_PARAM_LEN  ((uint8_t) 1)
#define HCI_HOST_BUFFER_SIZE_PARAM_LEN           ((uint8_t) 7)
#define HCI_HOST_NUM_COMPLETED_PACKETS_PARAM_LEN ((uint8_t) 5) /* Variable */
#define HCI_READ_LINK_SUPERV_TIMEOUT_PARAM_LEN   ((uint8_t) 2)
#define HCI_WRITE_LINK_SUPERV_TIMEOUT_PARAM_LEN  ((uint8_t) 4)
#define HCI_READ_NUM_SUPPORTED_IAC_PARAM_LEN     ((uint8_t) 0)
#define HCI_READ_CURRENT_IAC_LAP_PARAM_LEN       ((uint8_t) 0)
#define HCI_WRITE_CURRENT_IAC_LAP_PARAM_LEN      ((uint8_t) 4) /* Variable */
#define HCI_READ_PAGESCAN_PERIOD_MODE_PARAM_LEN  ((uint8_t) 0)
#define HCI_WRITE_PAGESCAN_PERIOD_MODE_PARAM_LEN ((uint8_t) 1)
#define HCI_READ_PAGESCAN_MODE_PARAM_LEN         ((uint8_t) 0)
#define HCI_WRITE_PAGESCAN_MODE_PARAM_LEN        ((uint8_t) 1)
#define HCI_SET_AFH_CHANNEL_CLASS_PARAM_LEN      ((uint8_t) 10)
#define HCI_READ_INQUIRY_SCAN_TYPE_PARAM_LEN     ((uint8_t) 0)
#define HCI_WRITE_INQUIRY_SCAN_TYPE_PARAM_LEN    ((uint8_t) 1)
#define HCI_READ_INQUIRY_MODE_PARAM_LEN          ((uint8_t) 0)
#define HCI_WRITE_INQUIRY_MODE_PARAM_LEN         ((uint8_t) 1)
#define HCI_READ_PAGE_SCAN_TYPE_PARAM_LEN        ((uint8_t) 0)
#define HCI_WRITE_PAGE_SCAN_TYPE_PARAM_LEN       ((uint8_t) 1)
#define HCI_READ_AFH_CHANNEL_CLASS_M_PARAM_LEN   ((uint8_t) 0)
#define HCI_WRITE_AFH_CHANNEL_CLASS_M_PARAM_LEN  ((uint8_t) 1)
#define HCI_READ_ANON_MODE_PARAM_LEN             ((uint8_t) 0)
#define HCI_WRITE_ANON_MODE_PARAM_LEN            ((uint8_t) 1)
#define HCI_READ_ALIAS_AUTH_ENABLE_PARAM_LEN     ((uint8_t) 0)
#define HCI_WRITE_ALIAS_AUTH_ENABLE_PARAM_LEN    ((uint8_t) 1)
#define HCI_READ_ANON_ADDR_CHANGE_PARAMS_PARAM_LEN         ((uint8_t) 0)
#define HCI_WRITE_ANON_ADDR_CHANGE_PARAMS_PARAM_LEN        ((uint8_t) 6)
#define HCI_RESET_FIXED_ADDRESS_ATTEMPTS_COUNTER_PARAM_LEN ((uint8_t) 1)
#define HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_PARAM_LEN  ((uint8_t) 0)
#define HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA_PARAM_LEN ((uint8_t) 241)
#define HCI_REFRESH_ENCRYPTION_KEY_PARAM_LEN               ((uint8_t) 2)
#define HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_PARAM_LEN ((uint8_t) 0)
#define HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL_PARAM_LEN   ((uint8_t) 1)
#define HCI_ENHANCED_FLUSH_PARAM_LEN             ((uint8_t) 3)
#define HCI_SET_EVENT_MASK_PAGE_2_PARAM_LEN      ((uint8_t) 8)
#define HCI_SEND_KEYPRESS_NOTIFICATION_PARAM_LEN ((uint8_t) 7)
#define HCI_READ_SIMPLE_PAIRING_MODE_PARAM_LEN   ((uint8_t) 0)
#define HCI_WRITE_SIMPLE_PAIRING_MODE_PARAM_LEN  ((uint8_t) 1)
#define HCI_READ_LOCAL_OOB_DATA_PARAM_LEN        ((uint8_t) 0)
#define HCI_READ_ENH_TX_POWER_LEVEL_PARAM_LEN    ((uint8_t) 3)
#define HCI_READ_LE_HOST_SUPPORT_PARAM_LEN       ((uint8_t) 0)
#define HCI_WRITE_LE_HOST_SUPPORT_PARAM_LEN      ((uint8_t) 2)
#define HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT_PARAM_LEN    ((uint8_t) 0)
#define HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_PARAM_LEN   ((uint8_t) 1)
#define HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_PARAM_LEN      ((uint8_t) 2)
#define HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_PARAM_LEN     ((uint8_t) 4)
#define HCI_READ_LOCAL_OOB_EXTENDED_DATA_PARAM_LEN            ((uint8_t) 0)
#define HCI_SET_RESERVED_LT_ADDR_PARAM_LEN       ((uint8_t) 1)
#define HCI_DELETE_RESERVED_LT_ADDR_PARAM_LEN    ((uint8_t) 1)
#define HCI_SET_CSB_DATA_PARAM_LEN               ((uint8_t) 255)
#define HCI_READ_SYNCHRONIZATION_TRAIN_PARAMS_PARAM_LEN  ((uint8_t) 0)
#define HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_PARAM_LEN ((uint8_t) 9)

/* Informational Parameters (HCI_INFO) */
#define HCI_READ_LOCAL_VER_INFO_PARAM_LEN        ((uint8_t) 0)
#define HCI_READ_LOCAL_SUPP_COMMANDS_PARAM_LEN   ((uint8_t) 0)
#define HCI_READ_LOCAL_SUPP_FEATURES_PARAM_LEN   ((uint8_t) 0)
#define HCI_READ_LOCAL_EXT_FEATURES_PARAM_LEN    ((uint8_t) 1)
#define HCI_READ_BUFFER_SIZE_PARAM_LEN           ((uint8_t) 0)
#define HCI_READ_COUNTRY_CODE_PARAM_LEN          ((uint8_t) 0)
#define HCI_READ_BD_ADDR_PARAM_LEN               ((uint8_t) 0)
#define HCI_READ_LOCAL_SUPP_CODECS_PARAMS_LEN    ((uint8_t) 0)

/* Status (HCI_STATUS) */
#define HCI_READ_FAILED_CONTACT_COUNT_PARAM_LEN  ((uint8_t) 2)
#define HCI_RESET_FAILED_CONTACT_COUNT_PARAM_LEN ((uint8_t) 2)
#define HCI_GET_LINK_QUALITY_PARAM_LEN           ((uint8_t) 2)
#define HCI_READ_RSSI_PARAM_LEN                  ((uint8_t) 2)
#define HCI_READ_AFH_CHANNEL_MAP_PARAM_LEN       ((uint8_t) 2)
#define HCI_READ_CLOCK_PARAM_LEN                 ((uint8_t) 3)
#define HCI_READ_ENCRYPTION_KEY_SIZE_PARAM_LEN   ((uint8_t) 2)
#define HCI_SET_TRIGGERED_CLOCK_CAPTURE_PARAM_LEN ((uint8_t) 6)

/* Testing (HCI_TEST) */
#define HCI_READ_LOOPBACK_MODE_PARAM_LEN               ((uint8_t) 0)
#define HCI_WRITE_LOOPBACK_MODE_PARAM_LEN              ((uint8_t) 1)
#define HCI_ENABLE_DUT_MODE_PARAM_LEN                  ((uint8_t) 0)
#define HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE_PARAM_LEN  ((uint8_t) 1)
#define HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE_PARAM_LEN ((uint8_t) 4)

#define HCI_DEBUG_REQUEST_PARAM_LEN              ((uint8_t) 2)

/* ULP */
#define HCI_ULP_SET_EVENT_MASK_PARAM_LEN                            ((uint8_t)  8)
#define HCI_ULP_READ_BUFFER_SIZE_PARAM_LEN                          ((uint8_t)  0)
#define HCI_ULP_READ_LOCAL_SUPPORTED_FEATURES_PARAM_LEN             ((uint8_t)  0)
#define HCI_ULP_SET_RANDOM_ADDRESS_PARAM_LEN                        ((uint8_t)  6)
#define HCI_ULP_SET_ADVERTISING_PARAMETERS_PARAM_LEN                ((uint8_t) 15)
#define HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_PARAM_LEN         ((uint8_t)  0)
#define HCI_ULP_SET_ADVERTISING_DATA_PARAM_LEN                      ((uint8_t) 32)
#define HCI_ULP_SET_SCAN_RESPONSE_DATA_PARAM_LEN                    ((uint8_t) 32)
#define HCI_ULP_SET_ADVERTISE_ENABLE_PARAM_LEN                      ((uint8_t)  1)
#define HCI_ULP_SET_SCAN_PARAMETERS_PARAM_LEN                       ((uint8_t)  7)
#define HCI_ULP_SET_SCAN_ENABLE_PARAM_LEN                           ((uint8_t)  2)
#define HCI_ULP_CREATE_CONNECTION_PARAM_LEN                         ((uint8_t) 25)
#define HCI_ULP_CREATE_CONNECTION_CANCEL_PARAM_LEN                  ((uint8_t)  0)
#define HCI_ULP_READ_WHITE_LIST_SIZE_PARAM_LEN                      ((uint8_t)  0)
#define HCI_ULP_CLEAR_WHITE_LIST_PARAM_LEN                          ((uint8_t)  0)
#define HCI_ULP_ADD_DEVICE_TO_WHITE_LIST_PARAM_LEN                  ((uint8_t)  7)
#define HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST_PARAM_LEN             ((uint8_t)  7)
#define HCI_ULP_CONNECTION_UPDATE_PARAM_LEN                         ((uint8_t) 14)
#define HCI_ULP_SET_HOST_CHANNEL_CLASSIFICATION_PARAM_LEN           ((uint8_t)  5)
#define HCI_ULP_READ_CHANNEL_MAP_PARAM_LEN                          ((uint8_t)  2)
#define HCI_ULP_READ_REMOTE_USED_FEATURES_PARAM_LEN                 ((uint8_t)  2)
#define HCI_ULP_ENCRYPT_PARAM_LEN                                   ((uint8_t) 32)
#define HCI_ULP_RAND_PARAM_LEN                                      ((uint8_t)  0)
#define HCI_ULP_START_ENCRYPTION_PARAM_LEN                          ((uint8_t) 28)
#define HCI_ULP_LONG_TERM_KEY_REQUEST_REPLY_PARAM_LEN               ((uint8_t) 18)
#define HCI_ULP_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_PARAM_LEN      ((uint8_t)  2)
#define HCI_ULP_READ_SUPPORTED_STATES_PARAM_LEN                     ((uint8_t)  0)
#define HCI_ULP_RECEIVER_TEST_PARAM_LEN                             ((uint8_t)  1)
#define HCI_ULP_TRANSMITTER_TEST_PARAM_LEN                          ((uint8_t)  3)
#define HCI_ULP_TEST_END_PARAM_LEN                                  ((uint8_t)  0)
#define HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY_PARAM_LEN ((uint8_t) 14)
#define HCI_ULP_REMOTE_CONNECTION_PARRAMTER_REQUEST_NEGATIVE_REPLY_PARAM_LEN    ((uint8_t) 3)
#define HCI_ULP_SET_DATA_LENGTH_PARAM_LEN                           ((uint8_t)  6)
#define HCI_ULP_READ_SUGGESTED_DEFAULT_DATA_LENGTH_PARAM_LEN        ((uint8_t)  0)
#define HCI_ULP_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH_PARAM_LEN       ((uint8_t)  4)
#define HCI_ULP_READ_LOCAL_P256_PUBLIC_KEY_PARAM_LEN                ((uint8_t)  0)
#define HCI_ULP_GENERATE_DHKEY_PARAM_LEN                            ((uint8_t) 64)
#define HCI_ULP_READ_MAXIMUM_DATA_LENGTH_PARAM_LEN                  ((uint8_t)  0)
#define HCI_ULP_ADD_DEVICE_TO_RESOLVING_LIST_PARAM_LEN              ((uint8_t) 39)
#define HCI_ULP_REMOVE_DEVICE_FROM_RESOLVING_LIST_PARAM_LEN         ((uint8_t)  7)
#define HCI_ULP_CLEAR_RESOLVING_LIST_PARAM_LEN                      ((uint8_t)  0)
#define HCI_ULP_READ_RESOLVING_LIST_SIZE_PARAM_LEN                  ((uint8_t)  0)
#define HCI_ULP_READ_PEER_RESOLVABLE_ADDRESS_PARAM_LEN              ((uint8_t)  7)
#define HCI_ULP_READ_LOCAL_RESOLVABLE_ADDRESS_PARAM_LEN             ((uint8_t)  7)
#define HCI_ULP_SET_ADDRESS_RESOLUTION_ENABLE_PARAM_LEN             ((uint8_t)  1)
#define HCI_ULP_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT_PARAM_LEN    ((uint8_t)  2)
#define HCI_ULP_READ_PHY_PARAM_LEN                                  ((uint8_t)  2)
#define HCI_ULP_SET_DEFAULT_PHY_PARAM_LEN                           ((uint8_t)  3)
#define HCI_ULP_SET_PHY_PARAM_LEN                                   ((uint8_t)  7)
/*LEERT & LEETT: It does not take into consideration the `unknown[]` parameter for now */
#define HCI_ULP_ENHANCED_RECEIVER_TEST_PARAM_LEN                    ((uint8_t)  3)
#define HCI_ULP_ENHANCED_TRANSMITTER_TEST_PARAM_LEN                 ((uint8_t)  4)
#define HCI_ULP_SET_PRIVACY_MODE_PARAM_LEN                          ((uint8_t)  8)

/******************************************************************************
        Event Parameter Lengths
 *****************************************************************************/
#define HCI_EV_INQUIRY_COMPLETE_PARAM_LEN                        ((uint8_t)  1)
#define HCI_EV_INQUIRY_RESULT_PARAM_LEN                          ((uint8_t) 15) /* Variable */
#define HCI_EV_CONN_COMPLETE_PARAM_LEN                           ((uint8_t) 11)
#define HCI_EV_CONN_REQUEST_PARAM_LEN                            ((uint8_t) 10)
#define HCI_EV_DISCONNECT_COMPLETE_PARAM_LEN                     ((uint8_t)  4)
#define HCI_EV_AUTH_COMPLETE_PARAM_LEN                           ((uint8_t)  3)
#define HCI_EV_REMOTE_NAME_REQ_COMPLETE_MAX_LEN                  ((uint8_t)255)
#define HCI_EV_REMOTE_NAME_REQ_COMPLETE_BASIC_LEN                ((uint8_t)  7)
#define HCI_EV_ENCRYPTION_CHANGE_PARAM_LEN                       ((uint8_t)  4)
#define HCI_EV_CHANGE_CONN_LINK_KEY_COMPLETE_PARAM_LEN           ((uint8_t)  3)
#define HCI_EV_MASTER_LINK_KEY_COMPLETE_PARAM_LEN                ((uint8_t)  4)
#define HCI_EV_READ_REM_SUPP_FEATURES_COMPLETE_PARAM_LEN         ((uint8_t) 11)
#define HCI_EV_READ_REMOTE_VER_INFO_COMPLETE_PARAM_LEN           ((uint8_t)  8)
#define HCI_EV_QOS_SETUP_COMPLETE_PARAM_LEN                      ((uint8_t) 21)
#define HCI_EV_COMMAND_COMPLETE_PARAM_LEN                        ((uint8_t)  3) /* Variable see below */
#define HCI_EV_COMMAND_STATUS_PARAM_LEN                          ((uint8_t)  4)
#define HCI_EV_HARDWARE_ERROR_PARAM_LEN                          ((uint8_t)  1)
#define HCI_EV_FLUSH_OCCURRED_PARAM_LEN                          ((uint8_t)  2)
#define HCI_EV_ROLE_CHANGE_PARAM_LEN                             ((uint8_t)  8)
#define HCI_EV_NUMBER_COMPLETED_PKTS_PARAM_LEN                   ((uint8_t)  5) /* Variable */
#define HCI_EV_MODE_CHANGE_PARAM_LEN                             ((uint8_t)  6)
#define HCI_EV_RETURN_LINK_KEYS_PARAM_LEN                        ((uint8_t) 23) /* Variable */
#define HCI_EV_PIN_CODE_REQ_PARAM_LEN                            ((uint8_t)  6)
#define HCI_EV_LINK_KEY_REQ_PARAM_LEN                            ((uint8_t)  6)
#define HCI_EV_LINK_KEY_NOTIFICATION_PARAM_LEN                   ((uint8_t) 23)
#define HCI_EV_LOOPBACK_COMMAND_PARAM_LEN                        ((uint8_t)  0) /* Variable */
#define HCI_EV_DATA_BUFFER_OVERFLOW_PARAM_LEN                    ((uint8_t)  1)
#define HCI_EV_MAX_SLOTS_CHANGE_PARAM_LEN                        ((uint8_t)  3)
#define HCI_EV_READ_CLOCK_OFFSET_COMPLETE_PARAM_LEN              ((uint8_t)  5)
#define HCI_EV_CONN_PACKET_TYPE_CHANGED_PARAM_LEN                ((uint8_t)  5)
#define HCI_EV_QOS_VIOLATION_PARAM_LEN                           ((uint8_t)  2)
#define HCI_EV_PAGE_SCAN_MODE_CHANGE_PARAM_LEN                   ((uint8_t)  7)
#define HCI_EV_PAGE_SCAN_REP_MODE_CHANGE_PARAM_LEN               ((uint8_t)  7)
#define HCI_EV_FLOW_SPEC_COMPLETE_PARAM_LEN                      ((uint8_t) 22)
#define HCI_EV_INQUIRY_RESULT_WITH_RSSI_PARAM_LEN                ((uint8_t) 15) /* Variable */
#define HCI_EV_READ_REM_EXT_FEATURES_COMPLETE_PARAM_LEN          ((uint8_t) 13)
#define HCI_EV_FIXED_ADDRESS_PARAM_LEN                           ((uint8_t)  9)
#define HCI_EV_ALIAS_ADDRESS_PARAM_LEN                           ((uint8_t)  8)
#define HCI_EV_GENERATE_ALIAS_REQ_PARAM_LEN                      ((uint8_t)  2)
#define HCI_EV_ACTIVE_ADDRESS_PARAM_LEN                          ((uint8_t) 12)
#define HCI_EV_ALLOW_PRIVATE_PAIRING_PARAM_LEN                   ((uint8_t)  2)
#define HCI_EV_ALIAS_ADDRESS_REQ_PARAM_LEN                       ((uint8_t)  6)
#define HCI_EV_ALIAS_NOT_RECOGNISED_PARAM_LEN                    ((uint8_t)  7)
#define HCI_EV_FIXED_ADDRESS_ATTEMPT_PARAM_LEN                   ((uint8_t)  4)
#define HCI_EV_SYNC_CONN_COMPLETE_PARAM_LEN                      ((uint8_t) 17)
#define HCI_EV_SYNC_CONN_CHANGED_PARAM_LEN                       ((uint8_t)  9)
#define HCI_EV_SNIFF_SUB_RATE_PARAM_LEN                          ((uint8_t) 11)
#define HCI_EV_EXTENDED_INQUIRY_RESULT_PARAM_LEN                 ((uint8_t)255)
#define HCI_EV_ENCRYPTION_KEY_REFRESH_COMPLETE_PARAM_LEN         ((uint8_t)  3)
#define HCI_EV_IO_CAPABILITY_REQUEST_PARAM_LEN                   ((uint8_t)  6)
#define HCI_EV_IO_CAPABILITY_RESPONSE_PARAM_LEN                  ((uint8_t)  9)
#define HCI_EV_USER_CONFIRMATION_REQUEST_PARAM_LEN               ((uint8_t) 10)
#define HCI_EV_USER_PASSKEY_REQUEST_PARAM_LEN                    ((uint8_t)  6)
#define HCI_EV_REMOTE_OOB_DATA_REQUEST_PARAM_LEN                 ((uint8_t)  6)
#define HCI_EV_SIMPLE_PAIRING_COMPLETE_PARAM_LEN                 ((uint8_t)  7)
#define HCI_EV_LST_CHANGE_PARAM_LEN                              ((uint8_t)  4)
#define HCI_EV_ENHANCED_FLUSH_COMPLETE_PARAM_LEN                 ((uint8_t)  2)
#define HCI_EV_USER_PASSKEY_NOTIFICATION_PARAM_LEN               ((uint8_t) 10)
#define HCI_EV_KEYPRESS_NOTIFICATION_PARAM_LEN                   ((uint8_t)  7)
#define HCI_EV_REM_HOST_SUPPORTED_FEATURES_PARAM_LEN             ((uint8_t) 14)
#define HCI_EV_TRIGGERED_CLOCK_CAPTURE_PARAM_LEN                 ((uint8_t)  9)
#define HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE_PARAM_LEN          ((uint8_t)  1)
#define HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED_PARAM_LEN          ((uint8_t) 29)
#define HCI_EV_CSB_RECEIVE_PARAM_MIN_LEN                         ((uint8_t) 18)
#define HCI_EV_CSB_TIMEOUT_PARAM_LEN                             ((uint8_t)  7)
#define HCI_EV_TRUNCATED_PAGE_COMPLETE_PARAM_LEN                 ((uint8_t)  7)
#define HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT_PARAM_LEN             ((uint8_t)  0)
#define HCI_EV_CSB_CHANNEL_MAP_CHANGE_PARAM_LEN                  ((uint8_t) 10)
#define HCI_EV_INQUIRY_RESPONSE_NOTIFICATION_PARAM_LEN           ((uint8_t)  4)
#define HCI_EV_ULP_CONNECTION_COMPLETE_PARAM_LEN                 ((uint8_t) 19)
#define HCI_EV_ULP_ADVERTISING_REPORT_PARAM_LEN                  ((uint8_t) 12) /* Variable */
#define HCI_EV_ULP_CONNECTION_UPDATE_COMPLETE_PARAM_LEN          ((uint8_t) 10)
#define HCI_EV_ULP_READ_REMOTE_USED_FEATURES_COMPLETE_PARAM_LEN  ((uint8_t) 12)
#define HCI_EV_ULP_LONG_TERM_KEY_REQUEST_PARAM_LEN               ((uint8_t) 13)
#define HCI_EV_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_PARAM_LEN ((uint8_t) 11)
#define HCI_EV_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE_PARAM_LEN    ((uint8_t) 66)
#define HCI_EV_ULP_GENERATE_DHKEY_COMPLETE_PARAM_LEN             ((uint8_t) 34)
#define HCI_EV_ULP_ENHANCED_CONNECTION_COMPLETE_PARAM_LEN        ((uint8_t) 31)
#define HCI_EV_ULP_DIRECT_ADVERTISING_REPORT_PARAM_LEN           ((uint8_t) 18) /* Variable */
#define HCI_EV_DEBUG_PARAM_LEN                                   ((uint8_t) 20)
#define HCI_EV_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_PARAM_LEN   ((uint8_t)  2)
#define HCI_EV_ULP_PHY_UPDATE_COMPLETE_PARAM_LEN                 ((uint8_t)  6)
#define HCI_EV_MNFR_EXTENSION_PARAM_LEN                          ((uint8_t) 20)
#define HCI_EV_ULP_PHY_DATA_LENGTH_CHANGE_LEN                    ((uint8_t) 11)
#define HCI_EV_ULP_CHANNEL_SELECTION_ALGORITHM_PARAM_LEN         ((uint8_t)  4)


/******************************************************************************
   HCI_COMMAND_COMPLETE, Argument Length Definitions (Full length)
   Should consist of: nhcp (1) + opcode (2) + return parameters
                    : = 3 + return parameters from spec (incl. status)
   When an argument length is dependant on the number of elements in the array
   the defined length contains the constant parameter lengths only. The full
   array length must be calculated.
*****************************************************************************/
/* Link Control (HCI_LINK) */
#define HCI_INQUIRY_CANCEL_ARG_LEN                          ((uint8_t)  4)
#define HCI_PERIODIC_INQ_MODE_ARG_LEN                       ((uint8_t)  4)
#define HCI_EXIT_PERIODIC_INQ_MODE_ARG_LEN                  ((uint8_t)  4)
#define HCI_CREATE_CONNECTION_CANCEL_ARG_LEN                ((uint8_t) 10)
#define HCI_LINK_KEY_REQ_REPLY_ARG_LEN                      ((uint8_t) 10)
#define HCI_LINK_KEY_REQ_NEG_REPLY_ARG_LEN                  ((uint8_t) 10)
#define HCI_PIN_CODE_REQ_REPLY_ARG_LEN                      ((uint8_t) 10)
#define HCI_PIN_CODE_REQ_NEG_REPLY_ARG_LEN                  ((uint8_t) 10)
#define HCI_REMOTE_NAME_REQ_CANCEL_ARG_LEN                  ((uint8_t) 10)
#define HCI_READ_LMP_HANDLE_ARG_LEN                         ((uint8_t) 11)
#define HCI_PRIVATE_PAIRING_REQ_REPLY_ARG_LEN               ((uint8_t)  6)
#define HCI_PRIVATE_PAIRING_REQ_NEG_REPLY_ARG_LEN           ((uint8_t)  6)
#define HCI_GENERATED_ALIAS_ARG_LEN                         ((uint8_t)  6)
#define HCI_ALIAS_ADDRESS_REQ_REPLY_ARG_LEN                 ((uint8_t) 10)
#define HCI_ALIAS_ADDRESS_REQ_NEG_REPLY_ARG_LEN             ((uint8_t) 10)
#define HCI_IO_CAPABILITY_REQUEST_REPLY_ARG_LEN             ((uint8_t) 10)
#define HCI_USER_CONFIRMATION_REQUEST_REPLY_ARG_LEN         ((uint8_t) 10)
#define HCI_USER_CONFIRMATION_REQUEST_NEG_REPLY_ARG_LEN     ((uint8_t) 10)
#define HCI_USER_PASSKEY_REQUEST_REPLY_ARG_LEN              ((uint8_t) 10)
#define HCI_USER_PASSKEY_REQUEST_NEG_REPLY_ARG_LEN          ((uint8_t) 10)
#define HCI_REMOTE_OOB_DATA_REQUEST_REPLY_ARG_LEN           ((uint8_t) 10)
#define HCI_REMOTE_OOB_DATA_REQUEST_NEG_REPLY_ARG_LEN       ((uint8_t) 10)
#define HCI_IO_CAPABILITY_REQUEST_NEG_REPLY_ARG_LEN         ((uint8_t) 10)
#define HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY_ARG_LEN  ((uint8_t) 10)
#define HCI_TRUNCATED_PAGE_CANCEL_ARG_LEN      ((uint8_t) 10)
#define HCI_SET_CSB_ARG_LEN                    ((uint8_t) 7)
#define HCI_SET_CSB_RECEIVE_ARG_LEN            ((uint8_t) 11)

/* Link Policy (HCI_POLICY) */
#define HCI_ROLE_DISCOVERY_ARG_LEN             ((uint8_t) 7)
#define HCI_READ_LINK_POLICY_SETTINGS_ARG_LEN  ((uint8_t) 8)
#define HCI_WRITE_LINK_POLICY_SETTINGS_ARG_LEN ((uint8_t) 6)
#define HCI_READ_DEFAULT_LINK_POLICY_SETTINGS_ARG_LEN  ((uint8_t) 6)
#define HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_ARG_LEN  ((uint8_t) 4)
#define HCI_SNIFF_SUB_RATE_ARG_LEN             ((uint8_t) 6)

/* Controller and Baseband (HCI_HOST_BB) */
#define HCI_SET_EVENT_MASK_ARG_LEN             ((uint8_t) 4)
#define HCI_RESET_ARG_LEN                      ((uint8_t) 4)
#define HCI_SET_EVENT_FILTER_ARG_LEN           ((uint8_t) 4)
#define HCI_FLUSH_ARG_LEN                      ((uint8_t) 6)
#define HCI_READ_PIN_TYPE_ARG_LEN              ((uint8_t) 5)
#define HCI_WRITE_PIN_TYPE_ARG_LEN             ((uint8_t) 4)
#define HCI_CREATE_NEW_UNIT_KEY_ARG_LEN        ((uint8_t) 4)
#define HCI_READ_STORED_LINK_KEY_ARG_LEN       ((uint8_t) 8)
#define HCI_WRITE_STORED_LINK_KEY_ARG_LEN      ((uint8_t) 5)
#define HCI_DELETE_STORED_LINK_KEY_ARG_LEN     ((uint8_t) 6)
#define HCI_CHANGE_LOCAL_NAME_ARG_LEN          ((uint8_t) 4)
#define HCI_READ_LOCAL_NAME_ARG_LEN            ((uint8_t) 252)
#define HCI_READ_CONN_ACCEPT_TIMEOUT_ARG_LEN   ((uint8_t) 6)
#define HCI_WRITE_CONN_ACCEPT_TIMEOUT_ARG_LEN  ((uint8_t) 4)
#define HCI_READ_PAGE_TIMEOUT_ARG_LEN          ((uint8_t) 6)
#define HCI_WRITE_PAGE_TIMEOUT_ARG_LEN         ((uint8_t) 4)
#define HCI_READ_SCAN_ENABLE_ARG_LEN           ((uint8_t) 5)
#define HCI_WRITE_SCAN_ENABLE_ARG_LEN          ((uint8_t) 4)
#define HCI_READ_PAGESCAN_ACTIVITY_ARG_LEN     ((uint8_t) 8)
#define HCI_WRITE_PAGESCAN_ACTIVITY_ARG_LEN    ((uint8_t) 4)
#define HCI_READ_INQUIRYSCAN_ACTIVITY_ARG_LEN  ((uint8_t) 8)
#define HCI_WRITE_INQUIRYSCAN_ACTIVITY_ARG_LEN ((uint8_t) 4)
#define HCI_READ_AUTH_ENABLE_ARG_LEN           ((uint8_t) 5)
#define HCI_WRITE_AUTH_ENABLE_ARG_LEN          ((uint8_t) 4)
#define HCI_READ_ENC_MODE_ARG_LEN              ((uint8_t) 5)
#define HCI_WRITE_ENC_MODE_ARG_LEN             ((uint8_t) 4)
#define HCI_READ_CLASS_OF_DEVICE_ARG_LEN       ((uint8_t) 7)
#define HCI_WRITE_CLASS_OF_DEVICE_ARG_LEN      ((uint8_t) 4)
#define HCI_READ_VOICE_SETTING_ARG_LEN         ((uint8_t) 6)
#define HCI_WRITE_VOICE_SETTING_ARG_LEN        ((uint8_t) 4)
#define HCI_READ_AUTO_FLUSH_TIMEOUT_ARG_LEN    ((uint8_t) 8)
#define HCI_WRITE_AUTO_FLUSH_TIMEOUT_ARG_LEN   ((uint8_t) 6)
#define HCI_READ_NUM_BCASTXS_ARG_LEN           ((uint8_t) 5)
#define HCI_WRITE_NUM_BCASTXS_ARG_LEN          ((uint8_t) 4)
#define HCI_READ_HOLD_MODE_ACTIVITY_ARG_LEN    ((uint8_t) 5)
#define HCI_WRITE_HOLD_MODE_ACTIVITY_ARG_LEN   ((uint8_t) 4)
#define HCI_READ_TX_POWER_LEVEL_ARG_LEN        ((uint8_t) 7)
#define HCI_READ_SCO_FLOW_CON_ENABLE_ARG_LEN   ((uint8_t) 5)
#define HCI_WRITE_SCO_FLOW_CON_ENABLE_ARG_LEN  ((uint8_t) 4)
#define HCI_SET_HC_TO_H_FLOW_CONTROL_ARG_LEN   ((uint8_t) 4)
#define HCI_HOST_BUFFER_SIZE_ARG_LEN           ((uint8_t) 4)
#define HCI_HOST_NUM_COMPLETED_PKTS_ARG_LEN    ((uint8_t) 4)
#define HCI_READ_LINK_SUPERV_TIMEOUT_ARG_LEN   ((uint8_t) 8)
#define HCI_WRITE_LINK_SUPERV_TIMEOUT_ARG_LEN  ((uint8_t) 6)
#define HCI_READ_NUM_SUPPORTED_IAC_ARG_LEN     ((uint8_t) 5)
#define HCI_READ_CURRENT_IAC_LAP_ARG_LEN       ((uint8_t) 5) /* Variable */
#define HCI_WRITE_CURRENT_IAC_LAP_ARG_LEN      ((uint8_t) 4)
#define HCI_READ_PAGESCAN_PERIOD_MODE_ARG_LEN  ((uint8_t) 5)
#define HCI_WRITE_PAGESCAN_PERIOD_MODE_ARG_LEN ((uint8_t) 4)
#define HCI_READ_PAGESCAN_MODE_ARG_LEN         ((uint8_t) 5)
#define HCI_WRITE_PAGESCAN_MODE_ARG_LEN        ((uint8_t) 4)
#define HCI_SET_AFH_CHANNEL_CLASS_ARG_LEN      ((uint8_t) 4)
#define HCI_READ_INQUIRY_SCAN_TYPE_ARG_LEN     ((uint8_t) 5)
#define HCI_WRITE_INQUIRY_SCAN_TYPE_ARG_LEN    ((uint8_t) 4)
#define HCI_READ_INQUIRY_MODE_ARG_LEN          ((uint8_t) 5)
#define HCI_WRITE_INQUIRY_MODE_ARG_LEN         ((uint8_t) 4)
#define HCI_READ_PAGE_SCAN_TYPE_ARG_LEN        ((uint8_t) 5)
#define HCI_WRITE_PAGE_SCAN_TYPE_ARG_LEN       ((uint8_t) 4)
#define HCI_READ_AFH_CHANNEL_CLASS_M_ARG_LEN   ((uint8_t) 5)
#define HCI_WRITE_AFH_CHANNEL_CLASS_M_ARG_LEN  ((uint8_t) 4)
#define HCI_READ_ANON_MODE_ARG_LEN             ((uint8_t) 5)
#define HCI_WRITE_ANON_MODE_ARG_LEN            ((uint8_t) 4)
#define HCI_READ_ALIAS_AUTH_ENABLE_ARG_LEN     ((uint8_t) 5)
#define HCI_WRITE_ALIAS_AUTH_ENABLE_ARG_LEN    ((uint8_t) 4)
#define HCI_READ_ANON_ADDR_CHANGE_PARAMS_ARG_LEN ((uint8_t) 10)
#define HCI_WRITE_ANON_ADDR_CHANGE_PARAMS_ARG_LEN ((uint8_t)  4)
#define HCI_RESET_FIXED_ADDRESS_ATTEMPTS_COUNTER_ARG_LEN ((uint8_t) 4)
#define HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_ARG_LEN  ((uint8_t) 245)
#define HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA_ARG_LEN ((uint8_t) 4)
#define HCI_REFRESH_ENCRYPTION_KEY_ARG_LEN     ((uint8_t) 6)
#define HCI_WRITE_SIMPLE_PAIRING_MODE_ARG_LEN  ((uint8_t) 4)
#define HCI_READ_SIMPLE_PAIRING_MODE_ARG_LEN   ((uint8_t) 5)
#define HCI_READ_LOCAL_OOB_DATA_ARG_LEN        ((uint8_t) 36)
#define HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_ARG_LEN ((uint8_t) 5)
#define HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL_ARG_LEN ((uint8_t) 4)
#define HCI_SET_EVENT_MASK_PAGE_2_ARG_LEN      ((uint8_t) 4)
#define HCI_SEND_KEYPRESS_NOTIFICATION_ARG_LEN ((uint8_t) 10)
#define HCI_READ_ENH_TX_POWER_LEVEL_ARG_LEN    ((uint8_t) 9)
#define HCI_READ_LE_HOST_SUPPORT_ARG_LEN       ((uint8_t) 6)
#define HCI_WRITE_LE_HOST_SUPPORT_ARG_LEN      ((uint8_t) 4)
#define HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT_ARG_LEN    ((uint8_t) 5)
#define HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_ARG_LEN   ((uint8_t) 4)
#define HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_ARG_LEN      ((uint8_t) 8)
#define HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_ARG_LEN     ((uint8_t) 6)
#define HCI_READ_LOCAL_OOB_EXTENDED_DATA_ARG_LEN            ((uint8_t) 68)
#define HCI_SET_RESERVED_LT_ADDR_ARG_LEN       ((uint8_t) 5)
#define HCI_DELETE_RESERVED_LT_ADDR_ARG_LEN    ((uint8_t) 5)
#define HCI_SET_CSB_DATA_ARG_LEN               ((uint8_t) 5)
#define HCI_READ_SYNCHRONIZATION_TRAIN_PARAMS_ARG_LEN  ((uint8_t) 11)
#define HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_ARG_LEN ((uint8_t) 6)

/* Information Parameters (HCI_INFO) */
#define HCI_READ_LOCAL_VER_INFO_ARG_LEN        ((uint8_t) 12)
#define HCI_READ_LOCAL_COMMANDS_LEN            ((uint8_t) 68)
#define HCI_READ_LOCAL_FEATURES_LEN            ((uint8_t) 12)
#define HCI_READ_LOCAL_EXT_FEATURES_LEN        ((uint8_t) 14)
#define HCI_READ_BUFFER_SIZE_ARG_LEN           ((uint8_t) 11)
#define HCI_READ_COUNTRY_CODE_ARG_LEN          ((uint8_t) 5)
#define HCI_READ_BD_ADDR_ARG_LEN               ((uint8_t) 10)
#define HCI_READ_LOCAL_SUPP_CODECS_ARG_LEN     ((uint8_t) 6)

/* Status Parameters (HCI_STATUS) */
#define HCI_READ_FAILED_CONTACT_COUNT_ARG_LEN  ((uint8_t) 8)
#define HCI_RESET_FAILED_CONTACT_COUNT_ARG_LEN ((uint8_t) 6)
#define HCI_GET_LINK_QUALITY_ARG_LEN           ((uint8_t) 7)
#define HCI_READ_RSSI_ARG_LEN                  ((uint8_t) 7)
#define HCI_READ_AFH_CHANNEL_MAP_ARG_LEN       ((uint8_t) 17)
#define HCI_READ_CLOCK_ARG_LEN                 ((uint8_t) 12)
#define HCI_READ_ENCRYPTION_KEY_SIZE_ARG_LEN   ((uint8_t) 7)
#define HCI_SET_TRIGGERED_CLOCK_CAPTURE_ARG_LEN ((uint8_t) 4)

/* Testing (HCI_TEST) */
#define HCI_READ_LOOPBACK_MODE_ARG_LEN         ((uint8_t) 5)
#define HCI_WRITE_LOOPBACK_MODE_ARG_LEN        ((uint8_t) 4)
#define HCI_ENABLE_DUT_ARG_LEN                 ((uint8_t) 4)
#define HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE_ARG_LEN ((uint8_t) 4)
#define HCI_MNFR_EXTENSION_ARG_LEN             ((uint8_t) 4)
#define HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE_ARG_LEN ((uint8_t) 6)

/* ULP */
#define HCI_ULP_SET_EVENT_MASK_ARG_LEN                            ((uint8_t)  4)
#define HCI_ULP_READ_BUFFER_SIZE_ARG_LEN                          ((uint8_t)  7)
#define HCI_ULP_READ_LOCAL_SUPPORTED_FEATURES_ARG_LEN             ((uint8_t) 12)
#define HCI_ULP_SET_RANDOM_ADDRESS_ARG_LEN                        ((uint8_t)  4)
#define HCI_ULP_SET_ADVERTISING_PARAMETERS_ARG_LEN                ((uint8_t)  4)
#define HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_ARG_LEN         ((uint8_t)  5)
#define HCI_ULP_SET_ADVERTISING_DATA_ARG_LEN                      ((uint8_t)  4)
#define HCI_ULP_SET_SCAN_RESPONSE_DATA_ARG_LEN                    ((uint8_t)  4)
#define HCI_ULP_SET_ADVERTISE_ENABLE_ARG_LEN                      ((uint8_t)  4)
#define HCI_ULP_SET_SCAN_PARAMETERS_ARG_LEN                       ((uint8_t)  4)
#define HCI_ULP_SET_SCAN_ENABLE_ARG_LEN                           ((uint8_t)  4)
#define HCI_ULP_CREATE_CONNECTION_CANCEL_ARG_LEN                  ((uint8_t)  4)
#define HCI_ULP_READ_WHITE_LIST_SIZE_ARG_LEN                      ((uint8_t)  5)
#define HCI_ULP_CLEAR_WHITE_LIST_ARG_LEN                          ((uint8_t)  4)
#define HCI_ULP_ADD_DEVICE_TO_WHITE_LIST_ARG_LEN                  ((uint8_t)  4)
#define HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST_ARG_LEN             ((uint8_t)  4)
#define HCI_ULP_SET_HOST_CHANNEL_CLASSIFICATION_ARG_LEN           ((uint8_t)  4)
#define HCI_ULP_READ_CHANNEL_MAP_ARG_LEN                          ((uint8_t) 11)
#define HCI_ULP_ENCRYPT_ARG_LEN                                   ((uint8_t) 20)
#define HCI_ULP_RAND_ARG_LEN                                      ((uint8_t) 12)
#define HCI_ULP_LONG_TERM_KEY_REQUEST_REPLY_ARG_LEN               ((uint8_t)  6)
#define HCI_ULP_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_ARG_LEN      ((uint8_t)  6)
#define HCI_ULP_READ_SUPPORTED_STATES_ARG_LEN                     ((uint8_t) 12)
#define HCI_ULP_RECEIVER_TEST_ARG_LEN                             ((uint8_t)  4)
#define HCI_ULP_TRANSMITTER_TEST_ARG_LEN                          ((uint8_t)  4)
#define HCI_ULP_TEST_END_ARG_LEN                                  ((uint8_t)  6)
#define HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY_ARG_LEN ((uint8_t)  6)
#define HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY_ARG_LEN ((uint8_t)  6)
#define HCI_ULP_SET_DATA_LENGTH_ARG_LEN                           ((uint8_t)  6)
#define HCI_ULP_READ_SUGGESTED_DEFAULT_DATA_LENGTH_ARG_LEN        ((uint8_t)  8)
#define HCI_ULP_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH_ARG_LEN       ((uint8_t)  4)
#define HCI_ULP_READ_MAXIMUM_DATA_LENGTH_ARG_LEN                  ((uint8_t) 12)
#define HCI_ULP_ADD_DEVICE_TO_RESOLVING_LIST_ARG_LEN              ((uint8_t)  4)
#define HCI_ULP_REMOVE_DEVICE_FROM_RESOLVING_LIST_ARG_LEN         ((uint8_t)  4)
#define HCI_ULP_CLEAR_RESOLVING_LIST_ARG_LEN                      ((uint8_t)  4)
#define HCI_ULP_READ_RESOLVING_LIST_SIZE_ARG_LEN                  ((uint8_t)  5)
#define HCI_ULP_READ_PEER_RESOLVABLE_ADDRESS_ARG_LEN              ((uint8_t) 10)
#define HCI_ULP_READ_LOCAL_RESOLVABLE_ADDRESS_ARG_LEN             ((uint8_t) 10)
#define HCI_ULP_SET_ADDRESS_RESOLUTION_ENABLE_ARG_LEN             ((uint8_t)  4)
#define HCI_ULP_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT_ARG_LEN    ((uint8_t)  4)
#define HCI_ULP_READ_PHY_ARG_LEN                                  ((uint8_t)  8)  
#define HCI_ULP_SET_DEFAULT_PHY_ARG_LEN                           ((uint8_t)  4) 
#define HCI_ULP_ENHANCED_RECEIVER_TEST_ARG_LEN                    ((uint8_t)  4) 
#define HCI_ULP_ENHANCED_TRANSMITTER_TEST_ARG_LEN                 ((uint8_t)  4) 
#define HCI_ULP_SET_PRIVACY_MODE_ARG_LEN                          ((uint8_t)  4)

/* Some aliasses to simplify auto-generated code */
#define HCI_PERIODIC_INQUIRY_MODE_ARG_LEN \
    HCI_PERIODIC_INQ_MODE_ARG_LEN
#define HCI_EXIT_PERIODIC_INQUIRY_MODE_ARG_LEN \
    HCI_EXIT_PERIODIC_INQ_MODE_ARG_LEN
#define HCI_READ_NUM_BCAST_RETXS_ARG_LEN \
    HCI_READ_NUM_BCASTXS_ARG_LEN
#define HCI_WRITE_NUM_BCAST_RETXS_ARG_LEN \
    HCI_WRITE_NUM_BCASTXS_ARG_LEN
#define HCI_SET_HCTOHOST_FLOW_CONTROL_ARG_LEN \
    HCI_SET_HC_TO_H_FLOW_CONTROL_ARG_LEN
#define HCI_READ_LOCAL_SUPP_FEATURES_ARG_LEN \
    HCI_READ_LOCAL_FEATURES_LEN
#define HCI_READ_LOCAL_SUPP_COMMANDS_ARG_LEN \
    HCI_READ_LOCAL_COMMANDS_LEN
#define HCI_READ_LOCAL_EXT_FEATURES_ARG_LEN \
    HCI_READ_LOCAL_EXT_FEATURES_LEN
#define HCI_ENABLE_DUT_MODE_ARG_LEN \
    HCI_ENABLE_DUT_ARG_LEN

/******************************************************************************
   Broadcast flag defines.  Active broadcast and piconet broadcast are
   not for SCO use
 *****************************************************************************/
#define HCI_BROADCAST_FLAG_ACTIVE   ((hci_connection_handle_t)0x4000)
#define HCI_BROADCAST_FLAG_PICONET  ((hci_connection_handle_t)0x8000)
#define HCI_BROADCAST_FLAG_MASK     ((hci_connection_handle_t)0xC000)
/* This would be a value of 1 over HCI */
#define HCI_NBC_DEFAULT             (2)

/******************************************************************************
   Packet boundary flag defines, these are already 'bit shifted'
   These are for use with ACL only, for SCO use 00 (reserved)
   The pbf is 2 bits, a start flag can be 00 or 10 ie bit 0 = 0 , the
   continuation flag is bit 0 = 1. So when testing for a start flag, can use the
   the logic that if it is not a continuation flag it must be a start one.
 *****************************************************************************/
#define HCI_PKT_BOUNDARY_FLAG_START_NONFLUSH ((hci_packet_boundary_flag_t)0)
#define HCI_PKT_BOUNDARY_FLAG_CONT           ((hci_packet_boundary_flag_t)0x1000)
#define HCI_PKT_BOUNDARY_FLAG_START_FLUSH    ((hci_packet_boundary_flag_t)0x2000)
#define HCI_PKT_BOUNDARY_MASK                ((hci_packet_boundary_flag_t)0x3000)

#define HCI_CONNECTION_HANDLE_MASK     ((hci_connection_handle_t)0x0FFF)

/******************************************************************************
    HCI Successful return value
 *****************************************************************************/
#define HCI_SUCCESS                         ((hci_return_t)0x00)
#define HCI_COMMAND_CURRENTLY_PENDING       ((hci_return_t)0x00)

/******************************************************************************
    HCI Error codes
 *****************************************************************************/
#define HCI_ERROR_ILLEGAL_COMMAND           ((hci_error_t)0x01)
#define HCI_ERROR_NO_CONNECTION             ((hci_error_t)0x02)
#define HCI_ERROR_HARDWARE_FAIL             ((hci_error_t)0x03)
#define HCI_ERROR_PAGE_TIMEOUT              ((hci_error_t)0x04)
#define HCI_ERROR_AUTH_FAIL                 ((hci_error_t)0x05)
#define HCI_ERROR_KEY_MISSING               ((hci_error_t)0x06)
#define HCI_ERROR_MEMORY_FULL               ((hci_error_t)0x07)
#define HCI_ERROR_CONN_TIMEOUT              ((hci_error_t)0x08)
#define HCI_ERROR_MAX_NR_OF_CONNS           ((hci_error_t)0x09)
#define HCI_ERROR_MAX_NR_OF_SCO             ((hci_error_t)0x0A)
#define HCI_ERROR_MAX_NR_OF_ACL             ((hci_error_t)0x0B)
#define HCI_ERROR_COMMAND_DISALLOWED        ((hci_error_t)0x0C)
#define HCI_ERROR_REJ_BY_REMOTE_NO_RES      ((hci_error_t)0x0D)  /* no resource */
#define HCI_ERROR_REJ_BY_REMOTE_SEC         ((hci_error_t)0x0E)  /* security violation */
#define HCI_ERROR_REJ_BY_REMOTE_PERS        ((hci_error_t)0x0F)  /* personal device */
#define HCI_ERROR_HOST_TIMEOUT              ((hci_error_t)0x10)
#define HCI_ERROR_UNSUPPORTED_FEATURE       ((hci_error_t)0x11)  /* or incorrect param value */
#define HCI_ERROR_ILLEGAL_FORMAT            ((hci_error_t)0x12)
#define HCI_ERROR_OETC_USER                 ((hci_error_t)0x13)  /* other end terminated */
#define HCI_ERROR_OETC_LOW_RESOURCE         ((hci_error_t)0x14)  /* other end terminated */
#define HCI_ERROR_OETC_POWERING_OFF         ((hci_error_t)0x15)  /* other end terminated */
#define HCI_ERROR_CONN_TERM_LOCAL_HOST      ((hci_error_t)0x16)  /* local host terminated */
#define HCI_ERROR_AUTH_REPEATED             ((hci_error_t)0x17)
#define HCI_ERROR_PAIRING_NOT_ALLOWED       ((hci_error_t)0x18)
#define HCI_ERROR_UNKNOWN_LMP_PDU           ((hci_error_t)0x19)
#define HCI_ERROR_UNSUPPORTED_REM_FEATURE   ((hci_error_t)0x1A) /* HCI_ERROR_UNSUPPORTED_LMP_FEATURE */
#define HCI_ERROR_SCO_OFFSET_REJECTED       ((hci_error_t)0x1B)
#define HCI_ERROR_SCO_INTERVAL_REJECTED     ((hci_error_t)0x1C)
#define HCI_ERROR_SCO_AIR_MODE_REJECTED     ((hci_error_t)0x1D)
#define HCI_ERROR_INVALID_LMP_PARAMETERS    ((hci_error_t)0x1E)
#define HCI_ERROR_UNSPECIFIED               ((hci_error_t)0x1F)
#define HCI_ERROR_UNSUPP_LMP_PARAM          ((hci_error_t)0x20)
#define HCI_ERROR_ROLE_CHANGE_NOT_ALLOWED   ((hci_error_t)0x21)
#define HCI_ERROR_LMP_RESPONSE_TIMEOUT      ((hci_error_t)0x22)
#define HCI_ERROR_LMP_TRANSACTION_COLLISION ((hci_error_t)0x23)
#define HCI_ERROR_LMP_PDU_NOT_ALLOWED       ((hci_error_t)0x24)
#define HCI_ERROR_ENC_MODE_NOT_ACCEPTABLE   ((hci_error_t)0x25)
#define HCI_ERROR_UNIT_KEY_USED             ((hci_error_t)0x26)
#define HCI_ERROR_QOS_NOT_SUPPORTED         ((hci_error_t)0x27)
#define HCI_ERROR_INSTANT_PASSED            ((hci_error_t)0x28)
#define HCI_ERROR_PAIR_UNIT_KEY_NO_SUPPORT  ((hci_error_t)0x29)
#define HCI_ERROR_DIFFERENT_TRANSACTION_COLLISION  ((hci_error_t)0x2A)
#define HCI_ERROR_SCM_INSUFFICIENT_RESOURCES ((hci_error_t)0x2B)
#define HCI_ERROR_QOS_UNACCEPTABLE_PARAMETER ((hci_error_t)0x2C)
#define HCI_ERROR_QOS_REJECTED              ((hci_error_t)0x2D)
#define HCI_ERROR_CHANNEL_CLASS_NO_SUPPORT  ((hci_error_t)0x2E)
#define HCI_ERROR_INSUFFICIENT_SECURITY     ((hci_error_t)0x2F)
#define HCI_ERROR_PARAM_OUT_OF_MAND_RANGE   ((hci_error_t)0x30)
#define HCI_ERROR_SCM_NO_LONGER_REQD        ((hci_error_t)0x31)
#define HCI_ERROR_ROLE_SWITCH_PENDING       ((hci_error_t)0x32)
#define HCI_ERROR_SCM_PARAM_CHANGE_PENDING  ((hci_error_t)0x33)
#define HCI_ERROR_RESVD_SLOT_VIOLATION      ((hci_error_t)0x34)
#define HCI_ERROR_ROLE_SWITCH_FAILED        ((hci_error_t)0x35)
#define HCI_ERROR_INQUIRY_RESPONSE_DATA_TOO_LARGE ((hci_error_t)0x36)
#define HCI_ERROR_SP_NOT_SUPPORTED_BY_HOST  ((hci_error_t)0x37)
#define HCI_ERROR_HOST_BUSY_PAIRING         ((hci_error_t)0x38)
#define HCI_ERROR_CONN_REJ_NO_SUITABLE_CHANNEL_FOUND   ((hci_error_t)0x39)
#define HCI_ERROR_CONTROLLER_BUSY                      ((hci_error_t)0x3A)
#define HCI_ERROR_UNACCEPTABLE_CONN_PARAMETERS         ((hci_error_t)0x3B)
#define HCI_ERROR_DIRECTED_ADVERTISING_TIMEOUT         ((hci_error_t)0x3C)
#define HCI_ERROR_CONN_TERMINATED_DUE_TO_MIC_FAILURE   ((hci_error_t)0x3D)
#define HCI_ERROR_CONNECTION_FAILED_TO_BE_ESTABLISHED  ((hci_error_t)0x3E)

 /*****************************************************************************
 *
 *  CSB Error codes
 *
 *****************************************************************************/

#define CSB_RX_FAIL ((hci_error_t)1)

 /*****************************************************************************
 *
 *  HCI miscellaneous common fields
 *
 *****************************************************************************/

#define HCI_DEFAULT        ((uint8_t)0)     /* 0 in a message field means default */
                                            /* however, the interpretation
                                               varies depending on the message */
#define HCI_MASTER         ((hci_role_t)0)
#define HCI_SLAVE          ((hci_role_t)1)

#define HCI_MASTER_SLAVE_UNKNOWN          ((hci_role_t)2)

#define HCI_LINK_KEY_SEMIPERM  ((hci_key_flag_t)0)
#define HCI_LINK_KEY_TEMP      ((hci_key_flag_t)1)

#define HCI_LINK_ENC_OFF       ((hci_link_enc_t)0)
#define HCI_LINK_ENC_ON        ((hci_link_enc_t)1)

#define HCI_QOS_NO_TRAFFIC     ((hci_qos_type_t)0)
#define HCI_QOS_BEST_EFFORT    ((hci_qos_type_t)1)
#define HCI_QOS_GUARANTEED     ((hci_qos_type_t)2)

/*----------------------------------------------------------------------------*
 *
 *  HCI packet types
 *
 *---------------------------------------------------------------------------*/
#define HCI_PKT_DM1       ((hci_pkt_type_t)0x0008)    /* SCO and ACL */
#define HCI_PKT_DH1       ((hci_pkt_type_t)0x0010)    /* ACL only */
#define HCI_PKT_HV1       ((hci_pkt_type_t)0x0020)    /* SCO only */
#define HCI_PKT_HV2       ((hci_pkt_type_t)0x0040)    /* SCO only */
#define HCI_PKT_HV3       ((hci_pkt_type_t)0x0080)    /* SCO only */
#define HCI_PKT_DV        ((hci_pkt_type_t)0x0100)    /* SCO only */
#define HCI_PKT_AUX1      ((hci_pkt_type_t)0x0200)    /* ACL only NOT L2CAP */
#define HCI_PKT_DM3       ((hci_pkt_type_t)0x0400)    /* ACL only */
#define HCI_PKT_DH3       ((hci_pkt_type_t)0x0800)    /* ACL only */
#define HCI_PKT_DM5       ((hci_pkt_type_t)0x4000)    /* ACL only */
#define HCI_PKT_DH5       ((hci_pkt_type_t)0x8000)    /* ACL only */

#define HCI_PKT_HV123           ((hci_pkt_type_t)0x00E0)    /* All SCO only */
#define HCI_PKT_HV123_INVERSE   ((hci_pkt_type_t)0xFF1F)    /* All SCO only */

#define HCI_PKT_2DH1      ((hci_pkt_type_t)0x0002)    /* ACL only */
#define HCI_PKT_3DH1      ((hci_pkt_type_t)0x0004)    /* ACL only */
#define HCI_PKT_2DH3      ((hci_pkt_type_t)0x0100)    /* ACL only */
#define HCI_PKT_3DH3      ((hci_pkt_type_t)0x0200)    /* ACL only */
#define HCI_PKT_2DH5      ((hci_pkt_type_t)0x1000)    /* ACL only */
#define HCI_PKT_3DH5      ((hci_pkt_type_t)0x2000)    /* ACL only */

#define HCI_PKT_2MBP_LIST ((hci_pkt_type_t)HCI_PKT_2DH1|HCI_PKT_2DH3|HCI_PKT_2DH5)
#define HCI_PKT_3MBP_LIST ((hci_pkt_type_t)HCI_PKT_3DH1|HCI_PKT_3DH3|HCI_PKT_3DH5)
#define HCI_PKT_MR_1_SLOT_LIST ((hci_pkt_type_t)HCI_PKT_2DH1|HCI_PKT_3DH1)
#define HCI_PKT_MR_3_SLOT_LIST ((hci_pkt_type_t)HCI_PKT_2DH3|HCI_PKT_3DH3)
#define HCI_PKT_MR_5_SLOT_LIST ((hci_pkt_type_t)HCI_PKT_2DH5|HCI_PKT_3DH5)
#define HCI_PKT_MR_LIST ((hci_pkt_type_t)HCI_PKT_2MBP_LIST|HCI_PKT_3MBP_LIST)
#define HCI_PKT_3_SLOT_LIST ((hci_pkt_type_t)HCI_PKT_DM3|HCI_PKT_DH3)
#define HCI_PKT_5_SLOT_LIST ((hci_pkt_type_t)HCI_PKT_DM5|HCI_PKT_DH5)

/* This is the ESCO pkt type definitions */
#define HCI_ESCO_PKT_HV1   ((hci_pkt_type_t)0x0001)    /* eSCO only */
#define HCI_ESCO_PKT_HV2   ((hci_pkt_type_t)0x0002)    /* eSCO only */
#define HCI_ESCO_PKT_HV3   ((hci_pkt_type_t)0x0004)    /* eSCO only */
#define HCI_ESCO_PKT_EV3   ((hci_pkt_type_t)0x0008)    /* eSCO only */
#define HCI_ESCO_PKT_EV4   ((hci_pkt_type_t)0x0010)    /* eSCO only */
#define HCI_ESCO_PKT_EV5   ((hci_pkt_type_t)0x0020)    /* eSCO only */

/* bits 0x01,0x02 & 0x04 to 0x20, 0x40 & 0x80 */
#define ESCO_TO_HCI_PKT_BIT_SHIFT  5

/* Medium Rate eSCO */
#define HCI_ESCO_PKT_2EV3  ((hci_pkt_type_t)0x0040)    /* eSCO only */
#define HCI_ESCO_PKT_3EV3  ((hci_pkt_type_t)0x0080)    /* eSCO only */
#define HCI_ESCO_PKT_2EV5  ((hci_pkt_type_t)0x0100)    /* eSCO only */
#define HCI_ESCO_PKT_3EV5  ((hci_pkt_type_t)0x0200)    /* eSCO only */

#define HCI_ALL_MR_ESCO    ((hci_pkt_type_t)HCI_ESCO_PKT_2EV3 | HCI_ESCO_PKT_3EV3 | HCI_ESCO_PKT_2EV5 | HCI_ESCO_PKT_3EV5)
#define HCI_ALL_ESCO       ((hci_pkt_type_t)HCI_ESCO_PKT_EV3 | HCI_ESCO_PKT_EV4 | HCI_ESCO_PKT_EV5 | HCI_ALL_MR_ESCO)

/* eSCO Retransmission effort */
#define HCI_ESCO_NO_RETX            ((uint8_t) 0x00)
#define HCI_ESCO_POWER_SAVING_RETX  ((uint8_t) 0x01)
#define HCI_ESCO_LINK_QUALITY_RETX  ((uint8_t) 0x02)
#define HCI_ESCO_DONT_CARE_RETX     ((uint8_t) 0xFF)

/* Bandwidth defaults */
#define HCI_ESCO_BDW_DONT_CARE      ((uint32_t) 0xFFFFFFFF)

/* Max latency default */
#define HCI_ESCO_MAX_LATENCY_DEFAULT ((uint16_t) 0xFFFF)

/* Packet type defaults */
#define HCI_ESCO_DEFAULT_PKT_TYPE   ((uint16_t) 0xFFFF)

/*----------------------------------------------------------------------------*
 *
 *  HCI event mask values
 *  As 8 byte quantity split into two uint32_t so require high and low
 *  part masks.
 *  NB: as currently assigned, these are a simple function of the
 *  corresponding HCI event code (HCI_EV_*).
 *
 *---------------------------------------------------------------------------*/

/*
 * Some compilers complain about shifts by out of range values (negative or
 * more than 32) even if they're on dead code paths. Strictly, this complaint
 * is not necessary as even though such a shift produces undefined behaviour,
 * the ANSI C spec explicitly states that this is not a problem on code paths
 * that aren't taken because of a constant expression in a short-circuit
 * operator. See section 6.4 and particularly footnote 55.
 *
 * However, it's better if header files don't emit warnings so to keep such
 * compilers happy we add a second test. At the end of the day, the expression
 * value is still constant so it doesn't cost us anything. Since the same test
 * is used in more than one place, it's placed in a macro to avoid repetition.
 */

#define HCI_EV_MASK_SHIFT_IS_SAFE(amount) ((amount) >= 0 && (amount) < 32)

#define HCI_EV_MASK_SAFE_SINGLE_BIT_MASK(amount) \
    ((hci_event_mask_t) \
     (HCI_EV_MASK_SHIFT_IS_SAFE(amount) ? \
      1UL << (HCI_EV_MASK_SHIFT_IS_SAFE(amount) ? (amount) : 0) : \
      0))

/* Note, event 1 ends up in bit 0, hence the extra -1 in HCI_EV_MASK_HI and
   HCI_EV_MASK_LO */

#define HCI_EV_MASK_HI(x) HCI_EV_MASK_SAFE_SINGLE_BIT_MASK((int) (x) - 33)
#define HCI_EV_MASK_LO(x) HCI_EV_MASK_SAFE_SINGLE_BIT_MASK((int) (x) - 1)

/* For page 2 event mask, event 64 ends up in bit 0, hence -64 for
   HCI_EV_MASK_LO2 and -96 for HCI_EV_MASK_HI2 */

#define HCI_EV_MASK_HI2(x) HCI_EV_MASK_SAFE_SINGLE_BIT_MASK((int) (x) - 96)
#define HCI_EV_MASK_LO2(x) HCI_EV_MASK_SAFE_SINGLE_BIT_MASK((int) (x) - 64)

#define HCI_EV_MASK_MSB                     ((hci_event_mask_t)0x00000000)
#define HCI_EV_MASK_NONE_HI                 ((hci_event_mask_t)0x00000000)
#define HCI_EV_MASK_NONE_LO                 ((hci_event_mask_t)0x00000000)
#define HCI_EV_MASK_INQUIRY_COMPLETE_HI     HCI_EV_MASK_HI(HCI_EV_INQUIRY_COMPLETE)
#define HCI_EV_MASK_INQUIRY_COMPLETE_LO     HCI_EV_MASK_LO(HCI_EV_INQUIRY_COMPLETE)
#define HCI_EV_MASK_INQUIRY_RESULT_HI       HCI_EV_MASK_HI(HCI_EV_INQUIRY_RESULT)
#define HCI_EV_MASK_INQUIRY_RESULT_LO       HCI_EV_MASK_LO(HCI_EV_INQUIRY_RESULT)
#define HCI_EV_MASK_CONN_COMPLETE_HI        HCI_EV_MASK_HI(HCI_EV_CONN_COMPLETE)
#define HCI_EV_MASK_CONN_COMPLETE_LO        HCI_EV_MASK_LO(HCI_EV_CONN_COMPLETE)
#define HCI_EV_MASK_CONN_REQ_HI             HCI_EV_MASK_HI(HCI_EV_CONN_REQUEST)
#define HCI_EV_MASK_CONN_REQ_LO             HCI_EV_MASK_LO(HCI_EV_CONN_REQUEST)
#define HCI_EV_MASK_DISC_COMPLETE_HI        HCI_EV_MASK_HI(HCI_EV_DISCONNECT_COMPLETE)
#define HCI_EV_MASK_DISC_COMPLETE_LO        HCI_EV_MASK_LO(HCI_EV_DISCONNECT_COMPLETE)
#define HCI_EV_MASK_AUTH_COMPLETE_HI        HCI_EV_MASK_HI(HCI_EV_AUTH_COMPLETE)
#define HCI_EV_MASK_AUTH_COMPLETE_LO        HCI_EV_MASK_LO(HCI_EV_AUTH_COMPLETE)
#define HCI_EV_MASK_REM_NAME_REQ_COMP_HI    HCI_EV_MASK_HI(HCI_EV_REMOTE_NAME_REQ_COMPLETE)
#define HCI_EV_MASK_REM_NAME_REQ_COMP_LO    HCI_EV_MASK_LO(HCI_EV_REMOTE_NAME_REQ_COMPLETE)
#define HCI_EV_MASK_CHG_CONN_ENC_EN_HI      HCI_EV_MASK_HI(HCI_EV_ENCRYPTION_CHANGE)
#define HCI_EV_MASK_CHG_CONN_ENC_EN_LO      HCI_EV_MASK_LO(HCI_EV_ENCRYPTION_CHANGE)
#define HCI_EV_MASK_CHG_CONN_LINK_KEY_HI    HCI_EV_MASK_HI(HCI_EV_CHANGE_CONN_LINK_KEY_COMPLETE)
#define HCI_EV_MASK_CHG_CONN_LINK_KEY_LO    HCI_EV_MASK_LO(HCI_EV_CHANGE_CONN_LINK_KEY_COMPLETE)
#define HCI_EV_MASK_MASTER_LINK_KEY_HI      HCI_EV_MASK_HI(HCI_EV_MASTER_LINK_KEY_COMPLETE)
#define HCI_EV_MASK_MASTER_LINK_KEY_LO      HCI_EV_MASK_LO(HCI_EV_MASTER_LINK_KEY_COMPLETE)
#define HCI_EV_MASK_READ_REM_SUPP_FEAT_HI   HCI_EV_MASK_HI(HCI_EV_READ_REM_SUPP_FEATURES_COMPLETE)
#define HCI_EV_MASK_READ_REM_SUPP_FEAT_LO   HCI_EV_MASK_LO(HCI_EV_READ_REM_SUPP_FEATURES_COMPLETE)
#define HCI_EV_MASK_READ_REM_EXT_FEAT_HI    HCI_EV_MASK_HI(HCI_EV_READ_REM_EXT_FEATURES_COMPLETE)
#define HCI_EV_MASK_READ_REM_EXT_FEAT_LO    HCI_EV_MASK_LO(HCI_EV_READ_REM_EXT_FEATURES_COMPLETE)
#define HCI_EV_MASK_READ_REM_VER_INFO_HI    HCI_EV_MASK_HI(HCI_EV_READ_REMOTE_VER_INFO_COMPLETE)
#define HCI_EV_MASK_READ_REM_VER_INFO_LO    HCI_EV_MASK_LO(HCI_EV_READ_REMOTE_VER_INFO_COMPLETE)
#define HCI_EV_MASK_QOS_SETUP_COMP_HI       HCI_EV_MASK_HI(HCI_EV_QOS_SETUP_COMPLETE)
#define HCI_EV_MASK_QOS_SETUP_COMP_LO       HCI_EV_MASK_LO(HCI_EV_QOS_SETUP_COMPLETE)
#define HCI_EV_MASK_COMMAND_COMPLETE_HI     HCI_EV_MASK_HI(HCI_EV_COMMAND_COMPLETE)
#define HCI_EV_MASK_COMMAND_COMPLETE_LO     HCI_EV_MASK_LO(HCI_EV_COMMAND_COMPLETE)
#define HCI_EV_MASK_COMMAND_STATUS_HI       HCI_EV_MASK_HI(HCI_EV_COMMAND_STATUS)
#define HCI_EV_MASK_COMMAND_STATUS_LO       HCI_EV_MASK_LO(HCI_EV_COMMAND_STATUS)
#define HCI_EV_MASK_HARDWARE_ERROR_HI       HCI_EV_MASK_HI(HCI_EV_HARDWARE_ERROR)
#define HCI_EV_MASK_HARDWARE_ERROR_LO       HCI_EV_MASK_LO(HCI_EV_HARDWARE_ERROR)
#define HCI_EV_MASK_FLUSH_OCCURRED_EV_HI    HCI_EV_MASK_HI(HCI_EV_FLUSH_OCCURRED)
#define HCI_EV_MASK_FLUSH_OCCURRED_EV_LO    HCI_EV_MASK_LO(HCI_EV_FLUSH_OCCURRED)
#define HCI_EV_MASK_ROLE_CHANGE_HI          HCI_EV_MASK_HI(HCI_EV_ROLE_CHANGE)
#define HCI_EV_MASK_ROLE_CHANGE_LO          HCI_EV_MASK_LO(HCI_EV_ROLE_CHANGE)
#define HCI_EV_MASK_NUM_HCI_DATA_PKTS_HI    HCI_EV_MASK_HI(HCI_EV_NUMBER_COMPLETED_PKTS)
#define HCI_EV_MASK_NUM_HCI_DATA_PKTS_LO    HCI_EV_MASK_LO(HCI_EV_NUMBER_COMPLETED_PKTS)
#define HCI_EV_MASK_MODE_CHANGE_HI          HCI_EV_MASK_HI(HCI_EV_MODE_CHANGE)
#define HCI_EV_MASK_MODE_CHANGE_LO          HCI_EV_MASK_LO(HCI_EV_MODE_CHANGE)
#define HCI_EV_MASK_RETURN_LINK_KEYS_HI     HCI_EV_MASK_HI(HCI_EV_RETURN_LINK_KEYS)
#define HCI_EV_MASK_RETURN_LINK_KEYS_LO     HCI_EV_MASK_LO(HCI_EV_RETURN_LINK_KEYS)
#define HCI_EV_MASK_PIN_CODE_REQ_HI         HCI_EV_MASK_HI(HCI_EV_PIN_CODE_REQ)
#define HCI_EV_MASK_PIN_CODE_REQ_LO         HCI_EV_MASK_LO(HCI_EV_PIN_CODE_REQ)
#define HCI_EV_MASK_LINK_KEY_REQ_HI         HCI_EV_MASK_HI(HCI_EV_LINK_KEY_REQ)
#define HCI_EV_MASK_LINK_KEY_REQ_LO         HCI_EV_MASK_LO(HCI_EV_LINK_KEY_REQ)
#define HCI_EV_MASK_LINK_KEY_NOTIFY_HI      HCI_EV_MASK_HI(HCI_EV_LINK_KEY_NOTIFICATION)
#define HCI_EV_MASK_LINK_KEY_NOTIFY_LO      HCI_EV_MASK_LO(HCI_EV_LINK_KEY_NOTIFICATION)
#define HCI_EV_MASK_LOOPBACK_COMMAND_HI     HCI_EV_MASK_HI(HCI_EV_LOOPBACK_COMMAND)
#define HCI_EV_MASK_LOOPBACK_COMMAND_LO     HCI_EV_MASK_LO(HCI_EV_LOOPBACK_COMMAND)
#define HCI_EV_MASK_DATA_BUFFER_OVERFLOW_HI HCI_EV_MASK_HI(HCI_EV_DATA_BUFFER_OVERFLOW)
#define HCI_EV_MASK_DATA_BUFFER_OVERFLOW_LO HCI_EV_MASK_LO(HCI_EV_DATA_BUFFER_OVERFLOW)
#define HCI_EV_MASK_MAX_SLOTS_CHANGE_HI     HCI_EV_MASK_HI(HCI_EV_MAX_SLOTS_CHANGE)
#define HCI_EV_MASK_MAX_SLOTS_CHANGE_LO     HCI_EV_MASK_LO(HCI_EV_MAX_SLOTS_CHANGE)
#define HCI_EV_MASK_READ_CLOCK_OFFSET_HI    HCI_EV_MASK_HI(HCI_EV_READ_CLOCK_OFFSET_COMPLETE)
#define HCI_EV_MASK_READ_CLOCK_OFFSET_LO    HCI_EV_MASK_LO(HCI_EV_READ_CLOCK_OFFSET_COMPLETE)
#define HCI_EV_MASK_CONN_PKT_TYPE_HI        HCI_EV_MASK_HI(HCI_EV_CONN_PACKET_TYPE_CHANGED)
#define HCI_EV_MASK_CONN_PKT_TYPE_LO        HCI_EV_MASK_LO(HCI_EV_CONN_PACKET_TYPE_CHANGED)
#define HCI_EV_MASK_QOS_VIOLATION_HI        HCI_EV_MASK_HI(HCI_EV_QOS_VIOLATION)
#define HCI_EV_MASK_QOS_VIOLATION_LO        HCI_EV_MASK_LO(HCI_EV_QOS_VIOLATION)
#define HCI_EV_MASK_PAGE_SCAN_MODE_HI       HCI_EV_MASK_HI(HCI_EV_PAGE_SCAN_MODE_CHANGE)
#define HCI_EV_MASK_PAGE_SCAN_MODE_LO       HCI_EV_MASK_LO(HCI_EV_PAGE_SCAN_MODE_CHANGE)
#define HCI_EV_MASK_PAGE_SCAN_REP_MODE_HI   HCI_EV_MASK_HI(HCI_EV_PAGE_SCAN_REP_MODE_CHANGE)
#define HCI_EV_MASK_PAGE_SCAN_REP_MODE_LO   HCI_EV_MASK_LO(HCI_EV_PAGE_SCAN_REP_MODE_CHANGE)


/* 1.2 Features */
#define HCI_EV_MASK_FLOW_SPEC_COMPLETE_HI    HCI_EV_MASK_HI(HCI_EV_FLOW_SPEC_COMPLETE)
#define HCI_EV_MASK_FLOW_SPEC_COMPLETE_LO    HCI_EV_MASK_LO(HCI_EV_FLOW_SPEC_COMPLETE)
#define HCI_EV_MASK_INQUIRY_RESULT_RSSI_HI   HCI_EV_MASK_HI(HCI_EV_INQUIRY_RESULT_WITH_RSSI)
#define HCI_EV_MASK_INQUIRY_RESULT_RSSI_LO   HCI_EV_MASK_LO(HCI_EV_INQUIRY_RESULT_WITH_RSSI)
#define HCI_EV_MASK_RR_EXT_FEATURES_HI       HCI_EV_MASK_HI(HCI_EV_READ_REM_EXT_FEATURES_COMPLETE)
#define HCI_EV_MASK_RR_EXT_FEATURES_LO       HCI_EV_MASK_LO(HCI_EV_READ_REM_EXT_FEATURES_COMPLETE)
#define HCI_EV_MASK_FIXED_ADDRESS_HI         HCI_EV_MASK_HI(HCI_EV_FIXED_ADDRESS)
#define HCI_EV_MASK_FIXED_ADDRESS_LO         HCI_EV_MASK_LO(HCI_EV_FIXED_ADDRESS)
#define HCI_EV_MASK_ALIAS_ADDRESS_HI         HCI_EV_MASK_HI(HCI_EV_ALIAS_ADDRESS)
#define HCI_EV_MASK_ALIAS_ADDRESS_LO         HCI_EV_MASK_LO(HCI_EV_ALIAS_ADDRESS)
#define HCI_EV_MASK_GENERATE_ALIAS_REQ_HI    HCI_EV_MASK_HI(HCI_EV_GENERATE_ALIAS_REQ)
#define HCI_EV_MASK_GENERATE_ALIAS_REQ_LO    HCI_EV_MASK_LO(HCI_EV_GENERATE_ALIAS_REQ)
#define HCI_EV_MASK_ACTIVE_ADDRESS_HI        HCI_EV_MASK_HI(HCI_EV_ACTIVE_ADDRESS)
#define HCI_EV_MASK_ACTIVE_ADDRESS_LO        HCI_EV_MASK_LO(HCI_EV_ACTIVE_ADDRESS)
#define HCI_EV_MASK_ALLOW_PRIVATE_PAIRING_HI HCI_EV_MASK_HI(HCI_EV_ALLOW_PRIVATE_PAIRING)
#define HCI_EV_MASK_ALLOW_PRIVATE_PAIRING_LO HCI_EV_MASK_LO(HCI_EV_ALLOW_PRIVATE_PAIRING)
#define HCI_EV_MASK_ALIAS_ADDRESS_REQ_HI     HCI_EV_MASK_HI(HCI_EV_ALIAS_ADDRESS_REQ)
#define HCI_EV_MASK_ALIAS_ADDRESS_REQ_LO     HCI_EV_MASK_LO(HCI_EV_ALIAS_ADDRESS_REQ)
#define HCI_EV_MASK_ALIAS_NOT_RECOGNISED_HI  HCI_EV_MASK_HI(HCI_EV_ALIAS_NOT_RECOGNISED)
#define HCI_EV_MASK_ALIAS_NOT_RECOGNISED_LO  HCI_EV_MASK_LO(HCI_EV_ALIAS_NOT_RECOGNISED)
#define HCI_EV_MASK_FIXED_ADDRESS_ATTEMPT_HI HCI_EV_MASK_HI(HCI_EV_FIXED_ADDRESS_ATTEMPT)
#define HCI_EV_MASK_FIXED_ADDRESS_ATTEMPT_LO HCI_EV_MASK_LO(HCI_EV_FIXED_ADDRESS_ATTEMPT)
#define HCI_EV_MASK_SYNC_CONN_COMP_HI        HCI_EV_MASK_HI(HCI_EV_SYNC_CONN_COMPLETE)
#define HCI_EV_MASK_SYNC_CONN_COMP_LO        HCI_EV_MASK_LO(HCI_EV_SYNC_CONN_COMPLETE)
#define HCI_EV_MASK_SYNC_CONN_CHANGED_HI     HCI_EV_MASK_HI(HCI_EV_SYNC_CONN_CHANGED)
#define HCI_EV_MASK_SYNC_CONN_CHANGED_LO     HCI_EV_MASK_LO(HCI_EV_SYNC_CONN_CHANGED)

/* 2.1 Features */
#define HCI_EV_MASK_SNIFF_SUB_RATE_HI               HCI_EV_MASK_HI(HCI_EV_SNIFF_SUB_RATE)
#define HCI_EV_MASK_SNIFF_SUB_RATE_LO               HCI_EV_MASK_LO(HCI_EV_SNIFF_SUB_RATE)
#define HCI_EV_MASK_EXTENDED_INQUIRY_RES_HI         HCI_EV_MASK_HI(HCI_EV_EXTENDED_INQUIRY_RESULT)
#define HCI_EV_MASK_EXTENDED_INQUIRY_RES_LO         HCI_EV_MASK_LO(HCI_EV_EXTENDED_INQUIRY_RESULT)
#define HCI_EV_MASK_ENCRYPTION_KEY_REFRESH_COMPLETE_LO   HCI_EV_MASK_LO(HCI_EV_ENCRYPTION_KEY_REFRESH_COMPLETE)
#define HCI_EV_MASK_ENCRYPTION_KEY_REFRESH_COMPLETE_HI   HCI_EV_MASK_HI(HCI_EV_ENCRYPTION_KEY_REFRESH_COMPLETE)
#define HCI_EV_MASK_IO_CAPABILITY_REQUEST_HI        HCI_EV_MASK_HI(HCI_EV_IO_CAPABILITY_REQUEST)
#define HCI_EV_MASK_IO_CAPABILITY_REQUEST_LO        HCI_EV_MASK_LO(HCI_EV_IO_CAPABILITY_REQUEST)
#define HCI_EV_MASK_IO_CAPABILITY_RESPONSE_HI       HCI_EV_MASK_HI(HCI_EV_IO_CAPABILITY_RESPONSE)
#define HCI_EV_MASK_IO_CAPABILITY_RESPONSE_LO       HCI_EV_MASK_LO(HCI_EV_IO_CAPABILITY_RESPONSE)
#define HCI_EV_MASK_USER_CONFIRMATION_REQUEST_HI    HCI_EV_MASK_HI(HCI_EV_USER_CONFIRMATION_REQUEST)
#define HCI_EV_MASK_USER_CONFIRMATION_REQUEST_LO    HCI_EV_MASK_LO(HCI_EV_USER_CONFIRMATION_REQUEST)
#define HCI_EV_MASK_USER_PASSKEY_REQUEST_HI         HCI_EV_MASK_HI(HCI_EV_USER_PASSKEY_REQUEST)
#define HCI_EV_MASK_USER_PASSKEY_REQUEST_LO         HCI_EV_MASK_LO(HCI_EV_USER_PASSKEY_REQUEST)
#define HCI_EV_MASK_REMOTE_OOB_DATA_REQUEST_HI      HCI_EV_MASK_HI(HCI_EV_REMOTE_OOB_DATA_REQUEST)
#define HCI_EV_MASK_REMOTE_OOB_DATA_REQUEST_LO      HCI_EV_MASK_LO(HCI_EV_REMOTE_OOB_DATA_REQUEST)
#define HCI_EV_MASK_SIMPLE_PAIRING_COMPLETE_HI      HCI_EV_MASK_HI(HCI_EV_SIMPLE_PAIRING_COMPLETE)
#define HCI_EV_MASK_SIMPLE_PAIRING_COMPLETE_LO      HCI_EV_MASK_LO(HCI_EV_SIMPLE_PAIRING_COMPLETE)
#define HCI_EV_MASK_LST_CHANGE_HI                   HCI_EV_MASK_HI(HCI_EV_LST_CHANGE)
#define HCI_EV_MASK_LST_CHANGE_LO                   HCI_EV_MASK_LO(HCI_EV_LST_CHANGE)
#define HCI_EV_MASK_ENHANCED_FLUSH_COMPLETE_EV_LO   HCI_EV_MASK_LO(HCI_EV_ENHANCED_FLUSH_COMPLETE)
#define HCI_EV_MASK_ENHANCED_FLUSH_COMPLETE_EV_HI   HCI_EV_MASK_HI(HCI_EV_ENHANCED_FLUSH_COMPLETE)
#define HCI_EV_MASK_USER_PASSKEY_NOTIFICATION_HI    HCI_EV_MASK_HI(HCI_EV_USER_PASSKEY_NOTIFICATION)
#define HCI_EV_MASK_USER_PASSKEY_NOTIFICATION_LO    HCI_EV_MASK_LO(HCI_EV_USER_PASSKEY_NOTIFICATION)
#define HCI_EV_MASK_KEYPRESS_NOTIFICATION_HI        HCI_EV_MASK_HI(HCI_EV_KEYPRESS_NOTIFICATION)
#define HCI_EV_MASK_KEYPRESS_NOTIFICATION_LO        HCI_EV_MASK_LO(HCI_EV_KEYPRESS_NOTIFICATION)
#define HCI_EV_MASK_REM_HOST_SUPPORTED_FEATURES_HI  HCI_EV_MASK_HI(HCI_EV_REM_HOST_SUPPORTED_FEATURES)
#define HCI_EV_MASK_REM_HOST_SUPPORTED_FEATURES_LO  HCI_EV_MASK_LO(HCI_EV_REM_HOST_SUPPORTED_FEATURES)

#define HCI_EV_MASK_ULP_HI                          HCI_EV_MASK_HI(HCI_EV_ULP)
#define HCI_EV_MASK_ULP_LO                          HCI_EV_MASK_LO(HCI_EV_ULP)
#define HCI_EV_TRIGGERED_CLOCK_CAPTURE_HI           HCI_EV_MASK_HI2(HCI_EV_TRIGGERED_CLOCK_CAPTURE)
#define HCI_EV_TRIGGERED_CLOCK_CAPTURE_LO           HCI_EV_MASK_LO2(HCI_EV_TRIGGERED_CLOCK_CAPTURE)
#define HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE_HI    HCI_EV_MASK_HI2(HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE)
#define HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE_LO    HCI_EV_MASK_LO2(HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE)
#define HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED_HI    HCI_EV_MASK_HI2(HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED)
#define HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED_LO    HCI_EV_MASK_LO2(HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED)
#define HCI_EV_CSB_RECEIVE_HI                       HCI_EV_MASK_HI2(HCI_EV_CSB_RECEIVE)
#define HCI_EV_CSB_RECEIVE_LO                       HCI_EV_MASK_LO2(HCI_EV_CSB_RECEIVE)
#define HCI_EV_CSB_TIMEOUT_HI                       HCI_EV_MASK_HI2(HCI_EV_CSB_TIMEOUT)
#define HCI_EV_CSB_TIMEOUT_LO                       HCI_EV_MASK_LO2(HCI_EV_CSB_TIMEOUT)
#define HCI_EV_TRUNCATED_PAGE_COMPLETE_HI           HCI_EV_MASK_HI2(HCI_EV_TRUNCATED_PAGE_COMPLETE)
#define HCI_EV_TRUNCATED_PAGE_COMPLETE_LO           HCI_EV_MASK_LO2(HCI_EV_TRUNCATED_PAGE_COMPLETE)
#define HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT_HI       HCI_EV_MASK_HI2(HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT)
#define HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT_LO       HCI_EV_MASK_LO2(HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT)
#define HCI_EV_CSB_CHANNEL_MAP_CHANGE_HI            HCI_EV_MASK_HI2(HCI_EV_CSB_CHANNEL_MAP_CHANGE)
#define HCI_EV_CSB_CHANNEL_MAP_CHANGE_LO            HCI_EV_MASK_LO2(HCI_EV_CSB_CHANNEL_MAP_CHANGE)
#define HCI_EV_INQUIRY_RESPONSE_NOTIFICATION_HI     HCI_EV_MASK_HI2(HCI_EV_INQUIRY_RESPONSE_NOTIFICATION)
#define HCI_EV_INQUIRY_RESPONSE_NOTIFICATION_LO     HCI_EV_MASK_LO2(HCI_EV_INQUIRY_RESPONSE_NOTIFICATION)
#define HCI_EV_MASK_APT_EXPIRED_HI                  HCI_EV_MASK_HI2(HCI_EV_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED)
#define HCI_EV_MASK_APT_EXPIRED_LO                  HCI_EV_MASK_LO2(HCI_EV_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED)

/* Any event introduced after Synchronous Connection Changed Event is by
   default masked so old hosts working with new controllers don't receive
   events they don't enderstand */

#define HCI_EV_MASK_DEFAULT_HI               ((hci_event_mask_t)0x00001FFF)
#define HCI_EV_MASK_DEFAULT_LO               ((hci_event_mask_t)0xFFFFFFFF)

/* All events in page 2 are masked by default */

#define HCI_EV_MASK_2_DEFAULT_HI               ((hci_event_mask_t)0x00000000)
#define HCI_EV_MASK_2_DEFAULT_LO               ((hci_event_mask_t)0x00000000)

/* RFU bits must be 0, reserved must be ignored */
#ifdef INSTALL_ULP
#define HCI_EV_MASK_VALID_HI                 ((hci_event_mask_t)0x3FFFFFFF)
#else
#define HCI_EV_MASK_VALID_HI                 ((hci_event_mask_t)0x1FFFFFFF)
#endif /* INSTALL_ULP */
#define HCI_EV_MASK_VALID_LO                 ((hci_event_mask_t)0xFFFFFFFF)

#define HCI_EV_MASK_2_VALID_HI               ((hci_event_mask_t)0x00000000)
#define HCI_EV_MASK_2_VALID_LO               ((hci_event_mask_t)0x00FFFFFF)

#define HCI_EV_MASK_VALID_HI_BT2_0           ((hci_event_mask_t)0x00001FFF)
#define HCI_EV_MASK_VALID_LO_BT2_0           ((hci_event_mask_t)0xFFFFFFFF)

/* ULP */
#define HCI_EV_MASK_ULP_DEFAULT_HI               ((hci_event_mask_t)0x00000000)
#define HCI_EV_MASK_ULP_DEFAULT_LO               ((hci_event_mask_t)0x0000001F)
#define HCI_EV_MASK_ULP_VALID_HI                 ((hci_event_mask_t)0x00000000)
#ifdef INSTALL_ULP_CHAN_SELECT
#define HCI_EV_MASK_ULP_VALID_LO                 ((hci_event_mask_t)0x000809FF)
#else
#define HCI_EV_MASK_ULP_VALID_LO                 ((hci_event_mask_t)0x000009FF)
#endif

#define HCI_EV_MASK_ULP_CONNECTION_COMPLETE_HI                 HCI_EV_MASK_HI(HCI_EV_ULP_CONNECTION_COMPLETE)
#define HCI_EV_MASK_ULP_CONNECTION_COMPLETE_LO                 HCI_EV_MASK_LO(HCI_EV_ULP_CONNECTION_COMPLETE)
#define HCI_EV_MASK_ULP_ADVERTISING_REPORT_HI                  HCI_EV_MASK_HI(HCI_EV_ULP_ADVERTISING_REPORT)
#define HCI_EV_MASK_ULP_ADVERTISING_REPORT_LO                  HCI_EV_MASK_LO(HCI_EV_ULP_ADVERTISING_REPORT)
#define HCI_EV_MASK_ULP_CONNECTION_UPDATE_COMPLETE_HI          HCI_EV_MASK_HI(HCI_EV_ULP_CONNECTION_UPDATE_COMPLETE)
#define HCI_EV_MASK_ULP_CONNECTION_UPDATE_COMPLETE_LO          HCI_EV_MASK_LO(HCI_EV_ULP_CONNECTION_UPDATE_COMPLETE)
#define HCI_EV_MASK_ULP_READ_REMOTE_USED_FEATURES_COMPLETE_HI  HCI_EV_MASK_HI(HCI_EV_ULP_READ_REMOTE_USED_FEATURES_COMPLETE)
#define HCI_EV_MASK_ULP_READ_REMOTE_USED_FEATURES_COMPLETE_LO  HCI_EV_MASK_LO(HCI_EV_ULP_READ_REMOTE_USED_FEATURES_COMPLETE)
#define HCI_EV_MASK_ULP_LONG_TERM_KEY_REQUEST_HI               HCI_EV_MASK_HI(HCI_EV_ULP_LONG_TERM_KEY_REQUEST)
#define HCI_EV_MASK_ULP_LONG_TERM_KEY_REQUEST_LO               HCI_EV_MASK_LO(HCI_EV_ULP_LONG_TERM_KEY_REQUEST)
#define HCI_EV_MASK_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_HI HCI_EV_MASK_HI(HCI_EV_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST)
#define HCI_EV_MASK_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_LO HCI_EV_MASK_LO(HCI_EV_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST)
#define HCI_EV_MASK_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE_HI    HCI_EV_MASK_HI(HCI_EV_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE)
#define HCI_EV_MASK_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE_LO    HCI_EV_MASK_LO(HCI_EV_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE)
#define HCI_EV_MASK_ULP_GENERATE_DHKEY_COMPLETE_HI             HCI_EV_MASK_HI(HCI_EV_ULP_GENERATE_DHKEY_COMPLETE)
#define HCI_EV_MASK_ULP_GENERATE_DHKEY_COMPLETE_LO             HCI_EV_MASK_LO(HCI_EV_ULP_GENERATE_DHKEY_COMPLETE)
#define HCI_EV_MASK_ULP_ENHANCED_CONNECTION_COMPLETE_HI        HCI_EV_MASK_HI(HCI_EV_ULP_ENHANCED_CONNECTION_COMPLETE)
#define HCI_EV_MASK_ULP_ENHANCED_CONNECTION_COMPLETE_LO        HCI_EV_MASK_LO(HCI_EV_ULP_ENHANCED_CONNECTION_COMPLETE)
#define HCI_EV_MASK_ULP_DIRECT_ADVERTISING_REPORT_HI           HCI_EV_MASK_HI(HCI_EV_ULP_DIRECT_ADVERTISING_REPORT)
#define HCI_EV_MASK_ULP_DIRECT_ADVERTISING_REPORT_LO           HCI_EV_MASK_LO(HCI_EV_ULP_DIRECT_ADVERTISING_REPORT)
#define HCI_EV_MASK_ULP_PHY_UPDATE_COMPLETE_HI                 HCI_EV_MASK_HI(HCI_EV_ULP_PHY_UPDATE_COMPLETE)
#define HCI_EV_MASK_ULP_PHY_UPDATE_COMPLETE_LO                 HCI_EV_MASK_LO(HCI_EV_ULP_PHY_UPDATE_COMPLETE)
#define HCI_EV_MASK_ULP_DATA_LENGTH_CHANGE_HI                  HCI_EV_MASK_HI(HCI_EV_ULP_DATA_LENGTH_CHANGE)
#define HCI_EV_MASK_ULP_DATA_LENGTH_CHANGE_LO                  HCI_EV_MASK_LO(HCI_EV_ULP_DATA_LENGTH_CHANGE)
#define HCI_EV_MASK_ULP_CHANNEL_SELECTION_ALGORITHM_HI         HCI_EV_MASK_HI(HCI_EV_ULP_CHANNEL_SELECTION_ALGORITHM)
#define HCI_EV_MASK_ULP_CHANNEL_SELECTION_ALGORITHM_LO         HCI_EV_MASK_LO(HCI_EV_ULP_CHANNEL_SELECTION_ALGORITHM)



/*-----------------------------------------------------------------------------*
 *
 *   Auto accept values
 *
 *------------------------------------------------------------------------------*/
#define HCI_AUTO_ACCEPT_OFF                 ((uint8_t)0x01)
#define HCI_AUTO_ACCEPT_ON                  ((uint8_t)0x02)
#define HCI_AUTO_ACCEPT_WITH_ROLE_SWITCH    ((uint8_t)0x03)

/*-----------------------------------------------------------------------------*
 *
 *   Inquiry Values :   HCI_INQUIRY, HCI_PERIODIC_INQUIRY_MODE,
 *                      HCWI_WRITE_INQUIRYSCAN_ACTIVITY
 *
 *------------------------------------------------------------------------------*/
#define HCI_INQUIRY_LENGTH_MIN              ((uint8_t)0x01)
#define HCI_INQUIRY_LENGTH_MAX              ((uint8_t)0x30)
#define HCI_INQUIRY_RESPONSES_MIN           ((uint8_t)0x01)
#define HCI_INQUIRY_RESPONSES_MAX           ((uint8_t)0xFF)
#define HCI_INQUIRY_MAX_PERIOD_MIN          ((uint16_t)0x0003)
#define HCI_INQUIRY_MAX_PERIOD_MAX          ((uint16_t)0xFFFF)
#define HCI_INQUIRY_MIN_PERIOD_MIN          ((uint16_t)0x0002)
#define HCI_INQUIRY_MIN_PERIOD_MAX          ((uint16_t)0xFFFE)
#define HCI_INQUIRYSCAN_INTERVAL_MIN        ((uint16_t)0x0012)
#define HCI_INQUIRYSCAN_INTERVAL_DEFAULT    ((uint16_t)0x1000)
#define HCI_INQUIRYSCAN_INTERVAL_MAX        ((uint16_t)0x1000)
#define HCI_INQUIRYSCAN_WINDOW_MIN          ((uint16_t)0x0011)
#define HCI_INQUIRYSCAN_WINDOW_DEFAULT      ((uint16_t)0x0012)
#define HCI_INQUIRYSCAN_WINDOW_MAX          ((uint16_t)0x1000)

#ifndef HCI_MAX_INQUIRY_RESULTS
#define HCI_MAX_INQUIRY_RESULTS 1  /* Max responses in inquiry result message */
#endif

/*-----------------------------------------------------------------------------*
 *
 *   Scan enable values
 *
 *------------------------------------------------------------------------------*/
#define HCI_SCAN_ENABLE_OFF             ((uint8_t)0x00) /* default */
#define HCI_SCAN_ENABLE_INQ             ((uint8_t)0x01)
#define HCI_SCAN_ENABLE_PAGE            ((uint8_t)0x02)
#define HCI_SCAN_ENABLE_INQ_AND_PAGE    ((uint8_t)0x03)

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Page and Connection Accept Timer Defaults
 *
 *----------------------------------------------------------------------------*/
#define HCI_DEFAULT_PAGE_TIMEOUT                0x2000
#define HCI_DEFAULT_CONNECTION_ACCEPT_TIMEOUT   0x1F40

/*-----------------------------------------------------------------------------*
 *
 *   HCI CREATE CONNECTION Boundary conditions
 *
 *------------------------------------------------------------------------------*/
#define HCI_DO_NOT_ALLOW_ROLE_SWITCH    ((uint8_t)0x00)
#define HCI_ALLOW_ROLE_SWITCH           ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *  HCI ACCEPT CONNECTION REQUEST
 *  Role Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_ROLE_BECOME_MASTER          ((uint8_t)0x00)
#define HCI_ROLE_STAY_SLAVE             ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   Authentication enable values
 *
 *------------------------------------------------------------------------------*/
#define HCI_AUTH_ENABLE_OFF             ((uint8_t)0x00)
#define HCI_AUTH_ENABLE_ON              ((uint8_t)0x01)    /* for all connections */

/*-----------------------------------------------------------------------------*
 *
 *   Encryption mode values
 *
 *------------------------------------------------------------------------------*/
#define HCI_ENC_OFF                     ((uint8_t)0x00)
#define HCI_ENC_ON                      ((uint8_t)0x01)
/* Secure Connections encryption_enable defines for Encryption Event message */
#define HCI_BREDR_ENC_E0_ON             HCI_ENC_ON
#define HCI_BREDR_ENC_AES_ON            ((uint8_t)0x02)
#define HCI_LE_ENC_AES_ON               HCI_ENC_ON

#define HCI_ENC_MODE_OFF                ((uint8_t)0x00)
#define HCI_ENC_MODE_PT_TO_PT           ((uint8_t)0x01)
#define HCI_ENC_MODE_PT_TO_PT_AND_BCAST ((uint8_t)0x02)

/*-----------------------------------------------------------------------------*
 *
 *   Voice setting mask values
 *
 *------------------------------------------------------------------------------*/
#define HCI_VOICE_INPUT_MASK            ((uint16_t)0x0300)
#define HCI_VOICE_INPUT_LIN             ((uint16_t)0x0000)
#define HCI_VOICE_INPUT_MU_LAW          ((uint16_t)0x0100)
#define HCI_VOICE_INPUT_A_LAW           ((uint16_t)0x0200)
#define HCI_VOICE_INPUT_RESERVED        ((uint16_t)0x0300)
#define HCI_VOICE_FORMAT_MASK           ((uint16_t)0x00C0)
#define HCI_VOICE_FORMAT_1SCOMP         ((uint16_t)0x0000)
#define HCI_VOICE_FORMAT_2SCOMP         ((uint16_t)0x0040)
#define HCI_VOICE_FORMAT_SMAG           ((uint16_t)0x0080)
#define HCI_VOICE_FORMAT_UNSIGNED       ((uint16_t)0x00C0)
#define HCI_VOICE_SAMP_SIZE_MASK        ((uint16_t)0x0020)
#define HCI_VOICE_SAMP_SIZE_8BIT        ((uint16_t)0x0000)
#define HCI_VOICE_SAMP_SIZE_16BIT       ((uint16_t)0x0020)
#define HCI_VOICE_LINEAR_PCM_MASK       ((uint16_t)0x001C)
#define HCI_VOICE_LINEAR_PCM_SHIFT      2
#define HCI_VOICE_AIR_CODING_MASK       ((uint16_t)0x0003)
#define HCI_VOICE_AIR_CODING_CVSD       ((uint16_t)0x0000)
#define HCI_VOICE_AIR_CODING_MU_LAW     ((uint16_t)0x0001)
#define HCI_VOICE_AIR_CODING_A_LAW      ((uint16_t)0x0002)
#define HCI_VOICE_TRANSPARENT_DATA      ((uint16_t)0x0003)

#define HCI_VOICE_SETTINGS_DEFAULT      ((uint16_t)0x0060)
#define HCI_VOICE_SETTING_MAX_VALUE     ((uint16_t)0x03FF) /* 10 bits meaningful */

/*-----------------------------------------------------------------------------*
 *
 *   Coding Format
 *
 *-----------------------------------------------------------------------------*/
#define HCI_CODING_FORMAT_MU_LAW        ((uint8_t)0x00)
#define HCI_CODING_FORMAT_A_LAW         ((uint8_t)0x01)
#define HCI_CODING_FORMAT_CVSD          ((uint8_t)0x02)
#define HCI_CODING_FORMAT_TRANSPARENT   ((uint8_t)0x03)
#define HCI_CODING_FORMAT_LINEAR        ((uint8_t)0x04)
#define HCI_CODING_FORMAT_MSBC          ((uint8_t)0x05)
#define HCI_CODING_FORMAT_VENDOR_SPEC   ((uint8_t)0xFF)

/*-----------------------------------------------------------------------------*
 *
 *   PCM Data Format
 *
 *-----------------------------------------------------------------------------*/
#define HCI_PCM_DATA_FORMAT_1SCOMP      ((uint8_t)0x01)
#define HCI_PCM_DATA_FORMAT_2SCOMP      ((uint8_t)0x02)
#define HCI_PCM_DATA_FORMAT_SMAG        ((uint8_t)0x03)
#define HCI_PCM_DATA_FORMAT_UNSIGNED    ((uint8_t)0x04)

/*-----------------------------------------------------------------------------*
 *
 *   Vendor specific SCO data paths
 *
 *-----------------------------------------------------------------------------*/
#define HCI_DATA_PATH_HCI               ((uint8_t)0x00)
#define HCI_DATA_PATH_PCM               ((uint8_t)0x01)
#define HCI_DATA_PATH_I2S               ((uint8_t)0x02)
#define HCI_DATA_PATH_CODEC             ((uint8_t)0x03)
#define HCI_DATA_PATH_PSKEY             ((uint8_t)0x07)
#define HCI_DATA_PATH_MASK              ((uint8_t)0x07)
#define HCI_DATA_PATH_SHIFT             ((uint8_t)0)
#define HCI_DATA_PATH_SLOT_MASK         ((uint8_t)0x18)
#define HCI_DATA_PATH_SLOT_SHIFT        ((uint8_t)3)
#define HCI_DATA_PATH_CHANNEL_MASK      ((uint8_t)0x18)
#define HCI_DATA_PATH_CHANNEL_SHIFT     ((uint8_t)3)
#define HCI_DATA_PATH_INSTANCE_MASK     ((uint8_t)0x60)
#define HCI_DATA_PATH_INSTANCE_SHIFT    ((uint8_t)5)

/*-----------------------------------------------------------------------------*
 *
 *   Transport Layer
 *
 *-----------------------------------------------------------------------------*/
#define HCI_TRANSPORT_LAYER_DISABLED    ((uint8_t)0x00)
#define HCI_TRANSPORT_LAYER_WCI1        ((uint8_t)0x01)
#define HCI_TRANSPORT_LAYER_WCI2        ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   MWS Channel Type
 *
 *-----------------------------------------------------------------------------*/
#define HCI_MWS_CHANNEL_TYPE_TDD        ((uint8_t)0x00)
#define HCI_MWS_CHANNEL_TYPE_FDD        ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   Write Automatic Flush Timeout Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_MAX_FLUSH_TIMEOUT           ((uint16_t)0x07FF)

/*-----------------------------------------------------------------------------*
 *
 *   Hold Mode Activity Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_HOLD_CURR_PWR_STATE         ((uint8_t)0x00)
#define HCI_HOLD_SUSPEND_PAGE_SCAN      ((uint8_t)0x01)
#define HCI_HOLD_SUSPEND_INQ_SCAN       ((uint8_t)0x02)
#define HCI_HOLD_SUSPEND_PER_INQ        ((uint8_t)0x04)

#define HCI_HOLD_MIN_INTERVAL_MIN       ((uint16_t)0x0002)
#define HCI_HOLD_MIN_INTERVAL_MAX       ((uint16_t)0xFF00)

/*-----------------------------------------------------------------------------*
 *
 *   Sniff Mode Activity Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_SNIFF_MAX_INTERVAL_MIN      ((uint16_t)0x0002)
#define HCI_SNIFF_MIN_INTERVAL_MIN      ((uint16_t)0x0002)
#define HCI_SNIFF_ATTEMPT_MIN           ((uint16_t)0x0001)
#define HCI_SNIFF_ATTEMPT_MAX           ((uint16_t)0x7FFF)
#define HCI_SNIFF_TIMEOUT_MIN           ((uint16_t)0x0000)
#define HCI_SNIFF_TIMEOUT_MAX           ((uint16_t)0x7FFF)
#define HCI_SNIFF_SUB_RATE_LATENCY_MIN  ((uint16_t)0x0002)
#define HCI_SNIFF_SUB_RATE_LATENCY_MAX  ((uint16_t)0xFFFE)
#define HCI_SNIFF_SUB_RATE_TIMEOUT_MAX  ((uint16_t)0xFFFE)

/*-----------------------------------------------------------------------------*
 *
 *   Host Controller Channel Classification Mode Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_CHANNEL_CLASS_MODE_DISABLE  ((uint8_t)0x00)
#define HCI_CHANNEL_CLASS_MODE_ENABLE   ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Version Information Values
 *
 *   https://www.bluetooth.com/specifications/assigned-numbers/host-controller-interface
 *
 *------------------------------------------------------------------------------*/
#define HCI_VER_1_0               ((hci_version_t)0x00)
#define HCI_VER_1_1               ((hci_version_t)0x01)
#define HCI_VER_1_2               ((hci_version_t)0x02)
#define HCI_VER_2_0               ((hci_version_t)0x03)
#define HCI_VER_2_1               ((hci_version_t)0x04)
#define HCI_VER_3_0               ((hci_version_t)0x05)
#define HCI_VER_4_0               ((hci_version_t)0x06)
#define HCI_VER_4_1               ((hci_version_t)0x07)
#define HCI_VER_4_2               ((hci_version_t)0x08)
#define HCI_VER_5_0               ((hci_version_t)0x09)
#define HCI_VER_5_1               ((hci_version_t)0x0a)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Country Code Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_CO_CODE_NA_AND_EUR     ((uint8_t)0x00)
#define HCI_CO_CODE_FRANCE         ((uint8_t)0x01)
#define HCI_CO_CODE_SPAIN          ((uint8_t)0x02)
#define HCI_CO_CODE_JAPAN          ((uint8_t)0x03)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Current device mode Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_BT_MODE_ACTIVE         ((hci_bt_mode_t)0x00)
#define HCI_BT_MODE_HOLD           ((hci_bt_mode_t)(HCI_BT_MODE_ACTIVE + 1))
#define HCI_BT_MODE_SNIFF          ((hci_bt_mode_t)(HCI_BT_MODE_HOLD + 1))
/*
 * Marks end range. Do not remove HCI_BT_MODE_MAX.
 * For new additions (if any):
 * - These defines must be relatively defined (above) & must be unique in value.
 * - Also the max define (below) must be accordingly updated.
 */
#define HCI_BT_MODE_MAX            ((hci_bt_mode_t)HCI_BT_MODE_SNIFF + 1)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Test Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_GEN_SELF_TEST          ((uint8_t)0x00)
#define HCI_LOOPBACK_OFF           ((uint8_t)0x00)
#define HCI_LOCAL_LOOPBACK         ((uint8_t)0x01)
#define HCI_REMOTE_LOOPBACK        ((uint8_t)0x02)
#define HCI_LOOPBACK_MODE_MAX      ((uint8_t)0x03)

/*----------------------------------------------------------------------------*
 *
 *   HCI Link Type Values
 *
 *---------------------------------------------------------------------------*/
#define HCI_LINK_TYPE_SCO          ((uint8_t)0x00)
#define HCI_LINK_TYPE_ACL          ((uint8_t)0x01)
#define HCI_LINK_TYPE_ESCO         ((uint8_t)0x02)
#define HCI_LINK_TYPE_DONT_CARE    ((uint8_t)0x03)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Page Scan Repetition Mode Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_PAGE_SCAN_REP_MODE_R0     ((uint8_t)0x00)
#define HCI_PAGE_SCAN_REP_MODE_R1     ((uint8_t)0x01)
#define HCI_PAGE_SCAN_REP_MODE_R2     ((uint8_t)0x02)

/*----------------------------------------------------------------------------*
 *
 *   HCI Page Scan Mode Values
 *
 *---------------------------------------------------------------------------*/
#define HCI_PAGE_SCAN_MODE_MANDATORY      ((uint8_t)0x00)
#define HCI_PAGE_SCAN_MODE_OPTIONAL_1     ((uint8_t)0x01)
#define HCI_PAGE_SCAN_MODE_OPTIONAL_2     ((uint8_t)0x02)
#define HCI_PAGE_SCAN_MODE_OPTIONAL_3     ((uint8_t)0x03)
#define HCI_PAGE_SCAN_MODE_DEFAULT        HCI_PAGE_SCAN_MODE_MANDATORY

/*----------------------------------------------------------------------------*
 *
 *   HCI Page Scan Interval : HCI_WRITE_PAGESCAN_ACTIVITY
 *
 *---------------------------------------------------------------------------*/
#define HCI_PAGESCAN_INTERVAL_MIN       ((uint16_t) 0x12)
#define HCI_PAGESCAN_INTERVAL_DEFAULT   ((uint16_t) 0x800)
#define HCI_PAGESCAN_INTERVAL_MAX       ((uint16_t) 0x1000)

/*----------------------------------------------------------------------------*
 *
 *   HCI Page Scan Window : HCI_WRITE_PAGESCAN_ACTIVITY
 *
 *---------------------------------------------------------------------------*/
#define HCI_PAGESCAN_WINDOW_MIN       ((uint16_t) 0x11)
#define HCI_PAGESCAN_WINDOW_DEFAULT   ((uint16_t) 0x12)
#define HCI_PAGESCAN_WINDOW_MAX       ((uint16_t) 0x1000)

/*----------------------------------------------------------------------------*
 *
 *   HCI Page Scan PERIOD : HCI_WRITE_PAGESCAN_PERIOD_MODE
 *
 *---------------------------------------------------------------------------*/
#define HCI_PAGESCAN_PERIOD_MODE_P0         ((uint8_t) 0x00)
#define HCI_PAGESCAN_PERIOD_MODE_P1         ((uint8_t) 0x01)
#define HCI_PAGESCAN_PERIOD_MODE_P2         ((uint8_t) 0x02)
#define HCI_PAGESCAN_PERIOD_MODE_DEFAULT    HCI_PAGESCAN_PERIOD_MODE_P0

/*----------------------------------------------------------------------------*
 *
 *   HCI Page and Inquiry Scan Type : HCI_WRITE_PAGE_SCAN_TYPE
 *                                    HCI_WRITE_INQUIRY_SCAN_TYPE
 *
 *---------------------------------------------------------------------------*/
#define HCI_SCAN_TYPE_LEGACY           ((uint8_t) 0x00)
#define HCI_SCAN_TYPE_INTERLACED       ((uint8_t) 0x01)

/*----------------------------------------------------------------------------*
 *
 *   HCI Inquiry Mode : HCI_WRITE_INQUIRY_MODE
 *
 *---------------------------------------------------------------------------*/
#define HCI_INQUIRY_MODE_STANDARD      ((uint8_t) 0x00)
#define HCI_INQUIRY_MODE_WITH_RSSI     ((uint8_t) 0x01)
#define HCI_INQUIRY_MODE_WITH_EIR      ((uint8_t) 0x02)

/*----------------------------------------------------------------------------*
 *
 *   HCI Clock offset Values
 *
 *---------------------------------------------------------------------------*/
#define HCI_CLOCK_OFFSET_MASK       ((clock_offset_t)0x7fff)
#define HCI_CLOCK_OFFSET_VALID_MASK ((clock_offset_t)0x8000)
#define HCI_CLOCK_OFFSET_INVALID    ((clock_offset_t)0x0000)
#define HCI_CLOCK_OFFSET_VALID      ((clock_offset_t)0x8000)

/*----------------------------------------------------------------------------*
 *
 *   HCI Link Policy Settings
 *
 *---------------------------------------------------------------------------*/
#define DISABLE_ALL_LM_MODES         ((link_policy_settings_t)0x0000)
#define ENABLE_MS_SWITCH             ((link_policy_settings_t)0x0001)
#define ENABLE_HOLD                  ((link_policy_settings_t)0x0002)
#define ENABLE_SNIFF                 ((link_policy_settings_t)0x0004)
#define LINK_POLICY_MASK             ((link_policy_settings_t)0x00FF)

/*----------------------------------------------------------------------------*
 *
 *   HCI Filter types
 *
 *---------------------------------------------------------------------------*/
#define CLEAR_ALL_FILTERS            ((filter_type_t)0x00)
#define INQUIRY_RESULT_FILTER        ((filter_type_t)0x01)
#define CONNECTION_FILTER            ((filter_type_t)0x02)

/*----------------------------------------------------------------------------*
 *
 *   HCI Filter condition types
 *
 *---------------------------------------------------------------------------*/
#define NEW_DEVICE_RESPONDED         ((filter_condition_type_t)0x00)
#define ALL_CONNECTION               ((filter_condition_type_t)0x00)
#define CLASS_OF_DEVICE_RESPONDED    ((filter_condition_type_t)0x01)
#define ADDRESSED_DEVICE_RESPONDED   ((filter_condition_type_t)0x02)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Pin types
 *
 *------------------------------------------------------------------------------*/
#define HCI_VARIABLE_PIN    ((pin_type_t)0x00)
#define HCI_FIXED_PIN       ((pin_type_t)0x01)


/*-----------------------------------------------------------------------------*
 *
 *   Enhanced flushable packet types
 *   NB values 0x01 - 0xFF Reserved for future use
 *------------------------------------------------------------------------------*/
#define HCI_AUTO_FLUSHABLE_ONLY ((flushable_packet_type_t)0x00)
#define FLUSH_INFINITE_TIMEOUT 0x0


/*-----------------------------------------------------------------------------*
 *
 *   HCI Pin Code Length
 *
 *------------------------------------------------------------------------------*/
#define HCI_MIN_PIN_LENGTH  ((uint8_t)0x01)
#define HCI_MAX_PIN_LENGTH  ((uint8_t)0x10)

/*-----------------------------------------------------------------------------*
 *
 *   Size of link keys
 *
 *------------------------------------------------------------------------------*/
#define SIZE_LINK_KEY   0x10

/*---------------------------------------------------------------------------*
 *
 *  Size of Out Of Band data hash and rand
 *
 * --------------------------------------------------------------------------*/

#define SIZE_OOB_DATA   0x10

/*-----------------------------------------------------------------------------*
 *
 *   HCI Read stored link key read all flag types
 *
 *------------------------------------------------------------------------------*/
#define RETURN_BDADDR   ((read_all_flag_t)0x00)
#define RETURN_ALL      ((read_all_flag_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Delete stored link key read all flag types
 *
 *------------------------------------------------------------------------------*/
#define DELETE_BDADDR   ((delete_all_flag_t)0x00)
#define DELETE_ALL      ((delete_all_flag_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   HCI IAC LAP Boundary values and Other Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_NUM_CURRENT_IAC_MIN   ((uint8_t)0x01)
#define HCI_NUM_CURRENT_IAC_MAX   ((uint8_t)0x40)
#define HCI_IAC_LAP_MIN           ((uint32_t)0x9E8B00)
#define HCI_IAC_LAP_MAX           ((uint32_t)0x9E8B3F)

#define HCI_INQ_CODE_GIAC  ((uint32_t)0x9e8b33)
/*-----------------------------------------------------------------------------*
 *
 *   HCI Connection Accept Timeout values
 *
 *------------------------------------------------------------------------------*/
#define HCI_CONNECTION_ACCEPT_TIMEOUT_MIN   ((uint16_t)0x01)
#define HCI_CONNECTION_ACCEPT_TIMEOUT_MAX   ((uint16_t)0x0B540)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Handle Range
 *
 *------------------------------------------------------------------------------*/
#define HCI_HANDLE_MAX   ((uint16_t)0x0EFF)
#define HCI_HANDLE_INVALID ((uint16_t) 0xffff)

/*----------------------------------------------------------------------------*
 *
 *   HCI Link Supervision Timeout Values
 *
 *---------------------------------------------------------------------------*/
#define HCI_LINK_SUPERVISION_INFINITY   ((uint16_t)0x0000)
#define HCI_LINK_SUPERVISION_MIN        ((uint16_t)0x0001)
#define HCI_LINK_SUPERVISION_DEFAULT    ((uint16_t)0x7D00)
#define HCI_LINK_SUPERVISION_MAX        ((uint16_t)0xFFFF)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Transmit Power Type Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_READ_CURRENT_TX_POWER       ((uint8_t)0x00)
#define HCI_READ_MAX_TX_POWER           ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   HCI SCO Flow Control Type Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_SCO_FLOW_CONTROL_DISABLED   ((uint8_t)0x00)
#define HCI_SCO_FLOW_CONTROL_ENABLED    ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *
 *   HCI HC to H Flow Control Type Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_HCITOH_FLOW_CONTROL_DISABLED            ((uint8_t)0x00)
#define HCI_HCITOH_FLOW_CONTROL_ENABLED_ACL_ONLY    ((uint8_t)0x01)
#define HCI_HCITOH_FLOW_CONTROL_ENABLED_SCO_ONLY    ((uint8_t)0x02)
#define HCI_HCITOH_FLOW_CONTROL_ENABLED_ACL_AND_SCO ((uint8_t)0x03)

/*-----------------------------------------------------------------------------*
 *
 *   HCI Write Stored Link Key, HCI Return Link Keys Event,
 *   maximum number of keys that can be present
 *   in a write stored link key. Max command parameter size= 255 bytes.
 *      Num keys parameter = 1 byte
 *      (BD_ADDR + Key) = (6+16) = X
 *      Max Keys = (255-1)/X = 10
 *
 *------------------------------------------------------------------------------*/
#define HCI_STORED_LINK_KEY_MAX             ((uint8_t)10)
#define HCI_RETURN_LINK_KEY_VAR_ARG_SIZE    22

/*-----------------------------------------------------------------------------*
 *
 *   HCI Link Key Notification Event Key Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_COMBINATION_KEY                      ((uint8_t)0)
#define HCI_LOCAL_UNIT_KEY                       ((uint8_t)1)
#define HCI_REMOTE_UNIT_KEY                      ((uint8_t)2)
#define HCI_DEBUG_COMBINATION_KEY                ((uint8_t)3)
#define HCI_UNAUTHENTICATED_COMBINATION_KEY_P192 ((uint8_t)4)
#define HCI_AUTHENTICATED_COMBINATION_KEY_P192   ((uint8_t)5)
#define HCI_CHANGED_COMBINATION_KEY              ((uint8_t)6)
#define HCI_UNAUTHENTICATED_COMBINATION_KEY_P256 ((uint8_t)7)
#define HCI_AUTHENTICATED_COMBINATION_KEY_P256   ((uint8_t)8)
#define HCI_KEY_TYPE_UNKNOWN                     ((uint8_t)0xFF)

/*---------------------------------------------------------------------------*
 *
 *  Coding of BR/EDR Features. Features is 64 bits long, it is sent across HCI
 *  as an array of 8 uint8s, hence the feature definitions are split into 8 bit
 *  quantities. The HCI_FEATURE_MASK/INDEX and HCI_FEATURE_IS_SUPPORTED
 *  macros should be used to manipulate individual bits. Note that this last
 *  macro returns non-zero (not necessarily 1) for a supported feature.
 *
 *  If a feature is present the bit is set.
 *
 *---------------------------------------------------------------------------*/
#define LMP_FEATURES_THREE_SLOT_PACKETS_BIT                0
#define LMP_FEATURES_FIVE_SLOT_PACKETS_BIT                 1
#define LMP_FEATURES_ENCRYPTION_BIT                        2
#define LMP_FEATURES_SLOT_OFFSET_BIT                       3
#define LMP_FEATURES_TIMING_ACCURACY_BIT                   4
#define LMP_FEATURES_SWITCH_BIT                            5
#define LMP_FEATURES_HOLD_MODE_BIT                         6
#define LMP_FEATURES_SNIFF_MODE_BIT                        7
/* Bit 8 was Park Mode */
#define LMP_FEATURES_RSSI_BIT                              9
#define LMP_FEATURES_CHANNEL_QUALITY_DRIVEN_DATA_RATE_BIT  10
#define LMP_FEATURES_SCO_LINK_BIT                          11
#define LMP_FEATURES_HV2_PACKETS_BIT                       12
#define LMP_FEATURES_HV3_PACKETS_BIT                       13
#define LMP_FEATURES_U_LAW_BIT                             14
#define LMP_FEATURES_A_LAW_BIT                             15
#define LMP_FEATURES_CVSD_BIT                              16
#define LMP_FEATURES_PAGING_SCHEME_BIT                     17
#define LMP_FEATURES_POWER_CONTROL_BIT                     18
#define LMP_FEATURES_TRANSPARENT_SCO_BIT                   19
#define LMP_FEATURES_FLOW_CONTROL_LAG_BIT0_BIT             20
#define LMP_FEATURES_FLOW_CONTROL_LAG_BIT1_BIT             21
#define LMP_FEATURES_FLOW_CONTROL_LAG_BIT2_BIT             22
#define LMP_FEATURES_BCAST_ENCRYPTION_BIT                  23
/* Bit 24 was Scatter Mode */
#define LMP_FEATURES_MR_2MBPS_BIT                          25
#define LMP_FEATURES_MR_3MBPS_BIT                          26
#define LMP_FEATURES_ENHANCED_INQUIRY_SCAN_BIT             27
#define LMP_FEATURES_INTERLACED_INQUIRY_SCAN_BIT           28
#define LMP_FEATURES_INTERLACED_PAGE_SCAN_BIT              29
#define LMP_FEATURES_RSSI_INQUIRY_RESULT_BIT               30
#define LMP_FEATURES_EV3_PACKETS_BIT                       31
#define LMP_FEATURES_EV4_PACKETS_BIT                       32
#define LMP_FEATURES_EV5_PACKETS_BIT                       33
#define LMP_FEATURES_ABSENCE_MASKS_BIT                     34
#define LMP_FEATURES_AFH_CAPABLE_SLAVE_BIT                 35
#define LMP_FEATURES_AFH_CLASSIFICATION_SLAVE_BIT          36
#define LMP_FEATURES_BR_EDR_NOT_SUPPORTED_BIT              37
#define LMP_FEATURES_LE_SUPPORTED_CONTROLLER_BIT           38
#define LMP_FEATURES_3SLOT_MR_BIT                          39
#define LMP_FEATURES_5SLOT_MR_BIT                          40
#define LMP_FEATURES_SNIFF_SUB_RATE_BIT                    41
#define LMP_FEATURES_ENCRYPTION_PAUSE_RESUME_BIT           42
#define LMP_FEATURES_AFH_CAPABLE_MASTER_BIT                43
#define LMP_FEATURES_AFH_CLASSIFICATION_MASTER_BIT         44
#define LMP_FEATURES_MR_ESCO_2MBPS_BIT                     45
#define LMP_FEATURES_MR_ESCO_3MBPS_BIT                     46
#define LMP_FEATURES_3SLOT_ESCO_MR_BIT                     47
#define LMP_FEATURES_EXTENDED_INQUIRY_RESPONSE_BIT         48
#define LMP_FEATURES_SIMULTANEOUS_LE_BR_EDR_CAPABLE_CONTROLLER_BIT      49
/* Bits 49-50 were Alphanumeric Pin Aware/Capable */
#define LMP_FEATURES_SIMPLE_PAIRING_BIT                    51
#define LMP_FEATURES_ENCAPSULATED_PDU_BIT                  52
/* Bit 53 Erroneous Data Reporting (not supported) */
#define LMP_FEATURES_NON_FLUSHABLE_PBF_BIT                 54
/* Bit 55 reserved */
#define LMP_FEATURES_LINK_SUPERVISION_TIMEOUT_BIT          56
#define LMP_FEATURES_INQUIRY_RESPONSE_TX_POWER_LEVEL_BIT   57
#define LMP_FEATURES_ENHANCED_POWER_CONTROL_BIT            58
/* Bit 59 was (briefly) BR/EDR Secure Connections */

/* Bits 59-62 reserved */
#define LMP_FEATURES_EXTENDED_FEATURES_BIT                 63

#define HCI_FEATURE_MASK(bit) (1U << ((bit) & 7))
#define HCI_FEATURE_INDEX(bit) ((bit) / 8)
#define HCI_FEATURE_IS_SUPPORTED(bit, array) \
        ((array)[HCI_FEATURE_INDEX(bit)] & HCI_FEATURE_MASK(bit))

/*-----------------------------------------------------------------------------*
 *
 * Extended Features Page 1 - host supported features
 * These are compatable with the HCI_FEATURE_* macros above as long as they
 * are used on the correct array.
 *
 *------------------------------------------------------------------------------*/
#define LMP_EXT_FEATURE_SSP_HOST_SUPPORT_BIT                            0
#define LMP_EXT_FEATURE_LE_SUPPORTED_HOST_BIT                           1
#define LMP_EXT_FEATURE_SIMULTANEOUS_LE_BR_EDR_CAPABLE_HOST_BIT         2
#define LMP_EXT_FEATURE_SECURE_CONNECTIONS_HOST_SUPPORT_BIT             3

/*-----------------------------------------------------------------------------*
 *
 * Extended Features Page 2 - controller supported features
 * These are compatable with the HCI_FEATURE_* macros above as long as they
 * are used on the correct array.
 *
 *------------------------------------------------------------------------------*/
#define LMP_EXT_FEATURE_CSB_MASTER_OPERATION_BIT                        0
#define LMP_EXT_FEATURE_CSB_SLAVE_OPERATION_BIT                         1
#define LMP_EXT_FEATURE_SYNCHRONIZATION_TRAIN_BIT                       2
#define LMP_EXT_FEATURE_SYNCHRONIZATION_SCAN_BIT                        3
#define LMP_EXT_FEATURE_INQUIRY_RESP_NOTIFICATION_EVENT_BIT             4
#define LMP_EXT_FEATURE_SECURE_CONNECTIONS_CONTROLLER_SUPPORT_BIT       8
#define LMP_EXT_FEATURE_PING_BIT                                        9


/*-----------------------------------------------------------------------------*
 *
 *   Supported features definitions for backwards compatibility only.
 *   Please don't add any more here and don't use in new code.
 *   Use the macros defined above.
 *
 *------------------------------------------------------------------------------*/
/* Byte 0 features */
#define HCI_FEATURE_3_SLOT_PACKETS          \
            HCI_FEATURE_MASK(LMP_FEATURES_THREE_SLOT_PACKETS)
#define HCI_FEATURE_5_SLOT_PACKETS          \
            HCI_FEATURE_MASK(LMP_FEATURES_FIVE_SLOT_PACKETS)
#define HCI_FEATURE_ENCRYPTION  HCI_FEATURE_MASK(LMP_FEATURES_ENCRYPTION_BIT)
#define HCI_FEATURE_SLOT_OFFSET HCI_FEATURE_MASK(LMP_FEATURES_SLOT_OFFSET_BIT)
#define HCI_FEATURE_TIMING_ACCURACY         \
            HCI_FEATURE_MASK(LMP_FEATURES_TIMING_ACCURACY_BIT)
#define HCI_FEATURE_SWITCH      HCI_FEATURE_MASK(LMP_FEATURES_SWITCH_BIT)
#define HCI_FEATURE_HOLD_MODE   HCI_FEATURE_MASK(LMP_FEATURES_HOLD_MODE_BIT)
#define HCI_FEATURE_SNIFF_MODE  HCI_FEATURE_MASK(LMP_FEATURES_SNIFF_MODE_BIT)
/* Byte 1 features */
#define HCI_FEATURE_RSSI        HCI_FEATURE_MASK(LMP_FEATURES_RSSI_BIT)
#define HCI_FEATURE_CQD_DATA_RATE           \
            HCI_FEATURE_MASK(LMP_FEATURES_CHANNEL_QUALITY_DRIVEN_DATA_RATE_BIT)
#define HCI_FEATURE_SCO_LINK    HCI_FEATURE_MASK(LMP_FEATURES_SCO_LINK_BIT)
#define HCI_FEATURE_HV2_PACKETS HCI_FEATURE_MASK(LMP_FEATURES_HV2_PACKETS_BIT)
#define HCI_FEATURE_HV3_PACKETS HCI_FEATURE_MASK(LMP_FEATURES_HV3_PACKETS_BIT)
#define HCI_FEATURE_U_LAW_LOG   HCI_FEATURE_MASK(LMP_FEATURES_U_LAW_BIT)
#define HCI_FEATURE_A_LAW_LOG   HCI_FEATURE_MASK(LMP_FEATURES_A_LAW_BIT)
/* Byte 2 features */
#define HCI_FEATURE_CVSD        HCI_FEATURE_MASK(LMP_FEATURES_CVSD_BIT)
#define HCI_FEATURE_PAGING_SCHEME           \
            HCI_FEATURE_MASK(LMP_FEATURES_PAGING_SCHEME_BIT)
#define HCI_FEATURE_POWER_CONTROL           \
            HCI_FEATURE_MASK(LMP_FEATURES_POWER_CONTROL_BIT)
#define HCI_FEATURE_TRANSPARENT_SCO_DATA    \
            HCI_FEATURE_MASK(LMP_FEATURES_TRANSPARENT_SCO_BIT)
#define HCI_FEATURE_FLOW_CONTROL_LAG_BIT0   \
            HCI_FEATURE_MASK(LMP_FEATURES_FLOW_CONTROL_LAG_BIT0_BIT)
#define HCI_FEATURE_FLOW_CONTROL_LAG_BIT1   \
            HCI_FEATURE_MASK(LMP_FEATURES_FLOW_CONTROL_LAG_BIT1_BIT)
#define HCI_FEATURE_FLOW_CONTROL_LAG_BIT2   \
            HCI_FEATURE_MASK(LMP_FEATURES_FLOW_CONTROL_LAG_BIT2_BIT)

/*-----------------------------------------------------------------------------*
 *   Simple Pairing
 *   HCI Simple_Pairing_Mode
 *
 *------------------------------------------------------------------------------*/
#define HCI_SIMPLE_PAIRING_MODE_NOTSET          ((uint8_t)0x00)
#define HCI_SIMPLE_PAIRING_MODE_ENABLED         ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *   Simple Pairing/Security Manager
 *   Combined Input/Output Capabilities
 *
 *   Note that HCI_IO_CAP_KEYBOARD_DISPLAY is not valid for Simple Pairing
 *
 *------------------------------------------------------------------------------*/
#define HCI_IO_CAP_MIN                          0
#define HCI_IO_CAP_DISPLAY_ONLY                 ((uint8_t)0x00)
#define HCI_IO_CAP_DISPLAY_YES_NO               ((uint8_t)0x01)
#define HCI_IO_CAP_KEYBOARD_ONLY                ((uint8_t)0x02)
#define HCI_IO_CAP_NO_INPUT_NO_OUTPUT           ((uint8_t)0x03)
#define HCI_IO_CAP_KEYBOARD_DISPLAY             ((uint8_t)0x04)
#define HCI_IO_CAP_MAX                          4

/*-----------------------------------------------------------------------------*
 *   Simple Pairing
 *   HCI OOB Data Present
 *
 *------------------------------------------------------------------------------*/
#define HCI_OOB_DATA_MIN                        0
#define HCI_OOB_DATA_NOT_PRESENT                ((uint8_t)0x00)
#define HCI_P192_OOB_DATA_PRESENT               ((uint8_t)0x01)
#define HCI_P256_OOB_DATA_PRESENT               ((uint8_t)0x02)
#define HCI_P192_P256_OOB_DATA_PRESENT          ((uint8_t)0x03)
#define HCI_OOB_DATA_MAX                        3

/*-----------------------------------------------------------------------------*
 *   Simple Pairing
 *   HCI Authentication_Requirements
 *
 *------------------------------------------------------------------------------*/
#define HCI_MITM_PROTECTION_MIN                 0
#define HCI_MITM_NOT_REQUIRED_NO_BONDING        ((uint8_t)0x00)
#define HCI_MITM_REQUIRED_NO_BONDING            ((uint8_t)0x01)
#define HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING ((uint8_t)0x02)
#define HCI_MITM_REQUIRED_DEDICATED_BONDING     ((uint8_t)0x03)
#define HCI_MITM_NOT_REQUIRED_GENERAL_BONDING   ((uint8_t)0x04)
#define HCI_MITM_REQUIRED_GENERAL_BONDING       ((uint8_t)0x05)
#define HCI_MITM_PROTECTION_MAX                 5

/*-----------------------------------------------------------------------------*
 *   Simple Pairing
 *   Simple_Pairing_Debug_Mode
 *
 *------------------------------------------------------------------------------*/
#define HCI_SIMPLE_PAIRING_DEBUG_MODE_DISABLED  ((uint8_t)0x00)
#define HCI_SIMPLE_PAIRING_DEBUG_MODE_ENABLED   ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *   Secure Connections
 *   DM1 ACL-U Mode
 *
 *------------------------------------------------------------------------------*/
#define HCI_DM1_ACL_U_MODE_DISABLED             ((uint8_t)0x00)
#define HCI_DM1_ACL_U_MODE_ENABLED              ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *   Secure Connections
 *   eSCO Loopback Mode
 *
 *------------------------------------------------------------------------------*/
#define HCI_ESCO_LOOPBACK_MODE_DISABLED         ((uint8_t)0x00)
#define HCI_ESCO_LOOPBACK_MODE_ENABLED          ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *   Simple Pairing
 *   Notification_Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_NOTIFICATION_TYPE_PASSKEY_STARTED         ((uint8_t)0x00)
#define HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ENTERED   ((uint8_t)0x01)
#define HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ERASED    ((uint8_t)0x02)
#define HCI_NOTIFICATION_TYPE_PASSKEY_CLEARED         ((uint8_t)0x03)
#define HCI_NOTIFICATION_TYPE_PASSKEY_COMPLETED       ((uint8_t)0x04)
#define HCI_NOTIFICATION_TYPE_MAX       HCI_NOTIFICATION_TYPE_PASSKEY_COMPLETED

/*-----------------------------------------------------------------------------*
 *
 *   LM SCO handle type. Currently not available through the HCI interface.
 *
 *------------------------------------------------------------------------------*/
#define HCI_LM_SCO_HANDLE_INVALID       ((uint8_t) 0x00)

/*-----------------------------------------------------------------------------*
 *   ULP Test Mode
 *   Frequency of test data
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_TEST_MAX_FREQ                      ((uint8_t)0x27)

/*-----------------------------------------------------------------------------*
 *   ULP Test Mode
 *   Payload Pattern Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_TEST_PATTERN_PRBS9                 ((uint8_t)0x00)
#define HCI_ULP_TEST_PATTERN_11110000              ((uint8_t)0x01)
#define HCI_ULP_TEST_PATTERN_10101010              ((uint8_t)0x02)
#define HCI_ULP_TEST_PATTERN_PRBS15                ((uint8_t)0x03)
#define HCI_ULP_TEST_PATTERN_ALL_1                 ((uint8_t)0x04)
#define HCI_ULP_TEST_PATTERN_ALL_0                 ((uint8_t)0x05)
#define HCI_ULP_TEST_PATTERN_00001111              ((uint8_t)0x06)
#define HCI_ULP_TEST_PATTERN_01010101              ((uint8_t)0x07)
#define HCI_ULP_TEST_PATTERN_MAX     HCI_ULP_TEST_PATTERN_01010101

/*-----------------------------------------------------------------------------*
 *   LE Support Command
 *   LE Supported Host
 *
 *------------------------------------------------------------------------------*/
#define HCI_LE_SUPPORTED_HOST_DISABLED             ((uint8_t)0x00)
#define HCI_LE_SUPPORTED_HOST_ENABLED              ((uint8_t)0x01)


/*-----------------------------------------------------------------------------*
 *   LE Support Command
 *   Simultaneous LE Host
 *
 *------------------------------------------------------------------------------*/
#define HCI_SIMULTANEOUS_LE_HOST_DISABLED          ((uint8_t)0x00)
#define HCI_SIMULTANEOUS_LE_HOST_ENABLED           ((uint8_t)0x01)


/*---------------------------------------------------------------------------*
 *
 *  Coding of ULP Features. Features is 64 bits long, it is sent across HCI as
 *  an array of 8 uint8s, hence the feature definitions are split into 8 bit
 *  quantities. The ULP_FEATURE_MASK/INDEX and ULP_FEATURE_IS_SUPPORTED
 *  macros should be used to manipulate individual bits. Note that this last
 *  macro returns non-zero (not necessarily 1) for a supported feature.
 *
 *  If a feature is present the bit is set.
 *
 *---------------------------------------------------------------------------*/
#define ULP_FEATURE_ENCRYPTION                              0
#define ULP_FEATURE_CONNECTION_PARAMETERS_REQUEST_PROCEDURE 1
#define ULP_FEATURE_EXTENDED_REJECT_INDICATION              2
#define ULP_FEATURE_SLAVE_INITIATED_FEATURES_EXCHANGE       3
#define ULP_FEATURE_LE_PING                                 4
#define ULP_FEATURE_DATA_LENGTH_EXTENSION                   5
#define ULP_FEATURE_LL_PRIVACY                              6
#define ULP_FEATURE_2LE_PHY                                 8
#define ULP_FEATURE_STABLE_MODULATION_INDEX                 9
#define ULP_FEATURE_CHANNEL_SELECTION_ALGORITHM_2           14

/* Note that this is still to be defined. i.e. FIPD has T.B.D. 
 * Use 10 for now
 */
#define ULP_FEATURE_BLR_PHY                                 10

#define ULP_FEATURE_MASK(feature) (1U << ((feature) & 15))
#define ULP_FEATURE_INDEX(feature) ((feature)/16)

/* Query macro */
#define ULP_FEATURE_IS_SUPPORTED(featureset, feature) \
    ( (featureset)[ULP_FEATURE_INDEX(feature)] \
      & ULP_FEATURE_MASK(feature))

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Advertising_Interval_[Min/Max]
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_DEFAULT_ADVERT_INTERVAL 0x0800

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Advertising_Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_ADVERT_CONNECTABLE_UNDIRECTED           ((uint8_t)0x00)
#define HCI_ULP_ADVERT_CONNECTABLE_DIRECTED_HIGH_DUTY   ((uint8_t)0x01)
#define HCI_ULP_ADVERT_DISCOVERABLE                     ((uint8_t)0x02)
#define HCI_ULP_ADVERT_NON_CONNECTABLE                  ((uint8_t)0x03)
#define HCI_ULP_ADVERT_CONNECTABLE_DIRECTED_LOW_DUTY    ((uint8_t)0x04)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  X_Address_Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_ADDRESS_PUBLIC                     ((uint8_t)0x00)
#define HCI_ULP_ADDRESS_RANDOM                     ((uint8_t)0x01)
/* Controller generates RPA based on local IRK
 * from RL else fallback to Public Address(FBP)
 */
#define HCI_ULP_ADDRESS_GENERATE_RPA_FBP           ((uint8_t)0x02)
/* Controller generates RPA based on local IRK
 * from RL else fallback to Random Address(FBR) from
 * HCI_ULP_SET_RANDOM_ADDRESS
 */
#define HCI_ULP_ADDRESS_GENERATE_RPA_FBR           ((uint8_t)0x03)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Peer_Address_Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_PEER_ADDRESS_PUBLIC                ((uint8_t)0x00)
#define HCI_ULP_PEER_ADDRESS_RANDOM                ((uint8_t)0x01)
#define HCI_ULP_PEER_ADDRESS_RPA_PUBLIC_IA         ((uint8_t)0x02)
#define HCI_ULP_PEER_ADDRESS_RPA_RANDOM_IA         ((uint8_t)0x03)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Advertising_Channel_Map
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_ADVERT_CHANNEL_37           ((uint8_t)0x01)
#define HCI_ULP_ADVERT_CHANNEL_38           ((uint8_t)0x02)
#define HCI_ULP_ADVERT_CHANNEL_39           ((uint8_t)0x04)
#define HCI_ULP_ADVERT_CHANNEL_ALL          ((uint8_t)(HCI_ULP_ADVERT_CHANNEL_37 | \
                                                       HCI_ULP_ADVERT_CHANNEL_38 | \
                                                       HCI_ULP_ADVERT_CHANNEL_39))
#define HCI_ULP_ADVERT_CHANNEL_DEFAULT      HCI_ULP_ADVERT_CHANNEL_ALL
#define HCI_ULP_ADVERT_CHANNEL_MIN_VALUE    ((uint8_t)0x01)
#define HCI_ULP_ADVERT_CHANNEL_MAX_VALUE    ((uint8_t)0x07) /* 3 bits meaningful */

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Advertising_Filter_Policy
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_ADV_FP_ALLOW_ANY            ((uint8_t)0x00)
#define HCI_ULP_ADV_FP_ALLOW_CONNECTIONS    ((uint8_t)0x01)
#define HCI_ULP_ADV_FP_ALLOW_SCANNING       ((uint8_t)0x02)
#define HCI_ULP_ADV_FP_ALLOW_WHITELIST      ((uint8_t)0x03)
#define HCI_ULP_ADV_FP_ALLOW_MAX            HCI_ULP_ADV_FP_ALLOW_WHITELIST

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Advertising_Enable
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_ADVERTISING_DISABLED        ((uint8_t)0x00)
#define HCI_ULP_ADVERTISING_ENABLED         ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  LE_Scan_Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_PASSIVE_SCANNING            ((uint8_t)0x00)
#define HCI_ULP_ACTIVE_SCANNING             ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  LE_Scan_Interval
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_SCAN_INTERVAL_MIN           ((uint16_t)0x0004)
#define HCI_ULP_SCAN_INTERVAL_MAX           ((uint16_t)0x4000)
#define HCI_ULP_SCAN_INTERVAL_DEFAULT       ((uint16_t)0x0010)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  LE_Scan_Window
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_SCAN_WINDOW_MIN             ((uint16_t)0x0004)
#define HCI_ULP_SCAN_WINDOW_MAX             ((uint16_t)0x4000)
#define HCI_ULP_SCAN_WINDOW_DEFAULT         ((uint16_t)0x0010)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Scanning_Filter_Policy
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_SCAN_FP_ALLOW_ALL               ((uint8_t)0x00)
#define HCI_ULP_SCAN_FP_ALLOW_WHITELIST         ((uint8_t)0x01)
#define HCI_ULP_SCAN_FP_ALLOW_DIRECT_ALL        ((uint8_t)0x02)
#define HCI_ULP_SCAN_FP_ALLOW_DIRECT_WHITELIST  ((uint8_t)0x03)
#define HCI_ULP_SCAN_FP_ALLOW_MAX               HCI_ULP_SCAN_FP_ALLOW_DIRECT_WHITELIST

/*-----------------------------------------------------------------------------*
 *  ULP
 *  LE_Scan_Enable
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_SCAN_DISABLED               ((uint8_t)0x00)
#define HCI_ULP_SCAN_ENABLED                ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Filter_Duplicates
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_FILTER_DUPLICATES_DISABLED  ((uint8_t)0x00)
#define HCI_ULP_FILTER_DUPLICATES_ENABLED   ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  x_Conn_Interval
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_CONN_INTERVAL_MIN           ((uint16_t)0x0006)
#define HCI_ULP_CONN_INTERVAL_MAX           ((uint16_t)0x0C80)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Conn_Latency
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_CONN_LATENCY_MAX            ((uint16_t)0x01F3)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Supervision_Timeout
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_SUPERVISION_TIMEOUT_MIN     ((uint16_t)0x000A)
#define HCI_ULP_SUPERVISION_TIMEOUT_MAX     ((uint16_t)0x0C80)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  x_CE_Length
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_CE_LENGTH_MIN               ((uint16_t)0x0000)
#define HCI_ULP_CE_LENGTH_MAX               ((uint16_t)0xFFFF)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Initiator_Filter_Policy
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_INITIATOR_FP_DONT_USE_WHITELIST   ((uint8_t)0x00)
#define HCI_ULP_INITIATOR_FP_USE_WHITELIST        ((uint8_t)0x01)
#define HCI_ULP_INITIATOR_FP_MAX                  HCI_ULP_INITIATOR_FP_USE_WHITELIST

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Connection Complete Role
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_EV_ROLE_MASTER                    ((uint8_t)0x00)
#define HCI_ULP_EV_ROLE_SLAVE                     ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Master Clock Accuracy
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_EV_CLOCK_ACCURACY_500PPM          ((uint8_t)0x00)
#define HCI_ULP_EV_CLOCK_ACCURACY_250PPM          ((uint8_t)0x01)
#define HCI_ULP_EV_CLOCK_ACCURACY_150PPM          ((uint8_t)0x02)
#define HCI_ULP_EV_CLOCK_ACCURACY_100PPM          ((uint8_t)0x03)
#define HCI_ULP_EV_CLOCK_ACCURACY_75PPM           ((uint8_t)0x04)
#define HCI_ULP_EV_CLOCK_ACCURACY_50PPM           ((uint8_t)0x05)
#define HCI_ULP_EV_CLOCK_ACCURACY_30PPM           ((uint8_t)0x06)
#define HCI_ULP_EV_CLOCK_ACCURACY_20PPM           ((uint8_t)0x07)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Advertising Report Event_Type
 *
 *------------------------------------------------------------------------------*/
#define HCI_ULP_EV_ADVERT_CONNECTABLE_UNDIRECTED  ((uint8_t)0x00)
#define HCI_ULP_EV_ADVERT_CONNECTABLE_DIRECTED    ((uint8_t)0x01)
#define HCI_ULP_EV_ADVERT_DISCOVERABLE            ((uint8_t)0x02)
#define HCI_ULP_EV_ADVERT_NON_CONNECTABLE         ((uint8_t)0x03)
#define HCI_ULP_EV_ADVERT_SCAN_RESPONSE           ((uint8_t)0x04)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  2LE PHY
 *  LERP, LEERT and LE_PHY_Update_Complete_Event use an enumeration for the PHY 
 *  values. All other commands use bit-fields(thus different values for LE_CODED.)

 *------------------------------------------------------------------------------*/
#define HCI_ULP_2LE_PHY_INVALID                   ((uint8_t)0x00)
#define HCI_ULP_2LE_PHY_1MBPS                     ((uint8_t)0x01)
#define HCI_ULP_2LE_PHY_2MBPS                     ((uint8_t)0x02)
#define HCI_ULP_PHY_LE_CODED_ENUM                 ((uint8_t)0x03)
#define HCI_ULP_LR_PHY_125KBPS                    ((uint8_t)0x03)
#define HCI_ULP_LR_PHY_500KBPS                    ((uint8_t)0x04)
#define HCI_ULP_PHY_LE_CODED                      ((uint8_t)0x04)
#define HCI_ULP_PHY_OPTIONS_NO_PREFERENCE         ((uint16_t)0x00)
#define HCI_ULP_PHY_OPTIONS_500KBPS               ((uint16_t)0x01)
#define HCI_ULP_PHY_OPTIONS_125KBPS               ((uint16_t)0x02)
#define HCI_ULP_2LE_MOD_INDEX_STANDARD            ((uint8_t)0x00)
#define HCI_ULP_2LE_MOD_INDEX_STABLE              ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*
 *  ULP
 *  Channel Selection Algorithm
 *------------------------------------------------------------------------------*/
#define HCI_ULP_CHANNEL_SELECTION_ALGORITHM_1     ((uint16_t)0x00)
#define HCI_ULP_CHANNEL_SELECTION_ALGORITHM_2     ((uint16_t)0x01)

/*-----------------------------------------------------------------------------*
 *  Secure Connections
 *  Secure Connections Host Support
 *
 *------------------------------------------------------------------------------*/
#define HCI_SECURE_CONNECTIONS_HOST_SUPPORT_DISABLED             ((uint8_t)0x00)
#define HCI_SECURE_CONNECTIONS_HOST_SUPPORT_ENABLED              ((uint8_t)0x01)

/*-----------------------------------------------------------------------------*


 *  Connectionless Slave Broadcast
 *  Sync Train Default Values
 *
 *------------------------------------------------------------------------------*/
#define HCI_SYNC_TRAIN_INTERVAL_MIN               ((uint8_t)0x20)
#define HCI_SYNC_TRAIN_INTERVAL_MAX               ((uint16_t)0xFFFE)
#define HCI_SYNC_TRAIN_TIMEOUT_MIN                ((uint8_t)0x2)
#define HCI_SYNC_TRAIN_TIMEOUT_MAX                ((uint32_t)0x7FFFFFEUL)
#define HCI_SYNC_TRAIN_TIMEOUT_DEFAULT            ((uint32_t)0x2EE00UL)

#define HCI_SYNC_TRAIN_SCAN_WINDOW_MIN            ((uint8_t)0x22)
#define HCI_SYNC_TRAIN_SCAN_WINDOW_MAX            ((uint16_t)0xFFFE)
#define HCI_SYNC_TRAIN_SCAN_INTERVAL_MIN          (HCI_SYNC_TRAIN_SCAN_WINDOW_MIN + (uint8_t)0x02)
#define HCI_SYNC_TRAIN_SCAN_INTERVAL_MAX          HCI_SYNC_TRAIN_SCAN_WINDOW_MAX
#define HCI_SYNC_TRAIN_SCAN_TIMEOUT_MIN           HCI_SYNC_TRAIN_SCAN_WINDOW_MIN
#define HCI_SYNC_TRAIN_SCAN_TIMEOUT_MAX           HCI_SYNC_TRAIN_SCAN_WINDOW_MAX

/*-----------------------------------------------------------------------------*
 *  Connectionless Slave Broadcast
 *  Range of broadcast parameters
 *
 *------------------------------------------------------------------------------*/
#define HCI_CSB_BROADCAST_INTERVAL_MIN            ((uint16_t)0x0002)
#define HCI_CSB_BROADCAST_INTERVAL_MAX            ((uint16_t)0xFFFE)
#define HCI_CSB_BROADCAST_TIMEOUT_MIN             HCI_CSB_BROADCAST_INTERVAL_MIN
#define HCI_CSB_BROADCAST_TIMEOUT_MAX             HCI_CSB_BROADCAST_INTERVAL_MAX

#define HCI_CSB_FRAGMENT_TYPE_CONTINUATION        ((uint8_t)0)
#define HCI_CSB_FRAGMENT_TYPE_START               ((uint8_t)1)
#define HCI_CSB_FRAGMENT_TYPE_END                 ((uint8_t)2)
#define HCI_CSB_FRAGMENT_TYPE_SINGLE              ((uint8_t)3)
#define HCI_CSB_FRAGMENT_TYPE_INVALID             ((uint8_t)4)

#define HCI_CSB_LPO_MODE_MAX                      ((uint8_t)1)

/*-----------------------------------------------------------------------------*
 *  CSB Enable/Disable
 *------------------------------------------------------------------------------*/
#define HCI_CSB_BROADCAST_DISABLED                ((uint8_t)0x00)
#define HCI_CSB_BROADCAST_ENABLED                 ((uint8_t)0x01)


/*-----------------------------------------------------------------------------*
 *  LT ADDR
 *
 *------------------------------------------------------------------------------*/
/* Max. number of active slaves in a piconet. */
#define ACTIVE_SLAVE_LT_ADDR_MAX                  (7)

/* LT addr used for Active Slave Broadcast */
#define ACTIVE_SLAVE_BC_LT_ADDR                    (0)

/*-----------------------------------------------------------------------------*
 *
 *  Variable Argument HCI definitions, used to auto-size the HCI
 *
 *  To restrict the maximum pool size required the variable length HCI commands
 *  (Payload of up to 255 bytes (Not 256)), the HCI payload is split into several
 *  chunks of size HCI_VAR_ARG_POOL_SIZE. The HCI command structure, points to
 *  a number of these blocks. Each of these blocks are pmalloc'd on demand and
 *  the number allocated depends on the number of arguments required at that
 *  point in time. Each of these blocks holds a number of packets per ptr
 *  depending on the packet size.
 *
 *  The calculations below, auto-size the number of pmalloc'd pointer blocks
 *  required and the number of packets that each of the blocks can hold.
 *
 *  HCI_ARC_CONVERT(input_struct) - This makes the size of the structure the
 *                                  same number on windows and XAP. Hence
 *                                  allowing xap friendly code.
 *
 *  HCI_PMALLOC_ARC_CONVERT       - This takes into account that
 *                                  HCI_VAR_ARG_POOL_SIZE is in XAP words.
 *
 *  So for example, the handle complete calculations come out as follows:
 *
 *      254 = Max Payload size - HCI_byte_sizeof(num_handles)
 *      HCI_VAR_ARG_POOL_SIZE=32   :Fixed size of POOL.
 *      HCI_BYTE_SIZE_OF_HANDLE_COMPLETE_T=4 :Number of bytes sent across the
 *          HCI interface for each handle_complete.
 *      HCI_ARC_CONVERT(HANDLE_COMPLETE_T)=2
 *
 *      HCI_EV_HANDLE_COMPLETES_PER_PTR         =(32/2)=16
 *      HCI_EV_NUM_HANDLE_COMPLETE_PACKET_PTRS  =((254/4)+16-1) / 16 = 4
 *
 *  The (16-1) which os seen in HCI_EV_NUM_HANDLE_COMPLETE_PACKET_PTRS, is
 *  used to accomodate the rounding down error, caused by integer division.
 *  If an extra block is required, this calculation will ensure it is
 *  allocated.
 *
 *------------------------------------------------------------------------------*/
#define HCI_ARC_CONVERT(input_struct)           (sizeof(input_struct)*sizeof(uint8_t)/sizeof(uint16_t))
#define HCI_PMALLOC_ARC_CONVERT                 zpmalloc(HCI_VAR_ARG_POOL_SIZE * sizeof(uint16_t))

#define HCI_NAME_LENGTH                         248
#define HCI_LOCAL_NAME_BYTES_PER_PTR            HCI_VAR_ARG_POOL_SIZE
#define HCI_LOCAL_NAME_BYTE_PACKET_PTRS         ((HCI_NAME_LENGTH + HCI_LOCAL_NAME_BYTES_PER_PTR -1) / HCI_LOCAL_NAME_BYTES_PER_PTR)

#define HCI_BYTE_SIZE_OF_HANDLE_COMPLETE_T      4
#define HCI_EV_HANDLE_COMPLETES_PER_PTR         (HCI_VAR_ARG_POOL_SIZE/HCI_ARC_CONVERT(HANDLE_COMPLETE_T))
#define HCI_EV_NUM_HANDLE_COMPLETE_PACKET_PTRS  ( ((254 / HCI_BYTE_SIZE_OF_HANDLE_COMPLETE_T) + HCI_EV_HANDLE_COMPLETES_PER_PTR -1) / HCI_EV_HANDLE_COMPLETES_PER_PTR)

#define HCI_HOST_NUM_COMPLETED_PACKETS_PER_PTR  HCI_EV_HANDLE_COMPLETES_PER_PTR
#define HCI_HOST_NUM_COMPLETED_PACKET_PTRS      HCI_EV_NUM_HANDLE_COMPLETE_PACKET_PTRS

/* HCI_ARC_CONVERT(HCI_INQ_RESULT_T) is not used for the following definition
   because it is only guaranteed to be correct on the XAP and the definition
   here is also used by BCHS on various other architectures. Crucially, BCHS
   needs to be able to get the correct value of HCI_MAX_INQ_RESULT_PTRS as
   otherwise they have no way of determining the size of various DM event
   primitives that return inquiry results. Unlike HCI, Bluestack uses the
   Bluecore Friendly Format for physical interfaces and this specifies that
   pointers are passed as 16 bit zeros, the data (to which these pointed)
   follows at the end of the primitive. Therefore BCHS needs to know how
   many of these pointers have been sent. So we define a static value here
   and then use a compile time assert in nonhci_convert.c to ensure that
   this equals HCI_ARC_CONVERT(HCI_INQ_RESULT_T) when compiling on the xap.
   See B-30015. */
#ifdef __KALIMBA32__
#define KAL32_SIZE_OF_HCI_INQ_RESULT_T          10
#define HCI_MAX_INQ_RESULT_PER_PTR              (HCI_VAR_ARG_POOL_SIZE/KAL32_SIZE_OF_HCI_INQ_RESULT_T)
#else
#define XAP_SIZE_OF_HCI_INQ_RESULT_T            10
#define HCI_MAX_INQ_RESULT_PER_PTR              (HCI_VAR_ARG_POOL_SIZE/XAP_SIZE_OF_HCI_INQ_RESULT_T)
#endif
#define HCI_BYTE_SIZE_OF_HCI_INQ_RESULT_T       14
#define HCI_MAX_INQ_RESULT_PTRS                 ( ((254 / HCI_BYTE_SIZE_OF_HCI_INQ_RESULT_T) + HCI_MAX_INQ_RESULT_PER_PTR -1) / HCI_MAX_INQ_RESULT_PER_PTR)

#define HCI_BYTE_SIZE_OF_HCI_IAC_LAP            3
#define HCI_IAC_LAP_PER_PTR                     (HCI_VAR_ARG_POOL_SIZE/HCI_ARC_CONVERT(uint24_t))
#define HCI_IAC_LAP_PTRS                        ( ((254 / HCI_BYTE_SIZE_OF_HCI_IAC_LAP) + HCI_IAC_LAP_PER_PTR -1) / HCI_IAC_LAP_PER_PTR)

#define HCI_EVENT_PKT_LENGTH                    255
#define HCI_LOOPBACK_BYTES_PER_PTR              HCI_VAR_ARG_POOL_SIZE
#define HCI_LOOPBACK_BYTE_PACKET_PTRS           ((HCI_EVENT_PKT_LENGTH + HCI_LOOPBACK_BYTES_PER_PTR -1) / HCI_LOOPBACK_BYTES_PER_PTR)

#define HCI_READ_SUPP_COMMANDS_EVENT_LENGTH     64
#define HCI_READ_SUPP_COMMANDS_PER_PTR          HCI_VAR_ARG_POOL_SIZE
#define HCI_READ_SUPP_COMMANDS_PACKET_PTRS      ((HCI_READ_SUPP_COMMANDS_EVENT_LENGTH + HCI_READ_SUPP_COMMANDS_PER_PTR - 1) / HCI_READ_SUPP_COMMANDS_PER_PTR)

#define HCI_EIR_DATA_LENGTH                     240
#define HCI_EIR_DATA_BYTES_PER_PTR              HCI_VAR_ARG_POOL_SIZE
#define HCI_EIR_DATA_PACKET_PTRS                ((HCI_EIR_DATA_LENGTH + HCI_EIR_DATA_BYTES_PER_PTR -1) / HCI_EIR_DATA_BYTES_PER_PTR)
#define HCI_EIR_DATA_LENGTH_REMAINDER           (HCI_EIR_DATA_LENGTH - (HCI_EIR_DATA_PACKET_PTRS-1) * HCI_EIR_DATA_BYTES_PER_PTR)

#define HCI_SET_CSB_DATA_LENGTH                 252
#define HCI_SET_CSB_DATA_BYTES_PER_PTR          HCI_VAR_ARG_POOL_SIZE
#define HCI_SET_CSB_DATA_PACKET_PTRS            ((HCI_SET_CSB_DATA_LENGTH + HCI_SET_CSB_DATA_BYTES_PER_PTR - 1) / HCI_SET_CSB_DATA_BYTES_PER_PTR)

#define HCI_CSB_RECV_DATA_LENGTH                237
#define HCI_CSB_RECV_BYTES_PER_PTR              HCI_VAR_ARG_POOL_SIZE
#define HCI_CSB_RECV_PACKET_PTRS                ((HCI_CSB_RECV_DATA_LENGTH + HCI_CSB_RECV_BYTES_PER_PTR - 1) / HCI_CSB_RECV_BYTES_PER_PTR)

/* Supported codecs are 1 octet on-the-wire format */
#define HCI_SUPPORTED_CODEC_OCTETS_SIZE         1u
#define HCI_NUM_SUPPORTED_CODECS_OCTETS_SIZE    1u
#define HCI_SUPPORTED_CODECS_DATA_LENGTH        128
/* The firmware puts only one uint8_t in each XAP word*/
#define HCI_SUPPORTED_CODECS_PER_PTR            (HCI_VAR_ARG_POOL_SIZE / HCI_SUPPORTED_CODEC_OCTETS_SIZE)
#define HCI_SUPPORTED_CODECS_PTRS               ((HCI_SUPPORTED_CODECS_DATA_LENGTH + (HCI_SUPPORTED_CODECS_PER_PTR * HCI_SUPPORTED_CODEC_OCTETS_SIZE) - 1)  \
                                                /  (HCI_SUPPORTED_CODECS_PER_PTR * HCI_SUPPORTED_CODEC_OCTETS_SIZE))

/* Vendor specific codecs are 4 octets on-the-wire format */
#define HCI_VENDOR_CODEC_OCTETS_SIZE                4u
#define HCI_NUM_VENDOR_SPECIFIC_CODECS_OCTETS_SIZE  1u
#define HCI_VENDOR_SPECIFIC_CODECS_DATA_LENGTH  64
#define HCI_VENDOR_SPECIFIC_CODECS_PER_PTR      (HCI_VAR_ARG_POOL_SIZE / HCI_VENDOR_CODEC_OCTETS_SIZE)
#define HCI_VENDOR_SPECIFIC_CODECS_PTRS         ((HCI_VENDOR_SPECIFIC_CODECS_DATA_LENGTH + (HCI_VENDOR_SPECIFIC_CODECS_PER_PTR * HCI_VENDOR_CODEC_OCTETS_SIZE) - 1) \
                                                / (HCI_VENDOR_SPECIFIC_CODECS_PER_PTR  * HCI_VENDOR_CODEC_OCTETS_SIZE))


#define HCI_ULP_ADVERTISING_DATA_LENGTH         31
#define HCI_ULP_ADVERTISING_DATA_BYTES_PER_PTR  HCI_VAR_ARG_POOL_SIZE
#define HCI_ULP_ADVERTISING_DATA_PTRS           ((HCI_ULP_ADVERTISING_DATA_LENGTH + HCI_ULP_ADVERTISING_DATA_BYTES_PER_PTR - 1) / HCI_ULP_ADVERTISING_DATA_BYTES_PER_PTR)

/* Vendor specific command and event */
#define HCI_VS_DATA_LENGTH                          255
#define HCI_VS_DATA_BYTES_PER_PTR                   HCI_VAR_ARG_POOL_SIZE
#define HCI_VS_DATA_BYTE_PACKET_PTRS                ((HCI_VS_DATA_LENGTH + HCI_VS_DATA_BYTES_PER_PTR - 1) / HCI_VS_DATA_BYTES_PER_PTR)

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      If the DM is on-chip we combine the DM and HCI structures to save code
 *      space at the sacrifice of a few words of pmalloc space.
 *
 * CAUTION
 *      Whenever we add a new structure, we need to moidfy
 *      hci_convert/layout.pl so that the modified primitives can be
 *      serialized appropriately.
 *---------------------------------------------------------------------------*/
#ifdef DONT_BREAK_HCI_H
#define ADDED_BD_ADDRESS
#define ADDED_TYPED_BD_ADDRESS
#define ADDED_BD_ADDR_TRANSPORT
#else
#define ADDED_BD_ADDRESS        BD_ADDR_T               bd_addr;
#define ADDED_TYPED_BD_ADDRESS  TYPED_BD_ADDR_T         addrt;
#define ADDED_BD_ADDR_TRANSPORT TP_BD_ADDR_T            tp_addrt;
#endif

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      HCI_READ_CLOCK definitions
 *---------------------------------------------------------------------------*/
#define HCI_LOCAL_CLOCK      0
#define HCI_PICONET_CLOCK    1

/*============================================================================*
                           Public Data Types
 *===========================================================================*/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Command Packet Common Fields and Return packet
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    hci_op_code_t    op_code;             /* op code of command */
    uint8_t          length;              /* parameter total length */
} HCI_COMMAND_COMMON_T;

typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    BD_ADDR_T               bd_addr;
} HCI_WITH_CH_COMMON_T;

typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    TYPED_BD_ADDR_T         addrt;
} HCI_ULP_WITH_CH_COMMON_T;

typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    TP_BD_ADDR_T            tp_addrt;
} HCI_WITH_CH_PT_COMMON_T;


typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    BD_ADDR_T               bd_addr;
} HCI_WITH_BD_COMMON_T;

typedef struct
{
    hci_connection_handle_t handle;
    BD_ADDR_T              bd_addr;
} HCI_BD_ADDR_GENERIC_RET_T;

typedef struct
{
    BD_ADDR_T              bd_addr;
}  HCI_WITH_BD_ADDR_RET_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_WITH_HANDLE_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Generic command packet
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    hci_op_code_t    op_code;             /* op code of command */
    uint8_t          length;              /* parameter total length */
    uint8_t          data ;               /* Dummy field used to determine
                                             address of parameter data in
                                             an arbitrary HCI command */
} HCI_COMMAND_GENERIC_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Event Packet Common Fields
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    hci_event_code_t event_code;          /* event code */
    uint8_t          length;              /* parameter total length */
} HCI_EVENT_COMMON_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Data Packet Common Fields
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint16_t         handle_plus_flags;   /* handle is 12 lsb, flags 4 msb */
    uint16_t         length;              /* data total length */
} HCI_ACL_DATA_COMMON_T;

typedef struct
{
    uint16_t         handle;              /* handle is 12 lsb */
    uint8_t          length;              /* data total length */
} HCI_SCO_DATA_COMMON_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Variable argument fields
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    hci_connection_handle_t  handle;
    uint16_t                 num_completed;

} HANDLE_COMPLETE_T;

typedef struct
{
    BD_ADDR_T           bd_addr;                /* Bluetooth device address */
    uint8_t             link_key[SIZE_LINK_KEY];/* Link key */

} LINK_KEY_BD_ADDR_T;


/******************************************************************************
 *
 *      Link Control Command Structures
 *
 *****************************************************************************/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Inquiry command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    uint24_t               lap;             /* Lower 3 Bytes used only */
    uint8_t                inquiry_length;
    uint8_t                num_responses;
} HCI_INQUIRY_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Inquiry cancel command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
} HCI_INQUIRY_CANCEL_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Periodic inquiry mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    uint16_t               max_period_length;  /* non zero, max_period_length > min_period_length */
    uint16_t               min_period_length;  /* non zero, min_period_length > inquiry_length */
    uint24_t               lap;                /* Lower 3 bytes used only */
    uint8_t                inquiry_length;
    uint8_t                num_responses;
} HCI_PERIODIC_INQUIRY_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Exit periodic inquiry mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
} HCI_EXIT_PERIODIC_INQUIRY_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Create connection command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    hci_pkt_type_t         pkt_type;

    /* These fields are filled in by the Device Manager */
    page_scan_rep_mode_t   page_scan_rep_mode;
    page_scan_mode_t       page_scan_mode;
    clock_offset_t         clock_offset;
    uint8_t                allow_role_switch;
} HCI_CREATE_CONNECTION_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Disconnect command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    hci_error_t             reason;
} HCI_DISCONNECT_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Add SCO connection command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    hci_pkt_type_t          pkt_type;
} HCI_ADD_SCO_CONNECTION_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Create connection cancel command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_CREATE_CONNECTION_CANCEL_T;

typedef struct
{
    BD_ADDR_T              bd_addr;
} HCI_CREATE_CONNECTION_CANCEL_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Accept connection request command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    hci_role_t             role;
} HCI_ACCEPT_CONNECTION_REQ_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Reject connection request command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    hci_error_t            reason;
} HCI_REJECT_CONNECTION_REQ_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Link key request reply command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    uint8_t                key_val[SIZE_LINK_KEY];
} HCI_LINK_KEY_REQ_REPLY_T;

typedef struct
{
    BD_ADDR_T              bd_addr;
} HCI_LINK_KEY_REQ_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Link key request negative reply command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_LINK_KEY_REQ_NEG_REPLY_T;

typedef HCI_LINK_KEY_REQ_REPLY_RET_T HCI_LINK_KEY_REQ_NEG_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      PIN code request reply command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    uint8_t                pin_code_length;    /* in bytes, 1 to 16 decimal */
    uint16_t               pin[8];             /* 8 16bit words. */
}  HCI_PIN_CODE_REQ_REPLY_T;

typedef HCI_LINK_KEY_REQ_REPLY_RET_T HCI_PIN_CODE_REQ_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      PIN code request negative reply command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
}   HCI_PIN_CODE_REQ_NEG_REPLY_T;

typedef HCI_LINK_KEY_REQ_REPLY_RET_T HCI_PIN_CODE_REQ_NEG_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Change connection packet type command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    hci_pkt_type_t          pkt_type;
} HCI_CHANGE_CONN_PKT_TYPE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Authentication requested command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
} HCI_AUTH_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set connection encryption command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    hci_link_enc_t          enc_enable;         /* 0 = off, 1 = on */
} HCI_SET_CONN_ENCRYPTION_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Change connection link key command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_CHANGE_CONN_LINK_KEY_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Master link key command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_key_flag_t      link_key_type; /* 0 = regular link key, 1 = temp link key */
}  HCI_MASTER_LINK_KEY_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Remote name request command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;

    /* These fields are filled in by the Device Manager */
    page_scan_rep_mode_t   page_scan_rep_mode;
    page_scan_mode_t       page_scan_mode;
    clock_offset_t         clock_offset;
} HCI_REMOTE_NAME_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Remote name request cancel command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_REMOTE_NAME_REQ_CANCEL_T;

typedef struct
{
    BD_ADDR_T              bd_addr;
} HCI_REMOTE_NAME_REQ_CANCEL_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read remote supported features command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_READ_REMOTE_SUPP_FEATURES_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read remote extended features command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint8_t                 page_num;
} HCI_READ_REMOTE_EXT_FEATURES_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read remote version information command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDR_TRANSPORT
}  HCI_READ_REMOTE_VER_INFO_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Clock Offset command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_READ_CLOCK_OFFSET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read SCO LMP Handle command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
     /*
    Note - ther is _NO_  ADDED_BD_ADDRESS for this
    primitive because the handle passed in is actually a SCO
    handle which an application will obtain by monitoring
    SCO indications.
     */
} HCI_READ_LMP_HANDLE_T;

/* Command Complete Args */
typedef struct
{
    hci_connection_handle_t handle;
    uint8_t                 lmp_handle;
    uint32_t                reserved;
} HCI_READ_LMP_HANDLE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Anon mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
} HCI_EXCHANGE_FIXED_INFO_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Anon mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    BD_ADDR_T               local_alias_addr;
} HCI_EXCHANGE_ALIAS_INFO_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Anon mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
} HCI_PRIVATE_PAIRING_REQ_REPLY_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_PRIVATE_PAIRING_REQ_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Anon mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
} HCI_PRIVATE_PAIRING_REQ_NEG_REPLY_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_PRIVATE_PAIRING_REQ_NEG_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Anon mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    BD_ADDR_T               bd_addr_alias;
} HCI_GENERATED_ALIAS_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_GENERATED_ALIAS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Anon mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    BD_ADDR_T               bd_addr;
    BD_ADDR_T               bd_addr_alias;
} HCI_ALIAS_ADDRESS_REQ_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_ALIAS_ADDRESS_REQ_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Anon mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    BD_ADDR_T               bd_addr;
} HCI_ALIAS_ADDRESS_REQ_NEG_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_ALIAS_ADDRESS_REQ_NEG_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Enhanced Setup Synchronous connection command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    uint32_t                tx_bdw;
    uint32_t                rx_bdw;
    uint8_t                 tx_coding_format[5];
    uint8_t                 rx_coding_format[5];
    uint16_t                tx_codec_frame_size;
    uint16_t                rx_codec_frame_size;
    uint32_t                input_bdw;
    uint32_t                output_bdw;
    uint8_t                 input_coding_format[5];
    uint8_t                 output_coding_format[5];
    uint16_t                input_coded_data_size;
    uint16_t                output_coded_data_size;
    uint8_t                 input_pcm_data_format;
    uint8_t                 output_pcm_data_format;
    uint8_t                 input_pcm_sample_payload_msb_position;
    uint8_t                 output_pcm_sample_payload_msb_position;
    uint8_t                 input_data_path;
    uint8_t                 output_data_path;
    uint8_t                 input_transport_unit_size;
    uint8_t                 output_transport_unit_size;
    uint16_t                max_latency;
    hci_pkt_type_t          pkt_type;
    uint8_t                 retx_effort;
} HCI_ENHANCED_SETUP_SYNC_CONN_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Accept Enhanced Synchronous connection command
 *
 *---------------------------------------------------------------------------*/

typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    BD_ADDR_T               bd_addr;
    uint32_t                tx_bdw;
    uint32_t                rx_bdw;
    uint8_t                 tx_coding_format[5];
    uint8_t                 rx_coding_format[5];
    uint16_t                tx_codec_frame_size;
    uint16_t                rx_codec_frame_size;
    uint32_t                input_bdw;
    uint32_t                output_bdw;
    uint8_t                 input_coding_format[5];
    uint8_t                 output_coding_format[5];
    uint16_t                input_coded_data_size;
    uint16_t                output_coded_data_size;
    uint8_t                 input_pcm_data_format;
    uint8_t                 output_pcm_data_format;
    uint8_t                 input_pcm_sample_payload_msb_position;
    uint8_t                 output_pcm_sample_payload_msb_position;
    uint8_t                 input_data_path;
    uint8_t                 output_data_path;
    uint8_t                 input_transport_unit_size;
    uint8_t                 output_transport_unit_size;
    uint16_t                max_latency;
    hci_pkt_type_t          pkt_type;
    uint8_t                 retx_effort;
} HCI_ENHANCED_ACCEPT_SYNC_CONN_REQ_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Setup Synchronous connection command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    uint32_t                tx_bdw;
    uint32_t                rx_bdw;
    uint16_t                max_latency;
    uint16_t                voice_settings;
    uint8_t                 retx_effort;
    hci_pkt_type_t          pkt_type;
} HCI_SETUP_SYNCHRONOUS_CONN_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Accept Synchronous connection command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    BD_ADDR_T               bd_addr;
    uint32_t                tx_bdw;
    uint32_t                rx_bdw;
    uint16_t                max_latency;
    uint16_t                voice_settings;
    uint8_t                 retx_effort;
    hci_pkt_type_t          pkt_type;
} HCI_ACCEPT_SYNCHRONOUS_CONN_REQ_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Reject Synchronous connection command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    hci_error_t            reason;
} HCI_REJECT_SYNCHRONOUS_CONN_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      IO Capability Request Reply Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    uint8_t                io_capability;
    uint8_t                oob_data_present;
    uint8_t                authentication_requirements;
} HCI_IO_CAPABILITY_REQUEST_REPLY_T;

typedef struct
{
    BD_ADDR_T              bd_addr;
} HCI_IO_CAPABILITY_REQUEST_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Confirmation Request Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_USER_CONFIRMATION_REQUEST_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_USER_CONFIRMATION_REQUEST_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Confirmation Request Negative Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_USER_CONFIRMATION_REQUEST_NEG_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_USER_CONFIRMATION_REQUEST_NEG_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Passkey Request Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    uint32_t               numeric_value;
} HCI_USER_PASSKEY_REQUEST_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_USER_PASSKEY_REQUEST_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Passkey Request Negative Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_USER_PASSKEY_REQUEST_NEG_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_USER_PASSKEY_REQUEST_NEG_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Remote OOB Data Request Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    uint8_t                c[SIZE_OOB_DATA];
    uint8_t                r[SIZE_OOB_DATA];
} HCI_REMOTE_OOB_DATA_REQUEST_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_REMOTE_OOB_DATA_REQUEST_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Remote OOB Data Request Negative Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_REMOTE_OOB_DATA_REQUEST_NEG_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_REMOTE_OOB_DATA_REQUEST_NEG_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      IO Capability Request Negative Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    hci_error_t            reason;
} HCI_IO_CAPABILITY_REQUEST_NEG_REPLY_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_IO_CAPABILITY_REQUEST_NEG_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Secure Connections Simple Pairing
 *      Remote OOB Extended Data Request Reply
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    uint8_t                c_192[SIZE_OOB_DATA];
    uint8_t                r_192[SIZE_OOB_DATA];
    uint8_t                c_256[SIZE_OOB_DATA];
    uint8_t                r_256[SIZE_OOB_DATA];
} HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY_T;

typedef struct
{
    BD_ADDR_T              bd_addr;
} HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Truncated page
 *      Reply to Truncated page
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    BD_ADDR_T            bd_addr;
    page_scan_rep_mode_t page_scan_rep_mode;
    clock_offset_t       clock_offset;
} HCI_TRUNCATED_PAGE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Truncated page cancel
 *      Reply to Truncated page cancel
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
} HCI_TRUNCATED_PAGE_CANCEL_T;


typedef struct
{
    BD_ADDR_T bd_addr;
} HCI_TRUNCATED_PAGE_CANCEL_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Start the Synchronization train
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
} HCI_START_SYNCHRONIZATION_TRAIN_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Start/Stop Connectionless Slave Broadcast Tx
 *      Reply to Start Connectionless Slave Broadcast Tx
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    uint8_t              enable;
    uint8_t              lt_addr;
    uint8_t              lpo_allowed;
    hci_pkt_type_t       packet_type;
    uint16_t             interval_min;
    uint16_t             interval_max;
    uint16_t             supervision_timeout;
} HCI_SET_CSB_T;

typedef struct
{
    uint8_t  lt_addr;
    uint16_t interval;
} HCI_SET_CSB_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Receive Synchronization Train
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    BD_ADDR_T            bd_addr;
    uint16_t             sync_scan_timeout;
    uint16_t             sync_scan_window;
    uint16_t             sync_scan_interval;
} HCI_RECEIVE_SYNCHRONIZATION_TRAIN_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set Connectionless Slave Broadcast Receive
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    uint8_t              enable;
    BD_ADDR_T            bd_addr;
    uint8_t              lt_addr;
    uint16_t             interval;
    uint32_t             clock_offset;
    uint32_t             next_csb_clock;
    uint16_t             supervision_timeout;
    uint8_t              remote_timing_accuracy;
    uint8_t              skip;
    hci_pkt_type_t       packet_type;
    uint8_t              afh_channel_map[10];
} HCI_SET_CSB_RECEIVE_T;

typedef struct
{
    BD_ADDR_T bd_addr;
    uint8_t   lt_addr;
} HCI_SET_CSB_RECEIVE_RET_T;

/******************************************************************************
 *
 *      Link Policy Command Structures
 *
 *****************************************************************************/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Hold mode command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint16_t                max_interval;
    uint16_t                min_interval;
} HCI_HOLD_MODE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Sniff mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint16_t                max_interval;
    uint16_t                min_interval;
    uint16_t                attempt;
    uint16_t                timeout;
} HCI_SNIFF_MODE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Exit Sniff mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_EXIT_SNIFF_MODE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      QoS setup command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint8_t                 flags;              /* Reserved */
    hci_qos_type_t          service_type;
    uint32_t                token_rate;         /* in bytes per second */
    uint32_t                peak_bandwidth;     /* peak bandwidth in bytes per sec */
    uint32_t                latency;            /* in microseconds */
    uint32_t                delay_variation;    /* in microseconds */
} HCI_QOS_SETUP_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Role discovery command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_ROLE_DISCOVERY_T;

typedef struct
{
    hci_connection_handle_t handle;
    hci_role_t              current_role;
} HCI_ROLE_DISCOVERY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Switch role command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    hci_role_t             role;
} HCI_SWITCH_ROLE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Link Policy Settings command/event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_READ_LINK_POLICY_SETTINGS_T;

typedef struct
{
    hci_connection_handle_t handle;
    link_policy_settings_t  link_policy_settings;
} HCI_READ_LINK_POLICY_SETTINGS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Link Policy Settings command/event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    link_policy_settings_t  link_policy_settings;
} HCI_WRITE_LINK_POLICY_SETTINGS_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_WRITE_LINK_POLICY_SETTINGS_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Default Link Policy Settings command/event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
} HCI_READ_DEFAULT_LINK_POLICY_SETTINGS_T;

typedef struct
{
    link_policy_settings_t  default_lps;
} HCI_READ_DEFAULT_LINK_POLICY_SETTINGS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Default Link Policy Settings command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    link_policy_settings_t  default_lps;
} HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Flow Specification
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint8_t                 flags;
    uint8_t                 flow_direction; /* 0=out (to air), 1=in (from air) */
    uint8_t                 service_type;
    uint32_t                token_rate;
    uint32_t                token_bucket_size;
    uint32_t                peak_bandwidth;
    uint32_t                access_latency;
} HCI_FLOW_SPEC_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Sniff Sub Rate
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint16_t                max_remote_latency;
    uint16_t                min_remote_timeout;
    uint16_t                min_local_timeout;
} HCI_SNIFF_SUB_RATE_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_SNIFF_SUB_RATE_RET_T;


/******************************************************************************
 *
 *      Host Controller and Baseband Command Structures
 *
 *****************************************************************************/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set Event Mask command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_event_mask_t         event_mask[2];      /* ouch, 8 bytes */
} HCI_SET_EVENT_MASK_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Reset command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_RESET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set Event Filter command
 *
 *      NOTE: Variable length, spec states that unused fields shouldn't be
 *            present, this defn doesn't comply!
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
     uint24_t  class_of_device;
     uint24_t  mask;
} CLASS_MASK_T;

typedef struct
{
    CLASS_MASK_T    class_mask;
    auto_accept_t   auto_accept;
} CLASS_MASK_AUTO_T;

typedef struct
{
    BD_ADDR_T           bd_addr;
    auto_accept_t       auto_accept;
} ADDR_AUTO_T;

typedef union
{
    CLASS_MASK_T        class_mask;     /* type 1 condtype 1 */
    BD_ADDR_T           bd_addr;        /* type 1 condtype 2 */
    auto_accept_t       auto_accept;    /* type 2 condtype 0 */
    CLASS_MASK_AUTO_T   cma;            /* type 2 condtype 1 */
    ADDR_AUTO_T         addr_auto;      /* type 2 condtype 2 */
} CONDITION_T;

typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    filter_type_t           filter_type;
    filter_condition_type_t filter_condition_type;
    CONDITION_T             condition;
} HCI_SET_EVENT_FILTER_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Flush command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_FLUSH_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_FLUSH_RET_T;



/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Enhanced flush command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    flushable_packet_type_t pkt_type;
} HCI_ENHANCED_FLUSH_T;



/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Pin Type command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
} HCI_READ_PIN_TYPE_T;

typedef struct
{
    pin_type_t              pin_type;
} HCI_READ_PIN_TYPE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Pin Type command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    pin_type_t              pin_type;
} HCI_WRITE_PIN_TYPE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Create New Unit Key command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_CREATE_NEW_UNIT_KEY_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Stored Link Key command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    read_all_flag_t        read_all;
} HCI_READ_STORED_LINK_KEY_T;

typedef struct
{
    uint16_t                max_num_keys;
    uint16_t                num_keys_read;
} HCI_READ_STORED_LINK_KEY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Stored Link Key command/event
 *
 *      NOTE: Each pointed at block contain a single LINK_KEY_BDADDR_T
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    uint8_t                number_keys;        /* number of link keys to write */

    /*
     * Store single link_key_bd_addr per ptr.
     */
    LINK_KEY_BD_ADDR_T      *link_key_bd_addr[HCI_STORED_LINK_KEY_MAX];

} HCI_WRITE_STORED_LINK_KEY_T;

typedef struct
{
    uint8_t                 num_keys_written;
} HCI_WRITE_STORED_LINK_KEY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Delete Stored Link Key command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    BD_ADDR_T              bd_addr;
    delete_all_flag_t      flag;
} HCI_DELETE_STORED_LINK_KEY_T;

typedef struct
{
    uint16_t                 num_keys_deleted;
} HCI_DELETE_STORED_LINK_KEY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Change Local Name command
 *
 *      NOTE: The name is null terminated (unless it is exactly 248 bytes!)
 *            and if less than 247 bytes command is smaller than this struct,
 *            the size of the struct is therefore variable.
 *
 *      HCI_LOCAL_NAME_BYTES_PER_PTR    - bytes in a pointed at block.
 *      HCI_LOCAL_NAME_BYTE_PACKET_PTRS - Number of pointer blocks.
  *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
    /*
     * The pointers below, point to a byte array of size HCI_LOCAL_NAME_BYTES_PER_PTR.
     */
    uint8_t                *name_part[HCI_LOCAL_NAME_BYTE_PACKET_PTRS];
} HCI_CHANGE_LOCAL_NAME_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Local Name command
 *
 *      NOTE: The name is null terminated (unless it is exactly 248 bytes!)
 *            and if less than 247 bytes command is smaller than this struct,
 *            the size of the struct is therefore variable.
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T   common;
} HCI_READ_LOCAL_NAME_T ;

typedef struct
{
    /*
     * The pointers below, point to a byte array of size HCI_LOCAL_NAME_BYTES_PER_PTR.
     */
    uint8_t                *name_part[HCI_LOCAL_NAME_BYTE_PACKET_PTRS];
} HCI_READ_LOCAL_NAME_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Connection Accept Timeout command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_CONN_ACCEPT_TIMEOUT_T;

typedef struct
{
    uint16_t                 conn_accept_timeout;
} HCI_READ_CONN_ACCEPT_TIMEOUT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Connection Accept Timeout command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 conn_accept_timeout;
} HCI_WRITE_CONN_ACCEPT_TIMEOUT_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Page Timeout command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_PAGE_TIMEOUT_T;

typedef struct
{
    uint16_t                 page_timeout;
} HCI_READ_PAGE_TIMEOUT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Page Timeout command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 page_timeout;
} HCI_WRITE_PAGE_TIMEOUT_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Scan Enable command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_SCAN_ENABLE_T;

typedef struct
{
    uint8_t                  scan_enable;
} HCI_READ_SCAN_ENABLE_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Scan Enable command
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  scan_enable; /* 1-Inq_Scan, 2-Page_scan, 3-both */
} HCI_WRITE_SCAN_ENABLE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read PageScan Activity command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_PAGESCAN_ACTIVITY_T;

typedef struct
{
    uint16_t                 pagescan_interval;
    uint16_t                 pagescan_window;
} HCI_READ_PAGESCAN_ACTIVITY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write PageScan Activity command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 pagescan_interval;
    uint16_t                 pagescan_window;
} HCI_WRITE_PAGESCAN_ACTIVITY_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read InquiryScan Activity command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_INQUIRYSCAN_ACTIVITY_T;

typedef struct
{
    uint16_t                 inqscan_interval;
    uint16_t                 inqscan_window;
} HCI_READ_INQUIRYSCAN_ACTIVITY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write InquiryScan Activity command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 inqscan_interval;
    uint16_t                 inqscan_window;
} HCI_WRITE_INQUIRYSCAN_ACTIVITY_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Authentication Enable command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_AUTH_ENABLE_T;

typedef struct
{
    uint8_t                  auth_enable;
} HCI_READ_AUTH_ENABLE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Authentication Enable command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  auth_enable;
} HCI_WRITE_AUTH_ENABLE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Encryption Mode command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_ENC_MODE_T;

typedef struct
{
    uint8_t                  mode;
} HCI_READ_ENC_MODE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Encryption Mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  mode;
} HCI_WRITE_ENC_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Class of Device command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_CLASS_OF_DEVICE_T;

typedef struct
{
    uint24_t                 dev_class;       /* Lower 3 bytes only used */
} HCI_READ_CLASS_OF_DEVICE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Class of Device command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint24_t                  dev_class; /* Lower 3 bytes only used */
} HCI_WRITE_CLASS_OF_DEVICE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Voice Setting command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_VOICE_SETTING_T;

typedef struct
{
    uint16_t                 voice_setting;
} HCI_READ_VOICE_SETTING_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Voice Setting command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 voice_setting;
} HCI_WRITE_VOICE_SETTING_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Automatic Flush Timeout command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_READ_AUTO_FLUSH_TIMEOUT_T;

typedef struct
{
    hci_connection_handle_t handle;
    uint16_t                timeout;           /* N x 0.625msec */
} HCI_READ_AUTO_FLUSH_TIMEOUT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Automatic Flush Timeout command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint16_t                timeout;           /* N x 0.625msec */
} HCI_WRITE_AUTO_FLUSH_TIMEOUT_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_WRITE_AUTO_FLUSH_TIMEOUT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Num Broadcast Retransmissions command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_NUM_BCAST_RETXS_T;

typedef struct
{
    uint8_t                  num;
} HCI_READ_NUM_BCAST_RETXS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Num Broadcast Retransmissions command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  num;
} HCI_WRITE_NUM_BCAST_RETXS_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Hold Mode Activity command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_HOLD_MODE_ACTIVITY_T;

typedef struct
{
    uint8_t                  activity;
} HCI_READ_HOLD_MODE_ACTIVITY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Hold Mode Activity command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    /* 1 - suspend Page Scan */
    /* 2 - suspend Inquiry Scan */
    /* 4 - suspend Periodic Inquiries */
    uint8_t                  activity; 
} HCI_WRITE_HOLD_MODE_ACTIVITY_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Transmit Power Level command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDR_TRANSPORT
    uint8_t                 type; /* 0=current 1=Max */
} HCI_READ_TX_POWER_LEVEL_T;

typedef struct
{
    hci_connection_handle_t handle;
    int8_t                  pwr_level;
} HCI_READ_TX_POWER_LEVEL_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Enhanced Transmit Power Level command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint8_t                 type; /* 0=current 1=Max */
} HCI_READ_ENH_TX_POWER_LEVEL_T;

typedef struct
{
    hci_connection_handle_t handle;
    int8_t                  pwr_level_GFSK;     /* Basic rate   */
    int8_t                  pwr_level_DQPSK;    /* EDR 2M       */
    int8_t                  pwr_level_8DPSK;    /* EDR 3M       */
} HCI_READ_ENH_TX_POWER_LEVEL_RET_T;



/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read SCO Flow Control Enable command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_SCO_FLOW_CON_ENABLE_T;

typedef struct
{
    uint8_t     sco_flow_control_enable;
} HCI_READ_SCO_FLOW_CON_ENABLE_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write SCO Flow Control Enable command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    uint8_t                 sco_flow_control_enable;
} HCI_WRITE_SCO_FLOW_CON_ENABLE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set Host Contoller to Host Flow Control command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  enable;            /* 0=off, 1=on */
} HCI_SET_HCTOHOST_FLOW_CONTROL_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Host Buffer Size command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 acl_packet_len;
    uint8_t                  sco_packet_len;
    uint16_t                 acl_total_packets;
    uint16_t                 sco_total_packets;
} HCI_HOST_BUFFER_SIZE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Host Number of Completed Packets command
 *
 *      NOTE: Variable struct. num_handles denotes how many connection
 *            handles follow and then the same number of counts.
 *
 *      Max Size of HCI payload = 255, Variable Size = 255-(uint8_t) = 254
 *
 *      HCI_BYTE_SIZE_OF_HANDLE_COMPLETE_T  - Byte size of HANDLE_COMPLETE_T
 *        when transmitted across the HCI interface.
 *      HCI_HOST_NUM_COMPLETED_PACKETS_PER_PTR - Number of HANDLE_COMPLETE_T's
 *        in a pointed at block.
 *      HCI_HOST_NUM_COMPLETED_PACKET_PTRS - Number of pointer blocks.
 *----------------------------------------------------------------------------*/

typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  num_handles;

    /*
     * HANDLE_COMPLETE_T (*num_completed_pkts_ptr[4])[16]
     *
     * The pointers below, point to
     * HANDLE_COMPLETE_T[HCI_HOST_NUM_COMPLETED_PACKETS_PER_PTR],
     * ie a pointer to the beginning of 16 HANDLE_COMPLETE_T.
     */
    HANDLE_COMPLETE_T *num_completed_pkts_ptr[HCI_HOST_NUM_COMPLETED_PACKET_PTRS];

} HCI_HOST_NUM_COMPLETED_PACKETS_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Link Supervision Timeout command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_READ_LINK_SUPERV_TIMEOUT_T;

typedef struct
{
    hci_connection_handle_t handle;
    uint16_t                timeout;
} HCI_READ_LINK_SUPERV_TIMEOUT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Link Supervision Timeout command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
    uint16_t                timeout;
} HCI_WRITE_LINK_SUPERV_TIMEOUT_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_WRITE_LINK_SUPERV_TIMEOUT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Number Of Supported IAC command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_NUM_SUPPORTED_IAC_T;

typedef struct
{
    uint8_t                  num;
} HCI_READ_NUM_SUPPORTED_IAC_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Current IAC LAP command/event
 *
 *      NOTE: Variable struct.
 *            We have read num_current_iac.
 *
 *      Max Size of HCI payload = 255, Variable Size = 255-(uint8_t) = 254
 *
 *      HCI_IAC_LAP_PER_PTR - Number of uint24_t's
 *        in a pointed at block.
 *      HCI_IAC_LAP_PTRS - Number of pointer blocks.
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_CURRENT_IAC_LAP_T;

typedef struct
{
    uint8_t              num_current_iac;
    /*
     * Array of Pointers to an array of uint24_t
     */
    uint24_t *iac_lap[HCI_IAC_LAP_PTRS];

} HCI_READ_CURRENT_IAC_LAP_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Current IAC LAP command
 *
 *      NOTE: Variable struct.
 *            We have written num_current_iac.
 *
 *      Max Size of HCI payload = 255, Variable Size = 255-(uint8_t) = 254
 *
 *      HCI_IAC_LAP_PER_PTR - Number of uint24_t's
 *        in a pointed at block.
 *      HCI_IAC_LAP_PTRS - Number of pointer blocks.
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    uint8_t              num_current_iac;
    /*
     * Array of Pointers to an array of uint24_t
     */
    uint24_t *iac_lap[HCI_IAC_LAP_PTRS];

} HCI_WRITE_CURRENT_IAC_LAP_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Page Scan Period Mode command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_PAGESCAN_PERIOD_MODE_T;

typedef struct
{
    uint8_t                  mode;
} HCI_READ_PAGESCAN_PERIOD_MODE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Page Scan Period Mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  mode; /* 0=P0 1=P1 2=P2 */
} HCI_WRITE_PAGESCAN_PERIOD_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Page Scan Mode command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_PAGESCAN_MODE_T;

typedef struct
{
    uint8_t                  mode;
} HCI_READ_PAGESCAN_MODE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Page Scan Mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  mode; /* 0=Mandatory 1=optional1 2=opt2 3=opt3 */
} HCI_WRITE_PAGESCAN_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set AFH Channel Classification command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  map[10];
} HCI_SET_AFH_CHANNEL_CLASS_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Inquiry Scan Type command/event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_INQUIRY_SCAN_TYPE_T;

typedef struct
{
    uint8_t                  mode;
} HCI_READ_INQUIRY_SCAN_TYPE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Inquiry Scan Type command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  mode;  /* 0=legacy 1=interlaced */
} HCI_WRITE_INQUIRY_SCAN_TYPE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Inquiry Mode command/event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_INQUIRY_MODE_T;

typedef struct
{
    uint8_t                  mode;
} HCI_READ_INQUIRY_MODE_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Inquiry Mode command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  mode;  /* 0=standard 1=with rssi */
} HCI_WRITE_INQUIRY_MODE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Page Scan Type command/event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_PAGE_SCAN_TYPE_T;

typedef struct
{
    uint8_t                  mode;
} HCI_READ_PAGE_SCAN_TYPE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Page Scan Type command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  mode;  /* 0=legacy 1=interlaced */
} HCI_WRITE_PAGE_SCAN_TYPE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read AFH Channel Classification Mode command/event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_AFH_CHANNEL_CLASS_M_T;

typedef struct
{
    uint8_t                  class_mode;
} HCI_READ_AFH_CHANNEL_CLASS_M_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write AFH Channel Classification Mode command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  class_mode;
} HCI_WRITE_AFH_CHANNEL_CLASS_M_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Anon Mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_ANON_MODE_T;

typedef struct
{
    uint8_t                  mode;
} HCI_READ_ANON_MODE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Anon Mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  mode;
} HCI_WRITE_ANON_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Anon Mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_ALIAS_AUTH_ENABLE_T;

typedef struct
{
    uint8_t                  enabled;
} HCI_READ_ALIAS_AUTH_ENABLE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Anon Mode
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  enable;
} HCI_WRITE_ALIAS_AUTH_ENABLE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Anon Address Change Parameters
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_ANON_ADDR_CHANGE_PARAMS_T;

typedef struct
{
    uint32_t                 addr_update_time;
    uint16_t                 addr_inquiry_period;
} HCI_READ_ANON_ADDR_CHANGE_PARAMS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Anon Address Change parameters
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint32_t                 addr_update_time;
    uint16_t                 addr_inquiry_period;
} HCI_WRITE_ANON_ADDR_CHANGE_PARAMS_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Reset fixed address attempts counter
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  max_failures;
} HCI_RESET_FIXED_ADDRESS_ATTEMPTS_COUNTER_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Extended Inquiry Response Data Parameters
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_T;

typedef struct
{
    uint8_t                  fec_required;
    uint8_t                 *eir_data_part[HCI_EIR_DATA_PACKET_PTRS];
} HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Extended Inquiry Response Data Parameters
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  fec_required;
    uint8_t                 *eir_data_part[HCI_EIR_DATA_PACKET_PTRS];
} HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Refresh Encryption Key
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  handle;
    ADDED_TYPED_BD_ADDRESS

} HCI_REFRESH_ENCRYPTION_KEY_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Read Simple Pairing Mode Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_SIMPLE_PAIRING_MODE_T;

typedef struct
{
    uint8_t                  simple_pairing_mode;
} HCI_READ_SIMPLE_PAIRING_MODE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Write Simple Pairing Mode Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  simple_pairing_mode;
} HCI_WRITE_SIMPLE_PAIRING_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Read Local OOB Data Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_LOCAL_OOB_DATA_T;

typedef struct
{
    uint8_t                  c[SIZE_OOB_DATA];
    uint8_t                  r[SIZE_OOB_DATA];
} HCI_READ_LOCAL_OOB_DATA_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Read Inquiry Transmit Power Level Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_T;

typedef struct
{
    int8_t                  tx_power;
} HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Write Inquiry Transmit Power Level Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    int8_t                   tx_power;
} HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Send Keypresss Notification Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    BD_ADDR_T                bd_addr;
    uint8_t                  notification_type;
} HCI_SEND_KEYPRESS_NOTIFICATION_T;

typedef struct
{
    BD_ADDR_T               bd_addr;
} HCI_SEND_KEYPRESS_NOTIFICATION_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set Event Mask Page 2 command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    hci_event_mask_t     event_mask[2]; /* ouch, 8 bytes */
} HCI_SET_EVENT_MASK_PAGE_2_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      LE
 *      Read LE Host Support Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_LE_HOST_SUPPORT_T;

typedef struct
{
    uint8_t                  le_supported_host;
    uint8_t                  simultaneous_le_host;
} HCI_READ_LE_HOST_SUPPORT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      LE
 *      Write LE Host Support Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  le_supported_host;
    uint8_t                  simultaneous_le_host;
} HCI_WRITE_LE_HOST_SUPPORT_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Secure Connections
 *      Read Secure Connections Host Support Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT_T;

typedef struct
{
    uint8_t                  secure_connections_host_support;
} HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Secure Connections
 *      Write Secure Connections Host Support Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  secure_connections_host_support;
} HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set Reserved LT ADDR
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    uint8_t              lt_addr;
} HCI_SET_RESERVED_LT_ADDR_T;

typedef struct
{
    uint8_t lt_addr;
} HCI_SET_RESERVED_LT_ADDR_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Delete Reserved LT ADDR
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    uint8_t              lt_addr;
} HCI_DELETE_RESERVED_LT_ADDR_T;

typedef struct
{
    uint8_t lt_addr;
} HCI_DELETE_RESERVED_LT_ADDR_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Connectionless Slave Broadcast Data
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    uint8_t              lt_addr;
    uint8_t              fragment;
    uint8_t              data_length;
    /*
     * The pointers below, point to a byte array of size HCI_WRITE_CLB_BYTES_PER_PTR.
     */
    uint8_t              *data_part[HCI_SET_CSB_DATA_PACKET_PTRS];
} HCI_SET_CSB_DATA_T;

typedef struct
{
    uint8_t lt_addr;
} HCI_SET_CSB_DATA_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Synchronization Train Parameters
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
} HCI_READ_SYNCHRONIZATION_TRAIN_PARAMS_T;

typedef struct
{
    uint16_t sync_train_ref_interval;
    uint32_t sync_train_timeout;
    uint8_t  service_data;
} HCI_READ_SYNCHRONIZATION_TRAIN_PARAMS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Write Synchronization Train Parameters
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T common;
    uint16_t             interval_min;
    uint16_t             interval_max;
    uint32_t             sync_train_timeout;
    uint8_t              service_data;
} HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_T;

typedef struct
{
    uint16_t sync_train_ref_interval;
} HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Secure Connections
 *      Read Authenticated Payload Timeout Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  handle;
    ADDED_BD_ADDR_TRANSPORT
} HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_T;

typedef struct
{
    hci_connection_handle_t  handle;
    uint16_t                 authenticated_payload_timeout;
} HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Secure Connections
 *      Write Authenticated Payload Timeout Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  handle;
    ADDED_BD_ADDR_TRANSPORT
    uint16_t                 authenticated_payload_timeout;
} HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_T;

typedef struct
{
    hci_connection_handle_t  handle;
} HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Read Local OOB Extended Data Command
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_LOCAL_OOB_EXTENDED_DATA_T;

typedef struct
{
    uint8_t value[SIZE_OOB_DATA];
} HCI_OOB_R;

typedef struct
{
    uint8_t value[SIZE_OOB_DATA];
} HCI_OOB_C;

typedef struct
{
    HCI_OOB_C *c_192;
    HCI_OOB_R *r_192;
    HCI_OOB_C *c_256;
    HCI_OOB_R *r_256;
} HCI_READ_LOCAL_OOB_EXTENDED_DATA_RET_T;


/******************************************************************************
 *
 *      Informational Parameter Structures
 *
 *****************************************************************************/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Local Version Information command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_LOCAL_VER_INFO_T;

typedef struct
{
    hci_version_t            hci_version;
    uint16_t                 hci_revision;
    uint8_t                  lmp_version;        /* defined in LMP */
    uint16_t                 manuf_name;         /* defined in LMP */
    uint16_t                 lmp_subversion;
} HCI_READ_LOCAL_VER_INFO_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Local Supported Commnads command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_LOCAL_SUPP_COMMANDS_T;

typedef struct
{
    uint8_t *supp_commands[HCI_READ_SUPP_COMMANDS_PACKET_PTRS];
} HCI_READ_LOCAL_SUPP_COMMANDS_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Local Supported Features command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_LOCAL_SUPP_FEATURES_T;

typedef struct
{
    uint8_t                  lmp_supp_features[8]; /* As per the specification */
} HCI_READ_LOCAL_SUPP_FEATURES_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Local Extended Features command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  page_num;
} HCI_READ_LOCAL_EXT_FEATURES_T;

typedef struct
{
    uint8_t                  page_num;
    uint8_t                  max_page_num;
    uint8_t                  lmp_ext_features[8];
} HCI_READ_LOCAL_EXT_FEATURES_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Buffer Size
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_BUFFER_SIZE_T;

typedef struct
{
    uint16_t                 acl_data_pkt_length;
    uint8_t                  sco_data_pkt_length;
    uint16_t                 total_acl_data_pkts;
    uint16_t                 total_sco_data_pkts;
} HCI_READ_BUFFER_SIZE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Country Code
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_COUNTRY_CODE_T;

typedef struct
{
    uint8_t                  country_code;
} HCI_READ_COUNTRY_CODE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read BD_ADDR
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_BD_ADDR_T;

typedef struct
{
    BD_ADDR_T                bd_addr;
} HCI_READ_BD_ADDR_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Local Supported Codecs command
 *
 *      NOTE: Variable struct.
 *            We have read num_of_supported_codecs and
 *                     num_of_supported_vendor_specific_codecs.
 *
 *      MAX Size of HCI payload = 255, Variable Size = 255-2*(uint8_t) = 253
 *      which need to be splitted in half. We decided to use 128 octets( = 64
 *      XAP words = 128 codecs of 1 octet each) for supported_codecs and 125
 *      for vendor_specific_codecs. However, for the moment we will only use
 *      64 octets ( = 32 XAP words = 16 codecs of 4 octets each) for the
 *      vendor_specific_codecs.
 *
 *      HCI_SUPPORTED_CODECS_PER_PTR - Number of XAP uint8_t`s in a block.
 *      On a XAP sizeof(uint8_t) = sizeof(uint16_t) = 1 (XAP word i.e. 2 octets).
 *      The firmware will put a uint8_t in each XAP uint8_t; therefore the upper
 *      bits of a XAP uint_8 will be empty.
 *      HCI_SUPPORTED_CODECS_PTRS - Number of pointers to blocks.
 *
 *      HCI_VENDOR_SPECIFIC_CODECS_PER_PTR - Number of uint32_t's in a block
 *      HCI_VENDOR_SPECIFIC_CODECS_PTRS - Number of pointers to blocks.
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
} HCI_READ_LOCAL_SUPP_CODECS_T;

typedef struct
{
    uint8_t                 num_of_supported_codecs;
    uint8_t                 *supported_codecs[HCI_SUPPORTED_CODECS_PTRS];

    uint8_t                 num_of_supported_vendor_specific_codecs;
    uint32_t                *vendor_specific_codecs[HCI_VENDOR_SPECIFIC_CODECS_PTRS];

} HCI_READ_LOCAL_SUPP_CODECS_RET_T;


/******************************************************************************
 *
 *      Status Parameter Structures
 *
 *****************************************************************************/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Failed Contact Counter command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_READ_FAILED_CONTACT_COUNT_T;

typedef struct
{
    hci_connection_handle_t handle;
    uint16_t                failed_contact_count;
} HCI_READ_FAILED_CONTACT_COUNT_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Reset Failed Contact Counter command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_RESET_FAILED_CONTACT_COUNT_T;

typedef struct
{
    hci_connection_handle_t handle;
} HCI_RESET_FAILED_CONTACT_COUNT_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Get Link Quality command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_GET_LINK_QUALITY_T;

typedef struct
{
    hci_connection_handle_t handle;
    uint8_t                 link_quality;
} HCI_GET_LINK_QUALITY_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read RSSI command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDR_TRANSPORT
} HCI_READ_RSSI_T;

typedef struct
{
    hci_connection_handle_t handle;
    int8_t                  rssi;
} HCI_READ_RSSI_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read AFH channel map.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t handle;
    ADDED_BD_ADDRESS
} HCI_READ_AFH_CHANNEL_MAP_T;

typedef struct
{
    hci_connection_handle_t  handle;
    uint8_t                  mode;
    uint8_t                  map[10];
} HCI_READ_AFH_CHANNEL_MAP_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read BT Clock
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  handle;
    ADDED_BD_ADDRESS
    uint8_t                  which_clock;
} HCI_READ_CLOCK_T;

typedef struct
{
    hci_connection_handle_t  handle;
    uint32_t                 clock;
    uint16_t                 accuracy;
} HCI_READ_CLOCK_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Encryption Key Size
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  handle;
    ADDED_BD_ADDRESS
} HCI_READ_ENCRYPTION_KEY_SIZE_T;

typedef struct
{
    hci_connection_handle_t  handle;
    uint8_t                  key_size;
} HCI_READ_ENCRYPTION_KEY_SIZE_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Set Triggered Clock Capture
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  handle;
    uint8_t                  enable;
    uint8_t                  which_clock;
    uint8_t                  lpo_allowed;
    uint8_t                  num_events_to_filter;
} HCI_SET_TRIGGERED_CLOCK_CAPTURE_T;


/******************************************************************************
 *
 *      Testing Commands Structures
 *
 *****************************************************************************/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Loopback Mode command/event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_READ_LOOPBACK_MODE_T;

typedef struct
{
    /* 1 - local loopback enable 2 - remote Loopback enable */
    uint8_t                  mode; 
} HCI_READ_LOOPBACK_MODE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     Write Loopback Mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    /* 1 - local loopback enable 2 - remote Loopback enable */
    uint8_t                  mode; 
} HCI_WRITE_LOOPBACK_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Enable Device Under Test Mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ENABLE_DUT_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     Simple Pairing
 *     Write Simple Pairing Debug Mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  simple_pairing_debug_mode;
} HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     Write Secure Connections Test Mode command.
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  handle;
    uint8_t                  dm1_acl_u_mode;
    uint8_t                  esco_loopback_mode;
} HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE_T;

typedef struct
{
    hci_connection_handle_t  handle;
} HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE_RET_T;


/******************************************************************************
 *
 *      ULP Command Structures
 *
 *****************************************************************************/

 /*---------------------------------------------------------------------------*
  * PURPOSE
  *     HCI ULP Set Event Mask command
  *
  *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T      common;
    hci_event_mask_t          ulp_event_mask[2];
} HCI_ULP_SET_EVENT_MASK_T;


 /*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Buffer Size command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T      common;
} HCI_ULP_READ_BUFFER_SIZE_T;

typedef struct
{
    uint16_t                  data_packet_length;
    uint8_t                   num_data_packets;
} HCI_ULP_READ_BUFFER_SIZE_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Local Supported Features command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_READ_LOCAL_SUPPORTED_FEATURES_T;

typedef struct
{
    uint8_t                  feature_set[8];
} HCI_ULP_READ_LOCAL_SUPPORTED_FEATURES_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Random Address command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    BD_ADDR_T                random_address;
} HCI_ULP_SET_RANDOM_ADDRESS_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Advertising Parameters command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 adv_interval_min;
    uint16_t                 adv_interval_max;
    uint8_t                  advertising_type;
    uint8_t                  own_address_type;
    uint8_t                  direct_address_type;
    BD_ADDR_T                direct_address;
    uint8_t                  advertising_channel_map;
    uint8_t                  advertising_filter_policy;
} HCI_ULP_SET_ADVERTISING_PARAMETERS_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Advertising Channel TX Power command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_T;

typedef struct
{
    int8_t                   tx_power;
} HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Advertising Data command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  advertising_data_len;
    uint8_t                  advertising_data[31];
} HCI_ULP_SET_ADVERTISING_DATA_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Scan Response Data command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  scan_response_data_len;
    uint8_t                  scan_response_data[31];
} HCI_ULP_SET_SCAN_RESPONSE_DATA_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Advertise Enable command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  advertising_enable;
} HCI_ULP_SET_ADVERTISE_ENABLE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Scan Parameters command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  scan_type;
    uint16_t                 scan_interval;
    uint16_t                 scan_window;
    uint8_t                  own_address_type;
    uint8_t                  scanning_filter_policy;
} HCI_ULP_SET_SCAN_PARAMETERS_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Scan Enable command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  scan_enable;
    uint8_t                  filter_duplicates;
} HCI_ULP_SET_SCAN_ENABLE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Create Connection command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 scan_interval;
    uint16_t                 scan_window;
    uint8_t                  initiator_filter_policy;
    uint8_t                  peer_address_type;
    BD_ADDR_T                peer_address;
    uint8_t                  own_address_type;
    uint16_t                 conn_interval_min;
    uint16_t                 conn_interval_max;
    uint16_t                 conn_latency;
    uint16_t                 supervision_timeout;
    uint16_t                 minimum_ce_length;
    uint16_t                 maximum_ce_length;
} HCI_ULP_CREATE_CONNECTION_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Create Connection Cancel command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_CREATE_CONNECTION_CANCEL_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read White List Size command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_READ_WHITE_LIST_SIZE_T;

typedef struct
{
    uint8_t                  white_list_size;
} HCI_ULP_READ_WHITE_LIST_SIZE_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Clear White List command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_CLEAR_WHITE_LIST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Add Device to White List command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  address_type;
    BD_ADDR_T                address;
} HCI_ULP_ADD_DEVICE_TO_WHITE_LIST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Remove Device From White List command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  address_type;
    BD_ADDR_T                address;
} HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Connection Update command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  connection_handle;
    ADDED_TYPED_BD_ADDRESS
    uint16_t                 conn_interval_min;
    uint16_t                 conn_interval_max;
    uint16_t                 conn_latency;
    uint16_t                 supervision_timeout;
    uint16_t                 minimum_ce_length;
    uint16_t                 maximum_ce_length;
} HCI_ULP_CONNECTION_UPDATE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Host Channel Classification command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  channel_map[5];
} HCI_ULP_SET_HOST_CHANNEL_CLASSIFICATION_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Channel Map command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  connection_handle;
    ADDED_TYPED_BD_ADDRESS
} HCI_ULP_READ_CHANNEL_MAP_T;

typedef struct
{
    hci_connection_handle_t  connection_handle;
    uint8_t                  ulp_channel_map[5];
} HCI_ULP_READ_CHANNEL_MAP_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Remote Used Features command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T      common;
    hci_connection_handle_t   connection_handle;
    ADDED_TYPED_BD_ADDRESS
} HCI_ULP_READ_REMOTE_USED_FEATURES_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Encrypt command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  aes_key[16];
    uint8_t                  plaintext_data[16];
} HCI_ULP_ENCRYPT_T;

typedef struct
{
    uint8_t                  encrypted_data[16];
} HCI_ULP_ENCRYPT_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Rand command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T      common;
} HCI_ULP_RAND_T;

typedef struct
{
    uint8_t                   random_number[8];
} HCI_ULP_RAND_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Start Encryption command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  connection_handle;
    uint8_t                  random_number[8];
    uint16_t                 encrypted_diversifier;
    uint8_t                  long_term_key[16];
} HCI_ULP_START_ENCRYPTION_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Long Term Key Requested Reply command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  connection_handle;
    uint8_t                  long_term_key[16];
} HCI_ULP_LONG_TERM_KEY_REQUEST_REPLY_T;

typedef struct
{
    hci_connection_handle_t  connection_handle;
} HCI_ULP_LONG_TERM_KEY_REQUEST_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Long Term Key Requested Negative Reply command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    hci_connection_handle_t  connection_handle;
} HCI_ULP_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_T;

typedef struct
{
    hci_connection_handle_t  connection_handle;
} HCI_ULP_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Supported States command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_READ_SUPPORTED_STATES_T;

typedef struct
{
    uint8_t                  supported_states[8];
} HCI_ULP_READ_SUPPORTED_STATES_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Receiver Test
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  rx_channel;
} HCI_ULP_RECEIVER_TEST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Transmitter Test
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  tx_channel;
    uint8_t                  length_test_data;
    uint8_t                  packet_payload;
} HCI_ULP_TRANSMITTER_TEST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Test End
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_TEST_END_T;

typedef struct
{
    uint16_t                 number_of_packets;
} HCI_ULP_TEST_END_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Remote Connection Parameter Request Reply
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t connection_handle;
    uint16_t                interval_min;
    uint16_t                interval_max;
    uint16_t                latency;
    uint16_t                timeout;
    uint16_t                minimum_ce_length;
    uint16_t                maximum_ce_length;
} HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY_T;

typedef struct
{
    hci_connection_handle_t connection_handle;
} HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Remote Connection Parameter Request Negative Reply
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t connection_handle;
    hci_error_t             reason;
} HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY_T;

typedef struct
{
    hci_connection_handle_t  connection_handle;
} HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Data Length
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    hci_connection_handle_t connection_handle;
    uint16_t                txoctets;
    uint16_t                txtime;
} HCI_ULP_SET_DATA_LENGTH_T;

typedef struct
{
    hci_connection_handle_t connection_handle;
} HCI_ULP_SET_DATA_LENGTH_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Suggested Default Data Length
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
} HCI_ULP_READ_SUGGESTED_DEFAULT_DATA_LENGTH_T;

typedef struct
{
    uint16_t                tx_octets;
    uint16_t                tx_time;
} HCI_ULP_READ_SUGGESTED_DEFAULT_DATA_LENGTH_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Write Suggested Default Data Length
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    uint16_t                suggested_max_tx_octets;
    uint16_t                suggested_max_tx_time;
} HCI_ULP_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Maximum Data Length
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T    common;
} HCI_ULP_READ_MAXIMUM_DATA_LENGTH_T;

typedef struct
{
    uint16_t                supported_max_tx_octets;
    uint16_t                supported_max_tx_time;
    uint16_t                supported_max_rx_octets;
    uint16_t                supported_max_rx_time;
} HCI_ULP_READ_MAXIMUM_DATA_LENGTH_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Local P-256 Public Key
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_READ_LOCAL_P256_PUBLIC_KEY_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Generate DHKey
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t remote_p256_public_key[64];
} HCI_ULP_GENERATE_DHKEY_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Add Device to Resolving List command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  peer_identity_address_type;
    BD_ADDR_T                peer_identity_address;
    uint8_t                  peer_irk[16];
    uint8_t                  local_irk[16];
} HCI_ULP_ADD_DEVICE_TO_RESOLVING_LIST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Remove Device From Resolving List command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  peer_identity_address_type;
    BD_ADDR_T                peer_identity_address;
} HCI_ULP_REMOVE_DEVICE_FROM_RESOLVING_LIST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Clear Resolving List command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_CLEAR_RESOLVING_LIST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Resolving List Size command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
} HCI_ULP_READ_RESOLVING_LIST_SIZE_T;

typedef struct
{
    uint8_t                  resolving_list_size;
} HCI_ULP_READ_RESOLVING_LIST_SIZE_RET_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Peer Resolvable Address command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  peer_identity_address_type;
    BD_ADDR_T                peer_identity_address;
} HCI_ULP_READ_PEER_RESOLVABLE_ADDRESS_T;

typedef struct
{
    BD_ADDR_T                peer_resolvable_address;
} HCI_ULP_READ_PEER_RESOLVABLE_ADDRESS_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read Local Resolvable Address command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  peer_identity_address_type;
    BD_ADDR_T                peer_identity_address;
} HCI_ULP_READ_LOCAL_RESOLVABLE_ADDRESS_T;

typedef struct
{
    BD_ADDR_T                local_resolvable_address;
} HCI_ULP_READ_LOCAL_RESOLVABLE_ADDRESS_RET_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Address Resolution Enable command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  address_resolution_enable;
} HCI_ULP_SET_ADDRESS_RESOLUTION_ENABLE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Resolvable Private Address Timeout command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint16_t                 rpa_timeout;
} HCI_ULP_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Privacy Mode command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  peer_identity_address_type;
    BD_ADDR_T                peer_identity_address;
    uint8_t                  privacy_mode;
} HCI_ULP_SET_PRIVACY_MODE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Read PHY command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    hci_connection_handle_t   connection_handle;
    uint8_t                   tx_phy;
    uint8_t                   rx_phy;
} HCI_ULP_READ_PHY_RET_T;

typedef struct
{
    HCI_COMMAND_COMMON_T      common;
    hci_connection_handle_t   connection_handle;
} HCI_ULP_READ_PHY_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set Host Preferred PHY command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T      common;
    uint8_t                   all_phys;
    uint8_t                   tx_phys;
    uint8_t                   rx_phys;
} HCI_ULP_SET_DEFAULT_PHY_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Set PHY command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T      common;
    hci_connection_handle_t   connection_handle;
    uint8_t                   all_phys;
    uint8_t                   tx_phys;
    uint8_t                   rx_phys;
    uint16_t                  phy_options;
} HCI_ULP_SET_PHY_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Enhanced Receiver Test command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  rx_channel;
    uint8_t                  phy;
    uint8_t                  mod_index;
} HCI_ULP_ENHANCED_RECEIVER_TEST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *     HCI ULP Enhanced Transmitter Test command
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_COMMAND_COMMON_T     common;
    uint8_t                  tx_channel;
    uint8_t                  length_test_data;
    uint8_t                  packet_payload;
    uint8_t                  phy;
} HCI_ULP_ENHANCED_TRANSMITTER_TEST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Allow Debug commands to go over HCI.  These are a special case of
 *      manufacturer-specific commands.
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint8_t                 command;
    uint16_t                command_size;
    uint16_t                handle;
    uint16_t                num0;
    uint16_t                num1;
    uint16_t                num2;
    uint16_t                num3;
    uint16_t                num4;
    uint16_t                num5;
    uint16_t                num6;
    uint16_t                num7;
    uint16_t                num8;
} HCI_DEBUG_REQUEST_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Allow manufacturer's extension commands to go over HCI.  These are
 *      used for:
 *
 *      1. To tunnel additional protocols over HCI (see specification
 *          bc01-s-023c).  These are carried in BlueCore-Friendly format.
 *      2. For debug commands.
 *
 *----------------------------------------------------------------------------*/
typedef union
{
    uint8_t              *bcf;              /* carries tunnelled primitives */
    HCI_DEBUG_REQUEST_T  debug_request;     /* carries debug requests       */
} MNFR_EXTN_PAYLOAD_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Allow manufacturer's extension commands to go over HCI.  These are
 *      used for:
 *
 *      1. To tunnel additional protocols over HCI (see specification
 *          bc01-s-023c).
 *      2. For debug commands.
 *
 *      The payload_descriptor indicates the channel ID and contains two
 *      bits which control fragmentation and reassembly of HCI commands
 *      into protocol primitives.  Bits in the payload descriptor are:
 *
 *      7       Indicates 'fragment end'
 *      6       Indicates 'fragment start'
 *      [5:0]   Channel ID
 *
 *      Channel IDs in the range 0 to 15 are equivalent to the BCSP protocol
 *      IDs, although some are not used.  Channel ID CHANNEL_ID_DEBUG indicates
 *      that the command is a debug command.
 *
 *----------------------------------------------------------------------------*/

#define FRAGMENT_END        (0x80)
#define FRAGMENT_START      (0x40)
#define CHANNEL_ID_DEBUG    (20) /* same as HOSTIO_PROTOCOL_DEBUG used in fw*/
#define CHANNEL_ID_MASK     (0x3F)

typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    uint8_t                 payload_descriptor;
    MNFR_EXTN_PAYLOAD_T     payload;
} HCI_MNFR_EXTENSION_T;

typedef struct
{
    HCI_COMMAND_COMMON_T    common;
    uint8_t                 *vs_data_part[HCI_VS_DATA_BYTE_PACKET_PTRS];
} HCI_VS_COMMAND_T;

typedef struct
{
    uint8_t                 *vs_data_part[HCI_VS_DATA_BYTE_PACKET_PTRS];
} HCI_VS_COMMAND_RET_T;


/******************************************************************************
 *
 *      Event Structures
 *
 *****************************************************************************/

 typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    BD_ADDR_T                bd_addr;
} HCI_EV_COMMON_EVENT_T;

/* Vendor specific event used to send AFH map available information */
typedef struct
{
    uint8_t             event_code; /* carries vendor event */
    uint8_t             map[10];    /* AFH Map data */
    uint32_t            clock;      /* Instant at which AFH is taken */
} HCI_EV_CSB_AFH_MAP_AVAILABLE_T;

 /*----------------------------------------------------------------------------*
 * PURPOSE
 *      Debug Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint8_t debug_event;
    uint16_t num0;
    uint16_t num1;
    uint16_t num2;
    uint16_t num3;
    uint16_t num4;
    uint16_t num5;
    uint16_t num6;
    uint16_t num7;
    uint16_t num8;
} HCI_EV_DEBUG_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Allow manufacturer's extension events to go over HCI or to Bluestack.
 *      These are used for:
 *
 *      1. To tunnel additional protocols over HCI (see specification
 *          bc01-s-023c).  These are carried in BlueCore-Friendly format.
 *      2. For responses to debug commands over HCI.
 *      3. Vendor specific events to go to Bluestack (If DM is on chip).
 *
 *      Note: If new vendor specific event is added in EV_MNFR_EXTN_PAYLOAD_T
 *      then HCI_EV_MNFR_EXTENSION_PARAM_LEN length need to be revised.
 *
 *----------------------------------------------------------------------------*/
typedef union
{
    uint8_t                    *bcf;         /* carries tunnelled primitives */
    uint8_t                    vendor_event; /* carries vendor event code */
    HCI_EV_DEBUG_T             debug_event;  /* carries debug events */
    HCI_EV_CSB_AFH_MAP_AVAILABLE_T csb_afh_map_available; /*carries afh data */
    /* Add new vendor specific event here */
} EV_MNFR_EXTN_PAYLOAD_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Allow manufacturer's extension eventes to go over HCI.  These are
 *      used for:
 *
 *      1. To tunnel additional protocols over HCI (see specification
 *          bc01-s-023c).
 *      2. For responses to debug commands.
 *
 *      The payload_descriptor indicates the channel ID and contains two
 *      bits which control fragmentation and reassembly of HCI commands
 *      into protocol primitives.  Bits in the payload descriptor are:
 *
 *      7       Indicates 'fragment end'
 *      6       Indicates 'fragment start'
 *      [5:0]   Channel ID
 *
 *      Channel IDs in the range 0 to 15 are equivalent to the BCSP protocol
 *      IDs, although some are not used.  Channel ID CHANNEL_ID_DEBUG indicates
 *      that the command is a debug command.
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T          event;
/* coding of payload_descriptor is the same as HCI_MNFR_EXTENSION_T */
    uint8_t                     payload_descriptor;
    EV_MNFR_EXTN_PAYLOAD_T      payload;

} HCI_EV_MNFR_EXTENSION_T;

typedef struct
{
    HCI_EVENT_COMMON_T          event;
    uint8_t                     *vs_data_part[HCI_VS_DATA_BYTE_PACKET_PTRS];
} HCI_EV_VS_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Inquiry Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
} HCI_EV_INQUIRY_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Inquiry Result Event
 *
 *      NOTE: Variable struct.
 *            We have num_responses results.
 *
 *      Max Size of HCI payload = 255, Variable Size = 255-(uint8_t) = 254
 *
 *      HCI_MAX_INQ_RESULT_PER_PTR - Number of HCI_INQ_RESULT_T's
 *        in a pointed at block.
 *      HCI_MAX_INQ_RESULT_PTRS - Number of pointer blocks.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    BD_ADDR_T                bd_addr;
    page_scan_rep_mode_t     page_scan_rep_mode;
    uint8_t                  page_scan_period_mode;
    page_scan_mode_t         page_scan_mode;
    uint24_t                 dev_class;  /* Lower 3 bytes only used */
    bt_clock_offset_t        clock_offset;
} HCI_INQ_RESULT_T;

typedef struct
{
    HCI_EVENT_COMMON_T       event;
    uint8_t                  num_responses;
    /*
     * Pointers to pmalloc'd space containing the inquiry results.
     */
    /*
     * The array of pointers below, point to
     * HCI_INQ_RESULT_T[HCI_MAX_INQ_RESULT_PER_PTR],
     * ie a pointer to the beginning of X HCI_INQ_RESULT_T.
     */
    HCI_INQ_RESULT_T         *result[HCI_MAX_INQ_RESULT_PTRS];

} HCI_EV_INQUIRY_RESULT_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Inquiry Result With RSSI Event
 *
 *      NOTE: Variable struct.
 *            We have num_responses results.
 *
 *      Max Size of HCI payload = 255, Variable Size = 255-(uint8_t) = 254
 *
 *      HCI_MAX_INQ_RESULT_PER_PTR - Number of HCI_INQ_RESULT_T's
 *        in a pointed at block.
 *      HCI_MAX_INQ_RESULT_PTRS - Number of pointer blocks.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    BD_ADDR_T                bd_addr;
    page_scan_rep_mode_t     page_scan_rep_mode;
    uint8_t                  page_scan_period_mode;
    uint24_t                 dev_class;  /* Lower 3 bytes only used */
    bt_clock_offset_t        clock_offset;
    int8_t                   rssi;
} HCI_INQ_RESULT_WITH_RSSI_T;

typedef struct
{
    HCI_EVENT_COMMON_T       event;
    uint8_t                  num_responses;
    /*
     * Pointers to pmalloc'd space containing the inquiry results.
     */
    /*
     * The array of pointers below, point to
     * HCI_INQ_RESULT_T[HCI_MAX_INQ_RESULT_PER_PTR],
     * ie a pointer to the beginning of X HCI_INQ_RESULT_T.
     */
    HCI_INQ_RESULT_WITH_RSSI_T         *result[HCI_MAX_INQ_RESULT_PTRS];

} HCI_EV_INQUIRY_RESULT_WITH_RSSI_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Extended Inquiry Result Event
 *
 *      NOTE: Variable struct; the length depends on the received EIR data.
 *
 *      Max Size of HCI payload = 255, Variable Size <= 240
 *
 *      HCI_MAX_INQ_RESULT_PER_PTR - Number of HCI_INQ_RESULT_T's
 *        in a pointed at block.
 *      HCI_MAX_INQ_RESULT_PTRS - Number of pointer blocks.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T          event;
    uint8_t                     num_responses;
    HCI_INQ_RESULT_WITH_RSSI_T  result;

    /*
     * Pointers to pmalloc'd space containing the EIR data.
     */
    uint8_t                 *eir_data_part[HCI_EIR_DATA_PACKET_PTRS];

} HCI_EV_EXTENDED_INQUIRY_RESULT_T;

 /*---------------------------------------------------------------------------*
 * PURPOSE
 *      Connection Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    BD_ADDR_T                bd_addr;
    uint8_t                  link_type;
    uint8_t                  enc_mode;
} HCI_EV_CONN_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Connection Request Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
    uint24_t                 dev_class;     /* Lower 3 bytes only used */
    uint8_t                  link_type;
} HCI_EV_CONN_REQUEST_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Disconnection Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    hci_error_t              reason;        /* Reason for disconnection 0x08, 0x13 - 0x16 only */
} HCI_EV_DISCONNECT_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Authentication Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
} HCI_EV_AUTH_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Remote Name Request Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    BD_ADDR_T                bd_addr;
    /*
     * The pointers below, point to a byte array of size HCI_LOCAL_NAME_BYTES_PER_PTR.
     */
    uint8_t                *name_part[HCI_LOCAL_NAME_BYTE_PACKET_PTRS];
} HCI_EV_REMOTE_NAME_REQ_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Encryption Change Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    hci_link_enc_t           enc_enable;
} HCI_EV_ENCRYPTION_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Change Connection Link Key Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
} HCI_EV_CHANGE_CONN_LINK_KEY_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Master Link Key Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    hci_key_flag_t           key_flag;
} HCI_EV_MASTER_LINK_KEY_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Remote Supported Features Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    uint16_t                 lmp_supp_features[4];
} HCI_EV_READ_REM_SUPP_FEATURES_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Remote Version Information Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    uint8_t                  lmp_version;        /* defined in LMP */
    uint16_t                 manuf_name;         /* defined in LMP */
    uint16_t                 lmp_subversion;
} HCI_EV_READ_REMOTE_VER_INFO_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      QoS Setup Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    uint8_t                  flags;              /* reserved */
    hci_qos_type_t           service_type;
    uint32_t                 token_rate;
    uint32_t                 peak_bandwidth;
    uint32_t                 latency;
    uint32_t                 delay_variation;
} HCI_EV_QOS_SETUP_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Command Status Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    uint8_t                  num_hci_command_pkts;
    hci_op_code_t            op_code;            /* op code of command that
                                                    caused this event */
} HCI_EV_COMMAND_STATUS_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Hardware Error Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    uint8_t                  hw_error;           /* TBD */
} HCI_EV_HARDWARE_ERROR_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Flush Occurred Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
} HCI_EV_FLUSH_OCCURRED_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Enhanced flush complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
} HCI_EV_ENHANCED_FLUSH_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Role Change Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    BD_ADDR_T                bd_addr;
    hci_role_t               new_role;
} HCI_EV_ROLE_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Number of completed packets Event
 *
 *      NOTE: Another variable struct. num_handles denotes how many connection
 *            handles follow and then the same number of counts.
 *
 *      Max Size of HCI payload = 256, Variable Size = 256-(uint8_t) = 255
 *
 *      HCI_BYTE_SIZE_OF_HANDLE_COMPLETE_T  - Byte size of HANDLE_COMPLETE_T
 *        when transmitted across the HCI interface.
 *      HCI_EV_NUMBER_COMPLETED_PACKETS_PER_PTR - Number of HANDLE_COMPLETE_T's
 *        in a pointed at block.
 *      HCI_EV_NUMBER_COMPLETED_PACKET_PTRS - Number of pointer blocks.
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    uint8_t                  num_handles;

    /*
     * HANDLE_COMPLETE_T (*num_completed_pkts_ptr[4])[16]
     *
     * The pointers below, point to
     * HANDLE_COMPLETE_T[HCI_HOST_NUM_COMPLETED_PACKETS_PER_PTR],
     * ie a pointer to the beginning of 16 HANDLE_COMPLETE_T.
     */
    HANDLE_COMPLETE_T *num_completed_pkts_ptr[HCI_EV_NUM_HANDLE_COMPLETE_PACKET_PTRS];

} HCI_EV_NUMBER_COMPLETED_PKTS_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Mode Change Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    hci_bt_mode_t            curr_mode;
    uint16_t                 interval;
} HCI_EV_MODE_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Return Link Keys Event
 *
 *      NOTE: Each pointed at block contain a single LINK_KEY_BDADDR_T
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    uint8_t                  number_keys;
    /*
     * Store single link_key_bd_addr per ptr.
     */
    LINK_KEY_BD_ADDR_T  *link_key_bd_addr[HCI_STORED_LINK_KEY_MAX];

} HCI_EV_RETURN_LINK_KEYS_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      PIN Code Request Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
} HCI_EV_PIN_CODE_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Link Key Request Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
} HCI_EV_LINK_KEY_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Link Key Notification Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
    uint8_t                  link_key[SIZE_LINK_KEY];
    uint8_t                  key_type;
} HCI_EV_LINK_KEY_NOTIFICATION_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Loopback Command Event
 *
 *      NOTE: command_packet is variable length, it contains the full command
 *            packet sent, including the header.
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint8_t     bytes[HCI_LOOPBACK_BYTES_PER_PTR];
} HCI_EV_LOOPBACK_BYTE_STRUCT_T;

typedef struct
{
    HCI_EVENT_COMMON_T              event;
    HCI_EV_LOOPBACK_BYTE_STRUCT_T   *loopback_part_ptr[HCI_LOOPBACK_BYTE_PACKET_PTRS];
} HCI_EV_LOOPBACK_COMMAND_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Data Buffer Overflow Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    uint8_t                  link_type;
} HCI_EV_DATA_BUFFER_OVERFLOW_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Max Slots Change Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
    uint8_t                  lmp_max_slots;
} HCI_EV_MAX_SLOTS_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Clock Offset Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    clock_offset_t           clock_offset;
} HCI_EV_READ_CLOCK_OFFSET_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Connection Packet Type Changed Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    hci_pkt_type_t           pkt_type;
} HCI_EV_CONN_PACKET_TYPE_CHANGED_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      QoS Violation Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
} HCI_EV_QOS_VIOLATION_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Page Scan Mode Change Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T      event;
    BD_ADDR_T               bd_addr;
    page_scan_mode_t        page_scan_mode;
} HCI_EV_PAGE_SCAN_MODE_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Page Scan Repitition Mode Change Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T      event;
    BD_ADDR_T               bd_addr;
    page_scan_rep_mode_t    page_scan_rep_mode;
} HCI_EV_PAGE_SCAN_REP_MODE_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Flow Specification changed event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T      event;
    hci_return_t            status;
    hci_connection_handle_t handle;
    uint8_t                 flags;
    uint8_t                 flow_direction; /* 0=out (to air), 1=in (from air) */
    uint8_t                 service_type;
    uint32_t                token_rate;
    uint32_t                token_bucket_size;
    uint32_t                peak_bandwidth;
    uint32_t                access_latency;
} HCI_EV_FLOW_SPEC_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Read Remote Extended Features Complete Event
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    uint8_t                  page_num;
    uint8_t                  max_page_num;
    uint16_t                 lmp_ext_features[4];
} HCI_EV_READ_REM_EXT_FEATURES_COMPLETE_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      FIXED_ADDRESS
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    BD_ADDR_T                bd_addr;
} HCI_EV_FIXED_ADDRESS_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      ALIAS_ADDRESS
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    hci_connection_handle_t  handle;
    BD_ADDR_T                bd_addr;
} HCI_EV_ALIAS_ADDRESS_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      GENERATE_ALIAS_REQ
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
} HCI_EV_GENERATE_ALIAS_REQ_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      ACTIVE_ADDRESS
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr_fixed;
    BD_ADDR_T                bd_addr;
} HCI_EV_ACTIVE_ADDRESS_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      ALLOW_PRIVATE_PAIRING
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
} HCI_EV_ALLOW_PRIVATE_PAIRING_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      ALIAS_ADDRESS_REQUEST
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
} HCI_EV_ALIAS_ADDRESS_REQ_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      ALIAS_NOT_RECOGNISED
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;             /* cast to error if error */
    BD_ADDR_T                bd_addr;
} HCI_EV_ALIAS_NOT_RECOGNISED_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      FIXED_ADDRESS_ATTEMPT
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    uint32_t                 reserved;
} HCI_EV_FIXED_ADDRESS_ATTEMPT_T;


 /*---------------------------------------------------------------------------*
 * PURPOSE
 *      Synchronous Connection Complete Event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;
    hci_connection_handle_t  handle;
    BD_ADDR_T                bd_addr;
    uint8_t                  link_type;
    uint8_t                  tx_interval;
    uint8_t                  wesco;
    uint16_t                 rx_packet_length;
    uint16_t                 tx_packet_length;
    uint8_t                  air_mode;
} HCI_EV_SYNC_CONN_COMPLETE_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      Synchronous Connection Changed Event
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;
    hci_connection_handle_t  handle;
    uint8_t                  tx_interval;
    uint8_t                  wesco;
    uint16_t                 rx_packet_length;
    uint16_t                 tx_packet_length;
} HCI_EV_SYNC_CONN_CHANGED_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      Sniff Sub Rate Complete Event (0x2e)
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;
    hci_connection_handle_t  handle;
    uint16_t                 transmit_latency;
    uint16_t                 receive_latency;
    uint16_t                 remote_timeout;
    uint16_t                 local_timeout;
} HCI_EV_SNIFF_SUB_RATE_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      Encryption Key Refresh Complete (0x30)
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;
    hci_connection_handle_t  handle;
} HCI_EV_ENCRYPTION_KEY_REFRESH_COMPLETE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      IO Capability Request Event (0x31)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
} HCI_EV_IO_CAPABILITY_REQUEST_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      IO Capability Response Event (0x32)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
    uint8_t                  io_capability;
    uint8_t                  oob_data_present;
    uint8_t                  authentication_requirements;
} HCI_EV_IO_CAPABILITY_RESPONSE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Confirmation Request Event (0x33)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
    uint32_t                 numeric_value;
} HCI_EV_USER_CONFIRMATION_REQUEST_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Passkey Request Event (0x34)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
} HCI_EV_USER_PASSKEY_REQUEST_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Remote OOB Data Request Event (0x35)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
} HCI_EV_REMOTE_OOB_DATA_REQUEST_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      Simple Pairing Complete Event (0x36)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;
    BD_ADDR_T                bd_addr;
} HCI_EV_SIMPLE_PAIRING_COMPLETE_T;


/*---------------------------------------------------------------------------*
 * PURPOSE
 *      Link Supervision Timeout Changed Event (0x38)
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
    uint16_t                 timeout;
} HCI_EV_LST_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Passkey Notification Event (0x3B)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
    uint32_t                 passkey;
} HCI_EV_USER_PASSKEY_NOTIFICATION_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Simple Pairing
 *      User Key Press Notification Event (0x3C)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
    uint8_t                  notification_type;
} HCI_EV_KEYPRESS_NOTIFICATION_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Remote Host Supported Features Notification Event (0x3D)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    BD_ADDR_T                bd_addr;
    uint16_t                 host_features[4];
}  HCI_EV_REM_HOST_SUPPORTED_FEATURES_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Triggered Clock Capture Event (0x4E)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T      event;
    hci_connection_handle_t handle;
    uint8_t                 which_clock;
    uint32_t                clock;
    uint16_t                slot_offset;
} HCI_EV_TRIGGERED_CLOCK_CAPTURE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Synchronization Train Complete Event (0x4F)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T event;
    hci_return_t       status;
} HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Synchronization Train Received Event (0x50)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_return_t             status;
    BD_ADDR_T                bd_addr;
    uint32_t                 clock_offset;
    uint8_t                  map[10];
    uint8_t                  lt_addr;
    uint32_t                 next_broadcast_instant;
    uint16_t                 csb_interval;
    uint8_t                  service_data;
} HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Connectionless Slave Broadcast Data Received Event (0x51)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T event;
    BD_ADDR_T          bd_addr;
    uint8_t            lt_addr;
    uint32_t           clock;
    uint32_t           offset;
    uint8_t            receive_status;
    uint8_t            fragment;
    uint8_t            data_length;
    /*
     * The pointers below, point to a byte array of size HCI_CSB_RECV_BYTES_PER_PTR.
     */
    uint8_t            *data_part[HCI_CSB_RECV_PACKET_PTRS];
} HCI_EV_CSB_RECEIVE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Connectionless Slave Broadcast Timeout Event (0x52)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T event;
    BD_ADDR_T          bd_addr;
    uint8_t            lt_addr;
} HCI_EV_CSB_TIMEOUT_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Truncated Page Complete event (0x53)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T event;
    hci_return_t       status;
    BD_ADDR_T          bd_addr;
} HCI_EV_TRUNCATED_PAGE_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Page Response Timeout event (0x54)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T event;
} HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Connectionless Slave Broadcast Channel Map Change  (0x55)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T event;
    uint8_t            map[10];
} HCI_EV_CSB_CHANNEL_MAP_CHANGE_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Inquiry Response Notification  (0x56)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T event;
    uint24_t           lap;
    int8_t             rssi;
} HCI_EV_INQUIRY_RESPONSE_NOTIFICATION_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Ping Failed Event (0x57)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T       event;
    hci_connection_handle_t  handle;
} HCI_EV_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_T;

/*------------------------ Start of ULP events -------------------------------*/

typedef struct
{
    hci_event_code_t event_code;          /* event code */
    uint8_t          length;              /* parameter total length */
    uint8_t          ulp_sub_opcode;
} HCI_ULP_EVENT_COMMON_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Connection Creation
 *      HCI ULP Connection Complete Event (0x3E, subevent code: 0x01)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_return_t            status;
    hci_connection_handle_t connection_handle;
    uint8_t                 role;
    uint8_t                 peer_address_type;
    BD_ADDR_T               peer_address;
    uint16_t                conn_interval;
    uint16_t                conn_latency;
    uint16_t                supervision_timeout;
    uint8_t                 clock_accuracy;
}  HCI_EV_ULP_CONNECTION_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Advertising
 *      HCI ULP Advertising Report Event (0x3E, subevent code: 0x02)
 *
 * Note:
 * As per spec (Vol-2/Part-E/Section-7.7.65.2), "The Controller may queue these
 * advertising reports and send information from multiple devices in one LE Advertising
 * Report event.."
 * We because of memory constraints in on-chip variants, don't queue advertising
 * reports. So each advertising report corresponds to a single advertising indicator
 * from single peer device.
 * 
 * Also data field is defined as an array of pointers because of the framework to handle
 * variable length parameters in HCI commands/events and to move such variable length
 * parameters into a separate memory block.
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    uint8_t                 num_reports;
    uint8_t                 event_type;
    uint8_t                 address_type;
    BD_ADDR_T               address;
    uint8_t                 length_data;
    uint8_t                 *data[HCI_ULP_ADVERTISING_DATA_PTRS];
    int8_t                  rssi;
}  HCI_EV_ULP_ADVERTISING_REPORT_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Connection Update
 *      HCI ULP Connection Update Complete Event (0x3E, subevent code: 0x03)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_return_t            status;
    hci_connection_handle_t connection_handle;
    uint16_t                conn_interval;
    uint16_t                conn_latency;
    uint16_t                supervision_timeout;
}  HCI_EV_ULP_CONNECTION_UPDATE_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Read Remote Used Features
 *      HCI ULP Read Remote Used Features Complete Event (0x3E,
 *                                                      subevent code: 0x04)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T     event;
    hci_return_t               status;
    hci_connection_handle_t    connection_handle;
    uint8_t                    features[8];
}  HCI_EV_ULP_READ_REMOTE_USED_FEATURES_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Encryption
 *      HCI ULP Long Term Key Request Event (0x3E, subevent code: 0x05)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T     event;
    hci_connection_handle_t    connection_handle;
    uint8_t                    random_number[8];
    uint16_t                   encryption_diversifier;
}  HCI_EV_ULP_LONG_TERM_KEY_REQUEST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Connection Update
 *      HCI ULP Remote Connection Parameter Request (0x3E, subevent code: 0x06)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_connection_handle_t connection_handle;
    uint16_t                interval_min;
    uint16_t                interval_max;
    uint16_t                latency;
    uint16_t                timeout;
} HCI_EV_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Encryption
 *      HCI ULP Read Local P-256 Public Key Complete Event (0x3E, 
 *                                                        subevent code: 0x08)
 * NOTE
 *      The local_p256_public_key parameter is in the spec as an array of
 *      octets, however, it comes out of the ECC code as a little endian array
 *      of words and we'll serialise that as a little endian array of octets,
 *      making the octets on the wire correct. Making it a uint8_t would use
 *      twice as much memory for this struct and require code to unpack the ECC
 *      output into local_p256_public_key.
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_return_t            status;
    uint16_t                local_p256_public_key[32];
} HCI_EV_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Encryption
 *      HCI ULP Generate DHKey Complete Event (0x3E, subevent code: 0x09)
 *
 * NOTE
 *      The dhkey parameter is in the spec as an array of octets, however,
 *      it comes out of the ECC code as a little endian array of words and
 *      we'll serialise that as a little endian array of octets, making the
 *      octets on the wire correct. Making it a uint8_t would use twice as much
 *      memory for this struct and require code to unpack the ECC output into
 *      dhkey.
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_return_t            status;
    uint16_t                 dhkey[16];
} HCI_EV_ULP_GENERATE_DHKEY_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Data Length change Event
 *      HCI ULP Data Length Change Event (0x3E, subevent code: 0x07)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_connection_handle_t connection_handle;
    uint16_t                max_tx_octets;
    uint16_t                max_tx_time;
    uint16_t                max_rx_octets;
    uint16_t                max_rx_time;
}  HCI_EV_ULP_DATA_LENGTH_CHANGE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Enhanced Connection Complete
 *      HCI ULP Enhanced Connection Complete Event (0x3E, subevent code: 0x0A)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_return_t            status;
    hci_connection_handle_t connection_handle;
    uint8_t                 role;
    uint8_t                 peer_address_type;
    BD_ADDR_T               peer_address;
    BD_ADDR_T               local_resolvable_private_address;
    BD_ADDR_T               peer_resolvable_private_address;
    uint16_t                conn_interval;
    uint16_t                conn_latency;
    uint16_t                supervision_timeout;
    uint8_t                 master_clock_accuracy;
} HCI_EV_ULP_ENHANCED_CONNECTION_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Direct Advertising
 *      HCI ULP Direct Advertising Report Event (0x3E, subevent code: 0x0B)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    uint8_t                 num_reports;
    uint8_t                 event_type;
    uint8_t                 address_type;
    BD_ADDR_T               address;
    uint8_t                 direct_address_type;
    BD_ADDR_T               direct_address;
    int8_t                  rssi;
} HCI_EV_ULP_DIRECT_ADVERTISING_REPORT_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP PHY Update
 *      HCI ULP PHY Update Complete (0x3E, subevent code: 0x0C)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_return_t            status;
    hci_connection_handle_t connection_handle;
    uint8_t                 tx_phy;
    uint8_t                 rx_phy;
} HCI_EV_ULP_PHY_UPDATE_COMPLETE_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      ULP Connection Creation
 *      HCI ULP Channel Selection Algorithm (0x3E, subevent code: 0x14)
 *
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_ULP_EVENT_COMMON_T  event;
    hci_connection_handle_t connection_handle;
    uint8_t                 channel_selection_algorithm;
} HCI_EV_ULP_CHANNEL_SELECTION_ALGORITHM_T;

/*------------------------------------------------------------------------
 *
 *      UNION OF COMMAND COMPLETE ARGUMENTS
 *
 *-----------------------------------------------------------------------*/

typedef union
{
    HCI_WITH_BD_ADDR_RET_T                           evt_with_bd_addr;
    HCI_WITH_HANDLE_RET_T                            evt_with_handle;
    HCI_BD_ADDR_GENERIC_RET_T                        Generic_BD_ADDR;

    /* Link Control */
    HCI_CREATE_CONNECTION_CANCEL_RET_T               create_connection_cancel_args;
    HCI_LINK_KEY_REQ_REPLY_RET_T                     link_key_req_reply_args;
    HCI_LINK_KEY_REQ_NEG_REPLY_RET_T                 link_key_reqneg_reply_args;
    HCI_PIN_CODE_REQ_REPLY_RET_T                     pin_code_req_reply_args;
    HCI_PIN_CODE_REQ_NEG_REPLY_RET_T                 pin_code_reqneg_reply_args;
    HCI_REMOTE_NAME_REQ_CANCEL_RET_T                 remote_name_req_cancel_args;
    HCI_READ_LMP_HANDLE_RET_T                        read_lmp_handle_args;
    HCI_PRIVATE_PAIRING_REQ_REPLY_RET_T              private_pairing_req_reply;
    HCI_PRIVATE_PAIRING_REQ_NEG_REPLY_RET_T          private_pairing_req_neg_reply;
    HCI_GENERATED_ALIAS_RET_T                        generated_alias;
    HCI_ALIAS_ADDRESS_REQ_REPLY_RET_T                alias_address_req_reply;
    HCI_ALIAS_ADDRESS_REQ_NEG_REPLY_RET_T            alias_address_req_neg_reply;
    HCI_IO_CAPABILITY_REQUEST_REPLY_RET_T            io_capability_request_reply_args;
    HCI_USER_CONFIRMATION_REQUEST_REPLY_RET_T        user_confirmation_request_reply_args;
    HCI_USER_CONFIRMATION_REQUEST_NEG_REPLY_RET_T    user_confirmation_request_neg_reply_args;
    HCI_USER_PASSKEY_REQUEST_REPLY_RET_T             user_passkey_request_reply_args;
    HCI_USER_PASSKEY_REQUEST_NEG_REPLY_RET_T         user_passkey_request_neg_reply_args;
    HCI_REMOTE_OOB_DATA_REQUEST_REPLY_RET_T          remote_oob_data_request_reply_args;
    HCI_REMOTE_OOB_DATA_REQUEST_NEG_REPLY_RET_T      remote_oob_data_request_neg_reply_args;
    HCI_IO_CAPABILITY_REQUEST_NEG_REPLY_RET_T        io_capability_request_neg_reply_args;
    HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY_RET_T remote_oob_extended_data_request_reply_args;
    HCI_TRUNCATED_PAGE_CANCEL_RET_T      truncated_page_cancel_reply_args;
    HCI_SET_CSB_RET_T                    set_csb_args;
    HCI_SET_CSB_RECEIVE_RET_T            set_csb_receive_args;

    /* Link Policy */
    HCI_ROLE_DISCOVERY_RET_T            role_discovery_args;
    HCI_READ_LINK_POLICY_SETTINGS_RET_T read_link_policy_settings_args;
    HCI_WRITE_LINK_POLICY_SETTINGS_RET_T write_link_policy_settings_args;
    HCI_READ_DEFAULT_LINK_POLICY_SETTINGS_RET_T read_default_lps_args;
    HCI_SNIFF_SUB_RATE_RET_T            sniff_sub_rate_args;

    /* Controller and Baseband */
    HCI_FLUSH_RET_T                     flush_args;
    HCI_READ_PIN_TYPE_RET_T             read_pin_type_args;
    HCI_READ_STORED_LINK_KEY_RET_T      read_stored_link_key_args;
    HCI_WRITE_STORED_LINK_KEY_RET_T     write_stored_link_key_args;
    HCI_DELETE_STORED_LINK_KEY_RET_T    delete_stored_link_key_args;
    HCI_READ_LOCAL_NAME_RET_T           read_local_name_args;
    HCI_READ_CONN_ACCEPT_TIMEOUT_RET_T  read_conn_accept_timeout_args;
    HCI_READ_PAGE_TIMEOUT_RET_T         read_page_timeout_args;
    HCI_READ_SCAN_ENABLE_RET_T          read_scan_enable_args;
    HCI_READ_PAGESCAN_ACTIVITY_RET_T    read_pagescan_activity_args;
    HCI_READ_INQUIRYSCAN_ACTIVITY_RET_T read_inqscan_activity_args;
    HCI_READ_AUTH_ENABLE_RET_T          read_auth_enable_args;
    HCI_READ_ENC_MODE_RET_T             read_enc_mode_args;
    HCI_READ_CLASS_OF_DEVICE_RET_T      read_class_device_args;
    HCI_READ_VOICE_SETTING_RET_T        read_voice_setting_args;
    HCI_READ_AUTO_FLUSH_TIMEOUT_RET_T   read_auto_flush_timeout_args;
    HCI_WRITE_AUTO_FLUSH_TIMEOUT_RET_T  write_auto_flush_timeout_args;
    HCI_READ_NUM_BCAST_RETXS_RET_T      read_num_bcast_retxs_args;
    HCI_READ_HOLD_MODE_ACTIVITY_RET_T   read_hold_mode_activity_args;
    HCI_READ_TX_POWER_LEVEL_RET_T       read_tx_power_level_args;
    HCI_READ_SCO_FLOW_CON_ENABLE_RET_T  read_sco_flow_con_enable_args;
    HCI_READ_LINK_SUPERV_TIMEOUT_RET_T  read_link_superv_timeout_args;
    HCI_WRITE_LINK_SUPERV_TIMEOUT_RET_T write_link_superv_timeout_args;
    HCI_READ_NUM_SUPPORTED_IAC_RET_T    read_num_supported_iac_args;
    HCI_READ_CURRENT_IAC_LAP_RET_T      read_current_iac_lap_args;
    HCI_READ_PAGESCAN_PERIOD_MODE_RET_T read_pagescan_period_mode_args;
    HCI_READ_PAGESCAN_MODE_RET_T        read_pagescan_mode_args;
    HCI_READ_AFH_CHANNEL_CLASS_M_RET_T  read_afh_channel_class_m_args;
    HCI_READ_INQUIRY_SCAN_TYPE_RET_T    read_inquiry_scan_type_args;
    HCI_READ_INQUIRY_MODE_RET_T         read_inquiry_mode_args;
    HCI_READ_PAGE_SCAN_TYPE_RET_T       read_page_scan_type_args;
    HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_RET_T read_extended_inquiry_response_data_args;
    HCI_READ_SIMPLE_PAIRING_MODE_RET_T          read_simple_pairing_mode_args;
    HCI_READ_LOCAL_OOB_DATA_RET_T               read_local_oob_data_args;
    HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_RET_T read_inquiry_response_tx_power_level_args;
    HCI_SEND_KEYPRESS_NOTIFICATION_RET_T        send_keypress_notification_args;
    HCI_READ_ENH_TX_POWER_LEVEL_RET_T   read_enhanced_tx_power_level_args;
    HCI_READ_LE_HOST_SUPPORT_RET_T      read_le_host_support_args;
    HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT_RET_T  read_secure_connections_host_support_args;
    HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_RET_T    read_authenticated_payload_timeout_args;
    HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_RET_T   write_authenticated_payload_timeout_args;
    HCI_READ_LOCAL_OOB_EXTENDED_DATA_RET_T          read_local_oob_extended_data_args;
    HCI_SET_RESERVED_LT_ADDR_RET_T      set_reserved_lt_addr_args;
    HCI_DELETE_RESERVED_LT_ADDR_RET_T   delete_reserved_lt_addr_args;
    HCI_SET_CSB_DATA_RET_T              set_csb_data_args;
    HCI_READ_SYNCHRONIZATION_TRAIN_PARAMS_RET_T  read_synchronization_train_params_args;
    HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_RET_T write_synchronization_train_params_args;

    /* Informational */
    HCI_READ_LOCAL_VER_INFO_RET_T       read_local_ver_info_args;
    HCI_READ_LOCAL_SUPP_COMMANDS_RET_T  read_local_supp_commands_args;
    HCI_READ_LOCAL_SUPP_FEATURES_RET_T  read_local_supp_features_args;
    HCI_READ_LOCAL_EXT_FEATURES_RET_T   read_local_ext_features_args;
    HCI_READ_BUFFER_SIZE_RET_T          read_buffer_size_args;
    HCI_READ_COUNTRY_CODE_RET_T         read_country_code_args;
    HCI_READ_BD_ADDR_RET_T              read_bd_addr_args;
    HCI_READ_LOCAL_SUPP_CODECS_RET_T    read_local_supp_codecs_args;

    /* Status */
    HCI_READ_FAILED_CONTACT_COUNT_RET_T read_failed_contact_count_args;
    HCI_RESET_FAILED_CONTACT_COUNT_RET_T reset_failed_contact_count_args;
    HCI_GET_LINK_QUALITY_RET_T          get_link_quality_args;
    HCI_READ_RSSI_RET_T                 read_rssi_args;
    HCI_READ_LOOPBACK_MODE_RET_T        read_loopback_mode_args;
    HCI_READ_AFH_CHANNEL_MAP_RET_T      read_afh_channel_map_args;
    HCI_READ_CLOCK_RET_T                read_clock_args;
    HCI_READ_ENCRYPTION_KEY_SIZE_RET_T  read_encryption_key_size_args;
    HCI_SET_TRIGGERED_CLOCK_CAPTURE_T   set_triggered_clock_capture_args;

    /* Test */
    HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE_RET_T  write_secure_connections_test_mode_args;

    /* ULP */
    HCI_ULP_READ_BUFFER_SIZE_RET_T                          ulp_read_buffer_size_args;
    HCI_ULP_READ_LOCAL_SUPPORTED_FEATURES_RET_T             ulp_read_local_supported_features_args;
    HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_RET_T         ulp_read_advertising_channel_tx_power_args;
    HCI_ULP_READ_CHANNEL_MAP_RET_T                          ulp_read_channel_map_args;
    HCI_ULP_READ_WHITE_LIST_SIZE_RET_T                      ulp_read_white_list_size_args;
    HCI_ULP_ENCRYPT_RET_T                                   ulp_encrypt_args;
    HCI_ULP_RAND_RET_T                                      ulp_rand_args;
    HCI_ULP_LONG_TERM_KEY_REQUEST_REPLY_RET_T               ulp_long_term_key_request_reply_args;
    HCI_ULP_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_RET_T      ulp_long_term_key_request_negative_reply_args;
    HCI_ULP_READ_SUPPORTED_STATES_RET_T                     ulp_read_supported_states_args;
    HCI_ULP_TEST_END_RET_T                                  ulp_test_end_args;
    HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY_RET_T ulp_remote_connection_parameter_request_reply_args;
    HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY_RET_T ulp_remote_connection_parameter_request_negative_reply_args;
    HCI_ULP_SET_DATA_LENGTH_RET_T                           ulp_set_data_length_args;
    HCI_ULP_READ_SUGGESTED_DEFAULT_DATA_LENGTH_RET_T        ulp_read_suggested_default_data_length_args;
    HCI_ULP_READ_MAXIMUM_DATA_LENGTH_RET_T                  ulp_read_maximum_data_length_args;
    HCI_ULP_READ_PHY_RET_T                                  ulp_read_phy_args;
    HCI_ULP_READ_RESOLVING_LIST_SIZE_RET_T                  ulp_read_resolving_list_size_args;
    HCI_VS_COMMAND_RET_T                                    vs_command_args;
} HCI_COMMAND_COMPLETE_ARGS_T;


 /*----------------------------------------------------------------------------*
 * PURPOSE
 *      Command Complete Event
 *      Use specific return structures for particular commands
 *
 *      The status argument, should in all rights be in the argument_ptr
 *      list but, as every command complete event returns a status we have
 *      moved it into the body of the structure to prevent us having malloc
 *      commands which only return a status.
 *----------------------------------------------------------------------------*/
typedef struct
{
    HCI_EVENT_COMMON_T              event;
    uint8_t                         num_hci_command_pkts;
    hci_op_code_t                   op_code;                /* op code of command that caused this event */
    hci_return_t                    status;
    HCI_COMMAND_COMPLETE_ARGS_T*    argument_ptr;           /* Pointer to arguments */
} HCI_EV_COMMAND_COMPLETE_T;

/*------------------------------------------------------------------------
 *
 *      UNION OF       PRIMITIVES
 *
 *-----------------------------------------------------------------------*/

typedef union hci_uprim_tag
{
    hci_op_code_t    op_code;           /* op code of command */
    hci_event_code_t event_code;        /* op code of event */

    HCI_COMMAND_COMMON_T                hci_cmd;
    HCI_COMMAND_GENERIC_T               hci_generic ;
    HCI_EVENT_COMMON_T                  hci_event;

    HCI_ULP_EVENT_COMMON_T              hci_ulp_event;

    HCI_WITH_CH_COMMON_T                hci_cmd_with_handle;
    HCI_ULP_WITH_CH_COMMON_T            hci_ulp_with_handle;
    HCI_WITH_CH_PT_COMMON_T             hci_cmd_with_handle_trnsprt;
    HCI_WITH_BD_COMMON_T                hci_cmd_with_bd_addr;
    HCI_EV_COMMON_EVENT_T               hci_ev_with_handle;

    HCI_ACL_DATA_COMMON_T               hci_acl_data;
    HCI_SCO_DATA_COMMON_T               hci_sco_data;

    /* Link Control */
    HCI_INQUIRY_T                                hci_inquiry;
    HCI_INQUIRY_CANCEL_T                         hci_inquiry_cancel;
    HCI_PERIODIC_INQUIRY_MODE_T                  hci_periodic_inquiry_mode;
    HCI_EXIT_PERIODIC_INQUIRY_MODE_T             hci_exit_periodic_inquiry_mode;
    HCI_CREATE_CONNECTION_T                      hci_create_connection;
    HCI_DISCONNECT_T                             hci_disconnect;
    HCI_ADD_SCO_CONNECTION_T                     hci_add_sco_connection;
    HCI_CREATE_CONNECTION_CANCEL_T               hci_create_connection_cancel;
    HCI_ACCEPT_CONNECTION_REQ_T                  hci_accept_connection;
    HCI_REJECT_CONNECTION_REQ_T                  hci_reject_connection;
    HCI_LINK_KEY_REQ_REPLY_T                     hci_link_key_req_reply;
    HCI_LINK_KEY_REQ_NEG_REPLY_T                 hci_link_key_reqneg_reply;
    HCI_PIN_CODE_REQ_REPLY_T                     hci_pin_code_req_reply;
    HCI_PIN_CODE_REQ_NEG_REPLY_T                 hci_pin_code_reqneg_reply;
    HCI_CHANGE_CONN_PKT_TYPE_T                   hci_change_conn_pkt_type;
    HCI_AUTH_REQ_T                               hci_auth_req;
    HCI_SET_CONN_ENCRYPTION_T                    hci_set_conn_encryption;
    HCI_CHANGE_CONN_LINK_KEY_T                   hci_change_conn_link_key;
    HCI_MASTER_LINK_KEY_T                        hci_master_link_key;
    HCI_REMOTE_NAME_REQ_T                        hci_remote_name_req;
    HCI_REMOTE_NAME_REQ_CANCEL_T                 hci_remote_name_req_cancel;
    HCI_READ_REMOTE_SUPP_FEATURES_T              hci_read_rem_supp_features;
    HCI_READ_REMOTE_EXT_FEATURES_T               hci_read_rem_ext_features;
    HCI_READ_REMOTE_VER_INFO_T                   hci_read_rem_ver_info;
    HCI_READ_CLOCK_OFFSET_T                      hci_read_clock_offset;
    HCI_READ_LMP_HANDLE_T                        hci_read_lmp_handle;
    HCI_EXCHANGE_FIXED_INFO_T                    hci_exchange_fixed_info;
    HCI_EXCHANGE_ALIAS_INFO_T                    hci_exchange_alias_info;
    HCI_PRIVATE_PAIRING_REQ_REPLY_T              hci_private_pairing_req_reply;
    HCI_PRIVATE_PAIRING_REQ_NEG_REPLY_T          hci_private_pairing_req_neg_reply;
    HCI_GENERATED_ALIAS_T                        hci_generated_alias;
    HCI_ALIAS_ADDRESS_REQ_REPLY_T                hci_alias_address_req_reply;
    HCI_ALIAS_ADDRESS_REQ_NEG_REPLY_T            hci_alias_address_req_neg_reply;
    HCI_SETUP_SYNCHRONOUS_CONN_T                 hci_setup_sync_connection;
    HCI_ACCEPT_SYNCHRONOUS_CONN_REQ_T            hci_accept_sync_connection_req;
    HCI_REJECT_SYNCHRONOUS_CONN_REQ_T            hci_reject_sync_connection_req;
    HCI_IO_CAPABILITY_REQUEST_REPLY_T            hci_io_capability_request_reply;
    HCI_USER_CONFIRMATION_REQUEST_REPLY_T        hci_user_confirmation_request_reply;
    HCI_USER_CONFIRMATION_REQUEST_NEG_REPLY_T    hci_user_confirmation_request_neg_reply;
    HCI_USER_PASSKEY_REQUEST_REPLY_T             hci_user_passkey_request_reply;
    HCI_USER_PASSKEY_REQUEST_NEG_REPLY_T         hci_user_passkey_request_neg_reply;
    HCI_REMOTE_OOB_DATA_REQUEST_REPLY_T          hci_remote_oob_data_request_reply;
    HCI_REMOTE_OOB_DATA_REQUEST_NEG_REPLY_T      hci_remote_oob_data_request_neg_reply;
    HCI_IO_CAPABILITY_REQUEST_NEG_REPLY_T        hci_io_capability_request_neg_reply;
    HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY_T hci_remote_oob_extended_data_request_reply;
    HCI_TRUNCATED_PAGE_T                         hci_truncated_page;
    HCI_TRUNCATED_PAGE_CANCEL_T                  hci_truncated_page_cancel;
    HCI_START_SYNCHRONIZATION_TRAIN_T            hci_start_synchronization;
    HCI_SET_CSB_T                                hci_set_csb;
    HCI_RECEIVE_SYNCHRONIZATION_TRAIN_T          hci_receive_synchronization_train;
    HCI_SET_CSB_RECEIVE_T                        hci_set_csb_receive;
    HCI_ENHANCED_SETUP_SYNC_CONN_T               hci_enhanced_setup_sync_connection;
    HCI_ENHANCED_ACCEPT_SYNC_CONN_REQ_T          hci_enhanced_accept_sync_connection_req;

    /* Link Policy */
    HCI_HOLD_MODE_T                     hci_hold_mode;
    HCI_SNIFF_MODE_T                    hci_sniff_mode;
    HCI_EXIT_SNIFF_MODE_T               hci_exit_sniff_mode;
    HCI_QOS_SETUP_T                     hci_qos_setup;
    HCI_ROLE_DISCOVERY_T                hci_role_discovery;
    HCI_SWITCH_ROLE_T                   hci_switch_role;
    HCI_READ_LINK_POLICY_SETTINGS_T     hci_read_link_policy_settings;
    HCI_WRITE_LINK_POLICY_SETTINGS_T    hci_write_link_policy_settings;
    HCI_READ_DEFAULT_LINK_POLICY_SETTINGS_T     hci_read_default_lps;
    HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_T    hci_write_default_lps;
    HCI_FLOW_SPEC_T                     hci_flow_spec;
    HCI_SNIFF_SUB_RATE_T                hci_sniff_sub_rate;

    /* Controller and Baseband */
    HCI_SET_EVENT_MASK_T                hci_set_event_mask;
    HCI_RESET_T                         hci_reset;
    HCI_SET_EVENT_FILTER_T              hci_set_event_filter;
    HCI_FLUSH_T                         hci_flush;
    HCI_READ_PIN_TYPE_T                 hci_read_pin_type;
    HCI_WRITE_PIN_TYPE_T                hci_write_pin_type;
    HCI_CREATE_NEW_UNIT_KEY_T           hci_create_new_unit_key;
    HCI_READ_STORED_LINK_KEY_T          hci_read_stored_link_key;
    HCI_WRITE_STORED_LINK_KEY_T         hci_write_stored_link_key;
    HCI_DELETE_STORED_LINK_KEY_T        hci_delete_stored_link_key;
    HCI_CHANGE_LOCAL_NAME_T             hci_change_local_name;
    HCI_READ_LOCAL_NAME_T               hci_read_local_name;
    HCI_READ_CONN_ACCEPT_TIMEOUT_T      hci_read_conn_accept_timeout;
    HCI_WRITE_CONN_ACCEPT_TIMEOUT_T     hci_write_conn_accept_timeout;
    HCI_READ_PAGE_TIMEOUT_T             hci_read_page_timeout;
    HCI_WRITE_PAGE_TIMEOUT_T            hci_write_page_timeout;
    HCI_READ_SCAN_ENABLE_T              hci_scan_enable;
    HCI_WRITE_SCAN_ENABLE_T             hci_write_scan_enable;
    HCI_READ_PAGESCAN_ACTIVITY_T        hci_read_pagescan_activity;
    HCI_WRITE_PAGESCAN_ACTIVITY_T       hci_write_pagescan_activity;
    HCI_READ_INQUIRYSCAN_ACTIVITY_T     hci_read_inqscan_activity;
    HCI_WRITE_INQUIRYSCAN_ACTIVITY_T    hci_write_inqscan_activity;
    HCI_READ_AUTH_ENABLE_T              hci_read_auth_enable;
    HCI_WRITE_AUTH_ENABLE_T             hci_write_auth_enable;
    HCI_READ_ENC_MODE_T                 hci_read_enc_mode;
    HCI_WRITE_ENC_MODE_T                hci_write_enc_mode;
    HCI_READ_CLASS_OF_DEVICE_T          hci_read_class_of_device;
    HCI_WRITE_CLASS_OF_DEVICE_T         hci_write_class_of_device;
    HCI_READ_VOICE_SETTING_T            hci_read_voice_settings;
    HCI_WRITE_VOICE_SETTING_T           hci_write_voice_settings;
    HCI_READ_AUTO_FLUSH_TIMEOUT_T       hci_read_auto_flush_timeout;
    HCI_WRITE_AUTO_FLUSH_TIMEOUT_T      hci_write_auto_flush_timeout;
    HCI_READ_NUM_BCAST_RETXS_T          hci_read_num_bcast_retxs;
    HCI_WRITE_NUM_BCAST_RETXS_T         hci_write_num_bcast_retxs;
    HCI_READ_HOLD_MODE_ACTIVITY_T       hci_read_hold_mode_activity;
    HCI_WRITE_HOLD_MODE_ACTIVITY_T      hci_write_hold_mode_activity;
    HCI_READ_TX_POWER_LEVEL_T           hci_read_tx_power_level;
    HCI_READ_SCO_FLOW_CON_ENABLE_T      hci_read_sco_flow_con_enable;
    HCI_WRITE_SCO_FLOW_CON_ENABLE_T     hci_write_sco_flow_con_enable;
    HCI_SET_HCTOHOST_FLOW_CONTROL_T     hci_set_hctohost_flow_control;
    HCI_HOST_BUFFER_SIZE_T              hci_host_buffer_size;
    HCI_HOST_NUM_COMPLETED_PACKETS_T    hci_host_num_coml_pkts;
    HCI_READ_LINK_SUPERV_TIMEOUT_T      hci_read_link_superv_timeout;
    HCI_WRITE_LINK_SUPERV_TIMEOUT_T     hci_write_link_superv_timeout;
    HCI_READ_NUM_SUPPORTED_IAC_T        hci_read_num_supported_iac;
    HCI_READ_CURRENT_IAC_LAP_T          hci_read_curr_iac_lap;
    HCI_WRITE_CURRENT_IAC_LAP_T         hci_write_curr_iac_lap;
    HCI_READ_PAGESCAN_PERIOD_MODE_T     hci_read_pagescan_period_mode;
    HCI_WRITE_PAGESCAN_PERIOD_MODE_T    hci_write_pagescan_period_mode;
    HCI_READ_PAGESCAN_MODE_T            hci_read_pagescan_mode;
    HCI_WRITE_PAGESCAN_MODE_T           hci_write_pagescan_mode;
    HCI_SET_AFH_CHANNEL_CLASS_T         hci_set_afh_channel_class;
    HCI_READ_INQUIRY_SCAN_TYPE_T        hci_read_inquiry_scan_type;
    HCI_WRITE_INQUIRY_SCAN_TYPE_T       hci_write_inquiry_scan_type;
    HCI_READ_INQUIRY_MODE_T             hci_read_inquiry_mode;
    HCI_WRITE_INQUIRY_MODE_T            hci_write_inquiry_mode;
    HCI_READ_PAGE_SCAN_TYPE_T           hci_read_page_scan_type;
    HCI_WRITE_PAGE_SCAN_TYPE_T          hci_write_page_scan_type;
    HCI_READ_AFH_CHANNEL_CLASS_M_T      hci_read_afh_channel_class_m;
    HCI_WRITE_AFH_CHANNEL_CLASS_M_T     hci_write_afh_channel_class_m;
    HCI_READ_ANON_MODE_T                hci_read_anon_mode;
    HCI_WRITE_ANON_MODE_T               hci_write_anon_mode;
    HCI_READ_ALIAS_AUTH_ENABLE_T        hci_read_alias_auth_enable;
    HCI_WRITE_ALIAS_AUTH_ENABLE_T       hci_write_alias_auth_enable;
    HCI_READ_ANON_ADDR_CHANGE_PARAMS_T  hci_read_anon_addr_change_params;
    HCI_WRITE_ANON_ADDR_CHANGE_PARAMS_T hci_write_anon_addr_change_params;
    HCI_RESET_FIXED_ADDRESS_ATTEMPTS_COUNTER_T
            hci_reset_fixed_address_attempts_counter;
    HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_T
            hci_read_extended_inquiry_response_data;
    HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA_T
            hci_write_extended_inquiry_response_data;
    HCI_REFRESH_ENCRYPTION_KEY_T                hci_refresh_encryption_key;
    HCI_WRITE_SIMPLE_PAIRING_MODE_T             hci_write_simple_pairing_mode;
    HCI_READ_SIMPLE_PAIRING_MODE_T              hci_read_simple_pairing_mode;
    HCI_READ_LOCAL_OOB_DATA_T                   hci_read_local_oob_data;
    HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_T     hci_read_inquiry_response_tx_power_level;
    HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL_T    hci_write_inquiry_transmit_power_level;
    HCI_SEND_KEYPRESS_NOTIFICATION_T            hci_send_keypress_notification;
    HCI_ENHANCED_FLUSH_T                    hci_enhanced_flush;
    HCI_SET_EVENT_MASK_PAGE_2_T         hci_set_event_mask_page_2;
    HCI_READ_ENH_TX_POWER_LEVEL_T       hci_read_enh_tx_power_level;
    HCI_READ_LE_HOST_SUPPORT_T          hci_read_le_host_support;
    HCI_WRITE_LE_HOST_SUPPORT_T         hci_write_le_host_support;
    HCI_READ_SECURE_CONNECTIONS_HOST_SUPPORT_T    hci_read_secure_connections_host_support;
    HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT_T   hci_write_secure_connections_host_support;
    HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_T      hci_read_authenticated_payload_timeout;
    HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_T     hci_write_authenticated_payload_timeout;
    HCI_READ_LOCAL_OOB_EXTENDED_DATA_T            hci_read_local_oob_extended_data;
    HCI_SET_RESERVED_LT_ADDR_T          hci_set_reserved_lt_addr;
    HCI_DELETE_RESERVED_LT_ADDR_T       hci_delete_reserved_lt_addr;
    HCI_SET_CSB_DATA_T                  hci_set_csb_data;
    HCI_READ_SYNCHRONIZATION_TRAIN_PARAMS_T  hci_read_synchronization_train_params;
    HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_T hci_write_synchronization_train_params;

    /* Information */
    HCI_READ_LOCAL_VER_INFO_T           hci_read_local_ver_info;
    HCI_READ_LOCAL_SUPP_COMMANDS_T      hci_read_local_supp_commands;
    HCI_READ_LOCAL_SUPP_FEATURES_T      hci_read_local_supp_features;
    HCI_READ_LOCAL_EXT_FEATURES_T       hci_read_local_ext_features;
    HCI_READ_BUFFER_SIZE_T              hci_read_buffer_size;
    HCI_READ_COUNTRY_CODE_T             hci_read_country_code;
    HCI_READ_BD_ADDR_T                  hci_read_bd_addr;
    HCI_READ_LOCAL_SUPP_CODECS_T        hci_read_local_supp_codecs;

    /* Status */
    HCI_READ_FAILED_CONTACT_COUNT_T     hci_read_failed_contact_count;
    HCI_RESET_FAILED_CONTACT_COUNT_T    hci_reset_failed_contact_count;
    HCI_GET_LINK_QUALITY_T              hci_get_link_quality;
    HCI_READ_RSSI_T                     hci_read_rssi;
    HCI_READ_AFH_CHANNEL_MAP_T          hci_read_afh_channel_map;
    HCI_READ_CLOCK_T                    hci_read_clock;
    HCI_READ_ENCRYPTION_KEY_SIZE_T      hci_read_encryption_key_size;
    HCI_SET_TRIGGERED_CLOCK_CAPTURE_T   hci_set_triggered_clock_capture;

    /* Testing */
    HCI_READ_LOOPBACK_MODE_T            hci_read_loopback_mode;
    HCI_WRITE_LOOPBACK_MODE_T           hci_write_loopback_mode;
    HCI_ENABLE_DUT_MODE_T               hci_enable_dut_mode;
    HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE_T hci_write_simple_pairing_debug_mode;
    HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE_T hci_write_secure_connections_test_mode;

    /* ULP */
    HCI_ULP_SET_EVENT_MASK_T                            hci_ulp_set_event_mask;
    HCI_ULP_READ_BUFFER_SIZE_T                          hci_ulp_read_buffer_size;
    HCI_ULP_READ_LOCAL_SUPPORTED_FEATURES_T             hci_ulp_read_local_supported_features;
    HCI_ULP_SET_RANDOM_ADDRESS_T                        hci_ulp_set_random_address;
    HCI_ULP_SET_ADVERTISING_PARAMETERS_T                hci_ulp_set_advertising_parameters;
    HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_T         hci_ulp_read_advertising_channel_tx_power;
    HCI_ULP_SET_ADVERTISING_DATA_T                      hci_ulp_set_advertising_data;
    HCI_ULP_SET_SCAN_RESPONSE_DATA_T                    hci_ulp_set_scan_response_data;
    HCI_ULP_SET_ADVERTISE_ENABLE_T                      hci_ulp_set_advertise_enable;
    HCI_ULP_SET_SCAN_PARAMETERS_T                       hci_ulp_set_scan_parameters;
    HCI_ULP_SET_SCAN_ENABLE_T                           hci_ulp_set_scan_enable;
    HCI_ULP_CREATE_CONNECTION_T                         hci_ulp_create_connection;
    HCI_ULP_CREATE_CONNECTION_CANCEL_T                  hci_ulp_create_connection_cancel;
    HCI_ULP_READ_WHITE_LIST_SIZE_T                      hci_ulp_read_white_list_size;
    HCI_ULP_CLEAR_WHITE_LIST_T                          hci_ulp_clear_white_list;
    HCI_ULP_ADD_DEVICE_TO_WHITE_LIST_T                  hci_ulp_add_device_to_white_list;
    HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST_T             hci_ulp_remove_device_from_white_list;
    HCI_ULP_CONNECTION_UPDATE_T                         hci_ulp_connection_update;
    HCI_ULP_SET_HOST_CHANNEL_CLASSIFICATION_T           hci_ulp_set_host_channel_classification;
    HCI_ULP_READ_CHANNEL_MAP_T                          hci_ulp_read_channel_map;
    HCI_ULP_READ_REMOTE_USED_FEATURES_T                 hci_ulp_read_remote_used_features;
    HCI_ULP_ENCRYPT_T                                   hci_ulp_encrypt;
    HCI_ULP_RAND_T                                      hci_ulp_rand;
    HCI_ULP_START_ENCRYPTION_T                          hci_ulp_start_encryption;
    HCI_ULP_LONG_TERM_KEY_REQUEST_REPLY_T               hci_ulp_long_term_key_request_reply;
    HCI_ULP_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_T      hci_ulp_long_term_key_request_negative_reply;
    HCI_ULP_READ_SUPPORTED_STATES_T                     hci_ulp_read_supported_states;
    HCI_ULP_RECEIVER_TEST_T                             hci_ulp_receiver_test;
    HCI_ULP_TRANSMITTER_TEST_T                          hci_ulp_transmitter_test;
    HCI_ULP_TEST_END_T                                  hci_ulp_test_end;
    HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY_T hci_ulp_remote_connection_parameter_request_reply;
    HCI_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY_T hci_ulp_remote_connection_parameter_request_negative_reply;
    HCI_ULP_SET_DATA_LENGTH_T                           hci_ulp_set_data_length;
    HCI_ULP_READ_SUGGESTED_DEFAULT_DATA_LENGTH_T        hci_ulp_read_suggested_default_data_length;
    HCI_ULP_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH_T       hci_ulp_write_suggested_default_data_length;
    HCI_ULP_READ_MAXIMUM_DATA_LENGTH_T                  hci_ulp_read_maximum_data_length;
    HCI_ULP_READ_LOCAL_P256_PUBLIC_KEY_T                hci_ulp_read_local_p256_public_key;
    HCI_ULP_GENERATE_DHKEY_T                            hci_ulp_generate_dhkey;
    HCI_ULP_ADD_DEVICE_TO_RESOLVING_LIST_T              hci_ulp_add_device_to_resolving_list;
    HCI_ULP_REMOVE_DEVICE_FROM_RESOLVING_LIST_T         hci_ulp_remove_device_from_resolving_list;
    HCI_ULP_CLEAR_RESOLVING_LIST_T                      hci_ulp_clear_resolving_list;
    HCI_ULP_READ_RESOLVING_LIST_SIZE_T                  hci_ulp_read_resolving_list_size;
    HCI_ULP_READ_PEER_RESOLVABLE_ADDRESS_T              hci_ulp_read_peer_resolvable_address;
    HCI_ULP_READ_LOCAL_RESOLVABLE_ADDRESS_T             hci_ulp_read_local_resolvable_address;
    HCI_ULP_SET_ADDRESS_RESOLUTION_ENABLE_T             hci_ulp_set_address_resolution_enable;
    HCI_ULP_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT_T    hci_ulp_set_resolvable_private_address_timeout;
    HCI_ULP_SET_PRIVACY_MODE_T                          hci_ulp_set_privacy_mode;
    HCI_ULP_READ_PHY_T                                  hci_ulp_read_phy;
    HCI_ULP_SET_DEFAULT_PHY_T                           hci_ulp_default_phy;
    HCI_ULP_SET_PHY_T                                   hci_ulp_set_phy;
    HCI_ULP_ENHANCED_RECEIVER_TEST_T                    hci_ulp_enhanced_receiver_test;
    HCI_ULP_ENHANCED_TRANSMITTER_TEST_T                 hci_ulp_enhanced_transmitter_test;

    HCI_MNFR_EXTENSION_T                hci_mnfr_extension;
    HCI_VS_COMMAND_T                    hci_vs_command;

    /* Events */
    HCI_EV_INQUIRY_COMPLETE_T               hci_inquiry_complete_event;
    HCI_EV_INQUIRY_RESULT_T                 hci_inquiry_result_event;
    HCI_EV_CONN_COMPLETE_T                  hci_connection_complete_event;
    HCI_EV_CONN_REQUEST_T                   hci_connection_request_event;
    HCI_EV_DISCONNECT_COMPLETE_T            hci_disconnect_complete_event;
    HCI_EV_AUTH_COMPLETE_T                  hci_auth_complete_event;
    HCI_EV_REMOTE_NAME_REQ_COMPLETE_T       hci_remote_name_request_complete;
    HCI_EV_ENCRYPTION_CHANGE_T              hci_enc_change_event;
    HCI_EV_CHANGE_CONN_LINK_KEY_COMPLETE_T  hci_change_conn_linkkey_coml_event;
    HCI_EV_MASTER_LINK_KEY_COMPLETE_T       hci_master_linkkey_compl_event;
    HCI_EV_READ_REM_SUPP_FEATURES_COMPLETE_T  hci_read_rem_supp_features_event;
    HCI_EV_READ_REMOTE_VER_INFO_COMPLETE_T  hci_read_rem_ver_info_compl_event;
    HCI_EV_QOS_SETUP_COMPLETE_T             hci_qos_setup_compl_event;
    HCI_EV_COMMAND_COMPLETE_T               hci_command_complete_event;
    HCI_EV_COMMAND_STATUS_T                 hci_command_status_event;
    HCI_EV_HARDWARE_ERROR_T                 hci_hardware_error_event;
    HCI_EV_FLUSH_OCCURRED_T                 hci_flush_occurred_event;
    HCI_EV_ROLE_CHANGE_T                    hci_role_change_event;
    HCI_EV_NUMBER_COMPLETED_PKTS_T          hci_num_compl_pkts_event;
    HCI_EV_MODE_CHANGE_T                    hci_mode_change_event;
    HCI_EV_RETURN_LINK_KEYS_T               hci_ret_linkkeys_event;
    HCI_EV_PIN_CODE_REQ_T                   hci_pin_code_req_event;
    HCI_EV_LINK_KEY_REQ_T                   hci_link_key_req_event;
    HCI_EV_LINK_KEY_NOTIFICATION_T          hci_link_key_notif_event;
    HCI_EV_LOOPBACK_COMMAND_T               hci_loopback_command_event;
    HCI_EV_DATA_BUFFER_OVERFLOW_T           hci_data_buff_overflow_event;
    HCI_EV_MAX_SLOTS_CHANGE_T               hci_max_slots_change_event;
    HCI_EV_READ_CLOCK_OFFSET_COMPLETE_T     hci_read_clock_offset_compl_event;
    HCI_EV_CONN_PACKET_TYPE_CHANGED_T       hci_conn_pkt_type_chge_event;
    HCI_EV_QOS_VIOLATION_T                  hci_qos_violation_event;
    HCI_EV_PAGE_SCAN_MODE_CHANGE_T          hci_pagescan_mode_chge_event;
    HCI_EV_PAGE_SCAN_REP_MODE_CHANGE_T      hci_pagescan_repmode_chge_event;
    HCI_EV_FLOW_SPEC_COMPLETE_T             hci_flow_spec_complete_event;
    HCI_EV_INQUIRY_RESULT_WITH_RSSI_T       hci_inquiry_result_with_rssi_event;
    HCI_EV_READ_REM_EXT_FEATURES_COMPLETE_T hci_read_rem_ext_features_event;
    HCI_EV_FIXED_ADDRESS_T                  hci_fixed_address;
    HCI_EV_ALIAS_ADDRESS_T                  hci_alias_address;
    HCI_EV_GENERATE_ALIAS_REQ_T             hci_generate_alias_req;
    HCI_EV_ACTIVE_ADDRESS_T                 hci_active_address;
    HCI_EV_ALLOW_PRIVATE_PAIRING_T          hci_allow_private_pairing;
    HCI_EV_ALIAS_ADDRESS_REQ_T              hci_alias_address_req;
    HCI_EV_ALIAS_NOT_RECOGNISED_T           hci_alias_not_recognized;
    HCI_EV_FIXED_ADDRESS_ATTEMPT_T          hci_fixed_address_attempt;
    HCI_EV_SYNC_CONN_COMPLETE_T             hci_sync_connection_complete_event;
    HCI_EV_SYNC_CONN_CHANGED_T              hci_sync_connection_changed_event;
    HCI_EV_SNIFF_SUB_RATE_T                 hci_sniff_sub_rate_event;
    HCI_EV_EXTENDED_INQUIRY_RESULT_T        hci_extended_inquiry_result_event;
    HCI_EV_ENCRYPTION_KEY_REFRESH_COMPLETE_T hci_encryption_key_refresh_complete_event;
    HCI_EV_IO_CAPABILITY_REQUEST_T          hci_io_capability_request_event;
    HCI_EV_IO_CAPABILITY_RESPONSE_T         hci_io_capability_response_event;
    HCI_EV_USER_CONFIRMATION_REQUEST_T      hci_user_confirmation_request_event;
    HCI_EV_USER_PASSKEY_REQUEST_T           hci_user_passkey_request_event;
    HCI_EV_REMOTE_OOB_DATA_REQUEST_T        hci_remote_oob_data_request_event;
    HCI_EV_SIMPLE_PAIRING_COMPLETE_T        hci_simple_pairing_complete_event;
    HCI_EV_LST_CHANGE_T                     hci_lst_change_event;
    HCI_EV_ENHANCED_FLUSH_COMPLETE_T        hci_enhanced_flush_complete_event;
    HCI_EV_USER_PASSKEY_NOTIFICATION_T      hci_user_passkey_notification_event;
    HCI_EV_KEYPRESS_NOTIFICATION_T          hci_keypress_notification_event;
    HCI_EV_REM_HOST_SUPPORTED_FEATURES_T    hci_rem_host_supported_features_event;
    HCI_EV_TRIGGERED_CLOCK_CAPTURE_T        hci_triggered_clock_capture_event;
    HCI_EV_SYNCHRONIZATION_TRAIN_COMPLETE_T hci_synchronization_train_complete;
    HCI_EV_SYNCHRONIZATION_TRAIN_RECEIVED_T hci_synchronization_train_received;
    HCI_EV_CSB_RECEIVE_T                    hci_csb_receive;
    HCI_EV_CSB_TIMEOUT_T                    hci_csb_timeout;
    HCI_EV_TRUNCATED_PAGE_COMPLETE_T        hci_truncated_page_complete;
    HCI_EV_SLAVE_PAGE_RESPONSE_TIMEOUT_T    hci_slave_page_response_timeout;
    HCI_EV_CSB_CHANNEL_MAP_CHANGE_T         hci_csb_channel_map_change;
    HCI_EV_INQUIRY_RESPONSE_NOTIFICATION_T  hci_inquiry_response_notification;

    HCI_EV_ULP_CONNECTION_COMPLETE_T                 hci_ulp_connection_complete;
    HCI_EV_ULP_ADVERTISING_REPORT_T                  hci_ulp_advertising_report;
    HCI_EV_ULP_CONNECTION_UPDATE_COMPLETE_T          hci_ulp_connection_update_complete;
    HCI_EV_ULP_READ_REMOTE_USED_FEATURES_COMPLETE_T  hci_ulp_read_remote_used_features_complete;
    HCI_EV_ULP_LONG_TERM_KEY_REQUEST_T               hci_ulp_long_term_key_request;
    HCI_EV_ULP_REMOTE_CONNECTION_PARAMETER_REQUEST_T hci_ulp_remote_connection_parameter_request;
    HCI_EV_ULP_READ_LOCAL_P256_PUB_KEY_COMPLETE_T    hci_ulp_read_local_p256_public_key_complete;
    HCI_EV_ULP_GENERATE_DHKEY_COMPLETE_T             hci_ulp_generate_dhkey_complete;
    HCI_EV_ULP_ENHANCED_CONNECTION_COMPLETE_T        hci_ulp_enhanced_connection_complete;
    HCI_EV_ULP_DIRECT_ADVERTISING_REPORT_T           hci_ulp_direct_advertising_report;
    HCI_EV_ULP_PHY_UPDATE_COMPLETE_T                 hci_ulp_phy_update_complete;
    HCI_EV_ULP_DATA_LENGTH_CHANGE_T                  hci_ulp_data_length_change;
    HCI_EV_ULP_CHANNEL_SELECTION_ALGORITHM_T         hci_ulp_channel_selection_algorithm;

    HCI_EV_MNFR_EXTENSION_T                        hci_mnfr_extension_event;
    HCI_EV_VS_T                                    hci_vs_event;
    HCI_EV_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_T hci_authenticated_payload_timeout_expired_event;
} HCI_UPRIM_T;

/*============================================================================*
                               Public Data
 *===========================================================================*/
/* None */

/*============================================================================*
                             Public Functions
 *===========================================================================*/
/* None */

#ifdef __cplusplus
}
#endif

#endif /* ndef _HCI_H */
