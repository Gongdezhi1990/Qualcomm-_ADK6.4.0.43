/****************************************************************************
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.

*****************************************************************************/

#include "avrcp_shim.h"

#include <panic.h>
#include <stream.h>
#include <stdio.h>
#include <print.h>
#include <stdlib.h>
#include <string.h>
#include <vmal.h>
#include <message.h>


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/* Static for now - need StreamRegionSource which actually owns the data */
static uint8 op_data[] = {
    0x31, 
    0x32, 
    0x33, 
    0x34, 
    0x45
};
#endif 

static uint8 *gdata;

static TaskData cleanUpTask;



static void avrcpDataCleanUp(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
    case MESSAGE_SOURCE_EMPTY:
        {
            if (gdata)
            {
                free(gdata);
                        
            }
        }
        break;

    default:
        break;
    }
}

static Source avrcpSourceFromData(uint8 *data, uint16 length)
{
    uint16 i;
    Source src;

    if(!length || !data)
    {
        return 0;
    }
  
    gdata = (uint8*) malloc(length);
    for (i=0;i<length;i++)
        gdata[i] = data[i];
    
    src = StreamRegionSource(gdata, length);
    
    cleanUpTask.handler = avrcpDataCleanUp;

    MessageStreamTaskFromSink(StreamSinkFromSource(src), &cleanUpTask);

    return src;
}

void AvrcpHandleComplexMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        default:
            Panic();
            break;
    }
}

void AvrcpInitTestExtra(Task theAppTask, uint16 dev_type)
{
    avrcp_init_params config;
    config.device_type = dev_type;
    config.profile_extensions = AVRCP_VERSION_1_3;

    if(dev_type & avrcp_target){

        config.supported_target_features = (0x0F) |
                    AVRCP_PLAYER_APPLICATION_SETTINGS|AVRCP_GROUP_NAVIGATION;
    }
    if(dev_type & avrcp_controller){
        config.supported_controller_features = 0x0f;
    }

    AvrcpInit(theAppTask, &config);
}

void AvrcpInitBrowseTest(Task theAppTask, uint16 dev_type, 
                       uint16 extensions)
{
    avrcp_init_params config;
    uint8 supp_feature=0;
    config.device_type = dev_type;

    if(extensions & AVRCP_VERSION_1_3) 
    {
       supp_feature = AVRCP_PLAYER_APPLICATION_SETTINGS|AVRCP_GROUP_NAVIGATION;
    }

    if((extensions & AVRCP_BROWSING_SUPPORTED) == AVRCP_BROWSING_SUPPORTED)
    {
        supp_feature |=  (AVRCP_CATEGORY_1|AVRCP_CATEGORY_3);
    }

    if((extensions & AVRCP_DATABASE_AWARE_PLAYER_SUPPORTED)
                     == AVRCP_DATABASE_AWARE_PLAYER_SUPPORTED)
    {
        supp_feature |= AVRCP_VIRTUAL_FILE_SYSTEM_BROWSING |
                        AVRCP_MULTIPLE_MEDIA_PLAYERS;
    }

    if(supp_feature == 0)
    {
        supp_feature = AVRCP_CATEGORY_2;
    }

    if(dev_type & avrcp_target){
        config.supported_target_features = supp_feature;
    }

    if(dev_type & avrcp_controller){
        config.supported_controller_features = supp_feature;
    }
    config.profile_extensions = extensions;

    AvrcpInit(theAppTask, &config);
}

/*****************************************************************************/
void AvrcpConnectTestNull(Task theAppTask)
{
    AvrcpConnectRequest(theAppTask, 0);
}


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/*****************************************************************************/
void AvrcpPassthroughTestExtra(AVRCP *avrcp, avc_subunit_type subunit_type,
                               avc_subunit_id subunit_id, bool state, 
                               avc_operation_id opid)
{
    Source data_source = StreamRegionSource(op_data, 5);
    AvrcpPassthroughRequest(avrcp, subunit_type, subunit_id, state, opid,
                     5, data_source);
}


/*****************************************************************************/
void AvrcpPassthroughVendorTestExtra(AVRCP *avrcp, 
                                    avc_subunit_type subunit_type, 
                                    avc_subunit_id subunit_id, bool state )
{
    Source data_source;
    avc_operation_id opid= opid_vendor_unique;
    uint8 *new_op_data = PanicUnlessMalloc(100);
    uint8 i, length=100;

    /* set some value instead of keeping junk */
    for(i=0; i< length; i++)
        new_op_data[i]=i;
    
    gdata = new_op_data;
    data_source = StreamRegionSource(new_op_data, length);
    cleanUpTask.handler = avrcpDataCleanUp;
    MessageStreamTaskFromSink(StreamSinkFromSource(data_source), &cleanUpTask);

    AvrcpPassthroughRequest(avrcp, subunit_type, subunit_id, state, opid, 
                     100, data_source);
}


