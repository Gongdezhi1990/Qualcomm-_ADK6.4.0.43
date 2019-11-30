/**
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 * \file  volume_control.c
 * \ingroup  capabilities
 *
 *  Volume Control
 *
 */

/****************************************************************************
Include Files
 */
#include "capabilities.h"
#include "volume_control_cap.h"

#include "platform/pl_interrupt.h"
#include "platform/pl_fractional.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "op_msg_helpers.h"
#include "vol_ctrl_config.h"
#include "common_conversions.h"

#include "adaptor/adaptor.h"
#include "obpm_prim.h"

#include "patch/patch.h"

/****************************************************************************
Private Constant Definitions
 */

#define VOL_CTRL_BLOCK_SIZE      1
#define VOL_CTRL_BUFFER_SIZE     128

#define ZEROdBinQ5  1<<(DAWTH-5)

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define _VOL_CTRL_VOL_CAP_ID CAP_ID_DOWNLOAD_VOL_CTRL_VOL
#else
#define _VOL_CTRL_VOL_CAP_ID CAP_ID_VOL_CTRL_VOL
#endif

#define VOL_CTRL_CHANNEL_MASK ((1<<VOL_CTRL_CONSTANT_NUM_CHANNELS)-1)

typedef enum {
    VOL_CTRL_KICK_WAIT_ON_DATA,
    VOL_CTRL_KICK_WAIT_ON_SPACE,
    VOL_CTRL_KICK_WAIT_ON_DATA_LATE,
    VOL_CTRL_KICK_WAIT_ON_SPACE_LATE,
    VOL_CTRL_KICK_AUX_PENDING
} VOL_CTRL_KICK_REASON;

/****************************************************************************
Private Function Definitions
 */

/* Assembly Functions */
void vol_ctrl_update_aux_state(vol_ctrl_data_t *op_extra_data,unsigned num_auxs,vol_time_constants_t *lpvcs);
void vol_ctrl_compute_time_constants(unsigned sample_rate,unsigned vol_tc,vol_time_constants_t *lpvcs);
void vol_ctrl_update_channel(vol_ctrl_data_t *op_extra_data, vol_ctrl_channel_t *chan_ptr,vol_ctrl_gains_t *volptr, vol_time_constants_t *lpvcs);

/* Local functions */
static bool vol_ctrl_fixup_buffer_details(vol_ctrl_data_t *op_extra_data, unsigned terminal_id, OP_BUF_DETAILS_RSP *resp);
static void vol_ctrl_recalc_main_buffer_size(vol_ctrl_data_t *op_extra_data, unsigned terminal_id);
static void vol_ctrl_recalc_aux_buffer_size(vol_ctrl_data_t *op_extra_data);
static void vol_ctrl_kick_waiting(vol_ctrl_data_t *op_extra_data, TOUCHED_TERMINALS *touched, VOL_CTRL_KICK_REASON reason);

/** The volume control capability function handler table */
const handler_lookup_struct vol_ctlr_handler_table =
{
    vol_ctlr_create,          /* OPCMD_CREATE */
    vol_ctlr_destroy,         /* OPCMD_DESTROY */
    vol_ctlr_start,           /* OPCMD_START */
    base_op_stop,             /* OPCMD_STOP */
    base_op_reset,            /* OPCMD_RESET */
    vol_ctlr_connect,         /* OPCMD_CONNECT */
    vol_ctlr_disconnect,      /* OPCMD_DISCONNECT */
    vol_ctlr_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,  /* OPCMD_DATA_FORMAT */
    vol_ctlr_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry vol_ctlr_opmsg_handler_table[] =
{{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,                base_op_opmsg_get_capability_version},

        {OPMSG_COMMON_ID_SET_CONTROL,                       vol_ctlr_opmsg_obpm_set_control},
        {OPMSG_COMMON_ID_GET_PARAMS,                        vol_ctlr_opmsg_obpm_get_params},
        {OPMSG_COMMON_ID_GET_DEFAULTS,                      vol_ctlr_opmsg_obpm_get_defaults},
        {OPMSG_COMMON_ID_SET_PARAMS,                        vol_ctlr_opmsg_obpm_set_params},
        {OPMSG_COMMON_ID_GET_STATUS,                        vol_ctlr_opmsg_obpm_get_status},

        {OPMSG_COMMON_ID_SET_UCID,                          vol_ctlr_opmsg_set_ucid},
        {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                 vol_ctlr_opmsg_get_ps_id},

        {OPMSG_COMMON_SET_SAMPLE_RATE,                      vol_ctlr_opmsg_set_sample_rate},
        {OPMSG_COMMON_SET_DATA_STREAM_BASED,                vol_ctlr_opmsg_data_stream_based},
        {0, NULL}};

/* Capability descriptor */
const CAPABILITY_DATA vol_ctlr_cap_data =
{
        _VOL_CTRL_VOL_CAP_ID,             /* Capability ID */
        VOL_CTRL_VOL_VERSION_MAJOR, VOL_CTRL_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        2*VOL_CTRL_CONSTANT_NUM_CHANNELS,VOL_CTRL_CONSTANT_NUM_CHANNELS,  /* Max number of sinks/inputs and sources/outputs */
        &vol_ctlr_handler_table,      /* Pointer to message handler function table */
        vol_ctlr_opmsg_handler_table, /* Pointer to operator message handler function table */
        vol_ctlr_process_data,        /* Pointer to data processing function */
        0,                               /* TODO - Processing time information */
        sizeof(vol_ctrl_data_t)      /* Size of capability-specific per-instance data */
};

/****************************************************************************
Public Function Declarations
 */


/* ********************************** API functions ************************************* */

bool vol_ctlr_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    vol_ctrl_data_t *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;

    patch_fn_shared(volume_control_wrapper);

    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /*allocate the colume control shared memory*/
    op_extra_data->shared_volume_ptr = allocate_shared_volume_cntrl();
    if(!op_extra_data->shared_volume_ptr)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

      op_extra_data->lpvols = &op_extra_data->host_vol;
      op_extra_data->stream_based = FALSE;

      /* Initialize mute control */
      op_extra_data->mute_period = 10;

    /* Initialize extended data for operator.  Assume intialized to zero*/
    op_extra_data->ReInitFlag = 1;

    if(!cpsInitParameters(&op_extra_data->parms_def,(unsigned*)VOL_CTRL_GetDefaults(op_data->cap_data->id),(unsigned*)&op_extra_data->parameters,sizeof(VOL_CTRL_PARAMETERS)))
    {
       base_op_change_response_status(response_data, STATUS_CMD_FAILED);
       return TRUE;
    }

    return TRUE;
}



