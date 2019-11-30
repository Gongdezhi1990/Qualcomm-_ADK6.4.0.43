/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_connect.h
DESCRIPTION
    Header file for the internal connection message handler functions.

*/
#ifndef __IAP2_CONNECT_H__
#define __IAP2_CONNECT_H__

#include "iap2_private.h"

#define IAP2_CONNECT_RETRY_DELAY    (D_SEC(1))

void iap2HandleClSdpServiceSearchAttributeCfm(CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);
void iap2HandleClRfcommConnectInd(CL_RFCOMM_CONNECT_IND_T *ind);
void iap2HandleInternalConnectTimeout(iap2_link *link);
void iap2HandleInternalConnectSdpSearchReq(iap2_link *link);
void iap2HandleClRfcommClientConnectCfm(iap2_link *link, const CL_RFCOMM_CLIENT_CONNECT_CFM_T *cfm);
void iap2HandleClRfcommServerConnectCfm(iap2_link *link, const CL_RFCOMM_SERVER_CONNECT_CFM_T *cfm);
void iap2HandleClRfcommDisconnectInd(iap2_link *link, const CL_RFCOMM_DISCONNECT_IND_T *ind);
void iap2HandleClRfcommDisconnectCfm(iap2_link *link, const CL_RFCOMM_DISCONNECT_CFM_T *cfm);

#endif /* __IAP2_CONNECT_H__ */
