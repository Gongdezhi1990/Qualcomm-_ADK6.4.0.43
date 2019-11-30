/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_caps.c        

DESCRIPTION
This file defines the APIs for GetCapabilities feature    

NOTES

*/


/****************************************************************************
    Header files
*/
#include <source.h>
#include "avrcp_caps_handler.h"
#include "avrcp_metadata_transfer.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/****************************************************************************
*NAME    
*    AvrcpGetCapsRequest
*
*DESCRIPTION
*  API function to send AvrcpGetCapabilities Request from CT.
*    
*PARAMETERS
*   avrcp            - Task
*   caps             - Requested capability. capability value MUST be either
*                      avrcp_capability_company_id or 
*                      avrcp_capability_event_supported.
*
*RETURN
****************************************************************************/
void AvrcpGetCapsRequest(AVRCP *avrcp, avrcp_capability_id caps)
{
    uint8 caps_params[] = {0};
    avrcp_status_code status;

#ifdef AVRCP_DEBUG_LIB    
    if ((caps != avrcp_capability_company_id) && 
        (caps != avrcp_capability_event_supported))
        AVRCP_DEBUG(("Invalid capability type requested 0x%x\n", caps));
#endif

    caps_params[0] = caps;

    status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_CAPS_PDU_ID, 
                                        avrcp_get_caps, 
                                        sizeof(caps_params),
                                        caps_params, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendGetCapsCfm(avrcp, status, 0, 0, 0, 0, 0);
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
*NAME    
*   AvrcpGetCapsResponse 
*
*DESCRIPTION
*  API function to respond to Get Capabilities request.
*  TG application shall
*  call function on receiving AVRCP_GET_CAPS_IND.
*    
*PARAMETERS
*   avrcp                   - Task
*   avrcp_response_code     - response.  Expected responses are 
*                             avctp_response_stable or avctp_response_rejected
*                             or avrcp_response_rejected_invalid_param.
*   caps                    - Capability ID requested.
*   caps_list               - List for capability values.
*
*RETURN
*******************************************************************************/
void AvrcpGetCapsResponse(AVRCP                 *avrcp, 
                          avrcp_response_type   response, 
                          avrcp_capability_id   caps, 
                          uint16                size_caps_list, 
                          Source                caps_list)
{
#ifdef AVRCP_DEBUG_LIB    
    if ((caps != avrcp_capability_company_id) && 
        (caps != avrcp_capability_event_supported))
        AVRCP_DEBUG(("Invalid capability type requested 0x%x\n", caps));
#endif

    /* Only allow a response to be sent if the corresponding command arrived. */
    if (avrcp->bitfields.block_received_data == avrcp_get_caps)
    {
        avrcpSendGetCapsResponse(   avrcp, 
                                    response, 
                                    caps, 
                                    size_caps_list, 
                                    caps_list);
    }
    else
    {
        SourceEmpty( caps_list );
        AVRCP_INFO(("AvrcpGetCapabilitiesResponse: " 
                    "CT not waiting for response\n"));
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/
