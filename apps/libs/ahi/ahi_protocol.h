/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_protocol.h

DESCRIPTION
    Definition of the AHI protocol format.

*/

#ifndef AHI_PROTOCOL_H_
#define AHI_PROTOCOL_H_

#include <csrtypes.h>


/*  AHI message header (big-endian):
    0 bytes  1        2        3        4          5    len+4
    +--------+--------+--------+--------+--------+--/ /---+
    | OPCODE | FLAGS  |     LENGTH      |  PAYLOAD  ...   |
    +--------+--------+--------+--------+--------+--/ /---+
*/

/* AHI msg header size in bytes */
#define AHI_MSG_HEADER_SIZE (4)

/* Calculate the size of an AHI msg by reading the message itself. */
#define AHI_MSG_SIZE(msg) (AHI_MSG_HEADER_SIZE + *(uint16 *)((uint8 *)msg + (AHI_MSG_INDEX_LENGTH >> 1)))

/* Maximum size in bytes of AHI msg header + data */
#define AHI_MSG_SIZE_MAX 64

/* Maximum size in bytes of the payload of an AHI message. */
#define AHI_MSG_PAYLOAD_SIZE_MAX (AHI_MSG_SIZE_MAX - AHI_MSG_HEADER_SIZE)

/* Byte index of fields of an AHI msg header. */
#define AHI_MSG_INDEX_OPCODE 0
#define AHI_MSG_INDEX_FLAGS 1
#define AHI_MSG_INDEX_LENGTH 2
#define AHI_MSG_INDEX_PAYLOAD 4

/* AHI opcodes. Max value is 0xFF.
   The opcode values are defined in CS-341524-DD. */

/* Sent by Host.

AHI_VERSION_REQ
AHI_MODE_GET_REQ
AHI_MODE_SET_REQ
AHI_CONFIG_FILE_GET_REQ
AHI_CONFIG_GET_REQ
AHI_CONFIG_SET_REQ
AHI_EVENT_INJECT_IND

*/

/* Sent by Device.

AHI_VERSION_CFM
AHI_MODE_GET_CFM
AHI_MODE_SET_CFM
AHI_CONFIG_FILE_GET_CFM
AHI_CONFIG_GET_CFM
AHI_CONFIG_SET_CFM
AHI_EVENT_REPORT_IND
AHI_STATE_MACHINE_STATE_REPORT_IND

*/

#define AHI_MSG_BASE 0x0

#define AHI_VERSION_REQ 0x1
#define AHI_VERSION_CFM 0x2

#define AHI_EVENT_INJECT_IND 0x3

#define AHI_CONFIG_GET_REQ 0x5
#define AHI_CONFIG_GET_CFM 0x6

#define AHI_CONFIG_SET_REQ 0x7
#define AHI_CONFIG_SET_CFM 0x8

#define AHI_MODE_GET_REQ 0x9
#define AHI_MODE_GET_CFM 0xA

#define AHI_MODE_SET_REQ 0xB
#define AHI_MODE_SET_CFM 0xC

#define AHI_CONFIG_FILE_GET_REQ 0xD
#define AHI_CONFIG_FILE_GET_CFM 0xE

#define AHI_EVENT_REPORT_IND 0xF

#define AHI_STATE_MACHINE_STATE_REPORT_IND 0x11

#define AHI_CONFIG_FILE_SIGNATURE_REQ 0x13
#define AHI_CONFIG_FILE_SIGNATURE_CFM 0x14

#define AHI_CONFIG_FILE_GET_DATA_IND 0x15

#define AHI_CONNECT_REQ 0x17
#define AHI_CONNECT_CFM 0x18

#define AHI_DISCONNECT_REQ 0x19
#define AHI_DISCONNECT_CFM 0x1A

#define AHI_DISABLE_TRANSPORT_REQ 0x1B
#define AHI_DISABLE_TRANSPORT_CFM 0x1C

#define AHI_PRODUCT_ID_GET_REQ 0x1D
#define AHI_PRODUCT_ID_GET_CFM 0x1E

#define AHI_APP_BUILD_ID_GET_REQ 0x1F
#define AHI_APP_BUILD_ID_GET_CFM 0x20

