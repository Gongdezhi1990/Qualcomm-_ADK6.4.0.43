/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2017
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef KIP_MSG_PRIM_H
#define KIP_MSG_PRIM_H


#define KIP_MSG_PROTOCOL_HEADER_WORD                                    (0xC409)


/*******************************************************************************

  NAME
    KIP_MSG_ID

  DESCRIPTION
    **************** REQ IDs **************************** ****************
    RESP IDs **************************** Make sure that the responses are
    identical to their requests, but with bit 14 set. Leave a matching gap,
    unsolicited msg from op does not have RESP, but for easy debug match up
    the REQ/RESP ID values. Leave a matching gap, PS_SHUTDOWN* msg have no
    RESP, but for easy debug match up the REQ/RESP ID values. Leave a
    matching gap, HANDLE_EOF* msg have no RESP, but for easy debug match up
    the REQ/RESP ID values.

 VALUES
    CREATE_OPERATOR_REQ               -
    DESTROY_OPERATOR_REQ              -
    START_OPERATOR_REQ                -
    STOP_OPERATOR_REQ                 -
    RESET_OPERATOR_REQ                -
    OPERATOR_MSG_REQ                  -
    UNSOLICITED_FROM_OP_REQ           -
    STREAM_CONNECT_REQ                -
    STREAM_TRANSFORM_DISCONNECT_REQ   -
    STREAM_CREATE_ENDPOINTS_REQ       -
    STREAM_DESTROY_ENDPOINTS_REQ      -
    PS_READ_REQ                       -
    PS_WRITE_REQ                      -
    PS_DELETE_REQ                     -
    PS_SHUTDOWN_REQ                   -
    PS_SHUTDOWN_COMPLETE_REQ          -
    P1_RUN_UNTIL_PREPROC_OPERATOR_REQ -
    EXPORTED_CONSTANTS_ACTION_REQ     -
    HANDLE_EOF_REQ                    -
    GET_MEM_USAGE_REQ                 -
    TRANSFORM_LIST_REMOVE_ENTRY_REQ   -
    METADATA_CHANNEL_ACTIVATED_REQ    -
    PUBLISH_FAULT_REQ                 -
    SET_SYSTEM_KEY_REQ                -
    OPERATOR_GET_ENDPOINT_REQ         -
    OPERATOR_LICENSE_QUERY_REQ        -
    CREATE_OPERATOR_RES               -
    DESTROY_OPERATOR_RES              -
    START_OPERATOR_RES                -
    STOP_OPERATOR_RES                 -
    RESET_OPERATOR_RES                -
    OPERATOR_MSG_RES                  -
    STREAM_CONNECT_RES                -
    STREAM_TRANSFORM_DISCONNECT_RES   -
    STREAM_CREATE_ENDPOINTS_RES       -
    STREAM_DESTROY_ENDPOINTS_RES      -
    PS_READ_RES                       -
    PS_WRITE_RES                      -
    PS_DELETE_RES                     -
    P1_RUN_UNTIL_PREPROC_OPERATOR_RES -
    GET_MEM_USAGE_RES                 -
    TRANSFORM_LIST_REMOVE_ENTRY_RES   -
    METADATA_CHANNEL_ACTIVATED_RES    -
    PUBLISH_FAULT_RES                 -
    SET_SYSTEM_KEY_RES                -
    OPERATOR_GET_ENDPOINT_RES         -
    OPERATOR_LICENSE_QUERY_RES        -

*******************************************************************************/
typedef enum
{
    KIP_MSG_ID_CREATE_OPERATOR_REQ = 0x1000,
    KIP_MSG_ID_DESTROY_OPERATOR_REQ = 0x1001,
    KIP_MSG_ID_START_OPERATOR_REQ = 0x1002,
    KIP_MSG_ID_STOP_OPERATOR_REQ = 0x1003,
    KIP_MSG_ID_RESET_OPERATOR_REQ = 0x1004,
    KIP_MSG_ID_OPERATOR_MSG_REQ = 0x1005,
    KIP_MSG_ID_UNSOLICITED_FROM_OP_REQ = 0x1006,
    KIP_MSG_ID_STREAM_CONNECT_REQ = 0x1007,
    KIP_MSG_ID_STREAM_TRANSFORM_DISCONNECT_REQ = 0x1008,
    KIP_MSG_ID_STREAM_CREATE_ENDPOINTS_REQ = 0x1009,
    KIP_MSG_ID_STREAM_DESTROY_ENDPOINTS_REQ = 0x100A,
    KIP_MSG_ID_PS_READ_REQ = 0x100B,
    KIP_MSG_ID_PS_WRITE_REQ = 0x100C,
    KIP_MSG_ID_PS_DELETE_REQ = 0x100D,
    KIP_MSG_ID_PS_SHUTDOWN_REQ = 0x100E,
    KIP_MSG_ID_PS_SHUTDOWN_COMPLETE_REQ = 0x100F,
    KIP_MSG_ID_P1_RUN_UNTIL_PREPROC_OPERATOR_REQ = 0x1010,
    KIP_MSG_ID_EXPORTED_CONSTANTS_ACTION_REQ = 0x1011,
    KIP_MSG_ID_HANDLE_EOF_REQ = 0x1012,
    KIP_MSG_ID_GET_MEM_USAGE_REQ = 0x1013,
    KIP_MSG_ID_TRANSFORM_LIST_REMOVE_ENTRY_REQ = 0x1014,
    KIP_MSG_ID_METADATA_CHANNEL_ACTIVATED_REQ = 0x1015,
    KIP_MSG_ID_PUBLISH_FAULT_REQ = 0x1016,
    KIP_MSG_ID_SET_SYSTEM_KEY_REQ = 0x1017,
    KIP_MSG_ID_OPERATOR_GET_ENDPOINT_REQ = 0x1018,
    KIP_MSG_ID_OPERATOR_LICENSE_QUERY_REQ = 0x1019,
    KIP_MSG_ID_CREATE_OPERATOR_RES = 0x5000,
    KIP_MSG_ID_DESTROY_OPERATOR_RES = 0x5001,
    KIP_MSG_ID_START_OPERATOR_RES = 0x5002,
    KIP_MSG_ID_STOP_OPERATOR_RES = 0x5003,
    KIP_MSG_ID_RESET_OPERATOR_RES = 0x5004,
    KIP_MSG_ID_OPERATOR_MSG_RES = 0x5005,
    KIP_MSG_ID_STREAM_CONNECT_RES = 0x5007,
    KIP_MSG_ID_STREAM_TRANSFORM_DISCONNECT_RES = 0x5008,
    KIP_MSG_ID_STREAM_CREATE_ENDPOINTS_RES = 0x5009,
    KIP_MSG_ID_STREAM_DESTROY_ENDPOINTS_RES = 0x500A,
    KIP_MSG_ID_PS_READ_RES = 0x500B,
    KIP_MSG_ID_PS_WRITE_RES = 0x500C,
    KIP_MSG_ID_PS_DELETE_RES = 0x500D,
    KIP_MSG_ID_P1_RUN_UNTIL_PREPROC_OPERATOR_RES = 0x5010,
    KIP_MSG_ID_GET_MEM_USAGE_RES = 0x5013,
    KIP_MSG_ID_TRANSFORM_LIST_REMOVE_ENTRY_RES = 0x5014,
    KIP_MSG_ID_METADATA_CHANNEL_ACTIVATED_RES = 0x5015,
    KIP_MSG_ID_PUBLISH_FAULT_RES = 0x5016,
    KIP_MSG_ID_SET_SYSTEM_KEY_RES = 0x5017,
    KIP_MSG_ID_OPERATOR_GET_ENDPOINT_RES = 0x5018,
    KIP_MSG_ID_OPERATOR_LICENSE_QUERY_RES = 0x5019
} KIP_MSG_ID;


#define KIP_MSG_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    KIP_MSG_CREATE_OPERATOR_REQ

  DESCRIPTION

  MEMBERS
    con_id        - connection id with sender/receiver processor and SW client
                    IDs as per common adaptor definition
    Capability_ID - The type of capability being requested
    Op_ID         - OP ID being imposed from P0
    num_keys      - Number of configuration key-value pairs
    info          - An array of key-value objects

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} KIP_MSG_CREATE_OPERATOR_REQ;

/* The following macros take KIP_MSG_CREATE_OPERATOR_REQ *kip_msg_create_operator_req_ptr */
#define KIP_MSG_CREATE_OPERATOR_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_CREATE_OPERATOR_REQ_CON_ID_GET(kip_msg_create_operator_req_ptr) ((kip_msg_create_operator_req_ptr)->_data[0])
#define KIP_MSG_CREATE_OPERATOR_REQ_CON_ID_SET(kip_msg_create_operator_req_ptr, con_id) ((kip_msg_create_operator_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_CREATE_OPERATOR_REQ_CAPABILITY_ID_WORD_OFFSET (1)
#define KIP_MSG_CREATE_OPERATOR_REQ_CAPABILITY_ID_GET(kip_msg_create_operator_req_ptr) ((kip_msg_create_operator_req_ptr)->_data[1])
#define KIP_MSG_CREATE_OPERATOR_REQ_CAPABILITY_ID_SET(kip_msg_create_operator_req_ptr, capability_id) ((kip_msg_create_operator_req_ptr)->_data[1] = (uint16)(capability_id))
#define KIP_MSG_CREATE_OPERATOR_REQ_OP_ID_WORD_OFFSET (2)
#define KIP_MSG_CREATE_OPERATOR_REQ_OP_ID_GET(kip_msg_create_operator_req_ptr) ((kip_msg_create_operator_req_ptr)->_data[2])
#define KIP_MSG_CREATE_OPERATOR_REQ_OP_ID_SET(kip_msg_create_operator_req_ptr, op_id) ((kip_msg_create_operator_req_ptr)->_data[2] = (uint16)(op_id))
#define KIP_MSG_CREATE_OPERATOR_REQ_NUM_KEYS_WORD_OFFSET (3)
#define KIP_MSG_CREATE_OPERATOR_REQ_NUM_KEYS_GET(kip_msg_create_operator_req_ptr) ((kip_msg_create_operator_req_ptr)->_data[3])
#define KIP_MSG_CREATE_OPERATOR_REQ_NUM_KEYS_SET(kip_msg_create_operator_req_ptr, num_keys) ((kip_msg_create_operator_req_ptr)->_data[3] = (uint16)(num_keys))
#define KIP_MSG_CREATE_OPERATOR_REQ_INFO_WORD_OFFSET (4)
#define KIP_MSG_CREATE_OPERATOR_REQ_INFO_GET(kip_msg_create_operator_req_ptr) ((kip_msg_create_operator_req_ptr)->_data[4])
#define KIP_MSG_CREATE_OPERATOR_REQ_INFO_SET(kip_msg_create_operator_req_ptr, info) ((kip_msg_create_operator_req_ptr)->_data[4] = (uint16)(info))
#define KIP_MSG_CREATE_OPERATOR_REQ_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_CREATE_OPERATOR_REQ_CREATE(con_id, Capability_ID, Op_ID, num_keys, info) \
    (uint16)(con_id), \
    (uint16)(Capability_ID), \
    (uint16)(Op_ID), \
    (uint16)(num_keys), \
    (uint16)(info)
