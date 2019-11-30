/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_csr_features.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_link_manager.h"
#include "hfp_csr_features.h"
#include "hfp_send_data.h"
#include "hfp_parse.h"

#include <panic.h>
#include <print.h>
#include <stdio.h>
#include <string.h>


/* Handy macros for checking if CSR2CSR features are enabled */
#define agCsrFeatureEnabled(link, feature) ((link->bitfields.ag_csr_features_enabled) && (link->ag_csr_features.feature))
#define hfCsrFeatureEnabled(feature)       (theHfp->csr_features.feature)
#define csrFeatureEnabled(link, feature)   (hfCsrFeatureEnabled(feature) && agCsrFeatureEnabled(link, feature))


/*
    Send AT+CSRSF to the AG indicating our supported features
*/
void hfpCsrFeaturesReq(hfp_link_data* link)
{
    hfp_csr_features f = theHfp->csr_features;
    
    /* Don't send AT+CSRF if none of the CSR2CSR features are enabled on the HF */
    if(f.caller_name || f.raw_text || f.sms || f.batt_level || f.pwr_source || f.codecs || f.codec_bandwidths)
    {
        char cmd[24];   /* Length is sufficient as values bit fielded */
        
        /* Create the AT cmd we're sending */
        sprintf(cmd, "AT+CSRSF=%d,%d,%d,%d,%d,%d,%d\r", f.caller_name, f.raw_text, f.sms, f.batt_level,
                                                        f.pwr_source, f.codecs, f.codec_bandwidths);
        
        /* Send the AT cmd over the air */
        hfpSendAtCmd(link, (uint16)strlen(cmd), cmd, hfpCsrSfPending);
    }
}


/*
    Handle reception of a +CSRF response containing the CODEC and bandwidth options.
*/
void hfpCsrFeaturesHandleBandwidthCodecRes(Task link_ptr, const struct hfpCsrFeaturesHandleBandwidthCodecRes *features)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    
    /* Update AG feature set to what both we and the AG support */
    link->ag_csr_features.caller_name      = features->callername;
    link->ag_csr_features.raw_text         = features->rawtext;
    link->ag_csr_features.sms              = features->sms;
    link->ag_csr_features.batt_level       = features->battlevel;
    link->ag_csr_features.pwr_source       = features->pwrsource;
    link->ag_csr_features.codecs           = features->codecs;
    link->ag_csr_features.codec_bandwidths = features->codecbandwidths;
}


/*
    Handle reception of a +CSRF response containing the CODEC option.
*/
void hfpCsrFeaturesHandleCodecRes(Task link_ptr, const struct hfpCsrFeaturesHandleCodecRes *features)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;

    link->ag_csr_features.caller_name      = features->callername;
    link->ag_csr_features.raw_text         = features->rawtext;
    link->ag_csr_features.sms              = features->sms;
    link->ag_csr_features.batt_level       = features->battlevel;
    link->ag_csr_features.pwr_source       = features->pwrsource;
    link->ag_csr_features.codecs           = features->codecs;
    link->ag_csr_features.codec_bandwidths = hfp_csr_bandwidth_none;
}


/*
    Handle reception of a +CSRF response.
*/
void hfpCsrFeaturesHandleNormalRes(Task link_ptr, const struct hfpCsrFeaturesHandleNormalRes *features)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;

    link->ag_csr_features.caller_name      = features->callername;
    link->ag_csr_features.raw_text         = features->rawtext;
    link->ag_csr_features.sms              = features->sms;
    link->ag_csr_features.batt_level       = features->battlevel;
    link->ag_csr_features.pwr_source       = features->pwrsource;
    link->ag_csr_features.codecs           = hfp_csr_codec_mask_none;;
    link->ag_csr_features.codec_bandwidths = hfp_csr_bandwidth_none;
}


/*
    Have received response to AT+CSRSF
*/
void hfpCsrFeaturesHandleCsrSfAtAck(hfp_link_data* link, hfp_lib_status status)
{
    link->bitfields.ag_csr_features_enabled = (status == hfp_success);
}


