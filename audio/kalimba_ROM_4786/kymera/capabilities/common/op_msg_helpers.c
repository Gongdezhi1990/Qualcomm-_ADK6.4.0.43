/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  op_msg_utilities.c
 * \ingroup  capabilities
 *
 *  Common functions, used by capabilities to set/get OBPM params, defaults, statistics.
 *
 */

#include "op_msg_helpers.h"
#include "pmalloc/pl_malloc.h"
#include "opmsg_prim.h"
#include "base_op.h"
#include "const_data.h"
#include "sys_events.h"
#include "adaptor/adaptor.h"
#include "audio_log/audio_log.h"
#include "patch/patch.h"

/* Initialize Operator's parameter usage */
bool cpsInitParameters(CPS_PARAM_DEF *param_def_ptr,unsigned *default_ptr,unsigned *param_ptr,unsigned param_size)
{
#ifdef K32
   const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST,FORMAT_DSP_NATIVE,default_ptr);
#else
   const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_PACKED16,default_ptr);
#endif
   void *def_ptr;

   
   patch_fn(cpsInitParameters); 

   param_def_ptr->ucid = 0;
   param_def_ptr->param_state = PARAMETERS_NOT_READY;

   param_def_ptr->default_ptr = default_ptr;
   param_def_ptr->param_ptr   = param_ptr; 
   param_def_ptr->num_params  = param_size >> LOG2_ADDR_PER_WORD;

   /* Get External Default Parmeters */
   def_ptr = const_data_access(&mem_desc,0,NULL,param_def_ptr->num_params<<LOG2_ADDR_PER_WORD);
   if(!def_ptr)
   {
      return (FALSE);
   }

   /* call the "set_defaults" assembly function(this loads the default values) */
   cpsSetDefaults((unsigned*)def_ptr,param_def_ptr->param_ptr,param_def_ptr->num_params);

   /* Release Default Parameters */
   const_data_release(def_ptr);
      
   return (TRUE);

}
 
/* Respond to request for Parameters */
bool cpsGetParameterMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    OPMSG_PARAM_MSG  *obpm_msg = (OPMSG_PARAM_MSG*)message_data;
    unsigned rsize,osize,*resp;
    unsigned num_blocks = obpm_msg->num_blocks & CPS_NUM_BLOCKS_MASK; /* Actual number of blocks */
    unsigned msg_len = OPMGR_GET_OPMSG_LENGTH(obpm_msg);

    patch_fn(cpsGetParameterMsgHandler); 

    /* Make sure the message is long enough to contain the request blocks
     * Expected length is 2 (ID, num_blocks) + 2 per block.
     */
    if (msg_len < 2 + 2*num_blocks)
    {
        L2_DBG_MSG2("cpsGetParameterMsgHandler FAILED msg_len = %u blocks = %u", msg_len, num_blocks);
        return FALSE;
    }

    rsize = cpsComputeGetParamsResponseSize( obpm_msg->data_block,obpm_msg->num_blocks,param_def_ptr->num_params);
    osize = rsize + 2;  /* op_msg_id + result_field + payload*/

#ifdef USE_ACCMD
    if (GET_CON_ID_SEND_ID(OPMGR_GET_OPMSG_CLIENT_ID(obpm_msg)) == RESPOND_TO_OBPM)
#endif /* USE_ACCMD */
    {
        if (osize > OPMGR_GET_OPMSG_LENGTH(obpm_msg))
        {
            osize=2;
        }
    }
    *resp_length = osize;

    if ((resp = xpmalloc(osize*sizeof(unsigned)))==NULL)
    {
        return (FALSE);
    }
    *resp_data = (OP_OPMSG_RSP_PAYLOAD *)resp;

    resp[0] = OPMGR_GET_OPMSG_MSG_ID(obpm_msg);          /*message ID*/

    if(osize>2)
    {
        resp[1] = OPMSG_RESULT_STATES_NORMAL_STATE;               /*result_field*/
    }
    else
    {
        resp[1] = (rsize ? OPMSG_RESULT_STATES_MESSAGE_TOO_BIG : OPMSG_RESULT_STATES_INVALID_PARAMETER);  /*result_field*/
        return TRUE;
    }
    if (param_def_ptr->param_state != PARAMETERS_READY)
    {
        resp[1] = OPMSG_RESULT_STATES_PARAMETER_STATE_NOT_READY;  /*result_field*/
    }
    cpsGetParameters( obpm_msg->data_block,obpm_msg->num_blocks, (unsigned*)param_def_ptr->param_ptr,&resp[2]);

    return TRUE;
}

