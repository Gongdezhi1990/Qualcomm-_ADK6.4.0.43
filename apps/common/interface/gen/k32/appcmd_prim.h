/*****************************************************************************

            Copyright (c) 2018 Qualcomm Technologies International, Ltd.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef APPCMD_PRIM_H__
#define APPCMD_PRIM_H__

#include "hydra/hydra_types.h"


/*******************************************************************************

  NAME
    APPCMD_CARD_OVERRIDE_ID

  DESCRIPTION
    Needs to match 'enum sdioemb_override' in slot_api.h

 VALUES
    SD_HOST_APPCMD_OVERRIDE_OFF    - Card detection not overridden.
    SD_HOST_APPCMD_OVERRIDE_INSERT - Card overridden as inserted.
    SD_HOST_APPCMD_OVERRIDE_REMOVE - Card overridden as removed.

*******************************************************************************/
typedef enum
{
    SD_HOST_APPCMD_OVERRIDE_OFF = 0,
    SD_HOST_APPCMD_OVERRIDE_INSERT = 1,
    SD_HOST_APPCMD_OVERRIDE_REMOVE = 2
} APPCMD_CARD_OVERRIDE_ID;
/*******************************************************************************

  NAME
    APPCMD_COMMAND_ID

  DESCRIPTION
    The APPCMD command set.

 VALUES
    APPCMD_SET_MIB_KEY
                   -
    APPCMD_BUF_MSG_WRITE_MSG_IND
                   -
    APPCMD_BUF_MSG_NUM_MSG_REQ
                   -
    APPCMD_BUF_MSG_READ_MSG_REQ
                   -
    APPCMD_BUF_MSG_CONSUMED_MSG_IND
                   -
    APPCMD_BUF_MSG_TEST_CREATE_BUF_REQ
                   -
    APPCMD_BUF_MSG_TEST_DESTROY_BUF_REQ
                   -
    APPCMD_GET_BUFFER_SIZE
                   -
    APPCMD_GET_RSP_BUFFER
                   -
    APPCMD_CALL_FUNCTION
                   -
    APPCMD_GET_CPU_USAGE
                   -
    APPCMD_SET_MIB_OCTET_KEY
                   -
    APPCMD_MEMSET
                   -
    APPCMD_MEMCHECK
                   -
    APPCMD_START_TEST
                   -
    APPCMD_GET_MIB_KEY
                   -
    APPCMD_GET_MIB_OCTET_KEY
                   -
    APPCMD_GET_MIB_OCTET_LENGTH
                   -
    APPCMD_BUF_MSG_HANDLE_REQ
                   -
    APPCMD_BUF_MSG_FREE_SPACE_REQ
                   -
    APPCMD_BUF_MSG_PAGE_REQ
                   -

*******************************************************************************/
typedef enum
{
    APPCMD_SET_MIB_KEY = 1,
    APPCMD_SET_MIB_OCTET_KEY = 2,
    APPCMD_START_TEST = 3,
    APPCMD_GET_MIB_KEY = 4,
    APPCMD_GET_MIB_OCTET_KEY = 5,
    APPCMD_GET_MIB_OCTET_LENGTH = 6,
    APPCMD_BUF_MSG_HANDLE_REQ = 7,
    APPCMD_BUF_MSG_FREE_SPACE_REQ = 8,
    APPCMD_BUF_MSG_PAGE_REQ = 9,
    APPCMD_BUF_MSG_WRITE_MSG_IND = 10,
    APPCMD_BUF_MSG_NUM_MSG_REQ = 11,
    APPCMD_BUF_MSG_READ_MSG_REQ = 12,
    APPCMD_BUF_MSG_CONSUMED_MSG_IND = 13,
    APPCMD_BUF_MSG_TEST_CREATE_BUF_REQ = 14,
    APPCMD_BUF_MSG_TEST_DESTROY_BUF_REQ = 15,
    APPCMD_GET_BUFFER_SIZE = 16,
    APPCMD_GET_RSP_BUFFER = 17,
    APPCMD_CALL_FUNCTION = 18,
    APPCMD_GET_CPU_USAGE = 19,
    APPCMD_MEMSET = 20,
    APPCMD_MEMCHECK = 21
} APPCMD_COMMAND_ID;
/*******************************************************************************

  NAME
    APPCMD_DIRECTION

  DESCRIPTION

 VALUES
    COMMAND  -
    RESPONSE -

*******************************************************************************/
typedef enum
{
    APPCMD_DIRECTION_COMMAND = 0,
    APPCMD_DIRECTION_RESPONSE = 1
} APPCMD_DIRECTION;
/*******************************************************************************

  NAME
    APPCMD_MISC_VALUES

  DESCRIPTION
    Additional miscellaneous values

 VALUES
    CMD_BUF_LEN - The length of the buffer used by the firmware for the
                  parameter and result arrays.  This is not defined by the
                  protocol, but is presented here as it is unlikely to change
                  frequently and the host may use it to optimise fetches.  The
                  firmware violates the protocol by not returning a length for
                  results where the value returned is fixed, hence making it
                  harder for the host implementation to deal with the buffer
                  without knowing the size.

*******************************************************************************/
typedef enum
{
    APPCMD_MISC_VALUES_CMD_BUF_LEN = 10
} APPCMD_MISC_VALUES;
/*******************************************************************************

  NAME
    APPCMD_RESPONSE

  DESCRIPTION
    Values written to the response area.  The top bit of the 16-bit field is
    handled separately, as described by the protocol document.  The names
    should be self explanatory.

 VALUES
    SUCCESS                -
    INVALID_PARAMETERS     -
    INVALID_STATE          -
    UNKNOWN_COMMAND        -
    UNIMPLEMENTED          -
    RESULT_PENDING         -
    INVALID_PARAMETER_MASK -
    UNSPECIFIED            -

*******************************************************************************/
typedef enum
{
    APPCMD_RESPONSE_SUCCESS = 0x4000,
    APPCMD_RESPONSE_INVALID_PARAMETERS = 0x4001,
    APPCMD_RESPONSE_INVALID_STATE = 0x4002,
    APPCMD_RESPONSE_UNKNOWN_COMMAND = 0x4003,
    APPCMD_RESPONSE_UNIMPLEMENTED = 0x4004,
    APPCMD_RESPONSE_RESULT_PENDING = 0x4005,
    APPCMD_RESPONSE_INVALID_PARAMETER_MASK = 0x4100,
    APPCMD_RESPONSE_UNSPECIFIED = 0x4fff
} APPCMD_RESPONSE;
/*******************************************************************************

  NAME
    APPCMD_TEST_BLUESTACK_HIF_CMD_ID

  DESCRIPTION

 VALUES
    GET_RX_BPTR -
    GET_TX_BPTR -
    CMD_SENT    -

*******************************************************************************/
typedef enum
{
    APPCMD_TEST_BLUESTACK_HIF_CMD_ID_GET_RX_BPTR = 0,
    APPCMD_TEST_BLUESTACK_HIF_CMD_ID_GET_TX_BPTR = 1,
    APPCMD_TEST_BLUESTACK_HIF_CMD_ID_CMD_SENT = 2
} APPCMD_TEST_BLUESTACK_HIF_CMD_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_ETHERNET_STACK_TEST_ID

  DESCRIPTION

 VALUES
    START_RX_THROUGHPUT        -
    GET_TEST_THROUGHPUT_RESULT -

