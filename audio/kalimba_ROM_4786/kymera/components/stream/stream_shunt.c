/****************************************************************************
 * Copyright (c) 2012 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_shunt.c
 * \ingroup stream
 *
 * stream shunt type file. <br>
 * This file contains stream functions for shunt endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_shunt_get_endpoint <br>
 */

#ifndef TODO_IMPLEMENT_SHUNT

/****************************************************************************
Include Files
*/

#include "stream_private.h"

/****************************************************************************
Private Type Declarations
*/

/**
 * TASAUX_PDU is a general purpose pointer type to a to air shunt aux msg PDU
 */
typedef uint16* TASAUX_PDU;

/**
 * FASAUX_PDU is a general purpose pointer type to a from air shunt aux msg PDU
 */
typedef uint16* FASAUX_PDU;

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

/**
 * Returns the to air shunt signal ID from the aux message PDU
 */
#define TASAUX_SIGNAL_ID_GET(pdu) \
    ((pdu)[TASAUX_HEADER_SIGNAL_ID_WORD_OFFSET])

/**
 * Returns the from air shunt signal ID from the aux message PDU
 */
#define FASAUX_SIGNAL_ID_GET(pdu) \
    ((pdu)[FASAUX_HEADER_SIGNAL_ID_WORD_OFFSET])


/****************************************************************************
Private Variable Definitions
*/

/**
 * Reference count of current number of instances of to air shunt service.
 * Used for controlling dorm.
 */
static int tas_reference_count = 0;

/**
 * Reference count of current number of instances of from air shunt service.
 * Used for controlling dorm.
 */
static int fas_reference_count = 0;

/****************************************************************************
Private Function Declarations
*/

static ENDPOINT *get_shunt_endpoint(unsigned int acl, unsigned int cid,
                                    ENDPOINT_DIRECTION dir);
static ENDPOINT *get_shunt_endpoint_from_sstag(SUBSERV_TAG sstag);
static unsigned get_new_shunt_endpoint_key(ENDPOINT_DIRECTION dir);
static bool shunt_close(ENDPOINT *endpoint);
static bool shunt_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr);
static bool shunt_disconnect (ENDPOINT *endpoint);
static bool shunt_buffer_details (ENDPOINT *endpoint, unsigned int *buffer_size);
static void shunt_kick(ENDPOINT *ep, bool valid);
static bool shunt_configure (ENDPOINT *endpoint, unsigned int key, uint32 value,
                             ACCMD_ASYNC_DETAILS *details);
static void shunt_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static AUDIO_DATA_FORMAT shunt_get_data_format (ENDPOINT *endpoint);
static void send_aux_msg_l2cap_atu(SUBSERV_TAG sstag, unsigned int atu);
static void send_aux_msg_tokens_allocated(SUBSERV_TAG sstag, unsigned int tokens);
static void send_aux_msg_autoflush_enable(SUBSERV_TAG sstag, unsigned int autoflush);
#if 0
static void send_fasaux_msg_l2cap_mtu(SUBSERV_TAG sstag, unsigned int mtu);
static void send_fasaux_msg_notify_when_drained(SUBSERV_TAG sstag, unsigned int notify);
#endif
static SUBSERV_TAG request_to_air_shunt_service(unsigned int acl,
                                                   unsigned int cid);
static SUBSERV_TAG request_from_air_shunt_service(unsigned int acl,
                                                     unsigned int cid);
static SUBSERV_RETURN tas_callback(SUBSERV_INSTANCE_DATA *idata);
static SUBSERV_RETURN fas_callback(SUBSERV_INSTANCE_DATA *idata);
static void tas_start_confirm_handler(SUBSERV_INSTANCE_DATA *idata);
static void fas_start_confirm_handler(SUBSERV_INSTANCE_DATA *idata);
static bool tas_aux_msg_handler(SUBSERV_TAG sstag, const TASAUX_PDU pdu, unsigned int pdu_len);
static bool fas_aux_msg_handler(SUBSERV_TAG sstag, const FASAUX_PDU pdu, unsigned int pdu_len);

DEFINE_ENDPOINT_FUNCTIONS(shunt_functions, shunt_close, shunt_connect, 
                          shunt_disconnect, shunt_buffer_details, 
                          shunt_kick, shunt_configure, shunt_get_timing,
                          stream_sync_sids_dummy, shunt_get_data_format);

/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
 *
 * stream_shunt_get_endpoint
 *
 */
ENDPOINT *stream_shunt_get_endpoint(unsigned int acl,
                                    unsigned int cid,
                                    ENDPOINT_DIRECTION dir,
                                    ACCMD_ASYNC_DETAILS *details)
{
    /* We always expect an instance of ACCMD_ASYNC_DETAILS */
    if (details == NULL)
    {
        panic(PANIC_AUDIO_ACCMD_ASYNC_DETAILS_NULL);
    }

    /* Find out if the requested endpoint already exists */
    ENDPOINT *endpoint = get_shunt_endpoint(acl, cid, dir );

    /* If it doesn't we'll create it */
    if (endpoint == NULL)
    {
        /* Get the next free shunt endpoint key. Ideally we would have
         * prefered to derive the key from the ACL handle and L2CAP channel
         * ID. However, that would have required more than 24 bits.
         */
        unsigned key = get_new_shunt_endpoint_key(dir);

        /* Create the endpoint */
        if ((endpoint = STREAM_NEW_ENDPOINT(shunt, key, dir, details->conidx)) != NULL)
        {
            /* Store the ACL handle and L2CAP channel ID in the endpoint */
            endpoint->state.shunt.acl = acl;
            endpoint->state.shunt.cid = cid;

            /* Standard settings for host created endpoints */
            endpoint->can_be_closed = TRUE;

            /* Shunt endpoints are always at the end of a chain */
            endpoint->is_real = TRUE;

            /* Clear out the details used for kicking the BTSS. If kicking is
             * required then the BTSS will supply the necessary information
             * for these fields through the optional KICK_PARAMS option IE.
             */
            endpoint->state.shunt.ss_blockid = 0;
            endpoint->state.shunt.ss_int_status = 0;

            /* The cbuffer structure that encapsulates the remote buffers
             * handles will be created later once we've had confirmation
             * that the to air shunt service has been started successfully.
             */
            endpoint->state.shunt.cbuffer = NULL;

            endpoint->state.shunt.sstag = SUBSERV_TAG_INVALID;
            if(SINK == dir)
            {
                /* Request that an instance of the BTSS to air shunt service
                 * be started.
                 */
                endpoint->state.shunt.sstag = request_to_air_shunt_service(acl, cid);
            }
            else
            {
                /* Request that an instance of the BTSS to air shunt service
                 * be started.
                 */
                endpoint->state.shunt.sstag = request_from_air_shunt_service(acl,cid);
            }
            if (endpoint->state.shunt.sstag != SUBSERV_TAG_INVALID)
            {
                /* Initial service request succeeded. Indicate that an
                 * asynchronous response will be sent later (once the service
                 * has been fully started).
                 */
                details->async = TRUE;
            }
            else
            {
                /* Request failed - clean up endpoint */
                stream_destroy_endpoint(endpoint);
                endpoint = NULL;
            }
        }
    }

    return endpoint;
}


