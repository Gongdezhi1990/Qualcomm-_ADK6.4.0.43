/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_csb_handler.h

DESCRIPTION

NOTES

*/

#ifndef DISABLE_CSB

#include "connection.h"
#include "connection_private.h"

#include <vm.h>

void connectionHandleDmHciSetReservedLtAddrCfm(const DM_HCI_SET_RESERVED_LT_ADDR_CFM_T *cfm);
void connectionHandleDmHciDeleteReservedLtAddrCfm(const DM_HCI_DELETE_RESERVED_LT_ADDR_CFM_T *cfm);
void connectionHandleDmHciSetCsbCfm(const DM_HCI_SET_CSB_CFM_T *cfm);
void connectionHandleDmHciWriteSynchronizationTrainParamsCfm(const DM_HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_CFM_T *cfm);
void connectionHandleDmHciStartSynchronizationTrainCfm(const DM_HCI_START_SYNCHRONIZATION_TRAIN_CFM_T *cfm);
void connectionHandleDmHciReceiveSynchronizationTrainCfm(const DM_HCI_RECEIVE_SYNCHRONIZATION_TRAIN_CFM_T *cfm);
void connectionHandleDmHciSetCsbReceiveCfm(const DM_HCI_SET_CSB_RECEIVE_CFM_T *cfm);
void connectionHandleDmHciCsbTimeoutInd(const DM_HCI_CSB_TIMEOUT_IND_T *ind);
void connectionHandleDmHciCsbAfhMapAvailableInd(const DM_HCI_CSB_AFH_MAP_AVAILABLE_IND_T *ind);
void connectionHandleDmHciCsbChannelMapChangeInd(DM_HCI_CSB_CHANNEL_MAP_CHANGE_IND_T *ind);

#endif /* DISABLE_CSB */