bool vol_ctlr_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    vol_ctrl_data_t *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;

    /* call base_op destroy that creates and fills response message, too */
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        return(FALSE);
    }

    /*free volume control shared memory*/
    release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
    op_extra_data->shared_volume_ptr = NULL;

    /* Release Channels */
    destroy_processing(op_extra_data);

    return TRUE;
}

bool vol_ctlr_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_SCHED_INFO_RSP* resp;

    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    /* Populate the response*/
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;

    resp->block_size = VOL_CTRL_BLOCK_SIZE;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}

/**
 * \brief Placeholder for fixups
 * \param terminal_id Allow any implementation to tell what changed
 * \param resp The response to update
 */
static bool vol_ctrl_fixup_buffer_details(vol_ctrl_data_t *op_extra_data, unsigned terminal_id, OP_BUF_DETAILS_RSP *resp)
{
    patch_fn_shared(volume_control_wrapper);

    return TRUE;
}


bool vol_ctlr_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_BUF_DETAILS_RSP *resp;
    vol_ctrl_data_t* opx_data;
    unsigned buffer_size, base_buffer_size;
#if !defined(DISABLE_IN_PLACE) || defined(INSTALL_METADATA)
    unsigned terminal_id;
#if !defined(DISABLE_IN_PLACE) && !defined(VC_NOT_RUN_IN_PLACE)
    unsigned term_idx;
#endif /* !defined(DISABLE_IN_PLACE) && !defined(VC_NOT_RUN_IN_PLACE) */
#endif /* !defined(DISABLE_IN_PLACE) || defined(INSTALL_METADATA) */

    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    resp = (OP_BUF_DETAILS_RSP*)*response_data;
    base_buffer_size = resp->b.buffer_size;
    
    opx_data = (vol_ctrl_data_t*)(op_data->extra_op_data);

#ifdef INSTALL_METADATA
    {
        /* If an input/output connection is already present and has metadata then
         * we are obliged to return that buffer so that metadata can be shared
         * between channels. */
        tCbuffer *meta_buff;
        terminal_id = ((unsigned *)message_data)[0];
        if ((terminal_id & TERMINAL_SINK_MASK) == TERMINAL_SINK_MASK)
        {
            /* Only the main input channels consume metadata, the aux inputs do not */
            if ((terminal_id & 0x1) == 0)
            {
                meta_buff = ((vol_ctrl_data_t *)(op_data->extra_op_data))->metadata_ip_buffer;
                resp->supports_metadata = TRUE;
            }
            else
            {
                meta_buff = NULL;
                resp->supports_metadata = FALSE;
            }
        }
        else
        {
            meta_buff = ((vol_ctrl_data_t *)(op_data->extra_op_data))->metadata_op_buffer;
            resp->supports_metadata = TRUE;
        }

        resp->metadata_buffer = meta_buff;

    }
#endif /* INSTALL_METADATA */
    