*******************************************************************************/
typedef enum
{
    APPCMD_TEST_ETHERNET_STACK_TEST_ID_START_RX_THROUGHPUT = 1,
    APPCMD_TEST_ETHERNET_STACK_TEST_ID_GET_TEST_THROUGHPUT_RESULT = 2
} APPCMD_TEST_ETHERNET_STACK_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_ID

  DESCRIPTION
    Values used as first byte of a APPCMD_START_TEST command array

 VALUES
    NFC                  -
    Fault_Test           -
    Trans_Bt             -
    Memory_Protection    -
    Bitserial_DEPRECATED -
    Led                  -
    USB_Host             -
    IPC                  -
    Ethernet_Stack       -
    Trap_API             -
    ROFS                 -
    Pio_Ctrl             -
    SD_Host              -
    Bc_Buf_Adapter_Test  -
    SDIO_App             -
    Trans_Wlan           -
    Bluestack_Hif        -
    Trans_Audio          -
    Uart_Raw             -
    USB_Audio_Class      -
    Memory_Exception     -
    Cache_Test           -

*******************************************************************************/
typedef enum
{
    APPCMD_TEST_ID_NFC = 0x1f,
    APPCMD_TEST_ID_FAULT_TEST = 0xfa,
    APPCMD_TEST_ID_TRANS_BT = 1,
    APPCMD_TEST_ID_BC_BUF_ADAPTER_TEST = 2,
    APPCMD_TEST_ID_TRANS_WLAN = 3,
    APPCMD_TEST_ID_BLUESTACK_HIF = 4,
    APPCMD_TEST_ID_TRANS_AUDIO = 5,
    APPCMD_TEST_ID_UART_RAW = 6,
    APPCMD_TEST_ID_USB_AUDIO_CLASS = 7,
    APPCMD_TEST_ID_MEMORY_EXCEPTION = 8,
    APPCMD_TEST_ID_CACHE_TEST = 9,
    APPCMD_TEST_ID_MEMORY_PROTECTION = 10,
    APPCMD_TEST_ID_BITSERIAL_DEPRECATED = 11,
    APPCMD_TEST_ID_LED = 12,
    APPCMD_TEST_ID_USB_HOST = 13,
    APPCMD_TEST_ID_IPC = 14,
    APPCMD_TEST_ID_ETHERNET_STACK = 15,
    APPCMD_TEST_ID_TRAP_API = 16,
    APPCMD_TEST_ID_ROFS = 17,
    APPCMD_TEST_ID_PIO_CTRL = 18,
    APPCMD_TEST_ID_SD_HOST = 19,
    APPCMD_TEST_ID_SDIO_APP = 20
} APPCMD_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_IPC_TEST_ID

  DESCRIPTION

 VALUES
    APPCMD_IPC_TEST_ID_SHARED_MEM  -
    APPCMD_IPC_TEST_ID_HI_PRI_HDLR -

*******************************************************************************/
typedef enum
{
    APPCMD_IPC_TEST_ID_SHARED_MEM = 0,
    APPCMD_IPC_TEST_ID_HI_PRI_HDLR = 1
} APPCMD_TEST_IPC_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_LED_TEST_ID

  DESCRIPTION

 VALUES
    LED_APPCMD_INIT          -
    LED_APPCMD_CONFIGURE     -
    LED_APPCMD_ENABLE        -
    LED_APPCMD_RESET         -
    LED_APPCMD_MAX_INSTANCES -
    LED_APPCMD_GET_CONFIG    -
    LED_APPCMD_SYNC          -

*******************************************************************************/
typedef enum
{
    LED_APPCMD_INIT = 1,
    LED_APPCMD_CONFIGURE = 2,
    LED_APPCMD_ENABLE = 3,
    LED_APPCMD_RESET = 4,
    LED_APPCMD_MAX_INSTANCES = 5,
    LED_APPCMD_GET_CONFIG = 6,
    LED_APPCMD_SYNC = 7
} APPCMD_TEST_LED_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_MEMORY_EXCEPTION_TEST_ID

  DESCRIPTION

 VALUES
    EXCEP_APPCMD_BRANCH_THROUGH_ZERO
                   -
    EXCEP_APPCMD_UNALIGNED_16BIT_ACCESS
                   -
    EXCEP_APPCMD_UNALIGNED_32BIT_ACCESS
                   -
    EXCEP_APPCMD_STACK_OVERFLOW
                   -
    EXCEP_APPCMD_OUT_OF_MMU_PAGES
                   -
    EXCEP_APPCMD_MMU_INVALID_READ
                   -
    EXCEP_APPCMD_MMU_INVALID_WRITE
                   -
    EXCEP_APPCMD_MMU_UNMAPPED_READ
                   -

