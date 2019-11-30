/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \addtogroup Audio Audio endpoint
 * \ingroup endpoints
 * \file  stream_audio_hydra.c
 *
 * stream audio type file. <br>
 * This file contains stream functions for audio endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_audio_get_endpoint <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "stream_endpoint_audio.h" /* For protected access to audio ep base class */
#include "opmgr/opmgr_endpoint_override.h"
#include "pl_fractional.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

enum hydra_audio_get_endpoint_params
{
    HYDRA_AUDIO_PARAMS_INSTANCE = 0,
    HYDRA_AUDIO_PARAMS_CHANNEL,
    HYDRA_AUDIO_PARAMS_NUM
};

/****************************************************************************
Private Macro Declarations
*/

#define STREAM_AUDIO_HW_FRM_KEY(k)                 ((audio_hardware)(((k)>>8)&0xFF))

/*#define STREAM_AUDIO_HYDRA_WARP_VERBOSE*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static void destroy_audio_endpoint(ENDPOINT *endpoint);
static bool audio_close (ENDPOINT *endpoint);
static bool audio_connect (ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool audio_disconnect (ENDPOINT *endpoint);
static bool audio_buffer_details (ENDPOINT *endpoint, BUFFER_DETAILS *details);
static bool audio_start (ENDPOINT *endpoint, KICK_OBJECT *ko);
static bool audio_stop (ENDPOINT *endpoint);
static bool audio_configure (ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool audio_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void audio_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static bool audio_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format);
static void adjust_audio_rate(ENDPOINT *ep, int32 adjust_val);
static bool enact_audio_rm(ENDPOINT* endpoint, uint32 value);
#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
static bool audio_init_warp_update_descs(ENDPOINT* ep);
static void adjust_audio_rate_warp(ENDPOINT *ep, int32 adjust_val);
static void set_rate_warp(ENDPOINT *ep, int use_val);
static void enact_audio_rm_warp(ENDPOINT *ep, uint32 val);
static int get_measured_sp_deviation(ENDPOINT *ep);
static int get_sp_deviation(ENDPOINT *ep);
static unsigned get_audio_rate(ENDPOINT* ep);
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */
#ifdef INSTALL_AUDIO_EP_CLRM
static bool match_audio_rate(ENDPOINT *ep, const RATE_RELATIVE_RATE* ref);
#endif
#ifdef INSTALL_MCLK_SUPPORT
static bool stream_audio_activate_mclk_callback(unsigned sid, MCLK_MGR_RESPONSE status);
#endif
static bool is_hw_rate_adjustment_supported(ENDPOINT *endpoint);
static bool is_locally_clocked(ENDPOINT *endpoint);

DEFINE_ENDPOINT_FUNCTIONS (audio_functions, audio_close, audio_connect,
                           audio_disconnect, audio_buffer_details,
                           audio_kick, stream_sched_kick_dummy,
                           audio_start, audio_stop,
                           audio_configure, audio_get_config,
                           audio_get_timing, sync_endpoints);

/****************************************************************************
Private Function Declarations
*/

/****************************************************************************
Public Function Definitions
*/
/****************************************************************************
 *
 * stream_audio_get_endpoint
 *
 */
ENDPOINT *stream_audio_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned int hardware,
                                    unsigned num_params,
                                    unsigned *params,
                                    bool *pending)
{
    patch_fn_shared(stream_audio_hydra);

    if (num_params!=HYDRA_AUDIO_PARAMS_NUM)
    {
        return NULL;
    }

    int instance = params[HYDRA_AUDIO_PARAMS_INSTANCE];
    int channel  = params[HYDRA_AUDIO_PARAMS_CHANNEL];

    /* First go and find a stream on the same audio hardware, if not found
     * then create a new stream. */
    unsigned key = create_stream_key(hardware, instance, channel);
    ENDPOINT *endpoint = stream_get_endpoint_from_key_and_functions(key, dir,
                                                  &endpoint_audio_functions);
    if(!endpoint)
    {
        endpoint_audio_state* ep_audio;

        if ((endpoint = STREAM_NEW_ENDPOINT(audio, key, dir, con_id)) == NULL)
        {
            return NULL;
        }

        ep_audio = &endpoint->state.audio;

        /* All is well */
        endpoint->can_be_closed = TRUE;
        endpoint->can_be_destroyed = FALSE;
        /* Audio endpoints are always at the end of a chain */
        endpoint->is_real = TRUE;
        ep_audio->is_overridden = FALSE;

        /* Initialise endpoint synchronisation values */
        ep_audio->head_of_sync = endpoint;
        ep_audio->nep_in_sync = NULL;

        /* Initialise rate matching values */
        endpoint->deferred.config_deferred_kick = TRUE;
        ep_audio->rm_support = RATEMATCHING_SUPPORT_NONE;
        ep_audio->rm_adjust_amount = 0;
        ep_audio->rm_adjust_prev = 0;
        ep_audio->rm_report_sp_deviation = 0;
        ep_audio->rm_enable_sw_rate_adjust = TRUE;
        ep_audio->rm_enable_hw_rate_adjust = TRUE;
        ep_audio->rm_enable_clrm_measure = TRUE;
#ifdef INSTALL_AUDIO_EP_CLRM
#ifdef TODO_AUDIO_EP_CLRM_DISABLE_MATCH
        ep_audio->rm_enable_clrm_match = FALSE;
#else
        ep_audio->rm_enable_clrm_match = TRUE;
#endif
        ep_audio->rm_enable_clrm_adaptive = rate_match_default_config.enable_adapt;
#endif /* INSTALL_AUDIO_EP_CLRM */
#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
        ep_audio->rm_update_desc.channel_mask = 0;
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */

        /* By default we expect to produce FIXP audio, so activate the necessary HW shift and cbops */
        ep_audio->shift = DAWTH - 16;

#ifdef AUDIO_SOURCE_GENERATE_METADATA
#ifdef AUDIO_SOURCE_ENABLE_TOA_METADATA_BY_DEFAULT
        if(SOURCE == endpoint->direction)
        {
            /* support metadata by default */
            endpoint->state.audio.generate_metadata = TRUE;
        }
#endif
#endif

#ifdef INSTALL_UNINTERRUPTABLE_ANC
        /* ANC isn't using this newly created endpoint */
        ep_audio->anc.instance_id = ACCMD_ANC_INSTANCE_NONE_ID;
        ep_audio->anc.input_path_id = ACCMD_ANC_PATH_NONE_ID;
#endif /* INSTALL_UNINTERRUPTABLE_ANC */

        /*
           Create single-channel cbop infrastructure as in the old days.
           When add_to_sync is done, any endpoint found in group gets its cbops destroyed, but using its info incl. buffer ptrs,
           we create a new multi-channel cbop. Simple more wasteful approach is to create it for max number of channels.
           If connect is done prior to sync'ing, then connect hooks in buffer info and params individually to the single-channel cbops
           as it did in the old days. Later sync command(s) will consolidate these for the EPs that are sync'ed...
           Other EPs will be left alone with their individual single-channel cbops.
         */
        if((endpoint->cbops = cbops_mgr_create(endpoint->direction,
                CBOPS_DISCARD | CBOPS_DC_REMOVE | CBOPS_RATEADJUST | CBOPS_UNDERRUN)) == NULL)
        {
            destroy_audio_endpoint(endpoint);
            return NULL;
        }

        if(!stream_audio_post_create_check(hardware, instance, channel, dir,
                           endpoint, pending))
        {
            destroy_audio_endpoint(endpoint);
            return NULL;
        }
        endpoint->state.audio.hw_allocated = !(*pending);
    }

#if defined(INSTALL_UNINTERRUPTABLE_ANC) && defined(INSTALL_ANC_STICKY_ENDPOINTS)
    endpoint_audio_state* ep_audio = &endpoint->state.audio;

    /* Newly opened/re-opened so indicate that ANC should not close the endpoint */
    ep_audio->anc.close_pending = FALSE;
#endif /* defined(INSTALL_UNINTERRUPTABLE_ANC) && defined(INSTALL_ANC_STICKY_ENDPOINTS) */

    return endpoint;
}

/****************************************************************************
Private Function Definitions
*/
/* ******************************* Helper functions ************************************ */

/* remove_from_synchronisation */
bool remove_from_synchronisation(ENDPOINT *ep)
{
    patch_fn_shared(stream_audio_hydra);

    if ( IS_AUDIO_ENDPOINT_SYNCED(ep) )
    {
        remove_from_sync_list(ep);
        /* Call into audio to reset the synchronisation groups in hardware */
        audio_vsm_sync_sids(stream_external_id_from_endpoint(ep), 0);
    }
    return TRUE;
}

/* add_to_synchronisation */
bool add_to_synchronisation(ENDPOINT *ep1, ENDPOINT *ep2)
{
    patch_fn_shared(stream_audio_hydra);

    /* If the two endpoint is the same or they are already synced return TRUE. */
    if ((ep1 == ep2) || (ALREADY_SYNCHRONISED(ep1,ep2)))
    {
        return TRUE;
    }

    /* Cannot synchronise with a running group of endpoints.*/
    if (SYNC_GROUP_IS_RUNNING(ep1) || SYNC_GROUP_IS_RUNNING(ep2))
    {
        return FALSE;
    }

    /* Call into audio to set the synchronisation groups in hardware */
    if(!audio_vsm_sync_sids(stream_external_id_from_endpoint(ep1), stream_external_id_from_endpoint(ep2)))
    {
        /* Hardware sync failed. So nothing else to do */
        return FALSE;
    }

    /* add new endpoint to the synchronisation list */

    return add_to_sync_list(ep1,ep2);
}

static void destroy_audio_endpoint(ENDPOINT *endpoint)
{
    /* Failed to get everything, give back what we might have asked for */
    if(endpoint->cbops != NULL)
    {
        /* This gets called on create audio endpoint. The cbops is still single slot */
        cbops_mgr_destroy(endpoint->cbops);
        endpoint->cbops = NULL;
    }
    stream_destroy_endpoint(endpoint);
}

#ifdef INSTALL_MCLK_SUPPORT
/**
 * \brief Activate mclk output for an audio interface

 * \param ep endpoint
 * \param activate_output if not 0, user wants to activate mclk OUTPUT for this endpoint, otherwise it will
 *        de-activate the output. Activation/De-activation request will only be done if:
 *        - the endpoint can have mclk output (e.g i2s master)
 *        - interface wants to route the MCLK output via GPIO, Note that the MCLK output can be generated from
 *          internal clock too.
 * \param enable_mclk makes the mclk available to use by the endpoint (instead of root clock). For an interface
 *        to use MCLK we need to make sure that the MCLK is available and stable this should be able to be done
 *        automatically before an interface gets activated(normally at connection point), so we might deprecate
 *        this flag in future.
 * \param pending set to TRUE by the endpoint if it needs to wait before the mclk becomes available
 */
bool stream_audio_activate_mclk(ENDPOINT *ep, unsigned activate_output, unsigned enable_mclk, bool *pending)
{
    return audio_vsm_activate_mclk(stream_external_id_from_endpoint(ep),
                                   (bool) activate_output,
                                   (bool) enable_mclk,
                                   &ep->state.audio.mclk_claimed,
                                   pending,
                                   stream_audio_activate_mclk_callback);
}
#endif /* #ifdef INSTALL_MCLK_SUPPORT */

/**
 * \brief closes the audio endpoint by requesting the release method on the
 *        audio hardware
 *
 * \param *endpoint pointer to the endpoint that is being closed.
 *
 * \return success or failure
 */
