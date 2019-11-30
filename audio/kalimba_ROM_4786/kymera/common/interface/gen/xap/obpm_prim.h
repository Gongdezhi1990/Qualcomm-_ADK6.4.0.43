/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2017
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef OBPM_PRIM_H
#define OBPM_PRIM_H

#include "opmsg_prim.h"

#define MAP_CAPID_UCID_SBID_TO_PSKEYID(capid, ucid, sbid)               ((((capid) & CAPID_PS_MASK) << CAPID_PS_SHIFT) | \
                                                                         (((ucid) & UCID_PS_MASK) << UCID_PS_SHIFT) | \
                                                                         (((sbid) & SBID_PS_MASK) << SBID_PS_SHIFT))
#define EXTRACT_CAPID_FROM_LOGICAL_PSKEYID(id)                          (((id) >> CAPID_PS_SHIFT) & CAPID_PS_MASK)
#define EXTRACT_SBID_FROM_LOGICAL_PSKEYID(id)                           (((id) >> SBID_PS_SHIFT) & SBID_PS_MASK)
#define EXTRACT_UCID_FROM_LOGICAL_PSKEYID(id)                           (((id) >> UCID_PS_SHIFT) & UCID_PS_MASK)
#define SBID_PS_SHIFT                                                   (0)
#define SBID_PS_MASK                                                    (0x0001)
#define UCID_PS_MASK                                                    (0x003F)
#define CAPID_PS_MASK                                                   (0xFFFF)
#define UCID_PS_SHIFT                                                   (1)
#define CAPID_PS_SHIFT                                                  (7)


/*******************************************************************************

  NAME
    OBPM_SYS_EVENT

  DESCRIPTION
    Flags for system status events

 VALUES
    OP_CREATE     - Operator Created
    OP_DESTROY    - Operator Destroyed
    OP_START      - Operator Started
    OP_STOP       - Operator Stopped
    OP_RESET      - Operator Reset
    OP_CONNECT    - Route Connected
    OP_DISCONNECT - Route DIsconnected
    EP_CREATE     - Real Endpoint Created
    EP_DESTROY    - Real Endpoint Destroyed
    UCID_CHANGE   - Operator UCID change
    PS_READY      - PS-Store Ready for use
    LICENCE_READY - Licencing system available
    MIP_AVAILABLE - MIP measurements available

*******************************************************************************/
typedef enum
{
    OBPM_SYS_EVENT_OP_CREATE = 0x0001,
    OBPM_SYS_EVENT_OP_DESTROY = 0x0002,
    OBPM_SYS_EVENT_OP_START = 0x0004,
    OBPM_SYS_EVENT_OP_STOP = 0x0008,
    OBPM_SYS_EVENT_OP_RESET = 0x0010,
    OBPM_SYS_EVENT_OP_CONNECT = 0x0020,
    OBPM_SYS_EVENT_OP_DISCONNECT = 0x0040,
    OBPM_SYS_EVENT_EP_CREATE = 0x0100,
    OBPM_SYS_EVENT_EP_DESTROY = 0x0200,
    OBPM_SYS_EVENT_UCID_CHANGE = 0x0400,
    OBPM_SYS_EVENT_PS_READY = 0x1000,
    OBPM_SYS_EVENT_LICENCE_READY = 0x2000,
    OBPM_SYS_EVENT_MIP_AVAILABLE = 0x4000
} OBPM_SYS_EVENT;
/*******************************************************************************

  NAME
    OBPM_Signal_Id

  DESCRIPTION
    IDs for now are rather arbitrary, just to avoid overlap with many draft
    IDs that are evolving.

 VALUES
    GET_OPID_LIST_REQ            - If capability ID is non-zero, Kymera will
                                   return only those operators IDs that were
                                   instantiated from the specified capability.
                                   If capability ID is zero, Kymera will return
                                   a list of all operator IDs and their
                                   capability IDs.
    GET_OPID_LIST_RES            - If the message failed for some reason (e.g.
                                   no result of the capability ID-based
                                   filtering or no operators exist at all), the
                                   response has no payload (length = 0).
    OPERATOR_MESSAGE_REQ         - This message is sent to Kymera operators.
                                   Some operator messages may only have the
                                   operator message ID and no extra data.
    OPERATOR_MESSAGE_RES         - Response message. Status code is always
                                   present, 0 for success, otherwise it carries
                                   an error code.
    GET_CONN_LIST_REQ            - Requests a list of connection (or transform)
                                   IDs. Response will contain these together
                                   with the source/sink IDs of each
                                   connection.If source and/or sink ID are
                                   non-zero, Kymera will filter based on these
                                   i.e. return only those connections that
                                   contain the specified endpoint(s) or
                                   operators. E.g. one can retrieve all
                                   connections where operator 0x4040 is
                                   connected as source (so response will contain
                                   e.g. 0x6041 if second source terminal is
                                   connected), or where 0x2ea0 is the source
                                   endpoint. If source and/or sink IDs are zero,
                                   Kymera ignores the respective parameter from
                                   a transform filtering viewpoint. One can
                                   therefore obtain only connections where one
                                   endpoint or operator matches the specified
                                   one, or all connections that exist at that
                                   point in time.
    GET_CONN_LIST_RES            - Response message. If the message failed for
                                   some reason (e.g. no result from filtering or
                                   no connections exist at all), the response
                                   has no payload (length = 0).
    GET_INSTALLED_CAPID_LIST_REQ - Requests a list of capabilities that are
                                   currently installed on the device and can be
                                   instantiated. This may include capabilities
                                   that have been downloaded and are not part of
                                   the original image. It is highly likely that
                                   this includes capabilities that have not been
                                   instantiated at the current time.
    GET_INSTALLED_CAPID_LIST_RES - Response message. If the message failed for
                                   some reason then the response has no payload.
    GET_ENDPOINT_TYPE_REQ        - Requests endpoint type information.
    GET_ENDPOINT_TYPE_RES        - Response message
    GET_ENDPOINT_CONFIG_REQ      - Requests endpoint configuration information.
    GET_ENDPOINT_CONFIG_RES      - Response message
    SET_ENDPOINT_CONFIG_REQ      - Requests setting endpoint configuration
                                   information.
    SET_ENDPOINT_CONFIG_RES      - Response message
    GET_SYSTEM_STREAM_RATE_REQ   - Get the configured system streaming rate.
    GET_SYSTEM_STREAM_RATE_RES   - Response message to system streaming rate.
    SET_SYSTEM_STREAM_RATE_REQ   - Set the configured system streaming rate.
    SET_SYSTEM_STREAM_RATE_RES   - Response message to system streaming rate.
    WRITE_PS_ENTRY_REQ           - Writes PS key.
    WRITE_PS_ENTRY_RES           - Response message
    READ_PS_ENTRY_REQ            - Reads PS key.
    READ_PS_ENTRY_RES            - Response message
    GET_BUILD_INFO_REQ           - Get build ID information.
    GET_BUILD_INFO_RES           - Response message
    GET_MEM_USAGE_REQ            - Get memory usage.
    GET_MEM_USAGE_RES            - Response message to memory usage.
    CLEAR_MEM_WATERMARKS_REQ     - Clear memory watermarks.
    CLEAR_MEM_WATERMARKS_RES     - Response message to clear memory watermarks.
    GET_MIPS_USAGE_REQ           - Get mips usage for an operator.
    GET_MIPS_USAGE_RES           - Response message to mips usage.
    LICENCE_STATUS_REQ           - Licence status query
    LICENCE_STATUS_RES           - Licence status response
    SYSTEM_EVENT_REQ             - System Event query
    SYSTEM_EVENT_RES             - System Even response
    ENABLE_PROFILER_REQ          - Enable/disable profiler.
    ENABLE_PROFILER_RES          - Response message to Enable/disable profiler
                                   request.

*******************************************************************************/
typedef enum
{
    OBPM_SIGNAL_ID_GET_OPID_LIST_REQ = 0x4242,
    OBPM_SIGNAL_ID_GET_OPID_LIST_RES = 0x4243,
    OBPM_SIGNAL_ID_OPERATOR_MESSAGE_REQ = 0x4343,
    OBPM_SIGNAL_ID_OPERATOR_MESSAGE_RES = 0x4344,
    OBPM_SIGNAL_ID_GET_CONN_LIST_REQ = 0x4444,
    OBPM_SIGNAL_ID_GET_CONN_LIST_RES = 0x4445,
    OBPM_SIGNAL_ID_GET_INSTALLED_CAPID_LIST_REQ = 0x4446,
    OBPM_SIGNAL_ID_GET_INSTALLED_CAPID_LIST_RES = 0x4447,
    OBPM_SIGNAL_ID_GET_ENDPOINT_TYPE_REQ = 0x4545,
    OBPM_SIGNAL_ID_GET_ENDPOINT_TYPE_RES = 0x4546,
    OBPM_SIGNAL_ID_GET_ENDPOINT_CONFIG_REQ = 0x4646,
    OBPM_SIGNAL_ID_GET_ENDPOINT_CONFIG_RES = 0x4647,
    OBPM_SIGNAL_ID_SET_ENDPOINT_CONFIG_REQ = 0x4747,
    OBPM_SIGNAL_ID_SET_ENDPOINT_CONFIG_RES = 0x4748,
    OBPM_SIGNAL_ID_GET_SYSTEM_STREAM_RATE_REQ = 0x4749,
    OBPM_SIGNAL_ID_GET_SYSTEM_STREAM_RATE_RES = 0x474A,
    OBPM_SIGNAL_ID_SET_SYSTEM_STREAM_RATE_REQ = 0x474B,
    OBPM_SIGNAL_ID_SET_SYSTEM_STREAM_RATE_RES = 0x474C,
    OBPM_SIGNAL_ID_WRITE_PS_ENTRY_REQ = 0x4848,
    OBPM_SIGNAL_ID_WRITE_PS_ENTRY_RES = 0x4849,
    OBPM_SIGNAL_ID_READ_PS_ENTRY_REQ = 0x4949,
    OBPM_SIGNAL_ID_READ_PS_ENTRY_RES = 0x494A,
    OBPM_SIGNAL_ID_GET_BUILD_INFO_REQ = 0x4A4A,
    OBPM_SIGNAL_ID_GET_BUILD_INFO_RES = 0x4A4B,
    OBPM_SIGNAL_ID_GET_MEM_USAGE_REQ = 0x4A4C,
    OBPM_SIGNAL_ID_GET_MEM_USAGE_RES = 0x4A4D,
    OBPM_SIGNAL_ID_CLEAR_MEM_WATERMARKS_REQ = 0x4A4E,
    OBPM_SIGNAL_ID_CLEAR_MEM_WATERMARKS_RES = 0x4A4F,
    OBPM_SIGNAL_ID_GET_MIPS_USAGE_REQ = 0x4A50,
    OBPM_SIGNAL_ID_GET_MIPS_USAGE_RES = 0x4A51,
    OBPM_SIGNAL_ID_LICENCE_STATUS_REQ = 0x4A52,
    OBPM_SIGNAL_ID_LICENCE_STATUS_RES = 0x4A53,
    OBPM_SIGNAL_ID_SYSTEM_EVENT_REQ = 0x4A54,
    OBPM_SIGNAL_ID_SYSTEM_EVENT_RES = 0x4A55,
    OBPM_SIGNAL_ID_ENABLE_PROFILER_REQ = 0x4A56,
    OBPM_SIGNAL_ID_ENABLE_PROFILER_RES = 0x4A57
} OBPM_SIGNAL_ID;


/*******************************************************************************

  NAME
    OBPM_Status_Id

  DESCRIPTION
    Status IDs type of the response message.

*******************************************************************************/
typedef uint16 OBPM_Status_Id;


#define OBPM_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    OBPM_CLEAR_MEM_WATERMARKS_RES

  DESCRIPTION
    Response message showing if the mem watermarks was successfully cleared.

  MEMBERS
    Status - Returns STATUS_OK if the operation was successful.

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_CLEAR_MEM_WATERMARKS_RES;

/* The following macros take OBPM_CLEAR_MEM_WATERMARKS_RES *obpm_clear_mem_watermarks_res_ptr */
#define OBPM_CLEAR_MEM_WATERMARKS_RES_STATUS_WORD_OFFSET (0)
#define OBPM_CLEAR_MEM_WATERMARKS_RES_STATUS_GET(obpm_clear_mem_watermarks_res_ptr) ((obpm_clear_mem_watermarks_res_ptr)->_data[0])
#define OBPM_CLEAR_MEM_WATERMARKS_RES_STATUS_SET(obpm_clear_mem_watermarks_res_ptr, status) ((obpm_clear_mem_watermarks_res_ptr)->_data[0] = (uint16)(status))
#define OBPM_CLEAR_MEM_WATERMARKS_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_CLEAR_MEM_WATERMARKS_RES_CREATE(Status) \
    (uint16)(Status)
#define OBPM_CLEAR_MEM_WATERMARKS_RES_PACK(obpm_clear_mem_watermarks_res_ptr, Status) \
    do { \
        (obpm_clear_mem_watermarks_res_ptr)->_data[0] = (uint16)((uint16)(Status)); \
    } while (0)

#define OBPM_CLEAR_MEM_WATERMARKS_RES_MARSHALL(addr, obpm_clear_mem_watermarks_res_ptr) memcpy((void *)(addr), (void *)(obpm_clear_mem_watermarks_res_ptr), 1)
#define OBPM_CLEAR_MEM_WATERMARKS_RES_UNMARSHALL(addr, obpm_clear_mem_watermarks_res_ptr) memcpy((void *)(obpm_clear_mem_watermarks_res_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_CONN_LIST_REQ

  DESCRIPTION

  MEMBERS
    Source_ID - Source ID (can be zero)
    Sink_ID   - Sink ID (can be zero)

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_CONN_LIST_REQ;