#define KIP_MSG_CREATE_OPERATOR_REQ_PACK(kip_msg_create_operator_req_ptr, con_id, Capability_ID, Op_ID, num_keys, info) \
    do { \
        (kip_msg_create_operator_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_create_operator_req_ptr)->_data[1] = (uint16)((uint16)(Capability_ID)); \
        (kip_msg_create_operator_req_ptr)->_data[2] = (uint16)((uint16)(Op_ID)); \
        (kip_msg_create_operator_req_ptr)->_data[3] = (uint16)((uint16)(num_keys)); \
        (kip_msg_create_operator_req_ptr)->_data[4] = (uint16)((uint16)(info)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_CREATE_OPERATOR_RES

  DESCRIPTION

  MEMBERS
    status              - status code
    con_id              - connection id with sender/receiver processor and SW
                          client IDs as per common adaptor definition
    opid_or_reason_code - Operator ID if successful, error reason code if failure

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_CREATE_OPERATOR_RES;

/* The following macros take KIP_MSG_CREATE_OPERATOR_RES *kip_msg_create_operator_res_ptr */
#define KIP_MSG_CREATE_OPERATOR_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_CREATE_OPERATOR_RES_STATUS_GET(kip_msg_create_operator_res_ptr) ((kip_msg_create_operator_res_ptr)->_data[0])
#define KIP_MSG_CREATE_OPERATOR_RES_STATUS_SET(kip_msg_create_operator_res_ptr, status) ((kip_msg_create_operator_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_CREATE_OPERATOR_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_CREATE_OPERATOR_RES_CON_ID_GET(kip_msg_create_operator_res_ptr) ((kip_msg_create_operator_res_ptr)->_data[1])
#define KIP_MSG_CREATE_OPERATOR_RES_CON_ID_SET(kip_msg_create_operator_res_ptr, con_id) ((kip_msg_create_operator_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_CREATE_OPERATOR_RES_OPID_OR_REASON_CODE_WORD_OFFSET (2)
#define KIP_MSG_CREATE_OPERATOR_RES_OPID_OR_REASON_CODE_GET(kip_msg_create_operator_res_ptr) ((kip_msg_create_operator_res_ptr)->_data[2])
#define KIP_MSG_CREATE_OPERATOR_RES_OPID_OR_REASON_CODE_SET(kip_msg_create_operator_res_ptr, opid_or_reason_code) ((kip_msg_create_operator_res_ptr)->_data[2] = (uint16)(opid_or_reason_code))
#define KIP_MSG_CREATE_OPERATOR_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_CREATE_OPERATOR_RES_CREATE(status, con_id, opid_or_reason_code) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(opid_or_reason_code)
#define KIP_MSG_CREATE_OPERATOR_RES_PACK(kip_msg_create_operator_res_ptr, status, con_id, opid_or_reason_code) \
    do { \
        (kip_msg_create_operator_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_create_operator_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_create_operator_res_ptr)->_data[2] = (uint16)((uint16)(opid_or_reason_code)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_DESTROY_OPERATORS_REQ

  DESCRIPTION

  MEMBERS
    con_id       - connection id with sender/receiver processor and SW client
                   IDs as per common adaptor definition
    Count        - The number of operators in the list to destroy
    Destroy_list - The list of operators to destroy

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_DESTROY_OPERATORS_REQ;

/* The following macros take KIP_MSG_DESTROY_OPERATORS_REQ *kip_msg_destroy_operators_req_ptr */
#define KIP_MSG_DESTROY_OPERATORS_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_DESTROY_OPERATORS_REQ_CON_ID_GET(kip_msg_destroy_operators_req_ptr) ((kip_msg_destroy_operators_req_ptr)->_data[0])
#define KIP_MSG_DESTROY_OPERATORS_REQ_CON_ID_SET(kip_msg_destroy_operators_req_ptr, con_id) ((kip_msg_destroy_operators_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_DESTROY_OPERATORS_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_DESTROY_OPERATORS_REQ_COUNT_GET(kip_msg_destroy_operators_req_ptr) ((kip_msg_destroy_operators_req_ptr)->_data[1])
#define KIP_MSG_DESTROY_OPERATORS_REQ_COUNT_SET(kip_msg_destroy_operators_req_ptr, count) ((kip_msg_destroy_operators_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_DESTROY_OPERATORS_REQ_DESTROY_LIST_WORD_OFFSET (2)
#define KIP_MSG_DESTROY_OPERATORS_REQ_DESTROY_LIST_GET(kip_msg_destroy_operators_req_ptr) ((kip_msg_destroy_operators_req_ptr)->_data[2])
#define KIP_MSG_DESTROY_OPERATORS_REQ_DESTROY_LIST_SET(kip_msg_destroy_operators_req_ptr, destroy_list) ((kip_msg_destroy_operators_req_ptr)->_data[2] = (uint16)(destroy_list))
#define KIP_MSG_DESTROY_OPERATORS_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_DESTROY_OPERATORS_REQ_CREATE(con_id, Count, Destroy_list) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(Destroy_list)
#define KIP_MSG_DESTROY_OPERATORS_REQ_PACK(kip_msg_destroy_operators_req_ptr, con_id, Count, Destroy_list) \
    do { \
        (kip_msg_destroy_operators_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_destroy_operators_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_destroy_operators_req_ptr)->_data[2] = (uint16)((uint16)(Destroy_list)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_DESTROY_OPERATORS_RES

  DESCRIPTION

  MEMBERS
    status      - status code
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Count       - The number of operators destroyed
    reason_code - Error reason code if not all operators destroyed

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_DESTROY_OPERATORS_RES;

/* The following macros take KIP_MSG_DESTROY_OPERATORS_RES *kip_msg_destroy_operators_res_ptr */
#define KIP_MSG_DESTROY_OPERATORS_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_DESTROY_OPERATORS_RES_STATUS_GET(kip_msg_destroy_operators_res_ptr) ((kip_msg_destroy_operators_res_ptr)->_data[0])
#define KIP_MSG_DESTROY_OPERATORS_RES_STATUS_SET(kip_msg_destroy_operators_res_ptr, status) ((kip_msg_destroy_operators_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_DESTROY_OPERATORS_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_DESTROY_OPERATORS_RES_CON_ID_GET(kip_msg_destroy_operators_res_ptr) ((kip_msg_destroy_operators_res_ptr)->_data[1])
#define KIP_MSG_DESTROY_OPERATORS_RES_CON_ID_SET(kip_msg_destroy_operators_res_ptr, con_id) ((kip_msg_destroy_operators_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_DESTROY_OPERATORS_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_DESTROY_OPERATORS_RES_COUNT_GET(kip_msg_destroy_operators_res_ptr) ((kip_msg_destroy_operators_res_ptr)->_data[2])
#define KIP_MSG_DESTROY_OPERATORS_RES_COUNT_SET(kip_msg_destroy_operators_res_ptr, count) ((kip_msg_destroy_operators_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_DESTROY_OPERATORS_RES_REASON_CODE_WORD_OFFSET (3)
#define KIP_MSG_DESTROY_OPERATORS_RES_REASON_CODE_GET(kip_msg_destroy_operators_res_ptr) ((kip_msg_destroy_operators_res_ptr)->_data[3])
#define KIP_MSG_DESTROY_OPERATORS_RES_REASON_CODE_SET(kip_msg_destroy_operators_res_ptr, reason_code) ((kip_msg_destroy_operators_res_ptr)->_data[3] = (uint16)(reason_code))
#define KIP_MSG_DESTROY_OPERATORS_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_DESTROY_OPERATORS_RES_CREATE(status, con_id, Count, reason_code) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(reason_code)
#define KIP_MSG_DESTROY_OPERATORS_RES_PACK(kip_msg_destroy_operators_res_ptr, status, con_id, Count, reason_code) \
    do { \
        (kip_msg_destroy_operators_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_destroy_operators_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_destroy_operators_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
        (kip_msg_destroy_operators_res_ptr)->_data[3] = (uint16)((uint16)(reason_code)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ

  DESCRIPTION

  MEMBERS
    con_id        - connection id with sender/receiver processor and SW client
                    IDs as per common adaptor definition
    mi            - message id as would be used with put_message
    constant_p    - value of the pointer to the constant table in the ROM
    operator_id   - ID of the operator triggering the request
    callback_info - Value of a pointer to a structure, internal to the
                    exported_constants code, that has all the information
                    required to initiate a callback when a set of constants have
                    been processed. The pointer need not be in globally readable
                    memory as it is returned to the origin.

*******************************************************************************/
typedef struct
{
    uint16 _data[8];
} KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ;

/* The following macros take KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ *kip_msg_exported_constants_action_req_ptr */
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CON_ID_GET(kip_msg_exported_constants_action_req_ptr) ((kip_msg_exported_constants_action_req_ptr)->_data[0])
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CON_ID_SET(kip_msg_exported_constants_action_req_ptr, con_id) ((kip_msg_exported_constants_action_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_MI_WORD_OFFSET (1)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_MI_GET(kip_msg_exported_constants_action_req_ptr) ((kip_msg_exported_constants_action_req_ptr)->_data[1])
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_MI_SET(kip_msg_exported_constants_action_req_ptr, mi) ((kip_msg_exported_constants_action_req_ptr)->_data[1] = (uint16)(mi))
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CONSTANT_P_WORD_OFFSET (2)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CONSTANT_P_GET(kip_msg_exported_constants_action_req_ptr)  \
    (((uint32)((kip_msg_exported_constants_action_req_ptr)->_data[2]) | \
      ((uint32)((kip_msg_exported_constants_action_req_ptr)->_data[3]) << 16)))
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CONSTANT_P_SET(kip_msg_exported_constants_action_req_ptr, constant_p) do { \
        (kip_msg_exported_constants_action_req_ptr)->_data[2] = (uint16)((constant_p) & 0xffff); \
        (kip_msg_exported_constants_action_req_ptr)->_data[3] = (uint16)((constant_p) >> 16); } while (0)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_OPERATOR_ID_WORD_OFFSET (4)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_OPERATOR_ID_GET(kip_msg_exported_constants_action_req_ptr)  \
    (((uint32)((kip_msg_exported_constants_action_req_ptr)->_data[4]) | \
      ((uint32)((kip_msg_exported_constants_action_req_ptr)->_data[5]) << 16)))
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_OPERATOR_ID_SET(kip_msg_exported_constants_action_req_ptr, operator_id) do { \
        (kip_msg_exported_constants_action_req_ptr)->_data[4] = (uint16)((operator_id) & 0xffff); \
        (kip_msg_exported_constants_action_req_ptr)->_data[5] = (uint16)((operator_id) >> 16); } while (0)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CALLBACK_INFO_WORD_OFFSET (6)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CALLBACK_INFO_GET(kip_msg_exported_constants_action_req_ptr)  \
    (((uint32)((kip_msg_exported_constants_action_req_ptr)->_data[6]) | \
      ((uint32)((kip_msg_exported_constants_action_req_ptr)->_data[7]) << 16)))
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CALLBACK_INFO_SET(kip_msg_exported_constants_action_req_ptr, callback_info) do { \
        (kip_msg_exported_constants_action_req_ptr)->_data[6] = (uint16)((callback_info) & 0xffff); \
        (kip_msg_exported_constants_action_req_ptr)->_data[7] = (uint16)((callback_info) >> 16); } while (0)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_WORD_SIZE (8)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CREATE(con_id, mi, constant_p, operator_id, callback_info) \
    (uint16)(con_id), \
    (uint16)(mi), \
    (uint16)((constant_p) & 0xffff), \
    (uint16)((constant_p) >> 16), \
    (uint16)((operator_id) & 0xffff), \
    (uint16)((operator_id) >> 16), \
    (uint16)((callback_info) & 0xffff), \
    (uint16)((callback_info) >> 16)
#define KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_PACK(kip_msg_exported_constants_action_req_ptr, con_id, mi, constant_p, operator_id, callback_info) \
    do { \
        (kip_msg_exported_constants_action_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_exported_constants_action_req_ptr)->_data[1] = (uint16)((uint16)(mi)); \
        (kip_msg_exported_constants_action_req_ptr)->_data[2] = (uint16)((uint16)((constant_p) & 0xffff)); \
        (kip_msg_exported_constants_action_req_ptr)->_data[3] = (uint16)(((constant_p) >> 16)); \
        (kip_msg_exported_constants_action_req_ptr)->_data[4] = (uint16)((uint16)((operator_id) & 0xffff)); \
        (kip_msg_exported_constants_action_req_ptr)->_data[5] = (uint16)(((operator_id) >> 16)); \
        (kip_msg_exported_constants_action_req_ptr)->_data[6] = (uint16)((uint16)((callback_info) & 0xffff)); \
        (kip_msg_exported_constants_action_req_ptr)->_data[7] = (uint16)(((callback_info) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_GET_MEM_USAGE_REQ

  DESCRIPTION
    Get memory usage of secondary processor.

  MEMBERS
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} KIP_MSG_GET_MEM_USAGE_REQ;

/* The following macros take KIP_MSG_GET_MEM_USAGE_REQ *kip_msg_get_mem_usage_req_ptr */
#define KIP_MSG_GET_MEM_USAGE_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_GET_MEM_USAGE_REQ_CON_ID_GET(kip_msg_get_mem_usage_req_ptr) ((kip_msg_get_mem_usage_req_ptr)->_data[0])
#define KIP_MSG_GET_MEM_USAGE_REQ_CON_ID_SET(kip_msg_get_mem_usage_req_ptr, con_id) ((kip_msg_get_mem_usage_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_GET_MEM_USAGE_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_GET_MEM_USAGE_REQ_CREATE(con_id) \
    (uint16)(con_id)
#define KIP_MSG_GET_MEM_USAGE_REQ_PACK(kip_msg_get_mem_usage_req_ptr, con_id) \
    do { \
        (kip_msg_get_mem_usage_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_GET_MEM_USAGE_RES

  DESCRIPTION

  MEMBERS
    status       - status code
    con_id       - connection id with sender/receiver processor and SW client
                   IDs as per common adaptor definition
    heap_size    - The heap size in words.
    heap_current - Current heap usage in words.
    heap_min     - Minimum available heap in words.
    pool_size    - The pool memory size in words.
    pool_current - Current pool memory usage in words.
    pool_min     - Minimum available pool memory in words.

*******************************************************************************/
typedef struct
{
    uint16 _data[14];
} KIP_MSG_GET_MEM_USAGE_RES;

/* The following macros take KIP_MSG_GET_MEM_USAGE_RES *kip_msg_get_mem_usage_res_ptr */
#define KIP_MSG_GET_MEM_USAGE_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_GET_MEM_USAGE_RES_STATUS_GET(kip_msg_get_mem_usage_res_ptr) ((kip_msg_get_mem_usage_res_ptr)->_data[0])
#define KIP_MSG_GET_MEM_USAGE_RES_STATUS_SET(kip_msg_get_mem_usage_res_ptr, status) ((kip_msg_get_mem_usage_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_GET_MEM_USAGE_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_GET_MEM_USAGE_RES_CON_ID_GET(kip_msg_get_mem_usage_res_ptr) ((kip_msg_get_mem_usage_res_ptr)->_data[1])
#define KIP_MSG_GET_MEM_USAGE_RES_CON_ID_SET(kip_msg_get_mem_usage_res_ptr, con_id) ((kip_msg_get_mem_usage_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_SIZE_WORD_OFFSET (2)
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_SIZE_GET(kip_msg_get_mem_usage_res_ptr)  \
    (((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[2]) | \
      ((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[3]) << 16)))
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_SIZE_SET(kip_msg_get_mem_usage_res_ptr, heap_size) do { \
        (kip_msg_get_mem_usage_res_ptr)->_data[2] = (uint16)((heap_size) & 0xffff); \
        (kip_msg_get_mem_usage_res_ptr)->_data[3] = (uint16)((heap_size) >> 16); } while (0)
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_CURRENT_WORD_OFFSET (4)
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_CURRENT_GET(kip_msg_get_mem_usage_res_ptr)  \
    (((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[4]) | \
      ((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[5]) << 16)))
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_CURRENT_SET(kip_msg_get_mem_usage_res_ptr, heap_current) do { \
        (kip_msg_get_mem_usage_res_ptr)->_data[4] = (uint16)((heap_current) & 0xffff); \
        (kip_msg_get_mem_usage_res_ptr)->_data[5] = (uint16)((heap_current) >> 16); } while (0)
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_MIN_WORD_OFFSET (6)
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_MIN_GET(kip_msg_get_mem_usage_res_ptr)  \
    (((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[6]) | \
      ((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[7]) << 16)))
#define KIP_MSG_GET_MEM_USAGE_RES_HEAP_MIN_SET(kip_msg_get_mem_usage_res_ptr, heap_min) do { \
        (kip_msg_get_mem_usage_res_ptr)->_data[6] = (uint16)((heap_min) & 0xffff); \
        (kip_msg_get_mem_usage_res_ptr)->_data[7] = (uint16)((heap_min) >> 16); } while (0)
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_SIZE_WORD_OFFSET (8)
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_SIZE_GET(kip_msg_get_mem_usage_res_ptr)  \
    (((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[8]) | \
      ((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[8 + 1]) << 16)))
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_SIZE_SET(kip_msg_get_mem_usage_res_ptr, pool_size) do { \
        (kip_msg_get_mem_usage_res_ptr)->_data[8] = (uint16)((pool_size) & 0xffff); \
        (kip_msg_get_mem_usage_res_ptr)->_data[8 + 1] = (uint16)((pool_size) >> 16); } while (0)
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_CURRENT_WORD_OFFSET (10)
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_CURRENT_GET(kip_msg_get_mem_usage_res_ptr)  \
    (((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[10]) | \
      ((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[11]) << 16)))
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_CURRENT_SET(kip_msg_get_mem_usage_res_ptr, pool_current) do { \
        (kip_msg_get_mem_usage_res_ptr)->_data[10] = (uint16)((pool_current) & 0xffff); \
        (kip_msg_get_mem_usage_res_ptr)->_data[11] = (uint16)((pool_current) >> 16); } while (0)
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_MIN_WORD_OFFSET (12)
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_MIN_GET(kip_msg_get_mem_usage_res_ptr)  \
    (((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[12]) | \
      ((uint32)((kip_msg_get_mem_usage_res_ptr)->_data[13]) << 16)))
#define KIP_MSG_GET_MEM_USAGE_RES_POOL_MIN_SET(kip_msg_get_mem_usage_res_ptr, pool_min) do { \
        (kip_msg_get_mem_usage_res_ptr)->_data[12] = (uint16)((pool_min) & 0xffff); \
        (kip_msg_get_mem_usage_res_ptr)->_data[13] = (uint16)((pool_min) >> 16); } while (0)
#define KIP_MSG_GET_MEM_USAGE_RES_WORD_SIZE (14)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_GET_MEM_USAGE_RES_CREATE(status, con_id, heap_size, heap_current, heap_min, pool_size, pool_current, pool_min) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)((heap_size) & 0xffff), \
    (uint16)((heap_size) >> 16), \
    (uint16)((heap_current) & 0xffff), \
    (uint16)((heap_current) >> 16), \
    (uint16)((heap_min) & 0xffff), \
    (uint16)((heap_min) >> 16), \
    (uint16)((pool_size) & 0xffff), \
    (uint16)((pool_size) >> 16), \
    (uint16)((pool_current) & 0xffff), \
    (uint16)((pool_current) >> 16), \
    (uint16)((pool_min) & 0xffff), \
    (uint16)((pool_min) >> 16)
#define KIP_MSG_GET_MEM_USAGE_RES_PACK(kip_msg_get_mem_usage_res_ptr, status, con_id, heap_size, heap_current, heap_min, pool_size, pool_current, pool_min) \
    do { \
        (kip_msg_get_mem_usage_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[2] = (uint16)((uint16)((heap_size) & 0xffff)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[3] = (uint16)(((heap_size) >> 16)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[4] = (uint16)((uint16)((heap_current) & 0xffff)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[5] = (uint16)(((heap_current) >> 16)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[6] = (uint16)((uint16)((heap_min) & 0xffff)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[7] = (uint16)(((heap_min) >> 16)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[8] = (uint16)((uint16)((pool_size) & 0xffff)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[8 + 1] = (uint16)(((pool_size) >> 16)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[10] = (uint16)((uint16)((pool_current) & 0xffff)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[11] = (uint16)(((pool_current) >> 16)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[12] = (uint16)((uint16)((pool_min) & 0xffff)); \
        (kip_msg_get_mem_usage_res_ptr)->_data[13] = (uint16)(((pool_min) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_HANDLE_EOF_REQ

  DESCRIPTION
    Handle EOF requests from the partner processor.

  MEMBERS
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    cb_ref - Reference to the metadata EOF callback data.

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_HANDLE_EOF_REQ;

/* The following macros take KIP_MSG_HANDLE_EOF_REQ *kip_msg_handle_eof_req_ptr */
#define KIP_MSG_HANDLE_EOF_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_HANDLE_EOF_REQ_CON_ID_GET(kip_msg_handle_eof_req_ptr) ((kip_msg_handle_eof_req_ptr)->_data[0])
#define KIP_MSG_HANDLE_EOF_REQ_CON_ID_SET(kip_msg_handle_eof_req_ptr, con_id) ((kip_msg_handle_eof_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_HANDLE_EOF_REQ_CB_REF_WORD_OFFSET (1)
#define KIP_MSG_HANDLE_EOF_REQ_CB_REF_GET(kip_msg_handle_eof_req_ptr)  \
    (((uint32)((kip_msg_handle_eof_req_ptr)->_data[1]) | \
      ((uint32)((kip_msg_handle_eof_req_ptr)->_data[2]) << 16)))
#define KIP_MSG_HANDLE_EOF_REQ_CB_REF_SET(kip_msg_handle_eof_req_ptr, cb_ref) do { \
        (kip_msg_handle_eof_req_ptr)->_data[1] = (uint16)((cb_ref) & 0xffff); \
        (kip_msg_handle_eof_req_ptr)->_data[2] = (uint16)((cb_ref) >> 16); } while (0)
#define KIP_MSG_HANDLE_EOF_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_HANDLE_EOF_REQ_CREATE(con_id, cb_ref) \
    (uint16)(con_id), \
    (uint16)((cb_ref) & 0xffff), \
    (uint16)((cb_ref) >> 16)
#define KIP_MSG_HANDLE_EOF_REQ_PACK(kip_msg_handle_eof_req_ptr, con_id, cb_ref) \
    do { \
        (kip_msg_handle_eof_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_handle_eof_req_ptr)->_data[1] = (uint16)((uint16)((cb_ref) & 0xffff)); \
        (kip_msg_handle_eof_req_ptr)->_data[2] = (uint16)(((cb_ref) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_MESSAGE_FROM_OPERATOR_REQ

  DESCRIPTION

  MEMBERS
    con_id  - connection id with sender/receiver processor and SW client IDs as
              per common adaptor definition
    opid    - The ID of the operator that sent the message
    message - The message that the operator sent

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_MESSAGE_FROM_OPERATOR_REQ;

/* The following macros take KIP_MSG_MESSAGE_FROM_OPERATOR_REQ *kip_msg_message_from_operator_req_ptr */
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_CON_ID_GET(kip_msg_message_from_operator_req_ptr) ((kip_msg_message_from_operator_req_ptr)->_data[0])
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_CON_ID_SET(kip_msg_message_from_operator_req_ptr, con_id) ((kip_msg_message_from_operator_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_OPID_WORD_OFFSET (1)
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_OPID_GET(kip_msg_message_from_operator_req_ptr) ((kip_msg_message_from_operator_req_ptr)->_data[1])
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_OPID_SET(kip_msg_message_from_operator_req_ptr, opid) ((kip_msg_message_from_operator_req_ptr)->_data[1] = (uint16)(opid))
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_WORD_OFFSET (2)
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_GET(kip_msg_message_from_operator_req_ptr) ((kip_msg_message_from_operator_req_ptr)->_data[2])
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_SET(kip_msg_message_from_operator_req_ptr, message) ((kip_msg_message_from_operator_req_ptr)->_data[2] = (uint16)(message))
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_CREATE(con_id, opid, message) \
    (uint16)(con_id), \
    (uint16)(opid), \
    (uint16)(message)
#define KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_PACK(kip_msg_message_from_operator_req_ptr, con_id, opid, message) \
    do { \
        (kip_msg_message_from_operator_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_message_from_operator_req_ptr)->_data[1] = (uint16)((uint16)(opid)); \
        (kip_msg_message_from_operator_req_ptr)->_data[2] = (uint16)((uint16)(message)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ

  DESCRIPTION

  MEMBERS
    con_id     - connection id with sender/receiver processor and SW client IDs
                 as per common adaptor definition
    channel_id - metadata data channel id

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ;

/* The following macros take KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ *kip_msg_metadata_channel_activated_req_ptr */
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_CON_ID_GET(kip_msg_metadata_channel_activated_req_ptr) ((kip_msg_metadata_channel_activated_req_ptr)->_data[0])
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_CON_ID_SET(kip_msg_metadata_channel_activated_req_ptr, con_id) ((kip_msg_metadata_channel_activated_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_CHANNEL_ID_WORD_OFFSET (1)
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_CHANNEL_ID_GET(kip_msg_metadata_channel_activated_req_ptr) ((kip_msg_metadata_channel_activated_req_ptr)->_data[1])
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_CHANNEL_ID_SET(kip_msg_metadata_channel_activated_req_ptr, channel_id) ((kip_msg_metadata_channel_activated_req_ptr)->_data[1] = (uint16)(channel_id))
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_CREATE(con_id, channel_id) \
    (uint16)(con_id), \
    (uint16)(channel_id)
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_REQ_PACK(kip_msg_metadata_channel_activated_req_ptr, con_id, channel_id) \
    do { \
        (kip_msg_metadata_channel_activated_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_metadata_channel_activated_req_ptr)->_data[1] = (uint16)((uint16)(channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES

  DESCRIPTION

  MEMBERS
    status     - status code
    con_id     - connection id with sender/receiver processor and SW client IDs
                 as per common adaptor definition
    channel_id - metadata data channel id

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES;

/* The following macros take KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES *kip_msg_metadata_channel_activated_res_ptr */
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_STATUS_GET(kip_msg_metadata_channel_activated_res_ptr) ((kip_msg_metadata_channel_activated_res_ptr)->_data[0])
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_STATUS_SET(kip_msg_metadata_channel_activated_res_ptr, status) ((kip_msg_metadata_channel_activated_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_CON_ID_GET(kip_msg_metadata_channel_activated_res_ptr) ((kip_msg_metadata_channel_activated_res_ptr)->_data[1])
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_CON_ID_SET(kip_msg_metadata_channel_activated_res_ptr, con_id) ((kip_msg_metadata_channel_activated_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_CHANNEL_ID_WORD_OFFSET (2)
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_CHANNEL_ID_GET(kip_msg_metadata_channel_activated_res_ptr) ((kip_msg_metadata_channel_activated_res_ptr)->_data[2])
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_CHANNEL_ID_SET(kip_msg_metadata_channel_activated_res_ptr, channel_id) ((kip_msg_metadata_channel_activated_res_ptr)->_data[2] = (uint16)(channel_id))
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_CREATE(status, con_id, channel_id) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(channel_id)
#define KIP_MSG_METADATA_CHANNEL_ACTIVATED_RES_PACK(kip_msg_metadata_channel_activated_res_ptr, status, con_id, channel_id) \
    do { \
        (kip_msg_metadata_channel_activated_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_metadata_channel_activated_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_metadata_channel_activated_res_ptr)->_data[2] = (uint16)((uint16)(channel_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPERATOR_GET_ENDPOINT_REQ

  DESCRIPTION

  MEMBERS
    con_id    - Connection id with sender/receiver processor and SW client IDs
                as per common adaptor definition
    opid      - The operator id of which to request source/sink ep id
    idx       - The endpoint source/sink channel index of the operator
    direction - Endpoint direction (source/sink)

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_OPERATOR_GET_ENDPOINT_REQ;

/* The following macros take KIP_MSG_OPERATOR_GET_ENDPOINT_REQ *kip_msg_operator_get_endpoint_req_ptr */
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_CON_ID_GET(kip_msg_operator_get_endpoint_req_ptr) ((kip_msg_operator_get_endpoint_req_ptr)->_data[0])
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_CON_ID_SET(kip_msg_operator_get_endpoint_req_ptr, con_id) ((kip_msg_operator_get_endpoint_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_OPID_WORD_OFFSET (1)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_OPID_GET(kip_msg_operator_get_endpoint_req_ptr) ((kip_msg_operator_get_endpoint_req_ptr)->_data[1])
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_OPID_SET(kip_msg_operator_get_endpoint_req_ptr, opid) ((kip_msg_operator_get_endpoint_req_ptr)->_data[1] = (uint16)(opid))
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_IDX_WORD_OFFSET (2)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_IDX_GET(kip_msg_operator_get_endpoint_req_ptr) ((kip_msg_operator_get_endpoint_req_ptr)->_data[2])
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_IDX_SET(kip_msg_operator_get_endpoint_req_ptr, idx) ((kip_msg_operator_get_endpoint_req_ptr)->_data[2] = (uint16)(idx))
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_DIRECTION_WORD_OFFSET (3)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_DIRECTION_GET(kip_msg_operator_get_endpoint_req_ptr) ((kip_msg_operator_get_endpoint_req_ptr)->_data[3])
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_DIRECTION_SET(kip_msg_operator_get_endpoint_req_ptr, direction) ((kip_msg_operator_get_endpoint_req_ptr)->_data[3] = (uint16)(direction))
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_CREATE(con_id, opid, idx, direction) \
    (uint16)(con_id), \
    (uint16)(opid), \
    (uint16)(idx), \
    (uint16)(direction)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_REQ_PACK(kip_msg_operator_get_endpoint_req_ptr, con_id, opid, idx, direction) \
    do { \
        (kip_msg_operator_get_endpoint_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_operator_get_endpoint_req_ptr)->_data[1] = (uint16)((uint16)(opid)); \
        (kip_msg_operator_get_endpoint_req_ptr)->_data[2] = (uint16)((uint16)(idx)); \
        (kip_msg_operator_get_endpoint_req_ptr)->_data[3] = (uint16)((uint16)(direction)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPERATOR_GET_ENDPOINT_RES

  DESCRIPTION

  MEMBERS
    status - status code
    con_id - Connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    opepid - Operator's endpoint ID if successfull, 0 if not

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_OPERATOR_GET_ENDPOINT_RES;

/* The following macros take KIP_MSG_OPERATOR_GET_ENDPOINT_RES *kip_msg_operator_get_endpoint_res_ptr */
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_STATUS_GET(kip_msg_operator_get_endpoint_res_ptr) ((kip_msg_operator_get_endpoint_res_ptr)->_data[0])
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_STATUS_SET(kip_msg_operator_get_endpoint_res_ptr, status) ((kip_msg_operator_get_endpoint_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_CON_ID_GET(kip_msg_operator_get_endpoint_res_ptr) ((kip_msg_operator_get_endpoint_res_ptr)->_data[1])
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_CON_ID_SET(kip_msg_operator_get_endpoint_res_ptr, con_id) ((kip_msg_operator_get_endpoint_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_OPEPID_WORD_OFFSET (2)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_OPEPID_GET(kip_msg_operator_get_endpoint_res_ptr) ((kip_msg_operator_get_endpoint_res_ptr)->_data[2])
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_OPEPID_SET(kip_msg_operator_get_endpoint_res_ptr, opepid) ((kip_msg_operator_get_endpoint_res_ptr)->_data[2] = (uint16)(opepid))
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_CREATE(status, con_id, opepid) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(opepid)
#define KIP_MSG_OPERATOR_GET_ENDPOINT_RES_PACK(kip_msg_operator_get_endpoint_res_ptr, status, con_id, opepid) \
    do { \
        (kip_msg_operator_get_endpoint_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_operator_get_endpoint_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_operator_get_endpoint_res_ptr)->_data[2] = (uint16)((uint16)(opepid)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPERATOR_LICENSE_QUERY_REQ

  DESCRIPTION

  MEMBERS
    con_id  - Connection id with sender/receiver processor and SW client IDs as
              per common adaptor definition
    length  - The number of key-value pairs in the message.
    payload - An array of key-value objects

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_OPERATOR_LICENSE_QUERY_REQ;

/* The following macros take KIP_MSG_OPERATOR_LICENSE_QUERY_REQ *kip_msg_operator_license_query_req_ptr */
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_CON_ID_GET(kip_msg_operator_license_query_req_ptr) ((kip_msg_operator_license_query_req_ptr)->_data[0])
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_CON_ID_SET(kip_msg_operator_license_query_req_ptr, con_id) ((kip_msg_operator_license_query_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_LENGTH_WORD_OFFSET (1)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_LENGTH_GET(kip_msg_operator_license_query_req_ptr) ((kip_msg_operator_license_query_req_ptr)->_data[1])
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_LENGTH_SET(kip_msg_operator_license_query_req_ptr, length) ((kip_msg_operator_license_query_req_ptr)->_data[1] = (uint16)(length))
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_PAYLOAD_WORD_OFFSET (2)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_PAYLOAD_GET(kip_msg_operator_license_query_req_ptr)  \
    (((uint32)((kip_msg_operator_license_query_req_ptr)->_data[2]) | \
      ((uint32)((kip_msg_operator_license_query_req_ptr)->_data[3]) << 16)))
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_PAYLOAD_SET(kip_msg_operator_license_query_req_ptr, payload) do { \
        (kip_msg_operator_license_query_req_ptr)->_data[2] = (uint16)((payload) & 0xffff); \
        (kip_msg_operator_license_query_req_ptr)->_data[3] = (uint16)((payload) >> 16); } while (0)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_CREATE(con_id, length, payload) \
    (uint16)(con_id), \
    (uint16)(length), \
    (uint16)((payload) & 0xffff), \
    (uint16)((payload) >> 16)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_REQ_PACK(kip_msg_operator_license_query_req_ptr, con_id, length, payload) \
    do { \
        (kip_msg_operator_license_query_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_operator_license_query_req_ptr)->_data[1] = (uint16)((uint16)(length)); \
        (kip_msg_operator_license_query_req_ptr)->_data[2] = (uint16)((uint16)((payload) & 0xffff)); \
        (kip_msg_operator_license_query_req_ptr)->_data[3] = (uint16)(((payload) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPERATOR_LICENSE_QUERY_RES

  DESCRIPTION

  MEMBERS
    status  - status code
    con_id  - Connection id with sender/receiver processor and SW client IDs as
              per common adaptor definition
    length  - The number of key-value pairs in the message.
    payload - An array of key-value objects

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} KIP_MSG_OPERATOR_LICENSE_QUERY_RES;

/* The following macros take KIP_MSG_OPERATOR_LICENSE_QUERY_RES *kip_msg_operator_license_query_res_ptr */
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_STATUS_GET(kip_msg_operator_license_query_res_ptr) ((kip_msg_operator_license_query_res_ptr)->_data[0])
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_STATUS_SET(kip_msg_operator_license_query_res_ptr, status) ((kip_msg_operator_license_query_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_CON_ID_GET(kip_msg_operator_license_query_res_ptr) ((kip_msg_operator_license_query_res_ptr)->_data[1])
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_CON_ID_SET(kip_msg_operator_license_query_res_ptr, con_id) ((kip_msg_operator_license_query_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_LENGTH_WORD_OFFSET (2)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_LENGTH_GET(kip_msg_operator_license_query_res_ptr)  \
    (((uint32)((kip_msg_operator_license_query_res_ptr)->_data[2]) | \
      ((uint32)((kip_msg_operator_license_query_res_ptr)->_data[3]) << 16)))
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_LENGTH_SET(kip_msg_operator_license_query_res_ptr, length) do { \
        (kip_msg_operator_license_query_res_ptr)->_data[2] = (uint16)((length) & 0xffff); \
        (kip_msg_operator_license_query_res_ptr)->_data[3] = (uint16)((length) >> 16); } while (0)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_PAYLOAD_WORD_OFFSET (4)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_PAYLOAD_GET(kip_msg_operator_license_query_res_ptr)  \
    (((uint32)((kip_msg_operator_license_query_res_ptr)->_data[4]) | \
      ((uint32)((kip_msg_operator_license_query_res_ptr)->_data[5]) << 16)))
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_PAYLOAD_SET(kip_msg_operator_license_query_res_ptr, payload) do { \
        (kip_msg_operator_license_query_res_ptr)->_data[4] = (uint16)((payload) & 0xffff); \
        (kip_msg_operator_license_query_res_ptr)->_data[5] = (uint16)((payload) >> 16); } while (0)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_CREATE(status, con_id, length, payload) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)((length) & 0xffff), \
    (uint16)((length) >> 16), \
    (uint16)((payload) & 0xffff), \
    (uint16)((payload) >> 16)
#define KIP_MSG_OPERATOR_LICENSE_QUERY_RES_PACK(kip_msg_operator_license_query_res_ptr, status, con_id, length, payload) \
    do { \
        (kip_msg_operator_license_query_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_operator_license_query_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_operator_license_query_res_ptr)->_data[2] = (uint16)((uint16)((length) & 0xffff)); \
        (kip_msg_operator_license_query_res_ptr)->_data[3] = (uint16)(((length) >> 16)); \
        (kip_msg_operator_license_query_res_ptr)->_data[4] = (uint16)((uint16)((payload) & 0xffff)); \
        (kip_msg_operator_license_query_res_ptr)->_data[5] = (uint16)(((payload) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPERATOR_MESSAGE_REQ

  DESCRIPTION

  MEMBERS
    con_id     - connection id with sender/receiver processor and SW client IDs
                 as per common adaptor definition
    opid       - The operator to send the message to
    op_message - Message to send to the specified operator

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_OPERATOR_MESSAGE_REQ;

/* The following macros take KIP_MSG_OPERATOR_MESSAGE_REQ *kip_msg_operator_message_req_ptr */
#define KIP_MSG_OPERATOR_MESSAGE_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_OPERATOR_MESSAGE_REQ_CON_ID_GET(kip_msg_operator_message_req_ptr) ((kip_msg_operator_message_req_ptr)->_data[0])
#define KIP_MSG_OPERATOR_MESSAGE_REQ_CON_ID_SET(kip_msg_operator_message_req_ptr, con_id) ((kip_msg_operator_message_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_OPERATOR_MESSAGE_REQ_OPID_WORD_OFFSET (1)
#define KIP_MSG_OPERATOR_MESSAGE_REQ_OPID_GET(kip_msg_operator_message_req_ptr) ((kip_msg_operator_message_req_ptr)->_data[1])
#define KIP_MSG_OPERATOR_MESSAGE_REQ_OPID_SET(kip_msg_operator_message_req_ptr, opid) ((kip_msg_operator_message_req_ptr)->_data[1] = (uint16)(opid))
#define KIP_MSG_OPERATOR_MESSAGE_REQ_OP_MESSAGE_WORD_OFFSET (2)
#define KIP_MSG_OPERATOR_MESSAGE_REQ_OP_MESSAGE_GET(kip_msg_operator_message_req_ptr) ((kip_msg_operator_message_req_ptr)->_data[2])
#define KIP_MSG_OPERATOR_MESSAGE_REQ_OP_MESSAGE_SET(kip_msg_operator_message_req_ptr, op_message) ((kip_msg_operator_message_req_ptr)->_data[2] = (uint16)(op_message))
#define KIP_MSG_OPERATOR_MESSAGE_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPERATOR_MESSAGE_REQ_CREATE(con_id, opid, op_message) \
    (uint16)(con_id), \
    (uint16)(opid), \
    (uint16)(op_message)
#define KIP_MSG_OPERATOR_MESSAGE_REQ_PACK(kip_msg_operator_message_req_ptr, con_id, opid, op_message) \
    do { \
        (kip_msg_operator_message_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_operator_message_req_ptr)->_data[1] = (uint16)((uint16)(opid)); \
        (kip_msg_operator_message_req_ptr)->_data[2] = (uint16)((uint16)(op_message)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPERATOR_MESSAGE_RES

  DESCRIPTION

  MEMBERS
    status              - status code
    con_id              - connection id with sender/receiver processor and SW
                          client IDs as per common adaptor definition
    opid_or_reason_code - Operator ID if success, error code if failed
    response            - Contains the message response if command returns
                          successfully, undefined in failure case

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_OPERATOR_MESSAGE_RES;

/* The following macros take KIP_MSG_OPERATOR_MESSAGE_RES *kip_msg_operator_message_res_ptr */
#define KIP_MSG_OPERATOR_MESSAGE_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_OPERATOR_MESSAGE_RES_STATUS_GET(kip_msg_operator_message_res_ptr) ((kip_msg_operator_message_res_ptr)->_data[0])
#define KIP_MSG_OPERATOR_MESSAGE_RES_STATUS_SET(kip_msg_operator_message_res_ptr, status) ((kip_msg_operator_message_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_OPERATOR_MESSAGE_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_OPERATOR_MESSAGE_RES_CON_ID_GET(kip_msg_operator_message_res_ptr) ((kip_msg_operator_message_res_ptr)->_data[1])
#define KIP_MSG_OPERATOR_MESSAGE_RES_CON_ID_SET(kip_msg_operator_message_res_ptr, con_id) ((kip_msg_operator_message_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_OPERATOR_MESSAGE_RES_OPID_OR_REASON_CODE_WORD_OFFSET (2)
#define KIP_MSG_OPERATOR_MESSAGE_RES_OPID_OR_REASON_CODE_GET(kip_msg_operator_message_res_ptr) ((kip_msg_operator_message_res_ptr)->_data[2])
#define KIP_MSG_OPERATOR_MESSAGE_RES_OPID_OR_REASON_CODE_SET(kip_msg_operator_message_res_ptr, opid_or_reason_code) ((kip_msg_operator_message_res_ptr)->_data[2] = (uint16)(opid_or_reason_code))
#define KIP_MSG_OPERATOR_MESSAGE_RES_RESPONSE_WORD_OFFSET (3)
#define KIP_MSG_OPERATOR_MESSAGE_RES_RESPONSE_GET(kip_msg_operator_message_res_ptr) ((kip_msg_operator_message_res_ptr)->_data[3])
#define KIP_MSG_OPERATOR_MESSAGE_RES_RESPONSE_SET(kip_msg_operator_message_res_ptr, response) ((kip_msg_operator_message_res_ptr)->_data[3] = (uint16)(response))
#define KIP_MSG_OPERATOR_MESSAGE_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPERATOR_MESSAGE_RES_CREATE(status, con_id, opid_or_reason_code, response) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(opid_or_reason_code), \
    (uint16)(response)
#define KIP_MSG_OPERATOR_MESSAGE_RES_PACK(kip_msg_operator_message_res_ptr, status, con_id, opid_or_reason_code, response) \
    do { \
        (kip_msg_operator_message_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_operator_message_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_operator_message_res_ptr)->_data[2] = (uint16)((uint16)(opid_or_reason_code)); \
        (kip_msg_operator_message_res_ptr)->_data[3] = (uint16)((uint16)(response)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPLIST_CMD_REQ

  DESCRIPTION
    Struct for messages involving lists of operators - it can be used
    wherever it makes it easier to parameterise things as the fields are
    common across such commands

  MEMBERS
    con_id  - connection id with sender/receiver processor and SW client IDs as
              per common adaptor definition
    Count   - The number of operators in the list
    op_list - The list of operators

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_OPLIST_CMD_REQ;

/* The following macros take KIP_MSG_OPLIST_CMD_REQ *kip_msg_oplist_cmd_req_ptr */
#define KIP_MSG_OPLIST_CMD_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_OPLIST_CMD_REQ_CON_ID_GET(kip_msg_oplist_cmd_req_ptr) ((kip_msg_oplist_cmd_req_ptr)->_data[0])
#define KIP_MSG_OPLIST_CMD_REQ_CON_ID_SET(kip_msg_oplist_cmd_req_ptr, con_id) ((kip_msg_oplist_cmd_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_OPLIST_CMD_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_OPLIST_CMD_REQ_COUNT_GET(kip_msg_oplist_cmd_req_ptr) ((kip_msg_oplist_cmd_req_ptr)->_data[1])
#define KIP_MSG_OPLIST_CMD_REQ_COUNT_SET(kip_msg_oplist_cmd_req_ptr, count) ((kip_msg_oplist_cmd_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_OPLIST_CMD_REQ_OP_LIST_WORD_OFFSET (2)
#define KIP_MSG_OPLIST_CMD_REQ_OP_LIST_GET(kip_msg_oplist_cmd_req_ptr) ((kip_msg_oplist_cmd_req_ptr)->_data[2])
#define KIP_MSG_OPLIST_CMD_REQ_OP_LIST_SET(kip_msg_oplist_cmd_req_ptr, op_list) ((kip_msg_oplist_cmd_req_ptr)->_data[2] = (uint16)(op_list))
#define KIP_MSG_OPLIST_CMD_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPLIST_CMD_REQ_CREATE(con_id, Count, op_list) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(op_list)
#define KIP_MSG_OPLIST_CMD_REQ_PACK(kip_msg_oplist_cmd_req_ptr, con_id, Count, op_list) \
    do { \
        (kip_msg_oplist_cmd_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_oplist_cmd_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_oplist_cmd_req_ptr)->_data[2] = (uint16)((uint16)(op_list)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_OPLIST_CMD_RES

  DESCRIPTION
    Struct for messages involving lists of operators - it can be used
    wherever it makes it easier to parameterise things as the fields are
    common across such commands

  MEMBERS
    status      - status code
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Count       - The number of operators acted on
    reason_code - Error reason code if all operators have not been acted on

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_OPLIST_CMD_RES;

/* The following macros take KIP_MSG_OPLIST_CMD_RES *kip_msg_oplist_cmd_res_ptr */
#define KIP_MSG_OPLIST_CMD_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_OPLIST_CMD_RES_STATUS_GET(kip_msg_oplist_cmd_res_ptr) ((kip_msg_oplist_cmd_res_ptr)->_data[0])
#define KIP_MSG_OPLIST_CMD_RES_STATUS_SET(kip_msg_oplist_cmd_res_ptr, status) ((kip_msg_oplist_cmd_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_OPLIST_CMD_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_OPLIST_CMD_RES_CON_ID_GET(kip_msg_oplist_cmd_res_ptr) ((kip_msg_oplist_cmd_res_ptr)->_data[1])
#define KIP_MSG_OPLIST_CMD_RES_CON_ID_SET(kip_msg_oplist_cmd_res_ptr, con_id) ((kip_msg_oplist_cmd_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_OPLIST_CMD_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_OPLIST_CMD_RES_COUNT_GET(kip_msg_oplist_cmd_res_ptr) ((kip_msg_oplist_cmd_res_ptr)->_data[2])
#define KIP_MSG_OPLIST_CMD_RES_COUNT_SET(kip_msg_oplist_cmd_res_ptr, count) ((kip_msg_oplist_cmd_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_OPLIST_CMD_RES_REASON_CODE_WORD_OFFSET (3)
#define KIP_MSG_OPLIST_CMD_RES_REASON_CODE_GET(kip_msg_oplist_cmd_res_ptr) ((kip_msg_oplist_cmd_res_ptr)->_data[3])
#define KIP_MSG_OPLIST_CMD_RES_REASON_CODE_SET(kip_msg_oplist_cmd_res_ptr, reason_code) ((kip_msg_oplist_cmd_res_ptr)->_data[3] = (uint16)(reason_code))
#define KIP_MSG_OPLIST_CMD_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_OPLIST_CMD_RES_CREATE(status, con_id, Count, reason_code) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(reason_code)
#define KIP_MSG_OPLIST_CMD_RES_PACK(kip_msg_oplist_cmd_res_ptr, status, con_id, Count, reason_code) \
    do { \
        (kip_msg_oplist_cmd_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_oplist_cmd_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_oplist_cmd_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
        (kip_msg_oplist_cmd_res_ptr)->_data[3] = (uint16)((uint16)(reason_code)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ

  DESCRIPTION

  MEMBERS
    con_id    - connection id with sender/receiver processor and SW client IDs
                as per common adaptor definition
    Count     - The number of operators in the list to test
    Test_list - The list of operators to test

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ;

/* The following macros take KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ *kip_msg_p1_run_until_preproc_operators_req_ptr */
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_CON_ID_GET(kip_msg_p1_run_until_preproc_operators_req_ptr) ((kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[0])
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_CON_ID_SET(kip_msg_p1_run_until_preproc_operators_req_ptr, con_id) ((kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_COUNT_GET(kip_msg_p1_run_until_preproc_operators_req_ptr) ((kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[1])
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_COUNT_SET(kip_msg_p1_run_until_preproc_operators_req_ptr, count) ((kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_TEST_LIST_WORD_OFFSET (2)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_TEST_LIST_GET(kip_msg_p1_run_until_preproc_operators_req_ptr) ((kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[2])
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_TEST_LIST_SET(kip_msg_p1_run_until_preproc_operators_req_ptr, test_list) ((kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[2] = (uint16)(test_list))
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_CREATE(con_id, Count, Test_list) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(Test_list)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_REQ_PACK(kip_msg_p1_run_until_preproc_operators_req_ptr, con_id, Count, Test_list) \
    do { \
        (kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_p1_run_until_preproc_operators_req_ptr)->_data[2] = (uint16)((uint16)(Test_list)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES

  DESCRIPTION

  MEMBERS
    status      - status code
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Count       - The number of operators tested
    reason_code - Error reason code if not all operators tested

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES;

/* The following macros take KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES *kip_msg_p1_run_until_preproc_operators_res_ptr */
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_STATUS_GET(kip_msg_p1_run_until_preproc_operators_res_ptr) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[0])
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_STATUS_SET(kip_msg_p1_run_until_preproc_operators_res_ptr, status) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_CON_ID_GET(kip_msg_p1_run_until_preproc_operators_res_ptr) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[1])
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_CON_ID_SET(kip_msg_p1_run_until_preproc_operators_res_ptr, con_id) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_COUNT_GET(kip_msg_p1_run_until_preproc_operators_res_ptr) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[2])
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_COUNT_SET(kip_msg_p1_run_until_preproc_operators_res_ptr, count) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_REASON_CODE_WORD_OFFSET (3)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_REASON_CODE_GET(kip_msg_p1_run_until_preproc_operators_res_ptr) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[3])
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_REASON_CODE_SET(kip_msg_p1_run_until_preproc_operators_res_ptr, reason_code) ((kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[3] = (uint16)(reason_code))
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_CREATE(status, con_id, Count, reason_code) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(reason_code)
#define KIP_MSG_P1_RUN_UNTIL_PREPROC_OPERATORS_RES_PACK(kip_msg_p1_run_until_preproc_operators_res_ptr, status, con_id, Count, reason_code) \
    do { \
        (kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
        (kip_msg_p1_run_until_preproc_operators_res_ptr)->_data[3] = (uint16)((uint16)(reason_code)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_DELETE_REQ

  DESCRIPTION

  MEMBERS
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    Key_Id - The 24-bit key ID padded to 32 bits.

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_PS_DELETE_REQ;

/* The following macros take KIP_MSG_PS_DELETE_REQ *kip_msg_ps_delete_req_ptr */
#define KIP_MSG_PS_DELETE_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_PS_DELETE_REQ_CON_ID_GET(kip_msg_ps_delete_req_ptr) ((kip_msg_ps_delete_req_ptr)->_data[0])
#define KIP_MSG_PS_DELETE_REQ_CON_ID_SET(kip_msg_ps_delete_req_ptr, con_id) ((kip_msg_ps_delete_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_PS_DELETE_REQ_KEY_ID_WORD_OFFSET (1)
#define KIP_MSG_PS_DELETE_REQ_KEY_ID_GET(kip_msg_ps_delete_req_ptr)  \
    ((KIP_PS_KEY)((uint32)((kip_msg_ps_delete_req_ptr)->_data[1]) | \
                  ((uint32)((kip_msg_ps_delete_req_ptr)->_data[2]) << 16)))
#define KIP_MSG_PS_DELETE_REQ_KEY_ID_SET(kip_msg_ps_delete_req_ptr, key_id) do { \
        (kip_msg_ps_delete_req_ptr)->_data[1] = (uint16)((key_id) & 0xffff); \
        (kip_msg_ps_delete_req_ptr)->_data[2] = (uint16)((key_id) >> 16); } while (0)
#define KIP_MSG_PS_DELETE_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_DELETE_REQ_CREATE(con_id, Key_Id) \
    (uint16)(con_id), \
    (uint16)((Key_Id) & 0xffff), \
    (uint16)((Key_Id) >> 16)
#define KIP_MSG_PS_DELETE_REQ_PACK(kip_msg_ps_delete_req_ptr, con_id, Key_Id) \
    do { \
        (kip_msg_ps_delete_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_ps_delete_req_ptr)->_data[1] = (uint16)((uint16)((Key_Id) & 0xffff)); \
        (kip_msg_ps_delete_req_ptr)->_data[2] = (uint16)(((Key_Id) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_DELETE_RES

  DESCRIPTION

  MEMBERS
    status - status code
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    Key_Id - The 24-bit key ID padded to 32 bits.

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_PS_DELETE_RES;

/* The following macros take KIP_MSG_PS_DELETE_RES *kip_msg_ps_delete_res_ptr */
#define KIP_MSG_PS_DELETE_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_PS_DELETE_RES_STATUS_GET(kip_msg_ps_delete_res_ptr) ((kip_msg_ps_delete_res_ptr)->_data[0])
#define KIP_MSG_PS_DELETE_RES_STATUS_SET(kip_msg_ps_delete_res_ptr, status) ((kip_msg_ps_delete_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_PS_DELETE_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_PS_DELETE_RES_CON_ID_GET(kip_msg_ps_delete_res_ptr) ((kip_msg_ps_delete_res_ptr)->_data[1])
#define KIP_MSG_PS_DELETE_RES_CON_ID_SET(kip_msg_ps_delete_res_ptr, con_id) ((kip_msg_ps_delete_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_PS_DELETE_RES_KEY_ID_WORD_OFFSET (2)
#define KIP_MSG_PS_DELETE_RES_KEY_ID_GET(kip_msg_ps_delete_res_ptr)  \
    ((KIP_PS_KEY)((uint32)((kip_msg_ps_delete_res_ptr)->_data[2]) | \
                  ((uint32)((kip_msg_ps_delete_res_ptr)->_data[3]) << 16)))
#define KIP_MSG_PS_DELETE_RES_KEY_ID_SET(kip_msg_ps_delete_res_ptr, key_id) do { \
        (kip_msg_ps_delete_res_ptr)->_data[2] = (uint16)((key_id) & 0xffff); \
        (kip_msg_ps_delete_res_ptr)->_data[3] = (uint16)((key_id) >> 16); } while (0)
#define KIP_MSG_PS_DELETE_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_DELETE_RES_CREATE(status, con_id, Key_Id) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)((Key_Id) & 0xffff), \
    (uint16)((Key_Id) >> 16)
#define KIP_MSG_PS_DELETE_RES_PACK(kip_msg_ps_delete_res_ptr, status, con_id, Key_Id) \
    do { \
        (kip_msg_ps_delete_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_ps_delete_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_ps_delete_res_ptr)->_data[2] = (uint16)((uint16)((Key_Id) & 0xffff)); \
        (kip_msg_ps_delete_res_ptr)->_data[3] = (uint16)(((Key_Id) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_READ_REQ

  DESCRIPTION

  MEMBERS
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    Key_Id - The 24-bit key ID padded to 32 bits.
    offset - Position of the data in this message within the underlying storage.

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_PS_READ_REQ;

/* The following macros take KIP_MSG_PS_READ_REQ *kip_msg_ps_read_req_ptr */
#define KIP_MSG_PS_READ_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_PS_READ_REQ_CON_ID_GET(kip_msg_ps_read_req_ptr) ((kip_msg_ps_read_req_ptr)->_data[0])
#define KIP_MSG_PS_READ_REQ_CON_ID_SET(kip_msg_ps_read_req_ptr, con_id) ((kip_msg_ps_read_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_PS_READ_REQ_KEY_ID_WORD_OFFSET (1)
#define KIP_MSG_PS_READ_REQ_KEY_ID_GET(kip_msg_ps_read_req_ptr)  \
    ((KIP_PS_KEY)((uint32)((kip_msg_ps_read_req_ptr)->_data[1]) | \
                  ((uint32)((kip_msg_ps_read_req_ptr)->_data[2]) << 16)))
#define KIP_MSG_PS_READ_REQ_KEY_ID_SET(kip_msg_ps_read_req_ptr, key_id) do { \
        (kip_msg_ps_read_req_ptr)->_data[1] = (uint16)((key_id) & 0xffff); \
        (kip_msg_ps_read_req_ptr)->_data[2] = (uint16)((key_id) >> 16); } while (0)
#define KIP_MSG_PS_READ_REQ_OFFSET_WORD_OFFSET (3)
#define KIP_MSG_PS_READ_REQ_OFFSET_GET(kip_msg_ps_read_req_ptr) ((kip_msg_ps_read_req_ptr)->_data[3])
#define KIP_MSG_PS_READ_REQ_OFFSET_SET(kip_msg_ps_read_req_ptr, offset) ((kip_msg_ps_read_req_ptr)->_data[3] = (uint16)(offset))
#define KIP_MSG_PS_READ_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_READ_REQ_CREATE(con_id, Key_Id, offset) \
    (uint16)(con_id), \
    (uint16)((Key_Id) & 0xffff), \
    (uint16)((Key_Id) >> 16), \
    (uint16)(offset)
#define KIP_MSG_PS_READ_REQ_PACK(kip_msg_ps_read_req_ptr, con_id, Key_Id, offset) \
    do { \
        (kip_msg_ps_read_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_ps_read_req_ptr)->_data[1] = (uint16)((uint16)((Key_Id) & 0xffff)); \
        (kip_msg_ps_read_req_ptr)->_data[2] = (uint16)(((Key_Id) >> 16)); \
        (kip_msg_ps_read_req_ptr)->_data[3] = (uint16)((uint16)(offset)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_READ_RES

  DESCRIPTION

  MEMBERS
    status    - status code
    con_id    - connection id with sender/receiver processor and SW client IDs
                as per common adaptor definition
    Key_Id    - The 24-bit key ID padded to 32 bits.
    Total_Len - Length of the value in storage.
    data      - the actual data; length implicit in KIP payload length

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} KIP_MSG_PS_READ_RES;

/* The following macros take KIP_MSG_PS_READ_RES *kip_msg_ps_read_res_ptr */
#define KIP_MSG_PS_READ_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_PS_READ_RES_STATUS_GET(kip_msg_ps_read_res_ptr) ((kip_msg_ps_read_res_ptr)->_data[0])
#define KIP_MSG_PS_READ_RES_STATUS_SET(kip_msg_ps_read_res_ptr, status) ((kip_msg_ps_read_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_PS_READ_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_PS_READ_RES_CON_ID_GET(kip_msg_ps_read_res_ptr) ((kip_msg_ps_read_res_ptr)->_data[1])
#define KIP_MSG_PS_READ_RES_CON_ID_SET(kip_msg_ps_read_res_ptr, con_id) ((kip_msg_ps_read_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_PS_READ_RES_KEY_ID_WORD_OFFSET (2)
#define KIP_MSG_PS_READ_RES_KEY_ID_GET(kip_msg_ps_read_res_ptr)  \
    ((KIP_PS_KEY)((uint32)((kip_msg_ps_read_res_ptr)->_data[2]) | \
                  ((uint32)((kip_msg_ps_read_res_ptr)->_data[3]) << 16)))
#define KIP_MSG_PS_READ_RES_KEY_ID_SET(kip_msg_ps_read_res_ptr, key_id) do { \
        (kip_msg_ps_read_res_ptr)->_data[2] = (uint16)((key_id) & 0xffff); \
        (kip_msg_ps_read_res_ptr)->_data[3] = (uint16)((key_id) >> 16); } while (0)
#define KIP_MSG_PS_READ_RES_TOTAL_LEN_WORD_OFFSET (4)
#define KIP_MSG_PS_READ_RES_TOTAL_LEN_GET(kip_msg_ps_read_res_ptr) ((kip_msg_ps_read_res_ptr)->_data[4])
#define KIP_MSG_PS_READ_RES_TOTAL_LEN_SET(kip_msg_ps_read_res_ptr, total_len) ((kip_msg_ps_read_res_ptr)->_data[4] = (uint16)(total_len))
#define KIP_MSG_PS_READ_RES_DATA_WORD_OFFSET (5)
#define KIP_MSG_PS_READ_RES_DATA_GET(kip_msg_ps_read_res_ptr) ((kip_msg_ps_read_res_ptr)->_data[5])
#define KIP_MSG_PS_READ_RES_DATA_SET(kip_msg_ps_read_res_ptr, data) ((kip_msg_ps_read_res_ptr)->_data[5] = (uint16)(data))
#define KIP_MSG_PS_READ_RES_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_READ_RES_CREATE(status, con_id, Key_Id, Total_Len, data) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)((Key_Id) & 0xffff), \
    (uint16)((Key_Id) >> 16), \
    (uint16)(Total_Len), \
    (uint16)(data)
#define KIP_MSG_PS_READ_RES_PACK(kip_msg_ps_read_res_ptr, status, con_id, Key_Id, Total_Len, data) \
    do { \
        (kip_msg_ps_read_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_ps_read_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_ps_read_res_ptr)->_data[2] = (uint16)((uint16)((Key_Id) & 0xffff)); \
        (kip_msg_ps_read_res_ptr)->_data[3] = (uint16)(((Key_Id) >> 16)); \
        (kip_msg_ps_read_res_ptr)->_data[4] = (uint16)((uint16)(Total_Len)); \
        (kip_msg_ps_read_res_ptr)->_data[5] = (uint16)((uint16)(data)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ

  DESCRIPTION

  MEMBERS
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ;

/* The following macros take KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ *kip_msg_ps_shutdown_complete_req_ptr */
#define KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ_CON_ID_GET(kip_msg_ps_shutdown_complete_req_ptr) ((kip_msg_ps_shutdown_complete_req_ptr)->_data[0])
#define KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ_CON_ID_SET(kip_msg_ps_shutdown_complete_req_ptr, con_id) ((kip_msg_ps_shutdown_complete_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ_CREATE(con_id) \
    (uint16)(con_id)
#define KIP_MSG_PS_SHUTDOWN_COMPLETE_REQ_PACK(kip_msg_ps_shutdown_complete_req_ptr, con_id) \
    do { \
        (kip_msg_ps_shutdown_complete_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_SHUTDOWN_REQ

  DESCRIPTION

  MEMBERS
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} KIP_MSG_PS_SHUTDOWN_REQ;

/* The following macros take KIP_MSG_PS_SHUTDOWN_REQ *kip_msg_ps_shutdown_req_ptr */
#define KIP_MSG_PS_SHUTDOWN_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_PS_SHUTDOWN_REQ_CON_ID_GET(kip_msg_ps_shutdown_req_ptr) ((kip_msg_ps_shutdown_req_ptr)->_data[0])
#define KIP_MSG_PS_SHUTDOWN_REQ_CON_ID_SET(kip_msg_ps_shutdown_req_ptr, con_id) ((kip_msg_ps_shutdown_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_PS_SHUTDOWN_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_SHUTDOWN_REQ_CREATE(con_id) \
    (uint16)(con_id)
#define KIP_MSG_PS_SHUTDOWN_REQ_PACK(kip_msg_ps_shutdown_req_ptr, con_id) \
    do { \
        (kip_msg_ps_shutdown_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_WRITE_REQ

  DESCRIPTION

  MEMBERS
    con_id    - connection id with sender/receiver processor and SW client IDs
                as per common adaptor definition
    Key_Id    - The 24-bit key ID padded to 32 bits.
    Total_Len - Length of the value in storage.
    offset    - Position of the data in this message within the underlying
                storage.
    data      - the actual data; length implicit in KIP payload length

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} KIP_MSG_PS_WRITE_REQ;

/* The following macros take KIP_MSG_PS_WRITE_REQ *kip_msg_ps_write_req_ptr */
#define KIP_MSG_PS_WRITE_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_PS_WRITE_REQ_CON_ID_GET(kip_msg_ps_write_req_ptr) ((kip_msg_ps_write_req_ptr)->_data[0])
#define KIP_MSG_PS_WRITE_REQ_CON_ID_SET(kip_msg_ps_write_req_ptr, con_id) ((kip_msg_ps_write_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_PS_WRITE_REQ_KEY_ID_WORD_OFFSET (1)
#define KIP_MSG_PS_WRITE_REQ_KEY_ID_GET(kip_msg_ps_write_req_ptr)  \
    ((KIP_PS_KEY)((uint32)((kip_msg_ps_write_req_ptr)->_data[1]) | \
                  ((uint32)((kip_msg_ps_write_req_ptr)->_data[2]) << 16)))
#define KIP_MSG_PS_WRITE_REQ_KEY_ID_SET(kip_msg_ps_write_req_ptr, key_id) do { \
        (kip_msg_ps_write_req_ptr)->_data[1] = (uint16)((key_id) & 0xffff); \
        (kip_msg_ps_write_req_ptr)->_data[2] = (uint16)((key_id) >> 16); } while (0)
#define KIP_MSG_PS_WRITE_REQ_TOTAL_LEN_WORD_OFFSET (3)
#define KIP_MSG_PS_WRITE_REQ_TOTAL_LEN_GET(kip_msg_ps_write_req_ptr) ((kip_msg_ps_write_req_ptr)->_data[3])
#define KIP_MSG_PS_WRITE_REQ_TOTAL_LEN_SET(kip_msg_ps_write_req_ptr, total_len) ((kip_msg_ps_write_req_ptr)->_data[3] = (uint16)(total_len))
#define KIP_MSG_PS_WRITE_REQ_OFFSET_WORD_OFFSET (4)
#define KIP_MSG_PS_WRITE_REQ_OFFSET_GET(kip_msg_ps_write_req_ptr) ((kip_msg_ps_write_req_ptr)->_data[4])
#define KIP_MSG_PS_WRITE_REQ_OFFSET_SET(kip_msg_ps_write_req_ptr, offset) ((kip_msg_ps_write_req_ptr)->_data[4] = (uint16)(offset))
#define KIP_MSG_PS_WRITE_REQ_DATA_WORD_OFFSET (5)
#define KIP_MSG_PS_WRITE_REQ_DATA_GET(kip_msg_ps_write_req_ptr) ((kip_msg_ps_write_req_ptr)->_data[5])
#define KIP_MSG_PS_WRITE_REQ_DATA_SET(kip_msg_ps_write_req_ptr, data) ((kip_msg_ps_write_req_ptr)->_data[5] = (uint16)(data))
#define KIP_MSG_PS_WRITE_REQ_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_WRITE_REQ_CREATE(con_id, Key_Id, Total_Len, offset, data) \
    (uint16)(con_id), \
    (uint16)((Key_Id) & 0xffff), \
    (uint16)((Key_Id) >> 16), \
    (uint16)(Total_Len), \
    (uint16)(offset), \
    (uint16)(data)
#define KIP_MSG_PS_WRITE_REQ_PACK(kip_msg_ps_write_req_ptr, con_id, Key_Id, Total_Len, offset, data) \
    do { \
        (kip_msg_ps_write_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_ps_write_req_ptr)->_data[1] = (uint16)((uint16)((Key_Id) & 0xffff)); \
        (kip_msg_ps_write_req_ptr)->_data[2] = (uint16)(((Key_Id) >> 16)); \
        (kip_msg_ps_write_req_ptr)->_data[3] = (uint16)((uint16)(Total_Len)); \
        (kip_msg_ps_write_req_ptr)->_data[4] = (uint16)((uint16)(offset)); \
        (kip_msg_ps_write_req_ptr)->_data[5] = (uint16)((uint16)(data)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PS_WRITE_RES

  DESCRIPTION

  MEMBERS
    status - status code
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    Key_Id - The 24-bit key ID padded to 32 bits.
    Rank   - Level of persistence (16 bits)

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} KIP_MSG_PS_WRITE_RES;

/* The following macros take KIP_MSG_PS_WRITE_RES *kip_msg_ps_write_res_ptr */
#define KIP_MSG_PS_WRITE_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_PS_WRITE_RES_STATUS_GET(kip_msg_ps_write_res_ptr) ((kip_msg_ps_write_res_ptr)->_data[0])
#define KIP_MSG_PS_WRITE_RES_STATUS_SET(kip_msg_ps_write_res_ptr, status) ((kip_msg_ps_write_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_PS_WRITE_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_PS_WRITE_RES_CON_ID_GET(kip_msg_ps_write_res_ptr) ((kip_msg_ps_write_res_ptr)->_data[1])
#define KIP_MSG_PS_WRITE_RES_CON_ID_SET(kip_msg_ps_write_res_ptr, con_id) ((kip_msg_ps_write_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_PS_WRITE_RES_KEY_ID_WORD_OFFSET (2)
#define KIP_MSG_PS_WRITE_RES_KEY_ID_GET(kip_msg_ps_write_res_ptr)  \
    ((KIP_PS_KEY)((uint32)((kip_msg_ps_write_res_ptr)->_data[2]) | \
                  ((uint32)((kip_msg_ps_write_res_ptr)->_data[3]) << 16)))
#define KIP_MSG_PS_WRITE_RES_KEY_ID_SET(kip_msg_ps_write_res_ptr, key_id) do { \
        (kip_msg_ps_write_res_ptr)->_data[2] = (uint16)((key_id) & 0xffff); \
        (kip_msg_ps_write_res_ptr)->_data[3] = (uint16)((key_id) >> 16); } while (0)
#define KIP_MSG_PS_WRITE_RES_RANK_WORD_OFFSET (4)
#define KIP_MSG_PS_WRITE_RES_RANK_GET(kip_msg_ps_write_res_ptr) ((PERSIST)(kip_msg_ps_write_res_ptr)->_data[4])
#define KIP_MSG_PS_WRITE_RES_RANK_SET(kip_msg_ps_write_res_ptr, rank) ((kip_msg_ps_write_res_ptr)->_data[4] = (uint16)(rank))
#define KIP_MSG_PS_WRITE_RES_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PS_WRITE_RES_CREATE(status, con_id, Key_Id, Rank) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)((Key_Id) & 0xffff), \
    (uint16)((Key_Id) >> 16), \
    (uint16)(Rank)
#define KIP_MSG_PS_WRITE_RES_PACK(kip_msg_ps_write_res_ptr, status, con_id, Key_Id, Rank) \
    do { \
        (kip_msg_ps_write_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_ps_write_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_ps_write_res_ptr)->_data[2] = (uint16)((uint16)((Key_Id) & 0xffff)); \
        (kip_msg_ps_write_res_ptr)->_data[3] = (uint16)(((Key_Id) >> 16)); \
        (kip_msg_ps_write_res_ptr)->_data[4] = (uint16)((uint16)(Rank)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PUBLISH_FAULT_REQ

  DESCRIPTION

  MEMBERS
    con_id      - Connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    fault_id    - Fault identifier
    processor   - Processor on which the fault occured.
    num_reports - Number of reports of the failure.
    timestamp   - Time of the first report of the fault.
    args_len    - The length of diatribe passed along with the fault.
    args        - Diatribe argument

*******************************************************************************/
typedef struct
{
    uint16 _data[8];
} KIP_MSG_PUBLISH_FAULT_REQ;

/* The following macros take KIP_MSG_PUBLISH_FAULT_REQ *kip_msg_publish_fault_req_ptr */
#define KIP_MSG_PUBLISH_FAULT_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_PUBLISH_FAULT_REQ_CON_ID_GET(kip_msg_publish_fault_req_ptr) ((kip_msg_publish_fault_req_ptr)->_data[0])
#define KIP_MSG_PUBLISH_FAULT_REQ_CON_ID_SET(kip_msg_publish_fault_req_ptr, con_id) ((kip_msg_publish_fault_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_PUBLISH_FAULT_REQ_FAULT_ID_WORD_OFFSET (1)
#define KIP_MSG_PUBLISH_FAULT_REQ_FAULT_ID_GET(kip_msg_publish_fault_req_ptr) ((kip_msg_publish_fault_req_ptr)->_data[1])
#define KIP_MSG_PUBLISH_FAULT_REQ_FAULT_ID_SET(kip_msg_publish_fault_req_ptr, fault_id) ((kip_msg_publish_fault_req_ptr)->_data[1] = (uint16)(fault_id))
#define KIP_MSG_PUBLISH_FAULT_REQ_PROCESSOR_WORD_OFFSET (2)
#define KIP_MSG_PUBLISH_FAULT_REQ_PROCESSOR_GET(kip_msg_publish_fault_req_ptr) ((kip_msg_publish_fault_req_ptr)->_data[2])
#define KIP_MSG_PUBLISH_FAULT_REQ_PROCESSOR_SET(kip_msg_publish_fault_req_ptr, processor) ((kip_msg_publish_fault_req_ptr)->_data[2] = (uint16)(processor))
#define KIP_MSG_PUBLISH_FAULT_REQ_NUM_REPORTS_WORD_OFFSET (3)
#define KIP_MSG_PUBLISH_FAULT_REQ_NUM_REPORTS_GET(kip_msg_publish_fault_req_ptr) ((kip_msg_publish_fault_req_ptr)->_data[3])
#define KIP_MSG_PUBLISH_FAULT_REQ_NUM_REPORTS_SET(kip_msg_publish_fault_req_ptr, num_reports) ((kip_msg_publish_fault_req_ptr)->_data[3] = (uint16)(num_reports))
#define KIP_MSG_PUBLISH_FAULT_REQ_TIMESTAMP_WORD_OFFSET (4)
#define KIP_MSG_PUBLISH_FAULT_REQ_TIMESTAMP_GET(kip_msg_publish_fault_req_ptr)  \
    (((uint32)((kip_msg_publish_fault_req_ptr)->_data[4]) | \
      ((uint32)((kip_msg_publish_fault_req_ptr)->_data[5]) << 16)))
#define KIP_MSG_PUBLISH_FAULT_REQ_TIMESTAMP_SET(kip_msg_publish_fault_req_ptr, timestamp) do { \
        (kip_msg_publish_fault_req_ptr)->_data[4] = (uint16)((timestamp) & 0xffff); \
        (kip_msg_publish_fault_req_ptr)->_data[5] = (uint16)((timestamp) >> 16); } while (0)
#define KIP_MSG_PUBLISH_FAULT_REQ_ARGS_LEN_WORD_OFFSET (6)
#define KIP_MSG_PUBLISH_FAULT_REQ_ARGS_LEN_GET(kip_msg_publish_fault_req_ptr) ((kip_msg_publish_fault_req_ptr)->_data[6])
#define KIP_MSG_PUBLISH_FAULT_REQ_ARGS_LEN_SET(kip_msg_publish_fault_req_ptr, args_len) ((kip_msg_publish_fault_req_ptr)->_data[6] = (uint16)(args_len))
#define KIP_MSG_PUBLISH_FAULT_REQ_ARGS_WORD_OFFSET (7)
#define KIP_MSG_PUBLISH_FAULT_REQ_ARGS_GET(kip_msg_publish_fault_req_ptr) ((kip_msg_publish_fault_req_ptr)->_data[7])
#define KIP_MSG_PUBLISH_FAULT_REQ_ARGS_SET(kip_msg_publish_fault_req_ptr, args) ((kip_msg_publish_fault_req_ptr)->_data[7] = (uint16)(args))
#define KIP_MSG_PUBLISH_FAULT_REQ_WORD_SIZE (8)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PUBLISH_FAULT_REQ_CREATE(con_id, fault_id, processor, num_reports, timestamp, args_len, args) \
    (uint16)(con_id), \
    (uint16)(fault_id), \
    (uint16)(processor), \
    (uint16)(num_reports), \
    (uint16)((timestamp) & 0xffff), \
    (uint16)((timestamp) >> 16), \
    (uint16)(args_len), \
    (uint16)(args)
#define KIP_MSG_PUBLISH_FAULT_REQ_PACK(kip_msg_publish_fault_req_ptr, con_id, fault_id, processor, num_reports, timestamp, args_len, args) \
    do { \
        (kip_msg_publish_fault_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_publish_fault_req_ptr)->_data[1] = (uint16)((uint16)(fault_id)); \
        (kip_msg_publish_fault_req_ptr)->_data[2] = (uint16)((uint16)(processor)); \
        (kip_msg_publish_fault_req_ptr)->_data[3] = (uint16)((uint16)(num_reports)); \
        (kip_msg_publish_fault_req_ptr)->_data[4] = (uint16)((uint16)((timestamp) & 0xffff)); \
        (kip_msg_publish_fault_req_ptr)->_data[5] = (uint16)(((timestamp) >> 16)); \
        (kip_msg_publish_fault_req_ptr)->_data[6] = (uint16)((uint16)(args_len)); \
        (kip_msg_publish_fault_req_ptr)->_data[7] = (uint16)((uint16)(args)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_PUBLISH_FAULT_RES

  DESCRIPTION

  MEMBERS
    con_id - Connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} KIP_MSG_PUBLISH_FAULT_RES;

/* The following macros take KIP_MSG_PUBLISH_FAULT_RES *kip_msg_publish_fault_res_ptr */
#define KIP_MSG_PUBLISH_FAULT_RES_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_PUBLISH_FAULT_RES_CON_ID_GET(kip_msg_publish_fault_res_ptr) ((kip_msg_publish_fault_res_ptr)->_data[0])
#define KIP_MSG_PUBLISH_FAULT_RES_CON_ID_SET(kip_msg_publish_fault_res_ptr, con_id) ((kip_msg_publish_fault_res_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_PUBLISH_FAULT_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_PUBLISH_FAULT_RES_CREATE(con_id) \
    (uint16)(con_id)
#define KIP_MSG_PUBLISH_FAULT_RES_PACK(kip_msg_publish_fault_res_ptr, con_id) \
    do { \
        (kip_msg_publish_fault_res_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_REQ_STRUC

  DESCRIPTION
    KIP REQ message payload definition - this data comes after common IPC
    header info.

  MEMBERS
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    data   - Message data

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} KIP_MSG_REQ_STRUC;

/* The following macros take KIP_MSG_REQ_STRUC *kip_msg_req_struc_ptr */
#define KIP_MSG_REQ_STRUC_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_REQ_STRUC_CON_ID_GET(kip_msg_req_struc_ptr) ((kip_msg_req_struc_ptr)->_data[0])
#define KIP_MSG_REQ_STRUC_CON_ID_SET(kip_msg_req_struc_ptr, con_id) ((kip_msg_req_struc_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_REQ_STRUC_DATA_WORD_OFFSET (1)
#define KIP_MSG_REQ_STRUC_DATA_GET(kip_msg_req_struc_ptr) ((kip_msg_req_struc_ptr)->_data[1])
#define KIP_MSG_REQ_STRUC_DATA_SET(kip_msg_req_struc_ptr, data) ((kip_msg_req_struc_ptr)->_data[1] = (uint16)(data))
#define KIP_MSG_REQ_STRUC_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_REQ_STRUC_CREATE(con_id, data) \
    (uint16)(con_id), \
    (uint16)(data)
#define KIP_MSG_REQ_STRUC_PACK(kip_msg_req_struc_ptr, con_id, data) \
    do { \
        (kip_msg_req_struc_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_req_struc_ptr)->_data[1] = (uint16)((uint16)(data)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_RESET_OPERATORS_REQ

  DESCRIPTION

  MEMBERS
    con_id     - connection id with sender/receiver processor and SW client IDs
                 as per common adaptor definition
    Count      - The number of operators in the list to reset
    reset_list - The list of operators to reset

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_RESET_OPERATORS_REQ;

/* The following macros take KIP_MSG_RESET_OPERATORS_REQ *kip_msg_reset_operators_req_ptr */
#define KIP_MSG_RESET_OPERATORS_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_RESET_OPERATORS_REQ_CON_ID_GET(kip_msg_reset_operators_req_ptr) ((kip_msg_reset_operators_req_ptr)->_data[0])
#define KIP_MSG_RESET_OPERATORS_REQ_CON_ID_SET(kip_msg_reset_operators_req_ptr, con_id) ((kip_msg_reset_operators_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_RESET_OPERATORS_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_RESET_OPERATORS_REQ_COUNT_GET(kip_msg_reset_operators_req_ptr) ((kip_msg_reset_operators_req_ptr)->_data[1])
#define KIP_MSG_RESET_OPERATORS_REQ_COUNT_SET(kip_msg_reset_operators_req_ptr, count) ((kip_msg_reset_operators_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_RESET_OPERATORS_REQ_RESET_LIST_WORD_OFFSET (2)
#define KIP_MSG_RESET_OPERATORS_REQ_RESET_LIST_GET(kip_msg_reset_operators_req_ptr) ((kip_msg_reset_operators_req_ptr)->_data[2])
#define KIP_MSG_RESET_OPERATORS_REQ_RESET_LIST_SET(kip_msg_reset_operators_req_ptr, reset_list) ((kip_msg_reset_operators_req_ptr)->_data[2] = (uint16)(reset_list))
#define KIP_MSG_RESET_OPERATORS_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_RESET_OPERATORS_REQ_CREATE(con_id, Count, reset_list) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(reset_list)
#define KIP_MSG_RESET_OPERATORS_REQ_PACK(kip_msg_reset_operators_req_ptr, con_id, Count, reset_list) \
    do { \
        (kip_msg_reset_operators_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_reset_operators_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_reset_operators_req_ptr)->_data[2] = (uint16)((uint16)(reset_list)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_RESET_OPERATORS_RES

  DESCRIPTION

  MEMBERS
    status      - status code
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Count       - The number of operators reset
    reason_code - Error reason code if all operators have not been reset

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_RESET_OPERATORS_RES;

/* The following macros take KIP_MSG_RESET_OPERATORS_RES *kip_msg_reset_operators_res_ptr */
#define KIP_MSG_RESET_OPERATORS_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_RESET_OPERATORS_RES_STATUS_GET(kip_msg_reset_operators_res_ptr) ((kip_msg_reset_operators_res_ptr)->_data[0])
#define KIP_MSG_RESET_OPERATORS_RES_STATUS_SET(kip_msg_reset_operators_res_ptr, status) ((kip_msg_reset_operators_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_RESET_OPERATORS_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_RESET_OPERATORS_RES_CON_ID_GET(kip_msg_reset_operators_res_ptr) ((kip_msg_reset_operators_res_ptr)->_data[1])
#define KIP_MSG_RESET_OPERATORS_RES_CON_ID_SET(kip_msg_reset_operators_res_ptr, con_id) ((kip_msg_reset_operators_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_RESET_OPERATORS_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_RESET_OPERATORS_RES_COUNT_GET(kip_msg_reset_operators_res_ptr) ((kip_msg_reset_operators_res_ptr)->_data[2])
#define KIP_MSG_RESET_OPERATORS_RES_COUNT_SET(kip_msg_reset_operators_res_ptr, count) ((kip_msg_reset_operators_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_RESET_OPERATORS_RES_REASON_CODE_WORD_OFFSET (3)
#define KIP_MSG_RESET_OPERATORS_RES_REASON_CODE_GET(kip_msg_reset_operators_res_ptr) ((kip_msg_reset_operators_res_ptr)->_data[3])
#define KIP_MSG_RESET_OPERATORS_RES_REASON_CODE_SET(kip_msg_reset_operators_res_ptr, reason_code) ((kip_msg_reset_operators_res_ptr)->_data[3] = (uint16)(reason_code))
#define KIP_MSG_RESET_OPERATORS_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_RESET_OPERATORS_RES_CREATE(status, con_id, Count, reason_code) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(reason_code)
#define KIP_MSG_RESET_OPERATORS_RES_PACK(kip_msg_reset_operators_res_ptr, status, con_id, Count, reason_code) \
    do { \
        (kip_msg_reset_operators_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_reset_operators_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_reset_operators_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
        (kip_msg_reset_operators_res_ptr)->_data[3] = (uint16)((uint16)(reason_code)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_RES_STRUC

  DESCRIPTION
    KIP RESP message payload definition - this data comes after common IPC
    header info.

  MEMBERS
    status - status code
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    data   - Message data

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_RES_STRUC;

/* The following macros take KIP_MSG_RES_STRUC *kip_msg_res_struc_ptr */
#define KIP_MSG_RES_STRUC_STATUS_WORD_OFFSET (0)
#define KIP_MSG_RES_STRUC_STATUS_GET(kip_msg_res_struc_ptr) ((kip_msg_res_struc_ptr)->_data[0])
#define KIP_MSG_RES_STRUC_STATUS_SET(kip_msg_res_struc_ptr, status) ((kip_msg_res_struc_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_RES_STRUC_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_RES_STRUC_CON_ID_GET(kip_msg_res_struc_ptr) ((kip_msg_res_struc_ptr)->_data[1])
#define KIP_MSG_RES_STRUC_CON_ID_SET(kip_msg_res_struc_ptr, con_id) ((kip_msg_res_struc_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_RES_STRUC_DATA_WORD_OFFSET (2)
#define KIP_MSG_RES_STRUC_DATA_GET(kip_msg_res_struc_ptr) ((kip_msg_res_struc_ptr)->_data[2])
#define KIP_MSG_RES_STRUC_DATA_SET(kip_msg_res_struc_ptr, data) ((kip_msg_res_struc_ptr)->_data[2] = (uint16)(data))
#define KIP_MSG_RES_STRUC_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_RES_STRUC_CREATE(status, con_id, data) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(data)
#define KIP_MSG_RES_STRUC_PACK(kip_msg_res_struc_ptr, status, con_id, data) \
    do { \
        (kip_msg_res_struc_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_res_struc_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_res_struc_ptr)->_data[2] = (uint16)((uint16)(data)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_SET_SYSTEM_KEY_REQ

  DESCRIPTION

  MEMBERS
    con_id          - Connection id with sender/receiver processor and SW client
                      IDs as per common adaptor definition
    num_pairs       - The number of key-value pairs in the message.
    key_value_pairs - An array of key-value objects

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_SET_SYSTEM_KEY_REQ;

/* The following macros take KIP_MSG_SET_SYSTEM_KEY_REQ *kip_msg_set_system_key_req_ptr */
#define KIP_MSG_SET_SYSTEM_KEY_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_SET_SYSTEM_KEY_REQ_CON_ID_GET(kip_msg_set_system_key_req_ptr) ((kip_msg_set_system_key_req_ptr)->_data[0])
#define KIP_MSG_SET_SYSTEM_KEY_REQ_CON_ID_SET(kip_msg_set_system_key_req_ptr, con_id) ((kip_msg_set_system_key_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_SET_SYSTEM_KEY_REQ_NUM_PAIRS_WORD_OFFSET (1)
#define KIP_MSG_SET_SYSTEM_KEY_REQ_NUM_PAIRS_GET(kip_msg_set_system_key_req_ptr) ((kip_msg_set_system_key_req_ptr)->_data[1])
#define KIP_MSG_SET_SYSTEM_KEY_REQ_NUM_PAIRS_SET(kip_msg_set_system_key_req_ptr, num_pairs) ((kip_msg_set_system_key_req_ptr)->_data[1] = (uint16)(num_pairs))
#define KIP_MSG_SET_SYSTEM_KEY_REQ_KEY_VALUE_PAIRS_WORD_OFFSET (2)
#define KIP_MSG_SET_SYSTEM_KEY_REQ_KEY_VALUE_PAIRS_GET(kip_msg_set_system_key_req_ptr)  \
    (((uint32)((kip_msg_set_system_key_req_ptr)->_data[2]) | \
      ((uint32)((kip_msg_set_system_key_req_ptr)->_data[3]) << 16)))
#define KIP_MSG_SET_SYSTEM_KEY_REQ_KEY_VALUE_PAIRS_SET(kip_msg_set_system_key_req_ptr, key_value_pairs) do { \
        (kip_msg_set_system_key_req_ptr)->_data[2] = (uint16)((key_value_pairs) & 0xffff); \
        (kip_msg_set_system_key_req_ptr)->_data[3] = (uint16)((key_value_pairs) >> 16); } while (0)
#define KIP_MSG_SET_SYSTEM_KEY_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_SET_SYSTEM_KEY_REQ_CREATE(con_id, num_pairs, key_value_pairs) \
    (uint16)(con_id), \
    (uint16)(num_pairs), \
    (uint16)((key_value_pairs) & 0xffff), \
    (uint16)((key_value_pairs) >> 16)
#define KIP_MSG_SET_SYSTEM_KEY_REQ_PACK(kip_msg_set_system_key_req_ptr, con_id, num_pairs, key_value_pairs) \
    do { \
        (kip_msg_set_system_key_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_set_system_key_req_ptr)->_data[1] = (uint16)((uint16)(num_pairs)); \
        (kip_msg_set_system_key_req_ptr)->_data[2] = (uint16)((uint16)((key_value_pairs) & 0xffff)); \
        (kip_msg_set_system_key_req_ptr)->_data[3] = (uint16)(((key_value_pairs) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_SET_SYSTEM_KEY_RES

  DESCRIPTION

  MEMBERS
    con_id - Connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} KIP_MSG_SET_SYSTEM_KEY_RES;

/* The following macros take KIP_MSG_SET_SYSTEM_KEY_RES *kip_msg_set_system_key_res_ptr */
#define KIP_MSG_SET_SYSTEM_KEY_RES_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_SET_SYSTEM_KEY_RES_CON_ID_GET(kip_msg_set_system_key_res_ptr) ((kip_msg_set_system_key_res_ptr)->_data[0])
#define KIP_MSG_SET_SYSTEM_KEY_RES_CON_ID_SET(kip_msg_set_system_key_res_ptr, con_id) ((kip_msg_set_system_key_res_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_SET_SYSTEM_KEY_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_SET_SYSTEM_KEY_RES_CREATE(con_id) \
    (uint16)(con_id)
#define KIP_MSG_SET_SYSTEM_KEY_RES_PACK(kip_msg_set_system_key_res_ptr, con_id) \
    do { \
        (kip_msg_set_system_key_res_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_START_OPERATORS_REQ

  DESCRIPTION

  MEMBERS
    con_id     - connection id with sender/receiver processor and SW client IDs
                 as per common adaptor definition
    Count      - The number of operators in the list to start
    Start_list - The list of operators to start

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_START_OPERATORS_REQ;

/* The following macros take KIP_MSG_START_OPERATORS_REQ *kip_msg_start_operators_req_ptr */
#define KIP_MSG_START_OPERATORS_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_START_OPERATORS_REQ_CON_ID_GET(kip_msg_start_operators_req_ptr) ((kip_msg_start_operators_req_ptr)->_data[0])
#define KIP_MSG_START_OPERATORS_REQ_CON_ID_SET(kip_msg_start_operators_req_ptr, con_id) ((kip_msg_start_operators_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_START_OPERATORS_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_START_OPERATORS_REQ_COUNT_GET(kip_msg_start_operators_req_ptr) ((kip_msg_start_operators_req_ptr)->_data[1])
#define KIP_MSG_START_OPERATORS_REQ_COUNT_SET(kip_msg_start_operators_req_ptr, count) ((kip_msg_start_operators_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_START_OPERATORS_REQ_START_LIST_WORD_OFFSET (2)
#define KIP_MSG_START_OPERATORS_REQ_START_LIST_GET(kip_msg_start_operators_req_ptr) ((kip_msg_start_operators_req_ptr)->_data[2])
#define KIP_MSG_START_OPERATORS_REQ_START_LIST_SET(kip_msg_start_operators_req_ptr, start_list) ((kip_msg_start_operators_req_ptr)->_data[2] = (uint16)(start_list))
#define KIP_MSG_START_OPERATORS_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_START_OPERATORS_REQ_CREATE(con_id, Count, Start_list) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(Start_list)
#define KIP_MSG_START_OPERATORS_REQ_PACK(kip_msg_start_operators_req_ptr, con_id, Count, Start_list) \
    do { \
        (kip_msg_start_operators_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_start_operators_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_start_operators_req_ptr)->_data[2] = (uint16)((uint16)(Start_list)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_START_OPERATORS_RES

  DESCRIPTION

  MEMBERS
    status      - status code
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Count       - The number of operators started
    reason_code - Error reason code if not all operators were started

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_START_OPERATORS_RES;

/* The following macros take KIP_MSG_START_OPERATORS_RES *kip_msg_start_operators_res_ptr */
#define KIP_MSG_START_OPERATORS_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_START_OPERATORS_RES_STATUS_GET(kip_msg_start_operators_res_ptr) ((kip_msg_start_operators_res_ptr)->_data[0])
#define KIP_MSG_START_OPERATORS_RES_STATUS_SET(kip_msg_start_operators_res_ptr, status) ((kip_msg_start_operators_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_START_OPERATORS_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_START_OPERATORS_RES_CON_ID_GET(kip_msg_start_operators_res_ptr) ((kip_msg_start_operators_res_ptr)->_data[1])
#define KIP_MSG_START_OPERATORS_RES_CON_ID_SET(kip_msg_start_operators_res_ptr, con_id) ((kip_msg_start_operators_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_START_OPERATORS_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_START_OPERATORS_RES_COUNT_GET(kip_msg_start_operators_res_ptr) ((kip_msg_start_operators_res_ptr)->_data[2])
#define KIP_MSG_START_OPERATORS_RES_COUNT_SET(kip_msg_start_operators_res_ptr, count) ((kip_msg_start_operators_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_START_OPERATORS_RES_REASON_CODE_WORD_OFFSET (3)
#define KIP_MSG_START_OPERATORS_RES_REASON_CODE_GET(kip_msg_start_operators_res_ptr) ((kip_msg_start_operators_res_ptr)->_data[3])
#define KIP_MSG_START_OPERATORS_RES_REASON_CODE_SET(kip_msg_start_operators_res_ptr, reason_code) ((kip_msg_start_operators_res_ptr)->_data[3] = (uint16)(reason_code))
#define KIP_MSG_START_OPERATORS_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_START_OPERATORS_RES_CREATE(status, con_id, Count, reason_code) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(reason_code)
#define KIP_MSG_START_OPERATORS_RES_PACK(kip_msg_start_operators_res_ptr, status, con_id, Count, reason_code) \
    do { \
        (kip_msg_start_operators_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_start_operators_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_start_operators_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
        (kip_msg_start_operators_res_ptr)->_data[3] = (uint16)((uint16)(reason_code)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STOP_OPERATORS_REQ

  DESCRIPTION

  MEMBERS
    con_id    - connection id with sender/receiver processor and SW client IDs
                as per common adaptor definition
    Count     - The number of operators in the list to stop
    stop_list - The list of operators to stop

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_STOP_OPERATORS_REQ;

/* The following macros take KIP_MSG_STOP_OPERATORS_REQ *kip_msg_stop_operators_req_ptr */
#define KIP_MSG_STOP_OPERATORS_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_STOP_OPERATORS_REQ_CON_ID_GET(kip_msg_stop_operators_req_ptr) ((kip_msg_stop_operators_req_ptr)->_data[0])
#define KIP_MSG_STOP_OPERATORS_REQ_CON_ID_SET(kip_msg_stop_operators_req_ptr, con_id) ((kip_msg_stop_operators_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_STOP_OPERATORS_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_STOP_OPERATORS_REQ_COUNT_GET(kip_msg_stop_operators_req_ptr) ((kip_msg_stop_operators_req_ptr)->_data[1])
#define KIP_MSG_STOP_OPERATORS_REQ_COUNT_SET(kip_msg_stop_operators_req_ptr, count) ((kip_msg_stop_operators_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_STOP_OPERATORS_REQ_STOP_LIST_WORD_OFFSET (2)
#define KIP_MSG_STOP_OPERATORS_REQ_STOP_LIST_GET(kip_msg_stop_operators_req_ptr) ((kip_msg_stop_operators_req_ptr)->_data[2])
#define KIP_MSG_STOP_OPERATORS_REQ_STOP_LIST_SET(kip_msg_stop_operators_req_ptr, stop_list) ((kip_msg_stop_operators_req_ptr)->_data[2] = (uint16)(stop_list))
#define KIP_MSG_STOP_OPERATORS_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STOP_OPERATORS_REQ_CREATE(con_id, Count, stop_list) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(stop_list)
#define KIP_MSG_STOP_OPERATORS_REQ_PACK(kip_msg_stop_operators_req_ptr, con_id, Count, stop_list) \
    do { \
        (kip_msg_stop_operators_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_stop_operators_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_stop_operators_req_ptr)->_data[2] = (uint16)((uint16)(stop_list)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STOP_OPERATORS_RES

  DESCRIPTION

  MEMBERS
    status      - status code
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Count       - The number of operators stopped
    reason_code - Error reason code if not all operators stopped

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} KIP_MSG_STOP_OPERATORS_RES;

/* The following macros take KIP_MSG_STOP_OPERATORS_RES *kip_msg_stop_operators_res_ptr */
#define KIP_MSG_STOP_OPERATORS_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_STOP_OPERATORS_RES_STATUS_GET(kip_msg_stop_operators_res_ptr) ((kip_msg_stop_operators_res_ptr)->_data[0])
#define KIP_MSG_STOP_OPERATORS_RES_STATUS_SET(kip_msg_stop_operators_res_ptr, status) ((kip_msg_stop_operators_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_STOP_OPERATORS_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_STOP_OPERATORS_RES_CON_ID_GET(kip_msg_stop_operators_res_ptr) ((kip_msg_stop_operators_res_ptr)->_data[1])
#define KIP_MSG_STOP_OPERATORS_RES_CON_ID_SET(kip_msg_stop_operators_res_ptr, con_id) ((kip_msg_stop_operators_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_STOP_OPERATORS_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_STOP_OPERATORS_RES_COUNT_GET(kip_msg_stop_operators_res_ptr) ((kip_msg_stop_operators_res_ptr)->_data[2])
#define KIP_MSG_STOP_OPERATORS_RES_COUNT_SET(kip_msg_stop_operators_res_ptr, count) ((kip_msg_stop_operators_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_STOP_OPERATORS_RES_REASON_CODE_WORD_OFFSET (3)
#define KIP_MSG_STOP_OPERATORS_RES_REASON_CODE_GET(kip_msg_stop_operators_res_ptr) ((kip_msg_stop_operators_res_ptr)->_data[3])
#define KIP_MSG_STOP_OPERATORS_RES_REASON_CODE_SET(kip_msg_stop_operators_res_ptr, reason_code) ((kip_msg_stop_operators_res_ptr)->_data[3] = (uint16)(reason_code))
#define KIP_MSG_STOP_OPERATORS_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STOP_OPERATORS_RES_CREATE(status, con_id, Count, reason_code) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(reason_code)
#define KIP_MSG_STOP_OPERATORS_RES_PACK(kip_msg_stop_operators_res_ptr, status, con_id, Count, reason_code) \
    do { \
        (kip_msg_stop_operators_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_stop_operators_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_stop_operators_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
        (kip_msg_stop_operators_res_ptr)->_data[3] = (uint16)((uint16)(reason_code)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_CONNECT_REQ

  DESCRIPTION

  MEMBERS
    con_id       - connection id with sender/receiver processor and SW client
                   IDs as per common adaptor definition
    Source_ID    - Source to connect
    Sink_ID      - Sink to connect
    Transform_ID - Transform ID being imposed from P0
    Channel_ID   - Channel ID of data channel (may be 0 when secondary processor
                   is to create it or when all endpoints are on secondary
                   processor side)

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} KIP_MSG_STREAM_CONNECT_REQ;

/* The following macros take KIP_MSG_STREAM_CONNECT_REQ *kip_msg_stream_connect_req_ptr */
#define KIP_MSG_STREAM_CONNECT_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_STREAM_CONNECT_REQ_CON_ID_GET(kip_msg_stream_connect_req_ptr) ((kip_msg_stream_connect_req_ptr)->_data[0])
#define KIP_MSG_STREAM_CONNECT_REQ_CON_ID_SET(kip_msg_stream_connect_req_ptr, con_id) ((kip_msg_stream_connect_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_STREAM_CONNECT_REQ_SOURCE_ID_WORD_OFFSET (1)
#define KIP_MSG_STREAM_CONNECT_REQ_SOURCE_ID_GET(kip_msg_stream_connect_req_ptr) ((kip_msg_stream_connect_req_ptr)->_data[1])
#define KIP_MSG_STREAM_CONNECT_REQ_SOURCE_ID_SET(kip_msg_stream_connect_req_ptr, source_id) ((kip_msg_stream_connect_req_ptr)->_data[1] = (uint16)(source_id))
#define KIP_MSG_STREAM_CONNECT_REQ_SINK_ID_WORD_OFFSET (2)
#define KIP_MSG_STREAM_CONNECT_REQ_SINK_ID_GET(kip_msg_stream_connect_req_ptr) ((kip_msg_stream_connect_req_ptr)->_data[2])
#define KIP_MSG_STREAM_CONNECT_REQ_SINK_ID_SET(kip_msg_stream_connect_req_ptr, sink_id) ((kip_msg_stream_connect_req_ptr)->_data[2] = (uint16)(sink_id))
#define KIP_MSG_STREAM_CONNECT_REQ_TRANSFORM_ID_WORD_OFFSET (3)
#define KIP_MSG_STREAM_CONNECT_REQ_TRANSFORM_ID_GET(kip_msg_stream_connect_req_ptr) ((kip_msg_stream_connect_req_ptr)->_data[3])
#define KIP_MSG_STREAM_CONNECT_REQ_TRANSFORM_ID_SET(kip_msg_stream_connect_req_ptr, transform_id) ((kip_msg_stream_connect_req_ptr)->_data[3] = (uint16)(transform_id))
#define KIP_MSG_STREAM_CONNECT_REQ_CHANNEL_ID_WORD_OFFSET (4)
#define KIP_MSG_STREAM_CONNECT_REQ_CHANNEL_ID_GET(kip_msg_stream_connect_req_ptr) ((kip_msg_stream_connect_req_ptr)->_data[4])
#define KIP_MSG_STREAM_CONNECT_REQ_CHANNEL_ID_SET(kip_msg_stream_connect_req_ptr, channel_id) ((kip_msg_stream_connect_req_ptr)->_data[4] = (uint16)(channel_id))
#define KIP_MSG_STREAM_CONNECT_REQ_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_CONNECT_REQ_CREATE(con_id, Source_ID, Sink_ID, Transform_ID, Channel_ID) \
    (uint16)(con_id), \
    (uint16)(Source_ID), \
    (uint16)(Sink_ID), \
    (uint16)(Transform_ID), \
    (uint16)(Channel_ID)
#define KIP_MSG_STREAM_CONNECT_REQ_PACK(kip_msg_stream_connect_req_ptr, con_id, Source_ID, Sink_ID, Transform_ID, Channel_ID) \
    do { \
        (kip_msg_stream_connect_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_stream_connect_req_ptr)->_data[1] = (uint16)((uint16)(Source_ID)); \
        (kip_msg_stream_connect_req_ptr)->_data[2] = (uint16)((uint16)(Sink_ID)); \
        (kip_msg_stream_connect_req_ptr)->_data[3] = (uint16)((uint16)(Transform_ID)); \
        (kip_msg_stream_connect_req_ptr)->_data[4] = (uint16)((uint16)(Channel_ID)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_CONNECT_RES

  DESCRIPTION

  MEMBERS
    status       - status code
    con_id       - connection id with sender/receiver processor and SW client
                   IDs as per common adaptor definition
    Transform_ID - Streams transform id

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_STREAM_CONNECT_RES;

/* The following macros take KIP_MSG_STREAM_CONNECT_RES *kip_msg_stream_connect_res_ptr */
#define KIP_MSG_STREAM_CONNECT_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_STREAM_CONNECT_RES_STATUS_GET(kip_msg_stream_connect_res_ptr) ((kip_msg_stream_connect_res_ptr)->_data[0])
#define KIP_MSG_STREAM_CONNECT_RES_STATUS_SET(kip_msg_stream_connect_res_ptr, status) ((kip_msg_stream_connect_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_STREAM_CONNECT_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_STREAM_CONNECT_RES_CON_ID_GET(kip_msg_stream_connect_res_ptr) ((kip_msg_stream_connect_res_ptr)->_data[1])
#define KIP_MSG_STREAM_CONNECT_RES_CON_ID_SET(kip_msg_stream_connect_res_ptr, con_id) ((kip_msg_stream_connect_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_STREAM_CONNECT_RES_TRANSFORM_ID_WORD_OFFSET (2)
#define KIP_MSG_STREAM_CONNECT_RES_TRANSFORM_ID_GET(kip_msg_stream_connect_res_ptr) ((kip_msg_stream_connect_res_ptr)->_data[2])
#define KIP_MSG_STREAM_CONNECT_RES_TRANSFORM_ID_SET(kip_msg_stream_connect_res_ptr, transform_id) ((kip_msg_stream_connect_res_ptr)->_data[2] = (uint16)(transform_id))
#define KIP_MSG_STREAM_CONNECT_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_CONNECT_RES_CREATE(status, con_id, Transform_ID) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Transform_ID)
#define KIP_MSG_STREAM_CONNECT_RES_PACK(kip_msg_stream_connect_res_ptr, status, con_id, Transform_ID) \
    do { \
        (kip_msg_stream_connect_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_stream_connect_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_stream_connect_res_ptr)->_data[2] = (uint16)((uint16)(Transform_ID)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ

  DESCRIPTION

  MEMBERS
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Source_ID   - Source to connect
    Sink_ID     - Sink to connect
    Channel_ID  - Channel ID of data channel
    Buffer_size - Size of buffer required
    flags       - local endpoint flags

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ;

/* The following macros take KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ *kip_msg_stream_create_endpoints_req_ptr */
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_CON_ID_GET(kip_msg_stream_create_endpoints_req_ptr) ((kip_msg_stream_create_endpoints_req_ptr)->_data[0])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_CON_ID_SET(kip_msg_stream_create_endpoints_req_ptr, con_id) ((kip_msg_stream_create_endpoints_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_SOURCE_ID_WORD_OFFSET (1)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_SOURCE_ID_GET(kip_msg_stream_create_endpoints_req_ptr) ((kip_msg_stream_create_endpoints_req_ptr)->_data[1])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_SOURCE_ID_SET(kip_msg_stream_create_endpoints_req_ptr, source_id) ((kip_msg_stream_create_endpoints_req_ptr)->_data[1] = (uint16)(source_id))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_SINK_ID_WORD_OFFSET (2)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_SINK_ID_GET(kip_msg_stream_create_endpoints_req_ptr) ((kip_msg_stream_create_endpoints_req_ptr)->_data[2])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_SINK_ID_SET(kip_msg_stream_create_endpoints_req_ptr, sink_id) ((kip_msg_stream_create_endpoints_req_ptr)->_data[2] = (uint16)(sink_id))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_CHANNEL_ID_WORD_OFFSET (3)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_CHANNEL_ID_GET(kip_msg_stream_create_endpoints_req_ptr) ((kip_msg_stream_create_endpoints_req_ptr)->_data[3])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_CHANNEL_ID_SET(kip_msg_stream_create_endpoints_req_ptr, channel_id) ((kip_msg_stream_create_endpoints_req_ptr)->_data[3] = (uint16)(channel_id))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_BUFFER_SIZE_WORD_OFFSET (4)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_BUFFER_SIZE_GET(kip_msg_stream_create_endpoints_req_ptr) ((kip_msg_stream_create_endpoints_req_ptr)->_data[4])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_BUFFER_SIZE_SET(kip_msg_stream_create_endpoints_req_ptr, buffer_size) ((kip_msg_stream_create_endpoints_req_ptr)->_data[4] = (uint16)(buffer_size))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_FLAGS_WORD_OFFSET (5)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_FLAGS_GET(kip_msg_stream_create_endpoints_req_ptr) ((kip_msg_stream_create_endpoints_req_ptr)->_data[5])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_FLAGS_SET(kip_msg_stream_create_endpoints_req_ptr, flags) ((kip_msg_stream_create_endpoints_req_ptr)->_data[5] = (uint16)(flags))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_CREATE(con_id, Source_ID, Sink_ID, Channel_ID, Buffer_size, flags) \
    (uint16)(con_id), \
    (uint16)(Source_ID), \
    (uint16)(Sink_ID), \
    (uint16)(Channel_ID), \
    (uint16)(Buffer_size), \
    (uint16)(flags)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_REQ_PACK(kip_msg_stream_create_endpoints_req_ptr, con_id, Source_ID, Sink_ID, Channel_ID, Buffer_size, flags) \
    do { \
        (kip_msg_stream_create_endpoints_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_stream_create_endpoints_req_ptr)->_data[1] = (uint16)((uint16)(Source_ID)); \
        (kip_msg_stream_create_endpoints_req_ptr)->_data[2] = (uint16)((uint16)(Sink_ID)); \
        (kip_msg_stream_create_endpoints_req_ptr)->_data[3] = (uint16)((uint16)(Channel_ID)); \
        (kip_msg_stream_create_endpoints_req_ptr)->_data[4] = (uint16)((uint16)(Buffer_size)); \
        (kip_msg_stream_create_endpoints_req_ptr)->_data[5] = (uint16)((uint16)(flags)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_CREATE_ENDPOINTS_RES

  DESCRIPTION

  MEMBERS
    status      - status code
    con_id      - connection id with sender/receiver processor and SW client IDs
                  as per common adaptor definition
    Channel_ID  - Channel ID of data channel
    Buffer_size - Size of buffer required
    flags       - local endpoint flags

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} KIP_MSG_STREAM_CREATE_ENDPOINTS_RES;

/* The following macros take KIP_MSG_STREAM_CREATE_ENDPOINTS_RES *kip_msg_stream_create_endpoints_res_ptr */
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_STATUS_GET(kip_msg_stream_create_endpoints_res_ptr) ((kip_msg_stream_create_endpoints_res_ptr)->_data[0])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_STATUS_SET(kip_msg_stream_create_endpoints_res_ptr, status) ((kip_msg_stream_create_endpoints_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_CON_ID_GET(kip_msg_stream_create_endpoints_res_ptr) ((kip_msg_stream_create_endpoints_res_ptr)->_data[1])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_CON_ID_SET(kip_msg_stream_create_endpoints_res_ptr, con_id) ((kip_msg_stream_create_endpoints_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_CHANNEL_ID_WORD_OFFSET (2)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_CHANNEL_ID_GET(kip_msg_stream_create_endpoints_res_ptr) ((kip_msg_stream_create_endpoints_res_ptr)->_data[2])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_CHANNEL_ID_SET(kip_msg_stream_create_endpoints_res_ptr, channel_id) ((kip_msg_stream_create_endpoints_res_ptr)->_data[2] = (uint16)(channel_id))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_BUFFER_SIZE_WORD_OFFSET (3)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_BUFFER_SIZE_GET(kip_msg_stream_create_endpoints_res_ptr) ((kip_msg_stream_create_endpoints_res_ptr)->_data[3])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_BUFFER_SIZE_SET(kip_msg_stream_create_endpoints_res_ptr, buffer_size) ((kip_msg_stream_create_endpoints_res_ptr)->_data[3] = (uint16)(buffer_size))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_FLAGS_WORD_OFFSET (4)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_FLAGS_GET(kip_msg_stream_create_endpoints_res_ptr) ((kip_msg_stream_create_endpoints_res_ptr)->_data[4])
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_FLAGS_SET(kip_msg_stream_create_endpoints_res_ptr, flags) ((kip_msg_stream_create_endpoints_res_ptr)->_data[4] = (uint16)(flags))
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_CREATE(status, con_id, Channel_ID, Buffer_size, flags) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Channel_ID), \
    (uint16)(Buffer_size), \
    (uint16)(flags)
#define KIP_MSG_STREAM_CREATE_ENDPOINTS_RES_PACK(kip_msg_stream_create_endpoints_res_ptr, status, con_id, Channel_ID, Buffer_size, flags) \
    do { \
        (kip_msg_stream_create_endpoints_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_stream_create_endpoints_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_stream_create_endpoints_res_ptr)->_data[2] = (uint16)((uint16)(Channel_ID)); \
        (kip_msg_stream_create_endpoints_res_ptr)->_data[3] = (uint16)((uint16)(Buffer_size)); \
        (kip_msg_stream_create_endpoints_res_ptr)->_data[4] = (uint16)((uint16)(flags)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ

  DESCRIPTION

  MEMBERS
    con_id    - connection id with sender/receiver processor and SW client IDs
                as per common adaptor definition
    Source_ID - Source to connect
    Sink_ID   - Sink to connect

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ;

/* The following macros take KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ *kip_msg_stream_destroy_endpoints_req_ptr */
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_CON_ID_GET(kip_msg_stream_destroy_endpoints_req_ptr) ((kip_msg_stream_destroy_endpoints_req_ptr)->_data[0])
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_CON_ID_SET(kip_msg_stream_destroy_endpoints_req_ptr, con_id) ((kip_msg_stream_destroy_endpoints_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_SOURCE_ID_WORD_OFFSET (1)
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_SOURCE_ID_GET(kip_msg_stream_destroy_endpoints_req_ptr) ((kip_msg_stream_destroy_endpoints_req_ptr)->_data[1])
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_SOURCE_ID_SET(kip_msg_stream_destroy_endpoints_req_ptr, source_id) ((kip_msg_stream_destroy_endpoints_req_ptr)->_data[1] = (uint16)(source_id))
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_SINK_ID_WORD_OFFSET (2)
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_SINK_ID_GET(kip_msg_stream_destroy_endpoints_req_ptr) ((kip_msg_stream_destroy_endpoints_req_ptr)->_data[2])
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_SINK_ID_SET(kip_msg_stream_destroy_endpoints_req_ptr, sink_id) ((kip_msg_stream_destroy_endpoints_req_ptr)->_data[2] = (uint16)(sink_id))
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_CREATE(con_id, Source_ID, Sink_ID) \
    (uint16)(con_id), \
    (uint16)(Source_ID), \
    (uint16)(Sink_ID)
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_REQ_PACK(kip_msg_stream_destroy_endpoints_req_ptr, con_id, Source_ID, Sink_ID) \
    do { \
        (kip_msg_stream_destroy_endpoints_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_stream_destroy_endpoints_req_ptr)->_data[1] = (uint16)((uint16)(Source_ID)); \
        (kip_msg_stream_destroy_endpoints_req_ptr)->_data[2] = (uint16)((uint16)(Sink_ID)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES

  DESCRIPTION

  MEMBERS
    status - status code
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES;

/* The following macros take KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES *kip_msg_stream_destroy_endpoints_res_ptr */
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_STATUS_GET(kip_msg_stream_destroy_endpoints_res_ptr) ((kip_msg_stream_destroy_endpoints_res_ptr)->_data[0])
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_STATUS_SET(kip_msg_stream_destroy_endpoints_res_ptr, status) ((kip_msg_stream_destroy_endpoints_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_CON_ID_GET(kip_msg_stream_destroy_endpoints_res_ptr) ((kip_msg_stream_destroy_endpoints_res_ptr)->_data[1])
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_CON_ID_SET(kip_msg_stream_destroy_endpoints_res_ptr, con_id) ((kip_msg_stream_destroy_endpoints_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_CREATE(status, con_id) \
    (uint16)(status), \
    (uint16)(con_id)
#define KIP_MSG_STREAM_DESTROY_ENDPOINTS_RES_PACK(kip_msg_stream_destroy_endpoints_res_ptr, status, con_id) \
    do { \
        (kip_msg_stream_destroy_endpoints_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_stream_destroy_endpoints_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ

  DESCRIPTION

  MEMBERS
    con_id        - connection id with sender/receiver processor and SW client
                    IDs as per common adaptor definition
    Count         - Transform count
    transform_ids - The list of transform IDs to disconnect

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ;

/* The following macros take KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ *kip_msg_stream_transform_disconnect_req_ptr */
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_CON_ID_GET(kip_msg_stream_transform_disconnect_req_ptr) ((kip_msg_stream_transform_disconnect_req_ptr)->_data[0])
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_CON_ID_SET(kip_msg_stream_transform_disconnect_req_ptr, con_id) ((kip_msg_stream_transform_disconnect_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_COUNT_GET(kip_msg_stream_transform_disconnect_req_ptr) ((kip_msg_stream_transform_disconnect_req_ptr)->_data[1])
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_COUNT_SET(kip_msg_stream_transform_disconnect_req_ptr, count) ((kip_msg_stream_transform_disconnect_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_TRANSFORM_IDS_WORD_OFFSET (2)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_TRANSFORM_IDS_GET(kip_msg_stream_transform_disconnect_req_ptr) ((kip_msg_stream_transform_disconnect_req_ptr)->_data[2])
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_TRANSFORM_IDS_SET(kip_msg_stream_transform_disconnect_req_ptr, transform_ids) ((kip_msg_stream_transform_disconnect_req_ptr)->_data[2] = (uint16)(transform_ids))
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_CREATE(con_id, Count, transform_ids) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(transform_ids)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_REQ_PACK(kip_msg_stream_transform_disconnect_req_ptr, con_id, Count, transform_ids) \
    do { \
        (kip_msg_stream_transform_disconnect_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_stream_transform_disconnect_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_stream_transform_disconnect_req_ptr)->_data[2] = (uint16)((uint16)(transform_ids)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES

  DESCRIPTION

  MEMBERS
    status - status code
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    Count  - Returns count of successfully disconnected transforms

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES;

/* The following macros take KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES *kip_msg_stream_transform_disconnect_res_ptr */
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_STATUS_GET(kip_msg_stream_transform_disconnect_res_ptr) ((kip_msg_stream_transform_disconnect_res_ptr)->_data[0])
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_STATUS_SET(kip_msg_stream_transform_disconnect_res_ptr, status) ((kip_msg_stream_transform_disconnect_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_CON_ID_GET(kip_msg_stream_transform_disconnect_res_ptr) ((kip_msg_stream_transform_disconnect_res_ptr)->_data[1])
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_CON_ID_SET(kip_msg_stream_transform_disconnect_res_ptr, con_id) ((kip_msg_stream_transform_disconnect_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_COUNT_GET(kip_msg_stream_transform_disconnect_res_ptr) ((kip_msg_stream_transform_disconnect_res_ptr)->_data[2])
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_COUNT_SET(kip_msg_stream_transform_disconnect_res_ptr, count) ((kip_msg_stream_transform_disconnect_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_CREATE(status, con_id, Count) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count)
#define KIP_MSG_STREAM_TRANSFORM_DISCONNECT_RES_PACK(kip_msg_stream_transform_disconnect_res_ptr, status, con_id, Count) \
    do { \
        (kip_msg_stream_transform_disconnect_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_stream_transform_disconnect_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_stream_transform_disconnect_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ

  DESCRIPTION

  MEMBERS
    con_id        - connection id with sender/receiver processor and SW client
                    IDs as per common adaptor definition
    Count         - Transform count
    transform_ids - The list of transform IDs to remove on P0 from
                    kip_transform_list

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ;

/* The following macros take KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ *kip_msg_transform_list_remove_entry_req_ptr */
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_CON_ID_WORD_OFFSET (0)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_CON_ID_GET(kip_msg_transform_list_remove_entry_req_ptr) ((kip_msg_transform_list_remove_entry_req_ptr)->_data[0])
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_CON_ID_SET(kip_msg_transform_list_remove_entry_req_ptr, con_id) ((kip_msg_transform_list_remove_entry_req_ptr)->_data[0] = (uint16)(con_id))
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_COUNT_WORD_OFFSET (1)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_COUNT_GET(kip_msg_transform_list_remove_entry_req_ptr) ((kip_msg_transform_list_remove_entry_req_ptr)->_data[1])
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_COUNT_SET(kip_msg_transform_list_remove_entry_req_ptr, count) ((kip_msg_transform_list_remove_entry_req_ptr)->_data[1] = (uint16)(count))
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_TRANSFORM_IDS_WORD_OFFSET (2)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_TRANSFORM_IDS_GET(kip_msg_transform_list_remove_entry_req_ptr) ((kip_msg_transform_list_remove_entry_req_ptr)->_data[2])
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_TRANSFORM_IDS_SET(kip_msg_transform_list_remove_entry_req_ptr, transform_ids) ((kip_msg_transform_list_remove_entry_req_ptr)->_data[2] = (uint16)(transform_ids))
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_CREATE(con_id, Count, transform_ids) \
    (uint16)(con_id), \
    (uint16)(Count), \
    (uint16)(transform_ids)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_REQ_PACK(kip_msg_transform_list_remove_entry_req_ptr, con_id, Count, transform_ids) \
    do { \
        (kip_msg_transform_list_remove_entry_req_ptr)->_data[0] = (uint16)((uint16)(con_id)); \
        (kip_msg_transform_list_remove_entry_req_ptr)->_data[1] = (uint16)((uint16)(Count)); \
        (kip_msg_transform_list_remove_entry_req_ptr)->_data[2] = (uint16)((uint16)(transform_ids)); \
    } while (0)


/*******************************************************************************

  NAME
    KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES

  DESCRIPTION

  MEMBERS
    status - status code
    con_id - connection id with sender/receiver processor and SW client IDs as
             per common adaptor definition
    Count  - Returns count of successfully removed kip_transform_list entries on
             P0

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES;

/* The following macros take KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES *kip_msg_transform_list_remove_entry_res_ptr */
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_STATUS_WORD_OFFSET (0)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_STATUS_GET(kip_msg_transform_list_remove_entry_res_ptr) ((kip_msg_transform_list_remove_entry_res_ptr)->_data[0])
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_STATUS_SET(kip_msg_transform_list_remove_entry_res_ptr, status) ((kip_msg_transform_list_remove_entry_res_ptr)->_data[0] = (uint16)(status))
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_CON_ID_WORD_OFFSET (1)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_CON_ID_GET(kip_msg_transform_list_remove_entry_res_ptr) ((kip_msg_transform_list_remove_entry_res_ptr)->_data[1])
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_CON_ID_SET(kip_msg_transform_list_remove_entry_res_ptr, con_id) ((kip_msg_transform_list_remove_entry_res_ptr)->_data[1] = (uint16)(con_id))
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_COUNT_WORD_OFFSET (2)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_COUNT_GET(kip_msg_transform_list_remove_entry_res_ptr) ((kip_msg_transform_list_remove_entry_res_ptr)->_data[2])
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_COUNT_SET(kip_msg_transform_list_remove_entry_res_ptr, count) ((kip_msg_transform_list_remove_entry_res_ptr)->_data[2] = (uint16)(count))
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_CREATE(status, con_id, Count) \
    (uint16)(status), \
    (uint16)(con_id), \
    (uint16)(Count)
#define KIP_MSG_TRANSFORM_LIST_REMOVE_ENTRY_RES_PACK(kip_msg_transform_list_remove_entry_res_ptr, status, con_id, Count) \
    do { \
        (kip_msg_transform_list_remove_entry_res_ptr)->_data[0] = (uint16)((uint16)(status)); \
        (kip_msg_transform_list_remove_entry_res_ptr)->_data[1] = (uint16)((uint16)(con_id)); \
        (kip_msg_transform_list_remove_entry_res_ptr)->_data[2] = (uint16)((uint16)(Count)); \
    } while (0)


#endif /* KIP_MSG_PRIM_H */

