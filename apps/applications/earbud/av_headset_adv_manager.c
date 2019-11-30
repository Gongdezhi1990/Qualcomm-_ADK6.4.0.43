/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Management of Bluetooth Low Energy advertising
*/

#include <adc.h>
#include <panic.h>

#include "av_headset.h"
#include "av_headset_adv_manager.h"
#include "av_headset_adv_manager_private.h"
#include "av_headset_log.h"
#include "av_headset_test.h"
#include "hydra_macros.h"

static struct _advManagerAdvert  advertData[1];


static void appAdvManagerInitAdvert(advManagerAdvert *advert)
{
    memset(advert,0,sizeof(*advert));
}


advManagerAdvert *appAdvManagerNewAdvert(void)
{
    advManagerAdvert *advert = &advertData[0];

    if (advert->in_use)
        return NULL;

    appAdvManagerInitAdvert(advert);

    advert->channel_map_mask = BLE_ADV_CHANNEL_ALL;
    advert->in_use = TRUE;

    return advert;
}


/*! \todo documented interface needs to check if we are advertising,
        stop advertising and then fail accordingly */
bool appAdvManagerDeleteAdvert(advManagerAdvert *advert)
{
    /* Could just return T/F for an invalid advert, but should not happen */
    PanicFalse(VALID_ADVERT_POINTER(advert));

    if (!advert->in_use)
        return TRUE;

    /* Clear advert in case any rogue pointers remain */
    appAdvManagerInitAdvert(advert);

    advert->in_use = FALSE;
    return TRUE;
}


static bool appAdvManagerSetAdvertisingData(uint8 size_ad_data, const uint8 *ad_data)
{
    if (0 == size_ad_data || size_ad_data > BLE_AD_PDU_SIZE || !ad_data)
    {
        DEBUG_LOG("appAdvManagerSetAdvertisingData Bad length.");

        return FALSE;
    }

    ConnectionDmBleSetAdvertisingDataReq(size_ad_data,ad_data);
    return TRUE;
}


/* If the advert has space for the full local device name, or at least
   MIN_LOCAL_NAME_LENGTH, then reduce the space available by that much.

   That allows elements that precede the name to be truncated while 
   leaving space for the name */
static uint8 reserveSpaceForLocalName(uint8* space, uint16 name_length)
{
    uint8 required_space = MIN(name_length, MIN_LOCAL_NAME_LENGTH);
    
    if((*space) >= required_space)
    {
        *space -= required_space;
        return required_space;
    }
    return 0;
}


static void restoreSpaceForLocalName(uint8* space, uint8 reserved_space)
{
    *space += reserved_space;
}


static void saveLocalName(const CL_DM_LOCAL_NAME_COMPLETE_T *name)
{
    advManagerTaskData *advMgr = appGetAdvManager();

    free(advMgr->localName);
    advMgr->localName = NULL;

    if (name->status == hci_success)
    {
        advMgr->localName = calloc(1,name->size_local_name+1);
        memcpy(advMgr->localName,name->local_name,name->size_local_name);
    }

    if (ADV_MGR_STATE_STARTING == advMgr->state)
    {
        advMgr->state = ADV_MGR_STATE_INITIALISED;

        MessageSend(appGetAppTask(), APP_ADVMGR_INIT_CFM, NULL);
    }
}


static uint8* addHeaderToAdData(uint8* ad_data, uint8* space, uint8 size, uint8 type)
{
    WRITE_AD_DATA(ad_data, space, size);
    WRITE_AD_DATA(ad_data, space, type);

    return ad_data;
}


static uint8* addUuidToAdData(uint8* ad_data, uint8* space, uint16 uuid)
{
    WRITE_AD_DATA(ad_data, space, uuid & 0xFF);
    WRITE_AD_DATA(ad_data, space, uuid >> 8);

    return ad_data;
}


/*! Send a completion message for the current blocking operation (if any).
    If there is no task registered for the message, no message is sent */
