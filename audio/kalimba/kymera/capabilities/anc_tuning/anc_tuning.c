/****************************************************************************
 * Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \file  anc_tuning.c
 * \ingroup  operators
 *
 *  Implementation of the ANC tuning capability. Expects to be connected to
 *  sink and source real audio endpoints on the frontend. 
 *
 *  Supports configuring all ANC hardware related parameters (filter coefficients, 
 *  DC blocker, LPF etc). Parameter information captured in capability XML provides 
 *  a mapping from an external tuning tool to the ANC Hardware via this capability. 
 *
 *  Capability also acts as an audio router allowing any of its 4 sources (2 towards
 *  speaker and 2 towards a backend output) to get its input from one of 8 possible 
 *  sinks (4 from ANC microphones, 2 from monitor microphones from within the ANC H/W and 
 *  2 backend inputs). In a typical use-case, the backend output is USB Tx and backend
 *  input is USB Rx.
 * 
 */
/****************************************************************************
Include Files
*/
#include "anc_tuning_defs.h"
#include "mem_utils/scratch_memory.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "op_msg_helpers.h"
#include "anc_tuning_config.h"
#include "stream/stream_endpoint.h"
#include "stream/stream_for_override.h"
#include "stream/stream_for_opmgr.h"
#include "platform/pl_fractional.h"

/****************************************************************************
Private Constant Definitions
*/
#define ANC_TUNING_ANC_TUNING_VERSION_MINOR 1
#define HAL_ANC_BACKGROUND    1

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define ANC_TUNING_CAP_ID   CAP_ID_DOWNLOAD_ANC_TUNING
#else
#define ANC_TUNING_CAP_ID   CAP_ID_ANC_TUNING
#endif

/****************************************************************************
Private Type Definitions
*/
typedef struct stream_anc_control_param_type {
    ACCMD_ANC_INSTANCE anc_instance;
    uint32 bit_enable;
    uint32 bit_select;
} stream_anc_control_param_type;

typedef struct stream_anc_enable_param_type {
    unsigned con_id;
    uint16 anc_enable_0;
    uint16 anc_enable_1;
    bool (*resp_callback)(unsigned con_id, unsigned status);
} stream_anc_enable_param_type;

/****************************************************************************
Private Function Definitions
*/

/* Capability API handlers */
bool anc_tuning_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
bool anc_tuning_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
bool anc_tuning_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
bool anc_tuning_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
bool anc_tuning_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
bool anc_tuning_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
bool anc_tuning_opmsg_cps_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool anc_tuning_opmsg_cps_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool anc_tuning_opmsg_cps_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool anc_tuning_opmsg_cps_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool anc_tuning_opmsg_cps_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool anc_tuning_opmsg_cps_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool anc_tuning_opmsg_cps_get_psid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool anc_tuning_opmsg_frontend_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing functions */
void anc_tuning_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);
extern void anc_tuning_processing(ANC_TUNING_OP_DATA *p_ext_data, unsigned num_samples);
void anc_tuning_update_processing(OPERATOR_DATA *op_data);
void anc_tuning_channel_setup(ANC_TUNING_OP_DATA *p_ext_data);
void anc_tuning_set_monitor(ACCMD_ANC_INSTANCE anc_instance,unsigned decimation_chain_input);
void anc_tuning_config_connected_to_ep(ENDPOINT *ep, ACCMD_CONFIG_KEY key, uint32 value);
uint16 anc_tuning_set_parameters(ANC_TUNING_OP_DATA *p_ext_data, ACCMD_ANC_INSTANCE instance_id);
void anc_tuning_enable_wrapper(uint16 inst0, uint16 inst1);

/*****************************************************************************
Private Constant Declarations
*/
/** The anc tuning capability function handler table */
const handler_lookup_struct anc_tuning_handler_table =
{
    anc_tuning_create,                         /* OPCMD_CREATE */
    anc_tuning_destroy,                        /* OPCMD_DESTROY */
    base_op_start,                             /* OPCMD_START */
    base_op_stop,                              /* OPCMD_STOP */
    base_op_reset,                             /* OPCMD_RESET */
    anc_tuning_connect,                        /* OPCMD_CONNECT */
    anc_tuning_disconnect,                     /* OPCMD_DISCONNECT */
    anc_tuning_buffer_details,                 /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,                   /* OPCMD_DATA_FORMAT */
    anc_tuning_sched_info                      /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table - this is the set of operator
 * messages that the capability understands and will attempt to service. */
const opmsg_handler_lookup_table_entry anc_tuning_opmsg_handler_table[] =
    {
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,            base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,                       anc_tuning_opmsg_cps_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                        anc_tuning_opmsg_cps_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                      anc_tuning_opmsg_cps_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                        anc_tuning_opmsg_cps_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                        anc_tuning_opmsg_cps_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                          anc_tuning_opmsg_cps_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                 anc_tuning_opmsg_cps_get_psid},
    {OPMSG_ANC_TUNING_ID_FRONTEND_CONFIG,               anc_tuning_opmsg_frontend_config},
    {0, NULL}};

/* Capability data - This is the definition of the capability that Opmgr
 * uses to create the anc tuning capability */
