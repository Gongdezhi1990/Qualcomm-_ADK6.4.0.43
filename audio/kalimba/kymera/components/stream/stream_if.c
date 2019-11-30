/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_if.c
 * \ingroup stream
 *
 * stream interface file. <br>
 * This file is the interface to the outside world. <br>
 * Whoever that maybe......<br>
 *
 * \section sec1 Contains:
 * stream_if_get_endpoint <br>
 * stream_if_close_endpoint <br>
 * stream_if_configure_sid <br>
 * stream_if_sync_sids <br>
 * stream_if_connect <br>
 * stream_if_transform_connect <br>
 * stream_if_transform_disconnect <br>
 * stream_if_disconnect <br>
 * stream_if_get_source_from_sink <br>
 * stream_if_get_sink_from_source <br>
 *
 *
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "stream_kip.h"
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */

#if defined(INSTALL_UNINTERRUPTABLE_ANC) && defined(INSTALL_ANC_STICKY_ENDPOINTS)
#include "accmd_prim.h"
#endif /* defined(INSTALL_UNINTERRUPTABLE_ANC) && defined(INSTALL_ANC_STICKY_ENDPOINTS) */

/****************************************************************************
Private Type Declarations
*/

/* Structure for holding creation information for deferred operation */
typedef struct
{
    ENDPOINT *ep;
    unsigned con_id;
    bool (*callback)(unsigned con_id, unsigned status, unsigned source_id);
} ep_create_info_struct;

#ifdef INSTALL_MCLK_SUPPORT
/* Structure for holding information for deferred
 * mclk activation operation. The mclk activation
 * will only be deferred if the mclk isn't available
 * at the time of request and has to be claimed first
 * (from curator).
 */
typedef struct
{
    ENDPOINT *ep;
    unsigned con_id;
    bool (*callback)(unsigned con_id, unsigned status);
} ep_mclk_activate_info_struct;
#endif /* #ifdef INSTALL_MCLK_SUPPORT */


/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/


/****************************************************************************
Private Variable Definitions
*/

/* Info about a deferred endpoint creation
 * for now assume only one can be in progress at a time
 */
static ep_create_info_struct *ep_create_info = NULL;

#ifdef INSTALL_MCLK_SUPPORT
/* Info about a deferred mclk activation. Only one
 * request can be processed at a time.
 */
static ep_mclk_activate_info_struct *ep_mclk_activate_info = NULL;
#endif
/* Sampling rate at which the streams in the system are
 * configured to work
 */
static uint32 system_stream_rate = STREAM_AUDIO_SAMPLE_RATE_48K;

/* Kick period at which the streams in the system are
 * configured to work
 */
#if defined(INSTALL_MIB) && !defined(UNIT_TEST_BUILD) && !defined(DESKTOP_TEST_BUILD)
/* Value will be read from MIB on first use */
#define INITIAL_KICK_PERIOD  0
#else
#define INITIAL_KICK_PERIOD  DEFAULT_KICK_PERIOD_IN_USECS
#endif

static TIME_INTERVAL system_kick_period = (TIME_INTERVAL)INITIAL_KICK_PERIOD;

/****************************************************************************
Private Function Declarations
*/


/****************************************************************************
Public Function Definitions
*/

void stream_if_ep_creation_complete(ENDPOINT *ep, bool status)
{
    if ((ep_create_info == NULL) ||
        (ep_create_info->ep != ep))
    {
        /* This isn't the endpoint you're looking for... */
        panic_diatribe(PANIC_AUDIO_ENDPOINT_CREATION_SEQUENCE_ERROR, (DIATRIBE_TYPE)((uintptr_t)ep));
    }
    else if(!status)
    {
        /* Create failed. Send back command failed and clean up the endpoint */
        ep_create_info->callback(REVERSE_CONNECTION_ID(ep_create_info->con_id),
                STATUS_CMD_FAILED, 0);
        stream_close_endpoint(ep);
    }
    else
    {
        /* Fire off the callback and tidy up */
        ep_create_info->callback(REVERSE_CONNECTION_ID(ep_create_info->con_id),
                                 STATUS_OK, stream_external_id_from_endpoint(ep));
    }
    pdelete(ep_create_info);
    ep_create_info = NULL;
}

/****************************************************************************
 *
 * stream_if_get_endpoint
 *
 */
