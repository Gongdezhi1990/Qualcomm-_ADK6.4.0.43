/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_audio_common_a7da_kas.c
 * \ingroup stream
 *
 * stream audio type file. <br>
 * This file contains stream functions for audio&file endpoints because they are
 * very similar in a7da_kas <br>
 *
 * \section sec1 Contains:
 * audio_common_close <br>
 * audio_common_stop <br>
 * audio_common_disconnect <br>
 * audio_common_buffer_details <br>
 * audio_common_start <br>
 * audio_common_get_timing <br>
 * audio_common_connect <br>
 * audio_get_data_format <br>
 * audio_common_set_data_format <br>
 * a7da_audio_common_get_buffer_size <br>
 * a7da_get_endpoint_from_parameters <br>
 * add_channel_copy_section <br>
 * add_interleaving_section <br>
 */

/****************************************************************************
Include Files
*/
#include "stream_private.h"
#include "stream_endpoint_audio.h" /* For protected access audio ep base class */
#include "kascmd_prim.h"           /* need the types for configuration keys */
#include "kas_dma_endpoint.h"
#include "kas_audio_endpoint.h"
#include "stream_for_kas_audio.h"
#include "stream_common_a7da_kas.h"

/****************************************************************************
Private Type Declarations
*/

/*  List of created port structures */
a7da_audio_port   *a7da_port_list = NULL;

/****************************************************************************
Private Constant Declarations
*/


/****************************************************************************
Private Macro Declarations
*/
/** The default size of an audio buffer */
#define KAS_MIN_AUDIO_BUFFER_SIZE     128
#define KAS_MAX_AUDIO_BUFFER_SIZE     256
/**
 * Check if two file endpoints are already synchronised.
 */
#define FILE_EP_ALREADY_SYNCHRONISED(ep1,ep2) ( (ep1)->state.file.head_of_sync == (ep2)->state.file.head_of_sync )
/**
 * Check if the synchronisation group which includes the given file endpoint
 * is already running.
 */
#define FILE_SYNC_GROUP_IS_RUNNING(ep) (((ep)->state.file.head_of_sync)->state.file.running)

/**
 * Find if a file endpoint is synchronised
 */
#define IS_FILE_ENDPOINT_SYNCED(ep)  ( (ep) != (ep)->state.file.head_of_sync || NULL != (ep)->state.file.nep_in_sync )

/**
 * Returns TRUE if the audio endpoint is overriden
 */
#define IS_OVERRIDDEN(ep) (((ep)->stream_endpoint_type==endpoint_audio) ? ((ep)->state.audio.is_overridden) : FALSE)

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

bool has_synced_connection_to_same_entity(ENDPOINT *ep);
void a7da_audio_common_destroy_port(a7da_audio_port *port);
/****************************************************************************
Private Function Definitions
*/

/****************************************************************************
Public Function Definitions
*/

/* add_to_synchronisation */
bool add_to_synchronisation(ENDPOINT *ep1, ENDPOINT *ep2)
{
    if(ep1->stream_endpoint_type == endpoint_audio)
    {
        /* If the two endpoint is the same or they are already synced return TRUE. */
        if ((ep1 == ep2) || (ALREADY_SYNCHRONISED(ep1,ep2)))
        {
            return TRUE;
        }

        ENDPOINT **p_ep;
        /* iterate until the end - it's just a while. */
        for(p_ep = &ep1->state.audio.head_of_sync; *p_ep != NULL; p_ep = &((*p_ep)->state.audio.nep_in_sync));
        /* Add the new element to the end of the list. */
        *p_ep = ep2;
        /* Set the head of the synch for ep2. */
        ep2->state.audio.head_of_sync = ep1->state.audio.head_of_sync;
        return TRUE;
    }
    else
    {
        /* If the two endpoint is the same or they are already synced return TRUE. */
        if ((ep1 == ep2) || (FILE_EP_ALREADY_SYNCHRONISED(ep1,ep2)))
        {
            return TRUE;
        }
       
        ENDPOINT **p_ep;
        /* iterate until the end - it's just a while. */
        for(p_ep = &ep1->state.file.head_of_sync; *p_ep != NULL; p_ep = &((*p_ep)->state.file.nep_in_sync));
        /* Add the new element to the end of the list. */
        *p_ep = ep2;
        /* Set the head of the synch for ep2. */
        ep2->state.file.head_of_sync = ep1->state.file.head_of_sync;
        return TRUE;

    }
}

