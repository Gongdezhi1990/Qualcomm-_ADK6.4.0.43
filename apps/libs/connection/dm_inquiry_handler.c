/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_inquiry_handler.c        

DESCRIPTION
    This file contains the implementation of the inquiry management
    entity. This is responsible for ensuring that only one inquiry
    request at a time is issued to Bluestack. It also manages the 
    inquiry process by filtering on class of device and initiating
    remote name requests (if requested by the client).

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_inquiry_handler.h"

#include <bdaddr.h>
#include <string.h>
#include <vm.h>

#ifndef CL_EXCLUDE_INQUIRY

/* Size to limit EIR data to */
#define MAX_EIR_DATA_SIZE 40

/****************************************************************************
NAME	
    inquirySendResult

DESCRIPTION
    Send a CL_DM_INQUIRE_RESULT message to the client

RETURNS
    void
*/
static void inquirySendResult(Task theAppTask, inquiry_status status, const bdaddr *addr, uint32 dev_class, uint16 clock_offset, page_scan_rep_mode_t ps_rep_mode, page_scan_mode_t ps_mode, int16 rssi, uint8 size_eir_data, uint8* eir_data)
{
    if (theAppTask)
    {	
        /* Create an inquiry result message and populate it with the supplied args */
        MAKE_CL_MESSAGE_WITH_LEN(CL_DM_INQUIRE_RESULT,size_eir_data);
        message->status = status;
        
        /* Check the address has been set */
        if (addr)
            message->bd_addr = *addr;
        else
            BdaddrSetZero(&message->bd_addr);
        
        message->dev_class = dev_class;
        message->clock_offset = clock_offset;
        message->page_scan_rep_mode = connectionConvertPageScanRepMode_t(ps_rep_mode);
        message->page_scan_mode = connectionConvertPageScanMode_t(ps_mode);
        message->rssi = rssi;
		message->size_eir_data = size_eir_data;
		if(size_eir_data)
		{
            memmove(message->eir_data, eir_data, size_eir_data);
		}
		else
		{
			message->eir_data[0] = 0;
		}
        MessageSend(theAppTask, CL_DM_INQUIRE_RESULT, message);
    }
}


/****************************************************************************
NAME	
    inquiryAddEirData

PARAMS
    [IN/OUT] size_eir_data  Current size of the eir_data
    [OUT]    eir_data       Buffer into which the EIR data needs to be copied
    [IN]     size_data      Length of data to be copied
    [IN]     eir_data_part  Array from which the data needs to be copied from

DESCRIPTION
    Copy a block of data from an eir data part to the eir result.

RETURNS
    
*/
static uint8* inquiryAddEirData(uint8* size_eir_data, uint8* eir_data,
        uint8 size_data, const uint8* eir_data_part)
{
    /* Allocate space for the new data in result */
    uint8* result = PanicNull( realloc(eir_data, (*size_eir_data) + size_data) );
    /* Copy data from data part to result */
    memmove( (result + (*size_eir_data)), eir_data_part, size_data );
    /* Update size */
    *size_eir_data += size_data;
    
    return result;
}