/*****************************************************************************/
void AvrcpVendorDependentTestExtra(AVRCP *avrcp, 
                                   avc_subunit_type subunit_type,
                                   avc_subunit_id subunit_id,
                                   uint8 ctype, uint32 company_id)
{
    Source data_source = StreamRegionSource(op_data, 5);
    AvrcpVendorDependentRequest(avrcp, subunit_type, subunit_id, ctype, 
                        company_id, 5, data_source);
}

/*****************************************************************************/
void AvrcpPassthroughTestNull(AVRCP *avrcp, avc_subunit_type subunit_type,
                              avc_subunit_id subunit_id, bool state,
                              avc_operation_id opid)
{
    AvrcpPassthroughRequest(avrcp, subunit_type, subunit_id, state, opid, 0, 0);
}

/*****************************************************************************/
void AvrcpGetCurrentAppSettingValueTestExtra(AVRCP *avrcp,
                                             uint16 size_attributes,
                                             uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpGetAppValueRequest(avrcp, size_attributes, pdu_src);
}

/*****************************************************************************/
void AvrcpSetAppSettingValueTestExtra(AVRCP *avrcp,
                                      uint16 size_attributes,
                                      uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpSetAppValueRequest(avrcp, size_attributes, pdu_src);
}

/*****************************************************************************/
void AvrcpGetElementAttributesTestExtra(AVRCP *avrcp, 
                                        uint32 identifier_high,
                                        uint32 identifier_low, 
                                        uint16 size_attributes,
                                         uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpGetElementAttributesRequest(avrcp, identifier_high, identifier_low,
                              size_attributes, pdu_src);
}

/*****************************************************************************/

void AvrcpInformDisplayableCharacterSetTestExtra(AVRCP *avrcp, 
                                                uint16 size_attributes, 
                                                uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpInformCharacterSetRequest(avrcp, size_attributes, pdu_src);
}
/*****************************************************************************/

void AvrcpGetAppSettingAttributeTextTestExtra(AVRCP *avrcp, 
                                              uint16 size_attributes, 
                                              uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpGetAppAttributeTextRequest(avrcp, size_attributes, pdu_src);
}

/*****************************************************************************/
void AvrcpGetAppSettingValueTextTestExtra(AVRCP *avrcp, 
                                          uint16 attribute_id, 
                                          uint16 size_values, 
                                          uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(values, size_values);
    AvrcpGetAppValueTextRequest(avrcp, attribute_id, size_values, pdu_src);
}

/*****************************************************************************/
/****************************************************************************
*NAME 
*   AvrcpBrowseGetFolderItemsTest
*
*DESCRIPTION
*   Shim layer API for AvrcpBrowseGetFolderItems.  
*   It requests only 5 items. 
*
*PARAMETRS
*   avrcp                  - Task
*   avrcp_browse_scope     - Sope for media navigation.
*   num_attr               - 0 ...255 ( 255 for all attributes).
*RETURN
*   AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM
*****************************************************************************/

void AvrcpBrowseGetFolderItemsTest(AVRCP                *avrcp,  
                                   avrcp_browse_scope   scope)
{
    uint8 num_attr = 2;
    uint8 attr[] = {0, 0, 0, 1, 0, 0, 0, 2};
    Source pdu_src = avrcpSourceFromData(attr, 8);

    AvrcpBrowseGetFolderItemsRequest(avrcp, scope, 0, 10, num_attr, pdu_src);
}

/****************************************************************************
*NAME 
*   AvrcpBrowseGetItemAttributesTest
*
*DESCRIPTION
*   Shim layer API for AvrcpBrowseGetItemAttributes
*
*PARAMETRS
*   avrcp                  - Task
*   avrcp_browse_scope     - Scope
*   avrcp_browse_uid       - Browse UID of the item requested 
*   uint16                 - UID counter in TG FS
*   num_attr               - 0 ...255 ( 255 for all attributes).
*RETURN
*   AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM
*****************************************************************************/
void AvrcpBrowseGetItemAttributesTest(AVRCP*               avrcp, 
                                      avrcp_browse_scope   scope,  
                                      uint16               uid_counter)  
{
    avrcp_browse_uid uid;
    uint8 num_attr = 2;
    uint8 attr[] = {0, 0, 0, 1, 0, 0, 0, 2};
    Source pdu_src =  avrcpSourceFromData(attr, 8);
    uid.msb = 0x4578;
    uid.lsb = 0x3421;

    AvrcpBrowseGetItemAttributesRequest(avrcp,scope,uid,uid_counter,num_attr, pdu_src); 
}