/* The following macros take OBPM_CONN_LIST_REQ *obpm_conn_list_req_ptr */
#define OBPM_CONN_LIST_REQ_SOURCE_ID_WORD_OFFSET (0)
#define OBPM_CONN_LIST_REQ_SOURCE_ID_GET(obpm_conn_list_req_ptr) ((obpm_conn_list_req_ptr)->_data[0])
#define OBPM_CONN_LIST_REQ_SOURCE_ID_SET(obpm_conn_list_req_ptr, source_id) ((obpm_conn_list_req_ptr)->_data[0] = (uint16)(source_id))
#define OBPM_CONN_LIST_REQ_SINK_ID_WORD_OFFSET (1)
#define OBPM_CONN_LIST_REQ_SINK_ID_GET(obpm_conn_list_req_ptr) ((obpm_conn_list_req_ptr)->_data[1])
#define OBPM_CONN_LIST_REQ_SINK_ID_SET(obpm_conn_list_req_ptr, sink_id) ((obpm_conn_list_req_ptr)->_data[1] = (uint16)(sink_id))
#define OBPM_CONN_LIST_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_CONN_LIST_REQ_CREATE(Source_ID, Sink_ID) \
    (uint16)(Source_ID), \
    (uint16)(Sink_ID)
#define OBPM_CONN_LIST_REQ_PACK(obpm_conn_list_req_ptr, Source_ID, Sink_ID) \
    do { \
        (obpm_conn_list_req_ptr)->_data[0] = (uint16)((uint16)(Source_ID)); \
        (obpm_conn_list_req_ptr)->_data[1] = (uint16)((uint16)(Sink_ID)); \
    } while (0)

#define OBPM_CONN_LIST_REQ_MARSHALL(addr, obpm_conn_list_req_ptr) memcpy((void *)(addr), (void *)(obpm_conn_list_req_ptr), 2)
#define OBPM_CONN_LIST_REQ_UNMARSHALL(addr, obpm_conn_list_req_ptr) memcpy((void *)(obpm_conn_list_req_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_CONN_LIST_RES

  DESCRIPTION

  MEMBERS
    Conn_ID   - Connection ID
    Source_ID - Source ID (can be zero)
    Sink_ID   - Sink ID (can be zero)
    payload   - Arguments

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OBPM_CONN_LIST_RES;

/* The following macros take OBPM_CONN_LIST_RES *obpm_conn_list_res_ptr */
#define OBPM_CONN_LIST_RES_CONN_ID_WORD_OFFSET (0)
#define OBPM_CONN_LIST_RES_CONN_ID_GET(obpm_conn_list_res_ptr) ((obpm_conn_list_res_ptr)->_data[0])
#define OBPM_CONN_LIST_RES_CONN_ID_SET(obpm_conn_list_res_ptr, conn_id) ((obpm_conn_list_res_ptr)->_data[0] = (uint16)(conn_id))
#define OBPM_CONN_LIST_RES_SOURCE_ID_WORD_OFFSET (1)
#define OBPM_CONN_LIST_RES_SOURCE_ID_GET(obpm_conn_list_res_ptr) ((obpm_conn_list_res_ptr)->_data[1])
#define OBPM_CONN_LIST_RES_SOURCE_ID_SET(obpm_conn_list_res_ptr, source_id) ((obpm_conn_list_res_ptr)->_data[1] = (uint16)(source_id))
#define OBPM_CONN_LIST_RES_SINK_ID_WORD_OFFSET (2)
#define OBPM_CONN_LIST_RES_SINK_ID_GET(obpm_conn_list_res_ptr) ((obpm_conn_list_res_ptr)->_data[2])
#define OBPM_CONN_LIST_RES_SINK_ID_SET(obpm_conn_list_res_ptr, sink_id) ((obpm_conn_list_res_ptr)->_data[2] = (uint16)(sink_id))
#define OBPM_CONN_LIST_RES_PAYLOAD_WORD_OFFSET (3)
#define OBPM_CONN_LIST_RES_PAYLOAD_GET(obpm_conn_list_res_ptr) ((obpm_conn_list_res_ptr)->_data[3])
#define OBPM_CONN_LIST_RES_PAYLOAD_SET(obpm_conn_list_res_ptr, payload) ((obpm_conn_list_res_ptr)->_data[3] = (uint16)(payload))
#define OBPM_CONN_LIST_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OBPM_CONN_LIST_RES_CREATE(Conn_ID, Source_ID, Sink_ID, payload) \
    (uint16)(Conn_ID), \
    (uint16)(Source_ID), \
    (uint16)(Sink_ID), \
    (uint16)(payload)
#define OBPM_CONN_LIST_RES_PACK(obpm_conn_list_res_ptr, Conn_ID, Source_ID, Sink_ID, payload) \
    do { \
        (obpm_conn_list_res_ptr)->_data[0] = (uint16)((uint16)(Conn_ID)); \
        (obpm_conn_list_res_ptr)->_data[1] = (uint16)((uint16)(Source_ID)); \
        (obpm_conn_list_res_ptr)->_data[2] = (uint16)((uint16)(Sink_ID)); \
        (obpm_conn_list_res_ptr)->_data[3] = (uint16)((uint16)(payload)); \
    } while (0)

#define OBPM_CONN_LIST_RES_MARSHALL(addr, obpm_conn_list_res_ptr) memcpy((void *)(addr), (void *)(obpm_conn_list_res_ptr), 4)
#define OBPM_CONN_LIST_RES_UNMARSHALL(addr, obpm_conn_list_res_ptr) memcpy((void *)(obpm_conn_list_res_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    OBPM_ENABLE_PROFILER_REQ

  DESCRIPTION
    Request message for enabling/disabling the profiler. Status set to 0
    disables the profiler, while setting it to 1 enables the profiler.

  MEMBERS
    status - New profiler status (0 - disabled / 1 - enabled)

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_ENABLE_PROFILER_REQ;

/* The following macros take OBPM_ENABLE_PROFILER_REQ *obpm_enable_profiler_req_ptr */
#define OBPM_ENABLE_PROFILER_REQ_STATUS_WORD_OFFSET (0)
#define OBPM_ENABLE_PROFILER_REQ_STATUS_GET(obpm_enable_profiler_req_ptr) ((obpm_enable_profiler_req_ptr)->_data[0])
#define OBPM_ENABLE_PROFILER_REQ_STATUS_SET(obpm_enable_profiler_req_ptr, status) ((obpm_enable_profiler_req_ptr)->_data[0] = (uint16)(status))
#define OBPM_ENABLE_PROFILER_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_ENABLE_PROFILER_REQ_CREATE(status) \
    (uint16)(status)
#define OBPM_ENABLE_PROFILER_REQ_PACK(obpm_enable_profiler_req_ptr, status) \
    do { \
        (obpm_enable_profiler_req_ptr)->_data[0] = (uint16)((uint16)(status)); \
    } while (0)

#define OBPM_ENABLE_PROFILER_REQ_MARSHALL(addr, obpm_enable_profiler_req_ptr) memcpy((void *)(addr), (void *)(obpm_enable_profiler_req_ptr), 1)
#define OBPM_ENABLE_PROFILER_REQ_UNMARSHALL(addr, obpm_enable_profiler_req_ptr) memcpy((void *)(obpm_enable_profiler_req_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_ENABLE_PROFILER_RES

  DESCRIPTION
    Response message holding the status after enabling/disabling profiler.

  MEMBERS
    Status - Returns STATUS_OK if the operation was correctly executed.

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_ENABLE_PROFILER_RES;

/* The following macros take OBPM_ENABLE_PROFILER_RES *obpm_enable_profiler_res_ptr */
#define OBPM_ENABLE_PROFILER_RES_STATUS_WORD_OFFSET (0)
#define OBPM_ENABLE_PROFILER_RES_STATUS_GET(obpm_enable_profiler_res_ptr) ((obpm_enable_profiler_res_ptr)->_data[0])
#define OBPM_ENABLE_PROFILER_RES_STATUS_SET(obpm_enable_profiler_res_ptr, status) ((obpm_enable_profiler_res_ptr)->_data[0] = (uint16)(status))
#define OBPM_ENABLE_PROFILER_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_ENABLE_PROFILER_RES_CREATE(Status) \
    (uint16)(Status)
#define OBPM_ENABLE_PROFILER_RES_PACK(obpm_enable_profiler_res_ptr, Status) \
    do { \
        (obpm_enable_profiler_res_ptr)->_data[0] = (uint16)((uint16)(Status)); \
    } while (0)

#define OBPM_ENABLE_PROFILER_RES_MARSHALL(addr, obpm_enable_profiler_res_ptr) memcpy((void *)(addr), (void *)(obpm_enable_profiler_res_ptr), 1)
#define OBPM_ENABLE_PROFILER_RES_UNMARSHALL(addr, obpm_enable_profiler_res_ptr) memcpy((void *)(obpm_enable_profiler_res_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_GET_BUILD_INFO_RES

  DESCRIPTION

  MEMBERS
    Kymera_ver_MSW - Numerical Build ID
    Kymera_ver_LSW - Numerical Build ID
    Build_ID       - Numerical Build ID
    Build_String   - Build ID string, LE-packed characters. It may contain a
                     trailing NULL word.

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OBPM_GET_BUILD_INFO_RES;

/* The following macros take OBPM_GET_BUILD_INFO_RES *obpm_get_build_info_res_ptr */
#define OBPM_GET_BUILD_INFO_RES_KYMERA_VER_MSW_WORD_OFFSET (0)
#define OBPM_GET_BUILD_INFO_RES_KYMERA_VER_MSW_GET(obpm_get_build_info_res_ptr) ((obpm_get_build_info_res_ptr)->_data[0])
#define OBPM_GET_BUILD_INFO_RES_KYMERA_VER_MSW_SET(obpm_get_build_info_res_ptr, kymera_ver_msw) ((obpm_get_build_info_res_ptr)->_data[0] = (uint16)(kymera_ver_msw))
#define OBPM_GET_BUILD_INFO_RES_KYMERA_VER_LSW_WORD_OFFSET (1)
#define OBPM_GET_BUILD_INFO_RES_KYMERA_VER_LSW_GET(obpm_get_build_info_res_ptr) ((obpm_get_build_info_res_ptr)->_data[1])
#define OBPM_GET_BUILD_INFO_RES_KYMERA_VER_LSW_SET(obpm_get_build_info_res_ptr, kymera_ver_lsw) ((obpm_get_build_info_res_ptr)->_data[1] = (uint16)(kymera_ver_lsw))
#define OBPM_GET_BUILD_INFO_RES_BUILD_ID_WORD_OFFSET (2)
#define OBPM_GET_BUILD_INFO_RES_BUILD_ID_GET(obpm_get_build_info_res_ptr) ((obpm_get_build_info_res_ptr)->_data[2])
#define OBPM_GET_BUILD_INFO_RES_BUILD_ID_SET(obpm_get_build_info_res_ptr, build_id) ((obpm_get_build_info_res_ptr)->_data[2] = (uint16)(build_id))
#define OBPM_GET_BUILD_INFO_RES_BUILD_STRING_WORD_OFFSET (3)
#define OBPM_GET_BUILD_INFO_RES_BUILD_STRING_GET(obpm_get_build_info_res_ptr) ((obpm_get_build_info_res_ptr)->_data[3])
#define OBPM_GET_BUILD_INFO_RES_BUILD_STRING_SET(obpm_get_build_info_res_ptr, build_string) ((obpm_get_build_info_res_ptr)->_data[3] = (uint16)(build_string))
#define OBPM_GET_BUILD_INFO_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_BUILD_INFO_RES_CREATE(Kymera_ver_MSW, Kymera_ver_LSW, Build_ID, Build_String) \
    (uint16)(Kymera_ver_MSW), \
    (uint16)(Kymera_ver_LSW), \
    (uint16)(Build_ID), \
    (uint16)(Build_String)
#define OBPM_GET_BUILD_INFO_RES_PACK(obpm_get_build_info_res_ptr, Kymera_ver_MSW, Kymera_ver_LSW, Build_ID, Build_String) \
    do { \
        (obpm_get_build_info_res_ptr)->_data[0] = (uint16)((uint16)(Kymera_ver_MSW)); \
        (obpm_get_build_info_res_ptr)->_data[1] = (uint16)((uint16)(Kymera_ver_LSW)); \
        (obpm_get_build_info_res_ptr)->_data[2] = (uint16)((uint16)(Build_ID)); \
        (obpm_get_build_info_res_ptr)->_data[3] = (uint16)((uint16)(Build_String)); \
    } while (0)

#define OBPM_GET_BUILD_INFO_RES_MARSHALL(addr, obpm_get_build_info_res_ptr) memcpy((void *)(addr), (void *)(obpm_get_build_info_res_ptr), 4)
#define OBPM_GET_BUILD_INFO_RES_UNMARSHALL(addr, obpm_get_build_info_res_ptr) memcpy((void *)(obpm_get_build_info_res_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    OBPM_GET_ENDPOINT_CONFIG_REQ

  DESCRIPTION

  MEMBERS
    Endpoint_ID - External endpoint ID as seen by host
    Key_IDs     - One or more key IDs of 32 bits each, MSW then LSW

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_GET_ENDPOINT_CONFIG_REQ;

