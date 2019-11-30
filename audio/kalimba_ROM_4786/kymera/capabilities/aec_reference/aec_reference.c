/**
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
* \file  aec_reference.c
* \ingroup  capabilities
*
*  AEC Reference
*
*/

/****************************************************************************
Include Files
*/
#include "aec_reference_cap_c.h"

#include "adaptor/adaptor.h"

#include "platform/pl_intrinsics.h"
#include "platform/pl_fractional.h"
#include "platform/pl_interrupt.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "mem_utils/shared_memory_ids.h"
#include "aec_reference_config.h"
#include "op_msg_helpers.h"
#include "opmgr/opmgr_for_ops.h"
#include "patch/patch.h"


/****************************************************************************
Public Type Declarations
*/

/* Period of Timer Task */
#ifndef CHIP_BASE_A7DA_KAS
#define AEC_REFERENCE_TIME_PERIOD   1000
#define AEC_REFERENCE_FRACTIONAL_PERIOD  FRACTIONAL(0.001)
#else
#define AEC_REFERENCE_TIME_PERIOD   900
#define AEC_REFERENCE_FRACTIONAL_PERIOD  FRACTIONAL(0.001)
#endif

#define AECREF_TICKS_PER_SECOND  1000

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define AEC_REFERENCE_CAP_ID CAP_ID_DOWNLOAD_AEC_REFERENCE
#else
#define AEC_REFERENCE_CAP_ID CAP_ID_AEC_REFERENCE
#endif

/****************************************************************************
Private Constant Definitions
*/

/* Message handlers */

/** The aec_reference capability function handler table */
const handler_lookup_struct aec_reference_handler_table =
{
    aec_reference_create,          /* OPCMD_CREATE */
    aec_reference_destroy,         /* OPCMD_DESTROY */
    aec_reference_start,           /* OPCMD_START */
    aec_reference_stop,            /* OPCMD_STOP */
    aec_reference_reset,           /* OPCMD_RESET */
    aec_reference_connect,         /* OPCMD_CONNECT */
    aec_reference_disconnect,      /* OPCMD_DISCONNECT */
    aec_reference_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    aec_reference_get_data_format, /* OPCMD_DATA_FORMAT */
    aec_reference_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */

const opmsg_handler_lookup_table_entry aec_reference_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,           base_op_opmsg_get_capability_version},

    {OPMSG_COMMON_ID_SET_CONTROL,                       aec_reference_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                        aec_reference_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                      aec_reference_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                        aec_reference_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                        aec_reference_opmsg_obpm_get_status},

    {OPMSG_COMMON_GET_CONFIGURATION,                    aec_reference_opmsg_ep_get_config},
    {OPMSG_COMMON_CONFIGURE,                            aec_reference_opmsg_ep_configure},
    {OPMSG_COMMON_GET_CLOCK_ID,                         aec_reference_opmsg_ep_clock_id},

    {OPMSG_AEC_REFERENCE_ID_SET_SAMPLE_RATES,           aec_reference_set_rates},

    {OPMSG_COMMON_ID_SET_UCID,                          aec_reference_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                 aec_reference_opmsg_get_ps_id},

    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                 aec_reference_opmsg_get_ps_id},
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
    {OPMSG_AEC_REFERENCE_ID_ENABLE_SPKR_INPUT_GATE,     aec_reference_opmsg_enable_spkr_ttp_gate},
#endif
    {0, NULL}};


/* Supports up to 8 MICs, 2 SPKRS, and AEC Reference */

const CAPABILITY_DATA aec_reference_cap_data =
    {
        AEC_REFERENCE_CAP_ID,              /* Capability ID */
        AEC_REFERENCE_AECREF_VERSION_MAJOR, AEC_REFERENCE_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        AEC_REF_NUM_SINK_TERMINALS, AEC_REF_NUM_SOURCE_TERMINALS,            /* Max number of sinks/inputs and sources/outputs */
        &aec_reference_handler_table,       /* Pointer to message handler function table */
        aec_reference_opmsg_handler_table,  /* Pointer to operator message handler function table */
        base_op_process_data,               /* Pointer to data processing function */
        0,                                  /* TODO - Processing time information */
        sizeof(AEC_REFERENCE_OP_DATA)       /* Size of capability-specific per-instance data */
    };


/****************************************************************************
Public Function Declarations
*/

#define SIDETONE_ENABLE_FLAG     0x02
#define SIDETONE_MIC_SPKR_FLAG   0x01
#define USE_SIDETONE_FLAG        0x03

/* ********************************** API functions ************************************* */
#if defined(IO_DEBUG)
    aec_latency_common *aec_latency_ptr = NULL;
#endif


bool aec_reference_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    AEC_REFERENCE_OP_DATA *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(aec_reference);

    /* Setup Response to Creation Request.   Assume Failure*/
    *response_id = OPCMD_CREATE;
    if(!base_op_build_std_response(STATUS_CMD_FAILED,op_data->id,response_data))
    {
        return(FALSE);
    }

    /* Processing is in timer task.  This prevents kicks from
       propagating to the main processing function */
    op_data->stop_chain_kicks = TRUE;

    /* Initialize extended data for operator.  Assume intialized to zero*/
    op_extra_data->cap_id = op_data->cap_data->id;
    op_extra_data->ReInitFlag = TRUE;
    op_extra_data->Cur_mode = AEC_REFERENCE_SYSMODE_FULL;
    op_extra_data->kick_id = TIMER_ID_INVALID;

    /* Task Period as fraction - this field is currently a fixed constant,
       but is not declared as such because we may want it to be configurable in the future */

    /* For Atlas this must be less than for equal to the ping/pong period.
       Also set AEC_REFERENCE_TIME_PERIOD
    */
    op_extra_data->task_period = AEC_REFERENCE_FRACTIONAL_PERIOD;
    op_extra_data->mic_rate_ability  = RATEMATCHING_SUPPORT_NONE;
    op_extra_data->spkr_rate_ability = RATEMATCHING_SUPPORT_NONE;
    op_extra_data->mic_shift  = 8;
    op_extra_data->spkr_shift = -8;

    /* ATlas does not use sidetone so don't add it */
#if !defined(CHIP_BASE_A7DA_KAS)
    op_extra_data->using_sidetone = SIDETONE_ENABLE_FLAG;
#else
    op_extra_data->using_sidetone = 0;
#endif

    /* Note:  sample rate config must be sent before the operator's terminals may be connected
       input_rate and output_rate are initialized to zero and checked in the connect operation */

    /* allocate buffer needed for resampler */
    op_extra_data->resampler_temp_buffer = (unsigned*)xzpmalloc(AEC_REF_RESAMPLER_TEMP_BUF_SIZE*sizeof(unsigned));
    if(!op_extra_data->resampler_temp_buffer)
    {
        goto aFailed;
    }

    /*allocate the volume control shared memory */
    op_extra_data->shared_volume_ptr = allocate_shared_volume_cntrl();
    if(!op_extra_data->shared_volume_ptr)
    {
        goto aFailed;
    }

    if(!cpsInitParameters(&op_extra_data->parms_def,(unsigned*)AEC_REFERENCE_GetDefaults(op_extra_data->cap_id),(unsigned*)&op_extra_data->params,sizeof(AEC_REFERENCE_PARAMETERS)))
    {
       goto aFailed;
    }

#if defined(IO_DEBUG)
    aec_latency_ptr = &op_extra_data->sync_block;
#endif

    /* operator state variable set to not running state. based on base_op.c */
    op_data->state = OP_NOT_RUNNING;

    /* chance to fix up */
    patch_fn_shared(aec_reference);

    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
aFailed:
    if(op_extra_data->shared_volume_ptr)
    {
        release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
        op_extra_data->shared_volume_ptr = NULL;
    }
    if(op_extra_data->resampler_temp_buffer)
    {
        pfree(op_extra_data->resampler_temp_buffer);
        op_extra_data->resampler_temp_buffer=NULL;
    }
    return TRUE;
 }


void aec_reference_set_mic_gains(OPERATOR_DATA *op_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*) op_data->extra_op_data;
    unsigned i, *lpadcgains = (unsigned*)&op_extra_data->params.OFFSET_ADC_GAIN1;

    patch_fn_shared(aec_reference);

    for(i=0;i<MAX_NUMBER_MICS;i++)
    {
      if(op_extra_data->input_stream[MicrophoneTerminalByIndex(i)])
        {
            uint32 config_value = (uint32)(lpadcgains[i]);
            set_override_ep_gain(get_overriden_endpoint(op_data->id,MicrophoneTerminalByIndex(i) | TERMINAL_SINK_MASK),config_value);
        }
    }
}

#if defined(IO_DEBUG)
   cbops_op *insert_op  = NULL;
   cbops_op *st_disgard_op = NULL;
#endif

void aec_reference_cleanup(OPERATOR_DATA *op_data)
{
    AEC_REFERENCE_OP_DATA *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    unsigned i;

    patch_fn_shared(aec_reference);

    /* Kill timer Task */
    /* This needs to be atomic, to make sure the right timer gets cancelled */
    LOCK_INTERRUPTS;
    if(op_extra_data->kick_id!=TIMER_ID_INVALID)
    {
        timer_cancel_event(op_extra_data->kick_id);
        op_extra_data->kick_id=TIMER_ID_INVALID;
    }
    UNLOCK_INTERRUPTS;

#if defined(IO_DEBUG)
    insert_op  = NULL;
    st_disgard_op = NULL;
#endif

    /* Clear links to graphs */
    op_extra_data->mic_rate_monitor_op=NULL;
    op_extra_data->spkr_rate_monitor_op=NULL;
    op_extra_data->mic_sidetone_op=NULL;
    op_extra_data->mic_sw_rateadj_op=NULL;
    op_extra_data->spkr_sw_rateadj_op=NULL;
    op_extra_data->channel_status=0;

    /* Free cbops graphs */
    if(op_extra_data->mic_graph)
    {
        destroy_graph(op_extra_data->mic_graph);
        op_extra_data->mic_graph = NULL;
    }
    if(op_extra_data->spkr_graph)
    {
        destroy_graph(op_extra_data->spkr_graph);
        op_extra_data->spkr_graph = NULL;
    }

    /* Free Sidetone buffers */
    if(op_extra_data->sidetone_buf)
    {
        cbuffer_destroy(op_extra_data->sidetone_buf);
        op_extra_data->sidetone_buf = NULL;
    }

    /* Free Internal buffers */
    for(i=0;i<AEC_NUM_SCRATCH_BUFFERS;i++)
    {
        if(op_extra_data->scratch_bufs[i])
        {
            cbuffer_destroy(op_extra_data->scratch_bufs[i]);
            op_extra_data->scratch_bufs[i] = NULL;
        }
    }

    /* TODO - Fill speakers with silence */

}

bool aec_reference_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    AEC_REFERENCE_OP_DATA *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(aec_reference);

    /* Setup Response to Destroy Request.*/
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        return(FALSE);
    }

    /* calling the "destroy" assembly function - this frees up all the capability-internal memory */
    /*free volume control shared memory*/
    release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
    op_extra_data->shared_volume_ptr = NULL;

    /* Make sure everything is cleared */
    aec_reference_cleanup(op_data);

    if(op_extra_data->resampler_temp_buffer)
    {
        pfree(op_extra_data->resampler_temp_buffer);
        op_extra_data->resampler_temp_buffer=NULL;
    }

    base_op_change_response_status(response_data,STATUS_OK);
    return(TRUE);
}

