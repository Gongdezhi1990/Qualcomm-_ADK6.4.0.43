/**
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
* \file  cvc_receive.c
* \ingroup  capabilities
*
*  CVC receive
*
*/

/****************************************************************************
Include Files
*/
#include "capabilities.h"
#include "cvc_receive_cap_c.h"
#include "adaptor/adaptor.h"
#include "mem_utils/dynloader.h"
#include "mem_utils/exported_constants.h"
#include "mem_utils/exported_constant_files.h"
#include "platform/pl_fractional.h"
#include "ps/ps.h"
#include "obpm_prim.h"  
#include "cvc_recv_config.h"
#include "op_msg_helpers.h"
#include "pl_assert.h"

#include "patch/patch.h"
/****************************************************************************
Local Definitions
*/

/****************************************************************************
Private Constant Definitions
*/
#ifdef CAPABILITY_DOWNLOAD_BUILD
#define CVC_RECEIVE_NB_CAP_ID CAP_ID_DOWNLOAD_CVC_RECEIVE_NB
#define CVC_RECEIVE_WB_CAP_ID CAP_ID_DOWNLOAD_CVC_RECEIVE_WB
#define CVC_RECEIVE_FE_CAP_ID CAP_ID_DOWNLOAD_CVC_RECEIVE_FE
#define CVC_RECEIVE_UWB_CAP_ID CAP_ID_DOWNLOAD_CVC_RECEIVE_UWB
#define CVC_RECEIVE_SWB_CAP_ID CAP_ID_DOWNLOAD_CVC_RECEIVE_SWB
#define CVC_RECEIVE_FB_CAP_ID CAP_ID_DOWNLOAD_CVC_RECEIVE_FB
#else
#define CVC_RECEIVE_NB_CAP_ID CAP_ID_CVC_RECEIVE_NB
#define CVC_RECEIVE_WB_CAP_ID CAP_ID_CVC_RECEIVE_WB
#define CVC_RECEIVE_FE_CAP_ID CAP_ID_CVC_RECEIVE_FE
#define CVC_RECEIVE_UWB_CAP_ID CAP_ID_CVC_RECEIVE_UWB
#define CVC_RECEIVE_SWB_CAP_ID CAP_ID_CVC_RECEIVE_SWB
#define CVC_RECEIVE_FB_CAP_ID CAP_ID_CVC_RECEIVE_FB
#endif


/* Message handlers */

/** The cvc receive capability function handler table */
const handler_lookup_struct cvc_receive_handler_table =
{
    cvc_receive_create,         /* OPCMD_CREATE */
    cvc_receive_destroy,        /* OPCMD_DESTROY */
    base_op_start,              /* OPCMD_START */
    base_op_stop,               /* OPCMD_STOP */
    base_op_reset,              /* OPCMD_RESET */
    cvc_receive_connect,        /* OPCMD_CONNECT */
    cvc_receive_disconnect,     /* OPCMD_DISCONNECT */
    cvc_receive_buffer_details, /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,    /* OPCMD_DATA_FORMAT */
    cvc_receive_get_sched_info  /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */

const opmsg_handler_lookup_table_entry cvc_receive_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,                base_op_opmsg_get_capability_version},

    {OPMSG_COMMON_ID_SET_CONTROL,                       cvc_receive_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                        cvc_receive_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                      cvc_receive_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                        cvc_receive_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                        cvc_receive_opmsg_obpm_get_status},

    {OPMSG_COMMON_ID_SET_UCID,                          cvc_receive_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                 cvc_receive_opmsg_get_ps_id},
    {0, NULL}};


const CAPABILITY_DATA cvc_receive_nb_cap_data =
    {
        CVC_RECEIVE_NB_CAP_ID,                /* Capability ID */
        CVC_RECV_NB_VERSION_MAJOR, CVC_RECIEVE_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        1, 1,                            /* Max number of sinks/inputs and sources/outputs */
        &cvc_receive_handler_table,      /* Pointer to message handler function table */
        cvc_receive_opmsg_handler_table, /* Pointer to operator message handler function table */
        cvc_receive_process_data,        /* Pointer to data processing function */
        0,                               /* TODO - Processing time information */
        sizeof(CVC_RECEIVE_OP_DATA)      /* Size of capability-specific per-instance data */
    };

