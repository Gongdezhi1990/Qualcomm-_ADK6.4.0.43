/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    transport_manager_private.c

DESCRIPTION
    private/utility functions for transport manager
*/
#include <stdlib.h>
#include <source.h>
#include <sink.h>
#include <stream.h>
#include "transport_manager_private.h"
#include "transport_manager.h"
#include "print.h"

static transport_mgr_private_t *trans_private_data;

/******************************************************************************/
transport_mgr_private_t * transportMgrGetPrivateData(void)
{
    return trans_private_data;
}

/******************************************************************************/
Task transportMgrGetTransportTask(void)
{
    transport_mgr_private_t *trans_data = transportMgrGetPrivateData();
    if (trans_data)
    {
        return &trans_data->transport_mgr_task;
    }

    return NULL;
}

/******************************************************************************/
void transportMgrInitPrivateData(void)
{
    if(!transportMgrGetPrivateData())
    {
        trans_private_data = (transport_mgr_private_t *)PanicNull(calloc(1, sizeof(transport_mgr_private_t)));
        trans_private_data->trans_reg_state = UNBLOCKED;
        trans_private_data->trans_list = NULL;
        trans_private_data->trans_data_list = NULL;
        trans_private_data->transport_mgr_task.handler=transportMgrMessageHandler;
    }
}

/******************************************************************************/
void transportMgrSetBusy(uint16 state)
{
    trans_private_data->trans_reg_state = state;
}

/******************************************************************************/
uint16 *isTransportMgrBusy(void)
{
    return &(trans_private_data->trans_reg_state);
}

/******************************************************************************/
void transportMgrAddRef(void *trans_data,transport_mgr_data_id_t data_block_id)
{

    switch (data_block_id)
    {
        case transport_mgr_link_data:
            if(((transport_mgr_link_data_t *)trans_data) != NULL)
            {
                ((transport_mgr_link_data_t *)trans_data)->next = trans_private_data->trans_list;
                trans_private_data->trans_list = ((transport_mgr_link_data_t *)trans_data);
            }
            break;
        case transport_mgr_data:
            if(((transport_mgr_data_t *)trans_data) != NULL)
            {
                ((transport_mgr_data_t *)trans_data)->next = trans_private_data->trans_data_list;
                trans_private_data->trans_data_list = ((transport_mgr_data_t *)trans_data);
            }
            break;
        default:
            break;
    }
}

/******************************************************************************/
bool transportMgrRemoveLinkRef(transport_mgr_link_data_t *trans_data)
{
    transport_mgr_link_data_t *temp,*prev;

    temp=prev=trans_private_data->trans_list;
    if((temp ==  NULL) ||(!trans_data))
    {
        return FALSE;
    }
    if(memcmp(temp->link_cfg,trans_data->link_cfg,sizeof(transport_mgr_link_cfg_t)) == 0)
    {
        trans_private_data->trans_list = temp->next;
        free(temp->link_cfg);
        free(temp);
        return TRUE;
    }
    /* its not the first node, so move ahead */
    temp = temp->next;
    while(temp!= NULL)
    {
        if(memcmp(temp->link_cfg,trans_data->link_cfg,sizeof(transport_mgr_link_cfg_t)) == 0)
        {
            prev->next = temp->next;
            free(temp->link_cfg);
            free(temp);
            return TRUE;
        }
        prev = temp;
        temp = temp->next;
    }
    return FALSE;
}

/******************************************************************************/
bool transportMgrRemoveDataRef(transport_mgr_data_t *trans_data)
{
    transport_mgr_data_t *temp,*prev;

    temp=prev=trans_private_data->trans_data_list;
    if((temp ==  NULL) ||(!trans_data))
    {
        return FALSE;
    }
    if(temp->data_id == trans_data->data_id)
    {
        trans_private_data->trans_data_list = temp->next;
        free(temp);
        return TRUE;
    }
    /* its not the first node, so move ahead */
    temp = temp->next;
    while(temp!= NULL)
    {
        if(temp->data_id == trans_data->data_id)
        {
            prev->next = temp->next;
            free(temp);
            return TRUE;
        }
        prev = temp;
        temp = temp->next;
    }
    return FALSE;
}

