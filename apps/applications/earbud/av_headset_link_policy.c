/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_link_policy.c
\brief      Link policy manager
*/

#include <panic.h>
#include <connection.h>
#include <sink.h>

#include "av_headset.h"
#include "av_headset_log.h"
#include <app/bluestack/dm_prim.h>

/*! Make and populate a bluestack DM primitive based on the type. 

    \note that this is a multiline macro so should not be used after a
    control statement (if, while) without the use of braces
 */
#define MAKE_PRIM_C(TYPE) MESSAGE_MAKE(prim,TYPE##_T); prim->common.op_code = TYPE; prim->common.length = sizeof(TYPE##_T);

/*! Lower power table for A2DP */
static const lp_power_table powertable_a2dp[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_active,     0,            0,            0,       0,       5},  /* Active mode for 5 sec */
    {lp_passive,    0,            0,            0,       0,       1},  /* Passive mode for 1 sec */
    {lp_sniff,      48,           400,          2,       4,       0}   /* Enter sniff mode*/
};

/*! Lower power table for the A2DP with media streaming as source */
static const lp_power_table powertable_a2dp_streaming_source[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_passive,    0,            0,            0,       0,       0},  /* Passive mode */
};

/*! Lower power table for the A2DP with media streaming as TWS sink */
static const lp_power_table powertable_a2dp_streaming_tws_sink[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_active,     0,            0,            0,       0,       5},  /* Active mode for 5 sec */
    {lp_passive,    0,            0,            0,       0,       1},  /* Passive mode for 1 sec */
    {lp_sniff,      48,           48,           2,       4,       0}   /* Enter sniff mode*/
};

/*! Lower power table for the A2DP with media streaming as sink */
static const lp_power_table powertable_a2dp_streaming_sink[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_active,     0,            0,            0,       0,       5},  /* Active mode for 5 sec */
    {lp_passive,    0,            0,            0,       0,       1},  /* Passive mode for 1 sec */
    {lp_sniff,      48,           48,           2,       4,       0}   /* Enter sniff mode*/
};

/*! Lower power table for the HFP. */
static const lp_power_table powertable_hfp[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_active,     0,            0,            0,       0,       1},  /* Active mode for 1 second */
    {lp_sniff,      48,           800,          2,       1,       0}   /* Enter sniff mode (30-500ms)*/
};

/*! Lower power table for the HFP when an audio connection is open 
*/
static const lp_power_table powertable_hfp_sco[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_passive,    0,            0,            0,       0,       1},  /* Passive mode */
    {lp_sniff,      48,           144,          2,       8,       0}   /* Enter sniff mode (30-90ms)*/
};

/*! Power table for the peer link when SCO forwarding active
*/
static const lp_power_table powertable_peer_SCO_fwd[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_active,     0,            0,            0,       0,       5},  /* Active mode for 5 seconds */
    {lp_passive,    0,            0,            0,       0,       10}, /* Passive mode. 10 seconds... redial ? */
    {lp_sniff,      48,           144,          2,       8,       0}   /* Enter sniff mode (30-90ms)*/
};

/*! Lower power table for TWS+ HFP when an audio connection is open */
static const lp_power_table powertable_twsplus_hfp_sco[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_sniff,      48,           144,          2,       8,       0}   /* Stay in sniff mode */
};

/*! Lower power table for AVRCP */
static const lp_power_table powertable_avrcp[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_active,     0,            0,            0,       0,       1},  /* Active mode for 1 second */
    {lp_sniff,      48,           800,          1,       0,       0}   /* Enter sniff mode*/
};

/*! Lower power table for PEER Link wiht TWS+ and streaming */
static const lp_power_table powertable_twsplus_peer_streaming[]=
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_sniff,      80,          100,             2,       4,      0}   /* Stay in sniff mode */
};

/*! \cond helper */
#define ARRAY_AND_DIM(ARRAY) (ARRAY), ARRAY_DIM(ARRAY)
/*! \endcond helper */