bool build_mic_graph(AEC_REFERENCE_OP_DATA *op_extra_data)
{
    cbops_graph *mic_graph;
    cbops_op *op_ptr;
    unsigned i,num_mics = op_extra_data->num_mic_channels,j,num_io;
    unsigned* idxs;
    unsigned  out_idx,intern_idx;
    unsigned  st_idx=0,st_rs_idx=0;
    
    patch_fn_shared(aec_reference);

    /*********************************************************************

    MICs --> RM --> DC_REMOVE -+--> resample --> OUT
                               |
                            ST filter
                               |
                             resample
                               |
                             SIDETONE

    **********************************************************************/

    /* Is there a MIC path */
    if(!(op_extra_data->channel_status&CHAN_STATE_MIC_MASK))
    {
        return(TRUE);
    }

    /* number of io buffers
      num_mic*[IN,INTERN,OUT] plus SideTONE

      Buffer Order
         MIC Inputs
         MIC Output
         MIC Internal
         SIDETONE OUT
    */

    out_idx    = num_mics;
    intern_idx = out_idx;
    num_io     = 2*num_mics;

    if(op_extra_data->using_sidetone==USE_SIDETONE_FLAG)
    {
       /* Need internal buffer between input and output */
       intern_idx = num_io;
       num_io    += num_mics;

       /* sidetone output */
       st_idx        = num_io;
       num_io++;

       st_rs_idx = st_idx;
       if(op_extra_data->mic_rate!=op_extra_data->spkr_rate)
       {
           /* Sidetone is inplace */
           st_rs_idx = intern_idx;
       }
    }
    else if( (op_extra_data->mic_rate_ability==RATEMATCHING_SUPPORT_SW) &&
             (op_extra_data->mic_rate!=op_extra_data->output_rate))
    {
       /* Need internal buffer between input and output */
       intern_idx = num_io;
       num_io    += num_mics;
    }
    else if (op_extra_data->mic_rate!=op_extra_data->output_rate)
    {
       /* No ratematching or sidetone.  Just resampling  */
       intern_idx = 0;
    }

    idxs = create_default_indexes(num_io);
    if(idxs == NULL)
    {
        return(FALSE);
    }

    /* Allocate mic graph */
    mic_graph = cbops_alloc_graph(num_io);
    if(!mic_graph)
    {
        goto aFailed;
    }
    op_extra_data->mic_graph = mic_graph;

    /* Setup IO Buffers*/
    for(j=0,i=0;(j<MAX_NUMBER_MICS);j++)
    {
        tCbuffer    *buffer_ptr_src,*buffer_ptr_snk;
        /* MICs may not be consecutive */
        buffer_ptr_src = op_extra_data->input_stream[MicrophoneTerminalByIndex(j)];
        buffer_ptr_snk = op_extra_data->output_stream[OutputTerminalByIndex(j)];
        if(buffer_ptr_src && buffer_ptr_snk && (i < num_mics))
        {
           /* Inputs */
           cbops_set_input_io_buffer(mic_graph,i,0,buffer_ptr_src);
           /* Outputs */
           cbops_set_output_io_buffer(mic_graph,out_idx+i,out_idx,buffer_ptr_snk);
           i++;
        }
    }

    if(intern_idx > out_idx)
    {
        for(i=0;i<num_mics;i++)
        {
             cbops_set_internal_io_buffer(mic_graph,intern_idx+i,intern_idx,op_extra_data->scratch_bufs[i]);
        }
    }

     op_extra_data->mic_rate_adjustment = 0;
#ifndef CHIP_BASE_A7DA_KAS
     /* Monitor rate */
     op_ptr = create_rate_monitor_op(AECREF_TICKS_PER_SECOND,0);

     if(!op_ptr)
     {
         goto aFailed;
     }

     if(op_extra_data->mic_rate_ability==RATEMATCHING_SUPPORT_HW)
     {
        rate_monitor_op_initiatlize(op_ptr,op_extra_data->mic_rate,TRUE,3*MS_PER_SEC);
     }
     else
     {
        rate_monitor_op_initiatlize(op_ptr,op_extra_data->mic_rate,FALSE,5*MS_PER_SEC);
     }

     op_extra_data->mic_rate_monitor_op = op_ptr;

     cbops_append_operator_to_graph(mic_graph,op_ptr);
#endif


    /* Handle input */
    if(op_extra_data->mic_rate_ability==RATEMATCHING_SUPPORT_SW)
    {
        /* Apply Software Rate Adjustment */
        op_ptr = create_sw_rate_adj_op(num_mics, idxs, &idxs[intern_idx],
                                       CBOPS_RATEADJUST_COEFFS,
                                       &op_extra_data->mic_rate_adjustment, op_extra_data->mic_shift);
        if(!op_ptr)
        {
           goto aFailed;
        }

        op_extra_data->mic_sw_rateadj_op=op_ptr;
        cbops_rateadjust_passthrough_mode(op_ptr,(op_extra_data->mic_rate_enactment==RATEMATCHING_SUPPORT_NONE)?TRUE:FALSE);

        cbops_append_operator_to_graph(mic_graph,op_ptr);

        /* Early DC remove on mic path. Before Sidetone split so the signal split
          to the speaker doesn't have DC */
        op_ptr = create_dc_remove_op(num_mics, &idxs[intern_idx], &idxs[intern_idx]);
        if(!op_ptr)
        {
           goto aFailed;
        }
        cbops_append_operator_to_graph(mic_graph,op_ptr);

    }
    else if(intern_idx != 0)
    {
        /* Otherwise, just copy data to next section */
        op_ptr = create_shift_op(num_mics, idxs, &idxs[intern_idx], op_extra_data->mic_shift);
        if(!op_ptr)
        {
           goto aFailed;
        }
        cbops_append_operator_to_graph(mic_graph,op_ptr);

        /* Early DC remove on mic path. Before Sidetone split so the signal split
          to the speaker doesn't have DC */
        op_ptr = create_dc_remove_op(num_mics, &idxs[intern_idx], &idxs[intern_idx]);
        if(!op_ptr)
        {
           goto aFailed;
        }
        cbops_append_operator_to_graph(mic_graph,op_ptr);
    }

    /* Handle output */
    if(op_extra_data->mic_rate!=op_extra_data->output_rate)
    {
         /* If only operator then shift input to output */
         unsigned shift_amount = (intern_idx != 0) ? 0 :  op_extra_data->mic_shift;

        /*  Add Resampler Operators per channel (mic_rate --> output_rate) */
         op_ptr = create_iir_resamplerv2_op(num_mics, &idxs[intern_idx], &idxs[out_idx],
                                       op_extra_data->mic_rate,op_extra_data->output_rate,
                                       AEC_REF_RESAMPLER_TEMP_BUF_SIZE,
                                       op_extra_data->resampler_temp_buffer, shift_amount, 0, 0);
         if(!op_ptr)
         {
             goto aFailed;
         }
         cbops_append_operator_to_graph(mic_graph,op_ptr);
    }
    else if (st_idx != 0)
    {
         /* Need to copy internal buffer to output before sidetone */
        op_ptr = create_shift_op(num_mics,&idxs[intern_idx], &idxs[out_idx],0);
        if(!op_ptr)
        {
           goto aFailed;
        }
        cbops_append_operator_to_graph(mic_graph,op_ptr);
    }

    /* Handle sidetone */
    if(st_idx != 0)
    {
         /* Minimum space needed in buffer */
         unsigned threshold = frac_mult(op_extra_data->spkr_rate,op_extra_data->task_period)+1;

         cbops_set_output_io_buffer(mic_graph,st_idx,st_idx,op_extra_data->sidetone_buf);

         /*  Add Sidetone operators:
            Sidetone Apply Operator
            Note:  Sidetone is before resampler.
            Better solution is to place it at lowest sample rate */

         op_ptr = create_sidetone_filter_op(intern_idx, st_rs_idx, 3,
                                           (cbops_sidetone_params*)&op_extra_data->params.OFFSET_ST_CLIP_POINT,
                                           (void*)&op_extra_data->params.OFFSET_ST_PEQ_CONFIG);

         op_extra_data->mic_sidetone_op = op_ptr;
         if(!op_ptr)
         {
              goto aFailed;
         }
         cbops_append_operator_to_graph(mic_graph,op_ptr);

         if(st_rs_idx!=st_idx)
         {
            // create resampler only for one in & out channel
            op_ptr = create_iir_resamplerv2_op(1, &idxs[st_rs_idx], &idxs[st_idx], op_extra_data->mic_rate, op_extra_data->spkr_rate,
            AEC_REF_RESAMPLER_TEMP_BUF_SIZE, op_extra_data->resampler_temp_buffer, 0, 0, 0);
            if(!op_ptr)
            {
               goto aFailed;
            }
            cbops_append_operator_to_graph(mic_graph,op_ptr);
         }

         /* Add in disgard on sidetone */

         op_ptr = create_sink_overflow_disgard_op(1,&idxs[st_idx],threshold);
#if defined(IO_DEBUG)
         st_disgard_op = op_ptr;
#endif
         if(!op_ptr)
         {
            goto aFailed;
         }
         cbops_append_operator_to_graph(mic_graph,op_ptr);
    }

    /* Insert latency operator */
    op_ptr = create_mic_latency_op(out_idx,&op_extra_data->sync_block);
    if(!op_ptr)
    {
       goto aFailed;
    }
    cbops_append_operator_to_graph(mic_graph,op_ptr);

    pfree(idxs);

    //}


    /* Each mic input has a corresponding output,
     * so it's safe to purge all channels now
     */
    aec_ref_purge_mics(mic_graph,num_mics);
        
    return(TRUE);
aFailed:
    pfree(idxs);
    return(FALSE);
}


