/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_handset_signalling.c
\brief	    Implementation of module providing signalling to handset.
*/

#include "av_headset.h"
#include "av_headset_role.h"
#include "av_headset_handset_signalling.h"
#include "av_headset_phy_state.h"
#include "av_headset_hfp.h"
#include "av_headset_log.h"
#include "av_headset_kymera.h"

#include <hfp.h>
#include <panic.h>

/******************************************************************************
 * General Definitions
 ******************************************************************************/

/*! Macro to make a message based on type. */
#define MAKE_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
/*! Macro to make a variable length message based on type. */
#define MAKE_MESSAGE_VAR(VAR, TYPE) TYPE##_T *VAR = PanicUnlessNew(TYPE##_T);

/*!@{ \name Definitions for the AVRCP msg for earbud state
    This is a vendor dependent command to inform handset of change of earbud state. */
#define AVRCP_TWS_EARBUD_STATE          0x30
#define AVRCP_TWS_EARBUD_STATE_SIZE     1
#define AVRCP_TWS_EARBUD_STATE_OFFSET   0
/*!@} */

/*!@{ \name Definitions for the AVRCP msg for earbud role
    This is a vendor dependent command to inform handset of earbud role. */
#define AVRCP_TWS_EARBUD_ROLE           0x31
#define AVRCP_TWS_EARBUD_ROLE_SIZE      1
#define AVRCP_TWS_EARBUD_ROLE_OFFSET    0
/*!@} */

/*!@{ \name Deinitions used to populate AVRCP msg for replace handset
 This is a vendor dependent command to inform handset replacement earbud. */
#define AVRCP_TWS_REPLACE_EARBUD                    0x32
#define AVRCP_TWS_REPLACE_EARBUD_SIZE               7
#define AVRCP_TWS_REPLACE_EARBUD_ADDR_TYPE_OFFSET   0
#define AVRCP_TWS_REPLACE_EARBUD_ADDR_OFFSET        1
#define AVRCP_TWS_REPLACE_EARBUD_ADDR_TYPE_BREDR    0
/*!@} */

/*!@{ \name TWS+ Unsolicited result codes */
char at_cmd_mic_quality_req[] = "\r\n+%QMQ\r\n";
char at_cmd_charging_state_req[] = "\r\n+%QBC\r\n";
char at_cmd_role_req[] = "\r\n+%QER\r\n";
char at_cmd_status_req[] = "\r\n+%QES\r\n";
char at_cmd_mic_delay_req[] = "\r\n+%QMD\r\n";
char at_cmd_dsp_status_req[] = "\r\n+%QDSP:";
/*@} */

/*! DSP Noise Reduction feature definition for TWS custom AT unsolicited result code */
#define AT_TWS_DSP_FEATURE_NR       (1<<0)
/*! DSP Echo Cancellation feature definition for TWS custom AT unsolicited result code */
#define AT_TWS_DSP_FEATURE_EC       (1<<1)

/*! Enumeration of types of TWS+ AT Commands the earbud understands. */
typedef enum
{
    AT_TYPE_UNKNOWN,
    AT_TYPE_MIC_QUALITY_REQ,
    AT_TYPE_CHARGING_STATE_REQ,
    AT_TYPE_ROLE_REQ,
    AT_TYPE_STATUS_REQ,
    AT_TYPE_MIC_DELAY_REQ,
    AT_TYPE_DSP_STATUS_REQ
} handsetSigAtType;

/*! Earbud states, as reported in Earbud State messages to handset. */
typedef enum
{
    /*! The Earbud is 'off', sent as the earbud is going to dormant. */
    EARBUD_STATE_OFF,
    /*! The Earbud is in the case. */
    EARBUD_STATE_IN_CASE,
    /*! The Earbud is out of the ear, may be in motion or at rest. */
    EARBUD_STATE_OUT_OF_EAR,
    /*! The Earbud is in the ear. */
    EARBUD_STATE_IN_EAR
} handsetSigEarbudState;

/*! \brief Conversion utility between internal physical state and external handset state.

    \param[in] state Physical state to convert.

    \return handsetSigEarbudState Handset signalling specific physical state type.
 */
