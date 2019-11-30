/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_kip.c
 * \ingroup stream
 *
 * Private Stream KIP elements. <br>
 *
 */

#ifndef STREAM_KIP_H
#define STREAM_KIP_H

/****************************************************************************
Include Files
*/

#include "stream/stream.h"
#include "stream/stream_transform.h"
#include "stream/stream_endpoint.h"
#include "kip_msg_prim.h"
#include "ipc/ipc_kip.h"
#include "ipc/ipc_macros.h"

/****************************************************************************
Type Declarations
*/

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)

/* Do not change these value since it is used for bit operation
 * Location of the endpoint based on the processor context. From P0
 * STREAM_EP_REMOTE_ALL means source and sink endpoints are not located
 * with P0 ( on dual core, those are with P1)
 */
typedef enum
{
    STREAM_EP_REMOTE_NONE   = 0x0,
    STREAM_EP_REMOTE_SOURCE = 0x1,
    STREAM_EP_REMOTE_SINK   = 0x2,
    STREAM_EP_REMOTE_ALL    = 0x3
} STREAM_EP_LOCATION;

/* Structure for keeping record of connect stages when shadow EPs are involved */
typedef struct
{
    /* Packed connection ID with remote processor id */
    uint16 packed_con_id;

    /* Local endpoint ids
     * This might be a external operator endpoint id or a real endpoint id
     * or a shadow operator endpoint id.
     */
    uint16 source_id;
    uint16 sink_id;

    /* internal transform id */
    uint16 tr_id;

    uint16 data_channel_id;
#ifdef INSTALL_METADATA_DUALCORE
    uint16 meta_channel_id;
    bool metadata_channel_is_activated : 1;
#endif
    bool data_channel_is_activated : 1;

    /* endpoint locations */
    STREAM_EP_LOCATION ep_location;

    STREAM_CONNECT_INFO connect_info;

    /* The callback to notify connection status */
    bool (*callback)(unsigned con_id, unsigned status, unsigned transform_id);

} STREAM_KIP_CONNECT_INFO;

#if defined(INSTALL_DUAL_CORE_SUPPORT)

typedef union
{
    bool (*tr_disc_cb)(unsigned, unsigned,unsigned);
    bool (*disc_cb)(unsigned, unsigned, unsigned, unsigned);
}STREAM_KIP_TRANSFORM_DISCONNECT_CB;

/* Structure for keeping record of transform disconnect
 * when shadow EPs are involved
 **/
typedef struct
{
    /* Packed connection ID with remote processor id */
    uint16 packed_con_id;

    /* count */
    uint16 count;

    /* success count */
    uint16 success_count;

    /* flag to show which callback to call */
    bool disc_cb_flag;

    /* remote success count */
    uint16 remote_success_count;

    /* The callback to notify transform disconnect status */
    STREAM_KIP_TRANSFORM_DISCONNECT_CB callback;

    /* transform list. Don't change the position from here */
    unsigned tr_list[1];

} STREAM_KIP_TRANSFORM_DISCONNECT_INFO;

#endif /* INSTALL_DUAL_CORE_SUPPORT */

#endif /* INSTALL_DUAL_CORE_SUPPORT | AUDIO_SECOND_CORE */

/* Structure used for maintaining remote kip transform information.
 * P0 uses the same to maintain remote transform list as well */
typedef struct STREAM_KIP_TRANSFORM_INFO
{
    /* This is the source id at the secondary core*/
    unsigned source_id;

    /* This is the sink id at the secondary core*/
    unsigned sink_id;

    /* data channel id. 0 if it is not present */
    uint16 data_channel_id;

#ifdef INSTALL_METADATA_DUALCORE
    /* Metadata channel ID */
    uint16 meta_channel_id;
#endif

    /* external transform id */
    unsigned id:8;

    /* remote Processor id */
    IPC_PROCESSOR_ID_NUM processor_id:3;

    /* Enable to allow using the data channel connected to
     * the transform. Disable it to allow deactivating the
     * data channel.
     */
    bool enabled:1;

    /*Real source ep*/
    bool real_source_ep:1;

    /*Real sink ep*/
    bool real_sink_ep:1;

    struct STREAM_KIP_TRANSFORM_INFO* next;
} STREAM_KIP_TRANSFORM_INFO;

