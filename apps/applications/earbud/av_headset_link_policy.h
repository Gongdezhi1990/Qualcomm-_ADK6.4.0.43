/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_link_policy.h
\brief	    Header file for the Link policy manager
*/

#ifndef __AV_HEADSET_LINK_POLICY_H
#define __AV_HEADSET_LINK_POLICY_H

#include "av_headset.h"

/*! Link policy task structure */
typedef struct
{
    TaskData task;              /*!< Link policy manager task */
    /*! \todo How do we end up with 3 (now 4) link states when
        we just have AG & peer */
#ifdef INCLUDE_HFP
    hci_role hfp_role:2;        /*!< Current role of HFP link */
#endif
#ifdef INCLUDE_AV
    hci_role av_sink_role:2;    /*!< Current role of AV link as A2DP Sink */
    hci_role av_source_role:2;  /*!< Current role of AV link as A2DP Source */
#endif
    hci_role scofwd_role:2;     /*!< Current role of peer link */
} lpTaskData;

#define appLinkPolicyGetCurrentScoFwdRole() \
    appGetLp()->scofwd_role

/*! Power table indexes */
typedef enum lp_power_table_index
{
    POWERTABLE_A2DP,
    POWERTABLE_A2DP_STREAMING_SOURCE,
    POWERTABLE_A2DP_STREAMING_TWS_SINK,
    POWERTABLE_A2DP_STREAMING_SINK,
    POWERTABLE_HFP,
    POWERTABLE_HFP_SCO,
    POWERTABLE_AVRCP,     /* Used for both Handset and Peer links */
    POWERTABLE_PEER_MODE, /* Used Peer links when SCO forwarding or TWS+ with A2DP Streaming */
    /*! Must be the final value */
    POWERTABLE_UNASSIGNED,
} lpPowerTableIndex;


/*! Power table types */
typedef enum lp_power_table_set
{
    POWERTABLE_SET_NORMAL,      /*!< Power table set used is for normal use */
    POWERTABLE_SET_TWSPLUS,     /*!< Power table set that is used for TWS+ phones */
} lpPowerTableSet;


/*! Link policy state per ACL connection, stored for us by the connection manager. */
typedef struct
{
    lpPowerTableIndex pt_index;         /*!< Current powertable in use */
    lpPowerTableSet   table_set_used;   /*!< Set of powertables used */
} lpPerConnectionState;

extern void appLinkPolicyInit(void);

/*! @brief Update the link supervision timeout.
    @param bd_addr The Bluetooth address of the peer device.
*/
extern void appLinkPolicyUpdateLinkSupervisionTimeout(const bdaddr *bd_addr);

/*! @brief Update the link power table based on the system state.
    @param bd_addr The Bluetooth address of the peer device.
*/
extern void appLinkPolicyUpdatePowerTable(const bdaddr *bd_addr);
extern void appLinkPolicyAllowRoleSwitch(const bdaddr *bd_addr);
extern void appLinkPolicyAllowRoleSwitchForSink(Sink sink);
extern void appLinkPolicyPreventRoleSwitch(const bdaddr *bd_addr);
extern void appLinkPolicyPreventRoleSwitchForSink(Sink sink);
extern void appLinkPolicyUpdateRoleFromSink(Sink sink);

/*! Handler for connection library messages not sent directly

    This function is called to handle any connection library messages sent to
    the application that the link policy module is interested in. If a message 
    is processed then the function returns TRUE.

    \note Some connection library messages can be sent directly as the 
        request is able to specify a destination for the response.

    \param  id              Identifier of the connection library message 
    \param  message         The message content (if any)
    \param  already_handled Indication whether this message has been processed by
                            another module. The handler may choose to ignore certain
                            messages if they have already been handled.

    \returns TRUE if the message has been processed, otherwise returns the
        value in already_handled
 */
extern bool appLinkPolicyHandleConnectionLibraryMessages(MessageId id,Message message, bool already_handled);


#endif /* __AV_HEADSET_LINK_POLICY_H */