/* The following macros take OBPM_GET_ENDPOINT_CONFIG_REQ *obpm_get_endpoint_config_req_ptr */
#define OBPM_GET_ENDPOINT_CONFIG_REQ_ENDPOINT_ID_WORD_OFFSET (0)
#define OBPM_GET_ENDPOINT_CONFIG_REQ_ENDPOINT_ID_GET(obpm_get_endpoint_config_req_ptr) ((obpm_get_endpoint_config_req_ptr)->_data[0])
#define OBPM_GET_ENDPOINT_CONFIG_REQ_ENDPOINT_ID_SET(obpm_get_endpoint_config_req_ptr, endpoint_id) ((obpm_get_endpoint_config_req_ptr)->_data[0] = (uint16)(endpoint_id))
#define OBPM_GET_ENDPOINT_CONFIG_REQ_KEY_IDS_WORD_OFFSET (1)
#define OBPM_GET_ENDPOINT_CONFIG_REQ_KEY_IDS_GET(obpm_get_endpoint_config_req_ptr) ((obpm_get_endpoint_config_req_ptr)->_data[1])
#define OBPM_GET_ENDPOINT_CONFIG_REQ_KEY_IDS_SET(obpm_get_endpoint_config_req_ptr, key_ids) ((obpm_get_endpoint_config_req_ptr)->_data[1] = (uint16)(key_ids))
#define OBPM_GET_ENDPOINT_CONFIG_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_ENDPOINT_CONFIG_REQ_CREATE(Endpoint_ID, Key_IDs) \
    (uint16)(Endpoint_ID), \
    (uint16)(Key_IDs)
#define OBPM_GET_ENDPOINT_CONFIG_REQ_PACK(obpm_get_endpoint_config_req_ptr, Endpoint_ID, Key_IDs) \
    do { \
        (obpm_get_endpoint_config_req_ptr)->_data[0] = (uint16)((uint16)(Endpoint_ID)); \
        (obpm_get_endpoint_config_req_ptr)->_data[1] = (uint16)((uint16)(Key_IDs)); \
    } while (0)

#define OBPM_GET_ENDPOINT_CONFIG_REQ_MARSHALL(addr, obpm_get_endpoint_config_req_ptr) memcpy((void *)(addr), (void *)(obpm_get_endpoint_config_req_ptr), 2)
#define OBPM_GET_ENDPOINT_CONFIG_REQ_UNMARSHALL(addr, obpm_get_endpoint_config_req_ptr) memcpy((void *)(obpm_get_endpoint_config_req_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_GET_ENDPOINT_CONFIG_RES

  DESCRIPTION

  MEMBERS
    Status         - Status code
    Endpoint_ID    - External endpoint ID as seen by host
    Key_IDs_values - key ID MSW, key ID LSW, value MSW, value LSW quad(s)

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OBPM_GET_ENDPOINT_CONFIG_RES;

/* The following macros take OBPM_GET_ENDPOINT_CONFIG_RES *obpm_get_endpoint_config_res_ptr */
#define OBPM_GET_ENDPOINT_CONFIG_RES_STATUS_WORD_OFFSET (0)
#define OBPM_GET_ENDPOINT_CONFIG_RES_STATUS_GET(obpm_get_endpoint_config_res_ptr) ((obpm_get_endpoint_config_res_ptr)->_data[0])
#define OBPM_GET_ENDPOINT_CONFIG_RES_STATUS_SET(obpm_get_endpoint_config_res_ptr, status) ((obpm_get_endpoint_config_res_ptr)->_data[0] = (uint16)(status))
#define OBPM_GET_ENDPOINT_CONFIG_RES_ENDPOINT_ID_WORD_OFFSET (1)
#define OBPM_GET_ENDPOINT_CONFIG_RES_ENDPOINT_ID_GET(obpm_get_endpoint_config_res_ptr) ((obpm_get_endpoint_config_res_ptr)->_data[1])
#define OBPM_GET_ENDPOINT_CONFIG_RES_ENDPOINT_ID_SET(obpm_get_endpoint_config_res_ptr, endpoint_id) ((obpm_get_endpoint_config_res_ptr)->_data[1] = (uint16)(endpoint_id))
#define OBPM_GET_ENDPOINT_CONFIG_RES_KEY_IDS_VALUES_WORD_OFFSET (2)
#define OBPM_GET_ENDPOINT_CONFIG_RES_KEY_IDS_VALUES_GET(obpm_get_endpoint_config_res_ptr) ((obpm_get_endpoint_config_res_ptr)->_data[2])
#define OBPM_GET_ENDPOINT_CONFIG_RES_KEY_IDS_VALUES_SET(obpm_get_endpoint_config_res_ptr, key_ids_values) ((obpm_get_endpoint_config_res_ptr)->_data[2] = (uint16)(key_ids_values))
#define OBPM_GET_ENDPOINT_CONFIG_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_ENDPOINT_CONFIG_RES_CREATE(Status, Endpoint_ID, Key_IDs_values) \
    (uint16)(Status), \
    (uint16)(Endpoint_ID), \
    (uint16)(Key_IDs_values)
#define OBPM_GET_ENDPOINT_CONFIG_RES_PACK(obpm_get_endpoint_config_res_ptr, Status, Endpoint_ID, Key_IDs_values) \
    do { \
        (obpm_get_endpoint_config_res_ptr)->_data[0] = (uint16)((uint16)(Status)); \
        (obpm_get_endpoint_config_res_ptr)->_data[1] = (uint16)((uint16)(Endpoint_ID)); \
        (obpm_get_endpoint_config_res_ptr)->_data[2] = (uint16)((uint16)(Key_IDs_values)); \
    } while (0)

#define OBPM_GET_ENDPOINT_CONFIG_RES_MARSHALL(addr, obpm_get_endpoint_config_res_ptr) memcpy((void *)(addr), (void *)(obpm_get_endpoint_config_res_ptr), 3)
#define OBPM_GET_ENDPOINT_CONFIG_RES_UNMARSHALL(addr, obpm_get_endpoint_config_res_ptr) memcpy((void *)(obpm_get_endpoint_config_res_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OBPM_GET_ENDPOINT_TYPE_REQ

  DESCRIPTION

  MEMBERS
    Endpoint_ID - External endpoint ID as seen by host

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_GET_ENDPOINT_TYPE_REQ;

/* The following macros take OBPM_GET_ENDPOINT_TYPE_REQ *obpm_get_endpoint_type_req_ptr */
#define OBPM_GET_ENDPOINT_TYPE_REQ_ENDPOINT_ID_WORD_OFFSET (0)
#define OBPM_GET_ENDPOINT_TYPE_REQ_ENDPOINT_ID_GET(obpm_get_endpoint_type_req_ptr) ((obpm_get_endpoint_type_req_ptr)->_data[0])
#define OBPM_GET_ENDPOINT_TYPE_REQ_ENDPOINT_ID_SET(obpm_get_endpoint_type_req_ptr, endpoint_id) ((obpm_get_endpoint_type_req_ptr)->_data[0] = (uint16)(endpoint_id))
#define OBPM_GET_ENDPOINT_TYPE_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_ENDPOINT_TYPE_REQ_CREATE(Endpoint_ID) \
    (uint16)(Endpoint_ID)
#define OBPM_GET_ENDPOINT_TYPE_REQ_PACK(obpm_get_endpoint_type_req_ptr, Endpoint_ID) \
    do { \
        (obpm_get_endpoint_type_req_ptr)->_data[0] = (uint16)((uint16)(Endpoint_ID)); \
    } while (0)

#define OBPM_GET_ENDPOINT_TYPE_REQ_MARSHALL(addr, obpm_get_endpoint_type_req_ptr) memcpy((void *)(addr), (void *)(obpm_get_endpoint_type_req_ptr), 1)
#define OBPM_GET_ENDPOINT_TYPE_REQ_UNMARSHALL(addr, obpm_get_endpoint_type_req_ptr) memcpy((void *)(obpm_get_endpoint_type_req_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_GET_ENDPOINT_TYPE_RES

  DESCRIPTION

  MEMBERS
    Status        - Status code
    Endpoint_ID   - External endpoint ID as seen by host
    Endpoint_Type - Endpoint type

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OBPM_GET_ENDPOINT_TYPE_RES;

/* The following macros take OBPM_GET_ENDPOINT_TYPE_RES *obpm_get_endpoint_type_res_ptr */
#define OBPM_GET_ENDPOINT_TYPE_RES_STATUS_WORD_OFFSET (0)
#define OBPM_GET_ENDPOINT_TYPE_RES_STATUS_GET(obpm_get_endpoint_type_res_ptr) ((obpm_get_endpoint_type_res_ptr)->_data[0])
#define OBPM_GET_ENDPOINT_TYPE_RES_STATUS_SET(obpm_get_endpoint_type_res_ptr, status) ((obpm_get_endpoint_type_res_ptr)->_data[0] = (uint16)(status))
#define OBPM_GET_ENDPOINT_TYPE_RES_ENDPOINT_ID_WORD_OFFSET (1)
#define OBPM_GET_ENDPOINT_TYPE_RES_ENDPOINT_ID_GET(obpm_get_endpoint_type_res_ptr) ((obpm_get_endpoint_type_res_ptr)->_data[1])
#define OBPM_GET_ENDPOINT_TYPE_RES_ENDPOINT_ID_SET(obpm_get_endpoint_type_res_ptr, endpoint_id) ((obpm_get_endpoint_type_res_ptr)->_data[1] = (uint16)(endpoint_id))
#define OBPM_GET_ENDPOINT_TYPE_RES_ENDPOINT_TYPE_WORD_OFFSET (2)
#define OBPM_GET_ENDPOINT_TYPE_RES_ENDPOINT_TYPE_GET(obpm_get_endpoint_type_res_ptr) ((obpm_get_endpoint_type_res_ptr)->_data[2])
#define OBPM_GET_ENDPOINT_TYPE_RES_ENDPOINT_TYPE_SET(obpm_get_endpoint_type_res_ptr, endpoint_type) ((obpm_get_endpoint_type_res_ptr)->_data[2] = (uint16)(endpoint_type))
#define OBPM_GET_ENDPOINT_TYPE_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_ENDPOINT_TYPE_RES_CREATE(Status, Endpoint_ID, Endpoint_Type) \
    (uint16)(Status), \
    (uint16)(Endpoint_ID), \
    (uint16)(Endpoint_Type)
#define OBPM_GET_ENDPOINT_TYPE_RES_PACK(obpm_get_endpoint_type_res_ptr, Status, Endpoint_ID, Endpoint_Type) \
    do { \
        (obpm_get_endpoint_type_res_ptr)->_data[0] = (uint16)((uint16)(Status)); \
        (obpm_get_endpoint_type_res_ptr)->_data[1] = (uint16)((uint16)(Endpoint_ID)); \
        (obpm_get_endpoint_type_res_ptr)->_data[2] = (uint16)((uint16)(Endpoint_Type)); \
    } while (0)

#define OBPM_GET_ENDPOINT_TYPE_RES_MARSHALL(addr, obpm_get_endpoint_type_res_ptr) memcpy((void *)(addr), (void *)(obpm_get_endpoint_type_res_ptr), 3)
#define OBPM_GET_ENDPOINT_TYPE_RES_UNMARSHALL(addr, obpm_get_endpoint_type_res_ptr) memcpy((void *)(obpm_get_endpoint_type_res_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OBPM_GET_INSTALLED_CAPID_LIST_REQ

  DESCRIPTION
    Requests a list of capabilities that are currently installed on the
    device and can be instantiated. This may include capabilities that have
    been downloaded and are not part of the original image. It is highly
    likely that this includes capabilities that have not been instantiated at
    the current time.

  MEMBERS
    start_index - Index of the capability to start reporting from in uint16s.
                  This is done for consistency with other protocols.

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_GET_INSTALLED_CAPID_LIST_REQ;

/* The following macros take OBPM_GET_INSTALLED_CAPID_LIST_REQ *obpm_get_installed_capid_list_req_ptr */
#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_START_INDEX_WORD_OFFSET (0)
#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_START_INDEX_GET(obpm_get_installed_capid_list_req_ptr) ((obpm_get_installed_capid_list_req_ptr)->_data[0])
#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_START_INDEX_SET(obpm_get_installed_capid_list_req_ptr, start_index) ((obpm_get_installed_capid_list_req_ptr)->_data[0] = (uint16)(start_index))
#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_CREATE(start_index) \
    (uint16)(start_index)
#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_PACK(obpm_get_installed_capid_list_req_ptr, start_index) \
    do { \
        (obpm_get_installed_capid_list_req_ptr)->_data[0] = (uint16)((uint16)(start_index)); \
    } while (0)

#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_MARSHALL(addr, obpm_get_installed_capid_list_req_ptr) memcpy((void *)(addr), (void *)(obpm_get_installed_capid_list_req_ptr), 1)
#define OBPM_GET_INSTALLED_CAPID_LIST_REQ_UNMARSHALL(addr, obpm_get_installed_capid_list_req_ptr) memcpy((void *)(obpm_get_installed_capid_list_req_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_GET_INSTALLED_CAPID_LIST_RES

  DESCRIPTION
    Response message.

  MEMBERS
    total_num_capabilities - Length of the list of capabilities, in uint16s
    num_capabilities       - Number of capabilities present in this message, in
                             uint16s
    capabilities           - List of capability IDs currently supported, one per
                             uint16.

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OBPM_GET_INSTALLED_CAPID_LIST_RES;