*******************************************************************************/
typedef enum
{
    EXCEP_APPCMD_BRANCH_THROUGH_ZERO = 1,
    EXCEP_APPCMD_UNALIGNED_16BIT_ACCESS = 2,
    EXCEP_APPCMD_UNALIGNED_32BIT_ACCESS = 3,
    EXCEP_APPCMD_STACK_OVERFLOW = 4,
    EXCEP_APPCMD_OUT_OF_MMU_PAGES = 5,
    EXCEP_APPCMD_MMU_INVALID_READ = 6,
    EXCEP_APPCMD_MMU_INVALID_WRITE = 7,
    EXCEP_APPCMD_MMU_UNMAPPED_READ = 8
} APPCMD_TEST_MEMORY_EXCEPTION_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_MEMORY_PROTECTION_TEST_ID

  DESCRIPTION
    Whilst initially added for testing memory protection hardware. These
    commands can be used for general hardwrae testing, e.g Serial RAM and
    have been extended to include tests for other hardware module e.g the
    trace hardware.

 VALUES
    MEMPROT_APPCMD_READ      -
    MEMPROT_APPCMD_WRITE     -
    MEMPROT_APPCMD_READ16    -
    MEMPROT_APPCMD_WRITE16   -
    MEMPROT_APPCMD_READ8     -
    MEMPROT_APPCMD_WRITE8    -
    MEMPROT_APPCMD_FUNC_CALL - Causes the processor to do a lot of function
                               calls

*******************************************************************************/
typedef enum
{
    MEMPROT_APPCMD_READ = 1,
    MEMPROT_APPCMD_WRITE = 2,
    MEMPROT_APPCMD_READ16 = 3,
    MEMPROT_APPCMD_WRITE16 = 4,
    MEMPROT_APPCMD_READ8 = 5,
    MEMPROT_APPCMD_WRITE8 = 6,
    MEMPROT_APPCMD_FUNC_CALL = 7
} APPCMD_TEST_MEMORY_PROTECTION_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_PIO_CTRL_TEST_ID

  DESCRIPTION

 VALUES
    PIO_CTRL_APPCMD_INIT  -
    PIO_CTRL_APPCMD_READ  -
    PIO_CTRL_APPCMD_WRITE -

*******************************************************************************/
typedef enum
{
    PIO_CTRL_APPCMD_INIT = 1,
    PIO_CTRL_APPCMD_READ = 2,
    PIO_CTRL_APPCMD_WRITE = 3
} APPCMD_TEST_PIO_CTRL_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_ROFS_TEST_ID

  DESCRIPTION

 VALUES
    IS_MOUNTED -
    FIND_FILES -
    READ_FILES -

*******************************************************************************/
typedef enum
{
    APPCMD_TEST_ROFS_TEST_ID_IS_MOUNTED = 0,
    APPCMD_TEST_ROFS_TEST_ID_FIND_FILES = 1,
    APPCMD_TEST_ROFS_TEST_ID_READ_FILES = 2
} APPCMD_TEST_ROFS_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_SDIO_APP_TEST_ID

  DESCRIPTION
    List of APPCMDs supported by the SDIO test application.

 VALUES
    SDIO_APP_APPCMD_REGISTER - Prompt the test application to register or
                               unregister its list of sdio functions with the SD
                               Host module.
    SDIO_APP_APPCMD_TESTSTEP - Set test application's next state.
    SDIO_APP_APPCMD_STATUS   - Request the current status of the test
                               application.

*******************************************************************************/
typedef enum
{
    SDIO_APP_APPCMD_REGISTER = 1,
    SDIO_APP_APPCMD_TESTSTEP = 2,
    SDIO_APP_APPCMD_STATUS = 3
} APPCMD_TEST_SDIO_APP_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_SD_HOST_TEST_ID

  DESCRIPTION
    List of APPCMDs supported by the SD Host module.

 VALUES
    SD_HOST_APPCMD_INIT - Initialise the SD host module.
    SD_HOST_APPCMD_CARD - Override the card detection state.

*******************************************************************************/
typedef enum
{
    SD_HOST_APPCMD_INIT = 1,
    SD_HOST_APPCMD_CARD = 2
} APPCMD_TEST_SD_HOST_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_TRANS_AUDIO_TEST_ID

  DESCRIPTION

 VALUES
    START              -
    STOP               -
    GET_ENDPOINT_ID    -
    IS_STARTED         -
    SEND_COMMAND       -
    GET_RESPONSE       -
    GET_BUFFERS        -
    TRIGGER_SEND       -
    SETUP_LASH_UP_LINK -
    CREATE_AUDIO_EP    -
    DESTROY_AUDIO_EP   -

*******************************************************************************/
typedef enum
{
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_START = 0,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_STOP = 1,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_IS_STARTED = 2,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_SEND_COMMAND = 3,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_GET_RESPONSE = 4,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_GET_BUFFERS = 5,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_TRIGGER_SEND = 6,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_SETUP_LASH_UP_LINK = 7,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_CREATE_AUDIO_EP = 8,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_DESTROY_AUDIO_EP = 9,
    APPCMD_TEST_TRANS_AUDIO_TEST_ID_GET_ENDPOINT_ID = 10
} APPCMD_TEST_TRANS_AUDIO_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_TRANS_BT_TEST_ID

  DESCRIPTION

 VALUES
    SEND_HCI_CMD               -
    RECEIVE_HCI_EVT            -
    IS_STARTED                 -
    STOP                       -
    TRIGGER_SEND               -
    GET_BUFFERS                -
    RUN_ACL_DMA_TEST           -
    CHECK_ACL_DMA_TEST         -
    CLEAN_UP_ACL_DMA_TEST      -
    TEST_THROUGHPUT            -
    IS_STOPPED                 -
    CREATE_ACL_CHANNEL         -
    GET_TEST_THROUGHPUT_RESULT -
    SEND_ACL_DATA              -
    RECEIVE_ACL_DATA           -
    DELETE_ACL_CHANNEL         -
    START                      -
    CREATE_VENDOR_CHANNEL      -
    DELETE_VENDOR_CHANNEL      -