/*! Structure for storing power tables */
struct powertable_data
{
    const lp_power_table *table;
    uint16 rows;
};

/*! Array of structs used to store the power tables for standard phones */
static const struct powertable_data powertables_standard[] = {
    [POWERTABLE_A2DP] =                    {ARRAY_AND_DIM(powertable_a2dp)},
    [POWERTABLE_A2DP_STREAMING_SOURCE] =   {ARRAY_AND_DIM(powertable_a2dp_streaming_source)},
    [POWERTABLE_A2DP_STREAMING_TWS_SINK] = {ARRAY_AND_DIM(powertable_a2dp_streaming_tws_sink)},
    [POWERTABLE_A2DP_STREAMING_SINK] =     {ARRAY_AND_DIM(powertable_a2dp_streaming_sink)},
    [POWERTABLE_HFP] =                     {ARRAY_AND_DIM(powertable_hfp)},
    [POWERTABLE_HFP_SCO] =                 {ARRAY_AND_DIM(powertable_hfp_sco)},
    [POWERTABLE_AVRCP] =                   {ARRAY_AND_DIM(powertable_avrcp)},    
    [POWERTABLE_PEER_MODE] =               {ARRAY_AND_DIM(powertable_peer_SCO_fwd)},
};

/*! Array of structs used to store the power tables for TWS+ phones */
static const struct powertable_data powertables_twsplus[] = {
    [POWERTABLE_A2DP] =                    {ARRAY_AND_DIM(powertable_a2dp)},
    [POWERTABLE_A2DP_STREAMING_SOURCE] =   {ARRAY_AND_DIM(powertable_a2dp_streaming_source)},
    [POWERTABLE_A2DP_STREAMING_TWS_SINK] = {ARRAY_AND_DIM(powertable_a2dp_streaming_tws_sink)},
    [POWERTABLE_A2DP_STREAMING_SINK] =     {ARRAY_AND_DIM(powertable_a2dp_streaming_sink)},
    [POWERTABLE_HFP] =                     {ARRAY_AND_DIM(powertable_hfp)},
    [POWERTABLE_HFP_SCO] =                 {ARRAY_AND_DIM(powertable_twsplus_hfp_sco)},
    [POWERTABLE_AVRCP] =                   {ARRAY_AND_DIM(powertable_avrcp)},    
    [POWERTABLE_PEER_MODE] =               {ARRAY_AND_DIM(powertable_twsplus_peer_streaming)},
};

void appLinkPolicyUpdateLinkSupervisionTimeout(const bdaddr *bd_addr)
{
    uint16 timeout;

    if (appDeviceIsPeer(bd_addr))
        timeout = appConfigEarbudLinkSupervisionTimeout() * 1000UL / 625;
    else
        timeout = appConfigDefaultLinkSupervisionTimeout() * 1000UL / 625;

    MAKE_PRIM_C(DM_HCI_WRITE_LINK_SUPERV_TIMEOUT_REQ);
    prim->handle = 0;
    BdaddrConvertVmToBluestack(&prim->bd_addr, bd_addr);
    prim->timeout = timeout;
    VmSendDmPrim(prim);
}