#define AHI_REPORT_MTU_IND 0x21
#define AHI_REPORT_CONN_INT_IND 0x22

#define AHI_MSG_MAX 0xFF

/*! 
    @brief Values for the flag field in a AHI message.
*/
typedef enum
{
    AHI_FLAG_NONE = 0x0,
    AHI_FLAG_MORE_DATA = 0x1  /**< There is at least one more msg in the sequence after this one */
} ahi_message_flag_t;

/*!
    @brief Status/error codes used within AHI messages.
*/
typedef enum
{
    AHI_STATUS_SUCCESS = 0x0,
    AHI_STATUS_REBOOT_REQUIRED = 0x1,

    AHI_STATUS_NOT_FOUND = 0x80,
    AHI_STATUS_REJECTED = 0x81,
    AHI_STATUS_BAD_PARAMETER = 0x82,
    AHI_STATUS_ALREADY_IN_USE = 0x83,
    AHI_STATUS_TRUNCATED = 0x84,
    AHI_STATUS_NO_MEMORY = 0x85,
    AHI_STATUS_NOT_CONNECTED = 0x86,

    AHI_STATUS_LAST = 0xFF
} ahi_message_status_t;

/*!
    @brief Application mode flags
*/
typedef enum
{
    AHI_MODE_UNDEFINED = 0x0,
    AHI_MODE_NORMAL = 0x1,
    AHI_MODE_CONFIG = 0x2,
    AHI_MODE_TEST = 0x4
} ahi_app_mode_flags_t;

/*!
    @brief The method used to generate the config signature.
*/
typedef enum
{
    AHI_SIGNATURE_UNDEFINED = 0x0,
    AHI_SIGNATURE_MD5 = 0x1
} ahi_signature_type_t;


/*! Ahi Message protocol major version. */
#define AHI_VERSION_MAJOR 0

/*! Ahi Message protocol minor version. */
#define AHI_VERSION_MINOR 4


/******************************************************************************
    Message payload contents

    If a message is not detailed here then it does not contain
    any payload data. It consists only of the header with the opcode set
    accordingly and a length of 0.

    All multi-byte fields are stored as big-endian order.
*/

/*  AHI_CONNECT_CFM
    0 bytes  4        5        6
    +---/ /--+--------+--------+
    | HEADER |      STATUS     |
    +---/ /--+--------+--------+
*/
#define AHI_CONNECT_CFM_LENGTH (2)
#define AHI_CONNECT_CFM_STATUS AHI_MSG_INDEX_PAYLOAD

/*  AHI_DISCONNECT_CFM
    0 bytes  4        5        6
    +---/ /--+--------+--------+
    | HEADER |      STATUS     |
    +---/ /--+--------+--------+
*/
#define AHI_DISCONNECT_CFM_LENGTH (2)
#define AHI_DISCONNECT_CFM_STATUS AHI_MSG_INDEX_PAYLOAD

/*  AHI_VERSION_CFM
    0 bytes  4        5        6        7        8        9        10
    +---/ /--+--------+--------+--------+--------+--------+--------+
    | HEADER |      STATUS     |      MAJOR      |      MINOR      |
    +---/ /--+--------+--------+--------+--------+--------+--------+
*/
#define AHI_VERSION_CFM_LENGTH (6)
#define AHI_VERSION_CFM_STATUS AHI_MSG_INDEX_PAYLOAD
#define AHI_VERSION_CFM_MAJOR (AHI_VERSION_CFM_STATUS + 2)
#define AHI_VERSION_CFM_MINOR (AHI_VERSION_CFM_MAJOR + 2)

/*  AHI_EVENT_INJECT_IND
    0 bytes  4        5        6
    +---/ /--+--------+--------+
    | HEADER |    EVENT ID     |
    +---/ /--+--------+--------+
*/
#define AHI_EVENT_INJECT_IND_LENGTH (2)
#define AHI_EVENT_INJECT_IND_EVENT AHI_MSG_INDEX_PAYLOAD