/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Gets a pointer to the specified shunt endpoint
 *
 * \param acl the ACL handle for the shunt
 * \param cid the L2CAP Channel ID for the shunt
 * \param dir direction of the endpoint (i.e. a source or a sink)
 *
 * \return pointer to the endpoint, or NULL if not found
 *
 */
static ENDPOINT *get_shunt_endpoint(unsigned int acl, unsigned int cid,
                                    ENDPOINT_DIRECTION dir)
{
    ENDPOINT *ep = stream_first_endpoint(dir);

    while ((ep != NULL) && ((ep->stream_endpoint_type != endpoint_shunt) ||
                            (ep->state.shunt.acl != acl) ||
                            (ep->state.shunt.cid != cid)))
    {
        ep = ep->next;
    }

    return ep;
}

/**
 * \brief Gets a pointer to the shunt endpoint with the specified service tag
 *
 * \param sstag the to air shunt service tag
 *
 * \return pointer to the endpoint, or NULL if not found
 *
 */
static ENDPOINT *get_shunt_endpoint_from_sstag(SUBSERV_TAG sstag)
{
    ENDPOINT *ep = stream_first_endpoint(SINK);

    while ((ep != NULL) && ((ep->stream_endpoint_type != endpoint_shunt) ||
                            (ep->state.shunt.sstag != sstag)))
    {
        ep = ep->next;
    }

    /* Search through source list if the SSTAG is for a shunt source */
    if(NULL == ep)
    {
        ep = stream_first_endpoint(SOURCE);
        while ((ep != NULL) && ((ep->stream_endpoint_type != endpoint_shunt) ||
                                (ep->state.shunt.sstag != sstag)))
        {
            ep = ep->next;
        }
    }

    return ep;
}

/**
 * \brief Gets an unused shunt endpoint key
 *
 * \param dir direction of the endpoint (i.e. a source or a sink)
 *
 * \return A currently unused shunt endpoint key. The value returned should
 *         be used immediately to create an endpoint, otherwise it should be
 *         discarded.
 *
 */
static unsigned get_new_shunt_endpoint_key(ENDPOINT_DIRECTION dir)
{
    unsigned key;

    /* Keep checking keys until we break out of the loop */
    for (key = 0; ; key++)
    {
        /* Get the first endpoint in the list */
        ENDPOINT *ep = stream_first_endpoint(dir);

        /* Check each endpoint */
        while (ep != NULL)
        {
            /* Is this a shunt endpoint with the key we're checking for? */
            if ((ep->stream_endpoint_type == endpoint_shunt) && (ep->key == key))
            {
                /* Yes, the key is already in use. Try the next one */
                break;
            }
            ep = ep->next;
        }

        /* The current key is unused and available - break out of for loop */
        if (ep == NULL)
        {
            break;
        }
    }

    return key;
}

static bool shunt_close(ENDPOINT *endpoint)
{
    SUBSERV_RESULT result;

    if (endpoint->state.shunt.cbuffer != NULL)
    {
        /* Cleanup the cbuffer structure that encapsulates the remote to air
         * shunt buffer handles
         */
        cbuffer_destroy(endpoint->state.shunt.cbuffer);
        endpoint->state.shunt.cbuffer = NULL;
    }

    /* Stop the instance of the shunt service associated with the endpoint */
    if (!subserv_service_stop( endpoint->state.shunt.sstag, &result ))
    {
        /* Terminal error */
        if(endpoint->direction == SINK)
        {
            panic_diatribe( PANIC_AUDIO_FAILED_TO_STOP_TO_AIR_SHUNT_SERVICE,
                        endpoint->state.shunt.sstag );
        }
        else
        {
            panic_diatribe( PANIC_AUDIO_FAILED_TO_STOP_FROM_AIR_SHUNT_SERVICE,
                        endpoint->state.shunt.sstag );
        }
    }

    /* Decrement the service reference count and permit deep sleep if zero */
    if(endpoint->direction == SINK)
    {
        if (--tas_reference_count == 0)
        {
            dorm_allow_deep_sleep(DORM_TO_AIR_SHUNT_SERVICE);
        }

        L1_DBG_MSG("To Air Shunt service terminated");
    }
    else
    {
        if (--fas_reference_count == 0)
        {
            dorm_allow_deep_sleep(DORM_FROM_AIR_SHUNT_SERVICE);
        }

        L1_DBG_MSG("From Air Shunt service terminated");
    }

    return TRUE;
}

static bool shunt_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr)
{
    /* Shunt specific endpoint connection code to go here */

    return TRUE;
}

static bool shunt_disconnect(ENDPOINT *endpoint)
{
    /* Shunt specific endpoint disconnection code to go here */

    return TRUE;
}

static bool shunt_buffer_details(ENDPOINT *endpoint, unsigned int *buffer_size)
{
    if (endpoint == NULL || buffer_size == NULL)
    {
        return FALSE;
    }

    *buffer_size = 0;            /* The operator that's to be attached to
                                     * the shunt can specify this as it knows
                                     * its requirements. */
    return TRUE;
}

