/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  dbe_wrapper.c
 * \ingroup  capabilities
 *
 *  dbe operator
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
#include "dbe_wrapper.h"
#include "dbe_c.h"

#include "patch/patch.h"

//#include "dbe_gen_c.h"

/****************************************************************************
Private Constant Definitions
*/
/** The terminal ID of the input terminal */
#define INPUT_TERMINAL_ID (0 | TERMINAL_SINK_MASK)
/** The terminal ID of the output terminal */
#define OUTPUT_TERMINAL_ID (0)
#define DBE_DBE_VERSION_MINOR 2

/****************************************************************************
Private Type Definitions
*/
#ifdef CAPABILITY_DOWNLOAD_BUILD
#define DBE_CAP_ID                 CAP_ID_DOWNLOAD_DBE
#define DBE_FULLBAND_CAP_ID        CAP_ID_DOWNLOAD_DBE_FULLBAND
#define DBE_FULLBAND_BASSOUTCAP_ID CAP_ID_DOWNLOAD_DBE_FULLBAND_BASSOUT
#else
#define DBE_CAP_ID                 CAP_ID_DBE
#define DBE_FULLBAND_CAP_ID        CAP_ID_DBE_FULLBAND
#define DBE_FULLBAND_BASSOUTCAP_ID CAP_ID_DBE_FULLBAND_BASSOUT
#endif
/*****************************************************************************
Private Constant Declarations
*/
/** The dbe capability function handler table */
const handler_lookup_struct dbe_wrapper_handler_table =
{
    dbe_wrapper_create,            /* OPCMD_CREATE */
    dbe_wrapper_destroy,           /* OPCMD_DESTROY */
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
const opmsg_handler_lookup_table_entry dbe_wrapper_opmsg_handler_table[] =
    {
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,             base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,                        dbe_wrapper_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                         dbe_wrapper_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                       dbe_wrapper_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                         dbe_wrapper_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                         dbe_wrapper_opmsg_obpm_get_status},
    {OPMSG_COMMON_ID_SET_UCID,                           dbe_wrapper_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                  dbe_wrapper_opmsg_get_ps_id},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                       dbe_wrapper_opmsg_set_sample_rate},
    {OPMSG_COMMON_SET_DATA_STREAM_BASED,                 multi_channel_stream_based},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,                    multi_channel_opmsg_set_buffer_size},
#ifdef INSTALL_METADATA
    {OPMSG_COMMON_SET_METADATA_DELAY,                    dbe_wrapper_opmsg_set_metadata_delay},
#endif
    {0, NULL}};

const CAPABILITY_DATA dbe_cap_data =
{
    DBE_CAP_ID,                                       /* Capability ID */
    DBE_DBE_VERSION_MAJOR, DBE_DBE_VERSION_MINOR,     /* Version information - hi and lo parts */
    DBE_CAP_MAX_CHANNELS, DBE_CAP_MAX_CHANNELS,       /* Max number of sinks/inputs and sources/outputs */
    &dbe_wrapper_handler_table,                       /* Pointer to message handler function table */
    dbe_wrapper_opmsg_handler_table,                  /* Pointer to operator message handler function table */
    dbe_wrapper_process_data,                         /* Pointer to data processing function */
    0,                                                /* TODO - Processing time information */
    sizeof(DBE_OP_DATA)                               /* Size of capability-specific per-instance data */
};

const CAPABILITY_DATA dbe_fullband_cap_data =
{
    DBE_FULLBAND_CAP_ID,                              /* Capability ID */
    DBE_DBE_VERSION_MAJOR, DBE_DBE_VERSION_MINOR,     /* Version information - hi and lo parts */
    DBE_CAP_MAX_CHANNELS, DBE_CAP_MAX_CHANNELS,       /* Max number of sinks/inputs and sources/outputs */
    &dbe_wrapper_handler_table,                       /* Pointer to message handler function table */
    dbe_wrapper_opmsg_handler_table,                  /* Pointer to operator message handler function table */
    dbe_wrapper_process_data,                         /* Pointer to data processing function */
    0,                                                /* TODO - Processing time information */
    sizeof(DBE_OP_DATA)                               /* Size of capability-specific per-instance data */
};

