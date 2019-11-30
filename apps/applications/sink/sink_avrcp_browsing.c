/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
\file    
\ingroup sink_app
\brief   
    The connection of the browsing channel will always be performed before sending a command using the Request functions in this file,
    so it is not neccessary to call sinkAvrcpBrowsingChannelConnectRequest directly.
    
    The disconnection of the browsing channel will always be made when the AVRCP control channel is disconnected. The browsing channel can
    also be removed after a period of inactivity by calling sinkAvrcpBrowsingDisconnectOnIdleRequest, where disconnection will occur
    if no commands are sent within the AVRCP_BROWSING_DISCONNECT_ON_IDLE_TIMER timeout.
    
    The application should use the Request functions in this file to send Browsing commands and the Cfm and Ind functions will handle the
    responses. Code will need to be added for display capable devices in order to display the information returned over the AVRCP browsing channel. 
    Most of the code for registering for notication of changes in TG device has already be added, although some functionality like registering for
    changes in battery status and system status has not been added.

*/
#ifdef ENABLE_AVRCP
    
#include "sink_avrcp_browsing.h"
#include "sink_avrcp_browsing_channel.h"
#include "sink_avrcp.h"
    

#ifdef DEBUG_AVRCP_BROWSING
#define AVRCP_BROWSING_DEBUG(x) DEBUG(x)
#else
#define AVRCP_BROWSING_DEBUG(x) 
#endif

#ifdef ENABLE_AVRCP_BROWSING

#include <source.h>