/* \brief Re-check and select link settings to reduce power consumpt
 * ion
        where possible

    This function checks what activity the application currently has,
    and decides what the best link settings are for the connection 
    to the specified device. This may include full power (#lp_active), 
    sniff (#lp_sniff), or passive(#lp_passive) where full power is 
    no longer required but the application would prefer not to enter
    sniff mode yet.

    \param bd_addr  Bluetooth address of the device to update link settings
*/
void appLinkPolicyUpdatePowerTable(const bdaddr *bd_addr)
{
    lpPowerTableIndex pt_index = POWERTABLE_UNASSIGNED;
    avInstanceTaskData *av_inst = NULL;
    Sink sink = 0;
    lpPerConnectionState old_lp_state;
    lpPerConnectionState new_lp_state;
    bdaddr handset_bd_addr;
    bool isTwsPlusHandset;
    lpPowerTableSet powertable_set;

    /* Determine if we are in TWS+ moe as we have different settings */
    isTwsPlusHandset = appDeviceGetHandsetBdAddr(&handset_bd_addr) &&
                       appDeviceIsTwsPlusHandset(&handset_bd_addr);
    powertable_set = isTwsPlusHandset ? POWERTABLE_SET_TWSPLUS
                                      : POWERTABLE_SET_NORMAL;

    /* Update peer link power table if we are SCO forwarding */
    if (appDeviceIsPeer(bd_addr) && appConfigScoForwardingEnabled() && appScoFwdIsStreaming())
    {
        pt_index = POWERTABLE_PEER_MODE;
        sink = appScoFwdGetSink();
    }
    /* Update peer power table if we are streamng in TWS+ mode */
    else if (appDeviceIsPeer(bd_addr) &&
             appDeviceIsHandsetA2dpStreaming() &&
             isTwsPlusHandset)
    {
        av_inst = appAvInstanceFindFromBdAddr(bd_addr);
        if (av_inst)
        {
            DEBUG_LOG("appLinkPolicyUpdatePowerTable Updating peer lp mode due to TWS+ streaming");
            pt_index = POWERTABLE_PEER_MODE;
            sink = appAvGetSink(av_inst);
        }
    }
    else
#ifdef INCLUDE_HFP
    if (appHfpIsScoActive())
    {
        pt_index = POWERTABLE_HFP_SCO;
        sink = appHfpGetSink();
    }
#endif
#ifdef INCLUDE_AV
    if (pt_index == POWERTABLE_UNASSIGNED)
    {
        av_inst = appAvInstanceFindFromBdAddr(bd_addr);
        if (av_inst)
        {
            if (appA2dpIsStreaming(av_inst))
            {
                if (appA2dpIsSinkNonTwsCodec(av_inst))
                {
                    pt_index = POWERTABLE_A2DP_STREAMING_SINK;
                }
                else if (appA2dpIsSinkTwsCodec(av_inst))
                {
                    pt_index = POWERTABLE_A2DP_STREAMING_TWS_SINK;
                }
                else if (appA2dpIsSourceCodec(av_inst))
                {
                    pt_index = POWERTABLE_A2DP_STREAMING_SOURCE;
                }
            }
            else if (!appA2dpIsDisconnected(av_inst))
            {
                pt_index = POWERTABLE_A2DP;
            }
            else if (appAvrcpIsConnected(av_inst))
            {
                pt_index = POWERTABLE_AVRCP;
            }
            sink = appAvGetSink(av_inst);
        }
    }
#endif
#ifdef INCLUDE_HFP
    if (pt_index == POWERTABLE_UNASSIGNED)
    {
        if (appHfpIsConnected())
        {
            pt_index = POWERTABLE_HFP;
            sink = appHfpGetSink();
        }
    }
#endif

    appConManagerGetLpState(bd_addr, &old_lp_state);
    new_lp_state.pt_index = pt_index;
    new_lp_state.table_set_used = powertable_set;

    if (   !appConManagerLpStateSame(&old_lp_state, &new_lp_state)
        && sink 
        && (pt_index < POWERTABLE_UNASSIGNED))
    {
        const struct powertable_data *selected = isTwsPlusHandset ?
                                                    &powertables_twsplus[pt_index] :
                                                    &powertables_standard[pt_index];

        ConnectionSetLinkPolicy(sink, selected->rows, selected->table);
        if(appDeviceIsPeer(bd_addr))
        {
            DEBUG_LOGF("appLinkPolicyUpdatePowerTable for peer, index=%d, prev=%d TWS+:%d x%p", 
                            pt_index, old_lp_state.pt_index, isTwsPlusHandset, selected->table);
        }
        else
        {
            DEBUG_LOGF("appLinkPolicyUpdatePowerTable, index=%d, prev=%d TWS+:%d x%p", 
                            pt_index, old_lp_state.pt_index, isTwsPlusHandset, selected->table);
        }

        appConManagerSetLpState(bd_addr, &new_lp_state);
    }
}