*******************************************************************************/
typedef enum
{
    APPCMD_TEST_TRANS_BT_TEST_ID_SEND_HCI_CMD = 0,
    APPCMD_TEST_TRANS_BT_TEST_ID_RECEIVE_HCI_EVT = 1,
    APPCMD_TEST_TRANS_BT_TEST_ID_CREATE_ACL_CHANNEL = 2,
    APPCMD_TEST_TRANS_BT_TEST_ID_SEND_ACL_DATA = 3,
    APPCMD_TEST_TRANS_BT_TEST_ID_RECEIVE_ACL_DATA = 4,
    APPCMD_TEST_TRANS_BT_TEST_ID_DELETE_ACL_CHANNEL = 5,
    APPCMD_TEST_TRANS_BT_TEST_ID_START = 6,
    APPCMD_TEST_TRANS_BT_TEST_ID_CREATE_VENDOR_CHANNEL = 7,
    APPCMD_TEST_TRANS_BT_TEST_ID_DELETE_VENDOR_CHANNEL = 8,
    APPCMD_TEST_TRANS_BT_TEST_ID_IS_STARTED = 11,
    APPCMD_TEST_TRANS_BT_TEST_ID_STOP = 12,
    APPCMD_TEST_TRANS_BT_TEST_ID_TRIGGER_SEND = 13,
    APPCMD_TEST_TRANS_BT_TEST_ID_GET_BUFFERS = 14,
    APPCMD_TEST_TRANS_BT_TEST_ID_RUN_ACL_DMA_TEST = 15,
    APPCMD_TEST_TRANS_BT_TEST_ID_CHECK_ACL_DMA_TEST = 16,
    APPCMD_TEST_TRANS_BT_TEST_ID_CLEAN_UP_ACL_DMA_TEST = 17,
    APPCMD_TEST_TRANS_BT_TEST_ID_TEST_THROUGHPUT = 18,
    APPCMD_TEST_TRANS_BT_TEST_ID_IS_STOPPED = 19,
    APPCMD_TEST_TRANS_BT_TEST_ID_GET_TEST_THROUGHPUT_RESULT = 20
} APPCMD_TEST_TRANS_BT_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_TRANS_WLAN_TEST_ID

  DESCRIPTION

 VALUES
    START             -
    STOP              -
    IS_STARTED        -
    SEND_SIGNAL       -
    RECEIVE_SIGNAL    -
    SEND_BULK_DATA    -
    RECEIVE_BULK_DATA -

*******************************************************************************/
typedef enum
{
    APPCMD_TEST_TRANS_WLAN_TEST_ID_START = 0,
    APPCMD_TEST_TRANS_WLAN_TEST_ID_STOP = 1,
    APPCMD_TEST_TRANS_WLAN_TEST_ID_IS_STARTED = 2,
    APPCMD_TEST_TRANS_WLAN_TEST_ID_SEND_SIGNAL = 3,
    APPCMD_TEST_TRANS_WLAN_TEST_ID_RECEIVE_SIGNAL = 4,
    APPCMD_TEST_TRANS_WLAN_TEST_ID_SEND_BULK_DATA = 5,
    APPCMD_TEST_TRANS_WLAN_TEST_ID_RECEIVE_BULK_DATA = 6
} APPCMD_TEST_TRANS_WLAN_TEST_ID;
/*******************************************************************************

  NAME
    APPCMD_TEST_UART_RAW_TEST_ID

  DESCRIPTION

 VALUES
    UART_APPCMD_LOOPBACK             -
    UART_APPCMD_STATUS               -
    UART_APPCMD_TRANSFER_LENGTH      -
    UART_APPCMD_GET_TX_DATA_BUFFER   -
    UART_APPCMD_GET_RX_DATA_BUFFER   -
    UART_APPCMD_PUT_DATA             -
    UART_APPCMD_SEND_DATA            -
    UART_APPCMD_GET_RX_DATA          -
    UART_APPCMD_RX_IDLE_TIMEOUT      -
    UART_APPCMD_RX_FORCE_END         -
    UART_APPCMD_NO_ACTIVITY_DETECTOR -
    UART_APPCMD_INIT                 -
    UART_APPCMD_GET_INTERRUPTS       -
    UART_APPCMD_LOOPBACK_MODE        -
    UART_APPCMD_PARITY               -
    UART_APPCMD_TWO_STOP_BITS        -
    UART_APPCMD_FLOW_CTRL            -
    UART_APPCMD_RTS                  -
    UART_APPCMD_RATE                 -
    UART_APPCMD_TX_ENABLE            -
    UART_APPCMD_RX_ENABLE            -

*******************************************************************************/
typedef enum
{
    UART_APPCMD_LOOPBACK = 1,
    UART_APPCMD_INIT = 2,
    UART_APPCMD_PARITY = 3,
    UART_APPCMD_TWO_STOP_BITS = 4,
    UART_APPCMD_FLOW_CTRL = 5,
    UART_APPCMD_RTS = 6,
    UART_APPCMD_RATE = 7,
    UART_APPCMD_TX_ENABLE = 8,
    UART_APPCMD_RX_ENABLE = 9,
    UART_APPCMD_STATUS = 10,
    UART_APPCMD_TRANSFER_LENGTH = 11,
    UART_APPCMD_GET_TX_DATA_BUFFER = 12,
    UART_APPCMD_GET_RX_DATA_BUFFER = 13,
    UART_APPCMD_PUT_DATA = 14,
    UART_APPCMD_SEND_DATA = 15,
    UART_APPCMD_GET_RX_DATA = 16,
    UART_APPCMD_RX_IDLE_TIMEOUT = 17,
    UART_APPCMD_RX_FORCE_END = 18,
    UART_APPCMD_NO_ACTIVITY_DETECTOR = 19,
    UART_APPCMD_GET_INTERRUPTS = 20,
    UART_APPCMD_LOOPBACK_MODE = 21
} APPCMD_TEST_UART_RAW_TEST_ID;


