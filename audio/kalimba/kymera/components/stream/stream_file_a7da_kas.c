/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \addtogroup File File endpoint
 * \ingroup endpoints
 * \file  stream_file_bluecore.c
 *
 * stream file type file. <br>
 * This file contains stream functions for bluecore file endpoints. <br>
 *
 */

/****************************************************************************
Include Files
*/
#include "stream_private.h"
#include "stream_endpoint_audio.h" /* For protected access audio ep base class */
#include "kascmd_prim.h"           /* need the types for configuration keys */
#include "kas_audio_endpoint.h"
#include "stream_common_a7da_kas.h"
#include "kas_dma_endpoint.h"
#include "opmgr/opmgr_endpoint_override.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/


/****************************************************************************
Private Macro Declarations
*/
/** Macro pairs to enable use of kalimba memory mapped register definitions without
 *  generating warnings */
#define POSTFIX(a, p) a ## p
#define UINT(aDDR) POSTFIX(aDDR,u)
#define ULNG(aDDR) POSTFIX(aDDR,ul)


/****************************************************************************
Private Variable Definitions
*/
static bool file_start (ENDPOINT *ep, KICK_OBJECT *ko);
static bool file_stop (ENDPOINT *ep);
static void file_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);
static bool file_configure(ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool file_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static void kas_file_rm_init(ENDPOINT *ep);

DEFINE_ENDPOINT_FUNCTIONS (file_functions, audio_common_close, audio_common_connect,
                            audio_common_disconnect, audio_common_buffer_details,
                            file_kick, stream_sched_kick_dummy,
                            file_start, file_stop,
                            file_configure, file_get_config,
                            audio_common_get_timing, sync_endpoints);

/****************************************************************************
Private Function Declarations
*/
/* ******************************* Helper functions ************************************ */
static inline unsigned get_file_max_data(endpoint_file_state *file)
{
    return file->source_buf->size;
}

/****************************************************************************
Public Function Definitions
*/
/****************************************************************************
 *
 * stream_file_get_endpoint
 *
 */
ENDPOINT *stream_file_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned num_params,
                                    unsigned *params)
{
    if (num_params != A7DA_AUDIO_PARAMS_NUM)
    {
        return NULL;
    }

    int instance  = params[A7DA_AUDIO_PARAMS_INSTANCE];
    int channel   = params[A7DA_AUDIO_PARAMS_CHANNEL];
    int master_id = params[A7DA_AUDIO_PARAMS_MASTER_ID];
    int num_ch    = params[A7DA_AUDIO_PARAMS_NUM_CHANNELS];
    uint32 cb_addr = ((uint32)params[A7DA_AUDIO_PARAMS_CB_ADDR_MSW]<<16) |
                        params[A7DA_AUDIO_PARAMS_CB_ADDR_LSW];

    /* First go and find a stream on the same audio hardware, if not found
     * then create a new stream. */
    uint24 key = create_stream_key(STREAM_DEVICE_FILE, instance, channel);
    ENDPOINT *endpoint = a7da_get_endpoint_from_parameters(key, cb_addr, dir,
                                                  &endpoint_file_functions);
    if(!endpoint)
    {
        if ((endpoint = STREAM_NEW_ENDPOINT(file, key, dir, con_id)) == NULL)
        {
            return NULL;
        }

        /* Initialise endpoint synchronisation values */
        endpoint->state.file.running = FALSE;
        endpoint->state.file.head_of_sync = endpoint;
        endpoint->state.file.nep_in_sync = NULL;
        endpoint->can_be_closed = TRUE;
        endpoint->can_be_destroyed = FALSE;
        /* Audio endpoints are always at the end of a chain */
        endpoint->is_real = TRUE;

        /* If number of channels is greater than allowed, fail */
        if(num_ch > NR_MAX_SYNCED_EPS)
        {
            stream_destroy_endpoint(endpoint);
            return NULL;
        }

        /* Creates master (or slave/logical) endpoint */
        if (master_id == 0)
        {
            /* Create the port descriptor and fill the default values */
            endpoint->state.file.port = a7da_audio_common_get_port(endpoint, dir,
                                            STREAM_DEVICE_FILE, instance, num_ch, cb_addr);
            if (endpoint->state.file.port == NULL)
            {
                cbops_mgr_destroy(endpoint->cbops);
                stream_destroy_endpoint(endpoint);
                return NULL;
            }
            /* Populate the cbops structure */
            if(!a7da_audio_common_create_cbops(endpoint,num_ch))
            {
                a7da_audio_common_destroy_port(endpoint->state.file.port);
                stream_destroy_endpoint(endpoint);
                return NULL;
            }
        }
        else
        {
            ENDPOINT *master = stream_endpoint_from_extern_id(master_id);
            /* Take the port descriptor and cbops from the master endpoint */
            endpoint->state.file.port = master->state.file.port;
            endpoint->cbops = master->cbops;
        }

        /* Initial value of block_size.   May be changed by EP_BLOCK_SIZE config message that
           is issued when the scheduling is negotiated at connection.  block_size is not used */
        endpoint->state.file.block_size = a7da_audio_common_get_buffer_size(endpoint)/(2*num_ch);
    }

    return endpoint;
}