/* remove_from_synchronisation */
bool remove_from_synchronisation(ENDPOINT *ep)
{
    if(ep->stream_endpoint_type == endpoint_audio)
    {
        if ( IS_AUDIO_ENDPOINT_SYNCED(ep) )
        {
            ENDPOINT **p_ep;
            ENDPOINT *new_head = ep->state.audio.head_of_sync;
            
            if( IS_FILE_ENDPOINT_HEAD_OF_SYNC(ep) )
            {
                /* Removing the head of the list. */
                new_head = ep->state.audio.nep_in_sync;
            }

            //remove from list; and give new head if necessary
            for(p_ep = &new_head; *p_ep != NULL; p_ep = &((*p_ep)->state.audio.nep_in_sync))
            {
                /* update the new head */
                (*p_ep)->state.audio.head_of_sync = new_head;
                /* Remove ep1 only if it is not the head of list */
                if((*p_ep)->state.audio.nep_in_sync == ep)
                {
                    (*p_ep)->state.audio.nep_in_sync = ep->state.audio.nep_in_sync;
                }
            }

            /* Now ep is "standalone" endpoint */
            ep->state.audio.head_of_sync = ep;
            ep->state.audio.nep_in_sync = NULL;
        }
    }
    else
    {
        if (IS_FILE_ENDPOINT_SYNCED(ep))
        {
            ENDPOINT **p_ep;
            ENDPOINT *new_head = ep->state.file.head_of_sync;
            
            if( IS_FILE_ENDPOINT_HEAD_OF_SYNC(ep) )
            {
                /* Removing the head of the list. */
                new_head = ep->state.file.nep_in_sync;
            }

            //remove from list; and give new head if necessary
            for(p_ep = &new_head; *p_ep != NULL; p_ep = &((*p_ep)->state.file.nep_in_sync))
            {
                /* update the new head */
                (*p_ep)->state.file.head_of_sync = new_head;
                /* Remove ep1 only if it is not the head of list */
                if((*p_ep)->state.file.nep_in_sync == ep)
                {
                    (*p_ep)->state.file.nep_in_sync = ep->state.file.nep_in_sync;
                }
            }

            /* Now ep is "standalone" endpoint */
            ep->state.file.head_of_sync = ep;
            ep->state.file.nep_in_sync = NULL;
        }
    }

    return TRUE;
}

/**
 * Retrieve the buffer size from the port.
 * */
unsigned a7da_audio_common_get_buffer_size_from_port(a7da_audio_port *port)
{
    if (port->is_hardware_type)
    {
        if (port->word_format==TRANSLATE_WORD_FORMAT_DUAL_16BIT)
        {
            return port->cb_desc_local.size*2;
        }
        else
        {
            return port->cb_desc_local.size;
        }
    }
    return ( stream_if_get_system_sampling_rate() < STREAM_AUDIO_SAMPLE_RATE_32K)?
        KAS_MIN_AUDIO_BUFFER_SIZE:KAS_MAX_AUDIO_BUFFER_SIZE;
}

/**
 * Retrieve the buffer size from the endpoint.
 * */
unsigned a7da_audio_common_get_buffer_size(ENDPOINT *endpoint)
{
    a7da_audio_port *port = (endpoint->stream_endpoint_type == endpoint_file) ?
                                endpoint->state.file.port:
                                endpoint->state.audio.port;
    return a7da_audio_common_get_buffer_size_from_port(port);
}

#ifdef A7DA_IO_DEBUG
#define SP_NUM_DEBUG_PORTS 15
a7da_audio_port *spDbg_audio_ports[SP_NUM_DEBUG_PORTS] = {NULL,NULL,NULL,NULL,NULL,
                                                 NULL,NULL,NULL,NULL,NULL,
                                                 NULL,NULL,NULL,NULL,NULL};