const CAPABILITY_DATA dbe_fullband_bassout_cap_data =
{
    DBE_FULLBAND_BASSOUTCAP_ID,                       /* Capability ID */
    DBE_DBE_VERSION_MAJOR, DBE_DBE_VERSION_MINOR,     /* Version information - hi and lo parts */
    DBE_CAP_MAX_CHANNELS, DBE_CAP_MAX_CHANNELS,       /* Max number of sinks/inputs and sources/outputs */
    &dbe_wrapper_handler_table,                       /* Pointer to message handler function table */
    dbe_wrapper_opmsg_handler_table,                  /* Pointer to operator message handler function table */
    dbe_wrapper_process_data,                         /* Pointer to data processing function */
    0,                                                /* TODO - Processing time information */
    sizeof(DBE_OP_DATA)                               /* Size of capability-specific per-instance data */
};

/****************************************************************************
Private Function Declarations
*/

/* ********************************** API functions ************************************* */

bool dbe_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    DBE_OP_DATA *p_ext_data = (DBE_OP_DATA *)(op_data->extra_op_data);

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Allocate channels, in-place, no hot connect */
    if( !multi_channel_create(op_data,(MULTI_INPLACE_FLAG|MULTI_METADATA_FLAG),sizeof(dbe_channels)) )
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }
    multi_channel_set_callbacks(op_data,dbe_channel_create,dbe_channel_destroy);

    /* allocate memory */
    if ( DBE_CAP_Create(p_ext_data) == FALSE )
    {
        DBE_CAP_Destroy(op_data);
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* Initialize extended data for operator.  Assume intialized to zero*/
    p_ext_data->ReInitFlag = 1;
    p_ext_data->Host_mode = DBE_SYSMODE_FULL;
    p_ext_data->Cur_mode  = DBE_SYSMODE_FULL;

    p_ext_data->sample_rate = 48000;

    switch(op_data->cap_data->id)
    {
       case DBE_CAP_ID:
          // bypass crossover and base output mixer
          p_ext_data->cap_config=DBE_CONFIG_BYPASS_XOVER|DBE_CONFIG_BYPASS_BASS_OUTPUT_MIX;
          break;
       case DBE_FULLBAND_CAP_ID:
          // do not bypass crossover or base output mixer
          p_ext_data->cap_config=0;
          break;
       case DBE_FULLBAND_BASSOUTCAP_ID:
          // do not bypass crossover.  Bypass base output mixer
          p_ext_data->cap_config=DBE_CONFIG_BYPASS_BASS_OUTPUT_MIX;
          break;
    }

    if(!cpsInitParameters(&p_ext_data->parms_def,(unsigned*)DBE_GetDefaults(op_data->cap_data->id),(unsigned*)&p_ext_data->dbe_cap_params,sizeof(DBE_PARAMETERS)))
    {
       base_op_change_response_status(response_data, STATUS_CMD_FAILED);
       DBE_CAP_Destroy(op_data);
       return TRUE;
    }

    op_data->state = OP_NOT_RUNNING;

    return TRUE;
}

bool dbe_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* call base_op destroy that creates and fills response message, too */
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        return(FALSE);
    }
    /* set internal capability state variable to "not_created" */
    DBE_CAP_Destroy(op_data);
    return TRUE;
}


/* ************************************* Data processing-related functions and wrappers **********************************/
static unsigned adjust_data(MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned amount)
{
   /* Limit to even amounts */
   if(amount&0x1)
   {
      amount--;
   }
   return amount;
}

void dbe_wrapper_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
   DBE_OP_DATA *p_ext_data = (DBE_OP_DATA *)(op_data->extra_op_data);
   dbe_channels *channels  = (dbe_channels*)multi_channel_first_active_channel(op_data);
   int samples_to_process;

   patch_fn(dbe_wrapper_process_data);
   /* Check status of terminals */
   samples_to_process = multi_channel_check_buffers_adjusted(op_data,touched,adjust_data,adjust_data);
   
   
   if(p_ext_data->ReInitFlag)
   {
      dbe_initialize(p_ext_data,channels);
   }

   /* call ASM processing function, returns non-zero if processing occurred */
   if(samples_to_process>0)
   {
#ifdef INSTALL_METADATA
       multi_channel_metadata_propagate(op_data,samples_to_process);
#endif
       dbe_processing(p_ext_data,samples_to_process,channels);
   }
}