/****************************************************************************
Constant Declarations
*/

/****************************************************************************
Macro Declarations
*/

#ifdef KIP_DEBUG
#define STREAM_KIP_ASSERT( x ) PL_ASSERT( x )
#else
#define STREAM_KIP_ASSERT( x )
#endif

#ifndef INSTALL_DELEGATE_AUDIO_HW

#define STREAM_KIP_VALIDATE_EPS(ep1, ep2) \
            (!(STREAM_EP_IS_REALEP_ID(ep1) || STREAM_EP_IS_REALEP_ID(ep2)))
#else

#define STREAM_KIP_VALIDATE_EPS(ep1, ep2) \
            (!(STREAM_EP_IS_REALEP_ID(ep1) && STREAM_EP_IS_REALEP_ID(ep2)))
#endif




/****************************************************************************
Variable Definitions
*/
/**
 * This keeps information about the remote transforms. On P0, this list contains
 * both transform associated with KIP endpoints as well as P1 transforms.
 * On P1 list, it contains only the KIP transforms. P0 and P1 local transforms
 * are maintained in its transform_list.
 **/
extern STREAM_KIP_TRANSFORM_INFO *kip_transform_list;


/****************************************************************************
Function Declarations
*/

/* IPC event callback handlers.*/
/**
 * \brief Indication from IPC when the data channel activated
 *
 * \param status     - IPC_SUCCESS on success
 * \param proc_id    - The remote processor id connected to the data channel
 * \param channel_id - The data channel id
 * \param param_len  - param length ( expected 0 and ignored)
 * \param params     - params ( none expected)
 *
 * \return IPC_STATUS since it is an ipc callback return
 */
IPC_STATUS stream_kip_data_channel_activated( IPC_STATUS status,
                                              IPC_PROCESSOR_ID_NUM proc_id,
                                              uint16 channal_id,
                                              unsigned param_len,
                                              void* params);
/**
 * \brief Indication from IPC when the data channel deactivated
 *
 * \param status     - IPC_SUCCESS on success
 * \param channel_id - The data channel id
 *
 * \return IPC_STATUS since it is an ipc callback return
 */
IPC_STATUS stream_kip_data_channel_deactivated( IPC_STATUS status,
                                                uint16 channel );


#ifdef INSTALL_DUAL_CORE_SUPPORT

/**
 * \brief Create a connect info record during connection state and partially
 *        initialise it.
 *
 * \param con_id    - The packed connection id
 * \param source_id - The source id at the local side
 * \param sink_id   - The sink id at the  local side
 * \param ep_location - Location of endpoints
 * \param callback  - The callback to be called after handling the response
 *
 * \return  A connect information record or NULL
 */
STREAM_KIP_CONNECT_INFO *stream_kip_create_connect_info_record(
                           unsigned con_id, unsigned source_id, unsigned sink_id,
                           STREAM_EP_LOCATION ep_location,
                           bool (*callback)(unsigned con_id, unsigned status,
                           unsigned transform_id));

/**
 * \brief Generate a transform id and send a KIP stream connect request
 *
 * \param con_id        The packed connection id
 * \param source_id     The source endpoint id
 * \param sink_id       The sink endpoint id
 * \param state         The connect state info
 *
 * \result bool        TRUE on success
 *
 * \return
 */
bool stream_kip_connect_endpoints( unsigned packed_con_id,
                                   unsigned source_id,
                                   unsigned sink_id,
                                   STREAM_KIP_CONNECT_INFO *state);

/**
 * \brief create local endpoints and send a KIP stream create endpoint request
 *
 * \param con_id        The packed connection id
 * \param source_id     The source endpoint id
 * \param sink_id       The sink endpoint id
 * \param state         The connect state info
 *
 * \result bool        TRUE on success
 *
 * \return
 */
bool stream_kip_create_endpoints( unsigned packed_con_id,
                                  unsigned source_id,
                                  unsigned sink_id,
                                  STREAM_KIP_CONNECT_INFO *state);

/**
 * \brief Handle the stream connect response from kip
 *
 * \param con_id        The connection id
 * \param status        status of the request
 * \param transform_id  transform id returned
 *
 * \return
 */