/*************************************************************************
NAME    
    sinkAvrcpBrowsingInit
    
DESCRIPTION
    Handles initialization of AVRCP browsing variables.

**************************************************************************/
void sinkAvrcpBrowsingInit(uint16 link_index)
{
    TaskData task;
    sinkAvrcpSetBrowsingUIDCounter(link_index,0);
    sinkAvrcpSetBrowsingMediaPlayerFeatures(link_index,AVRCP_BROWSING_PLAYER_FEATURES_INVALID);
    sinkAvrcpSetBrowsingMediaPlayerId(link_index,0);
    sinkAvrcpSetBrowsingScope(link_index,0);
    task.handler = avrcpBrowsingHandler;
    sinkAvrcpUpdateBrowsingHandler(link_index,&task);
    MessageFlushTask(sinkAvrcpGetBrowsingHandler(link_index));
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingIsSupported
    
DESCRIPTION
    Checks that both ends support Browsing.

RETURNS
    Returns TRUE if Browsing supported, FALSE otherwise.
**************************************************************************/
bool sinkAvrcpBrowsingIsSupported(uint16 Index)
{
    if ((sinkAvrcpGetExtensions(Index) & AVRCP_BROWSING_SUPPORTED) == AVRCP_BROWSING_SUPPORTED)
    {
        if (sinkAvrcpGetBrowsingMediaPlayerFeatures(Index) != AVRCP_BROWSING_PLAYER_FEATURES_INVALID)
        {
            if (sinkAvrcpGetBrowsingMediaPlayerFeatures(Index) & AVRCP_BROWSING_PLAYER_FEATURE_O7_BROWSING)
                return TRUE;
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingIsGroupNavigationSupported
    
DESCRIPTION
    Checks Group Navigation is supported.

RETURNS
    Returns TRUE if supported, FALSE otherwise.
**************************************************************************/
bool sinkAvrcpBrowsingIsGroupNavigationSupported(uint16 Index)
{
    if(Index < MAX_AVRCP_CONNECTIONS)
    {
        if (sinkAvrcpGetBrowsingMediaPlayerFeatures(Index) != AVRCP_BROWSING_PLAYER_FEATURES_INVALID)
        {
            if (sinkAvrcpGetBrowsingMediaPlayerFeatures(Index) & AVRCP_BROWSING_PLAYER_FEATURE_O7_GROUP_NAVIGATION)
                return TRUE;
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
NAME    
    avrcpBrowseMediaPlayerItem
    
DESCRIPTION
    Handles the GetFolderItems media player item data.

**************************************************************************/
static void avrcpBrowseMediaPlayerItem(uint16 Index, uint16 max_length, uint16 item_length, const uint8 *item)
{
    uint16 player_id = 0;
    uint8 major_player_type = 0;
    uint32 player_sub_type = 0;
    uint8 play_status = 0;
    uint16 charset_id = 0;
    uint16 displayable_name_length = 0;
    uint16 media_player_features = 0;
    /* check that the item array elements are accessible */
    if (max_length < AVRCP_BROWSE_MEDIA_PLAYER_ITEM_HEADER_SIZE)
        return;
        
    player_id = (item[0] << 8) | item[1];
    major_player_type = item[2];
    player_sub_type = ((uint32)item[3] << 24) | ((uint32)item[4] << 16) | ((uint32)item[5] << 8) | item[6];
    play_status = item[7];
    /* item[8] to item[23] is feature bit mask */
    charset_id = (item[24] << 8) | item[25];
    displayable_name_length = (item[26] << 8) | item[27];
            
    if (displayable_name_length > (max_length - AVRCP_BROWSE_MEDIA_PLAYER_ITEM_HEADER_SIZE))
        displayable_name_length = max_length - AVRCP_BROWSE_MEDIA_PLAYER_ITEM_HEADER_SIZE;
            
    if (player_id == sinkAvrcpGetBrowsingMediaPlayerId(Index))
    {
        /* this is the player that is currently selected so store the relevant player feature bits.
            Only store octets 7 and 8 of Player Feature Bits as the rest only states which PassThrough
            commands are supported, the device will send these anyway regardless */
            media_player_features = (item[16] << 8) | item[15]; /* store octets 7 and 8 of Player Feature Bitmask */
            sinkAvrcpSetBrowsingMediaPlayerFeatures(Index,media_player_features);
            AVRCP_BROWSING_DEBUG(("       stored player features[0x%x]\n",sinkAvrcpGetBrowsingMediaPlayerFeatures(Index))); 
    }
        
    AVRCP_BROWSING_DEBUG(("       player_id[%d] major_player_type[%d] player_sub_type[%ld] play_status[%d] charset_id[0x%x]\n",
                                  player_id, major_player_type, player_sub_type, play_status, charset_id));
    /* item[28] to item[28+displayable_name_length-1] is displayable name */
#ifdef DEBUG_AVRCP_BROWSING    
    {
        uint16 i;
        AVRCP_BROWSING_DEBUG(("        player name : "));
        for (i = 0; i < displayable_name_length; i++)
        {                    
            AVRCP_BROWSING_DEBUG(("%c", item[i + 28]));                    
        }
        AVRCP_BROWSING_DEBUG(("\n"));
     }
#endif
    
    /* TODO display player with ID of player_id and displayable name starting at item[28] with name length of displayable_name_length */    
}

/*************************************************************************
NAME    
    avrcpBrowseMediaElementItem
    
DESCRIPTION
    Handles the GetFolderItems media element item data.

**************************************************************************/
static void avrcpBrowseMediaElementItem(uint16 max_length, uint16 item_length, const uint8 *item)
{
    avrcp_browse_uid uid;    
    uint8 media_type;
    uint16 charset_id;
    uint16 displayable_name_length;
    uint8 number_attributes = 0;
    uint32 attribute_id;
    uint16 attribute_value_length;   
    uint16 index;
    uint16 i;
    
    /* check that the item array elements are accessible */
    if (max_length < AVRCP_BROWSE_MEDIA_ELEMENT_ITEM_HEADER_SIZE)
        return;
    
    uid.msb = ((uint32)item[0] << 24) | ((uint32)item[1] << 16) | ((uint32)item[2] << 8) | item[3];
    uid.lsb = ((uint32)item[4] << 24) | ((uint32)item[5] << 16) | ((uint32)item[6] << 8) | item[7];
    media_type = item[8];
    charset_id = (item[9] << 8) | item[10];
    displayable_name_length = (item[11] << 8) | item[12];
    
    AVRCP_BROWSING_DEBUG(("        uid_msb[0x%lx] uid_lsb[0x%lx] media_type[0x%x] charset[0x%x] name_length[0x%x]\n", uid.msb, uid.lsb, media_type, charset_id, displayable_name_length));
    
    /* if displayable name length is too long then discard this data as corrupt */
    if (displayable_name_length > (max_length - AVRCP_BROWSE_MEDIA_ELEMENT_ITEM_HEADER_SIZE))
        return;
    
    /* item[13] to item[13+displayable_name_length-1] is displayable name */
#ifdef DEBUG_AVRCP_BROWSING    
    {     
        uint16 j;
        AVRCP_BROWSING_DEBUG(("        displayable name : "));
        for (j = 0; j < displayable_name_length; j++)
        {                    
            AVRCP_BROWSING_DEBUG(("%c", item[j + 13]));                    
        }
        AVRCP_BROWSING_DEBUG(("\n"));
     }
#endif    
    
    if (max_length > (13 + displayable_name_length))
        number_attributes = item[13 + displayable_name_length];
    
    index = 13 + displayable_name_length + 1;
    max_length = max_length - index;
    
    AVRCP_BROWSING_DEBUG(("        number_attributes[0x%x]\n", number_attributes));
    
    for (i = 0; i < number_attributes; i++)
    {
        AVRCP_BROWSING_DEBUG(("        index[%d] max_length[%d]\n", index, max_length));
        if (max_length < AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE)
            break;
        attribute_id = ((uint32)item[index] << 24) | ((uint32)item[index + 1] << 16) | ((uint32)item[index + 2] << 8) | item[index + 3];
        charset_id = (item[index + 4] << 8) | item[index + 5];
        attribute_value_length = (item[index + 6] << 8) | item[index + 7];
        
        AVRCP_BROWSING_DEBUG(("        attr %d : attribute_id[0x%lx] charset[0x%x] attribute_value_length[0x%x]\n", i, attribute_id, charset_id, attribute_value_length));
        
        if (attribute_value_length > (max_length - AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE))
        {
            attribute_value_length = max_length - AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE;
            AVRCP_BROWSING_DEBUG(("        correct attr_length[0x%x] max_length[0x%x]\n", attribute_value_length, max_length));
        }
        
        /* item[index + 8] to item[index + 8 + attribute_value_length - 1] is attribute value */
        sinkAvrcpDisplayMediaAttributes(attribute_id, attribute_value_length, &item[index + 8]); /* display for debug purposes */
                
        max_length = max_length - AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE - attribute_value_length;
        index = index + AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE + attribute_value_length;
    }         
    
    {
        uint16 Index = sinkAvrcpGetActiveConnection();
        /* just display displayable name for menu */
        if (sinkAvrcpGetBrowsingScope(Index) == SCOPE_NON_ZERO(avrcp_virtual_filesystem_scope))
        {
            /* TODO display track in virtual filesystem with displayable name starting at item[13] and of length displayable_name_length */            
        }
        else if (sinkAvrcpGetBrowsingScope(Index)  == SCOPE_NON_ZERO(avrcp_now_playing_scope))
        {
            /* TODO display Now Playing track with displayable name starting at item[13] and of length displayable_name_length */            
        }
    }
}

/*************************************************************************
NAME    
    avrcpBrowseFolderItem
    
DESCRIPTION
    Handles the GetFolderItems folder item data.

**************************************************************************/
static void avrcpBrowseFolderItem(uint16 Index, uint16 max_length, uint16 item_length, const uint8 *item)
{
    avrcp_browse_uid uid;
    uint8 folder_type;
    uint8 is_playable;
    uint16 charset_id;
    uint16 displayable_name_length = 0;
    
    /* check that the item array elements are accessible */
    if (max_length < AVRCP_BROWSE_FOLDER_ITEM_HEADER_SIZE)
        return;
        
    uid.msb = ((uint32)item[0] << 24) | ((uint32)item[1] << 16) | ((uint32)item[2] << 8) | item[3];
    uid.lsb = ((uint32)item[4] << 24) | ((uint32)item[5] << 16) | ((uint32)item[6] << 8) | item[7];
    folder_type = item[8];
    is_playable = item[9];
    charset_id = (item[10] << 8) | item[11];
    displayable_name_length = (item[12] << 8) | item[13];
            
    if (displayable_name_length > (max_length - AVRCP_BROWSE_FOLDER_ITEM_HEADER_SIZE))
        displayable_name_length = max_length - AVRCP_BROWSE_FOLDER_ITEM_HEADER_SIZE;
            
    AVRCP_BROWSING_DEBUG(("        uid_msb[0x%lx] uid_lsb[0x%lx] folder_type[0x%x] is_playable[0x%x] charset[0x%x] name_length[0x%x]\n", uid.msb, uid.lsb, folder_type, is_playable, charset_id, displayable_name_length));
    
    /* item[28] to item[28+displayable_name_length-1] is displayable name */
#ifdef DEBUG_AVRCP_BROWSING    
    {
        uint16 i;
        AVRCP_BROWSING_DEBUG(("        media name : "));
        for (i = 0; i < displayable_name_length; i++)
        {                    
            AVRCP_BROWSING_DEBUG(("%c", item[i + 14]));                    
        }
        AVRCP_BROWSING_DEBUG(("\n"));
     }
#endif
    
    /* TODO display folder with displayable name starting at item[14] and of length displayable_name_length - is_playable states if this folder is playable */
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingPlayItem
    
DESCRIPTION
    Plays the item with the specified UID and folder scope.

**************************************************************************/
static bool sinkAvrcpBrowsingPlayItem(avrcp_browse_uid uid, avrcp_browse_scope scope)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Play Item scope[0x%x] uid[0x%lx 0x%lx]\n", scope, uid.msb, uid.lsb));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {  
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);

                AvrcpPlayItemRequest(sinkAvrcpGetProfileInstance(Index),
                                        scope,
                                        uid,  
                                         sinkAvrcpGetBrowsingUIDCounter(Index));                                
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_PLAY_ITEM);                
                message->uid = uid;
                message->scope = scope;    
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_PLAY_ITEM, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingAddToNowPlaying
    
DESCRIPTION
    Adds the item with the specified UID and folder scope to the Now Playing folder.

**************************************************************************/
static bool sinkAvrcpBrowsingAddToNowPlaying(avrcp_browse_uid uid, avrcp_browse_scope scope)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Add To Now Playing scope[0x%x] uid[0x%lx 0x%lx]\n", scope, uid.msb, uid.lsb));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                AvrcpAddToNowPlayingRequest(sinkAvrcpGetProfileInstance(Index),
                                            scope,
                                            uid,  
                                            sinkAvrcpGetBrowsingUIDCounter(Index));
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_ADD_TO_NOW_PLAYING);                
                message->uid = uid;
                message->scope = scope;    
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_ADD_TO_NOW_PLAYING, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingFlushHandlerTask
    
DESCRIPTION
    Flushes any pending messages from the handler.

**************************************************************************/
void sinkAvrcpBrowsingFlushHandlerTask(uint16 index)
{
    if(index < MAX_AVRCP_CONNECTIONS)
        MessageFlushTask(sinkAvrcpGetBrowsingHandler(index));
}

/*************************************************************************
NAME    
    avrcpBrowsingHandler
    
DESCRIPTION
    Handles all application created AVRCP Browsing messages.

**************************************************************************/
void avrcpBrowsingHandler(Task task, MessageId id, Message message)
{    
    switch (id)
    {
        case AVRCP_BROWSING_NOW_PLAYING_TRACK:     
        {
            sinkAvrcpBrowsingRetrieveNowPlayingTrackRequest(((AVRCP_BROWSING_NOW_PLAYING_TRACK_T *)message)->index,
                                                                ((AVRCP_BROWSING_NOW_PLAYING_TRACK_T *)message)->track_index_high,
                                                                ((AVRCP_BROWSING_NOW_PLAYING_TRACK_T *)message)->track_index_low,
                                                                ((AVRCP_BROWSING_NOW_PLAYING_TRACK_T *)message)->full_attributes);
            break;
        }
        case AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS:
        {
            sinkAvrcpBrowsingRetrieveMediaPlayersRequest(((AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS_T *)message)->start_index,
                                                            ((AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS_T *)message)->end_index);
            break;
        }
        case AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST:
        {
            sinkAvrcpBrowsingRetrieveNowPlayingListRequest(((AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST_T *)message)->start_index,
                                                              ((AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST_T *)message)->end_index);
            break;
        }
        case AVRCP_BROWSING_PLAY_ITEM:
        {
            sinkAvrcpBrowsingPlayItem(((AVRCP_BROWSING_PLAY_ITEM_T *)message)->uid,
                                         ((AVRCP_BROWSING_PLAY_ITEM_T *)message)->scope);
            break;
        }
        case AVRCP_BROWSING_RETRIEVE_FILESYSTEM:
        {
            sinkAvrcpBrowsingRetrieveFileSystemRequest(((AVRCP_BROWSING_RETRIEVE_FILESYSTEM_T *)message)->start_index,
                                                          ((AVRCP_BROWSING_RETRIEVE_FILESYSTEM_T *)message)->end_index);
            break;
        }
        case AVRCP_BROWSING_CHANGE_PATH:
        {
            sinkAvrcpBrowsingChangePathRequest(((AVRCP_BROWSING_CHANGE_PATH_T *)message)->direction, 
                                                  ((AVRCP_BROWSING_CHANGE_PATH_T *)message)->folder_uid);
            break;
        }
        case AVRCP_BROWSING_SET_PLAYER:
        {
            sinkAvrcpBrowsingSetBrowsedMediaPlayerRequest(((AVRCP_BROWSING_SET_PLAYER_T *)message)->player_id);
            break;
        }
        case AVRCP_BROWSING_SEARCH:
        {
            sinkAvrcpBrowsingSearchRequest(((AVRCP_BROWSING_SEARCH_T *)message)->size_string,
                                              ((AVRCP_BROWSING_SEARCH_T *)message)->string);
            break;
        }
        case AVRCP_BROWSING_RETRIEVE_SEARCH_LIST:
        {
            sinkAvrcpBrowsingRetrieveSearchItemsRequest(((AVRCP_BROWSING_RETRIEVE_SEARCH_LIST_T *)message)->start_index,
                                                            ((AVRCP_BROWSING_RETRIEVE_SEARCH_LIST_T *)message)->end_index);
            break;
        }
        case AVRCP_BROWSING_DISCONNECT_IDLE:
        {
            sinkAvrcpBrowsingChannelDisconnectRequest(((AVRCP_BROWSING_DISCONNECT_IDLE_T *)message)->avrcp);
            break;
        }
        case AVRCP_BROWSING_RETRIEVE_NUMBER_OF_ITEMS:
        {
            sinkAvrcpBrowsingRetrieveNumberOfItemsRequest(((AVRCP_BROWSING_RETRIEVE_NUMBER_OF_ITEMS_T *)message)->scope);
            break;
        }
        default:
        {
            break;
        }        
    }
}
    
/*************************************************************************
NAME    
    sinkAvrcpBrowsingDisconnectOnIdleRequest
    
DESCRIPTION
    Disconnects the AVRCP Browsing Channel when the link becomes idle, so that it is not connected when not in use.
    
**************************************************************************/
void sinkAvrcpBrowsingDisconnectOnIdleRequest(AVRCP *avrcp)
{
    uint16 Index;
    
    if (sinkAvrcpBrowsingChannelGetIndexFromInstance(avrcp, &Index))
    {
        if (sinkAvrcpBrowsingChannelIsConnected(Index))
        {
            MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_DISCONNECT_IDLE);            
            message->avrcp = avrcp;            
            MessageSendLater(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_DISCONNECT_IDLE, message, AVRCP_BROWSING_DISCONNECT_ON_IDLE_TIMER);
            
            AVRCP_BROWSING_DEBUG(("AVRCP Browsing Disconnect On Idle Request\n"));        
        }
    }
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingCancelDisconnectOnIdle
    
DESCRIPTION
    Cancels any pending disconnection on idle request.
    
**************************************************************************/
void sinkAvrcpBrowsingCancelDisconnectOnIdle(uint16 index)
{
    MessageCancelAll(sinkAvrcpGetBrowsingHandler(index), AVRCP_BROWSING_DISCONNECT_IDLE);
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingRetrieveNowPlayingTrackRequest
    
DESCRIPTION
    Retrieve Now Playing Track using the Browsing Channel.
    The function sinkAvrcpBrowsingGetItemAttributesCfm will be called when a response to this request is received.

        Index - AVRCP instance to use, usually the active one 
        track_index_high - upper 32 bits of unique track UID
        track_index_high - lower 32 bits of unique track UID
        full_attributes - TRUE if all media attributes should be returned, FALSE if only the basic media attributes should be returned

**************************************************************************/
void sinkAvrcpBrowsingRetrieveNowPlayingTrackRequest(uint16 Index, uint32 track_index_high, uint32 track_index_low, bool full_attributes)
{
    uint16 size_media_attributes;
    uint16 number_attributes;
    Source src_media_attributes;
    avrcp_browse_uid uid;
    
    if (full_attributes)
    {
        size_media_attributes = sizeof(avrcp_retrieve_media_attributes_full);
        src_media_attributes = StreamRegionSource(avrcp_retrieve_media_attributes_full, size_media_attributes);
        number_attributes = AVRCP_NUMBER_MEDIA_ATTRIBUTES_FULL;
    }
    else
    {
        size_media_attributes = sizeof(avrcp_retrieve_media_attributes_basic);
        src_media_attributes = StreamRegionSource(avrcp_retrieve_media_attributes_basic, size_media_attributes);
        number_attributes = AVRCP_NUMBER_MEDIA_ATTRIBUTES_BASIC;
    }
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Get Now Playing Attributes track[0x%lx 0x%lx]\n", track_index_high, track_index_low));
    
    if (sinkAvrcpBrowsingChannelIsConnected(Index))
    {
        uid.msb = track_index_high;
        uid.lsb = track_index_low;
        
        sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
            
        AvrcpBrowseGetItemAttributesRequest(sinkAvrcpGetProfileInstance(Index),
                                            avrcp_now_playing_scope,  
                                            uid,   
                                            sinkAvrcpGetBrowsingUIDCounter(Index), 
                                            number_attributes, 
                                            src_media_attributes);
    }
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_NOW_PLAYING_TRACK);        
        message->index = Index;
        message->track_index_high = track_index_high;
        message->track_index_low = track_index_low;
        message->full_attributes = full_attributes;
        sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
        sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_NOW_PLAYING_TRACK, message, Index);
    }
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingRetrieveNowPlayingListRequest
    
DESCRIPTION
    Returns the Now Playing List using the Browsing Channel.
    The function sinkAvrcpBrowsingGetFolderItemsCfm will be called when a response to this request is received.    
    
        start_index - the index of the first Now Playing item to return
        end_index - the index of the last Now Playing item to return

**************************************************************************/
bool sinkAvrcpBrowsingRetrieveNowPlayingListRequest(uint16 start_index, uint16 end_index)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Retrieve Now Playing List start[0x%x] end [0x%x]\n", start_index, end_index));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                if (sinkAvrcpGetBrowsingScope(Index) )
                {
                    MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST);                    
                    message->start_index = start_index;
                    message->end_index = end_index;                        
                    MessageSendConditionally(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST, message, sinkAvrcpBrowsingScopeBusy(Index));        
                }
                else
                {
                    uint16 size_media_attributes = sizeof(avrcp_retrieve_media_attributes_basic);
                    Source src_media_attributes = StreamRegionSource(avrcp_retrieve_media_attributes_basic, size_media_attributes);
                    sinkAvrcpSetBrowsingScope(Index, SCOPE_NON_ZERO(avrcp_now_playing_scope));                  
                    /* register to receive notifications of now playing content changes */
                    AvrcpRegisterNotificationRequest(sinkAvrcpGetProfileInstance(Index), avrcp_event_now_playing_content_changed, 0);
                            
                    /* retrieve now playing list */
                    AvrcpBrowseGetFolderItemsRequest(sinkAvrcpGetProfileInstance(Index),   
                                                        avrcp_now_playing_scope,   
                                                        start_index,   
                                                        end_index,      
                                                        AVRCP_NUMBER_MEDIA_ATTRIBUTES_BASIC, 
                                                        src_media_attributes);
                }
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST);
                message->start_index = start_index;
                message->end_index = end_index;    
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingPlayNowPlayingItemRequest
    
DESCRIPTION
    Plays the track from the Now Playing List, identified by the unique 8 octet UID.
    The AVRCP_PLAY_ITEM_CFM message will be returned from the AVRCP library as a response to this request.

**************************************************************************/
bool sinkAvrcpBrowsingPlayNowPlayingItemRequest(avrcp_browse_uid uid)
{
    return sinkAvrcpBrowsingPlayItem(uid, avrcp_now_playing_scope);
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingGetItemAttributesCfm
    
DESCRIPTION
    Returns the Now Playing Track using the Browsing Channel 

**************************************************************************/
void sinkAvrcpBrowsingGetItemAttributesCfm(const AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_T *msg)
{
    if (msg->status == avrcp_success)
    {
        uint16 i = 0;
        uint32 attribute_id = 0;
        uint16 charset_id = 0;
        uint16 attribute_length = 0;
        uint16 header_end = 0;
        const uint8 *lSource = SourceMap(msg->attr_value_list);
        uint16 source_size = SourceSize(msg->attr_value_list);
        uint16 size_attr_list = (source_size < msg->size_attr_list) ? source_size : msg->size_attr_list;
        
        if (lSource)
        {
            AVRCP_BROWSING_DEBUG(("   success; num_attr[%d] size_att[%d] size_source[%d]\n", msg->num_attributes, msg->size_attr_list, source_size));                
            
            header_end = i + AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE;
            while (size_attr_list >= header_end) /* check there is room left for data to exist */
            {         
                attribute_id = ((uint32)lSource[i] << 24) | ((uint32)lSource[i + 1] << 16) | ((uint32)lSource[i + 2] << 8) | lSource[i + 3];
                charset_id = (lSource[i + 4] << 8) | lSource[i + 5];
                attribute_length = (lSource[i + 6] << 8) | lSource[i + 7];
                
                AVRCP_BROWSING_DEBUG(("        attribute = 0x%lx\n", attribute_id));
                AVRCP_BROWSING_DEBUG(("        charset_id = 0x%x\n", charset_id));
                AVRCP_BROWSING_DEBUG(("        attribute_length = 0x%x\n", attribute_length));
                        
                if ((attribute_length + header_end) > size_attr_list)
                    attribute_length = size_attr_list - header_end;
                
#ifdef DEBUG_AVRCP    
                {
                    uint16 j;
                    AVRCP_BROWSING_DEBUG(("        attribute text : "));
                    for (j = i; j < (i + attribute_length); j++)
                    {                    
                        AVRCP_BROWSING_DEBUG(("%c", lSource[j + 8]));                    
                    }
                        AVRCP_BROWSING_DEBUG(("\n"));
                }
#endif                               
                
                AVRCP_BROWSING_DEBUG(("AVRCP NOW PLAYING:\n"));
                
#ifdef ENABLE_AVRCP_NOW_PLAYING            
                if (attribute_length)
                    sinkAvrcpDisplayMediaAttributes(attribute_id, attribute_length, &lSource[i + 8]);
#endif

                i = header_end + attribute_length;
                header_end = i + AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE;
            }                    
        }
        else
        {
            AVRCP_BROWSING_DEBUG(("   fail; no source\n"));
        }
    }
    else
    {
        AVRCP_BROWSING_DEBUG(("   fail; status %d\n", msg->status));
    }
    
    /* Send Idle Disconnect of Browsing Channel request, as browsing channel may not be used again for some time */
    sinkAvrcpBrowsingDisconnectOnIdleRequest(msg->avrcp);
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingMultipleMediaPlayersIsSupported
    
DESCRIPTION
    Checks that both ends support Multiple Media Players.

RETURNS
    Returns TRUE if Multiple Media Players supported, FALSE otherwise.

**************************************************************************/
bool sinkAvrcpBrowsingMultipleMediaPlayersIsSupported(uint16 Index)
{
    if ((sinkAvrcpGetFeatures(Index) & AVRCP_MULTIPLE_MEDIA_PLAYERS) == AVRCP_MULTIPLE_MEDIA_PLAYERS)
        return TRUE;

    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingRetrieveMediaPlayersRequest
    
DESCRIPTION
    Retrives the available media players on the remote device.
    The function sinkAvrcpBrowsingGetFolderItemsCfm will be called when a response to this request is received.
    
        start_index - the index of the first Media Player to return
        end_index - the index of the last Media Player to return

**************************************************************************/
bool sinkAvrcpBrowsingRetrieveMediaPlayersRequest(uint16 start_index, uint16 end_index)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Retrieve Media Players start[0x%x] end [0x%x]\n", start_index, end_index));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                if (sinkAvrcpGetBrowsingScope(Index))
                {
                    /* as the confirmation messages don't return the scope, can only send one scope message at a time */
                    MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS);                    
                    message->start_index = start_index;
                    message->end_index = end_index;                                   
                    MessageSendConditionally(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS, message,sinkAvrcpBrowsingScopeBusy(Index));        
                }
                else
                {
                    /* send library command now */
                     sinkAvrcpSetBrowsingScope(Index, SCOPE_NON_ZERO(avrcp_media_player_scope));                  
                 
                    /* get notification of changes in media players */
                    AvrcpRegisterNotificationRequest(sinkAvrcpGetProfileInstance(Index), avrcp_event_available_players_changed, 0);
                    
                    /* retrieve all media players */
                    AvrcpBrowseGetFolderItemsRequest(sinkAvrcpGetProfileInstance(Index),   
                                                        avrcp_media_player_scope,   
                                                        start_index,   
                                                        end_index,      
                                                        0, 
                                                        0);
                }
            }
            else
            {            
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS);
                message->start_index = start_index;
                message->end_index = end_index;               
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingGetFolderItemsCfm
    
DESCRIPTION
    Handle AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM AVRCP library message

**************************************************************************/
void sinkAvrcpBrowsingGetFolderItemsCfm(const AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T *msg)
{
    uint16 Index;
    avrcp_status_code avrcp_status = msg->status;
    
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {   
        uint16 i = 0;
        uint16 header_end = 0;
        uint8 item_type = 0;
        uint16 item_length = 0;       
        const uint8 *lSource = SourceMap(msg->item_list);
        uint16 source_size = SourceSize(msg->item_list);
        uint16 item_list_size = (source_size < msg->item_list_size) ? source_size : msg->item_list_size;
        
        if (lSource)
        {
            AVRCP_BROWSING_DEBUG(("   success; uid_counter[%d] num_items[%d] item_list_size[%d] source_size[%d]\n", msg->uid_counter, msg->num_items, msg->item_list_size, SourceSize(msg->item_list)));
        
            switch (sinkAvrcpGetBrowsingScope(Index))
            {
                case SCOPE_NON_ZERO(avrcp_media_player_scope):
                {
                    AVRCP_BROWSING_DEBUG(("Scope: Media Player start\n"));
                    /* TODO start of media player display - msg->num_items returned */
                    break;
                }
                case SCOPE_NON_ZERO(avrcp_now_playing_scope):
                {
                    AVRCP_BROWSING_DEBUG(("Scope: Now Playing start\n"));
                    /* TODO start of now playing display - msg->num_items returned */
                    break;
                }
                case SCOPE_NON_ZERO(avrcp_virtual_filesystem_scope):
                {
                    AVRCP_BROWSING_DEBUG(("Scope: Filesystem start\n"));
                    /* TODO start of file system display - msg->num_items returned */                    
                    break;
                }
                case SCOPE_NON_ZERO(avrcp_search_scope):
                {
                    AVRCP_BROWSING_DEBUG(("Scope: Search start\n"));
                    /* TODO start of search result display - msg->num_items returned */
                    break;
                }
                default:
                {
                    AVRCP_BROWSING_DEBUG(("Scope: Unknown start\n"));
                    break;
                }
            }        
        
            header_end = AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_HEADER_SIZE; /* item_type and item_length must exist */
            while (item_list_size >= header_end) /* check there is room left for data to exist */
            { 
                item_type = lSource[i];
                item_length = (lSource[i + 1] << 8) | lSource[i + 2];
                switch (item_type)
                {
                    case AVRCP_BROWSABLE_ITEM_TYPE_MEDIA_ELEMENT:
                    {
                        AVRCP_BROWSING_DEBUG(("   Media Element\n"));
                        avrcpBrowseMediaElementItem(item_list_size - header_end, item_length, &lSource[i + 3]);
                        break;
                    }
                    case AVRCP_BROWSABLE_ITEM_TYPE_MEDIA_PLAYER:
                    {
                        AVRCP_BROWSING_DEBUG(("   Media Player\n"));
                        avrcpBrowseMediaPlayerItem(Index, item_list_size - header_end, item_length, &lSource[i + 3]);
                        break;
                    }
                    case AVRCP_BROWSABLE_ITEM_TYPE_FOLDER:                
                    {
                        AVRCP_BROWSING_DEBUG(("   Folder\n"));
                        avrcpBrowseFolderItem(Index, item_list_size - header_end, item_length, &lSource[i + 3]);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                   
                i = header_end + item_length;            
                header_end = i + AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_HEADER_SIZE;
            } 
        }
        else
        {
            AVRCP_BROWSING_DEBUG(("   fail; no source\n"));
            avrcp_status = avrcp_fail;
        }       
    }
    else
    {
        AVRCP_BROWSING_DEBUG(("   fail; status %d\n", msg->status));
    }    
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        switch (sinkAvrcpGetBrowsingScope(Index))
        {
            case SCOPE_NON_ZERO(avrcp_media_player_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Media Player end\n"));
                /* TODO end of media player display */
                break;
            }
            case SCOPE_NON_ZERO(avrcp_now_playing_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Now Playing end\n"));
                /* TODO end of now playing display */
                break;
            }
            case SCOPE_NON_ZERO(avrcp_virtual_filesystem_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Filesystem end \n"));
                /* TODO end of filesystem display */
                break;
            }
            case SCOPE_NON_ZERO(avrcp_search_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Search end\n"));
                /* TODO end of search results display */
                break;
            }
            default:
            {
                AVRCP_BROWSING_DEBUG(("Scope: Unknown end\n"));
                break;
            }
        }                
        sinkAvrcpSetBrowsingScope(Index, 0);                  
    }
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingGetNumberOfItemsCfm
    
DESCRIPTION
    Handle AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM AVRCP library message

**************************************************************************/
void sinkAvrcpBrowsingGetNumberOfItemsCfm(const AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM_T *msg)
{
    uint16 Index;
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {   
        AVRCP_BROWSING_DEBUG(("   success; uid_counter[%d] num_items[%d] \n", msg->uid_counter, msg->num_items));
        
        switch (sinkAvrcpGetBrowsingScope(Index))
        {
            case SCOPE_NON_ZERO(avrcp_media_player_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Media Player start\n"));
                /* TODO start of media player display - msg->num_items returned */
                break;
            }
            case SCOPE_NON_ZERO(avrcp_now_playing_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Now Playing start\n"));
                /* TODO start of now playing display - msg->num_items returned */
                break;
            }
            case SCOPE_NON_ZERO(avrcp_virtual_filesystem_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Filesystem start\n"));
                /* TODO start of file system display - msg->num_items returned */                    
                break;
            }
            case SCOPE_NON_ZERO(avrcp_search_scope):
            {
                AVRCP_BROWSING_DEBUG(("Scope: Search start\n"));
                /* TODO start of search result display - msg->num_items returned */
                break;
            }
            default:
            {
                AVRCP_BROWSING_DEBUG(("Scope: Unknown start\n"));
                break;
            }
        }
        sinkAvrcpSetBrowsingScope(Index, 0);                  
    }
    else
    {
        AVRCP_BROWSING_DEBUG(("   fail; status %d\n", msg->status));
    }
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingSetAddressedMediaPlayerRequest
    
DESCRIPTION
    Sets the addressed media player on TG that supports multiple media players.
    The AVRCP_SET_ADDRESSED_PLAYER_CFM message from the AVRCP library will be recieved as a response to this request.

**************************************************************************/
bool sinkAvrcpBrowsingSetAddressedMediaPlayerRequest(uint16 player_id)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Set Addressed Media Player %d\n", player_id));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            AvrcpSetAddressedPlayerRequest(sinkAvrcpGetProfileInstance(Index), player_id);
            
            return TRUE;
        }
    }    
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingSetBrowsedMediaPlayerRequest
    
DESCRIPTION
    Sets the browsed media player on TG.
    The function sinkAvrcpBrowsingSetBrowsedPlayerCfm will be called as a result of receiving the AVRCP_BROWSE_SET_PLAYER_CFM message 
    from the AVRCP library as a response to this request.    

**************************************************************************/
bool sinkAvrcpBrowsingSetBrowsedMediaPlayerRequest(uint16 player_id)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Set Browsed Media Player %d\n", player_id));
  
    if (sinkAvrcpIsConnected(Index))
    {   
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                AvrcpBrowseSetPlayerRequest(sinkAvrcpGetProfileInstance(Index), player_id);                                  
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_SET_PLAYER);                
                message->player_id = player_id;                
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_SET_PLAYER, message, Index);
            }
            return TRUE;
        }        
    }    
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingSetBrowsedPlayerCfm
    