#if !defined(DISABLE_IN_PLACE) && !defined(VC_NOT_RUN_IN_PLACE)
    terminal_id = ((unsigned *)message_data)[0];

    /* buffer size of 2.5ms worth at the sample rate of the operator */
    buffer_size = frac_mult(opx_data->sample_rate,FRACTIONAL(0.0025));

    if (buffer_size==0)
    {
       buffer_size = VOL_CTRL_BUFFER_SIZE;
    }
    
    if (buffer_size < base_buffer_size)
    {
        buffer_size = base_buffer_size;
    }
    
    if(terminal_id&TERMINAL_SINK_MASK)
    {
        if(terminal_id&0x1)
        {
            /* Don't run in place on the aux inputs. */
            resp->runs_in_place = FALSE;

            /* Set the asked buffer size. */
            resp->b.buffer_size = buffer_size;
            opx_data->aux_buff_size = buffer_size;

            return vol_ctrl_fixup_buffer_details(opx_data, terminal_id, resp);
        }
        else
        {
            term_idx = (terminal_id&TERMINAL_NUM_MASK)>>1;

            /*input terminal. give the output buffer for the channel */
            resp->b.in_place_buff_params.buffer = opx_data->output_buffer[term_idx] ;

            /* Choose terminal associated with the term_idx. */
            resp->b.in_place_buff_params.in_place_terminal = term_idx;
        }
    }
    else
    {
        /* The output terminal index is directly mapped to the input_buffer array */
        unsigned buffer_idx = (terminal_id&TERMINAL_NUM_MASK);
        /* The input terminal index is the buffer index multiplied by 2. */
        term_idx = buffer_idx << 1;

        /*output terminal. give the input buffer for the channel */
        resp->b.in_place_buff_params.buffer = opx_data->input_buffer[buffer_idx];

        /* Choose terminal associated with the term_idx. */
        resp->b.in_place_buff_params.in_place_terminal = term_idx | TERMINAL_SINK_MASK;
    }

    /* Run in place*/
    resp->runs_in_place = TRUE;

    /* Set the asked buffer size. */
    resp->b.in_place_buff_params.size = buffer_size;


#else  /* !defined(DISABLE_IN_PLACE) && !defined(VC_NOT_RUN_IN_PLACE) */
    resp->b.buffer_size = buffer_size;
#endif /* !defined(DISABLE_IN_PLACE) && !defined(VC_NOT_RUN_IN_PLACE) */

    return vol_ctrl_fixup_buffer_details(opx_data, terminal_id, resp);
}

/**
 * \brief Placeholder for fixups
 * \param terminal_id Allow any implementation to tell what changed
 */
static void vol_ctrl_recalc_main_buffer_size(vol_ctrl_data_t *op_extra_data, unsigned terminal_id)
{
    patch_fn_shared(volume_control_wrapper);
}

/**
 * \brief Update aux_buffer_size with the minimum of connected aux buffers' sizes
 */
static void vol_ctrl_recalc_aux_buffer_size(vol_ctrl_data_t *op_extra_data)
{
    unsigned ch;
    unsigned aux_buffer_min_size = MAXINT;

    patch_fn_shared(volume_control_wrapper);

    for (ch = 0; ch < VOL_CTRL_CONSTANT_NUM_CHANNELS; ch += 1)
    {
        tCbuffer* aux_buffer = op_extra_data->aux_channel[ch].buffer;
        if (aux_buffer != NULL)
        {
            unsigned size = cbuffer_get_size_in_words(aux_buffer);
            aux_buffer_min_size = MIN(aux_buffer_min_size, size);
        }
    }
    op_extra_data->aux_buff_size = aux_buffer_min_size;
}

bool vol_ctlr_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;
    unsigned terminal_id   = ((unsigned*)message_data)[0];    /* extract the terminal_id */
    tCbuffer* pterminal_buf = (tCbuffer*)(uintptr_t)(((unsigned *)message_data)[1]);

    /* Setup Response to Connection Request. */
    *response_id = OPCMD_CONNECT;
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    /* (i)  check if the terminal ID is valid . The number has to be less than the maximum number of sinks or sources .  */
    /* (ii) check if we are connecting to the right type . It has to be a buffer pointer and not endpoint connection */
    if( !base_op_is_terminal_valid(op_data, terminal_id) || !pterminal_buf)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if(op_data->state==OP_RUNNING)
    {
        /* Only Auxilliary Terminals Can be Altered while running */
        if(!(terminal_id & TERMINAL_SINK_MASK) || !(terminal_id&0x1))
        {
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }

    if(terminal_id&TERMINAL_SINK_MASK)
    {
        unsigned term_idx = (terminal_id&TERMINAL_NUM_MASK)>>1;

        if(terminal_id&0x1)
        {
            interrupt_block();
            op_extra_data->aux_connected |= (1<<term_idx);
            op_extra_data->aux_channel[term_idx].buffer = pterminal_buf;
            interrupt_unblock();
            vol_ctrl_recalc_aux_buffer_size(op_extra_data);
        }
        else
        {
            op_extra_data->input_buffer[term_idx] = pterminal_buf;
#ifdef INSTALL_METADATA
            if(op_extra_data->metadata_ip_buffer == NULL)
            {
                if (buff_has_metadata(op_extra_data->input_buffer[term_idx]))
                {
                    op_extra_data->metadata_ip_buffer = op_extra_data->input_buffer[term_idx];
                }
            }
#endif /* INSTALL_METADATA */
            vol_ctrl_recalc_main_buffer_size(op_extra_data, terminal_id);
        }
    }
    else
    {
        unsigned term_idx = terminal_id&TERMINAL_NUM_MASK;
        op_extra_data->output_buffer[term_idx] = pterminal_buf;
#ifdef INSTALL_METADATA
        if(op_extra_data->metadata_op_buffer == NULL)
        {
            if (buff_has_metadata(op_extra_data->output_buffer[term_idx]))
            {
                op_extra_data->metadata_op_buffer = op_extra_data->output_buffer[term_idx];
            }
        }
#endif /* INSTALL_METADATA */
        vol_ctrl_recalc_main_buffer_size(op_extra_data, terminal_id);
    }
    return TRUE;
}