/* The following macros take OBPM_GET_INSTALLED_CAPID_LIST_RES *obpm_get_installed_capid_list_res_ptr */
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_TOTAL_NUM_CAPABILITIES_WORD_OFFSET (0)
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_TOTAL_NUM_CAPABILITIES_GET(obpm_get_installed_capid_list_res_ptr) ((obpm_get_installed_capid_list_res_ptr)->_data[0])
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_TOTAL_NUM_CAPABILITIES_SET(obpm_get_installed_capid_list_res_ptr, total_num_capabilities) ((obpm_get_installed_capid_list_res_ptr)->_data[0] = (uint16)(total_num_capabilities))
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_NUM_CAPABILITIES_WORD_OFFSET (1)
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_NUM_CAPABILITIES_GET(obpm_get_installed_capid_list_res_ptr) ((obpm_get_installed_capid_list_res_ptr)->_data[1])
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_NUM_CAPABILITIES_SET(obpm_get_installed_capid_list_res_ptr, num_capabilities) ((obpm_get_installed_capid_list_res_ptr)->_data[1] = (uint16)(num_capabilities))
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_CAPABILITIES_WORD_OFFSET (2)
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_CAPABILITIES_GET(obpm_get_installed_capid_list_res_ptr) ((obpm_get_installed_capid_list_res_ptr)->_data[2])
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_CAPABILITIES_SET(obpm_get_installed_capid_list_res_ptr, capabilities) ((obpm_get_installed_capid_list_res_ptr)->_data[2] = (uint16)(capabilities))
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_CREATE(total_num_capabilities, num_capabilities, capabilities) \
    (uint16)(total_num_capabilities), \
    (uint16)(num_capabilities), \
    (uint16)(capabilities)
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_PACK(obpm_get_installed_capid_list_res_ptr, total_num_capabilities, num_capabilities, capabilities) \
    do { \
        (obpm_get_installed_capid_list_res_ptr)->_data[0] = (uint16)((uint16)(total_num_capabilities)); \
        (obpm_get_installed_capid_list_res_ptr)->_data[1] = (uint16)((uint16)(num_capabilities)); \
        (obpm_get_installed_capid_list_res_ptr)->_data[2] = (uint16)((uint16)(capabilities)); \
    } while (0)

#define OBPM_GET_INSTALLED_CAPID_LIST_RES_MARSHALL(addr, obpm_get_installed_capid_list_res_ptr) memcpy((void *)(addr), (void *)(obpm_get_installed_capid_list_res_ptr), 3)
#define OBPM_GET_INSTALLED_CAPID_LIST_RES_UNMARSHALL(addr, obpm_get_installed_capid_list_res_ptr) memcpy((void *)(obpm_get_installed_capid_list_res_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OBPM_GET_MEM_USAGE_RES

  DESCRIPTION
    Response message containing informations about memory usage.

  MEMBERS
    HEAP_SIZE    - The heap size in words.
    HEAP_CURRENT - Current heap usage in words.
    HEAP_MIN     - Minimum available heap in words.
    POOL_SIZE    - The pool memory size in words.
    POOL_CURRENT - Current pool memory usage in words.
    POOL_MIN     - Minimum available pool memory in words.

*******************************************************************************/
typedef struct
{
    uint16 _data[12];
} OBPM_GET_MEM_USAGE_RES;

/* The following macros take OBPM_GET_MEM_USAGE_RES *obpm_get_mem_usage_res_ptr */
#define OBPM_GET_MEM_USAGE_RES_HEAP_SIZE_WORD_OFFSET (0)
#define OBPM_GET_MEM_USAGE_RES_HEAP_SIZE_GET(obpm_get_mem_usage_res_ptr)  \
    (((uint32)((obpm_get_mem_usage_res_ptr)->_data[0]) | \
      ((uint32)((obpm_get_mem_usage_res_ptr)->_data[1]) << 16)))
#define OBPM_GET_MEM_USAGE_RES_HEAP_SIZE_SET(obpm_get_mem_usage_res_ptr, heap_size) do { \
        (obpm_get_mem_usage_res_ptr)->_data[0] = (uint16)((heap_size) & 0xffff); \
        (obpm_get_mem_usage_res_ptr)->_data[1] = (uint16)((heap_size) >> 16); } while (0)
#define OBPM_GET_MEM_USAGE_RES_HEAP_CURRENT_WORD_OFFSET (2)
#define OBPM_GET_MEM_USAGE_RES_HEAP_CURRENT_GET(obpm_get_mem_usage_res_ptr)  \
    (((uint32)((obpm_get_mem_usage_res_ptr)->_data[2]) | \
      ((uint32)((obpm_get_mem_usage_res_ptr)->_data[3]) << 16)))
#define OBPM_GET_MEM_USAGE_RES_HEAP_CURRENT_SET(obpm_get_mem_usage_res_ptr, heap_current) do { \
        (obpm_get_mem_usage_res_ptr)->_data[2] = (uint16)((heap_current) & 0xffff); \
        (obpm_get_mem_usage_res_ptr)->_data[3] = (uint16)((heap_current) >> 16); } while (0)
#define OBPM_GET_MEM_USAGE_RES_HEAP_MIN_WORD_OFFSET (4)
#define OBPM_GET_MEM_USAGE_RES_HEAP_MIN_GET(obpm_get_mem_usage_res_ptr)  \
    (((uint32)((obpm_get_mem_usage_res_ptr)->_data[4]) | \
      ((uint32)((obpm_get_mem_usage_res_ptr)->_data[5]) << 16)))
#define OBPM_GET_MEM_USAGE_RES_HEAP_MIN_SET(obpm_get_mem_usage_res_ptr, heap_min) do { \
        (obpm_get_mem_usage_res_ptr)->_data[4] = (uint16)((heap_min) & 0xffff); \
        (obpm_get_mem_usage_res_ptr)->_data[5] = (uint16)((heap_min) >> 16); } while (0)
#define OBPM_GET_MEM_USAGE_RES_POOL_SIZE_WORD_OFFSET (6)
#define OBPM_GET_MEM_USAGE_RES_POOL_SIZE_GET(obpm_get_mem_usage_res_ptr)  \
    (((uint32)((obpm_get_mem_usage_res_ptr)->_data[6]) | \
      ((uint32)((obpm_get_mem_usage_res_ptr)->_data[7]) << 16)))
#define OBPM_GET_MEM_USAGE_RES_POOL_SIZE_SET(obpm_get_mem_usage_res_ptr, pool_size) do { \
        (obpm_get_mem_usage_res_ptr)->_data[6] = (uint16)((pool_size) & 0xffff); \
        (obpm_get_mem_usage_res_ptr)->_data[7] = (uint16)((pool_size) >> 16); } while (0)
#define OBPM_GET_MEM_USAGE_RES_POOL_CURRENT_WORD_OFFSET (8)
#define OBPM_GET_MEM_USAGE_RES_POOL_CURRENT_GET(obpm_get_mem_usage_res_ptr)  \
    (((uint32)((obpm_get_mem_usage_res_ptr)->_data[8]) | \
      ((uint32)((obpm_get_mem_usage_res_ptr)->_data[8 + 1]) << 16)))
#define OBPM_GET_MEM_USAGE_RES_POOL_CURRENT_SET(obpm_get_mem_usage_res_ptr, pool_current) do { \
        (obpm_get_mem_usage_res_ptr)->_data[8] = (uint16)((pool_current) & 0xffff); \
        (obpm_get_mem_usage_res_ptr)->_data[8 + 1] = (uint16)((pool_current) >> 16); } while (0)
#define OBPM_GET_MEM_USAGE_RES_POOL_MIN_WORD_OFFSET (10)
#define OBPM_GET_MEM_USAGE_RES_POOL_MIN_GET(obpm_get_mem_usage_res_ptr)  \
    (((uint32)((obpm_get_mem_usage_res_ptr)->_data[10]) | \
      ((uint32)((obpm_get_mem_usage_res_ptr)->_data[11]) << 16)))
#define OBPM_GET_MEM_USAGE_RES_POOL_MIN_SET(obpm_get_mem_usage_res_ptr, pool_min) do { \
        (obpm_get_mem_usage_res_ptr)->_data[10] = (uint16)((pool_min) & 0xffff); \
        (obpm_get_mem_usage_res_ptr)->_data[11] = (uint16)((pool_min) >> 16); } while (0)
#define OBPM_GET_MEM_USAGE_RES_WORD_SIZE (12)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_MEM_USAGE_RES_CREATE(HEAP_SIZE, HEAP_CURRENT, HEAP_MIN, POOL_SIZE, POOL_CURRENT, POOL_MIN) \
    (uint16)((HEAP_SIZE) & 0xffff), \
    (uint16)((HEAP_SIZE) >> 16), \
    (uint16)((HEAP_CURRENT) & 0xffff), \
    (uint16)((HEAP_CURRENT) >> 16), \
    (uint16)((HEAP_MIN) & 0xffff), \
    (uint16)((HEAP_MIN) >> 16), \
    (uint16)((POOL_SIZE) & 0xffff), \
    (uint16)((POOL_SIZE) >> 16), \
    (uint16)((POOL_CURRENT) & 0xffff), \
    (uint16)((POOL_CURRENT) >> 16), \
    (uint16)((POOL_MIN) & 0xffff), \
    (uint16)((POOL_MIN) >> 16)
#define OBPM_GET_MEM_USAGE_RES_PACK(obpm_get_mem_usage_res_ptr, HEAP_SIZE, HEAP_CURRENT, HEAP_MIN, POOL_SIZE, POOL_CURRENT, POOL_MIN) \
    do { \
        (obpm_get_mem_usage_res_ptr)->_data[0] = (uint16)((uint16)((HEAP_SIZE) & 0xffff)); \
        (obpm_get_mem_usage_res_ptr)->_data[1] = (uint16)(((HEAP_SIZE) >> 16)); \
        (obpm_get_mem_usage_res_ptr)->_data[2] = (uint16)((uint16)((HEAP_CURRENT) & 0xffff)); \
        (obpm_get_mem_usage_res_ptr)->_data[3] = (uint16)(((HEAP_CURRENT) >> 16)); \
        (obpm_get_mem_usage_res_ptr)->_data[4] = (uint16)((uint16)((HEAP_MIN) & 0xffff)); \
        (obpm_get_mem_usage_res_ptr)->_data[5] = (uint16)(((HEAP_MIN) >> 16)); \
        (obpm_get_mem_usage_res_ptr)->_data[6] = (uint16)((uint16)((POOL_SIZE) & 0xffff)); \
        (obpm_get_mem_usage_res_ptr)->_data[7] = (uint16)(((POOL_SIZE) >> 16)); \
        (obpm_get_mem_usage_res_ptr)->_data[8] = (uint16)((uint16)((POOL_CURRENT) & 0xffff)); \
        (obpm_get_mem_usage_res_ptr)->_data[8 + 1] = (uint16)(((POOL_CURRENT) >> 16)); \
        (obpm_get_mem_usage_res_ptr)->_data[10] = (uint16)((uint16)((POOL_MIN) & 0xffff)); \
        (obpm_get_mem_usage_res_ptr)->_data[11] = (uint16)(((POOL_MIN) >> 16)); \
    } while (0)

#define OBPM_GET_MEM_USAGE_RES_MARSHALL(addr, obpm_get_mem_usage_res_ptr) memcpy((void *)(addr), (void *)(obpm_get_mem_usage_res_ptr), 12)
#define OBPM_GET_MEM_USAGE_RES_UNMARSHALL(addr, obpm_get_mem_usage_res_ptr) memcpy((void *)(obpm_get_mem_usage_res_ptr), (void *)(addr), 12)


/*******************************************************************************

  NAME
    OBPM_GET_MIPS_USAGE_REQ

  DESCRIPTION
    Request message for getting memory usage of an operator.

  MEMBERS
    OP_IDS - Operator Ids to profile.

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_GET_MIPS_USAGE_REQ;

/* The following macros take OBPM_GET_MIPS_USAGE_REQ *obpm_get_mips_usage_req_ptr */
#define OBPM_GET_MIPS_USAGE_REQ_OP_IDS_WORD_OFFSET (0)
#define OBPM_GET_MIPS_USAGE_REQ_OP_IDS_GET(obpm_get_mips_usage_req_ptr) ((obpm_get_mips_usage_req_ptr)->_data[0])
#define OBPM_GET_MIPS_USAGE_REQ_OP_IDS_SET(obpm_get_mips_usage_req_ptr, op_ids) ((obpm_get_mips_usage_req_ptr)->_data[0] = (uint16)(op_ids))
#define OBPM_GET_MIPS_USAGE_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_MIPS_USAGE_REQ_CREATE(OP_IDS) \
    (uint16)(OP_IDS)
#define OBPM_GET_MIPS_USAGE_REQ_PACK(obpm_get_mips_usage_req_ptr, OP_IDS) \
    do { \
        (obpm_get_mips_usage_req_ptr)->_data[0] = (uint16)((uint16)(OP_IDS)); \
    } while (0)

#define OBPM_GET_MIPS_USAGE_REQ_MARSHALL(addr, obpm_get_mips_usage_req_ptr) memcpy((void *)(addr), (void *)(obpm_get_mips_usage_req_ptr), 1)
#define OBPM_GET_MIPS_USAGE_REQ_UNMARSHALL(addr, obpm_get_mips_usage_req_ptr) memcpy((void *)(obpm_get_mips_usage_req_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_GET_MIPS_USAGE_RES

  DESCRIPTION
    Response message holding the mips usage of an operator.

  MEMBERS
    Status        - Returns STATUS_OK if the operator exist.
    SLEEP         - Sleep in percentage.
    OP_USAGE_LIST - Operator id echoed followed by the mips usage in thousandths
                    of the operator. -1 of the operator does not exist.

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OBPM_GET_MIPS_USAGE_RES;