#endif
/**
 * Destroys the KAS port structure
 * \param port - the "port" structure to be destroyed
 * \param is_overriden - indicates if the ep is overriden and if buffers need to be destroyed.
*/
void a7da_audio_common_destroy_port(a7da_audio_port *port)
{
   a7da_audio_port **port_pp;

    /* remove port from list */
    interrupt_block();
    port_pp = &a7da_port_list;
    while( *port_pp != NULL)
    {
       if( *port_pp == port)
       {
            *port_pp = port->next;
            break;
       }
       port_pp = &((*port_pp)->next);
    }
    interrupt_unblock();

 #ifdef A7DA_IO_DEBUG
    unsigned j;
    for(j=0;j<SP_NUM_DEBUG_PORTS;j++)
    {
         if(spDbg_audio_ports[j]==port)
         {
            spDbg_audio_ports[j]=NULL;
            break;
         }
    }
#endif
    kas_endpoint_destroy_dma_request(port);
    if(port->ilv_buf)
    {
        cbuffer_destroy(port->ilv_buf);
    }
    pfree(port->channel_cbuffers);
    pfree(port);
}

/**
 * Populate the KAS port structure
 * \param instance - the audio device instance identifier
 * \param num_cg - the number of channels
 * \param cb_addr - address of the remote circular buffer descriptor
 * \param hardware - stream type (File, I2S, IACC, ...)
 *
 *  TODO: at some point decouple port and streams if possible (e.g. by moving all
 *        kas* specific functions and structures into kas_audio, similarly as hydra)
 */

a7da_audio_port * a7da_audio_common_get_port(ENDPOINT *endpoint, ENDPOINT_DIRECTION dir,
        unsigned hardware, unsigned instance, unsigned num_ch, uint32 cb_addr)
{
    unsigned buffer_size;
    /* Allocate the port descriptor */
    a7da_audio_port * port = xzpnew(a7da_audio_port);

    if (port == NULL)
    {
        return NULL;
    }
#ifdef A7DA_IO_DEBUG
    unsigned spidx;
    for(spidx=0;spidx<SP_NUM_DEBUG_PORTS;spidx++)
    {
         if(spDbg_audio_ports[spidx]==NULL)
         {
            spDbg_audio_ports[spidx]=port;
            break;
         }
    }
    port->sp_direction=dir;
    port->sp_type=hardware;
    port->sp_dev=instance;
#endif

    /* Allocate zeroed space for the channel buffer pointers */
    port->channel_cbuffers = (tCbuffer **) zpmalloc(sizeof(tCbuffer *)*num_ch);
    if (port->channel_cbuffers == NULL)
    {
        a7da_audio_common_destroy_port(port);
        return NULL;
    }

    /* Set the hardware and interleaving flags  */
    port->is_hardware_type = (hardware!=STREAM_DEVICE_FILE) && (hardware!=STREAM_DEVICE_APPDATA);
    if (port->is_hardware_type && (dir==SINK) && ((instance==IACC) || (instance==AC97)))
    {
        port->interleaved = FALSE;
    }
    else
    {
        port->interleaved = (num_ch>1);
    }

    /* Fill default values (these fields may need to be reconfigured afterwards) */
    port->num_channels = num_ch;
    port->cb_desc_addr = cb_addr;
    port->word_format = 0;
    port->locally_clocked = FALSE;
    port->sample_rate = 0;
    port->ilv_buf = (tCbuffer *)NULL;
    port->kick_object = NULL;
    port->flow_control_enabled = FALSE;

    /* Update the cBuffer descriptor data */
    if (port->interleaved)
    {
        remote_cbuf_read(&port->cb_desc_local, cb_addr, 1);
        buffer_size = a7da_audio_common_get_buffer_size_from_port(port);

        /* Create (de)interleaving buffer */
        port->ilv_buf = cbuffer_create_with_malloc(1+buffer_size*2, BUF_DESC_SW_BUFFER);
        if (NULL == port->ilv_buf)
        {
            a7da_audio_common_destroy_port(port);
            return NULL;
        }

        /* Now divide the size for N channels */
        buffer_size /= num_ch;
    }
    else
    {
        remote_cbuf_read(&port->cb_desc_local, cb_addr, num_ch);
        buffer_size = a7da_audio_common_get_buffer_size_from_port(port);
    }

    /* set default period size to half the DRAM buffer size */
    if (!port->is_hardware_type)
    {
        remote_cbuf_set_period_size(&port->cb_desc_local, 
                                remote_cbuf_get_size(&port->cb_desc_local)/2);
        port->channel_block_size = buffer_size;
    }
    else
    {
        port->channel_block_size = remote_cbuf_get_size(&port->cb_desc_local);
        if (port->interleaved)
        {
            port->channel_block_size /= num_ch;
        }
    }

    /* Now that the remote buffer information is known, setup and 
       populate the DMA request entry */
    if(!kas_endpoint_setup_dma_request(port))
    {
        a7da_audio_common_destroy_port(port);
        return NULL;
    }
	
#ifdef A7DA_IO_DEBUG
    port->sp_channels = port->num_channels;
    port->sp_interleaved = port->interleaved;
#endif

    /* Insert port into list */
    interrupt_block();
    port->next     = a7da_port_list;
    a7da_port_list = port;
    interrupt_unblock();

    return port;
}

