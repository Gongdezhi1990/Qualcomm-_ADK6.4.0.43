/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2018
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef IPC_MSG_PRIM_H
#define IPC_MSG_PRIM_H


#define IPC_MSG_PROTOCOL_HEADER_WORD                                    (0xC409)


/*******************************************************************************

  NAME
    IPC_MSG_ID

  DESCRIPTION
    **************** REQ IDs **************************** ****************
    RESP IDs **************************** Make sure that the responses are
    identical to their requests, but with bit 14 set.

 VALUES
    setup_ipc_req               -
    get_framework_info_req      -
    setup_sig_block_req         -
    watchdog_ping_req           -
    reset_ipc_req               -
    teardown_ipc_req            -
    config_sig_channel_req      -
    data_channel_activate_req   -
    data_channel_deactivate_req -
    SETUP_IPC_RES               -
    GET_FRAMEWORK_INFO_RES      -
    SETUP_SIG_BLOCK_RES         -
    IPC_ERROR_IND               -
    WATCHDOG_PING_RES           -
    RESET_IPC_RES               -
    TEARDOWN_IPC_RES            -
    CONFIG_SIG_CHANNEL_RES      -
    DATA_CHANNEL_ACTIVATE_RES   -
    DATA_CHANNEL_DEACTIVATE_RES -
    PX_SETUP_READY_IND          -

*******************************************************************************/
typedef enum
{
    IPC_MSG_ID_SETUP_IPC_REQ = 0x0010,
    IPC_MSG_ID_GET_FRAMEWORK_INFO_REQ = 0x0011,
    IPC_MSG_ID_SETUP_SIG_BLOCK_REQ = 0x0012,
    IPC_MSG_ID_WATCHDOG_PING_REQ = 0x0014,
    IPC_MSG_ID_RESET_IPC_REQ = 0x0015,
    IPC_MSG_ID_TEARDOWN_IPC_REQ = 0x0016,
    IPC_MSG_ID_CONFIG_SIG_CHANNEL_REQ = 0x0017,
    IPC_MSG_ID_DATA_CHANNEL_ACTIVATE_REQ = 0x0018,
    IPC_MSG_ID_DATA_CHANNEL_DEACTIVATE_REQ = 0x0019,
    IPC_MSG_ID_SETUP_IPC_RES = 0x4010,
    IPC_MSG_ID_GET_FRAMEWORK_INFO_RES = 0x4011,
    IPC_MSG_ID_SETUP_SIG_BLOCK_RES = 0x4012,
    IPC_MSG_ID_IPC_ERROR_IND = 0x4013,
    IPC_MSG_ID_WATCHDOG_PING_RES = 0x4014,
    IPC_MSG_ID_RESET_IPC_RES = 0x4015,
    IPC_MSG_ID_TEARDOWN_IPC_RES = 0x4016,
    IPC_MSG_ID_CONFIG_SIG_CHANNEL_RES = 0x4017,
    IPC_MSG_ID_DATA_CHANNEL_ACTIVATE_RES = 0x4018,
    IPC_MSG_ID_DATA_CHANNEL_DEACTIVATE_RES = 0x4019,
    IPC_MSG_ID_PX_SETUP_READY_IND = 0x401A
} IPC_MSG_ID;


#define IPC_MSG_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    IPC_MSG_CONFIG_SIG_CHANNEL_REQ

  DESCRIPTION

  MEMBERS
    sigchan      - Signal channel number
    sigchan_type - Signal channel type (dedicated or static)
    signal_id    - User supplied signal id

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} IPC_MSG_CONFIG_SIG_CHANNEL_REQ;

/* The following macros take IPC_MSG_CONFIG_SIG_CHANNEL_REQ *ipc_msg_config_sig_channel_req_ptr */
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGCHAN_WORD_OFFSET (0)
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGCHAN_GET(ipc_msg_config_sig_channel_req_ptr) ((ipc_msg_config_sig_channel_req_ptr)->_data[0])
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGCHAN_SET(ipc_msg_config_sig_channel_req_ptr, sigchan) ((ipc_msg_config_sig_channel_req_ptr)->_data[0] = (uint16)(sigchan))
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGCHAN_TYPE_WORD_OFFSET (1)
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGCHAN_TYPE_GET(ipc_msg_config_sig_channel_req_ptr) ((ipc_msg_config_sig_channel_req_ptr)->_data[1])
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGCHAN_TYPE_SET(ipc_msg_config_sig_channel_req_ptr, sigchan_type) ((ipc_msg_config_sig_channel_req_ptr)->_data[1] = (uint16)(sigchan_type))
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGNAL_ID_WORD_OFFSET (2)
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGNAL_ID_GET(ipc_msg_config_sig_channel_req_ptr) ((ipc_msg_config_sig_channel_req_ptr)->_data[2])
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_SIGNAL_ID_SET(ipc_msg_config_sig_channel_req_ptr, signal_id) ((ipc_msg_config_sig_channel_req_ptr)->_data[2] = (uint16)(signal_id))
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_CREATE(sigchan, sigchan_type, signal_id) \
    (uint16)(sigchan), \
    (uint16)(sigchan_type), \
    (uint16)(signal_id)
