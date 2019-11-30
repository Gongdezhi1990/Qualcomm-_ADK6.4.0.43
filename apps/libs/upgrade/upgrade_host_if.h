/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_host_if.h
    
DESCRIPTION

*/
#ifndef UPGRADE_HOST_IF_H_
#define UPGRADE_HOST_IF_H_

#include <message.h>
#include "upgrade.h"

void UpgradeHostIFClientConnect(Task clientTask);
void UpgradeHostIFClientSendData(uint8 *data, uint16 dataSize);

void UpgradeHostIFTransportConnect(Task transportTask, bool need_data_cfm, bool request_multiple_blocks);
void UpgradeHostIFProcessDataRequest(uint8 *data, uint16 dataSize);
void UpgradeHostIFTransportDisconnect(void);
bool UpgradeHostIFTransportInUse(void);

#endif /* UPGRADE_HOST_IF_H_ */