void stream_kip_connect_response_handler( unsigned con_id, unsigned status,
                                          unsigned transform_id,
                                          STREAM_KIP_CONNECT_INFO *state);

/**
 * \brief Handle the transform disconnect response from kip
 *
 * \param con_id        The connection id
 * \param status        status of the request
 * \param count         The number of disconnected transforms
 * \param state         The disconnect state
 *
 * \return
 */
void stream_kip_transform_disconnect_response_handler( unsigned con_id,
                                                       unsigned status,
                                                       unsigned count,
                                     STREAM_KIP_TRANSFORM_DISCONNECT_INFO *state);

/**
 * \brief Handling the incoming create endpoint response.
 *
 * \param con_id - connection id
 * \param status - status
 * \param channel_id - The data channel id. This must not be 0.
 * \param buffer_size - negotiated buffer size for the connection
 * \param flags       - The buffer related flags
 * \param state_info - The connection state information.
 *
 * \return
 */

void stream_kip_create_endpoints_response_handler( unsigned con_id,
                                                   unsigned status,
                                                   unsigned channel_id,
                                                   unsigned buffer_size,
                                                   unsigned flags,
                                               STREAM_KIP_CONNECT_INFO *state_info);

/**
 * \brief Handling the incoming destroy endpoint response.
 *
 * \param con_id - connection id
 * \param status - status
 * \param state_info - The connection state information.
 *
 * \return
 */

void stream_kip_destroy_endpoints_response_handler( unsigned con_id,
                                                    unsigned status,
                                             STREAM_KIP_CONNECT_INFO *state_info);



/**
 * \brief Find the start of secondary core transforms in the list
 *
 * \param count   - The total number of transforms in the list
 * \param tr_list -The transform list
 *
 * \return
 */
unsigned stream_kip_find_px_transform_start(unsigned count, unsigned *tr_id_list);

/**
 * \brief Create disconnect info record
 *
 * \prama con_id  - The connection id
 * \param count   - Number of transforms in the list
 * \param transforms - The transform list
 * \param callback  - The callback
 *
 * \return
 */
STREAM_KIP_TRANSFORM_DISCONNECT_INFO *stream_kip_create_disconnect_info_record(
                            unsigned con_id, unsigned count,
                            bool ep_disc_cb, unsigned *transforms,
                            STREAM_KIP_TRANSFORM_DISCONNECT_CB callback );
/**
 * \brief  Send a KIP stream disconnect
 *
 * \param state  - disconnect state
 * \param offset - Offset to start processing the transform list of the
 *                 state for the secondary core.
 * \return
 */

bool stream_kip_transform_disconnect( STREAM_KIP_TRANSFORM_DISCONNECT_INFO *state,
                                      unsigned success_count);


#endif /* INSTALL_DUAL_CORE_SUPPORT */

/**
 * \brief Disconnect the transform associated with an endpoint
 *
 * \param endpoint [IN]  - The KIP endpoint
 * \param tr_id    [OUT] - The (external) transform id
 *
 * \return
 */
bool stream_transform_id_from_endpoint(ENDPOINT *endpoint, unsigned *tr_id);

/**
 * \brief Remove the transform associated with an endpoint that
 *        is listed in P0 kip_transform_list.
 *
 * \param tr_id    - The external transform id
 * \param proc_id  - The processor_id
 *
 * \return
 */
bool stream_kip_cleanup_endpoint_transform( unsigned tr_id, IPC_PROCESSOR_ID_NUM proc_id);

/**
 * \brief Disconnect the transform associated with an endpoint
 *
 * \param endpoint - The KIP endpoint
 * \param proc_id  - The processor_id
 *
 * \return
 */
bool stream_kip_disconnect_endpoint( ENDPOINT *endpoint, IPC_PROCESSOR_ID_NUM proc_id);

/**
 * \brief  Destroy the data channel from ipc
 *
 * \param channel - The data channel to be deactivated.
 *
 * \return
 */
bool stream_kip_data_channel_destroy_ipc(uint16 channel);
/**
 * \brief  Destroy the data channel
 *
 * \param channel - Data channel to be destroyed.
 *
 * \return
 */
bool stream_kip_data_channel_destroy( uint16 channel );