void stream_if_get_endpoint(unsigned con_id, unsigned device, unsigned num_params,
        unsigned *params, ENDPOINT_DIRECTION dir,
        bool (*callback)(unsigned con_id, unsigned status, unsigned source_id))
{
    unsigned status = STATUS_CMD_FAILED;
    ENDPOINT *ep = NULL;
    bool pending = FALSE;
    patch_fn_shared(stream_if);

    /* TODO MULTICORE: If P0 Stream imposes EP ID, on P1 receiving an extra param with EP ID. Pass that EP ID
     * down as extra param to the new endpoint creation macros (and the function underneath them).
     * If on P0, it would not get this extra param - but if client can impose where to create EP,
     * then adaptor may fish out extra param with processor ID - and by the time we are here, con_id will
     * say that it is to be 'remote' EP.
     *
     */

    /* Right then this is the first interface that is going to get called */
    switch (device)
    {
        case STREAM_DEVICE_PCM:
        case STREAM_DEVICE_I2S:
        case STREAM_DEVICE_APPDATA:
#ifdef INSTALL_CODEC
        case STREAM_DEVICE_CODEC:
#endif
#ifdef INSTALL_DIGITAL_MIC
        case STREAM_DEVICE_DIGITAL_MIC:
#endif
#ifdef INSTALL_AUDIO_INTERFACE_PWM
        case STREAM_DEVICE_PWM:
#endif
            ep = stream_audio_get_endpoint(con_id, dir, device, num_params, params, &pending);
            break;

#if defined(INSTALL_SPDIF) || defined(INSTALL_AUDIO_INTERFACE_SPDIF)
        case STREAM_DEVICE_SPDIF:
        {
            if(SINK == dir)
            {
                /* For S/PDIF tx always use generic audio endpoint type */
                ep = stream_audio_get_endpoint(con_id, dir, device, num_params, params, &pending);
            }
            else
            {

#ifdef INSTALL_SPDIF
                /* INSTALL_SPDIF means that all the components required for full handling
                 * of SPDIF input included, so use the spdif endpoint type*/
                ep = stream_spdif_get_endpoint(con_id, dir, num_params, params, &pending);
#else
                /* if INSTALL_SPDIF isn't defined use generic audio endpoint */
                ep = stream_audio_get_endpoint(con_id, dir, device, num_params, params, &pending);
#endif /* #ifdef INSTALL_SPDIF */
            }
        }
        break;
#endif /* #if defined(INSTALL_SPDIF) || defined(INSTALL_AUDIO_INTERFACE_SPDIF) */
    case STREAM_DEVICE_OPERATOR:
            if (num_params == 2)
            {
#if defined(INSTALL_DUAL_CORE_SUPPORT)
                IPC_PROCESSOR_ID_NUM proc_id;
                if ((opmgr_get_processor_id_from_opid(params[0], &proc_id)) &&
                    (!KIP_ON_SAME_CORE(proc_id)))
                {
                    uint16 packed_con_id = PACK_CONID_PROCID(con_id, proc_id);

                    /* Ask the other core. P0 keeps opid's for both P0 and P1, */
                    /* P1 only keeps opid's for P1. So P0 forwards this req to */
                    /* P1, but P1 should never forwards this req to P0.        */
                    stream_kip_operator_get_endpoint(params[0], params[1], dir, packed_con_id, callback);
                    return;
                }
                else
#endif
                {
                    ep = stream_operator_get_endpoint(params[0], params[1], dir, con_id);
                }
            }
            else
            {
                status = STATUS_INVALID_CMD_LENGTH;
            }
            break;
#ifdef INSTALL_SCO
        case STREAM_DEVICE_SCO:
            ep = stream_sco_get_endpoint(con_id, dir, num_params, params);
            break;
#endif /* INSTALL_SCO */
#if defined (INSTALL_FILE) && (!defined (CHIP_BASE_HYDRA))
        case STREAM_DEVICE_FILE:
            ep = stream_file_get_endpoint(con_id, dir, num_params, params);
            break;
#endif /* INSTALL_FILE && (!defined (CHIP_BASE_HYDRA)) */
#if defined (INSTALL_SIMULATION_FILE) && (!defined (CHIP_BASE_HYDRA))
        case STREAM_DEVICE_SIMULATION_FILE:
            ep = stream_sim_file_get_endpoint(con_id, dir, num_params, params);
            break;
#endif /*INSTALL_SIMULATION_FILE && (!defined(CHIP_BASE_HYDRA)) */
#ifdef INSTALL_RAW_BUFFER
        case STREAM_DEVICE_RAW_BUFFER:
            ep = stream_raw_buffer_get_endpoint(con_id, params[0]);
            break;
#endif /* INSTALL_RAW_BUFFER */
#if defined (INSTALL_A2DP) && defined (CHIP_BASE_BC7)
        case STREAM_DEVICE_L2CAP:
            /* Kymera assumes that any L2CAP connection has A2DP traffic
             * (since it has no information to go on), and wraps it in an
             * 'a2dp' endpoint, which for instance triggers A2DP-style
             * rate-matching for the sink.
             * This wouldn't necessarily be the right thing for non-A2DP-like
             * uses of L2CAP, but so far we haven't had any of those.
             * FIXME: some way for applications to override this */
            ep = stream_a2dp_get_endpoint(con_id, dir, num_params, params);
            break;
#endif /* INSTALL_A2DP && CHIP_BASE_BC7 */
#ifdef INSTALL_SHUNT
        case STREAM_DEVICE_SHUNT:
            ep = stream_shunt_get_endpoint(con_id, dir, num_params, params);
            break;
#endif /* INSTALL_SHUNT */
    }

    if (ep)
    {
        set_system_event(SYS_EVENT_REAL_EP_CREATE);
        /* Check that the endpoint is tagged with this user - only look at "owner", i.e. what is sender ID */
        /* in the connection ID. If matches, send a response and make sure connection ID is reversed. */
#ifdef CRESCENDO_TODO_DUAL_CORE
        /* TODO */
        if(GET_SEND_RECV_ID_CLIENT_ID(GET_CON_ID_SEND_ID(ep->con_id)) == GET_SEND_RECV_ID_CLIENT_ID(GET_CON_ID_SEND_ID(con_id)))
#else
        if(GET_CON_ID_SEND_ID(ep->con_id) == GET_CON_ID_SEND_ID(con_id))
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
        {
            if (pending)
            {
                /* The endpoint returned a pending state, so we have to now wait for
                 * it to call the completion callback before signalling to the client
                 */
                if (ep_create_info != NULL)
                {
                    /* Should only have one in progress...? */
                    panic_diatribe(PANIC_AUDIO_ENDPOINT_CREATION_SEQUENCE_ERROR, (DIATRIBE_TYPE)((uintptr_t)(ep_create_info->ep)));
                }
                else
                {
                    /* Allocate some data for the deferred-creation context
                     * We'll panic if this fails, but it's a small structure
                     * and the endpoint is going to call the callback anyway,
                     * so we can't do without it...
                     */
                    ep_create_info = pnew(ep_create_info_struct);
                    ep_create_info->callback = callback;
                    ep_create_info->con_id = con_id;
                    ep_create_info->ep = ep;
                }
            }
            else
            {
                status = STATUS_OK;
                callback(REVERSE_CONNECTION_ID(con_id), status, stream_external_id_from_endpoint(ep));
            }
            return;
        }
    }

    callback(REVERSE_CONNECTION_ID(con_id), status, 0);
}

/****************************************************************************
 *
 * stream_if_close_endpoint
 *
 */
