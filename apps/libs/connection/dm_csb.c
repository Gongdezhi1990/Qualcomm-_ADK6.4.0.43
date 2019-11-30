/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_csb.c

DESCRIPTION

NOTES

*/

#include "connection.h"
#include "connection_private.h"
#include "bdaddr.h"

#include <vm.h>

#ifndef DISABLE_CSB

void ConnectionCsbSetReservedLtAddr(uint16 lt_addr)
{
    MAKE_PRIM_C(DM_HCI_SET_RESERVED_LT_ADDR_REQ);
    prim->lt_addr = lt_addr;
    VmSendDmPrim(prim);
}

void ConnectionCsbDeleteReservedLtAddr(uint16 lt_addr)
{
    MAKE_PRIM_C(DM_HCI_DELETE_RESERVED_LT_ADDR_REQ);
    prim->lt_addr = lt_addr;
    VmSendDmPrim(prim);
}

void ConnectionCsbSet(bool enable, uint16 lt_addr, bool lpo_allowed,
                      uint16 packet_type, uint16 interval_min, uint16 interval_max,
                      uint16 supervision_timeout)
{
    MAKE_PRIM_C(DM_HCI_SET_CSB_REQ);
    prim->enable = enable;
    prim->lt_addr = lt_addr;
    prim->lpo_allowed = lpo_allowed;
    prim->packet_type = packet_type;
    prim->interval_min = interval_min;
    prim->interval_max = interval_max;
    prim->supervision_timeout = supervision_timeout;
    VmSendDmPrim(prim);
}

void ConnectionCsbWriteSyncTrainParameters(uint16 interval_min, uint16 interval_max,
                                           uint32 sync_train_timeout, uint8 service_data)
{
    MAKE_PRIM_C(DM_HCI_WRITE_SYNCHRONIZATION_TRAIN_PARAMS_REQ);
    prim->interval_min = interval_min;
    prim->interval_max = interval_max;
    prim->sync_train_timeout = sync_train_timeout;
    prim->service_data = service_data;
    VmSendDmPrim(prim);
}

void ConnectionCsbStartSyncTrain(void)
{
    MAKE_PRIM_C(DM_HCI_START_SYNCHRONIZATION_TRAIN_REQ);
    VmSendDmPrim(prim);
}

void ConnectionCsbReceiveSyncTrain(const bdaddr *bd_addr, uint16 sync_scan_timeout, uint16 sync_scan_window, uint16 sync_scan_interval)
{
    MAKE_PRIM_C(DM_HCI_RECEIVE_SYNCHRONIZATION_TRAIN_REQ);
    BdaddrConvertVmToBluestack(&prim->bd_addr, bd_addr);   
    prim->sync_scan_timeout = sync_scan_timeout;
    prim->sync_scan_window = sync_scan_window;
    prim->sync_scan_interval = sync_scan_interval;
    VmSendDmPrim(prim);
}
        
void ConnectionCsbReceive(bool enable, const bdaddr *bd_addr, uint8 lt_addr, uint16 interval,
                          uint32 clock_offset, uint32 next_csb_clock, uint16 supervision_timeout, uint8 remote_timing_accuracy,
                          uint16 skip, uint16 packet_type, const AfhMap afh_map)
{
    uint16 index = 0;
    MAKE_PRIM_C(DM_HCI_SET_CSB_RECEIVE_REQ);

    prim->enable = enable;
    BdaddrConvertVmToBluestack(&prim->bd_addr, bd_addr);   
    prim->lt_addr = lt_addr;
    prim->interval = interval;
    prim->clock_offset = clock_offset;
    prim->next_csb_clock = next_csb_clock;
    prim->supervision_timeout = supervision_timeout;
    prim->remote_timing_accuracy = remote_timing_accuracy;
    prim->skip = skip;
    prim->packet_type = packet_type;
    for (index=0; index < sizeof(prim->afh_channel_map); index++)
    {
        prim->afh_channel_map[index] = afh_map[index] & 0xFF;
    }
    VmSendDmPrim(prim);    
}

void ConnectionCsbReceiveTimeoutResponse(const bdaddr *bd_addr, uint8 lt_addr)
{
    MAKE_PRIM_T(DM_HCI_CSB_RX_TIMEOUT_RSP);
    BdaddrConvertVmToBluestack(&prim->bd_addr, bd_addr);   
    prim->lt_addr = lt_addr;
    VmSendDmPrim(prim);    
}

void ConnectionCsbTransmitTimeoutResponse(uint16 lt_addr)
{
    MAKE_PRIM_T(DM_HCI_CSB_TX_TIMEOUT_RSP);
    prim->lt_addr = lt_addr;
    VmSendDmPrim(prim);    
}


#endif /* DISABLE_CSB */