/**
 * a7da_get_port_list - Return head of port list
 */
a7da_audio_port* a7da_get_port_list(void)
{
    return a7da_port_list;
}

/**
 * Retrieves an existing endpoint from the passed endpoint parameters
 * or returns NULL if not found */
ENDPOINT *a7da_get_endpoint_from_parameters(uint24 key, uint32 cb_addr,
                                            ENDPOINT_DIRECTION dir,
                                            const ENDPOINT_FUNCTIONS *functions)
{
    ENDPOINT *ep;
    a7da_audio_port *port;

    ep = stream_first_endpoint(dir);

    port = (ep->stream_endpoint_type == endpoint_file) ?
                    ep->state.file.port: ep->state.audio.port;

    while ((ep != NULL) && ((ep->functions != functions) || (ep->key != key)
            || port->cb_desc_addr!=cb_addr))
    {
        ep = ep->next;
        port = (ep->stream_endpoint_type == endpoint_file) ?
                            ep->state.file.port: ep->state.audio.port;
    }

    return ep;
}


/** Gets the endpoint to kick internally from the device side */
ENDPOINT* get_endpoint_to_kick(a7da_audio_port* port)
{
    return (ENDPOINT*) port->kick_object;
}

/** Sets the endpoint to kick internally from the device side */
void set_endpoint_to_kick(a7da_audio_port* port, ENDPOINT* ep)
{
    port->kick_object = (void*) ep;
}

/** Kicks the master endpoint associated to the device */
void kick_master_endpoint(a7da_audio_port* port)
{
    ENDPOINT *ep = get_endpoint_to_kick(port);
    if (!IS_OVERRIDDEN(ep))
    {
        ep->functions->kick(ep, STREAM_KICK_INTERNAL);
    }
}

/* ********************************** API functions ************************************* */

/**
 * \brief closes the audio endpoint by requesting the release method on the
 *        audio hardware
 *
 * \param *endpoint pointer to the endpoint that is being closed.
 *
 * \return success or failure
 */
bool audio_common_close(ENDPOINT *endpoint)
{
    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is call the close hw method on audio.
     */
    a7da_audio_port *port;

    /* If endpoint is synced, remove it from sync list. This is the only thing that
     * can fail. So do that first. */
    if(!remove_from_synchronisation(endpoint))
    {
        return FALSE;
    }

    if(endpoint->stream_endpoint_type == endpoint_file)
    {
        port = endpoint->state.file.port;
    }
    else
    {
        port = endpoint->state.audio.port;
    }

    /* Finally free the a7da audio port descriptor (if endpoint is mapped to "channel 0") */
    if(GET_CHANNEL_FROM_AUDIO_EP_KEY(endpoint->key)==0)
    {
        a7da_audio_common_destroy_port(port);
        if(endpoint->cbops != NULL)
        {
            /* Once the endpoint is disconnected we can also destroy the cbops_mgr. */
            cbops_mgr_destroy(endpoint->cbops);
        }
    }
    endpoint->cbops = NULL;
    return TRUE;
}

/**
 * \brief Checks if an endpoint in a synced group has an existing connection 
 *        to the same entity
 * \param *endpoint pointer to the endpoint that is being checked
 * \return TRUE if any endpoint in the group has a connection to the same operator
 */
bool has_synced_connection_to_same_entity(ENDPOINT *endpoint)
{
    ENDPOINT *p_ep;
    if (endpoint->stream_endpoint_type == endpoint_file)
    {
        for (p_ep = endpoint->state.file.head_of_sync; p_ep != NULL; p_ep = p_ep->state.file.nep_in_sync)
        {
            if ((p_ep != endpoint) && stream_is_connected_to_same_entity(p_ep, endpoint))
            {   
                return TRUE;
            }
        }
    }
    else
    {
        for (p_ep = endpoint->state.audio.head_of_sync; p_ep != NULL; p_ep = p_ep->state.audio.nep_in_sync)
        {
            if ((p_ep != endpoint) && stream_is_connected_to_same_entity(p_ep, endpoint))
            {   
                return TRUE;
            }
        }
    }
    return FALSE;
}