static handsetSigEarbudState appHandsetSigPhyStateToHandsetState(phyState state)
{
    /* convert physical state to handset signalling state. */
    switch (state)
    {
        case PHY_STATE_IN_CASE:
            return EARBUD_STATE_IN_CASE;
        case PHY_STATE_OUT_OF_EAR:
            /* FALL-THRU we don't distinguish motion or at rest in message
             * to handset, just that earbud is out of the ear. */
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            return EARBUD_STATE_OUT_OF_EAR;
        case PHY_STATE_IN_EAR:
            return EARBUD_STATE_IN_EAR;
        default:
            DEBUG_LOGF("appHandsetSigPhyStateToHandsetState unknown physical state %u", state);
            Panic();
            break;
    }

    /* keep compiler happy by returning something, we won't hit this as we'll have
     * panicked in the default case already */
    return EARBUD_STATE_IN_EAR;
}

/*! @brief Parse incoming AT string and return type of AT command. */
static handsetSigAtType appHandsetSigParseAtCmd(uint16 size_data, uint8* data)
{
    UNUSED(size_data);

    if (!strncmp((char*)data, at_cmd_mic_quality_req, sizeof(at_cmd_mic_quality_req)-1))
        return AT_TYPE_MIC_QUALITY_REQ;
    if (!strncmp((char*)data, at_cmd_charging_state_req, sizeof(at_cmd_charging_state_req)-1))
        return AT_TYPE_CHARGING_STATE_REQ;
    if (!strncmp((char*)data, at_cmd_role_req, sizeof(at_cmd_role_req)-1))
        return AT_TYPE_ROLE_REQ;
    if (!strncmp((char*)data, at_cmd_status_req, sizeof(at_cmd_status_req)-1))
        return AT_TYPE_STATUS_REQ;
    if (!strncmp((char*)data, at_cmd_mic_delay_req, sizeof(at_cmd_mic_delay_req)-1))
        return AT_TYPE_MIC_DELAY_REQ;
    if (!strncmp((char*)data, at_cmd_dsp_status_req, sizeof(at_cmd_dsp_status_req)-1))
        return AT_TYPE_DSP_STATUS_REQ;

    return AT_TYPE_UNKNOWN;
}

/*! @brief Send HANDSET_SIG_REPLACE_EARBUD_CFM to client with result. */
static void appHandsetSigMsgReplaceEarbudCfm(Task task, handsetSigStatus status)
{
    MAKE_MESSAGE(HANDSET_SIG_REPLACE_EARBUD_CFM);
    message->status = status;
    MessageSend(task, HANDSET_SIG_REPLACE_EARBUD_CFM, message);
}

/*! @brief Get AVRCP lock for sending conditional internal message to request TX to handset. */
static uint16* appHandsetSigGetAvrcpLock(void)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();
  
    /* either waiting for AVRCP to be connected, or for an existing operation
     * to complete */
    if (handset_sig->waiting_avrcp)
        return &handset_sig->waiting_avrcp;
    else
        return &handset_sig->current_avrcp_op;
}

/*! @brief Get HFP lock for sending conditional internal message to request TX to handset. */
static uint16* appHandsetSigGetHfpSlcLock(void)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();
  
    /* either waiting for HFP SLC to be connected, or for an existing operation
     * to complete */
    if (handset_sig->waiting_hfp_slc)
        return &handset_sig->waiting_hfp_slc;
    else
        return &handset_sig->current_slc_op;
}

/*! @brief Cancel any pending operations that are sat on our message queue.
 */
static void appHandsetSigCancelAvrcpPendingOperations(void)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();

    /* just clear out these messages, clients don't get confirmations for these
     * notification type messages. */
    MessageCancelAll(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_AVRCP_REQ);
    MessageCancelAll(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_ROLE_AVRCP_REQ);

    /* we need to send failure confirmation for replace earbud requests */
    if (MessageCancelAll(&handset_sig->task, HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ))
    {
        appHandsetSigMsgReplaceEarbudCfm(handset_sig->replace_earbud_client_task,
                                         handsetSigStatusReplaceEarbudFail);
        handset_sig->replace_earbud_client_task = NULL;
    }
}

/*! @brief Cancel any already in progress operations that are waiting for responses from handset.
 */
static void appHandsetSigCancelAvrcpInProgressOperation(void)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();

    switch (handset_sig->current_avrcp_op)
    {
        case AVRCP_TWS_REPLACE_EARBUD:
            appHandsetSigMsgReplaceEarbudCfm(handset_sig->replace_earbud_client_task,
                                             handsetSigStatusReplaceEarbudFail);
            break;
        default:
            break;
    }
    handset_sig->current_avrcp_op = 0;
}

/*! @brief Handle AV confirmation of AVRCP connect request.
 */