/**
 * \brief  Deactivate the data channel from ipc
 *
 * \param channel - The data channel to be deactivated.
 *
 * \return
 */
bool stream_kip_data_channel_deactivate_ipc(uint16 channel);

/**
 * \brief  Deactivate the data channel and its associated transform
 *
 * \param channel - The data channel to be deactivated.
 *
 * \return
 */
bool stream_kip_data_channel_deactivate( uint16 channel );

/**
 * \brief Get KIP transform from KIP transform information list
 *
 * \param id  transform id
 *
 * \return   KIP transform info entry
 */
STREAM_KIP_TRANSFORM_INFO* stream_kip_transform_info_from_id(unsigned id);

/**
 * \brief Get KIP transform from KIP transform information list
 *
 * \param id  endpoint id
 *
 * \return   KIP transform info entry
 */
STREAM_KIP_TRANSFORM_INFO* stream_kip_transform_info_from_epid(unsigned epid);

/**
 * \brief Helper function to find the ID of a remote endpoint connected to a
 * known endpoint.
 *
 * \param epid  The ID of the endpoint that is known
 *
 * \return The ID of the endpoint connected to endpoint with ID epid. 0 if not found.
 */
unsigned stream_kip_connected_to_epid(unsigned epid);

/**
 * \brief Helper function to create remote transform info and add it to
 *        the list.
 *
 * \param id - internal transform id
 * \param processor_id - remote processor id
 * \param source_id - remote source id
 * \param sink_id   - remote sink id
 * \param id - remote processor id
 * \param id - data channel id
 *
 * \return  tr_info
 */
STREAM_KIP_TRANSFORM_INFO* stream_kip_add_transform_info( unsigned id,
                                IPC_PROCESSOR_ID_NUM processor_id,
                                unsigned source_id,
                                unsigned sink_id,
                                uint16 data_chan_id );

/**
 * \brief Helper function to remove remote transform info
 *
 * \param id - transform
 *
 * \return  tr_info
 */
void stream_kip_remove_transform_info(STREAM_KIP_TRANSFORM_INFO *transform);

/**
 * \brief Helper function to remove remote transform info
 *
 * \param id - internal transform id
 *
 * \return  none
 */
void stream_kip_remove_transform_info_by_id(unsigned tr_id);

/**
 * \brief Helper function to retrieve entry in remote transform
 *        info list based on data channel ID.
 *
 * \param id - data channel id
 *
 * \return  tr_info
 */
STREAM_KIP_TRANSFORM_INFO* stream_kip_transform_info_from_chanid(uint16 data_chan_id);

/**
 * \brief Handling the incoming stream disconnect request from P0
 *
 * \param con_id        The connection id
 * \param count         Number of transforms to disconnect
 * \param tr_list       The list of transforms
 *
 * \return
 */
void stream_kip_transform_disconnect_request_handler( unsigned con_id,
                                                      unsigned count,
                                                      unsigned *tr_list );

/**
 * \brief Send operator's endpoint ID request to another processor (P1)
 *
 * \param opid          The operator id of which to request source/sink ep id
 * \param idx           The endpoint source/sink channel index of the operator
 * \param direction     Endpoint direction (source/sink)
 * \param con_id        The connection id
 *
 * \return
 */
bool stream_kip_operator_get_endpoint(unsigned int opid,
                                      unsigned int idx,
                                      ENDPOINT_DIRECTION dir,
                                      unsigned con_id,
                                    bool (*callback)(unsigned con_id, unsigned status, unsigned source_id));

#ifdef INSTALL_DUAL_CORE_SUPPORT
/**
 * \brief Handling the incoming kip_transform_list entry remove
 *        request from secondary core
 *
 * \param con_id        The connection id
 * \param count         Number of transforms to cleanup/remove
 * \param tr_list       The list of transforms
 *
 * \return
 */
void stream_kip_transform_list_remove_entry_request_handler( unsigned con_id,
                                                             unsigned count,
                                                             unsigned *tr_list );
#endif

#ifdef AUDIO_SECOND_CORE

/**
 * \brief Handle the transform pxcopy cleanup response from kip
 *
 * \param con_id        The connection id
 * \param status        status of the request
 * \param count         The number of removed pxcopy transforms
 * \param state         The disconnect state
 *
 * \return
 */
