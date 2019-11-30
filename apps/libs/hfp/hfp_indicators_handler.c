/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_indicators_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_common.h"
#include "hfp_indicators_handler.h"
#include "hfp_call_handler.h"
#include "hfp_send_data.h"
#include "hfp_link_manager.h"

#include <ctype.h>
#include <panic.h>
#include <string.h>
#include <util.h>

static __inline__ ptr matchChar(ptr p, ptr e, char ch)
{
    return p && p != e && ((int) *p) == ((int) ch) ? p+1 : 0;
}

/* Pull out indicator init state from struct passed inf rom parser */
static uint16 getIndicatorValue(const struct hfpHandleIndicatorStatus *s, uint16 index)
{
    if(1 <= index && index <= s->d.count)
    {
        struct value_hfpHandleIndicatorStatus_d t = get_hfpHandleIndicatorStatus_d(&s->d, index-1);
        return t.n;
    }
    return 0;
}

static void sendIndicatorServiceToApp(const hfp_link_data* link, uint16 service)
{
    /* Send a message to the app */
    MAKE_HFP_MESSAGE(HFP_SERVICE_IND);
    message->priority = hfpGetLinkPriority(link);
    message->service = service;
    MessageSend(theHfp->clientTask, HFP_SERVICE_IND, message);
}

static void sendIndicatorSignalToApp(const hfp_link_data* link, uint16 signal)
{
    /* Send a message to the app */
    MAKE_HFP_MESSAGE(HFP_SIGNAL_IND);
    message->priority = hfpGetLinkPriority(link);
    message->signal = signal;
    MessageSend(theHfp->clientTask, HFP_SIGNAL_IND, message);
}

static void sendIndicatorRoamToApp(const hfp_link_data* link, uint16 roam)
{
    /* Send a message to the app */
    MAKE_HFP_MESSAGE(HFP_ROAM_IND);
    message->priority = hfpGetLinkPriority(link);
    message->roam = roam;
    MessageSend(theHfp->clientTask, HFP_ROAM_IND, message);
}

static void sendIndicatorBattChgToApp(const hfp_link_data* link, uint16 batt_chg)
{
    /* Send a message to the app */
    MAKE_HFP_MESSAGE(HFP_BATTCHG_IND);
    message->priority = hfpGetLinkPriority(link);
    message->battchg = batt_chg;
    MessageSend(theHfp->clientTask, HFP_BATTCHG_IND, message);
}

static void sendUnhandledIndicatorToApp(const hfp_link_data* link, uint16 index, uint16 value)
{
    /*
        We have received an indicator not defined by the HFP spec. Pass it 
        up to the application as it might want to make use of these extra
        indicators that some AGs send out.
    */
    MAKE_HFP_MESSAGE(HFP_EXTRA_INDICATOR_UPDATE_IND);
    message->priority = hfpGetLinkPriority(link);
    message->index = index;
    message->value = value;
    MessageSend(theHfp->clientTask, HFP_EXTRA_INDICATOR_UPDATE_IND, message);
}

/* Record the index of an indicator present in +CIND: and requested by the application */
static void hfpStoreUnhandledIndicator(hfp_link_data* link, uint16 cind_index)
{
    /* Local copy of ptr and count*/
    uint16** extra_indicator_idxs = &link->ag_supported_indicators.extra_indicator_idxs;
    uint16* num_extra_indicator_idxs = &link->ag_supported_indicators.num_extra_indicator_idxs;
    uint16* temp;
    
    /* Make space to store indicator and store it */
    temp = realloc(*extra_indicator_idxs, ((*num_extra_indicator_idxs) + 1) * sizeof(uint16)); /** @todo These may really be uint8's */
    if(temp)
    {
        *extra_indicator_idxs = temp;
        (*extra_indicator_idxs)[*num_extra_indicator_idxs] = cind_index;
        (*num_extra_indicator_idxs)++;
    }
    else
    {
        HFP_DEBUG(("Extra Indicators realloc failed\n"));
    }
}

/* Send the indicator list to the app so it can parse it for any non-HFP indicators it is interested in */
static void hfpSendUnhandledIndicatorListToApp(hfp_link_data* link, uint16 register_index, uint16 cind_index, uint16 min_range, uint16 max_range)
{
    MAKE_HFP_MESSAGE(HFP_EXTRA_INDICATOR_INDEX_IND);
    message->priority = hfpGetLinkPriority(link);
    message->indicator_register_index = register_index;
    message->indicator_index = cind_index;
    message->min_range = min_range;
    message->max_range = max_range;
    MessageSend(theHfp->clientTask, HFP_EXTRA_INDICATOR_INDEX_IND, message);
    
    /* Store the extra indicator index internally */
    hfpStoreUnhandledIndicator(link, cind_index);
}