static void appHandsetSigHandleAvAvrcpConnectCfm(handsetSigTaskData* handset_sig,
                                                 AV_AVRCP_CONNECT_CFM_T *cfm)
{
    DEBUG_LOGF("appHandsetSigHandleAvAvrcpConnectCfm %d", cfm->status);

    if (cfm->status == avrcp_success)
    {
        /* clearing this will unlock any conditional messages waiting to be sent */
        handset_sig->waiting_avrcp = 0;

        /* remember av instance for use in sending AVRCP messages */
        handset_sig->av_inst = cfm->av_instance;
        
        /* register handset signalling task with AV to receiver vendor unique AVRCP
         * messages */
        appAvrcpVendorPassthroughRegister(handset_sig->av_inst, &handset_sig->task);
    }
    else
    {
        appHandsetSigCancelAvrcpPendingOperations();
    }
}

/*! @brief Accept incoming AVRCP if it is from a known handset.
 */
static void appHandsetSigHandleAvAvrcpConnectInd(handsetSigTaskData* handset_sig,
                                                 AV_AVRCP_CONNECT_IND_T *ind)
{
    /* Accept connection if it's from a TWS+ Handset */
    const bool accept = ((appDeviceIsHandset(&ind->bd_addr)) &&
                         (appDeviceTwsVersion(&ind->bd_addr) == DEVICE_TWS_VERSION));
    if (accept)
        DEBUG_LOG("appHandsetSigHandleAvAvrcpConnectInd, accepted");
    else
        DEBUG_LOG("appHandsetSigHandleAvAvrcpConnectInd, rejected");

    /* Send response, either passive acception or rejection */
    appAvAvrcpConnectResponse(&handset_sig->task, &handset_sig->task, &ind->bd_addr, ind->connection_id,
                              ind->signal_id, accept ? AV_AVRCP_ACCEPT_PASSIVE : AV_AVRCP_REJECT);
}

/*! @brief Handle disconnection of the AVRCP link to the handset.
 */
static void appHandsetSigHandleAvAvrcpDisconnectInd(handsetSigTaskData* handset_sig)
{
    DEBUG_LOG("appHandsetSigHandleAvAvrcpDisconnectInd");

    /* reset state that indicates we need AVRCP */
    handset_sig->av_inst = NULL;
    handset_sig->waiting_avrcp = 1;

    /* TODO move this into HFP diconnect handler */
//    handset_sig->waiting_hfp_slc = 1;

    /* deal with any pending operations we may have queued up or currently
     * in progress */
    appHandsetSigCancelAvrcpPendingOperations();
    appHandsetSigCancelAvrcpInProgressOperation();
}

/*! @brief Handle unsolicited messages received from handset.
 
    Not expecting any...
 */
static void appHandsetSigHandleAvAvrcpVendorPassthroughInd(AV_AVRCP_VENDOR_PASSTHROUGH_IND_T *ind)
{
    UNUSED(ind);
    DEBUG_LOGF("appHandsetSigHandleAvAvrcpVendorPassthroughInd unexpected %d", ind->opid);
    Panic();
}

/*! @brief Handle confirmation of AVRCP messages we've sent to handset.
 */
static void appHandsetSigHandleAvAvrcpVendorPassthroughConfirm(handsetSigTaskData* handset_sig,
                                                               AV_AVRCP_VENDOR_PASSTHROUGH_CFM_T *cfm)
{
    DEBUG_LOGF("appHandsetSigHandleAvAvrcpVendorPassthroughConfirm %d opid:%x",
               cfm->status, cfm->opid);

    switch (cfm->opid)
    {
        case AVRCP_TWS_REPLACE_EARBUD:
            appHandsetSigMsgReplaceEarbudCfm(handset_sig->replace_earbud_client_task,
                                             cfm->status == avrcp_success ?
                                             handsetSigStatusSuccess : handsetSigStatusReplaceEarbudFail);
            /* permit another replace earbud request */
            handset_sig->replace_earbud_client_task = NULL;
            break;

        default:
            DEBUG_LOGF("appHandsetSigHandleAvAvrcpVendorPassthroughConfirm unknown opid:%x", cfm->opid);
    }

    /* clear this to permit further pending AVRCP messages in the queue */
    handset_sig->current_avrcp_op = 0;
}