void stream_if_close_endpoint(unsigned con_id, unsigned endpoint_id,
        bool (*callback)(unsigned con_id, unsigned status))
{
    unsigned status = STATUS_CMD_FAILED;

    patch_fn_shared(stream_if);

#if defined(INSTALL_UNINTERRUPTABLE_ANC) && defined(INSTALL_ANC_STICKY_ENDPOINTS)

    ENDPOINT *endpoint;
    ACCMD_ANC_INSTANCE instance_id;

    /* Check if the endpoint is being used by ANC
     * if it is then don't close it but return a successful status
     */
    endpoint = stream_endpoint_from_extern_id(endpoint_id);

    /* Check if the endpoint exists */
    if (endpoint != NULL)
    {
        instance_id = get_anc_instance_id(endpoint);
        if (instance_id != ACCMD_ANC_INSTANCE_NONE_ID)
        {
            /* ANC is using the endpoint so don't actually do the close */
            status = STATUS_OK;

            /* Indicate that ANC must close the endpoint when tearing down ANC */
            set_anc_close_pending(endpoint, TRUE);
        }
        else
        {
            /* TODO MULTICORE: based on con_id, act on P0 for local endpoint,
             * or delegate to secondary processor.
             */
            if (stream_close_endpoint(stream_endpoint_from_extern_id(endpoint_id)))
            {
                status = STATUS_OK;
            }
        }
    }
#else /* defined(INSTALL_UNINTERRUPTABLE_ANC) && defined(INSTALL_ANC_STICKY_ENDPOINTS) */
    {
        /* TODO MULTICORE: based on con_id, act on P0 for local endpoint,
         * or delegate to secondary processor.
         */
        if (stream_close_endpoint(stream_endpoint_from_extern_id(endpoint_id)))
        {
            status = STATUS_OK;
        }
    }
#endif /* defined(INSTALL_UNINTERRUPTABLE_ANC) && defined(INSTALL_ANC_STICKY_ENDPOINTS) */


    callback(REVERSE_CONNECTION_ID(con_id), status);
}

/****************************************************************************
 *
 * stream_if_configure_sid
 *
 */
void stream_if_configure_sid(unsigned con_id, unsigned ep_id, unsigned int key, uint32 value,
        bool (*callback)(unsigned con_id, unsigned status))
{
    unsigned status = STATUS_CMD_FAILED;

    /* TODO MULTICORE: based on con_id, act on P0 for local endpoint,
     * or delegate to secondary processor.
     */
    ENDPOINT *ep = stream_endpoint_from_extern_id(ep_id);

    if (ep ? ep->functions->configure(ep, key, value) : FALSE)
    {
        status = STATUS_OK;
    }

    callback(REVERSE_CONNECTION_ID(con_id), status);
}

#ifdef INSTALL_MCLK_SUPPORT
/****************************************************************************
 *
 * stream_if_mclk_activate
 *
 */
void stream_if_mclk_activate(unsigned con_id, unsigned ep_id, unsigned activate_output,
                             unsigned enable_mclk, bool (*callback)(unsigned con_id, unsigned status))
{
    unsigned status = STATUS_CMD_FAILED;
    ENDPOINT *ep = stream_endpoint_from_extern_id(ep_id);
    patch_fn_shared(stream_if);

    if(ep && IS_AUDIO_ENDPOINT(ep))
    {
        bool pending = FALSE;
        if(stream_audio_activate_mclk(ep, activate_output, enable_mclk, &pending))
        {
            if(pending)
            {
                if (ep_mclk_activate_info != NULL)
                {
                    /* Should only have one in progress...? */
                    panic_diatribe(PANIC_AUDIO_MCLK_ACTIVATION_SEQUENCE_ERROR, (DIATRIBE_TYPE)((uintptr_t)ep));
                }
                else
                {
                    /* Allocate some data for the deferred-mclk_enabling context
                     * We'll panic if this fails, but it's a small structure
                     * and the endpoint is going to call the callback anyway,
                     * so we can't do without it...
                     */
                    ep_mclk_activate_info = pnew(ep_mclk_activate_info_struct);
                    ep_mclk_activate_info->callback = callback;
                    ep_mclk_activate_info->con_id = con_id;
                    ep_mclk_activate_info->ep = ep;
                }
                return;
            }
            else
            {
                status = STATUS_OK;
            }
        }
    }
    callback(REVERSE_CONNECTION_ID(con_id), status);
}

/****************************************************************************
 *
 * stream_if_ep_mclk_activate_complete
 *
 */
void stream_if_ep_mclk_activate_complete(ENDPOINT *ep, bool status)
{
    if ((ep_mclk_activate_info == NULL) ||
        (ep_mclk_activate_info->ep != ep))
    {
        /* This isn't the endpoint you're looking for... */
        panic_diatribe(PANIC_AUDIO_MCLK_ACTIVATION_SEQUENCE_ERROR, (DIATRIBE_TYPE)((uintptr_t)ep));
    }
    else if(!status)
    {
        /* We couldn't activate mclk. Send back failure */
        ep_mclk_activate_info->callback(REVERSE_CONNECTION_ID(ep_mclk_activate_info->con_id),
                STATUS_CMD_FAILED);
        /* TODO:any further action here? */
    }
    else
    {
        /* activation successful, report success */
        ep_mclk_activate_info->callback(REVERSE_CONNECTION_ID(ep_mclk_activate_info->con_id),
                                 STATUS_OK);
    }
    pdelete(ep_mclk_activate_info);
    ep_mclk_activate_info = NULL;
}

/****************************************************************************
 *
 * stream_if_set_mclk_source
 *
 */
void stream_if_set_mclk_source(unsigned con_id, unsigned use_external_mclk,
                            uint32 external_mclk_freq, bool (*callback)(unsigned con_id, unsigned status))
{
    unsigned status = STATUS_OK;

    /* tell the mclk manager to use external mclk or local mpll
     * when using external mclk, it's frequency also needs to be supplied
     */
    if(!audio_mclk_mgr_use_external_mclk(use_external_mclk, external_mclk_freq))
    {
        /* for some reason it couldn't switch to new mclk source
         * this might be for reason like mclk is in-use currently
         */
        status = STATUS_CMD_FAILED;
    }
    callback(REVERSE_CONNECTION_ID(con_id), status);
}

#endif /* #ifdef INSTALL_MCLK_SUPPORT */

/****************************************************************************
 *
 * stream_if_sync_sids
 *
 */
void stream_if_sync_sids(unsigned con_id, unsigned ep_id1, unsigned ep_id2,
        bool (*callback)(unsigned con_id, unsigned status))