static void appAdvManagerSendBlockingResponse(connection_lib_status sts)
{
    advManagerTaskData *advMgr = appGetAdvManager();

    if (advMgr->blockingOperation)
    {
        if (advMgr->blockingTask)
        {
            MAKE_MESSAGE(APP_ADVMGR_ADVERT_SET_DATA_CFM);

            message->status = sts;

            MessageSend(advMgr->blockingTask, advMgr->blockingOperation, message);
            advMgr->blockingTask = NULL;
        }
        advMgr->blockingCondition = ADV_SETUP_BLOCK_NONE;
        advMgr->blockingOperation  = 0;
        advMgr->blockingAdvert = NULL;
    }
}


static uint8* appAdvManagerAdvertdataAddName(uint8 *ad_data, uint8* space, uint16 size_local_name, const uint8 * local_name, bool shortened)
{
    uint8 name_field_length;
    uint8 name_data_length = size_local_name;
    uint8 ad_tag = ble_ad_type_complete_local_name;
    uint8 usable_space = USABLE_SPACE(space);

    if((name_data_length == 0) || usable_space <= 1)
        return ad_data;

    if (name_data_length > usable_space)
    {
        ad_tag = ble_ad_type_shortened_local_name;
        name_data_length = usable_space;
    }
    else if (shortened)
    {
        ad_tag = ble_ad_type_shortened_local_name;
    }

    name_field_length = AD_FIELD_LENGTH(name_data_length);
    ad_data = addHeaderToAdData(ad_data, space, name_field_length, ad_tag);

    /* Setup the local name advertising data */
    memmove(ad_data, local_name, name_data_length);
    ad_data += name_data_length;
    *space -= name_data_length;

    return ad_data;
}


static uint8* appAdvManagerAdvertdataAddFlags(uint8* ad_data, uint8* space, uint8 flags)
{
    uint8 usable_space = USABLE_SPACE(space);

    if (usable_space < 1)
    {
        DEBUG_LOG("appAdvManagerAdvertdataAddFlags. No space for flags in advert");
    }
    else
    {
        /* According to CSSv6 Part A, section 1.3 "FLAGS" states: 
            "The Flags data type shall be included when any of the Flag bits are non-zero and the advertising packet 
            is connectable, otherwise the Flags data type may be omitted"
         */
        ad_data = addHeaderToAdData(ad_data, space, FLAGS_DATA_LENGTH, ble_ad_type_flags);
        WRITE_AD_DATA(ad_data, space, flags);
    }

    return ad_data;
}


static uint8* appAdvManagerAdvertdataAddServices(uint8* ad_data, uint8* space,
                                                 uint16 *service_list, uint16 services)
{
    uint8 num_services_that_fit = NUM_SERVICES_THAT_FIT(space);

    if (services && num_services_that_fit)
    {
        uint8 service_data_length;
        uint8 service_field_length;
        uint8 ad_tag = ble_ad_type_complete_uuid16;

        if(services > num_services_that_fit)
        {
            ad_tag = ble_ad_type_more_uuid16;
            services = num_services_that_fit; 
        }

        /* Setup AD data for the services */
        service_data_length = SERVICE_DATA_LENGTH(services);
        service_field_length = AD_FIELD_LENGTH(service_data_length);
        ad_data = addHeaderToAdData(ad_data, space, service_field_length, ad_tag);

        while ((*space >= OCTETS_PER_SERVICE) && services--)
        {
            ad_data = addUuidToAdData(ad_data, space, *service_list++);
        }
        
        return ad_data;
    }
    return ad_data;
}