/****************************************************************************
*NAME 
*   AvrcpBrowseSearchTest
*
*DESCRIPTION
*   Shim layer API for AvrcpBrowseSearch
*
*PARAMETRS
*   avrcp                  - Task
*   str_length             - string length
*   Source                 - Search String.    
*
*RETURN
*   AVRCP_BROWSE_SEARCH_CFM
*****************************************************************************/
void AvrcpBrowseSearchTest( AVRCP*          avrcp, 
                            uint16          str_length, 
                            uint8*          string) 
{
    Source pdu_src = avrcpSourceFromData(string, str_length);
    AvrcpBrowseSearchRequest(avrcp, avrcp_char_set_utf_8, str_length, pdu_src);
} 
 
/****************************************************************************
*NAME 
*   AvrcpBrowseChangePathTest
*
*DESCRIPTION
*   Shim layer API for AvrcpChagePath
*
*PARAMETRS
*   avrcp                  - Task
*   uid_counter            - counter
*   num_items              - number of items
*
*RETURN
*   AVRCP_BROWSE_CHANGE_PATH_CFM
*****************************************************************************/
void AvrcpBrowseChangePathTest(AVRCP*                  avrcp,  
                               uint16                  uid_counter,
                               avrcp_browse_direction  direction)
{
    avrcp_browse_uid folder_uid;
    folder_uid.msb = 0x4578;
    folder_uid.lsb = 0x3421;

    AvrcpBrowseChangePathRequest(avrcp, uid_counter, direction, folder_uid);

}

/****************************************************************************
*NAME 
*   AvrcpPlayItemTest
*
*DESCRIPTION
*   Shim layer for AvrcpPlayItem
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_browse_scope  - scope                
*  uint16               - uid_counter
*****************************************************************************/
void AvrcpPlayItemTest( AVRCP*              avrcp, 
                        avrcp_browse_scope  scope,    
                          uint16            uid_counter)
{
    avrcp_browse_uid uid;
    uid.msb = 0x4578;
    uid.lsb = 0x3421;

    AvrcpPlayItemRequest(avrcp, scope, uid, uid_counter);
}

/****************************************************************************
*NAME 
*   AvrcpAddToNowPlayingTest
*
*DESCRIPTION
*   Shim layer for AvrcpAddToNowPlaying
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_browse_scope  - scope                
*  uint16               - uid_counter
*****************************************************************************/
void AvrcpAddToNowPlayingTest( AVRCP*              avrcp, 
                               avrcp_browse_scope  scope,    
                               uint16            uid_counter)
{
    avrcp_browse_uid uid;
    uid.msb = 0x4578;
    uid.lsb = 0x3421;

    AvrcpAddToNowPlayingRequest(avrcp, scope, uid, uid_counter);
}
#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/*****************************************************************************/
void AvrcpGetCapabilitiesResponseTestExtra(AVRCP *avrcp, 
                                           avrcp_response_type response, 
                                           avrcp_capability_id caps,
                                           uint16 size_caps_list, 
                                           uint8* caps_list)
{
    Source pdu_src;

    pdu_src = avrcpSourceFromData(caps_list, size_caps_list);
    AvrcpGetCapsResponse(avrcp, response, caps,
                                 size_caps_list, pdu_src);
}


/*****************************************************************************/
void AvrcpListAppSettingAttributeResponseTestExtra(AVRCP *avrcp, 
                                                avrcp_response_type response,
                                                uint16 size_attributes,
                                                uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpListAppAttributeResponse(avrcp, response,
                                         size_attributes, pdu_src);
}


/*****************************************************************************/
void AvrcpListAppSettingValueResponseTestExtra(AVRCP *avrcp,
                                             avrcp_response_type response, 
                                             uint16 size_values, uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(values, size_values);
    AvrcpListAppValueResponse(avrcp, response, size_values, pdu_src);
}



/*****************************************************************************/
void AvrcpGetCurrentAppSettingValueResponseTestExtra(AVRCP *avrcp, 
                                            avrcp_response_type response, 
                                            uint16 size_values, uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(values, size_values);
    AvrcpGetAppValueResponse(avrcp, response,
                                          size_values, pdu_src);
}



/*****************************************************************************/
void AvrcpGetElementAttributesResponseTestExtra(AVRCP *avrcp, 
                                                avrcp_response_type response, 
                                                uint16 number_of_attributes,
                                                uint16 size_attributes,
                                                 uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpGetElementAttributesResponse(avrcp, response, 
                                      number_of_attributes, 
                                      size_attributes, pdu_src);
}

