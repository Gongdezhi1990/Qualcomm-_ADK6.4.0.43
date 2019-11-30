/*
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    This module manages the link loss recovery mechanisms provided by the HFP and A2DP libraries.
*/

#ifndef _SINK_LINKLOSS_H_
#define _SINK_LINKLOSS_H_


/*!
    @brief __linkloss_stream_protection_state
 
    This enum defines the possible stream protection states.
*/
typedef enum __linkloss_stream_protection_state
{
    linkloss_stream_protection_off     = 0x0,
    linkloss_stream_protection_on      = 0x1
}linkloss_stream_protection_state_t;


/*************************************************************************
NAME    
    linklossProtectStreaming
    
DESCRIPTION
    This function enables/disables the stream protection from link loss 
    recovery mechanisms.
    
RETURNS
    
**************************************************************************/
void linklossProtectStreaming(linkloss_stream_protection_state_t new_state);

/*************************************************************************
NAME    
    linklossUpdateManagement
    
DESCRIPTION
    Enables/disabled the HFP and A2DP library managed link loss recovery 
    mechanism dependant on stream_protection_state.    
    
RETURNS
    
**************************************************************************/
void linklossUpdateManagement(const bdaddr *bd_addr);

/*************************************************************************
NAME    
    linklossSendLinkLossTone

DESCRIPTION
    Send a one-off event to trigger a link loss tone for a given device.

RETURNS

**************************************************************************/
void linklossSendLinkLossTone(bdaddr *bd_addr, uint32 delay);

/*************************************************************************
NAME    
    linklossCancelLinkLossTone

DESCRIPTION
    Cancel any queued link loss tone events.
    Note this will cancel *all* events and not just for a particular device.

RETURNS

**************************************************************************/
void linklossCancelLinkLossTone(void);

/*************************************************************************
NAME    
    linklossIsStreamProtected

DESCRIPTION
    Check if the a2dp stream for a device is marked as protected.

RETURNS
    TRUE if the stream is protected, FALSE otherwise

**************************************************************************/
bool linklossIsStreamProtected(const bdaddr *bd_addr);

/*************************************************************************
NAME    
    linklossResetStreamProtection

DESCRIPTION
    Clear the stream protection flag for the given a2dp link.
    This is intended to be used only when a new a2dp link connects.
    In any other case linklossProtectStreaming should be called.

**************************************************************************/
void linklossResetStreamProtection(uint16 a2dp_index);

#endif  /* _SINK_LINKLOSS_H_ */