DESCRIPTION
    Confirmation of setting the browsed player

**************************************************************************/
void sinkAvrcpBrowsingSetBrowsedPlayerCfm(const AVRCP_BROWSE_SET_PLAYER_CFM_T *msg)
{
    if (msg->status == avrcp_success)
    {        
        uint16 offset = 0;
        uint16 folder_items = 0;
        uint16 folder_name_length = 0;
        const uint8 *lSource = SourceMap(msg->folder_path);
        uint16 source_size = SourceSize(msg->folder_path);
        uint16 size_path = (source_size < msg->size_path) ? source_size : msg->size_path;
        
        AVRCP_BROWSING_DEBUG(("   success; uid_counter[0x%x] num_items[0x%lx] char_type[0x%x] folder_depth[0x%x] size_path[0x%x] source_size[%d]\n", 
                                                                                msg->uid_counter,
                                                                                msg->num_items,
                                                                                msg->char_type,
                                                                                msg->folder_depth,
                                                                                msg->size_path,
                                                                                SourceSize(msg->folder_path)));
        if (lSource)
        {
            AVRCP_BROWSING_DEBUG(("   current folder: "));        
            while (offset < size_path)
            {
                folder_name_length = lSource[offset++]; 
                if ((size_path - offset) >= folder_name_length)
                {                  
#ifdef DEBUG_AVRCP        
                    uint16 i;
                    for (i = offset; i < offset + folder_name_length; i++)
                    {
                        AVRCP_BROWSING_DEBUG(("%c", lSource[i]));        
                    }  
                    AVRCP_BROWSING_DEBUG(("\\"));
#endif                    
                }             
                offset += folder_name_length;
                folder_items += 1;
                /* see if the returned folder depth has been reached */
                if (folder_items >= msg->folder_depth)
                    break;
            }
            AVRCP_BROWSING_DEBUG(("\n"));
        }
    }
    else
    {
        AVRCP_BROWSING_DEBUG(("   fail; %d\n", msg->status));
    }
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingAddressedPlayerChangedInd
    
DESCRIPTION
    Notification of Addressed Player

**************************************************************************/
void sinkAvrcpBrowsingAddressedPlayerChangedInd(const AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND_T *msg)
{
    uint16 Index;
                
    AVRCP_BROWSING_DEBUG(("   addressed player changed ind [%d] [0x%x][0x%x]\n", msg->response, msg->player_id, msg->uid_counter));
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {         
            /* store that this command is supported by remote device */
            sinkAvrcpUpdateRegisteredEvent(Index,avrcp_event_addressed_player_changed);
        
            if (msg->response == avctp_response_changed)
            {
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_addressed_player_changed, 0);   
                /* retrieve the information for these media players */
                sinkAvrcpBrowsingRetrieveMediaPlayersRequest(0, AVRCP_MAX_MEDIA_PLAYERS - 1);
            }
            sinkAvrcpSetBrowsingMediaPlayerId(Index,msg->player_id);
            sinkAvrcpSetBrowsingUIDCounter(Index,msg->uid_counter);
        }     
        else
        {
            /* assume not supported by remote device */
            sinkAvrcpResetRegisteredEvent(Index,avrcp_event_addressed_player_changed);
        }
    }
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingRegisterChangedUIDNotification
    