const CAPABILITY_DATA cvc_receive_wb_cap_data =
    {
        CVC_RECEIVE_WB_CAP_ID,            /* Capability ID */
        CVC_RECV_WB_VERSION_MAJOR, CVC_RECIEVE_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        1, 1,                            /* Max number of sinks/inputs and sources/outputs */
        &cvc_receive_handler_table,      /* Pointer to message handler function table */
        cvc_receive_opmsg_handler_table, /* Pointer to operator message handler function table */
        cvc_receive_process_data,        /* Pointer to data processing function */
        0,                               /* TODO - Processing time information */
        sizeof(CVC_RECEIVE_OP_DATA)      /* Size of capability-specific per-instance data */
    };

const CAPABILITY_DATA cvc_receive_fe_cap_data =
    {
        CVC_RECEIVE_FE_CAP_ID,           /* Capability ID */
        CVC_RECV_FE_VERSION_MAJOR, CVC_RECIEVE_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        1, 1,                            /* Max number of sinks/inputs and sources/outputs */
        &cvc_receive_handler_table,      /* Pointer to message handler function table */
        cvc_receive_opmsg_handler_table, /* Pointer to operator message handler function table */
        cvc_receive_process_data,        /* Pointer to data processing function */
        0,                               /* TODO - Processing time information */
        sizeof(CVC_RECEIVE_OP_DATA)      /* Size of capability-specific per-instance data */
    };

#ifdef INSTALL_OPERATOR_CVC_24K
const CAPABILITY_DATA cvc_receive_uwb_cap_data =
    {
        CVC_RECEIVE_UWB_CAP_ID,           /* Capability ID */
        CVC_RECV_UWB_VERSION_MAJOR, CVC_RECIEVE_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        1, 1,                            /* Max number of sinks/inputs and sources/outputs */
        &cvc_receive_handler_table,      /* Pointer to message handler function table */
        cvc_receive_opmsg_handler_table, /* Pointer to operator message handler function table */
        cvc_receive_process_data,        /* Pointer to data processing function */
        0,                               /* TODO - Processing time information */
        sizeof(CVC_RECEIVE_OP_DATA)      /* Size of capability-specific per-instance data */
    };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
const CAPABILITY_DATA cvc_receive_swb_cap_data =
    {
        CVC_RECEIVE_SWB_CAP_ID,           /* Capability ID */
        CVC_RECV_SWB_VERSION_MAJOR, CVC_RECIEVE_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        1, 1,                            /* Max number of sinks/inputs and sources/outputs */
        &cvc_receive_handler_table,      /* Pointer to message handler function table */
        cvc_receive_opmsg_handler_table, /* Pointer to operator message handler function table */
        cvc_receive_process_data,        /* Pointer to data processing function */
        0,                               /* TODO - Processing time information */
        sizeof(CVC_RECEIVE_OP_DATA)      /* Size of capability-specific per-instance data */
    };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
const CAPABILITY_DATA cvc_receive_fb_cap_data =
    {
        CVC_RECEIVE_FB_CAP_ID,           /* Capability ID */
        CVC_RECV_FB_VERSION_MAJOR, CVC_RECIEVE_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
        1, 1,                            /* Max number of sinks/inputs and sources/outputs */
        &cvc_receive_handler_table,      /* Pointer to message handler function table */
        cvc_receive_opmsg_handler_table, /* Pointer to operator message handler function table */
        cvc_receive_process_data,        /* Pointer to data processing function */
        0,                               /* TODO - Processing time information */
        sizeof(CVC_RECEIVE_OP_DATA)      /* Size of capability-specific per-instance data */
    };
#endif

/****************************************************************************
Public Function Declarations
*/

void* MakeStandardResponse_RCV(unsigned id)
{
      OP_STD_RSP  *std_resp = xpnew(OP_STD_RSP);

      if(!std_resp)
      {
         return(NULL);
      }
      std_resp->op_id = id;
      std_resp->resp_data.err_code = 0;
      std_resp->status = STATUS_CMD_FAILED;
      return((void*)std_resp);
}