#define APPCMD_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    APPCMD_LED_TEST

  DESCRIPTION

  MEMBERS
    id        -
    spare     -
    param_len -

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_LED_TEST_ID id;
    uint16                  param_len;
} APPCMD_LED_TEST;

/* The following macros take APPCMD_LED_TEST *appcmd_led_test_ptr or uint32 *addr */
#define APPCMD_LED_TEST_ID_WORD_OFFSET (0)
#define APPCMD_LED_TEST_ID_GET(addr) ((APPCMD_TEST_LED_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_LED_TEST_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                              (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_LED_TEST_PARAM_LEN_WORD_OFFSET (1)
#define APPCMD_LED_TEST_PARAM_LEN_GET(addr) (((*(((const uint32 *)(addr)) + 1) & 0xffff)))
#define APPCMD_LED_TEST_PARAM_LEN_SET(addr, param_len) (*(((uint32 *)(addr)) + 1) =  \
                                                            (uint32)((*(((uint32 *)(addr)) + 1) & ~0xffff) | (((param_len)) & 0xffff)))
#define APPCMD_LED_TEST_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_LED_TEST_CREATE(id, param_len) \
    (uint32)(((id)) & 0xffff), \
    (uint32)(((param_len)) & 0xffff)
#define APPCMD_LED_TEST_PACK(addr, id, param_len) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
        *(((uint32 *)(addr)) + 1) = (uint32)((uint32)(((param_len)) & 0xffff)); \
    } while (0)

#define APPCMD_LED_TEST_MARSHALL(addr, appcmd_led_test_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_led_test_ptr)->id)) & 0xffff); \
        *((addr) + 1) = (uint32)((((appcmd_led_test_ptr)->param_len)) & 0xffff); \
    } while (0)

#define APPCMD_LED_TEST_UNMARSHALL(addr, appcmd_led_test_ptr) \
    do { \
        (appcmd_led_test_ptr)->id = APPCMD_LED_TEST_ID_GET(addr); \
        (appcmd_led_test_ptr)->param_len = APPCMD_LED_TEST_PARAM_LEN_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_PIO_CTRL_TEST

  DESCRIPTION

  MEMBERS
    id        -
    spare     -
    param_len -

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_PIO_CTRL_TEST_ID id;
    uint16                       param_len;
} APPCMD_PIO_CTRL_TEST;

/* The following macros take APPCMD_PIO_CTRL_TEST *appcmd_pio_ctrl_test_ptr or uint32 *addr */
#define APPCMD_PIO_CTRL_TEST_ID_WORD_OFFSET (0)
#define APPCMD_PIO_CTRL_TEST_ID_GET(addr) ((APPCMD_TEST_PIO_CTRL_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_PIO_CTRL_TEST_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                   (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_PIO_CTRL_TEST_PARAM_LEN_WORD_OFFSET (1)
#define APPCMD_PIO_CTRL_TEST_PARAM_LEN_GET(addr) (((*(((const uint32 *)(addr)) + 1) & 0xffff)))
#define APPCMD_PIO_CTRL_TEST_PARAM_LEN_SET(addr, param_len) (*(((uint32 *)(addr)) + 1) =  \
                                                                 (uint32)((*(((uint32 *)(addr)) + 1) & ~0xffff) | (((param_len)) & 0xffff)))
#define APPCMD_PIO_CTRL_TEST_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_PIO_CTRL_TEST_CREATE(id, param_len) \
    (uint32)(((id)) & 0xffff), \
    (uint32)(((param_len)) & 0xffff)
#define APPCMD_PIO_CTRL_TEST_PACK(addr, id, param_len) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
        *(((uint32 *)(addr)) + 1) = (uint32)((uint32)(((param_len)) & 0xffff)); \
    } while (0)

#define APPCMD_PIO_CTRL_TEST_MARSHALL(addr, appcmd_pio_ctrl_test_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_pio_ctrl_test_ptr)->id)) & 0xffff); \
        *((addr) + 1) = (uint32)((((appcmd_pio_ctrl_test_ptr)->param_len)) & 0xffff); \
    } while (0)

#define APPCMD_PIO_CTRL_TEST_UNMARSHALL(addr, appcmd_pio_ctrl_test_ptr) \
    do { \
        (appcmd_pio_ctrl_test_ptr)->id = APPCMD_PIO_CTRL_TEST_ID_GET(addr); \
        (appcmd_pio_ctrl_test_ptr)->param_len = APPCMD_PIO_CTRL_TEST_PARAM_LEN_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_SDIO_APP_REGISTER

  DESCRIPTION
    Register or unregister the test application with the sd host module

  MEMBERS
    id     -
    spare  - Padding so that the following are 32bit aligned
    state  - Any non zero value will register the function driver with the SDIO
             module

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_SDIO_APP_TEST_ID id;
    uint32                       state;
} APPCMD_SDIO_APP_REGISTER;

/* The following macros take APPCMD_SDIO_APP_REGISTER *appcmd_sdio_app_register_ptr or uint32 *addr */
#define APPCMD_SDIO_APP_REGISTER_ID_WORD_OFFSET (0)
#define APPCMD_SDIO_APP_REGISTER_ID_GET(addr) ((APPCMD_TEST_SDIO_APP_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_SDIO_APP_REGISTER_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                       (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_SDIO_APP_REGISTER_STATE_WORD_OFFSET (1)
#define APPCMD_SDIO_APP_REGISTER_STATE_GET(addr) (*(((const uint32 *)(addr)) + 1))
#define APPCMD_SDIO_APP_REGISTER_STATE_SET(addr, state) (*(((uint32 *)(addr)) + 1) = (uint32)(state))
#define APPCMD_SDIO_APP_REGISTER_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_SDIO_APP_REGISTER_CREATE(id, state) \
    (uint32)(((id)) & 0xffff), \
    (uint32)(state)