/* Respond to request for Deafults */
bool cpsGetDefaultsMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    OPMSG_PARAM_MSG  *obpm_msg = (OPMSG_PARAM_MSG*)message_data;
    unsigned rsize, osize, *resp;
    unsigned num_blocks = obpm_msg->num_blocks & CPS_NUM_BLOCKS_MASK; /* Actual number of blocks */
    unsigned msg_len = OPMGR_GET_OPMSG_LENGTH(obpm_msg);

#ifdef K32
    const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST,FORMAT_DSP_NATIVE,param_def_ptr->default_ptr);
#else
    const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_PACKED16,param_def_ptr->default_ptr);
#endif
    void *def_ptr;

    patch_fn(cpsGetDefaultsMsgHandler);

    /* Make sure the message is long enough to contain the request blocks
     * Expected length is 2 (ID, num_blocks) + 2 per block.
     */
    if (msg_len < 2 + 2*num_blocks)
    {
        L2_DBG_MSG2("cpsGetDefaultsMsgHandler FAILED msg_len = %u blocks = %u", msg_len, num_blocks);
        return FALSE;
    }

    rsize = cpsComputeGetParamsResponseSize( obpm_msg->data_block,obpm_msg->num_blocks,param_def_ptr->num_params);
    osize = rsize + 2;       /* op_msg_id + result_field + payload*/

#ifdef USE_ACCMD
    if (GET_CON_ID_SEND_ID(OPMGR_GET_OPMSG_CLIENT_ID(obpm_msg)) == RESPOND_TO_OBPM)
#endif /* USE_ACCMD */
    {
        if(osize > msg_len)
        {
            osize = 2;
        }
    }
    *resp_length = osize;

    if ((resp = xpmalloc(osize*sizeof(unsigned)))==NULL)
    {
        return(FALSE);
    }
    *resp_data = (OP_OPMSG_RSP_PAYLOAD *)resp;

    resp[0] = OPMGR_GET_OPMSG_MSG_ID(obpm_msg);          /*message ID*/

    if(osize > 2)
    {
        resp[1] = OPMSG_RESULT_STATES_NORMAL_STATE;                /*result_field*/
    }
    else
    {
        resp[1] = (rsize ? OPMSG_RESULT_STATES_MESSAGE_TOO_BIG : OPMSG_RESULT_STATES_INVALID_PARAMETER);   /*result_field*/
        return TRUE;
    }

    /* Get External Default Parmeters */
    def_ptr = const_data_access(&mem_desc,0,NULL,param_def_ptr->num_params<<LOG2_ADDR_PER_WORD);

    if(def_ptr)
    {
         cpsGetDefaults(obpm_msg->data_block,obpm_msg->num_blocks,(unsigned*)def_ptr,&resp[2]);

         /* Release Default Parameters */
         const_data_release(def_ptr);
    }
    else
    {
         resp[1] = OPMSG_RESULT_STATES_MESSAGE_TOO_BIG;
    }
    return TRUE;
}

bool cpsSetParameterMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    OPMSG_SET_PARAM_MSG    *obpm_msg = (OPMSG_SET_PARAM_MSG*)message_data;
    unsigned *resp;
    
    patch_fn(cpsSetParameterMsgHandler);    

    /* set response length */
    *resp_length = 2;
    resp = (unsigned*) xpmalloc(2*sizeof(unsigned));

    if ((*resp_data=(OP_OPMSG_RSP_PAYLOAD *)resp) == NULL)
    {
        return(FALSE);
    }

    resp[0] = OPMGR_GET_OPMSG_MSG_ID(obpm_msg);  /*message ID*/


    if((obpm_msg->num_blocks & 0x8000) != 0)   /*Check msb of num_blocks for defaults flag*/
    {
#ifdef K32
         const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST,FORMAT_DSP_NATIVE,param_def_ptr->default_ptr);
#else
         const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_PACKED16,param_def_ptr->default_ptr);