static bool audio_close(ENDPOINT *endpoint)
{
    patch_fn_shared(stream_audio_hydra);

#ifdef INSTALL_MCLK_SUPPORT
 if(endpoint->state.audio.mclk_claimed)
 {
     /* Endpoint has claimed mclk, it needs
      * to release it before we can close
      * the endpoint.
      */
     L2_DBG_MSG1("Failed to close audio endpoint (0x%x), MCLK needs to be released first", (unsigned)(uintptr_t)endpoint);

     return FALSE;
 }
#endif /* INSTALL_MCLK_SUPPORT */

    /* If endpoint is synced, remove it from sync list */
    if(!sync_endpoints(endpoint,NULL))
    {
        /* Fail if the sync failed */
        return FALSE;
    }
    if(endpoint->cbops != NULL)
    {
        /* Endpoint's cbops manager object will be independent of other endpoints since
         * it has been taken of a synchronisation list. So it is either a single slot
         * cbops manager object or a multichannel one with all the other channels marked
         * as not in use, because this is the last endpoint to call "desync".
         * In all cases, it is safe to call destroy.
         */
        cbops_mgr_destroy(endpoint->cbops);
        endpoint->cbops = NULL;
    }

#ifdef TIMED_PLAYBACK_MODE
    timed_playback_destroy(endpoint->state.audio.timed_playback);
#endif

    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is call the close hw method on audio, if it has
     * been allocated
     */
    return (endpoint->state.audio.hw_allocated)
                ? audio_vsm_release_hardware(stream_external_id_from_endpoint(endpoint))
                : TRUE;
}

#ifdef INSTALL_MCLK_SUPPORT
/**
 * \brief call back function for when endpoint needs to wait mclk claiming before
 *        activation takes place
 *
 * \param sid interface sid
 * \param status the result from mclk manager
 *
 * \return success or failure
 */
static bool stream_audio_activate_mclk_callback(unsigned sid, MCLK_MGR_RESPONSE status)
{
    ENDPOINT *ep = stream_endpoint_from_extern_id(sid);

    patch_fn_shared(stream_audio_hydra);

    if(MCLK_MGR_RESPONSE_CLAIM_SUCCESS == status)
    {
        /* we have claimed MCLK before using it */
        ep->state.audio.mclk_claimed = TRUE;
        /* activate mclk output if needed */
        audio_vsm_complete_mclk_output_activation(sid);
        /* report completion of mclk activation process  */
        stream_if_ep_mclk_activate_complete(ep, TRUE);
    }
    else if(MCLK_MGR_RESPONSE_RELEASE_SUCCESS == status)
    {
        /* we have released mclk,
         * the mclk output must already be de-activated
         * so everything is done now */
        ep->state.audio.mclk_claimed = FALSE;
        stream_if_ep_mclk_activate_complete(ep, TRUE);
    }
    else
    {
        /* failing to claim/release mclk */
        stream_if_ep_mclk_activate_complete(ep, FALSE);
    }

    return TRUE;
}
#endif /* #ifdef INSTALL_MCLK_SUPPORT */

/**
 * \brief Connect to the endpoint.
 *
 * \param *endpoint pointer to the endpoint that is being connected
 * \param *Cbuffer_ptr pointer to the Cbuffer struct for the buffer that is being connected.
 * \param *ep_to_kick pointer to the endpoint which will be kicked after a successful
 *              run. Note: this can be different from the connected to endpoint when
 *              in-place running is enabled.
 * \param *start_on_connect return flag which indicates if the endpoint wants be started
 *              on connect. Note: The endpoint will only be started if the connected
 *              to endpoint wants to be started too.
 *
 * \return success or failure
 */
static bool audio_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    unsigned int hw_buf_size;    /* in samples */
    unsigned flags = 0;

    patch_fn_shared(stream_audio_hydra);
    hw_buf_size = get_audio_buffer_length(audio_vsm_get_sample_rate_from_sid(stream_external_id_from_endpoint(endpoint)), endpoint->direction, TRUE);

    /* cbuffer size is in words, but we use octets from here on in. */
    endpoint->ep_to_kick = ep_to_kick;

    /* If this endpoint is synchronised and connected to the same operator then
     * we only need one kick to that operator. */
    if ( (ep_to_kick != NULL) && IS_AUDIO_ENDPOINT_SYNCED(endpoint))
    {
        ENDPOINT *p_ep;
        bool this_ep_reached_first = FALSE;
        for (p_ep = endpoint->state.audio.head_of_sync; p_ep != NULL; p_ep = p_ep->state.audio.nep_in_sync)
        {
            if (p_ep == endpoint)
            {
                this_ep_reached_first = TRUE;
            }
            else if (stream_is_connected_to_same_entity(p_ep, endpoint))
            {
                /* The first endpoint in the list that is connected to the operator should do the kicking */
                if (this_ep_reached_first)
                {
                    p_ep->ep_to_kick = NULL;
                }
                else
                {
                    endpoint->ep_to_kick = NULL;
                }
                break;
            }
        }
    }

    /* Set MMU handle sample size to 32-bit unpacked until we teach audio endpoints
     * to decide to use specific sample types.
     */
#ifdef BAC32
#ifdef TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT
    flags = BUF_DESC_UNPACKED_32BIT_MASK;
#else /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#error "sample size support in streams is not implemented"
#endif /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#endif /* BAC32 */

    if(stream_direction_from_endpoint(endpoint) == SOURCE)
    {
        if (!endpoint->state.audio.is_overridden)
        {
            endpoint->state.audio.sink_buf = Cbuffer_ptr;
            endpoint->state.audio.source_buf = cbuffer_create_mmu_buffer(flags | BUF_DESC_MMU_BUFFER_HW_WR,
                    &hw_buf_size);

            if (endpoint->state.audio.source_buf == NULL)
            {
                return FALSE;
            }
        }
        else
        {
            /* The input buffer is already wrapped because the buffer_details is
             * changed to BUF_DESC_MMU_BUFFER_HW_WR when the endpoint is overridden*/
            endpoint->state.audio.sink_buf = NULL;
            endpoint->state.audio.source_buf = Cbuffer_ptr;
        }

        /* Configure hardware transformation flags, to produce desired data format */
        cbuffer_set_write_shift(endpoint->state.audio.source_buf, endpoint->state.audio.shift);
    }
    else
    {
        if (!endpoint->state.audio.is_overridden)
        {
            endpoint->state.audio.source_buf = Cbuffer_ptr;
            endpoint->state.audio.sink_buf = cbuffer_create_mmu_buffer(flags | BUF_DESC_MMU_BUFFER_HW_RD,
                    &hw_buf_size);
            if (endpoint->state.audio.sink_buf == NULL)
            {
                return FALSE;
            }
        }
        else
        {
            /* The input buffer is already wrapped because the buffer_details is
             * changed to BUF_DESC_MMU_BUFFER_HW_WR when the endpoint is overridden*/
            endpoint->state.audio.source_buf = NULL;
            endpoint->state.audio.sink_buf = Cbuffer_ptr;
        }
        /* Fill the buffer with silence so that the audio hardware reads silence
         * when it is connected. */
        cbuffer_flush_and_fill(endpoint->state.audio.sink_buf, get_ep_buffer_zero_value(endpoint));
        /* Configure hardware transformation flags, to produce desired data format */
        cbuffer_set_read_shift(endpoint->state.audio.sink_buf, endpoint->state.audio.shift);
    }

    if (!endpoint->state.audio.is_overridden)
    {
        if(stream_direction_from_endpoint(endpoint) == SINK)
        {

            /* Every ep created single-channel cbops. Now we arrived at the point when the cbops in the chain
               get created and set up, with buffer and alg params hooked in.

             1. If connect() is done before sync for this endpoint having been done, then
                connect is done on just the "original" single-channel cbops,
                adding buffer info etc. as in the old days...

                When later on the sync is done, we consolidate the various single-channel cbops into
                a multi-channel cbops, based on existing single-channel cbops information on buffers - but
                we only use sync head's information for various alg parameters!

             2. If sync was done before connect (so currently connected EP is found in a sync group, i.e.
                it has non-NULL nep_in_sync or the ptr to sync head is not pointing to itself)
                then populate the already existing multi-channel cbops with the connect information as per above;
                the single-channel cbops in that case was already destroyed for this endpoint and
                its cbop ptr field already points to the multi-channel cbops chain!

             */

            if((endpoint->state.audio.head_of_sync != endpoint) || (endpoint->state.audio.nep_in_sync != NULL))
            {
                /* This EP has already been added to a sync group, so "plug" its connection info into the multi-channel
                 * cbops chain created by the "add to sync" operation, which removed the single-channel cbops originally created for
                 * this endpoint.
                 */

                /* This just connects channel information up, cbops chain was already created! */
                if (!cbops_mgr_connect_channel(endpoint->cbops, endpoint->state.audio.channel,
                        endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf))
                {
                    cbuffer_destroy(endpoint->state.audio.sink_buf);
                    endpoint->state.audio.sink_buf = NULL;
                    return FALSE;
                }
            }
            else
            {
                /* It's still a standalone endpoint, so connect as a single-channel cbops chain from scratch as in the older days.
                 * Fill in initial parameters cbops will use to communicate between themselves and with the endpoint(s).
                 */
                CBOP_VALS vals;
                set_endpoint_cbops_param_vals(endpoint, &vals);

                if (!cbops_mgr_connect(endpoint->cbops, 1, &endpoint->state.audio.source_buf, &endpoint->state.audio.sink_buf, &vals))
                {
                    cbuffer_destroy(endpoint->state.audio.sink_buf);
                    endpoint->state.audio.sink_buf = NULL;
                    return FALSE;
                }
            }
        }
        else /* It is a source EP */
        {
            /* If source already in a sync group, then connect info in multi-channel cbops */
            if((endpoint->state.audio.head_of_sync != endpoint) || (endpoint->state.audio.nep_in_sync != NULL))
            {
                /* This ep has already been added to a sync group, so "plug" its connection info into the multi-channel
                 * cbops chain created by the "add to sync" operation, which removed the single-channel cbops originally created for
                 * this endpoint.
                 */

                /* This just connects channel information up, cbops chain was already created! */
                if (!cbops_mgr_connect_channel(endpoint->cbops, endpoint->state.audio.channel,
                        endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf))
                {
                    cbuffer_destroy(endpoint->state.audio.source_buf);
                    endpoint->state.audio.source_buf = NULL;
                    return FALSE;
                }
            }
            else
            {
                /* Standalone endpoint, with its single-channel cbops created as in the older days.
                 * Using a barely populated common vals struct, the only values currently used by in-chain is
                 * rate adjust amount ptr and shift amount (also set inside cbops mgr). In future if vals carries any relevant value for whatever
                 * input chain operator, then fill fields here.
                 */
                CBOP_VALS vals;
                vals.rate_adjustment_amount = &(endpoint->state.audio.rm_adjust_amount);
                vals.shift_amount = 0;

                if (!cbops_mgr_connect(endpoint->cbops, 1, &endpoint->state.audio.source_buf, &endpoint->state.audio.sink_buf, &vals))
                {
                    cbuffer_destroy(endpoint->state.audio.source_buf);
                    endpoint->state.audio.source_buf = NULL;
                    return FALSE;
                }
            }
        }
        /* Initialise any rateadjust to passthrough as haven't been asked to rateadjust yet */
        cbops_mgr_rateadjust_passthrough_mode(endpoint->cbops, TRUE);
    }

    *start_on_connect = TRUE;
    return TRUE;
}
/**
 * \brief Disconnects from an endpoint and stops the data from flowing
 *
 * \param *endpoint pointer to the endpoint that is being disconnected
 *
 * \return success or failure
 */