/*! @brief Handle indication of state of HFP SLC.
 
    Clear the waiting flag which will enable transmission of any queued messages.
*/
static void appHandsetSigHandleAppHfpSlcStatusInd(handsetSigTaskData* handset_sig,
                                                  APP_HFP_SLC_STATUS_IND_T *ind)
{
    /* Accept connection if it's from a TWS+ Handset */
    const bool accept = ((appDeviceIsHandset(&ind->bd_addr)) &&
                         (appDeviceTwsVersion(&ind->bd_addr) == DEVICE_TWS_VERSION));

    if (!accept)
    {
        DEBUG_LOG("appHandsetSigHandleAppHfpSlcStatusInd SLC, ignore as not TWS+");
    }
    else
    {
        if (ind->slc_connected)
            DEBUG_LOG("appHandsetSigHandleAppHfpSlcStatusInd SLC connected");
        else
            DEBUG_LOG("appHandsetSigHandleAppHfpSlcStatusInd SLC disconnected");

        if (ind->slc_connected)
        {
            handset_sig->hfp_priority = ind->priority;
            handset_sig->waiting_hfp_slc = 0;
        }
        else
        {
            handset_sig->hfp_priority = hfp_invalid_link;
            handset_sig->waiting_hfp_slc = 1;
            handset_sig->current_slc_op = 0;
        }
    }
}

/*! @brief Handle confirmation of an AT command we sent.
 */
static void appHandsetSigHandleAppHfpAtCmdCfm(handsetSigTaskData* handset_sig,
                                              APP_HFP_AT_CMD_CFM_T* cfm)
{
    UNUSED(cfm);
    DEBUG_LOGF("appHandsetSigHandleAppHfpAtCmdCfm %d", cfm->status);

    /* clear current op to release next pending message */
    handset_sig->current_slc_op = 0;
}

/*! @brief Handle incoming AT command from handset.
 */
static void appHandsetSigHandleAppHfpAtCmdInd(handsetSigTaskData* handset_sig,
                                              APP_HFP_AT_CMD_IND_T* ind)
{
    handsetSigAtType type = AT_TYPE_UNKNOWN;

    DEBUG_LOGF("appHandsetSigHandleAppHfpAtCmdInd len:%d ", ind->size_data);
#if 0
    for (int i=0; i < ind->size_data; i++)
        printf("#%c#%x#", ind->data[i], ind->data[i]);
    printf("");
#endif
    
    type = appHandsetSigParseAtCmd(ind->size_data, ind->data);

    switch (type)
    {
        case AT_TYPE_MIC_QUALITY_REQ:
            DEBUG_LOG("appHandsetSigParseAtCmd AT_TYPE_MIC_QUALITY_REQ");
            appHandsetSigSendMicQualityReq(appKymeraScoVoiceQuality());
            break;
        case AT_TYPE_CHARGING_STATE_REQ:
            DEBUG_LOG("appHandsetSigParseAtCmd AT_TYPE_CHARGING_STATE_REQ");
            appHandsetSigSendChargingStateReq(handset_sig->charger_attached ?
                                                   EARBUD_CHARGING_STATE_CHARGING : 
                                                   EARBUD_CHARGING_STATE_DISCHARGING, 
                                              appBatteryGetPercent());
            break;
        case AT_TYPE_ROLE_REQ:
            DEBUG_LOG("appHandsetSigParseAtCmd AT_TYPE_ROLE_REQ");
            appHandsetSigSendEarbudRoleReq(appConfigIsLeft() ? EARBUD_ROLE_LEFT : EARBUD_ROLE_RIGHT);
            break;
        case AT_TYPE_STATUS_REQ:
            DEBUG_LOG("appHandsetSigParseAtCmd AT_TYPE_STATUS_REQ");
            appHandsetSigSendEarbudStateReq(handset_sig->current_phy_state);
            break;
        case AT_TYPE_MIC_DELAY_REQ:
            DEBUG_LOG("appHandsetSigParseAtCmd AT_TYPE_MIC_DELAY_REQ");
            appHandsetSigSendMicDelayReq(appConfigMicPathDelayVariationUs());
            break;
        case AT_TYPE_DSP_STATUS_REQ:
            DEBUG_LOG("appHandsetSigParseAtCmd AT_TYPE_DSP_STATUS_REQ");
            /*! \todo we could get the parameter from the request code and try
             * to disable the DSP feature. For now just reply that NR and EC
             * are still enabled. */
            appHandsetSigSendDspStatusReq(AT_TWS_DSP_FEATURE_NR | AT_TWS_DSP_FEATURE_EC);
            break;
        
        case AT_TYPE_UNKNOWN:
        default:
            DEBUG_LOG("appHandsetSigParseAtCmd Unknown AT command from handset");
            break;
    }
}

/*! \brief Record latest physical state when notified by phy_state module. */
static void appHandsetSigHandlePhyStateChangedInd(handsetSigTaskData* handset_sig,
                                                  PHY_STATE_CHANGED_IND_T* ind)
{
    DEBUG_LOGF("appHandsetSigHandlePhyStateChangedInd new state %u", ind->new_state);
    handset_sig->current_phy_state = ind->new_state;
}