/****************************************************************************
NAME    
    inquiryParseEir

DESCRIPTION
    Take the EIR data structure received from bluestack and concat the data 
	into a single block to send to the application

RETURNS
    A pointer to a single block containing the EIR data and the size of the 
	eir data
*/
static uint8* inquiryParseEir(uint8* size_eir_data, uint8 *inquiry_data[HCI_EIR_DATA_PACKET_PTRS], bool limit_data_size)
{
    /* Data Part */
    uint8 i = 0;                    /* Index into inquiry_data */
    uint8 size_data_part;           /* Size un-parsed data left in current part */
    uint8* eir_data_part;           /* Pointer to start of un-parsed data */
    uint8* eir_data_part_base;      /* Pointer to base of the current data part */
	
    /* Field Data */
    uint8 size_field;               /* Size of the field we're parsing */
    bool limit_exceeded;            /* If the field should be written */
	
    /* Result */
    uint8* eir_data = NULL;         /* Parsed EIR Data */
    *size_eir_data  = 0;            /* Size of Parsed Data */
	
    /* Return early if we have nothing to parse */
    if(inquiry_data[0] == NULL)
		{
        return eir_data;
    }
		
    /* Setup First Data Part */
    size_data_part = HCI_EIR_DATA_BYTES_PER_PTR;
    eir_data_part_base = VmGetPointerFromHandle(inquiry_data[i]);
    eir_data_part = eir_data_part_base;
	
    /* Get the size of the first field */
    size_field = (*eir_data_part) + 1;
    limit_exceeded = (((*size_eir_data) + size_field) > MAX_EIR_DATA_SIZE);
		
    /* Keep going until we reach NULL terminator */
    while(*eir_data_part != 0)
    {
        /* If the field is all in this part */
        if(size_field < size_data_part)
        {
            /* Copy to the end of field */
            if(!limit_data_size || !limit_exceeded)
                eir_data = inquiryAddEirData(size_eir_data, eir_data, size_field, eir_data_part);
            /* Move to the next field */
            size_data_part -= size_field;
            eir_data_part += size_field;
            size_field = (*eir_data_part) + 1;
            limit_exceeded = (((*size_eir_data) + size_field) > MAX_EIR_DATA_SIZE);
        }
        /* Field is split over multiple parts */
        else
        {
            /* Copy to the end of the data part */
            if(!limit_data_size || !limit_exceeded)
                eir_data = inquiryAddEirData(size_eir_data, eir_data, size_data_part, eir_data_part);
	
            /* Check if there's any more data parts to go */
            if(inquiry_data[++i] != NULL)
            {
                /* We're finished with this part so free it */
                free(eir_data_part_base);
	
                /* Move to the next data part */
                size_field -= size_data_part;
                size_data_part = HCI_EIR_DATA_BYTES_PER_PTR;
                eir_data_part_base = VmGetPointerFromHandle(inquiry_data[i]);
                eir_data_part = eir_data_part_base;
	
                /* Update field size if new field */
                if(size_field == 0)
                {
                    size_field = (*eir_data_part) + 1;
                    limit_exceeded = (((*size_eir_data) + size_field) > MAX_EIR_DATA_SIZE);
                }
            }
            /* Out of data */
            else
            {
                /* Data part will be freed below, just break */
                break;
            }
        }
	}
	
    /* Free the final data part */
    free(eir_data_part_base);
    
    /* Free the non-significant data parts (containing all zeros) if the memory exists */
    while (((eir_data_part_base = VmGetPointerFromHandle(inquiry_data[++i])) != NULL) && (i < HCI_EIR_DATA_PACKET_PTRS))
    {
        free(eir_data_part_base);
    }
            
    /* NULL Terminate Data */
    (*size_eir_data)++;
    eir_data = PanicNull( realloc(eir_data, *size_eir_data) );
    *(eir_data + (*size_eir_data) - 1) = 0;
    
	return eir_data;
}


/****************************************************************************
  NAME	
      connectionStartInquiry
  
  DESCRIPTION
      This function actually kicks off an inquiry message to BlueStack.
  
  RETURNS
      void
*/
void connectionHandleInquiryStart(connectionInquiryState *state, const CL_INTERNAL_DM_INQUIRY_REQ_T *inquiry_req)
{
	/* Check the state of the task lock before doing anything */
	if (!state->inquiryLock)
	{
		state->inquiryLock = inquiry_req->theAppTask;

		/* 
			If the class of device field is set, set an event filter so we 
			only receive inquiry results with devices with this class of 
			device. By default we filter in all devices so if class of 
			device mask set to zero we don't have to set the event filter.
		*/
		if (inquiry_req->class_of_device)
		{
            MAKE_PRIM_C(DM_HCI_SET_EVENT_FILTER_REQ);
			prim->filter_type = INQUIRY_RESULT_FILTER;
			prim->filter_condition_type = CLASS_OF_DEVICE_RESPONDED;
			prim->condition.class_mask.class_of_device = inquiry_req->class_of_device;

	        /* 
				We don't care what the bits other than the specific class 
				of device are set to so set them to zero 
			*/
			prim->condition.class_mask.mask = inquiry_req->class_of_device;
			VmSendDmPrim(prim);
		}

	    {
    if (inquiry_req->min_period)
    {
        /* Start an periodic inquiry with the supplied parameters */
        MAKE_PRIM_C(DM_HCI_PERIODIC_INQUIRY_MODE_REQ);
        prim->lap = inquiry_req->inquiry_lap;
        prim->inquiry_length = inquiry_req->timeout;
        prim->num_responses = inquiry_req->max_responses;
        prim->max_period_length = inquiry_req->max_period;
        prim->min_period_length = inquiry_req->min_period;                
        VmSendDmPrim(prim);
        state->periodic_inquiry = TRUE;
    }
    else
    {
		    /* Start an inquiry with the supplied parameters */
            MAKE_PRIM_C(DM_HCI_INQUIRY_REQ);
			prim->lap = inquiry_req->inquiry_lap;
			prim->inquiry_length = inquiry_req->timeout;
			prim->num_responses = inquiry_req->max_responses;
			VmSendDmPrim(prim);
        state->periodic_inquiry = FALSE;
		}
	}
    }
	else
	{
		/* Inquiry currently being performed, queue up the request */
		MAKE_CL_MESSAGE(CL_INTERNAL_DM_INQUIRY_REQ);
		COPY_CL_MESSAGE(inquiry_req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_INQUIRY_REQ, message, &state->inquiryLock);
	}
}


