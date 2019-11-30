/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  vse_wrapper.c
 * \ingroup  operators
 *
 *  vse operator
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
#include "vse_wrapper.h"
#include "vse_c.h"

#include "patch/patch.h"

/****************************************************************************
Private Constant Definitions
*/
/** The terminal ID of the input terminal */
#define INPUT_TERMINAL_ID (0 | TERMINAL_SINK_MASK)
/** The terminal ID of the output terminal */
#define OUTPUT_TERMINAL_ID (0)
#define VSE_VSE_VERSION_MINOR 2
/****************************************************************************
Private Type Definitions
*/

#define VSE_TWO_CHANNEL_MASK  0x3

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define VSE_CAP_ID CAP_ID_DOWNLOAD_VSE
#else
#define VSE_CAP_ID CAP_ID_VSE
#endif

/*****************************************************************************
Private Constant Declarations
*/
/** The VSE capability function handler table */
const handler_lookup_struct vse_wrapper_handler_table =
{
    vse_wrapper_create,            /* OPCMD_CREATE */
    vse_wrapper_destroy,           /* OPCMD_DESTROY */
    vse_wrapper_start,             /* OPCMD_START */
    multi_channel_stop,            /* OPCMD_STOP */
    multi_channel_reset,           /* OPCMD_RESET */
    multi_channel_connect,         /* OPCMD_CONNECT */
    multi_channel_disconnect,      /* OPCMD_DISCONNECT */
    multi_channel_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,       /* OPCMD_DATA_FORMAT */
    multi_channel_sched_info       /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry vse_wrapper_opmsg_handler_table[] =
    {
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,             base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,                        vse_wrapper_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                         vse_wrapper_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                       vse_wrapper_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                         vse_wrapper_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                         vse_wrapper_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                           vse_wrapper_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                  vse_wrapper_opmsg_get_ps_id},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                       vse_wrapper_opmsg_set_sample_rate},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,                 multi_channel_stream_based},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,                    multi_channel_opmsg_set_buffer_size},
#ifdef INSTALL_METADATA
    {OPMSG_COMMON_SET_METADATA_DELAY,                    vse_wrapper_opmsg_set_metadata_delay},
#endif
    {0, NULL}};

const CAPABILITY_DATA vse_cap_data =
{
    VSE_CAP_ID,                                                  /* Capability ID */
    VSE_VSE_VERSION_MAJOR, VSE_VSE_VERSION_MINOR,                /* Version information - hi and lo parts */
    VSE_CAP_MAX_CHANNELS, VSE_CAP_MAX_CHANNELS,                  /* Max number of sinks/inputs and sources/outputs */
    &vse_wrapper_handler_table,                                  /* Pointer to message handler function table */
    vse_wrapper_opmsg_handler_table,                             /* Pointer to operator message handler function table */
    vse_wrapper_process_data,                                    /* Pointer to data processing function */
    0,                                                           /* TODO - Processing time information */
    sizeof(VSE_OP_DATA)                                          /* Size of capability-specific per-instance data */
};

/****************************************************************************
Private Function Declarations
*/
/* ******************************* Helper functions ************************************ */


/* ********************************** API functions ************************************* */


bool vse_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    VSE_OP_DATA *p_ext_data = (VSE_OP_DATA *)(op_data->extra_op_data);

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Allocate channels, in-place, no hot connect */
    if( !multi_channel_create(op_data,(MULTI_INPLACE_FLAG|MULTI_METADATA_FLAG),sizeof(vse_channels)) )
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    multi_channel_set_callbacks(op_data,vse_channel_create,vse_channel_destroy);

    /* Initialize extended data for operator.  Assume intialized to zero*/
    p_ext_data->ReInitFlag = 1;
    p_ext_data->Host_mode = VSE_SYSMODE_FULL;
    p_ext_data->Cur_mode  = p_ext_data->Host_mode;
    p_ext_data->sample_rate = 48000;


    if(!cpsInitParameters(&p_ext_data->parms_def,(unsigned*)VSE_GetDefaults(VSE_CAP_ID),(unsigned*)&p_ext_data->vse_cap_params,sizeof(VSE_PARAMETERS)))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        multi_channel_detroy(op_data);
        return TRUE;
    }

    op_data->state = OP_NOT_RUNNING;

    return TRUE;
}

bool vse_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* call base_op destroy that creates and fills response message, too */
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        return(FALSE);
    }
    multi_channel_detroy(op_data);

    return TRUE;
}


bool vse_wrapper_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
   /* Both channels must be connected */
   if( multi_channel_active_channels(op_data)!=VSE_TWO_CHANNEL_MASK)
   {
      *response_id = OPCMD_START;
	   op_data->state = OP_NOT_RUNNING;
      return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
   }
   /* OK, Continue to normal start */
   return multi_channel_start(op_data,message_data,response_id,response_data);
}


/* ************************************* Data processing-related functions and wrappers **********************************/
void vse_wrapper_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    VSE_OP_DATA *p_ext_data = (VSE_OP_DATA *)(op_data->extra_op_data);
    vse_channels *channels  = (vse_channels*)multi_channel_first_active_channel(op_data);
    unsigned samples_to_process;

    patch_fn(vse_wrapper_process_data);

    /* Check status of terminals */
    samples_to_process = multi_channel_check_buffers(op_data,touched);
   
   
    if(p_ext_data->ReInitFlag)
    {
       vse_initialize(p_ext_data,channels);
    }

    /* call ASM processing function */
    if(samples_to_process>0)
    {
#ifdef INSTALL_METADATA
         multi_channel_metadata_propagate(op_data,samples_to_process);
#endif
         vse_processing(p_ext_data, samples_to_process,channels);
    }
}

/* **************************** Operator message handlers ******************************** */