static bool shunt_configure(ENDPOINT *endpoint, unsigned int key, uint32 value,
                            ACCMD_ASYNC_DETAILS *details)
{
    bool success;

    /* We always expect an instance of ACCMD_ASYNC_DETAILS */
    if (details == NULL)
    {
        panic(PANIC_AUDIO_ACCMD_ASYNC_DETAILS_NULL);
    }

    /* All configuration values associated with shunt endpoints are 16 bits
     * or less and are passed on to the BTSS using a 16 bit field. An initial
     * check is therefore performed on the supplied value to immediately
     * eliminate rogue values greater than 0xffff.
     */
    if (value & 0xffff0000UL)
    {
        return FALSE;
    }

    if (endpoint->con_id != details->conidx)
    {
        /* A different host is trying to configure the endpoint than who
           raised the endpoint. At the moment this is allowed, however
           we will emit a fault as this is unexpected behaviour.
           This will get sorted out in B-124916.
           */
           fault_diatribe(FAULT_AUDIO_STREAM_ENDPOINT_ACCESS_VIOLATION, 
                          details->conidx);
    }

    switch (key)
    {
        case ACCMD_CONFIG_KEY_STREAM_SINK_SHUNT_L2CAP_ATU:
            /* Store the L2CAP ATU in the shunt endpoint where it will be
             * used to establish L2CAP packet boundaries.
             */
            endpoint->state.shunt.atu = (unsigned int)value;

            /* Send the L2CAP ATU to BTSS using the aux msg channel */
            send_aux_msg_l2cap_atu(endpoint->state.shunt.sstag,
                                   (unsigned int)value);

            /* Indicate that ACCMD response will be sent asynchronously */
            details->async = TRUE;

            success = TRUE;
            break;

        case ACCMD_CONFIG_KEY_STREAM_SINK_SHUNT_AUTOFLUSH_ENABLE :

            /* Send the autoflush state to BTSS using the aux msg channel */
            send_aux_msg_autoflush_enable(endpoint->state.shunt.sstag,
                                          (unsigned int)value);

            /* Indicate that ACCMD response will be sent asynchronously */
            details->async = TRUE;

            success = TRUE;
            break;

        case ACCMD_CONFIG_KEY_STREAM_SINK_SHUNT_TOKENS_ALLOCATED:

            /* Send the tokens allocated to BTSS using the aux msg channel */
            send_aux_msg_tokens_allocated(endpoint->state.shunt.sstag,
                                          (unsigned int)value);

            /* Indicate that ACCMD response will be sent asynchronously */
            details->async = TRUE;

            success = TRUE;
            break;

        /* TODO: Add config for shunt sources */
        default:
            /* Unrecognised configuration key */
            success = FALSE;
            break;
    }

    return success;
}

void shunt_kick(ENDPOINT *ep, bool valid)
{
    int curr_index;
    hydra_trb_trx interrupt_trx;

    /* Return immediately if from air shunt or if BTSS kicking is not required */
    if (ep->state.shunt.ss_int_status == 0 || ep->direction == SOURCE)
    {
        return;
    }

    /* See if the endpoints Cbuffer write index has moved. It only moves when
     * a full L2CAP packet has been produced so its time to kick BT. Otherwise
     * there is nothing to do this time. */
    curr_index = cbuffer_get_write_offset(ep->state.shunt.cbuffer);
    if (ep->state.shunt.prev_write_index != curr_index)
    {
        ep->state.shunt.prev_write_index = curr_index;

        /* Audio can send an empty interrupt message, BT will check all shunts so
         * there is no additional information to send. This just causes an interrupt
         * on BT so they know that Audio has kicked them.
         */

        /* The transaction shape we want to send is:
         * Opcode = T_TRANSACTION_MAJOR_OPCODE_T_EXTENDED
         * Sub-system source ID = SYSTEM_BUS_AUDIO_SYS
         * Sub-system block source ID = blockid service provider asked to be kicked on
         * Sub-system destination ID = address of service provider
         * Sub-system block destination ID = blockid service provider asked to be kicked on
         * Tag = 0 Don't care value transaction isn't tracked.
         * Payload = Extended Opcode 4 bits = 0 T_TRANSACTION_MINOR_OPCODE_T_INTERRUPT_EVENT
         *           Interrupt Status 16 bits = status chosen by provider to indicate shunt instance
         *           Unused 52 bits
         *
         * N.B. Sub-system source ID and destination ID are set the same. Sending of
         * these is mutually exclusive so we set both to be the same to make sure
         * BT sees the value we want it to.
         */
        hydra_trb_trx_header_init(&interrupt_trx,
                T_TRANSACTION_MAJOR_OPCODE_T_EXTENDED,
                SYSTEM_BUS_AUDIO_SYS,
                ep->state.shunt.ss_blockid,
                SYSTEM_BUS_BT_SYS,
                ep->state.shunt.ss_blockid,
                0,
                ((T_TRANSACTION_MINOR_OPCODE_T_INTERRUPT_EVENT << 4) |
                        ((ep->state.shunt.ss_int_status >> 12) & 0xf)));
        interrupt_trx.data[2] = ((ep->state.shunt.ss_int_status << 4) & 0xFFF0);

        bus_message_blocking_transmit_arbitrary_transaction(&interrupt_trx);
    }

}

static void shunt_get_timing(ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    /* No specific timing information is required for the shunt endpoint.
     * It'll just get kicked after the operator that's connected to it.
     */
    time_info->period = 1;
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
}

static AUDIO_DATA_FORMAT shunt_get_data_format(ENDPOINT *endpoint)
{
    /* Temporarily set the data format to AUDIO_DATA_FORMAT_16_BIT to match
     * the output format reported by the SBC encoder when running in data mode.
     *
     * This will change soon as AUDIO_DATA_FORMAT is to be superseded by a new
     * and more flexible format descriptor (bitfield based) that will support
     * the notion of a 'data' format.
     */
    return AUDIO_DATA_FORMAT_16_BIT;
}

/**
 * \brief Sends a TASAUX_SIGNAL_ID_L2CAP_ATU auxiliary command channel message
 *
 * \param sstag The service tag of the instance to which the auxiliary message
 *              should be sent
 * \param atu The L2CAP ATU to send (the message payload)
 *
 */
