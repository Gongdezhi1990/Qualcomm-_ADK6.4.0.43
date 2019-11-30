/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*  */

#include <transport_manager.h>
#include "ama.h"
#include "ama_debug.h"
#include "ama_transport.h"
#include "ama_iap2.h"

static uint16 protocol_btalexa = 0;


void AmaIap2Init(Task task)
{
    if (protocol_btalexa == 0)
    {         
        if (TransportMgrRegisterAccessory(&protocol_btalexa, "com.amazon.btalexa") == transport_mgr_status_success)
        {
            transport_mgr_link_cfg_t trans_link_cfg;

            trans_link_cfg.type = transport_mgr_type_accessory;
            trans_link_cfg.trans_info.non_gatt_trans.trans_link_id = protocol_btalexa;
        
            TransportMgrRegisterTransport(task, &trans_link_cfg);
            amaTransportSetTxCallback(AmaIap2SendData, ama_transport_iap);
        }
    }
    
    AMA_DEBUG(("AMA btalexa = %d\n", protocol_btalexa));
}


bool AmaIap2SendData(uint8 *data, uint16 length)
{
    bool sent = FALSE;

    if (protocol_btalexa != 0)
    {
        transport_mgr_status_t status;

        status = TransportMgrWriteData(transport_mgr_type_accessory, protocol_btalexa, data, length);
        sent = status == transport_mgr_status_success;
    }

    AMA_DEBUG(("AMA Send %d = %d\n", length, sent));
    return sent;
}
