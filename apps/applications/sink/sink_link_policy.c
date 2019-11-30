/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    Handles link policy and role switch settings across all devices/profiles
*/

/****************************************************************************
    Header files
*/
#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_private_data_config_def.h"
#include "sink_link_policy.h"
#include "sink_statemanager.h"
#include "sink_devicemanager.h"
#include "sink_swat.h"
#include "sink_audio_routing.h"

#include <sink.h>

#ifdef ENABLE_PBAP
#include <bdaddr.h>
#include "sink_pbap.h"
#endif

#ifdef ENABLE_PEER
/* headers required for sending DM_HCI_QOS_SETUP_REQ */
#include <vm.h>

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/hci.h>
#include <app/bluestack/dm_prim.h>
#endif


/* Lower power table for HFP SLC */
static const lp_power_table lp_powertable_default[2]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_passive,    0,              0,              0,          0,          30},    /* Passive mode 30 seconds */
    {lp_sniff,      800,            800,            2,          1,          0 }     /* Enter sniff mode (500mS)*/
};

/* Lower power table for HFP when an audio connection is open */
static const lp_power_table lp_powertable_sco[2]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_passive,    0,              0,              0,          0,          30},    /*Passive mode 30 seconds */
    {lp_sniff,      160,            160,            2,          1,          0}      /* Enter sniff mode (100mS)*/
};

static const lp_power_table lp_powertable_a2dp_default[2]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_active,     0,              0,              0,          0,          10},    /* Active mode 10 seconds */
    {lp_sniff,      800,            800,            2,          1,          0 }     /* Enter sniff mode (500mS)*/
};

/* Lower power table for A2DP. */
static const lp_power_table lp_powertable_a2dp_stream_sink[]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_passive,    0,              0,              0,          0,          0}      /* Go into passive mode and stay there */
};

/* Lower power table for A2DP. */
static const lp_power_table lp_powertable_a2dp_stream_source[]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_active,    0,              0,              0,          0,          0}      /* Go into active mode and stay there */
};

#if defined ENABLE_PBAP || defined ENABLE_GAIA
/* Power table for PBAP or GAIA DFU access. */
static const lp_power_table lp_powertable_data_access[]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_active,    0,              0,              0,          0,          0}      /* Go into active mode and stay there */
};
#endif /* defined ENABLE_PBAP || defined ENABLE_GAIA */

#ifdef ENABLE_AVRCP
/* Lower power table for AVRCP  */
static const lp_power_table lp_powertable_avrcp[]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_active,    0,              0,              0,          0,          AVRCP_ACTIVE_MODE_INTERVAL},      /* Go into active mode for 10 seconds*/
    {lp_sniff,     800,            800,            2,          1,          0 } 
};
#endif

#ifdef ENABLE_SUBWOOFER
static const lp_power_table lp_powertable_subwoofer[2]=
{
    /* mode,        min_interval,   max_interval,   attempt,    timeout,    duration */
    {lp_active,     0,              0,              0,          0,          10},    /* Active mode 10 seconds */
    {lp_sniff,      160,            160,            2,          1,          0}      /* Enter sniff mode (100mS)*/
};
#endif

#ifdef DEBUG_LP
    #define LP_DEBUG(x) DEBUG(x)
#else
    #define LP_DEBUG(x) 
#endif   