DESCRIPTION
    Registers for Changed UID notification with the target

**************************************************************************/
void sinkAvrcpBrowsingRegisterChangedUIDNotification(AVRCP *avrcp)
{
    if(avrcp)
        AvrcpRegisterNotificationRequest(avrcp, avrcp_event_uids_changed, 0);
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingRetrieveFileSystemRequest
    
DESCRIPTION
    Retrieves the current tracks and folders in the remote Filesystem.
    The function sinkAvrcpBrowsingGetFolderItemsCfm will be called when a response to this request is received.
    
        start_index - the index of the first File System item to return
        end_index - the index of the last File System item to return

**************************************************************************/
bool sinkAvrcpBrowsingRetrieveFileSystemRequest(uint16 start_index, uint16 end_index)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Get File System start[0x%x] end [0x%x]\n", start_index, end_index));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                if (sinkAvrcpGetBrowsingScope(Index))
                {
                    MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_FILESYSTEM);                    
                    message->start_index = start_index;
                    message->end_index = end_index;                        
                    MessageSendConditionally(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_FILESYSTEM, message, sinkAvrcpBrowsingScopeBusy(Index));        
                }
                else
                {
                    uint16 size_media_attributes = sizeof(avrcp_retrieve_media_attributes_basic);
                    Source src_media_attributes = StreamRegionSource(avrcp_retrieve_media_attributes_basic, size_media_attributes);
                    sinkAvrcpSetBrowsingScope(Index, SCOPE_NON_ZERO(avrcp_virtual_filesystem_scope));                  
                    AvrcpBrowseGetFolderItemsRequest(sinkAvrcpGetProfileInstance(Index),   
                                                        avrcp_virtual_filesystem_scope,   
                                                        start_index,   
                                                        end_index,      
                                                        AVRCP_NUMBER_MEDIA_ATTRIBUTES_BASIC, 
                                                        src_media_attributes);
                }
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_FILESYSTEM);                
                message->start_index = start_index;
                message->end_index = end_index;    
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_FILESYSTEM, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;
}