/*****************************************************************************/
void AvrcpGetElementAttributesFragmentedResponseTestExtra(
                                      AVRCP               *avrcp,
                                      avrcp_response_type response,
                                      uint16              number_of_attributes) 
{
    uint8* attr_data=NULL;
    Source pdu_src;
    uint16 attr_len;
    uint16 length= 510;

    UNUSED(number_of_attributes);

    /* Allocate Memory greater than 502 to test Meta data fragmentation */
    attr_data = (uint8*)PanicUnlessMalloc(length);

    /* calculate the size of attribute values */
    attr_len = (length - 8);

    /* Fill the attributes */
    attr_data[0] =0x0;
    attr_data[1] =0x0;
    attr_data[2] =0x0;
    attr_data[3] =0x1;
    attr_data[4] = 0x00;
    attr_data[5] = 0x6A; /* Character Set - UTF-8 */
    attr_data[6] = attr_len >> 8;
    attr_data[7] = attr_len & 0xFF;
  
    gdata = attr_data;

    pdu_src = StreamRegionSource(gdata, length);
    
    cleanUpTask.handler = avrcpDataCleanUp;

    MessageStreamTaskFromSink(StreamSinkFromSource(pdu_src), &cleanUpTask);

    AvrcpGetElementAttributesResponse(avrcp, response, 
                                      1, length , 
                                      pdu_src);
}

/*****************************************************************************/
void AvrcpEventPlayerAppSettingChangedResponseTestExtra(AVRCP *avrcp, 
                                                avrcp_response_type response, 
                                                uint16 size_attributes, 
                                                uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpEventPlayerAppSettingChangedResponse(avrcp, response, 
                                              size_attributes, pdu_src);
}
/*****************************************************************************/

void AvrcpGetAppSettingAttributeTextResponseTestExtra(AVRCP *avrcp, 
                                        avrcp_response_type response, 
                                        uint16 number_of_attributes, 
                                        uint16 size_attributes, 
                                        uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(attributes, size_attributes);
    AvrcpGetAppAttributeTextResponse(avrcp, response, 
                                            number_of_attributes, 
                                            size_attributes, pdu_src);
}
/*****************************************************************************/

void AvrcpGetAppSettingValueTextResponseTestExtra(AVRCP *avrcp, 
                                        avrcp_response_type response, 
                                        uint16 number_of_values, 
                                        uint16 size_values, 
                                        uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(values, size_values);
    AvrcpGetAppValueTextResponse(avrcp, response,
                                        number_of_values, 
                                        size_values, pdu_src);
}