static uint16 linkPolicyNumberPhysicalConnections (void)
{
    uint16 connections = 0;
    Sink sink_pri, sink_sec, sink_a2dp_pri, sink_a2dp_sec;

    /* obtain any hfp link sinks */
    HfpLinkGetSlcSink(hfp_primary_link, &sink_pri);
    HfpLinkGetSlcSink(hfp_secondary_link, &sink_sec);
    
    /* obtain sinks for any a2dp links */
    sink_a2dp_pri = A2dpSignallingGetSink(getA2dpLinkDataDeviceId(a2dp_primary));
    sink_a2dp_sec = A2dpSignallingGetSink(getA2dpLinkDataDeviceId(a2dp_secondary));
    
    /* if primary hfp exists then check its role */
    if (sink_pri)
    {
        connections++;
    }
        
    /* if secondary hfp connection then check its role */    
    if (sink_sec)
    {
        connections++;
    }
    
    /* if primary a2dp exists and it is not the same device as pri or sec hfp connections */
    if (sink_a2dp_pri && !deviceManagerIsSameDevice(a2dp_primary, hfp_primary_link) && !deviceManagerIsSameDevice(a2dp_primary, hfp_secondary_link))
    {
        connections++;
    }
    
    /* if secondary a2dp exists and it is not the same device as pri or sec hfp connections */
    if (sink_a2dp_sec && !deviceManagerIsSameDevice(a2dp_secondary, hfp_primary_link) && !deviceManagerIsSameDevice(a2dp_secondary, hfp_secondary_link))
    {
        connections++;
    }
        
    LP_DEBUG(("LP: Number of physical connections = %u\n", connections ));    
    return connections;
}

/****************************************************************************
NAME    
    linkPolicyUseDefaultSettings

DESCRIPTION
    set the link policy based on no a2dp streaming or sco 
    
RETURNS
    void
*/
static void linkPolicyUseDefaultSettings(Sink sink)
{
    /* Set up our sniff sub rate params for SLC */
    power_table *userPowerTable = sinkDataGetUserPowerTable();
    ssr_params_t slc_params;
    sinkDataGetSsrSlcParams(&slc_params);
    
    ConnectionSetSniffSubRatePolicy(sink, slc_params.max_remote_ssr_latency, slc_params.min_remote_ssr_timeout, slc_params.min_local_ssr_timeout);
    
    /* audio not active, normal role, check for user defined power table */
    if((userPowerTable)&&(userPowerTable->normalEntries))
    {                  
        LP_DEBUG(("LP: SetLinkP - norm user table \n" ));    
        /* User supplied power table */
        ConnectionSetLinkPolicy(sink, userPowerTable->normalEntries ,&userPowerTable->powertable[0]);               
    }
    /* no user defined power table so use default normal power table */       
    else
    {    
        LP_DEBUG(("LP: SetLinkP - norm default table \n" ));    
        ConnectionSetLinkPolicy(sink, 2 ,lp_powertable_default);
    }              
}