/*! \brief Allow role switching 

    Update the policy for the connection (if any) to the specified
    Bluetooth address, so as to allow future role switching.

    \param  bd_addr The Bluetooth address of the device
*/
void appLinkPolicyAllowRoleSwitch(const bdaddr *bd_addr)
{
    MAKE_PRIM_C(DM_HCI_WRITE_LINK_POLICY_SETTINGS_REQ);
    BdaddrConvertVmToBluestack(&prim->bd_addr, bd_addr);
    prim->link_policy_settings = ENABLE_MS_SWITCH | ENABLE_SNIFF;
    VmSendDmPrim(prim);
    DEBUG_LOG("appLinkPolicyAllowRoleSwitch");
}

/*! \brief Allow role switching
    \param sink The sink for which to allow role switching
*/
void appLinkPolicyAllowRoleSwitchForSink(Sink sink)
{
    tp_bdaddr bd_addr;
    if (SinkGetBdAddr(sink, &bd_addr))
    {
        appLinkPolicyAllowRoleSwitch(&bd_addr.taddr.addr);
    }
}

/*! \brief Prevent role switching 

    Update the policy for the connection (if any) to the specified
    Bluetooth address, so as to prevent any future role switching.

    \param  bd_addr The Bluetooth address of the device
*/
void appLinkPolicyPreventRoleSwitch(const bdaddr *bd_addr)
{
    MAKE_PRIM_C(DM_HCI_WRITE_LINK_POLICY_SETTINGS_REQ);
    BdaddrConvertVmToBluestack(&prim->bd_addr, bd_addr);
    prim->link_policy_settings = ENABLE_SNIFF;
    VmSendDmPrim(prim);
    DEBUG_LOG("appLinkPolicyPreventRoleSwitch");
}

/*! \brief Prevent role switching
    \param sink The sink for which to prevent role switching
*/
void appLinkPolicyPreventRoleSwitchForSink(Sink sink)
{
    tp_bdaddr bd_addr;
    if (SinkGetBdAddr(sink, &bd_addr))
    {
        appLinkPolicyPreventRoleSwitch(&bd_addr.taddr.addr);
    }
}

