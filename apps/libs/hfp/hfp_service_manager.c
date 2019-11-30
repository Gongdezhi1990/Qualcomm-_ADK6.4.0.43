/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_service_manager.c

DESCRIPTION
    The service manager provides commonly used functionality for accessing 
    services in hfp_task_data.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_service_manager.h"
#include "hfp_link_manager.h"


/****************************************************************************
NAME
    hfpServiceInitReturnNext

DESCRIPTION
    Initialise a single service record with profile and channel number. This
    function returns a pointer service + 1
*/
static hfp_service_data* hfpServiceInitReturnNext(hfp_service_data* service, hfp_profile profile, uint8 channel)
{
    service->bitfields.rfc_server_channel = channel;
    service->bitfields.profile = profile;
    return service + 1;
}

/****************************************************************************
NAME
    hfpServicesInit

DESCRIPTION
    Set up theHfp->services structure with profiles from app configuration
    and corresponding default RFCOMM channels.
*/
void hfpServicesInit(hfp_profile profile, bool multipoint)
{
    hfp_service_data* service = theHfp->services;
    
    /* Create default service entries for HFP (these are the RFCOMM 
       channels we attempt to register for the services) */
    if(supportedProfileIsHfp(profile))
    {
        hfp_profile handsfree = profile & hfp_handsfree_all;
        
        /* Create a service entry for HFP */
        service = hfpServiceInitReturnNext(service, handsfree, HFP_DEFAULT_CHANNEL);
        
        /* Create a second HFP service entry if multipoint is enabled */
        if(multipoint)
            service = hfpServiceInitReturnNext(service, handsfree, HFP_DEFAULT_CHANNEL_2);
    }
    
    /* Create default service entries for HSP */
    if(supportedProfileIsHsp(profile))
    {
        hfp_profile headset = profile & hfp_headset_all;
        
        /* Create a service entry for HSP */
        service = hfpServiceInitReturnNext(service, headset, HSP_DEFAULT_CHANNEL);
        
        /* Create a second HSP service entry if multipoint is enabled */
        if(multipoint)
            service = hfpServiceInitReturnNext(service, headset, HSP_DEFAULT_CHANNEL_2);
    }
}

/****************************************************************************
NAME    
    hfpGetServiceFromChannel

DESCRIPTION
    Get service data corresponding to a given RFC server channel

RETURNS
    Pointer to the corresponding hfp_service_data if successful. 
    Otherwise NULL.
*/
hfp_service_data* hfpGetServiceFromChannel(uint8 channel)
{
    hfp_service_data*  service;
    
    for_all_services(service)
    {
        if(service->bitfields.rfc_server_channel == channel)
            return service;
    }

    return NULL;
}


/****************************************************************************
NAME    
    hfpGetServiceFromHandle

DESCRIPTION
    Get service data corresponding to a given service record handle

RETURNS
    Pointer to the corresponding hfp_service_data if successful. 
    Otherwise NULL.
*/
hfp_service_data* hfpGetServiceFromHandle(uint32 service_handle)
{
    hfp_service_data*  service;
    
    for_all_services(service)
    {
        if(service->sdp_record_handle == service_handle)
            return service;
    }

    return NULL;
}


/****************************************************************************
NAME    
    hfpGetServiceFromProfile

DESCRIPTION
    Get service data for any service that supports given profile

RETURNS
    Pointer to the corresponding hfp_service_data if successful. 
    Otherwise NULL.
*/
hfp_service_data* hfpGetServiceFromProfile(hfp_profile profile)
{
    hfp_service_data*  service;
    
    for_all_services(service)
    {
        if(service->bitfields.profile & profile)
            return service;
    }

    return NULL;
}


/****************************************************************************
NAME    
    hfpGetDisconnectedServiceFromProfile

DESCRIPTION
    Get service data for any service that supports given profile and is not 
    in use by a link in hfp_slc_complete state

RETURNS
    Pointer to the corresponding hfp_service_data if successful. 
    Otherwise NULL.
*/
hfp_service_data* hfpGetDisconnectedServiceFromProfile(hfp_profile profile)
{
    hfp_service_data* service;
    
    for_all_services(service)
    {
        /* If service matches the profile */
        if(service->bitfields.profile & profile)
        {
            /* If service is not connected then return it */
            hfp_link_data* link = hfpGetLinkFromService(service);
            if(link_is_disconnected(link))
            {
                return service;
            }
        }
    }
    return NULL;
}


/****************************************************************************
NAME    
    hfpGetVisibleServiceFromProfile

DESCRIPTION
    Get service data for any service that supports given profile and has a 
    visible SDP record

RETURNS
    Pointer to the corresponding hfp_service_data if successful. 
    Otherwise NULL.
*/
hfp_service_data* hfpGetVisibleServiceFromProfile(hfp_profile profile)
{
    hfp_service_data*  service;
    
    for_all_services(service)
    {
        if((service->bitfields.profile & profile) && service->sdp_record_handle)
            return service;
    }
    return NULL;
}