void stream_kip_transform_list_remove_entry_response_handler( unsigned con_id,
                                                              unsigned status,
                                                              unsigned count,
                                                              void     *state);

/**
 * \brief Handling the incoming stream connect request from P0
 *
 * \param con_id        The connection id
 * \param source_id     The source endpoint id
 * \param sink_id       The sink endpoint id
 * \param transform_id  The transform id
 * \param channel_id    The data channel id
 *
 * \return
 */
void stream_kip_connect_request_handler( unsigned con_id,
                                         unsigned source_id,
                                         unsigned sink_id,
                                         unsigned transform_id,
                                         unsigned channel_id );



/**
 * \brief Handling the incoming create endpoints request from P0
 *
 * \param con_id        The connection id
 * \param source_id     The source endpoint id
 * \param sink_id       The sink endpoint id
 * \param channel_id    The data channel id
 * \param buffer_size - The buffer size for negotiation
 * \param flags       - The buffer related flags
 *
 * \return
 */
void stream_kip_create_endpoints_request_handler( unsigned con_id,
                                                  unsigned source_id,
                                                  unsigned sink_id,
                                                  unsigned channel_id,
                                                  unsigned buffer_size,
                                                  unsigned flags );

/**
 * \brief Handling the incoming destroy endpoints request from P0
 *
 * \param con_id        The connection id
 * \param source_id     The source endpoint id
 * \param sink_id       The sink endpoint id
 *
 * \return
 */
void stream_kip_destroy_endpoints_request_handler( unsigned con_id,
                                                  unsigned source_id,
                                                  unsigned sink_id);

/**
 * \brief Return the external id for a specified operator endpoint on Px
 *        (x not 0). Whether a source or sink is requested is parameterised.
 *        If the source could not be found then the id will be returned as 0.
 *        Note: like stream_if_get_endpoint but specifically for enquiries
 *        from P0 on Px as to the operator endpoint ID
 *        (ACCMD stream_get_source/sink_req).
 *
 * \param con_id connection ID of the originator of this request
 * \param device the endpoint device type, such as SCO, USB, PCM or I2S.
 * \param num_params the number of parameters passed in the params array
 * \param params an array of typically 2 values a supporting parameter
 *      that typically specifies the particular instance and channel
 *      of the device type.
 * \param dir Whether a source or sink is being requested
 * \param callback the callback function to be called when the result is
 *      known
 *
 */
void stream_kip_px_if_get_endpoint(unsigned con_id, unsigned device, unsigned num_params,
        unsigned *params, ENDPOINT_DIRECTION dir,
        bool (*callback)(unsigned con_id, unsigned status, unsigned source_id));

#endif /* AUDIO_SECOND_CORE */


#ifdef INSTALL_METADATA_DUALCORE
/**
* \brief  Get metadata_buffer from the same endpoint based
*
* \return metadata_buffer if it found any, otherwise, NULL
*/
extern tCbuffer *stream_kip_return_metadata_buf(ENDPOINT *ep);

/**
* \brief  Check if this endpoint is in the last metadata data connection
*
* \return TRUE if it is, otherwise, FALSE;
*/
extern bool stream_kip_is_last_meta_connection(ENDPOINT *endpoint);

/**
* \brief  Request remote to set the activated flag in the kip state with
*         an existing metadata data channel. Then, send a response back
*/
extern void stream_kip_metadata_channel_activated_req_handler(
                                                       unsigned packed_con_id,
                                                       uint16 meta_channel_id);
/**
* \brief  Response to local to set the activated flag in the kip state
*         with an existing metadata data channel
*/
extern void stream_kip_metadata_channel_activated_resp_handler(
                                                       unsigned packed_con_id,
                                                       unsigned status,
                                                       uint16 meta_channel_id);

/**
 * \brief   Iterates through sink and source endpoints and if it find the buffer
 *          is connected to a shadow endpoint returns the associated IPC channel
 *          buffer.
 * \param   pointer to the buffer
 * \return  associated IPC channel buffer or NULL if not shadow endpoints found.
 */
tCbuffer* stream_kip_return_metadata_buf_from_buf(tCbuffer* buffer);

#endif /* INSTALL_METADATA_DUALCORE */


#endif /* STREAM_KIP_H */

