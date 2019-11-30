/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_baseband_handler.c        

DESCRIPTION
    This is the baseband management entity and is used to route responses
	back to the task that initiated the request.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_baseband_handler.h"

#include <bdaddr.h>
#include <vm.h>
#include <sink.h>

/****************************************************************************
NAME	
    connectionHandleReadClassOfDeviceRequest

DESCRIPTION
    Handles the internal message that initiates the read class of device

RETURNS
    void
*/
void connectionHandleReadClassOfDeviceRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ_T *req)
{
	/* Check the lock */
	if (!state->stateInfoLock)
	{
		/* Not currently processing one of these so set the lock */
		state->stateInfoLock = req->theAppTask;
        state->sink = 0;

		{
			/* Issue the request to BlueStack */
			MAKE_PRIM_C(DM_HCI_READ_CLASS_OF_DEVICE_REQ);
			VmSendDmPrim(prim);
		}
	}
	else
	{
		/* Currently busy processing a read cod request so queue this up */
		MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ, message, &state->stateInfoLock);
	}
}


/****************************************************************************
NAME	
    connectionHandleReadClassOfDeviceComplete

DESCRIPTION
    Handles the read class of device complete message from BlueStack

RETURNS
    void
*/
void connectionHandleReadClassOfDeviceComplete(connectionReadInfoState *state, const DM_HCI_READ_CLASS_OF_DEVICE_CFM_T *cfm)
{
    if (state->stateInfoLock)
    {
        /* Send result to task that originated the request */
        MAKE_CL_MESSAGE(CL_DM_CLASS_OF_DEVICE_CFM);
        message->status = connectionConvertHciStatus(cfm->status);
        message->dev_class = cfm->dev_class;
        MessageSend(state->stateInfoLock, CL_DM_CLASS_OF_DEVICE_CFM, message);
        
        /* Reset the resource lock */
        state->stateInfoLock = 0;
    }
}


/****************************************************************************
NAME	
    connectionHandleWritePageScanActivityRequest

DESCRIPTION
    Handles the internal message requesting a page scan activity write

RETURNS
    void
*/
void connectionHandleWritePageScanActivityRequest(const CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_PAGESCAN_ACTIVITY_REQ);
	prim->pagescan_interval = req->ps_interval;
	prim->pagescan_window = req->ps_window;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteInquiryScanActivityRequset

DESCRIPTION
    Handles the internal message requesting a inquiry scan activity write

RETURNS
    void
*/
void connectionHandleWriteInquiryScanActivityRequest(const CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_INQUIRYSCAN_ACTIVITY_REQ);
	prim->inqscan_interval = req->is_interval;
	prim->inqscan_window = req->is_window;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteInquiryScanTypeRequset

DESCRIPTION
    Handles the internal message requesting a write inquiry scan type

RETURNS
    void
*/
void connectionHandleWriteInquiryScanTypeRequest(const CL_INTERNAL_DM_WRITE_INQUIRY_SCAN_TYPE_REQ_T *req)
{
    MAKE_PRIM_C(DM_HCI_WRITE_INQUIRY_SCAN_TYPE_REQ);

    prim->mode = req->type;

    VmSendDmPrim(prim);
}

/****************************************************************************
NAME	
    connectionHandleWritePageScanTypeRequset

DESCRIPTION
    Handles the internal message requesting a write page scan type

RETURNS
    void
*/
void connectionHandleWritePageScanTypeRequest(const CL_INTERNAL_DM_WRITE_PAGE_SCAN_TYPE_REQ_T *req)
{
    MAKE_PRIM_C(DM_HCI_WRITE_PAGE_SCAN_TYPE_REQ);

    prim->mode = req->type;

    VmSendDmPrim(prim);
}

/****************************************************************************
NAME	
    connectionHandleWriteScanEnableRequset

DESCRIPTION
    Handles the internal message requesting a write scan enable

RETURNS
    void
*/
void connectionHandleWriteScanEnableRequest(const CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_SCAN_ENABLE_REQ);
	prim->scan_enable = connectionConvertHciScanEnable(req->mode);
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteCodRequest

DESCRIPTION
    Handles the internal message requesting a write class of device

RETURNS
    void
*/
void connectionHandleWriteCodRequest(const CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_CLASS_OF_DEVICE_REQ);
	prim->dev_class = req->class_of_device;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteCachedPageModeRequest

DESCRIPTION
    Handles the internal message requesting a to store the page mode for a given device

RETURNS
    void
*/
void connectionHandleWriteCachedPageModeRequest(const CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ_T *req)
{
	MAKE_PRIM_T(DM_WRITE_CACHED_PAGE_MODE_REQ);
	BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
    prim->page_scan_mode = connectionConvertPageScanMode(req->ps_mode);
    prim->page_scan_rep_mode = connectionConvertPageScanRepMode(req->ps_rep_mode);
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteCachedClkOffsetRequest

DESCRIPTION
    Handles the internal message requesting a to store the clock offset for 
	a given device

RETURNS
    void
*/
void connectionHandleWriteCachedClkOffsetRequest(const CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ_T *req)
{
	/* Send the request to BlueStack */
	MAKE_PRIM_T(DM_WRITE_CACHED_CLOCK_OFFSET_REQ);
	BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
	prim->clock_offset = req->clock_offset;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleClearParamCacheRequest

DESCRIPTION
    Handles the internal message requesting to clear the parameter cache for 
	a given device

RETURNS
    void
*/
void connectionHandleClearParamCacheRequest(const CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ_T *req)
{
	/* Send the request to BlueStack */
	MAKE_PRIM_T(DM_CLEAR_PARAM_CACHE_REQ);
	BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteFlushTimeoutRequest

DESCRIPTION
    Set the flush timeout for a particular ACL.

RETURNS
    void
*/
void connectionHandleWriteFlushTimeoutRequest(const CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ_T *req)
{
	tp_bdaddr tpaddr;

	/* 
		Convert the sink to the address of the underlying ACL and 
		only issue this if we have an ACL  
	*/
	if ( (SinkGetBdAddr(req->sink, &tpaddr)) )
	{
        /* Send the request to BlueStack */
        MAKE_PRIM_C(DM_HCI_WRITE_AUTO_FLUSH_TIMEOUT_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
        prim->timeout = req->flush_timeout;
        VmSendDmPrim(prim);
    }
}


/****************************************************************************/
void connectionHandleChangeLocalName(const CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ_T *req)
{
	uint16 i;
	MAKE_PRIM_C(DM_HCI_CHANGE_LOCAL_NAME_REQ); 
	prim->common.length = sizeof(DM_HCI_CHANGE_LOCAL_NAME_REQ_T) + req->length_name;

	if (req->length_name)
		prim->name_part[0] = VmGetHandleFromPointer(req->name);
	else
		prim->name_part[0] = 0;

	/* Set all the other ptrs in the message to null */
	for (i = 1; i < HCI_LOCAL_NAME_BYTE_PACKET_PTRS; i++)
		prim->name_part[i] = 0;

	VmSendDmPrim(prim);
}