/* The following macros take OBPM_GET_MIPS_USAGE_RES *obpm_get_mips_usage_res_ptr */
#define OBPM_GET_MIPS_USAGE_RES_STATUS_WORD_OFFSET (0)
#define OBPM_GET_MIPS_USAGE_RES_STATUS_GET(obpm_get_mips_usage_res_ptr) ((obpm_get_mips_usage_res_ptr)->_data[0])
#define OBPM_GET_MIPS_USAGE_RES_STATUS_SET(obpm_get_mips_usage_res_ptr, status) ((obpm_get_mips_usage_res_ptr)->_data[0] = (uint16)(status))
#define OBPM_GET_MIPS_USAGE_RES_SLEEP_WORD_OFFSET (1)
#define OBPM_GET_MIPS_USAGE_RES_SLEEP_GET(obpm_get_mips_usage_res_ptr) ((obpm_get_mips_usage_res_ptr)->_data[1])
#define OBPM_GET_MIPS_USAGE_RES_SLEEP_SET(obpm_get_mips_usage_res_ptr, sleep) ((obpm_get_mips_usage_res_ptr)->_data[1] = (uint16)(sleep))
#define OBPM_GET_MIPS_USAGE_RES_OP_USAGE_LIST_WORD_OFFSET (2)
#define OBPM_GET_MIPS_USAGE_RES_OP_USAGE_LIST_GET(obpm_get_mips_usage_res_ptr) ((obpm_get_mips_usage_res_ptr)->_data[2])
#define OBPM_GET_MIPS_USAGE_RES_OP_USAGE_LIST_SET(obpm_get_mips_usage_res_ptr, op_usage_list) ((obpm_get_mips_usage_res_ptr)->_data[2] = (uint16)(op_usage_list))
#define OBPM_GET_MIPS_USAGE_RES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_MIPS_USAGE_RES_CREATE(Status, SLEEP, OP_USAGE_LIST) \
    (uint16)(Status), \
    (uint16)(SLEEP), \
    (uint16)(OP_USAGE_LIST)
#define OBPM_GET_MIPS_USAGE_RES_PACK(obpm_get_mips_usage_res_ptr, Status, SLEEP, OP_USAGE_LIST) \
    do { \
        (obpm_get_mips_usage_res_ptr)->_data[0] = (uint16)((uint16)(Status)); \
        (obpm_get_mips_usage_res_ptr)->_data[1] = (uint16)((uint16)(SLEEP)); \
        (obpm_get_mips_usage_res_ptr)->_data[2] = (uint16)((uint16)(OP_USAGE_LIST)); \
    } while (0)

#define OBPM_GET_MIPS_USAGE_RES_MARSHALL(addr, obpm_get_mips_usage_res_ptr) memcpy((void *)(addr), (void *)(obpm_get_mips_usage_res_ptr), 3)
#define OBPM_GET_MIPS_USAGE_RES_UNMARSHALL(addr, obpm_get_mips_usage_res_ptr) memcpy((void *)(obpm_get_mips_usage_res_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OBPM_GET_SYSTEM_STREAM_RATE_RES

  DESCRIPTION

  MEMBERS
    Sample_rate - System sampling rate

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_GET_SYSTEM_STREAM_RATE_RES;

/* The following macros take OBPM_GET_SYSTEM_STREAM_RATE_RES *obpm_get_system_stream_rate_res_ptr */
#define OBPM_GET_SYSTEM_STREAM_RATE_RES_SAMPLE_RATE_WORD_OFFSET (0)
#define OBPM_GET_SYSTEM_STREAM_RATE_RES_SAMPLE_RATE_GET(obpm_get_system_stream_rate_res_ptr)  \
    (((uint32)((obpm_get_system_stream_rate_res_ptr)->_data[0]) | \
      ((uint32)((obpm_get_system_stream_rate_res_ptr)->_data[1]) << 16)))
#define OBPM_GET_SYSTEM_STREAM_RATE_RES_SAMPLE_RATE_SET(obpm_get_system_stream_rate_res_ptr, sample_rate) do { \
        (obpm_get_system_stream_rate_res_ptr)->_data[0] = (uint16)((sample_rate) & 0xffff); \
        (obpm_get_system_stream_rate_res_ptr)->_data[1] = (uint16)((sample_rate) >> 16); } while (0)
#define OBPM_GET_SYSTEM_STREAM_RATE_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_GET_SYSTEM_STREAM_RATE_RES_CREATE(Sample_rate) \
    (uint16)((Sample_rate) & 0xffff), \
    (uint16)((Sample_rate) >> 16)
#define OBPM_GET_SYSTEM_STREAM_RATE_RES_PACK(obpm_get_system_stream_rate_res_ptr, Sample_rate) \
    do { \
        (obpm_get_system_stream_rate_res_ptr)->_data[0] = (uint16)((uint16)((Sample_rate) & 0xffff)); \
        (obpm_get_system_stream_rate_res_ptr)->_data[1] = (uint16)(((Sample_rate) >> 16)); \
    } while (0)

#define OBPM_GET_SYSTEM_STREAM_RATE_RES_MARSHALL(addr, obpm_get_system_stream_rate_res_ptr) memcpy((void *)(addr), (void *)(obpm_get_system_stream_rate_res_ptr), 2)
#define OBPM_GET_SYSTEM_STREAM_RATE_RES_UNMARSHALL(addr, obpm_get_system_stream_rate_res_ptr) memcpy((void *)(obpm_get_system_stream_rate_res_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_LICENCE_STATUS_REQ

  DESCRIPTION
    Request message for getting licence status of a licenced component. A7DA
    uses 4 product ID words only aligned to variable scheme, Crescendo uses
    variable length.

  MEMBERS
    prod_id - Product ID words (variable number, 4 words for A7DA until variable
              scheme taken into use)

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_LICENCE_STATUS_REQ;

/* The following macros take OBPM_LICENCE_STATUS_REQ *obpm_licence_status_req_ptr */
#define OBPM_LICENCE_STATUS_REQ_PROD_ID_WORD_OFFSET (0)
#define OBPM_LICENCE_STATUS_REQ_PROD_ID_GET(obpm_licence_status_req_ptr) ((obpm_licence_status_req_ptr)->_data[0])
#define OBPM_LICENCE_STATUS_REQ_PROD_ID_SET(obpm_licence_status_req_ptr, prod_id) ((obpm_licence_status_req_ptr)->_data[0] = (uint16)(prod_id))
#define OBPM_LICENCE_STATUS_REQ_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_LICENCE_STATUS_REQ_CREATE(prod_id) \
    (uint16)(prod_id)
#define OBPM_LICENCE_STATUS_REQ_PACK(obpm_licence_status_req_ptr, prod_id) \
    do { \
        (obpm_licence_status_req_ptr)->_data[0] = (uint16)((uint16)(prod_id)); \
    } while (0)

#define OBPM_LICENCE_STATUS_REQ_MARSHALL(addr, obpm_licence_status_req_ptr) memcpy((void *)(addr), (void *)(obpm_licence_status_req_ptr), 1)
#define OBPM_LICENCE_STATUS_REQ_UNMARSHALL(addr, obpm_licence_status_req_ptr) memcpy((void *)(obpm_licence_status_req_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_LICENCE_STATUS_RES

  DESCRIPTION
    Response message with licence status of the component. NOTE: A7DA only
    uses 4 words of product ID until aligned to variable scheme, Crescendo
    uses variable length.

  MEMBERS
    Licence_Status - The licence status of the component, values are defined in
                     licencing.xml, LICENCING_STATUS enum.
    prod_id        - Product ID words (variable number, 4 words for A7DA until
                     variable scheme taken into use)

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_LICENCE_STATUS_RES;

/* The following macros take OBPM_LICENCE_STATUS_RES *obpm_licence_status_res_ptr */
#define OBPM_LICENCE_STATUS_RES_LICENCE_STATUS_WORD_OFFSET (0)
#define OBPM_LICENCE_STATUS_RES_LICENCE_STATUS_GET(obpm_licence_status_res_ptr) ((obpm_licence_status_res_ptr)->_data[0])
#define OBPM_LICENCE_STATUS_RES_LICENCE_STATUS_SET(obpm_licence_status_res_ptr, licence_status) ((obpm_licence_status_res_ptr)->_data[0] = (uint16)(licence_status))
#define OBPM_LICENCE_STATUS_RES_PROD_ID_WORD_OFFSET (1)
#define OBPM_LICENCE_STATUS_RES_PROD_ID_GET(obpm_licence_status_res_ptr) ((obpm_licence_status_res_ptr)->_data[1])
#define OBPM_LICENCE_STATUS_RES_PROD_ID_SET(obpm_licence_status_res_ptr, prod_id) ((obpm_licence_status_res_ptr)->_data[1] = (uint16)(prod_id))
#define OBPM_LICENCE_STATUS_RES_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_LICENCE_STATUS_RES_CREATE(Licence_Status, prod_id) \
    (uint16)(Licence_Status), \
    (uint16)(prod_id)
#define OBPM_LICENCE_STATUS_RES_PACK(obpm_licence_status_res_ptr, Licence_Status, prod_id) \
    do { \
        (obpm_licence_status_res_ptr)->_data[0] = (uint16)((uint16)(Licence_Status)); \
        (obpm_licence_status_res_ptr)->_data[1] = (uint16)((uint16)(prod_id)); \
    } while (0)

#define OBPM_LICENCE_STATUS_RES_MARSHALL(addr, obpm_licence_status_res_ptr) memcpy((void *)(addr), (void *)(obpm_licence_status_res_ptr), 2)
#define OBPM_LICENCE_STATUS_RES_UNMARSHALL(addr, obpm_licence_status_res_ptr) memcpy((void *)(obpm_licence_status_res_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_OPID_ENTRY

  DESCRIPTION
    One entry of OBPM_OPID_LIST_RES

  MEMBERS
    OPID   - ID of the operator
    Cap_ID - Capability ID of the operator

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_OPID_ENTRY;

/* The following macros take OBPM_OPID_ENTRY *obpm_opid_entry_ptr */
#define OBPM_OPID_ENTRY_OPID_WORD_OFFSET (0)
#define OBPM_OPID_ENTRY_OPID_GET(obpm_opid_entry_ptr) ((obpm_opid_entry_ptr)->_data[0])
#define OBPM_OPID_ENTRY_OPID_SET(obpm_opid_entry_ptr, opid) ((obpm_opid_entry_ptr)->_data[0] = (uint16)(opid))
#define OBPM_OPID_ENTRY_CAP_ID_WORD_OFFSET (1)
#define OBPM_OPID_ENTRY_CAP_ID_GET(obpm_opid_entry_ptr) ((obpm_opid_entry_ptr)->_data[1])
#define OBPM_OPID_ENTRY_CAP_ID_SET(obpm_opid_entry_ptr, cap_id) ((obpm_opid_entry_ptr)->_data[1] = (uint16)(cap_id))
#define OBPM_OPID_ENTRY_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_OPID_ENTRY_CREATE(OPID, Cap_ID) \
    (uint16)(OPID), \
    (uint16)(Cap_ID)
#define OBPM_OPID_ENTRY_PACK(obpm_opid_entry_ptr, OPID, Cap_ID) \
    do { \
        (obpm_opid_entry_ptr)->_data[0] = (uint16)((uint16)(OPID)); \
        (obpm_opid_entry_ptr)->_data[1] = (uint16)((uint16)(Cap_ID)); \
    } while (0)

#define OBPM_OPID_ENTRY_MARSHALL(addr, obpm_opid_entry_ptr) memcpy((void *)(addr), (void *)(obpm_opid_entry_ptr), 2)
#define OBPM_OPID_ENTRY_UNMARSHALL(addr, obpm_opid_entry_ptr) memcpy((void *)(obpm_opid_entry_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_OPID_HEADER

  DESCRIPTION

  MEMBERS
    total_num_operators - Total number of operators present in the system, in
                          uint16s
    num_operators       - Number of operators present in this message, in
                          uint16s

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_OPID_HEADER;

/* The following macros take OBPM_OPID_HEADER *obpm_opid_header_ptr */
#define OBPM_OPID_HEADER_TOTAL_NUM_OPERATORS_WORD_OFFSET (0)
#define OBPM_OPID_HEADER_TOTAL_NUM_OPERATORS_GET(obpm_opid_header_ptr) ((obpm_opid_header_ptr)->_data[0])
#define OBPM_OPID_HEADER_TOTAL_NUM_OPERATORS_SET(obpm_opid_header_ptr, total_num_operators) ((obpm_opid_header_ptr)->_data[0] = (uint16)(total_num_operators))
#define OBPM_OPID_HEADER_NUM_OPERATORS_WORD_OFFSET (1)
#define OBPM_OPID_HEADER_NUM_OPERATORS_GET(obpm_opid_header_ptr) ((obpm_opid_header_ptr)->_data[1])
#define OBPM_OPID_HEADER_NUM_OPERATORS_SET(obpm_opid_header_ptr, num_operators) ((obpm_opid_header_ptr)->_data[1] = (uint16)(num_operators))
#define OBPM_OPID_HEADER_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_OPID_HEADER_CREATE(total_num_operators, num_operators) \
    (uint16)(total_num_operators), \
    (uint16)(num_operators)
#define OBPM_OPID_HEADER_PACK(obpm_opid_header_ptr, total_num_operators, num_operators) \
    do { \
        (obpm_opid_header_ptr)->_data[0] = (uint16)((uint16)(total_num_operators)); \
        (obpm_opid_header_ptr)->_data[1] = (uint16)((uint16)(num_operators)); \
    } while (0)