/****************************************************************************
*NAME 
*   AvrcpBrowseSetPlayerResponseTest
*
*DESCRIPTION
*   Shim layer API for SetBrowsedPlayer 
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avctp_response_browsing_success on Success.
*   uint16             - uid counter 
*   uint32             - number of items in the folder
*****************************************************************************/
void AvrcpBrowseSetPlayerResponseTest(AVRCP*               avrcp, 
                                      avrcp_response_type  response,
                                      uint16               uid_counter,   
                                      uint32               num_items)
{
    if(response == avrcp_response_browsing_success)
    {
        uint8 folder[] = {0x01, 'A', 0x02, 'B', 'C' , 0x03, 'D', 'E', 'F'};
        Source pdu_src = avrcpSourceFromData(folder, 9);

        AvrcpBrowseSetPlayerResponse(avrcp, response, uid_counter, num_items, 
                                    avrcp_char_set_utf_8, 3, 9, pdu_src);
    }
    else
    {
        AvrcpBrowseSetPlayerResponse(avrcp, response, uid_counter, num_items, 
                                   avrcp_char_set_utf_8, 0, 0, 0);
    }
} 
/****************************************************************************
*NAME 
*   AvrcpBrowseGetFolderItemsResponseTest
*
*DESCRIPTION
*   Shim API for AvrcpBrowseGetFolderItemsResponse. It always returns 1 item
*   for media player, 2 item for folder items and 3 item for media element 
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avctp_response_browsing_success on Success.
*   uint16             - UID Counter. 0 for non database aware players
*   uint8 - Item_type  - 1 for Media player , 2 for file systam and any other
*                        value for media element item
*****************************************************************************/
void AvrcpBrowseGetFolderItemsResponseTest(AVRCP*               avrcp,
                                           avrcp_response_type  response, 
                                           uint16               uid_counter,
                                           uint8                item_type)
{
    Source src_pdu=0;
    uint16 num_items=0;
    uint16 item_list_size=0;
    uint8 *item=NULL;

    if(response != avrcp_response_browsing_success)
    {
       AvrcpBrowseGetFolderItemsResponse(avrcp, response, 0, 0, 0, 0); 
        return;
    }
    
   if(item_type == 1)
   {
        item = PanicUnlessMalloc(37);
        memset(item, 0, 37);
        item[0] = 0x01;
        item[2] = 34;
        item[3] = 0x01;
        item[4] = 0x10;
        item[5] = 0x03;
        item[9] = 0x01;
        memset(&item[11], 0xFF, 8);
        item[19] = 0x03;
        item[30] = 0x6A;
        item[32] = 0x04;
        strncpy((char*)&item[33], "CSR", 4);
        item_list_size = 37;
        num_items = 1;
    }
    else if(item_type == 2)
    {
        item = PanicUnlessMalloc(40);
        memset(item, 0, 40);
        item[0] = 0x02;
        item[2] = 0x11;
        memset(&item[3], 0x02, 8);
        item[10] = 0x01;
        item[11] = 0x03;
        item[12] = 0x01;
        item[14] = 0x6A;
        item[16] = 0x03;
        strncpy((char*)&item[17], "MJ", 3);
        memcpy(&item[20],&item[0], 20);
        item[30] = 0x02;
        item[31] = 0x05;
        strncpy((char*)&item[37], "MH", 3); 

        item_list_size = 40;
        num_items = 2;
    }
    else
    {
        item = PanicUnlessMalloc(126);
        item[0] = 0x03;
        item[2] = 0x27;
        memset(&item[3], 0x03, 8);
        item[10] = 0x01;
        item[11] = 0x01;
        item[13] = 0x6A;
        item[15] = 0x03;
        strncpy((char*)&item[16], "MJ", 3);
        item[19] = 0x02;
        item[23] = 0x01;
        item[25] = 0x6A;
        item[27] = 0x03;
        strncpy((char*)&item[28], "ha", 3);
        item[34] = 0x02;
        item[36] = 0x6A;
        item[38] = 0x03;
        strncpy((char*)&item[39], "MJ", 3);
        memcpy(&item[42], &item[0], 42);
        item[52] = 0x02;
        item[53] = 0x00;
        strncpy((char*)&item[58], "OS", 3);
        strncpy((char*)&item[70], "Si", 3);
        strncpy((char*)&item[81], "PJ", 3);
        memcpy(&item[84], &item[0], 42);
        item[94] = 0x03;
        strncpy((char*)&item[100], "TM", 3);
        strncpy((char*)&item[112], "Pa", 3);
        strncpy((char*)&item[123], "MJ", 3);
                       
        item_list_size = 126;
        num_items = 3;
    }

    src_pdu = StreamRegionSource(item, item_list_size);
    gdata = item;
    cleanUpTask.handler = avrcpDataCleanUp;
    MessageStreamTaskFromSink(StreamSinkFromSource(src_pdu), &cleanUpTask);

    AvrcpBrowseGetFolderItemsResponse(avrcp, response, uid_counter,num_items,
                                       item_list_size, src_pdu);  


} 


/****************************************************************************
*NAME 
*   AvrcpBrowseGetItemAttributesResponse
*
*DESCRIPTION
*   Shim layer for AvrcpBrowseGetItemAttributes
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avctp_response_browsing_success on Success.
*****************************************************************************/
void AvrcpBrowseGetItemAttributesResponseTest(  AVRCP*               avrcp, 
                                                avrcp_response_type  response)
{
    Source  src_pdu=0;
    uint16  size_attr_list=0;
    uint16  num_attributes=0;
    uint8 *item = NULL;

    if(response == avrcp_response_browsing_success)
    {
       item= PanicUnlessMalloc(25);
       memset(item, 0, 25);
       item[3] = 0x01;
       item[5] = 0x6A;
       item[7] = 0x05;
       strncpy((char*)&item[8],"SaRi", 5);
       item[16] = 0x02;
       item[18] = 0x6A;
       item[20] = 0x04;
       strncpy((char*)&item[21],"OMH", 4);       
        num_attributes = 2;
        size_attr_list =  25;
        src_pdu = StreamRegionSource(item, size_attr_list);
        gdata = item;
        cleanUpTask.handler = avrcpDataCleanUp;
        MessageStreamTaskFromSink(StreamSinkFromSource(src_pdu), &cleanUpTask);
    }
    AvrcpBrowseGetItemAttributesResponse(avrcp, response, num_attributes, 
                                         size_attr_list, src_pdu);

 
}

#endif /* !AVRCP_CT_ONLY_LIB*/