/****************************************************************************
NAME    
    linkPolicyUseA2dpSettings

DESCRIPTION
    set the link policy requirements based on current device audio state 
    
RETURNS
    void
*/
void linkPolicyUseA2dpSettings(uint8 DeviceId, uint8 StreamId, Sink sink )
{
    Sink sinkAG1,sinkAG2;
    bool faster_poll = FALSE;
    power_table *userPowerTable = sinkDataGetUserPowerTable();
    /* obtain any sco sinks */
    HfpLinkGetAudioSink(hfp_primary_link, &sinkAG1);
    HfpLinkGetAudioSink(hfp_secondary_link, &sinkAG2);
    
    /* determine if the connection is currently streaming and there are no scos currently open */    
    if ((!sinkAG1 && !sinkAG2) && (A2dpMediaGetState(DeviceId, StreamId) == a2dp_stream_streaming))
                                
    {
        /* is there a user power table available from ps ? */
        if((userPowerTable) && (userPowerTable->A2DPStreamEntries))
        {                
            LP_DEBUG(("LP: SetLinkP - A2dp user table \n"))

            /* User supplied power table for A2DP role */
            ConnectionSetLinkPolicy(sink, 
                                    userPowerTable->A2DPStreamEntries ,
                                    &userPowerTable->powertable[userPowerTable->normalEntries +userPowerTable->SCOEntries  ]
                                    );  
        }
        /* no user power table so use default A2DP table */
        else
        {    
            if (A2dpMediaGetRole(DeviceId, StreamId) == a2dp_source)
            {
                LP_DEBUG(("LP: SetLinkP - A2dp stream source table \n" ));    
                ConnectionSetLinkPolicy(sink, 1 ,lp_powertable_a2dp_stream_source);
                faster_poll = TRUE;
            }
            else
            {
                LP_DEBUG(("LP: SetLinkP - A2dp stream sink table \n" ));    
                ConnectionSetLinkPolicy(sink, 1 ,lp_powertable_a2dp_stream_sink);
            }
        }                         
    }
    /* if not streaming a2dp check for the prescence of sco data and if none found go to normal settings */
    else if ((!sinkAG1 && !sinkAG2) && (A2dpMediaGetState(DeviceId, StreamId) != a2dp_stream_streaming))
    {
        uint16 priority;
        
        if (getA2dpIndex(DeviceId, &priority) && (getA2dpPeerRemoteDevice(priority) == remote_device_peer))
        {
            LP_DEBUG(("LP: SetLinkP - a2dp default table \n" ));    
            ConnectionSetLinkPolicy(sink, 2 ,lp_powertable_a2dp_default);
        }
        else
        {
            /* set normal link policy settings */
            linkPolicyUseDefaultSettings(sink);
        }
    }
    /* default to passive to prevent any stream distortion issues should the current
       operating mode be incorrectly identified */ 
    else
    {
           LP_DEBUG(("LP: SetLinkP - default A2dp stream sink table \n" ));    
           ConnectionSetLinkPolicy(sink, 1 ,lp_powertable_a2dp_stream_sink);
    }
    
#ifdef ENABLE_PEER
    {   /* Set a reasonable poll interval for the relay link to help if we ever get into a */
        /* scatternet situation due to an AV SRC refusing to be slave.                     */
#if 0
        typed_bdaddr tbdaddr;

        if (A2dpDeviceGetBdaddr(DeviceId, &tbdaddr.addr))
        {
            MESSAGE_MAKE(prim, DM_HCI_QOS_SETUP_REQ_T);
            prim->common.op_code = DM_HCI_QOS_SETUP_REQ;
            prim->common.length = sizeof(DM_HCI_QOS_SETUP_REQ_T);
            prim->bd_addr.lap = tbdaddr.addr.lap;
            prim->bd_addr.uap = tbdaddr.addr.uap;
            prim->bd_addr.nap = tbdaddr.addr.nap;

            /* latency is the only thing used in the request and sets the poll interval */
            prim->service_type = HCI_QOS_GUARANTEED;
            prim->token_rate = 0xffffffff;
            prim->peak_bandwidth = 0x0000aaaa;
            prim->latency = faster_poll ? 10000 : 25000;
            prim->delay_variation = 0xffffffff;

            DEBUG(("LP: SetLinkP - Set QoS %lums\n",prim->latency));
            VmSendDmPrim(prim);
        }
#else
            UNUSED(faster_poll);  /* Only used in code #if 0'd above */
#endif

        /* Check connection role is suitable too */
        linkPolicyGetRole(&sink);
    }
#else
    UNUSED(faster_poll);  /* Only used in code #if 0'd above */
#endif
}


