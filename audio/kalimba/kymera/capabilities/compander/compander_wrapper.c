/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  compander_wrapper.c
 * \ingroup  operators
 *
 *  compander operator
 *
 */
/****************************************************************************
Include Files
*/

#include "capabilities.h"
#include "adaptor/adaptor.h"
#include "mem_utils/scratch_memory.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "opmsg_prim.h"
#include "op_msg_helpers.h"
#include "compander_wrapper.h"
#include "compander_c.h"

#include "patch/patch.h"


/****************************************************************************
Private Constant Definitions
*/
/** The terminal ID of the input terminal */
#define INPUT_TERMINAL_ID (0 | TERMINAL_SINK_MASK)
/** The terminal ID of the output terminal */
#define OUTPUT_TERMINAL_ID (0)
#define COMPANDER_COMPANDER_VERSION_MINOR 1
#define COMPANDER_NUM_PARAMETERS     (sizeof(COMPANDER_PARAMETERS) >> LOG2_ADDR_PER_WORD)
/****************************************************************************
Private Type Definitions
*/
#ifdef CAPABILITY_DOWNLOAD_BUILD
#define COMPANDER_CAP_ID CAP_ID_DOWNLOAD_COMPANDER
#else
#define COMPANDER_CAP_ID CAP_ID_COMPANDER
#endif

/*****************************************************************************
Private Constant Declarations
*/
/** The compander capability function handler table */
const handler_lookup_struct compander_wrapper_handler_table =
{
    compander_wrapper_create,      /* OPCMD_CREATE */
    compander_wrapper_destroy,     /* OPCMD_DESTROY */
    multi_channel_start,           /* OPCMD_START */
    multi_channel_stop,            /* OPCMD_STOP */
    multi_channel_reset,           /* OPCMD_RESET */
    multi_channel_connect,         /* OPCMD_CONNECT */
    multi_channel_disconnect,      /* OPCMD_DISCONNECT */
    multi_channel_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,       /* OPCMD_DATA_FORMAT */
    multi_channel_sched_info       /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry compander_wrapper_opmsg_handler_table[] =
    {
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,             base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,                        compander_wrapper_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                         compander_wrapper_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                       compander_wrapper_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                         compander_wrapper_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                         compander_wrapper_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                           compander_wrapper_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                  compander_wrapper_opmsg_get_ps_id},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                       compander_wrapper_opmsg_set_sample_rate},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,                 multi_channel_stream_based},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,                    multi_channel_opmsg_set_buffer_size},
    {0, NULL}};

const CAPABILITY_DATA compander_cap_data =
{
    COMPANDER_CAP_ID,                                                         /* Capability ID */
    COMPANDER_COMPANDER_VERSION_MAJOR, COMPANDER_COMPANDER_VERSION_MINOR,     /* Version information - hi and lo parts */
    COMPANDER_CAP_MAX_CHANNELS, COMPANDER_CAP_MAX_CHANNELS,                   /* Max number of sinks/inputs and sources/outputs */
    &compander_wrapper_handler_table,                                         /* Pointer to message handler function table */
    compander_wrapper_opmsg_handler_table,                                    /* Pointer to operator message handler function table */
    compander_wrapper_process_data,                                           /* Pointer to data processing function */
    0,                                                                        /* TODO - Processing time information */
    sizeof(COMPANDER_OP_DATA)                                                 /* Size of capability-specific per-instance data */
};


/****************************************************************************
Private Function Declarations
*/

/* ********************************** API functions ************************************* */ 