/*! @brief Build and transmit a AVRCP_TWS_EARBUD_STATE message to handset.
 */
static void appHandsetSigHandleInternalEarbudStateReq(handsetSigTaskData* handset_sig, 
                                                      HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_REQ_T *req)
{
    DEBUG_LOGF("appHandsetSigHandleInternalEarbudStateReq type %d", req->transport);

    if (req->transport == OPTYPE_AVRCP)
    {
        uint8 message[AVRCP_TWS_EARBUD_STATE_SIZE];
        int index = AVRCP_TWS_EARBUD_STATE_OFFSET;

        handset_sig->current_avrcp_op = AVRCP_TWS_EARBUD_STATE;
    
        /* convert from earbud phy_state to handset signalling form used between
         * earbud and handset for signalling */
        message[index] = appHandsetSigPhyStateToHandsetState(req->state);

        appAvrcpVendorPassthroughRequest(handset_sig->av_inst, AVRCP_TWS_EARBUD_STATE,
                                         AVRCP_TWS_EARBUD_STATE_SIZE, message);
    }
    else
    {
        char cmd[10];
        snprintf(cmd, sizeof(cmd), "AT%%QES=%1d\r", appHandsetSigPhyStateToHandsetState(req->state));
        handset_sig->current_slc_op = AVRCP_TWS_EARBUD_STATE;
        appHfpSendAtCmdReq(handset_sig->hfp_priority, cmd);
    }
}

/*! @brief Build and transmit a AVRCP_TWS_EARBUD_ROLE message to handset.
 */
static void appHandsetSigHandleInternalEarbudRoleReq(handsetSigTaskData* handset_sig,
                                                     HANDSET_SIG_INTERNAL_EARBUD_ROLE_REQ_T *req)
{
    DEBUG_LOGF("appHandsetSigHandleInternalEarbudRoleReq type %d", req->transport);

    if (req->transport == OPTYPE_AVRCP)
    {
        uint8 message[AVRCP_TWS_EARBUD_ROLE_SIZE];
        int index = AVRCP_TWS_EARBUD_ROLE_OFFSET;

        handset_sig->current_avrcp_op = AVRCP_TWS_EARBUD_ROLE;

        message[index] = req->role;

        appAvrcpVendorPassthroughRequest(handset_sig->av_inst, AVRCP_TWS_EARBUD_ROLE,
                                         AVRCP_TWS_EARBUD_ROLE_SIZE, message);
    }
    else
    {
        char cmd[10];
        snprintf(cmd, sizeof(cmd), "AT%%QER=%1d\r", req->role);
        handset_sig->current_slc_op = AVRCP_TWS_EARBUD_ROLE;
        appHfpSendAtCmdReq(handset_sig->hfp_priority, cmd);
    }
}

/*! @brief Build and transmit a AVRCP_TWS_REPLACE_EARBUD message to handset.
 */
static void appHandsetSigHandleInternalReplaceEarbudReq(handsetSigTaskData* handset_sig,
                                                        HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ_T *req)
{
    DEBUG_LOG("appHandsetSigHandleInternalReplaceEarbudReq");

    uint8 message[AVRCP_TWS_REPLACE_EARBUD_SIZE];
    int index = AVRCP_TWS_REPLACE_EARBUD_ADDR_TYPE_OFFSET;

    handset_sig->current_avrcp_op = AVRCP_TWS_REPLACE_EARBUD;

    message[index++] = AVRCP_TWS_REPLACE_EARBUD_ADDR_TYPE_BREDR;
    index = AVRCP_TWS_REPLACE_EARBUD_ADDR_OFFSET;
    message[index++] =  req->new_earbud.lap & 0xFF;
    message[index++] = (req->new_earbud.lap >> 8) & 0xFF;
    message[index++] = (req->new_earbud.lap >> 16) & 0xFF;
    message[index++] =  req->new_earbud.uap;
    message[index++] =  req->new_earbud.nap & 0xFF;
    message[index++] = (req->new_earbud.nap >> 8) & 0xFF;

    appAvrcpVendorPassthroughRequest(handset_sig->av_inst, AVRCP_TWS_REPLACE_EARBUD,
            AVRCP_TWS_REPLACE_EARBUD_SIZE, message);
}

/*! @brief Build and transmit an AT Mic Quality message to the handset.
 */