/****************************************************************************
NAME    
    HfpSetLinkPolicy

DESCRIPTION
    set the link policy requirements based on current device audio state 
    
RETURNS
    void
*/
void linkPolicyUseHfpSettings(hfp_link_priority priority, Sink slcSink)
{
    Sink audioSink;
    power_table *userPowerTable = sinkDataGetUserPowerTable();
    /* determine if there are any sco sinks */
    if(HfpLinkGetAudioSink(priority, &audioSink))
    {
        /* Set up our sniff sub rate params for SCO */
        ssr_params_t sco_params;
        sinkDataGetSsrScoParams(&sco_params);
        ConnectionSetSniffSubRatePolicy(slcSink, sco_params.max_remote_ssr_latency, sco_params.min_remote_ssr_timeout, sco_params.min_local_ssr_timeout);
       
        /* is there a user power table available from ps ? */
        if((userPowerTable) && (userPowerTable->SCOEntries))
        {                
            LP_DEBUG(("LP: SetLinkP - sco user table \n" ));    
            /* User supplied power table for SCO role */
            ConnectionSetLinkPolicy(slcSink, 
                                    userPowerTable->SCOEntries ,
                                    &userPowerTable->powertable[userPowerTable->normalEntries ]
                                    );               
        }
        /* no user power table so use default SCO table */
        else
        {    
            LP_DEBUG(("LP: SetLinkP - sco default table \n" ));    
            ConnectionSetLinkPolicy(slcSink, 2 ,lp_powertable_sco);
        }              
    }
    /* default of no a2dp streaming and no sco link */
    else
    {
        /* set normal link policy settings */
        linkPolicyUseDefaultSettings(slcSink);
    }           
}

#ifdef ENABLE_AVRCP
/****************************************************************************
NAME
    linkPolicyUseAvrcpSettings

DESCRIPTION
    set the link policy requirements for AVRCP alone active connections

RETURNS
    void
*/
void linkPolicyUseAvrcpSettings( Sink slcSink )
{
    if(slcSink)
    {
        ConnectionSetLinkPolicy(slcSink, 2 ,lp_powertable_avrcp); 
    }
}
#endif /*ENABLE_AVRCP*/

#if defined ENABLE_PBAP || defined ENABLE_GAIA
/****************************************************************************
NAME	
	linkPolicyDataAccessComplete

DESCRIPTION
	set the link policy requirements back after data access, based on current
    device audio state 
	
RETURNS
	void
*/
static void linkPolicyDataAccessComplete(Sink sink)
{
    tp_bdaddr tpaddr;
    uint8 DeviceId;
    uint8 StreamId;
    uint8 i;
    bool  a2dpSetting = FALSE;
        
    LP_DEBUG(("LP: data access complete\n"));
    
    /* If device is in the stream a2dp state, use a2dp link policy */
    for_all_a2dp(i)
    {
        DeviceId = getA2dpLinkDataDeviceId(i);
        StreamId = getA2dpLinkDataStreamId(i);
           
        if( SinkGetBdAddr(sink, &tpaddr) &&
            BdaddrIsSame(getA2dpLinkBdAddr(i), &tpaddr.taddr.addr) )
        {
            a2dpSetting = TRUE;
            if(A2dpMediaGetState(DeviceId, StreamId)== a2dp_stream_streaming)
                linkPolicyUseA2dpSettings(DeviceId, StreamId, A2dpMediaGetSink(DeviceId, StreamId));
            else
                linkPolicyUseHfpSettings(hfp_primary_link, sink);
        }
    }
        
    /* Otherwise, use hfp link policy */
    if(!a2dpSetting)
    {
        linkPolicyUseHfpSettings(hfp_primary_link, sink); 
    }
}

static void linkPolicySetDataActiveMode(Sink sink)
{
    LP_DEBUG(("LP: Set Link in Active Mode for data access\n"));

    if(SinkIsValid(sink))
    {
        ConnectionSetLinkPolicy(sink, 1 , lp_powertable_data_access);
    } 
}
#endif /* defined ENABLE_PBAP || defined ENABLE_GAIA */

#ifdef ENABLE_PBAP
/****************************************************************************
NAME	
	linkPolicyPhonebookAccessComplete

DESCRIPTION
	set the link policy requirements back after phonebook access 
	
RETURNS
	void
*/
void linkPolicyPhonebookAccessComplete(Sink sink)
{
    if (pbapGetAccess())
    {
        LP_DEBUG(("LP: PBAP access complete\n"));
        
        if (!sinkDataGetDfuAccess())
        {
            linkPolicyDataAccessComplete(sink);
        }
    
        pbapSetAccess(FALSE);
    }
}