/**
 * \brief Create the endpoint buffer and connect to the supplied buffer.
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
bool audio_common_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    a7da_audio_port *port;
    tCbuffer **source_buf;
    tCbuffer **sink_buf;
    bool is_overridden;
    unsigned channel;
    ENDPOINT *head_of_sync;
    bool is_in_sync_group;

    if(endpoint->stream_endpoint_type == endpoint_file)
    {
        port = endpoint->state.file.port;
        source_buf = &endpoint->state.file.source_buf;
        sink_buf = &endpoint->state.file.sink_buf;
        is_overridden = FALSE;
        head_of_sync = endpoint->state.file.head_of_sync;
        is_in_sync_group = IS_FILE_ENDPOINT_SYNCED(endpoint);
        channel = GET_CHANNEL_FROM_AUDIO_EP_KEY(endpoint->key);
    }
    else
    {
        port = endpoint->state.audio.port;
        source_buf = &endpoint->state.audio.source_buf;
        sink_buf = &endpoint->state.audio.sink_buf;
        is_overridden = endpoint->state.audio.is_overridden;
        head_of_sync = endpoint->state.audio.head_of_sync;
        is_in_sync_group = IS_AUDIO_ENDPOINT_SYNCED(endpoint);
        channel = GET_CHANNEL_FROM_AUDIO_EP_KEY(endpoint->key);
    }

    /* If endpoints are not synchronised yet, create the sync group at first connect.
       Otherwise, use the existing sync group. */
    ENDPOINT* device_ep_to_kick = get_endpoint_to_kick(port);
    if (device_ep_to_kick == NULL)
    {
        /* Check if the endpoint has been already synchronised */
        if (is_in_sync_group) 
        {
            set_endpoint_to_kick(port, head_of_sync);
        }
        else 
        {
            set_endpoint_to_kick(port, endpoint);
        }
    }
    else
    {
        /* Make sure it gets synchronised to the master endpoint */
        /* TODO: optimise CBOPS channel connect: it doesn't make sense to recreate 
                 the whole cbops mgr object for each channel added to the sync list */
        if (!add_to_synchronisation(device_ep_to_kick, endpoint))
        {
            /* Fail if endpoints cannot be synced */
            return FALSE;
        }
        is_in_sync_group = TRUE;
    }

    /* If this endpoint is synchronised and connected to the same operator then
     * we only need one kick to that operator. */
    if (is_in_sync_group && (ep_to_kick != NULL) && 
            has_synced_connection_to_same_entity(endpoint))
    {
        endpoint->ep_to_kick = NULL;
    }
    else 
    {
        endpoint->ep_to_kick = ep_to_kick;
    }

    /* Connect buffers */
    if (is_overridden)
    {
        /* Connect source and sink software buffers. */
        if(SOURCE == endpoint->direction)
        {
            *source_buf = Cbuffer_ptr;
            port->channel_cbuffers[channel] = Cbuffer_ptr;
        }
        else
        {
            *sink_buf = Cbuffer_ptr;
            port->channel_cbuffers[channel] = Cbuffer_ptr;
        }
    }
    else
    {
        /* Create & connect source and sink software buffers. */
        port->channel_cbuffers[channel] = cbuffer_create_with_malloc(1+port->channel_block_size*2, BUF_DESC_SW_BUFFER);
        if (NULL == port->channel_cbuffers[channel])
        {
            /* Fail since it cannot create I/O channel buffer */
            return FALSE;
        }

        /* Assign source and sink software buffers. */
        if(SOURCE == endpoint->direction)
        {
            *source_buf = port->channel_cbuffers[channel];
            *sink_buf = Cbuffer_ptr;
        }
        else
        {
            *source_buf = Cbuffer_ptr;
            *sink_buf = port->channel_cbuffers[channel];
        }
        

        /* This just connects channel information up, cbops chain was already created! */
        if (!cbops_mgr_connect_channel(endpoint->cbops, channel,
                                       *source_buf, *sink_buf))
        {
            cbuffer_destroy(port->channel_cbuffers[channel]);
            *source_buf = NULL;
            *sink_buf = NULL;
            port->channel_cbuffers[channel] = NULL;
            return FALSE;
        }

        /* Initialise any rateadjust to passthrough as haven't been asked to rateadjust yet */
        cbops_mgr_rateadjust_passthrough_mode(endpoint->cbops, TRUE);
    }
    *start_on_connect = FALSE;
    return TRUE;
}