{
    unsigned status = STATUS_OK;
    ENDPOINT *ep1 = stream_endpoint_from_extern_id(ep_id1);
    ENDPOINT *ep2 = stream_endpoint_from_extern_id(ep_id2);
    patch_fn_shared(stream_if);

    /* TODO MULTICORE: based on con_id, act on P0 for local endpoint,
     * or delegate to secondary processor. Only EPs local to a processor
     * can be sync'ed.
     */

    /* Endpoint used for rate matching comparison*/
    ENDPOINT *r_ep = NULL;

    if(ep1 == NULL)
    {
        if ( (ep2 != NULL) && IS_AUDIO_ENDPOINT(ep2))
        {
            /* Swap ep1 with ep2. Note: ep1 is null */
            ep1 = ep2;
            ep2 = NULL;
        }
        else
        {
            /* If both of them are null or ep2 is not an audio endpoint
             * then return command fail. */
            status = STATUS_CMD_FAILED;
        }
    }
    else if (IS_AUDIO_ENDPOINT(ep1))
    {
        if (ep2 && !IS_AUDIO_ENDPOINT(ep2))
        {
            /* ep2 is not audio return command fail. */
            status = STATUS_CMD_FAILED;
        }
    }
    else
    {
        /* ep1 is not audio return command fail. */
        status = STATUS_CMD_FAILED;
    }

    if(ep2 == NULL)
    {
        /* If ep2 is null we are going to desynchronise so we need to cache
         * the other endpoint in the pair so we can rebuild rate matching */
        if(IS_ENDPOINT_HEAD_OF_SYNC(ep1))
        {
            /* If the endpoint is the head of sync we want to cache the nep
             * in the temporary endpoint for when we rebuild rate matching */
            r_ep = ep1->state.audio.nep_in_sync;
        }
        else
        {
            /* We want to cache the head ep to rebuild with rate matching*/
            r_ep = ep1->state.audio.head_of_sync;
        }
    }
    else
    {
        r_ep = ep2;
    }
#ifdef INSTALL_SPDIF
    /* allow two spidf input streams
     * to get syncronised, this is releavant only
     * in two-channel config
     */
    if(STATUS_OK!=status
       && (ep1 != NULL) && IS_SPDIF_ENDPOINT(ep1)
       && (ep2 != NULL) && IS_SPDIF_ENDPOINT(ep2))
    {
        status = STATUS_OK;
    }
#endif /* #ifdef INSTALL_SPDIF */

    /* If status is still OK then synchronise the endpoints. (ep1 is never null)
     * If the synchronisation failed return command fail. */
    if(status ==STATUS_OK)
    {
        if(!ep1->functions->sync(ep1, ep2))
        {
            status = STATUS_CMD_FAILED;
        }
        else
        {
            /* When use is made of the sync-group information available in Bluecore
             * at the point the endpoint is created. There will no longer be a
             * need to fiddle with ratematching during synchronisation.
             */
            if (r_ep != NULL)
            {
#ifndef A7DA_KAS_DISABLE_STREAM_RM
                /* First we remove any pairs that involve either endpoint*/
                cease_ratematching(ep1->id);
                cease_ratematching(r_ep->id);

                /* Then rebuild. The rate matching code should recognise the difference
                * that we either have a single or split rate matching entity.  */
                if (!setup_ratematching(ep1->id))
                {
                    status = STATUS_CMD_FAILED;
                }
                if (!setup_ratematching(r_ep->id))
                {
                    status = STATUS_CMD_FAILED;
                }
#endif /* A7DA_KAS_DISABLE_STREAM_RM */
            }
        }
    }

    callback(REVERSE_CONNECTION_ID(con_id), status);
}


/****************************************************************************
 *
 * stream_if_transform_connect
 *
 * Processor independent stream_connect request. state_info must be NULL
 * if the operator endpoints are not already created. For single core
 * stream connection, state_info must be NULL.
 *
 */
void stream_if_transform_connect( unsigned con_id,  unsigned source_id,
                                  unsigned sink_id, unsigned transform_id,
                                  STREAM_CONNECT_INFO *state_info,
                                  bool (*callback)( unsigned con_id,
                                  unsigned status,  unsigned transform_id))
{
    unsigned transform = 0;
    bool status = TRUE;
    STREAM_CONNECT_INFO info;
    patch_fn_shared(stream_if);
    /*
     * Whenever this function gets called, get the connect information and
     * decide the starting state of the connect process. If sink and source
     * endpoints are with  either P0 and/or P1, all the states will be executed.
     *
     * Connect can start in stage 3 or in stage 1 depending upon the operator
     * locations.
     *
     * 1. Creating the endpoints if they don't exist
     * 2. Negotiating the buffer details and data format
     * 3. Establishing the transform and updating the kick information
     */

    ENDPOINT *source_ep = stream_endpoint_from_extern_id(source_id);
    ENDPOINT *sink_ep   = stream_endpoint_from_extern_id(sink_id);

    if (source_ep == NULL)
    {
        source_ep = stream_create_endpoint(source_id, con_id);
    }

    if (sink_ep == NULL)
    {
        sink_ep = stream_create_endpoint(sink_id, con_id);
    }

    if (state_info == NULL)
    {
        status = stream_connect_get_buffer(source_ep, sink_ep, &info);
        state_info = &info;
    }

    /* stage 3 - Establish transform
     * On P0 (as well as on a single core), this transform id will be always 0.
     * On P1, the transform id must not be 0. This must be validated before
     * coming here.
     */
    if( status )
    {
        unsigned id = STREAM_TRANSFORM_GET_INT_ID( transform_id);

        transform = stream_external_id_from_transform(stream_connect_endpoints(
                                                      source_ep, sink_ep,
                                                      state_info, id));
        if( transform == 0 )
        {
            status = FALSE;
        }
    }

    if(!status)
    {
#ifndef CRESCENDO_TODO_DUAL_CORE
        /* TODO - For dual core, we need to deactivate the data channel */
#endif
        stream_destroy_endpoint_id(source_id);
        stream_destroy_endpoint_id(sink_id);
    }

    callback( REVERSE_CONNECTION_ID(con_id),
              status? STATUS_OK: STATUS_CMD_FAILED,
              transform);
}



/****************************************************************************
 *
 * stream_if_connect
 *
 * This API is being called only in P0. This is not expected to be called on Pn.
 */