/****************************************************************************
NAME	
    connectionHandleInquiryResult

DESCRIPTION
    This function handles inquiry results as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleInquiryResult(const connectionInquiryState *state, const DM_HCI_INQUIRY_RESULT_IND_T *inq_result)
{
	uint16 array;
	uint16 index;
	uint16 results_left = inq_result->num_responses;

	/* Iterate through the array of inquiry result ptrs */
	for (array = 0; array < (inq_result->num_responses+HCI_MAX_INQ_RESULT_PER_PTR-1)/HCI_MAX_INQ_RESULT_PER_PTR; array++)
	{
		uint16 res_this_block = HCI_MAX_INQ_RESULT_PER_PTR;
		HCI_INQ_RESULT_T *resarray = (HCI_INQ_RESULT_T *) VmGetPointerFromHandle(inq_result->result[array]);
		
		if (results_left < res_this_block)
			res_this_block = results_left;
		
		for (index = 0; index< res_this_block; index++)
		{
			/* Send an inquiry result message to the client */
			bdaddr	addr;
			BdaddrConvertBluestackToVm(&addr, &((resarray+index)->bd_addr));
			inquirySendResult(state->inquiryLock, inquiry_status_result, &addr, 
				(resarray+index)->dev_class, (resarray+index)->clock_offset,
				(resarray+index)->page_scan_rep_mode, (resarray+index)->page_scan_mode,
				CL_RSSI_UNKNOWN, 0, NULL);
		}
		
		/* Free the data */
		free(resarray);
	}
}


/****************************************************************************
NAME	
    connectionHandleInquiryResultWithRssi

DESCRIPTION
    This function handles inquiry results with RSSI as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleInquiryResultWithRssi(const connectionInquiryState *state, const DM_HCI_INQUIRY_RESULT_WITH_RSSI_IND_T*inq_result)
{
	uint16 array;
	uint16 index;
	uint16 results_left = inq_result->num_responses;

	/* Iterate through the array of inquiry result ptrs */
	for (array = 0; array < (inq_result->num_responses+HCI_MAX_INQ_RESULT_PER_PTR-1)/HCI_MAX_INQ_RESULT_PER_PTR; array++)
	{
		uint16 res_this_block = HCI_MAX_INQ_RESULT_PER_PTR;
		HCI_INQ_RESULT_WITH_RSSI_T *resarray = (HCI_INQ_RESULT_WITH_RSSI_T *) VmGetPointerFromHandle(inq_result->result[array]);
		
		if (results_left < res_this_block)
			res_this_block = results_left;
		
		for (index = 0; index< res_this_block; index++)
		{
			/* Send an inquiry result message to the client */
			bdaddr	addr;
			BdaddrConvertBluestackToVm(&addr, &((resarray+index)->bd_addr));
			inquirySendResult(state->inquiryLock, inquiry_status_result, &addr, 
				(resarray+index)->dev_class, (resarray+index)->clock_offset,
				(resarray+index)->page_scan_rep_mode, 0, (int16)(resarray+index)->rssi,
				0, NULL);
		}
		
		/* Free the data */
		free(resarray);
	}
}


/****************************************************************************
NAME	
    connectionHandleExtendedInquiryResult

DESCRIPTION
    This function handles extended inquiry results as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleExtendedInquiryResult(const connectionInquiryState *state, const DM_HCI_EXTENDED_INQUIRY_RESULT_IND_T *ind)
{
    uint8 size_eir_data = 0;
	
    uint8* eir_data = inquiryParseEir(&size_eir_data, (uint8**)ind->eir_data_part, TRUE);
		
	bdaddr	addr;
	BdaddrConvertBluestackToVm(&addr, &ind->result.bd_addr);
	
	/* Send an inquiry result message to the client */
	inquirySendResult(state->inquiryLock, inquiry_status_result, &addr, 
					 ind->result.dev_class, ind->result.clock_offset,
					 ind->result.page_scan_rep_mode, 0, (int16)ind->result.rssi,
					 size_eir_data, eir_data);
	
	/* Free memory */
	if(size_eir_data)
		free(eir_data);
}


