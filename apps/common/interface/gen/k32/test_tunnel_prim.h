/*****************************************************************************

            Copyright (c) 2019 Qualcomm Technologies International, Ltd.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef TEST_TUNNEL_PRIM_H__
#define TEST_TUNNEL_PRIM_H__

#include "hydra/hydra_types.h"


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_PKT_TYPE

  DESCRIPTION

 VALUES
    Data             -
    Create_Link_Req  -
    Create_Cfm       -
    Destroy_Link_Req -
    Destroy_Cfm      -
    Flow_Control_Req -
    Flow_Control_Ind -
    DataFrame        -
    TimedDataFrame   -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_AUDIO_PKT_TYPE_DATA = 0,
    TEST_TUNNEL_AUDIO_PKT_TYPE_CREATE_LINK_REQ = 1,
    TEST_TUNNEL_AUDIO_PKT_TYPE_CREATE_CFM = 2,
    TEST_TUNNEL_AUDIO_PKT_TYPE_DESTROY_LINK_REQ = 3,
    TEST_TUNNEL_AUDIO_PKT_TYPE_DESTROY_CFM = 4,
    TEST_TUNNEL_AUDIO_PKT_TYPE_FLOW_CONTROL_REQ = 5,
    TEST_TUNNEL_AUDIO_PKT_TYPE_FLOW_CONTROL_IND = 6,
    TEST_TUNNEL_AUDIO_PKT_TYPE_DATAFRAME = 7,
    TEST_TUNNEL_AUDIO_PKT_TYPE_TIMEDDATAFRAME = 8
} TEST_TUNNEL_AUDIO_PKT_TYPE;
/*******************************************************************************

  NAME
    TEST_TUNNEL_CONTROL_ID

  DESCRIPTION

 VALUES
    connect_req    -
    connect_rsp    -
    disconnect_req -
    disconnect_rsp -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_CONTROL_ID_CONNECT_REQ = 0,
    TEST_TUNNEL_CONTROL_ID_CONNECT_RSP = 1,
    TEST_TUNNEL_CONTROL_ID_DISCONNECT_REQ = 2,
    TEST_TUNNEL_CONTROL_ID_DISCONNECT_RSP = 3
} TEST_TUNNEL_CONTROL_ID;
/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_TYPE

  DESCRIPTION

 VALUES
    Tx_Start_Req -
    Tx_Stop_Req  -
    Rx_Start_Req -
    Rx_Stop_Req  -
    Tx_State_Ind -
    Rx_State_Ind -
    Send_Data    -
    Recv_Data    -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_CSB_SERVICE_TYPE_TX_START_REQ = 0,
    TEST_TUNNEL_CSB_SERVICE_TYPE_TX_STOP_REQ = 1,
    TEST_TUNNEL_CSB_SERVICE_TYPE_RX_START_REQ = 2,
    TEST_TUNNEL_CSB_SERVICE_TYPE_RX_STOP_REQ = 3,
    TEST_TUNNEL_CSB_SERVICE_TYPE_TX_STATE_IND = 4,
    TEST_TUNNEL_CSB_SERVICE_TYPE_RX_STATE_IND = 5,
    TEST_TUNNEL_CSB_SERVICE_TYPE_SEND_DATA = 6,
    TEST_TUNNEL_CSB_SERVICE_TYPE_RECV_DATA = 7
} TEST_TUNNEL_CSB_SERVICE_TYPE;
/*******************************************************************************

  NAME
    TEST_TUNNEL_PRODUCTION_TEST_COMMANDS

  DESCRIPTION

 VALUES
    CURATOR -
    MIB     -
    NFC     -
    APPS0   -
    INVALID -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_PRODUCTION_TEST_COMMANDS_CURATOR = 0,
    TEST_TUNNEL_PRODUCTION_TEST_COMMANDS_MIB = 1,
    TEST_TUNNEL_PRODUCTION_TEST_COMMANDS_NFC = 2,
    TEST_TUNNEL_PRODUCTION_TEST_COMMANDS_APPS0 = 3,
    TEST_TUNNEL_PRODUCTION_TEST_COMMANDS_INVALID = 4
} TEST_TUNNEL_PRODUCTION_TEST_COMMANDS;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Interrupt_Id

  DESCRIPTION

 VALUES
    GET_CPU_USAGE_REQ -
    GET_CPU_USAGE_RSP -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_INTERRUPT_ID_GET_CPU_USAGE_REQ = 1,
    TEST_TUNNEL_APPS_FW_INTERRUPT_ID_GET_CPU_USAGE_RSP = 2
} TEST_TUNNEL_APPS_FW_INTERRUPT_ID;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Module_Id

  DESCRIPTION

 VALUES
    transport_bt    -
    transport_wlan  -
    transport_audio -
    interrupt       -
    siflash         -
    sd_mmc          -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_MODULE_ID_TRANSPORT_BT = 0,
    TEST_TUNNEL_APPS_FW_MODULE_ID_TRANSPORT_WLAN = 1,
    TEST_TUNNEL_APPS_FW_MODULE_ID_TRANSPORT_AUDIO = 2,
    TEST_TUNNEL_APPS_FW_MODULE_ID_INTERRUPT = 3,
    TEST_TUNNEL_APPS_FW_MODULE_ID_SIFLASH = 4,
    TEST_TUNNEL_APPS_FW_MODULE_ID_SD_MMC = 5
} TEST_TUNNEL_APPS_FW_MODULE_ID;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_SD_MMC_Cmd_Result

  DESCRIPTION

 VALUES
    SUCCESS -
    ERROR   -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RESULT_SUCCESS = 0,
    TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RESULT_ERROR = 1
} TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RESULT;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_SD_MMC_Cmd_Type

  DESCRIPTION

 VALUES
    Read_Req  -
    Write_Req -
    Erase_Req -
    Cmd_Rsp   -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_SD_MMC_CMD_TYPE_READ_REQ = 0,
    TEST_TUNNEL_APPS_FW_SD_MMC_CMD_TYPE_WRITE_REQ = 1,
    TEST_TUNNEL_APPS_FW_SD_MMC_CMD_TYPE_ERASE_REQ = 2,
    TEST_TUNNEL_APPS_FW_SD_MMC_CMD_TYPE_CMD_RSP = 3
} TEST_TUNNEL_APPS_FW_SD_MMC_CMD_TYPE;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Siflash_Cmd_Result

  DESCRIPTION

 VALUES
    SUCCESS -
    ERROR   -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RESULT_SUCCESS = 0,
    TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RESULT_ERROR = 1
} TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RESULT;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Siflash_Cmd_Type

  DESCRIPTION

 VALUES
    Erase_Req -
    Erase_Rsp -
    Write_Req -
    Write_Rsp -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_SIFLASH_CMD_TYPE_ERASE_REQ = 0,
    TEST_TUNNEL_APPS_FW_SIFLASH_CMD_TYPE_ERASE_RSP = 1,
    TEST_TUNNEL_APPS_FW_SIFLASH_CMD_TYPE_WRITE_REQ = 2,
    TEST_TUNNEL_APPS_FW_SIFLASH_CMD_TYPE_WRITE_RSP = 3
} TEST_TUNNEL_APPS_FW_SIFLASH_CMD_TYPE;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Trans_Audio_Test_Id

  DESCRIPTION

 VALUES
    TESTER_EP_REQ -
    TESTER_EP_RSP -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_TRANS_AUDIO_TEST_ID_TESTER_EP_REQ = 1,
    TEST_TUNNEL_APPS_FW_TRANS_AUDIO_TEST_ID_TESTER_EP_RSP = 2
} TEST_TUNNEL_APPS_FW_TRANS_AUDIO_TEST_ID;
/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Trans_Bt_Test_Id

  DESCRIPTION

 VALUES
    TEST_THROUGHPUT        -
    TEST_THROUGHPUT_RESULT -
    TEST_ACL_DMA           -
    TEST_ACL_DMA_RESULT    -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_APPS_FW_TRANS_BT_TEST_ID_TEST_THROUGHPUT = 1,
    TEST_TUNNEL_APPS_FW_TRANS_BT_TEST_ID_TEST_THROUGHPUT_RESULT = 2,
    TEST_TUNNEL_APPS_FW_TRANS_BT_TEST_ID_TEST_ACL_DMA = 3,
    TEST_TUNNEL_APPS_FW_TRANS_BT_TEST_ID_TEST_ACL_DMA_RESULT = 4
} TEST_TUNNEL_APPS_FW_TRANS_BT_TEST_ID;
/*******************************************************************************

  NAME
    Test_Tunnel_Csb_Service_State

  DESCRIPTION

 VALUES
    DOWN           -
    TO_BE_UP       -
    UP             -
    TO_BE_DOWN     -
    HAS_NO_SPACE   -
    HAS_MORE_SPACE -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_CSB_SERVICE_STATE_DOWN = 0,
    TEST_TUNNEL_CSB_SERVICE_STATE_TO_BE_UP = 1,
    TEST_TUNNEL_CSB_SERVICE_STATE_UP = 2,
    TEST_TUNNEL_CSB_SERVICE_STATE_TO_BE_DOWN = 3,
    TEST_TUNNEL_CSB_SERVICE_STATE_HAS_NO_SPACE = 4,
    TEST_TUNNEL_CSB_SERVICE_STATE_HAS_MORE_SPACE = 5
} TEST_TUNNEL_CSB_SERVICE_STATE;
/*******************************************************************************

  NAME
    Test_Tunnel_Id

  DESCRIPTION

 VALUES
    control         -
    rfcli           -
    host_comms      -
    btcli           -
    apps0_fw_test   -
    apps1_fw_test   -
    trap_api        -
    accmd           -
    audio_data      -
    csb_service     -
    production_test -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_ID_CONTROL = 0,
    TEST_TUNNEL_ID_RFCLI = 1,
    TEST_TUNNEL_ID_BTCLI = 2,
    TEST_TUNNEL_ID_APPS0_FW_TEST = 3,
    TEST_TUNNEL_ID_APPS1_FW_TEST = 4,
    TEST_TUNNEL_ID_TRAP_API = 5,
    TEST_TUNNEL_ID_ACCMD = 6,
    TEST_TUNNEL_ID_AUDIO_DATA = 7,
    TEST_TUNNEL_ID_CSB_SERVICE = 8,
    TEST_TUNNEL_ID_PRODUCTION_TEST = 9,
    TEST_TUNNEL_ID_HOST_COMMS = 10
} TEST_TUNNEL_ID;
/*******************************************************************************

  NAME
    Test_Tunnel_Response_Code

  DESCRIPTION

 VALUES
    SUCCESS            -
    INVALID_PARAMETERS -
    INVALID_STATE      -
    UNKNOWN_COMMAND    -
    UNIMPLEMENTED      -

*******************************************************************************/
typedef enum
{
    TEST_TUNNEL_RESPONSE_CODE_SUCCESS = 0,
    TEST_TUNNEL_RESPONSE_CODE_INVALID_PARAMETERS = 1,
    TEST_TUNNEL_RESPONSE_CODE_INVALID_STATE = 2,
    TEST_TUNNEL_RESPONSE_CODE_UNKNOWN_COMMAND = 3,
    TEST_TUNNEL_RESPONSE_CODE_UNIMPLEMENTED = 4
} TEST_TUNNEL_RESPONSE_CODE;


#define TEST_TUNNEL_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_CREATE_CFM

  DESCRIPTION

  MEMBERS
    response        -
    endpoint_id     -
    available_bytes -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_AUDIO_CREATE_CFM;

