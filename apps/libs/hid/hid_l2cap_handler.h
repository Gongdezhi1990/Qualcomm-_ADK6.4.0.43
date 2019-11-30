/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_l2cap_handler.h
DESCRIPTION
    Header file for the L2CAP message handler functions.

*/

#ifndef HID_L2CAP_HANDLER_H_
#define HID_L2CAP_HANDLER_H_

#include "hid.h"
#include "hid_private.h"

void hidHandleL2capRegisterCfm(HID_LIB *hidLib, const CL_L2CAP_REGISTER_CFM_T *cfm);
void hidHandleL2capConnectInd(HID_LIB *hidLib, const CL_L2CAP_CONNECT_IND_T *ind);
void hidHandleL2capConnectCfmLocal(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm);
void hidHandleL2capConnectCfmLocalInterrupt(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm);
void hidHandleL2capConnectCfmRemote(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm);
void hidHandleL2capConnectCfmDisconnecting(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm);
void hidHandleL2capConnectCfmDisconnectingFailed(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm);
void hidHandleL2capDisconnectIndConnecting(HID *hid, const CL_L2CAP_DISCONNECT_IND_T *ind);
void hidHandleL2capDisconnectIndConnected(HID *hid, const CL_L2CAP_DISCONNECT_IND_T *ind);
void hidHandleL2capDisconnectIndDisconnecting(HID *hid, const CL_L2CAP_DISCONNECT_IND_T *ind);
void hidHandleL2capDisconnectCfmDisconnecting(HID *hid, const CL_L2CAP_DISCONNECT_CFM_T *cfm);

#endif /* HID_L2CAP_HANDLER_H_ */