/****************************************************************************
NAME	
    connectionHandleInquiryComplete

DESCRIPTION
    This function handles an inquiry complete indication fromBlueStack.

RETURNS
    void
*/
void connectionHandleInquiryComplete(connectionInquiryState *state)
{
	/* Tell the client the inquiry has completed */
    inquirySendResult(state->inquiryLock, inquiry_status_ready, 0, (uint32) 0, 0, 0, 0, CL_RSSI_UNKNOWN, 0, NULL);

    /* Clear lock and reset event filter if not periodic inquiry */
    if (!state->periodic_inquiry)
	{
		/* Reset the event filter so its not longer filtering on cod */
        MAKE_PRIM_C(DM_HCI_SET_EVENT_FILTER_REQ);
		prim->filter_type = INQUIRY_RESULT_FILTER;
		prim->filter_condition_type = NEW_DEVICE_RESPONDED;
		VmSendDmPrim(prim);

        /* Reset the lock */
        state->inquiryLock = 0;
	}
}


/****************************************************************************
NAME	
    connectionHandleExitPeriodicInquiryComplete

DESCRIPTION
    This function handles an exit periodic inquiry complete indication from
    BlueStack.

RETURNS
    void
*/
void connectionHandleExitPeriodicInquiryComplete(connectionInquiryState *state)
{
    /* Clear lock and reset event filter if not periodic inquiry */
    if (state->periodic_inquiry)
	{
        /* Reset the event filter so its not longer filtering on cod */
        MAKE_PRIM_C(DM_HCI_SET_EVENT_FILTER_REQ);
        prim->filter_type = INQUIRY_RESULT_FILTER;
        prim->filter_condition_type = NEW_DEVICE_RESPONDED;
			VmSendDmPrim(prim);

        /* Reset the lock */
        state->inquiryLock = 0;
	}
}

/****************************************************************************
NAME	
    connectionHandleInquiryCancel

DESCRIPTION
    This function cancels an ongoing inquiry

RETURNS
    void
*/
void connectionHandleInquiryCancel(const connectionInquiryState *state, const CL_INTERNAL_DM_INQUIRY_CANCEL_REQ_T *cancel_req)
{
    /* Check if we have an inquiry in progress */
    if (state->inquiryLock)
    {
        if (state->inquiryLock == cancel_req->theAppTask)
        {
            if (state->periodic_inquiry)
    {
                /* We have an periodic inquiry so issue the exit */
                MAKE_PRIM_C(DM_HCI_EXIT_PERIODIC_INQUIRY_MODE_REQ);
                VmSendDmPrim(prim);
		}
	else
	{
                /* We have an inquiry so issue the cancel */
                MAKE_PRIM_C(DM_HCI_INQUIRY_CANCEL_REQ);
			VmSendDmPrim(prim);
		}
	}
	}
        else
    {
        /* Send an inquiry complete to the app telling it we're ready */
        inquirySendResult(cancel_req->theAppTask, inquiry_status_ready, 0, (uint32) 0, 0, 0, 0, CL_RSSI_UNKNOWN, 0, NULL);
	}
}

