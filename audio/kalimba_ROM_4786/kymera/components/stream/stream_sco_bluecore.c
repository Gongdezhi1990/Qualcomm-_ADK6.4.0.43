/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_sco_bluecore.c
 * \ingroup stream
 *
 * stream sco type file. <br>
 * This file contains stream functions for bluecore sco endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_sco_get_endpoint <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "sco_mgr.h"
#include "io_defs.h"            /* for cbuffer_reconfigure_rd|wr_port */
#include "cbops_mgr/cbops_mgr.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/
#define BC_SCO_BUFFER_SIZE 256
/** The location of the port number in the sco_get_endpoint params */
#define PORT_NUM    0
/** The location of the hci handle in the sco_get_endpoint params */
#define HCI_HANDLE  1
/** The location of the compression factor in the sco_get_endpoint params */
#define COMP_FACTOR 2

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static bool sco_close(ENDPOINT *endpoint);
static bool sco_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect);
static bool sco_disconnect(ENDPOINT *endpoint);
static void sco_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static bool sco_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details);
static void sco_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);

DEFINE_ENDPOINT_FUNCTIONS(sco_functions, sco_close, sco_connect,
                          sco_disconnect, sco_buffer_details,
                          sco_kick, sco_sched_kick, sco_start,
                          sco_stop, sco_configure, sco_get_config,
                          sco_get_timing, stream_sync_sids_dummy);

/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
 *
 * stream_sco_get_endpoint
 *
 */
ENDPOINT *stream_sco_get_endpoint(unsigned con_id, ENDPOINT_DIRECTION dir,
                                    unsigned num_params, unsigned *params)
{
    ENDPOINT *ep;
    unsigned key;
	
    /* We should receive 3 parameters: port no, hci_handle, comp_factor */
    if (num_params != 3)
    {
        return NULL;
    }

    /* The hci handle forms the key (unique for the type and direction) */
    key = params[HCI_HANDLE];

    /* Return the requested endpoint (NULL if not found) */
    ep = stream_get_endpoint_from_key_and_functions(key, dir,
                                                  &endpoint_sco_functions);
    if (!ep)
    {
        if ((ep = STREAM_NEW_ENDPOINT(sco, key, dir, con_id)) == NULL)
        {
            /* couldn't create the endpoint */
            return NULL;
        }

        /* Kick off the requesting sco info as this will take time, and we'll
         * get connected very soon on a bluecore. */
        if (!sco_mgr_link_init(params[HCI_HANDLE], dir, params[COMP_FACTOR],
                                params[PORT_NUM]))
        {
            stream_destroy_endpoint(ep);
            return NULL;
        }

        /* Wrap the port up as a cbuffer, if we can't do that then fail here */
        if(SOURCE == dir)
        {
            if (NULL == (ep->state.sco.source_buf =
                    cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_WR, params[PORT_NUM])))
            {
                stream_destroy_endpoint(ep);
                return NULL;
            }
            cbuffer_reconfigure_read_port(params[PORT_NUM], BITMODE_16BIT);      /* (sign extension), 16-bit */
        }
        else
        {
            if (NULL == (ep->state.sco.sink_buf =
                    cbuffer_wrap_bc_port(BUF_DESC_MMU_BUFFER_HW_RD, params[PORT_NUM])))
            {
                stream_destroy_endpoint(ep);
                return NULL;
            }
            cbuffer_reconfigure_write_port(params[PORT_NUM], BITMODE_16BIT);      /* (no saturation), 16-bit */
        }

        ep->state.sco.portid = params[PORT_NUM];
        ep->can_be_closed = TRUE;
        ep->can_be_destroyed = TRUE;
        ep->is_real = TRUE;

        /* initialise measured rate */
        ep->state.sco.rate_measurement = 1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT;
        
        /* By default we expect NBS FIXP audio, so activate the necessary cbops */
        if(( ep->cbops = cbops_mgr_create(ep->direction, CBOPS_SHIFT)) == NULL)
        {
            stream_destroy_endpoint(ep);
            return NULL;
        }
    }

    return ep;
}