const CAPABILITY_DATA anc_tuning_cap_data =
{
    ANC_TUNING_CAP_ID,                               /* Capability ID */
    ANC_TUNING_anc_tuning_VERSION_MAJOR, 1,          /* Version information - hi and lo parts */
    ANC_TUNING_MAX_SINKS, ANC_TUNING_MAX_SOURCES,    /* Max number of sinks/inputs and sources/outputs */
    &anc_tuning_handler_table,                       /* Pointer to message handler function table */
    anc_tuning_opmsg_handler_table,                  /* Pointer to operator message handler function table */
    anc_tuning_process_data,                         /* Pointer to data processing function */
    0,                                               /* TODO - Processing time information */
    sizeof(ANC_TUNING_OP_DATA)                       /* Size of capability-specific per-instance data */
};

/****************************************************************************
Private Function Declarations
*/

/* ********************************** API functions ************************************* */

/**
 * anc_tuning_sched_info
 * \brief get schedule info for anc_tuning op
 * \param op_data Pointer to the operator instance data
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool anc_tuning_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
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

    resp->block_size = ANC_TUNING_DEFAULT_BLOCK_SIZE;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}

/**
 * \brief Reports the buffer requirements of the requested capability terminal
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool anc_tuning_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* tuning capability doesn't support inplace, metadata etc. */
    return base_op_buffer_details(op_data, message_data, response_id, response_data);
}


/**
 * \brief dummy response callback function.
 *
 */
bool blicenceComp=FALSE;

static bool anc_tuning_dummy_callback(unsigned dummy_con_id, unsigned dummy_status)
{
    blicenceComp = TRUE;
    return TRUE;
}

/**
 * anc_tuning_create
 * \brief creates the anc_tuning op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool anc_tuning_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    ANC_TUNING_OP_DATA *p_ext_data = (ANC_TUNING_OP_DATA *)(op_data->extra_op_data);

    /* Call base_op create initializing operator to NOT_RUNNING state, 
       It also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Initialize extended data for operator.  Assume intialized to zero */
    p_ext_data->ReInitFlag = 1;
    p_ext_data->is_stereo = FALSE;
    p_ext_data->is_two_mic = FALSE;

    /* default sink to source mapping suitable for a plant (or S-path) recording */
    p_ext_data->sources[ANC_TUNING_SOURCE_USB_LEFT].sink_index = ANC_TUNING_SINK_FBMON_LEFT; 
    p_ext_data->sources[ANC_TUNING_SOURCE_USB_RIGHT].sink_index = ANC_TUNING_SINK_FBMON_RIGHT; 
    p_ext_data->sources[ANC_TUNING_SOURCE_DAC_LEFT].sink_index = ANC_TUNING_SINK_USB_LEFT;
    p_ext_data->sources[ANC_TUNING_SOURCE_DAC_RIGHT].sink_index = ANC_TUNING_SINK_USB_RIGHT;
    p_ext_data->fb_mon[0] = 1; /* Use FB signal from FB Mon mux */
    p_ext_data->fb_mon[1] = 0; /* Use FFa signal from FB Mon mux */

    if(!cpsInitParameters(&p_ext_data->params_def,ANC_TUNING_GetDefaults(ANC_TUNING_CAP_ID),(unsigned*)&p_ext_data->anc_tuning_cap_params,sizeof(ANC_TUNING_PARAMETERS)))
    {
       base_op_change_response_status(response_data, STATUS_CMD_FAILED);
       return TRUE;
    }

    op_data->state = OP_NOT_RUNNING;

    /* get licenses */
    anc_tuning_enable_wrapper(0,0);

    return TRUE;
}