bool build_spkr_graph(AEC_REFERENCE_OP_DATA *op_extra_data)
{
    unsigned    i,j,k,num_spkrs,num_inputs;
    cbops_graph *spkr_graph;
    cbops_op    *op_ptr;
    unsigned    num_io,in_threshold,out_threshold;
    tCbuffer    *buffer_ptr;
    unsigned    *idxs;
    unsigned    out_indx,intern_ins_idx,intern_rs_idx,intern_rm_idx;
    unsigned    ref_idx,st_in_idx;
    
    patch_fn_shared(aec_reference);

    /*********************************************************************

                                           RM ---> resample --> REF
                                             |
    IN --> INSERT --> DC_REMOVE -->  MIXER --+--> resample --> Sidetone Mix --> RM ----------> SPKR
   (0)                                   (intern_ins_idx)    (intern_rs_idx)  (intern_rm_idx)  (out_indx)
    **********************************************************************/
    /* Is there a Speaker path */
    if(!(op_extra_data->channel_status&CHAN_STATE_SPKR_MASK))
    {
        return(TRUE);
    }

    /* Determine IO configuration */
    num_spkrs  = op_extra_data->num_spkr_channels;
    num_inputs = op_extra_data->num_spkr_channels;
    if(!(op_extra_data->channel_status&AEC_REFERENCE_CONSTANT_CONN_TYPE_PARA))
    {
        if(op_extra_data->channel_status&AEC_REFERENCE_CONSTANT_CONN_TYPE_MIX)
        {
            /* Multiple inputs, mono output */
            num_spkrs=1;
        }
        else
        {
            /* Mono input, multiple outputs */
            num_inputs=1;
        }
    }

    /* outputs follow inputs */
    /* Internal buffers for output of insert follow outputs */
    out_indx       = num_inputs;

    /* Count the buffers needed */
    num_io         = num_inputs+num_spkrs;

    /* Assume no resampler or rate matching */
    intern_rs_idx = 0;
    intern_rm_idx = 0;
    intern_ins_idx = 0;

    /* Check for sidetone */
    st_in_idx = 0;
    if(op_extra_data->using_sidetone==USE_SIDETONE_FLAG)
    {
        st_in_idx = num_io;
        num_io++;
    }

    /* Check if resampler is required */
    if(op_extra_data->input_rate!=op_extra_data->spkr_rate)
    {
        intern_rs_idx = out_indx;

        if( (st_in_idx!=0) || (op_extra_data->spkr_rate_ability==RATEMATCHING_SUPPORT_SW) )
        {
            /* Need buffers following resampler */
            intern_rs_idx    = num_io;
            if(num_spkrs>num_inputs)
            {
                /* Resampler is Mono.  Feeds shift */
                num_io++;
            }
            else
            {
                /* Resampler is multi channel */
                num_io += num_inputs;
            }
        }
    }

    /* Check if rate matching is required */
    if(op_extra_data->spkr_rate_ability==RATEMATCHING_SUPPORT_SW)
    {
        intern_rm_idx = out_indx;
        /* Split needs buffer following rate matching (Mono)*/
        if(num_spkrs>num_inputs)
        {
            intern_rm_idx = num_io;
            num_io++;
        }
    }

    ref_idx  = num_io;
    if(op_extra_data->channel_status&AEC_REFERENCE_CONSTANT_CONN_TYPE_REF)
    {
        num_io++;
        if(op_extra_data->input_rate!=op_extra_data->output_rate)
        {
            /* Need buffer following rate matching */
            num_io++;
        }
    }

    /* Allocate buffer index array for easy setup */
    if(num_inputs<num_spkrs)
    {
       idxs = create_default_indexes(num_io+num_spkrs);
    }
    else
    {
       idxs = create_default_indexes(num_io);
    }
    if(idxs == NULL)
    {
        return(FALSE);
    }

    /* Allocate num_io buffers in spkr graph */
    spkr_graph = cbops_alloc_graph(num_io);
    if(!spkr_graph)
    {
        goto aFailed;
    }
    op_extra_data->spkr_graph = spkr_graph;

   /* Setup IO Buffers
      Buffer Order
         Inputs
         SPKR Outputs
         Internal (after insert)          : scratch[0:num_inputs-1]
         Internal (after resample)
         Internal (after rate match)
         Internal (after ref resample)    : scratch[num_inputs]
         REF OUT
   */

    /* Input, Output, and insert_op buffers */
    for(j=0,i=0,k=0;j<MAX_NUMBER_SPEAKERS;j++)
    {
        /* Inputs may not be contiguous */
        buffer_ptr =  op_extra_data->input_stream[SpeakerInputTerminalByIndex(j)];
        if(buffer_ptr)
        {
            /* Inputs */
            cbops_set_input_io_buffer(spkr_graph,i,0,buffer_ptr);
            i++;
        }
        /* Outputs may not be contiguous */
        buffer_ptr =  op_extra_data->output_stream[SpeakerTerminalByIndex(j)];
        if(buffer_ptr)
        {
            cbops_set_output_io_buffer(spkr_graph,out_indx + k,out_indx,buffer_ptr);
            k++;
        }
    }

    /* DC remove before reference tap */
    op_ptr = create_dc_remove_op(num_inputs, idxs, idxs);
    if(!op_ptr)
    {
        goto aFailed;
    }
    cbops_append_operator_to_graph(spkr_graph,op_ptr);

   /* Threshold for Insertion */
   in_threshold  = frac_mult(op_extra_data->input_rate,op_extra_data->task_period)+1;
   out_threshold = frac_mult(op_extra_data->spkr_rate,op_extra_data->task_period)+1;

   op_ptr = create_aec_ref_spkr_op(num_inputs,idxs,in_threshold,num_spkrs,&idxs[out_indx],out_threshold);
#if defined(IO_DEBUG)
   insert_op  = op_ptr;
#endif
   if(!op_ptr)
   {
        goto aFailed;
   }

   cbops_set_override_operator(spkr_graph,op_ptr);

    /* Add Mixer to section #1 if needed*/
    if(num_inputs > num_spkrs)
    {
        for(i=1;i<num_inputs;i++)
        {
            /* NOTE: left scratch input reused for output (inplace) */
            /*  - Add Mix Operator - stereo to mono (left = (left+right)/2 */
            op_ptr = create_mixer_op(intern_ins_idx, intern_ins_idx+i, intern_ins_idx, 0, FRACTIONAL(0.5));
            if(!op_ptr)
            {
                goto aFailed;
            }
            cbops_append_operator_to_graph(spkr_graph,op_ptr);
        }
        /* Input is now mono */
        num_inputs = 1;
    }

    /* Optional Section for Reference copy */
    if(op_extra_data->channel_status&AEC_REFERENCE_CONSTANT_CONN_TYPE_REF)
    {
        unsigned ref_rm_in_idx = intern_ins_idx;

        cbops_set_output_io_buffer(spkr_graph,ref_idx,ref_idx,op_extra_data->output_stream[AEC_REF_REFERENCE_TERMINAL]);

        if(op_extra_data->input_rate != op_extra_data->output_rate)
        {
            /* resampler outputs into internal buffer */
            ref_rm_in_idx=ref_idx+1;

            /* NOTE: left scratch input is input to resampler */
            cbops_set_internal_io_buffer(spkr_graph,ref_rm_in_idx,ref_rm_in_idx,op_extra_data->scratch_bufs[num_inputs]);

            /* Add reference sample rate conversion (input_rate --> output_rate) - for 1 channel only */
            op_ptr = create_iir_resamplerv2_op(1, &idxs[intern_ins_idx], &idxs[ref_rm_in_idx],
                                           op_extra_data->input_rate, op_extra_data->output_rate,
                                           AEC_REF_RESAMPLER_TEMP_BUF_SIZE,
                                           op_extra_data->resampler_temp_buffer, 0, 0, 0);

            if(!op_ptr)
            {
                goto aFailed;
            }
            cbops_append_operator_to_graph(spkr_graph,op_ptr);
        }

        /*  SW rate adjustment for reference */
        op_ptr = create_sw_rate_adj_op(1, &idxs[ref_rm_in_idx], &idxs[ref_idx],
                                       CBOPS_RATEADJUST_COEFFS,
                                       &op_extra_data->sync_block.rm_adjustment, 0);
        if(!op_ptr)
        {
            goto aFailed;
        }
        cbops_append_operator_to_graph(spkr_graph,op_ptr);

        /* speaker latency cbops operator */
		op_ptr = create_speaker_latency_op(idxs[ref_idx],&op_extra_data->sync_block);

        if(!op_ptr)
        {
            goto aFailed;
        }
        cbops_append_operator_to_graph(spkr_graph,op_ptr);
    }

    /* Add sample rate conversion per channel [num_inputs] (input_rate --> spkr_rate) */
    if(intern_rs_idx!=0)
    {
        int shift_amount = op_extra_data->spkr_shift;

        if(intern_rs_idx!=out_indx)
        {
            /* Buffers for output of resampler. */
            for(i=0; i < num_inputs ; i++)
            {
                 cbops_set_internal_io_buffer(spkr_graph,intern_rs_idx+i,intern_rs_idx,op_extra_data->scratch_bufs[num_inputs+i]);
            }
            shift_amount=0;
        }

        op_ptr = create_iir_resamplerv2_op(num_inputs, &idxs[intern_ins_idx], &idxs[intern_rs_idx],
                                           op_extra_data->input_rate, op_extra_data->spkr_rate,
                                           AEC_REF_RESAMPLER_TEMP_BUF_SIZE,
                                           op_extra_data->resampler_temp_buffer, shift_amount, 0, 0);
        if(!op_ptr)
        {
            goto aFailed;
        }
        cbops_append_operator_to_graph(spkr_graph,op_ptr);

        /* Move next input to output of resampler */
        intern_ins_idx = intern_rs_idx;
    }

    if(st_in_idx!=0)
    {
        /* Add sidetone mixer to resampler section,  Master channel only */

        cbops_set_input_io_buffer(spkr_graph,st_in_idx,st_in_idx,op_extra_data->sidetone_buf);

        /* Add sidetone mix operator [left input] (inplace, so pass it num_inputs as in & out index);
         * it is as per multi-channel model, but only ever works on one single channel
         */
        op_ptr = create_sidetone_mix_op(idxs[intern_ins_idx],idxs[intern_ins_idx],idxs[st_in_idx], out_threshold);
        if(!op_ptr)
        {
            goto aFailed;
        }
        cbops_append_operator_to_graph(spkr_graph,op_ptr);
    }

    /* Optional section for SW rate matching */
    if(intern_rm_idx != 0)
    {
        int shift_amount =  op_extra_data->spkr_shift;

        if(intern_rm_idx!=out_indx)
        {
            /* Use alternate set of scratch buffers from resampler */
            unsigned scratch_offset = (intern_rs_idx!=0)? 0 : num_inputs;

            /* Buffers for output of ratematch if it exists */
            for(i=0; i < num_inputs; i++)
            {
                 cbops_set_internal_io_buffer(spkr_graph,intern_rm_idx+i,intern_rm_idx,op_extra_data->scratch_bufs[scratch_offset+i]);
            }
            shift_amount=0;
        }

        /*  SW rate adjustment per channel [num_inputs] */
        op_ptr = create_sw_rate_adj_op(num_inputs, &idxs[intern_ins_idx], &idxs[intern_rm_idx],
                                       CBOPS_RATEADJUST_COEFFS,
                                       &op_extra_data->spkr_rate_adjustment,shift_amount);
        if(!op_ptr)
        {
            goto aFailed;
        }

        op_extra_data->spkr_sw_rateadj_op=op_ptr;
        cbops_rateadjust_passthrough_mode(op_ptr,(op_extra_data->spkr_rate_enactment==RATEMATCHING_SUPPORT_NONE)?TRUE:FALSE);

        cbops_append_operator_to_graph(spkr_graph,op_ptr);

        /* Move next input to output of ratematch */
        intern_ins_idx = intern_rm_idx;
    }


    /* Did previous operator terminate route? */
    if(intern_ins_idx!=out_indx)
    {
        /* Check for Mono to multi-channel */
        if(num_inputs<num_spkrs)
        {
            for(i=0; i < num_spkrs; i++)
            {
               idxs[num_io+i] = intern_ins_idx;
            }
            intern_ins_idx = num_io;
        }

        op_ptr = create_shift_op(num_spkrs, &idxs[intern_ins_idx], &idxs[out_indx], op_extra_data->spkr_shift);
        if(!op_ptr)
        {
            goto aFailed;
        }
        cbops_append_operator_to_graph(spkr_graph,op_ptr);
    }

    op_extra_data->spkr_rate_adjustment=0;

/* Atlas is going to transfer the DAC SW buffers into a ping/pong
   DRAM buffer at the ping/pong rate.   There is no ability
   to control the latency.  That is handled by the size of the
   ping/pong buffer */
#ifndef CHIP_BASE_A7DA_KAS
    /* Create Rate monitoring operator */
    op_ptr = create_rate_monitor_op(AECREF_TICKS_PER_SECOND,out_indx);
    if(!op_ptr)
    {
        goto aFailed;
    }

    if(op_extra_data->spkr_rate_ability==RATEMATCHING_SUPPORT_HW)
    {
        rate_monitor_op_initiatlize(op_ptr,op_extra_data->spkr_rate,TRUE,3*MS_PER_SEC);
    }
    else
    {
        rate_monitor_op_initiatlize(op_ptr,op_extra_data->spkr_rate,FALSE,5*MS_PER_SEC);
    }

    op_extra_data->spkr_rate_monitor_op=op_ptr;

    cbops_append_operator_to_graph(spkr_graph,op_ptr);
#endif

    pfree(idxs);
    return TRUE;
aFailed:
    pfree(idxs);
    return FALSE;
}