/*
    Handle +CSR= sent from AG
*/
void hfpCsrFeaturesHandleModifyIndicatorsInd(Task link_ptr, const struct hfpCsrFeaturesHandleModifyIndicatorsInd *inds)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    
    unsigned num = inds->p.count;
    unsigned cnt;
    struct value_hfpCsrFeaturesHandleModifyIndicatorsInd_p ind;
    
    for (cnt = 0; cnt < num; cnt++)
    {
        /* Get indicator */
        ind = get_hfpCsrFeaturesHandleModifyIndicatorsInd_p(&inds->p, (uint16)cnt);
        
        /* Update AG supported CSR features */
        switch(ind.ind)
        {
            case hfp_csr_caller_name:
                link->ag_csr_features.caller_name = ind.val;
                break;
            case hfp_csr_raw_text:
                link->ag_csr_features.raw_text = ind.val;
                break;
            case hfp_csr_sms:
                link->ag_csr_features.sms = ind.val;
                break;
            case hfp_csr_batt_level:
                link->ag_csr_features.batt_level = ind.val;
                break;
            case hfp_csr_pwr_source:
                link->ag_csr_features.pwr_source = ind.val;
                break;
            case hfp_csr_codecs:
                link->ag_csr_features.codecs = ind.val;
                break;
            case hfp_csr_codec_bandwidths:
                link->ag_csr_features.codec_bandwidths = ind.val;
                break;
        }
    }
}


/*
    Handle +CSR=0 sent from AG
*/
void hfpCsrFeaturesHandleDisableIndicatorsInd(Task link_ptr)
{
    /* Disable all CSR features on this link */
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    link->bitfields.ag_csr_features_enabled = FALSE;
}


/*
    Handle AG requesting a battery report
*/
void hfpCsrFeaturesHandleBatteryLevelRequestInd(Task link_ptr)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    
    if(csrFeatureEnabled(link, batt_level))
    {
        MAKE_HFP_MESSAGE(HFP_CSR_FEATURES_BATTERY_LEVEL_REQUEST_IND);
        message->priority = hfpGetLinkPriority(link);
        MessageSend(theHfp->clientTask, HFP_CSR_FEATURES_BATTERY_LEVEL_REQUEST_IND, message);
    }
}


/*! 
    Send a power status report to the AG
*/
void HfpCsrFeaturesBatteryLevelRequest(hfp_link_priority priority, uint16 batt_level)
{
    MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_BATTERY_LEVEL_REQ);
    message->link = hfpGetLinkFromPriority(priority);
    message->batt_level = batt_level;
    MessageSend(&theHfp->task, HFP_INTERNAL_CSR_BATTERY_LEVEL_REQ, message);
}


/*
    Handle CSR Power Level Request
*/
void hfpCsrFeaturesHandleBatteryLevelReq(const HFP_INTERNAL_CSR_BATTERY_LEVEL_REQ_T *req)
{
    hfp_link_data* link = req->link;
    
    if(csrFeatureEnabled(link, batt_level))
    {
        /* SLC connected, send the command */
        char cmd[20];

        /* Create the AT cmd we're sending */
        sprintf(cmd, "AT+CSRBATT=%d\r", req->batt_level);
        
        /* Send the AT cmd over the air */
        hfpSendAtCmd(req->link, (uint16)strlen(cmd), cmd, hfpCsrBatPending);
    }
}


/*! 
    Send a power source report to the AG
*/
void HfpCsrFeaturesPowerSourceRequest(hfp_link_priority priority, hfp_csr_power_status_report pwr_status)
{
    /* Send an internal message */
    MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_POWER_SOURCE_REQ);
    message->link = hfpGetLinkFromPriority(priority);
    message->pwr_status = pwr_status;
    MessageSend(&theHfp->task, HFP_INTERNAL_CSR_POWER_SOURCE_REQ, message);
}


/*
    Handle CSR Power Source Request
*/
void hfpCsrFeaturesHandlePowerSourceReq(const HFP_INTERNAL_CSR_POWER_SOURCE_REQ_T *req)
{
    if(csrFeatureEnabled(req->link, pwr_source))
    {
        /* SLC connected, send the command */
        char cmd[15];
        
        /* Create the AT cmd we're sending */
        sprintf(cmd, "AT+CSRPWR=%d\r", req->pwr_status);
        
        /* Send the AT cmd over the air */
        hfpSendAtCmd(req->link, (uint16)strlen(cmd), cmd, hfpCsrPwrPending);
    }
}


/*
    Handle Unsolicited +CSRTTS AT response
*/
void hfpCsrFeaturesHandleTextInd(Task link_ptr, const struct hfpCsrFeaturesHandleTextInd *text)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    
    if (csrFeatureEnabled(link, raw_text))
    {
        /* Leave space to NULL terminate string */
        uint16 length = ((text->text.length < HFP_MAX_ARRAY_LEN) ? text->text.length : (HFP_MAX_ARRAY_LEN - 1));
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_FEATURES_TEXT_IND, length + 1);
        message->priority = hfpGetLinkPriority(link);
        message->size_text = length;
        memmove(message->text, text->text.data, length);
        message->text[length] = '\0';
        MessageSend(theHfp->clientTask, HFP_CSR_FEATURES_TEXT_IND, message);
    }
}


