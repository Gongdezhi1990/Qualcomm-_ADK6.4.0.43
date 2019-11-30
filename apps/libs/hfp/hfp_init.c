/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    init.c

DESCRIPTION
    This file contains the initialisation code for the Hfp profile library.

NOTES

*/
/*lint -e655 */

/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_service_manager.h"
#include "hfp_link_manager.h"
#include "hfp_common.h"
#include "hfp_rfc.h"
#include "hfp_sdp.h"
#include "hfp_init.h"
#include "hfp_profile_handler.h"
#include "hfp_wbs_handler.h"

#include <panic.h>
#include <print.h>
#include <string.h>

/* The main HFP data struct */
hfp_task_data* theHfp;

/****************************************************************************
NAME
    hfpInitCfmToApp

DESCRIPTION
    Send an init cfm message to the application 
*/
static void hfpInitCfmToApp(hfp_init_status status)
{
    MAKE_HFP_MESSAGE(HFP_INIT_CFM);
    message->status = status;
    MessageSend(theHfp->clientTask, HFP_INIT_CFM, message);

    /* If the initialisation failed, free the allocated task */
    if (status > hfp_init_reinit_fail)
    {
        if (theHfp->links)
            free(theHfp->links);
        if (theHfp->services)
            free(theHfp->services);
        free(theHfp);
        theHfp = NULL;
    }
}


/****************************************************************************
NAME
    HfpInit

DESCRIPTION
    

MESSAGE RETURNED
    HFP_INIT_CFM

RETURNS
    void
*/
void HfpInit(Task theAppTask, const hfp_init_params *config, const char* extra_indicators)
{
    if(theHfp)
    {
        hfpInitCfmToApp(hfp_init_reinit_fail);
        return;
    }
    
    /* Check the app has passed in a valid pointer. */
    if (!config)
    {
        HFP_ASSERT_FAIL(("Config parameters not passed in\n"));
    }
    else
    {
        hfp_task_data*    lHfp;
        hfp_link_data*    link;
        hfp_profile       profile  = config->supported_profile;

        /* Calculate number of links and services */
        unsigned num_links    = (config->multipoint ? 2 : 1);
        unsigned num_services = 0;
        
        if(supportedProfileIsHfp(profile))
            num_services += num_links;
        
        if(supportedProfileIsHsp(profile))
            num_services += num_links;
        
        /* Allocate and zero our hfp_task_data */
        lHfp = theHfp = PanicNull(calloc(1,sizeof(*theHfp)));

        lHfp->links = PanicNull(calloc(num_links,sizeof(hfp_link_data)));
        lHfp->num_links = (uint8)num_links;

        if (num_services)
        {
            lHfp->services = PanicNull(calloc(num_services,sizeof(hfp_service_data)));
            lHfp->num_services = (uint8)num_services;
        }

        PRINT(("HFP Task Data\n"));
        PRINT(("%d Words for main task\n",           sizeof(hfp_task_data)));
        PRINT(("%d Words for links\n",               sizeof(hfp_link_data) * num_links));
        PRINT(("%d Words for services\n",            sizeof(hfp_service_data) * num_services));
    
        /* Set the handler function */
        lHfp->task.handler = hfpProfileHandler;
        
        /* Mask out unsupported features. */
        lHfp->hf_supported_features = (config->supported_features & ~HFP_ENHANCED_CALL_CONTROL);
        
        if(!supportedProfileIsHfp107(profile))
        {
            lHfp->hf_supported_features &= ~HFP_CODEC_NEGOTIATION;
            lHfp->hf_supported_features &= ~HFP_ESCO_S4_SUPPORTED;
            lHfp->hf_supported_features &= ~HFP_HF_INDICATORS;
            lHfp->hf_indicators = hfp_indicator_mask_none;
        }

        if(supportedProfileIsHfp107(profile))
        {
            lHfp->hf_supported_features |= HFP_ESCO_S4_SUPPORTED;
        }

        if(supportedProfileIsHfp107(profile) && (config->hf_indicators != hfp_indicator_mask_none))
        {
            lHfp->hf_indicators = config->hf_indicators;
        }
        else
        {
            /* Remove HF_INDICATORS feature if no HF Indicators are selected */
            lHfp->hf_supported_features &= ~HFP_HF_INDICATORS;
            lHfp->hf_indicators = hfp_indicator_mask_none;
        }
        
        /* Codec negotiation is supported */
        if(hfFeatureEnabled(HFP_CODEC_NEGOTIATION))
            hfpWbsEnable(config->supported_wbs_codecs);

        /* Qualcomm Codec Extension - supported Codec Mode IDs */
        lHfp->hf_codec_modes = config->hf_codec_modes & CODEC_MODE_ID_MASK;
        
        /* Set the number of link loss reconnect attempts */
        lHfp->bitfields.link_loss_time     = config->link_loss_time;
        lHfp->bitfields.link_loss_interval = config->link_loss_interval;

        /* Set up other config options */
        lHfp->extra_indicators = extra_indicators;
        lHfp->optional_indicators = config->optional_indicators;
        lHfp->bitfields.disable_nrec = config->disable_nrec;
        lHfp->bitfields.extended_errors = config->extended_errors;
        lHfp->csr_features = config->csr_features;        

        /* Store the app task so we know where to return responses */
        lHfp->clientTask = theAppTask;

        if(config->supported_profile == hfp_no_profile)
        {
            hfpInitCfmToApp(hfp_init_success);
            return;
        }

        /* Connection related state updated in separate function */
        for_all_links(link)
        {
            /*hfpLinkReset(link, FALSE); - link already memset to 0 above */
            link->ag_supported_features = (AG_THREE_WAY_CALLING | AG_IN_BAND_RING);

            /*  Linkloss should be managd by the HFP lib by default  */
            link->bitfields.manage_linkloss = TRUE;

            /* hfp attempt settings used when link is secure  */
            link->bitfields.hfp_attempt_settings = hfp_audio_setting_none;

            /* Not determined until Qualcomm Codec Selection after
             * SLC Connection/Audio connection.
             */
            link->qce_codec_mode_id = CODEC_MODE_ID_UNSUPPORTED;
        }
    
        /* Ensure only one HFP version is specified (1.7 takes priority) */
        if(supportedProfileIsHfp107(profile))
        {
            profile &= ~hfp_handsfree_profile;
        }
		
        /* Set up services and begin registration */
        hfpServicesInit(profile, config->multipoint);
        hfpServiceChannelRegister(theHfp->services);
        
        /* We want sync connect notifications */
        ConnectionSyncRegister(&theHfp->task);
    }
}