bool validate_channels_and_build(OPERATOR_DATA *op_data)
{
    AEC_REFERENCE_OP_DATA *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    unsigned i;
    unsigned channel_status=0;
    unsigned number_mics=0;
    unsigned number_spkrs=0;
    tCbuffer *inputBuf,*outputBuf;
    unsigned size = 100;    /* TODO - need to compute size of scratch buffers */
    
    patch_fn_shared(aec_reference);

    /* Microphones are connected */
    op_extra_data->source_kicks = 0;
    if(op_extra_data->input_stream[AEC_REF_MIC_TERMINAL1] && op_extra_data->output_stream[AEC_REF_OUTPUT_TERMINAL1])
    {
        bool all_mic_channels = TRUE;
        
        for(i=0;i<MAX_NUMBER_MICS;i++)
        {
            inputBuf  = op_extra_data->input_stream[MicrophoneTerminalByIndex(i)];
            outputBuf = op_extra_data->output_stream[OutputTerminalByIndex(i)];

            if(inputBuf && outputBuf)
            {
                number_mics++;

                if(i<4)
                {
                    channel_status |= (AEC_REFERENCE_CONSTANT_CONN_MIKE_1<<i);
                }
                else
                {
                    channel_status |= ((AEC_REFERENCE_CONSTANT_CONN_MIKE_5>>4)<<i);
                }
                op_extra_data->source_kicks |= (1 << OutputTerminalByIndex(i) );
            }
            else if(inputBuf || outputBuf)
            {
               all_mic_channels=FALSE;
               break;
            }
        }
        if(!all_mic_channels)
        {
           number_mics=0;
           channel_status &= ~CHAN_STATE_MIC_MASK;
           op_extra_data->source_kicks = 0;
        }
    }

    /* Speakers are connected */
    op_extra_data->sink_kicks   = 0;
    if(op_extra_data->input_stream[AEC_REF_INPUT_TERMINAL1] && op_extra_data->output_stream[AEC_REF_SPKR_TERMINAL1])
    {
        int in_count=1;
        int out_count=1;

        /* Master Channel is connected */
        channel_status |= AEC_REFERENCE_CONSTANT_CONN_SPKR_1;
        /* Primary sink is connected */
        op_extra_data->sink_kicks = (1<<AEC_REF_INPUT_TERMINAL1);

        for(i=1;i<MAX_NUMBER_SPEAKERS;i++)
        {
            int sink_idx = SpeakerInputTerminalByIndex(i);

            /* Old terminals kept for backwards compatibility */
            outputBuf = op_extra_data->output_stream[SpeakerTerminalByIndex(i)];
            inputBuf  = op_extra_data->input_stream[sink_idx];

            if(inputBuf || outputBuf)
            {
                channel_status |= (AEC_REFERENCE_CONSTANT_CONN_SPKR_1<<i);

                if(inputBuf)
                {
                    op_extra_data->sink_kicks |= (1<<sink_idx);
                    in_count++;
                }
                else if(in_count>1)
                {
                    /* Special case allows mono input split to multiple outputs*/
                    return(FALSE);
                }

                if(outputBuf)
                {
                    out_count++;
                }
                else if(out_count>1)
                {
                    /* Special case allows mono output mixed from multiple inputs*/
                    return(FALSE);
                }
            }
        }
        /* Verify complete channel setup */
        number_spkrs = out_count;
        if(in_count==out_count)
        {
            /* Parallel channels */
            channel_status |= AEC_REFERENCE_CONSTANT_CONN_TYPE_PARA;
        }
        else if(out_count==1)
        {
            /* Mono Output.  Mix inputs */
            channel_status |= AEC_REFERENCE_CONSTANT_CONN_TYPE_MIX;
            number_spkrs = in_count;
        }
        else if(in_count!=1)
        {
            /* Mono Input.  Split outputs */
            return(FALSE);
        }


        /* Check AEC reference */
        if(op_extra_data->output_stream[AEC_REF_REFERENCE_TERMINAL] && (channel_status&AEC_REFERENCE_CONSTANT_CONN_MIKE_1))
        {
            op_extra_data->source_kicks |= 1<<AEC_REF_REFERENCE_TERMINAL;
            channel_status |= AEC_REFERENCE_CONSTANT_CONN_TYPE_REF;
        }
    }

    patch_fn_shared(aec_reference);

    /* Check if the configuration has not changed */
    if(channel_status==op_extra_data->channel_status)
    {
        return (TRUE);
    }

    /* Just to make sure we are starting clean */
    aec_reference_cleanup(op_data);

    // number_spkrs
    op_extra_data->channel_status = channel_status;
    op_extra_data->num_spkr_channels = number_spkrs;
    op_extra_data->num_mic_channels = number_mics;

    if( (number_spkrs>0) && (number_mics>0) )
    {
         op_extra_data->using_sidetone |= SIDETONE_MIC_SPKR_FLAG;
    }
    else
    {
         op_extra_data->using_sidetone &= SIDETONE_ENABLE_FLAG;
    }

    if(op_extra_data->using_sidetone==USE_SIDETONE_FLAG)
    {
        /* Allocate Buffer between cbops Graphs */
        op_extra_data->sidetone_buf = cbuffer_create_with_malloc_fast(3*frac_mult(op_extra_data->spkr_rate , AEC_REFERENCE_FRACTIONAL_PERIOD), BUF_DESC_SW_BUFFER);
        if(!op_extra_data->sidetone_buf)
        {
           goto aFailed;
        }
    }

    /* Allocate scratch buffers  */
    if(number_spkrs<number_mics)
    {
        number_spkrs = number_mics;
    }
    for(i=0;i<(number_spkrs<<1);i++)
    {
        op_extra_data->scratch_bufs[i] = cbuffer_create_with_malloc_fast(size, BUF_DESC_SW_BUFFER);
        if(!op_extra_data->scratch_bufs[i])
        {
            goto aFailed;
        }
    }

    /** Setup Latency Control */
    {
    	unsigned jitter = frac_mult(op_extra_data->output_rate,op_extra_data->task_period);

	    op_extra_data->sync_block.jitter     = 	jitter + (jitter>>1);				/* 1.5 times task period */
	    op_extra_data->sync_block.ref_delay  = frac_mult(op_extra_data->output_rate,FRACTIONAL(0.001)); 	 /* 1.0 msec */
	    op_extra_data->sync_block.block_sync = 0;

	    op_extra_data->sync_block.rm_adjustment = 0;
        op_extra_data->sync_block.frm_count = 0;
   }

    /******** Setup MIC cbops graph *******/
    /*  Assume all MICs are likewise clocked and rate matched with same sample rate */

    if(!(build_mic_graph(op_extra_data)))
    {
        goto aFailed;
    }

    /******** Setup SPKR cbops chain *******/

    if(!(build_spkr_graph(op_extra_data)))
    {
        goto aFailed;
    }

#ifdef CHIP_BASE_A7DA_KAS
   interrupt_block();
    /* Make sure the output is purged before starting graphs.  This assists latency/wrap protection  */
    /* Block interrupts because there could be an interrupt between clearing of two channels
       which may cause channels to go out of phase */
    for(i=0;i<MAX_NUMBER_SPEAKERS;i++)
    {
        outputBuf = op_extra_data->output_stream[SpeakerTerminalByIndex(i)];
        if(outputBuf)
        {
            cbuffer_move_write_to_read_point(outputBuf,0);
        }
    }
   interrupt_unblock();
#endif

    /*  Start Timer Task */
    op_extra_data->ReInitFlag = TRUE;
    op_extra_data->kick_id = timer_schedule_event_in(AEC_REFERENCE_TIME_PERIOD,aec_reference_timer_task,(void*)op_data);

    return(TRUE);
aFailed:
    aec_reference_cleanup(op_data);
    return(FALSE);
}

bool aec_reference_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    patch_fn_shared(aec_reference);

    /* Setup Response to Start Request.   Assume Failure*/
    *response_id = OPCMD_START;
    if(!base_op_build_std_response(STATUS_CMD_FAILED,op_data->id,response_data))
    {
        return(FALSE);
    }
    /* do something only if the current state is "connected" */
    if(op_data->state == OP_RUNNING)
    {
        base_op_change_response_status(response_data,STATUS_OK);
        return(TRUE);
    }

    /* Validate channel configuration */
    if(!validate_channels_and_build(op_data))
    {
        return(TRUE);
    }
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
    {
        AEC_REFERENCE_OP_DATA *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
        if(op_extra_data->spkr_ttp_gate_enable)
        {
            /* initialisation for ttp gate feature */
            aec_reference_spkr_ttp_gate_init(op_extra_data);
        }
    }
#endif /* AEC_REFERENCE_SPKR_TTP_GATE */
    /* set internal capability state variable to "running" */
    /* operator state variable set to not running state. based on base_op.c */
    op_data->state = OP_RUNNING;

    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
}

bool aec_reference_stop_reset(OPERATOR_DATA *op_data,void **response_data)
{
    patch_fn_shared(aec_reference);

    if(!base_op_build_std_response(STATUS_CMD_FAILED,op_data->id,response_data))
    {
        return(FALSE);
    }
    /* do something only if the current state is "running" */
    if(op_data->state == OP_RUNNING)
    {
        aec_reference_cleanup(op_data);
    }

    /* operator state variable set to not running state. based on base_op.c */
    op_data->state = OP_NOT_RUNNING;
    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
}

bool aec_reference_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* Setup Response to Stop Request.   Assume Failure*/
    *response_id = OPCMD_STOP;
    return(aec_reference_stop_reset(op_data,response_data));
}

bool aec_reference_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* Setup Response to Reset Request.   Assume Failure*/
    *response_id = OPCMD_RESET;
    return(aec_reference_stop_reset(op_data,response_data));
}