bool vol_ctlr_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;
    unsigned terminal_id = *((unsigned*)message_data);

    /* Setup Response to Disconnection Request. */
    *response_id = OPCMD_DISCONNECT;
    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    /* (i)  check if the terminal ID is valid . The number has to be less than the maximum number of sinks or sources .  */
    /* (ii) check if we are connecting to the right type . It has to be a buffer pointer and not endpoint connection */
    if( !base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    if(op_data->state==OP_RUNNING)
    {
        /* Only Auxilliary Terminals Can be Altered while running */
        if(!(terminal_id & TERMINAL_SINK_MASK) || !(terminal_id&0x1))
        {
            base_op_change_response_status(response_data, STATUS_CMD_FAILED);
            return TRUE;
        }
    }

    if(terminal_id&TERMINAL_SINK_MASK)
    {
        unsigned term_idx = (terminal_id&TERMINAL_NUM_MASK)>>1;

        if(terminal_id&0x1)
        {
            interrupt_block();
            op_extra_data->aux_connected &= ~(1<<term_idx);
            op_extra_data->aux_channel[term_idx].buffer = NULL;
            interrupt_unblock();
            vol_ctrl_recalc_aux_buffer_size(op_extra_data);
        }
        else
        {
#ifdef INSTALL_METADATA
            if (op_extra_data->metadata_ip_buffer == op_extra_data->input_buffer[term_idx])
            {
                unsigned i;
                bool found_alternative = FALSE;
                for (i = 0; i < VOL_CTRL_CONSTANT_NUM_CHANNELS; i++)
                {
                    if (i == term_idx)
                    {
                        continue;
                    }
                    if (op_extra_data->input_buffer[i] != NULL && buff_has_metadata(op_extra_data->input_buffer[i]))
                    {
                        op_extra_data->metadata_ip_buffer = op_extra_data->input_buffer[i];
                        found_alternative = TRUE;
                        break;
                    }
                }
                if (!found_alternative)
                {
                    op_extra_data->metadata_ip_buffer = NULL;
                }
            }
#endif /* INSTALL_METADATA */
            op_extra_data->input_buffer[term_idx] = NULL;
            vol_ctrl_recalc_main_buffer_size(op_extra_data, terminal_id);
        }
    }
    else
    {
        unsigned term_idx = terminal_id&TERMINAL_NUM_MASK;
#ifdef INSTALL_METADATA
        if (op_extra_data->metadata_op_buffer == op_extra_data->output_buffer[term_idx])
        {
            unsigned i;
            bool found_alternative = FALSE;
            for (i = 0; i < VOL_CTRL_CONSTANT_NUM_CHANNELS; i++)
            {
                if (i == term_idx)
                {
                    continue;
                }
                if (op_extra_data->output_buffer[i] != NULL && buff_has_metadata(op_extra_data->output_buffer[i]))
                {
                    op_extra_data->metadata_op_buffer = op_extra_data->output_buffer[i];
                    found_alternative = TRUE;
                    break;
                }
            }
            if (!found_alternative)
            {
                op_extra_data->metadata_op_buffer = NULL;
            }
        }
#endif /* INSTALL_METADATA */
        op_extra_data->output_buffer[term_idx] = NULL;
        vol_ctrl_recalc_main_buffer_size(op_extra_data, terminal_id);
    }

    return TRUE;
}

bool vol_ctlr_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* Setup Response to Start Request.   Assume Failure*/
    *response_id = OPCMD_START;

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    if (op_data->state==OP_RUNNING)
    {
        return TRUE;
    }

    if (!setup_processing((vol_ctrl_data_t*)op_data->extra_op_data))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* set internal capability state variable to "running" */
    /* operator state variable set to not running state. based on base_op.c */
    op_data->state = OP_RUNNING;

    return TRUE;
}

/* ************************************* Data processing-related functions and wrappers **********************************/

