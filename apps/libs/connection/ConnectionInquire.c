/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionInquire.c        

DESCRIPTION

NOTES
    An inquiry can only be initiated by one task at a time. If an inquiry
    request is received while the connection lib is already performing an 
    inquiry a CL_DM_INQUIRE_RESULT message is returned with status set to
    busy.

    Setting the Class of Device field to zero will turn off class of device
    filtering of inquiry results and all devices found will be returned.
*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <vm.h>
#include <string.h>

#ifndef CL_EXCLUDE_INQUIRY

/*****************************************************************************/
void ConnectionInquire(Task theAppTask, uint32 inquiry_lap, uint8 max_responses, uint8 timeout, uint32 class_of_device)
{
    /* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB
    if ((inquiry_lap <0x9E8B00) || (inquiry_lap > 0x9E8B3F))
    {
        CL_DEBUG(("Out of range inquiry_lap 0x%lx\n", inquiry_lap));
    }    

    if ((timeout < HCI_INQUIRY_LENGTH_MIN) || (timeout > HCI_INQUIRY_LENGTH_MAX))
    {
        CL_DEBUG(("Out of range timeout 0x%x\n", timeout));
    }
#endif

    {
        /* Post an internal message so we can check the state machine */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_INQUIRY_REQ);
        message->theAppTask = theAppTask;
        message->inquiry_lap = inquiry_lap;
        message->max_responses = max_responses;
        message->timeout = timeout;
        message->class_of_device = class_of_device;
        message->min_period = 0;
        message->max_period = 0;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_INQUIRY_REQ, message);
    }
}

/*****************************************************************************/
void ConnectionInquirePeriodic(Task theAppTask, uint16 min_period, uint16 max_period, uint32 inquiry_lap, uint8 max_responses, uint8 timeout, uint32 class_of_device)
{
    /* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB
    if ((inquiry_lap <0x9E8B00) || (inquiry_lap > 0x9E8B3F))
    {
        CL_DEBUG(("Out of range inquiry_lap 0x%lx\n", inquiry_lap));
    }    

    if ((timeout < HCI_INQUIRY_LENGTH_MIN) || (timeout > HCI_INQUIRY_LENGTH_MAX))
    {
        CL_DEBUG(("Out of range timeout 0x%x\n", timeout));
    }
    
    if ((max_period < HCI_INQUIRY_MAX_PERIOD_MIN) || (max_period > HCI_INQUIRY_MAX_PERIOD_MAX))
    {
        CL_DEBUG(("Out of range max_period 0x%x\n", max_period));
    }
    
    if ((min_period < HCI_INQUIRY_MIN_PERIOD_MIN) || (min_period > HCI_INQUIRY_MIN_PERIOD_MAX))
    {
        CL_DEBUG(("Out of range max_period 0x%x\n", min_period));
    }
#endif

    {
        /* Post an internal message so we can check the state machine */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_INQUIRY_REQ);
        message->theAppTask = theAppTask;
        message->inquiry_lap = inquiry_lap;
        message->max_responses = max_responses;
        message->timeout = timeout;
        message->class_of_device = class_of_device;
        message->min_period = min_period;
        message->max_period = max_period;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_INQUIRY_REQ, message);
    }
}

#endif