#define APPCMD_SDIO_APP_REGISTER_PACK(addr, id, state) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
        *(((uint32 *)(addr)) + 1) = (uint32)((uint32)(state)); \
    } while (0)

#define APPCMD_SDIO_APP_REGISTER_MARSHALL(addr, appcmd_sdio_app_register_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_sdio_app_register_ptr)->id)) & 0xffff); \
        *((addr) + 1) = (uint32)((appcmd_sdio_app_register_ptr)->state); \
    } while (0)

#define APPCMD_SDIO_APP_REGISTER_UNMARSHALL(addr, appcmd_sdio_app_register_ptr) \
    do { \
        (appcmd_sdio_app_register_ptr)->id = APPCMD_SDIO_APP_REGISTER_ID_GET(addr); \
        (appcmd_sdio_app_register_ptr)->state = APPCMD_SDIO_APP_REGISTER_STATE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_SDIO_APP_STATUS

  DESCRIPTION
    Poll the test application's status

  MEMBERS
    id     -
    spare  - Padding so that the following are 32bit aligned

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_SDIO_APP_TEST_ID id;
} APPCMD_SDIO_APP_STATUS;

/* The following macros take APPCMD_SDIO_APP_STATUS *appcmd_sdio_app_status_ptr or uint32 *addr */
#define APPCMD_SDIO_APP_STATUS_ID_WORD_OFFSET (0)
#define APPCMD_SDIO_APP_STATUS_ID_GET(addr) ((APPCMD_TEST_SDIO_APP_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_SDIO_APP_STATUS_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                     (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_SDIO_APP_STATUS_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_SDIO_APP_STATUS_CREATE(id) \
    (uint32)(((id)) & 0xffff)
#define APPCMD_SDIO_APP_STATUS_PACK(addr, id) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
    } while (0)

#define APPCMD_SDIO_APP_STATUS_MARSHALL(addr, appcmd_sdio_app_status_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_sdio_app_status_ptr)->id)) & 0xffff); \
    } while (0)

#define APPCMD_SDIO_APP_STATUS_UNMARSHALL(addr, appcmd_sdio_app_status_ptr) \
    do { \
        (appcmd_sdio_app_status_ptr)->id = APPCMD_SDIO_APP_STATUS_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_SDIO_APP_TESTSTEP

  DESCRIPTION
    Set test application's next state.

  MEMBERS
    id        -
    spare     - Padding so that the following are 32bit aligned
    step      - Set the test application to an arbitrary step
    increment - Any non-zero value increments the step by one

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_SDIO_APP_TEST_ID id;
    uint32                       step;
    uint32                       increment;
} APPCMD_SDIO_APP_TESTSTEP;

/* The following macros take APPCMD_SDIO_APP_TESTSTEP *appcmd_sdio_app_teststep_ptr or uint32 *addr */
#define APPCMD_SDIO_APP_TESTSTEP_ID_WORD_OFFSET (0)
#define APPCMD_SDIO_APP_TESTSTEP_ID_GET(addr) ((APPCMD_TEST_SDIO_APP_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_SDIO_APP_TESTSTEP_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                       (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_SDIO_APP_TESTSTEP_STEP_WORD_OFFSET (1)
#define APPCMD_SDIO_APP_TESTSTEP_STEP_GET(addr) (*(((const uint32 *)(addr)) + 1))
#define APPCMD_SDIO_APP_TESTSTEP_STEP_SET(addr, step) (*(((uint32 *)(addr)) + 1) = (uint32)(step))
#define APPCMD_SDIO_APP_TESTSTEP_INCREMENT_WORD_OFFSET (2)
#define APPCMD_SDIO_APP_TESTSTEP_INCREMENT_GET(addr) (*(((const uint32 *)(addr)) + 2))
#define APPCMD_SDIO_APP_TESTSTEP_INCREMENT_SET(addr, increment) (*(((uint32 *)(addr)) + 2) = (uint32)(increment))
#define APPCMD_SDIO_APP_TESTSTEP_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_SDIO_APP_TESTSTEP_CREATE(id, step, increment) \
    (uint32)(((id)) & 0xffff), \
    (uint32)(step), \
    (uint32)(increment)
#define APPCMD_SDIO_APP_TESTSTEP_PACK(addr, id, step, increment) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
        *(((uint32 *)(addr)) + 1) = (uint32)((uint32)(step)); \
        *(((uint32 *)(addr)) + 2) = (uint32)((uint32)(increment)); \
    } while (0)

#define APPCMD_SDIO_APP_TESTSTEP_MARSHALL(addr, appcmd_sdio_app_teststep_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_sdio_app_teststep_ptr)->id)) & 0xffff); \
        *((addr) + 1) = (uint32)((appcmd_sdio_app_teststep_ptr)->step); \
        *((addr) + 2) = (uint32)((appcmd_sdio_app_teststep_ptr)->increment); \
    } while (0)

#define APPCMD_SDIO_APP_TESTSTEP_UNMARSHALL(addr, appcmd_sdio_app_teststep_ptr) \
    do { \
        (appcmd_sdio_app_teststep_ptr)->id = APPCMD_SDIO_APP_TESTSTEP_ID_GET(addr); \
        (appcmd_sdio_app_teststep_ptr)->step = APPCMD_SDIO_APP_TESTSTEP_STEP_GET(addr); \
        (appcmd_sdio_app_teststep_ptr)->increment = APPCMD_SDIO_APP_TESTSTEP_INCREMENT_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_SD_HOST_CARD

  DESCRIPTION

  MEMBERS
    id     -
    spare  - Padding so that the following are 32bit aligned
    slot   - SD Host slot ID (0-3)
    state  - Card detection override state

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_SD_HOST_TEST_ID id;
    uint32                      slot;
    APPCMD_CARD_OVERRIDE_ID     state;
} APPCMD_SD_HOST_CARD;