static void appAdvManagerSetupAdvert(advManagerAdvert *advert)
{
    uint8 space = MAX_AD_DATA_SIZE_IN_OCTETS * sizeof(uint8);
    uint8 *ad_start = (uint8*)PanicNull(malloc(space));
    uint8 *ad_head = ad_start;
    unsigned name_len = 0;
    uint8 space_reserved_for_name = 0;

    PanicFalse(VALID_ADVERT_POINTER(advert));
    PanicFalse(advert->content.parameters);

    if (advert->content.flags)
    {
        ad_head = appAdvManagerAdvertdataAddFlags(ad_head, &space, advert->flags);
    }

    if (advert->content.local_name)
    {
        name_len = strlen((char *)advert->local_name);
        space_reserved_for_name = reserveSpaceForLocalName(&space, name_len);
    }

    if (advert->content.services)
    {
        ad_head = appAdvManagerAdvertdataAddServices(ad_head, &space, 
                                                     advert->services_uuid16, 
                                                     advert->num_services);
    }

    if (advert->content.local_name)
    {
        restoreSpaceForLocalName(&space, space_reserved_for_name);

        ad_head = appAdvManagerAdvertdataAddName(ad_head, &space, name_len, advert->local_name, advert->content.local_name_shortened);
    }

    if (appAdvManagerSetAdvertisingData(ad_head - ad_start, ad_start))
    {
        appGetAdvManager()->blockingAdvert = advert;
        appGetAdvManager()->blockingCondition = ADV_SETUP_BLOCK_ADV_DATA_CFM;
    }
    else
    {
        appAdvManagerSendBlockingResponse(fail);
    }

    free(ad_start);
}


static void appAdvManagerEnableAdvertising(advManagerAdvert *advert)
{
    /*! \todo Need to deal with gatt as well as advertising set this way */
    UNUSED(advert);
}


static void appAdvManagerHandleStartAdvert(const ADV_MANAGER_START_ADVERT_T *message)
{
    appGetAdvManager()->blockingTask = message->requester;
    appGetAdvManager()->blockingOperation = APP_ADVMGR_ADVERT_START_CFM;

    appAdvManagerSetupAdvert(message->advert);

    /*! \todo implement sequencing */
    DEBUG_LOG("appAdvManagerHandleStartAdvert not yet implemented with requester tasks / sequence");
    Panic(); 
    appAdvManagerEnableAdvertising(message->advert);
}


static void appAdvManagerHandleSetupAdvert(const ADV_MANAGER_SETUP_ADVERT_T *message)
{
    appGetAdvManager()->blockingTask = message->requester;
    appGetAdvManager()->blockingOperation = APP_ADVMGR_ADVERT_SET_DATA_CFM;

    appAdvManagerSetupAdvert(message->advert);
}


static void appAdvManagerHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch (id)
    {
            /* Connection library message sent directly */
        case CL_DM_LOCAL_NAME_COMPLETE:
            saveLocalName((const CL_DM_LOCAL_NAME_COMPLETE_T*)message);
            return;

            /* Internal messages */
        case ADV_MANAGER_START_ADVERT:
            appAdvManagerHandleStartAdvert((const ADV_MANAGER_START_ADVERT_T*)message);
            return;

        case ADV_MANAGER_SETUP_ADVERT:
            appAdvManagerHandleSetupAdvert((const ADV_MANAGER_SETUP_ADVERT_T*)message);
            return;
    }

    DEBUG_LOG("appAdvManagerHandleMessage. Unhandled message. Id: 0x%X (%d) 0x%p", id, id, message);
}


static bool appAdvManagerHandleSetAdvertisingDataCfm(const CL_DM_BLE_SET_ADVERTISING_DATA_CFM_T* cfm)
{
    if (appGetAdvManager()->blockingCondition == ADV_SETUP_BLOCK_ADV_DATA_CFM)
    {
        if (success == cfm->status)
        {
            advManagerAdvert *advert = appGetAdvManager()->blockingAdvert;

            DEBUG_LOG("appAdvManagerHandleSetAdvertisingDataCfm: success");

            appGetAdvManager()->blockingCondition = ADV_SETUP_BLOCK_ADV_PARAMS_CFM;

            ConnectionDmBleSetAdvertisingParamsReq(advert->advertising_type, 
                                                   advert->use_own_random, 
                                                   advert->channel_map_mask, 
                                                   &advert->interval_and_filter);
        }
        else
        {
            DEBUG_LOG("appAdvManagerHandleSetAdvertisingDataCfm: failed:%d", cfm->status);

            appAdvManagerSendBlockingResponse(fail);
        }
    }
    else
    {
        DEBUG_LOG("appAdvManagerHandleSetAdvertisingDataCfm. RECEIVED in unexpected blocking state %d",
                                    appGetAdvManager()->blockingCondition);
    }

    return TRUE;
}