bool aec_reference_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    unsigned terminal_id    = ((unsigned*)message_data)[0];    /* extract the terminal_id */
    tCbuffer* pterminal_buf = (tCbuffer*)(uintptr_t)(((unsigned *)message_data)[1]);

    patch_fn_shared(aec_reference);

    L3_DBG_MSG1("aec_reference connect TID = %x", terminal_id);

    /* Setup Response to Connection Request.   Assume Failure*/
    *response_id = OPCMD_CONNECT;
    if(!base_op_build_std_response(STATUS_CMD_FAILED,op_data->id,response_data))
    {
        return(FALSE);
    }

    /* Only allow connection if operator has been configured */
    if((op_extra_data->input_rate==0) || (op_extra_data->output_rate==0) )
    {
        return(TRUE);
    }

    /* (i)  check if the terminal ID is valid . The number has to be less than the maximum number of sinks or sources .  */
    /* (ii) check if we are connecting to the right type . It has to be a buffer pointer and not endpoint connection */
    if( !base_op_is_terminal_valid(op_data, terminal_id) || !pterminal_buf)
    {
        base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* check if the terminal is already connected and if not , connect the terminal */
    if(terminal_id & TERMINAL_SINK_MASK)
    {
        terminal_id &= ~ TERMINAL_SINK_MASK;

        if(terminal_id==AEC_REF_MIC_TERMINAL1)
        {
            uint32 config_value;

            /* get info about overridden endpoints */
            op_extra_data->mic_endpoint = get_overriden_endpoint(op_data->id,AEC_REF_MIC_TERMINAL1 | TERMINAL_SINK_MASK);

            if(!get_override_ep_ratematch_ability(op_extra_data->mic_endpoint,&config_value))
            {
                /* Should never fail */
                base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
                return TRUE;
            }
#if defined(ENABLE_FORCE_SW_RATEMATCH)
            /* Save rate matching ability HW or SW*/
            op_extra_data->mic_rate_ability   = RATEMATCHING_SUPPORT_SW;
            op_extra_data->mic_rate_enactment = RATEMATCHING_SUPPORT_SW;
#else
            /* Save rate matching ability HW or SW*/
            op_extra_data->mic_rate_ability   = (unsigned)config_value;
            op_extra_data->mic_rate_enactment = RATEMATCHING_SUPPORT_NONE;
#endif

            if(!get_override_ep_sample_rate(op_extra_data->mic_endpoint,&config_value))
            {
                /* Should never fail */
                base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
                return TRUE;
            }
            op_extra_data->mic_rate = (unsigned)config_value;

        }
        else if(terminal_id==AEC_REF_INPUT_TERMINAL1)
        {
            op_extra_data->spkr_in_endpoint = get_overriden_endpoint(op_data->id,AEC_REF_INPUT_TERMINAL1 | TERMINAL_SINK_MASK);
        }

        if(op_extra_data->input_stream[terminal_id])
        {
            /* Should never try to connect an already connected terminal*/
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
        else
        {
            op_extra_data->input_stream[terminal_id] = pterminal_buf;
        }
#ifdef AEC_REFERENCE_SUPPORT_METADATA
        /* Metadata might be enabled for speaker graph inputs
         * in multi input cases we expect all use same metadata
         * buffer.
         */
        if(IsSpeakerInputTerminal(terminal_id) &&
           buff_has_metadata(pterminal_buf))
        {
            /* set metadata buffer if it hasn't been set already */
            if(NULL == op_extra_data->spkr_input_metadata_buffer)
            {
                op_extra_data->spkr_input_metadata_buffer = pterminal_buf;
            }
        }
#endif /* AEC_REFERENCE_SUPPORT_METADATA */
    }
    else
    {
        if(terminal_id==AEC_REF_SPKR_TERMINAL1)
        {
            uint32 config_value;

            op_extra_data->spkr_endpoint = get_overriden_endpoint(op_data->id,AEC_REF_SPKR_TERMINAL1);

            if(!get_override_ep_ratematch_ability(op_extra_data->spkr_endpoint,&config_value))
            {
                /* Should never fail */
                base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
                return TRUE;
            }

#if defined(ENABLE_FORCE_SW_RATEMATCH)
            op_extra_data->spkr_rate_ability   = RATEMATCHING_SUPPORT_SW;
            op_extra_data->spkr_rate_enactment = RATEMATCHING_SUPPORT_SW;
#else
            op_extra_data->spkr_rate_ability   = (unsigned)config_value;
            op_extra_data->spkr_rate_enactment = RATEMATCHING_SUPPORT_NONE;
#endif

            if(!get_override_ep_sample_rate(op_extra_data->spkr_endpoint,&config_value))
            {
                /* Should never fail */
                base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
                return TRUE;
            }
            op_extra_data->spkr_rate = (unsigned)config_value;

        }
        else if (terminal_id==AEC_REF_OUTPUT_TERMINAL1)
        {
           op_extra_data->mic_out_endpoint = get_overriden_endpoint(op_data->id,AEC_REF_OUTPUT_TERMINAL1);
           op_extra_data->sync_block.mic_data = 0;
        }
        else if(terminal_id==AEC_REF_REFERENCE_TERMINAL)
        {
        	 op_extra_data->sync_block.speaker_data = 0;
        }

        if(op_extra_data->output_stream[terminal_id])
        {
            /* Should never try to connect an already connected terminal*/
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
        else
        {
            op_extra_data->output_stream[terminal_id] = pterminal_buf;
        }
    }

    /* OP not running, accept connections */
    /* OP is running, validate terminals and reset config */
    if(op_data->state != OP_RUNNING)
    {
        base_op_change_response_status(response_data,STATUS_OK);
    }
    else if(validate_channels_and_build(op_data))
    {
       base_op_change_response_status(response_data,STATUS_OK);
    }
    return TRUE;
}

bool aec_reference_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    unsigned terminal_id = *((unsigned*)message_data);

    patch_fn_shared(aec_reference);

    /* Setup Response to Disconnection Request. Assume Failure*/
    *response_id = OPCMD_DISCONNECT;
    if(!base_op_build_std_response(STATUS_CMD_FAILED,op_data->id,response_data))
    {
        return(FALSE);
    }

    /* check if the terminal ID is valid . The number has to be less than the maximum number of sinks or sources.  */
    if(!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* check if the terminal is connected and if so , disconnect the terminal */
    if(terminal_id & TERMINAL_SINK_MASK)
    {
        terminal_id &= ~ TERMINAL_SINK_MASK;

#ifdef AEC_REFERENCE_SUPPORT_METADATA
        if(IsSpeakerInputTerminal(terminal_id))
        {
            tCbuffer *this_buf = op_extra_data->input_stream[terminal_id];
            if(this_buf == op_extra_data->spkr_input_metadata_buffer)
            {
                /* disconnecting buffer is the metadata buffer,
                 * change the metadata buffer to another connected
                 * buffer with metadata, if there is any.
                 */
                tCbuffer *new_metadata_buf = NULL;
                int idx;
                for(idx=0; idx < MAX_NUMBER_SPEAKERS; idx++)
                {
                    tCbuffer *inp_buf = op_extra_data->input_stream[SpeakerInputTerminalByIndex(idx)];
                    if(inp_buf != NULL &&
                       inp_buf != this_buf &&
                       buff_has_metadata(inp_buf))
                    {
                        new_metadata_buf = inp_buf;
                        break;
                    }
                }
                op_extra_data->spkr_input_metadata_buffer = new_metadata_buf;
            }
        }
#endif /* AEC_REFERENCE_SUPPORT_METADATA */

        if(terminal_id==AEC_REF_MIC_TERMINAL1)
        {
           op_extra_data->mic_endpoint=NULL;
        }
        else if(terminal_id==AEC_REF_INPUT_TERMINAL1)
        {
            op_extra_data->spkr_in_endpoint=NULL;
        }

        if(op_extra_data->input_stream[terminal_id])
        {
            op_extra_data->input_stream[terminal_id] = NULL;
        }
        else
        {
            /* Should never try to disconnect an already disconnected terminal*/
            panic_diatribe(PANIC_AUDIO_ALREADY_DISCONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }
    else
    {
        if(terminal_id==AEC_REF_SPKR_TERMINAL1)
        {
           op_extra_data->spkr_endpoint=NULL;
        }
        else if(terminal_id==AEC_REF_OUTPUT_TERMINAL1)
        {
           op_extra_data->mic_out_endpoint=NULL;
        }

        if(op_extra_data->output_stream[terminal_id])
        {
            op_extra_data->output_stream[terminal_id] = NULL;
        }
        else
        {
            /* Streams should not have reached this point of sending us a connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_DISCONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }

    /* OP not running, accept disconnections */
    /* OP is running, validate terminals and reset config */
    if(op_data->state != OP_RUNNING)
    {
        base_op_change_response_status(response_data,STATUS_OK);
    }
    else if(validate_channels_and_build(op_data))
    {
       base_op_change_response_status(response_data,STATUS_OK);
    }

    return TRUE;
}

bool aec_reference_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    unsigned term_id =  *((unsigned *)message_data);
    OP_BUF_DETAILS_RSP *resp;

    patch_fn_shared(aec_reference);

    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }
    resp = (OP_BUF_DETAILS_RSP*)*response_data;

    if ( term_id & TERMINAL_SINK_MASK)
    {
        if(IsMicrophoneTerminal(term_id))
        {
            /* override MIC endpoints */
            resp->needs_override = TRUE;
            resp->b.buffer_size  = 0;
        }
        else
        {
            /* buffer size based on sample rate. 3ms worth */
            resp->b.buffer_size  = frac_mult(op_extra_data->input_rate,FRACTIONAL(0.0030));
#ifdef AEC_REFERENCE_SUPPORT_METADATA
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
            if(op_extra_data->spkr_ttp_gate_enable)
#endif
            {
                /* metadata in speaker graph input side is
                 * supported if ttp gate is enabled
                 * NOTE: this will be done unconditionally later
                 */
                L3_DBG_MSG("AEC_REFERENCE: metadata is supported for speaker inputs");
                resp->metadata_buffer = op_extra_data->spkr_input_metadata_buffer;
                resp->supports_metadata = TRUE;
            }
#endif /* SCO_RX_OP_GENERATE_METADATA */
        }
    }
    else
    {
        if(IsSpeakerTerminal(term_id))
        {
           /* override SPKR endpoints */
            resp->needs_override = TRUE;
            resp->b.buffer_size  = 0;
        }
        else
        {
            /* buffer size based on sample rate */
            resp->b.buffer_size  = frac_mult(op_extra_data->output_rate,FRACTIONAL(0.0087));
        }
    }

    return TRUE;
}

bool aec_reference_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    OP_SCHED_INFO_RSP* resp;
    unsigned terminal_id;

    patch_fn_shared(aec_reference);


    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    /* Populate the response */
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;

    /* block_size set to 1 if ep marked as "real", else it can be any arbitrary
     * value so simplifying the code by choosing 1. */
    resp->block_size = 1;

    /* The real endpoints are locally clocked if the respective overridden
     * endpoint is locally clocked. */
    terminal_id = *((unsigned int*)message_data);
    if( (terminal_id&TERMINAL_SINK_MASK) && !IsMicrophoneTerminal(terminal_id) )
    {
        resp->locally_clocked = is_override_ep_locally_clocked(op_extra_data->spkr_endpoint);
    }
    else if (((terminal_id & TERMINAL_SINK_MASK) == 0) && !IsSpeakerTerminal(terminal_id))
    {
        resp->locally_clocked = is_override_ep_locally_clocked(op_extra_data->mic_endpoint );
    }
    else
    {
        resp->locally_clocked = TRUE;
    }

    return TRUE;
}

bool aec_reference_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* we allocate a standard response, and another 3 fields: number of connected terminals, terminal ID and block size */
    unsigned* resp = (unsigned*)xpmalloc(3*sizeof(unsigned));

    patch_fn_shared(aec_reference);

    *response_id = OPCMD_DATA_FORMAT;
    if((*response_data = (void*)resp) == NULL)
    {
        return(FALSE);
    }
    resp[0] = op_data->id;
    resp[1] = STATUS_OK;
    resp[2] = AUDIO_DATA_FORMAT_FIXP;
    return TRUE;
}



/* ************************************* Data processing-related functions and wrappers **********************************/


/* **************************** Operator message handlers ******************************** */

bool aec_reference_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    patch_fn_shared(aec_reference);

    /* In the case of this capability, nothing is done for control message. Just follow protocol and ignore any content. */
    return cps_control_setup(message_data, resp_length, resp_data,NULL);
}

bool aec_reference_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(aec_reference);

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool aec_reference_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(aec_reference);

    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool aec_reference_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;

    bool retval;

    patch_fn_shared(aec_reference);

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the parameters */
    op_extra_data->ReInitFlag = TRUE;

    return retval;
}