/****************************************************************************
NAME    
    hfpHandleNull

DESCRIPTION
    Handle in here the AT cmds we want to ignore

RETURNS
    void
*/
void hfpHandleNull(Task link_ptr, const struct hfpHandleNull *ignored)
{
    UNUSED(link_ptr);
    UNUSED(ignored);
}


/****************************************************************************
NAME    
    hfpHandleExtraIndicatorsList

DESCRIPTION
    Helper function to check if an indicator is in the extra indicators
    string the application configured at init.

RETURNS
    void
*/
static void hfpHandleExtraIndicatorsList(hfp_link_data* link, ptr indicator, ptr indicators_end, uint16 index, uint16 length)
{
    size_t indicator_length = 0;
    unsigned i = 0;
    uint16 register_index = 0;
    ptr temp;

    /* Get the end ptr for the string the app passed in. */
    ptr end = (ptr)(theHfp->extra_indicators + strlen(theHfp->extra_indicators));

    /* Find the length of the indicator in the CIND string */
    indicator_length = (ptr)UtilFind(0xFFFF, '"', (const uint16*)indicator, 0, 1, length) - indicator;

    /* 
        For each indicator string in the config string the app passed in see 
        if the current indicator matches that string 
    */
    while ((ptr)theHfp->extra_indicators+i < end)
    {
        uint16 cind_index = 0;
        unsigned min = 0;
        unsigned max = 0;

        /* Match the indicator string against the app config string */
        if (!UtilCompare((const uint16 *) indicator, (const uint16 *) theHfp->extra_indicators+i, indicator_length))/**@todo: Really uint8*/
        {
            /* We have a match, so set the cind_index. */
            cind_index = index;

            /* Get the min and max range */
            temp = hfpFindAndSkipMatchingCharInString(indicator, indicators_end, '(');
            temp = hfpSkipSpace(temp, indicators_end);
            while (temp != indicators_end && isdigit((int) *temp)) min = min * 10 + (*temp++ - '0');
            temp = hfpSkipSpace(temp, indicators_end);
            while (temp != indicators_end && (*temp==',' || *temp=='-')) temp++;
            temp = hfpSkipSpace(temp, indicators_end);
            while (temp != indicators_end && isdigit((int) *temp)) max = max * 10 + (*temp++ - '0');
        }

        /* Send the update to the app - if any */
        if (cind_index)
            hfpSendUnhandledIndicatorListToApp(link, register_index, cind_index, (uint16)min, (uint16)max);

        /* Increment i to the end of the current string, look for the \r separator */
        while ((ptr)theHfp->extra_indicators+i != end && *(theHfp->extra_indicators+i) != '\r')
            i++;

        /* Skip past the \r */
        i++;

        /* Increment the index into the configuration array */
        register_index++;
    }
}


/****************************************************************************
NAME    
    hfpHandleIndicatorList

DESCRIPTION
    Generic indicator list handler, if we don't get a dictionary look up
    match we'll end up in here so we need to parse the string manually as it
    is too complicated for the parser.

AT INDICATION
    +CIND

RETURNS
    void
*/
void hfpHandleIndicatorList(Task link_ptr, const struct hfpHandleIndicatorList *ind)
{
    hfp_link_data*  link           = (hfp_link_data*)link_ptr;
    hfp_indicators  ind_indexes    = { 0, 0, 0, 0, 0, 0, 0 };
    uint16          index          = 0;
    ptr             indicator      = ind->str.data;
    uint16          length         = ind->str.length;
    ptr             indicators_end = indicator + length;
    ptr             next_indicator;

    /* Skip any leading whitespace */
    indicator = hfpSkipSpace(indicator, indicators_end);
    
    /* Make sure we reset the count of extra indicators */
    link->ag_supported_indicators.num_extra_indicator_idxs = 0;
    
    /* While we can find another '(' ... */
    while((next_indicator = matchChar(indicator, indicators_end, '(')) != 0)
    {
        /* Increment index for this indicator */
        index++;
        /* Set indicator to the start of the indicator name */
        indicator = matchChar(hfpSkipSpace(next_indicator, indicators_end), indicators_end, '"');

        /* 
            Make sure the string matches but also make sure that 
            only that string matches e.g. for "call" we don't 
            want "call_tmp" to match. Store the indicator index.
        */
        if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "service", 7))
        {
            ind_indexes.service = index;
        }
        else if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "callsetup", 9))
        {
            if(ind_indexes.call_setup)
                ind_indexes.extra_call_setup = index;
            else
                ind_indexes.call_setup = index;
        }
        else if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "call_setup", 10))
        {
            if(ind_indexes.call_setup)
                ind_indexes.extra_call_setup = index;
            else
                ind_indexes.call_setup = index;
        }
        else if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "call\"", 5))
        {
            ind_indexes.call = index;
        }
        else if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "signal", 6))
        {
            ind_indexes.signal_strength = index;
        }
        else if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "roam", 4))
        {
            ind_indexes.roaming_status = index;
        }
        else if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "battchg", 7))
        {
            ind_indexes.battery_charge = index;
        }
        else if (!UtilCompare((const uint16 *) indicator, (const uint16 *) "callheld", 8))
        {
            ind_indexes.call_hold_status = index;
        }
        
        /* This is to work out the extra indicators we must tell the app about */
        if (theHfp->extra_indicators)
            hfpHandleExtraIndicatorsList(link, indicator, indicators_end, index, length);

        /* skip , ( ... ) ) */
        indicator = hfpFindAndSkipMatchingCharInString(indicator, indicators_end, ')');
        indicator = hfpFindAndSkipMatchingCharInString(indicator, indicators_end, ')');

        /* Skip separating comma if present. */
        next_indicator = hfpSkipSpace(matchChar(indicator, indicators_end, ','), indicators_end);
        
        if(next_indicator) indicator = next_indicator;
    }
    
    /* Store the total number of indicators listed */
    link->ag_supported_indicators.num_indicators = index;
    
    /* To work with old AGs don't check callsetup indicator. */
    if (ind_indexes.service && ind_indexes.call)
    {
        /* Update local indicator index values */
        link->ag_supported_indicators.indicator_idxs = ind_indexes;
    }
}