void destroy_processing(vol_ctrl_data_t   *op_extra_data)
{
    if(!op_extra_data->channels)
    {
        return;
    }

    pfree(op_extra_data->channels);
    op_extra_data->channels = NULL;
}


static void vol_ctrl_setup_mute(vol_ctrl_data_t *op_extra_data,unsigned bMute)
{
   int mute_increment;

   /*  cur_mute_gain = 0 at start, and default mute=FALSE  
         To immediately unmute set mute_period=0
   */

   if((op_extra_data->sample_rate<1) || (op_extra_data->mute_period<1) )
   {
      /* Immediately set mute gain */
      op_extra_data->cur_mute_gain  = bMute ? 0 : FRACTIONAL(1.0);
      mute_increment=1;
   }
   else
   { 
      
      if(op_extra_data->mute_period<1000)
      {
	     mute_increment = pl_fractional_divide(op_extra_data->mute_period,1000);
		 mute_increment = pl_fractional_divide(1,frac_mult(mute_increment,op_extra_data->sample_rate)); 
      }
	  else
	  {
	     mute_increment = pl_fractional_divide(1,op_extra_data->sample_rate);
      }
   }

   /* Set direction of transition */
  op_extra_data->mute_increment = (bMute) ? -mute_increment : mute_increment;

}

bool setup_processing(vol_ctrl_data_t   *op_extra_data)
{
    unsigned touched_sink = TOUCHED_NOTHING;
    unsigned touched_src  = TOUCHED_NOTHING;
    unsigned i,sink_bit,initial_channel_gain,chan_count=0;
    vol_ctrl_channel_t *chan_ptr;
	vol_ctrl_gains_t *lpvols =&op_extra_data->host_vol;

    patch_fn_shared(volume_control_wrapper);

    /* Release data object */
    destroy_processing(op_extra_data);


    /* Validate channel sinks and sources.  Count connected channels   */
    for(i=0,sink_bit=0;i<VOL_CTRL_CONSTANT_NUM_CHANNELS;i++,sink_bit+=2)
    {
        if(op_extra_data->input_buffer[i])
        {
            if(op_extra_data->output_buffer[i])
            {
                chan_count++;
                touched_sink |= ( TOUCHED_SINK_0 <<sink_bit );
                touched_src  |=  (TOUCHED_SOURCE_0 << i);
            }
            else
            {
                return FALSE;
            }
        }
        else if(op_extra_data->output_buffer[i])
        {
            return FALSE;
        }
    }

    /* Must have at least one channel */
    if(chan_count<1)
    {
        return FALSE;
    }

    /* A valid set of channels is connected */
    op_extra_data->channels = (vol_ctrl_channel_t*)xzpmalloc(chan_count*sizeof(vol_ctrl_channel_t));
    if(!op_extra_data->channels)
    {
        return FALSE;
    }
    op_extra_data->touched_src  = touched_src;
    op_extra_data->touched_sink = touched_sink;
    op_extra_data->num_channels = chan_count;

    /* Initialize Mute */
    op_extra_data->cur_mute_gain = 0;
    vol_ctrl_setup_mute(op_extra_data,lpvols->mute);

    /* Link Channels with channel object */
    chan_ptr = op_extra_data->channels;
    for(i=0;i<VOL_CTRL_CONSTANT_NUM_CHANNELS;i++)
    {
        if(op_extra_data->output_buffer[i])
        {
		    /* If inital value not specified before operator start, use -96db as default
			 * If an initial value is specified calculate channel gain as master_gain + channel_trim - post gain
			 */ 
		    if(op_extra_data->vol_initialised)
		    {
			    initial_channel_gain = dB60toLinearQ5(op_extra_data->lpvols->channel_trims[i] + op_extra_data->lpvols->master_gain - op_extra_data->post_gain);
			}
			else
			{
				initial_channel_gain = 0;
			};
					 
			chan_ptr->chan_idx = i;
			chan_ptr->channel_gain = initial_channel_gain;
			chan_ptr->prim_mix_gain = dB60toLinearQ5(0); /*set to 0db. this will be recalculated as part of the update function*/
			chan_ptr->last_volume  = chan_ptr->channel_gain;
			chan_ptr++;
        }
    }

   op_extra_data->vol_initialised = 0;
   op_extra_data->wait_on_space_buffer=NULL;
   op_extra_data->wait_on_data_buffer=NULL;
   op_extra_data->aux_pending = FALSE;
   op_extra_data->used_all_input = FALSE;

    return TRUE;
}

void vol_ctlr_timer_task(void *kick_object)
{
    OPERATOR_DATA *op_data = (OPERATOR_DATA*)kick_object;
    vol_ctrl_data_t *op_extra_data = (vol_ctrl_data_t*) op_data->extra_op_data;

    patch_fn_shared(volume_control_wrapper);

    op_extra_data->pending_timer = TIMER_ID_INVALID;

    /* Raise a bg int to process */
    opmgr_kick_operator(op_data);
}

/**
 * \brief Hook for additional kicking in waiting situations
 */
