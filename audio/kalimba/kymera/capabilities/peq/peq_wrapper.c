/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  peq_wrapper.c
 * \ingroup  operators
 *
 *  peq operator
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
#include "peq_wrapper.h"
#include "peq_c.h"
#include "peq_gen_c.h"

#include "patch/patch.h"


/****************************************************************************
Private Constant Definitions
*/
/** The terminal ID of the input terminal */
#define INPUT_TERMINAL_ID (0 | TERMINAL_SINK_MASK)
/** The terminal ID of the output terminal */
#define OUTPUT_TERMINAL_ID (0)
#define PEQ_PEQ_VERSION_MINOR 2

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define PEQ_CAP_ID CAP_ID_DOWNLOAD_PEQ
#else
#define PEQ_CAP_ID CAP_ID_PEQ
#endif

/****************************************************************************
Private Type Definitions
*/

/*****************************************************************************
Private Constant Declarations
*/

/** The PEQ capability function handler table */
const handler_lookup_struct peq_wrapper_handler_table =
{
    peq_wrapper_create,             /* OPCMD_CREATE */
    peq_wrapper_destroy,            /* OPCMD_DESTROY */
    peq_wrapper_start,              /* OPCMD_START */
    multi_channel_stop,             /* OPCMD_STOP */
    multi_channel_reset,            /* OPCMD_RESET */
    multi_channel_connect,          /* OPCMD_CONNECT */
    multi_channel_disconnect,       /* OPCMD_DISCONNECT */
    multi_channel_buffer_details,   /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,        /* OPCMD_DATA_FORMAT */
    multi_channel_sched_info        /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry peq_wrapper_opmsg_handler_table[] =
    {
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,             base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,                        peq_wrapper_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                         peq_wrapper_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                       peq_wrapper_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                         peq_wrapper_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                         peq_wrapper_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                           peq_wrapper_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                  peq_wrapper_opmsg_get_ps_id},
    {OPMSG_PEQ_ID_OVERRIDE_COEFFS,                       peq_wrapper_opmsg_override_coeffs},
    {OPMSG_PEQ_ID_LOAD_CONFIG,                           peq_wrapper_opmsg_load_config},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                       peq_wrapper_opmsg_set_sample_rate},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,                 multi_channel_stream_based},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,                    multi_channel_opmsg_set_buffer_size},
#ifdef INSTALL_METADATA
    {OPMSG_COMMON_SET_METADATA_DELAY,                    peq_wrapper_opmsg_set_metadata_delay},
#endif
    {0, NULL}};

const CAPABILITY_DATA peq_cap_data =
{
    PEQ_CAP_ID,                                   /* Capability ID */
    PEQ_PEQ_VERSION_MAJOR, PEQ_PEQ_VERSION_MINOR, /* Version information - hi and lo parts */
    PEQ_CAP_MAX_CHANNELS, PEQ_CAP_MAX_CHANNELS,   /* Max number of sinks/inputs and sources/outputs */
    &peq_wrapper_handler_table,                   /* Pointer to message handler function table */
    peq_wrapper_opmsg_handler_table,              /* Pointer to operator message handler function table */
    peq_wrapper_process_data,                     /* Pointer to data processing function */
    0,                                            /* TODO - Processing time information */
    sizeof(PEQ_OP_DATA)                           /* Size of capability-specific per-instance data */
};

/****************************************************************************
Private Function Declarations
*/
/* ******************************* Helper functions ************************************ */

inline static void peq_recompute_coeffs(PEQ_OP_DATA *p_ext_data)
{
   /* If not overriding, signal coefficient reset */
   if(p_ext_data->coeff_override_flag==0)
   {
       p_ext_data->ParameterLoadStatus = -1;
   }
}



/* ********************************** API functions ************************************* */
#ifdef PEQ_DEBUG
PEQ_OP_DATA* peq_cap_extra_op_data = NULL;
unsigned peq_cap_extra_op_data_len = 0;
#endif