/****************************************************************************
Private Function Definitions
*/

static bool sco_close(ENDPOINT *endpoint)
{
    /* Once the endpoint is disconnected we can also destroy
     * the cbops_mgr. */
    if ( !cbops_mgr_destroy(endpoint->cbops) )
    {
        return FALSE;
    }

    /* cbops was successfully destroyed so delete the reference. */
    endpoint->cbops =NULL;

    /* By the time we reach this function we will have stopped everything from
     * running, so all we need to do is tidy up the buffer for the port.
     */
    if (SOURCE == endpoint->direction)
    {
        cbuffer_destroy(endpoint->state.sco.source_buf);
    }
    else
    {
        cbuffer_destroy(endpoint->state.sco.sink_buf);
    }

    /* Let the sco_mgr know that this endpoint is going away. */
    return sco_mgr_link_remove(endpoint->key, endpoint->direction,
                            endpoint->state.sco.portid);
}

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
static bool sco_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick, bool* start_on_connect)
{
    endpoint->ep_to_kick = ep_to_kick;
    if (SOURCE == endpoint->direction)
    {
        /* Remember this buffer so that the endpoint can write into it
         * when it gets kicked */
        endpoint->state.sco.sink_buf = Cbuffer_ptr;
        /* Make a record of the SCO source endpoint in the sps structure for
         * first kick scheduling calculations. */
        sco_from_air_endpoint_set(stream_sco_get_hci_handle(endpoint), endpoint);
    }
    else
    {
        /* Remember this buffer so that the endpoint can read from it
         * when it gets kicked */
        endpoint->state.sco.source_buf = Cbuffer_ptr;
        /* Make a record of the SCO sink endpoint in the sps structure for
         * first kick scheduling calculations. */
        sco_to_air_endpoint_set(stream_sco_get_hci_handle(endpoint), endpoint);
    }
    *start_on_connect = FALSE;
    /* The cbops has been created, create cbops chain. */
    return cbops_mgr_connect(endpoint->cbops, 1, &endpoint->state.sco.source_buf,
                             &endpoint->state.sco.sink_buf, NULL);
}

static bool sco_disconnect(ENDPOINT *endpoint)
{
    cbops_mgr_disconnect(endpoint->cbops);

    /* TODO do we need to stop any scheduling at this time? */
    if (SOURCE == endpoint->direction)
    {
        endpoint->state.sco.sink_buf = NULL;
    }
    else
    {
        endpoint->state.sco.source_buf = NULL;
    }

    return TRUE;
}

void sco_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    /* Most of the work here is the same on all platforms, the wants kicks field
     * differs and is populated by the platform specific code. */
    sco_common_get_timing(endpoint, time_info);

    if (SOURCE == endpoint->direction)
    {
        time_info->wants_kicks = FALSE;
    }
    else
    {
        time_info->wants_kicks = TRUE;
    }
}

static bool sco_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }

    /* This is the size of the xap side buffer, which we mirror */
    details->b.buff_params.size = BC_SCO_BUFFER_SIZE;
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER;
    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;
    return TRUE;
}

/**
 * \brief Performs a port buffer copy. It simply tries to copy as much data as
 * possible to/from the port. This might turn out to be too simple.
 *
 * \param endpoint pointer to the endpoint that received a kick
 * \param kick_dir kick direction
 */
static void sco_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir)
{
    /* This should never get kicked from down stream because the source endpoint
     * asked not to be during connect, so we don't bother checking where the kick
     * came from. */

    /* Execute the cbops chain, it will copy as much as we can
     * into/out of the port and execute the necessary cbops_operation. */
    cbops_mgr_process_data(endpoint->cbops, BC_SCO_BUFFER_SIZE);

    /* The endpoints work is finished if it is a source then we need to kick
     * the next endpoint in the chain. If it is a sink we are the end of the
     * chain and there is nothing more to do. */
    if (SOURCE == endpoint->direction)
    {
        propagate_kick(endpoint, STREAM_KICK_INTERNAL);
    }
}