static void vol_ctrl_kick_waiting(vol_ctrl_data_t *op_extra_data, TOUCHED_TERMINALS *touched, VOL_CTRL_KICK_REASON reason)
{
    patch_fn_shared(volume_control_wrapper);
}

RUN_FROM_PM_RAM
void vol_ctlr_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    vol_ctrl_data_t     *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;
    unsigned             i,samples_to_process,num_channels,amount,touched_sink;
    unsigned             block_size = VOL_CTRL_BLOCK_SIZE;

    patch_fn(volume_control_process_data_patch);

    if (! op_extra_data->aux_pending)
    {
        op_extra_data->used_all_input = FALSE;

        /* Accerators for data/space */
        samples_to_process = MAXINT;
        if(op_extra_data->stream_based)
        {
           num_channels = 1;
        }
        else
        {
           if(op_extra_data->wait_on_space_buffer)
           {
              samples_to_process = cbuffer_calc_amount_space_in_words(op_extra_data->wait_on_space_buffer);
              if(samples_to_process<block_size)
              {
                 vol_ctrl_kick_waiting(op_extra_data, touched, VOL_CTRL_KICK_WAIT_ON_SPACE_LATE);
                 return;
              }
              op_extra_data->wait_on_space_buffer=NULL;
           }
           if(op_extra_data->wait_on_data_buffer)
           {
              amount = cbuffer_calc_amount_data_in_words(op_extra_data->wait_on_data_buffer);
              if(amount<block_size)
              {
                 vol_ctrl_kick_waiting(op_extra_data, touched, VOL_CTRL_KICK_WAIT_ON_DATA_LATE);
                return;
              }
              op_extra_data->wait_on_data_buffer=NULL;
           }
           num_channels = op_extra_data->num_channels;
        }

        /* Compute channel transfer amount */
        for(i=0;i<num_channels;i++)
        {
          unsigned term_idx = op_extra_data->channels[i].chan_idx;
          unsigned in_buff_size;
          tCbuffer *buffer;

          buffer = op_extra_data->input_buffer[term_idx];
          amount = cbuffer_calc_amount_data_in_words(buffer);
          if(amount<=samples_to_process)
          {
             if(amount<=block_size)
             {
                op_extra_data->wait_on_data_buffer = buffer;
                vol_ctrl_kick_waiting(op_extra_data, touched, VOL_CTRL_KICK_WAIT_ON_DATA);
                return;
             }
             samples_to_process=amount;
             op_extra_data->used_all_input = TRUE;
          }

          buffer = op_extra_data->output_buffer[term_idx];
          amount = cbuffer_calc_amount_space_in_words(buffer);

          /* Relatively likely to need changes below */
          patch_fn(volume_control_adjust_amount);

          /* Account for any difference between aux and main channel buffer sizes */
          in_buff_size = cbuffer_get_size_in_words(op_extra_data->input_buffer[term_idx]);
          if ((op_extra_data->aux_connected != 0) && (in_buff_size > op_extra_data->aux_buff_size))
          {
              if (amount < (in_buff_size - op_extra_data->aux_buff_size))
              {
                  amount = 0;
              }
              else
              {
                  amount -= (in_buff_size - op_extra_data->aux_buff_size);
              }
          }

          if(amount<samples_to_process)
          {
             if(amount<block_size)
             {
                op_extra_data->wait_on_space_buffer = buffer;
                vol_ctrl_kick_waiting(op_extra_data, touched, VOL_CTRL_KICK_WAIT_ON_SPACE);
                return;
             }
             samples_to_process=amount;
             op_extra_data->used_all_input = FALSE;
          }
        }

        /* Time constants based on samples to process and sample rate */
        op_extra_data->tc.num_words = samples_to_process;
        vol_ctrl_compute_time_constants(op_extra_data->sample_rate,op_extra_data->parameters.OFFSET_VOLUME_TC,&op_extra_data->tc);
    }
    else
    {
        bool timer_valid;
        interrupt_block();
        timer_valid = (op_extra_data->pending_timer != TIMER_ID_INVALID);
        if (timer_valid)
        {
            timer_cancel_event(op_extra_data->pending_timer);
            op_extra_data->pending_timer = TIMER_ID_INVALID;
        }
        interrupt_unblock();
    }

    /* Update AUX state.
       If the aux stream doesn't have enough data it indicates a state change */
    unsigned aux_check = (op_extra_data->aux_active | op_extra_data->aux_connected)
                         & VOL_CTRL_CHANNEL_MASK;

    /* Relatively likely to need changes below */
    patch_fn(volume_control_aux_check);

    if (aux_check != 0)
    {
        vol_ctrl_aux_channel_t* aux_ch;
        unsigned aux_kick = 0;

        /* If not pending, check the pending condition first,
         * without updating aux state. That means checking all
         * connected aux channels' amount of data.
         * Thus, even if pending, get the amounts per aux channel
         * now and save in the advance_buffer field.
         */
        for (i = 0, aux_ch = &op_extra_data->aux_channel[0];
             i < VOL_CTRL_CONSTANT_NUM_CHANNELS;
             i += 1, aux_ch += 1)
        {
            if (aux_ch->buffer == NULL)
            {
                aux_ch->advance_buffer = 0;
            }
            else
            {
                amount = cbuffer_calc_amount_data_in_words(aux_ch->buffer);
                aux_ch->advance_buffer = amount;
                if (!op_extra_data->aux_pending && (amount < op_extra_data->tc.num_words))
                {
                    aux_kick |= (2 << (2 * i));
                }
            }
        }

        op_extra_data->aux_pending = (aux_kick != 0);
        if (op_extra_data->aux_pending)
        {
            /* While not already pending, one or more of the aux buffers did
             * not contain enough data. Simply kick all connected aux inputs. */
            touched->sinks = aux_kick;
            vol_ctrl_kick_waiting(op_extra_data, touched, VOL_CTRL_KICK_AUX_PENDING);

            op_extra_data->pending_timer = timer_schedule_event_in(
                stream_if_get_system_kick_period()/2,
                vol_ctlr_timer_task,
                (void*)op_data );
            return;
        }

        /* Now all aux inputs have enough data, or some aux inputs were left
         * pending once and some still don't have enough data, so those now
         * have to do a stopping transition. Amount of available data is in
         * each aux channel's advance_buffer field.
         */

        vol_ctrl_update_aux_state(op_extra_data, aux_check, &op_extra_data->tc);

    }
    else
    {
        op_extra_data->aux_pending = FALSE;
    }