bool compander_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    COMPANDER_OP_DATA *p_ext_data = (COMPANDER_OP_DATA *)(op_data->extra_op_data);

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Allocate channels, in-place, no hot connect */
    if( !multi_channel_create(op_data,(MULTI_INPLACE_FLAG|MULTI_METADATA_FLAG),sizeof(compander_channels)) )
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    multi_channel_set_callbacks(op_data,compander_channel_create,compander_channel_destroy);

    /* Initialize extended data for operator.  Assume intialized to zero*/
    p_ext_data->ReInitFlag = 1;
    p_ext_data->Host_mode = COMPANDER_SYSMODE_FULL;
    p_ext_data->Cur_mode  = COMPANDER_SYSMODE_FULL;
  
    p_ext_data->sample_rate = 48000;

    if(!cpsInitParameters(&p_ext_data->parms_def,(unsigned*)COMPANDER_GetDefaults(COMPANDER_CAP_ID),(unsigned*)&p_ext_data->compander_cap_params,sizeof(COMPANDER_PARAMETERS)))
    {
       base_op_change_response_status(response_data, STATUS_CMD_FAILED);
       multi_channel_detroy(op_data);
       return TRUE;
    }
   
    op_data->state = OP_NOT_RUNNING;

    return TRUE;
}

bool compander_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* call base_op destroy that creates and fills response message, too */
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        return(FALSE);
    }
    multi_channel_detroy(op_data);

    return TRUE;
}


/* ************************************* Data processing-related functions and wrappers **********************************/
void compander_wrapper_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
   COMPANDER_OP_DATA *p_ext_data = (COMPANDER_OP_DATA *)(op_data->extra_op_data);
   compander_channels *channels  = (compander_channels*)multi_channel_first_active_channel(op_data);
   unsigned samples_to_process;

   /* Make sure there are channels to process */
   if(channels==NULL)
   {
      return;
   }
   patch_fn(compander_wrapper_process_data);

   /* Handle initialization.  May change block size */
   if(p_ext_data->ReInitFlag)
   {
      unsigned block_size;

      compander_initialize(p_ext_data,multi_channel_active_channels(op_data),channels);

      /* Set compander block size */
      block_size = channels->compander_object->gain_update_rate;
      if(block_size>1)
      {
         multi_channel_set_block_size(op_data,block_size);
      }
   }

   /* Check status of terminals */
   samples_to_process = multi_channel_check_buffers(op_data,touched);
   
   if(samples_to_process>0)
   {
       /* Compander may process less than available */
       samples_to_process = compander_processing(p_ext_data,samples_to_process,channels);
       /* If compander processed nothiing, do not kick.  
          This should not happen as block size is set to ensure some processing */
       if(samples_to_process==0)
       {
          touched->sources = 0;
          touched->sinks   = 0;
       }
#ifdef INSTALL_METADATA
       else
       {
          multi_channel_metadata_propagate(op_data,samples_to_process);
       }
#endif
   }
}


/* **************************** Operator message handlers ******************************** */

bool compander_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA  *op_extra_data = (COMPANDER_OP_DATA*)op_data->extra_op_data;
    unsigned            i,num_controls,cntrl_value; 
    CPS_CONTROL_SOURCE  cntrl_src;
    unsigned            result = OPMSG_RESULT_STATES_NORMAL_STATE;
    
    if(!cps_control_setup(message_data, resp_length, resp_data,&num_controls))
    {
       return FALSE;
    }

    for(i=0;i<num_controls;i++)
    {
        unsigned  cntrl_id=cps_control_get(message_data,i,&cntrl_value,&cntrl_src);

        if(cntrl_id != OPMSG_CONTROL_MODE_ID)
        {
            result = OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL;
            break;
        }
        /* Only interested in low 8-bits of value */
        cntrl_value &= 0xFF;
        if (cntrl_value >= COMPANDER_SYSMODE_MAX_MODES)
        {
            result = OPMSG_RESULT_STATES_INVALID_CONTROL_VALUE;
            break;
        }
        /* Control is Mode */
        if(cntrl_src == CPS_SOURCE_HOST)
        {
           op_extra_data->Host_mode = cntrl_value;
        }
        else
        {
            op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ?  0 : COMPANDER_CONTROL_MODE_OVERRIDE;
            op_extra_data->Obpm_mode = cntrl_value;
        }
    }

    if(op_extra_data->Ovr_Control & COMPANDER_CONTROL_MODE_OVERRIDE)
    {
       op_extra_data->Cur_mode  = op_extra_data->Obpm_mode;
    }
    else
    {
       op_extra_data->Cur_mode  = op_extra_data->Host_mode;
    }

    cps_response_set_result(resp_data,result);

    /* Set the Reinit flag after setting the paramters */
    if (result == OPMSG_RESULT_STATES_NORMAL_STATE)
    {
        op_extra_data->ReInitFlag = 1;
    }
    return TRUE;
}