/*  AHI_MODE_GET_CFM
    0 bytes  4        5        6        7        8
    +---/ /--+--------+--------+--------+--------+
    | HEADER |      STATUS     |    APP_MODE     |
    +---/ /--+--------+--------+--------+--------+
*/
#define AHI_MODE_GET_CFM_LENGTH (4)
#define AHI_MODE_GET_CFM_STATUS AHI_MSG_INDEX_PAYLOAD
#define AHI_MODE_GET_CFM_APP_MODE (AHI_MODE_GET_CFM_STATUS + 2)

/*  AHI_MODE_SET_REQ
    0 bytes  4        5        6
    +---/ /--+--------+--------+
    | HEADER |    APP_MODE     |
    +---/ /--+--------+--------+
*/
#define AHI_MODE_SET_REQ_LENGTH (2)
#define AHI_MODE_SET_REQ_APP_MODE AHI_MSG_INDEX_PAYLOAD

/*  AHI_MODE_SET_CFM
    0 bytes  4        5        6        7        8
    +---/ /--+--------+--------+--------+--------+
    | HEADER |      STATUS     |    APP_MODE     |
    +---/ /--+--------+--------+--------+--------+
*/
#define AHI_MODE_SET_CFM_LENGTH (4)
#define AHI_MODE_SET_CFM_STATUS AHI_MSG_INDEX_PAYLOAD
#define AHI_MODE_SET_CFM_APP_MODE (AHI_MODE_SET_CFM_STATUS + 2)

/*  AHI_CONFIG_FILE_GET_CFM
    0 bytes  4        5        6        7        8        9        10
    +---/ /--+--------+--------+--------+--------+--------+--------+
    | HEADER |      STATUS     |          SIZE_ON_DEVICE           |
    +---/ /--+--------+--------+--------+--------+--------+--------+
*/
#define AHI_CONFIG_FILE_GET_CFM_LENGTH (6)
#define AHI_CONFIG_FILE_GET_CFM_STATUS AHI_MSG_INDEX_PAYLOAD
#define AHI_CONFIG_FILE_GET_CFM_SIZE (AHI_CONFIG_FILE_GET_CFM_STATUS + 2)

/*    AHI_CONFIG_FILE_GET_DATA_IND
    0 bytes  4            4+max(data_size, AHI_CONFIG_FILE_GET_DATA_IND_DATA_SIZE_MAX)
    +---/ /--+--------//--+
    | HEADER |    DATA    |
    +---/ /--+--------//--+
    
    The length of each packet is either the maximum payload allowed or for
    the final packet, the remaining bytes.

    If there are more data packets to follow after this one, flags is set to
    AHI_FLAG_MORE_DATA. For the final packet it is AHI_FLAG_NONE.
*/
#define AHI_CONFIG_FILE_GET_DATA_IND_LENGTH (0)
#define AHI_CONFIG_FILE_GET_DATA_IND_DATA AHI_MSG_INDEX_PAYLOAD
#define AHI_CONFIG_FILE_GET_DATA_IND_DATA_SIZE_MAX (AHI_MSG_PAYLOAD_SIZE_MAX - AHI_CONFIG_FILE_GET_DATA_IND_LENGTH)

/*  AHI_CONFIG_GET_REQ
    0 bytes  4        5        6
    +---/ /--+--------+--------+
    | HEADER | CONFIG BLOCK ID |
    +---/ /--+--------+--------+
*/
#define AHI_CONFIG_GET_REQ_LENGTH (2)
#define AHI_CONFIG_GET_REQ_CONFIG_ID AHI_MSG_INDEX_PAYLOAD

/*  AHI_CONFIG_GET_CFM
    0 bytes  4        5        6        7        8        9        10                  10+size
    +---/ /--+--------+--------+--------+--------+--------+--------+--------+------//--+
    | HEADER | CONFIG BLOCK ID |     STATUS      |      SIZE       | CONFIG BLOCK DATA |
    +---/ /--+--------+--------+--------+--------+--------+--------+--------+------//--+

    The size is variable and depends on the size of the config block data.
*/
#define AHI_CONFIG_GET_CFM_LENGTH (6)
#define AHI_CONFIG_GET_CFM_CONFIG_ID AHI_MSG_INDEX_PAYLOAD
#define AHI_CONFIG_GET_CFM_STATUS (AHI_CONFIG_GET_CFM_CONFIG_ID + 2)
#define AHI_CONFIG_GET_CFM_SIZE (AHI_CONFIG_GET_CFM_STATUS + 2)
#define AHI_CONFIG_GET_CFM_DATA (AHI_CONFIG_GET_CFM_SIZE + 2)
#define AHI_CONFIG_GET_CFM_DATA_SIZE_MAX (AHI_MSG_PAYLOAD_SIZE_MAX - AHI_CONFIG_GET_CFM_LENGTH)