#if defined(INSTALL_OPERATOR_CREATE_PENDING) && defined(INSTALL_CAPABILITY_CONSTANT_EXPORT)
void cvc_receive_create_pending_cb(OPERATOR_DATA *op_data,
                              uint16 cmd_id,void *msg_body,
                              tRoutingInfo *rinfo,unsigned cb_value)
{
    external_constant_callback_when_available((void*)cb_value,mkqid(op_data->task_id,1),cmd_id,msg_body,rinfo);
}
#endif

void cvc_receive_release_constants(OPERATOR_DATA *op_data)
{
    UNUSED(op_data);

#if defined(INSTALL_OPERATOR_CREATE_PENDING) && defined(INSTALL_CAPABILITY_CONSTANT_EXPORT)
    external_constant_release(cvclib_dataDynTable_Main,op_data->id);
    external_constant_release(CVC_RCV_CAP_dataDynTable_Main,op_data->id);
    external_constant_release(oms270_DynamicMemDynTable_Main,op_data->id);
    external_constant_release(filter_bank_DynamicMemDynTable_Main,op_data->id);
    external_constant_release(vad400_DynamicMemDynTable_Main,op_data->id);
#endif
}

/* ********************************** API functions ************************************* */
bool ups_state_rcv(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    if((length==2)&&(status==STATUS_OK))
    {
      /* Persistent gain stored as two 16-bit values.  
         Verify that gain is valid */
        op_extra_data->agc_state = ((data[0]&0xFFFF)<<16) | (data[1]&0xFFFF);
        if(op_extra_data->agc_state < FRACTIONAL(0.0015625))
        {
            op_extra_data->agc_state=FRACTIONAL(0.015625);
        }
        /* Set the Reinit flag after setting the paramters */
        op_extra_data->ReInitFlag = 1;
    }

    return(TRUE);
}

bool cvc_receive_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
      CVC_RECEIVE_OP_DATA *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
      PS_KEY_TYPE key;

      patch_fn_shared(cvc_receive_wrapper);
	  
#if defined(INSTALL_OPERATOR_CREATE_PENDING) && defined(INSTALL_CAPABILITY_CONSTANT_EXPORT)
      /* Reserve (and request) any dynamic memory tables that may be in external
       * file system.
       * A negative return value indicates a fatal error */
      if (   !external_constant_reserve(cvclib_dataDynTable_Main,op_data->id)
          || !external_constant_reserve(CVC_RCV_CAP_dataDynTable_Main,op_data->id)
          || !external_constant_reserve(oms270_DynamicMemDynTable_Main,op_data->id)
          || !external_constant_reserve(filter_bank_DynamicMemDynTable_Main,op_data->id)
          || !external_constant_reserve(vad400_DynamicMemDynTable_Main,op_data->id))
      {
         L2_DBG_MSG("cvc_receive_create failed reserving constants");
         cvc_receive_release_constants(op_data);
         return FALSE;
      }

      /* Now see if these tables are available yet */
      if (   !is_external_constant_available(cvclib_dataDynTable_Main,op_data->id)
          || !is_external_constant_available(CVC_RCV_CAP_dataDynTable_Main,op_data->id)
          || !is_external_constant_available(oms270_DynamicMemDynTable_Main,op_data->id)
          || !is_external_constant_available(filter_bank_DynamicMemDynTable_Main,op_data->id)
          || !is_external_constant_available(vad400_DynamicMemDynTable_Main,op_data->id))
      {
         /* Database isn't available yet. Arrange for a callback 
          * Only need to check on one table */
         *response_id = (unsigned)vad400_DynamicMemDynTable_Main;
         *response_data = (void*)(pending_operator_cb)cvc_receive_create_pending_cb;

         L4_DBG_MSG("cvc_receive_create - requesting callback when constants available");

         return (bool)HANDLER_INCOMPLETE;
      }