/******************************************************************************/
transport_mgr_link_data_t *transportMgrFindDataWithTransInfo(uint16 link_param,transport_mgr_type_t trans_type)
{
    transport_mgr_link_data_t *temp;
    temp = trans_private_data->trans_list;

    while(temp != NULL)
    {
        if(trans_type == transport_mgr_type_gatt)
        {
            if((temp->link_cfg->trans_info.gatt_trans.start_handle <= link_param) ||
                (temp->link_cfg->trans_info.gatt_trans.end_handle >= link_param) )
                return temp;
        }
        else
        {
             if(temp->link_cfg->trans_info.non_gatt_trans.trans_link_id == link_param)
                return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

/******************************************************************************/
transport_mgr_data_t *transportMgrFindDataFromDataList(uint16 data_id)
{
    transport_mgr_data_t *temp;
    temp = trans_private_data->trans_data_list;

    while(temp != NULL)
    {
        if(temp->data_id == data_id)
        {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

/******************************************************************************/
void transportMgrFlushInput(transport_mgr_type_t type, uint16 trans_id)
{
    uint16 unread;
    Source source;
    transport_mgr_link_data_t *temp = transportMgrFindDataWithTransInfo(trans_id,type);
    UNUSED(type);

    if( temp != NULL)
    {
        source = StreamSourceFromSink(temp->sink);
        unread = SourceSize(source);
        if(unread)
            SourceDrop(source,unread);
    }
}

/******************************************************************************/
uint16 transportMgrFindSizeOfDataList(void)
{
    uint16 size = 0;
    transport_mgr_data_t *temp = trans_private_data->trans_data_list;
    while(temp != NULL)
    {
        size++;
        temp = temp->next;
    }
    return size;
}
/******************************************************************************/
bool transportMgrWriteSinkData(transport_mgr_type_t type, uint16 trans_id, uint8 * data, uint16 len)
{
    uint16 offset;
    uint8 *mapped_addr;
    transport_mgr_link_data_t *temp = transportMgrFindDataWithTransInfo(trans_id,type);
    UNUSED(type);

    if( temp != NULL)
    {
        mapped_addr = SinkMap(temp->sink);
        if(mapped_addr)
        {
            offset = SinkClaim(temp->sink, len);
            if(offset != 0xFFFF)
            {
                memcpy(mapped_addr+offset, data, len);
                return TRUE;
            }
        }
    }
    return FALSE;
}

/******************************************************************************/
bool transportMgrSendSinkData(transport_mgr_type_t type, uint16 trans_id, uint16 len)
{
    transport_mgr_link_data_t *temp = transportMgrFindDataWithTransInfo(trans_id,type);
    UNUSED(type);

    if( temp != NULL)
    {
        return SinkFlush(temp->sink, len);
    }
    return FALSE;
}

/******************************************************************************/
uint16 transportMgrFindTranslinkIdFromSink(Sink ref_sink)
{
    transport_mgr_link_data_t *temp;
    temp = trans_private_data->trans_list;

    while(temp != NULL)
    {
        if(temp->sink == ref_sink)
            return temp->link_cfg->trans_info.non_gatt_trans.trans_link_id;
        temp = temp->next;
    }
    return INVALID_TRANS_LINK_ID;
}

/******************************************************************************/
transport_mgr_link_data_t *transportMgrFindTransDataFromSink(Sink ref_sink)
{
    transport_mgr_link_data_t *temp;
    temp = trans_private_data->trans_list;

    while(temp != NULL)
    {
        if(temp->sink == ref_sink)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

/******************************************************************************/
void transportMgrUpdateSink(uint16 trans_link_id, Sink sink_param,transport_mgr_type_t trans_type)
{
    transport_mgr_link_data_t *temp = transportMgrFindDataWithTransInfo(trans_link_id,trans_type);
    if(temp != NULL)
    {
        temp->sink = sink_param;
    }
}

/******************************************************************************/
void transportMgrListFindAndSendMessage(uint16 trans_link_id,MessageId id,void *message,transport_mgr_type_t trans_type)
{
    transport_mgr_link_data_t *temp  = transportMgrFindDataWithTransInfo(trans_link_id,trans_type);

    if(temp != NULL)
    {
            MessageSend(temp->application_task, id, message);
    }
}

/******************************************************************************/
void transportMgrListBroadcastGattMessage(MessageId id,void *message)
{

    transport_mgr_link_data_t *temp;
    temp = trans_private_data->trans_list;

    while(temp != NULL)
        {
            if(temp->link_cfg->type == transport_mgr_type_gatt)
                MessageSend(temp->application_task, id, message);
            temp = temp->next;
        }

}

#ifdef HOSTED_TEST_ENVIRONMENT
/*****************************************************************************/
void transportMgrDeinit()
{
    transport_mgr_link_data_t *temp ;
    transport_mgr_link_data_t *next;
    if(transportMgrGetPrivateData())
    {
        temp = trans_private_data->trans_list;
        while(temp != NULL)
        {
            next = temp->next;
            free(temp);
            temp = next;
        }
        free(trans_private_data);
        trans_private_data = NULL;
    }
}
#endif /*VA_TEST_BUILD*/