static void send_aux_msg_l2cap_atu(SUBSERV_TAG sstag, unsigned int atu)
{
    /* Create, populate and send an L2CAP_ATU auxiliary message to the service
     * instance associated with the specified service tag.
     */
    TASAUX_L2CAP_ATU *msg = pnew(TASAUX_L2CAP_ATU);
    TASAUX_HEADER_SIGNAL_ID_SET(&(msg->header), TASAUX_SIGNAL_ID_L2CAP_ATU);
    TASAUX_L2CAP_ATU_PACK((TASAUX_PDU)msg, atu);

    subserv_aux_cmd_send(sstag, (TASAUX_PDU)msg, TASAUX_L2CAP_ATU_WORD_SIZE);

    /* Free the message now it's been sent */
    pfree(msg);
}

#if 0
/**
 * \brief Sends a FASAUX_SIGNAL_ID_L2CAP_MTU auxiliary command channel message
 *
 * \param sstag The service tag of the instance to which the auxiliary message
 *              should be sent
 * \param mtu The L2CAP MTU to send (the message payload)
 *
 */
static void send_fasaux_msg_l2cap_mtu(SUBSERV_TAG sstag, unsigned int mtu)
{
    /* Create, populate and send an L2CAP_MTU auxiliary message to the service
     * instance associated with the specified service tag.
     */
    FASAUX_L2CAP_MTU *msg = pnew(FASAUX_L2CAP_MTU);
    FASAUX_HEADER_SIGNAL_ID_SET(&(msg->header), FASAUX_SIGNAL_ID_L2CAP_MTU);
    FASAUX_L2CAP_MTU_PACK((FASAUX_PDU)msg, atu);

    subserv_aux_cmd_send(sstag, (FASAUX_PDU)msg, FASAUX_L2CAP_MTU_WORD_SIZE);

    /* Free the message now it's been sent */
    pfree(msg);
}
#endif
/**
 * \brief Sends a TASAUX_TOKENS_ALLOCATED auxiliary command channel message
 *
 * \param sstag The service tag of the instance to which the auxiliary message
 *              should be sent
 * \param tokens The token count to send (the message payload)
 *
 */
static void send_aux_msg_tokens_allocated(SUBSERV_TAG sstag, unsigned int tokens)
{
    /* Create, populate and send a TOKENS_ALLOCATED auxiliary message to the
     * service instance associated with the specified service tag.
     */
    TASAUX_TOKENS_ALLOCATED *msg = pnew(TASAUX_TOKENS_ALLOCATED);
    TASAUX_HEADER_SIGNAL_ID_SET(&(msg->header), TASAUX_SIGNAL_ID_TOKENS_ALLOCATED);
    TASAUX_TOKENS_ALLOCATED_PACK((TASAUX_PDU)msg, tokens);

    subserv_aux_cmd_send(sstag, (TASAUX_PDU)msg, TASAUX_TOKENS_ALLOCATED_WORD_SIZE);

    /* Free the message now it's been sent */
    pfree(msg);
}

/**
 * \brief Sends a TASAUX_AUTOFLUSH_ENABLE auxiliary command channel message
 *
 * \param sstag The service tag of the instance to which the auxiliary message
 *              should be sent
 * \param autoflush The autoflush state to send (the message payload)
 *
 */
static void send_aux_msg_autoflush_enable(SUBSERV_TAG sstag, unsigned int autoflush)
{
    /* Create, populate and send an AUTOFLUSH_ENABLE auxiliary message to the
     * service instance associated with the specified service tag.
     */
    TASAUX_AUTOFLUSH_ENABLE *msg = pnew(TASAUX_AUTOFLUSH_ENABLE);
    TASAUX_HEADER_SIGNAL_ID_SET(&(msg->header), TASAUX_SIGNAL_ID_AUTOFLUSH_ENABLE);
    TASAUX_AUTOFLUSH_ENABLE_PACK((TASAUX_PDU)msg, autoflush);

    subserv_aux_cmd_send(sstag, (TASAUX_PDU)msg, TASAUX_AUTOFLUSH_ENABLE_WORD_SIZE);

    /* Free the message now it's been sent */
    pfree(msg);
}
#if 0
/**
 * \brief Sends a FASAUX_SIGNAL_ID_NOTIFY_WHEN_DRAINED auxiliary command channel message
 *
 * \param sstag The service tag of the instance to which the auxiliary message
 *              should be sent
 * \param notify The notify state to send (the message payload)
 *
 */
static void send_fasaux_msg_notify_when_drained(SUBSERV_TAG sstag, unsigned int notify)
{
    /* Create, populate and send an NOTIFY_WHEN_DRAINED auxiliary message to the
     * service instance associated with the specified service tag.
     */
    FASAUX_NOTIFY_WHEN_DRAINED *msg = pnew(FASAUX_NOTIFY_WHEN_DRAINED);
    FASAUX_HEADER_SIGNAL_ID_SET(&(msg->header), FASAUX_SIGNAL_ID_NOTIFY_WHEN_DRAINED);
    FASAUX_NOTIFY_WHEN_DRAINED_PACK((FASAUX_PDU)msg, notify);

    subserv_aux_cmd_send(sstag, (FASAUX_PDU)msg, FASAUX_NOTIFY_WHEN_DRAINED_WORD_SIZE);

    /* Free the message now it's been sent */
    pfree(msg);
}
#endif
/**
 * \brief Requests that an instance of the to air shunt service be started
 *
 * \param acl the ACL handle to be associated with the service instance
 * \param cid the L2CAP Channel ID to be associated with the service instance
 *
 * \return A \c SUBSERV_TAG that will be used to refer to the service instance, if the
 *         initial request was successfully sent, else \c SUBSERV_TAG_INVALID.
 *
 */
