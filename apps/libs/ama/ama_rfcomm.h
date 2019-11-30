/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_rfcomm.h

DESCRIPTION
    Ama RFCOMM functions
*/

#ifndef __AMA_RFCOMM_H_
#define __AMA_RFCOMM_H_

#include "bdaddr.h"
#include "transport_manager.h"


bool AmaRFCommSendData(uint8* data, uint16 length);
bool AmaRfCommInit(Task task,  bdaddr *bd_addr);
bdaddr* amaGetRfCommPeerAddress(void);
void amaRegisterRfCommSdp(uint16 trans_link_id);
void amaRfcommLinkCreatedCfm(TRANSPORT_MGR_LINK_CREATED_CFM_T *cfm);
void amaRfcommLinkDisconnectedCfm(TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *cfm);



#endif /* __AMA_RFCOMM_H_ */