bool sinkAvrcpBrowsingRetrieveNumberOfItemsRequest(avrcp_browse_scope browsing_scope)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Get Number of Items for scope[%d]\n", browsing_scope));
  
    if (sinkAvrcpIsConnected(Index))
    {
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                AvrcpBrowseGetNumberOfItemsRequest(sinkAvrcpGetProfileInstance(Index),   
                                                    browsing_scope);
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_NUMBER_OF_ITEMS);
                message->scope = browsing_scope;
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_NUMBER_OF_ITEMS, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingChangePathRequest
    
DESCRIPTION
    Changes the path in the remote Filesystem.
    The function sinkAvrcpBrowsingChangePathCfm will be called as a result of receiving the AVRCP_BROWSE_CHANGE_PATH_CFM message 
    from the AVRCP library as a response to this request.    

**************************************************************************/
bool sinkAvrcpBrowsingChangePathRequest(avrcp_browse_direction direction, avrcp_browse_uid folder_uid)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Change path direction[0x%x] folder_uid [0x%lx 0x%lx]\n", direction, folder_uid.msb, folder_uid.lsb));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                AvrcpBrowseChangePathRequest(sinkAvrcpGetProfileInstance(Index),  
                                               sinkAvrcpGetBrowsingUIDCounter(Index),
                                                direction,
                                                folder_uid); 
               
                    
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_CHANGE_PATH);                
                message->direction = direction;
                message->folder_uid = folder_uid;    
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_CHANGE_PATH, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;       
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingPlayBrowsedItemRequest
    