bool peq_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    PEQ_OP_DATA *p_ext_data = (PEQ_OP_DATA *)(op_data->extra_op_data);
    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Allocate channels, in-place, no hot connect */
    if( !multi_channel_create(op_data,(MULTI_INPLACE_FLAG|MULTI_METADATA_FLAG),sizeof(peq_channels)) )
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        PEQ_CAP_Destroy(op_data);
        return TRUE;
    }
    multi_channel_set_callbacks(op_data,peq_channel_create,peq_channel_destroy);

    /* allocate memory for coefficients */
    /* create the "parameters" object (note, these is not same as the OBPM parameters) */
    p_ext_data->peq_coeff_params = xzppmalloc(PEQ_PARAMS_OBJECT_SIZE(PEQ_MAX_STAGES), MALLOC_PREFERENCE_DM1);
    if(p_ext_data->peq_coeff_params == NULL)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        PEQ_CAP_Destroy(op_data);
        return TRUE;
    }

#ifdef PEQ_DEBUG
    /* ext_op_data into global variable for debugging support*/
    peq_cap_extra_op_data = p_ext_data;
    peq_cap_extra_op_data_len = sizeof(PEQ_OP_DATA);
#endif


    /* Initialize extended data for operator.  Assume intialized to zero*/
    p_ext_data->Host_mode = PEQ_SYSMODE_FULL;
    p_ext_data->Cur_mode  = PEQ_SYSMODE_FULL;
    p_ext_data->coeff_override_flag = 0;
    p_ext_data->sample_rate = 8000;

    if(!cpsInitParameters(&p_ext_data->parms_def,(unsigned*)PEQ_GetDefaults(PEQ_CAP_ID),(unsigned*)&p_ext_data->peq_cap_params,sizeof(PEQ_PARAMETERS)))
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        PEQ_CAP_Destroy(op_data);
        return TRUE;
    }

    peq_recompute_coeffs(p_ext_data);

    op_data->state = OP_NOT_RUNNING;

    return TRUE;
}

bool peq_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* check that we are not trying to destroy a running operator */
    if(op_data->state == OP_RUNNING)
    {
        *response_id = OPCMD_DESTROY;

        /* We can't destroy a running operator. */
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }
    else
    {
        PEQ_CAP_Destroy(op_data);
        /* call base_op destroy that creates and fills response message, too */
        return base_op_destroy(op_data, message_data, response_id, response_data);
    }
}

bool peq_wrapper_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    PEQ_OP_DATA *p_ext_data = (PEQ_OP_DATA *)(op_data->extra_op_data);

    if (!multi_channel_start(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Initiate coefficient update */
    if(p_ext_data->ParameterLoadStatus!=0)
    {
         peq_coeff_change(p_ext_data);
    }
    return TRUE;
}

/* ************************************* Data processing-related functions and wrappers **********************************/
void peq_wrapper_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    unsigned samples_to_process;
    PEQ_OP_DATA  *p_ext_data = (PEQ_OP_DATA *)(op_data->extra_op_data);
    peq_channels *channels   = (peq_channels*)multi_channel_first_active_channel(op_data);

    /* Check status of terminals */
    samples_to_process = multi_channel_check_buffers(op_data,touched);

    /* Update coefficients */
    if(p_ext_data->ParameterLoadStatus!=0)
    {
         peq_coeff_change(p_ext_data);
    }

    /* Check for re-initialization */
    if(p_ext_data->ReInitFlag)
    {
       peq_initialize(p_ext_data,channels);
    }

    /* Perform peq filtering */
    if(samples_to_process>0)
    {
#ifdef INSTALL_METADATA
         multi_channel_metadata_propagate(op_data,samples_to_process);
#endif
         peq_processing(p_ext_data,channels,samples_to_process);
    }
}



/* **************************** Operator message handlers ******************************** */