#endif
         void *def_ptr;

         /* Get External Default Parmeters */
         def_ptr = const_data_access(&mem_desc,0,NULL,param_def_ptr->num_params<<LOG2_ADDR_PER_WORD);
         if(!def_ptr)
         {
            resp[1] = OPMSG_RESULT_STATES_INVALID_PARAMETER;     /*result_field*/
            return TRUE;
         }

         cpsSetDefaults((unsigned *)def_ptr,param_def_ptr->param_ptr,param_def_ptr->num_params);

         /* Release Default Parameters */
         const_data_release(def_ptr);
    }

    if(cpsSetParameters(obpm_msg->data_block,(obpm_msg->num_blocks & 0x7FFF),param_def_ptr->param_ptr,param_def_ptr->num_params))
    {
        resp[1] = OPMSG_RESULT_STATES_INVALID_PARAMETER;     /*result_field*/
    }
    else if (param_def_ptr->param_state != PARAMETERS_READY)
    {
        resp[1] = OPMSG_RESULT_STATES_PARAMETER_STATE_NOT_READY;  /*result_field*/
    }
    else
    {
        resp[1] = OPMSG_RESULT_STATES_NORMAL_STATE;        /*result_field*/
    }

    return TRUE;
}

bool cpsSetParameterFromPsStore(CPS_PARAM_DEF *param_def_ptr,uint16 length, unsigned* data, STATUS_KYMERA status)
{
    
    patch_fn(cpsSetParameterFromPsStore);  
    
    if((length>1)&&(status==STATUS_OK))
    {
#ifdef K32
         const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST,FORMAT_DSP_NATIVE,param_def_ptr->default_ptr);
#else
         const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_PACKED16,param_def_ptr->default_ptr);
#endif
         void *def_ptr;

         /* Get External Default Parmeters */
         def_ptr = const_data_access(&mem_desc,0,NULL,param_def_ptr->num_params<<LOG2_ADDR_PER_WORD);
         if(def_ptr)
         {
            cpsSetDefaults((unsigned*)def_ptr,param_def_ptr->param_ptr,param_def_ptr->num_params);

            /* Release Default Parameters */
            const_data_release(def_ptr);
         }

         cpsSetParameters((OPMSG_SET_PARAM_BLOCK*)&data[1],(data[0] & 0x7FFF),param_def_ptr->param_ptr,param_def_ptr->num_params);
    }
    /* We got Parameters */
    param_def_ptr->param_state = PARAMETERS_READY;

    return TRUE;
}


bool cpsSetUcidMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
   OPMSG_SET_UCID_MSG    *op_msg = (OPMSG_SET_UCID_MSG*)message_data;
   
   patch_fn(cpsSetUcidMsgHandler);     

   param_def_ptr->ucid = op_msg->ucid;

   /* Signal OBPM that UCID has changed for an operator */
   set_system_event(SYS_EVENT_UCID_CHANGE);  

   return TRUE;
}

extern bool cpsGetUcidMsgHandler(CPS_PARAM_DEF *param_def_ptr,unsigned cap_id,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    OPMSG_GET_LOGICAL_PS_ID_MSG    *op_msg = (OPMSG_GET_LOGICAL_PS_ID_MSG*)message_data;
    unsigned *resp;

    patch_fn(cpsGetUcidMsgHandler);     
    
    resp = (unsigned*) xpmalloc(4*sizeof(unsigned));
    if (!resp)
    {
       return (FALSE);
    }

    resp[0] = OPMGR_GET_OPMSG_MSG_ID(op_msg);  /*message ID*/
    resp[1] = OPMSG_RESULT_STATES_NORMAL_STATE;         /*result field*/
    resp[2] = param_def_ptr->ucid;
    resp[3] = cap_id;
  
    /* set response length */
    *resp_length = 4;
    /* allocate and fill response data */
    *resp_data = (void*)resp;

    return TRUE;
}

/* Handle setcontrol without any operation performed on the message data */
bool cps_control_setup(void* message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data,unsigned *num_controls)
{
    /* In the case of this capability, nothing is done for control message. Just follow protocol and ignore any content. */
    OP_OPMSG_RSP_PAYLOAD *resp = xpnew(OP_OPMSG_RSP_PAYLOAD);

    patch_fn(cps_control_resp_helper);     

    if(resp == NULL)
    {
        return FALSE;
    }

    resp->msg_id = OPMGR_GET_OPMSG_MSG_ID(((OPMSG_PARAM_MSG*)message_data));
    resp->u.cps.status = OPMSG_RESULT_STATES_NORMAL_STATE;

    *resp_data = resp;
    *resp_length = 2;

    if(num_controls!=NULL)
    {
         OPMSG_SET_CONTROL_MSG    *op_msg = (OPMSG_SET_CONTROL_MSG*)message_data;
         *num_controls = op_msg->num_blocks;
    }
    return TRUE;
}