void stream_if_connect(unsigned con_id, unsigned source_id, unsigned sink_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned transform_id))
{
    patch_fn_shared(stream_if);

#if defined(INSTALL_DUAL_CORE_SUPPORT)
    STREAM_EP_LOCATION ep_location = STREAM_EP_REMOTE_NONE;
    IPC_PROCESSOR_ID_NUM remote_processor_id = IPC_PROCESSOR_0;

    /*
     * If dual core support is enabled, the sink and/or the source may be located
     * on a remote processor other than P0.
     *
     *  - If both sink and source are operator eps and not on P0:
     *          - Generate a transform id for remote use
     *          - Send kip_stream_connect_request_req to Pn
     *          - Store the context and call back
     *
     *  - else If either sink or source is an operator ep on Pn
     *          - If other ep is not an audio real endpoint when delegating
     *            audio endpoint is supported.
     *              - Derive the shadow endpoint id for remote and local
     *              - if source is local, create a data channel
     *              - create local endpoints & get the endpoint details(buffer)
     *           - else ( delegating audio endpoint is supported)
     *              - get the real audio endpoint details
     *
     *           - Send kip_create_endpoint_req() to the remote
     *           - Store the context and callback
     *
     *   - else (everything on P0)
     *          - Create local endpoints and get the endpoint details (buffer)
     *          - Call stream_if_transform_connect() and proceed with local connection.
     */

    /* Figure out whether the provided source endpoint is not a local one */
    if ( STREAM_EP_IS_OPEP_ID(source_id) )
    {
        IPC_PROCESSOR_ID_NUM proc_id;

        /*
         * There must be an operator associated with this id, if it is not,
         * it will return failure while attempting to create the endpoints.
         */
        if( opmgr_get_processor_id( source_id, &proc_id) &&
            (!KIP_ON_SAME_CORE(proc_id)))
        {
            remote_processor_id = proc_id;
            ep_location |= STREAM_EP_REMOTE_SOURCE;
        }
    }

    /* figure out whether the provided sink endpoint is not a local one */
    if ( STREAM_EP_IS_OPEP_ID(sink_id) )
    {
        IPC_PROCESSOR_ID_NUM proc_id;

        /*
         * There must be an operator associated with this id, if it is not,
         * it will return failure while attempting to create the endpoints.
         */
        if( opmgr_get_processor_id( sink_id, &proc_id) &&
            (!KIP_ON_SAME_CORE(proc_id)))
        {
            if( KIP_PRIMARY_CORE_ID(remote_processor_id))
            {
                remote_processor_id = proc_id;
            }
            else if( remote_processor_id != proc_id )
            {
                /* Connecting operators running on two different
                 * secondary cores are not supported. For
                 * dual core, it should not happen!
                 */
                callback(REVERSE_CONNECTION_ID(con_id), STATUS_CMD_FAILED, 0);
                return;
            }

            /* sink is also with secondary core */
            ep_location |= STREAM_EP_REMOTE_SINK;
        }
    }

    /*
     * If none of the operator endpoints are located in Pn,
     * just proceed with the single core scenario
     */
    if( ep_location != STREAM_EP_REMOTE_NONE )
    {
        /* create a state information */
        STREAM_KIP_CONNECT_INFO *state_info;
        unsigned remote_source_id = source_id;
        unsigned remote_sink_id = sink_id;
        unsigned con_proc_id;
        bool result;

        /* Generate the KIP shadow endpoint ids
         * If the source operator endpoint location is remote, then remote sink
         * will be the shadow of local sink and local source will be the shadow
         * of remote source.
         */
        if( ep_location == STREAM_EP_REMOTE_SOURCE )
        {
            source_id = STREAM_GET_SHADOW_EP_ID(source_id);
            remote_sink_id = STREAM_GET_SHADOW_EP_ID(sink_id);
        }
        else if( ep_location == STREAM_EP_REMOTE_SINK)
        {
            sink_id = STREAM_GET_SHADOW_EP_ID(sink_id);
            remote_source_id = STREAM_GET_SHADOW_EP_ID(source_id);
        }

        /* pack con id and processor id for kip transactions */
        con_proc_id =  PACK_CONID_PROCID(con_id, remote_processor_id);

        /* create the connect state record only for connection */
        state_info = stream_kip_create_connect_info_record( con_proc_id,
                                                    source_id, sink_id,
                                                ep_location, callback );

        if( state_info == NULL)
        {
            callback(REVERSE_CONNECTION_ID(con_id), STATUS_CMD_FAILED, 0);
            return;
        }

        if (ep_location == STREAM_EP_REMOTE_ALL)
        {

            result = stream_kip_connect_endpoints( con_proc_id,
                                                   remote_source_id,
                                                   remote_sink_id,
                                                   state_info );
        }
        else
        {
            /* Atleast one operator endpoint is in Pn. Create local
             * endpoints and remote endpoints.
             * buffer details must be presented to the remote.
             */
            result = stream_kip_create_endpoints( con_proc_id,
                                                  remote_source_id,
                                                  remote_sink_id,
                                                  state_info);
        }

        if(!result)
        {
            pdelete( state_info );
            callback(REVERSE_CONNECTION_ID(con_id), STATUS_CMD_FAILED, 0);
        }

        /* Done with dual core scenario */
        return;
    }

#endif /* INSTALL_DUAL_CORE_SUPPORT */

    /* Comes here only when stream connect happens only at P0 */
    stream_if_transform_connect( con_id, source_id, sink_id, 0, NULL, callback);
}

/****************************************************************************
 *
 * stream_if_part_transform_disconnect
 *
 * This is the common function for P0 and P1 to disconnect the transform.
 * This provides a count of already disconnected transforms for reporting
 * in the callback.
 * stream_if_transform_disconnect is the P0 only interface function
 * being called by ACCMD.
 */
void stream_if_part_transform_disconnect( unsigned con_id, unsigned count,
                                                unsigned *transforms,
                                                unsigned success_count,
                        bool (*callback)(unsigned con_id, unsigned status, unsigned count))
{
    unsigned i;
    TRANSFORM *tfm;
    unsigned status = STATUS_OK;
    patch_fn_shared(stream_if);

    for (i = 0; i < count; i++)
    {
        /* Get the endpoints before the disconnect. If they are operator
         * endpoints they need to be destroyed here. */
        tfm = stream_transform_from_external_id(transforms[i]);

        if (tfm == NULL)
        {
            status = STATUS_CMD_FAILED;
            break;
        }

        if (!stream_transform_disconnect(tfm))
        {
            status = STATUS_CMD_FAILED;
            break;
        }
    }

    callback(REVERSE_CONNECTION_ID(con_id), status, i + success_count );
}

/****************************************************************************
 *
 * stream_if_transform_disconnect
 *
 * ACCMD calls this API . If dual core enabled, it filters P1 list first
 * before calling stream_if_local_transform_disconnect
 *
 */