DESCRIPTION
    Plays the browsed item determined by unique UID.
    The AVRCP_PLAY_ITEM_CFM message will be returned from the AVRCP library as a response to this request.

**************************************************************************/
bool sinkAvrcpBrowsingPlayBrowsedItemRequest(avrcp_browse_uid uid)
{
    return sinkAvrcpBrowsingPlayItem(uid, avrcp_virtual_filesystem_scope);
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingAddBrowsedToNowPlayingRequest
    
DESCRIPTION
    Adds the browsed item determined by unique UID to the now playing folder.
    The AVRCP_ADD_TO_NOW_PLAYING_CFM message from the AVRCP library will be recieved as a response to this request.

**************************************************************************/
bool sinkAvrcpBrowsingAddBrowsedToNowPlayingRequest(avrcp_browse_uid uid)
{
    return sinkAvrcpBrowsingAddToNowPlaying(uid, avrcp_virtual_filesystem_scope);
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingChangePathCfm
    
DESCRIPTION
    Result of changing path in the remote Filesystem

**************************************************************************/
void sinkAvrcpBrowsingChangePathCfm(const AVRCP_BROWSE_CHANGE_PATH_CFM_T *msg)
{
    if (msg->status == avrcp_success)
    {
        /* TODO filesystem path has changed, may need to update display of virtual filesystem */
    }
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingUIDsChangedInd
    
DESCRIPTION
    Notification that UIDs have changed.

**************************************************************************/
void sinkAvrcpBrowsingUIDsChangedInd(const AVRCP_EVENT_UIDS_CHANGED_IND_T *msg)
{
    uint16 Index;
                
    AVRCP_BROWSING_DEBUG(("   UIDs changed ind [%d] uid_counter[0x%x]\n", msg->response, msg->uid_counter));
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {                    
            /* store current UID counter */
            sinkAvrcpSetBrowsingUIDCounter(Index, msg->uid_counter);
            
            if (msg->response == avctp_response_changed)
            {
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_uids_changed, 0);   

                if (sinkAvrcpGetActiveConnection() == Index) /* update display if this is the active connection */
                {
                    /* TODO if any UIDs have been stored then will need to send a command to receive new UIDs */
                }
            }
        }     
        else
        {
            /* assume not supported by remote device */
            sinkAvrcpResetEventCapabilities(Index,avrcp_event_uids_changed);
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingAvailablePlayersChangedInd
    
DESCRIPTION
    Notification that available players have changed.

**************************************************************************/
void sinkAvrcpBrowsingAvailablePlayersChangedInd(const AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND_T *msg)
{
    uint16 Index;
                
    AVRCP_BROWSING_DEBUG(("   Available players changed ind [%d]\n", msg->response));
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {                           
            if (msg->response == avctp_response_changed)
            {
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_available_players_changed, 0);   

                if (sinkAvrcpGetActiveConnection() == Index) /* update display if this is the active connection */
                {
                    /* TODO if relying on any media player information, then will need to retrieve new media player information */
                }
            }
        }     
        else
        {
            /* assume not supported by remote device */
            sinkAvrcpResetEventCapabilities(Index,avrcp_event_available_players_changed);
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpSearchIsSupported
    
DESCRIPTION
    Checks that both ends support Searching.

RETURNS
    Returns TRUE if Searching supported, FALSE otherwise.
**************************************************************************/
bool sinkAvrcpSearchIsSupported(uint16 Index)
{
    if ((sinkAvrcpGetExtensions(Index) & AVRCP_SEARCH_SUPPORTED) == AVRCP_SEARCH_SUPPORTED)
    {
        if (sinkAvrcpGetBrowsingMediaPlayerFeatures(Index) != AVRCP_BROWSING_PLAYER_FEATURES_INVALID)
        {
            if (sinkAvrcpGetBrowsingMediaPlayerFeatures(Index)& AVRCP_BROWSING_PLAYER_FEATURE_O7_SEARCHING)
                return TRUE;
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingSearchRequest
    
DESCRIPTION
    Searches for tracks within the currently browsed folder.
    The function sinkAvrcpBrowsingSearchCfm will be called as a result of receiving the AVRCP_BROWSE_SEARCH_CFM message 
    from the AVRCP library as a response to this request.
    
        size_string - the size of string
        string - the search string

**************************************************************************/
bool sinkAvrcpBrowsingSearchRequest(uint16 size_string, Source string)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Search\n"));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpSearchIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                AvrcpBrowseSearchRequest(sinkAvrcpGetProfileInstance(Index),
                                            avrcp_char_set_utf_8, 
                                            size_string,
                                            string);
               
                    
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_SEARCH);                
                message->size_string = size_string;
                message->string = string;    
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_SEARCH, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;       
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingRetrieveSearchItemsRequest
    
DESCRIPTION
    Returns the List of Media Tracks that matched the search criteria.
    The function sinkAvrcpBrowsingGetFolderItemsCfm will be called when a response to this request is received.
    
        start_index - the index of the first Search item to return
        end_index - the index of the last Search item to return

**************************************************************************/
bool sinkAvrcpBrowsingRetrieveSearchItemsRequest(uint16 start_index, uint16 end_index)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_BROWSING_DEBUG(("AVRCP: Avrcp Retrieve Search Items start[0x%x] end [0x%x]\n", start_index, end_index));
  
    if (sinkAvrcpIsConnected(Index))
    {               
        if (sinkAvrcpBrowsingIsSupported(Index))
        {
            if (sinkAvrcpBrowsingChannelIsConnected(Index))
            {
                sinkAvrcpBrowsingCancelDisconnectOnIdle(Index);
                
                if (sinkAvrcpGetBrowsingScope(Index))
                {
                    MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_SEARCH_LIST);                   
                    message->start_index = start_index;
                    message->end_index = end_index;                        
                    MessageSendConditionally(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_SEARCH_LIST, message, sinkAvrcpBrowsingScopeBusy(Index) );        
                }
                else
                {
                    uint16 size_media_attributes = sizeof(avrcp_retrieve_media_attributes_basic);
                    Source src_media_attributes = StreamRegionSource(avrcp_retrieve_media_attributes_basic, size_media_attributes);
                    sinkAvrcpSetBrowsingScope(Index, SCOPE_NON_ZERO(avrcp_search_scope));                                                            
                    /* retrieve now playing list */
                    AvrcpBrowseGetFolderItemsRequest(sinkAvrcpGetProfileInstance(Index),   
                                                        avrcp_search_scope,   
                                                        start_index,   
                                                        end_index,      
                                                        AVRCP_NUMBER_MEDIA_ATTRIBUTES_BASIC, 
                                                        src_media_attributes);
                }
            }
            else
            {  
                MAKE_AVRCP_MESSAGE(AVRCP_BROWSING_RETRIEVE_SEARCH_LIST);                
                message->start_index = start_index;
                message->end_index = end_index;    
                sinkAvrcpBrowsingChannelConnectRequest(sinkAvrcpGetProfileInstance(Index));
                sinkAvrcpBrowsingChannelSendMessageWhenConnected(sinkAvrcpGetBrowsingHandler(Index), AVRCP_BROWSING_RETRIEVE_SEARCH_LIST, message, Index);
            }
            return TRUE;
        }
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingPlaySearchItemRequest
    
DESCRIPTION
    Plays the searched item determined by unique UID.
    The AVRCP_PLAY_ITEM_CFM message will be returned from the AVRCP library as a response to this request.

**************************************************************************/
bool sinkAvrcpBrowsingPlaySearchItemRequest(avrcp_browse_uid uid)
{
    return sinkAvrcpBrowsingPlayItem(uid, avrcp_search_scope);
}


/*************************************************************************
NAME    
    sinkAvrcpBrowsingSearchCfm
    
DESCRIPTION
    Confirmation of search request.

**************************************************************************/
void sinkAvrcpBrowsingSearchCfm(const AVRCP_BROWSE_SEARCH_CFM_T *msg)
{
    if (msg->status == avrcp_success)
    {
        AVRCP_BROWSING_DEBUG(("   success uid_counter[0x%x] num_items[0x%lx]\n", msg->uid_counter, msg->num_items)); 
        /* now retrieve the actual media element items */
        sinkAvrcpBrowsingRetrieveSearchItemsRequest(0, msg->num_items - 1);
    }
    else
    {
        AVRCP_BROWSING_DEBUG(("   fail %d\n", msg->status));
    }    
}


#endif /* ENABLE_AVRCP_BROWSING */

#else /* ENABLE_AVRCP*/
static const int avrcp_disabled;
#endif /* ENABLE_AVRCP */