unsigned cps_control_get(void* message_data, unsigned index,unsigned *value,CPS_CONTROL_SOURCE *source)
{
   OPMSG_SET_CONTROL_MSG    *op_msg    = (OPMSG_SET_CONTROL_MSG*)message_data;
   OPMSG_SET_CONTROL_BLOCK  *cntrl_blk = &op_msg->data[index];

   if(source!=NULL)
   {
        if(OPMGR_GET_OPMSG_CLIENT_ID(op_msg) != RESPOND_TO_OBPM)
        {
           *source = CPS_SOURCE_HOST;
        }
        else 
        {
           *source = (cntrl_blk->control_id & OPMSG_CONTROL_OBPM_OVERRIDE) ?  CPS_SOURCE_OBPM_DISABLE : CPS_SOURCE_OBPM_ENABLE;
        }
   }
   if(value!=NULL)
   {
      *value =(unsigned)(((cntrl_blk->msw&0xFFFF)<<16)|(cntrl_blk->lsw & 0xFFFF));
   }
   return cntrl_blk->control_id&0x7FFF;
}


void cps_response_set_result(OP_OPMSG_RSP_PAYLOAD **resp_data,unsigned result)
{
   OP_OPMSG_RSP_PAYLOAD *resp = *resp_data;
   
    patch_fn(cps_response_set_result); 
    
   resp->u.cps.status = result;
}


/* build a response message for operations where OBPM requests parameter or statistics values */
bool common_obpm_response_helper(OP_MSG_REQ *op_msg, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data, unsigned size)
{
    OP_OPMSG_RSP_PAYLOAD* resp;
    unsigned status = OPMSG_RESULT_STATES_NORMAL_STATE;
    
    patch_fn(common_obpm_response_helper);

    if(size == 0)
    {
        status = OPMSG_RESULT_STATES_INVALID_PARAMETER;
    }

    /* Length is op_msg_id + result_field + payload */
    *resp_length = 2 + size;
    resp = (OP_OPMSG_RSP_PAYLOAD*)xpmalloc((*resp_length)*sizeof(unsigned));

    if(resp == NULL)
    {
        return FALSE;
    }

    *resp_data = resp;

    /* First two fields have the opmsg ID and the capability- & opmsg-specific status.
     * In case of error, there is no other payload.
     */
    resp->msg_id = OPMGR_GET_OPMSG_MSG_ID(op_msg);
    resp->u.cps.status = status;

    return TRUE;
}


bool common_obpm_status_helper(void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data, unsigned size,unsigned **resptr)
{
   OPMSG_GET_STATUS_MSG *status_msg = (OPMSG_GET_STATUS_MSG*)message_data;
   unsigned  osize,*resp;
   
    patch_fn(common_obpm_status_helper);
    
#ifdef K32
   osize = ((size*2) >> LOG2_ADDR_PER_WORD) + 3;
#else
   osize = ((size + ((size + 1)>>1)) >> LOG2_ADDR_PER_WORD) + 3;
#endif
  
   if (osize > OPMGR_GET_OPMSG_LENGTH(status_msg))
   {
        osize=2;
   }

   /* allocate and fill response data */
   *resp_length = osize;
   if ((resp = xpmalloc(osize*sizeof(unsigned)))==0)
   {
        return (FALSE);
   }
   *resp_data = (OP_OPMSG_RSP_PAYLOAD *)resp;

   resp[0] = OPMGR_GET_OPMSG_MSG_ID(status_msg);          /*message ID*/

   if (osize > 2)
   {
        resp[1] = OPMSG_RESULT_STATES_NORMAL_STATE;            /*result field*/
        resp[2] = status_msg->stat_config;                     /*stat config*/

        *resptr = resp + 3;
   }
   else
   {
        resp[1] = OPMSG_RESULT_STATES_MESSAGE_TOO_BIG;         /*result field*/
        *resptr = NULL;
   }
   return(TRUE);
}