static bool audio_disconnect(ENDPOINT *endpoint)
{
    patch_fn_shared(stream_audio_hydra);
    if (!endpoint->state.audio.is_overridden)
    {
        if((endpoint->state.audio.head_of_sync != endpoint) || (endpoint->state.audio.nep_in_sync != NULL))
        {
            /* If endpoint in a sync group, then mark the channel as unused in cbops.
             * The endpoint will keep hold of the same channel within the multichannel
             * cbop as the channel number was allocated in the synchronise routine */
            cbops_mgr_set_unused_channel(endpoint->cbops, endpoint->state.audio.channel);
        }
        else
        {
            /* For stand alone endpoints, disconnect the cbops */
            cbops_mgr_disconnect(endpoint->cbops);
        }
    }


    /* Wipe the cbuffer ptrs held in audio state just in case someone treats it as valid */
    if (SOURCE == endpoint->direction)
    {
        endpoint->state.audio.sink_buf = NULL;
        if (!endpoint->state.audio.is_overridden)
        {
            cbuffer_destroy(endpoint->state.audio.source_buf);
        }
        endpoint->state.audio.source_buf = NULL;
    }
    else
    {
        endpoint->state.audio.source_buf = NULL;
        if (!endpoint->state.audio.is_overridden)
        {
            cbuffer_destroy(endpoint->state.audio.sink_buf);
        }
        else
        {
            /* If the sink is the head of the sync group, it silences all the endpoints
             * in the group. Otherwise the endpoint silences itself only.
             */
            if(IS_ENDPOINT_HEAD_OF_SYNC(endpoint))
            {
                ENDPOINT *synced;
                /* At stop the output buffer is filled with silence. This doesn't get
                 * called when we're overridden so ensure it happens here in case the
                 * responsible party didn't do it.
                 */
                for (synced = endpoint; synced != NULL;
                                        synced = synced->state.audio.nep_in_sync)
                {
                    /* The buffer may have been NULLified already, if the endpoint in the group
                     * was disconnected. In that case, avoid trying to silence it.
                     */
                    if(synced->state.audio.sink_buf != NULL)
                    {
                        cbuffer_flush_and_fill(synced->state.audio.sink_buf,
                                               get_ep_buffer_zero_value(synced));
                    }
                }
            }
            else if(endpoint->state.audio.sink_buf != NULL)
            {
                cbuffer_flush_and_fill(endpoint->state.audio.sink_buf,
                                       get_ep_buffer_zero_value(endpoint));
            }
        }

        endpoint->state.audio.sink_buf = NULL;
    }

    /* Reset ep_to_kick to it's default state of no endpoint to kick*/
    endpoint->ep_to_kick = NULL;
    /* Clear the override flag */
    endpoint->state.audio.is_overridden = FALSE;
    return TRUE;
}

/**
 * \brief Obtains details of the buffer required for this connection
 *
 * \param endpoint pointer to the endpoint from which the buffer
 *        information is required
 * \param details pointer to the BUFFER_DETAILS structure to be populated.
 *
 * \return TRUE/FALSE success or failure
 *
 */
static bool audio_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{
    patch_fn_shared(stream_audio_hydra);
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }

    /* Get required buffer size based on sample rate and kick period */
    details->b.buff_params.size = get_audio_buffer_length(
            audio_vsm_get_sample_rate_from_sid(stream_external_id_from_endpoint(endpoint)), endpoint->direction, FALSE);

    if (endpoint->state.audio.is_overridden)
    {
        if (SOURCE == endpoint->direction)
        {
            details->b.buff_params.flags = BUF_DESC_MMU_BUFFER_HW_WR;
        }
        else
        {
            details->b.buff_params.flags = BUF_DESC_MMU_BUFFER_HW_RD;
        }
        /* Set MMU handle sample size to 32-bit unpacked until we teach audio endpoints
         * to decide to use specific sample types.
         */
#ifdef BAC32
#ifdef TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT
        details->b.buff_params.flags |= BUF_DESC_UNPACKED_32BIT_MASK;
#else /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#error "sample size support in streams is not implemented"
#endif /* TODO_CRESCENDO_STREAMS_SAMPLE_SIZE_SUPPORT */
#endif /* BAC32 */
    }
    else
    {
#ifdef TIMED_PLAYBACK_MODE
        if (SINK == endpoint->direction)
        {
            /* check if any of the synced endpoints are already connected.*/
            ENDPOINT *synced_ep = endpoint->state.audio.head_of_sync;
            details->supports_metadata = TRUE;
            details->metadata_buffer = NULL;

            while(synced_ep)
            {
                /* return the first connected buffer*/
                if (synced_ep->state.audio.source_buf)
                {
                    details->metadata_buffer = synced_ep->state.audio.source_buf;
                    break;
                }
                synced_ep = synced_ep->state.audio.nep_in_sync;
            }
        }
#endif
#ifdef AUDIO_SOURCE_GENERATE_METADATA
        if (SOURCE == endpoint->direction)
        {
            /* check if any of the synced endpoints are already connected.
             * metadata is supported only if it has been enabled by the
             * user, user should enable it for all the synchronised endpoint
             * for this purpose we only look at head of sync group.
             */
            ENDPOINT *synced_ep = endpoint->state.audio.head_of_sync;
            if(synced_ep->state.audio.generate_metadata)
            {
                details->supports_metadata = TRUE;
                details->metadata_buffer = NULL;
                while(synced_ep)
                {
                    /* return the first connected buffer*/
                    if (synced_ep->state.audio.sink_buf)
                    {
                        details->metadata_buffer = synced_ep->state.audio.sink_buf;
                        break;
                    }
                    synced_ep = synced_ep->state.audio.nep_in_sync;
                }
            }
        }
#endif /* #ifdef AUDIO_SOURCE_GENERATE_METADATA */
        details->b.buff_params.flags = BUF_DESC_SW_BUFFER;
    }


    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    details->can_override = TRUE;
    details->wants_override = TRUE;

    return TRUE;
}

/**
 * \brief Starts a kick interrupt source based off this audio endpoint.
 *
 * \param ep pointer to the endpoint which is responsible for scheduling
 * the kick.
 * \param ko pointer to the KICK_OBJECT that called start.
 *
 *  \return success or failure
 */
static bool audio_start (ENDPOINT *ep, KICK_OBJECT *ko)
{
    endpoint_audio_state* ep_audio;
    mmu_handle handle;
    /* in samples */
    unsigned int offset = 0;
    unsigned int max_offset;

    ep_audio = &ep->state.audio;

    /* If endpoint is already running don't do anything more */
    if (ep_audio->running)
    {
        return TRUE;
    }

    /* Retrieve the buffer handle, max offset and initial offset for the endpoint */
    if (ep->direction == SOURCE)
    {
        handle = cbuffer_get_write_mmu_handle(ep->state.audio.source_buf);
    }
    else
    {
        handle = cbuffer_get_read_mmu_handle(ep->state.audio.sink_buf);
    }

    max_offset = mmu_buffer_get_size(handle);
    /* Set the offset to halfway through the buffer,
     * accounting for the fact that the read offset may have moved
     */
    offset = (mmu_buffer_get_handle_offset(handle) + (max_offset / 2)) % max_offset;

    if (IS_AUDIO_ENDPOINT_SYNCED(ep) && ep->direction == SOURCE)
    {
        /* If starting a source endpoint that is part of sync group, update the
         * same offset for all the connected source endpoints in the sync group */
        ENDPOINT *synced;
        for (synced = ep->state.audio.head_of_sync; synced != NULL;  synced = synced->state.audio.nep_in_sync)
        {
            if (synced->connected_to != NULL && synced->direction == SOURCE)
            {
                audio_vsm_update_initial_buffer_offset(stream_external_id_from_endpoint(synced), offset);
            }
        }
    }

    L3_DBG_MSG2("Activating EP: %08X with offset %d", stream_external_id_from_endpoint(ep), offset);
    if (!audio_vsm_activate_sid(stream_external_id_from_endpoint(ep), handle, offset, max_offset))
    {
        return FALSE;
    }

    /* Set the running flag */
    ep_audio->running = TRUE;

    /* If endpoint is overridden, don't do anything else */
    if (ep_audio->is_overridden)
    {
        return TRUE;
    }

    /* No hard deadline, so we kick as and when data arrives */
    if (IS_ENDPOINT_HEAD_OF_SYNC(ep) && (ep_audio->monitor_threshold != 0))
    {
        bool ok;
        unsigned int sample_rate;
        /* Set up a monitor to kick us whenever a certain number of
         * samples have arrived. This needs to be the same as the input
         * block size of the attached operator. Handily, that should
         * already be stashed in source->state.audio.monitor_threshold
         * (assuming that the far side *is* an operator, which it should be). */
        PL_PRINT_P1(TR_STREAM, "stream_IS_AUDIO_ENDPOINT_start: starting audio monitor every %d samples\n",
                        ep_audio->monitor_threshold);

        sample_rate = (unsigned int)
                      audio_vsm_get_sample_rate_from_sid(stream_external_id_from_endpoint(ep));

        /* Configure kick interrupts' handling to be deferred,
         * before enabling them */
        stream_set_deferred_kick(ep, ep->deferred.config_deferred_kick);

        if(ep->direction == SOURCE)
        {
            unsigned buf_offset;
            ENDPOINT *synced;

            /* Get read and write pointers sync'ed. When the first monitor interrupt
             * arrives, HW wrote a block worth of data. We always move the
             * pointer that we own. */
            /* Include an extra bit of headroom to give time to ensure the cbop
             * processing doesn't trample the write pointer. Start the interrupt
             * source first so that an interrupt here doesn't erode the priming
             * level. */
            ok = stream_monitor_int_wr_enable(ep,
                     cbuffer_get_write_mmu_handle(ep_audio->source_buf),
                     ko);
            cbuffer_move_read_to_write_point(ep_audio->source_buf, CBOP_MIN_HEADROOM_SAMPLES);
            buf_offset = cbuffer_get_read_offset(ep_audio->source_buf);

            for (synced = ep; synced != NULL;  synced = synced->state.audio.nep_in_sync)
            {
                int *addr = synced->state.audio.source_buf->base_addr + buf_offset;
                synced->state.audio.source_buf->read_ptr = addr;
                cbuffer_move_write_to_read_point(synced->state.audio.sink_buf, 0);

                synced->state.audio.sample_rate = sample_rate;

#ifdef AUDIO_SOURCE_GENERATE_METADATA
                /* Align any metadata indexes with the pointers that have been moved.
                 * This probably only needs to happen for the head, but it's
                 * easier to do it for each channel. Only align the write pointer
                 * as that's the one that is moved here.
                 */
                buff_metadata_align_to_buff_write_ptr(ep_audio->sink_buf);
                ep_audio->last_tag_left_words = 0;

                /* set the minimum length of tags, in case the monitor level has been set to a very
                 * small value we make sure that the tag lengths aren't very small,
                 * this is set by AUDIO_SOURCE_TOA_MIN_TAG_LEN.
                 * Note: we also make sure that the full buffer size can convey 4 min-length tags,
                 * so the limitation will be further relaxed for low buffer sizes.
                 */
                ep_audio->min_tag_len = (cbuffer_get_size_in_words(ep_audio->sink_buf) -1) >> 2;
                if(ep_audio->min_tag_len > AUDIO_SOURCE_TOA_MIN_TAG_LEN)
                {
                    ep_audio->min_tag_len = AUDIO_SOURCE_TOA_MIN_TAG_LEN;
                }
#endif /* AUDIO_SOURCE_GENERATE_METADATA */

                synced->state.audio.sync_started = FALSE;
            }
        }
        else
        {
            unsigned buf_offset;
            ENDPOINT *synced;

            /* Calculate a cbop headroom amount based on sampling rate and a default processing time allowance */
            unsigned initial_amount = ep_audio->monitor_threshold +
                                      AUDIO_RM_HEADROOM_AMOUNT +
                                      (STREAM_KICK_PERIOD_FROM_USECS(CBOP_PROCESSING_TIME_ALLOWANCE_IN_USECS) /
                                       ( (unsigned int)( (unsigned long)STREAM_KICK_PERIOD_FROM_USECS(SECOND) /
                                                         sample_rate ) ) );

            /* get read and write pointers set such that by first interrupt,
             * the HW consumes a block of silence. After that, it will
             * silence insert until real data turns up. We move the pointers that
             * we own.
             */
            /* Include an extra bit of headroom to give time to perform the cbop processing before the read pointer
             * catches up. Start the interrupt source first so that an interrupt
             * here doesn't erode the priming level. */
            ok = stream_monitor_int_rd_enable(ep,
                         cbuffer_get_read_mmu_handle(ep_audio->sink_buf),
                         ko);
            cbuffer_move_write_to_read_point(ep_audio->sink_buf, initial_amount);
            buf_offset = cbuffer_get_write_offset(ep_audio->sink_buf);

            for (synced = ep; synced != NULL;  synced = synced->state.audio.nep_in_sync)
            {
                int *addr = synced->state.audio.sink_buf->base_addr + buf_offset;
                synced->state.audio.sink_buf->write_ptr = addr;
                cbuffer_move_read_to_write_point(synced->state.audio.source_buf, 0);

                synced->state.audio.sample_rate = sample_rate;

#ifdef INSTALL_METADATA
                /* Align any metadata indexes with the pointers that have been moved.
                 * This probably only needs to happen for the head, but it's
                 * easier to do it for each channel. Only align the read pointer
                 * as that's the one that is moved here. It's dangerous
                 * to align the write pointer as the operator may have added a delay
                 * to the write index already, and we'll trash that. */
                buff_metadata_align_to_buff_read_ptr(ep_audio->source_buf);
#endif /* INSTALL_METADATA */

                synced->state.audio.sync_started = FALSE;

                /* Initialise the rate matching features. */
                synced->state.audio.rm_adjust_amount = 0;
                synced->state.audio.rm_diff = 0;
            }
        }
        /* Re-init the cbops with the Cbuffers as they are at this moment!
           Otherwise they would carry on from where they left off
           (if there was a previous stop, then start of the endpoint).
         */
        cbops_mgr_buffer_reinit(ep->cbops);

        rate_measure_set_nominal_rate(&ep_audio->rm_measure, sample_rate);
        rate_measure_stop(&ep_audio->rm_measure);
        /* TODO higher precision */
        ep_audio->rm_expected_time = ( (RATE_TIME)ep_audio->rm_measure.sample_period
                                  << RATE_SAMPLE_PERIOD_TO_TIME_SHIFT)
                                * ep_audio->monitor_threshold;
        ep_audio->rm_int_time = ep_audio->rm_expected_time;
        ep_audio->rm_period_start_time = (RATE_TIME)hal_get_time()
                                         << RATE_TIME_EXTRA_RESOLUTION;

#ifdef TIMED_PLAYBACK_MODE
        if (ep_audio->source_buf->metadata)
        {
            ep_audio->data_flow_started = FALSE;
            ep_audio->use_timed_playback = FALSE;
        }
#endif
#ifdef INSTALL_AUDIO_EP_CLRM
        rate_match_init(&ep_audio->rm_control, &ep_audio->rm_measure, NULL,
                        &rate_measurement_validity_default, sample_rate);
        ep_audio->rm_control.enable_adapt = ep_audio->rm_enable_clrm_adaptive;
#endif /* INSTALL_AUDIO_EP_CLRM */

        if (ok)
        {
            ep_audio->monitor_enabled = TRUE;
        }
        else
        {
            /* We don't have any free monitors. Since the HW provides one monitor
             * per PCM slot, it implies that we probably shouldn't have ever
             * created this audio endpoint in the first place. */
            panic_diatribe(PANIC_AUDIO_STREAM_MONITOR_TOO_FEW_RESOURCES, stream_external_id_from_endpoint(ep));
        }
    }

    return TRUE;
}