#endif

      /* Setup Response to Creation Request.   Assume Failure*/
      *response_id = OPCMD_CREATE;
      if((*response_data = MakeStandardResponse_RCV(op_data->id))==NULL)
      {
          cvc_receive_release_constants(op_data);
         return(FALSE);
      }

      /* Initialize extended data for operator.  Assume intialized to zero*/
      op_extra_data->cap_id = op_data->cap_data->id;
      op_extra_data->ReInitFlag = 1;
      op_extra_data->Host_mode = CVC_RECV_SYSMODE_FULL;
      op_extra_data->Cur_mode = CVC_RECV_SYSMODE_STANDBY;

      switch(op_extra_data->cap_id)
      {
      case CVC_RECEIVE_WB_CAP_ID:
         op_extra_data->frame_size_in = 120;
         op_extra_data->sample_rate = 16000;
         break;
#ifdef INSTALL_OPERATOR_CVC_24K
      case CVC_RECEIVE_UWB_CAP_ID:
         op_extra_data->frame_size_in = 120;
         op_extra_data->sample_rate = 24000;
         break;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
      case CVC_RECEIVE_SWB_CAP_ID:
         op_extra_data->frame_size_in = 240;
         op_extra_data->sample_rate = 32000;
         op_extra_data->apply_resample = RCV_VARIANT_SWB;
         break;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
      case CVC_RECEIVE_FB_CAP_ID:
         op_extra_data->frame_size_in = 360;
         op_extra_data->sample_rate = 48000;
         op_extra_data->apply_resample = RCV_VARIANT_FB;
         break;
#endif
      case CVC_RECEIVE_FE_CAP_ID:
      case CVC_RECEIVE_NB_CAP_ID:
      default:
         op_extra_data->frame_size_in = 60;
         op_extra_data->sample_rate = 8000;
         break;
      }

      op_extra_data->frame_size_out=op_extra_data->frame_size_in;
      if(op_extra_data->cap_id==CVC_RECEIVE_FE_CAP_ID)
      {
         /* For FE output is double the input */
         op_extra_data->frame_size_out <<= 1;
      }

      /*allocate the colume control shared memory*/
      op_extra_data->shared_volume_ptr = allocate_shared_volume_cntrl();
      if(!op_extra_data->shared_volume_ptr)
      {
          cvc_receive_release_constants(op_data);
         return(TRUE);
      }

      /* call the "create" assembly function*/
      if(CVC_RCV_CAP_Create(op_extra_data) !=0)
      {
         /* Free all the scratch memory we reserved */
         CVC_RCV_CAP_Destroy(op_extra_data);
         release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
         op_extra_data->shared_volume_ptr = NULL;
         cvc_receive_release_constants(op_data);
         return(TRUE);
      }

      if(!cpsInitParameters(&op_extra_data->parms_def,(unsigned*)CVC_RECV_GetDefaults(op_extra_data->cap_id),(unsigned*)op_extra_data->params,sizeof(CVC_RECV_PARAMETERS)))
      {
         /* Free all the scratch memory we reserved */
         CVC_RCV_CAP_Destroy(op_extra_data);
         release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
         op_extra_data->shared_volume_ptr = NULL;
         cvc_receive_release_constants(op_data);
         return(TRUE);
      }

      /* The default value of agc_state is 1.0 in q.17*/
      op_extra_data->agc_state = FRACTIONAL(0.015625);

      /* Load state info from UCID 0 */
      key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_extra_data->cap_id,0,OPMSG_P_STORE_STATE_VARIABLE_SUB_ID);
      ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_state_rcv);

      /* operator state variable set to not running state. based on base_op.c */
      op_data->state = OP_NOT_RUNNING;

      base_op_change_response_status(response_data,STATUS_OK);
      return TRUE;
 }

bool cvc_receive_ups_set_state(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank, STATUS_KYMERA status,
                                     uint16 extra_status_info)
{
    return TRUE;
}

bool cvc_receive_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
      CVC_RECEIVE_OP_DATA *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
      unsigned key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_extra_data->cap_id,0,OPMSG_P_STORE_STATE_VARIABLE_SUB_ID);
      uint16 state_data[2];

      /* Setup Response to Destroy Request.*/
      if(!base_op_destroy(op_data, message_data, response_id, response_data))
      {
         return(FALSE);
      }

      /* calling the "destroy" assembly function - this frees up all the capability-internal memory */
      CVC_RCV_CAP_Destroy(op_extra_data);

      /*free volume control shared memory*/
      release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
      op_extra_data->shared_volume_ptr = NULL;

      base_op_change_response_status(response_data,STATUS_OK);

      /* Save State information before destroy */
      /* Persistent Gain stored as two 16-bit values */
      state_data[0] = (op_extra_data->agc_state>>16)&0xFFFF;
      state_data[1] = op_extra_data->agc_state&0xFFFF;
      ps_entry_write((void*)op_data,key,PERSIST_ANY,2,state_data,cvc_receive_ups_set_state);
      
      cvc_receive_release_constants(op_data);

      return(TRUE);
}

