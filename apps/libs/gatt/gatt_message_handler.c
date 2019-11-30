/****************************************************************************
Copyright (c) 2011 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_message_handler.c        

DESCRIPTION
    Handles messages received from the ATT protocol and internal GATT library
    messages and routes them to the appropriate handler functions.

NOTES

*/

#include "gatt.h"
#include "gatt_private.h"

#if (GATT_FEATURES & (GATT_FIND_INCLUDED_SERVICES | GATT_READ))
/*************************************************************************
NAME    
    handleAttReadCfm
    
DESCRIPTION
    Routes ATT_READ_CFM to correct sub-procedure handler.
    
RETURNS
    
*/
static void handleAttReadCfm(const ATT_READ_CFM_T *m)
{
    conn_data_t* data = (conn_data_t*)PanicNull(gattGetCidData(m->cid));
    switch (data->scenario)
    {
#if (GATT_FEATURES & GATT_READ)
        case gatt_ms_read:
            gattHandleAttReadCfmRead(m);
            break;
#endif

#if (GATT_FEATURES & GATT_FIND_INCLUDED_SERVICES)
        case gatt_ms_find_includes:
            gattHandleAttReadCfmInclude(m);
            break;
#endif
                                    
        default:
            GATT_DEBUG_INFO(("Unrouted ATT_READ_CFM\n"));
    }
}
#endif

#if (GATT_FEATURES & (GATT_RELATIONSHIP_DISCOVERY | \
                      GATT_CHARACTERISTIC_DISCOVERY))
/*************************************************************************
NAME    
    handleAttReadByTypeCfm
    
DESCRIPTION
    Routes ATT_READ_BY_TYPE_CFM to correct sub-procedure handler.
    
RETURNS
    
*/
static void handleAttReadByTypeCfm(const ATT_READ_BY_TYPE_CFM_T *m)
{
    conn_data_t* data = (conn_data_t*)PanicNull(gattGetCidData(m->cid));
    switch (data->scenario)
    {
#if (GATT_FEATURES & GATT_FIND_INCLUDED_SERVICES)
        case gatt_ms_find_includes:
            gattHandleAttReadByTypeCfmInclude(m);
            break;
#endif

#if (GATT_FEATURES & GATT_DISC_ALL_CHARACTERISTICS)
        case gatt_ms_discover_all_characteristics:
            gattHandleAttReadByTypeCfmCharacteristic(m);
            break;
#endif

#if (GATT_FEATURES & GATT_READ_BY_UUID)
        case gatt_ms_read_by_uuid:
            gattHandleAttReadByTypeCfmRead(m);
            break;
#endif            
    }
}
#endif

/****************************************************************************
NAME
    gattMessageHandler

DESCRIPTION
    Recieves and routes messages from ATT protocol and internal GATT messages.
*/