void stream_if_transform_disconnect(unsigned con_id, unsigned count,
                                    unsigned *transforms,
        bool (*callback)(unsigned con_id, unsigned status, unsigned count))
{
    patch_fn_shared(stream_if);
#if defined(INSTALL_DUAL_CORE_SUPPORT)
    unsigned px_tr_count;

    /* Find the first P1 transform in the list */
    px_tr_count = stream_kip_find_px_transform_start( count, transforms );

    /* If there are no P1 transforms in the list, px_tr_count returns count */
    if( px_tr_count < count)
    {
        STREAM_KIP_TRANSFORM_DISCONNECT_INFO *state_info;
        STREAM_KIP_TRANSFORM_INFO *tr;
        unsigned con_proc_id;
        STREAM_KIP_TRANSFORM_DISCONNECT_CB cb;

        tr = stream_kip_transform_info_from_id(
                                   STREAM_TRANSFORM_GET_INT_ID(transforms[0]));

        /* tr will not be NULL if we are inside this if condition.
         * so not validating tr */
        con_proc_id =  PACK_CONID_PROCID(con_id, tr->processor_id);


        cb.tr_disc_cb = callback;
        /* Create a stream disconnect state record for disconnection */
        state_info = stream_kip_create_disconnect_info_record( con_proc_id,
                                                               count,
                                                               FALSE,
                                                               transforms,
                                                               cb );

        if( state_info == NULL ||
            !stream_kip_transform_disconnect( state_info, px_tr_count ) )
        {
            callback(REVERSE_CONNECTION_ID(con_id), STATUS_CMD_FAILED, 0);
        }

        return;
    }
#endif  /* INSTALL_DUAL_CORE_SUPPORT */

    /* It came here only because there is no P1 transforms in the list */
    stream_if_part_transform_disconnect( con_id, count, transforms,
                                         0 , callback );
}

/****************************************************************************
 *
 * stream_if_disconnect
 *
 */
void stream_if_disconnect(unsigned con_id, unsigned source_id, unsigned sink_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned transform_id_source, unsigned transform_id_sink))
{
    unsigned status = STATUS_OK;
    unsigned tfid_sink = 0;
    unsigned tfid_source = 0;
    patch_fn_shared(stream_if);

#ifdef AUDIO_SECOND_CORE
    /* This API must be called only at P0 */
    if( KIP_SECONDARY_CONTEXT())
    {
        callback(REVERSE_CONNECTION_ID(con_id), STATUS_INVALID_CMD_PARAMS, 0, 0);
        return;
    }
#endif

    if ((sink_id == 0) && (source_id == 0))
    {
        callback(REVERSE_CONNECTION_ID(con_id), STATUS_INVALID_CMD_PARAMS, 0, 0);
        return;
    }

#ifdef INSTALL_DUAL_CORE_SUPPORT
    if( KIP_PRIMARY_CONTEXT())
    {
        STREAM_KIP_TRANSFORM_INFO *tr = NULL;
        unsigned sink_trid=0, source_trid=0;
        unsigned transforms[2];
        unsigned count = 0;

        /* get the remote source transform id if exits */
        tr = stream_kip_transform_info_from_epid( source_id );
        if( tr != NULL )
        {
            source_trid = STREAM_TRANSFORM_GET_EXT_ID( tr ->id );
        }

        /* get the remote source transform id if exits */
        tr = stream_kip_transform_info_from_epid( sink_id );
        if( tr != NULL )
        {
            sink_trid = STREAM_TRANSFORM_GET_EXT_ID( tr ->id );
        }

        /* send the request to KIP if atleast one transform present */
        if( (source_trid != 0) || (sink_trid != 0))
        {
            STREAM_KIP_TRANSFORM_DISCONNECT_INFO *state_info;
            STREAM_KIP_TRANSFORM_DISCONNECT_CB cb;

            unsigned con_proc_id, px_tr_count = 0;
            unsigned remote_trid = (source_trid == 0)? sink_trid: source_trid;

            tr = stream_kip_transform_info_from_id(
                                   STREAM_TRANSFORM_GET_INT_ID(remote_trid));

            /* tr will not be NULL if we are inside this if condition.
             * so not validating tr */
            con_proc_id =  PACK_CONID_PROCID(con_id, tr->processor_id);

            if( source_trid == 0 )
            {
                struct ENDPOINT *source = stream_endpoint_from_extern_id(source_id);

                /* find the source id in the local transform
                 * and add to the list even if it is a local
                 * transform.
                 */
                if(( source != NULL) && (source->connected_to != NULL))
                {
                    source_trid = stream_external_id_from_transform(
                                  stream_transform_from_endpoint(source));

                    if(STREAM_EP_IS_SHADOW( source->connected_to))
                    {
                        px_tr_count = 1;
                    }
                }
            }

            if( sink_trid == 0 )
            {
                struct ENDPOINT *sink = stream_endpoint_from_extern_id(sink_id);

                /* find the source id in the local transform
                 * and add to the list even if it is a local
                 * transform.
                 */
                if(( sink != NULL) && (sink->connected_to != NULL))
                {
                    sink_trid = stream_external_id_from_transform(
                                  stream_transform_from_endpoint(sink));
                }
            }

            transforms[0] = source_trid;
            transforms[1] = sink_trid;
            count = 2;
            cb.disc_cb = callback;

            /* Create a stream disconnect state record for disconnection */
            state_info = stream_kip_create_disconnect_info_record( con_proc_id,
                                                                   count,
                                                                   TRUE,
                                                                   transforms,
                                                                   cb);

            if (( source_trid == 0 ) || (source_trid == sink_trid))
            {
                /* skip it by marking it as success */
                state_info->success_count = 1;
            }


            if( state_info == NULL ||
                !stream_kip_transform_disconnect( state_info, px_tr_count ) )
            {
                callback(REVERSE_CONNECTION_ID(con_id), STATUS_CMD_FAILED, 0, 0);
            }

            return;
        }
    }

#endif /* INSTALL_DUAL_CORE_SUPPORT */

    /* No second core transforms involved. Just continue handling at
     * single core
     */

    if (sink_id != 0)
    {
        TRANSFORM *tfm = NULL;
        struct ENDPOINT *sink = stream_endpoint_from_extern_id(sink_id);

        if (sink == NULL)
        {
            status = STATUS_CMD_FAILED;
        }
        else
        /* If the sink_id exists and is connected to *something* ... */
        if (sink->connected_to != NULL)
        {
            tfm = stream_transform_from_endpoint(sink);
            tfid_sink = stream_external_id_from_transform(tfm);

            /* If the sink is connect to 'our' source... */
            if (sink->connected_to == stream_endpoint_from_extern_id(source_id))
            {
                tfid_source = tfid_sink;
            }

            if (!stream_transform_disconnect(tfm))
            {
                status = STATUS_CMD_FAILED;
            }
        }
        /* Valid sink that is not connected is not a fault */
    }

    if ((source_id != 0) && (tfid_source == 0))
    {
        TRANSFORM *tfm = NULL;
        struct ENDPOINT *source = stream_endpoint_from_extern_id(source_id);

        if (source == NULL)
        {
            status = STATUS_CMD_FAILED;
        }
        else
        /* If the source_id exists and is connected to 'other' sink... */
        if (source->connected_to != NULL)
        {
            /* Source id is not connected sink id, we handled that earlier on... */
            tfm = stream_transform_from_endpoint(source);
            tfid_source = stream_external_id_from_transform(tfm);

            if (!stream_transform_disconnect(tfm))
            {
                status = STATUS_CMD_FAILED;
            }
        }
        /* Valid source that is not connected is not a fault */
    }

    callback(REVERSE_CONNECTION_ID(con_id), status, tfid_source, tfid_sink);
}