/*! \brief Check and update links

    This function checks the role of the individual links and attempts
    role switches when required.
    
    Role switches are only attempts to avoid scatternets.

    We don't try to role switch when streaming from the phone
    We don't try to role switch when forwarding to the peer


    TWS Standard Setup - Need to setup before starting streaming or SCO

                        Peer Disc.  Peer Conn.
        Handset Disc.      N/A         P:D
        Handset AV         H:M       H:M  P:M
        Handset HFP        H:M       H:M  P:M


    TWS+ Setup - Need to setup before starting streaming with TWS+ codec or SCO

                        Peer Disc.  Peer Conn.
        Handset Disc.      N/A       H:X  P:D
        Handset AV         H:S       H:S  P:M
        Handset HFP        H:S       H:S  P:M
*/    
static void appLinkPolicyCheckRole(void)
{
    avInstanceTaskData *av_inst_source, *av_inst_sink, *tws_inst_sink;
    lpTaskData *theLp = appGetLp();
    bdaddr handset_bd_addr;
    
    DEBUG_LOG("appLinkPolicyCheckRole");

    /* We don't need to role switch if don't have a handset connected */
    if (!appDeviceIsHandsetAnyProfileConnected())
    {
        DEBUG_LOG("appLinkPolicyCheckRole, no handset connected");
        return;
    }

    /* We have a handset connected */

    /* Check the links if connected to a TWS Standard device */
    if (appDeviceGetHandsetBdAddr(&handset_bd_addr) && !appDeviceIsTwsPlusHandset(&handset_bd_addr))
    {
        /* Get AV source instance (NULL if not connected) */
        av_inst_source = appAvGetA2dpSource();
        av_inst_sink = appAvGetA2dpSink(AV_CODEC_NON_TWS);
        bool src_streaming = (av_inst_source != NULL) && appA2dpIsStreaming(av_inst_source);
        bool sink_streaming = (av_inst_sink != NULL) && appA2dpIsStreaming(av_inst_sink);

        /* Check the current role of the peer link
         * Only allowed to role switch if we are not streaming or SCO forwarding to the peer */

        DEBUG_LOG("src_streaming %u, appScoFwdIsSending %u, appScoFwdIsConnected %u",
                  src_streaming, appScoFwdIsSending(), appScoFwdIsConnected());
        
        if (!src_streaming && !appScoFwdIsSending())
        {
            /* Check if we need to role swap sco forwarding link */
            if (appScoFwdIsConnected() && theLp->scofwd_role != hci_role_master)
            {
                DEBUG_LOG("appLinkPolicyCheckRole, ScoFwd link is slave, attemping role switch");
                ConnectionSetRole(&theLp->task, appScoFwdGetSink(), hci_role_master);
                theLp->scofwd_role = hci_role_master;
            }

            /* Try and be master of the source link to the peer */
            if (av_inst_source && (theLp->av_source_role != hci_role_master))
            {
                DEBUG_LOG("appLinkPolicyCheckRole, av_source link is slave, attempting role switch");
                ConnectionSetRole(&theLp->task, appAvGetSink(av_inst_source), hci_role_master);
                theLp->av_source_role = hci_role_master;
            }
        }
        else
        {
            DEBUG_LOG("appLinkPolicyCheckRole, can't switch peer as streaming or sco forwarding");
        }

        /* Check current role of the handset link
         * Only allowed to role switch if we are not streaming and don't have an active SCO */
        if (!sink_streaming && !appHfpIsScoActive())
        {
            /* Try and be the master of the HFP link */
            if (appHfpIsConnected() && (theLp->hfp_role != hci_role_master))
            {
                DEBUG_LOG("appLinkPolicyCheckRole, hfp link is slave, attempting role switch");
                ConnectionSetRole(&theLp->task, appHfpGetSink(), hci_role_master);
                theLp->hfp_role = hci_role_master;
            }

            /* Try and be master of the sink link to the handset */
            if (av_inst_sink && (theLp->av_sink_role != hci_role_master))
            {
                DEBUG_LOG("appLinkPolicyCheckRole, av_sink link is slave, attempting role switch");
                ConnectionSetRole(&theLp->task, appAvGetSink(av_inst_sink), hci_role_master);
                theLp->av_sink_role = hci_role_master;
            }
        }
    }
    else /* TWS+ Handset */
    {
        tws_inst_sink = appAvGetA2dpSink(AV_CODEC_TWS);
        bool tws_sink_streaming = (tws_inst_sink != NULL) && appA2dpIsStreaming(tws_inst_sink);

        /* Check current role of the handset link
         * Only allowed to role switch if we are not streaming and don't have an active SCO */
        if (!tws_sink_streaming && !appHfpIsScoActive())
        {
            /* Try and be the slave of the HFP link */
            if (appHfpIsConnected() && (theLp->hfp_role != hci_role_slave))
            {
                DEBUG_LOG("appLinkPolicyCheckRole, hfp link is master to TWS+ phone, attempting role switch");
                ConnectionSetRole(&theLp->task, appHfpGetSink(), hci_role_slave);
                theLp->hfp_role = hci_role_slave;
            }

            if (tws_inst_sink && (theLp->av_sink_role != hci_role_slave))
            {
                DEBUG_LOG("appLinkPolicyCheckRole, tws_sink link is master to TWS+ phone, attempting role switch");
                ConnectionSetRole(&theLp->task, appAvGetSink(tws_inst_sink), hci_role_slave);
                theLp->av_sink_role = hci_role_slave;
            }
        }
    }
}