bool cvc_receive_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    unsigned terminal_id   = ((unsigned*)message_data)[0];    /* extract the terminal_id */
    tCbuffer* pterminal_buf = (tCbuffer*)(uintptr_t)(((unsigned *)message_data)[1]);

    /* Setup Response to Connection Request.   Assume Failure*/
    *response_id = OPCMD_CONNECT;
    if((*response_data = MakeStandardResponse_RCV(op_data->id))==NULL)
    {
        return(FALSE);
    }

    /* Allow Connect/Disconnect while running*/
    op_extra_data->op_all_connected = FALSE;

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
        if (NULL == op_extra_data->input_stream->cbuffer)
        {
            op_extra_data->input_stream->cbuffer = pterminal_buf;
        }
        else
        {
            /* Streams should not have reached this point of sending us a connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }
    else
    {
        if (NULL == op_extra_data->output_stream->cbuffer)
        {
            op_extra_data->output_stream->cbuffer = pterminal_buf;
        }
        else
        {
            /* Streams should not have reached this point of sending us a connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }

    /* check if all the terminals have been connected */
    /* set internal capability state variable to "connected" */
    if((op_extra_data->input_stream->cbuffer != NULL) &&
            (op_extra_data->output_stream->cbuffer != NULL))
    {
        op_extra_data->op_all_connected = TRUE;
    }

    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
}

bool cvc_receive_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    unsigned terminal_id = *((unsigned*)message_data);

    /* Setup Response to Disconnection Request. Assume Failure*/
    *response_id = OPCMD_DISCONNECT;
    if((*response_data = MakeStandardResponse_RCV(op_data->id))==NULL)
    {
        return(FALSE);
    }

    /* check if the terminal ID is valid . The number has to be less than the maximum number of sinks or sources.  */
    if(!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* Allow Connect/Disconnect while running*/
    op_extra_data->op_all_connected = FALSE;

    /* check if the terminal is already connected and if not , connect the terminal */
    if(terminal_id & TERMINAL_SINK_MASK)
    {
        if (NULL != op_extra_data->input_stream->cbuffer)
        {
            op_extra_data->input_stream->cbuffer = NULL;
        }
        else
        {
            /* Streams should not have reached this point of sending us a connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }
    else
    {
        if (NULL != op_extra_data->output_stream->cbuffer)
        {
            op_extra_data->output_stream->cbuffer = NULL;
        }
        else
        {
            /* Streams should not have reached this point of sending us a connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }

    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
}

bool cvc_receive_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    unsigned terminal = ((unsigned *)message_data)[0];
    OP_BUF_DETAILS_RSP   *resp;
 
    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    resp = (OP_BUF_DETAILS_RSP*)*response_data;
    
    if (op_extra_data->cap_id == CVC_RECEIVE_FE_CAP_ID)
    {
        if (terminal & TERMINAL_SINK_MASK)
        {
            /* return the sink buffer size
               Make twice size needed to stay in sync if input is a real endpoint*/
            resp->b.buffer_size = 2*op_extra_data->frame_size_in;
        }
        else
        {
            /* return the source buffer size */
            resp->b.buffer_size = 2*op_extra_data->frame_size_out;
        }
    }
    else
    {
#ifdef DISABLE_IN_PLACE
        resp->b.buffer_size = 2*op_extra_data->frame_size_in;
#else
        if (terminal & TERMINAL_SINK_MASK)
        {
            /* input terminal. give the output buffer. */
            resp->b.in_place_buff_params.buffer = op_extra_data->output_stream->cbuffer;
        }
        else
        {
            /* Output terminal. give the input buffer. */
            resp->b.in_place_buff_params.buffer = op_extra_data->input_stream->cbuffer;
        }

        /* Run in place*/
        resp->runs_in_place = TRUE;

        /* Set the asked buffer size. 120 for narrow band, 240 for wide band. */
        resp->b.in_place_buff_params.size = 2*op_extra_data->frame_size_in;

        /* Choose terminal. */
        resp->b.in_place_buff_params.in_place_terminal = terminal^TERMINAL_SINK_MASK;   
#endif /* DISABLE_IN_PLACE*/
    }

#ifdef CVC_RECEIVE_SUPPORT_METADATA
    /* Support metadata if the ratio of input to output ratio
     * is 1:1 or 1:2. This condition is always TRUE in current
     * use cases, so metadata transport will be supported.
     */
    if((op_extra_data->frame_size_out == op_extra_data->frame_size_in)
       || (op_extra_data->frame_size_out == (op_extra_data->frame_size_in*2)))
    {
	    L2_DBG_MSG("CVC_RECEIVE_GENERATE_METADATA, metadata is supported");
        /* supports metadata in both side  */
        ((OP_BUF_DETAILS_RSP*)*response_data)->metadata_buffer = 0;
        ((OP_BUF_DETAILS_RSP*)*response_data)->supports_metadata = TRUE;
    }
#endif /* SCO_RX_OP_GENERATE_METADATA */

    return TRUE;
}

