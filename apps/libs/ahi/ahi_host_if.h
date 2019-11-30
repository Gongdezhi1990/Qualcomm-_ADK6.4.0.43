/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_if.h

DESCRIPTION
    AHI Host interface - glues the AHI library internals to the
    AHI protocol. Contains:
    
      Helper functions for sending AHI data packets to the Host.

      Helper functions for converting AHI data packets to internal
      AHI messages.

*/

#ifndef AHI_HOST_IF_H_
#define AHI_HOST_IF_H_

#include "ahi_msg.h"


/******************************************************************************
NAME
    ahiHostIfProcessData

DESCRIPTION
    Process a AHI data packet, e.g. convert it to an internal AHI message
    and send it to the AHI library task.

RETURNS
    n/a
*/
void ahiHostIfProcessData(Task transport_task, const uint8 *data);

void ahiSendConnectCfm(Task transport_task, uint16 status);
void ahiSendDisconnectCfm(Task transport_task, uint16 status);

/******************************************************************************
NAME
    ahiSendVersionCfm

DESCRIPTION
    Build and send an AHI_VERSION_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendVersionCfm(Task transport_task, AHI_VERSION_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendConfigGetCfm

DESCRIPTION
    Build and send an AHI_CONFIG_GET_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendConfigGetCfm(Task transport_task, AHI_CONFIG_GET_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendConfigSetCfm

DESCRIPTION
    Build and send an AHI_CONFIG_SET_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendConfigSetCfm(Task transport_task, AHI_CONFIG_SET_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendConfigFileGetCfm

DESCRIPTION
    Build and send an AHI_CONFIG_FILE_GET_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendConfigFileGetCfm(Task transport_task, AHI_CONFIG_FILE_GET_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendConfigFileGetDataInd

DESCRIPTION
    Build and send an AHI_CONFIG_FILE_GET_DATA_IND packet to a Host.

RETURNS
    n/a
*/
void ahiSendConfigFileGetDataInd(Task transport_task, AHI_CONFIG_FILE_GET_DATA_IND_T *ind);

/******************************************************************************
NAME
    ahiSendModeGetCfm

DESCRIPTION
    Build and send an AHI_MODE_GET_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendModeGetCfm(Task transport_task, AHI_MODE_GET_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendModeSetCfm

DESCRIPTION
    Build and send an AHI_MODE_SET_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendModeSetCfm(Task transport_task, AHI_MODE_SET_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendEventReportInd

DESCRIPTION
    Build and send an AHI_EVENT_REPORT_IND packet to a Host.

RETURNS
    n/a
*/
void ahiSendEventReportInd(AHI_EVENT_REPORT_IND_T *ind);


/******************************************************************************
NAME
    ahiSendStateMachineStateReportInd

DESCRIPTION
    Build and send an AHI_STATE_MACHINE_STATE_REPORT_IND packet to a Host.

RETURNS
    n/a
*/
void ahiSendStateMachineStateReportInd(AHI_STATE_MACHINE_STATE_REPORT_IND_T *ind);

/******************************************************************************
NAME
    ahiSendMtuReportInd

DESCRIPTION
    Build and send an AHI_REPORT_MTU_IND_T packet to a Host.

RETURNS
    n/a
*/
void ahiSendMtuReportInd(AHI_REPORT_MTU_IND_T *ind);

/******************************************************************************
NAME
    ahiSendConnIntReportInd

DESCRIPTION
    Build and send an AHI_REPORT_CONN_INT_IND_T packet to a Host.

RETURNS
    n/a
*/
void ahiSendConnIntReportInd(AHI_REPORT_CONN_INT_IND_T *ind);

/******************************************************************************
NAME
    ahiSendConfigFileSignatureCfm

DESCRIPTION
    Build and send an AHI_CONFIG_FILE_SIGNATURE_CFM packet to a Host.

    Currently the signature is always assumed to be a MD5 128bit hash
    in ASCII hex format (i.e. a 32bytes sequence of ASCII values).

RETURNS
    n/a
*/
void ahiSendConfigFileSignatureCfm(Task transport_task, AHI_CONFIG_FILE_SIGNATURE_CFM_T *cfm);

void ahiSendDisableTransportCfm(Task transport_task, AHI_DISABLE_TRANSPORT_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendProductIdGetCfm

DESCRIPTION
    Build and send an AHI_PRODUCT_ID_GET_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendProductIdGetCfm(Task transport_task, AHI_PRODUCT_ID_GET_CFM_T *cfm);

/******************************************************************************
NAME
    ahiSendAppBuildIdGetCfm

DESCRIPTION
    Build and send an AHI_APP_BUILD_ID_GET_CFM packet to a Host.

RETURNS
    n/a
*/
void ahiSendAppBuildIdGetCfm(Task transport_task, AHI_APP_BUILD_ID_GET_CFM_T *cfm);


/******************************************************************************
NAME
    ahiSendDelayedSendData

DESCRIPTION
    Attempt to send a AHI data packet that was delayed because the host
    transport queue was full.
*/
void ahiSendDelayedSendData(AHI_INTERNAL_DELAYED_SEND_DATA_T *msg);



/******************************************************************************
NAME
    ahiCalculateModeSetStatus

DESCRIPTION
    Calculate the status to return to the Host after it requested an 
    app mode change.
*/
uint16 ahiCalculateModeSetStatus(ahi_application_mode_t app_mode, ahi_application_mode_t requested_mode, bool need_reboot);

/******************************************************************************
NAME
    ahiConvertApiStatusToMessageStatus

DESCRIPTION
    Convert a ahi_status_t to an equivalent ahi_message_status_t.
*/
uint16 ahiConvertApiStatusToMessageStatus(ahi_status_t status);

#endif /* AHI_HOST_IF_H_ */

