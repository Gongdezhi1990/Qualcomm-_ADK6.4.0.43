/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_slc.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include "hfp_service_manager.h"
#include "hfp_slc_handler.h"
#include "hfp_sdp.h"
#include "hfp_common.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    HfpSlcConnectRequest

DESCRIPTION
    This function is used to initate the creation of a Service Level
    Connection (SLC). 
    
    The bd_addr specifies the address of the remote device (the Audio Gateway 
    or AG) to which the connection will be created.

MESSAGE RETURNED
    HFP_SLC_CONNECT_CFM

RETURNS
    void
*/
void HfpSlcConnectRequest(const bdaddr *bd_addr, hfp_profile profiles, hfp_profile first_attempt)
{
    /* Get an idle link */
    hfp_link_data *link = hfpGetLinkFromBdaddr(bd_addr);
    
#ifdef HFP_DEBUG_LIB
    if (!bd_addr)
        HFP_DEBUG(("Null address ptr passed in.\n"));
#endif
    
    /* If this device isn't already connected */
    if(!link)
    {
        link = hfpGetIdleLink();

        if(link)
        {
            /* Get a free service of the right type to connect from */
            hfp_service_data* service = hfpGetServiceFromProfile(first_attempt);
            
            if(!service)
            {
                /* first_attempt is not supported, try anything... */
                profiles &= ~first_attempt;
                service = hfpGetServiceFromProfile(profiles);
            }
            
            if(service)
            {
                /* Perform a service attrribute search to get the rfcomm channel 
                   of the remote service */
                link->bitfields.ag_profiles_to_try = profiles;
                hfpGetProfileServerChannel(link, service, bd_addr);
                return;
            }
            
        }
    }
    
    /* We've used up our resources, tell the app */
    hfpSendSlcConnectCfmToApp(NULL, bd_addr, hfp_connect_failed_busy);
}


/****************************************************************************
NAME    
    HfpSlcDisconnectRequest

DESCRIPTION
    This function initiates the disconnection of an SLC for a particular 
    profile instance (theHfp).

MESSAGE RETURNED
    HFP_SLC_DISCONNECT_IND

RETURNS
    void
*/
void HfpSlcDisconnectRequest(hfp_link_priority priority)
{
    /* Send an internal message to kick off the disconnect */
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_SLC_DISCONNECT_REQ, priority);
}