/**
 * \brief Stops the kick interrupt source that was started by this endpoint.
 *
 * \param ep Pointer to the endpoint this function is being called on.
 *
 * \return success or failure
 */
static bool audio_stop (ENDPOINT *ep)
{
    KICK_OBJECT *kick_object;

    patch_fn_shared(stream_audio_hydra);
    if(!ep->state.audio.running)
    {
        /* The kick source is already stopped */
        return FALSE;
    }

    /* Deactivate the audio before unwrapping the mmu buffer as the audio code
     * does some tidying up of the buffer. */
    /* If the deactivation failed then don't complete the stop as endpoint
     * hardware may still be in use. */
    if (!audio_vsm_deactivate_sid(stream_external_id_from_endpoint(ep)))
    {
        return FALSE;
    }

    /* Update the running flag */
    ep->state.audio.running = FALSE;

    /* If endpoint is overridden, don't do anything else */
    if (ep->state.audio.is_overridden)
    {
        /*Until disconnect the ep is overridden*/
        return TRUE;
    }

    if (IS_ENDPOINT_HEAD_OF_SYNC(ep))
    {
        /* For audio endpoints we don't have hard deadlines
         * which means kick_object->ep_kick == kick_object->ep_sched.
         */
        kick_object = kick_obj_from_sched_endpoint(ep);
        if (NULL == kick_object)
        {
            /* The function call above is looking up the KICK_OBJECT that
             * called this function, so it's very bad if it doesn't exist. */
            panic_diatribe(PANIC_AUDIO_STREAM_MONITOR_STOP_FAILED,
                                stream_external_id_from_endpoint(ep));
        }

        if(ep->state.audio.monitor_enabled)
        {
            bool ok;
            if(ep->direction == SOURCE)
            {
                ok = stream_monitor_int_wr_disable(kick_object);
            }
            else
            {
                ok = stream_monitor_int_rd_disable(kick_object);
            }
            if (!ok)
            {
                panic_diatribe(PANIC_AUDIO_STREAM_MONITOR_STOP_FAILED,
                        stream_external_id_from_endpoint(ep));
            }
            ep->state.audio.monitor_enabled = FALSE;
            stream_set_deferred_kick(ep, FALSE);
        }
    }

#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
    if (ep->state.audio.rm_support == RATEMATCHING_SUPPORT_HW)
    {
        enact_audio_rm_warp(ep, 0);
    }
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */

    if(ep->direction == SINK)
    {
        ENDPOINT *synced;
        /* Leave behind a pristine buffer if this is a sink - so only silence is pumped out.
         * The pointers are not bothered with, HW can free-run and start will snap the pointers
         * to proper places as per priming.
         */
        for (synced = ep; synced != NULL; synced = synced->state.audio.nep_in_sync)
        {
            cbuffer_flush_and_fill(synced->state.audio.sink_buf, get_ep_buffer_zero_value(synced));
        }
    }

    return TRUE;
}

#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
/**
 * Prepare to apply a HW ratematch related setting to all KCODEC channels
 * belonging to endpoints in the sync group. This should really match
 * the response given in stream_ratematch_mgr.c: audio_clock_source_same().
 * Limit to the same direction as the endpoint ep. Chris Avery says that
 * currently (Jun'16) you can't put a source and sink device in the same
 * sync group.
 */
static bool audio_init_warp_update_descs(ENDPOINT* ep)
{
    ENDPOINT* eps;
    WARP_UPDATE_DESC* warp_desc = &ep->state.audio.rm_update_desc;
    bool success;

    patch_fn_shared(stream_audio_hydra);

    audio_vsm_init_warp_update_desc(warp_desc, ep->direction == SINK);

    success = audio_vsm_add_warp_update_desc(warp_desc, stream_external_id_from_endpoint(ep));

    for (eps = ep->state.audio.head_of_sync;
            success && (eps != NULL);
            eps = eps->state.audio.nep_in_sync)
    {
        /* A FALSE return here would only mean that there is a device
         * in the sync group which is not covered by the same HW warp
         * mechanism. Not sure if that should fail the whole setup;
         * proceed for now.
         */
        audio_vsm_add_warp_update_desc(warp_desc, stream_external_id_from_endpoint(eps));
    }

    return success;
}

/**
 * adjust_audio_rate_warp
 */
static void adjust_audio_rate_warp(ENDPOINT *ep, int32 adjust_val)
{
    /* summarized from stream_audio_a7da_kas.c: adjust_audio_rate_iacc_snk(), adjust_audio_rate_iacc_src() */
    int use_val;
    endpoint_audio_state* audio = &ep->state.audio;

#ifdef STREAM_AUDIO_HYDRA_WARP_VERBOSE
    L2_DBG_MSG5("adjust_audio_rate((%d,%d,%d,%d,0x%02x) HW",
            get_hardware_type(ep), get_hardware_instance(ep), get_hardware_channel(ep), ep->direction,
            audio->rm_update_desc.channel_mask
        );
#endif

    /*
     * A positive adjust_val means increase the source sampling
     * frequency, or decrease the sink sampling frequency.
     * Subsequently, deal in positive values for increases.
     */
    if (ep->direction == SINK)
    {
        adjust_val = - adjust_val;
    }

    /* Determine the actual bitfield required for the warp adjustment. The HW warp has +-3.125% adjustment available,
     * the adjust_val is for a wider range (and is sign-extended in a 32-bit word). The upshot is that (for the 32-bit
     * input representation), we need to arithmetic-shift "adjust_val" to give a 13-bit warp value in the LSBs.
     * Further info is in CS-236709-SP-E-Marco_Audio_Analog_APB_Register_Map
     * Note: the new warp value must be corrected by adding the current warp to the new warp setting, it is
     *       a rough approximation to: w_corr = 1 - (1-w_prev)*(1-w_new).
     */

#ifdef TIMED_PLAYBACK_MODE
    if (ep->state.audio.use_timed_playback)
    {
        /*
         * Timed playback warp values are relative to nominal rate, not the current warped rate,
         * so they must be used directly and not added to the previous value
         */
        use_val = adjust_val;
    }
    else
#endif
    {
        use_val = audio->rm_adjust_prev + (int)adjust_val;
    }
    set_rate_warp(ep, use_val);
}

static void set_rate_warp(ENDPOINT *ep, int use_val)
{
    endpoint_audio_state* audio = &ep->state.audio;

    patch_fn_shared(stream_audio_hydra);
    if (use_val >= FRACTIONAL(0.031250))
    {
        use_val = FRACTIONAL(0.031250) - 1;
    }
    else if (use_val < - FRACTIONAL(0.031250))
    {
        use_val = - FRACTIONAL(0.031250);
    }

    if (use_val != audio->rm_adjust_prev)
    {
        audio_vsm_set_warp(
                &audio->rm_update_desc,
                use_val,
                &audio->rm_hw_sp_deviation);
#ifdef STREAM_AUDIO_HYDRA_WARP_VERBOSE
        L3_DBG_MSG3("set_rate_warp rm_adjust_prev: %d -> %d, report: %d",
                    audio->rm_adjust_prev, use_val, audio->rm_hw_sp_deviation);
#endif
        audio->rm_adjust_prev = use_val;
    }
}
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */

