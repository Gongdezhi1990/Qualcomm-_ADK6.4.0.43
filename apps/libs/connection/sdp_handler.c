/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_handler.c        

DESCRIPTION
		

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "sdp_handler.h"

#include <bdaddr.h>
#include <string.h>
#include <vm.h>
#include <app/bluestack/sdc_prim.h>

#ifndef CL_EXCLUDE_SDP

/*lint -e725 -e830 */

typedef enum 
{
	sdp_start_search,
	sdp_lock_set,
	sdp_search_lock_set,
	sdp_session_open_other_dev
} sdp_search_req;


/* Request the service handle(s) of the HSP service at the AG */
static const uint8 SdpPingServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, 0x00, 0x00 /* 4 byte UUID - will be filled in by app */
};

/****************************************************************************
NAME	
	sendSdpCloseSearchCfm

DESCRIPTION
	Issue a confirm message to the task that requested the search be closed

RETURNS
	void	
*/
static void sendSdpCloseSearchCfm(Task appTask, sdp_close_status cfm_result)
{
    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_SDP_CLOSE_SEARCH_CFM);
        message->status = cfm_result;
        MessageSend(appTask, CL_SDP_CLOSE_SEARCH_CFM, message);
    }
}


/****************************************************************************
NAME	
	sendSearchRequest

DESCRIPTION
	Decide whather a search request primitive can be sent to BlueStack or not

RETURNS
	sdp_search_req - see typedef above
*/
static sdp_search_req sendSearchRequest(Task theAppTask, const connectionSdpState *state, const bdaddr *bd_addr)
{
	/* Check the first lock */
	if (state->sdpLock)
	{
		/* SDP search session open or non search primitive sent to BlueStack */
		if (BdaddrIsZero(&state->sdpServerAddr))
		{
			/* 
				SDP search session not open but another prim is currently 
				being handled (sdpLock set) so we can't start the search yet
			*/
			return sdp_lock_set;
		}
		else
		{
			/* Addr set - SDP search session open */
			if (state->sdpSearchLock)
			{
				/* Search currently active so can't start another one */
				return sdp_search_lock_set;
			}
			else
			{
				/* SDP session open but no active search */
				if (BdaddrIsSame(&state->sdpServerAddr, bd_addr))
				{
					/* 
						SDP search session open to the device we want to search 
						so we can just start our search
					*/
					return sdp_start_search;
				}
				else
				{
					/* 
						SDP search session open to a different device so we can't
						do our search until this session is closed.
					*/
					return sdp_session_open_other_dev;
				}
			}
		}
	}
	else
	{
		/* Check if we currently have a search active, block internal requests if sdpLock not set */
		if (state->sdpSearchLock || theAppTask == connectionGetCmTask())
		{
			/* Search currently active */
			return sdp_search_lock_set;
		}
		else
		{
			/* No search currently active */
			return sdp_start_search;
		}
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpRegisterRequest

DESCRIPTION
	Sends a register request to the SDP server

RETURNS
	void	
*/
void connectionHandleSdpRegisterRequest(connectionSdpState *state, const CL_INTERNAL_SDP_REGISTER_RECORD_REQ_T *req)
{
	/* If the resource is not locked, send downt he request */
	if (!state->sdpLock)
	{
		/* Set the lock */
		state->sdpLock = req->theAppTask;

		{
			MAKE_PRIM_T(SDS_REGISTER_REQ);
			prim->phandle = 0;
			prim->num_rec_bytes = req->record_length;
			prim->service_rec = VmGetHandleFromPointer(req->record);
			VmSendSdpPrim(prim);
		}
	}
	else
	{
		/* Resource locked, queue up the request */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_REGISTER_RECORD_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_REGISTER_RECORD_REQ, message, &state->sdpLock);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpRegisterCfm

DESCRIPTION
	Handle the register response received from BlueStack

RETURNS
	void	
*/
void connectionHandleSdpRegisterCfm(connectionSdpState *state, const SDS_REGISTER_CFM_T *cfm)
{
    if (state->sdpLock)
    {
        /* Send response to the task that originated the request */
        MAKE_CL_MESSAGE(CL_SDP_REGISTER_CFM);
        message->status = connectionConvertSdsResult(cfm->result);
        message->service_handle = cfm->svc_rec_hndl;
        MessageSend(state->sdpLock, CL_SDP_REGISTER_CFM, message);
        
        /* Reset the resource lock  if the state is not pending*/
        if (cfm->result != SDS_PENDING)
            state->sdpLock = 0;
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpUnregisterRequest

DESCRIPTION
	Handle the request to unregister a particular service record

RETURNS
	void	
*/
void connectionHandleSdpUnregisterRequest(connectionSdpState *state, const CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ_T *req)
{
	/* Check the resource lock */
	if (!state->sdpLock)
	{
		/* Set lock */
		state->sdpLock = req->theAppTask;

		/* Resource free so send request down to BlueStack */
		{
			MAKE_PRIM_T(SDS_UNREGISTER_REQ);
			prim->phandle = 0;
			prim->svc_rec_hndl = req->service_handle;
			VmSendSdpPrim(prim);
		}
	}
	else
	{
		/* Resource is locked, queue up the request for later */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ, message, &state->sdpLock);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpUnregisterCfm

DESCRIPTION
	Handle the response from BlueStack to an SDP unregister request

RETURNS
	void	
*/
void connectionHandleSdpUnregisterCfm(connectionSdpState *state, const SDS_UNREGISTER_CFM_T *cfm)
{
    if (state->sdpLock)
    {
        /* Send response to the client */
        MAKE_CL_MESSAGE(CL_SDP_UNREGISTER_CFM);
        message->status =  connectionConvertSdsResult(cfm->result);
        message->service_handle = cfm->svc_rec_hndl;
        MessageSend(state->sdpLock, CL_SDP_UNREGISTER_CFM, message);
        
        /* Reset the resource lock  if the state is not pending*/
        if (cfm->result != SDS_PENDING)
            state->sdpLock = 0;
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpServerConfigMtu

DESCRIPTION
	Send a request to BlueStack to configure the SDP server MTU size

RETURNS
	void	
*/
void connectionHandleSdpServerConfigMtu(const connectionSdpState *state, const CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ_T *req)
{
	/* 
		We don't get a response from this so send it without setting the lock. 
		Still have to make sure the lock isn't set though so check it before 
		sending the prim 
	*/
	if (!state->sdpLock)
	{
		MAKE_PRIM_T(SDS_CONFIG_REQ);
		prim->l2cap_mtu = req->mtu;
		VmSendSdpPrim(prim);
	}
	else
	{
		/* Resource is locked, queue up the request for later */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ, message, &state->sdpLock);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpClientConfigMtu

DESCRIPTION
	Send a request to BlueStack to configure the SDP client MTU size

RETURNS
	void	
*/
void connectionHandleSdpClientConfigMtu(const connectionSdpState *state, const CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ_T *req)
{
	/* 
		We don't get a response from this so send it without setting the lock. 
		Still have to make sure the lock isn't set though so check it before 
		sending the prim 
	*/
	if (!state->sdpLock)
	{
		MAKE_PRIM_T(SDC_CONFIG_REQ);
		prim->l2cap_mtu = req->mtu;
		VmSendSdpPrim(prim);
	}
	else
	{
		/* Resource is locked, queue up the request for later */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ, message, &state->sdpLock);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpOpenSearchRequest

DESCRIPTION
	Send a request to BlueSTack to open an SDP search session

RETURNS
	void	
*/
void connectionHandleSdpOpenSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_OPEN_SEARCH_REQ_T *req)
{
	/* Check the state of the resource lock */
	if (!state->sdpLock)
	{
		/* Resource free, set the lock */
		state->sdpLock = req->theAppTask;

		/* Store the address of the device we're opening the search to */
		state->sdpServerAddr = req->bd_addr;

		/* Send the request to BlueStack */
		{
			MAKE_PRIM_T(SDC_OPEN_SEARCH_REQ);
			BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
			VmSendSdpPrim(prim);
		}
	}
	else if(req->theAppTask != connectionGetCmTask())
	{
		/* Resource busy so queue up the request */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_OPEN_SEARCH_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_OPEN_SEARCH_REQ, message, &state->sdpLock);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpOpenSearchCfm

DESCRIPTION
	Handle the response to the open SDP session request

RETURNS
	void	
*/
void connectionHandleSdpOpenSearchCfm(connectionSdpState *state, const SDC_OPEN_SEARCH_CFM_T *cfm)
{
	if(state->sdpLock == connectionGetCmTask())
	{
		/* Request was internal, start SDP Ping */
		if (cfm->result == SDC_OPEN_SEARCH_OK)
			ConnectionSdpServiceSearchRequest(connectionGetCmTask(), &state->sdpServerAddr, 1, sizeof(SdpPingServiceRequest), SdpPingServiceRequest);
	}
	else if(state->sdpLock)
	{
		/* Send a response message up to the client */
        MAKE_CL_MESSAGE(CL_SDP_OPEN_SEARCH_CFM);
        message->status = connectionConvertSdpOpenStatus(cfm->result);
        MessageSend(state->sdpLock, CL_SDP_OPEN_SEARCH_CFM, message);
	}

	/* Check if the open search succeeded */
	if (cfm->result != SDC_OPEN_SEARCH_OK)
	{
		/* Reset the resource lock */
		state->sdpLock = 0;
		
		/* Reset the stored address */
		BdaddrSetZero(&state->sdpServerAddr);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpCloseSearchRequest

DESCRIPTION
	Issue a close search request to BlueStack

RETURNS
	void	
*/
void connectionHandleSdpCloseSearchRequest(const connectionSdpState *state, const CL_INTERNAL_SDP_CLOSE_SEARCH_REQ_T *req)
{
	/* Check the state of the resource lock */
	if (state->sdpLock)
	{
		/* Check if we have a search session currently open */
		if (!BdaddrIsZero(&state->sdpServerAddr))
		{
			/* We do have a search session open */
			if (state->sdpLock == req->theAppTask)
			{
				/* Only allow the task that opened the search to close it */
				MAKE_PRIM_T(SDC_CLOSE_SEARCH_REQ);
				VmSendSdpPrim(prim);
			}
			else
			{
				/* Task requesting the search be closed did not open it - send error */
				sendSdpCloseSearchCfm(req->theAppTask, sdp_close_task_did_not_open_search);
			}
		}
		else
		{
			/* We don't have a search session open, reply immediately with error */
			sendSdpCloseSearchCfm(req->theAppTask, sdp_close_search_not_open);
		}
	}
	else
	{
		/* There can't be a search open, reply immediately with an error */	
		sendSdpCloseSearchCfm(req->theAppTask, sdp_close_search_not_open);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpCloseSearchInd

DESCRIPTION
	Indication from BlueStack informing us that an SDP search session has 
	been closed.

RETURNS
	void	
*/
void connectionHandleSdpCloseSearchInd(connectionSdpState *state, const SDC_CLOSE_SEARCH_IND_T *ind)
{
    sdp_close_status status;

    switch(ind->result)
    {
        case SDC_SDS_DISCONNECTED:
            status = sdp_close_sdc_sds_disconnected;
            break;
        case SDC_SDC_DISCONNECTED:
            status = sdp_close_sdc_sdc_disconnected;
            break;
        default:
            status = sdp_close_unknown;
            break;
    }
	/* Inform the client that the SDP search session has been closed */
	sendSdpCloseSearchCfm(state->sdpLock, status);

	/* Reset the resource lock */
	state->sdpLock = 0;
		
	/* Reset the stored address */
	BdaddrSetZero(&state->sdpServerAddr);
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchRequest

DESCRIPTION
	Handle a request to initiate a service search to a remote device.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_SERVICE_SEARCH_REQ_T *req)
{
	sdp_search_req srch_result = sendSearchRequest(req->theAppTask, state, &req->bd_addr);
	if (srch_result == sdp_start_search)
	{
		/* Send the request to BlueStack */
		MAKE_PRIM_T(SDC_SERVICE_SEARCH_REQ);
		prim->phandle = 0;
        BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
        prim->size_srch_pttrn = req->length;

		if (req->length)
        {
            uint8* search_pattern = PanicUnlessMalloc(req->length);
            memmove(search_pattern, req->search_pattern, req->length);
            prim->srch_pttrn = VmGetHandleFromPointer(search_pattern);
        }
		else
        {
			prim->srch_pttrn = 0;
        }

        prim->max_num_recs = req->max_responses;
		VmSendSdpPrim(prim);

		/* Lock the resource */
		state->sdpSearchLock = req->theAppTask;

        /* set the flags */
        ((connectionState*)connectionGetCmTask())->flags |= req->flags;
	}
	else if(req->theAppTask != connectionGetCmTask())
	{
		Task *c = 0;

		/* Queue up the search request */
        MAKE_CL_MESSAGE_WITH_LEN(CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, req->length);
        COPY_CL_MESSAGE_WITH_LEN(CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, req->length, req, message);

		/* Use the return value from searchRequest function to decide which resource to wait on */
		if (srch_result == sdp_lock_set)
			c = &state->sdpLock;
		else if (srch_result == sdp_search_lock_set)
			c = &state->sdpSearchLock;
		else if (srch_result == sdp_session_open_other_dev)
			c = &state->sdpLock;
		else
			/* Should not get here but if we do panic */
			Panic();

		/* Queus up message with condition determined above */
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, message, c);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchCfm

DESCRIPTION
	SDP service search results received so handle them here.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchCfm(connectionSdpState *state, const SDC_SERVICE_SEARCH_CFM_T *cfm)
{
    connectionState *cstate = (connectionState*)connectionGetCmTask();
    uint16 len;
    uint16 msgid;

    /* extra length required for a message */
    if (cstate->flags & CONNECTION_FLAG_SDP_REFERENCE)
    {
        cstate->flags &= ~CONNECTION_FLAG_SDP_REFERENCE;
        len = 0;
        msgid = CL_SDP_SERVICE_SEARCH_REF_CFM;
    }
    else
    {
        len = cfm->size_rec_list;
        msgid = CL_SDP_SERVICE_SEARCH_CFM;
    }
    
	if (state->sdpSearchLock == connectionGetCmTask())
	{
		/* Internal CFM means we're SDP Pinging */
		/* Create next request */
        MAKE_CL_MESSAGE_WITH_LEN(CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, sizeof(SdpPingServiceRequest));

		message->theAppTask = connectionGetCmTask();
		message->bd_addr = state->sdpServerAddr;
		message->max_responses = 1;
		message->length = sizeof(SdpPingServiceRequest);				 
		memmove(message->search_pattern, SdpPingServiceRequest, sizeof(SdpPingServiceRequest));
        message->flags = 0;

		/* Send after 2 seconds to avoid thrashing the radio */
		MessageSendLater(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, message, D_SEC(2));
		
		/* Tidy up memory in cfm */
		if (cfm->size_rec_list)
        {
            uint8 *record_list = VmGetPointerFromHandle(cfm->rec_list);
            free(record_list);
        }
        
		/* Free the lock so the next request can be handled */
		state->sdpSearchLock = 0;
	}
    else if (state->sdpSearchLock)
    {
        CL_SDP_SERVICE_SEARCH_CFM_T *new_cfm=NULL;
        CL_SDP_SERVICE_SEARCH_REF_CFM_T *message;

        /* Send a cfm to the client task */
        if (cfm->size_rec_list && msgid == CL_SDP_SERVICE_SEARCH_CFM)
        {
            new_cfm = PanicUnlessMalloc(sizeof(CL_SDP_SERVICE_SEARCH_CFM_T) + cfm->size_rec_list);
            message = (CL_SDP_SERVICE_SEARCH_REF_CFM_T *)new_cfm;
        }
        else 
        {
            message = PanicUnlessMalloc(sizeof(CL_SDP_SERVICE_SEARCH_REF_CFM_T));
        }
        message->status = connectionConvertSdpSearchStatus(cfm->response);
        message->num_records = cfm->num_recs_ret;
        message->error_code = cfm->err_code;
        BdaddrConvertBluestackToVm(&message->bd_addr, &cfm->bd_addr);
        message->size_records = cfm->size_rec_list;
        
        if (cfm->size_rec_list)
        {
            uint8 *record_list = VmGetPointerFromHandle(cfm->rec_list);

            if (msgid == CL_SDP_SERVICE_SEARCH_CFM)
            {
                /* pass as a value */
                memmove(new_cfm->records, record_list, len);
                free(record_list);
            }
            else
            {
                /* pass as a reference */
                message->records = record_list;
            }
        }
        else
            message->records = NULL;
        
        MessageSend(state->sdpSearchLock, msgid, message);
        
        /* Reset the resource lock */
        state->sdpSearchLock = 0;
    }
    else
    {
        if (cfm->size_rec_list)
        {
            uint8 *record_list = VmGetPointerFromHandle(cfm->rec_list);
            free(record_list);
        }
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpAttributeSearchRequest

DESCRIPTION
	Request to issue an SDP attribute search request to BlueStack.

RETURNS
	void	
*/
void connectionHandleSdpAttributeSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ_T *req)
{
	sdp_search_req res = sendSearchRequest(req->theAppTask, state, &req->bd_addr);
	
	if (res == sdp_start_search)
	{
		/* Send the request to BlueStack */
		MAKE_PRIM_T(SDC_SERVICE_ATTRIBUTE_REQ);
		prim->phandle = 0;
        BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
		prim->svc_rec_hndl = req->service_handle;
		prim->size_attr_list = req->size_attribute_list;
		if (req->size_attribute_list)
		{
			uint8* attrs = (uint8 *)PanicUnlessMalloc(req->size_attribute_list);
            memmove(attrs, req->attribute_list, req->size_attribute_list);
            prim->attr_list = VmGetHandleFromPointer(attrs);
        }
        else
        {
            prim->attr_list = NULL;
        }
        
		prim->max_num_attr = req->max_num_attr;
		VmSendSdpPrim(prim);

		/* Lock the resource */
		state->sdpSearchLock = req->theAppTask;

        /* set the flags */
        ((connectionState*)connectionGetCmTask())->flags |= req->flags;
	}
	else
	{
		Task *c = 0;

		/* Queue up the search request */
		MAKE_CL_MESSAGE_WITH_LEN( CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ, 
                                  req->size_attribute_list );
		COPY_CL_MESSAGE_WITH_LEN( CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ, 
                                  req->size_attribute_list, req, message);

		/* Use the return value from searchRequest function to decide which resource to wait on */
		if (res == sdp_lock_set)
			c = &state->sdpLock;
		else if (res == sdp_search_lock_set)
			c = &state->sdpSearchLock;
		else if (res == sdp_session_open_other_dev)
			c = &state->sdpLock;
		else
			/* Should not get here but if we do panic */
			Panic();

		/* Queus up message with condition determined above */
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ, message, c);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpAttributeSearchCfm

DESCRIPTION
	A confirm to the attribute search has been received, handle it here.

RETURNS
	void	
*/
void connectionHandleSdpAttributeSearchCfm(connectionSdpState *state, const SDC_SERVICE_ATTRIBUTE_CFM_T *cfm)
{
    connectionState *cstate = (connectionState*)connectionGetCmTask();
    uint16 len;
    uint16 msgid;

    /* extra length required for a message */
    if (cstate->flags & CONNECTION_FLAG_SDP_REFERENCE)
    {
        cstate->flags &= ~CONNECTION_FLAG_SDP_REFERENCE;
        len = 0;
        msgid = CL_SDP_ATTRIBUTE_SEARCH_REF_CFM;
    }
    else
    {
        len = cfm->size_attr_list;
        msgid = CL_SDP_ATTRIBUTE_SEARCH_CFM;
    }

    if (state->sdpSearchLock)
    {
        CL_SDP_ATTRIBUTE_SEARCH_CFM_T *new_cfm=NULL;
        CL_SDP_ATTRIBUTE_SEARCH_REF_CFM_T *message;

        /* Send a cfm to the client task */	
        if (cfm->size_attr_list && (msgid == CL_SDP_ATTRIBUTE_SEARCH_CFM))
        {
            new_cfm = PanicUnlessMalloc(sizeof(CL_SDP_ATTRIBUTE_SEARCH_CFM_T) + len);
            message = (CL_SDP_ATTRIBUTE_SEARCH_REF_CFM_T*)new_cfm;
        }
        else
        {
            message = PanicUnlessMalloc(sizeof(CL_SDP_ATTRIBUTE_SEARCH_REF_CFM_T));
        }

        message->status = connectionConvertSdpSearchStatus(cfm->response);
        message->error_code = cfm->err_code;
        BdaddrConvertBluestackToVm(&message->bd_addr, &cfm->bd_addr);
        message->size_attributes = cfm->size_attr_list;	
        
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);

            if (msgid == CL_SDP_ATTRIBUTE_SEARCH_CFM)
            {
                /* pass as a value */
                memmove(new_cfm->attributes, attribute_list, len);
                free(attribute_list);
            }
            else
            {
                /* pass as a reference */
                message->attributes = attribute_list;
            }
        }
        else
            message->attributes = NULL;
        
        MessageSend(state->sdpSearchLock, msgid, message);
        
        /* Reset the resource lock */
        state->sdpSearchLock = 0;
    }
    else
    {
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);
            free(attribute_list);
        }
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchAttrRequest

DESCRIPTION
	Handle a request to perfor a service/ attribute search. If allowed, send
	request to BlueStack otherwise queue it up until it can be sent.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchAttrRequest(connectionSdpState *state, const CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ_T *req)
{
	sdp_search_req res = sendSearchRequest(req->theAppTask, state, &req->bd_addr);
	
	if (res == sdp_start_search)
	{
		/* Send the request to BlueStack */
		MAKE_PRIM_T(SDC_SERVICE_SEARCH_ATTRIBUTE_REQ);
		prim->phandle = 0;
        BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
		prim->size_srch_pttrn = req->size_search_pattern;

        if( req->size_search_pattern )
        {
            uint8* pattern = (uint8 *)PanicUnlessMalloc(req->size_search_pattern);
			memmove( pattern , req->search_attr, req->size_search_pattern);
            prim->srch_pttrn = VmGetHandleFromPointer( pattern ); 
		}
		else
        {
            prim->srch_pttrn = NULL;
        }

        prim->size_attr_list = req->size_attribute_list;

		if (req->size_attribute_list)
		{
			uint8* attrs = (uint8 *)PanicUnlessMalloc(req->size_attribute_list);
            memmove(attrs, req->search_attr + req->size_search_pattern, 
                    req->size_attribute_list);
            prim->attr_list = VmGetHandleFromPointer(attrs);
        }
        else
        {
            prim->attr_list = NULL;
        }

        prim->max_num_attr = req->max_num_attributes;
		VmSendSdpPrim(prim);

		/* Lock the resource */
		state->sdpSearchLock = req->theAppTask;

        /* set the flags */
        ((connectionState*)connectionGetCmTask())->flags |= req->flags;
	}
	else
	{
		Task *c = 0;
        uint16 len =req->size_search_pattern + req->size_attribute_list;

		/* Queue up the search request */
		MAKE_CL_MESSAGE_WITH_LEN(CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ, 
                                 len );
        COPY_CL_MESSAGE_WITH_LEN(CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ,
                                 len, req, message );         

		/* Use the return value from searchRequest function to decide which resource to wait on */
		if (res == sdp_lock_set)
			c = &state->sdpLock;
		else if (res == sdp_search_lock_set)
			c = &state->sdpSearchLock;
		else if (res == sdp_session_open_other_dev)
			c = &state->sdpLock;
		else
			/* Should not get here but if we do panic */
			Panic();

		/* Queus up message with condition determined above */
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ, message, c);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchAttributeCfm

DESCRIPTION
	HAndle the search cfm received from BlueStack.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchAttributeCfm(connectionSdpState *state, const SDC_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{	
    connectionState *cstate = (connectionState*)connectionGetCmTask();
    uint16 len;
    uint16 msgid;

    /* extra length required for a message */
    if (cstate->flags & CONNECTION_FLAG_SDP_REFERENCE)
    {
        cstate->flags &= ~CONNECTION_FLAG_SDP_REFERENCE;
        len = 0;
        msgid = CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM;
    }
    else
    {
        len = cfm->size_attr_list;
        msgid = CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM;
    }

    if (state->sdpSearchLock)
    {
        CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *new_cfm = NULL;
        CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM_T *message;

        if (cfm->size_attr_list && (msgid == CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM))
        {
            new_cfm = PanicUnlessMalloc(sizeof(CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T) + len);
            message = (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM_T*)new_cfm;
        }
        else
        {
            message = PanicUnlessMalloc(sizeof(CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM_T));
        }

        /* Send a cfm to the client task */
        message->status = connectionConvertSdpSearchStatus(cfm->response);
        message->more_to_come = cfm->more_to_come;
        message->error_code = cfm->err_code;
        BdaddrConvertBluestackToVm(&message->bd_addr, &cfm->bd_addr);
        message->size_attributes = cfm->size_attr_list;
        
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);
            
            if (msgid == CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM)
            {
                /* pass as a value */
                memmove(new_cfm->attributes, attribute_list, len);
                free(attribute_list);
            }
            else
            {
                /* pass as a reference */
                message->attributes = attribute_list;
            }
        }
        else
            message->attributes = NULL;
        
        MessageSend(state->sdpSearchLock, msgid, message);

        /* Reset the resource lock */
	    if (!cfm->more_to_come) 
            state->sdpSearchLock = 0;
    }
    else
    {
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);
            free(attribute_list);
        }
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpTerminatePrimitiveRequest

DESCRIPTION
	Send a request to BVlueStack to terminate the current SDP search.

RETURNS
	void	
*/
void connectionHandleSdpTerminatePrimitiveRequest(const connectionSdpState *state, const CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ_T *req)
{
	if (state->sdpSearchLock && (req->theAppTask == state->sdpSearchLock))
	{
		/* 
			There must be an active search ongoing and the task trying to cancel it 
			must be the same task that started the search
		*/		
		MAKE_PRIM_T(SDC_TERMINATE_PRIMITIVE_REQ);
		prim->phandle = 0;
		VmSendSdpPrim(prim);
	}
	/* 
		For now we just silently ignore the terminate primitive if it is issued when we 
		don't have an active search or when the task issuing it is not the same as the
		task that started the search.
	*/
}

/*lint +e725 +e830 */
#endif