/****************************************************************************
NAME	
    connectionHandleWriteInquiryTx

DESCRIPTION
    This function will initiate a write of the inquiry tx power of the device

RETURNS
    void
*/
void connectionHandleWriteInquiryTx(connectionReadInfoState* infoState, const CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ_T *req)
{
	/* Check command supported by firmware */
	if(infoState->version != bluetooth_unknown)
	{
		MAKE_PRIM_C(DM_HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL_REQ);
		prim->tx_power = req->tx_power;
		VmSendDmPrim(prim);
	}
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryTx

DESCRIPTION
    This function will initiate a read of the inquiry tx power of the device

RETURNS
    void
*/
void connectionHandleReadInquiryTx(connectionReadInfoState* infoState, connectionInquiryState *state, const CL_INTERNAL_DM_READ_INQUIRY_TX_REQ_T *req)
{
    /* Check command supported by firmware */
    if(infoState->version != bluetooth_unknown)
    {
        /* Check the state of the task lock before doing anything */
        if (!state->inquiryLock)
        {
            /* One request at a time */
            state->inquiryLock = req->theAppTask;

            /* Issue request to read the inquiry tx */
            {
                MAKE_PRIM_C(DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_REQ);
                VmSendDmPrim(prim);
            }
        }
        else
        {
            /* Remote name request currently being performed, queue up the request */
            MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_INQUIRY_TX_REQ);
            COPY_CL_MESSAGE(req, message);
            MessageSendConditionallyOnTask(connectionGetCmTask(), 
                                           CL_INTERNAL_DM_READ_INQUIRY_TX_REQ, 
                                           message, &state->inquiryLock);
        }
    }
    else
    {
        /* Tell the app this is unsupported */
        MAKE_CL_MESSAGE(CL_DM_READ_INQUIRY_TX_CFM);
        message->status = hci_error_unsupported_feature;
        message->tx_power = 0;
        MessageSend(req->theAppTask, CL_DM_READ_INQUIRY_TX_CFM, message);
    }
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryTx

DESCRIPTION
    This function handles a read inquiry tx result

RETURNS
    void
*/
void connectionHandleReadInquiryTxComplete(connectionInquiryState *state, const DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_CFM_T *cfm)
{
	if(state->inquiryLock)
	{
		MAKE_CL_MESSAGE(CL_DM_READ_INQUIRY_TX_CFM);
		message->status = connectionConvertHciStatus(cfm->status);
		message->tx_power = cfm->tx_power;
		MessageSend(state->inquiryLock, CL_DM_READ_INQUIRY_TX_CFM, message);
                
                state->inquiryLock = 0;
	}	
}

/****************************************************************************
NAME	
    connectionHandleWriteIacLapRequest

DESCRIPTION
    Write IAC

RETURNS
    void
*/
void connectionHandleWriteIacLapRequest(connectionInquiryState *state, const CL_INTERNAL_DM_WRITE_IAC_LAP_REQ_T *req)
{
	/* Check the state of the task lock before doing anything */
	if (!state->iacLock)
	{
		uint16 index;
		uint24_t *ptr;
        MAKE_PRIM_C(DM_HCI_WRITE_CURRENT_IAC_LAP_REQ);

		/* One request at a time */
		state->iacLock = req->theAppTask;

		/* Store number of IACs */
		prim->num_current_iac = req->num_iac;
	
		/* Zero the entries */
		memset(prim->iac_lap, 0, sizeof(uint24_t *) * HCI_IAC_LAP_PTRS);

		/* Allocate memory block */
		prim->iac_lap[0] = (uint24_t *)malloc(sizeof(uint24_t) * prim->num_current_iac);
		ptr = prim->iac_lap[0];

		/* Copy IACs */
		for (index = 0; index < req->num_iac; index++)
			ptr[index] = req->iac[index];
	
		/* Vm friendly */
		prim->iac_lap[0] = VmGetHandleFromPointer(prim->iac_lap[0]);

		/* Send request */
		VmSendDmPrim(prim);
	}
	else
	{
		/* Inquiry currently being performed, queue up the request */
		MAKE_CL_MESSAGE_WITH_LEN(CL_INTERNAL_DM_WRITE_IAC_LAP_REQ, sizeof(uint32) * req->num_iac);
		COPY_CL_MESSAGE_WITH_LEN(CL_INTERNAL_DM_WRITE_IAC_LAP_REQ, sizeof(uint32) * req->num_iac, req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_IAC_LAP_REQ, message, &state->iacLock);
	}
}

/****************************************************************************
NAME	
    connectionHandleWriteIacLapComplete

DESCRIPTION
    Write IAC confirmation

RETURNS
    void
*/
void connectionHandleWriteIacLapComplete(connectionInquiryState *state, const DM_HCI_WRITE_CURRENT_IAC_LAP_CFM_T *prim)
{
    if (state->iacLock)
    {
        MAKE_CL_MESSAGE(CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM);
        if (!prim->status)
            message->status = success;
        else
            message->status = fail;
        
        MessageSend(state->iacLock,
                    CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM,
                    message);

	/* Reset resource lock */
	state->iacLock = 0;
    }
}

/****************************************************************************
NAME	
    connectionHandleWriteInquiryModeRequest

DESCRIPTION
    Write inquiry mode

RETURNS
    void
*/
void connectionHandleWriteInquiryModeRequest(connectionInquiryState *state, const CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ_T *req)
{
	/* Check the state of the task lock before doing anything */
	if (!state->inquiryLock)
	{
		state->inquiryLock = req->theAppTask;

	    {   /* Issue inquiry mode change with the supplied parameter */
            MAKE_PRIM_C(DM_HCI_WRITE_INQUIRY_MODE_REQ);
			prim->mode = connectionConvertInquiryMode_t(req->mode);
			VmSendDmPrim(prim);
		}
	}
	else
	{
		/* Inquiry currently being performed, queue up the request */
		MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ, message, &state->inquiryLock);
	}
}

/****************************************************************************
NAME	
    connectionHandleWriteInquiryModeComplete

DESCRIPTION
    Write inquiry mode result

RETURNS
    void
*/
void connectionHandleWriteInquiryModeComplete(connectionInquiryState *state, const DM_HCI_WRITE_INQUIRY_MODE_CFM_T *prim)
{
    if (state->inquiryLock)
    {
        MAKE_CL_MESSAGE(CL_DM_WRITE_INQUIRY_MODE_CFM);
        message->status = (!prim->status)?success:fail;
        
        MessageSend(state->inquiryLock,
                    CL_DM_WRITE_INQUIRY_MODE_CFM,
                    message);
        
        state->inquiryLock = 0;
    }
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryModeRequest

DESCRIPTION
    Read inquiry mode

RETURNS
    void
*/
void connectionHandleReadInquiryModeRequest(connectionInquiryState *state, const CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ_T *req)
{
	/* Check the state of the task lock before doing anything */
	if (!state->inquiryLock)
	{
		state->inquiryLock = req->theAppTask;

	    {   /* Issue inquiry mode read */
            MAKE_PRIM_C(DM_HCI_READ_INQUIRY_MODE_REQ);
			VmSendDmPrim(prim);
		}
	}
	else
	{
		/* Inquiry currently being performed, queue up the request */
		MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ);
		COPY_CL_MESSAGE(req, message);
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ, message, &state->inquiryLock);
	}
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryModeComplete

DESCRIPTION
    Read inquiry mode result

RETURNS
    void
*/
void connectionHandleReadInquiryModeComplete(connectionInquiryState *state, const DM_HCI_READ_INQUIRY_MODE_CFM_T *prim)
{
    if (state->inquiryLock)
    {
        MAKE_CL_MESSAGE(CL_DM_READ_INQUIRY_MODE_CFM);
        message->status = (!prim->status)?success:fail;
        message->mode = connectionConvertInquiryMode(prim->mode);
        
        MessageSend(state->inquiryLock, CL_DM_READ_INQUIRY_MODE_CFM, message);
        state->inquiryLock = 0;
    }
}


/****************************************************************************
NAME
    connectionHandleWriteEirDataRequest

DESCRIPTION
    Handles request for Writing the Extended Inquiry Data. 

RETURNS
    void
*/
void connectionHandleWriteEirDataRequest(connectionReadInfoState *infoState, const CL_INTERNAL_DM_WRITE_EIR_DATA_REQ_T *req)
{
    uint8 i;
    uint8 *p;
    uint8 octets_copied = 0;
    uint8 remainder;
    uint8 eir_data_length;

    if(infoState->version >= bluetooth2_1)
        /* Extended Inquiry Response (EIR) is supported from version 2.1 onwards) */
	{
        MAKE_PRIM_C(DM_HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA_REQ);
    	prim->fec_required = req->fec_required;

   		eir_data_length = (req->size_eir_data <= HCI_EIR_DATA_LENGTH)? req->size_eir_data : HCI_EIR_DATA_LENGTH;  

    	for (i=0; i<(eir_data_length / HCI_EIR_DATA_BYTES_PER_PTR); i++)
    	{
			p = PanicUnlessMalloc(HCI_EIR_DATA_BYTES_PER_PTR);
            memmove(p, req->eir_data + octets_copied, HCI_EIR_DATA_BYTES_PER_PTR);
        	octets_copied += HCI_EIR_DATA_BYTES_PER_PTR;
			prim->eir_data_part[i] = VmGetHandleFromPointer(p);
    	}

    	remainder = eir_data_length % HCI_EIR_DATA_BYTES_PER_PTR;
    	if (remainder)
    	{
        	p = PanicUnlessMalloc(HCI_EIR_DATA_BYTES_PER_PTR);
            memmove(p, req->eir_data+octets_copied, remainder);
        	memset(p + remainder, 0, HCI_EIR_DATA_BYTES_PER_PTR - remainder);
			prim->eir_data_part[i] = VmGetHandleFromPointer(p);
        	i++;
    	}
    	for (; i < HCI_EIR_DATA_PACKET_PTRS; i++)
    	{
        	prim->eir_data_part[i] = NULL;
    	}

    	VmSendDmPrim(prim);
	}

    if(req->eir_data)
        free(req->eir_data);
}

/****************************************************************************
NAME
    connectionHandleReadEirDataRequest

DESCRIPTION
    Handles request for Reading the Extended Inquiry Data.

RETURNS
    void
*/
void connectionHandleReadEirDataRequest(connectionReadInfoState *infoState, connectionInquiryState *state, const CL_INTERNAL_DM_READ_EIR_DATA_REQ_T *req)
{
    if(infoState->version >= bluetooth2_1)
	{
		/* Check the state of the task lock before doing anything */
		if (!state->inquiryLock)
		{
			state->inquiryLock = req->task;
			{
                MAKE_PRIM_C(DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_REQ);
				VmSendDmPrim(prim);
			}
		}
		else
		{
			/* Inquiry currently being performed, queue up the request */
			MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_EIR_DATA_REQ);
			COPY_CL_MESSAGE(req, message);
			MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_EIR_DATA_REQ, message, &state->inquiryLock);
		}
	}
	else
	{
		/* Not supported, tell the app */
		MAKE_CL_MESSAGE(CL_DM_READ_EIR_DATA_CFM);
		message->status = hci_error_unsupported_feature;
		message->fec_required = FALSE;
		message->size_eir_data = 0;
		message->eir_data[0] = 0;
    	MessageSend(state->inquiryLock, CL_DM_READ_EIR_DATA_CFM, message);
	}
}