/* The following macros take APPCMD_SD_HOST_CARD *appcmd_sd_host_card_ptr or uint32 *addr */
#define APPCMD_SD_HOST_CARD_ID_WORD_OFFSET (0)
#define APPCMD_SD_HOST_CARD_ID_GET(addr) ((APPCMD_TEST_SD_HOST_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_SD_HOST_CARD_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                  (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_SD_HOST_CARD_SLOT_WORD_OFFSET (1)
#define APPCMD_SD_HOST_CARD_SLOT_GET(addr) (*(((const uint32 *)(addr)) + 1))
#define APPCMD_SD_HOST_CARD_SLOT_SET(addr, slot) (*(((uint32 *)(addr)) + 1) = (uint32)(slot))
#define APPCMD_SD_HOST_CARD_STATE_WORD_OFFSET (2)
#define APPCMD_SD_HOST_CARD_STATE_GET(addr) ((APPCMD_CARD_OVERRIDE_ID)*(((const uint32 *)(addr)) + 2))
#define APPCMD_SD_HOST_CARD_STATE_SET(addr, state) (*(((uint32 *)(addr)) + 2) = (uint32)(state))
#define APPCMD_SD_HOST_CARD_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_SD_HOST_CARD_CREATE(id, slot, state) \
    (uint32)(((id)) & 0xffff), \
    (uint32)(slot), \
    (uint32)(state)
#define APPCMD_SD_HOST_CARD_PACK(addr, id, slot, state) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
        *(((uint32 *)(addr)) + 1) = (uint32)((uint32)(slot)); \
        *(((uint32 *)(addr)) + 2) = (uint32)((uint32)(state)); \
    } while (0)

#define APPCMD_SD_HOST_CARD_MARSHALL(addr, appcmd_sd_host_card_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_sd_host_card_ptr)->id)) & 0xffff); \
        *((addr) + 1) = (uint32)((appcmd_sd_host_card_ptr)->slot); \
        *((addr) + 2) = (uint32)((appcmd_sd_host_card_ptr)->state); \
    } while (0)

#define APPCMD_SD_HOST_CARD_UNMARSHALL(addr, appcmd_sd_host_card_ptr) \
    do { \
        (appcmd_sd_host_card_ptr)->id = APPCMD_SD_HOST_CARD_ID_GET(addr); \
        (appcmd_sd_host_card_ptr)->slot = APPCMD_SD_HOST_CARD_SLOT_GET(addr); \
        (appcmd_sd_host_card_ptr)->state = APPCMD_SD_HOST_CARD_STATE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_SD_HOST_INIT

  DESCRIPTION

  MEMBERS
    id     -

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_SD_HOST_TEST_ID id;
} APPCMD_SD_HOST_INIT;

/* The following macros take APPCMD_SD_HOST_INIT *appcmd_sd_host_init_ptr or uint32 *addr */
#define APPCMD_SD_HOST_INIT_ID_WORD_OFFSET (0)
#define APPCMD_SD_HOST_INIT_ID_GET(addr) ((APPCMD_TEST_SD_HOST_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_SD_HOST_INIT_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                  (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_SD_HOST_INIT_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_SD_HOST_INIT_CREATE(id) \
    (uint32)(((id)) & 0xffff)
#define APPCMD_SD_HOST_INIT_PACK(addr, id) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
    } while (0)

#define APPCMD_SD_HOST_INIT_MARSHALL(addr, appcmd_sd_host_init_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_sd_host_init_ptr)->id)) & 0xffff); \
    } while (0)

#define APPCMD_SD_HOST_INIT_UNMARSHALL(addr, appcmd_sd_host_init_ptr) \
    do { \
        (appcmd_sd_host_init_ptr)->id = APPCMD_SD_HOST_INIT_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_UART_RAW_TEST

  DESCRIPTION

  MEMBERS
    id        -
    spare     -
    param_len -

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_UART_RAW_TEST_ID id;
    uint16                       param_len;
} APPCMD_UART_RAW_TEST;

/* The following macros take APPCMD_UART_RAW_TEST *appcmd_uart_raw_test_ptr or uint32 *addr */
#define APPCMD_UART_RAW_TEST_ID_WORD_OFFSET (0)
#define APPCMD_UART_RAW_TEST_ID_GET(addr) ((APPCMD_TEST_UART_RAW_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_UART_RAW_TEST_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                   (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_UART_RAW_TEST_PARAM_LEN_WORD_OFFSET (1)
#define APPCMD_UART_RAW_TEST_PARAM_LEN_GET(addr) (((*(((const uint32 *)(addr)) + 1) & 0xffff)))
#define APPCMD_UART_RAW_TEST_PARAM_LEN_SET(addr, param_len) (*(((uint32 *)(addr)) + 1) =  \
                                                                 (uint32)((*(((uint32 *)(addr)) + 1) & ~0xffff) | (((param_len)) & 0xffff)))
#define APPCMD_UART_RAW_TEST_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_UART_RAW_TEST_CREATE(id, param_len) \
    (uint32)(((id)) & 0xffff), \
    (uint32)(((param_len)) & 0xffff)
#define APPCMD_UART_RAW_TEST_PACK(addr, id, param_len) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
        *(((uint32 *)(addr)) + 1) = (uint32)((uint32)(((param_len)) & 0xffff)); \
    } while (0)

#define APPCMD_UART_RAW_TEST_MARSHALL(addr, appcmd_uart_raw_test_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_uart_raw_test_ptr)->id)) & 0xffff); \
        *((addr) + 1) = (uint32)((((appcmd_uart_raw_test_ptr)->param_len)) & 0xffff); \
    } while (0)