/****************************************************************************
NAME    
    hfpHandleIndicatorStatus

DESCRIPTION
    Generic handler for initial indicator status update. Called when we don't 
    have a dictionary match.

AT INDICATION
    +CIND

RETURNS
    void
*/
void hfpHandleIndicatorStatus(Task link_ptr, const struct hfpHandleIndicatorStatus *status)
{ 
    hfp_link_data * link            = (hfp_link_data*)link_ptr;
    hfp_indicators* hfp_ind_indexes = &link->ag_supported_indicators.indicator_idxs;
    hfp_indicators  hfp_ind_values;
    
    /* Extract values for the indicators */
    hfp_ind_values.service          = getIndicatorValue(status, hfp_ind_indexes->service);
    hfp_ind_values.call             = getIndicatorValue(status, hfp_ind_indexes->call);
    hfp_ind_values.call_setup       = getIndicatorValue(status, hfp_ind_indexes->call_setup);
    hfp_ind_values.extra_call_setup = getIndicatorValue(status, hfp_ind_indexes->extra_call_setup);
    hfp_ind_values.signal_strength  = getIndicatorValue(status, hfp_ind_indexes->signal_strength);
    hfp_ind_values.roaming_status   = getIndicatorValue(status, hfp_ind_indexes->roaming_status);
    hfp_ind_values.battery_charge   = getIndicatorValue(status, hfp_ind_indexes->battery_charge);
    hfp_ind_values.call_hold_status = getIndicatorValue(status, hfp_ind_indexes->call_hold_status);

    /* Notify the application */
    sendIndicatorServiceToApp(link, hfp_ind_values.service);
    hfpHandleCallIndication(link, (hfp_call)hfp_ind_values.call);

    /* Send messages only if AG supports the indicator i.e. index non zero */
    if (hfp_ind_indexes->call_setup)
        hfpHandleCallSetupIndication(link, (hfp_call_setup)hfp_ind_values.call_setup);

    if (hfp_ind_indexes->signal_strength)
        sendIndicatorSignalToApp(link, hfp_ind_values.signal_strength);
    
    if (hfp_ind_indexes->roaming_status)
        sendIndicatorRoamToApp(link, hfp_ind_values.roaming_status);
    
    if (hfp_ind_indexes->battery_charge)
        sendIndicatorBattChgToApp(link, hfp_ind_values.battery_charge);

    if (hfp_ind_indexes->call_hold_status)
        hfpHandleCallHeldIndication(link, (hfp_call_hold)hfp_ind_values.call_hold_status);
}