static void appAdvManagerHandleSetAdvertisingParamCfm(const CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM_T *cfm)
{

    if (appGetAdvManager()->blockingCondition == ADV_SETUP_BLOCK_ADV_PARAMS_CFM)
    {
        DEBUG_LOG("appAdvManagerHandleSetAdvertisingParamCfm: Status %d",cfm->status);

        appAdvManagerSendBlockingResponse(cfm->status);
    }
    else
    {
        DEBUG_LOG("appAdvManagerHandleSetAdvertisingParamCfm: RECEIVED in unexpected blocking state %d",
                                    appGetAdvManager()->blockingCondition);
    }
}

bool appAdvManagerHandleConnectionLibraryMessages(MessageId id, Message message, bool already_handled)
{
    switch (id)
    {
        case CL_DM_BLE_SET_ADVERTISING_DATA_CFM:
            appAdvManagerHandleSetAdvertisingDataCfm((const CL_DM_BLE_SET_ADVERTISING_DATA_CFM_T *)message);
            return TRUE;

        case CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM:
            appAdvManagerHandleSetAdvertisingParamCfm((const CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM_T *)message);
            return TRUE;
    }

    if (!already_handled)
    {
        DEBUG_LOG("appAdvManagerHandleConnectionLibraryMessages. Unhandled message. Id: 0x%X (%d) 0x%p", id, id, message);
    }

    return already_handled;
}


/*! Stopping any ongoing adverts. 

    \todo message if successful ?  with task ?
*/
static void appAdvManagerStopAllAdverts(void)
{
    advManagerTaskData *advMan = appGetAdvManager();
    
    if (ADV_MGR_STATE_ADVERTISING == advMan->state)
    {
        /*! \todo Stop advertising !!!! */
        DEBUG_LOG("Standalone adverts not implemented");
        Panic();
        advMan->state = ADV_MGR_STATE_INITIALISED;
    }
}


/*! \todo Somewhat more extensive validation */
static bool appAdvManagerCheckAdvert(advManagerAdvert *advert)
{
    return VALID_ADVERT_POINTER(advert);
}


bool appAdvManagerSetAdvertData(advManagerAdvert *advert, Task requester)
{
    MAKE_MESSAGE(ADV_MANAGER_SETUP_ADVERT);

    message->advert = advert;
    message->requester = requester;
    MessageSendConditionally(appGetAdvManagerTask(), ADV_MANAGER_SETUP_ADVERT, message, &appGetAdvManager()->blockingCondition);

    return TRUE;
}



bool appAdvManagerStart(advManagerAdvert *advert, Task requester)
{
    MAKE_MESSAGE(ADV_MANAGER_START_ADVERT);

    /*! \todo Need to decide if adv manager has multiple adverts or not */

    DEBUG_LOG("appAdvManagerStart not implemented");
    Panic();

    appAdvManagerStopAllAdverts();

    if (!appAdvManagerCheckAdvert(advert))
        return FALSE;

    message->advert = advert;
    message->requester = requester;
    MessageSendConditionally(appGetAdvManagerTask(), ADV_MANAGER_START_ADVERT, message, &appGetAdvManager()->blockingCondition);

    return TRUE;
}


bool appAdvManagerSetName(advManagerAdvert *advert,uint8 *name)
{
    PanicFalse(VALID_ADVERT_POINTER(advert));
    PanicNull(name);

    unsigned name_len = strlen((char *)name);
    unsigned max_name_len = sizeof(advert->local_name)-1;   /* Avoid #define */
    bool shorten_name = name_len > max_name_len;

    advert->content.local_name = TRUE;
    advert->content.local_name_shortened = shorten_name;

    if (shorten_name)
    {
        name_len = max_name_len;
    }

    memset(advert->local_name,0,sizeof(advert->local_name));
    memcpy(advert->local_name,name,name_len);

    return TRUE;
}