/**
 * anc_tuning_connect
 * \brief connecting a sink or source terminal of anc_tuning op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool anc_tuning_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    ANC_TUNING_OP_DATA *p_ext_data = (ANC_TUNING_OP_DATA *)(op_data->extra_op_data);
    unsigned terminal_id = ((unsigned*)message_data)[0]; 
    unsigned  terminal_num = terminal_id & TERMINAL_NUM_MASK;
    unsigned  terminal_mask;
    OVERRIDE_EP_HANDLE  ep_hdl;
    tCbuffer* buffer       = (tCbuffer*)(((uintptr_t *)message_data)[1]);

    if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }

    terminal_mask = (1<<terminal_num);

    if(terminal_id & TERMINAL_SINK_MASK)
    {
       if(terminal_num>=ANC_TUNING_MAX_SINKS)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       }
     
       if(p_ext_data->sinks[terminal_num].buffer!=NULL)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       }      

       ep_hdl = get_overriden_endpoint(op_data->id,terminal_id);

       LOCK_INTERRUPTS;
       p_ext_data->sinks[terminal_num].buffer = buffer;
       p_ext_data->sinks[terminal_num].ep_handle = ep_hdl;
       p_ext_data->connect_change     = TRUE;
       p_ext_data->connected_sinks |= terminal_mask;
       UNLOCK_INTERRUPTS;
    }
    else
    {
       if(terminal_num>=ANC_TUNING_MAX_SOURCES)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       }    
       if(p_ext_data->sources[terminal_num].buffer!=NULL)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       } 

       ep_hdl = get_overriden_endpoint(op_data->id,terminal_id);

       LOCK_INTERRUPTS;
       p_ext_data->sources[terminal_num].buffer = buffer;
       p_ext_data->sources[terminal_num].ep_handle = ep_hdl;
       p_ext_data->connect_change       = TRUE;
       p_ext_data->connected_sources |= terminal_mask;
       UNLOCK_INTERRUPTS;
    }

    anc_tuning_update_processing(op_data);
    return TRUE;

}

/**
 * anc_tuning_disconnect
 * \brief disconnecting a sink or source terminal of anc_tuning op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool anc_tuning_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    ANC_TUNING_OP_DATA *p_ext_data = (ANC_TUNING_OP_DATA *)(op_data->extra_op_data);
    unsigned  terminal_id  = ((unsigned*)message_data)[0];
    unsigned  terminal_num = terminal_id & TERMINAL_NUM_MASK;
    unsigned  terminal_mask;

   if (!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
   {
        return FALSE;
   }

    terminal_mask = ~(1<<terminal_num);

    if(terminal_id & TERMINAL_SINK_MASK)
    {
       if(terminal_num>=ANC_TUNING_MAX_SINKS)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       }

       if(p_ext_data->sinks[terminal_num].buffer==NULL)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       }
       LOCK_INTERRUPTS;
       p_ext_data->sinks[terminal_num].buffer = NULL;
       p_ext_data->sinks[terminal_num].ep_handle = NULL;
       p_ext_data->connect_change       = TRUE;
       p_ext_data->connected_sinks &= terminal_mask;
       UNLOCK_INTERRUPTS;
    }

    else
    {
       if(terminal_num>=ANC_TUNING_MAX_SOURCES)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       }
       if(p_ext_data->sources[terminal_num].buffer==NULL)
       {
           base_op_change_response_status(response_data, STATUS_CMD_FAILED);
           return TRUE;
       }
       LOCK_INTERRUPTS;
       p_ext_data->sources[terminal_num].buffer = NULL;
       p_ext_data->sources[terminal_num].ep_handle = NULL;
       p_ext_data->connect_change       = TRUE;
       p_ext_data->connected_sources &= terminal_mask;
       UNLOCK_INTERRUPTS;

    }

    anc_tuning_update_processing(op_data);
    return TRUE;

}

/**
 * anc_tuning_destroy
 * \brief destroys the anc_tuning op
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool anc_tuning_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* check that we are not trying to destroy a running operator */
    if(op_data->state == OP_RUNNING)
    {
        *response_id = OPCMD_DESTROY;

        /* We can't destroy a running operator. */
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }
    else
    {   /* Nothing to clean up, all allocations from framework */
        /* call base_op destroy that creates and fills response message, too */
        return base_op_destroy(op_data, message_data, response_id, response_data);
    }
 }

/* *********************Operator Message Handle functions ******************************** */

/* OBPM controls for sink to source mapping */
bool anc_tuning_opmsg_cps_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA        *p_ext_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;
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
        
        if(cntrl_id== ANC_TUNING_CONSTANT_SOURCE_ROUTES1_CTRL)
        {
            p_ext_data->sources[ANC_TUNING_SOURCE_USB_LEFT].sink_index = cntrl_value;
        }
        else if(cntrl_id== ANC_TUNING_CONSTANT_SOURCE_ROUTES2_CTRL)
        {
            p_ext_data->sources[ANC_TUNING_SOURCE_USB_RIGHT].sink_index = cntrl_value;
        }
        else if(cntrl_id== ANC_TUNING_CONSTANT_SOURCE_ROUTES3_CTRL)
        {
            p_ext_data->sources[ANC_TUNING_SOURCE_DAC_LEFT].sink_index = cntrl_value;
        }
        else if(cntrl_id== ANC_TUNING_CONSTANT_SOURCE_ROUTES4_CTRL)
        {
            p_ext_data->sources[ANC_TUNING_SOURCE_DAC_RIGHT].sink_index = cntrl_value;
        }
        else if(cntrl_id== ANC_TUNING_CONSTANT_INST_FLAGS0_CTRL)
        {
            p_ext_data->fb_mon[0] = cntrl_value;

        }
        else if(cntrl_id== ANC_TUNING_CONSTANT_INST_FLAGS1_CTRL)
        {
            p_ext_data->fb_mon[1] = cntrl_value;
        }
        else
        {            
            result = OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL;
            break;
        }
    }

    cps_response_set_result(resp_data,result);

    p_ext_data->connect_change  = TRUE;
    anc_tuning_update_processing(op_data);

    return TRUE;
}

bool anc_tuning_opmsg_cps_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA   *op_extra_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->params_def,message_data,resp_length,resp_data);
}

bool anc_tuning_opmsg_cps_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA   *op_extra_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;

    return cpsGetDefaultsMsgHandler(&op_extra_data->params_def,message_data,resp_length,resp_data);
}