bool peq_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA        *op_extra_data = (PEQ_OP_DATA*)op_data->extra_op_data;
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
        if (cntrl_value >= PEQ_SYSMODE_MAX_MODES)
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
            op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ?  0 : PEQ_CONTROL_MODE_OVERRIDE;
            op_extra_data->Obpm_mode = cntrl_value;
        }
    }

    if(op_extra_data->Ovr_Control & PEQ_CONTROL_MODE_OVERRIDE)
    {
       op_extra_data->Cur_mode = op_extra_data->Obpm_mode;
    }
    else
    {
      op_extra_data->Cur_mode = op_extra_data->Host_mode;
    }

    cps_response_set_result(resp_data,result);

    /* Set the Reinit flag after setting the paramters */
    if (result == OPMSG_RESULT_STATES_NORMAL_STATE)
    {
        op_extra_data->ReInitFlag = 1;
    }
    return TRUE;
}

bool peq_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA   *op_extra_data = (PEQ_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool peq_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA   *op_extra_data = (PEQ_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool peq_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA   *op_extra_data = (PEQ_OP_DATA*)op_data->extra_op_data;
    bool retval;

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* If not overriding, signal coefficient reset */
    peq_recompute_coeffs(op_extra_data);

    return retval;
}

bool peq_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA   *op_extra_data = (PEQ_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(PEQ_STATISTICS),&resp))
    {
         return FALSE;
    }

    if(resp)
    {
        OPSTATE_INTERNAL op_state_stat = multi_channel_active_channels(op_data) ? OPSTATE_INTERNAL_CONNECTED : OPSTATE_INTERNAL_READY;
        unsigned comp_config=0;

        resp = cpsPackWords(&op_extra_data->Cur_mode, &op_extra_data->Ovr_Control, resp);
        resp = cpsPackWords(&op_extra_data->coeff_override_flag, &comp_config, resp);
        cpsPackWords((unsigned*)&op_data->state, (unsigned*)&op_state_stat, resp);
    }
    return TRUE;
}



bool peq_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA   *op_extra_data = (PEQ_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(PEQ_CAP_ID,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_peq);

    return retval;;
}

bool peq_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA   *op_extra_data = (PEQ_OP_DATA*)op_data->extra_op_data;

    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,PEQ_CAP_ID,message_data,resp_length,resp_data);
}

bool peq_wrapper_opmsg_load_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return(TRUE);
}

bool peq_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA *p_ext_data = (PEQ_OP_DATA *)(op_data->extra_op_data);
    unsigned    sample_rate;

    sample_rate = 25 * ((unsigned*)message_data)[3];

    if(p_ext_data->sample_rate != sample_rate)
    {
         p_ext_data->sample_rate = sample_rate;
         peq_recompute_coeffs(p_ext_data);
    }

    return(TRUE);
}

#ifdef INSTALL_METADATA
bool peq_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    MULTI_CHANNEL_DEF *chan_def = (MULTI_CHANNEL_DEF*)op_data->cap_class_ext;

    /* The message value is in samples. Convert it to octets */
    chan_def->metadata_delay = OCTETS_PER_SAMPLE * (((unsigned*)message_data)[3]);

    return (TRUE);
}
#endif /* INSTALL_METADATA */