static void adjust_audio_rate(ENDPOINT *ep, int32 adjust_val)
{
    patch_fn_shared(stream_audio_hydra);

    if (pl_abs_i32(adjust_val) >= FRACTIONAL(0.031250))
    {
        L3_DBG_MSG2("Audio adjustment : %d EP: %08X : discarded", (int)(adjust_val), (uintptr_t)ep);
        return;
    }

    L3_DBG_MSG2("Audio adjustment : %d EP: %08X", (int)(adjust_val), (uintptr_t)ep);

    /* The parameter adjust_val is a Q0.23 value (24bit arch) resp. Q0.31
     * (32bit arch), representing (real sink rate/real source rate) - 1.
     * I.e. for a SW rate adjuster:
     *  >0 means fractionally interpolate, <0 means fractionally decimate.
     * For a HW RM source:
     *  >0 means increase sampling frequency, <0 means decrease.
     * For a HW RM sink:
     *  >0 means decrease sampling frequency, <0 means increase.
     */
    switch (ep->state.audio.rm_support)
    {
        case RATEMATCHING_SUPPORT_SW:
#ifdef STREAM_AUDIO_HYDRA_WARP_VERBOSE
            L3_DBG_MSG5("adjust_audio_rate((%d,%d,%d,%d), %d) SW",
                    get_hardware_type(ep), get_hardware_instance(ep), get_hardware_channel(ep), ep->direction,
                    (int)adjust_val
                );
#endif
            ep->state.audio.rm_adjust_amount = ((int)adjust_val);
            break;

#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
        case RATEMATCHING_SUPPORT_HW:
            adjust_audio_rate_warp(ep, adjust_val);
            break;
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */

        default:
            break;
    }
    return;
}

#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
static void enact_audio_rm_warp(ENDPOINT *ep, uint32 val)
{
#ifdef STREAM_AUDIO_HYDRA_WARP_VERBOSE
    L3_DBG_MSG5("enact_audio_rm_warp (%d,%d,%d,%d) %d",
            get_hardware_type(ep), get_hardware_instance(ep),
            get_hardware_channel(ep), ep->direction,
            val);
#endif
    if (val)
    {
        /* Convert the sync group into a channel bitmask.
         * This is safe because sync groups do not change while a
         * chain is running, and lazy because it will only be done
         * for endpoints which subsequently perform HW RM.
         */
        audio_init_warp_update_descs(ep);
    }
    else
    {
        endpoint_audio_state* audio = &ep->state.audio;

        /* Stop hardware rate adjustment on this endpoint */
        audio_vsm_set_warp(&audio->rm_update_desc, 0, &audio->rm_hw_sp_deviation);
        audio->rm_adjust_prev = 0;
        audio->rm_update_desc.channel_mask = 0;
    }
}
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */

static bool enact_audio_rm(ENDPOINT* endpoint, uint32 value)
{
    patch_fn_shared(stream_audio_hydra);

#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
    if (endpoint->state.audio.rm_support == RATEMATCHING_SUPPORT_HW)
    {
        enact_audio_rm_warp(endpoint, value);
        return TRUE;
    }
    else
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */
    {
#ifdef INSTALL_AUDIO_EP_CLRM
        /* When enabling rate adjustment, obtain pointer to the rate_adjust_and_shift
         * operator, in order to use cbops_sra_set_rate_adjust()
         */
        if (value)
        {
            if (endpoint->cbops == NULL || endpoint->cbops->graph == NULL)
            {
                return FALSE;
            }
            cbops_graph* head = endpoint->cbops->graph;
            struct endpoint_audio_state* ep_audio = &endpoint->state.audio;

            ep_audio->rate_adjust_op =
                    find_cbops_op(head, cbops_rate_adjust_table);
            endpoint->state.audio.rate_adjust_op_num_channels =
                    cbops_get_num_inputs(head);

            if ((ep_audio->rate_adjust_op == NULL)
                || (ep_audio->rate_adjust_op_num_channels < 1))
            {
                return FALSE;
            }
        }
#endif /* INSTALL_AUDIO_EP_CLRM */
        return audio_configure_rm_enacting(endpoint, value);
    }
}

#ifdef INSTALL_AUDIO_EP_CLRM
/**
 * \brief Regulate sample rate to match the reference (closed loop rate matching)
 */