bool anc_tuning_opmsg_cps_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA   *op_extra_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;
    bool retval;

    retval = cpsSetParameterMsgHandler(&op_extra_data->params_def,message_data,resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

/* OBPM statistics */
bool anc_tuning_opmsg_cps_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA   *op_extra_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(ANC_TUNING_STATISTICS),&resp))
    {
          return FALSE;
    }

    if(resp)
    {
        unsigned val1,val2;
        val1 = op_extra_data->sources[0].sink_index;
        val2 = op_extra_data->sources[1].sink_index;
        resp = cpsPackWords(&val1,&val2,resp);
        val1 = op_extra_data->sources[2].sink_index;
        val2 = op_extra_data->sources[3].sink_index;
        resp = cpsPackWords(&val1,&val2,resp);
        val1 = op_extra_data->fb_mon[0];
        val2 = op_extra_data->fb_mon[1];
        resp = cpsPackWords(&val1,&val2,resp);
        
        /* Peak statistics */
        val1 = op_extra_data->sources[0].peak;
        val2 = op_extra_data->sources[1].peak;
        resp = cpsPackWords(&val1,&val2,resp);
        op_extra_data->sources[0].peak=0;
        op_extra_data->sources[1].peak=0;
        val1 = op_extra_data->sources[2].peak;
        val2 = op_extra_data->sources[3].peak;
        cpsPackWords(&val1,&val2,resp);
        op_extra_data->sources[2].peak=0;
        op_extra_data->sources[3].peak=0;
    }

    return TRUE;
}

static bool ups_params_anc_tuning(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    ANC_TUNING_OP_DATA   *p_ext_data = (ANC_TUNING_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&p_ext_data->params_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    p_ext_data->ReInitFlag = 1;

    return(TRUE);
}

bool anc_tuning_opmsg_cps_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA   *p_ext_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&p_ext_data->params_def,message_data,resp_length,resp_data);
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(ANC_TUNING_CAP_ID,p_ext_data->params_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_anc_tuning);

    return retval;
}

bool anc_tuning_opmsg_cps_get_psid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA   *op_extra_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;

    return cpsGetUcidMsgHandler(&op_extra_data->params_def,ANC_TUNING_CAP_ID,message_data,resp_length,resp_data);
}

bool anc_tuning_opmsg_frontend_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    ANC_TUNING_OP_DATA   *p_ext_data = (ANC_TUNING_OP_DATA*)op_data->extra_op_data;

    if(op_data->state == OP_RUNNING)
    {
       return FALSE;
    }

    p_ext_data->is_stereo = OPMSG_FIELD_GET(message_data,OPMSG_ANC_TUNING_FRONTEND_CONFIG,IS_STEREO);
    p_ext_data->is_two_mic = OPMSG_FIELD_GET(message_data,OPMSG_ANC_TUNING_FRONTEND_CONFIG,IS_TWO_MIC);

    p_ext_data->connect_change  = TRUE;
    anc_tuning_update_processing(op_data);

    return TRUE;
}

/* *************************** Opearor Data Processsing ********************************** */

/**
 * anc_tuning_process_data
 * \brief process function when anc_tuning operator is kicked
 *
 * \param op_data Pointer to the operator instance data.
 * \param touched Location to write which terminals touched at this kick
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
void anc_tuning_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    ANC_TUNING_OP_DATA *p_ext_data = (ANC_TUNING_OP_DATA *)(op_data->extra_op_data);
    anc_source_t *lp_source=NULL;
    anc_sink_t   *lp_sink=NULL;
    unsigned      available, amount;

    if (p_ext_data->connect_change)
    {
        p_ext_data->connect_change = FALSE;
        anc_tuning_channel_setup(p_ext_data);
    }

    if(!p_ext_data->first_sink || !p_ext_data->first_source)
    {
        return ;
    }

    /* when a parameter is changed, disable ANC, change parameter
       and then re-enable ANC. */
    if(p_ext_data->ReInitFlag && blicenceComp)
    {
        /* Disable ANC. A value of zero ensures ANC is completely shut off */
        uint16 inst0_ena=0,inst1_ena=0; 

        p_ext_data->ReInitFlag = 0;

        /* disable ANC */
        anc_tuning_enable_wrapper(inst0_ena,inst1_ena);
        L2_DBG_MSG2("anc_tuning_cap: disabling top level anc with %d and %d \n",inst0_ena,inst1_ena);

        inst0_ena = anc_tuning_set_parameters(p_ext_data,ACCMD_ANC_INSTANCE_ANC0_ID); 
        if(p_ext_data->is_stereo)
        {
            inst1_ena = anc_tuning_set_parameters(p_ext_data,ACCMD_ANC_INSTANCE_ANC1_ID); 
        }

        /* enable ANC */
        anc_tuning_enable_wrapper(inst0_ena,inst1_ena);
        L2_DBG_MSG2("anc_tuning_cap: enabling top level anc with %d and %d \n",inst0_ena,inst1_ena);
    }
    
    available = MAXINT;
    lp_source=p_ext_data->first_source;

    do
    {
        /* Minimum Space */
        amount = cbuffer_calc_amount_space_in_words(lp_source->buffer);
        if(amount<ANC_TUNING_DEFAULT_BLOCK_SIZE)
        {
            return;
        }
        if(available>amount)
        {
            available = amount;
        }
        lp_source = lp_source->next;
    }while(lp_source);
    
    lp_sink = p_ext_data->first_sink;

    do
    {
        /* Minimum Data */
        amount = cbuffer_calc_amount_data_in_words(lp_sink->buffer);
        if(amount<ANC_TUNING_DEFAULT_BLOCK_SIZE)
        {
            return;
        }
        if(available>amount)
        {
            available = amount;
        }
        lp_sink = lp_sink->next;
    }while(lp_sink);

    /* call asm processing */
    anc_tuning_processing(p_ext_data,available);

    /* touched output */
    touched->sources = p_ext_data->connected_sources;
    /* touched input */
    touched->sinks = p_ext_data->connected_sinks;

}