/**
 * \brief Disconnects from an endpoint and stops the data from flowing
 *
 * \param *endpoint pointer to the endpoint that is being disconnected
 *
 * \return success or failure
 */
bool audio_common_disconnect(ENDPOINT *endpoint)
{
    a7da_audio_port *port;
    tCbuffer **source_buf;
    tCbuffer **sink_buf;
    bool *running;
    unsigned channel;
    bool is_overridden;

    if(endpoint->stream_endpoint_type == endpoint_file)
    {
        port = endpoint->state.file.port;
        source_buf = &endpoint->state.file.source_buf;
        sink_buf = &endpoint->state.file.sink_buf;
        running = &endpoint->state.file.running;
        channel = GET_CHANNEL_FROM_AUDIO_EP_KEY(endpoint->key);
        is_overridden = FALSE;
    }
    else
    {
        port = endpoint->state.audio.port;
        source_buf = &endpoint->state.audio.source_buf;
        sink_buf = &endpoint->state.audio.sink_buf;
        running = &endpoint->state.audio.running;
        channel = GET_CHANNEL_FROM_AUDIO_EP_KEY(endpoint->key);
        is_overridden = endpoint->state.audio.is_overridden;
    }

    /* Forget all about the connected buffer, it's gone away or is about to. If
     * the endpoint hasn't been stopped then this function shouldn't have been
     * called. */
    if(*running)
    {
        panic_diatribe(PANIC_AUDIO_UNEXPECTED_ENDPOINT_DISCONNECT,
                stream_external_id_from_endpoint(endpoint));
    }

    if (!is_overridden)
    {
        /* Mark the channel as unused in cbops. */
        cbops_mgr_set_unused_channel(endpoint->cbops, channel);

        /* Destroy the channel buffer */
        if (port->channel_cbuffers[channel])
        {
            cbuffer_destroy(port->channel_cbuffers[channel]);
        }
    }

    /* Unsync the endpoints */
    if (!remove_from_synchronisation(endpoint))
    {
        /* Fail if endpoints cannot be unsynced */
        return FALSE;
    }

    /* The buffers should be already destroyed at this stage, 
       so invalidate their pointers */
    *source_buf = NULL;
    *sink_buf = NULL;
    port->channel_cbuffers[channel] = NULL;

    /* If the current head endpoint is disconnected, then clear its kick-reference */
    if (get_endpoint_to_kick(port)==endpoint)
    {
        set_endpoint_to_kick(port, NULL);
    }

    /* Reset ep_to_kick to it's default state of no endpoint to kick*/
    endpoint->ep_to_kick = NULL;
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
bool audio_common_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }
    details->b.buff_params.size = a7da_audio_common_get_buffer_size(endpoint);
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;
    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    details->can_override = TRUE;
    details->wants_override = FALSE;
    return TRUE;
}

/**
 * \brief Get the timing requirements of this audio endpoint
 *
 * \param endpoint pointer to the endpoint to get the timing info for
 * \param time_info a pointer to an ENDPOINT_TIMING_INFORMATION structure to
 * populate with the endpoint's timing information
 */
void audio_common_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
#ifdef DESKTOP_TEST_BUILD
    /* For unit tests, just return the value set by the test */
    extern unsigned audio_sink_rate, audio_source_rate;
    if (SOURCE == endpoint->direction)
    {
        time_info->period = (unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) / audio_source_rate);
    }
    else
    {
        time_info->period = (unsigned int)((unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) / audio_sink_rate);
    }