static SUBSERV_TAG request_to_air_shunt_service(unsigned int acl,
                                                   unsigned int cid)
{
    SUBSERV_PARAMETER params[1];
    SUBSERV_PARAMETER_PTR param_ptr;
    uint16 ies[SERVICE_IE_OPTION_PAYLOAD_OFFSET +
               CCP_TO_AIR_SHUNT_PARAMS_WORD_SIZE];
    SERVICE_IE_PTR ies_ptr;
    SUBSERV_RESULT result;


    /* Set up the single parameter IE
     * ------------------------------
     */

    /* Initialise the param_ptr structure */
    param_ptr.ptr = params;
    param_ptr.len = 1;
    memset(params, 0, sizeof(params));
    
    /*
     * The parameter is the single buffer for data to the provider. The
     * consumer requests a value to be filled in, indicating to globserv
     * and the provider that there is no remaining negotation so the service
     * can be activated at the same time. The buffer size will be specified
     * by the provider.
     */
    subserv_pm_direction_set(params, SUBSERV_PM_DIR_TO_REMOTE);
    subserv_pm_supplier_set(params, SUBSERV_PM_SUPP_REMOTE_VALUE_REQUIRED);
    subserv_pm_type_set(params, SUBSERV_RESOURCE_MMU_BUFFER);
    subserv_pm_resource_size_set(params, 0);


    /* Set up the mandatory option IE
     * ------------------------------
     */

    /* Initialise the ies_ptr structure */
    ies_ptr.ptr = ies;
    ies_ptr.len = SERVICE_IE_OPTION_PAYLOAD_OFFSET +
                                 CCP_TO_AIR_SHUNT_PARAMS_WORD_SIZE;

    /* Set up the option IE header */
    SERVICE_IE_HEADER_SET( ies,
                           CCP_TO_AIR_SHUNT_PARAMS_WORD_SIZE * 2,
                           CCP_IE_ID_TO_AIR_SHUNT_PARAMS );

    /* Set up the option IE payload */
    CCP_TO_AIR_SHUNT_PARAMS_PACK(
           (CCP_TO_AIR_SHUNT_PARAMS*)(&ies[SERVICE_IE_OPTION_PAYLOAD_OFFSET]),
           acl, cid);

    L1_DBG_MSG2("Requesting To Air Shunt Service on acl %x L2CAP channel %x", acl, cid);

    /* Request an instance of the service
     * ----------------------------------
     * (We request version 1. Version 0 was advertised by older BT
     * firmware but uses incompatible IE numbering. See B-138980.)
     */
    return subserv_service_start( &result,
                                  CCP_SERVICE_ID_BLUETOOTH_TO_AIR_SHUNT,
                                  SYSTEM_BUS_BT_SYS,
                                  1,
                                  &param_ptr,
                                  &ies_ptr,
                                  tas_callback );
}

/**
 * \brief Requests that an instance of the from air shunt service be started
 *
 * \param acl the ACL handle to be associated with the service instance
 * \param cid the L2CAP Channel ID to be associated with the service instance
 *
 * \return A \c SUBSERV_TAG that will be used to refer to the service instance, if the
 *         initial request was successfully sent, else \c SUBSERV_TAG_INVALID.
 *
 */
static SUBSERV_TAG request_from_air_shunt_service(unsigned int acl,
                                                     unsigned int cid)
{
    SUBSERV_PARAMETER params[1];
    SUBSERV_PARAMETER_PTR param_ptr;
    uint16 ies[SERVICE_IE_OPTION_PAYLOAD_OFFSET +
               CCP_FROM_AIR_SHUNT_PARAMS_WORD_SIZE];
    SERVICE_IE_PTR ies_ptr;
    SUBSERV_RESULT result;


    /* Set up the single parameter IE
     * ------------------------------
     */

    /* Initialise the param_ptr structure */
    param_ptr.ptr = params;
    param_ptr.len = 1;
    memset(params, 0, sizeof(params));

    /*
     * The parameter is the single buffer for data to the provider. The
     * consumer requests a value to be filled in, indicating to globserv
     * and the provider that there is no remaining negotation so the service
     * can be activated at the same time. The buffer size will be specified
     * by the provider.
     */
    subserv_pm_direction_set(params, SUBSERV_PM_DIR_TO_REMOTE);
    subserv_pm_supplier_set(params, SUBSERV_PM_SUPP_REMOTE_VALUE_REQUIRED);
    subserv_pm_type_set(params, SUBSERV_RESOURCE_MMU_BUFFER);
    subserv_pm_resource_size_set(params, 0);


    /* Set up the mandatory option IE
     * ------------------------------
     */

    /* Initialise the ies_ptr structure */
    ies_ptr.ptr = ies;
    ies_ptr.len = SERVICE_IE_OPTION_PAYLOAD_OFFSET +
                                 CCP_FROM_AIR_SHUNT_PARAMS_WORD_SIZE;

    /* Set up the option IE header */
    SERVICE_IE_HEADER_SET( ies,
                           CCP_FROM_AIR_SHUNT_PARAMS_WORD_SIZE * 2,
                           CCP_IE_ID_FROM_AIR_SHUNT_PARAMS );

    /* Set up the option IE payload */
    CCP_FROM_AIR_SHUNT_PARAMS_PACK(
           (CCP_FROM_AIR_SHUNT_PARAMS*)(&ies[SERVICE_IE_OPTION_PAYLOAD_OFFSET]),
           acl, cid);

    L1_DBG_MSG2("Requesting From Air Shunt Service on acl %x L2CAP channel %x", acl, cid);

    /* Request an instance of the service
     * ----------------------------------
     */
    return subserv_service_start( &result,
                                  CCP_SERVICE_ID_BLUETOOTH_FROM_AIR_SHUNT,
                                  SYSTEM_BUS_BT_SYS,
                                  0,
                                  &param_ptr,
                                  &ies_ptr,
                                  fas_callback );
}

/**
 * \brief  The callback for the to air shunt service used by subserv to
 *         inform us of our success or failure when starting/stopping an
 *         instance of the service. Also handles incoming aux messages.
 *
 * \param  idata Pointer to subserv instance data structure
 *
 * \return  A value from the \c SUBSERV_RETURN enum indicating the result
 */