static void appHandsetSigHandleInternalAtMicQualityReq(handsetSigTaskData* handset_sig,
                                    HANDSET_SIG_INTERNAL_AT_MIC_QUALITY_REQ_T* req)
{
    char cmd[11];
    snprintf(cmd, sizeof(cmd), "AT%%QMQ=%d\r", req->mic_quality);
    handset_sig->current_slc_op = AT_TYPE_MIC_QUALITY_REQ;
    appHfpSendAtCmdReq(handset_sig->hfp_priority, cmd);
}

/*! @brief Build and transmit an AT Mic Delay message to the handset.
 */
static void appHandsetSigHandleInternalAtMicDelayReq(handsetSigTaskData* handset_sig,
                                    HANDSET_SIG_INTERNAL_AT_MIC_DELAY_REQ_T* req)
{
    char cmd[14];
    snprintf(cmd, sizeof(cmd), "AT%%QMD=%d\r", req->mic_delay_us);
    handset_sig->current_slc_op = AT_TYPE_MIC_DELAY_REQ;
    appHfpSendAtCmdReq(handset_sig->hfp_priority, cmd);
}

/*! @brief Build and transmit an AT Charging State message to the handset.
 */
static void appHandsetSigHandleInternalAtChargingStateReq(handsetSigTaskData* handset_sig,
                                    HANDSET_SIG_INTERNAL_AT_CHARGING_STATE_REQ_T* req)
{
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "AT%%QBC=%d,%d\r", req->charging_state, req->battery_level_percent);
    handset_sig->current_slc_op = AT_TYPE_CHARGING_STATE_REQ;
    appHfpSendAtCmdReq(handset_sig->hfp_priority, cmd);
}

/*! @brief Build and transmit an AT DSP Status message to the handset.
 */
static void appHandsetSigHandleInternalAtDspStatusReq(handsetSigTaskData* handset_sig,
                                    HANDSET_SIG_INTERNAL_AT_DSP_STATUS_REQ_T* req)
{
    char cmd[11];
    snprintf(cmd, sizeof(cmd), "AT%%QDSP=%1d\r", req->dsp_status);
    handset_sig->current_slc_op = AT_TYPE_DSP_STATUS_REQ;
    appHfpSendAtCmdReq(handset_sig->hfp_priority, cmd);
}

/*! @brief Handset signalling task message handler.
 */
static void appHandsetSigHandleMessage(Task task, MessageId id, Message message)
{
    handsetSigTaskData* handset_sig = (handsetSigTaskData*)task;

    switch(id)
    {
        /* AV module handset connection related messages */
        case AV_AVRCP_CONNECT_CFM:
            appHandsetSigHandleAvAvrcpConnectCfm(handset_sig,
                                        (AV_AVRCP_CONNECT_CFM_T *)message);
            break;
        case AV_AVRCP_CONNECT_IND:
            appHandsetSigHandleAvAvrcpConnectInd(handset_sig,
                                        (AV_AVRCP_CONNECT_IND_T *)message);
            break;
        case AV_AVRCP_DISCONNECT_IND:
            appHandsetSigHandleAvAvrcpDisconnectInd(handset_sig);
            break;

        /* AV module handset comms related messages */
        case AV_AVRCP_VENDOR_PASSTHROUGH_IND:
            appHandsetSigHandleAvAvrcpVendorPassthroughInd(
                                (AV_AVRCP_VENDOR_PASSTHROUGH_IND_T*)message);
            break;
        case AV_AVRCP_VENDOR_PASSTHROUGH_CFM:
            appHandsetSigHandleAvAvrcpVendorPassthroughConfirm(handset_sig,
                                (AV_AVRCP_VENDOR_PASSTHROUGH_CFM_T*)message);
            break;            

        /* Ignore volume messages */
        case AV_AVRCP_SET_VOLUME_IND:
        case AV_AVRCP_VOLUME_CHANGED_IND:
            break;
            
        /* HFP module handset comms related messages */
        case APP_HFP_SLC_STATUS_IND:
            appHandsetSigHandleAppHfpSlcStatusInd(handset_sig,
                                                  (APP_HFP_SLC_STATUS_IND_T*)message);
            break;
        case APP_HFP_AT_CMD_CFM:
            appHandsetSigHandleAppHfpAtCmdCfm(handset_sig,
                                              (APP_HFP_AT_CMD_CFM_T*)message);
            break;
        case APP_HFP_AT_CMD_IND:
            appHandsetSigHandleAppHfpAtCmdInd(handset_sig,
                                              (APP_HFP_AT_CMD_IND_T*)message);
            break;

        /* charger state messages */
        case CHARGER_MESSAGE_ATTACHED:
            handset_sig->charger_attached = TRUE;
            break;
        case CHARGER_MESSAGE_DETACHED:
            handset_sig->charger_attached = FALSE;
            break;

        /* physical state messages */
        case PHY_STATE_CHANGED_IND:
            appHandsetSigHandlePhyStateChangedInd(handset_sig,
                                                  (PHY_STATE_CHANGED_IND_T*)message);
            break;

        /* internal messages */
        case HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_AVRCP_REQ:
        case HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_HFP_REQ:
            appHandsetSigHandleInternalEarbudStateReq(handset_sig,
                           (HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_REQ_T*)message);
            break;

        case HANDSET_SIG_INTERNAL_EARBUD_ROLE_AVRCP_REQ:
        case HANDSET_SIG_INTERNAL_EARBUD_ROLE_HFP_REQ:
            appHandsetSigHandleInternalEarbudRoleReq(handset_sig,
                            (HANDSET_SIG_INTERNAL_EARBUD_ROLE_REQ_T*)message);
            break;

        case HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ:
            appHandsetSigHandleInternalReplaceEarbudReq(handset_sig,
                          (HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ_T*)message);
            break;

        case HANDSET_SIG_INTERNAL_AT_MIC_QUALITY_REQ:
            appHandsetSigHandleInternalAtMicQualityReq(handset_sig,
                            (HANDSET_SIG_INTERNAL_AT_MIC_QUALITY_REQ_T*)message);
            break;

        case HANDSET_SIG_INTERNAL_AT_MIC_DELAY_REQ:
            appHandsetSigHandleInternalAtMicDelayReq(handset_sig,
                            (HANDSET_SIG_INTERNAL_AT_MIC_DELAY_REQ_T*)message);
            break;

        case HANDSET_SIG_INTERNAL_AT_CHARGING_STATE_REQ:
            appHandsetSigHandleInternalAtChargingStateReq(handset_sig,
                            (HANDSET_SIG_INTERNAL_AT_CHARGING_STATE_REQ_T*)message);
            break;

        case HANDSET_SIG_INTERNAL_AT_DSP_STATUS_REQ:
            appHandsetSigHandleInternalAtDspStatusReq(handset_sig,
                            (HANDSET_SIG_INTERNAL_AT_DSP_STATUS_REQ_T*)message);
            break;

        default:
            break;
    }
}