#else
    a7da_audio_port *port;
    unsigned int block_size;

    if(endpoint->stream_endpoint_type == endpoint_file)
    {
        port = endpoint->state.file.port;
/*
        Get block size from connected operator.  Use to calculate period
        if sample rate is known.  Otherwise, period defaults to 2 msec.
*/
        block_size = 1;
        time_info->period = 0;
        /*  File endpoints wants kicks*/
        time_info->wants_kicks = TRUE;
    }
    else
    {
        port = endpoint->state.audio.port;
        block_size = endpoint->state.audio.block_size;
        time_info->wants_kicks = FALSE;

        if (port->sample_rate!=0)
        {
          /* Return the kick period. */
              time_info->period = (unsigned int)(
                      (unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL)
                      * block_size / port->sample_rate);
        }
        else
        {
              time_info->period = 0;
        }
    }


    time_info->block_size = block_size;
    time_info->locally_clocked = port->locally_clocked;
#endif
    time_info->has_deadline = FALSE;
}


/**
 * \brief get the audio data format of the data sent/expected on the other side
 * of the port
 *
 * \param endpoint pointer to the endpoint to get the data format of.
 *
 * \return the data format
 */
AUDIO_DATA_FORMAT audio_get_data_format (ENDPOINT *endpoint)
{
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
bool audio_common_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format)
{
    /* The data format can only be set before connect */
    if (NULL != endpoint->connected_to)
    {
        return FALSE;
    }

    switch (format)
    {
    // TODO: Add support for various formats here
    case AUDIO_DATA_FORMAT_FIXP:
        return TRUE;
    case AUDIO_DATA_FORMAT_16_BIT:
        return TRUE;
    default:
        return FALSE;
    }

}

/* Set cbops values */
void set_endpoint_cbops_param_vals(ENDPOINT* ep, CBOP_VALS *vals)
{
    if(ep->stream_endpoint_type == endpoint_file)
    {
        /* TODO: ratemaching and info latency_ctrl for file is missing*/

        /* Connect the rate adjustment to the calculated value for file endpoint */
        vals->rate_adjustment_amount = &(ep->state.file.rm_adjust_amount);

        vals->rm_diff_ptr = &(ep->state.file.rm_diff);
        vals->delta_samples_ptr = NULL;
        return;
    }

    vals->data_block_size_ptr = &(ep->state.audio.latency_ctrl_info.data_block);

    /* The silence counter is to become the single sync group counter (when synced) */
    vals->total_inserts_ptr = &(ep->state.audio.latency_ctrl_info.silence_samples);

    /* The rm_diff is to become the single sync group rm_diff (when synced) */
    vals->rm_diff_ptr = &(ep->state.audio.rm_diff);

    /* Connect the rate adjustment to the calculated value */
    vals->rate_adjustment_amount = &(ep->state.audio.rm_adjust_amount);


    /* TODO: using silence insertion only for now */
    vals->insertion_vals_ptr = NULL;

    /* TODO: make sure a7da_kas is not using delta_samples like BC */
    vals->delta_samples_ptr = NULL;

    /* Endpoint block size (that equates to the endpoint kick period's data amount).
     * This "arrives" later on, and is owned, possibly updated, by endpoint only.
     */
    vals->block_size_ptr = (unsigned*)&(ep->state.audio.block_size);

    vals->rm_headroom = AUDIO_RM_HEADROOM_AMOUNT;

    vals->sync_started_ptr = &ep->state.audio.sync_started;
}

/** Populate the cbops structure */
bool a7da_audio_common_create_cbops(ENDPOINT *ep,unsigned num_ch)
{
    cbops_mgr* cbops = cbops_mgr_create(ep->direction, 
                        CBOPS_COPY_ONLY | CBOPS_RATEADJUST);
    if(cbops == NULL)
    {
        return FALSE;
    }

    tCbuffer *in_buffs[NR_MAX_SYNCED_EPS], *out_buffs[NR_MAX_SYNCED_EPS];
    
    /* All buffers should be initially marked as absent */
    unsigned chan;
    for(chan = 0; chan < num_ch; chan++)
    {
        in_buffs[chan] = NULL;
        out_buffs[chan] = NULL;
    }

    CBOP_VALS vals;
    // Populate the cbops values structure
    set_endpoint_cbops_param_vals(ep, &vals);
    vals.rate_adjustment_amount = NULL;
    vals.shift_amount = 0;

    // Connect the dummy buffers and set the cbops parameters
    if(!cbops_mgr_connect(cbops, num_ch, in_buffs, out_buffs, &vals))
    {
        cbops_mgr_destroy(cbops);
        return FALSE;
    }
    ep->cbops = cbops;
    return TRUE;
}