static SUBSERV_RETURN tas_callback(SUBSERV_INSTANCE_DATA *idata)
{
    SUBSERV_RETURN rtn = SUBSERV_RETURN_FAILURE;

    /* Switch based on the service operation received */
    switch (idata->op)
    {
        case SUBSERV_OP_START_CONFIRM:
        {
            /* Handle the start confirm notification */
            tas_start_confirm_handler(idata);

            /* Increment the service reference count and prevent deep sleep */
            tas_reference_count++;
            dorm_disallow_deep_sleep(DORM_TO_AIR_SHUNT_SERVICE);

            L1_DBG_MSG("To Air Shunt service started");
            rtn = SUBSERV_RETURN_SUCCESS;
            break;
        }
        case SUBSERV_OP_START_FAIL:
        {
            ENDPOINT *endpoint = get_shunt_endpoint_from_sstag(idata->sstag);

            /* The BTSS failed to start the service. Send the outstanding
             * ACCMD response relating to the original stream_get_sink
             * message indicating failure.
             */
            accmd_send_get_sink_resp(endpoint->con_id, 0);

            /* Clean up the endpoint */
            stream_destroy_endpoint(endpoint);
            break;
        }
        case SUBSERV_OP_AUX_CMD:
        {
            /* Handle the received auxiliary command channel message */
            if (tas_aux_msg_handler(idata->sstag,
                                (TASAUX_PDU)idata->ops.aux_cmd.data,
                                idata->ops.aux_cmd.data_len))
            {
                rtn = SUBSERV_RETURN_SUCCESS;
            }
            break;
        }
        default:
        {
            /* Unhandled or unrecognised command */
            break;
        }
    }
    return rtn;
}

/**
 * \brief  The callback for the from air shunt service used by subserv to
 *         inform us of our success or failure when starting/stopping an
 *         instance of the service. Also handles incoming aux messages.
 *
 * \param  idata Pointer to subserv instance data structure
 *
 * \return  A value from the \c SUBSERV_RETURN enum indicating the result
 */
static SUBSERV_RETURN fas_callback(SUBSERV_INSTANCE_DATA *idata)
{
    SUBSERV_RETURN rtn = SUBSERV_RETURN_FAILURE;

    /* Switch based on the service operation received */
    switch (idata->op)
    {
        case SUBSERV_OP_START_CONFIRM:
        {
            /* Handle the start confirm notification */
            fas_start_confirm_handler(idata);

            /* Increment the service reference count and prevent deep sleep */
            fas_reference_count++;
            dorm_disallow_deep_sleep(DORM_FROM_AIR_SHUNT_SERVICE);

            L1_DBG_MSG("From Air Shunt service started");
            rtn = SUBSERV_RETURN_SUCCESS;
            break;
        }
        case SUBSERV_OP_START_FAIL:
        {
            ENDPOINT *endpoint = get_shunt_endpoint_from_sstag(idata->sstag);

            /* The BTSS failed to start the service. Send the outstanding
             * ACCMD response relating to the original stream_get_source
             * message indicating failure.
             */
            accmd_send_get_source_resp(endpoint->con_id, 0);

            /* Clean up the endpoint */
            stream_destroy_endpoint(endpoint);
            break;
        }
        case SUBSERV_OP_AUX_CMD:
        {
            /* Handle the received auxiliary command channel message */
            if (fas_aux_msg_handler(idata->sstag,
                                (FASAUX_PDU)idata->ops.aux_cmd.data,
                                idata->ops.aux_cmd.data_len))
            {
                rtn = SUBSERV_RETURN_SUCCESS;
            }
            break;
        }
        default:
        {
            /* Unhandled or unrecognised command */
            break;
        }
    }
    return rtn;
}

/**
 * \brief  The handler for the SUBSERV_OP_START_CONFIRM notification that
 *         informs us that the service started successfully.
 *
 * \param  idata Pointer to subserv instance data structure
 *
 */