/****************************************************************************
NAME	
	linkPolicySetLinkinActiveMode

DESCRIPTION
	set the link as active mode for phonebook access 
	
RETURNS
	void
*/
void linkPolicySetLinkinActiveMode(Sink sink)
{
    LP_DEBUG(("LP: Set Link in Active Mode for PBAP\n"));

    if (!pbapGetAccess() && !sinkDataGetDfuAccess())
    {
        linkPolicySetDataActiveMode(sink);
    }
    
    pbapSetAccess(TRUE);
}
#endif /* ENABLE_PBAP */

#ifdef ENABLE_GAIA
/****************************************************************************
NAME	
	linkPolicyDfuAccessComplete

DESCRIPTION
	set the link policy requirements back after DFU access 
	
RETURNS
	void
*/
void linkPolicyDfuAccessComplete(Sink sink)
{
    if (sinkDataGetDfuAccess())
    {
        LP_DEBUG(("LP: DFU access complete\n"));
      
#ifdef ENABLE_PBAP       
        if (!pbapGetAccess())
#endif
            linkPolicyDataAccessComplete(sink);

        sinkDataSetDfuAccess(FALSE);
    }
}

/****************************************************************************
NAME	
	linkPolicySetDfuActiveMode

DESCRIPTION
	set the link as active mode for DFU access 
	
RETURNS
	void
*/
void linkPolicySetDfuActiveMode(Sink sink)
{
    LP_DEBUG(("LP: Set Link in Active Mode for DFU\n"));
#ifdef ENABLE_PBAP   
    if (!pbapGetAccess())
#endif
        if (!sinkDataGetDfuAccess())
        {
            linkPolicySetDataActiveMode(sink);
        }
    
    sinkDataSetDfuAccess(TRUE);
}
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
void linkPolicyGetRole(Sink* sink_passed)
{
    /* no specific sink to check, check all available - happens on the back of each hfp connect cfm 
     * and handleA2DPSignallingConnected
     */
    LP_DEBUG(("LP: linkPolicyGetRole - sink = %p\n",(void*)*sink_passed));
    if (sink_passed)
    {
        if (SinkIsValid(*sink_passed) )
        {
            /*only attempt to switch the sink that has failed to switch*/
            ConnectionGetRole(&theSink.task , *sink_passed) ;
            LP_DEBUG(("LP: GET 1 role[%p]\n", (void*)*sink_passed));
        }
    }
}


/****************************************************************************
NAME    
    linkPolicyHandleRoleInd
    
DESCRIPTION
    this is a function handles notification of a role change by a remote device

RETURNS
    void
*/
void linkPolicyHandleRoleInd (const CL_DM_ROLE_IND_T *ind)
{
    LP_DEBUG(("RoleInd, status=%u  role=%s\n", ind->status,  (ind->role == hci_role_master) ? "master" : "slave"));
    
    if (ind->status == hci_success)
    {
        uint16 num_sinks;
        Sink sink;
        tp_bdaddr tp_bd_addr;

        tp_bd_addr.taddr.type = TYPED_BDADDR_PUBLIC;
        tp_bd_addr.taddr.addr = ind->bd_addr;
        
        num_sinks = 1;
        sink = (Sink)NULL;
        
        if (StreamSinksFromBdAddr(&num_sinks, &sink, &tp_bd_addr))
        {
            sinkA2dpSetLinkRole(sink, ind->role);
        }
    }
}


