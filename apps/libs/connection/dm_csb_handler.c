/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_csb_handler.c

DESCRIPTION

NOTES

*/

#include "connection.h"
#include "connection_private.h"
#include "dm_csb_handler.h"
#include "bdaddr.h"

#include <vm.h>
#include <stream.h>

#ifndef DISABLE_CSB

static Task connectionGetCsbMsgDestTask(void) {
    return connectionGetCsbTask() ? connectionGetCsbTask() : connectionGetAppTask();
}

void connectionHandleDmHciSetReservedLtAddrCfm(const DM_HCI_SET_RESERVED_LT_ADDR_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_CSB_SET_RESERVED_LT_ADDR_CFM);
    message->status = connectionConvertHciStatus(cfm->status);
    message->lt_addr = cfm->lt_addr;
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_SET_RESERVED_LT_ADDR_CFM,
                message);
}
    
void connectionHandleDmHciDeleteReservedLtAddrCfm(const DM_HCI_DELETE_RESERVED_LT_ADDR_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_CSB_DELETE_RESERVED_LT_ADDR_CFM);
    message->status = connectionConvertHciStatus(cfm->status);
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_DELETE_RESERVED_LT_ADDR_CFM,
                message);
}

void connectionHandleDmHciSetCsbCfm(const DM_HCI_SET_CSB_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_CSB_SET_CSB_CFM);
    message->status = connectionConvertHciStatus(cfm->status);
    message->lt_addr = cfm->lt_addr;
    message->interval = cfm->interval;
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_SET_CSB_CFM,
                message);
}

void connectionHandleDmHciWriteSynchronizationTrainParamsCfm(const DM_HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_CSB_WRITE_SYNC_TRAIN_PARAMETERS_CFM);
    message->status = connectionConvertHciStatus(cfm->status);
    message->sync_train_ref_interval = cfm->sync_train_ref_interval;
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_WRITE_SYNC_TRAIN_PARAMETERS_CFM,
                message);
}

void connectionHandleDmHciStartSynchronizationTrainCfm(const DM_HCI_START_SYNCHRONIZATION_TRAIN_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_CSB_START_SYNC_TRAIN_CFM);
    message->status = connectionConvertHciStatus(cfm->status);
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_START_SYNC_TRAIN_CFM,
                message);
}
        
void connectionHandleDmHciReceiveSynchronizationTrainCfm(const DM_HCI_RECEIVE_SYNCHRONIZATION_TRAIN_CFM_T *cfm)
{
    uint16 index = 0;
    MAKE_CL_MESSAGE(CL_CSB_SYNC_TRAIN_RECEIVED_CFM);

    message->status = connectionConvertHciStatus(cfm->status);
    BdaddrConvertBluestackToVm(&message->bd_addr, &cfm->bd_addr);
    message->clock_offset = cfm->clock_offset;
    for (index=0; index < sizeof(cfm->map); index++)
    {
        message->afh_map[index] = cfm->map[index];
    }
    message->lt_addr = cfm->lt_addr;
    message->next_csb_clock = cfm->next_broadcast_instant;
    message->interval = cfm->csb_interval;
    message->service_data = cfm->service_data;
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_SYNC_TRAIN_RECEIVED_CFM,
                message);
}

void connectionHandleDmHciSetCsbReceiveCfm(const DM_HCI_SET_CSB_RECEIVE_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_CSB_RECEIVE_CFM);
    message->status = connectionConvertHciStatus(cfm->status);
    BdaddrConvertBluestackToVm(&message->bd_addr, &cfm->bd_addr);
    message->lt_addr = cfm->lt_addr;
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_RECEIVE_CFM,
                message);
}

void connectionHandleDmHciCsbTimeoutInd(const DM_HCI_CSB_TIMEOUT_IND_T *ind)
{
    bdaddr bd_addr;

    BdaddrConvertBluestackToVm(&bd_addr, &ind->bd_addr);
    if (StreamCsbSource(&bd_addr, ind->lt_addr))
    {
        MAKE_CL_MESSAGE(CL_CSB_RECEIVE_TIMEOUT_IND);

        message->bd_addr = bd_addr;
        message->lt_addr = ind->lt_addr;
        MessageSend(connectionGetCsbMsgDestTask(),
                    CL_CSB_RECEIVE_TIMEOUT_IND,
                    message);
    }
    else if (StreamCsbSink(ind->lt_addr))
    {
        MAKE_CL_MESSAGE(CL_CSB_TRANSMIT_TIMEOUT_IND);

        message->lt_addr = ind->lt_addr;
        MessageSend(connectionGetCsbMsgDestTask(),
                    CL_CSB_RECEIVE_TIMEOUT_IND,
                    message);
    }
    else
        Panic();

}

void connectionHandleDmHciCsbAfhMapAvailableInd(const DM_HCI_CSB_AFH_MAP_AVAILABLE_IND_T *ind)
{
    uint16 index;
    MAKE_CL_MESSAGE(CL_CSB_AFH_MAP_IND);

    message->clock = ind->clock;
    for (index=0; index < sizeof(ind->map); index++)
    {
        message->map[index] = ind->map[index];
    }
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_AFH_MAP_IND,
                message);
}

void connectionHandleDmHciCsbChannelMapChangeInd(DM_HCI_CSB_CHANNEL_MAP_CHANGE_IND_T *ind)
{
    uint16 index;
    MAKE_CL_MESSAGE(CL_CSB_CHANNEL_MAP_CHANGE_IND);

    for (index=0; index < sizeof(ind->map); index++)
    {
        message->map[index] = ind->map[index];
    }
    MessageSend(connectionGetCsbMsgDestTask(),
                CL_CSB_CHANNEL_MAP_CHANGE_IND,
                message);
}

#endif /* DISABLE_CSB */