/* **************************** Operator message handlers ******************************** */

bool dbe_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA        *op_extra_data = (DBE_OP_DATA*)op_data->extra_op_data;
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
        if (cntrl_value >= DBE_SYSMODE_MAX_MODES)
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
            op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ? 0 : DBE_CONTROL_MODE_OVERRIDE;
            op_extra_data->Obpm_mode = cntrl_value;
        }
    }

    /* Set current mode */
    if(op_extra_data->Ovr_Control&DBE_CONTROL_MODE_OVERRIDE)
    {
       op_extra_data->Cur_mode=op_extra_data->Obpm_mode;
    }
    else
    {
       op_extra_data->Cur_mode=op_extra_data->Host_mode;
    }

    cps_response_set_result(resp_data,result);

    /* Set the Reinit flag after setting the paramters */
    if (result == OPMSG_RESULT_STATES_NORMAL_STATE)
    {
        op_extra_data->ReInitFlag = 1;
    }
    return TRUE;
}

bool dbe_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA   *op_extra_data = (DBE_OP_DATA*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool dbe_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA   *op_extra_data = (DBE_OP_DATA*)op_data->extra_op_data;

    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool dbe_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA   *op_extra_data = (DBE_OP_DATA*)op_data->extra_op_data;
    bool retval;

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

bool dbe_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA   *op_extra_data = (DBE_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(DBE_STATISTICS),&resp))
    {
         return FALSE;
    }

    if(resp)
    {
        OPSTATE_INTERNAL op_state_stat = multi_channel_active_channels(op_data) ? OPSTATE_INTERNAL_CONNECTED : OPSTATE_INTERNAL_READY;
        unsigned comp_config=0;

        resp = cpsPackWords(&op_extra_data->Cur_mode,&op_extra_data->Ovr_Control ,resp);
        resp = cpsPackWords(&comp_config,(unsigned*)&op_state_stat ,resp);
        cpsPackWords((unsigned*)&op_state_stat,NULL ,resp);
    }

    return TRUE;
}



bool dbe_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA   *op_extra_data = (DBE_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_data->cap_data->id, op_extra_data->parms_def.ucid, OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_dbe);

    return retval;
}

bool dbe_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA   *op_extra_data = (DBE_OP_DATA*)op_data->extra_op_data;

    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,DBE_CAP_ID,message_data,resp_length,resp_data);
}

bool dbe_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    DBE_OP_DATA *p_ext_data = (DBE_OP_DATA *)(op_data->extra_op_data);
    unsigned sample_rate;
    L4_DBG_MSG("dbe_wrapper_set_samplerate  \n");

    /* Only supports sample rates: 32kHz, 44.1 kHz, 48 kHz, 88.2 kHz, and 96 kHz) */
    sample_rate = 25 * ((unsigned*)message_data)[3];
    switch(sample_rate)
    {
    case 32000:
    case 44100:
    case 48000:
    case 88200:
    case 96000:
         p_ext_data->sample_rate = sample_rate;
         p_ext_data->ReInitFlag = 1;
         break;
    default:
        return FALSE;
    }
    return(TRUE);
}

#ifdef INSTALL_METADATA
bool dbe_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    MULTI_CHANNEL_DEF *chan_def = (MULTI_CHANNEL_DEF*)op_data->cap_class_ext;

    /* The message value is in samples. Convert it to octets */
    chan_def->metadata_delay = OCTETS_PER_SAMPLE * (((unsigned*)message_data)[3]);

    return (TRUE);
}
#endif /* INSTALL_METADATA */