/****************************************************************************
NAME    
    hfpHandleIndicatorStatusUpdate

DESCRIPTION
    Indicator status change message received from the AG.

AT INDICATION
    +CIEV

RETURNS
    void
*/
void hfpHandleIndicatorStatusUpdate(Task link_ptr, const struct hfpHandleIndicatorStatusUpdate *ind)
{
    hfp_link_data* link        = (hfp_link_data*)link_ptr;
    hfp_indicators ind_indexes = link->ag_supported_indicators.indicator_idxs;
    
    /* Check which indicator we have received and send indication to the app */
    if (ind->index == ind_indexes.service)
        sendIndicatorServiceToApp(link, ind->value);
        
    else if (ind->index == ind_indexes.call)
        hfpHandleCallIndication(link, (hfp_call)ind->value);

    else if (ind->index == ind_indexes.call_setup || ind->index == ind_indexes.extra_call_setup)
        hfpHandleCallSetupIndication(link, (hfp_call_setup)ind->value);

    else if (ind->index == ind_indexes.signal_strength)
        sendIndicatorSignalToApp(link, ind->value);
        
    else if (ind->index == ind_indexes.roaming_status)
        sendIndicatorRoamToApp(link, ind->value);
        
    else if (ind->index == ind_indexes.battery_charge)
        sendIndicatorBattChgToApp(link, ind->value);
        
    else if (ind->index == ind_indexes.call_hold_status)
        hfpHandleCallHeldIndication(link, (hfp_call_hold)ind->value);

    else if (link->ag_supported_indicators.num_extra_indicator_idxs)
        sendUnhandledIndicatorToApp(link, ind->index, ind->value);
}


/* Helper function to set the status of indicator of index indicator_idx in AT+BIA command */
static void hfpUpdateIndicatorsCommand(char* indicators_start, uint16 indicator_idx, hfp_indicator_status status)
{
    /* If indicator_idx is set and status is not the default */
    if(indicator_idx && status != hfp_indicator_off)
    {
        /* work out which char we want to modify in relation to first indicator */
        unsigned char_idx = (indicator_idx-1) * 2;
        /* set the char 
         * '1' for on. If off we do not change the value (assume already 0)
         * ' ' translates as 'use original value' broadly equivalent to hfp_indicator_ignore
         */
        *(indicators_start + char_idx) =  (status == hfp_indicator_on) ? '1':' ';
    }
}


/****************************************************************************
NAME    
    hfpHandleSetActiveIndicatorsReq

DESCRIPTION
    Handle request to set active indicators from the app

AT INDICATION
    +BIA=

RETURNS
    void
*/
void hfpHandleSetActiveIndicatorsReq(hfp_link_data* link)
{
    if(hfpLinkIsHfp107(link))
    {
        static const char atCmd[] = {'A','T','+','B','I','A','='};
        static const char atCmdSeparator[] = {'0',','};
        static const char atEnd[] = {'0','\r','\0'};
        
        /* 'AT+BIA=' takes 7 chars, two chars for each indicator plus three more for atEnd */
        unsigned size_bia = sizeof(atCmd) + (link->ag_supported_indicators.num_indicators * sizeof(atCmdSeparator)) + sizeof(atEnd);
        char *bia;
        char *p;
        
        bia = (char*)malloc(size_bia * sizeof(char));
        
        if(bia)
        {
            uint16 i;
            /* Add "AT+BIA=" */
            memmove(bia, atCmd, sizeof(atCmd));
            /* Fill in default value for all indicators "0,"*/
            for(p = bia + sizeof(atCmd); p < bia + size_bia - sizeof(atEnd); p += sizeof(atCmdSeparator))
                memmove(p, atCmdSeparator, sizeof(atCmdSeparator));
            /* Add "0\r\0" */
            memmove(p, atEnd, sizeof(atEnd));
            
            /* set p to point to the first indicator position in the command */
            p = bia + sizeof(atCmd);
            
            /* If app requested to turn indicators on update the message with char '1' */
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.service,         theHfp->optional_indicators.service        );
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.signal_strength, theHfp->optional_indicators.signal_strength);
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.roaming_status,  theHfp->optional_indicators.roaming_status );
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.battery_charge,  theHfp->optional_indicators.battery_charge );
            
            /* Make sure mandatory indicators are all on */
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.call,             hfp_indicator_on);
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.call_setup,       hfp_indicator_on);
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.call_hold_status, hfp_indicator_on);
            hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.indicator_idxs.extra_call_setup, hfp_indicator_on);
            
            /* Make sure any extra indicators the app cares about are on */
            for(i=0; i < link->ag_supported_indicators.num_extra_indicator_idxs; i++)
                hfpUpdateIndicatorsCommand(p, link->ag_supported_indicators.extra_indicator_idxs[i], hfp_indicator_on);
            
            /* Send the command */
            hfpSendAtCmd(link, (uint16)strlen(bia), bia, hfpBiaCmdPending);
            free(bia);
        }
        else
        {
            HFP_DEBUG(("BIA malloc failed\n"));
        }
    }
    else
    {
        /* Skip on the CMEE */
        hfpSendCommonInternalMessage(HFP_INTERNAL_AT_CMEE_REQ, link);
    }
}


/****************************************************************************
NAME    
    hfpHandleBiaAtAck

DESCRIPTION
    Handle Ack from AG in response to AT+BIA=

RETURNS
    void
*/
void hfpHandleBiaAtAck(hfp_link_data* link)
{
    hfpSendCommonInternalMessage(HFP_INTERNAL_AT_CMEE_REQ, link);
}