/****************************************************************************
NAME    
    linkPolicyHandleRoleCfm
    
DESCRIPTION
    this is a function checks the returned role of the device and makes the decision of
    whether to change it or not, if it  needs changing it sends a role change reuest

RETURNS
    void
*/
void linkPolicyHandleRoleCfm(const CL_DM_ROLE_CFM_T *cfm)
{
    hci_role requiredRole = hci_role_dont_care;
    
    LP_DEBUG(("RoleConfirm, status=%u  sink=%p  role=%s\n", cfm->status, (void*)cfm->sink, (cfm->role == hci_role_master) ? "master" : "slave"));
   
    /* ensure role read successfully */
    if ((cfm->status == hci_success)&&(!sinkDataIsRoleSwitchingDisbaled()))
    {
        /* when multipoint enabled connect as master, this can be switched to slave
        later on if required when only 1 ag is connected */
        if((sinkDataIsMultipointEnabled()) && (linkPolicyNumberPhysicalConnections() > 1))
        {
#if defined ENABLE_PEER
            uint16 priority;
            
            if (getA2dpIndexFromSink(cfm->sink, &priority) && (getA2dpPeerRemoteDevice(priority) == remote_device_peer))
            {
                if (A2dpMediaGetRole(getA2dpLinkDataDeviceId(priority), getA2dpLinkDataStreamId(priority)) == a2dp_source)
                {
                    LP_DEBUG(("LP: Multipoint: Peer (relaying), require Master role\n")) ;
                    requiredRole = hci_role_master;
                    /* Set the link supervision timeout as the role switch will have reset it back
                       to firmware defaults */
                    ConnectionSetLinkSupervisionTimeout(cfm->sink, SINK_LINK_SUPERVISION_TIMEOUT);
                }
                else
                {
                    LP_DEBUG(("LP: Multipoint: Peer, don't change role\n")) ;
                    requiredRole = hci_role_dont_care;
                }
            }
            else
#endif
            {
#if defined ENABLE_PEER && defined PEER_SCATTERNET_DEBUG   /* Scatternet debugging only */
                if (getA2dpIndexFromSink(cfm->sink, &priority) && sinkA2dpIsAgRoleToBeInverted(priority))
                {
                    LP_DEBUG(("LP: Multipoint: Non-peer, require Slave role (inverted)\n")) ;
                    requiredRole = hci_role_slave;
                }
                else
#endif
                {
                    LP_DEBUG(("LP: Multipoint: Non-peer, require Master role\n")) ;
                    requiredRole = hci_role_master;
                    /* Set the link supervision timeout as the role switch will have reset it back
                       to firmware defaults */
                    ConnectionSetLinkSupervisionTimeout(cfm->sink, SINK_LINK_SUPERVISION_TIMEOUT);
                }
            }
        }
#ifdef ENABLE_SUBWOOFER
        /* when a sub woofer is in use the sink app needs to be master of all links
           to maintain stable connections */
        else if((cfm->status == hci_success)&&(SwatGetSignallingSink(sinkSwatGetDevId())))
        {
            LP_DEBUG(("LP: Subwoofer, require Master role\n")) ;
            requiredRole = hci_role_master;

            /* Restore the Set link supervision timeout to 1 second as this is reset after a role change */
            ConnectionSetLinkSupervisionTimeout(SwatGetSignallingSink(sinkSwatGetDevId()), SUBWOOFER_LINK_SUPERVISION_TIMEOUT);
        }        
#endif        
        /* non multipoint case, device needs to be slave */
        else
        {
             power_table *userPowerTable = sinkDataGetUserPowerTable();
            /* Set required role to slave as only one AG connected */
            if((userPowerTable)&&(userPowerTable->normalEntries))
            {   /* if user supplied role request then use that */
                LP_DEBUG(("LP: Singlepoint, require Master role\n")) ;
                requiredRole = userPowerTable->normalRole;
            }
            else 
            {
#if defined ENABLE_PEER
                uint16 priority;
            
                if (getA2dpIndexFromSink(cfm->sink, &priority) && (getA2dpPeerRemoteDevice(priority) == remote_device_peer))
                {
                    if (A2dpMediaGetRole(getA2dpLinkDataDeviceId(priority), getA2dpLinkDataStreamId(priority)) == a2dp_source)
                    {
                        LP_DEBUG(("LP: Singlepoint: Peer (relaying), require Master role\n")) ;
                        requiredRole = hci_role_master;
                        /* Set the link supervision timeout as the role switch will have reset it back
                           to firmware defaults */
                        ConnectionSetLinkSupervisionTimeout(cfm->sink, SINK_LINK_SUPERVISION_TIMEOUT);
                    }
                    else
                    {
                        LP_DEBUG(("LP: Singlepoint: Peer, don't change role\n")) ;
                        requiredRole = hci_role_dont_care;
                    }
                }
                else
#endif
                {   /* otherwise don't change the role */
                    LP_DEBUG(("LP: Singlepoint, don't change role\n")) ;
                    requiredRole = hci_role_dont_care;
                }
            }
        }
    }
    /* check for failure of role switch due to AG having a sco open, if this is the case then
    reschedule the role switch until it is successfull or fails completely */
    else if((cfm->status == hci_error_role_change_not_allowed)&&(!sinkDataIsRoleSwitchingDisbaled()))
    {
        LP_DEBUG(("LP: hci_error_role_change_not_allowed on sink = %p\n",(void*)cfm->sink));
    }
    /* check if the role switch is pending, possibly due to congestion. */
    else if(cfm->status == hci_error_role_switch_pending)
    {
        LP_DEBUG(("LP: hci_error_role_switch_pending on sink = %p\n",(void*)cfm->sink));  
    }
    /* automatic role switching is disabled, use the hfp_power_table pskey role requirements
       instead */
    else if(cfm->status == hci_success)
    {
        power_table *userPowerTable = sinkDataGetUserPowerTable();
        LP_DEBUG(("LP: Bypass Automatic role sw, use hfp_power_table role requirements\n")) ;

        /* check for the prescence of a user configured role requirement */
        if(userPowerTable)
        {
            /* determine device state, if stream a2dp check for power table entry and use that role
               if available */
            if((stateManagerGetState() == deviceA2DPStreaming)&&(userPowerTable->A2DPStreamEntries))
            {
                LP_DEBUG(("LP: Bypass: use A2dp role\n")) ;
                requiredRole = userPowerTable->A2DPStreamRole;
            }
            /* or if in call and sco is present check for sco power table entry and use role from that */
            else if((stateManagerGetState() > deviceConnected)&&(sinkAudioIsVoiceRouted())&&(userPowerTable->SCOEntries))
            {
                LP_DEBUG(("LP: Bypass: use SCO role\n")) ;
                requiredRole = userPowerTable->SCORole;
            }
            /* or default to normal role power table entry and use role from that */
            else if(userPowerTable->normalEntries)
            {                    
                LP_DEBUG(("LP: Bypass: use Normal role\n")) ;
                requiredRole = userPowerTable->normalRole;
            }
            /* if no suitable power table entries available then default to slave role */
            else
            {
                LP_DEBUG(("LP: Bypass: use default slave role\n")) ;
                requiredRole = hci_role_slave;
            }
        }
    }        
    
    /* Request a role change if required */
    if (requiredRole == hci_role_dont_care) 
    {
        /* Only set the local link role if not a 'pending' status */
        if(cfm->status == hci_error_role_switch_pending)
        {
            /* Pending status, ignore */
            LP_DEBUG(("LP: Role switch pending\n")) ;
        }
        else
        {
            LP_DEBUG(("LP: Role change set locally %s %p\n",(cfm->role == hci_role_master) ? "master" : "slave", (void*)cfm->sink)) ;
            
            /* Set the local role, even on error status */
            sinkA2dpSetLinkRole(cfm->sink, cfm->role);
        }
    }
    else
    {
        if (cfm->role == requiredRole)
        {
            LP_DEBUG(("LP: Role set locally, already %s %p\n",(requiredRole == hci_role_master) ? "master" : "slave", (void*)cfm->sink)) ;
            
            /* Set the local role */
            sinkA2dpSetLinkRole(cfm->sink, cfm->role);
        }
        else
        {
            LP_DEBUG(("LP: Set dev as %s %p\n",(requiredRole == hci_role_master) ? "master" : "slave", (void*)cfm->sink)) ;

            /* Request role change to the 'requiredRole' */
            ConnectionSetRole(&theSink.task, cfm->sink, requiredRole);             
        }
    }

}