/****************************************************************************
NAME
    hfpInitRfcommRegisterCfm

DESCRIPTION
    Rfcomm channel has been allocated.

RETURNS
    void
*/
void hfpInitRfcommRegisterCfm(const CL_RFCOMM_REGISTER_CFM_T *cfm)
{
    if(cfm->status == success)
    {
        static unsigned index = 0;
        hfp_service_data* service = &theHfp->services[index];
        
        /* We may not get back the channel we requested, update
           the service entry before registering an SDP record */
        service->bitfields.rfc_server_channel = cfm->server_channel;
        hfpRegisterServiceRecord(service);
        
        /* Register RFCOMM channel for the next service */
        if(service < HFP_SERVICE_LAST)
        {
            service = &theHfp->services[++index];
            hfpServiceChannelRegister(service);
        }
        
        return;
    }
    
    /* Request failed or no free service */
    hfpInitCfmToApp(hfp_init_rfc_chan_fail);
}


/****************************************************************************
NAME
    hfpInitSdpRegisterComplete

DESCRIPTION
    SDP registration has completed

RETURNS
    void
*/
void hfpInitSdpRegisterComplete(hfp_lib_status status)
{
    if(status == hfp_success)
    {
        /* Get the service we requested to register */
        hfp_service_data* service = hfpGetServiceFromChannel(theHfp->busy_channel);
        if(service)
        {
            if(service == HFP_SERVICE_LAST)
            {
                /* Last service successfully registered, done. */
                hfpInitCfmToApp(hfp_init_success);
                theHfp->bitfields.initialised = TRUE;
            }
            /* Make sure we clear the registering channel */
            theHfp->busy_channel = 0;
            return;
        }
    }
    /* Either request failed or couldn't find the right service */
    hfpInitCfmToApp(hfp_init_sdp_reg_fail);
}


/****************************************************************************
NAME
    hfpInitSdpRegisterCfm

DESCRIPTION
    SDP register request from Connection library has returned

RETURNS
    void
*/
void hfpInitSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm)
{
    hfp_lib_status status = (cfm->status == success) ? hfp_success : hfp_fail;
    if(status == hfp_success)
    {
        /* Get the service we requested to register */
        hfp_service_data* service = hfpGetServiceFromChannel(theHfp->busy_channel);
        /* Store the service handle */
        service->sdp_record_handle = cfm->service_handle;
    }
    hfpInitSdpRegisterComplete(status);
}
