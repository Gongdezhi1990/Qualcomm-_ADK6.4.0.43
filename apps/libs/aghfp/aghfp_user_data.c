/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"

#include <string.h>


/*******************************************************************************/
void AghfpSendUserSpecificData(AGHFP *aghfp, uint16 size_data, uint8 *data)
{
    if (size_data)
    {
        /* Only send the data if the client has bothered to give us any! */
        MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_USER_DATA_REQ, size_data);

        if (message)
        {		
    	    /* Copy number */
            message->size_data = size_data;
	        memmove(message->data, data, size_data);	    
            MessageSend(&aghfp->task, AGHFP_INTERNAL_USER_DATA_REQ, message);
        }					
        else
        {
            aghfpSendCommonCfmMessageToApp(AGHFP_USER_DATA_CFM, aghfp, aghfp_fail);
        }	
    }
    else
    {
        aghfpSendCommonCfmMessageToApp(AGHFP_USER_DATA_CFM, aghfp, aghfp_fail);
    }	
}