static bool match_audio_rate(ENDPOINT *ep, const RATE_RELATIVE_RATE* ref)
{
    endpoint_audio_state* ep_audio = &ep->state.audio;

    patch_fn_shared(stream_audio_hydra);

    TIME now = hal_get_time();
    int use_val = 0;

    if (rate_match_update(&ep_audio->rm_control, ref, &use_val, now))
    {
        switch(ep_audio->rm_support)
        {
            case RATEMATCHING_SUPPORT_SW:
                if (SINK == ep->direction)
                {
                    /* A positive deviation means increase rate. In case
                     * of a SRA at a sink, this means lowering the adjustment.
                     */
                    use_val = - use_val;
                }
#ifdef RATE_MATCH_DEBUG
                if (rate_match_is_trace_enabled(&ep_audio->rm_control))
                {
                    get_sp_deviation(ep);
                    rate_match_trace_client(&ep_audio->rm_control, use_val,
                                            ep_audio->rm_report_sp_deviation,
                                            ep_audio->rm_num_delayed_kicks);
                    ep_audio->rm_num_delayed_kicks = 0;
                }
#endif /* RATE_MATCH_DEBUG */
                if (ep_audio->rate_adjust_op != NULL)
                {
                    /* Set adjustment and disable built-in ramping */
                    cbops_sra_set_rate_adjust(
                            ep_audio->rate_adjust_op,
                            ep_audio->rate_adjust_op_num_channels,
                            use_val);
                }
                break;

#if defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM)
            case RATEMATCHING_SUPPORT_HW:
                set_rate_warp(ep, use_val);
#ifdef RATE_MATCH_DEBUG
                rate_match_trace_client(&ep_audio->rm_control,
                                        ep_audio->rm_adjust_prev,
                                        ep_audio->rm_hw_sp_deviation, 0);
#endif /* RATE_MATCH_DEBUG */
                break;
#endif /* defined(INSTALL_CODEC) || defined(INSTALL_DIGITAL_MIC) || defined(INSTALL_AUDIO_INTERFACE_PWM) */

            default:
                break;
        }

        /* Signal to ratematch manager to skip
         * EP_RATEMATCH_RATE/EP_RATEMATCH_ADJUSTMENT */
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

/* KCODEC_ADC_GAIN_FINE values for digital mics mapping the kymera monotonic gain levels
 * 0-17 to go from -30dB to +24dB supported by the KCODEC ADC digital gain block
 * Bit 15 is set to enable the fine gain KCODEC_ADC_GAIN_SELECT_FINE */
static const uint16 digmic_digital_gain_word[] =
{
    0x8001,/* (0)-30.10299957 dB */
    0x8002,/* (1)-24.08239965 dB */
    0x8003,/* (2)-20.56057447 dB */
    0x8004,/* (3)-18.06179974 dB */
    0x8006,/* (4)-14.53997456 dB */
    0x8008,/* (5)-12.04119983 dB */
    0x800B,/* (6)-9.275145863 dB */
    0x8010,/* (7)-6.020599913 dB */
    0x8017,/* (8)-2.868442846 dB */
    0x8020,/* (9)0.0000000000 dB */
    0x802D,/* (10)2.961250709 dB */
    0x8040,/* (11)6.020599913 dB */
    0x805B,/* (12)9.077828280 dB */
    0x8080,/* (13)12.04119983 dB */
    0x80B4,/* (14)15.00245054 dB */
    0x80FF,/* (15)18.02780404 dB */
    0x8168,/* (16)21.02305045 dB */
    0x81FF /* (17)24.06541844 dB */
};

/**
 * \brief Translate between gain scales
 *
 * Operators overriding endpoints think about gains in a Kymera-defined
 * scale with 1/60 dB steps. The Hydra audio driver code inherits from
 * BlueCore a 'friendly gain scale' with 3dB steps. This function
 * translates from the former to the latter.
 *
 * \param hardware Hardware type for which the gain scale is required.
 * \param kymera_gain Gain in 2's complement 32-bit format, 1/60 dB steps
 * \param is_dac Whether this for ADC (towards DSP) or DAC (away from DSP) --
 *   influences zero point of gain scale
 *
 * \return An unsigned number on the traditional 0-22 gain scale (3dB steps)
 */
static unsigned int kymera_to_bluecore_gain(STREAM_DEVICE hardware, uint32 kymera_gain, bool is_dac)
{
    unsigned traditional_gain;
    int32 signed_gain = (int32)kymera_gain; /* XXX hope this works */

    /* The absolute values are a bit arbitrary. We make some sort of
     * effort to fit in with previous conventions, although those
     * conventions are mostly rumour. */

    signed_gain /= 60*3; /* 1/60dB steps -> 3dB steps */

    /* 'Zero points' on these scales are rumour. Source:
     * http://ukbugdb/B-215628#h10433714 */
    if (is_dac)
    {
        /* There's a pretty clear consensus that 15 is the zero point
         * for DACs. See for instance B-220681 */
        signed_gain += 15;
    }
    else
    {
        /* It's less clear what the zero point is for ADCs.
         * Pick a popular value. */
        signed_gain += 9;
    }

    /* The BlueCore gain scale traditionally has values 0-22.
     * We leave clipping at the high end to the audio driver, in case
     * higher levels are ever supported.
     * However, the interface is unsigned, so we clip at the low end. */
    traditional_gain = signed_gain > 0 ? (unsigned int)signed_gain : 0;
    if(hardware == STREAM_DEVICE_DIGITAL_MIC)
    {
        /* digital mics expect the raw gain value. Other hardware devices can manage
         * the platform dependent value on their own.
         */
        if(traditional_gain>=ARRAY_DIM(digmic_digital_gain_word))
        {
            traditional_gain = ARRAY_DIM(digmic_digital_gain_word)-1;
        }
        return digmic_digital_gain_word[traditional_gain];
    }
    return traditional_gain;
}

/**
 * \brief configure an audio endpoint with a key and value pair
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value value to which the key is to be configured
 *
 * \return Whether the request succeeded.
 *
 */
static bool audio_configure(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    endpoint_audio_state* ep_audio = &endpoint->state.audio;

    patch_fn_shared(stream_audio_hydra);
    if((key & ENDPOINT_INT_CONFIGURE_KEYS_MASK) != 0)
    {
        switch (key)
        {
        case EP_DATA_FORMAT:
            return audio_set_data_format(endpoint, (AUDIO_DATA_FORMAT)value);

        case EP_OVERRIDE_ENDPOINT:
        {
            /* Set the logical value of override flag. */
            ep_audio->is_overridden = (bool)value;

            return TRUE;
        }

        case EP_CBOPS_PARAMETERS:
        {
            bool retval;
            CBOPS_PARAMETERS *parameters = (CBOPS_PARAMETERS *)(uintptr_t) value;

            if (parameters)
            {
                /* cbops_mgr should not be updated when endpoint is running. */
                if (!endpoint->is_enabled &&  endpoint->cbops && \
                    opmgr_override_pass_cbops_parameters(parameters, endpoint->cbops,
                                                         ep_audio->source_buf,
                                                         ep_audio->sink_buf))
                {
                    retval = TRUE;
                }
                else
                {
                    retval = FALSE;
                }
            }
            else
            {
                /* Panic can return in unit test*/
                retval = FALSE;
                panic_diatribe(PANIC_AUDIO_STREAM_INVALID_CONFIGURE_KEY,
                                                                endpoint->id);
            }

            free_cbops_parameters(parameters);
            return retval;
        }
        case EP_BLOCK_SIZE:
        {
            unsigned buffsize;
            /* Set monitor threshold to match block size,
             * but constrain to half the buffer size
             */
            if (SINK == endpoint->direction)
            {
                buffsize = cbuffer_get_size_in_words(ep_audio->sink_buf);
            }
            else
            {
                buffsize = cbuffer_get_size_in_words(ep_audio->source_buf);
            }
            if (value > buffsize / 2)
            {
                L2_DBG_MSG2("audio_configure EP_BLOCK_SIZE value = %u buffer size = %u, constraining to buffsize/2", value, buffsize);
                value = buffsize / 2;
            }
            ep_audio->monitor_threshold = (int)value;
            return TRUE;
        }
        case EP_RATEMATCH_ADJUSTMENT:
            adjust_audio_rate(endpoint, (int32)value);
            return TRUE;
        case EP_RATEMATCH_REFERENCE:
#ifdef INSTALL_AUDIO_EP_CLRM
#ifdef TIMED_PLAYBACK_MODE
            /* Timed playback takes precedence */
            if (ep_audio->use_timed_playback)
            {
                /* Backwards compatible for now: i.e. proceed to calculate
                 * and get EP_RATEMATCH_ADJUSTMENT even though I don't think
                 * in TTP we want to use that value.
                 */
                return FALSE;
            }
            else
#endif
            if (endpoint->state.audio.rm_enable_clrm_match)
            {
                const ENDPOINT_RATEMATCH_REFERENCE_PARAMS* params =
                        (const ENDPOINT_RATEMATCH_REFERENCE_PARAMS*)(uintptr_t)value;
                /* Returning FALSE tells the ratematch manager
                 * to fall back to EP_RATEMATCH_ADJUSTMENT */
                return match_audio_rate(endpoint, &params->ref);
            }
            else
            {
                return FALSE;
            }
#else /* INSTALL_AUDIO_EP_CLRM */
            return FALSE;
#endif /* INSTALL_AUDIO_EP_CLRM */
        case EP_RATEMATCH_ENACTING:
            return enact_audio_rm(endpoint, value);
        case EP_SET_INPUT_GAIN:
        {
            stream_config_key cfg_key;
            STREAM_DEVICE hardware = get_hardware_type(endpoint);

            switch(hardware)
            {
            case STREAM_DEVICE_CODEC:
                cfg_key = ACCMD_CONFIG_KEY_STREAM_CODEC_INPUT_GAIN;
                break;
            case STREAM_DEVICE_DIGITAL_MIC:
                cfg_key = ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_INPUT_GAIN;
                break;
            default:
                return FALSE;
            }
            return audio_vsm_configure_sid(stream_external_id_from_endpoint(endpoint),
                                      cfg_key, kymera_to_bluecore_gain(hardware, value, FALSE));
        }
        case EP_SET_OUTPUT_GAIN:
            if(get_hardware_type(endpoint) == STREAM_DEVICE_CODEC)
            {
                return audio_vsm_configure_sid(stream_external_id_from_endpoint(endpoint),
                                           ACCMD_CONFIG_KEY_STREAM_CODEC_OUTPUT_GAIN,
                                           kymera_to_bluecore_gain(STREAM_DEVICE_CODEC, value, TRUE));
            }
            else
            {
                return FALSE;
            }

        default:
            return FALSE;
        }
    }
    else
    {
        switch (key)
        {
#ifdef INSTALL_UNINTERRUPTABLE_ANC
            /* Set the ANC instance associated with the endpoint */
            case ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE:

            /* Set the ANC input path associated with the endpoint */
            case ACCMD_CONFIG_KEY_STREAM_ANC_INPUT:

            /* Configure the ANC DC filter/SM LPF */
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFA_DC_FILTER_ENABLE:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFB_DC_FILTER_ENABLE:
            case ACCMD_CONFIG_KEY_STREAM_ANC_SM_LPF_FILTER_ENABLE:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFA_DC_FILTER_SHIFT:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFB_DC_FILTER_SHIFT:
            case ACCMD_CONFIG_KEY_STREAM_ANC_SM_LPF_FILTER_SHIFT:

            /* Configure the ANC path gains */
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFA_GAIN:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFB_GAIN:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FB_GAIN:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFA_GAIN_SHIFT:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFB_GAIN_SHIFT:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FB_GAIN_SHIFT:

            /* Enable adaptive ANC */
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFA_ADAPT_ENABLE:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FFB_ADAPT_ENABLE:
            case ACCMD_CONFIG_KEY_STREAM_ANC_FB_ADAPT_ENABLE:

            /* Set ANC controls */
            case ACCMD_CONFIG_KEY_STREAM_ANC_CONTROL:
            {
                bool status;

                /* Is this a source or sink endpoint? */
                if (stream_direction_from_endpoint(endpoint) == SOURCE)
                {
                    status = audio_hwm_anc_source_configure(endpoint, (ACCMD_CONFIG_KEY)key, value);
                }
                else
                {
                    status = audio_hwm_anc_sink_configure(endpoint, (ACCMD_CONFIG_KEY)key, value);
                }

                return status;
            }
#endif /* INSTALL_UNINTERRUPTABLE_ANC*/

            case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_SW_ADJUST:
                ep_audio->rm_enable_sw_rate_adjust = (value != 0);
                return TRUE;
            case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_HW_ADJUST:
                ep_audio->rm_enable_hw_rate_adjust = (value != 0);
                return TRUE;
            case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_FAST_MEASUREMENT:
                ep_audio->rm_enable_clrm_measure = (value != 0);
                return TRUE;
            case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_FAST_MATCHING:
#ifdef INSTALL_AUDIO_EP_CLRM
                ep_audio->rm_enable_clrm_match = (value != 0);
                return TRUE;
#else /* INSTALL_AUDIO_EP_CLRM */
                return (value == 0);
#endif /* INSTALL_AUDIO_EP_CLRM */
            case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_DEFERRED_KICK:
                endpoint->deferred.config_deferred_kick = (value != 0);
                return TRUE;
            case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_VARIABLE_RESPONSIVENESS:
#ifdef INSTALL_AUDIO_EP_CLRM
                ep_audio->rm_enable_clrm_adaptive = (value != 0);
                if (ep_audio->running)
                {
                    ep_audio->rm_control.enable_adapt = ep_audio->rm_enable_clrm_adaptive;
                }
                return TRUE;
#else/* INSTALL_AUDIO_EP_CLRM */
                return (value == 0);
#endif /* INSTALL_AUDIO_EP_CLRM */
            case ACCMD_CONFIG_KEY_STREAM_RM_RATE_MATCH_TRACE:
#ifdef RATE_MATCH_DEBUG
                rate_match_trace_enable(&ep_audio->rm_control, (value != 0));
#endif
                return TRUE;

            case ACCMD_CONFIG_KEY_STREAM_AUDIO_SINK_DELAY:
            {
#ifdef TIMED_PLAYBACK_MODE
                if(SINK == endpoint->direction)
                {
                    endpoint->state.audio.endpoint_delay_us = value;
                    if (endpoint->state.audio.timed_playback != NULL)
                    {
                        timed_playback_set_delay(endpoint->state.audio.timed_playback, value);
                    }
                    return TRUE;
                }
#endif
                return FALSE;
            }
#ifdef AUDIO_SOURCE_GENERATE_METADATA
            case ACCMD_CONFIG_KEY_STREAM_AUDIO_SOURCE_METADATA_ENABLE:
            {
                /* only when the source endpoint is disconnected can we
                 * change the generate_metadata flag.
                 */
                if(SOURCE == endpoint->direction &&
                   NULL == endpoint->connected_to)
                {
                    endpoint->state.audio.generate_metadata = (bool) value;
                    return TRUE;
                }
                return FALSE;
            }
#endif
            case ACCMD_CONFIG_KEY_STREAM_AUDIO_SAMPLE_PERIOD_DEVIATION:
            {
                if (! endpoint->state.audio.running)
                {
#if DAWTH<32
                    endpoint->state.audio.rm_report_sp_deviation = (int)((int32)value >> (32-DAWTH));
#else
                    endpoint->state.audio.rm_report_sp_deviation = value;
#endif
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            default:
                return audio_vsm_configure_sid( stream_external_id_from_endpoint(endpoint),
                                                (stream_config_key)key, value);
        }
    }
}

/**
 * Return a relative deviation of the sample period to the nominal
 * period, i.e.
 *
 *   measured sample period
 *   ---------------------- - 1
 *   nominal sample period
 *
 * as a signed fractional number.
 */
static int get_measured_sp_deviation(ENDPOINT *ep)
{
    int val;
    endpoint_audio_state* audio;

    patch_fn_shared(stream_audio_hydra);

    audio = &ep->state.audio;

    RATE_STIME diff_time = (RATE_STIME)audio->rm_int_time
                           - (RATE_STIME)audio->rm_expected_time;

    /* Right shift and pl_fractional_divide should be done on
     * non-negative values.
     */
    RATE_SHORT_INTERVAL diff_interval;
    if (diff_time >= 0)
    {
        diff_interval = (RATE_SHORT_INTERVAL)(diff_time >> RATE_SAMPLE_PERIOD_TO_TIME_SHIFT);
        val = pl_fractional_divide(diff_interval,
                                   audio->monitor_threshold
                                   * audio->rm_measure.sample_period);
    }
    else
    {
        diff_interval = (RATE_SHORT_INTERVAL)(- diff_time >> RATE_SAMPLE_PERIOD_TO_TIME_SHIFT);
        val = - pl_fractional_divide(diff_interval,
                                     audio->monitor_threshold
                                     * audio->rm_measure.sample_period);
    }

    return val;
}

/**
 * \brief Return a relative deviation of the sample period to the nominal
 * sample period, as a signed fractional.
 *
 * \note See also get_measured_sp_deviation.
 */
static int get_sp_deviation(ENDPOINT *ep)
{
    int val = 0;
    endpoint_audio_state* audio;

    patch_fn_shared(stream_audio_hydra);

    audio = &ep->state.audio;

    /* If the endpoint isn't running or the DSP owns the clock then there is
     * nothing to compensate for so indicate that the rate is perfect. */
    if (!audio->running || is_locally_clocked(ep))
    {
        if (audio->rm_support == RATEMATCHING_SUPPORT_HW)
        {
            val = audio->rm_hw_sp_deviation;
        }
    }
    else
    {
        val = get_measured_sp_deviation(ep);
    }

    if (val != audio->rm_report_sp_deviation)
    {
        audio->rm_report_sp_deviation = val;
#ifdef STREAM_AUDIO_HYDRA_WARP_VERBOSE
        L3_DBG_MSG4("get_sp_deviation(0x%04x,%d) %d * 2^-31 = %d * 10e-6",
                    ep->key, ep->direction, val, frac_mult(val, 1000000));
#endif
    }
    return val;
}

/**
 * Return an approximate quotient (measured sample rate)/(nominal sample rate),
 * in Qm.22
 */
static unsigned get_audio_rate(ENDPOINT* ep)
{
    int sp_deviation = get_sp_deviation(ep);
    int rel_rate;

    /* RM_PERFECT_RATE is 1.0 in Qm.22 */
    rel_rate = RM_PERFECT_RATE - frac_mult(sp_deviation, RM_PERFECT_RATE);

    return (unsigned)rel_rate;
}

/**
 *
 */
static bool is_hw_rate_adjustment_supported(ENDPOINT *endpoint)
{
    if (endpoint->state.audio.rm_enable_hw_rate_adjust)
    {
        uint32 result = 0;

        if ( audio_vsm_get_configuration(
                    stream_external_id_from_endpoint(endpoint),
                    ACCMD_INFO_KEY_AUDIO_HW_RM_AVAILABLE,
                    &result))
        {
            return (result != 0);
        }
    }

    return FALSE;
}

static bool is_locally_clocked(ENDPOINT *endpoint)
{
    SID sid = stream_external_id_from_endpoint(endpoint);
#ifdef INSTALL_MCLK_SUPPORT
    /* The case where an interface is clocked from MPLL but using a
     * non-standard rate, is expected to only occur when using MPLL
     * to rate match under SW control. In this case this interface
     * reports locally clocked, and should report HW rate match capable.
     */
    if (audio_vsm_get_master_mode_from_sid(sid))
    {
        if (! audio_vsm_mclk_is_in_use())
        {
            /* No interface is using external MCLK, so this one can't */
            return TRUE;
        }

        /* If any endpoint in the same sync group has claimed MCLK,
         * that is assumed to be the clock for this endpoint.
         * Only nonsense cases would not fulfil this assumption.
         */
        ENDPOINT* ep_in_sync;
        for (ep_in_sync = endpoint->state.audio.head_of_sync;
             ep_in_sync != NULL;
             ep_in_sync = ep_in_sync->state.audio.nep_in_sync)
        {
            if (ep_in_sync->state.audio.mclk_claimed)
            {
                return FALSE;
            }
        }
        /* Something is using MCLK, but the sync group of which
         * this endpoint is a member is not.
         */
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else
    return audio_vsm_get_master_mode_from_sid(sid);
#endif
}

/*
 * \brief get audio endpoint configuration
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 *
 */
static bool audio_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    endpoint_audio_state* audio = &endpoint->state.audio;

    patch_fn_shared(stream_audio_hydra);
    switch (key)
    {
    case EP_DATA_FORMAT:
        result->u.value = audio_get_data_format(endpoint);
        return TRUE;
    case STREAM_INFO_KEY_AUDIO_SAMPLE_RATE:
        result->u.value = audio_vsm_get_sample_rate_from_sid(stream_external_id_from_endpoint(endpoint));
        return TRUE;
    case EP_BLOCK_SIZE:
        result->u.value = audio->monitor_threshold;
        return TRUE;

    case EP_CBOPS_PARAMETERS:
    {
        CBOPS_PARAMETERS *parameters;
        unsigned cbops_flags = cbops_get_flags(endpoint->cbops);
        parameters = create_cbops_parameters(cbops_flags, EMPTY_FLAG);

        if (!parameters)
        {
            return FALSE;
        }

        /* Set additional parameters if needed.*/
        result->u.value = (uint32)(uintptr_t) parameters;
        return TRUE;
    }
    case EP_RATEMATCH_ABILITY:
    {
        bool success;
        if (is_hw_rate_adjustment_supported(endpoint))
        {
            result->u.value = (uint32)RATEMATCHING_SUPPORT_HW;
            audio->rm_support = RATEMATCHING_SUPPORT_HW;
            success = TRUE;
        }
        else if (audio->rm_enable_sw_rate_adjust)
        {
            success = audio_get_config_rm_ability(endpoint, &result->u.value);
            if (success)
            {
                audio->rm_support = (unsigned)(result->u.value);
            }
        }
        else
        {
            result->u.value = RATEMATCHING_SUPPORT_NONE;
            audio->rm_support = RATEMATCHING_SUPPORT_NONE;
            success = TRUE;
        }
#ifdef STREAM_AUDIO_HYDRA_WARP_VERBOSE
        L3_DBG_MSG4("str_aud_hyd aud_get_cfg ((%d,%d,%d,%d), RM_ABIL)",
                get_hardware_type(endpoint), get_hardware_instance(endpoint),
                get_hardware_channel(endpoint), endpoint->direction);
        L3_DBG_MSG2("... %d, %d", success, result->u.value);
#endif
        return success;
    }
    case EP_RATEMATCH_RATE:
        result->u.value = get_audio_rate(endpoint);
        L3_DBG_MSG2("Audio rate : %d EP: %08X", result->u.value, (uintptr_t)endpoint);
        return TRUE;
    case EP_RATEMATCH_MEASUREMENT:
        result->u.rm_meas.sp_deviation = get_sp_deviation(endpoint);
        if (audio->rm_enable_clrm_measure)
        {
            result->u.rm_meas.measurement.nominal_rate_div25 =
                    audio->rm_measure.sample_rate_div25;
            result->u.rm_meas.measurement.q.num_samples =
                    audio->rm_measure.sample_rate_div25;
            result->u.rm_meas.measurement.q.delta_usec = SECOND / 25;
            result->u.rm_meas.measurement.valid =
                    rate_measure_take_measurement(&audio->rm_measure,
                                                  &result->u.rm_meas.measurement.q,
                                                  &rate_measurement_validity_default,
                                                  hal_get_time());
        }
        else
        {
            result->u.rm_meas.measurement.valid = FALSE;
        }
        return TRUE;
    case STREAM_INFO_KEY_AUDIO_SAMPLE_PERIOD_DEVIATION:
        /* Return the last sp_deviation without triggering a new measurement */
        result->u.value = audio->rm_report_sp_deviation;
        return TRUE;
    case STREAM_INFO_KEY_AUDIO_LOCALLY_CLOCKED:
        result->u.value = is_locally_clocked(endpoint);
        return TRUE;
    default:
        return FALSE;
    }
}

/**
 * \brief Get the timing requirements of this audio endpoint
 *
 * \param endpoint pointer to the endpoint to get the timing info for
 * \param time_info a pointer to an ENDPOINT_TIMING_INFORMATION structure to
 * populate with the endpoint's timing information
 */
static void audio_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    /* an audio endpoint only is concerned with the frequency */
    time_info->period = (unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) /
                audio_vsm_get_sample_rate_from_sid(stream_external_id_from_endpoint(endpoint)));
    time_info->block_size = 1;
    time_info->has_deadline = FALSE;
    time_info->locally_clocked = is_locally_clocked(endpoint);
    /* Effort is only done on this endpoint's interrupt so no kicks are
     * desired from the rest of the chain. */
    time_info->wants_kicks = FALSE;
    return;
}

/*
 * audio_get_data_format
 */
AUDIO_DATA_FORMAT audio_get_data_format (ENDPOINT *endpoint)
{
    patch_fn_shared(stream_audio_hydra);

    /* Audio always supplies FIXP data to connected endpoints */
    return AUDIO_DATA_FORMAT_FIXP;
}

/**
 * \brief set the audio data format that the endpoint will place in/consume from
 * the buffer
 *
 * \param endpoint pointer to the endpoint to set the data format of.
 * \param format AUDIO_DATA_FORMAT requested to be produced/consumed by the endpoint
 *
 * \return whether the set operation was successful
 */
static bool audio_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format)
{
    AUDIO_DATA_FORMAT hw_format;
    tCbuffer **in_buffs, **out_buffs;
    unsigned nr_chans;
    CBOP_VALS vals;
    bool head_setup = TRUE;
    bool status = FALSE;

    patch_fn_shared(stream_audio_hydra);

    /* The data format can only be set before connect. Also audio only supports
     * FIXP for now. Fail if an attempt is made to set format to any thing else */
    if (NULL != endpoint->connected_to || format != AUDIO_DATA_FORMAT_FIXP)
    {
        return status;
    }

    /* Read the hardware format to determine the amount of shift required to
     * operate with FIXP data */
    hw_format = audio_vsm_get_data_format_from_sid(stream_external_id_from_endpoint(endpoint));
    /* For now there is no conversion we can on top of how the hardware was
     * configured. So if the requested set isn't what we already produce fail. */
    switch (hw_format)
    {
    case AUDIO_DATA_FORMAT_24_BIT:
        endpoint->state.audio.shift = DAWTH - 24;
        break;
    case AUDIO_DATA_FORMAT_16_BIT:
        endpoint->state.audio.shift = DAWTH - 16;
        break;
    case AUDIO_DATA_FORMAT_13_BIT:
        endpoint->state.audio.shift = DAWTH - 13;
        break;
    case AUDIO_DATA_FORMAT_8_BIT:
        endpoint->state.audio.shift = DAWTH - 8;
        break;
    default:
        /* We can't achieve any other format so fail */
        return status;
    }

    if(!cbops_mgr_alloc_buffer_info(endpoint->cbops, &nr_chans, &in_buffs, &out_buffs))
    {
        return status;
    }

    if(nr_chans > 0)
    {
        /* Snapshot cbuffer into at this point in the cbops. It may return fail if number of channels
         * is out of kilter, but we just created things properly so it can't whine.
         */
        cbops_mgr_get_buffer_info(endpoint->cbops, nr_chans, in_buffs, out_buffs);

        /* The head's cbuffer info may have changed since it was last seen in cbops down under...
         * Hook in the buffer info before re-creation of chain happens, for bullet proofing against
         * lifecycle phase-related mishaps.
         * Update in the received buffer table, too that we use later.
         */
        in_buffs[endpoint->state.audio.channel] = endpoint->state.audio.source_buf;
        out_buffs[endpoint->state.audio.channel] =  endpoint->state.audio.sink_buf;

        cbops_mgr_connect_channel(endpoint->cbops, endpoint->state.audio.channel,
                endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf);
    }
    else
    {
        /* Even section head hasn't yet been set up - this is therefore a set format before chain gets properly
         * created. Hence it can't belong to an existing sync group, therefore chain is single channel chain
         * for this endpoint in question.
         */
        nr_chans = 1;
        head_setup = FALSE;
        in_buffs = &endpoint->state.audio.source_buf;
        out_buffs = &endpoint->state.audio.sink_buf;
    }


    /* NOTE: In all subsequent changes to the cbops chain, the Hydra case has to tell the cbops_mgr_remove/append
     * functions that they should change the chain regardless of all buffer information being present. This is because
     * in Hydra case, the set data format can occur after the sync groups have been built up, and before connect() setting
     * up individual channels' buffer information. So at this point in time, we may have no or only some buffer pointers,
     * and we need to avoid common functionality in cbops_mgr taking the (in all other cases correct) decision to not touch
     * the chain itself until all buffer info is provided.
     */

    /* Ensure that what gets "recreated" is having the sync group head's parameters!
     * Of course, on some platforms may not have yet sync'ed, in which case it takes its own params.
     * Add to sync list operation will later anyway set things up with sync head driving the show.
     */
    set_endpoint_cbops_param_vals(endpoint->state.audio.head_of_sync , &vals);
    vals.rate_adjustment_amount = &(endpoint->state.audio.rm_adjust_amount);

    status = cbops_mgr_append(endpoint->cbops, CBOPS_DC_REMOVE | CBOPS_RATEADJUST , nr_chans, in_buffs, out_buffs, &vals, TRUE);
    /* Initialise rateadjust to passthrough as haven't been asked to rateadjust yet */
    cbops_mgr_rateadjust_passthrough_mode(endpoint->cbops, TRUE);

    if(head_setup)
    {
        cbops_mgr_free_buffer_info(in_buffs, out_buffs);
    }

    return status;
}

/*
 * \brief This is called in response to the endpoint's monitor interrupt,
 * i.e. every monitor\_threshold * T_{sample}
 */
unsigned get_rm_data(ENDPOINT *endpoint)
{
    RATE_TIME curr_time;
    RATE_TIME delta_time;

    patch_fn(stream_audio_hydra_get_rm_data);

    endpoint_audio_state *audio = &endpoint->state.audio;

    if (endpoint->deferred.kick_is_deferred)
    {
        curr_time = endpoint->deferred.interrupt_handled_time;
    }
    else
    {
        curr_time = hal_get_time();
    }
    curr_time <<= RATE_TIME_EXTRA_RESOLUTION;
    /* Wrapping subtraction; curr_time is always later than rm_period_start_time */
    delta_time = curr_time - audio->rm_period_start_time;
    audio->rm_period_start_time = curr_time;

    /* Integrate over 128 samples */
    audio->rm_int_time -= audio->rm_int_time >> RM_AVG_SHIFT;
    audio->rm_int_time += delta_time >> RM_AVG_SHIFT;

    /* Hardware sample count currently always equals the monitor threshold,
     * i.e. there are no adjustments.
     */
    return audio->monitor_threshold;
}

/**
 * Process data collected by get_rm_data
 */
void process_rm_data(ENDPOINT *endpoint,
                     unsigned num_cbops_read, unsigned num_cbops_written)
{
#if defined(INSTALL_AUDIO_EP_CLRM) || defined(AUDIO_SOURCE_GENERATE_METADATA)
    endpoint_audio_state* ep_audio = &endpoint->state.audio;

    patch_fn_shared(stream_audio_hydra);
    if (endpoint->connected_to != NULL)
    {
        TIME last_sample_time =
                (TIME)(endpoint->state.audio.rm_period_start_time
                >> RATE_TIME_EXTRA_RESOLUTION);

#ifdef INSTALL_AUDIO_EP_CLRM
        if (num_cbops_read == 0 && num_cbops_written == 0)
        {
            /* This can happen for instance when the connected
             * operator does not run between two endpoint kicks.
             * Do not update rate measurement state.
             */
        }
        else if (pl_abs((int)num_cbops_read - (int)num_cbops_written) >
                 ((ep_audio->monitor_threshold >> 5) + 2))
        {
            /* Heuristic for underrun or discard cbops having added/removed
             * samples, which will mess with rate matching, so restart.
             */
            rate_measure_stop(&ep_audio->rm_measure);

            L3_DBG_MSG3("audio_ep 0x%04x cbops_rd %d cbops_wr %d underrun/discard restart RM",
                        stream_external_id_from_endpoint(endpoint),
                        num_cbops_read, num_cbops_written);
        }
        else
        {
            TIME_INTERVAL corr_us;
            RATE_SHORT_INTERVAL correction;
            const bool sra_adjusting = (ep_audio->rm_support == RATEMATCHING_SUPPORT_SW)
                                       && (ep_audio->rate_adjust_op != NULL);
            int sra_phase = 0;

            correction = 0;

            if (sra_adjusting)
            {
                if (endpoint->direction == SINK)
                {
                    /* Keep track of the data in the MMU buffer:
                     * the time of consumption of the last sample that
                     * was just written to the MMU buffer is
                     * that amount x sample period in the future.
                     * This covers cbops writing different amounts than
                     * the monitor threshold for various reasons, including
                     * SRA, and transfers limited by available data.
                     */
                    correction =
                            (int)cbuffer_calc_amount_data_in_words(ep_audio->sink_buf)
                            * (RATE_SHORT_INTERVAL)ep_audio->rm_measure.sample_period;
                }
                else
                {
                    /* Keep track of the data in the MMU buffer:
                     * the time of arrival of the last sample that
                     * was just read from the MMU buffer is
                     * that amount x sample period in the past.
                     * This covers cbops reading different amounts than
                     * the monitor threshold for various reasons, including
                     * SRA, and transfers limited by available space.
                     */
                    correction =
                            - (int)cbuffer_calc_amount_data_in_words(ep_audio->source_buf)
                            * (RATE_SHORT_INTERVAL)ep_audio->rm_measure.sample_period;
                }

                sra_phase = cbops_sra_get_phase(ep_audio->rate_adjust_op);

                /* correction or corr_us is approximately the time from the
                 * last output sample of the SRA to its last input sample.
                 * Note sra_phase is normally negative.
                 */
                RATE_SHORT_INTERVAL phase_correction;

                phase_correction = frac_mult(ep_audio->rm_measure.sample_period,
                                             sra_phase);
                if (endpoint->direction == SINK)
                {
                    correction -= phase_correction;
                }
                else
                {
                    correction += phase_correction;
                }

                /* Signed rounding right shift by RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION */
                corr_us = frac_mult(correction,
                                    (1<<(DAWTH-1-RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION)));

                /* The addition may wrap */
                last_sample_time += corr_us;
            }

#ifdef TIMED_PLAYBACK_MODE
            if (ep_audio->use_timed_playback
                && (ep_audio->timed_playback != NULL))
            {
                /* TODO measure the adjusted rate, restart on TTP start / silence insert / sample drop */
                rate_measure_stop(&ep_audio->rm_measure);
            }
            else
#endif /* TIMED_PLAYBACK_MODE */
            {
                unsigned num_stream_samples;

                if (! sra_adjusting)
                {
                    num_stream_samples = ep_audio->monitor_threshold;
                }
                else
                {
                    num_stream_samples =
                            (SINK == endpoint->direction) ? num_cbops_read : num_cbops_written;

                    /* Reduce responsiveness when the measurement looks unusual;
                     * that involves some heuristics:
                     * - amount processed differs from monitor threshold,
                     *   except for 8k/16k as SCO data is bursty and buffers
                     *   may be minimized for latency
                     */
                    if ( (ep_audio->sample_rate != 8000) &&
                         (ep_audio->sample_rate != 16000) &&
                         ( pl_abs((int)num_stream_samples-(int)ep_audio->monitor_threshold)
                           > (((int)ep_audio->monitor_threshold+15)/16) ) )
                    {
                        L3_DBG_MSG3("audio_ep 0x%04x monitor_thr %d num_str_s %d set unreliable",
                                    stream_external_id_from_endpoint(endpoint),
                                    ep_audio->monitor_threshold, num_stream_samples);
                        rate_measure_set_unreliable(&ep_audio->rm_measure);
                    }
                }

                rate_measure_update(&ep_audio->rm_measure, num_stream_samples,
                                    last_sample_time, 0);
            }
        }
#endif /* INSTALL_AUDIO_EP_CLRM */

#ifdef AUDIO_SOURCE_GENERATE_METADATA
        if ((endpoint->direction == SOURCE)
            && buff_has_metadata(ep_audio->sink_buf))
        {
            audio_generate_metadata(endpoint, num_cbops_written, last_sample_time);
        }
#endif /* AUDIO_SOURCE_GENERATE_METADATA */
    }
#endif /* INSTALL_AUDIO_EP_CLRM or AUDIO_SOURCE_GENERATE_METADATA */
}

/* Set cbops values */
void set_endpoint_cbops_param_vals(ENDPOINT* ep, CBOP_VALS *vals)
{
    patch_fn_shared(stream_audio_hydra);

    vals->data_block_size_ptr = &(ep->state.audio.latency_ctrl_info.data_block);

    /* The silence counter is to become the single sync group counter (when synced) */
    vals->total_inserts_ptr = &(ep->state.audio.latency_ctrl_info.silence_samples);

    /* The rm_diff is to become the single sync group rm_diff (when synced) */
    vals->rm_diff_ptr = &(ep->state.audio.rm_diff);

    vals->rate_adjustment_amount = &(ep->state.audio.rm_adjust_amount);

    /* TODO: using silence insertion only for now */
    vals->insertion_vals_ptr = NULL;

    /* Delta samples is not in use on Hydra - ptr to it must be set to NULL */
    vals->delta_samples_ptr = NULL;

    /* Endpoint block size (that equates to the endpoint kick period's data amount).
     * This "arrives" later on, and is owned, possibly updated, by endpoint only.
     */
    vals->block_size_ptr = (unsigned*)&(ep->state.audio.monitor_threshold);

    vals->rm_headroom = AUDIO_RM_HEADROOM_AMOUNT;

    vals->sync_started_ptr = &ep->state.audio.sync_started;
}

#ifdef AUDIO_SOURCE_GENERATE_METADATA
/**
 * audio_generate_metadata
 * \breif generates metadata for audio source endpoints
 * \param endpoint pointer to audio endpoint structure
 * \param new_words_written amount of new words written into buffer
 */
void audio_generate_metadata(ENDPOINT *endpoint, unsigned new_words_written,
                             TIME last_sample_time)
{
    endpoint_audio_state *audio = &endpoint->state.audio;
    metadata_tag *mtag;
    unsigned b4idx, afteridx;

    /* nothing required if the buffer doesn't have metadata enabled,
     * or no new sample has been written into the buffer */
    if(!buff_has_metadata(audio->sink_buf) || new_words_written == 0)
    {
        return;
    }

    patch_fn_shared(stream_audio_hydra);

    /* A metadata tag that we create should cover a minimum amount
     * of samples (min_tag_len). Normally audio samples are copied
     * in chunks larger than that value, so most of the cases one
     * tag is created per each 'new_words_written'. However, if that's
     * less than min_tag_len, a tag with minimum length is created, which
     * means it will cover some samples that will be received later.
     */

    /* if previous tag was incomplete, we need first to complete the tag */
    if(audio->last_tag_left_words > 0)
    {
        /* last written tag was incomplete, we keep adding
         * Null tag until full length of incomplete tag is
         * covered.
         */
        unsigned null_tag_len = audio->last_tag_left_words;
        if(null_tag_len > new_words_written)
        {
            null_tag_len = new_words_written;
        }

        /* append Null tag, with length = null_tag_len */
        b4idx = 0;
        afteridx = null_tag_len*OCTETS_PER_SAMPLE;
        buff_metadata_append(audio->sink_buf, NULL, b4idx, afteridx);

        /* update amount left */
        audio->last_tag_left_words -= null_tag_len;
        new_words_written -= null_tag_len;
        STREAM_METADATA_DBG_MSG2("AUDIO_SOURCE_GENERATE_METADATA, NULL TAG ADDED to complete old written tag, time=0x%08x, tag_len=%d",
                                 hal_get_time(), afteridx);
        if(new_words_written == 0)
        {
            /* all new words used for completing old tag */
            return;
        }
    }

    /* create a new tag to append */
    b4idx = 0;
    afteridx = new_words_written*OCTETS_PER_SAMPLE;
    mtag = buff_metadata_new_tag();
    if (mtag != NULL)
    {
        /* Calculating time of arrival, here it is the time that first sample of this tag
         * arrives in the source buffer. We use current time and go back by the duration
         * of total samples ahead of first sample of the tag.
         * Notes:
         *    - Since we assume the last sample in source buffer just arrived, there could
         *      up to 1-sample duration bias.
         *
         *    - if we use HW/SW rate matching, for better accuracy we might want to apply an adjustment
         *      to 'amount_in_buffer' and/or 'new_words_written' below.
         *
         *    - There will be more inaccuracy if cbops inserts silence or trashes input.
         *
         *    None of the above concerns causes accumulation error, and in practice the jitter is negligible.
         */
        unsigned amount_in_buffer = cbuffer_calc_amount_data_in_words(audio->source_buf);
        INTERVAL time_passed = (INTERVAL) (((uint48)(amount_in_buffer+new_words_written)*SECOND)/audio->sample_rate);
        TIME time_of_arrival = time_sub(last_sample_time, time_passed);

        /* Pass on the integrated relative sample period deviation */
        mtag->sp_adjust = get_sp_deviation(endpoint);

        /* see if we have minimum amount for tag */
        if(new_words_written >= audio->min_tag_len)
        {
            /* we have enough new samples to append a complete tag */
            mtag->length = new_words_written*OCTETS_PER_SAMPLE;
        }
        else
        {
            /* new received samples aren't enough to form a
             * new complete tag, we append a new tag with
             * minimum length, this tag is incomplete and
             * will be completed in next calls when we receive
             * new samples by appending Null tags.
             */
            mtag->length = audio->min_tag_len*OCTETS_PER_SAMPLE;
            audio->last_tag_left_words = audio->min_tag_len - new_words_written;
        }

        /* set the time of arrival */
        METADATA_TIME_OF_ARRIVAL_SET(mtag, time_of_arrival);
        STREAM_METADATA_DBG_MSG5("AUDIO_SOURCE_GENERATE_METADATA, NEW TAG ADDED,"
                                 "adjust=0x%08x, toa=0x%08x(%dus in the past), new_words=%d, tag_length=%d",
                                 (unsigned)mtag->sp_adjust,
                                 mtag->timestamp,
                                 time_sub(hal_get_time(), mtag->timestamp),
                                 new_words_written,
                                 mtag->length);
    }
    else
    {
        STREAM_METADATA_DBG_MSG1("AUDIO_SOURCE_GENERATE_METADATA, NULL TAG ADDED, time=0x%08x", hal_get_time());
    }
    /* append generated metadata to the output buffer */
    buff_metadata_append(audio->sink_buf, mtag, b4idx, afteridx);

}
#endif /* AUDIO_SOURCE_GENERATE_METADATA */