#if defined(IO_DEBUG)
    unsigned ref_spkr_refdrop=0;
    unsigned ref_micref_delay=0;
    unsigned ref_st_drop=0;
    unsigned ref_inserts=0;
    unsigned ref_last_inserts_total=0;
    unsigned ref_last_inserts_insert=0;
    unsigned ref_last_inserts_wrap=0;
#endif

bool aec_reference_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;

#if defined(IO_DEBUG)
    unsigned num_inserts_total  = 0;
    unsigned num_inserts_insert = 0;
    unsigned num_inserts_wrap   = 0;

    unsigned last_acc_mic = 0;
    unsigned last_acc_spkr = 0;
#endif

    patch_fn_shared(aec_reference);

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(AEC_REFERENCE_STATISTICS) ,&resp))
    {
         return FALSE;
    }

    if(resp)
    {
        unsigned config_flag = 0;
        unsigned volume=op_extra_data->shared_volume_ptr->current_volume_level;

        if(op_extra_data->using_sidetone==USE_SIDETONE_FLAG)
        {
           config_flag = flag_uses_SIDETONE;
        }
        if(op_extra_data->channel_status&AEC_REFERENCE_CONSTANT_CONN_TYPE_REF)
        {
           config_flag += flag_uses_AEC_REFERENCE;
        }

        resp = cpsPackWords(&op_extra_data->Cur_mode,&op_extra_data->Ovr_Control, resp);
        resp = cpsPackWords(&config_flag, &volume, resp);
        resp = cpsPackWords(&op_extra_data->channel_status, &op_extra_data->mic_rate, resp);
        resp = cpsPackWords(&op_extra_data->output_rate, &op_extra_data->input_rate, resp);
        resp = cpsPackWords(&op_extra_data->spkr_rate,NULL, resp);

        /* Rate Matching statistics */
        /* TODO: Make these on demand instead of always ON */

#if defined(IO_DEBUG)
       if(st_disgard_op)
       {
           ref_st_drop = get_sink_overflow_disgard_drops(st_disgard_op);
       }
#ifdef AEC_REFERENCE_LATENCY_DEBUG
       ref_spkr_refdrop = op_extra_data->sync_block.speaker_drops + op_extra_data->sync_block.speaker_inserts;
       ref_micref_delay = op_extra_data->sync_block.speaker_delay;
#endif
       if(insert_op)
       {
          num_inserts_total  = get_aec_ref_cbops_inserts_total(insert_op);
          num_inserts_insert = get_aec_ref_cbops_insert_op_insert_total(insert_op);
          num_inserts_wrap   = get_aec_ref_cbops_wrap_op_insert_total(insert_op);

          if(num_inserts_total!=ref_last_inserts_total)
          {
             ref_last_inserts_total=num_inserts_total;
             ref_last_inserts_insert=num_inserts_insert;
             ref_last_inserts_wrap=num_inserts_wrap;
             ref_inserts++;
          }
       }

        last_acc_mic = get_rate_monitor_last_acc(op_extra_data->mic_rate_monitor_op);
        last_acc_spkr = get_rate_monitor_last_acc(op_extra_data->spkr_rate_monitor_op);

        resp = cpsPackWords(&op_extra_data->mic_rate_enactment,&op_extra_data->spkr_rate_enactment, resp);
        resp = cpsPackWords((unsigned*)&last_acc_mic,(unsigned*)&last_acc_spkr, resp);
        resp = cpsPackWords(&op_extra_data->mic_rate_adjustment,&op_extra_data->spkr_rate_adjustment, resp);
        resp = cpsPackWords(&op_extra_data->sync_block.rm_adjustment,(unsigned*)&ref_last_inserts_total, resp);
        resp = cpsPackWords((unsigned*)&ref_spkr_refdrop,(unsigned*)&ref_st_drop, resp);
        cpsPackWords((unsigned*)&ref_micref_delay,NULL, resp);
#endif

    }

    return TRUE;
}

/* Callback function for getting parameters from persistent store */
bool ups_params_aec(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                        uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    patch_fn_shared(aec_reference);

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the parameters */
    op_extra_data->ReInitFlag = TRUE;

    return(TRUE);
}

bool aec_reference_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    patch_fn_shared(aec_reference);

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_extra_data->cap_id,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_aec);

    return retval;
}

bool aec_reference_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;

    patch_fn_shared(aec_reference);

    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,op_extra_data->cap_id,message_data,resp_length,resp_data);
}

bool aec_reference_set_rates(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    unsigned irate,orate;
    bool bChanged = FALSE;

    patch_fn_shared(aec_reference);

    /* Rates needed for creating cbops and for "aec_reference_buffer_details" */
    irate  = OPMSG_FIELD_GET(message_data, OPMSG_AEC_SET_SAMPLE_RATES, INPUT_RATE);
    orate  = OPMSG_FIELD_GET(message_data, OPMSG_AEC_SET_SAMPLE_RATES, OUTPUT_RATE);
    
    

    if(op_extra_data->input_rate!=irate)
    {
        op_extra_data->input_rate = irate;
        bChanged = TRUE;
    }
    if(op_extra_data->output_rate!=orate)
    {
        op_extra_data->output_rate = orate;
        bChanged = TRUE;
    }

    
    if(bChanged && (op_data->state == OP_RUNNING))
    {
        /* Validate channel configuration.   Force rebuild of cbops chains */
        op_extra_data->channel_status=0;
        if(!validate_channels_and_build(op_data))
        {
         /* Not the best way to indicate failure, but best we can do */
            return FALSE;
        }
    }

    return TRUE;
}

#ifdef AEC_REFERENCE_SUPPORT_METADATA
/**
 * aec_reference_get_spkr_ttp_error
 * \brief get the current TTP error for speaker input
 *  This function normally should be called at the start of speaker
 *  graph task.
 * \param op_extra_data pointer to AEC_REFERENCE_OP_DATA data
 * \param cur_time current time
 * \param error pointer for TTP error to be populated by this function.
 *        It will be the difference between desired playback time(ttp)
 *        and (estimated) actual playback time. So the error will be
 *        positive if ttp is later than actual playback time.
 * return whether the error is valid
 */
bool aec_reference_get_spkr_ttp_error(AEC_REFERENCE_OP_DATA   *op_extra_data, TIME cur_time, TIME_INTERVAL *error)
{
    /* see if we have valid metadata buffer */
    tCbuffer *met_buf = op_extra_data->spkr_input_metadata_buffer;
    patch_fn_shared(aec_reference_run);

    if (met_buf!= NULL && buff_has_metadata(met_buf))
    {
        /* see if input has a timestamped tag */
        unsigned b4idx = 0;
        metadata_tag *mtag = buff_metadata_peek_ex(met_buf, &b4idx);
        if((mtag != NULL) && (IS_TIMESTAMPED_TAG(mtag)))
        {
            /* go back to first input sample */
            TIME_INTERVAL time_back = convert_samples_to_time(b4idx / OCTETS_PER_SAMPLE, op_extra_data->input_rate);

            /* set the time stamp,
             * TODO: for further accuracy we could consider the tag sp_adjust as well */
            op_extra_data->last_read_timestamp = time_sub(mtag->timestamp, time_back);

            /* this will stay valid */
            op_extra_data->last_read_timestamp_valid = TRUE;
        }
    }

    if(op_extra_data->last_read_timestamp_valid)
    {
        /* We know the time stamp for first sample in input buffer
         * Now work out how much delay is ahead of that sample.
         *
         * See how many samples are in output (MMU) buffer and
         * convert that to time. There could be up to 1 sample jitter.         *
         */
        unsigned amount_in_output = cbuffer_calc_amount_data_in_words(op_extra_data->output_stream[AEC_REF_SPKR_TERMINAL1]);
        TIME_INTERVAL offset = convert_samples_to_time(amount_in_output, op_extra_data->output_rate);

        /* if we have SW rate adjustment in the graph */
        if(op_extra_data->spkr_sw_rateadj_op != NULL)
        {
            /* adjust for the phase difference between first input and first output sample */
            offset -= (int)frac_mult(SECOND, (int)cbops_sra_get_phase(op_extra_data->spkr_sw_rateadj_op)) / (int) op_extra_data->input_rate;
        }
        /* TODO: further adjustment for any resampler, HW rate matching, algorithmic delay */

        /* compute error */
        *error = time_sub(op_extra_data->last_read_timestamp, cur_time) - offset;
    }
    return op_extra_data->last_read_timestamp_valid;
}


/**
 * aec_reference_update_spkr_last_read_timestamp
 * \brief updates time stamp for speaker input buffer
 *        Call this function after consuming inputs, e.g. after
 *        cbops process, this will enable to have fresh updated time
 *        stamp associated with last consumed sample.
 * \param op_extra_data pointer to AEC_REFERENCE_OP_DATA data
 * \param amount_read amount read from input buffer
 */
void aec_reference_update_spkr_last_read_timestamp(AEC_REFERENCE_OP_DATA   *op_extra_data, unsigned amount_read)
{
    if(op_extra_data->last_read_timestamp_valid)
    {
        /* work out time stamp for next input block for the just read block,
         * TODO: for further accuracy we could consider the tag sp_adjust as well */
        TIME_INTERVAL time_passed = convert_samples_to_time(amount_read, op_extra_data->input_rate);
        op_extra_data->last_read_timestamp = time_add(op_extra_data->last_read_timestamp, time_passed);
    }
}
#endif /* AEC_REFERENCE_SUPPORT_METADATA */