#ifdef INSTALL_METADATA
    /* Propagate any metadata to the output. Any handling of it's presence or
     * not is handled by the metadata library */
    metadata_strict_transport(op_extra_data->metadata_ip_buffer,
                                op_extra_data->metadata_op_buffer,
                                op_extra_data->tc.num_words * OCTETS_PER_SAMPLE );
#endif  /* INSTALL_METADATA */

    /* Update Main Channels */
    vol_ctrl_update_channel(op_extra_data,op_extra_data->channels,op_extra_data->lpvols,&op_extra_data->tc);

    /* Handle backwards kicks for main channels */
    touched_sink = (op_extra_data->used_all_input) ? op_extra_data->touched_sink : 0;

   /* Update Aux Buffers */
   op_extra_data->aux_state   = 0;
   amount    = op_extra_data->aux_connected;
   if(amount)
   {
      vol_ctrl_aux_channel_t *aux_ptr=op_extra_data->aux_channel;
      i = 0;
      do
      {
          if(aux_ptr->state==AUX_STATE_IN_AUX)
          {
               unsigned index_mask = (1<<i);
               op_extra_data->aux_state |= index_mask;
               // only advance if used by a channel?
               if(!(op_extra_data->aux_in_use&index_mask))
               {
                   aux_ptr->advance_buffer=0;
               }
          }
          if(aux_ptr->advance_buffer)
          {
              touched_sink |= ( TOUCHED_SINK_0 << ((i<<1)+1) );
              cbuffer_advance_read_ptr(aux_ptr->buffer,aux_ptr->advance_buffer);
          }
          i++;
          aux_ptr++;
          amount>>=1;
      }while(amount);
   }

   touched->sinks   = touched_sink;
   touched->sources = op_extra_data->touched_src;

}


/* **************************** Operator message handlers ******************************** */