/* ********************************** API functions ************************************* */

/*
 * \brief configure an file endpoint with a key and value pair
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value value to which the key is to be configured
 *
 * \return Whether the request succeeded.
 *
 */
static bool file_configure(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    switch(key)
    {
    case EP_BLOCK_SIZE:
        /* Use block size and sample rate to calculate kick period */
        endpoint->state.file.block_size = (unsigned int)value;
        return TRUE;
    case EP_DATA_FORMAT:
    case KASCMD_CONFIG_KEY_AUDIO_DATA_FORMAT:
        return audio_common_set_data_format(endpoint, (AUDIO_DATA_FORMAT)value);
    case KASCMD_CONFIG_KEY_AUDIO_SAMPLE_RATE:
        endpoint->state.file.port->sample_rate = (unsigned) value;
        return TRUE;
    case KASCMD_CONFIG_KEY_DRAM_PACKING_FORMAT:
        endpoint->state.file.port->word_format = (unsigned) value;
        return TRUE;
    case KASCMD_CONFIG_KEY_INTERLEAVING_MODE:
        /* For KAS file endpoints always carry interleaved data for multi channel
           no need to configure it.*/
        return TRUE;
    case KASCMD_CONFIG_KEY_CLOCK_MASTER_SLAVE:
        endpoint->state.file.port->locally_clocked = (bool) value;
        return TRUE;
    case KASCMD_CONFIG_KEY_FLOW_CONTROL:
        endpoint->state.file.port->flow_control_enabled = (bool) value;
        return TRUE;
    case KASCMD_CONFIG_KEY_PERIOD_SIZE:
        return remote_cbuf_set_period_size(&endpoint->state.file.port->cb_desc_local, (unsigned) value);
    case EP_CBOPS_PARAMETERS:
    {
        bool retval;
        CBOPS_PARAMETERS *parameters = (CBOPS_PARAMETERS *)(uintptr_t) value;

        if (parameters)
        {
            /* cbops_mgr should not be updated when endpoint is running. */
            if (!endpoint->is_enabled &&  endpoint->cbops && \
                opmgr_override_pass_cbops_parameters(parameters, endpoint->cbops,
                          endpoint->state.file.source_buf, endpoint->state.file.sink_buf))
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

    case EP_RATEMATCH_ADJUSTMENT:
    {
        L4_DBG_MSG2("FILE_EP_RATEMATCH_ADJUSTMENT ep: %06p value: %06x", endpoint, value);
        /* For the sake of reducing the MIPS burden no safety checks just do it. */
        endpoint->state.file.rm_adjust_amount = (unsigned int)value;
        return TRUE;
    }
    case EP_RATEMATCH_REFERENCE:
        return FALSE;
    case EP_RATEMATCH_ENACTING:
    {
            /* Configure non-hardware enacting endpoint to use CBOPS SRA etc */
            return audio_configure_rm_enacting(endpoint, value);
    }
    default:
        return FALSE;
    }
}

/**
 *
 */
static int get_file_rate(ENDPOINT *ep)
{
    int32 val;

    /* If the endpoint isn't running or is locally clocked then there is nothing to compensate for,
     * so indicate that the rate is perfect. */
    if ((!ep->state.file.running) || (ep->state.file.port->locally_clocked))
    {
        return 1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT;
    }

    val = (ep->state.file.rm_int_rsamples << STREAM_RATEMATCHING_FIX_POINT_SHIFT) / ep->state.file.rm_int_wsamples;

    return (int) val;
}

/*
 * \brief get file endpoint configuration
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 *
 */
static bool file_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    switch(key)
    {
    case EP_BLOCK_SIZE:
        /* Use block size and sample rate to calculate kick period */
        result->u.value = endpoint->state.file.block_size;
        return TRUE;
    case EP_DATA_FORMAT:
    case KASCMD_CONFIG_KEY_AUDIO_DATA_FORMAT:
        result->u.value = (uint32)audio_get_data_format(endpoint);
        return TRUE;
    case STREAM_INFO_KEY_AUDIO_SAMPLE_RATE:
    case KASCMD_CONFIG_KEY_AUDIO_SAMPLE_RATE:
        result->u.value = endpoint->state.file.port->sample_rate;
        return TRUE;
    case KASCMD_CONFIG_KEY_DRAM_PACKING_FORMAT:
        result->u.value = endpoint->state.file.port->word_format;
        return TRUE;
    case KASCMD_CONFIG_KEY_INTERLEAVING_MODE:
        result->u.value = endpoint->state.file.port->interleaved;
        return TRUE;
    case KASCMD_CONFIG_KEY_CLOCK_MASTER_SLAVE:
        result->u.value = endpoint->state.file.port->locally_clocked;
        return TRUE;
    case KASCMD_CONFIG_KEY_PERIOD_SIZE:
        result->u.value = remote_cbuf_get_period_size(&endpoint->state.file.port->cb_desc_local);
        return TRUE;
    case KASCMD_CONFIG_KEY_FLOW_CONTROL:
        result->u.value = endpoint->state.file.port->flow_control_enabled;
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

        if (cbops_flags & CBOPS_SHIFT)
        {
            if(endpoint->direction == SOURCE)
            {
                if (!cbops_parameters_set_shift_amount(parameters, 8))
                {
                    return FALSE;
                }
            }
            else
            {
                if (!cbops_parameters_set_shift_amount(parameters, -8))
                {
                    return FALSE;
                }
            }
        }
        result->u.value = (uint32)(uintptr_t) parameters;
        return TRUE;
    }
    case EP_RATEMATCH_ABILITY:
    {
        uint32 tmp;
#ifdef TODO_ENABLE_FILE_ENDPOINT_RATE_MATCHING
        if (endpoint->state.file.port->locally_clocked)
        {
            tmp = (uint32)RATEMATCHING_SUPPORT_AUTO;
        }
        else
        {
            tmp = (uint32)RATEMATCHING_SUPPORT_SW;
        }
#else  /* TODO_ENABLE_FILE_ENDPOINT_RATE_MATCHING */
        tmp = (uint32)RATEMATCHING_SUPPORT_AUTO;
#endif /* TODO_ENABLE_FILE_ENDPOINT_RATE_MATCHING */
        result->u.value = tmp;
        endpoint->state.file.port->rm_support = (uint24)tmp;
        return TRUE;
    }
    case EP_RATEMATCH_RATE:
    {
        /* This is the code to do the measured audio rate */
        result->u.value = get_file_rate(endpoint);
        return TRUE;
    }
    case EP_RATEMATCH_MEASUREMENT:
    {
        /* This is the code to do the measured audio rate */
        result->u.rm_meas.sp_deviation =
                STREAM_RATEMATCHING_RATE_TO_FRAC(get_file_rate(endpoint));
        result->u.rm_meas.measurement.valid = FALSE;
        return TRUE;
    }
    default:
        return FALSE;
    }
}