/****************************************************************************
 *
 * stream_if_get_sink_from_source
 *
 */
void stream_if_get_sink_from_source(unsigned con_id, unsigned source_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned sink_id))
{
    unsigned sink_id = 0;
    unsigned status = STATUS_OK;
    struct ENDPOINT *sink_ep;
    struct ENDPOINT *source_ep = stream_endpoint_from_extern_id(source_id);
    patch_fn_shared(stream_if);
    if(source_ep == NULL)
    {
        status = STATUS_CMD_FAILED;
    }
    else
    {
        /*
        #ifndef TODO_CRESCENDO
        unsigned hw_type;
        hw_type=((source_ep->key)>>8)&0xff;
        if ((source_ep->stream_endpoint_type == endpoint_audio) && (hw_type == ACCMD_STREAM_DEVICE_DIGITAL_MIC))
        {
            callback(REVERSE_CONNECTION_ID(con_id), STATUS_CMD_FAILED,0);
            return;
        }
        #endif
        */
        sink_ep=stream_get_endpoint_from_key_and_functions(source_ep->key, SINK, source_ep->functions);
        if (sink_ep != NULL)
        {
            sink_id=stream_external_id_from_endpoint(sink_ep);
        }
        else
        {
            status = STATUS_CMD_FAILED;
        }
    }
    callback(REVERSE_CONNECTION_ID(con_id), status,sink_id);
    return;
}

/****************************************************************************
 *
 * stream_if_get_source_from_sink
 *
 */
void stream_if_get_source_from_sink(unsigned con_id, unsigned sink_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned source_id))
{
    unsigned source_id = 0;
    unsigned status = STATUS_OK;
    struct ENDPOINT *source_ep;
    struct ENDPOINT *sink_ep = stream_endpoint_from_extern_id(sink_id);
    patch_fn_shared(stream_if);
    if(sink_ep == NULL)
    {
        status = STATUS_CMD_FAILED;
    }
    else
    {
        source_ep=stream_get_endpoint_from_key_and_functions(sink_ep->key, SOURCE, sink_ep->functions);
        if (source_ep != NULL)
        {
            source_id=stream_external_id_from_endpoint(source_ep);
        }
        else
        {
            status = STATUS_CMD_FAILED;
        }
    }
    callback(REVERSE_CONNECTION_ID(con_id), status,source_id);
    return;
}

/****************************************************************************/
void stream_if_transform_from_ep(unsigned con_id, unsigned sid,
        bool (*callback)(unsigned con_id, unsigned status, unsigned tr_id))
{
    unsigned dest_con_id = REVERSE_CONNECTION_ID(con_id);
    patch_fn_shared(stream_if);

    if (sid == 0)
    {
        callback(dest_con_id, STATUS_INVALID_CMD_PARAMS, 0);
        return;
    }

#ifdef INSTALL_DUAL_CORE_SUPPORT
    if (KIP_PRIMARY_CONTEXT())
    {
        STREAM_KIP_TRANSFORM_INFO *tr;

        tr = stream_kip_transform_info_from_epid(sid);
        if (tr)
        {
            callback(dest_con_id, STATUS_OK,
                     STREAM_TRANSFORM_GET_EXT_ID(tr->id));
            return;
        }
    }
#ifdef AUDIO_SECOND_CORE
    else
#endif /* AUDIO_SECOND_CORE */
#endif /* INSTALL_DUAL_CORE_SUPPORT */
#ifdef AUDIO_SECOND_CORE
    {
        /* This API must be called only at P0 */
        callback(dest_con_id, STATUS_INVALID_CMD_PARAMS, 0);
        return;
    }
#endif /* AUDIO_SECOND_CORE */

    /* No second core transforms involved. */
    {
        struct ENDPOINT *ep = stream_endpoint_from_extern_id(sid);
        TRANSFORM *tfm;

        if (ep)
        {
            tfm = stream_transform_from_endpoint(ep);
        }
        else
        {
            /*
             * This could be a legitimate operator endpoint; these are
             * only created on connection. In that case we need to
             * return OK + transform 0, as if the endpoint exists but
             * is unconnected.
             * (This does mean that we also return 'OK' for completely
             * bogus endpoints. Oh well.)
             */
            tfm = NULL;
        }
        callback(dest_con_id, STATUS_OK,
                 stream_external_id_from_transform(tfm));
    }
}

/****************************************************************************
 *
 * stream_if_propagate_kick
 *
 * Passes on a kick from an endpoint to the thing it's connected to.
 * ep_id = endpoint which has generated a kick (identified by external ID).
 * Note the use of stream_endpoint_from_extern_id, which searches through
 * a list of endpoints. This process could be optimised if the API
 * took an ENDPOINT* directly.
 */