/*  AHI_CONFIG_SET_REQ
    0 bytes  4        5        6        7        8                   8+max(size, AHI_CONFIG_SET_REQ_DATA_SIZE_MAX)
    +---/ /--+--------+--------+--------+--------+--------+------//--+
    | HEADER | CONFIG BLOCK ID |   TOTAL_SIZE    | CONFIG BLOCK DATA |
    +---/ /--+--------+--------+--------+--------+--------+------//--+

    The size is variable and depends on the size of the config block data.
    Config data blocks larger than AHI_CONFIG_SET_REQ_DATA_SIZE_MAX are split
    over multiple AHI data packets and the flags field in the header is set
    to AHI_FLAG_MORE_DATA in all but the last packet.
*/
#define AHI_CONFIG_SET_REQ_LENGTH (4)
#define AHI_CONFIG_SET_REQ_CONFIG_ID AHI_MSG_INDEX_PAYLOAD
#define AHI_CONFIG_SET_REQ_SIZE (AHI_CONFIG_SET_REQ_CONFIG_ID + 2)
#define AHI_CONFIG_SET_REQ_DATA (AHI_CONFIG_SET_REQ_SIZE + 2)
#define AHI_CONFIG_SET_REQ_DATA_SIZE_MAX (AHI_MSG_PAYLOAD_SIZE_MAX - AHI_CONFIG_SET_REQ_LENGTH)

/*  AHI_CONFIG_SET_CFM
    0 bytes  4        5        6        7        8
    +---/ /--+--------+--------+--------+--------+
    | HEADER | CONFIG BLOCK ID |     STATUS      |
    +---/ /--+--------+--------+--------+--------+
*/
#define AHI_CONFIG_SET_CFM_LENGTH (4)
#define AHI_CONFIG_SET_CFM_CONFIG_ID AHI_MSG_INDEX_PAYLOAD
#define AHI_CONFIG_SET_CFM_STATUS (AHI_CONFIG_SET_CFM_CONFIG_ID + 2)

/*  AHI_EVENT_REPORT_IND
    0 bytes  4        5        6                   6++max(payload_size, AHI_EVENT_REPORT_IND_DATA_SIZE_MAX)
    +---/ /--+--------+--------+--------+------//--+
    | HEADER |    EVENT ID     |     PAYLOAD       |
    +---/ /--+--------+--------+--------+------//--+

    If the event has a payload it is copied into this message as a binary blob.
    If the payload is larger than AHI_SYSTEM_EVENT_SEND_IND_SIZE_MAX then it is
    truncated to AHI_SYSTEM_EVENT_SEND_IND_SIZE_MAX to avoid using multiple
    data packets.
*/
#define AHI_EVENT_REPORT_IND_LENGTH (2)
#define AHI_EVENT_REPORT_IND_EVENT AHI_MSG_INDEX_PAYLOAD
#define AHI_EVENT_REPORT_IND_DATA (AHI_EVENT_REPORT_IND_EVENT + 2)
#define AHI_EVENT_REPORT_IND_DATA_SIZE_MAX (AHI_MSG_PAYLOAD_SIZE_MAX - AHI_EVENT_REPORT_IND_LENGTH)

/*  AHI_STATE_MACHINE_STATE_REPORT_IND
    0 bytes  4        5        6        7        8
    +---/ /--+--------+--------+--------+--------+
    | HEADER | STATE MACHINE ID|      STATE      |
    +---/ /--+--------+--------+--------+--------+
*/
#define AHI_STATE_MACHINE_STATE_REPORT_IND_LENGTH (4)
#define AHI_STATE_MACHINE_STATE_REPORT_IND_STATE_MACHINE_ID AHI_MSG_INDEX_PAYLOAD
#define AHI_STATE_MACHINE_STATE_REPORT_IND_STATE (AHI_STATE_MACHINE_STATE_REPORT_IND_STATE_MACHINE_ID + 2)