/*! @brief Initialise handset signalling.
 */
void appHandsetSigInit(void)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();
    memset(handset_sig, 0, sizeof(*handset_sig));
    handset_sig->task.handler = appHandsetSigHandleMessage;
    handset_sig->waiting_avrcp = 1;
    handset_sig->waiting_hfp_slc = 1;
    handset_sig->charger_attached = FALSE;
    handset_sig->current_phy_state = PHY_STATE_UNKNOWN;

    /* tell AV we want notification when an AVRCP connection occurs */
    appAvAvrcpClientRegister(&handset_sig->task, 0);

    /* tell HFP we want notification when an SLC connection occurs,
     * and that handset_sig module handles custom AT commands. */
    appHfpClientRegister(&handset_sig->task);
    appHfpRegisterAtCmdTask(&handset_sig->task);

    /* register for charger status events */
    appChargerClientRegister(&handset_sig->task);

    /* register for physical state change notifications. */
    appPhyStateRegisterClient(&handset_sig->task);
}

/*! @brief Request an AVRCP_TWS_EARBUD_STATE message is sent to the handset. */
/*
    Either we already have an AVRCP and/or HFP link to the handset in which
    case send the messages straight away, or block conditional on them
    being available and send the messages once they are.

    If a new request arrives and we still have messages waiting on the queue
    then the state they describe is now out-of-date, so just delete them and 
    post new request messages.

    Create 2 messages, one that blocks on AVRCP and the other on HFP SLC
    availability. The two are independent and we will send the messages
    whenever the appropriate channel is available.
*/
void appHandsetSigSendEarbudStateReq(phyState state)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();
    MAKE_MESSAGE_VAR(avrcp_message, HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_REQ);
    MAKE_MESSAGE_VAR(hfp_message, HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_REQ);

    DEBUG_LOGF("appHandsetSigSendEarbudStateReq state %u", state);

    /* clear any current AVRCP_TWS_EARBUD_STATE messages in the queue */
    MessageCancelAll(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_AVRCP_REQ);
    MessageCancelAll(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_HFP_REQ);

    /* post request messages for AVRCP and HFP */
    avrcp_message->transport = OPTYPE_AVRCP;
    avrcp_message->state = state;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_AVRCP_REQ,
            avrcp_message, appHandsetSigGetAvrcpLock());

    hfp_message->transport = OPTYPE_AT;
    hfp_message->state = state;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_HFP_REQ,
            hfp_message, appHandsetSigGetHfpSlcLock());
}