static void calc_file_rm_updates(kas_remote_cbuffer *remote_cb, endpoint_file_state *file)
{
    uint24 delta_samples;



    delta_samples = CIRC_SUB(remote_cb->read_ptr_offset, file->rm_prev_rp, remote_cb->size);
    file->rm_int_rsamples -= file->rm_int_rsamples >> (RM_AVG_SHIFT);
    file->rm_int_rsamples += delta_samples << (RM_FIX_POINT_SHIFT);

    delta_samples = CIRC_SUB(remote_cb->write_ptr_offset, file->rm_prev_wp, remote_cb->size);
    file->rm_int_wsamples -= file->rm_int_wsamples >> (RM_AVG_SHIFT);
    file->rm_int_wsamples += delta_samples << (RM_FIX_POINT_SHIFT);

    file->rm_prev_rp = remote_cb->read_ptr_offset;
    file->rm_prev_wp = remote_cb->write_ptr_offset;
}



static void get_file_rm_data(ENDPOINT *ep)
{
    endpoint_file_state *file = &ep->state.file;
    kas_remote_cbuffer *remote_cb = &ep->state.file.port->cb_desc_local;
    a7da_audio_port *port = ep->state.file.port;

    if(ep->direction == SOURCE)
    {
        remote_cbuf_get_wptr(remote_cb, port->cb_desc_addr);
    }
    else
    {
        remote_cbuf_get_rptr(remote_cb, port->cb_desc_addr);
    }

    if(file->rm_period_started)
    {
        if(ep->direction == SOURCE)
        {
            if(file->rm_prev_wp != remote_cb->write_ptr_offset)
            {
                calc_file_rm_updates(remote_cb, file);
            }
        }
        else /* ep->direction == SINK */
        {
            if(file->rm_prev_rp != remote_cb->read_ptr_offset)
            {
                calc_file_rm_updates(remote_cb, file);
            }
        }
    }
    else
    {
        file->rm_prev_rp = remote_cb->read_ptr_offset;
        file->rm_prev_wp = remote_cb->write_ptr_offset;
        file->rm_period_started = TRUE;
    }
}