bool ups_params_peq(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    PEQ_OP_DATA   *op_extra_data = (PEQ_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    peq_recompute_coeffs(op_extra_data);

    return(TRUE);
}

/**
 * \brief Free memory which is be used for the entire lifespan of the operator
 *
 * \param p_ext_data : pointer to the extra op data structure
 */
void PEQ_CAP_Destroy(OPERATOR_DATA *op_data)
{
	 PEQ_OP_DATA *p_ext_data = (PEQ_OP_DATA *)(op_data->extra_op_data);
	
    multi_channel_detroy(op_data);

    /* free the coefficient params object */
    if (p_ext_data->peq_coeff_params)
    {
        pfree(p_ext_data->peq_coeff_params);
        p_ext_data->peq_coeff_params = NULL; 
    }
    /* Free background coefficients */
    if (p_ext_data->peq_coeff_background)
    {
        pfree(p_ext_data->peq_coeff_background);
        p_ext_data->peq_coeff_background = NULL; 
    }
}

/**
 * \brief Allocate a PEQ data object, which contains (among other things), the
 * PEQ's history buffers. Returns true if succesful, false if failed
 *
 * \param terminal_num : (input) Channel which will be associated with the PEQ object
 * \param p_ext_data : pointer to the extra op data structure
 */
bool peq_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx)
{
   PEQ_OP_DATA *p_ext_data = (PEQ_OP_DATA *)(op_data->extra_op_data);
   peq_channels *peq_chan = (peq_channels*)chan_ptr;
   t_peq_object *p_dobject;

   /* allocate_peq_object */
   p_dobject = xzppmalloc(DH_PEQ_OBJECT_SIZE(PEQ_MAX_STAGES), MALLOC_PREFERENCE_DM2);
   if(p_dobject==NULL)
   {
      return FALSE;
   }

   p_dobject->max_stages = PEQ_MAX_STAGES;
   p_dobject->params_ptr = p_ext_data->peq_coeff_params;
   peq_chan->peq_object  = p_dobject;

   p_ext_data->ReInitFlag = 1;

   return TRUE;
}

/**
 * \brief free PEQ data object. returns true if the object was deleted,
 * false otherwise
 *
 * \param terminal_num : channel to free
 * \param p_ext_data : pointer to the extra op data structure
 */
void peq_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr)
{
   peq_channels *peq_chan  = (peq_channels*)chan_ptr;

   if(peq_chan->peq_object)
   {
      pfree(peq_chan->peq_object);
      peq_chan->peq_object=NULL;
   }

}

/**
 * \brief copy 16-bit payload data to a coefficient params object. FOrmat of payload data conforms to
 * section 3.6.3	OPMSG_PEQ_OVERRIDE_COEFFS message from section 3.6.1 of PEQ capability design document.
 *
 * \param p_coeff_params : pointer to coefficient params object, which is the destination of the copy
 * \param payload : 16-bit payload data which contains the biquad coefficients
 */
#ifndef K32
#define PEQ_OVR_COEF_MSW_SHIFT     (8)
#define PEQ_OVR_COEF_MSW_MASK      (0xFFFF)
#define PEQ_OVR_COEF_LSW_SHIFT     (8)
#define PEQ_OVR_COEF_LSW_MASK      (0xFF00)
#else  //K32
#define PEQ_OVR_COEF_MSW_SHIFT     (16)
#define PEQ_OVR_COEF_MSW_MASK      (0xFFFF)
#define PEQ_OVR_COEF_LSW_SHIFT     (0)
#define PEQ_OVR_COEF_LSW_MASK      (0xFFFF)
#endif //K32