/**
 * sco_get_data_format
 */
AUDIO_DATA_FORMAT sco_get_data_format (ENDPOINT *endpoint)
{
    patch_fn_shared(stream_sco);
    if(stream_direction_from_endpoint(endpoint) == SOURCE)
    {
        if ((cbops_get_flags(endpoint->cbops) & CBOPS_SHIFT) == CBOPS_SHIFT)
        {
            return AUDIO_DATA_FORMAT_FIXP_WITH_METADATA;
        }
        return AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA;
    }
    else
    {
        if ((cbops_get_flags(endpoint->cbops) & CBOPS_SHIFT) == CBOPS_SHIFT)
        {
            return AUDIO_DATA_FORMAT_FIXP;
        }
        return AUDIO_DATA_FORMAT_16_BIT;
    }
}

/**
 * sco_set_data_format
 */
bool sco_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format)
{
    patch_fn_shared(stream_sco);
    /* The data format can only be set before connect */
    if (NULL != endpoint->connected_to)
    {
        return FALSE;
    }
    if (endpoint->is_enabled)
    {
        /* cbops_mgr should not be updated when endpoint is running. */
        return FALSE;
    }

    /* Sources and sinks have different data formats due to metadata */
    if(stream_direction_from_endpoint(endpoint) == SOURCE)
    {
        switch(format)
        {
        case AUDIO_DATA_FORMAT_FIXP_WITH_METADATA:
            cbuffer_reconfigure_read_port(endpoint->state.sco.portid,
                            BITMODE_16BIT);      /* (sign extension), 16-bit */
            CBOP_VALS vals;
            vals.shift_amount = 8;
            return cbops_mgr_append(endpoint->cbops, CBOPS_SHIFT, 1,
                    &endpoint->state.sco.source_buf, &endpoint->state.sco.sink_buf, &vals, FALSE);
        case AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA:
            cbuffer_reconfigure_read_port(endpoint->state.sco.portid,
                            BITMODE_16BIT | BYTESWAP_MASK | NOSIGNEXT_MASK);
            return cbops_mgr_remove(endpoint->cbops, CBOPS_SHIFT, 1,
                    &endpoint->state.sco.source_buf, &endpoint->state.sco.sink_buf, FALSE);
        default:
            return FALSE;
        }
    }
    else
    {
        switch(format)
        {
        case AUDIO_DATA_FORMAT_FIXP:
            cbuffer_reconfigure_write_port(endpoint->state.sco.portid,
                            BITMODE_16BIT);      /* (not saturating), 16-bit */
            CBOP_VALS vals;
            vals.shift_amount = -8;
            return cbops_mgr_append(endpoint->cbops, CBOPS_SHIFT, 1,
                    &endpoint->state.sco.source_buf, &endpoint->state.sco.sink_buf, &vals, FALSE);
        case AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP:
            cbuffer_reconfigure_write_port(endpoint->state.sco.portid,
                            BITMODE_16BIT | BYTESWAP_MASK);
            return cbops_mgr_remove(endpoint->cbops, CBOPS_SHIFT, 1,
                    &endpoint->state.sco.source_buf, &endpoint->state.sco.sink_buf, FALSE);
            break;
        default:
            return FALSE;
        }
    }

}

/**
 * flush_endpoint_buffers
 */
void flush_endpoint_buffers(ENDPOINT *ep)
{
    patch_fn_shared(stream_sco);
    /* Sync read and write pointers, at the port side - source buf for source, sink buf for sink endpoint,
     * in BC case we don't have to zero buffer (it is not spinning as it is copied from/to port side at
     * each timed kick based on actual amount of data. So it would not copy leftover rubbish.
     */
    cbuffer_move_write_to_read_point(ep->state.sco.sink_buf, 0);
    cbuffer_move_read_to_write_point(ep->state.sco.source_buf, 0);
}