bool cvc_receive_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    OP_SCHED_INFO_RSP* resp;
    unsigned terminal = ((unsigned *)message_data)[0];

    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    /* Populate the response*/
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;

    /* Return 7.5 msec frame period in samples */
    if((op_extra_data->cap_id==CVC_RECEIVE_FE_CAP_ID) && !(terminal&TERMINAL_SINK_MASK) )
    {
        resp->block_size = op_extra_data->frame_size_out; 
    }
    else
    { 
        resp->block_size = op_extra_data->frame_size_in; 
    }

    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}

#ifdef CVC_RECEIVE_SUPPORT_METADATA
/**
 * cvc_receive_transport_metadata
 * \brief transfers metadata from input to output buffer
 * \param op_extra_data pointer to  CVC_RECEIVE_OP_DATA
 */
void cvc_receive_transport_metadata(CVC_RECEIVE_OP_DATA *op_extra_data)
{
    metadata_tag *in_mtag = NULL;
    metadata_tag *out_mtag = NULL;
    unsigned b4idx=0, afteridx=0;

    if(buff_has_metadata(op_extra_data->input_stream->cbuffer))
    {
        /* input has metadata, remove one input frames size */
        in_mtag = buff_metadata_remove(op_extra_data->input_stream->cbuffer,
                                       op_extra_data->frame_size_in * OCTETS_PER_SAMPLE,
                                       &b4idx, &afteridx);

        if(buff_has_metadata(op_extra_data->output_stream->cbuffer))
        {
            /* input has metadata, we only transfer metadata */
            out_mtag = in_mtag;
            in_mtag = NULL;
            if(op_extra_data->frame_size_out != op_extra_data->frame_size_in)
            {
                /* the ratio is expected to be 1:1 or 1:2 only */
                PL_ASSERT(op_extra_data->frame_size_out == (op_extra_data->frame_size_in * 2));

                /* adjust the tag info to double tag length */
                b4idx *= 2;
                afteridx *= 2;
                if (out_mtag != NULL)
                {
                    /* traverse through all tags in the removed list and double the length of the tags */
                    metadata_tag *mtag_temp = out_mtag;
                    while(mtag_temp != NULL)
                    {
                        mtag_temp->length *= 2;
                        mtag_temp = mtag_temp->next;
                    }
                }
            }

            /* transfer tag to output buffer */
            buff_metadata_append(op_extra_data->output_stream->cbuffer, out_mtag, b4idx, afteridx);
        }

        /* delete input tag list */
        buff_metadata_tag_list_delete(in_mtag);
    }
    else if (buff_has_metadata(op_extra_data->output_stream->cbuffer))
    {
        /* input doesn't have metadata but output does,
         * Normally this shouldn't be the case but in this
         * case we only append a balank tag to output buffer.
         */
        out_mtag = buff_metadata_new_tag();
        afteridx = op_extra_data->frame_size_out * OCTETS_PER_SAMPLE;
        out_mtag->length = afteridx;
        buff_metadata_append(op_extra_data->output_stream->cbuffer, out_mtag, b4idx, afteridx);
    }
}
#endif /* CVC_RECEIVE_SUPPORT_METADATA */