void aec_reference_timer_task(void *kick_object)
{
    OPERATOR_DATA           *op_data = (OPERATOR_DATA*)kick_object;
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*) op_data->extra_op_data;
    TIME next_fire_time;
    unsigned                 sink_kicks=0,source_kicks=0;

#ifdef PROFILER_ON
    if (op_data->profiler != NULL)
    {
        PROFILER_START(op_data->profiler);
    }
#endif

    patch_fn_shared(aec_reference_run);

    if(op_extra_data->ReInitFlag==TRUE)
    {
        op_extra_data->ReInitFlag=FALSE;

        /* Handle Reinitialize */
        if(op_extra_data->mic_sidetone_op)
        {
            initialize_sidetone_filter_op(op_extra_data->mic_sidetone_op);
        }

        aec_reference_set_mic_gains(op_data);
    }

    if(op_extra_data->mic_sidetone_op)
    {
        update_sidetone_filter_op(op_extra_data->mic_sidetone_op,
                                  op_extra_data->params.OFFSET_CONFIG & AEC_REFERENCE_CONFIG_SIDETONEENA,
                                  op_extra_data->shared_volume_ptr->ndvc_filter_sum_lpdnz);
    }

    /*  - Run MIC cbops */
    if(op_extra_data->mic_graph)
    {
#ifdef CHIP_BASE_A7DA_KAS
        if(cbuffer_calc_amount_data_in_words(op_extra_data->input_stream[AEC_REF_MIC_TERMINAL1])>0)
#endif
        {
           cbops_process_data(op_extra_data->mic_graph, CBOPS_MAX_COPY_SIZE-1);
        }
    }

    /*  - Run SPKR cbops */
    if(op_extra_data->spkr_graph)
    {

#ifdef CHIP_BASE_A7DA_KAS
        if(cbuffer_calc_amount_space_in_words(op_extra_data->output_stream[AEC_REF_SPKR_TERMINAL1])>0)
#endif
        {
#ifdef AEC_REFERENCE_SUPPORT_METADATA
            unsigned max_to_process = CBOPS_MAX_COPY_SIZE-1;
            tCbuffer *met_buf = op_extra_data->spkr_input_metadata_buffer;
            unsigned before_amount = 0;
            if(met_buf!= NULL && buff_has_metadata(met_buf))
            {
                /* amount of metadata available */
                unsigned meta_data_available = buff_metadata_available_octets(met_buf)/OCTETS_PER_SAMPLE;

                /* get amount in the buffer before running cbops */
                before_amount = cbuffer_calc_amount_data_in_words(met_buf);

                /* if we have metadata enabled then limit the amount to
                 * process to the amount of available metadata
                 */
                max_to_process = MIN(max_to_process, meta_data_available);
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
                if(op_extra_data->spkr_ttp_gate_enable)
#endif
                {
                    /* calculate current TTP error */
                    TIME current_time = hal_get_time();
                    TIME_INTERVAL error = 0;
                    bool error_valid = aec_reference_get_spkr_ttp_error(op_extra_data, current_time, &error);
                    if(error_valid)
                    {
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
                        /* check whether we need to open the gate */
                        aec_reference_check_speaker_ttp_gate(op_extra_data, error, &max_to_process);
#endif
                        L4_DBG_MSG2("AEC REFERENCE SPEAKER: time=%d, ttp error=%d", current_time, error);
                    }
                }
            }

            /* run cbops process */
            cbops_process_data(op_extra_data->spkr_graph, max_to_process);

            if(met_buf!= NULL && buff_has_metadata(met_buf))
            {
                /* calculate how much input has been consumed,
                 * The assumption is that the write pointer of
                 * input buffer isn't changed during cbops
                 * process.
                 */
                unsigned amount_processed;
                unsigned after_amount = cbuffer_calc_amount_data_in_words(met_buf);
                if(after_amount <= before_amount)
                {
                    amount_processed = before_amount - after_amount;
                }
                else
                {
                    /* This shall never happen, cbops will never
                     * consume more than available data in input buffer.
                     */
                    unsigned buff_size = cbuffer_get_size_in_words(met_buf);
                    amount_processed = buff_size + before_amount - after_amount;
                }

                if(amount_processed >0)
                {
                    /* delete metadata tags for consumed input */
                    unsigned b4idx, afteridx;
                    buff_metadata_tag_list_delete(
                        buff_metadata_remove(met_buf, OCTETS_PER_SAMPLE * amount_processed, &b4idx, &afteridx));
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
                    if(op_extra_data->spkr_ttp_gate_enable)
#endif
                    {
                        /* update timestamp */
                        aec_reference_update_spkr_last_read_timestamp(op_extra_data, amount_processed);
                    }
                }
            }
#else /* AEC_REFERENCE_SUPPORT_METADATA */
            cbops_process_data(op_extra_data->spkr_graph, CBOPS_MAX_COPY_SIZE-1);
#endif /* AEC_REFERENCE_SUPPORT_METADATA*/
#ifdef PROFILER_ON
            if (op_data->profiler != NULL)
            {
                op_data->profiler->kick_inc++;
            }
#endif
        }
    }

    /* Check for Kicks (outputs).   Use Output 1 available data*/
    if(op_extra_data->sync_block.block_sync)
    {
    		source_kicks = op_extra_data->source_kicks;
    		op_extra_data->sync_block.block_sync = 0;
    }

    /* Check for Kick (inputs).   Use Input 1 available space */
    if(op_extra_data->input_stream[AEC_REF_INPUT_TERMINAL1])
    {
        int available_space = cbuffer_calc_amount_space_in_words(op_extra_data->input_stream[AEC_REF_INPUT_TERMINAL1]);

        if(available_space >= op_extra_data->spkr_kick_size)
        {
            sink_kicks = op_extra_data->sink_kicks;
        }
    }


    if(sink_kicks || source_kicks)
    {
       opmgr_kick_from_operator(op_data,source_kicks,sink_kicks);
    }

    /* Next Timer Event */
    next_fire_time = time_add(get_last_fire_time(),AEC_REFERENCE_TIME_PERIOD);
    op_extra_data->kick_id = timer_schedule_event_at(next_fire_time,
            aec_reference_timer_task, (void*)op_data);
#ifdef PROFILER_ON
    if (op_data->profiler != NULL)
    {
        PROFILER_STOP(op_data->profiler);
    }
#endif
}


static unsigned aecref_get_rate_measurment(AEC_REFERENCE_OP_DATA *op_extra_data,bool bSink)
{
   /* 1.0 in Qx.22 independent of word width */
   unsigned rt_meas = 1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT;
   cbops_op    *rate_monitor_op=NULL;

   rate_monitor_op = op_extra_data->mic_rate_monitor_op;

   if(bSink)
   {
      /* If we are not using HW rate enactment assume that
      speaker and microphone are on the same clock.
      Use the microphone rate measurement for speaker */

      if((rate_monitor_op==NULL) ||
         (op_extra_data->spkr_rate_enactment==RATEMATCHING_SUPPORT_HW) ||
         (op_extra_data->mic_rate_enactment==RATEMATCHING_SUPPORT_HW))
      {
         rate_monitor_op = op_extra_data->spkr_rate_monitor_op;
      }
   }

   if (rate_monitor_op)
   {
      rt_meas = rate_monitor_op_get_rate(rate_monitor_op,0);
   }

   return rt_meas;
}

bool aec_reference_opmsg_ep_get_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    OPMSG_GET_CONFIG *msg = message_data;
    OPMSG_GET_CONFIG_RESULT *result = msg->result;
    unsigned term_idx = msg->header.cmd_header.client_id;

    patch_fn_shared(aec_reference);

    /* msg->value - Pointer which will be populated with the asked configuration value
       msg->cmd_header.client_id - Terminal ID (includes TERMINAL_SINK_MASK for sinks)
       msg->key - Parameter Key to return value for */

    switch(msg->key)
    {
        case OPMSG_OP_TERMINAL_DETAILS:
            /* Return a uint32 - Is Terminal emulating a real endpoint. Called at operator endpoint creation.
               */
            if(term_idx & TERMINAL_SINK_MASK)
            {
                result->value = (uint32)(IsMicrophoneTerminal(term_idx)?
                        OPMSG_GET_CONFIG_TERMINAL_DETAILS_NONE:
                        OPMSG_GET_CONFIG_TERMINAL_DETAILS_REAL);
            }
            else
            {
                result->value = (uint32)(IsSpeakerTerminal(term_idx)?
                        OPMSG_GET_CONFIG_TERMINAL_DETAILS_NONE:
                        OPMSG_GET_CONFIG_TERMINAL_DETAILS_REAL);
            }
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_ABILITY: /* uint32 */
           if(term_idx == (AEC_REF_INPUT_TERMINAL1|TERMINAL_SINK_MASK) )
           {
               result->value = (uint32)op_extra_data->spkr_rate_ability;
           }
           else if (term_idx == AEC_REF_OUTPUT_TERMINAL1)
           {
               result->value = (uint32)op_extra_data->mic_rate_ability;
           }
           else
           {
               result->value = (uint32)RATEMATCHING_SUPPORT_AUTO;
           }
           break;
        case OPMSG_OP_TERMINAL_KICK_PERIOD:       /* uint32 */
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_RATE:    /* uint32 */

            if(term_idx == (AEC_REF_INPUT_TERMINAL1|TERMINAL_SINK_MASK) )
            {
#if defined(CHIP_BASE_A7DA_KAS)
               get_override_ep_ratematch_rate(op_extra_data->spkr_endpoint,&result->value);
#else
               result->value = aecref_get_rate_measurment(op_extra_data,TRUE);
#endif
               patch_fn_shared(aec_reference);

               op_extra_data->spkr_rate_meas = (unsigned)(result->value);

            }
            else if (term_idx == AEC_REF_OUTPUT_TERMINAL1)
            {
#if defined(CHIP_BASE_A7DA_KAS)
               get_override_ep_ratematch_rate(op_extra_data->mic_endpoint,&result->value);
#else
               result->value = aecref_get_rate_measurment(op_extra_data,FALSE);
#endif
               patch_fn_shared(aec_reference);

               op_extra_data->mic_rate_meas = (unsigned)(result->value);
            }
            else
            {
                /* 1.0 in Qx.22 independent of word width */
                result->value = 1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT;
            }

            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_MEASUREMENT:
            /* TODO */
            result->rm_measurement.sp_deviation = 0;
            result->rm_measurement.measurement.valid = FALSE;
            break;
        case OPMSG_OP_TERMINAL_BLOCK_SIZE:        /* uint32 */
        case OPMSG_OP_TERMINAL_PROC_TIME:         /* uint32 */
        default:
            return FALSE;
    }

    return TRUE;
}