bool peq_wrapper_opmsg_override_coeffs(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    PEQ_OP_DATA *p_ext_data = (PEQ_OP_DATA *)(op_data->extra_op_data);

    /* We received client ID, length and then opmsgID and OBPM params */
     if (  ((unsigned*)message_data)[3] == 0)
     {
         /* If we were overriding, then signal reset of coefficents */
         if(p_ext_data->coeff_override_flag!=0)
         {
            p_ext_data->coeff_override_flag = 0;
            peq_recompute_coeffs(p_ext_data);
         }
         
     }
     else
     {
        unsigned payload_n_stages;
        int base_stage;
        int i;
        unsigned* payload = &((unsigned*)(message_data))[3];
        t_peq_params* p_coeff_params = p_ext_data->peq_coeff_params;

         payload_n_stages = payload[0];
         p_coeff_params->num_stages = payload_n_stages;
         p_coeff_params->gain_exponent = ((payload[1]&0xFFFF)<<16)|(payload[2]&0xFFFF);
         p_coeff_params->gain_mantisa = ((payload[3]&PEQ_OVR_COEF_MSW_MASK)<<PEQ_OVR_COEF_MSW_SHIFT)|
                                         ((payload[4]&PEQ_OVR_COEF_LSW_MASK)>>PEQ_OVR_COEF_LSW_SHIFT);
         for (i=0; i<payload_n_stages; i++)
         {
              base_stage = 5+12*i;
              p_coeff_params->coeffs_scales[i].b2 = ((payload[base_stage+0]&PEQ_OVR_COEF_MSW_MASK)<<PEQ_OVR_COEF_MSW_SHIFT)|
                                                    ((payload[base_stage+1]&PEQ_OVR_COEF_LSW_MASK)>>PEQ_OVR_COEF_LSW_SHIFT);
              p_coeff_params->coeffs_scales[i].b1 = ((payload[base_stage+2]&PEQ_OVR_COEF_MSW_MASK)<<PEQ_OVR_COEF_MSW_SHIFT)|
                                                    ((payload[base_stage+3]&PEQ_OVR_COEF_LSW_MASK)>>PEQ_OVR_COEF_LSW_SHIFT);
              p_coeff_params->coeffs_scales[i].b0 = ((payload[base_stage+4]&PEQ_OVR_COEF_MSW_MASK)<<PEQ_OVR_COEF_MSW_SHIFT)|
                                                    ((payload[base_stage+5]&PEQ_OVR_COEF_LSW_MASK)>>PEQ_OVR_COEF_LSW_SHIFT);
              p_coeff_params->coeffs_scales[i].a2 = ((payload[base_stage+6]&PEQ_OVR_COEF_MSW_MASK)<<PEQ_OVR_COEF_MSW_SHIFT)|
                                                    ((payload[base_stage+7]&PEQ_OVR_COEF_LSW_MASK)>>PEQ_OVR_COEF_LSW_SHIFT);
              p_coeff_params->coeffs_scales[i].a1 = ((payload[base_stage+8]&PEQ_OVR_COEF_MSW_MASK)<<PEQ_OVR_COEF_MSW_SHIFT)|
                                                    ((payload[base_stage+9]&PEQ_OVR_COEF_LSW_MASK)>>PEQ_OVR_COEF_LSW_SHIFT);
              p_coeff_params->coeffs_scales[i].scale = ((payload[base_stage+10]&0xFFFF)<<16)|(payload[base_stage+11]&0xFFFF);
         }

         /* set a flag to indicate that coefficients are now overridden abd abort coefficient update */
         p_ext_data->coeff_override_flag = 1;
         p_ext_data->ParameterLoadStatus = 0;
         p_ext_data->ReInitFlag = 1;
    }
    
    return(TRUE);
}

void peq_coeff_change(PEQ_OP_DATA  *p_ext_data)
{
   if(p_ext_data->ParameterLoadStatus < 0)
   {
       /* Start coefficient load */
       if(p_ext_data->peq_coeff_background == NULL)
       {
           /* Try to allocate background buffer */
           p_ext_data->peq_coeff_background = xzppmalloc(PEQ_PARAMS_OBJECT_SIZE(PEQ_MAX_STAGES), MALLOC_PREFERENCE_DM1);

           /* Failed to allocate.  Just build coefficients in current coefficient buffer */
           if(p_ext_data->peq_coeff_background == NULL)
           {
               while(p_ext_data->ParameterLoadStatus!=0)
               {
                  peq_compute_coefficients(p_ext_data,p_ext_data->peq_coeff_params);
               }
               /* Signal Reset */
               p_ext_data->ReInitFlag = 1;
               return;
           }
       }
   }

   /* Compute coefficients in stages */
   peq_compute_coefficients(p_ext_data,p_ext_data->peq_coeff_background);
   
   /* When done, signal reset of filters */
   if(p_ext_data->ParameterLoadStatus==0)
   {
      /* Switch coefficients */
      pfree(p_ext_data->peq_coeff_params);
      p_ext_data->peq_coeff_params     = p_ext_data->peq_coeff_background;
      p_ext_data->peq_coeff_background = NULL;

      /* Signal Reset */
      p_ext_data->ReInitFlag = 1;
   }
}