/* ********************************** Misc functions ************************************* */

void anc_tuning_update_processing(OPERATOR_DATA *op_data)
{
    if(op_data->state==OP_RUNNING)
    {
        /* Raise a bg int to process */
        opmgr_kick_operator(op_data);
    }
}

void anc_tuning_channel_setup(ANC_TUNING_OP_DATA *p_ext_data)
{
    unsigned valid_sinks=0;
    unsigned i,temp_mask;
    anc_source_t *lp_source=NULL;
    anc_sink_t   *lp_sink=NULL;

    p_ext_data->first_sink=NULL;
    p_ext_data->first_source=NULL;

    /* USB - Input */
    if((p_ext_data->connected_sinks&USB_SINK_MASK)==USB_SINK_MASK)
    {
        valid_sinks |= USB_SINK_MASK;
    }
    /* Left Stream */
    temp_mask = (p_ext_data->is_two_mic) ? ANC_SINK_MASK_2MIC : ANC_SINK_MASK_1MIC;
    if((p_ext_data->connected_sinks&temp_mask)==temp_mask)
    {
        valid_sinks |= temp_mask;
    }
    /* Right Stream */
    temp_mask <<= 1;
    if((p_ext_data->connected_sinks&temp_mask)==temp_mask)
    {
        valid_sinks |= temp_mask;
    }
    
    if(valid_sinks ==0)
    {
        return;
    }

    temp_mask = p_ext_data->is_stereo ? 0xF : 0x7;    
    if((temp_mask & p_ext_data->connected_sources)!=temp_mask)
    {
        return;
    }

    /* build sink list */
    for (i=0;i<ANC_TUNING_MAX_SINKS;i++)
    {
        if(valid_sinks & (1<<i))
        {
            anc_sink_t *lp_ptr=&p_ext_data->sinks[i];

            lp_ptr->next = lp_sink;
            lp_sink = lp_ptr;
        }
    }

    /* build source list */
    for (i=0;i<ANC_TUNING_MAX_SOURCES;i++)
    {
        anc_source_t *lp_ptr=&p_ext_data->sources[i];

        if(lp_ptr->buffer)
        {
            lp_ptr->next = lp_source;
            lp_source = lp_ptr;

            /* link a sink with the source */
            if(valid_sinks & (1<<lp_ptr->sink_index))
            {
                lp_ptr->sink = &p_ext_data->sinks[lp_ptr->sink_index];
            }
            else
            {
                lp_ptr->sink = NULL;
            }
        }
    }
 
    p_ext_data->first_source=lp_source;    
    p_ext_data->first_sink=lp_sink;

}

void anc_tuning_config_connected_to_ep(ENDPOINT *ep, ACCMD_CONFIG_KEY key, uint32 value)
{
    bool config_result=FALSE;

    if(ep)
    {
        config_result = stream_configure_connected_to_endpoint(ep,key,value);
        L2_DBG_MSG4("anc_tuning_cap: stream_config_connected_to(): ep:0x%x  key:0x%x  value:0x%x  result:%d\n",ep,key,value,config_result);
    }
    else
    {
        return;
    }
}


/**
 * anc_tuning_set_monitor
 * \brief appropriate decimation chain based on digital or analog mic for FB mon
 *
 * \param ANC instance
 * \param decimation chain number
 *
 * \return None
 */

void anc_tuning_set_monitor(ACCMD_ANC_INSTANCE anc_instance,unsigned decimation_chain_input)
{  
    stream_anc_control_param_type params;
    params.anc_instance = anc_instance;

#if defined (NO_ANC_TUNING_PATCHES)
    switch(decimation_chain_input)
    {
        case 3:
            params.bit_select = ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT1_SEL_MASK;
            stream_anc_set_anc_tune(anc_instance, params.bit_enable, params.bit_select);
            break;
        case 2:
            params.bit_select = ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT0_SEL_MASK;
            stream_anc_set_anc_tune(anc_instance, params.bit_enable, params.bit_select);
            break;
        case 1:
            params.bit_select = ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT1_SEL_MASK;
            stream_anc_set_anc_tune(anc_instance, params.bit_enable, params.bit_select);
           break;
        case 0:
            params.bit_select = ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT0_SEL_MASK;
            stream_anc_set_anc_tune(anc_instance, params.bit_enable, params.bit_select);
           break;
        default:
            params.bit_select = 0;
            params.bit_enable = 0;
            break;
    }

#else

    switch(decimation_chain_input)
    {
        case 3:
            params.bit_select = ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT1_SEL_MASK;
            break;
        case 2:
            params.bit_select = ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT0_SEL_MASK;
            break;
        case 1:
            params.bit_select = ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT1_SEL_MASK;
            break;
        case 0:
            params.bit_select = ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT0_SEL_MASK | ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT1_SEL_MASK;
            params.bit_enable = ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT0_SEL_MASK;
            break;
        default:
            params.bit_select = 0;
            params.bit_enable = 0;
            break;
    }

    stream_anc_user2((void*)&params);

#endif
}