/*  AHI_REPORT_MTU_IND
    0 bytes  4        5        6        7        8
    +---/ /--+--------+--------+--------+--------+
    | HEADER | MTU        |      
    +---/ /--+--------+--------+--------+--------+
*/
#define AHI_REPORT_MTU_IND_LENGTH (2)
#define AHI_REPORT_MTU_IND_ID AHI_MSG_INDEX_PAYLOAD

/*  AHI_REPORT_CONN_INT_IND
    0 bytes  4        5        6        7        8
    +---/ /--+--------+--------+--------+--------+
    | HEADER | MIN_CI   |      MAX_CI |
    +---/ /--+--------+--------+--------+--------+
*/
#define AHI_REPORT_CONN_INT_IND_LENGTH (2)
#define AHI_REPORT_CONN_INT_IND_ID AHI_MSG_INDEX_PAYLOAD

/*  AHI_CONFIG_FILE_SIGNATURE_CFM
    0 bytes  4        5        6        7        8                           40
    +---/ /--+--------+--------+--------+--------+--------+--------+------//--+
    | HEADER |      STATUS     |       TYPE      |    SIGNATURE (optional)    |
    +---/ /--+--------+--------+--------+--------+--------+--------+------//--+

    The length of SIGNATURE depends on TYPE.

    If STATUS is not AHI_STATUS_SUCCESS then the length of SIGNATURE will be 0.

    TYPE is currently always MD5 but it is included in case the signature is
    calculated a different way in future.

    The MD5 hash is stored as an ASCII hex array, i.e. the ASCII encoding of
    the hex value of each byte in the hash.
    The hash is 128bits == 16Bytes -> 32Bytes of ASCII values.
*/
#define AHI_CONFIG_FILE_SIGNATURE_CFM_LENGTH (4)
#define AHI_CONFIG_FILE_SIGNATURE_CFM_STATUS AHI_MSG_INDEX_PAYLOAD
#define AHI_CONFIG_FILE_SIGNATURE_CFM_TYPE (AHI_CONFIG_FILE_SIGNATURE_CFM_STATUS + 2)
#define AHI_CONFIG_FILE_SIGNATURE_CFM_SIGNATURE (AHI_CONFIG_FILE_SIGNATURE_CFM_STATUS + 4)
#define AHI_CONFIG_FILE_SIGNATURE_CFM_SIGNATURE_SIZE 32

/*  AHI_DISABLE_TRANSPORT_CFM
    0 bytes  4        5        6
    +---/ /--+--------+--------+
    | HEADER |      STATUS     |
    +---/ /--+--------+--------+
*/
#define AHI_DISABLE_TRANSPORT_CFM_LENGTH (2)
#define AHI_DISABLE_TRANSPORT_CFM_STATUS AHI_MSG_INDEX_PAYLOAD

/*  AHI_PRODUCT_ID_GET_CFM
    0 bytes  4        5        6        7        8        9       10
    +---/ /--+--------+--------+--------+--------+--------+--------+
    | HEADER |      STATUS     |            PRODUCT_ID             |
    +---/ /--+--------+--------+--------+--------+--------+--------+
*/
#define AHI_PRODUCT_ID_GET_CFM_LENGTH (6)
#define AHI_PRODUCT_ID_GET_CFM_STATUS AHI_MSG_INDEX_PAYLOAD
#define AHI_PRODUCT_ID_GET_CFM_ID (AHI_PRODUCT_ID_GET_CFM_STATUS + 2)

/*  AHI_APP_BUILD_ID_GET_CFM
    0 bytes  4        5        6        7        8
    +---/ /--+--------+--------+--------+--------+
    | HEADER |      STATUS     |    BUILD_ID     |
    +---/ /--+--------+--------+--------+--------+
*/
#define AHI_APP_BUILD_ID_GET_CFM_LENGTH (4)
#define AHI_APP_BUILD_ID_GET_CFM_STATUS AHI_MSG_INDEX_PAYLOAD
#define AHI_APP_BUILD_ID_GET_CFM_ID (AHI_APP_BUILD_ID_GET_CFM_STATUS + 2)

#endif /* AHI_PROTOCOL_H_ */