/*! \brief Update role of link

    This function is called whenver the role of a link has changed or been
    confirmed, it checks the Bluetooth Address of the updated link against
    the address of the HFP and A2DP links and updates the role of the matching
    link.
*/    
static void appLinkPolicyUpdateRole(const bdaddr *bd_addr, hci_role role)
{
#ifdef INCLUDE_AV
    lpTaskData *theLp = appGetLp();
    avInstanceTaskData *av_source_inst, *av_sink_inst;
               
    if (role == hci_role_master)
        appLinkPolicyUpdateLinkSupervisionTimeout(bd_addr);

    /* Check if role confirmation for AV sink link */
    av_sink_inst = appAvGetA2dpSink(AV_CODEC_ANY);
    if (av_sink_inst != NULL)
    {
        tp_bdaddr av_bd_addr;        
        if (SinkGetBdAddr(appAvGetSink(av_sink_inst), &av_bd_addr) &&
            BdaddrIsSame(bd_addr, &av_bd_addr.taddr.addr))
        {
            if (role == hci_role_master)
                DEBUG_LOG("appLinkPolicyUpdateRole, av sink, role=master");
            else
                DEBUG_LOG("appLinkPolicyUpdateRole, av sink, role=slave");

            theLp->av_sink_role = role;
        }
    }
    else
    {
        /* We don't have a link so reset the role*/
        theLp->av_sink_role = hci_role_dont_care;
    }

    /* Check if role confirmation for AV source link  */
    av_source_inst = appAvGetA2dpSource();
    if (av_source_inst != NULL)
    {
        tp_bdaddr av_bd_addr;
        if (SinkGetBdAddr(appAvGetSink(av_source_inst), &av_bd_addr) &&
            BdaddrIsSame(bd_addr, &av_bd_addr.taddr.addr))
        {
            if (role == hci_role_master)
                DEBUG_LOG("appLinkPolicyUpdateRole, av relay, role=master");
            else
                DEBUG_LOG("appLinkPolicyUpdateRole, av relay, role=slave");

            theLp->av_source_role = role;
        }
    }
    else
    {
        /* We don't have a link so reset the role*/
        theLp->av_source_role = hci_role_dont_care;
    }

    if (appConfigScoForwardingEnabled() && appScoFwdIsConnected())
    {
        tp_bdaddr scofwd_bdaddr;
        if (   SinkGetBdAddr(appScoFwdGetSink(), &scofwd_bdaddr)
            && BdaddrIsSame(bd_addr,&scofwd_bdaddr.taddr.addr))
        {
            if (role == hci_role_master)
            {
                DEBUG_LOG("appLinkPolicyUpdateRole, scofwd, role=master");
            }
            else
            {
                DEBUG_LOG("appLinkPolicyUpdateRole, scofwd, role=slave");
            }

            theLp->scofwd_role = role;
            
            /* Inform SCO forwarding of current role */
            appScoFwdNotifyRole(role);
        }
    }
    else
    {
        /* We don't have a link so reset the role*/
        theLp->scofwd_role = hci_role_dont_care;
    }

#ifdef INCLUDE_HFP
    /* Check if role confirmation for HFP link */
    if (appHfpIsConnected())
    {
        tp_bdaddr hfp_bd_addr;
        if (SinkGetBdAddr(appHfpGetSink(), &hfp_bd_addr) &&
            BdaddrIsSame(bd_addr, &hfp_bd_addr.taddr.addr))
        {
            if (role == hci_role_master)
                DEBUG_LOG("appLinkPolicyUpdateRole, hfp, role=master");
            else
                DEBUG_LOG("appLinkPolicyUpdateRole, hfp, role=slave");

            theLp->hfp_role = role;
        }
    }
    else
    {
        /* We don't have a link so reset the role*/
        theLp->hfp_role = hci_role_dont_care;
    }
#endif

#endif
}