bool vse_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA     *op_extra_data = (VSE_OP_DATA*)op_data->extra_op_data;
    unsigned            i,num_controls,cntrl_value; 
    CPS_CONTROL_SOURCE  cntrl_src;
    unsigned result = OPMSG_RESULT_STATES_NORMAL_STATE;
    
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
        /* Only interested in lower 8-bits of value */
        cntrl_value &= 0xFF;
        if (cntrl_value >= VSE_SYSMODE_MAX_MODES)
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
            op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ?  0 : VSE_CONTROL_MODE_OVERRIDE;
            op_extra_data->Obpm_mode = cntrl_value;
        }
    }

    cps_response_set_result(resp_data,result);

    /* Set Current Mode */
    if(op_extra_data->Ovr_Control&VSE_CONTROL_MODE_OVERRIDE)
    {
       op_extra_data->Cur_mode = op_extra_data->Obpm_mode;
    }
    else
    {
      op_extra_data->Cur_mode = op_extra_data->Host_mode;
    }

    /* Set the Reinit flag after setting the paramters */
    if (result == OPMSG_RESULT_STATES_NORMAL_STATE)
    {
        op_extra_data->ReInitFlag = 1;
    }
    return TRUE;
}

bool vse_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA   *op_extra_data = (VSE_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool vse_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA   *op_extra_data = (VSE_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool vse_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA   *op_extra_data = (VSE_OP_DATA*)op_data->extra_op_data;
    bool retval;

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

bool vse_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA   *op_extra_data = (VSE_OP_DATA*)op_data->extra_op_data;
    unsigned      *resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(VSE_STATISTICS),&resp))
    {
         return FALSE;
    }

    if(resp)
    {
       OPSTATE_INTERNAL op_state_status = (multi_channel_active_channels(op_data)==VSE_TWO_CHANNEL_MASK) ? OPSTATE_INTERNAL_CONNECTED : OPSTATE_INTERNAL_READY;
       unsigned comp_config=0;

        resp = cpsPackWords(&op_extra_data->Cur_mode,&op_extra_data->Ovr_Control ,resp);
        resp = cpsPackWords(&comp_config,(unsigned*)&op_data->state ,resp);
        cpsPackWords((unsigned*)&op_state_status,NULL ,resp);
    }
    return TRUE;
}

bool vse_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA   *op_extra_data = (VSE_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(VSE_CAP_ID,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_vse);

    return retval;
}

bool vse_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA   *op_extra_data = (VSE_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,VSE_CAP_ID,message_data,resp_length,resp_data);
}

bool vse_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    VSE_OP_DATA *p_ext_data = (VSE_OP_DATA *)(op_data->extra_op_data);

    /* We received client ID, length and then opmsgID and OBPM params */
    p_ext_data->sample_rate = 25 * ((unsigned*)message_data)[3];
    p_ext_data->ReInitFlag = 1;

    return(TRUE);
}

#ifdef INSTALL_METADATA
bool vse_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    MULTI_CHANNEL_DEF *chan_def = (MULTI_CHANNEL_DEF*)op_data->cap_class_ext;

    /* The message value is in samples. Convert it to octets */
    chan_def->metadata_delay = OCTETS_PER_SAMPLE * (((unsigned*)message_data)[3]);

    return (TRUE);
}
#endif /* INSTALL_METADATA */

bool ups_params_vse(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    VSE_OP_DATA   *op_extra_data = (VSE_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}


/**
 * \brief Allocate a vse data object, which contains (among other things), the
 * vse's history buffers. Returns true if succesful, false if failed
 *
 * \param terminal_num : (input) Channel which will be associated with the vse object
 * \param p_ext_data : pointer to the extra op data structure
 */
bool vse_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx)
{
   VSE_OP_DATA  *p_ext_data = (VSE_OP_DATA *)(op_data->extra_op_data);
   vse_channels *vse_chan  = (vse_channels*)chan_ptr;
   t_vse_object *p_dobject;
   unsigned int *out_ipsi;
   unsigned int *out_contra;

   p_dobject = xzppmalloc(VSE_OBJECT_SIZE , MALLOC_PREFERENCE_DM1);
   if (p_dobject == NULL)
   {
        /* failed to allocate */
        return FALSE;
   }
   vse_chan->vse_object = p_dobject;
   vse_chan->chan_idx   = chan_idx; 

   /* Link in parameters */
   p_dobject->params_ptr =(t_vse_params*)&p_ext_data->vse_cap_params;

   /* allocate the VSE Buffers*/
   out_ipsi = (unsigned int *)xzppmalloc(sizeof(unsigned int) *VSE_CAP_INTERNAL_BUFFER_SIZE , MALLOC_PREFERENCE_DM1);
   if (out_ipsi == NULL)
   {
        return FALSE;
   }
   p_dobject->out_ipsi = out_ipsi;

   out_contra = (unsigned int *)xzppmalloc(sizeof(unsigned int) *VSE_CAP_INTERNAL_BUFFER_SIZE , MALLOC_PREFERENCE_DM1);
   if (out_contra == NULL)
   {
        return FALSE;
   }
   p_dobject->out_contra = out_contra;

   
   /* signal reset.  change change */
   p_ext_data->ReInitFlag = 1;

   return TRUE;
}

void vse_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr)
{
   vse_channels *vse_chan  = (vse_channels*)chan_ptr;
   t_vse_object *p_dobject = vse_chan->vse_object;

   vse_chan->vse_object = NULL;

   if(p_dobject)
   {
      /* free the VSE buffers */
      if (p_dobject->out_ipsi)
      {
         pfree(p_dobject->out_ipsi);
      }
      if (p_dobject->out_contra)
      {
         pfree(p_dobject->out_contra);
      }
      pfree(p_dobject);
   }
}