/****************************************************************************
NAME
    connectionHandleReadEirDataComplete

DESCRIPTION
    Handles result from Reading the Extended Inquiry Data.

RETURNS
    void
*/
void connectionHandleReadEirDataComplete(connectionInquiryState *state, const DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_CFM_T *cfm)
{
	if(state->inquiryLock)
	{
		uint8 size_eir_data = 0;
        uint8* eir_data = inquiryParseEir(&size_eir_data, (uint8**)cfm->eir_data_part, FALSE);
		
		MAKE_CL_MESSAGE_WITH_LEN(CL_DM_READ_EIR_DATA_CFM, size_eir_data);
		message->status = connectionConvertHciStatus(cfm->status);
		message->fec_required = cfm->fec_required;
		message->size_eir_data = size_eir_data;
        memmove(message->eir_data, eir_data, size_eir_data);
    	MessageSend(state->inquiryLock, CL_DM_READ_EIR_DATA_CFM, message);
		
		free(eir_data);
	}
	state->inquiryLock = 0;
}

#endif

/****************************************************************************
NAME    
    connectionHandleReadRemoteName

DESCRIPTION
    This function will initiate a read of the remote name of the specified device

RETURNS
    void
*/
void connectionHandleReadRemoteName(connectionInquiryState *state, const CL_INTERNAL_DM_READ_REMOTE_NAME_REQ_T *req)
{
    /* Check the state of the task lock before doing anything */
    if (!state->nameLock)
    {
        /* One request at a time */
        state->nameLock = req->theAppTask;
        
        /* Issue request to read the remote name */
        {
        MAKE_PRIM_C(DM_HCI_REMOTE_NAME_REQUEST_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
        VmSendDmPrim(prim);
        }
    }
    else
    {
        /* Remote name request currently being performed, queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_NAME_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_REMOTE_NAME_REQ, message, &state->nameLock);
    }

}


/****************************************************************************
NAME    
    connectionHandleReadRemoteNameCancel

DESCRIPTION
    This function will cancel the ongoing RNR

RETURNS
    void
*/
void connectionHandleReadRemoteNameCancel(connectionInquiryState *state,
                   const CL_INTERNAL_DM_READ_REMOTE_NAME_CANCEL_REQ_T *req)
{
    /* Check a remote name request is already on process */
    if ( state->nameLock && ( state->nameLock == req->theAppTask ) )
    {
        /* Issue request to cancel read the remote name */
        MAKE_PRIM_C(DM_HCI_REMOTE_NAME_REQ_CANCEL_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
        VmSendDmPrim(prim);
    }

/* else ignore the request since RNR must have completed or already cancelled */
}



/****************************************************************************
NAME    
    remoteNameComplete

DESCRIPTION
    Send a CL_DM_REMOTE_NAME_COMPLETE message to the client

RETURNS
    void
*/
static void remoteNameComplete(Task theAppTask, const bdaddr *addr, hci_status status, char* name, uint16 length)
{
    if (theAppTask)
    {
        MAKE_CL_MESSAGE_WITH_LEN(CL_DM_REMOTE_NAME_COMPLETE, length);
        message->status = status;
        
        /* Check the address has been set */
        if (addr)
        {
            message->bd_addr = *addr;
        }
        else
        {
            BdaddrSetZero(&message->bd_addr);
        }
        
        message->size_remote_name = length;
        
        /* Populate remote name fields */
        if (length)
        {
            memmove(message->remote_name, name, length);
        }    
        else
        {
            message->remote_name[0] = 0;    
        }
        
        free(name);
        
        /* Send onto Client */
        MessageSend(theAppTask, CL_DM_REMOTE_NAME_COMPLETE, message);
    }
    else
    {
        if (length)
            free(name);
    }
}

/****************************************************************************
NAME    
    connectionHandleRemoteNameCancel

DESCRIPTION
    Remote name cancel

RETURNS
    void
*/
void connectionHandleRemoteNameCancelCfm( connectionInquiryState *state, 
                             const DM_HCI_REMOTE_NAME_REQ_CANCEL_CFM_T* prim)
{
    bdaddr    addr;
    BdaddrConvertBluestackToVm(&addr, &prim->bd_addr);

    /* Read failed, send Client message to notify them of the result */
    remoteNameComplete(state->nameLock, &addr, rnr_cancelled, NULL, 0);

    /* Reset resource lock */
    state->nameLock = 0;
}
    

/****************************************************************************
NAME    
    connectionHandleRemoteNameComplete

DESCRIPTION
    Remote name result

RETURNS
    void
*/
void connectionHandleRemoteNameComplete(connectionInquiryState *state, const DM_HCI_REMOTE_NAME_CFM_T* prim)
{
    bdaddr    addr;
    BdaddrConvertBluestackToVm(&addr, &prim->bd_addr);
    
    /* Providing the read was a success and we have a vaid name */
    if (!prim->status && prim->name_part[0])
    {
        uint16    length;
        uint8    i;

        /* Only handle the first segment */
        char*    name = VmGetPointerFromHandle(prim->name_part[0]);

        /* Find the length of the string in the first segment, limiting to     MAX_NAME_LENGTH bytes */
        for(length = 0; length < MAX_NAME_LENGTH; length++)
        {
            if (name[length] == '\0')
                break;
        }
        
        name[length] = '\0';
        
        /*     Free any other segments */
        for(i = 1;i < HCI_LOCAL_NAME_BYTE_PACKET_PTRS;i++)
        {
            if (prim->name_part[i] != NULL)
                free(VmGetPointerFromHandle(prim->name_part[i]));
        }
        
        /* Remote name read, send Client message to notify them of the result */
        remoteNameComplete(state->nameLock, &addr, rnr_success, name, length);
    }
    else
    {
        
        /* Read failed, send Client message to notify them of the result */
        remoteNameComplete(state->nameLock, &addr, 
                           prim->status? rnr_error: rnr_completed, NULL, 0);
    }
    
    /* Reset resource lock */
    state->nameLock = 0;
}

/****************************************************************************
NAME    
    connectionHandleReadLocalName

DESCRIPTION
    This function will initiate a read of the local name of the device

RETURNS
    void
*/
void connectionHandleReadLocalName(connectionInquiryState *state, const CL_INTERNAL_DM_READ_LOCAL_NAME_REQ_T *req)
{
    /* Check the state of the task lock before doing anything */
    if (!state->nameLock)
    {
        /* One request at a time */
        state->nameLock = req->theAppTask;
        
        /* Issue request to read the remote name */
        {
            MAKE_PRIM_C(DM_HCI_READ_LOCAL_NAME_REQ);
            VmSendDmPrim(prim);
        }
    }
    else
    {
        /* Remote name request currently being performed, queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_LOCAL_NAME_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_LOCAL_NAME_REQ, message, &state->nameLock);
    }
}

/****************************************************************************
NAME    
    localNameComplete

DESCRIPTION
    Send a CL_DM_LOCAL_NAME_COMPLETE message to the client

RETURNS
    void
*/
static void localNameComplete(Task theAppTask, hci_status status, char* name, uint16 length)
{
    if (theAppTask)
    {
        MAKE_CL_MESSAGE_WITH_LEN(CL_DM_LOCAL_NAME_COMPLETE, length);
        message->status = status;
        
        message->size_local_name = length;
        
        /* Populate local name fields */
        if (length)
        {
            memmove(message->local_name, name, length);    
            free(name);
        }    
        else
            message->local_name[0] = 0;    
        
        /* Send onto Client */
        MessageSend(theAppTask, CL_DM_LOCAL_NAME_COMPLETE, message);
    }
    else
    {
        if (length)
            free(name);
    }
}

/****************************************************************************
NAME    
    connectionHandleLocalNameComplete

DESCRIPTION
    Local name result

RETURNS
    void
*/
void connectionHandleLocalNameComplete(connectionInquiryState *state, const DM_HCI_READ_LOCAL_NAME_CFM_T* prim)
{
    /* Providing the read was a success and we have a vaid name */
    if (!prim->status && prim->name_part[0])
    {
        uint16    length;
        uint8    i;

        /* Only handle the first segment */
        char*    name = VmGetPointerFromHandle(prim->name_part[0]);

        /* Find the length of the string in the first segment, limiting to     MAX_NAME_LENGTH bytes */
        for(length = 0; length < MAX_NAME_LENGTH; length++)
        {
            if (name[length] == '\0')
                break;
        }
        
        name[length] = '\0';
        
        /*     Free any other segments */
        for(i = 1;i < HCI_LOCAL_NAME_BYTE_PACKET_PTRS;i++)
        {
            if (prim->name_part[i] != NULL)
                free(VmGetPointerFromHandle(prim->name_part[i]));
        }
        
        /* Remote name read, send Client message to notify them of the result */
        localNameComplete(state->nameLock, connectionConvertHciStatus(prim->status), name, length);
    }
    else
    {
        /* Read failed, send Client message to notify them of the result */
        localNameComplete(state->nameLock, connectionConvertHciStatus(prim->status), NULL, 0);
    }
    
    /* Reset resource lock */
    state->nameLock = 0;
}