/* The following macros take TEST_TUNNEL_AUDIO_CREATE_CFM *test_tunnel_audio_create_cfm_ptr */
#define TEST_TUNNEL_AUDIO_CREATE_CFM_RESPONSE_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_CREATE_CFM_RESPONSE_GET(test_tunnel_audio_create_cfm_ptr) ((TEST_TUNNEL_RESPONSE_CODE)(((test_tunnel_audio_create_cfm_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_CREATE_CFM_RESPONSE_SET(test_tunnel_audio_create_cfm_ptr, response) ((test_tunnel_audio_create_cfm_ptr)->_data[0] =  \
                                                                                                   (uint32)(((test_tunnel_audio_create_cfm_ptr)->_data[0] & ~0xffff) | (((response)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_CREATE_CFM_ENDPOINT_ID_GET(test_tunnel_audio_create_cfm_ptr) ((((test_tunnel_audio_create_cfm_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_AUDIO_CREATE_CFM_ENDPOINT_ID_SET(test_tunnel_audio_create_cfm_ptr, endpoint_id) ((test_tunnel_audio_create_cfm_ptr)->_data[0] =  \
                                                                                                         (uint32)(((test_tunnel_audio_create_cfm_ptr)->_data[0] & ~0xffff0000ul) | (((endpoint_id) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_AUDIO_CREATE_CFM_AVAILABLE_BYTES_WORD_OFFSET (1)
#define TEST_TUNNEL_AUDIO_CREATE_CFM_AVAILABLE_BYTES_GET(test_tunnel_audio_create_cfm_ptr) ((((test_tunnel_audio_create_cfm_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_AUDIO_CREATE_CFM_AVAILABLE_BYTES_SET(test_tunnel_audio_create_cfm_ptr, available_bytes) ((test_tunnel_audio_create_cfm_ptr)->_data[1] =  \
                                                                                                                 (uint32)(((test_tunnel_audio_create_cfm_ptr)->_data[1] & ~0xffff) | (((available_bytes)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_CREATE_CFM_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_CREATE_CFM_CREATE(response, endpoint_id, available_bytes) \
    (uint32)(((response)) & 0xffff) | \
    (uint32)(((endpoint_id) << 16) & 0xffff0000ul), \
    (uint32)(((available_bytes)) & 0xffff)
#define TEST_TUNNEL_AUDIO_CREATE_CFM_PACK(test_tunnel_audio_create_cfm_ptr, response, endpoint_id, available_bytes) \
    do { \
        (test_tunnel_audio_create_cfm_ptr)->_data[0] = (uint32)((uint32)(((response)) & 0xffff) | \
                                                                (uint32)(((endpoint_id) << 16) & 0xffff0000ul)); \
        (test_tunnel_audio_create_cfm_ptr)->_data[1] = (uint32)((uint32)(((available_bytes)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_CREATE_REQ

  DESCRIPTION

  MEMBERS
    sink                  -
    hdrlen                -
    type                  -
    connected_endpoint_id -
    params                -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_AUDIO_CREATE_REQ;

/* The following macros take TEST_TUNNEL_AUDIO_CREATE_REQ *test_tunnel_audio_create_req_ptr */
#define TEST_TUNNEL_AUDIO_CREATE_REQ_SINK_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_CREATE_REQ_SINK_GET(test_tunnel_audio_create_req_ptr) ((((test_tunnel_audio_create_req_ptr)->_data[0] & 0xff)))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_SINK_SET(test_tunnel_audio_create_req_ptr, sink) ((test_tunnel_audio_create_req_ptr)->_data[0] =  \
                                                                                           (uint32)(((test_tunnel_audio_create_req_ptr)->_data[0] & ~0xff) | (((sink)) & 0xff)))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_HDRLEN_GET(test_tunnel_audio_create_req_ptr) ((((test_tunnel_audio_create_req_ptr)->_data[0] & 0xff00) >> 8))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_HDRLEN_SET(test_tunnel_audio_create_req_ptr, hdrlen) ((test_tunnel_audio_create_req_ptr)->_data[0] =  \
                                                                                               (uint32)(((test_tunnel_audio_create_req_ptr)->_data[0] & ~0xff00) | (((hdrlen) << 8) & 0xff00)))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_TYPE_GET(test_tunnel_audio_create_req_ptr) ((ACCMD_STREAM_DEVICE)(((test_tunnel_audio_create_req_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_TYPE_SET(test_tunnel_audio_create_req_ptr, type) ((test_tunnel_audio_create_req_ptr)->_data[0] =  \
                                                                                           (uint32)(((test_tunnel_audio_create_req_ptr)->_data[0] & ~0xffff0000ul) | (((type) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_CONNECTED_ENDPOINT_ID_WORD_OFFSET (1)
#define TEST_TUNNEL_AUDIO_CREATE_REQ_CONNECTED_ENDPOINT_ID_GET(test_tunnel_audio_create_req_ptr) ((((test_tunnel_audio_create_req_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_CONNECTED_ENDPOINT_ID_SET(test_tunnel_audio_create_req_ptr, connected_endpoint_id) ((test_tunnel_audio_create_req_ptr)->_data[1] =  \
                                                                                                                             (uint32)(((test_tunnel_audio_create_req_ptr)->_data[1] & ~0xffff) | (((connected_endpoint_id)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_PARAMS_GET(test_tunnel_audio_create_req_ptr) ((((test_tunnel_audio_create_req_ptr)->_data[1] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_PARAMS_SET(test_tunnel_audio_create_req_ptr, params) ((test_tunnel_audio_create_req_ptr)->_data[1] =  \
                                                                                               (uint32)(((test_tunnel_audio_create_req_ptr)->_data[1] & ~0xffff0000ul) | (((params) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_AUDIO_CREATE_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_CREATE_REQ_CREATE(sink, hdrlen, type, connected_endpoint_id, params) \
    (uint32)(((sink)) & 0xff) | \
    (uint32)(((hdrlen) << 8) & 0xff00) | \
    (uint32)(((type) << 16) & 0xffff0000ul), \
    (uint32)(((connected_endpoint_id)) & 0xffff) | \
    (uint32)(((params) << 16) & 0xffff0000ul)
#define TEST_TUNNEL_AUDIO_CREATE_REQ_PACK(test_tunnel_audio_create_req_ptr, sink, hdrlen, type, connected_endpoint_id, params) \
    do { \
        (test_tunnel_audio_create_req_ptr)->_data[0] = (uint32)((uint32)(((sink)) & 0xff) | \
                                                                (uint32)(((hdrlen) << 8) & 0xff00) | \
                                                                (uint32)(((type) << 16) & 0xffff0000ul)); \
        (test_tunnel_audio_create_req_ptr)->_data[1] = (uint32)((uint32)(((connected_endpoint_id)) & 0xffff) | \
                                                                (uint32)(((params) << 16) & 0xffff0000ul)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_DATA

  DESCRIPTION

  MEMBERS
    endpoint_id -
    payload     -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_AUDIO_DATA;

/* The following macros take TEST_TUNNEL_AUDIO_DATA *test_tunnel_audio_data_ptr */
#define TEST_TUNNEL_AUDIO_DATA_ENDPOINT_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_DATA_ENDPOINT_ID_GET(test_tunnel_audio_data_ptr) ((((test_tunnel_audio_data_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_DATA_ENDPOINT_ID_SET(test_tunnel_audio_data_ptr, endpoint_id) ((test_tunnel_audio_data_ptr)->_data[0] =  \
                                                                                             (uint32)(((test_tunnel_audio_data_ptr)->_data[0] & ~0xffff) | (((endpoint_id)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_DATA_PAYLOAD_GET(test_tunnel_audio_data_ptr) ((((test_tunnel_audio_data_ptr)->_data[0] & 0xff0000) >> 16))
#define TEST_TUNNEL_AUDIO_DATA_PAYLOAD_SET(test_tunnel_audio_data_ptr, payload) ((test_tunnel_audio_data_ptr)->_data[0] =  \
                                                                                     (uint32)(((test_tunnel_audio_data_ptr)->_data[0] & ~0xff0000) | (((payload) << 16) & 0xff0000)))
#define TEST_TUNNEL_AUDIO_DATA_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_DATA_CREATE(endpoint_id, payload) \
    (uint32)(((endpoint_id)) & 0xffff) | \
    (uint32)(((payload) << 16) & 0xff0000)
#define TEST_TUNNEL_AUDIO_DATA_PACK(test_tunnel_audio_data_ptr, endpoint_id, payload) \
    do { \
        (test_tunnel_audio_data_ptr)->_data[0] = (uint32)((uint32)(((endpoint_id)) & 0xffff) | \
                                                          (uint32)(((payload) << 16) & 0xff0000)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_DATA_FRAME

  DESCRIPTION

  MEMBERS
    endpoint_id  -
    pkt_boundary -
    spare        -
    header_len   -
    header       -
    payload      -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_AUDIO_DATA_FRAME;

/* The following macros take TEST_TUNNEL_AUDIO_DATA_FRAME *test_tunnel_audio_data_frame_ptr */
#define TEST_TUNNEL_AUDIO_DATA_FRAME_ENDPOINT_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_DATA_FRAME_ENDPOINT_ID_GET(test_tunnel_audio_data_frame_ptr) ((((test_tunnel_audio_data_frame_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_ENDPOINT_ID_SET(test_tunnel_audio_data_frame_ptr, endpoint_id) ((test_tunnel_audio_data_frame_ptr)->_data[0] =  \
                                                                                                         (uint32)(((test_tunnel_audio_data_frame_ptr)->_data[0] & ~0xffff) | (((endpoint_id)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_PKT_BOUNDARY_GET(test_tunnel_audio_data_frame_ptr) ((((test_tunnel_audio_data_frame_ptr)->_data[0] & 0x10000) >> 16))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_PKT_BOUNDARY_SET(test_tunnel_audio_data_frame_ptr, pkt_boundary) ((test_tunnel_audio_data_frame_ptr)->_data[0] =  \
                                                                                                           (uint32)(((test_tunnel_audio_data_frame_ptr)->_data[0] & ~0x10000) | (((pkt_boundary) << 16) & 0x10000)))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_HEADER_LEN_GET(test_tunnel_audio_data_frame_ptr) ((((test_tunnel_audio_data_frame_ptr)->_data[0] & 0xff000000ul) >> 24))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_HEADER_LEN_SET(test_tunnel_audio_data_frame_ptr, header_len) ((test_tunnel_audio_data_frame_ptr)->_data[0] =  \
                                                                                                       (uint32)(((test_tunnel_audio_data_frame_ptr)->_data[0] & ~0xff000000ul) | (((header_len) << 24) & 0xff000000ul)))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_HEADER_WORD_OFFSET (1)
#define TEST_TUNNEL_AUDIO_DATA_FRAME_HEADER_GET(test_tunnel_audio_data_frame_ptr) ((((test_tunnel_audio_data_frame_ptr)->_data[1] & 0xff)))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_HEADER_SET(test_tunnel_audio_data_frame_ptr, header) ((test_tunnel_audio_data_frame_ptr)->_data[1] =  \
                                                                                               (uint32)(((test_tunnel_audio_data_frame_ptr)->_data[1] & ~0xff) | (((header)) & 0xff)))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_PAYLOAD_GET(test_tunnel_audio_data_frame_ptr) ((((test_tunnel_audio_data_frame_ptr)->_data[1] & 0xff00) >> 8))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_PAYLOAD_SET(test_tunnel_audio_data_frame_ptr, payload) ((test_tunnel_audio_data_frame_ptr)->_data[1] =  \
                                                                                                 (uint32)(((test_tunnel_audio_data_frame_ptr)->_data[1] & ~0xff00) | (((payload) << 8) & 0xff00)))
#define TEST_TUNNEL_AUDIO_DATA_FRAME_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_DATA_FRAME_CREATE(endpoint_id, pkt_boundary, header_len, header, payload) \
    (uint32)(((endpoint_id)) & 0xffff) | \
    (uint32)(((pkt_boundary) << 16) & 0x10000) | \
    (uint32)(((header_len) << 24) & 0xff000000ul), \
    (uint32)(((header)) & 0xff) | \
    (uint32)(((payload) << 8) & 0xff00)
#define TEST_TUNNEL_AUDIO_DATA_FRAME_PACK(test_tunnel_audio_data_frame_ptr, endpoint_id, pkt_boundary, header_len, header, payload) \
    do { \
        (test_tunnel_audio_data_frame_ptr)->_data[0] = (uint32)((uint32)(((endpoint_id)) & 0xffff) | \
                                                                (uint32)(((pkt_boundary) << 16) & 0x10000) | \
                                                                (uint32)(((header_len) << 24) & 0xff000000ul)); \
        (test_tunnel_audio_data_frame_ptr)->_data[1] = (uint32)((uint32)(((header)) & 0xff) | \
                                                                (uint32)(((payload) << 8) & 0xff00)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_DESTROY_CFM

  DESCRIPTION

  MEMBERS
    response    -
    endpoint_id -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_AUDIO_DESTROY_CFM;

/* The following macros take TEST_TUNNEL_AUDIO_DESTROY_CFM *test_tunnel_audio_destroy_cfm_ptr */
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_RESPONSE_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_RESPONSE_GET(test_tunnel_audio_destroy_cfm_ptr) ((TEST_TUNNEL_RESPONSE_CODE)(((test_tunnel_audio_destroy_cfm_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_RESPONSE_SET(test_tunnel_audio_destroy_cfm_ptr, response) ((test_tunnel_audio_destroy_cfm_ptr)->_data[0] =  \
                                                                                                     (uint32)(((test_tunnel_audio_destroy_cfm_ptr)->_data[0] & ~0xffff) | (((response)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_ENDPOINT_ID_GET(test_tunnel_audio_destroy_cfm_ptr) ((((test_tunnel_audio_destroy_cfm_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_ENDPOINT_ID_SET(test_tunnel_audio_destroy_cfm_ptr, endpoint_id) ((test_tunnel_audio_destroy_cfm_ptr)->_data[0] =  \
                                                                                                           (uint32)(((test_tunnel_audio_destroy_cfm_ptr)->_data[0] & ~0xffff0000ul) | (((endpoint_id) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_CREATE(response, endpoint_id) \
    (uint32)(((response)) & 0xffff) | \
    (uint32)(((endpoint_id) << 16) & 0xffff0000ul)
#define TEST_TUNNEL_AUDIO_DESTROY_CFM_PACK(test_tunnel_audio_destroy_cfm_ptr, response, endpoint_id) \
    do { \
        (test_tunnel_audio_destroy_cfm_ptr)->_data[0] = (uint32)((uint32)(((response)) & 0xffff) | \
                                                                 (uint32)(((endpoint_id) << 16) & 0xffff0000ul)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_DESTROY_REQ

  DESCRIPTION

  MEMBERS
    endpoint_id -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_AUDIO_DESTROY_REQ;

/* The following macros take TEST_TUNNEL_AUDIO_DESTROY_REQ *test_tunnel_audio_destroy_req_ptr */
#define TEST_TUNNEL_AUDIO_DESTROY_REQ_ENDPOINT_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_DESTROY_REQ_ENDPOINT_ID_GET(test_tunnel_audio_destroy_req_ptr) ((((test_tunnel_audio_destroy_req_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_DESTROY_REQ_ENDPOINT_ID_SET(test_tunnel_audio_destroy_req_ptr, endpoint_id) ((test_tunnel_audio_destroy_req_ptr)->_data[0] =  \
                                                                                                           (uint32)(((test_tunnel_audio_destroy_req_ptr)->_data[0] & ~0xffff) | (((endpoint_id)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_DESTROY_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_DESTROY_REQ_CREATE(endpoint_id) \
    (uint32)(((endpoint_id)) & 0xffff)
#define TEST_TUNNEL_AUDIO_DESTROY_REQ_PACK(test_tunnel_audio_destroy_req_ptr, endpoint_id) \
    do { \
        (test_tunnel_audio_destroy_req_ptr)->_data[0] = (uint32)((uint32)(((endpoint_id)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_FLOW_IND

  DESCRIPTION

  MEMBERS
    endpoint_id     -
    bytes_consumed  -
    available_bytes -
    pkts_consumed   -
    available_pkts  -

*******************************************************************************/
typedef struct
{
    uint32 _data[3];
} TEST_TUNNEL_AUDIO_FLOW_IND;

/* The following macros take TEST_TUNNEL_AUDIO_FLOW_IND *test_tunnel_audio_flow_ind_ptr */
#define TEST_TUNNEL_AUDIO_FLOW_IND_ENDPOINT_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_FLOW_IND_ENDPOINT_ID_GET(test_tunnel_audio_flow_ind_ptr) ((((test_tunnel_audio_flow_ind_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_ENDPOINT_ID_SET(test_tunnel_audio_flow_ind_ptr, endpoint_id) ((test_tunnel_audio_flow_ind_ptr)->_data[0] =  \
                                                                                                     (uint32)(((test_tunnel_audio_flow_ind_ptr)->_data[0] & ~0xffff) | (((endpoint_id)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_BYTES_CONSUMED_GET(test_tunnel_audio_flow_ind_ptr) ((((test_tunnel_audio_flow_ind_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_AUDIO_FLOW_IND_BYTES_CONSUMED_SET(test_tunnel_audio_flow_ind_ptr, bytes_consumed) ((test_tunnel_audio_flow_ind_ptr)->_data[0] =  \
                                                                                                           (uint32)(((test_tunnel_audio_flow_ind_ptr)->_data[0] & ~0xffff0000ul) | (((bytes_consumed) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_AVAILABLE_BYTES_WORD_OFFSET (1)
#define TEST_TUNNEL_AUDIO_FLOW_IND_AVAILABLE_BYTES_GET(test_tunnel_audio_flow_ind_ptr) ((((test_tunnel_audio_flow_ind_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_AVAILABLE_BYTES_SET(test_tunnel_audio_flow_ind_ptr, available_bytes) ((test_tunnel_audio_flow_ind_ptr)->_data[1] =  \
                                                                                                             (uint32)(((test_tunnel_audio_flow_ind_ptr)->_data[1] & ~0xffff) | (((available_bytes)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_PKTS_CONSUMED_GET(test_tunnel_audio_flow_ind_ptr) ((((test_tunnel_audio_flow_ind_ptr)->_data[1] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_AUDIO_FLOW_IND_PKTS_CONSUMED_SET(test_tunnel_audio_flow_ind_ptr, pkts_consumed) ((test_tunnel_audio_flow_ind_ptr)->_data[1] =  \
                                                                                                         (uint32)(((test_tunnel_audio_flow_ind_ptr)->_data[1] & ~0xffff0000ul) | (((pkts_consumed) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_AVAILABLE_PKTS_WORD_OFFSET (2)
#define TEST_TUNNEL_AUDIO_FLOW_IND_AVAILABLE_PKTS_GET(test_tunnel_audio_flow_ind_ptr) ((((test_tunnel_audio_flow_ind_ptr)->_data[2] & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_AVAILABLE_PKTS_SET(test_tunnel_audio_flow_ind_ptr, available_pkts) ((test_tunnel_audio_flow_ind_ptr)->_data[2] =  \
                                                                                                           (uint32)(((test_tunnel_audio_flow_ind_ptr)->_data[2] & ~0xffff) | (((available_pkts)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_IND_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_FLOW_IND_CREATE(endpoint_id, bytes_consumed, available_bytes, pkts_consumed, available_pkts) \
    (uint32)(((endpoint_id)) & 0xffff) | \
    (uint32)(((bytes_consumed) << 16) & 0xffff0000ul), \
    (uint32)(((available_bytes)) & 0xffff) | \
    (uint32)(((pkts_consumed) << 16) & 0xffff0000ul), \
    (uint32)(((available_pkts)) & 0xffff)
#define TEST_TUNNEL_AUDIO_FLOW_IND_PACK(test_tunnel_audio_flow_ind_ptr, endpoint_id, bytes_consumed, available_bytes, pkts_consumed, available_pkts) \
    do { \
        (test_tunnel_audio_flow_ind_ptr)->_data[0] = (uint32)((uint32)(((endpoint_id)) & 0xffff) | \
                                                              (uint32)(((bytes_consumed) << 16) & 0xffff0000ul)); \
        (test_tunnel_audio_flow_ind_ptr)->_data[1] = (uint32)((uint32)(((available_bytes)) & 0xffff) | \
                                                              (uint32)(((pkts_consumed) << 16) & 0xffff0000ul)); \
        (test_tunnel_audio_flow_ind_ptr)->_data[2] = (uint32)((uint32)(((available_pkts)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_FLOW_REQ

  DESCRIPTION

  MEMBERS
    endpoint_id -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_AUDIO_FLOW_REQ;

/* The following macros take TEST_TUNNEL_AUDIO_FLOW_REQ *test_tunnel_audio_flow_req_ptr */
#define TEST_TUNNEL_AUDIO_FLOW_REQ_ENDPOINT_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_FLOW_REQ_ENDPOINT_ID_GET(test_tunnel_audio_flow_req_ptr) ((((test_tunnel_audio_flow_req_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_REQ_ENDPOINT_ID_SET(test_tunnel_audio_flow_req_ptr, endpoint_id) ((test_tunnel_audio_flow_req_ptr)->_data[0] =  \
                                                                                                     (uint32)(((test_tunnel_audio_flow_req_ptr)->_data[0] & ~0xffff) | (((endpoint_id)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_FLOW_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_FLOW_REQ_CREATE(endpoint_id) \
    (uint32)(((endpoint_id)) & 0xffff)
#define TEST_TUNNEL_AUDIO_FLOW_REQ_PACK(test_tunnel_audio_flow_req_ptr, endpoint_id) \
    do { \
        (test_tunnel_audio_flow_req_ptr)->_data[0] = (uint32)((uint32)(((endpoint_id)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME

  DESCRIPTION

  MEMBERS
    presentation_time -
    endpoint_id       -
    pkt_boundary      -
    spare             -
    header_len        -
    header            -
    payload           -

*******************************************************************************/
typedef struct
{
    uint32 _data[3];
} TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME;

/* The following macros take TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME *test_tunnel_audio_timed_data_frame_ptr */
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PRESENTATION_TIME_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PRESENTATION_TIME_GET(test_tunnel_audio_timed_data_frame_ptr) ((test_tunnel_audio_timed_data_frame_ptr)->_data[0])
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PRESENTATION_TIME_SET(test_tunnel_audio_timed_data_frame_ptr, presentation_time) ((test_tunnel_audio_timed_data_frame_ptr)->_data[0] = (uint32)(presentation_time))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_ENDPOINT_ID_WORD_OFFSET (1)
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_ENDPOINT_ID_GET(test_tunnel_audio_timed_data_frame_ptr) ((((test_tunnel_audio_timed_data_frame_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_ENDPOINT_ID_SET(test_tunnel_audio_timed_data_frame_ptr, endpoint_id) ((test_tunnel_audio_timed_data_frame_ptr)->_data[1] =  \
                                                                                                                     (uint32)(((test_tunnel_audio_timed_data_frame_ptr)->_data[1] & ~0xffff) | (((endpoint_id)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PKT_BOUNDARY_GET(test_tunnel_audio_timed_data_frame_ptr) ((((test_tunnel_audio_timed_data_frame_ptr)->_data[1] & 0x10000) >> 16))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PKT_BOUNDARY_SET(test_tunnel_audio_timed_data_frame_ptr, pkt_boundary) ((test_tunnel_audio_timed_data_frame_ptr)->_data[1] =  \
                                                                                                                       (uint32)(((test_tunnel_audio_timed_data_frame_ptr)->_data[1] & ~0x10000) | (((pkt_boundary) << 16) & 0x10000)))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_HEADER_LEN_GET(test_tunnel_audio_timed_data_frame_ptr) ((((test_tunnel_audio_timed_data_frame_ptr)->_data[1] & 0xff000000ul) >> 24))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_HEADER_LEN_SET(test_tunnel_audio_timed_data_frame_ptr, header_len) ((test_tunnel_audio_timed_data_frame_ptr)->_data[1] =  \
                                                                                                                   (uint32)(((test_tunnel_audio_timed_data_frame_ptr)->_data[1] & ~0xff000000ul) | (((header_len) << 24) & 0xff000000ul)))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_HEADER_WORD_OFFSET (2)
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_HEADER_GET(test_tunnel_audio_timed_data_frame_ptr) ((((test_tunnel_audio_timed_data_frame_ptr)->_data[2] & 0xff)))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_HEADER_SET(test_tunnel_audio_timed_data_frame_ptr, header) ((test_tunnel_audio_timed_data_frame_ptr)->_data[2] =  \
                                                                                                           (uint32)(((test_tunnel_audio_timed_data_frame_ptr)->_data[2] & ~0xff) | (((header)) & 0xff)))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PAYLOAD_GET(test_tunnel_audio_timed_data_frame_ptr) ((((test_tunnel_audio_timed_data_frame_ptr)->_data[2] & 0xff00) >> 8))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PAYLOAD_SET(test_tunnel_audio_timed_data_frame_ptr, payload) ((test_tunnel_audio_timed_data_frame_ptr)->_data[2] =  \
                                                                                                             (uint32)(((test_tunnel_audio_timed_data_frame_ptr)->_data[2] & ~0xff00) | (((payload) << 8) & 0xff00)))
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_CREATE(presentation_time, endpoint_id, pkt_boundary, header_len, header, payload) \
    (uint32)(presentation_time), \
    (uint32)(((endpoint_id)) & 0xffff) | \
    (uint32)(((pkt_boundary) << 16) & 0x10000) | \
    (uint32)(((header_len) << 24) & 0xff000000ul), \
    (uint32)(((header)) & 0xff) | \
    (uint32)(((payload) << 8) & 0xff00)
#define TEST_TUNNEL_AUDIO_TIMED_DATA_FRAME_PACK(test_tunnel_audio_timed_data_frame_ptr, presentation_time, endpoint_id, pkt_boundary, header_len, header, payload) \
    do { \
        (test_tunnel_audio_timed_data_frame_ptr)->_data[0] = (uint32)((uint32)(presentation_time)); \
        (test_tunnel_audio_timed_data_frame_ptr)->_data[1] = (uint32)((uint32)(((endpoint_id)) & 0xffff) | \
                                                                      (uint32)(((pkt_boundary) << 16) & 0x10000) | \
                                                                      (uint32)(((header_len) << 24) & 0xff000000ul)); \
        (test_tunnel_audio_timed_data_frame_ptr)->_data[2] = (uint32)((uint32)(((header)) & 0xff) | \
                                                                      (uint32)(((payload) << 8) & 0xff00)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_AUDIO_TRANSPORT

  DESCRIPTION

  MEMBERS
    type    -
    payload -

*******************************************************************************/
typedef struct
{
    uint32 _data[3];
} TEST_TUNNEL_AUDIO_TRANSPORT;

/* The following macros take TEST_TUNNEL_AUDIO_TRANSPORT *test_tunnel_audio_transport_ptr */
#define TEST_TUNNEL_AUDIO_TRANSPORT_TYPE_WORD_OFFSET (0)
#define TEST_TUNNEL_AUDIO_TRANSPORT_TYPE_GET(test_tunnel_audio_transport_ptr) ((TEST_TUNNEL_AUDIO_PKT_TYPE)(((test_tunnel_audio_transport_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_AUDIO_TRANSPORT_TYPE_SET(test_tunnel_audio_transport_ptr, type) ((test_tunnel_audio_transport_ptr)->_data[0] =  \
                                                                                         (uint32)(((test_tunnel_audio_transport_ptr)->_data[0] & ~0xffff) | (((type)) & 0xffff)))
#define TEST_TUNNEL_AUDIO_TRANSPORT_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_AUDIO_TRANSPORT_CREATE(type) \
    (uint32)(((type)) & 0xffff)
#define TEST_TUNNEL_AUDIO_TRANSPORT_PACK(test_tunnel_audio_transport_ptr, type) \
    do { \
        (test_tunnel_audio_transport_ptr)->_data[0] = (uint32)((uint32)(((type)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CONTROL_RSP

  DESCRIPTION

  MEMBERS
    response -
    padding  -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_CONTROL_RSP;

/* The following macros take TEST_TUNNEL_CONTROL_RSP *test_tunnel_control_rsp_ptr */
#define TEST_TUNNEL_CONTROL_RSP_RESPONSE_WORD_OFFSET (0)
#define TEST_TUNNEL_CONTROL_RSP_RESPONSE_GET(test_tunnel_control_rsp_ptr) ((TEST_TUNNEL_RESPONSE_CODE)(((test_tunnel_control_rsp_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CONTROL_RSP_RESPONSE_SET(test_tunnel_control_rsp_ptr, response) ((test_tunnel_control_rsp_ptr)->_data[0] =  \
                                                                                         (uint32)(((test_tunnel_control_rsp_ptr)->_data[0] & ~0xffff) | (((response)) & 0xffff)))
#define TEST_TUNNEL_CONTROL_RSP_PADDING_GET(test_tunnel_control_rsp_ptr) ((((test_tunnel_control_rsp_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_CONTROL_RSP_PADDING_SET(test_tunnel_control_rsp_ptr, padding) ((test_tunnel_control_rsp_ptr)->_data[0] =  \
                                                                                       (uint32)(((test_tunnel_control_rsp_ptr)->_data[0] & ~0xffff0000ul) | (((padding) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_CONTROL_RSP_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CONTROL_RSP_CREATE(response, padding) \
    (uint32)(((response)) & 0xffff) | \
    (uint32)(((padding) << 16) & 0xffff0000ul)
#define TEST_TUNNEL_CONTROL_RSP_PACK(test_tunnel_control_rsp_ptr, response, padding) \
    do { \
        (test_tunnel_control_rsp_ptr)->_data[0] = (uint32)((uint32)(((response)) & 0xffff) | \
                                                           (uint32)(((padding) << 16) & 0xffff0000ul)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_RECV_DATA

  DESCRIPTION

  MEMBERS
    lt_addr        -
    remote_addr    -
    payload_length -
    msg_start      -
    payload        -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_CSB_SERVICE_RECV_DATA;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_RECV_DATA *test_tunnel_csb_service_recv_data_ptr */
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_LT_ADDR_GET(test_tunnel_csb_service_recv_data_ptr) ((((test_tunnel_csb_service_recv_data_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_LT_ADDR_SET(test_tunnel_csb_service_recv_data_ptr, lt_addr) ((test_tunnel_csb_service_recv_data_ptr)->_data[0] =  \
                                                                                                           (uint32)(((test_tunnel_csb_service_recv_data_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_REMOTE_ADDR_GET(test_tunnel_csb_service_recv_data_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_REMOTE_ADDR_SET(test_tunnel_csb_service_recv_data_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_PAYLOAD_LENGTH_GET(test_tunnel_csb_service_recv_data_ptr) ((((test_tunnel_csb_service_recv_data_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_PAYLOAD_LENGTH_SET(test_tunnel_csb_service_recv_data_ptr, payload_length) ((test_tunnel_csb_service_recv_data_ptr)->_data[0] =  \
                                                                                                                         (uint32)(((test_tunnel_csb_service_recv_data_ptr)->_data[0] & ~0xffff0000ul) | (((payload_length) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_MSG_START_WORD_OFFSET (1)
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_MSG_START_GET(test_tunnel_csb_service_recv_data_ptr) ((((test_tunnel_csb_service_recv_data_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_MSG_START_SET(test_tunnel_csb_service_recv_data_ptr, msg_start) ((test_tunnel_csb_service_recv_data_ptr)->_data[1] =  \
                                                                                                               (uint32)(((test_tunnel_csb_service_recv_data_ptr)->_data[1] & ~0xffff) | (((msg_start)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_PAYLOAD_GET(test_tunnel_csb_service_recv_data_ptr) ((((test_tunnel_csb_service_recv_data_ptr)->_data[1] & 0xff0000) >> 16))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_PAYLOAD_SET(test_tunnel_csb_service_recv_data_ptr, payload) ((test_tunnel_csb_service_recv_data_ptr)->_data[1] =  \
                                                                                                           (uint32)(((test_tunnel_csb_service_recv_data_ptr)->_data[1] & ~0xff0000) | (((payload) << 16) & 0xff0000)))
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_CREATE(lt_addr, remote_addr, payload_length, msg_start, payload) \
    (uint32)(((lt_addr)) & 0xffff) | \
    (uint32) | \
    (uint32)(((payload_length) << 16) & 0xffff0000ul), \
    (uint32)(((msg_start)) & 0xffff) | \
    (uint32)(((payload) << 16) & 0xff0000)
#define TEST_TUNNEL_CSB_SERVICE_RECV_DATA_PACK(test_tunnel_csb_service_recv_data_ptr, lt_addr, remote_addr_ptr, payload_length, msg_start, payload) \
    do { \
        (test_tunnel_csb_service_recv_data_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff) | \
                                                                     | \
                                                                     (uint32)(((payload_length) << 16) & 0xffff0000ul)); \
        (test_tunnel_csb_service_recv_data_ptr)->_data[1] = (uint32)((uint32)(((msg_start)) & 0xffff) | \
                                                                     (uint32)(((payload) << 16) & 0xff0000)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_RX_START_REQ

  DESCRIPTION

  MEMBERS
    lt_addr     -
    remote_addr -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_CSB_SERVICE_RX_START_REQ;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_RX_START_REQ *test_tunnel_csb_service_rx_start_req_ptr */
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_LT_ADDR_GET(test_tunnel_csb_service_rx_start_req_ptr) ((((test_tunnel_csb_service_rx_start_req_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_LT_ADDR_SET(test_tunnel_csb_service_rx_start_req_ptr, lt_addr) ((test_tunnel_csb_service_rx_start_req_ptr)->_data[0] =  \
                                                                                                                 (uint32)(((test_tunnel_csb_service_rx_start_req_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_REMOTE_ADDR_GET(test_tunnel_csb_service_rx_start_req_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_REMOTE_ADDR_SET(test_tunnel_csb_service_rx_start_req_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_CREATE(lt_addr, remote_addr) \
    (uint32)(((lt_addr)) & 0xffff) | \
    (uint32)
#define TEST_TUNNEL_CSB_SERVICE_RX_START_REQ_PACK(test_tunnel_csb_service_rx_start_req_ptr, lt_addr, remote_addr_ptr) \
    do { \
        (test_tunnel_csb_service_rx_start_req_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff) | \
                                                                        ); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND

  DESCRIPTION

  MEMBERS
    lt_addr     -
    remote_addr -
    state       -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND *test_tunnel_csb_service_rx_state_ind_ptr */
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_LT_ADDR_GET(test_tunnel_csb_service_rx_state_ind_ptr) ((((test_tunnel_csb_service_rx_state_ind_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_LT_ADDR_SET(test_tunnel_csb_service_rx_state_ind_ptr, lt_addr) ((test_tunnel_csb_service_rx_state_ind_ptr)->_data[0] =  \
                                                                                                                 (uint32)(((test_tunnel_csb_service_rx_state_ind_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_REMOTE_ADDR_GET(test_tunnel_csb_service_rx_state_ind_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_REMOTE_ADDR_SET(test_tunnel_csb_service_rx_state_ind_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_STATE_GET(test_tunnel_csb_service_rx_state_ind_ptr) ((TEST_TUNNEL_CSB_SERVICE_STATE)(((test_tunnel_csb_service_rx_state_ind_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_STATE_SET(test_tunnel_csb_service_rx_state_ind_ptr, state) ((test_tunnel_csb_service_rx_state_ind_ptr)->_data[0] =  \
                                                                                                             (uint32)(((test_tunnel_csb_service_rx_state_ind_ptr)->_data[0] & ~0xffff0000ul) | (((state) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_CREATE(lt_addr, remote_addr, state) \
    (uint32)(((lt_addr)) & 0xffff) | \
    (uint32) | \
    (uint32)(((state) << 16) & 0xffff0000ul)
#define TEST_TUNNEL_CSB_SERVICE_RX_STATE_IND_PACK(test_tunnel_csb_service_rx_state_ind_ptr, lt_addr, remote_addr_ptr, state) \
    do { \
        (test_tunnel_csb_service_rx_state_ind_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff) | \
                                                                        | \
                                                                        (uint32)(((state) << 16) & 0xffff0000ul)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ

  DESCRIPTION

  MEMBERS
    lt_addr     -
    remote_addr -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ *test_tunnel_csb_service_rx_stop_req_ptr */
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_LT_ADDR_GET(test_tunnel_csb_service_rx_stop_req_ptr) ((((test_tunnel_csb_service_rx_stop_req_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_LT_ADDR_SET(test_tunnel_csb_service_rx_stop_req_ptr, lt_addr) ((test_tunnel_csb_service_rx_stop_req_ptr)->_data[0] =  \
                                                                                                               (uint32)(((test_tunnel_csb_service_rx_stop_req_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_REMOTE_ADDR_GET(test_tunnel_csb_service_rx_stop_req_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_REMOTE_ADDR_SET(test_tunnel_csb_service_rx_stop_req_ptr, remote_addr_ptr) do {  } while (0)
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_CREATE(lt_addr, remote_addr) \
    (uint32)(((lt_addr)) & 0xffff) | \
    (uint32)
#define TEST_TUNNEL_CSB_SERVICE_RX_STOP_REQ_PACK(test_tunnel_csb_service_rx_stop_req_ptr, lt_addr, remote_addr_ptr) \
    do { \
        (test_tunnel_csb_service_rx_stop_req_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff) | \
                                                                       ); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_SEND_DATA

  DESCRIPTION

  MEMBERS
    lt_addr        -
    payload_length -
    pkt_boundary   -
    payload        -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_CSB_SERVICE_SEND_DATA;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_SEND_DATA *test_tunnel_csb_service_send_data_ptr */
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_LT_ADDR_GET(test_tunnel_csb_service_send_data_ptr) ((((test_tunnel_csb_service_send_data_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_LT_ADDR_SET(test_tunnel_csb_service_send_data_ptr, lt_addr) ((test_tunnel_csb_service_send_data_ptr)->_data[0] =  \
                                                                                                           (uint32)(((test_tunnel_csb_service_send_data_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PAYLOAD_LENGTH_GET(test_tunnel_csb_service_send_data_ptr) ((((test_tunnel_csb_service_send_data_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PAYLOAD_LENGTH_SET(test_tunnel_csb_service_send_data_ptr, payload_length) ((test_tunnel_csb_service_send_data_ptr)->_data[0] =  \
                                                                                                                         (uint32)(((test_tunnel_csb_service_send_data_ptr)->_data[0] & ~0xffff0000ul) | (((payload_length) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PKT_BOUNDARY_WORD_OFFSET (1)
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PKT_BOUNDARY_GET(test_tunnel_csb_service_send_data_ptr) ((((test_tunnel_csb_service_send_data_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PKT_BOUNDARY_SET(test_tunnel_csb_service_send_data_ptr, pkt_boundary) ((test_tunnel_csb_service_send_data_ptr)->_data[1] =  \
                                                                                                                     (uint32)(((test_tunnel_csb_service_send_data_ptr)->_data[1] & ~0xffff) | (((pkt_boundary)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PAYLOAD_GET(test_tunnel_csb_service_send_data_ptr) ((((test_tunnel_csb_service_send_data_ptr)->_data[1] & 0xff0000) >> 16))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PAYLOAD_SET(test_tunnel_csb_service_send_data_ptr, payload) ((test_tunnel_csb_service_send_data_ptr)->_data[1] =  \
                                                                                                           (uint32)(((test_tunnel_csb_service_send_data_ptr)->_data[1] & ~0xff0000) | (((payload) << 16) & 0xff0000)))
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_CREATE(lt_addr, payload_length, pkt_boundary, payload) \
    (uint32)(((lt_addr)) & 0xffff) | \
    (uint32)(((payload_length) << 16) & 0xffff0000ul), \
    (uint32)(((pkt_boundary)) & 0xffff) | \
    (uint32)(((payload) << 16) & 0xff0000)
#define TEST_TUNNEL_CSB_SERVICE_SEND_DATA_PACK(test_tunnel_csb_service_send_data_ptr, lt_addr, payload_length, pkt_boundary, payload) \
    do { \
        (test_tunnel_csb_service_send_data_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff) | \
                                                                     (uint32)(((payload_length) << 16) & 0xffff0000ul)); \
        (test_tunnel_csb_service_send_data_ptr)->_data[1] = (uint32)((uint32)(((pkt_boundary)) & 0xffff) | \
                                                                     (uint32)(((payload) << 16) & 0xff0000)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_TX_START_REQ

  DESCRIPTION

  MEMBERS
    lt_addr -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_CSB_SERVICE_TX_START_REQ;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_TX_START_REQ *test_tunnel_csb_service_tx_start_req_ptr */
#define TEST_TUNNEL_CSB_SERVICE_TX_START_REQ_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_TX_START_REQ_LT_ADDR_GET(test_tunnel_csb_service_tx_start_req_ptr) ((((test_tunnel_csb_service_tx_start_req_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_TX_START_REQ_LT_ADDR_SET(test_tunnel_csb_service_tx_start_req_ptr, lt_addr) ((test_tunnel_csb_service_tx_start_req_ptr)->_data[0] =  \
                                                                                                                 (uint32)(((test_tunnel_csb_service_tx_start_req_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_TX_START_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_TX_START_REQ_CREATE(lt_addr) \
    (uint32)(((lt_addr)) & 0xffff)
#define TEST_TUNNEL_CSB_SERVICE_TX_START_REQ_PACK(test_tunnel_csb_service_tx_start_req_ptr, lt_addr) \
    do { \
        (test_tunnel_csb_service_tx_start_req_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND

  DESCRIPTION

  MEMBERS
    lt_addr -
    state   -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND *test_tunnel_csb_service_tx_state_ind_ptr */
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_LT_ADDR_GET(test_tunnel_csb_service_tx_state_ind_ptr) ((((test_tunnel_csb_service_tx_state_ind_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_LT_ADDR_SET(test_tunnel_csb_service_tx_state_ind_ptr, lt_addr) ((test_tunnel_csb_service_tx_state_ind_ptr)->_data[0] =  \
                                                                                                                 (uint32)(((test_tunnel_csb_service_tx_state_ind_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_STATE_GET(test_tunnel_csb_service_tx_state_ind_ptr) ((TEST_TUNNEL_CSB_SERVICE_STATE)(((test_tunnel_csb_service_tx_state_ind_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_STATE_SET(test_tunnel_csb_service_tx_state_ind_ptr, state) ((test_tunnel_csb_service_tx_state_ind_ptr)->_data[0] =  \
                                                                                                             (uint32)(((test_tunnel_csb_service_tx_state_ind_ptr)->_data[0] & ~0xffff0000ul) | (((state) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_CREATE(lt_addr, state) \
    (uint32)(((lt_addr)) & 0xffff) | \
    (uint32)(((state) << 16) & 0xffff0000ul)
#define TEST_TUNNEL_CSB_SERVICE_TX_STATE_IND_PACK(test_tunnel_csb_service_tx_state_ind_ptr, lt_addr, state) \
    do { \
        (test_tunnel_csb_service_tx_state_ind_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff) | \
                                                                        (uint32)(((state) << 16) & 0xffff0000ul)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ

  DESCRIPTION

  MEMBERS
    lt_addr -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ;

/* The following macros take TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ *test_tunnel_csb_service_tx_stop_req_ptr */
#define TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ_LT_ADDR_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ_LT_ADDR_GET(test_tunnel_csb_service_tx_stop_req_ptr) ((((test_tunnel_csb_service_tx_stop_req_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ_LT_ADDR_SET(test_tunnel_csb_service_tx_stop_req_ptr, lt_addr) ((test_tunnel_csb_service_tx_stop_req_ptr)->_data[0] =  \
                                                                                                               (uint32)(((test_tunnel_csb_service_tx_stop_req_ptr)->_data[0] & ~0xffff) | (((lt_addr)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ_CREATE(lt_addr) \
    (uint32)(((lt_addr)) & 0xffff)
#define TEST_TUNNEL_CSB_SERVICE_TX_STOP_REQ_PACK(test_tunnel_csb_service_tx_stop_req_ptr, lt_addr) \
    do { \
        (test_tunnel_csb_service_tx_stop_req_ptr)->_data[0] = (uint32)((uint32)(((lt_addr)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_GENERIC_PAYLOAD

  DESCRIPTION

  MEMBERS
    data   -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_GENERIC_PAYLOAD;

/* The following macros take TEST_TUNNEL_GENERIC_PAYLOAD *test_tunnel_generic_payload_ptr */
#define TEST_TUNNEL_GENERIC_PAYLOAD_DATA_WORD_OFFSET (0)
#define TEST_TUNNEL_GENERIC_PAYLOAD_DATA_GET(test_tunnel_generic_payload_ptr) ((((test_tunnel_generic_payload_ptr)->_data[0] & 0xff)))
#define TEST_TUNNEL_GENERIC_PAYLOAD_DATA_SET(test_tunnel_generic_payload_ptr, data) ((test_tunnel_generic_payload_ptr)->_data[0] =  \
                                                                                         (uint32)(((test_tunnel_generic_payload_ptr)->_data[0] & ~0xff) | (((data)) & 0xff)))
#define TEST_TUNNEL_GENERIC_PAYLOAD_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_GENERIC_PAYLOAD_CREATE(data) \
    (uint32)(((data)) & 0xff)
#define TEST_TUNNEL_GENERIC_PAYLOAD_PACK(test_tunnel_generic_payload_ptr, data) \
    do { \
        (test_tunnel_generic_payload_ptr)->_data[0] = (uint32)((uint32)(((data)) & 0xff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_HOST_COMMS

  DESCRIPTION

  MEMBERS
    length   -
    sub_type -
    data     -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_HOST_COMMS;

/* The following macros take TEST_TUNNEL_HOST_COMMS *test_tunnel_host_comms_ptr */
#define TEST_TUNNEL_HOST_COMMS_LENGTH_WORD_OFFSET (0)
#define TEST_TUNNEL_HOST_COMMS_LENGTH_GET(test_tunnel_host_comms_ptr) ((((test_tunnel_host_comms_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_HOST_COMMS_LENGTH_SET(test_tunnel_host_comms_ptr, length) ((test_tunnel_host_comms_ptr)->_data[0] =  \
                                                                                   (uint32)(((test_tunnel_host_comms_ptr)->_data[0] & ~0xffff) | (((length)) & 0xffff)))
#define TEST_TUNNEL_HOST_COMMS_SUB_TYPE_GET(test_tunnel_host_comms_ptr) ((((test_tunnel_host_comms_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_HOST_COMMS_SUB_TYPE_SET(test_tunnel_host_comms_ptr, sub_type) ((test_tunnel_host_comms_ptr)->_data[0] =  \
                                                                                       (uint32)(((test_tunnel_host_comms_ptr)->_data[0] & ~0xffff0000ul) | (((sub_type) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_HOST_COMMS_DATA_WORD_OFFSET (1)
#define TEST_TUNNEL_HOST_COMMS_DATA_GET(test_tunnel_host_comms_ptr) ((((test_tunnel_host_comms_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_HOST_COMMS_DATA_SET(test_tunnel_host_comms_ptr, data) ((test_tunnel_host_comms_ptr)->_data[1] =  \
                                                                               (uint32)(((test_tunnel_host_comms_ptr)->_data[1] & ~0xffff) | (((data)) & 0xffff)))
#define TEST_TUNNEL_HOST_COMMS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_HOST_COMMS_CREATE(length, sub_type, data) \
    (uint32)(((length)) & 0xffff) | \
    (uint32)(((sub_type) << 16) & 0xffff0000ul), \
    (uint32)(((data)) & 0xffff)
#define TEST_TUNNEL_HOST_COMMS_PACK(test_tunnel_host_comms_ptr, length, sub_type, data) \
    do { \
        (test_tunnel_host_comms_ptr)->_data[0] = (uint32)((uint32)(((length)) & 0xffff) | \
                                                          (uint32)(((sub_type) << 16) & 0xffff0000ul)); \
        (test_tunnel_host_comms_ptr)->_data[1] = (uint32)((uint32)(((data)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_PRODUCTION_TEST

  DESCRIPTION

  MEMBERS
    command_id     -
    payload_length -
    seq_num        -
    status         -
    payload        -

*******************************************************************************/
typedef struct
{
    uint32 _data[3];
} TEST_TUNNEL_PRODUCTION_TEST;

/* The following macros take TEST_TUNNEL_PRODUCTION_TEST *test_tunnel_production_test_ptr */
#define TEST_TUNNEL_PRODUCTION_TEST_COMMAND_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_PRODUCTION_TEST_COMMAND_ID_GET(test_tunnel_production_test_ptr) ((TEST_TUNNEL_PRODUCTION_TEST_COMMANDS)(((test_tunnel_production_test_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_PRODUCTION_TEST_COMMAND_ID_SET(test_tunnel_production_test_ptr, command_id) ((test_tunnel_production_test_ptr)->_data[0] =  \
                                                                                                     (uint32)(((test_tunnel_production_test_ptr)->_data[0] & ~0xffff) | (((command_id)) & 0xffff)))
#define TEST_TUNNEL_PRODUCTION_TEST_PAYLOAD_LENGTH_GET(test_tunnel_production_test_ptr) ((((test_tunnel_production_test_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_PRODUCTION_TEST_PAYLOAD_LENGTH_SET(test_tunnel_production_test_ptr, payload_length) ((test_tunnel_production_test_ptr)->_data[0] =  \
                                                                                                             (uint32)(((test_tunnel_production_test_ptr)->_data[0] & ~0xffff0000ul) | (((payload_length) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_PRODUCTION_TEST_SEQ_NUM_WORD_OFFSET (1)
#define TEST_TUNNEL_PRODUCTION_TEST_SEQ_NUM_GET(test_tunnel_production_test_ptr) ((((test_tunnel_production_test_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_PRODUCTION_TEST_SEQ_NUM_SET(test_tunnel_production_test_ptr, seq_num) ((test_tunnel_production_test_ptr)->_data[1] =  \
                                                                                               (uint32)(((test_tunnel_production_test_ptr)->_data[1] & ~0xffff) | (((seq_num)) & 0xffff)))
#define TEST_TUNNEL_PRODUCTION_TEST_STATUS_GET(test_tunnel_production_test_ptr) ((((test_tunnel_production_test_ptr)->_data[1] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_PRODUCTION_TEST_STATUS_SET(test_tunnel_production_test_ptr, status) ((test_tunnel_production_test_ptr)->_data[1] =  \
                                                                                             (uint32)(((test_tunnel_production_test_ptr)->_data[1] & ~0xffff0000ul) | (((status) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_PRODUCTION_TEST_PAYLOAD_WORD_OFFSET (2)
#define TEST_TUNNEL_PRODUCTION_TEST_PAYLOAD_GET(test_tunnel_production_test_ptr) ((((test_tunnel_production_test_ptr)->_data[2] & 0xff)))
#define TEST_TUNNEL_PRODUCTION_TEST_PAYLOAD_SET(test_tunnel_production_test_ptr, payload) ((test_tunnel_production_test_ptr)->_data[2] =  \
                                                                                               (uint32)(((test_tunnel_production_test_ptr)->_data[2] & ~0xff) | (((payload)) & 0xff)))
#define TEST_TUNNEL_PRODUCTION_TEST_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_PRODUCTION_TEST_CREATE(command_id, payload_length, seq_num, status, payload) \
    (uint32)(((command_id)) & 0xffff) | \
    (uint32)(((payload_length) << 16) & 0xffff0000ul), \
    (uint32)(((seq_num)) & 0xffff) | \
    (uint32)(((status) << 16) & 0xffff0000ul), \
    (uint32)(((payload)) & 0xff)
#define TEST_TUNNEL_PRODUCTION_TEST_PACK(test_tunnel_production_test_ptr, command_id, payload_length, seq_num, status, payload) \
    do { \
        (test_tunnel_production_test_ptr)->_data[0] = (uint32)((uint32)(((command_id)) & 0xffff) | \
                                                               (uint32)(((payload_length) << 16) & 0xffff0000ul)); \
        (test_tunnel_production_test_ptr)->_data[1] = (uint32)((uint32)(((seq_num)) & 0xffff) | \
                                                               (uint32)(((status) << 16) & 0xffff0000ul)); \
        (test_tunnel_production_test_ptr)->_data[2] = (uint32)((uint32)(((payload)) & 0xff)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Interrupt_Cpu_Usage

  DESCRIPTION

  MEMBERS
    current_time             -
    total_shallow_sleep_time -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE;

/* The following macros take TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE *test_tunnel_apps_fw_interrupt_cpu_usage_ptr */
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_CURRENT_TIME_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_CURRENT_TIME_GET(test_tunnel_apps_fw_interrupt_cpu_usage_ptr) ((test_tunnel_apps_fw_interrupt_cpu_usage_ptr)->_data[0])
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_CURRENT_TIME_SET(test_tunnel_apps_fw_interrupt_cpu_usage_ptr, current_time) ((test_tunnel_apps_fw_interrupt_cpu_usage_ptr)->_data[0] = (uint32)(current_time))
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_TOTAL_SHALLOW_SLEEP_TIME_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_TOTAL_SHALLOW_SLEEP_TIME_GET(test_tunnel_apps_fw_interrupt_cpu_usage_ptr) ((test_tunnel_apps_fw_interrupt_cpu_usage_ptr)->_data[1])
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_TOTAL_SHALLOW_SLEEP_TIME_SET(test_tunnel_apps_fw_interrupt_cpu_usage_ptr, total_shallow_sleep_time) ((test_tunnel_apps_fw_interrupt_cpu_usage_ptr)->_data[1] = (uint32)(total_shallow_sleep_time))
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_CREATE(current_time, total_shallow_sleep_time) \
    (uint32)(current_time), \
    (uint32)(total_shallow_sleep_time)
#define TEST_TUNNEL_APPS_FW_INTERRUPT_CPU_USAGE_PACK(test_tunnel_apps_fw_interrupt_cpu_usage_ptr, current_time, total_shallow_sleep_time) \
    do { \
        (test_tunnel_apps_fw_interrupt_cpu_usage_ptr)->_data[0] = (uint32)((uint32)(current_time)); \
        (test_tunnel_apps_fw_interrupt_cpu_usage_ptr)->_data[1] = (uint32)((uint32)(total_shallow_sleep_time)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_SD_MMC_Cmd_Rsp

  DESCRIPTION

  MEMBERS
    cmd_id -
    result -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP;

/* The following macros take TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP *test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr */
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_CMD_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_CMD_ID_GET(test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr) ((test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr)->_data[0])
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_CMD_ID_SET(test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr, cmd_id) ((test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr)->_data[0] = (uint32)(cmd_id))
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_RESULT_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_RESULT_GET(test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr) ((TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RESULT)(test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr)->_data[1])
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_RESULT_SET(test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr, result) ((test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr)->_data[1] = (uint32)(result))
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_CREATE(cmd_id, result) \
    (uint32)(cmd_id), \
    (uint32)(result)
#define TEST_TUNNEL_APPS_FW_SD_MMC_CMD_RSP_PACK(test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr, cmd_id, result) \
    do { \
        (test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr)->_data[0] = (uint32)((uint32)(cmd_id)); \
        (test_tunnel_apps_fw_sd_mmc_cmd_rsp_ptr)->_data[1] = (uint32)((uint32)(result)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_SD_MMC_Data_Req

  DESCRIPTION

  MEMBERS
    cmd_id       -
    slot         -
    block_number -
    blocks_count -
    data_buffer  -

*******************************************************************************/
typedef struct
{
    uint32 _data[5];
} TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ;

/* The following macros take TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ *test_tunnel_apps_fw_sd_mmc_data_req_ptr */
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_CMD_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_CMD_ID_GET(test_tunnel_apps_fw_sd_mmc_data_req_ptr) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[0])
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_CMD_ID_SET(test_tunnel_apps_fw_sd_mmc_data_req_ptr, cmd_id) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[0] = (uint32)(cmd_id))
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_SLOT_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_SLOT_GET(test_tunnel_apps_fw_sd_mmc_data_req_ptr) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[1])
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_SLOT_SET(test_tunnel_apps_fw_sd_mmc_data_req_ptr, slot) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[1] = (uint32)(slot))
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_BLOCK_NUMBER_WORD_OFFSET (2)
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_BLOCK_NUMBER_GET(test_tunnel_apps_fw_sd_mmc_data_req_ptr) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[2])
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_BLOCK_NUMBER_SET(test_tunnel_apps_fw_sd_mmc_data_req_ptr, block_number) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[2] = (uint32)(block_number))
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_BLOCKS_COUNT_WORD_OFFSET (3)
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_BLOCKS_COUNT_GET(test_tunnel_apps_fw_sd_mmc_data_req_ptr) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[3])
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_BLOCKS_COUNT_SET(test_tunnel_apps_fw_sd_mmc_data_req_ptr, blocks_count) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[3] = (uint32)(blocks_count))
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_DATA_BUFFER_WORD_OFFSET (4)
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_DATA_BUFFER_GET(test_tunnel_apps_fw_sd_mmc_data_req_ptr) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[4])
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_DATA_BUFFER_SET(test_tunnel_apps_fw_sd_mmc_data_req_ptr, data_buffer) ((test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[4] = (uint32)(data_buffer))
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_CREATE(cmd_id, slot, block_number, blocks_count, data_buffer) \
    (uint32)(cmd_id), \
    (uint32)(slot), \
    (uint32)(block_number), \
    (uint32)(blocks_count), \
    (uint32)(data_buffer)
#define TEST_TUNNEL_APPS_FW_SD_MMC_DATA_REQ_PACK(test_tunnel_apps_fw_sd_mmc_data_req_ptr, cmd_id, slot, block_number, blocks_count, data_buffer) \
    do { \
        (test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[0] = (uint32)((uint32)(cmd_id)); \
        (test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[1] = (uint32)((uint32)(slot)); \
        (test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[2] = (uint32)((uint32)(block_number)); \
        (test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[3] = (uint32)((uint32)(blocks_count)); \
        (test_tunnel_apps_fw_sd_mmc_data_req_ptr)->_data[4] = (uint32)((uint32)(data_buffer)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_SD_MMC_Erase_Req

  DESCRIPTION

  MEMBERS
    cmd_id            -
    slot              -
    from_block_number -
    to_block_number   -

*******************************************************************************/
typedef struct
{
    uint32 _data[4];
} TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ;

/* The following macros take TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ *test_tunnel_apps_fw_sd_mmc_erase_req_ptr */
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_CMD_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_CMD_ID_GET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[0])
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_CMD_ID_SET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr, cmd_id) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[0] = (uint32)(cmd_id))
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_SLOT_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_SLOT_GET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[1])
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_SLOT_SET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr, slot) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[1] = (uint32)(slot))
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_FROM_BLOCK_NUMBER_WORD_OFFSET (2)
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_FROM_BLOCK_NUMBER_GET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[2])
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_FROM_BLOCK_NUMBER_SET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr, from_block_number) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[2] = (uint32)(from_block_number))
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_TO_BLOCK_NUMBER_WORD_OFFSET (3)
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_TO_BLOCK_NUMBER_GET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[3])
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_TO_BLOCK_NUMBER_SET(test_tunnel_apps_fw_sd_mmc_erase_req_ptr, to_block_number) ((test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[3] = (uint32)(to_block_number))
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_CREATE(cmd_id, slot, from_block_number, to_block_number) \
    (uint32)(cmd_id), \
    (uint32)(slot), \
    (uint32)(from_block_number), \
    (uint32)(to_block_number)
#define TEST_TUNNEL_APPS_FW_SD_MMC_ERASE_REQ_PACK(test_tunnel_apps_fw_sd_mmc_erase_req_ptr, cmd_id, slot, from_block_number, to_block_number) \
    do { \
        (test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[0] = (uint32)((uint32)(cmd_id)); \
        (test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[1] = (uint32)((uint32)(slot)); \
        (test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[2] = (uint32)((uint32)(from_block_number)); \
        (test_tunnel_apps_fw_sd_mmc_erase_req_ptr)->_data[3] = (uint32)((uint32)(to_block_number)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Siflash_Cmd_Rsp

  DESCRIPTION

  MEMBERS
    cmd_id -
    result -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP;

/* The following macros take TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP *test_tunnel_apps_fw_siflash_cmd_rsp_ptr */
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_CMD_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_CMD_ID_GET(test_tunnel_apps_fw_siflash_cmd_rsp_ptr) ((test_tunnel_apps_fw_siflash_cmd_rsp_ptr)->_data[0])
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_CMD_ID_SET(test_tunnel_apps_fw_siflash_cmd_rsp_ptr, cmd_id) ((test_tunnel_apps_fw_siflash_cmd_rsp_ptr)->_data[0] = (uint32)(cmd_id))
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_RESULT_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_RESULT_GET(test_tunnel_apps_fw_siflash_cmd_rsp_ptr) ((TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RESULT)(test_tunnel_apps_fw_siflash_cmd_rsp_ptr)->_data[1])
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_RESULT_SET(test_tunnel_apps_fw_siflash_cmd_rsp_ptr, result) ((test_tunnel_apps_fw_siflash_cmd_rsp_ptr)->_data[1] = (uint32)(result))
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_CREATE(cmd_id, result) \
    (uint32)(cmd_id), \
    (uint32)(result)
#define TEST_TUNNEL_APPS_FW_SIFLASH_CMD_RSP_PACK(test_tunnel_apps_fw_siflash_cmd_rsp_ptr, cmd_id, result) \
    do { \
        (test_tunnel_apps_fw_siflash_cmd_rsp_ptr)->_data[0] = (uint32)((uint32)(cmd_id)); \
        (test_tunnel_apps_fw_siflash_cmd_rsp_ptr)->_data[1] = (uint32)((uint32)(result)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Siflash_Erase_Req

  DESCRIPTION

  MEMBERS
    cmd_id    -
    interface -
    from_addr -
    to_addr   -

*******************************************************************************/
typedef struct
{
    uint32 _data[4];
} TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ;

/* The following macros take TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ *test_tunnel_apps_fw_siflash_erase_req_ptr */
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_CMD_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_CMD_ID_GET(test_tunnel_apps_fw_siflash_erase_req_ptr) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[0])
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_CMD_ID_SET(test_tunnel_apps_fw_siflash_erase_req_ptr, cmd_id) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[0] = (uint32)(cmd_id))
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_INTERFACE_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_INTERFACE_GET(test_tunnel_apps_fw_siflash_erase_req_ptr) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[1])
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_INTERFACE_SET(test_tunnel_apps_fw_siflash_erase_req_ptr, interface) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[1] = (uint32)(interface))
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_FROM_ADDR_WORD_OFFSET (2)
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_FROM_ADDR_GET(test_tunnel_apps_fw_siflash_erase_req_ptr) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[2])
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_FROM_ADDR_SET(test_tunnel_apps_fw_siflash_erase_req_ptr, from_addr) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[2] = (uint32)(from_addr))
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_TO_ADDR_WORD_OFFSET (3)
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_TO_ADDR_GET(test_tunnel_apps_fw_siflash_erase_req_ptr) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[3])
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_TO_ADDR_SET(test_tunnel_apps_fw_siflash_erase_req_ptr, to_addr) ((test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[3] = (uint32)(to_addr))
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_CREATE(cmd_id, interface, from_addr, to_addr) \
    (uint32)(cmd_id), \
    (uint32)(interface), \
    (uint32)(from_addr), \
    (uint32)(to_addr)
#define TEST_TUNNEL_APPS_FW_SIFLASH_ERASE_REQ_PACK(test_tunnel_apps_fw_siflash_erase_req_ptr, cmd_id, interface, from_addr, to_addr) \
    do { \
        (test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[0] = (uint32)((uint32)(cmd_id)); \
        (test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[1] = (uint32)((uint32)(interface)); \
        (test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[2] = (uint32)((uint32)(from_addr)); \
        (test_tunnel_apps_fw_siflash_erase_req_ptr)->_data[3] = (uint32)((uint32)(to_addr)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_FW_INTERRUPT

  DESCRIPTION

  MEMBERS
    id     -
    params -

*******************************************************************************/
typedef struct
{
    uint32 _data[3];
} TEST_TUNNEL_FW_INTERRUPT;

/* The following macros take TEST_TUNNEL_FW_INTERRUPT *test_tunnel_fw_interrupt_ptr */
#define TEST_TUNNEL_FW_INTERRUPT_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_FW_INTERRUPT_ID_GET(test_tunnel_fw_interrupt_ptr) ((TEST_TUNNEL_APPS_FW_INTERRUPT_ID)(test_tunnel_fw_interrupt_ptr)->_data[0])
#define TEST_TUNNEL_FW_INTERRUPT_ID_SET(test_tunnel_fw_interrupt_ptr, id) ((test_tunnel_fw_interrupt_ptr)->_data[0] = (uint32)(id))
#define TEST_TUNNEL_FW_INTERRUPT_PARAMS_WORD_OFFSET (1)
#define TEST_TUNNEL_FW_INTERRUPT_PARAMS_GET(test_tunnel_fw_interrupt_ptr, params_ptr) do {  \
        (params_ptr)->_data[0] = (test_tunnel_fw_interrupt_ptr)->_data[1]; \
        (params_ptr)->_data[1] = (test_tunnel_fw_interrupt_ptr)->_data[2]; } while (0)
#define TEST_TUNNEL_FW_INTERRUPT_PARAMS_SET(test_tunnel_fw_interrupt_ptr, params_ptr) do {  \
        (test_tunnel_fw_interrupt_ptr)->_data[1] = (params_ptr)->_data[0]; \
        (test_tunnel_fw_interrupt_ptr)->_data[2] = (params_ptr)->_data[1]; } while (0)
#define TEST_TUNNEL_FW_INTERRUPT_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_FW_INTERRUPT_CREATE(id, params) \
    (uint32)(id), \
    (uint32)
#define TEST_TUNNEL_FW_INTERRUPT_PACK(test_tunnel_fw_interrupt_ptr, id, params_ptr) \
    do { \
        (test_tunnel_fw_interrupt_ptr)->_data[0] = (uint32)((uint32)(id)); \
        (test_tunnel_fw_interrupt_ptr)->_data[1] = (uint32)((params_ptr)->_data[0]); \
        (test_tunnel_fw_interrupt_ptr)->_data[2] = (uint32)((params_ptr)->_data[1]); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Header

  DESCRIPTION

  MEMBERS
    Tunnel_Id     -
    reserved      -
    padding_bytes -
    reserved2     -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_HEADER;

/* The following macros take TEST_TUNNEL_HEADER *test_tunnel_header_ptr */
#define TEST_TUNNEL_HEADER_TUNNEL_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_HEADER_TUNNEL_ID_GET(test_tunnel_header_ptr) ((TEST_TUNNEL_ID)(((test_tunnel_header_ptr)->_data[0] & 0xff)))
#define TEST_TUNNEL_HEADER_TUNNEL_ID_SET(test_tunnel_header_ptr, tunnel_id) ((test_tunnel_header_ptr)->_data[0] =  \
                                                                                 (uint32)(((test_tunnel_header_ptr)->_data[0] & ~0xff) | (((tunnel_id)) & 0xff)))
#define TEST_TUNNEL_HEADER_RESERVED_GET(test_tunnel_header_ptr) ((((test_tunnel_header_ptr)->_data[0] & 0xff00) >> 8))
#define TEST_TUNNEL_HEADER_RESERVED_SET(test_tunnel_header_ptr, reserved) ((test_tunnel_header_ptr)->_data[0] =  \
                                                                               (uint32)(((test_tunnel_header_ptr)->_data[0] & ~0xff00) | (((reserved) << 8) & 0xff00)))
#define TEST_TUNNEL_HEADER_PADDING_BYTES_GET(test_tunnel_header_ptr) ((((test_tunnel_header_ptr)->_data[0] & 0x30000) >> 16))
#define TEST_TUNNEL_HEADER_PADDING_BYTES_SET(test_tunnel_header_ptr, padding_bytes) ((test_tunnel_header_ptr)->_data[0] =  \
                                                                                         (uint32)(((test_tunnel_header_ptr)->_data[0] & ~0x30000) | (((padding_bytes) << 16) & 0x30000)))
#define TEST_TUNNEL_HEADER_RESERVED2_GET(test_tunnel_header_ptr) ((((test_tunnel_header_ptr)->_data[0] & 0xfffc0000ul) >> 18))
#define TEST_TUNNEL_HEADER_RESERVED2_SET(test_tunnel_header_ptr, reserved2) ((test_tunnel_header_ptr)->_data[0] =  \
                                                                                 (uint32)(((test_tunnel_header_ptr)->_data[0] & ~0xfffc0000ul) | (((reserved2) << 18) & 0xfffc0000ul)))
#define TEST_TUNNEL_HEADER_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_HEADER_CREATE(Tunnel_Id, reserved, padding_bytes, reserved2) \
    (uint32)(((Tunnel_Id)) & 0xff) | \
    (uint32)(((reserved) << 8) & 0xff00) | \
    (uint32)(((padding_bytes) << 16) & 0x30000) | \
    (uint32)(((reserved2) << 18) & 0xfffc0000ul)
#define TEST_TUNNEL_HEADER_PACK(test_tunnel_header_ptr, Tunnel_Id, reserved, padding_bytes, reserved2) \
    do { \
        (test_tunnel_header_ptr)->_data[0] = (uint32)((uint32)(((Tunnel_Id)) & 0xff) | \
                                                      (uint32)(((reserved) << 8) & 0xff00) | \
                                                      (uint32)(((padding_bytes) << 16) & 0x30000) | \
                                                      (uint32)(((reserved2) << 18) & 0xfffc0000ul)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_SD_MMC

  DESCRIPTION

  MEMBERS
    type    -
    payload -

*******************************************************************************/
typedef struct
{
    uint32 _data[6];
} TEST_TUNNEL_SD_MMC;

/* The following macros take TEST_TUNNEL_SD_MMC *test_tunnel_sd_mmc_ptr */
#define TEST_TUNNEL_SD_MMC_TYPE_WORD_OFFSET (0)
#define TEST_TUNNEL_SD_MMC_TYPE_GET(test_tunnel_sd_mmc_ptr) ((TEST_TUNNEL_APPS_FW_SD_MMC_CMD_TYPE)(test_tunnel_sd_mmc_ptr)->_data[0])
#define TEST_TUNNEL_SD_MMC_TYPE_SET(test_tunnel_sd_mmc_ptr, type) ((test_tunnel_sd_mmc_ptr)->_data[0] = (uint32)(type))
#define TEST_TUNNEL_SD_MMC_PAYLOAD_WORD_OFFSET (1)
#define TEST_TUNNEL_SD_MMC_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_SD_MMC_CREATE(type) \
    (uint32)(type)
#define TEST_TUNNEL_SD_MMC_PACK(test_tunnel_sd_mmc_ptr, type) \
    do { \
        (test_tunnel_sd_mmc_ptr)->_data[0] = (uint32)((uint32)(type)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_TRANS_AUDIO_TEST

  DESCRIPTION

  MEMBERS
    id         -
    parameters -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_TRANS_AUDIO_TEST;

/* The following macros take TEST_TUNNEL_TRANS_AUDIO_TEST *test_tunnel_trans_audio_test_ptr */
#define TEST_TUNNEL_TRANS_AUDIO_TEST_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_TRANS_AUDIO_TEST_ID_GET(test_tunnel_trans_audio_test_ptr) ((TEST_TUNNEL_APPS_FW_TRANS_AUDIO_TEST_ID)(test_tunnel_trans_audio_test_ptr)->_data[0])
#define TEST_TUNNEL_TRANS_AUDIO_TEST_ID_SET(test_tunnel_trans_audio_test_ptr, id) ((test_tunnel_trans_audio_test_ptr)->_data[0] = (uint32)(id))
#define TEST_TUNNEL_TRANS_AUDIO_TEST_PARAMETERS_WORD_OFFSET (1)
#define TEST_TUNNEL_TRANS_AUDIO_TEST_PARAMETERS_GET(test_tunnel_trans_audio_test_ptr) ((test_tunnel_trans_audio_test_ptr)->_data[1])
#define TEST_TUNNEL_TRANS_AUDIO_TEST_PARAMETERS_SET(test_tunnel_trans_audio_test_ptr, parameters) ((test_tunnel_trans_audio_test_ptr)->_data[1] = (uint32)(parameters))
#define TEST_TUNNEL_TRANS_AUDIO_TEST_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_TRANS_AUDIO_TEST_CREATE(id, parameters) \
    (uint32)(id), \
    (uint32)(parameters)
#define TEST_TUNNEL_TRANS_AUDIO_TEST_PACK(test_tunnel_trans_audio_test_ptr, id, parameters) \
    do { \
        (test_tunnel_trans_audio_test_ptr)->_data[0] = (uint32)((uint32)(id)); \
        (test_tunnel_trans_audio_test_ptr)->_data[1] = (uint32)((uint32)(parameters)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_TRANS_BT_TEST_ACL_DMA

  DESCRIPTION

  MEMBERS
    handle_plus_flags -

*******************************************************************************/
typedef struct
{
    uint32 _data[1];
} TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA;

/* The following macros take TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA *test_tunnel_trans_bt_test_acl_dma_ptr */
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_HANDLE_PLUS_FLAGS_WORD_OFFSET (0)
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_HANDLE_PLUS_FLAGS_GET(test_tunnel_trans_bt_test_acl_dma_ptr) ((((test_tunnel_trans_bt_test_acl_dma_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_HANDLE_PLUS_FLAGS_SET(test_tunnel_trans_bt_test_acl_dma_ptr, handle_plus_flags) ((test_tunnel_trans_bt_test_acl_dma_ptr)->_data[0] =  \
                                                                                                                               (uint32)(((test_tunnel_trans_bt_test_acl_dma_ptr)->_data[0] & ~0xffff) | (((handle_plus_flags)) & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_CREATE(handle_plus_flags) \
    (uint32)(((handle_plus_flags)) & 0xffff)
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_PACK(test_tunnel_trans_bt_test_acl_dma_ptr, handle_plus_flags) \
    do { \
        (test_tunnel_trans_bt_test_acl_dma_ptr)->_data[0] = (uint32)((uint32)(((handle_plus_flags)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_TRANS_BT_TEST_ACL_DMA_RESULT

  DESCRIPTION

  MEMBERS
    response -
    pass     -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT;

/* The following macros take TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT *test_tunnel_trans_bt_test_acl_dma_result_ptr */
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_RESPONSE_WORD_OFFSET (0)
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_RESPONSE_GET(test_tunnel_trans_bt_test_acl_dma_result_ptr) ((TEST_TUNNEL_RESPONSE_CODE)(((test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_RESPONSE_SET(test_tunnel_trans_bt_test_acl_dma_result_ptr, response) ((test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[0] =  \
                                                                                                                           (uint32)(((test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[0] & ~0xffff) | (((response)) & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_PASS_GET(test_tunnel_trans_bt_test_acl_dma_result_ptr) (((((test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[0] & 0xffff0000) >> 16) |  \
                                                                                                          (((test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[1] & 0xffff) << (32 - 16))))
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_PASS_SET(test_tunnel_trans_bt_test_acl_dma_result_ptr, pass) do { \
        (test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[0] = (uint32)(((test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[0] & ~0xffff0000) | (((pass) << 16) & 0xffff0000)); \
        (test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[1] = (uint32)(((test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[1] & ~0xffff) | (((pass) >> (32 - 16)) & 0xffff)); } while (0)
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_CREATE(response, pass) \
    (uint32)(((response)) & 0xffff) | \
    (uint32)(((pass) << 16) & 0xffff0000), \
    (uint32)(((pass) >> (32 - 16)) & 0xffff)
#define TEST_TUNNEL_TRANS_BT_TEST_ACL_DMA_RESULT_PACK(test_tunnel_trans_bt_test_acl_dma_result_ptr, response, pass) \
    do { \
        (test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[0] = (uint32)((uint32)(((response)) & 0xffff) | \
                                                                            (uint32)(((pass) << 16) & 0xffff0000)); \
        (test_tunnel_trans_bt_test_acl_dma_result_ptr)->_data[1] = (uint32)((((pass) >> (32 - 16)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_TRANS_BT_THROUGHPUT_TEST_PARAMS

  DESCRIPTION

  MEMBERS
    handle_plus_flags   -
    transfer_size_kbyte -
    pkt_size_bytes      -
    outstanding_pkts    -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS;

/* The following macros take TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS *test_tunnel_trans_bt_throughput_test_params_ptr */
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_HANDLE_PLUS_FLAGS_WORD_OFFSET (0)
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_HANDLE_PLUS_FLAGS_GET(test_tunnel_trans_bt_throughput_test_params_ptr) ((((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_HANDLE_PLUS_FLAGS_SET(test_tunnel_trans_bt_throughput_test_params_ptr, handle_plus_flags) ((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[0] =  \
                                                                                                                                                   (uint32)(((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[0] & ~0xffff) | (((handle_plus_flags)) & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_TRANSFER_SIZE_KBYTE_GET(test_tunnel_trans_bt_throughput_test_params_ptr) ((((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_TRANSFER_SIZE_KBYTE_SET(test_tunnel_trans_bt_throughput_test_params_ptr, transfer_size_kbyte) ((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[0] =  \
                                                                                                                                                       (uint32)(((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[0] & ~0xffff0000ul) | (((transfer_size_kbyte) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_PKT_SIZE_BYTES_WORD_OFFSET (1)
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_PKT_SIZE_BYTES_GET(test_tunnel_trans_bt_throughput_test_params_ptr) ((((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[1] & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_PKT_SIZE_BYTES_SET(test_tunnel_trans_bt_throughput_test_params_ptr, pkt_size_bytes) ((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[1] =  \
                                                                                                                                             (uint32)(((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[1] & ~0xffff) | (((pkt_size_bytes)) & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_OUTSTANDING_PKTS_GET(test_tunnel_trans_bt_throughput_test_params_ptr) ((((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[1] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_OUTSTANDING_PKTS_SET(test_tunnel_trans_bt_throughput_test_params_ptr, outstanding_pkts) ((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[1] =  \
                                                                                                                                                 (uint32)(((test_tunnel_trans_bt_throughput_test_params_ptr)->_data[1] & ~0xffff0000ul) | (((outstanding_pkts) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_CREATE(handle_plus_flags, transfer_size_kbyte, pkt_size_bytes, outstanding_pkts) \
    (uint32)(((handle_plus_flags)) & 0xffff) | \
    (uint32)(((transfer_size_kbyte) << 16) & 0xffff0000ul), \
    (uint32)(((pkt_size_bytes)) & 0xffff) | \
    (uint32)(((outstanding_pkts) << 16) & 0xffff0000ul)
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_PARAMS_PACK(test_tunnel_trans_bt_throughput_test_params_ptr, handle_plus_flags, transfer_size_kbyte, pkt_size_bytes, outstanding_pkts) \
    do { \
        (test_tunnel_trans_bt_throughput_test_params_ptr)->_data[0] = (uint32)((uint32)(((handle_plus_flags)) & 0xffff) | \
                                                                               (uint32)(((transfer_size_kbyte) << 16) & 0xffff0000ul)); \
        (test_tunnel_trans_bt_throughput_test_params_ptr)->_data[1] = (uint32)((uint32)(((pkt_size_bytes)) & 0xffff) | \
                                                                               (uint32)(((outstanding_pkts) << 16) & 0xffff0000ul)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_TRANS_BT_THROUGHPUT_TEST_RESULT

  DESCRIPTION

  MEMBERS
    response -
    duration -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT;

/* The following macros take TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT *test_tunnel_trans_bt_throughput_test_result_ptr */
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_RESPONSE_WORD_OFFSET (0)
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_RESPONSE_GET(test_tunnel_trans_bt_throughput_test_result_ptr) ((TEST_TUNNEL_RESPONSE_CODE)(((test_tunnel_trans_bt_throughput_test_result_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_RESPONSE_SET(test_tunnel_trans_bt_throughput_test_result_ptr, response) ((test_tunnel_trans_bt_throughput_test_result_ptr)->_data[0] =  \
                                                                                                                                 (uint32)(((test_tunnel_trans_bt_throughput_test_result_ptr)->_data[0] & ~0xffff) | (((response)) & 0xffff)))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_DURATION_GET(test_tunnel_trans_bt_throughput_test_result_ptr) (((((test_tunnel_trans_bt_throughput_test_result_ptr)->_data[0] & 0xffff0000) >> 16) |  \
                                                                                                                    (((test_tunnel_trans_bt_throughput_test_result_ptr)->_data[1] & 0xffff) << (32 - 16))))
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_DURATION_SET(test_tunnel_trans_bt_throughput_test_result_ptr, duration) do { \
        (test_tunnel_trans_bt_throughput_test_result_ptr)->_data[0] = (uint32)(((test_tunnel_trans_bt_throughput_test_result_ptr)->_data[0] & ~0xffff0000) | (((duration) << 16) & 0xffff0000)); \
        (test_tunnel_trans_bt_throughput_test_result_ptr)->_data[1] = (uint32)(((test_tunnel_trans_bt_throughput_test_result_ptr)->_data[1] & ~0xffff) | (((duration) >> (32 - 16)) & 0xffff)); } while (0)
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_CREATE(response, duration) \
    (uint32)(((response)) & 0xffff) | \
    (uint32)(((duration) << 16) & 0xffff0000), \
    (uint32)(((duration) >> (32 - 16)) & 0xffff)
#define TEST_TUNNEL_TRANS_BT_THROUGHPUT_TEST_RESULT_PACK(test_tunnel_trans_bt_throughput_test_result_ptr, response, duration) \
    do { \
        (test_tunnel_trans_bt_throughput_test_result_ptr)->_data[0] = (uint32)((uint32)(((response)) & 0xffff) | \
                                                                               (uint32)(((duration) << 16) & 0xffff0000)); \
        (test_tunnel_trans_bt_throughput_test_result_ptr)->_data[1] = (uint32)((((duration) >> (32 - 16)) & 0xffff)); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CONTROL

  DESCRIPTION

  MEMBERS
    control_id -
    tunnel_id  -
    payload    -

*******************************************************************************/
typedef struct
{
    uint32 _data[2];
} TEST_TUNNEL_CONTROL;

/* The following macros take TEST_TUNNEL_CONTROL *test_tunnel_control_ptr */
#define TEST_TUNNEL_CONTROL_CONTROL_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_CONTROL_CONTROL_ID_GET(test_tunnel_control_ptr) ((TEST_TUNNEL_CONTROL_ID)(((test_tunnel_control_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CONTROL_CONTROL_ID_SET(test_tunnel_control_ptr, control_id) ((test_tunnel_control_ptr)->_data[0] =  \
                                                                                     (uint32)(((test_tunnel_control_ptr)->_data[0] & ~0xffff) | (((control_id)) & 0xffff)))
#define TEST_TUNNEL_CONTROL_TUNNEL_ID_GET(test_tunnel_control_ptr) ((TEST_TUNNEL_ID)(((test_tunnel_control_ptr)->_data[0] & 0xff0000) >> 16))
#define TEST_TUNNEL_CONTROL_TUNNEL_ID_SET(test_tunnel_control_ptr, tunnel_id) ((test_tunnel_control_ptr)->_data[0] =  \
                                                                                   (uint32)(((test_tunnel_control_ptr)->_data[0] & ~0xff0000) | (((tunnel_id) << 16) & 0xff0000)))
#define TEST_TUNNEL_CONTROL_PAYLOAD_GET(test_tunnel_control_ptr, payload_ptr) do {  \
        (payload_ptr)->_data[0] = (test_tunnel_control_ptr)->_data[0]; } while (0)
#define TEST_TUNNEL_CONTROL_PAYLOAD_SET(test_tunnel_control_ptr, payload_ptr) do {  \
        (test_tunnel_control_ptr)->_data[0] = (payload_ptr)->_data[0]; } while (0)
#define TEST_TUNNEL_CONTROL_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CONTROL_CREATE(control_id, tunnel_id, payload) \
    (uint32)(((control_id)) & 0xffff) | \
    (uint32)(((tunnel_id) << 16) & 0xff0000) | \
    (uint32)
#define TEST_TUNNEL_CONTROL_PACK(test_tunnel_control_ptr, control_id, tunnel_id, payload_ptr) \
    do { \
        (test_tunnel_control_ptr)->_data[0] = (uint32)((uint32)(((control_id)) & 0xffff) | \
                                                       (uint32)(((tunnel_id) << 16) & 0xff0000) | \
                                                       (payload_ptr)->_data[0]); \
    } while (0)


/*******************************************************************************

  NAME
    TEST_TUNNEL_CSB_SERVICE

  DESCRIPTION

  MEMBERS
    type    -
    payload -

*******************************************************************************/
typedef struct
{
    uint32 _data[3];
} TEST_TUNNEL_CSB_SERVICE;

/* The following macros take TEST_TUNNEL_CSB_SERVICE *test_tunnel_csb_service_ptr */
#define TEST_TUNNEL_CSB_SERVICE_TYPE_WORD_OFFSET (0)
#define TEST_TUNNEL_CSB_SERVICE_TYPE_GET(test_tunnel_csb_service_ptr) ((TEST_TUNNEL_CSB_SERVICE_TYPE)(((test_tunnel_csb_service_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_TYPE_SET(test_tunnel_csb_service_ptr, type) ((test_tunnel_csb_service_ptr)->_data[0] =  \
                                                                                 (uint32)(((test_tunnel_csb_service_ptr)->_data[0] & ~0xffff) | (((type)) & 0xffff)))
#define TEST_TUNNEL_CSB_SERVICE_PAYLOAD_GET(test_tunnel_csb_service_ptr, payload_ptr) do {  \
        (payload_ptr)->_data[0] = (test_tunnel_csb_service_ptr)->_data[0]; \
        (payload_ptr)->_data[1] = (test_tunnel_csb_service_ptr)->_data[1]; } while (0)
#define TEST_TUNNEL_CSB_SERVICE_PAYLOAD_SET(test_tunnel_csb_service_ptr, payload_ptr) do {  \
        (test_tunnel_csb_service_ptr)->_data[0] = (payload_ptr)->_data[0]; \
        (test_tunnel_csb_service_ptr)->_data[1] = (payload_ptr)->_data[1]; } while (0)
#define TEST_TUNNEL_CSB_SERVICE_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_CSB_SERVICE_CREATE(type, payload) \
    (uint32)(((type)) & 0xffff) | \
    (uint32)
#define TEST_TUNNEL_CSB_SERVICE_PACK(test_tunnel_csb_service_ptr, type, payload_ptr) \
    do { \
        (test_tunnel_csb_service_ptr)->_data[0] = (uint32)((uint32)(((type)) & 0xffff) | \
                                                           (payload_ptr)->_data[0]); \
        (test_tunnel_csb_service_ptr)->_data[1] = (uint32)((payload_ptr)->_data[1]); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Siflash_Write_Req

  DESCRIPTION

  MEMBERS
    cmd_id    -
    interface -
    address   -
    length    -
    data      -

*******************************************************************************/
typedef struct
{
    uint32 _data[4];
} TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ;

/* The following macros take TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ *test_tunnel_apps_fw_siflash_write_req_ptr */
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_CMD_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_CMD_ID_GET(test_tunnel_apps_fw_siflash_write_req_ptr) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[0])
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_CMD_ID_SET(test_tunnel_apps_fw_siflash_write_req_ptr, cmd_id) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[0] = (uint32)(cmd_id))
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_INTERFACE_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_INTERFACE_GET(test_tunnel_apps_fw_siflash_write_req_ptr) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[1])
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_INTERFACE_SET(test_tunnel_apps_fw_siflash_write_req_ptr, interface) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[1] = (uint32)(interface))
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_ADDRESS_WORD_OFFSET (2)
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_ADDRESS_GET(test_tunnel_apps_fw_siflash_write_req_ptr) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[2])
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_ADDRESS_SET(test_tunnel_apps_fw_siflash_write_req_ptr, address) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[2] = (uint32)(address))
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_LENGTH_WORD_OFFSET (3)
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_LENGTH_GET(test_tunnel_apps_fw_siflash_write_req_ptr) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[3])
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_LENGTH_SET(test_tunnel_apps_fw_siflash_write_req_ptr, length) ((test_tunnel_apps_fw_siflash_write_req_ptr)->_data[3] = (uint32)(length))
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_DATA_WORD_OFFSET (4)
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_CREATE(cmd_id, interface, address, length) \
    (uint32)(cmd_id), \
    (uint32)(interface), \
    (uint32)(address), \
    (uint32)(length)
#define TEST_TUNNEL_APPS_FW_SIFLASH_WRITE_REQ_PACK(test_tunnel_apps_fw_siflash_write_req_ptr, cmd_id, interface, address, length) \
    do { \
        (test_tunnel_apps_fw_siflash_write_req_ptr)->_data[0] = (uint32)((uint32)(cmd_id)); \
        (test_tunnel_apps_fw_siflash_write_req_ptr)->_data[1] = (uint32)((uint32)(interface)); \
        (test_tunnel_apps_fw_siflash_write_req_ptr)->_data[2] = (uint32)((uint32)(address)); \
        (test_tunnel_apps_fw_siflash_write_req_ptr)->_data[3] = (uint32)((uint32)(length)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Siflash

  DESCRIPTION

  MEMBERS
    type    -
    payload -

*******************************************************************************/
typedef struct
{
    uint32 _data[6];
} TEST_TUNNEL_SIFLASH;

/* The following macros take TEST_TUNNEL_SIFLASH *test_tunnel_siflash_ptr */
#define TEST_TUNNEL_SIFLASH_TYPE_WORD_OFFSET (0)
#define TEST_TUNNEL_SIFLASH_TYPE_GET(test_tunnel_siflash_ptr) ((TEST_TUNNEL_APPS_FW_SIFLASH_CMD_TYPE)(test_tunnel_siflash_ptr)->_data[0])
#define TEST_TUNNEL_SIFLASH_TYPE_SET(test_tunnel_siflash_ptr, type) ((test_tunnel_siflash_ptr)->_data[0] = (uint32)(type))
#define TEST_TUNNEL_SIFLASH_PAYLOAD_WORD_OFFSET (1)
#define TEST_TUNNEL_SIFLASH_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_SIFLASH_CREATE(type) \
    (uint32)(type)
#define TEST_TUNNEL_SIFLASH_PACK(test_tunnel_siflash_ptr, type) \
    do { \
        (test_tunnel_siflash_ptr)->_data[0] = (uint32)((uint32)(type)); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_TRANS_BT_TEST

  DESCRIPTION

  MEMBERS
    id     -
    params -

*******************************************************************************/
typedef struct
{
    uint32 _data[3];
} TEST_TUNNEL_TRANS_BT_TEST;

/* The following macros take TEST_TUNNEL_TRANS_BT_TEST *test_tunnel_trans_bt_test_ptr */
#define TEST_TUNNEL_TRANS_BT_TEST_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_TRANS_BT_TEST_ID_GET(test_tunnel_trans_bt_test_ptr) ((TEST_TUNNEL_APPS_FW_TRANS_BT_TEST_ID)(test_tunnel_trans_bt_test_ptr)->_data[0])
#define TEST_TUNNEL_TRANS_BT_TEST_ID_SET(test_tunnel_trans_bt_test_ptr, id) ((test_tunnel_trans_bt_test_ptr)->_data[0] = (uint32)(id))
#define TEST_TUNNEL_TRANS_BT_TEST_PARAMS_WORD_OFFSET (1)
#define TEST_TUNNEL_TRANS_BT_TEST_PARAMS_GET(test_tunnel_trans_bt_test_ptr, params_ptr) do {  \
        (params_ptr)->_data[0] = (test_tunnel_trans_bt_test_ptr)->_data[1]; \
        (params_ptr)->_data[1] = (test_tunnel_trans_bt_test_ptr)->_data[2]; } while (0)
#define TEST_TUNNEL_TRANS_BT_TEST_PARAMS_SET(test_tunnel_trans_bt_test_ptr, params_ptr) do {  \
        (test_tunnel_trans_bt_test_ptr)->_data[1] = (params_ptr)->_data[0]; \
        (test_tunnel_trans_bt_test_ptr)->_data[2] = (params_ptr)->_data[1]; } while (0)
#define TEST_TUNNEL_TRANS_BT_TEST_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_TRANS_BT_TEST_CREATE(id, params) \
    (uint32)(id), \
    (uint32)
#define TEST_TUNNEL_TRANS_BT_TEST_PACK(test_tunnel_trans_bt_test_ptr, id, params_ptr) \
    do { \
        (test_tunnel_trans_bt_test_ptr)->_data[0] = (uint32)((uint32)(id)); \
        (test_tunnel_trans_bt_test_ptr)->_data[1] = (uint32)((params_ptr)->_data[0]); \
        (test_tunnel_trans_bt_test_ptr)->_data[2] = (uint32)((params_ptr)->_data[1]); \
    } while (0)


/*******************************************************************************

  NAME
    Test_Tunnel_Apps_Fw_Message

  DESCRIPTION

  MEMBERS
    Module_Id -
    padding   -
    payload   -

*******************************************************************************/
typedef struct
{
    uint32 _data[4];
} TEST_TUNNEL_APPS_FW_MESSAGE;

/* The following macros take TEST_TUNNEL_APPS_FW_MESSAGE *test_tunnel_apps_fw_message_ptr */
#define TEST_TUNNEL_APPS_FW_MESSAGE_MODULE_ID_WORD_OFFSET (0)
#define TEST_TUNNEL_APPS_FW_MESSAGE_MODULE_ID_GET(test_tunnel_apps_fw_message_ptr) ((TEST_TUNNEL_APPS_FW_MODULE_ID)(((test_tunnel_apps_fw_message_ptr)->_data[0] & 0xffff)))
#define TEST_TUNNEL_APPS_FW_MESSAGE_MODULE_ID_SET(test_tunnel_apps_fw_message_ptr, module_id) ((test_tunnel_apps_fw_message_ptr)->_data[0] =  \
                                                                                                   (uint32)(((test_tunnel_apps_fw_message_ptr)->_data[0] & ~0xffff) | (((module_id)) & 0xffff)))
#define TEST_TUNNEL_APPS_FW_MESSAGE_PADDING_GET(test_tunnel_apps_fw_message_ptr) ((((test_tunnel_apps_fw_message_ptr)->_data[0] & 0xffff0000ul) >> 16))
#define TEST_TUNNEL_APPS_FW_MESSAGE_PADDING_SET(test_tunnel_apps_fw_message_ptr, padding) ((test_tunnel_apps_fw_message_ptr)->_data[0] =  \
                                                                                               (uint32)(((test_tunnel_apps_fw_message_ptr)->_data[0] & ~0xffff0000ul) | (((padding) << 16) & 0xffff0000ul)))
#define TEST_TUNNEL_APPS_FW_MESSAGE_PAYLOAD_WORD_OFFSET (1)
#define TEST_TUNNEL_APPS_FW_MESSAGE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define TEST_TUNNEL_APPS_FW_MESSAGE_CREATE(Module_Id, padding) \
    (uint32)(((Module_Id)) & 0xffff) | \
    (uint32)(((padding) << 16) & 0xffff0000ul)
#define TEST_TUNNEL_APPS_FW_MESSAGE_PACK(test_tunnel_apps_fw_message_ptr, Module_Id, padding) \
    do { \
        (test_tunnel_apps_fw_message_ptr)->_data[0] = (uint32)((uint32)(((Module_Id)) & 0xffff) | \
                                                               (uint32)(((padding) << 16) & 0xffff0000ul)); \
    } while (0)


#endif /* TEST_TUNNEL_PRIM_H__ */