bool vol_ctlr_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t    *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;
    unsigned            i,num_controls,cntrl_value;
    CPS_CONTROL_SOURCE  cntrl_src;
    unsigned result = OPMSG_RESULT_STATES_NORMAL_STATE;
    vol_ctrl_gains_t *lpvols =&op_extra_data->host_vol;
    bool              bIsOBPM=FALSE;

    patch_fn(volume_control_opmsg_obpm_set_control_patch);

    if(!cps_control_setup(message_data, resp_length, resp_data,&num_controls))
    {
        return FALSE;
    }

    for(i=0;i<num_controls;i++)
    {
        unsigned  cntrl_id=cps_control_get(message_data,i,&cntrl_value,&cntrl_src);

        /* Check for OBPM and Overide.   Override is all or none, not per control */
        if((i==0)&&(cntrl_src != CPS_SOURCE_HOST))
        {
           bIsOBPM = TRUE;
           lpvols  = &op_extra_data->obpm_vol;
           /* Polarity of override enable/disable is inverted */
           op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ?  VOL_CTRL_CONTROL_VOL_OVERRIDE : 0;
        }

        if(cntrl_id==VOL_CTRL_CONSTANT_POST_GAIN_CTRL)
        {
            if(bIsOBPM)
            {
                /* OBPM can not set post gain */
                result = OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL;
                break;
            }

            op_extra_data->post_gain = cntrl_value;

            op_extra_data->shared_volume_ptr->inv_post_gain = dB60toLinearQ5(-op_extra_data->post_gain);
        }
        else if(cntrl_id==VOL_CTRL_CONSTANT_MASTER_GAIN_CTRL)
        {
            lpvols->master_gain = cntrl_value;
			if(op_data->state!=OP_RUNNING)
			{
				op_extra_data->vol_initialised = 1;
			}
        }
        else if(cntrl_id==OPMSG_CONTROL_MUTE_ID)
        {
            lpvols->mute = cntrl_value;
            if(op_data->state==OP_RUNNING)
            {
               vol_ctrl_setup_mute(op_extra_data,lpvols->mute);
            }
        }
        else if(cntrl_id==VOL_CTRL_CONSTANT_MUTE_PERIOD_CTRL)
        {
            op_extra_data->mute_period = cntrl_value;
            if(op_data->state==OP_RUNNING)
            {
               vol_ctrl_setup_mute(op_extra_data,lpvols->mute);
            }
        }
        else if( (cntrl_id>=VOL_CTRL_CONSTANT_TRIM1_GAIN_CTRL) &&
                 (cntrl_id<=VOL_CTRL_CONSTANT_TRIM8_GAIN_CTRL) )
        {
            lpvols->channel_trims[cntrl_id-VOL_CTRL_CONSTANT_TRIM1_GAIN_CTRL] = cntrl_value;
        }
        else if( (cntrl_id>=VOL_CTRL_CONSTANT_AUX_GAIN_CTRL1) &&
                 (cntrl_id<=VOL_CTRL_CONSTANT_AUX_GAIN_CTRL8) )
        {
            lpvols->auxiliary_gain[cntrl_id-VOL_CTRL_CONSTANT_AUX_GAIN_CTRL1] = cntrl_value;
        }
        else
        {
            result = OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL;
            break;
        }
    }

    if(op_extra_data->Ovr_Control&VOL_CTRL_CONTROL_VOL_OVERRIDE)
    {
        op_extra_data->lpvols = &op_extra_data->obpm_vol;
    }
    else
    {
        op_extra_data->lpvols = &op_extra_data->host_vol;
    }
    op_extra_data->shared_volume_ptr->current_volume_level = op_extra_data->lpvols->master_gain;


    cps_response_set_result(resp_data,result);

    return TRUE;
}

bool vol_ctlr_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool vol_ctlr_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;

    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool vol_ctlr_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;
    bool retval;

    patch_fn(volume_control_opmsg_obpm_set_params_patch);

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

bool vol_ctlr_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;
    vol_ctrl_gains_t *lpvols = op_extra_data->lpvols;
    unsigned  *resp;

    patch_fn_shared(volume_control_wrapper);

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(VOL_CTRL_STATISTICS),&resp))
    {
        return FALSE;
    }

    if (resp)
    {
        resp = cpsPackWords(&op_extra_data->Ovr_Control,&op_extra_data->post_gain,resp);
        resp = cpsPackWords(&lpvols->master_gain,&lpvols->auxiliary_gain[0] , resp);
        resp = cpsPackWords(&lpvols->auxiliary_gain[1],&lpvols->auxiliary_gain[2] , resp);
        resp = cpsPackWords(&lpvols->auxiliary_gain[3],&lpvols->auxiliary_gain[4] , resp);
        resp = cpsPackWords(&lpvols->auxiliary_gain[5],&lpvols->auxiliary_gain[6] , resp);
        resp = cpsPackWords(&lpvols->auxiliary_gain[7],&lpvols->channel_trims[0] , resp);
        resp = cpsPackWords(&lpvols->channel_trims[1],&lpvols->channel_trims[2] , resp);
        resp = cpsPackWords(&lpvols->channel_trims[3],&lpvols->channel_trims[4] , resp);
        resp = cpsPackWords(&lpvols->channel_trims[5],&lpvols->channel_trims[6] , resp);
        resp = cpsPackWords(&lpvols->channel_trims[7],&op_extra_data->aux_state , resp);
    }

    return TRUE;
}



static bool ups_params_vc(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
        uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return TRUE;
}

bool vol_ctlr_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_data->cap_data->id,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_vc);

    return retval;
}

bool vol_ctlr_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t   *op_extra_data = (vol_ctrl_data_t*)op_data->extra_op_data;

    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,op_data->cap_data->id,message_data,resp_length,resp_data);
}

bool vol_ctlr_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t *op_extra_data = (vol_ctrl_data_t *)(op_data->extra_op_data);

    /* We received client ID, length and then opmsgID and OBPM params */
    op_extra_data->sample_rate = 25 * ((unsigned*)message_data)[3];

    return TRUE;
}

bool vol_ctlr_opmsg_data_stream_based(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    vol_ctrl_data_t *op_extra_data = (vol_ctrl_data_t *)(op_data->extra_op_data);

    /* We received client ID, length and then opmsgID and OBPM params */
    op_extra_data->stream_based = ((unsigned*)message_data)[3];

    return TRUE;
}