bool ups_params_dbe(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    /* returns true if succesful, false if failed */
    DBE_OP_DATA   *op_extra_data = (DBE_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}

/**
 * \brief Allocate memory which will be used for the entire lifespan of the operator
 *
 * \param p_ext_data : pointer to the extra op data structure
 */
bool DBE_CAP_Create(DBE_OP_DATA* p_ext_data)
{
    unsigned int *memptr;

    /* allocate the DBE Buffers*/
    memptr = (unsigned int *)xzpmalloc(sizeof(unsigned int)*DBE_CAP_INTERNAL_BUFFER_SIZE/2);
    if (memptr == NULL)
    {
        return FALSE;
    }
    p_ext_data->hp1_out = memptr;

    memptr = (unsigned int *)xzpmalloc(sizeof(unsigned int)*DBE_CAP_INTERNAL_BUFFER_SIZE/2);
    if (memptr == NULL)
    {
        return FALSE;
    }
    p_ext_data->hp3_out = memptr;

    memptr = (unsigned int *)xzpmalloc(sizeof(unsigned int)*DBE_CAP_INTERNAL_BUFFER_SIZE);
    if (memptr == NULL)
    {
        return FALSE;
    }
    p_ext_data->hp2_out = memptr;

    memptr = (unsigned int *)xzpmalloc(sizeof(unsigned int)*DBE_CAP_INTERNAL_BUFFER_SIZE/2);
    if (memptr == NULL)
    {
        return FALSE;
    }
    p_ext_data->ntp_tp_filters_buf = memptr;

    memptr = (unsigned int *)xzpmalloc(sizeof(unsigned int)*DBE_CAP_INTERNAL_BUFFER_SIZE);
    if (memptr == NULL)
    {
        return FALSE;
    }
    p_ext_data->high_freq_output_buf = memptr;

    return TRUE;
}

/**
 * \brief Free memory which is be used for the entire lifespan of the operator
 *
 * \param p_ext_data : pointer to the extra op data structure
 */
void DBE_CAP_Destroy(OPERATOR_DATA *op_data)
{
    DBE_OP_DATA *p_ext_data = (DBE_OP_DATA *)(op_data->extra_op_data);

    /* Handle multi-channel detroy */
    multi_channel_detroy(op_data);

    /* returns true if succesful, false if failed */
     if (p_ext_data->hp1_out)
     {
         pfree(p_ext_data->hp1_out);
         p_ext_data->hp1_out = NULL;
     }
     if (p_ext_data->hp3_out)
     {
         pfree(p_ext_data->hp3_out);
         p_ext_data->hp3_out = NULL;
     }
     if (p_ext_data->hp2_out)
     {
         pfree(p_ext_data->hp2_out);
         p_ext_data->hp2_out = NULL;
     }
     if (p_ext_data->ntp_tp_filters_buf)
     {
         pfree(p_ext_data->ntp_tp_filters_buf);
         p_ext_data->ntp_tp_filters_buf = NULL;
    }
    if (p_ext_data->high_freq_output_buf)
    {
         pfree(p_ext_data->high_freq_output_buf);
         p_ext_data->high_freq_output_buf = NULL;
    }
}


bool dbe_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx)
{
   DBE_OP_DATA *p_ext_data = (DBE_OP_DATA *)(op_data->extra_op_data);
   dbe_channels *dbe_chan  = (dbe_channels*)chan_ptr;
   t_dbe_object *p_dobject;

   p_dobject = xzppmalloc(DBE_OBJECT_SIZE , MALLOC_PREFERENCE_DM1);
   if (p_dobject == NULL)
   {
      /* failed to allocate */
      return FALSE;
   }
   dbe_chan->dbe_object = p_dobject;
   dbe_chan->chan_idx   = chan_idx; 

   p_dobject->cap_config = p_ext_data->cap_config;
   p_dobject->params_ptr =(t_dbe_params*)&p_ext_data->dbe_cap_params;

   p_dobject->hp1_out    = p_ext_data->hp1_out;
   p_dobject->hp3_out    = p_ext_data->hp3_out;
   p_dobject->hp2_out    = p_ext_data->hp2_out;
   p_dobject->ntp_tp_filters_buf   = p_ext_data->ntp_tp_filters_buf;
   p_dobject->high_freq_output_buf = p_ext_data->high_freq_output_buf;

   p_ext_data->ReInitFlag = 1;

   return TRUE;
}

void dbe_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr)
{
   dbe_channels *dbe_chan  = (dbe_channels*)chan_ptr;

   if(dbe_chan->dbe_object)
   {
      pfree(dbe_chan->dbe_object);
      dbe_chan->dbe_object=NULL;
   }
}