void stream_if_propagate_kick(unsigned ep_id)
{
    ENDPOINT *this_ep = stream_endpoint_from_extern_id(ep_id);
    ENDPOINT_KICK_DIRECTION kick_direction;
    patch_fn_shared(stream_if);

    PL_PRINT_P2(TR_STREAM, "stream_if_propagate_kick: ep_id 0x%x = endpoint %p\n", ep_id, this_ep);

    /* TODO MULTICORE: propagation between a local op EP and a shadow EP may happen. Latter needs to
     * act via KIP to reach other processor.
     */

    if (this_ep != NULL)
    {
        if (this_ep->direction == SOURCE)
        {
            kick_direction = STREAM_KICK_FORWARDS;
        }
        else
        {
            kick_direction = STREAM_KICK_BACKWARDS;
        }

        propagate_kick(this_ep, kick_direction);
    }
    else
    {
        /* Uh-oh. The caller must have passed in an iffy endpoint ID.
         * This should only be caused by a programming error, so let's panic. */
        panic_diatribe(PANIC_AUDIO_INVALID_KEY_OR_ENDPOINT_ID, ep_id);
    }
}

/****************************************************************************
 *
 * stream_if_kick_ep
 *
 */
void stream_if_kick_ep(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir)
{
    /* TODO MULTICORE: check for shadow EP case. */
    ep->functions->kick(ep, kick_dir);
}


/****************************************************************************
 *
 * stream_if_get_info
 *
 */
bool stream_if_get_info(unsigned id, unsigned key, uint32* value)
{
    ENDPOINT *ep = stream_endpoint_from_extern_id(id);

#ifdef STREAM_INFO_KEY_ENDPOINT_EXISTS
    if (key == STREAM_INFO_KEY_ENDPOINT_EXISTS)
    {
        /*
         * Special case: ENDPOINT_EXISTS doesn't need to be dispatched to
         * endpoint-specific code, and *does* need to return TRUE for
         * operator endpoints, even if they haven't yet been connected to
         * (and hence created by Kymera). The meaning of "exists" here
         * is "can potentially be connected to".
         * (Shadow endpoints also have the 'don't exist until connected
         * to' nature, but ACCMD clients won't be asking about those.)
         */
        if (ep)
        {
            /* Endpoint structure already exists, hence obviously
             * connectable. */
            return TRUE;
        }
        else if (STREAM_EP_IS_OPEP_ID(id) && opmgr_is_opidep_valid(id))
        {
            /* Currently unconnected operator terminal for which
             * stream_connect stands a chance of succeeding. */
            return TRUE;
        }
        else
        {
            /* No reason to believe that stream_connect will succeed
             * for this sid. */
            return FALSE;
        }
    }
#endif

    /*
     * For other info keys, the endpoint needs to actually exist as far
     * as Kymera is concerned to return any info.
     */
    if (ep == NULL)
    {
        return FALSE;
    }

    return stream_get_endpoint_config(ep, key, value);
}


/****************************************************************************
 *
 * stream_if_get_connection_list
 *
 */
void stream_if_get_connection_list(unsigned con_id, unsigned source_id, unsigned sink_id,
                                   bool (*callback)(unsigned con_id, unsigned status, unsigned length,
                                                    const unsigned* info_list))
{
    unsigned *conn_list;
    unsigned length;
    patch_fn_shared(stream_if);

    /* if the external ID is not zero, meaning no filtering, then make these into internal IDs */
    /* If an operator ID was received instead of endpoint, conversion will leave it intact */
    if(sink_id != 0)
    {
        TOGGLE_EP_ID_BETWEEN_INT_AND_EXT(sink_id);

        /* If an operator ID was sent, ensure all terminal bits are zeroed. If it is an endpoint,
         * it's superfluous but harmless - saves some conditional code. */
        sink_id &= (~STREAM_EP_CHAN_MASK);
    }

    if(source_id != 0)
    {
        TOGGLE_EP_ID_BETWEEN_INT_AND_EXT(source_id);
        source_id &= (~STREAM_EP_CHAN_MASK);
    }

    /* get the stuff */
    stream_get_connection_list(source_id, sink_id, &length, &conn_list);

    /* Pass info to callback with reversed connection ID */
    callback(REVERSE_CONNECTION_ID(con_id), 0, length, conn_list);
    pfree(conn_list);
}

/****************************************************************************
 *
 * stream_if_set_system_sampling_rate
 */
bool stream_if_set_system_sampling_rate(uint32 sampling_rate)
{
    if( sampling_rate >  STREAM_AUDIO_SAMPLE_RATE_96K || \
        sampling_rate <  STREAM_AUDIO_SAMPLE_RATE_8K )
    {
        return FALSE;
    }

    system_stream_rate = sampling_rate;
    return TRUE;
}

/****************************************************************************
 *
 * stream_if_get_system_sampling_rate
 */
uint32 stream_if_get_system_sampling_rate(void)
{
    return system_stream_rate;
}

/****************************************************************************
 *
 * stream_if_set_system_kick_period
 */
bool stream_if_set_system_kick_period(TIME_INTERVAL kp)
{
    if ((kp >= MIN_KICK_PERIOD_IN_USECS) && (kp <= MAX_KICK_PERIOD_IN_USECS))
    {
        system_kick_period = kp;
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
 *
 * stream_if_get_system_kick_period
 */
TIME_INTERVAL stream_if_get_system_kick_period(void)
{
#if defined(INSTALL_MIB) && !defined(UNIT_TEST_BUILD) && !defined(DESKTOP_TEST_BUILD)
    if (system_kick_period == 0)
    {
        /* This shouldn't happen on P1, because the P0 value is sent
         * via SYSTEM_KEYS_MSG_KICK_PERIOD at P1 boot
         */
#ifdef INSTALL_DUAL_CORE_SUPPORT
        PL_ASSERT(KIP_PRIMARY_CONTEXT());
#endif
        system_kick_period = mibgetrequ16(AUDIOENDPOINTKICKPERIOD);
    }
#endif
    return system_kick_period;
}



/****************************************************************************
 *
 * stream_if_eps_have_same_clock_source
 */
bool stream_if_eps_have_same_clock_source(unsigned ep1_id, unsigned ep2_id, bool* same_clock)
{
    ENDPOINT* ep1;
    ENDPOINT* ep2;

    patch_fn_shared(stream_if);

    if (same_clock == NULL)
    {
        return FALSE;
    }
    ep1 = stream_endpoint_from_extern_id(ep1_id);
    if (ep1 == NULL)
    {
        return FALSE;
    }
    ep2 = stream_endpoint_from_extern_id(ep2_id);
    if (ep2 == NULL)
    {
        return FALSE;
    }

    *same_clock = stream_rm_endpoints_have_same_clock_source(ep1, ep2);
    return TRUE;
}
