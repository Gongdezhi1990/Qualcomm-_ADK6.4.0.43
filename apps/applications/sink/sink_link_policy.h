/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_LINK_POLICY_H_
#define _SINK_LINK_POLICY_H_

#include <hfp.h>

/* link supervision timeout of 5 seconds (8000 * 0.625uS) */
#define SINK_LINK_SUPERVISION_TIMEOUT 0x1f80
        
/****************************************************************************
NAME	
	linkPolicyUseA2dpSettings

DESCRIPTION
	set the link policy requirements based on current device audio state 
	
RETURNS
	void
*/
void linkPolicyUseA2dpSettings(uint8 DeviceId, uint8 StreamId, Sink sink );


/****************************************************************************
NAME	
	linkPolicyUseHfpSettings

DESCRIPTION
	set the link policy requirements based on current device audio state 
	
RETURNS
	void
*/
void linkPolicyUseHfpSettings(hfp_link_priority priority, Sink slcSink );

#ifdef ENABLE_AVRCP
/****************************************************************************
NAME
    linkPolicyUseAvrcpSettings

DESCRIPTION
    set the link policy requirements for AVRCP alone active connections

RETURNS
    void
*/
void linkPolicyUseAvrcpSettings( Sink slcSink );
#endif

/****************************************************************************
NAME    
    linkPolicyGetRole
    
DESCRIPTION
    Request CL to get the role for a specific sink if one passed, or all
    connected HFP sinks if NULL passed.

RETURNS
    void
*/
void linkPolicyGetRole(Sink* sink_passed);


/****************************************************************************
NAME    
    linkPolicyHandleRoleInd
    
DESCRIPTION
    this is a function handles notification of a role change by a remote device

RETURNS
    void
*/
void linkPolicyHandleRoleInd (const CL_DM_ROLE_IND_T *ind);


/****************************************************************************
NAME
    linkPolicyHandleRoleCfm
    
DESCRIPTION
    this is a function checks the returned role of the device and makes the decision of
    whether to change it or not, if it  needs changing it sends a role change reuest

RETURNS
    nothing
*/
void linkPolicyHandleRoleCfm(const CL_DM_ROLE_CFM_T *cfm);

#ifdef ENABLE_PBAP

/****************************************************************************
NAME	
	linkPolicyPhonebookAccessComplete

DESCRIPTION
	set the link policy requirements back after phonebook access, based on current 
	device audio state 
	
RETURNS
	void
*/
void linkPolicyPhonebookAccessComplete(Sink sink);

/****************************************************************************
NAME	
	linkPolicySetLinkinActiveMode

DESCRIPTION
	set the link as active mode for phonebook access 
	
RETURNS
	void
*/
void linkPolicySetLinkinActiveMode(Sink sink);

/* PBAP only*/
#endif

#ifdef ENABLE_GAIA
/****************************************************************************
NAME	
	linkPolicyDfuAccessComplete

DESCRIPTION
	set the link policy requirements back after DFU access, based on current 
	device audio state 
	
RETURNS
	void
*/
void linkPolicyDfuAccessComplete(Sink sink);

/****************************************************************************
NAME	
	linkPolicySetDfuActiveMode

DESCRIPTION
	set the link as active mode for DFU access 
	
RETURNS
	void
*/
void linkPolicySetDfuActiveMode(Sink sink);
#endif /* ENABLE_GAIA */

/****************************************************************************
NAME    
    linkPolicyCheckRoles
    
DESCRIPTION
    this function obtains the sinks of any connection and performs a role check
    on them
RETURNS
    void
*/
void linkPolicyCheckRoles(void);

/****************************************************************************
NAME    
    linkPolicyUpdateSwatLink
    
DESCRIPTION
    this function checks the current swat state and media channel state and updates
    the link policy of the link or links
RETURNS
    void
*/
void linkPolicyUpdateSwatLink(void);

#endif /* _SINK_LINK_POLICY_H_ */