void kas_file_rm_init(ENDPOINT *ep)
{
    /* pre-load the rolling average counts of samples written/read with the value corresponding to a perfect rate match */
    ep->state.file.rm_int_wsamples = (uint48)ep->state.file.block_size << (RM_AVG_SHIFT + RM_FIX_POINT_SHIFT);
    ep->state.file.rm_int_rsamples = ep->state.file.rm_int_wsamples;

    ep->state.file.rm_adjust_amount = 0;
    ep->state.file.rm_diff = 0;
    ep->state.file.rm_period_started = FALSE;
}

/**
 * \brief Performs a port buffer copy. It simply tries to copy as much data as
 * possible to/from the port. This might turn out to be too simple.
 *
 * \param endpoint pointer to the endpoint that received a kick
 * \param valid whether the kick data is valid
 */
static void file_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir)
{
    ENDPOINT_KICK_DIRECTION dir;

    if (!endpoint->state.file.running)
    {
        return;
    }

    if(endpoint->state.file.port->rm_support == RATEMATCHING_SUPPORT_SW)
    {
        get_file_rm_data(endpoint);
    }

    /*  For Sources Kick comes from scheduled timer */
    stream_debug_buffer_levels(endpoint);
    /* Execute the cbops chain, it will copy as much as it can
        * into/out of the port and execute the necessary cbops operations. */
    if (IS_FILE_ENDPOINT_HEAD_OF_SYNC(endpoint))
    {
        cbops_mgr_process_data(endpoint->cbops, CBOPS_MAX_COPY_SIZE-1);

        /* Only submit DMA requests when kicked externally */
        if (kick_dir != STREAM_KICK_INTERNAL)
        {
            submit_dma_request(endpoint->state.file.port->dma_req);
        }
    }

    if ((kick_dir == STREAM_KICK_INTERNAL)||
        ((endpoint->direction==SOURCE) && (kick_dir==STREAM_KICK_BACKWARDS)))
    {
        /* Work out which way this sync group kicks (if it is a sync group) */
        if (SOURCE == endpoint->direction)
        {
            dir = STREAM_KICK_FORWARDS;
        }
        else
        {
            dir = STREAM_KICK_BACKWARDS;
        }
        /* Kick all the synchronised endpoints in the group. */
        while ((endpoint != NULL))
        {
            /* The endpoints work is finished if it is a source then we need to kick
             * the next endpoint in the chain. If it is a sink we are the end of the
             * chain and then the kicks will go backwards. */
            propagate_kick(endpoint, dir);

            endpoint = endpoint->state.file.nep_in_sync;
        }

    }
}

/**
 * \brief Starts a writing or reading to/from a file endpoint.
 *
 * \param ep pointer to the endpoint which is responsible for scheduling
 * the kick.
 * \param ko pointer to the KICK_OBJECT that will call the kick function.
 *
 * \return TRUE/FALSE success or failure
 */
static bool file_start (ENDPOINT *ep, KICK_OBJECT *ko)
{
    if (ep->state.file.running)
    {
        return TRUE;
    }

    if (IS_FILE_ENDPOINT_HEAD_OF_SYNC(ep))
    {
        /* Initialise shared buffers */
        kas_remote_cbuffer *cb = &ep->state.file.port->cb_desc_local;
        remote_cbuf_init(cb);
        if (ep->state.file.port->ilv_buf)
        {   /* Reset the interleaving buffer */
            cbuffer_empty_buffer(ep->state.file.port->ilv_buf);
        }
        /* Initialise ratematching */
        kas_file_rm_init(ep);
        L4_DBG_MSG2("file ep started %04x ep_id: %06x", ep, stream_external_id_from_endpoint(ep));
    }

    /* Reset the channel buffers */
    cbuffer_empty_buffer(ep->state.file.source_buf);
    cbuffer_empty_buffer(ep->state.file.sink_buf);
    /* Only sinc should be set as running, source will be change when data produced is received.*/
    if(ep->direction==SINK)
    {
        ep->state.file.running = TRUE;
    }
    return TRUE;
}


/**
 * \brief Stops the kick interrupt source that was started by this endpoint.
 *
 * \param ep Pointer to the endpoint this function is being called on.
 *
 * \return TRUE/FALSE success or failure
 */
static bool file_stop (ENDPOINT *ep)
{
    if (!ep->state.file.running)
    {
        return FALSE;
    }

    if (IS_FILE_ENDPOINT_HEAD_OF_SYNC(ep))
    {
        remote_cbuf_init(&ep->state.file.port->cb_desc_local);
        L4_DBG_MSG2("file ep stop %04x ep_id: 0x%06x", ep, stream_external_id_from_endpoint(ep));
    }
    ep->state.file.running = FALSE;
    return TRUE;
}