/* ************************************* Data processing-related functions and wrappers **********************************/

void cvc_receive_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    int available_space, samples_to_process;

    patch_fn(cvc_receive_process_data_patch);

    if(!op_extra_data->op_all_connected)
    {
       return;
    }

    /* number of samples to process at the input buffer(rcv-in)    */
    samples_to_process = cbuffer_calc_amount_data_in_words(op_extra_data->input_stream->cbuffer);

#ifdef CVC_RECEIVE_SUPPORT_METADATA
    if(buff_has_metadata(op_extra_data->input_stream->cbuffer))
    {
        /* limit amount to process to amount of available metadata */
        unsigned input_meta_available =  buff_metadata_available_octets(op_extra_data->input_stream->cbuffer)/OCTETS_PER_SAMPLE;
        samples_to_process = MIN(samples_to_process, input_meta_available);
    }
#endif

    /* Check for sufficient data and space */
    if(samples_to_process < op_extra_data->frame_size_in)
    {
       return;
    }

    /* available space at the output buffer(rcv-out) */
    available_space = cbuffer_calc_amount_space_in_words(op_extra_data->output_stream->cbuffer);
    if(available_space < op_extra_data->frame_size_out)
    {
       return;
    }

#ifdef CVC_RECEIVE_SUPPORT_METADATA
    cvc_receive_transport_metadata(op_extra_data);
#endif

    /* call the "process" assembly function */
    CVC_RCV_CAP_Process(op_extra_data);

    /* touched output */
    touched->sources = TOUCHED_SOURCE_0;
    /* touched input */
    touched->sinks = TOUCHED_SINK_0;
}


/* **************************** Operator message handlers ******************************** */

bool cvc_receive_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    unsigned            i,num_controls,cntrl_value; 
    CPS_CONTROL_SOURCE  cntrl_src;
    unsigned result = OPMSG_RESULT_STATES_NORMAL_STATE;

    patch_fn(cvc_receive_opmsg_obpm_set_control_patch);
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
        if (cntrl_value >= CVC_RECV_SYSMODE_MAX_MODES)
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
           op_extra_data->Ovr_Control = (cntrl_src == CPS_SOURCE_OBPM_DISABLE) ?  0 : CVC_RECV_CONTROL_MODE_OVERRIDE;
           op_extra_data->Obpm_mode = cntrl_value;
        }
    }

    cps_response_set_result(resp_data,result);

    /* Set the Reinit flag after setting the paramters */
    if (result == OPMSG_RESULT_STATES_NORMAL_STATE)
    {
        op_extra_data->ReInitFlag = 1;
    }
    return TRUE;
}

bool cvc_receive_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool cvc_receive_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
   
    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool cvc_receive_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    bool retval;

    patch_fn(cvc_receive_opmsg_obpm_set_params_patch);

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

bool cvc_receive_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;
    unsigned **stats = (unsigned**)op_extra_data->status_table;

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(CVC_RECV_STATISTICS),&resp))
    {
         return FALSE;
    }
    
    if(resp)
    {
        unsigned config_flag = CVC_RECV_CONFIG_FLAG;

        resp = cpsPackWords(&op_extra_data->Cur_mode, &op_extra_data->Ovr_Control, resp);
        resp = cpsPackWords(stats[0], stats[1], resp);
        /* Reset IN/OUT Peak Detectors*/
        *(stats[0])=0;
        *(stats[1])=0;
        resp = cpsPackWords(&config_flag, stats[2], resp);
        resp = cpsPackWords(stats[3], stats[4], resp);
        resp = cpsPackWords(stats[5], stats[6], resp);
        resp = cpsPackWords(stats[7], stats[8], resp);
        cpsPackWords(stats[9], NULL, resp);
    }
   
    return TRUE;
}



bool ups_params_rcv(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}

bool cvc_receive_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_extra_data->cap_id,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_rcv);

    return retval;
}

bool cvc_receive_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_RECEIVE_OP_DATA   *op_extra_data = (CVC_RECEIVE_OP_DATA*)op_data->extra_op_data;

    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,op_extra_data->cap_id,message_data,resp_length,resp_data);
}