static void tas_start_confirm_handler(SUBSERV_INSTANCE_DATA *idata)
{
    SUBSERV_PARAMETER *param;
    SERVICE_IE_PTR ies_ptr;
    uint16 *ie_payload;
    int remote_buffer_size;
    ENDPOINT *endpoint;

    /* Only one parameter is expected */
    if (idata->ops.start.params.len != 1)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_TO_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* Get a pointer to the parameter */
    param = idata->ops.start.params.ptr;

    /* Parameter type must be SUBSERV_RESOURCE_MMU_BUFFER */
    if (subserv_pm_type_get(param) != SUBSERV_RESOURCE_MMU_BUFFER)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_TO_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* Parameter must indicate a remote buffer */
    if (subserv_pm_supplier_get(param) != SUBSERV_PM_SUPP_REMOTE_DEFINITIVE)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_TO_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* Get the buffer size in words (the parameter specifies the size in
     * octets and therefore requires translation).
     */
    remote_buffer_size = buffer_octets_to_words(subserv_pm_resource_size_get(param));

    endpoint = get_shunt_endpoint_from_sstag(idata->sstag);

    /* Attempt to locate and process the optional KICK_PARAMS option IE */
    SERVICE_IE_PTR_COPY(&ies_ptr, &idata->ops.start.ies_in);
    if (hydra_service_ie_type_find(&ies_ptr, CCP_IE_ID_KICK_PARAMS))
    {
        ie_payload = (uint16 *)&(ies_ptr.ptr[SERVICE_IE_OPTION_PAYLOAD_OFFSET]);

        /* Retrieve the block ID and status bits from the option IE and
         * store them in the endpoint
         */
        endpoint->state.shunt.ss_blockid =
                        CCP_KICK_PARAMS_BLOCK_ID_GET((CCP_KICK_PARAMS*)ie_payload);
        endpoint->state.shunt.ss_int_status =
                        CCP_KICK_PARAMS_STATUS_BITS_GET((CCP_KICK_PARAMS*)ie_payload);
    }

    /* Locate and process the TO_AIR_SHUNT_BUFFER_HANDLES option IE */
    SERVICE_IE_PTR_COPY(&ies_ptr, &idata->ops.start.ies_in);
    if (hydra_service_ie_type_find(&ies_ptr, CCP_IE_ID_TO_AIR_SHUNT_BUFFER_HANDLES))
    {
        mmu_handle remote_rd_handle, remote_wr_handle;

        ie_payload = (uint16 *)&(ies_ptr.ptr[SERVICE_IE_OPTION_PAYLOAD_OFFSET]);

        /* Unpack the remote buffer handles from the option IE */
        mmu_port_unpack_buffer_handle(
                        CCP_TO_AIR_SHUNT_BUFFER_HANDLES_READ_HANDLE_GET((CCP_TO_AIR_SHUNT_BUFFER_HANDLES*)ie_payload),
                        &remote_rd_handle );
        mmu_port_unpack_buffer_handle(
                        CCP_TO_AIR_SHUNT_BUFFER_HANDLES_WRITE_HANDLE_GET((CCP_TO_AIR_SHUNT_BUFFER_HANDLES*)ie_payload),
                        &remote_wr_handle );

        /* Wrap up the remote buffer handles */
        endpoint->state.shunt.cbuffer = cbuffer_wrap_remote(
                                            BUF_DESC_WRAP_REMOTE_MMU_MOD_WR,
                                            remote_rd_handle,
                                            remote_wr_handle,
                                            remote_buffer_size );

        /* All Shunt buffers require a byte-swap to be applied to the buffer contents.
         * We deal with data in little-endian format but the BTSS expects it to
         * be big-endian. */
        cbuffer_set_write_byte_swap(endpoint->state.shunt.cbuffer);
         
        /* Set the starting value for the initial prev_write_index */
        endpoint->state.shunt.prev_write_index =
                cbuffer_get_write_offset(endpoint->state.shunt.cbuffer);
    }
    else
    {
        panic_diatribe(PANIC_AUDIO_INVALID_TO_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* The service is up and running and everything is fine.
     * Send the outstanding ACCMD response to the original stream_get_sink
     * ACCMD message indicating success.
     */
    accmd_send_get_sink_resp(endpoint->con_id,
                             stream_external_id_from_endpoint(endpoint));
}

/**
 * \brief  The handler for the SUBSERV_OP_START_CONFIRM notification that
 *         informs us that the From Air Service started successfully.
 *
 * \param  idata Pointer to subserv instance data structure
 *
 */
static void fas_start_confirm_handler(SUBSERV_INSTANCE_DATA *idata)
{
    SUBSERV_PARAMETER *param;
    SERVICE_IE_PTR ies_ptr;
    uint16 *ie_payload;
    int remote_buffer_size;
    ENDPOINT *endpoint;

    /* Only one parameter is expected */
    if (idata->ops.start.params.len != 1)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_FROM_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* Get a pointer to the parameter */
    param = idata->ops.start.params.ptr;

    /* Parameter type must be SUBSERV_RESOURCE_MMU_BUFFER */
    if (subserv_pm_type_get(param) != SUBSERV_RESOURCE_MMU_BUFFER)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_FROM_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* Parameter must indicate a remote buffer */
    if (subserv_pm_supplier_get(param) != SUBSERV_PM_SUPP_REMOTE_DEFINITIVE)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_FROM_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* Get the buffer size in words (the parameter specifies the size in
     * octets and therefore requires translation).
     */
    remote_buffer_size = buffer_octets_to_words(subserv_pm_resource_size_get(param));

    endpoint = get_shunt_endpoint_from_sstag(idata->sstag);

    /* Attempt to locate and process the optional KICK_PARAMS option IE */
    SERVICE_IE_PTR_COPY(&ies_ptr, &idata->ops.start.ies_in);
    if (hydra_service_ie_type_find(&ies_ptr, CCP_IE_ID_KICK_PARAMS))
    {
        ie_payload = (uint16 *)&(ies_ptr.ptr[SERVICE_IE_OPTION_PAYLOAD_OFFSET]);

        /* Retrieve the block ID and status bits from the option IE and
         * store them in the endpoint
         */
        endpoint->state.shunt.ss_blockid =
                        CCP_KICK_PARAMS_BLOCK_ID_GET((CCP_KICK_PARAMS*)ie_payload);
        endpoint->state.shunt.ss_int_status =
                        CCP_KICK_PARAMS_STATUS_BITS_GET((CCP_KICK_PARAMS*)ie_payload);
    }

    /* Locate and process the FROM_AIR_SHUNT_BUFFER_HANDLES option IE */
    SERVICE_IE_PTR_COPY(&ies_ptr, &idata->ops.start.ies_in);
    if (hydra_service_ie_type_find(&ies_ptr, CCP_IE_ID_FROM_AIR_SHUNT_BUFFER_HANDLES))
    {
        mmu_handle remote_rd_handle, remote_wr_handle;

        ie_payload = (uint16 *)&(ies_ptr.ptr[SERVICE_IE_OPTION_PAYLOAD_OFFSET]);

        /* Unpack the remote buffer handles from the option IE */
        mmu_port_unpack_buffer_handle(
                        CCP_FROM_AIR_SHUNT_BUFFER_HANDLES_READ_HANDLE_GET((CCP_FROM_AIR_SHUNT_BUFFER_HANDLES*)ie_payload),
                        &remote_rd_handle );
        mmu_port_unpack_buffer_handle(
                        CCP_FROM_AIR_SHUNT_BUFFER_HANDLES_WRITE_HANDLE_GET((CCP_FROM_AIR_SHUNT_BUFFER_HANDLES*)ie_payload),
                        &remote_wr_handle );

        /* Wrap up the remote buffer handles */
        endpoint->state.shunt.cbuffer = cbuffer_wrap_remote(
                                            BUF_DESC_WRAP_REMOTE_MMU_MOD_RD,
                                            remote_rd_handle,
                                            remote_wr_handle,
                                            remote_buffer_size );

        /* All Shunt buffers require a byte-swap to be applied to the buffer contents.
         * We deal with data in little-endian format but the BTSS expects it to
         * be big-endian. */
        cbuffer_set_write_byte_swap(endpoint->state.shunt.cbuffer);
#ifdef TODO_REMOVE_TEST_FAS_SHUNT
        /* The code below is to test we are getting data via shunt buffers and kicks from
         * audio endpoint monitor interrupts.
         * Set the starting value for the initial prev_write_index */
        endpoint->state.shunt.prev_write_index =
                cbuffer_get_write_offset(endpoint->state.shunt.cbuffer);
        endpoint->state.shunt.atu = 0;
#endif
    }
    else
    {
        panic_diatribe(PANIC_AUDIO_INVALID_FROM_AIR_SHUNT_SERVICE_IE, idata->sstag);
    }

    /* The service is up and running and everything is fine.
     * Send the outstanding ACCMD response to the original stream_get_source
     * ACCMD message indicating success.
     */
    accmd_send_get_source_resp(endpoint->con_id,
                             stream_external_id_from_endpoint(endpoint));
}

/**
 * \brief Handles messages received from BTSS over the AUX_CMD channel
 *
 * \param  sstag The service tag associated with the instance of the to air
 *         shunt sending the message
 * \param  pdu A pointer to the message that was received.
 * \param  pdu_len The length of the received message (pdu) in words.
 *
 * \return \c TRUE if message handled successfully, otherwise \c FALSE
 */
static bool tas_aux_msg_handler(SUBSERV_TAG sstag, const TASAUX_PDU pdu, unsigned int pdu_len)
{
    bool success = FALSE;

    ENDPOINT *endpoint = get_shunt_endpoint_from_sstag(sstag);
    if (NULL == endpoint || endpoint->direction != SINK)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_SSTAG_FOR_SHUNT_ENDPOINT, sstag);
    }

    /* Work out which message we've received and then process it */
    switch (TASAUX_SIGNAL_ID_GET(pdu))
    {
        /* This is the only aux msg we expect to receive from the BTSS.
         * Response to an aux configure message that was sent to the BTSS as
         * a result of receiving a to air shunt stream configure ACCMD.
         * Receipt of this message finally allows us to send the outstanding
         * ACCMD response message to the host.
         */
        case TASAUX_SIGNAL_ID_RESPONSE:
        {
            ACCMD_STATUS status;

            /* Before we process the message, check it is the expected size */
            if (pdu_len != TASAUX_RESPONSE_WORD_SIZE)
            {
                panic_diatribe(PANIC_AUDIO_BADLY_FORMED_TASAUX_MSG_RECEIVED,
                               TASAUX_SIGNAL_ID_GET(pdu));
            }

            /* Extract the boolean response from the message and convert it
             * to an ACCMD status.
             */
            status = TASAUX_RESPONSE_RESPONSE_GET(pdu) ?
                                  ACCMD_STATUS_OK : ACCMD_STATUS_CMD_FAILED;

            /* Create and send the outstanding ACCMD response message */
            accmd_send_std_resp(endpoint->con_id, status);
            success = TRUE;
            break;
        }
        /* This message is sent by the BTSS to notify us when the ACL link
         * has been dropped unexpectedly (i.e. not as a result of the to air
         * shunt endpoint being closed).
         */
        case TASAUX_SIGNAL_ID_ACL_DROPPED:
        {
            /* Before we process the message, check it is the expected size */
            if (pdu_len != TASAUX_ACL_DROPPED_WORD_SIZE)
            {
                panic_diatribe(PANIC_AUDIO_BADLY_FORMED_TASAUX_MSG_RECEIVED,
                               TASAUX_SIGNAL_ID_GET(pdu));
            }

            /* Close the shunt endpoint associated with the service instance.
             * This will automatically request the termination of the to air
             * shunt service instance.
             */
            if (!stream_close_endpoint(endpoint), NULL)
            {
                panic_diatribe(PANIC_AUDIO_FAILED_TO_CLOSE_TO_AIR_SHUNT_ENDPOINT,
                               stream_external_id_from_endpoint(endpoint));
            }
            success = TRUE;
            break;
        }
        default:
        {
            /* An unrecognised message has been received */
            panic_diatribe(PANIC_AUDIO_BADLY_FORMED_TASAUX_MSG_RECEIVED,
                           TASAUX_SIGNAL_ID_GET(pdu));
            break;
        }
    }

    return success;
}