void anc_tuning_enable_wrapper(uint16 inst0, uint16 inst1)
{
#if defined (NO_ANC_TUNING_PATCHES)
        stream_anc_enable_wrapper(0,inst0,inst1,anc_tuning_dummy_callback);
#else
        stream_anc_enable_param_type  params;

        params.anc_enable_0 = inst0;
        params.anc_enable_1 = inst1;
        params.resp_callback = anc_tuning_dummy_callback;

        stream_anc_user1((void*)&params);

#endif
}

/**
 * anc_tuning_set_parameters
 * \brief Set ANC parameters for both ANC instances 
 *
 * \param Operator Extra data
 * \param ANC instance number
 *
 * \return Bitfields specifying which Filter paths are enabled (=1) and disabled (=0)
 */


uint16 anc_tuning_set_parameters(ANC_TUNING_OP_DATA *p_ext_data, ACCMD_ANC_INSTANCE instance_id)
{
    ENDPOINT *ep_ffa=NULL,*ep_ffb=NULL,*ep_fb=NULL;
    unsigned i,type,inst_off = instance_id - ACCMD_ANC_INSTANCE_ANC0_ID;
    ANC_INST_PARAMS *anc_inst_ptr;
    uint16 anc_enable_flag = 0;
    bool is_two_mic,is_ffa_en,is_ffb_en,is_fb_en,is_ff_out_en;

    if(inst_off)
    {
        anc_inst_ptr = (ANC_INST_PARAMS*)(&p_ext_data->anc_tuning_cap_params.OFFSET_ANC_USECASE_R);
    }
    else
    {
        anc_inst_ptr = (ANC_INST_PARAMS*)(&p_ext_data->anc_tuning_cap_params.OFFSET_ANC_USECASE_L);
    }

    /* setup config flags */
    is_two_mic   = p_ext_data->is_two_mic;
    is_ffa_en    = anc_inst_ptr->OFFSET_FF_A_ENABLE;
    is_ffb_en    = anc_inst_ptr->OFFSET_FF_B_ENABLE;
    is_fb_en     = anc_inst_ptr->OFFSET_FB_ENABLE;
    is_ff_out_en = anc_inst_ptr->OFFSET_FF_OUT_ENABLE;

    if(!is_two_mic)
    {
        L2_DBG_MSG("anc_tuning_cap: enable more microphones to use FFb filter \n");
        is_ffb_en = FALSE;
    }

    /* setup enable flag */
    anc_enable_flag = (uint16)((is_ffa_en << 0)
       |(is_ffb_en << 1)
       |(is_fb_en << 2)
       |(is_ff_out_en << 3)); 

    L2_DBG_MSG1("anc_tuning_cap: anc enable flag has value %d \n",anc_enable_flag);

    /* ANC OFF - exit */
    if(!is_ff_out_en)
    {
        L2_DBG_MSG("anc_tuning_cap: ff out not enabled, so ANC OFF\n");
        return 0;
    }

    /* Get FB mon endpoints by checking the endpoint connected to sink terminal 2,3 */
    ENDPOINT *ep_fb_mon=NULL;
    ep_fb_mon = (ENDPOINT *)p_ext_data->sinks[ANC_TUNING_SINK_FBMON_LEFT+inst_off].ep_handle;

    if(ep_fb_mon)
    {
       ep_fb_mon = ep_fb_mon->connected_to;
    }


    if(ep_fb_mon)
    {
        type = ((ep_fb_mon->key)>>8)& 0xff;
      
        /* enable sigma delta modulator on the tuning output */
        uint32 en_and_mask;
        en_and_mask = (ACCMD_ANC_CONTROL_FB_TUNE_DSM_EN_MASK << ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT);
        en_and_mask |= ACCMD_ANC_CONTROL_FB_TUNE_DSM_EN_MASK;
        stream_anc_set_anc_control(instance_id,en_and_mask);

        /* If ANC uses analog mic, FB mon uses digital mic instance 1. If ANC uses digital mic, 
           FB mon uses instance 0. FB mon mic's are only setup so their decimators can be re purposed 
           to stream data from the ANC path.  
        */
 
        if(type == ACCMD_STREAM_DEVICE_DIGITAL_MIC)
        {
            /* MON1 = DIG_MIC_INST1_LEFT, MON2 = DIG_MIC_INST1_RIGHT */
            if(inst_off)
            {
                /* LOW_LATENCY_ANC_DECIMATION_INPUT_EN_3_FBTUNEOUT1 */
                anc_tuning_set_monitor(instance_id,3);
                
            }
            else
            {
                /* LOW_LATENCY_ANC_DECIMATION_INPUT_EN_2_FBTUNEOUT0 */
                anc_tuning_set_monitor(instance_id,2);
            }
        }
        else
        {
            /* MON1 = ADC_LEFT,   MON2=ADC_RIGHT */
            if(inst_off)
            {
                /* LOW_LATENCY_ANC_DECIMATION_INPUT_EN_1_FBTUNEOUT1 */
                anc_tuning_set_monitor(instance_id,1);
            }
            else
            {
                /* LOW_LATENCY_ANC_DECIMATION_INPUT_EN_0_FBTUNEOUT0 */
                anc_tuning_set_monitor(instance_id,0);
            }
       }
    }

    /* Get FFa endpoints by checking the endpoint connected to terminal sink 4,5 */
    ep_ffa = (ENDPOINT *)p_ext_data->sinks[ANC_TUNING_SINK_MIC1_LEFT+inst_off].ep_handle;

    /* FB mon set control. Now that the decimators are re purposed, associate a specific signal from within
       the ANC block for streaming */

    if(ep_ffa)
    {
        uint32 en_and_mask;
        
        /* FB mon set control to 1 - i.e FB path IIR input goes to FB MON. 0 - i.e FFa path IIR input goes to FB MON. */
        en_and_mask = (ACCMD_ANC_CONTROL_FB_ON_FBMON_IS_TRUE_MASK << ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT);
        if(p_ext_data->fb_mon[inst_off])
        {
            en_and_mask |= ACCMD_ANC_CONTROL_FB_ON_FBMON_IS_TRUE_MASK;
        }
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_CONTROL,en_and_mask);

        /* Set FFa mic gain if endpoint exists */
        set_override_ep_gain(ep_ffa,anc_inst_ptr->OFFSET_FF_A_FE_GAIN);
	
    }

    /* Get FFb endpoints by checking the endpoint connected to sink terminal 6,7 */
    ep_ffb = (ENDPOINT *)p_ext_data->sinks[ANC_TUNING_SINK_MIC2_LEFT+inst_off].ep_handle;

    /* Get FB endpoints by checking the endpoint connected to source terminal 2,3 */
    ep_fb  = (ENDPOINT *)p_ext_data->sources[ANC_TUNING_SOURCE_DAC_LEFT+inst_off].ep_handle;

    L2_DBG_MSG4("anc_tuning_cap: eps are ffa:0x%x  ffb:0x%x  fb:0x%x  fb_mon:0x%x\n",ep_ffa,ep_ffb,ep_fb,ep_fb_mon);

    /* Remove endpoints from instance. Remove endpoints from paths done internally. Done for each paraneter change */
    anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE,ACCMD_ANC_INSTANCE_NONE_ID);
    if(ep_ffb)
    {
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE,ACCMD_ANC_INSTANCE_NONE_ID);

        /* Set FFb mic gain if endpoint exists */
        set_override_ep_gain(ep_ffb,anc_inst_ptr->OFFSET_FF_B_FE_GAIN);

    }
    anc_tuning_config_connected_to_ep(ep_fb,ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE,ACCMD_ANC_INSTANCE_NONE_ID);

    /* Associate endpoints with ANC instances */
    anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE,instance_id);
    if(is_ffb_en)
    {
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE,instance_id);
    }
    anc_tuning_config_connected_to_ep(ep_fb,ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE,instance_id);  

    /* Set speaker gain if endpoint exists */
    set_override_ep_gain(ep_fb,anc_inst_ptr->OFFSET_SPKR_RECEIVER_PA_GAIN);
    
    /* Associate endpoints with filter paths */
    if(is_ffa_en)
    {
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_INPUT,ACCMD_ANC_PATH_FFA_ID);
    }
    if(is_ffb_en)
    {
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_INPUT, ACCMD_ANC_PATH_FFB_ID);
    }

    /* configure ffa parameters. Uses two methods to reach the ANC hardware: 
       configure a connected_to endpoint and using a shim layer
       that provides an interface between the ANC hardware and this capability 
    */
    if(is_ffa_en)    
    {

        uint32 ffa_dmic_x2_mask;
        ffa_dmic_x2_mask = (ACCMD_ANC_CONTROL_DMIC_X2_A_SEL_MASK << ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT);
        if(anc_inst_ptr->OFFSET_DMIC_X2_FF_A_ENABLE == 1)
        {
            ffa_dmic_x2_mask |= ACCMD_ANC_CONTROL_DMIC_X2_A_SEL_MASK;
        }
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_CONTROL,ffa_dmic_x2_mask);
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_FFA_DC_FILTER_ENABLE,anc_inst_ptr->OFFSET_FF_A_DCFLT_ENABLE); 
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_FFA_DC_FILTER_SHIFT,anc_inst_ptr->OFFSET_FF_A_DCFLT_SHIFT);
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_FFA_GAIN,anc_inst_ptr->OFFSET_ANC_FF_A_GAIN);
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_FFA_GAIN_SHIFT,anc_inst_ptr->OFFSET_ANC_FF_A_SHIFT); 

        /* Configure smLPF parameters */
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_SM_LPF_FILTER_ENABLE,anc_inst_ptr->OFFSET_SMLPF_ENABLE);
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_SM_LPF_FILTER_SHIFT,anc_inst_ptr->OFFSET_SM_LPF_SHIFT);

        /* set LPF and IIR filter coefficients for FFa path */
        stream_anc_set_anc_lpf_coeffs(instance_id,ACCMD_ANC_PATH_FFA_ID,(uint16)(anc_inst_ptr->OFFSET_ANC_FF_A_LPF_SHIFT0),(uint16)(anc_inst_ptr->OFFSET_ANC_FF_A_LPF_SHIFT1));

        for (i=0;i<ACCMD_ANC_IIR_FILTER_FFA_NUM_COEFFS;i++)
        {
            /* capability XML format is signed Q2.N. ANC HAL is expecting signed Q2.9 uint16's. However, filter design utility is expecting a Q19.13 
               so >>20 and <<16 (rounded instead of floored) for coefficients to end up being shaped correctly for the ANC hardware 
            */
            p_ext_data->coeffs[i] = (uint16)frac_mult((anc_inst_ptr->OFFSET_ANC_FF_A_COEFF[i]),FRACTIONAL(1.0/16.0)); 
        }
        stream_anc_set_anc_iir_coeffs(instance_id,ACCMD_ANC_PATH_FFA_ID,ACCMD_ANC_IIR_FILTER_FFA_NUM_COEFFS,p_ext_data->coeffs);
        stream_anc_select_active_iir_coeffs(instance_id,HAL_ANC_BACKGROUND);
        stream_anc_update_background_iir_coeffs(instance_id); 
    }

    /* configure ffb parameters. Uses two methods to reach the ANC hardware: 
       configure a connected_to endpoint and using a shim layer
       that provides an interface between the ANC hardware and this capability 
    */
    if(is_ffb_en)
    {

        uint32 ffb_dmic_x2_mask;
        ffb_dmic_x2_mask = (ACCMD_ANC_CONTROL_DMIC_X2_B_SEL_MASK << ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT);
        if(anc_inst_ptr->OFFSET_DMIC_X2_FF_B_ENABLE == 1)
        {
            ffb_dmic_x2_mask |= ACCMD_ANC_CONTROL_DMIC_X2_B_SEL_MASK;
        }
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_CONTROL,ffb_dmic_x2_mask);   
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_FFB_DC_FILTER_ENABLE,anc_inst_ptr->OFFSET_FF_B_DCFLT_ENABLE); 
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_FFB_DC_FILTER_SHIFT,anc_inst_ptr->OFFSET_FF_B_DCFLT_SHIFT); 
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_FFB_GAIN,anc_inst_ptr->OFFSET_ANC_FF_B_GAIN); 
        anc_tuning_config_connected_to_ep(ep_ffb,ACCMD_CONFIG_KEY_STREAM_ANC_FFB_GAIN_SHIFT,anc_inst_ptr->OFFSET_ANC_FF_B_SHIFT);

        /* set LPF and IIR filter coefficients for FFb path */
        stream_anc_set_anc_lpf_coeffs(instance_id, ACCMD_ANC_PATH_FFB_ID,(uint16)(anc_inst_ptr->OFFSET_ANC_FF_B_LPF_SHIFT0),(uint16)(anc_inst_ptr->OFFSET_ANC_FF_B_LPF_SHIFT1)); 

        for (i=0;i<ACCMD_ANC_IIR_FILTER_FFB_NUM_COEFFS;i++)
        {
            /* capability XML format is signed Q2.N. ANC HAL is expecting signed Q2.9 uint16's. However, filter design utility is expecting a Q19.13 
               so >>20 and <<16 (rounded instead of floored) for coefficients to end up being shaped correctly for the ANC hardware 
            */
            p_ext_data->coeffs[i] = (uint16)frac_mult((anc_inst_ptr->OFFSET_ANC_FF_B_COEFF[i]),FRACTIONAL(1.0/16.0)); 
        }
        stream_anc_set_anc_iir_coeffs(instance_id,ACCMD_ANC_PATH_FFB_ID,ACCMD_ANC_IIR_FILTER_FFB_NUM_COEFFS,p_ext_data->coeffs);
        stream_anc_select_active_iir_coeffs(instance_id,HAL_ANC_BACKGROUND);
        stream_anc_update_background_iir_coeffs(instance_id);
    }
    
    /* configure FB parameters. Uses two methods to reach the ANC hardware: 
       configure a connected_to endpoint and using a shim layer
       that provides an interface between the ANC hardware and this capability 
    */
    if(is_fb_en)
    {
        /* Really wants a DAC sink association, but the audio layer is expecting an ADC source association on the same instance */
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_FB_GAIN,anc_inst_ptr->OFFSET_ANC_FB_GAIN);
        anc_tuning_config_connected_to_ep(ep_ffa,ACCMD_CONFIG_KEY_STREAM_ANC_FB_GAIN_SHIFT,anc_inst_ptr->OFFSET_ANC_FB_SHIFT); 
       
        /* set LPF and IIR filter coefficients for FB path */
        stream_anc_set_anc_lpf_coeffs(instance_id,ACCMD_ANC_PATH_FB_ID,(uint16)(anc_inst_ptr->OFFSET_ANC_FB_LPF_SHIFT0),(uint16)(anc_inst_ptr->OFFSET_ANC_FB_LPF_SHIFT1));

        for (i=0;i<ACCMD_ANC_IIR_FILTER_FB_NUM_COEFFS;i++)
        {
            /* capability XML format is signed Q2.N. ANC HAL is expecting signed Q2.9 uint16's. However, filter design utility is expecting a Q19.13 
               so >>20 and <<16 (rounded instead of floored) for coefficients to end up being shaped correctly for the ANC hardware 
            */
            p_ext_data->coeffs[i] = (uint16)frac_mult((anc_inst_ptr->OFFSET_ANC_FB_COEFF[i]),FRACTIONAL(1.0/16.0)); 
        }

        stream_anc_set_anc_iir_coeffs(instance_id,ACCMD_ANC_PATH_FB_ID,ACCMD_ANC_IIR_FILTER_FB_NUM_COEFFS,p_ext_data->coeffs);
        stream_anc_select_active_iir_coeffs(instance_id,HAL_ANC_BACKGROUND);
        stream_anc_update_background_iir_coeffs(instance_id);

    }

    return anc_enable_flag;
}