/*! \brief Confirmation of link role

    This function is called to handle a CL_DM_ROLE_CFM message, this message is sent from the
    connection library in response to a call to ConnectionGetRole().
    
    Extract the Bluetooth address of the link and call appLinkPolicyUpdateRole to update the
    role of the appropriate link.

    \param  cfm The received confirmation
*/    
static void appLinkPolicyHandleClDmRoleConfirm(CL_DM_ROLE_CFM_T *cfm)
{
    tp_bdaddr bd_addr;

    if (appConfigScoForwardingEnabled())
    {
        CL_DM_ROLE_CFM_T *fwd = PanicUnlessNew(CL_DM_ROLE_CFM_T);
        *fwd = *cfm;

        MessageSend(&testTask,CL_DM_ROLE_CFM,fwd);
    }

    if (cfm->role == hci_role_master)
        DEBUG_LOGF("appLinkPolicyHandleClDmRoleConfirm, status=%d, role=master", cfm->status);
    else
        DEBUG_LOGF("appLinkPolicyHandleClDmRoleConfirm, status=%d, role=slave", cfm->status);
    
    if (SinkGetBdAddr(cfm->sink, &bd_addr))
    {
        appLinkPolicyUpdateRole(&bd_addr.taddr.addr, cfm->role);
        if (cfm->status == hci_success)
            appLinkPolicyCheckRole();
    }
}

/*! \brief Indication of link role

    This function is called to handle a CL_DM_ROLE_IND message, this message is sent from the
    connection library whenever the role of a link changes.
    
    Extract the Bluetooth address of the link and call appLinkPolicyUpdateRole to update the
    role of the appropriate link.  Call appLinkPolicyCheckRole to check if we need to perform
    a role switch.

    \param  ind The received indication
*/
static void appLinkPolicyHandleClDmRoleIndication(CL_DM_ROLE_IND_T *ind)
{
    if (ind->role == hci_role_master)
        DEBUG_LOG("appLinkPolicyHandleClDmRoleIndication, master");
    else
        DEBUG_LOG("appLinkPolicyHandleClDmRoleIndication, slave");

    appLinkPolicyUpdateRole(&ind->bd_addr, ind->role);
    appLinkPolicyCheckRole();
}


bool appLinkPolicyHandleConnectionLibraryMessages(MessageId id,Message message, bool already_handled)
{
    switch (id)
    {
        case CL_DM_ROLE_CFM:
            appLinkPolicyHandleClDmRoleConfirm((CL_DM_ROLE_CFM_T *)message);
            return TRUE;
        
        case CL_DM_ROLE_IND:
            appLinkPolicyHandleClDmRoleIndication((CL_DM_ROLE_IND_T *)message);
            return TRUE;
    }
    return already_handled;
}


/*! \brief Get updated link role

    Request from application to check and update the link role for the
    specified sink.

    \param  sink    The Sink to check and update
*/
void appLinkPolicyUpdateRoleFromSink(Sink sink)
{
    lpTaskData *theLp = appGetLp();   

    /* Get current role for this link */
    ConnectionGetRole(&theLp->task, sink);
}

/*! \brief Initialise link policy manager

    Call as startyp to initialise the link policy manager, set all
    the store rols to 'don't care'.
*/    
void appLinkPolicyInit(void)
{
    lpTaskData *theLp = appGetLp();   

#ifdef INCLUDE_AV
    theLp->av_sink_role = hci_role_dont_care;
    theLp->av_source_role = hci_role_dont_care;
#endif
#ifdef INCLUDE_HFP
    theLp->hfp_role = hci_role_dont_care;
#endif
    theLp->scofwd_role = hci_role_dont_care;
}