void gattMessageHandler(Task task, MessageId id, Message message) 
{
    gattState   *theGatt = (gattState *)task;
    uint16      type = ((const ATT_UPRIM_T *)message)->type;

    /* 
     * Large switch structures used to reduce call stack.
     * 'return' after successful message handling.
     * 'break' will drop through to debug info for unexpected messages.
     */
    switch (theGatt->state)
    {
        case gatt_state_uninitialised:
            /* Shouldn't get any messages in this state so ignore. */
            break;

        case gatt_state_initialising:
            if (MESSAGE_BLUESTACK_ATT_PRIM  == id)
            {
                switch(type)
                {
                    case ATT_REGISTER_CFM:
                        gattHandleAttRegisterCfm(
                            theGatt, 
                            (const ATT_REGISTER_CFM_T *)message
                            );
                        return;

                    case ATT_ADD_DB_CFM:
                        gattHandleAttAddDbCfm(
                            theGatt,
                            (const ATT_ADD_DB_CFM_T *)message
                            );
                        return; 
                        
                    default:
                        break;
                } /* end type switch */
            }
            break;
            
#ifdef GATT_CACHING            
        case gatt_state_gatt_caching_init:
            switch(type)
            {
                case ATT_READ_BY_TYPE_CFM:
                    gattInitGattCaching((const ATT_READ_BY_TYPE_CFM_T *)message);
                    return;

                default:
                    break;
            } /* end type switch */
            
        case gatt_state_gatt_caching_cont:
            switch(type)
            {
                case ATT_READ_BY_TYPE_CFM:
                    gattFindServChanged((const ATT_READ_BY_TYPE_CFM_T *)message);
                    return;

                default:
                    break;
            } /* end type switch */
#endif

        case gatt_state_initialised:
            switch(id)
            {
                case MESSAGE_BLUESTACK_ATT_PRIM:
                    switch(type)
                    {
                        case ATT_CONNECT_CFM:
                            gattHandleAttConnectCfm(
                                theGatt,
                                (const ATT_CONNECT_CFM_T *)message
                                );
                            return;

                        case ATT_DISCONNECT_IND:
                            gattHandleAttDisconnectInd(
                                (const ATT_DISCONNECT_IND_T *)message
                                );
                            return;

                        case ATT_CONNECT_IND:
                            gattHandleAttConnectInd(
                                (const ATT_CONNECT_IND_T *)message
                                );
                            return;

#if (GATT_FEATURES & GATT_EXCHANGE_MTU)
                        case ATT_EXCHANGE_MTU_CFM:
                            gattHandleAttExchangeMtuCfm(
                                (const ATT_EXCHANGE_MTU_CFM_T *)message
                                );
                            return;
#endif

                        case ATT_EXCHANGE_MTU_IND:
                            gattHandleAttExchangeMtuInd(
                                theGatt,
                                (const ATT_EXCHANGE_MTU_IND_T *)message
                                );
                            return;

#if (GATT_FEATURES & GATT_DISC_ALL_PRIMARY_SERVICES)
                        case ATT_READ_BY_GROUP_TYPE_CFM:
                            gattHandleAttReadByGroupTypeCfm(
                                (const ATT_READ_BY_GROUP_TYPE_CFM_T*)message);
                            return;
#endif

#if (GATT_FEATURES & GATT_DISC_PRIMARY_SERVICE)
                        case ATT_FIND_BY_TYPE_VALUE_CFM:
                            gattHandleAttFindByTypeValueCfm(
                                (const ATT_FIND_BY_TYPE_VALUE_CFM_T*)message);
                            return;
#endif

#if (GATT_FEATURES & (GATT_FIND_INCLUDED_SERVICES | \
                      GATT_DISC_ALL_CHARACTERISTICS | \
                      GATT_READ_BY_UUID))
                        case ATT_READ_BY_TYPE_CFM:
                            handleAttReadByTypeCfm(
                                (const ATT_READ_BY_TYPE_CFM_T*)message);
                            return;
#endif
                            
#if (GATT_FEATURES & GATT_DESCRIPTOR_DISCOVERY)
                        case ATT_FIND_INFO_CFM:
                            gattHandleAttFindInfoCfm(
                                (const ATT_FIND_INFO_CFM_T*)message);
                            return;
#endif
                            
#if (GATT_FEATURES & (GATT_FIND_INCLUDED_SERVICES | GATT_READ))
                        case ATT_READ_CFM:
                            handleAttReadCfm((const ATT_READ_CFM_T*)message);
                            return;                            
#endif

#if (GATT_FEATURES & GATT_READ_LONG)
                        case ATT_READ_BLOB_CFM:
                            gattHandleAttReadBlobCfm(
                                (const ATT_READ_BLOB_CFM_T*)message);
                            return;                            
#endif
                            
#if (GATT_FEATURES & GATT_READ_MULTIPLE)
                        case ATT_READ_MULTI_CFM:
                            gattHandleAttReadMultiCfm(
                                (const ATT_READ_MULTI_CFM_T*)message);
                            return;
#endif

#if (GATT_FEATURES & (GATT_WRITE_SIGNED | GATT_WRITE))
                        case ATT_WRITE_CFM:
                            gattHandleAttWriteCfm((const ATT_WRITE_CFM_T*)message);
                            return;
#endif

#if (GATT_FEATURES & GATT_WRITE_COMMAND)
                        case ATT_WRITE_CMD_CFM:
                            gattHandleAttWriteCmdCfm(
                                    (const ATT_WRITE_CMD_CFM_T *)message);
                            return;
#endif
                            
#if (GATT_FEATURES & (GATT_WRITE_LONG | GATT_WRITE_RELIABLE))
                        case ATT_PREPARE_WRITE_CFM:
                            gattHandleAttPrepareWriteCfm(
                                (const ATT_PREPARE_WRITE_CFM_T*)message);
                            return;

                        case ATT_EXECUTE_WRITE_CFM:
                            gattHandleAttExecuteWriteCfm(
                                (const ATT_EXECUTE_WRITE_CFM_T*)message);
                            return;
#endif
                            
                        case ATT_ACCESS_IND:
                            gattHandleAttAccessInd(
                                (const ATT_ACCESS_IND_T *)message
                                );
                            return;

                        case ATT_HANDLE_VALUE_NTF_CFM:
                            gattHandleAttHandleValueNtfCfm(
                                (const ATT_HANDLE_VALUE_NTF_CFM_T *)message
                                );
                            return;

                        case ATT_HANDLE_VALUE_CFM:
                            gattHandleAttHandleValueCfm(
                                (const ATT_HANDLE_VALUE_CFM_T *)message
                                );
                            return;

                        case ATT_HANDLE_VALUE_IND:
                            gattHandleAttHandleValueInd(
                                (const ATT_HANDLE_VALUE_IND_T *)message
                                );
                            return;
                            
#ifdef GATT_CACHING
                        case ATT_CHANGE_AWARE_IND:
                            gattHandleAttChangeAwareInd(
                                (const ATT_CHANGE_AWARE_IND_T *)message
                                );
                            return;
#endif

                        default:
                            break;
                    } /* ATT Message type switch */
                    break;

                /*
                 * Connection library messages
                 */
#if (GATT_FEATURES & GATT_PRIMARY_DISCOVERY)
                case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
                    gattHandleClSdpServiceSearchAttributeCfm(
                        (const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T*)message);
                    return;
#endif
                    
                /*
                 *Internal GATT library messages 
                 */
                case GATT_INTERNAL_CONNECT_REQ:
                    gattHandleInternalConnectReq(
                        theGatt,
                        (const GATT_INTERNAL_CONNECT_REQ_T *)message
                        );
                    return;

                case GATT_INTERNAL_DISCONNECT_REQ:
                    gattHandleInternalDisconnectReq(
                        (const GATT_INTERNAL_DISCONNECT_REQ_T *)message
                        );
                    return;

                case GATT_INTERNAL_DELETE_CONN:
                    gattHandleInternalDeleteConn(
                        (const GATT_INTERNAL_DELETE_CONN_T *)message
                        );
                    return;

#if (GATT_FEATURES & GATT_EXCHANGE_MTU)
                case GATT_INTERNAL_EXCHANGE_MTU_REQ:
                    gattHandleInternalExchangeMtuReq(
                        (const GATT_INTERNAL_EXCHANGE_MTU_REQ_T *)message
                        );
                    return;
#endif

#if (GATT_FEATURES & GATT_DISC_ALL_PRIMARY_SERVICES)
                case GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ:
                    gattHandleInternalDiscoverAllPrimaryServicesReq(
                        (const GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ_T*)
                        message);
                    return;
#endif

#if (GATT_FEATURES & GATT_DISC_PRIMARY_SERVICE)
                case GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ:
                    gattHandleInternalDiscoverPrimaryServiceReq(
                        (const GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ_T*)
                        message);
                    return;
#endif

#if (GATT_FEATURES & GATT_DESCRIPTOR_DISCOVERY)
                case GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ:
                    gattHandleInternalDiscoverAllDescriptorsReq(
                        (const GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ_T*)
                        message);
                    return;
#endif                    
                    
#if (GATT_FEATURES & GATT_RELATIONSHIP_DISCOVERY)
                        case GATT_INTERNAL_FIND_INCLUDES_REQ:
                            gattHandleInternalFindIncludesReq(
                                (const GATT_INTERNAL_FIND_INCLUDES_REQ_T*)message);
                            return;
#endif

#if (GATT_FEATURES & (GATT_CHARACTERISTIC_DISCOVERY))
                case GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ:
                    gattHandleGattDiscoverAllCharacteristicsReq(
                        (const GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ_T*)
                        message);
                    return;
#endif

#if (GATT_FEATURES & GATT_READ)
                case GATT_INTERNAL_READ_REQ:
                    gattHandleInternalReadReq(
                        (const GATT_INTERNAL_READ_REQ_T *)message);
                    return;
#endif

#if (GATT_FEATURES & GATT_READ_BY_UUID)
                case GATT_INTERNAL_READ_BY_UUID_REQ:
                    gattHandleInternalReadByUuidReq(
                        (const GATT_INTERNAL_READ_BY_UUID_REQ_T*)message);
                    return;
#endif

#if (GATT_FEATURES & GATT_READ_LONG)
                case GATT_INTERNAL_READ_LONG_REQ:
                    gattHandleInternalReadLongReq(
                        (const GATT_INTERNAL_READ_LONG_REQ_T *)message);
                    return;
#endif
                    
#if (GATT_FEATURES & GATT_READ_MULTIPLE)
                case GATT_INTERNAL_READ_MULTIPLE_REQ:
                    gattHandleInternalReadMultipleReq(
                        (const GATT_INTERNAL_READ_MULTIPLE_REQ_T*)message);
                    return;
#endif

#if (GATT_FEATURES & GATT_CHARACTERISTIC_WRITE)
                case GATT_INTERNAL_WRITE_REQ:
                    gattHandleInternalWriteReq(
                        (const GATT_INTERNAL_WRITE_REQ_T*)message);
                    return;
#endif

#if (GATT_FEATURES & GATT_WRITE_RELIABLE)
                case GATT_INTERNAL_EXECUTE_WRITE_REQ:
                    gattHandleInternalExecuteWriteReq(
                        (const GATT_INTERNAL_EXECUTE_WRITE_REQ_T*)message);
                    return;
#endif
                    
                case GATT_INTERNAL_ACCESS_RES:
                    gattHandleInternalAccessRes(
                        (const GATT_INTERNAL_ACCESS_RES_T *)message
                        );
                    return;

                case GATT_INTERNAL_HANDLE_VALUE_NTF:
                    gattHandleInternalHandleValueNtf(
                        (const GATT_INTERNAL_HANDLE_VALUE_NTF_T *)message
                        );
                    return;

                case GATT_INTERNAL_HANDLE_VALUE_REQ:
                    gattHandleInternalHandleValueReq(
                        (const GATT_INTERNAL_HANDLE_VALUE_REQ_T *)message
                        );
                    return;

                default:
                    break;
            } /* id switch */
                
        default:
            break;
    } /* theGatt->state switch*/    

    GATT_DEBUG_INFO((
        "GATT Msg Hndler - Unknown Msg - State %d : id 0x%02X : type 0x%02X\n",
        theGatt->state,
        id,
        type));
}