#define APPCMD_UART_RAW_TEST_UNMARSHALL(addr, appcmd_uart_raw_test_ptr) \
    do { \
        (appcmd_uart_raw_test_ptr)->id = APPCMD_UART_RAW_TEST_ID_GET(addr); \
        (appcmd_uart_raw_test_ptr)->param_len = APPCMD_UART_RAW_TEST_PARAM_LEN_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_VAR_LEN_ARRAY_16

  DESCRIPTION
    Define a variable length array for use within APPCMD. Note. The header
    generated from this for firmware is not especially helpful, as on the
    Kalimba this will be based on an uint32

  MEMBERS
    Elements -

*******************************************************************************/
typedef struct
{
    uint32 _data[APPCMD_PRIM_ANY_SIZE];
} APPCMD_VAR_LEN_ARRAY_16;


/*******************************************************************************

  NAME
    APPCMD_VAR_LEN_ARRAY_32

  DESCRIPTION
    Define a variable length array for use within APPCMD

  MEMBERS
    Elements -

*******************************************************************************/
typedef struct
{
    uint32 _data[APPCMD_PRIM_ANY_SIZE];
} APPCMD_VAR_LEN_ARRAY_32;


/*******************************************************************************

  NAME
    APPCMD_VAR_LEN_ARRAY_8

  DESCRIPTION
    Define a variable length array for use within APPCMD. Note. The header
    generated from this for firmware is not especially helpful, as on the
    Kalimba this will be based on an uint32

  MEMBERS
    Elements -

*******************************************************************************/
typedef struct
{
    uint32 _data[APPCMD_PRIM_ANY_SIZE];
} APPCMD_VAR_LEN_ARRAY_8;


/*******************************************************************************

  NAME
    APPCMD_TRANS_BT_TEST

  DESCRIPTION

  MEMBERS
    id              -
    param_len_bytes -
    params          -

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_TRANS_BT_TEST_ID id;
    uint16                       param_len_bytes;
    APPCMD_VAR_LEN_ARRAY_8       params;
} APPCMD_TRANS_BT_TEST;

/* The following macros take APPCMD_TRANS_BT_TEST *appcmd_trans_bt_test_ptr or uint32 *addr */
#define APPCMD_TRANS_BT_TEST_ID_WORD_OFFSET (0)
#define APPCMD_TRANS_BT_TEST_ID_GET(addr) ((APPCMD_TEST_TRANS_BT_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_TRANS_BT_TEST_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                   (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_TRANS_BT_TEST_PARAM_LEN_BYTES_GET(addr) (((*(((const uint32 *)(addr))) & 0xffff0000ul) >> 16))
#define APPCMD_TRANS_BT_TEST_PARAM_LEN_BYTES_SET(addr, param_len_bytes) (*(((uint32 *)(addr))) =  \
                                                                             (uint32)((*(((uint32 *)(addr))) & ~0xffff0000ul) | (((param_len_bytes) << 16) & 0xffff0000ul)))
#define APPCMD_TRANS_BT_TEST_PARAMS_WORD_OFFSET (1)
#define APPCMD_TRANS_BT_TEST_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_TRANS_BT_TEST_CREATE(id, param_len_bytes) \
    (uint32)(((id)) & 0xffff) | \
    (uint32)(((param_len_bytes) << 16) & 0xffff0000ul)
#define APPCMD_TRANS_BT_TEST_PACK(addr, id, param_len_bytes) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff) | \
                                         (uint32)(((param_len_bytes) << 16) & 0xffff0000ul)); \
    } while (0)

#define APPCMD_TRANS_BT_TEST_MARSHALL(addr, appcmd_trans_bt_test_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_trans_bt_test_ptr)->id)) & 0xffff) | \
                    (uint32)((((appcmd_trans_bt_test_ptr)->param_len_bytes) << 16) & 0xffff0000ul); \
    } while (0)

#define APPCMD_TRANS_BT_TEST_UNMARSHALL(addr, appcmd_trans_bt_test_ptr) \
    do { \
        (appcmd_trans_bt_test_ptr)->id = APPCMD_TRANS_BT_TEST_ID_GET(addr); \
        (appcmd_trans_bt_test_ptr)->param_len_bytes = APPCMD_TRANS_BT_TEST_PARAM_LEN_BYTES_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    APPCMD_TRANS_WLAN_TEST

  DESCRIPTION

  MEMBERS
    id     -
    spare  -
    params -

*******************************************************************************/
typedef struct
{
    APPCMD_TEST_TRANS_WLAN_TEST_ID id;
    APPCMD_VAR_LEN_ARRAY_8         params;
} APPCMD_TRANS_WLAN_TEST;

/* The following macros take APPCMD_TRANS_WLAN_TEST *appcmd_trans_wlan_test_ptr or uint32 *addr */
#define APPCMD_TRANS_WLAN_TEST_ID_WORD_OFFSET (0)
#define APPCMD_TRANS_WLAN_TEST_ID_GET(addr) ((APPCMD_TEST_TRANS_WLAN_TEST_ID)((*(((const uint32 *)(addr))) & 0xffff)))
#define APPCMD_TRANS_WLAN_TEST_ID_SET(addr, id) (*(((uint32 *)(addr))) =  \
                                                     (uint32)((*(((uint32 *)(addr))) & ~0xffff) | (((id)) & 0xffff)))
#define APPCMD_TRANS_WLAN_TEST_PARAMS_WORD_OFFSET (1)
#define APPCMD_TRANS_WLAN_TEST_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define APPCMD_TRANS_WLAN_TEST_CREATE(id) \
    (uint32)(((id)) & 0xffff)
#define APPCMD_TRANS_WLAN_TEST_PACK(addr, id) \
    do { \
        *(((uint32 *)(addr))) = (uint32)((uint32)(((id)) & 0xffff)); \
    } while (0)

#define APPCMD_TRANS_WLAN_TEST_MARSHALL(addr, appcmd_trans_wlan_test_ptr) \
    do { \
        *((addr)) = (uint32)((((appcmd_trans_wlan_test_ptr)->id)) & 0xffff); \
    } while (0)

#define APPCMD_TRANS_WLAN_TEST_UNMARSHALL(addr, appcmd_trans_wlan_test_ptr) \
    do { \
        (appcmd_trans_wlan_test_ptr)->id = APPCMD_TRANS_WLAN_TEST_ID_GET(addr); \
    } while (0)


#endif /* APPCMD_PRIM_H__ */

