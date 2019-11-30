/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    transport_manager_data_handler.c

DESCRIPTION
    Data handler for transport manager
*/
#include <stdlib.h>
#include <print.h>
#include <transport_adaptation.h>
#include <vmtypes.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include "transport_manager_private.h"
#include "transport_manager.h"

/* Utility */

/*static handlers*/

/* message handlers */



/******************************************************************************/
const uint8* TransportMgrReadData(transport_mgr_type_t type,uint16 trans_link_info)
{
    transport_mgr_data_t *data_instance = NULL;
    transport_mgr_link_data_t *link_instance = NULL;
    PRINT(("TransportMgrReadData\n"));

    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    link_instance = transportMgrFindDataWithTransInfo(trans_link_info,type);

    /* Check whether this link_id is already registered*/
    if(link_instance == NULL)
    {
        return NULL;
    }

    switch (type)
    {
    case transport_mgr_type_gatt:
    case transport_mgr_type_accessory:
        data_instance = transportMgrFindDataFromDataList(trans_link_info);
        return(data_instance? data_instance->data:NULL);
        
    default:
        return SourceMap(StreamSourceFromSink(link_instance->sink));
    }
}

/******************************************************************************/
bool TransportMgrWriteData(transport_mgr_type_t type,uint16 trans_link_info,uint8 *data_buffer,uint16 data_length)
{
    bool return_val = FALSE;
    PRINT(("TransportMgrWriteData\n"));

    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    /* Check whether this link_id is already registered*/
    if(!(transportMgrFindDataWithTransInfo(trans_link_info,type)))
    {
        return FALSE;
    }

    switch(type)
    {
        case transport_mgr_type_gatt:
            return_val = TransportGattSendData(trans_link_info, data_length, data_buffer);
            break;
            
        case transport_mgr_type_rfcomm:
            return_val = transportMgrWriteSinkData(type,trans_link_info, data_buffer, data_length);
            break;
            
        case transport_mgr_type_accessory:
            return_val = TransportAccessorySendData(trans_link_info, data_length, data_buffer);
            break;
            
        default:
            break;
    }

    return return_val;
}


/******************************************************************************/
transport_mgr_status_t TransportMgrDataConsumed(transport_mgr_type_t type,uint16 trans_link_info,uint16 data_consumed)
{
    transport_mgr_data_t *data_instance = NULL;
    transport_mgr_link_data_t *link_instance = NULL;
    transport_mgr_status_t ret_val = transport_mgr_status_fail;


    PRINT(("TransportMgrDataConsumed\n"));

    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    link_instance = transportMgrFindDataWithTransInfo(trans_link_info,type);

    /* Check whether this link_id is already registered*/
    if(link_instance == NULL)
    {
        return transport_mgr_status_not_registered;
    }

    switch(type)
    {
    case transport_mgr_type_gatt:
    case transport_mgr_type_accessory:
        {
            data_instance = transportMgrFindDataFromDataList(trans_link_info);
            if(data_instance)
            {
                transportMgrRemoveDataRef(data_instance);
                ret_val = transport_mgr_status_success;
            }
        }
        break;
        
    case transport_mgr_type_rfcomm:
        if(data_consumed <= (SourceSize(StreamSourceFromSink(link_instance->sink))))
        {
            SourceDrop(StreamSourceFromSink(link_instance->sink), data_consumed);
            ret_val = transport_mgr_status_success;
        }
        break;
        
    default:
        break;
    }
    
    return ret_val;
}

/******************************************************************************/
uint16 TransportMgrGetAvailableSpace(transport_mgr_type_t type,uint16 trans_link_info)
{
    uint16 space_available = NO_DATA_AVAILABLE;
    transport_mgr_link_data_t *temp = NULL;

    PRINT(("TransportMgrGetAvailableSpace\n"));
    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    switch (type)
    {
        case transport_mgr_type_gatt:
            space_available = GATT_DEFAULT_SPACE_AVAILABLE;
            break;

        case transport_mgr_type_rfcomm:
            temp = transportMgrFindDataWithTransInfo(trans_link_info,type);
            if( temp != NULL)
            {
                space_available = SinkSlack(temp->sink);
            }
            break;
            
        case transport_mgr_type_accessory:
            space_available = TransportAccessoryGetAvailableSpace(trans_link_info);
            break;
            
        default:
            break;
    }

    return space_available;
}
/******************************************************************************/
uint16 TransportMgrGetAvailableDataSize(transport_mgr_type_t type,uint16 trans_link_info)
{
    uint16 data_size = NO_DATA_AVAILABLE;
    transport_mgr_data_t *data_instance = NULL;
    transport_mgr_link_data_t *temp = NULL;
    PRINT(("TransportMgrGetAvailableDataSize\n"));
    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    data_instance = transportMgrFindDataFromDataList(trans_link_info);

    switch (type)
    {
    case transport_mgr_type_gatt:
    case transport_mgr_type_accessory:
        if(data_instance)
        {
            data_size = data_instance->data_size;
        }
        break;
        
    case transport_mgr_type_rfcomm:
        temp = transportMgrFindDataWithTransInfo(trans_link_info,type);
        if( temp != NULL)
        {
            data_size = SourceSize(StreamSourceFromSink(temp->sink));
        }
        break;
        
    default:
        break;
    }

    return data_size;
}

/******************************************************************************/
transport_mgr_status_t TransportMgrDataSend(transport_mgr_type_t type, uint16 trans_link_id, uint16 len)
{
    if((type == transport_mgr_type_rfcomm) && (transportMgrSendSinkData(type, trans_link_id, len)))
        return transport_mgr_status_success;
    else 
        return transport_mgr_status_fail;
}