bool aec_reference_opmsg_ep_configure(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    OPMSG_CONFIGURE  *msg = message_data;
    unsigned terminal_id = msg->header.cmd_header.client_id;
    uint32      value = msg->value;

    patch_fn_shared(aec_reference);

    /* msg->value - Pointer or Value for Key
       msg->cmd_header.client_id - Terminal ID (includes TERMINAL_SINK_MASK for sinks)
       msg->key - Parameter Key to return value for */

    switch(msg->key)
    {
        case OPMSG_OP_TERMINAL_DATA_FORMAT:
            /* value is data type */
            if( ((AUDIO_DATA_FORMAT)msg->value)!=AUDIO_DATA_FORMAT_FIXP )
            {
                return(FALSE);
            }
            break;
        case OPMSG_OP_TERMINAL_KICK_PERIOD:
            /* uint32 polling period in usec - ignore */
            break;
        case OPMSG_OP_TERMINAL_PROC_TIME:
            /* uint32 - N/A an operator will never receive this (has_deadline always FALSE for operators)   */
            break;

        case OPMSG_OP_TERMINAL_CBOPS_PARAMETERS:
            /* value is CBOPS_PARAMETERS *
               TODO - Really need to know type of endpoint: ADC/DAC, I2S, Digital MIC

               CBOPS_DC_REMOVE | CBOPS_SHIFT (set data format --> AUDIO_DATA_FORMAT_FIXP)
               CBOPS_RATEADJUST (EP_RATEMATCH_ENACTING) */

            if(terminal_id == (AEC_REF_MIC_TERMINAL1|TERMINAL_SINK_MASK) )
            {
                op_extra_data->mic_shift = get_shift_from_cbops_parameters((CBOPS_PARAMETERS *)(uintptr_t)value);
            }
            else if (terminal_id == AEC_REF_SPKR_TERMINAL1)
            {
                op_extra_data->spkr_shift = get_shift_from_cbops_parameters((CBOPS_PARAMETERS *)(uintptr_t)value);
            }
            break;
        case OPMSG_OP_TERMINAL_BLOCK_SIZE:
            /* uint32 expected block size per period

             endpoint->state.audio.block_size = (unsigned int)value;
             endpoint->state.audio.kick_period =  (unsigned int)(value * (unsigned long)STREAM_KICK_PERIOD_FROM_USECS(1000000UL) /
                          endpoint->state.audio.sample_rate); */
            if(terminal_id == (AEC_REF_INPUT_TERMINAL1|TERMINAL_SINK_MASK) )
            {
                op_extra_data->spkr_kick_size = (unsigned)value;
            }
            else if (terminal_id == AEC_REF_OUTPUT_TERMINAL1)
            {
                op_extra_data->sync_block.block_size = (unsigned)value;
            }
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_ENACTING:
            /* uint32 TRUE or FALSE.   Operator should perform rate matching if TRUE  */

            if(terminal_id == (AEC_REF_INPUT_TERMINAL1|TERMINAL_SINK_MASK) )
            {
                set_override_ep_ratematch_enacting(op_extra_data->spkr_endpoint,value);
                if(value==(uint32)FALSE)
                {
                    op_extra_data->spkr_rate_enactment = RATEMATCHING_SUPPORT_NONE;
                }
                else
                {
                    op_extra_data->spkr_rate_enactment = op_extra_data->spkr_rate_ability;
                }

#ifndef CHIP_BASE_A7DA_KAS
                if((op_extra_data->spkr_rate_enactment==RATEMATCHING_SUPPORT_HW) && (op_extra_data->spkr_rate_monitor_op))
                {
                   rate_monitor_op_initiatlize(op_extra_data->spkr_rate_monitor_op,op_extra_data->spkr_rate,TRUE,3*MS_PER_SEC);
                }
#endif

                if(op_extra_data->spkr_sw_rateadj_op)
                {
                   cbops_rateadjust_passthrough_mode(op_extra_data->spkr_sw_rateadj_op,(op_extra_data->spkr_rate_enactment==RATEMATCHING_SUPPORT_NONE)?TRUE:FALSE);

                }
            }
            else if (terminal_id == AEC_REF_OUTPUT_TERMINAL1)
            {
                set_override_ep_ratematch_enacting(op_extra_data->mic_endpoint,value);
                if(value==(uint32)FALSE)
                {
                    op_extra_data->mic_rate_enactment = RATEMATCHING_SUPPORT_NONE;
                }
                else
                {
                    op_extra_data->mic_rate_enactment = op_extra_data->mic_rate_ability;
                }
#ifndef CHIP_BASE_A7DA_KAS
                if((op_extra_data->mic_rate_enactment==RATEMATCHING_SUPPORT_HW) && (op_extra_data->mic_rate_monitor_op))
                {
                   rate_monitor_op_initiatlize(op_extra_data->mic_rate_monitor_op,op_extra_data->mic_rate,TRUE,3*MS_PER_SEC);
                }
#endif

                if(op_extra_data->mic_sw_rateadj_op)
                {
                   cbops_rateadjust_passthrough_mode(op_extra_data->mic_sw_rateadj_op,(op_extra_data->mic_rate_enactment==RATEMATCHING_SUPPORT_NONE)?TRUE:FALSE);
                }
            }
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_ADJUSTMENT:
           {
               int svalue = (int)value;
               /*  See BlueCore audio real endpoint function "adjust_audio_rate" for details */
               if(terminal_id == (AEC_REF_INPUT_TERMINAL1|TERMINAL_SINK_MASK) )
               {

#ifdef AEC_REFERENCE_SPKR_TTP_GATE
        if(op_extra_data->spkr_post_ttp_gate_average_error)
        {
            /* This is open loop rate matching, to prevent error from long run drifting
             * we fix the warp value by "very small" amount. In practice most of
             * the time the fix value will be 0, and it normally should take ~1hour before
             * fixing is needed.
             */
            int fix_val = aec_reference_spkr_post_ttp_gate_fixing_value(op_extra_data);
            svalue += fix_val;
        }
#endif /* AEC_REFERENCE_SPKR_TTP_GATE */

                   /* Send Rate Adjustment to hardware */
                   if(op_extra_data->spkr_rate_enactment==RATEMATCHING_SUPPORT_HW)
                   {
                       if(op_extra_data->output_stream[AEC_REF_SPKR_TERMINAL1])
                       {
#ifndef CHIP_BASE_A7DA_KAS
                          if((op_extra_data->spkr_rate_monitor_op) && (rate_monitor_op_is_complete(op_extra_data->spkr_rate_monitor_op)))
                          {
                             op_extra_data->spkr_rate_adjustment = svalue;
                             value = op_extra_data->spkr_rate_adjustment;
                             set_override_ep_ratematch_adjustment(op_extra_data->spkr_endpoint,value);
                             rate_monitor_op_restart(op_extra_data->spkr_rate_monitor_op);

                          }
#else
                          op_extra_data->spkr_rate_adjustment = svalue;
#endif
                       }
                   }
                   else
                   {
                      op_extra_data->spkr_rate_adjustment = svalue;
                   }
               }
               else if (terminal_id == AEC_REF_OUTPUT_TERMINAL1)
               {
                   /* Send Rate Adjustment to hardware */
                   if(op_extra_data->mic_rate_enactment == RATEMATCHING_SUPPORT_HW)
                   {
                      if(op_extra_data->input_stream[AEC_REF_MIC_TERMINAL1])
                      {
#ifndef CHIP_BASE_A7DA_KAS
                         if((op_extra_data->mic_rate_monitor_op) && (rate_monitor_op_is_complete(op_extra_data->mic_rate_monitor_op)))
                         {
                            op_extra_data->mic_rate_adjustment = svalue;
                            value = op_extra_data->mic_rate_adjustment;
                            set_override_ep_ratematch_adjustment(op_extra_data->mic_endpoint,value);
                            rate_monitor_op_restart(op_extra_data->mic_rate_monitor_op);
                         }
#else
                      op_extra_data->mic_rate_adjustment = svalue;
#endif
                      }
                   }
                   else
                   {
                      op_extra_data->mic_rate_adjustment = svalue;
                   }
               }

               /* If enacting rate adjustment, compute SW rate for reference stream */
               if(op_extra_data->spkr_rate_monitor_op && op_extra_data->mic_rate_monitor_op)
               {
                  int mic_ra  = (op_extra_data->mic_rate_enactment==RATEMATCHING_SUPPORT_SW) ?op_extra_data->mic_rate_adjustment :0;
                  int spkr_ra = (op_extra_data->spkr_rate_enactment==RATEMATCHING_SUPPORT_SW)?op_extra_data->spkr_rate_adjustment:0;
                  unsigned mic_rt  = aecref_get_rate_measurment(op_extra_data,FALSE);
                  unsigned spkr_rt = aecref_get_rate_measurment(op_extra_data,TRUE);

                  /* Update reference SW rate adjustment.
                     Clear frm_count to disable backup rate adjustment
                     when rate adjustment is enacted  */
                  op_extra_data->sync_block.frm_count = 0;
                  op_extra_data->sync_block.rm_adjustment = aecref_calc_ref_rate(mic_rt,mic_ra,spkr_rt,spkr_ra);
               }

            }
            break;
        case OPMSG_OP_TERMINAL_RATEMATCH_REFERENCE:
            /* TODO */
            /* break; */
        default:
            return(FALSE);
    }

    return(TRUE);
}

/* With the change of setting rate match ability for all terminals but
   AEC_REF_INPUT_TERMINAL1 and AEC_REF_OUTPUT_TERMINAL1 to AUTO
   this function is never called.

   TODO.  Remove OPMSG_COMMON_GET_CLOCK_ID from table and delete this function
*/

bool aec_reference_opmsg_ep_clock_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    unsigned *resp;
    OP_MSG_REQ *msg = message_data;
    unsigned terminal_id = msg->header.cmd_header.client_id;

    patch_fn_shared(aec_reference);

    /* There is a maximum of 2 possible rates at real terminals. All real sources
     * have a rate tied to the Mic rate. All real sinks have a rate tied to the
     * Spkr rate.
     *
     * If the Mic and Spkr share the same clock source then all real terminals
     * share the same rate.
     */

    /* Payload is a single word containing the clock ID */
    resp = xpmalloc(sizeof(unsigned));
    if (!resp)
    {
        return FALSE;
    }
    *resp_data = (OP_OPMSG_RSP_PAYLOAD*)resp;
    *resp_length = 1;

    /*
         This function is only called for terminals marked as real;AEC_REF_INPUT_TERMINAL[1:8] (sink),
            AEC_REF_OUTPUT_TERMINAL[1:8] (source), or AEC_REF_REFERENCE_TERMINAL (source).  Otherwise,
            clock ID will be reported as zero before this operation is called.

         If MIC and SPKR real endpoints are not connected then we report the same clock source.
     */

    if (terminal_id & TERMINAL_SINK_MASK)
    {
         AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
        /* This is only relevant for the input sinks. If the speaker has the
         * same clock source as the mic then report the same clock source of
         * the op id. If they differ then report op_id and 1 << 7 as the op id
         * is 7 bits long.
         */
        if(!op_extra_data->input_stream[AEC_REF_MIC_TERMINAL1] || !op_extra_data->output_stream[AEC_REF_SPKR_TERMINAL1])
        {
            *resp = op_data->id;
        }
        else if (override_endpoints_have_same_clock_source(op_extra_data->spkr_sw_rateadj_op,
                op_extra_data->mic_sw_rateadj_op))
        {
             *resp = op_data->id;
        }
        else
        {
             *resp = op_data->id | (1 << 7);
        }
    }
    else
    {
        /* The sources that this is relevant to is Outputs and Reference. These
         * all have the same clock source. Report default clock source as the
         * op id.
         */
         *resp = op_data->id;
    }

    return TRUE;
}