bool compander_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA   *op_extra_data = (COMPANDER_OP_DATA*)op_data->extra_op_data;
    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool compander_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA   *op_extra_data = (COMPANDER_OP_DATA*)op_data->extra_op_data;
    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

}

bool compander_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA   *op_extra_data = (COMPANDER_OP_DATA*)op_data->extra_op_data;
    
    bool retval;
    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;

}

bool compander_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA   *op_extra_data = (COMPANDER_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(COMPANDER_STATISTICS),&resp))
    {
         return FALSE;
    }

    if(resp)
    {
        OPSTATE_INTERNAL op_state_stat = multi_channel_active_channels(op_data) ? OPSTATE_INTERNAL_CONNECTED : OPSTATE_INTERNAL_READY;
        unsigned comp_config=0;

        resp = cpsPackWords(&op_extra_data->Cur_mode,&op_extra_data->Ovr_Control ,resp);
        resp = cpsPackWords(&comp_config,(unsigned*)&op_data->state ,resp);
        cpsPackWords((unsigned*)&op_state_stat,&op_extra_data->lookahead_status ,resp);
    }

    return TRUE;
}

bool compander_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA   *op_extra_data = (COMPANDER_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(COMPANDER_CAP_ID,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_compander);

    return retval;
}

bool compander_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA   *op_extra_data = (COMPANDER_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,COMPANDER_CAP_ID,message_data,resp_length,resp_data);

}

bool compander_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    COMPANDER_OP_DATA *p_ext_data = (COMPANDER_OP_DATA *)(op_data->extra_op_data);
   
   /* We received client ID, length and then opmsgID and OBPM params */
    p_ext_data->sample_rate = 25 * ((unsigned*)message_data)[3];
    p_ext_data->ReInitFlag = 1;

    return(TRUE);
}

bool ups_params_compander(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    COMPANDER_OP_DATA   *op_extra_data = (COMPANDER_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}

bool compander_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx)
{
   COMPANDER_OP_DATA *p_ext_data = (COMPANDER_OP_DATA *)(op_data->extra_op_data);
   compander_channels *compander_chan  = (compander_channels*)chan_ptr;
   t_compander_object *p_dobject;

   p_dobject = xzppmalloc(COMPANDER_OBJECT_SIZE , MALLOC_PREFERENCE_DM1);
   if (p_dobject == NULL)
   {
      /* failed to allocate */
      return FALSE;
   }
   compander_chan->compander_object = p_dobject;
   compander_chan->chan_mask        = 1<<chan_idx;

   p_dobject->params_ptr =(t_compander_params*)&p_ext_data->compander_cap_params;
   // *************************************************************************
   // Set the input/output cbuffer/framebuffer pointers for all data objects
   // *************************************************************************
   p_dobject->channel_input_ptr  = (void*)compander_chan->common.sink_buffer_ptr;
   p_dobject->channel_output_ptr = (void*)compander_chan->common.source_buffer_ptr;

   p_ext_data->ReInitFlag = 1;

   return TRUE;
}

void compander_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr)
{
   compander_channels *compander_chan  = (compander_channels*)chan_ptr;
   t_compander_object *p_dobject;

   p_dobject = compander_chan->compander_object;
   compander_chan->compander_object = NULL;

   if(p_dobject)
   {
      /* free lookahead history buffer */
      if (p_dobject->lookahead_hist_buf)
      {
         pfree(p_dobject->lookahead_hist_buf);
         p_dobject->lookahead_hist_buf = NULL;
      }
      pfree(p_dobject);
   }
}