/*
    Notification from AG that an SMS has arrived
*/
void hfpCsrFeaturesHandleSmsInd(Task link_ptr, const struct hfpCsrFeaturesHandleSmsInd *sms)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    uint16 size_number  = sms->senderNum.length;
    uint16 size_name    = sms->senderName.length;
    
    /* Make sure strings don't take over the max limit */
    if(size_number >= HFP_MAX_ARRAY_LEN)
        size_number = HFP_MAX_ARRAY_LEN - 1;
        
    if(size_name >= (HFP_MAX_ARRAY_LEN - size_number))
        size_name = (uint16)(HFP_MAX_ARRAY_LEN - size_number - 1);
    
    if (csrFeatureEnabled(link, sms))
    {
        /* Allow space to NULL terminate strings */
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_FEATURES_NEW_SMS_IND, size_number + size_name + 2);
        message->priority = hfpGetLinkPriority(link);
        message->index = sms->smsIndex;
        
        /* Set data offsets + sizes*/
        message->offset_number = 0;
        message->offset_name   = (uint16)(size_number + 1);
        message->size_number   = size_number;
        message->size_name     = size_name;
        
        /* Copy the name into message info */
        memmove(message->sender_info + message->offset_number, sms->senderNum.data,  size_number);
        memmove(message->sender_info + message->offset_name,   sms->senderName.data, size_name);
        
        /* Make sure strings are NULL terminated */
        message->sender_info[message->offset_number + size_number] = '\0';
        message->sender_info[message->offset_name   + size_name]   = '\0';
        
        MessageSend(theHfp->clientTask, HFP_CSR_FEATURES_NEW_SMS_IND, message);
    }
}


/*
    Retreive an SMS from the AG.
*/
void HfpCsrFeaturesGetSmsRequest(hfp_link_priority priority, uint16 index)
{
    MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_GET_SMS_REQ);
    message->link  = hfpGetLinkFromPriority(priority);
    message->index = index;
    MessageSend(&theHfp->task, HFP_INTERNAL_CSR_GET_SMS_REQ, message);
}


/*
    Handle internal Get SMS request.
*/
bool hfpHandleCsrGetSmsReq(const HFP_INTERNAL_CSR_GET_SMS_REQ_T *req)
{
    if (csrFeatureEnabled(req->link, sms))
    {
        /* Create the AT cmd we're sending */
        char cmd[20];
        sprintf(cmd, "AT+CSRGETSMS=%d\r", req->index);
        
        /* Send the AT cmd over the air */
        hfpSendAtCmd(req->link, (uint16)strlen(cmd), cmd, hfpCsrGetSmsPending);
        return TRUE;
    }
    return FALSE;
}


static void hfpCsrFeaturesSendSmsCfm(hfp_link_data* link, hfp_lib_status status, uint16 size_sms, const uint8* sms)
{
    /* Allow space to add NULL to the end of the SMS string */
    MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_FEATURES_GET_SMS_CFM, size_sms + 2);
    message->priority = hfpGetLinkPriority(link);
    message->status = status;
    
    /* Copy the SMS into message info (don't unclude NULL in length) */
    message->size_sms = size_sms;
    memmove(message->sms, sms, message->size_sms);
    message->sms[message->size_sms] = '\0';
    
    MessageSend(theHfp->clientTask, HFP_CSR_FEATURES_GET_SMS_CFM, message);
}


/*
    Handle response to a AT+CSRGETSMS
*/
void hfpCsrFeaturesHandleGetSmsRes(Task link_ptr, const struct hfpCsrFeaturesHandleGetSmsRes *sms)
{
    hfpCsrFeaturesSendSmsCfm((hfp_link_data*)link_ptr, hfp_success, sms->sms.length, sms->sms.data);
}


/*
    Handle Get SMS ack.
*/
void hfpCsrFeaturesHandleGetSmsAtAck(hfp_link_data* link, hfp_lib_status status)
{
    /* If the AG responded with OK, we should already have received a 
       +CSRGETSMS containing the information required, and this will 
       have been sent to the application. */
    if (status != hfp_success)
        hfpCsrFeaturesSendSmsCfm(link, status, 0, NULL);
}