#define OBPM_OPID_HEADER_MARSHALL(addr, obpm_opid_header_ptr) memcpy((void *)(addr), (void *)(obpm_opid_header_ptr), 2)
#define OBPM_OPID_HEADER_UNMARSHALL(addr, obpm_opid_header_ptr) memcpy((void *)(obpm_opid_header_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_OPID_LIST_REQ

  DESCRIPTION

  MEMBERS
    Cap_ID      - Capability ID, zero means all
    start_index - Number of results to skip

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_OPID_LIST_REQ;

/* The following macros take OBPM_OPID_LIST_REQ *obpm_opid_list_req_ptr */
#define OBPM_OPID_LIST_REQ_CAP_ID_WORD_OFFSET (0)
#define OBPM_OPID_LIST_REQ_CAP_ID_GET(obpm_opid_list_req_ptr) ((obpm_opid_list_req_ptr)->_data[0])
#define OBPM_OPID_LIST_REQ_CAP_ID_SET(obpm_opid_list_req_ptr, cap_id) ((obpm_opid_list_req_ptr)->_data[0] = (uint16)(cap_id))
#define OBPM_OPID_LIST_REQ_START_INDEX_WORD_OFFSET (1)
#define OBPM_OPID_LIST_REQ_START_INDEX_GET(obpm_opid_list_req_ptr) ((obpm_opid_list_req_ptr)->_data[1])
#define OBPM_OPID_LIST_REQ_START_INDEX_SET(obpm_opid_list_req_ptr, start_index) ((obpm_opid_list_req_ptr)->_data[1] = (uint16)(start_index))
#define OBPM_OPID_LIST_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_OPID_LIST_REQ_CREATE(Cap_ID, start_index) \
    (uint16)(Cap_ID), \
    (uint16)(start_index)
#define OBPM_OPID_LIST_REQ_PACK(obpm_opid_list_req_ptr, Cap_ID, start_index) \
    do { \
        (obpm_opid_list_req_ptr)->_data[0] = (uint16)((uint16)(Cap_ID)); \
        (obpm_opid_list_req_ptr)->_data[1] = (uint16)((uint16)(start_index)); \
    } while (0)

#define OBPM_OPID_LIST_REQ_MARSHALL(addr, obpm_opid_list_req_ptr) memcpy((void *)(addr), (void *)(obpm_opid_list_req_ptr), 2)
#define OBPM_OPID_LIST_REQ_UNMARSHALL(addr, obpm_opid_list_req_ptr) memcpy((void *)(obpm_opid_list_req_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_OPID_LIST_RES

  DESCRIPTION

  MEMBERS
    header  - List of capability IDs currently supported, one per uint16.
    payload - List of capability IDs currently supported, one per uint16.

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OBPM_OPID_LIST_RES;

/* The following macros take OBPM_OPID_LIST_RES *obpm_opid_list_res_ptr */
#define OBPM_OPID_LIST_RES_HEADER_WORD_OFFSET (0)
#define OBPM_OPID_LIST_RES_HEADER_GET(obpm_opid_list_res_ptr, header_ptr) do {  \
        (header_ptr)->_data[0] = (obpm_opid_list_res_ptr)->_data[0]; \
        (header_ptr)->_data[1] = (obpm_opid_list_res_ptr)->_data[1]; } while (0)
#define OBPM_OPID_LIST_RES_HEADER_SET(obpm_opid_list_res_ptr, header_ptr) do {  \
        (obpm_opid_list_res_ptr)->_data[0] = (header_ptr)->_data[0]; \
        (obpm_opid_list_res_ptr)->_data[1] = (header_ptr)->_data[1]; } while (0)
#define OBPM_OPID_LIST_RES_PAYLOAD_WORD_OFFSET (2)
#define OBPM_OPID_LIST_RES_PAYLOAD_GET(obpm_opid_list_res_ptr, payload_ptr) do {  \
        (payload_ptr)->_data[0] = (obpm_opid_list_res_ptr)->_data[2]; \
        (payload_ptr)->_data[1] = (obpm_opid_list_res_ptr)->_data[3]; } while (0)
#define OBPM_OPID_LIST_RES_PAYLOAD_SET(obpm_opid_list_res_ptr, payload_ptr) do {  \
        (obpm_opid_list_res_ptr)->_data[2] = (payload_ptr)->_data[0]; \
        (obpm_opid_list_res_ptr)->_data[3] = (payload_ptr)->_data[1]; } while (0)
#define OBPM_OPID_LIST_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OBPM_OPID_LIST_RES_CREATE(header, payload) \
    (uint16), \
    (uint16)
#define OBPM_OPID_LIST_RES_PACK(obpm_opid_list_res_ptr, header_ptr, payload_ptr) \
    do { \
        (obpm_opid_list_res_ptr)->_data[0] = (uint16)((header_ptr)->_data[0]); \
        (obpm_opid_list_res_ptr)->_data[1] = (uint16)((header_ptr)->_data[1]); \
        (obpm_opid_list_res_ptr)->_data[2] = (uint16)((payload_ptr)->_data[0]); \
        (obpm_opid_list_res_ptr)->_data[3] = (uint16)((payload_ptr)->_data[1]); \
    } while (0)

#define OBPM_OPID_LIST_RES_MARSHALL(addr, obpm_opid_list_res_ptr) memcpy((void *)(addr), (void *)(obpm_opid_list_res_ptr), 4)
#define OBPM_OPID_LIST_RES_UNMARSHALL(addr, obpm_opid_list_res_ptr) memcpy((void *)(obpm_opid_list_res_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    OBPM_OPMSG_REQ

  DESCRIPTION

  MEMBERS
    OPID     - Kymera operator ID the message is sent to
    OpMsg_ID - Echoed operator message ID
    payload  - Arguments

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OBPM_OPMSG_REQ;

/* The following macros take OBPM_OPMSG_REQ *obpm_opmsg_req_ptr */
#define OBPM_OPMSG_REQ_OPID_WORD_OFFSET (0)
#define OBPM_OPMSG_REQ_OPID_GET(obpm_opmsg_req_ptr) ((obpm_opmsg_req_ptr)->_data[0])
#define OBPM_OPMSG_REQ_OPID_SET(obpm_opmsg_req_ptr, opid) ((obpm_opmsg_req_ptr)->_data[0] = (uint16)(opid))
#define OBPM_OPMSG_REQ_OPMSG_ID_WORD_OFFSET (1)
#define OBPM_OPMSG_REQ_OPMSG_ID_GET(obpm_opmsg_req_ptr) ((obpm_opmsg_req_ptr)->_data[1])
#define OBPM_OPMSG_REQ_OPMSG_ID_SET(obpm_opmsg_req_ptr, opmsg_id) ((obpm_opmsg_req_ptr)->_data[1] = (uint16)(opmsg_id))
#define OBPM_OPMSG_REQ_PAYLOAD_WORD_OFFSET (2)
#define OBPM_OPMSG_REQ_PAYLOAD_GET(obpm_opmsg_req_ptr) ((obpm_opmsg_req_ptr)->_data[2])
#define OBPM_OPMSG_REQ_PAYLOAD_SET(obpm_opmsg_req_ptr, payload) ((obpm_opmsg_req_ptr)->_data[2] = (uint16)(payload))
#define OBPM_OPMSG_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OBPM_OPMSG_REQ_CREATE(OPID, OpMsg_ID, payload) \
    (uint16)(OPID), \
    (uint16)(OpMsg_ID), \
    (uint16)(payload)
#define OBPM_OPMSG_REQ_PACK(obpm_opmsg_req_ptr, OPID, OpMsg_ID, payload) \
    do { \
        (obpm_opmsg_req_ptr)->_data[0] = (uint16)((uint16)(OPID)); \
        (obpm_opmsg_req_ptr)->_data[1] = (uint16)((uint16)(OpMsg_ID)); \
        (obpm_opmsg_req_ptr)->_data[2] = (uint16)((uint16)(payload)); \
    } while (0)

#define OBPM_OPMSG_REQ_MARSHALL(addr, obpm_opmsg_req_ptr) memcpy((void *)(addr), (void *)(obpm_opmsg_req_ptr), 3)
#define OBPM_OPMSG_REQ_UNMARSHALL(addr, obpm_opmsg_req_ptr) memcpy((void *)(obpm_opmsg_req_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OBPM_OPMSG_RES

  DESCRIPTION

  MEMBERS
    OPID     - Kymera operator ID the message is sent to
    status   - Status code
    OpMsg_ID - Echoed operator message ID
    payload  - Arguments

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OBPM_OPMSG_RES;

/* The following macros take OBPM_OPMSG_RES *obpm_opmsg_res_ptr */
#define OBPM_OPMSG_RES_OPID_WORD_OFFSET (0)
#define OBPM_OPMSG_RES_OPID_GET(obpm_opmsg_res_ptr) ((obpm_opmsg_res_ptr)->_data[0])
#define OBPM_OPMSG_RES_OPID_SET(obpm_opmsg_res_ptr, opid) ((obpm_opmsg_res_ptr)->_data[0] = (uint16)(opid))
#define OBPM_OPMSG_RES_STATUS_WORD_OFFSET (1)
#define OBPM_OPMSG_RES_STATUS_GET(obpm_opmsg_res_ptr) ((OBPM_STATUS_ID)(obpm_opmsg_res_ptr)->_data[1])
#define OBPM_OPMSG_RES_STATUS_SET(obpm_opmsg_res_ptr, status) ((obpm_opmsg_res_ptr)->_data[1] = (uint16)(status))
#define OBPM_OPMSG_RES_OPMSG_ID_WORD_OFFSET (2)
#define OBPM_OPMSG_RES_OPMSG_ID_GET(obpm_opmsg_res_ptr) ((obpm_opmsg_res_ptr)->_data[2])
#define OBPM_OPMSG_RES_OPMSG_ID_SET(obpm_opmsg_res_ptr, opmsg_id) ((obpm_opmsg_res_ptr)->_data[2] = (uint16)(opmsg_id))
#define OBPM_OPMSG_RES_PAYLOAD_WORD_OFFSET (3)
#define OBPM_OPMSG_RES_PAYLOAD_GET(obpm_opmsg_res_ptr) ((obpm_opmsg_res_ptr)->_data[3])
#define OBPM_OPMSG_RES_PAYLOAD_SET(obpm_opmsg_res_ptr, payload) ((obpm_opmsg_res_ptr)->_data[3] = (uint16)(payload))
#define OBPM_OPMSG_RES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OBPM_OPMSG_RES_CREATE(OPID, status, OpMsg_ID, payload) \
    (uint16)(OPID), \
    (uint16)(status), \
    (uint16)(OpMsg_ID), \
    (uint16)(payload)
#define OBPM_OPMSG_RES_PACK(obpm_opmsg_res_ptr, OPID, status, OpMsg_ID, payload) \
    do { \
        (obpm_opmsg_res_ptr)->_data[0] = (uint16)((uint16)(OPID)); \
        (obpm_opmsg_res_ptr)->_data[1] = (uint16)((uint16)(status)); \
        (obpm_opmsg_res_ptr)->_data[2] = (uint16)((uint16)(OpMsg_ID)); \
        (obpm_opmsg_res_ptr)->_data[3] = (uint16)((uint16)(payload)); \
    } while (0)

#define OBPM_OPMSG_RES_MARSHALL(addr, obpm_opmsg_res_ptr) memcpy((void *)(addr), (void *)(obpm_opmsg_res_ptr), 4)
#define OBPM_OPMSG_RES_UNMARSHALL(addr, obpm_opmsg_res_ptr) memcpy((void *)(obpm_opmsg_res_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    OBPM_READ_PS_ENTRY_REQ

  DESCRIPTION

  MEMBERS
    Cap_ID       - Capability ID (can be zero)
    UCID         - Use case ID
    Sub_block_ID - If used: differentiate between data sets (e.g. state vs.
                   params)
    PS_rank      - Intended persistence rank (see sect. 6.4))

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OBPM_READ_PS_ENTRY_REQ;

/* The following macros take OBPM_READ_PS_ENTRY_REQ *obpm_read_ps_entry_req_ptr */
#define OBPM_READ_PS_ENTRY_REQ_CAP_ID_WORD_OFFSET (0)
#define OBPM_READ_PS_ENTRY_REQ_CAP_ID_GET(obpm_read_ps_entry_req_ptr) ((obpm_read_ps_entry_req_ptr)->_data[0])
#define OBPM_READ_PS_ENTRY_REQ_CAP_ID_SET(obpm_read_ps_entry_req_ptr, cap_id) ((obpm_read_ps_entry_req_ptr)->_data[0] = (uint16)(cap_id))
#define OBPM_READ_PS_ENTRY_REQ_UCID_WORD_OFFSET (1)
#define OBPM_READ_PS_ENTRY_REQ_UCID_GET(obpm_read_ps_entry_req_ptr) ((obpm_read_ps_entry_req_ptr)->_data[1])
#define OBPM_READ_PS_ENTRY_REQ_UCID_SET(obpm_read_ps_entry_req_ptr, ucid) ((obpm_read_ps_entry_req_ptr)->_data[1] = (uint16)(ucid))
#define OBPM_READ_PS_ENTRY_REQ_SUB_BLOCK_ID_WORD_OFFSET (2)
#define OBPM_READ_PS_ENTRY_REQ_SUB_BLOCK_ID_GET(obpm_read_ps_entry_req_ptr) ((obpm_read_ps_entry_req_ptr)->_data[2])
#define OBPM_READ_PS_ENTRY_REQ_SUB_BLOCK_ID_SET(obpm_read_ps_entry_req_ptr, sub_block_id) ((obpm_read_ps_entry_req_ptr)->_data[2] = (uint16)(sub_block_id))
#define OBPM_READ_PS_ENTRY_REQ_PS_RANK_WORD_OFFSET (3)
#define OBPM_READ_PS_ENTRY_REQ_PS_RANK_GET(obpm_read_ps_entry_req_ptr) ((obpm_read_ps_entry_req_ptr)->_data[3])
#define OBPM_READ_PS_ENTRY_REQ_PS_RANK_SET(obpm_read_ps_entry_req_ptr, ps_rank) ((obpm_read_ps_entry_req_ptr)->_data[3] = (uint16)(ps_rank))
#define OBPM_READ_PS_ENTRY_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OBPM_READ_PS_ENTRY_REQ_CREATE(Cap_ID, UCID, Sub_block_ID, PS_rank) \
    (uint16)(Cap_ID), \
    (uint16)(UCID), \
    (uint16)(Sub_block_ID), \
    (uint16)(PS_rank)