#define IPC_MSG_CONFIG_SIG_CHANNEL_REQ_PACK(ipc_msg_config_sig_channel_req_ptr, sigchan, sigchan_type, signal_id) \
    do { \
        (ipc_msg_config_sig_channel_req_ptr)->_data[0] = (uint16)((uint16)(sigchan)); \
        (ipc_msg_config_sig_channel_req_ptr)->_data[1] = (uint16)((uint16)(sigchan_type)); \
        (ipc_msg_config_sig_channel_req_ptr)->_data[2] = (uint16)((uint16)(signal_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_CONFIG_SIG_CHANNEL_RES

  DESCRIPTION

  MEMBERS
    status         - status code
    sigchan        - Signal channel number
    signal_id      - User supplied signal id
    remote_proc_id - Not used in message but does get used internally

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} IPC_MSG_CONFIG_SIG_CHANNEL_RES;

/* The following macros take IPC_MSG_CONFIG_SIG_CHANNEL_RES *ipc_msg_config_sig_channel_res_ptr */
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_STATUS_GET(ipc_msg_config_sig_channel_res_ptr) ((ipc_msg_config_sig_channel_res_ptr)->_data[0])
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_STATUS_SET(ipc_msg_config_sig_channel_res_ptr, status) ((ipc_msg_config_sig_channel_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_SIGCHAN_WORD_OFFSET (1)
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_SIGCHAN_GET(ipc_msg_config_sig_channel_res_ptr) ((ipc_msg_config_sig_channel_res_ptr)->_data[1])
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_SIGCHAN_SET(ipc_msg_config_sig_channel_res_ptr, sigchan) ((ipc_msg_config_sig_channel_res_ptr)->_data[1] = (uint16)(sigchan))
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_SIGNAL_ID_WORD_OFFSET (2)
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_SIGNAL_ID_GET(ipc_msg_config_sig_channel_res_ptr) ((ipc_msg_config_sig_channel_res_ptr)->_data[2])
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_SIGNAL_ID_SET(ipc_msg_config_sig_channel_res_ptr, signal_id) ((ipc_msg_config_sig_channel_res_ptr)->_data[2] = (uint16)(signal_id))
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_REMOTE_PROC_ID_WORD_OFFSET (3)
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_REMOTE_PROC_ID_GET(ipc_msg_config_sig_channel_res_ptr) ((ipc_msg_config_sig_channel_res_ptr)->_data[3])
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_REMOTE_PROC_ID_SET(ipc_msg_config_sig_channel_res_ptr, remote_proc_id) ((ipc_msg_config_sig_channel_res_ptr)->_data[3] = (uint16)(remote_proc_id))
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_CREATE(status, sigchan, signal_id, remote_proc_id) \
    (uint16)(status), \
    (uint16)(sigchan), \
    (uint16)(signal_id), \
    (uint16)(remote_proc_id)
#define IPC_MSG_CONFIG_SIG_CHANNEL_RES_PACK(ipc_msg_config_sig_channel_res_ptr, status, sigchan, signal_id, remote_proc_id) \
    do { \
        (ipc_msg_config_sig_channel_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_config_sig_channel_res_ptr)->_data[1] = (uint16)((uint16)(sigchan)); \
        (ipc_msg_config_sig_channel_res_ptr)->_data[2] = (uint16)((uint16)(signal_id)); \
        (ipc_msg_config_sig_channel_res_ptr)->_data[3] = (uint16)((uint16)(remote_proc_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ

  DESCRIPTION

  MEMBERS
    data_channel_id - Data channel ID
    channel         - Pointer to data channel object for the given data channel
                      ID
    param_size      - size of parameter block
    params          - opaque parameter block to be sent to remote processor

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ;

/* The following macros take IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ *ipc_msg_data_channel_activate_req_ptr */
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_DATA_CHANNEL_ID_WORD_OFFSET (0)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_DATA_CHANNEL_ID_GET(ipc_msg_data_channel_activate_req_ptr) ((ipc_msg_data_channel_activate_req_ptr)->_data[0])
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_DATA_CHANNEL_ID_SET(ipc_msg_data_channel_activate_req_ptr, data_channel_id) ((ipc_msg_data_channel_activate_req_ptr)->_data[0] = (uint16)(data_channel_id))
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_CHANNEL_WORD_OFFSET (1)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_CHANNEL_GET(ipc_msg_data_channel_activate_req_ptr)  \
    (((uint32)((ipc_msg_data_channel_activate_req_ptr)->_data[1]) | \
      ((uint32)((ipc_msg_data_channel_activate_req_ptr)->_data[2]) << 16)))
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_CHANNEL_SET(ipc_msg_data_channel_activate_req_ptr, channel) do { \
        (ipc_msg_data_channel_activate_req_ptr)->_data[1] = (uint16)((channel) & 0xffff); \
        (ipc_msg_data_channel_activate_req_ptr)->_data[2] = (uint16)((channel) >> 16); } while (0)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_PARAM_SIZE_WORD_OFFSET (3)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_PARAM_SIZE_GET(ipc_msg_data_channel_activate_req_ptr) ((ipc_msg_data_channel_activate_req_ptr)->_data[3])
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_PARAM_SIZE_SET(ipc_msg_data_channel_activate_req_ptr, param_size) ((ipc_msg_data_channel_activate_req_ptr)->_data[3] = (uint16)(param_size))
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_PARAMS_WORD_OFFSET (4)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_PARAMS_GET(ipc_msg_data_channel_activate_req_ptr) ((ipc_msg_data_channel_activate_req_ptr)->_data[4])
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_PARAMS_SET(ipc_msg_data_channel_activate_req_ptr, params) ((ipc_msg_data_channel_activate_req_ptr)->_data[4] = (uint16)(params))
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_CREATE(data_channel_id, channel, param_size, params) \
    (uint16)(data_channel_id), \
    (uint16)((channel) & 0xffff), \
    (uint16)((channel) >> 16), \
    (uint16)(param_size), \
    (uint16)(params)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_REQ_PACK(ipc_msg_data_channel_activate_req_ptr, data_channel_id, channel, param_size, params) \
    do { \
        (ipc_msg_data_channel_activate_req_ptr)->_data[0] = (uint16)((uint16)(data_channel_id)); \
        (ipc_msg_data_channel_activate_req_ptr)->_data[1] = (uint16)((uint16)((channel) & 0xffff)); \
        (ipc_msg_data_channel_activate_req_ptr)->_data[2] = (uint16)(((channel) >> 16)); \
        (ipc_msg_data_channel_activate_req_ptr)->_data[3] = (uint16)((uint16)(param_size)); \
        (ipc_msg_data_channel_activate_req_ptr)->_data[4] = (uint16)((uint16)(params)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_DATA_CHANNEL_ACTIVATE_RES

  DESCRIPTION

  MEMBERS
    status          - status code
    data_channel_id - Data channel ID

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} IPC_MSG_DATA_CHANNEL_ACTIVATE_RES;

/* The following macros take IPC_MSG_DATA_CHANNEL_ACTIVATE_RES *ipc_msg_data_channel_activate_res_ptr */
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_STATUS_GET(ipc_msg_data_channel_activate_res_ptr) ((ipc_msg_data_channel_activate_res_ptr)->_data[0])
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_STATUS_SET(ipc_msg_data_channel_activate_res_ptr, status) ((ipc_msg_data_channel_activate_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_DATA_CHANNEL_ID_WORD_OFFSET (1)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_DATA_CHANNEL_ID_GET(ipc_msg_data_channel_activate_res_ptr) ((ipc_msg_data_channel_activate_res_ptr)->_data[1])
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_DATA_CHANNEL_ID_SET(ipc_msg_data_channel_activate_res_ptr, data_channel_id) ((ipc_msg_data_channel_activate_res_ptr)->_data[1] = (uint16)(data_channel_id))
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_CREATE(status, data_channel_id) \
    (uint16)(status), \
    (uint16)(data_channel_id)
#define IPC_MSG_DATA_CHANNEL_ACTIVATE_RES_PACK(ipc_msg_data_channel_activate_res_ptr, status, data_channel_id) \
    do { \
        (ipc_msg_data_channel_activate_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_data_channel_activate_res_ptr)->_data[1] = (uint16)((uint16)(data_channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ

  DESCRIPTION

  MEMBERS
    data_channel_id - Data channel ID

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ;

/* The following macros take IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ *ipc_msg_data_channel_deactivate_req_ptr */
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ_DATA_CHANNEL_ID_WORD_OFFSET (0)
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ_DATA_CHANNEL_ID_GET(ipc_msg_data_channel_deactivate_req_ptr) ((ipc_msg_data_channel_deactivate_req_ptr)->_data[0])
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ_DATA_CHANNEL_ID_SET(ipc_msg_data_channel_deactivate_req_ptr, data_channel_id) ((ipc_msg_data_channel_deactivate_req_ptr)->_data[0] = (uint16)(data_channel_id))
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ_CREATE(data_channel_id) \
    (uint16)(data_channel_id)
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_REQ_PACK(ipc_msg_data_channel_deactivate_req_ptr, data_channel_id) \
    do { \
        (ipc_msg_data_channel_deactivate_req_ptr)->_data[0] = (uint16)((uint16)(data_channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES

  DESCRIPTION

  MEMBERS
    status          - status code
    data_channel_id - Data channel ID

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES;

/* The following macros take IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES *ipc_msg_data_channel_deactivate_res_ptr */
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_STATUS_GET(ipc_msg_data_channel_deactivate_res_ptr) ((ipc_msg_data_channel_deactivate_res_ptr)->_data[0])
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_STATUS_SET(ipc_msg_data_channel_deactivate_res_ptr, status) ((ipc_msg_data_channel_deactivate_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_DATA_CHANNEL_ID_WORD_OFFSET (1)
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_DATA_CHANNEL_ID_GET(ipc_msg_data_channel_deactivate_res_ptr) ((ipc_msg_data_channel_deactivate_res_ptr)->_data[1])
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_DATA_CHANNEL_ID_SET(ipc_msg_data_channel_deactivate_res_ptr, data_channel_id) ((ipc_msg_data_channel_deactivate_res_ptr)->_data[1] = (uint16)(data_channel_id))
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_CREATE(status, data_channel_id) \
    (uint16)(status), \
    (uint16)(data_channel_id)
#define IPC_MSG_DATA_CHANNEL_DEACTIVATE_RES_PACK(ipc_msg_data_channel_deactivate_res_ptr, status, data_channel_id) \
    do { \
        (ipc_msg_data_channel_deactivate_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_data_channel_deactivate_res_ptr)->_data[1] = (uint16)((uint16)(data_channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_GET_FRAMEWORK_INFO_REQ

  DESCRIPTION

  MEMBERS
    framework_type    - Framework Type
    framework_version - Framework Version

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} IPC_MSG_GET_FRAMEWORK_INFO_REQ;

/* The following macros take IPC_MSG_GET_FRAMEWORK_INFO_REQ *ipc_msg_get_framework_info_req_ptr */
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_FRAMEWORK_TYPE_WORD_OFFSET (0)
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_FRAMEWORK_TYPE_GET(ipc_msg_get_framework_info_req_ptr) ((ipc_msg_get_framework_info_req_ptr)->_data[0])
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_FRAMEWORK_TYPE_SET(ipc_msg_get_framework_info_req_ptr, framework_type) ((ipc_msg_get_framework_info_req_ptr)->_data[0] = (uint16)(framework_type))
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_FRAMEWORK_VERSION_WORD_OFFSET (1)
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_FRAMEWORK_VERSION_GET(ipc_msg_get_framework_info_req_ptr) ((ipc_msg_get_framework_info_req_ptr)->_data[1])
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_FRAMEWORK_VERSION_SET(ipc_msg_get_framework_info_req_ptr, framework_version) ((ipc_msg_get_framework_info_req_ptr)->_data[1] = (uint16)(framework_version))
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_CREATE(framework_type, framework_version) \
    (uint16)(framework_type), \
    (uint16)(framework_version)
#define IPC_MSG_GET_FRAMEWORK_INFO_REQ_PACK(ipc_msg_get_framework_info_req_ptr, framework_type, framework_version) \
    do { \
        (ipc_msg_get_framework_info_req_ptr)->_data[0] = (uint16)((uint16)(framework_type)); \
        (ipc_msg_get_framework_info_req_ptr)->_data[1] = (uint16)((uint16)(framework_version)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_GET_FRAMEWORK_INFO_RES

  DESCRIPTION

  MEMBERS
    status            - status code
    framework_type    - Framework Type
    framework_version - Framework Version

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} IPC_MSG_GET_FRAMEWORK_INFO_RES;

/* The following macros take IPC_MSG_GET_FRAMEWORK_INFO_RES *ipc_msg_get_framework_info_res_ptr */
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_STATUS_GET(ipc_msg_get_framework_info_res_ptr) ((ipc_msg_get_framework_info_res_ptr)->_data[0])
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_STATUS_SET(ipc_msg_get_framework_info_res_ptr, status) ((ipc_msg_get_framework_info_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_FRAMEWORK_TYPE_WORD_OFFSET (1)
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_FRAMEWORK_TYPE_GET(ipc_msg_get_framework_info_res_ptr) ((ipc_msg_get_framework_info_res_ptr)->_data[1])
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_FRAMEWORK_TYPE_SET(ipc_msg_get_framework_info_res_ptr, framework_type) ((ipc_msg_get_framework_info_res_ptr)->_data[1] = (uint16)(framework_type))
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_FRAMEWORK_VERSION_WORD_OFFSET (2)
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_FRAMEWORK_VERSION_GET(ipc_msg_get_framework_info_res_ptr) ((ipc_msg_get_framework_info_res_ptr)->_data[2])
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_FRAMEWORK_VERSION_SET(ipc_msg_get_framework_info_res_ptr, framework_version) ((ipc_msg_get_framework_info_res_ptr)->_data[2] = (uint16)(framework_version))
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_CREATE(status, framework_type, framework_version) \
    (uint16)(status), \
    (uint16)(framework_type), \
    (uint16)(framework_version)
#define IPC_MSG_GET_FRAMEWORK_INFO_RES_PACK(ipc_msg_get_framework_info_res_ptr, status, framework_type, framework_version) \
    do { \
        (ipc_msg_get_framework_info_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_get_framework_info_res_ptr)->_data[1] = (uint16)((uint16)(framework_type)); \
        (ipc_msg_get_framework_info_res_ptr)->_data[2] = (uint16)((uint16)(framework_version)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_IPC_ERROR_IND

  DESCRIPTION

  MEMBERS
    error_type  - Error type
    error_level - Error level
    error_code  - Error code

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} IPC_MSG_IPC_ERROR_IND;

/* The following macros take IPC_MSG_IPC_ERROR_IND *ipc_msg_ipc_error_ind_ptr */
#define IPC_MSG_IPC_ERROR_IND_ERROR_TYPE_WORD_OFFSET (0)
#define IPC_MSG_IPC_ERROR_IND_ERROR_TYPE_GET(ipc_msg_ipc_error_ind_ptr) ((ipc_msg_ipc_error_ind_ptr)->_data[0])
#define IPC_MSG_IPC_ERROR_IND_ERROR_TYPE_SET(ipc_msg_ipc_error_ind_ptr, error_type) ((ipc_msg_ipc_error_ind_ptr)->_data[0] = (uint16)(error_type))
#define IPC_MSG_IPC_ERROR_IND_ERROR_LEVEL_WORD_OFFSET (1)
#define IPC_MSG_IPC_ERROR_IND_ERROR_LEVEL_GET(ipc_msg_ipc_error_ind_ptr) ((ipc_msg_ipc_error_ind_ptr)->_data[1])
#define IPC_MSG_IPC_ERROR_IND_ERROR_LEVEL_SET(ipc_msg_ipc_error_ind_ptr, error_level) ((ipc_msg_ipc_error_ind_ptr)->_data[1] = (uint16)(error_level))
#define IPC_MSG_IPC_ERROR_IND_ERROR_CODE_WORD_OFFSET (2)
#define IPC_MSG_IPC_ERROR_IND_ERROR_CODE_GET(ipc_msg_ipc_error_ind_ptr) ((ipc_msg_ipc_error_ind_ptr)->_data[2])
#define IPC_MSG_IPC_ERROR_IND_ERROR_CODE_SET(ipc_msg_ipc_error_ind_ptr, error_code) ((ipc_msg_ipc_error_ind_ptr)->_data[2] = (uint16)(error_code))
#define IPC_MSG_IPC_ERROR_IND_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_IPC_ERROR_IND_CREATE(error_type, error_level, error_code) \
    (uint16)(error_type), \
    (uint16)(error_level), \
    (uint16)(error_code)
#define IPC_MSG_IPC_ERROR_IND_PACK(ipc_msg_ipc_error_ind_ptr, error_type, error_level, error_code) \
    do { \
        (ipc_msg_ipc_error_ind_ptr)->_data[0] = (uint16)((uint16)(error_type)); \
        (ipc_msg_ipc_error_ind_ptr)->_data[1] = (uint16)((uint16)(error_level)); \
        (ipc_msg_ipc_error_ind_ptr)->_data[2] = (uint16)((uint16)(error_code)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_PX_SETUP_READY_IND

  DESCRIPTION

  MEMBERS
    status - status code

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} IPC_MSG_PX_SETUP_READY_IND;

/* The following macros take IPC_MSG_PX_SETUP_READY_IND *ipc_msg_px_setup_ready_ind_ptr */
#define IPC_MSG_PX_SETUP_READY_IND_STATUS_WORD_OFFSET (0)
#define IPC_MSG_PX_SETUP_READY_IND_STATUS_GET(ipc_msg_px_setup_ready_ind_ptr) ((ipc_msg_px_setup_ready_ind_ptr)->_data[0])
#define IPC_MSG_PX_SETUP_READY_IND_STATUS_SET(ipc_msg_px_setup_ready_ind_ptr, status) ((ipc_msg_px_setup_ready_ind_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_PX_SETUP_READY_IND_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_PX_SETUP_READY_IND_CREATE(status) \
    (uint16)(status)
#define IPC_MSG_PX_SETUP_READY_IND_PACK(ipc_msg_px_setup_ready_ind_ptr, status) \
    do { \
        (ipc_msg_px_setup_ready_ind_ptr)->_data[0] = (uint16)((uint16)(status)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_RESET_IPC_REQ

  DESCRIPTION

  MEMBERS
    message_channel_id - Message Channel ID as returned by ipc_setup_comms

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} IPC_MSG_RESET_IPC_REQ;

/* The following macros take IPC_MSG_RESET_IPC_REQ *ipc_msg_reset_ipc_req_ptr */
#define IPC_MSG_RESET_IPC_REQ_MESSAGE_CHANNEL_ID_WORD_OFFSET (0)
#define IPC_MSG_RESET_IPC_REQ_MESSAGE_CHANNEL_ID_GET(ipc_msg_reset_ipc_req_ptr) ((ipc_msg_reset_ipc_req_ptr)->_data[0])
#define IPC_MSG_RESET_IPC_REQ_MESSAGE_CHANNEL_ID_SET(ipc_msg_reset_ipc_req_ptr, message_channel_id) ((ipc_msg_reset_ipc_req_ptr)->_data[0] = (uint16)(message_channel_id))
#define IPC_MSG_RESET_IPC_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_RESET_IPC_REQ_CREATE(message_channel_id) \
    (uint16)(message_channel_id)
#define IPC_MSG_RESET_IPC_REQ_PACK(ipc_msg_reset_ipc_req_ptr, message_channel_id) \
    do { \
        (ipc_msg_reset_ipc_req_ptr)->_data[0] = (uint16)((uint16)(message_channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_RESET_IPC_RES

  DESCRIPTION

  MEMBERS
    status             - status code
    message_channel_id - Message Channel ID as returned by ipc_setup_comms

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} IPC_MSG_RESET_IPC_RES;

/* The following macros take IPC_MSG_RESET_IPC_RES *ipc_msg_reset_ipc_res_ptr */
#define IPC_MSG_RESET_IPC_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_RESET_IPC_RES_STATUS_GET(ipc_msg_reset_ipc_res_ptr) ((ipc_msg_reset_ipc_res_ptr)->_data[0])
#define IPC_MSG_RESET_IPC_RES_STATUS_SET(ipc_msg_reset_ipc_res_ptr, status) ((ipc_msg_reset_ipc_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_RESET_IPC_RES_MESSAGE_CHANNEL_ID_WORD_OFFSET (1)
#define IPC_MSG_RESET_IPC_RES_MESSAGE_CHANNEL_ID_GET(ipc_msg_reset_ipc_res_ptr) ((ipc_msg_reset_ipc_res_ptr)->_data[1])
#define IPC_MSG_RESET_IPC_RES_MESSAGE_CHANNEL_ID_SET(ipc_msg_reset_ipc_res_ptr, message_channel_id) ((ipc_msg_reset_ipc_res_ptr)->_data[1] = (uint16)(message_channel_id))
#define IPC_MSG_RESET_IPC_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_RESET_IPC_RES_CREATE(status, message_channel_id) \
    (uint16)(status), \
    (uint16)(message_channel_id)
#define IPC_MSG_RESET_IPC_RES_PACK(ipc_msg_reset_ipc_res_ptr, status, message_channel_id) \
    do { \
        (ipc_msg_reset_ipc_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_reset_ipc_res_ptr)->_data[1] = (uint16)((uint16)(message_channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_SETUP_IPC_REQ

  DESCRIPTION

  MEMBERS
    Message_Channel_ID             - Message Channel ID as returned by
                                     ipc_setup_comms
    IPC_Version                    - IPC version
    IPC_Maximum_Message_Length     - Maximum length of any IPC message
    IPC_Maximum_Number_of_Messages - Maximum number of IPC messages that fits in
                                     the cbuffer data

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} IPC_MSG_SETUP_IPC_REQ;

/* The following macros take IPC_MSG_SETUP_IPC_REQ *ipc_msg_setup_ipc_req_ptr */
#define IPC_MSG_SETUP_IPC_REQ_MESSAGE_CHANNEL_ID_WORD_OFFSET (0)
#define IPC_MSG_SETUP_IPC_REQ_MESSAGE_CHANNEL_ID_GET(ipc_msg_setup_ipc_req_ptr) ((ipc_msg_setup_ipc_req_ptr)->_data[0])
#define IPC_MSG_SETUP_IPC_REQ_MESSAGE_CHANNEL_ID_SET(ipc_msg_setup_ipc_req_ptr, message_channel_id) ((ipc_msg_setup_ipc_req_ptr)->_data[0] = (uint16)(message_channel_id))
#define IPC_MSG_SETUP_IPC_REQ_IPC_VERSION_WORD_OFFSET (1)
#define IPC_MSG_SETUP_IPC_REQ_IPC_VERSION_GET(ipc_msg_setup_ipc_req_ptr) ((ipc_msg_setup_ipc_req_ptr)->_data[1])
#define IPC_MSG_SETUP_IPC_REQ_IPC_VERSION_SET(ipc_msg_setup_ipc_req_ptr, ipc_version) ((ipc_msg_setup_ipc_req_ptr)->_data[1] = (uint16)(ipc_version))
#define IPC_MSG_SETUP_IPC_REQ_IPC_MAXIMUM_MESSAGE_LENGTH_WORD_OFFSET (2)
#define IPC_MSG_SETUP_IPC_REQ_IPC_MAXIMUM_MESSAGE_LENGTH_GET(ipc_msg_setup_ipc_req_ptr) ((ipc_msg_setup_ipc_req_ptr)->_data[2])
#define IPC_MSG_SETUP_IPC_REQ_IPC_MAXIMUM_MESSAGE_LENGTH_SET(ipc_msg_setup_ipc_req_ptr, ipc_maximum_message_length) ((ipc_msg_setup_ipc_req_ptr)->_data[2] = (uint16)(ipc_maximum_message_length))
#define IPC_MSG_SETUP_IPC_REQ_IPC_MAXIMUM_NUMBER_OF_MESSAGES_WORD_OFFSET (3)
#define IPC_MSG_SETUP_IPC_REQ_IPC_MAXIMUM_NUMBER_OF_MESSAGES_GET(ipc_msg_setup_ipc_req_ptr) ((ipc_msg_setup_ipc_req_ptr)->_data[3])
#define IPC_MSG_SETUP_IPC_REQ_IPC_MAXIMUM_NUMBER_OF_MESSAGES_SET(ipc_msg_setup_ipc_req_ptr, ipc_maximum_number_of_messages) ((ipc_msg_setup_ipc_req_ptr)->_data[3] = (uint16)(ipc_maximum_number_of_messages))
#define IPC_MSG_SETUP_IPC_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_SETUP_IPC_REQ_CREATE(Message_Channel_ID, IPC_Version, IPC_Maximum_Message_Length, IPC_Maximum_Number_of_Messages) \
    (uint16)(Message_Channel_ID), \
    (uint16)(IPC_Version), \
    (uint16)(IPC_Maximum_Message_Length), \
    (uint16)(IPC_Maximum_Number_of_Messages)
#define IPC_MSG_SETUP_IPC_REQ_PACK(ipc_msg_setup_ipc_req_ptr, Message_Channel_ID, IPC_Version, IPC_Maximum_Message_Length, IPC_Maximum_Number_of_Messages) \
    do { \
        (ipc_msg_setup_ipc_req_ptr)->_data[0] = (uint16)((uint16)(Message_Channel_ID)); \
        (ipc_msg_setup_ipc_req_ptr)->_data[1] = (uint16)((uint16)(IPC_Version)); \
        (ipc_msg_setup_ipc_req_ptr)->_data[2] = (uint16)((uint16)(IPC_Maximum_Message_Length)); \
        (ipc_msg_setup_ipc_req_ptr)->_data[3] = (uint16)((uint16)(IPC_Maximum_Number_of_Messages)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_SETUP_IPC_RES

  DESCRIPTION

  MEMBERS
    status                         - status code
    IPC_Version                    - IPC version
    IPC_Maximum_Message_Length     - Maximum length of any IPC message
    IPC_Maximum_Number_of_Messages - Maximum number of IPC messages that fits in
                                     the cbuffer data

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} IPC_MSG_SETUP_IPC_RES;

/* The following macros take IPC_MSG_SETUP_IPC_RES *ipc_msg_setup_ipc_res_ptr */
#define IPC_MSG_SETUP_IPC_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_SETUP_IPC_RES_STATUS_GET(ipc_msg_setup_ipc_res_ptr) ((ipc_msg_setup_ipc_res_ptr)->_data[0])
#define IPC_MSG_SETUP_IPC_RES_STATUS_SET(ipc_msg_setup_ipc_res_ptr, status) ((ipc_msg_setup_ipc_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_SETUP_IPC_RES_IPC_VERSION_WORD_OFFSET (1)
#define IPC_MSG_SETUP_IPC_RES_IPC_VERSION_GET(ipc_msg_setup_ipc_res_ptr) ((ipc_msg_setup_ipc_res_ptr)->_data[1])
#define IPC_MSG_SETUP_IPC_RES_IPC_VERSION_SET(ipc_msg_setup_ipc_res_ptr, ipc_version) ((ipc_msg_setup_ipc_res_ptr)->_data[1] = (uint16)(ipc_version))
#define IPC_MSG_SETUP_IPC_RES_IPC_MAXIMUM_MESSAGE_LENGTH_WORD_OFFSET (2)
#define IPC_MSG_SETUP_IPC_RES_IPC_MAXIMUM_MESSAGE_LENGTH_GET(ipc_msg_setup_ipc_res_ptr) ((ipc_msg_setup_ipc_res_ptr)->_data[2])
#define IPC_MSG_SETUP_IPC_RES_IPC_MAXIMUM_MESSAGE_LENGTH_SET(ipc_msg_setup_ipc_res_ptr, ipc_maximum_message_length) ((ipc_msg_setup_ipc_res_ptr)->_data[2] = (uint16)(ipc_maximum_message_length))
#define IPC_MSG_SETUP_IPC_RES_IPC_MAXIMUM_NUMBER_OF_MESSAGES_WORD_OFFSET (3)
#define IPC_MSG_SETUP_IPC_RES_IPC_MAXIMUM_NUMBER_OF_MESSAGES_GET(ipc_msg_setup_ipc_res_ptr) ((ipc_msg_setup_ipc_res_ptr)->_data[3])
#define IPC_MSG_SETUP_IPC_RES_IPC_MAXIMUM_NUMBER_OF_MESSAGES_SET(ipc_msg_setup_ipc_res_ptr, ipc_maximum_number_of_messages) ((ipc_msg_setup_ipc_res_ptr)->_data[3] = (uint16)(ipc_maximum_number_of_messages))
#define IPC_MSG_SETUP_IPC_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_SETUP_IPC_RES_CREATE(status, IPC_Version, IPC_Maximum_Message_Length, IPC_Maximum_Number_of_Messages) \
    (uint16)(status), \
    (uint16)(IPC_Version), \
    (uint16)(IPC_Maximum_Message_Length), \
    (uint16)(IPC_Maximum_Number_of_Messages)
#define IPC_MSG_SETUP_IPC_RES_PACK(ipc_msg_setup_ipc_res_ptr, status, IPC_Version, IPC_Maximum_Message_Length, IPC_Maximum_Number_of_Messages) \
    do { \
        (ipc_msg_setup_ipc_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_setup_ipc_res_ptr)->_data[1] = (uint16)((uint16)(IPC_Version)); \
        (ipc_msg_setup_ipc_res_ptr)->_data[2] = (uint16)((uint16)(IPC_Maximum_Message_Length)); \
        (ipc_msg_setup_ipc_res_ptr)->_data[3] = (uint16)((uint16)(IPC_Maximum_Number_of_Messages)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_SETUP_SIG_BLOCK_REQ

  DESCRIPTION

  MEMBERS
    processor_id         - Processor ID
    signal_blocks_lut_id - Signal block's LUT entry ID

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} IPC_MSG_SETUP_SIG_BLOCK_REQ;

/* The following macros take IPC_MSG_SETUP_SIG_BLOCK_REQ *ipc_msg_setup_sig_block_req_ptr */
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_PROCESSOR_ID_WORD_OFFSET (0)
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_PROCESSOR_ID_GET(ipc_msg_setup_sig_block_req_ptr) ((ipc_msg_setup_sig_block_req_ptr)->_data[0])
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_PROCESSOR_ID_SET(ipc_msg_setup_sig_block_req_ptr, processor_id) ((ipc_msg_setup_sig_block_req_ptr)->_data[0] = (uint16)(processor_id))
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_SIGNAL_BLOCKS_LUT_ID_WORD_OFFSET (1)
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_SIGNAL_BLOCKS_LUT_ID_GET(ipc_msg_setup_sig_block_req_ptr) ((ipc_msg_setup_sig_block_req_ptr)->_data[1])
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_SIGNAL_BLOCKS_LUT_ID_SET(ipc_msg_setup_sig_block_req_ptr, signal_blocks_lut_id) ((ipc_msg_setup_sig_block_req_ptr)->_data[1] = (uint16)(signal_blocks_lut_id))
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_CREATE(processor_id, signal_blocks_lut_id) \
    (uint16)(processor_id), \
    (uint16)(signal_blocks_lut_id)
#define IPC_MSG_SETUP_SIG_BLOCK_REQ_PACK(ipc_msg_setup_sig_block_req_ptr, processor_id, signal_blocks_lut_id) \
    do { \
        (ipc_msg_setup_sig_block_req_ptr)->_data[0] = (uint16)((uint16)(processor_id)); \
        (ipc_msg_setup_sig_block_req_ptr)->_data[1] = (uint16)((uint16)(signal_blocks_lut_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_SETUP_SIG_BLOCK_RES

  DESCRIPTION

  MEMBERS
    status               - status code
    signal_blocks_lut_id - Signal block's LUT entry ID

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} IPC_MSG_SETUP_SIG_BLOCK_RES;

/* The following macros take IPC_MSG_SETUP_SIG_BLOCK_RES *ipc_msg_setup_sig_block_res_ptr */
#define IPC_MSG_SETUP_SIG_BLOCK_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_SETUP_SIG_BLOCK_RES_STATUS_GET(ipc_msg_setup_sig_block_res_ptr) ((ipc_msg_setup_sig_block_res_ptr)->_data[0])
#define IPC_MSG_SETUP_SIG_BLOCK_RES_STATUS_SET(ipc_msg_setup_sig_block_res_ptr, status) ((ipc_msg_setup_sig_block_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_SETUP_SIG_BLOCK_RES_SIGNAL_BLOCKS_LUT_ID_WORD_OFFSET (1)
#define IPC_MSG_SETUP_SIG_BLOCK_RES_SIGNAL_BLOCKS_LUT_ID_GET(ipc_msg_setup_sig_block_res_ptr) ((ipc_msg_setup_sig_block_res_ptr)->_data[1])
#define IPC_MSG_SETUP_SIG_BLOCK_RES_SIGNAL_BLOCKS_LUT_ID_SET(ipc_msg_setup_sig_block_res_ptr, signal_blocks_lut_id) ((ipc_msg_setup_sig_block_res_ptr)->_data[1] = (uint16)(signal_blocks_lut_id))
#define IPC_MSG_SETUP_SIG_BLOCK_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_SETUP_SIG_BLOCK_RES_CREATE(status, signal_blocks_lut_id) \
    (uint16)(status), \
    (uint16)(signal_blocks_lut_id)
#define IPC_MSG_SETUP_SIG_BLOCK_RES_PACK(ipc_msg_setup_sig_block_res_ptr, status, signal_blocks_lut_id) \
    do { \
        (ipc_msg_setup_sig_block_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_setup_sig_block_res_ptr)->_data[1] = (uint16)((uint16)(signal_blocks_lut_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_TEARDOWN_IPC_REQ

  DESCRIPTION

  MEMBERS
    message_channel_id - Message Channel ID as returned by ipc_setup_comms

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} IPC_MSG_TEARDOWN_IPC_REQ;

/* The following macros take IPC_MSG_TEARDOWN_IPC_REQ *ipc_msg_teardown_ipc_req_ptr */
#define IPC_MSG_TEARDOWN_IPC_REQ_MESSAGE_CHANNEL_ID_WORD_OFFSET (0)
#define IPC_MSG_TEARDOWN_IPC_REQ_MESSAGE_CHANNEL_ID_GET(ipc_msg_teardown_ipc_req_ptr) ((ipc_msg_teardown_ipc_req_ptr)->_data[0])
#define IPC_MSG_TEARDOWN_IPC_REQ_MESSAGE_CHANNEL_ID_SET(ipc_msg_teardown_ipc_req_ptr, message_channel_id) ((ipc_msg_teardown_ipc_req_ptr)->_data[0] = (uint16)(message_channel_id))
#define IPC_MSG_TEARDOWN_IPC_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_TEARDOWN_IPC_REQ_CREATE(message_channel_id) \
    (uint16)(message_channel_id)
#define IPC_MSG_TEARDOWN_IPC_REQ_PACK(ipc_msg_teardown_ipc_req_ptr, message_channel_id) \
    do { \
        (ipc_msg_teardown_ipc_req_ptr)->_data[0] = (uint16)((uint16)(message_channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_TEARDOWN_IPC_RES

  DESCRIPTION

  MEMBERS
    status             - status code
    message_channel_id - Message Channel ID as returned by ipc_setup_comms

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} IPC_MSG_TEARDOWN_IPC_RES;

/* The following macros take IPC_MSG_TEARDOWN_IPC_RES *ipc_msg_teardown_ipc_res_ptr */
#define IPC_MSG_TEARDOWN_IPC_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_TEARDOWN_IPC_RES_STATUS_GET(ipc_msg_teardown_ipc_res_ptr) ((ipc_msg_teardown_ipc_res_ptr)->_data[0])
#define IPC_MSG_TEARDOWN_IPC_RES_STATUS_SET(ipc_msg_teardown_ipc_res_ptr, status) ((ipc_msg_teardown_ipc_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_TEARDOWN_IPC_RES_MESSAGE_CHANNEL_ID_WORD_OFFSET (1)
#define IPC_MSG_TEARDOWN_IPC_RES_MESSAGE_CHANNEL_ID_GET(ipc_msg_teardown_ipc_res_ptr) ((ipc_msg_teardown_ipc_res_ptr)->_data[1])
#define IPC_MSG_TEARDOWN_IPC_RES_MESSAGE_CHANNEL_ID_SET(ipc_msg_teardown_ipc_res_ptr, message_channel_id) ((ipc_msg_teardown_ipc_res_ptr)->_data[1] = (uint16)(message_channel_id))
#define IPC_MSG_TEARDOWN_IPC_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_TEARDOWN_IPC_RES_CREATE(status, message_channel_id) \
    (uint16)(status), \
    (uint16)(message_channel_id)
#define IPC_MSG_TEARDOWN_IPC_RES_PACK(ipc_msg_teardown_ipc_res_ptr, status, message_channel_id) \
    do { \
        (ipc_msg_teardown_ipc_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (ipc_msg_teardown_ipc_res_ptr)->_data[1] = (uint16)((uint16)(message_channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    IPC_MSG_WATCHDOG_PING_RES

  DESCRIPTION

  MEMBERS
    status - status code

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} IPC_MSG_WATCHDOG_PING_RES;

/* The following macros take IPC_MSG_WATCHDOG_PING_RES *ipc_msg_watchdog_ping_res_ptr */
#define IPC_MSG_WATCHDOG_PING_RES_STATUS_WORD_OFFSET (0)
#define IPC_MSG_WATCHDOG_PING_RES_STATUS_GET(ipc_msg_watchdog_ping_res_ptr) ((ipc_msg_watchdog_ping_res_ptr)->_data[0])
#define IPC_MSG_WATCHDOG_PING_RES_STATUS_SET(ipc_msg_watchdog_ping_res_ptr, status) ((ipc_msg_watchdog_ping_res_ptr)->_data[0] = (uint16)(status))
#define IPC_MSG_WATCHDOG_PING_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define IPC_MSG_WATCHDOG_PING_RES_CREATE(status) \
    (uint16)(status)
#define IPC_MSG_WATCHDOG_PING_RES_PACK(ipc_msg_watchdog_ping_res_ptr, status) \
    do { \
        (ipc_msg_watchdog_ping_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
    } while (0)


#endif /* IPC_MSG_PRIM_H */