/*! @brief Request an AVRCP_TWS_EARBUD_ROLE message is sent to the handset.
 */
void appHandsetSigSendEarbudRoleReq(earbudRole role)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();
    MAKE_MESSAGE_VAR(avrcp_message, HANDSET_SIG_INTERNAL_EARBUD_ROLE_REQ);
    MAKE_MESSAGE_VAR(hfp_message, HANDSET_SIG_INTERNAL_EARBUD_ROLE_REQ);

    MessageCancelAll(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_ROLE_AVRCP_REQ);
    MessageCancelAll(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_ROLE_HFP_REQ);

    avrcp_message->transport = OPTYPE_AVRCP;
    avrcp_message->role = role;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_ROLE_AVRCP_REQ,
            avrcp_message, appHandsetSigGetAvrcpLock());
    hfp_message->transport = OPTYPE_AT;
    hfp_message->role = role;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_EARBUD_ROLE_HFP_REQ,
            hfp_message, appHandsetSigGetHfpSlcLock());
}

/*! @brief Request an AVRCP_TWS_REPLACE_EARBUD message is sent to the handset.*/
/*
    REPLACE_EARBUD messages only sent over AVRCP.
    We may also have multiple pairied handsets whom need notifying, so can have
    multiple messages of this type in the queue.
*/
handsetSigStatus appHandsetSigSendReplaceEarbudReq(Task task, const bdaddr* new_earbud)
{
    handsetSigTaskData* handset_sig = appGetHandsetSig();

    if (handset_sig->replace_earbud_client_task)
        return handsetSigStatusReplaceEarbudFailInProgress;
    else
    {
        MAKE_MESSAGE_VAR(avrcp_message, HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ);

        handset_sig->replace_earbud_client_task = task;
        avrcp_message->new_earbud = *new_earbud;
        MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ,
                avrcp_message, appHandsetSigGetAvrcpLock());
        return handsetSigStatusSuccess;
    }
}

/*! @brief Handle request to transmit mic quality AT command to handset.
 */
handsetSigStatus appHandsetSigSendMicQualityReq(uint8 mic_quality)
{
    MAKE_MESSAGE(HANDSET_SIG_INTERNAL_AT_MIC_QUALITY_REQ);
    handsetSigTaskData* handset_sig = appGetHandsetSig();
    message->mic_quality = mic_quality;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_AT_MIC_QUALITY_REQ,
                             message, appHandsetSigGetHfpSlcLock());
    return handsetSigStatusSuccess;
}

/*! @brief Handle request to transmit battery charging state AT command to handset.
 */
handsetSigStatus appHandsetSigSendChargingStateReq(handsetSigChargingState charging_state,
                                                   uint8 battery_level_percent)
{
    MAKE_MESSAGE(HANDSET_SIG_INTERNAL_AT_CHARGING_STATE_REQ);
    handsetSigTaskData* handset_sig = appGetHandsetSig();
    message->charging_state = charging_state;
    message->battery_level_percent = battery_level_percent;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_AT_CHARGING_STATE_REQ,
                             message, appHandsetSigGetHfpSlcLock());
    return handsetSigStatusSuccess;
}

/*! @brief Handle request to transmit mic delay AT command to handset.
 */
handsetSigStatus appHandsetSigSendMicDelayReq(uint16 mic_delay)
{
    MAKE_MESSAGE(HANDSET_SIG_INTERNAL_AT_MIC_DELAY_REQ);
    handsetSigTaskData* handset_sig = appGetHandsetSig();
    message->mic_delay_us = mic_delay;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_AT_MIC_DELAY_REQ,
                             message, appHandsetSigGetHfpSlcLock());
    return handsetSigStatusSuccess;
}

/*! @brief Handle request to transmit DSP status AT command to handset.
 */
handsetSigStatus appHandsetSigSendDspStatusReq(uint8 dsp_status)
{
    MAKE_MESSAGE(HANDSET_SIG_INTERNAL_AT_DSP_STATUS_REQ);
    handsetSigTaskData* handset_sig = appGetHandsetSig();
    message->dsp_status = dsp_status;
    MessageSendConditionally(&handset_sig->task, HANDSET_SIG_INTERNAL_AT_DSP_STATUS_REQ,
                             message, appHandsetSigGetHfpSlcLock());
    return handsetSigStatusSuccess;
}