/****************************************************************************
NAME    
    linkPolicyCheckRoles
    
DESCRIPTION
    this function obtains the sinks of any connection and performs a role check
    on them
RETURNS
    void
*/
void linkPolicyCheckRoles(void)
{
    Sink sink_pri, sink_sec, sink_a2dp_pri, sink_a2dp_sec;

    /* obtain any hfp link sinks */
    HfpLinkGetSlcSink(hfp_primary_link, &sink_pri);
    HfpLinkGetSlcSink(hfp_secondary_link, &sink_sec);
    
    /* obtain sinks for any a2dp links */
    sink_a2dp_pri = A2dpSignallingGetSink(getA2dpLinkDataDeviceId(a2dp_primary));
    sink_a2dp_sec = A2dpSignallingGetSink(getA2dpLinkDataDeviceId(a2dp_secondary));
    
    LP_DEBUG(("LP: linkPolicyCheckRoles: Hfp pri = %p, sec = %p, A2dp pri = %p, sec = %p\n",(void*)sink_pri
                                                                                           ,(void*)sink_sec
                                                                                           ,(void*)sink_a2dp_pri
                                                                                           ,(void*)sink_a2dp_sec)) ;

    /* if primary hfp exists then check its role */
    if(sink_pri)        
        linkPolicyGetRole(&sink_pri);
        
    /* if secondary hfp connection then check its role */    
    if(sink_sec)        
        linkPolicyGetRole(&sink_sec);

    /* if primary a2dp exists and it is not the same device as pri or sec hfp connections */
    if((sink_a2dp_pri)&&(!deviceManagerIsSameDevice(a2dp_primary, hfp_primary_link))&&(!deviceManagerIsSameDevice(a2dp_primary, hfp_secondary_link)))
        linkPolicyGetRole(&sink_a2dp_pri);
    
    /* if secondary a2dp exists and it is not the same device as pri or sec hfp connections */
    if((sink_a2dp_sec)&&(!deviceManagerIsSameDevice(a2dp_secondary, hfp_primary_link))&&(!deviceManagerIsSameDevice(a2dp_secondary, hfp_secondary_link)))
        linkPolicyGetRole(&sink_a2dp_sec);    

#ifdef ENABLE_SUBWOOFER
    /* check the subwoofer signalling sink if connected, this will have an impact of the role required for AG connections to prevent
       a scatternet scenario */
    if(SwatGetSignallingSink(sinkSwatGetDevId()))
    {
        Sink sink = SwatGetSignallingSink(sinkSwatGetDevId());
        linkPolicyGetRole(&sink);
    }
#endif        
}

/****************************************************************************
NAME    
    linkPolicyUpdateSwatLink
    
DESCRIPTION
    this function checks the current swat connection state and updates
    the link policy of the link
RETURNS
    void
*/
void linkPolicyUpdateSwatLink(void)
{
#ifdef ENABLE_SUBWOOFER
    /* attempt to get subwoofer signalling sink */
    Sink sink = SwatGetSignallingSink(sinkSwatGetDevId());

    /* determine if subwoofer is available */
    if(sink)
    {
        LP_DEBUG(("LP: SetLinkPolicy Swat\n" ));                 
        ConnectionSetLinkPolicy(sink, 2 , lp_powertable_subwoofer);
    }   
    else
    {
        /* no swat link */
        LP_DEBUG(("LP: SetLinkPolicy Swat - no link\n" ));                 
    }   
#endif      
}