/**
 * \brief Handles messages received from BTSS over the AUX_CMD channel
 *
 * \param  sstag The service tag associated with the instance of the from air
 *         shunt sending the message
 * \param  pdu A pointer to the message that was received.
 * \param  pdu_len The length of the received message (pdu) in words.
 *
 * \return \c TRUE if message handled successfully, otherwise \c FALSE
 */
static bool fas_aux_msg_handler(SUBSERV_TAG sstag, const FASAUX_PDU pdu, unsigned int pdu_len)
{
    bool success = FALSE;

    ENDPOINT *endpoint = get_shunt_endpoint_from_sstag(sstag);
    if (NULL == endpoint || endpoint->direction != SOURCE)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_SSTAG_FOR_SHUNT_ENDPOINT, sstag);
    }

    /* Work out which message we've received and then process it */
    switch (FASAUX_SIGNAL_ID_GET(pdu))
    {
        /* Response to an aux configure message that was sent to the BTSS as
         * a result of receiving a from air shunt stream configure ACCMD.
         * Receipt of this message finally allows us to send the outstanding
         * ACCMD response message to the host.
         */
        case FASAUX_SIGNAL_ID_RESPONSE:
        {
            ACCMD_STATUS status;

            /* Before we process the message, check it is the expected size */
            if (pdu_len != FASAUX_RESPONSE_WORD_SIZE)
            {
                panic_diatribe(PANIC_AUDIO_BADLY_FORMED_FASAUX_MSG_RECEIVED,
                               FASAUX_SIGNAL_ID_GET(pdu));
            }

            /* Extract the boolean response from the message and convert it
             * to an ACCMD status.
             */
            status = FASAUX_RESPONSE_RESPONSE_GET(pdu) ?
                                  ACCMD_STATUS_OK : ACCMD_STATUS_CMD_FAILED;

            /* Create and send the outstanding ACCMD response message */
            accmd_send_std_resp(endpoint->con_id, status);
            success = TRUE;
            break;
        }
        /* This message is sent by the BTSS to notify us when the ACL buffer is empty
         * and the shunt service was configured to notify on empty.
         */
        case FASAUX_SIGNAL_ID_ACL_BUFFER_EMPTY:
        {
            /* Before we process the message, check it is the expected size */
            if (pdu_len != FASAUX_SIGNAL_ID_ACL_BUFFER_EMPTY)
            {
                panic_diatribe(PANIC_AUDIO_BADLY_FORMED_FASAUX_MSG_RECEIVED,
                               FASAUX_SIGNAL_ID_GET(pdu));
            }

            /* Forward the message to the host */
            success = accmd_send_source_drained_ind(endpoint->con_id,
                                   stream_external_id_from_endpoint(endpoint));
            break;
        }
        default:
        {
            /* An unrecognised message has been received */
            panic_diatribe(PANIC_AUDIO_BADLY_FORMED_FASAUX_MSG_RECEIVED,
                           FASAUX_SIGNAL_ID_GET(pdu));
            break;
        }
    }

    return success;
}

#endif /* !TODO_IMPLEMENT_SHUNT */