#define OBPM_READ_PS_ENTRY_REQ_PACK(obpm_read_ps_entry_req_ptr, Cap_ID, UCID, Sub_block_ID, PS_rank) \
    do { \
        (obpm_read_ps_entry_req_ptr)->_data[0] = (uint16)((uint16)(Cap_ID)); \
        (obpm_read_ps_entry_req_ptr)->_data[1] = (uint16)((uint16)(UCID)); \
        (obpm_read_ps_entry_req_ptr)->_data[2] = (uint16)((uint16)(Sub_block_ID)); \
        (obpm_read_ps_entry_req_ptr)->_data[3] = (uint16)((uint16)(PS_rank)); \
    } while (0)

#define OBPM_READ_PS_ENTRY_REQ_MARSHALL(addr, obpm_read_ps_entry_req_ptr) memcpy((void *)(addr), (void *)(obpm_read_ps_entry_req_ptr), 4)
#define OBPM_READ_PS_ENTRY_REQ_UNMARSHALL(addr, obpm_read_ps_entry_req_ptr) memcpy((void *)(obpm_read_ps_entry_req_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    OBPM_READ_PS_ENTRY_RES

  DESCRIPTION

  MEMBERS
    Cap_ID       - Capability ID (can be zero)
    UCID         - Use case ID
    Sub_block_ID - If used: differentiate between data sets (e.g. state vs.
                   params)
    PS_rank      - Intended persistence rank (see sect. 6.4))
    Status       - Result: 0 - success, non-zero: failed
    payload      - Data payload, if succeeded (and entry has non-zero length)

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OBPM_READ_PS_ENTRY_RES;

/* The following macros take OBPM_READ_PS_ENTRY_RES *obpm_read_ps_entry_res_ptr */
#define OBPM_READ_PS_ENTRY_RES_CAP_ID_WORD_OFFSET (0)
#define OBPM_READ_PS_ENTRY_RES_CAP_ID_GET(obpm_read_ps_entry_res_ptr) ((obpm_read_ps_entry_res_ptr)->_data[0])
#define OBPM_READ_PS_ENTRY_RES_CAP_ID_SET(obpm_read_ps_entry_res_ptr, cap_id) ((obpm_read_ps_entry_res_ptr)->_data[0] = (uint16)(cap_id))
#define OBPM_READ_PS_ENTRY_RES_UCID_WORD_OFFSET (1)
#define OBPM_READ_PS_ENTRY_RES_UCID_GET(obpm_read_ps_entry_res_ptr) ((obpm_read_ps_entry_res_ptr)->_data[1])
#define OBPM_READ_PS_ENTRY_RES_UCID_SET(obpm_read_ps_entry_res_ptr, ucid) ((obpm_read_ps_entry_res_ptr)->_data[1] = (uint16)(ucid))
#define OBPM_READ_PS_ENTRY_RES_SUB_BLOCK_ID_WORD_OFFSET (2)
#define OBPM_READ_PS_ENTRY_RES_SUB_BLOCK_ID_GET(obpm_read_ps_entry_res_ptr) ((obpm_read_ps_entry_res_ptr)->_data[2])
#define OBPM_READ_PS_ENTRY_RES_SUB_BLOCK_ID_SET(obpm_read_ps_entry_res_ptr, sub_block_id) ((obpm_read_ps_entry_res_ptr)->_data[2] = (uint16)(sub_block_id))
#define OBPM_READ_PS_ENTRY_RES_PS_RANK_WORD_OFFSET (3)
#define OBPM_READ_PS_ENTRY_RES_PS_RANK_GET(obpm_read_ps_entry_res_ptr) ((obpm_read_ps_entry_res_ptr)->_data[3])
#define OBPM_READ_PS_ENTRY_RES_PS_RANK_SET(obpm_read_ps_entry_res_ptr, ps_rank) ((obpm_read_ps_entry_res_ptr)->_data[3] = (uint16)(ps_rank))
#define OBPM_READ_PS_ENTRY_RES_STATUS_WORD_OFFSET (4)
#define OBPM_READ_PS_ENTRY_RES_STATUS_GET(obpm_read_ps_entry_res_ptr) ((obpm_read_ps_entry_res_ptr)->_data[4])
#define OBPM_READ_PS_ENTRY_RES_STATUS_SET(obpm_read_ps_entry_res_ptr, status) ((obpm_read_ps_entry_res_ptr)->_data[4] = (uint16)(status))
#define OBPM_READ_PS_ENTRY_RES_PAYLOAD_WORD_OFFSET (5)
#define OBPM_READ_PS_ENTRY_RES_PAYLOAD_GET(obpm_read_ps_entry_res_ptr) ((obpm_read_ps_entry_res_ptr)->_data[5])
#define OBPM_READ_PS_ENTRY_RES_PAYLOAD_SET(obpm_read_ps_entry_res_ptr, payload) ((obpm_read_ps_entry_res_ptr)->_data[5] = (uint16)(payload))
#define OBPM_READ_PS_ENTRY_RES_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OBPM_READ_PS_ENTRY_RES_CREATE(Cap_ID, UCID, Sub_block_ID, PS_rank, Status, payload) \
    (uint16)(Cap_ID), \
    (uint16)(UCID), \
    (uint16)(Sub_block_ID), \
    (uint16)(PS_rank), \
    (uint16)(Status), \
    (uint16)(payload)
#define OBPM_READ_PS_ENTRY_RES_PACK(obpm_read_ps_entry_res_ptr, Cap_ID, UCID, Sub_block_ID, PS_rank, Status, payload) \
    do { \
        (obpm_read_ps_entry_res_ptr)->_data[0] = (uint16)((uint16)(Cap_ID)); \
        (obpm_read_ps_entry_res_ptr)->_data[1] = (uint16)((uint16)(UCID)); \
        (obpm_read_ps_entry_res_ptr)->_data[2] = (uint16)((uint16)(Sub_block_ID)); \
        (obpm_read_ps_entry_res_ptr)->_data[3] = (uint16)((uint16)(PS_rank)); \
        (obpm_read_ps_entry_res_ptr)->_data[4] = (uint16)((uint16)(Status)); \
        (obpm_read_ps_entry_res_ptr)->_data[5] = (uint16)((uint16)(payload)); \
    } while (0)

#define OBPM_READ_PS_ENTRY_RES_MARSHALL(addr, obpm_read_ps_entry_res_ptr) memcpy((void *)(addr), (void *)(obpm_read_ps_entry_res_ptr), 6)
#define OBPM_READ_PS_ENTRY_RES_UNMARSHALL(addr, obpm_read_ps_entry_res_ptr) memcpy((void *)(obpm_read_ps_entry_res_ptr), (void *)(addr), 6)


/*******************************************************************************

  NAME
    OBPM_SET_ENDPOINT_CONFIG_REQ

  DESCRIPTION

  MEMBERS
    Endpoint_ID    - External endpoint ID as seen by host
    Key_IDs_values - key ID MSW, key ID LSW, value MSW, value LSW quad(s)

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_SET_ENDPOINT_CONFIG_REQ;

/* The following macros take OBPM_SET_ENDPOINT_CONFIG_REQ *obpm_set_endpoint_config_req_ptr */
#define OBPM_SET_ENDPOINT_CONFIG_REQ_ENDPOINT_ID_WORD_OFFSET (0)
#define OBPM_SET_ENDPOINT_CONFIG_REQ_ENDPOINT_ID_GET(obpm_set_endpoint_config_req_ptr) ((obpm_set_endpoint_config_req_ptr)->_data[0])
#define OBPM_SET_ENDPOINT_CONFIG_REQ_ENDPOINT_ID_SET(obpm_set_endpoint_config_req_ptr, endpoint_id) ((obpm_set_endpoint_config_req_ptr)->_data[0] = (uint16)(endpoint_id))
#define OBPM_SET_ENDPOINT_CONFIG_REQ_KEY_IDS_VALUES_WORD_OFFSET (1)
#define OBPM_SET_ENDPOINT_CONFIG_REQ_KEY_IDS_VALUES_GET(obpm_set_endpoint_config_req_ptr) ((obpm_set_endpoint_config_req_ptr)->_data[1])
#define OBPM_SET_ENDPOINT_CONFIG_REQ_KEY_IDS_VALUES_SET(obpm_set_endpoint_config_req_ptr, key_ids_values) ((obpm_set_endpoint_config_req_ptr)->_data[1] = (uint16)(key_ids_values))
#define OBPM_SET_ENDPOINT_CONFIG_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_SET_ENDPOINT_CONFIG_REQ_CREATE(Endpoint_ID, Key_IDs_values) \
    (uint16)(Endpoint_ID), \
    (uint16)(Key_IDs_values)
#define OBPM_SET_ENDPOINT_CONFIG_REQ_PACK(obpm_set_endpoint_config_req_ptr, Endpoint_ID, Key_IDs_values) \
    do { \
        (obpm_set_endpoint_config_req_ptr)->_data[0] = (uint16)((uint16)(Endpoint_ID)); \
        (obpm_set_endpoint_config_req_ptr)->_data[1] = (uint16)((uint16)(Key_IDs_values)); \
    } while (0)

#define OBPM_SET_ENDPOINT_CONFIG_REQ_MARSHALL(addr, obpm_set_endpoint_config_req_ptr) memcpy((void *)(addr), (void *)(obpm_set_endpoint_config_req_ptr), 2)
#define OBPM_SET_ENDPOINT_CONFIG_REQ_UNMARSHALL(addr, obpm_set_endpoint_config_req_ptr) memcpy((void *)(obpm_set_endpoint_config_req_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_SET_ENDPOINT_CONFIG_RES

  DESCRIPTION

  MEMBERS
    Status - Status code

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_SET_ENDPOINT_CONFIG_RES;

/* The following macros take OBPM_SET_ENDPOINT_CONFIG_RES *obpm_set_endpoint_config_res_ptr */
#define OBPM_SET_ENDPOINT_CONFIG_RES_STATUS_WORD_OFFSET (0)
#define OBPM_SET_ENDPOINT_CONFIG_RES_STATUS_GET(obpm_set_endpoint_config_res_ptr) ((obpm_set_endpoint_config_res_ptr)->_data[0])
#define OBPM_SET_ENDPOINT_CONFIG_RES_STATUS_SET(obpm_set_endpoint_config_res_ptr, status) ((obpm_set_endpoint_config_res_ptr)->_data[0] = (uint16)(status))
#define OBPM_SET_ENDPOINT_CONFIG_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_SET_ENDPOINT_CONFIG_RES_CREATE(Status) \
    (uint16)(Status)
#define OBPM_SET_ENDPOINT_CONFIG_RES_PACK(obpm_set_endpoint_config_res_ptr, Status) \
    do { \
        (obpm_set_endpoint_config_res_ptr)->_data[0] = (uint16)((uint16)(Status)); \
    } while (0)

#define OBPM_SET_ENDPOINT_CONFIG_RES_MARSHALL(addr, obpm_set_endpoint_config_res_ptr) memcpy((void *)(addr), (void *)(obpm_set_endpoint_config_res_ptr), 1)
#define OBPM_SET_ENDPOINT_CONFIG_RES_UNMARSHALL(addr, obpm_set_endpoint_config_res_ptr) memcpy((void *)(obpm_set_endpoint_config_res_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_SET_SYSTEM_STREAM_RATE_REQ

  DESCRIPTION

  MEMBERS
    Sample_rate - System sampling rate

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OBPM_SET_SYSTEM_STREAM_RATE_REQ;

/* The following macros take OBPM_SET_SYSTEM_STREAM_RATE_REQ *obpm_set_system_stream_rate_req_ptr */
#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_SAMPLE_RATE_WORD_OFFSET (0)
#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_SAMPLE_RATE_GET(obpm_set_system_stream_rate_req_ptr)  \
    (((uint32)((obpm_set_system_stream_rate_req_ptr)->_data[0]) | \
      ((uint32)((obpm_set_system_stream_rate_req_ptr)->_data[1]) << 16)))
#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_SAMPLE_RATE_SET(obpm_set_system_stream_rate_req_ptr, sample_rate) do { \
        (obpm_set_system_stream_rate_req_ptr)->_data[0] = (uint16)((sample_rate) & 0xffff); \
        (obpm_set_system_stream_rate_req_ptr)->_data[1] = (uint16)((sample_rate) >> 16); } while (0)
#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_CREATE(Sample_rate) \
    (uint16)((Sample_rate) & 0xffff), \
    (uint16)((Sample_rate) >> 16)
#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_PACK(obpm_set_system_stream_rate_req_ptr, Sample_rate) \
    do { \
        (obpm_set_system_stream_rate_req_ptr)->_data[0] = (uint16)((uint16)((Sample_rate) & 0xffff)); \
        (obpm_set_system_stream_rate_req_ptr)->_data[1] = (uint16)(((Sample_rate) >> 16)); \
    } while (0)