void appAdvManagerUseLocalName(advManagerAdvert *advert)
{
    uint8 *local_name = appGetAdvManager()->localName;

    if (NULL == local_name)
    {
        DEBUG_LOG("appAdvManagerUseLocalName No name set, using empty name");
        local_name = (uint8 *)"";
    }

    appAdvManagerSetName(advert, local_name);
}


bool appAdvManagerSetDiscoverableMode(advManagerAdvert *advert, 
                           avHeadsetBleDiscoverableMode discoverable_mode)
{
    PanicFalse(VALID_ADVERT_POINTER(advert));

    uint16 flags = advert->flags & ~(BLE_FLAGS_GENERAL_DISCOVERABLE_MODE|BLE_FLAGS_LIMITED_DISCOVERABLE_MODE);

    if (discoverable_mode == avHeadsetBleDiscoverableModeGeneral)
    {
        flags |= BLE_FLAGS_GENERAL_DISCOVERABLE_MODE;
    }
    else if (discoverable_mode == avHeadsetBleDiscoverableModeLimited)
    {
        flags |= BLE_FLAGS_LIMITED_DISCOVERABLE_MODE;
    }

    advert->flags = flags;
    advert->content.flags = !!flags;

    return TRUE;
}


bool appAdvManagerSetReadNameReason(advManagerAdvert *advert,
                                    avHeadsetBleGapReadName reason)
{
    PanicFalse(VALID_ADVERT_POINTER(advert));
    
    uint16 flags = advert->flags & ~BLE_FLAGS_DUAL_HOST;

    /* broadcasting would set flag here also, but not supported */
    if(reason == avHeadsetBleGapReadNameAssociating)
    {
        flags |= BLE_FLAGS_DUAL_HOST;
    }
    advert->reason = reason;

    return TRUE;
}


static bool appAdvManagerServiceExists(advManagerAdvert *advert, uint16 uuid)
{
    if (advert && advert->content.services)
    {
        unsigned service = advert->num_services;
    
        while (service--)
        {
            if (advert->services_uuid16[service] == uuid)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool appAdvManagerSetService(advManagerAdvert *advert, uint16 uuid)
{

    PanicFalse(VALID_ADVERT_POINTER(advert));

    if (advert->num_services >= MAX_SERVICES)
        return FALSE;

    if (appAdvManagerServiceExists(advert, uuid))
        return TRUE;

    advert->services_uuid16[advert->num_services++] = uuid;
    advert->content.services = TRUE;

    return TRUE;
}


void appAdvManagerSetAdvertParams(advManagerAdvert *advert, ble_adv_params_t *adv_params)
{
    PanicFalse(VALID_ADVERT_POINTER(advert));
    PanicFalse(adv_params);

    advert->interval_and_filter = *adv_params;
    advert->content.parameters = TRUE;
}


void appAdvManagerSetAdvertisingType(advManagerAdvert *advert, ble_adv_type advert_type)
{
    PanicFalse(VALID_ADVERT_POINTER(advert));

    advert->advertising_type = advert_type;
    advert->content.advert_type = TRUE;
}


void appAdvManagerSetAdvertisingChannels(advManagerAdvert *advert, uint8 channel_mask)
{
    PanicFalse(VALID_ADVERT_POINTER(advert));

    advert->channel_map_mask = channel_mask;
}


void appAdvManagerSetUseOwnRandomAddress(advManagerAdvert *advert, bool use_random_address)
{
    PanicFalse(VALID_ADVERT_POINTER(advert));

    advert->use_own_random = use_random_address;
}


void appAdvManagerInit(void)
{
    advManagerTaskData *advMan = appGetAdvManager();
    memset(advMan, 0, sizeof(*advMan));
    advMan->task.handler = appAdvManagerHandleMessage;

    DEBUG_LOG("appAdvManagerInit");

    ConnectionReadLocalName(appGetAdvManagerTask());
}