#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_MARSHALL(addr, obpm_set_system_stream_rate_req_ptr) memcpy((void *)(addr), (void *)(obpm_set_system_stream_rate_req_ptr), 2)
#define OBPM_SET_SYSTEM_STREAM_RATE_REQ_UNMARSHALL(addr, obpm_set_system_stream_rate_req_ptr) memcpy((void *)(obpm_set_system_stream_rate_req_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OBPM_SET_SYSTEM_STREAM_RATE_RES

  DESCRIPTION

  MEMBERS
    Status - Status code

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_SET_SYSTEM_STREAM_RATE_RES;

/* The following macros take OBPM_SET_SYSTEM_STREAM_RATE_RES *obpm_set_system_stream_rate_res_ptr */
#define OBPM_SET_SYSTEM_STREAM_RATE_RES_STATUS_WORD_OFFSET (0)
#define OBPM_SET_SYSTEM_STREAM_RATE_RES_STATUS_GET(obpm_set_system_stream_rate_res_ptr) ((obpm_set_system_stream_rate_res_ptr)->_data[0])
#define OBPM_SET_SYSTEM_STREAM_RATE_RES_STATUS_SET(obpm_set_system_stream_rate_res_ptr, status) ((obpm_set_system_stream_rate_res_ptr)->_data[0] = (uint16)(status))
#define OBPM_SET_SYSTEM_STREAM_RATE_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_SET_SYSTEM_STREAM_RATE_RES_CREATE(Status) \
    (uint16)(Status)
#define OBPM_SET_SYSTEM_STREAM_RATE_RES_PACK(obpm_set_system_stream_rate_res_ptr, Status) \
    do { \
        (obpm_set_system_stream_rate_res_ptr)->_data[0] = (uint16)((uint16)(Status)); \
    } while (0)

#define OBPM_SET_SYSTEM_STREAM_RATE_RES_MARSHALL(addr, obpm_set_system_stream_rate_res_ptr) memcpy((void *)(addr), (void *)(obpm_set_system_stream_rate_res_ptr), 1)
#define OBPM_SET_SYSTEM_STREAM_RATE_RES_UNMARSHALL(addr, obpm_set_system_stream_rate_res_ptr) memcpy((void *)(obpm_set_system_stream_rate_res_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_SYSTEM_EVENT_RES

  DESCRIPTION

  MEMBERS
    Event_Status - Bitflag of system events

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OBPM_SYSTEM_EVENT_RES;

/* The following macros take OBPM_SYSTEM_EVENT_RES *obpm_system_event_res_ptr */
#define OBPM_SYSTEM_EVENT_RES_EVENT_STATUS_WORD_OFFSET (0)
#define OBPM_SYSTEM_EVENT_RES_EVENT_STATUS_GET(obpm_system_event_res_ptr) ((obpm_system_event_res_ptr)->_data[0])
#define OBPM_SYSTEM_EVENT_RES_EVENT_STATUS_SET(obpm_system_event_res_ptr, event_status) ((obpm_system_event_res_ptr)->_data[0] = (uint16)(event_status))
#define OBPM_SYSTEM_EVENT_RES_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OBPM_SYSTEM_EVENT_RES_CREATE(Event_Status) \
    (uint16)(Event_Status)
#define OBPM_SYSTEM_EVENT_RES_PACK(obpm_system_event_res_ptr, Event_Status) \
    do { \
        (obpm_system_event_res_ptr)->_data[0] = (uint16)((uint16)(Event_Status)); \
    } while (0)

#define OBPM_SYSTEM_EVENT_RES_MARSHALL(addr, obpm_system_event_res_ptr) memcpy((void *)(addr), (void *)(obpm_system_event_res_ptr), 1)
#define OBPM_SYSTEM_EVENT_RES_UNMARSHALL(addr, obpm_system_event_res_ptr) memcpy((void *)(obpm_system_event_res_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    OBPM_WRITE_PS_ENTRY_REQ

  DESCRIPTION

  MEMBERS
    Cap_ID       - Capability ID (can be zero)
    UCID         - Use case ID
    Sub_block_ID - If used: differentiate between data sets (e.g. state vs.
                   params)
    PS_rank      - Intended persistence rank (see sect. 6.4))
    payload      - If present, data payload (zero-length entries are possible)

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OBPM_WRITE_PS_ENTRY_REQ;

/* The following macros take OBPM_WRITE_PS_ENTRY_REQ *obpm_write_ps_entry_req_ptr */
#define OBPM_WRITE_PS_ENTRY_REQ_CAP_ID_WORD_OFFSET (0)
#define OBPM_WRITE_PS_ENTRY_REQ_CAP_ID_GET(obpm_write_ps_entry_req_ptr) ((obpm_write_ps_entry_req_ptr)->_data[0])
#define OBPM_WRITE_PS_ENTRY_REQ_CAP_ID_SET(obpm_write_ps_entry_req_ptr, cap_id) ((obpm_write_ps_entry_req_ptr)->_data[0] = (uint16)(cap_id))
#define OBPM_WRITE_PS_ENTRY_REQ_UCID_WORD_OFFSET (1)
#define OBPM_WRITE_PS_ENTRY_REQ_UCID_GET(obpm_write_ps_entry_req_ptr) ((obpm_write_ps_entry_req_ptr)->_data[1])
#define OBPM_WRITE_PS_ENTRY_REQ_UCID_SET(obpm_write_ps_entry_req_ptr, ucid) ((obpm_write_ps_entry_req_ptr)->_data[1] = (uint16)(ucid))
#define OBPM_WRITE_PS_ENTRY_REQ_SUB_BLOCK_ID_WORD_OFFSET (2)
#define OBPM_WRITE_PS_ENTRY_REQ_SUB_BLOCK_ID_GET(obpm_write_ps_entry_req_ptr) ((obpm_write_ps_entry_req_ptr)->_data[2])
#define OBPM_WRITE_PS_ENTRY_REQ_SUB_BLOCK_ID_SET(obpm_write_ps_entry_req_ptr, sub_block_id) ((obpm_write_ps_entry_req_ptr)->_data[2] = (uint16)(sub_block_id))
#define OBPM_WRITE_PS_ENTRY_REQ_PS_RANK_WORD_OFFSET (3)
#define OBPM_WRITE_PS_ENTRY_REQ_PS_RANK_GET(obpm_write_ps_entry_req_ptr) ((obpm_write_ps_entry_req_ptr)->_data[3])
#define OBPM_WRITE_PS_ENTRY_REQ_PS_RANK_SET(obpm_write_ps_entry_req_ptr, ps_rank) ((obpm_write_ps_entry_req_ptr)->_data[3] = (uint16)(ps_rank))
#define OBPM_WRITE_PS_ENTRY_REQ_PAYLOAD_WORD_OFFSET (4)
#define OBPM_WRITE_PS_ENTRY_REQ_PAYLOAD_GET(obpm_write_ps_entry_req_ptr) ((obpm_write_ps_entry_req_ptr)->_data[4])
#define OBPM_WRITE_PS_ENTRY_REQ_PAYLOAD_SET(obpm_write_ps_entry_req_ptr, payload) ((obpm_write_ps_entry_req_ptr)->_data[4] = (uint16)(payload))
#define OBPM_WRITE_PS_ENTRY_REQ_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OBPM_WRITE_PS_ENTRY_REQ_CREATE(Cap_ID, UCID, Sub_block_ID, PS_rank, payload) \
    (uint16)(Cap_ID), \
    (uint16)(UCID), \
    (uint16)(Sub_block_ID), \
    (uint16)(PS_rank), \
    (uint16)(payload)
#define OBPM_WRITE_PS_ENTRY_REQ_PACK(obpm_write_ps_entry_req_ptr, Cap_ID, UCID, Sub_block_ID, PS_rank, payload) \
    do { \
        (obpm_write_ps_entry_req_ptr)->_data[0] = (uint16)((uint16)(Cap_ID)); \
        (obpm_write_ps_entry_req_ptr)->_data[1] = (uint16)((uint16)(UCID)); \
        (obpm_write_ps_entry_req_ptr)->_data[2] = (uint16)((uint16)(Sub_block_ID)); \
        (obpm_write_ps_entry_req_ptr)->_data[3] = (uint16)((uint16)(PS_rank)); \
        (obpm_write_ps_entry_req_ptr)->_data[4] = (uint16)((uint16)(payload)); \
    } while (0)

#define OBPM_WRITE_PS_ENTRY_REQ_MARSHALL(addr, obpm_write_ps_entry_req_ptr) memcpy((void *)(addr), (void *)(obpm_write_ps_entry_req_ptr), 5)
#define OBPM_WRITE_PS_ENTRY_REQ_UNMARSHALL(addr, obpm_write_ps_entry_req_ptr) memcpy((void *)(obpm_write_ps_entry_req_ptr), (void *)(addr), 5)


/*******************************************************************************

  NAME
    OBPM_WRITE_PS_ENTRY_RES

  DESCRIPTION

  MEMBERS
    Cap_ID       - Capability ID (can be zero)
    UCID         - Use case ID
    Sub_block_ID - If used: differentiate between data sets (e.g. state vs.
                   params)
    PS_rank      - Intended persistence rank (see sect. 6.4))
    Status       - Result: 0 - success, non-zero: failed

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OBPM_WRITE_PS_ENTRY_RES;

/* The following macros take OBPM_WRITE_PS_ENTRY_RES *obpm_write_ps_entry_res_ptr */
#define OBPM_WRITE_PS_ENTRY_RES_CAP_ID_WORD_OFFSET (0)
#define OBPM_WRITE_PS_ENTRY_RES_CAP_ID_GET(obpm_write_ps_entry_res_ptr) ((obpm_write_ps_entry_res_ptr)->_data[0])
#define OBPM_WRITE_PS_ENTRY_RES_CAP_ID_SET(obpm_write_ps_entry_res_ptr, cap_id) ((obpm_write_ps_entry_res_ptr)->_data[0] = (uint16)(cap_id))
#define OBPM_WRITE_PS_ENTRY_RES_UCID_WORD_OFFSET (1)
#define OBPM_WRITE_PS_ENTRY_RES_UCID_GET(obpm_write_ps_entry_res_ptr) ((obpm_write_ps_entry_res_ptr)->_data[1])
#define OBPM_WRITE_PS_ENTRY_RES_UCID_SET(obpm_write_ps_entry_res_ptr, ucid) ((obpm_write_ps_entry_res_ptr)->_data[1] = (uint16)(ucid))
#define OBPM_WRITE_PS_ENTRY_RES_SUB_BLOCK_ID_WORD_OFFSET (2)
#define OBPM_WRITE_PS_ENTRY_RES_SUB_BLOCK_ID_GET(obpm_write_ps_entry_res_ptr) ((obpm_write_ps_entry_res_ptr)->_data[2])
#define OBPM_WRITE_PS_ENTRY_RES_SUB_BLOCK_ID_SET(obpm_write_ps_entry_res_ptr, sub_block_id) ((obpm_write_ps_entry_res_ptr)->_data[2] = (uint16)(sub_block_id))
#define OBPM_WRITE_PS_ENTRY_RES_PS_RANK_WORD_OFFSET (3)
#define OBPM_WRITE_PS_ENTRY_RES_PS_RANK_GET(obpm_write_ps_entry_res_ptr) ((obpm_write_ps_entry_res_ptr)->_data[3])
#define OBPM_WRITE_PS_ENTRY_RES_PS_RANK_SET(obpm_write_ps_entry_res_ptr, ps_rank) ((obpm_write_ps_entry_res_ptr)->_data[3] = (uint16)(ps_rank))
#define OBPM_WRITE_PS_ENTRY_RES_STATUS_WORD_OFFSET (4)
#define OBPM_WRITE_PS_ENTRY_RES_STATUS_GET(obpm_write_ps_entry_res_ptr) ((obpm_write_ps_entry_res_ptr)->_data[4])
#define OBPM_WRITE_PS_ENTRY_RES_STATUS_SET(obpm_write_ps_entry_res_ptr, status) ((obpm_write_ps_entry_res_ptr)->_data[4] = (uint16)(status))
#define OBPM_WRITE_PS_ENTRY_RES_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OBPM_WRITE_PS_ENTRY_RES_CREATE(Cap_ID, UCID, Sub_block_ID, PS_rank, Status) \
    (uint16)(Cap_ID), \
    (uint16)(UCID), \
    (uint16)(Sub_block_ID), \
    (uint16)(PS_rank), \
    (uint16)(Status)
#define OBPM_WRITE_PS_ENTRY_RES_PACK(obpm_write_ps_entry_res_ptr, Cap_ID, UCID, Sub_block_ID, PS_rank, Status) \
    do { \
        (obpm_write_ps_entry_res_ptr)->_data[0] = (uint16)((uint16)(Cap_ID)); \
        (obpm_write_ps_entry_res_ptr)->_data[1] = (uint16)((uint16)(UCID)); \
        (obpm_write_ps_entry_res_ptr)->_data[2] = (uint16)((uint16)(Sub_block_ID)); \
        (obpm_write_ps_entry_res_ptr)->_data[3] = (uint16)((uint16)(PS_rank)); \
        (obpm_write_ps_entry_res_ptr)->_data[4] = (uint16)((uint16)(Status)); \
    } while (0)

#define OBPM_WRITE_PS_ENTRY_RES_MARSHALL(addr, obpm_write_ps_entry_res_ptr) memcpy((void *)(addr), (void *)(obpm_write_ps_entry_res_ptr), 5)
#define OBPM_WRITE_PS_ENTRY_RES_UNMARSHALL(addr, obpm_write_ps_entry_res_ptr) memcpy((void *)(obpm_write_ps_entry_res_ptr), (void *)(addr), 5)


#endif /* OBPM_PRIM_H */

